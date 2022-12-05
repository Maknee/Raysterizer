#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
        class RayTracedAO
        {
        public:
            enum OutputType
            {
                OUTPUT_RAY_TRACE,
                OUTPUT_TEMPORAL_ACCUMULATION,
                OUTPUT_BILATERAL_BLUR,
                OUTPUT_UPSAMPLE
            };

            const static int         kNumOutputTypes = 4;
            const static OutputType  kOutputTypeEnums[];
            const static std::string kOutputTypeNames[];

        public:
            explicit RayTracedAO() {}
            Error Setup(std::shared_ptr<CommonResources> common_resources_);
            void Render(CMShared<CommandBuffer> command_buffer);
            void UpdateGui();
            void SetOutputType(OutputType output_type) { current_output = output_type; }
            OutputType GetOutputType() const { return current_output; }

            void EnableBilateralBlur(bool enable) { bilateral_blur.enabled = enable; }

        private:
            void ClearImage(CMShared<CommandBuffer> command_buffer);
            void Raytrace(CMShared<CommandBuffer> command_buffer);
            void Denoise(CMShared<CommandBuffer> command_buffer);
            void ResetArgs(CMShared<CommandBuffer> command_buffer);
            void TemporalAccumulation(CMShared<CommandBuffer> command_buffer);
            void BilateralBlur(CMShared<CommandBuffer> command_buffer);
            void Upsample(CMShared<CommandBuffer> command_buffer);

        public:
            std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;

        private:
            struct RayTrace
            {
                float                        ray_length = 7.0f;
                float                        bias = 0.3f;
                CMShared<ComputePipeline> pipeline;
                CMShared<PipelineLayout>  pipeline_layout;
                CMShared<Image>           image;
                CMShared<ImageView>       view;
                CMShared<Texture> texture;
                CMShared<DescriptorSet>   write_ds;
                CMShared<DescriptorSet>   read_ds;
                CMShared<DescriptorSet>   bilinear_read_ds;
            };

            struct ResetArgsStruct
            {
                CMShared<PipelineLayout>  pipeline_layout;
                CMShared<ComputePipeline> pipeline;
            };

            struct TemporalAccumulationStruct
            {
                float                            alpha = 0.01f;
                float                            moments_alpha = 0.2f;
                bool                             blur_as_input = false;
                CMShared<Buffer>              denoise_tile_coords_buffer;
                CMShared<Buffer>              denoise_dispatch_args_buffer;
                CMShared<ComputePipeline>     pipeline;
                CMShared<PipelineLayout>      pipeline_layout;
                CMShared<DescriptorSetLayout> read_ds_layout;
                CMShared<DescriptorSetLayout> write_ds_layout;
                CMShared<DescriptorSetLayout> indirect_buffer_ds_layout;
                std::vector<CMShared<Image>>               color_image;
                std::vector<CMShared<ImageView>>           color_view;
                std::vector<CMShared<Texture>>           color_texture;
                std::vector<CMShared<Image>>               history_length_image;
                std::vector<CMShared<ImageView>>           history_length_view;
                std::vector<CMShared<Texture>>           history_length_texture;
                CMShared<DescriptorSet>       write_ds[2];
                CMShared<DescriptorSet>       read_ds[2];
                CMShared<DescriptorSet>       output_read_ds[2];
                CMShared<DescriptorSet>       indirect_buffer_ds;
            };

            struct BilateralBlurStruct
            {
                bool enabled = true;
                int32_t                      blur_radius = 4;
                CMShared<PipelineLayout>  layout;
                CMShared<ComputePipeline> pipeline;
                std::vector<CMShared<Image>>               image;
                std::vector<CMShared<ImageView>>           image_view;
                std::vector<CMShared<Texture>>           texture;
                CMShared<DescriptorSet>   read_ds[2];
                CMShared<DescriptorSet>   write_ds[2];
            };

            struct UpsampleStruct
            {
                float                        power = 1.2f;
                CMShared<PipelineLayout>  layout;
                CMShared<ComputePipeline> pipeline;
                CMShared<Image>           image;
                CMShared<ImageView>       image_view;
                CMShared<Texture>       texture;
                CMShared<DescriptorSet>   read_ds;
                CMShared<DescriptorSet>   write_ds;
            };

            std::shared_ptr<CommonResources> common_resources;
            RayTraceScale                  scale = RayTraceScale::RAY_TRACE_SCALE_QUARTER_RES;
            uint32_t                       g_buffer_mip = 0;
            OutputType                     current_output = OUTPUT_UPSAMPLE;
            uint32_t                       width;
            uint32_t                       height;
            bool                           denoise = true;
            bool                           first_frame = true;
            RayTrace                       ray_trace;
            ResetArgsStruct                      reset_args;
            TemporalAccumulationStruct           temporal_accumulation;
            BilateralBlurStruct                  bilateral_blur;
            UpsampleStruct                       upsample;
        };
	}
}