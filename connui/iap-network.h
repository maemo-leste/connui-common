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

void iap_network_entry_clear(network_entry *network);
int iap_network_entry_service_compare(network_entry *network1, network_entry *network2);
gboolean iap_network_entry_to_dbus_iter(DBusMessageIter *iter,  network_entry *entry);
network_entry *iap_network_entry_dup(network_entry *entry);
gboolean iap_network_entry_from_dbus_iter(DBusMessageIter *iter, network_entry *entry);
gboolean iap_network_entry_disconnect(guint connection_flags, network_entry *entry);
gboolean iap_network_entry_is_saved(network_entry *entry);
int iap_network_entry_compare(network_entry *network1, network_entry *network2);
int iap_network_entry_network_compare(network_entry *network1, network_entry *network2);
gboolean iap_network_entry_equal(gconstpointer a, gconstpointer b);
guint iap_network_entry_hash(gconstpointer key);

#endif // IAPNETWORK_H
