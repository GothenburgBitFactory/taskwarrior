cmake_minimum_required (VERSION 3.0)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (CheckFunctionExists)
include (CheckStructHasMember)

set (HAVE_CMAKE true)

project (task)
include (CXXSniffer)

set (PROJECT_VERSION "2.6.2")

OPTION (ENABLE_WASM "Enable 'wasm' support" OFF)

if (ENABLE_WASM)
  message ("Enabling WASM support.")
  set(CMAKE_EXECUTABLE_SUFFIX ".js")
endif (ENABLE_WASM)

OPTION (ENABLE_SYNC "Enable 'task sync' support" ON)

if (ENABLE_SYNC)
  set (USE_GNUTLS ON CACHE BOOL "Build gnutls support." FORCE)
else (ENABLE_SYNC)
  set (USE_GNUTLS OFF CACHE BOOL "Build gnutls support." FORCE)
  message (WARNING "ENABLE_SYNC=OFF. Not building sync support.")
endif (ENABLE_SYNC)

message ("-- Looking for libshared")
if (EXISTS ${CMAKE_SOURCE_DIR}/src/libshared/src)
  message ("-- Found libshared")
else (EXISTS ${CMAKE_SOURCE_DIR}/src/libshared/src)
  message ("-- Cloning libshared")
  execute_process (COMMAND git submodule update --init
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif (EXISTS ${CMAKE_SOURCE_DIR}/src/libshared/src)

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
set (PACKAGE_BUGREPORT "support@gothenburgbitfactory.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

if (FREEBSD OR DRAGONFLY)
SET (TASK_MAN1DIR man/man1 CACHE STRING "Installation directory for man pages, section 1")
SET (TASK_MAN5DIR man/man5 CACHE STRING "Installation directory for man pages, section 5")
else (FREEBSD OR DRAGONFLY)
SET (TASK_MAN1DIR share/man/man1 CACHE STRING "Installation directory for man pages, section 1")
SET (TASK_MAN5DIR share/man/man5 CACHE STRING "Installation directory for man pages, section 5")
endif (FREEBSD OR DRAGONFLY)
SET (TASK_DOCDIR  share/doc/task CACHE STRING "Installation directory for doc files")
SET (TASK_RCDIR "${TASK_DOCDIR}/rc" CACHE STRING "Installation directory for configuration files")
SET (TASK_BINDIR  bin            CACHE STRING "Installation directory for the binary")

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
if (DARWIN OR FREEBSD OR OPENBSD)
  # Apple and FreeBSD include the uuid functions in their libc, rather than libuuid
  check_function_exists (uuid_unparse_lower HAVE_UUID_UNPARSE_LOWER)
else (DARWIN OR FREEBSD OR OPENBSD)
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
endif (DARWIN OR FREEBSD OR OPENBSD)

if (HAVE_UUID_UNPARSE_LOWER)
  message ("-- Found libuuid")
else (HAVE_UUID_UNPARSE_LOWER)
  message ("-- Found libuuid, using internal uuid_unparse_lower")
endif (HAVE_UUID_UNPARSE_LOWER)

if (HAIKU)
  # search for socket() in libnetwork on Haiku
  message("-- Looking for libnetwork")
  find_library (NETWORK_LIBRARY NAMES network)
  if (NETWORK_LIBRARY)
    set (TASK_LIBRARIES    ${TASK_LIBRARIES} ${NETWORK_LIBRARY})
  else (NETWORK_LIBRARY)
    message(FATAL_ERROR "-- libnetwork not found.")
  endif (NETWORK_LIBRARY)
endif (HAIKU)

if (SOLARIS)
  # accept() is in libsocket according to its manpage
  message("-- Looking for libsocket")
  find_library (SOCKET_LIBRARY NAMES socket)
  if (SOCKET_LIBRARY)
    set (TASK_LIBRARIES    ${TASK_LIBRARIES} ${SOCKET_LIBRARY})
  else (SOCKET_LIBRARY)
    message(FATAL_ERROR "-- libsocket not found.")
  endif (SOCKET_LIBRARY)

  # inet_ntop() is in libnsl according to its manpage
  message("-- Looking for libnsl")
  find_library (NSL_LIBRARY NAMES nsl)
  if (NSL_LIBRARY)
    set (TASK_LIBRARIES    ${TASK_LIBRARIES} ${NSL_LIBRARY})
  else (NSL_LIBRARY)
    message(FATAL_ERROR "-- libnsl not found.")
  endif (NSL_LIBRARY)
endif (SOLARIS)

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

set (doc_FILES NEWS ChangeLog README.md INSTALL AUTHORS COPYING LICENSE)
foreach (doc_FILE ${doc_FILES})
  install (FILES ${doc_FILE}  DESTINATION ${TASK_DOCDIR})
endforeach (doc_FILE)

add_custom_command(OUTPUT run-review
    COMMAND docker build -q --build-arg PR=$(PR) --build-arg LIBPR=$(LIBPR) -t taskwarrior-review:$(PR)s$(LIBPR) - < scripts/review-dockerfile
    COMMAND docker run --rm --memory 1g --hostname pr-$(PR)s$(LIBPR) -it taskwarrior-review:$(PR)s$(LIBPR) bash || :
)
add_custom_target(review DEPENDS run-review)

add_custom_command(OUTPUT run-reproduce
    COMMAND docker build -q --build-arg RELEASE=$(RELEASE) -t taskwarrior-reproduce:$(RELEASE) - < scripts/reproduce-dockerfile
    COMMAND docker run --rm --memory 1g --hostname tw-$(RELEASE) -it taskwarrior-reproduce:$(RELEASE) bash || :
)
add_custom_target(reproduce DEPENDS run-reproduce)

# ---

set (CPACK_SOURCE_GENERATOR "TGZ")
set (CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set (CPACK_SOURCE_IGNORE_FILES  "CMakeCache" "CMakeFiles" "CPackConfig" "CPackSourceConfig"
                                "_CPack_Packages" "cmake_install" "install_manifest" "Makefile$"
                                "test" "package-config" "misc/*" "src/task$" "src/calc$" "performance"
                                "src/libtask.a" "src/columns/libcolumns.a" "src/commands/libcommands.a"
                                "swp$" "src/lex$" "task-.*.tar.gz" "commit.h" "cmake.h$" "\\\\.gitmodules"
                                "src/libshared/\\\\.git" ".github/" ".*\\\\.gitignore$"
                                "src/liblibshared.a" "docker-compose.yml" "\\\\.git/")
include (CPack)
