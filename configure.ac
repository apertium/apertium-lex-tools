AC_PREREQ(2.61)

m4_define([required_libxml_version], [2.6.17])
m4_define([required_apertium_version], [3.2.0])
m4_define([required_lttoolbox_version], [3.2.0])

AC_INIT([apertium-lex-tools], [0.1.0], [ftyers@prompsit.com])
AM_INIT_AUTOMAKE([apertium-lex-tools], [0.1.0], no-define)
AC_CONFIG_HEADER([lrx_config.h])

AC_PROG_CXX
AM_SANITY_CHECK
AC_LANG_CPLUSPLUS


PKG_CHECK_MODULES([LTTOOLBOX], [lttoolbox-3.2 >= required_lttoolbox_version])

AC_SUBST(LTTOOLBOX_CFLAGS)
AC_SUBST(LTTOOLBOX_LIBS)

PKG_CHECK_MODULES([APERTIUM], [apertium-3.2 >= required_apertium_version])

AC_SUBST(APERTIUM_CFLAGS)
AC_SUBST(APERTIUM_LIBS)

PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= required_libxml_version])

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

# Checks for libraries.
AC_CHECK_LIB(xml2, xmlReaderForFile)

AC_CHECK_FUNCS([setlocale strdup getopt_long])

AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked, getopt_long, fgetwc_unlocked, fputwc_unlocked, fgetws_unlocked, fputws_unlocked])

CPPFLAGS="$CPPFLAGS $CFLAGS $APERTIUM_CFLAGS $LIBXML_CFLAGS"
LIBS="$LIBS $APERTIUM_LIBS $LIBXML_LIBS -lz"

AC_CONFIG_FILES([
                 Makefile
                 ])
AC_OUTPUT
