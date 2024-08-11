cmake_minimum_required (VERSION 3.22)

OPTION(SYSTEM_CORROSION "Use system provided corrosion instead of vendored version" OFF)
if(SYSTEM_CORROSION)
  find_package(Corrosion REQUIRED)
else()
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/taskchampion-cpp/corrosion)
endif()

# Import taskchampion-lib as a CMake library. This implements the Rust side of
# the cxxbridge, and depends on the `taskchampion` crate.
corrosion_import_crate(
  MANIFEST_PATH "${CMAKE_SOURCE_DIR}/Cargo.toml"
  LOCKED
  CRATES "taskchampion-lib")

# Set up `taskchampion-cpp`, the C++ side of the bridge.
corrosion_add_cxxbridge(taskchampion-cpp
  CRATE taskchampion_lib
  FILES lib.rs
)
