#ifndef CONNUI_BOX_VIEW_H
#define CONNUI_BOX_VIEW_H

#define CONNUI_TYPE_BOX_VIEW (connui_box_view_get_type())
#define CONNUI_BOX_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CONNUI_TYPE_BOX_VIEW, ConnuiBoxView))
#define CONNUI_IS_BOX_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CONNUI_TYPE_BOX_VIEW))
#define CONNUI_BOX_VIEW_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CONNUI_TYPE_BOX_VIEW, ConnuiBoxViewClass))
#define CONNUI_BOX_VIEW_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE((obj), CONNUI_TYPE_BOX_VIEW, ConnuiBoxViewPrivate))

typedef struct _ConnuiBoxView ConnuiBoxView;
typedef struct _ConnuiBoxViewClass ConnuiBoxViewClass;
typedef struct _ConnuiBoxViewPrivate ConnuiBoxViewPrivate;

struct _ConnuiBoxView
{
  GtkVBox parent;
  ConnuiBoxViewPrivate *priv;
};

struct _ConnuiBoxViewClass
{
  GtkVBoxClass parent_class;
  void (*update_child)(ConnuiBoxView *, GtkWidget *, GtkTreeModel *, GtkTreeIter *);
};

GType connui_box_view_get_type(void) G_GNUC_CONST;

GtkWidget *connui_box_view_new_with_model(GtkTreeModel *model);
GtkTreeModel *connui_box_view_get_model(ConnuiBoxView *view);

#endif // CONNUI_BOX_VIEW_H
