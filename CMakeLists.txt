cmake_minimum_required(VERSION 3.10)

project(karg VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(karg STATIC
    karg/karg.c
    karg/karg_types.c
)

target_include_directories(karg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)