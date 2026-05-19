cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(PixelInverter VERSION 1.0 LANGUAGES C)

# Estándar de C (C11 es ideal para desarrollo de sistemas)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Indicar dónde están los archivos de cabecera (.h)
include_directories(include)

# Buscar todos los archivos fuente en src/
file(GLOB SOURCES "src/*.c")

# Definir el ejecutable y dónde guardarlo
add_executable(pixel_inverter ${SOURCES})

# Opcional: Forzar que el binario se guarde en la carpeta build 
# (aunque CMake suele hacer esto por defecto en compilaciones 'out-of-source')
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})