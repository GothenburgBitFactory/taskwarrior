cmake_minimum_required (VERSION 3.22)

add_custom_target (performance ./run_perf
                               DEPENDS task_executable
                               WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/performance)

