#ifndef CONNUI_CELLULAR_DATA_H
#define CONNUI_CELLULAR_DATA_H

typedef void (*connui_cellular_data_notify) (gboolean suspended,guint32 suspendcode,gpointer user_data); 
void connui_cellular_data_close(connui_cellular_data_notify callback);
gboolean connui_cellular_data_status(connui_cellular_data_notify callback,gpointer user_data);

#endif
