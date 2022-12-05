#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		class BufferObject
		{
		public:
			explicit BufferObject() = default;
			explicit BufferObject(GLuint id_) :
				id(id_)
			{
			}

			const GLuint GetId() { return id; }
			
			void SetPointerView(BlockHashedPointerView pointer_view_);

			BlockHashedPointerView& GetPointerView() { return pointer_view; }
			const BlockHashedPointerView& GetPointerView() const { return pointer_view; }

			void SetMappedPointer(void* mapped_pointer_, GLbitfield access, GLintptr offset_ = 0, GLsizeiptr length_ = 0);
			Error SyncMappedPointerWithPointerView(bool reset_mapped_pointer = true);

		protected:
			GLuint id{};
			BlockHashedPointerView pointer_view{};
			void* mapped_pointer{};
			GLbitfield mapped_access{};
			GLintptr mapped_offset{};
			GLsizeiptr mapped_size{};
		};

		struct VertexAttribPointer
		{
			std::size_t GetTypeToSize() const { return Raysterizer::OpenGL::Util::GLenumToSize(type); }
			std::size_t GetNumElements() const { return size; }
			std::size_t GetTotalSize() const { return GetTypeToSize() * GetNumElements(); }
			std::size_t GetStride() const
			{
				if (total_size != 0)
				{
					return total_size;
				}
				else
				{
					return GetTotalSize();
				}
			}

		public:
			GLuint index{};
			GLuint size{};
			GLenum type{};
			bool normalized{};
			GLuint total_size{};
			GLuint offset{};
			
			bool enabled{};
			GLuint divisor{};

			GLuint associated_vbo{};
		};

		class VertexArrayObject : public BufferObject
		{
		public:
			explicit VertexArrayObject() :
				BufferObject()
			{
			}
			explicit VertexArrayObject(GLuint id_) :
				BufferObject(id_)
			{
			}

			/*
			Error AddVertexAttribPointer(
				GLuint index,
				GLuint size,
				GLenum type,
				bool normalized,
				GLuint total_size,
				GLuint offset
			)
			{
				if (vertex_attrib_pointers.size() <= index)
				{
					vertex_attrib_pointers.resize(index + 1);
				}
				VertexAttribPointer& vertex_attrib_pointer = vertex_attrib_pointers[index];
				vertex_attrib_pointer.index = index;
				vertex_attrib_pointer.size = Raysterizer::OpenGL::Util::GLenumToSize(type) * size;
				vertex_attrib_pointer.type = type;
				vertex_attrib_pointer.normalized = normalized;
				vertex_attrib_pointer.total_size = total_size;
				vertex_attrib_pointer.offset = offset;
				vertex_attrib_pointer.enabled = false;

				return NoError();
			}
			*/
			
			Expected<VertexAttribPointer&> GetVertexAttribPointer(GLuint index);
			const std::vector<VertexAttribPointer>& GetVertexAttribPointers() { return vertex_attrib_pointers; }

			void SetBoundVBOID(GLuint vbo_id) { bound_vbo_id = vbo_id; }
			GLuint GetBoundVBOID() { return bound_vbo_id; }
			void SetBoundEBOID(GLuint ebo_id) { bound_ebo_id = ebo_id; }
			GLuint GetBoundEBOID() { return bound_ebo_id; }

		private:
			std::vector<VertexAttribPointer> vertex_attrib_pointers;
			GLuint bound_vbo_id;
			GLuint bound_ebo_id;
		};


		class BufferObjectContainingData : public BufferObject
		{
		public:
			explicit BufferObjectContainingData() :
				BufferObject()
			{
			}

			explicit BufferObjectContainingData(GLuint id_) :
				BufferObject(id_)
			{
			}

			void SetPointerView(BlockHashedPointerView pointer_view_);
			void SetUsage(GLenum usage_) { usage = usage_; }
			std::vector<uint8_t>& GetData() { return data; }
			const std::vector<uint8_t>& GetData() const { return data; }

		protected:
			std::vector<uint8_t> data;
			GLenum usage{};
		};

		class ElementBufferObject : public BufferObjectContainingData
		{
		public:
			explicit ElementBufferObject() :
				BufferObjectContainingData()
			{
			}
			explicit ElementBufferObject(GLuint id_) :
				BufferObjectContainingData(id_)
			{
			}

			explicit ElementBufferObject(GLuint id_, BufferObjectContainingData buffer_object_containing_data) :
				BufferObjectContainingData(std::move(buffer_object_containing_data))
			{
				id = id_;
			}

			void SetChangedDataStride(bool changed_data_stride_) { changed_data_stride = changed_data_stride_; }
			bool GetChangedDataStride() { return changed_data_stride; }

		private:
			bool changed_data_stride = false;
		};

		class VertexBufferObject : public BufferObjectContainingData
		{
		public:
			explicit VertexBufferObject() :
				BufferObjectContainingData()
			{
			}
			explicit VertexBufferObject(GLuint id_) :
				BufferObjectContainingData(id_)
			{
			}

			explicit VertexBufferObject(GLuint id_, BufferObjectContainingData buffer_object_containing_data) :
				BufferObjectContainingData(std::move(buffer_object_containing_data))
			{
				id = id_;
			}

			ElementBufferObject& GetElementBufferObject() { return element_buffer_object; }
			void SetElementBufferFirst(std::size_t element_buffer_first_) { element_buffer_first = element_buffer_first_; }
			auto GetElementBufferFirst() { return element_buffer_first; }
			void SetElementBufferCount(std::size_t element_buffer_count_) { element_buffer_count = element_buffer_count_; }
			auto GetElementBufferCount() { return element_buffer_count; }
		private:
			// used for glDrawArrays when element buffer doesn't exist and we need one anyways that goes from [0..n]
			ElementBufferObject element_buffer_object;
			std::size_t element_buffer_first = 0;
			std::size_t element_buffer_count = 0;
		};

		class UniformBufferObject : public BufferObjectContainingData
		{
		public:
			explicit UniformBufferObject() :
				BufferObjectContainingData()
			{
			}

			explicit UniformBufferObject(GLuint id_) :
				BufferObjectContainingData(id_)
			{
			}

			explicit UniformBufferObject(GLuint id_, BufferObjectContainingData buffer_object_containing_data) :
				BufferObjectContainingData(std::move(buffer_object_containing_data))
			{
				id = id_;
			}

		private:
		};

		class UniformBufferBindingPoint
		{
		public:
			explicit UniformBufferBindingPoint() = default;
			explicit UniformBufferBindingPoint(GLuint index_) :
				index(index_)
			{
			}

			void SetBoundUBOID(GLuint ubo_id) { bound_ubo_id = ubo_id; }
			GLuint GetBoundUBOID() { return bound_ubo_id; }
			void SetBoundUBOIDOffset(GLintptr offset) { bound_ubo_offset = offset; }
			GLintptr GetBoundUBOIDOffset() { return bound_ubo_offset; }
			void SetBoundUBOIDSize(GLsizeiptr size) { bound_ubo_size = size; }
			GLsizeiptr GetBoundUBOIDSize() { return bound_ubo_size; }

		private:
			GLuint index;
			GLuint bound_ubo_id;
			GLintptr bound_ubo_offset;
			GLsizeiptr bound_ubo_size;
		};

		using Buffer = std::variant<
			//VertexArrayObject,
			VertexBufferObject,
			ElementBufferObject,
			UniformBufferObject
		>;

		class BufferManager
		{
		public:
			template<typename T, typename... Args>
			Error AllocateBuffer(GLuint id, Args&&... args)
			{
				T buffer = T{ id, std::forward<Args>(args)... };
				if constexpr(std::is_same_v<T, VertexArrayObject>)
				{
					auto [_, success] = vaos.try_emplace(id, buffer);
					if (!success)
					{
						return StringError("Buffer already exists");
					}
				}
				else if constexpr(std::is_same_v<T, VertexBufferObject>)
				{
					auto [_, success] = vbos.try_emplace(id, buffer);
					if (!success)
					{
						return StringError("Buffer already exists");
					}
				}
				else if constexpr(std::is_same_v<T, ElementBufferObject>)
				{
					auto [_, success] = vbos.try_emplace(id, buffer);
					if (!success)
					{
						return StringError("Buffer already exists");
					}
				}
				else if constexpr (std::is_same_v<T, UniformBufferObject>)
				{
					auto [_, success] = ubos.try_emplace(id, buffer);
					if (!success)
					{
						return StringError("Buffer already exists");
					}
				}

				return NoError();
			}

			Error DeleteBuffer(GLuint id);
			Error DeleteVAO(GLuint id);

			Error ConvertVertexBufferObjectToElementBufferObject(GLuint id);
			Error ConvertVertexBufferObjectToUniformBufferObject(GLuint id);
			Expected<Buffer&> GetBuffer(GLuint id);
			Expected<VertexBufferObject&> GetVBO(GLuint id);
			Expected<VertexArrayObject&> GetVAO(GLuint id);
			Error BindVBOToVAO(GLuint vao_id, GLuint vbo_id);
			Error BindEBOToVAO(GLuint vao_id, GLuint ebo_id);
			Expected<Buffer&> GetVBOBoundToVAO(GLuint id);
			Expected<Buffer&> GetEBOBoundToVAO(GLuint id);

			std::vector<UniformBufferBindingPoint>& GetUniformBufferBindingPoints();
			const std::vector<UniformBufferBindingPoint>& GetUniformBufferBindingPoints() const;
			UniformBufferBindingPoint& GetUniformBufferBindingPoint(GLuint index);
			Expected<UniformBufferObject&> GetUBO(GLuint id);

			static BufferManager& Get()
			{
				static BufferManager c;
				return c;
			};

		private:
			flat_hash_map<GLuint, VertexArrayObject> vaos;
			flat_hash_map<GLuint, Buffer> vbos;
			
			std::vector<UniformBufferBindingPoint> uniform_buffer_binding_points;
		};
	}
}