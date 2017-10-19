#include <dbus/dbus.h>

#include "connui-dbus.h"
#include "connui-log.h"

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
  {
    ULOG_ERR("Unable to create DBUS method call '%s.%s'", interface, method);
  }

  return message;
}
