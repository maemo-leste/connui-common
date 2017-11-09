#ifndef CONNUI_CELL_RENDERER_OPERATOR_H
#define CONNUI_CELL_RENDERER_OPERATOR_H


#define CONNUI_TYPE_CELL_RENDERER_OPERATOR (connui_cell_renderer_operator_get_type())
#define CONNUI_CELL_RENDERER_OPERATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CONNUI_TYPE_CELL_RENDERER_OPERATOR, ConnuiCellRendererOperator))
#define CONNUI_IS_CELL_RENDERER_OPERATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CONNUI_TYPE_CELL_RENDERER_OPERATOR))
#define CONNUI_CELL_RENDERER_OPERATOR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CONNUI_TYPE_CELL_RENDERER_OPERATOR, ConnuiCellRendererOperatorClass))

typedef struct _ConnuiCellRendererOperator ConnuiCellRendererOperator;

GType connui_cell_renderer_operator_get_type(void) G_GNUC_CONST;
GtkCellRenderer *connui_cell_renderer_operator_new(void);

#endif // CONNUI_CELL_RENDERER_OPERATOR_H
