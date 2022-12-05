#include "include/shader_binding_table.h"

namespace RaysterizerEngine
{
	void ShaderBindingTable::Reset()
	{
		raygen_programs.clear();
		miss_programs.clear();
		hit_groups.clear();
		callable_shaders.clear();

		raygen_offset = 0;
		raygen_stride = 0;

		miss_offset = 0;
		miss_stride = 0;

		hit_group_offset = 0;
		hit_group_stride = 0;

		callable_shader_offset = 0;
		callable_shader_stride = 0;
	}

	void ShaderBindingTable::AddRayGenProgram(uint32_t group_index, PointerView data)
	{
		std::vector<uint8_t> data_vec{};
		if (data.Valid())
		{
			data_vec = data.ExtractData();
		}
		raygen_programs.emplace_back(ShaderTableEntry{ group_index, data_vec });
	}

	void ShaderBindingTable::AddMissProgram(uint32_t group_index, PointerView data)
	{
		std::vector<uint8_t> data_vec{};
		if (data.Valid())
		{
			data_vec = data.ExtractData();
		}
		miss_programs.emplace_back(ShaderTableEntry{ group_index, data_vec });
	}

	void ShaderBindingTable::AddHitGroup(uint32_t group_index, PointerView data)
	{
		std::vector<uint8_t> data_vec{};
		if (data.Valid())
		{
			data_vec = data.ExtractData();
		}
		hit_groups.emplace_back(ShaderTableEntry{ group_index, data_vec });
	}

	void ShaderBindingTable::AddCallableShader(uint32_t group_index, PointerView data)
	{
		std::vector<uint8_t> data_vec{};
		if (data.Valid())
		{
			data_vec = data.ExtractData();
		}
		callable_shaders.emplace_back(ShaderTableEntry{ group_index, data_vec });
	}

	std::vector<uint8_t> ShaderBindingTable::GetHandleBytes(vk::Device device, const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR properties, CMShared<PipelineLayout> pl, CMShared<RaytracingPipeline> pipeline)
	{
		this->properties = properties;

		const auto& raytracing_shader_group_create_infos = pl->pipeline_layout_info->raytracing_shader_group_create_infos;
		const auto& shader_modules = pl->pipeline_layout_info->shader_modules;

		auto index = 0;
		if (raytracing_shader_group_create_infos)
		{
			/*
			if (shader_modules.size() != raytracing_shader_group_create_infos->size())
			{
				PANIC("Expected shader modules to be same as create info");
			}
			*/
			for (auto i = 0; i < raytracing_shader_group_create_infos->size(); i++)
			{
				//const auto& shader_module = shader_modules[i];
				const auto& raytracing_shader_group_create_info_with_stage = (*raytracing_shader_group_create_infos)[i];
				const auto& raytracing_shader_group_create_info = raytracing_shader_group_create_info_with_stage.raytracing_shader_group_create_info;
				auto stage = raytracing_shader_group_create_info_with_stage.shader_stage_flags;

				if (stage & vk::ShaderStageFlagBits::eRaygenKHR)
				{
					AddRayGenProgram(i);
					if (raytracing_shader_group_create_info.generalShader == VK_SHADER_UNUSED_KHR)
					{
						PANIC("Expected general shader");
					}
				}
				else if (stage & vk::ShaderStageFlagBits::eMissKHR)
				{
					AddMissProgram(i);
					if (raytracing_shader_group_create_info.generalShader == VK_SHADER_UNUSED_KHR)
					{
						PANIC("Expected general shader");
					}
				}
				else if (stage & vk::ShaderStageFlagBits::eClosestHitKHR)
				{
					AddHitGroup(i);
					if (raytracing_shader_group_create_info.closestHitShader == VK_SHADER_UNUSED_KHR)
					{
						PANIC("Expected closest hit shader");
					}
				}
				else if (stage & vk::ShaderStageFlagBits::eAnyHitKHR)
				{
					AddHitGroup(i);
					if (raytracing_shader_group_create_info.anyHitShader == VK_SHADER_UNUSED_KHR)
					{
						PANIC("Expected any hit shader");
					}
				}
				else if (stage & vk::ShaderStageFlagBits::eIntersectionKHR)
				{
					AddHitGroup(i);
					if (raytracing_shader_group_create_info.intersectionShader == VK_SHADER_UNUSED_KHR)
					{
						PANIC("Expected intersection shader");
					}
				}
				else
				{
					PANIC("Expected raytracing shader stage flags");
				}
			}
		}

		auto [raygen_entry_size, miss_entry_size, hit_groups_size, callable_shaders_size, sbt_size] = ComputeShaderTableSize();

		auto shader_group_handle_size = properties.shaderGroupHandleSize;
		auto shader_group_alignment_size = properties.shaderGroupBaseAlignment;
		const uint32_t group_count = raygen_programs.size() + miss_programs.size() + hit_groups.size() + callable_shaders.size();
		const auto handle_sizes = shader_group_handle_size * group_count;
		std::vector<uint8_t> handles(handle_sizes);

		auto result = device.getRayTracingShaderGroupHandlesKHR(**pipeline, 0, group_count, handle_sizes, handles.data());
		assert(result == vk::Result::eSuccess);

		// write data into buffer
		vk::DeviceSize offset{};
		std::vector<uint8_t> arranged_handles(sbt_size);
		auto* arranged_handles_ptr = arranged_handles.data();

		auto CopyShaderData = [&](const std::vector<ShaderTableEntry>& shader_table_entry, uint32_t shader_table_entry_size)
		{
			for (auto& shader : shader_table_entry)
			{
				memcpy(arranged_handles_ptr, handles.data() + shader_group_handle_size * shader.group_index, shader_group_handle_size);

				if (!shader.data.empty())
				{
					//TODO: this is not actually correct -- need to memcpy the data with args
					memcpy(arranged_handles_ptr + shader_group_alignment_size, shader.data.data(), shader.data.size());
				}

				arranged_handles_ptr += shader_table_entry_size;
				offset = arranged_handles_ptr - arranged_handles.data();
			}
		};

		raygen_stride = raygen_entry_size;
		miss_stride = miss_entry_size;
		hit_group_stride = hit_groups_size;

		raygen_size = raygen_entry_size * raygen_programs.size();
		miss_size = miss_entry_size * miss_programs.size();
		hit_group_size = hit_groups_size * hit_groups.size();
		callable_shader_stride = callable_shaders_size;

		raygen_offset = offset;
		CopyShaderData(raygen_programs, raygen_entry_size);
		miss_offset = offset;
		CopyShaderData(miss_programs, miss_entry_size);
		hit_group_offset = offset;
		CopyShaderData(hit_groups, hit_groups_size);
		callable_shader_offset = offset;
		CopyShaderData(callable_shaders, callable_shaders_size);
		if (callable_shaders.size() == 0)
		{
			callable_shader_offset = 0;
			callable_shader_stride = 0;
		}

		return arranged_handles;
	}

	std::vector<uint8_t> ShaderBindingTable::GetHandleBytes(vk::Device& device, const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& properties, vk::Pipeline& pipeline)
	{
		this->properties = properties;

		auto [raygen_entry_size, miss_entry_size, hit_groups_size, callable_shaders_size, sbt_size] = ComputeShaderTableSize();

		auto shader_group_handle_size = properties.shaderGroupHandleSize;
		auto shader_group_alignment_size = properties.shaderGroupBaseAlignment;
		const uint32_t group_count = raygen_programs.size() + miss_programs.size() + hit_groups.size() + callable_shaders.size();
		const auto handle_sizes = shader_group_handle_size * group_count;
		std::vector<uint8_t> handles(handle_sizes);

		auto result = device.getRayTracingShaderGroupHandlesKHR(pipeline, 0, group_count, handle_sizes, handles.data());
		assert(result == vk::Result::eSuccess);

		// write data into buffer
		vk::DeviceSize offset{};
		std::vector<uint8_t> arranged_handles(sbt_size);
		auto* arranged_handles_ptr = arranged_handles.data();

		auto CopyShaderData = [&](const std::vector<ShaderTableEntry>& shader_table_entry, uint32_t shader_table_entry_size)
		{
			for (auto& shader : shader_table_entry)
			{
				memcpy(arranged_handles_ptr, handles.data() + shader_group_handle_size * shader.group_index, shader_group_handle_size);

				if (!shader.data.empty())
				{
					//TODO: this is not actually correct -- need to memcpy the data with args
					memcpy(arranged_handles_ptr + shader_group_alignment_size, shader.data.data(), shader.data.size());
				}

				arranged_handles_ptr += shader_table_entry_size;
				offset = arranged_handles_ptr - arranged_handles.data();
			}
		};

		raygen_stride = raygen_entry_size;
		miss_stride = miss_entry_size;
		hit_group_stride = hit_groups_size;

		raygen_size = raygen_entry_size * raygen_programs.size();
		miss_size = miss_entry_size * miss_programs.size();
		hit_group_size = hit_groups_size * hit_groups.size();
		callable_shader_stride = callable_shaders_size;

		raygen_offset = offset;
		CopyShaderData(raygen_programs, raygen_entry_size);
		miss_offset = offset;
		CopyShaderData(miss_programs, miss_entry_size);
		hit_group_offset = offset;
		CopyShaderData(hit_groups, hit_groups_size);
		callable_shader_offset = offset;
		CopyShaderData(callable_shaders, callable_shaders_size);
		if (callable_shaders.size() == 0)
		{
			callable_shader_offset = 0;
			callable_shader_stride = 0;
		}

		return arranged_handles;
	}

	void ShaderBindingTable::Generate(vk::Device& device, const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& properties, vk::Pipeline& pipeline)
	{
		auto arranged_handles = GetHandleBytes(device, properties, pipeline);
		/*
		auto buffer = Raysterizer::Vulkan::CreateBufferFromData(arranged_handles,
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		*/
	}

	std::tuple<vk::DeviceSize, vk::DeviceSize, vk::DeviceSize, vk::DeviceSize, vk::DeviceSize> ShaderBindingTable::ComputeShaderTableSize()
	{
		// Compute the entry size of each program type depending on the maximum number of parameters in each category
		auto raygen_entry_size = ComputeShaderTableEntrySize(raygen_programs);
		auto miss_entry_size = ComputeShaderTableEntrySize(miss_programs);
		auto hit_groups_size = ComputeShaderTableEntrySize(hit_groups);
		auto callable_shaders_size = ComputeShaderTableEntrySize(hit_groups);

		// The total SBT size is the sum of the entries for ray generation, miss and hit groups
		auto sbt_size = raygen_entry_size * static_cast<vk::DeviceSize>(raygen_programs.size())
			+ miss_entry_size * static_cast<vk::DeviceSize>(miss_programs.size())
			+ hit_groups_size * static_cast<vk::DeviceSize>(hit_groups.size());
		return { raygen_entry_size, miss_entry_size, hit_groups_size, callable_shaders_size, sbt_size };
	}

	vk::DeviceSize ShaderBindingTable::ComputeShaderTableEntrySize(const std::vector<ShaderTableEntry>& shader_table_entry)
	{
		// Find the maximum number of parameters used by a single entry
		std::size_t max_args = 0;
		for (const auto& shader : shader_table_entry)
		{
			max_args = std::max(max_args, shader.data.size());
		}

		// A SBT entry is made of a program ID and a set of 4-byte parameters (offsets or push constants)
		vk::DeviceSize entry_size = properties.shaderGroupHandleSize + static_cast<vk::DeviceSize>(max_args);

		// The entries of the shader binding table must be 16-bytes-aligned
		//entry_size = Raysterizer::Util::AlignUp(entry_size, 0x10);
		entry_size = Util::AlignUp(entry_size, properties.shaderGroupBaseAlignment);
		//entry_size = Raysterizer::Util::AlignUp(entry_size, properties.shaderGroupHandleAlignment);

		return entry_size;
	}
}