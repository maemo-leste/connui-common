#include <glib.h>

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
