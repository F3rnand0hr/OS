cmake_minimum_required(VERSION 3.10)
project(SchedulerPrioridades C)

set(CMAKE_C_STANDARD 11)


# Incluir la carpeta de cabeceras
include_directories(include)

# 1. Ejecutable del Scheduler (Receiver)
add_executable(scheduler 
    src/main.c 
    src/functions.c
)

# 2. Ejecutable del Emisor
add_executable(emisor 
    src/emisor.c
)

# Enlaces necesarios: 
# rt: Para Message Queues (POSIX RealTime)
# m: Para math.h (pow, tgamma)
target_link_libraries(scheduler rt m)
target_link_libraries(emisor rt)