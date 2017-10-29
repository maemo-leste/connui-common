#include <dbus/dbus.h>
#include <gconf/gconf-client.h>
#include <hildon/hildon.h>
#include <libintl.h>
#include <libosso.h>

#include "iap-common.h"

#include "connui-dbus.h"
#include "connui-log.h"

GtkWidget *
iap_common_show_saved_not_found_banner(GtkWidget *widget)
{
  return hildon_banner_show_information(GTK_WIDGET(widget), NULL,
                                        dgettext("osso-connectivity-ui",
                                                 "conn_ib_net_conn_not_found"));
}

void
iap_common_pack_to_hildon_button(GtkWidget *hbutton, GtkWidget *child,
                                 gboolean expand, gboolean fill)
{
  GtkWidget *widget1;
  GtkWidget *widget2;

  g_return_if_fail(hbutton != NULL && child != NULL);

  widget1 = gtk_bin_get_child(GTK_BIN(hbutton));

  if (widget1)
  {
    widget2 = gtk_bin_get_child(GTK_BIN(widget1));

    if (widget2)
      gtk_box_pack_start(GTK_BOX(widget2), child, expand, fill, 0);
  }
}

dbus_bool_t
iap_common_activate_iap(const gchar *iap)
{
  DBusMessage *mcall;
  dbus_bool_t rv;

  if (!iap)
    iap = "";

  mcall = connui_dbus_create_method_call("com.nokia.icd",
                                         "/com/nokia/icd",
                                         "com.nokia.icd",
                                         "activate",
                                         DBUS_TYPE_STRING, &iap,
                                         DBUS_TYPE_INVALID);

  if (mcall)
  {
    rv = connui_dbus_send_system_mcall(mcall, -1, 0, 0, 0);
    dbus_message_unref(mcall);
  }
  else
  {
    CONNUI_ERR("iap_common_activate_iap: could not create message");
    rv = FALSE;
  }

  return rv;
}

void
iap_common_get_last_used_network(network_entry *network)
{
  GConfClient *gconf_client;
  GError *error = NULL;

  g_return_if_fail(network != NULL);

  gconf_client = gconf_client_get_default();

  g_return_if_fail(gconf_client != NULL);

  network->network_type =
      gconf_client_get_string(gconf_client,
                              "/system/osso/connectivity/IAP/last_used_type",
                              &error);
  if (!error)
  {
    network->network_attributes =
        gconf_client_get_int(gconf_client,
                             "/system/osso/connectivity/IAP/last_used_attrs",
                             &error);

    if (!error)
    {
      network->network_id = gconf_client_get_string(
            gconf_client,
            "/system/osso/connectivity/IAP/last_used_network", &error);
    }
  }

  g_object_unref(gconf_client);

  if (error)
  {
    CONNUI_ERR("could not get the last used IAP: '%s'", error->message);
    g_error_free(error);
    iap_network_entry_clear(network);
  }
}

gboolean
iap_common_set_last_used_network(network_entry *entry)
{
  GConfClient *gconf_client;
  GError *error = NULL;

  gconf_client = gconf_client_get_default();

  g_return_val_if_fail(gconf_client != NULL, FALSE);

  if (entry && entry->network_type && entry->network_id)
  {
    gconf_client_set_string(gconf_client,
                            "/system/osso/connectivity/IAP/last_used_type",
                            entry->network_type, &error);
    if (error)
      goto err;

    gconf_client_set_int(gconf_client,
                         "/system/osso/connectivity/IAP/last_used_attrs",
                         entry->network_attributes, &error);

    if (error)
      goto err;

    gconf_client_set_string(gconf_client,
                            "/system/osso/connectivity/IAP/last_used_network",
                            entry->network_id, &error);
  }
  else
  {
    gconf_client_unset(gconf_client,
                       "/system/osso/connectivity/IAP/last_used_type", NULL);
    gconf_client_unset(gconf_client,
                       "/system/osso/connectivity/IAP/last_used_attrs", NULL);
    gconf_client_unset(gconf_client,
                       "/system/osso/connectivity/IAP/last_used_network", NULL);
  }

  if (!error)
  {
    g_object_unref(gconf_client);
    return TRUE;
  }

err:
  CONNUI_ERR("could not set last used iap: '%s'", error->message);
  g_error_free(error);
  g_object_unref(gconf_client);

  return FALSE;
}

gchar *
iap_common_get_service_gconf_path(const gchar *service_type,
                                  const gchar *service_id)
{
  gchar *service_type_escaped;
  gchar *service_id_escaped;
  gchar *gconf_path;

  service_type_escaped = gconf_escape_key(service_type, -1);
  service_id_escaped = gconf_escape_key(service_id, -1);
  gconf_path = g_strconcat("/system/osso/connectivity/srv_provider", "/",
                           service_type_escaped,
                           "/custom_ui/",
                           service_id_escaped,
                           NULL);
  g_free(service_type_escaped);
  g_free(service_id_escaped);

  return gconf_path;
}

static gboolean
delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  gtk_dialog_response(GTK_DIALOG(widget), GPOINTER_TO_INT(user_data));

  return TRUE;
}

void
iap_common_set_close_response(GtkWidget *widget, gint response_id)
{
  g_signal_connect(G_OBJECT(widget), "close", G_CALLBACK(gtk_dialog_response),
                   GINT_TO_POINTER(response_id));
  g_signal_connect(G_OBJECT(widget), "delete-event",
                   G_CALLBACK(delete_event_cb), GINT_TO_POINTER(response_id));
}

gboolean
iap_common_get_preferred_service(gchar **preferred_type, gchar **preferred_id)
{
  GConfClient *gconf_client;
  gchar *_preferred_type = NULL;
  gchar *_preferred_id = NULL;

  gconf_client = gconf_client_get_default();

  if (!gconf_client)
  {
    CONNUI_ERR("Unable to get GConf client!");
    return FALSE;
  }

  _preferred_type = gconf_client_get_string(
        gconf_client, "/system/osso/connectivity/srv_provider/preferred_type",
        NULL);

  if (!_preferred_type)
    goto err;

  _preferred_id = gconf_client_get_string(
        gconf_client, "/system/osso/connectivity/srv_provider/preferred_id",
        NULL);

  if (!_preferred_id)
    goto err;

  g_object_unref(gconf_client);

  if (preferred_id)
    *preferred_id = _preferred_id;
  else
    g_free(_preferred_id);

  if (preferred_type)
    *preferred_type = _preferred_type;
  else
    g_free(_preferred_type);

  return TRUE;

err:
  g_object_unref(gconf_client);
  g_free(_preferred_id);
  g_free(_preferred_type);

  return FALSE;
}

static GtkWidget *
iap_common_make_connection(const gchar *iap, network_entry *entry)
{
  gchar *iap_name;
  GtkWidget *label;
  GtkWidget *image;
  GtkWidget *hbox;

  if (iap)
    iap_name = iap_settings_get_name(iap);
  else if (entry)
    iap_name = iap_settings_get_name_by_network(entry, NULL, iap);
  else
    iap_name = NULL;

  label = gtk_label_new(iap_name);
  g_free(iap_name);

  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
  g_object_set(G_OBJECT(label), "ellipsize", 3, "wrap", 1, 0);
  image = gtk_image_new();
  hbox = gtk_hbox_new(0, 0);
  gtk_box_pack_start(GTK_BOX(hbox), image, 0, 0, 0);
  gtk_box_pack_start(GTK_BOX(hbox), label, 1, 1, 0);

  return hbox;
}

GtkWidget *
iap_common_make_connection_entry(const gchar *iap)
{
  return iap_common_make_connection(iap, NULL);
}
