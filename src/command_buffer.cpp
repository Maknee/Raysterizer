#include "include/command_buffer.h"

namespace RaysterizerEngine
{
	CommandBuffer::CommandBuffer()
	{

	}

	CommandBuffer::CommandBuffer(vk::CommandBuffer command_buffer_, CommandBufferCreateInfo command_buffer_create_info_) :
		command_buffer(std::move(command_buffer_)), command_buffer_create_info(std::move(command_buffer_create_info_))
	{

	}

	void CommandBuffer::Record(std::function<void(CommandBuffer&)> f, const vk::CommandBufferBeginInfo& command_buffer_begin_info)
	{
		if (end_recording)
		{
			PANIC("Command buffer has already finished recording!");
		}
		Begin(command_buffer_begin_info);
		f(*this);
	}

	void CommandBuffer::RecordAndEnd(std::function<void(CommandBuffer&)> f, const vk::CommandBufferBeginInfo& command_buffer_begin_info)
	{
		Record(f, command_buffer_begin_info);
		End();
	}

	void CommandBuffer::Reset()
	{
		if (command_buffer_create_info.command_pool->command_pool_create_info.flags & vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		{
			command_buffer.reset();
		}
		ResetInUse();
		begin_recording = false;
		end_recording = false;

		dependency_buffers.clear();
		dependency_textures.clear();
		dependency_descriptor_sets.clear();
		bottom_level_acceleration_structures.clear();
		top_level_acceleration_structures.clear();
		frame_buffers.clear();
	}

	void CommandBuffer::Begin(const vk::CommandBufferBeginInfo& command_buffer_begin_info)
	{
		if (!begin_recording)
		{
			command_buffer.begin(command_buffer_begin_info);
			begin_recording = true;
		}
	}

	void CommandBuffer::End()
	{
		if (begin_recording)
		{
			if (!end_recording)
			{
				command_buffer.end();
				end_recording = true;
			}
		}
	}

	void CommandBuffer::AddDependencyTo(CMShared<Buffer> buffer)
	{
		dependency_buffers.emplace_back(buffer);
	}

	void CommandBuffer::AddDependencyTo(const std::vector<CMShared<Buffer>>& buffers)
	{
		for (const auto& buffer : buffers)
		{
			AddDependencyTo(buffer);
		}
	}

	void CommandBuffer::AddDependencyTo(CMShared<Texture> texture)
	{
		dependency_textures.emplace_back(texture);
		dependency_images.emplace_back(texture->image);
		dependency_image_views.emplace_back(texture->image_view);
		dependency_samplers.emplace_back(texture->sampler);
	}

	void CommandBuffer::AddDependencyTo(const std::vector<CMShared<Texture>>& textures)
	{
		for (const auto& texture : textures)
		{
			AddDependencyTo(texture);
		}
	}

	void CommandBuffer::AddDependencyTo(CMShared<DescriptorSet> descriptor_set)
	{
		PANIC("Not supposed to be supported...");
		dependency_descriptor_sets.emplace_back(descriptor_set);
	}

	void CommandBuffer::AddDependencyTo(CMShared<BottomLevelAccelerationStructure> bottom_level_acceleration_structure)
	{
		bottom_level_acceleration_structures.emplace_back(bottom_level_acceleration_structure);
	}

	void CommandBuffer::AddDependencyTo(CMShared<TopLevelAccelerationStructure> top_level_acceleration_structure)
	{
		top_level_acceleration_structures.emplace_back(top_level_acceleration_structure);
	}

	void CommandBuffer::AddDependencyTo(CMShared<FrameBuffer> frame_buffer)
	{
		frame_buffers.emplace_back(frame_buffer);
	}

	Error CommandBuffer::InsertImageMemoryBarrier(CMShared<Image> image, vk::ImageLayout old_image_layout, vk::ImageLayout new_image_layout, vk::ImageSubresourceRange subresource_range, vk::PipelineStageFlags src_pipeline_stage_flags, vk::PipelineStageFlags dst_pipeline_stage_flags, vk::DependencyFlags dependency_flags)
	{
		auto image_memory_barrier = vk::ImageMemoryBarrier{}
			.setSrcQueueFamilyIndex({})
			.setDstQueueFamilyIndex({})
			.setOldLayout(old_image_layout)
			.setNewLayout(new_image_layout)
			.setImage(**image)
			.setSubresourceRange(subresource_range);

		switch (old_image_layout)
		{
		case vk::ImageLayout::eUndefined:
		{
			image_memory_barrier.setSrcAccessMask({});
			break;
		}
		case vk::ImageLayout::ePreinitialized:
		{
			image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eHostWrite);
			break;
		}
		case vk::ImageLayout::eColorAttachmentOptimal:
		{
			image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
			break;
		}
		case vk::ImageLayout::eDepthAttachmentOptimal:
		{
			image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
			break;
		}
		case vk::ImageLayout::eTransferSrcOptimal:
		{
			image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
			break;
		}
		case vk::ImageLayout::eTransferDstOptimal:
		{
			image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			break;
		}
		case vk::ImageLayout::eShaderReadOnlyOptimal:
		{
			image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
			break;
		}
		case vk::ImageLayout::eGeneral:
		{
			break;
		}
		default:
		{
			return StringError("Old image layout transition not supported: {}", vk::to_string(old_image_layout));
			break;
		}
		}

		switch (new_image_layout)
		{
		case vk::ImageLayout::eTransferDstOptimal:
		{
			image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			break;
		}
		case vk::ImageLayout::eTransferSrcOptimal:
		{
			image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
			break;
		}
		case vk::ImageLayout::eAttachmentOptimal:
		{
			image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
			break;
		}
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		{
			image_memory_barrier.setDstAccessMask(image_memory_barrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
			break;
		}
		case vk::ImageLayout::eShaderReadOnlyOptimal:
		{
			if (image_memory_barrier.srcAccessMask == vk::AccessFlags{})
			{
				image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
			}
			image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
			break;
		}
		case vk::ImageLayout::eGeneral:
		{
			break;
		}
		default:
		{
			return StringError("New image layout transition not supported: {}", vk::to_string(new_image_layout));
			break;
		}
		}
		
		command_buffer.pipelineBarrier(src_pipeline_stage_flags, dst_pipeline_stage_flags, dependency_flags, {}, {}, image_memory_barrier);

		return NoError();
	}

	Error CommandBuffer::InsertImageMemoryBarrier(vk::ImageMemoryBarrier image_memory_barrier, vk::PipelineStageFlags src_pipeline_stage_flags, vk::PipelineStageFlags dst_pipeline_stage_flags, vk::DependencyFlags dependency_flags)
	{
		command_buffer.pipelineBarrier(src_pipeline_stage_flags, dst_pipeline_stage_flags, dependency_flags, {}, {}, image_memory_barrier);

		return NoError();
	}

	Error CommandBuffer::SetImageLayout(CMShared<Image> image, vk::ImageLayout old_image_layout, vk::ImageLayout new_image_layout, vk::ImageSubresourceRange subresource_range, vk::PipelineStageFlags src_stage_mask, vk::PipelineStageFlags dst_stage_mask)
	{
		vk::ImageMemoryBarrier image_memory_barrier;

		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = **image;
		image_memory_barrier.subresourceRange = subresource_range;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (old_image_layout)
		{
		case vk::ImageLayout::eUndefined:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			image_memory_barrier.srcAccessMask = {};
			break;

		case vk::ImageLayout::ePreinitialized:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			break;

		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;

		case vk::ImageLayout::eTransferSrcOptimal:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			break;

		case vk::ImageLayout::eTransferDstOptimal:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			break;

		case vk::ImageLayout::eShaderReadOnlyOptimal:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (new_image_layout)
		{
		case vk::ImageLayout::eTransferDstOptimal:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			break;

		case vk::ImageLayout::eTransferSrcOptimal:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			break;

		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			image_memory_barrier.dstAccessMask = image_memory_barrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;

		case vk::ImageLayout::eShaderReadOnlyOptimal:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (image_memory_barrier.srcAccessMask == vk::AccessFlags{})
			{
				image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
			}
			image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Put barrier inside setup command buffer
		
		command_buffer.pipelineBarrier(
			src_stage_mask,
			dst_stage_mask,
			{},
			{},
			{},
			image_memory_barrier
		);

		return NoError();
	}
}