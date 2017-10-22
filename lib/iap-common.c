#include <hildon/hildon.h>
#include <libintl.h>

#include "iap-common.h"

GtkWidget *
iap_common_show_saved_not_found_banner(GtkWidget *widget)
{
  return hildon_banner_show_information(GTK_WIDGET(widget), NULL,
                                        dgettext("osso-connectivity-ui",
                                                 "conn_ib_net_conn_not_found"));
}

void iap_common_pack_to_hildon_button(GtkWidget *hbutton,GtkWidget *child,gboolean expand,gboolean fill)
{
  g_return_if_fail(hbutton != NULL && child != NULL);
  GtkWidget *widget1;
  GtkWidget *widget2;
  widget1 = gtk_bin_get_child(GTK_BIN(hbutton));
  if (widget1)
  {
    widget2 = gtk_bin_get_child(GTK_BIN(widget1));
    if (widget2)
    {
      gtk_box_pack_start(GTK_BOX(widget2), child, expand, fill, 0);
    }
  }
}
