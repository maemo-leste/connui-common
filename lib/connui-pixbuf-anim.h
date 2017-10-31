#ifndef CONNUI_PIXBUF_ANIM_H
#define CONNUI_PIXBUF_ANIM_H

#include <gtk/gtk.h>

typedef struct _ConnuiPixbufAnim ConnuiPixbufAnim;

typedef void (*pixbuf_anim_cb) (gpointer user_data, GdkPixbuf *pixbuf);

ConnuiPixbufAnim *connui_pixbuf_anim_new(const gchar *anim_name, gint size, GError **error);
void connui_pixbuf_anim_destroy(ConnuiPixbufAnim *anim);
void connui_pixbuf_anim_start(ConnuiPixbufAnim *anim, gpointer user_data, pixbuf_anim_cb callback);
void connui_pixbuf_anim_stop(ConnuiPixbufAnim *anim);
void connui_pixbuf_anim_show_current_and_advance(ConnuiPixbufAnim *anim);
ConnuiPixbufAnim *connui_pixbuf_anim_new_from_icons(int size, float rate, const gchar *first_icon, ...);

#endif // CONNUI_PIXBUF_ANIM_H
