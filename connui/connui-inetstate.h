#ifndef CONNUI_INETSTATE_H
#define CONNUI_INETSTATE_H

#include "iap-network.h"

struct _inetstate_network_stats
{
  dbus_uint32_t time_active;
  dbus_int32_t signal_strength;
  dbus_uint32_t tx_bytes;
  dbus_uint32_t rx_bytes;
};
typedef struct _inetstate_network_stats inetstate_network_stats;

enum inetstate_status {
  INETSTATE_STATUS_OFFLINE,
  INETSTATE_STATUS_ONLINE,
  INETSTATE_STATUS_CONNECTING,
  INETSTATE_STATUS_CONNECTED,
  INETSTATE_STATUS_DISCONNECTING,
  INETSTATE_STATUS_DISCONNECTED
};

typedef void (*inetstate_cb) (enum inetstate_status state, network_entry *entry, gpointer user_data);
typedef void (*inetstate_stats_cb) (network_entry *entry, inetstate_network_stats *stats, gpointer user_data);

gboolean connui_inetstate_statistics_start(gint interval, inetstate_stats_cb callback, gpointer user_data);
void connui_inetstate_statistics_stop(inetstate_stats_cb callback);

gboolean connui_inetstate_status(inetstate_cb callback, gpointer user_data);
void connui_inetstate_close(inetstate_cb callback);

#endif // CONNUI_INETSTATE_H
