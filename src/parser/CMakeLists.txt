cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/parser
                     ${TASK_INCLUDE_DIRS})

set (parser_SRCS LRParser.cpp LRParser.h
                 Parser.cpp   Parser.h
                 Tree.cpp     Tree.h
                 A3.cpp       A3.h
                 A3t.cpp      A3t.h
                 Arg.cpp      Arg.h
                 E9.cpp       E9.h)

add_library (parser STATIC ${parser_SRCS})
add_executable (parser_executable bnf.cpp)
add_executable (args_executable   args.cpp)

target_link_libraries (parser_executable columns commands task parser columns commands task ${TASK_LIBRARIES})
target_link_libraries (args_executable   columns commands task parser columns commands task ${TASK_LIBRARIES})

set_property (TARGET parser_executable PROPERTY OUTPUT_NAME "parser")
set_property (TARGET args_executable   PROPERTY OUTPUT_NAME "args")

#install (TARGETS parser_executable DESTINATION ${TASK_BINDIR})

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g -Wall")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")
