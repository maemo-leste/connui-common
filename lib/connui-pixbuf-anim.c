#include "connui-log.h"

#include "connui-pixbuf-anim.h"

struct _ConnuiPixbufAnim
{
  GdkPixbufAnimation *animation;
  GdkPixbufAnimationIter *iter;
  gpointer user_data;
  pixbuf_anim_cb callback;
  guint timeout;
};

ConnuiPixbufAnim *
connui_pixbuf_anim_new(const gchar *anim_name, gint size, GError **error)
{
  GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
  GtkIconInfo *icon_info;
  ConnuiPixbufAnim *anim;
  GdkPixbufAnimation *pixbuf_anim;
  GError *err = NULL;
  g_return_val_if_fail(anim_name != NULL && icon_theme != NULL, NULL);

  icon_info = gtk_icon_theme_lookup_icon(icon_theme, anim_name, size, 0);

  if (icon_info)
  {
    pixbuf_anim = gdk_pixbuf_animation_new_from_file(
          gtk_icon_info_get_filename(icon_info), &err);
    gtk_icon_info_free(icon_info);
  }
  else
    pixbuf_anim = gdk_pixbuf_animation_new_from_file(anim_name, &err);

  if (err)
    goto err_out;

  anim = g_new0(ConnuiPixbufAnim, 1);
  anim->animation = pixbuf_anim;

  return anim;

err_out:
  CONNUI_ERR("Unable to load animation '%s': %s", anim_name, err->message);

  if (error)
    *error = err;
  else
    g_clear_error(&err);

  return NULL;
}

void
connui_pixbuf_anim_destroy(ConnuiPixbufAnim *anim)
{
  if (!anim)
    return;

  connui_pixbuf_anim_stop(anim);

  if (anim->animation)
  {
    g_object_unref(anim->animation);
    anim->animation = NULL;
  }

  g_free(anim);
}

void
connui_pixbuf_anim_start(ConnuiPixbufAnim *anim, gpointer user_data,
                         pixbuf_anim_cb callback)
{
  g_return_if_fail(anim != NULL);

  connui_pixbuf_anim_stop(anim);
  anim->user_data = user_data;
  anim->callback = callback;
  anim->iter = NULL;
  connui_pixbuf_anim_show_current_and_advance(anim);
}

void
connui_pixbuf_anim_stop(ConnuiPixbufAnim *anim)
{
  g_return_if_fail(anim != NULL);

  if (anim->timeout)
  {
    g_source_remove(anim->timeout);
    anim->timeout = 0;
  }

  if (anim->iter)
  {
    g_object_unref(anim->iter);
    anim->iter = NULL;
  }

}

static gboolean
connui_pixbuf_anim_timeout(gpointer anim)
{
  g_return_val_if_fail(anim != NULL, FALSE);

  connui_pixbuf_anim_show_current_and_advance((ConnuiPixbufAnim *)anim);

  return FALSE;
}

void
connui_pixbuf_anim_show_current_and_advance(ConnuiPixbufAnim *anim)
{
  GdkPixbuf *pixbuf;
  int timeout;

  if (anim->iter)
    gdk_pixbuf_animation_iter_advance(anim->iter, NULL);
  else
  {
    anim->iter = gdk_pixbuf_animation_get_iter(anim->animation, NULL);

    if (!anim->iter)
    {
      CONNUI_ERR("NULL iterator from animation");
      return;
    }
  }

  pixbuf = gdk_pixbuf_animation_iter_get_pixbuf(anim->iter);

  if (anim->callback)
    anim->callback(anim->user_data, pixbuf);

  timeout = gdk_pixbuf_animation_iter_get_delay_time(anim->iter);

  if (timeout < 0)
  {
    g_object_unref(anim->iter);
    anim->iter = NULL;
    anim->timeout = g_idle_add(connui_pixbuf_anim_timeout, anim);
  }
  else
    anim->timeout = g_timeout_add(timeout, connui_pixbuf_anim_timeout, anim);
}
