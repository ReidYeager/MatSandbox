
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

project ("MatSandbox")

add_subdirectory ("external/")

#file(GLOB_RECURSE HeaderFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
#file(GLOB_RECURSE HeaderPpFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
#file(GLOB_RECURSE CSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
#file(GLOB_RECURSE CppSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE ImguiFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/*")

add_executable(MatSandbox
  ${ImguiFiles}

  # App =====
  src/defines.h
  src/common.h

  src/main.cpp
  src/core_initialization.cpp
  src/core_mainLoop.cpp
  src/core_shutdown.cpp

  src/camera.cpp

  src/compilation.cpp
  src/material.cpp

  src/mesh.cpp

  src/ui.cpp

  external/imgui/imgui.cpp
  external/imgui/imgui_demo.cpp
  external/imgui/imgui_draw.cpp
  external/imgui/imgui_tables.cpp
  external/imgui/imgui_widgets.cpp
  external/imgui/backends/imgui_impl_vulkan.cpp
  external/imgui/backends/imgui_impl_win32.cpp
)

target_include_directories (MatSandbox PUBLIC
  ./
  ExternalDeps
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(MatSandbox
  ExternalDeps
  ${Vulkan_LIBRARIES}/../../Lib/vulkan-1.lib
)

target_compile_definitions(MatSandbox PRIVATE GAME_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/")
target_compile_definitions(MatSandbox PRIVATE VULKAN_COMPILER="${Vulkan_LIBRARIES}/../../bin/glslc.exe")

