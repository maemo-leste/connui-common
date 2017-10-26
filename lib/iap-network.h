#ifndef IAPNETWORK_H
#define IAPNETWORK_H

#include <gtk/gtk.h>

struct _network_entry
{
	gchar *service_type;
	int service_attributes;
	gchar *service_id;
	gchar *network_type;
	int network_attributes;
	gchar *network_id;
};
typedef struct _network_entry network_entry;

struct scan_entry
{
	network_entry entry;
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

void iap_network_entry_clear(network_entry *network);

#endif // IAPNETWORK_H
