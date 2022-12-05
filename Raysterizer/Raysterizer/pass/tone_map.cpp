#include "tone_map.h"

#define GBUFFER_MIP_LEVELS 9

namespace Raysterizer
{
	namespace Pass
	{
		// -----------------------------------------------------------------------------------------------------------------------------------

		struct ToneMapPushConstants
		{
			int   single_channel;
			float exposure;
			VisualizationType visualization_type;
		};

		ToneMapPass::ToneMapPass()
		{

		}

		Error ToneMapPass::Setup(std::shared_ptr<CommonResources> common_resources_)
		{
			common_resources = common_resources_;

			render_pass = c.GetGlobalRenderPass();

			return NoError();
		}

		void ToneMapPass::Render(CMShared<CommandBuffer> command_buffer)
		{
			ScopedGPUProfileRaysterizer(command_buffer, "Tone map");

			auto& cb = **command_buffer;

			vk::Device device = c.GetDevice();
			RenderFrame& render_frame = c.GetRenderFrame();

			auto current_frame = c.GetFrame();
			auto current_frame_index = c.GetFrameIndex();
			auto num_frames = c.GetNumFrames();

			command_buffer->Begin();

			// Descriptor set / Descriptor set layout
			CMShared<DescriptorPool> dp = common_resources->global_descriptor_pools[current_frame_index];

			const static uint32_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];
			auto shader_module_create_infos = std::vector<ShaderModuleCreateInfo>
			{
				ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("tone_map.vert")},
				ShaderModuleCreateInfo{ShaderModuleSourceCreateInfo::File("tone_map.frag")}
			};
			flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};
			variable_set_index_to_count[0] = max_variable_bindings;

			auto plci = PipelineLayoutCreateInfo
			{
				shader_module_create_infos,
				variable_set_index_to_count
			};

			CMShared<PipelineLayoutInfo> pli{};
			{
				ScopedCPUProfileRaysterizer("PipelineLayoutInfo Creation");
				pli = AssignOrPanic(c.Get(plci));
				c.SetName(pli, fmt::format("Pipeline layout info {}", current_frame_index));
			}

			CMShared<DescriptorSet> ds{};
			{
				ScopedCPUProfileRaysterizer("DescriptorSet Creation");
				ds = AssignOrPanic(render_frame.Get(DescriptorSetCreateInfo{ dp, pli }));
				c.SetName(ds, fmt::format("Descriptor set frame {}", current_frame_index));
			}

			CMShared<PipelineLayout> pl{};
			{
				ScopedCPUProfileRaysterizer("PipelineLayout Creation");
				pl = AssignOrPanic(c.Get(pli));
				c.SetName(pl, fmt::format("Pipeline layout frame {}", current_frame_index));
			}

			// Descriptor set binding
			{
				ScopedCPUProfileRaysterizer("Descriptor set binding");

				auto& g_buffer_pass = *common_resources->g_buffer_pass;

				{
					std::vector<vk::ClearValue> clear_values(4);

					clear_values[0].color.float32[0] = 0.0f;
					clear_values[0].color.float32[1] = 0.0f;
					clear_values[0].color.float32[2] = 0.0f;
					clear_values[0].color.float32[3] = 0.0f;

					clear_values[1].color.float32[0] = 0.0f;
					clear_values[1].color.float32[1] = 0.0f;
					clear_values[1].color.float32[2] = 0.0f;
					clear_values[1].color.float32[3] = 0.0f;

					clear_values[2].color.float32[0] = 0.0f;
					clear_values[2].color.float32[1] = 0.0f;
					clear_values[2].color.float32[2] = 0.0f;
					clear_values[2].color.float32[3] = -1.0f;

					clear_values[3].depthStencil.depth = 1.0f;

					auto frame_buffer = c.GetFrameCurrentBuffer();

					auto render_pass_begin_info = vk::RenderPassBeginInfo{}
						.setRenderPass(*render_pass)
						.setRenderArea(vk::Rect2D{}
							.setOffset({ 0, 0 })
							.setExtent(c.GetWindowExtent())
						)
						.setFramebuffer(frame_buffer)
						.setClearValues(clear_values);

					cb.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

					auto width = common_resources->width;
					auto height = common_resources->height;

					cb.setViewport(0, vk::Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f });
					cb.setScissor(0, vk::Rect2D(width, height));
				}

				{
					std::vector<WriteDescriptorSetBindedResource> read_ds;

					if (common_resources->temporal_aa_pass->Enabled() && 
						(common_resources->current_visualization_type != VISUALIZATION_TYPE_GROUND_TRUTH &&
							common_resources->current_visualization_type != VISUALIZATION_TYPE_NORMALS))
					{
						read_ds = common_resources->temporal_aa_pass->OutputBindings();
					}
					else
					{
						if (common_resources->current_visualization_type == VISUALIZATION_TYPE_FINAL)
						{
							read_ds = common_resources->deferred_shading_pass->OutputBindings();
						}
						else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_SHADOWS)
						{
							read_ds = common_resources->ray_traced_shadows_pass->OutputBindings();
						}
						else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_AMBIENT_OCCLUSION)
						{
							read_ds = common_resources->ray_traced_ao_pass->OutputBindings();
						}
						else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_REFLECTIONS)
						{
							read_ds = common_resources->ray_traced_reflections_pass->OutputBindings();
						}
						else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_GLOBAL_ILLUIMINATION)
						{
							read_ds = common_resources->ddgi_pass->OutputBindings();
						}
						else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_GROUND_TRUTH)
						{
							read_ds = { common_resources->g_buffer_pass->GetGBuffer1Texture() };
						}
						else if (common_resources->current_visualization_type == VISUALIZATION_TYPE_NORMALS)
						{
							read_ds = { common_resources->g_buffer_pass->GetGBuffer2Texture() };
						}
						else
						{
							PANIC("Not supported");
						}
					}
					for (auto& r : read_ds)
					{
						if (auto texture = std::get_if<CMShared<Texture>>(&r))
						{
							(*texture)->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
						}
					}

					PanicIfError(ds->Bind(0, 0, read_ds));

					PanicIfError(render_frame.FlushPendingWrites(ds));

					ToneMapPushConstants push_constants;

					push_constants.single_channel = (common_resources->current_visualization_type == VISUALIZATION_TYPE_SHADOWS || common_resources->current_visualization_type == VISUALIZATION_TYPE_AMBIENT_OCCLUSION) ? 1 : 0;
					push_constants.exposure = exposure;
					push_constants.visualization_type = common_resources->current_visualization_type;

					PanicIfError(render_frame.BindPushConstant(command_buffer, pl, PointerView(push_constants)));
				}
			}

			// Pipeline
			CMShared<GraphicsPipeline> gp{};
			{
				auto vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo{};

				auto pcbas = vk::PipelineColorBlendAttachmentState{}
					.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
					.setBlendEnable(VK_FALSE);

				std::vector<vk::PipelineColorBlendAttachmentState> pcbases{ pcbas };

				auto gpci = GraphicsPipelineCreateInfo
				{
					render_pass,
					pl,
					vertex_input_state_create_info,
					vk::PipelineTessellationStateCreateInfo{},
					vk::PipelineInputAssemblyStateCreateInfo{}
						.setTopology(vk::PrimitiveTopology::eTriangleList)
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
						.setLogicOpEnable(VK_FALSE)
						.setLogicOp(vk::LogicOp::eCopy)
						.setAttachments(pcbases),
					vk::PipelineMultisampleStateCreateInfo{}
						.setRasterizationSamples(vk::SampleCountFlagBits::e1)
						.setSampleShadingEnable(VK_FALSE)
						.setMinSampleShading(1.0f)
						.setPSampleMask(nullptr)
						.setAlphaToCoverageEnable(VK_FALSE)
						.setAlphaToOneEnable(VK_FALSE),
					vk::PipelineDepthStencilStateCreateInfo{}
						.setDepthTestEnable(VK_TRUE)
						.setDepthWriteEnable(VK_TRUE)
						.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
						.setBack(vk::StencilOpState{}.setCompareOp(vk::CompareOp::eAlways))
						.setDepthBoundsTestEnable(VK_FALSE)
						.setMinDepthBounds(0.0f)
						.setMaxDepthBounds(1.0f)
						.setStencilTestEnable(VK_FALSE),
					vk::PipelineDynamicStateCreateInfo{},
					vk::PipelineCache{}
				};

				gpci.viewport
					.setY(gpci.viewport.height - gpci.viewport.y)
					.setHeight(-gpci.viewport.height);

				gpci.pipeline_rasterization_state_create_info
					.setCullMode(vk::CullModeFlagBits::eNone)
					.setFrontFace(vk::FrontFace::eClockwise);

				{
					ScopedCPUProfileRaysterizer("GraphicsPipeline Creation");
					gp = AssignOrPanic(c.Get(gpci));
					c.SetName(gp, fmt::format("GraphicsPipeline {}", current_frame_index));
				}
			}

			cb.bindPipeline(vk::PipelineBindPoint::eGraphics, gp->pipeline);
			cb.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pl->pipeline_layout, 0, ds->descriptor_sets, {});

			cb.draw(3, 1, 0, 0);

			cb.endRenderPass();

			{
				auto subresource_range = vk::ImageSubresourceRange{}
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setBaseMipLevel(0)
					.setLevelCount(GBUFFER_MIP_LEVELS)
					.setBaseArrayLayer(0)
					.setLayerCount(1);

				subresource_range.aspectMask = vk::ImageAspectFlagBits::eDepth;
				//PanicIfError(command_buffer->SetImageLayout(common_resources->g_buffer_pass->GetDepthTexture()->image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal, subresource_range));
			}
		}

		void ToneMapPass::UpdateGui()
		{
			ImGui::InputFloat("Exposure", &exposure);
		}
	}
}