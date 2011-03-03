cmake_minimum_required (VERSION 2.8)
include (CheckFunctionExists)

set (HAVE_CMAKE true)

project (task)
set (PROJECT_VERSION "1.9.4")

SET (TASK_MAN1DIR share/man/man1 CACHE STRING "Installation directory for man pages, section 1")
SET (TASK_MAN5DIR share/man/man5 CACHE STRING "Installation directory for man pages, section 5")
SET (TASK_DOCDIR  share/doc/task CACHE STRING "Installation directory for doc files")
SET (TASK_BINDIR  bin            CACHE STRING "Installation directory for the binary")

message ("-- Looking for SHA1 references")
if (EXISTS .git/index)
  set (HAVE_COMMIT true)
  execute_process (COMMAND git log -1 --pretty=format:%h
                   OUTPUT_VARIABLE COMMIT)
  configure_file ( ${CMAKE_SOURCE_DIR}/commit.h.in
                   ${CMAKE_SOURCE_DIR}/commit.h)
  message ("-- Found SHA1 reference: ${COMMIT}")
endif (EXISTS .git/index)


set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "support@taskwarrior.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

message ("-- Looking for Lua51")
find_package (Lua51)
if (LUA51_FOUND)
  message ("-- Found Lua51: ${LUA_LIBRARIES}")
  set (HAVE_LIBLUA true)
  set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${LUA_INCLUDE_DIR})
  set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${LUA_LIBRARIES})
endif (LUA51_FOUND)

message ("-- Looking for pthread")
find_path (PTHREAD_INCLUDE_DIR pthread.h)
find_library (PTHREAD_LIBRARY NAMES pthread)
if (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)
  message ("-- Found pthread: ${PTHREAD_LIBRARY}")
  set (HAVE_LIBPTHREAD true)
  set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${PTHREAD_INCLUDE_DIR})
  set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${PTHREAD_LIBRARIES})
endif (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)

#message ("-- Looking for readline")
#find_path    (READLINE_INCLUDE_DIR   readline/readline.h)
#find_library (READLINE_LIBRARY NAMES readline)
#if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
#  message ("-- Found readline: ${READLINE_LIBRARY}")
#  set (HAVE_LIBREADLINE true)
#  set (HAVE_READLINE true)
#  set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${READLINE_INCLUDE_DIR})
#  set (TASK_LIBRARIES    ${TASK_LIBRARIES }   ${READLINE_LIBRARIES})
#endif (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)

check_function_exists (random  HAVE_RANDOM)
check_function_exists (srandom HAVE_SRANDOM)
check_function_exists (uuid_unparse_lower HAVE_UUID)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (LINUX true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (DARWIN true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	set (FREEBSD true)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (UNKNOWN true)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

message ("-- Configuring auto.h")
configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/auto.h)

add_subdirectory (src)
add_subdirectory (doc)
add_subdirectory (i18n)
add_subdirectory (scripts)
if (EXISTS test)
  add_subdirectory (test EXCLUDE_FROM_ALL)
endif (EXISTS test)

set (doc_FILES NEWS ChangeLog README INSTALL AUTHORS COPYING)
foreach (doc_FILE ${doc_FILES})
  install (FILES ${doc_FILE}  DESTINATION ${TASK_DOCDIR})
endforeach (doc_FILE)

# ---

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set (CPACK_SOURCE_IGNORE_FILES  "CMakeCache" "CMakeFiles" "CPackConfig" "CPackSourceConfig"
                                "_CPack_Packages" "cmake_install" "install_manifest"
                                "Makefile$" "test" "package-config" "misc/*"
                                "src/task$" "src/libtask.a" "auto.h$"
                                "/\\.gitignore" "/\\.git/" "swp$")
include (CPack)
