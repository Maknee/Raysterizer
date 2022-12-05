#pragma once

#include "pch.h"

#define APP_NAME "Raysterizer"

#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 0

#define APP_VERSION VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH)

#define WINDOW_NAME APP_NAME
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
//#define WINDOW_WIDTH 542
//#define WINDOW_HEIGHT 450

// Internal enables

//#define ENABLE_ANY_HIT

#define RAYSTERIZER_DEBUG_MARKER

#define ENABLE_NOT_IMPLEMENTED_CHECK_DRAW_OPENGL 0
#define ENABLE_CHECK_SPIRV_GL_POSITION 0
//#define ENABLE_CHECK_SPIRV_GL_POSITION 1
#define RAYSTERIZER_COMPILE_DUMP_DISASSEMBLY 0
#define RAYSTERIZER_TLAS_INSTANCE_TRANSPOSE_TRANSFORM

//#define RAYSTERIZER_FORCE_FRAG_COLOR "vec4(1.0f, 1.0f, 1.0f, 1.0f)"
//#define RAYSTERIZER_FORCE_FRAG_COLOR "vec4(distance, 0.0f, 0.0f, 1.0f)"

//constexpr bool RAYSTERIZER_OPENGL_INTERLOP_BUFFER_OPTIMIZATION = true;
constexpr bool RAYSTERIZER_OPENGL_INTERLOP_BUFFER_OPTIMIZATION = false;

//constexpr bool RAYSTERIZER_OPENGL_INTERLOP_TEXTURE_OPTIMIZATION = true;
constexpr bool RAYSTERIZER_OPENGL_INTERLOP_TEXTURE_OPTIMIZATION = false;

inline static std::vector<std::string> disable_opengl_callback_check_for_programs{ "Craft.exe", "java.exe" };
constexpr bool RAYSTERIZER_VULKAN_CHECK_COPY_REFLECTION_DATA_TYPE_SIZE = false;
constexpr bool RAYSTERIZER_OPENGL_GLFW_POLL_EVENTS = false;

constexpr std::size_t RAYSTERIZER_MAX_RECURSION_DEPTH = 16;

constexpr std::size_t UNDEFINED_ID = -1;

inline RaysterizerEngine::Context c;
inline std::shared_ptr<VulkanWindow> vulkan_window;

// VM RELATED

#define RAYSTERIZER_RUN_SPIRV_VM_ASYNC false
#define RAYSTERIZER_RUN_TRANSFORMATION_BUFFER_SIZE 1024

// PROFILER
constexpr uint32_t fnv1a(const char* s, uint32_t h = 0x811C9DC5) {
    return !*s ? h : fnv1a(s + 1, (h ^ (uint8_t)*s) * 0x01000193);
}

#define ScopedCPUProfileOpenGLConst(x) ScopedCPUProfileColorDepth(x, OPENGL_PROFILE_COLOR, DEFAULT_PROFILER_DEPTH);
#define ScopedCPUProfileOpenGL(x) ScopedCPUProfileColorDepth(x, OPENGL_PROFILE_COLOR ^ fnv1a(x), DEFAULT_PROFILER_DEPTH);
#define ScopedCPUProfileOpenGLCurrentFunction() ScopedCPUProfileOpenGL(__FUNCTION__)

#ifndef NDEBUG

#define OPENGL_DEBUG_LOG_FUNCTION(...) \
	DEBUG(__VA_ARGS__); \
	ScopedCPUProfileOpenGLCurrentFunction(); \

#else

#define OPENGL_DEBUG_LOG_FUNCTION(...)

#endif

