#pragma once

#include "pch.h"

namespace Raysterizer::Hooks
{
	enum class CallConvention
	{
		stdcall_t,
		cdecl_t
	};

	template <CallConvention cc, typename retn, typename ...args>
	struct convention;

	template <typename retn, typename ...args>
	struct convention<CallConvention::stdcall_t, retn, args...>
	{
		typedef retn(__stdcall* type)(args ...);
	};

	template <typename retn, typename ...args>
	struct convention<CallConvention::cdecl_t, retn, args...>
	{
		typedef retn(__cdecl* type)(args ...);
	};

	template <typename T>
	inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
	{
		return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
	}

	template <CallConvention cc, typename retn, typename ...args>
	class Hook
	{
		typedef typename convention<cc, retn, args...>::type type;

		size_t _orig;
		type _trampoline;
		type _detour;

		bool _isApplied;

	public:
		Hook() : _orig(0), _trampoline(0), _detour(0), _isApplied(false)
		{
		}

		template <typename T>
		Hook(T pFunc, type detour) : _orig(0), _isApplied(false), apply<T>(pFunc, detour)
		{
		}

		~Hook()
		{
			remove();
		}

		template <typename T>
		void apply(T pFunc, type detour)
		{
			CallOnce
			{
				MH_Initialize();
			};
			_detour = detour;
			_orig = (size_t)(pFunc);

			MH_STATUS s = MH_CreateHookEx((PBYTE)pFunc, (PBYTE)_detour, &_trampoline);
			if (s != MH_OK)
			{
				PanicMessageBox("Couldn't create hook: " + std::to_string(s));
			}

			s = MH_EnableHook((PBYTE)pFunc);
			if (s != MH_OK)
			{
				PanicMessageBox("Couldn't enable hook: " + std::to_string(s));
			}

			_isApplied = true;
		}

		bool remove()
		{
			if (!_isApplied)
				return false;

			_isApplied = false;
			return MH_DisableHook((PBYTE)_orig) == MH_OK;
		}

		bool disable()
		{
			return MH_DisableHook((PBYTE)_orig) == MH_OK;
		}

		bool enable()
		{
			return MH_EnableHook((PBYTE)_orig) == MH_OK;
		}

		void SetEnabled(bool enabled)
		{
			if (enabled)
			{
				MH_EnableHook((PBYTE)_orig);
			}
			else
			{
				MH_DisableHook((PBYTE)_orig);
			}
		}

		retn callOrig(args ... p)
		{
			return _trampoline(p...);
		}

		bool isApplied() const
		{
			return _isApplied;
		}
	};

	class NopHook
	{
	public:
		explicit NopHook(uintptr_t address_, int nop_amount_) :
			address((uint8_t*)address_), nop_amount(nop_amount_)
		{
			original_instructions.resize(nop_amount);
		}

		void Hook()
		{
			DWORD old_protect;
			VirtualProtect(address, nop_amount, PAGE_EXECUTE_READWRITE, &old_protect);
			std::copy(address, address + nop_amount, std::begin(original_instructions));
			memset((void*)address, nop_instruction, nop_amount);
			VirtualProtect(address, nop_amount, old_protect, &old_protect);
			changed = true;
		}

		void Remove()
		{
			if (changed)
			{
				DWORD old_protect;
				VirtualProtect(address, nop_amount, PAGE_EXECUTE_READWRITE, &old_protect);
				std::copy(std::begin(original_instructions), std::end(original_instructions), address);
				VirtualProtect(address, nop_amount, old_protect, &old_protect);
				changed = false;
			}
		}

	private:
		const uint8_t nop_instruction = 0x90;

		uint8_t* address;
		int nop_amount;
		std::vector<uint8_t> original_instructions;
		bool changed = false;
	};
}