/**
 * @author Latyr
 * @email lfall@sig-num.com
 * @create date 2020-01-31 14:51:36
 * @modify date 2020-01-31 14:51:36
 * @desc [description]
 */

#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <string.h>
#include "mySerial.h"

#include "app_utils.h"

extern mySerial pCom;
extern bool waiting;

const char* usb_dev;

const char * app_cmdStr [APP_CMD_MAX] = {
    [APP_CMD_UNKNOWN]           = STR_CMD_UNKNOWN,
    [APP_CMD_MONITOR]           = STR_CMD_MONITOR,
    [APP_CMD_READSN]            = STR_CMD_READSN,
    [APP_CMD_READFWINFO]        = STR_CMD_READFWINFO,
    [APP_CMD_READSESSIONIDS]    = STR_CMD_READSESSIONIDS,
    [APP_CMD_SETSN]             = STR_CMD_SETSN,
    [APP_CMD_FAKER]             = STR_CMD_FAKER,
    [APP_CMD_UNLOCK]            = STR_CMD_UNLOCK,
    [APP_CMD_ENABLE]            = STR_CMD_ENABLE,
    [APP_CMD_REBOOT]            = STR_CMD_REBOOT,
    [APP_CMD_REBOOT_PAD]   		= STR_CMD_REBOOT_PAD,
    [APP_CMD_REBOOT_QUAD]   	= STR_CMD_REBOOT_QUAD,
    [APP_CMD_FORCE_SELECT_RA]   = STR_CMD_FORCE_SELECT_RA,
    [APP_CMD_FORCE_SELECT_LA]   = STR_CMD_FORCE_SELECT_LA,
    [APP_CMD_FORCE_SELECT_LL]   = STR_CMD_FORCE_SELECT_LL,
    [APP_CMD_CHANGE_NOTCH]      = STR_CMD_NOTCH_FILTER,
    [APP_CMD_PAD_READSN]        = STR_CMD_PAD_READSN,
    [APP_CMD_PAD_WRITESN]       = STR_CMD_PAD_WRITESN,
    [APP_CMD_GET_ERROR]         = STR_CMD_GET_ERROR,
    [APP_CMD_PRINT_ERROR_REPORT]= STR_CMD_PRINT_ERROR_REPORT,
    [APP_CMD_GET_DPM_CLOCK]     = STR_CMD_GET_DPM_CLOCK,
    [APP_CMD_READ_HEARTBEAT]    = "",
    [APP_CMD_DSP_SAMPLE_SWITCH] = STR_CMD_DSP_SAMPLE_SWITCH,
    [APP_CMD_FAKE_CRC]          = STR_CMD_FAKE_CRC,
    [APP_CMD_FAKE_CRC_SN]       = STR_CMD_FAKE_CRC_SN,
    [APP_CMD_FAKE_CRC_F41]      = STR_CMD_FAKE_CRC_F41,
    [APP_CMD_FAKE_CRC_F42]      = STR_CMD_FAKE_CRC_F42,
    [APP_CMD_FAKE_F41_UPDATE]   = STR_CMD_FAKE_F41_UPDATE,
    [APP_CMD_FAKE_F42_UPDATE]   = STR_CMD_FAKE_F42_UPDATE,
    [APP_CMD_FAKE_F7_WDT_REFRESH]   = STR_CMD_FAKE_F7_WDT_REFRESH,
    [APP_CMD_FAKE_F41_WDT_REFRESH]  = STR_CMD_FAKE_F41_WDT_REFRESH,
    [APP_CMD_FAKE_F42_WDT_REFRESH]  = STR_CMD_FAKE_F42_WDT_REFRESH,
    [APP_CMD_FAKE_STOP_PAD2DSP_COM]	= STR_CMD_FAKE_STOP_PAD2DSP_COM,
	[APP_CMD_FAKE_STOP_F72PAD_COM]	= STR_CMD_FAKE_STOP_F72PAD_COM,
 	[APP_CMD_FAKE_STOP_DSP2F7_DATA_COM]	= STR_CMD_FAKE_STOP_DSP2F7_DATA_COM,
    [APP_CMD_FAKE_STOP_DSP2F7_MSG_COM]	= STR_CMD_FAKE_STOP_DSP2F7_MSG_COM,
	[APP_CMD_FAKE_DISABLE_F4_CONVERSION]	= STR_CMD_FAKE_DISABLE_F4_CONVERSION,
	[APP_CMD_FAKE_ENABLE_F4_CONVERSION]	    = STR_CMD_FAKE_ENABLE_F4_CONVERSION,
};

bool openComPort ()
{
    int result;
    
	std::string usb_name;
	printf("Available Ports:\n");
	result = system("ls -l /dev/ttyUSB*");
	if (result != 0)
    {
		std::cout << "\n no USB devices were found\n" << std::endl;
		return false;
	}
	else 
	{
		/*printf("Enter COM port [/dev/ttyUSB*]: \n");
		std::cin >> usb_name;
		usb_dev = usb_name.c_str();*/
		usb_dev = "/dev/ttyUSB0";
        printf ("Opening %s\n", usb_dev);
	}
   
    pCom.setDeviceName (usb_dev);
    pCom.setBaud (921600);
    pCom.Open();

    return pCom.IsOpen();
}

void periodicSendTest ()
{
    printf("\ntask2: started\n");
    unsigned char testString [32] = "ReadSerialNumber ";
    auto pTStart = std::chrono::system_clock::now();
    auto pTEnd   = std::chrono::system_clock::now();
    std::chrono::duration <double> pTDiff = pTEnd - pTStart;
    uint8_t inc = 0;
    bool done = false;
    while (1)
    {   
        //send every 5 secs for testing purpose
        while (pTDiff.count() <= 5)
        {    
            pTEnd   = std::chrono::system_clock::now();
            pTDiff  = pTEnd - pTStart;
        }
        if (!pCom.IsOpen()) continue;
        char strinc[2];
        if (inc % 2 == 0) sprintf(strinc,"%d", inc);
        else sprintf(strinc,"%c", '-');
        //strcat((char*)testString, "ABCDEFGHI");
        //strcat((char*)testString, strinc);
        strcat((char*)testString, ";");
        auto pTStart1 = std::chrono::system_clock::now();
        //if (done) return;
        for (int h = 0; h < strlen((const char*)testString) + 1; h++)
        {
            bool res = pCom.Send(testString + h, 1);
            TxDelay();
        }
        done = true;
        //ACK_req = true;
        //memcpy(sentStr, testString, sizeof(sentStr));
        auto pTEnd1 = std::chrono::system_clock::now();
        std::chrono::duration <double> pTDiff1 = pTEnd1 - pTStart1;
        int sz = strlen((const char*)testString);
        std::cout << "Time to send string of " 
                  << sz << " bytes : " << pTDiff1.count() << " s\n";
        inc = (inc == 9) ? inc = 0 : inc + 1;
        printf("\nsent: %s\n\n", testString);
        pTStart = pTEnd = std::chrono::system_clock::now();
        pTDiff  = pTEnd - pTStart;
        int u = sizeof(testString) - 1;
        while (testString[u] != ' ')
            testString[u--] = 0;
    }
    printf("\ntask2: interrupted\n");
}

void TxDelay ()
{
    auto pTS = std::chrono::system_clock::now();
    auto pTE   = std::chrono::system_clock::now();
    std::chrono::duration <double> pTD = pTS - pTE;
    while (pTD.count() < 2.5e-3)
    {
        auto pTE   = std::chrono::system_clock::now();
        pTD = pTE - pTS;
    }
}

void testTask ()
{
    auto pTStart = std::chrono::system_clock::now();
    auto pTEnd   = std::chrono::system_clock::now();
    std::chrono::duration <double> pTDiff = pTEnd - pTStart;
    while(1)
    {
        while (pTDiff.count() <= 5)
        {    
            pTEnd   = std::chrono::system_clock::now();
            pTDiff  = pTEnd - pTStart;
        }
        printf("testTask -- 5 sec\n");
        pTStart = pTEnd = std::chrono::system_clock::now();
        pTDiff  = pTEnd - pTStart;
    }
}

void user_wait ()
{
    waiting = true;
    for (int y = 0; y < 800000000; y++) {if(!waiting)break;}
}
