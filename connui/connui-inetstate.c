#include <icd/dbus_api.h>

#include <string.h>

#include "connui-inetstate.h"

#include "connui-dbus.h"
#include "connui-flightmode.h"
#include "connui-log.h"
#include "connui-utils.h"

struct inetstate_stats
{
  GSList *notifiers;
  DBusPendingCall *pending;
  guint statistics_timeout;
  gint timeout;
};

struct inetstate
{
  GHashTable *connected;
  GHashTable *connecting;
  gboolean offline;
  GSList *notifiers;
  DBusPendingCall *pending;
  guint timeout_id;
};

struct inetstate_conn_data
{
  network_entry *entry;
  gboolean connecting;
  gboolean connected;
};

static struct inetstate_stats *g_inetstate_stats = NULL;
static struct inetstate *g_inetstate = NULL;

static struct inetstate_stats **
inetstate_get_stats()
{
  return &g_inetstate_stats;
}

static struct inetstate **
inetstate_get()
{
  return &g_inetstate;
}

static DBusHandlerResult
connui_inetstate_stats(DBusConnection *connection, DBusMessage *message,
                       void *user_data)
{
  struct inetstate_stats **stats = (struct inetstate_stats **)user_data;
  DBusMessageIter iter;
  network_entry entry;
  inetstate_network_stats net_stats;
  inetstate_network_stats *pstats = &net_stats;
  network_entry *pentry = &entry;

  if (stats && *stats)
  {
    if (dbus_message_is_signal(message, ICD_DBUS_API_INTERFACE,
                               ICD_DBUS_API_STATISTICS_SIG))
    {
      memset(&entry, 0, sizeof(entry));

      dbus_message_iter_init(message, &iter);

      if (iap_network_entry_from_dbus_iter(&iter, &entry))
      {
        if (connui_dbus_get_value_and_iterate(&iter, DBUS_TYPE_UINT32,
                                              &net_stats.time_active))
        {
          if (connui_dbus_get_value_and_iterate(&iter, DBUS_TYPE_INT32,
                                                &net_stats.signal_strength))
          {
            if (connui_dbus_get_value_and_iterate(&iter, DBUS_TYPE_UINT32,
                                                  &net_stats.tx_bytes) )
            {
              if (connui_dbus_get_value_and_iterate(&iter, DBUS_TYPE_UINT32,
                                                    &net_stats.rx_bytes) )
              {
                if ((*stats)->notifiers)
                {
                  connui_utils_notify_notify((*stats)->notifiers, &pentry,
                                             &pstats, NULL);
                }
              }
              else
                CONNUI_ERR("could not get rx_bytes");
            }
            else
              CONNUI_ERR("could not get tx_bytes");
          }
          else
            CONNUI_ERR("could not get signal value");
        }
        else
          CONNUI_ERR("could not get time");
      }
      else
        CONNUI_ERR("could not get network info from signal");

      iap_network_entry_clear(&entry);
    }
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
connui_inetstate_statistics_request_cb(DBusPendingCall *pending,
                                       gpointer user_data)
{
  struct inetstate_stats **stats = (struct inetstate_stats **)user_data;

  DBusMessage *reply;

  g_return_if_fail(stats != NULL && *stats != NULL);

  if ((*stats)->pending)
  {
    dbus_pending_call_unref((*stats)->pending);
    (*stats)->pending = NULL;
  }

  reply = dbus_pending_call_steal_reply(pending);

  if (reply)
  {
    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
    {
      CONNUI_ERR("Error when getting statistics: %s",
                 dbus_message_get_error_name(reply));
    }

    dbus_message_unref(reply);
  }
  else
    CONNUI_ERR("no message in pending call");
}

static gboolean
connui_inetstate_statistics_request(struct inetstate_stats **stats)
{
  DBusMessage *mcall;

  g_return_val_if_fail(stats != NULL && *stats != NULL, FALSE);

  if ((*stats)->pending)
    return TRUE;

  mcall = connui_dbus_create_method_call(ICD_DBUS_API_INTERFACE,
                                         ICD_DBUS_API_PATH,
                                         ICD_DBUS_API_INTERFACE,
                                         ICD_DBUS_API_STATISTICS_REQ,
                                         DBUS_TYPE_INVALID);

  if (mcall)
  {
    if (connui_dbus_send_system_mcall(mcall, -1,
                                      connui_inetstate_statistics_request_cb,
                                      stats, &(*stats)->pending))
    {
      dbus_message_unref(mcall);
      return TRUE;
    }

    CONNUI_ERR("could not send message");
    dbus_message_unref(mcall);
  }
  else
    CONNUI_ERR("could not create message");

  return FALSE;
}

static gboolean
connui_inetstate_statistics_request_timeout(gpointer user_data)
{
  struct inetstate_stats **stats = (struct inetstate_stats **)user_data;

  g_return_val_if_fail(stats != NULL && *stats != NULL &&
      (*stats)->statistics_timeout != 0, FALSE);

  connui_inetstate_statistics_request(stats);

  return TRUE;
}

gboolean
connui_inetstate_statistics_start(gint interval, inetstate_stats_cb callback,
                                  gpointer user_data)
{
  struct inetstate_stats **stats = inetstate_get_stats();

  if (!*stats)
  {
    *stats = g_new0(struct inetstate_stats, 1);

    if (!connui_dbus_connect_system_path(ICD_DBUS_API_PATH,
                                         connui_inetstate_stats, stats))
    {
      CONNUI_ERR("common inetstate: could not connect statistics sig");
      return FALSE;
    }
  }

  (*stats)->notifiers = connui_utils_notify_add((*stats)->notifiers,
                                               (connui_utils_notify)callback,
                                               user_data);

  if (!(interval <= 0 || ((*stats)->timeout && interval >= (*stats)->timeout)))
  {
    if ((*stats)->statistics_timeout)
    {
      g_source_remove((*stats)->statistics_timeout);
      (*stats)->statistics_timeout = 0;
    }

    if (!(*stats)->statistics_timeout)
    {
      (*stats)->timeout = interval;
      (*stats)->statistics_timeout = g_timeout_add(
            interval, (GSourceFunc)connui_inetstate_statistics_request_timeout,
            stats);

    }
  }

  return connui_inetstate_statistics_request(stats);
}

void
connui_inetstate_statistics_stop(inetstate_stats_cb callback)
{
  struct inetstate_stats **stats = inetstate_get_stats();

  if (stats && *stats)
  {
    (*stats)->notifiers =
        connui_utils_notify_remove((*stats)->notifiers,
                                   (connui_utils_notify)callback);

    if (!(*stats)->notifiers)
    {
      if ((*stats)->statistics_timeout)
      {
        g_source_remove((*stats)->statistics_timeout);
        (*stats)->statistics_timeout = 0;
      }

      if ((*stats)->pending)
      {
        dbus_pending_call_cancel((*stats)->pending);
        dbus_pending_call_unref((*stats)->pending);
        (*stats)->pending = NULL;
      }

      connui_dbus_disconnect_system_path(ICD_DBUS_API_PATH);
      g_free(*stats);
      *stats = NULL;
    }
  }
}

static void
connui_inetstate_notifiers_notify(struct inetstate *inetstate,
                                  enum inetstate_status state,
                                  network_entry *entry)
{
  connui_utils_notify_notify(
        inetstate->notifiers, &state, &entry, DBUS_TYPE_INVALID);
}

static gboolean
remove_all(gpointer key, gpointer value, gpointer user_data)
{
  return TRUE;
}

static void
inetstate_remove_all_hashed_entries(struct inetstate **inetstate)
{
  g_hash_table_foreach_remove((*inetstate)->connected, remove_all, NULL);
  g_hash_table_foreach_remove((*inetstate)->connecting, remove_all, NULL);
}

static gboolean
find_first(gpointer key, gpointer value, gpointer user_data)
{
  return TRUE;
}

static void
connui_inetstat_report_state(struct inetstate **inetstate,
                             struct inetstate_conn_data *connecting,
                             struct inetstate_conn_data *connected,
                             network_entry *entry)
{
  if (connecting ||
      (connecting = (struct inetstate_conn_data *)g_hash_table_find(
         (*inetstate)->connecting, find_first, NULL)))
  {
    if (connecting->connecting)
    {
      connui_inetstate_notifiers_notify(*inetstate,
                                        INETSTATE_STATUS_CONNECTING,
                                        connecting->entry);
    }
    else
    {
      connui_inetstate_notifiers_notify(*inetstate,
                                        INETSTATE_STATUS_DISCONNECTING,
                                        connecting->entry);
    }
  }
  else if (connected ||
           (connected = (struct inetstate_conn_data *)g_hash_table_find(
              (*inetstate)->connected, find_first, NULL)))
  {
    connui_inetstate_notifiers_notify(*inetstate,
                                      INETSTATE_STATUS_CONNECTED,
                                      connected->entry);
  }
  else
  {
    connui_inetstate_notifiers_notify(*inetstate,
                                      INETSTATE_STATUS_ONLINE,
                                      entry);
  }
}

static gboolean
connui_inetstate_signal_timeout(gpointer user_data)
{
  struct inetstate **inetstate = (struct inetstate **)user_data;

  g_return_val_if_fail(inetstate != NULL && *inetstate != NULL, FALSE);

  CONNUI_ERR("ICd claims we are connected, but refuses to provide state for the connected IAPs. We report the current state");

  (*inetstate)->timeout_id = 0;
  connui_inetstat_report_state(inetstate, NULL, NULL, NULL);

  return FALSE;
}

static void
connui_inetstate_state_req_cb(DBusPendingCall *pending, void *user_data)
{
  struct inetstate **inetstate = (struct inetstate **)user_data;
  DBusMessage *reply;

  if (inetstate && *inetstate)
  {
    if ((*inetstate)->pending)
    {
      dbus_pending_call_unref((*inetstate)->pending);
      (*inetstate)->pending = 0;
    }

    reply = dbus_pending_call_steal_reply(pending);

    if (!reply)
    {
      CONNUI_ERR("connui_inetstate: no message in pending call");
      return;
    }

    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
    {
      CONNUI_ERR("connui_inetstate: got error '%s', assuming we're disconnected",
                 dbus_message_get_error_name(reply));
      dbus_message_unref(reply);
    }
    else
    {
      DBusError error;
      dbus_uint32_t num_signals = 0;

      dbus_error_init(&error);

      if (!dbus_message_get_args(reply, &error,
                                 DBUS_TYPE_UINT32, &num_signals,
                                 DBUS_TYPE_INVALID))
      {
        CONNUI_ERR("connui_inetstate: could not get args from mcall: '%s'",
                   error.message);
        dbus_message_unref(reply);
        dbus_error_free(&error);
        return;
      }

      dbus_message_unref(reply);

      if (num_signals)
      {
        if (!(*inetstate)->timeout_id )
        {
          (*inetstate)->timeout_id =
              g_timeout_add(2000, connui_inetstate_signal_timeout, inetstate);
        }

        return;
      }
    }

    inetstate_remove_all_hashed_entries(inetstate);
    connui_inetstate_notifiers_notify(*inetstate,
                                      INETSTATE_STATUS_ONLINE,
                                      NULL);
  }
}

static void
connui_inetstate_flightmode_cb(gboolean offline, gpointer user_data)
{
  struct inetstate **inetstate = (struct inetstate **)user_data;
  gboolean was_offline;
  DBusMessage *mcall;

  if (!inetstate || !*inetstate)
    return;

  was_offline = (*inetstate)->offline;
  (*inetstate)->offline = offline;

  if (offline)
  {
    inetstate_remove_all_hashed_entries(inetstate);
    connui_inetstate_notifiers_notify(*inetstate,
                                      INETSTATE_STATUS_OFFLINE,
                                      NULL);
  }
  else
  {
    if (was_offline)
    {
      connui_inetstate_notifiers_notify((*inetstate),
                                        INETSTATE_STATUS_ONLINE,
                                        NULL);
    }

    mcall = connui_dbus_create_method_call(ICD_DBUS_API_INTERFACE,
                                           ICD_DBUS_API_PATH,
                                           ICD_DBUS_API_INTERFACE,
                                           ICD_DBUS_API_STATE_REQ,
                                           DBUS_TYPE_INVALID);
    if (mcall)
    {
      if (!connui_dbus_send_system_mcall(mcall, -1,
                                         connui_inetstate_state_req_cb,
                                         inetstate, &(*inetstate)->pending))
      {
        CONNUI_ERR("connui_inetstate: could not send mcall");
      }

      dbus_message_unref(mcall);
    }
    else
      CONNUI_ERR("connui_inetstate: could not create mcall message");
  }
}

static void
connui_inetstate_free_conn_data(gpointer data)
{
  struct inetstate_conn_data *conndata = (struct inetstate_conn_data *)data;

  g_return_if_fail(data != NULL);

  iap_network_entry_clear(conndata->entry);
  g_free(conndata->entry);
  g_free(conndata);
}

static void
connui_inetstate_icd_dbus_watcher(gchar *name, gchar *old_owner,
                                  gchar *new_owner, gpointer user_data)
{
  struct inetstate **inetstate = (struct inetstate **)user_data;

  if (name && !strcmp(name, ICD_DBUS_API_INTERFACE) &&
      inetstate && *inetstate && old_owner && *old_owner &&
      new_owner && *new_owner)
  {
    inetstate_remove_all_hashed_entries(inetstate);
    connui_inetstate_notifiers_notify(*inetstate,
                                      INETSTATE_STATUS_ONLINE,
                                      NULL);
  }
}

static DBusHandlerResult
connui_inetstate_icd_signal_cb(DBusConnection *connection, DBusMessage *message,
                               void *user_data)
{
  struct inetstate **inetstate = (struct inetstate **)user_data;
  DBusMessageIter iter;
  network_entry entry;
  dbus_uint32_t connection_state;
  gchar *network_type = NULL;

  if (!inetstate || !*inetstate || (*inetstate)->offline ||
      !dbus_message_is_signal(message, ICD_DBUS_API_INTERFACE,
                              ICD_DBUS_API_STATE_SIG))
  {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  memset(&entry, 0, sizeof(entry));
  dbus_message_iter_init(message, &iter);

  if (!iap_network_entry_from_dbus_iter(&iter, &entry))
    goto out;

  connui_dbus_get_value_and_iterate(&iter, DBUS_TYPE_STRING, &network_type);

  if (!connui_dbus_get_value_and_iterate(&iter,
                                         DBUS_TYPE_UINT32, &connection_state))
  {
    CONNUI_ERR("connui_inetstate: unable to get error state from signal");
    goto out;
  }

  if (entry.network_type && *entry.network_type &&
      entry.network_id && *entry.network_id)
  {
    struct inetstate_conn_data *connected = NULL;
    struct inetstate_conn_data *connecting = NULL;

    if ((*inetstate)->timeout_id)
    {
      g_source_remove((*inetstate)->timeout_id);
      (*inetstate)->timeout_id = 0;
    }

    switch (connection_state)
    {
      case ICD_STATE_DISCONNECTED:
      {
        if (*network_type)
        {
          struct inetstate_conn_data *conn_data = (struct inetstate_conn_data *)
              g_hash_table_lookup((*inetstate)->connected, &entry);

          if (!conn_data)
          {
            conn_data = (struct inetstate_conn_data *)
                g_hash_table_lookup((*inetstate)->connecting, &entry);
          }

          if (conn_data && conn_data->connected)
          {
            connui_inetstate_notifiers_notify(*inetstate,
                                              INETSTATE_STATUS_DISCONNECTED,
                                              &entry);
          }
        }

        g_hash_table_remove((*inetstate)->connected, &entry);
        g_hash_table_remove((*inetstate)->connecting, &entry);
        break;
      }
      case ICD_STATE_CONNECTING:
      case ICD_STATE_DISCONNECTING:
      {
        struct inetstate_conn_data *conn_data =
            g_new0(struct inetstate_conn_data, 1);

        conn_data->entry = iap_network_entry_dup(&entry);;
        conn_data->connecting = connection_state == ICD_STATE_CONNECTING;
        conn_data->connected =
            g_hash_table_lookup((*inetstate)->connected, &entry) != NULL;
        g_hash_table_remove((*inetstate)->connected, &entry);
        g_hash_table_replace((*inetstate)->connecting, conn_data->entry,
                             conn_data);
        connecting = conn_data;
        break;
      }
      case ICD_STATE_CONNECTED:
      {
        struct inetstate_conn_data *conn_data =
            g_new0(struct inetstate_conn_data, 1);

        conn_data->entry = iap_network_entry_dup(&entry);
        conn_data->connected = TRUE;
        g_hash_table_replace((*inetstate)->connected, conn_data->entry,
                             conn_data);
        g_hash_table_remove((*inetstate)->connecting, &entry);
        connected = conn_data;
        break;
      }
      default:
        CONNUI_ERR("Unknown state %d received from ICd2", connection_state);
        goto out;
    }

    connui_inetstat_report_state(inetstate, connecting, connected, &entry);
  }
  else
  {
   CONNUI_ERR("connui_inetstate: ICd sent %s with invalid values",
              ICD_DBUS_API_STATE_SIG);
  }

out:
  iap_network_entry_clear(&entry);

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

gboolean
connui_inetstate_status(inetstate_cb callback, gpointer user_data)
{
  struct inetstate **inetstate = inetstate_get();

  if (*inetstate)
  {
    (*inetstate)->notifiers =
        connui_utils_notify_add((*inetstate)->notifiers,
                                (connui_utils_notify)callback, user_data);
    goto ok;
  }

  *inetstate = g_new0(struct inetstate, 1);

  (*inetstate)->connected =
      g_hash_table_new_full(iap_network_entry_hash,
                            iap_network_entry_equal, NULL,
                            connui_inetstate_free_conn_data);
  (*inetstate)->connecting =
      g_hash_table_new_full(iap_network_entry_hash,
                            iap_network_entry_equal, NULL,
                            connui_inetstate_free_conn_data);
  (*inetstate)->notifiers =
      connui_utils_notify_add((*inetstate)->notifiers,
                              (connui_utils_notify)callback, user_data);

  if (connui_dbus_connect_system_bcast_signal(ICD_DBUS_API_INTERFACE,
                                              connui_inetstate_icd_signal_cb,
                                              inetstate, NULL))
  {
    if (connui_dbus_register_watcher(connui_inetstate_icd_dbus_watcher,
                                     inetstate, ICD_DBUS_API_INTERFACE))
    {
      goto ok;
    }

    CONNUI_ERR("common inetstate: could not register ICd watcher");
  }
  else
    CONNUI_ERR("common inetstate: could not connect system bcast sig");

  connui_dbus_disconnect_system_bcast_signal(ICD_DBUS_API_INTERFACE,
                                             connui_inetstate_icd_signal_cb,
                                             inetstate, NULL);

  (*inetstate)->notifiers =
      connui_utils_notify_remove((*inetstate)->notifiers,
                                 (connui_utils_notify)callback);

  if ((*inetstate)->connected)
    g_hash_table_destroy((*inetstate)->connected);

  if ((*inetstate)->connecting)
    g_hash_table_destroy((*inetstate)->connecting);

  g_free(*inetstate);
  *inetstate = NULL;
  return FALSE;

ok:
  connui_flightmode_status(connui_inetstate_flightmode_cb, inetstate);

  return TRUE;
}

void
connui_inetstate_close(inetstate_cb callback)
{
  struct inetstate **inetstate = inetstate_get();

  if (!*inetstate)
    return;

  (*inetstate)->notifiers =
      connui_utils_notify_remove((*inetstate)->notifiers,
                                 (connui_utils_notify)callback);

  if ((*inetstate)->notifiers)
    return;

  if ((*inetstate)->pending)
  {
    dbus_pending_call_cancel((*inetstate)->pending);
    dbus_pending_call_unref((*inetstate)->pending);
    (*inetstate)->pending = NULL;
  }

  if ((*inetstate)->timeout_id)
  {
    g_source_remove((*inetstate)->timeout_id);
    (*inetstate)->timeout_id = 0;
  }

  connui_dbus_unregister_watcher(connui_inetstate_icd_dbus_watcher,
                                 ICD_DBUS_API_INTERFACE);
  connui_flightmode_close(connui_inetstate_flightmode_cb);
  connui_dbus_disconnect_system_bcast_signal(ICD_DBUS_API_INTERFACE,
                                             connui_inetstate_icd_signal_cb,
                                             inetstate, NULL);
  g_hash_table_destroy((*inetstate)->connected);
  g_hash_table_destroy((*inetstate)->connecting);
  g_free(*inetstate);
  *inetstate = NULL;
}
