#ifndef IAP_COMMON_H
#define IAP_COMMON_H

#include "iap-network.h"

dbus_bool_t iap_common_activate_iap(const gchar *iap);
void iap_common_get_last_used_network(network_entry *network);
gboolean iap_common_set_last_used_network(network_entry *entry);
GtkWidget *iap_common_show_saved_not_found_banner(GtkWidget *widget);
void iap_common_pack_to_hildon_button(GtkWidget *hbutton,GtkWidget *child,gboolean expand,gboolean fill);
gchar *iap_common_get_service_gconf_path(const gchar *service_type, const gchar *service_id);
void iap_common_set_close_response(GtkWidget *widget, gint response_id);
gboolean iap_common_get_preferred_service(gchar **preferred_type, gchar **preferred_id);
GtkWidget *iap_common_make_connection_entry(const gchar *iap);
void iap_common_get_service_properties(gchar *service_type, gchar *service_id, gchar *prop_name, ...);

#endif // IAP_COMMON_H
