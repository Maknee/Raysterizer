#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	enum QueueType : uint8_t
	{
		Graphics,
		Compute,
		Transfer,
		Present,
		End
	};

	struct CommandPoolCreateInfo
	{
		QueueType queue_type;
		vk::CommandPoolCreateFlags flags;
	public:
		bool operator==(const CommandPoolCreateInfo& o) const noexcept {
			return queue_type == o.queue_type && flags == o.flags;
		}
	};

	class CommandPool
	{
	public:
		vk::CommandPool command_pool{};
		CommandPoolCreateInfo command_pool_create_info{};

	public:
		const vk::CommandPool& operator*() const noexcept
		{
			return command_pool;
		}

		operator vk::CommandPool() noexcept
		{
			return command_pool;
		}

	public:
		bool operator==(const CommandPool& o) const noexcept {
			return command_pool == o.command_pool &&
				command_pool_create_info == o.command_pool_create_info;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<CommandPoolCreateInfo>
	{
		size_t operator()(const CommandPoolCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.queue_type, o.flags));
			return h;
		}
	};

	template<>
	struct hash<CommandPool>
	{
		size_t operator()(const CommandPool& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.command_pool));
			HashCombine(h, StdHash(o.command_pool_create_info));
			return h;
		}
	};
}