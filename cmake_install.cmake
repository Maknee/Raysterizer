# Install script for directory: C:/Users/hehe/Desktop/raysterizer

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Raysterizer")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/hehe/Desktop/raysterizer/_deps/json-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/vk_bootstrap-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/nameof-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/tracy-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/glfw-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/minhook-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/fmt-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/spdlog-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/googletest-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/benchmark-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/parallel_hashmap-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/spirv_vm-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/taskflow-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/concurrentqueue-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/pegtl-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/glslang-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/spirv_tools-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/spirv_cross-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/shaderc-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/_deps/fossilize-build/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/tests/cmake_install.cmake")
  include("C:/Users/hehe/Desktop/raysterizer/Raysterizer/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/hehe/Desktop/raysterizer/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
