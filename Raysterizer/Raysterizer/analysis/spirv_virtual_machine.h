#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Analysis
	{
		struct SPVInstruction
		{
			// An array of words for this instruction, in native endianness.
			std::vector<uint32_t> words{};
			SpvOp opcode;
			// The extended instruction type, if opcode is OpExtInst.  Otherwise
			// this is the "none" value.
			spv_ext_inst_type_t ext_inst_type;
			// The type id, or 0 if this instruction doesn't have one.
			uint32_t type_id;
			// The result id, or 0 if this instruction doesn't have one.
			uint32_t result_id;
			// The array of parsed operands.
			std::vector<spv_parsed_operand_t> operands;

			uint32_t offset{};
		};

		class SPVInstructionManager
		{
		public:
			explicit SPVInstructionManager() = default;
			explicit SPVInstructionManager(spv_target_env target_env, std::vector<uint32_t> spv)
			{
				context = SPVContextPtr(spvContextCreate(target_env), &spvContextDestroy);
				name_mapper = std::make_unique<spvtools::FriendlyNameMapper>(context.get(), spv.data(), spv.size());
				grammar = std::make_unique<spvtools::AssemblyGrammar>(context.get());
				if (!grammar->isValid())
				{
					PANIC("INVALID GRAMMAR");
				}

				spv_diagnostic_t diagnostic{};
				spv_diagnostic diagnostic2 = &diagnostic;
				spvBinaryParse(&*context, this, spv.data(), spv.size(),
							   SPVInstructionManager::SPVHandleHeader, SPVInstructionManager::SPVHandleInstruction, nullptr);
				if (diagnostic2->error)
				{
					PANIC(diagnostic2->error);
				}
			}

			void Init(spv_target_env target_env, std::vector<uint32_t> spv)
			{
				context = SPVContextPtr(spvContextCreate(target_env), &spvContextDestroy);
				name_mapper = std::make_unique<spvtools::FriendlyNameMapper>(context.get(), spv.data(), spv.size());
				grammar = std::make_unique<spvtools::AssemblyGrammar>(context.get());
				if (!grammar->isValid())
				{
					PANIC("INVALID GRAMMAR");
				}

				spv_diagnostic_t diagnostic{};
				spv_diagnostic diagnostic2 = &diagnostic;
				spvBinaryParse(&*context, this, spv.data(), spv.size(),
							   SPVInstructionManager::SPVHandleHeader, SPVInstructionManager::SPVHandleInstruction, nullptr);
				if (diagnostic2->error)
				{
					PANIC(diagnostic2->error);
				}
			}

			spv_result_t HandleHeader(spv_endianness_t endian, uint32_t version, uint32_t generator, uint32_t id_bound, uint32_t schema)
			{
				//spvOpcodeString
				word_offset += SPV_INDEX_INSTRUCTION;

				return spv_result_t::SPV_SUCCESS;
			}

			spv_result_t HandleInstruction(const spv_parsed_instruction_t& parsed_inst)
			{
				SPVInstruction inst{};
				inst.words = std::vector<uint32_t>(parsed_inst.words, parsed_inst.words + parsed_inst.num_words);
				inst.opcode = static_cast<SpvOp>(parsed_inst.opcode);
				inst.ext_inst_type = parsed_inst.ext_inst_type;
				inst.type_id = inst.type_id;
				inst.result_id = inst.result_id;
				inst.operands = std::vector<spv_parsed_operand_t>(parsed_inst.operands, parsed_inst.operands + parsed_inst.num_operands);
				inst.offset = word_offset;

				if (word_offset >= word_offset_to_instruction.size())
				{
					word_offset_to_instruction.resize(word_offset + 1);
				}

				word_offset_to_instruction[word_offset] = inst;
				instructions.emplace_back(inst);

				word_offset += parsed_inst.num_words;

				return spv_result_t::SPV_SUCCESS;
			}

			spv_context_t& GetContext()
			{
				return *context;
			}

			spvtools::AssemblyGrammar& GetGrammar()
			{
				return *grammar;
			}

			spvtools::FriendlyNameMapper& GetNameMapper()
			{
				return *name_mapper;
			}

			static spv_result_t SPVHandleHeader(void* user_data, spv_endianness_t endian,
												uint32_t /* magic */, uint32_t version,
												uint32_t generator, uint32_t id_bound,
												uint32_t schema)
			{
				assert(user_data);
				auto spv_instruction_manager = static_cast<SPVInstructionManager*>(user_data);
				return spv_instruction_manager->HandleHeader(endian, version, generator,
															 id_bound, schema);
			}

			static spv_result_t SPVHandleInstruction(void* user_data, const spv_parsed_instruction_t* parsed_instruction)
			{
				assert(user_data);
				auto disassembler = static_cast<SPVInstructionManager*>(user_data);
				return disassembler->HandleInstruction(*parsed_instruction);
			}

			Expected<const SPVInstruction&> GetInstructionFromOffset(std::size_t offset)
			{
				if (const auto& instruction = word_offset_to_instruction[offset])
				{
					return *instruction;
				}
				return StringError("Could not find matching instruction from offset");
			}

			const std::vector<SPVInstruction>& GetInstructions()
			{
				return instructions;
			}

		private:
			using SPVContextPtr = std::unique_ptr<spv_context_t, std::function<void(spv_context_t*)>>;
			SPVContextPtr context{};
			std::unique_ptr<spvtools::AssemblyGrammar> grammar{};
			std::unique_ptr<spvtools::FriendlyNameMapper> name_mapper{};

			const uint32_t SPV_INDEX_INSTRUCTION = 5u;
			const uint32_t WORD_SIZE = sizeof(uint32_t);
			std::size_t word_offset{};

			//flat_hash_map<std::size_t, SPVInstruction> word_offset_to_instruction;
			std::vector<std::optional<SPVInstruction>> word_offset_to_instruction;
			std::vector<SPVInstruction> instructions;
		};

		class SPIRVMVariable
		{
		public:
			explicit SPIRVMVariable() = default;
			explicit SPIRVMVariable(spvm_result_t pointer_type_, spvm_member_t* owner_member_, std::vector<spvm_member_t> members_) :
				pointer_type(pointer_type_), owner_member(owner_member_), members(members_)
			{
			}

			spvm_result_t& GetPointerType()
			{
				return pointer_type;
			}

			spvm_member_t* GetOwnerMember()
			{
				return owner_member;
			}

			std::vector<spvm_member_t>& GetMembers()
			{
				/*
				std::vector<spvm_member_t> members_(pointer_type->member_count);
				for (auto i = 0; i < pointer_type->member_count; i++)
				{
					members_[i] = &pointer_type->members[i];
				}
				*/
				return members;
			}

		private:
			spvm_result_t pointer_type{};
			spvm_member_t* owner_member{};
			std::vector<spvm_member_t> members{};
		};

		// Used by Run() function
		using SpvmStatePtr = std::shared_ptr<spvm_state>;

		struct PositionAffectedInstructionWithLine
		{
			uint64_t line;
			PositionAffectedInstruction inst;
		};

		struct CommonSetupRunInfo
		{
			std::vector<PositionAffectedInstructionWithLine> position_affected_lines_vector;
			phmap::flat_hash_set<uint32_t> word_offset_executed;
			std::vector<std::size_t> line_to_word_offset;
			uint32_t position_id;
			glm::vec4 original_position;
			std::vector<std::string> id_to_name;
		};

		struct RunResult
		{
			glm::mat4 model;
			glm::mat4 projection_view;
		};

		using PositionIndices = small_vector<uint32_t, 4>;
		struct PositionView
		{
			uint32_t id;
			glm::mat4 m = glm::mat4(1.0f);
			PositionIndices position_indices;
			/*
			std::function<void(glm::mat4&, std::size_t)> opdot_callback;

			void OpDotCallback(PositionView& o, std::size_t i)
			{
				if (opdot_callback)
				{
					opdot_callback(o.m, i);
					opdot_callback = {};
				}
			}
			*/
		};

		constexpr glm::mat4 DEFAULT_MATRIX = glm::mat4(1.0f);
		
		struct TransformationOrdering
		{
			PositionView* position_view;
			bool right;
		};

		struct SPIRVVirtualMachineState
		{
			SpvmStatePtr vm_state{ nullptr };
			std::shared_ptr<spvm_ext_opcode_func> glsl_ext_data{};
			flat_hash_map<std::string, SPIRVMVariable> name_to_variable_cache;
			std::vector<std::vector<float*>> id_to_spv_floats;
			spvm_word main_function{};

			flat_hash_map<std::string, std::vector<float*>> name_to_spv_floats_cache;

			// per run
			std::vector<std::optional<PositionView>> id_to_position_view;
			std::vector<std::function<void(glm::mat4&, std::size_t)>> position_view_opdot_callbacks;
			small_vector<TransformationOrdering, RAYSTERIZER_RUN_TRANSFORMATION_BUFFER_SIZE> transformations_orderings;

			bool cached_appended_ordering_ready = false;
			std::vector<std::optional<std::vector<std::vector<float*>>>> cached_appended_ordering;
			std::vector<std::vector<PositionView>> cached_appended_position_view;
		};

		struct SetupRunInfo
		{
			std::shared_ptr<CommonSetupRunInfo> common_info;
			SPIRVVirtualMachineState spirv_vm_state;
		};

		class SPIRVVirtualMachine
		{
		public:
			explicit SPIRVVirtualMachine(std::vector<uint32_t> spv_);
			SPIRVVirtualMachineState CreateMachineState();
			void CacheNamesToMember(SPIRVVirtualMachineState& spirv_vm_state, std::string name, spvm_result_t type, spvm_member_t& owner_member, spvm_member_t& members, spvm_word member_count);

			Expected<RunResult> CacheSetupRunInfo(SetupRunInfo& setup_run_info);
			SetupRunInfo SetupRun();
			Expected<RunResult> RunAfterSetup(SetupRunInfo& setup_run_info);

			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, int data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::ivec2 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::ivec3 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::ivec4 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, unsigned int data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::uvec2 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::uvec3 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::uvec4 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, float data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::vec2 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::vec3 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::vec4 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::mat2 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::mat3 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::mat4 data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, PointerView view);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, std::vector<glm::vec2> data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, std::vector<glm::vec3> data);
			Error SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, std::vector<glm::vec4> data);

			Expected<glm::vec4> GetVariableVec4(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name);
			Expected<glm::mat4> GetVariableMat4(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name);
			Expected<const std::vector<float*>&> GetVariableInBytesRef(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name);

			Expected<SPIRVMVariable&> GetUnderlyingVMType(SPIRVVirtualMachineState& spirv_vm_state, std::string_view var);

			Error AssociateUnderlyingVMType(SPIRVVirtualMachineState& dst, SPIRVVirtualMachineState& src, std::string_view var);
			Error DeassociateUnderlyingVMType(SPIRVVirtualMachineState& dst, std::string_view var);

		private:
			std::vector<uint32_t> spv{};
			std::unique_ptr<SPIRVAnalyzer> analyzer{};
			SPVInstructionManager inst_manager{};

			using SpvmContextPtr = std::unique_ptr<spvm_context, std::function<void(spvm_context*)>>;
			SpvmContextPtr ctx{ nullptr };
			using SpvmProgPtr = std::unique_ptr<spvm_program, std::function<void(spvm_program*)>>;
			SpvmProgPtr vm_prog{ nullptr };
			std::shared_ptr<spvm_ext_opcode_func> glsl_ext_data{};

			std::size_t mvp_matrix_id{};

			std::mutex save_state_mutex;

			std::atomic<bool> is_cached_appended_ordering = false;
			std::mutex cached_appended_ordering_m;
			std::condition_variable cached_appended_ordering_cv;
			bool cached_appended_ordering_ready = false;
			flat_hash_map<uint32_t, std::vector<uint32_t>> cached_appended_ordering;

			std::atomic<bool> is_cached_transformation_ordering = false;
			std::vector<uint32_t> cached_transformation_ordering;

			std::shared_ptr<CommonSetupRunInfo> common_info;
		};
	}
}