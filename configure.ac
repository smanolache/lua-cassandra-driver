AC_INIT([lua-cassandra-driver], [0.0.0], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-portability -Wno-unsupported])

AM_PROG_AR

AC_CONFIG_MACRO_DIR([m4])

AC_ENABLE_STATIC(no)
AC_PROG_LIBTOOL

AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug], [enable debug @<:@default=no@:>@])], [ if test "${enable_debug:-no}" != no; then CFLAGS="${CFLAGS} -Wall -g -O0 -fno-inline"; else CFLAGS="${CFLAGS} -Wall -DNDEBUG=1 -O2"; fi ], [ CFLAGS="${CFLAGS} -Wall -DNDEBUG=1 -O2" ])


AC_PREFIX_DEFAULT([/usr])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/unit/Makefile
])

AC_OUTPUT
