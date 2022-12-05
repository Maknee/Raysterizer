#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
		class HosekWilkieSkyModel
		{
		public:
			Error Setup(std::shared_ptr<CommonResources> common_resources_);
            void Update(CMShared<CommandBuffer> command_buffer, glm::vec3 direction);

            CMShared<Image> GetImage() { return cubemap_image; }
            CMShared<ImageView> GetImageView() { return cubemap_image_view; }
            CMShared<Texture> GetTexture() const { return cubemap_texture; }

		private:
			std::shared_ptr<CommonResources> common_resources{};

            CMShared<Image>                    cubemap_image;
            CMShared<ImageView>                cubemap_image_view;
            CMShared<Texture>                cubemap_texture;
            std::vector<CMShared<ImageView>>   face_image_views;
            std::vector<vk::UniqueFramebuffer> face_framebuffers;
            CMShared<RenderPass>               cubemap_renderpass;
            CMShared<GraphicsPipeline>         cubemap_pipeline;
            CMShared<PipelineLayout>           cubemap_pipeline_layout;
            CMShared<Buffer>                   cube_vbo;
            CMShared<DescriptorSetLayout>      ds_layout;
            CMShared<DescriptorSet>            ds;
            CMShared<Buffer>                   ubo;

            std::vector<glm::mat4>            view_projection_mats;
            float                             normalized_sun_y = 1.15f;
            float                             albedo = 0.1f;
            float                             turbidity = 4.0f;
            glm::vec3                         A, B, C, D, E, F, G, H, I;
            glm::vec3                         Z;
		};
	}
}