#ifndef WLAN_COMMON_H
#define WLAN_COMMON_H

#define WLAN_CAP_WLAN 0x1
#define WLAN_CAP_ADHOCWLAN 0x2
#define wlan_capability_4 0x4
#define wlan_capability_8 0x8
#define WLAN_CAP_SECURITY_NONE 0x10
#define WLAN_CAP_SECURITY_WEP 0x20
#define WLAN_CAP_SECURITY_WPA_PSK 0x40
#define WLAN_CAP_SECURITY_WPA_EAP 0x80

typedef enum
{
  STRENGTH_NONE,
  STRENGTH_WEAK,
  STRENGTH_FAIR,
  STRENGTH_GOOD,
  STRENGTH_EXELLENT
} wlan_signal_strength_e;


gboolean wlan_common_mangle_ssid(gchar *ssid, size_t len);
int wlan_common_gconf_wlan_security_to_capability(const gchar *security);
const char *wlan_common_get_icon_name_by_saved(gboolean saved);
const char *wlan_common_get_iaptype_icon_name_by_capability(int capability);
const char *wlan_common_get_icon_name_by_strength(wlan_signal_strength_e strength);
const char *wlan_common_get_saved_icon_name_by_network(network_entry *entry);
wlan_signal_strength_e wlan_common_get_signal_by_rssi(int rssi);
const char *wlan_common_get_security_icon_name_by_network_attrs(unsigned int attrs);

#endif // WLAN_COMMON_H
