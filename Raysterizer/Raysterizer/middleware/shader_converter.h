#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace MiddleWare
	{
		struct VulkanRasterizerSource
		{
			std::string vert;
			std::string frag;
			std::string tesc;
			std::string tese;
		};

		class ShaderConverter
		{
		public:
			void ConvertToVulkanRasterizerShaderSetup(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
				Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer);
			VulkanRasterizerSource ConvertToVulkanRasterizerShader(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
				Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer);

			Error SetupRaytracingGLSLParsing(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
				Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer,
				std::function<std::size_t()> allocate_storage_binding_index,
				std::function<std::size_t()> allocate_uniform_binding_index,
				std::function<std::size_t()> allocate_sampler_binding_index
				);
			std::string RemapOpenGLToRaytracingGLSLUpdated2(Raysterizer::Analysis::GLSLAnalyzer& vertex_analyzer,
				Raysterizer::Analysis::GLSLAnalyzer& fragment_analyzer);

			void AdjustOpenGLToSPIRVVersion450(Raysterizer::Analysis::GLSLTokenizer& tokeizer);
			std::string AdjustOpenGLToSPIRVVersion450(std::string_view spirv, EShLanguage lang = EShLanguage::EShLangVertex);
			void AdjustOpenGLToSPIRVVersion450(std::vector<uint32_t>& spirv);

			bool IsUniformName(std::string_view name)
			{
				if (auto found = uniform_names.find(name); found != std::end(uniform_names))
				{
					return true;
				}
				return false;
			}

			Expected<std::string> ConvertNameToTransformedName(std::string_view name, std::size_t index)
			{
				if (auto found = uniform_name_to_info.find(name); found != std::end(uniform_name_to_info))
				{
					const auto& [_, info] = *found;

					return fmt::format("{prefix}",
						"prefix"_a = fmt::format("{}_{}", name, unique_source_hash),
						"index"_a = index,
						"name"_a = name);


					if (info.is_opaque)
					{
						return fmt::format("{prefix}[{index}]",
										   "prefix"_a = fmt::format("{}_{}", name, unique_source_hash),
										   "index"_a = index,
										   "name"_a = name);
					}
					else
					{
						if (info.parent_block_name)
						{
							auto parent_block_name = *info.parent_block_name;
							return fmt::format("{parent}.{prefix}",
											   "parent"_a = fmt::format("{}_{}", parent_block_name, unique_source_hash),
											   "prefix"_a = name,
											   "index"_a = index,
											   "name"_a = name);
						}
						else
						{
							return fmt::format("{prefix}.data",
											   "prefix"_a = fmt::format("{}_{}", name, unique_source_hash),
											   "index"_a = index,
											   "name"_a = name);
						}
					}
				}

				return StringError("Cannot convert name");
			}


			Expected<std::string> ConvertNameToAppendedName(std::string_view name)
			{
				if (auto found = uniform_name_to_info.find(name); found != std::end(uniform_name_to_info))
				{
					const auto& [_, info] = *found;

					if (info.is_opaque)
					{
						return fmt::format("{prefix}",
							"prefix"_a = fmt::format("{}_{}", name, unique_source_hash)
						);
					}
					else
					{
						if (info.parent_block_name)
						{
							auto parent_block_name = *info.parent_block_name;
							return fmt::format("{parent}.{prefix}",
								"parent"_a = fmt::format("{}_{}", parent_block_name, unique_source_hash),
								"prefix"_a = name
							);
						}
						else
						{
							return fmt::format("{prefix}.data",
								"prefix"_a = fmt::format("{}_{}", name, unique_source_hash)
							);
						}
					}
				}

				return StringError("Cannot convert name");
			}

			/*
			void SetIndexDatatype(std::string_view index_datatype_)
			{
				index_datatype = index_datatype_;
			}
			*/

			const std::string& GetIndexDatatype() const
			{
				return index_datatype;
			}

			const std::string& GetIndexDatatypeEntireName() const
			{
				return index_datatype_entire_name;
			}

			std::string AppendHash(const std::string& str) const
			{
				return str + "_" + unique_source_hash;
			}

			std::string opengl_vertex_name{};

			std::string vertex_buffer_prefix{};
			std::string vertex_struct_prefix{};
			std::string index_buffer_prefix{};
			std::string index_struct_prefix{};
			std::string uniform_buffer_prefix{};
			std::string uniform_struct_prefix{};

			std::string draw_call_state_struct_name{};
			std::string draw_call_states_struct_name{};
			std::string draw_call_states_name{};

			phmap::flat_hash_set<std::string> uniform_names{};
			std::vector<std::string> typical_uniform_names{};
			std::vector<std::string> not_typical_uniform_names{};

			std::string unique_source_hash;

			struct UniformInfo
			{
				uint32_t set;
				uint32_t binding;
				bool is_opaque{};
				std::optional<std::string> parent_block_name{};
				std::string transformed_name;
			};
			flat_hash_map<std::string, UniformInfo> uniform_name_to_info{};

			const UniformInfo& GetUniformInfo(std::string_view s)
			{
				if (auto found = uniform_name_to_info.find(s); found != std::end(uniform_name_to_info))
				{
					return found->second;
				}
				else
				{
					PANIC("Uniform info set not found for: {}", s);
				}
			}

			uint32_t GetSetForUniform(std::string_view s)
			{
				if (auto found = uniform_name_to_info.find(s); found != std::end(uniform_name_to_info))
				{
					return found->second.set;
				}
				else
				{
					PANIC("Uniform info set not found for: {}", s);
				}
			}

		public:
			const std::string_view shader_header_version_string = "#version";
			inline const static std::string shader_version_450 = R"(#version 450 core
)";
			const std::string shader_version = R"(
#version 460
)";
			const std::string shader_extensions = R"(
#extension GL_GOOGLE_include_directive : enable

)";

			const std::string include_common_h = R"(
#include "common.h"
)";

			const std::string hit_shader_define_common_h = R"(
#define RT_SHADER
#define HIT_SHADER

)";
			const std::string interpolate_barycentrics_code = "InterpolateBarycentrics";
			const std::string store_payload_value_code = "StorePayloadValue";

			const std::vector<std::string> common_definitions{ {""} };
			const flat_hash_map<std::string, std::string> main_token_transformations{
				/*
				{"return", "return"},
				{"gl_FragCoord", "vec4(gl_LaunchIDEXT.xyz, 0.0f)"},
				{"gl_FragCoord.xy", "gl_LaunchIDEXT.xy"},
				{"gl_FrontFacing", "true"},
				{"gl_VertexID", "gl_PrimitiveID"}
				*/
				{"gl_InstanceID", "gl_InstanceIndex"},
				{"gl_VertexID", "gl_VertexIndex"},
			};

			const flat_hash_map<std::string, std::string> token_transformations_unique{
				{"sampler", "sampler"},
			};

			const phmap::flat_hash_set<std::string> main_token_line_removals{ 
				/*
				{"gl_Position"},
				{"gl_PointSize"},
				{"gl_ClipDistance"},
				{"gl_FragDepth"}
				*/
			};

			const std::string index_datatype = "uint";
			const std::string index_datatype_name = "indices";
			const std::string index_datatype_entire_name = index_datatype + " " + index_datatype_name;

			const std::string frag_color_str = "temp_FragColor";
			const std::string type_extension = "_TYPE";

			/*
			uint32_t draw_call_state_buffer_set = 1;
			uint32_t draw_call_state_buffer_binding = 0;
			*/

			uint32_t vertex_buffer_set = 2;
			uint32_t vertex_buffer_binding = 0;

			uint32_t index_buffer_set = 3;
			uint32_t index_buffer_binding = 0;

			uint32_t uniform_buffers_start_set = 4;
			uint32_t uniform_buffers_start_binding = 0;

			uint32_t samplers_start_set = 4;
			uint32_t samplers_start_binding = 0;

			uint32_t draw_call_state_buffer_set = 0;
			uint32_t draw_call_state_buffer_binding = 0;

			phmap::flat_hash_set<std::string> vertex_uniform_definitions;
			phmap::flat_hash_set<std::string> fragment_uniform_definitions;
			phmap::flat_hash_set<std::string> uniform_definitions;

			bool initialized = false;
		};
	}
}