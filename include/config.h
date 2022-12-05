#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class RaysterizerConfig
	{
	public:
		explicit RaysterizerConfig();

		template<typename T>
		auto operator[](T&& t)
		{
			return j[std::forward<T>(t)];
		}

		json& GetJson() { return j; }
		const json& GetJson() const { return j; }

		/*
		template<typename T>
		Expected<T> operator[](std::string_view s)
		{
			return j[s];
		}
		*/

	private:
		json j;
	};

	inline RaysterizerConfig Config;
}