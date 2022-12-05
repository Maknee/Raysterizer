#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Pass
	{
		class GBufferPass
		{
		public:
			explicit GBufferPass();
			Error Setup(std::shared_ptr<CommonResources> common_resources_);
			Error SetupRenderPass();
			void BeginRender(CMShared<CommandBuffer> command_buffer);
			void EndRender(CMShared<CommandBuffer> command_buffer);
			void Render(CMShared<CommandBuffer> command_buffer,
				CMShared<PipelineLayout> pl,
				CMShared<DescriptorSet> ds,
				std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions,
				std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions,
				flat_hash_map<uint32_t, CMShared<Buffer>> instance_binding_to_buffers,
				CMShared<Buffer> index_buffer,
				vk::IndexType index_type,
				vk::PrimitiveTopology primitive_topology,
				vk::CompareOp depth_compare_op,
				vk::Viewport viewport,
				uint32_t start_index,
				uint32_t index_count,
				uint32_t instance_count,
				uint32_t vertex_offset);
			void Finialize(CMShared<CommandBuffer> command_buffer, bool copy_to_mipmap = false);
			void Clear(CMShared<CommandBuffer> command_buffer, bool clear_color, bool clear_depth, bool clear_stencil);

			CMShared<Texture> GetGBuffer1Texture() const;
			CMShared<Texture> GetGBuffer2Texture() const;
			CMShared<Texture> GetGBuffer3Texture() const;
			CMShared<Texture> GetDepthTexture() const;

			CMShared<Texture> GetPrevGBuffer1Texture() const;
			CMShared<Texture> GetPrevGBuffer2Texture() const;
			CMShared<Texture> GetPrevGBuffer3Texture() const;
			CMShared<Texture> GetPrevDepthTexture() const;

			std::vector<WriteDescriptorSetBindedResource> OutputBindings() const;
			std::vector<WriteDescriptorSetBindedResource> HistoryBindings() const;

			auto GetBeginRenderPassExecutionCount() const { return begin_render_pass_execution_count; }
			auto HasFinishedPassMoreThanOnceThisFrame() const { return begin_render_pass_execution_count > 1; }
		
		private:
		public:
			std::shared_ptr<CommonResources> common_resources{};

			std::vector<CMShared<Image>> g_buffer_1_images{};
			std::vector<CMShared<Image>> g_buffer_2_images{};
			std::vector<CMShared<Image>> g_buffer_3_images{};
			std::vector<CMShared<Image>> depth_images{};

			std::vector<CMShared<ImageView>> g_buffer_1_image_views{};
			std::vector<CMShared<ImageView>> g_buffer_2_image_views{};
			std::vector<CMShared<ImageView>> g_buffer_3_image_views{};
			std::vector<CMShared<ImageView>> depth_image_views{};

			std::vector<CMShared<ImageView>> g_buffer_1_fbo_image_views{};
			std::vector<CMShared<ImageView>> g_buffer_2_fbo_image_views{};
			std::vector<CMShared<ImageView>> g_buffer_3_fbo_image_views{};
			std::vector<CMShared<ImageView>> depth_fbo_image_views{};

			std::vector<CMShared<Texture>> g_buffer_1_textures{};
			std::vector<CMShared<Texture>> g_buffer_2_textures{};
			std::vector<CMShared<Texture>> g_buffer_3_textures{};
			std::vector<CMShared<Texture>> depth_textures{};

			CMShared<RenderPass> render_pass;
			std::vector<vk::UniqueFramebuffer> frame_buffers{};

			CMShared<RenderPass> load_op_render_pass;
			std::vector<vk::UniqueFramebuffer> load_op_frame_buffers{};

			CMShared<PipelineLayout> pipeline_layout;

			CMShared<CommandBuffer> lasted_used_command_buffer;

			vk::PrimitiveTopology current_primitive_topology = vk::PrimitiveTopology::eTriangleList;

			FrameCounter begin_render_pass_execution_count = 0;
			bool finished_render = true;
		};
	}
}