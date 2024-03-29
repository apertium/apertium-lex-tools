AC_PREREQ(2.61)

m4_define([PKG_VERSION_MAJOR], [0])
m4_define([PKG_VERSION_MINOR], [5])
m4_define([PKG_VERSION_PATCH], [0])

# Bump if the ABI (not API) changed in a backwards-incompatible manner
m4_define([PKG_VERSION_ABI], [1])

m4_define([required_libxml_version], [2.6.17])
m4_define([required_lttoolbox_version], [3.7.2])

AC_INIT([apertium-lex-tools], [PKG_VERSION_MAJOR.PKG_VERSION_MINOR.PKG_VERSION_PATCH], [apertium-stuff@lists.sourceforge.net])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

VERSION=$PACKAGE_VERSION
VERSION_ABI=PKG_VERSION_ABI

AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(VERSION_ABI)

AC_PROG_CXX
AM_SANITY_CHECK
AC_PROG_LIBTOOL
AC_LANG_CPLUSPLUS

CFLAGS="-Wall -Wextra $CFLAGS"
CXXFLAGS="-Wall -Wextra $CXXFLAGS"

AC_ARG_ENABLE(debug,
              [  --enable-debug  Enable "-g -Wall" compiler options],
              [CXXFLAGS="-g -Wall";CFLAGS="-g -Wall"])

dnl IRSTLM support
AH_TEMPLATE(HAVE_IRSTLM, [Have IRSTLM])
AC_ARG_ENABLE(irstlm, AC_HELP_STRING([--enable-irstlm],
	[enable IRSTLM ranker (needed for monolingual rule-learning)]),
	[irstlm=${enableval}], [irstlm=no])
AM_CONDITIONAL([HAVE_IRSTLM], [test "x$irstlm" != "xno"])

AH_TEMPLATE(HAVE_YASMET, [Have YASMET])
AC_ARG_ENABLE(yasmet, AC_HELP_STRING([--enable-yasmet],
	[enable YASMET (needed for maximum-entropy training)]),
	[yasmet=${enableval}], [yasmet=no])
AM_CONDITIONAL([HAVE_YASMET], [test "x$yasmet" != "xno"])


PKG_CHECK_MODULES([LTTOOLBOX], [lttoolbox >= required_lttoolbox_version])

AC_SUBST(LTTOOLBOX_CFLAGS)
AC_SUBST(LTTOOLBOX_LIBS)

PKG_CHECK_MODULES([LIBXML], [libxml-2.0 >= required_libxml_version])

AC_SUBST(LIBXML_CFLAGS)
AC_SUBST(LIBXML_LIBS)

PKG_CHECK_MODULES([ICU], [icu-i18n, icu-io, icu-uc])

AC_CHECK_HEADER([utf8cpp/utf8.h], [CPPFLAGS="-I/usr/include/utf8cpp/ $CPPFLAGS"], [
  AC_CHECK_HEADER([utf8.h], [], [AC_MSG_ERROR([You don't have utfcpp installed.])])
])

AC_SUBST(ICU_CFLAGS)
AC_SUBST(ICU_LIBS)

# Checks for libraries.
AC_CHECK_LIB(xml2, xmlReaderForFile)

AC_CHECK_FUNCS([setlocale strdup])

AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked])

CPPFLAGS="$CPPFLAGS $CFLAGS $LTTOOLBOX_CFLAGS $LIBXML_CFLAGS $ICU_CFLAGS"
LIBS="$LIBS $LTTOOLBOX_LIBS $LIBXML_LIBS $ICU_LIBS"

# Checks for highest supported C++ standard
AC_LANG(C++)
for version in 23 2b 20 2a 17; do
  version_flag="-std=c++${version}"
  AX_CHECK_COMPILE_FLAG([${version_flag}], [break], [version_flag=none])
done
AS_IF([test "$version_flag" == none], [
  AC_MSG_ERROR([Could not enable at least C++17 - upgrade your compiler])
])
CXXFLAGS="$CXXFLAGS ${version_flag}"

AS_IF([test "x$irstlm" == "xno"],
      [AC_MSG_NOTICE([IRSTLM is not enabled; you will not be able run monolingual rule-learning; enable using --with-irstlm])])

AS_IF([test "x$yasmet" == "xno"],
      [AC_MSG_NOTICE([YASMET is not enabled; you will not be able run maximum-entropy training; enable using --with-yasmet])])

AC_ARG_ENABLE([python-bindings],
              AS_HELP_STRING([--enable-python-bindings],
                             [build python bindings (default=disabled)]),
              [enable_python_bindings=$enableval],
              [enable_python_bindings=no])
AM_CONDITIONAL([HAVE_PYTHON_BINDINGS], [test x$enable_python_bindings = xyes])

if test "x$enable_python_bindings" = "xyes"
then
	AM_PATH_PYTHON([3.4], [], [AC_MSG_WARN([Can't generate SWIG wrapper without Python])])
	AC_CONFIG_FILES([python/Makefile python/setup.py])

	AC_ARG_VAR([PYTHON_INSTALL_PARAMS], [Parameters to pass to the Python 3 module install step])
	if test "x$PYTHON_INSTALL_PARAMS" = "x"
	then
	  PYTHON_INSTALL_PARAMS="--prefix=\$(prefix) --root=\$(DESTDIR)/"
	fi
fi

AC_CONFIG_FILES([Makefile apertium-lex-tools.pc src/Makefile scripts/Makefile])
AC_OUTPUT
