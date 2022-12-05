#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct SamplerCreateInfo
	{
		vk::SamplerCreateInfo sampler_create_info{};

	public:
		bool operator==(const SamplerCreateInfo& o) const
		{
			return sampler_create_info == o.sampler_create_info;
		}
	};

	struct Sampler
	{
		vk::Sampler sampler;
		SamplerCreateInfo sampler_create_info;

	public:
		vk::Sampler operator*() const
		{
			return sampler;
		}

		operator vk::Sampler() noexcept
		{
			return sampler;
		}

		bool operator==(const Sampler& o) const noexcept {
			return sampler == o.sampler;
		}
	};
}

namespace std
{
	using namespace RaysterizerEngine;

	template<>
	struct hash<SamplerCreateInfo>
	{
		size_t operator()(const SamplerCreateInfo& o) const noexcept
		{
			return Hash(o.sampler_create_info);
		}
	};

	template<>
	struct hash<Sampler>
	{
		size_t operator()(const Sampler& o) const noexcept
		{
			return Hash(o.sampler);
		}
	};
}