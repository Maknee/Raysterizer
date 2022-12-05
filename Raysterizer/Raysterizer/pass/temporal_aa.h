#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
        class TemporalAA
        {
        public:
            explicit TemporalAA() {}
            Error Setup(std::shared_ptr<CommonResources> common_resources_);
            void Render(CMShared<CommandBuffer> command_buffer);
            void Update();
            void UpdateGui();

            void Enable(bool enable) { enabled = enable; }
            void SetFeedbackMin(float v) { feedback_min = v; }
            void SetFeedbackMax(float v) { feedback_max = v; }

        private:
            void CreateImages();

        public:
            bool Enabled() const;
            glm::vec2 PrevJitter() const;
            glm::vec2 CurrentJitter() const;
            std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;

        private:
            uint32_t                                width;
            uint32_t                                height;
            std::shared_ptr<CommonResources> common_resources;
            std::vector<CMShared<Image>>         images;
            std::vector<CMShared<ImageView>>     views;
            std::vector<CMShared<Texture>>     textures;
            CMShared<ComputePipeline>            pipeline;
            CMShared<PipelineLayout>             pipeline_layout;
            std::vector<CMShared<DescriptorSet>> read_ds;
            std::vector<CMShared<DescriptorSet>> write_ds;
            bool                                    enabled = true;
            bool                                    sharpen = true;
            bool                                    reset = true;
            float                                   feedback_min = 0.88f;
            float                                   feedback_max = 0.97f;
            std::vector<glm::vec2>                  jitter_samples;
            glm::vec3                               prev_camera_pos = glm::vec3(0.0f);
            glm::vec2                               prev_jitter = glm::vec2(0.0f);
            glm::vec2                               current_jitter = glm::vec2(0.0f);
        };
	}
}