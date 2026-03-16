cmake_minimum_required(VERSION 3.10)

project(ElevatorSystem C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Warnings básicos
add_compile_options(-Wall -Wextra)

# Include directory
include_directories(include)

# Ejecutable
add_executable(elevators
    src/main.c
    src/functions.c
)

# Enlazar pthread
find_package(Threads REQUIRED)
target_link_libraries(elevators Threads::Threads)