#pragma once

#include "pch.h"

namespace Raysterizer
{
    namespace Pass
    {
        class EquirectangularToCubemap
        {
        public:
            Error Setup(std::shared_ptr<CommonResources> common_resources_, vk::Format image_format);
            void Convert(CMShared<Image> input_image, CMShared<Image> output_image);

        private:
            std::shared_ptr<CommonResources> common_resources{};

            CMShared<RenderPass>               cubemap_renderpass;
            CMShared<GraphicsPipeline>         cubemap_pipeline;
            CMShared<PipelineLayout>           cubemap_pipeline_layout;
            CMShared<Buffer>                   cube_vbo;

            std::vector<glm::mat4>            view_projection_mats;
        };
    }
}