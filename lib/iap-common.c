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
iap_common_get_last_used_network(struct network_entry *network)
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

void
iap_network_entry_clear(struct network_entry *network)
{
  if (!network)
    return;

  g_free(network->service_type);
  network->service_type = NULL;

  network->service_attributes = 0;

  g_free(network->service_id);
  network->service_id = NULL;

  g_free(network->network_type);
  network->network_type = NULL;

  network->network_attributes = 0;

  g_free(network->network_id);
  network->network_id = NULL;
}
