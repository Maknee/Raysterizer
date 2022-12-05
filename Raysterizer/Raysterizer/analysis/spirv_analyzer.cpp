#include "analysis/spirv_analyzer.h"

namespace Raysterizer
{
	namespace Analysis
	{
		using namespace spvtools::opt;
		constexpr char* POSITION_ID_MESSAGE = "POSITION";
		constexpr char* AFFECT_POSITION_ID_MESSAGE = "AFFECT_POSITION";
		constexpr char* ID_CONSTRUCTION_ID_MESSAGE = "ID_CONSTRUCTION";

		struct InstructionPointerHasher
		{
			std::size_t operator()(const Instruction* i) const
			{
				return std::hash<void*>()((void*)i);
			}
		};

		struct InstructionPointerCompare
		{
			bool operator()(const Instruction* i1, const Instruction* i2) const
			{
				return *i1 == *i2;
			}
		};

		class Graph
		{
		public:
			void AddNeighbor(Instruction* n, Instruction* neighbor)
			{
				if (auto found = node_to_neighbors.find(n); found != std::end(node_to_neighbors))
				{
					auto& neighbors = found->second;
					neighbors.emplace(neighbor);
				}
				else
				{
					node_to_neighbors[n] = { neighbor };
				}
			}

			phmap::flat_hash_set<Instruction*>& GetNeighbors(Instruction* n)
			{
				if (auto found = node_to_neighbors.find(n); found != std::end(node_to_neighbors))
				{
					return found->second;
				}
				else
				{
					node_to_neighbors[n] = {};
					return node_to_neighbors[n];
					//PANIC("Not found");
				}
			}

			auto& GetNodeToNeighbors() { return node_to_neighbors; }

		private:
			flat_hash_map<Instruction*, phmap::flat_hash_set<Instruction*>, InstructionPointerHasher, InstructionPointerCompare> node_to_neighbors;
		};

		class MVPAnalysisPass : public MemPass
		{
		public:
			explicit MVPAnalysisPass(std::vector<uint32_t> spv_) :
				spv(std::move(spv_))
			{
			}

			const char* name() const override { return "MVP Analysis"; }

			IRContext::Analysis GetPreservedAnalyses() override
			{
				return IRContext::kAnalysisDefUse |
					IRContext::kAnalysisInstrToBlockMapping |
					IRContext::kAnalysisCombinators | IRContext::kAnalysisCFG |
					IRContext::kAnalysisDominatorAnalysis |
					IRContext::kAnalysisLoopAnalysis |
					IRContext::kAnalysisScalarEvolution |
					IRContext::kAnalysisRegisterPressure |
					IRContext::kAnalysisValueNumberTable |
					IRContext::kAnalysisStructuredCFG |
					IRContext::kAnalysisBuiltinVarId |
					IRContext::kAnalysisIdToFuncMapping |
					IRContext::kAnalysisNameMap;
			}

			Status Process() final
			{
				PerformAnalysis();

				return Status::SuccessWithoutChange;
			}

		private:
			std::vector<uint32_t> spv{};
			glm::mat4 mvp{};
			std::size_t instruction_line = 0;
			std::size_t read_mvp_instruction_line = 0;
			bool found_main = false;

			void PerformAnalysis()
			{
				auto name_context = std::unique_ptr<spv_context_t>(spvContextCreate(context()->grammar().target_env()));
				auto friendly_mapper = spvtools::FriendlyNameMapper(name_context.get(), spv.data(), spv.size());

				// Attempt to get last write to gl_Position
				// Find Store and then trace to multiply mat and vec
				const auto& create_message = consumer();
				auto PrintInst = [&](Instruction* inst)
				{
#ifndef NDEBUG
					if(0)
					{
						auto response = inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
						fmt::print("{}\n", response);
					}
#endif
				};
				
				auto InstToString = [](Instruction* inst) -> std::string
				{
					auto str = std::string(inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));
					return str;
				};

				/*
				for (auto& func : *get_module())
				{
					DominatorTree& dom_tree =
						context()->GetDominatorAnalysis(&func)->GetDomTree();

					dom_tree.DumpTreeAsDot(std::cout);

					for (const DominatorTreeNode& node : dom_tree)
					{
						fmt::print("{}\n", node.id());
					}
				}
				*/

				// Find the spirv id for position out attribute variable
				Instruction* position_type_inst = nullptr;
				for (Instruction& inst : get_module()->types_values())
				{
					auto result_id = inst.result_id();
					auto opcode = inst.opcode();
					//PrintInst(&inst);

					if (opcode == SpvOpVariable)
					{
						for (const auto& decoration : get_decoration_mgr()->GetDecorationsFor(result_id, false))
						{
							PrintInst(decoration);
							/*
							 * %_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
							 * %POSITION = OpVariable %_ptr_Input_v4float Input
							 *
							 * ->
							 *
							 * OpDecorate% POSITION Location 0
							 */
							auto decoration_id = decoration->GetSingleWordInOperand(1);
							if (decoration_id == spv::Decoration::DecorationLocation)
							{
								auto uniform_location = decoration->GetSingleWordInOperand(2);
								if (uniform_location == 0)
								{
									create_message(SPV_MSG_INFO, POSITION_ID_MESSAGE, { result_id, 0, 0 }, "");
									position_type_inst = &inst;
									break;
								}
							}
						}
					}
				}

				if (!position_type_inst)
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
						std::ofstream f("debug.spv", std::ios::trunc);
						f << disassembly;
					}

					PANIC("No position id");
				}
				PrintInst(position_type_inst);
				const auto position_id = position_type_inst->result_id();

				if (0)
				{
					fmt::print("START POSITION INST\n");
					PrintInst(position_type_inst);
				}
				// Search all references until gl_Position is hit
				/*
				get_module()->ForEachInst([&](Instruction* inst)
				{
					auto opcode = inst->opcode();
					if (!found_main)
					{
						if (opcode == SpvOpFunction)
						{
							auto function_result_id = inst->result_id();
							auto function_name = friendly_mapper.NameForId(function_result_id);
							if (function_name == "main")
							{
								found_main = true;
								return;
							}
						}
					}
					if (!found_main)
					{
						return;
					}

					if (opcode == SpvOpAccessChain)
					{
						auto result_id = inst->result_id();
						auto num_operands = inst->NumOperands();
						if (num_operands == 4)
						{
							auto pointer_id = inst->GetSingleWordInOperand(0);

							Instruction* pointer_inst = get_def_use_mgr()->GetDef(pointer_id);
							uint32_t pointer_type_id = pointer_inst->type_id();
							Instruction* pointer_type_inst = get_def_use_mgr()->GetDef(pointer_type_id);
							uint32_t type_id = pointer_type_inst->GetSingleWordInOperand(1);
							PrintInst(pointer_inst);
							PrintInst(pointer_type_inst);

							auto* const_mgr = context()->get_constant_mgr();

							auto ptr_name = friendly_mapper.NameForId(pointer_type_id);
							if (ptr_name == "_ptr_Output_gl_PerVertex")
							{
								get_def_use_mgr()->WhileEachUser(inst, [&](Instruction* user_inst)
								{
									PrintInst(user_inst);

									//OpStore %70 %68
									if (user_inst->opcode() == SpvOpStore)
									{
										//%70
										uint32_t dest_opsource_inst_id = user_inst->GetSingleWordInOperand(0);
										Instruction* dest_opsource_inst = get_def_use_mgr()->GetDef(dest_opsource_inst_id);
										PrintInst(dest_opsource_inst);
										if (inst == dest_opsource_inst)
										{
											//%68
											uint32_t source_opsource_inst_id = user_inst->GetSingleWordInOperand(1);
											Instruction* source_opsource_inst = get_def_use_mgr()->GetDef(source_opsource_inst_id);
											PrintInst(source_opsource_inst);

											if (source_opsource_inst->opcode() == SpvOpMatrixTimesVector)
											{
												//%68 = OpMatrixTimesVector %v4float %62 %67
												uint32_t matrix_id = source_opsource_inst->GetSingleWordInOperand(0);
												uint32_t vector_id = source_opsource_inst->GetSingleWordInOperand(1);
												PrintInst(get_def_use_mgr()->GetDef(matrix_id));
												PrintInst(get_def_use_mgr()->GetDef(vector_id));
												read_mvp_instruction_line = instruction_line;

												create_message(SPV_MSG_INFO, name(), { matrix_id, 0, 0 }, "");
											}
										}
									}
									return true;
								});
							}

							for (auto i = 1; i < num_operands; ++i)
							{
								Instruction* type_inst = get_def_use_mgr()->GetDef(type_id);
								auto result_id = type_inst->result_id();

								PrintInst(type_inst);
								switch (type_inst->opcode())
								{
								case SpvOpTypeStruct: {
									const analysis::IntConstant* member_idx =
										const_mgr->FindDeclaredConstant(inst->GetSingleWordInOperand(i))
										->AsIntConstant();
									assert(member_idx);
									if (member_idx->type()->AsInteger()->width() == 32)
									{
										type_id = type_inst->GetSingleWordInOperand(member_idx->GetU32());
									}
									else
									{
										type_id = type_inst->GetSingleWordInOperand(
											static_cast<uint32_t>(member_idx->GetU64()));
									}
								} break;
								case SpvOpTypeArray:
								case SpvOpTypeRuntimeArray:
								case SpvOpTypeVector:
								case SpvOpTypeMatrix:
									type_id = type_inst->GetSingleWordInOperand(0);
									break;
								default:
									break;
								}
							}
						}
					}
				});
				*/

				std::vector<Instruction*> instructions;
				flat_hash_map<Instruction*, std::size_t> instruction_to_index;
				auto instruction_index = 0;
				std::vector<Instruction*> gl_Position_access_instructions;
				Instruction* gl_Position_var_inst{};

				flat_hash_map<Instruction*, std::size_t> main_instruction_to_word_offset;
				auto main_index = 0;

				std::vector<Instruction*> main_local_variable_insts;
				get_module()->ForEachInst([&](Instruction* inst)
				{
					instructions.emplace_back(inst);
					instruction_to_index[inst] = instruction_index++;

					auto opcode = inst->opcode();
					if (!found_main)
					{
						if (opcode == SpvOpFunction)
						{
							auto function_result_id = inst->result_id();
							auto function_name = friendly_mapper.NameForId(function_result_id);
							if (function_name == "main")
							{
								found_main = true;
							}
						}
						else
						{
							return;
						}
					}
					else
					{
						if (!spvOpcodeIsDebug(opcode))
						{
							//PrintInst(inst);
						}
						else
						{
							//fmt::print("DEBUG\n");
						}
						//PrintInst(inst);
						main_instruction_to_word_offset[inst] = main_index;
						main_index++;

						if (opcode == SpvOpVariable)
						{
							main_local_variable_insts.emplace_back(inst);
						}

						if (opcode == SpvOpFunctionEnd)
						{
							found_main = false;
						}
					}

					if (opcode == SpvOpAccessChain)
					{
						auto result_id = inst->result_id();
						auto num_operands = inst->NumOperands();
						if (num_operands == 4)
						{
							auto pointer_id = inst->GetSingleWordInOperand(0);

							Instruction* pointer_inst = get_def_use_mgr()->GetDef(pointer_id);
							uint32_t pointer_type_id = pointer_inst->type_id();
							Instruction* pointer_type_inst = get_def_use_mgr()->GetDef(pointer_type_id);

							auto* const_mgr = context()->get_constant_mgr();

							auto ptr_name = friendly_mapper.NameForId(pointer_type_id);
							if (ptr_name == "_ptr_Output_gl_PerVertex")
							{
								uint32_t type_id = pointer_type_inst->GetSingleWordInOperand(1);
								auto type_id_name = friendly_mapper.NameForId(type_id);

								uint32_t access_offset_id = inst->GetSingleWordInOperand(1);
								auto access_offset_name = friendly_mapper.NameForId(access_offset_id);

								// make sure accesses gl_Position
								if (type_id_name == "gl_PerVertex" && access_offset_name == "int_0")
								{
									gl_Position_var_inst = pointer_inst;
									get_def_use_mgr()->WhileEachUser(inst, [&](Instruction* user_inst)
									{
										// OpStore %376 %374
										if (user_inst->opcode() == SpvOpStore)
										{
											gl_Position_access_instructions.emplace_back(user_inst);
										}
										else
										{
											PANIC("Assume always SpvOpStore to store vec4 into gl_Position");
										}
										return true;
									});
								}                       
							}
						}
					}
				});

				if (gl_Position_access_instructions.size() != 1)
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
						std::ofstream f("debug.spv", std::ios::trunc);
						f << disassembly;
					}

					PANIC("gl_Position access is assumed to be 1");
				}

				auto& gl_Position_access_instruction = gl_Position_access_instructions[0];
				//PrintInst(gl_Position_access_instruction);

				/*
				 * %376 = OpAccessChain %_ptr_Output_v4float %__1 %int_0
				 * OpStore %376 %374
				 *              ^
				 * Find the code that xrefs here (cause %376 is gl_Position)
				 * 
				 * %374 = OpLoad %v4float %_768
				 * 
				 * where %374 is the value stored into gl_Position
				 */

				const auto gl_Position_id = gl_Position_access_instruction->GetSingleWordInOperand(1);
				Instruction* start_inst = get_def_use_mgr()->GetDef(gl_Position_access_instruction->GetSingleWordInOperand(1));

				if (0)
				{
					fmt::print("START INST\n");
					PrintInst(start_inst);
				}

				const Function* main_function{};
				for (const auto& f : *get_module())
				{
					auto function_name = friendly_mapper.NameForId(f.result_id());
					if (function_name == "main")
					{
						main_function = &f;
						break;
					}
				}
				DominatorAnalysis* dominator_analysis = context()->GetDominatorAnalysis(main_function);
				PostDominatorAnalysis* post_dominator_analysis = context()->GetPostDominatorAnalysis(main_function);

				// Start the graph off with the instruction that loads gl_Position
				// %511 = OpLoad %v4float %510
				// ^ gl_Position          ^ gl_Position pointer
				Graph graph;
				graph.AddNeighbor(gl_Position_access_instruction, start_inst);
				std::queue<Instruction*> instructions_to_visit{};
				instructions_to_visit.emplace(start_inst);


				if (0)
				{
					/*
					 * Find variable to stores / loads
					 */
					Graph variable_inst_to_access_or_load_inst;
					Graph access_or_load_inst_to_variable_inst;
					Graph variable_inst_to_store_inst;
					Graph store_inst_to_variable_inst;

					get_module()->ForEachInst([&](Instruction* inst)
						{
							auto opcode = inst->opcode();
							if (opcode != SpvOpVariable)
							{
								return;
							}
							get_def_use_mgr()->WhileEachUse(inst, [&](Instruction* ref_inst, uint32_t id)
								{
									auto opcode = ref_inst->opcode();
									if (opcode == SpvOpName)
									{
										return true;
									}
									//fmt::print("{} -> {} !!\n", InstToString(inst), InstToString(ref_inst));
									
									if (opcode == SpvOpAccessChain || opcode == SpvOpLoad)
									{
										variable_inst_to_access_or_load_inst.AddNeighbor(inst, ref_inst);
										access_or_load_inst_to_variable_inst.AddNeighbor(ref_inst, inst);
									}
									else if (opcode == SpvOpStore)
									{
										variable_inst_to_store_inst.AddNeighbor(inst, ref_inst);
										store_inst_to_variable_inst.AddNeighbor(ref_inst, inst);
									}

									if (opcode == SpvOpAccessChain)
									{
										get_def_use_mgr()->WhileEachUse(ref_inst, [&](Instruction* ref_inst2, uint32_t id)
											{
												auto opcode = ref_inst2->opcode();
												if (opcode == SpvOpName)
												{
													return true;
												}

												if (opcode == SpvOpStore)
												{
													//fmt::print("{} -> {} !!\n", InstToString(inst), InstToString(ref_inst2));
													variable_inst_to_store_inst.AddNeighbor(inst, ref_inst2);
													store_inst_to_variable_inst.AddNeighbor(ref_inst2, inst);
												}
												else if (opcode == SpvOpLoad)
												{
													//fmt::print("{} -> {} !!\n", InstToString(inst), InstToString(ref_inst2));
													variable_inst_to_access_or_load_inst.AddNeighbor(inst, ref_inst2);
													access_or_load_inst_to_variable_inst.AddNeighbor(ref_inst2, inst);
												}
												return true;
											});
									}

									return true;
								});
						});

					/*
					 * Make a second pass to account for stores that are not directly 
					 * TODO?
					 */

					if (0)
					{
						for (auto& [n, vars] : store_inst_to_variable_inst.GetNodeToNeighbors())
						{
							for (auto& var : vars)
							{
								fmt::print("{:08X} | {} -> {} \n", (uintptr_t)n, InstToString(n), InstToString(var));
							}
						}

						fmt::print("||||||\n");
						for (auto& [n, vars] : variable_inst_to_access_or_load_inst.GetNodeToNeighbors())
						{
							for (auto& var : vars)
							{
								fmt::print("{} -> {} \n", InstToString(n), InstToString(var));
							}
						}
					}

					flat_hash_map<Instruction*, std::vector<std::vector<Instruction*>>, InstructionPointerHasher, InstructionPointerCompare> load_or_access_to_store_paths;

					bool found_main = false;
					get_module()->ForEachInst([&](Instruction* inst)
						{
							auto opcode = inst->opcode();
							if (!found_main)
							{
								if (opcode == SpvOpFunction)
								{
									auto function_result_id = inst->result_id();
									auto function_name = friendly_mapper.NameForId(function_result_id);
									if (function_name == "main")
									{
										found_main = true;
										return;
									}
								}
							}
							if (!found_main)
							{
								return;
							}

							switch (opcode)
							{
							case SpvOpAccessChain:
							case SpvOpLoad:
							{
								std::vector<Instruction*> path;

								// loop until we find a store! (must always end on store???)
								auto DFSRecursion = [&](auto&& DFSRecursion_, Instruction* n) -> void
								{
									path.emplace_back(n);
									if (n->result_id())
									{
										get_def_use_mgr()->ForEachUser(n->result_id(), [&](Instruction* ref_inst)
											{
												//fmt::print("{} -> {}\n", InstToString(n), InstToString(ref_inst));
												DFSRecursion_(DFSRecursion_, ref_inst);
											});
									}
									else
									{
										if (n->opcode() != SpvOpStore)
										{
											PANIC("Expected store?");
										}
										load_or_access_to_store_paths[inst].emplace_back(path);
										//fmt::print("END! {}\n", InstToString(n));
									}
									path.pop_back();
								};


								//fmt::print("Woot\n");
								DFSRecursion(DFSRecursion, inst);
								//fmt::print("Woot2\n");
								break;
							}
							case SpvOpStore:
							{
								break;
							}
							default:
							{
								break;
							}
							}
						}
					);


					if (0)
					{
						fmt::print("!!!!!!!!!!!\n");
						for (auto& [n, pss] : load_or_access_to_store_paths)
						{
							fmt::print("1 \t{}\n", InstToString(n));
							//for (auto& ps : pss)
							auto& ps = pss[0];
							{
								fmt::print("\n");
								for (auto& p : ps)
								{
									fmt::print("2 \t\t{}\n", InstToString(p));
								}
								fmt::print("\n");
							}
						}
						fmt::print("!!!!!!!!!!!\n");
					}

					/*
					 * Find all paths
					 */
					if (0)
					{
						// %POSITION = OpVariable %_ptr_Input_v4float Input
						Instruction* begin_inst = position_type_inst;
						Instruction* end_inst = start_inst;

						phmap::flat_hash_set<Instruction*> unvisited_instructions{ begin_inst };
						phmap::flat_hash_set<Instruction*> visited_instructions;

						while (!unvisited_instructions.empty())
						{
							Instruction* inst = *std::begin(unvisited_instructions);
							unvisited_instructions.erase(inst);

							if (visited_instructions.contains(inst))
							{
								continue;
							}
							else
							{
								visited_instructions.emplace(inst);
							}

							if (0 && InstToString(inst).find("%187 = OpLoad %f") != std::string::npos)
							{
								fmt::print("XDDD\n");
							}

							if (IsOpcodeSkippable(inst->opcode()))
							{
								continue;
							}

							const auto start_operand_index = inst->type_id() > 0 ? 1 : 0 + inst->result_id() > 0 ? 1 : 0;
							for (auto i = start_operand_index; i < inst->NumOperands(); i++)
							{
								const Operand& operand = inst->GetOperand(i);
								const auto& words = operand.words;
								if (words.size() > 1)
								{
									continue;
								}
								const auto& word = words[0];

								// Ignore operands defining types
								if (
									operand.type != SPV_OPERAND_TYPE_ID &&
									operand.type != SPV_OPERAND_TYPE_RESULT_ID &&
									operand.type != SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID &&
									operand.type != SPV_OPERAND_TYPE_SCOPE_ID
									)
								{
									continue;
								}

								// if def isn't the same as the current instruction (the instruction is declared somewhere else)
								Instruction* def_inst = get_def_use_mgr()->GetDef(word);
								if (def_inst)
								{
									if (inst != def_inst)
									{
										auto& neighbors = graph.GetNeighbors(inst);
										// Add it as a neighbor
										if (!neighbors.contains(def_inst))
										{
											//fmt::print("Neighbor def inst {} -> {}\n", InstToString(inst), InstToString(def_inst));
											graph.AddNeighbor(inst, def_inst);
											unvisited_instructions.emplace(def_inst);
										}
									}
								}

								get_def_use_mgr()->WhileEachUser(inst, [&](Instruction* ref_inst)
									{
										if (ref_inst == inst)
										{
											return true;
										}
										unvisited_instructions.emplace(ref_inst);

										Instruction* parent_inst = inst;
										if (parent_inst->opcode() == SpvOpAccessChain || parent_inst->opcode() == SpvOpVariable)
										{
											std::swap(parent_inst, ref_inst);
										}
										graph.AddNeighbor(ref_inst, parent_inst);
										//fmt::print("Neighbor user {} -> {}\n", InstToString(ref_inst), InstToString(parent_inst));

										return true;
									});
							}
						}
					}

					/*
					 * Create load to store paths
					 * load X ... insts ... store Y
					 */
					flat_hash_map<Instruction*, std::vector<std::vector<Instruction*>>, InstructionPointerHasher, InstructionPointerCompare> load_to_store;
					if (0)
					{
						{
							Instruction* start_n = nullptr;
							std::vector<Instruction*> path;
							auto DFSRecursion = [&](auto&& DFSRecursion_, Instruction* n) -> void
							{
								auto& store_neighbors = store_inst_to_variable_inst.GetNeighbors(n);
								if (!store_neighbors.empty())
								{
									path.emplace_back(n);
									for (auto i = 0; i < path.size(); i++)
									{
										const auto& p = path[i];
										//PrintInst(p);
									}
									//fmt::print("\n XDDDDDD \n");
									//PrintInst(n);

									load_to_store[start_n].emplace_back(path);

									if (0)
									{
										fmt::print("\n XDDDDDD \n");
										for (auto i = path.size() - 1; i > 0; i--)
										{
											const auto& p = path[i];
											PrintInst(p);
										}
										fmt::print("\n XDDDDDD \n");
									}

									path.pop_back();
									return;
								}
								path.emplace_back(n);

								for (auto& neighbor : graph.GetNeighbors(n))
								{
									if (std::find(std::begin(path), std::end(path), neighbor) != std::end(path))
									{
										continue;
									}
									DFSRecursion_(DFSRecursion_, neighbor);
								}
								path.pop_back();
							};

							for (auto& [variable_n, neighbors] : variable_inst_to_access_or_load_inst.GetNodeToNeighbors())
							{
								for (auto& n : neighbors)
								{
									start_n = n;
									DFSRecursion(DFSRecursion, n);
								}
							}
						}
					}

					if (0)
					{
						fmt::print("!!!!!!!!!!!\n");
						for (auto& [n, pss] : load_to_store)
						{
							fmt::print("1 \t{}\n", InstToString(n));
							//for (auto& ps : pss)
							auto& ps = pss[0];
							{
								fmt::print("\n");
								for (auto& p : ps)
								{
									fmt::print("2 \t\t{}\n", InstToString(p));
								}
								fmt::print("\n");
							}
						}
						fmt::print("!!!!!!!!!!!\n");
					}

					load_to_store = load_or_access_to_store_paths;

					/*
					 * Merge paths until we get from start node to end node
					 * X -> P1 + P2 + P3 = Y
					 */
					Instruction* start_node = gl_Position_access_instruction;
					Instruction* end_node = position_type_inst;
					//std::swap(start_node, end_node);

					flat_hash_map<Instruction*, PositionAffectedInstruction, InstructionPointerHasher, InstructionPointerCompare> instructions_influencing_gl_Position;
					phmap::flat_hash_set<Instruction*> visited_instructions;


					// OpStore %376 %374
					//         ^
					// %376 is gl_Position
					// %374 is the value to store into gl_Position
					//instructions_influencing_gl_Position[start_node] = PositionAffectedInstruction{ end_node->GetSingleWordInOperand(0) };

					// %72 = OpLoad %v4float %POSITION
					//instructions_influencing_gl_Position[end_node] = PositionAffectedInstruction{ start_node->result_id() };

					// Find all paths from %position -> %gl_Position
					// NP-hard of course (exponential), but hopefully small enough graph
					if (1)
					{
						flat_hash_map<Instruction*, std::vector<std::vector<Instruction*>>, InstructionPointerHasher, InstructionPointerCompare> path;
						phmap::flat_hash_set<Instruction*, InstructionPointerHasher, InstructionPointerCompare> store_insts_that_lead_to_gl_Position;

						auto DFSRecursion = [&](auto&& DFSRecursion_, Instruction* var) -> void
						{
							auto AddAllPaths = [&]()
							{
								for (auto& [store_inst, ps] : path)
								{
									for (auto& pp : ps)
									{
										for (auto& n : pp)
										{
											// add in variable inst if defined
											/*
											for (auto& var_inst : access_or_load_inst_to_variable_inst.GetNeighbors(n))
											{
												instructions_influencing_gl_Position[var_inst] = PositionAffectedInstruction{ var_inst->result_id() };
											}
											*/
											for (auto& var_inst : store_inst_to_variable_inst.GetNeighbors(n))
											{
												instructions_influencing_gl_Position[var_inst] = PositionAffectedInstruction{ var_inst->result_id() };
											}

											if (n->result_id())
											{
												instructions_influencing_gl_Position[n] = PositionAffectedInstruction{ n->result_id() };
											}
											else
											{
												instructions_influencing_gl_Position[n] = PositionAffectedInstruction{ n->GetSingleWordInOperand(0) };
											}

											// cache that this path already gets to the end, so if another path goes into this path, then assume
											// it already reaches the end
											if (n->opcode() == SpvOpStore)
											{
												store_insts_that_lead_to_gl_Position.emplace(n);
											}
										}
									}
								}
							};

							if (*var == *gl_Position_var_inst)
							{
								AddAllPaths();
								return;
							}

							//fmt::print("Var Inst {}\n", InstToString(var));
							for (auto& access_or_load_inst : variable_inst_to_access_or_load_inst.GetNeighbors(var))
							{
								//fmt::print("Associated load inst {}\n", InstToString(access_or_load_inst));
								if (auto found = load_to_store.find(access_or_load_inst); found != std::end(load_to_store))
								{
									auto& load_to_store_paths = found->second;
									for (auto& load_to_store_path : load_to_store_paths)
									{
										if (!load_to_store_path.empty())
										{
											Instruction* store_inst = load_to_store_path.back();
											//fmt::print("Inst {:08X} {}\n", (uintptr_t)store_inst, InstToString(store_inst));
											//fmt::print("Found! {}\n", load_to_store_path.size());
											//continue;
											if (store_inst->opcode() == SpvOpStore)
											{
												auto already_iterated = path.contains(store_inst); //|| visited_instructions.contains(access_or_load_inst);

												//visited_instructions.emplace(access_or_load_inst);

												path[store_inst].emplace_back(load_to_store_path);

												// optimization (we already know that this path reaches the end)
												if (store_insts_that_lead_to_gl_Position.contains(store_inst))
												{
													AddAllPaths();
													already_iterated = true;
												}

												if (!already_iterated)
												{
													for (auto& var_inst : store_inst_to_variable_inst.GetNeighbors(store_inst))
													{
														DFSRecursion_(DFSRecursion_, var_inst);
														/*
														fmt::print("Var Inst {}\n", InstToString(var_inst));
														for (auto& load_inst : variable_inst_to_access_or_load_inst.GetNeighbors(var_inst))
														{
															DFSRecursion_(DFSRecursion_, load_inst);
														}
														*/
													}
												}

												auto& p = path[store_inst];
												p.pop_back();
												if (p.empty())
												{
													path.erase(store_inst);
												}
											}
											else
											{
												PANIC("Expected store!");
											}
										}
									}
								}
							}
						};


						//fmt::print("{}\n", InstToString(gl_Position_var_inst));
						/*
						fmt::print("{}\n", InstToString(end_node));
						auto& start_node_variables = variable_inst_to_access_or_load_inst.GetNeighbors(end_node);
						if (start_node_variables.empty())
						{
							PANIC("Should be variable defined for start node");
						}
						*/
						for (auto& [var_inst, _] : variable_inst_to_access_or_load_inst.GetNodeToNeighbors())
						{
							DFSRecursion(DFSRecursion, end_node);
						}
						//DFSRecursion(DFSRecursion, end_node);
					}

					// BFS
					if (0)
					{
						flat_hash_map<Instruction*, std::vector<std::vector<Instruction*>>, InstructionPointerHasher, InstructionPointerCompare> path;

						std::deque<Instruction*> instructions_left;
						instructions_left.push_back(end_node);
						while (!instructions_left.empty())
						{
							Instruction* var = instructions_left.front();
							instructions_left.pop_front();

							visited_instructions.emplace(var);

							if (*var == *gl_Position_var_inst)
							{
								for (auto& [_, ps] : path)
								{
									for (auto& pp : ps)
									{
										for (auto& n : pp)
										{
											if (n->result_id())
											{
												instructions_influencing_gl_Position[n] = PositionAffectedInstruction{ n->result_id() };
											}
											else
											{
												instructions_influencing_gl_Position[n] = PositionAffectedInstruction{ n->GetSingleWordInOperand(0) };
											}
										}
									}
								}
								return;
							}

							//fmt::print("Var Inst {}\n", InstToString(var));
							for (auto& access_or_load_inst : variable_inst_to_access_or_load_inst.GetNeighbors(var))
							{
								//fmt::print("Associated load inst {}\n", InstToString(access_or_load_inst));
								if (auto found = load_to_store.find(access_or_load_inst); found != std::end(load_to_store))
								{
									auto& load_to_store_paths = found->second;
									for (auto& load_to_store_path : load_to_store_paths)
									{
										if (!load_to_store_path.empty())
										{
											Instruction* store_inst = load_to_store_path.back();
											//fmt::print("Inst {:08X} {}\n", (uintptr_t)store_inst, InstToString(store_inst));
											//fmt::print("Found! {}\n", load_to_store_path.size());
											//continue;
											if (store_inst->opcode() == SpvOpStore)
											{
												auto already_iterated = path.contains(var);
												path[var].emplace_back(load_to_store_path);

												if (!already_iterated)
												{
													for (auto& var_inst : store_inst_to_variable_inst.GetNeighbors(store_inst))
													{
														if (!visited_instructions.contains(var_inst))
														{
															instructions_left.emplace_back(var_inst);
															//fmt::print("Var Inst {}\n", InstToString(var_inst));
														}
													}
												}

												auto& p = path[var];
												p.pop_back();
												if (p.empty())
												{
													path.erase(store_inst);
												}
											}
											else
											{
												PANIC("Expected store!");
											}
										}
									}
								}
							}
						}
					}

					if (0)
					{
						fmt::print("????\n");
						PrintInst(start_node);
						PrintInst(end_node);

						fmt::print("????\n");
						std::map<Instruction*, phmap::flat_hash_set<Instruction*>> neighbors(std::begin(graph.GetNodeToNeighbors()), std::end(graph.GetNodeToNeighbors()));
						for (auto& neighbor : graph.GetNeighbors(start_node))
						{
							PrintInst(neighbor);
						}
						fmt::print("\n");
						fmt::print("=======\n");

						/*
						for (auto& [n, neighbors] : neighbors)
						{
							fmt::print("{}\n", InstToString(n));
						}
						fmt::print("=======\n");

						for (auto& [n, neighbors] : neighbors)
						{
							fmt::print("=======\n");
							fmt::print("{}\n", InstToString(n));
							fmt::print("|||||||\n");
							for (auto& neighbor : neighbors)
							{
								PrintInst(neighbor);
							}
							fmt::print("||||||\n");
						}
						*/
						fmt::print("????\n");
						for (auto& [n, id] : std::map<Instruction*, PositionAffectedInstruction>(std::begin(instructions_influencing_gl_Position), std::end(instructions_influencing_gl_Position)))
						{
							fmt::print("{} -> {}\n", InstToString(n), id.position_affected_id);
						}
						fmt::print("????\n");
					}

					std::map<uint32_t, Instruction*> word_offset_to_inst;
					for (auto& [inst, position_affected] : instructions_influencing_gl_Position)
					{
						if (auto found = main_instruction_to_word_offset.find(inst); found != std::end(main_instruction_to_word_offset))
						{
							auto index = found->second;
							word_offset_to_inst[index] = inst;

							position_affected.debug_line = std::string(inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));
							create_message(SPV_MSG_INFO, AFFECT_POSITION_ID_MESSAGE, { index, 0, 0 }, (const char*)&position_affected);
						}
						else
						{
							//PANIC("Not found");
						}
					}

				}

				std::vector<Instruction*> main_local_variable_insts_def_uses{};

				if (1)
				{
					// %POSITION = OpVariable %_ptr_Input_v4float Input
					Instruction* begin_inst = position_type_inst;
					Instruction* end_inst = start_inst;

					phmap::flat_hash_set<Instruction*> unvisited_instructions{ begin_inst };
					phmap::flat_hash_set<Instruction*> visited_instructions;

					while (!unvisited_instructions.empty())
					{
						Instruction* inst = *std::begin(unvisited_instructions);
						unvisited_instructions.erase(inst);

						if (visited_instructions.contains(inst))
						{
							continue;
						}
						else
						{
							visited_instructions.emplace(inst);
						}

						if (0 && InstToString(inst).find("%76 = OpLoad %mat4v4float %projecti") != std::string::npos)
						{
							fmt::print("XDDD\n");
						}

						if (IsOpcodeSkippable(inst->opcode()))
						{
							continue;
						}

						const auto start_operand_index = inst->type_id() > 0 ? 1 : 0 + inst->result_id() > 0 ? 1 : 0;
						for (auto i = start_operand_index; i < inst->NumOperands(); i++)
						{
							const Operand& operand = inst->GetOperand(i);
							const auto& words = operand.words;
							if (words.size() > 1)
							{
								continue;
							}
							const auto& word = words[0];

							// Ignore operands defining types
							if (
								operand.type != SPV_OPERAND_TYPE_ID &&
								operand.type != SPV_OPERAND_TYPE_RESULT_ID &&
								operand.type != SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID &&
								operand.type != SPV_OPERAND_TYPE_SCOPE_ID
								)
							{
								continue;
							}

							// if def isn't the same as the current instruction (the instruction is declared somewhere else)
							Instruction* def_inst = get_def_use_mgr()->GetDef(word);
							if (def_inst)
							{
								if (inst != def_inst)
								{
									auto& neighbors = graph.GetNeighbors(inst);
									// Add it as a neighbor
									if (!neighbors.contains(def_inst))
									{
										//fmt::print("Neighbor def inst{} -> {}\n", InstToString(inst), InstToString(def_inst));
										graph.AddNeighbor(inst, def_inst);
										unvisited_instructions.emplace(def_inst);
									}
								}
							}

							get_def_use_mgr()->WhileEachUser(inst, [&](Instruction* ref_inst)
								{
									if (ref_inst == inst)
									{
										return true;
									}
									unvisited_instructions.emplace(ref_inst);
									
									Instruction* parent_inst = inst;
									if (parent_inst->opcode() == SpvOpAccessChain || parent_inst->opcode() == SpvOpVariable)
									{
										std::swap(parent_inst, ref_inst);
									}
									graph.AddNeighbor(ref_inst, parent_inst);
									//fmt::print("Neighbor user {} -> {}\n", InstToString(ref_inst), InstToString(parent_inst));

									return true;
								});
						}
					}

					if (0)
					{
						fmt::print("!!!\n");

						for (auto& n : graph.GetNeighbors(end_inst))
						{
							fmt::print(" 111 {} -> {}\n", InstToString(end_inst), InstToString(n));
							for (auto& n2 : graph.GetNeighbors(n))
							{
								fmt::print(" 222 {} -> {}\n", InstToString(n), InstToString(n2));
							}
						}
						fmt::print("!!!\n");
					}

					if(1)
					{
						Instruction* start_node = gl_Position_access_instruction;
						Instruction* end_node = position_type_inst;
						//std::swap(start_node, end_node);

						flat_hash_map<Instruction*, PositionAffectedInstruction> instructions_influencing_gl_Position;

						// OpStore %376 %374
						//         ^
						// %376 is gl_Position
						// %374 is the value to store into gl_Position
						instructions_influencing_gl_Position[start_node] = PositionAffectedInstruction{ end_node->GetSingleWordInOperand(0) };

						// %72 = OpLoad %v4float %POSITION
						instructions_influencing_gl_Position[end_node] = PositionAffectedInstruction{ start_node->result_id() };
						// DFS to find path from root to position
						std::vector<Instruction*> path;
						auto DFSRecursion = [&](auto&& DFSRecursion_, Instruction* n) -> void
						{
							if (n == end_node)
							{
								for (auto i = 0; i < path.size(); i++)
								{
									const auto& p = path[i];
									if (p->result_id())
									{
										instructions_influencing_gl_Position[p] = PositionAffectedInstruction{ p->result_id() };
									}
									else
									{
										instructions_influencing_gl_Position[p] = PositionAffectedInstruction{ p->GetSingleWordInOperand(0) };
									}

									if (0)// && p->GetSingleWordInOperand(0) == 13) //&& p->result_id() == 274)
									{
										fmt::print("\n XDDDDDD \n");
										//PrintInst(start_node);
										//PrintInst(end_node);
										for (auto i = path.size() - 1; i > 0; i--)
										{
											const auto& p = path[i];
											PrintInst(p);
										}
										PrintInst(p);
										/*
										for (auto i = 0; i < path.size(); i++)
										{
											const auto& p = path[i];
											PrintInst(p);
										}
										*/
										/*
										fmt::print("\n XDDDDDD \n");
										for (auto& neighbor : graph.GetNeighbors(p))
										{
											PrintInst(neighbor);
										}
										*/

										fmt::print("\n XDDDDDD \n");
									}
								}

								/*
								fmt::print("\n XDDDDDD \n");
								for (auto i = path.size() - 1; i > 0; i--)
								{
									const auto& p = path[i];
									PrintInst(p);
								}
								fmt::print("\n XDDDDDD \n");
								*/
								return;
							}
							path.emplace_back(n);

							for (auto& neighbor : graph.GetNeighbors(n))
							{
								if (std::find(std::begin(path), std::end(path), neighbor) != std::end(path))
								{
									continue;
								}
								DFSRecursion_(DFSRecursion_, neighbor);
							}
							path.pop_back();
						};
						DFSRecursion(DFSRecursion, start_node);
						
						if (0)
						{
							fmt::print("????\n");
							PrintInst(start_node);
							PrintInst(end_node);

							fmt::print("????\n");
							std::map<Instruction*, phmap::flat_hash_set<Instruction*>> neighbors(std::begin(graph.GetNodeToNeighbors()), std::end(graph.GetNodeToNeighbors()));
							for (auto& neighbor : graph.GetNeighbors(start_node))
							{
								PrintInst(neighbor);
							}
							fmt::print("\n");
							fmt::print("=======\n");

							for (auto& [n, neighbors] : neighbors)
							{
								fmt::print("{}\n", InstToString(n));
							}
							fmt::print("=======\n");

							for (auto& [n, neighbors] : neighbors)
							{
								fmt::print("=======\n");
								fmt::print("{}\n", InstToString(n));
								fmt::print("|||||||\n");
								for (auto& neighbor : neighbors)
								{
									PrintInst(neighbor);
								}
								fmt::print("||||||\n");
							}

							fmt::print("????\n");
							for (auto& [n, id] : std::map<Instruction*, PositionAffectedInstruction>(std::begin(instructions_influencing_gl_Position), std::end(instructions_influencing_gl_Position)))
							{
								fmt::print("{} -> {}\n", InstToString(n), id.position_affected_id);
							}
							fmt::print("????\n");
						}


						IdConstructionTree id_construction_tree;
						get_module()->ForEachInst([&](Instruction* inst) {
							const auto start_operand_index = inst->type_id() > 0 ? 1 : 0 + inst->result_id() > 0 ? 1 : 0;

							auto result_id = inst->result_id();
							std::vector<uint32_t> operands;
							auto opcode = inst->opcode();

							for (auto i = 0; i < inst->NumInOperands(); i++)
							{
								const Operand& operand = inst->GetInOperand(i);
								const auto& words = operand.words;
								if (words.size() > 1)
								{
									continue;
								}
								const auto& word = words[0];

								operands.emplace_back(word);
							}

							IdConstructionNode node{ result_id, operands, opcode };
							id_construction_tree.result_to_node.emplace(result_id, node);
						});

						std::map<uint32_t, Instruction*> word_offset_to_inst;
						for (auto& [inst, position_affected] : instructions_influencing_gl_Position)
						{
							if (auto found = main_instruction_to_word_offset.find(inst); found != std::end(main_instruction_to_word_offset))
							{
								if (0)
								{
									PrintInst(inst);
								}

								auto index = found->second;
								word_offset_to_inst[index] = inst;

								position_affected.debug_line = std::string(inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));
								create_message(SPV_MSG_INFO, AFFECT_POSITION_ID_MESSAGE, { index, 0, 0 }, (const char*)&position_affected);
							}
							else
							{
								//PANIC("Not found");
							}

							create_message(SPV_MSG_INFO, ID_CONSTRUCTION_ID_MESSAGE, { 0, 0, 0 }, (const char*)&id_construction_tree);
						}
					}
				}

				// This setups the graph
				// Starts a graph that goes from the start node that has a path every other node
				if(0)
				{
					phmap::flat_hash_set<Instruction*> seen_instructions;
					phmap::flat_hash_set<Instruction*> visited_instructions;
					phmap::flat_hash_set<Instruction*> user_visited_instructions;
					while (!instructions_to_visit.empty() || !main_local_variable_insts_def_uses.empty())
					{
						Instruction* inst{};
						
						if (!instructions_to_visit.empty())
						{
							inst = instructions_to_visit.front();
							instructions_to_visit.pop();
						}
						else if (!main_local_variable_insts_def_uses.empty())
						{
							inst = main_local_variable_insts_def_uses.back();
							main_local_variable_insts_def_uses.erase(std::end(main_local_variable_insts_def_uses) - 1);
							if (visited_instructions.contains(inst))
							{
								continue;
							}
						}
						visited_instructions.emplace(inst);

						if (0)
						{
							auto response = inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
							if (response.find("%191 = OpAccessChain %_ptr_Functio") != std::string::npos)
							{
								fmt::print("{}\n", response);
							}
							if (response.find("%188 = OpAccessChain %_ptr_Functi") != std::string::npos)
							{
								fmt::print("{}\n", response);
							}
						}

						if (IsOpcodeSkippable(inst->opcode()))
						{
							continue;
						}

						if (0)
						{
							fmt::print("NEW INST\n");
							PrintInst(inst);
							fmt::print("\n");
						}

						const auto start_operand_index = inst->type_id() > 0 ? 1 : 0 + inst->result_id() > 0 ? 1 : 0;
						for (auto i = start_operand_index; i < inst->NumOperands(); i++)
						{
							const Operand& operand = inst->GetOperand(i);
							const auto& words = operand.words;
							if (words.size() > 1)
							{
								continue;
							}
							const auto& word = words[0];

							// Ignore operands defining types
							if (
								operand.type != SPV_OPERAND_TYPE_ID &&
								operand.type != SPV_OPERAND_TYPE_RESULT_ID &&
								operand.type != SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID &&
								operand.type != SPV_OPERAND_TYPE_SCOPE_ID &&
								1
								)
							{
								continue;
							}

							// Get the declaration of the word
							Instruction* def_inst = get_def_use_mgr()->GetDef(word);
							if (!def_inst)
							{
								continue;
							}

							// Ignore definition instructions that are names, defining types, etc...
							auto def_inst_opcode = def_inst->opcode();
							if (IsOpcodeSkippable(def_inst_opcode))
							{
								continue;
							}

							auto& neighbors = graph.GetNeighbors(inst);
							// if def isn't the same as the current instruction (the instruction is declared somewhere else)
							if (inst != def_inst)
							{
								// Add it as a neighbor
								if (!neighbors.contains(def_inst))
								{
									if (!visited_instructions.contains(def_inst) && !seen_instructions.contains(def_inst))
									{
										instructions_to_visit.emplace(def_inst);
										seen_instructions.emplace(def_inst);
									}
									graph.AddNeighbor(inst, def_inst);
								}
							}

							// Iterate until we reach
							if (0)
							{
								fmt::print("DEF INST\n");
								PrintInst(def_inst);
								fmt::print("===========\n");
							}

							bool a = user_visited_instructions.contains(def_inst);
							bool b = inst == def_inst;
							bool c = visited_instructions.contains(def_inst);
							if (visited_instructions.contains(def_inst)
								&& !(!user_visited_instructions.contains(def_inst) && inst == def_inst))
							{
								continue;
							}
							else
							{
								user_visited_instructions.emplace(def_inst);
							}

							/*
							 * %o = OpVariable %_ptr_Function_VS_OUTPUT Function
							 * users would be...
							 * REF INST ||         %146 = OpAccessChain %_ptr_Function_v4float %o %int_0
							 * REF INST ||         %250 = OpAccessChain %_ptr_Function_v4float %o %int_1
							 * REF INST ||         %339 = OpAccessChain %_ptr_Function_v4float %o %int_2
							 */
							get_def_use_mgr()->WhileEachUser(def_inst, [&](Instruction* ref_inst)
							{
								if (0)
								{
									fmt::print("REF INST || ");
									PrintInst(ref_inst);
								}

								// Iterate until we reach our node
								if (ref_inst == inst)
								{
									return true;
								}

								// Ignore instructions that come after instruction usage
								// Except if it is a store...
								/*
								 * Example!
								 * 
								 * %145 = OpCompositeConstruct %v4float %132 %136 %140 %144 ; 0x00003db4
                                 * %146 = OpAccessChain %_ptr_Function_v4float %o %int_0 ; 0x00003dd0
                                 * OpStore %146 %145 ; 0x00003de4
								 * 
								 * Here, at %146, the OpStore isn't going to be stored when clearly it is affecting %146!
								 */
								if (instruction_to_index[ref_inst] > instruction_to_index[inst])
								{
									if (!graph.GetNeighbors(def_inst).contains(ref_inst) && def_inst->opcode() == SpvOpAccessChain && ref_inst->opcode() == SpvOpStore)
									{

									}
									else
									{
										return true;
									}
								};

								//PrintInst(ref_inst);

								if (!visited_instructions.contains(ref_inst) && !seen_instructions.contains(ref_inst))
								{
									if (0)
									{
										auto response = ref_inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
										if (response.find("%71 = OpLoad %v4floa") != std::string::npos)
										{
											fmt::print("{}\n", response);
										}
									}

									instructions_to_visit.emplace(ref_inst);
									seen_instructions.emplace(ref_inst);
								}
								graph.AddNeighbor(inst, ref_inst);

								return true;
							});
							//fmt::print("==========\n");
						};

						/*
						for (auto i = 0; i < inst->NumInOperandWords(); i++)
						{
							inst->Dump();
							//const Operand& operand = inst->GetOperand(inst->TypeResultIdCount() + i);
							auto word = inst->GetSingleWordInOperand(i);
							Instruction* type_inst = get_def_use_mgr()->GetDef(word);
							auto result_id = type_inst->result_id();

							PrintInst(type_inst);
							fmt::print("\n");
							get_def_use_mgr()->WhileEachUser(type_inst, [&](Instruction* ref_inst)
							{
								// Iterate until we reach our node
								if (ref_inst == inst)
								{
									return false;
								}

								PrintInst(ref_inst);

								if (!visited_instructions.contains(ref_inst))
								{
									instructions_to_visit.emplace(ref_inst);
									graph.AddNeighbor(inst, ref_inst);
								}

								return true;
							});
							fmt::print("\n");
						}
						*/
					}
				}
				if(0)
				{
					Instruction* start_node = gl_Position_access_instruction;
					Instruction* end_node = position_type_inst;

					flat_hash_map<Instruction*, PositionAffectedInstruction> instructions_influencing_gl_Position;

					// OpStore %376 %374
					//         ^
					// %376 is gl_Position
					// %374 is the value to store into gl_Position
					instructions_influencing_gl_Position[start_node] = PositionAffectedInstruction{ end_node->GetSingleWordInOperand(0) };

					// %72 = OpLoad %v4float %POSITION
					instructions_influencing_gl_Position[end_node] = PositionAffectedInstruction{ start_node->result_id() };
					// DFS to find path from root to position
					if (1)
					{
						std::vector<Instruction*> path;
						auto DFSRecursion = [&](auto&& DFSRecursion_, Instruction* n) -> void
						{
							if (n == end_node)
							{
								for (auto i = 0; i < path.size(); i++)
								{
									const auto& p = path[i];
									if (p->result_id())
									{
										instructions_influencing_gl_Position[p] = PositionAffectedInstruction{ p->result_id() };
									}
									else
									{
										instructions_influencing_gl_Position[p] = PositionAffectedInstruction{ p->GetSingleWordInOperand(0) };
									}

									if (0)// && p->GetSingleWordInOperand(0) == 13) //&& p->result_id() == 274)
									{
										fmt::print("\n XDDDDDD \n");
										PrintInst(start_node);
										PrintInst(end_node);
										for (auto i = path.size() - 1; i > 0; i--)
										{
											const auto& p = path[i];
											PrintInst(p);
										}
										fmt::print("\n XDDDDDD \n");
										for (auto& neighbor : graph.GetNeighbors(p))
										{
											PrintInst(neighbor);
										}

										fmt::print("\n XDDDDDD \n");
									}
								}
								
								/*
								fmt::print("\n XDDDDDD \n");
								for (auto i = path.size() - 1; i > 0; i--)
								{
									const auto& p = path[i];
									PrintInst(p);
								}
								fmt::print("\n XDDDDDD \n");
								*/
								return;
							}
							path.emplace_back(n);

							for (auto& neighbor : graph.GetNeighbors(n))
							{
								if (std::find(std::begin(path), std::end(path), neighbor) != std::end(path))
								{
									continue;
								}
								DFSRecursion_(DFSRecursion_, neighbor);
							}
							path.pop_back();
						};
						DFSRecursion(DFSRecursion, start_node);
					}

					if (0)
					{
						fmt::print("????\n");
						PrintInst(start_node);
						PrintInst(end_node);

						fmt::print("????\n");
						std::map<Instruction*, phmap::flat_hash_set<Instruction*>> neighbors(std::begin(graph.GetNodeToNeighbors()), std::end(graph.GetNodeToNeighbors()));
						for (auto& neighbor : graph.GetNeighbors(start_node))
						{
							PrintInst(neighbor);
						}
						fmt::print("\n");
						fmt::print("=======\n");

						for (auto& [n, neighbors] : neighbors)
						{
							fmt::print("{}\n", InstToString(n));
						}
						fmt::print("=======\n");

						for (auto& [n, neighbors] : neighbors)
						{
							fmt::print("=======\n");
							fmt::print("{}\n", InstToString(n));
							fmt::print("|||||||\n");
							for (auto& neighbor : neighbors)
							{
								PrintInst(neighbor);
							}
							fmt::print("||||||\n");
						}

						fmt::print("????\n");
						for (auto& [n, id] : std::map<Instruction*, PositionAffectedInstruction>(std::begin(instructions_influencing_gl_Position), std::end(instructions_influencing_gl_Position)))
						{
							fmt::print("{} -> {}\n", InstToString(n), id.position_affected_id);
						}
						fmt::print("????\n");
					}

					/*
					 * Make another pass that accounts for stores to variables
					 *
					 * %510 = OpAccessChain %_ptr_Function_v4float %o %int_0 ; 0x00005cf8
	                 * %511 = OpLoad %v4float %510 ; 0x00005d0c
                     * %512 = OpAccessChain %_ptr_Output_v4float %__0 %int_0 ; 0x00005d1c
					 * OpStore %512 %511
					 * 
					 * OpStore %512 %511
					 *         ^    ^
					 * gl_Position  what final position is stored, which in this case is %o
					 * 
					 * we want to see where %o is written to
					 * 
					 */

					// OpStore %512 %511
					//         ^ gl_Position
					// start_node
					if (0 && start_node->opcode() == SpvOpStore)
					{
						Instruction* src_store_inst = get_def_use_mgr()->GetDef(start_node->GetSingleWordInOperand(1));
						// %511 = OpLoad %v4float %510 ; 0x00005d0c
						if (src_store_inst->opcode() == SpvOpLoad)
						{
							Instruction* load_inst = get_def_use_mgr()->GetDef(src_store_inst->GetSingleWordInOperand(0));
							// %510 = OpAccessChain %_ptr_Function_v4float %o %int_0 ; 0x00005cf8
							if (load_inst->opcode() == SpvOpAccessChain)
							{
								auto InstToString = [](Instruction* inst) -> std::string
								{
									auto str = std::string(inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));
									return str;
								};

								for (auto i = 0; i < load_inst->NumInOperandWords(); i++)
								{
									auto word = load_inst->GetSingleWordInOperand(i);
									fmt::print("{}\n", i);
								}

								Instruction* variable_def_inst = get_def_use_mgr()->GetDef(load_inst->GetSingleWordInOperand(0));
								get_def_use_mgr()->WhileEachUser(variable_def_inst, [&](Instruction* ref_inst)
								{
									if (ref_inst->NumInOperandWords() != load_inst->NumInOperandWords() || ref_inst->GetSingleWordInOperand(1) != load_inst->GetSingleWordInOperand(1))
									{
										return true;
									}
									fmt::print("{} -> {}\n", InstToString(variable_def_inst), InstToString(ref_inst));
									std::vector<Instruction*> path;
									auto DFSRecursion = [&](auto&& DFSRecursion_, Instruction* to_search, std::vector<Instruction*>& path, Instruction* n) -> void
									{
										if (n == to_search)
										{
											if (!std::any_of(std::begin(path), std::end(path), [&](const auto& e)
												{
													return e->opcode() == SpvOpStore;
												}
											))
											{
												return;
											}
											if (!std::any_of(std::begin(path) + 1, std::end(path), [&](const auto& e)
												{
													if (e->opcode() == SpvOpAccessChain)
													{
														if (e->NumInOperandWords() == load_inst->NumInOperandWords() && e->GetSingleWordInOperand(1) == load_inst->GetSingleWordInOperand(1))
														{
															return true;
														}
													}
													return false;
												}
											))
											{
												return;
											}

												for (auto i = 0; i < path.size() && path.size() > 1; i++)
												{
													const auto& p = path[i];
													if (p->result_id())
													{
														instructions_influencing_gl_Position[p] = PositionAffectedInstruction{ p->result_id() };
													}
													else
													{
														instructions_influencing_gl_Position[p] = PositionAffectedInstruction{ p->GetSingleWordInOperand(0) };
													}

													if (1)// && p->GetSingleWordInOperand(0) == 13) //&& p->result_id() == 274)
													{
														fmt::print("\n XDDDDDD \n");
														for (auto i = path.size() - 1; i > 0; i--)
														{
															const auto& p = path[i];
															PrintInst(p);
														}
														fmt::print("\n XDDDDDD \n");
														for (auto& neighbor : graph.GetNeighbors(p))
														{
															PrintInst(neighbor);
														}

														fmt::print("\n XDDDDDD \n");
													}
												}

												return;
										}
										path.emplace_back(n);
										for (auto& neighbor : graph.GetNeighbors(n))
										{
											if (std::find(std::begin(path), std::end(path), neighbor) != std::end(path))
											{
												continue;
											}
											DFSRecursion_(DFSRecursion_, to_search, path, neighbor);
										}
										path.pop_back();
									};

									DFSRecursion(DFSRecursion, variable_def_inst, path, ref_inst);
									return true;
								});
							}
						}
					}
					//fmt::print("UH\n");

					/*
					// BFS to find path from root to position
					std::queue<std::vector<Instruction*>> queue;
					queue.emplace(std::vector<Instruction*>{ start_node });

					while (!queue.empty())
					{
						std::vector<Instruction*> path = queue.back();
						queue.pop();

						auto* n = path.back();

						// Found it
						if (*n == *end_node)
						{
							for (const auto& p : path)
							{
								instructions_influencing_gl_Position.emplace(p);
							}
							return;
						}

						for (auto& neighbor : graph.GetNeighbors(n))
						{
							if (neighbor == end_node)
							{
								PANIC("SSS");
							}

							// Check if neighbor is not visited in the current path
							auto found = std::find_if(std::begin(path), std::end(path), [&](const auto& e) { return e == neighbor; });
							if (found != std::end(path))
							{
								//continue;
							}

							auto new_path = path;
							new_path.emplace_back(neighbor);
							queue.emplace(new_path);
						}
					}
					*/

					/*
					fmt::print("XD!!!!!!!!!!!!!!!\n\n");
					std::vector<Instruction*> aaa(std::begin(instructions_influencing_gl_Position), std::end(instructions_influencing_gl_Position));
					std::sort(std::begin(aaa), std::end(aaa), [](const auto& e1, const auto& e2) { return e1->unique_id() < e2->unique_id(); });
					for (const auto& inst : aaa)
					{
						PrintInst(inst);
					}
					fmt::print("XD\n");
					*/
					std::map<uint32_t, Instruction*> word_offset_to_inst;
					for (auto& [inst, position_affected] : instructions_influencing_gl_Position)
					{
						if (auto found = main_instruction_to_word_offset.find(inst); found != std::end(main_instruction_to_word_offset))
						{
							auto index = found->second;
							word_offset_to_inst[index] = inst;

							position_affected.debug_line = std::string(inst->PrettyPrint(SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));
							create_message(SPV_MSG_INFO, AFFECT_POSITION_ID_MESSAGE, { index, 0, 0 }, (const char*)&position_affected);
						}
						else
						{
							//PANIC("Not found");
						}
					}

					auto GetValueFromConstId = [&](uint32_t id) -> uint32_t
					{
						auto* const_mgr = context()->get_constant_mgr();
						const analysis::IntConstant* member_idx =
							const_mgr->FindDeclaredConstant(id)
							->AsIntConstant();
						if (!member_idx)
						{
							PANIC("Not null");
						}
						if (member_idx->type()->AsInteger()->width() == 32)
						{
							return member_idx->GetU32();
						}
						else
						{
							return member_idx->GetU64();
						}
					};

					struct PositionView
					{
						Instruction* inst;
						std::vector<uint32_t> position_indices;
					};

					flat_hash_map<uint32_t, PositionView> id_to_position_view;
					id_to_position_view[position_id] = PositionView{ nullptr, {0, 1, 2, 3} };

					start_node;
					end_node;

					if (0)
					{
						for (const auto& [word_offset, inst] : word_offset_to_inst)
						{
							PrintInst(inst);

							auto opcode = inst->opcode();
							if (opcode == SpvOpAccessChain)
							{
								//%83 = OpAccessChain %_ptr_Function_float %_631 %uint_0 ; 0x000038d8
								//...
								//%84 = OpLoad %float %83 ; 0x000038ec
								//%93 = OpCompositeConstruct %v4float %84 %87 %90 %92 ; 0x00003968
								//OpStore %_633 %93 ; 0x00003984
								//vec4 _633 = vec4(_631.x, _631.y, _631.z, POSITION.w);
								const auto result_id = inst->result_id();
								const auto base_id = inst->GetSingleWordInOperand(0);
								if (inst->NumInOperands() != 2)
								{
									PANIC("Access should only be one deference");
								}
								const auto memory_access_id = inst->GetSingleWordInOperand(1);
								const auto access_index = GetValueFromConstId(memory_access_id);

								if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
								{
									PositionView& ref_position_view = found->second;
									if (ref_position_view.position_indices.size() != 4)
									{
										PANIC("Should be entire POSITION? (float 4)");
									}
									id_to_position_view[result_id] = PositionView{ inst, {access_index} };
								}
							}
							else if (opcode == SpvOpVectorShuffle)
							{
								//%73 = OpVectorShuffle % v3float % 72 % 72 0 1 2; 0x00003808
								//vec4 _633 = vec4(_631.x, _631.y, _631.z, POSITION.w);

								const auto result_id = inst->result_id();
								const auto base_id = inst->GetSingleWordInOperand(0);
								const auto base_id2 = inst->GetSingleWordInOperand(1);
								if (base_id != base_id2)
								{
									PANIC("Assume shuffle same vec");
								}
								std::vector<uint32_t> shuffle_indices;
								auto remaining_operands = inst->NumInOperands();
								for (auto i = 2; i < remaining_operands; i++)
								{
									shuffle_indices.emplace_back(inst->GetSingleWordInOperand(i));
								}
								if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
								{
									PositionView& ref_position_view = found->second;
									id_to_position_view[result_id] = PositionView{ inst, shuffle_indices };
								}
							}
							else if (opcode == SpvOpLoad)
							{
								//%72 = OpLoad %v4float %POSITION ; 0x000037f8
								const auto result_id = inst->result_id();
								const auto base_id = inst->GetSingleWordInOperand(0);
								if (inst->NumInOperands() != 1)
								{
									PANIC("Load should only be one deference");
								}
								if (auto found = id_to_position_view.find(base_id); found != std::end(id_to_position_view))
								{
									PositionView& ref_position_view = found->second;
									id_to_position_view[result_id] = PositionView{ inst, ref_position_view.position_indices };
								}
							}
							else if (opcode == SpvOpDot)
							{
							}
							else
							{
								const auto result_id = inst->result_id();
								if (result_id)
								{
									for (auto i = 0; i < inst->NumInOperands(); i++)
									{
										const auto id = inst->GetSingleWordInOperand(0);
										if (auto found = id_to_position_view.find(id); found != std::end(id_to_position_view))
										{
											PositionView& ref_position_view = found->second;
											id_to_position_view[result_id] = PositionView{ inst, ref_position_view.position_indices };
										}
									}
								}
							}
							/*
							const auto start_operand_index = inst->type_id() > 0 ? 1 : 0 + inst->result_id() > 0 ? 1 : 0;
							for (auto i = start_operand_index; i < inst->NumOperands(); i++)
							{
								const Operand& operand = inst->GetOperand(i);
								const auto& words = operand.words;
								if (words.size() > 1)
								{
									continue;
								}
								const auto& word = words[0];
								if (auto found = id_to_position_view.find(word); found != std::end(id_to_position_view))
								{
									PositionView& ref_position_view = found->second;
									if (opcode == SpvOpAccessChain)
									{

									}
								}


								std::cout << word << "        ";
								std::cout << "\n";
								std::cout << "";
							}
							*/
						}
					}
				}
			}
		};

		SPIRVAnalyzer::SPIRVAnalyzer(std::vector<uint32_t> spv_) :
			spv(std::move(spv_))
		{
		}

		Error SPIRVAnalyzer::Run()
		{
			spvtools::Optimizer opt(spv_target_env::SPV_ENV_UNIVERSAL_1_5);
			//opt.RegisterPerformancePasses();

			auto mvp_pass = std::make_unique<MVPAnalysisPass>(spv);
			auto pass_token = spvtools::Optimizer::PassToken(std::move(mvp_pass));
			opt.RegisterPass(std::move(pass_token));
			auto mvp_passname = opt.GetPassNames()[0];

			opt.SetMessageConsumer([&](spv_message_level_t level, const char* source,
									   const spv_position_t& position, const char* message)
			{
				if (level == SPV_MSG_INFO)
				{
					if (source == AFFECT_POSITION_ID_MESSAGE)
					{
						position_affected_lines.try_emplace(position.line, *(PositionAffectedInstruction*)message);
					}
					else if (source == POSITION_ID_MESSAGE)
					{
						position_id = position.line;
					}
					else if (source == ID_CONSTRUCTION_ID_MESSAGE)
					{
						id_construction_tree = *(IdConstructionTree*)message;
					}
				}
			});

			spvtools::OptimizerOptions opt_options{};
			opt_options.set_run_validator(false);
			if (!opt.Run(spv.data(), spv.size(), &spv_optimized, opt_options))
			{
				return StringError("Failed to run optimizer");
			}

			if (position_id == 0)
			{
				return StringError("Failed to find position id");
			}

			return NoError();
		}

	}
}
