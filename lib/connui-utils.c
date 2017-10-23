#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libosso.h>
#include <gtk/gtk.h>
#include <mce/dbus-names.h>
#include <mce/mode-names.h>

#include "connui-dbus.h"
#include "connui-utils.h"
#include "connui-log.h"

static GtkSettings *settings;

typedef struct _connui_notifier connui_notifier;
struct _connui_notifier
{
  connui_utils_notify callback;
  gpointer user_data;
};

gint
connui_utils_callback_compare(gconstpointer a, gconstpointer b)
{
  if (a == b)
    return 0;
  else
    return -1;
}

GSList *
connui_utils_find_callback(GSList *list, connui_utils_notify notify)
{
  return g_slist_find_custom(list, notify, connui_utils_callback_compare);
}

osso_context_t *
connui_utils_inherit_osso_context(osso_context_t *lib_osso,
                                  const gchar *application,
                                  const gchar *version)
{
  gchar *app = g_strconcat(osso_application_name_get(lib_osso), "-",
                           application, NULL);
  osso_context_t *osso = osso_initialize(app, version, FALSE, 0);

  g_free(app);

  return osso;
}

void
connui_utils_reload_theme()
{
  if (settings || (settings = gtk_settings_get_default()))
    gtk_rc_reparse_all_for_settings(settings, FALSE);
}

void
connui_utils_unblank_display()
{
  DBusMessage *message = connui_dbus_create_method_call(MCE_SERVICE,
                                                        MCE_REQUEST_PATH,
                                                        MCE_REQUEST_IF,
                                                        MCE_DISPLAY_ON_REQ,
                                                        DBUS_TYPE_INVALID);
  if (!message)
  {
    CONNUI_ERR("unable to create request");
    return;
  }

  if (!connui_dbus_send_system_mcall(message, -1, NULL, NULL, NULL))
    CONNUI_ERR("can't send request");
}

GSList *
connui_utils_notify_remove(GSList *list, connui_utils_notify notify)
{
  GSList *iterator;
  connui_utils_notify *cb;
  GSList *temp;

  if (list)
  {
    iterator = list;

    for (;;)
    {
      cb = (connui_utils_notify *)iterator->data;

      if (iterator->data)
      {
        if (*cb == notify)
          break;
      }

      iterator = iterator->next;

      if (!iterator)
        return list;
    }

    temp = g_slist_remove_link(list, iterator);
    g_free(cb);
    g_slist_free(iterator);
    list = temp;
  }

  return list;
}

GSList *
connui_utils_notify_add(GSList *list, connui_utils_notify callback,
                        gpointer user_data)
{
  GSList *l;
  connui_notifier *notify;

  for (l = list; l && l->data != callback; l = l->next);

  if (l)
    return list;

  if (!(notify = g_new0(connui_notifier, 1)))
  {
    CONNUI_ERR("Unable to allocate new notify");
    return list;
  }

  notify->callback = callback;
  notify->user_data = user_data;

  return g_slist_append(l, notify);
}

void
connui_utils_notify_notify(GSList *list, gpointer first_arg, ...)
{
  //todo
}
