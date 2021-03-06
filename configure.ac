#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([Ginger],[0.9.2],[https://github.com/Spicery/ginger],[ginger])
AC_CONFIG_SRCDIR([appginger.png])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strchr strstr strtoul])

# Add feature for snap sensitivity.
AC_ARG_ENABLE([snap],
  AS_HELP_STRING([--enable-snap], [Enable this feature for snapcraft; makes the executable paths respect $SNAP]),
  AS_IF([test "x$enableval" = xyes], [usesnap=1], [usesnap=0]),
  [usesnap=0]
)
AC_SUBST( usesnap, $usesnap )

# Allow Boost to be somewhere that requires compile and/or link options.
BOOST_CPPFLAGS=""
BOOST_LDFLAGS=""
AC_ARG_WITH([boost],
 AS_HELP_STRING([--with-boost@<:@=DIR@:>@],
 [root directory of Boost installation]),
 [
 boost_root="${withval}"
 if test "${boost_root}" != yes; then
 boost_include="${boost_root}/include"
 boost_libdir="${boost_root}/lib"
 echo "BOOST FOUND"
 fi
 ])

AC_ARG_WITH([boost-include],
 AS_HELP_STRING([--with-boost-include=DIR],
 [specify exact directory for Boost headers]),
 [boost_include="$withval"])

AC_ARG_WITH([boost-libdir],
 AS_HELP_STRING([--with-boost-libdir=DIR],
 [specify exact directory for Boost libraries]),
 [boost_libdir="$withval"])

if test "${boost_include}"; then
 BOOST_CPPFLAGS="-I${boost_include}"
 echo "BOOST_CPPFLAGS set"
fi

if test "${boost_libdir}"; then
 BOOST_LDFLAGS="-L${boost_libdir}"
 echo "BOOST_LDDFLAGS set"
fi

AC_CONFIG_FILES([Makefile
                 apps/Makefile
                 apps/appginger/Makefile
                 apps/appginger/cpp/Makefile
                 apps/appginger/unittest/Makefile
                 apps/automatic/simple/Makefile
                 apps/automatic/sys/Makefile
                 apps/automatic/sysfn/Makefile
                 apps/automatic/machine/Makefile
                 apps/automatic/metainfo/Makefile
                 apps/common2gnx/Makefile
                 apps/common2gnx/cpp/Makefile
                 apps/lisp2gnx/Makefile
                 apps/lisp2gnx/lsp/Makefile
                 apps/fetchgnx/Makefile
                 apps/fetchgnx/cpp/Makefile
                 apps/fetchgnx/unittest/Makefile
                 apps/file2gnx/Makefile
                 apps/file2gnx/cpp/Makefile
                 apps/file2gnx/unittest/Makefile
                 apps/ginger/Makefile
                 apps/ginger/cpp/Makefile
                 apps/ginger-admin/Makefile
                 apps/ginger-admin/cpp/Makefile
                 apps/ginger-help/Makefile
                 apps/ginger-help/cpp/Makefile
                 apps/gprojlibtool/Makefile
                 apps/gprojlibtool/cpp/Makefile
                 apps/gvmtest/Makefile
                 apps/gvmtest/cpp/Makefile
                 apps/ivylibtool/Makefile
                 apps/ivylibtool/cpp/Makefile
                 apps/libgng/Makefile
                 apps/libgng/cpp/Makefile
                 apps/libgng/unittest/Makefile
                 apps/libmetainfo/Makefile
                 apps/libmetainfo/cpp/Makefile
                 apps/libmetainfo/unittest/Makefile
                 apps/shebang/Makefile
                 apps/shebang/cpp/Makefile
                 apps/simplifygnx/Makefile
                 apps/simplifygnx/cpp/Makefile
                 apps/simplifygnx/unittest/Makefile
                 apps/tidymnx/Makefile
                 apps/tidymnx/cpp/Makefile
                 apps/gson2gnx/Makefile
                 apps/gson2gnx/cpp/Makefile
                 apps/lnx2mnx/Makefile
                 apps/lnx2mnx/cpp/Makefile
                 apps/src2lnx/Makefile
                 apps/src2lnx/cpp/Makefile
                 apps/lnx2csv/Makefile
                 apps/lnx2csv/cpp/Makefile
                 autodocs/Makefile
                 projects/Makefile])

AC_OUTPUT
