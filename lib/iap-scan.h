#ifndef IAPSCAN_H
#define IAPSCAN_H

typedef struct _connui_scan_entry connui_scan_entry;

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);

typedef struct _connui_wlan_info connui_wlan_info;

typedef void (*iap_scan_cancel_fn)(gpointer user_data);

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);
GtkWidget *iap_scan_tree_create(GtkTreeIterCompareFunc sort_func, gpointer*user_data);
GtkWidget *iap_scan_view_create(GtkWidget *scan_tree_view);
void iap_scan_free_scan_entry(connui_scan_entry *entry);

gboolean iap_scan_icd_scan_start(connui_wlan_info **info, gchar **network_types);

void iap_scan_close();
void iap_scan_stop();

gboolean iap_scan_add_scan_entry(connui_scan_entry *scan_entry, gboolean unk);

#endif // IAPSCAN_H
