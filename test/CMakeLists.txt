cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test ${LUA_INCLUDE_DIR} ${PTHREAD_INCLUDE_DIR} ${READLINE_INCLUDE_DIR})

add_custom_target (test ./run_all DEPENDS date.t
                                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

add_executable (date.t date.t.cpp test.cpp)
target_link_libraries (date.t task ${LUA_LIBRARIES} ${READLINE_LIBRARY})
