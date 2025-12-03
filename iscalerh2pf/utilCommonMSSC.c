// ===============================================================================
// FILE NAME: utilGeneralMSSC.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================

//#include "appGui.h"
#include "appDataMgr.h"
#include "appDataItemAvailableMgr.h"
#include "appLANProcAPI.h"
#include "appSystem.h"

#include "utilDbgMsg.h"
#include "utilHostAPI.h"
#include "utilCommonMSSCAPI.h"
#include "utilDataMapping.h"
#include "utilDatabaseAPI.h"
#include "utilHPBU_Tester.h"

#include "halMCUCtrlAPI.h"
#include "halLDProc.h"
#include "halGui.h"  //A35G2_Simon_0116
#include "halFanCtrlAPI.h"

#include "palGui.h"
#include "palFormatterMgr.h"


extern sGUI_CALLBACK GuiCb;
fpCLISTRINGCALLBACK fpCLICallback = NULL;
static sCLI_DATA_INFO m_sLD_PWM_Info = {0};

UINT8 __CheckCmd(char *cSrcA, char *cDest, UINT8 cuLens)
{
    // transfor uper char.
    char cSrc;
    UINT8 len;

    if(cSrcA == NULL || cDest == NULL)
    {
        return 0;
    }

    if(strlen(cSrcA) < cuLens || strlen(cDest) < cuLens)
        return 0;

    for(len = 0; len < cuLens; len++)
    {
        if(cSrcA[len] >= 97 && cSrcA[len] <= 122)
        {
            cSrc = cSrcA[len] - 32;
        }
        else
        {
            cSrc = cSrcA[len];
        }
        if(cSrc != cDest[len])
            return 0;
    }

    return 1;
}

// ==============================================================================
// FUNCTION NAME: __CheckNum
// DESCRIPTION:
//
//
// Params:
// char cCharNum:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/08, Larry Create
// --------------------
// ==============================================================================
UINT8 __CheckNum(char cCharNum)
{
    if(cCharNum >= '0' && cCharNum <= '9')
    {
        return 1;
    }
    return 0;
}

// ==============================================================================
// FUNCTION NAME: __CheckText
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
UINT8 __CheckText(char cCharNum)
{
    if((cCharNum >= '0' && cCharNum <= '9') ||
       (cCharNum >= 'a' && cCharNum <= 'z') ||
       (cCharNum >= 'A' && cCharNum <= 'Z') ||cCharNum == '-')//H30K_David_0041
    {
        return 1;
    }
    return 0;
}

// ==============================================================================
// FUNCTION NAME: __CheckAsciiCharacter
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
UINT8 __CheckAsciiCharacter(char cCharNum)
{// BruceLin#20210114
    if(cCharNum >= ' ' && cCharNum < 0x7F)
    {
        return 1;
    }
    return 0;
}

// ==============================================================================
// FUNCTION NAME: __CMD_Respond
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
void __CMD_Respond(UINT8 ucCh, char *pstring)
{
    if(ucCh > eccNumber)
        return;

    switch(ucCh)
    {
        case eccPC:
            LOG_MSG(db_ALWAYS, "%s", pstring);
            break;

        case eccHDBaseT:
            {
                char cTemp[1024] = {'\0'};

                snprintf(cTemp, 1023, "%s" , pstring);

                utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, strlen(cTemp) + 1, cTemp);
            }
            break;

        case eccTelnet:
            palLANProcTelnetReply((UINT8 *)pstring, strlen(pstring)+1); //A35G2_BRC_Casper_0083 //A35G2_BRC_Casper_0092
            break;

        default:
            LOG_MSG(db_ALWAYS, "\r\n(CLI_StringOutput unknown channel %d)\r\n", ucCh);
            break;
    }
}

// ==============================================================================
// FUNCTION NAME: Low2UpCase
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
void Low2UpCase(UINT8 *pcData)
{
    if((*pcData >= 'a') && (*pcData <= 'z'))
    {
        *pcData = *pcData - 'a' + 'A';
    }
}

// ==============================================================================
// FUNCTION NAME: Hex2Bin
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
eRESULT Hex2Bin(UINT8 *pcData)
{
    Low2UpCase(pcData);

    if((*pcData >= '0') && (*pcData <= '9'))
    {
        *pcData = *pcData - '0';
    }
    else if((*pcData >= 'A') && (*pcData <= 'F'))
    {
        *pcData = *pcData - 'A' + 10;
    }
    else
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

    return rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: AscIIToHex
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
eRESULT AscIIToHex(UINT8 ucHighByte, UINT8 ucLowByte, UINT8 *pucData)
{
   eRESULT eResult = rcERROR;

   if((Hex2Bin(&ucHighByte) == rcSUCCESS) && (Hex2Bin(&ucLowByte) == rcSUCCESS))
   {
       *pucData = (ucHighByte << 4) | ucLowByte;
       eResult = rcSUCCESS;
   }

   return eResult;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_DataControl
// DESCRIPTION:
//
//
// Params:
// UINT16 uiDataCode:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/09, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_DataControl(UINT16 uiDataCode)
{
	return palDataMgr_DataCode_Control((eDATA_CODE)uiDataCode);
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ControlFunction
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
// 2024/09/04, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_ControlFunction(UINT16 uiControl)
{
	return palDataMgr_MenuItem_Control((eDATA_CONTROL)uiControl);
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_DataTypeGet
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
UINT8 utilCommonCLI_DataTypeGet(UINT16 uiDataCode)
{
    UINT8 ucType = 0;

    if(palDataMgr_Data_Type((eDATA_CODE)uiDataCode) == DATA_TYPE_STRING)
    {
        ucType = DATATYPE_STRING;
    }
    else
    {
        ucType = DATATYPE_DIGIT;
    }

    return ucType;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_String_Get
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_String_Get(UINT16 uiDataCode, UINT8 *pucString)
{
    return (INT8)palDataMgr_Data_Access((eDATA_CODE)uiDataCode, edaREAD, pucString);
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_String_Set
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_String_Set(UINT16 uiDataCode, UINT8 *pucString)
{
    return (INT8)palDataMgr_Data_Access((eDATA_CODE)uiDataCode, edaWRITE_THROUGH_WITH_ACTION, pucString);
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_DataConversionGet
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_DataConversionGet(UINT16 uiDataCode, INT32 *piValue)
{
    INT8 cResult = eEXEC_CODE_PASS;
    INT32 iValue = 0;
    INT32 iValue2 = 0;

    cResult = palDataMgr_Data_Access(uiDataCode, edaREAD, &iValue2); //GuiCb.fpGui_DataCode_Value_GetCb(uiDataCode, &iValue2);

    if((eEXEC_CODE_PASS == cResult) && IS_COMMON_DATA_CODE(uiDataCode))
    {
        UINT8 ucDataType = palDataMgr_Data_Type((eDATA_CODE)uiDataCode);

        iValue = CM2CLI(uiDataCode, iValue2);

        switch(ucDataType)
        {
            case DATA_TYPE_UI_DIGIT_8:
               *piValue = (UINT8)iValue;
                break;

            case DATA_TYPE_UI_DIGIT_16:
                *piValue = (UINT16)iValue;
                break;

            case DATA_TYPE_UI_DIGIT_32:
                *piValue = (UINT32)iValue;
                break;

            case DATA_TYPE_UI_DIGIT_64:
                *piValue = (UINT64)iValue;
                break;

            case DATA_TYPE_I_DIGIT_8:
                *piValue = (INT8)iValue;
                break;

            case DATA_TYPE_I_DIGIT_16:
                *piValue= (INT16)iValue;
                break;

            case DATA_TYPE_I_DIGIT_32:
                *piValue = (INT32)iValue;
                break;

            case DATA_TYPE_I_DIGIT_64:
                *piValue = (INT64)iValue;
                break;


            case DATA_TYPE_NA:
            case DATA_TYPE_STRING:
            case DATA_TYPE_FLOAT:
            default:
                break;
        }
    }
    else
    {
        *piValue = iValue2;
    }

    return cResult;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_DataConversionSet
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
eEXEC_CODE utilCommonCLI_DataConversionSet(UINT16 uiDataCode, INT32 iValue)
{
    eEXEC_CODE cResult = eEXEC_CODE_PASS;

    if(IS_COMMON_DATA_CODE(uiDataCode))
    {
        iValue = CLI2CM(uiDataCode, iValue);

        if( iValue < 0 )
        {
            return eEXEC_CODE_FAIL;
        }
    }
    cResult = palDataMgr_Data_Access(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue); //GuiCb.fpGui_DataCode_Value_SetCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, iValue);


    return cResult;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_RangeCheck
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_RangeCheck(UINT16 uiDataCode, INT32 iValue)
{
    INT32   iMax = 0;
    INT32   iMin = 0;

    if(IS_COMMON_DATA_CODE(uiDataCode))
    {
        iValue = CLI2CM(uiDataCode, iValue);

        if(utilDataMapping_CMValueRangeCheck(uiDataCode, iValue) == DATA_RANGE_CHECK_PASS)
        {
            return eEXEC_CODE_PASS;
        }
    }
    else
    {
        if((eEXEC_CODE_PASS == palDataMgr_Data_Range_Get((eDATA_CODE)uiDataCode, edrMAX, &iMax)) &&
           (eEXEC_CODE_PASS == palDataMgr_Data_Range_Get((eDATA_CODE)uiDataCode, edrMIN, &iMin)))
        {
            if((iMax >= (INT32)iValue) && ((INT32)iValue >= iMin))
            {
                return eEXEC_CODE_PASS;
            }
        }
    }

    return eEXEC_CODE_FAIL;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_Increase_Data
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_Increase_Data(UINT16 uiDataCode)
{
    INT8 cResult = eEXEC_CODE_FAIL;

    INT32 iSetting = 0;
    INT32 iMax = 0;

    cResult = palDataMgr_Data_Access(uiDataCode, edaREAD, &iSetting); //GuiCb.fpGui_DataCode_Value_GetCb(uiDataCode, &iSetting);


    if(cResult == eEXEC_CODE_PASS)
    {
        if(eEXEC_CODE_PASS == palDataMgr_Data_Range_Get((eDATA_CODE)uiDataCode, edrMAX, &iMax))
        {
            iSetting++;
            if(iMax <= iSetting)
            {
                iSetting = iMax;
            }

            cResult = palDataMgr_Data_Access(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iSetting); //GuiCb.fpGui_DataCode_Value_SetCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, iSetting);

            return eEXEC_CODE_PASS;
        }
    }

    return eEXEC_CODE_FAIL;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_Decrease_Data
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
// 2021/03/24, Larry Create
// --------------------
// ==============================================================================
INT8 utilCommonCLI_Decrease_Data(UINT16 uiDataCode)
{
    INT8 cResult = eEXEC_CODE_FAIL;

    INT32 iSetting = 0;
    INT32 iMin = 0;

    cResult = palDataMgr_Data_Access(uiDataCode, edaREAD, &iSetting); //GuiCb.fpGui_DataCode_Value_GetCb(uiDataCode, &iSetting);

    if(cResult == eEXEC_CODE_PASS)
    {
        if(eEXEC_CODE_PASS == palDataMgr_Data_Range_Get((eDATA_CODE)uiDataCode, edrMIN, &iMin))
        {
            iSetting--;
            if(iSetting <= iMin)
            {
                iSetting = iMin;
            }

            cResult = palDataMgr_Data_Access(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iSetting); //GuiCb.fpGui_DataCode_Value_SetCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, iSetting);

            return eEXEC_CODE_PASS;
        }
    }

    return eEXEC_CODE_FAIL;
}

// ==============================================================================
// FUNCTION NAME: utilCommon_Callback_fun
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
void utilCommon_Callback_fun(fpCLISTRINGCALLBACK fpCallBack)
{
    fpCLICallback = fpCallBack;
}

void utilCommon_Init(void)
{
    GuiCb = Gui_fpCallbackGet();
}

// ==============================================================================
// FUNCTION NAME: utilCommon_CLI_Callback
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
void utilCommon_CLI_Callback(UINT8 cCh, UINT8 cStyle, UINT8 * pcBuffer, UINT16 uiSize)//A35G2_Coda_0045
{
    if(fpCLICallback != NULL)
        fpCLICallback(cCh, cStyle, pcBuffer, uiSize);
}

#if defined(SCALER_C821_C789)
extern BOOL dvC821_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);
extern UINT32 dvC821_Read(const UINT32 ulAddr, const UINT8 ucBankOffset);
extern void dvC821_Write(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);

extern BOOL dvC789_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);
extern UINT32 dvC789_Read(UINT32 ulAddr);
extern void dvC789_Write(UINT32 ulAddr, UINT32 ulData);
#elif defined(SCALER_C341)
extern BOOL dvC341_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);
extern UINT32 dvC341_Read(const UINT32 ulAddr, const UINT8 ucBankOffset);
extern void dvC341_Write(const UINT32 ulAddr, UINT32 ulData, const UINT8 ucBankOffset);
#endif

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_SYSDBMK
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
eCLI_ERROR_CODE utilCommonCLI_SYSDBMK(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    char  ucOutputString[128] = {""};

    if(sCmdFormat->eAccessMode == ecmRead)
    {
        sprintf(ucOutputString, "01 APP_SYSTEM          %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_SYSTEM));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "02 APP_DATAPATH        %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_DATAPATH));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "03 APP_SCALER          %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_SCALER));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "04 APP_MAILBOX         %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_MAILBOX));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "05 db_APP_POLL         %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_POLL));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "06 db_APP_ENVIRONMENT  %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_ENVIRONMENT));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "07 db_APP_GUI          %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_GUI));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "08 db_APP_ILLUMINATION %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_ILLUMINATION));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "09 db_APP_DATA_MGR     %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_DATA_MGR));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "10 db_APP_CORE         %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_CORE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "11 db_APP_CLI          %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_CLI));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "12 db_APP_ETHERNET     %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_ETHERNET));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "13 db_APP_INPUTKEY     %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_INPUTKEY));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "14 db_APP_SCHEDULE     %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_SCHEDULE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "15 db_APP_MONITOR      %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_MONITOR));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "16 db_APP_LAN          %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_LAN));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "17 db_APP_CLI_LAN      %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_CLI_LAN));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "18 db_APP_ACK_LAN      %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_ACK_LAN));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "19 db_APP_TASK         %d\r\n",    (BOOL)utilDbgMsg_Get(db_APP_TASK));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "21 db_HAL_SCALER       %d\r\n",    (BOOL)utilDbgMsg_Get(db_HAL_SCALER));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "22 db_HAL_EEP          %d\r\n",    (BOOL)utilDbgMsg_Get(db_HAL_EEP));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "23 db_HAL_WARPING      %d\r\n",    (BOOL)utilDbgMsg_Get(db_HAL_WARPING));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "24 db_HAL_GUI          %d\r\n",    (BOOL)utilDbgMsg_Get(db_HAL_GUI));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "25 db_HAL_IR           %d\r\n",    (BOOL)utilDbgMsg_Get(db_HAL_IR));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "26 db_HAL_FORMATTER    %d\r\n",    (BOOL)utilDbgMsg_Get(db_HAL_FORMATTER));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "27 db_HAL_LD           %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_LD));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "28 db_HAL_FAN          %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_FAN));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "29 db_HAL_MCU          %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_MCU));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "30 db_HAL_GUI_CK       %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_GUI_CK));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "31 db_HAL_SOURCE_INFO  %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_SOURCE_INFO));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "32 db_HAL_PROAV        %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_PROAV));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "33 db_HAL_PROAV_GUI    %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_PROAV_GUI));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "34 db_HAL_HDMIRX       %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_HDMIRX));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "35 db_HAL_HDMIRX       %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_SEMAPHORE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "36 db_HAL_HDMIRX       %d\r\n",     (BOOL)utilDbgMsg_Get(db_HAL_LENS));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "41 db_DV_SEMAPHORE     %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_SEMAPHORE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "44 db_DV_PROAV_OSD     %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_PROAV_OSD));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "45 db_DV_VCXO       %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_VCXO));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "46 db_DV_I2C           %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_I2C));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "47 db_DV_PROAV_WARPING %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_PROAV_WARPING));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "48 db_DV_FAN           %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_FAN));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "49 db_DV_SCALER        %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_SCALER));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "50 db_DV_SCALER_OSD    %d\r\n",    (BOOL)utilDbgMsg_Get(db_DV_SCALER_OSD));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "51 db_UTL_CLI          %d\r\n",    (BOOL)utilDbgMsg_Get(db_UTL_CLI));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "52 db_DV_3D_CONFIG     %d\r\n",    (BOOL)utilDbgMsg_Get(db_DV_3D_CONFIG));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "53 db_DV_GEO          %d\r\n",     (BOOL)utilDbgMsg_Get(db_DV_GEO));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "54 db_UPGRADE          %d\r\n",    (BOOL)utilDbgMsg_Get(db_UPGRADE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "55 db_UTL_CMD          %d\r\n",     (BOOL)utilDbgMsg_Get(db_UTL_CMD));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "56 db_ADC_CALIBARATION %d\r\n",     (BOOL)utilDbgMsg_Get(db_ADC_CALIBARATION));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "57 db_UTL_DATABASE     %d\r\n",     (BOOL)utilDbgMsg_Get(db_UTL_DATABASE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "58 db_IPC              %d\r\n",     (BOOL)utilDbgMsg_Get(db_IPC));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "59 db_IPC_REV          %d\r\n",     (BOOL)utilDbgMsg_Get(db_IPC_REV));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "60 db_TRACE_CODE       %d\r\n",     (BOOL)utilDbgMsg_Get(db_TRACE_CODE));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "61 db_BIST             %d\r\n",     (BOOL)utilDbgMsg_Get(db_BIST));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "62 db_KEEP_UART_SW     %d\r\n",     (BOOL)utilDbgMsg_Get(db_KEEP_UART_SW));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        sprintf(ucOutputString, "63 db_ASSERT           %d\r\n",     (BOOL)utilDbgMsg_Get(db_ASSERT));
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

        palDataMgr_UI_EventSend(edcUI_EVENT_DBMSG, TRUE, NULL);
    }
    else if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT64 ulMask = 0;

        if(sCmdFormat->lData > 99 && sCmdFormat->lData < 164)
        {
            INT32 ulGuiMask = sCmdFormat->lData - 100;

            ulMask = ((UINT64)0x01 << ulGuiMask);

            if((BOOL)utilDbgMsgCT_Get(ulMask))
            {
                utilDbgMsgCT_Set(ulMask, 0);
                sprintf(ucOutputString, "%d CT Disable\r\n", sCmdFormat->lData);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
            else
            {
                utilDbgMsgCT_Set(ulMask, 1);
                sprintf(ucOutputString, "%d CT Enable\r\n", sCmdFormat->lData);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }

            ulMask = utilDbgMsgCT_Get(db_ALL);
            palDataMgr_Data_Access(edcUI_DEBUG_MASK, edaWRITE_THROUGH_WITH_ACTION, (UINT8 *)&ulMask);
        }
        else if(sCmdFormat->lData == 199)
        {
            ulMask = utilDbgMsgCT_Get(db_ALL);

            if(ulMask)
            {
                utilDbgMsgCT_Set(db_ALL, 0);
                ulMask = 0;
                sprintf(ucOutputString, "All CT Disable\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
            else
            {
                utilDbgMsgCT_Set(db_ALL, 1);
                ulMask = db_ALL;
                sprintf(ucOutputString, "All CT Enable\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
            palDataMgr_Data_Access(edcUI_DEBUG_MASK, edaWRITE_THROUGH_WITH_ACTION, (UINT8 *)&ulMask); //GuiCb.fpGui_DataCode_PointerVar_SetCb(edcDEBUG_MASK, (UINT8 *)&ulMask);
        }
        else if(sCmdFormat->lData == 99)
        {
            ulMask = utilDbgMsg_Get(db_ALL);

            if(ulMask)
            {
                utilDbgMsg_Set(db_ALL, 0);
                ulMask = 0;
                sprintf(ucOutputString, "All Disable\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
            else
            {
                utilDbgMsg_Set(db_ALL, 1);
                ulMask = db_ALL;
                sprintf(ucOutputString, "All Enable\r\n");
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
            palDataMgr_Data_Access(edcDEBUG_MASK, edaWRITE_THROUGH_WITH_ACTION, (UINT8 *)&ulMask); //GuiCb.fpGui_DataCode_PointerVar_SetCb(edcDEBUG_MASK, (UINT8 *)&ulMask);
        }
        else if(sCmdFormat->lData == 98)
        {
            utilDbgMsg_Set(db_ALL, 0);
            ulMask = 0;
            sprintf(ucOutputString, "All Disable\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

            palDataMgr_Data_Access(edcDEBUG_MASK, edaWRITE_THROUGH_WITH_ACTION, (UINT8 *)&ulMask); //GuiCb.fpGui_DataCode_PointerVar_SetCb(edcDEBUG_MASK, (UINT8 *)&ulMask);
        }
        else if(sCmdFormat->lData == 97)
        {
            utilDbgMsg_Set(db_ALL, 1);
            ulMask = db_ALL;
            sprintf(ucOutputString, "All Enable\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);

            palDataMgr_Data_Access(edcDEBUG_MASK, edaWRITE_THROUGH_WITH_ACTION, (UINT8 *)&ulMask); //GuiCb.fpGui_DataCode_PointerVar_SetCb(edcDEBUG_MASK, (UINT8 *)&ulMask);
        }
        else if(sCmdFormat->lData == 88)
        {
            //halScaler_InputSelectSet(0, 1, FALSE);
        }
        else if(sCmdFormat->lData == 87)
        {
            //halScaler_InputSelectSet(0, 0, FALSE);
        }
        else if(sCmdFormat->lData < 64)
        {
            ulMask = ((UINT64)0x01 << sCmdFormat->lData);

            if((BOOL)utilDbgMsg_Get(ulMask))
            {
                utilDbgMsg_Set(ulMask, 0);
                sprintf(ucOutputString, "%d Disable\r\n", sCmdFormat->lData);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }
            else
            {
                utilDbgMsg_Set(ulMask, 1);
                sprintf(ucOutputString, "%d Enable\r\n", sCmdFormat->lData);
                __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
            }

            ulMask = utilDbgMsg_Get(db_ALL);
            palDataMgr_Data_Access(edcDEBUG_MASK, edaWRITE_THROUGH_WITH_ACTION, (UINT8 *)&ulMask); //GuiCb.fpGui_DataCode_PointerVar_SetCb(edcDEBUG_MASK, (UINT8 *)&ulMask);
        }
        else
        {
            eErrorCode = eCLI_ERROR_CODE_SETFAIL;
        }
    }

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICP
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
eCLI_ERROR_CODE utilCommonCLI_ICP(sCLI_COMMON_FORMAT* sCmdFormat)
{
#if defined(SCALER_C821_C789) || defined(SCALER_C341)

    char  ucOutputString[128] = {""};

    if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
    {
        UINT8 ucBank = 0;
        UINT8 ucReg = 0;
        UINT8 ucData = 0;
        UINT8 aucData[3] = {0};
        UINT32 ulAddr = 0;
        UINT32 ulData = 0;

        if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '0')//C821 Read
        {
            sscanf(sCmdFormat->cTextString, "%*[^,],%hhx,%hhx", &ucBank, &ucReg);
            ulAddr = ucBank<<16 | ucReg << 8 | 0x01;

#if defined(SCALER_C821_C789)
            if(dvC821_SemaphoreTake(TRUE, __FUNCTION__))
            {
                ulData = dvC821_Read(ulAddr, 0);
                dvC821_SemaphoreTake(FALSE, __FUNCTION__);
            }
#elif defined(SCALER_C341)
            if(dvC341_SemaphoreTake(TRUE, __FUNCTION__))
            {
                ulData = dvC341_Read(ulAddr, 0);
                dvC341_SemaphoreTake(FALSE, __FUNCTION__);
            }
#endif

            sprintf(ucOutputString, "C821 Addr 0x%06x Get 0x%08x\r\n", ulAddr, ulData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else if(sCmdFormat->cTextString[0] == 'W' && sCmdFormat->cTextString[1] == '0') //C821 Write
        {
            sscanf(sCmdFormat->cTextString, "%*[^,],%hhx,%hhx,%hhx", &ucBank, &ucReg, &ucData);
            ulAddr = ucBank<<16 | ucReg << 8 | 0x01;

#if defined(SCALER_C821_C789)
            if(dvC821_SemaphoreTake(TRUE, __FUNCTION__))
            {
                dvC821_Write(ulAddr, (UINT32)ucData, 0);
                dvC821_SemaphoreTake(FALSE, __FUNCTION__);
            }
#elif defined(SCALER_C341)
            if(dvC341_SemaphoreTake(TRUE, __FUNCTION__))
            {
                dvC341_Write(ulAddr, (UINT32)ucData, 0);
                dvC341_SemaphoreTake(FALSE, __FUNCTION__);
            }
#endif


            sprintf(ucOutputString, "C821 Addr 0x%06x Set %08x\r\n", ulAddr, ucData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '1')//C789 Read
        {
            sscanf(sCmdFormat->cTextString, "%*[^,],%hhx,%hhx", &ucBank, &ucReg);
            ulAddr = ucBank<<16 | ucReg << 8 | 0x01;

#ifdef SCALER_C821_C789
            if(dvC789_SemaphoreTake(TRUE, __FUNCTION__))
            {
                ulData = dvC789_Read(ulAddr);
                dvC789_SemaphoreTake(FALSE, __FUNCTION__);
            }
#endif

            sprintf(ucOutputString, "C789 Addr 0x%06x Get 0x%08x\r\n", ulAddr, ulData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else if(sCmdFormat->cTextString[0] == 'W' && sCmdFormat->cTextString[1] == '1') //C789 Write
        {
            //ucReg
            sscanf(sCmdFormat->cTextString, "%*[^,],%hhx,%hhx,%hhx", &ucBank, &ucReg, &ucData);
            ulAddr = ucBank<<16 | ucReg << 8 | 0x01;

#ifdef SCALER_C821_C789
            if(dvC789_SemaphoreTake(TRUE, __FUNCTION__))
            {
                dvC789_Write(ulAddr, (UINT32)ucData);
                dvC789_SemaphoreTake(FALSE, __FUNCTION__);
            }
#endif
            sprintf(ucOutputString, "C789 Addr 0x%06x Set %08x\r\n", ulAddr, ucData);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "ichip comamnd fail!!\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
    }
    else
    {
        sprintf(ucOutputString, "comamnd fail!!\r\n");
        __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
    }
#endif

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICPC821_SET
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
// 2023/02/15, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_ICPC821_SET(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#if defined(SCALER_C821_C789)
    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 regEnum;
        UINT32 data;
        sscanf(sCmdFormat->cTextString, "%x,%x", &regEnum, &data);

        if(dvC821_SemaphoreTake(TRUE, __FUNCTION__))
        {
            dvC821_Write(regEnum, data, 0);
            dvC821_SemaphoreTake(FALSE, __FUNCTION__);
        }
    }
#endif /* #if defined(SCALER_C821_C789) || defined(SCALER_C341) */

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICPC821_GET
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
// 2023/02/15, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_ICPC821_GET(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#if defined(SCALER_C821_C789)
    char  ucOutputString[64] = {0};

    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 data = 0;
        UINT32 regEnum;

        sscanf(sCmdFormat->cTextString, "%x", &regEnum);

        if(dvC821_SemaphoreTake(TRUE, __FUNCTION__))
        {
            data = dvC821_Read(regEnum, 0);
            dvC821_SemaphoreTake(FALSE, __FUNCTION__);
        }

        sprintf(sCmdFormat->cTextString, "\"%x,%08x\"", regEnum, data);
    }
#endif /* #if defined(SCALER_C821_C789) */

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICPC821_SET
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
// 2023/02/15, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_ICPC789_SET(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#if defined(SCALER_C821_C789)
    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 regEnum;
        UINT32 data;
        sscanf(sCmdFormat->cTextString, "%x,%x", &regEnum, &data);

        if(dvC789_SemaphoreTake(TRUE, __FUNCTION__))
        {
            dvC789_Write(regEnum, data);
            dvC789_SemaphoreTake(FALSE, __FUNCTION__);
        }
    }
#endif /* #if defined(SCALER_C821_C789) || defined(SCALER_C341) */

    return eErrorCode;
}


// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICPC821_GET
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
// 2023/02/15, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_ICPC789_GET(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#if defined(SCALER_C821_C789)
    char  ucOutputString[64] = {0};

    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 data = 0;
        UINT32 regEnum;

        sscanf(sCmdFormat->cTextString, "%x", &regEnum);

        if(dvC789_SemaphoreTake(TRUE, __FUNCTION__))
        {
            data = dvC789_Read(regEnum);
            dvC789_SemaphoreTake(FALSE, __FUNCTION__);
        }

        sprintf(sCmdFormat->cTextString, "\"%x,%08x\"", regEnum, data);
    }
#endif /* #if defined(SCALER_C821_C789) || defined(SCALER_C341) */

    return eErrorCode;
}

// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICPC341_SET
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
// 2024/01/19, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_ICPC341_SET(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#ifdef SCALER_C341
    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 regEnum;
        UINT32 data;
        sscanf(sCmdFormat->cTextString, "%x,%x", &regEnum, &data);

        if(dvC341_SemaphoreTake(TRUE, __FUNCTION__))
        {
            dvC341_Write(regEnum, data, 0);
            dvC341_SemaphoreTake(FALSE, __FUNCTION__);
        }
    }
#endif

    return eErrorCode;
}


// ==============================================================================
// FUNCTION NAME: utilCommonCLI_ICPC341_GET
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
// 2024/01/19, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_ICPC341_GET(sCLI_COMMON_FORMAT* sCmdFormat)
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#ifdef SCALER_C341
    char  ucOutputString[64] = {0};

    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 data = 0;
        UINT32 regEnum;

        sscanf(sCmdFormat->cTextString, "%x", &regEnum);

        if(dvC341_SemaphoreTake(TRUE, __FUNCTION__))
        {
            data = dvC341_Read(regEnum, 0);
            dvC341_SemaphoreTake(FALSE, __FUNCTION__);
        }

        sprintf(sCmdFormat->cTextString, "\"%x,%08x\"", regEnum, data);
    }
#endif

    return eErrorCode;
}

#ifdef SCALER_FPGA_F34
extern int dvProAV_AccessRaw_Read(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data);
extern int dvProAV_AccessRaw_Write(UINT8 bank, UINT8 offset, UINT16 len, UINT8 *data);
extern int dvProAV_Access_Write(UINT32 regEnum, UINT32 data);
extern int dvProAV_Access_Read(UINT32 regEnum, UINT32 *data);
#endif

eCLI_ERROR_CODE utilCommonCLI_PRO(sCLI_COMMON_FORMAT* sCmdFormat)
{
   // BYTE  aucData[128] = {0};
#ifdef SCALER_FPGA_F34
    if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_STRING)
    {
        UINT8 ucBank = 0;
        UINT8 ucReg = 0;
        UINT8 ucData = 0;
        UINT8 ucMask = 0;
        UINT8 aucData[256] = {0};
        //UINT32 ulAddr = 0;
        UINT32 ulData = 0;

        //ulAddr = ucBank<<16 | ucReg << 8 | 0x01;

        if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '0')
        {
            sscanf(sCmdFormat->cTextString, "%*[^,],%x", &ucBank);

            switch(ucBank)
            {
                case 0x00:
                case 0x01:
                case 0x30:
                case 0x02:
                case 0x03:
                case 0x05:
                case 0x07:
                case 0x08:
                case 0x09:
                case 0x0A:
                case 0x0B:
                case 0x0C:
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x21:
                case 0x70:
                case 0x71:
                case 0x72:
                case 0x73:
                case 0x74:
                case 0x75:
                case 0x80:
                case 0x81:
                case 0x82:
                case 0x85:
                case 0x86:
                case 0x87:
                case 0x83:
                case 0x20:
                case 0x84:
                case 0x90:
                case 0x91:
                case 0x92:
                case 0x93:
                case 0x94:
                case 0x95:
                case 0xA0:
                case 0xB0:
                    dvProAV_AccessRaw_Read(ucBank, 0, 128, &aucData[0]);

                    LOG_MSG(db_ALWAYS, "   \n===========================Bank 0x%02x==========================\r\n",ucBank);
                    ulData = 0;

                    LOG_MSG(db_ALWAYS, "   [00][01][02][03][04][05][06][07][08][09][0A][0B][0C][0D][0E][0F]\r\n");

                    for(ucBank = 0; ucBank  < 8; ucBank++)
                    {
                        MS_SLEEP(10);
                        LOG_MSG(db_ALWAYS, "%d0-[%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X]\r\n",
                            ucBank,
                            aucData[0+ulData],
                            aucData[1+ulData],
                            aucData[2+ulData],
                            aucData[3+ulData],
                            aucData[4+ulData],
                            aucData[5+ulData],
                            aucData[6+ulData],
                            aucData[7+ulData],
                            aucData[8+ulData],
                            aucData[9+ulData],
                            aucData[10+ulData],
                            aucData[11+ulData],
                            aucData[12+ulData],
                            aucData[13+ulData],
                            aucData[14+ulData],
                            aucData[15+ulData]
                            ); //A70LK_Casper_0001
                            ulData += 16; //A70LK_Casper_0001
                    }
                    return eCLI_ERROR_CODE_NO;
            }

            LOG_MSG(db_ALWAYS, "Bank 0x%02x not define\r\n", ucBank);

            return eCLI_ERROR_CODE_NO;

        }
        else if(sCmdFormat->cTextString[0] == 'R' && sCmdFormat->cTextString[1] == '1')     // ProAV_Rex_0060
        {
            sscanf(sCmdFormat->cTextString, "%*[^,],%x,%x", &ucBank, &ucReg);

            dvProAV_AccessRaw_Read(ucBank, ucReg, 1, &ucData);

            LOG_MSG(db_ALWAYS, "Read Bank[0x%02x]Reg[0x%02x]Data[0x%02x] \r\n", ucBank, ucReg ,ucData);
            return eCLI_ERROR_CODE_NO;
        }
        else if(sCmdFormat->cTextString[0] == 'W' && sCmdFormat->cTextString[1] == '0')
        {
            ucMask = 0xFF;

            sscanf(sCmdFormat->cTextString, "%*[^,],%x,%x,%x,%x", &ucBank, &ucReg, &ucData, &ucMask);

            if(ucMask != 0xFF)
            {
                UINT32 Temp = 0x81000000;

                Temp = Temp | ((ucBank << 16) & 0xFF0000) | ((ucReg << 8) & 0xFF00) | (ucMask & 0xFF);

                dvProAV_Access_Write(Temp, (uint32)ucData);
            }
            else
            {
                dvProAV_AccessRaw_Write(ucBank, ucReg, 1, &ucData);
            }

            LOG_MSG(db_ALWAYS, "Write Bank[0x%02x]Reg[0x%02x]Data[0x%02x]Mask[0x%02x]\r\n", ucBank, ucReg ,ucData, ucMask);
            return eCLI_ERROR_CODE_NO;

        }
    }
    else if(sCmdFormat->ucDataType == eCLI_DATA_TYPE_DONTCARE)  // '?'
    {
        LOG_MSG(db_ALWAYS, "[prog\"W0 ucBank ucReg ucData\"]\n");
        LOG_MSG(db_ALWAYS, "[prog\"R1 ucBank ucReg\"]\n");
    }
#endif

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilCommonCLI_PROSETT(sCLI_COMMON_FORMAT* sCmdFormat)
{
eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#ifdef SCALER_FPGA_F34
    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 regEnum;
        UINT32 data;
        sscanf(sCmdFormat->cTextString, "%x,%x", &regEnum, &data);

        dvProAV_Access_Write((regEnum | 0x80000000), data);
    }
#endif /* 0 */

return eErrorCode;
}

eCLI_ERROR_CODE utilCommonCLI_PROGETT(sCLI_COMMON_FORMAT* sCmdFormat)
{
eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;

#ifdef SCALER_FPGA_F34
    char  ucOutputString[64] = {0};

    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 data = 0;
        UINT32 regEnum;

        sscanf(sCmdFormat->cTextString, "%x", &regEnum);

        dvProAV_Access_Read((regEnum | 0x40000000), &data);

        sprintf(sCmdFormat->cTextString, "\"%x,%08x\"", regEnum, data);
    }
#endif /* 0 */

return eErrorCode;
}


// ==============================================================================
// FUNCTION NAME: utilCommon_BODCUST
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
// 2021/08/20, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_BODCUST(sCLI_COMMON_FORMAT* sCmdFormat)
{
    if(sCmdFormat->eAccessMode == ecmRead)
    {
#ifdef CUSTOM_CHRISTIE
        sCmdFormat->lData = CUSTOMER_ID_CHRISTIE;
#elif defined(CUSTOM_BARCO)
        sCmdFormat->lData = CUSTOMER_ID_BARCO;
#elif defined(CUSTOM_OPTOMA)
        sCmdFormat->lData = CUSTOMER_ID_OPTOMA;
#else //TBD
        sCmdFormat->lData = 0xFF;
#endif /* 0 */
    }
    else if(sCmdFormat->eAccessMode == ecmWrite)
    {
        BYTE cCustom = 0;

        if(strcmp("custom=christie", sCmdFormat->cTextString)==0)
        {
            cCustom = CUSTOMER_ID_CHRISTIE;
            halMCU_CustomerID_Set(cCustom);
            halLDCtrl_CustomerID_Set(cCustom);
        }
        else if(strcmp("custom=optoma", sCmdFormat->cTextString)==0)
        {
            cCustom = CUSTOMER_ID_OPTOMA;
            halMCU_CustomerID_Set(cCustom);
            halLDCtrl_CustomerID_Set(cCustom);
        }
        else if(strcmp("custom=barco", sCmdFormat->cTextString)==0)
        {
            cCustom = CUSTOMER_ID_BARCO;
            halMCU_CustomerID_Set(cCustom);
            halLDCtrl_CustomerID_Set(cCustom);
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }
    }

    return eCLI_ERROR_CODE_NO;
}

// ==============================================================================
// FUNCTION NAME: utilCommon_BODPLAT
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
// 2021/08/20, Larry Create
// --------------------
// ==============================================================================
eCLI_ERROR_CODE utilCommonCLI_BODPLAT(sCLI_COMMON_FORMAT* sCmdFormat)
{
    if(sCmdFormat->eAccessMode == ecmRead)
    {
        switch(palSystem_ModelIDGet())
        {
            case MODEL_ID_0:
                sCmdFormat->lData = MODULE_TYPE_ID0_PLATFORM;
                break;

            case MODEL_ID_1:
                sCmdFormat->lData = MODULE_TYPE_ID1_PLATFORM;
                break;

            case MODEL_ID_2:
                sCmdFormat->lData = MODULE_TYPE_ID2_PLATFORM;
                break;

            default:
                sCmdFormat->lData = MODULE_NA_PLATFORM;
                break;
        }
    }
    else if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT8 ucString[64] = {0};
        char  ucOutputString[128] = {""};
        BYTE cPlatform = 0;
        BYTE cResult = 0;

        if(strcmp("platform=0", sCmdFormat->cTextString)==0)
        {
            cPlatform = MODULE_TYPE_ID0_PLATFORM;

            if(cPlatform != MODULE_NA_PLATFORM)
            {
                palSystem_ModelIDSet(cPlatform);
                halMCU_PlatformID_Set(cPlatform);
#ifdef PLATFORM_A35G2
                halLDCtrl_PlatformID_Set(cPlatform);
#else
                halFanCtrl_Platform_Set(Board_ModelID_Get());
#endif /* PLATFORM_A35G2 */
                cResult = 1;
            }
        }
        else if(strcmp("platform=1", sCmdFormat->cTextString)==0)
        {
            cPlatform = MODULE_TYPE_ID1_PLATFORM;
            if(cPlatform != MODULE_NA_PLATFORM)
            {
                palSystem_ModelIDSet(cPlatform);
                halMCU_PlatformID_Set(cPlatform);
#ifdef PLATFORM_A35G2
                halLDCtrl_PlatformID_Set(cPlatform);
#else
                halFanCtrl_Platform_Set(Board_ModelID_Get());
#endif /* PLATFORM_A35G2 */
                cResult = 1;
            }
        }
        else if(strcmp("platform=2", sCmdFormat->cTextString)==0)
        {
            cPlatform = MODULE_TYPE_ID2_PLATFORM;
            if(cPlatform != MODULE_NA_PLATFORM)
            {
                palSystem_ModelIDSet(cPlatform);
                halMCU_PlatformID_Set(cPlatform);
#ifdef PLATFORM_A35G2
                halLDCtrl_PlatformID_Set(cPlatform);
#else
                halFanCtrl_Platform_Set(Board_ModelID_Get());
#endif /* PLATFORM_A35G2 */
                cResult = 1;
            }
        }
        else
        {
            return eCLI_ERROR_CODE_SETFAIL;
        }

        if(cResult)
        {
            utilCommonCLI_DataConversionSet(edcCUSTOMER_CODE, cPlatform);
            utilCommonCLI_DataConversionSet(edcMODEL_NAME, cPlatform);
            utilCommonCLI_String_Get(edcMODEL_NAME, ucString);
            sprintf(ucOutputString, "Model is %s\r\n", ucString);
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
        else
        {
            sprintf(ucOutputString, "platform not support\r\n");
            __CMD_Respond((eCLI_CHANNEL)sCmdFormat->ucCmdFrom, ucOutputString);
        }
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilCommonCLI_TTT(sCLI_COMMON_FORMAT* sCmdFormat)
{
    if(sCmdFormat->eAccessMode == ecmRead)
    {
        SetTesterMode();
        sCmdFormat->lData = 1;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilCommonCLI_ShowOSDBitmapRawData(sCLI_COMMON_FORMAT* sCmdFormat)  //A35G2_Simon_0116
{
    eCLI_ERROR_CODE eErrorCode = eCLI_ERROR_CODE_NO;
    char  ucOutputString[128] = {""};

    if(sCmdFormat->eAccessMode == ecmWrite)
    {
        UINT32 ulStartVertPosition = (UINT32)sCmdFormat->lData;
        palGui_ShowOsdBitmapData(ulStartVertPosition);
    }

    return eErrorCode;
}

eCLI_ERROR_CODE utilCommonCLI_DecToString(UINT16 uiDataCode, INT32 iValue, char* cString)
{
#if 0
    /* 00 */ eCM_SCALING_MODE_AUTO,
    /* 01 */ eCM_SCALING_MODE_NATIVE,
    /* 02 */ eCM_SCALING_MODE_4_3,
    /* 03 */ eCM_SCALING_MODE_LETTER_BOX,
    /* 04 */ eCM_SCALING_MODE_FULL_SIZE,
    /* 05 */ eCM_SCALING_MODE_FULL_WIDTH,
    /* 06 */ eCM_SCALING_MODE_FULL_HEIGHT,
    /* 07 */ eCM_SCALING_MODE_CUSTOM,
    /* 08 */ eCM_SCALING_MODE_3D,
    /* 09 */ eCM_SCALING_MODE_16_9,
    /* 10 */ eCM_SCALING_MODE_16_10,
    /* 11 */ eCM_SCALING_MODE_21_9,
#endif /* 0 */

    const char *m_sAspectRatioString[] =
    {
        "Auto",
        "Native",
        "4:3",
        "Letterbox",
        "Full Size",
        "Full Width",
        "Full Height",
        "Custom",
        "3D",
        "16:9",
        "16:10",
        "21:9",
    };

    switch(uiDataCode)
    {
        case edcMAIN_ASPECT_RATIO:
        case edcSUB_ASPECT_RATIO:
            if(iValue < eCM_SCALING_MODE_NUMBER)
            {
                sprintf(cString, "%s", m_sAspectRatioString[iValue]);
            }
            else
            {
                return eCLI_ERROR_CODE_STRINGERR;
            }
            break;

        default:
            return eCLI_ERROR_CODE_STRINGERR;
    }

    return eCLI_ERROR_CODE_NO;
}

eCLI_ERROR_CODE utilCommonCLI_ScheduleParseEvent_to_Rs232(UINT8 * ucToParseStr, UINT32 * uiEventInfo)
{
    INT32 iValue = 0;
    UINT8 aucDataString[32] = {0};  //下面使用 strtok , 會把 . 取代為 NULL , 所以先複製一份來處理

    memcpy(aucDataString, ucToParseStr, strlen((char *)ucToParseStr));
    LOG_MSG(db_APP_SCHEDULE, "\r\nparse str = %s\r\n", aucDataString);
    UINT8 ucIndex = 0 ;
    char *pch ;

    pch = strtok((char *)aucDataString, ",") ;   // 以 , 這個字元來分割字串

    if(pch == NULL)
    {
        LOG_MSG(db_APP_SCHEDULE, " strtok overflow \r\n");
        return eCLI_ERROR_CODE_DATAOVERRANGE ;
    }

    while(pch != NULL && ucIndex < 3)
    {
        uiEventInfo[ucIndex] = atoi(pch) ;

        LOG_MSG(db_APP_SCHEDULE, "parse %d = %02d\r\n", ucIndex, uiEventInfo[ucIndex]);

        if((uiEventInfo[ucIndex] < 0) || (uiEventInfo[ucIndex] > 2359))  //G100_Owen_0047
        {
            return eCLI_ERROR_CODE_DATAOVERRANGE;
        }


        ucIndex++ ;
        pch = strtok(NULL, ",") ;
    }

    return eCLI_ERROR_CODE_NO;
}

typedef enum
{
    eSWGEC_PWM_CMD_SUB1_ALL = 0,
    eSWGEC_PWM_CMD_SUB1_RLD = 1,
    eSWGEC_PWM_CMD_SUB1_GLD = 2,
    eSWGEC_PWM_CMD_SUB1_BLD = 3,

    eSWGEC_PWM_CMD_SUB1_NUMBER,
}eSWGEC_PWM_CMD_SUB1;

typedef enum
{
    eSWGEC_PWM_CMD_SUB2_ALL = 0,
    eSWGEC_PWM_CMD_SUB2_R = 1,
    eSWGEC_PWM_CMD_SUB2_G = 2,
    eSWGEC_PWM_CMD_SUB2_B = 3,
    eSWGEC_PWM_CMD_SUB2_Y = 4,

    eSWGEC_PWM_CMD_SUB2_NUMBER,
}eSWGEC_PWM_CMD_SUB2;

typedef enum
{
    eSWGEC_HSG_CMD_SUB1_ALL = 0,
    eSWGEC_HSG_CMD_SUB1_R,
    eSWGEC_HSG_CMD_SUB1_G,
    eSWGEC_HSG_CMD_SUB1_B,
    eSWGEC_HSG_CMD_SUB1_C,
    eSWGEC_HSG_CMD_SUB1_M,
    eSWGEC_HSG_CMD_SUB1_Y,
    eSWGEC_HSG_CMD_SUB1_W,

    eSWGEC_HSG_CMD_SUB1_NUMBER,
}eSWGEC_HSG_CMD_SUB1;

typedef enum
{
    eSWGEC_HSG_CMD_SUB2_H = 1,
    eSWGEC_HSG_CMD_SUB2_S,
    eSWGEC_HSG_CMD_SUB2_G,

    eSWGEC_HSG_CMD_SUB2_NUMBER,
}eSWGEC_HSG_CMD_SUB2;

typedef enum
{
    eSWGEC_CMD_PWM_GET = 0,
    eSWGEC_CMD_PWM_SET,
    eSWGEC_CMD_PWM_RESET,

    eSWGEC_CMD_HSG_GET,
    eSWGEC_CMD_HSG_SET,
    eSWGEC_CMD_HSG_RESET,
    eSWGEC_CMD_OSD_PATTERN,
    eSWGEC_CMD_DDP_PATTERN,

    eSWGEC_CMD_PICTUREMODE_SET,
    eSWGEC_CMD_PICTUREMODE_GET,
    eSWGEC_CMD_PICTUREMODE_LIST,
    eSWGEC_CMD_PICTUREMODE_STATUS,

    eSWGEC_CMD_NUMBER
}eSWGEC_CMD;

char *pcSWgec_cmd[] =
{
    "LDPWMGET",
    "LDPWMSET",
    "LDPWMRESET",
    "HSGG",
    "HSGS",
    "HSGRST",
    "OTP",
    "DTP",
    "DPMOP",
    "DPMOG",
    "DPMOL",
    "DPMOS",
};

#define SW_GEC_CMD_NUMBER sizeof(pcSWgec_cmd)/sizeof(pcSWgec_cmd[0])

void utilCommonCLI_LDPWM_Get(UINT8 eCh, UINT8 cSub1, UINT8 cSub2)
{
    char  ucOutputString[128] = {""};

    switch(cSub1)
    {
        case eSWGEC_PWM_CMD_SUB1_RLD:
            switch(cSub2)
            {
                case eSWGEC_PWM_CMD_SUB2_R:
                    {
                        UINT16 uiValue = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_RLD_PWM_R, edaREAD, (void*)&uiValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>", pcSWgec_cmd[eSWGEC_CMD_PWM_GET], cSub1, cSub2, uiValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_G:
                    break;
                case eSWGEC_PWM_CMD_SUB2_B:
                    break;
                case eSWGEC_PWM_CMD_SUB2_Y:
                    {
                        UINT16 uiValue = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_RLD_PWM_Y, edaREAD, (void*)&uiValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>", pcSWgec_cmd[eSWGEC_CMD_PWM_GET], cSub1, cSub2, uiValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_PWM_CMD_SUB1_GLD:
            switch(cSub2)
            {
                case eSWGEC_PWM_CMD_SUB2_R:
                    break;
                case eSWGEC_PWM_CMD_SUB2_G:
                    break;
                case eSWGEC_PWM_CMD_SUB2_B:
                    break;
                case eSWGEC_PWM_CMD_SUB2_Y:
                    break;
            }
            break;
        case eSWGEC_PWM_CMD_SUB1_BLD:
            switch(cSub2)
            {
                case eSWGEC_PWM_CMD_SUB2_R:
                    {
                        UINT16 uiValue = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_R, edaREAD, (void*)&uiValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_PWM_GET], cSub1, cSub2, uiValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_G:
                    {
                        UINT16 uiValue = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_G, edaREAD, (void*)&uiValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_PWM_GET], cSub1, cSub2, uiValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_B:
                    {
                        UINT16 uiValue = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_B, edaREAD, (void*)&uiValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_PWM_GET], cSub1, cSub2, uiValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_Y:
                    {
                        UINT16 uiValue = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_Y, edaREAD, (void*)&uiValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_PWM_GET], cSub1, cSub2, uiValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
    }
}

void utilCommonCLI_LDPWM_Set(UINT8 cSub1, UINT8 cSub2, UINT16 uiValue)
{
    switch(cSub1)
    {
        case eSWGEC_PWM_CMD_SUB1_ALL:
            if(cSub2 == 0)
            {
                palFormatterMgr_PWM_Set(0, uiValue);
                palFormatterMgr_PWM_Set(1, uiValue);
                palFormatterMgr_PWM_Set(2, uiValue);
                palFormatterMgr_PWM_Set(3, uiValue);
                palFormatterMgr_PWM_Set(4, uiValue);
                palFormatterMgr_PWM_Set(5, uiValue);
            }
            break;

        case eSWGEC_PWM_CMD_SUB1_RLD:
            switch(cSub2)
            {
                case eSWGEC_PWM_CMD_SUB2_ALL:
                    {
                        //palFormatterMgr_PWM_Set(0, uiValue);
                        //palFormatterMgr_PWM_Set(1, uiValue);
                        //palFormatterMgr_PWM_Set(2, uiValue);
                        //palFormatterMgr_PWM_Set(3, uiValue);
                        palFormatterMgr_PWM_Set(4, uiValue);
                        palFormatterMgr_PWM_Set(5, uiValue);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_R:
                    {
                        UINT8 uiTemp = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiTemp);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_RLD_PWM_R, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_G:
                    break;
                case eSWGEC_PWM_CMD_SUB2_B:
                    break;
                case eSWGEC_PWM_CMD_SUB2_Y:
                    {
                        UINT8 uiTemp = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiTemp);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_RLD_PWM_Y, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                    }
                    break;
            }
            break;
        case eSWGEC_PWM_CMD_SUB1_GLD:
            switch(cSub2)
            {
                case eSWGEC_PWM_CMD_SUB2_R:
                    break;
                case eSWGEC_PWM_CMD_SUB2_G:
                    break;
                case eSWGEC_PWM_CMD_SUB2_B:
                    break;
                case eSWGEC_PWM_CMD_SUB2_Y:
                    break;
            }
            break;
        case eSWGEC_PWM_CMD_SUB1_BLD:
            switch(cSub2)
            {
                case eSWGEC_PWM_CMD_SUB2_ALL:
                    {
                        palFormatterMgr_PWM_Set(0, uiValue);
                        palFormatterMgr_PWM_Set(1, uiValue);
                        palFormatterMgr_PWM_Set(2, uiValue);
                        palFormatterMgr_PWM_Set(3, uiValue);
                        //palFormatterMgr_PWM_Set(4, uiValue);
                        //palFormatterMgr_PWM_Set(5, uiValue);
                    }
                    break;

                case eSWGEC_PWM_CMD_SUB2_R:
                    {
                        UINT8 uiTemp = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiTemp);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_R, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_G:
                    {
                        UINT8 uiTemp = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiTemp);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_G, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_B:
                    {
                        UINT8 uiTemp = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiTemp);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_B, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                    }
                    break;
                case eSWGEC_PWM_CMD_SUB2_Y:
                    {
                        UINT8 uiTemp = 0;
                        palDataMgr_Data_Access(edcFORMATTOR_PWM_UPDATE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiTemp);
                        palDataMgr_Data_Access(edcFACTORY_FORMATTER_BLD_PWM_Y, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiValue);
                    }
                    break;
            }
            break;
    }
}

void utilCommonCLI_HSG_Get(UINT8 eCh, UINT8 cSub1, UINT8 cSub2)
{
    char  ucOutputString[128] = {""};

    switch(cSub1)
    {
        case eSWGEC_HSG_CMD_SUB1_R:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_RED_HUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_RED_SATURATION, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_RED_GAIN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_G:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_GREEN_HUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_GREEN_SATURATION, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_GREEN_GAIN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_B:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_BLUE_HUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_BLUE_SATURATION, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_BLUE_GAIN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_C:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_CYAN_HUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_CYAN_SATURATION, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_CYAN_GAIN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_M:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_MAGENTA_HUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_MAGENTA_SATURATION, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_MAGENTA_GAIN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_Y:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_YELLOW_HUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_YELLOW_SATURATION, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_YELLOW_GAIN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_W:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_RED, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_GREEN, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        UINT8 ucValue = 0;
                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_BLUE, edaREAD, (void*)&ucValue);
                        sprintf(ucOutputString, "<%s, %d, %d, %d>", pcSWgec_cmd[eSWGEC_CMD_HSG_GET], cSub1, cSub2, ucValue);
                        __CMD_Respond(eCh, ucOutputString);
                    }
                    break;
            }
            break;
    }
}

void utilCommonCLI_HSG_Set(UINT8 cSub1, UINT8 cSub2, UINT8 ucValue)
{
    switch(cSub1)
    {
        case eSWGEC_HSG_CMD_SUB1_R:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_RED_HUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_RED_SATURATION, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_RED_GAIN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_G:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_GREEN_HUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_GREEN_SATURATION, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_GREEN_GAIN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_B:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_BLUE_HUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_BLUE_SATURATION, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_BLUE_GAIN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_C:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_CYAN_HUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_CYAN_SATURATION, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_CYAN_GAIN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_M:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_MAGENTA_HUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_MAGENTA_SATURATION, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_MAGENTA_GAIN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_Y:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_YELLOW_HUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_YELLOW_SATURATION, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_YELLOW_GAIN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
        case eSWGEC_HSG_CMD_SUB1_W:
            switch(cSub2)
            {
                case eSWGEC_HSG_CMD_SUB2_H:
                    {
                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_RED, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_S:
                    {
                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_GREEN, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
                case eSWGEC_HSG_CMD_SUB2_G:
                    {
                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_BLUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucValue);
                    }
                    break;
            }
            break;
    }
}


UINT8 utilCommonCLI_SWGEC_Handle(UINT8 eCh, UINT8 *pcData)
{
    UINT8 ucData = 0;
    UINT8 ucCount = 0;

    ucData = *pcData;

    if(ucData == SWGEC_CMD_HEADER || m_sLD_PWM_Info.uiCLI_Poll_Position != 0)
    {
        m_sLD_PWM_Info.ucCLI_DATA[m_sLD_PWM_Info.uiCLI_Poll_Position++] = ucData;

        if(ucData == SWGEC_CMD_HEADER) //(sys(sys?)
        {
            m_sLD_PWM_Info.uiCLI_Poll_Position = 0;
            m_sLD_PWM_Info.ucCLI_DATA[m_sLD_PWM_Info.uiCLI_Poll_Position++] = ucData;
        }

        if(m_sLD_PWM_Info.uiCLI_Poll_Position > CLI_DATA_SIZE)
        {
            m_sLD_PWM_Info.uiCLI_Poll_Position = 0;
            return 0;
        }

        if(ucData == SWGEC_CMD_END)
        {
            char cDataString[64] = {'\0'};
            char cmd[32] = {'\0'};
            UINT16 sub1 = 0;
            UINT16 sub2 = 0;
            UINT16 value = 0;

            m_sLD_PWM_Info.ucCLI_DATA[m_sLD_PWM_Info.uiCLI_Poll_Position] = '\0';
            snprintf(cDataString, 64, "%s", (char*)&m_sLD_PWM_Info.ucCLI_DATA[1]);
            sscanf(cDataString, "%[^,],%hu,%hu,%hu", cmd, &sub1, &sub2, &value);

            for(ucCount = 0; ucCount < eSWGEC_CMD_NUMBER; ucCount++)
            {
                if(__CheckCmd(cDataString, pcSWgec_cmd[ucCount], strlen(pcSWgec_cmd[ucCount])))
                {
                    switch(ucCount)
                    {
                        case eSWGEC_CMD_PWM_GET:
                            utilCommonCLI_LDPWM_Get(eCh, sub1, sub2);
                            break;

                        case eSWGEC_CMD_PWM_SET:
                            utilCommonCLI_LDPWM_Set(sub1, sub2, value);
                            break;

                        case eSWGEC_CMD_PWM_RESET:
                            {
                                UINT8 ucPOWER_MODE = 0;
                                palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, (void*)&ucPOWER_MODE);
                                palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);
                            }
                            break;

                        case eSWGEC_CMD_HSG_GET:
                            {
                                UINT8 ucInputKey = 0;

                                palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKey);
                                if(ucInputKey != eSOURCE_KEY_CHANGE_SOURCE)
                                {
                                    ucInputKey = eSOURCE_KEY_CHANGE_SOURCE;
                                    palDataMgr_Data_Access(edcINPUT_KEY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucInputKey);
                                }

                                utilCommonCLI_HSG_Get(eCh, sub1, sub2);
                            }
                            break;

                        case eSWGEC_CMD_HSG_SET:
                            {
                                UINT8 ucInputKey = 0;

                                palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKey);
                                if(ucInputKey != eSOURCE_KEY_CHANGE_SOURCE)
                                {
                                    ucInputKey = eSOURCE_KEY_CHANGE_SOURCE;
                                    palDataMgr_Data_Access(edcINPUT_KEY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucInputKey);
                                }

                                utilCommonCLI_HSG_Set(sub1, sub2, (UINT8)value);
                            }
                            break;

                        case eSWGEC_CMD_HSG_RESET:
                            switch(sub1)
                            {
                                case eSWGEC_HSG_CMD_SUB1_ALL:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;

                                case eSWGEC_HSG_CMD_SUB1_R:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_RED_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;
                                case eSWGEC_HSG_CMD_SUB1_G:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_GREEN_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;
                                case eSWGEC_HSG_CMD_SUB1_B:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_BLUE_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;
                                case eSWGEC_HSG_CMD_SUB1_C:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_CYAN_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }

                                    break;
                                case eSWGEC_HSG_CMD_SUB1_M:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_MAGENTA_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;
                                case eSWGEC_HSG_CMD_SUB1_Y:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_YELLOW_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;
                                case eSWGEC_HSG_CMD_SUB1_W:
                                    {
                                        UINT8 ucReset = 0;
                                        palDataMgr_Data_Access(edcHSG_WHITE_GAIN_RESET_DEFAULT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucReset);
                                    }
                                    break;
                            }
                            break;

                        case eSWGEC_CMD_OSD_PATTERN:
                            {
                                UINT8 ucPattern = (UINT8)sub1;
                                palDataMgr_Data_Access(edcSERVICE_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPattern);
                            }
                            break;

                        case eSWGEC_CMD_DDP_PATTERN:
                            {
                                UINT8 ucPattern = (UINT8)sub1;
                                palDataMgr_Data_Access(edcFACTORY_TEST_PATTERN, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPattern);
                            }
                            break;

                        case eSWGEC_CMD_PICTUREMODE_SET:
                            {
                                UINT8 ucPictureMode = (UINT8)sub1;
                                UINT8 ucCurrentMode = 0;
                                UINT32 ucPictureModeAvailable = 0;

                                palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, (void*)&ucCurrentMode);

                                if(palDataMgr_DataCode_Control(edcPICTURE_SETTINGS) == eFUNC_CONTROL_ENABLE)
                                {
                                    switch(ucPictureMode)
                                    {
                                        case eCM_PICTURE_SETTINGS_3D:
                                        case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                                            {
                                                UINT8 uc3DEnable = 0;
                                                UINT8 uc3DMode = 0;

                                                palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, (void*)&uc3DEnable);

                                                if(uc3DEnable != eCM_3D_FORMAT_FRAME_SEQUENTIAL)
                                                {
                                                    uc3DEnable = eCM_3D_FORMAT_FRAME_SEQUENTIAL;
                                                    palDataMgr_Data_Access(edc3D_ENABLE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uc3DEnable);
                                                }

                                                if(eCM_PICTURE_SETTINGS_3D == ucPictureMode)
                                                {
                                                    uc3DMode = eCM_3D_MODE_ON; //active 3D
                                                    palDataMgr_Data_Access(edc3D_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uc3DMode);
                                                }
                                                else
                                                {
                                                    uc3DMode = eCM_3D_MODE_PASSIVE_3D;
                                                    palDataMgr_Data_Access(edc3D_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uc3DMode);
                                                }
                                            }
                                            break;

                                        case eCM_PICTURE_SETTINGS_PRESENTATION:
                                        case eCM_PICTURE_SETTINGS_VIDEO:
                                        case eCM_PICTURE_SETTINGS_BRIGHT:
                                        case eCM_PICTURE_SETTINGS_ENHANCED:
                                        case eCM_PICTURE_SETTINGS_REC709:
                                        case eCM_PICTURE_SETTINGS_REAL:
                                        case eCM_PICTURE_SETTINGS_DICOMSIM:
                                        case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                                        case eCM_PICTURE_SETTINGS_BLENDING:
                                        case eCM_PICTURE_SETTINGS_USER:
                                        //case eCM_PICTURE_SETTINGS_HDR:
                                        case eCM_PICTURE_SETTINGS_SRGB:
                                        case eCM_PICTURE_SETTINGS_SUPER_BRIGHT:
                                        case eCM_PICTURE_SETTINGS_SUPER_RED:
                                        //case eCM_PICTURE_SETTINGS_HLG:
                                            {
                                                UINT8 uc3DEnable = 0;

                                                palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, (void*)&uc3DEnable);

                                                if(uc3DEnable != eCM_3D_FORMAT_OFF)
                                                {
                                                    uc3DEnable = eCM_3D_FORMAT_OFF;
                                                    palDataMgr_Data_Access(edc3D_ENABLE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uc3DEnable);
                                                }

                                                palDataItemAvailable_AccessRead(eDI_PICTURE_SETTINGS, (void*)&ucPictureModeAvailable);

                                                for(ucCount = 0; ucCount < eCM_PICTURE_SETTINGS_NUMBER; ucCount++)
                                                {
                                                    UINT8 ucGuiDisplayModeID = (UINT8)CM2GUI(edcPICTURE_SETTINGS, ucCount);

                                                    if((ucPictureModeAvailable >> ucCount) & 0x01 && ucGuiDisplayModeID > eGUI_PICTURE_SETTINGS_NUMBER)
                                                    {
                                                        BIT_CLEAR(ucPictureModeAvailable, ucCount);
                                                    }
                                                }

                                                if((ucPictureModeAvailable >> ucPictureMode) & 0x01)
                                                {
                                                    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPictureMode);
                                                }
                                            }
                                            break;
                                    }
                                }
                            }
                            break;

                        case eSWGEC_CMD_PICTUREMODE_GET:
                            if(sub1 == 99)
                            {
                                char  ucOutputString[128] = {""};
                                UINT8 ucCurrentMode = 0;

                                palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, (void*)&ucCurrentMode);

                                sprintf(ucOutputString, "<%s, %d>\n", pcSWgec_cmd[eSWGEC_CMD_PICTUREMODE_GET], ucCurrentMode);
                                __CMD_Respond(eCh, ucOutputString);
                            }
                            break;

                        case eSWGEC_CMD_PICTUREMODE_LIST:
                            break;

                        case eSWGEC_CMD_PICTUREMODE_STATUS:
                            {
                                char  ucOutputString[128] = {""};
                                UINT8 ucPictureMode = (UINT8)sub1;
                                UINT8 ucGuiDisplayModeID = 0, ucCount = 0;
                                UINT32 ucPictureModeAvailable = 0;

                                palDataItemAvailable_AccessRead(eDI_PICTURE_SETTINGS, (void*)&ucPictureModeAvailable);

                                for(ucCount = 0; ucCount < eCM_PICTURE_SETTINGS_NUMBER; ucCount++)
                                {
                                    ucGuiDisplayModeID = (UINT8)CM2GUI(edcPICTURE_SETTINGS, ucCount);

                                    if((ucPictureModeAvailable >> ucCount) & 0x01 && ucGuiDisplayModeID > eGUI_PICTURE_SETTINGS_NUMBER)
                                    {
                                        BIT_CLEAR(ucPictureModeAvailable, ucCount);
                                    }
                                }

                                if(sub1 < eCM_PICTURE_SETTINGS_NUMBER)
                                {
                                    sprintf(ucOutputString, "<%s, %d, %d>\n", pcSWgec_cmd[eSWGEC_CMD_PICTUREMODE_STATUS], ucPictureMode, (ucPictureModeAvailable >> sub1) & 0x01);
                                    __CMD_Respond(eCh, ucOutputString);
                                }
                                else if(sub1 == 99)
                                {
                                    UINT16 uiOffset = 0;

                                    uiOffset = sprintf(ucOutputString, "<%s, %d", pcSWgec_cmd[eSWGEC_CMD_PICTUREMODE_STATUS], ucPictureMode);

                                    for(ucCount = 0; ucCount < eCM_PICTURE_SETTINGS_NUMBER; ucCount++)
                                    {
                                        uiOffset += sprintf(ucOutputString + uiOffset, ", %d",  (ucPictureModeAvailable >> ucCount) & 0x01);
                                    }

                                    uiOffset += sprintf(ucOutputString + uiOffset, ">\n");
                                    __CMD_Respond(eCh, ucOutputString);
                                }

                            }
                            break;
                    }
                    break;
                }
            }

            m_sLD_PWM_Info.uiCLI_Poll_Position = 0;

            return 1;
        }
    }
    return 0;
}

UINT8 utilCommonCLI_SWGEC_Telnet_Handle(UINT8 *pcData)
{
    if(pcData[0] == SWGEC_CMD_HEADER)
	{
	    UINT8 ucCount = 0;
	    UINT16 uiStrlen = strlen(pcData);

	    for(ucCount = 0; (ucCount < uiStrlen || ucCount < 255); ucCount++)
        {
    	    BYTE cData = pcData[ucCount];

            utilCommonCLI_SWGEC_Handle(eccTelnet, &cData);
        }
	}
}

