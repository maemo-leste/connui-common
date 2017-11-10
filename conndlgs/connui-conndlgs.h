/**
  @file connui-conndlgs.h

  Copyright (C) 2011 Jonathan Wilson

  @author Jonathan wilson <jfwfreo@tpgi.com.au>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 as
  published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <icd/osso-ic-ui-dbus.h>

#define IAP_DIALOGS_PLUGIN_DEFINE(dialog, match) \
static gboolean \
iap_dialog_##dialog##_show(int iap_id, DBusMessage *message, \
                           iap_dialogs_showing_fn showing, \
                           iap_dialogs_done_fn done, \
                           void *libosso); \
static gboolean \
iap_dialog_##dialog##_cancel(DBusMessage *message); \
G_MODULE_EXPORT int \
g_module_check_init(GModule *module G_GNUC_UNUSED) \
{ \
  iap_dialog_register_service(ICD_UI_DBUS_INTERFACE, ICD_UI_DBUS_PATH); \
\
  return 0; \
} \
G_MODULE_EXPORT void \
g_module_unload(GModule *module G_GNUC_UNUSED) \
{ \
  iap_dialog_unregister_service(ICD_UI_DBUS_INTERFACE, ICD_UI_DBUS_PATH); \
} \
G_MODULE_EXPORT gboolean \
iap_dialogs_plugin_match(DBusMessage *message) \
{ \
  return dbus_message_is_method_call(message, ICD_UI_DBUS_INTERFACE, match); \
} \
G_MODULE_EXPORT gboolean \
iap_dialogs_plugin_show(void *iap_id, DBusMessage *message, \
                        void (*showing)(DBusMessage *), \
                        void (*done)(void *, gboolean), \
                        void *libosso) \
{ \
  g_return_val_if_fail(showing != NULL, FALSE); \
  g_return_val_if_fail(done != NULL, FALSE); \
  g_return_val_if_fail(libosso != NULL, FALSE); \
\
  return iap_dialog_##dialog##_show(iap_id, message, showing, done, libosso); \
} \
G_MODULE_EXPORT gboolean \
iap_dialogs_plugin_cancel(DBusMessage *message) \
{ \
  return iap_dialog_##dialog##_cancel(message); \
}

typedef void (*iap_dialogs_showing_fn)(DBusMessage *message);
typedef void (*iap_dialogs_done_fn)(int iap_id, gboolean unk);

typedef gboolean (*iap_dialogs_match_fn)(DBusMessage *message);
typedef gboolean (*iap_dialogs_show_fn)(int iap_id, DBusMessage *message, iap_dialogs_showing_fn showing, iap_dialogs_done_fn done, osso_context_t libosso);
typedef gboolean (*iap_dialogs_cancel_fn)(DBusMessage *message);

DBusConnection *iap_dialog_get_connection(DBusMessage *message);
void iap_dialog_register_service(gchar *service, gchar *path);
//iap_dialog_request_dialog
void iap_dialog_unregister_service(gchar *service, gchar *path);

