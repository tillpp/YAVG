cmake_minimum_required(VERSION 3.13)  
include(FetchContent)
project(YAVG)
set(CMAKE_CXX_STANDARD 20)           


# sources
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
       ${PROJECT_SOURCE_DIR}/src/**.cpp
       ${PROJECT_SOURCE_DIR}/src/**.c
       ${PROJECT_SOURCE_DIR}/libs/src/glad.c)

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

FetchContent_MakeAvailable(glfw)


include_directories(${CMAKE_SOURCE_DIR}/libs/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/lib)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable (YAVG ${SOURCE_FILES})
target_link_libraries(YAVG glfw)

