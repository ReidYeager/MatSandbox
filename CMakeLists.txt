
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
  src/main.cpp
  src/defines.h
  src/window.h
  src/window.cpp
)

target_include_directories (MatSandbox PUBLIC
  ./
  ./src/imgui/
  ExternalDeps
  ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(MatSandbox
  ExternalDeps
  ${Vulkan_LIBRARIES}/../../Lib/vulkan-1.lib
)

target_compile_definitions(MatSandbox PRIVATE GAME_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/")

