#include "shader_converter.h"

#include <spirv_reflect.c>

namespace Raysterizer
{
	namespace MiddleWare
	{
		namespace
		{
			using namespace std::literals;

			struct InVariable
			{
				std::string_view type;
				std::string_view name;
			};

			static constexpr auto in_pattern = ctll::fixed_string{ R"((?:(?:[\w()]+\s+)|\s+)*in\s+(.*)\s+(.*);)" };

			constexpr auto MatchInPattern(std::string_view sv) noexcept
			{
				return ctre::match<in_pattern>(sv);
			}
			
			constexpr std::optional<InVariable> GetInVariable(std::string_view sv) noexcept
			{
				if (auto m = MatchInPattern(sv))
				{
					auto type = m.get<1>().to_view();
					auto name = m.get<2>().to_view();
					return InVariable{ type, name };
				}
				else
				{
					return std::nullopt;
				}
			}

			static_assert(GetInVariable(std::string_view("in float3 rawtex0;"))->name == std::string_view("rawtex0"));
			static_assert(GetInVariable(std::string_view("  in float3 rawtex0;"))->name == std::string_view("rawtex0"));
			static_assert(GetInVariable(std::string_view("ATTRIBUTE_LOCATION(8) in float3 rawtex0;"))->name == std::string_view("rawtex0"));

			struct OutVariable
			{
				std::string_view type;
				std::string_view name;
			};

			static constexpr auto out_pattern = ctll::fixed_string{ R"((?:(?:[\w()]+\s+)+|\s*)*out\s+(.*)\s+(.*);)" };

			constexpr auto MatchOutPattern(std::string_view sv) noexcept
			{
				return ctre::match<out_pattern>(sv);
			}

			constexpr std::optional<OutVariable> GetOutVariable(std::string_view sv) noexcept
			{
				if (auto m = MatchOutPattern(sv))
				{
					auto type = m.get<1>().to_view();
					auto name = m.get<2>().to_view();
					return OutVariable{ type, name };
				}
				else
				{
					return std::nullopt;
				}
			}

			struct UniformVariable
			{
				std::string_view type;
				std::string_view name;
			};

			static constexpr auto uniform_pattern = ctll::fixed_string{ R"([ ]*uniform[ ]*(.*)[ ]*(.*);)" };

			constexpr auto MatchUniformPattern(std::string_view sv) noexcept
			{
				return ctre::match<uniform_pattern>(sv);
			}

			constexpr std::optional<UniformVariable> GetUniformVariable(std::string_view sv) noexcept
			{
				if (auto m = MatchUniformPattern(sv))
				{
					auto type = m.get<1>().to_view();
					auto name = m.get<2>().to_view();
					return UniformVariable{ type, name };
				}
				else
				{
					return std::nullopt;
				}
			}

			static constexpr auto gl_pattern = ctll::fixed_string{ R"(\w*gl_\w+ .*)" };

			constexpr auto MatchGLPattern(std::string_view sv) noexcept
			{
				return ctre::match<gl_pattern>(sv);
			}

			constexpr bool HasGLPattern(std::string_view sv) noexcept
			{
				if (auto m = MatchGLPattern(sv))
				{
					return true;
				}
				else
				{
					return false;
				}
			}

		}

		bool ContainsLine(std::string_view str, std::string_view contains)
		{
			return str.find(contains) != std::string_view::npos;
		}

		void ReplaceAll(std::string& str, std::string_view from, std::string_view to)
		{
			auto pos = 0;
			while ((pos = str.find(from, pos)) != std::string_view::npos)
			{
				str.replace(pos, from.length(), to);
				pos += to.length();
			}
		}

		namespace
		{
			/*
			auto IsWhiteSpace(char c)
			{
				constexpr std::string_view whitespace_characters = " \t\r\n";
				return std::any_of(std::begin(whitespace_characters), std::end(whitespace_characters), [](const auto& c2) { return c == c2; });
			}

			auto IsSpecialToken(char c)
			{
				constexpr std::string_view special_characters = "{}();,";
				return std::any_of(std::begin(special_characters), std::end(special_characters), [](const auto& c2) { return c == c2; });
			}
			*/

			constexpr auto whitespace_characters = " \t\n";
			constexpr auto special_characters = "{}();,=[]&.-";

			template<typename T>
			std::optional<std::tuple<std::string_view, std::string_view>> GetNextToken(const T& str, std::size_t& current_offset)
			{
				if (current_offset == std::string::npos)
				{
					return {};
				}
				auto token_start = str.find_first_not_of(whitespace_characters, current_offset);
				auto token_end = str.find_first_of(whitespace_characters, token_start);
				auto special_token_start = str.find_first_of(special_characters, token_start);
				if (special_token_start != std::string::npos &&
					special_token_start < token_end)
				{
					token_end = special_token_start;
					if (token_start == special_token_start)
					{
						token_end += 1;
					}
				}

				auto len = token_end - token_start;
				if (token_start == std::string::npos)
				{
					current_offset = std::string::npos;
					return {};
				}
				auto whitespace_view = std::string_view(str).substr(current_offset, token_start - current_offset);
				auto token_view = std::string_view(str).substr(token_start, len);

				current_offset = token_end;
				return std::tuple{ whitespace_view, token_view };
			};

			struct TokenPair
			{
				std::string whitespace;
				std::string token;
			};

			template<typename T>
			auto ParseStringIntoTokenPairs(const T& str)
			{
				std::vector<TokenPair> token_pairs;
				std::size_t index = 0;
				while (auto p = GetNextToken(str, index))
				{
					auto& [whitespace, token] = *p;
					TokenPair t{ std::string(whitespace), std::string(token) };
					token_pairs.emplace_back(std::move(t));
				}
				return token_pairs;
			}

			template<typename T>
			auto ParseSourceLinesIntoTokenPairs(const std::vector<T>& source_lines)
			{
				std::vector<std::vector<TokenPair>> source_line_tokens;

				std::size_t source_index = 0;
				for (auto i = 0; i < source_lines.size(); i++)
				{
					const auto& line = source_lines[i];
					auto line_pairs = ParseStringIntoTokenPairs(line);
					source_line_tokens.emplace_back(std::move(line_pairs));
				}
				return source_line_tokens;
			}
		}

		namespace
		{
			bool IsMacroValidChar(char c)
			{
				return ::isalnum(c) || c == '_' || c == '.';
			}

			bool IsMacroValidArgument(const std::string_view str)
			{
				if (std::all_of(std::begin(str), std::end(str), IsMacroValidChar))
				{
					return true;
				}
				return false;
			}

			class FunctionalMacro
			{
			public:
				explicit FunctionalMacro() = default;
				explicit FunctionalMacro(std::vector<std::string> replacements, std::vector<TokenPair> tokens_) :
					tokens(std::move(tokens_))
				{
					flat_hash_map<std::size_t, std::string> index_to_replacement;
					for (auto i = 0; i < replacements.size(); i++)
					{
						const auto& replacement = replacements[i];
						index_to_replacement[i] = replacement;
					}
					replacement_index_to_token_indices.resize(replacements.size());

					for (auto i = 0; i < tokens.size(); i++)
					{
						const auto& token = tokens[i].token;
						if (auto found = std::find(std::begin(replacements), std::end(replacements), token); found != std::end(replacements))
						{
							replacement_index_to_token_indices[found - std::begin(replacements)].emplace_back(i);
						}
					}
				}

				std::string ExpandMacroWith(std::vector<std::string> arguments) const
				{
					std::stringstream output;

					if (arguments.size() != replacement_index_to_token_indices.size())
					{
						PANIC("Argument size is not equal {} != \n", arguments.size(), replacement_index_to_token_indices.size());
					}

					flat_hash_map<std::size_t, std::string> token_index_to_replacement;
					for (auto i = 0; i < arguments.size(); i++)
					{
						const auto& argument = arguments[i];

						const auto& token_indices = replacement_index_to_token_indices[i];
						for (const auto& index : token_indices)
						{
							token_index_to_replacement.try_emplace(index, argument);
						}
					}

					for (auto i = 0; i < tokens.size(); i++)
					{
						const auto& token = tokens[i].token;
						const auto& whitespace = tokens[i].whitespace;

						output << whitespace;
						if (auto found = token_index_to_replacement.find(i); found != std::end(token_index_to_replacement))
						{
							const auto& replacement = found->second;

							output << replacement;
						}
						else
						{
							output << token;
						}
					}

					return output.str();
				}

			private:
				std::vector<TokenPair> tokens;
				std::vector<std::vector<std::size_t>> replacement_index_to_token_indices;
			};

			std::string PreprocessShaderCode(const std::string_view source)
			{
				std::stringstream ss;

				auto AppendOut = [&](const auto& str)
				{
					ss << str;
				};

				auto source_lines = Util::SplitString(source, "\n");
				std::vector<std::vector<TokenPair>> source_line_tokens = ParseSourceLinesIntoTokenPairs(source_lines);
				
				//#define pos gl_Position
				flat_hash_map<std::string, std::string> basic_macros;
				
				//#define IMAGE_BINDING(format, x) layout(format, binding = x)
				flat_hash_map<std::string, FunctionalMacro> functional_macros;
				
				for (auto i = 0; i < source_lines.size(); i++)
				{
					const auto& line = source_lines[i];
					const auto& tokens = source_line_tokens[i];
					if (tokens.size() > 0)
					{
						if (tokens[0].token == "#define")
						{
							const auto& macro_name = tokens[1].token;

							if (tokens.size() <= 1)
							{
								PANIC("Missing macro define");
							}

							const auto& left_parens_whitespace = tokens[2].whitespace;
							const auto& is_left_parens = tokens[2].token;
							if (is_left_parens == "(" && left_parens_whitespace.empty())
							{
								std::vector<std::string> replacements;
								auto j = 3;
								for (; tokens[j].token != ")"; j++)
								{
									const auto& token = tokens[j].token;
									// filter out replacements that is not just words
									if (!IsMacroValidArgument(token))
									{
										continue;
									}

									replacements.emplace_back(token);
								}

								// this is a functional macro
								std::vector<TokenPair> remaining_token_pairs(std::begin(tokens) + j + 1, std::end(tokens));

								FunctionalMacro functional_macro(replacements, remaining_token_pairs);

								functional_macros[macro_name] = std::move(functional_macro);
							}
							else
							{
								std::stringstream basic_macro;
								for (auto j = 2; j < tokens.size(); j++)
								{
									basic_macro << tokens[j].token << tokens[j].whitespace;
								}
								basic_macros[macro_name] = basic_macro.str();
							}
						}
						else
						{
							// regular token
							
							// check if tokens use macros
							for (auto i = 0; i < tokens.size(); i++)
							{
								const auto& token = tokens[i].token;
								const auto& whitespace = tokens[i].whitespace;
								
								if (auto found = basic_macros.find(token); found != std::end(basic_macros))
								{
									const auto& replacement = found->second;
									AppendOut(whitespace);
									AppendOut(replacement);
								}
								else if (auto found = functional_macros.find(token); found != std::end(functional_macros))
								{
									const auto& functional_macro = found->second;

									std::string token_arguments;
									i+=2;
									for (; tokens[i].token != ")"; i++)
									{
										const auto& token = tokens[i].token;
										if (token != "," && !IsMacroValidArgument(token))
										{
											continue;
										}
										token_arguments += token;
									}
									auto arguments = Util::SplitString(token_arguments, ",");

									auto replacement = functional_macro.ExpandMacroWith(arguments);

									AppendOut(replacement);
								}
								else
								{
									AppendOut(whitespace);
									AppendOut(token);
								}
							}
						}
					}
					else
					{

					}
					AppendOut("\n");
				}

				return ss.str();
			}
		}

		void ShaderConverter::ConvertToVulkanRasterizerShaderSetup(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer, Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer)
		{
			auto GetLastString = [](const auto& str)
			{
				if (auto found = str.rfind(" "); found != std::string::npos)
				{
					return str.substr(found + 1);
				}
				else
				{
					return str;
				}
			};

			draw_call_state_buffer_set = uniform_buffers_start_set;
			draw_call_state_buffer_binding = 0;

			{
				uint32_t set = uniform_buffers_start_set;
				uint32_t binding = uniform_buffers_start_binding;

				auto GenUniformNameToInfo = [&](auto& analyzer)
				{
					const auto& uniforms = analyzer.GetUniforms();
					const auto& uniform_blocks = analyzer.GetUniformBlocks();

					for (const auto& [name, uniform] : uniform_blocks)
					{
						const auto& glsl_type = uniform.GetGLSLType();
						const auto& qualifier = glsl_type->getQualifier();

						//TODO: move somewhere else
						UniformInfo info{};
						info.set = set;
						info.binding = binding;
						info.is_opaque = !glsl_type->containsNonOpaque();
						uniform_name_to_info.try_emplace(name, info);
						binding++;
					}

					for (const auto& [name, uniform] : uniforms)
					{
						const auto& glsl_type = uniform.GetGLSLType();
						const auto& qualifier = glsl_type->getQualifier();

						//TODO: move somewhere else
						UniformInfo info{};
						info.set = set;
						info.binding = binding;
						info.is_opaque = !glsl_type->containsNonOpaque();

						if (uniform.IsPartOfUniformBlock())
						{
							const auto& uniform_block_name = uniform.GetParentUniformBlockName();
							info.parent_block_name = uniform_block_name;
							const auto& parent_uniform = uniform_name_to_info[uniform_block_name];
							info.set = parent_uniform.set;
							info.binding = parent_uniform.binding;
							info.transformed_name = fmt::format("{}", uniform_block_name);
						}
						else
						{
							binding++;
						}
						uniform_name_to_info.try_emplace(name, info);
					}
				};

				GenUniformNameToInfo(vertex_analyzer);
				GenUniformNameToInfo(fragment_analyzer);
			}

			const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
			const auto& vertex_pipeline_index_to_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

			for (auto& [name, pipeline_input] : vertex_pipeline_inputs)
			{
				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				// TODO: Assuming vertex is first member...
				if (qualifier.hasLayout() && qualifier.hasLocation())
				{
					if (qualifier.layoutLocation == 0)
					{
						opengl_vertex_name = name;
					}
				}
				else
				{
					PANIC("Needs to have layout");
				}
			}

			auto GetUniformDefinitions = [this, &GetLastString](Raysterizer::Analysis::GLSLAnalyzer& analyzer)
			{
				phmap::flat_hash_set<std::string> definitions;

				auto& uniforms = analyzer.GetUniforms();
				for (auto& [name, uniform] : uniforms)
				{
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					if (uniform.IsPartOfUniformBlock())
					{
						continue;
					}
					else
					{
						/*
						 * uniform mat4x4 model;
						 *
						 * ->
						 *
						 * layout(set = 0, binding = 0) uniform model_XXX_TYPE
						 * {
						 *    mat4x4 data;
						 * } model_XXX[];
						 */
						constexpr std::string_view uniform_str = "uniform ";
						const auto& uniform_info = GetUniformInfo(name);
						auto uniform_definition = uniform.GetInferredDefinition(uniform_info.set, uniform_info.binding);
						if (glsl_type->containsNonOpaque())
						{
							/*
							 * layout(set = 0, binding = 0) uniform mat4x4 model_XXX
							 *
							 * ->
							 *
							 * layout(set = 0, binding = 0) uniform model_XXX_TYPE
							 * {
							 *    mat4x4 data;
							 * } model_XXX[];
							 */
							auto begin_type_index = uniform_definition.find(uniform_str) + uniform_str.length();
							auto end_type_index = uniform_definition.find(" ", begin_type_index);
							auto type_name = uniform_definition.substr(begin_type_index, end_type_index - begin_type_index);
							uniform_definition.erase(begin_type_index, end_type_index - begin_type_index + 1);
							if (auto found = uniform_definition.find("["); found != std::string::npos)
							{
								/*
								 * layout(set = 1, binding = 0) uniform CB0[53]_383474522287577896_TYPE
								 * {
								 *   vec4 data;
								 * } CB0_383474522287577896[];
								 * ->
								 * layout(set = 1, binding = 0) uniform CB0_383474522287577896_TYPE
								 * {
								 *   vec4 data[53];
								 * } CB0_383474522287577896[];
								 */

								auto uniform_definition_no_array = uniform_definition.substr(0, found);
								auto uniform_definition_array = uniform_definition.substr(found);
								const auto unique_definition = fmt::format("{}{}", uniform_definition_no_array, type_extension);
								auto last_string = GetLastString(unique_definition);
								uniform_name_to_info[name].transformed_name = last_string;

								definitions.emplace(fmt::format(R"(
{definition}
{{
  {type_name} {name}{array_size};
}};
)",
"definition"_a = unique_definition,
"type_name"_a = type_name,
"name"_a = name,
"array_size"_a = uniform_definition_array
));
							}
							else
							{
								const auto unique_definition = fmt::format("{}{}", uniform_definition, type_extension);
								auto last_string = GetLastString(unique_definition);
								uniform_name_to_info[name].transformed_name = last_string;

								definitions.emplace(fmt::format(R"(
{definition}
{{
  {type_name} {name};
}};
)",
"definition"_a = unique_definition,
"type_name"_a = type_name,
"name"_a = name
));
							}
						}
						else
						{
							/*
							 * layout(set = 0, binding = 0) uniform sampler2D texture_diffuse_1_XXX
							 *
							 * ->
							 *
							 * layout(set = 0, binding = 0) uniform sampler2D texture_diffuse_1_XXX[]
							 */

							 // TODO... figure out a way to handle this... with multiarray samplers
							 /*
							  * layout(set = 1, binding = 0) uniform sampler2D samp[8]
							  *
							  * ->
							  *
							  * layout(set = 1, binding = 0) uniform sampler2D samp_3572196053326352771[]
							  *
							  * instead of
							  *
							  * layout(set = 1, binding = 0) uniform sampler2D samp[8]_3572196053326352771[]
							  */

							const auto unique_definition = uniform_definition;
							uniform_name_to_info[name].transformed_name = name;//GetLastString(unique_definition);

							definitions.emplace(fmt::format("{};\n", unique_definition));
						}
					}
				}

				const auto& uniform_blocks = analyzer.GetUniformBlocks();
				for (const auto& [name, uniform] : uniform_blocks)
				{
					/*
					 * layout(std140) uniform Matrices
					 * {
					 *   mat4x4 projection;
					 *   mat4x4 view;
					 * };
					 *
					 * ->
					 *
					 * layout(set = 1, binding = 0, column_major, std140) uniform Matrices
					 * {
					 *   mat4x4 projection;
					 *   mat4x4 view;
					 * } Matrices_8985763445692194806[];
					 */
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					const auto& uniform_info = GetUniformInfo(name);
					auto uniform_definition = uniform.GetInferredDefinition(uniform_info.set, uniform_info.binding);
					//auto last_string = GetLastString(uniform_definition);
					//uniform_name_to_info[name].transformed_name = last_string;
					uniform_name_to_info[name].transformed_name = name;

					definitions.emplace(fmt::format("{};\n", uniform_definition));
				}

				return definitions;
			};

			vertex_uniform_definitions = GetUniformDefinitions(vertex_analyzer);
			fragment_uniform_definitions = GetUniformDefinitions(fragment_analyzer);

			initialized = true;
		}

		VulkanRasterizerSource ShaderConverter::ConvertToVulkanRasterizerShader(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
			Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer)
		{
			auto vertex_source = PreprocessShaderCode(vertex_analyzer.GetSource());
			auto fragment_source = PreprocessShaderCode(fragment_analyzer.GetSource());

			const auto entire_source = vertex_source + fragment_source;
			unique_source_hash = std::to_string(XXH64(entire_source.data(), entire_source.length(), 0));
			const auto& unique_suffix = unique_source_hash;

			auto UniquifyString = [&unique_suffix](const auto& str)
			{
				return fmt::format("{}_{}", str, unique_suffix);
			};

			auto GetLastString = [](const auto& str)
			{
				if (auto found = str.rfind(" "); found != std::string::npos)
				{
					return str.substr(found + 1);
				}
				else
				{
					return str;
				}
			};

			auto vertex_buffer_name = UniquifyString("VertexBuffer");
			auto index_buffer_name = UniquifyString("IndexBuffer");

			vertex_buffer_prefix = vertex_buffer_name;
			vertex_struct_prefix = "vertices";
			index_buffer_prefix = index_buffer_name;
			index_struct_prefix = "indices";

			draw_call_state_struct_name = "DrawCallState";
			draw_call_states_struct_name = UniquifyString("DrawCallStates");
			draw_call_states_name = UniquifyString("draw_call_states");

			const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
			const auto& vertex_pipeline_index_to_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

			// count the tokens to check if function is used in the program (ie. it is called aka 2 or more tokens) (DOES NOT WORK FOR RECURSION)
			auto GetTokenToCount = [](Raysterizer::Analysis::GLSLAnalyzer& analyzer) -> flat_hash_map<std::string, std::size_t>
			{
				auto& tokenizer = analyzer.GetTokenizer();
				flat_hash_map<std::string, std::size_t> token_to_count;
				for (const auto& token : tokenizer.GetTokens())
				{
					const auto& s = token.s;
					if (auto found = token_to_count.find(s); found != std::end(token_to_count))
					{
						found->second++;
					}
					else
					{
						token_to_count[s] = 1;
					}
				}
				return token_to_count;
			};
			auto vertex_token_to_count = GetTokenToCount(vertex_analyzer);
			auto fragment_token_to_count = GetTokenToCount(fragment_analyzer);

			////////////////////////////////////////////////////////////
			// Generated code here
			////////////////////////////////////////////////////////////

#define DEBUG_TRANSFORM_NAME false
			std::stringstream ss{};
			std::stringstream frag_ss{};
			std::stringstream tesc_ss{};
			std::stringstream tese_ss{};

			auto AppendOut = [&](const auto& str)
			{
				ss << str;
			};

			auto AppendOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto AppendOutFrag = [&](const auto& str)
			{
				frag_ss << str;
			};

			auto AppendOutFragFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendOutFrag(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto AppendOutTesc = [&](const auto& str)
			{
				tesc_ss << str;
			};

			auto AppendOutTescFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendOutTesc(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto AppendOutTese = [&](const auto& str)
			{
				tese_ss << str;
			};

			auto AppendOutTeseFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendOutTese(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto GetLayout = [&](uint32_t set, uint32_t binding, std::string additional = "")
			{
				return fmt::format("layout(set = {}, binding = {}{})", set, binding, additional);
			};

			uniform_definitions = vertex_uniform_definitions;
			uniform_definitions.insert(std::begin(fragment_uniform_definitions), std::end(fragment_uniform_definitions));

			std::string vertex_uniform_definitions_code;
			for (const auto& def : vertex_uniform_definitions)
			{
				vertex_uniform_definitions_code += fmt::format("{}", def);
			}

			std::string fragment_uniform_definitions_code;
			for (const auto& def : fragment_uniform_definitions)
			{
				fragment_uniform_definitions_code += fmt::format("{}", def);
			}

			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("UNIFORM DEFINITIONS\n");
				fmt::print("{}\n", vertex_uniform_definitions_code);
				fmt::print("{}\n", fragment_uniform_definitions_code);
			}

			////////////////////////////////////////////////////////////
			// Generated code here
			////////////////////////////////////////////////////////////

			std::stringstream generated_definitions_code_stream;

			auto AppendDefGenCodeOut = [&](const auto& str)
			{
				generated_definitions_code_stream << str;
			};

			auto AppendDefGenCodeOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendDefGenCodeOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			std::stringstream fragment_generated_definitions_code_stream;

			auto AppendDefGenCodeOutFrag = [&](const auto& str)
			{
				fragment_generated_definitions_code_stream << str;
			};

			auto AppendDefGenCodeOutFragFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendDefGenCodeOutFrag(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};


			////////////////////////////////////////////////////////////
			// Add definitions for vertex IN variables
			////////////////////////////////////////////////////////////

			/*
			 * // Vertex inputs
			 * ...
			 * vec3 aPos; <--- Add here
			 *
			 * void main() {
			 * ...
			 * TexCoord = ...
			 */

			AppendDefGenCodeOutFmt("// Vertex inputs\n");
			for (auto& [_, pipeline_input_] : vertex_pipeline_index_to_input)
			{
				auto& pipeline_input = *pipeline_input_;

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				std::optional<uint32_t> override_location{};
				if (pipeline_input.GetLocation() && *pipeline_input.GetLocation() != qualifier.layoutLocation)
				{
					override_location = pipeline_input.GetLocation();
				}
				if (!override_location && qualifier.layoutLocation == qualifier.layoutLocationEnd)
				{
					// try to fix with just the index
					override_location = pipeline_input.GetIndex();
				}
				// This is ensure that the render call has the correct location (because we overrode it)
				if (override_location)
				{
					//pipeline_input.SetLocation(*override_location);
				}

				auto definition = pipeline_input.GetInferredDefinitionWithQualifiers(false, false, override_location);
				AppendDefGenCodeOutFmt("{};\n", definition);
			}
			AppendDefGenCodeOutFmt("\n");


			////////////////////////////////////////////////////////////
			// Add definitions for Vertex OUT variables
			////////////////////////////////////////////////////////////

			/*
			 * vec2 TexCoord; <--- Add here
			 *
			 * void main() {
			 * ...
			 * TexCoord = ...
			 */

			AppendDefGenCodeOutFmt("// Vertex outputs\n");
			phmap::flat_hash_set<std::string> vertex_outputs;
			auto& vertex_pipeline_outputs = vertex_analyzer.GetPipelineOutputs();

			/*
			This is to map vertex outputs to same location as fragment input location

			// Vertex outputs
			layout(location = 7) out float diffuse;
			layout(location = 8) out float fog_factor;
			layout(location = 4) out vec2 fragment_uv;
			layout(location = 9) out float fog_height;
			layout(location = 5) out float fragment_ao;
			layout(location = 6) out float fragment_light;

			->

			// Fragment inputs
			layout(location = 0) in vec2 fragment_uv;
			layout(location = 2) in float fragment_ao;
			layout(location = 3) in float fragment_light;
			layout(location = 1) in float diffuse;
			layout(location = 5) in float fog_factor;
			layout(location = 4) in float fog_height;

			*/
			flat_hash_map<std::string, uint32_t> vertex_pipeline_outputs_to_location;
			for (auto& [name, pipeline_output] : vertex_pipeline_outputs)
			{
				// vec2 TexCoord;

				const auto& object_reflection = pipeline_output.GetObjectReflection();
				const auto& glsl_type = pipeline_output.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				if (std::string_view(name).substr(0, 3) == "gl_")
				{
					continue;
				}

				std::optional<uint32_t> override_location{};
				if (pipeline_output.GetLocation() && *pipeline_output.GetLocation() != qualifier.layoutLocation)
				{
					override_location = pipeline_output.GetLocation();
				}
				if (!override_location && qualifier.layoutLocation == qualifier.layoutLocationEnd)
				{
					// try to fix with just the index
					override_location = pipeline_output.GetIndex();
				}

				if (override_location)
				{
					vertex_pipeline_outputs_to_location[name] = *override_location;
				}
				else
				{
					vertex_pipeline_outputs_to_location[name] = qualifier.layoutLocation;
				}

				auto definition = pipeline_output.GetInferredDefinitionWithQualifiers(false, false, override_location);
				vertex_outputs.emplace(definition);
			}

			for (const auto& definition : vertex_outputs)
			{
				AppendDefGenCodeOutFmt("{};\n", definition);
			}

			AppendDefGenCodeOutFmt("\n");

			////////////////////////////////////////////////////////////
			// Add definitions for fragment IN variables
			////////////////////////////////////////////////////////////

			/*
			 * vec2 TexCoord; <--- Add here
			 *
			 * void main() {
			 * ...
			 * TexCoord = ...
			 */

			AppendDefGenCodeOutFragFmt("// Fragment inputs\n");
			phmap::flat_hash_set<std::string> fragment_inputs;
			auto& fragment_pipeline_inputs = fragment_analyzer.GetPipelineInputs();

			for (auto& [name, pipeline_input] : fragment_pipeline_inputs)
			{
				// vec2 TexCoord;

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				std::optional<uint32_t> override_location{};
				if (pipeline_input.GetLocation() && *pipeline_input.GetLocation() != qualifier.layoutLocation)
				{
					override_location = pipeline_input.GetLocation();
				}
				if (!override_location && qualifier.layoutLocation == qualifier.layoutLocationEnd)
				{
					// try to fix with just the index
					override_location = pipeline_input.GetIndex();
				}

				if (auto found = vertex_pipeline_outputs_to_location.find(name); found != std::end(vertex_pipeline_outputs_to_location))
				{
					override_location = found->second;
				}

				auto definition = pipeline_input.GetInferredDefinitionWithQualifiers(false, false, override_location);
				fragment_inputs.emplace(definition);
			}

			for (const auto& definition : fragment_inputs)
			{
				AppendDefGenCodeOutFragFmt("{};\n", definition);
			}

			AppendDefGenCodeOutFragFmt("\n");

			 ////////////////////////////////////////////////////////////
			 // Add definitions for fragment OUT variables
			 ////////////////////////////////////////////////////////////

			 /*
			  * vec2 TexCoord; <--- Add here
			  *
			  * void main() {
			  * ...
			  * TexCoord = ...
			  */

			AppendDefGenCodeOutFragFmt("// Fragment outputs\n");
			const auto& fragment_pipeline_outputs = fragment_analyzer.GetPipelineOutputs();
			for (const auto& [name, pipeline_output] : fragment_pipeline_outputs)
			{
				// vec4 FragColor;

				const auto& object_reflection = pipeline_output.GetObjectReflection();
				const auto& glsl_type = pipeline_output.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				std::optional<uint32_t> override_location{};
				if (pipeline_output.GetLocation() && *pipeline_output.GetLocation() != qualifier.layoutLocation)
				{
					override_location = pipeline_output.GetLocation();
				}
				if (!override_location && qualifier.layoutLocation == qualifier.layoutLocationEnd)
				{
					// try to fix with just the index
					override_location = pipeline_output.GetIndex();
				}

				//auto definition = pipeline_output.GetInferredDefinitionWithQualifiers(false, false, override_location);
				auto definition = pipeline_output.GetInferredDefinitionWithoutQualifiers(false, false);
				AppendDefGenCodeOutFragFmt("{};\n", definition);
			}
			AppendDefGenCodeOutFragFmt("\n");

			/////////////////////////////////////////

			std::stringstream generated_epilogue_code_stream;

			auto AppendEpilogueGenCodeOut = [&](const auto& str)
			{
				generated_epilogue_code_stream << str;
			};

			auto AppendEpilogueGenCodeOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendEpilogueGenCodeOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto generated_epilogue_code = generated_epilogue_code_stream.str();
			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("GENERATED EPILOGUE\n");
				fmt::print("{}\n", generated_epilogue_code);
			}

			using namespace Raysterizer::Analysis;

			GLSLTokenizer original_vertex_tokenizer;
			PanicIfError(original_vertex_tokenizer.Init(vertex_analyzer.GetTokenizer().GetTokens(), EShLanguage::EShLangVertex));
			GLSLTokenizer original_fragment_tokenizer;
			PanicIfError(original_fragment_tokenizer.Init(fragment_analyzer.GetTokenizer().GetTokens(), EShLanguage::EShLangFragment));

			/////////////////////////////////////////
			// Filter entire code (basically replace any tokens)

			auto IterateAndFilterTokensFunction = [&](GLSLTokenizer& tokenizer)
			{
				tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto& s = token.s;
						if (auto found = token_transformations_unique.find(s); found != std::end(token_transformations_unique))
						{
							s = UniquifyString(found->second);
						}
						return true;
					}
				);
			};
			IterateAndFilterTokensFunction(vertex_analyzer.GetTokenizer());
			IterateAndFilterTokensFunction(fragment_analyzer.GetTokenizer());

			/////////////////////////////////////////
			// Filter main code (basically replace any tokens)

			// TODO: finish all occurances
			const phmap::flat_hash_set<yytokentype> definition_key_words{
				yytokentype::BOOL_,
				yytokentype::INT_,
				yytokentype::UINT_,
				yytokentype::FLOAT_,
				yytokentype::BVEC2,
				yytokentype::BVEC3,
				yytokentype::BVEC4,
				yytokentype::IVEC2,
				yytokentype::IVEC3,
				yytokentype::IVEC4,
				yytokentype::UVEC2,
				yytokentype::UVEC3,
				yytokentype::UVEC4,
				yytokentype::VEC2,
				yytokentype::VEC3,
				yytokentype::VEC4,
				yytokentype::MAT2_,
				yytokentype::MAT3_,
				yytokentype::MAT4_,
				yytokentype::MAT2X2,
				yytokentype::MAT2X3,
				yytokentype::MAT2X4,
				yytokentype::MAT3X2,
				yytokentype::MAT3X3,
				yytokentype::MAT3X4,
				yytokentype::MAT4X2,
				yytokentype::MAT4X3,
				yytokentype::MAT4X4,
			};

			phmap::flat_hash_set<std::string> vertex_main_code_local_definitions;

			auto IterateAndExtractMainFunction = [](GLSLTokenizer& tokenizer, std::function<bool(Analysis::GLSLTokenizerIterator& i, Token& token)> f)
			{
				auto braces_count = 0;

				// only considers functions with void as return value, need to consider actual functions with other return values...
				auto found_void = false;
				Analysis::GLSLTokenizerIterator begin{};
				tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						if (!found_void && token.type == yytokentype::VOID_)
						{
							auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });
							if (next_token.type == yytokentype::IDENTIFIER && next_token.s == "main")
							{
								found_void = true;
								begin = i;
							}
						}
						if (found_void)
						{
							if (token.type == yytokentype::LEFT_BRACE)
							{
								// remove the beginning -- "void main() {"
								if (braces_count == 0)
								{
									for (auto j = begin.index; j < i.index + 1; j++)
									{
										auto& token = tokenizer.GetTokenAtIndex(begin);
										PanicIfError(tokenizer.RemoveTokenAtIndex(Analysis::GLSLTokenizerIterator{ begin.index }));
									}
									i = begin;
									auto& token = tokenizer.GetTokenAtIndex(begin);
									i.index--;
									braces_count++;
									return true;
								}
								else
								{
									braces_count++;
								}
							}
							else if (token.type == yytokentype::RIGHT_BRACE)
							{
								braces_count--;
								if (braces_count == 0)
								{
									found_void = false;
									PanicIfError(tokenizer.RemoveTokenAtIndex(i));
									return true;
								}
								else if (braces_count < 0)
								{
									auto token_string = GLSLTokenizer::CovertTokensToString(tokenizer.GetTokens());
									PANIC("Not possible! {}", token_string);
								}
							}
							if (braces_count > 0)
							{
								return f(i, token);
							}
						}
						return true;
					});
			};

			std::vector<Token> vertex_main_tokens{};
			{
				auto& analyzer = vertex_analyzer;
				GLSLTokenizer& tokenizer = analyzer.GetTokenizer();

				auto& uniforms = analyzer.GetUniforms();

				IterateAndExtractMainFunction(tokenizer, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto type = token.type;
						auto& s = token.s;

						if (auto found = main_token_transformations.find(s); found != std::end(main_token_transformations))
						{
							s = found->second;
						}

						if (auto found = definition_key_words.find(type); found != std::end(definition_key_words))
						{
							//         V and then take this definition
							//    vec4 _215 = _160;
							//    ^ find this

							auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });
							if (next_token.type == yytokentype::IDENTIFIER)
							{
								vertex_main_code_local_definitions.emplace(next_token.s);

							}
						}

						if (main_token_line_removals.contains(s))
						{
							PanicIfError(tokenizer.RemoveLine(i));
						}
						else
						{
							vertex_main_tokens.emplace_back(token);
							PanicIfError(tokenizer.RemoveTokenAtIndex(i));
						}

						return true;
					});
			}

			std::vector<Token> fragment_main_tokens{};
			{
				auto& analyzer = fragment_analyzer;
				GLSLTokenizer& tokenizer = analyzer.GetTokenizer();

				auto& uniforms = analyzer.GetUniforms();

				IterateAndExtractMainFunction(tokenizer, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto type = token.type;
						auto& s = token.s;

						if (auto found = main_token_transformations.find(s); found != std::end(main_token_transformations))
						{
							s = found->second;
						}

						if (0)
						{
							if (auto found = definition_key_words.find(type); found != std::end(definition_key_words))
							{
								//         V and then take this definition
								//    vec4 _215 = _160;
								//    ^ find this

								auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });
								if (next_token.type == yytokentype::IDENTIFIER)
								{
									// check if defintions already exists in vertex side
									if (vertex_main_code_local_definitions.contains(next_token.s))
									{
										// skip this definition part
										PanicIfError(tokenizer.RemoveTokenAtIndex(i));
										return true;
									}
								}
							}
						}

						if (main_token_line_removals.contains(s))
						{
							PanicIfError(tokenizer.RemoveLine(i));
						}
						else
						{
							fragment_main_tokens.emplace_back(token);
							PanicIfError(tokenizer.RemoveTokenAtIndex(i));
						}

						return true;
					});
			}

			struct Contents
			{
				flat_hash_map<std::string, std::vector<Token>> struct_to_contents;
				flat_hash_map<std::string, std::vector<Token>> const_to_contents;
				flat_hash_map<std::string, std::vector<Token>> function_to_contents;
				std::vector<std::string> function_order;

				void Merge(Contents& other)
				{
					struct_to_contents.insert(
						std::make_move_iterator(std::begin(other.struct_to_contents)),
						std::make_move_iterator(std::end(other.struct_to_contents))
					);
					const_to_contents.insert(
						std::make_move_iterator(std::begin(other.const_to_contents)),
						std::make_move_iterator(std::end(other.const_to_contents))
					);
					function_to_contents.insert(
						std::make_move_iterator(std::begin(other.function_to_contents)),
						std::make_move_iterator(std::end(other.function_to_contents))
					);

					phmap::flat_hash_set<std::string> distinct_function_order(std::begin(function_order), std::end(function_order));
					for (auto& other_order : other.function_order)
					{
						if (!distinct_function_order.contains(other_order))
						{
							function_order.emplace_back(std::move(other_order));
						}
					}
				}
			};

			auto FindStructDefinitions = [](Raysterizer::Analysis::GLSLAnalyzer& analyzer, const flat_hash_map<std::string, std::size_t>& token_to_count) -> Contents
			{
				auto& tokenizer = analyzer.GetTokenizer();

				auto found_struct = false;
				std::string struct_name{};
				auto right_braces_line = -2;
				auto semicolon_line = -1;
				std::vector<Token> tokens;
				Contents contents;

				auto AddToStructName = [&](std::string& const_name, std::vector<Token>& tokens)
				{
					contents.struct_to_contents.insert({ const_name, tokens });
				};

				auto AddToConstName = [&](std::string& const_name, std::vector<Token>& tokens)
				{
					contents.const_to_contents.insert({ const_name, tokens });
				};

				auto AddToFunctionName = [&](std::string& const_name, std::vector<Token>& tokens)
				{
					contents.function_to_contents.insert({ const_name, tokens });
					contents.function_order.emplace_back(const_name);
				};

				tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
				{
					auto type = token.type;
					auto& s = token.s;
					if (!found_struct && type == yytokentype::STRUCT)
					{
						std::vector<Token> prev_tokens;

						// include everything inside this...
						auto current_line = i.line;
						tokenizer.IterateStartPrevMut(Analysis::GLSLTokenizerIterator{ i.index - 1 }, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
							{
								if (token.line != current_line)
								{
									return false;
								}
								prev_tokens.emplace_back(token);
								tokenizer.RemoveTokenAtIndex(i);
								i++;
								return true;
							});

						std::reverse_copy(std::begin(prev_tokens), std::end(prev_tokens), std::back_inserter(tokens));
						found_struct = true;
					}

					// now search for struct definition
					if (found_struct)
					{
						if (type == yytokentype::IDENTIFIER)
						{
							if (struct_name.empty())
							{
								struct_name = s;
							}
						}

						// now wait until "}" and ";" are on same line
						else if (type == yytokentype::RIGHT_BRACE)
						{
							right_braces_line = i.line;
						}
						else if (type == yytokentype::SEMICOLON)
						{
							semicolon_line = i.line;
						}

						tokens.emplace_back(token);
						PanicIfError(tokenizer.RemoveTokenAtIndex(i));
						if (right_braces_line == semicolon_line)
						{
							AddToStructName(struct_name, tokens);

							right_braces_line = -2;
							semicolon_line = -1;
							found_struct = false;
							struct_name.clear();
							tokens.clear();
						}
					}

					// check for function
					// float fogFactorLinear(const float dist, const float start, const float end) {
					else if (type == yytokentype::IDENTIFIER)
					{
						auto iter = Analysis::GLSLTokenizerIterator{ i.index + 1 };
						if (tokenizer.HasTokenAtIndex(iter))
						{
							auto& next_token = tokenizer.GetTokenAtIndex(iter);
							if (next_token.type == yytokentype::LEFT_PAREN)
							{
								std::string const_name = token.s;

								bool expect_left_braces = false;
								auto braces_count = 0;

								tokenizer.IterateStartMut(Analysis::GLSLTokenizerIterator{ i.index + 1 }, [&](Analysis::GLSLTokenizerIterator& i2, Token& token)
									{
										if (braces_count > 0)
										{
											if (token.type == yytokentype::LEFT_BRACE)
											{
												braces_count++;
											}
											else if (token.type == yytokentype::RIGHT_BRACE)
											{
												braces_count--;
												if (braces_count == 0)
												{
													tokenizer.IterateStartPrevMut(Analysis::GLSLTokenizerIterator{ i.index }, [&](Analysis::GLSLTokenizerIterator& i3, Token& token)
														{
															if (i3.line != i.line)
															{
																i = i3;
																i.index++;
																return false;
															}
															else if (i3.index == 0)
															{
																i = i3;
															}
															return true;
														});

													std::vector<Token> tokens;
													auto start_index = i.index;
													auto end_iter = i2;

													auto start_line = -1;
													std::string entire_function_declaration;

													for (auto j = start_index; j < end_iter.index + 1; j++)
													{
														Analysis::GLSLTokenizerIterator indexer_iter{ i.index };
														auto& token = tokenizer.GetTokenAtIndex(indexer_iter);
														if (start_line == -1)
														{
															start_line = token.line;
														}
														else
														{
															if (entire_function_declaration.empty() && token.line != start_line)
															{
																entire_function_declaration = GLSLTokenizer::CovertTokensToString(tokens);
															}
														}

														tokens.emplace_back(token);

														PanicIfError(tokenizer.RemoveTokenAtIndex(indexer_iter));
													}
													if (entire_function_declaration.empty())
													{
														entire_function_declaration = GLSLTokenizer::CovertTokensToString(tokens);
													}

													i.index--;

													// search up until "(" and assume previous is function name
													std::string function_name;
													for (auto k = 0; k < tokens.size(); k++)
													{
														const auto& t = tokens[k];
														if (t.type == yytokentype::LEFT_PAREN)
														{
															auto kk = 1;
															while (function_name.empty())
															{
																function_name = tokens[k - kk].s;
																kk++;
															}
															break;
														}
													}
													if (auto found = token_to_count.find(function_name); found != std::end(token_to_count))
													{
														auto is_function_called = found->second > 1;
														if (is_function_called)
														{
															AddToFunctionName(entire_function_declaration, tokens);
														}
													}
													else
													{
														return false;
													}

													return false;
												}
											}
										}
										else if (token.type == yytokentype::RIGHT_PAREN)
										{
											expect_left_braces = true;
										}
										else if (expect_left_braces)
										{
											if (token.type == yytokentype::LEFT_BRACE)
											{
												braces_count++;
											}
											else
											{
												return false;
											}
										}
										return true;
									});
							}
						}
					}
					// check for basic definitions
					// const float brightness = 1.0f / 128.0f;
					else if (type == yytokentype::CONST_)
					{
						std::string const_name{};
						auto start_iter = i;
						tokenizer.IterateStartMut(Analysis::GLSLTokenizerIterator{ i.index }, [&](Analysis::GLSLTokenizerIterator& i2, Token& token)
							{
								if (token.type == yytokentype::SEMICOLON)
								{
									std::vector<Token> tokens;
									for (auto j = start_iter.index; j < i2.index + 1; j++)
									{
										Analysis::GLSLTokenizerIterator indexer_iter{ start_iter.index };
										auto& token = tokenizer.GetTokenAtIndex(indexer_iter);
										tokens.emplace_back(token);
										PanicIfError(tokenizer.RemoveTokenAtIndex(indexer_iter));
									}
									AddToConstName(const_name, tokens);
									i.index--;
									return false;
								}
								// wait until first identifier (aka name of varible shows up)
								else if (token.type == yytokentype::IDENTIFIER && const_name.empty())
								{
									const_name = token.s;
								}
								return true;
							});
					}

					return true;
				});
				return contents;
			};

			auto vertex_struct_definitions_ = FindStructDefinitions(vertex_analyzer, vertex_token_to_count);
			auto fragment_struct_definitions_ = FindStructDefinitions(fragment_analyzer, fragment_token_to_count);

			//vertex_struct_definitions.Merge(fragment_struct_definitions);

			auto ConvertDefinitionToString = [](flat_hash_map<std::string, std::vector<Token>>& mapping) -> std::string
			{
				std::stringstream ss;
				for (const auto& [_, tokens] : mapping)
				{
					auto code = GLSLTokenizer::CovertTokensToString(tokens) + ";\n\n";
					ss << code;
				}
				auto code = ss.str();
				return code;
			};

			auto ConvertFunctionDefinitionToString = [&vertex_analyzer, &fragment_analyzer, &UniquifyString](flat_hash_map<std::string, std::vector<Token>>& mapping, std::vector<std::string>& order) -> std::string
			{
				std::stringstream ss;
				for (const auto& o : order)
				{
					if (auto found = mapping.find(o); found != std::end(mapping))
					{
						auto& [_, tokens] = *found;

						GLSLTokenizer tokenizer;
						PanicIfError(tokenizer.Init(tokens));

						ss << "\n" << tokenizer.ConvertToStringWithoutHeader() << "\n\n";
					}
					else
					{
						PANIC("Expected value {}", o);
					}
				}
				auto code = ss.str();
				return code;
			};

			auto vertex_struct_definitions = ConvertDefinitionToString(vertex_struct_definitions_.struct_to_contents);
			auto vertex_const_definitions = ConvertDefinitionToString(vertex_struct_definitions_.const_to_contents);
			auto vertex_function_definitions = ConvertFunctionDefinitionToString(vertex_struct_definitions_.function_to_contents, vertex_struct_definitions_.function_order);

			auto fragment_struct_definitions = ConvertDefinitionToString(fragment_struct_definitions_.struct_to_contents);
			auto fragment_const_definitions = ConvertDefinitionToString(fragment_struct_definitions_.const_to_contents);
			auto fragment_function_definitions = ConvertFunctionDefinitionToString(fragment_struct_definitions_.function_to_contents, fragment_struct_definitions_.function_order);

			///
			auto vertex_main_code = GLSLTokenizer::CovertTokensToString(vertex_main_tokens);
			auto fragment_main_code = GLSLTokenizer::CovertTokensToString(fragment_main_tokens);
			if (DEBUG_TRANSFORM_NAME)
			{
				//fmt::print("OTHER CODE:\n{}\n", other_code);

				fmt::print("VMAIN:\n{}\n", vertex_main_code);
				//fmt::print("VOTHER:\n{}\n", vertex_other_code);

				fmt::print("FMAIN:\n{}\n", fragment_main_code);
				//fmt::print("FOTHER\n{}\n", fragment_other_code);
			}

			std::string position_name{};
			std::string vertex_normal_name{};
			std::string normal_name{};

			std::string color_name{};
			std::string color_texture_sampler_name{};
			std::string color_texture_coordinates_name{};

			static const std::regex position_search = Config["shader"]["converter"]["position_search"];
			static const std::regex normal_search = Config["shader"]["converter"]["normal_search"];
			static const std::regex coord_tex_search = Config["shader"]["converter"]["coord_tex_search"];

			for (auto& [name, pipeline_input] : vertex_pipeline_inputs)
			{
				// vec3 aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
				// vec3 aNormal = InterpolateBarycentrics(v_XXX_1.aNormal, v_XXX_2.aNormal, v_XXX_3.aNormal);
				// vec2 aTexCoords = InterpolateBarycentrics(v_XXX_1.aTexCoords, v_XXX_2.aTexCoords, v_XXX_3.aTexCoords);

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				auto member = object_reflection.name;

				if (glsl_type->isVector() && glsl_type->getBasicType() == glslang::TBasicType::EbtFloat)
				{
					if (std::regex_search(member, normal_search))
					{
						vertex_normal_name = member;
					}
				}
			}

			for (auto& [name, pipeline_input] : fragment_pipeline_inputs)
			{
				// vec3 aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
				// vec3 aNormal = InterpolateBarycentrics(v_XXX_1.aNormal, v_XXX_2.aNormal, v_XXX_3.aNormal);
				// vec2 aTexCoords = InterpolateBarycentrics(v_XXX_1.aTexCoords, v_XXX_2.aTexCoords, v_XXX_3.aTexCoords);

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				auto member = object_reflection.name;

				if (glsl_type->isVector() && glsl_type->getBasicType() == glslang::TBasicType::EbtFloat)
				{
					// assume position comes first
					if (auto location = pipeline_input.GetLocation(); location && *location == 0)
					{
						position_name = member;
					}
					else if (std::regex_search(member, normal_search))
					{
						normal_name = member;
					}
					else if (std::regex_search(member, coord_tex_search))
					{
						color_texture_coordinates_name = member;
					}
				}
			}

			auto FindUniformNames = [&](Raysterizer::Analysis::GLSLAnalyzer& analyzer)
			{
				for (auto& [name, uniform] : analyzer.GetUniforms())
				{
					const auto& object_reflection = uniform.GetObjectReflection();
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					/*
					* texture(texture_diffuse1, TexCoords);
					*
					* ->
					*
					* texture(texture_diffuse1_2068731311240691495[nonuniformEXT(buffer_index_2068731311240691495)], TextCoords);
					*/
					// check if this is a primitive == NonOpaque
					if (glsl_type->containsOpaque())
					{
						if (name.find("diffuse") != std::string::npos || name.find("Diffuse") != std::string::npos)
						{
							color_texture_sampler_name = name;
						}
					}
				}
			};
			FindUniformNames(vertex_analyzer);
			FindUniformNames(fragment_analyzer);

			for (const auto& [name, pipeline_output] : fragment_pipeline_outputs)
			{
				const auto& object_reflection = pipeline_output.GetObjectReflection();
				const auto& glsl_type = pipeline_output.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				
				std::optional<uint32_t> override_location{};
				if (pipeline_output.GetLocation() && *pipeline_output.GetLocation() != qualifier.layoutLocation)
				{
					override_location = pipeline_output.GetLocation();
				}
				if (!override_location && qualifier.layoutLocation == qualifier.layoutLocationEnd)
				{
					// try to fix with just the index
					override_location = pipeline_output.GetIndex();
				}

				const static std::string color_search = Config["shader"]["converter"]["color_search"];
				static std::regex color_search_regex{ color_search };
				if (!color_search.empty())
				{
					if (std::regex_search(name, color_search_regex))
					{
						color_name = name;
						break;
					}
				}
				else
				{
					if (override_location)
					{
						auto location = *override_location;
						if (location == 0)
						{
							color_name = name;
							break;
						}
					}
				}
			}

			static std::string vertex_custom_code_header;
			static std::string vertex_custom_code;

			static std::string fragment_custom_code_header;
			static std::string fragment_custom_code;

			static std::string tesc_custom_code_header;
			static std::string tesc_custom_code;

			static std::string tese_custom_code_header;
			static std::string tese_custom_code;

			CallOnce
			{
				if (auto f = std::ifstream("common.h"))
				{
					std::vector<std::string> header_lines;
					std::string line{};
					while (std::getline(f, line))
					{
						header_lines.emplace_back(line);
					}

					bool start_copying{ false };
					std::stringstream vertex_ss_header;
					std::stringstream vertex_ss;
					std::stringstream fragment_ss_header;
					std::stringstream fragment_ss;
					std::stringstream tesc_ss_header;
					std::stringstream tesc_ss;
					std::stringstream tese_ss_header;
					std::stringstream tese_ss;

					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "//VertexRasterizationHeader")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "//VertexRasterizationHeaderEnd")
							{
								break;
							}
							vertex_ss_header << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "void VertexRasterizationCustomCode()")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "}")
							{
								break;
							}
							vertex_ss << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "//FragmentRasterizationHeader")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "//FragmentRasterizationHeaderEnd")
							{
								break;
							}
							fragment_ss_header << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "void FragmentRasterizationCustomCode()")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "}")
							{
								break;
							}
							fragment_ss << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "//TessellationControlRasterizationHeader")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "//TessellationControlRasterizationHeaderEnd")
							{
								break;
							}
							tesc_ss_header << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "void TessellationControlRasterizationCustomCode()")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "}")
							{
								break;
							}
							tesc_ss << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "//TessellationEvaluationRasterizationHeader")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "//TessellationEvaluationRasterizationHeaderEnd")
							{
								break;
							}
							tese_ss_header << l << "\n";
						}
					}

					start_copying = false;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "void TessellationEvaluationRasterizationCustomCode()")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "}")
							{
								break;
							}
							tese_ss << l << "\n";
						}
					}

					vertex_custom_code_header = vertex_ss_header.str();
					vertex_custom_code = vertex_ss.str();

					fragment_custom_code_header = fragment_ss_header.str();
					fragment_custom_code = fragment_ss.str();

					tesc_custom_code_header = tesc_ss_header.str();
					tesc_custom_code = tesc_ss.str();

					tese_custom_code_header = tese_ss_header.str();
					tese_custom_code = tese_ss.str();
				}
			};

			// Restore tokenizers
			vertex_analyzer.GetTokenizer() = std::move(original_vertex_tokenizer);
			fragment_analyzer.GetTokenizer() = std::move(original_fragment_tokenizer);
			
			auto vertex_defines_for_user_code = fmt::format(R"(
{position_definition}
{normal_definition}
{color_texture_sampler_definition}
{color_texture_coordinates_definition}

{custom_code}

)",

"position_definition"_a = !position_name.empty() ? fmt::format("#define RAYSTERIZER_POSITION {}", position_name) : "\n",
"normal_definition"_a = !vertex_normal_name.empty() ? fmt::format("#define RAYSTERIZER_NORMAL {}", vertex_normal_name) : "\n",
"color_texture_sampler_definition"_a = !color_texture_sampler_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR_TEXTURE_SAMPLER {}", color_texture_sampler_name) : "\n",
"color_texture_coordinates_definition"_a = !color_texture_coordinates_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR_TEXTURE_COORDINATES {}", color_texture_coordinates_name) : "\n",

"custom_code_header"_a = vertex_custom_code_header,
"custom_code"_a = vertex_custom_code
);

			if (fragment_main_code.find("samp") != std::string::npos && fragment_main_code.find("tevcoord") != std::string::npos && fragment_main_code.find("ocol0 . rgb") != std::string::npos)
			{
				//color_name = "vec3(1.0)";
				//color_name = "texture(samp[0], vec3((gl_FragCoord / 1000.0).xy, 0.0))";
				//color_name = "(vec3(tevin_b.xyz) / 255.0)";
				
				//color_name = "vec3(tex0.xy, 0.0)";
				//color_name = "texture(samp[0], vec3(tex0.xy, 0.0))";
				//color_name = "texture(samp[0], float3(float2(tevcoord.xy).xy* texdim[0].xy, 0.0))";

				if (0)
				{
					auto first = fragment_main_code.find("ivec2 indtevtrans0");
					auto second = fragment_main_code.find("ivec2 indtevtrans2");
					fragment_main_code.replace(first, second - first, "");

					auto last = fragment_main_code.rfind("prev.rgb = clamp((((tevin_d.rgb))");
					auto third = fragment_main_code.find("ivec2 indtevtrans3");
					fragment_main_code.replace(third, last - third, "");
				}

				std::string s = "tex0 . xyz = o . tex0 ;";
				if (auto index = vertex_main_code.find(s); index != std::string::npos)
				{
					//vertex_main_code.replace(index, s.length(), "tex0.xyz = vec3(rawtex0.xy, 1.0);");
					//vertex_main_code.replace(index, s.length(), "tex0.xyz = rawcolor0.xyz;");
					//vertex_main_code.replace(index, s.length(), "tex0.xyz = vec3(normalize(rawtex0.xy), 1.0);");
					//vertex_main_code.replace(index, s.length(), "tex0.xyz = vec3(normalize(rawtex0.x), normalize(rawtex0.y), 1.0);");
					
				}
			}

			auto fragment_defines_for_user_code = fmt::format(R"(
{position_definition}
{normal_definition}
{color_definition}
{color_texture_sampler_definition}
{color_texture_coordinates_definition}

{custom_code}

)",

"position_definition"_a = !position_name.empty() ? fmt::format("#define RAYSTERIZER_POSITION {}", position_name) : "\n",
"normal_definition"_a = !normal_name.empty() ? fmt::format("#define RAYSTERIZER_NORMAL {}", normal_name) : "\n",
"color_definition"_a = !color_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR {}", color_name) : "\n",
"color_texture_sampler_definition"_a = !color_texture_sampler_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR_TEXTURE_SAMPLER {}", color_texture_sampler_name) : "\n",
"color_texture_coordinates_definition"_a = !color_texture_coordinates_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR_TEXTURE_COORDINATES {}", color_texture_coordinates_name) : "\n",

"custom_code"_a = fragment_custom_code
);

			// FULL
			auto shader_header = fmt::format("{}{}{}", shader_version, shader_extensions, include_common_h);
			auto main_function_begin = R"(
void main()
{
)";
			auto main_function_end = R"(

#undef RAY_VERTEX_1
#undef RAY_VERTEX_2
#undef RAY_VERTEX_3

#undef RAY_POSITION
#undef RAY_NORMAL
#undef RAY_COLOR_TEXTURE_SAMPLER
#undef RAY_COLOR_TEXTURE_COORDINATES

}
)";

			auto generated_definitions_code = generated_definitions_code_stream.str();
			auto fragment_generated_definitions_code = fragment_generated_definitions_code_stream.str();
			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("GENERATED DEFINITIONS\n");
				fmt::print("{}\n", generated_definitions_code);
			}

			AppendOutFmt("{}\n", shader_header);
			AppendOutFmt("{}\n", vertex_struct_definitions);
			AppendOutFmt("{}\n", vertex_uniform_definitions_code);
			AppendOutFmt("{}\n", generated_definitions_code);
			AppendOutFmt("{}\n", vertex_const_definitions);
			AppendOutFmt("{}\n", vertex_function_definitions);
			AppendOutFmt("{}\n", vertex_custom_code_header);

			AppendOutFmt("{}\n", main_function_begin);
			AppendOutFmt("{}\n", vertex_main_code);
			AppendOutFmt("{}\n", generated_epilogue_code);
			AppendOutFmt("{}\n", vertex_defines_for_user_code);
			AppendOutFmt("{}\n", main_function_end);



			AppendOutFragFmt("{}\n", shader_header);
			AppendOutFragFmt("{}\n", fragment_struct_definitions);
			AppendOutFragFmt("{}\n", fragment_uniform_definitions_code);
			AppendOutFragFmt("{}\n", fragment_generated_definitions_code);
			AppendOutFragFmt("{}\n", fragment_const_definitions);
			AppendOutFragFmt("{}\n", fragment_function_definitions);
			AppendOutFragFmt("{}\n", fragment_custom_code_header);

			AppendOutFragFmt("{}\n", main_function_begin);
			AppendOutFragFmt("{}\n", fragment_main_code);
			AppendOutFragFmt("{}\n", fragment_defines_for_user_code);
			AppendOutFragFmt("{}\n", generated_epilogue_code);
			AppendOutFragFmt("{}\n", main_function_end);

			// add [] to every definition
			std::string tesc_in_code;
			{
				for (const auto& s : fragment_inputs)
				{
					for (const auto& [name, _] : vertex_pipeline_outputs)
					{
						if (auto found = s.find(name); found != std::string::npos)
						{
							auto updated_s = fmt::format("{}[];\n", s);
							tesc_in_code += updated_s;
							
							break;
						}
					}
				}
			}

			std::string tesc_out_code;

			{
				for (const auto& s : fragment_inputs)
				{
					for (const auto& [name, _] : vertex_pipeline_outputs)
					{
						if (auto found = s.find(name); found != std::string::npos)
						{
							auto updated_s = fmt::format("{}[];\n", s);
							Util::ReplaceString(updated_s, name, fmt::format("OUT_{}", name));
							Util::ReplaceString(updated_s, "in ", "out ");

							tesc_out_code += updated_s;
							break;
						}
					}
				}
			}

			std::string frag_out_code = "// Fragment outputs\n";

			for (const auto& [name, pipeline_output] : fragment_pipeline_outputs)
			{
				// vec4 FragColor;

				const auto& object_reflection = pipeline_output.GetObjectReflection();
				const auto& glsl_type = pipeline_output.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				auto definition = pipeline_output.GetInferredDefinitionWithoutQualifiers();
				frag_out_code += fmt::format("{};\n", definition);
			}

			// assign each header with basic definition of type
			auto tesc_main_tokens = fragment_main_tokens;
			for (auto i = 0; i < tesc_main_tokens.size(); i++)
			{
				auto& token = tesc_main_tokens[i];
				auto& s = token.s;
				if (token.type == yytokentype::IDENTIFIER)
				{
					if (auto found = vertex_pipeline_outputs.find(s); found != std::end(vertex_pipeline_outputs))
					{
						s = fmt::format("{}[gl_InvocationID]", s);
					}
				}
			}
			auto tesc_main_code = GLSLTokenizer::CovertTokensToString(tesc_main_tokens);

			std::string tesc_assignment_code;
			for (const auto& [name, _] : vertex_pipeline_outputs)
			{
				if (std::string_view(name).substr(0, 3) == "gl_")
				{
					continue;
				}
				tesc_assignment_code += fmt::format("OUT_{}[gl_InvocationID] = {}[gl_InvocationID];\n", name, name);
			}

			auto tesc_shader_header = fmt::format(R"(
{shader_header}

#extension GL_EXT_buffer_reference2 : require

#extension GL_EXT_scalar_block_layout : require
//#extension GL_EXT_shader_8bit_storage : require
//#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

)",

"shader_header"_a = shader_header);

			auto tesc_defines_for_user_code = fmt::format(R"(
{position_definition}
{normal_definition}
{color_definition}
{color_texture_sampler_definition}
{color_texture_coordinates_definition}

{custom_code}

{assignment_code}

)",

"position_definition"_a = !position_name.empty() ? fmt::format("#define RAYSTERIZER_POSITION {}", position_name) : "\n",
"normal_definition"_a = !normal_name.empty() ? fmt::format("#define RAYSTERIZER_NORMAL {}", normal_name) : "\n",
"color_definition"_a = !color_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR {}", color_name) : "\n",
"color_texture_sampler_definition"_a = !color_texture_sampler_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR_TEXTURE_SAMPLER {}", color_texture_sampler_name) : "\n",
"color_texture_coordinates_definition"_a = !color_texture_coordinates_name.empty() ? fmt::format("#define RAYSTERIZER_COLOR_TEXTURE_COORDINATES {}", color_texture_coordinates_name) : "\n",

"custom_code"_a = tesc_custom_code,
"assignment_code"_a = tesc_assignment_code
);

			AppendOutTescFmt("{}\n", shader_header);
			AppendOutTescFmt("{}\n", fragment_struct_definitions);
			AppendOutTescFmt("{}\n", fragment_uniform_definitions_code);
			AppendOutTescFmt("{}\n", tesc_in_code);
			AppendOutTescFmt("{}\n", tesc_out_code);
			AppendOutTescFmt("{}\n", frag_out_code);
			AppendOutTescFmt("{}\n", fragment_const_definitions);
			AppendOutTescFmt("{}\n", fragment_function_definitions);
			AppendOutTescFmt("{}\n", tesc_custom_code_header);

			AppendOutTescFmt("{}\n", main_function_begin);
			AppendOutTescFmt("{}\n", tesc_main_code);
			AppendOutTescFmt("{}\n", tesc_defines_for_user_code);
			AppendOutTescFmt("{}\n", generated_epilogue_code);
			AppendOutTescFmt("{}\n", main_function_end);

			auto tese_in_code = tesc_in_code;
			std::string tese_out_code;
			{
				for (const auto& s : fragment_inputs)
				{
					for (const auto& [name, _] : vertex_pipeline_outputs)
					{
						if (auto found = s.find(name); found != std::string::npos)
						{
							auto updated_s = fmt::format("{};\n", s);
							Util::ReplaceString(updated_s, name, fmt::format("OUT_{}", name));
							Util::ReplaceString(updated_s, "in ", "out ");

							tese_out_code += updated_s;
							break;
						}
					}
				}
			}

			std::string tese_assignment_code;
			for (const auto& [name, pipeline_output] : vertex_pipeline_outputs)
			{
				if (std::string_view(name).substr(0, 3) == "gl_")
				{
					continue;
				}

				std::string casting;

				const auto* glsl_type = pipeline_output.GetGLSLType();
				auto glsl_basic_type = glsl_type->getBasicType();
				if (glsl_basic_type == EbtInt)
				{
					casting = "int";
				}
				else if (glsl_basic_type == EbtUint)
				{
					casting = "uint";
				}
				else if (glsl_basic_type == EbtBool)
				{
					casting = "bool";
				}

				tese_assignment_code += fmt::format(R"(
OUT_{name} = {casting}((gl_TessCoord.x * {name}[0]) + (gl_TessCoord.y * {name}[1]) + (gl_TessCoord.z * {name}[2]));
)",
"name"_a = name,
"casting"_a = casting
);
			}

			AppendOutTeseFmt("{}\n", shader_header);
			AppendOutTeseFmt("{}\n", tese_in_code);
			AppendOutTeseFmt("{}\n", tese_out_code);
			AppendOutTeseFmt("{}\n", tese_custom_code_header);

			AppendOutTeseFmt("{}\n", main_function_begin);
			AppendOutTeseFmt("{}\n", tese_assignment_code);
			AppendOutTeseFmt("{}\n", tese_custom_code);
			AppendOutTeseFmt("{}\n", generated_epilogue_code);
			AppendOutTeseFmt("{}\n", main_function_end);

			auto vert_out = ss.str();
			auto frag_out = frag_ss.str();
			auto tesc_out = tesc_ss.str();
			auto tese_out = tese_ss.str();

			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("{}\n", vert_out);
			}

			return VulkanRasterizerSource{ vert_out, frag_out, tesc_out, tese_out };
		}


		Error ShaderConverter::SetupRaytracingGLSLParsing(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
			Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer,
			std::function<std::size_t()> allocate_storage_binding_index,
			std::function<std::size_t()> allocate_uniform_binding_index,
			std::function<std::size_t()> allocate_sampler_binding_index
		)
		{
			auto vertex_source = PreprocessShaderCode(vertex_analyzer.GetSource());
			auto fragment_source = PreprocessShaderCode(fragment_analyzer.GetSource());

			const auto entire_source = vertex_source + fragment_source;
			unique_source_hash = std::to_string(XXH64(entire_source.data(), entire_source.length(), 0));
			const auto& unique_suffix = unique_source_hash;
			
			auto UniquifyString = [&unique_suffix](const auto& str)
			{
				return fmt::format("{}_{}", str, unique_suffix);
			};

			auto GetLastString = [](const auto& str)
			{
				if (auto found = str.rfind(" "); found != std::string::npos)
				{
					return str.substr(found + 1);
				}
				else
				{
					return str;
				}
			};

			draw_call_state_buffer_set = uniform_buffers_start_set;
			draw_call_state_buffer_binding = allocate_storage_binding_index();

			{
				uint32_t set = uniform_buffers_start_set;
				uint32_t binding = uniform_buffers_start_binding;

				uint32_t sampler_binding = samplers_start_binding;
				auto GenUniformNameToInfo = [&](auto& analyzer)
				{
					const auto& uniforms = analyzer.GetUniforms();
					const auto& uniform_blocks = analyzer.GetUniformBlocks();

					for (const auto& [name, uniform] : uniform_blocks)
					{
						const auto& glsl_type = uniform.GetGLSLType();
						const auto& qualifier = glsl_type->getQualifier();

						//TODO: move somewhere else
						UniformInfo info{};
						info.set = set;
						//info.binding = binding;
						info.binding = allocate_uniform_binding_index();
						info.is_opaque = !glsl_type->containsNonOpaque();
						uniform_name_to_info.try_emplace(name, info);
						binding++;
					}

					for (const auto& [name, uniform] : uniforms)
					{
						const auto& glsl_type = uniform.GetGLSLType();
						const auto& qualifier = glsl_type->getQualifier();

						//TODO: move somewhere else
						UniformInfo info{};
						info.set = set;
						info.binding = binding;
						info.is_opaque = !glsl_type->containsNonOpaque();
						
						if (info.is_opaque)
						{
							if (glsl_type->getBasicType() == glslang::TBasicType::EbtSampler)
							{
								//info.binding = sampler_binding++;
								info.binding = allocate_sampler_binding_index();
							}
							else
							{
								PANIC("Not supported");
							}
						}
						else
						{
							info.binding = allocate_uniform_binding_index();
						}

						if (uniform.IsPartOfUniformBlock())
						{
							const auto& uniform_block_name = uniform.GetParentUniformBlockName();
							info.parent_block_name = uniform_block_name;
							const auto& parent_uniform = uniform_name_to_info[uniform_block_name];
							info.set = parent_uniform.set;
							info.binding = parent_uniform.binding;
							info.transformed_name = fmt::format("{}", uniform_block_name);
						}
						else
						{
							binding++;
						}
						uniform_name_to_info.try_emplace(name, info);
					}
				};

				GenUniformNameToInfo(vertex_analyzer);
				GenUniformNameToInfo(fragment_analyzer);
			}

			auto vertex_buffer_name = UniquifyString("VertexBuffer");
			auto index_buffer_name = UniquifyString("IndexBuffer");

			vertex_buffer_prefix = vertex_buffer_name;
			vertex_struct_prefix = "vertices";
			index_buffer_prefix = index_buffer_name;
			index_struct_prefix = "indices";

			draw_call_state_struct_name = "DrawCallState";
			draw_call_states_struct_name = UniquifyString("DrawCallStates");
			draw_call_states_name = UniquifyString("draw_call_states");

			const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
			const auto& vertex_pipeline_index_to_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

			for (auto& [name, pipeline_input] : vertex_pipeline_inputs)
			{
				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				// TODO: Assuming vertex is first member...
				if (qualifier.hasLayout() && qualifier.hasLocation())
				{
					if (qualifier.layoutLocation == 0)
					{
						opengl_vertex_name = name;
					}
				}
				else
				{
					PANIC("Needs to have layout");
				}
			}

			auto GetUniformDefinitions = [this, &UniquifyString, &GetLastString](Raysterizer::Analysis::GLSLAnalyzer& analyzer)
			{
				phmap::flat_hash_set<std::string> definitions;

				auto& uniforms = analyzer.GetUniforms();
				for (auto& [name, uniform] : uniforms)
				{
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					if (uniform.IsPartOfUniformBlock())
					{
						continue;
					}
					else
					{
						/*
						 * uniform mat4x4 model;
						 *
						 * ->
						 *
						 * layout(set = 0, binding = 0) uniform model_XXX_TYPE
						 * {
						 *    mat4x4 data;
						 * } model_XXX[];
						 */
						constexpr std::string_view uniform_str = "uniform ";
						const auto& uniform_info = GetUniformInfo(name);
						auto uniform_definition = uniform.GetInferredDefinition(uniform_info.set, uniform_info.binding);
						if (glsl_type->containsNonOpaque())
						{
							/*
							 * layout(set = 0, binding = 0) uniform mat4x4 model_XXX
							 *
							 * ->
							 *
							 * layout(set = 0, binding = 0) uniform model_XXX_TYPE
							 * {
							 *    mat4x4 data;
							 * } model_XXX[];
							 */
							auto begin_type_index = uniform_definition.find(uniform_str) + uniform_str.length();
							auto end_type_index = uniform_definition.find(" ", begin_type_index);
							auto type_name = uniform_definition.substr(begin_type_index, end_type_index - begin_type_index);
							uniform_definition.erase(begin_type_index, end_type_index - begin_type_index + 1);
							if (auto found = uniform_definition.find("["); found != std::string::npos)
							{
								/*
								 * layout(set = 1, binding = 0) uniform CB0[53]_383474522287577896_TYPE
								 * {
								 *   vec4 data;
								 * } CB0_383474522287577896[];
								 * ->
								 * layout(set = 1, binding = 0) uniform CB0_383474522287577896_TYPE
								 * {
								 *   vec4 data[53];
								 * } CB0_383474522287577896[];
								 */

								auto uniform_definition_no_array = uniform_definition.substr(0, found);
								auto uniform_definition_array = uniform_definition.substr(found);
								const auto unique_definition = fmt::format("{}{}", UniquifyString(uniform_definition_no_array), type_extension);
								uniform_name_to_info[name].transformed_name = GetLastString(unique_definition);

								const auto unique_name = UniquifyString(name);

								definitions.emplace(fmt::format(R"(
{definition}
{{
  {type_name} data{array_size};
}} {name}[];
)",
"definition"_a = unique_definition,
"type_name"_a = type_name,
"name"_a = unique_name,
"array_size"_a = uniform_definition_array
));
							}
							else
							{
								const auto unique_definition = fmt::format("{}{}", UniquifyString(uniform_definition), type_extension);
								uniform_name_to_info[name].transformed_name = GetLastString(unique_definition);

								const auto unique_name = UniquifyString(name);

								definitions.emplace(fmt::format(R"(
{definition}
{{
  {type_name} data;
}} {name}[];
)",
"definition"_a = unique_definition,
"type_name"_a = type_name,
"name"_a = unique_name
));
							}
						}
						else
						{
							/*
							 * layout(set = 0, binding = 0) uniform sampler2D texture_diffuse_1_XXX
							 *
							 * ->
							 *
							 * layout(set = 0, binding = 0) uniform sampler2D texture_diffuse_1_XXX[]
							 */

							 // TODO... figure out a way to handle this... with multiarray samplers
							 /*
							  * layout(set = 1, binding = 0) uniform sampler2D samp[8]
							  *
							  * ->
							  *
							  * layout(set = 1, binding = 0) uniform sampler2D samp_3572196053326352771[]
							  *
							  * instead of
							  *
							  * layout(set = 1, binding = 0) uniform sampler2D samp[8]_3572196053326352771[]
							  */
							std::string remainder = "";
							if (auto found = uniform_definition.find("["); found != std::string::npos)
							{
								//remainder = uniform_definition.substr(found);
								uniform_definition.erase(found);
							}

							const auto unique_definition = UniquifyString(uniform_definition);
							uniform_name_to_info[name].transformed_name = GetLastString(unique_definition);

							definitions.emplace(fmt::format("{}[]{};\n", unique_definition, remainder));
						}
					}
				}

				const auto& uniform_blocks = analyzer.GetUniformBlocks();
				for (const auto& [name, uniform] : uniform_blocks)
				{
					/*
					 * layout(std140) uniform Matrices
					 * {
					 *   mat4x4 projection;
					 *   mat4x4 view;
					 * };
					 *
					 * ->
					 *
					 * layout(set = 1, binding = 0, column_major, std140) uniform Matrices
					 * {
					 *   mat4x4 projection;
					 *   mat4x4 view;
					 * } Matrices_8985763445692194806[];
					 */
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					const auto& uniform_info = GetUniformInfo(name);
					auto uniform_definition = uniform.GetInferredDefinition(uniform_info.set, uniform_info.binding);
					uniform_name_to_info[name].transformed_name = GetLastString(uniform_definition);

					definitions.emplace(fmt::format("{} {}[];\n", uniform_definition, UniquifyString(name)));
				}

				return definitions;
			};

			auto vertex_uniform_definitions = GetUniformDefinitions(vertex_analyzer);
			auto fragment_uniform_definitions = GetUniformDefinitions(fragment_analyzer);

			uniform_definitions = std::move(vertex_uniform_definitions);
			uniform_definitions.insert(std::make_move_iterator(std::begin(fragment_uniform_definitions)),
				std::make_move_iterator(std::end(fragment_uniform_definitions)));

			return NoError();
		}

		std::string ShaderConverter::RemapOpenGLToRaytracingGLSLUpdated2(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer, Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer)
		{
#define DEBUG_TRANSFORM_NAME false
			std::stringstream ss{};

			auto AppendOut = [&](const auto& str)
			{
				ss << str;
			};

			auto AppendOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto GetLayout = [&](uint32_t set, uint32_t binding, std::string additional = "")
			{
				return fmt::format("layout(set = {}, binding = {}{})", set, binding, additional);
			};

			auto vertex_source = PreprocessShaderCode(vertex_analyzer.GetSource());
			auto fragment_source = PreprocessShaderCode(fragment_analyzer.GetSource());

			const auto& unique_suffix = unique_source_hash;

			auto vertex_source_lines = RaysterizerEngine::Util::SplitString(vertex_source, "\n");
			auto fragment_source_lines = RaysterizerEngine::Util::SplitString(fragment_source, "\n");

			auto UniquifyString = [&unique_suffix](const auto& str)
			{
				return fmt::format("{}_{}", str, unique_suffix);
			};

			auto GetLastString = [](const auto& str)
			{
				if (auto found = str.rfind(" "); found != std::string::npos)
				{
					return str.substr(found + 1);
				}
				else
				{
					return str;
				}
			};

			std::string uniform_definitions_code;
			for (const auto& def : uniform_definitions)
			{
				uniform_definitions_code += fmt::format("{}", def);
			}

			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("UNIFORM DEFINITIONS\n");
				fmt::print("{}\n", uniform_definitions_code);
			}

			////////////////////////////////////////////////////////////
			// Generated code here
			////////////////////////////////////////////////////////////

			std::stringstream generated_definitions_code_stream;

			auto AppendDefGenCodeOut = [&](const auto& str)
			{
				generated_definitions_code_stream << str;
			};

			auto AppendDefGenCodeOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendDefGenCodeOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			/*
			 * Convert inputs into a buffer
			 *
			 * layout(location = 0) vec3 aPos;
			 * layout(location = 1) vec3 aNormal;
			 * layout(location = 2) vec2 aTexCoords;
			 *
			 * ->
			 *
			 * struct Vertex_XXX
			 * {
			 *   vec3 aPos;
			 *   vec3 aNormal;
			 *   vec2 aTexCoords;
			 * };
			 *
			 * layout(set = XXX, binding = 0) buffer VertexBuffer_XXX
			 * {
			 *   Vertex_XXX vertices[];
			 * } vertex_buffer_XXX[];
			 */

			 ////////////////////////////////////////////////////////////
			 // Create vertex struct
			 ////////////////////////////////////////////////////////////

			auto struct_vertex_name = UniquifyString("Vertex");

			// struct Vertex_XXX
			AppendDefGenCodeOutFmt("struct {name}\n",
				"name"_a = struct_vertex_name);

			// {
			AppendDefGenCodeOut("{\n");

			//   vec3 aPos;
			//   vec3 aNormal;
			//   vec2 aTexCoords;
			const auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
			const auto& vertex_pipeline_index_to_input = vertex_analyzer.GetPipelineIndexToPipelineInput();

			for (const auto& [_, pipeline_input_] : vertex_pipeline_index_to_input)
			{
				auto& pipeline_input = *pipeline_input_;

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				auto index = pipeline_input.GetIndex();

				if (qualifier.hasLayout() && qualifier.hasLocation())
				{
					auto definition = pipeline_input.GetInferredDefinitionWithoutQualifiers();
					AppendDefGenCodeOutFmt("  {};\n", definition);
				}
				else
				{
					PANIC("No layout");
				}
			}

			if (vertex_pipeline_index_to_input.empty())
			{
				PANIC("No position input!");
				// If there is no data (vertex for input) for some reason..., just put dummpy data???
				AppendDefGenCodeOutFmt("  {};\n", "vec3 GENERATED_DUMMY_UNUSED_POSIITON");
			}

			// }
			AppendDefGenCodeOut("};\n");
			AppendDefGenCodeOut("\n");

			////////////////////////////////////////////////////////////
			// Create vertex buffer
			////////////////////////////////////////////////////////////

			auto vertex_buffer_name = UniquifyString("VertexBuffer");
			auto vertex_buffer_alias = UniquifyString("vertex_buffer");

			// layout(set = XXX, binding = 0) buffer VertexBuffer_ShaderXXX
			AppendDefGenCodeOutFmt("{layout} readonly buffer {name}\n",
				"layout"_a = GetLayout(vertex_buffer_set, vertex_buffer_binding),
				"name"_a = vertex_buffer_name);

			// {
			AppendDefGenCodeOut("{\n");

			//   Vertex_XXX vertices[];
			AppendDefGenCodeOutFmt("  {struct_name} vertices[];\n",
				"struct_name"_a = struct_vertex_name);

			// } vertex_buffer_shader_XXX[];
			AppendDefGenCodeOut("} ");
			AppendDefGenCodeOutFmt("{buffer_alias}[];\n",
				"buffer_alias"_a = vertex_buffer_alias);
			AppendDefGenCodeOut("\n");

			////////////////////////////////////////////////////////////
			// Create index buffer
			////////////////////////////////////////////////////////////

			auto index_buffer_name = UniquifyString("IndexBuffer");
			auto index_buffer_alias = UniquifyString("index_buffer");

			// layout(set = XXX, binding = 0) buffer IndexBuffer_XXX
			AppendDefGenCodeOutFmt("{layout} readonly buffer {name}\n",
				"layout"_a = GetLayout(index_buffer_set, index_buffer_binding, ", scalar"),
				"name"_a = index_buffer_name);

			// {
			AppendDefGenCodeOut("{\n");

			//   uint indices[];
			AppendDefGenCodeOutFmt("  {}[];\n", index_datatype_entire_name);

			// } indices_buffer_shader_XXX[];
			AppendDefGenCodeOut("} ");
			AppendDefGenCodeOutFmt("{buffer_alias}[];\n",
				"buffer_alias"_a = index_buffer_alias);
			AppendDefGenCodeOut("\n");

			////////////////////////////////////////////////////////////
			// Add definitions for vertex IN variables
			////////////////////////////////////////////////////////////

			/*
   			 * // Vertex inputs
 			 * ...
			 * vec3 aPos; <--- Add here
			 *
			 * void main() {
			 * ...
			 * TexCoord = ...
			 */

			AppendDefGenCodeOut("// Vertex inputs\n");
			for (auto& [_, pipeline_input_] : vertex_pipeline_index_to_input)
			{
				auto& pipeline_input = *pipeline_input_;

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				auto index = pipeline_input.GetIndex();

				if (qualifier.hasLayout() && qualifier.hasLocation())
				{
					auto definition = pipeline_input.GetInferredDefinitionWithoutQualifiers(false, false);
					AppendDefGenCodeOutFmt("{};\n", definition);
				}
				else
				{
					PANIC("No layout");
				}
			}
			AppendDefGenCodeOut("//////////////////");
			AppendDefGenCodeOut("\n");

			////////////////////////////////////////////////////////////
			// Add definitions for vertex OUT / fragment IN variables
			////////////////////////////////////////////////////////////

			/*
			 * vec2 TexCoord; <--- Add here
			 *
			 * void main() {
			 * ...
			 * TexCoord = ...
			 */

			AppendDefGenCodeOut("// Vertex outputs / Fragment inputs\n");
			phmap::flat_hash_set<std::string> vertex_outputs_and_fragment_inputs;

			auto& vertex_pipeline_outputs = vertex_analyzer.GetPipelineOutputs();
			auto& fragment_pipeline_inputs = fragment_analyzer.GetPipelineInputs();

			for (auto& [name, pipeline_input] : vertex_pipeline_outputs)
			{
				// vec2 TexCoord;

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				if (qualifier.builtIn == glslang::TBuiltInVariable::EbvNone)
				{
					auto definition = pipeline_input.GetInferredDefinitionWithoutQualifiers();
					vertex_outputs_and_fragment_inputs.emplace(definition);
				}
			}

			for (auto& [name, pipeline_input] : fragment_pipeline_inputs)
			{
				// vec2 TexCoord;

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				auto definition = pipeline_input.GetInferredDefinitionWithoutQualifiers();
				vertex_outputs_and_fragment_inputs.emplace(definition);
			}

			for (const auto& definition : vertex_outputs_and_fragment_inputs)
			{
				AppendDefGenCodeOutFmt("{};\n", definition);
			}

			AppendDefGenCodeOut("\n");

			////////////////////////////////////////////////////////////
			// Add definitions for fragment IN variables
			////////////////////////////////////////////////////////////

			/*
			 * vec2 TexCoord; <--- Add here
			 *
			 * void main() {
			 * ...
			 * TexCoord = ...
			 */

			 /*
			 AppendDefGenCodeOut("// Fragment inputs\n");
			 auto& fragment_pipeline_inputs = fragment_analyzer.GetPipelineInputs();
			 for (auto& [name, pipeline_input] : fragment_pipeline_inputs)
			 {
				 // vec2 TexCoord;

				 const auto& object_reflection = pipeline_input.GetObjectReflection();
				 const auto& glsl_type = pipeline_input.GetGLSLType();
				 const auto& qualifier = glsl_type->getQualifier();

				 auto definition = pipeline_input.GetInferredDefinitionWithoutQualifiers();
				 AppendDefGenCodeOutFmt("{};\n", definition);
			 }
			 AppendDefGenCodeOut("\n");
			 */

			 ////////////////////////////////////////////////////////////
			 // Add definitions for fragment OUT variables
			 ////////////////////////////////////////////////////////////

			 /*
			  * vec2 TexCoord; <--- Add here
			  *
			  * void main() {
			  * ...
			  * TexCoord = ...
			  */

			AppendDefGenCodeOut("// Fragment outputs\n");
			const auto& fragment_pipeline_outputs = fragment_analyzer.GetPipelineOutputs();
			for (const auto& [name, pipeline_output] : fragment_pipeline_outputs)
			{
				// vec4 FragColor;

				const auto& object_reflection = pipeline_output.GetObjectReflection();
				const auto& glsl_type = pipeline_output.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();

				auto definition = pipeline_output.GetInferredDefinitionWithoutQualifiers();
				AppendDefGenCodeOutFmt("{};\n", definition);
			}
			AppendDefGenCodeOut("\n");

			auto GetUniformDefinitionsForwardDeclaration = [&UniquifyString](Raysterizer::Analysis::GLSLAnalyzer& analyzer)
			{
				phmap::flat_hash_set<std::string> definitions;

				auto& uniforms = analyzer.GetUniforms();
				for (auto& [name, uniform] : uniforms)
				{
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					if (!glsl_type->containsNonOpaque())
					{
						continue;
					}

					auto uniform_definition = uniform.GetInferredDefinitionWithoutQualifiers();
					definitions.emplace(fmt::format("{};\n", uniform_definition));
				}

				return definitions;
			};

			/*
			 * vec4 model; <--- Add here
			 * vec4 view; <--- Add here
			 * vec4 projection; <--- Add here
			 * sampler2D texture_diffuse1; // IGNORE OPAQUES
			 *
			 * void main() {
			 * ...
			 * projection = ...
			 */

			auto vertex_uniform_forward_definitions = GetUniformDefinitionsForwardDeclaration(vertex_analyzer);
			auto fragment_uniform_forward_definitions = GetUniformDefinitionsForwardDeclaration(fragment_analyzer);

			phmap::flat_hash_set<std::string> uniform_forward_definitions(std::move(vertex_uniform_forward_definitions));
			uniform_forward_definitions.insert(std::make_move_iterator(std::begin(fragment_uniform_forward_definitions)),
				std::make_move_iterator(std::end(fragment_uniform_forward_definitions)));

			AppendDefGenCodeOut("// Uniforms\n");
			for (const auto& u : uniform_forward_definitions)
			{
				AppendDefGenCodeOut(u);
			}
			AppendDefGenCodeOut("\n");

			// Common definitions
			AppendDefGenCodeOut("// Common Definitions\n");
			for (const auto& u : common_definitions)
			{
				AppendDefGenCodeOutFmt("{}\n", u);
			}

			/*
			auto draw_call_state_struct_name = "DrawCallState";
			draw_call_states_name = "draw_call_states";
			auto draw_call_state_name = "draw_call_state";
			*/

			auto draw_call_state_name = UniquifyString("draw_call_state");

			/*
			 * // GLOBAL STATE
			 * layout(set = 0, binding = 0) uniform DrawCallState
			 * {
			 *	 uint buffer_index;
			 * } draw_call_states[];
			 */

			AppendDefGenCodeOutFmt(R"(
// GLOBAL STATE

{layout} readonly buffer {draw_call_states_struct_name}
{{
	{draw_call_state_struct_name} {draw_call_state_name}[];
}} {draw_call_states_name};

uint buffer_index_{hash};

)",
"layout"_a = GetLayout(draw_call_state_buffer_set, draw_call_state_buffer_binding),
"draw_call_state_struct_name"_a = draw_call_state_struct_name,
"draw_call_states_struct_name"_a = draw_call_states_struct_name,
"draw_call_states_name"_a = draw_call_states_name,
"draw_call_state_name"_a = draw_call_state_name,
"hash"_a = unique_suffix
);

			////////////////////////////////////////////////////////////
			// Generated prologue
			////////////////////////////////////////////////////////////

			/*
			 *   DrawCallState draw_call_state = draw_call_states[nonuniformEXT(gl_InstanceCustomIndexNV)]
			 *   uint buffer_index = draw_call_state.buffer_index;
			 *
			 *   uint i_XXX_1 = index_buffer_shader_XXX[buffer_index].indices[gl_PrimitiveID + 0];
			 *   uint i_XXX_2 = index_buffer_shader_XXX[buffer_index].indices[gl_PrimitiveID + 1];
			 *   uint i_XXX_3 = index_buffer_shader_XXX[buffer_index].indices[gl_PrimitiveID + 2];
			 *	 VertexBuffer_ShaderXXX v_XXX_1 = vertex_buffer_shader_XXX[buffer_index].vertices[i_XXX_1];
			 *	 VertexBuffer_ShaderXXX v_XXX_2 = vertex_buffer_shader_XXX[buffer_index].vertices[i_XXX_2];
			 *	 VertexBuffer_ShaderXXX v_XXX_3 = vertex_buffer_shader_XXX[buffer_index].vertices[i_XXX_3];
			 *	 UniformBuffer_ShaderXXX u_XXX = uniform_buffer_shader_XXX[buffer_index];
			 *
			 *   ...
			 *
			 *   // vertex variables
			 *   aPos = v_XXX_1.aPos;
			 *   aNormal = v_XXX_1.aNormal;
			 *   aTexCoords = v_XXX_1.aTexCoords;
			 *
			 *   // uniform variables
			 *   model = model_XXX[buffer_index];
			 *   view = view_XXX[buffer_index];
			 *   projection = projection_XXX[buffer_index];
			 *
			 *   texture_diffuse1 = texture_diffuse1_XXX[buffer_index];
			 */

			std::stringstream generated_prologue_code_stream;

			auto AppendPrologueGenCodeOut = [&](const auto& str)
			{
				generated_prologue_code_stream << str;
			};

			auto AppendPrologueGenCodeOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendPrologueGenCodeOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto index_name = UniquifyString("i");
			auto index_1 = fmt::format("{}_1", index_name);
			auto index_2 = fmt::format("{}_2", index_name);
			auto index_3 = fmt::format("{}_3", index_name);

			auto vertex_name = UniquifyString("v");
			auto vertex_1 = fmt::format("{}_1", vertex_name);
			auto vertex_2 = fmt::format("{}_2", vertex_name);
			auto vertex_3 = fmt::format("{}_3", vertex_name);

			auto material_index = UniquifyString("material");
			auto buffer_index_name = UniquifyString("buffer_index");

			/*
			AppendDefGenCodeOutFmt(
R"(
uint {buffer_index} = 0;
)",
"buffer_index"_a = buffer_index_name);
			*/

			AppendPrologueGenCodeOutFmt(R"(
  {buffer_index} = gl_InstanceCustomIndexEXT;
  {draw_call_state_struct_name} {draw_call_state_name} = {draw_call_states_name}.{draw_call_state_name}[{buffer_index}];
  const uint {index_1} = uint({index_buffer_alias}[nonuniformEXT({buffer_index})].{index_datatype_name}[(3 * gl_PrimitiveID) + 0]);
  const uint {index_2} = uint({index_buffer_alias}[nonuniformEXT({buffer_index})].{index_datatype_name}[(3 * gl_PrimitiveID) + 1]);
  const uint {index_3} = uint({index_buffer_alias}[nonuniformEXT({buffer_index})].{index_datatype_name}[(3 * gl_PrimitiveID) + 2]);
  const {struct_vertex_name} {vertex_1} = {vertex_buffer_alias}[nonuniformEXT({buffer_index})].vertices[{index_1}];
  const {struct_vertex_name} {vertex_2} = {vertex_buffer_alias}[nonuniformEXT({buffer_index})].vertices[{index_2}];
  const {struct_vertex_name} {vertex_3} = {vertex_buffer_alias}[nonuniformEXT({buffer_index})].vertices[{index_3}];

)",
"draw_call_state_struct_name"_a = draw_call_state_struct_name,
"draw_call_states_name"_a = draw_call_states_name,
"draw_call_state_name"_a = draw_call_state_name,
"material_index"_a = material_index,
"buffer_index"_a = buffer_index_name,
"index_1"_a = index_1,
"index_2"_a = index_2,
"index_3"_a = index_3,
"index_datatype_name"_a = index_datatype_name,
"index_buffer_alias"_a = index_buffer_alias,
"vertex_1"_a = vertex_1,
"vertex_2"_a = vertex_2,
"vertex_3"_a = vertex_3,
"struct_vertex_name"_a = struct_vertex_name,
"vertex_buffer_alias"_a = vertex_buffer_alias
);

			/*
			 * // vertex variables
			 * const Vertex_XXX vertex_1_XXX = vertex_buffer_XXX[nonuniformEXT(buffer_index_XXX)].vertices[i_XXX_1];
			 * const Vertex_XXX vertex_2_XXX = vertex_buffer_XXX[nonuniformEXT(buffer_index_XXX)].vertices[i_XXX_2];
			 * const Vertex_XXX vertex_3_XXX = vertex_buffer_XXX[nonuniformEXT(buffer_index_XXX)].vertices[i_XXX_3];
			 *
			 * aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
			 * aNormal = InterpolateBarycentrics(v_XXX_1.aNormal, v_XXX_2.aNormal, v_XXX_3.aNormal);
			 * aTexCoords = InterpolateBarycentrics(v_XXX_1.aTexCoords, v_XXX_2.aTexCoords, v_XXX_3.aTexCoords);
			 *
			 * // uniform variables
			 * model = model_XXX[buffer_index];
			 * view = view_XXX[buffer_index];
			 * projection = projection_XXX[buffer_index];
			 *
			 * texture_diffuse1 = texture_diffuse1_XXX[buffer_index];
			 */

			for (auto& [name, pipeline_input] : vertex_pipeline_inputs)
			{
				// vec3 aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
				// vec3 aNormal = InterpolateBarycentrics(v_XXX_1.aNormal, v_XXX_2.aNormal, v_XXX_3.aNormal);
				// vec2 aTexCoords = InterpolateBarycentrics(v_XXX_1.aTexCoords, v_XXX_2.aTexCoords, v_XXX_3.aTexCoords);

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				auto member = object_reflection.name;

				// we want to cast
				// vec3 aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
				// ->
				// vec4 aPos = vec4(InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos));
				std::string cast_member = "";
				auto override_glsl_type = pipeline_input.GetOverrideGLSLType();
				auto override_vector_size = pipeline_input.GetOverrideVectorSize();

				if (override_glsl_type)
				{
					if (*override_glsl_type != glsl_type->getBasicType())
					{
						cast_member = pipeline_input.UpdateCastMember(*override_glsl_type, true);
					}
				}
				// cast to original vector size
				else if (override_vector_size)
				{
					if (*override_vector_size != glsl_type->getVectorSize())
					{
						cast_member = pipeline_input.UpdateCastMember(glsl_type->getBasicType(), false);
					}
				}
				else
				{
					cast_member = pipeline_input.UpdateCastMember(glsl_type->getBasicType(), true);
				}

				AppendPrologueGenCodeOutFmt("  {member} = {cast_member}({InterpolateBarycentrics}({vertex_1}.{member}, {vertex_2}.{member}, {vertex_3}.{member}));\n",
					"vertex_1"_a = vertex_1,
					"vertex_2"_a = vertex_2,
					"vertex_3"_a = vertex_3,
					"member"_a = member,
					"cast_member"_a = cast_member,
					"InterpolateBarycentrics"_a = interpolate_barycentrics_code);
			}
			AppendPrologueGenCodeOut("\n");

			auto GenAssignUniformCode = [&](auto& analyzer)
			{
				const auto& uniforms = analyzer.GetUniforms();
				const auto& uniform_blocks = analyzer.GetUniformBlocks();
				for (const auto& [name, uniform] : uniforms)
				{
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					if (!glsl_type->containsNonOpaque())
					{
						continue;
					}

					if (uniform.IsPartOfUniformBlock())
					{
						const auto& uniform_block_name = uniform.GetParentUniformBlockName();
						if (auto found = uniform_blocks.find(uniform_block_name); found != std::end(uniform_blocks))
						{
							auto& [_, uniform_block] = *found;
							AppendPrologueGenCodeOutFmt("  {name} = {uniform_block}[nonuniformEXT({buffer_index})].{name};\n",
								"name"_a = name,
								"uniform_block"_a = UniquifyString(uniform_block_name),
								"buffer_index"_a = buffer_index_name
							);
						}
						else
						{
							fmt::print("{}\n", vertex_source);
							PANIC("Uniform block could not be found {}", uniform_block_name);
						}
					}
					else
					{
						AppendPrologueGenCodeOutFmt("  {name} = {name_array}[nonuniformEXT({buffer_index})].data;\n",
							"name"_a = name,
							"name_array"_a = UniquifyString(name),
							"buffer_index"_a = buffer_index_name
						);
					}
				}
			};
			GenAssignUniformCode(vertex_analyzer);
			GenAssignUniformCode(fragment_analyzer);
			AppendPrologueGenCodeOut("\n");

			auto generated_prologue_code = generated_prologue_code_stream.str();

			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("GENERATED PROLOGUE\n");
				fmt::print("{}\n", generated_prologue_code);
			}

			/////////////////////////////////////////

			std::stringstream generated_epilogue_code_stream;

			auto AppendEpilogueGenCodeOut = [&](const auto& str)
			{
				generated_epilogue_code_stream << str;
			};

			auto AppendEpilogueGenCodeOutFmt = [&](auto&& format_string, auto&&... args)
			{
				AppendEpilogueGenCodeOut(fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			// Need to add the frag color to ray payload
			std::map<uint32_t, const Raysterizer::Analysis::PipelineOutput*> index_to_fragment_pipeline_outputs;
			if (fragment_pipeline_outputs.empty())
			{
				if (GetModuleHandle("RobloxStudioBeta.exe"))
				{
				}
				else
				{
					PANIC("Empty fragment output! No color?");
				}
			}

			for (const auto& [name, pipeline_output] : fragment_pipeline_outputs)
			{
				auto index = pipeline_output.GetIndex();
				index_to_fragment_pipeline_outputs.emplace(index, &pipeline_output);
			}

			for (auto& [index, pipeline_output_] : index_to_fragment_pipeline_outputs)
			{
				const auto& pipeline_output = *pipeline_output_;

				// StorePayloadValue(FragColor);
				const auto& object_reflection = pipeline_output.GetObjectReflection();

				// Assume the first index is output color
				auto definition = pipeline_output.GetInferredDefinitionWithoutQualifiers();
				AppendEpilogueGenCodeOutFmt("  {store_payload_value}({frag_value});\n",
					"store_payload_value"_a = store_payload_value_code,
					"frag_value"_a = object_reflection.name);

				AppendEpilogueGenCodeOutFmt("#define RAYSTERIZER_FRAG_COLOR {frag_value}\n",
					"frag_value"_a = object_reflection.name);

#ifdef RAYSTERIZER_FORCE_FRAG_COLOR
				AppendEpilogueGenCodeOutFmt("  float distance = gl_HitTEXT;\n");
				AppendEpilogueGenCodeOutFmt("  vec3 origin = gl_WorldRayOriginEXT;\n");
				AppendEpilogueGenCodeOutFmt("  vec3 direction = gl_WorldRayDirectionEXT;\n");
				AppendEpilogueGenCodeOutFmt("  {store_payload_value}({frag_value});\n",
					"store_payload_value"_a = store_payload_value_code,
					"frag_value"_a = RAYSTERIZER_FORCE_FRAG_COLOR);
#endif
				break;
			}

			auto generated_epilogue_code = generated_epilogue_code_stream.str();
			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("GENERATED EPILOGUE\n");
				fmt::print("{}\n", generated_epilogue_code);
			}

			using namespace Raysterizer::Analysis;

			GLSLTokenizer original_vertex_tokenizer;
			PanicIfError(original_vertex_tokenizer.Init(vertex_analyzer.GetTokenizer().GetTokens(), EShLanguage::EShLangVertex));
			GLSLTokenizer original_fragment_tokenizer;
			PanicIfError(original_fragment_tokenizer.Init(fragment_analyzer.GetTokenizer().GetTokens(), EShLanguage::EShLangFragment));

			auto IterateAndClear = [](GLSLAnalyzer& analyzer)
			{
				auto& tokenizer = analyzer.GetTokenizer();
				tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto& s = token.s;
						if (token.type == yytokentype::IDENTIFIER)
						{
							if (s == "gl_Position")
							{
								PanicIfError(tokenizer.RemoveLine(i));
							}
						}
						return true;
					});
			};
			IterateAndClear(vertex_analyzer);
			IterateAndClear(fragment_analyzer);

			/////////////////////////////////////////
			// Filter main code (basically replace any tokens)

			// TODO: finish all occurances
			const phmap::flat_hash_set<yytokentype> definition_key_words{
				yytokentype::BOOL_,
				yytokentype::INT_,
				yytokentype::UINT_,
				yytokentype::FLOAT_,
				yytokentype::BVEC2,
				yytokentype::BVEC3,
				yytokentype::BVEC4,
				yytokentype::IVEC2,
				yytokentype::IVEC3,
				yytokentype::IVEC4,
				yytokentype::UVEC2,
				yytokentype::UVEC3,
				yytokentype::UVEC4,
				yytokentype::VEC2,
				yytokentype::VEC3,
				yytokentype::VEC4,
				yytokentype::MAT2_,
				yytokentype::MAT3_,
				yytokentype::MAT4_,
				yytokentype::MAT2X2,
				yytokentype::MAT2X3,
				yytokentype::MAT2X4,
				yytokentype::MAT3X2,
				yytokentype::MAT3X3,
				yytokentype::MAT3X4,
				yytokentype::MAT4X2,
				yytokentype::MAT4X3,
				yytokentype::MAT4X4,
			};

			phmap::flat_hash_set<std::string> vertex_main_code_local_definitions;

			auto IterateAndExtractMainFunction = [](GLSLTokenizer& tokenizer, std::function<bool(Analysis::GLSLTokenizerIterator& i, Token& token)> f)
			{
				auto braces_count = 0;

				// only considers functions with void as return value, need to consider actual functions with other return values...
				auto found_void = false;
				Analysis::GLSLTokenizerIterator begin{};
				tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						if (!found_void && token.type == yytokentype::VOID_)
						{
							auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });
							if (next_token.type == yytokentype::IDENTIFIER && next_token.s == "main")
							{
								found_void = true;
								begin = i;
							}
						}
						if (found_void)
						{
							if (token.type == yytokentype::LEFT_BRACE)
							{
								// remove the beginning -- "void main() {"
								if (braces_count == 0)
								{
									for (auto j = begin.index; j < i.index + 1; j++)
									{
										auto& token = tokenizer.GetTokenAtIndex(begin);
										PanicIfError(tokenizer.RemoveTokenAtIndex(Analysis::GLSLTokenizerIterator{ begin.index }));
									}
									i = begin;
									auto& token = tokenizer.GetTokenAtIndex(begin);
									i.index--;
									braces_count++;
									return true;
								}
								else
								{
									braces_count++;
								}
							}
							else if (token.type == yytokentype::RIGHT_BRACE)
							{
								braces_count--;
								if (braces_count == 0)
								{
									found_void = false;
									PanicIfError(tokenizer.RemoveTokenAtIndex(i));
									return true;
								}
								else if (braces_count < 0)
								{
									PANIC("Not possible!");
								}
							}
							if (braces_count > 0)
							{
								return f(i, token);
							}
						}
						return true;
					});
			};

			std::vector<Token> vertex_main_tokens{};
			{
				auto& analyzer = vertex_analyzer;
				GLSLTokenizer& tokenizer = analyzer.GetTokenizer();

				auto& uniforms = analyzer.GetUniforms();

				IterateAndExtractMainFunction(tokenizer, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto type = token.type;
						auto& s = token.s;

						if (auto found = uniforms.find(s); found != std::end(uniforms))
						{
							auto& [name, uniform] = *found;
							const auto& object_reflection = uniform.GetObjectReflection();
							const auto& glsl_type = uniform.GetGLSLType();
							const auto& qualifier = glsl_type->getQualifier();

							/*
							* texture(texture_diffuse1, TexCoords);
							*
							* ->
							*
							* texture(texture_diffuse1_2068731311240691495[nonuniformEXT(buffer_index_2068731311240691495)], TextCoords);
							*/
							if (!glsl_type->containsNonOpaque())
							{
								s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
									"name_array"_a = UniquifyString(name),
									"buffer_index"_a = buffer_index_name
								);
							}

							if (uniform.IsPartOfUniformBlock())
							{
							}
						}

						if (auto found = main_token_transformations.find(s); found != std::end(main_token_transformations))
						{
							s = found->second;
						}

						if (auto found = definition_key_words.find(type); found != std::end(definition_key_words))
						{
							//         V and then take this definition
							//    vec4 _215 = _160;
							//    ^ find this

							auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });
							if (next_token.type == yytokentype::IDENTIFIER)
							{
								vertex_main_code_local_definitions.emplace(next_token.s);

							}
						}

						if (main_token_line_removals.contains(s))
						{
							PanicIfError(tokenizer.RemoveLine(i));
						}
						else
						{
							vertex_main_tokens.emplace_back(token);
							PanicIfError(tokenizer.RemoveTokenAtIndex(i));
						}

						return true;
					});
			}

			std::vector<Token> fragment_main_tokens{};
			{
				auto& analyzer = fragment_analyzer;
				GLSLTokenizer& tokenizer = analyzer.GetTokenizer();

				auto& uniforms = analyzer.GetUniforms();

				IterateAndExtractMainFunction(tokenizer, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto type = token.type;
						auto& s = token.s;

						if (auto found = uniforms.find(s); found != std::end(uniforms))
						{
							auto& [name, uniform] = *found;
							const auto& object_reflection = uniform.GetObjectReflection();
							const auto& glsl_type = uniform.GetGLSLType();
							const auto& qualifier = glsl_type->getQualifier();

							/*
							* texture(texture_diffuse1, TexCoords);
							*
							* ->
							*
							* texture(texture_diffuse1_2068731311240691495[nonuniformEXT(buffer_index_2068731311240691495)], TextCoords);
							*/
							if (!glsl_type->containsNonOpaque())
							{
								if (!glsl_type->containsArray())
								{
									s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
										"name_array"_a = UniquifyString(name),
										"buffer_index"_a = buffer_index_name
									);
								}
								else
								{
									/*
									* texture(samp[0], ...)
									*
									* ->
									*
									* texture(samp_17174555623704817986[nonuniformEXT(buffer_index_17174555623704817986) + 0], ...)
									*
									* instead of
									*
									* texture(samp_17174555623704817986[nonuniformEXT(buffer_index_17174555623704817986)][0], ...)
									*/

									auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });

									// Next token is "["
									if (next_token.type != yytokentype::LEFT_BRACKET)
									{
										PANIC("Expected bracket access!");
									}

									// loop and remove until we find "]"
									Analysis::GLSLTokenizerIterator right_bracket_iterator{};
									auto i_copy = i;
									tokenizer.IterateStartMut(i_copy, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
										{
											PanicIfError(tokenizer.RemoveTokenAtIndex(i));
											if (token.type == yytokentype::RIGHT_BRACKET)
											{
												return false;
											}
											return true;
										});

									/*
									* TODO:
									* How to fix this???:
									* {name_array}[nonuniformEXT({buffer_index})]
									*
									* ->
									*
									* {name_array}[nonuniformEXT({buffer_index}) + {array_access}]
									*
									* access the array with offset, so in vulkan size, it would access it with (index * draw_calls + array_acess)
									* thus, acting as a 2D array
									*/
									/*
									s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
										"name_array"_a = UniquifyString(name),
										"buffer_index"_a = buffer_index_name,
										"array_access"_a = tokens_inside_bracket_access.str()
									);
									*/
									s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
										"name_array"_a = UniquifyString(name),
										"buffer_index"_a = buffer_index_name
									);
								}
							}

							if (uniform.IsPartOfUniformBlock())
							{
							}
						}

						if (auto found = main_token_transformations.find(s); found != std::end(main_token_transformations))
						{
							s = found->second;
						}

						if (auto found = definition_key_words.find(type); found != std::end(definition_key_words))
						{
							//         V and then take this definition
							//    vec4 _215 = _160;
							//    ^ find this

							auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });
							if (next_token.type == yytokentype::IDENTIFIER)
							{
								// check if defintions already exists in vertex side
								if (vertex_main_code_local_definitions.contains(next_token.s))
								{
									// skip this definition part
									PanicIfError(tokenizer.RemoveTokenAtIndex(i));
									return true;
								}
							}
						}

						if (main_token_line_removals.contains(s))
						{
							PanicIfError(tokenizer.RemoveLine(i));
						}
						else
						{
							fragment_main_tokens.emplace_back(token);
							PanicIfError(tokenizer.RemoveTokenAtIndex(i));
						}

						return true;
					});
			}

			struct Contents
			{
				flat_hash_map<std::string, std::vector<Token>> struct_to_contents;
				flat_hash_map<std::string, std::vector<Token>> const_to_contents;
				flat_hash_map<std::string, std::vector<Token>> function_to_contents;
				std::vector<std::string> function_order;

				void Merge(Contents& other)
				{
					struct_to_contents.insert(
						std::make_move_iterator(std::begin(other.struct_to_contents)),
						std::make_move_iterator(std::end(other.struct_to_contents))
					);
					const_to_contents.insert(
						std::make_move_iterator(std::begin(other.const_to_contents)),
						std::make_move_iterator(std::end(other.const_to_contents))
					);
					function_to_contents.insert(
						std::make_move_iterator(std::begin(other.function_to_contents)),
						std::make_move_iterator(std::end(other.function_to_contents))
					);

					phmap::flat_hash_set<std::string> distinct_function_order(std::begin(function_order), std::end(function_order));
					for (auto& other_order : other.function_order)
					{
						if (!distinct_function_order.contains(other_order))
						{
							function_order.emplace_back(std::move(other_order));
						}
					}
				}
			};

			auto FindStructDefinitions = [](Raysterizer::Analysis::GLSLAnalyzer& analyzer) -> Contents
			{
				auto& tokenizer = analyzer.GetTokenizer();

				auto found_struct = false;
				std::string struct_name{};
				auto right_braces_line = -2;
				auto semicolon_line = -1;
				std::vector<Token> tokens;
				Contents contents;

				auto AddToStructName = [&](std::string& const_name, std::vector<Token>& tokens)
				{
					contents.struct_to_contents.insert({ const_name, tokens });
				};

				auto AddToConstName = [&](std::string& const_name, std::vector<Token>& tokens)
				{
					contents.const_to_contents.insert({ const_name, tokens });
				};

				auto AddToFunctionName = [&](std::string& const_name, std::vector<Token>& tokens)
				{
					contents.function_to_contents.insert({ const_name, tokens });
					contents.function_order.emplace_back(const_name);
				};

				tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
					{
						auto type = token.type;
						auto& s = token.s;
						if (!found_struct && type == yytokentype::STRUCT)
						{
							std::vector<Token> prev_tokens;

							// include everything inside this...
							auto current_line = i.line;
							tokenizer.IterateStartPrevMut(Analysis::GLSLTokenizerIterator{ i.index - 1 }, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
								{
									if (token.line != current_line)
									{
										return false;
									}
									prev_tokens.emplace_back(token);
									tokenizer.RemoveTokenAtIndex(i);
									i++;
									return true;
								});

							std::reverse_copy(std::begin(prev_tokens), std::end(prev_tokens), std::back_inserter(tokens));
							found_struct = true;
						}

						// now search for struct definition
						if (found_struct)
						{
							if (type == yytokentype::IDENTIFIER)
							{
								if (struct_name.empty())
								{
									struct_name = s;
								}
							}

							// now wait until "}" and ";" are on same line
							else if (type == yytokentype::RIGHT_BRACE)
							{
								right_braces_line = i.line;
							}
							else if (type == yytokentype::SEMICOLON)
							{
								semicolon_line = i.line;
							}

							tokens.emplace_back(token);
							PanicIfError(tokenizer.RemoveTokenAtIndex(i));
							if (right_braces_line == semicolon_line)
							{
								AddToStructName(struct_name, tokens);

								right_braces_line = -2;
								semicolon_line = -1;
								found_struct = false;
								struct_name.clear();
								tokens.clear();
							}
						}

						// check for function
						// float fogFactorLinear(const float dist, const float start, const float end) {
						else if (type == yytokentype::IDENTIFIER)
						{
							auto iter = Analysis::GLSLTokenizerIterator{ i.index + 1 };
							if (tokenizer.HasTokenAtIndex(iter))
							{
								auto& next_token = tokenizer.GetTokenAtIndex(iter);
								if (next_token.type == yytokentype::LEFT_PAREN)
								{
									std::string const_name = token.s;

									bool expect_left_braces = false;
									auto braces_count = 0;

									tokenizer.IterateStartMut(Analysis::GLSLTokenizerIterator{ i.index + 1 }, [&](Analysis::GLSLTokenizerIterator& i2, Token& token)
										{
											if (braces_count > 0)
											{
												if (token.type == yytokentype::LEFT_BRACE)
												{
													braces_count++;
												}
												else if (token.type == yytokentype::RIGHT_BRACE)
												{
													braces_count--;
													if (braces_count == 0)
													{
														tokenizer.IterateStartPrevMut(Analysis::GLSLTokenizerIterator{ i.index }, [&](Analysis::GLSLTokenizerIterator& i3, Token& token)
															{
																if (i3.line != i.line)
																{
																	i = i3;
																	i.index++;
																	return false;
																}
																else if (i3.index == 0)
																{
																	i = i3;
																}
																return true;
															});

														std::vector<Token> tokens;
														auto start_index = i.index;
														auto end_iter = i2;

														auto start_line = -1;
														std::string entire_function_declaration;

														for (auto j = start_index; j < end_iter.index + 1; j++)
														{
															Analysis::GLSLTokenizerIterator indexer_iter{ i.index };
															auto& token = tokenizer.GetTokenAtIndex(indexer_iter);
															if (start_line == -1)
															{
																start_line = token.line;
															}
															else
															{
																if (entire_function_declaration.empty() && token.line != start_line)
																{
																	entire_function_declaration = GLSLTokenizer::CovertTokensToString(tokens);
																}
															}

															tokens.emplace_back(token);

															PanicIfError(tokenizer.RemoveTokenAtIndex(indexer_iter));
														}
														if (entire_function_declaration.empty())
														{
															entire_function_declaration = GLSLTokenizer::CovertTokensToString(tokens);
														}

														AddToFunctionName(entire_function_declaration, tokens);
														i.index--;

														return false;
													}
												}
											}
											else if (token.type == yytokentype::RIGHT_PAREN)
											{
												expect_left_braces = true;
											}
											else if (expect_left_braces)
											{
												if (token.type == yytokentype::LEFT_BRACE)
												{
													braces_count++;
												}
												else
												{
													return false;
												}
											}
											return true;
										});
								}
							}
						}
						// check for basic definitions
						// const float brightness = 1.0f / 128.0f;
						else if (type == yytokentype::CONST_)
						{
							std::string const_name{};
							auto start_iter = i;
							tokenizer.IterateStartMut(Analysis::GLSLTokenizerIterator{ i.index }, [&](Analysis::GLSLTokenizerIterator& i2, Token& token)
								{
									if (token.type == yytokentype::SEMICOLON)
									{
										std::vector<Token> tokens;
										for (auto j = start_iter.index; j < i2.index + 1; j++)
										{
											Analysis::GLSLTokenizerIterator indexer_iter{ start_iter.index };
											auto& token = tokenizer.GetTokenAtIndex(indexer_iter);
											tokens.emplace_back(token);
											PanicIfError(tokenizer.RemoveTokenAtIndex(indexer_iter));
										}
										AddToConstName(const_name, tokens);
										i.index--;
										return false;
									}
									// wait until first identifier (aka name of varible shows up)
									else if (token.type == yytokentype::IDENTIFIER && const_name.empty())
									{
										const_name = token.s;
									}
									return true;
								});
						}

						return true;
					});
				return contents;
			};

			auto vertex_struct_definitions = FindStructDefinitions(vertex_analyzer);
			auto fragment_struct_definitions = FindStructDefinitions(fragment_analyzer);

			// Edit const and functions to see if any of the uniforms are used by them
			auto UpdateContentIfContainsUniformReference = [&](flat_hash_map<std::string, std::vector<Token>>& mapping, Raysterizer::Analysis::GLSLAnalyzer& analyzer)
			{
				for (auto& [definition, tokens] : mapping)
				{
					for (auto& token : tokens)
					{
						auto type = token.type;
						auto& s = token.s;
						auto& uniforms = analyzer.GetUniforms();

						if (auto found = uniforms.find(s); found != std::end(uniforms))
						{
							auto& [name, uniform] = *found;
							const auto& object_reflection = uniform.GetObjectReflection();
							const auto& glsl_type = uniform.GetGLSLType();
							const auto& qualifier = glsl_type->getQualifier();

							/*
							* texture(texture_diffuse1, TexCoords);
							*
							* ->
							*
							* texture(texture_diffuse1_2068731311240691495[nonuniformEXT(buffer_index_2068731311240691495)], TextCoords);
							*/
							// check if this is a primitive == NonOpaque
							if (glsl_type->containsNonOpaque())
							{
								s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
									"name_array"_a = UniquifyString(name),
									"buffer_index"_a = buffer_index_name
								);
							}
						}
					}
				}
			};

			/*
			UpdateContentIfContainsUniformReference(vertex_struct_definitions.const_to_contents, vertex_analyzer);
			UpdateContentIfContainsUniformReference(fragment_struct_definitions.const_to_contents, fragment_analyzer);
			UpdateContentIfContainsUniformReference(vertex_struct_definitions.function_to_contents, vertex_analyzer);
			UpdateContentIfContainsUniformReference(fragment_struct_definitions.function_to_contents, fragment_analyzer);
			*/

			vertex_struct_definitions.Merge(fragment_struct_definitions);

			auto ConvertDefinitionToString = [](flat_hash_map<std::string, std::vector<Token>>& mapping) -> std::string
			{
				std::stringstream ss;
				for (const auto& [_, tokens] : mapping)
				{
					auto code = GLSLTokenizer::CovertTokensToString(tokens) + ";\n\n";
					ss << code;
				}
				auto code = ss.str();
				return code;
			};

			auto ConvertFunctionDefinitionToString = [&vertex_analyzer, &fragment_analyzer, &UniquifyString, &buffer_index_name](flat_hash_map<std::string, std::vector<Token>>& mapping, std::vector<std::string>& order) -> std::string
			{
				std::stringstream ss;
				for (const auto& o : order)
				{
					if (auto found = mapping.find(o); found != std::end(mapping))
					{
						auto& [_, tokens] = *found;

						GLSLTokenizer tokenizer;
						PanicIfError(tokenizer.Init(tokens));

						tokenizer.IterateMut([&](Analysis::GLSLTokenizerIterator& i, Token& token)
							{
								auto type = token.type;
								auto& s = token.s;

								auto TransformSamplers = [&](flat_hash_map<std::string, Raysterizer::Analysis::Uniform>& uniforms)
								{
									if (auto found = uniforms.find(s); found != std::end(uniforms))
									{
										auto& [name, uniform] = *found;
										const auto& object_reflection = uniform.GetObjectReflection();
										const auto& glsl_type = uniform.GetGLSLType();
										const auto& qualifier = glsl_type->getQualifier();

										/*
										* texture(texture_diffuse1, TexCoords);
										*
										* ->
										*
										* texture(texture_diffuse1_2068731311240691495[nonuniformEXT(buffer_index_2068731311240691495)], TextCoords);
										*/
										if (!glsl_type->containsNonOpaque())
										{
											if (!glsl_type->containsArray())
											{
												s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
													"name_array"_a = UniquifyString(name),
													"buffer_index"_a = buffer_index_name
												);
											}
											else
											{
												/*
												* texture(samp[0], ...)
												*
												* ->
												*
												* texture(samp_17174555623704817986[nonuniformEXT(buffer_index_17174555623704817986) + 0], ...)
												*
												* instead of
												*
												* texture(samp_17174555623704817986[nonuniformEXT(buffer_index_17174555623704817986)][0], ...)
												*/

												auto& next_token = tokenizer.GetTokenAtIndex(Analysis::GLSLTokenizerIterator{ i.index + 1 });

												// Next token is "["
												if (next_token.type != yytokentype::LEFT_BRACKET)
												{
													PANIC("Expected bracket access!");
												}

												// loop and remove until we find "]"
												Analysis::GLSLTokenizerIterator right_bracket_iterator{};
												tokenizer.IterateStartMut(i, [&](Analysis::GLSLTokenizerIterator& i, Token& token)
													{
														tokenizer.RemoveTokenAtIndex(i);
														if (token.type == yytokentype::RIGHT_BRACKET)
														{
															return false;
														}
														return true;
													});

												/*
												* TODO:
												* How to fix this???:
												* {name_array}[nonuniformEXT({buffer_index})]
												*
												* ->
												*
												* {name_array}[nonuniformEXT({buffer_index}) + {array_access}]
												*
												* access the array with offset, so in vulkan size, it would access it with (index * draw_calls + array_acess)
												* thus, acting as a 2D array
												*/
												/*
												s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
													"name_array"_a = UniquifyString(name),
													"buffer_index"_a = buffer_index_name,
													"array_access"_a = tokens_inside_bracket_access.str()
												);
												*/
												s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
													"name_array"_a = UniquifyString(name),
													"buffer_index"_a = buffer_index_name
												);
											}
										}
									};
								};
								TransformSamplers(vertex_analyzer.GetUniforms());
								TransformSamplers(fragment_analyzer.GetUniforms());
								return true;
							});

						ss << tokenizer.ConvertToStringWithoutHeader();
					}
					else
					{
						PANIC("Expected value {}", o);
					}
				}
				auto code = ss.str();
				return code;
			};

			auto struct_definitions = ConvertDefinitionToString(vertex_struct_definitions.struct_to_contents);
			auto const_definitions = ConvertDefinitionToString(vertex_struct_definitions.const_to_contents);
			auto function_definitions = ConvertFunctionDefinitionToString(vertex_struct_definitions.function_to_contents, vertex_struct_definitions.function_order);

			///
			auto vertex_main_code = GLSLTokenizer::CovertTokensToString(vertex_main_tokens);
			auto fragment_main_code = GLSLTokenizer::CovertTokensToString(fragment_main_tokens);
			if (DEBUG_TRANSFORM_NAME)
			{
				//fmt::print("OTHER CODE:\n{}\n", other_code);

				fmt::print("VMAIN:\n{}\n", vertex_main_code);
				//fmt::print("VOTHER:\n{}\n", vertex_other_code);

				fmt::print("FMAIN:\n{}\n", fragment_main_code);
				//fmt::print("FOTHER\n{}\n", fragment_other_code);
			}

			std::string position_name{};
			std::string normal_name{};

			std::string color_texture_sampler_name{};
			std::string color_texture_coordinates_name{};

			static std::regex position_search = Config["shader"]["converter"]["position_search"];
			static std::regex normal_search = Config["shader"]["converter"]["normal_search"];
			static std::regex coord_tex_search = Config["shader"]["converter"]["coord_tex_search"];
			for (auto& [name, pipeline_input] : fragment_pipeline_inputs)
			{
				// vec3 aPos = InterpolateBarycentrics(v_XXX_1.aPos, v_XXX_2.aPos, v_XXX_3.aPos);
				// vec3 aNormal = InterpolateBarycentrics(v_XXX_1.aNormal, v_XXX_2.aNormal, v_XXX_3.aNormal);
				// vec2 aTexCoords = InterpolateBarycentrics(v_XXX_1.aTexCoords, v_XXX_2.aTexCoords, v_XXX_3.aTexCoords);

				const auto& object_reflection = pipeline_input.GetObjectReflection();
				const auto& glsl_type = pipeline_input.GetGLSLType();
				const auto& qualifier = glsl_type->getQualifier();
				auto member = object_reflection.name;

				if (glsl_type->isVector() && glsl_type->getBasicType() == glslang::TBasicType::EbtFloat)
				{
					// assume position comes first
					if (auto location = pipeline_input.GetLocation(); location && *location == 0)
					{
						position_name = member;
					}
					else if (std::regex_search(member, normal_search))
					{
						normal_name = member;
					}
					else if (std::regex_search(member, coord_tex_search))
					{
						color_texture_coordinates_name = member;
					}
				}
			}

			auto FindUniformNames = [&](Raysterizer::Analysis::GLSLAnalyzer& analyzer)
			{
				for (auto& [name, uniform] : analyzer.GetUniforms())
				{
					const auto& object_reflection = uniform.GetObjectReflection();
					const auto& glsl_type = uniform.GetGLSLType();
					const auto& qualifier = glsl_type->getQualifier();

					/*
					* texture(texture_diffuse1, TexCoords);
					*
					* ->
					*
					* texture(texture_diffuse1_2068731311240691495[nonuniformEXT(buffer_index_2068731311240691495)], TextCoords);
					*/
					// check if this is a primitive == NonOpaque
					if (glsl_type->containsOpaque())
					{
						auto s = fmt::format("{name_array}[nonuniformEXT({buffer_index})]",
							"name_array"_a = UniquifyString(name),
							"buffer_index"_a = buffer_index_name
						);

						if (name.find("diffuse") != std::string::npos || name.find("Diffuse") != std::string::npos)
						{
							color_texture_sampler_name = s;
						}
					}
				}
			};
			FindUniformNames(vertex_analyzer);
			FindUniformNames(fragment_analyzer);

			static std::string custom_code;
			CallOnce
			{
				if (auto f = std::ifstream("common.h"))
				{
					std::vector<std::string> header_lines;
					std::string line{};
					while (std::getline(f, line))
					{
						header_lines.emplace_back(line);
					}

					bool start_copying{ false };
					std::stringstream ss;
					for (auto i = 0; i < header_lines.size(); i++)
					{
						const auto& l = header_lines[i];
						if (l == "void RayCustomCode()")
						{
							i++;
							start_copying = true;
						}
						else if (start_copying)
						{
							if (l == "}")
							{
								break;
							}
							ss << l << "\n";
						}
					}

					custom_code = ss.str();
				}
			};

			// Restore tokenizers
			vertex_analyzer.GetTokenizer() = std::move(original_vertex_tokenizer);
			fragment_analyzer.GetTokenizer() = std::move(original_fragment_tokenizer);

			// allow user to use defines to access generated names
			auto defines_for_user_code = fmt::format(R"(
#define RAY_VERTEX_1 {vertex_1}
#define RAY_VERTEX_2 {vertex_2}
#define RAY_VERTEX_3 {vertex_3}

#define RAYSTERIZER_DRAW_CALL_STATE {draw_call_state_name}

{position_definition}
{normal_definition}
{color_texture_sampler_definition}
{color_texture_coordinates_definition}

{custom_code}

)",
"vertex_1"_a = vertex_1,
"vertex_2"_a = vertex_2,
"vertex_3"_a = vertex_3,

"draw_call_state_name"_a = draw_call_state_name,

"position_definition"_a = !position_name.empty() ? fmt::format("#define RAY_POSITION {}", position_name) : "\n",
"normal_definition"_a = !normal_name.empty() ? fmt::format("#define RAY_NORMAL {}", normal_name) : "\n",
"color_texture_sampler_definition"_a = !color_texture_sampler_name.empty() ? fmt::format("#define RAY_COLOR_TEXTURE_SAMPLER {}", color_texture_sampler_name) : "\n",
"color_texture_coordinates_definition"_a = !color_texture_coordinates_name.empty() ? fmt::format("#define RAY_COLOR_TEXTURE_COORDINATES {}", color_texture_coordinates_name) : "\n",

"custom_code"_a = custom_code
);

			// FULL
			auto shader_header = fmt::format("{}{}", shader_version, shader_extensions);
			auto main_function_begin = R"(
void main()
{
)";
			auto main_function_end = R"(

#undef RAY_VERTEX_1
#undef RAY_VERTEX_2
#undef RAY_VERTEX_3

#undef RAY_POSITION
#undef RAY_NORMAL
#undef RAY_COLOR_TEXTURE_SAMPLER
#undef RAY_COLOR_TEXTURE_COORDINATES

}
)";

			auto generated_definitions_code = generated_definitions_code_stream.str();
			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("GENERATED DEFINITIONS\n");
				fmt::print("{}\n", generated_definitions_code);
			}

			AppendOutFmt("{}\n", shader_header);
			AppendOutFmt("{}\n", struct_definitions);
			AppendOutFmt("{}\n", uniform_definitions_code);
			AppendOutFmt("{}\n", generated_definitions_code);
			AppendOutFmt("{}\n", const_definitions);
			AppendOutFmt("{}\n", function_definitions);

			//AppendOutFmt("{}\n", other_codes_without_structs);

			//AppendOutFmt("{}\n", vertex_other_code);
			//AppendOutFmt("{}\n", fragment_other_code);

			AppendOutFmt("{}\n", main_function_begin);
			AppendOutFmt("{}\n", generated_prologue_code);
			AppendOutFmt("{}\n", vertex_main_code);
			AppendOutFmt("{}\n", fragment_main_code);
			AppendOutFmt("{}\n", generated_epilogue_code);
			AppendOutFmt("{}\n", defines_for_user_code);
			AppendOutFmt("{}\n", main_function_end);

			auto out = ss.str();

			if (DEBUG_TRANSFORM_NAME)
			{
				fmt::print("{}\n", out);
			}

			return out;
		}

		namespace OpenGLToSPIRV
		{
			using namespace std::literals;

			struct InVariable
			{
				std::string_view type;
				std::string_view name;
			};

			static constexpr auto in_pattern = ctll::fixed_string{ R"(\w*in (.*) (.*);)" };

			constexpr auto MatchInPattern(std::string_view sv) noexcept
			{
				return ctre::match<in_pattern>(sv);
			}

			constexpr std::optional<InVariable> GetInVariable(std::string_view sv) noexcept
			{
				if (auto m = MatchInPattern(sv))
				{
					auto type = m.get<1>().to_view();
					auto name = m.get<2>().to_view();
					return InVariable{ type, name };
				}
				else
				{
					return std::nullopt;
				}
			}

			struct OutVariable
			{
				std::string_view type;
				std::string_view name;
			};

			static constexpr auto out_pattern = ctll::fixed_string{ R"(\w*out (.*) (.*);)" };

			constexpr auto MatchOutPattern(std::string_view sv) noexcept
			{
				return ctre::match<out_pattern>(sv);
			}

			constexpr std::optional<OutVariable> GetOutVariable(std::string_view sv) noexcept
			{
				if (auto m = MatchOutPattern(sv))
				{
					auto type = m.get<1>().to_view();
					auto name = m.get<2>().to_view();
					return OutVariable{ type, name };
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		void ShaderConverter::AdjustOpenGLToSPIRVVersion450(Raysterizer::Analysis::GLSLTokenizer& tokenizer)
		{
			auto in_default_location = 0;
			auto out_default_location = 0;

			Raysterizer::Analysis::GLSLTokenizerIterator definitions_iter{};
			bool inserted_temp_frag_color = false;
			tokenizer.IterateMut([&](Raysterizer::Analysis::GLSLTokenizerIterator& iter, Raysterizer::Analysis::Token& token)
				{
					using Raysterizer::Analysis::yytokentype;
					using Raysterizer::Analysis::Token;
					auto type = token.type;
					auto shader_type = tokenizer.GetShaderType();
					switch (type)
					{
					case yytokentype::ATTRIBUTE:
					{
						if (shader_type == EShLanguage::EShLangVertex)
						{
							// alternative old style for "in" for vertex shaders
							//layout(location = 0) in
							PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
									"layout", "(", "location", "=", std::to_string(in_default_location), ")", "in",
								}));
							PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
							in_default_location++;
						}
						break;
					}
					case yytokentype::VARYING:
					{
						// alternative old style for "in/out" for shaders
						//layout(location = 0) in
						if (shader_type == EShLanguage::EShLangVertex)
						{
							PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
								"out"
							}));
							PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
							in_default_location++;
						}
						else if (shader_type == EShLanguage::EShLangFragment)
						{
							PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
								"in"
							}));
							PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
							in_default_location++;
						}
						break;
					}
					/*
					case yytokentype::TEXTURE2D:
					{
						// outdated texture sampling (texture2D)
						PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
							"texture"
						}));
						PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
						break;
					}
					*/
					case yytokentype::YYEOF: // cannot find gl_FragColor as token type???
					{
						// outdated hardcoded gl_FragColor
						// replace the original string with just "FragColor" instead of "gl_FragColor" since "gl_" is reserved
						/*
						line.replace(found, gl_FragColor_str.length(), frag_color_str);

						// insert a definition if it doesn't exist already
						auto frag_definition = fmt::format("out vec4 {};", frag_color_str);
						definitions.emplace(frag_definition);
						*/
						PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
							"FragColor"
							}));
						PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
						break;
					}
					/*
					case yytokentype::DISCARD:
					{
						// outdated texture sampling (texture2D)
						PanicIfError(tokenizer.InsertCustomTypeAtIndex(iter, yytokentype::RETURN, "return"));
						PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
						break;
					}
					*/
					case yytokentype::IN_:
					{
						//
						if (!definitions_iter)
						{
							definitions_iter = iter;
						}
						if (tokenizer.CheckContainsTokenInLine(iter, yytokentype::LAYOUT) == std::nullopt && tokenizer.CheckContainsTokenInLine(iter, "location") == std::nullopt)
						{
							// Check also as function parameter
							/*
							if (!(tokenizer.CheckContainsTokenInLine(iter, yytokentype::COMMA) != std::nullopt &&
								tokenizer.CheckContainsTokenInLine(iter, yytokentype::LEFT_PAREN) != std::nullopt &&
								tokenizer.CheckContainsTokenInLine(iter, yytokentype::RIGHT_PAREN) != std::nullopt))
							{
								PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
								"layout", "(", "location", "=", std::to_string(in_default_location), ")",
									}));
								in_default_location++;
							}
							*/
							if (tokenizer.CheckContainsTokenInLine(iter, yytokentype::LEFT_PAREN) == std::nullopt)
							{
								PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
								"layout", "(", "location", "=", std::to_string(in_default_location), ")",
									}));
								in_default_location++;
							}
						}
						break;
					}
					case yytokentype::OUT_:
					{
						if (!definitions_iter)
						{
							definitions_iter = iter;
						}
						if (tokenizer.CheckContainsTokenInLine(iter, yytokentype::LAYOUT) != std::nullopt && tokenizer.CheckContainsTokenInLine(iter, "location") == std::nullopt)
						{
							// Check also as function parameter
							/*
							if (!(tokenizer.CheckContainsTokenInLine(iter, yytokentype::COMMA) != std::nullopt &&
								tokenizer.CheckContainsTokenInLine(iter, yytokentype::LEFT_PAREN) != std::nullopt &&
								tokenizer.CheckContainsTokenInLine(iter, yytokentype::RIGHT_PAREN) != std::nullopt))
							{
								out_default_location++;
							}
							*/
							if (tokenizer.CheckContainsTokenInLine(iter, yytokentype::LEFT_PAREN) == std::nullopt)
							{
								out_default_location++;
							}
						}
						break;
					}
					case yytokentype::IDENTIFIER:
					{
						if (token.s == "texture2D")
						{
							// outdated texture sampling (texture2D)
							PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
								"texture"
								}));
							PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
						}
						else if (token.s == "gl_FragColor")
						{
							//PanicIfError(tokenizer.RemoveLine(iter));
							if (shader_type == EShLanguage::EShLangFragment)
							{
								PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
									frag_color_str
									}));
								PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
								if (!inserted_temp_frag_color)
								{
									auto frag_color_declaration = std::vector<std::string>{ "out", "vec4", frag_color_str, ";" };
									PanicIfError(tokenizer.InsertTokensAtIndexVec(definitions_iter, frag_color_declaration));
									iter += frag_color_declaration.size();
								}
							}
						}
						break;
					}
					default:
						if (token.s == shader_header_version_string)
						{
							PanicIfError(tokenizer.InsertTokensAtIndexVec(iter, {
								std::string(shader_header_version_string), std::to_string(Raysterizer::Analysis::glsl_version), "core",
								}));
							PanicIfError(tokenizer.RemoveTokenAtIndex(iter));
						}
						break;
					}
					return true;
				}
			);
		}

		std::string ShaderConverter::AdjustOpenGLToSPIRVVersion450(std::string_view vertex_source, EShLanguage lang)
		{
			std::vector<std::string> lines = Util::SplitString(std::string(vertex_source), "\n");
			
			phmap::flat_hash_set<std::string> definitions;
			std::stringstream ss{};

			auto AppendOutFmt = [&](auto&& format_string, auto&&... args)
			{
				ss << (fmt::format(format_string, std::forward<decltype(args)>(args)...));
			};

			auto in_default_location = 0;
			auto out_default_location = 0;
			for (auto& line : lines)
			{
				constexpr auto attribute_str = std::string_view("attribute ");
				constexpr auto varying_str = std::string_view("varying ");
				constexpr auto texture2D_str = std::string_view("texture2D");
				constexpr auto gl_FragColor_str = std::string_view("gl_FragColor");
				constexpr auto frag_color_str = std::string_view("FragColor");
				constexpr auto discard_str = std::string_view("discard;");

				const phmap::flat_hash_set<std::string> remove_lines_with_token = {
				};

				if (line.empty())
				{
					AppendOutFmt("\n");
					continue;
				}
				else if (line.back() == '\r')
				{
					//line = line.substr(0, line.length() - 1);
					line = line.erase(line.length() - 1);
				}
				//

				// alternative old style for "in" for vertex shaders
				if (lang == EShLanguage::EShLangVertex && line.find(attribute_str) != std::string_view::npos)
				{
					line = fmt::format("layout(location = {}) in {}", in_default_location, line.substr(attribute_str.length()));
					in_default_location++;
				}
				// alternative old style for "in/out" for shaders
				if (lang == EShLanguage::EShLangVertex && line.find(varying_str) != std::string_view::npos)
				{
					line = fmt::format("out {}", line.substr(varying_str.length()));
					in_default_location++;
				}
				if (lang == EShLanguage::EShLangFragment && line.find(varying_str) != std::string_view::npos)
				{
					line = fmt::format("in {}\n", line.substr(varying_str.length()));
					in_default_location++;
				}
				// outdated texture sampling (texture2D)
				if (auto found = line.find(texture2D_str); found != std::string_view::npos)
				{
					line.replace(found, texture2D_str.length(), "texture");
				}
				// outdated hardcoded gl_FragColor
				if (auto found = line.find(gl_FragColor_str); found != std::string_view::npos)
				{
					// replace the original string with just "FragColor" instead of "gl_FragColor" since "gl_" is reserved
					line.replace(found, gl_FragColor_str.length(), frag_color_str);

					// insert a definition if it doesn't exist already
					auto frag_definition = fmt::format("out vec4 {};", frag_color_str);
					definitions.emplace(frag_definition);
				}
				if (auto found = line.find(discard_str); found != std::string_view::npos)
				{
					// replace "discard" (remove pixel), by returning from the function
					line.replace(found, discard_str.length(), "return;");
				}
				if (auto in_variable = GetInVariable(line))
				{
					if (line.find("layout") != 0 || line.find("location") == std::string_view::npos)
					{
						line = fmt::format("layout(location = {}) {}", in_default_location, line);
						in_default_location++;
					}
				}
				else if (auto out_variable = GetOutVariable(line))
				{
					if (line.find("layout") != 0 || line.find("location") == std::string_view::npos)
					{
						//out_vertex_source += fmt::format("layout(location = {}) {}\n", out_default_location, line);
						out_default_location++;
						//continue;
					}
				}
				else if (line.find(shader_header_version_string) == 0)
				{
					continue;
				}
				else if (line.find("#extension ") == 0)
				{
					continue;
				}

				{
					bool skip_line = false;
					for (const auto& token : remove_lines_with_token)
					{
						if (line.find(token) != std::string::npos)
						{
							skip_line = true;
							break;
						}
					}
					if (skip_line)
					{
						continue;
					}
				}

				AppendOutFmt("{}\n", line);
			}

			std::string out_string;
			for (const auto& str : definitions)
			{
				out_string += fmt::format("{}\n", str);
			}
			auto result = shader_version_450 + out_string + ss.str();
			return result;
		}

		void ShaderConverter::AdjustOpenGLToSPIRVVersion450(std::vector<uint32_t>& spirv)
		{
			spv_reflect::ShaderModule shader_module(spirv);
			assert(shader_module.GetResult() == SPV_REFLECT_RESULT_SUCCESS);

			uint32_t input_variable_count{};
			assert(shader_module.EnumerateInputVariables(&input_variable_count, nullptr) == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectInterfaceVariable*> input_variables(input_variable_count);
			assert(shader_module.EnumerateInputVariables(&input_variable_count, input_variables.data()) == SPV_REFLECT_RESULT_SUCCESS);

			{
				auto default_location = 0;
				for (auto& v_ : input_variables)
				{
					auto& v = *v_;
					if (v.location == INVALID_VALUE)
					{
						assert(shader_module.ChangeInputVariableLocation(&v, default_location) == SPV_REFLECT_RESULT_SUCCESS);
						default_location++;
					}
				}
			}

			uint32_t output_variable_count{};
			assert(shader_module.EnumerateOutputVariables(&output_variable_count, nullptr) == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectInterfaceVariable*> output_variables(output_variable_count);
			assert(shader_module.EnumerateOutputVariables(&output_variable_count, output_variables.data()) == SPV_REFLECT_RESULT_SUCCESS);

			{
				auto default_location = 0;
				for (auto& v_ : output_variables)
				{
					auto& v = *v_;
					if (v.location == INVALID_VALUE)
					{
						shader_module.ChangeOutputVariableLocation(&v, default_location);
						default_location++;
					}
				}
			}
			/*
			uint32_t interface_variables_count{};
			assert(shader_module.EnumerateInterfaceVariables(&interface_variables_count, nullptr) == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectInterfaceVariable*> interface_variables(interface_variables_count);
			assert(shader_module.EnumerateInterfaceVariables(&interface_variables_count, interface_variables.data()) == SPV_REFLECT_RESULT_SUCCESS);

			{
				auto default_location = 0;
				for (auto& v_ : interface_variables)
				{
					auto& v = *v_;
					if (v. == INVALID_VALUE)
					{
						shader_module.(&v, default_location);
						default_location++;
					}
				}
			}
			*/

			auto* code_start = shader_module.GetCode();
			auto code_size = shader_module.GetCodeSize() / 4;
			auto* code_end = code_start + code_size;
			spirv = std::vector<uint32_t>(code_start, code_end);
		}
	}
}