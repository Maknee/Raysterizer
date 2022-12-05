#include "include/fence.h"

namespace RaysterizerEngine
{
	Fence::Fence()
	{

	}

	Fence::Fence(vk::Fence fence_) :
		fence(std::move(fence_))
	{

	}

	void Fence::AddCompletionTo(CMShared<CommandBuffer> command_buffer)
	{
		command_buffer->SetInUse(true);
		command_buffer_completions.emplace_back(command_buffer);
	}
	
	void Fence::AddCompletionTo(CMShared<DescriptorSet> descriptor_set)
	{
		descriptor_set->SetInUse(true);
		descriptor_set_completions.emplace_back(descriptor_set);
	}

	Error Fence::PerformCompletionCallbacks()
	{
		for (auto& command_buffer : command_buffer_completions)
		{
			command_buffer->SetInUse(false);
		}
		command_buffer_completions.clear();
		for (auto& descriptor_set : descriptor_set_completions)
		{
			descriptor_set->SetInUse(false);
		}
		descriptor_set_completions.clear();
		return HasCompletionCallback::PerformCompletionCallbacks();
	}
}