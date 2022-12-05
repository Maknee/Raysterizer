#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct GraphicsPipelineCreateInfo
	{
		CMShared<RenderPass> render_pass{};
		CMShared<PipelineLayout> pipeline_layout{};
		vk::PipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
		vk::PipelineTessellationStateCreateInfo pipeline_tessellation_state_create_info{};
		vk::PipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
		vk::Viewport viewport;
		vk::Rect2D scissor;
		vk::PipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
		vk::PipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
		vk::PipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
		vk::PipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
		vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
		vk::PipelineCache pipeline_cache{};
		void* pipeline_viewport_state_create_info_next{};

	public:
		bool operator==(const GraphicsPipelineCreateInfo& o) const noexcept {
			/*
			if (pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount != o.pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount)
			{
				return false;
			}
			for (auto i = 0; i < pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount; i++)
			{
				if (pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions[i] != o.pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions[i])
				{
					return false;
				}
			}

			if (pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount != o.pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount)
			{
				return false;
			}
			for (auto i = 0; i < pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount; i++)
			{
				if (pipeline_vertex_input_state_create_info.pVertexBindingDescriptions[i] != o.pipeline_vertex_input_state_create_info.pVertexBindingDescriptions[i])
				{
					return false;
				}
			}

			if (pipeline_color_blend_state_create_info.attachmentCount != o.pipeline_color_blend_state_create_info.attachmentCount)
			{
				return false;
			}
			for (auto i = 0; i < pipeline_color_blend_state_create_info.attachmentCount; i++)
			{
				if (pipeline_color_blend_state_create_info.pAttachments[i] != o.pipeline_color_blend_state_create_info.pAttachments[i])
				{
					return false;
				}
			}
			*/

			// TODO: remove hack...
			/*
			if (!CALL_WITH_SEH_RETURN({
					CheckEquality(render_pass, o.render_pass);
				}))
			{
				return false;
			}
			*/

			return CheckEquality(render_pass, o.render_pass) &&
				CheckEquality(pipeline_layout, o.pipeline_layout) &&
				//pipeline_vertex_input_state_create_info == o.pipeline_vertex_input_state_create_info &&
				pipeline_input_assembly_state_create_info == o.pipeline_input_assembly_state_create_info &&
				pipeline_tessellation_state_create_info == o.pipeline_tessellation_state_create_info &&
				viewport == o.viewport &&
				scissor == o.scissor &&
				pipeline_rasterization_state_create_info == o.pipeline_rasterization_state_create_info &&
				//pipeline_color_blend_state_create_info == o.pipeline_color_blend_state_create_info &&
				pipeline_multisample_state_create_info == o.pipeline_multisample_state_create_info &&
				pipeline_depth_stencil_state_create_info == o.pipeline_depth_stencil_state_create_info &&
				//pipeline_dynamic_state_create_info == o.pipeline_dynamic_state_create_info &&
				pipeline_cache == o.pipeline_cache;
		}
	};

	struct GraphicsPipeline
	{
		vk::Pipeline pipeline{};
		GraphicsPipelineCreateInfo graphics_pipeline_create_info{};

	public:
		vk::Pipeline operator*() const
		{
			return pipeline;
		}

		operator vk::Pipeline() noexcept
		{
			return pipeline;
		}

	public:
		bool operator==(const GraphicsPipeline& o) const noexcept {
			return pipeline == o.pipeline;
			//return graphics_pipeline_create_info == o.graphics_pipeline_create_info;
		}
	};

	struct ComputePipelineCreateInfo
	{
		CMShared<PipelineLayout> pipeline_layout{};
		vk::PipelineCache pipeline_cache{};

	public:
		bool operator==(const ComputePipelineCreateInfo& o) const noexcept {
			return CheckEquality(pipeline_layout, o.pipeline_layout) &&
				pipeline_cache == o.pipeline_cache;
		}
	};

	struct ComputePipeline
	{
		vk::Pipeline pipeline{};
		ComputePipelineCreateInfo compute_pipeline_create_info{};

	public:
		vk::Pipeline operator*() const
		{
			return pipeline;
		}

		operator vk::Pipeline() noexcept
		{
			return pipeline;
		}

	public:
		bool operator==(const ComputePipeline& other) const noexcept {
			return pipeline == other.pipeline;
		}
	};

	struct RaytracingPipelineCreateInfo
	{
		CMShared<PipelineLayout> pipeline_layout{};
		uint32_t recursion_depth{};
		vk::DeferredOperationKHR deferred_operation{};
		vk::PipelineCache pipeline_cache{};

	public:
		bool operator==(const RaytracingPipelineCreateInfo& o) const noexcept {
			return CheckEquality(pipeline_layout, o.pipeline_layout) &&
				recursion_depth == o.recursion_depth &&
				deferred_operation == o.deferred_operation &&
				pipeline_cache == o.pipeline_cache;
		}
	};

	struct RaytracingPipeline
	{
		vk::Pipeline pipeline{};
		RaytracingPipelineCreateInfo raytracing_pipeline_create_info{};

	public:
		vk::Pipeline operator*() const
		{
			return pipeline;
		}

		operator vk::Pipeline() noexcept
		{
			return pipeline;
		}

	public:
		bool operator==(const RaytracingPipeline& other) const noexcept {
			return pipeline == other.pipeline;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<GraphicsPipelineCreateInfo>
	{
		size_t operator()(const GraphicsPipelineCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, StdHash(*o.render_pass));
			HashCombine(h, StdHash(*o.pipeline_layout));
			const auto& pipeline_vertex_input_state_create_info = o.pipeline_vertex_input_state_create_info;
			for (auto i = 0; i < pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount; i++)
			{
				const auto& v = pipeline_vertex_input_state_create_info.pVertexBindingDescriptions[i];
				HashCombine(h, Hash(v));
			}
			for (auto i = 0; i < pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount; i++)
			{
				const auto& v = pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions[i];
				HashCombine(h, Hash(v));
			}
			HashCombine(h, Hash(o.pipeline_input_assembly_state_create_info));
			HashCombine(h, Hash(o.pipeline_tessellation_state_create_info));
			HashCombine(h, Hash(o.viewport));
			HashCombine(h, Hash(o.scissor));
			HashCombine(h, Hash(o.pipeline_rasterization_state_create_info));
			const auto& pipeline_color_blend_state_create_info = o.pipeline_color_blend_state_create_info;
			for (auto i = 0; i < pipeline_color_blend_state_create_info.attachmentCount; i++)
			{
				HashCombine(h, Hash(pipeline_color_blend_state_create_info.pAttachments[i]));
			}
			HashCombine(h, Hash(o.pipeline_multisample_state_create_info));
			HashCombine(h, Hash(o.pipeline_depth_stencil_state_create_info));
			const auto& pipeline_dynamic_state_create_info = o.pipeline_dynamic_state_create_info;
			for (auto i = 0; i < pipeline_dynamic_state_create_info.dynamicStateCount; i++)
			{
				HashCombine(h, Hash(pipeline_dynamic_state_create_info.pDynamicStates[i]));
			}
			HashCombine(h, Hash(o.pipeline_cache));
			return h;
		}
	};

	template<>
	struct hash<ComputePipelineCreateInfo>
	{
		size_t operator()(const ComputePipelineCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, StdHash(*o.pipeline_layout));
			HashCombine(h, Hash(o.pipeline_cache));
			return h;
		}
	};

	template<>
	struct hash<RaytracingPipelineCreateInfo>
	{
		size_t operator()(const RaytracingPipelineCreateInfo& o) const noexcept
		{
			size_t h{};
			if (o.pipeline_layout)
			{
				HashCombine(h, StdHash(*o.pipeline_layout));
			}
			HashCombine(h, Hash(o.recursion_depth));
			HashCombine(h, Hash(o.deferred_operation));
			HashCombine(h, Hash(o.pipeline_cache));
			return h;
		}
	};
}