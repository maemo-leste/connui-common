#include <gconf/gconf-client.h>
#include <icd/network_api_defines.h>

#include <string.h>

#include "connui-dbus.h"
#include "iap-network.h"
#include "iap-settings.h"

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

network_entry *
iap_network_entry_dup(network_entry *entry)
{
  network_entry *dup_entry;

  if (!entry)
    return NULL;

  dup_entry = g_new0(network_entry, 1);
  dup_entry->service_type = g_strdup(entry->service_type);
  dup_entry->service_attributes = entry->service_attributes;
  dup_entry->service_id = g_strdup(entry->service_id);
  dup_entry->network_attributes = entry->network_attributes;
  dup_entry->network_type = g_strdup(entry->network_type);
  dup_entry->network_id = g_strdup(entry->network_id);

  return dup_entry;
}

gboolean
iap_network_entry_from_dbus_iter(DBusMessageIter *iter, network_entry *entry)
{
  int n_elements = 0;
  gchar *s = NULL;

  g_return_val_if_fail(entry != NULL, FALSE);

  if (connui_dbus_get_value_and_iterate(iter, DBUS_TYPE_STRING, &s))
  {
    entry->service_type = g_strdup(s);

    if (connui_dbus_get_value_and_iterate(iter, DBUS_TYPE_UINT32,
                                          &entry->service_attributes) &&
        connui_dbus_get_value_and_iterate(iter, DBUS_TYPE_STRING, &s))
    {
      entry->service_id = g_strdup(s);

      if (connui_dbus_get_value_and_iterate(iter, DBUS_TYPE_STRING, &s))
      {
        entry->network_type = g_strdup(s);

        if (connui_dbus_get_value_and_iterate(iter, DBUS_TYPE_UINT32,
                                              &entry->network_attributes) &&
            ((dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_ARRAY &&
              dbus_message_iter_get_element_type(iter) == DBUS_TYPE_BYTE) ||
             dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_STRING))
        {
          if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_ARRAY)
          {
            DBusMessageIter sub;

            dbus_message_iter_recurse(iter, &sub);
            dbus_message_iter_get_fixed_array(&sub, &s, &n_elements);
            entry->network_id = g_strndup(s, n_elements);
          }
          else
          {
            dbus_message_iter_get_basic(iter, &s);
            entry->network_id = g_strdup(s);
          }

          dbus_message_iter_next(iter);

          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

gboolean
iap_network_entry_disconnect(guint connection_flags, network_entry *entry)
{
  DBusMessage *mcall;
  DBusMessage *reply;
  DBusMessageIter iter;
  int reply_type;

  mcall = connui_dbus_create_method_call("com.nokia.icd2",
                                         "/com/nokia/icd2",
                                         "com.nokia.icd2",
                                         "disconnect_req",
                                         DBUS_TYPE_INVALID);
  if (!mcall)
    return FALSE;

  dbus_message_iter_init_append(mcall, &iter);

  if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,
                                      &connection_flags) ||
      !iap_network_entry_to_dbus_iter(&iter, entry))
  {
    dbus_message_unref(mcall);
    return FALSE;
  }

  reply = connui_dbus_recv_reply_system_mcall(mcall);
  dbus_message_unref(mcall);

  if (!reply)
    return FALSE;

  reply_type = dbus_message_get_type(reply);
  dbus_message_unref(reply);

  if (reply_type == DBUS_MESSAGE_TYPE_ERROR)
    return FALSE;

  return TRUE;
}

gboolean
iap_network_entry_is_saved(network_entry *entry)
{
  g_return_val_if_fail(entry != NULL, FALSE);

  if (entry->network_attributes & 0x1000000)
  {
    GConfValue *val =
        iap_settings_get_gconf_value(entry->network_id, "temporary");

    if (val)
    {
      gboolean temporary = gconf_value_get_bool(val);

      gconf_value_free(val);
      return !temporary;
    }

    return TRUE;
  }

  return FALSE;
}

int
iap_network_entry_compare(network_entry *network1, network_entry *network2)
{
  g_return_val_if_fail(network1 != NULL && network2 != NULL, 0);

  return iap_network_entry_service_compare(network1, network2) ||
      iap_network_entry_network_compare(network1, network2);
}

int
iap_network_entry_network_compare(network_entry *network1,
                                  network_entry *network2)
{
  int rv;
  unsigned int capabilities1;
  unsigned int capabilities2;

  if ((rv = g_strcmp0(network1->network_type, network2->network_type)))
    return rv;

  if ((rv = g_strcmp0(network1->network_id, network2->network_id)))
    return rv;

  capabilities1 = network2->network_attributes & ICD_NW_ATTR_LOCALMASK;
  capabilities2 = network1->network_attributes & ICD_NW_ATTR_LOCALMASK;

  if (capabilities1 | capabilities2)
  {
    if (capabilities2 <= capabilities1)
      rv = capabilities2 < capabilities1;
    else
      rv = -1;
  }

  return rv;
}

gboolean
iap_network_entry_equal(gconstpointer a, gconstpointer b)
{
  const network_entry *entry1 = a;
  const network_entry *entry2 = b;

  if (!entry1->service_type && entry2->service_type)
    return FALSE;

  if (!entry1->service_id && entry2->service_id)
    return FALSE;

  if (!entry1->network_type && entry2->network_type)
    return FALSE;

  if (!entry1->network_id && entry2->network_id)
    return FALSE;

  if ( !entry2->service_type && entry1->service_type)
    return FALSE;

  if (!entry2->service_id && entry1->service_id)
    return FALSE;

  if (!entry2->network_type && entry1->network_type)
    return FALSE;

  if (!entry2->network_id && entry1->network_id)
    return FALSE;

  if (((entry1->network_attributes & ICD_NW_ATTR_LOCALMASK) !=
       (entry2->network_attributes & ICD_NW_ATTR_LOCALMASK)) ||
      entry1->service_attributes != entry2->service_attributes)
  {
    return FALSE;
  }


  if ((entry1->service_type && entry2->service_type &&
       strcmp(entry1->service_type, entry2->service_type)) ||
      (entry1->service_id && entry2->service_id &&
       strcmp(entry1->service_id, entry2->service_id)) ||
      (entry1->network_type && entry2->network_type &&
       strcmp(entry1->network_type, entry2->network_type)))
  {
    return FALSE;
  }
  else if (entry1->network_id && entry2->network_id)
    return strcmp(entry1->network_id, entry2->network_id) == 0;

  return TRUE;
}

guint
iap_network_entry_hash(gconstpointer key)
{
  const network_entry *entry = (network_entry *)key;
  guint64 hash_sum = 0;

  if (entry->service_type)
    hash_sum = g_str_hash(entry->service_type);

  hash_sum += g_int_hash(&entry->service_attributes);

  if (entry->service_id)
    hash_sum += g_str_hash(entry->service_id);

  if (entry->network_type)
    hash_sum += g_str_hash(entry->network_type);

  hash_sum += g_int_hash(&entry->network_attributes);

  if (entry->network_id)
    hash_sum += g_str_hash(entry->network_id);

  return hash_sum / 6ULL;
}
