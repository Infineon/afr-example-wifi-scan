/******************************************************************************
* File Name: wifi_scan.c
*
* Description: This is the source code for CE229265 - Amazon FreeRTOS for PSoC 6
*              MCU - Wi-Fi Scan
*
* Related Document: See README.md
*
*******************************************************************************
* (c) 2019, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "aws_demo_config.h"
#include "task.h"
#include "iot_wifi.h"
#include "iot_threads.h"
#include <stdbool.h>


/*******************************************************************************
* Macros
********************************************************************************/
#define APP_TASK_STACK_SIZE                 (configMINIMAL_STACK_SIZE * 8)
#define APP_TASK_PRIORITY                   tskIDLE_PRIORITY

/* Maximum number of Wi-Fi networks to scan */
#define SCAN_NETWORK_COUNT                  (100u)

/* Delay between successive scans */
#define WIFI_SCAN_LOOP_DELAY_MS             (5000u)


/*******************************************************************************
* Global Variables
********************************************************************************/
/* Names used for printing the security type of a network */
const char *security_names[] =
{
    "Open",             /* Open - No Security. */
    "WEP",              /* WEP Security. */
    "WPA",              /* WPA Security. */
    "WPA2",             /* WPA2 Security. */
    "WPA2 Enterprise",  /* WPA2 Enterprise Security */
    "Unknown"           /* Unknown Security. */
};

/* Stores the Wi-Fi scan results */
static WIFIScanResult_t xScanResults[SCAN_NETWORK_COUNT];

void RunApplication(void * pArgument);


/*******************************************************************************
* Function Name: InitApplication
********************************************************************************
* Summary:
*  Initializes all the tasks, queues, and other resources required to run the
*  application.
*
*******************************************************************************/
void InitApplication(void)
{
    /* Set up the application task */
    Iot_CreateDetachedThread(RunApplication, NULL, APP_TASK_PRIORITY, APP_TASK_STACK_SIZE);
}


/*******************************************************************************
* Function Name: ApplicationTask
********************************************************************************
* Summary:
*  Scans and prints the Wi-Fi networks. This function runs as a FreeRTOS task.
*
* Parameters:
*  pArgument - pointer to the argument to this task.
*
*******************************************************************************/
void RunApplication(void * pArgument)
{
    uint32_t scanCount;
    const char *security_name;

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    /* Turn the Wi-Fi on first */
    WIFIReturnCode_t wifiStatus = WIFI_On();

    printf( "\nStarting Wi-Fi Scan Task...\n");

    if(wifiStatus == eWiFiSuccess)
    {
        while(true)
        {
            /* Initialize the scan result array with zeros */
            memset(xScanResults, 0x00, sizeof(xScanResults));

            /* Perform Wi-Fi scan */
            wifiStatus = WIFI_Scan(xScanResults, SCAN_NETWORK_COUNT);

            if(wifiStatus == eWiFiSuccess)
            {
                /* Print the results */
                printf( ("\n  -------------------------------------------------------------------------------\n") );
                printf( ("  #                  SSID                  RSSI   Channel      Security\n") );
                printf( ("  -------------------------------------------------------------------------------\n") );

                for(scanCount = 0; scanCount < SCAN_NETWORK_COUNT; scanCount++)
                {
                    if(xScanResults[scanCount].cSSID[0] == '\0')
                    {
                        break;
                    }

                    /* To prevent out of bound access of the security_names[] array */
                    security_name = security_names[eWiFiSecurityNotSupported];
                    if(xScanResults[scanCount].xSecurity < eWiFiSecurityNotSupported)
                    {
                        security_name = security_names[xScanResults[scanCount].xSecurity];
                    }

                    printf(" %2d   %-32s   %4d   %2d            %-20s\n",
                            (int)(scanCount + 1), xScanResults[scanCount].cSSID,
                            xScanResults[scanCount].cRSSI, xScanResults[scanCount].cChannel,
                            security_name);
                }
            }
            else
            {
                printf("\nWi-Fi scan returned error: 0x%x\n", wifiStatus);
            }

            vTaskDelay(pdMS_TO_TICKS(WIFI_SCAN_LOOP_DELAY_MS));
        }
    }
    else
    {
        printf("\nWi-Fi ON returned error: 0x%x\n", wifiStatus);
    }
}


/* [] END OF FILE */

