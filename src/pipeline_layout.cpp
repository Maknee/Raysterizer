#include "include/pipeline_layout.h"

namespace RaysterizerEngine
{
	Expected<DescriptorSetLayoutCreateInfos> PipelineLayoutCreateInfo::BuildDescriptorSetLayoutCreateInfos(const ShaderReflection& combined_shader_reflection) const
	{
		DescriptorSetLayoutCreateInfos descriptor_set_layout_create_infos{};

		const static std::size_t max_variable_bindings = Config["raysterizer"]["vulkan"]["max_variable_bindings"];

		flat_hash_map<uint32_t, uint32_t> set_to_last_binding;
		for (const auto& [set, descriptors] : combined_shader_reflection.sets)
		{
			auto SetLastBinding = [&, set = set](const auto& vs)
			{
				for (const auto& v : vs)
				{
					auto& b = set_to_last_binding[set];
					b = std::max(v.binding, b);
				}
			};
			SetLastBinding(descriptors.uniform_buffers);
			SetLastBinding(descriptors.storage_buffers);
			SetLastBinding(descriptors.texel_buffers);
			SetLastBinding(descriptors.samplers);
			SetLastBinding(descriptors.storage_images);
			SetLastBinding(descriptors.subpass_inputs);
			SetLastBinding(descriptors.acceleration_structures);
		}

		//std::map<uint32_t, uint32_t> variable_set_index_to_count(std::begin(variable_set_index_to_count), std::end(variable_set_index_to_count));
		for (const auto& [set, descriptors] : combined_shader_reflection.sets)
		{
			auto EnableVariableBindingIfLastBinding = [&, set = set](const auto& v)
			{
				if (variable_set_index_to_count.empty())
				{
					return;
				}
				if (auto set_to_last_binding_found = set_to_last_binding.find(set); set_to_last_binding_found != std::end(set_to_last_binding))
				{
					auto last_binding = set_to_last_binding_found->second;
					if (v.binding == last_binding)
					{
						if (auto found = variable_set_index_to_count.find(set); found != std::end(variable_set_index_to_count))
						{
							auto count = found->second;
							if (count > max_variable_bindings)
							{
								PANIC("MAX VARIABLE BINDINGS OUT OF BOUNDS {} > {}", max_variable_bindings, count);
							}
							descriptor_set_layout_create_infos.EnableVariableBindings(set, v.binding, count);
						}
						else
						{
							PANIC("Variable set index not set for {}", set);
						}
					}
				}
			};

			for (auto& ub : descriptors.uniform_buffers) {
				descriptor_set_layout_create_infos.AddBinding(set, ub.binding, max_variable_bindings, vk::DescriptorType::eUniformBuffer, ub.stage);
				if (ub.array_size > 1 || ub.array_size == -1)
				{
					EnableVariableBindingIfLastBinding(ub);
				}
			}

			for (auto& sb : descriptors.storage_buffers) {
				descriptor_set_layout_create_infos.AddBinding(set, sb.binding, max_variable_bindings, vk::DescriptorType::eStorageBuffer, sb.stage);
				if (sb.array_size > 1 || sb.array_size == -1)
				{
					EnableVariableBindingIfLastBinding(sb);
				}
			}

			for (auto& tb : descriptors.texel_buffers) {
				descriptor_set_layout_create_infos.AddBinding(set, tb.binding, max_variable_bindings, vk::DescriptorType::eUniformTexelBuffer, tb.stage);
				EnableVariableBindingIfLastBinding(tb);
			}

			for (auto& si : descriptors.samplers) {
				auto count = si.array_size == (unsigned)-1 ? max_variable_bindings : si.array_size;
				descriptor_set_layout_create_infos.AddBinding(set, si.binding, count, vk::DescriptorType::eCombinedImageSampler, si.stage);
				if (si.array_size == 0) {
					EnableVariableBindingIfLastBinding(si);
				}
			}

			for (auto& si : descriptors.storage_images) {
				auto count = si.array_size == (unsigned)-1 ? max_variable_bindings : si.array_size;
				descriptor_set_layout_create_infos.AddBinding(set, si.binding, count, vk::DescriptorType::eStorageImage, si.stage);
				if (si.array_size == 0) {
					EnableVariableBindingIfLastBinding(si);
				}
			}

			for (auto& si : descriptors.subpass_inputs) {
				descriptor_set_layout_create_infos.AddBinding(set, si.binding, max_variable_bindings, vk::DescriptorType::eInputAttachment, si.stage);
				EnableVariableBindingIfLastBinding(si);
			}

			for (auto& si : descriptors.acceleration_structures) {
				descriptor_set_layout_create_infos.AddBinding(set, si.binding, max_variable_bindings, vk::DescriptorType::eAccelerationStructureKHR, si.stage);
				EnableVariableBindingIfLastBinding(si);
			}
		}

		return descriptor_set_layout_create_infos;
	}

	vk::PipelineLayoutCreateInfo PipelineLayoutInfo::GetPipelineLayoutCreateInfo()
	{
		auto pipeline_create_info = vk::PipelineLayoutCreateInfo{}
			.setPushConstantRanges(push_constant_ranges)
			.setSetLayouts(descriptor_set_layouts.GetDescriptorSetLayouts());

		return pipeline_create_info;
	}

	Error PipelineLayoutInfo::MergeRaytracingGroups(std::vector<std::pair<std::size_t, std::size_t>> indicies)
	{
		if (!raytracing_shader_group_create_infos)
		{
			return StringError("Raytracing groups not set");
		}

		auto& raytracing_shader_group_create_infos_items = *raytracing_shader_group_create_infos;

		std::map<std::size_t, RayTracingShaderGroupCreateInfoWithShaderStageFlags> raytracing_shader_group_create_infos_mapping;
		for (auto i = 0; i < raytracing_shader_group_create_infos_items.size(); i++)
		{
			const auto& raytracing_shader_group_create_info = raytracing_shader_group_create_infos_items[i];
			raytracing_shader_group_create_infos_mapping.emplace(i, raytracing_shader_group_create_info);
		}

		for (const auto& [i1, i2] : indicies)
		{
			if (auto found = raytracing_shader_group_create_infos_mapping.find(i1); found != std::end(raytracing_shader_group_create_infos_mapping))
			{
				auto& [index1, raytracing_shader_group_create_info1] = *found;
				if (auto found2 = raytracing_shader_group_create_infos_mapping.find(i2); found2 != std::end(raytracing_shader_group_create_infos_mapping))
				{
					auto& [index2, raytracing_shader_group_create_info2] = *found2;

					if (raytracing_shader_group_create_info2.raytracing_shader_group_create_info.closestHitShader != VK_SHADER_UNUSED_KHR)
					{
						raytracing_shader_group_create_info1.raytracing_shader_group_create_info.closestHitShader = raytracing_shader_group_create_info2.raytracing_shader_group_create_info.closestHitShader;
					}
					if (raytracing_shader_group_create_info2.raytracing_shader_group_create_info.anyHitShader != VK_SHADER_UNUSED_KHR)
					{
						raytracing_shader_group_create_info1.raytracing_shader_group_create_info.anyHitShader = raytracing_shader_group_create_info2.raytracing_shader_group_create_info.anyHitShader;
					}
					if (raytracing_shader_group_create_info2.raytracing_shader_group_create_info.intersectionShader != VK_SHADER_UNUSED_KHR)
					{
						raytracing_shader_group_create_info1.raytracing_shader_group_create_info.intersectionShader = raytracing_shader_group_create_info2.raytracing_shader_group_create_info.intersectionShader;
					}

					raytracing_shader_group_create_infos_mapping.erase(found2);
				}
				else
				{
					return StringError("Index not found {} -> {}", i1, i2);
				}
			}
			else
			{
				return StringError("Index not found {} -> {}", i1, i2);
			}
		}

		std::vector<RayTracingShaderGroupCreateInfoWithShaderStageFlags> new_raytracing_shader_group_create_infos;
		for (const auto& [i, raytracing_shader_group_create_info] : raytracing_shader_group_create_infos_mapping)
		{
			new_raytracing_shader_group_create_infos.emplace_back(raytracing_shader_group_create_info);
		}

		raytracing_shader_group_create_infos = new_raytracing_shader_group_create_infos;

		return NoError();
	}

	Expected<std::vector<vk::RayTracingShaderGroupCreateInfoKHR>> PipelineLayoutInfo::GetRaytracingShaderGroupCreateInfos() const
	{
		if (!raytracing_shader_group_create_infos)
		{
			return StringError("Raytracing groups not set");
		}

		const auto& raytracing_shader_group_create_infos_items = *raytracing_shader_group_create_infos;
		std::vector<vk::RayTracingShaderGroupCreateInfoKHR> vk_raytracing_shader_group_create_infos(raytracing_shader_group_create_infos_items.size());
		std::transform(std::begin(raytracing_shader_group_create_infos_items), std::end(raytracing_shader_group_create_infos_items), std::begin(vk_raytracing_shader_group_create_infos), [](const auto& e) { return e.raytracing_shader_group_create_info; });

		return vk_raytracing_shader_group_create_infos;
	}
}