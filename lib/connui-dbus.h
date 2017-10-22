#ifndef CONNUI_DBUS_H
#define CONNUI_DBUS_H

typedef void (*connui_dbus_watcher) (gchar *name,gchar *old_owner,gchar *new_owner,gpointer user_data);

osso_return_t connui_dbus_activate_app(osso_context_t *osso, const gchar *application);
DBusMessage *connui_dbus_create_method_call(const char *bus_name, const char *path, const char *interface, const char *method, int first_arg_type, ...);
dbus_bool_t connui_dbus_application_activation(const char *path, const char *name, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_connect_system_path(const char *path, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_disconnect_system_path(const char *path);
dbus_bool_t connui_dbus_connect_session_path(const char *path, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_disconnect_session_path(const char *path);
dbus_bool_t connui_dbus_register_session_service(const char *path, const char *name, unsigned int flags, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_register_system_service(const char *path, const char *name, unsigned int flags, DBusObjectPathMessageFunction function, void *user_data);
void connui_dbus_close();
dbus_bool_t connui_dbus_get_value_and_iterate(DBusMessageIter *iter, int type, void *value);
dbus_bool_t connui_dbus_connect_system_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_connect_session_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_disconnect_session_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_disconnect_system_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_send_system_mcall(DBusMessage *mcall, int timeout_milliseconds, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call);
dbus_bool_t connui_dbus_send_session_mcall(DBusMessage *mcall, int timeout_milliseconds, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call);
DBusMessage *connui_dbus_recv_reply_session_mcall(DBusMessage *message);
DBusMessage *connui_dbus_recv_reply_system_mcall(DBusMessage *message);
dbus_bool_t connui_dbus_send_session_msg(DBusMessage *message);
dbus_bool_t connui_dbus_send_system_msg(DBusMessage *message);
dbus_bool_t connui_dbus_register_watcher(connui_dbus_watcher callback,gpointer user_data, const gchar *match); 
void connui_dbus_unregister_watcher(connui_dbus_watcher callback, const gchar *match);
dbus_bool_t connui_dbus_libosso_application_activation(const char *path, const char *name, DBusObjectPathMessageFunction function, void *user_data);

#endif // CONNUI_DBUS_H
