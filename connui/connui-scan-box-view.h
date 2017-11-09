#ifndef CONNUI_SCAN_BOX_VIEW_H
#define CONNUI_SCAN_BOX_VIEW_H

#define CONNUI_TYPE_SCAN_BOX_VIEW (connui_scan_box_view_get_type())
#define CONNUI_SCAN_BOX_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CONNUI_TYPE_SCAN_BOX_VIEW, ConnuiScanBoxView))
#define CONNUI_IS_SCAN_BOX_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CONNUI_TYPE_SCAN_BOX_VIEW))
#define CONNUI_SCAN_BOX_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CONNUI_TYPE_SCAN_BOX_VIEW, ConnuiScanBoxViewClass))
#define CONNUI_SCAN_BOX_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE((obj), CONNUI_TYPE_SCAN_BOX_VIEW, ConnuiScanBoxViewPrivate))

typedef struct _ConnuiScanBoxView ConnuiScanBoxView;

GType connui_scan_box_view_get_type(void) G_GNUC_CONST;

GtkWidget *connui_scan_box_view_new_with_model(GtkTreeModel *model);

#endif // CONNUI_SCAN_BOX_VIEW_H
