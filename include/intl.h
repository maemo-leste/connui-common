#ifndef INTL_H
#define INTL_H

#include <libintl.h>

#include "config.h"

#define _(msgid) dgettext(GETTEXT_PACKAGE, msgid)

#endif // INTL_H
