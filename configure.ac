AC_INIT([lasm], [0.1], [lukas@krickl.dev])

# check programs
AC_CANONICAL_TARGET

AC_LANG(C++)
AC_PROG_CXX

define(GET_LIBS,
[
    ./getlibs.sh
])

GET_LIBS([Updating external libraries])

AC_CONFIG_FILES([makefile])
AC_OUTPUT
