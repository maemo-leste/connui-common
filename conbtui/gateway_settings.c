#include <gconf/gconf-client.h>
#include <icd/osso-ic-gconf.h>

gchar *
gateway_settings_get_preferred(void)
{
  GConfClient *gconf = gconf_client_get_default();
  GError *error = NULL;
  gchar *rv;

  gconf_client_clear_cache(gconf);

  rv = gconf_client_get_string(gconf,
                               ICD_GCONF_SETTINGS "/BT/preferred", &error);
  if (error)
    g_error_free(error);

  g_object_unref(gconf);

  return rv;
}

void
gateway_settings_get_device_data(const gchar *device, gchar **name,
                                 gchar **unk1, gchar **unk2, gchar **unk3,
                                 gchar **unk4, gchar **unk5, gchar **unk6,
                                 gchar **unk7, gchar **unk8, gchar **unk9)
{
  g_assert(0);
}
