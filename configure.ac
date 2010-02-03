#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(task, 1.9.0.beta1, support@taskwarrior.org)

CFLAGS="${CFLAGS=}"
CXXFLAGS="${CXXFLAGS=}"

# this macro is used to get the arguments supplied
# to the configure script (./configure --enable-debug)
# Check if we have enable debug support.
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
                       [default=$debug_default]],, enable_debug=$debug_default)
# Yes, shell scripts can be used
if test "$enable_debug" = "yes"; then
  CXXFLAGS="$CFLAGS -Wall -pedantic -ggdb3 -DDEBUG"
  AC_MSG_RESULT(yes)
else
  CXXFLAGS="$CFLAGS -Wall -pedantic -O3"
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

# Check for Lua.
AC_ARG_ENABLE([lua],   AS_HELP_STRING([--enable-lua], [Disable feature lua]))
AC_ARG_WITH(lua-inc, [--with-lua-inc=DIR, Lua include files are in DIR],lua_inc=$withval,lua_inc='')
AC_ARG_WITH(lua-lib, [--with-lua-lib=DIR, Lua library files are in DIR],lua_lib=$withval,lua_lib='')

if test "x$enable_lua" = "xyes" ; then
    ac_save_CPPFLAGS="$CPPFLAGS"
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LDFLAGS="$LDFLAGS"

    LUA_CFLAGS=""
    LUA_LFLAGS=""

    if test -n "$lua_inc"; then
       CFLAGS="$CFLAGS -I$lua_inc"
       CPPFLAGS="$CPPFLAGS -I$lua_inc"
    fi
    if test -n "$lua_lib"; then
       LDFLAGS="$LDFLAGS -L$lua_lib"
       LUA_LFLAGS="-llua"
    fi
fi

if test "x$enable_lua" = "xyes" ; then
    AC_SUBST(LUA_CFLAGS)
    AC_SUBST(LUA_LFLAGS)
    AC_DEFINE([HAVE_LIBLUA], [1], [Building with Lua support])
fi

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADER([auto.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_SUBST(CFLAGS)

# Checks for libraries.
AC_CHECK_LIB(ncurses,initscr)
AC_CHECK_LIB(lua,lua_open)

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

