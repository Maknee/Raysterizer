#include "include/render_pass.h"

namespace RaysterizerEngine
{
	vk::RenderPassCreateInfo RenderPassCreateInfo::CreateVulkanRenderPassCreateInfo() const
	{
        /*
        for (auto& subpass_description : subpass_descriptions)
        {
            subpass_description
                .setColorAttachments(color_attachment_references)
                .setInputAttachments(nullptr)
                .setPreserveAttachments(nullptr);

            if (!resolve_attachment_references.empty())
            {
                subpass_description
                    .setResolveAttachments(resolve_attachment_references);
            }
            if (depth_stencil_reference)
            {
                subpass_description.setPDepthStencilAttachment(&*depth_stencil_reference);
            }
        }
        */

        auto render_pass_create_info = vk::RenderPassCreateInfo{}
            .setAttachments(attachment_descriptions)
            .setDependencies(subpass_dependencies)
            .setSubpasses(subpass_descriptions);

		return render_pass_create_info;
	}
}