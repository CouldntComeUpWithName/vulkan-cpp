cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 20)

project(VkRendererCpp)


set(SOURCES 
  "src/main.cpp" 
  "src/validation_layers.hpp" "src/validation_layers.cpp"
  "src/vulkan.hpp"
)
add_executable(demo ${SOURCES})
target_include_directories(demo PRIVATE "$ENV{VULKAN_SDK}/Include/")


target_link_libraries(demo PUBLIC Vulkan::Vulkan PRIVATE glfw PRIVATE fmt::fmt)

find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
  message("Couldn't found Vulkan SDK")
endif()
if(MSVC)
  add_compile_options(/MP /W4 /EHsc- /GR-)
else()
  # add_compile_options(-ftime-trace -Ofast -std=c++23 -S)
  add_compile_options(-Wall -fno-rtti -fno-exceptions -ftime-trace -Wextra -Wpedantic) # -mllvm -unroll-threshold=10000
endif()

target_precompile_headers(demo 
PUBLIC
  <iostream>
  <optional>
  <filesystem>
  <vector>
  <chrono>
  <utility>
  <cstdint>
  <memory>
  <span>
  <cstring>
  <source_location>
  <glm/glm.hpp>
  <fmt/core.h>
)

# target_include_directories(demo PRIVATE ${FMT_INC_DIR})

add_subdirectory(3rd_party)


