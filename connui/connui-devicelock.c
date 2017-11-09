#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libosso.h>
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <string.h>

#include "connui-dbus.h"
#include "connui-utils.h"
#include "connui-log.h"
#include "connui-devicelock.h"

struct GlobalDataStruct *devicelock_global_data;

static struct GlobalDataStruct **connui_devicelock_get_global_data(void)
{
  return &devicelock_global_data;
}

static void connui_devicelock_do_caller_cb(const char *status, struct GlobalDataStruct **info)
{
  g_return_if_fail(status != NULL && info != NULL && *info != NULL);

  int first_arg = (strcmp(status, MCE_DEVICE_LOCKED) == 0);
  if ((*info)->list)
    connui_utils_notify_notify((*info)->list, &first_arg, 0);
}

static DBusHandlerResult connui_devicelock_devicelock_changed_cb(DBusConnection *connection, DBusMessage *message, struct GlobalDataStruct **user_data)
{
  if (user_data)
  {
    if (*user_data)
    {
      if (dbus_message_is_signal(message, MCE_SIGNAL_IF, MCE_DEVLOCK_MODE_SIG))
      {
        DBusError error;
        char *status;
        dbus_error_init(&error);
        if (dbus_message_get_args(message, &error, 's', &status, 0))
        {
          connui_devicelock_do_caller_cb(status, user_data);
        }
        else
        {
          CONNUI_ERR("connui_fightmode: could not get args from sig, '%s'", error.message);
          dbus_error_free(&error);
        }
      }
    }
  }
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void connui_devicelock_get_mode_cb(DBusPendingCall *pending, struct GlobalDataStruct **user_data)
{
  if (user_data && *user_data)
  {
    if ((*user_data)->call)
    {
      dbus_pending_call_unref((*user_data)->call);
      (*user_data)->call = 0;
    }
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    if (reply)
    {
      if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_ERROR)
      {
        char *status;
        DBusError error;
        dbus_error_init(&error);
        if (!dbus_message_get_args(reply, &error, 's', &status, 0))
        {
          CONNUI_ERR("connui_devicelock: could not get args from mcall, '%s'", error.message);
          dbus_message_unref(reply);
          dbus_error_free(&error);
          return;
        }
        connui_devicelock_do_caller_cb(status, user_data);
      }
      dbus_message_unref(reply);
    }
    else
    {
      CONNUI_ERR("connui_devicelock: no message in pending call");
    }
  }
}

void connui_devicelock_close(connui_devicelock_notify callback)
{
  struct GlobalDataStruct **data = connui_devicelock_get_global_data();
  if (data)
  {
    if (*data)
    {
      (*data)->list = connui_utils_notify_remove((*data)->list, (connui_utils_notify)callback);
      if (!(*data)->list)
      {
        connui_dbus_disconnect_system_bcast_signal(MCE_SIGNAL_IF, (DBusHandleMessageFunction)connui_devicelock_devicelock_changed_cb, data, "member='devicelock_mode_ind'");
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

gboolean connui_devicelock_status(connui_devicelock_notify callback,gpointer user_data)
{
  struct GlobalDataStruct **data = connui_devicelock_get_global_data();
  if (*data)
  {
    (*data)->list = connui_utils_notify_add((*data)->list, (connui_utils_notify)callback, user_data);
  }
  else
  {
    *data = g_new0(struct GlobalDataStruct, 1);
    (*data)->list = connui_utils_notify_add((*data)->list, (connui_utils_notify)callback, user_data);
    if ( !connui_dbus_connect_system_bcast_signal(MCE_SIGNAL_IF, (DBusHandleMessageFunction)connui_devicelock_devicelock_changed_cb, data, "member='devicelock_mode_ind'"))
    {
      connui_utils_notify_remove((*data)->list, (connui_utils_notify)callback);
      g_free(*data);
      *data = 0;
      return FALSE;
    }
  }
  DBusMessage *message = connui_dbus_create_method_call(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DEVLOCK_MODE_GET, 0);
  if (!message)
    return TRUE;
  if (!connui_dbus_send_system_mcall(message, -1, (DBusPendingCallNotifyFunction)connui_devicelock_get_mode_cb, data, &(*data)->call))
    CONNUI_ERR("connui_devicelock: could not send mcall");
  dbus_message_unref(message);
  return TRUE;
}
