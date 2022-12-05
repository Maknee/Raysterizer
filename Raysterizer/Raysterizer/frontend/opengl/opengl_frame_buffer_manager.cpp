#include "opengl_frame_buffer_manager.h"

namespace Raysterizer
{
	namespace OpenGL
	{
		FrameBufferManager::FrameBufferManager()
		{
		}

		Error FrameBufferManager::AllocateFrameBufferObject(GLuint id)
		{
			id_to_fbo.try_emplace(id, FrameBufferObject{});
			return NoError();
		}
		
		Error FrameBufferManager::DeallocateFrameBufferObject(GLuint id)
		{
			id_to_fbo.erase(id);
			return NoError();
		}
		
		Expected<FrameBufferObject&> FrameBufferManager::GetFrameBufferObject(GLuint id)
		{
			if (auto found = id_to_fbo.find(id); found != std::end(id_to_fbo))
			{
				return found->second;
			}
			return StringError("Could not find frame buffer object {}", id);
		}
	}
}