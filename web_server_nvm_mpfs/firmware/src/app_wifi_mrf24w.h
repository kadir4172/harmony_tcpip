/*******************************************************************************
  File Name:
    app_wifi_mrf24w.h

  Summary:


  Description:

 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

#ifndef _APP_WIFI_MRF24W_H
#define _APP_WIFI_MRF24W_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "driver/wifi/mrf24w/src/drv_wifi_config.h"
#include "driver/wifi/mrf24w/src/drv_wifi_iwpriv.h"

#define WF_DISABLED DRV_WIFI_DISABLED
#define WF_ENABLED DRV_WIFI_ENABLED

#define WF_NETWORK_TYPE_INFRASTRUCTURE DRV_WIFI_NETWORK_TYPE_INFRASTRUCTURE
#define WF_NETWORK_TYPE_ADHOC DRV_WIFI_NETWORK_TYPE_ADHOC
#define WF_NETWORK_TYPE_P2P 0xff /* Unsupported */
#define WF_NETWORK_TYPE_SOFT_AP DRV_WIFI_NETWORK_TYPE_SOFT_AP

#define WF_SECURITY_OPEN DRV_WIFI_SECURITY_OPEN
#define WF_SECURITY_WEP_40 DRV_WIFI_SECURITY_WEP_40
#define WF_SECURITY_WEP_104 DRV_WIFI_SECURITY_WEP_104
#define WF_SECURITY_WPA_WITH_KEY DRV_WIFI_SECURITY_WPA_WITH_KEY
#define WF_SECURITY_WPA_WITH_PASS_PHRASE DRV_WIFI_SECURITY_WPA_WITH_PASS_PHRASE
#define WF_SECURITY_WPA2_WITH_KEY DRV_WIFI_SECURITY_WPA2_WITH_KEY
#define WF_SECURITY_WPA2_WITH_PASS_PHRASE DRV_WIFI_SECURITY_WPA2_WITH_PASS_PHRASE
#define WF_SECURITY_WPA_AUTO_WITH_KEY DRV_WIFI_SECURITY_WPA_AUTO_WITH_KEY
#define WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE DRV_WIFI_SECURITY_WPA_AUTO_WITH_PASS_PHRASE
#define WF_SECURITY_WPS_PUSH_BUTTON DRV_WIFI_SECURITY_WPS_PUSH_BUTTON
#define WF_SECURITY_WPS_PIN DRV_WIFI_SECURITY_WPS_PIN

#define WF_DEFAULT_ADHOC_HIDDEN_SSID DRV_WIFI_DEFAULT_ADHOC_HIDDEN_SSID
#define WF_DEFAULT_ADHOC_BEACON_PERIOD DRV_WIFI_DEFAULT_ADHOC_BEACON_PERIOD
#define WF_DEFAULT_ADHOC_MODE DRV_WIFI_DEFAULT_ADHOC_MODE

#define WF_DEFAULT_POWER_SAVE DRV_WIFI_DEFAULT_POWER_SAVE

#define WF_WEP_KEY_INVALID 0xff

#define WF_ASSERT(condition, msg) DRV_WIFI_ASSERT(condition, msg)

typedef DRV_WIFI_ADHOC_NETWORK_CONTEXT WF_ADHOC_NETWORK_CONTEXT;
typedef DRV_WIFI_CONFIG WF_CONFIG;
typedef DRV_WIFI_DEVICE_INFO WF_DEVICE_INFO;
typedef DRV_WIFI_SCAN_RESULT WF_SCAN_RESULT;

#endif /* _APP_WIFI_MRF24W_H */

/*******************************************************************************
 End of File
 */
