#include <gconf/gconf-client.h>
#include <icd/osso-ic-gconf.h>
#include <hildon/hildon.h>

#include <string.h>
#include <ctype.h>
#include <time.h>

#include "connui-dbus.h"
#include "connui-log.h"
#include "connui-pixbuf-cache.h"
#include "iap-common.h"
#include "iap-settings.h"
#include "wlan-common.h"

#include "org.ofono.Manager.h"
#include "org.ofono.SimManager.h"

#include "intl.h"

gchar *
iap_settings_create_iap_id()
{
  GConfClient *gconf;
  GError *error = NULL;
  gchar *uuid = NULL;

  if (!(gconf = gconf_client_get_default()))
  {
    CONNUI_ERR("Unable to get GConfClient!");
    return NULL;
  }

  while (1)
  {
    gboolean exists;
    gchar *s;
    gchar *client_dir;

    g_free(uuid);

    if (!g_file_get_contents("/proc/sys/kernel/random/uuid", &uuid, NULL,
                             &error) || !uuid)
    {
      if (error)
      {
        CONNUI_ERR("Unable to read file: %s", error->message);
        g_error_free(error);
      }
      else
        CONNUI_ERR("Unable to read file: no error");

      return NULL;
    }

    g_strchomp(g_strchug(uuid));

    s = gconf_escape_key(uuid, -1);
    client_dir = g_strconcat(ICD_GCONF_PATH, "/", s, NULL);
    g_free(s);
    exists = gconf_client_dir_exists(gconf, client_dir, NULL);
    g_free(client_dir);

    if (!exists)
    {
      g_object_unref(gconf);
      return uuid;
    }
  }

  return NULL;
}

const char *
iap_settings_enum_to_gconf_type(iap_type type)
{
  switch (type)
  {
    case DUN_CDMA_PSD:
      return "DUN_CDMA_PSD";
    case DUN_GSM_PS:
      return "DUN_GSM_PS";
    case DUN_CDMA_CSD:
      return "DUN_CDMA_CSD";
    case DUN_CDMA_QNC:
      return "DUN_CDMA_QNC";
    case DUN_GSM_CS:
      return "DUN_GSM_CS";
    case WLAN_INFRA:
      return "WLAN_INFRA";
    case WIMAX:
      return "WIMAX";
    case WLAN_ADHOC:
      return "WLAN_ADHOC";
    default:
      break;
  }

  return NULL;
}

iap_type
iap_settings_gconf_type_to_enum(const char *type)
{
  if (!type || !*type)
    return INVALID;

  if (!strcmp("DUN_GSM_CS", type))
    return DUN_GSM_CS;

  if (!strcmp("DUN_GSM_PS", type))
    return DUN_GSM_PS;

  if (!strcmp("DUN_CDMA_CSD", type))
    return DUN_CDMA_CSD;

  if (!strcmp("DUN_CDMA_QNC", type))
    return DUN_CDMA_QNC;

  if (!strcmp("DUN_CDMA_PSD", type))
    return DUN_CDMA_PSD;

  if (!strcmp("WLAN_ADHOC", type))
    return WLAN_ADHOC;

  if (!strcmp("WLAN_INFRA", type))
    return WLAN_INFRA;

  if (!strcmp("WIMAX", type))
    return WIMAX;

  return UNKNOWN;
}

gchar *
iap_settings_get_auto_connect()
{
  GConfClient *gconf;
  gchar *auto_connect;

  gconf = gconf_client_get_default();
  auto_connect =
      gconf_client_get_string(gconf, ICD_GCONF_PATH "/auto_connect", NULL);
  g_object_unref(gconf);

  return auto_connect;
}

gboolean
iap_settings_iap_is_easywlan(const gchar *iap_name)
{
  if (!iap_name)
    return FALSE;

  return !strncmp(iap_name, "[Easy", 5);
}

gboolean
iap_settings_remove_iap(const gchar *iap_name)
{
  GConfClient *gconf;
  gchar *s;
  gchar *key;

  if (!iap_name)
    return FALSE;

  gconf = gconf_client_get_default();

  if (!gconf)
    return FALSE;

  s = gconf_escape_key(iap_name, -1);
  key = g_strconcat(ICD_GCONF_PATH, "/", s, NULL);
  gconf_client_recursive_unset(
        gconf, key, GCONF_UNSET_INCLUDING_SCHEMA_NAMES, NULL);
  gconf_client_suggest_sync(gconf, NULL);
  g_free(key);
  g_free(s);
  g_object_unref(gconf);

  return TRUE;
}

gboolean
iap_settings_is_iaptype_supported(const gchar *type)
{
  GConfClient *gconf = gconf_client_get_default();
  gchar *dir;
  gboolean supported;
  GError *error = NULL;

  if (!gconf)
  {
    CONNUI_ERR("Unable to get GConf");
    return FALSE;
  }

  dir = g_strdup_printf(
        "%s/%s", "/system/osso/connectivity/network_type", type);
  supported = gconf_client_dir_exists(gconf, dir, &error);

  if (error)
  {
    CONNUI_ERR("%s", error->message);
    g_error_free(error);
  }

  g_free(dir);
  g_object_unref(gconf);

  return supported;
}

static gboolean
iap_settings_is_possibly_gconf_escaped(const gchar *iap)
{
  const gchar *p;

  if (!iap)
    return FALSE;

  for (p = strchr(iap, '@'); p; p = strchr(p + 1, '@'))
  {
    if (strlen(p) < 4)
      break;

    if (p[3] == '@' && g_ascii_isdigit(p[1]) && g_ascii_isdigit(p[2]))
        return TRUE;
  }

  return FALSE;
}

GConfValue *
iap_settings_get_gconf_value(const gchar *iap, const gchar *key)
{
  GConfClient *gconf_client;
  gchar *s;
  gchar *iap_gconf_key;
  GConfValue *value;
  GError *error = NULL;

  if (!iap || !key)
  {
    CONNUI_ERR("%s not specified", iap ? "key" : iap);
    return NULL;
  }

  if (!*iap)
  {
    CONNUI_ERR("IAP ID is empty string");
    return NULL;
  }

  gconf_client = gconf_client_get_default();

  if (!gconf_client)
  {
    CONNUI_ERR("Unable to get GConfClient");
    return NULL;
  }

  if (iap_settings_is_possibly_gconf_escaped(iap))
  {
    iap_gconf_key = g_strdup_printf(ICD_GCONF_PATH "/%s/%s", iap,
                                    key);

    if (gconf_client_dir_exists(gconf_client, iap_gconf_key, NULL))
    {
      if (iap_gconf_key)
        goto read_value;
    }
    else
      g_free(iap_gconf_key);
  }

  s = gconf_escape_key(iap, -1);
  iap_gconf_key = g_strdup_printf(ICD_GCONF_PATH "/%s/%s", s, key);
  g_free(s);

read_value:
  value = gconf_client_get(gconf_client, iap_gconf_key, &error);
  g_free(iap_gconf_key);

  if (error)
  {
    CONNUI_ERR("could not read key %s for iap %s: '%s'", key, iap,
               error->message);
    g_clear_error(&error);
    value = NULL;
  }

  g_object_unref(gconf_client);

  return value;
}

static gchar *
get_iap_name_by_type(const gchar *type)
{
  if (!type)
    return NULL;

  if (!strncmp(type, "WIMAX", 5))
    return g_strdup("Unknown WIMAX network");

  if (strncmp(type, "GPRS", 4))
    return NULL;

  return g_strdup(_("conn_va_placeholder_iap_name"));
}

gchar *
iap_settings_get_name(const gchar *iap)
{
  GConfValue *val;

  if (!iap || !*iap)
    return NULL;

  val = iap_settings_get_gconf_value(iap, "service_type");

  if (val)
  {
    gchar *service_type = g_strdup(gconf_value_get_string(val));
    gchar *name = NULL;

    gconf_value_free(val);
    val = iap_settings_get_gconf_value(iap, "service_id");

    if (val)
    {
      gchar *domainname = NULL;
      gchar *msgid = NULL;
      gchar *service_id = g_strdup(gconf_value_get_string(val));

      gconf_value_free(val);
      iap_common_get_service_properties(service_type, service_id,
                                        "gettext_catalog", &domainname,
                                        "name", &msgid,
                                        NULL);
      if (msgid && domainname)
        name = g_strdup(dgettext(domainname, msgid));

      g_free(msgid);
      g_free(domainname);
      g_free(service_id);
    }

    g_free(service_type);

    if (name)
      return name;
  }

  val = iap_settings_get_gconf_value(iap, "name");

  if (val)
  {

    gchar *name = g_strdup(gconf_value_get_string(val));
    gconf_value_free(val);

    return name;
  }

  val = iap_settings_get_gconf_value(iap, "temporary");

  if (val)
  {
    if (gconf_value_get_bool(val) == 1)
    {
      gchar *name = iap_settings_get_wlan_ssid(iap);

      gconf_value_free(val);

      if (name)
      {
        wlan_common_mangle_ssid(name, strlen(name));
        return name;
      }
    }
    else
      gconf_value_free(val);
  }

  val = iap_settings_get_gconf_value(iap, "type");

  if (val)
  {
    gchar *name = get_iap_name_by_type(gconf_value_get_string(val));

    gconf_value_free(val);

    if (name)
      return name;
  }

  return g_strdup(iap);
}

gchar *
iap_settings_get_wlan_ssid(const gchar *iap)
{
  GConfValue *val = iap_settings_get_gconf_value(iap, "wlan_ssid");
  gchar *ssid = NULL;

  if (!val)
    return NULL;

  if (val->type == GCONF_VALUE_LIST &&
      gconf_value_get_list_type(val) == GCONF_VALUE_INT)
  {
    int i = 0;
    GSList *l = gconf_value_get_list(val);

    ssid = g_new0(gchar, g_slist_length(l) + 1);

    for (l = gconf_value_get_list(val); l; i++, l = l->next)
      ssid[i] = gconf_value_get_int(l->data);
  }
  else if (val->type == GCONF_VALUE_STRING)
    ssid = g_strdup(gconf_value_get_string(val));
  else
    CONNUI_ERR("SSID value for IAP %s in unknown format", iap);

  gconf_value_free(val);

  return ssid;
}

gchar *
iap_settings_get_name_by_network(network_entry *entry, const gchar *name1,
                                 const gchar *name2)
{
  gchar *name;
  GConfValue *val;

  if (!entry)
    return NULL;

  if (entry->service_type && *entry->service_type &&
      entry->service_id &&*entry->service_id)
  {
    gchar *msgid = NULL;
    gchar *domainname = NULL;

    iap_common_get_service_properties(entry->service_type,
                                      entry->service_id,
                                      "gettext_catalog", &domainname,
                                      "name", &msgid,
                                      NULL);
    name = msgid;

    if (msgid)
    {
      if (!domainname)
        return name;

      name = g_strdup(dgettext(domainname, msgid));
    }
    else
    {
      if (!domainname || !name1)
      {
        g_free(msgid);
        g_free(domainname);
try_name1:

        if (name1 && *name1)
          return g_strdup(name1);

        goto try_name2;
      }

      name = g_strdup(dgettext(domainname, name1));
    }

    g_free(msgid);
    g_free(domainname);

    if (name)
      return name;

    goto try_name1;
  }

try_name2:
  if (name2 && *name2)
    return g_strdup(name2);

  if (entry->network_type && !strcmp(entry->network_type, "WLAN_INFRA"))
  {
    if (!entry->network_id || !*entry->network_id)
      return g_strdup(_("conn_fi_hidden_wlan"));
  }

  val = iap_settings_get_gconf_value(entry->network_id, "name");

  if (val)
  {
    name = g_strdup(gconf_value_get_string(val));
    gconf_value_free(val);

    return name;
  }

  if ((name = get_iap_name_by_type(entry->network_type)))
    return name;

  if (entry->network_type && !strncmp(entry->network_type, "WLAN_", 5) &&
      (name = iap_settings_get_wlan_ssid(entry->network_id)))
  {
    wlan_common_mangle_ssid(name, strlen(name));
  }
  else if (iap_settings_is_possibly_gconf_escaped(entry->network_id))
    name = gconf_unescape_key(entry->network_id, -1);
  else
    name = g_strdup(entry->network_id);

  return name;
}

gchar *
iap_settings_get_iap_icon_name_by_network(network_entry *entry)
{
  g_return_val_if_fail(entry != NULL, NULL);

  return iap_settings_get_iap_icon_name_by_type(entry->network_type,
                                                entry->service_type,
                                                entry->service_id);
}

gchar *
iap_settings_get_iap_icon_name_by_type(const gchar *network_type,
                                       const gchar *service_type,
                                       const gchar *service_id)
{
  gchar *icon_name = NULL;
  GConfClient *gconf = gconf_client_get_default();

  g_return_val_if_fail(gconf != NULL, NULL);

  if (service_type && *service_type && service_id && *service_id)
  {
    iap_common_get_service_properties(service_type, service_id,
            "type_icon_name", &icon_name, NULL);
  }

  if (!icon_name && network_type)
  {
    gchar *key = g_strdup_printf("%s/%s/icon_name",
                                 "/system/osso/connectivity/network_type",
                                 network_type);

    icon_name = gconf_client_get_string(gconf, key, NULL);
    g_free(key);
  }

  g_object_unref(gconf);

  return icon_name;
}

gchar *
iap_settings_get_iap_icon_name_by_network_and_signal(network_entry *entry,
                                                     int signal)
{
  GConfClient *gconf;
  gchar *key;
  gchar *icon_name;

  g_return_val_if_fail(entry != NULL, NULL);

  if (entry->service_type && *entry->service_type &&
      entry->service_id && *entry->service_id)
  {
    return iap_settings_get_iap_icon_name_by_network(entry);
  }

  if (!entry->network_type || !*entry->network_type)
    return iap_settings_get_iap_icon_name_by_network(entry);

  gconf = gconf_client_get_default();
  key = g_strdup_printf("%s/%s/icon_name_signal_%d",
                        "/system/osso/connectivity/network_type",
                        entry->network_type,
                        iap_common_get_signal_by_nw_level(signal));

  icon_name = gconf_client_get_string(gconf, key, NULL);
  g_free(key);
  g_object_unref(gconf);

  if (!icon_name)
    return iap_settings_get_iap_icon_name_by_network(entry);

  return icon_name;
}

gchar *
iap_settings_get_iap_type(const gchar *iap)
{
  GConfValue *val;
  gchar *type;

  val = iap_settings_get_gconf_value(iap, "type");

  if (!val)
    return NULL;

  type = g_strdup(gconf_value_get_string(val));
  gconf_value_free(val);

  return type;
}

gboolean
iap_settings_is_iap_visible(const gchar *iap)
{
  gboolean visible;
  GConfValue *val;

  if (!iap || !*iap)
  {
    CONNUI_ERR("IAP ID is NULL or empty");
    return FALSE;
  }

  if (iap_settings_iap_is_easywlan(iap))
    return FALSE;

  val = iap_settings_get_gconf_value(iap, "temporary");

  if (!val)
  {
    val = iap_settings_get_gconf_value(iap, "visible");

    if (!val)
      return TRUE;

    visible = gconf_value_get_bool(val);
  }
  else
    visible = !gconf_value_get_bool(val);

  gconf_value_free(val);

  return visible;
}

gint
iap_settings_get_search_interval()
{
  GConfClient *gconf;
  gint interval;
  GError *error = NULL;

  gconf = gconf_client_get_default();
  interval =
      gconf_client_get_int(gconf, ICD_GCONF_PATH "/search_interval", &error);

  if (error)
  {
    CONNUI_ERR("could not read search_interval [%s]", error->message);
    g_clear_error(&error);
  }

  g_object_unref(gconf);

  return interval;
}

gint
iap_settings_wlan_txpower_get()
{
  GConfClient *gconf;
  gint wlan_tx_power;
  GError *error = NULL;

  gconf = gconf_client_get_default();
  wlan_tx_power =
      gconf_client_get_int(gconf, ICD_GCONF_PATH "/wlan_tx_power", &error);

  if (error)
  {
    CONNUI_ERR("could not read txpower [%s]", error->message);
    g_clear_error(&error);
  }

  if (wlan_tx_power != 4)
    wlan_tx_power = 8;

  g_object_unref(gconf);

  return wlan_tx_power;
}

gchar *
iap_settings_get_iap_icon_name_by_id(const gchar *iap)
{
  gchar *icon_name;
  GConfValue *val;
  gchar *service_id = NULL;
  gchar *service_type = NULL;
  gchar *type;

  if (!iap)
    return NULL;

  val = iap_settings_get_gconf_value(iap, "service_type");

  if (val)
  {
    service_type = g_strdup(gconf_value_get_string(val));
    gconf_value_free(val);

    if (service_type && *service_type)
    {
      val = iap_settings_get_gconf_value(iap, "service_id");

      if (val)
      {
        service_id = g_strdup(gconf_value_get_string(val));
        gconf_value_free(val);
      }
    }
  }

  type = iap_settings_get_iap_type(iap);
  icon_name = iap_settings_get_iap_icon_name_by_type(type, service_type,
                                                     service_id);
  g_free(type);
  g_free(service_type);
  g_free(service_id);

  return icon_name;
}

gboolean
iap_settings_is_empty(const gchar *iap)
{
  size_t len;
  size_t i;

  if (!iap)
    return TRUE;

  len = strlen(iap);

  if (!len)
    return TRUE;

  for (i = 0; i < len; i++)
  {
    if (!isspace(iap[i]))
      return FALSE;
  }

  return TRUE;
}

gboolean
iap_settings_iap_exists(const gchar *iap_name, const gchar *iap)
{
  GConfClient *gconf;
  GSList *dirs;
  GSList *l;
  GError *err = NULL;
  gboolean exists = FALSE;

  g_return_val_if_fail(iap_name != NULL, FALSE);

  if (!(gconf = gconf_client_get_default()))
  {
      CONNUI_ERR("Unable to get GConfClient!");
      return FALSE;
  }

  dirs = gconf_client_all_dirs(gconf, ICD_GCONF_PATH, &err);

  if (err)
  {
    CONNUI_ERR("Unable to get IAP list from GConf: %s", err->message);
    g_clear_error(&err);
    g_object_unref(gconf);
    return FALSE;
  }

  for (l = dirs; l; l= l->next)
  {
    gchar *dir = l->data;
    char *unescaped;
    gchar *key;
    gchar *name;

    if (!dir)
    {
      CONNUI_ERR("gconf is broken, got NULL data from 'gconf_client_all_dirs()'");
      continue;
    }

    key = g_strconcat(dir, "/name", NULL);
    name = gconf_client_get_string(gconf, key, NULL);
    g_free(key);
    unescaped = gconf_unescape_key(g_strrstr(dir, "/") + 1, -1);

    if (!iap || strcmp(unescaped, iap))
    {
      if (name)
        exists = !strcmp(name, iap_name);
      else if (unescaped)
        exists = !strcmp(unescaped, iap_name);
    }

    g_free(unescaped);
    g_free(name);
    g_free(dir);

    if (exists)
      break;
  }

  g_slist_free(dirs);
  g_object_unref(gconf);

  return exists;
}

gboolean
iap_settings_set_gconf_value(const gchar *iap, const gchar *key,
                             const GConfValue *value)
{
  gchar *dir = NULL;
  GConfClient *gconf;
  GError *err = NULL;

  CONNUI_ERR("iap_settings_set_gconf_value: %s not specified",
             iap ? "key" : "iap");

  if (!*iap)
  {
    CONNUI_ERR("iap_settings_set_gconf_value: IAP ID is empty string");
    return FALSE;
  }

  if (!(gconf = gconf_client_get_default()))
  {
    CONNUI_ERR("Unable to get GConfClient");
    return FALSE;
  }

  if (iap_settings_is_possibly_gconf_escaped(iap))
  {
    dir = g_strdup_printf(ICD_GCONF_PATH "/%s/%s", iap, key);

    if (!gconf_client_dir_exists(gconf, dir, NULL))
    {
      g_free(dir);
      dir = NULL;
    }
  }

  if (!dir)
  {
    char *escaped = gconf_escape_key(iap, -1);
    dir = g_strdup_printf(ICD_GCONF_PATH "/%s/%s", escaped, key);
    g_free(escaped);
  }

  if (value)
    gconf_client_set(gconf, dir, value, &err);
  else
    gconf_client_unset(gconf, dir, &err);

  g_free(dir);
  g_object_unref(gconf);

  if (err)
  {
    CONNUI_ERR("could not write key %s for iap %s: '%s'", key, iap,
               err->message);
    g_clear_error(&err);
    return FALSE;
  }

  return TRUE;
}

static gboolean
iap_settings_is_gprs_iap_visible(const gchar *iap)
{
  const gchar *imsi;
  OrgOfonoManager *manager;
  gboolean rv = FALSE;
  GConfValue *val = iap_settings_get_gconf_value(iap, "sim_imsi");
  GError *error = NULL;

  if (val)
  {
    if (val->type == GCONF_VALUE_STRING)
      imsi = gconf_value_get_string(val);
    else
    {
      gconf_value_free(val);
      val = NULL;
    }
  }

  if (!val)
    return rv;

  manager = org_ofono_manager_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
        "org.ofono", "/", NULL, &error);

  if (manager)
  {
    GVariant *modems;

    if (org_ofono_manager_call_get_modems_sync(manager, &modems, NULL, &error))
    {
      GError *error = NULL;
      gchar *path;
      GVariantIter iter;

      g_variant_iter_init(&iter, modems);

      while (!rv && g_variant_iter_loop(&iter, "(&o@a{sv})", &path, NULL))
      {
        OrgOfonoSimManager *sim = org_ofono_sim_manager_proxy_new_for_bus_sync(
              G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
              "org.ofono", path, NULL, &error);

        if (sim)
        {
          GVariant *prop, *v = NULL;
          const gchar *sim_imsi = NULL;

          if (org_ofono_sim_manager_call_get_properties_sync(
                sim, &prop, NULL, NULL))
          {
            v = g_variant_lookup_value(prop, "SubscriberIdentity", NULL);
            g_variant_unref(prop);
          }

          if (v)
          {
            sim_imsi = g_variant_get_string(v, NULL);

            if (!g_strcmp0(sim_imsi, imsi))
              rv = TRUE;

            g_variant_unref(v);
          }

          g_object_unref(sim);
        }
      }

      g_variant_unref(modems);
    }
    else
      CONNUI_ERR("Error getting OFONO modems [%s]", error->message);

    g_object_unref(manager);
  }
  else
    CONNUI_ERR("Error getting OFONO manager [%s]", error->message);

  g_clear_error(&error);
  gconf_value_free(val);

  return rv;
}

gboolean
iap_settings_get_iap_list(GtkListStore *dun_iaps, const gchar **which_types,
                          int type_column, int id_column, int name_column,
                          int pixbuf_column, int service_type_column,
                          int service_id_column)
{
  GConfClient *gconf;
  GSList *dir;
  GSList *dirs;
  ConnuiPixbufCache *cache;
  GtkTreeIter iter;
  GError *error = NULL;

  g_return_val_if_fail(GTK_IS_LIST_STORE(dun_iaps), FALSE);

  gconf = gconf_client_get_default();
  dirs = gconf_client_all_dirs(gconf, ICD_GCONF_PATH, &error);

  if (error)
  {
    CONNUI_ERR("cannot get list of IAPs: %s", error->message);
    g_clear_error(&error);
    g_object_unref(gconf);
    return FALSE;
  }

  cache = connui_pixbuf_cache_new();

  for (dir = dirs; dir; dir = dir->next)
  {
    gchar *iap;
    gchar *type = NULL;

    if (!dir->data)
    {
      CONNUI_ERR(
            "gconf is broken, got NULL data from 'gconf_client_all_dirs()'");
      continue;
    }

    iap = gconf_unescape_key(g_strrstr(dir->data, "/") + 1, -1);

    if (iap && iap_settings_is_iap_visible(iap) &&
        (type = iap_settings_get_iap_type(iap)))
    {
      const char **found = NULL;

      if (which_types)
      {
        for (found = which_types; *found; found++)
        {
          if (!strcmp(type, *found) || !strcmp("*", *found))
            break;
        }
      }

      if ((!which_types || *found) &&
          (strcmp(type, "GPRS") || iap_settings_is_gprs_iap_visible(iap)))
      {
        gtk_list_store_append(GTK_LIST_STORE(dun_iaps), &iter);

        if (type_column != -1)
        {
          gtk_list_store_set(
                GTK_LIST_STORE(dun_iaps), &iter, type_column, type, -1);
        }

        if (id_column != -1)
        {
          gtk_list_store_set(
                GTK_LIST_STORE(dun_iaps), &iter, id_column, iap, -1);
        }

        if (name_column != -1)
        {
          gchar *name = iap_settings_get_name(iap);
          gtk_list_store_set(
                GTK_LIST_STORE(dun_iaps), &iter, name_column, name, -1);
          g_free(name);
        }

        if (service_type_column != -1)
        {
          GConfValue *val = iap_settings_get_gconf_value(iap, "service_type");

          if (val)
          {
            const char *s = gconf_value_get_string(val);

            if (s)
            {
              gtk_list_store_set(GTK_LIST_STORE(dun_iaps), &iter,
                                 service_type_column, s, -1);
              gconf_value_free(val);

              if (service_id_column != -1)
              {
                val = iap_settings_get_gconf_value(iap, "service_id");

                if (val)
                {
                  if ((s = gconf_value_get_string(val)))
                  {
                    gtk_list_store_set(GTK_LIST_STORE(dun_iaps), &iter,
                                       service_id_column, s, -1);
                  }

                  gconf_value_free(val);
                }
              }
            }
            else
              gconf_value_free(val);
          }
        }

        if (pixbuf_column != -1)
        {
          gchar *icon = iap_settings_get_iap_icon_name_by_id(iap);
          gint icon_size = hildon_get_icon_pixel_size(
                gtk_icon_size_from_name("hildon-small"));

          gtk_list_store_set(
                GTK_LIST_STORE(dun_iaps), &iter, pixbuf_column,
                connui_pixbuf_cache_get(cache, icon, icon_size), -1);
          g_free(icon);
        }
      }
    }

    g_free(type);
    g_free(iap);
    g_free(dir->data);
  }

  connui_pixbuf_cache_destroy(cache);
  g_slist_free(dirs);
  g_object_unref(gconf);

  return TRUE;
}
