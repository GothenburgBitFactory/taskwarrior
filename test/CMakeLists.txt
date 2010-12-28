include_directories (${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test) 
add_custom_target (test ./run_all DEPENDS date.t
                                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)
add_executable (date.t date.t.cpp test.cpp)
