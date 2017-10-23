#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libosso.h>

#include "connui-dbus.h"
#include "connui-log.h"
#include "connui-utils.h"

static DBusConnection *system_bus = NULL;
static DBusConnection *session_bus = NULL;

void
connui_dbus_close()
{
  if (system_bus)
  {
    dbus_connection_unref(system_bus);
    system_bus = NULL;
  }

  if (session_bus)
  {
    dbus_connection_unref(session_bus);
    session_bus = NULL;
  }
}

DBusMessage *
connui_dbus_create_method_call(const char *bus_name, const char *path,
                               const char *interface, const char *method,
                               int first_arg_type, ...)
{
  DBusMessage *message;
  va_list va;

  va_start(va, first_arg_type);
  message = dbus_message_new_method_call(bus_name, path, interface, method);

  if (message)
  {
    if (first_arg_type)
    {
      if (!dbus_message_append_args_valist(message, first_arg_type, va))
      {
        CONNUI_ERR("Unable to append arguments to DBUS method call '%s.%s'",
                   interface, method);
        dbus_message_unref(message);
        message = NULL;
      }
    }
  }
  else
    ULOG_ERR("Unable to create DBUS method call '%s.%s'", interface, method);

  return message;
}

osso_return_t
connui_dbus_activate_app(osso_context_t *osso, const gchar *application)
{
  return osso_rpc_async_run_with_defaults(osso, application,
                                          "statusbar_app_activate", NULL, NULL,
                                          DBUS_TYPE_INVALID);
}

static DBusConnection *
connui_dbus_get_system()
{
  if (!system_bus)
  {
    DBusError error;

    dbus_error_init(&error);
    system_bus = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

    if (dbus_error_is_set(&error))
    {
      CONNUI_ERR("Failed to initialize dbus system bus: '%s'", error.message);
      dbus_error_free(&error);
    }
    else
      dbus_connection_setup_with_g_main(system_bus, NULL);
  }

  return system_bus;
}

static DBusConnection *
connui_dbus_get_session()
{
  if (!session_bus)
  {
    DBusError error;

    dbus_error_init(&error);
    session_bus = dbus_bus_get(DBUS_BUS_SESSION, &error);

    if (dbus_error_is_set(&error))
    {
      CONNUI_ERR("Failed to initialize dbus sesion bus: '%s'", error.message);
      dbus_error_free(&error);
    }
    else
      dbus_connection_setup_with_g_main(session_bus, NULL);
  }

  return session_bus;
}

static DBusHandlerResult
message_function(DBusConnection *connection, DBusMessage *message,
                 void *user_data)
{
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static dbus_bool_t
connui_dbus_connect_path(DBusConnection *connection, const char *path,
                         DBusObjectPathMessageFunction function,
                         void *user_data)
{
  DBusObjectPathVTable vtable;

  g_return_val_if_fail(connection != NULL, FALSE);

  if (function)
    vtable.message_function = function;
  else
    vtable.message_function = message_function;

  vtable.unregister_function = NULL;

  if (dbus_connection_register_object_path(connection, path, &vtable,
                                           user_data))
  {
    return TRUE;
  }

  CONNUI_ERR("Unable to register signal/method call path");

  return FALSE;
}

dbus_bool_t
connui_dbus_connect_system_path(const char *path,
                                DBusObjectPathMessageFunction function,
                                void *user_data)
{
  if (connui_dbus_connect_path(connui_dbus_get_system(), path, function,
                               user_data))
  {
    return TRUE;
  }

  CONNUI_ERR("Unable to register signal/method system call path");

  return FALSE;
}

dbus_bool_t
connui_dbus_disconnect_system_path(const char *path)
{
  return dbus_connection_unregister_object_path(connui_dbus_get_system(), path);
}

dbus_bool_t
connui_dbus_connect_session_path(const char *path,
                                 DBusObjectPathMessageFunction function,
                                 void *user_data)
{
  if (connui_dbus_connect_path(connui_dbus_get_session(), path, function,
                               user_data))
  {
    return TRUE;
  }

  CONNUI_ERR("Unable to register signal/method session call path");

  return FALSE;
}

dbus_bool_t
connui_dbus_disconnect_session_path(const char *path)
{
  return
      dbus_connection_unregister_object_path(connui_dbus_get_session(), path);
}

static dbus_bool_t
connui_dbus_register_service(DBusConnection *connection, const char *path,
                             const char *name, unsigned int flags,
                             DBusObjectPathMessageFunction function,
                             void *user_data)
{
  DBusError error;

  g_return_val_if_fail(connection != NULL, FALSE);
  dbus_error_init(&error);

  if (connui_dbus_connect_path(connection, path, function, user_data) &&
      dbus_bus_request_name(connection, name, flags, &error) != -1)
  {
    return TRUE;
  }

  CONNUI_ERR("Could not register service, returned: %s", error.message);

  return FALSE;
}

dbus_bool_t
connui_dbus_register_session_service(const char *path, const char *name,
                                     unsigned int flags,
                                     DBusObjectPathMessageFunction function,
                                     void *user_data)
{
  return connui_dbus_register_service(connui_dbus_get_session(), path, name,
                                      flags, function, user_data);
}

dbus_bool_t
connui_dbus_register_system_service(const char *path, const char *name,
                                    unsigned int flags,
                                    DBusObjectPathMessageFunction function,
                                    void *user_data)
{
  return connui_dbus_register_service(connui_dbus_get_system(), path, name,
                                      flags, function, user_data);
}

dbus_bool_t
connui_dbus_libosso_application_activation(
    const char *path, const char *name, DBusObjectPathMessageFunction function,
    void *user_data)
{
  return
      connui_dbus_register_session_service(path, name, 0, function, user_data);
}

dbus_bool_t
connui_dbus_application_activation(const char *path, const char *name,
                                   DBusObjectPathMessageFunction function,
                                   void *user_data)
{
  if (connui_dbus_connect_system_path(path, function, user_data))
  {
    return connui_dbus_libosso_application_activation(path, name, function,
                                                      user_data);
  }

  CONNUI_ERR("Unable to register system bus signal/method call path");

  return FALSE;
}

dbus_bool_t
connui_dbus_get_value_and_iterate(DBusMessageIter *iter, int type, void *value)
{
  if (dbus_message_iter_get_arg_type(iter) != type)
    return FALSE;

  dbus_message_iter_get_basic(iter, value);
  dbus_message_iter_next(iter);

  return TRUE;
}

static dbus_bool_t
connui_dbus_connect_bcast_signal(DBusConnection *connection,
                                 const char *interface,
                                 DBusHandleMessageFunction function,
                                 void *user_data, const char *signal)
{
  gchar *rule;
  dbus_bool_t rv = FALSE;

  if (dbus_connection_add_filter(connection, function, user_data, NULL))
  {
    if (signal)
    {
      rule =
          g_strdup_printf("type='signal',interface='%s',%s", interface, signal);
    }
    else
      rule = g_strdup_printf("type='signal',interface='%s'", interface);

    if (rule)
    {
      DBusError error;

      dbus_error_init(&error);
      dbus_bus_add_match(connection, rule, &error);
      g_free(rule);

      if (dbus_error_is_set(&error))
      {
        CONNUI_ERR("Could not add match for broadcast signal: %s",
                   error.message);
      }
      else
        rv = TRUE;

      dbus_error_free(&error);
    }
  }
  else
    CONNUI_ERR("Could not add filter");

  return rv;
}

dbus_bool_t
connui_dbus_connect_system_bcast_signal(const char *interface,
                                        DBusHandleMessageFunction function,
                                        void *user_data, const char *signal)
{
  return
      connui_dbus_connect_bcast_signal(connui_dbus_get_system(), interface,
                                       function, user_data, signal);
}

dbus_bool_t
connui_dbus_connect_session_bcast_signal(const char *interface,
                                         DBusHandleMessageFunction function,
                                         void *user_data, const char *signal)
{
  return
      connui_dbus_connect_bcast_signal(connui_dbus_get_session(), interface,
                                       function, user_data, signal);
}

static dbus_bool_t
connui_dbus_disconnect_bcast_signal(DBusConnection *connection,
                                    const char *interface,
                                    DBusHandleMessageFunction function,
                                    void *user_data, const char *signal)
{
  gchar *rule;
  DBusError error;

  if (signal)
  {
    rule =
        g_strdup_printf("type='signal',interface='%s',%s", interface, signal);
  }
  else
    rule = g_strdup_printf("type='signal',interface='%s'", interface);

  if (!rule)
    return FALSE;

  dbus_error_init(&error);
  dbus_bus_remove_match(connection, rule, &error);
  g_free(rule);

  if (dbus_error_is_set(&error))
  {
    CONNUI_ERR("Could not remove match for broadcast signal: %s",
               error.message);
    dbus_error_free(&error);

    return FALSE;
  }

  dbus_connection_remove_filter(connection, function, user_data);

  return TRUE;
}

dbus_bool_t
connui_dbus_disconnect_session_bcast_signal(const char *interface,
                                            DBusHandleMessageFunction function,
                                            void *user_data, const char *signal)
{
  return connui_dbus_disconnect_bcast_signal(connui_dbus_get_session(),
                                             interface, function, user_data,
                                             signal);
}

dbus_bool_t
connui_dbus_disconnect_system_bcast_signal(const char *interface,
                                           DBusHandleMessageFunction function,
                                           void *user_data, const char *signal)
{
  return connui_dbus_disconnect_bcast_signal(connui_dbus_get_system(),
                                             interface, function, user_data,
                                             signal);
}

static DBusHandlerResult
name_owner_changed_cb(DBusConnection *connection, DBusMessage *message,
                      void *user_data)
{
  //todo
  return DBUS_HANDLER_RESULT_HANDLED;
}

static dbus_bool_t
connui_dbus_send_msg(DBusConnection *connection, DBusMessage *message)
{
  g_return_val_if_fail(connection != NULL, FALSE);
  g_return_val_if_fail(message != NULL, FALSE);

  int type = dbus_message_get_type(message);
  if (type == DBUS_MESSAGE_TYPE_SIGNAL || type == DBUS_MESSAGE_TYPE_METHOD_RETURN || type == DBUS_MESSAGE_TYPE_ERROR)
  {
    dbus_connection_send(connection, message, 0);
    return TRUE;
  }
  else
  {
    CONNUI_ERR("dbus message %p is not a signal, mcall return or error", message);
    return FALSE;
  }
}

dbus_bool_t connui_dbus_send_session_msg(DBusMessage *message)
{
  return connui_dbus_send_msg(connui_dbus_get_session(),message);
}

dbus_bool_t connui_dbus_send_system_msg(DBusMessage *message)
{
  return connui_dbus_send_msg(connui_dbus_get_system(),message);
}

static DBusMessage *
connui_dbus_mcall_reply(DBusConnection *connection, DBusMessage *message)
{
  DBusMessage *ret;
  DBusError error;
  if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL)
  {
    dbus_error_init(&error);
    ret = dbus_connection_send_with_reply_and_block(connection, message, 5000, &error);
    if (!ret)
    {
      CONNUI_ERR("Failed DBUS method call with error '%s': %s", error.name, error.message);
      dbus_error_free(&error);
    }
  }
  else
  {
    CONNUI_ERR("dbus message %p is not a method call", message);
    ret = NULL;
  }
  return ret;
}

DBusMessage *connui_dbus_recv_reply_session_mcall(DBusMessage *message)
{
  return connui_dbus_mcall_reply(connui_dbus_get_session(),message);
}

DBusMessage *connui_dbus_recv_reply_system_mcall(DBusMessage *message)
{
  return connui_dbus_mcall_reply(connui_dbus_get_system(),message);
}

static dbus_bool_t
connui_dbus_mcall_send(DBusConnection *connection, DBusMessage *mcall, int timeout, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call)
{
  DBusPendingCall *pending;
  g_return_val_if_fail(connection != NULL, FALSE);
  g_return_val_if_fail(mcall != NULL, FALSE);

  if (dbus_message_get_type(mcall) != DBUS_MESSAGE_TYPE_METHOD_CALL)
  {
    CONNUI_ERR("dbus message %p is not a method call", mcall);
    return FALSE;
  }
  if (notify)
  {
    if (!dbus_connection_send_with_reply(connection, mcall, &pending, timeout))
    {
      CONNUI_ERR("connui_dbus_send_message(): send with reply failed");
      return 0;
    }
    if (!dbus_pending_call_set_notify(pending, notify, user_data, 0))
    {
      CONNUI_ERR("connui_dbus_send_message(): set notify failed");
      dbus_pending_call_cancel(pending);
      dbus_pending_call_unref(pending);
      return 0;
    }
  }
  else
  {
    dbus_message_set_no_reply(mcall, TRUE);
    if (!dbus_connection_send(connection, mcall, 0))
    {
      CONNUI_ERR("connui_dbus_send_message(): send without reply failed");
      return 0;
    }
  }
  if (call)
  {
    return TRUE;
    *call = pending;
  }
  else if (pending)
  {
    dbus_pending_call_unref(pending);
    return TRUE;
  }
  else
  {
    return TRUE;
  }
}

dbus_bool_t connui_dbus_send_session_mcall(DBusMessage *mcall, int timeout_milliseconds, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call)
{
  return connui_dbus_mcall_send(connui_dbus_get_session(), mcall, timeout_milliseconds, notify, user_data, call);
}

dbus_bool_t connui_dbus_send_system_mcall(DBusMessage *mcall, int timeout_milliseconds, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call)
{
  return connui_dbus_mcall_send(connui_dbus_get_system(), mcall, timeout_milliseconds, notify, user_data, call);
}

GSList **connui_dbus_watchers;

void
connui_dbus_unregister_watcher(connui_dbus_watcher callback, const gchar *match)
{
  gchar *str;

  if (connui_dbus_watchers)
  {
    *connui_dbus_watchers =
        connui_utils_notify_remove(*connui_dbus_watchers, callback);
    str = g_strdup_printf("member='NameOwnerChanged',arg0='%s'", match);

    if (str)
    {
      connui_dbus_disconnect_system_bcast_signal("org.freedesktop.DBus",
                                                 name_owner_changed_cb,
                                                 &connui_dbus_watchers,
                                                 str);
      g_free(str);

      if (!*connui_dbus_watchers)
      {
        g_free(connui_dbus_watchers);
        connui_dbus_watchers = NULL;
      }
    }
  }
}

dbus_bool_t
connui_dbus_register_watcher(connui_dbus_watcher callback, gpointer user_data,
                             const gchar *match)
{
  gchar *str;
  dbus_bool_t result;

  if (!connui_dbus_watchers)
    connui_dbus_watchers = g_new0(GSList *, 1);

  *connui_dbus_watchers = connui_utils_notify_add(*connui_dbus_watchers,
                                                  callback, user_data);
  str = g_strdup_printf("member='NameOwnerChanged',arg0='%s'", match);

  if (str && connui_dbus_connect_system_bcast_signal("org.freedesktop.DBus",
                                                     name_owner_changed_cb,
                                                     &connui_dbus_watchers,str))
  {
    g_free(str);
    result = TRUE;
  }
  else
  {
    CONNUI_ERR("unable to register DBUS watcher signal");
    *connui_dbus_watchers = connui_utils_notify_remove(*connui_dbus_watchers,
                                                       callback);
    g_free(str);

    if (*connui_dbus_watchers)
      result = FALSE;
    else
    {
      g_free(connui_dbus_watchers);
      result = FALSE;
      connui_dbus_watchers = 0;
    }
  }

  return result;
}
