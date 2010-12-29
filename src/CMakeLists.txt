cmake_minimum_required (VERSION 2.8) 

include_directories (${CMAKE_SOURCE_DIR}/src) 
find_package (Lua51)
include (${LUA_INCLUDE_DIR})

#AC_DEFINE([SOLARIS], [], [Compiling on Solaris])
#AC_DEFINE([DARWIN], [], [Compiling on Darwin])
#AC_DEFINE([CYGWIN], [], [Compiling on Cygwin])
#AC_DEFINE([OPENBSD], [], [Compiling on OpenBSD])
#AC_DEFINE([HAIKU], [], [Compiling on Haiku])
#AC_DEFINE([FREEBSD], [], [Compiling on FreeBSD])
#AC_DEFINE([LINUX], [], [Compiling on Linux])
#AC_DEFINE([UNKNOWN], [], [Compiling on Unknown])
#AC_DEFINE([HAVE_LIBLUA], [1], [Defined if you have liblua])
#AC_CONFIG_SRCDIR([src/main.cpp])
#AC_CONFIG_HEADER([auto.h])
#AC_CHECK_LIB(pthread,pthread_create)
#AC_HEADER_STDC
#AC_CHECK_HEADERS([stdlib.h sys/file.h sys/stat.h sys/time.h unistd.h])
#AC_CHECK_HEADERS([sstream string vector map])
#AC_HEADER_STDBOOL
#AC_C_CONST
#AC_TYPE_PID_T
#AC_TYPE_SIZE_T
#AC_HEADER_TIME
#AC_STRUCT_TM
#AC_FUNC_MKTIME
#AC_FUNC_SELECT_ARGTYPES
#AC_CHECK_FUNCS([select])
#AC_CHECK_FUNC(uuid_unparse_lower, [AC_DEFINE([HAVE_UUID], [1], [Found uuid_unparse_lower])])
#AC_CHECK_FUNC(random, [AC_DEFINE([HAVE_RANDOM], [1], [Found random])])
#AC_CHECK_FUNC(srandom, [AC_DEFINE([HAVE_SRANDOM], [1], [Found srandom])])

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
target_link_libraries (task ${LUA_LIBRARIES})
install (TARGETS task DESTINATION bin)

set (CMAKE_BUILD_TYPE debug)
set (CMAKE_C_FLAGS_DEBUG "-ggdb3")
set (CMAKE_C_FLAGS_RELEASE "-O3")

