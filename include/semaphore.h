#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct SemaphoreCreateInfo
	{
		vk::SemaphoreCreateInfo semaphore_create_info{};
	public:
		bool operator==(const SemaphoreCreateInfo& o) const noexcept {
			return semaphore_create_info == o.semaphore_create_info;
		}
	};

	class Semaphore
	{
	public:
		vk::Semaphore semaphore{};
		uint64_t value{};
		vk::SemaphoreType semaphore_type{};

	public:
		const vk::Semaphore& operator*() const noexcept
		{
			return semaphore;
		}

		operator vk::Semaphore() noexcept
		{
			return semaphore;
		}

	public:
		bool operator==(const Semaphore& other) const noexcept {
			return semaphore == other.semaphore;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<SemaphoreCreateInfo>
	{
		size_t operator()(const SemaphoreCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.semaphore_create_info));
			return h;
		}
	};
}