#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct RenderPassCreateInfo
	{
	public:
		std::vector<vk::AttachmentDescription> attachment_descriptions;
		std::vector<vk::SubpassDescription> subpass_descriptions;
		std::vector<vk::SubpassDependency> subpass_dependencies;
		std::vector<vk::AttachmentReference> color_attachment_references;
		std::vector<vk::AttachmentReference> resolve_attachment_references;
		std::optional<vk::AttachmentReference> depth_stencil_reference;
		std::vector<size_t> color_attachment_reference_offsets;

		vk::RenderPassCreateInfo CreateVulkanRenderPassCreateInfo() const;

	public:
		bool operator==(const RenderPassCreateInfo& o) const noexcept {
			return attachment_descriptions == o.attachment_descriptions &&
				subpass_descriptions == o.subpass_descriptions &&
				subpass_dependencies == o.subpass_dependencies &&
				color_attachment_references == o.color_attachment_references &&
				resolve_attachment_references == o.resolve_attachment_references &&
				depth_stencil_reference == o.depth_stencil_reference &&
				color_attachment_reference_offsets == o.color_attachment_reference_offsets;
		}
	};

	struct RenderPass
	{
		vk::RenderPass render_pass{};
		RenderPassCreateInfo render_pass_create_info;

	public:
		vk::RenderPass operator*() const
		{
			return render_pass;
		}

		operator vk::RenderPass() noexcept
		{
			return render_pass;
		}

	public:
		bool operator==(const RenderPass& o) const noexcept {
			return render_pass == o.render_pass;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<RenderPassCreateInfo>
	{
		size_t operator()(const RenderPassCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.attachment_descriptions));
			HashCombine(h, Hash(o.subpass_descriptions));
			HashCombine(h, Hash(o.subpass_dependencies));
			HashCombine(h, Hash(o.color_attachment_references));
			HashCombine(h, Hash(o.resolve_attachment_references));
			if (o.depth_stencil_reference)
			{
				HashCombine(h, Hash(*o.depth_stencil_reference));
			}
			HashCombine(h, Hash(o.color_attachment_reference_offsets));
			return h;
		}
	};

	template<>
	struct hash<RenderPass>
	{
		size_t operator()(const RenderPass& o) const noexcept
		{
			return reinterpret_cast<size_t>(static_cast<VkRenderPass>(o.render_pass));
		}
	};
}
