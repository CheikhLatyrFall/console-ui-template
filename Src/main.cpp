/**
 * @author Latyr
 * @email lfall@sig-num.com
 * @create date 2020-01-30 12:39:18
 * @modify date 2020-01-30 12:39:18
 * @desc [description]
 */


#include <iostream>
#include <fstream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <string.h>
#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mySerial.h"
#include "keyinput.h"
#include "app_utils.h"
#include "app_info.h"

using namespace std;

extern const char * app_cmdStr[APP_CMD_MAX];
extern const char* usb_dev;

int usbCheckCnt = 0;
char usbCheckCmd [24] = {0};
bool event_disconnect = false;

mySerial pCom;
unsigned char sentStr   [128]   = {0};
unsigned char ackStr     [32]   = {0};
unsigned char data_str  [128]   = {0};
unsigned char compiled_string [128] = {0};
int comp_str_idx = 0;
int data_idx = 0;
int ack_idx = 0;

bool newHeartbeat = false;

bool ACK_req = false;

appMode_t currMode = APP_MODE_INIT;
appMode_t prevMode = currMode;
appCmd_t  lastValidCmd  = APP_CMD_UNKNOWN;
char StrParam [128] = {0};
char lastCmdSent[128] = {0};
uint8_t fakerParam = 0;

bool res = false;
bool doPrintUI   = false;
bool doOutputLog = false;
bool waiting = false;
bool run = true;
time_t AppStartTime;
ofstream myLogFile;
FILE * pLogFile = NULL;
bool LogFIleOpen = false;
bool AppStartingUp = true;

static void printUI ();
static bool DPM_sendCmd_enableLogOut (bool en);

static bool IsDisconnected ()
{
    usbCheckCnt ++; if (usbCheckCnt < 10000) return false;
    memset (usbCheckCmd, 0, sizeof(usbCheckCmd));
    strcat (usbCheckCmd, "ls -l ");
    strcat (usbCheckCmd, usb_dev);
    strcat (usbCheckCmd, " >>/dev/null 2>>/dev/null");
    int resl = system(usbCheckCmd);
    //std::printf("%d\n", resl);
    usbCheckCnt = 0;
    return (resl != 0);
}

static void CloseApp ()
{
    if (pCom.IsOpen())
    {
        if (pLogFile) fclose (pLogFile);
        std::printf("Closing Com Port...");
        pCom.Close();
    }
    printf("\r");
    std::exit (0);
}

static void printOutAppTime ()
{
    time_t AppCurrTime = time(0);
    time_t AppDurTime = AppCurrTime - AppStartTime;
    tm durTime;  memcpy(&durTime, gmtime(&AppDurTime), sizeof(tm));
    char timeStamp [24] = {0};
    sprintf(timeStamp, "%02d:%02d:%02d - ", durTime.tm_hour, durTime.tm_min, durTime.tm_sec);
    printf("%s", timeStamp);
#ifdef LOG_IN_FILE
    if (pLogFile && LogFIleOpen)
        fwrite (timeStamp , sizeof(char), strlen(timeStamp), pLogFile);
#endif
}


static bool confirmUserInput ()
{
    char confirmLetter[128] = {0};
    while (strcmp(confirmLetter,"Y") && strcmp(confirmLetter,"y") && 
           strcmp(confirmLetter,"N") && strcmp(confirmLetter,"n"))
    {
        std::printf("\r[Y]yes to confirm | [N]no to cancel\n\r");
        memset(confirmLetter, 0, sizeof(confirmLetter));
        scanf("%s", confirmLetter);
        std::printf("\rconfirmLetter = %s\n", confirmLetter);
    }
    if (!strcmp(confirmLetter,"Y") || !strcmp(confirmLetter,"y"))
    {
        std::printf("\ruser confirmed\n");
        return true;
    }
    std::printf("\ruser canceled\n");
    return false;
}

static appCmd_t getCommand (char car)
{
    appCmd_t tempComd = APP_CMD_UNKNOWN;
    switch (currMode)
    {
        case APP_MODE_INIT:
            if (car == 'I' || car == 'i')
                return APP_CMD_GET_ERROR;
            if (car == 'J' || car == 'j')
                return APP_CMD_PRINT_ERROR_REPORT;
            if (car == 'V' || car == 'v')
                return APP_CMD_GET_DPM_CLOCK;        
            if (car == 'L' || car == 'l')
                return APP_CMD_MONITOR;
            if (car == 'R' || car == 'r')
                return APP_CMD_READSN;
            if (car == 'O' || car == 'o')
                return APP_CMD_PAD_READSN;
            if (car == 'M' || car == 'm')
                return APP_CMD_READSESSIONIDS;
            if (car == 'H' || car == 'h')
                return APP_CMD_PAD_WRITESN;
            if (car == 'G' || car == 'g')
                return APP_CMD_READFWINFO;
            if (car == 'S' || car == 's')
                return APP_CMD_SETSN;
            if (car == 'F' || car == 'f')
                return APP_CMD_FAKER;
            if (car == 'P' || car == 'p')
                return APP_CMD_UNLOCK;
            if (car == 'W' || car == 'w')
                return APP_CMD_FORCE_SELECT_RA;
            if (car == 'N' || car == 'n')
                return APP_CMD_CHANGE_NOTCH;
            /*if (car == 'E' || car == 'e')
                return APP_CMD_ENABLE;*/
            if (car == 'A' || car == 'a')
				return APP_CMD_DSP_SAMPLE_SWITCH;
            if (car == 'Z' || car == 'z')
                return APP_CMD_REBOOT;
            if (car == 'K' || car == 'k')
				return APP_CMD_REBOOT_PAD;
            if (car == 'Y' || car == 'y')
				return APP_CMD_REBOOT_QUAD;
            if (car == 'T' || car == 't')
                return APP_CMD_READ_HEARTBEAT;
            break;

        case APP_MODE_FAKER:
            tempComd = APP_CMD_UNKNOWN;
            fakerParam = 0;
            if (car == '0' || car == '1')
                tempComd = APP_CMD_FAKE_CRC;
            if (car == '2' || car == '3')
                tempComd = APP_CMD_FAKE_CRC_SN;
            if (car == '4' || car == '5')
                tempComd = APP_CMD_FAKE_CRC_F41;
            if (car == '6' || car == '7')
                tempComd = APP_CMD_FAKE_CRC_F42;
            if (car == '8' || car == '9')
                tempComd = APP_CMD_FAKE_F41_UPDATE;
            if (car == 'A' || car == 'a' || car == 'B' || car == 'b')
                tempComd = APP_CMD_FAKE_F42_UPDATE;
            if (car == 'C' || car == 'c' || car == 'D' || car == 'd')
                tempComd = APP_CMD_FAKE_F7_WDT_REFRESH;
            if (car == 'E' || car == 'e' || car == 'F' || car == 'f')
                tempComd = APP_CMD_FAKE_F41_WDT_REFRESH;
            if (car == 'G' || car == 'g' || car == 'H' || car == 'h')
                tempComd = APP_CMD_FAKE_F42_WDT_REFRESH;
            if (car == 'I' || car == 'i')
            	tempComd = APP_CMD_FAKE_STOP_PAD2DSP_COM;
            if (car == 'J' || car == 'j')
            	tempComd = APP_CMD_FAKE_STOP_F72PAD_COM;
            if (car == 'K' || car == 'k')
            	tempComd = APP_CMD_FAKE_STOP_DSP2F7_DATA_COM;
            if (car == 'L' || car == 'l')
            	tempComd = APP_CMD_FAKE_STOP_DSP2F7_MSG_COM;
            if (car == 'M' || car == 'm')
            	tempComd = APP_CMD_FAKE_DISABLE_F4_CONVERSION;
            if (car == 'N' || car == 'n')
            	tempComd = APP_CMD_FAKE_ENABLE_F4_CONVERSION;
            
            if (tempComd != APP_CMD_UNKNOWN)
            {
                if (atoi(&car) % 2 == 0 || (car == 'A' || car == 'a') || 
                   (car == 'C' || car == 'c') || (car == 'E' || car == 'e') ||
                   (car == 'G' || car == 'g') || (car == 'I' || car == 'i') ||
                   (car == 'J' || car == 'j') || (car == 'K' || car == 'k') ||
                   (car == 'L' || car == 'l') || (car == 'M' || car == 'm')	||
				   (car == 'N' || car == 'n'))
                    fakerParam = 1;
                else
                    fakerParam = 0;
                return tempComd;
            }
            break;
        
        //ignore
        default:
            break;
    }

    //exit currMode, go back to INIT
    if (car == 'Q' || car == 'q')
    {
        if (lastValidCmd == APP_CMD_MONITOR)
            DPM_sendCmd_enableLogOut (false);
        prevMode = currMode;
        currMode = APP_MODE_INIT;
        doPrintUI = true;
        doOutputLog = false;
        if (waiting) waiting = false;
    }

    if ((car == 'F' || car == 'f')/* && 
        prevMode == APP_MODE_FAKER && currMode == APP_MODE_FREEZE*/)
    {
        prevMode = currMode;
        currMode = APP_MODE_FAKER;
        doPrintUI = true;
        if (waiting) waiting = false;
    }

    //simply update UI
    if (car == 'U' || car == 'u')
        printUI();

    return APP_CMD_UNKNOWN;
}

static bool sendCommand (char cmd[], char cmdParam[])
{
    if (cmd == NULL || cmdParam == NULL) return false;
    memset(lastCmdSent, 0, sizeof(lastCmdSent));
    if (!pCom.IsOpen()) return false;
    unsigned char cmdString[128] = {0};
    if (strlen(cmd) + strlen(cmdParam) + 2 >= sizeof(cmdString)) 
    {
        std::printf("\rcommand not sent, invalid size\n");
        return false; 
    }
    for (int h = 0; h < strlen(cmd); h++)
    {
        if (!isalnum((int)cmd[h])) 
        {
            std::printf("\rcommand not sent, invalid cmd format | %s\n", cmd);
            return false;
        }
    }
    if (strcmp(cmdParam, " "))
    {
        for (int h = 0; h < strlen(cmdParam); h++)
            if (!isalnum((int)cmdParam[h]) && cmdParam[h] != '-') 
            {
                std::printf("\rcommand not sent, wrong argument format\n");
                return false; //'-' added to accomodate for negative value parameters
            }
    }
    strncpy((char*)cmdString, cmd, strlen(cmd));
    char chSepString [2] = {0};
    strcat((char*)cmdString, " ");
    if (strcmp(cmdParam, " "))
        strcat((char*)cmdString, cmdParam);
    strcat((char*)cmdString, ";");
    printf("\rSending Command: %s\n\r", cmdString);
    bool r = true;
    for (int h = 0; h < strlen((const char*)cmdString) + 1; h++)
    {
        r &= pCom.Send(cmdString + h, 1);
        TxDelay();
    }
    if (r)
    {
        memcpy(lastCmdSent, cmdString, sizeof(lastCmdSent));
#ifdef LOG_IN_FILE
        if (pLogFile && LogFIleOpen)
        {
            char tmpStr [256] = "Cmd: ";    
            strcat(tmpStr, lastCmdSent);
            strcat(tmpStr, "\n");
            printOutAppTime ();
            fwrite (tmpStr , sizeof(char), strlen(tmpStr), pLogFile);
        }
#endif
    }
    return r;
}

static bool DPM_sendCmd_enableLogOut (bool en)
{
    memset(StrParam, 0, sizeof(StrParam));
    sprintf(StrParam, "%d", (int)en);
    res = sendCommand ((char*)app_cmdStr[APP_CMD_ENABLE], StrParam);
    if (res); /*printf("\rEnable LogOut command sent\n");*/
    else printf("\rFailed to send Enable LogOut cmd\n\r");
    //printf("\rlast cmd sent: %s\n", lastCmdSent);
    user_wait ();
    return res;
}

static void printUI ()
{
    switch (currMode)
    {
        case APP_MODE_INIT:
            AppStartingUp = false;
            system("clear");
            std::printf("\rSIG.ECHO usb console app v%s p%s:\n\
            \r  Main Menu:\n\
            \r\t[Z] Reboot Device\n\
            \r\t[K] Reboot PAD\n\
            \r\t[Y] Reboot QUAD\n\
            \r\t[I] Get Error Status\n\
            \r\t[J] Print Error Report\n\
            \r\t[V] Get DPM Clock\n\
            \r\t[L] Monitor Device\n\
            \r\t[S] Set Serial Number\n\
            \r\t[R] Read Serial Number\n\
            \r\t[H] Set PAD Serial Number\n\
            \r\t[O] Read PAD Serial Number\n\
            \r\t[M] Read Session IDs\n\
            \r\t[G] Read Firmware Info\n\
            \r\t[W] Force Sensor Selection\n\
            \r\t[N] Change DAC Notch Setting\n\
            \r\t[F] Faker\n\
            \r\t[P] Unlock Command Interface\n\
            \r\t[T] Read Heartbeat\n\
			\r\t[A] Sample Switch \n\
            \r\t[U] Update Interface\n\
            \r\t[Q] Main Menu\n\
            \r\t[X] Exit\n\r", APP_SOFTWARE_VERSION, APP_PROTOCOL_VERSION);
            break;

        case APP_MODE_FREEZE:
            //DPM_sendCmd_enableLogOut (false);
            doOutputLog = false;
            if (prevMode == APP_MODE_FAKER)
                std::printf("\rPress [F] to go back to Faker menu\n");
            if (!AppStartingUp)    
                std::printf("\rPress [Q] to go back to Main menu\n\r");
            break;

        case APP_MODE_FAKER:
            system("clear");
            std::printf("SIG.ECHO usb console app v%s p%s:\n\
            \r  Faker Options:\n\
            \
            \r\t[0] Compute Wrong   CRC\n\
            \r\t[1] Compute Correct CRC\n\
            \
            \r\t[2] Compute Wrong   Serial Number CRC\n\
            \r\t[3] Compute Correct Serial Number CRC\n\
            \
            \r\t[4] Compute Wrong   F41 CRC\n\
            \r\t[5] Compute Correct F41 CRC\n\
            \
            \r\t[6] Compute Wrong   F42 CRC\n\
            \r\t[7] Compute Correct F42 CRC\n\
            \
            \r\t[8] Stop  Updating F41\n\
            \r\t[9] Start Updating F41\n\
            \
            \r\t[A] Stop  Updating F42\n\
            \r\t[B] Start Updating F42\n\
            \
            \r\t[C] Disable F7 Watchdog Refresh\n\
            \r\t[D] Enable  F7 Watchdog Refresh\n\
            \
            \r\t[E] Disable F41 Watchdog Refresh\n\
            \r\t[F] Enable  F41 Watchdog Refresh\n\
            \
            \r\t[G] Disable F42 Watchdog Refresh\n\
            \r\t[H] Enable  F42 Watchdog Refresh\n\
            \
            \r\t[I] Disable F4  to DSP Data Communication\n\
            \r\t[J] Disable F7  to PAD Msg  Communication\n\
            \r\t[K] Disable DSP to F7  Data Communication\n\
            \r\t[L] Disable DSP to F7  Msg  Communication\n\
            \
            \r\t[M] Disable F4 Conversion\n\
			\r\t[N] Enable  F4 Conversion\n\
			\
            \r\t[Q] Main Menu\n\
            \r\t[X] Exit\n\r", APP_SOFTWARE_VERSION, APP_PROTOCOL_VERSION);
            break;

        case APP_MODE_PRINT:
            system("clear");
            //TODO: Enable printing
            break;
        
        case APP_MODE_GETUSERINPUT:
            res = false;
            if (lastValidCmd == APP_CMD_PAD_WRITESN)
            {
                while (!res)
                {
                    system("clear");
                    if (lastValidCmd == APP_CMD_PAD_WRITESN)
                        std::printf("\rEnter a valid PAD serial number + ENTER: ");
                    memset(StrParam, 0, sizeof(StrParam));
                    scanf("%s", StrParam);
                    system("clear");
                    std::printf("\rEntered user input is %s\n", StrParam);
                    res = confirmUserInput();
                    doOutputLog = true;
                    if (lastValidCmd == APP_CMD_PAD_WRITESN && res)
                    {    
                        if (sendCommand ((char*)app_cmdStr[APP_CMD_PAD_WRITESN], StrParam))
                            std::printf("\rCommand successfully sent : %s\n", lastCmdSent);
                        else
                            std::printf("\rFailed to send command : %s\n", lastCmdSent);
                        user_wait ();
                        doOutputLog = false;
                        user_wait ();
                        std::printf("\rReboot DPM? (recommended)\n");
                        res = confirmUserInput();
                        if (lastValidCmd == APP_CMD_PAD_WRITESN && res)
                        {
                            std::printf("\rClosing application, DPM will reboot\n");
                            memset(StrParam, 0, sizeof(StrParam));
                            strcpy(StrParam, " ");
                            sendCommand ((char*)app_cmdStr[APP_CMD_REBOOT], StrParam);
                            user_wait ();
                            CloseApp();
                        }
                        else
                        {
                            goto exit_user_input;
                        }
                        
                    }
                    else
                    {
                        goto exit_user_input;
                    }
                    
                }
            }
            else if (lastValidCmd == APP_CMD_SETSN)
            {
                while (!res)
                {
                    system("clear");
                    if (lastValidCmd == APP_CMD_SETSN)
                        std::printf("\rEnter a valid serial number + ENTER: ");
                    memset(StrParam, 0, sizeof(StrParam));
                    scanf("%s", StrParam);
                    system("clear");
                    std::printf("\rEntered user input is %s\n", StrParam);
                    res = confirmUserInput();
                    doOutputLog = true;
                    if (lastValidCmd == APP_CMD_SETSN && res)
                    {    
                        if (sendCommand ((char*)app_cmdStr[APP_CMD_SETSN], StrParam))
                            std::printf("\rCommand successfully sent : %s\n", lastCmdSent);
                        else
                            std::printf("\rFailed to send command : %s\n", lastCmdSent);
                        user_wait ();
                    }
                    else
                    {
                        goto exit_user_input;
                    }
                    
                }
            }
            else if (lastValidCmd == APP_CMD_FORCE_SELECT_RA)
            {
                //TODO: add code here to allowing RA and LA sensor selections
                doOutputLog = false;
                appCmd_t tempCmd = APP_CMD_MAX;
                while (!res)
                {
                    system("clear");
                    uint8_t ra = 0, la = 0, ll = 0;
                    bool selectLead = false;
                    bool selectSensor = false;
                    int IntParam = -2;
                    while ((strcmp(StrParam,"R") && strcmp(StrParam,"r") && 
                            strcmp(StrParam,"L") && strcmp(StrParam,"l") &&
                            strcmp(StrParam,"D") && strcmp(StrParam,"d")) || !selectLead)
                    {
                        ra = la = ll = 0;
                        system("clear");
                        std::printf("\rPress [Q] + ENTER to exit\n");
                        std::printf("\rLead to force on | press [RA]R, [LA]L or [LL]D then ENTER :");
                        memset(StrParam, 0, sizeof(StrParam));
                        scanf("%s", StrParam);
                        if (!(strcmp(StrParam, "R") && strcmp(StrParam, "r") && 
                              strcmp(StrParam, "L") && strcmp(StrParam, "l") &&
                              strcmp(StrParam, "D") && strcmp(StrParam, "d")))
                        {
                            system("clear");
                            std::printf("\rPress [Q] + ENTER to exit\n");
                            std::printf("\rForce selection on %s?\n", ((strcmp(StrParam, "l") == 0 || strcmp(StrParam, "L") == 0) ? "LA" :
                                                                      ((strcmp(StrParam, "R") == 0 || strcmp(StrParam, "r") == 0) ? "RA" : "LL")));
                            selectLead = confirmUserInput();    
                        }
                        else if (strcmp(StrParam, "Q") || strcmp(StrParam, "q"))
                            goto exit_user_input;
                    }
                    if      (strcmp(StrParam, "R") == 0 || strcmp(StrParam, "r") == 0)  {ra = 1; la = 0; ll = 0;}
                    else if (strcmp(StrParam, "L") == 0 || strcmp(StrParam, "l") == 0)  {ra = 0; la = 1; ll = 0;}
                    else if (strcmp(StrParam, "D") == 0 || strcmp(StrParam, "d") == 0)  {ra = 0; la = 0; ll = 1;}
                    while ((IntParam < -1 || IntParam > 127 || !selectSensor) && selectLead)
                    {
                        IntParam = -2;
                        system("clear");
                        std::printf("\rPress [Q] + ENTER to exit\n");
                        std::printf("\rSelect Sensor [0 ; 127] to force on %s (-1 to clear selection): ", 
                                     ((strcmp(StrParam, "l") == 0 || strcmp(StrParam, "L") == 0) ? "LA" :
                                     ((strcmp(StrParam, "R") == 0 || strcmp(StrParam, "r") == 0) ? "RA" : "LL")));
                        scanf("%d", &IntParam);
                        if (IntParam >= -1 && IntParam < 128)
                        {
                            system("clear");
                            std::printf("\rPress [Q] + ENTER to exit\n");
                            std::printf("\rChoose sensor %d on %s?\n", IntParam, (ra == 1) ? "RA" :((la == 1) ? "LA" : "LL"));
                            selectSensor = confirmUserInput();
                        }
                        else if (strcmp(StrParam, "Q") || strcmp(StrParam, "q"))
                            goto exit_user_input;
                    }
                    doOutputLog = true;
                    if (lastValidCmd == APP_CMD_FORCE_SELECT_RA && selectSensor)
                    {    
                        char SensorSelectStr [10] = {0};
                        snprintf(SensorSelectStr, sizeof(SensorSelectStr), "%d", IntParam);
                        tempCmd = ((ra == 1) ? APP_CMD_FORCE_SELECT_RA : ((la == 1) ? APP_CMD_FORCE_SELECT_LA : APP_CMD_FORCE_SELECT_LL));
                        if (sendCommand ((char*)app_cmdStr [tempCmd], SensorSelectStr))
                            std::printf("\rCommand successfully sent : %s\n", lastCmdSent);
                        else
                            std::printf("\rFailed to send command : %s\n", lastCmdSent);
                        user_wait ();
                    }
                    std::printf("\rDo you want to force another selection?\n");
                    res = !confirmUserInput();
                    res &= (selectSensor && selectLead);
                }
            }
exit_user_input:
            if (lastValidCmd != APP_CMD_PAD_WRITESN)
                system("clear");
            std::printf("\rPress [Q] to go back to menu\n");
            //go back to init ui
            doPrintUI = true;
            doOutputLog = true;
            prevMode = currMode;
            currMode = APP_MODE_FREEZE;
            res = false;
            break;

        case APP_MODE_MAX:
        default:
            break;
    }
}

void readCom ()
{
    bool doTimeStamp = false;
    //printf("\rtask1: started\n");
    while (run)
    {  
        if (!pCom.IsOpen())
        {
            if (pLogFile) fclose (pLogFile);
            run = false;
            std::printf("\rCom Port closed!\n");
            std::exit(0);
        }
        int nbRcv = pCom.Receive(data_str + data_idx,1);
        if (data_str[data_idx] == '\n')
        {
            std::printf("\r");
            doTimeStamp = true;
        }
        if (nbRcv)
        {
            //std::printf("r");
            //testing ////////////////////////////////////////////////
            if (newHeartbeat == false)
            {    
                compiled_string[comp_str_idx++] = data_str[data_idx];
                if (strstr((char*)compiled_string, "Heartbeat") || strstr((char*)compiled_string, "(LA, RA, LL)"))
                {
                    newHeartbeat = true;
                    memset(compiled_string, 0, sizeof(compiled_string));
                    comp_str_idx = 0;
                }
                if (comp_str_idx >= sizeof(compiled_string))
                    comp_str_idx = 0;
            }
            //////////////////////////////////////////////////////////

            if (doOutputLog)
            {
                if (doTimeStamp && data_str[data_idx] != '\n')
                {
                    printOutAppTime ();
                    doTimeStamp = false;
                }
                std::printf ("%c", *(data_str + data_idx));
#ifdef LOG_IN_FILE                
                if (pLogFile && LogFIleOpen)
                    fputc (*(data_str + data_idx), pLogFile);
#endif
            }

            //testing ////////////////////////////////////////////////
            /*if (newHeartbeat && (currMode == APP_MODE_INIT))
            {
                std::printf("\xE2\x99\xA5");
                newHeartbeat = false;
            }*/
            //////////////////////////////////////////////////////////

            data_idx ++;
            if (data_idx >= sizeof(data_str))
            {
                data_idx = 0;
                memset(data_str, 0, sizeof(data_str));
            }
        }
    }
    //printf("\rtask1: interrupted\n");
}

void readKeys ()
{
    //printf("\rtask3: started\n");
    int ch = 0;
#ifdef WAIT_HEARTBEAT    
    std::printf("\rWaiting for DPM heartbeat...\n");
    std::printf("\rPress [S] to skip\n");
    while (!newHeartbeat && (ch != 'S' && ch != 's')) //wait until heartbeat received to print
        ch = keyinput_kbhit ();
#endif
    if (currMode != APP_MODE_FREEZE)
        system("clear");
    printUI ();
    while (run && ch != 'x' && ch != 'X')
    {
        ch = keyinput_kbhit (); /*if (ch != 0) printf("key pressed == %c\n", (unsigned char)ch);*/
        
        if (IsDisconnected())
        {
            event_disconnect = true;
            std::printf("\rUSB device disconnected\n");
            ch = 'X';
        }

        //if (ch == 0) continue;
        appCmd_t currCmd = getCommand (ch);
        if (currMode != APP_MODE_INIT && currMode != APP_MODE_FAKER)
        {    
            prevMode = currMode;
            currCmd = APP_CMD_UNKNOWN;
        }
        switch (currCmd)
        {
            case APP_CMD_READ_HEARTBEAT:
                system("clear");
                doPrintUI = false;
                doOutputLog = true;
                break;
            
            case APP_CMD_PRINT_ERROR_REPORT:
                system("clear");
                doOutputLog = true;
                std::printf("\rPrinting Error Report...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_PRINT_ERROR_REPORT], StrParam);
                user_wait();
                user_wait();
                user_wait();
                lastValidCmd = APP_CMD_PRINT_ERROR_REPORT;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_GET_DPM_CLOCK:
                system("clear");
                doOutputLog = true;
                std::printf("\rGet DPM Clock...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_GET_DPM_CLOCK], StrParam);
                user_wait();
                user_wait();
                user_wait();
                lastValidCmd = APP_CMD_GET_DPM_CLOCK;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;
            
            case APP_CMD_GET_ERROR:
                system("clear");
                doOutputLog = true;
                std::printf("\rReading Error Status...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_GET_ERROR], StrParam);
                user_wait();
                lastValidCmd = APP_CMD_GET_ERROR;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_MONITOR:
                printf("\rEnabling logging...\n");
                doOutputLog = true;
                DPM_sendCmd_enableLogOut (true);
                lastValidCmd = APP_CMD_MONITOR;
                prevMode = currMode;
                currMode = APP_MODE_PRINT;
                break;

            case APP_CMD_UNLOCK:
                system("clear");
                doOutputLog = true;
                memset(StrParam, 0, sizeof(StrParam));
                strncpy(StrParam, USB_LOG_PASSWORD_STR, strlen(USB_LOG_PASSWORD_STR));
                res = sendCommand ((char*)app_cmdStr[APP_CMD_UNLOCK], StrParam);
                /*if (res) printf("\rUnlock command sent\n");
                else printf("\rFailed to send unlock cmd\n");*/
                user_wait ();
                lastValidCmd = APP_CMD_UNLOCK;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_READFWINFO:
                system("clear");
                doOutputLog = true;
                std::printf("\rReading Firmware Info...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_READFWINFO], StrParam);
                user_wait();
                lastValidCmd = APP_CMD_READFWINFO;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_READSN:
                system("clear");
                doOutputLog = true;
                std::printf("\rReading Serial Number...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_READSN], StrParam);
                user_wait ();
                lastValidCmd = APP_CMD_READSN;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_READSESSIONIDS:
                system("clear");
                doOutputLog = true;
                std::printf("\rReading Session IDs...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_READSESSIONIDS], StrParam);
                user_wait ();
                lastValidCmd = APP_CMD_READSESSIONIDS;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_SETSN:
                printf("\rSetting Serial Number...\n");
                //TODO: Set Serial Number
                lastValidCmd = APP_CMD_SETSN;
                prevMode = currMode;
                currMode = APP_MODE_GETUSERINPUT;
                doOutputLog = false;
                doPrintUI = true;
                break;

            case APP_CMD_PAD_READSN:
                system("clear");
                doOutputLog = true;
                std::printf("\rReading PAD Serial Number...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, "1");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_PAD_READSN], StrParam);
                user_wait ();
                user_wait ();
                //user_wait ();
                lastValidCmd = APP_CMD_PAD_READSN;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_PAD_WRITESN:
                printf("\rSetting PAD Serial Number...\n");
                //TODO: Set Serial Number
                lastValidCmd = APP_CMD_PAD_WRITESN;
                prevMode = currMode;
                currMode = APP_MODE_GETUSERINPUT;
                doOutputLog = false;
                doPrintUI = true;
                break;

            case APP_CMD_CHANGE_NOTCH:
                system("clear");
                doOutputLog = true;
                std::printf("\rChanging Notch FIlter Setting...\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_CHANGE_NOTCH], StrParam);
                user_wait();
                lastValidCmd = APP_CMD_CHANGE_NOTCH;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            case APP_CMD_FORCE_SELECT_RA:
                printf("\rForcing Sensor Selection...\n");
                //TODO: Set Serial Number
                lastValidCmd = APP_CMD_FORCE_SELECT_RA;
                prevMode = currMode;
                currMode = APP_MODE_GETUSERINPUT;
                doOutputLog = false;
                doPrintUI = true;
                break;

            case APP_CMD_FAKER:
                printf("\rLaunching Faker...\n");
                //TODO: Launch Faker
                lastValidCmd = APP_CMD_FAKER;
                prevMode = currMode;
                currMode = APP_MODE_FAKER;
                doOutputLog = false;
                doPrintUI = true;
                break;

            case APP_CMD_REBOOT:
                system("clear");
                doOutputLog = true;
                printf("\rRebooting Device... application will close\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_REBOOT], StrParam);
                user_wait();
                doOutputLog = false;
                lastValidCmd = APP_CMD_REBOOT;
                run = false;
                res = false;
                break;

            case APP_CMD_REBOOT_PAD:
            	system("clear");
            	doOutputLog = true;
            	printf("\rRebootting PAD...\n");
            	memset(StrParam, 0, sizeof(StrParam));
            	strcpy(StrParam," ");
            	res = sendCommand ((char*)app_cmdStr[APP_CMD_REBOOT_PAD], StrParam);
            	user_wait();
				user_wait();
				user_wait();
				user_wait();
				user_wait();
            	lastValidCmd = APP_CMD_REBOOT_PAD;
            	prevMode = currMode;
            	currMode = APP_MODE_FREEZE;
            	doOutputLog = false;
            	doPrintUI = true;
            	break;

            case APP_CMD_REBOOT_QUAD:
                system("clear");
                doOutputLog = true;
                printf("\rAttempting to reset QUAD UART\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_REBOOT_QUAD], StrParam);
                user_wait();
                user_wait();
				user_wait();
                user_wait();
                lastValidCmd = APP_CMD_REBOOT_QUAD;
            	prevMode = currMode;
            	currMode = APP_MODE_FREEZE;
            	doOutputLog = false;
            	doPrintUI = true;
                break;

            case APP_CMD_DSP_SAMPLE_SWITCH:
                system("clear");
                doOutputLog = true;
                printf("\r Switching sample\n");
                memset(StrParam, 0, sizeof(StrParam));
                strcpy(StrParam, " ");
                res = sendCommand ((char*)app_cmdStr[APP_CMD_DSP_SAMPLE_SWITCH], StrParam);
                user_wait();
                user_wait();
				user_wait();
                user_wait();
                lastValidCmd = APP_CMD_DSP_SAMPLE_SWITCH;
            	prevMode = currMode;
            	currMode = APP_MODE_FREEZE;
            	doOutputLog = false;
            	doPrintUI = true;
                break;

            case APP_CMD_FAKE_CRC:
            case APP_CMD_FAKE_CRC_SN:
            case APP_CMD_FAKE_CRC_F41:
            case APP_CMD_FAKE_CRC_F42:
            case APP_CMD_FAKE_F41_UPDATE:
            case APP_CMD_FAKE_F42_UPDATE:
            case APP_CMD_FAKE_F7_WDT_REFRESH:
            case APP_CMD_FAKE_F41_WDT_REFRESH:
            case APP_CMD_FAKE_F42_WDT_REFRESH:
            case APP_CMD_FAKE_STOP_PAD2DSP_COM:
            case APP_CMD_FAKE_STOP_F72PAD_COM:
            case APP_CMD_FAKE_STOP_DSP2F7_DATA_COM:
            case APP_CMD_FAKE_STOP_DSP2F7_MSG_COM:
            case APP_CMD_FAKE_DISABLE_F4_CONVERSION:
            case APP_CMD_FAKE_ENABLE_F4_CONVERSION:
                system("clear");
                doOutputLog = true;
                //std::printf("\rSetting crc16 faker...\n");
                memset(StrParam, 0, sizeof(StrParam));
                sprintf(StrParam, "%d", fakerParam);//fetch faker Param
                std::printf("\rFaker Param = %d | str = %s\n", fakerParam, StrParam);
                res = sendCommand ((char*)app_cmdStr[currCmd], StrParam);
                user_wait ();
                user_wait ();
                user_wait ();
                lastValidCmd = currCmd;
                doOutputLog = false;
                prevMode = currMode;
                currMode = APP_MODE_FREEZE;
                res = false;
                break;

            default:
                break;
        }
        
        if ((currCmd != APP_CMD_UNKNOWN || doPrintUI) && currCmd != APP_CMD_READ_HEARTBEAT)
        {
            printUI ();
            doPrintUI = false;
        }

        //TODO: add delay here to slow down printing of UI
        for (int u = 0; u < 1000000; u++);
    }
    //std::printf("\rtask3: interrupted\n");
    run = false;

    CloseApp();
}

int main ()
{
    printf("\rTest\n");

    //std::thread testTaskX(testTask);
    //testTaskX.join();

    std::thread task1;
    //std::thread task2;
    std::thread task3;

    if (!openComPort()) 
    {
        printf("\rCould not open Com port\n");
        //while(1);
        return false;
    }
    else
        printf("\rCom port openned\n");

    if (!keyinput_init())
    {
        printf("\rFailed to init keyread\n");
        return false;
    }

    //Timestamping and log file
    AppStartTime = time(0);
    tm LogTime;  memcpy(&LogTime, gmtime(&AppStartTime), sizeof(tm));
    //printf("\nyear %d month %02d day %02d hour %d min %d sec %d\n", LogTime.tm_year + 1900, LogTime.tm_mon + 1, LogTime.tm_mday, LogTime.tm_hour - 5, LogTime.tm_min, LogTime.tm_sec);
    char LogT [20] = {0}; 
    sprintf(LogT, "%04d%02d%02d%02d%02d%02d", LogTime.tm_year + 1900, LogTime.tm_mon + 1, LogTime.tm_mday, LogTime.tm_hour - 5, LogTime.tm_min, LogTime.tm_sec);
    char FileName [128] = "Log/DPMLog-";
    strcat (FileName, LogT);
    strcat (FileName, ".log");

#ifdef LOG_IN_FILE
    struct stat st = {0};
    if (stat("./Log", &st) == -1)
        system("sudo mkdir ./Log");
    printf("File name : %s\n\r", FileName);
    pLogFile = fopen(FileName, "wb");
    if (pLogFile == NULL)
    {
        std::printf("\rFailed to create file, logging will not be performed!\n");
        std::printf("\rContinue [Q] or Exit [X]\n");
        LogFIleOpen = false;
        currMode = APP_MODE_FREEZE;
        //return 0;
    }
    else
    {
        LogFIleOpen = true;
    }
    
#endif

    task1 = std::thread(readCom);
    //task2 = std::thread(periodicSendTest);
    task3 = std::thread(readKeys);
    
    task1.join();
    //task2.join();
    task3.join(); 

    while(run);

    if (pLogFile) fclose (pLogFile);

    std::printf("\r!application terminated!\n\r");
    return 0;
}
