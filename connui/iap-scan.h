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

typedef struct _connui_scan_entry connui_scan_entry;

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);

typedef struct _connui_wlan_info connui_wlan_info;

typedef void (*iap_scan_cancel_fn)(gpointer user_data);

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);
GtkWidget *iap_scan_tree_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);
GtkWidget *iap_scan_view_create(GtkWidget *scan_tree_view);
void iap_scan_free_scan_entry(connui_scan_entry *entry);

gboolean iap_scan_icd_scan_start(connui_wlan_info **info, gchar **network_types);

void iap_scan_close();
void iap_scan_stop();

gboolean iap_scan_add_scan_entry(connui_scan_entry *scan_entry, gboolean can_disconnect);

gint iap_scan_default_sort_func(GtkTreeModel *model, GtkTreeIter *iter1, GtkTreeIter *iter2, network_entry *last_used);
#endif // IAPSCAN_H
