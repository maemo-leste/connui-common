/*
 * connui-dbus-log.h
 *
 * Copyright (C) 2021 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef CONNUIDBUSLOG_H
#define CONNUIDBUSLOG_H


#define CONNUI_DBUS_ERR(mcall) \
do \
{ \
  DBusError error; \
\
  dbus_error_init(&error); \
\
  if (dbus_set_error_from_message(&error, mcall)) \
    CONNUI_ERR("Method call error occured, '%s' - '%s'", error.name, error.message); \
\
  dbus_error_free(&error); \
} while (0)

#endif /* CONNUIDBUSLOG_H */
