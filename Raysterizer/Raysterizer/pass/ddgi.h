#pragma once

#include "pch.h"

namespace Raysterizer
{
    namespace Pass
    {
        class DDGI
        {
        public:
            explicit DDGI() {}
            Error Setup(std::shared_ptr<CommonResources> common_resources_);
            void Render(CMShared<CommandBuffer> command_buffer);
            void UpdateGui();
            void SetProbeDistance(float probe_distance) { probe_grid.probe_distance = probe_distance; }
            auto GetProbeCounts() const { return probe_grid.probe_counts; }

        private:
            void InitializeProbeGrid();
            void RecreateProbeGridResources();
            void CreateImages();
            void CreateBuffers();
            void UpdatePropertiesUBO();
            void Raytrace(CMShared<CommandBuffer> command_buffer);
            void ProbeUpdate(CMShared<CommandBuffer> command_buffer);
            void ProbeUpdate2(CMShared<CommandBuffer> command_buffer, bool is_irradiance);
            void BorderUpdateCmd(CMShared<CommandBuffer> command_buffer);
            void SampleProbeGrid(CMShared<CommandBuffer> command_buffer);

        public:
            std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;
            std::vector<WriteDescriptorSetBindedResource> CurrentReadBindings() const;

        private:
            struct RayTrace
            {
                bool                             infinite_bounces = true;
                float                            infinite_bounce_intensity = 1.7f;
                int32_t                          rays_per_probe = 256;
                CMShared<DescriptorSet>       write_ds;
                CMShared<DescriptorSet>       read_ds;
                CMShared<DescriptorSetLayout> write_ds_layout;
                CMShared<DescriptorSetLayout> read_ds_layout;
                CMShared<RaytracingPipeline>  pipeline;
                CMShared<PipelineLayout>      pipeline_layout;
                CMShared<Image>               radiance_image;
                CMShared<ImageView>           radiance_view;
                CMShared<Texture>           radiance_texture;
                CMShared<Image>               direction_depth_image;
                CMShared<ImageView>           direction_depth_view;
                CMShared<Texture>           direction_depth_texture;
                CMShared<Buffer> sbt_buffer;
                ShaderBindingTable  sbt;
            };

            struct ProbeGrid
            {
                bool                             visibility_test = true;
                float                            probe_distance = 4.0f;
                float                            recursive_energy_preservation = 0.85f;
                uint32_t                         irradiance_oct_size = 8;
                uint32_t                         depth_oct_size = 16;
                glm::vec3                        grid_start_position;
                glm::ivec3                       probe_counts;
                CMShared<DescriptorSet>       write_ds[2];
                CMShared<DescriptorSet>       read_ds[2];
                CMShared<DescriptorSetLayout> write_ds_layout;

                std::vector<CMShared<Image>>               irradiance_image;
                std::vector<CMShared<ImageView>>           irradiance_view;
                std::vector<CMShared<Texture>>           irradiance_texture;

                std::vector<CMShared<Image>>               depth_image;
                std::vector<CMShared<ImageView>>           depth_view;
                std::vector<CMShared<Texture>>           depth_texture;

                std::vector<CMShared<Buffer>>              properties_ubo;
                size_t                           properties_ubo_size;
            };

            struct ProbeUpdateStruct
            {
                float                        hysteresis = 0.98f;
                float                        depth_sharpness = 50.0f;
                float                        max_distance = 6.0f;
                float                        normal_bias = 1.00f;
                CMShared<ComputePipeline> irradiance_pipeline;
                CMShared<ComputePipeline> depth_pipeline;
                CMShared<PipelineLayout>  irradiance_pipeline_layout;
                CMShared<PipelineLayout>  depth_pipeline_layout;
            };

            struct SampleProbeGridStruct
            {
                float                        gi_intensity = 1.0f;
                CMShared<Image>           image;
                CMShared<ImageView>       image_view;
                CMShared<Texture>       texture;
                CMShared<ComputePipeline> pipeline;
                CMShared<PipelineLayout>  pipeline_layout;
                CMShared<DescriptorSet>   write_ds;
                CMShared<DescriptorSet>   read_ds;
            };

            struct BorderUpdate
            {
                CMShared<ComputePipeline> irradiance_pipeline;
                CMShared<ComputePipeline> depth_pipeline;
                CMShared<PipelineLayout>  irradiance_pipeline_layout;
                CMShared<PipelineLayout>  depth_pipeline_layout;
            };

            uint32_t                              last_scene_id = UINT32_MAX;
            CMShared<CommonResources> common_resources;
            RayTraceScale                         scale;
            uint32_t                              g_buffer_mip = 0;
            uint32_t                              width;
            uint32_t                              height;
            bool                                  first_frame = true;
            bool                                  ping_pong = false;
            std::unique_ptr<std::random_device>                    random_device;
            std::mt19937                          random_generator;
            std::uniform_real_distribution<float> random_distribution_zo;
            std::uniform_real_distribution<float> random_distribution_no;
            RayTrace                              ray_trace;
            ProbeGrid                             probe_grid;
            ProbeUpdateStruct                           probe_update;
            BorderUpdate                          border_update;
            SampleProbeGridStruct                       sample_probe_grid;
        };
    }
}