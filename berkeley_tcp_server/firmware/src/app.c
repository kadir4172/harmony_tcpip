/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
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

#include "app.h"

#include "tcpip/tcpip.h"

#include <errno.h>
#include <sys/errno.h>

#define SERVER_PORT 9760
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback funtions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_TCPIP_WAIT_INIT;
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    SYS_STATUS          tcpipStat;
    const char          *netName, *netBiosName;
    static IPV4_ADDR    dwLastIP[2] = { {-1}, {-1} };
    IPV4_ADDR           ipAddr;
    int                 i, nNets;
    TCPIP_NET_HANDLE    netH;


    SYS_CMD_READY_TO_READ();
    switch(appData.state)
    {
        case APP_TCPIP_WAIT_INIT:
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if(tcpipStat < 0)
            {   // some error occurred
                SYS_CONSOLE_MESSAGE(" APP: TCP/IP stack initialization failed!\r\n");
                appData.state = APP_TCPIP_ERROR;
            }
            else if(tcpipStat == SYS_STATUS_READY)
            {
                // now that the stack is ready we can check the 
                // available interfaces
                nNets = TCPIP_STACK_NumberOfNetworksGet();

                for(i = 0; i < nNets; i++)
                {

                    netH = TCPIP_STACK_IndexToNet(i);
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);

#if defined(TCPIP_STACK_USE_NBNS)
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS enabled\r\n", netName, netBiosName);
#else
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS disabled\r\n", netName, netBiosName);
#endif  // defined(TCPIP_STACK_USE_NBNS)

                }

                appData.state = APP_TCPIP_WAIT_FOR_IP;
            }

            break;
 
        case APP_TCPIP_WAIT_FOR_IP:

            // if the IP address of an interface has changed
            // display the new value on the system console
            nNets = TCPIP_STACK_NumberOfNetworksGet();

            for (i = 0; i < nNets; i++)
            {
                netH = TCPIP_STACK_IndexToNet(i);
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);
                if(dwLastIP[i].Val != ipAddr.Val)
                {
                    dwLastIP[i].Val = ipAddr.Val;

                    SYS_CONSOLE_MESSAGE(TCPIP_STACK_NetNameGet(netH));
                    SYS_CONSOLE_MESSAGE(" IP Address: ");
                    SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                    if (ipAddr.v[0] != 0 && ipAddr.v[0] != 169) // Wait for a Valid IP
                    {
                        appData.state = APP_BSD_INIT;
                    }
                }
            }
            break;
        case APP_BSD_INIT:
        {
        	// Initialize all client socket handles so that we don't process
        	// them in the BSD_OPERATION state
        	for(i = 0; i < MAX_CLIENT; i++)
        		appData.ClientSock[i] = INVALID_SOCKET;

        	appData.state = APP_BSD_CREATE_SOCKET;

        }
        break;

        case APP_BSD_CREATE_SOCKET:
        {
            // Create a socket for this server to listen and accept connections on
            SOCKET tcpSkt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(tcpSkt == INVALID_SOCKET)
                return;
            appData.bsdServerSocket = (SOCKET)tcpSkt;

            appData.state = APP_BSD_BIND;
        }
        break;

        case APP_BSD_BIND:
        {
            // Bind socket to a local port
            struct sockaddr_in addr;
            int addrlen = sizeof(struct sockaddr_in);
            addr.sin_port = SERVER_PORT;
            addr.sin_addr.S_un.S_addr = IP_ADDR_ANY;
            if( bind( appData.bsdServerSocket, (struct sockaddr*)&addr, addrlen ) == SOCKET_ERROR )
                return;

            appData.state = APP_BSD_LISTEN;
            // No break needed
        }
        break;

        case APP_BSD_LISTEN:
        {
            if(listen(appData.bsdServerSocket, MAX_CLIENT) == 0)
            {
	        appData.state = APP_BSD_OPERATION;
                SYS_CONSOLE_PRINT("Waiting for Client Connection on port: %d\r\n", SERVER_PORT);
            }
        }
        break;
        case APP_BSD_OPERATION:
        {
            int length;
            struct sockaddr_in addRemote;
            int addrlen = sizeof(struct sockaddr_in);
            char bfr[15];

            for(i=0; i<MAX_CLIENT; i++)
            {
	            // Accept any pending connection requests, assuming we have a place to store the socket descriptor
                if(appData.ClientSock[i] == INVALID_SOCKET)
                    appData.ClientSock[i] = accept(appData.bsdServerSocket, (struct sockaddr*)&addRemote, &addrlen);

                // If this socket is not connected then no need to process anything
                if(appData.ClientSock[i] == INVALID_SOCKET)
                	continue;
                
	            // For all connected sockets, receive and send back the data
                length = recv( appData.ClientSock[i], bfr, sizeof(bfr), 0);

                if( length > 0 )
                {
                    bfr[length] = '\0';
                    send(appData.ClientSock[i], bfr, strlen(bfr), 0);
                }
                else if( length == 0 || errno != EWOULDBLOCK )
                {
                    closesocket( appData.ClientSock[i] );
                    appData.ClientSock[i] = INVALID_SOCKET;
                }
                // else just wait for some more data
            }
         }
        break;
        default:
            break;
    }
}
/*******************************************************************************
 End of File
 */

