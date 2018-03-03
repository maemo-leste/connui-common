/*
  osso-wlan-dev OSSO WLAN connectivity header files
  Copyright (C) 2005 Nokia Corporation
  
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
  @file wlancond.h
 
  Copyright (C) 2004-2008 Nokia. All rights reserved.
 
  @author Janne Ylälehto <janne.ylalehto@nokia.com>
*/

#ifndef _WLANCOND_H_
#define _WLANCOND_H_

/* Network mode */

#define WLANCOND_MODE_MASK  0x00000007
#define WLANCOND_INFRA (1 << 0)
#define WLANCOND_ADHOC (1 << 1)
#define WLANCOND_AUTO  (1 << 2)

/* Encryption */

#define WLANCOND_ENCRYPT_METHOD_MASK 0xF0
#define WLANCOND_OPEN     (1 << 4)
#define WLANCOND_WEP      (1 << 5)
#define WLANCOND_WPA_PSK  (1 << 6)
#define WLANCOND_WPA_EAP  (1 << 7)

#define WLANCOND_ENCRYPT_WPA2_MASK 0x100
#define WLANCOND_WPA2     (1 << 8)

/* Protected setup support */

#define WLANCOND_WPS_MASK 0x1E00
#define WLANCOND_WPS             (1 << 9)
#define WLANCOND_WPS_PIN         (1 << 10)
#define WLANCOND_WPS_PUSH_BUTTON (1 << 11)
#define WLANCOND_WPS_CONFIGURED  (1 << 12)

/* Cabability bits */

#define WLANCOND_RATE_MASK 0x0FFFE000
#define WLANCOND_RATE_540 (1 << 25)
#define WLANCOND_RATE_260 (1 << 24)
#define WLANCOND_RATE_180 (1 << 23)
#define WLANCOND_RATE_120 (1 << 22)
#define WLANCOND_RATE_110 (1 << 21)
#define WLANCOND_RATE_90  (1 << 20)
#define WLANCOND_RATE_60  (1 << 19)
#define WLANCOND_RATE_55  (1 << 18)
#define WLANCOND_RATE_20  (1 << 17)
#define WLANCOND_RATE_10  (1 << 16)

#define WLANCOND_RATE_480 (1 << 15)
#define WLANCOND_RATE_360 (1 << 14)
#define WLANCOND_RATE_240 (1 << 13)

/* Unsupported network */

#define WLANCOND_UNSUPPORTED_NETWORK_MASK 0x8000000
#define WLANCOND_UNSUPPORTED_NETWORK      (1 << 27)

/* In settings_and_connect we need algorithm mask */

#define WLANCOND_ENCRYPT_ALG_MASK 0x30000000
#define WLANCOND_WPA_TKIP         (1 << 28) //WPA with TKIP as unicast cipher
#define WLANCOND_WPA_AES          (1 << 29) //WPA with AES as unicast cipher
#define WLANCOND_ENCRYPT_GROUP_ALG_MASK 0xc0000000
#define WLANCOND_WPA_TKIP_GROUP   (1 << 30) //WPA with TKIP as group cipher
#define WLANCOND_WPA_AES_GROUP    (1 << 31) //WPA with AES as group cipher

/* Power level */

#define WLANCOND_TX_POWER10  (1 << 2)
#define WLANCOND_TX_POWER100 (1 << 3)

/* Scan definitions */

#define WLANCOND_NO_DELAYED_SHUTDOWN (1 << 0)
#define WLANCOND_PASSIVE_SCAN        (1 << 1)

/* Power save definitions and other flags*/

#define WLANCOND_DISABLE_POWERSAVE (1 << 0)
#define WLANCOND_MINIMUM_POWERSAVE (1 << 1)
#define WLANCOND_MAXIMUM_POWERSAVE (1 << 2)
#define WLANCOND_AUTOCONNECT       (1 << 3)

/* Some restrictions */

#define WLANCOND_MAX_SSID_SIZE    IW_ESSID_MAX_SIZE

#define WLANCOND_MIN_KEY_LEN 5    // 5 chars (40 bits)
#define WLANCOND_MAX_KEY_LEN 13   // 13 chars (104 bits)

#define WLANCOND_MIN_WLAN_CHANNEL 1
#define WLANCOND_MAX_WLAN_CHANNEL 13

#define WLANCOND_PMKID_LEN          16

/* Cipher suites */

#define CIPHER_SUITE_NONE   (1 << 0)
#define CIPHER_SUITE_WEP40  (1 << 1)
#define CIPHER_SUITE_WEP104 (1 << 2)
#define CIPHER_SUITE_TKIP   (1 << 3)
#define CIPHER_SUITE_CCMP   (1 << 4)

#endif
