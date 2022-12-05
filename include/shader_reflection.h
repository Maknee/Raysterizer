#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	using Spirv = std::vector<uint32_t>;

	enum class ShaderKind : uint32_t
	{
		UNKNOWN,
		VERT,
		TESC,
		TESE,
		GEOM,
		FRAG,
		COMP,
		RGEN,
		RCHIT,
		RMISS,
		RAHIT,
		RINT,
	};

	inline ShaderKind GetShaderKindFromExtension(const std::string& stage)
	{
		if (stage == ".vert")
		{
			return ShaderKind::VERT;
		}
		else if (stage == ".tesc")
		{
			return ShaderKind::TESC;
		}
		else if (stage == ".tese")
		{
			return ShaderKind::TESE;
		}
		else if (stage == ".geom")
		{
			return ShaderKind::GEOM;
		}
		else if (stage == ".frag")
		{
			return ShaderKind::FRAG;
		}
		else if (stage == ".comp")
		{
			return ShaderKind::COMP;
		}
		else if (stage == ".rgen")
		{
			return ShaderKind::RGEN;
		}
		else if (stage == ".rchit")
		{
			return ShaderKind::RCHIT;
		}
		else if (stage == ".rmiss")
		{
			return ShaderKind::RMISS;
		}
		else if (stage == ".rahit")
		{
			return ShaderKind::RAHIT;
		}
		else if (stage == ".rint")
		{
			return ShaderKind::RINT;
		}
		else
		{
			PANIC("Unknown shader stage");
			return ShaderKind::VERT;
		}
	}

	struct ShaderReflection {
		enum class Type {
			eubyte, eushort, euint, eint, efloat, edouble, euint64,
			eubyte2, eubyte3, eubyte4,
			eushort2, eushort3, eushort4,
			euvec2, euvec3, euvec4,
			eivec2, eivec3, eivec4,
			evec2, evec3, evec4,
			edvec2, edvec3, edvec4,
			euint64vec2, euint64vec3, euint64vec4,
			emat4, edmat4, estruct, eunknown
		};

		struct Attribute {
			std::string name;

			size_t location;
			Type type;
		};

		struct TextureAddress {
			unsigned container;
			float page;
		};

		struct Member {
			std::string name;
			std::string type_name; // if this is a struct
			spirv_cross::SPIRType spirv_type;
			Type type;
			size_t size;
			size_t offset;
			unsigned array_size;
			std::vector<Member> members;
		};

		struct FoundMember
		{
			std::size_t offset;
			Member* member;
		};

		// always a struct
		struct UniformBuffer {
			std::string name;
			
			uint32_t set;
			unsigned binding;
			size_t size;
			unsigned array_size;

			std::vector<Member> members;

			vk::ShaderStageFlags stage;

			flat_hash_map<std::string, FoundMember> found_member_cache;
			Expected<FoundMember> FindMember(std::string_view s);
		};

		struct StorageBuffer {
			std::string name;

			uint32_t set;
			unsigned binding;
			size_t min_size;
			unsigned array_size;

			std::vector<Member> members;

			vk::ShaderStageFlags stage;

			Expected<FoundMember> FindMember(std::string_view s);
		};

		struct StorageImage {
			std::string name;

			uint32_t set;
			unsigned binding;
			unsigned array_size;
			vk::ShaderStageFlags stage;
		};

		struct Sampler {
			std::string name;

			uint32_t set;
			unsigned binding;
			unsigned array_size;

			bool shadow; // if this is a samplerXXXShadow

			vk::ShaderStageFlags stage;
		};

		struct TexelBuffer {
			std::string name;

			uint32_t set;
			unsigned binding;
			vk::ShaderStageFlags stage;
		};

		struct SubpassInput {
			std::string name;

			uint32_t set;
			unsigned binding;
			vk::ShaderStageFlags stage;
		};

		struct AccelerationStructure {
			std::string name;

			uint32_t set;
			unsigned binding;
			vk::ShaderStageFlags stage;
		};

		struct SpecConstant {
			unsigned binding; // constant_id
			Type type;

			vk::ShaderStageFlags stage;
		};

		std::array<unsigned, 3> local_size;

		std::vector<Attribute> attributes;
		std::vector<VkPushConstantRange> push_constant_ranges;
		std::vector<SpecConstant> spec_constants;
		struct Descriptors {
			std::vector<UniformBuffer> uniform_buffers;
			std::vector<StorageBuffer> storage_buffers;
			std::vector<StorageImage> storage_images;
			std::vector<TexelBuffer> texel_buffers;
			std::vector<Sampler> samplers;
			std::vector<SubpassInput> subpass_inputs;
			std::vector<AccelerationStructure> acceleration_structures;

			unsigned highest_descriptor_binding = 0;
		};
		std::unordered_map<size_t, Descriptors> sets;
		vk::ShaderStageFlags stages = {};


		using DescriptorResource = std::variant<UniformBuffer, StorageBuffer, StorageImage, TexelBuffer, Sampler, SubpassInput, AccelerationStructure>;
		
		// set -> binding -> DescriptorResource
		flat_hash_map<uint32_t, flat_hash_map<uint32_t, std::shared_ptr<DescriptorResource>>> descriptor_resource_mapping{};
		flat_hash_map<std::string, std::shared_ptr<DescriptorResource>> name_to_descriptor_resource{};

		Error Reflect(Spirv spirv);
		Error Merge(const ShaderReflection& o);

		const auto& GetDescriptorResourceMapping() const { return descriptor_resource_mapping; }
		const auto& GetNameToDescriptorResource() const { return name_to_descriptor_resource; }

		Expected<std::shared_ptr<DescriptorResource>> GetDescriptorResource(uint32_t set, uint32_t binding);
		Expected<std::shared_ptr<DescriptorResource>> GetDescriptorResource(std::string_view s);
		template<typename T>
		Expected<T&> GetDescriptorResourceAs(std::string_view s)
		{
			AssignOrReturnError(auto descriptor_resource, GetDescriptorResource(s));

			if (auto* underlying_descriptor_resource = std::get_if<T>(&*descriptor_resource))
			{
				return *underlying_descriptor_resource;
			}
			else
			{
				return StringError("Could not adapt to storage resource for name {}", s);
			}
		}
	};
}