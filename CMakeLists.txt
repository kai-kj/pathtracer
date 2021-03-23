cmake_minimum_required(VERSION 3.10)

project(pathtracer C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)

file(GLOB_RECURSE SOURCES "src/*.c")
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
link_libraries(m OpenCL lua)

add_executable(pathtracer ${SOURCES})