#include <dbus/dbus.h>
#include <gconf/gconf-client.h>

#include <string.h>

#include "iap-network.h"

void
iap_network_entry_clear(network_entry *network)
{
  if (!network)
    return;

  g_free(network->service_type);
  network->service_type = NULL;

  network->service_attributes = 0;

  g_free(network->service_id);
  network->service_id = NULL;

  g_free(network->network_type);
  network->network_type = NULL;

  network->network_attributes = 0;

  g_free(network->network_id);
  network->network_id = NULL;
}

int
iap_network_entry_service_compare(network_entry *network1,
                                  network_entry *network2)
{
  int rv;

  g_return_val_if_fail(network1 != NULL && network2 != NULL, 0);

  if ((rv = g_strcmp0(network1->service_type, network2->service_type)))
    return rv;

  if ((rv = g_strcmp0(network1->service_id, network2->service_id)))
    return rv;

  if (network1->service_attributes && network2->service_attributes)
  {
    if (network1->service_attributes <= network2->service_attributes)
      rv = network1->service_attributes < network2->service_attributes;
    else
      rv = -1;
  }

  return rv;
}

gboolean
iap_network_entry_to_dbus_iter(DBusMessageIter *iter,  network_entry *entry)
{
  gchar **service_type;
  gchar *empty_string = "";

  g_return_val_if_fail(entry != NULL, FALSE);

  service_type = entry->service_type ? &entry->service_type : &empty_string;

  if (dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, service_type) &&
      dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32,
                                     &entry->service_attributes))
  {
    gchar **service_id =
        entry->service_id ? &entry->service_id : &empty_string;

    if (dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, service_id))
    {
      DBusMessageIter sub;
      gchar **network_type =
          entry->network_type ? &entry->network_type : &empty_string;

      if (dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
                                         network_type) &&
          dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32,
                                         &entry->network_attributes) &&
          dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
                                           DBUS_TYPE_BYTE_AS_STRING, &sub))
      {
        gchar **network_id =
            entry->network_id ? &entry->network_id : &empty_string;
        int n_elements = strlen(*network_id) + 1;

        if (dbus_message_iter_append_fixed_array(&sub, DBUS_TYPE_BYTE,
                                                 network_id, n_elements))
        {
          return dbus_message_iter_close_container(iter, &sub);
        }
      }
    }
  }

  return FALSE;
}
