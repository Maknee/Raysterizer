#include "include/vulkan_utils.h"

namespace RaysterizerEngine
{
	Error HasCompletionCallback::AddCompletionCallback(std::function<void()> completion_callback)
	{
		completion_callbacks.emplace_back(completion_callback);
		return NoError();
	}

	Error HasCompletionCallback::PerformCompletionCallbacks()
	{
		for (const auto& completion_callback : completion_callbacks)
		{
			completion_callback();
		}
		completion_callbacks.clear();
		return NoError();
	}

	void HasInUseOption::SetInUse(bool in_use_)
	{
		in_use = in_use_;
	}

	bool HasInUseOption::HasInUse() const
	{
		return in_use.has_value();
	}

	bool HasInUseOption::IsInUse() const
	{
		if (in_use)
		{
			return *in_use;
		}
		else
		{
			PANIC("Grabbing value of in use which has not been set yet");
		}
	}

	void HasInUseOption::ResetInUse()
	{
		in_use = std::nullopt;
	}
}