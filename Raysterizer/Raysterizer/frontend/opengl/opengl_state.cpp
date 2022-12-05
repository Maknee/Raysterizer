#include "opengl_state.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		Expected<GLuint> State::GetOpenGLIdFromTarget(GLenum target)
		{
			switch (target)
			{
			case GL_ARRAY_BUFFER:
				return active_vertex_buffer_object_id;
			case GL_ELEMENT_ARRAY_BUFFER:
				return active_element_buffer_object_id;
			case GL_UNIFORM_BUFFER:
				return active_uniform_buffer_object_id;
			default:
				return StringError("Unknown target: {}", target);
			}
		}

		Error State::SetActiveVertexArrayObject(GLuint id)
		{
			/*
			if (active_vertex_array_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			*/

			active_vertex_array_object_id = id;
			return NoError();
		}

		Expected<VertexArrayObject&> State::GetActiveVertexArrayObject()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (active_vertex_array_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			if(auto vao_or_err = buffer_manager.GetVAO(active_vertex_array_object_id))
			{
				auto& vao = *vao_or_err;
				return vao;
			} 
			else
			{
				return vao_or_err.takeError();
			}
		}

		Error State::SetActiveVertexBufferObject(GLuint id)
		{
			/*
			if (active_vertex_buffer_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			*/

			active_vertex_buffer_object_id = id;
			return NoError();
		}

		Expected<VertexBufferObject&> State::GetActiveVertexBufferObject()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (active_vertex_buffer_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			if (auto buffer_or_err = buffer_manager.GetBuffer(active_vertex_buffer_object_id))
			{
				auto& buffer = *buffer_or_err;
				if (auto vertex_buffer_object = std::get_if<VertexBufferObject>(&buffer))
				{
					return *vertex_buffer_object;
				}
				else
				{
					return StringError("Buffer is not array buffer");
				}
			}
			else
			{
				return buffer_or_err.takeError();
			}
		}

		Error State::SetActiveElementBufferObject(GLuint id)
		{
			/*
			if (active_element_buffer_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			*/

			active_element_buffer_object_id = id;
			if (auto vao_or_err = GetActiveVertexArrayObject())
			{
				auto& vao = *vao_or_err;
				vao.SetBoundEBOID(active_element_buffer_object_id);
			}
			else
			{
				llvm::consumeError(vao_or_err.takeError());
			}

			return NoError();
		}

		Expected<ElementBufferObject&> State::GetActiveElementBufferObject()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (active_element_buffer_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			if (auto buffer_or_err = buffer_manager.GetBuffer(active_element_buffer_object_id))
			{
				auto& buffer = *buffer_or_err;
				if (auto element_array_buffer = std::get_if<ElementBufferObject>(&buffer))
				{
					return *element_array_buffer;
				}
				else
				{
					return StringError("Buffer is not array buffer");
				}
			}
			else
			{
				return buffer_or_err.takeError();
			}
		}

		Error State::SetActiveUniformBufferObject(GLuint id)
		{
			active_uniform_buffer_object_id = id;
			return NoError();
		}

		Expected<UniformBufferObject&> State::GetActiveUniformBufferObject()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (active_uniform_buffer_object_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active buffer");
			}
			if (auto buffer_or_err = buffer_manager.GetBuffer(active_uniform_buffer_object_id))
			{
				auto& buffer = *buffer_or_err;
				if (auto uniform_buffer_object = std::get_if<UniformBufferObject>(&buffer))
				{
					return *uniform_buffer_object;
				}
				else
				{
					return StringError("Buffer is not array buffer");
				}
			}
			else
			{
				return buffer_or_err.takeError();
			}
		}

		Error State::SetActiveProgram(GLuint id)
		{
			active_program_id = id;
			return NoError();
		}

		Expected<Program&> State::GetActiveProgram()
		{
			auto& shader_manager = Raysterizer::OpenGL::ShaderManager::Get();
			if (active_program_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active program");
			}
			if (auto program_or_err = shader_manager.GetProgram(active_program_id))
			{
				auto& program = *program_or_err;
				return program;
			}
			else
			{
				return program_or_err.takeError();
			}
		}

		Expected<VertexShader&> State::GetActiveVertexShader()
		{
			auto& shader_manager = Raysterizer::OpenGL::ShaderManager::Get();
			if (active_program_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active program");
			}
			if (auto program_or_err = GetActiveProgram())
			{
				auto& program = *program_or_err;
				return program.GetVertexShader();
			}
			else
			{
				return program_or_err.takeError();
			}
		}

		Expected<FragmentShader&> State::GetActiveFragmentShader()
		{
			auto& shader_manager = Raysterizer::OpenGL::ShaderManager::Get();
			if (active_program_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active program");
			}
			if (auto program_or_err = GetActiveProgram())
			{
				auto& program = *program_or_err;
				return program.GetFragmentShader();
			}
			else
			{
				return program_or_err.takeError();
			}
		}

		Error State::SetActiveTextureUnit(GLenum id)
		{
			active_texture_unit = id;
			return NoError();
		}

		Expected<GLuint> State::GetActiveTextureUnit()
		{
			if (active_texture_unit == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active texture manager");
			}
			return active_texture_unit;
		}

		Error State::SetActiveTextureId(GLuint id)
		{
			active_texture_id = id;
			return NoError();
		}

		Expected<GLuint> State::GetActiveTextureId()
		{
			if (active_texture_id == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active texture manager");
			}
			return active_texture_id;
		}

		Error State::SetActiveFrameBufferObjectId(GLuint id)
		{
			active_frame_buffer_object_id = id;
			return NoError();
		}

		Expected<GLuint> State::GetActiveFrameBufferObjectId()
		{
			return active_frame_buffer_object_id;
		}

		Expected<VertexBufferObject&> State::GetVertexBufferObjectBoundToVAO()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (auto vao_or_err = GetActiveVertexArrayObject())
			{
				auto& vao = *vao_or_err;
				auto vbo_id = vao.GetBoundVBOID();
				if (auto buffer_or_err = buffer_manager.GetBuffer(vbo_id))
				{
					auto& buffer = *buffer_or_err;
					if (auto vertex_buffer_object = std::get_if<VertexBufferObject>(&buffer))
					{
						return *vertex_buffer_object;
					}
					else
					{
						return StringError("Buffer is not array buffer");
					}
				}
				else
				{
					return buffer_or_err.takeError();
				}
			}
			else
			{
				return vao_or_err.takeError();
			}
		}

		Expected<VertexBufferObject&> State::GetNonDivisorVertexBufferObjectBoundToVAO()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (auto vao_or_err = GetActiveVertexArrayObject())
			{
				auto& vao = *vao_or_err;
				const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

				GLuint vbo_id{};
				for (const auto& vap : vertex_attrib_pointers)
				{
					if (vap.divisor == 0)
					{
						vbo_id = vap.associated_vbo;
						break;
					}
				}

				if (vbo_id == 0)
				{
					return StringError("Should have at least one vbo without divisor");
				}

				if (auto buffer_or_err = buffer_manager.GetBuffer(vbo_id))
				{
					auto& buffer = *buffer_or_err;
					if (auto vertex_buffer_object = std::get_if<VertexBufferObject>(&buffer))
					{
						return *vertex_buffer_object;
					}
					else
					{
						return StringError("Buffer is not array buffer");
					}
				}
				else
				{
					return buffer_or_err.takeError();
				}
			}
			else
			{
				return vao_or_err.takeError();
			}
		}

		Expected<ElementBufferObject&> State::GetElementBufferObjectBoundToVAO()
		{
			auto& buffer_manager = Raysterizer::OpenGL::BufferManager::Get();
			if (auto vao_or_err = GetActiveVertexArrayObject())
			{
				auto& vao = *vao_or_err;
				auto ebo_id = vao.GetBoundEBOID();
				if (auto buffer_or_err = buffer_manager.GetBuffer(ebo_id))
				{
					auto& buffer = *buffer_or_err;
					if (auto element_array_buffer = std::get_if<ElementBufferObject>(&buffer))
					{
						return *element_array_buffer;
					}
					else
					{
						return StringError("Buffer is not array buffer");
					}
				}
				else
				{
					return buffer_or_err.takeError();
				}
			}
			else
			{
				return vao_or_err.takeError();
			}
		}

		/*
		Expected<Texture&> State::GetActiveTexture()
		{
			auto& texture_manager = Raysterizer::OpenGL::TextureManager::Get();
			if (active_texture_unit == UNDEFINED_ID)
			{
				return StringError("Didn't initialize active texture manager");
			}
			if (auto texture_or_err = texture_manager.GetTexture(active_texture_unit))
			{
				auto& texture = *texture_or_err;
				return texture;
			}
			else
			{
				return texture_or_err.takeError();
			}
		}
		*/
	}
}