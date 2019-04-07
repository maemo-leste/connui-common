#include <dbus/dbus.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <string.h>

#include "connui-box-view.h"
#include "connui-scan-box-view.h"
#include "connui-pixbuf-cache.h"

#include "iap-common.h"
#include "iap-scan.h"

#include "intl.h"

struct _ConnuiScanBoxViewPrivate
{
  ConnuiPixbufCache *cache;
};
typedef struct _ConnuiScanBoxViewPrivate ConnuiScanBoxViewPrivate;

struct _ConnuiScanBoxView
{
  ConnuiBoxView parent;
  ConnuiScanBoxViewPrivate *priv;
};

struct _ConnuiScanBoxViewClass
{
  ConnuiBoxViewClass parent_class;
};
typedef struct _ConnuiScanBoxViewClass ConnuiScanBoxViewClass;

#if (!GLIB_CHECK_VERSION (2, 38, 0))
/* Fremantle glib compatibility */
G_DEFINE_TYPE(ConnuiScanBoxView, connui_scan_box_view, CONNUI_TYPE_BOX_VIEW);
#else
G_DEFINE_TYPE_WITH_PRIVATE(ConnuiScanBoxView, connui_scan_box_view, CONNUI_TYPE_BOX_VIEW);
#endif

static void
add_icon_to_button(GtkWidget *button, GdkPixbuf *icon)
{
  GtkWidget *image;
  GtkWidget *parent;

  if (icon)
    image = gtk_image_new_from_pixbuf(icon);
  else
    image = gtk_image_new();

  iap_common_pack_to_hildon_button(button, image, FALSE, FALSE);

  parent = gtk_widget_get_parent(image);

  if (GTK_IS_BOX(parent))
    gtk_widget_show_all(parent);
  else
    gtk_widget_destroy(image);
}
static void
connui_scan_box_view_update_child(ConnuiBoxView *view, GtkWidget *hbutton,
                                  GtkTreeModel *model, GtkTreeIter *iter)
{
  gchar *service_text = NULL;
  gchar *icon = NULL;
  gchar *saved_icon = NULL;
  gchar *security_icon = NULL;
  GdkPixbuf *icon_pixbuf;
  GdkPixbuf *saved_icon_pixbuf;
  GdkPixbuf *sec_icon_pixbuf;
  network_entry *entry = NULL;
  gboolean can_disconnect = FALSE;
  ConnuiScanBoxViewPrivate *priv = CONNUI_SCAN_BOX_VIEW(view)->priv;

  gtk_tree_model_get(model, iter,
                     IAP_SCAN_LIST_IAP_ICON_NAME, &icon,
                     IAP_SCAN_LIST_SERVICE_TEXT, &service_text,
                     IAP_SCAN_LIST_SAVED_ICON, &saved_icon,
                     IAP_SCAN_LIST_SECURITY_ICON, &security_icon,
                     IAP_SCAN_LIST_SCAN_ENTRY, &entry,
                     IAP_SCAN_LIST_CAN_DISCONNECT, &can_disconnect,
                     -1);

  if (!service_text)
  {
    g_free(icon);
    g_free(saved_icon);
    g_free(security_icon);
    return;
  }

  icon_pixbuf = connui_pixbuf_cache_get(priv->cache, icon, 48);
  saved_icon_pixbuf = connui_pixbuf_cache_get(priv->cache, saved_icon, 48);
  sec_icon_pixbuf = connui_pixbuf_cache_get(priv->cache, security_icon, 48);

  if (can_disconnect)
  {
    gchar* tmp = service_text;
    const gchar *s = _("conn_fi_disconnect_iap");

    service_text = g_strdup_printf(s, tmp);
    g_free(tmp);
  }

  hildon_button_set_title(HILDON_BUTTON(hbutton), service_text);

  if (entry && entry->network_type &&
      !strncmp(entry->network_type, "WLAN_", 5) &&
      entry->network_attributes & 0x1000)
  {
    hildon_button_set_value(HILDON_BUTTON(hbutton), _("conn_va_wps_complient"));
  }

  hildon_button_set_alignment(HILDON_BUTTON(hbutton), 0.0, 0.5, 1.0, 0.0);
  hildon_button_set_title_alignment(HILDON_BUTTON(hbutton), 0.0, 0.5);
  hildon_button_set_value_alignment(HILDON_BUTTON(hbutton), 0.0, 0.5);
  add_icon_to_button(hbutton, icon_pixbuf);
  add_icon_to_button(hbutton, saved_icon_pixbuf);
  add_icon_to_button(hbutton, sec_icon_pixbuf);
  g_free(service_text);
  g_free(icon);
  g_free(saved_icon);
  g_free(security_icon);
}

static void
connui_scan_box_view_dispose(GObject *object)
{
  G_OBJECT_CLASS(connui_scan_box_view_parent_class)->dispose(object);
}

static void
connui_scan_box_view_finalize(GObject *object)
{
  connui_pixbuf_cache_destroy(CONNUI_SCAN_BOX_VIEW(object)->priv->cache);

  G_OBJECT_CLASS(connui_scan_box_view_parent_class)->finalize(object);
}

static void
connui_scan_box_view_class_init(ConnuiScanBoxViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = connui_scan_box_view_dispose;
  object_class->finalize = connui_scan_box_view_finalize;

  CONNUI_BOX_VIEW_CLASS(klass)->update_child =
      connui_scan_box_view_update_child;

#if (!GLIB_CHECK_VERSION (2, 38, 0))
  g_type_class_add_private(klass, sizeof(ConnuiScanBoxViewPrivate));
#endif
}

static void
connui_scan_box_view_init(ConnuiScanBoxView *view)
{

  ConnuiScanBoxViewPrivate *priv = CONNUI_SCAN_BOX_VIEW_GET_PRIVATE(view);

  view->priv = priv;

  priv->cache = connui_pixbuf_cache_new();
}

GtkWidget *
connui_scan_box_view_new_with_model(GtkTreeModel *model)
{
  ConnuiScanBoxView *view =
      g_object_new(CONNUI_TYPE_SCAN_BOX_VIEW, "model", model, NULL);

  return GTK_WIDGET(view);
}
