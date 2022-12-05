#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct PipelineLayoutCreateInfo
	{
		std::vector<ShaderModuleCreateInfo> shader_module_create_infos{};
		flat_hash_map<uint32_t, uint32_t> variable_set_index_to_count{};

		void SetVariableSetIndexCount(uint32_t set, uint32_t count)
		{
			variable_set_index_to_count[set] = count;
		}

		Expected<DescriptorSetLayoutCreateInfos> BuildDescriptorSetLayoutCreateInfos(const ShaderReflection& combined_shader_reflection) const;
	public:
		bool operator==(const PipelineLayoutCreateInfo& o) const
		{
			return shader_module_create_infos == o.shader_module_create_infos && variable_set_index_to_count == o.variable_set_index_to_count;
		}
	};

	struct RayTracingShaderGroupCreateInfoWithShaderStageFlags
	{
		vk::RayTracingShaderGroupCreateInfoKHR raytracing_shader_group_create_info{};
		vk::ShaderStageFlags shader_stage_flags{};

	public:
		bool operator==(const RayTracingShaderGroupCreateInfoWithShaderStageFlags& o) const noexcept
		{
			return raytracing_shader_group_create_info == o.raytracing_shader_group_create_info &&
				shader_stage_flags == o.shader_stage_flags;
		}
	};

	struct PipelineLayoutInfo
	{
		std::vector<vk::PushConstantRange> push_constant_ranges{};
		DescriptorSetLayouts descriptor_set_layouts{};
		std::vector<CMShared<ShaderModule>> shader_modules{};
		ShaderReflection combined_shader_reflection{};
		std::vector<vk::PipelineShaderStageCreateInfo> pipeline_shader_stages{};

		vk::PipelineLayoutCreateInfo pipeline_create_info{};

		// Raytracing
		std::optional<std::vector<RayTracingShaderGroupCreateInfoWithShaderStageFlags>> raytracing_shader_group_create_infos{};

	public:
		vk::PipelineLayoutCreateInfo GetPipelineLayoutCreateInfo();
		Error MergeRaytracingGroups(std::vector<std::pair<std::size_t, std::size_t>> indicies);
		Expected<std::vector<vk::RayTracingShaderGroupCreateInfoKHR>> GetRaytracingShaderGroupCreateInfos() const;
	public:
		bool operator==(const PipelineLayoutInfo& o) const noexcept
		{
			return push_constant_ranges == o.push_constant_ranges && 
				descriptor_set_layouts == o.descriptor_set_layouts &&
				CheckEquality(shader_modules, o.shader_modules) &&
				//pipeline_shader_stages == o.pipeline_shader_stages &&
				raytracing_shader_group_create_infos == o.raytracing_shader_group_create_infos;
		}
	};

	struct PipelineLayout
	{
		vk::PipelineLayout pipeline_layout{};
		CMShared<PipelineLayoutInfo> pipeline_layout_info{};

	public:
		vk::PipelineLayout operator*() const
		{
			return pipeline_layout;
		}

		operator vk::PipelineLayout() noexcept
		{
			return pipeline_layout;
		}

	public:
		bool operator==(const PipelineLayout& o) const noexcept
		{
			return pipeline_layout == o.pipeline_layout && CheckEquality(pipeline_layout_info, o.pipeline_layout_info);
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<PipelineLayoutCreateInfo>
	{
		size_t operator()(const PipelineLayoutCreateInfo& o) const noexcept
		{
			size_t h{};
			for (const auto& shader_module_create_info : o.shader_module_create_infos)
			{
				HashCombine(h, StdHash(shader_module_create_info));
			}
			for (const auto& [set_index, count] : o.variable_set_index_to_count)
			{
				HashCombine(h, Hash(set_index, count));
			}
			return h;
		}
	};

	template<>
	struct hash<PipelineLayoutInfo>
	{
		size_t operator()(const PipelineLayoutInfo& o) const noexcept
		{
			size_t h{};
			for (const auto& push_constant_range : o.push_constant_ranges)
			{
				HashCombine(h, Hash(push_constant_range));
			}
			HashCombine(h, StdHash(o.descriptor_set_layouts));
			for (const auto& shader_module : o.shader_modules)
			{
				HashCombine(h, StdHash(*shader_module));
			}
			/*
			for (const auto& pipeline_shader_stage : o.pipeline_shader_stages)
			{
				HashCombine(h, Hash(pipeline_shader_stage));
			}
			*/
			//HashCombine(h, Hash(o.pipeline_create_info));
			if (o.raytracing_shader_group_create_infos)
			{
				for (const auto& raytracing_shader_group_create_info : *o.raytracing_shader_group_create_infos)
				{
					HashCombine(h, Hash(raytracing_shader_group_create_info));
				}
			}
			return h;
		}
	};
	
	template<>
	struct hash<PipelineLayout>
	{
		size_t operator()(const PipelineLayout& o) const noexcept
		{
			size_t h{};
			//HashCombine(h, Hash(o.pipeline_layout));
			if (o.pipeline_layout_info)
			{
				HashCombine(h, StdHash(*o.pipeline_layout_info));
			}
			return h;
		}
	};
}