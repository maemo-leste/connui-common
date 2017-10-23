#ifndef CONNUI_FLIGHTMODE_H
#define CONNUI_FLIGHTMODE_H

typedef void (*connui_flightmode_notify) (gboolean offline, gpointer user_data);

void connui_flightmode_off_confirm();
void connui_flightmode_off();
void connui_flightmode_on();
void connui_flightmode_close(connui_flightmode_notify callback);
gboolean connui_flightmode_status(connui_flightmode_notify callback,gpointer user_data);

#endif
