#include "ray_traced_ao.h"

namespace Raysterizer
{
    namespace Pass
    {
        // -----------------------------------------------------------------------------------------------------------------------------------

        static const int RAY_TRACE_NUM_THREADS_X = 8;
        static const int RAY_TRACE_NUM_THREADS_Y = 4;

        static const uint32_t TEMPORAL_ACCUMULATION_NUM_THREADS_X = 8;
        static const uint32_t TEMPORAL_ACCUMULATION_NUM_THREADS_Y = 8;

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct AORayTracePushConstants
        {
            uint32_t num_frames;
            float    ray_length;
            float    bias;
            int32_t  g_buffer_mip;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct AOTemporalReprojectionPushConstants
        {
            float   alpha;
            int32_t g_buffer_mip;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct AOBilateralBlurPushConstants
        {
            glm::vec4  z_buffer_params;
            glm::ivec2 direction;
            int32_t    radius;
            int32_t    g_buffer_mip;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct AOUpsamplePushConstants
        {
            int32_t g_buffer_mip;
            float   power;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        const RayTracedAO::OutputType RayTracedAO::kOutputTypeEnums[] = {
            RayTracedAO::OUTPUT_RAY_TRACE,
            RayTracedAO::OUTPUT_TEMPORAL_ACCUMULATION,
            RayTracedAO::OUTPUT_BILATERAL_BLUR,
            RayTracedAO::OUTPUT_UPSAMPLE
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        const std::string RayTracedAO::kOutputTypeNames[] = {
            "Ray Trace",
            "Temporal Accumulation",
            "Bilateral Blur",
            "Upsample"
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        Error RayTracedAO::Setup(std::shared_ptr<CommonResources> common_resources_)
        {
            common_resources = common_resources_;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            scale = common_resources->ao.scale;

            float scale_divisor = powf(2.0f, float(scale));

            width = common_resources->width / scale_divisor;
            height = common_resources->height / scale_divisor;

            g_buffer_mip = static_cast<uint32_t>(scale);

            auto nearest_sampler = common_resources->nearest_sampler;

            temporal_accumulation.color_image.clear();
            temporal_accumulation.color_view.clear();
            temporal_accumulation.color_texture.clear();

            temporal_accumulation.history_length_image.clear();
            temporal_accumulation.history_length_view.clear();
            temporal_accumulation.history_length_texture.clear();

            bilateral_blur.image.clear();
            bilateral_blur.image_view.clear();
            bilateral_blur.texture.clear();

            // Create images
            {
                // Ray Trace
                {
                    auto image_width = static_cast<uint32_t>(ceil(float(width) / float(RAY_TRACE_NUM_THREADS_X)));
                    auto image_height = static_cast<uint32_t>(ceil(float(height) / float(RAY_TRACE_NUM_THREADS_Y)));

                    ray_trace.image = CommonResources::CreateImage(vk::ImageType::e2D, image_width, image_height, 1, 1, 1,
                        vk::Format::eR32Uint, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(ray_trace.image, fmt::format("AO Ray Trace Image"));

                    ray_trace.view = CommonResources::CreateImageView(ray_trace.image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(ray_trace.view, fmt::format("AO Ray Trace Image View"));

                    ray_trace.texture = std::make_shared<Texture>(ray_trace.image, ray_trace.view, nearest_sampler);
                }

                // Reprojection
                {
                    for (int i = 0; i < common_resources->ping_pong_size; i++)
                    {
                        auto color_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                            vk::Format::eR16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                        c.SetName(color_image, fmt::format("AO Reprojection Color Image {}", i));

                        auto color_view = CommonResources::CreateImageView(color_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                        c.SetName(color_view, fmt::format("AO Reprojection Color Image View {}", i));

                        auto color_texture = std::make_shared<Texture>(color_image, color_view, nearest_sampler);

                        temporal_accumulation.color_image.emplace_back(color_image);
                        temporal_accumulation.color_view.emplace_back(color_view);
                        temporal_accumulation.color_texture.emplace_back(color_texture);

                        auto history_length_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                            vk::Format::eR16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                        c.SetName(history_length_image, fmt::format("AO Reprojection Color Image {}", i));

                        auto history_length_view = CommonResources::CreateImageView(history_length_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                        c.SetName(history_length_view, fmt::format("AO Reprojection Color Image View {}", i));

                        auto history_length_texture = std::make_shared<Texture>(history_length_image, history_length_view, nearest_sampler);

                        temporal_accumulation.history_length_image.emplace_back(history_length_image);
                        temporal_accumulation.history_length_view.emplace_back(history_length_view);
                        temporal_accumulation.history_length_texture.emplace_back(history_length_texture);
                    }
                }

                // Bilateral Blur
                for (int i = 0; i < common_resources->ping_pong_size; i++)
                {
                    auto image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                        vk::Format::eR16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(image, fmt::format("AO Reprojection Blur Image {}", i));

                    auto image_view = CommonResources::CreateImageView(image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(image_view, fmt::format("AO Reprojection Blur Image View {}", i));

                    auto texture = std::make_shared<Texture>(image, image_view, nearest_sampler);

                    bilateral_blur.image.emplace_back(image);
                    bilateral_blur.image_view.emplace_back(image_view);
                    bilateral_blur.texture.emplace_back(texture);
                }

                // Upsample
                {
                    auto swapchain_width = common_resources->width;
                    auto swapchain_height = common_resources->height;
                    upsample.image = CommonResources::CreateImage(vk::ImageType::e2D, swapchain_width, swapchain_height, 1, 1, 1,
                        vk::Format::eR16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(upsample.image, fmt::format("AO Upsample Image"));

                    upsample.image_view = CommonResources::CreateImageView(upsample.image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(upsample.image_view, fmt::format("AO Upsample Image View"));

                    upsample.texture = std::make_shared<Texture>(upsample.image, upsample.image_view, nearest_sampler);
                }
            }

            // Create buffers
            {
                auto denoise_tile_coords_buffer_size = sizeof(glm::ivec2) * static_cast<uint32_t>(ceil(float(width) / float(TEMPORAL_ACCUMULATION_NUM_THREADS_X))) *
                    static_cast<uint32_t>(ceil(float(height) / float(TEMPORAL_ACCUMULATION_NUM_THREADS_Y)));
                temporal_accumulation.denoise_tile_coords_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer, denoise_tile_coords_buffer_size));
                temporal_accumulation.denoise_dispatch_args_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer, sizeof(int32_t) * 3));
                c.SetName(temporal_accumulation.denoise_tile_coords_buffer, fmt::format("Denoise tile coords buffer"));
                c.SetName(temporal_accumulation.denoise_dispatch_args_buffer, fmt::format("Denoise tile coords args buffer"));
            }

            return NoError();
        }

        void RayTracedAO::Render(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "AO");

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;

            ClearImage(command_buffer);
            Raytrace(command_buffer);

            if (denoise)
            {
                Denoise(command_buffer);

                if (scale != RAY_TRACE_SCALE_FULL_RES)
                {
                    Upsample(command_buffer);
                }
            }
        }

        void RayTracedAO::UpdateGui()
        {
            ImGui::Checkbox("Denoise", &denoise);
            ImGui::SliderFloat("Ray Length", &ray_trace.ray_length, 1.0f, 1000.0f);
            ImGui::SliderFloat("Power", &upsample.power, 1.0f, 5.0f);
            ImGui::InputFloat("Bias", &ray_trace.bias);
            ImGui::SliderFloat("Temporal Alpha", &temporal_accumulation.alpha, 0.0f, 0.5f);
            ImGui::Checkbox("Bilateral blur", &bilateral_blur.enabled);
            ImGui::SliderInt("Blur Radius", &bilateral_blur.blur_radius, 1, 10);
        }

        void RayTracedAO::ClearImage(CMShared<CommandBuffer> command_buffer)
        {
            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;

            if (first_frame)
            {
                auto subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.history_length_image[!common_resources->ping_pong], vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));
                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.color_image[!common_resources->ping_pong], vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));

                vk::ClearColorValue color;

                color.float32[0] = 0.0f;
                color.float32[1] = 0.0f;
                color.float32[2] = 0.0f;
                color.float32[3] = 0.0f;

                cb.clearColorImage(*temporal_accumulation.history_length_image[!common_resources->ping_pong], vk::ImageLayout::eGeneral, color, subresource_range);
                cb.clearColorImage(*temporal_accumulation.color_image[!common_resources->ping_pong], vk::ImageLayout::eGeneral, color, subresource_range);

                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.history_length_image[!common_resources->ping_pong], vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.color_image[!common_resources->ping_pong], vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));

                first_frame = false;
            }
        }

        void RayTracedAO::Raytrace(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Ray Trace");

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

            {
                auto memory_barrier = vk::MemoryBarrier{}
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                auto image_memory_barrier = vk::ImageMemoryBarrier{}
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eGeneral)
                    .setImage(**ray_trace.image)
                    .setSubresourceRange(subresource_range)
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite);

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eComputeShader, {}, memory_barrier, {}, image_memory_barrier);
            }

            // Ray Trace
            {
                const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("ao/ao_ray_trace.comp")},
                };

                flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};
                variable_set_index_to_count[0] = max_variable_bindings;

                auto plci = PipelineLayoutCreateInfo
                {
                    shader_module_create_infos,
                    variable_set_index_to_count,
                };

                CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
                CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
                CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

                ComputePipelineCreateInfo compute_pipeline_create_info{ pl };
                CMShared<ComputePipeline> cp = AssignOrPanic(c.Get(compute_pipeline_create_info));

                ray_trace.pipeline_layout = pl;
                ray_trace.pipeline = cp;
                c.SetName(ray_trace.pipeline_layout, "Ray Trace Pipeline Layout");
                c.SetName(ray_trace.pipeline, "Ray Trace Compute Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, common_resources->OutputBindings()));

                    PanicIfError(ds->Bind(1, 0, ray_trace.texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    
                    PanicIfError(ds->Bind(2, 0, common_resources->GetUBOBuffer()));
                    
                    PanicIfError(ds->Bind(3, 0, common_resources->g_buffer_pass->OutputBindings()));

                    PanicIfError(ds->Bind(4, 0, common_resources->blue_noise->GetSobolTexture()), vk::ImageLayout::eShaderReadOnlyOptimal);
                    PanicIfError(ds->Bind(4, 1, common_resources->blue_noise->GetSobolRankingTexture(BlueNoiseSpp::BLUE_NOISE_1SPP), vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    AORayTracePushConstants push_constants;

                    push_constants.bias = ray_trace.bias;
                    push_constants.ray_length = ray_trace.ray_length;
                    push_constants.num_frames = current_frame;
                    push_constants.g_buffer_mip = g_buffer_mip;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                auto block_x = static_cast<uint32_t>(ceil(float(width) / float(RAY_TRACE_NUM_THREADS_X)));
                auto block_y = static_cast<uint32_t>(ceil(float(height) / float(RAY_TRACE_NUM_THREADS_Y)));
                cb.dispatch(block_x, block_y, 1);
            }

            {
                PanicIfError(command_buffer->SetImageLayout(ray_trace.image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
            }
        }

        void RayTracedAO::Denoise(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Denoise");

            ResetArgs(command_buffer);
            TemporalAccumulation(command_buffer);
            BilateralBlur(command_buffer);
        }

        void RayTracedAO::ResetArgs(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Reset Args");

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            {
                std::vector<vk::BufferMemoryBarrier> buffer_barriers = {
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.denoise_tile_coords_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eIndirectCommandRead)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.denoise_dispatch_args_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect, vk::PipelineStageFlagBits::eComputeShader, {}, {}, buffer_barriers, {});
            }

            {
                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("ao/ao_denoise_reset_args.comp")},
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

                reset_args.pipeline_layout = pl;
                reset_args.pipeline = cp;
                c.SetName(reset_args.pipeline_layout, "Reset Args Pipeline Layout");
                c.SetName(reset_args.pipeline, "Reset Args Compute Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 1, temporal_accumulation.denoise_dispatch_args_buffer));
                    PanicIfError(render_frame.FlushPendingWrites(ds));
                }

                auto block_x = 1;
                auto block_y = 1;
                cb.dispatch(block_x, block_y, 1);
            }

            {
            }
        }

        void RayTracedAO::TemporalAccumulation(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Temporal Accumulation");

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto prev_frame_index = c.GetPrevFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            {
                auto subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eUndefined)
                        .setNewLayout(vk::ImageLayout::eGeneral)
                        .setImage(**temporal_accumulation.color_image[common_resources->ping_pong])
                        .setSubresourceRange(subresource_range)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eUndefined)
                        .setNewLayout(vk::ImageLayout::eGeneral)
                        .setImage(**temporal_accumulation.history_length_image[common_resources->ping_pong])
                        .setSubresourceRange(subresource_range)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                };

                std::vector<vk::BufferMemoryBarrier> buffer_barriers = {
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.denoise_tile_coords_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.denoise_dispatch_args_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, buffer_barriers, image_memory_barriers);
            }

            {
                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("ao/ao_denoise_reprojection.comp")},
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

                temporal_accumulation.pipeline_layout = pl;
                temporal_accumulation.pipeline = cp;
                c.SetName(temporal_accumulation.pipeline_layout, "AO Upstage Pipeline Layout");
                c.SetName(temporal_accumulation.pipeline, "AO Upstage Compute Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, temporal_accumulation.color_texture[common_resources->ping_pong], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(0, 1, temporal_accumulation.history_length_texture[common_resources->ping_pong], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(1, 0, common_resources->g_buffer_pass->OutputBindings()));

                    PanicIfError(ds->Bind(2, 0, common_resources->g_buffer_pass->HistoryBindings()));

                    PanicIfError(ds->Bind(3, 0, ray_trace.texture, vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(4, 0, temporal_accumulation.color_texture[!common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));

                    //PanicIfError(ds->Bind(5, 0, temporal_accumulation.color_texture[!common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(5, 1, temporal_accumulation.history_length_texture[!common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(6, 0, common_resources->GetUBOBuffer()));

                    PanicIfError(ds->Bind(7, 0, temporal_accumulation.denoise_tile_coords_buffer));
                    PanicIfError(ds->Bind(7, 1, temporal_accumulation.denoise_dispatch_args_buffer));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    AOTemporalReprojectionPushConstants push_constants;
                    push_constants.alpha = temporal_accumulation.alpha;
                    push_constants.g_buffer_mip = g_buffer_mip;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                auto block_x = static_cast<uint32_t>(ceil(float(width) / float(TEMPORAL_ACCUMULATION_NUM_THREADS_X)));
                auto block_y = static_cast<uint32_t>(ceil(float(height) / float(TEMPORAL_ACCUMULATION_NUM_THREADS_Y)));
                cb.dispatch(block_x, block_y, 1);
            }

            {
                auto subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                auto memory_barrier = vk::MemoryBarrier{}
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eGeneral)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setImage(**temporal_accumulation.color_image[common_resources->ping_pong])
                        .setSubresourceRange(subresource_range)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eGeneral)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setImage(**temporal_accumulation.history_length_image[common_resources->ping_pong])
                        .setSubresourceRange(subresource_range)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                };

                std::vector<vk::BufferMemoryBarrier> buffer_barriers = {
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setBuffer(**temporal_accumulation.denoise_tile_coords_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eIndirectCommandRead)
                        .setBuffer(**temporal_accumulation.denoise_dispatch_args_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect, {}, memory_barrier, buffer_barriers, image_memory_barriers);
            }
        }

        void RayTracedAO::BilateralBlur(CMShared<CommandBuffer> command_buffer)
        {
            if (!bilateral_blur.enabled)
            {
                return;
            }

            ScopedGPUProfileRaysterizer(command_buffer, "Bilateral Blur");

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

            auto& vertical_bilateral_blur_texture = bilateral_blur.texture[0];
            auto& horizontal_bilateral_blur_texture = bilateral_blur.texture[1];

            // Vertical
            {
                ScopedGPUProfileRaysterizer(command_buffer, "Vertical");

                auto& bilateral_blur_image = vertical_bilateral_blur_texture->image;
                auto& bilateral_blur_texture = vertical_bilateral_blur_texture;
                {
                    PanicIfError(command_buffer->SetImageLayout(bilateral_blur_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));

                    vk::ClearColorValue color;

                    color.float32[0] = 0.0f;
                    color.float32[1] = 0.0f;
                    color.float32[2] = 0.0f;
                    color.float32[3] = 0.0f;

                    cb.clearColorImage(*bilateral_blur_image, vk::ImageLayout::eGeneral, color, subresource_range);
                }

                {
                    auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                    {
                        ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("ao/ao_denoise_bilateral_blur.comp")},
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

                    upsample.layout = pl;
                    upsample.pipeline = cp;
                    c.SetName(upsample.layout, "AO Vertical Bilaterial Pipeline Layout");
                    c.SetName(upsample.pipeline, "AO Vertical Bilaterial Compute Pipeline");

                    cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                    {
                        PanicIfError(ds->Bind(0, 0, bilateral_blur_texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                        PanicIfError(ds->Bind(1, 0, temporal_accumulation.color_texture[common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));

                        //PanicIfError(ds->Bind(2, 0, temporal_accumulation.color_texture[common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));
                        PanicIfError(ds->Bind(2, 1, temporal_accumulation.history_length_texture[common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));

                        PanicIfError(ds->Bind(3, 0, common_resources->g_buffer_pass->OutputBindings()));

                        PanicIfError(ds->Bind(4, 0, temporal_accumulation.denoise_tile_coords_buffer));

                        PanicIfError(render_frame.FlushPendingWrites(ds));

                        AOBilateralBlurPushConstants push_constants;

                        push_constants.z_buffer_params = common_resources->ao.z_buffer_params;
                        push_constants.direction = glm::ivec2(1, 0);
                        push_constants.radius = bilateral_blur.blur_radius;
                        push_constants.g_buffer_mip = g_buffer_mip;

                        PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                    }

                    cb.dispatchIndirect(**temporal_accumulation.denoise_dispatch_args_buffer, 0);
                }

                {
                    PanicIfError(command_buffer->SetImageLayout(bilateral_blur_image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                }
            }

            // Horizontal
            {
                ScopedGPUProfileRaysterizer(command_buffer, "Horizontal");

                auto& bilateral_blur_image = horizontal_bilateral_blur_texture->image;
                auto& bilateral_blur_texture = horizontal_bilateral_blur_texture;
                {
                    PanicIfError(command_buffer->SetImageLayout(bilateral_blur_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));

                    vk::ClearColorValue color;

                    color.float32[0] = 0.0f;
                    color.float32[1] = 0.0f;
                    color.float32[2] = 0.0f;
                    color.float32[3] = 0.0f;

                    cb.clearColorImage(*bilateral_blur_image, vk::ImageLayout::eGeneral, color, subresource_range);
                }

                {
                    auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                    {
                        ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("ao/ao_denoise_bilateral_blur.comp")},
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

                    upsample.layout = pl;
                    upsample.pipeline = cp;
                    c.SetName(upsample.layout, "AO Horizontal Bilaterial Pipeline Layout");
                    c.SetName(upsample.pipeline, "AO Horizontal Bilaterial Compute Pipeline");

                    cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                    {
                        PanicIfError(ds->Bind(0, 0, bilateral_blur_texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                        PanicIfError(ds->Bind(1, 0, vertical_bilateral_blur_texture, vk::ImageLayout::eShaderReadOnlyOptimal));

                        //PanicIfError(ds->Bind(2, 0, vertical_bilateral_blur_texture, vk::ImageLayout::eShaderReadOnlyOptimal));
                        PanicIfError(ds->Bind(2, 1, temporal_accumulation.history_length_texture[common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));

                        PanicIfError(ds->Bind(3, 0, common_resources->g_buffer_pass->OutputBindings()));

                        PanicIfError(ds->Bind(4, 0, temporal_accumulation.denoise_tile_coords_buffer));

                        PanicIfError(render_frame.FlushPendingWrites(ds));

                        AOBilateralBlurPushConstants push_constants;

                        push_constants.z_buffer_params = common_resources->ao.z_buffer_params;
                        push_constants.direction = glm::ivec2(0, 1);
                        push_constants.radius = bilateral_blur.blur_radius;
                        push_constants.g_buffer_mip = g_buffer_mip;

                        PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                    }

                    cb.dispatchIndirect(**temporal_accumulation.denoise_dispatch_args_buffer, 0);
                }

                {
                    PanicIfError(command_buffer->SetImageLayout(bilateral_blur_image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                }
            }
        }

        void RayTracedAO::Upsample(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Upsample");

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

            {
                PanicIfError(command_buffer->SetImageLayout(upsample.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));
            }

            {
                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("ao/AO_upsample.comp")},
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

                upsample.layout = pl;
                upsample.pipeline = cp;
                c.SetName(upsample.layout, "AO Upsample Pipeline Layout");
                c.SetName(upsample.pipeline, "AO Upsample Compute Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, upsample.texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    if (bilateral_blur.enabled)
                    {
                        PanicIfError(ds->Bind(1, 0, bilateral_blur.texture[1], vk::ImageLayout::eShaderReadOnlyOptimal));
                    }
                    else
                    {
                        PanicIfError(ds->Bind(1, 0, temporal_accumulation.color_texture[common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));
                    }

                    PanicIfError(ds->Bind(2, 0, common_resources->g_buffer_pass->OutputBindings()));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    AOUpsamplePushConstants push_constants;
                    push_constants.g_buffer_mip = g_buffer_mip;
                    push_constants.power = upsample.power;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                const int NUM_THREADS_X = 8;
                const int NUM_THREADS_Y = 8;

                auto block_x = static_cast<uint32_t>(ceil(float(upsample.image->image_create_info.image_create_info.extent.width) / float(NUM_THREADS_X)));
                auto block_y = static_cast<uint32_t>(ceil(float(upsample.image->image_create_info.image_create_info.extent.height) / float(NUM_THREADS_Y)));
                cb.dispatch(block_x, block_y, 1);
            }

            {
                PanicIfError(command_buffer->SetImageLayout(upsample.image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
            }
        }
        
        std::vector<WriteDescriptorSetBindedResource> RayTracedAO::OutputBindings() const
        {
            if (denoise)
            {
                if (current_output == OUTPUT_RAY_TRACE)
                {
                    return { ray_trace.texture };
                }
                else if (current_output == OUTPUT_TEMPORAL_ACCUMULATION)
                {
                    return { temporal_accumulation.color_texture[common_resources->ping_pong] };
                }
                else if (current_output == OUTPUT_BILATERAL_BLUR)
                {
                    return { bilateral_blur.texture[common_resources->ping_pong] };
                }
                else
                {
                    if (scale == RAY_TRACE_SCALE_FULL_RES)
                    {
                        return { bilateral_blur.texture[1] };
                    }
                    else
                    {
                        return { upsample.texture };
                    }
                }
            }
            else
            {
                return { ray_trace.texture };
            }
        }
    }
}
