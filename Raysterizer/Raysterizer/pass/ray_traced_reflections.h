#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
        class RayTracedReflections
        {
        public:
            enum OutputType
            {
                OUTPUT_RAY_TRACE,
                OUTPUT_TEMPORAL_ACCUMULATION,
                OUTPUT_ATROUS,
                OUTPUT_UPSAMPLE
            };

            const static int         kNumOutputTypes = 4;
            const static OutputType  kOutputTypeEnums[];
            const static std::string kOutputTypeNames[];

        public:
            explicit RayTracedReflections() {}
            Error Setup(std::shared_ptr<CommonResources> common_resources_);
            void Render(CMShared<CommandBuffer> command_buffer);
            void UpdateGui();
            void SetOutputType(OutputType output_type) { current_output = output_type; }
            OutputType GetOutputType() const { return current_output; }

            void SetRoughDDGIIntensity(float rough_ddgi_intensity) { ray_trace.rough_ddgi_intensity = rough_ddgi_intensity; }

        private:
            void ClearImage(CMShared<CommandBuffer> command_buffer);
            void Raytrace(CMShared<CommandBuffer> command_buffer);
            void ResetArgs(CMShared<CommandBuffer> command_buffer);
            void TemporalAccumulation(CMShared<CommandBuffer> command_buffer);
            void ATrousFilter(CMShared<CommandBuffer> command_buffer);
            void Upsample(CMShared<CommandBuffer> command_buffer);

        public:
            std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;

        private:
            struct RayTrace
            {
                bool                            sample_gi = true;
                bool                            approximate_with_ddgi = true;
                float                           gi_intensity = 0.5f;
                float                           rough_ddgi_intensity = 0.5f;
                float                           ibl_indirect_specular_intensity = 0.05f;
                float                           bias = 0.5f;
                float                           trim = 0.8f;
                CMShared<DescriptorSet>      write_ds;
                CMShared<DescriptorSet>      read_ds;
                CMShared<RaytracingPipeline> pipeline;
                CMShared<PipelineLayout>     pipeline_layout;
                CMShared<Image>              image;
                CMShared<ImageView>          view;
                CMShared<Texture>          texture;
                CMShared<Buffer> sbt_buffer;
                ShaderBindingTable  sbt;
            };

            struct ResetArgsStruct
            {
                CMShared<PipelineLayout>      pipeline_layout;
                CMShared<ComputePipeline>     pipeline;
            };

            struct TemporalAccumulationStruct
            {
                float                            alpha = 0.01f;
                float                            moments_alpha = 0.2f;
                bool                             blur_as_input = false;
                CMShared<Buffer>              denoise_tile_coords_buffer;
                CMShared<Buffer>              denoise_dispatch_args_buffer;
                CMShared<Buffer>              shadow_tile_coords_buffer;
                CMShared<Buffer>              shadow_dispatch_args_buffer;
                CMShared<ComputePipeline>     pipeline;
                CMShared<PipelineLayout>      pipeline_layout;
                CMShared<Image>               prev_image;
                CMShared<ImageView>           prev_view;
                CMShared<Texture>           prev_texture;
                std::vector<CMShared<Image>>               current_output_image;
                std::vector<CMShared<ImageView>>           current_output_view;
                std::vector<CMShared<Texture>>           current_output_texture;
                std::vector<CMShared<Image>>               current_moments_image;
                std::vector<CMShared<ImageView>>           current_moments_view;
                std::vector<CMShared<Texture>>           current_moments_texture;
                CMShared<DescriptorSet>       current_write_ds[2];
                CMShared<DescriptorSet>       current_read_ds[2];
                CMShared<DescriptorSet>       output_only_read_ds;
                CMShared<DescriptorSet>       prev_read_ds[2];
                CMShared<DescriptorSet>       indirect_buffer_ds;
            };

            struct CopyShadowTiles
            {
                CMShared<PipelineLayout>  pipeline_layout;
                CMShared<ComputePipeline> pipeline;
            };

            struct ATrous
            {
                float                        phi_color = 10.0f;
                float                        phi_normal = 32.0f;
                float                        sigma_depth = 1.0f;
                int32_t                      radius = 1;
                int32_t                      filter_iterations = 4;
                int32_t                      feedback_iteration = 1;
                int32_t                      read_idx = 0;
                CMShared<ComputePipeline> pipeline;
                CMShared<PipelineLayout>  pipeline_layout;
                std::vector<CMShared<Image>>           image;
                std::vector<CMShared<ImageView>>       view;
                std::vector<CMShared<Texture>>         texture;
                CMShared<DescriptorSet>   read_ds[2];
                CMShared<DescriptorSet>   write_ds[2];
            };

            struct UpsampleStruct
            {
                CMShared<PipelineLayout>  layout;
                CMShared<ComputePipeline> pipeline;
                CMShared<Image>           image;
                CMShared<ImageView>       image_view;
                CMShared<Texture>         texture;
                CMShared<DescriptorSet>   read_ds;
                CMShared<DescriptorSet>   write_ds;
            };

            std::shared_ptr<CommonResources> common_resources;
            RayTraceScale                  scale;
            OutputType                     current_output = OUTPUT_UPSAMPLE;
            uint32_t                       g_buffer_mip = 0;
            uint32_t                       width;
            uint32_t                       height;
            bool                           denoise = true;
            bool                           first_frame = true;
            RayTrace                       ray_trace;
            ResetArgsStruct                      reset_args;
            TemporalAccumulationStruct           temporal_accumulation;
            CopyShadowTiles                copy_shadow_tiles;
            ATrous                         a_trous;
            UpsampleStruct                       upsample;
        };
	}
}