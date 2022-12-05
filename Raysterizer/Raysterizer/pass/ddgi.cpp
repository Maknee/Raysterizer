#include "ddgi.h"

#define PROBE_MIN_DIFFERENCE_DIVISION 50.0
#define PROBE_MAX_DIFFERENCE_DIVISION 1.0

namespace Raysterizer
{
	namespace Pass
	{
        // -----------------------------------------------------------------------------------------------------------------------------------

        struct DDGIUniforms
        {
            glm::vec3  grid_start_position;
            glm::vec3  grid_step;
            glm::ivec3 probe_counts;
            float      max_distance;
            float      depth_sharpness;
            float      hysteresis;
            float      normal_bias;
            float      energy_preservation;
            int        irradiance_probe_side_length;
            int        irradiance_texture_width;
            int        irradiance_texture_height;
            int        depth_probe_side_length;
            int        depth_texture_width;
            int        depth_texture_height;
            int        rays_per_probe;
            int        visibility_test;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct RayTracePushConstants
        {
            glm::mat4 random_orientation;
            uint32_t  num_frames;
            uint32_t  infinite_bounces;
            float     gi_intensity;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct ProbeUpdatePushConstants
        {
            uint32_t first_frame;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        struct SampleProbeGridPushConstants
        {
            int   g_buffer_mip;
            float gi_intensity;
        };

        // -----------------------------------------------------------------------------------------------------------------------------------

        Error DDGI::Setup(std::shared_ptr<CommonResources> common_resources_)
        {
            common_resources = common_resources_;

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            scale = common_resources->ddgi.scale;

            float scale_divisor = powf(2.0f, float(scale));

            width = common_resources->width / scale_divisor;
            height = common_resources->height / scale_divisor;

            g_buffer_mip = static_cast<uint32_t>(scale);

            random_device = std::make_unique<std::random_device>();
            random_generator = std::mt19937((*random_device)());
            random_distribution_zo = std::uniform_real_distribution<float>(0.0f, 1.0f);
            random_distribution_no = std::uniform_real_distribution<float>(-1.0f, 1.0f);

            last_scene_id = 0;

            return NoError();
        }

        void DDGI::Render(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "DDGI");

            if (last_scene_id != common_resources->ddgi.id)
            {
                InitializeProbeGrid();
            }

            UpdatePropertiesUBO();
            Raytrace(command_buffer);
            ProbeUpdate(command_buffer);
            SampleProbeGrid(command_buffer);

            first_frame = false;
            ping_pong = !ping_pong;
        }

        void DDGI::UpdateGui()
        {
            ImGui::Text("Grid Size: [%i, %i, %i]", probe_grid.probe_counts.x, probe_grid.probe_counts.y, probe_grid.probe_counts.z);
            ImGui::Text("Probe Count: %i", probe_grid.probe_counts.x * probe_grid.probe_counts.y * probe_grid.probe_counts.z);
            ImGui::Checkbox("Visibility Test", &probe_grid.visibility_test);
            ImGui::Checkbox("Infinite Bounces", &ray_trace.infinite_bounces);

            glm::vec3 min_extents = common_resources->ddgi.min_extents;
            glm::vec3 max_extents = common_resources->ddgi.max_extents;

            glm::vec3 scene_length = max_extents - min_extents;

            auto longest_length = std::max(scene_length.x, scene_length.z);
            auto min_probe_distance = longest_length / PROBE_MIN_DIFFERENCE_DIVISION;
            auto max_probe_distance = longest_length / PROBE_MAX_DIFFERENCE_DIVISION;

            if (ImGui::InputFloat3("Min extent", &common_resources->ddgi.min_extents[0]))
            {
                InitializeProbeGrid();
            }
            if (ImGui::InputFloat3("Max extent", &common_resources->ddgi.max_extents[0]))
            {
                InitializeProbeGrid();
            }

            if (ImGui::InputInt("Rays Per Probe", &ray_trace.rays_per_probe))
            {
                RecreateProbeGridResources();
            }
            if (ImGui::SliderFloat("Probe Distance", &probe_grid.probe_distance, min_probe_distance, max_probe_distance))
            {
                InitializeProbeGrid();
            }
            ImGui::InputFloat("Hysteresis", &probe_update.hysteresis);
            ImGui::SliderFloat("Infinite Bounce Intensity", &ray_trace.infinite_bounce_intensity, 0.0f, 10.0f);
            ImGui::SliderFloat("GI Intensity", &sample_probe_grid.gi_intensity, 0.0f, 10.0f);
            ImGui::SliderFloat("Normal Bias", &probe_update.normal_bias, 0.0f, 10.0f);
            ImGui::InputFloat("Depth Sharpness", &probe_update.depth_sharpness);
        }

        void DDGI::InitializeProbeGrid()
        {
            auto LimitExtent = [](auto& a, auto& b, bool greater)
            {
                if (greater)
                {
                    if (a > b)
                    {
                        a = b;
                    }
                }
                else
                {
                    if (a < b)
                    {
                        a = b;
                    }
                }
            };
            for (auto i = 0; i < common_resources->ddgi.min_extents.length(); i++)
            {
                auto& min = common_resources->ddgi.min_extents[i];
                auto& max = common_resources->ddgi.max_extents[i];
                LimitExtent(min, max, true);
                LimitExtent(max, min, false);
            }
            if (common_resources->ddgi.sample_from_vertices)
            {
                common_resources->ddgi.sample_from_vertices = false;

                if (common_resources->ddgi.min_extents == glm::vec3(std::numeric_limits<float>::max()))
                {
                    common_resources->ddgi.min_extents = glm::vec3(0);
                }
                if (common_resources->ddgi.max_extents == glm::vec3(std::numeric_limits<float>::min()))
                {
                    common_resources->ddgi.max_extents = glm::vec3(0);
                }

                // Get the min and max extents of the scene.
                glm::vec3 min_extents = common_resources->ddgi.min_extents;
                glm::vec3 max_extents = common_resources->ddgi.max_extents;

                // Compute the scene length.
                glm::vec3 scene_length = max_extents - min_extents;

                auto longest_length = std::max(scene_length.x, scene_length.z);
                auto probe_distance = longest_length / PROBE_MAX_DIFFERENCE_DIVISION;
                probe_grid.probe_distance = probe_distance;
            }

            // Get the min and max extents of the scene.
            glm::vec3 min_extents = common_resources->ddgi.min_extents;
            glm::vec3 max_extents = common_resources->ddgi.max_extents;

            // Compute the scene length.
            glm::vec3 scene_length = max_extents - min_extents;

            // Compute the number of probes along each axis.
            // Add 2 more probes to fully cover scene.
            probe_grid.probe_counts = glm::ivec3(scene_length / probe_grid.probe_distance) + glm::ivec3(2);
            probe_grid.grid_start_position = min_extents;
            probe_update.max_distance = probe_grid.probe_distance * 1.5f;

            // Assign current scene ID
            last_scene_id = common_resources->ddgi.id;

            RecreateProbeGridResources();
        }

        void DDGI::RecreateProbeGridResources()
        {
            first_frame = true;

            CreateImages();
            CreateBuffers();
        }

        void DDGI::CreateImages()
        {
            auto nearest_sampler = common_resources->nearest_sampler;
            auto num_frames = c.GetNumFrames();

            uint32_t total_probes = probe_grid.probe_counts.x * probe_grid.probe_counts.y * probe_grid.probe_counts.z;

            probe_grid.irradiance_image.clear();
            probe_grid.irradiance_view.clear();
            probe_grid.irradiance_texture.clear();

            probe_grid.depth_image.clear();
            probe_grid.depth_view.clear();
            probe_grid.depth_texture.clear();

            // Ray Trace
            {
                ray_trace.radiance_image = CommonResources::CreateImage(vk::ImageType::e2D, ray_trace.rays_per_probe, total_probes, 1, 1, 1,
                    vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                c.SetName(ray_trace.radiance_image, fmt::format("DDGI Ray Trace Radiance Image"));

                ray_trace.radiance_view = CommonResources::CreateImageView(ray_trace.radiance_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                c.SetName(ray_trace.radiance_view, fmt::format("DDGI Ray Trace Radiance Image View"));

                ray_trace.radiance_texture = std::make_shared<Texture>(ray_trace.radiance_image, ray_trace.radiance_view, nearest_sampler);

                ray_trace.direction_depth_image = CommonResources::CreateImage(vk::ImageType::e2D, ray_trace.rays_per_probe, total_probes, 1, 1, 1,
                    vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                c.SetName(ray_trace.direction_depth_image, fmt::format("DDGI Ray Trace Direction Depth Image"));

                ray_trace.direction_depth_view = CommonResources::CreateImageView(ray_trace.direction_depth_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                c.SetName(ray_trace.direction_depth_view, fmt::format("DDGI Ray Trace Direction Depth Image View"));

                ray_trace.direction_depth_texture = std::make_shared<Texture>(ray_trace.direction_depth_image, ray_trace.direction_depth_view, nearest_sampler);
            }

            // Probe Grid
            {
                // 1-pixel of padding surrounding each probe, 1-pixel padding surrounding entire texture for alignment.
                const int irradiance_width = (probe_grid.irradiance_oct_size + 2) * probe_grid.probe_counts.x * probe_grid.probe_counts.y + 2;
                const int irradiance_height = (probe_grid.irradiance_oct_size + 2) * probe_grid.probe_counts.z + 2;

                const int depth_width = (probe_grid.depth_oct_size + 2) * probe_grid.probe_counts.x * probe_grid.probe_counts.y + 2;
                const int depth_height = (probe_grid.depth_oct_size + 2) * probe_grid.probe_counts.z + 2;

                for (int i = 0; i < common_resources->ping_pong_size; i++)
                {
                    auto irradiance_image = CommonResources::CreateImage(vk::ImageType::e2D, irradiance_width, irradiance_height, 1, 1, 1,
                        vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(irradiance_image, fmt::format("DDGI Irradiance Probe Grid Image {}", i));

                    auto irradiance_view = CommonResources::CreateImageView(irradiance_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(irradiance_view, fmt::format("DDGI Irradiance Probe Grid Image View {}", i));

                    auto irradiance_texture = std::make_shared<Texture>(irradiance_image, irradiance_view, nearest_sampler);

                    probe_grid.irradiance_image.emplace_back(irradiance_image);
                    probe_grid.irradiance_view.emplace_back(irradiance_view);
                    probe_grid.irradiance_texture.emplace_back(irradiance_texture);

                    auto depth_image = CommonResources::CreateImage(vk::ImageType::e2D, depth_width, depth_height, 1, 1, 1,
                        vk::Format::eR16G16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                    c.SetName(depth_image, fmt::format("DDGI Depth Probe Grid Image {}", i));

                    auto depth_view = CommonResources::CreateImageView(depth_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                    c.SetName(depth_view, fmt::format("DDGI Depth Probe Grid Image View {}", i));

                    auto depth_texture = std::make_shared<Texture>(depth_image, depth_view, nearest_sampler);

                    probe_grid.depth_image.emplace_back(depth_image);
                    probe_grid.depth_view.emplace_back(depth_view);
                    probe_grid.depth_texture.emplace_back(depth_texture);
                }
            }

            // Sample Probe Grid
            {
                sample_probe_grid.image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, 1, 1,
                    vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::SampleCountFlagBits::e1);
                c.SetName(sample_probe_grid.image, fmt::format("DDGI Sample Probe Grid Image"));

                sample_probe_grid.image_view = CommonResources::CreateImageView(sample_probe_grid.image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
                c.SetName(sample_probe_grid.image_view, fmt::format("DDGI Sample Probe Grid View"));

                sample_probe_grid.texture = std::make_shared<Texture>(sample_probe_grid.image, sample_probe_grid.image_view, nearest_sampler);
            }
        }

        void DDGI::CreateBuffers()
        {
            RenderFrame& render_frame = c.GetRenderFrame();
            auto num_frames = c.GetNumFrames();

            probe_grid.properties_ubo.clear();

            probe_grid.properties_ubo_size = sizeof(DDGIUniforms);
            for (auto i = 0; i < num_frames; i++)
            {
                auto properties_ubo = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, probe_grid.properties_ubo_size, true));
                c.SetName(properties_ubo, fmt::format("Probe grid buffer {}", i));

                probe_grid.properties_ubo.emplace_back(properties_ubo);
            }
        }
        
        void DDGI::UpdatePropertiesUBO()
        {
            auto current_frame_index = c.GetFrameIndex();

            DDGIUniforms ubo;

            ubo.grid_start_position = probe_grid.grid_start_position;
            ubo.grid_step = glm::vec3(probe_grid.probe_distance);
            ubo.probe_counts = probe_grid.probe_counts;
            ubo.max_distance = probe_update.max_distance;
            ubo.depth_sharpness = probe_update.depth_sharpness;
            ubo.hysteresis = probe_update.hysteresis;
            ubo.normal_bias = probe_update.normal_bias;
            ubo.energy_preservation = probe_grid.recursive_energy_preservation;
            ubo.irradiance_probe_side_length = probe_grid.irradiance_oct_size;
            ubo.irradiance_texture_width = probe_grid.irradiance_image[0]->image_create_info.image_create_info.extent.width;
            ubo.irradiance_texture_height = probe_grid.irradiance_image[0]->image_create_info.image_create_info.extent.height;
            ubo.depth_probe_side_length = probe_grid.depth_oct_size;
            ubo.depth_texture_width = probe_grid.depth_image[0]->image_create_info.image_create_info.extent.width;
            ubo.depth_texture_height = probe_grid.depth_image[0]->image_create_info.image_create_info.extent.height;
            ubo.rays_per_probe = ray_trace.rays_per_probe;
            ubo.visibility_test = (int32_t)probe_grid.visibility_test;

            PanicIfError(probe_grid.properties_ubo[current_frame_index]->Copy(PointerView(ubo)));
        }
        
        void DDGI::Raytrace(CMShared<CommandBuffer> command_buffer)
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

            uint32_t read_idx = static_cast<uint32_t>(!ping_pong);

            if (first_frame)
            {
                PanicIfError(command_buffer->SetImageLayout(probe_grid.irradiance_image[read_idx], vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
                PanicIfError(command_buffer->SetImageLayout(probe_grid.depth_image[read_idx], vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
            }

            {
                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eUndefined)
                       .setNewLayout(vk::ImageLayout::eGeneral)
                       .setImage(**ray_trace.radiance_image)
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eUndefined)
                       .setNewLayout(vk::ImageLayout::eGeneral)
                       .setImage(**ray_trace.direction_depth_image)
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eRayTracingShaderKHR, {}, {}, {}, image_memory_barriers);
            }

            // Ray Trace
            {
                const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_ray_trace.rgen")},
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_ray_trace.rmiss")},
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_ray_trace.rchit")},
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

                auto recursion_depth = 1;
                RaytracingPipelineCreateInfo raytracing_pipeline_create_info{ pl, recursion_depth };
                CMShared<RaytracingPipeline> rp = AssignOrPanic(c.Get(raytracing_pipeline_create_info));

                ray_trace.pipeline_layout = pl;
                ray_trace.pipeline = rp;
                c.SetName(ray_trace.pipeline_layout, "Ray Trace Pipeline Layout");
                c.SetName(ray_trace.pipeline, "Ray Trace Compute Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, common_resources->OutputBindings()));

                    PanicIfError(ds->Bind(1, 0, ray_trace.radiance_texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(1, 1, ray_trace.direction_depth_texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(2, 0, common_resources->GetUBOBuffer()));

                    PanicIfError(ds->Bind(3, 0, common_resources->sky_environment->OutputBindings()));

                    PanicIfError(ds->Bind(4, 0, probe_grid.irradiance_texture[!ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(4, 1, probe_grid.depth_texture[!ping_pong], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(4, 2, probe_grid.properties_ubo[current_frame_index]));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    RayTracePushConstants push_constants;

                    push_constants.random_orientation = glm::mat4_cast(glm::angleAxis(random_distribution_zo(random_generator) * (float(glm::pi<float>()) * 2.0f), glm::normalize(glm::vec3(random_distribution_no(random_generator), random_distribution_no(random_generator), random_distribution_no(random_generator)))));
                    push_constants.num_frames = current_frame;
                    push_constants.infinite_bounces = ray_trace.infinite_bounces && !first_frame ? 1u : 0u;
                    push_constants.gi_intensity = ray_trace.infinite_bounce_intensity;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                auto& sbt = ray_trace.sbt;
                auto& sbt_buffer = ray_trace.sbt_buffer;

                if (!sbt_buffer)
                {
                    auto properties = c.GetPhysicalDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR, vk::PhysicalDeviceAccelerationStructurePropertiesKHR>();
                    auto raytracing_properties = properties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

                    sbt.Reset();
                    sbt.AddRayGenProgram(0);
                    sbt.AddMissProgram(1);
                    sbt.AddHitGroup(2);

                    sbt_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eUniformBuffer, 1, true));
                    auto sbt_data = sbt.GetHandleBytes(c.GetDevice(), raytracing_properties, **rp);
                    if (sbt_buffer->GetSize() < sbt_data.size())
                    {
                        sbt_buffer = AssignOrPanic(render_frame.ResizeBuffer(sbt_buffer, sbt_data.size()));
                    }
                    PanicIfError(sbt_buffer->Copy(PointerView(sbt_data)));
                }

                vk::StridedDeviceAddressRegionKHR raygen_sbt{};
                raygen_sbt.setDeviceAddress(sbt_buffer->GetAddress() + sbt.GetRaygenOffset());
                raygen_sbt.setStride(sbt.GetRaygenStride());
                raygen_sbt.setSize(sbt.GetRaygenSize());

                vk::StridedDeviceAddressRegionKHR raymiss_sbt{};
                raymiss_sbt.setDeviceAddress(sbt_buffer->GetAddress() + sbt.GetMissOffset());
                raymiss_sbt.setStride(sbt.GetMissStride());
                raymiss_sbt.setSize(sbt.GetMissSize());

                vk::StridedDeviceAddressRegionKHR rayhit_sbt{};
                rayhit_sbt.setDeviceAddress(sbt_buffer->GetAddress() + sbt.GetHitGroupOffset());
                rayhit_sbt.setStride(sbt.GetHitGroupStride());
                rayhit_sbt.setSize(sbt.GetHitGroupSize());

                vk::StridedDeviceAddressRegionKHR callable_sbt{};
                callable_sbt.setDeviceAddress({});
                callable_sbt.setStride({});
                callable_sbt.setSize({});

                uint32_t num_total_probes = probe_grid.probe_counts.x * probe_grid.probe_counts.y * probe_grid.probe_counts.z;

                cb.traceRaysKHR(raygen_sbt, raymiss_sbt, rayhit_sbt, callable_sbt, ray_trace.rays_per_probe, num_total_probes, 1);
            }

            {
                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eGeneral)
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                       .setImage(**ray_trace.radiance_image)
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eGeneral)
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                       .setImage(**ray_trace.direction_depth_image)
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eRayTracingShaderKHR, {}, {}, {}, image_memory_barriers);
            }

        }

        void DDGI::ProbeUpdate(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Probe Update");

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

            uint32_t write_idx = static_cast<uint32_t>(ping_pong);

            {
                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eUndefined)
                       .setNewLayout(vk::ImageLayout::eGeneral)
                       .setImage(**probe_grid.irradiance_image[write_idx])
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eUndefined)
                       .setNewLayout(vk::ImageLayout::eGeneral)
                       .setImage(**probe_grid.depth_image[write_idx])
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eComputeShader, {}, {}, {}, image_memory_barriers);
            }

            {
                ProbeUpdate2(command_buffer, true);
                ProbeUpdate2(command_buffer, false);
            }

            {
                auto memory_barrier = vk::MemoryBarrier{}
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite);

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, memory_barrier, {}, {});
            }

            {
                BorderUpdateCmd(command_buffer);
            }

            {
                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eGeneral)
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                       .setImage(**probe_grid.irradiance_image[write_idx])
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eGeneral)
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                       .setImage(**probe_grid.depth_image[write_idx])
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR, {}, {}, {}, image_memory_barriers);
            }
        }
        
        void DDGI::ProbeUpdate2(CMShared<CommandBuffer> command_buffer, bool is_irradiance)
        {
            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            uint32_t read_idx = static_cast<uint32_t>(!ping_pong);
            uint32_t write_idx = static_cast<uint32_t>(ping_pong);

            auto subresource_range = vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            if (is_irradiance)
            {
                ScopedGPUProfileRaysterizer(command_buffer, "Irradiance");

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_irradiance_probe_update.comp")},
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

                probe_update.irradiance_pipeline_layout = pl;
                probe_update.irradiance_pipeline = cp;
                c.SetName(probe_update.irradiance_pipeline_layout, "GI Irradiance Probe Update Pipeline Layout");
                c.SetName(probe_update.irradiance_pipeline, "GI Irradiance Probe Update Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, probe_grid.irradiance_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(0, 1, probe_grid.depth_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(1, 0, probe_grid.irradiance_texture[read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 1, probe_grid.depth_texture[read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 2, probe_grid.properties_ubo[current_frame_index]));

                    PanicIfError(ds->Bind(2, 0, ray_trace.radiance_texture, vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(2, 1, ray_trace.direction_depth_texture, vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    ProbeUpdatePushConstants push_constants;
                    push_constants.first_frame = (uint32_t)first_frame;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                const uint32_t dispatch_x = static_cast<uint32_t>(probe_grid.probe_counts.x * probe_grid.probe_counts.y);
                const uint32_t dispatch_y = static_cast<uint32_t>(probe_grid.probe_counts.z);

                cb.dispatch(dispatch_x, dispatch_y, 1);
            }
            else
            {
                ScopedGPUProfileRaysterizer(command_buffer, "Depth");

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_depth_probe_update.comp")},
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

                probe_update.depth_pipeline_layout = pl;
                probe_update.depth_pipeline = cp;
                c.SetName(probe_update.depth_pipeline_layout, "GI Depth Probe Update Pipeline Layout");
                c.SetName(probe_update.depth_pipeline, "GI Depth Probe Update Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, probe_grid.irradiance_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(0, 1, probe_grid.depth_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(1, 0, probe_grid.irradiance_texture[read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 1, probe_grid.depth_texture[read_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 2, probe_grid.properties_ubo[current_frame_index]));

                    PanicIfError(ds->Bind(2, 0, ray_trace.radiance_texture, vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(2, 1, ray_trace.direction_depth_texture, vk::ImageLayout::eShaderReadOnlyOptimal));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    ProbeUpdatePushConstants push_constants;
                    push_constants.first_frame = (uint32_t)first_frame;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                const uint32_t dispatch_x = static_cast<uint32_t>(probe_grid.probe_counts.x * probe_grid.probe_counts.y);
                const uint32_t dispatch_y = static_cast<uint32_t>(probe_grid.probe_counts.z);

                cb.dispatch(dispatch_x, dispatch_y, 1);
            }
        }

        void DDGI::BorderUpdateCmd(CMShared<CommandBuffer> command_buffer)
        {
            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            uint32_t read_idx = static_cast<uint32_t>(!ping_pong);
            uint32_t write_idx = static_cast<uint32_t>(ping_pong);

            auto subresource_range = vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            {
                ScopedGPUProfileRaysterizer(command_buffer, "Irradiance");

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_irradiance_border_update.comp")},
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

                border_update.irradiance_pipeline_layout = pl;
                border_update.irradiance_pipeline = cp;
                c.SetName(border_update.irradiance_pipeline_layout, "Irradiance Border Update Pipeline Layout");
                c.SetName(border_update.irradiance_pipeline, "Irradiance Border Update Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, probe_grid.irradiance_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(0, 1, probe_grid.depth_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(render_frame.FlushPendingWrites(ds));
                }


                const uint32_t dispatch_x = static_cast<uint32_t>(probe_grid.probe_counts.x * probe_grid.probe_counts.y);
                const uint32_t dispatch_y = static_cast<uint32_t>(probe_grid.probe_counts.z);

                cb.dispatch(dispatch_x, dispatch_y, 1);
            }

            {
                ScopedGPUProfileRaysterizer(command_buffer, "Depth");

                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_depth_border_update.comp")},
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

                border_update.depth_pipeline_layout = pl;
                border_update.depth_pipeline = cp;
                c.SetName(border_update.depth_pipeline_layout, "Depth Border Update Pipeline Layout");
                c.SetName(border_update.depth_pipeline, "Depth Border Update Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, probe_grid.irradiance_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
                    PanicIfError(ds->Bind(0, 1, probe_grid.depth_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(render_frame.FlushPendingWrites(ds));
                }

                const uint32_t dispatch_x = static_cast<uint32_t>(probe_grid.probe_counts.x * probe_grid.probe_counts.y);
                const uint32_t dispatch_y = static_cast<uint32_t>(probe_grid.probe_counts.z);

                cb.dispatch(dispatch_x, dispatch_y, 1);
            }
        }

        void DDGI::SampleProbeGrid(CMShared<CommandBuffer> command_buffer)
        {
            ScopedGPUProfileRaysterizer(command_buffer, "Sample Probe Grid");

            vk::Device device = c.GetDevice();
            RenderFrame& render_frame = c.GetRenderFrame();

            auto current_frame = c.GetFrame();
            auto current_frame_index = c.GetFrameIndex();
            auto num_frames = c.GetNumFrames();

            auto& cb = **command_buffer;
            CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

            uint32_t read_idx = static_cast<uint32_t>(!ping_pong);
            uint32_t write_idx = static_cast<uint32_t>(ping_pong);

            auto subresource_range = vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1);

            {
                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eUndefined)
                       .setNewLayout(vk::ImageLayout::eGeneral)
                       .setImage(**sample_probe_grid.image)
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, {}, image_memory_barriers);
            }

            {
                auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
                {
                    ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("gi/gi_sample_probe_grid.comp")},
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

                sample_probe_grid.pipeline_layout = pl;
                sample_probe_grid.pipeline = cp;
                c.SetName(sample_probe_grid.pipeline_layout, "GI Sample Probe Grid Pipeline Layout");
                c.SetName(sample_probe_grid.pipeline, "GI Sample Probe Grid Pipeline");

                cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
                cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

                {
                    PanicIfError(ds->Bind(0, 0, sample_probe_grid.texture, vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

                    PanicIfError(ds->Bind(1, 0, probe_grid.irradiance_texture[write_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 1, probe_grid.depth_texture[write_idx], vk::ImageLayout::eShaderReadOnlyOptimal));
                    PanicIfError(ds->Bind(1, 2, probe_grid.properties_ubo[current_frame_index]));

                    PanicIfError(ds->Bind(2, 0, common_resources->g_buffer_pass->OutputBindings()));

                    PanicIfError(ds->Bind(3, 0, common_resources->GetUBOBuffer()));

                    PanicIfError(render_frame.FlushPendingWrites(ds));

                    SampleProbeGridPushConstants push_constants;
                    push_constants.g_buffer_mip = g_buffer_mip;
                    push_constants.gi_intensity = sample_probe_grid.gi_intensity;

                    PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
                }

                const int NUM_THREADS_X = 32;
                const int NUM_THREADS_Y = 32;

                auto dispatch_x = static_cast<uint32_t>(ceil(float(sample_probe_grid.image->image_create_info.image_create_info.extent.width) / float(NUM_THREADS_X)));
                auto dispatch_y = static_cast<uint32_t>(ceil(float(sample_probe_grid.image->image_create_info.image_create_info.extent.height) / float(NUM_THREADS_Y)));

                cb.dispatch(dispatch_x, dispatch_y, 1);
            }

            {
                std::vector<vk::ImageMemoryBarrier> image_memory_barriers = {
                   vk::ImageMemoryBarrier{}
                       .setOldLayout(vk::ImageLayout::eGeneral)
                       .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                       .setImage(**sample_probe_grid.image)
                       .setSubresourceRange(subresource_range)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                       .setSrcAccessMask(vk::AccessFlagBits::eShaderRead),
                };

                cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, image_memory_barriers);
            }
        }

        std::vector<WriteDescriptorSetBindedResource> DDGI::OutputBindings() const
        {
            return { sample_probe_grid.texture };
        }

        std::vector<WriteDescriptorSetBindedResource> DDGI::CurrentReadBindings() const
        {
            auto current_frame_index = c.GetFrameIndex();
            return { probe_grid.irradiance_texture[!ping_pong], probe_grid.depth_texture[!ping_pong], probe_grid.properties_ubo[current_frame_index]};
        }
    }
}
