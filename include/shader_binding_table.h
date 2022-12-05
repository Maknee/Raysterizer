#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct ShaderTableEntry
	{
		uint32_t group_index;
		std::vector<uint8_t> data;
	};

	class ShaderBindingTable
	{
	public:
		void Reset();
		void AddRayGenProgram(uint32_t group_index, PointerView data = PointerView{});
		void AddMissProgram(uint32_t group_index, PointerView data = PointerView{});
		void AddHitGroup(uint32_t group_index, PointerView data = PointerView{});
		void AddCallableShader(uint32_t group_index, PointerView data = PointerView{});

		std::vector<uint8_t> GetHandleBytes(vk::Device device, const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR properties, CMShared<PipelineLayout> pl, CMShared<RaytracingPipeline> pipeline);
		std::vector<uint8_t> GetHandleBytes(vk::Device& device, const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& properties, vk::Pipeline& pipeline);
		void Generate(vk::Device& device, const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& properties, vk::Pipeline& pipeline);

		vk::DeviceSize GetRaygenOffset() { return raygen_offset; }
		vk::DeviceSize GetRaygenStride() { return raygen_stride; }
		vk::DeviceSize GetRaygenSize() { return raygen_size; }

		vk::DeviceSize GetMissOffset() { return miss_offset; }
		vk::DeviceSize GetMissStride() { return miss_stride; }
		vk::DeviceSize GetMissSize() { return miss_size; }

		vk::DeviceSize GetHitGroupOffset() { return hit_group_offset; }
		vk::DeviceSize GetHitGroupStride() { return hit_group_stride; }
		vk::DeviceSize GetHitGroupSize() { return hit_group_size; }

		vk::DeviceSize GetCallableShaderOffset() { return callable_shader_offset; }
		vk::DeviceSize GetCallableShaderStride() { return callable_shader_stride; }

	private:
		vk::DeviceSize ComputeShaderTableEntrySize(const std::vector<ShaderTableEntry>& shader_table_entry);
		std::tuple<vk::DeviceSize, vk::DeviceSize, vk::DeviceSize, vk::DeviceSize, vk::DeviceSize> ComputeShaderTableSize();

		std::vector<ShaderTableEntry> raygen_programs;
		std::vector<ShaderTableEntry> miss_programs;
		std::vector<ShaderTableEntry> hit_groups;
		std::vector<ShaderTableEntry> callable_shaders;

		vk::DeviceSize raygen_offset{};
		vk::DeviceSize raygen_stride{};
		vk::DeviceSize raygen_size{};

		vk::DeviceSize miss_offset{};
		vk::DeviceSize miss_stride{};
		vk::DeviceSize miss_size{};

		vk::DeviceSize hit_group_offset{};
		vk::DeviceSize hit_group_stride{};
		vk::DeviceSize hit_group_size{};

		vk::DeviceSize callable_shader_offset{};
		vk::DeviceSize callable_shader_stride{};

		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR properties;
	};
}