cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS date.t t.t tdb.t duration.t t.benchmark.t text.t autocomplete.t
               seq.t record.t att.t subst.t nibbler.t filt.t cmd.t config.t
               util.t color.t list.t path.t file.t grid.t directory.t rx.t
               taskmod.t lisp.t rectangle.t tree.t tree2.t uri.t json.t)

add_custom_target (test ./run_all DEPENDS ${test_SRCS}
                                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} task ${TASK_LIBRARIES})
endforeach (src_FILE)
