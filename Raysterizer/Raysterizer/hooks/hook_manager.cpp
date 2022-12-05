#include "hook_manager.h"

namespace Raysterizer::Hooks
{

struct ModuleExport
{
	void* address;
	const char* name;
	unsigned short ordinal;
};

fs::path GetSystemPath()
{
	static fs::path system_path;
	if (system_path.empty())
	{
		WCHAR buf[MAX_PATH];
		if (0 == GetEnvironmentVariableW(L"MODULE_PATH_OVERRIDE", buf, ARRAYSIZE(buf)))
		{
			// First try environment variable, use system directory if it does not exist or is empty
			GetSystemDirectoryW(buf, ARRAYSIZE(buf));
		}

		if (system_path = buf; system_path.has_stem())
			system_path += L'\\'; // Always convert to directory path (with a trailing slash)

		system_path = system_path.lexically_normal();
	}
	return system_path;
}

std::vector<ModuleExport> EnumerateExports(HMODULE handle)
{
	const auto image_base = reinterpret_cast<const BYTE*>(handle);
	const auto image_header = reinterpret_cast<const IMAGE_NT_HEADERS*>(image_base +
																		reinterpret_cast<const IMAGE_DOS_HEADER*>(image_base)->e_lfanew);

	if (image_header->Signature != IMAGE_NT_SIGNATURE ||
		image_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		return {}; // The handle does not point to a valid module

	const auto export_dir = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(image_base +
																			image_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	const auto export_base = static_cast<WORD>(export_dir->Base);

	if (export_dir->NumberOfFunctions == 0)
		return {}; // This module does not contain any exported functions

	std::vector<ModuleExport> exports;
	exports.reserve(export_dir->NumberOfNames);

	for (size_t i = 0; i < exports.capacity(); i++)
	{
		ModuleExport symbol;
		symbol.ordinal = reinterpret_cast<const WORD*>(image_base + export_dir->AddressOfNameOrdinals)[i] + export_base;
		symbol.name = reinterpret_cast<const char*>(image_base +
													reinterpret_cast<const DWORD*>(image_base + export_dir->AddressOfNames)[i]);
		symbol.address = const_cast<void*>(reinterpret_cast<const void*>(image_base +
																		 reinterpret_cast<const DWORD*>(image_base + export_dir->AddressOfFunctions)[symbol.ordinal - export_base]));
		exports.emplace_back(symbol);
	}

	return exports;
}

static std::string lastWinErrorAsString()
{
	// Adapted from this SO thread:
	// http://stackoverflow.com/a/17387176/1198654

	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0)
	{
		return "Unknown error";
	}

	LPSTR messageBuffer = nullptr;
	constexpr DWORD fmtFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS;

	const auto size = FormatMessageA(fmtFlags, nullptr, errorMessageID,
									 MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
									 (LPSTR)&messageBuffer, 0, nullptr);

	const std::string message{ messageBuffer, size };
	LocalFree(messageBuffer);

	return message + "(error " + std::to_string(errorMessageID) + ")";
}

HookManager::HookManager()
{
	Init();
}

HookManager& HookManager::Get()
{
	static HookManager m;
	return m;
}

namespace
{
	PROC WINAPI hooked_wglGetProcAddress(LPCSTR lpszProc)
	{
		auto& m = HookManager::Get();
		std::string_view name{ lpszProc };

		if (auto proxy_function_or_err = m.GetProxyFunctionByName(name))
		{
			auto proxy_function = *proxy_function_or_err;

			return reinterpret_cast<PROC>(proxy_function);
		}
		else
		{
			llvm::consumeError(proxy_function_or_err.takeError());

			static const decltype(wglGetProcAddress)* original_wglGetProcAddress = reinterpret_cast<decltype(wglGetProcAddress)*>(AssignOrPanic(m.GetOriginalFunction(wglGetProcAddress)));
			auto original_function = original_wglGetProcAddress(lpszProc);

			// check if we can resolve this as a wglGetProcAddress hook (in case we have hooked it)
			if (auto hook_address_or_err = m.GetWglGetProcAddressHook(name))
			{
				auto hooked_address = *hook_address_or_err;
				PanicIfError(m.ResolveWglGetProcAddressHook(name, original_function));

				// return hooked function instead
				return reinterpret_cast<PROC>(hooked_address);
			}
			else
			{
				llvm::consumeError(hook_address_or_err.takeError());
			}

			return original_function;
		}
	}
}

void HookManager::HookOpenglModule()
{
	const auto system_path = GetSystemPath();
	const auto module_path = system_path / "opengl32.dll";
	if (HMODULE handle; !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, module_path.wstring().c_str(), &handle))
	{
		// Delayed
	}
	else
	{
		// Already loaded
	}
	auto opengl32_module = ((HMODULE)LoadLibraryEx(module_path.string().c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32));
	if (opengl32_module == 0)
	{
		PANIC("{}", lastWinErrorAsString());
	}
	auto opengl32_exports = EnumerateExports(opengl32_module);

	auto this_handle = Util::GetCurrentModule();
	auto proxy_exports = EnumerateExports(this_handle);

	for (const auto& e : proxy_exports)
	{
		if (e.name == nullptr || e.address == nullptr)
		{
			continue;
		}

		//fmt::print("[{}] -> {}\n", e.name, e.address);
		name_to_proxy_function[e.name] = e.address;
		proxy_function_to_name[e.address] = e.name;
	}

	for (const auto& e : opengl32_exports)
	{
		if (e.name == nullptr || e.address == nullptr)
			continue;

		//fmt::print("{}\n", e.name);

		auto found_corresponding_proxy_function = name_to_proxy_function.find(e.name);
		if (found_corresponding_proxy_function != std::end(name_to_proxy_function))
		{
			auto proxy_function = found_corresponding_proxy_function->second;
			proxy_to_original_function[proxy_function] = e.address;
			name_to_original_function[e.name] = e.address;
			//fmt::print("[{}] -> {}\n", e.name, e.address);
		}
		else
		{
			DEBUG("Not found {}", e.name);
		}
	}

	if (0)
	{
		for (auto& [proxy_function, original_function] : proxy_to_original_function)
		{
			DEBUG("[{}] -> {}\n", proxy_function, original_function);
		}
	}

	PanicIfError(HookOriginalFunctionByName("wglGetProcAddress", hooked_wglGetProcAddress));
}

void HookManager::Init()
{
	static bool initialized = false;
	if (!initialized)
	{
		HookOpenglModule();
		initialized = true;
	}
	else
	{
		PANIC("Opengl already hooked!");
	}
}

Expected<Address> HookManager::GetOriginalFunction(Address address)
{
	auto found = proxy_to_original_function.find(address);
	if (found != std::end(proxy_to_original_function))
	{
		auto& original = found->second;
		return original;
	}
	return StringError("Proxy function not found for {}", fmt::ptr(address));
}

Expected<Address> HookManager::GetHookedFunction(Address address)
{
	auto found = proxy_to_hooked_function.find(address);
	if (found != std::end(proxy_to_hooked_function))
	{
		auto original = found->second;
		return original;
	}
	return StringError("Hooked function not found for {}", fmt::ptr(address));
}

Expected<Address> HookManager::GetOriginalFunctionByName(std::string_view name)
{
	if (auto found = name_to_original_function.find(name); found != std::end(name_to_original_function))
	{
		auto original = found->second;
		return original;
	}
	else if (auto found_or_err = WglGetProcAddress(name))
	{
		return *found_or_err;
	}
	else
	{
		llvm::consumeError(found_or_err.takeError());
	}

	return StringError("Original function not found for {}", name);
}

Expected<Address> HookManager::GetProxyFunctionByName(std::string_view name)
{
	auto found = name_to_proxy_function.find(name);
	if (found != std::end(name_to_proxy_function))
	{
		auto original = found->second;
		return original;
	}
	return StringError("Proxy function not found for {}", name);
}

Error HookManager::HookOriginalFunctionByName(std::string_view name, Address hooked_function)
{
	/*
	Address original_function{};
	AssignOrReturnError(original_function, GetProxyFunctionByName(name));
	proxy_to_hooked_function.emplace(original_function, hooked_function);
	return NoError();
	*/
	if (auto proxy_function_or_err = GetProxyFunctionByName(name))
	{
		auto proxy_function = *proxy_function_or_err;
		{
			auto [previous_instance, success] = proxy_to_hooked_function.try_emplace(proxy_function, hooked_function);
			if (!success)
			{
				// Entry already exists, overwrite
				//previous_instance->second = hooked_function;
				return StringError("Hook already exists");
			}
		}

		{
			Address original_function;
			AssignOrReturnError(original_function, GetOriginalFunctionByName(name));
			auto [previous_instance, success] = hooked_to_original_function.try_emplace(hooked_function, original_function);
			if (!success)
			{
				return StringError("Hook already exists");
			}
		}
	}
	else
	{
		// This isn't going to be installed now
		// so delay this installation
		llvm::consumeError(proxy_function_or_err.takeError());
		HookOriginalFunctionByNameDelay(name, hooked_function);
	}
	return NoError();
}

void HookManager::HookOriginalFunctionByNameDelay(std::string_view name, Address hooked_function)
{
	name_to_wgl_get_proc_address_hook_function[name] = hooked_function;
}

Expected<Address> HookManager::GetHookedToOriginalFunction(Address address)
{
	if (auto found = hooked_to_original_function.find(address); found != std::end(hooked_to_original_function))
	{
		auto proxy = found->second;
		return proxy;
	}
	return StringError("Hooked address not found");
}

Expected<Address> HookManager::WglGetProcAddress(std::string_view name)
{
	auto& m = HookManager::Get();
	static const decltype(wglGetProcAddress)* original_wglGetProcAddress = reinterpret_cast<decltype(wglGetProcAddress)*>(AssignOrPanic(m.GetOriginalFunction(wglGetProcAddress)));
	auto result = original_wglGetProcAddress(name.data());
	if (!result)
	{
		return StringError("WglGetProcAddress hook function not found for {}", name);
	}
	return static_cast<Address>(result);
}

Expected<Address> HookManager::GetWglGetProcAddressHook(std::string_view name)
{
	// Find in our current module exports first
	if (auto found = name_to_proxy_function.find(name); found != std::end(name_to_proxy_function))
	{
		auto& original = found->second;
		return original;
	}

	// Else run redirect that we have specified
	if (auto found = name_to_wgl_get_proc_address_hook_function.find(name); found != std::end(name_to_wgl_get_proc_address_hook_function))
	{
		auto& original = found->second;
		return original;
	}
	return StringError("WglGetProcAddress hook function not found for {}", name);
}

Error HookManager::ResolveWglGetProcAddressHook(std::string_view name, Address original_function)
{
	Address hooked_function{};
	AssignOrReturnError(hooked_function, GetWglGetProcAddressHook(name));
	//proxy_to_original_function[hooked_function] = original_function;
	proxy_to_hooked_function.emplace(hooked_function, original_function);
	return NoError();
}

}
