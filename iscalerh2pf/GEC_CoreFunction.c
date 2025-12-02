/*********************************************************************************
 * FILE NAME: GEC_COREFUNCTION.C
 * DESCRIPTION:
 *
 *
 *  Created on: 26 June, 2017
 *      Author: John.Chung
*********************************************************************************/
//A70LV_John_0008 start add basic function and flow of GEC
#include "GEC_UserOption.h"
#include "GEC_CoreFunction.h"
#include "GEC_CLITable.h"
#include "GEC_EventTable.h"

#include "utilCRCAPI.h"
#include "appLANProcAPI.h"
#include "CustomDef.h"


static UINT8 m_ucGECmode = TRUE;
static UINT8 m_ucGECdebug = FALSE;  //G100_Steven_0011
static UINT8 ucIsGECCommand = FALSE; //ZU860_John_0043 fix CLI conflict issue
UINT32 uWAP_Flag = 0; //G50_Casper_0006

#define GEC_DEBUG_MSG //A70LV_John_0121 move GEC debug message to define

///////////////// callback function start ///////////////
sGEC_CALLBACK sGEC_callback =
{
    .fpSetTpCallback = NULL,
    .fpCCT_StorageSetCallback = NULL,
    .fpCCT_StorageGetCallback = NULL,
};

void GEC_RegCallback(sGEC_CALLBACK fpCallback)
{
    sGEC_callback = fpCallback;
    //sGEC_callback.fpSetTpCallback          = fpCallback.fpSetTpCallback;
    //sGEC_callback.fpCCT_StorageSetCallback = fpCallback.fpCCT_StorageSetCallback;  //palIapProc_CCT_StorageSet
    //sGEC_callback.fpCCT_StorageGetCallback = fpCallback.fpCCT_StorageGetCallback;
}
////////////////// callback function end ////////////////

void GEC_DebugMode_Set(UINT8 ucvalue)
{
    m_ucGECdebug = ucvalue;
}

UINT8 GEC_DebugMode_Get(void)
{
    return m_ucGECdebug;
}

/************************************************************************************************
 * @function : GEC_EventHandler(UINT8 *pcInput)
 *             &pcInput to CLI input.
 * @brief: GEC CLI command enter point
 * @params: NULL
 * @upper function: None (by user setting)
 * @author: John.Chung
 * @Date : 25 June, 2017
 ***********************************************************************************************/
void GEC_EventHandler(UINT8 *pcInput)
{
    static eGEC_CLI_STATE GEC_CLI_State = eGEC_CLI_STATE_Invalid;
    static UINT8 ucGEC_CLI[GEC_CLI_MAX_LENGTH] = {0}; //[full input command]
    static UINT8 *pucIndex; // if *pcIndex > 255, *pcIndex = 0
    static UINT8 ucLength = 0;

    //GEC_printf( "\r\n GEC_EventHandler %d ",GEC_CLI_State);

    //detect string
    switch (*pcInput) //Keyword to change state
    {
    case '[':
        GEC_CLI_State = eGEC_CLI_STATE_Initial;
        break;

    case ']':
        if (GEC_CLI_State == eGEC_CLI_STATE_Input)
        {
        	//GEC_printf( "\r\n GEC_CLI_STATE_End ");
            GEC_CLI_State = eGEC_CLI_STATE_End;
        }
        else
        {
            GEC_CLI_State = eGEC_CLI_STATE_Invalid;
        }
        break;

    default:
        if (GEC_CLI_State == eGEC_CLI_STATE_Input ||
            GEC_CLI_State == eGEC_CLI_STATE_Invalid)
        {
            //no need to change state and keep space
        }
        else if (GEC_CLI_State == eGEC_CLI_STATE_Initial)
        {
            GEC_CLI_State = eGEC_CLI_STATE_Input;
        }
        else if (GEC_CLI_State == eGEC_CLI_STATE_End)
        {
            GEC_CLI_State = eGEC_CLI_STATE_Invalid;
        }
        break;
    }

    if ((ucLength+1) >= GEC_CLI_MAX_LENGTH)  //A70LV_John_0084 fix RS232 stress test crash
    {
        if(TRUE == GEC_DebugMode_Get())
        {
#ifdef GEC_DEBUG_MSG
            GEC_printf( "\r\nString length= %d + 1", ucLength);
#endif
        }
        GEC_CLI_State = eGEC_CLI_STATE_Invalid;
    }

    //initial state judge
    if(GEC_CLI_State == eGEC_CLI_STATE_Input)
    {
        if(*pcInput != 0x8) //0x8 == 'backspace'
        {
            *pucIndex = *pcInput;
            pucIndex++;
            ucLength++;
        }
        else //add typo correction function (backspace only)
        {
            pucIndex--;
            ucLength--;
            *pucIndex = 0x00;
        }
    }
    else if(GEC_CLI_State == eGEC_CLI_STATE_Initial)
    {
        memset(ucGEC_CLI, 0x00, GEC_CLI_MAX_LENGTH);
        pucIndex = &ucGEC_CLI[0];
        ucLength = 0;
        *pucIndex = *pcInput;
        pucIndex++;
    }
    else if(GEC_CLI_State == eGEC_CLI_STATE_End) //command format checked, go to command wording verification
    {
        *pucIndex = *pcInput;
        ucLength++;
        if(TRUE == GEC_DebugMode_Get())
        {
#ifdef GEC_DEBUG_MSG
            GEC_printf( "\r\n%s = Input string",ucGEC_CLI);  //A70LV_John_0003 modify GEC code common define
#endif
        }
        //ZU860_John_0044 start fix CLI conflict issue
        if(GEC_CommandProcess_Get() != FALSE)
        {
        	GEC_CommandProcess_Set(FALSE);
        }
        //ZU860_John_0044 end
        GEC_CLI_SubIndex_Convert(ucGEC_CLI, ucLength); //Correct GEC command input, go to command process
        ucLength = 0;   //A70LV_John_0084 fix RS232 stress test crash
    }
    else if(GEC_CLI_State == eGEC_CLI_STATE_Invalid)
    {
    	 //GEC_printf( "\r\n EEE");
    	//ZU860_John_0044 start fix CLI conflict issue
    	if(GEC_CommandProcess_Get() != FALSE)
    	{
    		GEC_CommandProcess_Set(FALSE);
    	}
    	//ZU860_John_0044 end
        if (ucLength != 0)
        {
            memset(ucGEC_CLI, 0x00, GEC_CLI_MAX_LENGTH);
            pucIndex = &ucGEC_CLI[0];
            ucLength = 0;
        }
    }
    else
    {
    	// GEC_printf( "\r\n FFF");
    	//ZU860_John_0044 start fix CLI conflict issue
    	if(GEC_CommandProcess_Get() != FALSE)
    	{
    		GEC_CommandProcess_Set(FALSE);
    	}
    	//ZU860_John_0044 end
        GEC_CLI_State = eGEC_CLI_STATE_Invalid; //set for out of State insurance
    }

}
//A70LV_John_0008 end

/************************************************************************************************
 * @function : GEC_CLI_SubIndex_Convert(UINT8 *pCOMMAND, UINT8 length)
 *             *pCOMMAND = raw CLI command that user had inputed.
 *             length    = raw CLI command length
 * @brief: calculate the SubIndex from command input
 * @params:
 * UINT8 cCount = 0;                                         //cursor for counting command
 * UINT8 cChecker = 0;                                       //cursor for command check
 * eGEC_COMMAND_State eState = eGEC_COMMAND_State_Invalid;  //judge the command input state by input
 * eGEC_COMMAND eCOMMAND = eGEC_COMMAND_INVALID;            //judge the command is exist or not
 * UINT8 SUBNUMBER[SUBNUMBER_LENGTH] = {0};                  //temporary container of distilled CLI command for further analysis
 * UINT32 CLI_Index[eCLI_Index_Invalid] = {0};                 //index data of [Command,SubID,DataLength,Data]
 *
 * @upper function: GEC_EventHandler
 * @author: John.Chung
 * @Date : 25 June, 2017
 ***********************************************************************************************/
void GEC_CLI_SubIndex_Convert(UINT8 *pucCOMMAND, UINT8 ucLength)
{
    UINT8 ucCount = 0;
    UINT8 cChecker = 0;
    eGEC_COMMAND_STATE eState = eGEC_COMMAND_State_Invalid;
    eGEC_COMMAND eCOMMAND = eGEC_COMMAND_INVALID;
    UINT8 cBuffer[GEC_CLI_MAX_LENGTH] = {0};
    sCLI_INDEX CLI_SubIndex;

    if(TRUE == GEC_DebugMode_Get())
    {
#ifdef GEC_DEBUG_MSG
        GEC_printf( "\r\nConvert length = %d", ucLength);
#endif
    }

    //initial SubIndex value
    CLI_SubIndex.cCLI_INDEX_Command = eGEC_COMMAND_INVALID;
    CLI_SubIndex.cCLI_INDEX_SubID = 0;
    CLI_SubIndex.cCLI_INDEX_DataLength = 0;
    CLI_SubIndex.cCLI_INDEX_Data = cBuffer;
    CLI_SubIndex.cCLI_INDEX_DataNumber = 0;

    for (ucCount = 0; ucCount <= ucLength; ucCount++)
    {
        switch(pucCOMMAND[ucCount]) //set command state
        {
        case '[':
            if(ucCount == 0)
            {
                eState = eGEC_COMMAND_STATE_Initial;
            }
            else
            {
                eState = eGEC_COMMAND_State_Invalid;
            }

            break;

        case ',':
            switch (eState)
            {
            case eGEC_COMMAND_STATE_Initial:
                eState = eGEC_COMMAND_STATE_Comma1;
                break;

            case eGEC_COMMAND_STATE_Comma1:
                eState = eGEC_COMMAND_STATE_Comma2;
                break;

            case eGEC_COMMAND_STATE_Comma2:
                eState = eGEC_COMMAND_STATE_Comma3;
                break;

            default:
                eState = eGEC_COMMAND_State_Invalid;
                break;
            }
            break;

            case ']':
                if (eState == eGEC_COMMAND_STATE_Comma3)
                {
                    eState = eGEC_COMMAND_STATE_End;
                }
                else
                {
                    eState = eGEC_COMMAND_State_Invalid;
                }
                break;

            default:
                if(eState == eGEC_COMMAND_STATE_End)
                {
                    eState = eGEC_COMMAND_State_Invalid;
                }
                break;
        }

        //check command state
        if (eState == eGEC_COMMAND_STATE_Initial)
        {
            utilGEC_Low2UpCase(&pucCOMMAND[ucCount]); //transform low case to up case
        }
        else if (eState == eGEC_COMMAND_STATE_Comma1)
        {
            if(pucCOMMAND[ucCount] != ',') //command inputing...
            {
                cBuffer[cChecker] = pucCOMMAND[ucCount];
                cChecker++;
                if (cChecker > GEC_CLI_MAX_LENGTH) //prevent overflow
                {
                    memset(cBuffer, 0x00, GEC_CLI_MAX_LENGTH);
                    cChecker = 0;
                }
            }
            else //paser command "apcct"
            {
                //This comparison does not need to use buffer
                for (eCOMMAND = eGEC_GetModelName; eCOMMAND < eGEC_COMMAND_INVALID; eCOMMAND++) //compare each command string
                {
                    if((ucCount - 1) == strlen(sGEC_CommandLUT[eCOMMAND].GEC_Command))
                    {
                        for (cChecker = 0; cChecker < ucCount; cChecker++)
                        {
                            if(pucCOMMAND[cChecker + 1] != sGEC_CommandLUT[eCOMMAND].GEC_Command[cChecker])
                            {
                                break;
                            }
                        }

                        if((cChecker + 1) == ucCount) // full compared = finish
                        {
                            break;
                        }
                    }
                }
                CLI_SubIndex.cCLI_INDEX_Command = eCOMMAND;
                cChecker = 0; //initial cChecker position
            }
        }
        else if (eState == eGEC_COMMAND_STATE_Comma2)
        {
            if(pucCOMMAND[ucCount] != ',') //command inputing...
            {
                cBuffer[cChecker] = pucCOMMAND[ucCount];
                cChecker++;
                if (cChecker > GEC_CLI_MAX_LENGTH) //prevent overflow
                {
                    memset(cBuffer, 0x00, GEC_CLI_MAX_LENGTH);
                    cChecker = 0;
                }
            }
            else
            {
                if(cBuffer[1] == 'x' || cBuffer[1] == 'X') //hex
                {
                	CLI_SubIndex.cCLI_INDEX_SubID = (UINT32)strtol(cBuffer,NULL,16);  //G100_Steven_0011
                }
                else //dec
                {
                	CLI_SubIndex.cCLI_INDEX_SubID = (UINT32)atoi(cBuffer);  //G100_Steven_0011
                }
                memset(cBuffer, 0x00, cChecker);
                cChecker = 0;
            }
        }
        else if (eState == eGEC_COMMAND_STATE_Comma3)
        {
            if(pucCOMMAND[ucCount] != ',') //command inputing...
            {
                cBuffer[cChecker] = pucCOMMAND[ucCount];
                cChecker++;
                if (cChecker > GEC_CLI_MAX_LENGTH) //prevent overflow
                {
                    memset(cBuffer, 0x00, GEC_CLI_MAX_LENGTH);
                    cChecker = 0;
                }
            }
            else
            {
                if(cBuffer[1] == 'x' || cBuffer[1] == 'X') //hex
                {
                	//sprintf((char*)cBuffer, "%X", (UINT32*)&CLI_SubIndex.cCLI_INDEX_DataNumber);
                	CLI_SubIndex.cCLI_INDEX_DataNumber = (UINT32)strtol(cBuffer,NULL,16);  //G100_Steven_0011
                }
                else //dec
                {
                	//sprintf((char*)cBuffer, "%d", (UINT32*)&CLI_SubIndex.cCLI_INDEX_DataNumber);
                	CLI_SubIndex.cCLI_INDEX_DataNumber = (UINT32)atoi(cBuffer);  //G100_Steven_0011
                }
                memset(cBuffer, 0x00, cChecker);
                cChecker = 0;
            }
        }
        else if (eState == eGEC_COMMAND_STATE_End)
        {
            CLI_SubIndex.cCLI_INDEX_DataLength = cChecker;
            CLI_SubIndex.cCLI_INDEX_Data = cBuffer;
            if(TRUE == GEC_DebugMode_Get())
            {
#ifdef GEC_DEBUG_MSG
                GEC_printf( "\r\nLine = %d\r\n",__LINE__);  //A70LV_John_0003 modify GEC code common define
                GEC_printf( "Command  = %d\r\n",CLI_SubIndex.cCLI_INDEX_Command);  //A70LV_John_0003 modify GEC code common define
                GEC_printf( "Sub ID   = %d\r\n",CLI_SubIndex.cCLI_INDEX_SubID);  //A70LV_John_0003 modify GEC code common define
                GEC_printf( "data len = %d\r\n",CLI_SubIndex.cCLI_INDEX_DataLength);  //A70LV_John_0003 modify GEC code common define
                GEC_printf( "data numb= %d\r\n",CLI_SubIndex.cCLI_INDEX_DataNumber);  //A70LV_John_0003 modify GEC code common define
                GEC_printf( "data     = %s\r\n",CLI_SubIndex.cCLI_INDEX_Data);  //A70LV_John_0003 modify GEC code common define
#endif
            }

            if(GEC_AccessMode_Get() == TRUE)
            {
                GEC_CLI_Command_Execute(&CLI_SubIndex);
            }
        }
    }
}

/************************************************************************************************
 * @function : utilGEC_Low2UpCase
 * @brief: convert all low case to upper case for command usage
 * @params: NULL
 * @upper function: GEC_CLI_SubIndex_Convert
 * @author: John.Chung
 * @Date : 25 June, 2017
 ***********************************************************************************************/
static void utilGEC_Low2UpCase(UINT8 *pcData)
{
    if((*pcData >= 'a') && (*pcData <= 'z'))
    {
        *pcData = *pcData - 'a' + 'A';
    }
}

/************************************************************************************************
 * @function : GEC_AccessMode_Get(void)
 * @brief: check status of GEC mode enable or disable
 *         This function is recommended for user build in CLI command
 * @params: NULL
 * @upper function: None (by user setting)
 * @author: John.Chung
 * @Date : 25 June, 2017
 ***********************************************************************************************/
UINT8 GEC_AccessMode_Get(void)
{
    if(TRUE == GEC_DebugMode_Get())
    {
        GEC_printf( "\r\n GEC_AccessMode_Get");
    }
    return m_ucGECmode;
}

/************************************************************************************************
 * @function : GEC_CLI_Command_Execute
 * @brief: print out error code depend on command index input
 *         the index format = {Command,SubID,DataLength,Data}
 * @params: NULL
 * @upper function: GEC_CLI_SubIndex_Convert
 * @author: John.Chung
 * @Date : 25 June, 2017
 ***********************************************************************************************/
void GEC_CLI_Command_Execute(sCLI_INDEX *sCLI)
{
    UINT16 uiStep = 0; //for debug use
    eERROR_LIST ucFanCount;

    if(TRUE == GEC_DebugMode_Get())
    {
        GEC_printf( "\r\n GEC_CLI_Command_Execute\r\n");  //A70LV_John_0003 modify GEC code common define
    }

//ZU860_John_0043 start fix CLI conflict issue
    if (sCLI->cCLI_INDEX_Command < eGEC_COMMAND_INVALID)
    {
        GEC_CommandProcess_Set(TRUE);
    }
//ZU860_John_0043 end

    switch(sCLI->cCLI_INDEX_Command)
    {
    /*
#if 0
    case eGEC_GetModelName:
    case eGEC_GetFwVersion:
    case eGEC_GetSerialNumber:
        GEC_GetCommonInfo(sCLI.cCLI_INDEX_SubID, sCLI.cCLI_INDEX_Command);
        break;

    case eGEC_GetGecInfo:
        if (sCLI.cCLI_INDEX_SubID == 0x01)
        {
            eGEC_COMMAND eStep = eGEC_COMMAND_INVALID;
            for (eStep = eGEC_GetModelName; eStep <= eGEC_GetGecVersion; eStep ++)
            {
                GEC_GetCommonInfo(sCLI.cCLI_INDEX_SubID, eStep);
            }
        }
        else
        {
            GEC_GetCommonInfo(sCLI.cCLI_INDEX_SubID, sCLI.cCLI_INDEX_Command);
        }
        break;

    case eGEC_GetGecVersion:
        GEC_GetCommonInfo(sCLI.cCLI_INDEX_SubID, sCLI.cCLI_INDEX_Command);
        break;

    case eGEC_GetGecEncryption:
        GEC_printf( "\r\n");  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "Function in construction");  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "\r\n");  //A70LV_John_0003 modify GEC code common define
        break;

    case eGEC_GetErrorCode:
        GetErrorCode(eGEC_ErrorRead_AP, sCLI);
        break;

    case eGEC_SetErrorCodeClear:
        if (sCLI.cCLI_INDEX_SubID != 0x01)
        {
            GEC_ErrorCode_Reset();
        }
        else
        {
            GEC_ErrorCode_EraseAll();
        }
        break;

    case eGEC_GetAccCountClear:
    case eGEC_SetAccCountClear:
        GEC_printf( "\r\n");  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "Function in construction");  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "\r\n");  //A70LV_John_0003 modify GEC code common define
        break;

    case eGEC_GetGecTable:
        GEC_GetGecTable();
        break;

    case eGEC_GetErrorCount:
        GEC_GetErrorCount();
        break;

    case eGEC_SetErrorCodeMode:
        if(FALSE == sCLI.cCLI_INDEX_SubID)
        {
            GEC_DebugMode_Set(FALSE);
        }
        else
        {
            GEC_DebugMode_Set(TRUE);
        }
        break;

    case eGEC_HPBUTester:
        uiStep = 0;
        ucFanCount = eFanLock;
        GEC_printf("\r\ndummy code writiing...");
        while (uiStep < sCLI.cCLI_INDEX_SubID)
        {
            ucFanCount++;
            uiStep++;
            GEC_ErrorCode_Write(ucFanCount, 3600+uiStep*60);
            if(ucFanCount == eFanLock15)
            {
                ucFanCount = eFanLock;
            }
        }
        GEC_printf("\r\nDummy code write finished");
        break;

    case eGEC_SetNVRAM:
        GEC_EEPROM_Modify(sCLI.cCLI_INDEX_SubID,sCLI.cCLI_INDEX_DataNumber);
        break;

    case eGEC_GetNVRAM:
        GEC_EEPROM_Print(sCLI.cCLI_INDEX_SubID,sCLI.cCLI_INDEX_DataNumber);
        break;
#endif
*/
    case eGEC_APCCT: //ZU860_John_0018 add auto tuning of corrected color temperature//A70LV_John_0077 add CCT function for AP
        {
            UINT32 uiValue = 0;
            if(sCLI->cCLI_INDEX_Data[1] == 'x' || sCLI->cCLI_INDEX_Data[1] == 'X') //hex
            {
               // sscanf((char*)sCLI->cCLI_INDEX_Data, "%X", (UINT32*)&uiValue);
            	uiValue = (UINT32)strtol((char*)sCLI->cCLI_INDEX_Data,NULL,16);  //G100_Steven_0011
            }
            else //dec
            {
                //sscanf((char*)sCLI->cCLI_INDEX_Data, "%d", (UINT32*)&uiValue);
            	uiValue = (UINT32)atoi((char*)sCLI->cCLI_INDEX_Data);  //G100_Steven_0011
            }
            GEC_APCCT_Process(sCLI->cCLI_INDEX_SubID, sCLI->cCLI_INDEX_DataNumber, uiValue);
        }
        break;

    default:
        break;
    }
    if(TRUE == GEC_DebugMode_Get())
    {
        GEC_printf("\r\n");
        GEC_printf( "Line = %d\r\n",__LINE__);  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "Command  = %d\r\n",sCLI->cCLI_INDEX_Command);  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "Sub ID   = %d\r\n",sCLI->cCLI_INDEX_SubID);  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "data len = %d\r\n",sCLI->cCLI_INDEX_DataLength);  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "data numb= %d\r\n",sCLI->cCLI_INDEX_DataNumber);  //A70LV_John_0003 modify GEC code common define
        GEC_printf( "data     = ");
        for (uiStep = 0; uiStep < sCLI->cCLI_INDEX_DataLength; uiStep++)
        {
            GEC_printf("%c",sCLI->cCLI_INDEX_Data[uiStep]);
        }
        GEC_printf( "\r\n");  //A70LV_John_0003 modify GEC code common define
    }

}

//ZU860_John_0043 start fix CLI conflict issue
void GEC_CommandProcess_Set(UINT8 ucEnable)
{
    ucIsGECCommand = ucEnable;
}
UINT8 GEC_CommandProcess_Get(void)
{
    return ucIsGECCommand;
}
//ZU860_John_0043 end

/************************************************************************************************
 * @function : GEC_GetCommonInfo
 * @brief: print out common information
 * @params: NULL
 * @upper function: GEC_CLI_Command_Execute
 * @author: John.Chung
 * @Date : 27 June, 2017
 ***********************************************************************************************/
#if 0
void GEC_GetCommonInfo(UINT16 subID, UINT16 CommonInfo)
{
    char cString[GEC_ERRORCODE_OUTPUT_MAXSIZE]; //ZU860_John_0046 fix print size bug
    char cStringTemp[GEC_ERRORCODE_OUTPUT_MAXSIZE]; //ZU860_John_0046 fix print size bug

    memset(cString, 0x00, GEC_ERRORCODE_OUTPUT_MAXSIZE); //ZU860_John_0047 fix reset size

    if(TRUE == GEC_DebugMode_Get())
    {
        GEC_printf( "\r\n GEC_GetCommonInfo");  //A70LV_John_0003 modify GEC code common define
    }

    sprintf(cStringTemp,"\r\n[%s,",sGEC_CommandLUT[CommonInfo].GEC_Command);
    strcat(cString, cStringTemp);
    sprintf(cStringTemp,"0x%02X,",subID);
    strcat(cString, cStringTemp);

#if 0
    switch (CommonInfo)
    {
    case eGEC_GetModelName:
        sprintf(cStringTemp,"0x%02X,",strlen(GEC_ModelNameTable));
        strcat(cString, cStringTemp);
        sprintf(cStringTemp,"%s",GEC_ModelNameTable);
        strcat(cString, cStringTemp);
        break;

    case eGEC_GetFwVersion:
        sprintf(cStringTemp,"0x%02X,",strlen(GEC_FWVersionString));
        strcat(cString, cStringTemp);
        sprintf(cStringTemp,"%s",GEC_FWVersionString);
        strcat(cString, cStringTemp);
        break;

    case eGEC_GetSerialNumber:
        sprintf(cStringTemp,"0x%02X,",strlen(GEC_SerialNumString));
        strcat(cString, cStringTemp);
        sprintf(cStringTemp,"%s",GEC_SerialNumString);
        strcat(cString, cStringTemp);
        break;

    case eGEC_GetGecInfo:
        sprintf(cStringTemp,"0x%02X,",strlen(GEC_GECInfoString));
        strcat(cString, cStringTemp);
        sprintf(cStringTemp,"%s",GEC_GECInfoString);
        strcat(cString, cStringTemp);
        break;

    case eGEC_GetGecVersion:
        sprintf(cStringTemp,"0x%02X,",strlen(GEC_GECVersionString));
        strcat(cString, cStringTemp);
        sprintf(cStringTemp,"%s",GEC_GECVersionString);
        strcat(cString, cStringTemp);
        break;

    default:
        sprintf(cStringTemp,"========OUTPUT Not supported=========");
        strcat(cString, cStringTemp);
        break;
    }
#endif

    sprintf(cStringTemp,"]");
    strcat(cString, cStringTemp);
    GEC_printf("%s",cString);
}
#endif

/************************************************************************************************
 * @function : GetErrorCode
 * @brief: print out error code
 * @params:
 * static UINT32 uiERRORCOUNT = 0;     //value assigned by command input, which results the error code you want to read
 * UINT32 counter = 0;               //counter for error code print
 * UINT32 *Total_ErrorCode; //point to the total error code number in NVRAM for convenient
 * @upper function: GEC_CLI_Command_Execute
 * @author: John.Chung
 * @Date : 25 June, 2017
 ***********************************************************************************************/
#if 0
void GetErrorCode(eERRORCODE_READ_TYPE ReadType, sCLI_INDEX sCLI)   //John_20180911 TBD
{
    char cString[GEC_ERRORCODE_OUTPUT_MAXSIZE] ={0};
    static UINT32 uiStep = 1;

    switch(sCLI.cCLI_INDEX_SubID)
    {
    case eGEC_ERRORCODE_GET_FIRST:
        uiStep = 1;
        break;

    case eGEC_ERRORCODE_GET_CURRENT:
        break;

    case eGEC_ERRORCODE_GET_NEXT:
        uiStep++;
        break;

    case eGEC_ERRORCODE_GET_ASSIGNED:
        if(sCLI.cCLI_INDEX_DataNumber == 0)
        {
            GEC_printf("\r\nZero is not available.\r\n");
            return;
        }
        else
        {
            uiStep = sCLI.cCLI_INDEX_DataNumber;
            break;
        }

    case eGEC_ERRORCODE_GET_ALL:
        ///GEC_ReadAll_to_AP();
        return; //attention this must be return

    case eGEC_ERRORCODE_GET_LAST:
        ///uiStep = GEC_GetTotalErrorCount();
        break;

    default:
        break;
    }

    switch(ReadType)
    {
    case eGEC_ErrorRead_AP:
        ///GEC_ReadSingle_to_AP(sCLI.cCLI_INDEX_SubID, uiStep, cString);
        break;

    case eGEC_ErrorRead_Name:
        ///GEC_ReadSingle_to_Name(sCLI.cCLI_INDEX_SubID, uiStep, cString);
    }
    GEC_printf("%s\r\n", cString);
}
#endif

//A70LV_John_0079 start Improve CCT function performance for AP test//A70LV_John_0077 start add CCT function for AP
//Data_X: for write = Display mode, for read = Read mode
//Data_Y: for write = LD type     , for read = Display mode
//Data_Z: for write = Value       , for read = LD type
//A70LV_John_0120 start add APCCT command for value get from DDP
//I2C first byte for reading
//0x00~0x0F reserved for Ratio, 0x80~0x8F reserved for Offset, 0xA0 reserved for Native PWM
//For DDP communication
//#define APCCT_SET_RATIO             (0x00)
#define APCCT_SET_PWM_100           (0x20) //G50_Casper_0006
#define APCCT_SET_PWM_50            (0x30) //G50_Casper_0006
#define APCCT_SET_PWM_30            (0x40) //G50_Casper_0006
#define APCCT_SET_PWM_10            (0x50) //G50_Casper_0006
//#define APCCT_SET_OFFSET            (0x80)
#define APCCT_DDP_SEQUENCE_GET      (0x90)
#define APCCT_DDP_NATIVE_PWM100_GET (0xA0)
#define APCCT_DDP_NATIVE_PWM50_GET  (0xB0)
#define APCCT_DDP_NATIVE_PWM30_GET  (0xC0)
#define APCCT_DDP_NATIVE_PWM10_GET  (0xD0)

#define APCCT_TP_SET                (0xE0) //G100_Steven_0014
#define APCCT_DEBUG                 (0xE1) //R70PD_Casper_0027
//#define APCCT_SET_APPLY             (0xED)
#define APCCT_WAP_APPLY             (0xEF) //G50_Casper_0006

//#define APCCT_GET_RATIO             (0xF0)
#define APCCT_GET_PWM               (0xF1)
//#define APCCT_GET_OFFSET            (0xF2)
#define APCCT_GET_SEQUENCE          (0xF3)
#define APCCT_GET_NATIVE_PWM        (0xF4)

#define APCCT_SET_TAG_ZERO          (0xFA)  //Tag fail test command
//#define APCCT_SET_APPLY_RESET       (0xFB)  //CRC fail test command

//#define APCCT_REFRESH_PWM           (0xFD)
#define APCCT_GET_DISPLAY_AND_LD    (0xFE)
#define APCCT_SET_DEFAULT           (0xFF)
//A70LV_John_0120 end
void GEC_APCCT_Process(UINT32 Data_X, UINT32 Data_Y, UINT32 Data_Z)
{
    static UINT16 uiWAP_PWM100[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    static UINT16 uiWAP_PWM50[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    static UINT16 uiWAP_PWM30[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    static UINT16 uiWAP_PWM10[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    UINT16 uiCount = 0;
    UINT8  ucWAP_Mode = 0;
//A70LV_John_0080 start CCT: add CCT initial check function
    //UINT16 uiCRC = 0;
    uCCT_TABLE uCCTTable = {0};
    static UINT8 ucCCTGet = FALSE;
    UINT8  ucWAP_Init = 0;
    UINT8  ucWAP_Level = 0;

    if(ucCCTGet == FALSE)
    {
        ucCCTGet = TRUE;
        sGEC_callback.fpCCT_StorageGetCallback(&uCCTTable);  //G100_Steven_0011

        if(uCCTTable.sDataStruct.ulCCT_Tag == CCT_NEW_CALIBRATION_TAG) //G50_Casper_0006
        {
            //uiCRC = utilCRC16Calc(&uCCTTable.aucTemp[6], (512 - 6));

            //if(uiCRC == uCCTTable.sDataStruct.uiCCT_CRC)
            {
                for(uiCount = 0; uiCount < eCM_PICTURE_SETTINGS_NUMBER; uiCount++) //HICC2_Doulas_0003//G100_Steven_0012
                {
                    memcpy(uiWAP_PWM100[uiCount], uCCTTable.sDataStruct.uiPWM100Data[uiCount], (2*eLD_SEG_NUMBERS));
                    memcpy(uiWAP_PWM50[uiCount], uCCTTable.sDataStruct.uiPWM50Data[uiCount], (2*eLD_SEG_NUMBERS));
                    memcpy(uiWAP_PWM30[uiCount], uCCTTable.sDataStruct.uiPWM30Data[uiCount], (2*eLD_SEG_NUMBERS));
                    memcpy(uiWAP_PWM10[uiCount], uCCTTable.sDataStruct.uiPWM10Data[uiCount], (2*eLD_SEG_NUMBERS));
                }
            }
        }
        else //A70LV_John_0118 start fix WAP command transfer
        {
            ucWAP_Init = APCCT_SET_DEFAULT;
            palFormatterMgr_APCCT_Set((UINT8 *)&ucWAP_Init);
        }//A70LV_John_0118 end
    }

	if(Data_X >= APCCT_SET_PWM_100 && Data_X < (APCCT_SET_PWM_100 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_100); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM100[ucWAP_Mode][Data_Y] = GEC_Limit_PWM(Data_Z); //HICC2_Casper_0043
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_100,Data_Y,uiWAP_PWM100[ucWAP_Mode][Data_Y]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, ucWAP_Mode, Data_Y, uiWAP_PWM100[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X >= APCCT_SET_PWM_50 && Data_X < (APCCT_SET_PWM_50 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_50); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM50[ucWAP_Mode][Data_Y] = GEC_Limit_PWM(Data_Z); //HICC2_Casper_0043
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_50,Data_Y,uiWAP_PWM50[ucWAP_Mode][Data_Y]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, ucWAP_Mode, Data_Y, uiWAP_PWM50[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X >= APCCT_SET_PWM_30 && Data_X < (APCCT_SET_PWM_30 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_30); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM30[ucWAP_Mode][Data_Y] = GEC_Limit_PWM(Data_Z); //HICC2_Casper_0043
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_30,Data_Y,uiWAP_PWM30[ucWAP_Mode][Data_Y]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, ucWAP_Mode, Data_Y, uiWAP_PWM30[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X >= APCCT_SET_PWM_10 && Data_X < (APCCT_SET_PWM_10 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_10); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM10[ucWAP_Mode][Data_Y] = GEC_Limit_PWM(Data_Z); //HICC2_Casper_0043
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_10,Data_Y,uiWAP_PWM10[ucWAP_Mode][Data_Y]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, ucWAP_Mode, Data_Y, uiWAP_PWM10[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM100_GET) //0xA0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_100);
        palFormatterMgr_WAP_Get(uiWAP_PWM100[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM100[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM50_GET) //0xB0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_50);
        palFormatterMgr_WAP_Get(uiWAP_PWM50[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM50[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM30_GET) //0xC0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_30);
        palFormatterMgr_WAP_Get(uiWAP_PWM30[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM30[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM10_GET) //0xD0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_10);
        palFormatterMgr_WAP_Get(uiWAP_PWM10[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM10[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_WAP_APPLY) //G50_Casper_0006
    {
        for(uiCount = 0; uiCount < eCM_PICTURE_SETTINGS_NUMBER; uiCount++)
        {
            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_100);
            palFormatterMgr_WAP_Get(uiWAP_PWM100[uiCount]);

            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_50);
            palFormatterMgr_WAP_Get(uiWAP_PWM50[uiCount]);

            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_30);
            palFormatterMgr_WAP_Get(uiWAP_PWM30[uiCount]);

            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_10);
            palFormatterMgr_WAP_Get(uiWAP_PWM10[uiCount]);
        }
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM100Data[0][0], uiWAP_PWM100, sizeof(uiWAP_PWM100));
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM50Data[0][0], uiWAP_PWM50, sizeof(uiWAP_PWM50));
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM30Data[0][0], uiWAP_PWM30, sizeof(uiWAP_PWM30));
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM10Data[0][0], uiWAP_PWM10, sizeof(uiWAP_PWM10));

        uCCTTable.sDataStruct.ulCCT_Tag = CCT_NEW_CALIBRATION_TAG;
        uWAP_Flag = CCT_NEW_CALIBRATION_TAG;

        sGEC_callback.fpCCT_StorageSetCallback(&uCCTTable); //HICC2_Casper_0014
        //palDataMgr_CCT_Set(&uCCTTable);

        palLANProcSendToLAN(edcWAP_ENABLE);

        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, 0, 0, 0);
    }
    else if(Data_X == APCCT_GET_PWM) //0xF1
    {
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM100[Data_Y][Data_Z]);
    }
    else if(Data_X == APCCT_SET_TAG_ZERO) //0xFA //A70LV_John_0118 start fix WAP command transfer //HICC2_Casper_0014
    {
        sGEC_callback.fpCCT_StorageGetCallback(&uCCTTable);
        if(uCCTTable.sDataStruct.ulCCT_Tag == CCT_NEW_CALIBRATION_TAG)
        {
            uCCTTable.sDataStruct.ulCCT_Tag = 0;
            uWAP_Flag = 0;
        }
        else
        {
            uCCTTable.sDataStruct.ulCCT_Tag = CCT_NEW_CALIBRATION_TAG;
            uWAP_Flag = CCT_NEW_CALIBRATION_TAG;
        }
        sGEC_callback.fpCCT_StorageSetCallback(&uCCTTable);
        palLANProcSendToLAN(edcWAP_ENABLE);

        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, 0, 0, 0);
    } //A70LV_John_0118 end
    else if(Data_X == APCCT_GET_DISPLAY_AND_LD) //0xFE ok
    {
        GEC_printf("[%s,0000,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, WAP_PICTURE_MODE_MAX, eLD_SEG_NUMBERS); //G100_Steven_0012
    }
    else if(Data_X == APCCT_SET_DEFAULT) //0xFF //HICC2_Casper_0014
    {
        memset(uiWAP_PWM100, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));  //HICC2_Steven_0019
        memset(uiWAP_PWM50, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));
        memset(uiWAP_PWM30, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));
        memset(uiWAP_PWM10, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));

        palFormatterMgr_APCCT_Set((UINT8 *)&Data_X); // Init DDP WAP PWM //G50_Casper_0006
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, 1, 1, 1);
    }
    else if(Data_X == APCCT_TP_SET) //G100_Steven_0014 start
    {
    	UINT8 ucTP = eCM_TEST_PATTERN_OFF;

    	//GEC_printf("[APCCT_TP_SET %d]",Data_Y);

    	if(Data_Y == eGEC_TP_NONE)
    	{
    		ucTP = eCM_TEST_PATTERN_OFF;
    	}
    	else if(Data_Y == eGEC_TP_WHITE)
		{
    		ucTP = eCM_TEST_PATTERN_WHITE;
		}
    	else if(Data_Y == eGEC_TP_RED)
    	{
    		ucTP = eCM_TEST_PATTERN_RED;
    	}
    	else if(Data_Y == eGEC_TP_GREEN)
    	{
    		ucTP = eCM_TEST_PATTERN_GREEN;
    	}

    	sGEC_callback.fpSetTpCallback(ucTP);
    	//palDataMgr_Data_Access(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucTP) ;
    } //G100_Steven_0014 end
}
//A70LV_John_0079 end //A70LV_John_0077 end

UINT32 GEC_WAP_Flag_Get(void) //G50_Casper_0006
{
    return uWAP_Flag;
}

void GEC_WAP_Flag_Set(UINT32 uFlag) //G50_Casper_0006
{
    uWAP_Flag = uFlag;
}

void GEC_APCCT_Process_HPBU_Tester(UINT32 Data_X, UINT32 Data_Y, UINT32 Data_Z) //G50_Alan_0095 //R70G2_RIC_AC_0034 //remove GEC_printf for HPBU Tester cmd use //G50_Casper_0008
{
#if 1
    static UINT16 uiWAP_PWM100[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    static UINT16 uiWAP_PWM50[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    static UINT16 uiWAP_PWM30[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    static UINT16 uiWAP_PWM10[eCM_PICTURE_SETTINGS_NUMBER][eLD_SEG_NUMBERS]={0};
    UINT8  ucWAP_Mode = 0;
    UINT16 uiCount = 0;
//A70LV_John_0080 start CCT: add CCT initial check function
    //UINT16 uiCRC = 0;
    uCCT_TABLE uCCTTable = {0};
    static UINT8 ucWAPGet = FALSE;
    UINT8  ucWAP_Init = 0;
    UINT8  ucWAP_Level = 0;

    if(ucWAPGet == FALSE)
    {
        ucWAPGet = TRUE;
        sGEC_callback.fpCCT_StorageGetCallback(&uCCTTable); //HICC2_Casper_0014

        if(uCCTTable.sDataStruct.ulCCT_Tag == CCT_NEW_CALIBRATION_TAG) //G50_Casper_0006
        {
            {
                for(uiCount = 0; uiCount < eCM_PICTURE_SETTINGS_NUMBER; uiCount++) //G100_Steven_0012
                {
                    memcpy(uiWAP_PWM100[uiCount], uCCTTable.sDataStruct.uiPWM100Data[uiCount], (2*eLD_SEG_NUMBERS));
                    memcpy(uiWAP_PWM50[uiCount], uCCTTable.sDataStruct.uiPWM50Data[uiCount], (2*eLD_SEG_NUMBERS));
                    memcpy(uiWAP_PWM30[uiCount], uCCTTable.sDataStruct.uiPWM30Data[uiCount], (2*eLD_SEG_NUMBERS));
                    memcpy(uiWAP_PWM10[uiCount], uCCTTable.sDataStruct.uiPWM10Data[uiCount], (2*eLD_SEG_NUMBERS));
                }
            }
        }
        else
        {
            //for(uiCount = 0; uiCount < eCM_PICTURE_SETTINGS_NUMBER; uiCount++)              //G100_Steven_0012	//HICC2_Zonic_0001
            //{
                ucWAP_Init = APCCT_SET_DEFAULT;
                palFormatterMgr_APCCT_Set((UINT8 *)&ucWAP_Init);
            //}
        }
    }

    if(Data_X >= APCCT_SET_PWM_100 && Data_X < (APCCT_SET_PWM_100 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_100); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM100[ucWAP_Mode][Data_Y] = Data_Z;
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_100,Data_Y,uiWAP_PWM100[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X >= APCCT_SET_PWM_50 && Data_X < (APCCT_SET_PWM_50 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_50); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM50[ucWAP_Mode][Data_Y] = Data_Z;
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_50,Data_Y,uiWAP_PWM50[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X >= APCCT_SET_PWM_30 && Data_X < (APCCT_SET_PWM_30 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_30); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM30[ucWAP_Mode][Data_Y] = Data_Z;
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_30,Data_Y,uiWAP_PWM30[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X >= APCCT_SET_PWM_10 && Data_X < (APCCT_SET_PWM_10 + WAP_PICTURE_MODE_MAX)) //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_X - APCCT_SET_PWM_10); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        uiWAP_PWM10[ucWAP_Mode][Data_Y] = Data_Z;
        palFormatterMgr_WAP_Set(ucWAP_Mode,eWAP_POWERLEVEL_10,Data_Y,uiWAP_PWM10[ucWAP_Mode][Data_Y]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM100_GET) //0xA0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_100);
        palFormatterMgr_WAP_Get(uiWAP_PWM100[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM100[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM50_GET) //0xB0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_50);
        palFormatterMgr_WAP_Get(uiWAP_PWM50[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM50[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM30_GET) //0xC0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_30);
        palFormatterMgr_WAP_Get(uiWAP_PWM30[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM30[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_DDP_NATIVE_PWM10_GET) //0xD0 ok //G50_Casper_0006
    {
        ucWAP_Mode = GEC_MapToCM_PicMode(Data_Y); //Mapping to CM_Picture mode
        if(ucWAP_Mode >= eCM_PICTURE_SETTINGS_NUMBER)
        {
            return;
        }
        palFormatterMgr_WAP_SetGet(ucWAP_Mode,eWAP_POWERLEVEL_10);
        palFormatterMgr_WAP_Get(uiWAP_PWM10[ucWAP_Mode]);
        GEC_printf("[%s,%04d,%04d,%04d]",sGEC_CommandLUT[eGEC_APCCT].GEC_Command, Data_Y, Data_Z, uiWAP_PWM10[ucWAP_Mode][Data_Z]);
    }
    else if(Data_X == APCCT_WAP_APPLY) //G50_Casper_0006
    {
        for(uiCount = 0; uiCount < eCM_PICTURE_SETTINGS_NUMBER; uiCount++)
        {
            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_100);
            palFormatterMgr_WAP_Get(uiWAP_PWM100[uiCount]);

            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_50);
            palFormatterMgr_WAP_Get(uiWAP_PWM50[uiCount]);

            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_30);
            palFormatterMgr_WAP_Get(uiWAP_PWM30[uiCount]);

            palFormatterMgr_WAP_SetGet(uiCount,eWAP_POWERLEVEL_10);
            palFormatterMgr_WAP_Get(uiWAP_PWM10[uiCount]);
        }
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM100Data[0][0], uiWAP_PWM100, sizeof(uiWAP_PWM100));
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM50Data[0][0], uiWAP_PWM50, sizeof(uiWAP_PWM50));
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM30Data[0][0], uiWAP_PWM30, sizeof(uiWAP_PWM30));
        memcpy((UINT8*)&uCCTTable.sDataStruct.uiPWM10Data[0][0], uiWAP_PWM10, sizeof(uiWAP_PWM10));

        uCCTTable.sDataStruct.ulCCT_Tag = CCT_NEW_CALIBRATION_TAG;
        uWAP_Flag = CCT_NEW_CALIBRATION_TAG;

        sGEC_callback.fpCCT_StorageSetCallback(&uCCTTable); //HICC2_Casper_0014
    }
    else if(Data_X == APCCT_SET_TAG_ZERO) //0xFA //A70LV_John_0118 start fix WAP command transfer
    {
        uCCTTable.sDataStruct.ulCCT_Tag = 0;
        sGEC_callback.fpCCT_StorageSetCallback(&uCCTTable); //HICC2_Casper_0014

    } //A70LV_John_0118 end
    else if(Data_X == APCCT_SET_DEFAULT) //0xFF
    {
        memset(uiWAP_PWM100, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));  //HICC2_Steven_0019
        memset(uiWAP_PWM50, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));
        memset(uiWAP_PWM30, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));
        memset(uiWAP_PWM10, 0, (2*eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS));

        palFormatterMgr_APCCT_Set((UINT8 *)&Data_X); // Init DDP WAP PWM //G50_Casper_0006
    }
#endif
}

eCM_PICTURE_SETTINGS_ID GEC_MapToCM_PicMode(eWAP_PICTURE_SETTINGS_ID ePicMode)
{
	UINT8 ucDisplayModeID = (UINT8)GUI2CM(edcPICTURE_SETTINGS, ePicMode);

	if(ucDisplayModeID > eCM_PICTURE_SETTINGS_NUMBER)
    {
        GEC_printf("(func:%s, line:%d) map fail (%d)\r\n", __FUNCTION__, __LINE__, ePicMode);
        return eCM_PICTURE_SETTINGS_NUMBER;
    }
	return (eCM_PICTURE_SETTINGS_ID)ucDisplayModeID;
}

UINT16 GEC_Limit_PWM(UINT16 uiPWM)
{
	if((INT16)uiPWM < 0)
	{
		return 0;
	}
	else
	{
		return uiPWM;
	}
}
	
