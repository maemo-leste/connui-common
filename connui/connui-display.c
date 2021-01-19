#include <mce/dbus-names.h>

#include "connui-dbus.h"
#include "connui-dbus-log.h"
#include "connui-log.h"
#include "connui-utils.h"

#include "connui-display.h"


typedef struct
{
  GSList *list;
  DBusPendingCall *call;
} connui_display_info;

static connui_display_info *display_info;

static connui_display_info **
get_display_info()
{
  return &display_info;
}

static void
connui_display_event_do_caller_cb(const gchar *status,
                                  connui_display_info **info)
{
  g_return_if_fail(status != NULL && info != NULL && *info != NULL);


  if ((*info)->list)
    connui_utils_notify_notify_POINTER((*info)->list, (gpointer)status);
}

static DBusHandlerResult
connui_display_event(DBusConnection *connection, DBusMessage *message,
                     void *user_data)
{
  connui_display_info **info = (connui_display_info **)user_data;

  if (info && *info)
  {
    if (dbus_message_is_signal(message, MCE_SIGNAL_IF, MCE_DISPLAY_SIG))
    {
      DBusError error;
      const gchar *status;

      dbus_error_init(&error);

      if (dbus_message_get_args(message, &error,
                                DBUS_TYPE_STRING, &status,
                                DBUS_TYPE_INVALID))
      {
        connui_display_event_do_caller_cb(status, info);
      }
      else
      {
        CONNUI_ERR("could not get args from sig, '%s'", error.message);
        dbus_error_free(&error);
      }
    }
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void
get_display_status_cb(DBusPendingCall *pending, void *user_data)
{
  DBusMessage *mcall;
  connui_display_info **info = (connui_display_info **)user_data;

  if (!info || !(*info))
    return;

  if ((*info)->call)
  {
    dbus_pending_call_unref((*info)->call);
    (*info)->call = NULL;
  }

  mcall = dbus_pending_call_steal_reply(pending);

  if (!mcall)
  {
    CONNUI_ERR("no message in pending call");
    return;
  }

  if (dbus_message_get_type(mcall) != DBUS_MESSAGE_TYPE_ERROR)
  {
    DBusError error;
    const gchar *status;

    dbus_error_init(&error);

    if (dbus_message_get_args(mcall, &error,
                              DBUS_TYPE_STRING, &status,
                              DBUS_TYPE_INVALID))
    {
      connui_display_event_do_caller_cb(status, info);
    }
    else
    {
      CONNUI_ERR("could not get args from mcall, '%s'", error.message);
      dbus_error_free(&error);
    }
  }
  else
    CONNUI_DBUS_ERR(mcall);

  dbus_message_unref(mcall);
}

gboolean
connui_display_event_status(connui_utils_notify callback, gpointer user_data)
{
  connui_display_info **info;
  DBusMessage *mcall;

  info = get_display_info();

  if (*info)
    (*info)->list = connui_utils_notify_add((*info)->list, callback, user_data);
  else
  {
    *info = g_new0(connui_display_info, 1);
    (*info)->list = connui_utils_notify_add((*info)->list, callback, user_data);

    if (!connui_dbus_connect_system_bcast_signal(MCE_SIGNAL_IF,
                                                 connui_display_event, info,
                                                 "member='"MCE_DISPLAY_SIG"'"))
    {
      CONNUI_ERR("Unable to connect signal MCE_DISPLAY_SIG");
      (*info)->list = connui_utils_notify_remove((*info)->list, callback);
      g_free(*info);
      *info = NULL;
      return FALSE;
    }
  }

  mcall = connui_dbus_create_method_call(MCE_SERVICE,
                                      MCE_REQUEST_PATH,
                                      MCE_REQUEST_IF,
                                      MCE_DISPLAY_STATUS_GET,
                                      DBUS_TYPE_INVALID);
  if (!mcall)
  {
    CONNUI_ERR("Could not create MCE_DISPLAY_STATUS_GET message");
    return FALSE;
  }

  if (connui_dbus_send_system_mcall(mcall, -1, get_display_status_cb, info,
                                    &(*info)->call))
  {
    dbus_message_unref(mcall);
    return TRUE;
  }

  CONNUI_ERR("could not send mcall");
  dbus_message_unref(mcall);

  return FALSE;
}

void
connui_display_event_close(connui_utils_notify callback)
{
  connui_display_info **info = get_display_info();

  if (!info || !*info)
    return;

  (*info)->list = connui_utils_notify_remove((*info)->list, callback);

  if (!(*info)->list)
  {
    connui_dbus_disconnect_system_bcast_signal(MCE_SIGNAL_IF,
                                               connui_display_event,
                                               info,
                                               "member='"MCE_DISPLAY_SIG"'");
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
