#include <gtk/gtk.h>

#include "connui-box-view.h"
#include "connui-scan-box_view.h"
#include "connui-pixbuf-cache.h"

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
  void (*update_child)(ConnuiScanBoxView *, GtkWidget *, GtkTreeModel *, GtkTreeIter *);
};
typedef struct _ConnuiScanBoxViewClass ConnuiScanBoxViewClass;

#if (!GLIB_CHECK_VERSION (2, 38, 0))
/* Fremantle glib compatibility */
G_DEFINE_TYPE(ConnuiScanBoxView, connui_scan_box_view, CONNUI_TYPE_BOX_VIEW);
#else
G_DEFINE_TYPE_WITH_PRIVATE(ConnuiScanBoxView, connui_scan_box_view, CONNUI_TYPE_BOX_VIEW);
#endif

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

  connui_scan_box_view_parent_class = g_type_class_peek_parent(klass);

  object_class->dispose = connui_scan_box_view_dispose;
  object_class->finalize = connui_scan_box_view_finalize;

  CONNUI_BOX_VIEW_GET_CLASS(klass)->update_child =
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
