cmake_minimum_required(VERSION 3.13)  
include(FetchContent)
project(YAVOG)
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

set(BIN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_Declare(
  vulkan
  URL https://sdk.lunarg.com/sdk/download/1.4.341.1/linux/vulkansdk-linux-x86_64-1.4.341.1.tar.xz
)

FetchContent_MakeAvailable(glfw vulkan glm)

add_library(vulkan SHARED IMPORTED)
set_target_properties(vulkan PROPERTIES
    IMPORTED_LOCATION "${vulkan_SOURCE_DIR}/x86_64/lib/libvulkan.so"
    INTERFACE_INCLUDE_DIRECTORIES "${vulkan_SOURCE_DIR}/x86_64/include"
)

include_directories(${CMAKE_SOURCE_DIR}/libs/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/lib)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable (YAVOG ${SOURCE_FILES})
target_link_libraries(YAVOG glfw)
target_link_libraries(YAVOG vulkan)
target_link_libraries(YAVOG glm)




# shaders
function (add_slang_shader_target TARGET)
  cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
  set (SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
  set (ENTRY_POINTS -entry vertMain -entry fragMain)
  add_custom_command (
          OUTPUT ${SHADERS_DIR}
          COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
  )
  add_custom_command (
          OUTPUT  ${BIN_OUTPUT_DIRECTORY}/slang.spv
          COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_3 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${BIN_OUTPUT_DIRECTORY}/slang.spv
          WORKING_DIRECTORY ${SHADERS_DIR}
          DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
          COMMENT "Compiling Slang Shaders"
          VERBATIM
  )
  add_custom_target (${TARGET} DEPENDS ${BIN_OUTPUT_DIRECTORY}/slang.spv)
endfunction()


file(GLOB_RECURSE SHADER_SLANG_SOURCES CONFIGURE_DEPENDS
       ${PROJECT_SOURCE_DIR}/shaders/**.slang)

set(SLANGC_EXECUTABLE "${vulkan_SOURCE_DIR}/x86_64/bin/slangc")
add_slang_shader_target( shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(YAVOG shaders)
