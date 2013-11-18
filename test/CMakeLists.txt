cmake_minimum_required (VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS autocomplete.t color.t config.t date.t directory.t dom.t
               duration.t file.t i18n.t json.t list.t msg.t nibbler.t path.t
               rx.t t.t t2.t t3.t taskmod.t tdb2.t text.t uri.t utf8.t util.t
               view.t width.t json_test)

message ("-- Configuring run_all")
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

set (TESTBLOB "./*.t")
if (CYGWIN)
  set (TESTBLOB "./*.t ./*.t.exe")
endif (CYGWIN)

else (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

set (TESTBLOB "${CMAKE_SOURCE_DIR}/test/*.t ${CMAKE_BINARY_DIR}/test/*.t")
if (CYGWIN)
  set (TESTBLOB "${CMAKE_SOURCE_DIR}/test/*.t ${CMAKE_BINARY_DIR}/test/*.t.exe")
endif (CYGWIN)

endif (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

configure_file (run_all.in run_all)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS} task_executable
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

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
