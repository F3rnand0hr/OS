cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(l2 VERSION 1.0 LANGUAGES C)

# Estándar de C (C11 es ideal para desarrollo de sistemas)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Indicar dónde están los archivos de cabecera (.h)
include_directories(include)

# Buscar todos los archivos fuente en src/
file(GLOB SOURCES "src/*.c")

# Definir el ejecutable y dónde guardarlo
add_executable(l2 ${SOURCES})



set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})