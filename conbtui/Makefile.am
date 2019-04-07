lib_LTLIBRARIES = libconbtui.la

libconbtui_la_CFLAGS =	$(CONNUI_CFLAGS) -I$(srcdir)/../include -Wall -Werror \
			-DOSSOLOG_COMPILE=1 -Wno-format-extra-args
libconbtui_la_LDFLAGS = -Wl,--no-undefined
libconbtui_la_LIBADD = $(CONNUI_LIBS) -losso

libconbtui_la_SOURCES =	\
			gateway/settings.c \
			gateway/pin.c \
			gateway/common.c

MAINTAINERCLEANFILES = Makefile.in
