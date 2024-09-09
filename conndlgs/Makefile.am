bin_PROGRAMS = osso-connectivity-ui-conndlgs

osso_connectivity_ui_conndlgs_CFLAGS = \
			$(GIO_CFLAGS)$(LAUNCHER_DEPS_CFLAGS) \
			$(GMODULE_CFLAGS) $(GIOUNIX_CFLAGS) $(CONNUI_CFLAGS) \
			-I$(srcdir)/../include -Wall -Werror \
			-DOSSOLOG_COMPILE=1 -Wno-format-extra-args \
			-DCONNDLGS_PLUGIN_DIR=\"$(conndlgslibdir)\"

osso_connectivity_ui_conndlgs_LDADD = \
			$(top_builddir)/connui/libconnui.la -losso \
			$(CONNUI_LIBS) $(GMODULE_LIBS) $(GIO_LIBS)

osso_connectivity_ui_conndlgs_LDFLAGS = \
			$(LAUNCHER_DEPS_LIBS) -Wl,--as-needed

osso_connectivity_ui_conndlgs_SOURCES = \
			osso-connectivity-ui-conndlgs.c

MAINTAINERCLEANFILES = Makefile.in
