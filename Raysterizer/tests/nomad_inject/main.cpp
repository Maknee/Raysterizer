#include <Windows.h>

void DllInitialize()
{
    {
        SetEnvironmentVariable("ALLUSERSPROFILE", "C:/ProgramData");
        SetEnvironmentVariable("AMDRMPATH", "C:/Program Files/AMD/RyzenMaster/");
        SetEnvironmentVariable("APPDATA", "C:/Users/hehe/AppData/Roaming");
        SetEnvironmentVariable("COMPUTERNAME", "DESKTOP-6ELBASE");
        SetEnvironmentVariable("ChocolateyInstall", "C:/ProgramData/chocolatey");
        SetEnvironmentVariable("ChocolateyLastPathUpdate", "132985314288116225");
        SetEnvironmentVariable("ComSpec", "C:/Windows/system32/cmd.exe");
        SetEnvironmentVariable("CommonProgramFiles", "C:/Program Files/Common Files");
        SetEnvironmentVariable("CommonProgramFiles(x86)", "C:/Program Files (x86)/Common Files");
        SetEnvironmentVariable("CommonProgramW6432", "C:/Program Files/Common Files");
        SetEnvironmentVariable("DISABLE_VK_LAYER_VALVE_steam_overlay_1", "1");
        SetEnvironmentVariable("DriverData", "C:/Windows/System32/Drivers/DriverData");
        SetEnvironmentVariable("ENABLE_VKSC_LAYER_NV_nomad_release_public_2022_2_1", "1");
        SetEnvironmentVariable("ENABLE_VK_LAYER_NV_nomad_release_public_2022_2_1", "1");
        SetEnvironmentVariable("FPS_BROWSER_APP_PROFILE_STRING", "Internet Explorer");
        SetEnvironmentVariable("FPS_BROWSER_USER_PROFILE_STRING", "Default");
        SetEnvironmentVariable("GTCInterceptionDllPath", "C:/PROGRA~1/NVIDIA~1/NSIGHT~1.1/target/WINDOW~1/");
        SetEnvironmentVariable("HOMEDRIVE", "C:");
        SetEnvironmentVariable("HOMEPATH", "/Users/hehe");
        SetEnvironmentVariable("LOCALAPPDATA", "C:/Users/hehe/AppData/Local");
        SetEnvironmentVariable("LOGONSERVER", "//DESKTOP-6ELBASE");
        SetEnvironmentVariable("NOMAD_AGGRESSIVE_REPLAY_THREAD_CONTROL", "Auto");
        SetEnvironmentVariable("NOMAD_ALLOW_UNSUPPORTED_CAPTURE", "0");
        SetEnvironmentVariable("NOMAD_BLOCK_ON_FIRST_INCOMPATIBILITY", "Auto");
        SetEnvironmentVariable("NOMAD_CAPTURE_MODE", "Manual");
        SetEnvironmentVariable("NOMAD_CAPTURE_NTH_FRAME_INDEX", "100");
        SetEnvironmentVariable("NOMAD_COLLECT_LINE_TABLES", "1");
        SetEnvironmentVariable("NOMAD_COLLECT_SHADER_REFLECTION", "1");
        SetEnvironmentVariable("NOMAD_D3D12_ENABLE_CACHED_PIPELINE_STATE_SUPPORT", "0");
        SetEnvironmentVariable("NOMAD_D3D12_REPLAY_FENCE_BEHAVIOR", "Default");
        SetEnvironmentVariable("NOMAD_D3D12_WRITE_WATCH", "Auto");
        SetEnvironmentVariable("NOMAD_D3D_REVISION_ZERO_DATA_COLLECTION", "Auto");
        SetEnvironmentVariable("NOMAD_D3D_SHADER_COMPILE_SYNCHRONOUS", "0");
        SetEnvironmentVariable("NOMAD_DRIVER_INSTRUMENTATION", "1");
        SetEnvironmentVariable("NOMAD_DXGI_SYNC_INTERVAL", "0");
        SetEnvironmentVariable("NOMAD_ENABLE_CAPTURE_NTH_FRAME", "0");
        SetEnvironmentVariable("NOMAD_FORCE_REPAINT", "0");
        SetEnvironmentVariable("NOMAD_FRAME_CAPTURE_SINGLETHREADED", "0");
        SetEnvironmentVariable("NOMAD_HUD_LEGACY_CAPTURE_KEY_ENABLED", "0");
        SetEnvironmentVariable("NOMAD_HWPM_COLLECTION", "1");
        SetEnvironmentVariable("NOMAD_IGNORE_DX_OGL_OVER_VK_LIBRARIES", "Auto");
        SetEnvironmentVariable("NOMAD_OPENGL_DELIMITER", "SwapBuffers");
        SetEnvironmentVariable("NOMAD_OPENGL_NOERROR", "Application Controlled");
        SetEnvironmentVariable("NOMAD_OPENGL_REPORT_NULL_CLIENT_SIDE_BUFFER", "1");
        SetEnvironmentVariable("NOMAD_REPLAY_CAPTURED_EXECUTEINDIRECT_BUFFER", "0");
        SetEnvironmentVariable("NOMAD_REPORT_FORCE_FAILED_QUERY_INTERFACES", "1");
        SetEnvironmentVariable("NOMAD_REPORT_UNKNOWN_OBJECTS", "1");
        SetEnvironmentVariable("NOMAD_RTX_ACCELERATION_STRUCTURE_COLLECT_BUILDS_AND_REFITS", "Auto");
        SetEnvironmentVariable("NOMAD_RTX_ACCELERATION_STRUCTURE_COLLECT_TO_VIDMEM", "Auto");
        SetEnvironmentVariable("NOMAD_RTX_ACCELERATION_STRUCTURE_GEOMETRY_TRACKING_MODE", "Auto");
        SetEnvironmentVariable("NOMAD_RTX_ACCELERATION_STRUCTURE_REPORT_SHALLOW_GEOMETRY_TRACKING_WARNINGS_VARBASE", "1");
        SetEnvironmentVariable("NOMAD_SASS_COLLECTION", "1");
        SetEnvironmentVariable("NOMAD_SERIALIZATION", "1");
        SetEnvironmentVariable("NOMAD_SUPPRESS_RAW_INPUT_INTERCEPTION", "0");
        SetEnvironmentVariable("NOMAD_TARGET_HUD", "1");
        SetEnvironmentVariable("NOMAD_UNWEAVE_THREADS", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_ALLOW_UNSAFE_LAYERS", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_BUFFER_DEVICE_ADDRESS_CAPTURE_REPLAY_SUPPORT", "1");
        SetEnvironmentVariable("NOMAD_VULKAN_COHERENT_BUFFER_COLLECTION", "1");
        SetEnvironmentVariable("NOMAD_VULKAN_FORCE_CAPTURE_REPLAY_DEVICE_MEMORY", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_FORCE_VALIDATION", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_FULL_MEMORY_SERIALIZATION", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_HEAP_RESERVE_MB", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_REVISION_ZERO_DATA_COLLECTION", "Auto");
        SetEnvironmentVariable("NOMAD_VULKAN_SAFE_OBJECT_LOOKUP", "Auto");
        SetEnvironmentVariable("NOMAD_VULKAN_SC_ADDITIONAL_COMMAND_BUFFER_RESERVE", "800");
        SetEnvironmentVariable("NOMAD_VULKAN_SERIALIZATION_OBJECT_SET", "Only Active");
        SetEnvironmentVariable("NOMAD_VULKAN_UNSAFE_PNEXT_INSPECTION", "0");
        SetEnvironmentVariable("NOMAD_VULKAN_USE_WRITE_WATCH_MEMORY", "Auto");
        SetEnvironmentVariable("NSIGHT_CONNECTION_BASE_PORT", "49152");
        SetEnvironmentVariable("NSIGHT_CONNECTION_MAX_PORTS", "64");
        SetEnvironmentVariable("NSIGHT_DISABLE_OPENGL_SHADER_DEBUGGER_DRIVER_SUPPORT", "0");
        SetEnvironmentVariable("NSIGHT_HUD_CAPTURE_KEY", "122");
        SetEnvironmentVariable("NSIGHT_HUD_CAPTURE_KEY_AS_STRING", "F11");
        SetEnvironmentVariable("NSIGHT_HUD_CAPTURE_KEY_MODIFIER", "0");
        SetEnvironmentVariable("NUMBER_OF_PROCESSORS", "12");
        SetEnvironmentVariable("NVIDIA_PROCESS_INJECTION_CRASH_REPORTING", "1");
        SetEnvironmentVariable("NVTOOLSEXT_PATH", "C:/Program Files/NVIDIA Corporation/NvToolsExt/");
        SetEnvironmentVariable("NVTX_INJECTION64_PATH", "Nvda.Graphics.Interception.dll");
        SetEnvironmentVariable("NV_PROCESS_INJECTION_SUPPRESS_MODULE_LOAD", "GameOverlayRenderer.dll;GameOverlayRenderer64.dll;SteamOverlayVulkanLayer.dll;SteamOverlayVulkanLayer64.dll");
        SetEnvironmentVariable("NV_TPS_LAUNCH_ENV_HASH", "5316591265157291315");
        SetEnvironmentVariable("NV_TPS_LAUNCH_TOKEN", "Frame Debugger");
        SetEnvironmentVariable("OS", "Windows_NT");
        SetEnvironmentVariable("OneDrive", "C:/Users/hehe/OneDrive");
        SetEnvironmentVariable("PATHEXT", ".COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC");
        SetEnvironmentVariable("PROCESSOR_ARCHITECTURE", "AMD64");
        SetEnvironmentVariable("PROCESSOR_IDENTIFIER, AMD64 Family 25 Model 33 Stepping 0", "AuthenticAMD");
        SetEnvironmentVariable("PROCESSOR_LEVEL", "25");
        SetEnvironmentVariable("PROCESSOR_REVISION", "2100");
        SetEnvironmentVariable("PSModulePath", "C:/Program Files/WindowsPowerShell/Modules;C:/Windows/system32/WindowsPowerShell/v1.0/Modules");
        SetEnvironmentVariable("PUBLIC", "C:/Users/Public");
        SetEnvironmentVariable("Path", "C:/Program Files/Eclipse Adoptium/jdk-11.0.15.10-hotspot/bin;C:/ProgramData/Oracle/Java/javapath;C:/VulkanSDK/1.3.211.0/Bin;C:/Windows/system32;C:/Windows;C:/Windows/System32/Wbem;C:/Windows/System32/WindowsPowerShell/v1.0/;C:/Windows/System32/OpenSSH/;C:/Program Files (x86)/NVIDIA Corporation/PhysX/Common;C:/Program Files/dotnet/;C:/Program Files/VSCodium/bin;C:/ProgramData/chocolatey/bin;C:/Users/hehe/.cargo/bin;C:/Users/hehe/AppData/Local/Microsoft/WindowsApps;C:/Users/hehe/.dotnet/tools;C:/Users/hehe/anaconda3/Library/bin/;C:/Program Files/CMake/bin;C:/ffmpeg/bin;C:/Windows/Microsoft.NET/Framework/v4.0.30319/;");
        SetEnvironmentVariable("ProgramData", "C:/ProgramData");
        SetEnvironmentVariable("ProgramFiles", "C:/Program Files");
        SetEnvironmentVariable("ProgramFiles(x86)", "C:/Program Files (x86)");
        SetEnvironmentVariable("ProgramW6432", "C:/Program Files");
        SetEnvironmentVariable("SESSIONNAME", "Console");
        SetEnvironmentVariable("SystemDrive", "C:");
        SetEnvironmentVariable("SystemRoot", "C:/Windows");
        SetEnvironmentVariable("TEMP", "C:/Users/hehe/AppData/Local/Temp");
        SetEnvironmentVariable("TMP", "C:/Users/hehe/AppData/Local/Temp");
        SetEnvironmentVariable("USERDOMAIN", "DESKTOP-6ELBASE");
        SetEnvironmentVariable("USERDOMAIN_ROAMINGPROFILE", "DESKTOP-6ELBASE");
        SetEnvironmentVariable("USERNAME", "hehe");
        SetEnvironmentVariable("USERPROFILE", "C:/Users/hehe");
        SetEnvironmentVariable("VK_SDK_PATH", "C:/VulkanSDK/1.3.211.0");
        SetEnvironmentVariable("VULKAN_SDK", "C:/VulkanSDK/1.3.211.0");
        SetEnvironmentVariable("windir", "C:/Windows");

        auto e = LoadLibraryA("C:/Program Files/NVIDIA Corporation/Nsight Graphics 2022.2.1/target/windows-desktop-nomad-x64/Nomad.Injection.dll");
        auto q = LoadLibraryA("C:/Program Files/NVIDIA Corporation/Nsight Graphics 2022.2.1/target/windows-desktop-nomad-x64/Nvda.Graphics.Interception.dll");
        auto w = LoadLibraryA("C:/Program Files/NVIDIA Corporation/Nsight Graphics 2022.2.1/target/windows-desktop-nomad-x64/NvLog.dll");
        auto r = LoadLibraryA("C:/Program Files/NVIDIA Corporation/Nsight Graphics 2022.2.1/target/windows-desktop-nomad-x64/nvperf_grfx_target.dll");
    }

    //auto lib = LoadLibrary("C:/Windows/System32/vulkan-1.dll");
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
    }
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}
