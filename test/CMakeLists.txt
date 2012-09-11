cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS autocomplete.t color.t config.t date.t directory.t dom.t
               duration.t file.t i18n.t json.t list.t nibbler.t path.t rx.t
               t.t t2.t taskmod.t tdb2.t text.t uri.t util.t view.t json_test)

add_custom_target (test ./run_all --verbose
                                  DEPENDS ${test_SRCS} task_executable
                                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} task commands task columns ${TASK_LIBRARIES})
endforeach (src_FILE)

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
