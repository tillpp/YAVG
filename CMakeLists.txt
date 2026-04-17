cmake_minimum_required(VERSION 3.13)  
include(FetchContent)
project(YAVG)
set(CMAKE_CXX_STANDARD 20)           


# sources
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
       ${PROJECT_SOURCE_DIR}/src/**.cpp
       ${PROJECT_SOURCE_DIR}/src/**.c)

if(WIN32)
       set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/windows)
       set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/windows)
       set(ARCHIVE_OUTPUT_DIRECTORY       ${CMAKE_SOURCE_DIR}/bin/windows)
else()
       set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/linux)
       set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/linux)
       set(ARCHIVE_OUTPUT_DIRECTORY       ${CMAKE_SOURCE_DIR}/bin/linux)
endif(WIN32)


FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_Declare(
  vulkan
  URL https://sdk.lunarg.com/sdk/download/1.4.341.1/linux/vulkansdk-linux-x86_64-1.4.341.1.tar.xz
)

FetchContent_MakeAvailable(glfw vulkan)

add_library(vulkan SHARED IMPORTED)
set_target_properties(vulkan PROPERTIES
    IMPORTED_LOCATION "${vulkan_SOURCE_DIR}/x86_64/lib/libvulkan.so"
    INTERFACE_INCLUDE_DIRECTORIES "${vulkan_SOURCE_DIR}/x86_64/include"
)

include_directories(${CMAKE_SOURCE_DIR}/libs/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/lib)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable (YAVG ${SOURCE_FILES})
target_link_libraries(YAVG glfw)
target_link_libraries(YAVG vulkan)

