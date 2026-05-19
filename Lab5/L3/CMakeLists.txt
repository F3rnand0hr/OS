cmake_minimum_required(VERSION 3.10)
project(TimerProject C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(l3 src/main.c)

# Importante: timer_create requiere la librería librt en sistemas Linux antiguos
target_link_libraries(l3 rt)