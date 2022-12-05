#include "temporal_aa.h"

namespace Raysterizer
{
    namespace Pass
    {
        #define HALTON_SAMPLES 16

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct TAAPushConstants
        {
            glm::vec4 texel_size;
            glm::vec4 current_prev_jitter;
            glm::vec4 time_params;
            float     feedback_min;
            float     feedback_max;
            int       sharpen;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        float halton_sequence(int base, int index)
        {
            float result = 0;
            float f = 1;
            while (index > 0)
            {
                f /= base;
                result += f * (index % base);
                index = floor(index / base);
            }

            return result;
        }

        // -----------------------------------------------------------------------------------------------------------------------------------


        Error TemporalAA::Setup(std::shared_ptr<CommonResources> common_resources_)
        {
            common_resources = common_resources_;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            width = common_resources->width;
            height = common_resources->height;

            CreateImages();

            jitter_samples.clear();

            for (int i = 1; i <= HALTON_SAMPLES; i++)
            {
                jitter_samples.push_back(glm::vec2((2.0f * halton_sequence(2, i) - 1.0f), (2.0f * halton_sequence(3, i) - 1.0f)));
            }

            return NoError();
        }

        void TemporalAA::Render(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Temporal AA");

            auto delta_seconds = 0.0;
            Update();

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            auto subresource_range = vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            if (enabled)
            {
                const uint32_t NUM_THREADS = 32;
                const uint32_t write_idx = (uint32_t)common_resources->ping_pong;
                const uint32_t read_idx = (uint32_t)!common_resources->ping_pong;

                {
                    PanicIfError(command_buffer->SetImageLayout(images[write_idx], vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));
                }

                {
                    auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                    {
                        ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("other/taa.comp")},
                    };

                    auto plci = PipelineLayoutCreateInfo
                    {
                        shader_module_create_infos,
                    };

                    CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
                    CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
                    CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

                    ComputePipelineCreateInfo compute_pipeline_create_info{ pl };
                    CMShared<ComputePipeline> cp = AssignOrPanic(c.Get(compute_pipeline_create_info));

                    pipeline_layout = pl;
                    pipeline = cp;
                    c.SetName(pipeline_layout, "Reset Args Pipeline Layout");
                    c.SetName(pipeline, "Reset Args Compute Pipeline");

                    cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                    {
                        std::vector<WriteDescriptorSetBindedResource> read_ds;

                        if (common_resources->current_visualization_type == VISUALIZATION_TYPE_FINAL)
                        {
                            read_ds = common_resources->deferred_shading_pass->OutputBindings();
                        }
                        else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_SHADOWS)
                        {
                            read_ds = common_resources->ray_traced_shadows_pass->OutputBindings();
                        }
                        else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_AMBIENT_OCCLUSION)
                        {
                            read_ds = common_resources->ray_traced_ao_pass->OutputBindings();
                        }
                        else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_REFLECTIONS)
                        {
                            read_ds = common_resources->ray_traced_reflections_pass->OutputBindings();
                        }
                        else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_GLOBAL_ILLUIMINATION)
                        {
                            read_ds = common_resources->ddgi_pass->OutputBindings();
                        }
                        else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_GROUND_TRUTH)
                        {
                            read_ds = { common_resources->g_buffer_pass->GetGBuffer1Texture() };
                        }
                        else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_NORMALS)
                        {
                            read_ds = { common_resources->g_buffer_pass->GetGBuffer2Texture() };
                        }
                        else
                        {
                            PANIC("Not supported");
                        }

                        for (auto& r : read_ds)
                        {
                            if (auto texture = std::get_if<CMShared<Texture>>(&r))
                            {
                                (*texture)->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
                            }
                        }

                        PanicIfError(ds->Bind(0, 0, textures[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                        PanicIfError(ds->Bind(1, 0, read_ds));
                        PanicIfError(ds->Bind(2, 0, textures[read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                        PanicIfError(ds->Bind(3, 1, common_resources->g_buffer_pass->GetGBuffer2Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                        PanicIfError(ds->Bind(3, 3, common_resources->g_buffer_pass->GetDepthTexture(), vk::ImageLayout::eShaderReadOnlyOptimal));

                        PanicIfError(render_frame.FlushPendingWrites(ds));

                        TAAPushConstants push_constants;

                        push_constants.texel_size = glm::vec4(1.0f / float(width), 1.0f / float(height), float(width), float(height));
                        push_constants.current_prev_jitter = glm::vec4(current_jitter, prev_jitter);
                        push_constants.time_params = glm::vec4(static_cast<float>(glfwGetTime()), sinf(static_cast<float>(glfwGetTime())), cosf(static_cast<float>(glfwGetTime())), delta_seconds);
                        push_constants.feedback_min = feedback_min;
                        push_constants.feedback_max = feedback_max;
                        push_constants.sharpen = static_cast<int>(sharpen);

                        PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                    }

                    auto block_x = static_cast<uint32_t>(ceil(float(width) / float(NUM_THREADS)));
                    auto block_y = static_cast<uint32_t>(ceil(float(height) / float(NUM_THREADS)));
                    cb.dispatch(block_x, block_y, 1);
                }

                {
                    PanicIfError(command_buffer->SetImageLayout(images[write_idx], vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                }
            }
        }

        void TemporalAA::Update()
        {
            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            if (enabled)
            {
                prev_jitter = current_jitter;
                uint32_t  sample_idx = num_frames % (jitter_samples.size());
                glm::vec2 halton = jitter_samples[sample_idx];

                current_jitter = glm::vec2(halton.x / float(width), halton.y / float(height));
            }
            else
            {
                prev_jitter = glm::vec2(0.0f);
                current_jitter = glm::vec2(0.0f);
            }
        }

        void TemporalAA::UpdateGui()
        {
            if (ImGui::Checkbox("Enabled", &enabled))
            {
                if (enabled)
                {
                    reset = true;
                }
            }
            ImGui::Checkbox("Sharpen", &sharpen);
            ImGui::SliderFloat("Feedback Min", &feedback_min, 0.0f, 1.0f);
            ImGui::SliderFloat("Feedback Max", &feedback_max, 0.0f, 1.0f);
        }

        void TemporalAA::CreateImages()
        {
            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto nearest_sampler = common_resources->nearest_sampler;

            images.clear();
            views.clear();
            textures.clear();

            for (auto i = 0; i < common_resources->ping_pong_size; i++)
            {
                auto image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                    vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                c.SetName(image, fmt::format("TemporalAA Image {}", i));

                auto view = CommonResources::CreateImageView(image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                c.SetName(view, fmt::format("TemporalAA Image View {}", i));

                auto texture = std::make_shared<Texture>(image, view, nearest_sampler);

                images.emplace_back(image);
                views.emplace_back(view);
                textures.emplace_back(texture);
            }
        }
        
        bool TemporalAA::Enabled() const
        {
            return enabled;
        }

        glm::vec2 TemporalAA::PrevJitter() const
        {
            return prev_jitter;
        }

        glm::vec2 TemporalAA::CurrentJitter() const
        {
            return current_jitter;
        }
        
        std::vector<WriteDescriptorSetBindedResource> TemporalAA::OutputBindings() const
        {
            return { textures[common_resources->ping_pong] };
        }
    }
}