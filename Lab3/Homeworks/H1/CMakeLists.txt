cmake_minimum_required(VERSION 3.10)
project(L1_matrix_threads C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

find_package(Threads REQUIRED)

add_executable(matrix_threads
  src/main.c
  src/functions.c
)

target_compile_options(matrix_threads PRIVATE -Wall -Wextra)
target_link_libraries(matrix_threads PRIVATE Threads::Threads)
