cmake_minimum_required (VERSION 3.24...4.0)

include(FetchContent)

option(SYSTEM_CORROSION "Use system provided corrosion instead of vendored version" OFF)
option(TASK_USE_CORROSION_SUBMODULE "Whether to use corrosion from the submodule instead of downloading" OFF)
set(TASK_CORROSION_VERSION v0.5.2 CACHE STRING "Corrosion version to use in FetchContent")

if(TASK_USE_CORROSION_SUBMODULE)
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/corrosion/.git)
    # Try to initialize the submodule
    message(STATUS "Submodule update: corrosion")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive corrosion
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE submodule_res
    )
    if(NOT submodule_res EQUAL 0)
      message(WARNING "git submodule init failed, setting TASK_USE_CORROSION_SUBMODULE to OFF")
      set(TASK_USE_CORROSION_SUBMODULE OFF CACHE BOOL "(Failed to git submodule init)" FORCE)
    endif()
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/corrosion/CMakeLists.txt)
    # Maybe the sources were populated in a different way, use them still
    message(STATUS "Could not initialize submodule corrosion, but the files were already there")
  else()
    message(WARNING "cannot initialize git submodule, setting TASK_USE_CORROSION_SUBMODULE to OFF")
    set(TASK_USE_CORROSION_SUBMODULE OFF CACHE BOOL "(Cannot git submodule init)" FORCE)
  endif()
endif()

if(TASK_USE_CORROSION_SUBMODULE)
  set(FETCHCONTENT_SOURCE_DIR_CORROSION ${CMAKE_CURRENT_SOURCE_DIR}/corrosion)
endif()
if(SYSTEM_CORROSION)
  # Make sure find_package is always used without FetchContent fallback
  set(CMAKE_REQUIRE_FIND_PACKAGE_Corrosion ON)
endif()

FetchContent_Declare(Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG        ${TASK_CORROSION_VERSION}
    FIND_PACKAGE_ARGS CONFIG
)
FetchContent_MakeAvailable(Corrosion)

if(SYSTEM_CORROSION)
  find_package(Corrosion REQUIRED)
else()
  add_subdirectory(${CMAKE_SOURCE_DIR}/src/taskchampion-cpp/corrosion)
endif()

option (ENABLE_TLS_NATIVE_ROOTS "Use the system's TLS root certificates" OFF)

if (ENABLE_TLS_NATIVE_ROOTS)
  message ("Enabling native TLS roots")
  set(TASKCHAMPION_FEATURES "tls-native-roots")
endif (ENABLE_TLS_NATIVE_ROOTS)

# Import taskchampion-lib as a CMake library. This implements the Rust side of
# the cxxbridge, and depends on the `taskchampion` crate.
corrosion_import_crate(
  MANIFEST_PATH "${CMAKE_SOURCE_DIR}/Cargo.toml"
  LOCKED
  CRATES "taskchampion-lib"
  FEATURES "${TASKCHAMPION_FEATURES}")

# Set up `taskchampion-cpp`, the C++ side of the bridge.
corrosion_add_cxxbridge(taskchampion-cpp
  CRATE taskchampion_lib
  FILES lib.rs
)
