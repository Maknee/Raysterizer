#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		class Context
		{
		public:
			explicit Context();
			void Init();

			static Context& Get()
			{
				static Context c;
				return c;
			};

			State& state = State::Get();
			BufferManager& buffer_manager = BufferManager::Get();
			ShaderManager& shader_manager = ShaderManager::Get();
			TextureManager& texture_manager = TextureManager::Get();
			SamplerManager& sampler_manager = SamplerManager::Get();
			FrameBufferManager& frame_buffer_manager = FrameBufferManager::Get();
			Raysterizer::MiddleWare::RaysterizerVulkanState& raysterizer_vulkan_state = Raysterizer::MiddleWare::RaysterizerVulkanState::Get();
			
			Raysterizer::MiddleWare::PipelineManager& GetPipelineManager() { return raysterizer_vulkan_state.GetPipelineManager(); }

			// Parsed from system
			std::atomic<bool> compatibility_mode = false;
			GLuint opengl_version{};
			GLuint shader_version{};
			std::string gl_version{};

			/////////////////////////////////////////
		private:
		};
	}
}