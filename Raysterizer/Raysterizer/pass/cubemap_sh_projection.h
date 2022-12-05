#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
		class CubemapSHProjection
		{
		public:
			Error Setup(std::shared_ptr<CommonResources> common_resources_, CMShared<Texture> cubemap);
            void Update(CMShared<CommandBuffer> command_buffer);

            CMShared<Image> GetImage() { return image; }
            CMShared<ImageView> GetImageView() { return image_view; }
            CMShared<Texture> GetTexture() const { return texture; }

		private:
			std::shared_ptr<CommonResources> common_resources{};

            CMShared<ImageView>           cubemap_image_view;
            CMShared<Texture>           cubemap_texture;
            CMShared<Image>               image;
            CMShared<ImageView>           image_view;
            CMShared<Texture>           texture;
            CMShared<Image>               intermediate_image;
            CMShared<ImageView>           intermediate_image_view;
            CMShared<Texture>           intermediate_texture;
            CMShared<ComputePipeline>     projection_pipeline;
            CMShared<ComputePipeline>     add_pipeline;
            CMShared<PipelineLayout>      projection_pipeline_layout;
            CMShared<PipelineLayout>      add_pipeline_layout;
            CMShared<DescriptorSetLayout> ds_layout;
            CMShared<DescriptorSet>       projection_ds;
            CMShared<DescriptorSet>       add_ds;
            float                        size;
		};
	}
}