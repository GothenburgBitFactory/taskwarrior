cmake_minimum_required (VERSION 3.22...4.0)

configure_file(compare_runs.py compare_runs.py COPYONLY)
configure_file(load load)
configure_file(run_perf run_perf)

add_custom_target (performance ${CMAKE_BINARY_DIR}/performance/run_perf
                               DEPENDS task_executable
                               WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/performance)
