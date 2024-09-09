AC_INIT([Common connectivity UI library], patsubst(esyscmd([dpkg-parsechangelog -SVersion | sed -n '/^\([^-+]*\).*$/ {s//\1/;p}']), [
]), [], [connui-common])

AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_HEADER_STDC

AC_PATH_X
AC_PATH_XTRA
AC_SUBST(X_CFLAGS)

AM_GCONF_SOURCE_2
AC_PATH_PROG(GCONFTOOL, gconftool-2)

PKG_PROG_PKG_CONFIG

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_SUBST(CFLAGS)

PKG_CHECK_MODULES(CONNUI,
			[glib-2.0 dnl
			gtk+-2.0 dnl
			dbus-glib-1 dnl
			hildon-1 dnl
			icd2 dnl
			osso-ic dnl
			libosso dnl
			gconf-2.0])

AC_SUBST(CONNUI_LIBS)
AC_SUBST(CONNUI_CFLAGS)

PKG_CHECK_MODULES(GMODULE, gmodule-2.0)
AC_SUBST(GMODULE_CFLAGS)
AC_SUBST(GMODULE_LIBS)

PKG_CHECK_MODULES(GIO, gio-2.0)
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)

PKG_CHECK_MODULES(GIOUNIX, gio-unix-2.0)
AC_SUBST(GIOUNIX_CFLAGS)
AC_SUBST(GIOUNIX_LIBS)

AC_PATH_PROG([GDBUS_CODEGEN], [gdbus-codegen])

if test -z "$GDBUS_CODEGEN"; then
  AC_MSG_ERROR([gdbus-codegen is required but was not found]);
fi

AC_ARG_ENABLE([maemo-launcher],
              [AS_HELP_STRING([--enable-maemo-launcher],
                              [build with maemo-launcher support])],
                              [case "${enableval}" in
                               yes) maemo_launcher=true ;;
                               no)  maemo_launcher=false ;;
                               *) AC_MSG_ERROR([bad value ${enableval} for --enable-maemo-launcher]) ;;
                               esac], [maemo_launcher=false])

MAEMO_LAUNCHER_PACKAGE=""
if test "x$maemo_launcher" = "xtrue"; then
  MAEMO_LAUNCHER_PACKAGE="maemo-launcher-app"
  PKG_CHECK_MODULES(LAUNCHER_DEPS, $MAEMO_LAUNCHER_PACKAGE)
  LAUNCHER_DEPS_LIBS="-Xcompiler \"$LAUNCHER_DEPS_LIBS\""
else
  LAUNCHER_DEPS_CFLAGS=""
  LAUNCHER_DEPS_LIBS=""
fi

AC_SUBST(LAUNCHER_DEPS_CFLAGS)
AC_SUBST(LAUNCHER_DEPS_LIBS)
#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

conndlgslibdir=$libdir/conndlgs
AC_SUBST(conndlgslibdir)

hildondesktoplibdir=`pkg-config libhildondesktop-1 --variable=hildondesktoplibdir`
AC_SUBST(hildondesktoplibdir)

dnl Localization
GETTEXT_PACKAGE=osso-connectivity-ui
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "${GETTEXT_PACKAGE}", [gettext package])

AC_OUTPUT([
Makefile
connui/Makefile
connui.pc
conbtui/Makefile
conbtui.pc
conndlgs/Makefile
])

