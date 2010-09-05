#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(task, 1.9.3, support@taskwarrior.org)


# Source type.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)


# Local copies for modification and later AC_SUBST.
CFLAGS="${CFLAGS=}"
CXXFLAGS="${CXXFLAGS=}"


# this macro is used to get the arguments supplied
# to the configure script (./configure --enable-debug)
# Check if we have enable debug support.
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
                       [default=$debug_default]],, enable_debug=$debug_default)

if test "x$enable_debug" = "xyes"; then
  CFLAGS="$CFLAGS -Wall -pedantic -ggdb3 -DDEBUG"
  CXXFLAGS="$CXXFLAGS -Wall -pedantic -ggdb3 -DDEBUG"
  AC_MSG_RESULT(yes)
else
  CFLAGS="$CFLAGS -Wall -pedantic -O3"
  CXXFLAGS="$CXXFLAGS -Wall -pedantic -O3"
  AC_MSG_RESULT(no)
fi


# Check for OS.
OS=`uname|sed -e 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'|cut -c 1-5`
if test "$OS" = "sunos"; then
  AC_MSG_NOTICE([OS Solaris detected])
  AC_DEFINE([SOLARIS], [], [Compiling on Solaris])
elif test "$OS" = "darwi"; then
  AC_MSG_NOTICE([OS Darwin detected])
  AC_DEFINE([DARWIN], [], [Compiling on Darwin])
elif test "$OS" = "cygwi"; then
  AC_MSG_NOTICE([OS Cygwin detected])
  AC_DEFINE([CYGWIN], [], [Compiling on Cygwin])
elif test "$OS" = "openb"; then
  AC_MSG_NOTICE([OS OpenBSD detected])
  AC_DEFINE([OPENBSD], [], [Compiling on OpenBSD])
elif test "$OS" = "haiku"; then
  AC_MSG_NOTICE([OS Haiku detected])
  AC_DEFINE([HAIKU], [], [Compiling on Haiku])
elif test "$OS" = "freeb"; then
  AC_MSG_NOTICE([OS FreeBSD detected])
  AC_DEFINE([FREEBSD], [], [Compiling on FreeBSD])
elif test "$OS" = "linux"; then
  AC_MSG_NOTICE([OS Linux detected])
  AC_DEFINE([LINUX], [], [Compiling on Linux])
else
  AC_MSG_NOTICE([OS not detected])
  AC_DEFINE([UNKNOWN], [], [Compiling on Unknown])
fi


# ncurses enabled by default.
AC_ARG_WITH([ncurses],
            [AS_HELP_STRING([--without-ncurses], [disable support for ncurses])],
            [with_ncurses=no],
            [with_ncurses=yes])

AC_ARG_WITH([ncurses-inc],
            [AS_HELP_STRING ([--with-ncurses-inc=DIR], [ncurses include files are in DIR])],
            [ncurses_inc=$withval],
            [ncurses_inc=''])

AC_ARG_WITH([ncurses-lib],
            [AS_HELP_STRING ([--with-ncurses-lib=DIR], [ncurses lib files are in DIR])],
            [ncurses_lib=$withval],
            [ncurses_lib=''])

if test "x$with_ncurses" = "xyes" ; then
  AC_DEFINE([HAVE_LIBNCURSES], [1], [Defined if you have libncurses])
  if test -n "$ncurses_inc"; then
    CFLAGS="$CFLAGS -I$ncurses_inc"
    CXXFLAGS="$CXXFLAGS -I$ncurses_inc"
  fi

  if test -n "$ncurses_lib"; then
    LDFLAGS="$LDFLAGS -L$ncurses_lib"
  fi

  AC_CHECK_LIB([ncurses],[main])
fi


# Readline enabled by default.
#AC_ARG_WITH([readline],
#            [AS_HELP_STRING([--without-readline], [disable support for readline])],
#            [with_readline=no],
#            [with_readline=yes])
#
#AC_ARG_WITH([readline-inc],
#            [AS_HELP_STRING ([--with-readline-inc=DIR], [readline include files are in DIR])],
#            [readline_inc=$withval],
#            [readline_inc=''])
#
#AC_ARG_WITH([readline-lib],
#            [AS_HELP_STRING ([--with-readline-lib=DIR], [readline lib files are in DIR])],
#            [readline_lib=$withval],
#            [readline_lib=''])
#
#if test "x$with_readline" = "xyes" ; then
#  AC_DEFINE([HAVE_LIBREADLINE], [1], [Defined if you have libreadline])
#  if test -n "$readline_inc"; then
#    CFLAGS="$CFLAGS -I$readline_inc"
#    CXXFLAGS="$CXXFLAGS -I$readline_inc"
#  fi
#
#  if test -n "$readline_lib"; then
#    LDFLAGS="$LDFLAGS -L$readline_lib"
#  fi
#
#  AC_CHECK_LIB([readline],[main])
#fi


# Lua disabled by default.
AC_ARG_WITH([lua],
            [AS_HELP_STRING([--with-lua], [enable support for lua])],
            [with_lua=yes],
            [with_lua=no])

AC_ARG_WITH([lua-inc],
            [AS_HELP_STRING ([--with-lua-inc=DIR], [lua include files are in DIR])],
            [lua_inc=$withval],
            [lua_inc=''])

AC_ARG_WITH([lua-lib],
            [AS_HELP_STRING ([--with-lua-lib=DIR], [lua lib files are in DIR])],
            [lua_lib=$withval],
            [lua_lib=''])

if test "x$with_lua" = "xyes" ; then
  AC_DEFINE([HAVE_LIBLUA], [1], [Defined if you have liblua])
  if test -n "$lua_inc"; then
    CFLAGS="$CFLAGS -I$lua_inc"
    CXXFLAGS="$CXXFLAGS -I$lua_inc"
  fi

  if test -n "$lua_lib"; then
    LDFLAGS="$LDFLAGS -L$lua_lib"
  fi

  AC_CHECK_LIB([lua],[main])
fi


# Allow the changes above to take effect.
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)


# Now the smaller details.
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADER([auto.h])


# Checks for libraries.
AC_CHECK_LIB(pthread,pthread_create)


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
AC_CHECK_FUNC(uuid_unparse_lower, [AC_DEFINE([HAVE_UUID], [1], [Found uuid_unparse_lower])])
AC_CHECK_FUNC(random, [AC_DEFINE([HAVE_RANDOM], [1], [Found random])])
AC_CHECK_FUNC(srandom, [AC_DEFINE([HAVE_SRANDOM], [1], [Found srandom])])


# Generate the Makefiles.
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

# End.

