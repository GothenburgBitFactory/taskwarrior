cmake_minimum_required (VERSION 3.22)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/tc/corrosion)

# Import taskchampion-lib as a CMake library.
corrosion_import_crate(
  MANIFEST_PATH "${CMAKE_SOURCE_DIR}/Cargo.toml"
  LOCKED
  CRATES "taskchampion-lib")

include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/tc
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${CMAKE_SOURCE_DIR}/taskchampion/lib
                     ${TASK_INCLUDE_DIRS})

set (tc_SRCS
  ffi.h
  util.cpp util.h
  Replica.cpp Replica.h
  Server.cpp Server.h
  WorkingSet.cpp WorkingSet.h
  Task.cpp Task.h)

add_library (tc STATIC ${tc_SRCS})
target_link_libraries(tc taskchampion-lib)
