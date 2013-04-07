cmake_minimum_required (VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

include (CheckFunctionExists)
include (CheckStructHasMember)

set (HAVE_CMAKE true)

project (task)
set (PROJECT_VERSION "2.3.0")

SET (TASK_MAN1DIR share/man/man1 CACHE STRING "Installation directory for man pages, section 1")
SET (TASK_MAN5DIR share/man/man5 CACHE STRING "Installation directory for man pages, section 5")
SET (TASK_DOCDIR  share/doc/task CACHE STRING "Installation directory for doc files")
SET (TASK_BINDIR  bin            CACHE STRING "Installation directory for the binary")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (LINUX true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (DARWIN true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set (FREEBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
  set (OPENBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
  set (NETBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  set (SOLARIS true)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (UNKNOWN true)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

message ("-- Looking for SHA1 references")
if (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)
  set (HAVE_COMMIT true)
  execute_process (COMMAND git log -1 --pretty=format:%h
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   OUTPUT_VARIABLE COMMIT)
  configure_file ( ${CMAKE_SOURCE_DIR}/commit.h.in
                   ${CMAKE_SOURCE_DIR}/commit.h)
  message ("-- Found SHA1 reference: ${COMMIT}")
endif (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)

set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "support@taskwarrior.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

message ("-- Looking for GnuTLS")
find_package (GnuTLS)
if (GNUTLS_FOUND)
  message ("-- Found GnuTLS: ${GNUTLS_LIBRARIES}")
  set (HAVE_LIBGNUTLS true)
  set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${GNUTLS_INCLUDE_DIR})
  set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${GNUTLS_LIBRARIES})
endif (GNUTLS_FOUND)

#message ("-- Looking for pthread")
#find_path (PTHREAD_INCLUDE_DIR pthread.h)
#find_library (PTHREAD_LIBRARY NAMES pthread)
#if (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)
#  message ("-- Found pthread: ${PTHREAD_LIBRARY}")
#  set (HAVE_LIBPTHREAD true)
#  set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${PTHREAD_INCLUDE_DIR})
#  set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${PTHREAD_LIBRARIES})
#endif (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)

# include the readline library finder module
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# find readline
message ("-- Looking for GNU Readline")
find_package (Readline)
if (READLINE_FOUND)
  message ("-- Found GNU Readline: ${READLINE_LIBRARIES}")
  set (HAVE_READLINE true)
  set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${READLINE_INCLUDE_DIR})
  set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${READLINE_LIBRARIES})
endif (READLINE_FOUND)

check_function_exists (random  HAVE_RANDOM)
check_function_exists (srandom HAVE_SRANDOM)
check_function_exists (timegm  HAVE_TIMEGM)

check_struct_has_member ("struct tm" tm_gmtoff time.h HAVE_TM_GMTOFF)
check_struct_has_member ("struct stat" st_birthtime "sys/types.h;sys/stat.h" HAVE_ST_BIRTHTIME)

message ("-- Looking for libuuid")
if (DARWIN)
  # Apple includes the uuid functions in their libc, rather than libuuid
  set (HAVE_UUID true)
  check_function_exists (uuid_unparse_lower HAVE_UUID_UNPARSE_LOWER)
else (DARWIN)
  find_path    (UUID_INCLUDE_DIR   uuid/uuid.h)
  find_library (UUID_LIBRARY NAMES uuid)
  if (UUID_INCLUDE_DIR AND UUID_LIBRARY)
    set (HAVE_UUID true)
    set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${UUID_INCLUDE_DIR})
    set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${UUID_LIBRARY})
    # Look for uuid_unparse_lower
    set (CMAKE_REQUIRED_INCLUDES  ${CMAKE_REQUIRED_INCLUDES}  ${UUID_INCLUDE_DIR})
    set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${UUID_LIBRARY})
    check_function_exists (uuid_unparse_lower HAVE_UUID_UNPARSE_LOWER)
  endif (UUID_INCLUDE_DIR AND UUID_LIBRARY)
endif (DARWIN)

if (HAVE_UUID AND HAVE_UUID_UNPARSE_LOWER)
  message ("-- Found libuuid")
elseif (HAVE_UUID AND NOT HAVE_UUID_UNPARSE_LOWER)
  message ("-- Found libuuid, using internal uuid_unparse_lower")
else (HAVE_UUID AND HAVE_UUID_UNPARSE_LOWER)
  message ("-- libuuid not found, using internal uuid")
endif (HAVE_UUID AND HAVE_UUID_UNPARSE_LOWER)

# Set the package language.
if (LANGUAGE)
  set (PACKAGE_LANGUAGE ${LANGUAGE})
else (LANGUAGE)
  set (PACKAGE_LANGUAGE 1)
endif (LANGUAGE)

set (LANGUAGE_EN_US 1)
set (LANGUAGE_ES_ES 2)
set (LANGUAGE_DE_DE 3)
set (LANGUAGE_FR_FR 4)

message ("-- Configuring cmake.h")
configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/cmake.h)

add_subdirectory (src)
add_subdirectory (src/commands)
add_subdirectory (src/columns)
add_subdirectory (src/shell)
add_subdirectory (doc)
add_subdirectory (i18n)
add_subdirectory (scripts)
if (EXISTS test)
  add_subdirectory (test EXCLUDE_FROM_ALL)
endif (EXISTS test)
if (EXISTS performance)
  add_subdirectory (performance EXCLUDE_FROM_ALL)
endif (EXISTS performance)

set (doc_FILES NEWS ChangeLog README INSTALL AUTHORS COPYING)
foreach (doc_FILE ${doc_FILES})
  install (FILES ${doc_FILE}  DESTINATION ${TASK_DOCDIR})
endforeach (doc_FILE)

# ---

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set (CPACK_SOURCE_IGNORE_FILES  "CMakeCache" "CMakeFiles" "CPackConfig" "CPackSourceConfig"
                                "_CPack_Packages" "cmake_install" "install_manifest" "Makefile$"
				"test" "package-config" "misc/*" "src/task$" "src/libtask.a"
				"src/columns/libcolumns.a" "src/commands/libcommands.a"
                                "auto.h$" "/\\.gitignore" "/\\.git/" "swp$")
include (CPack)
