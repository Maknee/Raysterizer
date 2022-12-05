#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct DescriptorPoolCreateInfo
	{
		DescriptorSetLayoutCreateInfos descriptor_set_layout_create_infos{};
	public:
		bool operator==(const DescriptorPoolCreateInfo& o) const noexcept {
			return descriptor_set_layout_create_infos == o.descriptor_set_layout_create_infos;
		}
	};

	class DescriptorPool
	{
	public:
		vk::DescriptorPool descriptor_pool{};
		
	public:
		vk::DescriptorPool operator*() const
		{
			return descriptor_pool;
		}

		operator vk::DescriptorPool() noexcept
		{
			return descriptor_pool;
		}

	public:
		bool operator==(const DescriptorPool& o) const noexcept {
			return descriptor_pool == o.descriptor_pool;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<DescriptorPoolCreateInfo>
	{
		size_t operator()(const DescriptorPoolCreateInfo& o) const noexcept
		{
			size_t h{};
			HashCombine(h, StdHash(o.descriptor_set_layout_create_infos));
			return h;
		}
	};

	template<>
	struct hash<DescriptorPool>
	{
		size_t operator()(const DescriptorPool& o) const noexcept
		{
			size_t h{};
			HashCombine(h, Hash(o.descriptor_pool));
			return h;
		}
	};
}