#include <gconf/gconf-client.h>

#include "iap-network.h"

void
iap_network_entry_clear(network_entry *network)
{
  if (!network)
    return;

  g_free(network->service_type);
  network->service_type = NULL;

  network->service_attributes = 0;

  g_free(network->service_id);
  network->service_id = NULL;

  g_free(network->network_type);
  network->network_type = NULL;

  network->network_attributes = 0;

  g_free(network->network_id);
  network->network_id = NULL;
}
