cmake_minimum_required (VERSION 2.8)
set(cmake_allow_loose_loop_constructs true)

project (task)

set (package "task")
set (version "1.9.4")
set (package_bugreport "support@taskwarrior.org")
set (package_name "${package}")
set (package_tarname "${package}")
set (package_version "${version}")
set (package_string "${package} ${version}")

add_definitions(-DPACKAGE="${package}")
add_definitions(-DVERSION="${version}")
add_definitions(-DPACKAGE_BUGREPORT="${package_bugreport}")
add_definitions(-DPACKAGE_NAME="${package_name}")
add_definitions(-DPACKAGE_TARNAME="${ipackage_tarname}")
add_definitions(-DPACKAGE_VERSION="${package_version}")
add_definitions(-DPACKAGE_STRING="${package_string}")

add_subdirectory (src)
add_subdirectory (test EXCLUDE_FROM_ALL)
