cmake_minimum_required(VERSION 3.10)
project(MonitorMatricial)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(l4 src/main.c)

# Asegurar que el binario se guarde en build/
set_target_properties(l4 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")