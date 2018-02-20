#include <dbus/dbus.h>
#include <gtk/gtk.h>
#include <libintl.h>
#include <hildon/hildon.h>
#include <string.h>

#include "connui-box-view.h"
#include "connui-scan-box-view.h"
#include "connui-pixbuf-cache.h"

#include "iap-common.h"

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
  gchar *title = NULL;
  gchar *icon1 = NULL;
  gchar *icon2 = NULL;
  gchar *icon3 = NULL;
  GdkPixbuf *icon1buf;
  GdkPixbuf *icon2buf;
  GdkPixbuf *icon3buf;
  network_entry *entry = NULL;
  gboolean can_disconnect = FALSE;
  ConnuiScanBoxViewPrivate *priv = CONNUI_SCAN_BOX_VIEW(view)->priv;

  gtk_tree_model_get(model, iter, 0, &icon1, 4, &title, 7, &icon2, 9,
                     &icon3, 10, &entry, 11, &can_disconnect, -1);

  if (!title)
  {
    g_free(icon1);
    g_free(icon2);
    g_free(icon3);
    return;
  }

  icon1buf = connui_pixbuf_cache_get(priv->cache, icon1, 48);
  icon2buf = connui_pixbuf_cache_get(priv->cache, icon2, 48);
  icon3buf = connui_pixbuf_cache_get(priv->cache, icon3, 48);

  if (can_disconnect)
  {
    gchar* tmp = title;
    const gchar *s = dgettext("osso-connectivity-ui", "conn_fi_disconnect_iap");

    title = g_strdup_printf(s, tmp);
    g_free(tmp);
  }

  hildon_button_set_title(HILDON_BUTTON(hbutton), title);

  if (entry && entry->network_type &&
      !strncmp(entry->network_type, "WLAN_", 5) &&
      entry->network_attributes & 0x1000)
  {
    hildon_button_set_value(HILDON_BUTTON(hbutton),
                            dgettext("osso-connectivity-ui",
                                     "conn_va_wps_complient"));
  }

  hildon_button_set_alignment(HILDON_BUTTON(hbutton), 0.0, 0.5, 1.0, 0.0);
  hildon_button_set_title_alignment(HILDON_BUTTON(hbutton), 0.0, 0.5);
  hildon_button_set_value_alignment(HILDON_BUTTON(hbutton), 0.0, 0.5);
  add_icon_to_button(hbutton, icon1buf);
  add_icon_to_button(hbutton, icon2buf);
  add_icon_to_button(hbutton, icon3buf);
  g_free(title);
  g_free(icon1);
  g_free(icon2);
  g_free(icon3);
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
