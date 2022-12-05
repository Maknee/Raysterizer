#include "opengl_shader_manager.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		Error Program::BindActiveTextureUnitToName(GLenum active_texture_unit, std::string_view name)
		{
			auto actual_active_texture_unit = GL_TEXTURE0 + active_texture_unit;
			active_texture_unit_to_name[actual_active_texture_unit] = std::string(name);
			/*
			auto [_, success] = active_texture_unit_to_name.try_emplace(actual_active_texture_unit, std::string(name));
			if (!success)
			{
				// TODO: actually just overwrite
				//return StringError("Texture already exists");
			}
			*/
			return NoError();
		}

		Error Program::UnbindActiveTextureUnitToName(GLenum active_texture_unit)
		{
			auto num_erased = active_texture_unit_to_name.erase(active_texture_unit);

			// Applications may call unbind twice, doesn't matter...
			/*
			if (num_erased == 0)
			{
				return StringError("Texture unit does not exist");
			}
			*/

			return NoError();
		}

		Error Program::BindTextureUnitToSampler(GLuint texture_id, GLuint sampler)
		{
			texture_unit_to_sampler_id[texture_id] = sampler;
			return NoError();
		}

		Expected<GLuint> Program::GetSamplerFromTextureUnit(GLuint texture_id)
		{
			if (auto found = texture_unit_to_sampler_id.find(texture_id); found != std::end(texture_unit_to_sampler_id))
			{
				return found->second;
			}
			return StringError("Not found texture id", texture_id);
		}

		Error ShaderManager::AllocateVertexShader(GLuint id)
		{
			auto [_, success] = vertex_shaders.try_emplace(id, std::make_shared<VertexShader>(id));
			if (!success)
			{
				return StringError("Failed to insert vertex shader");
			}
			return NoError();
		}

		Error ShaderManager::AllocateFragmentShader(GLuint id)
		{
			auto [_, success] = fragment_shaders.try_emplace(id, std::make_shared<FragmentShader>(id));
			if (!success)
			{
				return StringError("Failed to insert fragment shader");
			}
			return NoError();
		}

		Error ShaderManager::DeleteShader(GLuint id)
		{
			if (vertex_shaders.erase(id) != 0)
			{
				return NoError();
			}
			else if (fragment_shaders.erase(id) != 0)
			{
				return NoError();
			}
			return StringError("Failed to insert fragment shader");
		}

		Error ShaderManager::AllocateProgram(GLuint id)
		{
			auto [_, success] = programs.try_emplace(id, Program{ id });
			if (!success)
			{
				return StringError("Failed to allocate program");
			}
			return NoError();
		}

		Error ShaderManager::DeleteProgram(GLuint id)
		{
			auto success = programs.erase(id);
			if (!success)
			{
				return StringError("Failed to delete program");
			}
			return NoError();
		}

		Error ShaderManager::LinkVertexShaderToProgram(GLuint id, GLuint shader_id)
		{
			if (auto found = programs.find(id); found != std::end(programs))
			{
				auto& program = found->second;
				
				// Make sure shader exists
				if (auto shader_or_err = GetVertexShader(shader_id))
				{
					auto& shader = *shader_or_err;
					program.SetVertexShader(shader);
				}
				else
				{
					return shader_or_err.takeError();
				}

			}
			return NoError();
		}

		Error ShaderManager::LinkFragmentShaderToProgram(GLuint id, GLuint shader_id)
		{
			if (auto found = programs.find(id); found != std::end(programs))
			{
				auto& program = found->second;

				// Make sure shader exists
				if (auto shader_or_err = GetFragmentShader(shader_id))
				{
					auto& shader = *shader_or_err;
					program.SetFragmentShader(shader);
				}
				else
				{
					return shader_or_err.takeError();
				}
			}
			return NoError();
		}

		Expected<std::shared_ptr<VertexShader>> ShaderManager::GetVertexShader(GLuint id)
		{
			if (auto found = vertex_shaders.find(id); found != std::end(vertex_shaders))
			{
				auto& shader = found->second;
				return shader;
			}
			return StringError("Vertex shader not found");
		}

		Expected<std::shared_ptr<FragmentShader>> ShaderManager::GetFragmentShader(GLuint id)
		{
			if (auto found = fragment_shaders.find(id); found != std::end(fragment_shaders))
			{
				auto& shader = found->second;
				return shader;
			}
			return StringError("Fragment shader not found");
		}

		Expected<Program&> ShaderManager::GetProgram(GLuint id)
		{
			if (auto found = programs.find(id); found != std::end(programs))
			{
				auto& program = found->second;
				return program;
			}
			return StringError("Program not found");
		}
	}
}