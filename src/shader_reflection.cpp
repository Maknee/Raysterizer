#include "include/shader_reflection.h"

namespace RaysterizerEngine
{
	Expected<ShaderReflection::FoundMember> ShaderReflection::UniformBuffer::FindMember(std::string_view s)
	{
		if (auto found = found_member_cache.find(s); found != std::end(found_member_cache))
		{
			return found->second;
		}

		auto member_access_names = RaysterizerEngine::Util::SplitString(s, ".");

		auto total_offset = 0;
		auto iteration = 0;
		ShaderReflection::Member* found_member{};
		std::vector<ShaderReflection::Member>* current_members = &members;
		for (const auto& member_access_name : member_access_names)
		{
			for (auto& current_member : *current_members)
			{
				if (current_member.name == member_access_name)
				{
					total_offset += current_member.size;
					found_member = &current_member;
					current_members = &current_member.members;
					iteration++;
				}
			}
		}

		if (found_member != nullptr && iteration == member_access_names.size())
		{
			FoundMember found_member_result{ total_offset, found_member };
			found_member_cache.try_emplace(s, found_member_result);
			return found_member_result;
		}
		return StringError("Member not found! {}", s);
	}

	Expected<ShaderReflection::FoundMember> ShaderReflection::StorageBuffer::FindMember(std::string_view s)
	{
		auto member_access_names = RaysterizerEngine::Util::SplitString(s, ".");
		
		auto total_offset = 0;
		auto iteration = 0;
		ShaderReflection::Member* found_member{};
		std::vector<ShaderReflection::Member>* current_members = &members;
		for (const auto& member_access_name : member_access_names)
		{
			for (auto& current_member : *current_members)
			{
				if (current_member.name == member_access_name)
				{
					total_offset += current_member.size;
					found_member = &current_member;
					current_members = &current_member.members;
					iteration++;
				}
			}
		}

		if (found_member != nullptr && iteration == member_access_names.size())
		{
			FoundMember found_member_result{ total_offset, found_member };
			return found_member_result;
		}
		return StringError("Member not found! {}", s);
	}

	static auto binding_cmp = [](auto& s1, auto& s2) { return s1.binding < s2.binding; };
	static auto binding_eq = [](auto& s1, auto& s2) { return s1.binding == s2.binding; };

	template<class T>
	void unq(T& s) {
		std::sort(s.begin(), s.end(), binding_cmp);
		for (auto it = s.begin(); it != s.end();) {
			vk::ShaderStageFlags stages = it->stage;
			for (auto it2 = it; it2 != s.end(); it2++) {
				if (it->binding == it2->binding) {
					stages |= it2->stage;
				}
				else
					break;
			}
			it->stage = stages;
			auto it2 = it;
			for (; it2 != s.end(); it2++) {
				if (it->binding != it2->binding)
					break;
				it2->stage = stages;
			}
			it = it2;
		}
		s.erase(std::unique(s.begin(), s.end(), binding_eq), s.end());
	}

	ShaderReflection::Type to_type(spirv_cross::SPIRType s) {
		using namespace spirv_cross;

		switch (s.basetype) {
		case SPIRType::UByte:
			switch (s.vecsize) {
			case 1:	return ShaderReflection::Type::eubyte; break;
			case 2: return ShaderReflection::Type::eubyte2; break;
			case 3: return ShaderReflection::Type::eubyte3; break;
			case 4: return ShaderReflection::Type::eubyte4; break;
			default: PANIC("NYI");
			}
		case SPIRType::UShort:
			switch (s.vecsize) {
			case 1:	return ShaderReflection::Type::eushort; break;
			case 2: return ShaderReflection::Type::eushort2; break;
			case 3: return ShaderReflection::Type::eushort3; break;
			case 4: return ShaderReflection::Type::eushort4; break;
			default: PANIC("NYI");
			}
		case SPIRType::Float:
			switch (s.columns) {
			case 1:
				switch (s.vecsize) {
				case 1:	return ShaderReflection::Type::efloat; break;
				case 2: return ShaderReflection::Type::evec2; break;
				case 3: return ShaderReflection::Type::evec3; break;
				case 4: return ShaderReflection::Type::evec4; break;
				default: assert("NYI" && 0);
				}
			case 4:
				return ShaderReflection::Type::emat4; break;
			}
		case SPIRType::Double:
			switch (s.columns) {
			case 1:
				switch (s.vecsize) {
				case 1:	return ShaderReflection::Type::edouble; break;
				case 2: return ShaderReflection::Type::edvec2; break;
				case 3: return ShaderReflection::Type::edvec3; break;
				case 4: return ShaderReflection::Type::edvec4; break;
				default: assert("NYI" && 0);
				}
			case 4:
				return ShaderReflection::Type::edmat4; break;
			}
		case SPIRType::Int:
			switch (s.vecsize) {
			case 1:	return ShaderReflection::Type::eint; break;
			case 2: return ShaderReflection::Type::eivec2; break;
			case 3: return ShaderReflection::Type::eivec3; break;
			case 4: return ShaderReflection::Type::eivec4; break;
			default: PANIC("NYI");
			}
		case SPIRType::UInt:
			switch (s.vecsize) {
			case 1:	return ShaderReflection::Type::euint; break;
			case 2: return ShaderReflection::Type::euvec2; break;
			case 3: return ShaderReflection::Type::euvec3; break;
			case 4: return ShaderReflection::Type::euvec4; break;
			default: PANIC("NYI");
			}
		case SPIRType::Struct: return ShaderReflection::Type::estruct;
		case SPIRType::UInt64:
			switch (s.vecsize) {
			case 1:	return ShaderReflection::Type::euint64; break;
			case 2: return ShaderReflection::Type::euint64vec2; break;
			case 3: return ShaderReflection::Type::euint64vec3; break;
			case 4: return ShaderReflection::Type::euint64vec4; break;
			default: PANIC("NYI");
			}
		default:
			PANIC("NYI");
			return ShaderReflection::Type::eunknown;
		}
	}

	void reflect_members(const spirv_cross::Compiler& spirv_compiler, const spirv_cross::SPIRType& type, std::vector<ShaderReflection::Member>& members) {
		for (uint32_t i = 0; i < type.member_types.size(); i++) {
			auto& t = type.member_types[i];
			ShaderReflection::Member m;
			auto spirtype = spirv_compiler.get_type(t);
			m.spirv_type = spirtype;
			m.type = to_type(spirtype);
			if (m.type == ShaderReflection::Type::estruct) {
				m.type_name = spirv_compiler.get_name(t);
				if (m.type_name == "") {
					m.type_name = spirv_compiler.get_name(spirtype.parent_type);
				}
			}
			m.name = spirv_compiler.get_member_name(type.self, i);
			m.size = spirv_compiler.get_declared_struct_member_size(type, i);
			m.offset = spirv_compiler.type_struct_member_offset(type, i);

			if (m.type == ShaderReflection::Type::estruct) {
				m.size = spirv_compiler.get_declared_struct_size(spirtype);
				reflect_members(spirv_compiler, spirtype, m.members);
			}

			if (spirtype.array.size() > 0) {
				m.array_size = spirtype.array[0];
			}
			else {
				m.array_size = 1;
			}

			members.push_back(m);
		}
	}

	Error ShaderReflection::Reflect(Spirv spirv)
	{
		spirv_cross::Compiler spirv_compiler(std::move(spirv));

		auto resources = spirv_compiler.get_shader_resources();
		auto entry_name = spirv_compiler.get_entry_points_and_stages()[0];
		auto entry_point = spirv_compiler.get_entry_point(entry_name.name, entry_name.execution_model);
		auto model = entry_point.model;
		auto stage = [=]() {
			switch (model) {
			case spv::ExecutionModel::ExecutionModelVertex: return vk::ShaderStageFlagBits::eVertex;
			case spv::ExecutionModel::ExecutionModelTessellationControl: return vk::ShaderStageFlagBits::eTessellationControl;
			case spv::ExecutionModel::ExecutionModelTessellationEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
			case spv::ExecutionModel::ExecutionModelGeometry: return vk::ShaderStageFlagBits::eGeometry;
			case spv::ExecutionModel::ExecutionModelFragment: return vk::ShaderStageFlagBits::eFragment;
			case spv::ExecutionModel::ExecutionModelGLCompute: return vk::ShaderStageFlagBits::eCompute;
			case spv::ExecutionModel::ExecutionModelRayGenerationKHR: return vk::ShaderStageFlagBits::eRaygenKHR;
			case spv::ExecutionModel::ExecutionModelIntersectionKHR: return vk::ShaderStageFlagBits::eIntersectionKHR;
			case spv::ExecutionModel::ExecutionModelAnyHitKHR: return vk::ShaderStageFlagBits::eAnyHitKHR;
			case spv::ExecutionModel::ExecutionModelClosestHitKHR: return vk::ShaderStageFlagBits::eClosestHitKHR;
			case spv::ExecutionModel::ExecutionModelMissKHR: return vk::ShaderStageFlagBits::eMissKHR;
			case spv::ExecutionModel::ExecutionModelCallableKHR: return vk::ShaderStageFlagBits::eCallableKHR;
			default:
			{
				PANIC("Stage not supported!");
				return vk::ShaderStageFlagBits::eVertex;
			}
			}
		}();
		stages = stage;
		if (stage == vk::ShaderStageFlagBits::eVertex) {
			for (auto& sb : resources.stage_inputs) {
				auto type = spirv_compiler.get_type(sb.type_id);
				auto location = spirv_compiler.get_decoration(sb.id, spv::DecorationLocation);
				Attribute a;
				a.location = location;
				a.name = sb.name.c_str();
				a.type = to_type(type);
				attributes.push_back(a);
			}
		}
		// uniform buffers
		for (auto& ub : resources.uniform_buffers) {
			auto type = spirv_compiler.get_type(ub.type_id);
			auto binding = spirv_compiler.get_decoration(ub.id, spv::DecorationBinding);
			auto set = spirv_compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);
			UniformBuffer un;
			un.set = set;
			un.binding = binding;
			un.stage = stage;
			un.name = std::string(ub.name.c_str());
			if (type.array.size() > 0)
				un.array_size = type.array[0];
			else
				un.array_size = 1;
			if (type.basetype == spirv_cross::SPIRType::Struct) {
				reflect_members(spirv_compiler, spirv_compiler.get_type(ub.type_id), un.members);
			}
			un.size = spirv_compiler.get_declared_struct_size(type);
			sets[set].uniform_buffers.push_back(un);
		}

		for (auto& sb : resources.storage_buffers) {
			auto type = spirv_compiler.get_type(sb.type_id);
			auto binding = spirv_compiler.get_decoration(sb.id, spv::DecorationBinding);
			auto set = spirv_compiler.get_decoration(sb.id, spv::DecorationDescriptorSet);
			StorageBuffer un;
			un.set = set;
			un.binding = binding;
			un.stage = stage;
			un.name = sb.name.c_str();
			un.min_size = spirv_compiler.get_declared_struct_size(spirv_compiler.get_type(sb.type_id));
			if (type.array.size() > 0)
				un.array_size = type.array[0];
			else
				un.array_size = 1;
			if (type.basetype == spirv_cross::SPIRType::Struct) {
				reflect_members(spirv_compiler, spirv_compiler.get_type(sb.type_id), un.members);
			}
			sets[set].storage_buffers.push_back(un);
		}

		for (auto& si : resources.sampled_images) {
			auto type = spirv_compiler.get_type(si.type_id);
			auto binding = spirv_compiler.get_decoration(si.id, spv::DecorationBinding);
			auto set = spirv_compiler.get_decoration(si.id, spv::DecorationDescriptorSet);
			Sampler t;
			t.set = set;
			t.binding = binding;
			t.name = std::string(si.name.c_str());
			t.stage = stage;
			// maybe spirv cross bug?
			t.array_size = type.array.size() == 1 ? (type.array[0] == 1 ? 0 : type.array[0]) : -1;
			t.shadow = type.image.depth;
			sets[set].samplers.push_back(t);
		}

		for (auto& sb : resources.storage_images) {
			auto type = spirv_compiler.get_type(sb.type_id);
			auto binding = spirv_compiler.get_decoration(sb.id, spv::DecorationBinding);
			auto set = spirv_compiler.get_decoration(sb.id, spv::DecorationDescriptorSet);
			StorageImage un;
			un.set = set;
			un.binding = binding;
			un.stage = stage;
			un.name = sb.name.c_str();
			// maybe spirv cross bug?
			un.array_size = type.array.size() == 1 ? (type.array[0] == 1 ? 0 : type.array[0]) : -1;
			sets[set].storage_images.push_back(un);
		}

		// subpass inputs
		for (auto& si : resources.subpass_inputs) {
			auto type = spirv_compiler.get_type(si.type_id);
			auto binding = spirv_compiler.get_decoration(si.id, spv::DecorationBinding);
			auto set = spirv_compiler.get_decoration(si.id, spv::DecorationDescriptorSet);
			SubpassInput s;
			s.name = std::string(si.name.c_str());
			s.set = set;
			s.binding = binding;
			s.stage = stage;
			sets[set].subpass_inputs.push_back(s);
		}

		for (auto& si : resources.acceleration_structures) {
			auto type = spirv_compiler.get_type(si.type_id);
			auto binding = spirv_compiler.get_decoration(si.id, spv::DecorationBinding);
			auto set = spirv_compiler.get_decoration(si.id, spv::DecorationDescriptorSet);
			AccelerationStructure as;
			as.name = std::string(si.name.c_str());
			as.set = set;
			as.binding = binding;
			as.stage = stage;
			sets[set].acceleration_structures.push_back(as);
		}

		for (auto& sc : spirv_compiler.get_specialization_constants()) {
			spec_constants.emplace_back(SpecConstant{ sc.constant_id, to_type(spirv_compiler.get_type(spirv_compiler.get_constant(sc.id).constant_type)), (vk::ShaderStageFlags)stage });
		}

		// remove duplicated bindings (aliased bindings)
		// TODO: we need to preserve this information somewhere
		for (auto& [index, set] : sets) {
			unq(set.samplers);
			unq(set.uniform_buffers);
			unq(set.storage_buffers);
			unq(set.texel_buffers);
			unq(set.subpass_inputs);
			unq(set.storage_images);
			unq(set.acceleration_structures);
		}

		std::sort(spec_constants.begin(), spec_constants.end(), binding_cmp);

		for (auto& [index, set] : sets) {
			unsigned max_binding = 0;
			for (auto& ub : set.uniform_buffers) {
				max_binding = std::max(max_binding, ub.binding);
			}
			for (auto& ub : set.storage_buffers) {
				max_binding = std::max(max_binding, ub.binding);
			}
			for (auto& ub : set.samplers) {
				max_binding = std::max(max_binding, ub.binding);
			}
			for (auto& ub : set.subpass_inputs) {
				max_binding = std::max(max_binding, ub.binding);
			}
			for (auto& ub : set.storage_buffers) {
				max_binding = std::max(max_binding, ub.binding);
			}
			for (auto& ub : set.acceleration_structures) {
				max_binding = std::max(max_binding, ub.binding);
			}
			set.highest_descriptor_binding = max_binding;
		}

		// push constants
		for (auto& si : resources.push_constant_buffers) {
			auto type = spirv_compiler.get_type(si.base_type_id);
			VkPushConstantRange pcr;
			pcr.offset = 0;
			pcr.size = (uint32_t)spirv_compiler.get_declared_struct_size(type);
			pcr.stageFlags = VkShaderStageFlags(stage);
			push_constant_ranges.push_back(pcr);
		}

		if (stage == vk::ShaderStageFlagBits::eCompute) {
			local_size = { spirv_compiler.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 0),
						   spirv_compiler.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 1),
						   spirv_compiler.get_execution_mode_argument(spv::ExecutionMode::ExecutionModeLocalSize, 2) };
		}
		
		for (const auto& [set, descriptors] : sets)
		{
			auto& descriptor_resource_mapping_binding = descriptor_resource_mapping[set];
			for (const auto& e : descriptors.uniform_buffers)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
			for (const auto& e : descriptors.storage_buffers)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
			for (const auto& e : descriptors.storage_images)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
			for (const auto& e : descriptors.texel_buffers)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
			for (const auto& e : descriptors.samplers)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
			for (const auto& e : descriptors.subpass_inputs)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
			for (const auto& e : descriptors.acceleration_structures)
			{
				auto resource = std::make_shared<DescriptorResource>(e);
				descriptor_resource_mapping_binding[e.binding] = resource;
				name_to_descriptor_resource[e.name] = resource;
			}
		}

		return NoError();
	}

	Error ShaderReflection::Merge(const ShaderReflection& o)
	{
		attributes.insert(attributes.end(), o.attributes.begin(), o.attributes.end());
		push_constant_ranges.insert(push_constant_ranges.end(), o.push_constant_ranges.begin(), o.push_constant_ranges.end());
		spec_constants.insert(spec_constants.end(), o.spec_constants.begin(), o.spec_constants.end());

		std::sort(push_constant_ranges.begin(), push_constant_ranges.end(), [](const auto& s1, const auto& s2)
			{
				return s1.stageFlags < s2.stageFlags;
			});
		push_constant_ranges.erase(std::unique(push_constant_ranges.begin(), push_constant_ranges.end(), [](const auto& s1, const auto& s2)
			{
				return s1.stageFlags == s2.stageFlags;
			}), push_constant_ranges.end());

		unq(spec_constants);
		for (auto& [index, os] : o.sets) {
			auto& s = sets[index];
			s.samplers.insert(s.samplers.end(), os.samplers.begin(), os.samplers.end());
			s.uniform_buffers.insert(s.uniform_buffers.end(), os.uniform_buffers.begin(), os.uniform_buffers.end());
			s.storage_buffers.insert(s.storage_buffers.end(), os.storage_buffers.begin(), os.storage_buffers.end());
			s.texel_buffers.insert(s.texel_buffers.end(), os.texel_buffers.begin(), os.texel_buffers.end());
			s.subpass_inputs.insert(s.subpass_inputs.end(), os.subpass_inputs.begin(), os.subpass_inputs.end());
			s.storage_images.insert(s.storage_images.end(), os.storage_images.begin(), os.storage_images.end());
			s.acceleration_structures.insert(s.acceleration_structures.end(), os.acceleration_structures.begin(), os.acceleration_structures.end());

			unq(s.samplers);
			unq(s.uniform_buffers);
			unq(s.storage_buffers);
			unq(s.texel_buffers);
			unq(s.subpass_inputs);
			unq(s.storage_images);
			unq(s.acceleration_structures);
			s.highest_descriptor_binding = std::max(s.highest_descriptor_binding, os.highest_descriptor_binding);
		}

		stages |= o.stages;

		//descriptor_resource_mapping.insert(std::begin(o.descriptor_resource_mapping), std::end(o.descriptor_resource_mapping));
		for (const auto& [set, binding_resource] : o.descriptor_resource_mapping)
		{
			if (auto found = descriptor_resource_mapping.find(set); found != std::end(descriptor_resource_mapping))
			{
				auto& [binding, binded_resource] = *found;
				binded_resource.insert(std::begin(binding_resource), std::end(binding_resource));
			}
			else
			{
				descriptor_resource_mapping.emplace(set, binding_resource);
			}
		}
		name_to_descriptor_resource.insert(std::begin(o.name_to_descriptor_resource), std::end(o.name_to_descriptor_resource));

		return NoError();
	}

	Expected<std::shared_ptr<ShaderReflection::DescriptorResource>> ShaderReflection::GetDescriptorResource(uint32_t set, uint32_t binding)
	{
		if (auto found_binding = descriptor_resource_mapping.find(set); found_binding != std::end(descriptor_resource_mapping))
		{
			const auto& binding_to_descriptor_resource = found_binding->second;
			if (auto found_descriptor_resource = binding_to_descriptor_resource.find(binding); found_descriptor_resource != std::end(binding_to_descriptor_resource))
			{
				const auto& descriptor_resource = found_descriptor_resource->second;
				return descriptor_resource;
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
	}

	Expected<std::shared_ptr<ShaderReflection::DescriptorResource>> ShaderReflection::GetDescriptorResource(std::string_view s)
	{
		if (auto found = name_to_descriptor_resource.find(s); found != std::end(name_to_descriptor_resource))
		{
			auto& descriptor_resource = found->second;
			return descriptor_resource;
		}

		return StringError("Not found {}", s);
	}
}