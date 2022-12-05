#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	constexpr std::size_t hash_seed = 0;

	template<typename T>
	static inline auto Hash(const T& t)
	{
		return XXH64(&t, sizeof(T), hash_seed);
	}

	template<typename T>
	static inline auto HashSingle(const T& t)
	{
		return XXH64(&t, sizeof(T), hash_seed);
	}

	template<typename ...Args>
	static inline auto Hash(Args&&... ts)
	{
		return (Hash(ts) ^ ...);
	}

	template<>
	static inline auto Hash(const std::string& t)
	{
		return XXH64(t.c_str(), t.length(), hash_seed);
	}

	template<typename T2>
	static inline auto Hash(const std::vector<T2>& t)
	{
		return XXH64(t.data(), t.size(), hash_seed);
	}

	template<typename T>
	static inline auto StdHash(const T& t)
	{
		return std::hash<T>{}(t);
	}

	template<typename T, typename T2>
	static inline void HashCombine(T& t, T2 t2)
	{
		t = HashSingle(t) + HashSingle(t2);
	}
}