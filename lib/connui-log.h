#ifndef CONNUILOG_H
#define CONNUILOG_H

#include <osso-log.h>

#define CONNUI_ERR(FMT, ...) ULOG_ERR("%s(): " FMT, __FUNCTION__, __VA_ARGS__)

#endif // CONNUILOG_H
