cmake_minimum_required(VERSION 3.10)
project(FiltroGaussianoOS)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(filtro_blur 
    src/main.c 
    src/functions.c
)

# pthread; librt solo en Linux (para clock_gettime)
if (APPLE)
  target_link_libraries(filtro_blur pthread)
else ()
  target_link_libraries(filtro_blur pthread rt)
endif ()