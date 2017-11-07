#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <icd/dbus_api.h>
#include <libicd-network-wlan-dev.h>

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
  IAP_SCAN_LIST_SSID = 1,
  IAP_SCAN_LIST_SERVICE_TYPE = 2,
  IAP_SCAN_LIST_SERVICE_ID = 3,
  IAP_SCAN_LIST_SERVICE_TEXT = 4,
  IAP_SCAN_LIST_NETWORK_TYPE = 5,
  IAP_SCAN_LIST_IS_SAVED = 6,
  IAP_SCAN_LIST_SAVED_ICON = 7,
  IAP_SCAN_LIST_STRENGHT_ICON = 8,
  IAP_SCAN_LIST_SECURITY_ICON = 9,
  IAP_SCAN_LIST_SCAN_ENTRY = 10,
  IAP_SCAN_LIST_UNKNOWN_BOOL = 11
};


struct _connui_wlan_info
{
  int flags;
  void (*complete_cb)(gpointer);
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
  void (*selection_changed_cb)(GtkTreeSelection *, gpointer);
  int field_38;
  ConnuiPixbufCache *pixbuf_cache;
  GSList *scan_list;
  gchar *preffered_type;
  gchar *preffered_id;
  int network_types_len;
  GSList *network_types;
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

static int
iap_scan_entry_compare(connui_scan_entry *entry1, connui_scan_entry *entry2)
{
  if (!entry1 && entry2)
    return 1;

  if (entry1 && !entry2)
    return -1;

  if (entry1 == entry2)
    return 0;

  if (entry1->service_priority != entry2->service_priority)
  {
    if (entry1->service_priority > entry2->service_priority)
      return -1;
    else
      return 1;
  }

  if (entry1->network_priority != entry2->network_priority)
  {
    if (entry1->network_priority > entry2->network_priority)
      return -1;
    else
      return 1;
  }

  if (entry1->signal_strength != entry2->signal_strength)
  {
    if (entry1->signal_strength > entry2->signal_strength)
      return -1;
    else
      return 1;
  }

  return 0;
}

static GSList *
iap_scan_add_related_result(connui_wlan_info **info,
                            connui_scan_entry *new_scan_entry,
                            gint (*compare_func)(gconstpointer a,
                                                 gconstpointer b))
{
  GSList *scan_list;
  GSList *rv = NULL;

  g_return_val_if_fail(
        info != NULL && *info != NULL && new_scan_entry != NULL, NULL);

  scan_list = (*info)->scan_list;

  if (!scan_list)
    return NULL;

  do
  {
    connui_scan_entry *entry;
    GSList *l = g_slist_find_custom(scan_list, new_scan_entry, compare_func);

    if (!l)
      break;

    entry = (connui_scan_entry *)l->data;

    if ( l->data != new_scan_entry )
    {
      entry->list = g_slist_insert_sorted(entry->list, new_scan_entry,
                                          (GCompareFunc)iap_scan_entry_compare);
      rv = g_slist_insert_sorted(rv, entry,
                                 (GCompareFunc)iap_scan_entry_compare);
    }

    scan_list = l->next;
  }
  while (scan_list);

  return rv;
}

static void
iap_scan_selection_changed(connui_wlan_info **info)
{
  if ((*info)->tree_view && (*info)->selection_changed_cb)
  {
    (*info)->selection_changed_cb(
          gtk_tree_view_get_selection((*info)->tree_view),
          (*info)->user_data);
  }
}

static void
iap_scan_select_first_item(connui_wlan_info **info)
{
  g_return_if_fail(info != NULL && *info != NULL);

  if ((*info)->network_types_len > 0 &&
      (*info)->network_types_len == g_slist_length((*info)->network_types))
  {
    if ((*info)->tree_view)
    {
      GtkTreeIter iter;
      GtkTreeModel *model = gtk_tree_view_get_model((*info)->tree_view);

      if (model && !(*info)->selected &&
          gtk_tree_model_get_iter_first(model, &iter))
      {
        connui_scan_entry *entry = NULL;

        gtk_tree_model_get(model, &iter, IAP_SCAN_LIST_SCAN_ENTRY, &entry, -1);

        if (entry)
        {
          GtkTreePath *path;

          gtk_tree_selection_select_iter(
                gtk_tree_view_get_selection((*info)->tree_view), &iter);

          path = gtk_tree_model_get_path(model, &iter);
          gtk_tree_view_set_cursor((*info)->tree_view, path, 0, 0);
          gtk_tree_path_free(path);

          iap_scan_selection_changed(info);
          gtk_widget_grab_focus(GTK_WIDGET((*info)->tree_view));
        }
      }
    }

    (*info)->field_14 = FALSE;

    if ((*info)->cancel_cb)
    {
      (*info)->cancel_cb((*info)->user_data);
      (*info) = *info;
    }

    g_slist_foreach((*info)->network_types, (GFunc)&g_free, 0);
    g_slist_free((*info)->network_types);

    (*info)->network_types = NULL;
  }
}

static DBusHandlerResult
iap_scan_icd_signal(DBusConnection *connection, DBusMessage *message,
                    connui_wlan_info **info)
{
  connui_scan_entry *scan_entry;
  network_entry network;
  DBusError error;
  dbus_int32_t signal_dB;
  gchar *station_id;
  dbus_int32_t network_priority;
  dbus_int32_t service_priority;
  dbus_int32_t signal_strength;
  gsize network_id_len;
  gchar *network_id;
  dbus_uint32_t network_attributes;
  gchar *network_name;
  gchar *network_type;
  gchar *service_id;
  dbus_uint32_t service_attributes;
  gchar *service_name;
  gchar *service_type;
  dbus_uint32_t timestamp;
  dbus_uint32_t status;

  if (!*info || (*info)->active_scan_count <= 0)
  {
    CONNUI_ERR("no info struct for icd scan signal or no scan ongoing");
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  dbus_error_init(&error);

  if (!dbus_message_is_signal(message,
                              ICD_DBUS_API_INTERFACE,
                              ICD_DBUS_API_SCAN_SIG))
  {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  if (!dbus_message_get_args(
        message, &error, DBUS_TYPE_UINT32, &status,
        DBUS_TYPE_UINT32, &timestamp, DBUS_TYPE_STRING, &service_type,
        DBUS_TYPE_STRING, &service_name, DBUS_TYPE_UINT32, &service_attributes,
        DBUS_TYPE_STRING, &service_id, DBUS_TYPE_INT32, &service_priority,
        DBUS_TYPE_STRING, &network_type, DBUS_TYPE_STRING, &network_name,
        DBUS_TYPE_UINT32, &network_attributes,
        DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &network_id, &network_id_len,
        DBUS_TYPE_INT32, &network_priority, DBUS_TYPE_INT32, &signal_strength,
        DBUS_TYPE_STRING, &station_id, DBUS_TYPE_INT32, &signal_dB,
        DBUS_TYPE_INVALID))

  {
    CONNUI_ERR("could not get arguments from message");
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  network.service_type = service_type;
  network.service_id = service_id;
  network.service_attributes = service_attributes;
  network.network_type = network_type;
  network.network_id = g_strndup(network_id, network_id_len);
  network.network_attributes = network_attributes;

  if (status == ICD_SCAN_COMPLETE ||
      (network.network_id && *network.network_id) ||
      iap_scan_is_hidden_wlan(&network))
  {
    GSList *l = g_slist_find_custom((*info)->scan_list, &network,
                                    (GCompareFunc)iap_network_entry_compare);

    g_free(network.network_id);
    network.network_id = NULL;

    if (l)
      scan_entry = (connui_scan_entry *)l->data;
    else
      scan_entry = NULL;

    if (!(*info)->field_14)
    {
      if (status == ICD_SCAN_COMPLETE)
        goto finish;

      if ((*info)->complete_cb)
        (*info)->complete_cb((*info)->user_data);

      (*info)->field_14 = TRUE;
    }

    if ((status == ICD_SCAN_NEW || status == ICD_SCAN_UPDATE) && !scan_entry)
    {
      scan_entry = g_new0(connui_scan_entry, 1);
      scan_entry->timestamp = timestamp;
      scan_entry->network.service_type = g_strdup(service_type);
      scan_entry->network.service_attributes = service_attributes;
      scan_entry->service_name = g_strdup(service_name);
      scan_entry->service_priority = service_priority;
      scan_entry->network.service_id = g_strdup(service_id);
      scan_entry->network.network_type = g_strdup((const gchar *)network_type);
      scan_entry->network.network_attributes = network_attributes;
      scan_entry->network_name = g_strdup(network_name);
      scan_entry->network_priority = network_priority;
      scan_entry->signal_strength = signal_strength;
      scan_entry->network.network_id = g_strndup(network_id, network_id_len);
      scan_entry->station_id = g_strdup(station_id);

      if (!iap_scan_add_scan_entry(scan_entry, FALSE))
      {
        iap_scan_free_scan_entry(scan_entry);
        scan_entry = NULL;
        goto finish;
      }

      if (scan_entry->network.network_type &&
          !strcmp(scan_entry->network.network_type, "WLAN_INFRA") &&
          !iap_network_entry_is_saved(&scan_entry->network) &&
          !(*info)->tree_view)
      {
        dbus_uint32_t cap = 0;
        nwattr2cap(scan_entry->network.network_attributes, &cap);

        if (cap & WLANCOND_WPS_MASK )
        {
          connui_scan_entry *wps_entry;

          wps_entry = g_new0(connui_scan_entry, 1);
          wps_entry->timestamp = timestamp;
          wps_entry->network.service_type = g_strdup(service_type);
          wps_entry->network.service_attributes = service_attributes;
          wps_entry->service_name = g_strdup(service_name);
          wps_entry->service_priority = service_priority;
          wps_entry->network.service_id = g_strdup(service_id);
          wps_entry->network.network_type = g_strdup(network_type);
          cap2nwattr(cap, &wps_entry->network.network_attributes);
          wps_entry->network_name = g_strdup(network_name);
          wps_entry->network_priority = network_priority;
          wps_entry->signal_strength = signal_strength;
          wps_entry->network.network_id = g_strndup(network_id, network_id_len);
          wps_entry->station_id = g_strdup(station_id);

          if (iap_scan_add_scan_entry(wps_entry, FALSE))
            iap_scan_update_network_in_list(info, wps_entry);
          else
            iap_scan_free_scan_entry(wps_entry);
        }
      }
    }

    if (scan_entry && (status <= (unsigned int)ICD_SCAN_UPDATE))
    {
      if (!scan_entry->service_name ||
          strcmp(service_name, scan_entry->service_name))
      {
        g_free(scan_entry->service_name);
        scan_entry->service_name = g_strdup(service_name);
      }

      if (!scan_entry->network_name ||
          strcmp(network_name, scan_entry->network_name) )
      {
        g_free(scan_entry->network_name);
        scan_entry->network_name = g_strdup(network_name);
      }

      if (!scan_entry->station_id ||
          strcmp(station_id, scan_entry->station_id) )
      {
        g_free(scan_entry->station_id);
        scan_entry->station_id = g_strdup(station_id);
      }

      scan_entry->timestamp = timestamp;
      scan_entry->service_priority = service_priority;
      scan_entry->network_priority = network_priority;
      scan_entry->signal_strength = signal_strength;
      iap_scan_update_network_in_list(info, scan_entry);
    }
    else
    {
finish:

      if (scan_entry && status == ICD_SCAN_EXPIRE)
      {
        scan_entry->signal_strength = 0;
        iap_scan_list_store_set_valist(info, scan_entry,
                                       IAP_SCAN_LIST_UNKNOWN_BOOL, 0,
                                       -1);
        iap_scan_update_network_in_list(info, scan_entry);
      }
      else if (status == ICD_SCAN_COMPLETE)
      {
        if (!g_slist_find_custom((*info)->network_types, network_type,
                                 (GCompareFunc)&strcmp))
        {
          (*info)->network_types = g_slist_prepend((*info)->network_types,
                                                   g_strdup(network_type));
        }

        iap_scan_select_first_item(info);
      }
    }

    /* FIXME field_2C */
    if (!(*info)->field_2C && (*info)->tree_view && (*info)->scan_list &&
        GTK_WIDGET_REALIZED((*info)->tree_view))
    {
      GtkTreePath *path = gtk_tree_path_new_from_string("0");

      gtk_tree_view_scroll_to_cell((*info)->tree_view, path, 0, 1, 0.0, 0.0);
      gtk_tree_path_free(path);
      (*info)->field_2C = FALSE;
      return DBUS_HANDLER_RESULT_HANDLED;
    }
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

void
iap_scan_free_scan_entry(connui_scan_entry *entry)
{
  iap_network_entry_clear(&entry->network);
  g_free(entry->service_name);
  g_free(entry->network_name);
  g_free(entry->station_id);
  g_slist_free(entry->list);
  g_free(entry);
}
