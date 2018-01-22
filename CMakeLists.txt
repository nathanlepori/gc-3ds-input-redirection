cmake_minimum_required(VERSION 3.9)
project(gc_3ds_input_redirection C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_work")

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(Config REQUIRED)

include_directories(include ${SDL2_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.c")

set(LIBS ${SDL2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CONFIG_LIBRARY})

add_executable(gc_3ds_input_redirection ${SOURCES})

target_link_libraries(gc_3ds_input_redirection ${LIBS})
