#include <gtk/gtk.h>

#include <string.h>

#include "iap-common.h"

#include "connui-cell-renderer-operator.h"

enum
{
  PROP_0,
  PROP_PIXBUF,
  PROP_SERVICE_TYPE,
  PROP_SERVICE_ID,
  PROP_SERVICE_TEXT
};

struct _ConnuiCellRendererOperator
{
  GtkCellRendererText parent;
  GtkCellRenderer *pixbuf;
  gchar *service_type;
  gchar *service_id;
  gchar *service_text;
};

struct _ConnuiCellRendererOperatorClass
{
  GtkCellRendererTextClass parent_class;
};
typedef struct _ConnuiCellRendererOperatorClass ConnuiCellRendererOperatorClass;

G_DEFINE_TYPE(ConnuiCellRendererOperator, connui_cell_renderer_operator, GTK_TYPE_CELL_RENDERER_TEXT);

GtkCellRenderer *
connui_cell_renderer_operator_new()
{
  return
      GTK_CELL_RENDERER(g_object_new(CONNUI_TYPE_CELL_RENDERER_OPERATOR, NULL));
}

static void
connui_cell_renderer_operator_get_size(GtkCellRenderer *cell, GtkWidget *widget,
                                       GdkRectangle *cell_area, gint *x_offset,
                                       gint *y_offset, gint *width,
                                       gint *height)
{
  //todo
}

static void
connui_cell_renderer_operator_render(GtkCellRenderer *cell, GdkDrawable *window,
                                     GtkWidget *widget,
                                     GdkRectangle *background_area,
                                     GdkRectangle *cell_area,
                                     GdkRectangle *expose_area,
                                     GtkCellRendererState flags)
{
  ConnuiCellRendererOperator *self = CONNUI_CELL_RENDERER_OPERATOR(cell);
  GdkRectangle *final_background_area;
  GdkRectangle *final_cell_area;
  GdkRectangle *final_expose_area;
  GdkRectangle temp_expose_area;
  GdkRectangle temp_cell_area;
  GdkRectangle temp_background_area;
  gint height;
  gint width;

  gtk_cell_renderer_get_size(self->pixbuf, widget, 0, 0, 0, &width, &height);
  gtk_cell_renderer_render(self->pixbuf, window, widget, background_area,
                           cell_area, expose_area, flags);

  if (background_area)
  {
    temp_background_area.x = background_area->x;
    temp_background_area.y = background_area->y;
    temp_background_area.width = background_area->width;
    temp_background_area.height = background_area->height;

    if (background_area->width - width < 0)
      return;

    temp_background_area.width = background_area->width - width;
    temp_background_area.x = width + background_area->x;
  }

  if (cell_area)
  {
    temp_cell_area.x = cell_area->x;
    temp_cell_area.y = cell_area->y;
    temp_cell_area.width = cell_area->width;
    temp_cell_area.height = cell_area->height;

    if (cell_area->width - width < 0)
      return;

    temp_cell_area.width = cell_area->width - width;
    temp_cell_area.x = width + cell_area->x;
  } 

  if (expose_area)
  {
    temp_expose_area.x = expose_area->x;
    temp_expose_area.y = expose_area->y;
    temp_expose_area.width = expose_area->width;
    temp_expose_area.height = expose_area->height;

    if (expose_area->width - width < 0)
      return;

    temp_expose_area.width = expose_area->width - width;
    temp_expose_area.x = width + expose_area->x;
  } 

  if (background_area)
    final_background_area = &temp_background_area;
  else
    final_background_area = 0;

  if (cell_area)
    final_cell_area = &temp_cell_area;
  else
    final_cell_area = 0;

  if (expose_area)
    final_expose_area = &temp_expose_area;
  else
    final_expose_area = 0;

  GTK_CELL_RENDERER_CLASS(connui_cell_renderer_operator_parent_class)->
      render(cell, window, widget, final_background_area, final_cell_area,
             final_expose_area, flags);
}

static void
set_service_properties(ConnuiCellRendererOperator *self)
{
  iap_common_set_service_properties(self->service_type, self->service_id,
                                    self->service_text, G_OBJECT(self->pixbuf),
                                    G_OBJECT(&self->parent));
}

static void
set_service_type_and_id(ConnuiCellRendererOperator *self,
                        const gchar *service_type,
                        const gchar *service_id)
{
  if (!service_type && self->service_type)
  {
    g_free(self->service_type);
    self->service_type = NULL;
  }

  if (!service_id && self->service_id)
  {
    g_free(self->service_id);
    self->service_id = NULL;
  }

  if (!service_type && !service_id)
  {
    g_object_set(G_OBJECT(&self->parent), "pixbuf", NULL, NULL);
    return;
  }

  if (!self->service_type || strcmp(self->service_type, service_type) ||
      !self->service_id || strcmp(self->service_id, service_id))
  {
    if (strcmp(self->service_type, service_type))
    {
      g_free(self->service_type);
      self->service_type = g_strdup(service_type);
    }

    if (strcmp(self->service_id, service_id))
    {
      g_free(self->service_id);
      self->service_id = g_strdup(service_id);
    }

    /* WTF is going on here ?!? */
    if (self->service_type)
    {
      if (!self->service_id)
        return;

      if (*self->service_type)
      {
        if (*self->service_id)
          set_service_properties(self);
        return;
      }

      if (*self->service_id)
        return;
    }
    else if (self->service_id)
    {
      return;
    }

    set_service_properties(self);
  }
}

static void
connui_cell_renderer_operator_init(ConnuiCellRendererOperator *self)
{
  self->pixbuf = gtk_cell_renderer_pixbuf_new();
  g_object_ref_sink(self->pixbuf);
  g_object_set(G_OBJECT(self), "ellipsize", 3, NULL);
  g_object_set(G_OBJECT(self), "xalign", 0, NULL);
  self->service_text = NULL;
  self->service_type = NULL;
  self->service_id = NULL;
}

static void
connui_cell_renderer_operator_get_property(GObject *object, guint prop_id,
                                           GValue *value, GParamSpec *pspec)
{
  ConnuiCellRendererOperator *self = CONNUI_CELL_RENDERER_OPERATOR(object);

  switch (prop_id)
  {
    case PROP_PIXBUF:
    {
      gpointer val;

      g_object_get(self->pixbuf, "pixbuf", &val, NULL);
      g_value_take_object(value, val);
      break;
    }
    case PROP_SERVICE_TYPE:
      g_value_set_string(value, self->service_type);
      break;
    case PROP_SERVICE_ID:
      g_value_set_string(value, self->service_id);
      break;
    case PROP_SERVICE_TEXT:
      g_value_set_string(value, self->service_text);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
connui_cell_renderer_operator_set_property(GObject *object, guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec)
{
  ConnuiCellRendererOperator *self = CONNUI_CELL_RENDERER_OPERATOR(object);

  switch (prop_id)
  {
    case PROP_PIXBUF:
      g_object_set(self->pixbuf, "pixbuf", g_value_get_object(value), NULL);
      g_object_set(self->pixbuf, "icon-name", NULL, NULL);
      break;
    case PROP_SERVICE_TYPE:
      set_service_type_and_id(self, g_value_get_string(value),
                              self->service_id);
      break;
    case PROP_SERVICE_ID:

      set_service_type_and_id(self, self->service_type,
                              g_value_get_string(value));
      break;
    case PROP_SERVICE_TEXT:
      g_free(self->service_text);
      self->service_text = g_value_dup_string(value);
      set_service_properties(self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
connui_cell_renderer_operator_dispose(GObject *object)
{
  G_OBJECT_CLASS(connui_cell_renderer_operator_parent_class)->dispose(object);
}

static void
connui_cell_renderer_operator_finalize(GObject *object)
{
  ConnuiCellRendererOperator *self = CONNUI_CELL_RENDERER_OPERATOR(object);

  g_object_unref(self->pixbuf);
  g_free(self->service_type);
  g_free(self->service_id);
  g_free(self->service_text);

  G_OBJECT_CLASS(connui_cell_renderer_operator_parent_class)->finalize(object);
}

static void
connui_cell_renderer_operator_class_init(ConnuiCellRendererOperatorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkCellRendererClass *gtk_cell_renderer_class =
      GTK_CELL_RENDERER_CLASS(klass);

  connui_cell_renderer_operator_parent_class = g_type_class_peek_parent(klass);

  object_class->get_property = connui_cell_renderer_operator_get_property;
  object_class->set_property = connui_cell_renderer_operator_set_property;
  object_class->dispose = connui_cell_renderer_operator_dispose;
  object_class->finalize = connui_cell_renderer_operator_finalize;

  gtk_cell_renderer_class->get_size = connui_cell_renderer_operator_get_size;
  gtk_cell_renderer_class->render = connui_cell_renderer_operator_render;

  g_object_class_install_property(object_class, PROP_PIXBUF,
                                  g_param_spec_object("pixbuf",
                                                      "Pixbuf Object",
                                                      "Logo to render",
                                                      GDK_TYPE_PIXBUF,
                                                      G_PARAM_WRITABLE | G_PARAM_READABLE));

  g_object_class_install_property(object_class, PROP_SERVICE_TYPE,
                                  g_param_spec_string("service-type",
                                                      "Service type",
                                                      "Service type which is used to set logo and markup",
                                                      NULL,
                                                      G_PARAM_WRITABLE | G_PARAM_READABLE));
  g_object_class_install_property(object_class, PROP_SERVICE_ID,
                                  g_param_spec_string("service-id",
                                                      "Service ID",
                                                      "Service ID which is used to set logo and markup",
                                                      NULL,
                                                      G_PARAM_WRITABLE | G_PARAM_READABLE));
  g_object_class_install_property(object_class, PROP_SERVICE_TEXT,
                                  g_param_spec_string("service-text",
                                                      "Service text",
                                                      "Service text is used to set text content",
                                                      NULL,
                                                      G_PARAM_WRITABLE | G_PARAM_READABLE));
}
