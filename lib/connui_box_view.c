#include <gtk/gtk.h>
#include <hildon/hildon-button.h>

#include "connui_box_view.h"

struct _ConnuiBoxViewPrivate
{
  GtkTreeModel *model;
  GArray *children;
};

enum
{
  PROP_0,
  PROP_MODEL
};

enum
{
  ROW_ACTIVATED,
  LAST_SIGNAL
};

#if (!GLIB_CHECK_VERSION (2, 38, 0))
/* Fremantle glib compatibility */
G_DEFINE_TYPE(ConnuiBoxView, connui_box_view, GTK_TYPE_VBOX);
#else
G_DEFINE_TYPE_WITH_PRIVATE(ConnuiBoxView, connui_box_view, GTK_TYPE_VBOX);
#endif

static guint connui_box_view_signals[LAST_SIGNAL];

static gint
connui_box_view_get_path_index(GtkTreePath *path)
{
  g_return_val_if_fail(path != NULL, -1);

  return gtk_tree_path_get_indices(path)[0];
}

static GtkWidget *
connui_box_view_get_child_for_path(ConnuiBoxView *view, GtkTreePath *path)
{
  gint idx = connui_box_view_get_path_index(path);

  g_return_val_if_fail(
        idx >= 0 && view != NULL && idx < view->priv->children->len, NULL);

  return g_array_index(view->priv->children, GtkWidget *, idx);
}

static void
connui_box_view_update_child(ConnuiBoxView *view, GtkWidget *child,
                             GtkTreeModel *model, GtkTreeIter *iter)
{
  ConnuiBoxViewClass *klass;

  g_return_if_fail(CONNUI_IS_BOX_VIEW(view));

  klass = CONNUI_BOX_VIEW_GET_CLASS(view);

  if (klass->update_child)
    klass->update_child(view, child, model, iter);
}

static void
connui_box_view_row_changed(GtkTreeModel *model, GtkTreePath *path,
                            GtkTreeIter *iter, gpointer user_data)
{
  ConnuiBoxView *view = CONNUI_BOX_VIEW(user_data);
  GtkWidget *child = connui_box_view_get_child_for_path(view, path);

  g_return_if_fail(child != NULL);

  connui_box_view_update_child(view, child, model, iter);
}

static void
connui_box_view_row_deleted(GtkTreeModel *model, GtkTreePath *path,
                            gpointer user_data)
{
  ConnuiBoxView *view = CONNUI_BOX_VIEW(user_data);
  ConnuiBoxViewPrivate* priv = view->priv;
  gint idx = connui_box_view_get_path_index(path);
  GtkWidget *child = connui_box_view_get_child_for_path(view, path);

  g_return_if_fail(child != NULL);

  gtk_widget_destroy(child);
  g_array_remove_index(priv->children, idx);
}

static void
connui_box_view_button_clicked(GtkButton *button, gpointer user_data)
{
  ConnuiBoxView *view = CONNUI_BOX_VIEW(user_data);
  ConnuiBoxViewPrivate* priv = view->priv;
  GArray *children = priv->children;

  if (children->len)
  {
    int idx;

    for (idx = 0; idx < children->len; idx++)
    {
      if (GTK_WIDGET(button) == g_array_index(children, GtkWidget *, idx))
        break;
    }

    if (idx < children->len)
    {
      GtkTreePath *path = gtk_tree_path_new();

      gtk_tree_path_append_index(path, idx);
      g_signal_emit(G_OBJECT(view),
                    connui_box_view_signals[ROW_ACTIVATED], 0, path);
      gtk_tree_path_free(path);
      return;
    }
  }

  g_warning("Unable to find clicked button from the view");
}

static void
connui_box_view_row_inserted(GtkTreeModel *model, GtkTreePath *path,
                             GtkTreeIter *iter, gpointer user_data)
{
  ConnuiBoxView *view = CONNUI_BOX_VIEW(user_data);
  ConnuiBoxViewPrivate* priv = view->priv;
  gint idx = connui_box_view_get_path_index(path);
  GtkWidget *button = NULL;

  g_return_if_fail(idx >= 0 && view != NULL);

  button = hildon_button_new(
        HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_FULLSCREEN_WIDTH,
        HILDON_BUTTON_ARRANGEMENT_VERTICAL);
  gtk_widget_set_size_request(GTK_WIDGET(button), -1, 70);
  g_signal_connect(G_OBJECT(button), "clicked",
                   G_CALLBACK(connui_box_view_button_clicked), view);
  gtk_box_pack_start(GTK_BOX(view), button, 0, 0, 0);
  gtk_box_reorder_child(GTK_BOX(view), button, idx);
  gtk_widget_show_all(button);
  g_array_insert_vals(priv->children, idx, &button, 1u);
  connui_box_view_update_child(view, button, model, iter);
}

static void
connui_box_view_rows_reordered(GtkTreeModel *model, GtkTreePath *path,
                               GtkTreeIter *iter, gpointer new_order,
                               gpointer user_data)
{
  ConnuiBoxView *view = CONNUI_BOX_VIEW(user_data);
  ConnuiBoxViewPrivate* priv = view->priv;
  gint *order = (gint *)new_order;
  gint n_children = gtk_tree_model_iter_n_children(model, iter);

  if (iter && n_children > 1)
  {
    GArray *children = g_array_new(FALSE, TRUE, sizeof(GtkWidget *));
    int idx;

    for (idx = 0; idx < n_children; idx++)
    {
      GtkWidget *child = g_array_index(priv->children, GtkWidget *, order[idx]);

      gtk_box_reorder_child(GTK_BOX(view), child, idx + 1);
      g_array_insert_vals(children, idx, &child, 1);
    }

    g_array_free(priv->children, TRUE);
    priv->children = children;
  }
}

static void
connui_box_view_set_model(ConnuiBoxView *view, GtkTreeModel *model)
{
  ConnuiBoxViewPrivate *priv = view->priv;

  if (priv->model)
  {
    g_signal_handlers_disconnect_matched(
          G_OBJECT(priv->model), G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
          0, 0, NULL, connui_box_view_row_changed, view);
    g_signal_handlers_disconnect_matched(
          G_OBJECT(priv->model), G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
          0, 0, NULL, connui_box_view_row_deleted, view);
    g_signal_handlers_disconnect_matched(
          G_OBJECT(priv->model), G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
          0, 0, NULL, connui_box_view_row_inserted, view);
    g_signal_handlers_disconnect_matched(
          G_OBJECT(priv->model), G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
          0, 0, NULL, connui_box_view_rows_reordered, view);
    g_object_unref(priv->model);
    priv->model = NULL;
  }

  if (model)
  {
    priv->model = g_object_ref(model);
    g_signal_connect(G_OBJECT(model), "row-changed",
                     G_CALLBACK(connui_box_view_row_changed), view);
    g_signal_connect(G_OBJECT(model), "row-deleted",
                     G_CALLBACK(connui_box_view_row_deleted), view);
    g_signal_connect(G_OBJECT(model), "row-inserted",
                     G_CALLBACK(connui_box_view_row_inserted), view);
    g_signal_connect(G_OBJECT(model), "rows-reordered",
                     G_CALLBACK(connui_box_view_rows_reordered), view);
  }
}

static void
connui_box_view_get_property(GObject *object, guint prop_id, GValue *value,
                             GParamSpec *pspec)
{
  ConnuiBoxViewPrivate *priv = CONNUI_BOX_VIEW(object)->priv;

  switch (prop_id)
  {
    case PROP_MODEL:
      g_value_set_object(value, priv->model);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
connui_box_view_set_property(GObject *object, guint prop_id,
                             const GValue *value, GParamSpec *pspec)
{
  switch (prop_id)
  {
    case PROP_MODEL:
      connui_box_view_set_model(CONNUI_BOX_VIEW(object),
                                (GtkTreeModel *)g_value_get_object(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
connui_box_view_dispose(GObject *object)
{
  G_OBJECT_CLASS(connui_box_view_parent_class)->dispose(object);
}

static void
connui_box_view_finalize(GObject *object)
{
  ConnuiBoxView *view = CONNUI_BOX_VIEW(object);
  ConnuiBoxViewPrivate *priv = view->priv;

  connui_box_view_set_model(view, NULL);
  g_array_free(priv->children, TRUE);

  G_OBJECT_CLASS(connui_box_view_parent_class)->finalize(object);
}

static void
connui_box_view_class_init(ConnuiBoxViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  connui_box_view_parent_class = g_type_class_peek_parent(klass);

  object_class->get_property = connui_box_view_get_property;
  object_class->set_property = connui_box_view_set_property;
  object_class->dispose = connui_box_view_dispose;
  object_class->finalize = connui_box_view_finalize;

  g_object_class_install_property(object_class, PROP_MODEL,
                                  g_param_spec_object(
                                    "model",
                                    "Tree model",
                                    "Tree model to be used for the view",
                                    GTK_TYPE_TREE_MODEL,
                                    G_PARAM_WRITABLE|G_PARAM_READABLE));

  connui_box_view_signals[ROW_ACTIVATED] =
      g_signal_new("row_activated",
                   G_TYPE_FROM_CLASS (klass),
                   G_SIGNAL_ACTION|G_SIGNAL_RUN_LAST,
                   0, NULL, NULL,
                   g_cclosure_marshal_VOID__BOXED,
                   G_TYPE_NONE,
                   1, GTK_TYPE_TREE_PATH);
#if (!GLIB_CHECK_VERSION (2, 38, 0))
  g_type_class_add_private(klass, sizeof(ConnuiBoxViewPrivate));
#endif
}

static void
connui_box_view_init(ConnuiBoxView *view)
{
  ConnuiBoxViewPrivate *priv = CONNUI_BOX_VIEW_GET_PRIVATE(view);

  view->priv = priv;
  priv->model = NULL;
  priv->children = g_array_new(FALSE, TRUE, sizeof(GtkWidget*));
}

GtkWidget *
connui_box_view_new_with_model(GtkTreeModel *model)
{
  return GTK_WIDGET(g_object_new(CONNUI_TYPE_BOX_VIEW, "model", model, NULL));
}

GtkTreeModel *
connui_box_view_get_model(ConnuiBoxView *view)
{
  return view->priv->model;
}
