#include "ray_traced_shadows.h"

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

        struct ShadowRayTracePushConstants
        {
            float    bias;
            uint32_t num_frames;
            int32_t  g_buffer_mip;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct ShadowTemporalAccumulationPushConstants
        {
            float   alpha;
            float   moments_alpha;
            int32_t g_buffer_mip;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct ShadowATrousFilterPushConstants
        {
            int     radius;
            int     step_size;
            float   phi_visibility;
            float   phi_normal;
            float   sigma_depth;
            int32_t g_buffer_mip;
            float   power;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct ShadowUpsamplePushConstants
        {
            int32_t g_buffer_mip;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        const RayTracedShadows::OutputType RayTracedShadows::kOutputTypeEnums[] = {
            RayTracedShadows::OUTPUT_RAY_TRACE,
            RayTracedShadows::OUTPUT_TEMPORAL_ACCUMULATION,
            RayTracedShadows::OUTPUT_ATROUS,
            RayTracedShadows::OUTPUT_UPSAMPLE
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        const std::string RayTracedShadows::kOutputTypeNames[] = {
            "Ray Trace",
            "Temporal Accumulation",
            "A-Trous",
            "Upsample"
        };

        Error RayTracedShadows::Setup(std::shared_ptr<CommonResources> common_resources_)
        {
            common_resources = common_resources_;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            scale = common_resources->shadows.scale;

            float scale_divisor = powf(2.0f, float(scale));

            width = common_resources->width / scale_divisor;
            height = common_resources->height / scale_divisor;

            g_buffer_mip = static_cast<uint32_t>(scale);

            auto nearest_sampler = common_resources->nearest_sampler;

            temporal_accumulation.current_moments_image.clear();
            temporal_accumulation.current_moments_view.clear();
            temporal_accumulation.current_moments_texture.clear();

            a_trous.image.clear();
            a_trous.view.clear();
            a_trous.texture.clear();

            // Create images
            {
                // Ray Trace
                {
                    auto image_width = static_cast<uint32_t>(ceil(float(width) / float(RAY_TRACE_NUM_THREADS_X)));
                    auto image_height = static_cast<uint32_t>(ceil(float(height) / float(RAY_TRACE_NUM_THREADS_Y)));

                    ray_trace.image = CommonResources::CreateImage(vk::ImageType::e2D, image_width, image_height, 1, 1, 1,
                        vk::Format::eR32Uint, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(ray_trace.image, fmt::format("Shadows Ray Trace Image"));

                    ray_trace.view = CommonResources::CreateImageView(ray_trace.image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(ray_trace.view, fmt::format("Shadows Ray Trace Image View"));

                    ray_trace.texture = std::make_shared<Texture>(ray_trace.image, ray_trace.view, nearest_sampler);
                }

                // Reprojection
                {
                    temporal_accumulation.current_output_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                        vk::Format::eR16G16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(temporal_accumulation.current_output_image, fmt::format("Shadows Reprojection Output Image"));

                    temporal_accumulation.current_output_view = CommonResources::CreateImageView(temporal_accumulation.current_output_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(temporal_accumulation.current_output_view, fmt::format("Shadows Reprojection Output Image View"));

                    temporal_accumulation.current_output_texture = std::make_shared<Texture>(temporal_accumulation.current_output_image, temporal_accumulation.current_output_view, nearest_sampler);

                    for (int i = 0; i < common_resources->ping_pong_size; i++)
                    {
                        auto current_moments_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                            vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                        c.SetName(current_moments_image, fmt::format("Shadows Reprojection Moments Image {}", i));

                        auto current_moments_view = CommonResources::CreateImageView(current_moments_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                        c.SetName(current_moments_view, fmt::format("Shadows Reprojection Moments Image View {}", i));

                        auto current_moments_texture = std::make_shared<Texture>(current_moments_image, current_moments_view, nearest_sampler);

                        temporal_accumulation.current_moments_image.emplace_back(current_moments_image);
                        temporal_accumulation.current_moments_view.emplace_back(current_moments_view);
                        temporal_accumulation.current_moments_texture.emplace_back(current_moments_texture);
                    }

                    temporal_accumulation.prev_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                        vk::Format::eR16G16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(temporal_accumulation.prev_image, fmt::format("Shadows Previous Reprojection Image"));

                    temporal_accumulation.prev_view = CommonResources::CreateImageView(temporal_accumulation.prev_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(temporal_accumulation.prev_view, fmt::format("Shadows Previous Reprojection Image View"));

                    temporal_accumulation.prev_texture = std::make_shared<Texture>(temporal_accumulation.prev_image, temporal_accumulation.prev_view, nearest_sampler);
                }

                // A-Trous Filter
                for (int i = 0; i < common_resources->ping_pong_size; i++)
                {
                    auto image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                        vk::Format::eR16G16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(image, fmt::format("A-Trous Filter Image {}", i));

                    auto image_view = CommonResources::CreateImageView(image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(image_view, fmt::format("A-Trous Filter Image View {}", i));

                    auto texture = std::make_shared<Texture>(image, image_view, nearest_sampler);

                    a_trous.image.emplace_back(image);
                    a_trous.view.emplace_back(image_view);
                    a_trous.texture.emplace_back(texture);
                }

                // Upsample
                {
                    auto swapchain_width = common_resources->width;
                    auto swapchain_height = common_resources->height;
                    upsample.image = CommonResources::CreateImage(vk::ImageType::e2D, swapchain_width, swapchain_height, 1, 1, 1,
                        vk::Format::eR16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(upsample.image, fmt::format("Shadows Upsample Image"));

                    upsample.image_view = CommonResources::CreateImageView(upsample.image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(upsample.image_view, fmt::format("Shadows Upsample Image View"));

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


                auto shadow_tile_coords_buffer_size = sizeof(glm::ivec2) * static_cast<uint32_t>(ceil(float(width) / float(TEMPORAL_ACCUMULATION_NUM_THREADS_X))) * static_cast<uint32_t>(ceil(float(height) / float(TEMPORAL_ACCUMULATION_NUM_THREADS_Y)));
                temporal_accumulation.shadow_tile_coords_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer, shadow_tile_coords_buffer_size));
                temporal_accumulation.shadow_dispatch_args_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::GpuOnly, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer, sizeof(int32_t) * 3));
                c.SetName(temporal_accumulation.shadow_tile_coords_buffer, fmt::format("Shadow tile coords buffer"));
                c.SetName(temporal_accumulation.shadow_dispatch_args_buffer, fmt::format("Shadow tile coords args buffer"));
            }

            return NoError();
        }

        void RayTracedShadows::Render(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Ray Traced Shadows");

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            ClearImage(command_buffer);
            Raytrace(command_buffer);

            if (denoise)
            {
                ResetArgs(command_buffer);
                TemporalAccumulation(command_buffer);
                ATrousFilter(command_buffer);

                if (scale != RAY_TRACE_SCALE_FULL_RES)
                {
                    Upsample(command_buffer);
                }
            }
        }

        void RayTracedShadows::UpdateGui()
        {
            ImGui::Checkbox("Denoise", &denoise);
            ImGui::InputFloat("Bias", &ray_trace.bias);
            ImGui::InputFloat("Alpha", &temporal_accumulation.alpha);
            ImGui::InputFloat("Alpha Moments", &temporal_accumulation.moments_alpha);
            ImGui::InputFloat("Phi Visibility", &a_trous.phi_visibility);
            ImGui::InputFloat("Phi Normal", &a_trous.phi_normal);
            ImGui::InputFloat("Sigma Depth", &a_trous.sigma_depth);
            ImGui::SliderInt("Filter Iterations", &a_trous.filter_iterations, 1, 5);
            ImGui::SliderFloat("Power", &a_trous.power, 0.1f, 10.0f);
        }

        void RayTracedShadows::ClearImage(CMShared<CommandBuffer> command_buffer)
        {
            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            if (first_frame)
            {
                auto subresource_range = vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.prev_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));
                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.current_moments_image[!common_resources->ping_pong], vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, subresource_range));

                vk::ClearColorValue color;

                color.float32[0] = 0.0f;
                color.float32[1] = 0.0f;
                color.float32[2] = 0.0f;
                color.float32[3] = 0.0f;

                cb.clearColorImage(*temporal_accumulation.prev_image, vk::ImageLayout::eGeneral, color, subresource_range);
                cb.clearColorImage(*temporal_accumulation.current_moments_image[!common_resources->ping_pong], vk::ImageLayout::eGeneral, color, subresource_range);

                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.prev_image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.current_moments_image[!common_resources->ping_pong], vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));

                first_frame = false;
            }
        }

        void RayTracedShadows::Raytrace(CMShared<CommandBuffer> command_buffer)
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
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("shadows/shadows_ray_trace.comp")},
                };

                flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};
                variable_set_index_to_count[0] = max_variable_bindings;

                auto plci = PipelineLayoutCreateInfo
                {
                    shader_module_create_infos,
                    variable_set_index_to_count,
                };

                //
                CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
                CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
                CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

                ComputePipelineCreateInfo compute_pipeline_create_info{ pl };
                CMShared<ComputePipeline> cp = AssignOrPanic(c.Get(compute_pipeline_create_info));

                ray_trace.pipeline_layout = pl;
                ray_trace.pipeline = cp;
                c.SetName(ray_trace.pipeline_layout, "Ray Trace Pipeline Layout");
                c.SetName(ray_trace.pipeline, "Ray Trace Compute Pipeline");

                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});
                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);

                {
                    PanicIfError(ds->Bind(0, 0, common_resources->OutputBindings()));

                    PanicIfError(ds->Bind(1, 0, ray_trace.texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    
                    PanicIfError(ds->Bind(2, 0, common_resources->GetUBOBuffer()));
                    
                    PanicIfError(ds->Bind(3, 0, common_resources->g_buffer_pass->GetGBuffer1Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(3, 1, common_resources->g_buffer_pass->GetGBuffer2Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(3, 2, common_resources->g_buffer_pass->GetGBuffer3Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(3, 3, common_resources->g_buffer_pass->GetDepthTexture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    
                    PanicIfError(ds->Bind(4, 0, common_resources->blue_noise->GetSobolTexture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(4, 1, common_resources->blue_noise->GetSobolRankingTexture(BlueNoiseSpp::BLUE_NOISE_1SPP), vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    ShadowRayTracePushConstants push_constants;

                    push_constants.bias = ray_trace.bias;
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

        void RayTracedShadows::ResetArgs(CMShared<CommandBuffer> command_buffer)
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
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.shadow_tile_coords_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eIndirectCommandRead)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.shadow_dispatch_args_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect, vk::PipelineStageFlagBits::eComputeShader, {}, {}, buffer_barriers, {});
            }

            {
                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("shadows/shadows_denoise_reset_args.comp")},
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

                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});
                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);

                {
                    PanicIfError(ds->Bind(0, 1, temporal_accumulation.denoise_dispatch_args_buffer));
                    PanicIfError(ds->Bind(0, 3, temporal_accumulation.shadow_dispatch_args_buffer));
                    PanicIfError(render_frame.FlushPendingWrites(ds));
                }

                auto block_x = 1;
                auto block_y = 1;
                cb.dispatch(block_x, block_y, 1);
            }

            {
            }
        }

        void RayTracedShadows::TemporalAccumulation(CMShared<CommandBuffer> command_buffer)
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

                auto memory_barrier = vk::MemoryBarrier{}
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eUndefined)
                        .setNewLayout(vk::ImageLayout::eGeneral)
                        .setImage(**temporal_accumulation.current_output_image)
                        .setSubresourceRange(subresource_range)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eUndefined)
                        .setNewLayout(vk::ImageLayout::eGeneral)
                        .setImage(**temporal_accumulation.current_moments_image[common_resources->ping_pong])
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
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.shadow_tile_coords_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setBuffer(**temporal_accumulation.shadow_dispatch_args_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, memory_barrier, buffer_barriers, image_memory_barriers);
            }

            {
                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("shadows/shadows_denoise_reprojection.comp")},
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
                c.SetName(temporal_accumulation.pipeline_layout, "Shadows Upstage Pipeline Layout");
                c.SetName(temporal_accumulation.pipeline, "Shadows Upstage Compute Pipeline");

                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});
                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);

                {
                    PanicIfError(ds->Bind(0, 0, temporal_accumulation.current_output_texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(0, 1, temporal_accumulation.current_moments_texture[common_resources->ping_pong], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(1, 0, common_resources->g_buffer_pass->GetGBuffer1Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 1, common_resources->g_buffer_pass->GetGBuffer2Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 2, common_resources->g_buffer_pass->GetGBuffer3Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 3, common_resources->g_buffer_pass->GetDepthTexture(), vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(2, 0, common_resources->g_buffer_pass->GetPrevGBuffer1Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(2, 1, common_resources->g_buffer_pass->GetPrevGBuffer2Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(2, 2, common_resources->g_buffer_pass->GetPrevGBuffer3Texture(), vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(2, 3, common_resources->g_buffer_pass->GetPrevDepthTexture(), vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(3, 0, ray_trace.texture, vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(4, 0, temporal_accumulation.prev_texture, vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(4, 1, temporal_accumulation.current_moments_texture[!common_resources->ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(5, 0, common_resources->GetUBOBuffer()));

                    PanicIfError(ds->Bind(6, 0, temporal_accumulation.denoise_tile_coords_buffer));
                    PanicIfError(ds->Bind(6, 1, temporal_accumulation.denoise_dispatch_args_buffer));
                    PanicIfError(ds->Bind(6, 2, temporal_accumulation.shadow_tile_coords_buffer));
                    PanicIfError(ds->Bind(6, 3, temporal_accumulation.shadow_dispatch_args_buffer));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    ShadowTemporalAccumulationPushConstants push_constants;
                    push_constants.alpha = temporal_accumulation.alpha;
                    push_constants.moments_alpha = temporal_accumulation.moments_alpha;
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
                        .setImage(**temporal_accumulation.current_output_image)
                        .setSubresourceRange(subresource_range)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                    vk::ImageMemoryBarrier{}
                        .setOldLayout(vk::ImageLayout::eGeneral)
                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                        .setImage(**temporal_accumulation.current_moments_image[common_resources->ping_pong])
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
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                        .setBuffer(**temporal_accumulation.shadow_tile_coords_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                    vk::BufferMemoryBarrier{}
                        .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                        .setDstAccessMask(vk::AccessFlagBits::eIndirectCommandRead)
                        .setBuffer(**temporal_accumulation.shadow_dispatch_args_buffer)
                        .setOffset(0)
                        .setSize(VK_WHOLE_SIZE),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect, {}, memory_barrier, buffer_barriers, image_memory_barriers);
            }
        }

        void RayTracedShadows::ATrousFilter(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "A-Trous Filter");

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

            bool ping_pong = false;
            int32_t read_idx = 0;
            int32_t write_idx = 1;

            for (int i = 0; i < a_trous.filter_iterations; i++)
            {
                read_idx = (int32_t)ping_pong;
                write_idx = (int32_t)!ping_pong;

                if (i == 0)
                {
                    std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                        vk::ImageMemoryBarrier{}
                            .setOldLayout(vk::ImageLayout::eUndefined)
                            .setNewLayout(vk::ImageLayout::eGeneral)
                            .setImage(**a_trous.image[write_idx])
                            .setSubresourceRange(subresource_range)
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                    };

                    cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, {}, image_memory_barriers);
                }
                else
                {
                    std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                        vk::ImageMemoryBarrier{}
                            .setOldLayout(vk::ImageLayout::eGeneral)
                            .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                            .setImage(**a_trous.image[read_idx])
                            .setSubresourceRange(subresource_range)
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                        vk::ImageMemoryBarrier{}
                            .setOldLayout(vk::ImageLayout::eUndefined)
                            .setNewLayout(vk::ImageLayout::eGeneral)
                            .setImage(**a_trous.image[write_idx])
                            .setSubresourceRange(subresource_range)
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                    };

                    cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, image_memory_barriers);
                }

                {
                    vk::ClearColorValue color;

                    color.float32[0] = 1.0f;
                    color.float32[1] = 1.0f;
                    color.float32[2] = 1.0f;
                    color.float32[3] = 1.0f;

                    cb.clearColorImage(*a_trous.image[write_idx], vk::ImageLayout::eGeneral, color, subresource_range);
                }

                // Copy Shadow Tiles
                {
                    ScopedGPUProfileRaysterizer(command_buffer, "Copy Shadow Tiles");

                    auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                    {
                        ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("shadows/shadows_denoise_copy_shadow_tiles.comp")},
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

                    copy_shadow_tiles.pipeline_layout = pl;
                    copy_shadow_tiles.pipeline = cp;
                    c.SetName(copy_shadow_tiles.pipeline_layout, "Copy Shadow Tiles Pipeline Layout");
                    c.SetName(copy_shadow_tiles.pipeline, "Copy Shadow Tiles Compute Pipeline");

                    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});
                    cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);

                    {
                        PanicIfError(ds->Bind(0, 0, a_trous.texture[write_idx], vk::ImageLayout::eGeneral));
                        PanicIfError(ds->Bind(1, 2, temporal_accumulation.shadow_tile_coords_buffer));

                        PanicIfError(render_frame.FlushPendingWrites(ds));
                    }

                    cb.dispatchIndirect(**temporal_accumulation.shadow_dispatch_args_buffer, 0);
                }

                // A-Trous Filter
                {
                    ScopedGPUProfileRaysterizer(command_buffer, "A-Trous Filter Iteration");

                    auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                    {
                        ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("shadows/shadows_denoise_atrous.comp")},
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

                    a_trous.pipeline_layout = pl;
                    a_trous.pipeline = cp;
                    c.SetName(a_trous.pipeline_layout, "A-Trous Pipeline Layout");
                    c.SetName(a_trous.pipeline, "A-Trous Compute Pipeline");

                    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});
                    cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);

                    {
                        PanicIfError(ds->Bind(0, 0, a_trous.texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                        if (i == 0)
                        {
                            PanicIfError(ds->Bind(1, 0, temporal_accumulation.current_output_texture, vk::ImageLayout::eShaderReadOnlyOptimal));
                        }
                        else
                        {
                            PanicIfError(ds->Bind(1, 0, a_trous.texture[read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                        }

                        PanicIfError(ds->Bind(2, 0, common_resources->g_buffer_pass->OutputBindings()));

                        PanicIfError(ds->Bind(3, 0, temporal_accumulation.denoise_tile_coords_buffer));

                        PanicIfError(render_frame.FlushPendingWrites(ds));

                        ShadowATrousFilterPushConstants push_constants;

                        push_constants.radius = a_trous.radius;
                        push_constants.step_size = 1 << i;
                        push_constants.phi_visibility = a_trous.phi_visibility;
                        push_constants.phi_normal = a_trous.phi_normal;
                        push_constants.sigma_depth = a_trous.sigma_depth;
                        push_constants.g_buffer_mip = g_buffer_mip;
                        push_constants.power = i == (a_trous.filter_iterations - 1) ? a_trous.power : 0.0f;

                        PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                    }

                    cb.dispatchIndirect(**temporal_accumulation.denoise_dispatch_args_buffer, 0);
                }

                ping_pong = !ping_pong;

                if (a_trous.feedback_iteration == i)
                {
                    PanicIfError(command_buffer->SetImageLayout(a_trous.image[write_idx], vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal, subresource_range));
                    PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.prev_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferDstOptimal, subresource_range));

                    vk::ImageCopy image_copy_region{};
                    image_copy_region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    image_copy_region.srcSubresource.layerCount = 1;
                    image_copy_region.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                    image_copy_region.dstSubresource.layerCount = 1;
                    image_copy_region.extent.width = width;
                    image_copy_region.extent.height = height;
                    image_copy_region.extent.depth = 1;

                    cb.copyImage(*a_trous.image[write_idx], vk::ImageLayout::eTransferSrcOptimal, *temporal_accumulation.prev_image, vk::ImageLayout::eTransferDstOptimal, image_copy_region);

                    PanicIfError(command_buffer->SetImageLayout(a_trous.image[write_idx], vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral, subresource_range));
                    PanicIfError(command_buffer->SetImageLayout(temporal_accumulation.prev_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                }
            }

            a_trous.read_idx = write_idx;

            auto memory_barrier = vk::MemoryBarrier{}
                .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                vk::ImageMemoryBarrier{}
                    .setOldLayout(vk::ImageLayout::eGeneral)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setImage(**a_trous.image[write_idx])
                    .setSubresourceRange(subresource_range)
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
            };

            cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eFragmentShader, {}, memory_barrier, {}, image_memory_barriers);
        }

        void RayTracedShadows::Upsample(CMShared<CommandBuffer> command_buffer)
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
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("shadows/shadows_upsample.comp")},
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
                c.SetName(upsample.layout, "Shadows Upsample Pipeline Layout");
                c.SetName(upsample.pipeline, "Shadows Upsample Compute Pipeline");

                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});
                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);

                {
                    PanicIfError(ds->Bind(0, 0, upsample.texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(1, 0, a_trous.texture[a_trous.read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(ds->Bind(2, 0, common_resources->g_buffer_pass->OutputBindings()));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    ShadowUpsamplePushConstants push_constants;

                    push_constants.g_buffer_mip = g_buffer_mip;
                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                const int NUM_THREADS_X = 32;
                const int NUM_THREADS_Y = 32;

                auto block_x = static_cast<uint32_t>(ceil(float(upsample.image->image_create_info.image_create_info.extent.width) / float(NUM_THREADS_X)));
                auto block_y = static_cast<uint32_t>(ceil(float(upsample.image->image_create_info.image_create_info.extent.height) / float(NUM_THREADS_Y)));
                cb.dispatch(block_x, block_y, 1);
            }

            {
                PanicIfError(command_buffer->SetImageLayout(upsample.image, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
            }
        }

        std::vector<WriteDescriptorSetBindedResource> RayTracedShadows::OutputBindings() const
        {
            auto current_frame_index = c.GetFrameIndex();

            if (denoise)
            {
                if (current_output == OUTPUT_RAY_TRACE)
                {
                    return { ray_trace.texture };
                }
                else if (current_output == OUTPUT_TEMPORAL_ACCUMULATION)
                {
                    return { temporal_accumulation.current_output_texture };
                }
                else if (current_output == OUTPUT_ATROUS)
                {
                    return { a_trous.texture[a_trous.read_idx] };
                }
                else
                {
                    if (scale == RAY_TRACE_SCALE_FULL_RES)
                    {
                        return { a_trous.texture[a_trous.read_idx] };
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
