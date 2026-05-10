cmake_minimum_required(VERSION 3.10)
project(SchedulerBenchmark C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
add_definitions(-D_GNU_SOURCE)

find_library(RT_LIBRARY rt)

# Definimos los archivos fuente (que están en src)
add_executable(manager 
    src/main.c 
    src/functions.c
)

# --- CORRECCIÓN DE RUTA ---
# Le decimos al compilador que los .h están en la carpeta 'include'
target_include_directories(manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(RT_LIBRARY)
    target_link_libraries(manager ${RT_LIBRARY})
else()
    target_link_libraries(manager rt)
endif()