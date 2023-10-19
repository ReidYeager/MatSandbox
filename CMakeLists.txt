
cmake_minimum_required (VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("MatSandbox")

add_subdirectory ("external/")

file(GLOB_RECURSE HeaderFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE HeaderPpFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
file(GLOB_RECURSE CSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE CppSourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(MatSandbox ${HeaderFiles} ${HeaderPpFiles} ${CSourceFiles} ${CppSourceFiles})

target_include_directories (MatSandbox PUBLIC
  ./
  ExternalDeps
)

target_link_libraries(MatSandbox
  ExternalDeps
)

target_compile_definitions(MatSandbox PRIVATE GAME_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/")

