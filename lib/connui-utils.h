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
GSList *connui_utils_notify_remove(GSList *list,connui_utils_notify notify);
GSList *connui_utils_notify_add(GSList *list,connui_utils_notify notify,gpointer user_data);
void connui_utils_notify_notify(GSList *list,gpointer first_arg,...);

#endif
