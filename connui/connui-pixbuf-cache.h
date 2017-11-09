#ifndef CONNUI_PIXBUF_CACHE_H
#define CONNUI_PIXBUF_CACHE_H

typedef struct _ConnuiPixbufCache ConnuiPixbufCache;

ConnuiPixbufCache *connui_pixbuf_cache_new();
void connui_pixbuf_cache_destroy(ConnuiPixbufCache *cache);

GdkPixbuf *connui_pixbuf_load(const gchar *icon, gint size);
void connui_pixbuf_unref(GdkPixbuf *pixbuf);

GdkPixbuf *connui_pixbuf_cache_get_with_flags(ConnuiPixbufCache *cache,
                                              const gchar *icon, gint size,
                                              guint flags);
GdkPixbuf *connui_pixbuf_cache_get(ConnuiPixbufCache *cache, const gchar *icon,
                                   gint size);

#endif // CONNUI_PIXBUF_CACHE_H
