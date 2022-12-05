#pragma once

#include "pch.h"

#undef wglChoosePixelFormat
#undef wglGetPixelFormat
#undef wglSetPixelFormat
#undef wglDescribePixelFormat
#undef wglDescribeLayerPlane
#undef wglRealizeLayerPalette
#undef wglGetLayerPaletteEntries
#undef wglSetLayerPaletteEntries
#undef wglCreateContext
#undef wglCreateLayerContext
#undef wglCopyContext
#undef wglDeleteContext
#undef wglShareLists
#undef wglMakeCurrent
#undef wglGetCurrentDC
#undef wglSwapBuffers
#undef wglSwapLayerBuffers
#undef wglSwapMultipleBuffers
#undef wglSwapLayerBuffers
#undef wglUseFontBitmapsA
#undef wglUseFontBitmapsW
#undef wglUseFontOutlinesA
#undef wglUseFontOutlinesW
#undef wglGetProcAddress
#undef wglGetDefaultProcAddress

EXTERN_HOOK_EXPORT int WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd);
EXTERN_HOOK_EXPORT int WINAPI wglGetPixelFormat(HDC hdc);
EXTERN_HOOK_EXPORT BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR* ppfd);
EXTERN_HOOK_EXPORT int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);
EXTERN_HOOK_EXPORT BOOL WINAPI wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd);
EXTERN_HOOK_EXPORT BOOL WINAPI wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL b);
EXTERN_HOOK_EXPORT int WINAPI wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF* pcr);
EXTERN_HOOK_EXPORT int WINAPI wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF* pcr);
EXTERN_HOOK_EXPORT HGLRC WINAPI wglCreateContext(HDC hdc);
EXTERN_HOOK_EXPORT HGLRC WINAPI wglCreateLayerContext(HDC hdc, int iLayerPlane);
EXTERN_HOOK_EXPORT BOOL WINAPI wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask);
EXTERN_HOOK_EXPORT BOOL WINAPI wglDeleteContext(HGLRC hglrc);
EXTERN_HOOK_EXPORT BOOL WINAPI wglShareLists(HGLRC hglrc1, HGLRC hglrc2);
EXTERN_HOOK_EXPORT BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc);
EXTERN_HOOK_EXPORT HDC WINAPI wglGetCurrentDC();
EXTERN_HOOK_EXPORT HGLRC WINAPI wglGetCurrentContext();
EXTERN_HOOK_EXPORT BOOL WINAPI wglSwapBuffers(HDC hdc);
EXTERN_HOOK_EXPORT BOOL WINAPI wglSwapLayerBuffers(HDC hdc, UINT flags);
EXTERN_HOOK_EXPORT DWORD WINAPI wglSwapMultipleBuffers(UINT cNumBuffers, const WGLSWAP* pBuffers);
EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontBitmapsA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3);
EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontBitmapsW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3);
EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontOutlinesA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf);
EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontOutlinesW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf);
EXTERN_HOOK_EXPORT PROC WINAPI wglGetProcAddress(LPCSTR lpszProc);
EXTERN_HOOK_EXPORT PROC WINAPI wglGetDefaultProcAddress(LPCSTR lpszProc);
