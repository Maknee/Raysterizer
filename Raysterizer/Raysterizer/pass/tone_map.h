#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
		class ToneMapPass
		{
		public:
			explicit ToneMapPass();
			Error Setup(std::shared_ptr<CommonResources> common_resources_);
			void Render(CMShared<CommandBuffer> command_buffer);
			void UpdateGui();

		private:
			std::shared_ptr<CommonResources> common_resources{};

			CMShared<RenderPass> render_pass;

			uint32_t                       width;
			uint32_t                       height;
			float                          exposure = 1.0f;
		};
	}
}