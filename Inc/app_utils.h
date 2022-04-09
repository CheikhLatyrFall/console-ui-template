/**
 * @author Latyr
 * @email lfall@sig-num.com
 * @create date 2020-01-31 14:51:13
 * @modify date 2020-01-31 14:51:13
 * @desc [description]
 */

#ifndef _APP_UTILS_H
#define _APP_UTILS_H

#include <stdlib.h>

#define USB_LOG_PASSWORD_STR "SIGNUMASFGASFGASDFGLKJH1234567890"

#define STR_CMD_UNKNOWN         		"Unknown"
#define STR_CMD_MONITOR         		"UnlockLog"
#define STR_CMD_READSN          		"ReadSerialNumber"
#define STR_CMD_READSESSIONIDS  		"ReadSessionIDs"
#define STR_CMD_READFWINFO      		"ReadFirmwareInfo"
#define STR_CMD_SETSN           		"SerialNumber"
#define STR_CMD_FAKER           		"Faker"
#define STR_CMD_UNLOCK          		"UnlockInterface"
#define STR_CMD_ENABLE          		"EnableUSBPrint"
#define STR_CMD_REBOOT          		"Reboot"
#define STR_CMD_REBOOT_PAD				"RebootPAD"
#define STR_CMD_REBOOT_QUAD				"RebootQUAD"
#define STR_CMD_FORCE_SELECT_RA 		"ForceSelectionRA"
#define STR_CMD_FORCE_SELECT_LA 		"ForceSelectionLA"
#define STR_CMD_FORCE_SELECT_LL 		"ForceSelectionLL"
#define STR_CMD_NOTCH_FILTER    		"ChangeFilterSetting"
#define STR_CMD_PAD_READSN      		"ReadPADSerialNumber"
#define STR_CMD_PAD_WRITESN     		"WritePADSerialNumber"
#define STR_CMD_GET_ERROR       		"GetErrorStatus"
#define STR_CMD_PRINT_ERROR_REPORT      "PrintErrorReport"
#define STR_CMD_GET_DPM_CLOCK           "GetDPMClock"
#define STR_CMD_DSP_SAMPLE_SWITCH		"DspSampleSwitch"

#define STR_CMD_FAKE_CRC        		"FakeCRCCalculation"
#define STR_CMD_FAKE_CRC_F41    		"FakeCRCF41"
#define STR_CMD_FAKE_CRC_F42    		"FakeCRCF42"
#define STR_CMD_FAKE_CRC_SN     		"FakeCRCSettings"
#define STR_CMD_FAKE_F41_UPDATE 		"FakeSkipF41SensorUpdate"
#define STR_CMD_FAKE_F42_UPDATE 		"FakeSkipF42SensorUpdate"
#define STR_CMD_FAKE_F7_WDT_REFRESH     "FakeSkipF7WdtRefresh"
#define STR_CMD_FAKE_F41_WDT_REFRESH    "FakeSkipF41WdtRefresh"
#define STR_CMD_FAKE_F42_WDT_REFRESH    "FakeSkipF42WdtRefresh"
#define STR_CMD_FAKE_STOP_PAD2DSP_COM	"FakeStopPAD2DSPCom"
#define STR_CMD_FAKE_STOP_F72PAD_COM	"FakeStopF72PADCom"
#define STR_CMD_FAKE_STOP_DSP2F7_DATA_COM	"FakeStopDSP2F7DataCom"
#define STR_CMD_FAKE_STOP_DSP2F7_MSG_COM    "FakeStopDSP2F7MsgCom"
#define STR_CMD_FAKE_DISABLE_F4_CONVERSION "FakeDisableF4Conversion"
#define STR_CMD_FAKE_ENABLE_F4_CONVERSION  "FakeEnableF4Conversion"



#ifdef PCAPP //file shared with firmware, avoid including these in Firmware   

#define SEND_DELAY for (int delay = 0; delay < 1000000; delay++); //~2.5ms on ASUS pc, smaller delay might cause disturb current sequencing in F7

typedef enum
{
    APP_MODE_INIT = 0,
    APP_MODE_PRINT,
    APP_MODE_GETUSERINPUT,
    APP_MODE_FAKER,
    APP_MODE_FREEZE,
    APP_MODE_MAX
} appMode_t;

typedef enum
{
    APP_CMD_UNKNOWN = 0,
    APP_CMD_MONITOR,
    APP_CMD_READSN,
    APP_CMD_READFWINFO,
    APP_CMD_READSESSIONIDS,
    APP_CMD_SETSN,
    APP_CMD_FAKER,
    APP_CMD_UNLOCK,
    APP_CMD_ENABLE,
    APP_CMD_REBOOT,
    APP_CMD_REBOOT_PAD,
    APP_CMD_REBOOT_QUAD,
    APP_CMD_FORCE_SELECT_RA,
    APP_CMD_FORCE_SELECT_LA,
    APP_CMD_FORCE_SELECT_LL,
    APP_CMD_CHANGE_NOTCH,
    APP_CMD_PAD_READSN,
    APP_CMD_PAD_WRITESN,
    APP_CMD_GET_ERROR,
    APP_CMD_PRINT_ERROR_REPORT,
    APP_CMD_GET_DPM_CLOCK,
    APP_CMD_READ_HEARTBEAT,
    APP_CMD_DSP_SAMPLE_SWITCH,
    APP_CMD_FAKE_CRC,
    APP_CMD_FAKE_CRC_SN,
    APP_CMD_FAKE_CRC_F41,
    APP_CMD_FAKE_CRC_F42,
    APP_CMD_FAKE_F41_UPDATE,
    APP_CMD_FAKE_F42_UPDATE,
    APP_CMD_FAKE_F7_WDT_REFRESH,
    APP_CMD_FAKE_F41_WDT_REFRESH,
    APP_CMD_FAKE_F42_WDT_REFRESH,
    APP_CMD_FAKE_STOP_PAD2DSP_COM,
	APP_CMD_FAKE_STOP_F72PAD_COM,
	APP_CMD_FAKE_STOP_DSP2F7_DATA_COM,
    APP_CMD_FAKE_STOP_DSP2F7_MSG_COM,
	APP_CMD_FAKE_DISABLE_F4_CONVERSION,
	APP_CMD_FAKE_ENABLE_F4_CONVERSION,
    APP_CMD_MAX
} appCmd_t;

bool openComPort ();
void TxDelay ();
void testTask ();
void periodicSendTest ();
void user_wait ();

#endif//PCAPP

#endif//_APP_UTILS_H
