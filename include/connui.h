/**
  @file libconnui.h

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

#include <glib.h>
#include <dbus/dbus.h>
#include <gtk/gtk.h>
#include <libosso.h>
#include <gconf/gconf-client.h>

//libconnui
struct pixbuf_cache
{
	GHashTable *table1;
	GHashTable *table2;
	GtkIconTheme *icon_theme;
};

typedef void (*pixbuf_anim_cb) (gpointer user_data, GdkPixbuf *pixbuf);

struct pixbuf_anim
{
	GdkPixbufAnimation *animation;
	GdkPixbufAnimationIter *iterator;
	gpointer user_data;
	pixbuf_anim_cb callback;
	guint timeout;
};

typedef struct _ConnuiBoxView ConnuiBoxView;
typedef struct _ConnuiBoxViewClass ConnuiBoxViewClass;
typedef struct _ConnuiBoxViewPrivate ConnuiBoxViewPrivate;
struct _ConnuiBoxView
{
	GtkVBox parent;
	ConnuiBoxViewPrivate *priv;
};

struct _ConnuiBoxViewClass
{
	GtkVBoxClass parent;
	int unknown;
};

typedef struct _ConnuiCellRendererOperator ConnuiCellRendererOperator;
typedef struct _ConnuiCellRendererOperatorClass ConnuiCellRendererOperatorClass;

struct _ConnuiCellRendererOperator
{
	GtkCellRendererText parent;
	GtkCellRenderer *pixbuf;
	gchar *service_type;
	gchar *service_id;
	gchar *service_text;
};

struct _ConnuiCellRendererOperatorClass
{
	GtkCellRendererTextClass parent;
};

struct network_entry
{
	gchar *service_type;
	int service_attributes;
	gchar *service_id;
	gchar *network_type;
	int network_attributes;
	gchar *network_id;
};

struct scan_entry
{
	struct network_entry entry;
	guint timestamp;
	gchar *service_name;
	gint service_priority;
	gchar *network_name;
	gint network_priority;
	gint signal_strength;
	gchar *station_id;
	GtkTreeIter iterator;
	GSList *list;
};

struct network_stats
{
	guint time_active;
	gint signal_strength;
	guint tx_bytes;
	guint rx_bytes;
};
typedef void (*cellular_data_suspended_cb) (gboolean suspended,guint32 suspendcode,gpointer user_data);
typedef void (*devlock_cb) (gboolean locked, gpointer user_data);
typedef void (*display_event_cb) (gchar *state, gpointer user_data);
typedef void (*flightmode_cb) (gboolean offline, gpointer user_data);
typedef void (*inetstate_cb) (int state, struct network_entry *entry, gpointer user_data);
typedef void (*inetstate_stats_cb) (struct network_entry *entry,struct network_stats *stats, gpointer user_data);
typedef void (*dbus_watcher_cb) (gchar *name,gchar *old_owner,gchar *new_owner,gpointer user_data);
typedef void (*notify_cb) ();

GtkTreeModel *connui_box_view_get_model(ConnuiBoxView *object);
GType connui_box_view_get_type();
ConnuiBoxView *connui_box_view_new_with_model(GtkTreeModel *model);
GType connui_cell_renderer_operator_get_type();
ConnuiCellRendererOperator *connui_cell_renderer_operator_new();
void connui_cellular_data_suspended_close(cellular_data_suspended_cb callback);
gboolean connui_cellular_data_suspended_status(cellular_data_suspended_cb callback,gpointer user_data);
osso_return_t connui_dbus_activate_app(osso_context_t *osso, const gchar *application);
dbus_bool_t connui_dbus_application_activation(const char *path,const char *name,DBusObjectPathMessageFunction function,void *user_data);
void connui_dbus_close();
dbus_bool_t connui_dbus_connect_session_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_connect_session_path(const char *path, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_connect_system_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_connect_system_path(const char *path, DBusObjectPathMessageFunction function, void *user_data);
DBusMessage *connui_dbus_create_method_call(const char *bus_name, const char *path, const char *interface, const char *method, int first_arg_type, ...);
dbus_bool_t connui_dbus_disconnect_session_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_disconnect_session_path(DBusConnection *connection);
dbus_bool_t connui_dbus_disconnect_system_bcast_signal(const char *interface, DBusHandleMessageFunction function, void *user_data, const char *signal);
dbus_bool_t connui_dbus_disconnect_system_path(DBusConnection *connection);
dbus_bool_t connui_dbus_get_value_and_iterate(DBusMessageIter *iter, int type,void *value);
dbus_bool_t connui_dbus_libosso_application_activation(const char *path, const char *name, DBusObjectPathMessageFunction function, void *user_data);
DBusMessage *connui_dbus_recv_reply_session_mcall(DBusMessage *message);
DBusMessage *connui_dbus_recv_reply_system_mcall(DBusMessage *message);
dbus_bool_t connui_dbus_register_session_service(const char *path, const char *name, unsigned int flags, DBusObjectPathMessageFunction function, void *user_data);
dbus_bool_t connui_dbus_register_system_service(const char *path, const char *name, unsigned int flags, DBusObjectPathMessageFunction function, void *user_data);
gboolean connui_dbus_register_watcher(dbus_watcher_cb callback,gpointer user_data);
dbus_bool_t connui_dbus_send_session_mcall(DBusMessage *mcall, int timeout_milliseconds, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call);
dbus_bool_t connui_dbus_send_session_msg(DBusMessage *message);
dbus_bool_t connui_dbus_send_system_mcall(DBusMessage *mcall, int timeout_milliseconds, DBusPendingCallNotifyFunction notify, void *user_data, DBusPendingCall **call);
dbus_bool_t connui_dbus_send_system_msg(DBusMessage *message);
void connui_dbus_unregister_watcher(dbus_watcher_cb callback);
void connui_devicelock_close(devlock_cb callback);
gboolean connui_devicelock_status(devlock_cb callback,gpointer user_data);
void connui_display_event_close(display_event_cb callback);
gboolean connui_display_event_status(display_event_cb callback,gpointer user_data);
void connui_flightmode_close(flightmode_cb callback);
void connui_flightmode_off();
void connui_flightmode_off_confirm();
void connui_flightmode_on();
gboolean connui_flightmode_status(flightmode_cb callback,gpointer user_data);
void connui_inetstate_close(inetstate_cb callback);
gboolean connui_inetstate_statistics_start(guint interval,inetstate_stats_cb callback,gpointer user_data);
void connui_inetstate_statistics_stop(inetstate_stats_cb callback);
gboolean connui_inetstate_status(inetstate_cb callback,gpointer user_data);
void connui_pixbuf_anim_destroy(struct pixbuf_anim *anim);
struct pixbuf_anim *connui_pixbuf_anim_new(gchar *anim_name,int size);
struct pixbuf_anim *connui_pixbuf_anim_new_from_icons(int size,float rate,gchar *first_icon,...);
void connui_pixbuf_anim_start(struct pixbuf_anim *anim,gpointer user_data,pixbuf_anim_cb callback);
void connui_pixbuf_anim_stop(struct pixbuf_anim *anim);
void connui_pixbuf_cache_destroy(struct pixbuf_cache *cache);
GdkPixbuf *connui_pixbuf_cache_get(struct pixbuf_cache *cache,gchar *icon,gint size);
GdkPixbuf *connui_pixbuf_cache_get_with_flags(struct pixbuf_cache *cache,gchar *icon,gint size,guint flags);
struct pixbuf_cache *connui_pixbuf_cache_new();
GdkPixbuf *connui_pixbuf_load(gchar *name,gint size);
void connui_pixbuf_unref(GdkPixbuf *);
GType connui_scan_box_view_get_type();
GtkWidget *connui_scan_box_view_new_with_model(GtkTreeModel *model);
GSList *connui_utils_find_callback(GSList *list,notify_cb notify);
osso_context_t *connui_utils_inherit_osso_context(osso_context_t *context, gchar *name, gchar *version);
GSList *connui_utils_notify_add(GSList *list,notify_cb notify,gpointer user_data);
void connui_utils_notify_notify(GSList *list,gpointer first_arg,...);
GSList *connui_utils_notify_remove(GSList *list,notify_cb notify);
void connui_utils_reload_theme();
void connui_utils_unblank_display();
void iap_common_activate_iap(gchar *iap);
void iap_common_get_last_used_network(struct network_entry *network);
gboolean iap_common_get_preferred_service(gchar **preferred_type,gchar **preferred_id);
gchar *iap_common_get_service_gconf_path(gchar *service_type,gchar *service_id);
void iap_common_get_service_properties(gchar *service_type,gchar *service_id,gchar *first_property,...);
gint iap_common_get_signal_by_nw_level(gint level);
GtkWidget *iap_common_make_connection_entry(gchar *iap);
GtkWidget 
*iap_common_make_connection_entry_for_network(struct network_entry *entry);
GtkWidget *iap_common_make_connection_entry_with_type(gchar *iap,GtkWidget *image,GtkWidget *connectionentry);
GtkWidget 
*iap_common_make_connection_entry_with_type_for_network(struct network_entry *entry,GtkWidget *image,GtkWidget *connectionentry);
void iap_common_pack_to_hildon_button(GtkWidget *hbutton,GtkWidget *child,gboolean expand,gboolean fill);
void iap_common_set_close_response(GtkWidget *widget,gint response_id);
gboolean iap_common_set_last_used_network(struct network_entry *entry);
void iap_common_set_service_properties(gchar *service_type,gchar *service_id,gchar *service_text,GtkWidget *container,GtkWidget *label);
void iap_common_set_service_properties_for_iap(gchar *iap,GtkWidget *container);
void iap_common_set_service_properties_for_network(struct network_entry *entry,GtkWidget *container);
void iap_common_show_saved_not_found_banner(GtkWidget *widget);
void iap_network_entry_clear(struct network_entry *entry);
int iap_network_entry_compare(struct network_entry *network1,struct network_entry *network2);
gboolean iap_network_entry_connect(guint connection_flags,struct network_entry **entries);
gboolean iap_network_entry_disconnect(guint connection_flags,struct network_entry *entry);
struct network_entry *iap_network_entry_dup(struct network_entry *entry);
gboolean iap_network_entry_equal(gconstpointer a,gconstpointer b);
gboolean iap_network_entry_from_dbus_iter(DBusMessageIter *iter,struct network_entry entry);
guint iap_network_entry_hash(gconstpointer key);
gboolean iap_network_entry_is_saved(struct network_entry *entry);
int iap_network_entry_network_compare(struct network_entry *network1,struct network_entry *network2);
int iap_network_entry_service_compare(struct network_entry *network1,struct network_entry *network2);
gboolean iap_network_entry_to_dbus_iter(DBusMessageIter *iter,struct network_entry *entry);
gboolean iap_scan_add_scan_entry(struct scan_entry *entry,gboolean unk);
void iap_scan_close();
gint iap_scan_default_sort_func(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer user_data);
void iap_scan_free_scan_entry(struct scan_entry *entry);
//iap_scan_start
//iap_scan_start_for_network_types
void iap_scan_stop();
GtkListStore *iap_scan_store_create(GtkTreeIterCompareFunc sort,gpointer user_data);
GtkWidget *iap_scan_tree_create(GtkTreeIterCompareFunc sort,gpointer user_data);
GtkWidget *iap_scan_view_create(GtkWidget *scan_tree_view);
gchar *iap_settings_create_iap_id();
gchar *iap_settings_enum_to_gconf_type(int type);
int iap_settings_gconf_type_to_enum(gchar *type);
gchar *iap_settings_get_auto_connect();
GConfValue *iap_settings_get_gconf_value(gchar *iap,gchar *key);
gchar *iap_settings_get_iap_icon_name_by_id(gchar *iap);
gchar *iap_settings_get_iap_icon_name_by_network(struct network_entry *entry);
gchar *iap_settings_get_iap_icon_name_by_network_and_signal(struct network_entry *entry,gint signal);
//iap_settings_get_iap_list
gchar *iap_settings_get_name(gchar *iap);
gchar *iap_settings_get_name_by_network(struct network_entry *network,gchar *name1,gchar *name2);
gint iap_settings_get_search_interval();
gchar *iap_settings_get_wlan_ssid(gchar *iap);
gboolean iap_settings_iap_exists(gchar *iap_name,gchar *iap);
gboolean iap_settings_iap_is_easywlan(gchar *iap);
gboolean iap_settings_is_empty(gchar *iap);
gboolean iap_settings_is_iap_visible(gchar *iap);
gboolean iap_settings_is_iaptype_supported(gchar *type);
gboolean iap_settings_remove_iap(gchar *iap);
void iap_settings_set_gconf_value(gchar *iap,gchar *key,GConfValue *value);
gint iap_settings_wlan_txpower_get();
void open_log(gchar *ident,gboolean open);
gint wlan_common_gconf_wlan_security_to_capability(gchar *security);
gchar *wlan_common_get_iaptype_icon_name_by_capability(gint capability);
gchar *wlan_common_get_icon_name_by_saved(gboolean saved);
gchar *wlan_common_get_icon_name_by_strength(guint strength);
gchar *wlan_common_get_saved_icon_name_by_network(struct network_entry *network);
gchar *wlan_common_get_security_icon_name_by_network_attrs(gint attrs);
gint wlan_common_get_signal_by_rssi(gint rssi);
gboolean wlan_common_mangle_ssid(gchar *ssid,gint len);


//libconnui_cell
struct network_info
{
	char network_service_status;
	gchar *country_code;
	gchar *operator_code;
	gchar *operator_name;
	char umts_avail;
	char network_type;
};

struct network_state
{
	int network_reg_status;
	unsigned int lac;
	unsigned int cell_id;
	struct network_info *network;
	char supported_services;
	char network_signals_bar;
	char rat_name;
	char network_radio_state;
	int operator_name_type;
	gchar *operator_name;
	gchar *alternative_operator_name;
};
typedef void (*call_status_cb) (gboolean calls, gpointer user_data);
typedef void (*cs_status_cb) (gchar state, gpointer user_data);
typedef void (*net_status_cb) (struct network_state *state, gpointer user_data);
typedef void (*sim_status_cb) (guint status, gpointer user_data);
typedef void (*ssc_state_cb) (gchar *state, gpointer user_data);

void connui_cell_call_status_close(call_status_cb callback);
gboolean connui_cell_call_status_register(call_status_cb callback,gpointer user_data);
gboolean connui_cell_code_ui_cancel_dialog();
//connui_cell_code_ui_change_code
//connui_cell_code_ui_create_dialog
//connui_cell_code_ui_deactivate_simlock
//connui_cell_code_ui_destroy
//connui_cell_code_ui_error_note_type_to_text
//connui_cell_code_ui_init
//connui_cell_code_ui_is_sim_locked_with_error
//connui_cell_code_ui_set_current_code_active
//connui_cell_code_ui_update_sim_status
void connui_cell_cs_status_close(cs_status_cb callback);
gboolean connui_cell_cs_status_register(cs_status_cb callback,gpointer user_data);
//connui_cell_datacounter_close
//connui_cell_datacounter_register
//connui_cell_datacounter_reset
//connui_cell_datacounter_save
gboolean connui_cell_emergency_call();
GStrv connui_cell_emergency_get_numbers();
//connui_cell_net_cancel_list
//connui_cell_net_cancel_select
//connui_cell_net_cancel_service_call
//connui_cell_net_get_call_forwarding_enabled
//connui_cell_net_get_call_waiting_enabled
//connui_cell_net_get_caller_id_presentation
//connui_cell_net_get_current
guchar connui_cell_net_get_network_selection_mode(gint32 *error);
gchar *connui_cell_net_get_operator_name(struct network_info *network,gboolean longname,gint32 *error);
guchar connui_cell_net_get_radio_access_mode(gint32 *error);
gboolean connui_cell_net_is_activated(gint32 *error);
//connui_cell_net_list
//connui_cell_net_select
//connui_cell_net_set_call_forwarding_enabled
//connui_cell_net_set_call_waiting_enabled
//connui_cell_net_set_caller_id_presentation
//connui_cell_net_set_caller_id_presentation_bluez
gboolean connui_cell_net_set_radio_access_mode(guchar mode,gint32 *error);
void connui_cell_net_status_close(net_status_cb callback);
gboolean connui_cell_net_status_register(net_status_cb callback,gpointer user_data);
struct network_info *connui_cell_network_dup(struct network_info *info);
void connui_cell_network_free(struct network_info *info);
GType connui_cell_note_get_type();
//connui_cell_note_new_information
void connui_cell_reset_network();
//connui_cell_security_code_change
//connui_cell_security_code_close
//connui_cell_security_code_get_active
//connui_cell_security_code_get_enabled
//connui_cell_security_code_register
//connui_cell_security_code_set_active
//connui_cell_security_code_set_enabled
//connui_cell_security_code_set_enabled_internal
gboolean connui_cell_sim_deactivate_lock(gchar *code,gint32 *error);
gchar *connui_cell_sim_get_service_provider(guint32 *unknown,gint32 *error);
gboolean connui_cell_sim_is_locked(gint32 *error);
gboolean connui_cell_sim_is_network_in_service_provider_info(gint32 *error,guchar *provider);
void connui_cell_sim_status_close(sim_status_cb callback);
gboolean connui_cell_sim_status_register(sim_status_cb callback,gpointer user_data);
guint32 connui_cell_sim_verify_attempts_left(guint32 level,gint32 *error);
void connui_cell_ssc_state_close(ssc_state_cb callback);
gboolean connui_cell_ssc_state_register(ssc_state_cb callback,gpointer user_data);

//libconbtui
gchar *gateway_pin_random_digit_string(gint digits);

//libconnui_iapsettings
GtkWidget *iap_widgets_create_h22_entry();
const gchar *iap_widgets_h22_entry_get_text(GtkWidget *widget);
