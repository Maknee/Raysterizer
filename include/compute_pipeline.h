#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	struct ComputePipeline
	{
		vk::Pipeline pipeline{};

		bool operator==(const ComputePipeline& other) const noexcept {
			return pipeline == other.pipeline;
		}
	};

	/*
	template<>
	class ContextManaged<ImageView>
	{
	};
	*/
}

/*
namespace std {
	template<>
	struct hash<RaysterizerEngine::ImageView> {
		size_t operator()(const RaysterizerEngine::ImageView& x) const noexcept {
			return 0;
		}
	};
}
*/