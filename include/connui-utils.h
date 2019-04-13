#ifndef CONNUI_UTILS_H
#define CONNUI_UTILS_H

typedef void (*connui_utils_notify) ();

struct GlobalDataStruct
{
  GSList *list;
  DBusPendingCall *call;
};

GSList *connui_utils_find_callback(GSList *list,connui_utils_notify notify);
osso_context_t *connui_utils_inherit_osso_context(osso_context_t *lib_osso, const gchar *application, const gchar *version);
void connui_utils_reload_theme();
void connui_utils_unblank_display();
GSList *connui_utils_notify_remove(GSList *list, connui_utils_notify callback);
GSList *connui_utils_notify_add(GSList *list, connui_utils_notify callback, gpointer user_data);

void connui_utils_notify_notify_POINTER(GSList *list, gpointer arg1);
void connui_utils_notify_notify_POINTER_POINTER(GSList *list, gpointer arg1, gpointer arg2);
void connui_utils_notify_notify_POINTER_POINTER_POINTER(GSList *list, gpointer arg1, gpointer arg2, gpointer arg3);
void connui_utils_notify_notify_INT_POINTER_POINTER_POINTER_POINTER(GSList *list, gint arg1, gpointer arg2, gpointer arg3, gpointer arg4, gpointer arg5);
void connui_utils_notify_notify_BOOLEAN(GSList *list, gboolean arg1);
void connui_utils_notify_notify_INT_POINTER(GSList *list, int arg1, gpointer arg2);
void connui_utils_notify_notify_BOOLEAN_UINT(GSList *list, gboolean arg1, guint arg2);
void connui_utils_notify_notify_UINT(GSList *list, guint arg1);
void connui_utils_notify_notify_UINT64_UINT64_UINT_BOOLEAN_POINTER(GSList *list, guint64 arg1, guint64 arg2, guint arg3, gboolean arg4, gpointer arg5);

void open_log(const char *ident, gboolean open);

#endif
