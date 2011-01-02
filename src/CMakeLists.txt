cmake_minimum_required (VERSION 2.8)
include (CheckFunctionExists)

set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "support@taskwarrior.org")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

include_directories (${CMAKE_SOURCE_DIR}/src) 

find_package (Lua51)
if (LUA51_FOUND)
  set (HAVE_LIBLUA true)
endif (LUA51_FOUND)

find_path (PTHREAD_INCLUDE_DIR pthread.h)
find_library (PTHREAD_LIBRARY NAMES pthread)
if (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)
  set (HAVE_LIBPTHREAD true)
endif (PTHREAD_INCLUDE_DIR AND PTHREAD_LIBRARY)

#find_path (READLINE_INCLUDE_DIR readline/readline.h)
#find_library (READLINE_LIBRARY NAMES readline)
#if (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
#  set (HAVE_LIBREADLINE true)
#  set (HAVE_READLINE true)
#endif (READLINE_INCLUDE_DIR AND READLINE_LIBRARY)

check_function_exists (random  HAVE_RANDOM)
check_function_exists (srandom HAVE_SRANDOM)
check_function_exists (uuid_unparse_lower HAVE_UUID)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (LINUX true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (DARWIN true)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (UNKNOWN true)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

configure_file (
  ${CMAKE_SOURCE_DIR}/cmake.h.in
  ${CMAKE_SOURCE_DIR}/auto.h)

include_directories (${CMAKE_SOURCE_DIR}/src ${LUA_INCLUDE_DIR} ${PTHREAD_INCLUDE_DIR}${READLINE_INCLUDE_DIR})

#AC_CHECK_HEADERS([stdlib.h sys/file.h sys/stat.h sys/time.h unistd.h])
#AC_CHECK_HEADERS([sstream string vector map])

set (task_SRCS API.cpp API.h Att.cpp Att.h Cmd.cpp Cmd.h Color.cpp Color.h
               Config.cpp Config.h Context.cpp Context.h Date.cpp Date.h
               Directory.cpp Directory.h Duration.cpp Duration.h File.cpp
               File.h Filter.cpp Filter.h Grid.cpp Grid.h Hooks.cpp Hooks.h
               Keymap.cpp Keymap.h Lisp.cpp Lisp.h Location.cpp Location.h
               Nibbler.cpp Nibbler.h Path.cpp Path.h Permission.cpp
               Permission.h Record.cpp Record.h Rectangle.cpp Rectangle.h
               Sensor.cpp Sensor.h Sequence.cpp Sequence.h StringTable.cpp
               StringTable.h Subst.cpp Subst.h TDB.cpp TDB.h Table.cpp Table.h
               Task.cpp Task.h Taskmod.cpp Taskmod.h Thread.cpp Thread.h
               Timer.cpp Timer.h Transport.cpp Transport.h TransportSSH.cpp
               TransportSSH.h TransportRSYNC.cpp TransportRSYNC.h
               TransportCurl.cpp TransportCurl.h Tree.cpp Tree.h burndown.cpp
               command.cpp custom.cpp dependency.cpp diag.cpp edit.cpp
               export.cpp history.cpp i18n.h import.cpp interactive.cpp
               main.cpp main.h recur.cpp report.cpp rules.cpp rx.cpp rx.h
               text.cpp text.h util.cpp util.h Uri.cpp Uri.h)

add_executable (task ${task_SRCS})
target_link_libraries (task ${LUA_LIBRARIES} ${READLINE_LIBRARY})

install (TARGETS task DESTINATION bin)

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_C_FLAGS_DEBUG "-ggdb3")
set (CMAKE_C_FLAGS_RELEASE "-O3")
