#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		class Shader
		{
		public:
			explicit Shader(GLuint id_) :
				id(id_)
			{
			}

			void SetSource(std::string_view source_)
			{
				source = source_;
			}

			const std::string& GetSource()
			{
				return source;
			}

			auto& GetAnalyzer() { return *analyzer; }
		protected:
			GLuint id;
			std::string source;
			std::unique_ptr<Raysterizer::Analysis::GLSLAnalyzer> analyzer = std::make_unique<Raysterizer::Analysis::GLSLAnalyzer>();
		};

		class VertexShader: public Shader
		{
		public:
			explicit VertexShader(GLuint id) :
				Shader(id)
			{
			}

			void SetSource(std::string_view source_)
			{
				Shader::SetSource(source_);
				if (auto found = source.find("gl_Position"); found != std::string::npos)
				{
					accesses_gl_position = true;
				}
				else
				{
					accesses_gl_position = false;
				}
			}

			bool AccessesGLPosition() const { return accesses_gl_position; }

		private:
			// TODO:
			// Probably makes sense to put this into analyzer...
			bool accesses_gl_position = false;
		};

		struct FragmentShader : public Shader
		{
		public:
			explicit FragmentShader(GLuint id) :
				Shader(id)
			{
			}
		};

		struct SamplerId
		{
			GLuint sampler_id{};
		};

		using UniformData = std::variant<
			bool,
			int,
			glm::ivec2,
			glm::ivec3,
			glm::ivec4,
			unsigned int,
			glm::uvec2,
			glm::uvec3,
			glm::uvec4,
			float,
			glm::vec2,
			glm::vec3,
			glm::vec4,
			glm::mat2,
			glm::mat3,
			glm::mat4,
			SamplerId,
			std::vector<uint8_t>,
			std::vector<glm::vec2>,
			std::vector<glm::vec3>,
			std::vector<glm::vec4>
		>;

		inline UniformData ConvertTypeToUniformData(GLenum type, std::size_t array_size = 1)
		{
			switch (type)
			{
			case GL_BOOL:
				return int{};
			case GL_INT:
				return int{};
			case GL_INT_VEC2:
				return glm::ivec2{};
			case GL_INT_VEC3:
				return glm::ivec3{};
			case GL_INT_VEC4:
				return glm::ivec4{};
			case GL_UNSIGNED_INT:
				return unsigned int{};
			case GL_UNSIGNED_INT_VEC2:
				return glm::uvec2{};
			case GL_UNSIGNED_INT_VEC3:
				return glm::uvec3{};
			case GL_UNSIGNED_INT_VEC4:
				return glm::uvec4{};
			case GL_FLOAT:
				return float{};
			case GL_FLOAT_VEC2:
				if (array_size > 1)
				{
					return std::vector<glm::vec2>(array_size);
				}
				return glm::vec2{};
			case GL_FLOAT_VEC3:
				if (array_size > 1)
				{
					return std::vector<glm::vec3>(array_size);
				}
				return glm::vec3{};
			case GL_FLOAT_VEC4:
				if (array_size > 1)
				{
					return std::vector<glm::vec4>(array_size);
				}
				return glm::vec4{};
			case GL_FLOAT_MAT2:
				return glm::mat2{};
			case GL_FLOAT_MAT3:
				return glm::mat3{};
			case GL_FLOAT_MAT4:
				return glm::mat4{};
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
				return SamplerId{};
			default:
				//PANIC("Not able to convert type to uniform data 0x{:X}", type);
				return int{};
			}
			return int{};
		}

		struct Uniform
		{
		public:
			void SetData(UniformData data_)
			{
				if (data.index() != data_.index())
				{
					PANIC("Index not same");
				}
				data = data_;
			}

			const UniformData& GetData() const
			{
				return data;
			}

			UniformData& GetData()
			{
				return data;
			}

			template<typename T>
			Error TransferDataTo(T& other_data) const
			{
				if (auto underlying_data = std::get_if<T>(&data))
				{
					other_data = *underlying_data;
				}
				else
				{
					return StringError("Unable to transfer data to other buffer");
				}
				return NoError();
			}

			GLuint location;
			std::string name;
			UniformData data;
		};

		struct UniformBlock
		{
		public:
			GLuint index;
			std::string name;
			GLuint block_binding;
		};

		struct Attrib
		{
			void SetData(UniformData data_)
			{
				if (data.index() != data_.index())
				{
					PANIC("Index not same");
				}
				data = data_;
			}

			GLuint location;
			std::string name;
			UniformData data;
		};

		class Program
		{
		public:
			explicit Program(GLuint id_) :
				id(id_)
			{
			}

			GLuint GetId()
			{
				return id;
			}

			void SetVertexShader(std::shared_ptr<VertexShader> vertex_shader_)
			{
				if (vertex_shader_ == nullptr)
				{
					PANIC("Null vertex shader assigned!");
				}
				vertex_shader = vertex_shader_;
			}

			VertexShader& GetVertexShader()
			{
				return *vertex_shader;
			}

			void SetFragmentShader(std::shared_ptr<FragmentShader> fragment_shader_)
			{
				if (fragment_shader_ == nullptr)
				{
					PANIC("Null fragment shader assigned!");
				}
				fragment_shader = fragment_shader_;
			}

			FragmentShader& GetFragmentShader()
			{
				return *fragment_shader;
			}

			Error LinkProgram()
			{
				/*
				program = std::make_unique<glslang::TProgram>();
				program->addShader(&vertex_shader->GetAnalyzer().GetShader());
				program->addShader(&fragment_shader->GetAnalyzer().GetShader());

				EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgAST | EShMsgDebugInfo);

				if (!program->link(messages))
				{
					return StringError("Shader error");
				}

				constexpr auto reflection_opts = EShReflectionOptions::EShReflectionBasicArraySuffix |
					EShReflectionOptions::EShReflectionSeparateBuffers |
					EShReflectionOptions::EShReflectionStrictArraySuffix |
					EShReflectionOptions::EShReflectionAllBlockVariables |
					EShReflectionOptions::EShReflectionIntermediateIO |
					EShReflectionOptions::EShReflectionUnwrapIOBlocks |
					EShReflectionOptions::EShReflectionAllIOVariables;
				program->buildReflection(reflection_opts);
				*/

				return NoError();
			}

			auto& GetProgram() { return *program; }
			const auto& GetProgram() const { return *program; }

			Error AllocateUniform(GLuint location, std::string_view name, GLenum type, std::size_t array_size)
			{
				auto [_, success] = uniforms.try_emplace(location, Uniform{ location, std::string(name), ConvertTypeToUniformData(type, array_size) });
				if (!success)
				{
					return StringError("Uniform location already exists");
				}
				/*
				if (auto found = uniforms.find(location); found == std::end(uniforms))
				{
					uniforms[location] = Uniform{ location, std::string(name), ConvertTypeToUniformData(type)};
				}
				else
				{
					return StringError("Uniform already allocated");
				}
				*/
				return NoError();
			}

			Expected<Uniform&> GetUniform(GLuint location)
			{
				if (auto found = uniforms.find(location); found != std::end(uniforms))
				{
					auto& uniform = found->second;
					return uniform;
				}
				else
				{
					return StringError("Uniform not found");
				}
			}

			const auto& GetUniformMapping()
			{
				return uniforms;
			}

			Error AllocateUniformBlock(GLuint index, std::string_view name)
			{
				uniform_blocks[index] = UniformBlock{ index, std::string(name) };
				return NoError();
			}

			Expected<UniformBlock&> GetUniformBlock(GLuint index)
			{
				if (auto found = uniform_blocks.find(index); found != std::end(uniform_blocks))
				{
					auto& uniform_block = found->second;
					return uniform_block;
				}
				else
				{
					return StringError("Uniform block not found");
				}
			}

			const auto& GetUniformBlockMapping()
			{
				return uniform_blocks;
			}

			Error AllocateAttrib(GLuint location, std::string_view name, GLenum type)
			{
				auto [_, success] = attribs.try_emplace(location, Attrib{ location, std::string(name), ConvertTypeToUniformData(type) });
				if (!success)
				{
					return StringError("Attrib location already exists");
				}
				return NoError();
			}

			Expected<Attrib&> GetAttrib(GLuint location)
			{
				if (auto found = attribs.find(location); found != std::end(attribs))
				{
					auto& attrib = found->second;
					return attrib;
				}
				else
				{
					return StringError("Uniform not found");
				}
			}

			const auto& GetAttribMapping()
			{
				return attribs;
			}

			Error SetLocationToTextureId(GLuint location, GLuint value)
			{
				active_location_to_texture_id[location] = value;
				active_texture_id_to_location[value] = location;
				return NoError();
			}

			const flat_hash_map<GLuint, GLuint>& GetLocationToTextureId()
			{
				return active_location_to_texture_id;
			}

			const auto& GetTextureIdToLocation() const { return active_texture_id_to_location; }

			Error BindActiveTextureUnitToName(GLenum active_texture_unit, std::string_view name);
			Error UnbindActiveTextureUnitToName(GLenum active_texture_unit);
			std::map<GLenum, std::string>& GetActiveTextureUnitToName() { return active_texture_unit_to_name; }

			Error BindTextureUnitToSampler(GLuint texture_id, GLuint sampler);
			Expected<GLuint> GetSamplerFromTextureUnit(GLuint texture_id);
			const auto& GetTextureUnitToSampler() const { return texture_unit_to_sampler_id; }

		private:
			GLuint id{};
			
			// Shader
			std::shared_ptr<VertexShader> vertex_shader;
			std::shared_ptr<FragmentShader> fragment_shader;
			std::unique_ptr<glslang::TProgram> program;

#ifndef NDEBUG
			std::unordered_map<GLuint, Uniform> uniforms;
			// index to block
			std::unordered_map<GLuint, UniformBlock> uniform_blocks;
			std::unordered_map<GLuint, Attrib> attribs;
#else
			flat_hash_map<GLuint, Uniform> uniforms;
			// index to block
			flat_hash_map<GLuint, UniformBlock> uniform_blocks;
			flat_hash_map<GLuint, Attrib> attribs;
#endif

			// TODO:
			// this is a hack to keep track of uniforms that are samplers
			// location -> texture id
			flat_hash_map<GLuint, GLuint> active_location_to_texture_id;
			flat_hash_map<GLuint, GLuint> active_texture_id_to_location;

			std::map<GLenum, std::string> active_texture_unit_to_name;

			flat_hash_map<GLuint, GLuint> texture_unit_to_sampler_id;
		};

		class ShaderManager
		{
		public:
			Error AllocateVertexShader(GLuint id);
			Error AllocateFragmentShader(GLuint id);
			Error DeleteShader(GLuint id);
			Error AllocateProgram(GLuint id);
			Error DeleteProgram(GLuint id);
			Error LinkVertexShaderToProgram(GLuint id, GLuint shader_id);
			Error LinkFragmentShaderToProgram(GLuint id, GLuint shader_id);

			Expected<std::shared_ptr<VertexShader>> GetVertexShader(GLuint id);
			Expected<std::shared_ptr<FragmentShader>> GetFragmentShader(GLuint id);
			Expected<Program&> GetProgram(GLuint id);
			flat_hash_map<GLuint, Program>& GetPrograms() { return programs; }

			static ShaderManager& Get()
			{
				static ShaderManager shader_manager;
				return shader_manager;
			}

		private:
			flat_hash_map<GLuint, std::shared_ptr<VertexShader>> vertex_shaders;
			flat_hash_map<GLuint, std::shared_ptr<FragmentShader>> fragment_shaders;
			flat_hash_map<GLuint, Program> programs;
		};
	}
}