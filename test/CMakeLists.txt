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
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${CMAKE_SOURCE_DIR}/src/taskchampion/lib
                     ${CMAKE_SOURCE_DIR}/test
                     ${CMAKE_SOURCE_DIR}/taskchampion/lib
                     ${TASK_INCLUDE_DIRS})

set (test_SRCS
  col.t
  dom.t
  eval.t
  lexer.t
  t.t
  tw-2689.t
  tdb2.t
  tc.t
  util.t
  variant_add.t
  variant_and.t
  variant_cast.t
  variant_divide.t
  variant_equal.t
  variant_exp.t
  variant_gt.t
  variant_gte.t
  variant_inequal.t
  variant_lt.t
  variant_lte.t
  variant_match.t
  variant_math.t
  variant_modulo.t
  variant_multiply.t
  variant_nomatch.t
  variant_not.t
  variant_or.t
  variant_partial.t
  variant_subtract.t
  variant_xor.t
  view.t
  )

add_custom_target (test ./run_all --verbose
                        DEPENDS ${test_SRCS} task_executable
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_custom_target (build_tests DEPENDS ${test_SRCS}
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

foreach (src_FILE ${test_SRCS})
  add_executable (${src_FILE} "${src_FILE}.cpp" test.cpp)
  target_link_libraries (${src_FILE} task tc commands columns libshared task tc commands columns libshared task commands columns libshared ${TASK_LIBRARIES})
  if (DARWIN)
    target_link_libraries (${src_FILE} "-framework CoreFoundation -framework Security -framework SystemConfiguration")
  endif (DARWIN)
endforeach (src_FILE)

configure_file(run_all run_all COPYONLY)
configure_file(problems problems COPYONLY)
configure_file(bash_tap.sh bash_tap.sh COPYONLY)
configure_file(bash_tap_tw.sh bash_tap_tw.sh COPYONLY)

add_subdirectory(basetest)
add_subdirectory(simpletap)
set (pythonTests
  abbreviation.t
  add.t
  alias.t
  annotate.t
  append.t
  args.t
  bash_completion.t
  blocked.t
  bulk.t
  burndown.t
  calc.t
  calendar.t
  caseless.t
  color.cmd.t
  color.rules.t
  columns.t
  commands.t
  completed.t
  configuration.t
  confirmation.t
  context.t
  count.t
  custom.config.t
  custom.recur_ind.t
  custom.t
  custom.tag_ind.t
  date.iso.t
  dateformat.t
  datesort.t
  datetime-negative.t
  debug.t
  default.t
  delete.t
  denotate.t
  dependencies.t
  diag.t
  diag_color.t
  dom2.t
  due.t
  duplicate.t
  edit.t
  encoding.t
  enpassant.t
  exec.t
  export.t
  feature.559.t
  feature.default.project.t
  feature.print.empty.columns.t
  feature.recurrence.t
  feedback.t
  filter.t
  fontunderline.t
  format.t
  gc.t
  helpers.t
  history.t
  hooks.env.t
  hooks.on-add.t
  hooks.on-launch.t
  hooks.on-modify.t
  hyphenate.t
  ids.t
  import.t
  info.t
  limit.t
  list.all.projects.t
  log.t
  logo.t
  math.t
  modify.t
  nag.t
  obfuscate.t
  oldest.t
  operators.t
  overdue.t
  partial.t
  prepend.t
  pri_sort.t
  project.t
  quotes.t
  rc.override.t
  recurrence.t
  reports.t
  search.t
  sequence.t
  shell.t
  show.t
  sorting.t
  special.t
  start.t
  stats.t
  substitute.t
  sugar.t
  summary.t
  tag.t
  taskrc.t
  template.t
  timesheet.t
  tw-1379.t
  tw-1837.t
  tw-20.t
  tw-2575.t
  tw-262.t
  tw-295.t
  uda.t
  uda_orphan.t
  uda_report.t
  uda_sort.t
  undo.t
  unicode.t
  unique.t
  upgrade.t
  urgency.t
  urgency_inherit.t
  uuid.t
  verbose.t
  version.t
  wait.t
  hooks.on-exit.t
  )

foreach (python_Test ${pythonTests})
  configure_file(${python_Test} "${python_Test}.py" COPYONLY)
endforeach(python_Test)

#SET(CMAKE_BUILD_TYPE gcov)
#SET(CMAKE_CXX_FLAGS_GCOV "--coverage")
#SET(CMAKE_C_FLAGS_GCOV "--coverage")
#SET(CMAKE_EXE_LINKER_FLAGS_GCOV "--coverage")
