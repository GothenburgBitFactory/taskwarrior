cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${TASK_INCLUDE_DIRS})

set (columns_SRCS Column.cpp Column.h)

add_library (columns STATIC ${columns_SRCS})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_C_FLAGS_DEBUG "-ggdb3")
set (CMAKE_C_FLAGS_RELEASE "-O3")
