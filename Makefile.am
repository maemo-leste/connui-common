SUBDIRS = connui conbtui conndlgs

dbusdir = $(sysconfdir)/dbus-1/system.d/
dbus_DATA = data/icd-ui.conf

osso_af_initdir = $(sysconfdir)/osso-af-init
osso_af_init_DATA = data/icd-ui.conf

servicesdir = $(datadir)/dbus-1/services/
services_DATA = data/osso-connectivity-ui-conndlgs.service

system_servicesdir = $(datadir)/dbus-1/system-services/
system_services_DATA = data/com.nokia.osso_connectivity_ui_conndlgs.service

connuiinclude_HEADERS = include/connui.h \
			include/connui-box-view.h \
			include/connui-conndlgs.h \
			include/connui-cellular-data.h \
			include/connui-display.h \
			include/connui-dbus.h \
			include/connui-flightmode.h \
			include/connui-inetstate.h \
			include/connui-log.h \
			include/connui-pixbuf-cache.h \
			include/connui-pixbuf-anim.h \
			include/connui-scan-box-view.h \
			include/connui-utils.h \
			include/iap-common.h \
			include/iap-scan.h \
			include/iap-settings.h \
			include/iap-network.h \
			include/libicd-network-wlan-dev.h \
			include/wlancond.h \
			include/wlancond-dbus.h

connuiincludedir = $(includedir)/connui

conbtuiinclude_HEADERS = include/gateway-settings.h

conbtuiincludedir = $(includedir)/conbtui

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA	= connui.pc conbtui.pc
