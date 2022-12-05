#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		struct FrameBufferObject
		{
			GLuint color_attachment_texture{};
			GLuint depth_attachment_texture{};
		};

		class FrameBufferManager
		{
		public:
			explicit FrameBufferManager();
			Error AllocateFrameBufferObject(GLuint id);
			Error DeallocateFrameBufferObject(GLuint id);
			Expected<FrameBufferObject&> GetFrameBufferObject(GLuint id);

			static FrameBufferManager& Get()
			{
				static FrameBufferManager m;
				return m;
			}

		private:
			flat_hash_map<GLuint, FrameBufferObject> id_to_fbo;
		};
	}
}
