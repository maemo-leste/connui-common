#include <glib.h>
#include <osso-log.h>

#include <syslog.h>
#include <stdio.h>


/* FIXME -change logging for debug builds */
static void
g_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
              const gchar *message, gpointer user_data)
{
  ;
}

static void
g_print_handler(const gchar *string)
{
  printf("LOG_INFO: %s\n", string);
}

static void
g_printerr_handler(const gchar *string)
{
  fprintf(stderr, "LOG_ERR: %s\n", string);
}

void
open_log(const char *ident, gboolean open)
{
  if (open)
  {
    ULOG_OPEN(ident);
    g_log_set_default_handler(g_log_handler, NULL);
  }
  else
  {
    g_set_print_handler(g_print_handler);
    g_set_printerr_handler(g_printerr_handler);
  }
}
