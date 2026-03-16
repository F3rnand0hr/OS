cmake_minimum_required(VERSION 3.10)
project(keyword_counter C)

set(CMAKE_C_STANDARD 99)

add_executable(keyword_counter
    src/main.c
    src/functions.c
)

target_include_directories(keyword_counter PRIVATE include)