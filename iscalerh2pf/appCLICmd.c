#include "appCLICmd.h"
#include "appSystem.h"
#include "utilDbgMsg.h"
//#include "appGui.h"
#include "Board_Uart.h"
#include "utilHostAPI.h"
#include "utilQueueAPI.h"
#include "utilGeneralMSSCAPI.h"
#include "utilCommonMSSCAPI.h"
#include <linux/input.h>
#include <termios.h>
#include "utilHPBU_Tester.h"       //G100_Wilsonj_0018 open
#include "GEC_UserOption.h"        //G100_Steven_0007

#ifdef SIMULATOR_ISCALER
#include "halInputCtrlAPI.h"
#include "halKeyCodeList.h"
#endif
#include "halMCUCtrlAPI.h"
#include "ProjectSettings.h"

#define CLI_STRING_BUFFER   5

static sAPP_CLICMD_INFORMATION  m_sAppCLICmdProcess;
static sAPP_CLICMD_INFORMATION  m_sAppCLICmdDecode;

static sQUEUE m_asCLI_Ring_Buffer;
static sCLI_BUFFER_FORMAT m_sCLI_Buffer[CLI_STRING_BUFFER];

INT16 iValueT = 50;//A70LV_Doulas_0012

struct termios initial_settings;      //for test  //set keyboard blocking or non-blocking
void RestoreKeyboardBlocking(struct termios *initial_settings);
void SetKeyboardNonBlock(struct termios *initial_settings);

int get_thread_policy(pthread_attr_t *attr);
int get_thread_priority(pthread_attr_t *attr);

#ifdef SIMULATOR_ISCALER
int getch(void)
{
    int ch;
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    memcpy(&newt, &oldt, sizeof(newt));
    newt.c_lflag &= ~( ECHO | ICANON | ECHOE | ECHOK |
                       ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

void* key(void* data)
{
    char input = 255;

	sKEY_DATA sInputKey;

    while(1)
    {
        input = getch();

        switch(input)
        {
            case '[':
                {
                    input = getch();
                    switch(input)
                    {
                        case 'A':
							sInputKey.wKeyCode = keUP;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                            LOG_MSG(db_ALWAYS, "Key UP\r\n");
							palInputProc_BufferInsert(&sInputKey);
                            break;

                        case 'B':
							sInputKey.wKeyCode = keDOWN;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                            LOG_MSG(db_ALWAYS, "Key DOWN\r\n");
							palInputProc_BufferInsert(&sInputKey);
                            break;

                        case 'C':
							sInputKey.wKeyCode = keRIGHT;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                            LOG_MSG(db_ALWAYS, "Key RIGHT\r\n");
							palInputProc_BufferInsert(&sInputKey);
                            break;

                        case 'D':
							sInputKey.wKeyCode = keLEFT;
							sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
							sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                            LOG_MSG(db_ALWAYS, "Key LEFT\r\n");
							palInputProc_BufferInsert(&sInputKey);
                            break;
                    }
                }
                break;

	        case 'E':
            case 'e':
				sInputKey.wKeyCode = keENTER;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                LOG_MSG(db_ALWAYS, "Key Enter\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;


            case 'Q':
            case 'q':
				sInputKey.wKeyCode = keBACK;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                LOG_MSG(db_ALWAYS, "Key Exit\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case 'M':
            case 'm':
				sInputKey.wKeyCode = keMENU;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                LOG_MSG(db_ALWAYS, "Key Menu\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case 'A':
            case 'a':
				sInputKey.wKeyCode = keAUTO;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR; //G100_Julie_0040
                LOG_MSG(db_ALWAYS, "Key Auto\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '0':
				sInputKey.wKeyCode = ke0;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 0\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '1':
				sInputKey.wKeyCode = ke1;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 1\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '2':
				sInputKey.wKeyCode = ke2;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 2\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '3':
				sInputKey.wKeyCode = ke3;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 3\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '4':
				sInputKey.wKeyCode = ke4;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 4\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '5':
				sInputKey.wKeyCode = ke5;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 5\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '6':
				sInputKey.wKeyCode = ke6;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 6\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '7':
				sInputKey.wKeyCode = ke7;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 7\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '8':
				sInputKey.wKeyCode = ke8;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 8\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

            case '9':
				sInputKey.wKeyCode = ke9;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;//eKEYINPUT_IR;
                LOG_MSG(db_ALWAYS, "Key 9\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

#if 0
            case 'I':
            case 'i':
                sInputKey.wKeyCode = keLENSVUP;
                sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                sInputKey.eKeyType = eKEYINPUT_CLI;
                LOG_MSG(db_ALWAYS, "Key LENS UP\r\n");
                palInputProc_BufferInsert(&sInputKey);
                break;


            case 'J':
            case 'j':
                sInputKey.wKeyCode = keLENSHLEFT;
                sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                sInputKey.eKeyType = eKEYINPUT_CLI;
                LOG_MSG(db_ALWAYS, "Key LENS LIFT\r\n");
                palInputProc_BufferInsert(&sInputKey);
                break;

            case 'K':
            case 'k':
                sInputKey.wKeyCode = keLENSVDOWN;
                sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                sInputKey.eKeyType = eKEYINPUT_CLI;
                LOG_MSG(db_ALWAYS, "Key LENS DOWN\r\n");
                palInputProc_BufferInsert(&sInputKey);
                break;

            case 'L':
            case 'l':
                sInputKey.wKeyCode = keLENSHRIGHT;
                sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                sInputKey.eKeyType = eKEYINPUT_CLI;
                LOG_MSG(db_ALWAYS, "Key LENS RIGHT\r\n");
                palInputProc_BufferInsert(&sInputKey);
                break;
#endif /* 0 */

            case 'O':
            case 'o':
               sInputKey.wKeyCode = keOSD;
               sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
               sInputKey.eKeyType = eKEYINPUT_CLI;
               LOG_MSG(db_ALWAYS, "Key OSD\r\n");
               palInputProc_BufferInsert(&sInputKey);
               break;

           case 'P':
           case 'p':
               sInputKey.wKeyCode = kePIP;
               sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
               sInputKey.eKeyType = eKEYINPUT_CLI;
               LOG_MSG(db_ALWAYS, "Key PIP\r\n");
               palInputProc_BufferInsert(&sInputKey);
               break;

            case 'F':
            case 'f':
               sInputKey.wKeyCode = keINFO;
               sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
               sInputKey.eKeyType = eKEYINPUT_CLI;
               LOG_MSG(db_ALWAYS, "Key LENS DOWN\r\n");
               palInputProc_BufferInsert(&sInputKey);
               break;

	        case 'S':
            case 's':
				sInputKey.wKeyCode = keSTANDBY;
				sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
				sInputKey.eKeyType = eKEYINPUT_CLI;
                LOG_MSG(db_ALWAYS, "Key STANDBY\r\n");
				palInputProc_BufferInsert(&sInputKey);
                break;

	        case 'I':
            case 'i':
                sInputKey.wKeyCode = keINPUT;
                sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
                sInputKey.eKeyType = eKEYINPUT_CLI;
                LOG_MSG(db_ALWAYS, "Input \r\n");
                palInputProc_BufferInsert(&sInputKey);
                break;

            default:
                MS_SLEEP(100);
                break;

        }

    }
}

void* serial(void* data)
{
    while(1)
    {
        FILE *pFile = NULL;

        pFile = fopen("/tmp/serial", "r");

        if(pFile != NULL)
        {
            char cTemp[256] = {'\0'};
            UINT16 ret = 0;

            ret = fread(cTemp, 1, 256, pFile);
			fclose(pFile);
			remove("/tmp/serial");

            for(UINT16 count = 0; count < ret; count++)
            {
                if(utilGeneral_CLI_Handle(eccPC, cTemp[count]))
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }

#if 0
#ifdef CUSTOM_CHRISTIE
                if(utilChristie_CLI_Handle(eccPC, &cTemp[count]))
                {
                    //palEnvironment_SleepTimerClear();
                }
#elif defined(CUSTOM_OPTOMA)	//A35G2_Coda_0045
                if(utilOptoma_CLI_Handle(eccPC, &cTemp[count]))// BruceLin#20210107
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }

#ifdef GENERAL_MSSC_CLI
                if(utilGeneral_CLI_Handle(eccPC, &cTemp[count]))
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }
#endif /* GENERAL_MSSC_CLI */
#else
                if(utilGeneral_CLI_Handle(eccPC, cTemp[count]))
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }
#endif /* 0 */
#endif
            }
        }
        else
        {
            MS_SLEEP(100);
        }
	}
}


void* cli(void* data)
{
    while(1)
    {
        FILE *pFile = NULL;

        pFile = fopen("/tmp/cli", "r");

        if(pFile != NULL)
        {
			sPAYLOAD sInPayload;
			sPAYLOAD sOutPayload;
			sLAN_DATA_FORMAT slan;
            char cTemp[256] = {'\0'};
            UINT16 ret = 0;
			char cmd;
			UINT8 cMain;
			UINT8 cSub;
			char cDataCode[6] = {'\0'};
			char cDataValue[6] = {'\0'};
			UINT16 uiSize;

            ret = fread(cTemp, 1, 256, pFile);
			fclose(pFile);
			remove("/tmp/cli");

			printf("%s\n", cTemp);

			sscanf((char*)cTemp, "%c,%d,%d,%5[^,],%5s\n", &cmd, &cMain, &cSub, cDataCode, cDataValue);   //G100_Simon_0060

			if(cmd == 'w' || cmd == 'W')
			{
				sInPayload.uModule.sBIT.ucModuleID = cMain;
				sInPayload.uModule.sBIT.cRW = CMD_WRITE;
				sInPayload.ucSubCmd = cSub;
				slan.uiDataCode = atol(cDataCode);
				slan.uData.lDataValue = atol(cDataValue);

				uiSize = 4;

				memcpy(&sInPayload.aucData[0], &slan, sizeof(sLAN_DATA_FORMAT));

                utilCLICmd_Execute(&sInPayload,
                                   &sOutPayload,
                                   (UINT16 *)&uiSize);
			}

        }
        else
        {
            MS_SLEEP(100);
        }

	}
}

#endif

// ==============================================================================
// FUNCTION NAME: appCLI_StringInsert
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/09/02, Larry Create
// --------------------
// ==============================================================================
void appCLI_StringInsert(UINT8 cCh, UINT8 cStyle, UINT8 * pcBuffer, UINT16 uiSize)	//A35G2_Coda_0045
{
    eRESULT eResult = rcERROR;
    sCLI_BUFFER_FORMAT sCLI_format = {'\0'};
    sCLI_format.cCh = cCh;
    sCLI_format.cStyle = cStyle;	//A35G2_Coda_0045
    snprintf(sCLI_format.cString, CLI_DATA_SIZE, "%s\0", pcBuffer);

    if(eResult == (eRESULT)utilQueueWrite(&m_asCLI_Ring_Buffer, (UINT8 *)&sCLI_format))
    {
        ASSERT_ALWAYS();
    }
}

// ==============================================================================
// FUNCTION NAME: appCLICmdProcess_Task
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/09/02, Larry Create
// --------------------
// ==============================================================================
static void* appCLICmdProcess_Task(void* pParameters)
{
    UINT8  ucHpbuTestEnable = 0;
    //char input = 255;

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    MS_SLEEP(500);

    while(!utilHost_IPC_Ready())
    {
        MS_SLEEP(10);
        //等ipc init做完
    }

    while(1)
    {
        sCLI_BUFFER_FORMAT sCLI_format;

        if(rcSUCCESS == (eRESULT)utilQueueRead(&m_asCLI_Ring_Buffer, (UINT8 *)&sCLI_format))
        {
            char ucReturnString[MSSC_CMD_LENGTH_MAX] = {'\0'};
#ifdef CUSTOM_CHRISTIE
            {
                sCLI_CHRISTIE_FORMAT sCmdFormat = {0};
                sCmdFormat.ucCmdFrom = sCLI_format.cCh;

                utilChristie_CLI_Process((eCLI_CHANNEL)sCLI_format.cCh, sCLI_format.cString, ucReturnString, &sCmdFormat, CLI_EXECTUE_NORMAL);
            }
#elif defined(CUSTOM_OPTOMA)//A35G2_Coda_0045
            {
                sCLI_OPTOMA_FORMAT sCmdFormat = {0};
                sCmdFormat.ucCmdFrom = sCLI_format.cCh;
                if(sCLI_format.cStyle == eCLI_STYLE_OPTOMA)//A35G2_Coda_0045
                {
                    utilOptoma_CLI_Process((eCLI_CHANNEL)sCLI_format.cCh, sCLI_format.cString, ucReturnString, &sCmdFormat, CLI_EXECTUE_NORMAL);
                }
            }
#ifdef GENERAL_MSSC_CLI
            {
                sCLI_GENERAL_FORMAT sCmdFormat = {0};
                sCmdFormat.ucCmdFrom = sCLI_format.cCh;
                if(sCLI_format.cStyle == eCLI_STYLE_GENERAL)//A35G2_Coda_0045
                {
                    utilGeneral_CLI_Process((eCLI_CHANNEL)sCLI_format.cCh, sCLI_format.cString, ucReturnString, &sCmdFormat, CLI_EXECTUE_NORMAL);
                }
            }
#endif /* GENERAL_MSSC_CLI */

#else
            {
                sCLI_GENERAL_FORMAT sCmdFormat = {0};
                sCmdFormat.ucCmdFrom = sCLI_format.cCh;

                utilGeneral_CLI_Process((eCLI_CHANNEL)sCLI_format.cCh, sCLI_format.cString, ucReturnString, &sCmdFormat, CLI_EXECTUE_NORMAL);
            }
#endif
        }
        else
        {
            MS_SLEEP(10);
        }

        palSystem_TaskMonitorTimerReset(eTID_CLI_CMD_PROC, NULL);  //A35G2_Simon_0075

	}
}

// ==============================================================================
// FUNCTION NAME: appCLICmdDecode_Task
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/09/02, Larry Create
// --------------------
// ==============================================================================
static void* appCLICmdDecode_Task(void* pParameters)
{
    UINT8  ucHpbuTestEnable = 0;
    //char input = 255;

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    MS_SLEEP(500);

#ifndef APP_BACKGROUND
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
        SetKeyboardNonBlock(&initial_settings);  //更改為 non-blocking mode , 否則後面使用 getchar() 會 block 住
#endif
#endif /* APP_BACKGROUND */

    while(!utilHost_IPC_Ready())
    {
        MS_SLEEP(10);
        //等ipc init做完
    }

    while(1)
    {
        char input = 255;

#ifndef APP_BACKGROUND
        input = getchar();

        if(input != 255)    //255 : no key
        {
            printf("%c", input);
        }

        if(input == 3)  //Ctrl + C
        {
            #if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
            RestoreKeyboardBlocking(&initial_settings);
            #endif
        }
#else
        Board_HW_Read_PCUART();
        Board_Uart_Read(uaPCUART, 1, &input);

        //if(input != 255)    //255 : no key
        //{
            //printf("%c", input);
        //}
#endif /* APP_BACKGROUND */

        if( input != -1 && input != 10 && input != 255)  //G100_Steven_0002
        {

            ucHpbuTestEnable = utilHPBUTest_CLI_Handle(input);    //G100_Wilsonj_0018 open

            if(!ucHpbuTestEnable)
            {
#if OPEN_WAP
            	 GEC_EventHandler(&input); //G100_Steven_0007
#endif

#if 0
                if(utilGeneral_CLI_Handle(eccPC, (UINT8)input))
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }
#endif

                utilCommonCLI_SWGEC_Handle(eccPC, (UINT8*)&input);

#if 1
#if defined(CUSTOM_CHRISTIE)
                if(utilChristie_CLI_Handle(eccPC, (UINT8*)&input))
                {
                    //palEnvironment_SleepTimerClear();
                }
#elif defined(CUSTOM_OPTOMA)
                if(utilOptoma_CLI_Handle(eccPC, (UINT8)input))// BruceLin#20210107
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }
#ifdef GENERAL_MSSC_CLI
                if(utilGeneral_CLI_Handle(eccPC, (UINT8)input))
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }
#endif /* GENERAL_MSSC_CLI */
#else
                if(utilGeneral_CLI_Handle(eccPC, (UINT8)input))
                {
                    //palEnvironment_SleepTimerClear(); //A70LV_Larry_0082 //A70LV_Larry_0130 mask
                }
#endif /* 0 */
#endif
            }
            //CLI_Command_Handle_HPBUTest();
        }
        else
        {
            MS_SLEEP(10);
        }

        palSystem_TaskMonitorTimerReset(eTID_CLI_CMD_DECODE, NULL);  //A35G2_Simon_0075

	}

#ifndef APP_BACKGROUND
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    RestoreKeyboardBlocking(&initial_settings);
#endif
#endif /* APP_BACKGROUND */

}

eEXEC_CODE appCLICmd_Init(void)
{
    UINT8 ucEcho = 0;
    INT16 iCreateThreadError = 0;

#if(CURRENT_RTOS_TYPE != RTOS_STATIC)
    BaseType_t xReturned;
#endif

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    //Create Mailbox
    LOG_MSG(db_APP_CLI, "CLI queue create start\n");
    if(palMailBox_Create_MsgQueue(&m_sAppCLICmdProcess.xMsgQueue, QUEUE_CLI_NAME, &m_sAppCLICmdProcess.xMsgQueue_attr) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
    LOG_MSG(db_APP_CLI, "CLI queue create end\n");

    //Create Thread
    pthread_attr_init(&m_sAppCLICmdProcess.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sAppCLICmdProcess.xTaskHandle_attr, APP_CLI_STACK_SIZE);
    pthread_attr_setdetachstate( &m_sAppCLICmdProcess.xTaskHandle_attr, PTHREAD_CREATE_DETACHED );

    //Create Thread
    pthread_attr_init(&m_sAppCLICmdDecode.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sAppCLICmdDecode.xTaskHandle_attr, APP_CLI_STACK_SIZE);
    pthread_attr_setdetachstate( &m_sAppCLICmdDecode.xTaskHandle_attr, PTHREAD_CREATE_DETACHED );


#ifndef SIMULATOR_ISCALER
    iCreateThreadError = pthread_create(&m_sAppCLICmdProcess.xTaskHandle, &m_sAppCLICmdProcess.xTaskHandle_attr, appCLICmdProcess_Task, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
    LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , m_sAppCLICmdProcess.xTaskHandle);

    iCreateThreadError = pthread_create(&m_sAppCLICmdDecode.xTaskHandle, &m_sAppCLICmdDecode.xTaskHandle_attr, appCLICmdDecode_Task, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
    LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , m_sAppCLICmdDecode.xTaskHandle);

    utilQueueInitial(&m_asCLI_Ring_Buffer, CLI_STRING_BUFFER, sizeof(sCLI_BUFFER_FORMAT) / sizeof(UINT8), (UINT8*)&m_sCLI_Buffer);
    utilCommon_Callback_fun(appCLI_StringInsert);
    utilCommon_Init();

    //utilGeneral_CLI_Init();

#if 1
#if defined(CUSTOM_CHRISTIE)
    utilChristie_CLI_Init();
#elif defined(CUSTOM_OPTOMA)
    utilOptoma_CLI_Init();
#ifdef GENERAL_MSSC_CLI
    utilGeneral_CLI_Init();
#endif /* GENERAL_MSSC_CLI */
#else
    utilGeneral_CLI_Init();
#endif /* 0 */
#endif


#else
    pthread_t pkey,pserial,pcli;
    pthread_create(&pkey, NULL, key, "key");
    pthread_create(&pserial, NULL, serial, "serial");
    pthread_create(&pcli, NULL, cli, "cli");
#endif

#else

    if(palMailBox_Create(&m_sAppCLICmdInfo.xMsgQueue, &m_sAppCLICmdInfo.xEventGroupHandle) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    m_sAppCLICmdInfo.lPollPeriod = 100;//Because lPollPeriod to 0 means max delay

    xReturned = xTaskCreate(appCLICmd_Task, (const char *) "CLITSK",
                APP_CLI_STACK_SIZE,
                NULL,
                APP_CLI_PRIORITY,
                &m_sAppCLICmdInfo.xTaskHandle);

    if(xReturned != pdPASS)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Task Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Task Pass!\r\n", __FUNCTION__, __LINE__);

#endif


    return eEXEC_CODE_PASS;
}

// ==============================================================================
// FUNCTION NAME: appCLICmd_SerialPortEcho
// DESCRIPTION:
//
//
// Params:
// UINT8 ucEcho:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/04, Larry Create
// --------------------
// ==============================================================================
void appCLICmd_SerialPortEcho(UINT8 ucEcho)
{
    if(sGlobalCfg.sFN_CFG.SupportSerialEcho)
    {
#if defined(CUSTOM_CHRISTIE)
        utilChristie_CLI_SerialPortEcho(eccPC, ucEcho);
#elif defined(CUSTOM_OPTOMA)
        utilOptoma_CLI_SerialPortEcho(eccPC, ucEcho);
#ifdef GENERAL_MSSC_CLI
        utilGeneral_CLI_SerialPortEcho(eccPC, ucEcho);
#endif /* GENERAL_MSSC_CLI */
#else
        utilGeneral_CLI_SerialPortEcho(eccPC, ucEcho);
#endif /* 0 */
    }
}

// ==============================================================================
// FUNCTION NAME: appCLICmd_ProjectorAddress
// DESCRIPTION:
//
//
// Params:
// UINT8 ucAddress:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/21, Larry Create
// --------------------
// ==============================================================================
void appCLICmd_ProjectorAddress(UINT8 ucAddress)
{
#if defined(CUSTOM_CHRISTIE)
    utilChristie_CLI_AddressID((UINT32)ucAddress);
#elif defined(CUSTOM_OPTOMA)
    utilOptoma_CLI_AddressID((UINT32)ucAddress);
#ifdef GENERAL_MSSC_CLI
    utilGeneral_CLI_AddressID((UINT32)ucAddress);
#endif /* GENERAL_MSSC_CLI */
#else
    utilGeneral_CLI_AddressID((UINT32)ucAddress);
#endif /* 0 */

    halMCU_ProjectorAddressSet((UINT8)ucAddress);
}

void RestoreKeyboardBlocking(struct termios *initial_settings)
{
	tcsetattr(0, TCSANOW, initial_settings);
}

void SetKeyboardNonBlock(struct termios *initial_settings)
{
    struct termios new_settings;
    tcgetattr(0,initial_settings);

    new_settings = *initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &new_settings);
}

int get_thread_policy(pthread_attr_t *attr)
{
    int policy;
    int rs = pthread_attr_getschedpolicy(attr,&policy);

    switch(policy)
    {
        case SCHED_FIFO:
            printf("policy= SCHED_FIFO\n");
            break;
        case SCHED_RR:
            printf("policy= SCHED_RR");
            break;
        case SCHED_OTHER:
            printf("policy=SCHED_OTHER\n");
            break;
        default:
            printf("policy=UNKNOWN\n");
            break;
    }
    return policy;
}

int get_thread_priority(pthread_attr_t *attr)
{
    struct sched_param param;
    int rs = pthread_attr_getschedparam(attr,&param);
    printf("priority=%d\n",param.__sched_priority);
    return param.__sched_priority;
}


