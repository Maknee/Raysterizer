#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct DescriptorSetCreateInfo
	{
		CMShared<DescriptorPool> descriptor_pool;
		CMShared<PipelineLayoutInfo> pipeline_layout_info;

	public:
		bool operator==(const DescriptorSetCreateInfo& o) const noexcept {
			return CheckEquality(descriptor_pool, o.descriptor_pool) &&
				CheckEquality(pipeline_layout_info, o.pipeline_layout_info);
		}
	};

	using WriteDescriptorSetBindedResource = std::variant<
		CMShared<Buffer>, CMShared<Texture>, CMShared<Sampler>, CMShared<TopLevelAccelerationStructure>, 
		std::vector<CMShared<Buffer>>, std::vector<CMShared<Texture>>, std::vector<CMShared<Sampler>>, std::vector<CMShared<TopLevelAccelerationStructure>>
	>;
	using WriteDescriptorSetDescriptorInfo = std::variant<
		vk::DescriptorBufferInfo, vk::DescriptorImageInfo, vk::WriteDescriptorSetAccelerationStructureKHR,
		std::vector<vk::DescriptorBufferInfo>, std::vector<vk::DescriptorImageInfo>, std::vector<vk::WriteDescriptorSetAccelerationStructureKHR>
	>;
	struct WriteDescriptorSet
	{
		uint32_t set{};
		vk::WriteDescriptorSet write_descriptor_set{};
		WriteDescriptorSetBindedResource binded_resource{};

		// To be written (referenced by write_descriptor_set as a pointer)
		WriteDescriptorSetDescriptorInfo descriptor_info{};

		// Is this supposed to be written?
		bool pending_write{};
	public:
		Error SyncWriteDescriptorSetWithDescriptorInfo();

	public:
		bool operator==(const WriteDescriptorSet& o) const noexcept
		{
			bool same = set == o.set;

			if (auto buffer = std::get_if<CMShared<Buffer>>(&binded_resource))
			{
				if (auto o_buffer = std::get_if<CMShared<Buffer>>(&o.binded_resource))
				{
					if (auto descriptor_buffer_info = std::get_if<vk::DescriptorBufferInfo>(&descriptor_info))
					{
						if (auto o_descriptor_buffer_info = std::get_if<vk::DescriptorBufferInfo>(&o.descriptor_info))
						{
							same &= CheckEquality(*buffer, *o_buffer);
							same &= descriptor_buffer_info->buffer == o_descriptor_buffer_info->buffer;
							same &= descriptor_buffer_info->offset == o_descriptor_buffer_info->offset;
							same &= descriptor_buffer_info->range == o_descriptor_buffer_info->range;
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else if (auto texture = std::get_if<CMShared<Texture>>(&binded_resource))
			{
				if (auto o_texture = std::get_if<CMShared<Texture>>(&o.binded_resource))
				{
					same &= CheckEquality(*texture, *o_texture);
				}
				else
				{
					return false;
				}
			}
			else if (auto sampler = std::get_if<CMShared<Sampler>>(&binded_resource))
			{
				if (auto o_sampler = std::get_if<CMShared<Sampler>>(&o.binded_resource))
				{
					same &= CheckEquality(*sampler, *o_sampler);
				}
				else
				{
					return false;
				}
			}
			else if (auto tlas = std::get_if<CMShared<TopLevelAccelerationStructure>>(&binded_resource))
			{
				if (auto o_tlas = std::get_if<CMShared<TopLevelAccelerationStructure>>(&o.binded_resource))
				{
					same &= CheckEquality(*tlas, *o_tlas);
				}
				else
				{
					return false;
				}
			}
			else if (auto buffers = std::get_if<std::vector<CMShared<Buffer>>>(&binded_resource))
			{
				if (auto o_buffers = std::get_if<std::vector<CMShared<Buffer>>>(&o.binded_resource))
				{
					if (auto descriptor_buffer_infos = std::get_if<std::vector<vk::DescriptorBufferInfo>>(&descriptor_info))
					{
						if (auto o_descriptor_buffer_infos = std::get_if<std::vector<vk::DescriptorBufferInfo>>(&o.descriptor_info))
						{
							same &= CheckEquality(*buffers, *o_buffers);
							if (descriptor_buffer_infos->size() != o_descriptor_buffer_infos->size())
							{
								return false;
							}
							for (auto i = 0; i < descriptor_buffer_infos->size(); i++)
							{
								auto& descriptor_buffer_info = (*descriptor_buffer_infos)[i];
								auto& o_descriptor_buffer_info = (*o_descriptor_buffer_infos)[i];
								same &= descriptor_buffer_info.buffer == o_descriptor_buffer_info.buffer;
								same &= descriptor_buffer_info.offset == o_descriptor_buffer_info.offset;
								same &= descriptor_buffer_info.range == o_descriptor_buffer_info.range;
							}
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else if (auto textures = std::get_if<std::vector<CMShared<Texture>>>(&binded_resource))
			{
				if (auto o_textures = std::get_if<std::vector<CMShared<Texture>>>(&o.binded_resource))
				{
					same &= CheckEquality(*textures, *o_textures);
				}
				else
				{
					return false;
				}
			}
			else if (auto samplers = std::get_if<std::vector<CMShared<Sampler>>>(&binded_resource))
			{
				if (auto o_samplers = std::get_if<std::vector<CMShared<Sampler>>>(&o.binded_resource))
				{
					same &= CheckEquality(*samplers, *o_samplers);
				}
				else
				{
					return false;
				}
			}
			else if (auto tlases = std::get_if<std::vector<CMShared<TopLevelAccelerationStructure>>>(&binded_resource))
			{
				if (auto o_tlases = std::get_if<std::vector<CMShared<TopLevelAccelerationStructure>>>(&o.binded_resource))
				{
					same &= CheckEquality(*tlases, *o_tlases);
				}
				else
				{
					return false;
				}
			}
			else
			{
				PANIC("Couldn't hash binded resource, unknown resource");
			}
			return same;
		}
	};

	class DescriptorSet : public HasInUseOption
	{
	public:
		explicit DescriptorSet();
		explicit DescriptorSet(std::vector<vk::DescriptorSet> descriptor_sets_, DescriptorSetCreateInfo descriptor_set_create_info_);

	public:
		std::vector<vk::DescriptorSet> descriptor_sets{};
		DescriptorSetCreateInfo descriptor_set_create_info{};

		// Writes that need to be flushed to descriptor sets
		//std::vector<vk::WriteDescriptorSet> pending_write_descriptor_sets{};

		// Mapping [set -> binding -> write descriptor set]
		flat_hash_map<uint32_t, flat_hash_map<uint32_t, WriteDescriptorSet>> write_descriptor_sets{};
	public:
		Error InitializeWriteDescriptorSets();
		Error Bind(uint32_t set, uint32_t binding, CMShared<Buffer> buffer, vk::DeviceSize offset = 0, std::optional<vk::DeviceSize> range = std::nullopt);
		Error Bind(uint32_t set, uint32_t binding, CMShared<Texture> texture, CMShared<Sampler> sampler);
		Error Bind(uint32_t set, uint32_t binding, CMShared<Texture> texture, std::optional<vk::ImageLayout> image_layout = std::nullopt, std::optional<vk::DescriptorType> override_descriptor_type = std::nullopt);
		Error Bind(uint32_t set, uint32_t binding, CMShared<Sampler> sampler);
		Error Bind(uint32_t set, uint32_t binding, CMShared<TopLevelAccelerationStructure> tlas);
		Error Bind(uint32_t set, uint32_t binding, std::vector<CMShared<Buffer>> buffers);
		Error Bind(uint32_t set, uint32_t binding, std::vector<CMShared<Texture>> textures);
		Error Bind(uint32_t set, uint32_t binding, std::vector<CMShared<Sampler>> samplers);
		Error Bind(uint32_t set, uint32_t binding, std::vector<CMShared<TopLevelAccelerationStructure>> tlases);
		Error Bind(uint32_t set, uint32_t binding, WriteDescriptorSetBindedResource write_descriptor_resource);
		Error Bind(uint32_t set, uint32_t binding, std::vector<WriteDescriptorSetBindedResource> write_descriptor_resources);
		Error SetPendingWrite(uint32_t set, uint32_t binding, bool pending_write);

		Error FlushPendingWrites(vk::Device device);

		auto& GetWriteDescriptorSets() { return write_descriptor_sets; }
		const auto& GetWriteDescriptorSets() const { return write_descriptor_sets; }

	public:
		bool operator==(const DescriptorSet& o) const noexcept {
			return descriptor_sets == o.descriptor_sets;
		}

	private:
		Expected<vk::DescriptorType> GetDescriptorSetCorrespondingToSetAndBinding(uint32_t set, uint32_t binding);
		Error UpdateWriteDescriptorSet(uint32_t set, uint32_t binding, const WriteDescriptorSet& write_descriptor_set, const WriteDescriptorSetDescriptorInfo& write_descriptor_set_descriptor_info);
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<DescriptorSetCreateInfo>
	{
		size_t operator()(const DescriptorSetCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, StdHash(*o.descriptor_pool));
			HashCombine(h, StdHash(*o.pipeline_layout_info));
			return h;
		}
	};

	template<>
	struct hash<WriteDescriptorSetDescriptorInfo>
	{
		size_t operator()(const WriteDescriptorSetDescriptorInfo& o) const noexcept
		{
			size_t h{};
			if (auto descriptor_buffer_info = std::get_if<vk::DescriptorBufferInfo>(&o))
			{
				HashCombine(h, descriptor_buffer_info->buffer);
				HashCombine(h, descriptor_buffer_info->offset);
				HashCombine(h, descriptor_buffer_info->range);
			}
			else if (auto descriptor_image_info = std::get_if<vk::DescriptorImageInfo>(&o))
			{
			}
			else if (auto write_descriptor_set_acceleration_structure = std::get_if<vk::WriteDescriptorSetAccelerationStructureKHR>(&o))
			{
			}
			else if (auto descriptor_buffer_infos = std::get_if<std::vector<vk::DescriptorBufferInfo>>(&o))
			{
				for (auto i = 0; i < descriptor_buffer_infos->size(); i++)
				{
					auto& descriptor_buffer_info = (*descriptor_buffer_infos)[i];
					HashCombine(h, descriptor_buffer_info.buffer);
					HashCombine(h, descriptor_buffer_info.offset);
					HashCombine(h, descriptor_buffer_info.range);
				}
			}
			else if (auto descriptor_image_infos = std::get_if<std::vector<vk::DescriptorImageInfo>>(&o))
			{
			}
			else if (auto write_descriptor_set_acceleration_structures = std::get_if<std::vector<vk::WriteDescriptorSetAccelerationStructureKHR>>(&o))
			{
			}
			else
			{
				PANIC("Expected valid type");
			}
			return h;
		}
	};

	template<>
	struct hash<WriteDescriptorSet>
	{
		size_t operator()(const WriteDescriptorSet& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.set));
			HashCombine(h, Hash(o.write_descriptor_set.dstBinding));

			if (auto buffer = std::get_if<CMShared<Buffer>>(&o.binded_resource))
			{
				if (*buffer)
				{
					HashCombine(h, StdHash(*buffer));
				}
			}
			else if (auto texture = std::get_if<CMShared<Texture>>(&o.binded_resource))
			{
				if (*texture)
				{
					HashCombine(h, StdHash(*texture));
				}
			}
			else if (auto sampler = std::get_if<CMShared<Sampler>>(&o.binded_resource))
			{
				if (*sampler)
				{
					HashCombine(h, StdHash(*sampler));
				}
			}
			else if (auto tlas = std::get_if<CMShared<TopLevelAccelerationStructure>>(&o.binded_resource))
			{
				if (*tlas)
				{
					HashCombine(h, StdHash(*tlas));
				}
			}
			else if (auto buffers = std::get_if<std::vector<CMShared<Buffer>>>(&o.binded_resource))
			{
				std::for_each(std::begin(*buffers), std::end(*buffers), [&](const auto& e) { HashCombine(h, StdHash(e)); });
			}
			else if (auto textures = std::get_if<std::vector<CMShared<Texture>>>(&o.binded_resource))
			{
				std::for_each(std::begin(*textures), std::end(*textures), [&](const auto& e) { HashCombine(h, StdHash(e)); });
			}
			else if (auto samplers = std::get_if<std::vector<CMShared<Sampler>>>(&o.binded_resource))
			{
				std::for_each(std::begin(*samplers), std::end(*samplers), [&](const auto& e) { HashCombine(h, StdHash(e)); });
			}
			else if (auto tlases = std::get_if<std::vector<CMShared<TopLevelAccelerationStructure>>>(&o.binded_resource))
			{
				std::for_each(std::begin(*tlases), std::end(*tlases), [&](const auto& e) { HashCombine(h, StdHash(e)); });
			}
			else
			{
				PANIC("Couldn't hash binded resource, unknown resource");
			}
			HashCombine(h, StdHash(o.descriptor_info));
			return h;
		}
	};
}