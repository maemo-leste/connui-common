#ifndef IAP_COMMON_H
#define IAP_COMMON_H

dbus_bool_t iap_common_activate_iap(const gchar *iap);
GtkWidget *iap_common_show_saved_not_found_banner(GtkWidget *widget);
void iap_common_pack_to_hildon_button(GtkWidget *hbutton,GtkWidget *child,gboolean expand,gboolean fill);
gchar *iap_common_get_service_gconf_path(const gchar *service_type, const gchar *service_id);

#endif // IAP_COMMON_H
