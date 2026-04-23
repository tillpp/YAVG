cmake_minimum_required(VERSION 3.13)  
include(FetchContent)
project(YAVOG)
set(CMAKE_CXX_STANDARD 20)           

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      add_compile_options(-m64 -Wa,-mbig-obj)
endif()

if(MSVC)
  add_compile_options( -static -lpthread)
endif()

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
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4
)
FetchContent_Declare(
  fastnoiselite
  GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
  GIT_TAG 4a17083770411d2af8fd99b1f249ea10143dc959 # don't remove Tag. Small star library, better check code after each update for malicious code.
  SOURCE_SUBDIR Cpp
)
FetchContent_MakeAvailable(glfw glm stb fastnoiselite)

#vulkan
if(WIN32)
  message(${CMAKE_RC_COMPILER})
  FetchContent_Declare(
    vulkan_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG v1.3.280
  )
  FetchContent_MakeAvailable(vulkan_headers)
  FetchContent_Declare(
    vulkan_loader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_TAG v1.3.280
  )
  FetchContent_MakeAvailable(vulkan_loader)
  message("${vulkan_headers_SOURCE_DIR}/include")

  FetchContent_Declare(
    slang
    URL https://github.com/shader-slang/slang/releases/download/v2026.7/slang-2026.7-windows-x86_64.zip
  )
  FetchContent_MakeAvailable(slang)
  
else()
  FetchContent_Declare(
    vulkan
    URL https://sdk.lunarg.com/sdk/download/1.4.341.1/linux/vulkansdk-linux-x86_64-1.4.341.1.tar.xz
  )
  FetchContent_MakeAvailable(vulkan)
  add_library(vulkan SHARED IMPORTED)
  set_target_properties(vulkan PROPERTIES
      IMPORTED_LOCATION "${vulkan_SOURCE_DIR}/x86_64/lib/libvulkan.so"
      INTERFACE_INCLUDE_DIRECTORIES "${vulkan_SOURCE_DIR}/x86_64/include"
  )
endif(WIN32)


include_directories(${CMAKE_SOURCE_DIR}/libs/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/lib)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable (YAVOG ${SOURCE_FILES})
target_include_directories(YAVOG PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(YAVOG glfw)
target_link_libraries(YAVOG vulkan)
target_link_libraries(YAVOG glm)

if(MSVC)
  target_compile_options(YAVOG PRIVATE /wd2397 /wd4838)
endif()
target_include_directories(YAVOG PRIVATE
    ${stb_SOURCE_DIR}
)
target_include_directories(YAVOG PRIVATE
  ${fastnoiselite_SOURCE_DIR}/Cpp
)
if(WIN32)
  add_compile_definitions(_WIN32_WINNT=0x0600)
  target_link_libraries(YAVOG ws2_32)
  target_link_libraries(YAVOG wsock32)
  target_include_directories(YAVOG PRIVATE ${vulkan_headers_SOURCE_DIR}/include)
endif()

if(MINGW)
    target_link_options(YAVOG PRIVATE
        -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive
    )
endif()

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
          COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_3 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${BIN_OUTPUT_DIRECTORY}/slang.spv
          WORKING_DIRECTORY ${SHADERS_DIR}
          DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES} ${slang}
          COMMENT "Compiling Slang Shaders"
          VERBATIM
  )
  add_custom_target (${TARGET} DEPENDS ${BIN_OUTPUT_DIRECTORY}/slang.spv)
endfunction()


file(GLOB_RECURSE SHADER_SLANG_SOURCES CONFIGURE_DEPENDS
       ${PROJECT_SOURCE_DIR}/shaders/**.slang)

if(WIN32)
  set(SLANGC_EXECUTABLE "${slang_SOURCE_DIR}/bin/slangc.exe")
else()
  set(SLANGC_EXECUTABLE "${vulkan_SOURCE_DIR}/x86_64/bin/slangc")
endif(WIN32)

add_slang_shader_target( shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(YAVOG shaders)

if(WIN32)
    add_custom_command(TARGET YAVOG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:YAVOG> $<TARGET_RUNTIME_DLLS:YAVOG>
        COMMAND_EXPAND_LISTS
    )
endif()