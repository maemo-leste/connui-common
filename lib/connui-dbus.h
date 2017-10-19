#ifndef CONNUI_DBUS_H
#define CONNUI_DBUS_H

osso_return_t connui_dbus_activate_app(osso_context_t *osso, const gchar *application);
DBusMessage *connui_dbus_create_method_call(const char *bus_name, const char *path, const char *interface, const char *method, int first_arg_type, ...);
dbus_bool_t connui_dbus_application_activation(const char *path, const char *name, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_connect_system_path(const char *path, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_register_session_service(const char *path, const char *name, unsigned int flags, DBusObjectPathMessageFunction function, void *user_data);

#endif // CONNUI_DBUS_H
