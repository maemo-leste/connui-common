#include <hildon/hildon.h>

#include "intl.h"

static GtkWidget *note = NULL;

static gint
gateway_common_show_confirmation_note(GtkWindow *parent, const gchar *title)
{
  if (GTK_IS_WIDGET(note))
    note = hildon_note_new_confirmation(parent, title);

  return gtk_dialog_run(GTK_DIALOG(note));
}

void
gateway_common_show_status_close()
{
  if (GTK_IS_WIDGET(note))
  {
    gtk_widget_destroy(note);
    note = NULL;
  }
}

gboolean
gateway_common_show_flight_mode(GtkWindow *parent)
{
  gint res;

  res = gateway_common_show_confirmation_note(parent, _("conn_ni_no_bt"));
  gateway_common_show_status_close();

  return res == GTK_RESPONSE_OK;
}
