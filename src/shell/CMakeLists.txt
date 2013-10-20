cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/shell
                     ${TASK_INCLUDE_DIRS})

set (tasksh_SRCS Readline.cpp Readline.h)

add_library (tasksh STATIC ${tasksh_SRCS})
add_executable (tasksh_executable main.cpp)

# Yes, 'task' is included twice, other linking fails on CentOS.
target_link_libraries (tasksh_executable commands columns task tasksh ${TASK_LIBRARIES})

set_property (TARGET tasksh_executable PROPERTY OUTPUT_NAME "tasksh")

install (TARGETS tasksh_executable DESTINATION ${TASK_BINDIR})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
