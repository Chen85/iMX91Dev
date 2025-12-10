#if defined(CUSTOM_OPTOMA)
// ===============================================================================
// FILE NAME: utilChristieMSSC.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================
#include "appSystem.h"
#include "appDataMgr.h"
#include "appInputProc.h"
#include "appLANProcAPI.h"
#include "appDataPath.h"
#include "appPoll.h"
#include "appIllumination.h"
#include "appEnvironment.h"
#include "appDataItemAvailableMgr.h"

#include "palCoreVar.h"

#include "halScaler.h"
#include "halFrontEndCtrlAPI.h"
#include "halMotorCtrlAPI.h"
#include "halFormatter.h"
#include "halInputCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "halWarping.h"

#include "utilCommon.h"
#include "utilDbgMsg.h"
#include "utilCommonMSSCAPI.h"
#include "utilHPBU_Tester.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "utilOPD_TEST.h"
#include "utilDataMapping.h"
#include "utilDatabaseAPI.h"
#include "utilDataMgrAPI.h"
#include "utilWarpDemo.h"
#include "utilGeneralMSSCAPI.h"

#include "dvVCXO949.h"
#include "Board_Uart.h"
#include "GEC_CoreFunction.h"
#include "GEC_EventTable.h"
#include "palMotorMgr.h"


#define OPTOMA_CMD_SPACE          ' '
#define OPTOMA_CMD_LENGTH_MAX     (230)
#define OPTOMA_CMD_LENGTH_MIN     (6) //(SYS?)\0

#define CLI_COM_NORMAL            (0)
#define CLI_COM_LENS              CLI_COM_NORMAL | CLI_COM_TYPE_PN_ONLY | CLI_COM_WRITE_ONLY
#ifdef CLI_ERROR_LOG_ENABLE //A65_OPTOMA_Jerry_0006	//A35G2_Coda_0045
#define OPTOMA_CMD_DEBUG_DISABLE        0
#define OPTOMA_CMD_DEBUG_STEP_01        1
#define OPTOMA_CMD_DEBUG_STEP_03        3
#define OPTOMA_CMD_DEBUG_ENABLE         5
UINT8 bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_DISABLE;
#endif

typedef enum
{
    eCLI_DECODE_STEP_HEADER = 0,
    //eCLI_DECODE_STEP_PRE_CHAR_CHECK, //PREFIX_CHAR check
    eCLI_DECODE_STEP_ADDRESS_CHECK,  //Projector Host check
    eCLI_DECODE_STEP_MAIN,
    eCLI_DECODE_STEP_SUB,
    eCLI_DECODE_STEP_DATA_FORMAT,
    eCLI_DECODE_STEP_END,

}eCLI_DECODE_STEP;
#if 0
typedef struct
{
    UINT8 ucInterFace;//PC,LAN
    UINT8 ucCLI_DATA[CLI_DATA_SIZE + 1];
    UINT16 uiCLI_Front;
    UINT16 uiCLI_End;
    UINT16 uiCLI_Poll_Position;
    UINT16 uiCLI_Length; //for B series, no header type
    UINT8  ucSerialPortEcho; //A70LV_Larry_0060

}sCLI_DATA_INFO;

typedef struct
{
    UINT8 ucUartCh;
    char  cCMD_Hander;
    char  cCMD_End;

}sCLI_CONFIG;
#endif
typedef struct
{
    char*  cMainCmd;
    UINT16 uiMainCmdID;

    UINT8  ucMainCmdLenght;

    char*  cSubCmd;
    UINT16 uiSubCmdID;

    UINT8  ucCmdIsRead;
    UINT8  ucCmdBytesOnRange;

    UINT16 uiFunCodeID;
    eCLI_ERROR_CODE (*iCLI_Replay)(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat);

    UINT16 uiSpecialFlag; //read only, write only,...

}sCLI_OPTOMA_LUT;

static sCLI_DATA_INFO m_sCLI_CustomInfo;
static UINT32 m_ulAddressID = 0;
static UINT8  m_ucWriteBusy = 0;

//Special case function reply

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_KEY
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_OPTOMA_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/02/09, Simon Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilOptoma_CLI_KEY(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0035 modify
{

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;

    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};
        sInputKey.eKeyEvent = eKEY_EVENT_PRESSED;
        sInputKey.eKeyType = eKEYINPUT_IR;
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_KEY_IR_POWER_OFF:
                sInputKey.wKeyCode = keSTANDBY;
                break;

            case eOptomaUART_KEY_IR_UP:
                sInputKey.wKeyCode = keUP;
                break;

            case eOptomaUART_KEY_IR_LEFT:
                sInputKey.wKeyCode = keLEFT;
                break;

            case eOptomaUART_KEY_IR_ENTER:
                sInputKey.wKeyCode = keENTER;
                break;

            case eOptomaUART_KEY_IR_RIGHT:
                sInputKey.wKeyCode = keRIGHT;
                break;

            case eOptomaUART_KEY_IR_DOWN:
                sInputKey.wKeyCode = keDOWN;
                break;

            case eOptomaUART_KEY_IR_V_KEYTONE_PLUS:
                sInputKey.wKeyCode = keVKEYSTONE_UP;
                break;

            case eOptomaUART_KEY_IR_V_KEYTONE_MINUS:
                sInputKey.wKeyCode = keVKEYSTONE_DOWN;
                break;

            case eOptomaUART_KEY_IR_BRIGHTNESS:
                sInputKey.wKeyCode = keBRIGHT;
                break;

            case eOptomaUART_KEY_IR_MENU:
                sInputKey.wKeyCode = keMENU;
                break;

            case eOptomaUART_KEY_IR_AV_MUTE:
            case eOptomaUART_KEY_IR_MUTE:
                sInputKey.wKeyCode = keMUTE;
                break;

            case eOptomaUART_KEY_IR_CONTRAST:
                sInputKey.wKeyCode = keCONTRAST;
                break;

            case eOptomaUART_KEY_IR_ZOOM_PLUS:
                sInputKey.wKeyCode = keZOOMOUT;
                break;

            case eOptomaUART_KEY_IR_ZOOM_MINUS:
                sInputKey.wKeyCode = keZOOMIN;
                break;

            case eOptomaUART_KEY_IR_FOCUS_PLUS:
                sInputKey.wKeyCode = keFOCUSUP;
                break;

            case eOptomaUART_KEY_IR_FOCUS_MINUS:
                sInputKey.wKeyCode = keFOCUSDOWN;
                break;

            case eOptomaUART_KEY_IR_MODE:
                sInputKey.wKeyCode = keMODE;
                break;

            case eOptomaUART_KEY_IR_INFO:
                sInputKey.wKeyCode = keINFO;
                break;

            case eOptomaUART_KEY_IR_RE_SYNC:
                sInputKey.wKeyCode = keAUTO;
                break;

            case eOptomaUART_KEY_IR_HDMI1:
                sInputKey.wKeyCode = ke1;
                break;

            case eOptomaUART_KEY_IR_HDMI2:
                sInputKey.wKeyCode = ke2;
                break;

            case eOptomaUART_KEY_IR_SOURCE:
                sInputKey.wKeyCode = keINPUT;
                break;

            case eOptomaUART_KEY_IR_1:
                sInputKey.wKeyCode = ke1;
                break;

            case eOptomaUART_KEY_IR_2:
                sInputKey.wKeyCode = ke2;
                break;

            case eOptomaUART_KEY_IR_3:
                sInputKey.wKeyCode = ke3;
                break;

            case eOptomaUART_KEY_IR_4:
                sInputKey.wKeyCode = ke4;
                break;

            case eOptomaUART_KEY_IR_5:
                sInputKey.wKeyCode = ke5;
                break;

            case eOptomaUART_KEY_IR_6:
                sInputKey.wKeyCode = ke6;
                break;

            case eOptomaUART_KEY_IR_7:
                sInputKey.wKeyCode = ke7;
                break;

            case eOptomaUART_KEY_IR_8:
                sInputKey.wKeyCode = ke8;
                break;

            case eOptomaUART_KEY_IR_9:
                sInputKey.wKeyCode = ke9;
                break;

            case eOptomaUART_KEY_IR_0:
                sInputKey.wKeyCode = ke0;
                break;

            case eOptomaUART_KEY_IR_GAMMA:
                sInputKey.wKeyCode = keGAMMA;
                break;

            case eOptomaUART_KEY_IR_PIP:
                sInputKey.wKeyCode = kePIP;
                break;

            case eOptomaUART_KEY_IR_LENS_H_LEFT:
                sInputKey.wKeyCode = keLENSHLEFT;
                break;

            case eOptomaUART_KEY_IR_LENS_H_RIGHT:
                sInputKey.wKeyCode = keLENSHRIGHT;
                break;

            case eOptomaUART_KEY_IR_LENS_V_LEFT:
                sInputKey.wKeyCode = keLENSVDOWN;
                break;

            case eOptomaUART_KEY_IR_LENS_V_RIGHT:
                sInputKey.wKeyCode = keLENSVUP;
                break;

            case eOptomaUART_KEY_IR_H_KEYSTONE_PUUS:
                sInputKey.wKeyCode = keHKEYSTONE_RIGHT;
                break;

            case eOptomaUART_KEY_IR_H_KEYSTONE_MINUS:
                sInputKey.wKeyCode = keHKEYSTONE_LEFT;
                break;

            case eOptomaUART_KEY_IR_HOTKEY_USER1:
                sInputKey.wKeyCode = keHOTKEY;
                break;

			case eOptomaUART_KEY_IR_HOTKEY_USER2:
                sInputKey.wKeyCode = keHOTKEY2;
                break;

            case eOptomaUART_KEY_IR_PATTERN:
                sInputKey.wKeyCode = keTEST;
                break;

            case eOptomaUART_KEY_IR_EXIT:
                sInputKey.wKeyCode = keBACK;
                break;

            case eOptomaUART_KEY_IR_DISPLAY_PORT:
                sInputKey.wKeyCode = ke3; //TBD
                break;

            case eOptomaUART_KEY_IR_RETURN:
                sInputKey.wKeyCode = keBACK;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palInputProc_InputKeycode(sInputKey.wKeyCode);
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_PWR
// DESCRIPTION:
//
//
// Params:
// eCLI_MODE eAccessMode:
// sCLI_OPTOMA_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilOptoma_PWR(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0034
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

    if(eAccessMode == ecmRead)
    {
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            sCmdFormat->lData = 0;
        }
        else
        {
            sCmdFormat->lData = 1;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 0)// || sCmdFormat->lData == 2)  //A65_OPTOMA_Doulas_0100 Modify //A65_OPTOMA_John_0002 fix Optoma telnet command parser
        {
            UINT8 ucData = 0;
            UINT8 ucPwrStatus =	palSystem_PowerStateGet();//A35G2_David_0008

            palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucData);
            if(ucPwrStatus == ePOWER_STATE_ACTIVE && palSystem_SystemReadyGet()) //A35G2_David_0008
            {
                if(palDataPath_DataPathIsReady())
                {
                    if (ucData)
                    {
                        palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
                        palDataMgr_Camera_OSD_Lock_Set(eOSD_LOCKED);
                        palEnvironment_Fake_Power_Down_Set(TRUE);
                    }
                    else
                    {
                        palSystem_PowerDown();
                    }
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else// for  ~0000 0 command //R70G2_Optoma_David_0151//A35G2_David_0008
            {
                //LOG_MSG(db_ALWAYS, "~0000 0: %d, \r\n", eCLI_ERROR_CODE_SETFAIL);
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(sCmdFormat->lData == 1)//A65_OPTOMA_John_0002 fix Optoma telnet command parser
        {
            UINT8 ucData = (UINT8)palEnvironment_Fake_Power_Down_Get();
            UINT8 ucPwrStatus =	palSystem_PowerStateGet();

            if(ucPwrStatus != ePOWER_STATE_COOLING && ucPwrStatus != ePOWER_STATE_ACTIVE) //A65_OPTOMA_Julie_0073//A35G2_David_0008
            {
                if(ucData)
                {
                    ucData = FALSE;
                    palEnvironment_Fake_Power_Down_Set(FALSE);
                    palDataMgr_Camera_OSD_Lock_Set(eOSD_NOT_LOCKED);	//G100_Clare_0056
                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
                }
                palSystem_PowerKey();
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            eErrorCode = eCLI_ERROR_CODE_SETFAIL;
        }
    }

    return eErrorCode;
}

eCLI_ERROR_CODE utilOptoma_SIN(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //ZU860_Doulas_0027 modify//A70LV_Doulas_0135
{
    UINT16 DataCode = edcMAIN_INPUT;
    INT32 Value = eCM_SOURCE_RESERVED;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    if(eAccessMode == ecmRead)
    {
        if(palDataPath_IsSourceLock() == FALSE)
        {
            sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL;
            return eCLI_ERROR_CODE_NO;
        }

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaREAD, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(Value)
        {
            default:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL;// 0
                break;

            case eCM_SOURCE_HDMI1:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_HDMI1;// 7
                break;

            case eCM_SOURCE_HDMI2:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_HDMI2;// 8
                break;

            case eCM_SOURCE_DVI:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_DVI_D;// 1
                break;

            case eCM_SOURCE_3GSDI:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_3G_SDI;// 18
                break;

            case eCM_SOURCE_HDBASET:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_HDBASET;// 16
                break;

            case eCM_SOURCE_12GSDI:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_12G_SDI;
                break;

            case eCM_SOURCE_DISPLAYPORT:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_DP;
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            default:
                return eCLI_ERROR_CODE_DATAOVERRANGE;
                break;

            case eOptomaUART_MAIN_SOURCE_SET_HDMI1:// 1
                Value = eCM_SOURCE_HDMI1;
                break;

            case eOptomaUART_MAIN_SOURCE_SET_HDMI2:// 15
                Value = eCM_SOURCE_HDMI2;
                break;

			#if 0
            case eOptomaUART_MAIN_SOURCE_SET_DVI_D:// 2
                Value = eCM_SOURCE_DVI;
                break;

            case eOptomaUART_MAIN_SOURCE_SET_3G_SDI:// 22
                Value = eCM_SOURCE_3GSDI;
                break;
			#endif

            case eOptomaUART_MAIN_SOURCE_SET_HDBASET:// 21
                Value = eCM_SOURCE_HDBASET;
                break;

			#if 0
            case eOptomaUART_MAIN_SOURCE_SET_12G_SDI:
                Value = eCM_SOURCE_12GSDI;
                break;
			#endif

            case eOptomaUART_MAIN_SOURCE_SET_DP:
                Value = eCM_SOURCE_DISPLAYPORT;
                break;

        }

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SIN_Name(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)  //ZU860_Doulas_0114
{
    UINT16 DataCode = edcMAIN_INPUT;
    INT32 Value = eCM_SOURCE_RESERVED;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaREAD, &Value))
        {
        return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(Value)
        {
            default:
                return eCLI_ERROR_CODE_DATAOVERRANGE;
                break;

            case eCM_SOURCE_VGA:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "VGA");
                break;

            case eCM_SOURCE_HDMI1:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "HDMI1");
                break;

            case eCM_SOURCE_HDMI2:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "HDMI2");
                break;

            case eCM_SOURCE_DVI:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "DVI");
                break;

            case eCM_SOURCE_DISPLAYPORT:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "DISPLAY PORT");
                break;

            case eCM_SOURCE_3GSDI:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "3G-SDI");
                break;

            case eCM_SOURCE_HDBASET:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "HDBaseT");
                break;

            case eCM_SOURCE_12GSDI:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "12G-SDI");
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SIN_PIIP(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0076 Modify//ZU860_Doulas_0027 modify//A70LV_Doulas_0135
{
    UINT16 DataCode = edcSUB_INPUT;
    INT32 Value = eCM_SOURCE_RESERVED;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    if(eAccessMode == ecmRead)
    {
        if(palDataPath_IsSourceLockSub() == FALSE)
        {
            sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL;
            return eCLI_ERROR_CODE_NO;
        }

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaREAD, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(Value)
        {
            default:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_NO_SIGNAL;// 0
                break;

            case eCM_SOURCE_HDMI1:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_HDMI1;// 7
                break;

            case eCM_SOURCE_HDMI2:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_HDMI2;// 8
                break;

            case eCM_SOURCE_DVI:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_DVI_D;// 1
                break;

            case eCM_SOURCE_3GSDI:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_3G_SDI;// 17
                break;

            case eCM_SOURCE_HDBASET:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_HDBASET;// 16
                break;

            case eCM_SOURCE_DISPLAYPORT:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_DISPLAYPORT;
                break;

            case eCM_SOURCE_12GSDI:
                sCmdFormat->lData = eOptomaUART_SUB_SOURCE_RES_12G_SDI;
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            default:
                return eCLI_ERROR_CODE_DATAOVERRANGE;
                break;

            case eOptomaUART_SUB_SOURCE_SET_HDMI1:
                Value = eCM_SOURCE_HDMI1;
                break;

            case eOptomaUART_SUB_SOURCE_SET_HDMI2:
                Value = eCM_SOURCE_HDMI2;
                break;

            case eOptomaUART_SUB_SOURCE_SET_DVI_D:
                Value = eCM_SOURCE_DVI;
                break;

            case eOptomaUART_SUB_SOURCE_SET_3G_SDI:
                Value = eCM_SOURCE_3GSDI;
                break;

            case eOptomaUART_SUB_SOURCE_SET_HDBASET:
                Value = eCM_SOURCE_HDBASET;
                break;


            case eOptomaUART_SUB_SOURCE_SET_12G_SDI:
                Value = eCM_SOURCE_12GSDI;
                break;

            case eOptomaUART_SUB_SOURCE_SET_DISPLAYPORT:
                Value = eCM_SOURCE_DISPLAYPORT;
                break;
        }

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SIN_PIIP_Name(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //ZU860_Doulas_0114
{
    UINT16 DataCode = edcSUB_INPUT;
    INT32 Value = eCM_SOURCE_RESERVED;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaREAD, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(Value)
        {
            default:
                return eCLI_ERROR_CODE_DATAOVERRANGE;
                break;

            case eCM_SOURCE_VGA:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "VGA");
                break;

            case eCM_SOURCE_HDMI1:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "HDMI1");
                break;

            case eCM_SOURCE_HDMI2:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "HDMI2");
                break;

            case eCM_SOURCE_DVI:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "DVI");
                break;

            case eCM_SOURCE_DISPLAYPORT:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "DISPLAY PORT");
                break;

            case eCM_SOURCE_3GSDI:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "3G-SDI");
                break;

            case eCM_SOURCE_HDBASET:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "HDBaseT");
                break;

            case eCM_SOURCE_12GSDI:
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "12G-SDI");
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

// ??security on ?��? ~00112 1 ~1234
eCLI_ERROR_CODE utilOptoma_DEF(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A70LV_Doulas_0138
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    UINT8 ucPIN_PROTECT_EN;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edcFACTORY_RESET))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(palDataMgr_Data_Access(edcPIN_PROTECT, edaREAD, &ucPIN_PROTECT_EN) != eEXEC_CODE_PASS)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(ucPIN_PROTECT_EN == ets_OFF)
        {
            if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(sCmdFormat->lData == 1)      //ZU860_Doulas_0041 modify
            {
                eResult = palDataMgr_Data_Access(edcFACTORY_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucValue) ;
                if(eResult != eEXEC_CODE_PASS)
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else
                return eCLI_ERROR_CODE_SETFAIL;
        }
        else //PIN Protect On
        {
            UINT8 ucPWord[5] = {0};
            UINT8 ucPWord_Input[5] = {0};
            UINT8 ucVAl;
            UINT8 ucStringLen = 0;

            if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            ucStringLen = strlen((char*)sCmdFormat->cTextString);

            if(ucStringLen != 7)    //1~1234 ,7bytes
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if (sCmdFormat->cTextString[0] != '1')  //data value
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))  //check command format
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            if(palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord) != eEXEC_CODE_PASS)  //get password
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
            //LOG_MSG(db_ALWAYS, " $$String = %s (%s)\r\n", sCmdFormat->cTextString, ucPWord);

            for(ucVAl = 0 ; ucVAl < 4 ; ucVAl++)  //password 4 bytes
            {
                if((sCmdFormat->cTextString[ucVAl+3] >= '0') &&
                (sCmdFormat->cTextString[ucVAl+3] <= '9'))
                {
                    ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl+3] ;
                    if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            ucPIN_PROTECT_EN = ets_OFF;
            palDataMgr_Data_Access(edcPIN_PROTECT, edaWRITE_THROUGH_WITH_ACTION, &ucPIN_PROTECT_EN) ;
            eResult = palDataMgr_Data_Access(edcFACTORY_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucValue) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_OSD(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)       //A70LV_Doulas_0163
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucValue = 0;

        palDataMgr_Data_Access(edcSHOW_MESSAGES, edaREAD, &ucValue);
        sCmdFormat->lData = ucValue;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue;
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->lData <= ets_ON) && (sCmdFormat->lData >= ets_OFF))
        {
            ucValue = (UINT8)sCmdFormat->lData;
            palDataMgr_Data_Access(edcSHOW_MESSAGES, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LIF_TPHS(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A70LV_Larry_0268
{
    UINT16 DataCode = edcTOTAL_PROJECTOR_HOURS;

    if(eAccessMode == ecmRead)
    {
        UINT32 ulVlaue1 = 0;
        UINT32 ulVlaue2 = 0;

        palDataMgr_Data_Access(DataCode, edaREAD, &ulVlaue1) ;

        if(ulVlaue1 > ulVlaue2)
        {
            sCmdFormat->lData= (INT32)(ulVlaue1/60);
        }
        else
        {
            sCmdFormat->lData= (INT32)(ulVlaue2/60);
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT32 ulVlaue = (UINT32)sCmdFormat->lData;
        INT32  iMax = 0;
        INT32  iMin = 0;

        ulVlaue = ulVlaue * 60;
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(DataCode, edrMAX, &iMax)) &&
            (eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(DataCode, edrMIN, &iMin)))
        {

            if((iMax >= (INT32)ulVlaue) && ((INT32)ulVlaue >= iMin))
            {
                palIllumination_Init_TotalProjectHoursSet(ulVlaue);
                return eCLI_ERROR_CODE_NO;
            }
        }
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_DisplayMode(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0074 Modify//ZU860_Doulas_0022
{
    UINT16 DataCode = edcPICTURE_SETTINGS;
    INT32 Value = 0;
    UINT8 ucPictureMode = 0;

    if(eAccessMode == ecmRead)
    {
        if(palDataPath_IsSourceLock() == FALSE)
        {
            sCmdFormat->lData = 0;
            return eCLI_ERROR_CODE_NO;
        }

        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucPictureMode);
        if(ucPictureMode == eCM_PICTURE_SETTINGS_USER)
        {
            DataCode = edcUSER_COLOR_MODE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
		if(ucPictureMode == eCM_PICTURE_SETTINGS_DICOMSIM) //A35G2_Coda_0061//H30K_David_0009
		{
			Value = CLI_PICTURE_SETTINGS_DICOMSIM_READ;
		}

        sCmdFormat->lData = Value;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check //A65_OPTOMA_Julie_0062
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(
            (sCmdFormat->lData == CLI_PRE_USER_BRIGHT) ||
            (sCmdFormat->lData == CLI_PRE_USER_HDR) ||
            (sCmdFormat->lData == CLI_PRE_USER_DICOMSIM) ||
            (sCmdFormat->lData == CLI_PRE_USER_BLENDING) ||
            (sCmdFormat->lData == CLI_PRE_USER_3D) ||
            (sCmdFormat->lData == CLI_PRE_USER_2D_HIGH_SPEED))
        {
            DataCode = edcUSER_COLOR_MODE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        //Check 2D hight speed/ 3D
        if(palDataMgr_IS_3D_Enable())
        {
            if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_3D) &&
                (sCmdFormat->lData != CLI_PRE_USER_3D))
            return eCLI_ERROR_CODE_SETFAIL;
        }
        else if(halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
        {
            if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_2D_HIGH_SPEED) &&
                (sCmdFormat->lData != CLI_PRE_USER_2D_HIGH_SPEED))
            return eCLI_ERROR_CODE_SETFAIL;
        }
        else
        {
            if((sCmdFormat->lData == CLI_PICTURE_SETTINGS_2D_HIGH_SPEED) ||
                (sCmdFormat->lData == CLI_PICTURE_SETTINGS_3D) ||
                (sCmdFormat->lData == CLI_PRE_USER_2D_HIGH_SPEED) ||
                (sCmdFormat->lData == CLI_PRE_USER_3D))
            return eCLI_ERROR_CODE_SETFAIL;
        }

        //Check HDR
        palDataMgr_Data_Access(edcHDR_AUTOENABLE, edaREAD, &Value);
        if((Value == ets_ON) && (palDataPath_HDR_Info_Get() == eHDR_SETTING_HDR))
        {
            if((sCmdFormat->lData != CLI_PICTURE_SETTINGS_HDR) &&
                (sCmdFormat->lData != CLI_PRE_USER_HDR))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            if((sCmdFormat->lData == CLI_PICTURE_SETTINGS_HDR) ||
                (sCmdFormat->lData == CLI_PRE_USER_HDR))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        if((sCmdFormat->lData == CLI_PRE_USER_BRIGHT) ||
            (sCmdFormat->lData == CLI_PRE_USER_HDR) ||
            (sCmdFormat->lData == CLI_PRE_USER_DICOMSIM) ||
            (sCmdFormat->lData == CLI_PRE_USER_BLENDING) ||
            (sCmdFormat->lData == CLI_PRE_USER_3D) ||
            (sCmdFormat->lData == CLI_PRE_USER_2D_HIGH_SPEED))
        {
            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucPictureMode);
            if(ucPictureMode != eCM_PICTURE_SETTINGS_USER)
            {
                Value = CLI_PICTURE_SETTINGS_USER;
                DataCode = edcPICTURE_SETTINGS;
                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            DataCode = edcUSER_COLOR_MODE;
            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_WallColor(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0022
{
    UINT16 DataCode = edcWALL_COLOR;
    INT32 Value = eAF_AC_EXECUTE;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Gamma(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0101 modify//ZU860_Doulas_0022
{
    UINT16 DataCode = edcGAMMA;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ColorTemperature(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0022
{
    UINT16 DataCode = edcCOLOR_TEMPERATURE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))//H30K_David_0015
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value-1;//H30K_David_0015
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ColorSpace(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0023
{
    UINT16 DataCode = edcCOLOR_SPACE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Sub_ColorSpace(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)//HICC2_Julie_0004
{
    UINT16 DataCode = edcSUB_COLOR_SPACE;
    INT32 Value = 0;
    char aucString[32] = {0};

    if(eAccessMode == ecmRead)
    {
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        palDataMgr_Data_Access(DataCode, edaREAD, &aucString);

        if(eEXEC_CODE_PASS == palDataMgr_Data_Access(DataCode, edaREAD, &aucString))
        {
            if(strcmp(aucString , "RGB") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_RGB_RGB;
            }
            else if(strcmp(aucString , "RGB(0-255)") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_RGB_RGB;
            }
            else if(strcmp(aucString , "YUV") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_YUV;
            }
            else if((strcmp(aucString , "RGB(16-235)") == 0 ) || (strcmp(aucString , "RGB Video") == 0 ))
            {
                Value = eOptomaUART_COLOR_SPACE_RGB_RGB_VIDEO;
            }
            else if(strcmp(aucString , "REC709") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_REC709;
            }
            else if(strcmp(aucString , "REC601") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_REC601;
            }
            else
            {
                return eCLI_ERROR_CODE_ReplyFail;
            }

            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Main_ColorSpace(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)//HICC2_Julie_0004
{
    UINT16 DataCode = edcMAIN_COLOR_SPACE;
    INT32 Value = 0;
    char aucString[32] = {0};

    if(eAccessMode == ecmRead)
    {
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        palDataMgr_Data_Access(DataCode, edaREAD, &aucString);

        if(eEXEC_CODE_PASS == palDataMgr_Data_Access(DataCode, edaREAD, &aucString))
        {
            if(strcmp(aucString , "RGB") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_RGB_RGB;
            }
            else if(strcmp(aucString , "RGB(0-255)") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_RGB_RGB;
            }
            else if(strcmp(aucString , "YUV") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_YUV;
            }
            else if((strcmp(aucString , "RGB(16-235)") == 0 ) || (strcmp(aucString , "RGB Video") == 0 ))
            {
                Value = eOptomaUART_COLOR_SPACE_RGB_RGB_VIDEO;
            }
            else if(strcmp(aucString , "REC709") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_REC709;
            }
            else if(strcmp(aucString , "REC601") == 0 )
            {
                Value = eOptomaUART_COLOR_SPACE_REC601;
            }
            else
            {
                return eCLI_ERROR_CODE_ReplyFail;
            }

            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ColorWheelSpeed(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)    //A65_OPTOMA_Julie_0064 //ZU860_Doulas_0023
{
    UINT16 DataCode = edcCOLOR_WHEEL_SPEED;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == palDataMgr_Data_Access((eDATA_CODE)DataCode, edaREAD, &Value))
        {
            sCmdFormat->lData = Value;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        Value = sCmdFormat->lData;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BrightnessMode(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0023
{
    UINT16 DataCode = edcPOWER_MODE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
         }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

    }

    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_AspectRatio(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0025
{
    UINT16 DataCode = edcSIZE_PRESETS;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_FourCorners(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0026
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT16 uwValue = 0;
        INT32  idMax = 0;
        INT32  idMin = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_4CORNER_TL_RT:
            case eOptomaUART_4CORNER_TL_LT:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edc4CORNER_TOP_LEFT_HORZ))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }

                palDataMgr_Data_Access(edc4CORNER_TOP_LEFT_HORZ, edaREAD, &uwValue);

                if(sCmdFormat->lData == eOptomaUART_4CORNER_TL_RT)
                {
                    palDataMgr_Data_Range_Get(edc4CORNER_TOP_LEFT_HORZ, edrMAX, &idMax);

                    if((INT32)uwValue >= idMax)
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                    else
                    {
                        uwValue++;
                    }
                }
                else
                {
                    palDataMgr_Data_Access(edc4CORNER_TOP_LEFT_HORZ, edrMIN, &idMin);

                    if((INT32)uwValue <= idMin)
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                    else
                    {
                        uwValue--;
                    }
                }

                palDataMgr_Data_Access(edc4CORNER_TOP_LEFT_HORZ, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
                break;

            case eOptomaUART_4CORNER_TL_UP:
            case eOptomaUART_4CORNER_TL_DN:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edc4CORNER_TOP_LEFT_VERT))  //available check
                {
                return eCLI_ERROR_CODE_SETFAIL;
                }

                palDataMgr_Data_Access(edc4CORNER_TOP_LEFT_VERT, edaREAD, &uwValue);
                if(sCmdFormat->lData == eOptomaUART_4CORNER_TL_DN)
                {
                palDataMgr_Data_Range_Get(edc4CORNER_TOP_LEFT_VERT, edrMAX, &idMax);
                if((INT32)uwValue >= idMax)
                {
                return eCLI_ERROR_CODE_SETFAIL;
                }
                else
                uwValue++;
                }
                else
                {
                palDataMgr_Data_Range_Get(edc4CORNER_TOP_LEFT_VERT, edrMIN, &idMin);
                if((INT32)uwValue <= idMin)
                {
                return eCLI_ERROR_CODE_SETFAIL;
                }
                else
                uwValue--;
                }

                palDataMgr_Data_Access(edc4CORNER_TOP_LEFT_VERT, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
                break;

    case eOptomaUART_4CORNER_TR_RT:
    case eOptomaUART_4CORNER_TR_LT:
    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edc4CORNER_TOP_RIGHT_HORZ))  //available check
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }

    palDataMgr_Data_Access(edc4CORNER_TOP_RIGHT_HORZ, edaREAD, &uwValue);
    if(sCmdFormat->lData == eOptomaUART_4CORNER_TR_LT)
    {
    palDataMgr_Data_Range_Get(edc4CORNER_TOP_RIGHT_HORZ, edrMAX, &idMax);
    if((INT32)uwValue >= idMax)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue++;
    }
    else
    {
    palDataMgr_Data_Range_Get(edc4CORNER_TOP_RIGHT_HORZ, edrMIN, &idMin);
    if((INT32)uwValue <= idMin)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue--;
    }

    palDataMgr_Data_Access(edc4CORNER_TOP_RIGHT_HORZ, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
    break;

    case eOptomaUART_4CORNER_TR_UP:
    case eOptomaUART_4CORNER_TR_DN:
    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edc4CORNER_TOP_RIGHT_VERT))  //available check
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }

    palDataMgr_Data_Access(edc4CORNER_TOP_RIGHT_VERT, edaREAD, &uwValue);
    if(sCmdFormat->lData == eOptomaUART_4CORNER_TR_DN)
    {
    palDataMgr_Data_Range_Get(edc4CORNER_TOP_RIGHT_VERT, edrMAX, &idMax);
    if((INT32)uwValue >= idMax)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue++;
    }
    else
    {
    palDataMgr_Data_Range_Get(edc4CORNER_TOP_RIGHT_VERT, edrMIN, &idMin);
    if((INT32)uwValue <= idMin)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue--;
    }

    palDataMgr_Data_Access(edc4CORNER_TOP_RIGHT_VERT, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
    break;

    case eOptomaUART_4CORNER_BL_RT:
    case eOptomaUART_4CORNER_BL_LT:
    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edc4CORNER_BOTTOM_LEFT_HORZ))  //available check
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_LEFT_HORZ, edaREAD, &uwValue);
    if(sCmdFormat->lData == eOptomaUART_4CORNER_BL_RT)
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_LEFT_HORZ, edrMAX, &idMax);
    if((INT32)uwValue >= idMax)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue++;
    }
    else
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_LEFT_HORZ, edrMIN, &idMin);
    if((INT32)uwValue <= idMin)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue--;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_LEFT_HORZ, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
    break;
    case eOptomaUART_4CORNER_BL_UP:
    case eOptomaUART_4CORNER_BL_DN:
    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edc4CORNER_BOTTOM_LEFT_VERT))  //available check
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_LEFT_VERT, edaREAD, &uwValue);
    if(sCmdFormat->lData == eOptomaUART_4CORNER_BL_UP)
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_LEFT_VERT, edrMAX, &idMax);
    if((INT32)uwValue >= idMax)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue++;
    }
    else
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_LEFT_VERT, edrMIN, &idMin);
    if((INT32)uwValue <= idMin)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue--;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_LEFT_VERT, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
    break;

    case eOptomaUART_4CORNER_BR_RT:
    case eOptomaUART_4CORNER_BR_LT:
    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edc4CORNER_BOTTOM_RIGHT_HORZ))  //available check
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_RIGHT_HORZ, edaREAD, &uwValue);
    if(sCmdFormat->lData == eOptomaUART_4CORNER_BR_LT)
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_RIGHT_HORZ, edrMAX, &idMax);
    if((INT32)uwValue >= idMax)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue++;
    }
    else
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_RIGHT_HORZ, edrMIN, &idMin);
    if((INT32)uwValue <= idMin)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue--;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_RIGHT_HORZ, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
    break;
    case eOptomaUART_4CORNER_BR_UP:
    case eOptomaUART_4CORNER_BR_DN:
    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(edc4CORNER_BOTTOM_RIGHT_VERT))  //available check
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_RIGHT_VERT, edaREAD, &uwValue);
    if(sCmdFormat->lData == eOptomaUART_4CORNER_BR_UP)
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_RIGHT_VERT, edrMAX, &idMax);
    if((INT32)uwValue >= idMax)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue++;
    }
    else
    {
    palDataMgr_Data_Range_Get(edc4CORNER_BOTTOM_RIGHT_VERT, edrMIN, &idMin);
    if((INT32)uwValue <= idMin)
    {
    return eCLI_ERROR_CODE_SETFAIL;
    }
    else
    uwValue--;
    }

    palDataMgr_Data_Access(edc4CORNER_BOTTOM_RIGHT_VERT, edaWRITE_THROUGH_WITH_ACTION, &uwValue);
    break;

    default:
    return eCLI_ERROR_CODE_SETFAIL;
    }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Layout(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0026
{
    UINT16 DataCode = edcMAIN_LAYOUT;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucDataIndex = sCmdFormat->lData;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &Value); //A65_OPTOMA_Julie_0070

        if((Value == eCM_SCREEN_MODE_PIP && (ucDataIndex < CLI_MAIN_LAYOUT_PIP_TOP_LEFT || ucDataIndex > CLI_MAIN_LAYOUT_PIP_BOTTOM_RIGHT)) || (Value == CLI_SCREEN_MODE_PBP && (ucDataIndex < CLI_MAIN_LAYOUT_PBP_MAIN_LEFT || ucDataIndex > CLI_MAIN_LAYOUT_PBP_MAIN_BOTTOM)))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Size(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0026
{
    UINT16 DataCode = edcSIZE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Projection(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0027
{
    UINT16 DataCode = edcPROJECTION_MDOE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
        return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LensLock(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0028
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucLensLock = 0;

        palDataMgr_Data_Access(edcLOCK_ALL_LENS_MOTORS, edaREAD, &ucLensLock) ;

        switch(ucLensLock)
        {
            case eLENS_LOCK_ALL_ALLOW:
                sCmdFormat->lData= (INT32)eOptomaUART_LENS_LOCK_RESPONSE_ALL_ALLOW;     //ZU860_Doulas_0065 modify
                break;

            case eLENS_LOCK_ALL_LOCKED:
                sCmdFormat->lData= (INT32)eOptomaUART_LENS_LOCK_RESPONSE_ALL_LOCKED;    //ZU860_Doulas_0065 modify
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucLensLock = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcLOCK_ALL_LENS_MOTORS))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_LENS_LOCK_ALL_ALLOW:
                ucLensLock = eLENS_LOCK_ALL_ALLOW;
                break;

            case eOptomaUART_LENS_LOCK_ALL_LOCKED:
                ucLensLock = eLENS_LOCK_ALL_LOCKED;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Access(edcLOCK_ALL_LENS_MOTORS, edaWRITE_THROUGH_WITH_ACTION, &ucLensLock) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LensShift(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0028
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            if(palEnvironment_LensCalFlag_Get() == FALSE) //A65_OPTOMA_Julie_0070
            {
                //optoma model, TBD //appGui_Send_MenuOpen_Lens_Calibration_ConfirmDialog();
            }
            else
            {
                sInputKey.wKeyCode = keLENS;

                if(sCmdFormat->ucCmdFrom == eccTelnet)
                {
                    sInputKey.eKeyType = eKEYINPUT_LAN;
                }
                else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
                {
                    sInputKey.eKeyType = eKEYINPUT_HDBaseT;
                }
                else
                {
                    sInputKey.eKeyType = eKEYINPUT_CLI;
                }

                switch(sCmdFormat->lData)
                {
                    case eOptomaUART_LENS_SHIFT_DOWN:
                        palInputProc_KeyEventSend(sInputKey);
                        halMotorLensSet(eLENS_CMDS_STEP_DOWN);
                        break;

                    case eOptomaUART_LENS_SHIFT_UP:
                        palInputProc_KeyEventSend(sInputKey);
                        halMotorLensSet(eLENS_CMDS_STEP_UP);
                        break;

                    case eOptomaUART_LENS_SHIFT_RIGHT:
                        palInputProc_KeyEventSend(sInputKey);
                        halMotorLensSet(eLENS_CMDS_STEP_RIGHT);
                        break;

                    case eOptomaUART_LENS_SHIFT_LEFT:
                        palInputProc_KeyEventSend(sInputKey);
                        halMotorLensSet(eLENS_CMDS_STEP_LEFT);
                        break;

                    default:
                        return eCLI_ERROR_CODE_SETFAIL;
                }
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Zoom(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0028
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            if(sCmdFormat->ucCmdFrom == eccTelnet)
            {
                sInputKey.eKeyType = eKEYINPUT_LAN;
            }
            else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
            {
                sInputKey.eKeyType = eKEYINPUT_HDBaseT;
            }
            else
            {
                sInputKey.eKeyType = eKEYINPUT_CLI;
            }

            switch(sCmdFormat->lData)
            {
                case eOptomaUART_ZOOM_PLUS:
                    sInputKey.wKeyCode = keZOOMOUT;
                    palInputProc_KeyEventSend(sInputKey);
                    halMotorZoomSet(eZOOMLENS_CMD_ZOOM_OUT_STEP);
                    break;

                case eOptomaUART_ZOOM_MINUS:
                    sInputKey.wKeyCode = keZOOMIN;
                    palInputProc_KeyEventSend(sInputKey);
                    halMotorZoomSet(eZOOMLENS_CMD_ZOOM_IN_STEP);
                    break;

                default:
                    return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Focus(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0028
{
    INT32 Value = eAF_AC_EXECUTE;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        sKEY_DATA sInputKey = {0};

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palInputProc_LensMotorLockGet() == eLENS_LOCK_ALL_ALLOW)
        {
            if(sCmdFormat->ucCmdFrom == eccTelnet)
            {
                sInputKey.eKeyType = eKEYINPUT_LAN;
            }
            else if(sCmdFormat->ucCmdFrom == eccHDBaseT)
            {
                sInputKey.eKeyType = eKEYINPUT_HDBaseT;
            }
            else
            {
                sInputKey.eKeyType = eKEYINPUT_CLI;
            }

            switch(sCmdFormat->lData)
            {
                case eOptomaUART_FOCUS_PLUS:
                    sInputKey.wKeyCode = keFOCUSUP;
                    palInputProc_KeyEventSend(sInputKey);
                    halMotorFocusSet(eZOOMLENS_CMD_FOCUS_FAR_STEP);
                    break;

                case eOptomaUART_FOCUS_MINUS:
                    sInputKey.wKeyCode = keFOCUSDOWN;
                    palInputProc_KeyEventSend(sInputKey);
                    halMotorFocusSet(eZOOMLENS_CMD_FOCUS_NEAR_STEP);
                    break;

                case eOptomaUART_AUTO_FOCUS_EXECUTE:
                    if(eFUNC_CONTROL_ENABLE != palDataMgr_Control_Camera_Lens())
                    {
                        LOG_MSG(db_APP_GUI, "%s[%d]: %s() appDataMgr_Control_Camera_Lens() fail\r\n", __FILE__, __LINE__, __FUNCTION__);
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcAUTO_FOCUS, edaWRITE_RAM_ONLY_WITH_ACTION, &Value))
                    {
                        LOG_MSG(db_APP_GUI, "%s[%d]: %s() palDataMgr_Data_Access(edcAUTO_FOCUS) fail\r\n", __FILE__, __LINE__, __FUNCTION__);
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                    break;

                default:
                    return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Language(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)      //ZU860_Owen_0009 add
{
    UINT16 DataCode = edcLANGUAGE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_AutoPowerOff(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0029
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucAutoPowerOff = 0;

        palDataMgr_Data_Access(edcAUTO_SHUTDOWN, edaREAD, &ucAutoPowerOff) ;
        sCmdFormat->lData= (INT32)ucAutoPowerOff;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucAutoPowerOff = 0;
        INT32 iMaxVal, iMinVal;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcAUTO_SHUTDOWN))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 1) //input 1 fail  //A35G2_Coda_0062
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Range_Get(edcAUTO_SHUTDOWN, edrMAX, &iMaxVal);
        palDataMgr_Data_Range_Get(edcAUTO_SHUTDOWN, edrMIN, &iMinVal);

        if((sCmdFormat->lData > iMaxVal)||(sCmdFormat->lData < iMinVal))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        ucAutoPowerOff = (UINT8)(sCmdFormat->lData / AUTO_SHUTDOWN_TIMER_STEP_VALUE) * AUTO_SHUTDOWN_TIMER_STEP_VALUE;
        palDataMgr_Data_Access(edcAUTO_SHUTDOWN, edaWRITE_THROUGH_WITH_ACTION, &ucAutoPowerOff) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SleepTimer(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0029
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucSleepTimer = 0;

        palDataMgr_Data_Access(edcSLEEP_TIMER, edaREAD, &ucSleepTimer) ;
        sCmdFormat->lData= (INT32)ucSleepTimer * SLEEP_TIMER_STEP_VALUE;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucSleepTimer = 0;
        INT32 iMaxVal, iMinVal;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcSLEEP_TIMER))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Range_Get(edcSLEEP_TIMER, edrMAX, &iMaxVal);
        palDataMgr_Data_Range_Get(edcSLEEP_TIMER, edrMIN, &iMinVal);

        if((sCmdFormat->lData > (iMaxVal*SLEEP_TIMER_STEP_VALUE))||(sCmdFormat->lData < (iMinVal*SLEEP_TIMER_STEP_VALUE)))  //A35G2_Coda_0062
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(SLEEP_TIMER_STEP_VALUE != 0) //A65_OPTOMA_Julie_0062  //A35G2_Coda_0062
        {
            ucSleepTimer = (UINT8)(sCmdFormat->lData / SLEEP_TIMER_STEP_VALUE);
            palDataMgr_Data_Access(edcSLEEP_TIMER, edaWRITE_THROUGH_WITH_ACTION, &ucSleepTimer) ;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_PowerMode(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0030
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucPowerMode = 0;

        palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, &ucPowerMode) ;

        switch(ucPowerMode)
        {
            case eSTANDBY_MODE_2W:
                sCmdFormat->lData= (INT32)eOptomaUART_STANDBY_MODE_2W;
                break;

            case eSTANDBY_MODE_0d5W:
                sCmdFormat->lData= (INT32)eOptomaUART_STANDBY_MODE_0d5W;
                break;

            case eSTANDBY_MODE_COMMUNICATION:
                sCmdFormat->lData= (INT32)eOptomaUART_STANDBY_MODE_COMMUNICATION;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucPowerMode = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcSTANDBY_MODE))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_STANDBY_MODE_2W:
                ucPowerMode = eSTANDBY_MODE_2W;
                break;

            case eOptomaUART_STANDBY_MODE_0d5W:
                ucPowerMode = eSTANDBY_MODE_0d5W;
                break;

            case eOptomaUART_STANDBY_MODE_COMMUNICATION:
                ucPowerMode = eSTANDBY_MODE_COMMUNICATION;
                break;

            default:
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcSTANDBY_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucPowerMode) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LensApplyPosition(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0030
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucApplyPosition = 0;

        palDataMgr_Data_Access(edcLENS_APPLY_POSITION, edaREAD, &ucApplyPosition) ;
        sCmdFormat->lData= (INT32)ucApplyPosition + 1;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucApplyPosition = 0;
        UINT16 ucLMAP_Index = 0x01 << (sCmdFormat->lData-1);    //A35G2_Owen_0006

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcLENS_APPLY_POSITION)) ||  //available check
            !(ucLMAP_Index & palEnvironment_LensMemorySavingCondition_Get()))    //A35G2_Owen_0006
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_APPLY_POSITION_1:
                ucApplyPosition = 0;
                break;

            case eOptomaUART_APPLY_POSITION_2:
                ucApplyPosition = 1;
                break;

            case eOptomaUART_APPLY_POSITION_3:
                ucApplyPosition = 2;
                break;

            case eOptomaUART_APPLY_POSITION_4:
                ucApplyPosition = 3;
                break;

            case eOptomaUART_APPLY_POSITION_5:
                ucApplyPosition = 4;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Access(edcLENS_APPLY_POSITION, edaWRITE_THROUGH_WITH_ACTION, &ucApplyPosition) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LensSaveCurrentPosition(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0030
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucSaveCurrentPosition = 0;

        palDataMgr_Data_Access(edcLENS_SAVE_CURRENT_POSITION, edaREAD, &ucSaveCurrentPosition) ;
        sCmdFormat->lData= (INT32)ucSaveCurrentPosition + 1;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucSaveCurrentPosition = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcLENS_SAVE_CURRENT_POSITION))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_SAVE_CURRENT_POSITION_1:
                ucSaveCurrentPosition = 0;
                break;

            case eOptomaUART_SAVE_CURRENT_POSITION_2:
                ucSaveCurrentPosition = 1;
                break;

            case eOptomaUART_SAVE_CURRENT_POSITION_3:
                ucSaveCurrentPosition = 2;
                break;

            case eOptomaUART_SAVE_CURRENT_POSITION_4:
                ucSaveCurrentPosition = 3;
                break;

            case eOptomaUART_SAVE_CURRENT_POSITION_5:
                ucSaveCurrentPosition = 4;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcLENS_SAVE_CURRENT_POSITION, edaWRITE_THROUGH_WITH_ACTION, &ucSaveCurrentPosition) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_TestPattern(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0031
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucTestPattern = 0;

        palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaREAD, &ucTestPattern) ;

        switch(ucTestPattern)
        {
            case eOSD_TP_OFF:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_OFF;
                break;

            case eOSD_TP_GREEN_GRID:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_GREEN_GRID;
                break;

            case eOSD_TP_MAGENTA_GRID:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_MAGENTA_GRID;
                break;

            case eOSD_TP_WHITE_GRID:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_WHITE_GRID;
                break;

            case eOSD_TP_WHITE:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_WHITE;
                break;

            case eOSD_TP_BLACK:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_BLACK;
                break;

            case eOSD_TP_RED:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_RED;
                break;

            case eOSD_TP_GREEN:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_GREEN;
                break;

            case eOSD_TP_BLUE:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_BLUE;
                break;

            case eOSD_TP_YELLOW:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_YELLOW;
                break;

            case eOSD_TP_MAGENTA:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_MAGENTA;
                break;

            case eOSD_TP_CYAN:
                sCmdFormat->lData= (INT32)eOptomaUART_OSD_TP_CYAN;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucTestPattern = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcOSDTEST_PATTERN))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_OSD_TP_OFF:
                ucTestPattern = eOSD_TP_OFF;
                break;

            case eOptomaUART_OSD_TP_GREEN_GRID:
                ucTestPattern = eOSD_TP_GREEN_GRID;
                break;

            case eOptomaUART_OSD_TP_MAGENTA_GRID:
                ucTestPattern = eOSD_TP_MAGENTA_GRID;
                break;

            case eOptomaUART_OSD_TP_WHITE_GRID:
                ucTestPattern = eOSD_TP_WHITE_GRID;
                break;

            case eOptomaUART_OSD_TP_WHITE:
                ucTestPattern = eOSD_TP_WHITE;
                break;

            case eOptomaUART_OSD_TP_BLACK:
                ucTestPattern = eOSD_TP_BLACK;
                break;

            case eOptomaUART_OSD_TP_RED:
                ucTestPattern = eOSD_TP_RED;
                break;

            case eOptomaUART_OSD_TP_GREEN:
                ucTestPattern = eOSD_TP_GREEN;
                break;

            case eOptomaUART_OSD_TP_BLUE:
                ucTestPattern = eOSD_TP_BLUE;
                break;

            case eOptomaUART_OSD_TP_YELLOW:
                ucTestPattern = eOSD_TP_YELLOW;
                break;

            case eOptomaUART_OSD_TP_MAGENTA:
                ucTestPattern = eOSD_TP_MAGENTA;
                break;

            case eOptomaUART_OSD_TP_CYAN:
                ucTestPattern = eOSD_TP_CYAN;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTestPattern);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_IR_Function(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0032
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucIR = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_FRONT_IR_OFF:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcFRONT_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_OFF;
                palDataMgr_Data_Access(edcFRONT_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

            case eOptomaUART_FRONT_IR_ON:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcFRONT_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_ON;
                palDataMgr_Data_Access(edcFRONT_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
            break;

            case eOptomaUART_TOP_IR_OFF:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcTOP_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_OFF;
                palDataMgr_Data_Access(edcTOP_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

            case eOptomaUART_TOP_IR_ON:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcTOP_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_ON;
                palDataMgr_Data_Access(edcTOP_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

            case eOptomaUART_HDBASET_IR_OFF:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcHDBASET_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_OFF;
                palDataMgr_Data_Access(edcHDBASET_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

            case eOptomaUART_HDBASET_IR_ON:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcHDBASET_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_ON;
                palDataMgr_Data_Access(edcHDBASET_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

           case eOptomaUART_REAR_IR_OFF: //TBD
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcREAR_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_OFF;
                palDataMgr_Data_Access(edcREAR_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

            case eOptomaUART_REAR_IR_ON: //TBD
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcREAR_IR))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucIR = ets_ON;
                palDataMgr_Data_Access(edcREAR_IR, edaWRITE_THROUGH_WITH_ACTION, &ucIR);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HotKeySetting(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0032
{
    UINT16 DataCode = edcHOT_KEY_SETTINGS;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HotKeySetting_User2(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Julie_0036
{
    UINT16 DataCode = edcHOT_KEY2_SETTINGS;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_MenuLocation(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0032
{
    UINT16 DataCode = edcMENU_LOCATION;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_MenuTimeOut(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0033
{
    UINT16 DataCode = edcMENU_TIME_OUT;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Logo(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)    //A65_OPTOMA_Julie_0088 //ZU860_Doulas_0033
{
    UINT16 DataCode = edcLOGO_CHANGE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucData;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->lData == eOptomaUART_LOGO_CHANGE_DEFAULT)
        {
            ucData = eCM_CHANGE_LOGO_DEFAULT;
        }
        else if(sCmdFormat->lData == eOptomaUART_LOGO_CHANGE_NEUTRAL)
        {
            ucData = eCM_CHANGE_LOGO_NEUTRAL;
        }
        else if(sCmdFormat->lData == eOptomaUART_LOGO_CHANGE_CUSTOM)
        {
            ucData = eCM_CHANGE_LOGO_USER;
        }
        else if(sCmdFormat->lData == eOptomaUART_LOGO_CHANGE_CAPTURED)
        {
            ucData = eCM_CHANGE_LOGO_CAPTURED;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &ucData);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_DeleteLogo(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0083
{
    UINT16 DataCode = edcLOGO_DELETE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(sCmdFormat->lData == 1) //Captured Logo
        {
            // Value = eCM_CHANGE_LOGO_CAPTURED;
            // palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value);
			return eCLI_ERROR_CODE_SETFAIL;
        }
        else if(sCmdFormat->lData == 2) //User Logo
        {
            Value = eCM_CHANGE_LOGO_USER;
            palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BackgroundColor(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0033
{
    UINT16 DataCode = edcBACKGROUND_COLOR;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SerialPortPath(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0033
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucSerialPortPath = 0;

        palDataMgr_Data_Access(edcSERIAL_PORT_PATH, edaREAD, &ucSerialPortPath) ;

        switch(ucSerialPortPath)
        {
            case eSERIAL_PORT_RS232:
                sCmdFormat->lData= (INT32)eOptomaUART_SERIAL_PORT_RS232;
                break;

            case eSERIAL_PORT_HDBASET:
                sCmdFormat->lData= (INT32)eOptomaUART_SERIAL_PORT_HDBASET;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucSerialPortPath = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcSERIAL_PORT_PATH))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_SERIAL_PORT_RS232:
                ucSerialPortPath = eSERIAL_PORT_RS232;
                break;

            case eOptomaUART_SERIAL_PORT_HDBASET:
                ucSerialPortPath = eSERIAL_PORT_HDBASET;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcSERIAL_PORT_PATH, edaWRITE_THROUGH_WITH_ACTION, &ucSerialPortPath) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ReSync(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0034
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucReSync = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcAUTO_IMAGE))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_RESYNC_GO:
                palDataMgr_Data_Access(edcAUTO_IMAGE, edaREAD, &ucReSync) ;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcAUTO_IMAGE, edaWRITE_THROUGH_WITH_ACTION, &ucReSync) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_3D_Tech(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT16 DataCode = 0;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)	//A65_OPTOMA_Doulas_0074 Modify
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_3D_MODE_OFF:
            case CLI_3D_MODE_ON:
            case CLI_3D_MODE_PASSIVE_3D:
                DataCode = edc3D_MODE;
                break;

            case CLI_3D_TECH_DLP_LINK:
            case CLI_3D_TECH_3D_SYNC:
                DataCode = edcDLPLink_ON;
                break;
        }

        if(DataCode)
        {
            if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
            {
                return eCLI_ERROR_CODE_DATAERROR;
            }

            if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Security(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0039
{
    UINT8 ucPIN_PROTECT_EN;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcPIN_PROTECT))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        if(palDataMgr_Data_Access(edcPIN_PROTECT, edaREAD, &ucPIN_PROTECT_EN) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        sCmdFormat->lData= (INT32)ucPIN_PROTECT_EN;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucPWord[5] = {0};
        UINT8 ucPWord_Input[5] = {0};
        UINT8 ucVAl = 0;
        UINT8 ucStringLen = 0;

        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        //LOG_MSG(db_ALWAYS, " @@String = %s (%d,%d)\r\n", sCmdFormat->cTextString,sCmdFormat->ucDataType,ucStringLen);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen != 7)    //0 ~1234 ,7bytes
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[0] != '0') && (sCmdFormat->cTextString[0] != '1'))  //data value
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))  //check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord) != eEXEC_CODE_PASS)  //get password
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        //LOG_MSG(db_ALWAYS, " $$String = %s (%s)\r\n", sCmdFormat->cTextString, ucPWord);

        ucVAl = 0;
        for(; ucVAl < 4 ; ucVAl++)  //password 4 bytes
        {
            if((sCmdFormat->cTextString[ucVAl+3] >= '0') &&
                (sCmdFormat->cTextString[ucVAl+3] <= '9'))
            {
                ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl+3];

                if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                {
                	return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        if(sCmdFormat->cTextString[0] == '0')
        {
            ucPIN_PROTECT_EN = ets_OFF;
        }
        else
        {
            ucPIN_PROTECT_EN = ets_ON;
        }

        //LOG_MSG(db_ALWAYS, " ##String = %s (%d)\r\n", sCmdFormat->cTextString, ucPIN_PROTECT_EN);
        eResult = palDataMgr_Data_Access(edcPIN_PROTECT, edaWRITE_THROUGH_WITH_ACTION, &ucPIN_PROTECT_EN);

        if(ucPIN_PROTECT_EN == 0)
        {
			ucVAl = 1;//A35G2_David_0006
			palDataMgr_Data_Access(edcUI_EVENT_PIN_PROTECT_CLEAR, edaWRITE_THROUGH_WITH_ACTION, &ucVAl);
        }

        if(eResult != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SecurityTimer(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A65_OPTOMA_Julie_0096 //A65_OPTOMA_Julie_0088 //ZU860_Doulas_0039 //A35G2_Coda_0061
{
    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT32 ucTimer = 0;
        UINT8  ucMonth = 0;
        UINT8  ucDay = 0;
        UINT8  ucHour = 0;

        char cString[16];
        UINT8 cData[16];
        UINT8 ucStringLen = 0;

        ucStringLen = strlen((char*)sCmdFormat->cTextString);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DECIMALERR;
        }

        if(ucStringLen != 6)	//MMDDHH ,6bytes
        {
            return eCLI_ERROR_CODE_FORMATERR;
        }

        for(UINT8 cIndex=0; cIndex < (ucStringLen); cIndex++)
        {
            if((sCmdFormat->cTextString[cIndex] >= '0') && (sCmdFormat->cTextString[cIndex] <= '9'))
            {
                strncpy((char*)&cString[cIndex], (char*)&sCmdFormat->cTextString+(cIndex), 1);
                cData[cIndex] = atoi(&cString[cIndex]);
                //LOG_MSG(db_ALWAYS,"(%s,%d)%d:[%d]\r\n", __func__, __LINE__, cIndex, cData[cIndex]);
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        ucMonth = (cData[0] * 10) + cData[1];
        ucDay	= (cData[2] * 10) + cData[3];
        ucHour	= (cData[4] * 10) + cData[5];

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(edcSECURITY_TIMER_MONTH, ucMonth))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(edcSECURITY_TIMER_DAY, ucDay))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(edcSECURITY_TIMER_HOUR, ucHour))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        palDataMgr_Data_Access(edcSECURITY_TIMER_MONTH, edaWRITE_THROUGH_WITH_ACTION, &ucMonth);
        palDataMgr_Data_Access(edcSECURITY_TIMER_DAY, edaWRITE_THROUGH_WITH_ACTION, &ucDay);
        palDataMgr_Data_Access(edcSECURITY_TIMER_HOUR, edaWRITE_THROUGH_WITH_ACTION, &ucHour);
    }
    return eCLI_ERROR_CODE_NO;
}
eCLI_ERROR_CODE utilOptoma_Security_Status(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)       //A65_OPTOMA_David_0014
{
    if(eAccessMode == ecmRead)
    {
            UINT8 Security_state = 0;

		  Security_state=palDataMgr_Control_Security_Lock();//A65_OPTOMA_David_0014
	        if(Security_state == eFUNC_CONTROL_ENABLE)
            {
                sCmdFormat->lData = TRUE;//security empty
            }
            else
            {
                sCmdFormat->lData = FALSE;// security setting
            }
        }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Security_ChangePassword(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084
{
    UINT8 ucPIN_PROTECT_EN;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcPIN_PROTECT))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        if(palDataMgr_Data_Access(edcPIN_PROTECT, edaREAD, &ucPIN_PROTECT_EN) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        sCmdFormat->lData= (INT32)ucPIN_PROTECT_EN;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucPWord_Current[5] = {0};
        UINT8 ucPWord_Input[5] = {0};
        UINT8 ucPWord_Change[5] = {0};
        UINT8 ucVAl;
        UINT8 ucStringLen = 0;

        ucStringLen = strlen((char*)sCmdFormat->cTextString);

        LOG_MSG(db_APP_CLI,"(%s,%d)ucDataType[%d] ucStringLen[%d] cTextString[%s]\r\n", __func__, __LINE__,sCmdFormat->ucDataType, ucStringLen, sCmdFormat->cTextString);

        if(sCmdFormat->lData == 1 && ucStringLen == 1 )//A65_OPTOMA_David_0014
        {
            UINT8 ucPWord[5] = {0};
            ucPWord[0] = 0xFF;
            ucPWord[1] = 0xFF;
            ucPWord[2] = 0xFF;
            ucPWord[3] = 0xFF;
			ucVAl=0;
            palDataMgr_Data_Access(edcCHANGE_PIN, edaWRITE_THROUGH_WITH_ACTION, ucPWord);//H30K_David_0037
            palDataMgr_Data_Access(edcPIN_PROTECT, edaWRITE_THROUGH_WITH_ACTION, &ucVAl);//H30K_David_0037
            LOG_MSG(db_APP_CLI , "(%s,%d)ucPWord =(%s)\r\n", __func__, __LINE__, ucPWord);
            return eCLI_ERROR_CODE_NO;//eResult;
        }

        //mmmm ~nnnn, m=current password, n=change password.
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen != 10)    //1234 ~5678 ,10bytes
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[4] != ' ') || (sCmdFormat->cTextString[5] != '~'))  //check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord_Current) != eEXEC_CODE_PASS)  //get password
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        for(ucVAl = 0; ucVAl < 4 ; ucVAl++)  //password 4 bytes
        {
            if((sCmdFormat->cTextString[ucVAl] >= '0') && (sCmdFormat->cTextString[ucVAl] <= '9'))
            {
                ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl];

                if(ucPWord_Input[ucVAl] != ucPWord_Current[ucVAl])
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        for(ucVAl = 0; ucVAl < 4 ; ucVAl++)  //change password 4 bytes
        {
            if((sCmdFormat->cTextString[ucVAl+6] >= '0') && (sCmdFormat->cTextString[ucVAl+6] <= '9'))
            {
                ucPWord_Change[ucVAl] = sCmdFormat->cTextString[ucVAl+6] ;
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        LOG_MSG(db_APP_CLI, "PWord_Current =(%s), PWord_Input =(%s), PWord_Change =(%s)\r\n", ucPWord_Current, ucPWord_Input, ucPWord_Change);

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcCHANGE_PIN, edaWRITE_THROUGH_NO_ACTION, ucPWord_Change))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Security_Password_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084
{
    if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData == 99)
        {
            UINT8 ucPWord[5] = {0};
            ucPWord[0] = '1';
            ucPWord[1] = '2';
            ucPWord[2] = '3';
            ucPWord[3] = '4';

            palDataMgr_Data_Access(edcCHANGE_PIN, edaWRITE_THROUGH_NO_ACTION, ucPWord);
            palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord);
            LOG_MSG(db_ALWAYS, "(%s,%d)ucPWord =(%s)\r\n", __func__, __LINE__, ucPWord);
        }
    }
}

eCLI_ERROR_CODE utilOptoma_AllVersion(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)    //A65_OPTOMA_Julie_0088 //A65_OPTOMA_Julie_0036 //ZU860_Doulas_0043 //HICC2_Julie_0001
{
    UINT8 ucString[64] = {0};
    char  ucOutputString[128] = {""};

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcRELEASE_VERSION, edaREAD, ucString);
        sprintf(sCmdFormat->cTextString, "%s",ucString);
        sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;

        #if 0
        palDataMgr_Data_Access(edcRELEASE_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"Main Version\"         %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFIRMWARE, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"I-SCALER Version\"     %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFRONTEND_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"F-MCU Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcMOTOR_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"M-MCU Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFMCU_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"A-MCU Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"LAN Version\"          %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFORMATER_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"Formatter Version\"    %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcXPR_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"FPGA1 Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcFPGA3_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"FPGA2 Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcHDBASET_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"HDBaseT Version\"      %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcPMCU_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"RX24T Version\"        %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_Data_Access(edcCAMERA_FW_VERSION, edaREAD, ucString);
        sprintf(ucOutputString, "\r\"Camera Version\"       %s\r\n", ucString);
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        #endif
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LensCalibration(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0048
{
    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucLensCalibration = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_LENS_CALIBRATION_NORMAL:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcLENS_CALIBRATION))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucLensCalibration = 1;
                palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucLensCalibration) ;
                break;

            case eOptomaUART_LENS_CALIBRATION_UST:
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcUST_SET))  //available check
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            case eOptomaUART_LENS_CALIBRATION_NO:    //ZU860_Doulas_0065 Add
                palDataMgr_Data_Access(edcUST_SET, edaWRITE_THROUGH_WITH_ACTION, ets_OFF);	//ZU860_Clare_0106
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_3D_Invert(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //H30K_David_0018
{
    UINT16 DataCode = edc3D_INVERT;
    INT32 Value = 0;
	UINT8 ucInvert = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = ~Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        switch(sCmdFormat->lData)
        {
            case ets_OFF:
                ucInvert =  ets_ON;
				 //LOG_MSG(db_ALWAYS , "(ets_OFF edc3D_INVERT:%d, ucInvert:%d)\r\n", edc3D_INVERT,ucInvert );
                palDataMgr_Data_Access(edc3D_INVERT, edaWRITE_THROUGH_WITH_ACTION, &ucInvert) ;
                break;

            case ets_ON:
                ucInvert = ets_OFF;
				//LOG_MSG(db_ALWAYS , "(ets_ON edc3D_INVERT:%d, ucInvert:%d)\r\n", edc3D_INVERT,ucInvert );
                palDataMgr_Data_Access(edc3D_INVERT, edaWRITE_THROUGH_WITH_ACTION, &ucInvert) ;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_3D_Format(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0050
{
    UINT16 DataCode = edc3D_ENABLE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LR_Reference(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0063
{
    UINT16 DataCode = edc3D_LR_REFERENCE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Brightness(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //H30K_David_0041
{
    UINT16 DataCode = edcBRIGHTNESS;
    INT32 Value = 0;
    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value-50;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        INT16 ucBrightness = 0;


        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcBRIGHTNESS))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }


        ucBrightness =sCmdFormat->lData;
		if(ucBrightness > 50 || ucBrightness < -50)
		{
			   return eCLI_ERROR_CODE_SETFAIL;
		}
		else
		{
		  ucBrightness=ucBrightness+50;
          palDataMgr_Data_Access(edcBRIGHTNESS, edaWRITE_THROUGH_WITH_ACTION, &ucBrightness) ;
		}
    }
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilOptoma_Brightness_AD(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0063
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucBrightness = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcBRIGHTNESS))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Access(edcBRIGHTNESS, edaREAD, &ucBrightness);

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BRIGHTNESS_AD_ADD:
                if(ucBrightness < BRIGHTNESS_MAX_VALUE)
                {
                    ucBrightness++;
                }
                break;

            case eOptomaUART_BRIGHTNESS_AD_DECREASE:
                if(ucBrightness > BRIGHTNESS_MIN_VALUE)
                {
                    ucBrightness--;
                }
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcBRIGHTNESS, edaWRITE_THROUGH_WITH_ACTION, &ucBrightness) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Contrast(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //H30K_David_0041

{
    UINT16 DataCode = edcCONTRAST;
    INT32 Value = 0;
    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value-50;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        INT16 ucContrast = 0;


        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcCONTRAST))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }


        ucContrast =sCmdFormat->lData;
		if(ucContrast > 50 || ucContrast < -50)
		{
			   return eCLI_ERROR_CODE_SETFAIL;
		}
		else
		{
		  ucContrast=ucContrast+50;
          palDataMgr_Data_Access(edcCONTRAST, edaWRITE_THROUGH_WITH_ACTION, &ucContrast) ;
		}
    }
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilOptoma_Contrast_AD(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0063
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucContrast = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcCONTRAST))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Access(edcCONTRAST, edaREAD, &ucContrast);

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_CONTRAST_AD_ADD:
                if(ucContrast < CONTRAST_MAX_VALUE)
                {
                    ucContrast++;
                }
                break;

            case eOptomaUART_CONTRAST_AD_DECREASE:
                if(ucContrast > CONTRAST_MIN_VALUE)
                {
                    ucContrast--;
                }
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcCONTRAST, edaWRITE_THROUGH_WITH_ACTION, &ucContrast) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ModelName(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)             //ZU860_Doulas_0065
{
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData= (INT32)6;	//6= Optoma UHD
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Output_3D_State(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)       //ZU860_Doulas_0065//A35G2_Coda_0061
{
    if(eAccessMode == ecmRead)
    {
        UINT8 uc3D_state = 0;
        UINT8 uc3D2D_Value = eCM_3D_2D_NUMBER;

        uc3D_state = palDataMgr_Control_3D_InvertDepend();

        if(uc3D_state == eFUNC_CONTROL_ENABLE)
        {
            palDataMgr_Data_Access(edc3D_2D, edaREAD, &uc3D2D_Value) ; //A65_OPTOMA_Julie_0097 //A65_OPTOMA_Julie_0089

            if(uc3D2D_Value != eCM_3D_2D_3D)
            {
                sCmdFormat->lData= (INT32)eOptomaUART_OUTPUT_3D_STATE_2D;
            }
            else
            {
                sCmdFormat->lData= (INT32)eOptomaUART_OUTPUT_3D_STATE_3D;
            }
        }
        else
        {
            sCmdFormat->lData= (INT32)eOptomaUART_OUTPUT_3D_STATE_2D;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Front_IR_Get(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0065
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucFront_IR = 0;

        palDataMgr_Data_Access(edcFRONT_IR, edaREAD, &ucFront_IR) ;

        switch(ucFront_IR)
        {
            case ets_OFF:
                sCmdFormat->lData= (INT32)ucFront_IR;
                break;

            case ets_ON:
                sCmdFormat->lData= (INT32)ucFront_IR;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_TOP_IR_Get(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0065
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucTOP_IR = 0;

        palDataMgr_Data_Access(edcTOP_IR, edaREAD, &ucTOP_IR) ;

        switch(ucTOP_IR)
        {
            case ets_OFF:
                sCmdFormat->lData= (INT32)ucTOP_IR;
                break;

            case ets_ON:
                sCmdFormat->lData= (INT32)ucTOP_IR;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HDBASET_IR_Get(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0036
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucHDBaseT_IR = 0;

        palDataMgr_Data_Access(edcHDBASET_IR, edaREAD, &ucHDBaseT_IR) ;

        switch(ucHDBaseT_IR)
        {
            case ets_OFF:
                sCmdFormat->lData= (INT32)ucHDBaseT_IR;
                break;

            case ets_ON:
                sCmdFormat->lData= (INT32)ucHDBaseT_IR;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Rear_IR_Get(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucRear_IR = 0;

        palDataMgr_Data_Access(edcREAR_IR, edaREAD, &ucRear_IR) ;

        switch(ucRear_IR)
        {
            case ets_OFF:
                sCmdFormat->lData= (INT32)ucRear_IR;
                break;

            case ets_ON:
                sCmdFormat->lData= (INT32)ucRear_IR;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_OSD_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0065
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcOsdReset))  //available check     //ZU860_Doulas_0066
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 1)      //ZU860_Doulas_0041 modify
        {
            eResult = palDataMgr_Data_Access(edcOsdReset, edaWRITE_THROUGH_WITH_ACTION, &ucValue) ;

            if(eResult != eEXEC_CODE_PASS)
            {
            return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_3D_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0066
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edc3D_Reset))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 1)
        {
            eResult = palDataMgr_Data_Access(edc3D_Reset, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Image_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0066
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcImageReset))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 1)
        {
            eResult = palDataMgr_Data_Access(edcImageReset, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

            if(eResult != eEXEC_CODE_PASS)
            {
            return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LensMemory_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Doulas_0066
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcLensMemoryReset))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)    //ZU860_Doulas_0075 Add
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 1)
        {
            eResult = palDataMgr_Data_Access(edcLensMemoryReset, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}



eCLI_ERROR_CODE utilOptoma_HDRLevel(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0022 //H30K_David_0020
{
    UINT16 DataCode = edcHDR_LEVEL;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value-1;
			//LOG_MSG(db_ALWAYS, "==1=HDRLevel(%d)\r\n",Value);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucHDRLevel = 0;
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }


		switch(sCmdFormat->lData)
        {
            case eCM_HDR_LEVEL_SDR:
                ucHDRLevel= eCM_HDR_LEVEL1;
                palDataMgr_Data_Access(edcHDR_LEVEL, edaWRITE_THROUGH_WITH_ACTION, &ucHDRLevel) ;
                break;

            case eCM_HDR_LEVEL1:
                ucHDRLevel= eCM_HDR_LEVEL2;
                palDataMgr_Data_Access(edcHDR_LEVEL, edaWRITE_THROUGH_WITH_ACTION, &ucHDRLevel) ;
                break;
			case eCM_HDR_LEVEL2:
                ucHDRLevel= eCM_HDR_LEVEL3;
                palDataMgr_Data_Access(edcHDR_LEVEL, edaWRITE_THROUGH_WITH_ACTION, &ucHDRLevel) ;
                break;
			case eCM_HDR_LEVEL3:
                ucHDRLevel= eCM_HDR_LEVEL4;
                palDataMgr_Data_Access(edcHDR_LEVEL, edaWRITE_THROUGH_WITH_ACTION, &ucHDRLevel) ;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}



eCLI_ERROR_CODE utilOptoma_ExtremeBlack(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0102
{
    if(eAccessMode == ecmRead)
    {

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucExtremeBlack = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcREAL_BLACK))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case ets_OFF:
                ucExtremeBlack = ets_OFF;
                palDataMgr_Data_Access(edcREAL_BLACK, edaWRITE_THROUGH_WITH_ACTION, &ucExtremeBlack) ;
                break;

            case ets_ON:
                ucExtremeBlack = ets_ON;
                palDataMgr_Data_Access(edcREAL_BLACK, edaWRITE_THROUGH_WITH_ACTION, &ucExtremeBlack) ;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SerialPortBaudRate(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0103
{
    UINT16 DataCode = edcSERIAL_PORT_BAUD_RATE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        static const UINT32 m_BAUD_RATE_Array[] = {1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200}; //HICC2_Sammy_0002//A65_OPTOMA_Julie_0036

        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = m_BAUD_RATE_Array[Value];
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SerialPortOutBaudRate(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0085
{
    UINT16 DataCode = edcSERIAL_PORT_OUT_BAUD_RATE;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        static const UINT32 m_BAUD_RATE_Array[] = {1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200};	//HICC2_Sammy_0002

        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = m_BAUD_RATE_Array[Value];
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LAN_IP_Address(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0113
{
    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcLAN_IP_ADDRESS, edaREAD, sCmdFormat->cTextString);
        sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_LAN_FW_Version(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //ZU860_Doulas_0117
{
    UINT8 ucString[64] = {0};

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
        palDataMgr_Data_Access(edcLAN_VERSION, edaREAD, ucString);
        sprintf(sCmdFormat->cTextString, "%s",ucString);
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Info_String(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0063 //ZU860_Doulas_0118
{
    UINT8 ucPowerStatus = 0;
    UINT32 udwLD_Hours = 0;
    INT32 InputSource = 0;
    INT32 DisplayMode = 0;
    INT32 Value = eCM_SOURCE_RESERVED;
    UINT16 DataCode = edcPICTURE_SETTINGS;
    UINT8 ucPictureMode = 0;

    if(eAccessMode == ecmRead)
    {
        sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;

        //Power[%d]=a
        if((palSystem_PowerStateGet() == ePOWER_STATE_STANDBY) || palEnvironment_Fake_Power_Down_Get())
        {
            ucPowerStatus = 0;
        }
        else
        {
            ucPowerStatus = 1;
        }

        //LightSource Life[%05d]=bbbbb
        palDataMgr_Data_Access(edcLD_HOURS, edaREAD, &udwLD_Hours);

        //source none
        if(palDataPath_IsSourceLock() == FALSE)
        {
            InputSource = eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL;
            DisplayMode = eOptomaUART_NO_SIGNAL;
        }
        else
        {
            //Input Source[%02d]=cc
            palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &Value);

            switch(Value)
            {
                default:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL;// 0
                    break;

                case eCM_SOURCE_HDMI1:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_HDMI1;// 7
                    break;

                case eCM_SOURCE_HDMI2:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_HDMI2;// 8
                    break;

                case eCM_SOURCE_DVI:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_DVI_D;// 1
                    break;

                case eCM_SOURCE_3GSDI:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_3G_SDI;// 18
                    break;

                case eCM_SOURCE_HDBASET:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_HDBASET;// 16
                    break;

                case eCM_SOURCE_DISPLAYPORT:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_DP;
                    break;

                case eCM_SOURCE_12GSDI:
                    InputSource = eOptomaUART_MAIN_SOURCE_RES_12G_SDI;
                    break;
            }

            //Display Mode[%02d]=ee
            palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &ucPictureMode);
            if(ucPictureMode == eCM_PICTURE_SETTINGS_USER)
            {
                DataCode = edcUSER_COLOR_MODE;
            }

            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionGet(DataCode, &DisplayMode))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        sprintf(sCmdFormat->cTextString, "%d%05d%02d%02d%02d%02d", ucPowerStatus, udwLD_Hours/60, InputSource, VER_MAJOR, VER_MINOR, DisplayMode); //A35G2_Coda_0128
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Fan_Speed(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0118
{
    UINT16 uTempRPM = 0;

    if(eAccessMode == ecmRead)
    {
        if(Drivercfg_Value_Get(palSystem_ModelIDGet(),eFan_Exist, (UINT8)(sCmdFormat->lData)) != DEVICE_IS_EXIST)
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
        else
        {
            uTempRPM = palDataMgr_Access_Get_FAN_RPM((UINT8)(sCmdFormat->lData));
            sCmdFormat->lData = uTempRPM;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_AutoImage(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Owen_0019
{
    UINT8 ucAutoImg = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcAUTO_IMAGE))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_AUTOMATIC_1:
                ucAutoImg = eAUTO_IMAGE_NORMAL;
                palDataMgr_Data_Access(edcAUTO_IMAGE, edaWRITE_THROUGH_WITH_ACTION, &ucAutoImg);
                break;

            case eOptomaUART_AUTOMATIC_2:
                ucAutoImg = eAUTO_IMAGE_WIDE;
                palDataMgr_Data_Access(edcAUTO_IMAGE, edaWRITE_THROUGH_WITH_ACTION, &ucAutoImg);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_WarpSetting(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Owen_0019
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcWARP_SETTING, edaREAD, &ucSetting);

        switch(ucSetting)
        {
            case eWARPING_SETTING_ALL_OFF:
                sCmdFormat->lData= (INT32)eOptomaUART_WARPSETTING_ALLOFF;
                break;

            case eWARPING_SETTING_ALL_ON:
                sCmdFormat->lData= (INT32)eOptomaUART_WARPSETTING_ALLON;
                break;

            case eWARPING_SETTING_BLEND_OFF:
                sCmdFormat->lData= (INT32)eOptomaUART_WARPSETTING_BLENDOFF;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcWARP_SETTING))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_WARPSETTING_ALLOFF:
                ucSetting = eWARPING_SETTING_ALL_OFF;
                palDataMgr_Data_Access(edcWARP_SETTING, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            case eOptomaUART_WARPSETTING_ALLON:
                ucSetting = eWARPING_SETTING_ALL_ON;
                palDataMgr_Data_Access(edcWARP_SETTING, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            case eOptomaUART_WARPSETTING_BLENDOFF:
                ucSetting = eWARPING_SETTING_BLEND_OFF;
                palDataMgr_Data_Access(edcWARP_SETTING, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SystemUpdate(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //ZU860_Owen_0019
{
    UINT8 ucValue = 1;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case ets_OFF:
                ucValue = ets_OFF;
                palDataMgr_Data_Access(edcNOTIFICATIONSYSTEMUPDATE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                break;

            case ets_ON:
                ucValue = ets_ON;
                palDataMgr_Data_Access(edcNOTIFICATIONSYSTEMUPDATE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                break;

            case 3: //A65_OPTOMA_Julie_0092
                ucValue = ets_OFF;
                palDataMgr_Data_Access(edcSILENTFOTA_SYSTEMUPDATE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                break;

            case 4: //A65_OPTOMA_Julie_0092
                ucValue = ets_ON;
                palDataMgr_Data_Access(edcSILENTFOTA_SYSTEMUPDATE, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
                break;

            case 9: //A65_OPTOMA_Julie_0062
                if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcSystemUpdateStatus))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }

                eFOTA_SYSTEM_UPDATE_STATUS SystemUpdateStatus = eFOTA_SYSTEM_UPDATE_STATUS_QUERY_FW;
                palDataMgr_Data_Access(edcSystemUpdateStatus, edaWRITE_THROUGH_WITH_ACTION, &SystemUpdateStatus);
            #if 0 //mask temporary //ac test
                appGui_OpenMenu(WAIT_MESSAGE_MENU_ICOUNT);
            #endif
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_GetDeviceType(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{// BruceLin#20210113
    // Ok1: Projector
    // Ok2: IFP
    // Ok3: AIO LED
    // rest of: Projector
    if(eAccessMode == ecmRead)
    {
        sCmdFormat->lData = 1;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_CameraModuleStatus(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{// BruceLin#20210113
    UINT16 DataCode = edcCAMERA_MODULE_STATUS;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BroadcastMessage(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{// BruceLin#20210113
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        //LOG_MSG(db_APP_GUI, "%s[%d]: %s(%s)\r\n", __FILE__, __LINE__, __FUNCTION__, sCmdFormat->cTextString);
        #if 0 //mask temporary //ac test
        appGui_Send_CLI_Broadcast_Msg_Event(sCmdFormat->cTextString);
        #endif
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_OMSOptomaBroadCastMsg(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT8 ucStringLen = 0;

    if(eAccessMode == ecmWrite)
    {
        ucStringLen = strlen((char*)sCmdFormat->cTextString);

        if(ucStringLen > BROADCAST_MSG_LEN)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Access(edcUI_EVENT_BROADCAST_MSG, edaWRITE_RAM_ONLY_NO_ACTION, (char*)sCmdFormat->cTextString) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HSG_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0074
{
    UINT16 DataCode = 0;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)//H30K_David_0042
        {
            case eOptomaUART_HSG_RESET_ALL:
                DataCode = edcHSG_RESET_DEFAULT;
                break;

            case eOptomaUART_HSG_RESET_RGBCMY:
                DataCode = edcHSG_RGBCMY_RESET;
                break;

            case eOptomaUART_HSG_RESET_WHITE:
                DataCode = edcHSG_WHITE_GAIN_RESET_DEFAULT;
                break;

			case eOptomaUART_HSG_RESET_RED:
                DataCode = edcHSG_RED_RESET_DEFAULT;
                break;

 			case eOptomaUART_HSG_RESET_GREEN:
                 DataCode = edcHSG_GREEN_RESET_DEFAULT;
                 break;

			case eOptomaUART_HSG_RESET_BLUE:
                 DataCode = edcHSG_BLUE_RESET_DEFAULT;
                 break;

			case eOptomaUART_HSG_RESET_CYAN :
                 DataCode = edcHSG_CYAN_RESET_DEFAULT;
                 break;

			case eOptomaUART_HSG_RESET_MAGENTA :
                 DataCode = edcHSG_MAGENTA_RESET_DEFAULT;
                 break;

			case eOptomaUART_HSG_RESET_YELLOW  :
                 DataCode = edcHSG_YELLOW_RESET_DEFAULT;
                 break;
        }

        Value = 1;

        if(DataCode)
        {
            if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
            {
                return eCLI_ERROR_CODE_DATAERROR;
            }

            if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Image_Shift_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A65_OPTOMA_Doulas_0076
{
    UINT8 ucValue = 1;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->lData == 1)
        {
            palDataMgr_HV_Start_Position_Reset();
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Grid_Color(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
{
	UINT8 ucSetting = 0;
    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcADV_WARP_GRID_COLOR, edaREAD, &ucSetting);//H30K_David_0027
        sCmdFormat->lData= (INT32)(ucSetting+1) ;//H30K_David_0027
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_GRID_COLOR_GREEN:
                ucValue = COLOR_IDX__GREEN;
                break;

            case eOptomaUART_GRID_COLOR_MAGENTA:
                ucValue = COLOR_IDX__MAGENTA;
                break;

            case eOptomaUART_GRID_COLOR_RED:
                ucValue = COLOR_IDX__RED;
                break;

            case eOptomaUART_GRID_COLOR_CYAN:
                ucValue = COLOR_IDX__CYAN;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcADV_WARP_GRID_COLOR))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcADV_WARP_GRID_COLOR, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Grid_Background(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
    {
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_GRID_BACKGROUND_BLACK:
                ucValue = BKG_COLOR__BLACK;
                break;

            case eOptomaUART_GRID_BACKGROUND_TRANSPARENT:
                ucValue = BKG_COLOR__TRANSPARENT;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcADV_WARP_GRID_BACKGROUND))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcADV_WARP_GRID_BACKGROUND, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Grid_Points(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_GRID_POINTS_2x2:
                ucValue = WARP_POINT__2x2;
                break;

            case eOptomaUART_GRID_POINTS_3x3:
                ucValue = WARP_POINT__3x3;
                break;

            case eOptomaUART_GRID_POINTS_5x5:
                ucValue = WARP_POINT__5x5;
                break;

            case eOptomaUART_GRID_POINTS_9x9:
                ucValue = WARP_POINT__9x9;
                break;

            case eOptomaUART_GRID_POINTS_17x17:
                ucValue = WARP_POINT__17x17;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcADV_WARP_GRID_POINTS))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcADV_WARP_GRID_POINTS, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Overlap_Grid_Number(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_OVERLAP_GRID_NUM_4:
                ucValue = eOptomaUART_OVERLAP_GRID_NUM_4 -1;
                break;

            case eOptomaUART_OVERLAP_GRID_NUM_6:
                ucValue = eOptomaUART_OVERLAP_GRID_NUM_6 -1;
                break;

            case eOptomaUART_OVERLAP_GRID_NUM_8:
                ucValue = eOptomaUART_OVERLAP_GRID_NUM_8 -1;
                break;

            case eOptomaUART_OVERLAP_GRID_NUM_10:
                ucValue = eOptomaUART_OVERLAP_GRID_NUM_10 -1;
                break;

            case eOptomaUART_OVERLAP_GRID_NUM_12:
                ucValue = eOptomaUART_OVERLAP_GRID_NUM_12 -1;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcADV_BLEND_OVERLAP_GRID_NUMBER))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcADV_BLEND_OVERLAP_GRID_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Blend_Gamma(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BLEND_GAMMA_1_P_8:
                ucValue = eCM_BLENDING_GAMMA_1_8;
                break;

            case eOptomaUART_BLEND_GAMMA_1_P_9:
                ucValue = eCM_BLENDING_GAMMA_1_9;
                break;

            case eOptomaUART_BLEND_GAMMA_2_P_0:
                ucValue = eCM_BLENDING_GAMMA_2_0;
                break;

            case eOptomaUART_BLEND_GAMMA_2_P_1:
                ucValue = eCM_BLENDING_GAMMA_2_1;
                break;

            case eOptomaUART_BLEND_GAMMA_2_P_2:
                ucValue = eCM_BLENDING_GAMMA_2_2;
                break;

            case eOptomaUART_BLEND_GAMMA_2_P_3:
                ucValue = eCM_BLENDING_GAMMA_2_3;
                break;

            case eOptomaUART_BLEND_GAMMA_2_P_4:
                ucValue = eCM_BLENDING_GAMMA_2_4;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcADV_BLEND_GAMMA))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(edcADV_BLEND_GAMMA, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }
    return eCLI_ERROR_CODE_NO;
}


eCLI_ERROR_CODE utilOptoma_WarpControl(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //H30K_David_0021

{
    UINT16 DataCode = edcADV_WARP_CONTROL;
	UINT8 ucValue = 0;

    if(eAccessMode == ecmRead)
    {
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        palDataMgr_Data_Access(DataCode, edaREAD, &ucValue);

        switch(ucValue)
        {
            case WARP_CTRL__BASIC:
                sCmdFormat->lData= 1;
                break;

            case WARP_CTRL__ADVANCED:
                sCmdFormat->lData= 5;
                break;

            case WARP_CTRL__AP:
                sCmdFormat->lData= 2;
                break;

            default:
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucHDRLevel = 0;
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

		if(palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &ucValue) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}



eCLI_ERROR_CODE utilOptoma_WarpSettingSave(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcWARP_MEMORY_SAVE))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_WARP_MEMORY_SAVE1:
            case eOptomaUART_WARP_MEMORY_SAVE2:
            case eOptomaUART_WARP_MEMORY_SAVE3:
            case eOptomaUART_WARP_MEMORY_SAVE4:
            case eOptomaUART_WARP_MEMORY_SAVE5:
                ucSetting = (UINT8)(sCmdFormat->lData );//H30K_David_0028
                palDataMgr_Data_Access(edcWARP_MEMORY_SAVE, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_WarpSettingApply(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0076
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaREAD, &ucSetting);
        sCmdFormat->lData= (INT32)(ucSetting) ;//H30K_David_0028
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcWARP_MEMORY_APPLY))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_WARP_MEMORY_APPLY1:
            case eOptomaUART_WARP_MEMORY_APPLY2:
            case eOptomaUART_WARP_MEMORY_APPLY3:
            case eOptomaUART_WARP_MEMORY_APPLY4:
            case eOptomaUART_WARP_MEMORY_APPLY5:
                ucSetting = (UINT8)(sCmdFormat->lData );//H30K_David_0028

                if(palDataMgr_ApplyWarpMemoryItemAvailable(ucSetting) == 0)	 //not available
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HDMI_OUTPUT_Set(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0077
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_NO;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcHDMI_OUT))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_HDMI_OUT_HDMI1:
                ucSetting = eGUI_HDMI_OUT_HDMI1;
                palDataMgr_Data_Access(edcHDMI_OUT, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            case eOptomaUART_HDMI_OUT_HDMI2:
                ucSetting = eGUI_HDMI_OUT_HDMI2;
                palDataMgr_Data_Access(edcHDMI_OUT, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HDMI1_EDID_Set(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0077
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
    	palDataMgr_Data_Access(edcHDMI_EDID_1, edaREAD, &ucSetting);//H30K_David_0026
        sCmdFormat->lData= (INT32)(ucSetting+1) ;//H30K_David_0026
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcHDMI_EDID_1))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_HDMI1_EDID_1_4:
                ucSetting = eCM_EDID_TYPE_V14;
                palDataMgr_Data_Access(edcHDMI_EDID_1, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            case eOptomaUART_HDMI1_EDID_2_0:
                ucSetting = eCM_EDID_TYPE_V20;
                palDataMgr_Data_Access(edcHDMI_EDID_1, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HDMI2_EDID_Set(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0077
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcHDMI_EDID_2, edaREAD, &ucSetting);//H30K_David_0026//H30K_David_0044
        sCmdFormat->lData= (INT32)(ucSetting+1) ;//H30K_David_0026
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcHDMI_EDID_2))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_HDMI2_EDID_1_4:
                ucSetting = eCM_EDID_TYPE_V14;
                palDataMgr_Data_Access(edcHDMI_EDID_2, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            case eOptomaUART_HDMI2_EDID_2_0:
                ucSetting = eCM_EDID_TYPE_V20;
                palDataMgr_Data_Access(edcHDMI_EDID_2, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_HDBASET_EDID_Set(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0077
{
    UINT8 ucSetting = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(edcHDBASET_EDID, edaREAD, &ucSetting);//H30K_David_0026//H30K_David_0044
        sCmdFormat->lData= (INT32)(ucSetting+1) ;//H30K_David_0026
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcHDBASET_EDID))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_HDBASET_EDID_1_4:
                ucSetting = eCM_EDID_TYPE_V14;
                palDataMgr_Data_Access(edcHDBASET_EDID, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            case eOptomaUART_HDBASET_EDID_2_0:
                ucSetting = eCM_EDID_TYPE_V20;
                palDataMgr_Data_Access(edcHDBASET_EDID, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Backup_Restore_Save(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0037
{
    UINT8 ucSetting = 0;
    UINT16 uiDatacode = edcBACKUP_RESTORE_SAVE;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(uiDatacode))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BACKUP_RESTORE_MEMORY1:
            case eOptomaUART_BACKUP_RESTORE_MEMORY2:
            case eOptomaUART_BACKUP_RESTORE_MEMORY3:
            case eOptomaUART_BACKUP_RESTORE_MEMORY4:
            case eOptomaUART_BACKUP_RESTORE_MEMORY5:
                ucSetting = (UINT8)(sCmdFormat->lData - 1);
                palDataMgr_Data_Access(uiDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Backup_Restore_Load(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0037
{
    UINT8 ucSetting = 0;
    UINT16 uiDatacode = edcBACKUP_RESTORE_RESTORE;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(uiDatacode, edaREAD, &ucSetting);
        sCmdFormat->lData= (INT32)(ucSetting+1) ;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(uiDatacode))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BACKUP_RESTORE_MEMORY1:
                if(access(BACKUP_CONFIG_0_PATH, F_OK) == 0)
                {
                    ucSetting = (UINT8)(sCmdFormat->lData - 1);
                    palDataMgr_Data_Access(uiDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

                case eOptomaUART_BACKUP_RESTORE_MEMORY2:
                if(access(BACKUP_CONFIG_1_PATH, F_OK) == 0)
                {
                    ucSetting = (UINT8)(sCmdFormat->lData - 1);
                    palDataMgr_Data_Access(uiDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            case eOptomaUART_BACKUP_RESTORE_MEMORY3:
                if(access(BACKUP_CONFIG_2_PATH, F_OK) == 0)
                {
                    ucSetting = (UINT8)(sCmdFormat->lData - 1);
                    palDataMgr_Data_Access(uiDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            case eOptomaUART_BACKUP_RESTORE_MEMORY4:
                if(access(BACKUP_CONFIG_3_PATH, F_OK) == 0)
                {
                    ucSetting = (UINT8)(sCmdFormat->lData - 1);
                    palDataMgr_Data_Access(uiDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            case eOptomaUART_BACKUP_RESTORE_MEMORY5:
                if(access(BACKUP_CONFIG_4_PATH, F_OK) == 0)
                {
                    ucSetting = (UINT8)(sCmdFormat->lData - 1);
                    palDataMgr_Data_Access(uiDatacode, edaWRITE_THROUGH_WITH_ACTION, &ucSetting);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_MenuTransparency(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0037
{
    UINT16 DataCode = edcMENU_TRANSPARENCY;

    if(eAccessMode == ecmRead)
    {
        UINT32 ulVlaue = 0;

        palDataMgr_Data_Access(DataCode, edaREAD, &ulVlaue);
        sCmdFormat->lData= (INT32)(ulVlaue/MENU_TRANSPARENCY_STEP_VALUE);
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT32 ulVlaue = (UINT32)sCmdFormat->lData;
        INT32  iMax = 0;
        INT32  iMin = 0;

        ulVlaue = ulVlaue * MENU_TRANSPARENCY_STEP_VALUE;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(DataCode, edrMAX, &iMax)) &&
            (eEXEC_CODE_PASS == palDataMgr_Data_Range_Get(DataCode, edrMIN, &iMin)))
        {
            if((iMax >= (INT32)ulVlaue) && ((INT32)ulVlaue >= iMin))
            {
                palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &ulVlaue);
                return eCLI_ERROR_CODE_NO;
            }
        }
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BackLight(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0037
{
    UINT16 DataCode1 = edcKEYPAD_BACKLIGHT;
    UINT16 DataCode2 = edcPWRKEY_BACKLIGHT;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucBackLight = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode1)) || (eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode2)))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BACKLIGHT_KEYPAD_OFF:
            case eOptomaUART_BACKLIGHT_KEYPAD_ON:
            {
                if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode1, sCmdFormat->lData))
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
                }

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode1, sCmdFormat->lData))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                return eCLI_ERROR_CODE_NO;
            }
                break;

            case eOptomaUART_BACKLIGHT_PWRKEY_OFF:
                ucBackLight = ets_OFF;
                palDataMgr_Data_Access(DataCode2, edaWRITE_THROUGH_WITH_ACTION, &ucBackLight);
                break;

            case eOptomaUART_BACKLIGHT_PWRKEY_ON:
                ucBackLight = ets_ON;
                palDataMgr_Data_Access(DataCode2, edaWRITE_THROUGH_WITH_ACTION, &ucBackLight);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Black_Level_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //ZU860_Doulas_0101 modify//ZU860_Doulas_0022 //A65_OPTOMA_Julie_0046
{
    UINT16 DataCode = edcADV_BLACK_LEVEL_RESET;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BLACK_LEVEL_RESET_BOTTOM:
                Value = eBLACK_LEVEL_RESET_BOTTOM;
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_RESET, edaWRITE_THROUGH_WITH_ACTION, &Value);
                break;

            case eOptomaUART_BLACK_LEVEL_RESET_TOP:
                Value = eBLACK_LEVEL_RESET_TOP;
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_RESET, edaWRITE_THROUGH_WITH_ACTION, &Value);
                break;

            case eOptomaUART_BLACK_LEVEL_RESET_ALL:
                Value = eBLACK_LEVEL_RESET_ALL;
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_RESET, edaWRITE_THROUGH_WITH_ACTION, &Value);
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
                break;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Backup_Input_First_Input(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0100
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucValue = 0;

        palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaREAD, &ucValue);

        switch(ucValue)
        {
            case eBK_INPUT_SOURCE_HDMI1:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_HMI1;
                break;

            case eBK_INPUT_SOURCE_HDMI2:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_HMI2;
                break;

            case eBK_INPUT_SOURCE_HDBASET:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_HDBASET;
                break;

            case eBK_INPUT_SOURCE_DVI:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_DVI;
                break;

            default:
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BACKUP_INPUT_HMI1:
                ucValue = eBK_INPUT_SOURCE_HDMI1;
                break;

            case eOptomaUART_BACKUP_INPUT_HMI2:
                ucValue = eBK_INPUT_SOURCE_HDMI2;
                break;

            case eOptomaUART_BACKUP_INPUT_HDBASET:
                ucValue = eBK_INPUT_SOURCE_HDBASET;
                break;

            case eOptomaUART_BACKUP_INPUT_DVI:
                ucValue = eBK_INPUT_SOURCE_DVI;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcBACKUPINPUT_PRIMARY_INPUT))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palDataMgr_Data_Access(edcBACKUPINPUT_PRIMARY_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucValue) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Backup_Input_Second_Input(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0100
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucValue = 0;

        palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaREAD, &ucValue) ;

        switch(ucValue)
        {
            case eBK_INPUT_SOURCE_HDMI1:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_HMI1;
                break;

            case eBK_INPUT_SOURCE_HDMI2:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_HMI2;
                break;

            case eBK_INPUT_SOURCE_HDBASET:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_HDBASET;
                break;

            case eBK_INPUT_SOURCE_DVI:
                sCmdFormat->lData= (INT32)eOptomaUART_BACKUP_INPUT_DVI;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_BACKUP_INPUT_HMI1:
                ucValue = eBK_INPUT_SOURCE_HDMI1;
                break;

            case eOptomaUART_BACKUP_INPUT_HMI2:
                ucValue = eBK_INPUT_SOURCE_HDMI2;
                break;

            case eOptomaUART_BACKUP_INPUT_HDBASET:
                ucValue = eBK_INPUT_SOURCE_HDBASET;
                break;

            case eOptomaUART_BACKUP_INPUT_DVI:
                ucValue = eBK_INPUT_SOURCE_DVI;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(edcBACKUPINPUT_SECONDARY_INPUT))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palDataMgr_Data_Access(edcBACKUPINPUT_SECONDARY_INPUT, edaWRITE_THROUGH_WITH_ACTION, &ucValue) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Backup_Input_Current_Input(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)	//A65_OPTOMA_Julie_0088
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucValue = 0;

        palDataMgr_Data_Access(edcBACKUPINPUT_CURRENT_SOURCE, edaREAD, &ucValue) ;

        switch(ucValue)
        {
            default:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_NO_SIGNAL;// 0
                break;

            case eCM_SOURCE_HDMI1:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_HDMI1;// 7
                break;

            case eCM_SOURCE_HDMI2:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_HDMI2;// 8
                break;

            case eCM_SOURCE_DVI:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_DVI_D;// 1
                break;

            case eCM_SOURCE_3GSDI:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_3G_SDI;// 18
                break;

            case eCM_SOURCE_HDBASET:
                sCmdFormat->lData = eOptomaUART_MAIN_SOURCE_RES_HDBASET;// 16
                break;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Auto_Focus_Status(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0090 //A65_OPTOMA_Julie_0048
{
    UINT16 DataCode = edcAUTO_FOCUS_STATUS;
    UINT8 ucAF_Status = eAF_STATUS_TIMEOUT;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        eResult = palDataMgr_Data_Access(DataCode, edaREAD, &ucAF_Status);
        sCmdFormat->lData = ucAF_Status;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

#if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Tim_0012, add, start

eCLI_ERROR_CODE utilOptoma_ACU_Calibration(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084 //A65_OPTOMA_Julie_0083  //ZU860_Doulas_0101 modify//ZU860_Doulas_0022 //A65_OPTOMA_Julie_0048
{
    if(eAccessMode == ecmRead)
    {
        UINT8 ucACU_Status = eACU_STATUS_NONE;

        ucACU_Status = palDataMgr_ACU_Status_Get();

        if(ucACU_Status == eACU_STATUS_PROCESSING || ucACU_Status == eACU_STATUS_SUCCESS)
        {
            sCmdFormat->lData = ucACU_Status;
        }
        else if((ucACU_Status >= eACU_STATUS_TARGET_POSITION_ERR) && (ucACU_Status <= eACU_STATUS_TIME_OUT))
        {
            sCmdFormat->lData = 2;
        }
        else
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT16 DataCode;
        UINT8 ucValue = ets_ON;
        eEXEC_CODE eResult = eEXEC_CODE_PASS;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(sCmdFormat->lData == 1)
        {
            DataCode = edcACU_EXECUTE;

            if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
            {
                return eCLI_ERROR_CODE_DATAERROR;
            }

            eResult = palDataMgr_Data_Access(DataCode, edaWRITE_RAM_ONLY_WITH_ACTION, &ucValue);
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(sCmdFormat->lData == 9)
        {
            DataCode = edcACU_RESET;

            if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
            {
                return eCLI_ERROR_CODE_DATAERROR;
            }

            eResult = palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &ucValue);

            if(eResult != eEXEC_CODE_PASS)
            {
            return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}
#endif

eCLI_ERROR_CODE utilOptoma_Date_and_Time(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)    //A65_OPTOMA_Julie_0084 //A65_OPTOMA_Julie_0048
{
    if(eAccessMode == ecmRead)
    {
        UINT16 uwYear;
        UINT8 ucMonth;
        UINT8 ucDay;
        UINT8 ucHour;
        UINT8 ucMinute;
        char aucVerString[64];
        time_t sTime;
        struct tm *sTemp;

        time(&sTime);
        sTemp = localtime(&sTime);

        uwYear = (1900 + sTemp->tm_year);  //start at 1900 year
        ucMonth = (1 + sTemp->tm_mon);     //start at 0-11 month
        ucDay = sTemp->tm_mday;            //start at 01-31 day
        ucHour = sTemp->tm_hour;
        ucMinute = sTemp->tm_min;

        sprintf(aucVerString, "%04d%02d%02d%02d%02d",
        uwYear, ucMonth, ucDay,
        ucHour,
        ucMinute);

        sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
        sprintf(sCmdFormat->cTextString, "%s", aucVerString);

    }
    else if(eAccessMode == ecmWrite)
    {
        UINT16 DataCode;
        UINT8 ucValue = ets_ON;
        LOG_MSG(db_APP_CLI,"(%s,%d)ucCmdFrom[%d]cMainCode[%s]lData[%d]\r\n", __func__, __LINE__, sCmdFormat->ucDataType, sCmdFormat->cMainCode, sCmdFormat->lData);

        if(sCmdFormat->cMainCode[0] == '4' && sCmdFormat->cMainCode[1] == '7' && sCmdFormat->cMainCode[2] == '5')
        {
            DataCode = edcDATE_YEAR;
        }
        else if(sCmdFormat->cMainCode[0] == '4' && sCmdFormat->cMainCode[1] == '7' && sCmdFormat->cMainCode[2] == '6')
        {
            DataCode = edcDATE_MONTH;
        }
        else if(sCmdFormat->cMainCode[0] == '4' && sCmdFormat->cMainCode[1] == '7' && sCmdFormat->cMainCode[2] == '7')
        {
            DataCode = edcDATE_DAY;
        }
        else if(sCmdFormat->cMainCode[0] == '4' && sCmdFormat->cMainCode[1] == '7' && sCmdFormat->cMainCode[2] == '8')
        {
            DataCode = edcDATE_HOUR;
        }
        else if(sCmdFormat->cMainCode[0] == '4' && sCmdFormat->cMainCode[1] == '7' && sCmdFormat->cMainCode[2] == '9')
        {
            DataCode = edcDATE_MINUTE;
        }
        else
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        palDataMgr_Data_Access(edcDATE_APPLY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Clock_Mode(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValue = 0;
        UINT32 uiDataCode = edcDATE_MODE;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DECIMALERR;
        }

        switch(sCmdFormat->lData)
        {
            case eOptomaUART_CLOCK_NTP_SERVER:
                ucValue = 0;
                break;

            case eOptomaUART_CLOCK_MANUAL:
                ucValue = 1;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(uiDataCode))  //available check
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        palDataMgr_Data_Access(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &ucValue);
    }

	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_NTP_Server(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084
{
    UINT16 DataCode = edcDATE_NTP_SERVER;
    UINT8 ucValue = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(DataCode, edaREAD, &ucValue);
        sCmdFormat->lData = ucValue;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData > 0)
        {
            ucValue = (sCmdFormat->lData) - 1;
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Time_Zone(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084
{
    UINT16 DataCode = edcDATE_ZONE;
    UINT8 ucValue = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(DataCode, edaREAD, &ucValue);
        sCmdFormat->lData = ucValue;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData > 0)
        {
            ucValue = (sCmdFormat->lData) - 1;
        }
        else
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Update_Interval(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084
{
    UINT16 DataCode = edcDATE_UPDATE_INTERVAL;
    UINT8 ucValue = 0;

    if(eAccessMode == ecmRead)
    {
        palDataMgr_Data_Access(DataCode, edaREAD, &ucValue);
        sCmdFormat->lData = ucValue;
    }
    else if(eAccessMode == ecmWrite)
    {
        switch(sCmdFormat->lData)
        {
            case eOptomaUART_INTERVAL_HOURLY:
                ucValue = 0;
                break;

            case eOptomaUART_INTERVAL_DAILY:
                ucValue = 1;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Schedule_Function_Set(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084 //A65_OPTOMA_Julie_0083
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucValueOn = ets_ON;
        UINT8 ucValueOff = ets_OFF;
        UINT8 ucStringLen;

        if((sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC) && (sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(((sCmdFormat->cTextString[0] == '1') || (sCmdFormat->cTextString[0] == '0')) && (sCmdFormat->cTextString[1] == ' ') && (sCmdFormat->cTextString[2] == '~'))	//data value & command format
        {
            //LOG_MSG(db_ALWAYS,"(%s,%d)cTextString[3]=[%d] [%d][%s]\r\n", __func__, __LINE__, sCmdFormat->cTextString[3], sCmdFormat->lData, sCmdFormat->cTextString);
            ucStringLen = strlen((char*)sCmdFormat->cTextString);	//A65_OPTOMA_Julie_0088
            if(ucStringLen != 4)	//~XX284 0 ~n,4bytes
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            UINT32 uiDataCode = edcSCHEDULE_MONDAY_ENABLE;
            UINT8  ucWeekdayValue = sCmdFormat->cTextString[3] - 48;//Chr'0'= Dec(48)
            UINT8  ucWeekEnable = sCmdFormat->cTextString[0] - 48;

            LOG_MSG(db_APP_CLI,"(%s,%d)ucWeekdayValue[%d] ucWeekEnable[%d]\r\n", __func__, __LINE__, ucWeekdayValue, ucWeekEnable);

            if(ucWeekdayValue == 0)
            {
                uiDataCode = edcSCHEDULE_SUNDAY_ENABLE; //datacode[697-703]=[MONDAY,TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY,SUNDAY]
            }
            else if(ucWeekdayValue < 7)
            {
                uiDataCode = edcSCHEDULE_MONDAY_ENABLE + ucWeekdayValue - 1;
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            palDataMgr_Data_Access(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &ucWeekEnable);
        }

        else if(sCmdFormat->lData == 0)//H30K_David_0033
        {
            eResult = palDataMgr_Data_Access(edcSCHEDULE_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucValueOff);

            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(sCmdFormat->lData == 1)
        {

            eResult = palDataMgr_Data_Access(edcSCHEDULE_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucValueOn);

            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(sCmdFormat->lData == 9)
        {
            eResult = palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_ALL, edaWRITE_THROUGH_WITH_ACTION, &ucValueOn) ;
            if(eResult != eEXEC_CODE_PASS)
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Schedule_View_Today(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0048
{
    UINT16 DataCode = edcSCHEDULE_VIEW_TODAY;
    UINT8  ucTodayWeekday = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        eResult = palDataMgr_Data_Access(DataCode, edaREAD, &ucTodayWeekday); //[0-6]=[Sunday, Monday ~ Saturday]

        if(ucTodayWeekday == 0)
        {
            sCmdFormat->lData = 7;
        }
        else if(ucTodayWeekday <= 6)
        {
            sCmdFormat->lData = ucTodayWeekday;
        }
        else
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    return eCLI_ERROR_CODE_NO;
}

UINT8 utilOptoma_Schedule_Function_Data_Conversion(UINT8 ucFunc, UINT8 ucIndex) //A65_OPTOMA_Julie_0084
{
    UINT8 ucValue = eSCH_EVENT_TYPE_OFF;

    if(ucFunc == eSCH_EVENT_TYPE_POWER)
    {
        switch(ucIndex)
        {
            case eOptomaUART_SCH_EVENT_POWER_ON:
                ucValue = eSCH_EVENT_POWER_ON;
                break;

            case eOptomaUART_SCH_EVENT_POWER_OFF_STBY:
			#if(defined(PLATFORM_H30_4K))

			#else
                ucValue = eSCH_EVENT_POWER_OFF_STBY;
			#endif
                break;

            case eOptomaUART_SCH_EVENT_POWER_OFF_NSTBY:
			#if(defined(PLATFORM_H30_4K))

			#else
                ucValue = eSCH_EVENT_POWER_OFF_NSTBY;
			#endif
                break;

            case eOptomaUART_SCH_EVENT_POWER_OFF_CSTBY:
			#if(defined(PLATFORM_H30_4K))

			#else
                ucValue = eSCH_EVENT_POWER_OFF_CSTBY;
			#endif
                break;

            case eOptomaUART_SCH_EVENT_POWER_OFF:
                ucValue = eSCH_EVENT_POWER_OFF;
                break;

            default:
                break;
        }
    }
    else if(ucFunc == eSCH_EVENT_TYPE_INPUT_SOURCE)
    {
        switch(ucIndex)
        {
            case eOptomaUART_SCH_EVENT_INPUT_VGA:
			#if(defined(PLATFORM_H30_4K))

			#else
                ucValue = eSCH_EVENT_INPUT_VGA;
			#endif
                break;

            case eOptomaUART_SCH_EVENT_INPUT_HDMI1:
                ucValue = eSCH_EVENT_INPUT_HDMI1;
                break;

            case eOptomaUART_SCH_EVENT_INPUT_HDMI2:
                ucValue = eSCH_EVENT_INPUT_HDMI2;
                break;

            case eOptomaUART_SCH_EVENT_INPUT_DVI:
			#if(defined(PLATFORM_H30_4K))

			#else
                ucValue = eSCH_EVENT_INPUT_DVI;
			#endif
                break;

            case eOptomaUART_SCH_EVENT_INPUT_DP:
                ucValue = eSCH_EVENT_INPUT_DP;
                break;

            case eOptomaUART_SCH_EVENT_INPUT_3GSDI:
			#if(defined(PLATFORM_H30_4K))

			#else
                ucValue = eSCH_EVENT_INPUT_3GSDI;
			#endif
                break;

            case eOptomaUART_SCH_EVENT_INPUT_HDBT:
                ucValue = eSCH_EVENT_INPUT_HDBT;
                break;

            default:
                break;
        }
    }
    else if(ucFunc == eSCH_EVENT_TYPE_LIGHT_SOURCE)
    {
        switch(ucIndex)
        {
            case eOptomaUART_SCH_EVENT_LIGHT_NORMAL:
                ucValue = eSCH_EVENT_LIGHT_NORMAL;
                break;

            case eOptomaUART_SCH_EVENT_LIGHT_ECO:
                ucValue = eSCH_EVENT_LIGHT_ECO;
                break;

            case eOptomaUART_SCH_EVENT_LIGHT_CUSTOM:
                ucValue = eSCH_EVENT_LIGHT_CUSTOM;
                break;

            default:
                break;
        }
    }
    else if(ucFunc == eSCH_EVENT_TYPE_SHUTTER)
    {
        switch(ucIndex)
        {
            case eOptomaUART_SCH_EVENT_SHUTTER_ON:
                ucValue = eSCH_EVENT_SHUTTER_ON;
                break;

            case eOptomaUART_SCH_EVENT_SHUTTER_OFF:
                ucValue = eSCH_EVENT_SHUTTER_OFF;
                break;

            default:
                break;
        }
    }
    else
    {
        //no action.
    }
    return ucValue;
}

eCLI_ERROR_CODE utilOptoma_Schedule_Event_Set(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084//HICC2_Julie_0002
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_DataMustToBeString;
        }

        char cString[16];
        UINT8 cData[16];
        UINT8 ucStringLen = 0;

        ucStringLen = strlen((char*)&sCmdFormat->cTextString);

        for(UINT8 cIndex=0; cIndex < ucStringLen; cIndex++)
        {
            strncpy((char*)&cString[cIndex], (char*)&sCmdFormat->cTextString+cIndex, 1);
            cData[cIndex] = atoi(&cString[cIndex]);
        }
        //LOG_MSG(db_ALWAYS,"(%s,%d)[%s],cTextString[7]:[%d]\r\n", __func__, __LINE__, sCmdFormat->cTextString, sCmdFormat->cTextString[7]);

        if((sCmdFormat->cTextString[1] == ' ') && (sCmdFormat->cTextString[2] == '~'))
        {
            //~XX471 9 ~n, n=1~7 Mon. to Sunday
            if(cData[0] == 9 && ucStringLen == 4)
            {
                UINT8 ucValue = cData[3];
                UINT8 ucResetWeekdayValue = 0 ;
                UINT32 ucCurrentWeekdayValue = 0;
                UINT8 ucResetDayValue = 1;

                ucResetWeekdayValue = ((ucValue == 7)? 0 : ucValue);
                LOG_MSG(db_APP_CLI, "user wday reset (%d)\r\n", ucResetWeekdayValue);

                if(ucResetWeekdayValue > 6 || ucResetWeekdayValue == 0)
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE ;
                }

                palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekdayValue);
                palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucResetWeekdayValue);
                palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_DAY, edaWRITE_THROUGH_WITH_ACTION, &ucResetDayValue);
                palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekdayValue);
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }
        else if(ucStringLen > 8)
        {
            UINT16 DataCode;
            UINT8 ucWeekday;
            UINT8 cHour_digits, cMinute_digits;
            UINT16 uwTotalMinute = 0;
            UINT8 ucEventNum;
            UINT8 ucFunctionType;
            UINT8 ucValue, ucIndex, ucTypeIndex;
            UINT32 ucBackupCurrentWeekdayValue = 0;
            UINT8 ucBackupCurrentEventListNum = 0;
            sDST_SCHEDULE_EVENT_INFO sCurrentEvent = {0};

            //dhhmmnnabb
            //d=0~6 Sun. ~ Saturday
            //hhmm=00:00~23:59
            //nn=event 01~16
            //a=head Function a~d
            //bb=Funtions, 01~05

            //weekday = d
            DataCode = edcSCHEDULE_VIEW_WEEKDAY;
            ucWeekday = cData[0];

            if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucWeekday))
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            //Time = hhmm
            DataCode = edcSCHEDULE_EVENT_TIME;
            cHour_digits = (cData[1] * 10) + cData[2];
            cMinute_digits = (cData[3] * 10) + cData[4];
            uwTotalMinute = (cHour_digits * 60) + cMinute_digits;

            if(cHour_digits < 24 && cMinute_digits < 60)
            {
                if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, uwTotalMinute))
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
                }
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            //Event = nn
            ucEventNum = (cData[5] * 10) + cData[6] - 1;
            if(ucEventNum < 0 || ucEventNum > 15)
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            LOG_MSG(db_APP_CLI, "(%s,%d)ucWeekday[%d] uwTotalMinute[%d] ucEventNum[%d]\r\n", __func__, __LINE__, ucWeekday, uwTotalMinute, ucEventNum);

            //Function = a~d
            if(((sCmdFormat->cTextString[7] >= 'a') && (sCmdFormat->cTextString[7] <= 'd')))
            {
                ucFunctionType = sCmdFormat->cTextString[7] - 97 + 1; //Chr'a'= Dec(97) = Hex(0x61)
                DataCode = edcSCHEDULE_EVENT_TYPE;

                if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucFunctionType))
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
                }
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            //Funtions index = bb
            if(ucStringLen == 9)
            {
                ucValue = cData[8];
            }
            else if (ucStringLen == 10)
            {
                ucValue = (cData[8] * 10) + cData[9];
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            ucIndex = utilOptoma_Schedule_Function_Data_Conversion(ucFunctionType, ucValue);

            if(ucIndex == 0)
            {
                return eCLI_ERROR_CODE_DATAERROR;
            }
            ucTypeIndex = palDataMgr_Schedule_CommonScheduleEventIndex_Translate_GuiEventIndexValue(ucFunctionType, ucIndex); //A65_OPTOMA_Julie_0088

            DataCode = edcSCHEDULE_EVENT_TYPE_INDEX;

            if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucTypeIndex))
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
            LOG_MSG(db_APP_CLI, "(%s,%d)ucFunctionType[%d] ucValue[%d] ucTypeIndex[%d]\r\n", __func__, __LINE__, ucFunctionType, ucValue, ucTypeIndex);

            //-------------------
            palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &sCurrentEvent);

            //backup system value
            ucBackupCurrentEventListNum = sCurrentEvent.ucCurrentEventListNum;
            palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucBackupCurrentWeekdayValue);

            //set event reset

            LOG_MSG(db_APP_SCHEDULE, "-->user wday %d event %d set %d/%d (%s,%d)\r\n", ucWeekday, uwTotalMinute, ucFunctionType, ucTypeIndex, sCmdFormat->cTextString, strlen((char *)sCmdFormat->cTextString));

            palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucWeekday);//set weekday [0-6]=[Sunday, Monday ~ Saturday]

            sCurrentEvent.ucCurrentEventListNum = ucEventNum;
            palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);//set event list
            palDataMgr_Data_Access(edcSCHEDULE_EVENT_TIME, edaWRITE_THROUGH_WITH_ACTION, &uwTotalMinute);//set time
            palDataMgr_Data_Access(edcSCHEDULE_EVENT_TYPE, edaWRITE_THROUGH_WITH_ACTION, &ucFunctionType);
            palDataMgr_Data_Access(edcSCHEDULE_EVENT_TYPE_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucTypeIndex);
            MS_SLEEP(1);
            palDataMgr_Schedule_Sort_Event_List_By_ExecuteTime();
            palLANProcScheduleSend();

            //restore system value

            sCurrentEvent.ucCurrentEventListNum = ucBackupCurrentEventListNum;
            palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
            palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucBackupCurrentWeekdayValue);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Schedule_Weekday_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084//HICC2_Julie_0002
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        char cString[16];
        UINT8 cData[16];
        UINT8 ucValue;
        UINT8 ucStringLen = 0;
        ucStringLen = strlen((char*)&sCmdFormat->cTextString);

        for(UINT8 cIndex=0; cIndex < ucStringLen; cIndex++)
        {
            strncpy((char*)&cString[cIndex], (char*)&sCmdFormat->cTextString+cIndex, 1);
            cData[cIndex] = atoi(&cString[cIndex]);
            //LOG_MSG(db_ALWAYS,"(%s,%d)%d:[%d]\r\n", __func__, __LINE__, cIndex, cData[cIndex]);
        }

        //~XX472 d ~nn, d=0~6, ~nn=01~16
        UINT32 ucCurrentWeekdayValue = 0;
        UINT8 ucCurrentEventListNum = 0;
        UINT8 ucUserEventListNum = 0;
        UINT32 ucUserChoiceWeekday = 0;
        UINT8 ucIndex = 1;
        sDST_SCHEDULE_EVENT_INFO sCurrentEvent = {0};

        if((sCmdFormat->cTextString[1] == ' ') && (sCmdFormat->cTextString[2] == '~'))
        {
            //d=0~6
            if(cData[0] >=0 && cData[0] <=6)
            {
                ucUserChoiceWeekday = cData[0];
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            //nn=01~16
            if(ucStringLen == 4)
            {
                ucValue = cData[3];
            }
            else if (ucStringLen == 5)
            {
                ucValue = (cData[3] * 10) + cData[4];
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            if(ucValue > 0 && ucValue < 17)
            {
                ucUserEventListNum = ucValue - 1;
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        LOG_MSG(db_APP_CLI, "user event reset weekday %d event %d\r\n", ucUserChoiceWeekday, ucUserEventListNum);

        //backup system value
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaREAD, &sCurrentEvent);

        ucCurrentEventListNum = sCurrentEvent.ucCurrentEventListNum;

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Get(edcSCHEDULE_VIEW_WEEKDAY, &ucCurrentWeekdayValue);

        //set event reset
        sCurrentEvent.ucCurrentEventListNum = ucUserEventListNum;

        LOG_MSG(db_APP_SCHEDULE, "user event reset %d (%s,%d)\r\n", ucUserEventListNum, sCmdFormat->cMainCode, ucUserChoiceWeekday);
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //ppGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucUserChoiceWeekday);
        palDataMgr_Data_Access(edcSCHEDULE_EVENT_RESET_FUCNTION, edaWRITE_THROUGH_WITH_ACTION, &ucIndex); //appGui_DataCode_Value_Set(edcSCHEDULE_EVENT_RESET_FUCNTION, edaWRITE_THROUGH_WITH_ACTION, 1);
        MS_SLEEP(1);
        palDataMgr_Schedule_Sort_Event_List_By_ExecuteTime();
        palLANProcScheduleSend();

        //restore system value
        sCurrentEvent.ucCurrentEventListNum = ucCurrentEventListNum - 1 ;
        palDataMgr_Data_Access(edcUI_SCHEDULE_EVENT_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, &sCurrentEvent);
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekdayValue); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, ucCurrentWeekdayValue);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Schedule_Copy_Weekday(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0084//HICC2_Julie_0002
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        char cString[16];
        UINT8 cData[16];
        UINT8 ucStringLen = 0;
        ucStringLen = strlen((char*)&sCmdFormat->cTextString);

        for(UINT8 cIndex=0; cIndex < ucStringLen; cIndex++)
        {
            strncpy((char*)&cString[cIndex], (char*)&sCmdFormat->cTextString+cIndex, 1);
            cData[cIndex] = atoi(&cString[cIndex]);
            //LOG_MSG(db_ALWAYS,"(%s,%d)%d:[%d]\r\n", __func__, __LINE__, cIndex, cData[cIndex]);
        }

        //~XX473 n ~n, n=1~7 Mon. to Sunday
        UINT32 ucCurrentWeekday = 0;
        UINT8 ucCopyWeekday = 0;
        UINT32 ucUserChoiceWeekday = 0;
        UINT8 ucValue;

        if((sCmdFormat->cTextString[1] == ' ') && (sCmdFormat->cTextString[2] == '~'))
        {
            //copy weekday
            ucValue = cData[0];
            if(ucValue > 0)
            {
            ucUserChoiceWeekday = ((ucValue == 7)? 0 : ucValue);
            }
            else
            {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
            }

            //target weekday
            ucValue = cData[3];
            if(ucValue > 0)
            {
                ucCopyWeekday = ((ucValue == 7)? 0 : ucValue);
            }
            else
            {
                return eCLI_ERROR_CODE_DATAOVERRANGE;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(ucCopyWeekday > 6|| ucUserChoiceWeekday > 6 || (ucCopyWeekday == ucUserChoiceWeekday))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE ;
        }

        //backup system value
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaREAD, &ucCurrentWeekday);

        //set copy event

        LOG_MSG(db_APP_SCHEDULE, "user Copy wday from %d (%s) to %d\r\n", ucUserChoiceWeekday, sCmdFormat->cMainCode, ucCopyWeekday);

        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucUserChoiceWeekday); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucUserChoiceWeekday);

        //appDataMgr_Schedule_User_Current_Copy_Weekday_Num_Set(ucCopyWeekday);
        palDataMgr_Data_Access(edcSCHEDULE_COPY_EVENT_INDEX, edaWRITE_THROUGH_WITH_ACTION, &ucCopyWeekday); //utilCommonCLI_DataConversionSet(edcSCHEDULE_COPY_EVENT_INDEX, ucCopyWeekday);
        //appGui_DataCode_Value_Set(edcSCHEDULE_COPY_EVENT_INDEX, edaWRITE_THROUGH_WITH_ACTION, ucCopyWeekday);

        //restore system value
        palDataMgr_Data_Access(edcSCHEDULE_VIEW_WEEKDAY, edaWRITE_THROUGH_WITH_ACTION, &ucCurrentWeekday); //appGui_DataCode_Value_Set(edcSCHEDULE_VIEW_WEEKDAY, ucCurrentWeekday);
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Fade_In(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0048
{
    UINT16 DataCode = edcFADE_IN;
	INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))//H30K_David_0023
        {
            sCmdFormat->lData = Value/FADE_TIMER_STEP_VALUE;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 Value = (sCmdFormat->lData) * FADE_TIMER_STEP_VALUE;

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, Value))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        else
        {
            palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value);
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Fade_Out(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0048
{
    UINT16 DataCode = edcFADE_OUT;
	INT32 Value = 0;



    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))//H30K_David_0024
        {
            sCmdFormat->lData = Value/FADE_TIMER_STEP_VALUE;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

    }
    else if(eAccessMode == ecmWrite)
    {
        INT32 Value = (sCmdFormat->lData) * FADE_TIMER_STEP_VALUE;

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, Value))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }
        else
        {
            palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &Value);
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelEnable(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 ValueEnable = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        DataCode = edcADV_BLACK_LEVEL_AREA;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        DataCode = edcADV_BLACK_LEVEL_ENABLE;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_BLACK_LEVEL_BOTTOM_OFF:
                ValueArea    = BLACKLEVEL_AREA__BOTTOM;
                ValueEnable  = ets_OFF;
                break;

            case CLI_BLACK_LEVEL_TOP_OFF:
                ValueArea    = BLACKLEVEL_AREA__TOP;
                ValueEnable  = ets_OFF;
                break;

            case CLI_BLACK_LEVEL_BOTTOM_ON:
                ValueArea    = BLACKLEVEL_AREA__BOTTOM;
                ValueEnable  = ets_ON;
                break;

            case CLI_BLACK_LEVEL_TOP_ON:
                ValueArea    = BLACKLEVEL_AREA__TOP;
                ValueEnable  = ets_ON;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_AREA, ValueArea))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_ENABLE, ValueEnable))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelBrightness(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        DataCode = edcADV_BLACK_LEVEL_AREA;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        DataCode = edcADV_BLACK_LEVEL_BRIGHTNESS;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_BLACK_LEVEL_BRIGHTNESS_BOTTOM_ADD:
                ValueArea = BLACKLEVEL_AREA__BOTTOM;

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_AREA, ValueArea))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_BRIGHTNESS, edaREAD, &Value);
                Value++;
                break;

            case CLI_BLACK_LEVEL_BRIGHTNESS_BOTTOM_DEL:
                ValueArea = BLACKLEVEL_AREA__BOTTOM;

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_AREA, ValueArea))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_BRIGHTNESS, edaREAD, &Value);
                Value--;
                break;

            case CLI_BLACK_LEVEL_BRIGHTNESS_TOP_ADD:
                ValueArea = BLACKLEVEL_AREA__TOP;

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_AREA, ValueArea))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_BRIGHTNESS, edaREAD, &Value);
                Value++;
                break;

            case CLI_BLACK_LEVEL_BRIGHTNESS_TOP_DEL:
                ValueArea = BLACKLEVEL_AREA__TOP;

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_AREA, ValueArea))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                palDataMgr_Data_Access(edcADV_BLACK_LEVEL_BRIGHTNESS, edaREAD, &Value);
                Value--;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcADV_BLACK_LEVEL_BRIGHTNESS, Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelBottomRed(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    DataCode = edcADV_BLACK_LEVEL_AREA;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    ValueArea = BLACKLEVEL_AREA__BOTTOM;

    if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ValueArea))
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    DataCode = edcADV_BLACK_LEVEL_RED;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelTopRed(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    DataCode = edcADV_BLACK_LEVEL_AREA;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    ValueArea = BLACKLEVEL_AREA__TOP;

    if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ValueArea))
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    DataCode = edcADV_BLACK_LEVEL_RED;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelBottomGreen(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    DataCode = edcADV_BLACK_LEVEL_AREA;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    ValueArea = BLACKLEVEL_AREA__BOTTOM;

    if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ValueArea))
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    DataCode = edcADV_BLACK_LEVEL_GREEN;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelTopGreen(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    DataCode = edcADV_BLACK_LEVEL_AREA;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    ValueArea = BLACKLEVEL_AREA__TOP;

    if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ValueArea))
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    DataCode = edcADV_BLACK_LEVEL_GREEN;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelBottomBlue(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    DataCode = edcADV_BLACK_LEVEL_AREA;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    ValueArea = BLACKLEVEL_AREA__BOTTOM;

    if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ValueArea))
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    DataCode = edcADV_BLACK_LEVEL_BLUE;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_BlackLevelTopBlue(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0168
{
    UINT16 DataCode;
    INT32 ValueArea = 0;
    INT32 Value = 0;

    DataCode = edcADV_BLACK_LEVEL_AREA;

    if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_DATAERROR;
    }

    ValueArea = BLACKLEVEL_AREA__TOP;

    if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ValueArea))
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    DataCode = edcADV_BLACK_LEVEL_BLUE;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_DigitalZoom_Shift_And_Reset(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0174
{
    UINT16 DataCode;
    UINT8 ucValue = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_DIGITAL_ZOOM_PROPORTIONAL_OFF:
            case CLI_DIGITAL_ZOOM_PROPORTIONAL_ON:
                DataCode = edcDIGITAL_ZOOM_PROPORTIONAL;
                if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }

                if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
                {
                    return eCLI_ERROR_CODE_DATAOVERRANGE;
                }

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

			case CLI_DIGITAL_ZOOM_RESET:
				DataCode = edcDIGITAL_ZOOM_RESET;
                if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
                {
                    return eCLI_ERROR_CODE_DATAERROR;
                }

                if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ets_ON))
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;


            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Warp_Control(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Doulas_0174
{
    UINT16 DataCode;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_WARP_CONTROL_BASIC:
                Value = WARP_CTRL__BASIC;
                break;

            case CLI_WARP_CONTROL_ADVANCED:
                Value = WARP_CTRL__ADVANCED;
                break;

            case CLI_WARP_CONTROL_AP:
                Value = WARP_CTRL__AP;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }

        DataCode = edcADV_WARP_CONTROL;

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Lan_Path_Switch(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)// HICC2_Bruce_0021
{
    if(eAccessMode == ecmRead)
    {
    	return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->lData != 0 && sCmdFormat->lData != 1)
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        palDataMgr_Data_Access(edcLAN_PATH_SWITCH, edaWRITE_RAM_ONLY_WITH_ACTION, &sCmdFormat->lData);
        palDataMgr_ExecuteLanIPApply(1);
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Crestron_IP_Address(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A65_OPTOMA_Doulas_0177
{
    if(eAccessMode == ecmRead)
    {
    	return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucVAl;
        UINT8 ucStringLen = 0;
        INT16 uiPassword[4] = {0};
        char  cString[20] = {0};
        char  cString2[20] = {0};
        char *pch ;
        UINT8 ucIndex = 0 ;

        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        //LOG_MSG(db_ALWAYS, "sCmdFormat->ucDataType = %d\r\n",sCmdFormat->ucDataType);
        //LOG_MSG(db_ALWAYS, "string(%d) = %s\r\n",ucStringLen,sCmdFormat->cTextString);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen != 18)	// 1 ~nnn.nnn.nnn.nnn ,18bytes
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if (sCmdFormat->cTextString[0] != '1')	//data value
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))	//check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[6] != '.') ||
            (sCmdFormat->cTextString[10] != '.') ||
            (sCmdFormat->cTextString[14] != '.'))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        for(ucVAl = 0 ; ucVAl < (ucStringLen-3); ucVAl++) //A65_OPTOMA_Julie_0085 reslove compile warn.
        {
            if(sCmdFormat->cTextString[3+ucVAl] != '.')
            {
                if(__CheckNum(sCmdFormat->cTextString[3+ucVAl]) == 0)
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            cString[ucVAl] = sCmdFormat->cTextString[3+ucVAl];
        }

        memcpy(cString2, cString, 20 * sizeof(char));

        pch = strtok((char *)cString2, ".") ;   // ¥H . ³o­?¦r¤¸¨?¤?³?¦r¦ê
        while(pch != NULL && ucIndex < 4)
        {
            uiPassword[ucIndex] = atoi(pch) ;
            ucIndex++ ;
            pch = strtok(NULL, ".") ;
        }

        for(ucVAl = 0 ; ucVAl < 4 ; ucVAl++)	//range check
        {
            if((uiPassword[ucVAl] > 255) || (uiPassword[ucVAl] < 0))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        //LOG_MSG(db_ALWAYS, " $$String = %s (%d,%d,%d,%d)\r\n", cString, uiPassword[0],uiPassword[1],uiPassword[2],uiPassword[3]);
        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcCRESTRON_ADDRESS, edaWRITE_THROUGH_WITH_ACTION, cString))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Crestron_IPID(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A65_OPTOMA_Doulas_0177
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucVAl;
        UINT8 ucStringLen = 0;
        INT16 uiVal = 0;
        char  cString[5] = {0};

        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        //LOG_MSG(db_ALWAYS, "sCmdFormat->ucDataType = %d\r\n",sCmdFormat->ucDataType);
        //LOG_MSG(db_ALWAYS, "string(%d) = %s\r\n",ucStringLen,sCmdFormat->cTextString);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen > 6 || ucStringLen <= 3)    // 1 ~nnn ,6bytes  //A35G2_Coda_0062
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if (sCmdFormat->cTextString[0] != '1')  //data value
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))  //check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        for(ucVAl = 0 ; ucVAl < 3 ; ucVAl++)
        {
            if(sCmdFormat->cTextString[3 + ucVAl] != 0)  //A35G2_Coda_0062
            {
                if(__CheckNum(sCmdFormat->cTextString[3 + ucVAl]) == 0)
                {
                	return eCLI_ERROR_CODE_SETFAIL;
                }
                cString[ucVAl] = sCmdFormat->cTextString[3 + ucVAl];
            }
        }

        uiVal = atoi(cString) ;

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(edcCRESTRON_PID, uiVal))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        //LOG_MSG(db_ALWAYS, " $$String = %s (%s)\r\n", sCmdFormat->cTextString, ucPWord);
        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcCRESTRON_PID, uiVal))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Crestron_Port(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A65_OPTOMA_Doulas_0177
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucVAl;
        UINT8 ucStringLen = 0;
        INT32 uiVal = 0;
        char  cString[7] = {0};

        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        //LOG_MSG(db_ALWAYS, "sCmdFormat->ucDataType = %d\r\n",sCmdFormat->ucDataType);
        //LOG_MSG(db_ALWAYS, "string(%d) = %s\r\n",ucStringLen,sCmdFormat->cTextString);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen > 8 || ucStringLen <= 3)	// 1 ~nnnnn ,8bytes  //A35G2_Coda_0062
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if (sCmdFormat->cTextString[0] != '1')	//data value
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))	//check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        for(ucVAl = 0 ; ucVAl < 5 ; ucVAl++)
        {
            if(sCmdFormat->cTextString[3 + ucVAl] != 0)  //A35G2_Coda_0062
            {
                if(__CheckNum(sCmdFormat->cTextString[3 + ucVAl]) == 0)
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                cString[ucVAl] = sCmdFormat->cTextString[3 + ucVAl];
            }
        }

        uiVal = atoi(cString) ;

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(edcCRESTRON_PORT, uiVal))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        //LOG_MSG(db_ALWAYS, " $$String = %s (%s)\r\n", sCmdFormat->cTextString, ucPWord);
        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(edcCRESTRON_PORT, uiVal))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_PJLink_Service(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)   //A65_OPTOMA_Doulas_0179
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucVAl;
        UINT8 ucStringLen = 0;
        INT16 uiPassword[4] = {0};
        char  cString[20] = {0};
        char  cString2[20] = {0};
        char *pch ;
        UINT8 ucIndex = 0 ;

        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        //LOG_MSG(db_ALWAYS, "sCmdFormat->ucDataType = %d\r\n",sCmdFormat->ucDataType);
        //LOG_MSG(db_ALWAYS, "string(%d) = %s\r\n",ucStringLen,sCmdFormat->cTextString);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen != 18)	// 1 ~nnn.nnn.nnn.nnn ,18bytes
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if (sCmdFormat->cTextString[0] != '1')	//data value
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))	//check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[6] != '.') ||
            (sCmdFormat->cTextString[10] != '.') ||
            (sCmdFormat->cTextString[14] != '.'))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        for(ucVAl = 0 ; ucVAl < (ucStringLen-3); ucVAl++) //A65_OPTOMA_Julie_0085 reslove compile warn.
        {
            if(sCmdFormat->cTextString[3+ucVAl] != '.')
            {
                if(__CheckNum(sCmdFormat->cTextString[3+ucVAl]) == 0)
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            cString[ucVAl] = sCmdFormat->cTextString[3+ucVAl];
        }

        memcpy(cString2, cString, strlen(cString)+1); //A65_OPTOMA_Julie_0088

        pch = strtok((char *)cString2, ".") ;   // ¥H . ³o­?¦r¤¸¨?¤?³?¦r¦ê
        while(pch != NULL && ucIndex < 4)
        {
            uiPassword[ucIndex] = atoi(pch) ;
            ucIndex++ ;
            pch = strtok(NULL, ".") ;
        }

        for(ucVAl = 0 ; ucVAl < 4 ; ucVAl++)	//range check
        {
            if((uiPassword[ucVAl] > 255) || (uiPassword[ucVAl] < 0))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        //LOG_MSG(db_ALWAYS, " $$String = %s (%d,%d,%d,%d)\r\n", cString, uiPassword[0],uiPassword[1],uiPassword[2],uiPassword[3]);
        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(edcPJLINK_ADDRESS, edaWRITE_THROUGH_WITH_ACTION, cString))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_SNMP_Setting(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)     //A65_OPTOMA_Doulas_0179
{
    UINT16 DataCode = edcSNMP_TRAP_FAN_STALL;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucDataIndex = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_SNMP_FAN_ERROR_EMAIL:
                DataCode = edcSNMP_TRAP_FAN_STALL;
                ucDataIndex = ets_ON;
                break;

            case CLI_SNMP_FAN_ERROR_OFF:
                DataCode = edcSNMP_TRAP_FAN_STALL;
                ucDataIndex = ets_OFF;
                break;

            case CLI_SNMP_POWER_ONOFF_EMAIL:
                DataCode = edcSNMP_TRAP_POWER;
                ucDataIndex = ets_ON;
                break;

            case CLI_SNMP_POWER_ONOFF_OFF:
                DataCode = edcSNMP_TRAP_POWER;
                ucDataIndex = ets_OFF;
                break;

            case CLI_SNMP_VIDEO_LOSS_EMAIL:
                DataCode = edcSNMP_TRAP_SIGNAL_LOSS;
                ucDataIndex = ets_ON;
                break;

            case CLI_SNMP_VIDEO_LOSS_OFF:
                DataCode = edcSNMP_TRAP_SIGNAL_LOSS;
                ucDataIndex = ets_OFF;
                break;

            case CLI_SNMP_LASER_EMAIL:
                DataCode = edcSNMP_TRAP_LAMP_LIFE;
                ucDataIndex = ets_ON;
                break;

            case CLI_SNMP_LASER_OFF:
                DataCode = edcSNMP_TRAP_LAMP_LIFE;
                ucDataIndex = ets_OFF;
                break;

            default:
                return eCLI_ERROR_CODE_SETFAIL;
                break;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucDataIndex))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ucDataIndex))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}




eCLI_ERROR_CODE utilOptoma_Network_Control(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0095 //A35G2_Coda_0061
{
    UINT16 DataCode = edcLAN_APPLY;
    INT32 Value = ets_ON;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case CLI_NETWORK_SETUP_RESET:
                DataCode =edcNETWORK_RESET_ONLY;
				if(eFUNC_CONTROL_ENABLE != palDataMgr_MenuItem_Control(eacART_NET_ON_2X_10X_DISABLE))//Art-Net available check
		        {
		            return eCLI_ERROR_CODE_DATAERROR;
		        }

				if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
		        {
		            return eCLI_ERROR_CODE_DATAERROR;
		        }

		        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
		        {
		            return eCLI_ERROR_CODE_SETFAIL;
		        }

 				DataCode =edcUI_EVENT_NETWORK_WAIT_MSG_EXIT;//H30K_David_0035
				if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))//H30K_David_0035
		        {
		            return eCLI_ERROR_CODE_SETFAIL;
		        }
                break;

            case CLI_NETWORK_SETUP_APPLY:
                DataCode = edcLAN_APPLY;
		        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
		        {
		            return eCLI_ERROR_CODE_DATAERROR;
		        }

		        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
		        {
		            return eCLI_ERROR_CODE_SETFAIL;
		        }
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }


    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Crestron_Control(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0095 //A35G2_Coda_0061
{
    UINT16 DataCode;
    INT32 Value = ets_ON;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case 0:
            case 1:
                DataCode = edcCrestron;
                Value = sCmdFormat->lData;
                break;

            case 9:
                DataCode = edcCRESTRON_APPLY;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_PJLink_Control(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0095 //A35G2_Coda_0061
{
    UINT16 DataCode;
    INT32 Value = ets_ON;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        switch(sCmdFormat->lData)
        {
            case 0:
            case 1:
                DataCode = edcPJ_Link;
                Value = sCmdFormat->lData;
                break;

            case 9:
                DataCode = edcPJLINK_APPLY;
                break;

            default:
                return eCLI_ERROR_CODE_DATAERROR;
                break;
        }

        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_PJLink_Password(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)			//A65_OPTOMA_Doulas_0226 //A35G2_Coda_0087
{
    UINT16 DataCode;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucStringLen = 0;
        INT32 lMaxValue = 0;
        INT32 lMinValue = 0;
        UINT8 ucVAl;

        DataCode = edcPJLINK_SECRET_PASSWORD;
        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        palDataMgr_Data_Range_Get(DataCode, edrMAX, &lMaxValue);
        palDataMgr_Data_Range_Get(DataCode, edrMIN, &lMinValue);

        //LOG_MSG(db_ALWAYS,"#%s , %d\r\n",sCmdFormat->cTextString,ucStringLen);
        if(eFUNC_CONTROL_ENABLE != palDataMgr_DataCode_Control(DataCode))  //available check
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(ucStringLen > lMaxValue || ucStringLen < lMinValue)	//check string length
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        for(ucVAl = 0 ; ucVAl < ucStringLen; ucVAl++)
        {
            if((sCmdFormat->cTextString[ucVAl] < '0') ||
                ((sCmdFormat->cTextString[ucVAl] > '9') && (sCmdFormat->cTextString[ucVAl] < 'A')) ||
                ((sCmdFormat->cTextString[ucVAl] > 'Z') && (sCmdFormat->cTextString[ucVAl] < 'a')) ||
                (sCmdFormat->cTextString[ucVAl] > 'z'))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, (char*)sCmdFormat->cTextString))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

///////////////////////////////////////////////////////////////

// ==============================================================================
// FUNCTION NAME: m_sCli_Config
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
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
const static sCLI_CONFIG m_sCli_Custom_Config =
{
    eccPC, OPTOMA_CMD_HEADER_1, OPTOMA_CMD_END_1
};

#ifdef CLI_ERROR_LOG_ENABLE //A65_OPTOMA_Jerry_0006
eCLI_ERROR_CODE utilOptoma_AP_DebugCmd_Enable(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case OPTOMA_CMD_DEBUG_STEP_01: // Step 1
                if( bOptoma_Cmd_Debug_Enable == OPTOMA_CMD_DEBUG_DISABLE )
                {
                    bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_STEP_01;
                }
                else
                {
                    bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_DISABLE;
                }
                LOG_MSG(db_ALWAYS, "sCmdFormat->lData:%d bOptoma_Cmd_Debug_Enable: %d\r\n", sCmdFormat->lData, bOptoma_Cmd_Debug_Enable);
                break;

            case OPTOMA_CMD_DEBUG_STEP_03: // Step 2
                if( bOptoma_Cmd_Debug_Enable == OPTOMA_CMD_DEBUG_STEP_01 )
                {
                    bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_STEP_03;
                }
                else
                {
                    bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_DISABLE;
                }
                LOG_MSG(db_ALWAYS, "sCmdFormat->lData:%d bOptoma_Cmd_Debug_Enable: %d\r\n", sCmdFormat->lData, bOptoma_Cmd_Debug_Enable);
                break;

            case OPTOMA_CMD_DEBUG_ENABLE: //Step 3
                if( bOptoma_Cmd_Debug_Enable == OPTOMA_CMD_DEBUG_STEP_03 )
                {
                    bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_ENABLE;
                }
                else
                {
                    bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_DISABLE;
                }
                LOG_MSG(db_ALWAYS, "sCmdFormat->lData:%d bOptoma_Cmd_Debug_Enable: %d\r\n", sCmdFormat->lData, bOptoma_Cmd_Debug_Enable);
                break;

            default: // Disable
                LOG_MSG(db_ALWAYS, "sCmdFormat->lData:%d bOptoma_Cmd_Debug_Enable: %d\r\n", sCmdFormat->lData, bOptoma_Cmd_Debug_Enable);
                bOptoma_Cmd_Debug_Enable = OPTOMA_CMD_DEBUG_DISABLE;
                break;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_AP_DebugCmd(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT32 uiSystime_Minutes = palIllumination_TotalProjectSec_Get();

        LOG_MSG(db_ALWAYS, "utilOptoma_AP_DebugCmd bOptoma_Cmd_Debug_Enable: %d\r\n", bOptoma_Cmd_Debug_Enable);

        if( bOptoma_Cmd_Debug_Enable != OPTOMA_CMD_DEBUG_ENABLE )
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        switch(sCmdFormat->lData)
        {
            case 1:
                palLANProcSendLog((UINT32) Volt50VLost, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eVolt50VLost\r\n", Volt50VLost);
                break;

            case 2:
                palLANProcSendLog((UINT32) Volt12VLost, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eVolt12VLost\r\n", Volt12VLost);
                break;

            case 3:
                palLANProcSendLog((UINT32) LightModuleOCP01, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eLightModuleOCP01\n", LightModuleOCP01);
                break;

            case 4:
                palLANProcSendLog((UINT32) LightModuleOCP02, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eLightModuleOCP02\r\n", LightModuleOCP02);
                break;

            case 5:
                palLANProcSendLog((UINT32) SystemOverTemp, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eSystemOverTemp\r\n", SystemOverTemp);
                break;

            case 6:
                palLANProcSendLog((UINT32) LightModuleOverTemp01, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eLightModuleOverTemp01\r\n", LightModuleOverTemp01);
                break;

            case 7:
                palLANProcSendLog((UINT32) LightModuleOverTemp02, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eLightModuleOverTemp02\r\n", LightModuleOverTemp02);
                break;

            case 8:
                palLANProcSendLog((UINT32) FanLock01, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock01\r\n", FanLock01);
                break;

            case 9:
                palLANProcSendLog((UINT32) FanLock02, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock02\r\n", FanLock02);
                break;

            case 10:
                palLANProcSendLog((UINT32) FanLock03, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock03\r\n", FanLock03);
                break;

            case 11:
                palLANProcSendLog((UINT32) FanLock04, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock04\r\n", FanLock04);
                break;

            case 12:
                palLANProcSendLog((UINT32) FanLock05, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock05\r\n", FanLock05);
                break;

            case 13:
                palLANProcSendLog((UINT32) FanLock06, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock06\r\n", FanLock06);
                break;

            case 14:
                palLANProcSendLog((UINT32) FanLock07, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock07\r\n", FanLock07);
                break;

            case 15:
                palLANProcSendLog((UINT32) FanLock08, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanLock08\r\n", FanLock08);
                break;

            case 16:
                palLANProcSendLog((UINT32) FanStallError01, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall01\r\n", FanStallError01);
                break;

            case 17:
                palLANProcSendLog((UINT32) FanStallError02, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall02\r\n", FanStallError02);
                break;

            case 18:
                palLANProcSendLog((UINT32) FanStallError03, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall03\r\n", FanStallError03);
                break;

            case 19:
                palLANProcSendLog((UINT32) FanStallError04, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall04\r\n", FanStallError04);
                break;

            case 20:
                palLANProcSendLog((UINT32) FanStallError05, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall05\r\n", FanStallError05);
                break;

            case 21:
                palLANProcSendLog((UINT32) FanStallError06, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall06\r\n", FanStallError06);
                break;

            case 22:
                palLANProcSendLog((UINT32) FanStallError07, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall07\r\n", FanStallError07);
                break;

            case 23:
                palLANProcSendLog((UINT32) FanStallError08, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eFanStall08\r\n", FanStallError08);
                break;

            case 24:
                palLANProcSendLog((UINT32) DMDOverTemp01, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eDMDOverTemp01\r\n", DMDOverTemp01);
                break;

            case 25:
                palLANProcSendLog((UINT32) WheelStop01, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eWheelStop01\r\n", WheelStop01);
                break;

            case 26:
                palLANProcSendLog((UINT32) WheelStop02, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eWheelStop02\r\n", WheelStop02);
                break;

            case 27:
                palLANProcSendLog((UINT32) LANFailthenrestart, uiSystime_Minutes);
                LOG_MSG(db_ALWAYS, "Status Error Code: %x, eLANFailthenrestart\r\n", LANFailthenrestart);
                break;

            default:
                LOG_MSG(db_ALWAYS, "Status Error Code, No Such Item:%d !!\r\n", sCmdFormat->lData);
                return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}
#endif

eCLI_ERROR_CODE utilOptoma_ProService_TemperatureStatus(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0113 //A35G2_Coda_0124
{
    UINT16 DataCode = edcPROSERVICE_TEMPERATURE_STATUS;
    UINT8  ucStatus = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        eResult = palDataMgr_Data_Access(DataCode, edaREAD, &ucStatus);
        sCmdFormat->lData = ucStatus + 1;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ProService_FanStatus(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0113 //A35G2_Coda_0124
{
    UINT16 DataCode = edcPROSERVICE_FAN_STATUS;
    UINT8  ucStatus = 0;
    eEXEC_CODE eResult = eEXEC_CODE_PASS;

    if(eAccessMode == ecmRead)
    {
        eResult = palDataMgr_Data_Access(DataCode, edaREAD, &ucStatus);
        sCmdFormat->lData = ucStatus + 1;
    }
    else if(eAccessMode == ecmWrite)
    {
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_OSD_Lock(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //A65_OPTOMA_Julie_0125 //A35G2_Coda_0127
{
    UINT8 ucOSD_Lock_Status;
    UINT16 DataCode = edcOSD_LOCK;

    if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))  //available check
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }

    if(eAccessMode == ecmRead)
    {
        if(palDataMgr_Data_Access(DataCode, edaREAD, &ucOSD_Lock_Status) != eEXEC_CODE_PASS)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
        sCmdFormat->lData= (INT32)ucOSD_Lock_Status;
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT8 ucPWord[5] = {0};
        UINT8 ucPWord_Input[5] = {0};
        UINT8 ucVAl;
        UINT8 ucStringLen = 0;

        ucStringLen = strlen((char*)sCmdFormat->cTextString);
        //LOG_MSG(db_ALWAYS, " @@String = %s (%d,%d)\r\n", sCmdFormat->cTextString,sCmdFormat->ucDataType,ucStringLen);

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_STRING)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(ucStringLen != 7)    //0 ~1234 ,7bytes
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[0] != '1') && (sCmdFormat->cTextString[0] != '2'))  //data value, 1=on 2=off
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if((sCmdFormat->cTextString[1] != ' ') || (sCmdFormat->cTextString[2] != '~'))  //check command format
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(palDataMgr_Data_Access(edcCHANGE_PIN, edaREAD, ucPWord) != eEXEC_CODE_PASS)  //get password
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        //LOG_MSG(db_ALWAYS, " $$String = %s (%s)\r\n", sCmdFormat->cTextString, ucPWord);

        ucVAl = 0;

        for(; ucVAl < 4 ; ucVAl++)  //password 4 bytes
        {
            if((sCmdFormat->cTextString[ucVAl+3] >= '0') &&
                (sCmdFormat->cTextString[ucVAl+3] <= '9'))
            {
                ucPWord_Input[ucVAl] = sCmdFormat->cTextString[ucVAl+3] ;
                if(ucPWord_Input[ucVAl] != ucPWord[ucVAl])
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        }

        if(sCmdFormat->cTextString[0] == '2')
        {
            ucOSD_Lock_Status = eOSD_NOT_LOCKED;
        }
        else
        {
            ucOSD_Lock_Status = eOSD_LOCKED;
        }

        //LOG_MSG(db_ALWAYS, " ##String = %s (%d)\r\n", sCmdFormat->cTextString, ucOSD_Lock_Status);
        palDataMgr_Data_Access(DataCode, edaWRITE_THROUGH_WITH_ACTION, &ucOSD_Lock_Status) ;
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ColorGamut(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    if(eAccessMode == ecmRead)
    {
    #if 0 //TBD
        if(datapath_GetCurrentState() == DP_STATE_MONITORING)
        {
            uint08 IsHDR = coreApp_GetHDR_Detect();
            sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
            if(IsHDR)
            {
                sprintf(sCmdFormat->cTextString, "BT.2020");
            }
            else
            {
                sprintf(sCmdFormat->cTextString, "REC709");
            }
        }
    #endif
        sprintf(sCmdFormat->cTextString, "TBD");
	}
	else if(eAccessMode == ecmWrite)
	{
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
	}
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_OMS(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //H30K_David_0029
{

	eEXEC_CODE eResult = eEXEC_CODE_PASS;
	INT32 ucProServiceBindStatus = ePROSERVICE_STATUS_IDLE;

    if(eAccessMode == ecmRead)
    {
		eResult = palDataMgr_Data_Access(edcLD_HOURS, edaREAD, &ucProServiceBindStatus);

        if(ucProServiceBindStatus == ePROSERVICE_STATUS_BINDING)
        {
            sCmdFormat->lData = 1;
        }
        else
        {
            sCmdFormat->lData = 0;
        }
	}
	else if(eAccessMode == ecmWrite)
	{
        return eCLI_ERROR_CODE_COMMAND_READ_ONLY;
	}
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Picture_Mode(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)//HICC2_Julie_0003
{
    UINT16 DataCode = edcPICTURE_SETTINGS;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
            sCmdFormat->lData = Value;
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        UINT32 ucPictureModeAvailable = 0;
        INT32 iValue = 0;

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        iValue = CLI2CM(DataCode, sCmdFormat->lData);

        palDataItemAvailable_AccessRead(eDI_PICTURE_SETTINGS, (void*)&ucPictureModeAvailable);

        if(((ucPictureModeAvailable >> iValue) & 0x01) != 1)
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, sCmdFormat->lData))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_ART_NET_Channel(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat) //H30K_Julie_0005
{
    UINT16 DataCode = edcINVALID;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        UINT16 ulSubCmd = sCmdFormat->lData;//sCmdFormat->uiFuncID;
        UINT16 ulChannel = edcARTNET_CHANNEL_SETTINGS;
        UINT8 ucUser = 0;

        if(eEXEC_CODE_PASS != palDataMgr_Data_Access((eDATA_CODE)ulChannel, edaREAD, &ucUser))
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }

        if(ulSubCmd == 5)//Channel Settings
        {
            sCmdFormat->lData = ucUser + 1;
        }
        else if(ulSubCmd >= 11 && ulSubCmd <= 26)//Edit Channel
        {
            //n01a(eg.Ok10101=User 1 Channel 01, 01 Artet)
            //n16a(eg.Ok21607=User 2 Channel 16, 07 Zoom)
            if(ucUser == eCM_ARTNET_CHANNEL_SETTINGS_USER1)
            {
                DataCode = edcARTNET_USER1_CHANNEL_1 + ulSubCmd - 11;
            }
            else if(ucUser == eCM_ARTNET_CHANNEL_SETTINGS_USER2)
            {
                DataCode = edcARTNET_USER2_CHANNEL_1 + ulSubCmd - 11;
            }
            else
            {
                return eCLI_ERROR_CODE_REQUESTFAIL;
            }

            if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
            {
                sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
                sprintf(sCmdFormat->cTextString, "%d%02d%02d", (ucUser+1), (ulSubCmd-10), Value);
            }
            else
            {
                return eCLI_ERROR_CODE_REQUESTFAIL;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_REQUESTFAIL;
        }
    }
    else if(eAccessMode == ecmWrite)
    {
        INT32  ulData = sCmdFormat->lData;
        UINT16  ucUser = 0, ucChannel = 0, ucValue = 0;
        UINT8  ucStringLen = 0;
        char   aucString[16] = {'\0'}, acStr1[8] = {'\0'}, acStr2[8] = {'\0'}, acStr3[8] = {'\0'};

        if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
        {
            return eCLI_ERROR_CODE_DataMustToBeDecimal;
        }

        if(ulData == 1 || ulData == 2)
        {
            DataCode = edcARTNET_CHANNEL_SETTINGS;
            ucValue = ulData - 1;
        }
        else if(ulData == 11)
        {
            DataCode = edcARTNET_USER1_RESET;
            ucValue = 1;
        }
        else if(ulData == 12)
        {
            DataCode = edcARTNET_USER2_RESET;
            ucValue = 1;
        }
        else
        {
            //101a~116a, a=0~16
            //201a~216a, a=0~16
            sprintf(aucString, "%d", ulData);
            ucStringLen = strlen((char*)aucString);

            if(ucStringLen == 4)//1010~1019...1160~1169, 2010~2019...2160~2169,
            {
                strncpy((char*)&acStr1, (char*)&aucString[0], 1);
                strncpy((char*)&acStr2, (char*)&aucString[1], 2);
                strncpy((char*)&acStr3, (char*)&aucString[3], 1);
            }
            else if(ucStringLen == 5)//10110~10116...11610~11616, 20110~20116...21610~21616,
            {
                strncpy(acStr1, &aucString[0], 1);
                strncpy(acStr2, &aucString[1], 2);
                strncpy(acStr3, &aucString[3], 2);
            }
            else
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }

            ucUser = (UINT16)atol(acStr1);
            ucChannel = (UINT16)atol(acStr2);
            ucValue = (UINT16)atol(acStr3);

            if((ucValue > 16) || (ucChannel == 0 || ucChannel > 16) || (ucUser != 1 &&  ucUser != 2))
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
            else
            {
                if(ucUser == 1)
                {
                    DataCode = edcARTNET_USER1_CHANNEL_1 + ucChannel - 1;
                }
                else if(ucUser == 2)
                {
                    DataCode = edcARTNET_USER2_CHANNEL_1 + ucChannel - 1;
                }
            }
        }

        if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
        {
            return eCLI_ERROR_CODE_DATAERROR;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }

        if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, ucValue))
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }
    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_IPv6_DHCP(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT16 DataCode = edcIPV6_DHCP;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        return eCLI_ERROR_CODE_COMMAND_WRITE_ONLY;
	}
	else if(eAccessMode == ecmWrite)
	{
		if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
		{
			return eCLI_ERROR_CODE_DataMustToBeDecimal;
		}

		if(sCmdFormat->lData == 3 || sCmdFormat->lData == 4)
		{
			Value = sCmdFormat->lData - 3;
		}
		else
		{
			return eCLI_ERROR_CODE_DATAOVERRANGE;
		}

		if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
		{
			return eCLI_ERROR_CODE_DATAERROR;
		}

		if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, Value))
		{
			return eCLI_ERROR_CODE_DATAOVERRANGE;
		}

		if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}
	}
	return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilOptoma_Native_Resolution(eCLI_MODE eAccessMode, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT16 DataCode = edcPROJ_NATIVE_TIMING;
    INT32 Value = 0;

    if(eAccessMode == ecmRead)
    {
        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(DataCode, &Value))
        {
        	const char cNativeResolution[4][10]=
    		{
				"1920x1080",
				"1920x1200",
				"3840x2160",
				"3840x2400"
			};

			sCmdFormat->ucDataType = eCLI_DATA_TYPE_OUTPUT_STRING;
			sprintf(sCmdFormat->cTextString, "%s", cNativeResolution[Value]);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
	}
	else if(eAccessMode == ecmWrite)
	{
		if(sCmdFormat->ucDataType != eCLI_DATA_TYPE_DEC)
		{
			return eCLI_ERROR_CODE_DataMustToBeDecimal;
		}

		if((sCmdFormat->lData >= 1) && (sCmdFormat->lData <= 4))
		{
			Value = sCmdFormat->lData - 1;
		}
		else
		{
			return eCLI_ERROR_CODE_DATAOVERRANGE;
		}

		if(eFUNC_CONTROL_ENABLE != utilCommonCLI_DataControl(DataCode))
		{
			return eCLI_ERROR_CODE_DATAERROR;
		}

		if(eEXEC_CODE_PASS != utilCommonCLI_RangeCheck(DataCode, Value))
		{
			return eCLI_ERROR_CODE_DATAOVERRANGE;
		}

		if(eEXEC_CODE_PASS != utilCommonCLI_DataConversionSet(DataCode, Value))
		{
			return eCLI_ERROR_CODE_SETFAIL;
		}
	}
	return eCLI_ERROR_CODE_NO;
}


// ==============================================================================
// FUNCTION NAME: m_sOptomaLut
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
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
//A35G2_Coda_0045
const sCLI_OPTOMA_LUT m_sOptomaLut[] =
{
    //cMainCmd uiMainCmdID ucMainCmdLenght cSubCmd uiSubCmdID ucCmdIsRead cCmdBytesOnRange uiFunCodeID;

    //Aspect Ratio
    {"127", 127,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_AspectRatio,           CLI_COM_NORMAL},      //ZU860_Doulas_0025
    {"60",  60,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                 utilOptoma_AspectRatio,           CLI_COM_NORMAL},      //ZU860_Doulas_0025

    //Horz Position
    {"75",  75,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHORZ_POSITION,            NULL,           CLI_COM_NORMAL},
    {"63",  63,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHORZ_POSITION,            NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076

    //Vert Position
    {"76",  76,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcVERT_POSITION,            NULL,           CLI_COM_NORMAL},
    {"64",  64,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcVERT_POSITION,            NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076

	//image shift reset
	{"172", 172,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                  utilOptoma_Image_Shift_Reset,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076

    //Proportional
    {"543", 543,    3,  "11",  11,  READ_COMMAND,     DECIAML_3,        edcDIGITAL_ZOOM_PROPORTIONAL,        NULL,           CLI_COM_NORMAL},  //HICC2_Sammy_0002

    //H Digital Zoom
    {"504", 504,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_HORZ_ZOOM,        NULL,           CLI_COM_NORMAL},
    {"543", 543,    3,  "8",  8,  READ_COMMAND,     DECIAML_3,          edcDIGITAL_HORZ_ZOOM,        NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025

    //V Digital Zoom
    {"505", 505,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_VERT_ZOOM,        NULL,           CLI_COM_NORMAL},
    {"543", 543,    3,  "7",  7,  READ_COMMAND,     DECIAML_3,          edcDIGITAL_VERT_ZOOM,        NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025

    //H Digital Shift
    {"63",  63,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_HORZ_SHIFT,       NULL,           CLI_COM_NORMAL},
    {"543", 543,    3,  "1",  1,  READ_COMMAND,     DECIAML_3,          edcDIGITAL_HORZ_SHIFT,       NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025

    //V Digital Shift
    {"64",  64,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcDIGITAL_VERT_SHIFT,       NULL,           CLI_COM_NORMAL},
    {"543", 543,    3,  "2",  2,  READ_COMMAND,     DECIAML_3,          edcDIGITAL_VERT_SHIFT,       NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025

    //Keystone
    {"65",  65,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcHORIZONTAL_KEYSTONE,      NULL,           CLI_COM_NORMAL},
    {"66",  66,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcVERTICAL_KEYSTONE,        NULL,           CLI_COM_NORMAL},
    {"543", 543,    3,  "4",  4,  READ_COMMAND,     DECIAML_3,          edcHORIZONTAL_KEYSTONE,      NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025
    {"543", 543,    3,  "3",  3,  READ_COMMAND,     DECIAML_3,          edcVERTICAL_KEYSTONE,        NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025

    //Pincushion
    {"300", 300,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHORZ_PINCUSHION,          NULL,           CLI_COM_NORMAL},
    {"301", 301,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcVERT_PINCUSHION,          NULL,           CLI_COM_NORMAL},
    {"543", 543,    3,  "6",  6,  READ_COMMAND,     DECIAML_3,          edcHORZ_PINCUSHION,          NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025
    {"543", 543,    3,  "5",  5,  READ_COMMAND,     DECIAML_3,          edcVERT_PINCUSHION,          NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0025

    //4-Corner
    {"58",  58,     2,  "1",  1,  READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_LEFT_HORZ,     NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "2",  2,  READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_LEFT_VERT,     NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "3",  3,  READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_RIGHT_HORZ,    NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "4",  4,  READ_COMMAND,     DECIAML_3,          edc4CORNER_TOP_RIGHT_VERT,    NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "5",  5,  READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_LEFT_HORZ,  NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "6",  6,  READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_LEFT_VERT,  NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "7",  7,  READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_HORZ, NULL,          CLI_COM_NORMAL},
    {"58",  58,     2,  "8",  8,  READ_COMMAND,     DECIAML_3,          edc4CORNER_BOTTOM_RIGHT_VERT, NULL,          CLI_COM_NORMAL},
    {"59",  59,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                  utilOptoma_FourCorners,           CLI_COM_NORMAL},   //ZU860_Doulas_0026

  //Advanced Warp
	//Grid Color
	{"143", 143,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,     utilOptoma_Grid_Color,       CLI_COM_NORMAL},	//H30K_David_0027
    {"379", 379,    3,  "1",  1,  READ_COMMAND,     DECIAML_3,          edcINVALID,     utilOptoma_Grid_Color,       CLI_COM_NORMAL},   //H30K_David_0027
	//Grid Background
	{"145", 145,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,     utilOptoma_Grid_Background,  CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076
	//Grid Points
	{"144", 144,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,     utilOptoma_Grid_Points,      CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076
	//Warp inner
	{"146", 146,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcADV_WARP_INNER,     		 NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076
	//Warp sharpness
	{"148", 148,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcADV_WARP_SHARPNESS,     	 NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076
	//Overlap Grid Number
	{"169", 169,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID, utilOptoma_Overlap_Grid_Number,  CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076
	//Blend Gamma
	{"170", 170,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,     utilOptoma_Blend_Gamma,      CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076


    //Brightness
    {"125", 125,    3,  "1",  1,  READ_COMMAND,     DECIAML_3,          edcINVALID,                  utilOptoma_Brightness,           CLI_COM_NORMAL},//H30K_David_0041
    {"21",  21,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                  utilOptoma_Brightness,           CLI_COM_NORMAL},//H30K_David_0041
    {"46",  46,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                  utilOptoma_Brightness_AD,           CLI_COM_WRITE_ONLY},   //ZU860_Doulas_0063

    //Contrast
    {"126", 126,    3,  "1",  1,  READ_COMMAND,     DECIAML_3,          edcINVALID,                 utilOptoma_Contrast,           CLI_COM_NORMAL},//H30K_David_0041
    {"22",  22,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                 utilOptoma_Contrast,           CLI_COM_NORMAL},//H30K_David_0041
    {"47",  47,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                  utilOptoma_Contrast_AD,           CLI_COM_WRITE_ONLY},   //ZU860_Doulas_0063

    //Color Space
    {"37",  37,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_ColorSpace,             CLI_COM_NORMAL},  //ZU860_Doulas_0023
    {"295", 295,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_ColorSpace,             CLI_COM_NORMAL},

    //Source info. Main Color Space
    {"295", 295,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_Main_ColorSpace,         CLI_COM_READ_ONLY}, //HICC2_Julie_0004

    //Source info. Sub Color Space
    {"295", 295,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_Sub_ColorSpace,          CLI_COM_READ_ONLY}, //HICC2_Julie_0004

    //3D Format
    {"405", 405,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_3D_Format,           CLI_COM_NORMAL},   //ZU860_Doulas_0050 modify

    //3D INVERT
    {"231", 231,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_3D_Invert,           CLI_COM_NORMAL},//H30K_David_0046

    //3D Sync Out
    {"232", 232,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edc3D_SYNC_OUT,              NULL,           CLI_COM_NORMAL},

    //Frame Delay
    {"233", 233,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcFRAME_DELAY,              NULL,           CLI_COM_NORMAL},

    //RGB Gain/Bias
    {"24",  24,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcRED_GAIN,                 NULL,           CLI_COM_NORMAL},
    {"25",  25,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcGREEN_GAIN,               NULL,           CLI_COM_NORMAL},
    {"26",  26,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcBLUE_GAIN,                NULL,           CLI_COM_NORMAL},
    {"27",  27,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcRED_OFFSET,               NULL,           CLI_COM_NORMAL},
    {"28",  28,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcGREEN_OFFSET,             NULL,           CLI_COM_NORMAL},
    {"29",  29,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcBLUE_OFFSET,              NULL,           CLI_COM_NORMAL},

    {"498", 498,    3,  "1",  1,  READ_COMMAND,     DECIAML_3,          edcRED_GAIN,                 NULL,           CLI_COM_NORMAL},
    {"498", 498,    3,  "2",  2,  READ_COMMAND,     DECIAML_3,          edcGREEN_GAIN,               NULL,           CLI_COM_NORMAL},
    {"498", 498,    3,  "3",  3,  READ_COMMAND,     DECIAML_3,          edcBLUE_GAIN,                NULL,           CLI_COM_NORMAL},
    {"499", 499,    3,  "1",  1,  READ_COMMAND,     DECIAML_3,          edcRED_OFFSET,               NULL,           CLI_COM_NORMAL},
    {"499", 499,    3,  "2",  2,  READ_COMMAND,     DECIAML_3,          edcGREEN_OFFSET,             NULL,           CLI_COM_NORMAL},
    {"499", 499,    3,  "3",  3,  READ_COMMAND,     DECIAML_3,          edcBLUE_OFFSET,              NULL,           CLI_COM_NORMAL},

    //Reset RGB Gain/Offset
    {"517", 517,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcRESET_RGB_GAIN_OFFSET,    NULL,           CLI_COM_WRITE_ONLY},

    //Display Mode -Apply to User
    {"123", 123,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcPICTURE_SETTINGS,         NULL,           CLI_COM_NORMAL}, //ZU860_Doulas_0022
    {"20",  20,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_Picture_Mode,           CLI_COM_NORMAL}, //ZU860_Doulas_0022 //H30K_David_0034

	//Display Mode -Save to User
    {"424",  424,   3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSAVE_TO_USER,             NULL,      	 CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0095 //A35G2_Coda_0061

    //DynamicBlack
    {"191", 191,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDYNAMIC_BLACK,            NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0023
    {"271", 271,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcDYNAMIC_BLACK,            NULL,           CLI_COM_NORMAL},

	//DynamicBlack Speed
    {"253", 253,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDB_SPEED,            	 NULL,           CLI_COM_NORMAL},		//A65_OPTOMA_Doulas_0074

	//DynamicBlack Strength
    {"254", 254,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDB_STRENGTH,            	 NULL,           CLI_COM_NORMAL},		//A65_OPTOMA_Doulas_0074

	//DynamicBlack Level
    {"255", 255,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDB_LIGHT_LEVEL,           NULL,           CLI_COM_NORMAL},		//A65_OPTOMA_Doulas_0074

    //Freeze Screen
    {"04",  4,      2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcIMAGE_FREEZE,             NULL,           CLI_COM_NORMAL},
    {"377", 377,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcIMAGE_FREEZE,             NULL,           CLI_COM_NORMAL},

    //Gamma
    //{"35",  35,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcGAMMA,                     NULL,          CLI_COM_NORMAL},   //H30K_David_0022
    {"35",  35,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_Gamma,           CLI_COM_NORMAL},   //ZU860_Doulas_0022 ///H30K_David_0022
    //CCI
    {"128", 128,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_ColorTemperature,           CLI_COM_NORMAL},    //ZU860_Doulas_0022
    {"36",  36,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_ColorTemperature,           CLI_COM_NORMAL},    //ZU860_Doulas_0022

    //Color Wheel Speed
    {"547", 547,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_ColorWheelSpeed,           CLI_COM_NORMAL},     //ZU860_Doulas_0023
    {"559", 559,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_ColorWheelSpeed,           CLI_COM_NORMAL},     //ZU860_Doulas_0063 modify//ZU860_Doulas_0050

    //Language
    {"70",   70,    2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_Language,           CLI_COM_NORMAL},            //ZU860_Owen_0009 mod
    {"299", 299,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLANGUAGE,                  NULL,           CLI_COM_NORMAL},

    //Focus
    {"308", 308,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Focus,          CLI_COM_LENS},    //ZU860_Doulas_0028

    //ZOOM
    {"307", 307,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Zoom,          CLI_COM_LENS},     //ZU860_Doulas_0028

    //Lens Shift//LVO
    {"84",  84,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_LensShift,          CLI_COM_LENS},    //ZU860_Doulas_0028

    //Lens Memory Apply
    {"359", 359,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_LensApplyPosition,          CLI_COM_NORMAL},   //ZU860_Doulas_0030
    {"384", 384,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLENS_APPLY_POSITION,                   NULL,          CLI_COM_NORMAL},

    //Lens Memory Save
    {"360", 360,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_LensSaveCurrentPosition,          CLI_COM_NORMAL},   //ZU860_Doulas_0030

    //Lens Calibration
    {"525", 525,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_LensCalibration,           CLI_COM_WRITE_ONLY},  //ZU860_Doulas_0048 modify

    //Lens Function
    {"349", 349,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_LensLock,           CLI_COM_NORMAL},   //ZU860_Doulas_0028
    {"545", 545,    3,  "4",  4,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_LensLock,           CLI_COM_NORMAL},   //ZU860_Doulas_0065 modify//ZU860_Doulas_0028

    //Ceiling Mount
    {"523", 523,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcCEILING_MOUNT,             NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0037
    {"370", 370,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcCEILING_MOUNT,             NULL,           CLI_COM_NORMAL}, //R70PD_Steven_0001, 3DE ISS-0018069

    //Rear Projection
    {"524", 524,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcREAR_PROJECTION,           NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0037
    {"371", 371,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcREAR_PROJECTION,           NULL,           CLI_COM_NORMAL},  //R70PD_Steven_0001, 3DE ISS-0018069

    //Menu Transparency
    {"526", 526,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,         			utilOptoma_MenuTransparency,           CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0037
    {"382", 382,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,             	  	utilOptoma_MenuTransparency,          CLI_COM_NORMAL }, //HICC2_Sammy_0002

	//Lens Reset
    {"175", 175,    3,  "1",  1,  WRITE_COMMAND,    DECIAML_2,          edcLENS_RESET,         		  NULL,           CLI_COM_WRITE_ONLY},	//A65_OPTOMA_Julie_0037

    //Power on
    {"124", 124,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,       utilOptoma_PWR,          CLI_COM_NORMAL},     //ZU860_Doulas_0034
    {"00",  0,      2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,       utilOptoma_PWR,          CLI_COM_NORMAL},     //ZU860_Doulas_0034

    //Signal Power On
    {"113", 113,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSIGNAL_POWER_ON,           NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0029
    {"385", 385,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcSIGNAL_POWER_ON,           NULL,          CLI_COM_NORMAL},

    //Standby Power Mode
    {"114", 114,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSTANDBY_MODE,              NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0030 //H30K_David_0031
    {"150", 150,    3,  "16", 16,  READ_COMMAND,    DECIAML_2,          edcSTANDBY_MODE,              NULL,          CLI_COM_NORMAL},

    //Direct Power On
    {"105", 105,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcAC_POWER_ON,               NULL,          CLI_COM_NORMAL},
    {"385", 385,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcAC_POWER_ON,               NULL,          CLI_COM_NORMAL},

    //Auto Power Off
    {"106", 106,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_AutoPowerOff,          CLI_COM_NORMAL},   //ZU860_Doulas_0029
    {"387", 387,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcAUTO_SHUTDOWN,             NULL,          CLI_COM_NORMAL},

    //Sleep Timer
    {"107", 107,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_SleepTimer,          CLI_COM_NORMAL},   //ZU860_Doulas_0029
    {"388", 388,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_SleepTimer,          CLI_COM_NORMAL},

    //High Altitude
    {"101", 101,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcHIGH_ALTITUDE,             NULL,          CLI_COM_NORMAL},
    {"150", 150,    3,  "22", 22, READ_COMMAND,     DECIAML_2,          edcHIGH_ALTITUDE,             NULL,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0037

	//Power Setting Reset
    {"177", 177,    3,  "1",  1,  WRITE_COMMAND,    DECIAML_2,          edcPOWER_SETTINGS_RESET,      NULL,          CLI_COM_WRITE_ONLY},	//A65_OPTOMA_Julie_0039

#if 0
    //IR Control//IRC
    {eCLI_MAIN_IRC,     "TOPP",     READ_COMMAND,     DECIAML_2,          edcTOP_IR,                    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_IRC,     "TOPP",     WRITE_COMMAND,    DECIAML_2,          edcTOP_IR,                    NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_IRC,     "FRNT",     READ_COMMAND,     DECIAML_2,          edcFRONT_IR,                  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_IRC,     "FRNT",     WRITE_COMMAND,    DECIAML_2,          edcFRONT_IR,                  NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_IRC,     "HDBT",     READ_COMMAND,     DECIAML_2,          edcHDBASET_IR,                NULL,          CLI_COM_NORMAL},
    {eCLI_MAIN_IRC,     "HDBT",     WRITE_COMMAND,    DECIAML_2,          edcHDBASET_IR,                NULL,          CLI_COM_NORMAL},
#endif /* 0 */

    //Projector ID
    {"79",  79,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_ID,              NULL,          CLI_COM_NORMAL},       //ZU860_Doulas_0032
    {"558", 558,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcPROJECTOR_ID,              NULL,          CLI_COM_NORMAL},       //ZU860_Doulas_0032

    //Color Matching
    {"410", 410,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcHSG_ENABLE,                NULL,          CLI_COM_NORMAL},
    {"411", 411,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_AUTO_TEST_PATTERN,     NULL,          CLI_COM_NORMAL},

    {"333", 333,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_SATURATION,        NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0023 modify
    {"327", 327,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_HUE,               NULL,          CLI_COM_NORMAL},
    {"339", 339,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_RED_GAIN,              NULL,          CLI_COM_NORMAL},
    {"334", 334,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_SATURATION,      NULL,          CLI_COM_NORMAL},
    {"328", 328,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_HUE,             NULL,          CLI_COM_NORMAL},
    {"340", 340,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_GREEN_GAIN,            NULL,          CLI_COM_NORMAL},
    {"335", 335,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_SATURATION,       NULL,          CLI_COM_NORMAL},
    {"329", 329,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_HUE,              NULL,          CLI_COM_NORMAL},
    {"341", 341,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_BLUE_GAIN,             NULL,          CLI_COM_NORMAL},
    {"336", 336,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_SATURATION,       NULL,          CLI_COM_NORMAL},
    {"330", 330,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_HUE,              NULL,          CLI_COM_NORMAL},
    {"342", 342,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_CYAN_GAIN,             NULL,          CLI_COM_NORMAL},
    {"338", 338,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_SATURATION,    NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0114
    {"332", 332,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_HUE,           NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0114
    {"344", 344,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_MAGENTA_GAIN,          NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0114
    {"337", 337,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_SATURATION,     NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0114
    {"331", 331,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_HUE,            NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0114
    {"343", 343,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_YELLOW_GAIN,           NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0114
    {"345", 345,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_RED,        NULL,          CLI_COM_NORMAL},
    {"346", 346,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_GREEN,      NULL,          CLI_COM_NORMAL},
    {"347", 347,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcHSG_WHITE_GAIN_BLUE,       NULL,          CLI_COM_NORMAL},

    {"491", 491,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_RED_SATURATION,        NULL,          CLI_COM_READ_ONLY},
    {"491", 491,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_RED_HUE,               NULL,          CLI_COM_READ_ONLY},
    {"491", 491,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_RED_GAIN,              NULL,          CLI_COM_READ_ONLY},
    {"492", 492,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_SATURATION,      NULL,          CLI_COM_READ_ONLY},
    {"492", 492,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_HUE,             NULL,          CLI_COM_READ_ONLY},
    {"492", 492,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_GREEN_GAIN,            NULL,          CLI_COM_READ_ONLY},
    {"493", 493,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_SATURATION,       NULL,          CLI_COM_READ_ONLY},
    {"493", 493,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_HUE,              NULL,          CLI_COM_READ_ONLY},
    {"493", 493,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_BLUE_GAIN,             NULL,          CLI_COM_READ_ONLY},
    {"494", 494,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_SATURATION,       NULL,          CLI_COM_READ_ONLY},
    {"494", 494,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_HUE,              NULL,          CLI_COM_READ_ONLY},
    {"494", 494,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_CYAN_GAIN,             NULL,          CLI_COM_READ_ONLY},
    {"495", 495,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_SATURATION,    NULL,          CLI_COM_READ_ONLY},
    {"495", 495,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_HUE,           NULL,          CLI_COM_READ_ONLY},
    {"495", 495,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_MAGENTA_GAIN,          NULL,          CLI_COM_READ_ONLY},
    {"496", 496,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_SATURATION,     NULL,          CLI_COM_READ_ONLY},
    {"496", 496,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_HUE,            NULL,          CLI_COM_READ_ONLY},
    {"496", 496,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_YELLOW_GAIN,           NULL,          CLI_COM_READ_ONLY},
    {"497", 497,    3,  "1", 1,   READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_RED,        NULL,          CLI_COM_READ_ONLY},
    {"497", 497,    3,  "2", 2,   READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_GREEN,      NULL,          CLI_COM_READ_ONLY},
    {"497", 497,    3,  "3", 3,   READ_COMMAND,     DECIAML_3,          edcHSG_WHITE_GAIN_BLUE,       NULL,          CLI_COM_READ_ONLY},

    //Color Matching Reset to Default
    {"215", 215,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HSG_Reset,           CLI_COM_WRITE_ONLY},	//A65_OPTOMA_Doulas_0074 Modify

    //Information
    //Model Name
    {"151", 151,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcMODEL_NAME,                NULL, /*utilOptoma_ModelName,*/ CLI_COM_READ_ONLY},    //A65_OPTOMA_Julie_0036 //ZU860_Doulas_0065 modify
     //Model   //A35G2_Coda_0127
    {"151", 151,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_ModelName,          CLI_COM_READ_ONLY},    //ZU860_Doulas_0065 modify
    //Serial Number
    {"353", 353,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSERIAL_NUMBER,             NULL,          CLI_COM_NORMAL},   //A70LV_Doulas_0227 add //A70LV_Doulas_0125 remove
    //Native Resolution
    {"150", 150,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcNATIVE_RESOLUTION,         NULL,          CLI_COM_NORMAL},
    //Firmware//PIF
    {"122", 122,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_AllVersion,          CLI_COM_READ_ONLY},    //ZU860_Doulas_0043
    //Main Source
    {"150", 150,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_SIN_Name,          CLI_COM_NORMAL},     //ZU860_Doulas_0114 modify
    //Resolution
    {"150", 150,    3,  "4",  4,  READ_COMMAND,     DECIAML_2,          edcMAIN_RESOLUTION,              NULL,          CLI_COM_READ_ONLY},
    //Signal Format
    {"150", 150,    3,  "5",  5,  READ_COMMAND,     DECIAML_2,          edcMAIN_SIGNAL_FORMAT,           NULL,          CLI_COM_READ_ONLY},
    //Pixel Clock
    {"150", 150,    3,  "6",  6,  READ_COMMAND,     DECIAML_2,          edcMAIN_PIXEL_CLOCK,             NULL,          CLI_COM_READ_ONLY},
    //Horz Refresh
    {"150", 150,    3,  "7",  7,  READ_COMMAND,     DECIAML_2,          edcMAIN_HORZ_REFRESH,            NULL,          CLI_COM_READ_ONLY},
    //Vert Refresh
    {"150", 150,    3,  "8",  8,  READ_COMMAND,     DECIAML_2,          edcMAIN_VERT_REFRESH,            NULL,          CLI_COM_READ_ONLY}, //H30K_Tim_0004, add
    //Refresh
    {"150", 150,    3, "19", 19,  READ_COMMAND,     DECIAML_2,          edcMAIN_VERT_REFRESH,            NULL,          CLI_COM_READ_ONLY}, //HICC2_Julie_0004
    //Sub Source
    {"150", 150,    3,  "9",  9,  READ_COMMAND,     DECIAML_2,          edcINVALID,                      utilOptoma_SIN_PIIP_Name,          CLI_COM_NORMAL},     //ZU860_Doulas_0114 modify//ZU860_Doulas_0034
    {"121", 121,    3,  "9",  9,  READ_COMMAND,     DECIAML_2,          edcINVALID,                      utilOptoma_SIN_PIIP,          CLI_COM_NORMAL},     //ZU860_Doulas_0034

    //Resolution
    {"150", 150,    3,  "10", 10, READ_COMMAND,     DECIAML_2,          edcSUB_RESOLUTION,               NULL,          CLI_COM_READ_ONLY},
    //Signal Format
    {"150", 150,    3,  "11", 11, READ_COMMAND,     DECIAML_2,          edcSUB_SIGNAL_FORMAT,            NULL,          CLI_COM_READ_ONLY},
    //Pixel Clock
    {"150", 150,    3,  "12", 12, READ_COMMAND,     DECIAML_2,          edcSUB_PIXEL_CLOCK,              NULL,          CLI_COM_READ_ONLY},
    //Horz Refresh
    {"150", 150,    3,  "13", 13, READ_COMMAND,     DECIAML_2,          edcSUB_HORZ_REFRESH,             NULL,          CLI_COM_READ_ONLY},
    //Vert Refresh
    {"150", 150,    3,  "14", 14, READ_COMMAND,     DECIAML_2,          edcSUB_VERT_REFRESH,             NULL,          CLI_COM_READ_ONLY}, //H30K_Tim_0004, add
    //Refresh
    {"150", 150,    3,  "24", 24, READ_COMMAND,     DECIAML_2,          edcSUB_VERT_REFRESH,             NULL,          CLI_COM_READ_ONLY}, //HICC2_Julie_0004
    //Light Source Mode
    {"150", 150,    3,  "15", 15, READ_COMMAND,     DECIAML_2,          edcINVALID,                      utilOptoma_BrightnessMode,          CLI_COM_NORMAL},  //ZU860_Doulas_0023
    //Refresh Rate
    {"150", 150,    3,  "19", 19, READ_COMMAND,     DECIAML_2,          edcMAIN_HORZ_REFRESH,            NULL,          CLI_COM_READ_ONLY},

    //Device -> Projector Hours
    {"150", 150,    3,  "21", 21,  READ_COMMAND,    DECIAML_5,          edcLAMP_PERIOD_INFO,          NULL,          		CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A65_OPTOMA_Julie_0036

    //Light Source Hours -> Total Hours
    {"108", 108,    3,  "1",  1,  READ_COMMAND,     DECIAML_5,          edcLIGHT_SOURCE_HOURS_INFO,         NULL,  CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A65_OPTOMA_Julie_0036
	//Light Source Hours -> Normal
    {"108", 108,    3,  "3",  3,  READ_COMMAND,     DECIAML_5,          edcLIGHT_SOURCE_HOURS_NORAML_INFO,  NULL,  CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A65_OPTOMA_Julie_0036
    //Light Source Hours -> Eco Mode
    {"108", 108,    3,  "4",  4,  READ_COMMAND,     DECIAML_5,          edcLIGHT_SOURCE_HOURS_ECO_INFO,     NULL,  CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A65_OPTOMA_Julie_0036
    //Light Source Hours -> Custom Mode Power
    {"108", 108,    3,  "7",  7,  READ_COMMAND,     DECIAML_5,          edcLIGHT_SOURCE_HOURS_CUSTOM_INFO,  NULL,  CLI_COM_NORMAL| CLI_COM_NEED_SERVICE_CODE_WRITE}, //A65_OPTOMA_Julie_0036

    //Standby Power Mode
    {"150", 150,    3,  "16", 16, READ_COMMAND,    DECIAML_2,           edcINVALID,                   utilOptoma_PowerMode,          CLI_COM_NORMAL},   //ZU860_Doulas_0030
    //DHCP
    {"150", 150,    3,  "17", 17, READ_COMMAND,     DECIAML_2,          edcLAN_DHCP,                  NULL,          CLI_COM_NORMAL},
    {"461", 461,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLAN_DHCP,      	 		  NULL,          CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0177

    //LAN STATUS
    {"87",  87,     2,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLAN_STATUS,           	  NULL,          CLI_COM_READ_ONLY}, //A65_OPTOMA_Julie_0036
    {"87",  87,     2,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcLAN_IP_ADDRESS,            NULL,          CLI_COM_NORMAL},  //ZU860_Doulas_0113 Modify
    {"87",  87,     2,  "4",  4,  READ_COMMAND,     STRING_CHAR,        edcLAN_SUBNET_MASK,           NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "5",  5,  READ_COMMAND,     STRING_CHAR,        edcLAN_DEFAULT_GATEWAY,       NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "6",  6,  READ_COMMAND,     STRING_CHAR,        edcLAN_PRIMARY_DNS,           NULL,          CLI_COM_READ_ONLY}, //R70PD_AC_0007
    {"87",  87,     2,  "7",  7,  READ_COMMAND,     STRING_CHAR,        edcLAN_SECOND_DNS,            NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "9",  9,  READ_COMMAND,     STRING_CHAR,        edcIPV6_DHCP,            	  NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "12", 12, READ_COMMAND,     STRING_CHAR,        edcIPV6_IP_ADDRESS,           NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "13", 13, READ_COMMAND,     STRING_CHAR,        edcIPV6_PREFIX_LENGTH,        NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "14", 14, READ_COMMAND,     STRING_CHAR,        edcIPV6_DEFAULT_GATEWAY,      NULL,          CLI_COM_READ_ONLY},
    {"87",  87,     2,  "15", 15, READ_COMMAND,     STRING_CHAR,        edcIPV6_DNS,            	  NULL,          CLI_COM_READ_ONLY},

    //Factory Reset
    {"112", 112,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,       utilOptoma_DEF,          CLI_COM_WRITE_ONLY | CLI_COM_NEED_SERVICE_CODE_WRITE},     //A70LV_Doulas_0138

    //Test Pattern
    {"195", 195,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcOSDTEST_PATTERN,           NULL,          CLI_COM_NORMAL},   //A65_OPTOMA_Doulas_0076 Modify//ZU860_Doulas_0031

    //Light Source Mode
    {"110", 110,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_BrightnessMode,          CLI_COM_NORMAL},  //ZU860_Doulas_0023
    {"241", 241,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_BrightnessMode,          CLI_COM_NORMAL},

    //Constant Power Settings
    {"326", 326,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL},
    {"381", 381,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL},
    {"242", 242,    3,  "0",  0,  READ_COMMAND,     DECIAML_2,          edcCONSTANT_POWER_NUMBER,     NULL,          CLI_COM_NORMAL}, //HICC2_Julie_0004

    //Custom:Constant Brightness
    {"522", 522,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcCONSTANT_BRIGHTNESS,       NULL,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0088
    {"242", 242,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcCONSTANT_BRIGHTNESS,       NULL,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0088

    //Light Sensor Calibration
    //{"552", 552,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLIGHT_SENSOR_CALIBRATION,  NULL,          CLI_COM_NORMAL},

    //Main Source
    {"121", 121,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,       utilOptoma_SIN,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {"12",  12,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,       utilOptoma_SIN,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {"408", 408,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,       utilOptoma_SIN,          CLI_COM_NORMAL},		//A65_OPTOMA_Doulas_0177

    //Sub Source
    {"131", 131,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,  utilOptoma_SIN_PIIP,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify
    {"305", 305,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,  utilOptoma_SIN_PIIP,          CLI_COM_NORMAL},     //A70LV_Doulas_0135 modify

    //PIP/PBP Function
    {"302", 302,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcPIP_PBP_ENABLE,            NULL,          CLI_COM_NORMAL},
    {"134", 134,    3,  "1",  1,  READ_COMMAND,    	DECIAML_2,        	edcPIP_PBP_ENABLE,        	  NULL,          CLI_COM_NORMAL}, //HICC2_Sammy_0002

    //Swap
    {"306", 306,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSWAP,                      NULL,          CLI_COM_WRITE_ONLY},

    //Size
    {"304", 304,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Size,          CLI_COM_NORMAL},        //ZU860_Doulas_0026
    {"134", 134,    3,  "2",  2,  READ_COMMAND,    	DECIAML_2,        	edcINVALID,        	  		  utilOptoma_Size,          CLI_COM_NORMAL}, //HICC2_Sammy_0002

    //Layout
    {"303", 303,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Layout,          CLI_COM_NORMAL},       //ZU860_Doulas_0026
    {"134", 134,    3,  "3",  3,  READ_COMMAND,    	DECIAML_2,        	edcINVALID,        	  		  utilOptoma_Layout,          CLI_COM_NORMAL}, //HICC2_Sammy_0002

    //Auto Source
    {"100", 100,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL},

    //Input KEY
    {"140", 140,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_CLI_KEY,       CLI_COM_WRITE_ONLY},   //ZU860_Doulas_0035 modify

    //Automatic
    {"91",  91,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_AutoImage,          CLI_COM_NORMAL},   //ZU860_Owen_0019

    //Phase
    {"74",  74,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcPIXEL_PHASE,               NULL,          CLI_COM_NORMAL},

    //Color
    {"45",  45,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcCOLOR,                     NULL,          CLI_COM_NORMAL},
    {"292", 292,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcCOLOR,                     NULL,          CLI_COM_NORMAL },

    //Tint
    {"44",  44,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcTINT,                      NULL,          CLI_COM_NORMAL},       //ZU860_Doulas_0022
    {"293", 293,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcTINT,                      NULL,          CLI_COM_NORMAL },

    //Shutter
    {"02",  2,      2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcPICTURE_MUTE,              NULL,          CLI_COM_NORMAL },
    {"355", 355,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcPICTURE_MUTE,              NULL,          CLI_COM_NORMAL },      //ZU860_Doulas_0034 modify

    //Information Hide
    {"102", 102,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSHOW_MESSAGES,          	  NULL,          CLI_COM_NORMAL },   //A65_OPTOMA_Julie_0037//ZU860_Doulas_0033 modify
    {"383", 383,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSHOW_MESSAGES,             NULL,          CLI_COM_NORMAL }, //R70PD_Steven_0001, 3DE ISS-0018084

    //Wall Color
    {"506", 506,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_WallColor,        CLI_COM_NORMAL },      //ZU860_Doulas_0022
    {"296", 296,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_WallColor,        CLI_COM_NORMAL },

    //BrilliantColor
    {"34",  34,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcWHITE_PEAKING,             NULL,          CLI_COM_NORMAL},       //ZU860_Doulas_0022
    {"294", 294,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcWHITE_PEAKING,             NULL,          CLI_COM_NORMAL },

    //Sharpness
    {"23",  23,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDETAIL,                    NULL,          CLI_COM_NORMAL},       //ZU860_Doulas_0022

    //Edge Mask
    {"61",  61,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcEDGE_MASK,                 NULL,          CLI_COM_NORMAL},       //ZU860_Doulas_0025
    {"378", 378,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcEDGE_MASK,                 NULL,          CLI_COM_NORMAL},  //R70PD_Steven_0001, 3DE  ISS-0018026

    //PC Connection
    {"132", 132,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,         utilOptoma_WarpControl,          CLI_COM_NORMAL },    //ZU860_Owen_0019 ////H30K_David_0021

    //Warp adn Blend memory
    {"141", 141,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,         utilOptoma_WarpSettingSave,      CLI_COM_NORMAL},      //A65_OPTOMA_Doulas_0076 Modify//ZU860_Owen_0019 //H30K_David_0021
    //{"132", 132,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,       utilOptoma_WarpSetting,          CLI_COM_NORMAL},      //A65_OPTOMA_Doulas_0076 remove  //ZU860_Owen_0019
    {"147", 147,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,         utilOptoma_WarpSettingApply,     CLI_COM_NORMAL},      //A65_OPTOMA_Doulas_0076 Modify//ZU860_Owen_0019
    {"137", 137,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,         utilOptoma_WarpSettingApply,     CLI_COM_NORMAL},      //A65_OPTOMA_Doulas_0076 Modify//ZU860_Owen_0019
    {"174", 174,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcWARP_MEMORY_CLEAR,         NULL,     		     CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076

    //Projection
    {"71",  71,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Projection,           CLI_COM_NORMAL},      //ZU860_Doulas_0027
    {"129", 129,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Projection,           CLI_COM_NORMAL},      //ZU860_Doulas_0027

    //IR Function
    {"11",  11,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_IR_Function,          CLI_COM_NORMAL},   //ZU860_Doulas_0032
    {"542", 542,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Front_IR_Get,         CLI_COM_READ_ONLY},  //ZU860_Doulas_0065
    {"542", 542,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_TOP_IR_Get,           CLI_COM_READ_ONLY},  //ZU860_Doulas_0065
    {"542", 542,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_HDBASET_IR_Get,       CLI_COM_READ_ONLY},  //A65_OPTOMA_Julie_0036 //ZU860_Doulas_0065
    {"542", 542,    3,  "4",  4,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Rear_IR_Get,          CLI_COM_READ_ONLY},

    //Remote Code
    {"350", 350,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcPROJECTOR_ADDRESS,         NULL,          CLI_COM_NORMAL },      //ZU860_Doulas_0032
    {"138", 138,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcPROJECTOR_ADDRESS,         NULL,          CLI_COM_NORMAL },      //A65_OPTOMA_Julie_0036

    //Quick Switch Code
    {"314", 314,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcQUICK_KEY,                 NULL,          CLI_COM_NORMAL },      //A65_OPTOMA_Julie_0036
    {"138", 138,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcQUICK_KEY,                 NULL,          CLI_COM_NORMAL },      //A65_OPTOMA_Julie_0036

#if !defined(PLATFORM_H30_4K)	//HICC2_Sammy_0002
    //12V Trigger
    {"192", 192,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edc12V_TRIGGER,               NULL,          CLI_COM_NORMAL },      //ZU860_Doulas_0032
#endif
    //Hot-Key settings
    {"117", 117,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HotKeySetting,          CLI_COM_NORMAL },      //ZU860_Doulas_0032
    {"394", 394,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_HotKeySetting,          CLI_COM_NORMAL },

    //Hot-Key settings for User2
    {"118", 118,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HotKeySetting_User2,          CLI_COM_NORMAL }, //A65_OPTOMA_Julie_0036
    {"394", 394,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_HotKeySetting_User2,          CLI_COM_NORMAL },

    //Menu Location
    {"72",  72,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_MenuLocation,           CLI_COM_NORMAL},  //ZU860_Doulas_0032
    {"382", 382,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,             	  utilOptoma_MenuLocation,          CLI_COM_NORMAL }, //HICC2_Sammy_0002

    //Menu Timer
    {"515", 515,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_MenuTimeOut,          CLI_COM_NORMAL },      //ZU860_Doulas_0033
    {"382", 382,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcMENU_TIME_OUT,             NULL,          CLI_COM_NORMAL }, //R70PD_Steven_0001, 3DE  ISS-0018077

    //Auto Source
    {"563", 563,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL },      //ZU860_Doulas_0033
    {"372", 372,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINPUT_KEY,                 NULL,          CLI_COM_NORMAL },

    //Logo
    {"82",  82,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLOGO_CHANGE,               NULL,          CLI_COM_NORMAL},
	{"395", 395,	3,	"1",  1,  READ_COMMAND, 	DECIAML_2,			edcLOGO_CHANGE, 			  NULL,			 CLI_COM_NORMAL }, //R70PD_Steven_0001, 3DE ISS-0018115
	//{"82",  82,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Logo,           CLI_COM_NORMAL},   //ZU860_Doulas_0033
    //{"395", 395,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Logo,          CLI_COM_NORMAL }, //R70PD_Steven_0001, 3DE ISS-0018115

    //Captured Logo
    //{"83",  83,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLOGO_SAVE,                 NULL,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0088

    //Delete Logo
    {"407",  407,   3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_DeleteLogo,     CLI_COM_NORMAL},    //A65_OPTOMA_Julie_0083

    //Background Color
    {"104", 104,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_BackgroundColor,          CLI_COM_NORMAL },      //ZU860_Doulas_0033
    {"396", 396,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcBACKGROUND_COLOR,          NULL,          CLI_COM_NORMAL },

    //Serial Port Path
    {"557", 557,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_SerialPortPath,          CLI_COM_NORMAL },      //ZU860_Doulas_0033

    //Re-Sync
    {"01",  1,      2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_ReSync,           CLI_COM_NORMAL},   //ZU860_Doulas_0034

    //Filter Wheel index
    {"528", 528,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcFILTER_WHEEL_INDEX,        NULL,           CLI_COM_NORMAL},     //ZU860_Doulas_0035
    {"530", 530,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcFILTER_WHEEL_INDEX,        NULL,           CLI_COM_NORMAL},     //ZU860_Doulas_0035

    //Phosphor Wheel index
    {"529", 529,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcPHOSPHOR_WHEEL_INDEX,      NULL,           CLI_COM_NORMAL},     //ZU860_Doulas_0035
    {"531", 531,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcPHOSPHOR_WHEEL_INDEX,      NULL,           CLI_COM_NORMAL},     //ZU860_Doulas_0035

    //Sleep Timer Always On
    {"507", 507,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSleepTimer_AlwaysOn,       NULL,           CLI_COM_NORMAL},     //ZU860_Doulas_0037

    //3D-2D
    {"400", 400,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edc3D_2D,                     NULL,           CLI_COM_NORMAL},     //ZU860_Doulas_0037

    //3D Tech,3D Mode
    {"230", 230,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_3D_Tech,           CLI_COM_NORMAL},    //ZU860_Doulas_0038
    {"297", 297,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edc3D_MODE,                   NULL,           				CLI_COM_NORMAL},  //H30K_David_0016
    {"298", 298,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcDLPLink_ON,                NULL,           CLI_COM_NORMAL},	//R70PD_Zonic_0079 //H30K_David_0017

    //Security
    {"78",  78,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Security,           CLI_COM_NORMAL},    //ZU860_Doulas_0039
    {"391", 391,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Security,           CLI_COM_NORMAL},
    {"537", 537,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSECURITY_TIMER_MONTH,      NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0039
    {"538", 538,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSECURITY_TIMER_DAY,        NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0039
    {"539", 539,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSECURITY_TIMER_HOUR,       NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0039
    {"77",  77,     2,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_SecurityTimer,           CLI_COM_NORMAL},    //ZU860_Doulas_0039

    {"406", 406,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Security_ChangePassword, CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0084

    {"544", 544,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSECURITY_TIMER_MONTH,      NULL,           CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0048
    {"544", 544,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcSECURITY_TIMER_DAY,        NULL,           CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0048
    {"544", 544,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcSECURITY_TIMER_HOUR,       NULL,           CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0048
    {"544", 544,    3,  "5",  5,  READ_COMMAND,     DECIAML_2,          edcINVALID,         		  utilOptoma_Security_Status,   CLI_COM_READ_ONLY}, //A65_OPTOMA_David_0014
    //Geometric Correction reset
    {"561", 561,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcWARP_RESET,                NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0041

    //WLAN Enable
    {"450", 450,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcWLAN_ENABLE,               NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0041

    //WLAN MAC Address
    {"555", 555,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcWLAN_MAC_ADDRESS,          NULL,           CLI_COM_READ_ONLY},   //ZU860_Doulas_0041
    //LAN MAC Address
    {"555", 555,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLAN_MAC_ADDRESS,           NULL,           CLI_COM_READ_ONLY},   //ZU860_Doulas_0041

    //WLAN IP Address
    {"451", 451,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcWLAN_DEFAULT_GATEWAY,      NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0102 modify//ZU860_Doulas_0041

    //WLAN SSID
    {"451", 451,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcWLAN_SSID,                 NULL,           CLI_COM_READ_ONLY},   //ZU860_Doulas_0041

    //WLAN Start IP
    {"451", 451,    3,  "5",  5,  READ_COMMAND,     DECIAML_2,          edcWLAN_START_IP,             NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0102

    //WLAN End IP
    {"451", 451,    3,  "6",  6,  READ_COMMAND,     DECIAML_2,          edcWLAN_END_IP,               NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0102

    //Crestron
    {"454", 454,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Crestron_Control,          CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0095//A35G2_Coda_0061
    {"441", 441,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcCrestron,                  NULL,                                 CLI_COM_NORMAL},

    //Extron
    {"455", 455,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcExtron,                    NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0044
    {"442", 442,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcExtron,                    NULL,           CLI_COM_NORMAL},

    //PJ Link
    {"456", 456,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_PJLink_Control,          CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0095//A35G2_Coda_0061
    {"440", 440,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcPJ_Link,                   NULL,                               CLI_COM_NORMAL},

    //AMX Device Discovery
    {"457", 457,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcAMX,                       NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0044
    {"444", 444,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcAMX,                       NULL,           CLI_COM_NORMAL},

    //Telnet
    {"458", 458,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcTelnet,                    NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0044
    {"445", 445,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcTelnet,                    NULL,           CLI_COM_NORMAL},

    //HTTP
    {"459", 459,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcHTTP,                      NULL,           CLI_COM_NORMAL},      //ZU860_Doulas_0044
    {"446", 446,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcHTTP,                      NULL,           CLI_COM_NORMAL},

	//Network Control Reset
    //{"181", 181,    3,  "1",  1,  WRITE_COMMAND,    DECIAML_2,          edcNETWORK_CONTROL_RESET,     NULL,           CLI_COM_WRITE_ONLY},  //A65_OPTOMA_Julie_0036 //HICC2_Julie_0001

    //Output 3D state
    {"130", 130,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Output_3D_State,           CLI_COM_READ_ONLY},   //ZU860_Doulas_0065

    //OSD reset
    {"546", 546,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_OSD_Reset,           CLI_COM_NORMAL},      //ZU860_Doulas_0065

    //3D reset
    {"234", 234,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_3D_Reset,           CLI_COM_NORMAL},	//ZU860_Clare_0065//ZU860_Doulas_0066

    //Image reset
    {"509", 509,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Image_Reset,           CLI_COM_NORMAL},      //ZU860_Doulas_0066

    //Lens Memory reset
    {"361", 361,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_LensMemory_Reset,           CLI_COM_NORMAL},      //ZU860_Doulas_0066

    //AMBIENT_TEMPERATURE
    {"150", 150,    3,  "23", 23, READ_COMMAND,     DECIAML_2,          edcAMBIENT_TEMPERATURE,        NULL,          CLI_COM_NORMAL},   //HICC2_Sammy_0002

    //System Temperature
    {"150", 150,    3,  "18", 18, READ_COMMAND,     DECIAML_2,          edcSYSTEM_TEMPERATURE,        NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0097
    {"352", 352,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSYSTEM_TEMPERATURE,        NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0117

    //Color Depth
    {"156", 156,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcMAIN_COLOR_DEPTH,          NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0097 //H30K_David_0013
    //Color Gamut
    //{"156", 156,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_ColorGamut, CLI_COM_NORMAL},
    {"156", 156,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcMAIN_COLOR_GAMUT,          NULL,          CLI_COM_NORMAL}, //HICC2_Julie_0001

    //Sub Color Depth
    {"156", 156,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcSUB_COLOR_DEPTH,          NULL,          CLI_COM_NORMAL},
    //Sub Color Gamut
    {"156", 156,    3,  "4",  4,  READ_COMMAND,     DECIAML_2,          edcSUB_COLOR_GAMUT,          NULL,          CLI_COM_NORMAL},

    //Color Format
    {"157", 157,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcCOLOR_FORMAT_INFO,         NULL,          CLI_COM_NORMAL},   //ZU860_Doulas_0097

    //HDR
    {"565", 565,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcHDR_AUTOENABLE,            NULL,           CLI_COM_NORMAL},  //ZU860_Doulas_0100
    {"291", 291,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcHDR_AUTOENABLE,            NULL,           CLI_COM_NORMAL},  //H30K_David_0010
    //HDR Picture Mode
    {"566", 566,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HDRLevel,        CLI_COM_NORMAL},  //ZU860_Doulas_0100//H30K_David_0020
	{"291", 291,	3,	"2",  2,  READ_COMMAND, 	DECIAML_2,			edcINVALID,			  	  	  utilOptoma_HDRLevel, 		  CLI_COM_NORMAL},  //H30K_David_0020

    //Extreme Black
    {"218", 218,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_ExtremeBlack,           CLI_COM_NORMAL},  //ZU860_Doulas_0102
    {"271", 271,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcREAL_BLACK,                NULL,           CLI_COM_NORMAL},

	//Extreme Black Mute timer
    {"256", 256,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLIGHTS_OUT_TIMER_X05,      NULL,           CLI_COM_NORMAL},  //A65_OPTOMA_Doulas_0074

	//Extreme Black signal level
	{"257", 257,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcLIGHTS_OUT_SIGNAL_LEVEL,   NULL,			  CLI_COM_NORMAL}, 	//A65_OPTOMA_Doulas_0074

    //Keypad Lock
    {"103", 103,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcKEYPAD_LOCK,               NULL,           CLI_COM_NORMAL},
    {"392", 392,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcKEYPAD_LOCK,               NULL,           CLI_COM_NORMAL},

    //Keypad & Power Key LED Settings
    {"362", 362,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,          		  utilOptoma_BackLight,           CLI_COM_NORMAL},  //A65_OPTOMA_Julie_0037//ZU860_Doulas_0103
    {"393", 393,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcKEYPAD_BACKLIGHT,          NULL,           CLI_COM_NORMAL},
    {"393", 393,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcPWRKEY_BACKLIGHT,          NULL,    		  CLI_COM_NORMAL},//H30K_David_0012

    //Serial Port Baud Rate
    {"153", 153,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_SerialPortBaudRate,          CLI_COM_NORMAL},     //ZU860_Doulas_0103

	//Serial Port Out Baud Rate
	{"153", 153,	3,	"3",  3,  READ_COMMAND, 	DECIAML_2,			edcINVALID, 				  utilOptoma_SerialPortOutBaudRate,		  CLI_COM_NORMAL},	   //A65_OPTOMA_Doulas_0085

	//Network Factory Reset
	//{"176", 176,	3,	"1",  1,  WRITE_COMMAND,	DECIAML_2,			edcCOMMUNICATION_RESET,	  	  NULL, 		  CLI_COM_WRITE_ONLY},  //A65_OPTOMA_Julie_0037//A65_OPTOMA_Julie_0036
    {"181", 181,    3,  "1",  1,  WRITE_COMMAND,    DECIAML_2,          edcCOMMUNICATION_RESET,       NULL,           CLI_COM_WRITE_ONLY},  //A65_OPTOMA_Julie_0036//HICC2_Julie_0001//H30K_David_0032

    //LAN FW version
    {"357", 357,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_LAN_FW_Version,          CLI_COM_NORMAL},  //ZU860_Doulas_0117

    //Info String
    {"150", 150,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Info_String,          CLI_COM_NORMAL},  //ZU860_Doulas_0118

    //Fan speed
    {"351", 351,    3,  "0",  0,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL}, //HICC2_Sammy_0002
    {"351", 351,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "4",  4,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "5",  5,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "6",  6,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "7",  7,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "8",  8,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL},
    {"351", 351,    3,  "9",  9,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Fan_Speed,          CLI_COM_NORMAL}, //HICC2_Sammy_0002

    //Notification
    {"168", 168,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_SystemUpdate,       CLI_COM_NORMAL},       //ZU860_Owen_0019
    {"158", 158,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcNOTIFICATIONSYSTEMUPDATE,  NULL,                          CLI_COM_NORMAL},       //ZU860_Owen_0019
    {"398", 398,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSILENTFOTA_SYSTEMUPDATE,   NULL,                             CLI_COM_NORMAL},

    //Auto Image Setting ( Auto Focus, Auto Wall Color )
    {"222", 222,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,         	      utilOptoma_Auto_Focus_Status,  CLI_COM_NORMAL},   //A65_OPTOMA_Julie_0048

    // Low Latency Mode
    {"133", 133,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLOW_LATENCY_MODE,          NULL,                          CLI_COM_NORMAL},
    {"220", 220,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLOW_LATENCY_MODE,          NULL,                          CLI_COM_NORMAL},

    // Get Device Type
    {"149", 149,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_GetDeviceType,      CLI_COM_NORMAL},

    // Camera Module Status
    {"221", 221,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_CameraModuleStatus, CLI_COM_NORMAL},

    // Broadcast Message
    {"210", 210,    3,  NULL, 0,  WRITE_COMMAND,    STRING_CHAR,        edcINVALID,                   utilOptoma_OMSOptomaBroadCastMsg,   CLI_COM_NORMAL},

	//Display reset
	{"173", 173,    3,  "1",  1,  WRITE_COMMAND,    DECIAML_3,          edcDISPLAY_RESET,             NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0076

	//HDMI Output
	{"309", 309,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HDMI_OUTPUT_Set,     	CLI_COM_NORMAL },	//A65_OPTOMA_Doulas_0077

	//HDMI 1 EDID
	{"236", 236,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HDMI1_EDID_Set,       	CLI_COM_NORMAL },	//A65_OPTOMA_Doulas_0077
    {"374", 374,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_HDMI1_EDID_Set,         CLI_COM_NORMAL },

	//HDMI 2 EDID
	{"237", 237,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HDMI2_EDID_Set,       	CLI_COM_NORMAL },	//A65_OPTOMA_Doulas_0077
    {"375", 375,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_HDMI2_EDID_Set,         CLI_COM_NORMAL },

	//HDBaseT EDID
	{"238", 238,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_HDBASET_EDID_Set,     	CLI_COM_NORMAL },	//A65_OPTOMA_Doulas_0077
    {"376", 376,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_HDBASET_EDID_Set,      CLI_COM_NORMAL },

	//Native Resolution
	{"316", 316,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                   utilOptoma_Native_Resolution,     CLI_COM_NORMAL },
    {"448", 448,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                   utilOptoma_Native_Resolution,     CLI_COM_NORMAL },


	//Input Settings Reset
	{"178", 178,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINPUT_SETTINGS_RESET,      NULL,             			   	CLI_COM_NORMAL },	//A65_OPTOMA_Doulas_0077

    //Backup Restore Memory
    {"258", 258,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,         	utilOptoma_Backup_Restore_Save,      CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0037
    {"259", 259,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,         	utilOptoma_Backup_Restore_Load,      CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0037
    {"397", 397,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,             utilOptoma_Backup_Restore_Load,      CLI_COM_NORMAL },//H30K_David_0030

	//Setup reset
	{"179", 179,    3,  "1",  1,  WRITE_COMMAND,    DECIAML_3,          edcSETUP_RESET,             NULL,           CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0037

	//Black Level Reset
	{"167", 167,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,             	utilOptoma_Black_Level_Reset,       CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0046

	//Auto Switch (backup input)
    {"264", 264,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcBACKUPINPUT_AUTOSWITCH,  	NULL,       					CLI_COM_NORMAL},    //A65_OPTOMA_Doulas_0100
    {"274", 274,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_AUTOSWITCH,  	NULL,                          	CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0100

	//First Input (backup input)
	{"265", 265,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,  	utilOptoma_Backup_Input_First_Input,       		CLI_COM_NORMAL},    //A65_OPTOMA_Doulas_0100
    {"275", 275,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,  	utilOptoma_Backup_Input_First_Input,            CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0100

	//Second Input (backup input)
	{"266", 266,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,  	utilOptoma_Backup_Input_Second_Input,       	CLI_COM_NORMAL},    //A65_OPTOMA_Doulas_0100
    {"276", 276,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,  	utilOptoma_Backup_Input_Second_Input,           CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0100

	//backup input status
    {"277", 277,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcBACKUPINPUT_STATUS,  	NULL,                          	CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0088

	//backup input change
    {"278", 278,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcBACKUP_INPUT_CHANGE,  	NULL,                          	CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0088

	//backup input info
    {"279", 279,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,  	utilOptoma_Backup_Input_Current_Input,      CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0088

#if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Tim_0012, add, start
	//ACU Calibration
	{"261", 261,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,             	utilOptoma_ACU_Calibration,         CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0048
    {"225", 225,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_ACU_Calibration,         CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0048
#endif
	//Date and Time
    {"243", 243,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_Date_and_Time,           CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0048

	//Clock Mode
    {"474", 474,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_Clock_Mode,            CLI_COM_NORMAL},		//A65_OPTOMA_Julie_0084

	//Date Year
    {"475", 475,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_4,          edcINVALID,                 utilOptoma_Date_and_Time,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Date Month
    {"476", 476,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_Date_and_Time,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Date Day
    {"477", 477,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_Date_and_Time,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Time Hour
    {"478", 478,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_Date_and_Time,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Time Minute
    {"479", 479,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,                 utilOptoma_Date_and_Time,          CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Daylight Saving Time
    {"480", 480,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcDATE_DAYNIGHT_SAVING_TIME,			NULL,				    CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//NTP Server
    {"481", 481,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,				 utilOptoma_NTP_Server,				CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Time Zone
    {"482", 482,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,				 utilOptoma_Time_Zone,				CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Update Interval
    {"483", 483,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,				utilOptoma_Update_Interval,			CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Date and Time Apply
    {"484", 484,    3,  "1",  1,  WRITE_COMMAND,     DECIAML_2,          edcDATE_APPLY,				NULL,			CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0095 //A35G2_Coda_0061

	//IPv6 DHCP
    {"485", 485,    3,  "3",  3,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,				utilOptoma_IPv6_DHCP,				CLI_COM_NORMAL},
    {"485", 485,    3,  "4",  4,  WRITE_COMMAND,     DECIAML_2,          edcINVALID,				utilOptoma_IPv6_DHCP,				CLI_COM_NORMAL},

	//IPV6 Prefix Length
    {"486", 486,    3,  NULL, 0,  WRITE_COMMAND,     DECIAML_2,          edcIPV6_PREFIX_LENGTH,		NULL,			CLI_COM_NORMAL},

	//Schedule Mode, Schedule Weekday enable, Schedule Reset
    {"284", 284,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID, 			    utilOptoma_Schedule_Function_Set, CLI_COM_NORMAL},   //A65_OPTOMA_Julie_0084 //A65_OPTOMA_Julie_0083

	//Schedule Mode
    {"244", 244,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSCHEDULE_MODE,            NULL,          CLI_COM_NORMAL},						//A65_OPTOMA_Julie_0048

	//Schedule View Today
    {"243", 243,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,      			utilOptoma_Schedule_View_Today,     CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0048

	//Schedule Event
    {"471", 471,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,            		utilOptoma_Schedule_Event_Set,      CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Schedule Weekday Reset
    {"472", 472,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,            		utilOptoma_Schedule_Weekday_Reset,  CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0084

	//Schedule Copy Weekday
    {"473", 473,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,            		utilOptoma_Schedule_Copy_Weekday,  CLI_COM_NORMAL},		//A65_OPTOMA_Julie_0084

	//Fade in/out
    {"267", 267,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,           	 	 utilOptoma_Fade_In,            CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0048
    {"390", 390,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,          		 utilOptoma_Fade_In,          	CLI_COM_NORMAL},//H30K_David_0023
    {"268", 268,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,           	 	 utilOptoma_Fade_Out,           CLI_COM_NORMAL},	//A65_OPTOMA_Julie_0048
	{"390", 390,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,          		 utilOptoma_Fade_Out,          	CLI_COM_NORMAL},//H30K_David_0024

	//Shutter Startup
    {"269", 269,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcSTARTUP_SHUTTER,          NULL,          CLI_COM_NORMAL},						//A65_OPTOMA_Julie_0048
    {"390", 390,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSTARTUP_SHUTTER,          NULL,          CLI_COM_NORMAL},

    //Black level top/bottom off/on
    {"166",  166,   3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelEnable,      CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0168

	//Black level Brightness
    {"263", 263,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBrightness,  CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0168

	//Black level Bottom Red
	{"272", 272,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBottomRed,   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0168
    {"281", 281,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBottomRed,   CLI_COM_NORMAL},
	//Black level Top Red
	{"273", 273,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelTopRed,      CLI_COM_NORMAL},
    {"285", 285,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelTopRed,      CLI_COM_NORMAL},
	//Black level Bottom Green
	{"272", 272,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBottomGreen, CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0168
    {"282", 282,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBottomGreen, CLI_COM_NORMAL},
	//Black level Top Green
	{"273", 273,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelTopGreen,    CLI_COM_NORMAL},
    {"286", 286,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelTopGreen,    CLI_COM_NORMAL},
	//Black level Bottom Blue
	{"272", 272,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBottomBlue,  CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0168
    {"283", 283,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelBottomBlue,  CLI_COM_NORMAL},
	//Black level Top Blue
	{"273", 273,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelTopBlue,     CLI_COM_NORMAL},
    {"287", 287,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                  utilOptoma_BlackLevelTopBlue,     CLI_COM_NORMAL},

	//Digital Zoom Proportional / Digital Zoom reset
	{"364", 364,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,			utilOptoma_DigitalZoom_Shift_And_Reset,    CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0174

	//Digital Zoom Horizontal Shift
	{"365", 365,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDIGITAL_HORZ_SHIFT,		 NULL,       		               CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0174
	//Digital Zoom Vertical Shift
	{"366", 366,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcDIGITAL_VERT_SHIFT,		 NULL,       		               CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0174

	//Warp Control
	{"142", 142,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,			         utilOptoma_Warp_Control,    	   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0174

	//LAN interface
#if(NETWORK_IPV6_ENABLE)
	{"460", 460,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,      	 		 utilOptoma_Lan_Path_Switch,       CLI_COM_NORMAL},// HICC2_Bruce_0021
#else
	{"460", 460,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcLAN_PATH_SWITCH,      	 NULL,          				   CLI_COM_NORMAL},	//A65_OPTOMA_Doulas_0177
#endif
	{"386", 386,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLAN_PATH_SWITCH,          NULL,                             CLI_COM_NORMAL},


	//Network Setup: Apply/reset
	{"462", 462,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcINVALID,	 				 utilOptoma_Network_Control,       CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0095 //A35G2_Coda_0061

	//Creston IP Address
	{"465", 465,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcINVALID,	 				 utilOptoma_Crestron_IP_Address,   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0177

	//Creston IPID
	{"466", 466,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcINVALID,	 		 		 utilOptoma_Crestron_IPID,		   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0177

	//Creston Port
	{"467", 467,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcINVALID,	 		         utilOptoma_Crestron_Port,		   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0177

	//Authentication
	{"468", 468,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcPJLINK_SECRET_ENABLE,	 NULL,		   					   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179

	//PJLink password
	{"440", 440,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcPJLINK_SECRET_PASSWORD,   NULL,   						   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179
	{"470", 470,	3,	NULL, 0,  WRITE_COMMAND,	STRING_CHAR,		edcINVALID,	 				 utilOptoma_PJLink_Password,	   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0226 Modify//A65_OPTOMA_Doulas_0179  //A35G2_Coda_0087

	//PJLink Service
	{"469", 469,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcINVALID, 				 utilOptoma_PJLink_Service,   	   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179

	//Email 1
	{"443", 443,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcSNMP_RECIPIENT_EMAIL_1,   NULL,   						   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179
	//Email 2
	{"443", 443,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcSNMP_RECIPIENT_EMAIL_2,   NULL,   						   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179

	//Email - Fan Error,Power On/Off,Video Loss,Laser
	{"463", 463,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcINVALID,	 				utilOptoma_SNMP_Setting,		   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179

	//Email reset
	{"464", 464,	3,	NULL, 0,  WRITE_COMMAND,	DECIAML_2,			edcSNMP_RESET,	 			 NULL,							   CLI_COM_NORMAL}, //A65_OPTOMA_Doulas_0179

    //Quick Resync
    {"315", 315,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcAUTO_SOURCE_RESYNC,      	 NULL,          				   CLI_COM_NORMAL}, //A35G2_Coda_0064
    {"373", 373,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcAUTO_SOURCE_RESYNC,           NULL,            CLI_COM_NORMAL},

    //Pro Service(OMS) LED //A35G2_Coda_0124
    {"155", 155,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,				utilOptoma_ProService_TemperatureStatus,  CLI_COM_NORMAL},
//#if !defined(PLATFORM_H30_4K) //HICC2_Sammy_0002
    {"159", 159,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcINVALID,				utilOptoma_ProService_FanStatus,          CLI_COM_NORMAL},
//#else
	//Pressure(hPA)
    {"159", 159,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcALTIMETRY_VALUE,				NULL,          CLI_COM_NORMAL},	//HICC2_Sammy_0002
	//Humidity
    {"159", 159,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcBAROMETRY_VALUE,				NULL,          CLI_COM_NORMAL}, //HICC2_Sammy_0002
//#endif
    //OSD Lock  //A35G2_Coda_0127
    {"239", 239,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                 utilOptoma_OSD_Lock,				CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0125
    {"229", 229,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcOSD_LOCK,				NULL,								CLI_COM_NORMAL}, //A65_OPTOMA_Julie_0125

    //USB Power
    {"520", 520,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcUSB_POWER,               NULL,          CLI_COM_NORMAL },      //HICC2_Sammy_0002
    {"389", 389,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcUSB_POWER,               NULL,          CLI_COM_NORMAL},		//HICC2_Sammy_0002

	//Lens Type
    {"245", 245,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcLENS_TYPE,             NULL,          CLI_COM_NORMAL},		//HICC2_Sammy_0002

	// OMS
	{"247", 247,	3,	"1",  1,  READ_COMMAND, 	DECIAML_2,			edcINVALID, 			utilOptoma_OMS,             CLI_COM_NORMAL},//H30K_David_0029

    //Crestron V2
    {"999", 239,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,                utilOptoma_OSD_Lock/* utilGeneral_Debug_Schedule*/,				CLI_COM_NORMAL},

#ifdef CLI_ERROR_LOG_ENABLE //A65_OPTOMA_Jerry_0006
    {"990", 990,  3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                   utilOptoma_AP_DebugCmd_Enable,      CLI_COM_NORMAL},
    {"991", 991,  3,  NULL, 0,  WRITE_COMMAND,    DECIAML_3,          edcINVALID,                   utilOptoma_AP_DebugCmd,             CLI_COM_NORMAL},
#endif

    //H30K_Julie_0005, start.
    //Energy Saving
    //{"319", 319,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcENERGY_SAVING,               NULL,          CLI_COM_NORMAL},
    //{"249", 249,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcENERGY_SAVING,               NULL,          CLI_COM_NORMAL},

    //Control Art-Net
    {"452", 452,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcART_NET,               NULL,          CLI_COM_NORMAL},//H30K_David_0043
    {"447", 447,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcART_NET,               NULL,          CLI_COM_NORMAL},//H30K_David_0043

    //Art-Net/Net
    {"425", 425,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcARTNET_NET,               NULL,          CLI_COM_NORMAL},
    {"226", 226,    3,  "1",  1,  READ_COMMAND,     DECIAML_2,          edcARTNET_NET,               NULL,          CLI_COM_NORMAL},

    //Art-Net/Subnet
    {"426", 426,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcARTNET_SUBNET,               NULL,          CLI_COM_NORMAL},
    {"226", 226,    3,  "2",  2,  READ_COMMAND,     DECIAML_2,          edcARTNET_SUBNET,               NULL,          CLI_COM_NORMAL},

    //Art-Net/Universe
    {"427", 427,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcARTNET_UNIVERSE,               NULL,          CLI_COM_NORMAL},
    {"226", 226,    3,  "3",  3,  READ_COMMAND,     DECIAML_2,          edcARTNET_UNIVERSE,               NULL,          CLI_COM_NORMAL},

    //Art-Net/Start Address
    {"428", 428,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcARTNET_START_ADDRESS,               NULL,          CLI_COM_NORMAL},
    {"226", 226,    3,  "4",  4,  READ_COMMAND,     DECIAML_2,          edcARTNET_START_ADDRESS,               NULL,          CLI_COM_NORMAL},

    //Art-Net/Channel Settings, Art-Net/Edit Channel
    {"429", 429,    3,  NULL, 0,  WRITE_COMMAND,    DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3,  "5",  5,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "11", 11,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "12", 12,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "13", 13,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "14", 14,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "15", 15,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "16", 16,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "17", 17,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "18", 18,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "19", 19,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "20", 20,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "21", 21,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "22", 22,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "23", 23,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "24", 24,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "25", 25,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    {"226", 226,    3, "26", 26,  READ_COMMAND,     DECIAML_2,          edcINVALID,               utilOptoma_ART_NET_Channel,          CLI_COM_NORMAL},
    //H30K_Julie_0005, end.

};

#define OPTOMA_CMD_LUT_NUMBER sizeof(m_sOptomaLut)/sizeof(sCLI_OPTOMA_LUT)

static sCLI_DATA_INFO m_sCLI_CustomInfo;

// ==============================================================================
// FUNCTION NAME: utilOptomaCLI_AddressID
// DESCRIPTION:
//
//
// Params:
// UINT16 uiAddress:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/11/21, Larry Create
// --------------------
// ==============================================================================
void utilOptoma_CLI_AddressID(UINT32 ulAddress)
{
    m_ulAddressID = ulAddress;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CMD_Decode
// DESCRIPTION:
//
//
// Params:
// UINT8* pcString:
// sCLI_OPTOMA_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilOptoma_CMD_Decode(UINT8* pcString, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT8   ucStringLen = 0;
    UINT8   ucErrorCode = eCLI_ERROR_CODE_NO;
    UINT8   ucCmdIndex = 0;
    UINT8   ucDecdoeStep = eCLI_DECODE_STEP_HEADER;
    UINT8   ucCount = 0;
    UINT16  uiMainCode = 0;
    UINT8   ucMainCodeLenght = 0;
    UINT16  uiSubCode = 0;
    TickType_t m_ulStartTicks;

    ucStringLen = strlen((char*)pcString);
    LOG_MSG(db_APP_CLI, "pcString: %s, ucStringLen: %d\r\n", pcString, ucStringLen);

    if(ucStringLen > OPTOMA_CMD_LENGTH_MAX)
    {
        return eCLI_ERROR_CODE_SETFAIL;
    }
    if(palMotorLensCalibrationStatusGet() == 1)//H30K_David_0054
    {
    	return eCLI_ERROR_CODE_SETFAIL;
    }
    do
    {
        //LOG_MSG(db_ALWAYS, "uc %d %d %c\n", ucDecdoeStep, ucCmdIndex, pcString[ucCmdIndex]);
        switch(ucDecdoeStep)
        {
            case eCLI_DECODE_STEP_HEADER:
                if(pcString[ucCmdIndex] == OPTOMA_CMD_HEADER_1)
                {
                    ucCmdIndex++;
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucDecdoeStep = eCLI_DECODE_STEP_ADDRESS_CHECK;
                break;

            case eCLI_DECODE_STEP_ADDRESS_CHECK:
                if(__CheckNum(pcString[ucCmdIndex]) && __CheckNum(pcString[ucCmdIndex + 1]))
                {
                    memset(sCmdFormat->uiProjectorAddress, '\0', sizeof(sCmdFormat->uiProjectorAddress));

                    sCmdFormat->uiProjectorAddress[0] = (pcString[ucCmdIndex]);       //ZU860_Doulas_0007 modify
                    sCmdFormat->uiProjectorAddress[1] = (pcString[ucCmdIndex + 1]);   //ZU860_Doulas_0007 modify
                    sCmdFormat->uiProjectorAddress[2] = '\0';

                    sCmdFormat->ulProjectorAddressID = (UINT32)atoi(sCmdFormat->uiProjectorAddress);
                    ucCmdIndex += OPTOMA_ADDRESS_LEN_MAX;
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                ucDecdoeStep = eCLI_DECODE_STEP_MAIN;
                break;

            case eCLI_DECODE_STEP_MAIN:
                if(__CheckNum(pcString[ucCmdIndex]))
                {
                    sCmdFormat->cMainCode[uiMainCode] = '\0';

                    if(uiMainCode >= OPTOMA_MAIN_CMD_LEN)
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    sCmdFormat->cMainCode[uiMainCode] = pcString[ucCmdIndex];
                    uiMainCode++;
                    ucCmdIndex++;
                }
                else if(pcString[ucCmdIndex] == OPTOMA_CMD_SPACE)
                {
                    ucCmdIndex++;
                    ucMainCodeLenght = (UINT8)uiMainCode;
                    uiMainCode = (UINT16)atoi(sCmdFormat->cMainCode);
                    ucDecdoeStep = eCLI_DECODE_STEP_DATA_FORMAT;
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
                break;

            case eCLI_DECODE_STEP_SUB:
                break;

            case eCLI_DECODE_STEP_DATA_FORMAT:
                if((ucCmdIndex + OPTOMA_CMD_TEXT_SIZE + 2) < ucStringLen) //+2 for end ' " 'and ')'
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }

                if (uiMainCode == 210)// utilOptoma_BroadcastMessage()
                {// BruceLin#20210114
                    if(__CheckAsciiCharacter(pcString[ucCmdIndex]) == 0)
                    {
                        LOG_MSG(db_APP_GUI, "%s[%d]: %s() __CheckAsciiCharacter() == 0\r\n", __FILE__, __LINE__, __FUNCTION__);
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    UINT8 ucStrLen = ucStringLen - ucCmdIndex;

                    if(ucStringLen > ucCmdIndex)
                    {
                        ucStrLen = ucStringLen - ucCmdIndex;
                    }
                    else
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    memcpy(sCmdFormat->cTextString, (UINT8*)&pcString[ucCmdIndex], ucStrLen);
                    sCmdFormat->cTextString[ucStrLen - 1] = '\0';
                    sCmdFormat->ucDataType = eCLI_DATA_TYPE_STRING;
                    ucCmdIndex = ucStringLen - 2;
                    ucDecdoeStep = eCLI_DECODE_STEP_END;
                    LOG_MSG(db_APP_CLI, "\r\n--ucStrLen %d\r\n", ucStrLen);
                }
                else
                {
                    if(__CheckText(pcString[ucCmdIndex]) == 0)
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    UINT8 ucStringCheck = 0;
                    UINT8 ucStrLen = ucStringLen - ucCmdIndex;

                    if(ucStringLen > ucCmdIndex)
                    {
                        ucStrLen = ucStringLen - ucCmdIndex;
                    }
                    else
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    memcpy(sCmdFormat->cTextString, (UINT8*)&pcString[ucCmdIndex], ucStrLen);
                    sCmdFormat->cTextString[ucStrLen - 1] = '\0';

                    for(ucCount = 0; ucCount < ucStrLen; ucCount++)
                    {
                        if(sCmdFormat->cTextString[ucCount] == '\0')
                        {
                            break;
                        }
                        else
                        {
                            if(__CheckNum(sCmdFormat->cTextString[ucCount]) == 0)
                            {
                               if(sCmdFormat->cTextString[ucCount] !='-')//H30K_David_0041
                                ucStringCheck = 1;
                            }
                        }
                    }

                    if((ucStringCheck == 0) && ((ucStrLen + 1) < 30))
                    {
                        sCmdFormat->ucDataType = eCLI_DATA_TYPE_DEC;
                        sCmdFormat->lData = atoi(sCmdFormat->cTextString);
                    }
                    else
                    {
                        sCmdFormat->ucDataType = eCLI_DATA_TYPE_STRING;
                    }

                    ucCmdIndex = ucStringLen - 2;
                    ucDecdoeStep = eCLI_DECODE_STEP_END;
                    LOG_MSG(db_APP_CLI, "\r\n--ucStrLen %d\r\n", ucStrLen);
                }
                break;

            case eCLI_DECODE_STEP_END:
                if (uiMainCode == 210)// utilOptoma_BroadcastMessage()
                {// BruceLin#20210114
                    if(__CheckAsciiCharacter(pcString[ucCmdIndex]) || (pcString[ucCmdIndex] == OPTOMA_CMD_END_1))
                    {
                        ucCmdIndex++;
                        ucErrorCode = eCLI_ERROR_CODE_NO;
                    }
                    else
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                }
                else
                {
                    if(__CheckText(pcString[ucCmdIndex]) || (pcString[ucCmdIndex] == OPTOMA_CMD_END_1))
                    {
                        ucCmdIndex++;
                        ucErrorCode = eCLI_ERROR_CODE_NO;
                    }
                    else
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                }
                break;

            default:
                ucCmdIndex = ucStringLen;
                break;
        }
        }while(ucCmdIndex < ucStringLen);

        //LOG_MSG(db_APP_CLI, "Add code %s, Host code %s\n", sCmdFormat->uiProjectorAddress, sCmdFormat->uiProjectorHost);
        //LOG_MSG(db_APP_CLI, "Main code %s, main code id %d\n", sCmdFormat->cMainCode, uiMainCode);
        //LOG_MSG(db_APP_CLI, "Sub code %s, sub code id %d\n", sCmdFormat->cSubCode, uiSubCode);
        //LOG_MSG(db_APP_CLI, "CmdType %d\n", sCmdFormat->ucCmdType);

        m_ulStartTicks = xTaskGetTickCount();

        //Lookup sub code
        for(uiSubCode = 0; uiSubCode < OPTOMA_CMD_LUT_NUMBER; uiSubCode++)
        {
            if((m_sOptomaLut[uiSubCode].uiMainCmdID == uiMainCode) && (m_sOptomaLut[uiSubCode].ucMainCmdLenght == ucMainCodeLenght))
            {
                LOG_MSG(db_APP_CLI," uiMainCode = %d (uiSubCode %d) [Total %d]\r\n", uiMainCode, uiSubCode, OPTOMA_CMD_LUT_NUMBER);
                sCmdFormat->uiFuncID = uiSubCode;
                sCmdFormat->ucIsRead = WRITE_COMMAND;

                if(m_sOptomaLut[uiSubCode].ucCmdIsRead == READ_COMMAND)
                {
                    if(m_sOptomaLut[uiSubCode].cSubCmd == NULL)
                    {
                        continue;
                    }
                    else
                    {
                        if(m_sOptomaLut[uiSubCode].uiSubCmdID != sCmdFormat->lData)
                        {
                            continue;
                        }
                        else
                        {
                            sCmdFormat->ucIsRead = READ_COMMAND;
                        }
                    }
                }
                break;
            }
        }

        LOG_MSG(db_APP_CLI, "UTime %u\n", m_ulStartTicks - xTaskGetTickCount());
        LOG_MSG(db_APP_CLI," Final uiSuvCode = %d (%d)\r\n", uiSubCode, OPTOMA_CMD_LUT_NUMBER);

        if(uiSubCode == OPTOMA_CMD_LUT_NUMBER)
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        return ucErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CMD_Reply
// DESCRIPTION:
//
//
// Params:
// UINT8 ucChannel:
// UINT8 ucErrorCode:
// sCLI_OPTOMA_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
void utilOptoma_CMD_Reply(UINT8 ucChannel, UINT8 ucErrorCode, sCLI_OPTOMA_FORMAT* sCmdFormat, char *ucReturnString)
{
    UINT8 ucDataType = utilCommonCLI_DataTypeGet(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID); //A70LV_Larry_0065

    if((m_sOptomaLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_NO_REPLY) && (ucErrorCode == eCLI_ERROR_CODE_NO))
    {
        return;
    }

    if((ucErrorCode != eCLI_ERROR_CODE_NO) && (ucErrorCode < eCLI_ERROR_NUMBER))
    {
        if(ucChannel == eccTelnet)	//ZU860_Clare_0110
        {
            sprintf(ucReturnString, "[OPTM!F]\r");
        }
        else
        {
            sprintf(ucReturnString, "F\r");
        }
    }
    else
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND) //Write
        {
            if(ucChannel == eccTelnet)
            {
                sprintf(ucReturnString, "[OPTM!P]\r");
            }
            else
            {
                sprintf(ucReturnString, "P\r");
            }
        }
        else //Read
        {
            char ucCLITemp[32] = {'\0'};

            sprintf(ucCLITemp, "Ok");
            if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_OUTPUT_STRING)
            {
                ucDataType = DATATYPE_STRING;
            }

            switch(ucDataType)
            {
                case DATATYPE_DIGIT:
                    if(ucChannel == eccTelnet)
                    {
                    	sprintf(ucReturnString, "[OPTM!%s%d]\r", ucCLITemp, sCmdFormat->lData);
                    }
                    else
                    {
                    	sprintf(ucReturnString, "%s%d\r", ucCLITemp, sCmdFormat->lData);
                    }
                    break;

                case DATATYPE_STRING:
                    if(ucChannel == eccTelnet)
                    {
                    	sprintf(ucReturnString, "[OPTM!%s%s]\r", ucCLITemp, sCmdFormat->cTextString);
                    }
                    else
                    {
                    	sprintf(ucReturnString, "%s%s\r", ucCLITemp, sCmdFormat->cTextString);
                    }
                    break;

                default:
                    break;
            }
        }
    }
    //LOG_MSG(db_ALWAYS, "\r\n%s\r\n", ucReturnString);
    return;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CMD_Handle
// DESCRIPTION:
//
//
// Params:
// sCLI_OPTOMA_FORMAT* sCmdFormat:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilOptoma_CMD_Handle(sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO; //A35G2_Wesley_0155
    UINT8 ucDataType = 0;
    UINT8 ucFreeze = 0; //A35G2_Wesley_0155

    ucDataType = utilCommonCLI_DataTypeGet(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID);
    palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucFreeze); //A35G2_Wesley_0155//HICC2_Julie_0038

    if(sCmdFormat->uiFuncID < OPTOMA_CMD_LUT_NUMBER)
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND) //Write command
        {
        #ifdef NO_POWER_OFF_DURING_CAMERA_WORKING			//G100_Tim_0046, add, start
            if(palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )
            {
                return eCLI_ERROR_CODE_SETFAIL;
            }
        #endif //NO_POWER_OFF_DURING_CAMERA_WORKING 		//G100_Tim_0046, add, end

			//A35G2_Wesley_0155//HICC2_Julie_0038
    		if(ucFreeze)
            {
                switch(m_sOptomaLut[sCmdFormat->uiFuncID].uiMainCmdID)
    			{
                    case 4: //Freeze
                    case 2: //Shutter
                    case 0: //Power
                        break;

                    default:
                        ucErrorCode = eCLI_ERROR_CODE_SETFAIL;
                        break;
    			}
    			if(ucErrorCode != eCLI_ERROR_CODE_NO)
    			{
                    return eCLI_ERROR_CODE_SETFAIL;
    			}
            }

            if(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID != edcINVALID)
            {
                switch(utilCommonCLI_DataControl(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID))
                {
                    case eFUNC_CONTROL_ENABLE:
                        break;

                    default:
                        return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else //special case, ex, hpbu test, SST
            {
                if(m_sOptomaLut[sCmdFormat->uiFuncID].iCLI_Replay != NULL)
                {
                    return  m_sOptomaLut[sCmdFormat->uiFuncID].iCLI_Replay(ecmWrite, sCmdFormat);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            switch(sCmdFormat->ucDataType) //Write command
            {
                case eCLI_DATA_TYPE_DEC:
                {
                    if(eEXEC_CODE_PASS == utilCommonCLI_RangeCheck(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID, sCmdFormat->lData))
                    {
                        if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionSet(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID, sCmdFormat->lData))
                        {
                            return eCLI_ERROR_CODE_NO;
                        }
                    }
                    else
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }
                }
                    return eCLI_ERROR_CODE_SETFAIL;

                case eCLI_DATA_TYPE_STRING:
                {
                    if(ucDataType != DATATYPE_STRING)
                    {
                        return eCLI_ERROR_CODE_SETFAIL;
                    }

                    if(eEXEC_CODE_PASS == utilCommonCLI_String_Set(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID, (UINT8*)sCmdFormat->cTextString))
                    {
                        return eCLI_ERROR_CODE_NO;
                    }
                }
                    return eCLI_ERROR_CODE_SETFAIL;

                default:
                    break;
            }
        }
        else //Read
        {
            if(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID != edcINVALID)
            {
                switch(utilCommonCLI_DataControl(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID))
                {
                    case eFUNC_CONTROL_ENABLE:
                    case eFUNC_CONTROL_DISABLE:
                        break;

                    case eFUNC_CONTROL_SOURCE_DEPEND:
                        return eCLI_ERROR_CODE_SETFAIL;

                    case eFUNC_CONTROL_SERCIVE_PROTECT:
                        return eCLI_ERROR_CODE_SETFAIL;

                    default:
                        return eCLI_ERROR_CODE_SETFAIL;
                }
            }
            else //special case, ex, hpbu test, SST
            {
                if(m_sOptomaLut[sCmdFormat->uiFuncID].iCLI_Replay != NULL)
                {
                    return m_sOptomaLut[sCmdFormat->uiFuncID].iCLI_Replay(ecmRead, sCmdFormat);
                }
                else
                {
                    return eCLI_ERROR_CODE_SETFAIL;
                }
            }

            switch(ucDataType) //Read command
            {
                case DATATYPE_DIGIT:
                {
                    INT32 iData = 0;
                    if(eEXEC_CODE_PASS == utilCommonCLI_DataConversionGet(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID, &iData))
                    {
                        sCmdFormat->lData = iData;
                        return eCLI_ERROR_CODE_NO;
                    }
                }
                    return eCLI_ERROR_CODE_SETFAIL;

                case DATATYPE_STRING:
                {
                    UINT8 ucString[OPTOMA_CMD_LENGTH_MAX] = {'\0'};
                    UINT8 ucStrlen = 0;

                    if(eEXEC_CODE_PASS == utilCommonCLI_String_Get(m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID, ucString))
                    {
                        ucStrlen = strlen((char*)ucString);

                        if(ucStrlen > OPTOMA_CMD_TEXT_SIZE)
                        {
                            ucStrlen = OPTOMA_CMD_TEXT_SIZE;
                        }
                        memcpy(sCmdFormat->cTextString, ucString, ucStrlen);
                        return eCLI_ERROR_CODE_NO;
                    }
                }
                    return eCLI_ERROR_CODE_SETFAIL;

                default:
                    break;
            }
        }
    }
    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_Handle
// DESCRIPTION:
//
//
// Params:
// eCLI_CHANNEL eCh:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 utilOptoma_CLI_Handle(UINT8 eCh, UINT8 ucData)
{
    //UINT8 ucData = 0;// BruceLin#20210107
    UINT16 uiEmpty = 0;
    UINT8 ucDataOut = 0; //A70LV_John_0008 add basic function and flow of GEC

    if(eCh > eccNumber)
        return 0;

    if(ucData == m_sCli_Custom_Config.cCMD_Hander || m_sCLI_CustomInfo.uiCLI_Poll_Position != 0)
    {
        m_sCLI_CustomInfo.ucCLI_DATA[m_sCLI_CustomInfo.uiCLI_Poll_Position++] = ucData;
        if(m_sCLI_CustomInfo.uiCLI_Poll_Position > OPTOMA_CMD_LENGTH_MAX)
        {
            m_sCLI_CustomInfo.uiCLI_Poll_Position = 0;
            return 0;
        }

		if(ucData == m_sCli_Custom_Config.cCMD_Hander)
		{	//A35G2_Coda_0060
		    /*
			m_sCLI_CustomInfo.uiCLI_Poll_Position = 0;
			m_sCLI_CustomInfo.ucCLI_DATA[m_sCLI_CustomInfo.uiCLI_Poll_Position++] = ucData;
			*/
		}

        if(ucData ==  m_sCli_Custom_Config.cCMD_End)
        {
            sCLI_OPTOMA_FORMAT sCmdFormat = {0};
            sCmdFormat.ucCmdFrom = eCh; //T100_Simon_0020

            char ucReturnString[OPTOMA_CMD_LENGTH_MAX] = {'\0'};

            m_sCLI_CustomInfo.ucCLI_DATA[m_sCLI_CustomInfo.uiCLI_Poll_Position] = '\0';

            utilOptoma_CLI_Process(eCh, m_sCLI_CustomInfo.ucCLI_DATA, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);

            m_sCLI_CustomInfo.uiCLI_Poll_Position = 0;

            return 1;
        }
    }
    return 0;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_Init
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
// 2018/01/04, Larry Create
// --------------------
// ==============================================================================
void utilOptoma_CLI_Init(void)
{
    UINT8 ucCount = 0;

    for(ucCount = 0; ucCount<eccNumber; ucCount++)
    {
        memset(&m_sCLI_CustomInfo, 0, sizeof(sCLI_DATA_INFO));  //A35G2_Simon_0070 for cppcheck
        m_sCLI_CustomInfo.ucSerialPortEcho = 0;
    }
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_SerialPortEcho
// DESCRIPTION:
//
//
// Params:
// eCLI_CHANNEL eCh:
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
void utilOptoma_CLI_SerialPortEcho(UINT8 eCh, UINT8 ucEcho)
{
    m_sCLI_CustomInfo.ucSerialPortEcho = ucEcho;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_StringOutput
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
// 2018/06/01, Larry Create
// --------------------
// ==============================================================================
void utilOptoma_CLI_StringOutput(UINT8 eCh, char *ucOutputString)      //T100_Simon_0020
{
    switch(eCh)
    {
        case eccPC:
            LOG_MSG(db_ALWAYS, "%s",ucOutputString);    //ZU860_Doulas_0070 modify
            break;

        case eccTelnet:
            palLANProcTelnetReply((UINT8 *)ucOutputString, strlen(ucOutputString));
            break;

		case eccHDBaseT: //A65_OPTOMA_Julie_0062
			utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, strlen(ucOutputString),(UINT8 *)ucOutputString);
			break;

        default:
            LOG_MSG(db_ALWAYS, "\r\n(CLI_StringOutput unknown channel %d)\r\n", eCh);
            break;
    }
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_Process
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
// 2018/11/23, Larry Create
// --------------------
// ==============================================================================
UINT8 utilOptoma_CLI_Process(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_OPTOMA_FORMAT* sCmdFormat, UINT16 uiType)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO;
    INT32 ProjectorID = 0;

    ucErrorCode = utilOptoma_CMD_Decode(pcString, sCmdFormat);

    LOG_MSG(db_APP_CLI, "%s(ch%d) ucErrorCode = %d, InputID = %d, ProjectorID = %d\r\n", __FUNCTION__, cCh, ucErrorCode, sCmdFormat->ulProjectorAddressID, ProjectorID);

    if(ucErrorCode == eCLI_ERROR_CODE_NO)
    {
        if(sCmdFormat->ucIsRead == WRITE_COMMAND)
        {
            if(uiType & CLI_EXECTUE_WRITE_CMD)
            {
                ucErrorCode = utilOptoma_CLI_Execute(cCh, pcString, pcReturnString, sCmdFormat);

                if(m_sOptomaLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_CHECK_BUSY)	//A35G2_Coda_0045
                {
                    m_ucWriteBusy = 0;
                }
            }
            else if(uiType & CLI_EXECTUE_WRITE_BUFFER)
            {
                if(m_ucWriteBusy)
                {
                    ucErrorCode = eCLI_ERROR_CODE_SETFAIL;
                    utilOptoma_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
                    __CMD_Respond(cCh, pcReturnString);

                    return ucErrorCode;
                }

                if(m_sOptomaLut[sCmdFormat->uiFuncID].uiSpecialFlag & CLI_COM_CHECK_BUSY)	//A35G2_Coda_0045
                {
                    m_ucWriteBusy = 1;
                }

                utilCommon_CLI_Callback(cCh, eCLI_STYLE_OPTOMA, pcString, strlen((char*)pcString));	//A35G2_Coda_0045
            }
        }
        else //read
        {
            if(uiType & CLI_EXECTUE_READ_CMD)
            {
                ucErrorCode = utilOptoma_CLI_Execute(cCh, pcString,pcReturnString, sCmdFormat);
            }
            else if(uiType & CLI_EXECTUE_READ_BUFFER)
            {
                utilCommon_CLI_Callback(cCh, eCLI_STYLE_OPTOMA, pcString, strlen((char*)pcString));	//A35G2_Coda_0045
            }
        }
    }
    else
    {
        utilOptoma_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);
    }


    //palDataMgr_Data_Access(edcLAST_SERIAL_CMD_ERRORCODE, edaWRITE_RAM_ONLY_NO_ACTION, (void *)&ucErrorCode); //save last serial cmd error code   //T100_Simon_0002

    return ucErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilOptoma_CLI_Execute
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
UINT8 utilOptoma_CLI_Execute(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_OPTOMA_FORMAT* sCmdFormat)
{
    UINT8 ucErrorCode = eCLI_ERROR_CODE_NO;

    utilOPD_CLI_EventSet((eOPD_CLI_RS232_LOG + cCh), (char*)pcString);

    //G100_Wilsonj_0050 Start
    if((sCmdFormat->ulProjectorAddressID == 0) ||
       (sCmdFormat->ulProjectorAddressID == m_ulAddressID))
    {
        if(ucErrorCode == eCLI_ERROR_CODE_NO)
        {
            ucErrorCode = utilOptoma_CMD_Handle(sCmdFormat);
        }

        LOG_MSG(db_APP_CLI, "(%s, %d) (%d)\r\n", __FUNCTION__, __LINE__, ucErrorCode);

        utilOptoma_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);

    	if(ucErrorCode == eCLI_ERROR_CODE_NO)
		{
            if((edcLENS_CALIBRATION == m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND)) //A70LV_Larry_0315
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_CAL_MSG, TRUE, NULL);
            }
            else if((edcLENS_APPLY_POSITION == m_sOptomaLut[sCmdFormat->uiFuncID].uiFunCodeID) && (sCmdFormat->ucIsRead == WRITE_COMMAND))
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_MENUOPEN_LENS_MOVING_MSG, TRUE, NULL);
            }
            else if(ucErrorCode == eCLI_ERROR_CODE_NO && sCmdFormat->ucIsRead == WRITE_COMMAND) //A70LV_Larry_0213
            {
                palEnvironment_AutoShutDownClear();	//A35G2_CDS_Coda_0034
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_CLI, TRUE, NULL);
            }
	    }
    }
	else // //A35G2_Coda_0098
    {
    	ucErrorCode = eCLI_ERROR_CODE_FORMATERR;
        utilOptoma_CMD_Reply(cCh, ucErrorCode, sCmdFormat, pcReturnString);
        __CMD_Respond(cCh, pcReturnString);
    }

    return ucErrorCode;
}

void utilOptoma_OutputInfoMsg(char *ucOutputString)
{
    utilOptoma_CLI_StringOutput(eccPC, ucOutputString);
    utilOptoma_CLI_StringOutput(eccTelnet, ucOutputString);
    utilOptoma_CLI_StringOutput(eccHDBaseT, ucOutputString);
}

void palSystem_Optoma_Error_Message(UINT32 ulErrorIndex)
{
	UINT8 cStatus;
	UINT8 cIndex = 0;
	char cString[16] = {0};

	switch(ulErrorIndex)
	{
		//H30K_Tim_0002, mark //case FanLock:
		case FanLock01:
		case FanLock02:
		case FanLock03:
		case FanLock04:
		case FanLock05:
		case FanLock06:
		case FanLock07:
		case FanLock08:
		case FanLock09:
        case FanLock10:                 //H30K_Tim_0002, add
			utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_FAN_LOCK].cINFO_String);

			cIndex = ulErrorIndex - FanLock;
			//LOG_MSG(db_ALWAYS,"(%s, %d)cIndex[%d]\n", __FUNCTION__, __LINE__, cIndex);

			if(cIndex > 0 && cIndex < 6)
			{
				sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_FAN_1_LOCK + cIndex -1].cINFO_String);
			}
			else if(cIndex >= 6 && cIndex <= 10)            //H30K_Tim_0002, mod, < 10
			{
				sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_FAN_6_LOCK + cIndex - 6].cINFO_String);
			}
			else
			{
				cString[0] = '\0';
			}
			utilOptoma_OutputInfoMsg(cString);

			cStatus = ePROSERVICE_TEMPERATURE_STATUS_RED;
			palDataMgr_Data_Access(edcPROSERVICE_FAN_STATUS, edaWRITE_THROUGH_WITH_ACTION, &cStatus);
			break;

		case FanStallError:
		case FanStallError01:
		case FanStallError02:
		case FanStallError03:
		case FanStallError04:
		case FanStallError05:
		case FanStallError06:
		case FanStallError07:
		case FanStallError08:
		case FanStallError09:
			sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_FAN_LOCK].cINFO_String);
			utilOptoma_OutputInfoMsg(cString);

			cStatus = ePROSERVICE_TEMPERATURE_STATUS_ORANGE;
			palDataMgr_Data_Access(edcPROSERVICE_FAN_STATUS, edaWRITE_THROUGH_WITH_ACTION, &cStatus);
			break;

        //H30K_Tim_0003, mod, ***
        case SystemOverTemp:            //H30K is used for triggering ambient over temp.
            sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_HIGH_AMBIENT_TEMPERATURE].cINFO_String);
            utilOptoma_OutputInfoMsg(cString);
            sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_OVER_TEMPERATURE_1].cINFO_String);
            utilOptoma_OutputInfoMsg(cString);

            cStatus = ePROSERVICE_TEMPERATURE_STATUS_RED;
            palDataMgr_Data_Access(edcPROSERVICE_TEMPERATURE_STATUS, edaWRITE_THROUGH_WITH_ACTION, &cStatus);
            break;

        case DMDOverTemp01:
            sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_OVER_TEMPERATURE_1].cINFO_String);
            utilOptoma_OutputInfoMsg(cString);

            cStatus = ePROSERVICE_TEMPERATURE_STATUS_RED;
            palDataMgr_Data_Access(edcPROSERVICE_TEMPERATURE_STATUS, edaWRITE_THROUGH_WITH_ACTION, &cStatus);
            break;

        case LightModuleOverTemp:
        case LightModuleOverTemp01:
        case LightModuleOverTemp02:
        case LightModuleOverTemp03:
        case LightModuleOverTemp04:
        case LightModuleOverTemp05:
        case LightModuleOverTemp06:
        case LightModuleOverTemp07: // 07 ~ 15 are not used for H30K.
        case LightModuleOverTemp08:
        case LightModuleOverTemp09:
        case LightModuleOverTemp10:
        case LightModuleOverTemp11:
        case LightModuleOverTemp12:
        case LightModuleOverTemp13:
        case LightModuleOverTemp14:
        case LightModuleOverTemp15:
            sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_LD_NTC_1_OVER_TEMPERATURE].cINFO_String);
			utilOptoma_OutputInfoMsg(cString);

			cStatus = ePROSERVICE_TEMPERATURE_STATUS_RED;
			palDataMgr_Data_Access(edcPROSERVICE_TEMPERATURE_STATUS, edaWRITE_THROUGH_WITH_ACTION, &cStatus);
			break;
        //H30K_Tim_0003, mod, ***

		case AmbientHighTemp:
		case SystemHighTemp:
		    sprintf(cString, "%s\r", sOptoma_System_Auto_Send_Lut[eOPT_HIGH_AMBIENT_TEMPERATURE].cINFO_String);
			utilOptoma_OutputInfoMsg(cString);

			cStatus = ePROSERVICE_TEMPERATURE_STATUS_ORANGE;
			palDataMgr_Data_Access(edcPROSERVICE_TEMPERATURE_STATUS, edaWRITE_THROUGH_WITH_ACTION, &cStatus);
			break;
	}
}

#endif //defined(CUSTOM_OPTOMA)

