#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct CommandBufferCreateInfo
	{
		//CommandPoolCreateInfo command_pool_create_info{};
		CMShared<CommandPool> command_pool{};
		vk::CommandBufferLevel command_buffer_level{};
	public:
		bool operator==(const CommandBufferCreateInfo& o) const noexcept {
			return command_pool == o.command_pool &&
				command_buffer_level == o.command_buffer_level;
		}
	};

	class CommandBuffer : public HasCompletionCallback, public HasInUseOption
	{
	public:
		explicit CommandBuffer();
		explicit CommandBuffer(vk::CommandBuffer command_buffer_, CommandBufferCreateInfo command_buffer_create_info_);

	public:
		const vk::CommandBuffer& operator*() const
		{
			return command_buffer;
		}

		vk::CommandBuffer const operator->() const noexcept {
			return command_buffer;
		}

		vk::CommandBuffer operator->() noexcept {
			return command_buffer;
		}

		void Record(std::function<void(CommandBuffer&)> f, const vk::CommandBufferBeginInfo& command_buffer_begin_info = vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
		void RecordAndEnd(std::function<void(CommandBuffer&)> f, const vk::CommandBufferBeginInfo& command_buffer_begin_info = vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
		void Reset();
		void Begin(const vk::CommandBufferBeginInfo& command_buffer_begin_info = {});
		void End();
		
		void AddDependencyTo(CMShared<Buffer> buffer);
		void AddDependencyTo(const std::vector<CMShared<Buffer>>& buffers);
		void AddDependencyTo(CMShared<Texture> texture);
		void AddDependencyTo(const std::vector<CMShared<Texture>>& textures);
		void AddDependencyTo(CMShared<DescriptorSet> descriptor_set);
		void AddDependencyTo(CMShared<BottomLevelAccelerationStructure> bottom_level_acceleration_structure);
		void AddDependencyTo(CMShared<TopLevelAccelerationStructure> top_level_acceleration_structure);
		void AddDependencyTo(CMShared<FrameBuffer> frame_buffer);
		const std::vector<CMShared<Buffer>>& GetDependencyBuffers() const { return dependency_buffers; };

		Error InsertImageMemoryBarrier(CMShared<Image> image, vk::ImageLayout old_image_layout, vk::ImageLayout new_image_layout, vk::ImageSubresourceRange subresource_range, vk::PipelineStageFlags src_pipeline_stage_flags = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dst_pipeline_stage_flags = vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags dependency_flags = {});
		Error InsertImageMemoryBarrier(vk::ImageMemoryBarrier image_memory_barrier, vk::PipelineStageFlags src_pipeline_stage_flags, vk::PipelineStageFlags dst_pipeline_stage_flags, vk::DependencyFlags dependency_flags = {});
		Error InsertImageMemoryBarrier2(vk::ImageMemoryBarrier2KHR image_memory_barrier, vk::PipelineStageFlags src_pipeline_stage_flags, vk::PipelineStageFlags dst_pipeline_stage_flags, vk::DependencyFlags dependency_flags = {});
		Error SetImageLayout(CMShared<Image> image,
			vk::ImageLayout old_image_layout,
			vk::ImageLayout new_image_layout,
			vk::ImageSubresourceRange subresource_range,
			vk::PipelineStageFlags src_stage_mask = vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlags dst_stage_mask = vk::PipelineStageFlagBits::eAllCommands);
	public:
		bool operator==(const CommandBuffer& o) const noexcept {
			return command_buffer == o.command_buffer;
		}

	private:
		bool begin_recording = false;
		bool end_recording = false;

	public:
		vk::CommandBuffer command_buffer{};
		CommandBufferCreateInfo command_buffer_create_info{};

	private:
		std::vector<CMShared<Buffer>> dependency_buffers;
		std::vector<CMShared<Texture>> dependency_textures;
		std::vector<CMShared<Image>> dependency_images;
		std::vector<CMShared<ImageView>> dependency_image_views;
		std::vector<CMShared<Sampler>> dependency_samplers;
		std::vector<CMShared<DescriptorSet>> dependency_descriptor_sets;
		std::vector<CMShared<BottomLevelAccelerationStructure>> bottom_level_acceleration_structures;
		std::vector<CMShared<TopLevelAccelerationStructure>> top_level_acceleration_structures;
		std::vector<CMShared<FrameBuffer>> frame_buffers;
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<CommandBufferCreateInfo>
	{
		size_t operator()(const CommandBufferCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, StdHash(*o.command_pool));
			HashCombine(h, Hash(o.command_buffer_level));
			return h;
		}
	};

	template<>
	struct hash<CommandBuffer>
	{
		size_t operator()(const CommandBuffer& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.command_buffer));
			return h;
		}
	};
}