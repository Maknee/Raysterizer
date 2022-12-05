// Raysterizer.cpp : Defines the entry point for the application.
//

#include "include/pch.h"

extern "C" BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        //MessageBox(NULL, "It works!", "Status", MB_OK);
        DisableThreadLibraryCalls(hinstDLL);
    }
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}