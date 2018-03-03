/*
  osso-wlan-dev OSSO WLAN connectivity header files
  Copyright (C) 2005 - 2008 Nokia Corporation

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

/**
  @file dbus-wlan-names.h

  Copyright (C) 2004 - 2008 Nokia. All rights reserved.

  @author Janne Ylälehto <janne.ylalehto@nokia.com>
*/

#ifndef _DBUS_WLAN_NAMES_H_
#define _DBUS_WLAN_NAMES_H_

/* WLAN Connection Daemon D-Bus interface */
#define WLANCOND_SERVICE                      "com.nokia.wlancond"
#define WLANCOND_REQ_INTERFACE                "com.nokia.wlancond.request"
#define WLANCOND_SIG_INTERFACE                "com.nokia.wlancond.signal"
                                            
#define WLANCOND_SIG_PATH                     "/com/nokia/wlancond/signal"
#define WLANCOND_REQ_PATH                     "/com/nokia/wlancond/request"

#define WLANCOND_ERROR_FATAL                  "com.nokia.wlancond.error.fatal"
#define WLANCOND_ERROR_INVALID_PARAMS         "org.freedesktop.DBus.Error.InvalidArgs"
#define WLANCOND_ERROR_IOCTL_FAILED           "com.nokia.wlancond.error.ioctl"
#define WLANCOND_ERROR_INIT_FAILED            "com.nokia.wlancond.error.init"
#define WLANCOND_ERROR_OUT_OF_MEMORY          "com.nokia.wlancond.error.out_of_memory"
#define WLANCOND_ERROR_ALREADY_ACTIVE         "com.nokia.wlancond.error.already_active"
#define WLANCOND_ERROR_WLAN_DISABLED          "com.nokia.wlancond.error.wlan_disabled"
                                            
#define WLANCOND_SETTINGS_AND_CONNECT_REQ     "settings_and_connect"
#define WLANCOND_SCAN_REQ                     "scan"
#define WLANCOND_DISCONNECT_REQ               "disconnect"
#define WLANCOND_DISASSOCIATE_REQ             "disassociate"
#define WLANCOND_STATUS_REQ                   "status"
#define WLANCOND_CONNECTION_STATUS_REQ        "connection_status"
#define WLANCOND_INTERFACE_REQ                "interface"
#define WLANCOND_SET_PMKSA_REQ                "set_pmksa"
#define WLANCOND_SET_POWERSAVE_REQ            "set_powersave"
#define WLANCOND_IGNORE_COVER_REQ             "ignore_cover"


#define WLANCOND_CONNECTED_SIG                "connected"
#define WLANCOND_DISCONNECTED_SIG             "disconnected"
#define WLANCOND_SCAN_RESULTS_SIG             "scan_results"
#define WLANCOND_REGISTRAR_ERROR_SIG          "registrar_error"

#endif /* _DBUS_WLAN_NAMES_H_ */
