cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS att.t autocomplete.t cmd.t color.t config.t date.t directory.t
               dom.t duration.t file.t filt.t json.t list.t nibbler.t path.t
               record.t rectangle.t rx.t seq.t subst.t t.benchmark.t t.t
               taskmod.t tdb.t tdb2.t text.t tree.t tree2.t uri.t util.t
               variant.t view.t
               json_test)

add_custom_target (test ./run_all DEPENDS ${test_SRCS}
                                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} task commands columns ${TASK_LIBRARIES})
endforeach (src_FILE)
