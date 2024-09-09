#include "config.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <hildon/hildon.h>
#include <dbus/dbus.h>
#include <libosso.h>

#include <locale.h>
#include <libintl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "connui-dbus.h"
#include "connui-devicelock.h"
#include "connui-flightmode.h"
#include "connui-log.h"
#include "connui-utils.h"

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
  gboolean no_retry_dlg;
  iap_dialogs_match_fn match;
  iap_dialogs_show_fn show;
  iap_dialogs_cancel_fn cancel;
  GModule *module;
};
typedef struct _plugin_info_s plugin_info_s;

struct _service_s
{
  gchar *path;
  gchar *service;
  unsigned int refcnt;
};

typedef struct _service_s service_s;

static gboolean iap_dialog_pending_requests_cb(dialog_info_s *dialog_info);

static dialog_info_s *iap_dialog_info = NULL;

static dialog_info_s *
iap_dialog_get_info()
{
  const char* lib_path = CONNDLGS_PLUGIN_DIR;
  plugin_info_s *plugin_info;
  GDir *dir;

  if (iap_dialog_info)
    return iap_dialog_info;

  iap_dialog_info = g_new0(dialog_info_s, 1);

  iap_dialog_info->osso_context =
      osso_initialize("osso_connectivity_ui_conndlgs", VERSION, TRUE, NULL);
  iap_dialog_info->message_hash =
      g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                            (GDestroyNotify)dbus_connection_unref);

  dir = g_dir_open(lib_path, 0, NULL);

  if (!dir)
  {
    CONNUI_ERR("Unable to open plugin directory: %s", lib_path);
    return iap_dialog_info;
  }

  while (1)
  {
    gchar *path;
    GModule *plugin;
    const gchar *module_name = g_dir_read_name(dir);

    if (!module_name)
      break;

    path = g_module_build_path(lib_path, module_name);
    plugin = g_module_open(path, G_MODULE_BIND_LOCAL);

    if (plugin)
    {
      gpointer symbol;

      if (g_module_symbol(plugin, "iap_dialogs_plugin_match", &symbol))
      {
        plugin_info = g_new0(plugin_info_s, 1);
        plugin_info->match = (iap_dialogs_match_fn)symbol;
        plugin_info->module = plugin;
        plugin_info->no_retry_dlg = FALSE;

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
      plugin->no_retry_dlg = FALSE;
      g_hash_table_remove(info->message_hash, message);
      dbus_message_unref(message);
      pending->data = NULL;
      g_slist_free_1(pending);
    }
    else
      CONNUI_ERR("element '%p' in the pending list contains no data", pending);

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

static
void iap_dialog_showing()
{
  iap_dialog_get_info()->showing = TRUE;
}

static void
iap_dialog_done(int iap_id, gboolean process_pending)
{
  dialog_info_s *dialog_info = iap_dialog_get_info();
  plugin_info_s *plugin;

  g_return_if_fail(dialog_info != NULL);

  dialog_info->showing = FALSE;

  if (iap_id >= 0 && iap_id < g_slist_length(dialog_info->plugins))
  {
    plugin = (plugin_info_s *)g_slist_nth_data(dialog_info->plugins, iap_id);

    if (plugin)
      plugin->no_retry_dlg = 0;
  }

  if (!process_pending)
  {
    if (!dialog_info->pending)
      return;
  }
  else
    iap_dialog_cleanup(dialog_info);

  if (dialog_info->pending)
    g_idle_add((GSourceFunc)iap_dialog_pending_requests_cb, dialog_info);
}

static gboolean
iap_dialog_pending_requests_cb(dialog_info_s *dialog_info)
{
  GSList *pending;
  DBusMessage *message;

  g_return_val_if_fail(dialog_info != NULL, FALSE);

  pending = dialog_info->pending;

  if (!pending )
  {
    CONNUI_ERR("pending list called, but list NULL");
    return FALSE;
  }

  if (dialog_info->showing)
    return FALSE;

  message = (DBusMessage *)pending->data;

  if (message)
  {
    plugin_info_s *plugin = NULL;
    int iap_id;

    pending->data = NULL;
    iap_id = iap_dialog_show_requests(message, dialog_info, &plugin);

    if (iap_id == -1)
      CONNUI_ERR("could not show dialog");
    else
    {
      if (plugin->show)
      {
        if (!plugin->show(iap_id, message, iap_dialog_showing, iap_dialog_done,
                          dialog_info->osso_context))
        {
          CONNUI_ERR("Unable to show dialog");
          iap_dialog_done(iap_id, FALSE);
        }
      }
      else
        CONNUI_ERR("show function for dialog %d is NULL", iap_id);
    }

    g_hash_table_remove(dialog_info->message_hash, message);
    dbus_message_unref(message);
  }
  else
    CONNUI_ERR("pending requests list called, but NULL data");

  pending = dialog_info->pending;
  dialog_info->pending = g_slist_remove_link(dialog_info->pending,
                                             dialog_info->pending);
  g_slist_free_1(pending);

  if (dialog_info->pending)
    return TRUE;

  return FALSE;
}

static DBusHandlerResult
iap_dialog_service(DBusConnection *connection, DBusMessage *message,
                   void *user_data)
{
  int iap_id;
  DBusConnection *dbus;
  DBusMessage *reply;
  const char *error_name;
  char *icd_error;
  char *unk;

  dialog_info_s *info = (dialog_info_s *)user_data;
  plugin_info_s *plugin = NULL;

  g_return_val_if_fail(info != NULL, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

  iap_id = iap_dialog_show_requests(message, info, &plugin);

  if (iap_id >= 0 && plugin)
  {
    if ((!dbus_message_get_args(message, NULL,
                                DBUS_TYPE_STRING, &unk,
                                DBUS_TYPE_STRING, &icd_error,
                                DBUS_TYPE_INVALID) ||
         strcmp("com.nokia.icd.error.flight_mode", icd_error)) &&
        info->offline &&
        dbus_message_has_interface(message, "com.nokia.icd_ui"))
    {
      error_name = "com.nokia.icd_ui.error.flight_mode";
      goto error_reply;
    }

    if (!dbus_message_is_method_call(message, "com.nokia.icd_ui",
                                     "show_retry_dlg"))
    {
      plugin->no_retry_dlg = TRUE;
    }

    if (!info->pending)
    {
      connui_utils_reload_theme();
      g_idle_add((GSourceFunc)iap_dialog_pending_requests_cb, info);
    }

    dbus_message_ref(message);
    dbus = dbus_connection_ref(connection);
    g_hash_table_insert(info->message_hash, message, dbus);
    info->pending = g_slist_append(info->pending, message);
  }
  else if (iap_id == -1)
  {
    error_name = "org.freedesktop.DBus.Error.UnknownMethod";
    goto error_reply;
  }

  reply = dbus_message_new_method_return(message);

  if (reply)
    goto send_reply;

  CONNUI_ERR("could not create reply message");

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

error_reply:
  reply = dbus_message_new_error(message, error_name, 0);

  if (!reply)
  {
    CONNUI_ERR("could not create error message");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

send_reply:
  connui_dbus_send_system_msg(reply);
  dbus_message_unref(reply);

  return DBUS_HANDLER_RESULT_HANDLED;
}

gboolean
iap_dialog_register_service(const char *service, const char *path)
{
  dialog_info_s *info;
  GSList *services;
  service_s *srv;

  g_return_val_if_fail(service != NULL && path != NULL, FALSE);

  info = iap_dialog_get_info();
  services = info->services;

  while (services)
  {
    srv = services->data;

    if (!strcmp(srv->service, service) && !strcmp(srv->path, path))
    {
      srv->refcnt++;
      return TRUE;
    }

    services = services->next;
  }

  if (!connui_dbus_register_system_service(path, service, 0, iap_dialog_service,
                                           info))
  {
    CONNUI_ERR("Unable to register service %s, %s", service, path);
    return FALSE;
  }

  srv = g_new0(service_s, 1);
  srv->service = g_strdup(service);
  srv->refcnt = 1;
  srv->path = g_strdup(path);
  info->services = g_slist_prepend(info->services, srv);

  return TRUE;
}

DBusConnection *
iap_dialog_get_connection(DBusMessage *message)
{
  dialog_info_s *dialog_info;

  dialog_info = iap_dialog_get_info();

  g_return_val_if_fail(dialog_info != NULL, NULL);

  return (DBusConnection *)g_hash_table_lookup(dialog_info->message_hash,
                                               message);
}

static int
iap_dialog_request_timeout_cb(guint *data)
{
  *data = 0;

  return FALSE;
}

int
iap_dialog_request_dialog(guint timeout, iap_dialogs_done_fn *done_cb,
                          osso_context_t **osso_context)
{
  dialog_info_s *info;
  guint timeout_id = 0;

  info = iap_dialog_get_info();

  g_return_val_if_fail(info != NULL, -1);

  timeout_id = g_timeout_add(1000 * timeout,
                             (GSourceFunc)iap_dialog_request_timeout_cb,
                             &timeout_id);

  while (info->showing || info->locked)
  {
    if (!timeout_id)
      break;

    g_main_context_iteration(NULL, TRUE);
  }

  if (!timeout_id)
  {
    CONNUI_ERR("Unable to fulfill dialog request");
    return -1;
  }

  g_source_remove(timeout_id);

  if (osso_context)
    *osso_context = info->osso_context;

  if (done_cb)
    *done_cb = iap_dialog_done;

  iap_dialog_showing();

  /* WTF ?!? */
  return 0xFFFF;
}

void
iap_dialog_unregister_service(const char *service, const char *path)
{
  dialog_info_s *  info = iap_dialog_get_info();
  GSList *services;

  g_return_if_fail(service != NULL && path != NULL);

  services = info->services;

  while (services)
  {
    service_s *srv = services->data;

    if (!strcmp(srv->service, service) && !strcmp(srv->path, path))
    {
      srv->refcnt--;

      if (srv->refcnt < 1)
      {
        info->services = g_slist_delete_link(services, services);
        g_free(srv->service);
        g_free(srv->path);
        g_free(srv);
      }

      return;
    }

    services = services->next;
  }

  CONNUI_ERR("Service %s, %s not found", service, path);
}


int
main(int argc, char **argv)
{
  gboolean should_open = TRUE;
  dialog_info_s *info;

  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, "/usr/share/locale");
  textdomain(GETTEXT_PACKAGE);
  hildon_gtk_init(&argc, &argv);

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

  open_log("iap_conndlg " PACKAGE_VERSION, should_open);
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

  gtk_main();

  connui_flightmode_close(
        (connui_flightmode_notify)iap_dialog_request_flightmode_status_cb);
  connui_devicelock_close(
        (connui_devicelock_notify)iap_dialog_request_devicelock_status_cb);
  osso_deinitialize(info->osso_context);
  g_free(info);

  return 0;
}
