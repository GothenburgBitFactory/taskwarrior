cmake_minimum_required (VERSION 3.22)

# This is a work-around for the following CMake issue:
# https://gitlab.kitware.com/cmake/cmake/issues/16062
# The issue has been fixed in CMake 3.11.0; the policy is set
# to OLD for compatibility with older versions of CMake only.
if(POLICY CMP0037 AND ${CMAKE_VERSION} VERSION_LESS "3.11.0")
  cmake_policy(SET CMP0037 OLD)
endif()

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/tc/lib
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS
  col.test.cpp
  dom.test.cpp
  eval.test.cpp
  lexer.test.cpp
  t.test.cpp
  tw-2689.test.cpp
  tdb2.test.cpp
  tc.test.cpp
  util.test.cpp
  variant_add.test.cpp
  variant_and.test.cpp
  variant_cast.test.cpp
  variant_divide.test.cpp
  variant_equal.test.cpp
  variant_exp.test.cpp
  variant_gt.test.cpp
  variant_gte.test.cpp
  variant_inequal.test.cpp
  variant_lt.test.cpp
  variant_lte.test.cpp
  variant_match.test.cpp
  variant_math.test.cpp
  variant_modulo.test.cpp
  variant_multiply.test.cpp
  variant_nomatch.test.cpp
  variant_not.test.cpp
  variant_or.test.cpp
  variant_partial.test.cpp
  variant_subtract.test.cpp
  variant_xor.test.cpp
  view.test.cpp
  )

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS} task_executable
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} ${src_FILE} test.cpp)
  target_link_libraries (${src_FILE} task tc commands columns libshared task tc commands columns libshared task commands columns libshared ${TASK_LIBRARIES})
  if (DARWIN)
    target_link_libraries (${src_FILE} "-framework CoreFoundation -framework Security -framework SystemConfiguration")
  endif (DARWIN)

  # Add a custom `foo.test` target.
  string(REGEX REPLACE "\\.[^.]*$" "" test_target ${src_FILE})
  add_custom_target (${test_target}
                     ./${src_FILE}
                     DEPENDS ${src_FILE}
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)
configure_file(bash_tap.sh bash_tap.sh COPYONLY)
configure_file(bash_tap_tw.sh bash_tap_tw.sh COPYONLY)

add_subdirectory(basetest)
add_subdirectory(simpletap)

set (pythonTests
  abbreviation.test.py
  add.test.py
  alias.test.py
  annotate.test.py
  append.test.py
  args.test.py
  bash_completion.test.py
  blocked.test.py
  bulk.test.py
  burndown.test.py
  calc.test.py
  calendar.test.py
  caseless.test.py
  color.cmd.test.py
  color.rules.test.py
  columns.test.py
  commands.test.py
  completed.test.py
  configuration.test.py
  confirmation.test.py
  context.test.py
  count.test.py
  custom.config.test.py
  custom.recur_ind.test.py
  custom.test.py
  custom.tag_ind.test.py
  date.iso.test.py
  dateformat.test.py
  datesort.test.py
  datetime-negative.test.py
  debug.test.py
  default.test.py
  delete.test.py
  denotate.test.py
  dependencies.test.py
  diag.test.py
  diag_color.test.py
  dom2.test.py
  due.test.py
  duplicate.test.py
  edit.test.py
  encoding.test.py
  enpassant.test.py
  exec.test.py
  export.test.py
  feature.559.test.py
  feature.default.project.test.py
  feature.print.empty.columns.test.py
  feature.recurrence.test.py
  feedback.test.py
  filter.test.py
  fontunderline.test.py
  format.test.py
  gc.test.py
  helpers.test.py
  history.test.py
  hooks.env.test.py
  hooks.on-add.test.py
  hooks.on-launch.test.py
  hooks.on-modify.test.py
  hyphenate.test.py
  ids.test.py
  import.test.py
  info.test.py
  limit.test.py
  list.all.projects.test.py
  log.test.py
  logo.test.py
  math.test.py
  modify.test.py
  nag.test.py
  obfuscate.test.py
  oldest.test.py
  operators.test.py
  overdue.test.py
  partial.test.py
  prepend.test.py
  pri_sort.test.py
  project.test.py
  quotes.test.py
  rc.override.test.py
  recurrence.test.py
  reports.test.py
  search.test.py
  sequence.test.py
  shell.test.py
  show.test.py
  sorting.test.py
  special.test.py
  start.test.py
  stats.test.py
  substitute.test.py
  sugar.test.py
  summary.test.py
  tag.test.py
  taskrc.test.py
  timesheet.test.py
  tw-1379.test.py
  tw-1837.test.py
  tw-20.test.py
  tw-2575.test.py
  tw-262.test.py
  tw-295.test.py
  uda.test.py
  uda_orphan.test.py
  uda_report.test.py
  uda_sort.test.py
  undo.test.py
  unicode.test.py
  unique.test.py
  upgrade.test.py
  urgency.test.py
  urgency_inherit.test.py
  uuid.test.py
  verbose.test.py
  version.test.py
  wait.test.py
  hooks.on-exit.test.py
  )

foreach (python_Test ${pythonTests})
  configure_file(${python_Test} ${python_Test} COPYONLY)

  # Add a custom `foo.test` target.
  string(REGEX REPLACE "\\.[^.]*$" "" test_target ${python_Test})
  add_custom_target (${test_target}
                     ./${python_Test}
                     DEPENDS ${python_Test}
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)
endforeach(python_Test)

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
