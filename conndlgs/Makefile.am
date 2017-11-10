bin_PROGRAMS = osso-connectivity-ui-conndlgs

osso_connectivity_ui_conndlgs_CFLAGS = \
			$(CONNUI_CFLAGS) -I$(srcdir)/../include -Wall -Werror \
			-DOSSOLOG_COMPILE=1 -Wno-format-extra-args
osso_connectivity_ui_conndlgs_LDADD = $(CONNUI_LIBS) -losso $(top_builddir)/connui/libconnui.la
osso_connectivity_ui_conndlgs_LDFLAGS = -Wl,--export-dynamic

osso_connectivity_ui_conndlgs_SOURCES = \
			osso-connectivity-ui-conndlgs.c

MAINTAINERCLEANFILES = Makefile.in
