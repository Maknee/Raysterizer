#include "spirv_virtual_machine.h"

namespace Raysterizer
{
	namespace Analysis
	{
		namespace
		{
			std::vector<spvm_member_t> MembersToVec(spvm_member_t members, spvm_word member_count)
			{
				std::vector<spvm_member_t> members_vec(member_count);
				for (auto i = 0; i < member_count; i++)
				{
					members_vec[i] = &members[i];
				}

				return members_vec;
			}

			void GetSpvResultMember(std::vector<float*>& data, spvm_state_t state, spvm_member mem)
			{
				for (auto i = 0; i < mem.member_count; i++)
				{
					auto& member = mem.members[i];

					if (member.member_count == 0)
					{
						data.push_back(&member.value.f);
					}
					else
					{
						GetSpvResultMember(data, state, member);
					}
				}
			}

			void GetSpvResult(std::vector<float*>& data, spvm_state_t state, int id)
			{
				auto& result = state->results[id];
				auto member_count = result.member_count;
				for (auto i = 0; i < member_count && result.members; i++)
				{
					auto& member = result.members[i];

					if (member.member_count == 0)
					{
						data.push_back(&member.value.f);
					}
					else
					{
						GetSpvResultMember(data, state, member);
					}
				}
			}

			std::vector<float*> GetSpvFloats(spvm_state_t state, int id)
			{
				std::vector<float*> data{};
				GetSpvResult(data, state, id);
				return data;
			}

			std::vector<float*> GetSpvFloats2(spvm_state_t state, int id)
			{
				return GetSpvFloats(state, id);
			}

			std::vector<float> GetSpvFloatsVec(spvm_state_t state, int id)
			{
				std::vector<float*> data = GetSpvFloats(state, id);
				std::vector<float> data2;
				std::transform(std::begin(data), std::end(data), std::back_inserter(data2), [](const auto& e) { return *e; });
				return data2;
			}
		}

		SPIRVVirtualMachine::SPIRVVirtualMachine(std::vector<uint32_t> spv_) :
			spv(std::move(spv_))
		{
			/*
			spvtools::Optimizer opt(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
			{
				std::vector<uint32_t> spv_optimized{};
				opt.RegisterPerformancePasses().Run(spv.data(), spv.size(), &spv_optimized);
				spv = std::move(spv_optimized);
			}
			*/

			// Run this and optimize spv
			analyzer = std::make_unique<SPIRVAnalyzer>(spv);
			PanicIfError(analyzer->Run());

			inst_manager.Init(spv_target_env::SPV_ENV_UNIVERSAL_1_5, spv);

			// Source code
			spvm_source spv_source = reinterpret_cast<spvm_source>(spv.data());
			auto spv_length = spv.size();

			// Context
			ctx = SpvmContextPtr(spvm_context_initialize(), &spvm_context_deinitialize);

			// Setup program and state
			vm_prog = SpvmProgPtr(spvm_program_create(ctx.get(), spv_source, spv_length), &spvm_program_delete);
		}

		SPIRVVirtualMachineState SPIRVVirtualMachine::CreateMachineState()
		{
			auto vm_state = SpvmStatePtr(spvm_state_create(vm_prog.get()), &spvm_state_delete);

			// Load extension
			auto glsl_std_450 = spvm_state_get_result(vm_state.get(), "GLSL.std.450");
			if (glsl_std_450)
			{
				if (!glsl_ext_data)
				{
					glsl_ext_data = std::shared_ptr<spvm_ext_opcode_func>(spvm_build_glsl450_ext());
				}
				glsl_std_450->extension = glsl_ext_data.get();
			}
			else
			{
				PANIC("Ensure SPV has #version 450");
			}

			SPIRVVirtualMachineState spirv_vm_state;
			spirv_vm_state.vm_state = std::move(vm_state);
			spirv_vm_state.glsl_ext_data = glsl_ext_data;

			//
			auto* state = spirv_vm_state.vm_state.get();
			spirv_vm_state.id_to_spv_floats.resize(state->owner->bound);

			//
			spirv_vm_state.main_function = spvm_state_get_result_location(state, "main");
			spvm_state_prepare(state, spirv_vm_state.main_function);

			for (spvm_word i = 0; i < state->owner->bound; i++)
			{
				std::vector<float*> float_variables = GetSpvFloats(state, i);
				spirv_vm_state.id_to_spv_floats[i] = std::move(float_variables);

				spvm_result_t slot = &state->results[i];
				auto& name = slot->name;
				auto& type = slot->type;
				auto& owner = slot->owner;
				auto& members = slot->members;
				auto& member_count = slot->member_count;
				if ((type == spvm_result_type_variable || type == spvm_result_type_function_parameter)
					&& name != nullptr)
				{
					spvm_result_t vm_type = spvm_state_get_type_info(state->results, &state->results[slot->pointer]);
					if (owner == nullptr || owner == state->current_function)
					{
						auto& owner_member = slot->members;
						CacheNamesToMember(spirv_vm_state, name, vm_type, owner_member, members, member_count);
					}
				}
			}

			if (0)
			{
				for (auto& [name, member] : spirv_vm_state.name_to_variable_cache)
				{
					fmt::print("{}\n", name);
				}

				fmt::print("\n");
			}

			return spirv_vm_state;
		}

		void SPIRVVirtualMachine::CacheNamesToMember(SPIRVVirtualMachineState& spirv_vm_state, std::string name, spvm_result_t type, spvm_member_t& owner_member, spvm_member_t& members, spvm_word member_count)
		{
			auto* state = spirv_vm_state.vm_state.get();
			auto& name_to_variable_cache = spirv_vm_state.name_to_variable_cache;

			SPIRVMVariable spirvvm_variable(type, &owner_member, MembersToVec(members, member_count));
			name_to_variable_cache[name] = spirvvm_variable;

			for (auto i = 0; i < member_count; i++)
			{
				auto& member = members[i];
				auto& member_name = type->member_name[i];

				auto member_type = type;

				if (member.type != 0 && member.type != -1)
				{
					member_type = spvm_state_get_type_info(state->results, &state->results[member.type]);
				}
				if (member_type->member_count > 1 &&
					member_type->pointer != 0 &&
					member_type->value_type != spvm_value_type_matrix &&
					member_type->value_type != spvm_value_type_array)
				{
					member_type = spvm_state_get_type_info(state->results, &state->results[member_type->pointer]);
				}

				if (member.member_count == 0)
				{
					memset(&member_type->value_type, 0, sizeof(member_type->value_type));
					if (member_type->value_type == spvm_value_type_float && type->value_bitcount <= 32)
					{
						//float
					}
					else if (member_type->value_type == spvm_value_type_float)
					{
						//double
					}
					else
					{
						//int
					}
					
					auto new_name = name;
					if (type->type == spvm_value_type_struct)
					{
						new_name += fmt::format(".{}", member_name);
					}
					if (type->type == spvm_value_type_vector ||
						type->type == spvm_value_type_matrix ||
						type->type == spvm_value_type_array ||
						type->type == spvm_value_type_runtime_array)
					{
						new_name += fmt::format("[{}]", i);
					}

					SPIRVMVariable spirvvm_variable(member_type, &members, MembersToVec(&member, 1));
					name_to_variable_cache[new_name] = spirvvm_variable;
				}
				else
				{
					if ((type->value_type == spvm_value_type_runtime_array || type->value_type == spvm_value_type_array)
						&& i > 2 && i < member_count - 3)
					{
						//PANIC("IGNORED?");
						//continue;
					}
					
					auto new_name = name;
					if (type->value_type == spvm_value_type_struct)
					{
						if (name != "")
						{
							new_name += fmt::format(".");
						}
						new_name += fmt::format("{}", member_name);
					}
					if (type->value_type == spvm_value_type_matrix || 
						type->value_type == spvm_value_type_array || 
						type->value_type == spvm_value_type_runtime_array)
					{
						new_name += fmt::format("[{}]", i);
					}

					if ((type->value_type == spvm_value_type_runtime_array || type->value_type == spvm_value_type_array) && 
						member_count > 6 &&
						i == member_count - 3)
					{
						//PANIC("IGNORED?");
						//continue;
					}

					CacheNamesToMember(spirv_vm_state, new_name, member_type, members, member.members, member.member_count);
				}
			}
		}

		namespace
		{
			void DumpSpvResultMember(std::stringstream& ss, spvm_state_t state, spvm_member mem, int tabs)
			{
				auto OutputToStream = [&](auto str, int tabs2)
				{
					ss << std::string(tabs2, '\t') << str << "\n";
				};

				for (auto i = 0; i < mem.member_count; i++)
				{
					const auto& member = mem.members[i];

					if (member.member_count == 0)
					{
						ss << fmt::format("{}", member.value.f);
						if (i != mem.member_count - 1)
						{
							ss << ", ";
						}
					}
					else
					{
						ss << "[ ";
						DumpSpvResultMember(ss, state, member, tabs + 1);
						ss << " ]";
					}
				}
			}

			void DumpSpvResult(std::stringstream& ss, spvm_state_t state, int id, int tabs)
			{
				auto OutputToStream = [&](auto str, int tabs2)
				{
					ss << std::string(tabs2, '\t') << str << "\n";
				};

				//OutputToStream(fmt::format("ID: [{}]", id), tabs);
				ss << std::string(tabs, '\t') << fmt::format("[{}] ", id);

				const auto& result = state->results[id];
				if (result.name)
				{
					//OutputToStream(fmt::format("Name: {}", result.name), tabs);
					ss << fmt::format("[{}] ", result.name);
				}

				auto member_count = result.member_count;

				if (member_count)
				{
					//OutputToStream("[", tabs);
					ss << "[";
				}
				for (auto i = 0; i < member_count && result.members; i++)
				{
					const auto& member = result.members[i];

					if (member.member_count == 0)
					{
						ss << fmt::format("{}", member.value.f);
						if (i != member_count - 1)
						{
							ss << ", ";
						}
					}
					else
					{
						ss << "[";
						DumpSpvResultMember(ss, state, member, tabs + 1);
						ss << "] ";
					}
				}
				if (member_count)
				{
					//OutputToStream("]", tabs);
					ss << "]";
				}
				ss << "\n";
			}
		}

		SetupRunInfo SPIRVVirtualMachine::SetupRun()
		{
			auto spirv_vm_state = CreateMachineState();

			if (!common_info)
			{
				// Call main
				auto* state = spirv_vm_state.vm_state.get();

				SPVInstructionManager inst_manager(spv_target_env::SPV_ENV_UNIVERSAL_1_5, spv);
				const auto position_id = analyzer->GetPositionId();
				const flat_hash_map<uint32_t, PositionAffectedInstruction>& position_affected_lines = analyzer->GetPositionAffectedLines();

				auto GetCurrentVec = [&](uint32_t id)
				{
					auto position_ref = GetSpvFloats(state, id);
					std::vector<float> original_position_vec(position_ref.size(), 1.0f);
					std::transform(std::begin(position_ref), std::end(position_ref), std::begin(original_position_vec), [](const float* e)
					{
						return *e;
					});
					for (auto i = original_position_vec.size(); i < 4; i++)
					{
						original_position_vec.emplace_back(1.0f);
					}

					if (original_position_vec[3] != 1.0f)
					{
						//PANIC("w not 1.0f??");
					}

					// force w to be 1.0f
					original_position_vec[3] = 1.0f;
					auto original_position = glm::make_vec4(original_position_vec.data());
					return original_position;
				};

				auto original_position = GetCurrentVec(position_id);

				spvm_source code_start = reinterpret_cast<spvm_source>(spv.data());

				phmap::flat_hash_set<uint32_t> position_touched_ids{};
				flat_hash_map<uint32_t, std::vector<float*>> id_to_position_view{};

				position_touched_ids.emplace(position_id);
				id_to_position_view[position_id] = GetSpvFloats(state, position_id);

				auto GetPositionViewAffectedById = [&](std::size_t id)
				{
					if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
					{
						return found->second;
					}
					PANIC("Not found");
					return std::vector<float*>{};
				};

				std::vector<std::size_t> line_to_word_offset;
				phmap::flat_hash_set<uint32_t> word_offset_executed;
				flat_hash_map<std::size_t, std::string> word_offset_to_disassembly;
				auto main_code_current = state->code_current;
				while (state->code_current)
				{
					auto code_current = state->code_current;
					auto word_offset = code_current - code_start;
					word_offset_executed.emplace(word_offset);

					spvm_word opcode_data = code_current[0];
					spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift);
					SpvOp opcode = static_cast<SpvOp>(opcode_data & SpvOpCodeMask);
					auto inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));

					auto line_spv = std::vector<uint32_t>(std::begin(spv), std::begin(spv) + 5);
					for (auto i = 0; i < word_count; i++)
					{
						line_spv.emplace_back(code_current[i]);
					}
					//spv::Disassemble(std::cout, line_spv);
					const auto& disassembly = word_offset_to_disassembly[word_offset];

					if (0 && disassembly.find("%76 = OpLoad ") != std::string::npos)
					{
						auto mul_floats = GetSpvFloats(state, 73);
						auto mul_floats2 = GetSpvFloats(state, 75);
						fmt::print("");
					}

					word_offset_to_disassembly.emplace(word_offset, disassembly);

					//if (Raysterizer::Analysis::IsOpcodeSkippable(opcode))
					if (spvOpcodeIsDebug(opcode))
					{
						spvm_state_step_opcode(state);
						continue;
					}
					//fmt::print("{}", disassembly);

					//line_to_word_offset.emplace_back(word_offset);
					spvm_state_step_opcode(state);
				}

				{
					auto word_offset = main_code_current - code_start;
					auto inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));
					while (inst.opcode != SpvOpFunctionEnd)
					{
						inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));
						auto code_current = code_start + word_offset;

						spvm_word opcode_data = code_current[0];
						spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift);
						SpvOp opcode = static_cast<SpvOp>(opcode_data & SpvOpCodeMask);
						auto inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));

						auto line_spv = std::vector<uint32_t>(std::begin(spv), std::begin(spv) + 5);
						for (auto i = 0; i < word_count; i++)
						{
							line_spv.emplace_back(code_current[i]);
						}

						std::string disassembly{};
						spvtools::SpirvTools spirv_tools(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
						auto disassemble_result = spirv_tools.Disassemble(line_spv,
																			&disassembly,
																			SPV_BINARY_TO_TEXT_OPTION_INDENT |
																			SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
																			SPV_BINARY_TO_TEXT_OPTION_NO_HEADER);
						if (!disassemble_result)
						{
							//PANIC("Failed to diassemble");
						}

						auto this_word_count = word_offset;
						word_offset += word_count;

						if (spvOpcodeIsDebug(inst.opcode))
						{
							continue;
						}
						//fmt::print("{} {}", this_word_count, disassembly);

						line_to_word_offset.emplace_back(this_word_count);
					}
				}

				std::vector<PositionAffectedInstructionWithLine> position_affected_lines_vector;
				for (const auto& [line, inst] : position_affected_lines)
				{
					position_affected_lines_vector.emplace_back(PositionAffectedInstructionWithLine{ line, inst });
				}
				std::sort(std::begin(position_affected_lines_vector), std::end(position_affected_lines_vector),
							[](const auto& e1, const auto& e2)
				{
					return e1.line < e2.line;
				});

				for (auto iter = std::begin(position_affected_lines_vector); iter != std::end(position_affected_lines_vector);)
				{
					auto& position_affected_lines = *iter;
					auto line = position_affected_lines.line;

					auto word_offset = line_to_word_offset[line];
					if (!word_offset_executed.contains(word_offset))
					{
						iter = position_affected_lines_vector.erase(iter);
					}
					else
					{
						iter++;
					}
				}

				std::vector<std::string> id_to_name(state->owner->bound);
				for (auto i = 0; i < state->owner->bound; i++)
				{
					auto name = inst_manager.GetNameMapper().NameForId(i);
					id_to_name[i] = name;
				}

				common_info = std::make_shared<CommonSetupRunInfo>(CommonSetupRunInfo{ position_affected_lines_vector, word_offset_executed, line_to_word_offset, position_id, original_position, id_to_name });
			}

			SetupRunInfo setup_run_info{ common_info, std::move(spirv_vm_state) };

			return setup_run_info;
		}

		Expected<RunResult> SPIRVVirtualMachine::CacheSetupRunInfo(SetupRunInfo& setup_run_info)
		{
			ScopedCPUProfile("SPIRV VM RunAfterSetup");

			const auto& common_info = setup_run_info.common_info;
			const auto& position_affected_lines_vector = common_info->position_affected_lines_vector;
			const auto& word_offset_executed = common_info->word_offset_executed;
			const auto& line_to_word_offset = common_info->line_to_word_offset;
			const auto& position_id = common_info->position_id;
			const auto& id_to_name = common_info->id_to_name;

			SPIRVVirtualMachineState& spirv_vm_state = setup_run_info.spirv_vm_state;;

			auto& vm_state = spirv_vm_state.vm_state;
			auto& name_to_variable_cache = spirv_vm_state.name_to_variable_cache;
			auto& id_to_spv_floats = spirv_vm_state.id_to_spv_floats;
			const auto& main_function = spirv_vm_state.main_function;

			auto& cached_appended_ordering_ready = spirv_vm_state.cached_appended_ordering_ready;
			auto& cached_appended_ordering = spirv_vm_state.cached_appended_ordering;
			auto& cached_appended_position_view = spirv_vm_state.cached_appended_position_view;

			// restore state
			auto* state = vm_state.get();
			spvm_state_prepare(state, main_function);

			auto GetSpvFloats = [&id_to_spv_floats](int id) -> std::vector<float*>&
			{
				return id_to_spv_floats[id];
			};

			auto& position_data = GetSpvFloats(position_id);
			auto original_position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			for (auto i = 0; i < position_data.size(); i++)
			{
				original_position[i] = *(position_data[i]);
			}
			original_position.w = 1.0f;

			spvm_source code_start = reinterpret_cast<spvm_source>(spv.data());

			auto& transformations_orderings = spirv_vm_state.transformations_orderings;
			std::vector<std::optional<PositionView>>& id_to_position_view = spirv_vm_state.id_to_position_view;
			std::vector<std::function<void(glm::mat4&, std::size_t)>>& position_view_opdot_callbacks = spirv_vm_state.position_view_opdot_callbacks;

			transformations_orderings.clear();
			//id_to_position_view.clear();
			id_to_position_view.resize(state->owner->bound);
			position_view_opdot_callbacks.resize(state->owner->bound);

			cached_appended_ordering.resize(state->owner->bound);
			cached_appended_position_view.resize(state->owner->bound);

			/*
			std::deque<TransformationOrdering> transformations_orderings;
			
			std::vector<std::optional<PositionView>> id_to_position_view;
			id_to_position_view.resize(state->owner->bound);
			*/

			bool last_inserted_transformation_ordering_right = false;
			
			//
			auto AddPositionViewFront = [&](uint32_t copy_id, uint32_t id) -> PositionView&
			{
				auto& copied_position_view = id_to_position_view[copy_id];
				
				auto& pv = id_to_position_view[id];
				//pv = PositionView{ id, glm::mat4(1.0f) };
				if (pv)
				{
					pv->id = id;
					pv->m = glm::mat4(1.0f);
				}
				else
				{
					pv = PositionView{ id, glm::mat4(1.0f) };
				}

				transformations_orderings.emplace_back(TransformationOrdering{ &*pv, false });
				last_inserted_transformation_ordering_right = false;

				return *pv;
			};

			auto AddPositionViewBack = [&](uint32_t copy_id, uint32_t id) -> PositionView&
			{
				auto& copied_position_view = id_to_position_view[copy_id];

				auto& pv = id_to_position_view[id];
				//pv = PositionView{ id, glm::mat4(1.0f) };
				if (pv)
				{
					pv->id = id;
					pv->m = glm::mat4(1.0f);
				}
				else
				{
					pv = PositionView{ id, glm::mat4(1.0f) };
				}

				transformations_orderings.emplace_back(TransformationOrdering{ &*pv, true });
				last_inserted_transformation_ordering_right = true;

				return *pv;
			};

			auto AddPositionViewNoRef = [&](uint32_t id, bool insert_back = true, PositionIndices&& indices = {}, glm::mat4 mm = glm::mat4(1.0f))-> PositionView&
			{
				if (insert_back)
				{
					auto& pv = id_to_position_view[id];
					//pv = PositionView{ id, glm::mat4(1.0f), std::move(indices) };
					if (pv)
					{
						pv->id = id;
						pv->m = glm::mat4(1.0f);
						pv->position_indices = std::move(indices);
					}
					else
					{
						pv = PositionView{ id, glm::mat4(1.0f), std::move(indices) };
					}

					transformations_orderings.emplace_back(TransformationOrdering{ &*pv, true });
					last_inserted_transformation_ordering_right = true;

					return *pv;
				}
				else
				{
					auto& pv = id_to_position_view[id];
					//pv = PositionView{ id, glm::mat4(1.0f), std::move(indices) };
					if (pv)
					{
						pv->id = id;
						pv->m = glm::mat4(1.0f);
						pv->position_indices = std::move(indices);
					}
					else
					{
						pv = PositionView{ id, glm::mat4(1.0f), std::move(indices) };
					}

					transformations_orderings.emplace_back(TransformationOrdering{ &*pv, false });
					last_inserted_transformation_ordering_right = false;

					return *pv;
				}
			};

			// kick it off
			//id_to_position_view[position_id] = PositionView{ glm::mat4(1.0f), {0, 1, 2, 3} };
			//transformations_orderings.emplace_back(TransformationOrdering{ &id_to_position_view[position_id] });
			AddPositionViewNoRef(position_id, true, { 0, 1, 2, 3 });

			glm::vec4 current_position = original_position;

			uint64_t current_word_offset = 0;
			for (const auto& position_affected_lines : position_affected_lines_vector)
			{
				auto line = position_affected_lines.line;
				auto position_affected_id = position_affected_lines.inst.position_affected_id;
				const auto& debug_line = position_affected_lines.inst.debug_line;

				// optimization to call only necessary spirv instructions to buld the target
				{
					auto target_word_offset = line_to_word_offset[line];
					current_word_offset = state->code_current - code_start;
					//fmt::print("WTF {} {} {} {} {}\n\n", line, debug_line, (uint64_t)current_word_offset, (uint64_t)target_word_offset, target_word_offset - current_word_offset);

					while (current_word_offset <= target_word_offset)
					{
						current_word_offset = state->code_current - code_start;
						auto code_current = state->code_current;

						spvm_word opcode_data = code_current[0];
						spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift);
						SpvOp opcode = static_cast<SpvOp>(opcode_data & SpvOpCodeMask);

						if (0)
						{
							auto line_spv = std::vector<uint32_t>(std::begin(spv), std::begin(spv) + 5);
							for (auto i = 0; i < word_count; i++)
							{
								line_spv.emplace_back(code_current[i]);
							}

							spvtools::SpirvTools spirv_tools(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
							std::string disassembly{};
							if (!spirv_tools.Disassemble(line_spv,
								&disassembly,
								SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
								SPV_BINARY_TO_TEXT_OPTION_INDENT |
								SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
								SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET |
								SPV_BINARY_TO_TEXT_OPTION_COMMENT))
							{
								//PANIC("Failed to diassemble");
							}
							fmt::print("!!! {} {} {} {} | {}\n", (uint64_t)current_word_offset, word_count, (uint64_t)target_word_offset, target_word_offset - current_word_offset, disassembly);
							if (1 && disassembly.find("%76 = OpLoad ") != std::string::npos)
							{
								auto& mul_floats1 = GetSpvFloats(73);
								auto& mul_floats2 = GetSpvFloats(75);
								auto& mul_floats3 = GetSpvFloats(76);
								fmt::print("");
							}
						}

						// step over function (this loop is to handle function calls)
						if (opcode == SpvOpFunctionCall)
						{
							auto expected_next_instruction_index = code_current + word_count;
							do
							{
								spvm_state_step_opcode(state);
							} while (state->code_current != 0 && expected_next_instruction_index != state->code_current);
						}
						else
						{
							spvm_state_step_opcode(state);
						}
					}
				}

				auto word_offset = line_to_word_offset[line];
				const SPVInstruction& inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));

				const SpvOp& opcode = inst.opcode;
				uint32_t type_id = inst.type_id;
				uint32_t result_id = inst.result_id;
				const std::vector<uint32_t>& words = inst.words;
				const std::vector<spv_parsed_operand_t>& operands = inst.operands;

				// for some reason type_id and result_id is sometimes not parsed
				for (const auto& operand : operands)
				{
					if (operand.type == SPV_OPERAND_TYPE_TYPE_ID)
					{
						type_id = words[operand.offset];
					}
					else if (operand.type == SPV_OPERAND_TYPE_RESULT_ID)
					{
						result_id = words[operand.offset];
					}
				}

				auto op_start = (type_id > 0 ? 1 : 0) + (result_id > 0 ? 1 : 0);
				auto num_in_operands = operands.size() - op_start;
				const spv_parsed_operand_t* in_operands = operands.data() + op_start;

				/*
				auto OperandWords = [&](const spv_parsed_operand_t& operand)
				{
					auto offset = operand.offset;
					auto num_words = operand.num_words;
					std::vector<uint32_t> operand_words(num_words);
					for (auto i = offset; i < offset + num_words; i++)
					{
						operand_words[i - offset] = words[offset];
					}
					return operand_words;
				};
				*/
				auto GetSingleInOperandWord = [&](std::size_t i)
				{
					/*
					const auto& in_operand = in_operands[i];
					auto operand_words = OperandWords(in_operand);
					if (operand_words.size() != 1)
					{
						PANIC("Expected 1 word");
					}
					return operand_words[0];
					*/
					const auto& in_operand = in_operands[i];
					auto offset = in_operand.offset;
					auto num_words = in_operand.num_words;
					if (num_words != 1)
					{
						PANIC("Expected 1 word");
					}
					return words[offset];
				};

				switch (opcode)
				{
				case SpvOpAccessChain:
				{
					//%83 = OpAccessChain %_ptr_Function_float %_631 %uint_0 ; 0x000038d8
					//...
					//%84 = OpLoad %float %83 ; 0x000038ec
					//%93 = OpCompositeConstruct %v4float %84 %87 %90 %92 ; 0x00003968
					//OpStore %_633 %93 ; 0x00003984
					//vec4 _633 = vec4(_631.x, _631.y, _631.z, POSITION.w);
					const auto base_id = GetSingleInOperandWord(0);
					if (num_in_operands != 2)
					{
						//continue;
						//PANIC("Access should only be one deference");
					}
					// TODO: This assumes the last index is the index we are going for (not always true since you could have double array indexing)
					const auto memory_access_id = GetSingleInOperandWord(num_in_operands - 1);
					float* access_index_ptr = GetSpvFloats(memory_access_id)[0];
					auto access_index = *(uint32_t*)access_index_ptr;

					if (auto& pv = id_to_position_view[base_id])
					{
						auto position_view = AddPositionViewBack(base_id, result_id);
						position_view.position_indices = { access_index };
					}
					break;
				}
				case SpvOpVectorShuffle:
				{
					//%73 = OpVectorShuffle % v3float % 72 % 72 0 1 2; 0x00003808
					//vec4 _633 = vec4(_631.x, _631.y, _631.z, POSITION.w);
					const auto base_id = GetSingleInOperandWord(0);
					const auto base_id2 = GetSingleInOperandWord(1);
					auto start_index = 2;
					auto end_index = num_in_operands;
					if (base_id != base_id2)
					{
						//PANIC("Assume shuffle same vec");
						//Assume the one with the least has the correct start index
					}

					PositionIndices shuffle_indices;

					if (base_id != base_id2)
					{
						// Assume it's all of the vector?
						for (auto i = start_index; i < end_index; i++)
						{
							auto index = i - start_index;
							shuffle_indices.emplace_back(index);
						}
					}
					else
					{
						for (auto i = start_index; i < end_index; i++)
						{
							auto index = GetSingleInOperandWord(i);
							shuffle_indices.emplace_back(index);
						}
					}
					if (auto& pv = id_to_position_view[base_id])
					{
						auto& position_view = AddPositionViewBack(base_id, result_id);
						position_view.position_indices = shuffle_indices;
					}
					if (base_id != base_id2)
					{
						if (auto& pv = id_to_position_view[base_id])
						{
							auto& position_view = AddPositionViewBack(base_id2, result_id);
							position_view.position_indices = shuffle_indices;
						}
					}
					break;
				}
				case SpvOpLoad:
				{
					//%72 = OpLoad %v4float %POSITION ; 0x000037f8
					const auto base_id = GetSingleInOperandWord(0);
					if (num_in_operands != 1)
					{
						PANIC("Load should only be one deference");
					}
					if (auto& pv = id_to_position_view[base_id])
					{
						AddPositionViewBack(base_id, result_id);
					}
					break;
				}
				case SpvOpFMul:
				{
					//%80 = OpFMul %v3float %73 %79
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[id1])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id2);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[i][i] = *(mul_floats[i]);
						}
						auto& position_view = AddPositionViewBack(id1, result_id);
						position_view.m = m;
					}
					if (auto& pv = id_to_position_view[id2])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id1);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[i][i] = *(mul_floats[i]);
						}
						auto& position_view = AddPositionViewBack(id2, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpDot:
				{
					//%101 = OpDot %float %99 %100 ; 0x00003a10
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (num_in_operands != 2)
					{
						PANIC("Load should only be one deference");
					}
					if (auto& pv = id_to_position_view[id1])
					{
						// have to be resolved later
						auto& position_view = AddPositionViewBack(id1, result_id);
						position_view_opdot_callbacks[result_id] = [&GetSpvFloats, &state, &id_to_position_view, result_id, id2](glm::mat4& m, std::size_t row)
						{
							if (auto& pv = id_to_position_view[result_id])
							{
								auto& floats = GetSpvFloats(id2);
								for (auto i = 0; i < floats.size(); i++)
								{
									//m[row][i] = *(floats[i]);
									m[i][row] = *(floats[i]);
								}
							}
						};
					}
					else if (auto& pv = id_to_position_view[id2])
					{
						// have to be resolved later
						auto& position_view = AddPositionViewFront(id2, result_id);
						position_view_opdot_callbacks[result_id] = [&GetSpvFloats, &state, &id_to_position_view, result_id, id1](glm::mat4& m, std::size_t row)
						{
							if (auto& pv = id_to_position_view[result_id])
							{
								auto& floats = GetSpvFloats(id1);
								for (auto i = 0; i < floats.size(); i++)
								{
									//m[row][i] = *(floats[i]);
									m[i][row] = *(floats[i]);
								}
							}
						};
					}
					break;
				}
				case SpvOpCompositeConstruct:
				{
					//%93 = OpCompositeConstruct %v4float %84 %87 %90 %92
					PositionIndices indices(num_in_operands);
					std::iota(std::begin(indices), std::end(indices), 0);

					auto num_finds = 0;
					auto& position_view = AddPositionViewNoRef(result_id, true, std::move(indices));
					auto& t = position_view.m;
					for (auto i = 0; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);

						const auto& name = id_to_name[type_id];
						if (name == "v4float")
						{
							auto& values = GetSpvFloats(id);
							if (values.size() != 1)
							{
								PANIC("Only expected 1 float per index");
							}
							auto v = (*values[0]);

							if (auto& pv = id_to_position_view[id])
							{
								//pv->OpDotCallback(position_view, i);
								auto& position_view_opdot_callback = position_view_opdot_callbacks[id];
								if (position_view_opdot_callback)
								{
									position_view_opdot_callbacks[id](position_view.m, i);
									position_view_opdot_callback = {};
								}
								num_finds++;
							}
							else
							{
								/*
								for (auto it = std::begin(transformations_orderings); it != std::end(transformations_orderings) - 1; it++)
								{
									it->m = glm::mat4(1.0f);
								}

								for (auto j = 0; j < 4; j++)
								{
									t[j][i] = 0.0f;
								}
								t[3][i] = v;
								//t[i][i] = 1.0f / current_position[i];
								*/

								const auto& id_name = id_to_name[id];
								if (id_name.find("float_") == 0)
								{

								}
								else
								{
									t[i][i] = v / current_position[i];
								}
							}
						}
						else if (name == "mat4v4float")
						{
							auto& values = GetSpvFloats(id);
							if (values.size() != 4)
							{
								PANIC("Only expected 4 float per index");
							}
							std::vector<float> value_floats(values.size());
							std::transform(std::begin(values), std::end(values), std::begin(value_floats), [](const auto& e) { return *e; });
							glm::vec4 value_vec4 = glm::make_vec4(value_floats.data());

							if (auto& pv = id_to_position_view[id])
							{
								//pv->OpDotCallback(position_view, i);
								auto& position_view_opdot_callback = position_view_opdot_callbacks[id];
								if (position_view_opdot_callback)
								{
									position_view_opdot_callbacks[id](position_view.m, i);
									position_view_opdot_callback = {};
								}
								num_finds++;
							}
							else
							{
								for (auto j = 0; j < values.size(); j++)
								{
									t[i][j] = *(values[j]);
								}
							}
						}
					}

					auto saved_m = position_view.m;
					const auto& name = id_to_name[type_id];
					if (num_finds > 0 && num_finds <= num_in_operands && name == "v4float")
					{
						for (auto i = num_finds; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							auto id_name = id_to_name[id];
							if (id_name.find("float_") == 0)
							{
								float float_value = *GetSpvFloats(id)[0];
								auto diff = float_value / original_position[i];
								if (float_value != 0.0f && diff != 1.0f)
								{
									PANIC("not 1.0f visible (meaning w isn't 1.0f)");
								}
							}
						}

						/*
						for (auto i = num_finds; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							auto id_name = id_to_name[id];
							if (id_name.find("float_") == 0)
							{
								float float_value = *GetSpvFloats(id)[0];
								auto diff = float_value / original_position[i];
								saved_m[i][i] = diff;

								// force all to reset to 1.0f
								for (auto& t : transformations_orderings)
								{
									t.m[i][i] = 1.0f;
								}
							}
						}
						//PANIC("Expected vec4");
						*position_view.transform = saved_m;
						*/
					}
					break;
				}
				case SpvOpStore:
				{
					//OpStore %_631 %80
					const auto dst_id = GetSingleInOperandWord(0);
					const auto src_id = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[src_id])
					{
						AddPositionViewBack(src_id, dst_id);
					}
					break;
				}
				case SpvOpCompositeExtract:
				{
					//%274 = OpCompositeExtract %float %266 0
					const auto base_id = GetSingleInOperandWord(0);
					PositionIndices shuffle_indices;
					for (auto i = 1; i < num_in_operands; i++)
					{
						shuffle_indices.emplace_back(GetSingleInOperandWord(i));
					}
					if (auto& pv = id_to_position_view[base_id])
					{
						auto& position_view = AddPositionViewBack(base_id, result_id);
						position_view.position_indices = shuffle_indices;
					}
					break;
				}
				case SpvOpVectorTimesMatrix:
				{
					//%295 = OpVectorTimesMatrix %v4float %259 %294
					const auto base_id = GetSingleInOperandWord(0);
					const auto mat_id = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[base_id])
					{
						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto& mul_floats = GetSpvFloats(mat_id);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}
						m = glm::transpose(m);

						auto& position_view = AddPositionViewBack(base_id, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpMatrixTimesVector:
				{
					//%24 = OpMatrixTimesVector %v4float %15 %23
					const auto mat_id = GetSingleInOperandWord(0);
					const auto base_id = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[base_id])
					{
						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto& mul_floats = GetSpvFloats(mat_id);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}
						//m = glm::transpose(m);

						auto& position_view = AddPositionViewFront(base_id, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpMatrixTimesMatrix:
				{
					//%62 = OpMatrixTimesMatrix %mat4v4float %59 %61
					const auto mat_id1 = GetSingleInOperandWord(0);
					const auto mat_id2 = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[mat_id1])
					{
						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto& mul_floats = GetSpvFloats(mat_id2);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewFront(mat_id1, result_id);
						position_view.m = m;
					}
					if (auto& pv = id_to_position_view[mat_id2])
					{
						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto& mul_floats = GetSpvFloats(mat_id1);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewFront(mat_id2, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpVectorTimesScalar:
				{
					//%17 = OpVectorTimesScalar %v3float %15 %float_0_00392156886
					const auto base_id = GetSingleInOperandWord(0);
					const auto scalar_id = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[base_id])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(scalar_id);
						if (mul_floats.size() != 1)
						{
							PANIC("Scalar should be one float");
						}
						for (auto i : pv->position_indices)
						{
							m[i][i] = *(mul_floats[0]);
						}

						auto& position_view = AddPositionViewBack(base_id, result_id);
						position_view.m = m;
					}
					if (auto& pv = id_to_position_view[scalar_id])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(base_id);
						if (mul_floats.size() == 1)
						{
							PANIC("Base should not be one float");
						}
						/*
						for (auto i : ref_position_view.position_indices)
						{
							m[i][i] = *(mul_floats[0]);
						}
						*/

						auto& position_view = AddPositionViewBack(scalar_id, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpFSub:
				{
					//%20 = OpFSub %v3float %17 %19
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[id1])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id2);
						if (mul_floats.size() > 3)
						{
							PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = -*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id1, result_id);
						position_view.m = m;
					}
					else if (auto& pv = id_to_position_view[id2])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id1);
						if (mul_floats.size() > 3)
						{
							PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = -*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id2, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpFAdd:
				{
					//%32 = OpFAdd %v3float %27 %31
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[id1])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id2);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = +*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id1, result_id);
						position_view.m = m;
					}
					else if (auto& pv = id_to_position_view[id2])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id1);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = +*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id2, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpFDiv:
				{
					//%212 = OpFDiv %float %204 %211
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto& pv = id_to_position_view[id1])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id2);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] /= *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id1, result_id);
						position_view.m = m;
					}
					else if (auto& pv = id_to_position_view[id2])
					{
						glm::mat4 m(1.0f);
						auto& mul_floats = GetSpvFloats(id1);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] /= *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id2, result_id);
						position_view.m = m;
					}
					break;
				}
				case SpvOpConvertFToS:
				{
					//%19 = OpConvertFToS %int %18 ; 0x0000082c
					const auto id = GetSingleInOperandWord(0);
					if (auto& pv = id_to_position_view[id])
					{
						auto& position_view = AddPositionViewBack(id, result_id);
						// Usually not used back in transformation, used for indexing
						/*
						PositionView& ref_position_view = found->second;
						auto mul_floats = GetSpvFloats(id);
						if (mul_floats.size() != 1)
						{
							PANIC("Expected 1");
						}
						if (ref_position_view.position_indices.size() != 1)
						{
							PANIC("Expected 1");
						}
						auto mul_float = *(mul_floats[0]);
						auto& position_view = AddPositionViewBack(id, result_id);

						auto index = ref_position_view.position_indices[0];

						glm::mat4 m(1.0f);
						*position_view.transform = m;
						m[i][i] =
						*/
					}
					break;
				}
				case SpvOpIAdd:
				{
					//%43 = OpIAdd %int %42 %int_0 ; 0x000008fc
					const auto id = GetSingleInOperandWord(0);
					if (auto& pv = id_to_position_view[id])
					{
						auto& position_view = AddPositionViewBack(id, result_id);
					}
					break;
				}
				case SpvOpIMul:
				{
					//%42 = OpIMul %int %40 %int_1 ; 0x000008e8
					const auto id = GetSingleInOperandWord(0);
					if (auto& pv = id_to_position_view[id])
					{
						auto& position_view = AddPositionViewBack(id, result_id);
					}
					break;
				}
				case SpvOpExtInst:
				{
					//%48 = OpExtInst %v3float %2 FAbs %47
					const auto literal_instruction_id = GetSingleInOperandWord(1);
					switch (literal_instruction_id)
					{
					case GLSLstd450Round:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto& pv = id_to_position_view[id])
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					case GLSLstd450FAbs:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto& pv = id_to_position_view[id])
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					case GLSLstd450Sqrt:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto& pv = id_to_position_view[id])
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					case GLSLstd450FMax:
					case GLSLstd450Normalize:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto& pv = id_to_position_view[id])
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					default:
					{
						PANIC("Not supported");
						break;
					}
					}
					break;
				}
				case SpvOpFOrdEqual:
				{
					for (auto i = 1; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);
						if (auto& pv = id_to_position_view[id])
						{
							auto& position_view = AddPositionViewBack(id, result_id);

							//TODO...
						}
					}
					break;
				}
				case SpvOpSelect:
				{
					for (auto i = 1; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);
						if (auto& pv = id_to_position_view[id])
						{
							auto& position_view = AddPositionViewBack(id, result_id);

							//TODO...
						}
					}
					break;
				}
				case SpvOpVariable:
				{
					// TODO: loop id to itself?
					auto& position_view = AddPositionViewBack(result_id, result_id);
					break;
				}
				case SpvOpShiftRightArithmetic:
				case SpvOpConvertSToF:
				{
					for (auto i = 1; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);
						if (auto& pv = id_to_position_view[id])
						{
							auto& position_view = AddPositionViewBack(id, result_id);

							//TODO...
						}
					}
					break;
				}
				default:
				{
					PANIC("not supported", debug_line);
					break;
				}
				}

				if (last_inserted_transformation_ordering_right)
				{
					current_position = current_position * transformations_orderings.back().position_view->m;
				}
				else
				{
					current_position = transformations_orderings[0].position_view->m * current_position;
				}
				//fmt::print("GLM {}\n", glm::to_string(current_position));
			}

#define FINISH_FUNCTION 0
			// Finish the function
			while (FINISH_FUNCTION && state->code_current)
			{
				auto current_word_offset = state->code_current - code_start;
				auto code_current = state->code_current;

				spvm_word opcode_data = code_current[0];
				spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift);
				SpvOp opcode = static_cast<SpvOp>(opcode_data & SpvOpCodeMask);

				spvm_state_step_opcode(state);
			}

#define EXPAND_TO_MODEL_AND_PROJECTION_VIEW true

			if (EXPAND_TO_MODEL_AND_PROJECTION_VIEW)
			{
				if (!cached_appended_ordering_ready)
				{
					// attempt to expand mvp -> m * vp
					for (auto i = 0; i < transformations_orderings.size(); i++)
					{
						auto& t = transformations_orderings[i];
						auto t_id = t.position_view->id;
						auto m = t.position_view->m;
						if (t_id > 0)
						{
							struct AppendedOrdering
							{
								std::vector<float*> data;
								uint32_t id;
							};
							std::deque<AppendedOrdering> appended_orderings{};

							auto AppendOrderings = [&](auto&& AppendOrderings_, uint32_t id) -> void
							{
								auto& id_construction_tree = analyzer->GetIdConstructionTree();
								auto& result_to_node = id_construction_tree.result_to_node;
								if (auto found = result_to_node.find(id); found != std::end(result_to_node))
								{
									auto& [_, node] = *found;
									auto result_id = node.result_id;

									if (node.op == SpvOpMatrixTimesVector)
									{
										for (const auto& operand_id : node.operand_ids)
										{
											auto& mul_floats = GetSpvFloats(operand_id);
											if (mul_floats.size() != 16)
											{
												continue;
											}

											AppendOrderings_(AppendOrderings_, operand_id);
										}
									}
									else if (node.op == SpvOpMatrixTimesMatrix)
									{
										// remove if result id is already in (meaning we are expanding that one instead)
										appended_orderings.erase(std::remove_if(std::begin(appended_orderings), std::end(appended_orderings), [&](const auto& t)
											{
												return id == t.id;
											}), std::end(appended_orderings));

										for (const auto& operand_id : node.operand_ids)
										{
											auto& mul_floats = GetSpvFloats(operand_id);
											if (mul_floats.size() != 16)
											{
												continue;
											}

											appended_orderings.push_front(AppendedOrdering{ mul_floats, operand_id });
											AppendOrderings_(AppendOrderings_, operand_id);
										}
									}
								}
							};

							AppendOrderings(AppendOrderings, t_id);

							if (!appended_orderings.empty())
							{
								// add to cache
								//auto appended_orderings_vec = std::vector<std::vector<float*>>(std::begin(appended_orderings), std::end(appended_orderings));
								auto appended_orderings_vec = std::vector<std::vector<float*>>(appended_orderings.size());
								for (auto i = 0; i < appended_orderings.size(); i++)
								{
									appended_orderings_vec[i] = appended_orderings[i].data;
								}

								cached_appended_ordering[t_id] = appended_orderings_vec;
								i += appended_orderings_vec.size() - 1;
							}
						}
					}
					cached_appended_ordering_ready = true;
				}
			}

			auto mvp = glm::mat4(1.0f);
			bool switch_to_projection_view = false;
			glm::mat4 model(1.0f);
			glm::mat4 projection_view(1.0f);

#ifndef NDEBUG
			std::deque<glm::mat4> debug_model_transformations;
			std::deque<glm::mat4> debug_projection_view_transformations;
#endif
			for (auto& t : transformations_orderings)
			{
				auto& m = t.position_view->m;
				auto& t_id = t.position_view->id;
				auto right = t.right;

				auto DoCalc = [&](glm::mat4& m, bool right)
				{
					if (m == DEFAULT_MATRIX)
					{
						return;
					}
					if (EXPAND_TO_MODEL_AND_PROJECTION_VIEW)
					{
						if (right)
						{
							if (!switch_to_projection_view)
							{
								// check if last row is changed (affine matrix otherwise, which means model matrix)
								for (auto i = 0; i < 3; i++)
								{
									if (m[i][3] != 0.0f)
									{
										switch_to_projection_view = true;
										break;
									}
								}

								if (!switch_to_projection_view)
								{
									model = m * model;
#ifndef NDEBUG
									debug_model_transformations.push_front(m);
#endif
								}
								else
								{
									projection_view = m * projection_view;
#ifndef NDEBUG
									debug_projection_view_transformations.push_front(m);
#endif
								}
								switch_to_projection_view = true;
							}
							else
							{
								projection_view = m * projection_view;
#ifndef NDEBUG
								debug_projection_view_transformations.push_front(m);
#endif
							}
						}
						else
						{
							if (!switch_to_projection_view)
							{
								// check if last row is changed (affine matrix otherwise, which means model matrix)
								for (auto i = 0; i < 3; i++)
								{
									if (m[i][3] != 0.0f)
									{
										switch_to_projection_view = true;
										break;
									}
								}

								if (!switch_to_projection_view)
								{
									model = model * m;
#ifndef NDEBUG
									debug_model_transformations.push_back(m);
#endif
								}
								else
								{
									projection_view = projection_view * m;
#ifndef NDEBUG
									debug_projection_view_transformations.push_back(m);
#endif
								}
								switch_to_projection_view = true;
							}
							else
							{
								projection_view = projection_view * m;
#ifndef NDEBUG
								debug_projection_view_transformations.push_back(m);
#endif
							}
						}
					}
					else
					{
						if (right)
						{
							model = m * model;
#ifndef NDEBUG
							debug_model_transformations.push_back(m);
#endif
						}
						else
						{
							model = model * m;
#ifndef NDEBUG
							debug_model_transformations.push_front(m);
#endif
						}
					}
				};

				if (EXPAND_TO_MODEL_AND_PROJECTION_VIEW)
				{
					if (auto& appended_ordering_ids = cached_appended_ordering[t_id])
					{
						for (const auto& mul_floats : *appended_ordering_ids)
						{
							auto data = glm::mat4(
								*(mul_floats[0]), *(mul_floats[1]), *(mul_floats[2]), *(mul_floats[3]),
								*(mul_floats[4]), *(mul_floats[5]), *(mul_floats[6]), *(mul_floats[7]),
								*(mul_floats[8]), *(mul_floats[9]), *(mul_floats[10]), *(mul_floats[11]),
								*(mul_floats[12]), *(mul_floats[13]), *(mul_floats[14]), *(mul_floats[15]));

							DoCalc(data, true);
						}
					}
					else
					{
						DoCalc(m, right);
					}
				}
				else
				{
					DoCalc(m, right);
				}
			}

			glm::vec4 gl_Position_actual{};
			{
				auto word_offset = line_to_word_offset[position_affected_lines_vector.back().line];
				const SPVInstruction& inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));
				if (inst.opcode != SpvOpStore)
				{
					PANIC("Not store!");
				}

				auto dst_id = inst.words[1];
				auto gl_Position_actual_data = GetSpvFloats(dst_id);
				for (auto i = 0; i < gl_Position_actual_data.size(); i++)
				{
					gl_Position_actual[i] = *(gl_Position_actual_data[i]);
				}
			}

			auto gl_Position_expected = mvp * original_position;

			if (glm::any(glm::isnan(model[0])) ||
				glm::any(glm::isnan(model[1])) ||
				glm::any(glm::isnan(model[2])) ||
				glm::any(glm::isnan(model[3])))
			{
				// TODO: Fix... if it is nan, then there is something seriously wrong...
				model = glm::mat4(1.0f);
				//PANIC("SPIRV VM MODEL MATRIX IS NAN");
			}

			RunResult run_result{};
			run_result.model = model;
			run_result.projection_view = projection_view;

			return run_result;
		}


		Expected<RunResult> SPIRVVirtualMachine::RunAfterSetup(SetupRunInfo& setup_run_info)
		{
			ScopedCPUProfile("SPIRV VM RunAfterSetup");

			glm::mat4 mvp(1.0f);

			const auto& common_info = setup_run_info.common_info;
			const auto& position_affected_lines_vector = common_info->position_affected_lines_vector;
			const auto& word_offset_executed = common_info->word_offset_executed;
			const auto& line_to_word_offset = common_info->line_to_word_offset;
			const auto& position_id = common_info->position_id;
			const auto& original_position = common_info->original_position;

			SPIRVVirtualMachineState& spirv_vm_state = setup_run_info.spirv_vm_state;;

			auto& vm_state = spirv_vm_state.vm_state;
			auto& name_to_variable_cache = spirv_vm_state.name_to_variable_cache;
			auto& id_to_spv_floats = spirv_vm_state.id_to_spv_floats;
			const auto& main_function = spirv_vm_state.main_function;

			auto GetSpvFloats = [&id_to_spv_floats](int id) -> std::vector<float*>
			{
				return id_to_spv_floats[id];
			};

			spvm_source code_start = reinterpret_cast<spvm_source>(spv.data());

			struct TransformationOrdering
			{
				glm::mat4 m = glm::mat4(1.0f);
				bool right = true;
				std::string debug_line{};
				uint32_t id = 0;
			};

			std::vector<TransformationOrdering> transformations_orderings;

			struct PositionView
			{
				glm::mat4* transform{};
				std::vector<uint32_t> position_indices;
				std::function<void(glm::mat4&, std::size_t)> opdot_callback{};

				void OpDotCallback(glm::mat4& m, std::size_t i)
				{
					if (opdot_callback)
					{
						opdot_callback(m, i);
						opdot_callback = {};
					}
				}
			};

			flat_hash_map<uint32_t, PositionView> id_to_position_view;

			{
				auto& m = transformations_orderings.emplace_back(TransformationOrdering{ glm::mat4(1.0f), true });
				id_to_position_view[position_id] = PositionView{ &m.m, {0, 1, 2, 3} };
				//AddPositionViewNoRef(position_id, true, { 0, 1, 2, 3 }, glm::mat4(1.0f));
			}

			// restore state
			auto* state = vm_state.get();
			spvm_state_prepare(state, main_function);

			glm::vec4 current_position = original_position;

			uint64_t current_word_offset = 0;
			for (const auto& position_affected_lines : position_affected_lines_vector)
			{
				auto line = position_affected_lines.line;
				auto position_affected_id = position_affected_lines.inst.position_affected_id;
				const auto& debug_line = position_affected_lines.inst.debug_line;
				//fmt::print("LINE {}\n", debug_line);

				/*
				auto aaa = GetSpvFloats(orig_state, 113);
				auto aaas = GetSpvFloats(state, 113);
				auto aaa22s = GetSpvFloats(orig_state, 114);
				auto aaas3123 = GetSpvFloats(state, 114);
				auto aaas312312313 = GetSpvFloats(orig_state, 119);
				auto aaas3123312312 = GetSpvFloats(state, 119);
				auto aaas312331231sdsa2 = GetSpvFloats(orig_state, 116);
				auto aaas3123312dasd312 = GetSpvFloats(state, 116);
				auto dasda = GetSpvFloats(orig_state, 117);
				auto aaas3123312ddasdssadasd312 = GetSpvFloats(state, 117);
				*/

				// optimization to call only necessary spirv instructions to buld the target
				{
					auto target_word_offset = line_to_word_offset[line];
					current_word_offset = state->code_current - code_start;
					//fmt::print("WTF {} {} {} {} {}\n\n", line, debug_line, (uint64_t)current_word_offset, (uint64_t)target_word_offset, target_word_offset - current_word_offset);
					
					while (current_word_offset <= target_word_offset)
					{
						current_word_offset = state->code_current - code_start;
						auto code_current = state->code_current;

						spvm_word opcode_data = code_current[0];
						spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift);
						SpvOp opcode = static_cast<SpvOp>(opcode_data & SpvOpCodeMask);

						if(0)
						{
							auto line_spv = std::vector<uint32_t>(std::begin(spv), std::begin(spv) + 5);
							for (auto i = 0; i < word_count; i++)
							{
								line_spv.emplace_back(code_current[i]);
							}

							spvtools::SpirvTools spirv_tools(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
							std::string disassembly{};
							if (!spirv_tools.Disassemble(line_spv,
								&disassembly,
								SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
								SPV_BINARY_TO_TEXT_OPTION_INDENT |
								SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
								SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET |
								SPV_BINARY_TO_TEXT_OPTION_COMMENT))
							{
								//PANIC("Failed to diassemble");
							}
							fmt::print("!!! {} {} {} {} | {}\n", (uint64_t)current_word_offset, word_count,(uint64_t)target_word_offset, target_word_offset - current_word_offset, disassembly);
							if (1 && disassembly.find("%76 = OpLoad ") != std::string::npos)
							{
								auto mul_floats1 = GetSpvFloats(73);
								auto mul_floats2 = GetSpvFloats(75);
								auto mul_floats3 = GetSpvFloats(76);
								fmt::print("");
							}
						}

						// step over function (this loop is to handle function calls)
						if (opcode == SpvOpFunctionCall)
						{
							auto expected_next_instruction_index = code_current + word_count;
							do
							{
								spvm_state_step_opcode(state);
							} while (state->code_current != 0 && expected_next_instruction_index != state->code_current);
						}
						else
						{
							spvm_state_step_opcode(state);
						}
					}
				}

				//
				auto AddPositionViewFront = [&](uint32_t copy_id, uint32_t id) -> PositionView&
				{
					auto copied_position_view = id_to_position_view[copy_id];
					auto& m = transformations_orderings.emplace_back(TransformationOrdering{ glm::mat4(1.0f), false, {}, id });
					//m.debug_line = debug_line;
					copied_position_view.transform = &m.m;
					auto [iter, success] = id_to_position_view.try_emplace(id, copied_position_view);
					return iter->second;
					/*
					if (copied_position_view.transform)
					{
						auto& m = transformations.emplace_front(*copied_position_view.transform);
						copied_position_view.transform = &m;
						auto [iter, success] = id_to_position_view.try_emplace(id, copied_position_view);
						return iter->second;
					}
					else
					{
						auto& m = transformations.emplace_front(glm::mat4(1.0f));
						copied_position_view.transform = &m;
						auto [iter, success] = id_to_position_view.try_emplace(id, copied_position_view);
						return iter->second;
					}
					*/
				};

				auto AddPositionViewBack = [&](uint32_t copy_id, uint32_t id) -> PositionView&
				{
					auto copied_position_view = id_to_position_view[copy_id];
					auto& m = transformations_orderings.emplace_back(TransformationOrdering{ glm::mat4(1.0f), true, {}, id });
					//m.debug_line = debug_line;
					copied_position_view.transform = &m.m;
					auto [iter, success] = id_to_position_view.try_emplace(id, copied_position_view);
					return iter->second;

					/*
					if (copied_position_view.transform)
					{
						auto& m = transformations.emplace_back(*copied_position_view.transform);
						copied_position_view.transform = &m;
						auto [iter, success] = id_to_position_view.try_emplace(id, copied_position_view);
						return iter->second;
					}
					else
					{
						auto& m = transformations.emplace_back(glm::mat4(1.0f));
						copied_position_view.transform = &m;
						auto [iter, success] = id_to_position_view.try_emplace(id, copied_position_view);
						return iter->second;
					}
					*/
				};

				auto AddPositionViewNoRef = [&](uint32_t id, bool insert_back = true, std::vector<uint32_t> indices = {}, glm::mat4 mm = glm::mat4(1.0f))
				{
					if (insert_back)
					{
						auto& m = transformations_orderings.emplace_back(TransformationOrdering{ glm::mat4(1.0f), true, {}, id });
						//m.debug_line = debug_line;
						auto [iter, success] = id_to_position_view.try_emplace(id, PositionView{ &m.m, indices });
						return iter->second;
					}
					else
					{
						auto& m = transformations_orderings.emplace_back(TransformationOrdering{ glm::mat4(1.0f), false, {}, id });
						//m.debug_line = debug_line;
						auto [iter, success] = id_to_position_view.try_emplace(id, PositionView{ &m.m, indices });
						return iter->second;
					}
				};




				auto word_offset = line_to_word_offset[line];
				const SPVInstruction& inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));

				const SpvOp& opcode = inst.opcode;
				uint32_t type_id = inst.type_id;
				uint32_t result_id = inst.result_id;
				const std::vector<uint32_t>& words = inst.words;
				const std::vector<spv_parsed_operand_t>& operands = inst.operands;

				// for some reason type_id and result_id is sometimes not parsed
				for (const auto& operand : operands)
				{
					if (operand.type == SPV_OPERAND_TYPE_TYPE_ID)
					{
						type_id = words[operand.offset];
					}
					else if (operand.type == SPV_OPERAND_TYPE_RESULT_ID)
					{
						result_id = words[operand.offset];
					}
				}

				auto op_start = (type_id > 0 ? 1 : 0) + (result_id > 0 ? 1 : 0);
				std::vector<spv_parsed_operand_t> in_operands(operands.size() - op_start);
				std::copy(std::begin(operands) + op_start, std::end(operands), std::begin(in_operands));
				/*
				auto OperandWords = [&](const spv_parsed_operand_t& operand)
				{
					auto offset = operand.offset;
					auto num_words = operand.num_words;
					std::vector<uint32_t> operand_words(num_words);
					for (auto i = offset; i < offset + num_words; i++)
					{
						operand_words[i - offset] = words[offset];
					}
					return operand_words;
				};
				*/
				auto GetSingleInOperandWord = [&](std::size_t i)
				{
					/*
					const auto& in_operand = in_operands[i];
					auto operand_words = OperandWords(in_operand);
					if (operand_words.size() != 1)
					{
						PANIC("Expected 1 word");
					}
					return operand_words[0];
					*/
					const auto& in_operand = in_operands[i];
					auto offset = in_operand.offset;
					auto num_words = in_operand.num_words;
					if (num_words != 1)
					{
						PANIC("Expected 1 word");
					}
					return words[offset];
				};

				auto num_in_operands = in_operands.size();
				if (opcode == SpvOpAccessChain)
				{
					//%83 = OpAccessChain %_ptr_Function_float %_631 %uint_0 ; 0x000038d8
					//...
					//%84 = OpLoad %float %83 ; 0x000038ec
					//%93 = OpCompositeConstruct %v4float %84 %87 %90 %92 ; 0x00003968
					//OpStore %_633 %93 ; 0x00003984
					//vec4 _633 = vec4(_631.x, _631.y, _631.z, POSITION.w);
					const auto base_id = GetSingleInOperandWord(0);
					if (num_in_operands != 2)
					{
						//continue;
						//PANIC("Access should only be one deference");
					}
					// TODO: This assumes the last index is the index we are going for (not always true since you could have double array indexing)
					const auto memory_access_id = GetSingleInOperandWord(num_in_operands - 1);
					float* access_index_ptr = GetSpvFloats(memory_access_id)[0];
					auto access_index = *(uint32_t*)access_index_ptr;

					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						/*
						if (ref_position_view.position_indices.size() != 4)
						{
							PANIC("Should be entire POSITION? (float 4)");
						}
						*/
						auto position_view = AddPositionViewBack(base_id, result_id);
						position_view.position_indices = { access_index };
					}
				}
				else if (opcode == SpvOpVectorShuffle)
				{
					//%73 = OpVectorShuffle % v3float % 72 % 72 0 1 2; 0x00003808
					//vec4 _633 = vec4(_631.x, _631.y, _631.z, POSITION.w);
					const auto base_id = GetSingleInOperandWord(0);
					const auto base_id2 = GetSingleInOperandWord(1);
					auto start_index = 2;
					auto end_index = num_in_operands;
					if (base_id != base_id2)
					{
						//PANIC("Assume shuffle same vec");
						//Assume the one with the least has the correct start index
					}

					std::vector<uint32_t> shuffle_indices;

					if (base_id != base_id2)
					{
						// Assume it's all of the vector?
						for (auto i = start_index; i < end_index; i++)
						{
							auto index = i - start_index;
							shuffle_indices.emplace_back(index);
						}
					}
					else
					{
						for (auto i = start_index; i < end_index; i++)
						{
							auto index = GetSingleInOperandWord(i);
							shuffle_indices.emplace_back(index);
						}
					}
					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						auto& position_view = AddPositionViewBack(base_id, result_id);
						position_view.position_indices = shuffle_indices;
					}
					if (base_id != base_id2)
					{
						if (auto found = id_to_position_view.find(base_id2); found != std::end(id_to_position_view))
						{
							PositionView& ref_position_view = found->second;
							auto& position_view = AddPositionViewBack(base_id2, result_id);
							position_view.position_indices = shuffle_indices;
						}
					}
				}
				else if (opcode == SpvOpLoad)
				{
					//%72 = OpLoad %v4float %POSITION ; 0x000037f8
					const auto base_id = GetSingleInOperandWord(0);
					if (num_in_operands != 1)
					{
						PANIC("Load should only be one deference");
					}
					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						AddPositionViewBack(base_id, result_id);
					}
				}
				else if (opcode == SpvOpFMul)
				{
					//%80 = OpFMul %v3float %73 %79
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(id1); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id2);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[i][i] = *(mul_floats[i]);
						}
						auto& position_view = AddPositionViewBack(id1, result_id);
						*position_view.transform = m;
					}
					else if (auto found = id_to_position_view.find(id2); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id1);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[i][i] = *(mul_floats[i]);
						}
						auto& position_view = AddPositionViewBack(id2, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpDot)
				{
					//%101 = OpDot %float %99 %100 ; 0x00003a10
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (num_in_operands != 2)
					{
						PANIC("Load should only be one deference");
					}
					if (auto found = id_to_position_view.find(id1); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						// have to be resolved later
						auto& position_view = AddPositionViewBack(id1, result_id);
						position_view.opdot_callback = [&GetSpvFloats, &state, &id_to_position_view, result_id, id2](glm::mat4& m, std::size_t row)
						{
							if (auto found = id_to_position_view.find(result_id); found != std::end(id_to_position_view))
							{
								PositionView& position_view = found->second;

								auto floats = GetSpvFloats(id2);
								for (auto i = 0; i < floats.size(); i++)
								{
									//m[row][i] = *(floats[i]);
									m[i][row] = *(floats[i]);
								}
							}
						};
					}
					else if (auto found = id_to_position_view.find(id2); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						// have to be resolved later
						auto& position_view = AddPositionViewFront(id2, result_id);
						position_view.opdot_callback = [&GetSpvFloats, &state, &id_to_position_view, result_id, id1](glm::mat4& m, std::size_t row)
						{
							if (auto found = id_to_position_view.find(result_id); found != std::end(id_to_position_view))
							{
								PositionView& position_view = found->second;

								auto floats = GetSpvFloats(id1);
								for (auto i = 0; i < floats.size(); i++)
								{
									//m[row][i] = *(floats[i]);
									m[i][row] = *(floats[i]);
								}
							}
						};
					}
				}
				else if (opcode == SpvOpCompositeConstruct)
				{
					//%93 = OpCompositeConstruct %v4float %84 %87 %90 %92
					std::vector<uint32_t> indices(num_in_operands);
					std::iota(std::begin(indices), std::end(indices), 0);

					auto num_finds = 0;
					auto& position_view = AddPositionViewNoRef(result_id, true, indices);
					auto& t = *position_view.transform;
					for (auto i = 0; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);

						auto name = inst_manager.GetNameMapper().NameForId(type_id);
						if (name == "v4float")
						{
							auto values = GetSpvFloats(id);
							if (values.size() != 1)
							{
								PANIC("Only expected 1 float per index");
							}
							auto v = (*values[0]);

							if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
							{
								PositionView& ref_position_view = found->second;
								ref_position_view.OpDotCallback(t, i);
								num_finds++;
							}
							else
							{
								/*
								for (auto it = std::begin(transformations_orderings); it != std::end(transformations_orderings) - 1; it++)
								{
									it->m = glm::mat4(1.0f);
								}

								for (auto j = 0; j < 4; j++)
								{
									t[j][i] = 0.0f;
								}
								t[3][i] = v;
								//t[i][i] = 1.0f / current_position[i];
								*/

								auto id_name = inst_manager.GetNameMapper().NameForId(id);
								if (id_name.find("float_") == 0)
								{

								}
								else
								{
									t[i][i] = v / current_position[i];
								}
							}
						}
						else if (name == "mat4v4float")
						{
							auto values = GetSpvFloats(id);
							if (values.size() != 4)
							{
								PANIC("Only expected 4 float per index");
							}
							std::vector<float> value_floats(values.size());
							std::transform(std::begin(values), std::end(values), std::begin(value_floats), [](const auto& e) { return *e; });
							glm::vec4 value_vec4 = glm::make_vec4(value_floats.data());

							if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
							{
								PositionView& ref_position_view = found->second;
								ref_position_view.OpDotCallback(t, i);
								num_finds++;
							}
							else
							{
								for (auto j = 0; j < values.size(); j++)
								{
									t[i][j] = *(values[j]);
								}
							}
						}
					}

					auto saved_m = *position_view.transform;
					auto name = inst_manager.GetNameMapper().NameForId(type_id);
					if (num_finds > 0 && num_finds <= num_in_operands && name == "v4float")
					{
						for (auto i = num_finds; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							auto id_name = inst_manager.GetNameMapper().NameForId(id);
							if (id_name.find("float_") == 0)
							{
								float float_value = *GetSpvFloats(id)[0];
								auto diff = float_value / original_position[i];
								if (float_value != 0.0f && diff != 1.0f)
								{
									PANIC("not 1.0f visible (meaning w isn't 1.0f)");
								}
							}
						}

						/*
						for (auto i = num_finds; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							auto id_name = inst_manager.GetNameMapper().NameForId(id);
							if (id_name.find("float_") == 0)
							{
								float float_value = *GetSpvFloats(id)[0];
								auto diff = float_value / original_position[i];
								saved_m[i][i] = diff;

								// force all to reset to 1.0f
								for (auto& t : transformations_orderings)
								{
									t.m[i][i] = 1.0f;
								}
							}
						}
						//PANIC("Expected vec4");
						*position_view.transform = saved_m;
						*/
					}
				}
				else if (opcode == SpvOpStore)
				{
					//OpStore %_631 %80
					const auto dst_id = GetSingleInOperandWord(0);
					const auto src_id = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(src_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						AddPositionViewBack(src_id, dst_id);
					}
				}
				else if (opcode == SpvOpCompositeExtract)
				{
					//%274 = OpCompositeExtract %float %266 0
					const auto base_id = GetSingleInOperandWord(0);
					std::vector<uint32_t> shuffle_indices;
					for (auto i = 1; i < num_in_operands; i++)
					{
						shuffle_indices.emplace_back(GetSingleInOperandWord(i));
					}
					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;
						auto& position_view = AddPositionViewBack(base_id, result_id);
						position_view.position_indices = shuffle_indices;
					}
				}
				else if (opcode == SpvOpVectorTimesMatrix)
				{
					//%295 = OpVectorTimesMatrix %v4float %259 %294
					const auto base_id = GetSingleInOperandWord(0);
					const auto mat_id = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto mul_floats = GetSpvFloats(mat_id);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}
						m = glm::transpose(m);

						auto& position_view = AddPositionViewBack(base_id, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpMatrixTimesVector)
				{
					//%24 = OpMatrixTimesVector %v4float %15 %23
					const auto mat_id = GetSingleInOperandWord(0);
					const auto base_id = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto mul_floats = GetSpvFloats(mat_id);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}
						//m = glm::transpose(m);

						auto& position_view = AddPositionViewFront(base_id, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpMatrixTimesMatrix)
				{
					//%62 = OpMatrixTimesMatrix %mat4v4float %59 %61
					const auto mat_id1 = GetSingleInOperandWord(0);
					const auto mat_id2 = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(mat_id1); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto mul_floats = GetSpvFloats(mat_id2);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewFront(mat_id1, result_id);
						*position_view.transform = m;
					}
					else if (auto found = id_to_position_view.find(mat_id2); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						float* m_view = glm::value_ptr(m);
						auto mul_floats = GetSpvFloats(mat_id1);
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m_view[i] = *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewFront(mat_id2, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpVectorTimesScalar)
				{
					//%17 = OpVectorTimesScalar %v3float %15 %float_0_00392156886
					const auto base_id = GetSingleInOperandWord(0);
					const auto scalar_id = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(scalar_id);
						if (mul_floats.size() != 1)
						{
							PANIC("Scalar should be one float");
						}
						for (auto i : ref_position_view.position_indices)
						{
							m[i][i] = *(mul_floats[0]);
						}

						auto& position_view = AddPositionViewBack(base_id, result_id);
						*position_view.transform = m;
					}
					if (auto found = id_to_position_view.find(scalar_id); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(base_id);
						if (mul_floats.size() == 1)
						{
							PANIC("Base should not be one float");
						}
						/*
						for (auto i : ref_position_view.position_indices)
						{
							m[i][i] = *(mul_floats[0]);
						}
						*/

						auto& position_view = AddPositionViewBack(scalar_id, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpFSub)
				{
					//%20 = OpFSub %v3float %17 %19
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(id1); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id2);
						if (mul_floats.size() > 3)
						{
							PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = -*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id1, result_id);
						*position_view.transform = m;
					}
					else if (auto found = id_to_position_view.find(id2); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id1);
						if (mul_floats.size() > 3)
						{
							PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = -*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id2, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpFAdd)
				{
					//%32 = OpFAdd %v3float %27 %31
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(id1); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id2);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = +*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id1, result_id);
						*position_view.transform = m;
					}
					else if (auto found = id_to_position_view.find(id2); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id1);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] = +*(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id2, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpFDiv)
				{
					//%212 = OpFDiv %float %204 %211
					const auto id1 = GetSingleInOperandWord(0);
					const auto id2 = GetSingleInOperandWord(1);
					if (auto found = id_to_position_view.find(id1); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id2);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] /= *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id1, result_id);
						*position_view.transform = m;
					}
					else if (auto found = id_to_position_view.find(id2); found != std::end(id_to_position_view))
					{
						PositionView& ref_position_view = found->second;

						glm::mat4 m(1.0f);
						auto mul_floats = GetSpvFloats(id1);
						if (mul_floats.size() > 3)
						{
							//PANIC("Assume 3");
						}
						//for (auto i : ref_position_view.position_indices)
						for (auto i = 0; i < mul_floats.size(); i++)
						{
							m[3][i] /= *(mul_floats[i]);
						}

						auto& position_view = AddPositionViewBack(id2, result_id);
						*position_view.transform = m;
					}
				}
				else if (opcode == SpvOpConvertFToS)
				{
					//%19 = OpConvertFToS %int %18 ; 0x0000082c
					const auto id = GetSingleInOperandWord(0);
					if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
					{
						auto& position_view = AddPositionViewBack(id, result_id);
						// Usually not used back in transformation, used for indexing
						/*
						PositionView& ref_position_view = found->second;
						auto mul_floats = GetSpvFloats(id);
						if (mul_floats.size() != 1)
						{
							PANIC("Expected 1");
						}
						if (ref_position_view.position_indices.size() != 1)
						{
							PANIC("Expected 1");
						}
						auto mul_float = *(mul_floats[0]);
						auto& position_view = AddPositionViewBack(id, result_id);

						auto index = ref_position_view.position_indices[0];

						glm::mat4 m(1.0f);
						*position_view.transform = m;
						m[i][i] =
						*/
					}
				}
				else if (opcode == SpvOpIAdd)
				{
					//%43 = OpIAdd %int %42 %int_0 ; 0x000008fc
					const auto id = GetSingleInOperandWord(0);
					if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
					{
						auto& position_view = AddPositionViewBack(id, result_id);
					}
				}
				else if (opcode == SpvOpIMul)
				{
					//%42 = OpIMul %int %40 %int_1 ; 0x000008e8
					const auto id = GetSingleInOperandWord(0);
					if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
					{
						auto& position_view = AddPositionViewBack(id, result_id);
					}
				}
				else if (opcode == SpvOpExtInst)
				{
					//%48 = OpExtInst %v3float %2 FAbs %47
					const auto literal_instruction_id = GetSingleInOperandWord(1);
					switch (literal_instruction_id)
					{
					case GLSLstd450Round:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					case GLSLstd450FAbs:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					case GLSLstd450Sqrt:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					case GLSLstd450FMax:
					case GLSLstd450Normalize:
					{
						for (auto i = 2; i < num_in_operands; i++)
						{
							auto id = GetSingleInOperandWord(i);
							if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
							{
								auto& position_view = AddPositionViewBack(id, result_id);

								//TODO...
							}
						}
						break;
					}
					default:
					{
						PANIC("Not supported");
						break;
					}
					}
				}
				else if (opcode == SpvOpFOrdEqual)
				{
					for (auto i = 1; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);
						if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
						{
							auto& position_view = AddPositionViewBack(id, result_id);

							//TODO...
						}
					}
				}
				else if (opcode == SpvOpSelect)
				{
					for (auto i = 1; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);
						if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
						{
							auto& position_view = AddPositionViewBack(id, result_id);

							//TODO...
						}
					}
				}
				else if (opcode == SpvOpVariable)
				{
					// TODO: loop id to itself?
					auto& position_view = AddPositionViewBack(result_id, result_id);
				}
				else if (opcode == SpvOpShiftRightArithmetic || opcode == SpvOpConvertSToF)
				{
					for (auto i = 1; i < num_in_operands; i++)
					{
						auto id = GetSingleInOperandWord(i);
						if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
						{
							auto& position_view = AddPositionViewBack(id, result_id);

							//TODO...
						}
					}
				}
				else
				{
					PANIC("not supported", debug_line);
				}

				const auto& t = transformations_orderings.back();
				if (t.right)
				{
					current_position = current_position * t.m;
				}
				else
				{
					current_position = t.m * current_position;
				}
				//fmt::print("GLM {}\n", glm::to_string(current_position));
			}

			std::vector<glm::mat4> transforms2;
			for (auto& t : transformations_orderings)
			{
				if (t.m != glm::mat4(1.0f))
				{
					auto& m = t.m;

					if (0)
					{
						if (t.right)
						{
							fmt::print("RIGHT: {}\n", t.right);
							fmt::print("{}\n", t.debug_line);
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(m[i]));
							}
							fmt::print("{}\n", "*");
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(mvp[i]));
							}
							fmt::print("=\n{}\n", glm::to_string(m * mvp * original_position));
							fmt::print("\n", glm::to_string(m), glm::to_string(mvp));
						}
						else
						{
							fmt::print("LEFT: {}\n", t.right);
							fmt::print("{}\n", t.debug_line);
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(mvp[i]));
							}
							fmt::print("{}\n", "*");
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(m[i]));
							}
							fmt::print("=\n{}\n", glm::to_string(mvp * m * original_position));
							fmt::print("\n", glm::to_string(m), glm::to_string(mvp));
						}
					}
					if (1)
					{
						if (t.right)
						{
							mvp = m * mvp;
						}
						else
						{
							mvp = mvp * m;
						}
					}
					else
					{
						if (t.right)
						{
							mvp = mvp * m;
						}
						else
						{
							mvp = m * mvp;
						}
					}
				}
			}

			// Finish the function ???
			while (state->code_current)
			{
				auto current_word_offset = state->code_current - code_start;
				auto code_current = state->code_current;

				spvm_word opcode_data = code_current[0];
				spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift);
				SpvOp opcode = static_cast<SpvOp>(opcode_data & SpvOpCodeMask);

				spvm_state_step_opcode(state);
			}

			/*
			auto _631 = GetSpvFloats(state, 37);
			auto _633 = GetSpvFloats(state, 38);
			auto _636 = GetSpvFloats(state, 39);
			auto _639 = GetSpvFloats(state, 40);
			auto _768 = GetSpvFloats(state, 41);
			auto _294 = GetSpvFloats(state, 42);
			auto _294ss = GetSpvFloats(state, 43);
			auto _294ssss = GetSpvFloats(state, 45);
			*/
			/*
			auto _631as = GetSpvFloats(state, 186);
			auto _631ads = GetSpvFloats(state, 174);
			auto _631adsdasda = GetSpvFloats(state, 124);
			auto dasdaa = GetSpvFloats(state, 173);
			auto dasdasdadaa = GetSpvFloats(state, 125);
			auto dasdasdasdaa = GetSpvFloats(state, 172);
			auto dasdasdadasdsdaa = GetSpvFloats(state, 186);
			auto _631asa= GetSpvFloats(state, 186);
			*/

			//auto gl_Position_actual = AssignOrPanic(GetVariable("gl_Position"));
			//Assume last instruction is the instruction for gl_Position
			glm::vec4 gl_Position_actual{};
			{
				auto word_offset = line_to_word_offset[position_affected_lines_vector.back().line];
				const SPVInstruction& inst = AssignOrPanic(inst_manager.GetInstructionFromOffset(word_offset));
				if (inst.opcode != SpvOpStore)
				{
					PANIC("Not store!");
				}

				auto dst_id = inst.words[1];
				auto gl_Position_actual_data = GetSpvFloats(dst_id);
				for (auto i = 0; i < gl_Position_actual_data.size(); i++)
				{
					gl_Position_actual[i] = *(gl_Position_actual_data[i]);
				}
			}

			auto gl_Position_expected = mvp * original_position;

			transformations_orderings.erase(std::remove_if(std::begin(transformations_orderings), std::end(transformations_orderings), [](const auto& t) {
				if (t.m == glm::mat4(1.0f))
				{
					return true;
				}
				return false;
			}), std::end(transformations_orderings));

			if (EXPAND_TO_MODEL_AND_PROJECTION_VIEW)
			{
				if (!is_cached_transformation_ordering.exchange(true))
				{
					// attempt to expand mvp -> m * vp
					for (auto i = 0; i < transformations_orderings.size(); i++)
					{
						auto& t = transformations_orderings[i];
						auto original_id = t.id;
						if (t.id > 0)
						{
							std::deque<TransformationOrdering> appended_orderings{};

							auto AppendOrderings = [&](auto&& AppendOrderings_, uint32_t id) -> void
							{
								auto& id_construction_tree = analyzer->GetIdConstructionTree();
								auto& result_to_node = id_construction_tree.result_to_node;
								if (auto found = result_to_node.find(id); found != std::end(result_to_node))
								{
									auto& [_, node] = *found;
									auto result_id = node.result_id;

									if (node.op == SpvOpMatrixTimesVector)
									{
										for (const auto& operand_id : node.operand_ids)
										{
											auto mul_floats = GetSpvFloatsVec(state, operand_id);
											if (mul_floats.size() != 16)
											{
												continue;
											}

											AppendOrderings_(AppendOrderings_, operand_id);
										}
									}
									else if (node.op == SpvOpMatrixTimesMatrix)
									{
										// remove if result id is already in (meaning we are expanding that one instead)
										appended_orderings.erase(std::remove_if(std::begin(appended_orderings), std::end(appended_orderings), [&](const auto& t)
											{
												return id == t.id;
											}), std::end(appended_orderings));

										for (const auto& operand_id : node.operand_ids)
										{
											auto mul_floats = GetSpvFloatsVec(state, operand_id);
											if (mul_floats.size() != 16)
											{
												continue;
											}

											glm::mat4 data = glm::make_mat4x4(mul_floats.data());
											appended_orderings.push_front(TransformationOrdering{ data, true, {}, operand_id });
											AppendOrderings_(AppendOrderings_, operand_id);
										}
									}
								}
							};

							AppendOrderings(AppendOrderings, t.id);
							if (!appended_orderings.empty())
							{
								transformations_orderings.erase(std::begin(transformations_orderings) + i, std::begin(transformations_orderings) + i + 1);
								transformations_orderings.insert(std::begin(transformations_orderings) + i, std::begin(appended_orderings), std::end(appended_orderings));
								i += appended_orderings.size() - 1;
							}

							std::vector<uint32_t> ordering_ids;
							for (const auto& appended_ordering : appended_orderings)
							{
								ordering_ids.emplace_back(appended_ordering.id);
							}

							// add to cache
							cached_appended_ordering[original_id] = ordering_ids;
						}
					}

					std::unique_lock<std::mutex> l(cached_appended_ordering_m);
					cached_appended_ordering_ready = true;
					cached_appended_ordering_cv.notify_all();
				}
				else
				{
					std::unique_lock<std::mutex> l(cached_appended_ordering_m);
					while (!cached_appended_ordering_ready)
					{
						cached_appended_ordering_cv.wait(l);
					}

					for (auto i = 0; i < transformations_orderings.size(); i++)
					{
						auto& t = transformations_orderings[i];
						if (t.id > 0)
						{
							if (auto found = cached_appended_ordering.find(t.id); found != std::end(cached_appended_ordering))
							{
								const auto& appended_ordering_ids = found->second;

								std::vector<TransformationOrdering> appended_orderings{};
								for (const auto& operand_id : appended_ordering_ids)
								{
									auto mul_floats = GetSpvFloatsVec(state, operand_id);
									if (mul_floats.size() != 16)
									{
										continue;
									}

									glm::mat4 data = glm::make_mat4x4(mul_floats.data());
									appended_orderings.push_back(TransformationOrdering{ data, true, {}, operand_id });
								}

								if (!appended_orderings.empty())
								{
									transformations_orderings.erase(std::begin(transformations_orderings) + i, std::begin(transformations_orderings) + i + 1);
									transformations_orderings.insert(std::begin(transformations_orderings) + i, std::begin(appended_orderings), std::end(appended_orderings));
									i += appended_orderings.size() - 1;
								}
							}
							else
							{
								PANIC("cached appended does not exist");
							}
						}
					}
				}
			}

			/*
			if (!is_cached_transformation_ordering.exchange(true))
			{
				std::deque<TransformationOrdering> ordered_transformations_orderings{};
				for (const auto& t: transformations_orderings)
				{
					if (t.right)
					{
						ordered_transformations_orderings.push_back(t);
					}
					else
					{
						ordered_transformations_orderings.push_front(t);
					}
				}
				//cached_transformation_ordering = std(std::begin(ordered_transformations_orderings), std::end(ordered_transformations_orderings));
			}
			*/
			
			mvp = glm::mat4(1.0f);

			bool switch_to_projection_view = false;
			glm::mat4 model(1.0f);
			glm::mat4 projection_view(1.0f);
			for (auto& t : transformations_orderings)
			{
				if (t.m != glm::mat4(1.0f))
				{
					auto& m = t.m;
					bool do_right = true;
					if (do_right)
					{
						if (!EXPAND_TO_MODEL_AND_PROJECTION_VIEW)
						{
							if (t.right)
							{
								model = m * model;
							}
							else
							{
								model = model * m;
							}
						}
						if (EXPAND_TO_MODEL_AND_PROJECTION_VIEW)
						{
							if (t.right)
							{
								if (!switch_to_projection_view)
								{
									// check if last row is changed (affine matrix otherwise, which means model matrix)
									for (auto i = 0; i < 3; i++)
									{
										if (m[i][3] != 0.0f)
										{
											switch_to_projection_view = true;
											break;
										}
									}

									if (!switch_to_projection_view)
									{
										model = m * model;
									}
									else
									{
										projection_view = m * projection_view;
									}
									switch_to_projection_view = true;
								}
								else
								{
									projection_view = m * projection_view;
								}

								/*
								if (!switch_to_projection_view)
								{
									model = m * model;
									switch_to_projection_view = true;
									// don't need to check this hard to see how view matrix comes into play
									auto new_mat = m * model;
									// check if last row is changed (affine matrix otherwise, which means model matrix)
									for (auto i = 0; i < 3; i++)
									{
										//if (new_mat[i][3] != 0.0f)
										{
											switch_to_projection_view = true;
											break;
										}
									}
									if (!switch_to_projection_view)
									{
										model = new_mat;
									}
								}
								if (switch_to_projection_view)
								{
									projection_view = m * projection_view;
								}
								*/
							}
							else
							{
								if (!switch_to_projection_view)
								{
									// check if last row is changed (affine matrix otherwise, which means model matrix)
									for (auto i = 0; i < 3; i++)
									{
										if (m[i][3] != 0.0f)
										{
											switch_to_projection_view = true;
											break;
										}
									}

									if (!switch_to_projection_view)
									{
										model = model * m;
									}
									else
									{
										projection_view = projection_view * m;
									}
									switch_to_projection_view = true;
								}
								else
								{
									projection_view = projection_view * m;
								}
							}
						}
					}
					else
					{
						if (t.right)
						{
							mvp = mvp * m;
						}
						else
						{
							mvp = m * mvp;
						}
					}
				}
			}

			if (1 || !ENABLE_CHECK_SPIRV_GL_POSITION)
			{
				// if we want to dump
				if(0)
				{
					{
						spvtools::SpirvTools spirv_tools(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
						std::string disassembly{};
						if (!spirv_tools.Disassemble(spv,
							&disassembly,
							SPV_BINARY_TO_TEXT_OPTION_INDENT |
							SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
							SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET |
							SPV_BINARY_TO_TEXT_OPTION_COMMENT))
						{
							PANIC("Failed to diassemble");
						}
						//fmt::print("{}", disassembly);
						std::ofstream f("debug.spv", std::ios::trunc);
						f << disassembly;
					}

					{
						std::ofstream f("debug_simplify.spv", std::ios::trunc);
						for (const auto& position_affected_lines : position_affected_lines_vector)
						{
							auto line = position_affected_lines.line;
							auto position_affected_id = position_affected_lines.inst.position_affected_id;
							auto debug_line = position_affected_lines.inst.debug_line;
							f << fmt::format("{}\n", debug_line);
						}
					}
				}

				return RunResult{ model, projection_view};
				auto _v94 = glm::make_vec4(GetSpvFloatsVec(state, 94).data());
			}

			// TODO
			// calcuation is a bit off...
			// put elipson here (still need to debug, probably some double -> float conversion?, vice-versa)
			//auto e = glm::epsilonEqual(gl_Position_actual, gl_Position_expected, 0.001f);
			//if (!glm::all(e))
			// 1% difference
			std::array<bool, 4> e;
			for (auto i = 0; i < gl_Position_expected.length(); i++)
			{
				auto epilson_amounts = std::abs(gl_Position_expected[i] * 0.01f);
				auto diff = std::abs(gl_Position_actual[i] - gl_Position_expected[i]);
				auto ee = diff > epilson_amounts;
				e[i] = ee;
			}
			if (std::any_of(std::begin(e), std::end(e), [](const auto& e) { return e == true; }))
			{
				mvp = glm::mat4(1.0f);
				for (auto& t : transformations_orderings)
				{
					if (t.m != glm::mat4(1.0f))
					{
						auto& m = t.m;
						if (t.right)
						{
							fmt::print("RIGHT: {}\n", t.right);
							fmt::print("{}\n", t.debug_line);
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(m[i]));
							}
							fmt::print("{}\n", "*");
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(mvp[i]));
							}
							fmt::print("=\n{}\n", glm::to_string(m * mvp * original_position));
							fmt::print("\n", glm::to_string(m), glm::to_string(mvp));
						}
						else
						{
							fmt::print("LEFT: {}\n", t.right);
							fmt::print("{}\n", t.debug_line);
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(mvp[i]));
							}
							fmt::print("{}\n", "*");
							for (auto i = 0; i < 4; i++)
							{
								fmt::print("{}\n", glm::to_string(m[i]));
							}
							fmt::print("=\n{}\n", glm::to_string(mvp * m * original_position));
							fmt::print("\n", glm::to_string(m), glm::to_string(mvp));
						}
						if (1)
						{
							if (t.right)
							{
								mvp = m * mvp;
							}
							else
							{
								mvp = mvp * m;
							}
						}
						else
						{
							if (t.right)
							{
								mvp = mvp * m;
							}
							else
							{
								mvp = m * mvp;
							}
						}
					}
				}

				for (auto i = 0; i < 4; i++)
				{
					fmt::print("{}\n", glm::to_string(mvp[i]));
				}

				{
					spvtools::SpirvTools spirv_tools(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
					std::string disassembly{};
					if (!spirv_tools.Disassemble(spv,
												 &disassembly,
												 SPV_BINARY_TO_TEXT_OPTION_INDENT |
												 SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
												 SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET |
												 SPV_BINARY_TO_TEXT_OPTION_COMMENT))
					{
						PANIC("Failed to diassemble");
					}
					//fmt::print("{}", disassembly);
					std::ofstream f("debug.spv", std::ios::trunc);
					f << disassembly;
				}
				{
					std::ofstream f("debug_simplify.spv", std::ios::trunc);
					for (const auto& position_affected_lines : position_affected_lines_vector)
					{
						auto line = position_affected_lines.line;
						auto position_affected_id = position_affected_lines.inst.position_affected_id;
						auto debug_line = position_affected_lines.inst.debug_line;
						f << fmt::format("{}\n", debug_line);
					}
				}

				fmt::print("position {}\n", glm::to_string(original_position));
				fmt::print("MVP {}\n", glm::to_string(mvp));
				fmt::print("gl_Position expected: {}\n", glm::to_string(gl_Position_expected));
				fmt::print("gl_Position actual: {}\n", glm::to_string(gl_Position_actual));

				return StringError("MVP not producing same gl_Position\n");
				PANIC("MVP not producing same gl_Position\n");
			}

			//return mvp;
			return RunResult{ mvp };
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, int data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_int:
				if (members.size() != 1)
				{
					return StringError("Should have single member");
				}
				members[0]->value.s = std::move(data);
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::ivec2 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 2)
				{
					return StringError("Should have 2 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.s = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::ivec3 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 3)
				{
					return StringError("Should have 3 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.s = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::ivec4 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.s = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, unsigned int data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_int:
				if (members.size() != 1)
				{
					return StringError("Should have single member");
				}
				members[0]->value.u = std::move(data);
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::uvec2 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 2)
				{
					return StringError("Should have 2 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.u = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::uvec3 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 3)
				{
					return StringError("Should have 3 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.u = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::uvec4 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.u = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, float data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			if (members.size() != 1)
			{
				return StringError("Not in sync");
			}

			members[0]->value.f = std::move(data);
			/*
			switch (pointer_type->value_type)
			{
			case spvm_value_type_float:
				if (members.size() != 1)
				{
					return StringError("Should have single member");
				}
				members[0]->value.f = std::move(data);
				break;
			default:
				return StringError("Not in sync");
			}
			*/

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::vec2 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			if (members.size() != 2)
			{
				return StringError("Should have 2 members");
			}
			for (auto i = 0; i < members.size(); i++)
			{
				members[i]->value.f = data[i];
			}

			/*
			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 2)
				{
					return StringError("Should have 2 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}
			*/

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::vec3 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 3)
				{
					return StringError("Should have 3 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				switch (pointer_type->type)
				{
				case spvm_value_type_vector:
					if (members.size() != 3)
					{
						return StringError("Should have 3 members");
					}
					for (auto i = 0; i < members.size(); i++)
					{
						members[i]->value.f = data[i];
					}
					break;
				default:
					return StringError("Not in sync");
				}
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::vec4 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("{} should have 4 members", name);
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				break;
				//return StringError("Not in sync");
			}
			switch (pointer_type->type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::mat2 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_matrix:
				if (members.size() != 2)
				{
					return StringError("Should have 2 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					auto& member = members[i];
					for (auto j = 0; j < member->member_count; j++)
					{
						auto& sub_member = member->members[j];

						sub_member.value.f = data[i][j];
					}
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::mat3 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_matrix:
				if (members.size() != 3)
				{
					return StringError("Should have 3 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					auto& member = members[i];
					for (auto j = 0; j < member->member_count; j++)
					{
						auto& sub_member = member->members[j];

						sub_member.value.f = data[i][j];
					}
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, glm::mat4 data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_matrix:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					auto& member = members[i];
					for (auto j = 0; j < member->member_count; j++)
					{
						auto& sub_member = member->members[j];

						sub_member.value.f = data[i][j];
					}
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, std::vector<glm::vec2> data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			/*
			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 2)
				{
					return StringError("{} should have 2 members", name);
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				break;
				//return StringError("Not in sync");
			}
			switch (pointer_type->type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}
			*/

			return NoError();
		}


		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, std::vector<glm::vec3> data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			/*
			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("{} should have 4 members", name);
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				break;
				//return StringError("Not in sync");
			}
			switch (pointer_type->type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}
			*/

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, std::vector<glm::vec4> data)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			/*
			switch (pointer_type->value_type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("{} should have 4 members", name);
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				break;
				//return StringError("Not in sync");
			}
			switch (pointer_type->type)
			{
			case spvm_value_type_vector:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					members[i]->value.f = data[i];
				}
				break;
			default:
				return StringError("Not in sync");
			}
			*/

			return NoError();
		}

		Error SPIRVVirtualMachine::SetVariable(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name, PointerView view)
		{
			PANIC("Not supported");
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			switch (pointer_type->value_type)
			{
			case spvm_value_type_matrix:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					auto& member = members[i];
					for (auto j = 0; j < member->member_count; j++)
					{
						auto& sub_member = member->members[j];

						//sub_member.value.f = data[i][j];
					}
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return NoError();
		}

		Expected<glm::vec4> SPIRVVirtualMachine::GetVariableVec4(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			glm::vec4 data{};
			if (pointer_type->type == spvm_value_type_vector || pointer_type->value_type == spvm_value_type_vector)
			{
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					data[i] = members[i]->value.f;
				}
			}
			else
			{
				return StringError("Not in sync");
			}

			return data;
		}

		Expected<glm::mat4> SPIRVVirtualMachine::GetVariableMat4(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name)
		{
			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			glm::mat4 data{};
			switch (pointer_type->value_type)
			{
			case spvm_value_type_matrix:
				if (members.size() != 4)
				{
					return StringError("Should have 4 members");
				}
				for (auto i = 0; i < members.size(); i++)
				{
					auto& member = members[i];
					for (auto j = 0; j < member->member_count; j++)
					{
						auto& sub_member = member->members[j];

						data[i][j] = sub_member.value.f;
					}
				}
				break;
			default:
				return StringError("Not in sync");
			}

			return data;
		}

		Expected<const std::vector<float*>&> SPIRVVirtualMachine::GetVariableInBytesRef(SPIRVVirtualMachineState& spirv_vm_state, std::string_view name)
		{
			auto& name_to_spv_floats_cache = spirv_vm_state.name_to_spv_floats_cache;
			if (auto found = name_to_spv_floats_cache.find(name); found != std::end(name_to_spv_floats_cache))
			{
				return found->second;
			}

			AssignOrReturnError(SPIRVMVariable& vm_variable, GetUnderlyingVMType(spirv_vm_state, name));
			auto& pointer_type = vm_variable.GetPointerType();
			auto& members = vm_variable.GetMembers();

			auto* state = spirv_vm_state.vm_state.get();
			auto data2 = GetSpvFloats(state, pointer_type->value_type);
			for (spvm_word i = 0; i < state->owner->bound; i++)
			{
				spvm_result_t slot = &state->results[i];
				auto name2 = slot->name;
				auto type = slot->type;
				auto owner = slot->owner;
				auto members = slot->members;
				auto member_count = slot->member_count;
				if (type == spvm_result_type_variable && name2 != nullptr)
				{
					spvm_result_t vm_type = spvm_state_get_type_info(state->results, &state->results[slot->pointer]);
					if (owner == nullptr)
					{
						if (name == name2)
						{
							auto data2 = GetSpvFloats(state, i);
							auto [pair, _] = name_to_spv_floats_cache.emplace(name, data2);
							return pair->second;
						}
					}
				}
			}

			if (data2.size() == 0)
			{
				for (const auto& member : members)
				{
					data2.emplace_back(&member->value.f);
				}
			}

			if (data2.empty())
			{
				PANIC("DATA HAS NOTHING");
			}

			auto [pair, _] = name_to_spv_floats_cache.emplace(name, data2);
			return pair->second;
		}

		Expected<SPIRVMVariable&> SPIRVVirtualMachine::GetUnderlyingVMType(SPIRVVirtualMachineState& spirv_vm_state, std::string_view var)
		{
			auto& name_to_variable_cache = spirv_vm_state.name_to_variable_cache;
			auto* state = spirv_vm_state.vm_state.get();

			// exists in cache
			if (auto found = name_to_variable_cache.find(var); found != std::end(name_to_variable_cache))
			{
				return found->second;
			}

			// modified from https://github.com/dfranx/SHADERed/blob/d43338e0009ed5837384e2dbd43a30168ccb0519/src/SHADERed/Objects/DebugInformation.cpp

			// "var.lights[0].diffuse", "lights[0].diffuse", etc...
			constexpr auto indexer_strings = ".[";

			// var
			std::string_view base_var = var;
			auto base_v_idx = std::string_view::npos;
			if (base_v_idx = base_var.find_first_of(indexer_strings) != std::string_view::npos)
			{
				base_var = base_var.substr(0, base_v_idx);
			}

			spvm_result_t vm_base_var = spvm_state_get_result(state, const_cast<const spvm_string>(base_var.data()));

			// Start parsing
			spvm_member_t* pointer = nullptr;
			spvm_result_t pointer_type = nullptr;
			spvm_word pointer_m_count = 0;

			if (vm_base_var)
			{
				pointer = &vm_base_var->members;
				pointer_m_count = vm_base_var->member_count;

				if (vm_base_var->pointer)
				{
					pointer_type = spvm_state_get_type_info(state->results, &state->results[vm_base_var->pointer]);
				}
			}
			else
			{
				return StringError(fmt::format("Variable not found {}", var));
				/*
				// search in the nameless buffers
				bool foundBufferVariable = false;
				for (spvm_word i = 0; i < m_shader->bound; i++)
				{
					if (m_vm->results[i].name && strcmp(m_vm->results[i].name, "") == 0 && m_vm->results[i].type == spvm_result_type_variable)
					{
						spvm_result_t anonBuffer = &m_vm->results[i];
						spvm_result_t anonBufferType = spvm_state_get_type_info(m_vm->results, &m_vm->results[anonBuffer->pointer]);


						for (spvm_word j = 0; j < anonBufferType->member_name_count; j++)
							if (strcmp(anonBufferType->member_name[j], tokenName.c_str()) == 0)
							{
								pointer = anonBuffer->members[j].members;
								pointerMCount = anonBuffer->members[j].member_count;

								if (anonBuffer->members[j].type)
									pointerType = spvm_state_get_type_info(m_vm->results, &m_vm->results[anonBuffer->members[j].type]);

								foundBufferVariable = true;

								break;
							}
					}

					if (foundBufferVariable) break;
				}
				*/
			}

			if (!pointer || !*pointer)
			{
				return StringError("Pointer should not be null");
			}

			// find remaining accessors
			// lights[0].diffuse
			auto v_idx = base_v_idx + 1;
			auto next_v_idx = v_idx;
			while (next_v_idx = var.find_first_of(indexer_strings, v_idx) != std::string_view::npos)
			{
				auto token = var.substr(v_idx, next_v_idx - v_idx);
				v_idx = next_v_idx + 1;

				// array access
				// lights[0]
				//       ^

				if (token.size() > 0 && isdigit(token[0]))
				{
					// remove "]"
					token = token.substr(0, token.size() - 1);

					// check numbers in between "[" and "]"
					auto is_all_digits = std::all_of(std::begin(token), std::end(token), ::isdigit);
					if (is_all_digits && !token.empty())
					{
						int array_index_access{};
						if (auto [p, ec] = std::from_chars(token.data(), token.data() + token.size(), array_index_access); ec == std::errc())
						{
							return StringError(fmt::format("Error converting token: {} to number", token.data()));
						}

						if (array_index_access < pointer_m_count)
						{
							spvm_member_t mem = *pointer + array_index_access;

							pointer_m_count = mem->member_count;
							if (mem->type)
							{
								pointer_type = spvm_state_get_type_info(state->results, &state->results[mem->type]);
							}
							pointer = &mem->members;
						}
					}
				}
				// member name
				else if (pointer_type)
				{
					for (int i = 0; i < pointer_type->member_name_count; i++)
					{
						if (token == pointer_type->member_name[i]);
						{
							spvm_member_t mem = *pointer + i;

							pointer_m_count = mem->member_count;
							if (mem->type)
							{
								pointer_type = spvm_state_get_type_info(state->results, &state->results[mem->type]);
							}
							pointer = &mem->members;
						}
					}
				}
			}

			// last access
			// "diffuse"
			if (auto token = var.substr(v_idx); !token.empty())
			{
				if (token.size() > 0 && isdigit(token[0]))
				{
					// remove "]"
					token = token.substr(0, token.size() - 1);

					// check numbers in between "[" and "]"
					auto is_all_digits = std::all_of(std::begin(token), std::end(token), ::isdigit);
					if (is_all_digits && !token.empty())
					{
						int array_index_access{};
						if (auto [p, ec] = std::from_chars(token.data(), token.data() + token.size(), array_index_access); ec == std::errc())
						{
							return StringError(fmt::format("Error converting token: {} to number", token.data()));
						}

						if (array_index_access < pointer_m_count)
						{
							spvm_member_t mem = *pointer + array_index_access;

							pointer_m_count = mem->member_count;
							if (mem->type)
							{
								pointer_type = spvm_state_get_type_info(state->results, &state->results[mem->type]);
							}
							pointer = &mem->members;
						}
					}
				}
				// member name
				else if (pointer_type)
				{
					for (int i = 0; i < pointer_type->member_name_count; i++)
					{
						if (token == pointer_type->member_name[i]);
						{
							spvm_member_t mem = *pointer + i;

							pointer_m_count = mem->member_count;
							if (mem->type)
							{
								pointer_type = spvm_state_get_type_info(state->results, &state->results[mem->type]);
							}
							pointer = &mem->members;
						}
					}
				}
			}

			SPIRVMVariable spirvvm_variable(pointer_type, pointer, MembersToVec((*pointer)->members, (*pointer)->member_count));
			auto [pair, _] = name_to_variable_cache.emplace(var, spirvvm_variable);
			return pair->second;
		}

		Error SPIRVVirtualMachine::AssociateUnderlyingVMType(SPIRVVirtualMachineState& dst, SPIRVVirtualMachineState& src, std::string_view var)
		{
			AssignOrReturnError(SPIRVMVariable& dst_spirv_variable, GetUnderlyingVMType(dst, var));
			AssignOrReturnError(SPIRVMVariable& src_spirv_variable, GetUnderlyingVMType(src, var));

			auto& dst_pointer_type = dst_spirv_variable.GetPointerType();
			auto& src_pointer_type = src_spirv_variable.GetPointerType();

			auto dst_owner_member = dst_spirv_variable.GetOwnerMember();
			auto src_owner_member = src_spirv_variable.GetOwnerMember();

			auto& dst_members = dst_spirv_variable.GetMembers();
			auto& src_members = src_spirv_variable.GetMembers();

			if (dst_members.size() != src_members.size())
			{
				return StringError("Src and dst members size do not match! {} != {}", dst_members.size(), src_members.size());
			}

			// Already aliased
			if (*dst_owner_member == *src_owner_member)
			{
				return NoError();
			}

			spvm_member_free(dst_members[0], dst_members.size());
			
			*dst_owner_member = *src_owner_member;
			dst_members = src_members;

			dst.name_to_variable_cache[var] = src.name_to_variable_cache[var];
			dst.name_to_spv_floats_cache[var] = src.name_to_spv_floats_cache[var];
		
			return NoError();
		}

		Error SPIRVVirtualMachine::DeassociateUnderlyingVMType(SPIRVVirtualMachineState& dst, std::string_view var)
		{
			AssignOrReturnError(SPIRVMVariable& dst_spirv_variable, GetUnderlyingVMType(dst, var));

			auto& dst_pointer_type = dst_spirv_variable.GetPointerType();
			auto dst_owner_member = dst_spirv_variable.GetOwnerMember();
			auto& dst_members = dst_spirv_variable.GetMembers();

			*dst_owner_member = nullptr;

			dst.name_to_variable_cache.erase(var);
			dst.name_to_spv_floats_cache.erase(var);

			return NoError();
		}
	}
}