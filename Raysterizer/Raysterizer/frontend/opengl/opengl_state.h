#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		class State
		{
		public:
			Expected<GLuint> GetOpenGLIdFromTarget(GLenum target);

			Error SetActiveVertexArrayObject(GLuint id);
			Expected<VertexArrayObject&> GetActiveVertexArrayObject();
			
			Error SetActiveVertexBufferObject(GLuint id);
			Expected<VertexBufferObject&> GetActiveVertexBufferObject();

			Error SetActiveElementBufferObject(GLuint id);
			Expected<ElementBufferObject&> GetActiveElementBufferObject();

			Error SetActiveUniformBufferObject(GLuint id);
			Expected<UniformBufferObject&> GetActiveUniformBufferObject();

			Error SetActiveProgram(GLuint id);
			Expected<Program&> GetActiveProgram();

			Expected<VertexShader&> GetActiveVertexShader();
			Expected<FragmentShader&> GetActiveFragmentShader();

			Error SetActiveTextureUnit(GLenum id);
			Expected<GLuint> GetActiveTextureUnit();

			Error SetActiveTextureId(GLuint id);
			Expected<GLuint> GetActiveTextureId();

			Error SetActiveFrameBufferObjectId(GLuint id);
			Expected<GLuint> GetActiveFrameBufferObjectId();

			Expected<VertexBufferObject&> GetVertexBufferObjectBoundToVAO();
			Expected<VertexBufferObject&> GetNonDivisorVertexBufferObjectBoundToVAO();
			Expected<ElementBufferObject&> GetElementBufferObjectBoundToVAO();

			//Error MapUniformLocationToProgramId(GLuint uniform_location, GLuint program_id);
			//Expected<GLuint&> GetProgramIdFromUniformLocation();



			void SetHasDrawnThisFrame(bool has_drawn_this_frame_)
			{
				has_drawn_this_frame = has_drawn_this_frame_;
			}

			bool GetHasDrawnThisFrame() const
			{
				return has_drawn_this_frame;
			}

			struct Viewport
			{
				GLint x;
				GLint y;
				GLsizei width;
				GLsizei height;
			};

			void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
			{
				view_port = Viewport{ x, y, width, height };
			}
			
			Viewport GetViewport() const
			{
				return view_port;
			}

			bool IsViewportValid() const
			{
				if (view_port.width < (WINDOW_WIDTH * 0.25f) || view_port.height < (WINDOW_HEIGHT * 0.25f))
				{
					return false;
				}
				return true;
			}

			void SetClearColor(glm::vec4 c)
			{
				raysterizer_info.clear_color = c;
			}

			void SetZNear(float z_near)
			{
				raysterizer_info.z_near = z_near;
			}

			void SetZFar(float z_far)
			{
				raysterizer_info.z_far = z_far;
			}

			void SetPerformPerspectiveCorrection(bool perform_perspective_correction_)
			{
				perform_perspective_correction = perform_perspective_correction_;
			}

			bool GetPerformPerspectiveCorrection() const
			{
				return perform_perspective_correction;
			}

			static State& Get()
			{
				static State state;
				return state;
			};

			struct RaysterizerInfo
			{
				glm::mat4 projection_view{ 1.0f };
				glm::mat4 projection_view_inverse{ 1.0f };
				glm::vec4 clear_color{ 0.0f, 0.0f, 0.0f, 1.0f };
				float z_near{ 0.0f };
				float z_far{ 1000.0f };

				float aperture = 1.0f;
				float focus_distance = 1.0f;
				float heatmap_scale = 1.0f;
				uint total_number_of_samples = 1;
				uint number_of_samples = 1;
				uint number_of_bounces = 16;
				uint random_seed = 0;
				bool has_sky = true;
				bool show_heatmap = false;

				glm::vec4 camera_position;
			};

			RaysterizerInfo& GetRaysterizerInfo() { return raysterizer_info; }
			const RaysterizerInfo& GetRaysterizerInfo() const { return raysterizer_info; }

			void AddMarkedDeleteBuffers(GLuint id) { marked_delete_buffers.emplace(id); }
			std::vector<GLuint> GetAndFlushMarkedDeleteBuffers()
			{
				std::vector<GLuint> marked_delete_buffers_copy(std::begin(marked_delete_buffers), std::end(marked_delete_buffers));
				marked_delete_buffers.clear();
				return marked_delete_buffers_copy;
			}

		private:
			GLuint active_vertex_array_object_id = UNDEFINED_ID;
			GLuint active_vertex_buffer_object_id = UNDEFINED_ID;
			GLuint active_element_buffer_object_id = UNDEFINED_ID;
			GLuint active_uniform_buffer_object_id = UNDEFINED_ID;

			GLuint active_program_id = UNDEFINED_ID;

			GLuint active_texture_unit = UNDEFINED_ID;
			GLuint active_texture_id = UNDEFINED_ID;

			GLuint active_frame_buffer_object_id = DEFAULT_FRAME_BUFFER_OBJECT_ID;

			RaysterizerInfo raysterizer_info{};
			bool has_drawn_this_frame = false;

			Viewport view_port{};
			bool perform_perspective_correction = false;

			phmap::flat_hash_set<GLuint> marked_delete_buffers;
		};
	}
}