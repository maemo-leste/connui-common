#ifndef IAPSCAN_H
#define IAPSCAN_H

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
  IAP_SCAN_LIST_CAN_DISCONNECT = 11
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

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);

typedef struct _connui_wlan_info connui_wlan_info;

typedef void (*iap_scan_cancel_fn)(gpointer user_data);
typedef void (*iap_scan_started_fn)(gpointer user_data);
typedef gboolean (*iap_scan_network_added_fn)(connui_scan_entry *scan_entry, gpointer user_data);
typedef void (*iap_scan_selection_changed_fn)(GtkTreeSelection *selection, gpointer user_data);

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);
GtkWidget *iap_scan_tree_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);
GtkWidget *iap_scan_view_create(GtkWidget *scan_tree_view);
void iap_scan_free_scan_entry(connui_scan_entry *entry);

gboolean iap_scan_icd_scan_start(connui_wlan_info **info, gchar **network_types);

void iap_scan_close();
void iap_scan_stop();

gboolean iap_scan_add_scan_entry(connui_scan_entry *scan_entry, gboolean can_disconnect);

gint iap_scan_default_sort_func(GtkTreeModel *model, GtkTreeIter *iter1, GtkTreeIter *iter2, network_entry *last_used);

gboolean iap_scan_start(int flags, iap_scan_started_fn scan_started_cb,
                        iap_scan_cancel_fn scan_stopped_cb,
                        iap_scan_network_added_fn scan_network_added_cb,
                        GtkWidget *widget, void *unk,
                        iap_scan_selection_changed_fn selection_changed_cb,
                        gpointer user_data);
gboolean
iap_scan_start_for_network_types(gchar **network_types, int flags,
                                 void (*scan_started_cb)(gpointer),
                                 iap_scan_cancel_fn scan_stopped_cb,
                                 iap_scan_network_added_fn scan_network_added_cb,
                                 GtkWidget *widget, void *unk,
                                 void (*selection_changed_cb)(GtkTreeSelection *, gpointer),
                                 gpointer user_data);

#endif // IAPSCAN_H
