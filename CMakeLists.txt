# CMakeList.txt : CMake project for TestSetup, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OGL_intro")



# Add source to this project's executable.
add_executable (OGL_intro "src/main.cpp" "src/external/glad.c" "src/shader.h" "src/external/stb_image.cpp" "src/camera.h" "src/texture.h" "src/mesh.h" "src/model.h" "src/vao.h" "src/vbo.h" "src/ebo.h" "src/fbo.h" "src/rbo.h")
target_include_directories(OGL_intro PUBLIC "inc")
target_link_directories(OGL_intro PUBLIC "lib")
target_link_libraries(OGL_intro glfw3.lib opengl32.lib assimp-vc143-mt.lib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OGL_intro PROPERTY CXX_STANDARD 20)
endif()



# TODO: Add tests and install targets if needed.
