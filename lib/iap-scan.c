#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <icd/dbus_api.h>

#include <string.h>

#include "connui-cell-renderer-operator.h"
#include "connui-dbus.h"
#include "connui-log.h"
#include "connui-pixbuf-cache.h"

#include "iap-common.h"
#include "iap-network.h"
#include "iap-settings.h"
#include "wlan-common.h"

#include "iap-scan.h"

enum
{
  IAP_SCAN_LIST_IAP_ICON_NAME = 0,
  IAP_SCAN_LIST_SSID = 0,
  IAP_SCAN_LIST_SERVICE_TYPE = 2,
  IAP_SCAN_LIST_SERVICE_ID = 3,
  IAP_SCAN_LIST_SERVICE_TEXT = 4,
  IAP_SCAN_LIST_NETWORK_TYPE = 5,
  IAP_SCAN_LIST_IS_SAVED = 6,
  IAP_SCAN_LIST_SAVED_ICON = 7,
  IAP_SCAN_LIST_STRENGHT_ICON = 8,
  IAP_SCAN_LIST_SECURITY_ICON = 9
};


struct _connui_wlan_info
{
  int flags;
  DBusPendingCall *callback1;
  iap_scan_cancel_fn cancel_cb;
  int callback;
  gpointer user_data;
  int field_14;
  GtkTreeView *tree_view;
  GtkListStore *model;
  GtkBox *box;
  int field_24;
  int selected;
  int field_2C;
  int active_scan_count;
  int field_34;
  int field_38;
  ConnuiPixbufCache *pixbuf_cache;
  GSList *scan_list;
  gchar *preffered_type;
  gchar *preffered_id;
  int network_types_len;
  GSList *some_list;
  DBusPendingCall *pending;
};

struct _connui_scan_entry
{
  network_entry network;
  guint timestamp;
  gchar *service_name;
  gint service_priority;
  gchar *network_name;
  gint network_priority;
  gint signal_strength;
  gchar *station_id;
  GtkTreeIter iterator;
  GSList *list;
};
typedef struct _connui_scan_entry connui_scan_entry;

static void
iap_scan_list_store_set_valist(connui_wlan_info **info,
                               connui_scan_entry *scan_entry, ...)
{
  va_list ap;

  va_start(ap, scan_entry);

  if ((*info)->model)
    gtk_list_store_set_valist((*info)->model, &scan_entry->iterator, ap);

  va_end(ap);
}

static gboolean
iap_scan_is_hidden_wlan(network_entry *network)
{
  g_return_val_if_fail(network != NULL && network->network_type != NULL, FALSE);


  if (network->network_id && *network->network_id)
    return FALSE;

  return !strncmp(network->network_type, "WLAN_INFRA", 10);
}

static gboolean
iap_scan_entry_network_compare(connui_scan_entry *entry1,
                               connui_scan_entry *entry2)
{
  if (iap_scan_is_hidden_wlan(&entry1->network) &&
      iap_scan_is_hidden_wlan(&entry2->network))
  {
    return FALSE;
  }

  return iap_network_entry_compare(&entry1->network, &entry2->network);
}

GtkListStore *
iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data)
{
  GtkListStore *list_store;

  list_store =
      gtk_list_store_new(12,
                         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                         G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                         G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING,
                         G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_BOOLEAN);
  if (sort_func)
  {
    gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(list_store),
                                            sort_func, user_data, NULL);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(list_store), -1,
                                         GTK_SORT_ASCENDING);
  }

  return list_store;
}

gboolean
iap_scan_icd_scan_stop(connui_wlan_info **info)
{
  gboolean rv = FALSE;
  DBusMessage *mcall;

  if ((*info)->active_scan_count <= 0)
    return TRUE;

  mcall = connui_dbus_create_method_call(ICD_DBUS_API_INTERFACE,
                                         ICD_DBUS_API_PATH,
                                         ICD_DBUS_API_INTERFACE,
                                         ICD_DBUS_API_SCAN_CANCEL,
                                         DBUS_TYPE_INVALID);
  if (mcall)
  {
    if (connui_dbus_send_system_mcall(mcall, -1, 0, 0, 0))
      rv = TRUE;
    else
      CONNUI_ERR("could not send message");
  }

  (*info)->active_scan_count--;

  if ((*info)->cancel_cb)
    (*info)->cancel_cb((*info)->user_data);

  if (mcall)
    dbus_message_unref(mcall);

  return rv;
}

static void
iap_scan_update_network_in_list(connui_wlan_info **info,
                                connui_scan_entry *scan_entry)
{
  gchar *iap_icon;
  gchar *service_text;
  const char *strength_icon;

  g_return_if_fail(*info != NULL && scan_entry != NULL &&
      scan_entry->network.network_type != NULL);

  if ((*info)->flags & 1)
  {
    iap_icon = iap_settings_get_iap_icon_name_by_network_and_signal(
          &scan_entry->network, scan_entry->signal_strength);
  }
  else
    iap_icon = iap_settings_get_iap_icon_name_by_network(&scan_entry->network);

  service_text = iap_settings_get_name_by_network(&scan_entry->network,
                                                  scan_entry->service_name,
                                                  scan_entry->network_name);

  strength_icon = wlan_common_get_icon_name_by_strength(
        iap_common_get_signal_by_nw_level(scan_entry->signal_strength));


  if (!strncmp(scan_entry->network.network_type, "WLAN", 4))
  {
    const char *security_icon =
        wlan_common_get_security_icon_name_by_network_attrs(
          scan_entry->network.network_attributes);

    iap_scan_list_store_set_valist(
          info, scan_entry,
          IAP_SCAN_LIST_IAP_ICON_NAME, iap_icon,
          IAP_SCAN_LIST_SERVICE_TEXT, service_text,
          IAP_SCAN_LIST_STRENGHT_ICON, strength_icon,
          IAP_SCAN_LIST_SECURITY_ICON, security_icon,
          -1);
  }
  else
  {
    iap_scan_list_store_set_valist(
          info, scan_entry,
          IAP_SCAN_LIST_IAP_ICON_NAME, iap_icon,
          IAP_SCAN_LIST_SERVICE_TEXT, service_text,
          IAP_SCAN_LIST_STRENGHT_ICON, strength_icon,
          -1);
  }

  g_free(iap_icon);
  g_free(service_text);
}

GtkWidget *
iap_scan_tree_create(GtkTreeIterCompareFunc sort_func, gpointer*user_data)
{
  GtkListStore *scan_store;
  GtkWidget *tree_view;
  GtkTreeViewColumn *tree_column;
  GtkCellRenderer *renderer;

  scan_store = iap_scan_store_create(sort_func, user_data);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(scan_store));

  g_object_unref(G_OBJECT(scan_store));

  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);
  gtk_tree_selection_set_mode(
      gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view)),
      GTK_SELECTION_SINGLE);

  tree_column = gtk_tree_view_column_new();
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(tree_column, renderer, FALSE);
  gtk_tree_view_column_set_attributes(tree_column, renderer,
                                      "icon-name", IAP_SCAN_LIST_IAP_ICON_NAME,
                                      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_column);

  tree_column = gtk_tree_view_column_new();
  renderer = connui_cell_renderer_operator_new();
  gtk_tree_view_column_set_sizing(tree_column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_expand(tree_column, TRUE);
  gtk_tree_view_column_pack_start(tree_column, renderer, TRUE);
  gtk_tree_view_column_set_attributes(
        tree_column, renderer,
        "service-type", IAP_SCAN_LIST_SERVICE_TYPE,
        "service-id", IAP_SCAN_LIST_SERVICE_ID,
        "service-text", IAP_SCAN_LIST_SERVICE_TEXT,
        NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_column);

  tree_column = gtk_tree_view_column_new();
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(tree_column, renderer, FALSE);
  gtk_tree_view_column_set_attributes(tree_column, renderer,
                                      "icon-name", IAP_SCAN_LIST_SAVED_ICON,
                                      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_column);

  tree_column = gtk_tree_view_column_new();
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(tree_column, renderer, FALSE);
  gtk_tree_view_column_set_attributes(tree_column, renderer,
                                      "icon-name", IAP_SCAN_LIST_STRENGHT_ICON,
                                      NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_column);

  tree_column = gtk_tree_view_column_new();
  renderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(tree_column, renderer, FALSE);
  gtk_tree_view_column_set_attributes(tree_column, renderer, "icon-name",
                                      IAP_SCAN_LIST_SECURITY_ICON, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_column);

  return tree_view;
}

GtkWidget *
iap_scan_view_create(GtkWidget *scan_tree_view)
{
  GtkWidget *scrolled_window;

  g_return_val_if_fail(GTK_IS_WIDGET(scan_tree_view), NULL);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window),
                                      GTK_SHADOW_NONE);

  if (GTK_IS_TREE_VIEW(scan_tree_view))
    gtk_container_add(GTK_CONTAINER(scrolled_window), scan_tree_view);
  else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window),
                                          scan_tree_view);

  return scrolled_window;
}
