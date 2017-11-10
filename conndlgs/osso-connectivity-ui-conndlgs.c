#include <glib.h>
#include <glib/gprintf.h>
#include <gconf/gconf-client.h>
#include <hildon/hildon.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <libosso.h>

#include <locale.h>
#include <libintl.h>
#include <unistd.h>
#include <stdlib.h>

#include "connui-dbus.h"
#include "connui-log.h"
#include "connui-utils.h"
#include "connui-devicelock.h"
#include "connui-flightmode.h"

#include "connui-conndlgs.h"

struct _dialog_info_s
{
  gboolean showing;
  gboolean offline;
  gboolean locked;
  int unused1;
  osso_context_t *osso_context;
  GSList *pending;
  GSList *plugins;
  GSList *services;
  int unused2;
  GHashTable *message_hash;
};
typedef struct _dialog_info_s dialog_info_s;

struct _plugin_info_s
{
  gboolean char0;
  iap_dialogs_match_fn match;
  iap_dialogs_show_fn show;
  iap_dialogs_cancel_fn cancel;
  GModule *module;
};
typedef struct _plugin_info_s plugin_info_s;

static dialog_info_s *iap_dialog_info = NULL;

static dialog_info_s *
iap_dialog_get_info()
{
  plugin_info_s *plugin_info;
  GDir *dir;

  if (iap_dialog_info)
    return iap_dialog_info;

  iap_dialog_info = g_new0(dialog_info_s, 1);

  iap_dialog_info->osso_context =
      osso_initialize("osso_connectivity_ui_conndlgs", "2.88+0m5", TRUE, NULL);
  iap_dialog_info->message_hash =
      g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                            (GDestroyNotify)dbus_connection_unref);
  dir = g_dir_open("/usr/lib/conndlgs", 0, NULL);

  if (!dir)
  {
    CONNUI_ERR("Unable to open plugin directory: %s", "/usr/lib/conndlgs");
    return iap_dialog_info;
  }

  while (1)
  {
    gchar *path;
    GModule *plugin;
    const gchar *module_name = g_dir_read_name(dir);

    if (!module_name)
      break;

    path = g_module_build_path("/usr/lib/conndlgs", module_name);
    plugin = g_module_open(path, G_MODULE_BIND_LOCAL);

    if (plugin)
    {
      gpointer symbol;

      if (g_module_symbol(plugin, "iap_dialogs_plugin_match", &symbol))
      {
        plugin_info = g_new0(plugin_info_s, 1);
        plugin_info->match = (iap_dialogs_match_fn)symbol;
        plugin_info->module = plugin;
        plugin_info->char0 = FALSE;

        if (g_module_symbol(plugin, "iap_dialogs_plugin_show", &symbol) )
          plugin_info->show = (iap_dialogs_show_fn)symbol;

        if (g_module_symbol(plugin, "iap_dialogs_plugin_cancel", &symbol))
          plugin_info->cancel = (iap_dialogs_cancel_fn)symbol;

        iap_dialog_info->plugins =
            g_slist_append(iap_dialog_info->plugins, plugin_info);
      }
      else
      {
        CONNUI_ERR("Incompatible iap_dialog module %s", module_name);
        g_module_close(plugin);
      }
    }
    else
      CONNUI_ERR("Unable to load iap_dialog module %s: %s", module_name,
                 g_module_error());

    g_free(path);
  }

  g_dir_close(dir);

  return iap_dialog_info;
}

static int
iap_dialog_show_requests(DBusMessage *message, dialog_info_s *dialog_info,
                         plugin_info_s **plugin)
{
  GSList *plugins = dialog_info->plugins;
  plugin_info_s *candidate;
  int iap_id = 0;

  if (plugin)
    *plugin = NULL;

  if (!plugins)
    return -1;

  while (1)
  {
    candidate = (plugin_info_s *)plugins->data;

    if (candidate && candidate->match && candidate->match(message))
      break;

    plugins = plugins->next;
    iap_id++;

    if (!plugins)
      return -1;
  }

  if (plugin)
    *plugin = candidate;

  return iap_id;
}

static void
iap_dialog_cleanup(dialog_info_s *info)
{
  GSList *pending = info->pending;
  GSList *next;
  plugin_info_s *plugin = NULL;

  if (!pending)
    return;

  while (1)
  {
    DBusMessage *message = (DBusMessage *)pending->data;
    next = pending->next;

    if (pending->data &&
        dbus_message_has_interface(message, "com.nokia.icd_ui") &&
        (iap_dialog_show_requests(message, info, &plugin) >= 0) &&
        plugin && plugin->cancel)
    {
      info->pending = g_slist_remove_link(info->pending, pending);
      plugin->cancel(message);
      plugin->char0 = FALSE;
      g_hash_table_remove(info->message_hash, message);
      dbus_message_unref(message);
      pending->data = NULL;
      g_slist_free_1(pending);
    }
    else
      syslog(11, "element '%p' in the pending list contains no data", pending);

    pending = next;

    if (!next)
      break;
  }
}

static void
iap_dialog_request_flightmode_status_cb(dbus_bool_t offline,
                                        dialog_info_s *info)
{
  info->offline = offline;

  if (offline == TRUE)
    iap_dialog_cleanup(info);
}

static void
iap_dialog_request_devicelock_status_cb(dbus_bool_t locked, dialog_info_s *info)
{
  info->locked = locked;
}

static gboolean
show_error_note(const gchar *msg)
{
  DBusMessage *mcall;

  mcall = connui_dbus_create_method_call("com.nokia.cellular_ui",
                                         "/com/nokia/cellular_ui",
                                         "com.nokia.cellular_ui",
                                         "show_error_note",
                                         DBUS_TYPE_INVALID);
  if (mcall)
  {
    if (dbus_message_append_args(mcall, 's', &msg, NULL))
      connui_dbus_send_system_mcall(mcall, -1, NULL, NULL, NULL);
    else
      CONNUI_ERR("Could not append args to show error note method call");

    dbus_message_unref(mcall);
  }
  else
  {
    CONNUI_ERR("could not create show error note method call");
  }

  return FALSE;
}

static void
get_registration_status_cb(DBusPendingCall *pending, void *user_data)
{
  DBusMessage *message;
  GConfClient *gconf;
  DBusError error;
  dbus_int32_t error_value;
  dbus_uint32_t country_code;
  dbus_uint32_t operator_code;
  dbus_uint32_t cell_id;
  dbus_uint16_t lac;
  unsigned char supported_services;
  unsigned char network_type;
  unsigned char status;

  message = dbus_pending_call_steal_reply(pending);
  dbus_error_init(&error);

  if (dbus_set_error_from_message(&error, message))
  {
    CONNUI_ERR("Unable to get registration status: %s, %s", error.name,
               error.message);
    dbus_error_free(&error);
    dbus_message_unref(message);
    return;
  }

  dbus_error_init(&error);

  if (dbus_message_get_args(message, NULL,
                            DBUS_TYPE_BYTE, &status,
                            DBUS_TYPE_UINT16, &lac,
                            DBUS_TYPE_UINT32, &cell_id,
                            DBUS_TYPE_UINT32, &operator_code,
                            DBUS_TYPE_UINT32, &country_code,
                            DBUS_TYPE_BYTE, &network_type,
                            DBUS_TYPE_BYTE, &supported_services,
                            DBUS_TYPE_INT32, &error_value,
                            DBUS_TYPE_INVALID))
  {
    if (error_value)
    {
      CONNUI_ERR("get_registration_status failed with code %d", error_value);
      goto out;
    }
  }
  else
  {
    CONNUI_ERR("Unable to parse registration_status_change reply: %s, %s",
               error.name, error.message);
    dbus_error_free(&error);
    goto out;
  }

  gconf = gconf_client_get_default();

  if (gconf)
  {
    if (!gconf_client_get_bool(
          gconf,
          "/system/osso/connectivity/ui/gprs_data_warning_home_acknowledged",
          NULL) && !status)
    {
      g_idle_add((GSourceFunc)show_error_note, "home_notification");
    }

    if (!gconf_client_get_bool(
          gconf,
          "/system/osso/connectivity/ui/gprs_data_warning_roaming_acknowledged",
          NULL) && status)
    {
      g_idle_add((GSourceFunc)show_error_note, "roaming_notification");
    }

    g_object_unref(G_OBJECT(gconf));
  }

out:
  dbus_message_unref(message);
}

int
main(int argc, char *const *argv)
{
  gboolean should_open = TRUE;
  dialog_info_s *info;
  DBusConnection *dbus;
  DBusMessage *mcall;
  int user_data;

  if ( !g_threads_got_initialized )
    g_thread_init((GThreadFunctions *)g_threads_got_initialized);

  setlocale(LC_ALL, "");
  bindtextdomain("osso-connectivity-ui", "/usr/share/locale");
  textdomain("osso-connectivity-ui");
  hildon_gtk_init(&argc, (char ***)&argv);
  dbus_g_thread_init();

  should_open = 1;
  while (1)
  {
    int opt = getopt(argc, argv, "s");

    if (opt == -1)
      break;

    if (opt != 's')
    {
      g_printf("Usage: %s [-s]\n-s\tLog to standard out.\n", argv[0]);
      exit(1);
    }
    else
      should_open = FALSE;
  }

  open_log("iap_conndlg 2.88+0m5", should_open);
  info = iap_dialog_get_info();

  if (!connui_flightmode_status(
        (connui_flightmode_notify)iap_dialog_request_flightmode_status_cb,
        info))
  {
    CONNUI_ERR("Unable to register flightmode callback");
  }

  if (!connui_devicelock_status(
        (connui_devicelock_notify)iap_dialog_request_devicelock_status_cb,
        info))
  {
    CONNUI_ERR("Unable to register devicelock callback");
  }

  dbus = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
  user_data = 0;

  if (dbus)
  {
    mcall = dbus_message_new_method_call("com.nokia.phone.net",
                                         "/com/nokia/phone/net",
                                         "Phone.Net",
                                         "get_registration_status");
  }
  else
  {
    CONNUI_ERR("Could not get dbus connection");
    mcall = NULL;
  }

  if (mcall)
  {
    DBusPendingCall *pending;

    if (dbus_connection_send_with_reply(dbus, mcall, &pending, -1))
    {
      dbus_pending_call_set_notify(pending, get_registration_status_cb,
                                   &user_data, NULL);
      dbus_pending_call_unref(pending);
    }
    else
      CONNUI_ERR("Sending get_registration_status failed");

    dbus_message_unref(mcall);
  }
  else
    CONNUI_ERR("Unable to allocate new D-Bus get_registration_status message");


  gtk_main();

  connui_flightmode_close(
        (connui_flightmode_notify)iap_dialog_request_flightmode_status_cb);
  connui_devicelock_close(
        (connui_devicelock_notify)iap_dialog_request_devicelock_status_cb);
  osso_deinitialize(info->osso_context);
  g_free(info);

  return 0;
}
