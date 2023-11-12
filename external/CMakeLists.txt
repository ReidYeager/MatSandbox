
cmake_minimum_required (VERSION 3.20)

add_subdirectory ("Peridot") # Math
add_subdirectory ("Lapis") # Platform
add_subdirectory ("Opal") # Graphics

add_library (ExternalDeps INTERFACE)

target_include_directories (ExternalDeps INTERFACE
  Peridot/include/
  Lapis/include/
  Opal/include/
  imgui/
  ./
)

target_link_libraries(ExternalDeps INTERFACE
  Peridot
  Lapis
  Opal
)
