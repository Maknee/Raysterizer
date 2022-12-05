#pragma once

#include "pch.h"

namespace RaysterizerEngine
{
	class Context;
	template <typename Type>
	class ContextManaged {
		Context* context;
		Type payload;
	public:
		using element_type = Type;

		ContextManaged() : context(nullptr) {}

		explicit ContextManaged(Context& ctx, Type payload) : context(&ctx), payload(std::move(payload)) {}
		~ContextManaged() noexcept
		{
			if (context)
			{
				PanicIfError(context->EnqueueDestroy(std::move(payload)));
			}
		}
		ContextManaged(const ContextManaged&) = delete;
		ContextManaged& operator=(const ContextManaged&) = delete;
		ContextManaged(ContextManaged&& other) noexcept : context(other.context), payload(other.Release()) {}
		ContextManaged& operator=(ContextManaged&& other) noexcept
		{
			context = std::move(other.context);
			payload = std::move(other.payload);
			other.context = nullptr;
			return *this;
			/*
			auto tmp = other.context;
			Reset(other.Release());
			context = tmp;
			return *this;
			*/
		}

		explicit operator bool() const noexcept {
			return payload.operator bool();
		}

		Type const* operator->() const noexcept {
			return &payload;
		}

		Type* operator->() noexcept {
			return &payload;
		}

		Type const& operator*() const noexcept {
			return payload;
		}

		Type& operator*() noexcept {
			return payload;
		}

		const Type& Get() const noexcept {
			return payload;
		}

		Type& Get() noexcept {
			return payload;
		}

		void Reset(Type value = Type()) noexcept
		{
			PANIC("Not supported");
			/*
			if (payload != value) {
				if (context) {
					PanicIfError(context->EnqueueDestroy(std::move(payload)));
				}
				payload = std::move(value);
			}
			*/
		}

		Type Release() noexcept {
			context = nullptr;
			return std::move(payload);
		}

		void Swap(ContextManaged<Type>& rhs) noexcept {
			std::swap(payload, rhs.payload);
			std::swap(context, rhs.context);
		}
	};

	template <typename Type>
	inline void Swap(ContextManaged<Type>& lhs, ContextManaged<Type>& rhs) noexcept {
		lhs.swap(rhs);
	}

	/*
	template<typename T>
	using ContextManagedShared = std::shared_ptr<ContextManaged<T>>;

	template<typename T>
	using CMShared = ContextManagedShared<T>;
	*/

	template<typename T>
	using ContextManagedShared = std::shared_ptr<T>;

	template<typename T>
	using CMShared = ContextManagedShared<T>;

	template<typename T>
	inline auto StdHash(const CMShared<T>& t)
	{
		return std::hash<T>{}(*t);
	}

	template<typename T>
	inline bool CheckEquality(const CMShared<T>& a, const CMShared<T>& b)
	{
		if (a != nullptr && b != nullptr)
		{
			return *a == *b;
		}
		else
		{
			return a == b;
		}
	}
	
	template<typename T>
	inline bool CheckEquality(const std::vector<CMShared<T>>& a, const std::vector<CMShared<T>>& b)
	{
		if (a.size() != b.size())
		{
			return false;
		}

		for (auto i = 0; i < a.size(); i++)
		{
			const T& v1 = *(a[i]);
			const T& v2 = *(b[i]);
			if (!(v1 == v2))
			{
				return false;
			}
		}
		return true;
	}
}