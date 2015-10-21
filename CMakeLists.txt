cmake_minimum_required (VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

include (CheckFunctionExists)
include (CheckStructHasMember)

set (HAVE_CMAKE true)

project (task)
set (PROJECT_VERSION "2.5.0")

OPTION (ENABLE_SYNC "Enable 'task sync' support" ON)

if (ENABLE_SYNC)
  set (USE_GNUTLS ON CACHE BOOL "Build gnutls support." FORCE)
else (ENABLE_SYNC)
  set (USE_GNUTLS OFF CACHE BOOL "Build gnutls support." FORCE)
  message (WARNING "ENABLE_SYNC=OFF. Not building sync support.")
endif (ENABLE_SYNC)

message ("CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")

include (CheckCXXCompilerFlag)

# NOTE: If we are to actually use C++11 features, we should either require
#       a compiler that supports the -std=c++11 flag or check for the
#       features used.
#       Relying on -std=c++0x or even -std=gnu++0x is highly volatile.

CHECK_CXX_COMPILER_FLAG("-std=c++11" _HAS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" _HAS_CXX0X)
CHECK_CXX_COMPILER_FLAG("-std=gnu++0x" _HAS_GNU0X)

if (_HAS_CXX11)
  set (_CXX11_FLAGS "-std=c++11")
elseif (_HAS_CXX0X)
  message (WARNING "Enabling -std=c++0x draft compile flag. Your compiler does not support the standard '-std=c++11' option.  Consider upgrading.")
  set (_CXX11_FLAGS "-std=c++0x")
elseif (_HAS_GNU0X)
  message (WARNING "Enabling -std=gnu++0x draft compile flag. Your compiler does not support the standard '-std=c++11' option. Consider upgrading.")
  set (_CXX11_FLAGS "-std=gnu++0x")
else (_HAS_CXX11)
 message (FATAL_ERROR "C++11 support missing. Try upgrading your C++ compiler. If you have a good reason for using an outdated compiler, please let us know at support@taskwarrior.org.")
endif (_HAS_CXX11)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (LINUX true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (DARWIN true)
  set (_CXX11_FLAGS "${_CXX11_FLAGS} -stdlib=libc++")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD")
  set (KFREEBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set (FREEBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
  set (OPENBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
  set (NETBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  set (SOLARIS true)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "GNU")
  set (GNUHURD true)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "CYGWIN")
  set (CYGWIN true)
  # NOTE: Not setting -std=gnu++0x leads to compile errors even with
  #       GCC 4.8.3, and debugging those leads to insanity. Adding this
  #       workaround instead of fixing Cygwin.
  set (_CXX11_FLAGS "-std=gnu++0x")
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (UNKNOWN true)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set (CMAKE_CXX_FLAGS "${_CXX11_FLAGS} ${CMAKE_CXX_FLAGS}")

set (CMAKE_CXX_FLAGS "-Wall -Wsign-compare -Wreturn-type ${CMAKE_CXX_FLAGS}")

if (NETBSD)
   # Since readline, etc likely to be in /usr/pkg/lib, not standard library
   # Otherwise will remove links during install
   set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif (NETBSD)

if (FREEBSD)
SET (TASK_MAN1DIR man/man1 CACHE STRING "Installation directory for man pages, section 1")
SET (TASK_MAN5DIR man/man5 CACHE STRING "Installation directory for man pages, section 5")
else (FREEBSD)
SET (TASK_MAN1DIR share/man/man1 CACHE STRING "Installation directory for man pages, section 1")
SET (TASK_MAN5DIR share/man/man5 CACHE STRING "Installation directory for man pages, section 5")
endif (FREEBSD)
SET (TASK_DOCDIR  share/doc/task CACHE STRING "Installation directory for doc files")
SET (TASK_RCDIR "${TASK_DOCDIR}/rc" CACHE STRING "Installation directory for configuration files")
SET (TASK_BINDIR  bin            CACHE STRING "Installation directory for the binary")

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

if (USE_GNUTLS)
   message ("-- Looking for GnuTLS")
   find_package (GnuTLS)
   if (GNUTLS_FOUND)
     set (HAVE_LIBGNUTLS true)
     set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${GNUTLS_INCLUDE_DIR})
     set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${GNUTLS_LIBRARIES})
   endif (GNUTLS_FOUND)
endif (USE_GNUTLS)

if (ENABLE_SYNC AND NOT GNUTLS_FOUND)
  message (FATAL_ERROR "Cannot find GnuTLS. Use -DENABLE_SYNC=OFF to build Taskwarrior without sync support. See INSTALL for more information.")
endif (ENABLE_SYNC AND NOT GNUTLS_FOUND)

check_function_exists (timegm  HAVE_TIMEGM)
check_function_exists (get_current_dir_name HAVE_GET_CURRENT_DIR_NAME)
check_function_exists (wordexp HAVE_WORDEXP)

check_struct_has_member ("struct tm" tm_gmtoff time.h HAVE_TM_GMTOFF)
check_struct_has_member ("struct stat" st_birthtime "sys/types.h;sys/stat.h" HAVE_ST_BIRTHTIME)

message ("-- Looking for libuuid")
if (DARWIN OR FREEBSD)
  # Apple and FreeBSD include the uuid functions in their libc, rather than libuuid
  check_function_exists (uuid_unparse_lower HAVE_UUID_UNPARSE_LOWER)
else (DARWIN OR FREEBSD)
  find_path    (UUID_INCLUDE_DIR   uuid/uuid.h)
  find_library (UUID_LIBRARY NAMES uuid)
  if (UUID_INCLUDE_DIR AND UUID_LIBRARY)
    set (TASK_INCLUDE_DIRS ${TASK_INCLUDE_DIRS} ${UUID_INCLUDE_DIR})
    set (TASK_LIBRARIES    ${TASK_LIBRARIES}    ${UUID_LIBRARY})
    # Look for uuid_unparse_lower
    set (CMAKE_REQUIRED_INCLUDES  ${CMAKE_REQUIRED_INCLUDES}  ${UUID_INCLUDE_DIR})
    set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${UUID_LIBRARY})
    check_function_exists (uuid_unparse_lower HAVE_UUID_UNPARSE_LOWER)
  else (UUID_INCLUDE_DIR AND UUID_LIBRARY)
    message (FATAL_ERROR "-- libuuid not found.")
  endif (UUID_INCLUDE_DIR AND UUID_LIBRARY)
endif (DARWIN OR FREEBSD)

if (HAVE_UUID_UNPARSE_LOWER)
  message ("-- Found libuuid")
else (HAVE_UUID_UNPARSE_LOWER)
  message ("-- Found libuuid, using internal uuid_unparse_lower")
endif (HAVE_UUID_UNPARSE_LOWER)

# Set the package language.
if (LANGUAGE)
  set (PACKAGE_LANGUAGE ${LANGUAGE})
else (LANGUAGE)
  set (PACKAGE_LANGUAGE 1)
endif (LANGUAGE)

set (LANGUAGE_ENG_USA 1)
set (LANGUAGE_ESP_ESP 2)
set (LANGUAGE_DEU_DEU 3)
set (LANGUAGE_FRA_FRA 4)
set (LANGUAGE_ITA_ITA 5)
set (LANGUAGE_POR_PRT 6)
set (LANGUAGE_EPO_RUS 7)
set (LANGUAGE_POL_POL 8)
set (LANGUAGE_JPN_JPN 9)

message ("-- Configuring cmake.h")
configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/cmake.h)

add_subdirectory (src)
add_subdirectory (src/commands)
add_subdirectory (src/columns)
add_subdirectory (doc)
add_subdirectory (scripts)
if (EXISTS ${CMAKE_SOURCE_DIR}/test)
  add_subdirectory (test EXCLUDE_FROM_ALL)
endif (EXISTS ${CMAKE_SOURCE_DIR}/test)
if (EXISTS performance)
  add_subdirectory (performance EXCLUDE_FROM_ALL)
endif (EXISTS performance)

set (doc_FILES NEWS ChangeLog README.md INSTALL AUTHORS COPYING)
foreach (doc_FILE ${doc_FILES})
  install (FILES ${doc_FILE}  DESTINATION ${TASK_DOCDIR})
endforeach (doc_FILE)

# ---

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set (CPACK_SOURCE_IGNORE_FILES  "CMakeCache" "CMakeFiles" "CPackConfig" "CPackSourceConfig"
                                "_CPack_Packages" "cmake_install" "install_manifest" "Makefile$"
                                "test" "package-config" "misc/*" "src/task$" "src/calc$" "performance"
                                "src/libtask.a" "src/columns/libcolumns.a" "src/commands/libcommands.a"
                                "/\\\\.gitignore" "/\\\\.git/" "swp$" "src/lex$")
include (CPack)
