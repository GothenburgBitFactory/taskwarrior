cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

add_custom_target (test ./run_all DEPENDS date.t t.t tdb.t duration.t t.benchmark.t text.t
                                          autocomplete.t seq.t record.t att.t stringtable.t
                                          subst.t nibbler.t filt.t cmd.t config.t util.t
                                          color.t list.t path.t file.t grid.t directory.t
                                          rx.t taskmod.t lisp.t rectangle.t sensor.t tree.t
                                          tree2.t uri.t
                                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

add_executable (date.t date.t.cpp test.cpp)
target_link_libraries (date.t task ${TASK_LIBRARIES})

add_executable (t.t t.t.cpp test.cpp)
target_link_libraries (t.t task ${TASK_LIBRARIES})

add_executable (tdb.t tdb.t.cpp test.cpp)
target_link_libraries (tdb.t task ${TASK_LIBRARIES})

add_executable (duration.t duration.t.cpp test.cpp)
target_link_libraries (duration.t task ${TASK_LIBRARIES})

add_executable (t.benchmark.t t.benchmark.t.cpp test.cpp)
target_link_libraries (t.benchmark.t task ${TASK_LIBRARIES})

add_executable (text.t text.t.cpp test.cpp)
target_link_libraries (text.t task ${TASK_LIBRARIES})

add_executable (autocomplete.t autocomplete.t.cpp test.cpp)
target_link_libraries (autocomplete.t task ${TASK_LIBRARIES})

add_executable (seq.t seq.t.cpp test.cpp)
target_link_libraries (seq.t task ${TASK_LIBRARIES})

add_executable (record.t record.t.cpp test.cpp)
target_link_libraries (record.t task ${TASK_LIBRARIES})

add_executable (att.t att.t.cpp test.cpp)
target_link_libraries (att.t task ${TASK_LIBRARIES})

add_executable (stringtable.t stringtable.t.cpp test.cpp)
target_link_libraries (stringtable.t task ${TASK_LIBRARIES})

add_executable (subst.t subst.t.cpp test.cpp)
target_link_libraries (subst.t task ${TASK_LIBRARIES})

add_executable (nibbler.t nibbler.t.cpp test.cpp)
target_link_libraries (nibbler.t task ${TASK_LIBRARIES})

add_executable (filt.t filt.t.cpp test.cpp)
target_link_libraries (filt.t task ${TASK_LIBRARIES})

add_executable (cmd.t cmd.t.cpp test.cpp)
target_link_libraries (cmd.t task ${TASK_LIBRARIES})

add_executable (config.t config.t.cpp test.cpp)
target_link_libraries (config.t task ${TASK_LIBRARIES})

add_executable (util.t util.t.cpp test.cpp)
target_link_libraries (util.t task ${TASK_LIBRARIES})

add_executable (color.t color.t.cpp test.cpp)
target_link_libraries (color.t task ${TASK_LIBRARIES})

add_executable (list.t list.t.cpp test.cpp)
target_link_libraries (list.t task ${TASK_LIBRARIES})

add_executable (path.t path.t.cpp test.cpp)
target_link_libraries (path.t task ${TASK_LIBRARIES})

add_executable (file.t file.t.cpp test.cpp)
target_link_libraries (file.t task ${TASK_LIBRARIES})

add_executable (grid.t grid.t.cpp test.cpp)
target_link_libraries (grid.t task ${TASK_LIBRARIES})

add_executable (directory.t directory.t.cpp test.cpp)
target_link_libraries (directory.t task ${TASK_LIBRARIES})

add_executable (rx.t rx.t.cpp test.cpp)
target_link_libraries (rx.t task ${TASK_LIBRARIES})

add_executable (taskmod.t taskmod.t.cpp test.cpp)
target_link_libraries (taskmod.t task ${TASK_LIBRARIES})

add_executable (lisp.t lisp.t.cpp test.cpp)
target_link_libraries (lisp.t task ${TASK_LIBRARIES})

add_executable (rectangle.t rectangle.t.cpp test.cpp)
target_link_libraries (rectangle.t task ${TASK_LIBRARIES})

add_executable (sensor.t sensor.t.cpp test.cpp)
target_link_libraries (sensor.t task ${TASK_LIBRARIES})

add_executable (tree.t tree.t.cpp test.cpp)
target_link_libraries (tree.t task ${TASK_LIBRARIES})

add_executable (tree2.t tree2.t.cpp test.cpp)
target_link_libraries (tree2.t task ${TASK_LIBRARIES})

add_executable (uri.t uri.t.cpp test.cpp)
target_link_libraries (uri.t task ${TASK_LIBRARIES})
