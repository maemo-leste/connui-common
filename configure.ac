AC_INIT([connui], [2.88])
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
			gconf-2.0])

AC_SUBST(CONNUI_LIBS)
AC_SUBST(CONNUI_CFLAGS)

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

hildondesktoplibdir=`pkg-config libhildondesktop-1 --variable=hildondesktoplibdir`
AC_SUBST(hildondesktoplibdir)

AC_OUTPUT([
Makefile
connui/Makefile
connui.pc
conndlgs/Makefile
])

