# CMakeList.txt : CMake project for Raysterizer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.3)

include(FetchContent)

project(Raysterizer CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

#set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

macro(add_depend)
    SET(dep_name ${ARGV0})
    SET(endpoint_url ${ARGV1})
    SET(endpoint_tag ${ARGV2})
    SET(do_build_with_cmake ${ARGV3})

    FetchContent_Declare(
        ${dep_name}
        GIT_REPOSITORY ${endpoint_url}
        GIT_TAG        ${endpoint_tag}
        #GIT_SHALLOW true
    )

    FetchContent_GetProperties(${dep_name})

    if(NOT ${dep_name}_POPULATED)
        FetchContent_Populate(${dep_name})
	    message(STATUS "Working on ${dep_name}")

	    if(${do_build_with_cmake})
	        #set(MESSAGE_QUIET ON)
            add_subdirectory(${${dep_name}_SOURCE_DIR} ${${dep_name}_BINARY_DIR})
    	    #unset(MESSAGE_QUIET)
        else()
		    message("\tHeader only")
        endif()
    endif()

	#set_target_properties(${ARGV0} PROPERTIES FOLDER "Third-Party")
endmacro()

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_LTO OFF CACHE BOOL "" FORCE)

add_depend(glm https://github.com/g-truc/glm.git 0.9.9.6 FALSE)
add_depend(imgui https://github.com/ocornut/imgui.git v1.83 FALSE)
add_depend(imguizmo https://github.com/CedricGuillemet/ImGuizmo.git e3174578bdc99c715e51c5ad88e7d50b4eeb19b0 FALSE)
add_compile_definitions(VGIZMO_USES_GLM)
add_compile_definitions(IMGUIZMO_IMGUI_FOLDER=${imgui_SOURCE_DIR}/)
add_depend(imguizmo_quat https://github.com/BrutPitt/imGuIZMO.quat.git v3.0 FALSE)
add_depend(json https://github.com/nlohmann/json.git v3.10.5 TRUE)
#add_depend(vulkan_memory_allocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git cab4eb545c69e7f10b066ffee7340e1c8c10fd67 FALSE)
add_depend(vulkan_memory_allocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git 51c8b56011303e94840370089f816b19dbe7edf0 FALSE)
#add_depend(vk_bootstrap https://github.com/charles-lunarg/vk-bootstrap.git v0.5 TRUE)
add_depend(vk_bootstrap https://github.com/charles-lunarg/vk-bootstrap.git 65071da1817d846a1cff07f010f696003280a536 TRUE)
add_depend(xxhash https://github.com/Cyan4973/xxHash.git v0.8.0 FALSE)
add_depend(ctre https://github.com/hanickadot/compile-time-regular-expressions v3.4.1 FALSE)
add_depend(magic_enum https://github.com/Neargye/magic_enum v0.7.2 FALSE)
add_depend(nameof https://github.com/Neargye/nameof v0.10.0 TRUE)
add_depend(stb https://github.com/nothings/stb.git af1a5bc352164740c1cc1354942b1c6b72eacb8a FALSE)
add_depend(small_vector https://github.com/thelink2012/SmallVector.git master FALSE)

#set(Z3_BUILD_LIBZ3_SHARED OFF CACHE BOOL "" FORCE)
#set(Z3_SINGLE_THREADED ON CACHE BOOL "" FORCE)
#add_depend(z3_prover https://github.com/Z3Prover/z3 z3-4.8.10 TRUE)
#include("${z3_prover_BINARY_DIR}/Z3Config.cmake")

set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
set(TRACY_CALLSTACK ON CACHE BOOL "" FORCE)
add_depend(tracy https://github.com/wolfpld/tracy v0.8 TRUE)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_compile_definitions(_GLFW_WNDCLASSNAME="Raysterizer")
add_depend(glfw https://github.com/glfw/glfw.git 3.3 TRUE)

add_depend(minhook https://github.com/TsudaKageyu/minhook master TRUE)
add_depend(fmt https://github.com/fmtlib/fmt.git 7.1.2 TRUE)
add_depend(spdlog https://github.com/gabime/spdlog.git v1.7.0 TRUE)
add_depend(googletest https://github.com/google/googletest.git release-1.10.0 TRUE)
add_depend(benchmark https://github.com/google/benchmark.git v1.5.0 TRUE)
add_depend(parallel_hashmap https://github.com/greg7mdp/parallel-hashmap 1.34 TRUE)
add_depend(spirv_vm https://github.com/dfranx/SPIRV-VM 66a65d3a78ef798b2789f46242e65f89fb536f58 TRUE)

set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_depend(taskflow https://github.com/taskflow/taskflow v3.0.0 TRUE)
add_depend(concurrentqueue https://github.com/cameron314/concurrentqueue 3747268264d0fa113e981658a99ceeae4dad05b7 TRUE)

set(PEGTL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PEGTL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_depend(pegtl https://github.com/taocpp/PEGTL 3.0.0 TRUE)

set(ENABLE_HLSL ON CACHE BOOL "" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_RTTI FALSE CACHE BOOL "" FORCE)
set(ENABLE_EXCEPTIONS FALSE CACHE BOOL "" FORCE)
set(ENABLE_OPT TRUE CACHE BOOL "" FORCE)
#add_depend(glslang https://github.com/KhronosGroup/glslang.git master-tot TRUE)
#add_depend(glslang https://github.com/KhronosGroup/glslang.git 6d5916d4a91eb9c3b3be377382e2c1c12443fa11 TRUE)
add_depend(glslang https://github.com/KhronosGroup/glslang.git 2f5bc0b7414f5d063b9c2fcf231469aec1e549d3 TRUE)


set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
#add_depend(spirv_headers https://github.com/KhronosGroup/SPIRV-Headers.git 1.5.4 TRUE)
add_depend(spirv_headers https://github.com/KhronosGroup/SPIRV-Headers.git f027d53ded7e230e008d37c8b47ede7cd308e19d FALSE)


set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_EXCEPTIONS_ON_MSVC OFF CACHE BOOL "" FORCE)
set(SPIRV_TOOLS_BUILD_STATIC ON CACHE BOOL "" FORCE)

#set(SPIRV-Headers_SOURCE_DIR "${spirv_headers_SOURCE_DIR}")
#set(SPIRV_HEADER_DIR "${spirv_headers_SOURCE_DIR}")

set(SPIRV-Headers_SOURCE_DIR ${spirv_headers_SOURCE_DIR})
#add_depend(spirv_tools https://github.com/KhronosGroup/SPIRV-Tools.git v2020.5 TRUE)
#add_depend(spirv_tools https://github.com/KhronosGroup/SPIRV-Tools.git 8a0ebd40f86d1f18ad42ea96c6ac53915076c3c7 TRUE)
add_depend(spirv_tools https://github.com/KhronosGroup/SPIRV-Tools.git b27b1afd12d05bf238ac7368bb49de73cd620a8e TRUE)

set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)

#add_depend(spirv_cross https://github.com/KhronosGroup/SPIRV-Cross.git MoltenVK-1.1.5 TRUE)
add_depend(spirv_cross https://github.com/KhronosGroup/SPIRV-Cross.git 0b51794f0142a3124f4e351cfc0616a48268ba97 TRUE)

set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
#set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_COPYRIGHT_CHECK OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)
#set(SHADERC_GLSLANG_DIR ${glslang_SOURCE_DIR} CACHE STRING "" FORCE)
add_depend(shaderc https://github.com/google/shaderc.git v2020.4 TRUE)

set(FOSSILIZE_CLI OFF CACHE BOOL "" FORCE)
set(FOSSILIZE_TESTS OFF CACHE BOOL "" FORCE)
add_depend(fossilize https://github.com/ValveSoftware/Fossilize fa96c938bafb26dc0794942a16eb49f91acb1543 TRUE)

add_depend(mm_loader https://github.com/tishion/mmLoader 0c522e7a39ed29c059534fa9e14a09b2ad99d746 FALSE)

add_library(mm_loader_lib STATIC ${mm_loader_SOURCE_DIR}/src/mmLoader/mmLoader.c)

target_include_directories(mm_loader_lib PUBLIC 
    ${mm_loader_SOURCE_DIR}/src/mmLoader
)

#set(SPIRV-Tools ${spirv_tools})
#set(SPIRV-Tools_SOURCE_DIR "${spirv_tools_SOURCE_DIR}")

#include_directories(${imgui_SOURCE_DIR})
set(imgui_source ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_demo.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp ${imgui_SOURCE_DIR}/imgui_tables)
set(imguizmo_source ${imguizmo_SOURCE_DIR}/GraphEditor.cpp ${imguizmo_SOURCE_DIR}/ImCurveEdit.cpp ${imguizmo_SOURCE_DIR}/ImGradient.cpp ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp ${imguizmo_SOURCE_DIR}/ImSequencer.cpp)
set(imguizmo_quat_source ${imguizmo_quat_SOURCE_DIR}/imGuIZMO.quat/imGuIZMOquat.cpp)

add_library(imgui_lib STATIC ${imgui_source} ${imguizmo_source} ${imguizmo_quat_source})

set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
add_depend(spirv_reflect https://github.com/KhronosGroup/SPIRV-Reflect.git bd64d605bcfc6d12b5fe652c014d21990963aed2 FALSE)
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(imgui_lib PUBLIC 
    ${glm_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imguizmo_SOURCE_DIR}
    ${imguizmo_quat_SOURCE_DIR}/imGuIZMO.quat
	${glfw_SOURCE_DIR}/include
)

target_link_libraries(imgui_lib PUBLIC
    Vulkan::Vulkan
)

#add_library(imguizmo_quat_lib STATIC ${imguizmo_quat_source})

#target_include_directories(imguizmo_quat_lib PUBLIC 
#    ${imgui_SOURCE_DIR}
#    ${imguizmo_quat_SOURCE_DIR}/imGuIZMO.quat
#)

#target_link_libraries(imguizmo_quat_lib PUBLIC
#    imgui_lib
#)

if(MSVC)
  add_definitions(/bigobj /MP)
else()
  add_definitions(-Wa,-mbig-obj)
endif()

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB_RECURSE raysterizer_source ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/include/*.h)
message(STATUS "Working on ${raysterizer_source}")

# Add source to this project's executable.
#add_library(RaysterizerEngine SHARED ${raysterizer_source})
add_library(RaysterizerEngine STATIC ${raysterizer_source})

target_compile_options(RaysterizerEngine PRIVATE "$<$<CONFIG:Release>:/Zi>")
target_link_options(RaysterizerEngine PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(RaysterizerEngine PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(RaysterizerEngine PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

if(EXISTS ${PROJECT_SOURCE_DIR}/third-party/NVIDIA_Nsight_Aftermath_SDK)
    add_compile_definitions(NVIDIA_NSIGHT_AFTERMATH_SDK_ENABLED)

    set(NsightAftermath_dll ${PROJECT_SOURCE_DIR}/third-party/NVIDIA_Nsight_Aftermath_SDK/lib/x64/GFSDK_Aftermath_Lib.x64.dll)
    set(NsightAftermath_lib ${PROJECT_SOURCE_DIR}/third-party/NVIDIA_Nsight_Aftermath_SDK/lib/x64/GFSDK_Aftermath_Lib.x64.lib)
    set(NsightAftermath_include ${PROJECT_SOURCE_DIR}/third-party/NVIDIA_Nsight_Aftermath_SDK/include)
    #file(GLOB_RECURSE NsightAftermath_include ${PROJECT_SOURCE_DIR}/third-party/NVIDIA_Nsight_Aftermath_SDK/include/*.h)

    set(NsightAftermath_inc ${NsightAftermath_include}/GFSDK_Aftermath_Lib.inc)

    add_depend(na_samples https://github.com/NVIDIA/nsight-aftermath-samples 6f985096067e4efdb0b0d0b78f2e64dacd591798 FALSE)
    set(NsightAftermathSamples_source ${na_samples_SOURCE_DIR}/VkHelloNsightAftermath/NsightAftermathGpuCrashTracker.cpp ${na_samples_SOURCE_DIR}/VkHelloNsightAftermath/NsightAftermathShaderDatabase.cpp)
    set(NsightAftermathSamples_include ${na_samples_SOURCE_DIR}/VkHelloNsightAftermath)

    add_library(NsightAftermathSamples STATIC ${NsightAftermathSamples_source} ${PROJECT_SOURCE_DIR}/third-party/gfsdk_defines.cpp)

    find_package(PythonInterp REQUIRED)
    add_custom_command(OUTPUT ${NsightAftermath_inc}
        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/third-party/convert_dll_into_bytes.py ${NsightAftermath_dll} -O ${NsightAftermath_inc} -N ${NsightAftermath_include}/GFSDK_Aftermath_Defines.h -A ${NsightAftermath_include}/GFSDK_Aftermath_GpuCrashDump.h -B ${NsightAftermath_include}/GFSDK_Aftermath_GpuCrashDumpDecoding.h
        DEPENDS ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/third-party/convert_dll_into_bytes.py ${NsightAftermath_dll} ${NsightAftermath_inc}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

    target_compile_definitions(NsightAftermathSamples PUBLIC
        VK_USE_PLATFORM_WIN32_KHR=1
    )

    target_include_directories(NsightAftermathSamples PUBLIC
        ${VulkanHeaders_INCLUDE_DIR}

        ${NsightAftermath_inc}
        ${NsightAftermath_include}
        ${NsightAftermathSamples_include}
    )

    target_link_libraries(NsightAftermathSamples PUBLIC
        Vulkan::Vulkan

        #${NsightAftermath_lib}
    )

    target_link_libraries(RaysterizerEngine PUBLIC
        NsightAftermathSamples
    )

    target_compile_definitions(RaysterizerEngine PUBLIC
        NSIGHT_AFTERMATH_INC_PATH="${NsightAftermath_inc}"
    )
endif()

target_include_directories(RaysterizerEngine PUBLIC 
    .

	${glm_SOURCE_DIR}
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends
	${imguizmo_SOURCE_DIR}
    ${imguizmo_quat_SOURCE_DIR}
	${json_SOURCE_DIR}/include
	${minhook_SOURCE_DIR}/include
	${vulkan_memory_allocator_SOURCE_DIR}/include
	${vulkan_memory_allocator_SOURCE_DIR}/src
    ${vk_bootstrap_SOURCE_DIR}/src
	${xxhash_SOURCE_DIR}
	${ctre_SOURCE_DIR}/single-header

	${glfw_SOURCE_DIR}/include
	${fmt_SOURCE_DIR}
	${spdlog_SOURCE_DIR}
	${googletest_SOURCE_DIR}
	${benchmark_SOURCE_DIR}
	${parallel_hashmap_SOURCE_DIR}
    ${pegtl_SOURCE_DIR}/include
	${tracy_SOURCE_DIR}
    ${glslang_SOURCE_DIR}
    ${spirv_headers_SOURCE_DIR}/include
    ${spirv_tools_SOURCE_DIR}/include
    ${spirv_tools_SOURCE_DIR}
    ${spirv_tools_BINARY_DIR}
    ${shaderc_SOURCE_DIR}
    ${shaderc_SOURCE_DIR}/libshaderc_util/include
    ${spirv_cross_SOURCE_DIR}
    ${spirv_vm_SOURCE_DIR}/inc
    ${spirv_reflect_SOURCE_DIR}
    ${magic_enum_SOURCE_DIR}/include
    ${nameof_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${small_vector_SOURCE_DIR}

    ${taskflow_SOURCE_DIR}
    ${concurrentqueue_SOURCE_DIR}

    ${fossilize_SOURCE_DIR}

    ${mm_loader_SOURCE_DIR}/src/mmLoader

    #${Z3_CXX_INCLUDE_DIRS}
)

target_link_libraries(RaysterizerEngine PUBLIC
    ${CMAKE_DL_LIBS}
    Threads::Threads
    Vulkan::Vulkan

    nlohmann_json
    imgui_lib

    vk-bootstrap
    minhook
    glfw
    fmt
    spdlog
	pegtl
    TracyClient
    #glslang
    SPIRV
    #spirv_headers
    shaderc
    SPIRV-Tools-static

    #spirv-cross-c
    spirv-cross-cpp

    SPIRVVM

    Taskflow
    concurrentqueue
    
    fossilize
    mm_loader_lib
    #${Z3_LIBRARIES}
)

include(copy_dll)
target_precompile_headers(RaysterizerEngine PUBLIC ${PROJECT_SOURCE_DIR}/include/pch.h)
add_subdirectory(raysterizer)
