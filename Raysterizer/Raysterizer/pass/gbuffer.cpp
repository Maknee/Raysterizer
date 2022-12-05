#include "gbuffer.h"

#define GBUFFER_MIP_LEVELS 9

namespace Raysterizer
{
	namespace Pass
	{
		GBufferPass::GBufferPass()
		{

		}

		Error GBufferPass::Setup(std::shared_ptr<CommonResources> common_resources_)
		{
            common_resources = common_resources_;

			auto width = common_resources->width;
			auto height = common_resources->height;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			auto nearest_sampler = common_resources->nearest_sampler;

            for (int i = 0; i < num_frames; i++)
            {
				auto g_buffer_1_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, GBUFFER_MIP_LEVELS, 1, vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst, 
					vk::SampleCountFlagBits::e1);
				c.SetName(g_buffer_1_image, fmt::format("GBuffer 1 Image {}", i));

				auto g_buffer_2_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, GBUFFER_MIP_LEVELS, 1, vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
					vk::SampleCountFlagBits::e1);
				c.SetName(g_buffer_2_image, fmt::format("GBuffer 2 Image {}", i));

				auto g_buffer_3_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, GBUFFER_MIP_LEVELS, 1, vk::Format::eR16G16B16A16Sfloat, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
					vk::SampleCountFlagBits::e1);
				c.SetName(g_buffer_3_image, fmt::format("GBuffer 3 Image {}", i));

				auto depth_image = CommonResources::CreateImage(vk::ImageType::e2D, width, height, 1, GBUFFER_MIP_LEVELS, 1, Constants::DEPTH_FORMAT, VMA_MEMORY_USAGE_GPU_ONLY,
					vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
					vk::SampleCountFlagBits::e1);
				c.SetName(depth_image, fmt::format("GBuffer Depth Image {}", i));


				auto g_buffer_1_image_view = CommonResources::CreateImageView(g_buffer_1_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 0, GBUFFER_MIP_LEVELS);
				c.SetName(g_buffer_1_image_view, fmt::format("GBuffer 1 Image View {}", i));

				auto g_buffer_2_image_view = CommonResources::CreateImageView(g_buffer_2_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 0, GBUFFER_MIP_LEVELS);
				c.SetName(g_buffer_2_image_view, fmt::format("GBuffer 2 Image View {}", i));

				auto g_buffer_3_image_view = CommonResources::CreateImageView(g_buffer_3_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor, 0, GBUFFER_MIP_LEVELS);
				c.SetName(g_buffer_3_image_view, fmt::format("GBuffer 3 Image View {}", i));

				auto depth_image_view = CommonResources::CreateImageView(depth_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth, 0, GBUFFER_MIP_LEVELS);
				c.SetName(depth_image_view, fmt::format("GBuffer Depth Image View {}", i));


				auto g_buffer_1_fbo_image_view = CommonResources::CreateImageView(g_buffer_1_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
				c.SetName(g_buffer_1_fbo_image_view, fmt::format("GBuffer 1 FBO Image View {}", i));

				auto g_buffer_2_fbo_image_view = CommonResources::CreateImageView(g_buffer_2_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
				c.SetName(g_buffer_2_fbo_image_view, fmt::format("GBuffer 2 FBO Image View {}", i));

				auto g_buffer_3_fbo_image_view = CommonResources::CreateImageView(g_buffer_3_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
				c.SetName(g_buffer_3_fbo_image_view, fmt::format("GBuffer 3 FBO Image View {}", i));

				auto depth_fbo_image_view = CommonResources::CreateImageView(depth_image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eDepth);
				c.SetName(depth_fbo_image_view, fmt::format("GBuffer FBO Depth Image View {}", i));


				g_buffer_1_images.emplace_back(g_buffer_1_image);
				g_buffer_2_images.emplace_back(g_buffer_2_image);
				g_buffer_3_images.emplace_back(g_buffer_3_image);
				depth_images.emplace_back(depth_image);

				g_buffer_1_image_views.emplace_back(g_buffer_1_image_view);
				g_buffer_2_image_views.emplace_back(g_buffer_2_image_view);
				g_buffer_3_image_views.emplace_back(g_buffer_3_image_view);
				depth_image_views.emplace_back(depth_image_view);

				g_buffer_1_fbo_image_views.emplace_back(g_buffer_1_fbo_image_view);
				g_buffer_2_fbo_image_views.emplace_back(g_buffer_2_fbo_image_view);
				g_buffer_3_fbo_image_views.emplace_back(g_buffer_3_fbo_image_view);
				depth_fbo_image_views.emplace_back(depth_fbo_image_view);

				auto g_buffer_1_texture = std::make_shared<Texture>(g_buffer_1_image, g_buffer_1_image_view, nearest_sampler);
				auto g_buffer_2_texture = std::make_shared<Texture>(g_buffer_2_image, g_buffer_2_image_view, nearest_sampler);
				auto g_buffer_3_texture = std::make_shared<Texture>(g_buffer_3_image, g_buffer_3_image_view, nearest_sampler);
				auto depth_texture = std::make_shared<Texture>(depth_image, depth_image_view, nearest_sampler);

				g_buffer_1_textures.emplace_back(g_buffer_1_texture);
				g_buffer_2_textures.emplace_back(g_buffer_2_texture);
				g_buffer_3_textures.emplace_back(g_buffer_3_texture);
				depth_textures.emplace_back(depth_texture);
			}

			ReturnIfError(SetupRenderPass());

            return NoError();
		}

		Error GBufferPass::SetupRenderPass()
		{
			auto width = common_resources->width;
			auto height = common_resources->height;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			// Render pass
			{
				std::vector<vk::AttachmentDescription> attachments(4);

				// GBuffer1 attachment
				attachments[0].format = vk::Format::eR8G8B8A8Unorm;
				attachments[0].samples = vk::SampleCountFlagBits::e1;
				attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
				attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
				attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				attachments[0].initialLayout = vk::ImageLayout::eUndefined;
				attachments[0].finalLayout = vk::ImageLayout::eTransferSrcOptimal;

				// GBuffer2 attachment
				attachments[1].format = vk::Format::eR16G16B16A16Sfloat;
				attachments[1].samples = vk::SampleCountFlagBits::e1;
				attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
				attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
				attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				attachments[1].initialLayout = vk::ImageLayout::eUndefined;
				attachments[1].finalLayout = vk::ImageLayout::eTransferSrcOptimal;

				// GBuffer3 attachment
				attachments[2].format = vk::Format::eR16G16B16A16Sfloat;
				attachments[2].samples = vk::SampleCountFlagBits::e1;
				attachments[2].loadOp = vk::AttachmentLoadOp::eClear;
				attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
				attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				attachments[2].initialLayout = vk::ImageLayout::eUndefined;
				attachments[2].finalLayout = vk::ImageLayout::eTransferSrcOptimal;

				// Depth attachment
				attachments[3].format = Constants::DEPTH_FORMAT;
				attachments[3].samples = vk::SampleCountFlagBits::e1;
				attachments[3].loadOp = vk::AttachmentLoadOp::eClear;
				attachments[3].storeOp = vk::AttachmentStoreOp::eStore;
				attachments[3].stencilLoadOp = vk::AttachmentLoadOp::eClear;
				attachments[3].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				attachments[3].initialLayout = vk::ImageLayout::eUndefined;
				attachments[3].finalLayout = vk::ImageLayout::eTransferSrcOptimal;

				std::vector<vk::AttachmentReference> gbuffer_references(3);

				gbuffer_references[0].attachment = 0;
				gbuffer_references[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

				gbuffer_references[1].attachment = 1;
				gbuffer_references[1].layout = vk::ImageLayout::eColorAttachmentOptimal;

				gbuffer_references[2].attachment = 2;
				gbuffer_references[2].layout = vk::ImageLayout::eColorAttachmentOptimal;

				vk::AttachmentReference depth_reference{};
				depth_reference.attachment = 3;
				depth_reference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

				std::vector<vk::SubpassDescription> subpass_description(1);

				subpass_description[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
				subpass_description[0].setColorAttachments(gbuffer_references);
				subpass_description[0].pDepthStencilAttachment = &depth_reference;
				subpass_description[0].inputAttachmentCount = 0;
				subpass_description[0].pInputAttachments = nullptr;
				subpass_description[0].preserveAttachmentCount = 0;
				subpass_description[0].pPreserveAttachments = nullptr;
				subpass_description[0].pResolveAttachments = nullptr;

				// Subpass dependencies for layout transitions
				std::vector<vk::SubpassDependency> dependencies(2);

				dependencies[0] = vk::SubpassDependency{}
					.setSrcSubpass(VK_SUBPASS_EXTERNAL)
					.setDstSubpass(0)
					.setSrcStageMask(vk::PipelineStageFlagBits::eComputeShader)
					.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
					.setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
					.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
					.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

				dependencies[1] = vk::SubpassDependency{}
					.setSrcSubpass(0)
					.setDstSubpass(VK_SUBPASS_EXTERNAL)
					.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
					.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
					.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
					.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
					.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

				/*
				dependencies[0] = vk::SubpassDependency{}
					.setSrcSubpass(VK_SUBPASS_EXTERNAL)
					.setDstSubpass(0)
					.setSrcStageMask(vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
					.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
					.setSrcAccessMask(vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
					.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead)
					.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

				dependencies[1] = vk::SubpassDependency{}
					.setSrcSubpass(0)
					.setDstSubpass(VK_SUBPASS_EXTERNAL)
					.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
					.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
					.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
					.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
					.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
				*/

				RenderPassCreateInfo render_pass_create_info{};
				render_pass_create_info.attachment_descriptions = attachments;
				//render_pass_create_info.color_attachment_references = gbuffer_references;
				//render_pass_create_info.depth_stencil_reference = depth_reference;
				render_pass_create_info.subpass_descriptions = subpass_description;
				render_pass_create_info.subpass_dependencies = dependencies;

				AssignOrReturnError(render_pass, c.Get(render_pass_create_info));

				for (auto& a : attachments)
				{
					a.setLoadOp(vk::AttachmentLoadOp::eLoad);
				}
				render_pass_create_info.attachment_descriptions = attachments;
				AssignOrReturnError(load_op_render_pass, c.Get(render_pass_create_info));
			}

			// Frame buffer
			{
				frame_buffers.resize(num_frames);
				for (auto i = 0; i < num_frames; i++)
				{
					std::vector<vk::ImageView> attachments{ *g_buffer_1_fbo_image_views[i], *g_buffer_2_fbo_image_views[i], *g_buffer_3_fbo_image_views[i], *depth_fbo_image_views[i] };

					auto frame_buffer_create_info = vk::FramebufferCreateInfo{}
						.setRenderPass(*render_pass)
						.setAttachments(attachments)
						.setWidth(width)
						.setHeight(height)
						.setLayers(1);

					auto frame_buffer = AssignOrPanicVkError(device.createFramebufferUnique(frame_buffer_create_info));
					frame_buffers[i] = std::move(frame_buffer);
				}

				load_op_frame_buffers.resize(num_frames);
				for (auto i = 0; i < num_frames; i++)
				{
					std::vector<vk::ImageView> attachments{ *g_buffer_1_fbo_image_views[i], *g_buffer_2_fbo_image_views[i], *g_buffer_3_fbo_image_views[i], *depth_fbo_image_views[i] };

					auto frame_buffer_create_info = vk::FramebufferCreateInfo{}
						.setRenderPass(*load_op_render_pass)
						.setAttachments(attachments)
						.setWidth(width)
						.setHeight(height)
						.setLayers(1);

					auto frame_buffer = AssignOrPanicVkError(device.createFramebufferUnique(frame_buffer_create_info));
					load_op_frame_buffers[i] = std::move(frame_buffer);
				}
			}

			return NoError();
		}

		void GBufferPass::BeginRender(CMShared<CommandBuffer> command_buffer)
		{
			// wait until next frame to start
			if (finished_render)
			{
				finished_render = false;
			}
			else
			{
				return;
			}

			ScopedGPUProfileRaysterizer(command_buffer, "GBuffer Begin Render");

			auto& cb = **command_buffer;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto previous_frame_index = c.GetPrevFrameIndex();
			auto num_frames = c.GetNumFrames();

			const auto& raysterizer_vulkan_state = Raysterizer::MiddleWare::RaysterizerVulkanState::Get();
			const auto& render_state = raysterizer_vulkan_state.GetRenderState();

			command_buffer->Begin();

			if (common_resources->first_frame)
			{
				auto subresource_range = vk::ImageSubresourceRange{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(GBUFFER_MIP_LEVELS)
					.setBaseArrayLayer(0)
					.setLayerCount(1);

				const auto& g_buffer_1_image = g_buffer_1_images[current_frame_index];
				const auto& g_buffer_2_image = g_buffer_2_images[current_frame_index];
				const auto& g_buffer_3_image = g_buffer_3_images[current_frame_index];
				const auto& depth_image = depth_images[current_frame_index];

				PanicIfError(command_buffer->SetImageLayout(g_buffer_1_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
				PanicIfError(command_buffer->SetImageLayout(g_buffer_2_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
				PanicIfError(command_buffer->SetImageLayout(g_buffer_3_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
			
				// We use this as a read in from prev
				PanicIfError(command_buffer->SetImageLayout(g_buffer_3_images[previous_frame_index], vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));

				subresource_range.aspectMask = vk::ImageAspectFlagBits::eDepth;
				PanicIfError(command_buffer->SetImageLayout(depth_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
			}

			auto clear_color = render_state.GetClearColor();
			auto clear_depth = render_state.GetClearDepth();

			std::vector<vk::ClearValue> clear_values(4);

			clear_values[0].color.float32[0] = clear_color.r;
			clear_values[0].color.float32[1] = clear_color.g;
			clear_values[0].color.float32[2] = clear_color.b;
			clear_values[0].color.float32[3] = clear_color.a;

			clear_values[1].color.float32[0] = 0.0f;
			clear_values[1].color.float32[1] = 0.0f;
			clear_values[1].color.float32[2] = 0.0f;
			clear_values[1].color.float32[3] = 0.0f;

			clear_values[2].color.float32[0] = 0.0f;
			clear_values[2].color.float32[1] = 0.0f;
			clear_values[2].color.float32[2] = 0.0f;
			clear_values[2].color.float32[3] = -1.0f;

			clear_values[3].depthStencil.stencil = 0xCCCCCCCC;
			clear_values[3].depthStencil.depth = clear_depth;

			auto render_pass_begin_info = vk::RenderPassBeginInfo{}
				.setRenderPass(*render_pass)
				.setRenderArea(vk::Rect2D{}
					.setOffset({ 0, 0 })
					.setExtent(c.GetWindowExtent())
				)
				.setFramebuffer(*frame_buffers[current_frame_index])
				.setClearValues(clear_values);

			if (begin_render_pass_execution_count > 0)
			{
				render_pass_begin_info
					.setRenderPass(*load_op_render_pass)
					.setFramebuffer(*load_op_frame_buffers[current_frame_index]);
			}

			cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

			auto width = common_resources->width;
			auto height = common_resources->height;

			cb.setViewport(0, vk::Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f });
			cb.setScissor(0, vk::Rect2D(width, height));

			lasted_used_command_buffer = command_buffer;
			begin_render_pass_execution_count++;
		}

		void GBufferPass::EndRender(CMShared<CommandBuffer> command_buffer)
		{
			ScopedGPUProfileRaysterizer(command_buffer, "GBuffer End Render");
			
			if (finished_render)
			{
				return;
			}

			auto& cb = **command_buffer;
			cb.endRenderPass();

			RenderFrame& render_frame = c.GetRenderFrame();
			auto current_frame_index = c.GetFrameIndex();

			CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

			/*
			{
				ScopedGPUProfileRaysterizer(command_buffer, "GBuffer depth buffer");

				auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
				{
					ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("other/compute_normal_from_depth.comp")},
				};

				auto plci = PipelineLayoutCreateInfo
				{
					shader_module_create_infos,
				};

				CMShared<PipelineLayoutInfo> pli = AssignOrPanic(c.Get(plci));
				CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
				CMShared<PipelineLayout> pl = AssignOrPanic(c.Get(pli));

				ComputePipelineCreateInfo compute_pipeline_create_info{ pl };
				CMShared<ComputePipeline> cp = AssignOrPanic(c.Get(compute_pipeline_create_info));

				c.SetName(pl, "GI Irradiance Probe Update Pipeline Layout");
				c.SetName(cp, "GI Irradiance Probe Update Pipeline");

				cb.bindPipeline(vk::PipelineBindPoint::eCompute, cp->pipeline);
				cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pl->pipeline_layout, 0, ds->descriptor_sets, {});

				{
					PanicIfError(ds->Bind(0, 0, probe_grid.irradiance_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));
					PanicIfError(ds->Bind(0, 1, probe_grid.depth_texture[write_idx], vk::ImageLayout::eGeneral, vk::DescriptorType::eStorageImage));

					PanicIfError(render_frame.FlushPendingWrites(ds));

					ProbeUpdatePushConstants push_constants;
					push_constants.first_frame = (uint32_t)first_frame;

					PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
				}

				const uint32_t dispatch_x = static_cast<uint32_t>(probe_grid.probe_counts.x * probe_grid.probe_counts.y);
				const uint32_t dispatch_y = static_cast<uint32_t>(probe_grid.probe_counts.z);

				cb.dispatch(dispatch_x, dispatch_y, 1);
			}
			*/

			lasted_used_command_buffer = command_buffer;
			finished_render = true;
		}

		void GBufferPass::Render(CMShared<CommandBuffer> command_buffer,
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
			uint32_t vertex_offset)
		{
			ScopedGPUProfileRaysterizer(command_buffer, "GBuffer");

			BeginRender(command_buffer);
			auto& cb = **command_buffer;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			const auto& raysterizer_vulkan_state = Raysterizer::MiddleWare::RaysterizerVulkanState::Get();
			const auto& render_state = raysterizer_vulkan_state.GetRenderState();

			// Descriptor set / Descriptor set layout
			/*
			CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];
			CMShared<PipelineLayout> pl{};

			{
				ScopedCPUProfileRaysterizer("DescriptorSet Creation");
				CMShared<DescriptorSet> ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
				c.SetName(ds, fmt::format("Descriptor set frame {}", current_frame_index));
			}
			{
				ScopedCPUProfileRaysterizer("PipelineLayout Creation");
				pl = AssignOrPanic(c.Get(pli));
				c.SetName(pl, fmt::format("Pipeline layout frame {}", current_frame_index));
			}
			*/

			// Descriptor set binding
			{
				ScopedCPUProfileRaysterizer("GBuffer Descriptor set binding");

				const auto& g_buffer_1_texture = g_buffer_1_textures[current_frame_index];
				const auto& g_buffer_2_texture = g_buffer_2_textures[current_frame_index];
				const auto& g_buffer_3_texture = g_buffer_3_textures[current_frame_index];
				const auto& depth_texture = depth_textures[current_frame_index];

				/*
				PanicIfError(ds->Bind(0, 0, g_buffer_1_texture));
				PanicIfError(ds->Bind(0, 1, g_buffer_2_texture));
				PanicIfError(ds->Bind(0, 2, g_buffer_3_texture));
				PanicIfError(ds->Bind(0, 3, depth_texture));
				*/

				PanicIfError(render_frame.FlushPendingWrites(ds));
			}

			if (vertex_input_attribute_descriptions.empty())
			{
				auto vertex_input_attribute_description = vk::VertexInputAttributeDescription{}
					.setLocation(0)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(0);

				vertex_input_attribute_descriptions.emplace_back(vertex_input_attribute_description);
			}

			// Pipeline
			CMShared<GraphicsPipeline> gp{};
			{
				auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{}
					.setVertexAttributeDescriptions(vertex_input_attribute_descriptions)
					.setVertexBindingDescriptions(vertex_input_binding_descriptions);

				auto color_pcbas = vk::PipelineColorBlendAttachmentState{}
					.setBlendEnable(render_state.GetBlend())
					.setSrcColorBlendFactor(render_state.GetSrcColorBlendFactor())
					.setDstColorBlendFactor(render_state.GetDstColorBlendFactor())
					.setColorBlendOp(render_state.GetColorBlendOp())
					.setSrcAlphaBlendFactor(render_state.GetSrcAlphaBlendFactor())
					.setDstAlphaBlendFactor(render_state.GetDstAlphaBlendFactor())
					.setAlphaBlendOp(render_state.GetAlphaBlendOp())
					.setColorWriteMask(render_state.GetColorWriteMask());


				switch (common_resources->pipeline_manager->GetGameType())
				{
				case MiddleWare::GameType::Dolphin:
				{
					color_pcbas.setBlendEnable(false);
					break;
				}
				}

				auto pcbas = vk::PipelineColorBlendAttachmentState{}
					.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
					.setBlendEnable(VK_FALSE);

				std::vector<vk::PipelineColorBlendAttachmentState> pcbases{ color_pcbas, pcbas, pcbas };

				auto blend_color = render_state.GetBlendColor();
				bool color_logic_op_enabled = render_state.GetColorLogicOpEnabled();
				auto color_logic_op = render_state.GetColorLogicOp();

				//std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eDepthTestEnable, vk::DynamicState::eDepthWriteEnable, vk::DynamicState::eViewport, vk::DynamicState::eScissor };
				//std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::eDepthTestEnable, vk::DynamicState::eDepthWriteEnable };
				std::vector<vk::DynamicState> dynamic_states{ vk::DynamicState::ePrimitiveTopology, vk::DynamicState::eDepthCompareOp, vk::DynamicState::eViewport, vk::DynamicState::eScissor, };

				auto gpci = GraphicsPipelineCreateInfo
				{
					render_pass,
					pl,
					vertex_input_state_create_info,
					vk::PipelineTessellationStateCreateInfo{},
					//.setPatchControlPoints(3),
					vk::PipelineInputAssemblyStateCreateInfo{}
						.setTopology(current_primitive_topology)
						//.setTopology(vk::PrimitiveTopology::ePatchList)
						.setPrimitiveRestartEnable(VK_FALSE),
					vk::Viewport{}
						.setX(0.0f)
						.setY(0.0f)
						.setWidth(c.GetWindowExtent().width)
						.setHeight(c.GetWindowExtent().height)
						.setMinDepth(0.0f)
						.setMaxDepth(1.0f),
					vk::Rect2D{}
						.setOffset({ 0, 0 })
						.setExtent(c.GetWindowExtent()),
					vk::PipelineRasterizationStateCreateInfo{}
						.setDepthClampEnable(VK_FALSE)
						.setRasterizerDiscardEnable(VK_FALSE)
						.setPolygonMode(vk::PolygonMode::eFill)
						.setLineWidth(1.0f)
						.setCullMode(vk::CullModeFlagBits::eBack)
						.setFrontFace(vk::FrontFace::eClockwise)
						.setDepthBiasEnable(VK_FALSE)
						.setDepthBiasConstantFactor(0.0f)
						.setDepthBiasClamp(0.0f)
						.setDepthBiasSlopeFactor(0.0f),
					vk::PipelineColorBlendStateCreateInfo{}
						.setLogicOp(color_logic_op)
						.setBlendConstants({ blend_color.r, blend_color.g, blend_color.b, blend_color.a })
						.setAttachments(pcbases),
					vk::PipelineMultisampleStateCreateInfo{}
						.setRasterizationSamples(vk::SampleCountFlagBits::e1)
						.setSampleShadingEnable(VK_FALSE)
						.setMinSampleShading(1.0f)
						.setPSampleMask(nullptr)
						.setAlphaToCoverageEnable(VK_FALSE)
						.setAlphaToOneEnable(VK_FALSE),
					vk::PipelineDepthStencilStateCreateInfo{}
						//.setDepthTestEnable(VK_TRUE)
						//.setDepthWriteEnable(VK_TRUE)
						.setDepthTestEnable(render_state.GetDepthTest())
						.setDepthWriteEnable(render_state.GetDepthWriting())
						.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
						.setBack(vk::StencilOpState{}.setCompareOp(vk::CompareOp::eAlways))
						.setDepthBoundsTestEnable(VK_FALSE)
						.setMinDepthBounds(0.0f)
						.setMaxDepthBounds(0.0f)
						.setStencilTestEnable(VK_FALSE),
					vk::PipelineDynamicStateCreateInfo{}
						.setDynamicStates(dynamic_states),
					vk::PipelineCache{}
				};

				/*
				gpci.viewport
					.setY(gpci.viewport.height - gpci.viewport.y)
					.setHeight(-gpci.viewport.height);
				*/

				gpci.pipeline_rasterization_state_create_info
					.setCullMode(vk::CullModeFlagBits::eNone)
					.setFrontFace(vk::FrontFace::eClockwise);

				switch (common_resources->pipeline_manager->GetGameType())
				{
					case MiddleWare::GameType::Dolphin:
					{
						viewport.minDepth = 1.0f;
						viewport.maxDepth = 0.0f;
						/*
						gpci.pipeline_depth_stencil_state_create_info.setDepthTestEnable(true);
						gpci.pipeline_depth_stencil_state_create_info.setDepthWriteEnable(true);
						if (index_count == 6 || index_count == 645)
						{
							gpci.pipeline_depth_stencil_state_create_info.setDepthWriteEnable(false);
						}
						*/

						/*
						auto& color_pcbas = pcbases[0];
						color_pcbas.blendEnable = true;
						color_pcbas.srcColorBlendFactor = vk::BlendFactor::eSrc1Alpha;
						color_pcbas.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrc1Alpha;
						color_pcbas.colorBlendOp = vk::BlendOp::eAdd;
						color_pcbas.srcAlphaBlendFactor = vk::BlendFactor::eSrc1Alpha;
						color_pcbas.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrc1Alpha;
						color_pcbas.alphaBlendOp = vk::BlendOp::eAdd;
						color_pcbas.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

						gpci.pipeline_color_blend_state_create_info
							.setAttachments(pcbases);
						*/

						gpci.pipeline_rasterization_state_create_info
							.setCullMode(vk::CullModeFlagBits::eNone)
							.setFrontFace(vk::FrontFace::eCounterClockwise);
						break;
					}
					case MiddleWare::GameType::PPSSPP:
					{
						//gpci.pipeline_depth_stencil_state_create_info.setDepthTestEnable(false);
						//gpci.pipeline_depth_stencil_state_create_info.setDepthWriteEnable(false);
						
						gpci.pipeline_rasterization_state_create_info
							.setCullMode(vk::CullModeFlagBits::eFront)
							.setFrontFace(vk::FrontFace::eCounterClockwise);

						break;
					}
					default:
					{
						break;
					}
				}
				{
					ScopedCPUProfileRaysterizer("GBuffer Graphics Pipeline Creation");
					gp = AssignOrPanic(c.Get(gpci));
					c.SetName(gp, fmt::format("GraphicsPipeline {}", current_frame_index));
				}
			}

			pipeline_layout = pl;

			cb.bindPipeline(vk::PipelineBindPoint::eGraphics, gp->pipeline);
			cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});

			//if (primitive_topology != current_primitive_topology)
			{
				cb.setPrimitiveTopology(primitive_topology);
				current_primitive_topology = primitive_topology;

				cb.setDepthCompareOp(depth_compare_op);

				/*
				cb.setViewport(0, vk::Viewport{}
					.setX(0.0f)
					.setY(0.0f)
					.setWidth(c.GetWindowExtent().width)
					.setHeight(c.GetWindowExtent().height)
					.setMinDepth(0.0f)
					.setMaxDepth(1.0f)
					);
				*/

				cb.setViewport(0, viewport);

				cb.setScissor(0, vk::Rect2D{}
					.setOffset({ 0, 0 })
					.setExtent(c.GetWindowExtent())
					);

			}

			//cb.setDepthTestEnable(render_state.GetDepthTest());
			//cb.setDepthWriteEnable(render_state.GetDepthWriting());

			auto vertex_offset_zero = 0;

			if (instance_binding_to_buffers.empty())
			{
				const static auto dummy_vertex_buffer = AssignOrPanic(render_frame.CreateBuffer(MemoryUsage::CpuToGpu, vk::BufferUsageFlagBits::eVertexBuffer, 1, true));
				cb.bindVertexBuffers(0, **dummy_vertex_buffer, vertex_offset_zero);
			}

			for (const auto& [binding, buffer] : instance_binding_to_buffers)
			{
				cb.bindVertexBuffers(binding, **buffer, vertex_offset_zero);
			}

			//TODO: temp to start index = 0 because vertex buffers are copied via their starting index as 0, not the entire buffer
			start_index = 0;
			cb.bindIndexBuffer(**index_buffer, vertex_offset_zero, index_type);
			cb.drawIndexed(index_count, instance_count, start_index, vertex_offset, 0);

			lasted_used_command_buffer = command_buffer;
			//EndRender(command_buffer);
			//BeginRender(command_buffer);
		}

		void GBufferPass::Finialize(CMShared<CommandBuffer> command_buffer, bool copy_to_mipmap)
		{
			ScopedGPUProfileRaysterizer(command_buffer, "GBuffer Finalize");

			RenderFrame& render_frame = c.GetRenderFrame();
			auto current_frame_index = c.GetFrameIndex();

			BeginRender(command_buffer);
			EndRender(command_buffer);

			if (copy_to_mipmap)
			{
				// Downsample
				const auto& g_buffer_1_image = g_buffer_1_images[current_frame_index];
				const auto& g_buffer_2_image = g_buffer_2_images[current_frame_index];
				const auto& g_buffer_3_image = g_buffer_3_images[current_frame_index];
				const auto& depth_image = depth_images[current_frame_index];
				
				/*
				auto subresource_range = vk::ImageSubresourceRange{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1);

				PanicIfError(command_buffer->SetImageLayout(g_buffer_1_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
				PanicIfError(command_buffer->SetImageLayout(g_buffer_2_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
				PanicIfError(command_buffer->SetImageLayout(g_buffer_3_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
				subresource_range.setAspectMask(vk::ImageAspectFlagBits::eDepth);
				PanicIfError(command_buffer->SetImageLayout(depth_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresource_range));
				*/

				PanicIfError(render_frame.GenerateMipMaps(command_buffer, g_buffer_1_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::Filter::eNearest));
				PanicIfError(render_frame.GenerateMipMaps(command_buffer, g_buffer_2_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::Filter::eNearest));
				PanicIfError(render_frame.GenerateMipMaps(command_buffer, g_buffer_3_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor, vk::Filter::eNearest));
				PanicIfError(render_frame.GenerateMipMaps(command_buffer, depth_image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eDepth, vk::Filter::eNearest));
			}

			begin_render_pass_execution_count = 0;
		}

		void GBufferPass::Clear(CMShared<CommandBuffer> command_buffer, bool clear_color, bool clear_depth, bool clear_stencil)
		{
			if (command_buffer)
			{
				auto& vk_command_buffer = command_buffer->command_buffer;

				auto g_buffer_1_attachment = vk::ClearAttachment{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setClearValue(vk::ClearValue{0})
					.setColorAttachment(0);

				auto g_buffer_2_attachment = vk::ClearAttachment{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setClearValue(vk::ClearValue{ 0 })
					.setColorAttachment(1);

				auto g_buffer_3_attachment = vk::ClearAttachment{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setClearValue(vk::ClearValue{ 0 })
					.setColorAttachment(2);

				auto depth_attachment = vk::ClearAttachment{}
					.setAspectMask(vk::ImageAspectFlagBits::eDepth)
					.setClearValue(vk::ClearValue{ 0 })
					.setColorAttachment(3);

				auto clear_attachments = std::vector<vk::ClearAttachment>{ g_buffer_1_attachment, g_buffer_2_attachment, g_buffer_3_attachment, depth_attachment };

				CMShared<Texture> g_buffer_1_texture = GetGBuffer1Texture();
				CMShared<Texture> g_buffer_2_texture = GetGBuffer2Texture();
				CMShared<Texture> g_buffer_3_texture = GetGBuffer3Texture();
				CMShared<Texture> depth_texture = GetDepthTexture();

				auto g_buffer_1_clear_rect = vk::ClearRect{
					vk::Rect2D{
						{ 0, 0 },
						{ g_buffer_1_texture->image->image_create_info.image_create_info.extent.width, g_buffer_1_texture->image->image_create_info.image_create_info.extent.height }
					},
					0,
					0
				};

				auto g_buffer_2_clear_rect = vk::ClearRect{
					vk::Rect2D{
						{ 0, 0 },
						{ g_buffer_2_texture->image->image_create_info.image_create_info.extent.width, g_buffer_2_texture->image->image_create_info.image_create_info.extent.height }
					},
					0,
					0
				};

				auto g_buffer_3_clear_rect = vk::ClearRect{
					vk::Rect2D{
						{ 0, 0 },
						{ g_buffer_3_texture->image->image_create_info.image_create_info.extent.width, g_buffer_3_texture->image->image_create_info.image_create_info.extent.height }
					},
					0,
					0
				};

				auto depth_clear_rect = vk::ClearRect{
					vk::Rect2D{
						{ 0, 0 },
						{ depth_texture->image->image_create_info.image_create_info.extent.width, depth_texture->image->image_create_info.image_create_info.extent.height }
					},
					0,
					0
				};

				auto clear_rects = std::vector<vk::ClearRect>{ g_buffer_1_clear_rect, g_buffer_2_clear_rect, g_buffer_3_clear_rect, depth_clear_rect };

				//vk_command_buffer.clearAttachments(clear_attachments, clear_rects);
				EndRender(command_buffer);
				BeginRender(command_buffer);
			}
		}

		CMShared<Texture> GBufferPass::GetGBuffer1Texture() const
		{
			return g_buffer_1_textures[c.GetFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetGBuffer2Texture() const
		{
			return g_buffer_2_textures[c.GetFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetGBuffer3Texture() const
		{
			return g_buffer_3_textures[c.GetFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetDepthTexture() const
		{
			return depth_textures[c.GetFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetPrevGBuffer1Texture() const
		{
			return g_buffer_1_textures[c.GetPrevFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetPrevGBuffer2Texture() const
		{
			return g_buffer_2_textures[c.GetPrevFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetPrevGBuffer3Texture() const
		{
			return g_buffer_3_textures[c.GetPrevFrameIndex()];
		}

		CMShared<Texture> GBufferPass::GetPrevDepthTexture() const
		{
			return depth_textures[c.GetPrevFrameIndex()];
		}
		
		std::vector<WriteDescriptorSetBindedResource> GBufferPass::OutputBindings() const
		{
			return { GetGBuffer1Texture(), GetGBuffer2Texture(), GetGBuffer3Texture(), GetDepthTexture() };
		}
		
		std::vector<WriteDescriptorSetBindedResource> GBufferPass::HistoryBindings() const
		{
			return { GetPrevGBuffer1Texture(), GetPrevGBuffer2Texture(), GetPrevGBuffer3Texture(), GetPrevDepthTexture() };
		}
	}
}