#include "include/descriptor_set.h"

namespace RaysterizerEngine
{
	Error WriteDescriptorSet::SyncWriteDescriptorSetWithDescriptorInfo()
	{
		if (auto buffer = std::get_if<CMShared<Buffer>>(&binded_resource))
		{
			if (auto descriptor_buffer_info = std::get_if<vk::DescriptorBufferInfo>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(1);
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPBufferInfo(descriptor_buffer_info);
				write_descriptor_set.setPNext(nullptr);
			}
			else
			{
				return StringError("Expected descriptor buffer info");
			}
		}
		else if (auto texture = std::get_if<CMShared<Texture>>(&binded_resource))
		{
			if (auto image_descriptor_info = std::get_if<vk::DescriptorImageInfo>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(1);
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPImageInfo(image_descriptor_info);
				write_descriptor_set.setPNext(nullptr);
			}
			else
			{
				return StringError("Expected descriptor image info");
			}
		}
		else if (auto sampler = std::get_if<CMShared<Sampler>>(&binded_resource))
		{
			if (auto image_descriptor_info = std::get_if<vk::DescriptorImageInfo>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(1);
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPImageInfo(image_descriptor_info);
				write_descriptor_set.setPNext(nullptr);
			}
			else
			{
				return StringError("Expected descriptor image info");
			}
		}
		else if (auto tlas = std::get_if<CMShared<TopLevelAccelerationStructure>>(&binded_resource))
		{
			if (auto write_descriptor_set_acceleration_structure = std::get_if<vk::WriteDescriptorSetAccelerationStructureKHR>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(1);
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPImageInfo(nullptr);
				write_descriptor_set.setPNext(write_descriptor_set_acceleration_structure);
			}
			else
			{
				return StringError("Expected descriptor acceleration structure info");
			}
		}
		else if (auto buffers = std::get_if<std::vector<CMShared<Buffer>>>(&binded_resource))
		{
			if (auto descriptor_buffer_infos = std::get_if<std::vector<vk::DescriptorBufferInfo>>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(descriptor_buffer_infos->size());
				write_descriptor_set.setPBufferInfo(descriptor_buffer_infos->data());
				write_descriptor_set.setPImageInfo(nullptr);
				write_descriptor_set.setPNext(nullptr);
			}
			else
			{
				return StringError("Expected descriptor buffer info");
			}
		}
		else if (auto textures = std::get_if<std::vector<CMShared<Texture>>>(&binded_resource))
		{
			if (auto image_descriptor_infos = std::get_if<std::vector<vk::DescriptorImageInfo>>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(image_descriptor_infos->size());
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPImageInfo(image_descriptor_infos->data());
				write_descriptor_set.setPNext(nullptr);
			}
			else
			{
				return StringError("Expected descriptor image info");
			}
		}
		else if (auto samplers = std::get_if<std::vector<CMShared<Sampler>>>(&binded_resource))
		{
			if (auto image_descriptor_infos = std::get_if<std::vector<vk::DescriptorImageInfo>>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(image_descriptor_infos->size());
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPImageInfo(image_descriptor_infos->data());
				write_descriptor_set.setPNext(nullptr);
			}
			else
			{
				return StringError("Expected descriptor image info");
			}
		}
		else if (auto tlases = std::get_if<std::vector<CMShared<TopLevelAccelerationStructure>>>(&binded_resource))
		{
			if (auto write_descriptor_set_acceleration_structures = std::get_if<std::vector<vk::WriteDescriptorSetAccelerationStructureKHR>>(&descriptor_info))
			{
				write_descriptor_set.setDescriptorCount(write_descriptor_set_acceleration_structures->size());
				write_descriptor_set.setPBufferInfo(nullptr);
				write_descriptor_set.setPImageInfo(nullptr);
				write_descriptor_set.setPNext(write_descriptor_set_acceleration_structures->data());
			}
			else
			{
				return StringError("Expected descriptor acceleration structure info");
			}
		}
		else
		{
			return StringError("Couldn't sync descriptor set with descriptor info");
		}

		return NoError();
	}

	DescriptorSet::DescriptorSet()
	{

	}

	DescriptorSet::DescriptorSet(std::vector<vk::DescriptorSet> descriptor_sets_, DescriptorSetCreateInfo descriptor_set_create_info_) :
		descriptor_sets(std::move(descriptor_sets_)), descriptor_set_create_info(std::move(descriptor_set_create_info_))
	{

	}

	Error DescriptorSet::InitializeWriteDescriptorSets()
	{
		const auto& descriptor_resource_mapping = descriptor_set_create_info.pipeline_layout_info->combined_shader_reflection.descriptor_resource_mapping;

		for (const auto& [set, binding_to_descriptor_resource] : descriptor_resource_mapping)
		{
			for (const auto& [binding, descriptor_resource_] : binding_to_descriptor_resource)
			{
				const auto& descriptor_resource = *descriptor_resource_;
				if (auto uniform_buffer = std::get_if<ShaderReflection::UniformBuffer>(&descriptor_resource))
				{
					auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
						.setDstSet(descriptor_sets[set])
						.setDstBinding(binding)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eUniformBuffer);

					WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, CMShared<Buffer>{} };
					write_descriptor_sets[set][binding] = write_descriptor_set;
				}
				else if (auto storage_buffer = std::get_if<ShaderReflection::StorageBuffer>(&descriptor_resource))
				{
					auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
						.setDstSet(descriptor_sets[set])
						.setDstBinding(binding)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eStorageBuffer);

					WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, CMShared<Buffer>{} };
					write_descriptor_sets[set][binding] = write_descriptor_set;
				}
				else if (auto storage_image = std::get_if<ShaderReflection::StorageImage>(&descriptor_resource))
				{
					auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
						.setDstSet(descriptor_sets[set])
						.setDstBinding(binding)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eStorageImage);

					WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, CMShared<Texture>{} };
					write_descriptor_sets[set][binding] = write_descriptor_set;
				}
				else if (auto texel_buffer = std::get_if<ShaderReflection::TexelBuffer>(&descriptor_resource))
				{
					auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
						.setDstSet(descriptor_sets[set])
						.setDstBinding(binding)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eStorageTexelBuffer);

					WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, CMShared<Texture>{} };
					write_descriptor_sets[set][binding] = write_descriptor_set;
				}
				else if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
				{
					auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
						.setDstSet(descriptor_sets[set])
						.setDstBinding(binding)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

					WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, CMShared<Texture>{} };
					write_descriptor_sets[set][binding] = write_descriptor_set;
				}
				else if (auto acceleration_structure = std::get_if<ShaderReflection::AccelerationStructure>(&descriptor_resource))
				{
					auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
						.setDstSet(descriptor_sets[set])
						.setDstBinding(binding)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);

					WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, CMShared<TopLevelAccelerationStructure>{} };
					write_descriptor_sets[set][binding] = write_descriptor_set;
				}
				else if (auto subpass_input = std::get_if<ShaderReflection::SubpassInput>(&descriptor_resource))
				{
					return StringError("Subpass input cannot be not part of descriptor set");
				}
				else
				{
					return StringError("Unknown resource for set {} binding {}", set, binding);
				}
			}
		}

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, CMShared<Buffer> buffer, vk::DeviceSize offset, std::optional<vk::DeviceSize> range)
	{
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(descriptor_type);

		auto buffer_range = range ? *range : buffer->GetSize();

		auto descriptor_buffer_info = vk::DescriptorBufferInfo{}
			.setBuffer(*buffer)
			.setOffset(offset)
			.setRange(buffer_range);

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, buffer };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, descriptor_buffer_info));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, CMShared<Texture> texture, CMShared<Sampler> sampler)
	{
		PANIC("Not supported!");
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(descriptor_type);

		auto descriptor_texture_info = vk::DescriptorImageInfo{}
			.setImageView(*texture->image_view)
			.setImageLayout(texture->image->GetImageLayout())
			.setSampler(*sampler);

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, texture };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, descriptor_texture_info));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, CMShared<Texture> texture, std::optional<vk::ImageLayout> image_layout, std::optional<vk::DescriptorType> override_descriptor_type)
	{
		vk::DescriptorType descriptor_type{};
		if (override_descriptor_type)
		{
			descriptor_type = *override_descriptor_type;
		}
		else
		{
			AssignOrReturnError(descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));
		}

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(descriptor_type);

		if (image_layout)
		{
			texture->image->SetImageLayout(*image_layout);
		}
		auto descriptor_image_layout = texture->image->GetImageLayout();

		auto descriptor_texture_info = vk::DescriptorImageInfo{}
			.setImageView(*texture->image_view)
			.setImageLayout(descriptor_image_layout)
			.setSampler(*texture->sampler);

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, texture };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, descriptor_texture_info));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, CMShared<Sampler> sampler)
	{
		PANIC("Not supported, missing image field to descriptor image info");
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(descriptor_type);

		auto descriptor_sampler_info = vk::DescriptorImageInfo{}
			.setSampler(*sampler);

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, sampler };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, descriptor_sampler_info));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, CMShared<TopLevelAccelerationStructure> tlas)
	{
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(1)
			.setDescriptorType(descriptor_type);

		auto write_descriptor_set_acceleration_structure = vk::WriteDescriptorSetAccelerationStructureKHR{}
			.setAccelerationStructures(tlas->acceleration_structure_with_buffer.acceleration_structure);

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, tlas };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, write_descriptor_set_acceleration_structure));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, std::vector<CMShared<Buffer>> buffers)
	{
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		if (buffers.size() == 0)
		{
			return NoError();
		}

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(buffers.size())
			.setDescriptorType(descriptor_type);

		auto descriptor_buffer_infos = std::vector<vk::DescriptorBufferInfo>(buffers.size());
		for (auto i = 0; i < buffers.size(); i++)
		{
			const auto& buffer = buffers[i];

			auto descriptor_buffer_info = vk::DescriptorBufferInfo{}
				.setBuffer(*buffer)
				.setOffset(0)
				.setRange(buffer->GetSize());

			descriptor_buffer_infos[i] = descriptor_buffer_info;
		}

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, buffers };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, descriptor_buffer_infos));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, std::vector<CMShared<Texture>> textures)
	{
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		if (textures.size() == 0)
		{
			return NoError();
		}

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(textures.size())
			.setDescriptorType(descriptor_type);

		auto descriptor_image_infos = std::vector<vk::DescriptorImageInfo>(textures.size());
		for (auto i = 0; i < textures.size(); i++)
		{
			const auto& texture = textures[i];

			auto descriptor_texture_info = vk::DescriptorImageInfo{}
				.setImageView(*texture->image_view)
				.setImageLayout(texture->image->GetImageLayout())
				.setSampler(*texture->sampler);

			descriptor_image_infos[i] = descriptor_texture_info;
		}

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, textures };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, descriptor_image_infos));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, std::vector<CMShared<Sampler>> samplers)
	{
		PANIC("Not supported, missing image field to descriptor image info");
		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, std::vector<CMShared<TopLevelAccelerationStructure>> tlases)
	{
		AssignOrReturnError(vk::DescriptorType descriptor_type, GetDescriptorSetCorrespondingToSetAndBinding(set, binding));

		if (tlases.size() == 0)
		{
			return NoError();
		}

		auto vk_write_descriptor_set = vk::WriteDescriptorSet{}
			.setDstSet(descriptor_sets[set])
			.setDstBinding(binding)
			.setDescriptorCount(tlases.size())
			.setDescriptorType(descriptor_type);

		auto write_descriptor_set_acceleration_structures = std::vector<vk::WriteDescriptorSetAccelerationStructureKHR>(tlases.size());
		for (auto i = 0; i < tlases.size(); i++)
		{
			const auto& tlas = tlases[i];

			auto write_descriptor_set_acceleration_structure = vk::WriteDescriptorSetAccelerationStructureKHR{}
				.setAccelerationStructures(tlas->acceleration_structure_with_buffer.acceleration_structure);

			write_descriptor_set_acceleration_structures[i] = write_descriptor_set_acceleration_structure;
		}

		WriteDescriptorSet write_descriptor_set{ set, vk_write_descriptor_set, tlases };
		PanicIfError(UpdateWriteDescriptorSet(set, binding, write_descriptor_set, write_descriptor_set_acceleration_structures));

		return NoError();
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, WriteDescriptorSetBindedResource write_descriptor_resource)
	{
		if (auto buffer = std::get_if<CMShared<Buffer>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *buffer);
		}
		else if (auto texture = std::get_if<CMShared<Texture>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *texture);
		}
		else if (auto sampler = std::get_if<CMShared<Sampler>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *sampler);
		}
		else if (auto tlas = std::get_if<CMShared<TopLevelAccelerationStructure>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *tlas);
		}
		else if (auto buffers = std::get_if<std::vector<CMShared<Buffer>>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *buffers);
		}
		else if (auto textures = std::get_if<std::vector<CMShared<Texture>>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *textures);
		}
		else if (auto samplers = std::get_if<std::vector<CMShared<Sampler>>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *samplers);
		}
		else if (auto tlases = std::get_if<std::vector<CMShared<TopLevelAccelerationStructure>>>(&write_descriptor_resource))
		{
			return Bind(set, binding, *tlases);
		}
		else
		{
			return StringError("Couldn't hash binded resource, unknown resource");
		}
	}

	Error DescriptorSet::Bind(uint32_t set, uint32_t binding, std::vector<WriteDescriptorSetBindedResource> write_descriptor_resources)
	{
		for (auto i = 0; i < write_descriptor_resources.size(); i++)
		{
			auto binding_index = binding + i;
			if (auto err = Bind(set, binding_index, write_descriptor_resources[i]))
			{
				return err;
			}
		}
		return NoError();
	}

	Error DescriptorSet::SetPendingWrite(uint32_t set, uint32_t binding, bool pending_write)
	{
		if (auto found_set = write_descriptor_sets.find(set); found_set != std::end(write_descriptor_sets))
		{
			auto& write_descriptor_set_binding = found_set->second;
			if (auto found = write_descriptor_set_binding.find(binding); found != std::end(write_descriptor_set_binding))
			{
				auto& write_descriptor_set = found->second;
				write_descriptor_set.pending_write = pending_write;
			}
			else
			{
				return StringError("Out of bounds pending write set {} binding {}", set, binding);
			}
		}
		else
		{
			return StringError("Out of bounds pending write set {} binding {}", set, binding);
		}
		return NoError();
	}

	Error DescriptorSet::FlushPendingWrites(vk::Device device)
	{
#ifndef NDEBUG
		for (auto& [set, binding_write_descriptor_set] : write_descriptor_sets)
		{
			for (auto& [binding, write_descriptor_set] : binding_write_descriptor_set)
			{
				if (write_descriptor_set.pending_write)
				{
					ReturnIfError(write_descriptor_set.SyncWriteDescriptorSetWithDescriptorInfo());
					device.updateDescriptorSets(write_descriptor_set.write_descriptor_set, {});
					write_descriptor_set.pending_write = false;
				}
			}
		}

#else
		std::vector<vk::WriteDescriptorSet> pending_write_descriptor_sets;
		//std::transform(std::begin(pending_write_descriptor_sets), std::end(pending_write_descriptor_sets), std::begin(vk_pending_write_descriptor_sets), [](const auto& e) { return e.write_descriptor_set; });
		
		for (auto& [set, binding_write_descriptor_set] : write_descriptor_sets)
		{
			for (auto& [binding, write_descriptor_set] : binding_write_descriptor_set)
			{
				if (write_descriptor_set.pending_write)
				{
					ReturnIfError(write_descriptor_set.SyncWriteDescriptorSetWithDescriptorInfo());
					pending_write_descriptor_sets.emplace_back(write_descriptor_set.write_descriptor_set);
					write_descriptor_set.pending_write = false;
				}
			}
		}

		device.updateDescriptorSets(pending_write_descriptor_sets, {});
		pending_write_descriptor_sets.clear();
#endif

		return NoError();
	}

	Expected<vk::DescriptorType> DescriptorSet::GetDescriptorSetCorrespondingToSetAndBinding(uint32_t set, uint32_t binding)
	{
		if (set >= descriptor_sets.size())
		{
			return StringError("Set {} is out of bounds... Max set for this descriptor set is {}", set, descriptor_sets.size() - 1);
		}

		vk::DescriptorType descriptor_type{};

		const auto& descriptor_resource_mapping = descriptor_set_create_info.pipeline_layout_info->combined_shader_reflection.descriptor_resource_mapping;

		if (auto found_binding = descriptor_resource_mapping.find(set); found_binding != std::end(descriptor_resource_mapping))
		{
			const auto& binding_to_descriptor_resource = found_binding->second;
			if (auto found_descriptor_resource = binding_to_descriptor_resource.find(binding); found_descriptor_resource != std::end(binding_to_descriptor_resource))
			{
				const auto& descriptor_resource = *found_descriptor_resource->second;
				if (auto uniform_buffer = std::get_if<ShaderReflection::UniformBuffer>(&descriptor_resource))
				{
					descriptor_type = vk::DescriptorType::eUniformBuffer;
				}
				else if (auto storage_buffer = std::get_if<ShaderReflection::StorageBuffer>(&descriptor_resource))
				{
					descriptor_type = vk::DescriptorType::eStorageBuffer;
				}
				else if (auto storage_image = std::get_if<ShaderReflection::StorageImage>(&descriptor_resource))
				{
					descriptor_type = vk::DescriptorType::eStorageImage;
				}
				else if (auto texel_buffer = std::get_if<ShaderReflection::TexelBuffer>(&descriptor_resource))
				{
					descriptor_type = vk::DescriptorType::eStorageTexelBuffer;
				}
				else if (auto sampler = std::get_if<ShaderReflection::Sampler>(&descriptor_resource))
				{
					descriptor_type = vk::DescriptorType::eCombinedImageSampler;
				}
				else if (auto acceleration_structure = std::get_if<ShaderReflection::AccelerationStructure>(&descriptor_resource))
				{
					descriptor_type = vk::DescriptorType::eAccelerationStructureKHR;
				}
				else if (auto subpass_input = std::get_if<ShaderReflection::SubpassInput>(&descriptor_resource))
				{
					return StringError("Subpass input cannot be not part of descriptor set");
				}
				else
				{
					return StringError("Unknown resource for set {} binding {}", set, binding);
				}
			}
			else
			{
				return StringError("Binding {} is not in shader reflection", binding);
			}
		}
		else
		{
			return StringError("Set {} is not in shader reflection", set);
		}

		return descriptor_type;
	}

	Error DescriptorSet::UpdateWriteDescriptorSet(uint32_t set, uint32_t binding, const WriteDescriptorSet& write_descriptor_set, const WriteDescriptorSetDescriptorInfo& write_descriptor_set_descriptor_info)
	{
		auto [iter, inserted] = write_descriptor_sets[set].try_emplace(binding, write_descriptor_set);
		if (inserted)
		{
			// Here, a new write descriptor set that was inserted
			auto& inserted_write_descriptor_set = iter->second;

			// Update the descriptor sets
			auto updated_write_descriptor_set = write_descriptor_set;
			updated_write_descriptor_set = write_descriptor_set;
			updated_write_descriptor_set.descriptor_info = write_descriptor_set_descriptor_info;
			updated_write_descriptor_set.pending_write = true;
			inserted_write_descriptor_set = updated_write_descriptor_set;
			//ReturnIfError(inserted_write_descriptor_set.SyncWriteDescriptorSetWithDescriptorInfo());

			// Add to pending writes
			//pending_write_descriptor_sets.emplace_back(inserted_write_descriptor_set.write_descriptor_set);
		}
		else
		{
			// If already inserted, check if it is any different
			auto& inserted_write_descriptor_set = iter->second;

			auto new_hash = StdHash(write_descriptor_set);
			auto old_hash = StdHash(inserted_write_descriptor_set);
			if (old_hash != new_hash)
			{
				// Update the descriptor sets
				auto updated_write_descriptor_set = write_descriptor_set;
				updated_write_descriptor_set = write_descriptor_set;
				updated_write_descriptor_set.descriptor_info = write_descriptor_set_descriptor_info;
				updated_write_descriptor_set.pending_write = true;
				inserted_write_descriptor_set = updated_write_descriptor_set;
				//ReturnIfError(inserted_write_descriptor_set.SyncWriteDescriptorSetWithDescriptorInfo());

				// Add to pending writes
				//pending_write_descriptor_sets.emplace_back(inserted_write_descriptor_set.write_descriptor_set);
			}
			else
			{
				// Ignore since they are the same
			}
		}

		return NoError();
	}

}