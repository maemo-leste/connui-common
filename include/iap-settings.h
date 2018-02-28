#ifndef IAPSETTINGS_H
#define IAPSETTINGS_H

#include <gconf/gconf-client.h>

typedef enum
{
  INVALID = 0,
  DUN_GSM_CS = 0x1,
  DUN_GSM_PS = 0x2,
  DUN_CDMA_CSD = 0x4,
  DUN_CDMA_QNC = 0x8,
  iap_type_10 = 0x10,
  DUN_CDMA_PSD = 0x20,
  WLAN_ADHOC = 0x40,
  WLAN_INFRA = 0x80,
  WIMAX = 0x100,
  UNKNOWN = 0x1000
} iap_type;

gchar *iap_settings_create_iap_id();
const char *iap_settings_enum_to_gconf_type(iap_type type);
iap_type iap_settings_gconf_type_to_enum(const char *type);
gchar *iap_settings_get_auto_connect();
gboolean iap_settings_iap_is_easywlan(const gchar *iap_name);
gboolean iap_settings_remove_iap(const gchar *iap_name);
GConfValue *iap_settings_get_gconf_value(const gchar *iap, const gchar *key);
gchar *iap_settings_get_name(const gchar *iap);
gchar *iap_settings_get_wlan_ssid(const gchar *iap);
gchar *iap_settings_get_name_by_network(network_entry *entry, const gchar *name1, const gchar *name2);
gchar *iap_settings_get_iap_icon_name_by_network(network_entry *entry);
gchar *iap_settings_get_iap_icon_name_by_id(const gchar *iap);
gchar *iap_settings_get_iap_icon_name_by_type(const gchar *network_type, const gchar *service_type, const gchar *service_id);
gchar *iap_settings_get_iap_icon_name_by_network_and_signal(network_entry *entry, int signal);
gchar *iap_settings_get_iap_type(const gchar *iap);
gboolean iap_settings_is_iap_visible(const gchar *iap);
gint iap_settings_get_search_interval();
gint iap_settings_wlan_txpower_get();
gboolean iap_settings_is_empty(const gchar *iap);
gboolean iap_settings_iap_exists(const gchar *iap_name, const gchar *iap);
gboolean iap_settings_set_gconf_value(const gchar *iap, const gchar *key, const GConfValue *value);
gboolean iap_settings_get_iap_list(GtkListStore *dun_iaps, const gchar **which_types,
                                   int type_column, int id_column, int name_column,
                                   int pixbuf_column, int service_type_column,
                                   int service_id_column);
#endif // IAPSETTINGS_H
