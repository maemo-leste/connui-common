#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libosso.h>
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <string.h>

#include "connui-dbus.h"
#include "connui-utils.h"
#include "connui-log.h"
#include "connui-cellular-data.h"

struct GlobalDataStruct *cellular_data_global_data;

static struct GlobalDataStruct **connui_cellular_data_get_global_data(void)
{
  return &cellular_data_global_data;
}

static void connui_cellular_data_suspended_do_caller_cb(gboolean suspended, guint32 code, struct GlobalDataStruct **info)
{
  g_return_if_fail(info != NULL && *info != NULL);

  if ((*info)->list)
    connui_utils_notify_notify((*info)->list, &suspended, &code, 0);
}

static DBusHandlerResult connui_cellular_data_suspended_changed_cb(DBusConnection *connection, DBusMessage *message, struct GlobalDataStruct **user_data)
{
  guint32 code;
  gboolean suspended;
  if (user_data)
  {
    if (*user_data)
    {
      code = 0;
      if (dbus_message_is_signal(message, "com.nokia.csd.GPRS", "Available"))
      {
        suspended = FALSE;
      }
      else
      {
        if (!dbus_message_is_signal(message, "com.nokia.csd.GPRS", "Suspended"))
          return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        if ( !dbus_message_get_args(message, 0, 'u', &code, 0) )
          CONNUI_ERR("Can't get args from dbus message");
        suspended = TRUE;
      }
      connui_cellular_data_suspended_do_caller_cb(suspended, code, user_data);
    }
  }
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void connui_cellular_data_suspended_available_cb(DBusPendingCall *pending, struct GlobalDataStruct **user_data)
{
  DBusMessageIter sub;
  DBusMessageIter iter;
  gboolean value;
  if (user_data && *user_data)
  {
    value = FALSE;
    if ((*user_data)->call)
    {
      dbus_pending_call_unref((*user_data)->call);
      (*user_data)->call = 0;
    }
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    if (!reply)
    {
      CONNUI_ERR("connui_cellular_data_suspended: no message in pending call");
      return;
    }
    if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_ERROR)
    {
      if (!dbus_message_iter_init(reply, &iter) || dbus_message_iter_get_arg_type(&iter) != 'v')
      {
        CONNUI_ERR("connui_cellular_data_suspended: could not get args from mcall");
        dbus_message_unref(reply);
        return;
      }
      dbus_message_iter_recurse(&iter, &sub);
      if (!connui_dbus_get_value_and_iterate(&sub, 'b', &value))
      {
        CONNUI_ERR("connui_cellular_data_suspended: could not get status from mcall");
        dbus_message_unref(reply);
        return;
      }
      connui_cellular_data_suspended_do_caller_cb(value == 0, 0, user_data);
    }
    dbus_message_unref(reply);
    return;
  }
}

void connui_cellular_data_close(connui_cellular_data_notify callback)
{
  struct GlobalDataStruct **data = connui_cellular_data_get_global_data();
  if (data)
  {
    if (*data)
    {
      (*data)->list = connui_utils_notify_remove((*data)->list, (connui_utils_notify)callback);
      if (!(*data)->list)
      {
        connui_dbus_disconnect_system_bcast_signal("com.nokia.csd.GPRS", (DBusHandleMessageFunction)connui_cellular_data_suspended_changed_cb, data, 0);
        if ((*data)->call)
        {
          dbus_pending_call_cancel((*data)->call);
          dbus_pending_call_unref((*data)->call);
          (*data)->call = 0;
        }
        g_free(*data);
        *data = 0;
      }
    }
  }
}

gboolean connui_cellular_data_status(connui_cellular_data_notify callback,gpointer user_data)
{
  struct GlobalDataStruct **data = connui_cellular_data_get_global_data();
  if (*data)
  {
    (*data)->list = connui_utils_notify_add((*data)->list, (connui_utils_notify)callback, user_data);
  }
  else
  {
    *data = g_new0(struct GlobalDataStruct, 1);
    (*data)->list = connui_utils_notify_add((*data)->list, (connui_utils_notify)callback, user_data);
    if (!connui_dbus_connect_system_bcast_signal("com.nokia.csd.GPRS", (DBusHandleMessageFunction)connui_cellular_data_suspended_changed_cb, data, 0))
    {
      connui_utils_notify_remove((*data)->list, (connui_utils_notify)callback);
      g_free(*data);
      *data = 0;
      return FALSE;
    }
  }
  const char *s1 = "Available";
  const char *s2 = "com.nokia.csd.GPRS";
  DBusMessage *message = connui_dbus_create_method_call("com.nokia.csd", "/com/nokia/csd/gprs", "org.freedesktop.DBus.Properties", "Get", 's', &s1, 's', &s2, 0);
  if (message)
  {
    if (connui_dbus_send_system_mcall(message, -1, (DBusPendingCallNotifyFunction)connui_cellular_data_suspended_available_cb, data, &(*data)->call))
    {
      dbus_message_unref(message);
      return TRUE;
    }
    else
    {
      CONNUI_ERR("connui_cellular_data_suspended: could not send mcall");
      dbus_message_unref(message);
      return FALSE;
    }
  }
  return FALSE;
}
