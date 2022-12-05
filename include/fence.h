#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct FenceCreateInfo
	{
		vk::FenceCreateInfo fence_create_info{};
	public:
		bool operator==(const FenceCreateInfo& o) const noexcept {
			return fence_create_info == o.fence_create_info;
		}
	};

	class Fence : public HasCompletionCallback
	{
	public:
		explicit Fence();
		explicit Fence(vk::Fence fence_);

	public:
		const vk::Fence& operator*() const noexcept
		{
			return fence;
		}

		operator vk::Fence() noexcept
		{
			return fence;
		}

		void AddCompletionTo(CMShared<CommandBuffer> command_buffer);
		void AddCompletionTo(CMShared<DescriptorSet> descriptor_set);
		Error PerformCompletionCallbacks();

	public:
		bool operator==(const Fence& other) const noexcept {
			return fence == other.fence;
		}

	public:
		vk::Fence fence{};
		std::vector<CMShared<CommandBuffer>> command_buffer_completions;
		std::vector<CMShared<DescriptorSet>> descriptor_set_completions;
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<FenceCreateInfo>
	{
		size_t operator()(const FenceCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.fence_create_info));
			return h;
		}
	};
}