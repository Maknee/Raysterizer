#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct FrameBufferCreateInfo
	{
		CMShared<RenderPass> render_pass;
		std::vector<CMShared<ImageView>> attachments;
		uint32_t width;
		uint32_t height;
		uint32_t layers;
	};

	struct FrameBuffer
	{
		vk::Framebuffer frame_buffer{};
		FrameBufferCreateInfo frame_buffer_create_info{};

	public:
		vk::Framebuffer operator*() const
		{
			return frame_buffer;
		}

		operator vk::Framebuffer() noexcept
		{
			return frame_buffer;
		}

	public:
		bool operator==(const FrameBuffer& other) const noexcept {
			return frame_buffer == other.frame_buffer;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<FrameBuffer>
	{
		size_t operator()(const FrameBuffer& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.frame_buffer));
			return h;
		}
	};

}
