cmake_minimum_required (VERSION 2.8)

project (task)
set (PROJECT_VERSION "1.9.4")

add_subdirectory (src)
add_subdirectory (test EXCLUDE_FROM_ALL)
