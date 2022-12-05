#include "include/pch.h"

using namespace RaysterizerEngine;

void DllInitialize()
{
    {
        SetEnvironmentVariable("NVIDIA_PROCESS_INJECTION_CRASH_REPORTING", "0");
    }

    auto hwnd = GetConsoleWindow();
    if (!hwnd)
    {
        AllocConsole();
        FILE* f{};
        freopen_s(&f, "CONIN$", "r", stdin);
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
    }
    spdlog::info("Welcome to spdlog!");
    
    //UnregisterClassW((char*)_GLFW_WNDCLASSNAME, glfw_window->);

    //PanicIfError(c.Setup(vulkan_window));

    auto& m = Raysterizer::Hooks::HookManager::Get();
    //PanicIfError(m.HookOriginalFunctionByName("glShaderSource", hooked_glShaderSource));
    //Raysterizer::Hooks::HookFunctionByName("glShaderSource", hooked_glShaderSource);
    auto& context = Raysterizer::OpenGL::Context::Get();
}

extern "C" BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
  {
    DisableThreadLibraryCalls(hinstDLL);
    DllInitialize();
      /*
    std::thread t = std::thread([=]() { 
        spdlog::info("Welcome to spdlog!");
        DllInitialize();
    });
    t.detach();
    */
    //
  }
  case DLL_PROCESS_DETACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  default:
    break;
  }
  return TRUE;
}