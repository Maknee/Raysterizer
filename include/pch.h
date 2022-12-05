#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

// Symbols that leak and cause problems:
#undef FAR
#undef NEAR
#endif

//////////////////////////////////////////////////////////////////////////

#include <execution>
#include <algorithm>
#include <array>
#include <chrono>
#include <map>
#include <set>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <string_view>
#include <variant>
#include <condition_variable>
#include <charconv>
#include <future>

#include <cassert>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>

//////////////////////////////////////////////////////////////////////////

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VK_ENABLE_BETA_EXTENSIONS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_TYPESAFE_CONVERSION

#define VULKAN_HPP_NO_EXCEPTIONS

// Disable internal assertion
#define VULKAN_HPP_ASSERT(x) 

#include <vulkan/vulkan.hpp>

//////////////////////////////////////////////////////////////////////////

#include <vk_mem_alloc.h>

//////////////////////////////////////////////////////////////////////////

#define GLFW_INCLUDE_VULKAN
//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//////////////////////////////////////////////////////////////////////////

#include <StandAlone/ResourceLimits.h>
#include <StandAlone/Worklist.h>
#include <StandAlone/DirStackFileIncluder.h>

#include <glslang/Include/ShHandle.h>
#include <glslang/public/ShaderLang.h>
#include <glslang/MachineIndependent/reflection.h>
#include <glslang/MachineIndependent/LiveTraverser.h>
//#include <glslang/MachineIndependent/intermOut.cpp>
#include <glslang/MachineIndependent/ParseHelper.h>
#include <glslang/MachineIndependent/preprocessor/PpContext.h>
#include <glslang/MachineIndependent/ScanContext.h>
#include <glslang/MachineIndependent/Scan.h>

#include <SPIRV/Disassemble.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/GLSL.std.450.h>

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <spirv_reflect.hpp>
#include <spirv_reflect.h>

#include <spirv-tools/libspirv.h>
#include <spirv-tools/optimizer.hpp>

#include <source/name_mapper.h>
#include <source/opt/basic_block.h>
#include <source/opt/def_use_manager.h>
#include <source/opt/mem_pass.h>
#include <source/opt/module.h>
#include <source/opt/pass.h>
#include <source/opt/dominator_analysis.h>

//////////////////////////////////////////////////////////////////////////

extern "C"
{
	#include <spvm/context.h>
	#include <spvm/state.h>
	#include <spvm/ext/GLSL450.h>
}

//////////////////////////////////////////////////////////////////////////

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <ImGuizmo.h>
#include <imGuIZMOquat.h>

//////////////////////////////////////////////////////////////////////////

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/string_cast.hpp>

//////////////////////////////////////////////////////////////////////////

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//////////////////////////////////////////////////////////////////////////

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

//////////////////////////////////////////////////////////////////////////

#include <fmt/core.h>
using namespace fmt::literals;

//////////////////////////////////////////////////////////////////////////

#include <Minhook.h>

//////////////////////////////////////////////////////////////////////////

#include <parallel_hashmap/phmap.h>
#include <parallel_hashmap/btree.h>
using phmap::flat_hash_map;

//////////////////////////////////////////////////////////////////////////

#include <magic_enum.hpp>

//////////////////////////////////////////////////////////////////////////

#include <nameof.hpp>

//////////////////////////////////////////////////////////////////////////

//#include <c++/z3++.h>

//////////////////////////////////////////////////////////////////////////

#define XXH_INLINE_ALL
#define XXH_FORCE_MEMORY_ACCESS 2
#include <xxhash.h>

//////////////////////////////////////////////////////////////////////////

#include <ctre.hpp>

//////////////////////////////////////////////////////////////////////////

#include <tao/pegtl.hpp>

//////////////////////////////////////////////////////////////////////////

#include <concurrentqueue.h>

//////////////////////////////////////////////////////////////////////////

#include <taskflow/taskflow.hpp>

//////////////////////////////////////////////////////////////////////////

#include "third-party/llvm-expected.h"
template <typename T>
using Expected = llvm::Expected<T>;
using Error = llvm::Error;

//////////////////////////////////////////////////////////////////////////

#include <stb_image.h>

//////////////////////////////////////////////////////////////////////////

#include <SmallVector.h>
template <typename T, unsigned N>
using small_vector = llvm_vecsmall::SmallVector<T, N>;

//////////////////////////////////////////////////////////////////////////

#include <VkBootstrap.h>

//////////////////////////////////////////////////////////////////////////

#include <fossilize.hpp>
#include <fossilize_db.hpp>

//////////////////////////////////////////////////////////////////////////

#define ENABLE_PROFILER
#ifdef ENABLE_PROFILER
	#define TRACY_ENABLE
	//#define TRACY_ON_DEMAND
	//#define TRACY_CALLSTACK
	#include <TracyVulkan.hpp>
	#include <Tracy.hpp>
#endif

//////////////////////////////////////////////////////////////////////////

#include <mmLoader.h>

//////////////////////////////////////////////////////////////////////////

#ifdef NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED
	#include <GFSDK_Aftermath.h>
	#include <GFSDK_Aftermath_Defines.h>
	#include <GFSDK_Aftermath_GpuCrashDump.h>
	#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

	#include <NsightAftermathGpuCrashTracker.h>
	#include <NsightAftermathShaderDatabase.h>
#endif

//////////////////////////////////////////////////////////////////////////

#include <include/config.h>
#include <include/constants.h>
#include <include/hash.h>
#include <include/utils.h>

#include <include/profiler.h>

#include <include/pointer_view.h>
#include <include/wrappers.h>
#include <include/container.h>

#include <include/vulkan_utils.h>
#include <include/vma_allocator.h>
#include <include/vulkan_dispatch_loader.h>

#include <include/buffer.h>
#include <include/image.h>
#include <include/image_view.h>
#include <include/sampler.h>
#include <include/texture.h>
#include <include/shader_reflection.h>
#include <include/shader_module.h>
#include <include/render_pass.h>
#include <include/descriptor_set_layout.h>
#include <include/descriptor_pool.h>
#include <include/pipeline_layout.h>
#include <include/pipeline.h>
#include <include/acceleration_structure.h>
#include <include/frame_buffer.h>

#include <include/descriptor_set.h>

#include <include/fence.h>
#include <include/semaphore.h>
#include <include/command_pool.h>
#include <include/command_buffer.h>

#include <include/shader_binding_table.h>

#include <include/shader_module_manager.h>
#include <include/queue_batch_manager.h>
#include <include/resource_manager.h>

#include <include/render_frame.h>

#include <include/disk_cacher.h>

#include <include/vulkan_window.h>
#include <include/context.h>

#include <include/frame_type_traits.h>
#include <include/frame_pass_entry.h>
#include <include/frame_node.h>
#include <include/frame_resource_entry.h>
#include <include/frame_resource_node.h>
#include <include/frame_graph.h>
