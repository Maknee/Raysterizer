#include "include/wrappers.h"

namespace RaysterizerEngine
{
	/*
	template<typename Type>
	inline ContextManaged<Type>::~ContextManaged() noexcept
	{
		if (context)
		{
			PanicIfError(context->EnqueueDestroy(std::move(payload)));
		}
	}
	*/
	/*
	template<typename Type>
	inline void ContextManaged<Type>::Reset(Type value) noexcept {
		if (payload != value) {
			if (context && payload != Type{}) {
				PanicIfError(context->EnqueueDestroy(std::move(payload)));
			}
			payload = std::move(value);
		}
	}
	*/
}