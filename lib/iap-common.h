#ifndef IAP_COMMON_H
#define IAP_COMMON_H

struct network_entry
{
	gchar *service_type;
	int service_attributes;
	gchar *service_id;
	gchar *network_type;
	int network_attributes;
	gchar *network_id;
};

struct scan_entry
{
	struct network_entry entry;
	guint timestamp;
	gchar *service_name;
	gint service_priority;
	gchar *network_name;
	gint network_priority;
	gint signal_strength;
	gchar *station_id;
	GtkTreeIter iterator;
	GSList *list;
};

dbus_bool_t iap_common_activate_iap(const gchar *iap);
void iap_network_entry_clear(struct network_entry *network);
GtkWidget *iap_common_show_saved_not_found_banner(GtkWidget *widget);
void iap_common_pack_to_hildon_button(GtkWidget *hbutton,GtkWidget *child,gboolean expand,gboolean fill);

#endif // IAP_COMMON_H
