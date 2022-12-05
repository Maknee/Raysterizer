#include "wgl_opengl_hooks.h"

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

EXTERN_HOOK_EXPORT int WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", hdc, ppfd);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglChoosePixelFormat);
	auto result = redirect_function(hdc, ppfd);
	return result;
}

EXTERN_HOOK_EXPORT int WINAPI wglGetPixelFormat(HDC hdc)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", hdc);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglGetPixelFormat);
	auto result = redirect_function(hdc);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglSetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR* ppfd)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", hdc, iPixelFormat, ppfd);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglSetPixelFormat);
	auto result = redirect_function(hdc, iPixelFormat, ppfd);
	return result;
}

EXTERN_HOOK_EXPORT int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", hdc, iPixelFormat, nBytes, ppfd);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglDescribePixelFormat);
	auto result = redirect_function(hdc, iPixelFormat, nBytes, ppfd);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglDescribeLayerPlane);
	auto result = redirect_function(hdc, iPixelFormat, iLayerPlane, nBytes, plpd);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL b)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglRealizeLayerPalette);
	auto result = redirect_function(hdc, iLayerPlane, b);
	return result;
}

EXTERN_HOOK_EXPORT int WINAPI wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF* pcr)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglGetLayerPaletteEntries);
	auto result = redirect_function(hdc, iLayerPlane, iStart, cEntries, pcr);
	return result;
}

EXTERN_HOOK_EXPORT int WINAPI wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF* pcr)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglSetLayerPaletteEntries);
	auto result = redirect_function(hdc, iLayerPlane, iStart, cEntries, pcr);
	return result;
}

EXTERN_HOOK_EXPORT HGLRC WINAPI wglCreateContext(HDC hdc)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglCreateContext);
	auto result = redirect_function(hdc);
	return result;
}

EXTERN_HOOK_EXPORT HGLRC WINAPI wglCreateLayerContext(HDC hdc, int iLayerPlane)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglCreateLayerContext);
	auto result = redirect_function(hdc, iLayerPlane);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglCopyContext);
	auto result = redirect_function(hglrcSrc, hglrcDst, mask);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglDeleteContext(HGLRC hglrc)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglDeleteContext);
	auto result = redirect_function(hglrc);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglShareLists(HGLRC hglrc1, HGLRC hglrc2)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglShareLists);
	auto result = redirect_function(hglrc1, hglrc2);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglMakeCurrent);
	auto result = redirect_function(hdc, hglrc);
	return result;
}

EXTERN_HOOK_EXPORT HDC WINAPI wglGetCurrentDC()
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglGetCurrentDC);
	auto result = redirect_function();
	return result;
}

EXTERN_HOOK_EXPORT HGLRC WINAPI wglGetCurrentContext()
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglGetCurrentContext);
	auto result = redirect_function();
	return result;
}

/*
BOOL WINAPI wglDestroyPbufferARB(HPBUFFERARB hPbuffer)
{
	RETURN_DUMMY();
}

BOOL WINAPI wglQueryPbufferARB(HPBUFFERARB hPbuffer, int iAttribute, int* piValue)
{
	RETURN_DUMMY();
}

HDC WINAPI wglGetPbufferDCARB(HPBUFFERARB hPbuffer)
{
	RETURN_DUMMY();
}

int WINAPI wglReleasePbufferDCARB(HPBUFFERARB hPbuffer, HDC hdc)
{
	RETURN_DUMMY();
}

BOOL WINAPI wglSwapIntervalEXT(int interval)
{
	RETURN_DUMMY();
}

int WINAPI wglGetSwapIntervalEXT()
{
	RETURN_DUMMY();
}
*/

EXTERN_HOOK_EXPORT BOOL WINAPI wglSwapBuffers(HDC hdc)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglSwapBuffers);
	auto result = redirect_function(hdc);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglSwapLayerBuffers(HDC hdc, UINT flags)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglSwapLayerBuffers);
	auto result = redirect_function(hdc, flags);
	return result;
}

EXTERN_HOOK_EXPORT DWORD WINAPI wglSwapMultipleBuffers(UINT cNumBuffers, const WGLSWAP* pBuffers)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglSwapMultipleBuffers);
	auto result = redirect_function(cNumBuffers, pBuffers);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontBitmapsA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglUseFontBitmapsW);
	auto result = redirect_function(hdc, dw1, dw2, dw3);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontBitmapsW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglUseFontBitmapsW);
	auto result = redirect_function(hdc, dw1, dw2, dw3);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontOutlinesA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglUseFontOutlinesA);
	auto result = redirect_function(hdc, dw1, dw2, dw3, f1, f2, i, pgmf);
	return result;
}

EXTERN_HOOK_EXPORT BOOL WINAPI wglUseFontOutlinesW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglUseFontOutlinesW);
	auto result = redirect_function(hdc, dw1, dw2, dw3, f1, f2, i, pgmf);
	return result;
}

EXTERN_HOOK_EXPORT PROC WINAPI wglGetProcAddress(LPCSTR lpszProc)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", lpszProc);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglGetProcAddress);
	auto result = redirect_function(lpszProc);
	return result;
}

EXTERN_HOOK_EXPORT PROC WINAPI wglGetDefaultProcAddress(LPCSTR lpszProc)
{
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(wglGetDefaultProcAddress);
	auto result = redirect_function(lpszProc);
	return result;
}


