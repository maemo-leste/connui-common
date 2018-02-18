#include "connui-dbus.h"
#include "connui-utils.h"
#include "connui-log.h"
#include "connui-cellular-data.h"

typedef struct
{
  GSList *list;
  DBusPendingCall *call;
} connui_cellular_info;

static connui_cellular_info *cellular_info = NULL;

static connui_cellular_info **
get_cellular_data_info()
{
  return &cellular_info;
}

static void
connui_cellular_data_suspended_do_caller_cb(gboolean suspended, guint32 code,
                                            connui_cellular_info **info)
{
  g_return_if_fail(info != NULL && *info != NULL);

  if ((*info)->list)
    connui_utils_notify_notify((*info)->list, &suspended, &code, NULL);
}

static DBusHandlerResult
connui_cellular_data_suspended_changed_cb(DBusConnection *connection,
                                          DBusMessage *message,
                                          void *user_data)
{
  connui_cellular_info **info = (connui_cellular_info **)user_data;

  if (info && *info)
  {
    dbus_uint32_t code = 0;
    gboolean suspended;

    if (dbus_message_is_signal(message, "com.nokia.csd.GPRS", "Available"))
      suspended = FALSE;
    else
    {
      if (!dbus_message_is_signal(message, "com.nokia.csd.GPRS", "Suspended"))
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

      if (!dbus_message_get_args(message, NULL,
                                 DBUS_TYPE_UINT32, &code,
                                 DBUS_TYPE_INVALID))
      {
        CONNUI_ERR("Can't get args from dbus message");
      }

      suspended = TRUE;
    }

    connui_cellular_data_suspended_do_caller_cb(suspended, code, info);
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
connui_cellular_data_suspended_available_cb(DBusPendingCall *pending,
                                            void *user_data)
{
  connui_cellular_info **info = (connui_cellular_info **)user_data;
  DBusMessageIter sub;
  DBusMessageIter iter;
  dbus_bool_t suspended;
  DBusMessage *reply;

  if (!info || !(*info))
    return;

  suspended = FALSE;
  if ((*info)->call)
  {
    dbus_pending_call_unref((*info)->call);
    (*info)->call = NULL;
  }

  reply = dbus_pending_call_steal_reply(pending);

  if (!reply)
  {
    CONNUI_ERR("no message in pending call");
    return;
  }

  if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_ERROR)
  {
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
      CONNUI_ERR("could not get args from mcall");
      dbus_message_unref(reply);
      return;
    }

    dbus_message_iter_recurse(&iter, &sub);

    if (!connui_dbus_get_value_and_iterate(&sub, DBUS_TYPE_BOOLEAN, &suspended))
    {
      CONNUI_ERR("could not get status from mcall");
      dbus_message_unref(reply);
      return;
    }

    connui_cellular_data_suspended_do_caller_cb(!suspended, 0, info);
  }

  dbus_message_unref(reply);
}

void
connui_cellular_data_close(connui_cellular_data_notify callback)
{
  connui_cellular_info **info = get_cellular_data_info();

  if (info && *info)
  {
    (*info)->list = connui_utils_notify_remove((*info)->list,
                                               (connui_utils_notify)callback);
    if (!(*info)->list)
    {
      connui_dbus_disconnect_system_bcast_signal(
            "com.nokia.csd.GPRS", connui_cellular_data_suspended_changed_cb,
            info, NULL);

      if ((*info)->call)
      {
        dbus_pending_call_cancel((*info)->call);
        dbus_pending_call_unref((*info)->call);
        (*info)->call = NULL;
      }

      g_free(*info);
      *info = NULL;
    }
  }
}

gboolean
connui_cellular_data_suspended_status(connui_cellular_data_notify callback,
                                      gpointer user_data)
{
  connui_cellular_info **info = get_cellular_data_info();
  DBusMessage *mcall;
  const gchar *s1 = "Available";
  const gchar *s2 = "com.nokia.csd.GPRS";

  if (*info)
  {
    (*info)->list = connui_utils_notify_add((*info)->list,
                                            (connui_utils_notify)callback,
                                            user_data);
  }
  else
  {
    *info = g_new0(connui_cellular_info, 1);

    (*info)->list = connui_utils_notify_add((*info)->list,
                                            (connui_utils_notify)callback,
                                            user_data);

    if (!connui_dbus_connect_system_bcast_signal(
          "com.nokia.csd.GPRS",
          connui_cellular_data_suspended_changed_cb, info, NULL))
    {
      connui_utils_notify_remove((*info)->list, (connui_utils_notify)callback);
      g_free(*info);
      *info = NULL;
      return FALSE;
    }
  }

  mcall = connui_dbus_create_method_call("com.nokia.csd",
                                         "/com/nokia/csd/gprs",
                                         "org.freedesktop.DBus.Properties",
                                         "Get",
                                         DBUS_TYPE_STRING, &s1,
                                         DBUS_TYPE_STRING, &s2,
                                         DBUS_TYPE_INVALID);
  if (mcall)
  {
    if (connui_dbus_send_system_mcall(
          mcall, -1, connui_cellular_data_suspended_available_cb, info,
          &(*info)->call))
    {
      dbus_message_unref(mcall);
      return TRUE;
    }
    else
    {
      CONNUI_ERR("could not send mcall");
      dbus_message_unref(mcall);
      return FALSE;
    }
  }

  return FALSE;
}
