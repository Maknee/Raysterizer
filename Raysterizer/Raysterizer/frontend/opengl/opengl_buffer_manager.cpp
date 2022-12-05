#include "opengl_state.h"
#include "opengl_buffer_manager.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		void BufferObject::SetPointerView(BlockHashedPointerView pointer_view_)
		{
			pointer_view = std::move(pointer_view_);
		}

		void BufferObject::SetMappedPointer(void* mapped_pointer_, GLbitfield access, GLintptr offset_, GLsizeiptr length_)
		{
			mapped_pointer = mapped_pointer_;
			mapped_access = access;
			mapped_offset = offset_;
			if (length_ == 0)
			{
				mapped_size = pointer_view.GetTotalSize();
			}
			else
			{
				mapped_size = length_;
			}
		}

		Error BufferObject::SyncMappedPointerWithPointerView(bool reset_mapped_pointer)
		{
			if (!mapped_pointer)
			{
				return StringError("Mapped pointer is null");
			}

			const static bool opengl_render = Config["raysterizer"]["opengl"]["render"];
			if (opengl_render && pointer_view)
			{
				auto* mapped_data = reinterpret_cast<uint8_t*>(mapped_pointer);

				auto* view_data = pointer_view.GetDataAs<uint8_t*>();
				auto* view_data_offseted = view_data + mapped_offset;

				memcpy(view_data_offseted, mapped_data, mapped_size);
			}

			if (reset_mapped_pointer)
			{
				mapped_pointer = nullptr;
			}
			return NoError();
		}

		Expected<VertexAttribPointer&> VertexArrayObject::GetVertexAttribPointer(GLuint index)
		{
			if (index == Raysterizer::OpenGL::UNDEFINED_ID)
			{
				return StringError("Invalid vertex attribute pointer index {}", index);
			}

			if (vertex_attrib_pointers.size() <= index)
			{
				vertex_attrib_pointers.resize(index + 1);
			}

			VertexAttribPointer& vertex_attrib_pointer = vertex_attrib_pointers[index];

			return vertex_attrib_pointer;
		}

		void BufferObjectContainingData::SetPointerView(BlockHashedPointerView pointer_view_)
		{
			ScopedCPUProfileOpenGLCurrentFunction();

			auto total_size = pointer_view_.GetTotalSize();

			// Has nothing
			if (total_size == 0)
			{
				return;
			}
			if (!pointer_view_)
			{
				if (data.size() != total_size)
				{
					data.resize(total_size);
				}
				pointer_view_ = BlockHashedPointerView(data);
			}
			else
			{
				if (pointer_view_ != pointer_view)
				{
					static bool opengl_render = Config["raysterizer"]["opengl"]["render"];

					if (data.size() != total_size)
					{
						data.resize(total_size);
					}

					if (opengl_render)
					{
						// Overwrite every data and sync
						if (pointer_view_.GetData() != pointer_view.GetData())
						{
							pointer_view_.CopyBytesInto(data.data(), total_size);
						}
						pointer_view_ = BlockHashedPointerView(data.data(), pointer_view_.GetStride(), pointer_view_.GetNumElements());
					}
					else
					{
						// pointer view is pointing to data already, just recompute hash
						if (pointer_view_.GetData() != pointer_view.GetData())
						{
							pointer_view_.CopyBytesInto(data.data(), total_size);
						}
						pointer_view_ = BlockHashedPointerView(data.data(), pointer_view_.GetStride(), pointer_view_.GetNumElements());
					}
				}
			}
			BufferObject::SetPointerView(pointer_view_);
		}

		Error BufferManager::DeleteBuffer(GLuint id)
		{
			if (auto buffer = vbos.find(id); buffer != std::end(vbos))
			{
				vbos.erase(buffer);
				return NoError();
			}
			return StringError("VBO does not exist");
		}

		Error BufferManager::DeleteVAO(GLuint id)
		{
			if (auto buffer = vaos.find(id); buffer != std::end(vaos))
			{
				vaos.erase(buffer);
				return NoError();
			}
			return StringError("VAO does not exist");
		}

		Error BufferManager::ConvertVertexBufferObjectToElementBufferObject(GLuint id)
		{
			if (auto buffer_or_err = GetBuffer(id))
			{
				auto& buffer = *buffer_or_err;
				if (auto vbo = std::get_if<VertexBufferObject>(&buffer))
				{
					vbos[id] = ElementBufferObject{ id, *vbo };
					return NoError();
				}
				else
				{
					// This means that we have already converted
					return NoError();
				}
			}
			else
			{
				return buffer_or_err.takeError();
			}
		}

		Error BufferManager::ConvertVertexBufferObjectToUniformBufferObject(GLuint id)
		{
			if (auto buffer_or_err = GetBuffer(id))
			{
				auto& buffer = *buffer_or_err;
				if (auto vbo = std::get_if<VertexBufferObject>(&buffer))
				{
					vbos[id] = UniformBufferObject{ id, *vbo };
					return NoError();
				}
				else
				{
					// This means that we have already converted
					return NoError();
				}
			}
			else
			{
				return buffer_or_err.takeError();
			}
		}

		Expected<Buffer&> BufferManager::GetBuffer(GLuint id)
		{
			if (auto buffer = vbos.find(id); buffer != std::end(vbos))
			{
				return buffer->second;
			}
			else
			{
				return StringError("Buffer not found");
			}
		}

		Expected<VertexBufferObject&> BufferManager::GetVBO(GLuint id)
		{
			if (auto buffer_or_err = GetBuffer(id))
			{
				auto& buffer = *buffer_or_err;
				if (auto vbo = std::get_if<VertexBufferObject>(&buffer))
				{
					return *vbo;
				}
				else
				{
					return StringError("Not a vertex buffer object");
				};
			}
			else
			{
				return StringError("Buffer not found");
			}
		}

		Expected<VertexArrayObject&> BufferManager::GetVAO(GLuint id)
		{
			if (auto buffer = vaos.find(id); buffer != std::end(vaos))
			{
				return buffer->second;
			}
			else
			{
				return StringError("Buffer not found");
			}
		}

		Error BufferManager::BindVBOToVAO(GLuint vao_id, GLuint vbo_id)
		{
			if (auto vao_or_err = GetVAO(vao_id))
			{
				auto& vao = *vao_or_err;
				vao.SetBoundVBOID(vbo_id);
			}
			else
			{
				return vao_or_err.takeError();
			}
			return NoError();
		}

		Error BufferManager::BindEBOToVAO(GLuint vao_id, GLuint ebo_id)
		{
			if (auto vao_or_err = GetVAO(vao_id))
			{
				auto& vao = *vao_or_err;
				vao.SetBoundEBOID(ebo_id);
			}
			else
			{
				return vao_or_err.takeError();
			}
			return NoError();
		}

		Expected<Buffer&> BufferManager::GetVBOBoundToVAO(GLuint id)
		{
			if (auto vao_or_err = GetVAO(id))
			{
				auto& vao = *vao_or_err;
				if (auto vbo_or_err = GetBuffer(vao.GetBoundVBOID()))
				{
					auto& vbo = *vbo_or_err;
					return vbo;
				}
				else
				{
					return vbo_or_err.takeError();
				}
			}
			else
			{
				return vao_or_err.takeError();
			}
		}

		Expected<Buffer&> BufferManager::GetEBOBoundToVAO(GLuint id)
		{
			if (auto vao_or_err = GetVAO(id))
			{
				auto& vao = *vao_or_err;
				if (auto ebo_or_err = GetBuffer(vao.GetBoundEBOID()))
				{
					auto& ebo = *ebo_or_err;
					return ebo;
				}
				else
				{
					return ebo_or_err.takeError();
				}
			}
			else
			{
				return vao_or_err.takeError();
			}
		}

		std::vector<UniformBufferBindingPoint>& BufferManager::GetUniformBufferBindingPoints()
		{
			return uniform_buffer_binding_points;
		}

		const std::vector<UniformBufferBindingPoint>& BufferManager::GetUniformBufferBindingPoints() const
		{
			return uniform_buffer_binding_points;
		}
		
		UniformBufferBindingPoint& BufferManager::GetUniformBufferBindingPoint(GLuint index)
		{
			if (uniform_buffer_binding_points.size() <= index)
			{
				uniform_buffer_binding_points.resize(index + 1);
			}
			return uniform_buffer_binding_points[index];
		}

		Expected<UniformBufferObject&> BufferManager::GetUBO(GLuint id)
		{
			if (auto buffer_or_err = GetBuffer(id))
			{
				auto& buffer = *buffer_or_err;
				if (auto ubo = std::get_if<UniformBufferObject>(&buffer))
				{
					return *ubo;
				}
				else
				{
					return StringError("Not a uniform buffer object");
				}
			}
			else
			{
				return buffer_or_err.takeError();
			}
		}
	}
}