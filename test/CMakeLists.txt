cmake_minimum_required (VERSION 2.8)
if(POLICY CMP0037)
  cmake_policy(SET CMP0037 OLD)
endif()
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS autocomplete.t col.t color.t config.t fs.t i18n.t json.t list.t
               msg.t nibbler.t rx.t t.t tdb2.t text.t utf8.t util.t view.t
               json_test lexer.t iso8601d.t iso8601p.t eval.t dates.t
               variant_add.t variant_and.t variant_cast.t variant_divide.t
               variant_equal.t variant_exp.t variant_gt.t variant_gte.t
               variant_inequal.t variant_lt.t variant_lte.t variant_match.t
               variant_math.t variant_modulo.t variant_multiply.t
               variant_nomatch.t variant_not.t variant_or.t variant_partial.t
               variant_subtract.t variant_xor.t)

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS} task_executable
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} task commands task columns ${TASK_LIBRARIES})
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
