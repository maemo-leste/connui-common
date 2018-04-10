#include <glib.h>

#include <string.h>

#include "iap-network.h"
#include "wlan-common.h"

gboolean
wlan_common_mangle_ssid(gchar *ssid, size_t len)
{
  gboolean rv = TRUE;
  int i;

  for (i = 0; i < len; i++)
  {
    if (!g_ascii_isprint(ssid[i]))
    {
      ssid[i] = '?';
      rv = FALSE;
    }
  }

  return rv;
}

int
wlan_common_gconf_wlan_security_to_capability(const gchar *security)
{
  if (!security)
    return 0;

  if (!strncmp(security, "NONE", 4))
    return WLAN_CAP_SECURITY_NONE;
  else if (!strncmp(security, "WEP", 3))
    return WLAN_CAP_SECURITY_WEP;
  else if (!strncmp(security, "WPA_PSK", 7))
    return WLAN_CAP_SECURITY_WPA_PSK;
  else if (!strncmp(security, "WPA_EAP", 7))
    return WLAN_CAP_SECURITY_WPA_EAP;

  return 0;
}

const char *
wlan_common_get_icon_name_by_saved(gboolean saved)
{
  if (saved)
    return "connectivity_wlan_saved";

  return NULL;
}

const char *
wlan_common_get_iaptype_icon_name_by_capability(guint capability)
{
  if (capability & WLAN_CAP_WLAN)
    return "connect_manager_wlan";

  if(capability & WLAN_CAP_ADHOCWLAN)
    return "connect_manager_adhocwlan";

  return NULL;
}

const char *
wlan_common_get_icon_name_by_strength(wlan_signal_strength_e strength)
{
  switch (strength)
  {
    case STRENGTH_WEAK:
      return "qgn_indi_connectivity_wlan_signal_strenght_weak";
    case STRENGTH_FAIR:
      return "qgn_indi_connectivity_wlan_signal_strenght_fair";
    case STRENGTH_GOOD:
      return "qgn_indi_connectivity_wlan_signal_strenght_good";
    case STRENGTH_EXELLENT:
      return "qgn_indi_connectivity_wlan_signal_strenght_exellent";
      break;
    default:
      break;
  }

  return NULL;
}

const char *
wlan_common_get_saved_icon_name_by_network(network_entry *entry)
{
  g_return_val_if_fail(entry != NULL, NULL);

  if (!entry->network_type || strncmp(entry->network_type, "WLAN_", 5))
    return NULL;

  return wlan_common_get_icon_name_by_saved(iap_network_entry_is_saved(entry));
}

wlan_signal_strength_e
wlan_common_get_signal_by_rssi(int rssi)
{
  if (rssi >= -60)
    return STRENGTH_EXELLENT;
  else if (rssi >= -70)
    return STRENGTH_GOOD;
  else if (rssi >= -80)
    return STRENGTH_FAIR;
  else
    return STRENGTH_WEAK;
}

/* FIXME - use defines */
const char *
wlan_common_get_security_icon_name_by_network_attrs(unsigned int attrs)
{
  unsigned int attrs_masked;

  attrs_masked = ((attrs & 0xF000) >> 3) | ((attrs & 0x20000000) >> 26) |
      (attrs & 7) | ((attrs & 0xF00) << 20) | 2 * (attrs & 0x80) |
      2 * (attrs & 0x78);

  if (attrs_masked & 0xC0)
    return "connectivity_wlan_securitylevel3";
  else if (attrs_masked & 0x20)
    return "general_locked";
  else
    return "general_unlock";
}
