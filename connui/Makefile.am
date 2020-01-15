lib_LTLIBRARIES = libconnui.la

libconnui_la_CFLAGS =	$(CONNUI_CFLAGS) -I$(srcdir)/../include -Wall -Werror \
			-DOSSOLOG_COMPILE=1 -Wno-format-extra-args
libconnui_la_LDFLAGS = -Wl,--no-undefined -Wl,--as-needed
libconnui_la_LIBADD = $(CONNUI_LIBS) -losso

libconnui_la_SOURCES =	\
			iap-scan.c \
			connui-display.c \
			connui-pixbuf-anim.c \
			connui-dbus.c \
			wlan-common.c \
			iap-settings.c \
			iap-common.c \
			iap-network.c \
			connui-cell-renderer-operator.c \
			connui-pixbuf-cache.c \
			connui-box-view.c \
			connui-scan-box-view.c \
			connui-utils.c \
			connui-flightmode.c \
			connui-devicelock.c \
			connui-log.c \
			connui-inetstate.c \
			connui-cellular-data.c

MAINTAINERCLEANFILES = Makefile.in
