#ifndef IAPNETWORK_H
#define IAPNETWORK_H

#include <gtk/gtk.h>
#include <dbus/dbus.h>

struct _network_entry
{
  gchar *service_type;
  dbus_uint32_t service_attributes;
  gchar *service_id;
  gchar *network_type;
  dbus_uint32_t network_attributes;
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
int iap_network_entry_service_compare(network_entry *network1, network_entry *network2);
gboolean iap_network_entry_to_dbus_iter(DBusMessageIter *iter,  network_entry *entry);
network_entry *iap_network_entry_dup(network_entry *entry);
gboolean iap_network_entry_from_dbus_iter(DBusMessageIter *iter, network_entry *entry);
gboolean iap_network_entry_disconnect(guint connection_flags, network_entry *entry);
gboolean iap_network_entry_is_saved(network_entry *entry);

#endif // IAPNETWORK_H