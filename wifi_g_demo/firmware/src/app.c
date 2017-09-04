/*******************************************************************************
  File Name:
    app.c

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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <sys/attribs.h>
#include "app.h"
#include "app_wifi_mrf24w.h"

#if defined(TCPIP_IF_MRF24W)
#define WIFI_INTERFACE_NAME "MRF24W"
#endif

#if defined(TCPIP_STACK_USE_ZEROCONF_MDNS_SD)
#define IS_MDNS_RUN() true
#else
#define IS_MDNS_RUN() false
#define TCPIP_MDNS_ServiceRegister(a, b, c, d, e, f, g, h) do {} while (0)
#define TCPIP_MDNS_ServiceDeregister(a) do {} while (0)
#endif /* defined(TCPIP_STACK_USE_ZEROCONF_MDNS_SD) */

#define IS_WF_INTF(x) ((strcmp(x, "MRF24W") == 0) || (strcmp(x, "MRF24WN") == 0))

// *****************************************************************************
// *****************************************************************************
// Section: Global Variable Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */
static APP_DATA s_appData;
static BSP_LED_STATE s_LEDstate = BSP_LED_STATE_OFF;
static IWPRIV_GET_PARAM s_app_get_param;
static IWPRIV_SET_PARAM s_app_set_param;
static IWPRIV_EXECUTE_PARAM s_app_execute_param;

bool g_redirect_signal = false;
WF_CONFIG g_wifi_cfg;
WF_DEVICE_INFO g_wifi_deviceInfo;
WF_REDIRECTION_CONFIG g_redirectionConfig;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary local functions.
*/

static void APP_WIFI_RedirectionConfigInit(void);
static void APP_WIFI_IPv6MulticastFilter_Set(TCPIP_NET_HANDLE netH);
static void APP_WIFI_PowerSave_Config(bool enable);
static void APP_WIFI_DHCPS_Sync(TCPIP_NET_HANDLE netH);
static void APP_TCPIP_IFModules_Disable(TCPIP_NET_HANDLE netH);
static void APP_TCPIP_IFModules_Enable(TCPIP_NET_HANDLE netH);
static void APP_TCPIP_IF_Down(TCPIP_NET_HANDLE netH);
static void APP_TCPIP_IF_Up(TCPIP_NET_HANDLE netH);
static void APP_WIFIG_SSID_Set(TCPIP_NET_HANDLE netH);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize(void)

  Remarks:
    See prototype in app.h.
 */
void APP_Initialize(void)
{
	/* Following PORT initialization functions are for the 3 LEDs */
	/* PORT E Initialization */
	PLIB_PORTS_OpenDrainEnable(PORTS_ID_0, PORT_CHANNEL_E, SYS_PORT_E_ODC);
	PLIB_PORTS_Toggle(PORTS_ID_0, PORT_CHANNEL_E, SYS_PORT_E_LAT);
	//PLIB_PORTS_DirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_E, SYS_PORT_E_TRIS ^ 0xFFFF);
	PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_E, 0);

	/* PORT F Initialization */
	PLIB_PORTS_OpenDrainEnable(PORTS_ID_0, PORT_CHANNEL_F, SYS_PORT_F_ODC);
	PLIB_PORTS_Toggle(PORTS_ID_0, PORT_CHANNEL_F, SYS_PORT_F_LAT);
	//PLIB_PORTS_DirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_F, SYS_PORT_F_TRIS ^ 0xFFFF);
	PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_F, 0);
	PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_F, 1);

	s_appData.state = APP_MOUNT_DISK;
	s_appData.scanState = APP_WIFI_PRESCAN_INIT;

	s_app_set_param.conn.initConnAllowed = false;
	iwpriv_set(INITCONN_OPTION_SET, &s_app_set_param);
	s_app_set_param.scan.prescanAllowed = true;
	iwpriv_set(PRESCAN_OPTION_SET, &s_app_set_param);
}

static bool disk_mount(void)
{
	if (SYS_FS_Mount(SYS_FS_NVM_VOL, LOCAL_WEBSITE_PATH_FS, MPFS2, 0, NULL) == 0) {
		SYS_CONSOLE_PRINT("SYS_Initialize: The %s File System is mounted\r\n", SYS_FS_MPFS_STRING);
		s_appData.state = APP_TCPIP_WAIT_INIT;
		return true;
	}
	return false;
}

static bool tcpip_init_wait(void)
{
	SYS_STATUS tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);

	if (tcpipStat < 0) {
		SYS_CONSOLE_MESSAGE("APP: TCP/IP stack initialization failed!\r\n");
		s_appData.state = APP_TCPIP_ERROR;
		return true;
	} else if (tcpipStat == SYS_STATUS_READY) {
		s_appData.state = APP_WIFI_CONFIG;
		return true;
	}
	return false;
}

static bool wifi_config(IPV4_ADDR *defaultIpWiFi, TCPIP_NET_HANDLE *netHandleWiFi)
{
	/*
	 * Following "if condition" is useless when demo firstly
	 * boots up, since stack's status has already been checked in
	 * APP_TCPIP_WAIT_INIT. But it is necessary in redirection or
	 * Wi-Fi interface reset due to connection errors.
	 */
	iwpriv_get(DRVSTATUS_GET, &s_app_get_param);
	if (s_app_get_param.driverStatus.isOpen) {
		// get necessary Wi-Fi interface information
		s_app_get_param.devInfo.info = &g_wifi_deviceInfo;
		iwpriv_get(DEVICEINFO_GET, &s_app_get_param);
		*netHandleWiFi = TCPIP_STACK_NetHandleGet(WIFI_INTERFACE_NAME);
		defaultIpWiFi->Val = TCPIP_STACK_NetAddress(*netHandleWiFi);

		// initialize redirection variable
		APP_WIFIG_SSID_Set(*netHandleWiFi);
		APP_WIFI_RedirectionConfigInit();

		// reset flag used in redirection command
		if (g_redirect_signal)
			g_redirect_signal = false;

		s_appData.state = APP_WIFI_PRESCAN;
		return true;
	}
	return false;
}

static bool wifi_prescan(void)
{
	// if pre-scan option is set to false,
	// this state would just run once and pass,
	// APP_WIFI_Prescan() function would not actually
	// do anything
	uint8_t scanStatus = APP_WIFI_Prescan();

	if (scanStatus == IWPRIV_READY) {
		s_appData.state = APP_TCPIP_MODULES_ENABLE;
		return true;
	} else if (scanStatus == IWPRIV_ERROR) {
		SYS_CONSOLE_MESSAGE("Wi-Fi Prescan Error\r\n");
		s_appData.state = APP_TCPIP_MODULES_ENABLE;
		return true;
	}
	return false;
}

static void tcpip_module_enable(void)
{
	int i, nNets;

	// check available interfaces
	nNets = TCPIP_STACK_NumberOfNetworksGet();
	for (i = 0; i < nNets; ++i)
		APP_TCPIP_IFModules_Enable(TCPIP_STACK_IndexToNet(i));
	s_appData.state = APP_TCPIP_TRANSACT;
}

static void network_run(TCPIP_NET_HANDLE *netHandleWiFi, IPV4_ADDR *defaultIpWiFi)
{
	int i, nNets;
	static bool isWiFiPowerSaveConfigured = false;
	static bool wasNetUp[2] = {true, true}; // this app supports 2 interfaces so far

	// wait for redirection command from custom_http_app.c
	if (g_redirect_signal) {
		APP_TCPIP_IFModules_Disable(*netHandleWiFi);
		APP_TCPIP_IF_Down(*netHandleWiFi);
		SYS_CONSOLE_MESSAGE("Connecting to target AP . . .\r\n");
		APP_TCPIP_IF_Up(*netHandleWiFi);
		isWiFiPowerSaveConfigured = false;
		s_appData.state = APP_WIFI_CONFIG;
		return;
	}

	iwpriv_get(CONNSTATUS_GET, &s_app_get_param);
	if (s_app_get_param.conn.status == IWPRIV_CONNECTION_FAILED) {
		APP_TCPIP_IFModules_Disable(*netHandleWiFi);
		APP_TCPIP_IF_Down(*netHandleWiFi);
		APP_TCPIP_IF_Up(*netHandleWiFi);
		isWiFiPowerSaveConfigured = false;
		s_appData.state = APP_WIFI_CONFIG;
		return;
	} else if (s_app_get_param.conn.status == IWPRIV_CONNECTION_REESTABLISHED) {
		// restart dhcp client and config power save
		TCPIP_DHCP_Disable(*netHandleWiFi);
		TCPIP_DHCP_Enable(*netHandleWiFi);
		isWiFiPowerSaveConfigured = false;
	}

	/*
	 * Following for loop is to deal with manually controlling
	 * interface down/up (for example, through console commands
	 * or web page).
	 */
	nNets = TCPIP_STACK_NumberOfNetworksGet();
	for (i = 0; i < nNets; ++i) {
		TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(i);
		if (!TCPIP_STACK_NetIsUp(netH) && wasNetUp[i]) {
			const char *netName = TCPIP_STACK_NetNameGet(netH);
			wasNetUp[i] = false;
			APP_TCPIP_IFModules_Disable(netH);
			if (IS_WF_INTF(netName))
				isWiFiPowerSaveConfigured = false;
		}

		if (TCPIP_STACK_NetIsUp(netH) && !wasNetUp[i]) {
			wasNetUp[i] = true;
			APP_TCPIP_IFModules_Enable(netH);
		}
	}

	/*
	 * If we get a new IP address that is different than the default one,
	 * we will run PowerSave configuration.
	 */
	if (!isWiFiPowerSaveConfigured &&
		TCPIP_STACK_NetIsUp(*netHandleWiFi) &&
		(TCPIP_STACK_NetAddress(*netHandleWiFi) != defaultIpWiFi->Val)) {
		APP_WIFI_PowerSave_Config(true);
		isWiFiPowerSaveConfigured = true;
	}

	APP_WIFI_DHCPS_Sync(*netHandleWiFi);
}

static void led_toggle(void)
{
	static uint32_t startTick = 0;

	if (SYS_TMR_TickCountGet() - startTick >= SYS_TMR_TickCounterFrequencyGet() / 2ul) {
		startTick = SYS_TMR_TickCountGet();
		s_LEDstate ^= BSP_LED_STATE_ON;
		BSP_LEDStateSet(APP_TCPIP_LED_1, s_LEDstate);
	}
}

/*******************************************************************************
  Function:
    void APP_Tasks(void)

  Remarks:
    See prototype in app.h.
 */
void APP_Tasks(void)
{
	static IPV4_ADDR defaultIpWiFi = {-1};
	static TCPIP_NET_HANDLE netHandleWiFi = NULL;

	switch (s_appData.state) {
	case APP_MOUNT_DISK:
		if (!disk_mount())
			break;
	case APP_TCPIP_WAIT_INIT:
		if (!tcpip_init_wait())
			break;
	case APP_WIFI_CONFIG:
		if (!wifi_config(&defaultIpWiFi, &netHandleWiFi))
			break;
	case APP_WIFI_PRESCAN:
		if (!wifi_prescan())
			break;
	case APP_TCPIP_MODULES_ENABLE:
		tcpip_module_enable();
	case APP_TCPIP_TRANSACT:
		network_run(&netHandleWiFi, &defaultIpWiFi);
		led_toggle();
		break;
	default:
		break;
	}
}

uint8_t APP_WIFI_Prescan(void)
{
	switch (s_appData.scanState) {
		case APP_WIFI_PRESCAN_INIT:
			iwpriv_get(PRESCAN_OPTION_GET, &s_app_get_param);
			if (s_app_get_param.scan.prescanAllowed) {
				iwpriv_get(NETWORKTYPE_GET, &s_app_get_param);
				iwpriv_get(CONNSTATUS_GET, &s_app_get_param);
				if (s_app_get_param.netType.type == WF_NETWORK_TYPE_SOFT_AP &&
					s_app_get_param.conn.status == IWPRIV_CONNECTION_SUCCESSFUL)
					return IWPRIV_ERROR;
				iwpriv_execute(PRESCAN_START, &s_app_execute_param);
				s_appData.scanState = APP_WIFI_PRESCAN_WAIT;
				break;
			}
			return IWPRIV_READY;

		case APP_WIFI_PRESCAN_WAIT:
			iwpriv_get(PRESCAN_ISFINISHED_GET, &s_app_get_param);
			if (s_app_get_param.scan.prescanFinished) {
				iwpriv_get(SCANSTATUS_GET, &s_app_get_param);
				if (s_app_get_param.scan.scanStatus == IWPRIV_SCAN_SUCCESSFUL) {
					s_appData.scanState = APP_WIFI_PRESCAN_SAVE; // no break
				} else {
					s_appData.scanState = APP_WIFI_PRESCAN_INIT;
					return IWPRIV_ERROR;
				}
			} else {
				break;
			}

		case APP_WIFI_PRESCAN_SAVE:
			iwpriv_execute(SCANRESULTS_SAVE, &s_app_execute_param);
			if (s_app_execute_param.scan.saveStatus == IWPRIV_IN_PROGRESS)
				break;
			else // IWPRIV_READY
				s_appData.scanState = APP_WIFI_PRESCAN_RESET;

        case APP_WIFI_PRESCAN_RESET:
			{
				TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet(WIFI_INTERFACE_NAME);
				APP_TCPIP_IF_Down(netH);
				APP_TCPIP_IF_Up(netH);
				s_app_set_param.conn.initConnAllowed = true;
				iwpriv_set(INITCONN_OPTION_SET, &s_app_set_param);
				s_app_set_param.scan.prescanAllowed = false;
				iwpriv_set(PRESCAN_OPTION_SET, &s_app_set_param);
				s_appData.scanState = APP_WIFI_PRESCAN_WAIT_RESET;
			}

		case APP_WIFI_PRESCAN_WAIT_RESET:
			iwpriv_get(DRVSTATUS_GET, &s_app_get_param);
			if (s_app_get_param.driverStatus.isOpen)
				s_appData.scanState = APP_WIFI_PRESCAN_DONE; // no break
			else
				break;

		case APP_WIFI_PRESCAN_DONE:
			s_appData.scanState = APP_WIFI_PRESCAN_INIT;
			return IWPRIV_READY;
	}
	return IWPRIV_IN_PROGRESS;
}

/*******************************************************************************
  Function:
    static void APP_WIFI_RedirectionConfigInit(void)

  Remarks:
    Initialize redirection configuration variable
 */
static void APP_WIFI_RedirectionConfigInit(void)
{
    g_redirectionConfig.ssid[0] = 0;
    g_redirectionConfig.securityMode = WF_SECURITY_OPEN;
    g_redirectionConfig.securityKey[0] = 0;
    g_redirectionConfig.wepKeyIndex = WF_WEP_KEY_INVALID;
    g_redirectionConfig.networkType = WF_NETWORK_TYPE_INFRASTRUCTURE;
}

static void APP_WIFI_IPv6MulticastFilter_Set(TCPIP_NET_HANDLE netH)
{
#if defined(TCPIP_STACK_USE_IPV6)
    const uint8_t *pMacAddr = TCPIP_STACK_NetAddressMac(netH);
    uint8_t solicitedNodeMulticastMACAddr[] = {0x33, 0x33, 0xff, 0x00, 0x00, 0x00};
    uint8_t allNodesMulticastMACAddr[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01};
	int i;

    for (i = 3; i < 6; i++)
        solicitedNodeMulticastMACAddr[i] = pMacAddr[i];

    s_app_set_param.multicast.addr = solicitedNodeMulticastMACAddr;
    iwpriv_set(MULTICASTFILTER_SET, &s_app_set_param);
    s_app_set_param.multicast.addr = allNodesMulticastMACAddr;
    iwpriv_set(MULTICASTFILTER_SET, &s_app_set_param);
#endif /* defined(TCPIP_STACK_USE_IPV6) */
}

static void APP_WIFI_PowerSave_Config(bool enable)
{
#if WF_DEFAULT_POWER_SAVE == WF_ENABLED
    s_app_set_param.powerSave.enabled = enable;
    iwpriv_set(POWERSAVE_SET, &s_app_set_param);
#endif
}

static void APP_WIFI_DHCPS_Sync(TCPIP_NET_HANDLE netH)
{
#if defined(TCPIP_STACK_USE_DHCP_SERVER)
    bool updated;
    TCPIP_MAC_ADDR addr;

    s_app_get_param.clientInfo.addr = addr.v;
    iwpriv_get(CLIENTINFO_GET, &s_app_get_param);
    updated = s_app_get_param.clientInfo.updated;

    if (updated)
        TCPIP_DHCPS_LeaseEntryRemove(netH, (TCPIP_MAC_ADDR *)&addr);
#endif /* defined(TCPIP_STACK_USE_DHCP_SERVER) */
}

static void APP_TCPIP_IFModules_Disable(TCPIP_NET_HANDLE netH)
{
	const char *netName = TCPIP_STACK_NetNameGet(netH);

	if (IS_WF_INTF(netName) && TCPIP_STACK_NetIsUp(netH))
		APP_WIFI_PowerSave_Config(false);
	TCPIP_DHCPS_Disable(netH);
	TCPIP_DHCP_Disable(netH);
	TCPIP_DNSS_Disable(netH);
	TCPIP_DNS_Disable(netH, true);
	TCPIP_MDNS_ServiceDeregister(netH);
}

static void APP_TCPIP_IFModules_Enable(TCPIP_NET_HANDLE netH)
{
	int netIndex = TCPIP_STACK_NetIndexGet(netH);
	const char *netName = TCPIP_STACK_NetNameGet(netH);

	/*
	 * If it's not Wi-Fi interface, then leave it to the TCP/IP stack
	 * to configure its DHCP server/client status.
	 */
	if (IS_WF_INTF(netName)) {
		iwpriv_get(OPERATIONMODE_GET, &s_app_get_param);
		if (s_app_get_param.opMode.isServer) {
			TCPIP_DHCP_Disable(netH); // must stop DHCP client first
			TCPIP_DHCPS_Enable(netH); // start DHCP server
			TCPIP_DNS_Disable(netH, true);
			TCPIP_DNSS_Enable(netH);
		} else {
			TCPIP_DHCPS_Disable(netH); // must stop DHCP server first
			TCPIP_DHCP_Enable(netH); // start DHCP client
			TCPIP_DNSS_Disable(netH);
			TCPIP_DNS_Enable(netH, TCPIP_DNS_ENABLE_DEFAULT);
		}
		APP_WIFI_IPv6MulticastFilter_Set(netH);
	}
	if (IS_MDNS_RUN()) {
		char mDNSServiceName[] = "MyWebServiceNameX "; // base name of the service Must not exceed 16 bytes long
		// the last digit will be incremented by interface
		mDNSServiceName[sizeof(mDNSServiceName) - 2] = '1' + netIndex;
		TCPIP_MDNS_ServiceRegister(netH, mDNSServiceName, "_http._tcp.local", 80, ((const uint8_t *)"path=/index.htm"),
			1, NULL, NULL);
	}
}

static void APP_TCPIP_IF_Down(TCPIP_NET_HANDLE netH)
{
    TCPIP_STACK_NetDown(netH);
}

static void APP_TCPIP_IF_Up(TCPIP_NET_HANDLE netH)
{
    SYS_MODULE_OBJ tcpipStackObj;
    TCPIP_STACK_INIT tcpip_init_data;
    const TCPIP_NETWORK_CONFIG *pIfConf;
    uint16_t net_ix = TCPIP_STACK_NetIndexGet(netH);

    tcpipStackObj = TCPIP_STACK_Initialize(0, 0);
    TCPIP_STACK_InitializeDataGet(tcpipStackObj, &tcpip_init_data);
    pIfConf = tcpip_init_data.pNetConf + net_ix;
    TCPIP_STACK_NetUp(netH, pIfConf);
}

static void APP_WIFIG_SSID_Set(TCPIP_NET_HANDLE netH)
{
    const uint8_t *mac;
    uint8_t ssid[32 + 1] = {0};
    uint8_t ssidLen;

    s_app_get_param.ssid.ssid = ssid;
    iwpriv_get(SSID_GET, &s_app_get_param);
    ssidLen = s_app_get_param.ssid.ssidLen;
    if (ssidLen == 32) return;
    mac = TCPIP_STACK_NetAddressMac(netH);
    if (strcmp((const char *)ssid, "MCHP_G_xxxx") == 0) {
        sprintf((char *)ssid, "MCHP_G_%02x%02x", mac[4], mac[5]);
        ssidLen = strlen((char *)ssid);
        s_app_set_param.ssid.ssid = ssid;
        s_app_set_param.ssid.ssidLen = ssidLen;
        iwpriv_set(SSID_SET, &s_app_set_param);
    }
}

/*******************************************************************************
 End of File
*/
