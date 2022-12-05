#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	using FrameCounter = std::size_t;

	/*
	template<typename T>
	class Pool
	{
	public:
		template<typename... P>
		T* allocate(P &&... p)
		{
			if (vacants.empty())
			{
				unsigned num_objects = 64u << memory.size();
				T* ptr = new T[new_objects];
				if (!ptr)
				{
					return nullptr;
				}

				for (unsigned i = 0; i < num_objects; i++)
				{
					vacants.push_back(&ptr[i]);
				}

				memory.emplace_back(ptr);
			}

			T* ptr = vacants.back();
			vacants.pop_back();
			new(ptr) T(std::forward<P>(p)...);
			return ptr;
		}

		void free(T* ptr)
		{
			ptr->~T();
			vacants.push_back(ptr);
		}

		void clear()
		{
			vacants.clear();
			memory.clear();
		}

	protected:
		std::vector<T*> vacants;
		std::vector<std::unique_ptr<T>> memory;
	};
	*/

	template<typename T, typename T2>
	class CacheMapping
	{
	public:
		Expected<T2&> Get(const T& t)
		{
			if (auto found = mapping.find(t); found != std::end(mapping))
			{
				return found->second;
			}
			return StringError("Not Found");
		}

		Error Emplace(const T& t, const T2& t2)
		{
			mapping.emplace(t, t2);
			return NoError();
		}

		Error Emplace(const T& t, T2&& t2)
		{
			mapping.emplace(t, std::forward<T2>(t2));
			return NoError();
		}

		Error Emplace(T&& t, T2&& t2)
		{
			mapping.emplace(std::forward<T>(t), std::forward<T2>(t2));
			return NoError();
		}

		void Clear()
		{
			mapping.clear();
		}

		auto begin()
		{
			return std::begin(mapping);
		}

		auto end()
		{
			return std::end(mapping);
		}

	protected:
		flat_hash_map<T, T2> mapping;
		//std::unordered_map<T, T2> mapping;
	};

	template<typename T>
	struct CacheFrameCounterEntry
	{
		T t{};
		FrameCounter frame_counter{};
	};

	template<typename T>
	struct CacheFrameCounterEntryWithCompleted
	{
		T t{};
		FrameCounter frame_counter{};
		bool completed = false;
	};

	template<typename T, typename T2>
	class CacheMappingWithFrameCounter : public CacheMapping<T, CacheFrameCounterEntry<T2>>
	{
	public:
		void SetContext(Context* c_)
		{
			c = c_;
		}

		Expected<T2&> Get(const T& t)
		{
			if (auto found = mapping.find(t); found != std::end(mapping))
			{
				found->second.frame_counter = c->GetFrame();
				return found->second.t;
			}
			return StringError("Not Found");
		}

		Expected<CacheFrameCounterEntry<T2>&> GetWithCounters(const T& t)
		{
			if (auto found = mapping.find(t); found != std::end(mapping))
			{
				found->second.frame_counter = c->GetFrame();
				return found->second;
			}
			return StringError("Not Found");
		}

		void ClearEntriesPastFrameCounterDifference(FrameCounter diff)
		{
			for (auto it = std::begin(mapping); it != std::end(mapping);)
			{
				const auto& [t, e] = *it;
				auto frame = c->GetFrame();
				if ((frame - e.frame_counter) > diff)
				{
					it = mapping.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		auto& Emplace(const T& t, const T2& t2)
		{
			CacheFrameCounterEntry<T2> e{ t2, c->GetFrame() };
			auto [iter, inserted] = mapping.emplace(t, std::move(e));
			auto& [inserted_t1, inserted_t2] = *iter;
			return inserted_t2.t;
		}

		auto& Emplace(const T& t, T2&& t2)
		{
			CacheFrameCounterEntry<T2> e{ std::move(t2), c->GetFrame() };
			auto [iter, inserted] = mapping.emplace(t, std::move(e));
			auto& [inserted_t1, inserted_t2] = *iter;
			return inserted_t2.t;
		}

		auto& Emplace(T&& t, T2&& t2)
		{
			CacheFrameCounterEntry<T2> e{ std::move(t2), c->GetFrame() };
			auto [iter, inserted] = mapping.emplace(std::forward<T>(t), std::move(e));
			auto& [inserted_t1, inserted_t2] = *iter;
			return inserted_t2.t;
		}

	protected:
		Context* c{};
	};

	template<typename T>
	struct CacheFrameAndUsageCounterEntry
	{
		T t{};
		FrameCounter frame_counter{};
		std::size_t usage_counter{};
	};

	template<typename T, typename T2>
	class CacheMappingWithFrameAndUsageCounter : public CacheMapping<T, CacheFrameAndUsageCounterEntry<T2>>
	{
	public:
		void SetContext(Context* c_)
		{
			c = c_;
		}

		Expected<T2&> Get(const T& t)
		{
			if (auto found = mapping.find(t); found != std::end(mapping))
			{
				auto& e = found->second;
				e.frame_counter = c->GetFrame();
				e.usage_counter++;
				return found->second.t;
			}
			return StringError("Not Found");
		}

		Expected<CacheFrameAndUsageCounterEntry<T2>&> GetWithCounters(const T& t)
		{
			if (auto found = mapping.find(t); found != std::end(mapping))
			{
				auto& e = found->second;
				e.frame_counter = c->GetFrame();
				e.usage_counter++;
				return found->second;
			}
			return StringError("Not Found");
		}

		void ClearEntriesPastFrameCounterDifference(FrameCounter diff)
		{
			for (auto it = std::begin(mapping); it != std::end(mapping);)
			{
				const auto& [t, e] = *it;
				auto frame = c->GetFrame();
				if ((frame - e.frame_counter) > diff)
				{
					it = mapping.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		auto& Emplace(const T& t, const T2& t2)
		{
			CacheFrameAndUsageCounterEntry<T2> e{ t2, c->GetFrame(), 0 };
			auto [iter, inserted] = mapping.emplace(t, std::move(e));
			auto& [inserted_t1, inserted_t2] = *iter;
			return inserted_t2.t;
		}

		auto& Emplace(const T& t, T2&& t2)
		{
			CacheFrameAndUsageCounterEntry<T2> e{ std::move(t2), c->GetFrame(), 0 };
			auto [iter, inserted] = mapping.emplace(t, std::move(e));
			auto& [inserted_t1, inserted_t2] = *iter;
			return inserted_t2.t;
		}

		auto& Emplace(T&& t, T2&& t2)
		{
			CacheFrameAndUsageCounterEntry<T2> e{ std::move(t2), c->GetFrame(), 0 };
			auto [iter, inserted] = mapping.emplace(std::forward<T>(t), std::move(e));
			auto& [inserted_t1, inserted_t2] = *iter;
			return inserted_t2.t;
		}

	protected:
		Context* c{};
	};
}