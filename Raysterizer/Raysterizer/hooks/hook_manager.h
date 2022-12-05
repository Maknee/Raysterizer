#pragma once

#include "pch.h"

namespace Raysterizer::Hooks
{
	using Address = void*;
	class HookManager
	{
	private:
		void HookOpenglModule();
	public:
		explicit HookManager();
		static HookManager& Get();
		void Init();
		Expected<Address> GetOriginalFunction(Address address);
		Expected<Address> GetHookedFunction(Address address);

		Expected<Address> GetOriginalFunctionByName(std::string_view name);
		Expected<Address> GetProxyFunctionByName(std::string_view name);
		Error HookOriginalFunctionByName(std::string_view name, Address hooked_function);
		void HookOriginalFunctionByNameDelay(std::string_view name, Address hooked_function);

		Expected<Address> GetHookedToOriginalFunction(Address address);

		Expected<Address> WglGetProcAddress(std::string_view name);
		Expected<Address> GetWglGetProcAddressHook(std::string_view name);
		Error ResolveWglGetProcAddressHook(std::string_view name, Address original_function);
	public:
		flat_hash_map<Address, std::string> proxy_function_to_name;
		flat_hash_map<std::string, Address> name_to_proxy_function;
		flat_hash_map<std::string, Address> name_to_original_function;

		flat_hash_map<Address, Address> proxy_to_original_function;
		flat_hash_map<Address, Address> proxy_to_hooked_function;
		flat_hash_map<Address, Address> hooked_to_original_function;

		flat_hash_map<std::string, Address> name_to_wgl_get_proc_address_hook_function;
	};

	template<typename T>
	inline T GetFunction(T proxy)
	{
		static_assert(std::is_pointer_v<T>);

		auto& m = HookManager::Get();
		auto proxy_address = reinterpret_cast<Address>(proxy);
		auto hooked = m.GetHookedFunction(proxy_address);
		if (auto err = hooked.takeError())
		{
			llvm::consumeError(std::move(err));

			// Attempt to try to get the original function instead
			if (auto original_or_err = m.GetOriginalFunction(proxy_address))
			{
				auto original = *original_or_err;
				return reinterpret_cast<T>(original);
			}
			else
			{
				llvm::consumeError(original_or_err.takeError());

				// Finally, attempt to get hooked function to proxy function instead
				auto original = AssignOrPanic(m.GetHookedToOriginalFunction(proxy_address));
				return reinterpret_cast<T>(original);
			}
		}
		auto hooked_function = reinterpret_cast<T>(*hooked);
		return hooked_function;
	}

	template<typename T>
	inline void HookFunctionByName(std::string_view name, T address)
	{
		auto& m = HookManager::Get();
		PanicIfError(m.HookOriginalFunctionByName(name, static_cast<T>(address)));
	}
}
