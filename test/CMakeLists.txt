cmake_minimum_required (VERSION 3.22...4.0)

# -- C++ tests

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/commands
                     ${CMAKE_SOURCE_DIR}/src/columns
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${CMAKE_SOURCE_DIR}/test
                     ${TASK_INCLUDE_DIRS})

# All C++ test files. Note that the portion before `.cpp` must be a valid,
# unique C++ identifier.
set(test_SRCS
  col_test.cpp
  dom_test.cpp
  eval_test.cpp
  lexer_test.cpp
  t_test.cpp
  tw_2689_test.cpp
  tdb2_test.cpp
  tc_cpp_test.cpp
  util_test.cpp
  variant_add_test.cpp
  variant_and_test.cpp
  variant_cast_test.cpp
  variant_divide_test.cpp
  variant_equal_test.cpp
  variant_exp_test.cpp
  variant_gt_test.cpp
  variant_gte_test.cpp
  variant_inequal_test.cpp
  variant_lt_test.cpp
  variant_lte_test.cpp
  variant_match_test.cpp
  variant_math_test.cpp
  variant_modulo_test.cpp
  variant_multiply_test.cpp
  variant_nomatch_test.cpp
  variant_not_test.cpp
  variant_or_test.cpp
  variant_partial_test.cpp
  variant_subtract_test.cpp
  variant_xor_test.cpp
  view_test.cpp
  )

# Build `test_runner` containing all CPP tests, linked once.
create_test_sourcelist (cpp_test_SRCS cpp_tests.cpp ${test_SRCS})
add_executable(test_runner
  test.cpp
  ${cpp_test_SRCS}
)
target_link_libraries (test_runner task commands columns libshared task commands columns libshared task commands columns libshared ${TASK_LIBRARIES})
if (DARWIN)
  target_link_libraries (test_runner "-framework CoreFoundation -framework Security -framework SystemConfiguration")
endif (DARWIN)

foreach (test_FILE ${test_SRCS})
  get_filename_component (test_NAME ${test_FILE} NAME_WE)
  # Tell the source file what its own name is
  set_source_files_properties(${test_FILE} PROPERTIES COMPILE_FLAGS -DTEST_NAME=${test_NAME})
  add_test(NAME ${test_FILE}
    COMMAND test_runner ${test_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endforeach (test_FILE)

# -- Python tests

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
  import-v2.test.py
  info.test.py
  limit.test.py
  list.all.projects.test.py
  log.test.py
  logo.test.py
  math.test.py
  modify.test.py
  nag.test.py
  news.test.py
  obfuscate.test.py
  oldest.test.py
  operators.test.py
  overdue.test.py
  partial.test.py
  prepend.test.py
  pri_sort.test.py
  project.test.py
  purge.test.py
  quotes.test.py
  rc.override.test.py
  read-only.test.py
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
  tw-1999.test.py
  tw-20.test.py
  tw-2575.test.py
  tw-262.test.py
  tw-295.test.py
  tw-3527.test.py
  uda.test.py
  uda_orphan.test.py
  uda_report.test.py
  uda_sort.test.py
  undo.test.py
  unicode.test.py
  unique.test.py
  unusual_task.test.py
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

  add_test(NAME ${python_Test}
    COMMAND ${Python_EXECUTABLE} ${python_Test}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach(python_Test)

# Create a `make_tc_task` binary, used for unusual_task.test.py. In order to build this
# for the tests, it's added as a dependency of `test_runner`.
add_executable (make_tc_task make_tc_task.cpp)
target_link_libraries (make_tc_task task commands columns libshared task commands columns libshared task commands columns libshared ${TASK_LIBRARIES})
if (DARWIN)
  target_link_libraries (make_tc_task "-framework CoreFoundation -framework Security -framework SystemConfiguration")
endif (DARWIN)
add_dependencies(test_runner make_tc_task)

# -- Shell tests

set (shell_SRCS
  tw-1637.test.sh
  tw-1643.test.sh
  tw-1688.test.sh
  tw-1715.test.sh
  tw-1718.test.sh
  tw-1804.test.sh
  tw-1883.test.sh
  tw-1895.test.sh
  tw-1938.test.sh
  tw-2124.test.sh
  tw-2189.test.sh
  tw-2257.test.sh
  tw-2386.test.sh
  tw-2392.test.sh
  tw-2429.test.sh
  tw-2451.test.sh
  tw-2514.test.sh
  tw-2530.test.sh
  tw-2550.test.sh
  tw-2581.test.sh
  tw-3102.test.sh
  tw-3109.test.sh
)

configure_file(bash_tap.sh bash_tap.sh COPYONLY)
configure_file(bash_tap_tw.sh bash_tap_tw.sh COPYONLY)

foreach (shell_Test ${shell_SRCS})
  add_test(NAME ${shell_Test}
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/${shell_Test}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach(shell_Test)
