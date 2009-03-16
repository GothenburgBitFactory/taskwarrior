#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(task, 1.5.0, bugs@beckingham.net)

CFLAGS="${CFLAGS=}"
CXXFLAGS="${CXXFLAGS=}"
# this macro is used to get the arguments supplied
# to the configure script (./configure --enable-debug)
# Check if we have enable debug support.
AC_MSG_CHECKING(whether to enable debugging)
debug_default="yes"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
                       [default=$debug_default]],, enable_debug=$debug_default)
# Yes, shell scripts can be used
if test "$enable_debug" = "yes"; then
  CXXFLAGS="$CFLAGS -Wall -pedantic -ggdb3 -DDEBUG"
  AC_MSG_RESULT(yes)
else
  CXXFLAGS="$CFLAGS -O3"
  AC_MSG_RESULT(no)
fi

# Check for OS.
OS=`uname|sed -e 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'`
if test "$OS" = "sunos"; then
  AC_MSG_NOTICE([OS Solaris detected])
  AC_DEFINE([SOLARIS], [], [Compiling on Solaris])
else
  AC_MSG_NOTICE([OS Non-Solaris detected])
  AC_DEFINE([LINUX], [], [Compiling on Non-Solaris])
fi

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/task.cpp])
AC_CONFIG_HEADER([auto.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_SUBST(CFLAGS)

# Checks for libraries.
AC_CHECK_LIB(ncurses,initscr)
AC_CHECK_LIB(ncurses,endwin)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h sys/file.h sys/stat.h sys/time.h unistd.h])
AC_CHECK_HEADERS([sstream string vector map])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([select])
#AC_CHECK_FUNC(flock, [AC_DEFINE([HAVE_FLOCK], [1], [Found flock])])
AC_CHECK_FUNC(uuid_unparse_lower, [AC_DEFINE([HAVE_UUID], [1], [Found uuid_unparse_lower])])
AC_CHECK_FUNC(random, [AC_DEFINE([HAVE_RANDOM], [1], [Found random])])
AC_CHECK_FUNC(srandom, [AC_DEFINE([HAVE_SRANDOM], [1], [Found srandom])])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

