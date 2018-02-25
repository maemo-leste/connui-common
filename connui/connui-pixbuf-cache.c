#include <gtk/gtk.h>

#include "connui-pixbuf-cache.h"
#include "connui-log.h"

struct _ConnuiPixbufCache
{
  GHashTable *hash1;
  GHashTable *hash2;
  GtkIconTheme *icon_theme;
};

static void
connui_pixbuf_cache_icon_theme_changed_cb(GtkIconTheme *icon_theme,
                                          gpointer user_data)
{
  ConnuiPixbufCache *cache = (ConnuiPixbufCache *)user_data;

  if (cache->hash1)
  {
    g_hash_table_destroy(cache->hash1);
    cache->hash1 = NULL;
  }
}

void
connui_pixbuf_cache_destroy(ConnuiPixbufCache *cache)
{
  if (cache)
  {
    g_signal_handlers_disconnect_matched(
          G_OBJECT(cache->icon_theme),
          G_SIGNAL_MATCH_DATA|G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
          connui_pixbuf_cache_icon_theme_changed_cb, cache);

    if (cache->hash1)
      g_hash_table_destroy(cache->hash1);

    if (cache->hash2)
      g_hash_table_destroy(cache->hash2);

    g_free(cache);
  }
}

ConnuiPixbufCache *connui_pixbuf_cache_new()
{
  ConnuiPixbufCache *cache = g_new0(ConnuiPixbufCache, 1);

  cache->icon_theme = gtk_icon_theme_get_default();

  g_signal_connect(G_OBJECT(cache->icon_theme), "changed",
                   G_CALLBACK(connui_pixbuf_cache_icon_theme_changed_cb),
                   cache);

  return cache;
}

static void
connui_pixbuf_add_emblem_to_icon(GdkPixbuf **icon, int size, int flags)
{
  GdkPixbuf *emblem;

  if (!flags || !icon || !*icon)
    return;

  if (flags == 0x4000)
    emblem = connui_pixbuf_load("qgn_list_gene_fldr_exp", size);
  else if (flags == 0x4001)
    emblem = connui_pixbuf_load("qgn_list_gene_fldr_clp", size);
  else
    return;

  if (emblem)
  {
    GdkPixbuf *emblemmed = gdk_pixbuf_copy(*icon);

    if (emblemmed)
    {
      gint w;
      gint h;

      if (gdk_pixbuf_get_width(emblem) >= gdk_pixbuf_get_width(emblemmed))
        w = gdk_pixbuf_get_width(emblemmed);
      else
        w = gdk_pixbuf_get_width(emblem);

      if (gdk_pixbuf_get_height(emblem) >= gdk_pixbuf_get_height(emblemmed))
        h = gdk_pixbuf_get_height(emblemmed);
      else
        h = gdk_pixbuf_get_height(emblem);

      gdk_pixbuf_composite(
            emblem, emblemmed, 0, 0, w, h, 0.0, 0.0, 1.0, 1.0, 0, 255);
      connui_pixbuf_unref(emblem);
      connui_pixbuf_unref(*icon);
      *icon = emblemmed;
    }
  }
}

GdkPixbuf *
connui_pixbuf_cache_get_with_flags(ConnuiPixbufCache *cache, const gchar *icon,
                                   gint size, guint flags)
{
  GHashTable *table;
  GdkPixbuf *pixbuf = NULL;

  g_return_val_if_fail(cache != NULL, NULL);

  if (!icon)
    return NULL;

  if (!cache->hash1)
  {
    cache->hash1 = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                                         (GDestroyNotify)g_hash_table_destroy);
  }

  table = g_hash_table_lookup(cache->hash1, GINT_TO_POINTER(flags | size));

  if (!table)
  {
    table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                  (GDestroyNotify)connui_pixbuf_unref);
    g_hash_table_insert(cache->hash1, GINT_TO_POINTER(flags | size), table);
  }

  pixbuf = g_hash_table_lookup(table, icon);

  if (pixbuf)
    return pixbuf;

  pixbuf = connui_pixbuf_load(icon, size);

  if (!pixbuf)
  {
    if (!cache->hash2)
    {
      cache->hash2 =
          g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
                                (GDestroyNotify)g_hash_table_destroy);
    }

    table = g_hash_table_lookup(cache->hash2, GINT_TO_POINTER(flags | size));

    if (!table)
    {
      table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                    (GDestroyNotify)connui_pixbuf_unref);
      g_hash_table_insert(cache->hash2, GINT_TO_POINTER(flags | size), table);
    }

    pixbuf = g_hash_table_lookup(table, icon);

    if (pixbuf && !flags)
      goto finish;

    table = g_hash_table_lookup(cache->hash2, GINT_TO_POINTER(size));

    if (table)
      pixbuf = g_hash_table_lookup(table, icon);

    pixbuf = pixbuf;

    if (!pixbuf)
      return NULL;

    g_object_ref((gpointer)pixbuf);
  }

  connui_pixbuf_add_emblem_to_icon(&pixbuf, size, flags);

finish:

  if (pixbuf)
    g_hash_table_insert(table, g_strdup(icon), pixbuf);

  return pixbuf;
}

void
connui_pixbuf_unref(GdkPixbuf *pixbuf)
{
  if ( pixbuf )
    g_object_unref((gpointer)pixbuf);
}

GdkPixbuf *
connui_pixbuf_load(const gchar *icon, gint size)
{
  GdkPixbuf *pixbuf;
  GtkIconTheme *icon_theme;
  GError *error = NULL;

  if (!icon)
    return NULL;

  icon_theme = gtk_icon_theme_get_default();

  if (size <= 0)
  {
    gint *l = gtk_icon_theme_get_icon_sizes(icon_theme, icon);
    gint *p;

    for (p = l; *p && *p != -1; p++)
      size = *p;

    g_free(l);
  }

  pixbuf = gtk_icon_theme_load_icon(icon_theme, icon, size, 0, &error);

  if (error)
  {
    CONNUI_ERR("error loading pixbuf '%s': %s", icon, error->message);
    g_error_free(error);
  }

  return pixbuf;
}

GdkPixbuf *
connui_pixbuf_cache_get(ConnuiPixbufCache *cache, const gchar *icon, gint size)
{
  return connui_pixbuf_cache_get_with_flags(cache, icon, size, 0);
}
