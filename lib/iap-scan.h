#ifndef IAPSCAN_H
#define IAPSCAN_H

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);

typedef struct _connui_wlan_info connui_wlan_info;

typedef void (*iap_scan_cancel_fn)(gpointer user_data);

GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort_func, gpointer user_data);
GtkWidget *iap_scan_tree_create(GtkTreeIterCompareFunc sort_func, gpointer*user_data);
GtkWidget *iap_scan_view_create(GtkWidget *scan_tree_view);

#endif // IAPSCAN_H
