lib_LTLIBRARIES = libconnui.la

libconnui_la_CFLAGS =	$(CONNUI_CFLAGS) -I$(srcdir)/../include -Wall -Werror \
			-DOSSOLOG_COMPILE=1 -Wno-format-extra-args
libconnui_la_LDFLAGS = -avoid-version -Wl,--no-undefined
libconnui_la_LIBADD = $(CONNUI_LIBS) -losso

libconnui_la_SOURCES =	\
			connui-dbus.c \
			iap-common.c \
			connui-cell-renderer-operator.c \
			connui-pixbuf-cache.c \
			connui-box-view.c \
			connui-scan-box-view.c \
			connui-utils.c

MAINTAINERCLEANFILES = Makefile.in
