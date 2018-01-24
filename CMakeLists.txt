cmake_minimum_required(VERSION 3.9)
project(gc_3ds_input_redirection C)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)

include_directories(include ${SDL2_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.c")

set(LIBS ${SDL2_LIBRARIES})

add_executable(gc-3ds-input-redirection ${SOURCES})

target_link_libraries(gc-3ds-input-redirection ${LIBS})
