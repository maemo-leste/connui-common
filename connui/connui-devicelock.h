#ifndef CONNUI_DEVICELOCK_H
#define CONNUI_DEVICELOCK_H

typedef void (*connui_devicelock_notify) (gboolean locked, gpointer user_data); 

void connui_devicelock_close(connui_devicelock_notify callback);
gboolean connui_devicelock_status(connui_devicelock_notify callback,gpointer user_data);

#endif
