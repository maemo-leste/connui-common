lib_LTLIBRARIES = libconnui.la

libconnui_la_CFLAGS = $(CONNUI_CFLAGS) -I$(srcdir)/../include -Wall -Werror
libconnui_la_LDFLAGS = -avoid-version -Wl,--no-undefined
libconnui_la_LIBADD = $(CONNUI_LIBS)

libconnui_la_SOURCES =	\
			connui_pixbuf_cache.c \
			connui_box_view.c \
			connui_scan_box_view.c

MAINTAINERCLEANFILES = Makefile.in
