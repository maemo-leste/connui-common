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
connui_utils_notify_remove(GSList *list, connui_utils_notify callback)
{
  GSList *l;
  GSList *temp;

  if (list)
  {
    connui_notifier *notify;

    l = list;

    for (;;)
    {
      notify = (connui_notifier *)l->data;

      if (notify && notify->callback == callback)
        break;

      l = l->next;

      if (!l)
        return list;
    }

    temp = g_slist_remove_link(list, l);
    g_free(notify);
    g_slist_free(l);
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
connui_utils_notify_notify_POINTER(GSList *list, gpointer arg1)
{
  typedef void (*cb_POINTER)(gpointer, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
      ((cb_POINTER)notifier->callback)(arg1, notifier->user_data);

    list = list->next;
  }
}

void
connui_utils_notify_notify_POINTER_POINTER(GSList *list, gpointer arg1,
                                           gpointer arg2)
{
  typedef void (*cb_POINTER_POINTER)(gpointer, gpointer, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
    {
      ((cb_POINTER_POINTER)notifier->callback)(arg1, arg2, notifier->user_data);
    }

    list = list->next;
  }
}

void
connui_utils_notify_notify_POINTER_POINTER_POINTER(GSList *list, gpointer arg1,
                                                   gpointer arg2, gpointer arg3)
{
  typedef void (*cb_POINTER_POINTER_POINTER)(gpointer, gpointer, gpointer, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
    {
      ((cb_POINTER_POINTER_POINTER)notifier->callback)(arg1, arg2, arg3,
                                                       notifier->user_data);
    }

    list = list->next;
  }
}

void
connui_utils_notify_notify_INT_POINTER_POINTER_POINTER_POINTER(
    GSList *list, gint arg1, gpointer arg2, gpointer arg3, gpointer arg4,
    gpointer arg5)
{
  typedef void (*cb_INT_POINTER_POINTER_POINTER_POINTER)(
        gint, gpointer, gpointer, gpointer, gpointer, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
    {
      ((cb_INT_POINTER_POINTER_POINTER_POINTER)notifier->callback)(
            arg1, arg2, arg3, arg4, arg5, notifier->user_data);
    }

    list = list->next;
  }
}

void
connui_utils_notify_notify_BOOLEAN(GSList *list, gboolean arg1)
{
  typedef void (*cb_BOOLEAN)(gboolean, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
      ((cb_BOOLEAN)notifier->callback)(arg1, notifier->user_data);

    list = list->next;
  }
}

void
connui_utils_notify_notify_INT_POINTER(GSList *list, int arg1,
                                       gpointer arg2)
{
  typedef void (*cb_INT_POINTER)(int, gpointer, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
    {
      ((cb_INT_POINTER)notifier->callback)(arg1, arg2, notifier->user_data);
    }

    list = list->next;
  }
}

void
connui_utils_notify_notify_BOOLEAN_UINT(GSList *list, gboolean arg1, guint arg2)
{
  typedef void (*cb_BOOLEAN_UINT)(gboolean, guint, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
      ((cb_BOOLEAN_UINT)notifier->callback)(arg1, arg2, notifier->user_data);

    list = list->next;
  }
}

void
connui_utils_notify_notify_UINT(GSList *list, guint arg1)
{
  typedef void (*cb_UINT)(guint, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
      ((cb_UINT)notifier->callback)(arg1, notifier->user_data);

    list = list->next;
  }
}

void
connui_utils_notify_notify_UINT64_UINT64_TIMET_BOOLEAN_POINTER(
    GSList *list, guint64 arg1, guint64 arg2, time_t arg3, gboolean arg4,
    gpointer arg5)
{
  typedef void (*cb_UINT64_UINT64_TIMET_BOOLEAN_POINTER)(guint64, guint64, time_t, gboolean, gpointer, gpointer);

  while (list)
  {
    connui_notifier *notifier = (connui_notifier *)list->data;

    if (notifier && notifier->callback)
      ((cb_UINT64_UINT64_TIMET_BOOLEAN_POINTER)notifier->callback)(arg1, arg2, arg3, arg4, arg5, notifier->user_data);

    list = list->next;
  }
}

void
connui_utils_notify_notify(GSList *list, gpointer first_arg, ...)
{
  typedef void (*cb_1_arg)(gpointer);
  typedef void (*cb_2_arg)(gpointer, gpointer);
  typedef void (*cb_3_arg)(gpointer, gpointer, gpointer);
  typedef void (*cb_4_arg)(gpointer, gpointer, gpointer, gpointer);
  typedef void (*cb_5_arg)(gpointer, gpointer, gpointer, gpointer, gpointer);
  typedef void (*cb_6_arg)(gpointer, gpointer, gpointer, gpointer, gpointer,
                           gpointer);
  typedef void (*cb_7_arg)(gpointer, gpointer, gpointer, gpointer, gpointer,
                           gpointer, gpointer);

  GSList *copy = NULL;
  GSList *args = NULL;
  GSList *l;
  va_list ap;

  va_start(ap, first_arg);

  while (first_arg)
  {
    args = g_slist_append(args, first_arg);
    first_arg = va_arg(ap, gpointer);
  }

  va_end(ap);

  /* we must create copy and use it for callback calls, as callback may modify
   * the original */
  for (l = list; l; l = l->next)
  {
    connui_notifier *n = (connui_notifier *)l->data;

    copy = connui_utils_notify_add(copy, n->callback, n->user_data);
  }

  l = copy;

  while (l)
  {
    connui_notifier *n = (connui_notifier *)l->data;

    if (n && n->callback)
    {
      switch (g_slist_length(args))
      {
        case 0:
          ((cb_1_arg)n->callback)(n->user_data);
          break;
        case 1:
          ((cb_2_arg)n->callback)(
                args->data,
                n->user_data);
          break;
        case 2:
          ((cb_3_arg)n->callback)(
                args->data,
                args->next->data,
                n->user_data);
          break;
        case 3:
          ((cb_4_arg)n->callback)(
                args->data,
                args->next->data,
                args->next->next->data,
                n->user_data);
          break;
        case 4:
          ((cb_5_arg)n->callback)(
                args->data,
                args->next->data,
                args->next->next->data,
                args->next->next->next->data,
                n->user_data);
          break;
        case 5:
          ((cb_6_arg)n->callback)(
                args->data,
                args->next->data,
                args->next->next->data,
                args->next->next->next->data,
                args->next->next->next->next->data,
                n->user_data);
          break;
        case 6:
          ((cb_7_arg)n->callback)(
                args->data,
                args->next->data,
                args->next->next->data,
                args->next->next->next->data,
                args->next->next->next->next->data,
                args->next->next->next->next->next->data,
                n->user_data);
          break;
        default:
          CONNUI_ERR("Unable to call user callback, as it has %d parameters.",
                     g_slist_length(args));
          break;
      }
    }

    l = l->next;
  }

  g_slist_free(args);

  while (copy)
  {
    connui_notifier *notifier = (connui_notifier *)copy->data;

    copy = connui_utils_notify_remove(copy, notifier->callback);
  }
}
