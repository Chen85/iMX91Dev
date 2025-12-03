// ==============================================================================
// FILE NAME: UTILCLICMD.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 2014/02/06, Leo Create
// --------------------
// ==============================================================================


#include "utilDbgMsg.h"
#include "utilHostAPI.h"
#include "utilCLICmdAPI.h"
#include "utilCommonMSSCAPI.h"
#include "utilDataMgrAPI.h"
#include "utilOPD_TEST.h"
#include "utilDatabaseAPI.h"
//#include "utilDataMapping.h"
#include "utilCommon.h"
#include "utilIPCAPI.h"
#ifdef CUSTOM_CHRISTIE
#include "utilChristieMSSCAPI.h"
#endif

#include "halInputCtrlAPI.h"
#include "halWarping.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif
#include "halBoardCtrlAPI.h"

#include "PNGUtility.h"

#include "cmd_ap.h"
#include "dvLDDriver.h"
#include "dvMCUDriver.h"
#include "dvFrontEndDriver.h"

#include "ProjectSettings.h"
#include "CommonAPI.h"

#include "appDataMgr.h"
#include "appInputProc.h"
#include "appLANProcAPI.h"
#include "appIllumination.h"
#include "palMotorMgr.h"
#include "appSystem.h"


UINT8 m_ucStressBuff[1] = {0};  //G100_Owen_0080

///////////////// callback function start ///////////////
sUTILCLICMD_CALLBACK sUtilCLICmd_Callback;
void utilCLICmd_RegCallback(sUTILCLICMD_CALLBACK fpCallback)
{
    sUtilCLICmd_Callback = fpCallback;
}
////////////////// callback function end ////////////////

#ifdef PALDATAMGR_ACCESS_WITHLOG
#define fpDataMgr_Data_AccessCb(a,b,c) fpDataMgr_Data_AccessCb(a,b,c,__FUNCTION__,__LINE__)
#endif

void DrawPNG_Test(void);

static eRESULT utilCLI_System_Version_Get(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT8 *pcBuffer = (UINT8 *)psOutData;

    pcBuffer[0] = VER_MINOR;
    pcBuffer[1] = VER_MAJOR;

    *pwByteCount = 2;

    return rcSUCCESS;
}

//T100 : frontend board 傳送 cmd 來更新 scaler board 的 Fan infomation
static eRESULT utilCLI_Fan_Info_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    //LOG_MSG(db_ALWAYS, "\r\nutilCLI_Fan_Info_Update\r\n") ;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcFAN_INFORMATION, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;

    *pwByteCount = 0;

    return rcSUCCESS ;
}

//T100 : frontend board 傳送 cmd 來更新 scaler board 的 Temperature infomation
static eRESULT utilCLI_System_Temperature_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    //LOG_MSG(db_ALWAYS, "\r\nutilCLI_System_Temperature_Update\r\n") ;

    INT16 *uiData = (INT16*)psInData->aucData;
    INT16 uiTemp = 0;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTHERMAL_SENSOR_INFORMATION, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData);

    //for X35 //A35G2_CDS_Larry_0001
    #ifdef PLATFORM_A35G2
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[3]) ;
	#elif defined(PLATFORM_H30_4K)
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[7]) ; //A70Gen2_Larry_0005
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[8]) ; //A70Gen2_Larry_0005
	#else
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTHERMAL_SENSOR_8, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[6]); //HICC2_Doulas_0098 L1G sensor
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[7]) ; //A70Gen2_Larry_0005
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[8]) ; //A70Gen2_Larry_0005

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE2, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[9]) ;  //HICC2_Steven_0014
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE2, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[10]) ;  //HICC2_Steven_0014

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE3, edaWRITE_RAM_ONLY_NO_ACTION, &uiTemp);//Reserved.
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE3, edaWRITE_RAM_ONLY_NO_ACTION, &uiTemp);//Reserved.

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcABSOLUTE_HUMIDITY1, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[11]);//HICC2_Julie_0006
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcABSOLUTE_HUMIDITY2, edaWRITE_RAM_ONLY_NO_ACTION, &uiData[12]);//HICC2_Julie_0006
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcABSOLUTE_HUMIDITY3, edaWRITE_RAM_ONLY_NO_ACTION, &uiTemp);//Reserved.
    #endif

    *pwByteCount = 0;

    return rcSUCCESS ;
}
static eRESULT utilCLI_TEC_Current_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)	//G100_Clare_0017
{
    //LOG_MSG(db_ALWAYS, "\r\nutilCLI_System_Temperature_Update\r\n") ;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTECCURRENT_1, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTECCURRENT_2, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData + 2) ;
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTECCURRENT_3, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData    + 4) ;

    *pwByteCount = 0;

    return rcSUCCESS ;
}

static eRESULT utilCLI_Dimming_Mode(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
#ifdef CURSOR_FIXTURE

#else
    UINT8 *pcInBuffer  = (UINT8*)psInData->aucData;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcDIMMING_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&pcInBuffer[0]);
#endif
    *pwByteCount = 0;

    return rcSUCCESS ;
}

static eRESULT utilCLI_Press_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)	//G100_Clare_0017
{
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData + 2) ;//G100_Clare_0049

    *pwByteCount = 0;

    return rcSUCCESS ;
}

//G100_Clare_0047, add, >>>
static eRESULT utilCLI_TE_ID_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)	//G100_Clare_0017
{
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTE_ID, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;

    *pwByteCount = 0;

    return rcSUCCESS ;
}
//G100_Clare_0047, add, <<<
static eRESULT utilCLI_GSensorXYZ_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)	//G100_Clare_0017
{
    //LOG_MSG(db_ALWAYS, "\r\nutilCLI_System_Temperature_Update\r\n") ;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcGSensor_XYZ_Value, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;

    *pwByteCount = 0;

    return rcSUCCESS ;
}

static eRESULT utilCLI_XFPGA_Ready(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)   //G100_Owen_0052
{
    UINT8 aucVersion[2] = {0};
    UINT8 aucVerString[32] = {0};
    eRESULT eResut = rcERROR;

    LOG_MSG(db_APP_SYSTEM, "utilCLI_XFPGA_Ready\r\n");

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcXFPGA_VERSION, edaREAD, aucVerString);

    if(psInData->aucData[0] == TRUE)    //ready //G100_Simon_0059
    {
        eResut  = halBoard_XillinxFPGA_Version_BL_Get(&aucVersion[0]);  //A35G2_Simon_0065
        eResut &= halBoard_XillinxFPGA_Version_BH_Get(&aucVersion[1]);  //A35G2_Simon_0065

        if(eResut == rcSUCCESS)
        {
            LOG_MSG(db_APP_SYSTEM, "XFPGA  Q%02d.%02d\n", aucVersion[1], aucVersion[0]);
            sprintf((char*)aucVerString, "Q%02d.%02d", aucVersion[1], aucVersion[0]);
            sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcXFPGA_VERSION, edaWRITE_RAM_ONLY_NO_ACTION, aucVerString);
        }
        else
        {
            LOG_MSG(db_APP_SYSTEM, "XFPGA version get fail\n");
        }
    }

    *pwByteCount = 0;

    return rcSUCCESS ;
}


// ==============================================================================
// FUNCTION NAME: utilLD_LightSensor_Set
// DESCRIPTION:
//
//
// Params:
// sPAYLOAD *psInData:
// sPAYLOAD *psOutData:
// UINT16 *pwByteCount:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/04/25, Larry Create
// --------------------
// ==============================================================================
static eRESULT utilCLI_LightSensor_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT16 *uiData = (UINT16*)psInData->aucData;
    UINT8   ucCount = 0;
    UINT16  uiLightSensorValue[5] = {0};
	INT32 LightSensorInfoType = Syscfg_Value_Get_Typeint(eDC_LightSensorInfoType); //HICC2_Doulas_0008

    for(ucCount = 0; ucCount<5; ucCount++) //W/Y/R/B/G
    {
        if(uiData[ucCount]/10 >= 0xFFFF)
        {
            uiLightSensorValue[ucCount] = 0xFFFF;
        }
        else
        {
            uiLightSensorValue[ucCount] = uiData[ucCount];
        }

		if(LightSensorInfoType == eDC_LIGHT_SENSOR_INFO_TYPE_1) //HICC2_Doulas_0008
		{
			switch(ucCount)
			{
				case (eLD_SEQ_R+1) :
					sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((eDATA_CODE)(edcLIGHT_SENSOR_INFO_1 ), edaWRITE_RAM_ONLY_NO_ACTION, (void*)&uiLightSensorValue[ucCount]);
					break;

				case (eLD_SEQ_G+1) :
					sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((eDATA_CODE)(edcLIGHT_SENSOR_INFO_2 ), edaWRITE_RAM_ONLY_NO_ACTION, (void*)&uiLightSensorValue[ucCount]);
					break;

				case (eLD_SEQ_B+1) :
					sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((eDATA_CODE)(edcLIGHT_SENSOR_INFO_3 ), edaWRITE_RAM_ONLY_NO_ACTION, (void*)&uiLightSensorValue[ucCount]);
					break;

				case (eLD_SEQ_Y+1) :
					sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((eDATA_CODE)(edcLIGHT_SENSOR_INFO_4 ), edaWRITE_RAM_ONLY_NO_ACTION, (void*)&uiLightSensorValue[ucCount]);
					break;
			}
		}
		else
		{
        	sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((eDATA_CODE)(edcLIGHT_SENSOR_INFO_1 + ucCount), edaWRITE_RAM_ONLY_NO_ACTION, (void*)&uiLightSensorValue[ucCount]);
		}
    }
    LOG_MSG(db_APP_ILLUMINATION, "Light Sensor[W/Y/R/B/G] %d,%d,%d,%d,%d\r\n", //HICC2_Doulas_0007 Modify
        uiLightSensorValue[0], uiLightSensorValue[1], uiLightSensorValue[2], uiLightSensorValue[3], uiLightSensorValue[4]);

    //palIllumination_LightSensorValueSet(uiLightSensorValue); //T100_Larry_0017

    *pwByteCount = 0;

    return rcSUCCESS;
}

//T100 : frontend board 傳送 cmd 來更新 scaler board 的 LD infomation
static eRESULT utilCLI_System_LD_Info_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    //LOG_MSG(db_ALWAYS, "\r\nutilCLI_System_LD_Info_Update\r\n") ;

#ifdef PLATFORM_A35G2
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcLD_INFORMATION, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;
#else  //A70G2
    WORD *pcInBuffer  = (WORD*)psInData->aucData;
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcLD_INFORMATION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&pcInBuffer[0]);
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTECCURRENT_1, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&pcInBuffer[48]);
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcTECCURRENT_2, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&pcInBuffer[50]);
#endif

    *pwByteCount = 0;

    return rcSUCCESS ;
}

static eRESULT utilCLI_Pump_Info_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)	//HICC2_Doulas_0009
{
    WORD *pcInBuffer  = (WORD*)psInData->aucData;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcPUMP_RPM, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&pcInBuffer[0]);

    *pwByteCount = 0;

    return rcSUCCESS;
}

//A35G2_BRC_Casper_0117: frontend board 傳送 cmd 來更新 ProService Temperature Status
static eRESULT utilCLI_System_ProServiceTemperatureSet(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT8 ucCurrentStatus = 0;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcPROSERVICE_TEMPERATURE_STATUS, edaREAD, &ucCurrentStatus);
    //LOG_MSG(db_ALWAYS, "utilCLI_System_ProServiceTemperatureSet[%d]\r\n",psInData->aucData[0]) ;
    if(ucCurrentStatus != psInData->aucData[0])
    {
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcPROSERVICE_TEMPERATURE_STATUS, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData);
    }

    *pwByteCount = 0;

    return rcSUCCESS ;
}

//A35G2_BRC_Casper_0117: frontend board 傳送 cmd 來更新 ProService Temperature Status
static eRESULT utilCLI_System_ProServiceFanStatusSet(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT8 ucCurrentStatus = 0;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcPROSERVICE_FAN_STATUS, edaREAD, &ucCurrentStatus);
    //LOG_MSG(db_ALWAYS, "utilCLI_System_ProServiceFanStatusSet[%d]\r\n",psInData->aucData[0]) ;

    if(ucCurrentStatus != psInData->aucData[0])
    {
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcPROSERVICE_FAN_STATUS, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData);
    }

    *pwByteCount = 0;

    return rcSUCCESS ;
}

static eRESULT utilCLI_WheelSpeedPolling(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    WORD *pcInBuffer  = (WORD*)psInData->aucData;

    palDataMgr_WheelSpeedPollingSetting(*pcInBuffer, *(pcInBuffer+1));

    *pwByteCount = 0;

    return rcSUCCESS ;
}

//T100 : frontend board 傳送 cmd 來更新 scaler board 的 Fan infomation
static eRESULT utilCLI_IR_decode(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sKEY_DATA *pcInBuffer  = (sKEY_DATA*)psInData->aucData;
//    UINT8 ucCount = 0;

    LOG_MSG(db_APP_INPUTKEY, "pcInBuffer->eKeyEvent %d\r\n", pcInBuffer->eKeyEvent);
    LOG_MSG(db_APP_INPUTKEY, "pcInBuffer->wKeyCode %d\r\n", pcInBuffer->wKeyCode);
    LOG_MSG(db_APP_INPUTKEY, "pcInBuffer->eKeyType %d\r\n", pcInBuffer->eKeyType);

    sUtilCLICmd_Callback.fpInputProc_BufferInsertCb(pcInBuffer);

    *pwByteCount = 0;

    return rcSUCCESS;
}

static eRESULT utilCLI_HostReady(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT8 ucData = psInData->aucData[0];

    switch(sUtilCLICmd_Callback.fpSystem_PowerStateGetCb())
    {
        case ePOWER_STATE_RESET:
        case ePOWER_STATE_STANDBY:
            if(sUtilCLICmd_Callback.fpSystem_HostReadyGetCb() == 0)
            {
                if(ucData == FACTORY_RESET_NUMBER)
                {
                    sUtilCLICmd_Callback.fpDataMgr_ResetAllToDefaultCb();
					utilOPD_ResetAllToDefault(); //G100_Julie_0024
                }

                sUtilCLICmd_Callback.fpSystem_WarmUpCb();
            }
        default:
            sUtilCLICmd_Callback.fpSystem_HostReadySetCb(1);
            break;
    }

    LOG_MSG(db_UTL_CLI, "PowerState = %d, utilCLI_HostReady\r\n", sUtilCLICmd_Callback.fpSystem_PowerStateGetCb());

    *pwByteCount = 0;

    return rcSUCCESS;
}


static eRESULT utilCLI_System_State(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT32 ulMCU_State = 0 ;
    memcpy((UINT8 *)&ulMCU_State , (UINT8 *)psInData->aucData , sizeof(UINT32));

    palSystem_SetMcuState((UINT8)ulMCU_State);
    LOG_MSG(db_UTL_CLI, "MCU state %d\r\n", ulMCU_State);
    *pwByteCount = 0;
    return rcSUCCESS ;
}

static eRESULT utilCLI_System_ErrorLog_Notify(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    //UINT32 uiSystime_Minutes = palIllumination_TotalProjectSec_Get();
    UINT32 ulErrorIndex = 0 ;
#ifdef PLATFORM_H30_4K
	UINT8 ucValue = ets_OFF;
#endif

    memcpy((UINT8 *)&ulErrorIndex , (UINT8 *)psInData->aucData , sizeof(UINT32));

    utilDataMgr_ErrorRefValSet(0);
    if(strlen(psInData->aucData) > 4)
    {
        INT32 lTemp;
        memcpy((UINT8 *)&lTemp , (UINT8 *)psInData->aucData+4 , sizeof(UINT32));
        utilDataMgr_ErrorRefValSet(lTemp);
    }

    LOG_MSG(db_UTL_CLI, "ulErrorIndex %x\r\n", ulErrorIndex);


    #ifdef CUSTOM_OPTOMA		//A65_OPTOMA_Doulas_0069 //A35G2_Coda_0062
	palSystem_Optoma_Error_Message(ulErrorIndex);
	#endif

    utilDataMgr_WriteGecLog_CM(ulErrorIndex); //HICC2_Doulas_0031

    *pwByteCount = 0;
    return rcSUCCESS ;
}

static eRESULT utilCLI_System_PowerOff(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT8 ucData = 0 ;  //G100_Clare_0055
    printf("utilCLI_System_PowerOff\r\n");

    if(Board_Stage_Get() > EVT_STAGE)  //G100_Owen_0076
    {
        sUtilCLICmd_Callback.fpEnvironment_InstantCoolingCb();
        //G100_Clare_0055, add, >>>
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcFAST_POWER_ON, edaREAD, &ucData);
        if (ucData == TRUE)
        {
            sUtilCLICmd_Callback.fpEnvironment_Fake_Power_Down_SetCb(TRUE);
        }
        else
        //G100_Clare_0055, add, <<<
        {
            sUtilCLICmd_Callback.fpSystem_PowerDownCb();
        }
    }

    *pwByteCount = 0;
    return rcSUCCESS ;
}

static eRESULT utilCLI_System_HDBaseTCmd_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount) //G100_Owen_0016
{
    UINT8 cCh = eccHDBaseT;
    char ucReturnString[MSSC_CMD_LENGTH_MAX] = {'\0'};
#if 0
    if(!strcmp(psInData->aucData, "AMX")) //A35G2_CDS_Coda_0030
    {
        UINT8 ucString[32] ={0};	//A35G2_CDS_Coda_0031
        UINT8 ucCustomID = sUtilCLICmd_Callback.fpSystem_ModelIDGetCb();
        if(ucCustomID < eMODEL_TYPE_NUMBER)
        {
            sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcMODEL_NAME, edaREAD, ucString);	//A35G2_CDS_Coda_0031
            snprintf(ucReturnString, MSSC_CMD_LENGTH_MAX, AMXB_STR, ucString);
            utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, strlen(ucReturnString) + 1, ucReturnString);
        }
    }
    else if(psInData->aucData[0] == GENERAL_CMD_HEADER_1)
    {
        sCLI_GENERAL_FORMAT sCmdFormat = {0};

        sCmdFormat.ucCmdFrom = cCh;
        utilGeneral_CLI_Process(cCh, psInData->aucData, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
    }
#endif

#if 1
#ifdef CUSTOM_CHRISTIE
    if(!strcmp(psInData->aucData, "AMX")) //A35G2_CDS_Coda_0030
    {
        UINT8 ucString[32] ={0};	//A35G2_CDS_Coda_0031
        UINT8 ucCustomID = sUtilCLICmd_Callback.fpSystem_ModelIDGetCb();
        if(ucCustomID < eMODEL_TYPE_NUMBER)
        {
            sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcMODEL_NAME, edaREAD, ucString);	//A35G2_CDS_Coda_0031
            snprintf(ucReturnString, MSSC_CMD_LENGTH_MAX, AMXB_STR_CHRISTIE, ucString);
            utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, strlen(ucReturnString) + 1, ucReturnString);
        }
    }
    else if(psInData->aucData[0] == CHRISTIE_CMD_HEADER_1)
    {
        sCLI_CHRISTIE_FORMAT sCmdFormat = {0};

        sCmdFormat.ucCmdFrom = cCh;
        utilChristie_CLI_Process(cCh, psInData->aucData, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
    }
#elif CUSTOM_OPTOMA
    if(!strcmp(psInData->aucData, "AMX")) //A35G2_CDS_Coda_0030
    {
        UINT8 ucString[32] ={0};	//A35G2_CDS_Coda_0031
        UINT8 ucCustomID = sUtilCLICmd_Callback.fpSystem_ModelIDGetCb();
        if(ucCustomID < eMODEL_TYPE_NUMBER)
        {
            sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcMODEL_NAME, edaREAD, ucString);	//A35G2_CDS_Coda_0031
            snprintf(ucReturnString, MSSC_CMD_LENGTH_MAX, AMXB_STR, ucString);
            utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, strlen(ucReturnString) + 1, ucReturnString);
        }
    }
    else if(psInData->aucData[0] == OPTOMA_CMD_HEADER_1)
    {
        sCLI_OPTOMA_FORMAT sCmdFormat = {0};

        sCmdFormat.ucCmdFrom = cCh;
        utilOptoma_CLI_Process((eCLI_CHANNEL)cCh, psInData->aucData, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
    }
    else if(psInData->aucData[0] == GENERAL_CMD_HEADER_1) //A35G2_Coda_0059
    {
        sCLI_GENERAL_FORMAT sCmdFormat = {0};

        sCmdFormat.ucCmdFrom = cCh;
        utilGeneral_CLI_Process(cCh, psInData->aucData, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
    }
#else
    if(!strcmp(psInData->aucData, "AMX")) //A35G2_CDS_Coda_0030
    {
        UINT8 ucString[32] ={0};	//A35G2_CDS_Coda_0031
        UINT8 ucCustomID = sUtilCLICmd_Callback.fpSystem_ModelIDGetCb();
        if(ucCustomID < eMODEL_TYPE_NUMBER)
        {
            sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcMODEL_NAME, edaREAD, ucString);	//A35G2_CDS_Coda_0031
            snprintf(ucReturnString, MSSC_CMD_LENGTH_MAX, AMXB_STR, ucString);
            utilHost_SystemSet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_HDBASET, strlen(ucReturnString) + 1, ucReturnString);
        }
    }
    else if(psInData->aucData[0] == GENERAL_CMD_HEADER_1)
    {
        sCLI_GENERAL_FORMAT sCmdFormat = {0};

        sCmdFormat.ucCmdFrom = cCh;
        utilGeneral_CLI_Process(cCh, psInData->aucData, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
    }
#endif
#endif

    *pwByteCount = 0;
    return rcSUCCESS ;
}

// ==============================================================================
// FUNCTION NAME: utilCLI_System_Enter2WMode
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
// 2020/11/05, Larry Create
// --------------------
// ==============================================================================
static eRESULT utilCLI_System_Enter2WMode(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sUtilCLICmd_Callback.fpSystem_Enter2WModeCb();

    *pwByteCount = 0;
    return rcSUCCESS ;
}

static eRESULT utilCLI_System_FastPowerOnNotifyFromMCU(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)	//G100_Coda_00107
{
    if(sUtilCLICmd_Callback.fpEnvironment_Fake_Power_Down_GetCb())
    {
        sUtilCLICmd_Callback.fpGui_SendKeyEventCb(eKEYINPUT_KEYPAD, kePOWERON);
        //palEnvironment_Fake_Power_Down_Set(FALSE);
    }
    *pwByteCount = 0;
    return rcSUCCESS ;
}

static eRESULT utilCLI_System_LVPS_Info_Update(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount) //HICC2_Doulas_0165
{
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcLVPS_INFO, edaWRITE_RAM_ONLY_WITH_ACTION, psInData->aucData);

    *pwByteCount = 0;

    return rcSUCCESS ;
}

static eRESULT utilCLI_System_DimPowerSet(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    UINT8 *pcInBuffer  = (UINT8*)psInData->aucData;
    //UINT8 ucValue = 90;//TEMP_DIMPOWER_LEVEL; //###

    //appDataMgr_Data_Access(edcDIMMING_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&pcInBuffer[0]);
    #if 1
	sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcDIMMING_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&pcInBuffer[0]);
	#endif
    *pwByteCount = 0;

    return rcSUCCESS ;
}


#ifdef PLATFORM_H30_4K
static eRESULT utilCLI_System_BurnIn_Control(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
	#define eBURN_IN_CMD_NEXT_PATTERN   -2
	#define eBURN_IN_CMD_PROJ_OFF       -1

    INT8 cData = psInData->aucData[0];

	if(cData == eBURN_IN_CMD_PROJ_OFF)	//burn in power off
	{
    	palIllumination_BurnIn_PowerOffSet(TRUE);
	}
	else if(cData == eBURN_IN_CMD_NEXT_PATTERN) //change pattern
	{
		palDataMgr_UI_EventSend(edcUI_EVENT_BURNIN_PATTERN_UPDATE, TRUE, NULL);
	}

    *pwByteCount = 0;
    return rcSUCCESS ;
}
#endif



// ==============================================================================
// FUNCTION NAME: utilCLI_LAN_Data_Get
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
// 2018/01/09, Larry Create //A70LV_Larry_0068
// --------------------
// ==============================================================================
static eRESULT utilCLI_LAN_Data_Get(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
#if 0
    sLAN_DATA_FORMAT *sDataIn  = (sLAN_DATA_FORMAT*)psInData->aucData;
    sLAN_DATA_FORMAT *sDataOut = (sLAN_DATA_FORMAT*)psOutData;

    UINT16 uiLen = 0;
    INT32  iValue = 0;

    sDataOut->uiDataCode        = sDataIn->uiDataCode;
    sDataOut->ucType            = sUtilCLICmd_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)sDataOut->uiDataCode);
    sDataOut->ucEnable          = (UINT8)sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)sDataOut->uiDataCode);
    sDataOut->uData.lDataValue  = 0;

    if(sUtilCLICmd_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)sDataOut->uiDataCode) != DATA_TYPE_NA)
    {
        if((DATA_TYPE_I_DIGIT_64 >= sDataOut->ucType) && (DATA_TYPE_UI_DIGIT_8 <= sDataOut->ucType))
        {
            sUtilCLICmd_Callback.fpGui_DataCode_Value_GetCb(sDataOut->uiDataCode, &iValue);
            if(sDataOut->uiDataCode == edcMAIN_INPUT || sDataOut->uiDataCode == edcSUB_INPUT)   //T100_Simon_0038 Start
            {
                iValue = (INT32)palDataMgr_SystemSrcID_To_UserSrcID((UINT8)iValue) ;
            }                                                                                //T100_Simon_0038 End

            sDataOut->uData.lDataValue = iValue;
            uiLen = 4;

            LOG_MSG(db_APP_CLI_LAN,"#Data %d get Dec %d\n", sDataOut->uiDataCode, iValue);
        }
        else if(sDataOut->ucType == DATA_TYPE_STRING)
        {
            appGui_DataCode_String_Get(sDataOut->uiDataCode, sDataOut->uData.acBuffer);
            uiLen = strlen(sDataOut->uData.acBuffer);

            LOG_MSG(db_APP_CLI_LAN,"#Data %d get Str (%d)%s\n", sDataOut->uiDataCode, uiLen, sDataOut->uData.acBuffer);
        }
        else
        {
            LOG_MSG(db_APP_CLI_LAN,"#Data get not found\n");
        }
    }

    *pwByteCount = OFFSET_SUB_DATA_VALUE + uiLen;
#endif /* 0 */
   *pwByteCount = 0;

    return rcSUCCESS;
}

// ==============================================================================
// FUNCTION NAME: utilCLI_LAN_Data_Set
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
// 2018/01/09, Larry Create //A70LV_Larry_0068
// --------------------
// ==============================================================================
static eRESULT utilCLI_LAN_Data_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    eEXEC_CODE eResult = eEXEC_CODE_FAIL;
    sLAN_DATA_FORMAT *sDataIn   = (sLAN_DATA_FORMAT*)psInData->aucData;
    sLAN_DATA_FORMAT *sDataOut  = (sLAN_DATA_FORMAT*)psOutData;
    UINT16 uiDataCode = sDataIn->uiDataCode;
    UINT16 uiLen = 0;
    UINT8  ucType = 0;
    UINT8  ucFreeze = 0;
	//A35G2_Wesley_0137
    UINT8  ucAuto_Focus_Status = 0;
    UINT8  ucAuto_Wall_Color_Status = 0;
    UINT8  ucACU_Status = 0;
    _sGROUPING_CLIENT_RESULT sGroupAFSStatus = {0};
    _sGROUPING_CLIENT_RESULT sGroupACMStatus = {0};

    ucType = sUtilCLICmd_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)sDataIn->uiDataCode);

    sDataOut->uiDataCode        = uiDataCode;
    sDataOut->ucType            = ucType;
    sDataOut->ucEnable          = (UINT8)sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)sDataOut->uiDataCode);
    sDataOut->uData.lDataValue  = 0;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcIMAGE_FREEZE, edaREAD, &ucFreeze);  //G100_Simon_0030

	//A35G2_Wesley_0137
    //palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaREAD, &ucAuto_Focus_Status);
    //palDataMgr_Data_Access(edcAUTO_WALL_COLOR_STATUS, edaREAD, &ucAuto_Wall_Color_Status);
    //palDataMgr_Data_Access(edcACU_STATUS, edaREAD, &ucACU_Status);

	//HICC2_Steven_0047
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcAUTO_FOCUS_STATUS, edaREAD, &ucAuto_Focus_Status);
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcAUTO_WALL_COLOR_STATUS, edaREAD, &ucAuto_Wall_Color_Status);
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcACU_STATUS, edaREAD, &ucACU_Status);
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcGROUP_AUTO_FOCUS_STATUS, edaREAD, &sGroupAFSStatus);
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcGROUP_AUTO_COLOR_MATCH_STATUS, edaREAD, &sGroupACMStatus);

#if 1 //HICC2_Doulas_0134//need to check PIN_Protect
  	//G100_Coda_0095
    if((sUtilCLICmd_Callback.fpGui_PIN_Protect_CheckingCb() == ePASSWORD_PROTECT_LOCKED)
	&& (sUtilCLICmd_Callback.fpEnvironment_NetworkIsReceivePowerOnCmd_GetCb() == TRUE)
	&& (uiDataCode != edcPICTURE_MUTE) && (uiDataCode != edcDIRECT_PICTUREMUTE)	/*&& uiDataCode != edcLENS_CALIBRATION*/
	&& (uiDataCode != edcPIN_PROTECT) && (uiDataCode != edcWEB_PIN_PROTECT_EXECUTE) && (uiDataCode != edcWLAN_LINK_STATUS)
    && (uiDataCode != edcSYSTEM_POWER) && (uiDataCode != edcPOWER_STATUS) && (uiDataCode != edcACTION_KEY_CODE) //A65_OPTOMA_Julie_0085) //A35G2_Coda_0123
	&& (uiDataCode != edcCAMERA_FW_VERSION) && (uiDataCode != edcCAMERA_MODULE_STATUS) && (uiDataCode != edcACTION_TELNET_CLI)) //HICC2_Steven_0060
    {
    	LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (PIN_Protect)\n", uiDataCode);
    	return eEXEC_CODE_FAIL;
    }
#endif
    LOG_MSG(db_APP_CLI_LAN, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    LOG_MSG(db_APP_CLI_LAN, "DataCode[%d]%s Type[%d] En[%d]\n", sDataIn->uiDataCode
                                                              , sUtilCLICmd_Callback.fpDataMgr_DataCodeStringGetCb(sDataIn->uiDataCode)
                                                              , sDataIn->ucType
                                                              , sDataIn->ucEnable);

    if(sDataIn->ucType == DATA_TYPE_STRING)
    {
        LOG_MSG(db_APP_CLI_LAN, "DataValue %s\n", sDataIn->uData.acBuffer);
    }
    else
    {
        LOG_MSG(db_APP_CLI_LAN, "DataValue %d\n", sDataIn->uData.lDataValue);
    }

    if(ucType != DATA_TYPE_NA)
    {
        if((DATA_TYPE_I_DIGIT_64 >= ucType) && (DATA_TYPE_UI_DIGIT_8 <= ucType))
        {
            INT32   iMax = 0;
            INT32   iMin = 0;
            INT32   iValue = sDataIn->uData.lDataValue;

            if((eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMAX, &iMax)) &&
                (eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMIN, &iMin)))
            {
                if(iValue > iMax)
                {
                    iValue = iMax;
                    LOG_MSG(db_APP_CLI_LAN,"&Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }
                else if(iValue < iMin)
                {
                    iValue = iMin;
                    LOG_MSG(db_APP_CLI_LAN,"&Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }

                uiLen = 4;

                if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
                   sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE )  //G100_Simon_0030  //G100_Simon_0036
                {
                    LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (grayout)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucFreeze) &&
                        ((uiDataCode != edcIMAGE_FREEZE &&
                          uiDataCode != edcSUPPORT_MESSAGE &&
                          uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                        #ifdef CUSTOM_CHRISTIE// x35G2_Bruce_0004
                          uiDataCode != edcImageReset &&
                        #endif
                          uiDataCode != edcEMERGENCY_MESSAGE_CLEAN &&
                          uiDataCode != edcPICTURE_MUTE && //G100_Larry_0035
                          uiDataCode != edcDIRECT_PICTUREMUTE &&
                          uiDataCode != edcBACKUP_RESTORE_WEB_SAVE) && //G100_Larry_0039
                          uiDataCode != edcLAN_PATH_SWITCH &&   //A35G2_CDS_Simon_0010
						  uiDataCode != edcSYSTEM_POWER &&
                          uiDataCode != edcOPDFILE_COPYTOWEB && //HICC2_Steven_0059 fixed ISS-0026337
						  !(uiDataCode >= edcIPV6_DHCP && uiDataCode <= edcIPV6_DNS) && //HICC2_AC_0055 // HICC2_Bruce_0020
                          !(uiDataCode >= edcLAN_DHCP && uiDataCode <= edcWLAN_SSID) &&
                          !(uiDataCode >= edcCrestron && uiDataCode <= edcHTTP)))
                {
                    LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (freeze)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if(((ucAuto_Focus_Status == eAF_STATUS_PROCESSING) || (sGroupAFSStatus.clientRet == eGCS_STATUS_PROCESSING))  &&  //AF //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_FOCUS_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcOSD_SHOW &&
                         uiDataCode != edcGEO_OSD_ON &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_DRAW_FS_CHECKBOARD_ON_OSD &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_WARP_OSD_QUEUE_DRAW &&  //H2PF_Simon_0039
                         uiDataCode != edcNON &&
                         uiDataCode != edcFOCUS_MOVE &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if(((ucAuto_Wall_Color_Status == eAWC_STATUS_PROCESSING) || (sGroupACMStatus.clientRet == eGCS_STATUS_PROCESSING)) && //AC //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_WALL_COLOR_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcCONSTANT_POWER_NUMBER &&
                         uiDataCode != edcWALL_COLOR &&
                         uiDataCode != edcSERVICE_TEST_PATTERN &&
                         uiDataCode != edcSAVE_TO_USER &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         !(uiDataCode >= edcHSG_RED_HUE && uiDataCode <= edcHSG_RESET_DEFAULT) &&
                         !(uiDataCode >= edcHSG_RED_RESET_DEFAULT && uiDataCode <= edcHSG_WHITE_GAIN_RESET_DEFAULT)))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucACU_Status == eACU_STATUS_PROCESSING) &&  //ACU //A35G2_Wesley_0137
                        (uiDataCode != edcACU_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_WHITE &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID_CENTER &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_BLACK &&
						 uiDataCode != edcGEO_COLOR_UNIFORMITY_INTERFACE)) //A35G2_Wesley_0157
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if(IS_COMMON_DATA_CODE(uiDataCode))  //G100_Simon_0001
                {
                    if(sUtilCLICmd_Callback.fpDataMapping_CMValueRangeCheckCb(uiDataCode, iValue) == TRUE)   //G100_Simon_0002
                    {
#ifdef CUSTOM_CHRISTIE
						if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
						{
							if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||
							   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
							   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
							return eEXEC_CODE_FAIL;
						}
#endif
						if(uiDataCode == edcPICTURE_SETTINGS || uiDataCode == edcUSER_COLOR_MODE || uiDataCode == edcGAMMA || uiDataCode == edcCOLOR_SPACE || uiDataCode == edcSUB_IMAGE_COLOR_SPACE)//HICC2_Julie_0045
						{
							if(CommonAPI_CM_FuncIsAvailable(uiDataCode, iValue))
							{

							}
							else
							{
								return eEXEC_CODE_FAIL;
							}
						}

                        if(CM2GUI(uiDataCode, iValue) >= 0)  //check value
                        {
                            eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                        }
                    }
                    else
                    {
                        LOG_MSG(db_APP_CLI_LAN,"&Data %d (%d over range)\n", uiDataCode, iValue);
                        eResult = eEXEC_CODE_FAIL;
                    }
                }
                else
                {
#ifdef CUSTOM_CHRISTIE
					if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
					{
						if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||
						   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
						   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
						return eEXEC_CODE_FAIL;
					}
#endif
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                    //if((eResult == eEXEC_CODE_PASS) && (uiDataCode == edcHSG_AUTO_TEST_PATTERN)) //HICC2_Doulas_0051 remove
                    //{
                    //    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((UINT16)edcHSG_TEST_PATTERN_CTRL, edaREAD, &iValue);
                    //    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((UINT16)edcHSG_TEST_PATTERN_CTRL, edaWRITE_RAM_ONLY_WITH_ACTION, &iValue);
                    //}
                }

                if(eResult != eEXEC_CODE_PASS) //T100_Larry_0050
                {
                    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaREAD, &iValue);  //A35G2_CDS_Simon_0006
                    if(uiDataCode == edcSYSTEM_POWER) //HICC2_Doulas_0123 power down error and re-send status
                    {
                        sUtilCLICmd_Callback.fpLANProcSendToLANCb((UINT16)uiDataCode);
                        sUtilCLICmd_Callback.fpLANProcSendToLANCb((UINT16)edcPOWER_STATUS);
                    }
                }

                sDataOut->uData.lDataValue = iValue;

                LOG_MSG(db_APP_CLI_LAN,"&Data %d set Dec %d[%d]\n", uiDataCode, iValue,eResult); //HICC2_Doulas_0123
            }

            sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_LAN, (eDATA_CODE)uiDataCode); //G100_Julie_0003
        }
        else if(ucType == DATA_TYPE_STRING)
        {
            uiLen = strlen(sDataIn->uData.acBuffer);
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)
            {
                LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (grayout)\n", uiDataCode);
            }
            else
            {
                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                LOG_MSG(db_APP_CLI_LAN,"&Data %d set Str (%d)%s\n", uiDataCode, uiLen, sDataIn->uData.acBuffer);
            }

            memcpy(sDataOut->uData.acBuffer, sDataIn->uData.acBuffer, uiLen);
			sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_LAN, (eDATA_CODE)uiDataCode);

        }
        else if(ucType == DATA_TYPE_STRUCT)
        {
            //LOG_MSG(db_APP_CLI_LAN,"##(%d,%d)\n", sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode)
            //                                      ,sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode)); //HICC2_Doulas_0035
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&    //HICC2_Doulas_0035 Modify
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE) //HICC2_Doulas_0036
            {
                LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (grayout)\n", uiDataCode);
            }
            else if(uiDataCode == edcACTION_KEY_CODE)   //HICC2_Simon_0004
            {
                sKEY_DATA sKey = *(sKEY_DATA *)sDataIn->uData.acBuffer;
                LOG_MSG(db_APP_CLI_LAN, "Key: edcACTION_KEY_CODE with %d\r\n", sKey.wKeyCode);
                if(palInput_Web_LensShift_Check(sKey.wKeyCode) == FALSE)//HICC2_Julie_0048
                {
                    return eEXEC_CODE_FAIL;
                }
                else if(((palInput_Lens_Type_Get() == eLENS_ID_B20U100) || (palInput_Lens_Type_Get() == eLENS_ID_NVRAM_B20U100)) && (palInput_Fuji_Lens_Detect_Get() == 1)
                        && ((sKey.wKeyCode == keFOCUSUP) || (sKey.wKeyCode == keFOCUSDOWN) || (sKey.wKeyCode == keZOOMIN) || (sKey.wKeyCode == keZOOMOUT)
                        || (sKey.wKeyCode == keBACK_FOCUSUP) || (sKey.wKeyCode == keBACK_FOCUSDOWN)
                        || (sKey.wKeyCode == keLENSVUP) || (sKey.wKeyCode == keLENSVDOWN) || (sKey.wKeyCode == keLENSHLEFT) || (sKey.wKeyCode == keLENSHRIGHT)
                        || (sKey.wKeyCode == keHOLD_FOCUSUP) || (sKey.wKeyCode == keHOLD_FOCUSDOWN) || (sKey.wKeyCode == keHOLD_ZOOMIN) || (sKey.wKeyCode == keHOLD_ZOOMOUT)
                        || (sKey.wKeyCode == keHOLD_BACK_FOCUSUP) || (sKey.wKeyCode == keHOLD_BACK_FOCUSDOWN)
                        || (sKey.wKeyCode == keHOLD_LENSVUP) || (sKey.wKeyCode == keHOLD_LENSVDOWN) || (sKey.wKeyCode == keHOLD_LENSHLEFT) || (sKey.wKeyCode == keHOLD_LENSHRIGHT)))
                {
                    return eEXEC_CODE_FAIL;
                }
                else
                {
                    palInputProc_InputKeyListID(sKey.wKeyCode);
                    eResult = eEXEC_CODE_FAIL;  //no write datacode
                }
            }
            else
            {
                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                LOG_MSG(db_APP_CLI_LAN,"&Data %d set structure\n", uiDataCode);
            }

			sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_LAN, (eDATA_CODE)uiDataCode);
        }
        else
        {
            LOG_MSG(db_APP_CLI_LAN,"&Data set not found\n");
        }
    }

    if(edcLAN_DHCP == uiDataCode ||
       edcLAN_IP_ADDRESS == uiDataCode ||
       edcLAN_SUBNET_MASK == uiDataCode ||
       edcLAN_DEFAULT_GATEWAY == uiDataCode ||
       edcLAN_PRIMARY_DNS == uiDataCode ||
       edcLAN_SECOND_DNS == uiDataCode ||
       edcLAN_PATH_SWITCH == uiDataCode) //T100_Larry_0050
    {
        sUtilCLICmd_Callback.fpDataMgr_LanArgCopy2Cb();
        sUtilCLICmd_Callback.fpDataMgr_LanArg_Send_McuCb(uiDataCode); //A35Gen2_Coda
    }

    if(edcIPV6_DHCP == uiDataCode || //HICC2_AC_0055 // HICC2_Bruce_0020
       edcIPV6_IP_ADDRESS == uiDataCode ||
       edcIPV6_PREFIX_LENGTH == uiDataCode ||
       edcIPV6_DEFAULT_GATEWAY == uiDataCode ||
       edcIPV6_DNS == uiDataCode)
    {
        sUtilCLICmd_Callback.fpDataMgr_IPV6LanArgCopy2Cb();
        sUtilCLICmd_Callback.fpDataMgr_IPV6LanArg_Send_McuCb(uiDataCode);
    }

    if(edcWLAN_ENABLE == uiDataCode ||
       edcWLAN_START_IP == uiDataCode ||
       edcWLAN_END_IP == uiDataCode ||
       edcWLAN_SUBNET_MASK == uiDataCode ||
       edcWLAN_DEFAULT_GATEWAY == uiDataCode) //T100_Larry_0050
    {
        sUtilCLICmd_Callback.fpDataMgr_WLanArgCopy2Cb();
    }

    //G100_Wilsonj_0055 Start
    if(edcCRESTRON_ADDRESS == uiDataCode ||
       edcCRESTRON_PID == uiDataCode ||  //G100_Wilsonj_0059
       edcCRESTRON_PORT == uiDataCode)   //G100_Wilsonj_0059
    {
        sUtilCLICmd_Callback.fpDataMgr_CrestronArgCopy2Cb();
    }
    //G100_Wilsonj_0055 End

    //G100_Wilsonj_0059 Start
    if(edcPJLINK_ADDRESS == uiDataCode ||
	   edcPJLINK_SECRET_ENABLE  == uiDataCode || //A65_OPTOMA_Julie_0034
	   edcPJLINK_SECRET_PASSWORD  == uiDataCode) //A65_OPTOMA_Julie_0034 //A35G2_Coda_0048
    {
        sUtilCLICmd_Callback.fpDataMgr_PJLinkArgCopy2Cb();
    }
    //G100_Wilsonj_0059 End

    if(edcLAN_IP_ADDRESS == uiDataCode ||
       edcWLAN_DEFAULT_GATEWAY == uiDataCode ||
       edcLAN_SECOND_DNS == uiDataCode ||
       edcCRESTRON_ADDRESS == uiDataCode ||
       edcCRESTRON_PID == uiDataCode ||
       edcCRESTRON_PORT == uiDataCode ||
       edcPJLINK_ADDRESS == uiDataCode ||
       edcPJLINK_SECRET_ENABLE  == uiDataCode ||
       edcPJLINK_SECRET_PASSWORD  == uiDataCode)  //G100_Wilsonj_0059 //A35G2_Coda_0048
    {
        sUtilCLICmd_Callback.fpGui_SendDataCodeAndUpdateEventCb(uiDataCode);
    }

    if(edcLENS_CALIBRATION == uiDataCode)
    {
        sUtilCLICmd_Callback.fpGui_Send_LensCalibrationMenuOpenCb();
    }

    if(edcLENS_APPLY_POSITION == uiDataCode)
    {
        sUtilCLICmd_Callback.fpGui_Send_LensMovingMenuOpenCb();
    }
    else if((edcLENS_TOP_BOTTOM_CENTER == uiDataCode) //A35G2_Wesley_0162
    && (eFUNC_CONTROL_ENABLE == palDataMgr_DataCode_Control(edcLENS_TOP_BOTTOM_CENTER)))// A35G2_Bruce_0008 //A70Gen2_Julie_0109//HICC2_Julie_0014
    {
        sUtilCLICmd_Callback.fpGui_Send_LensMovingMenuOpenCb();
    }

    if(sUtilCLICmd_Callback.fpGui_Get_MenuState_IsOSD_OpenCb() &&
        (
           (edcOSDTEST_PATTERN == uiDataCode) ||
           (edcFACTORY_TEST_PATTERN == uiDataCode) ||
           (edcTWIST_PATTERN == uiDataCode)
       )
       )
    {
        sUtilCLICmd_Callback.fpGui_Send_OSD_ExitCb();
    }

    if(edcWEB_PIN_PROTECT_EXECUTE == uiDataCode) //A65_OPTOMA_Julie_0085 //A35G2_Coda_0123
    {
        UINT8 cData;
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcWEB_PIN_PROTECT_STATE, edaREAD, &cData);
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcWEB_PIN_PROTECT_STATE, edaWRITE_THROUGH_NO_ACTION, &cData);
    }

    if(eResult == eEXEC_CODE_PASS)
    {
        switch(uiDataCode)
        {
            case edcLAN_DHCP:
            //case edcLAN_IP_ADDRESS:
            case edcLAN_SUBNET_MASK:
            case edcLAN_DEFAULT_GATEWAY:
            case edcLAN_PRIMARY_DNS:
            //case edcLAN_SECOND_DNS:
            case edcWLAN_ENABLE:
            case edcWLAN_START_IP:
            case edcWLAN_END_IP:
            case edcWLAN_SUBNET_MASK:
            case edcWLAN_SSID:
            case edcLAN_MAC_ADDRESS:
            case edcLAN_VERSION:
            //case edcWLAN_DEFAULT_GATEWAY:
                break;

            default:
                //sUtilCLICmd_Callback.fpGui_SendUpdateOSDEventCb();
                //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, eOSD_UPDATE_CLI, NULL); //GuiCb.fpGui_SendUpdateOSD_CLI_EventCb(); //A70LV_Larry_0405

                if(palDataMgr_IsUIEventDataCode(uiDataCode) == FALSE)
                {
                    //LOG_MSG(db_ALWAYS,"(%s, %d)uiDataCode = %d\n", __FUNCTION__, __LINE__, uiDataCode);

                    if(palDataMgr_IsNetworkDataCode(uiDataCode) == FALSE)
                    {
                        sUtilCLICmd_Callback.fpLANProcUpdateOSD_SetCb(TRUE);
                    }

                    if(uiDataCode == edcWARPING_AP_APPLY ||
                       uiDataCode == edcBLEND_AP_APPLY
                       )
                    {
                        MS_SLEEP(200);    //H2PF_Simon_0038
                    }
                }
                break;
        }

        *pwByteCount = OFFSET_SUB_DATA_VALUE + uiLen;
    }
    else
    {
        sUtilCLICmd_Callback.fpLANProcSendToLANCb(uiDataCode);  //A35G2_CDS_Simon_0006
        *pwByteCount = 0;
    }

    if((uiDataCode != edcSHOW_MAINSOURCE_MESSAGES_INFO) &&
       (uiDataCode != edcREAL_DATE_TIME) &&
       (uiDataCode != edcUI_STATUS_IS_OSD_OPEN))    //HICC2_Doulas_0070
    {
        sUtilCLICmd_Callback.fpEnvironment_AutoShutDownClearCb();	//A35G2_BRC_Coda_0015
    }
    return rcSUCCESS;
}


// ==============================================================================
// FUNCTION NAME: utilCLI_LAN_AckDone_Set
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
// 2018/06/25, Larry Create
// --------------------
// ==============================================================================
static eRESULT utilCLI_LAN_AckDone_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    eEXEC_CODE eResult = eEXEC_CODE_FAIL;
    sLAN_DATA_FORMAT *sDataIn   = (sLAN_DATA_FORMAT*)psInData->aucData;
    sLAN_DATA_FORMAT *sDataOut  = (sLAN_DATA_FORMAT*)psOutData;
    UINT16 uiDataCode = sDataIn->uiDataCode;
    UINT16 uiLen = 0;
    UINT8  ucType = 0;
    UINT8  ucFreeze = 0;
	//A35G2_Wesley_0137
    UINT8  ucAuto_Focus_Status = 0;
    UINT8  ucAuto_Wall_Color_Status = 0;
    UINT8  ucACU_Status = 0;

    ucType = sUtilCLICmd_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)sDataIn->uiDataCode);

    sDataOut->uiDataCode        = uiDataCode;
    sDataOut->ucType            = ucType;
    sDataOut->ucEnable          = (UINT8)sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)sDataOut->uiDataCode);
    sDataOut->uData.lDataValue  = 0;

    //if((DATA_TYPE_I_DIGIT_64 >= ucType) && (DATA_TYPE_UI_DIGIT_8 <= ucType))
    //{
    //    LOG_MSG(db_ALWAYS, "[Datacode %d Value %d]\n", sDataIn->uiDataCode, sDataIn->uData.lDataValue);
    //}

#if 1 //HICC2_Doulas_0134//need to check PIN_Protect
    if((sUtilCLICmd_Callback.fpGui_PIN_Protect_CheckingCb() == ePASSWORD_PROTECT_LOCKED)
	&& (sUtilCLICmd_Callback.fpEnvironment_NetworkIsReceivePowerOnCmd_GetCb() == TRUE)
	&& (uiDataCode != edcPICTURE_MUTE) && (uiDataCode != edcDIRECT_PICTUREMUTE)	/*&& uiDataCode != edcLENS_CALIBRATION*/
	&& (uiDataCode != edcPIN_PROTECT) && (uiDataCode != edcWLAN_LINK_STATUS)
    && (uiDataCode != edcSYSTEM_POWER) && (uiDataCode != edcPOWER_STATUS) && (uiDataCode != edcACTION_KEY_CODE) //G100_Steven_0110 //G100_Coda_00121 //A35G2_BRC_Casper_0048 //A35G2_CDS_Coda_0033
	&& (uiDataCode != edcCAMERA_FW_VERSION) && (uiDataCode != edcCAMERA_MODULE_STATUS) && (uiDataCode != edcACTION_TELNET_CLI))//HICC2_Steven_0060
    {
    	LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (PIN_Protect)\n", uiDataCode);
    	sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);
        return eEXEC_CODE_FAIL;
    }
#endif
    if(palMotorLensCalibrationStatusGet() == 1)//H30K_David_0054
    {
        LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (LensCalibration=%d)\n\r", uiDataCode,palMotorLensCalibrationStatusGet());
		sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);
    	return eEXEC_CODE_FAIL;
    }
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcIMAGE_FREEZE, edaREAD, &ucFreeze);
	//A35G2_Wesley_0137
    palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaREAD, &ucAuto_Focus_Status);
    palDataMgr_Data_Access(edcAUTO_WALL_COLOR_STATUS, edaREAD, &ucAuto_Wall_Color_Status);
    palDataMgr_Data_Access(edcACU_STATUS, edaREAD, &ucACU_Status);

    LOG_MSG(db_APP_CLI_LAN, "AD DataCode[%d] %s\n", sDataIn->uiDataCode, sUtilCLICmd_Callback.fpDataMgr_DataCodeStringGetCb(sDataIn->uiDataCode));
    LOG_MSG(db_APP_CLI_LAN, "AD From %d\n", sDataIn->uiSendFrom);
    LOG_MSG(db_APP_CLI_LAN, "AD Enable %d\n", sDataIn->ucEnable);

    if(ucType != DATA_TYPE_NA)
    {
        if((DATA_TYPE_I_DIGIT_64 >= ucType) && (DATA_TYPE_UI_DIGIT_8 <= ucType))
        {
            INT32   iMax = 0;
            INT32   iMin = 0;
            INT32   iValue = sDataIn->uData.lDataValue;

            LOG_MSG(db_APP_CLI_LAN, "AD Data %d\n", sDataIn->uData.lDataValue);

            if((eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMAX, &iMax)) &&    //G100_Simon_0030
                (eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMIN, &iMin)))
            {
                if(iValue > iMax)
                {
                    iValue = iMax;
                    LOG_MSG(db_APP_CLI_LAN,"--&Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }
                else if(iValue < iMin)
                {
                    iValue = iMin;
                    LOG_MSG(db_APP_CLI_LAN,"&--Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }

                uiLen = 4;

                if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
                   sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)  //G100_Simon_0030   //G100_Simon_0036
                {
                    LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (grayout)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucFreeze) &&
                        ((uiDataCode != edcIMAGE_FREEZE &&
                          uiDataCode != edcSUPPORT_MESSAGE &&
                          uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                        #ifdef CUSTOM_CHRISTIE// x35G2_Bruce_0004
                          uiDataCode != edcImageReset &&
                        #endif
                          uiDataCode != edcEMERGENCY_MESSAGE_CLEAN &&
                          uiDataCode != edcPICTURE_MUTE && //G100_Larry_0035
                          uiDataCode != edcDIRECT_PICTUREMUTE &&
                          uiDataCode != edcBACKUP_RESTORE_WEB_SAVE) && //G100_Larry_0039
                          uiDataCode != edcLAN_PATH_SWITCH &&   //A35G2_CDS_Simon_0010
                          uiDataCode != edcOPDFILE_COPYTOWEB && //HICC2_Steven_0059 fixed ISS-0026337
						  !(uiDataCode >= edcIPV6_DHCP && uiDataCode <= edcIPV6_DNS) && //HICC2_AC_0055 // HICC2_Bruce_0020
                          !(uiDataCode >= edcLAN_DHCP && uiDataCode <= edcWLAN_SSID) &&
                          !(uiDataCode >= edcCrestron && uiDataCode <= edcHTTP)))//lan set function pass
                {
                    LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (freeze)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucAuto_Focus_Status == eAF_STATUS_PROCESSING) &&  //AF //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_FOCUS_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcOSD_SHOW &&
                         uiDataCode != edcGEO_OSD_ON &&  //H2PF_Simon_0035
                         uiDataCode != edcGEO_DRAW_FS_CHECKBOARD_ON_OSD &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_WARP_OSD_QUEUE_DRAW &&  //H2PF_Simon_0039
                         uiDataCode != edcNON &&
                         uiDataCode != edcFOCUS_MOVE &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucAuto_Wall_Color_Status == eAWC_STATUS_PROCESSING) &&  //AC //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_WALL_COLOR_STATUS &&
                         uiDataCode !=  edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcCONSTANT_POWER_NUMBER &&
                         uiDataCode != edcWALL_COLOR &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcSERVICE_TEST_PATTERN &&
                         uiDataCode != edcSAVE_TO_USER &&
                         !(uiDataCode >= edcHSG_RED_HUE && uiDataCode <= edcHSG_RESET_DEFAULT) &&
                         !(uiDataCode >= edcHSG_RED_RESET_DEFAULT && uiDataCode <= edcHSG_WHITE_GAIN_RESET_DEFAULT)))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucACU_Status == eACU_STATUS_PROCESSING) &&  //ACU //A35G2_Wesley_0137
                        (uiDataCode != edcACU_STATUS &&
                         uiDataCode !=  edcCAMERA_MODULE_STATUS &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_WHITE &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID_CENTER &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_BLACK &&
						 uiDataCode != edcGEO_COLOR_UNIFORMITY_INTERFACE)) //A35G2_Wesley_0157
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if(IS_COMMON_DATA_CODE(uiDataCode))  //G100_Simon_0002
                {
                    if(sUtilCLICmd_Callback.fpDataMapping_CMValueRangeCheckCb(uiDataCode, iValue) == TRUE)
                    {
#ifdef CUSTOM_CHRISTIE
						if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
						{
							if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||
							   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
							   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
							{
								palLANProcSendAckDone(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eEXEC_CODE_PASS);  //H2PF_Simon_0067
								return eEXEC_CODE_FAIL;
							}
						}
#endif
                        if(uiDataCode == edcPICTURE_SETTINGS || uiDataCode == edcUSER_COLOR_MODE || uiDataCode == edcGAMMA || uiDataCode == edcCOLOR_SPACE || uiDataCode == edcSUB_IMAGE_COLOR_SPACE)//HICC2_Julie_0045
                        {
                            if(CommonAPI_CM_FuncIsAvailable(uiDataCode, iValue))
                            {

                            }
                            else
                            {
                                palLANProcSendAckDone(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eEXEC_CODE_PASS);
                                return eEXEC_CODE_FAIL;
                            }
                        }

                        if((INT32)CM2GUI(uiDataCode, iValue) >= 0)
                        {
                            eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                        }
                    }
					else
					{
                        LOG_MSG(db_APP_CLI_LAN,"--&Data %d (%d over range)\n", uiDataCode, iValue);
					    eResult = eEXEC_CODE_FAIL;
					}
                }
				else
                {
#ifdef CUSTOM_CHRISTIE
					if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
					{
						if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||
						   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
						   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
						{
							palLANProcSendAckDone(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eEXEC_CODE_PASS);  //H2PF_Simon_0067
							return eEXEC_CODE_FAIL;
						}
					}
#endif
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                    //if((eResult == eEXEC_CODE_PASS) && (uiDataCode == edcHSG_AUTO_TEST_PATTERN)) //HICC2_Doulas_0051 remove
                    //{
                    //    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((UINT16)edcHSG_TEST_PATTERN_CTRL, edaREAD, &iValue);
                    //    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((UINT16)edcHSG_TEST_PATTERN_CTRL, edaWRITE_RAM_ONLY_WITH_ACTION, &iValue);
                    //}
                }

                if(eResult != eEXEC_CODE_PASS) //T100_Larry_0050
                {
                    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaREAD, &iValue); //A35G2_CDS_Simon_0006
                }

                sDataOut->uData.lDataValue = iValue;

                LOG_MSG(db_APP_CLI_LAN,"--&Data %d set Dec %d\n", uiDataCode, iValue);
            }

            sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_LAN, (eDATA_CODE)uiDataCode); //G100_Julie_0003
        }
        else if(ucType == DATA_TYPE_STRING)
        {
            uiLen = strlen(sDataIn->uData.acBuffer);
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)
            {
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (grayout)\n", uiDataCode);
            }
            else
            {
                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d set Str (%d)%s\n", uiDataCode, uiLen, sDataIn->uData.acBuffer);
            }

            memcpy(sDataOut->uData.acBuffer, sDataIn->uData.acBuffer, uiLen);
			sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_LAN, (eDATA_CODE)uiDataCode);

        }
        else if(ucType == DATA_TYPE_STRUCT)
        {
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)
            {
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (grayout)\n", uiDataCode);
            }
            else if(uiDataCode == edcACTION_KEY_CODE)   //HICC2_Simon_0004
            {
                sKEY_DATA sKey = *(sKEY_DATA *)sDataIn->uData.acBuffer;
                LOG_MSG(db_APP_CLI_LAN, "--Key: edcACTION_KEY_CODE with %d\r\n", sKey.wKeyCode);
                if(palInput_Web_LensShift_Check(sKey.wKeyCode) == FALSE)//HICC2_Julie_0048
                {
                    return eEXEC_CODE_FAIL;
                }
                else
                {
                    palInputProc_InputKeyListID(sKey.wKeyCode);
                    eResult = eEXEC_CODE_FAIL;  //no write datacode
                }
            }
            else
            {
                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d set structure\n", uiDataCode);
            }

			sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_LAN, (eDATA_CODE)uiDataCode);
        }
        else
        {
            LOG_MSG(db_APP_CLI_LAN,"-&Data set not found\n");
        }
    }

    if(edcLAN_DHCP == uiDataCode || edcWLAN_ENABLE == uiDataCode || edcCrestron == uiDataCode || edcPJ_Link  == uiDataCode) //A70LV_Larry_0252  //G100_Wilsonj_0059
    {
        sUtilCLICmd_Callback.fpDataMgr_LAN_IP_Copy2_AllCb(); //G100_Larry_0035
    }

    if(edcIPV6_DHCP == uiDataCode) //HICC2_AC_0055 // HICC2_Bruce_0020
    {
        sUtilCLICmd_Callback.fpDataMgr_IPV6LanArgCopy2Cb();
    }

    if(edcLENS_CALIBRATION == uiDataCode) //T100_Larry_0050
    {
        #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110

        if(palDataMgr_AutoFocusExecute_Get() == eAF_AC_NOT_EXECUTE &&
           palDataMgr_AutoWallColorExecute_Get() == eAF_AC_NOT_EXECUTE &&
           palDataMgr_AutoColorMatchExecute_Get() == eAF_AC_NOT_EXECUTE &&
           palDataMgr_ACU_Execute_Get() == eACU_NOT_EXECUTED &&
           palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)
        {
        	sUtilCLICmd_Callback.fpGui_Send_LensCalibrationMenuOpenCb();
        }
        #else
        sUtilCLICmd_Callback.fpGui_Send_LensCalibrationMenuOpenCb();
        #endif
    }

    if(edcLENS_APPLY_POSITION == uiDataCode) //T100_Larry_0050
    {
        sUtilCLICmd_Callback.fpGui_Send_LensMovingMenuOpenCb();
    }
    else if((edcLENS_TOP_BOTTOM_CENTER == uiDataCode) //A35G2_Wesley_0162
    && (eFUNC_CONTROL_ENABLE == palDataMgr_DataCode_Control(edcLENS_TOP_BOTTOM_CENTER)))// A35G2_Bruce_0008 //A70Gen2_Julie_0109//HICC2_Julie_0015
    {
        sUtilCLICmd_Callback.fpGui_Send_LensMovingMenuOpenCb();
    }
    else if(edcLIGHT_SENSOR_CALIBRATION_VALUE == uiDataCode)//HICC2_Julie_0040//HICC2_Julie_0042
    {
        if(sDataIn->uData.lDataValue == 0)
        {
            sUtilCLICmd_Callback.fpGui_Send_ABP_Cal_Msg_OpenCb();
        }
    }

    if(sGlobalCfg.sFN_CFG.SupportPin)
    {
        if(edcPIN_PROTECT == uiDataCode) //A35G2_CDS_Coda_0033
        {
            if(sDataIn->uData.lDataValue == 0)
            {
                sUtilCLICmd_Callback.fpGui_SendPinProtectClearEventCb();//HICC2_Julie_0042
            }
        }
    }

    if((uiDataCode != edcSHOW_MAINSOURCE_MESSAGES_INFO) &&
       (uiDataCode != edcREAL_DATE_TIME) &&
       (uiDataCode != edcUI_STATUS_IS_OSD_OPEN))    //HICC2_Doulas_0070
    {
        sUtilCLICmd_Callback.fpEnvironment_AutoShutDownClearCb();	//A35G2_BRC_Coda_0015
    }

    if(eResult == eEXEC_CODE_PASS)
    {
        sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);   //H2PF_Simon_0067
        //sUtilCLICmd_Callback.fpGui_SendUpdateOSDEventCb(); //A70LV_Larry_0250
        //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, eOSD_UPDATE_CLI, NULL); //GuiCb.fpGui_SendUpdateOSD_CLI_EventCb();
        if(palDataMgr_IsUIEventDataCode(uiDataCode) == FALSE)
        {
            //LOG_MSG(db_ALWAYS,"(%s, %d)uiDataCode = %d\n", __FUNCTION__, __LINE__, uiDataCode);
            sUtilCLICmd_Callback.fpLANProcUpdateOSD_SetCb(TRUE);
        }

        *pwByteCount = 0;
        return rcBUSY;
    }
    else
    {
        sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);  //avoid web page always 30s refresh time    //G100_Simon_0051  //H2PF_Simon_0067
        sUtilCLICmd_Callback.fpLANProcSendToLANCb(uiDataCode);   //A35G2_CDS_Simon_0006

        *pwByteCount = 0;
        return rcERROR;
    }
}

static eRESULT utilCLI_LAN_TelnetCmd_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sLAN_DATA_FORMAT *sDataIn   = (sLAN_DATA_FORMAT*)psInData->aucData;
    //sLAN_DATA_FORMAT *sDataOut  = (sLAN_DATA_FORMAT*)psOutData;

    char ucReturnString[255] = {'\0'};

    LOG_MSG(db_APP_CLI_LAN, "\r\n[Telnet In]%s\r\n"  , sDataIn->uData.acBuffer);

#if 1
	{
        utilCommonCLI_SWGEC_Telnet_Handle(sDataIn->uData.acBuffer);

#ifdef CUSTOM_CHRISTIE

    sCLI_CHRISTIE_FORMAT sCmdFormat = {0};
    sCmdFormat.ucCmdFrom = eccTelnet ; //T100_Simon_0020
    utilChristie_CLI_Process(eccTelnet, (UINT8 *)sDataIn->uData.acBuffer, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);

#elif defined (CUSTOM_OPTOMA)//A65_OPTOMA_John_0002 fix Optoma telnet command parser  //A35G2_Coda_0048

    UINT8 ucPowerOnFlag = 0;
    if((sDataIn->uData.acBuffer[3] == '0' && sDataIn->uData.acBuffer[4] == '0') || (sDataIn->uData.acBuffer[3] == '1' && sDataIn->uData.acBuffer[4] == '2' && sDataIn->uData.acBuffer[5] == '4')
		|| ((sDataIn->uData.acBuffer[3] == '1' && sDataIn->uData.acBuffer[4] == '5' && sDataIn->uData.acBuffer[5] == '0' && sDataIn->uData.acBuffer[7] == '1') && (sDataIn->uData.acBuffer[8] < '0' || sDataIn->uData.acBuffer[8] > '9' )))
    {
    	ucPowerOnFlag = 1;
    }

    if(((sUtilCLICmd_Callback.fpSystem_PowerStateGetCb() == ePOWER_STATE_STANDBY) || (sUtilCLICmd_Callback.fpSystem_PowerStateGetCb() == ePOWER_STATE_UPGRADE) || (sUtilCLICmd_Callback.fpEnvironment_Fake_Power_Down_GetCb())) && (ucPowerOnFlag != 1))
    {
    	sprintf(ucReturnString, "F\r");
    }
    else
    {
    	sCLI_OPTOMA_FORMAT sOptomaCmdFormat = {0};
    	sOptomaCmdFormat.ucCmdFrom = eccTelnet ;  	//A65_OPTOMA_Doulas_0019

    	if(sDataIn->uData.acBuffer[0] == OPTOMA_CMD_HEADER_1)
    	{
    		utilOptoma_CLI_Process(eccTelnet, (UINT8 *)sDataIn->uData.acBuffer, ucReturnString, &sOptomaCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
    	}
    }

    #ifdef GENERAL_MSSC_CLI
        sCLI_GENERAL_FORMAT sGenCmdFormat = {0};
        sGenCmdFormat.ucCmdFrom = eccTelnet ;

        if(sDataIn->uData.acBuffer[0] == GENERAL_CMD_HEADER_1)
        {
        	utilGeneral_CLI_Process(eccTelnet, (UINT8 *)sDataIn->uData.acBuffer, ucReturnString, &sGenCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);    //G100_Wilsonj_0018
        }
    #endif

#else
      sCLI_GENERAL_FORMAT sCmdFormat = {0};
      sCmdFormat.ucCmdFrom = eccTelnet ; //T100_Simon_0020

      utilGeneral_CLI_Process(eccTelnet, (UINT8 *)sDataIn->uData.acBuffer, ucReturnString, &sCmdFormat, CLI_EXECTUE_READ_CMD | CLI_EXECTUE_WRITE_BUFFER);
#endif

      //palLANProcTelnetReply(ucReturnString, strlen(ucReturnString)+1); //A35G2_BRC_Casper_0092

      *pwByteCount = 0;

      //*pwByteCount = sizeof(sDataOut->uiDataCode) +
                     //sizeof(sDataOut->ucType) +
                     //sizeof(sDataOut->ucEnable) +
                     //strlen(sDataOut->uData.acBuffer);
	 }
#endif /* 0 */
    return rcSUCCESS;
}

//Time API	//G100_Coda_0016
static eRESULT utilCLI_LAN_TimeCmd_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sLAN_STRUCT_DATA_FORMAT sStructPayload;
    _sDateTime sLanDateTime;

    memset((UINT8 *)&sStructPayload, 0, sizeof(_sStructPayload));
    memset((UINT8*)&sLanDateTime, 0, sizeof(_sDateTime));
    LOG_MSG(db_APP_SCHEDULE, "utilCLI_LAN_TimeCmd_Set Ready\r\n");
    //ToDo
    memcpy((UINT8*)&sStructPayload, &psInData->aucData, sizeof(sLAN_STRUCT_DATA_FORMAT));

    memcpy((UINT8*)&sLanDateTime, (UINT8*)&sStructPayload.sStructData.Data, sizeof(_sDateTime));
    sUtilCLICmd_Callback.fpDataMgr_DateTime_Unpack_LanPacketCb(&sLanDateTime); //G100_Coda_0038

    LOG_MSG(db_APP_SCHEDULE, "Mode = %s\r\n", sLanDateTime.Mode);
    LOG_MSG(db_APP_SCHEDULE, "Year = %s\r\n", sLanDateTime.Year);
    LOG_MSG(db_APP_SCHEDULE, "Month = %s\r\n", sLanDateTime.Month);
    LOG_MSG(db_APP_SCHEDULE, "Day = %s\r\n", sLanDateTime.Day);
    LOG_MSG(db_APP_SCHEDULE, "Hour = %s\r\n", sLanDateTime.Hour);
    LOG_MSG(db_APP_SCHEDULE, "Minute = %s\r\n", sLanDateTime.Minute);
    LOG_MSG(db_APP_SCHEDULE, "Second = %s\r\n", sLanDateTime.Second);
    LOG_MSG(db_APP_SCHEDULE, "Timezone = %s\r\n", sLanDateTime.Timezone);
    LOG_MSG(db_APP_SCHEDULE, "NTP = %s\r\n", sLanDateTime.NTP);
    LOG_MSG(db_APP_SCHEDULE, "Interval = %s\r\n", sLanDateTime.Interval);
    LOG_MSG(db_APP_SCHEDULE, "DST = %s\r\n", sLanDateTime.DST);
    LOG_MSG(db_APP_SCHEDULE, "Dstzone = %s\r\n", sLanDateTime.Dstzone);

    *pwByteCount = 0;
    return rcSUCCESS;

}

//Get Schedule data from LAN	//G100_Coda_0025
static eRESULT utilCLI_LAN_ScheduleCmd_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sLAN_STRUCT_DATA_FORMAT sStructPayload;
    sLAN_SCHEDULE_StructPayload sLanScheduleData;

    memset((UINT8 *)&sStructPayload, 0, sizeof(_sStructPayload));
    memset((UINT8*)&sLanScheduleData, 0, sizeof(sLAN_SCHEDULE_StructPayload));
    LOG_MSG(db_APP_SCHEDULE, "utilCLI_LAN_ScheduleCmd_Set Ready\r\n");
    //ToDo
    memcpy((UINT8*)&sStructPayload, &psInData->aucData, sizeof(sLAN_STRUCT_DATA_FORMAT));

    memcpy((UINT8*)&sLanScheduleData, (UINT8*)&sStructPayload.sStructData, sizeof(sLAN_SCHEDULE_StructPayload)); //G100_Coda_0040

    sUtilCLICmd_Callback.fpDataMgr_Schedule_Unpack_LanPacketCb(&sLanScheduleData.Data);
    sUtilCLICmd_Callback.fpEnvironment_AutoShutDownClearCb();	//A35G2_BRC_Coda_0015
    *pwByteCount = 0;
    return rcSUCCESS;

}

static eRESULT utilCLI_LAN_EmailNotifyCmd_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount) //A65_OPTOMA_Julie_0024 //A35G2_Coda_0048
{
    sLAN_STRUCT_DATA_FORMAT sStructPayload;
    _sEmail sEmailNotify;

    memset((UINT8 *)&sStructPayload, 0, sizeof(_sStructPayload));
    memset((UINT8*)&sEmailNotify, 0, sizeof(_sEmail));
    LOG_MSG(db_APP_SCHEDULE, "utilCLI_LAN_EmailNotifyCmd_Set Ready\r\n");

    //ToDo
    memcpy((UINT8*)&sStructPayload, &psInData->aucData, sizeof(sLAN_STRUCT_DATA_FORMAT));
    memcpy((UINT8*)&sEmailNotify, (UINT8*)&sStructPayload.sStructData.Data, sizeof(_sEmail));

    LOG_MSG(db_APP_SCHEDULE, "SmtpEn = %d\r\n", sEmailNotify.SmtpEn);
    LOG_MSG(db_APP_SCHEDULE, "SmtpService = %s\r\n", sEmailNotify.SmtpService);
    LOG_MSG(db_APP_SCHEDULE, "SmtpPort = %d\r\n", sEmailNotify.SmtpPort);
    LOG_MSG(db_APP_SCHEDULE, "SmtpTls = %d\r\n", sEmailNotify.SmtpTls);
    LOG_MSG(db_APP_SCHEDULE, "SmtpStarttls = %d\r\n", sEmailNotify.SmtpStarttls);
    LOG_MSG(db_APP_SCHEDULE, "SmtpAuthMode = %s\r\n", sEmailNotify.SmtpAuthMode);
    LOG_MSG(db_APP_SCHEDULE, "SmtpUsername = %s\r\n", sEmailNotify.SmtpUsername);
    LOG_MSG(db_APP_SCHEDULE, "SmtpPassword = %s\r\n", sEmailNotify.SmtpPassword);
    LOG_MSG(db_APP_SCHEDULE, "SendMail = %s\r\n", sEmailNotify.SendMail);
    LOG_MSG(db_APP_SCHEDULE, "Recipient1Email = %s\r\n", sEmailNotify.Recipient1Email);
    LOG_MSG(db_APP_SCHEDULE, "Recipient2Email = %s\r\n", sEmailNotify.Recipient2Email);

	sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcSNMP_RECIPIENT_EMAIL_1, edaWRITE_RAM_ONLY_NO_ACTION, (void *)&sEmailNotify.Recipient1Email);
	sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcSNMP_RECIPIENT_EMAIL_2, edaWRITE_RAM_ONLY_NO_ACTION, (void *)&sEmailNotify.Recipient2Email);

    *pwByteCount = 0;
    return rcSUCCESS;

}

static eRESULT utilCLI_Geo_Event(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
#if 0
    //sPACTET_FIXED_DATA_FORMAT *sDATA_FORMAT = (sPACTET_FIXED_DATA_FORMAT *)psInData->aucData;
    //UINT16 uiItemID = sDATA_FORMAT->uiItemID ;
    UINT16 uiItemID = *(UINT16 *)(psInData->aucData + 2) ;
    UINT8 ucAckDone = TRUE;     //A35G2_CDS_Simon_0011

    LOG_MSG(db_HAL_WARPING, "\r\nutilCLI_Geo_Event Start (%d)\r\n", uiItemID);

    if((sUtilCLICmd_Callback.fpSystem_PowerStateGetCb() == ePOWER_STATE_ACTIVE) && (sUtilCLICmd_Callback.fpEnvironment_Fake_Power_Down_GetCb() == FALSE))
    {
        switch(uiItemID)
        {
            case eBLENDER_CLEAR_WARPING:            utilCLI_Geo_ClearWarpingTable(psInData, psOutData, pwByteCount);  break;
            case eBLENDER_CONVERT_TP2DTDT:          utilCLI_Geo_convertTPtoDTDT(psInData, psOutData, pwByteCount);  break;
            case eBLENDER_SETLUT:                   utilCLI_Geo_setLut(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_GET_HORZ_LUT_INDEX:       utilCLI_Geo_getHorizontalLutIndex(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_HORZ_LUT_INDEX:       utilCLI_Geo_setHorizontalLutIndex(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_GET_VERT_LUT_INDEX:       utilCLI_Geo_getVerticalLutIndex(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_VERT_LUT_INDEX:       utilCLI_Geo_setVerticalLutIndex(psInData, psOutData, pwByteCount);   break;

            case eBLENDER_DRAW_EDGE_BLENDING:       utilCLI_Geo_DrawEdgeBlending(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_CLEAR_EDGE_BLENDING:      utilCLI_Geo_clearEdgeBlending(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_BLENDING_TABLE:       utilCLI_Geo_SetBlendingTable(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_CLEAR_BLENDING_TABLE:     utilCLI_Geo_ClearBlendingTable(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_BLENDING_GAMMA:       utilCLI_Geo_setBlendingGamma(psInData, psOutData, pwByteCount);   break;

            case eBLENDER_CLEAR_BLACK_LEVEL:        utilCLI_Geo_clearBlackLevel(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_BLACK_LEVEL_PALETTE:  utilCLI_Geo_setBlackLevelPalette(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_ENABLE_BLACK_LEVEL:       utilCLI_Geo_enableBlackLevel(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_BLACK_LEVEL_TABLE:    utilCLI_Geo_setBlackLevelTable(psInData, psOutData, pwByteCount);   break;

            case eBLENDER_SAVE_WARP_FLASH:          utilCLI_Geo_saveWarpToFlash(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_LOAD_WARP_FLASH:          utilCLI_Geo_loadWarpFromFlash(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SAVE_BLEND_FLASH:         utilCLI_Geo_saveBlendToFlash(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_LOAD_BLEND_FLASH:         utilCLI_Geo_loadBlendFromFlash(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SAVE_BLACK_LEVEL_FLASH:   utilCLI_Geo_setBlackLevelToFlash(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_LOAD_BLACK_LEVEL_FLASH:   utilCLI_Geo_getBlackLevelFromFlash(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_COLOR_UNIFORMITY_INTERFACE: utilCLI_Geo_ColorUniformityInterface(psInData, psOutData, pwByteCount);   break;

            case eBLENDER_SET_OSD_PALETTE:          utilCLI_Geo_setOSDPalette(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_SET_OSD_PALETTES:         utilCLI_Geo_setOSDPalettes(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_CLEAR_OSD:                utilCLI_Geo_ClearOSD(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_RECT:                utilCLI_Geo_DrawRect(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_RECTS:               utilCLI_Geo_DrawRects(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_LINE:                utilCLI_Geo_DrawLine(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_LINES:               utilCLI_Geo_DrawLines(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_CIRCLE:              utilCLI_Geo_DrawCircle(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_PNG:                 utilCLI_Geo_DrawPNG(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_SPRITE:              utilCLI_Geo_DrawSprite(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_SPRITES:             utilCLI_Geo_DrawSprites(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_COPY_OSD_RECT:       utilCLI_Geo_CopyOSDRect(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_CHECKS_ON_OSD:       utilCLI_Geo_DrawChecksOnOSD(psInData, psOutData, pwByteCount);   break;
            case eBLENDER_DRAW_STRING:              utilCLI_Geo_DrawString(psInData, psOutData, pwByteCount);    break;
            case eBLENDER_DRAW_OSD:                 utilCLI_Geo_DrawOSD(psInData, psOutData, pwByteCount);    break;
            case eBLENDER_DRAW_CHECKBOARD_ON_OSD:   utilCLI_Geo_DrawCheckboardOnOSD(psInData, psOutData, pwByteCount);    break;
            case eBLENDER_DRAW_BOXES:               utilCLI_Geo_DrawBoxes(psInData, psOutData, pwByteCount);    break;   //A35G2_Simon_0066
            case eBLENDER_FREEZE_IMAGE:             utilCLI_Geo_FreezeImage(psInData, psOutData, pwByteCount);    break;

		#if (ENABLE_COLOR_UNIFORMITY == TRUE)				//G100_Tim_0012, add, start
			case eBLENDER_TESTPATTERN_OFF:					utilCLI_Geo_TestPatternOff(psInData, psOutData, pwByteCount);			break;
			case eBLENDER_TESTPATTERN_ACU_WHITE:			utilCLI_Geo_TestPatternAcuWhite(psInData, psOutData, pwByteCount);		break;
			case eBLENDER_TESTPATTERN_ACU_GRID: 			utilCLI_Geo_TestPatternAcuGrid(psInData, psOutData, pwByteCount);		break;
			case eBLENDER_TESTPATTERN_ACU_GRID_CENTER:		utilCLI_Geo_TestPatternAcuGridCenter(psInData, psOutData, pwByteCount); break;
			case eBLENDER_TESTPATTERN_ACU_BLACK:			utilCLI_Geo_TestPatternAcuBlack(psInData, psOutData, pwByteCount);		break;

            case eBLENDER_COLOR_UNIFORMITY_TARGET_SELECT:	utilCLI_Geo_CuGridSelect(psInData, psOutData, pwByteCount);             break;		//A65_OPTOMA_CL_0001  //A65_OPTOMA_CL_0002 //A35G2_CDS_CL_0001
            case eBLENDER_DRAW_CU_TARGET_SELECT:            utilCLI_Geo_TestPatternCuGridSelect(psInData, psOutData, pwByteCount);  break;		//A65_OPTOMA_CL_0002 //A35G2_CDS_CL_0001
        #endif //ENABLE_COLOR_UNIFORMITY					//G100_Tim_0012, add, end
            case eBLENDER_DRAW_SOLID_COLOR:                 utilCLI_Geo_DrawSolidColor(psInData, psOutData, pwByteCount);           break;		//A65_OPTOMA_CL_0002 //A35G2_CDS_CL_0001
            case eBLENDER_DRAW_SET_TRANSPARENT_COLOR:       utilCLI_Geo_SetTransparentColor(psInData, psOutData, pwByteCount);      break;		//A65_OPTOMA_CL_0002 //A35G2_CDS_CL_0001
            case eBLENDER_DRAW_CIRCLE_GRID:                 utilCLI_Geo_DrawCircleGrid(psInData, psOutData, pwByteCount);    break;		//A65_OPTOMA_CL_0007

            case eBLENDER_STARTUP_INIT:                     utilCLI_Geo_StartupInit(psInData, psOutData, pwByteCount);              break;		//A35G2_Simon_0085

            default:
                ucAckDone = FALSE;  //A35G2_CDS_Simon_0011
                LOG_MSG(db_HAL_WARPING, "unknown Geo uiItemID %d\n", uiItemID);
                break;

        }
    }
    else
    {
        ucAckDone = FALSE;  //A35G2_CDS_Simon_0011
        ASSERT_ALWAYS();
    }

    if(ucAckDone == TRUE)  //A35G2_CDS_Simon_0011
    {
        LOG_MSG(db_HAL_WARPING, "Geo Cmd Ack Done (%d)\r\n", uiItemID);
        utilDatabase_WriteInformationData(eDI_GEOMETRY_CMD_ACK_DONE, DATA_TYPE_UI_DIGIT_16, 0, &uiItemID);
    }

    LOG_MSG(db_HAL_WARPING, "utilCLI_Geo_Event End (%d)\r\n", uiItemID);
#endif
}

eRESULT utilCLI_Geo_DrawOSDFeature(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
#if 0  //H2 wait review
    UINT16 uiItemID = *(UINT16 *)(psInData->aucData + 2);

    switch(uiItemID)
    {
        case eBLENDER_SET_OSD_PALETTE:          utilCLI_Geo_setOSDPalette(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_SET_OSD_PALETTES:         utilCLI_Geo_setOSDPalettes(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_CLEAR_OSD:                utilCLI_Geo_ClearOSD(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_RECT:                utilCLI_Geo_DrawRect(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_RECTS:               utilCLI_Geo_DrawRects(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_LINE:                utilCLI_Geo_DrawLine(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_LINES:               utilCLI_Geo_DrawLines(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_CIRCLE:              utilCLI_Geo_DrawCircle(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_PNG:                 utilCLI_Geo_DrawPNG(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_SPRITE:              utilCLI_Geo_DrawSprite(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_COPY_OSD_RECT:       utilCLI_Geo_CopyOSDRect(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_CHECKS_ON_OSD:       utilCLI_Geo_DrawChecksOnOSD(psInData, psOutData, pwByteCount);   break;
        case eBLENDER_DRAW_STRING:              utilCLI_Geo_DrawString(psInData, psOutData, pwByteCount);    break;
        case eBLENDER_DRAW_OSD:                 utilCLI_Geo_DrawOSD(psInData, psOutData, pwByteCount);    break;
        case eBLENDER_FREEZE_IMAGE:             utilCLI_Geo_FreezeImage(psInData, psOutData, pwByteCount);    break;  //A70LK_Simon_0001
        case eBLENDER_DRAW_CHECKBOARD_ON_OSD:   utilCLI_Geo_DrawCheckboardOnOSD(psInData, psOutData, pwByteCount);    break;
        case eBLENDER_DRAW_BOXES:               utilCLI_Geo_DrawBoxes(psInData, psOutData, pwByteCount);    break;   //A35G2_Simon_0066

            default:
                LOG_MSG(db_UTL_CLI, "unknouw uiItemID %d\n", uiItemID);
                break;
    }

    *pwByteCount = 0;
#endif

    return rcSUCCESS;

}


static eRESULT utilLI_OPD_UART_SET(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount) //G100_Julie_0004
{
    m_ucStressBuff[0] = psInData->aucData[0];   //G100_Owen_0080

    *pwByteCount = 0;
    return rcSUCCESS;
}

static eRESULT utilLI_OPD_UART_GET(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount) //G100_Julie_0004
{
    UINT8 *pcBuffer = (UINT8 *)psOutData;

    pcBuffer[0] = m_ucStressBuff[0];    //G100_Owen_0080

    *pwByteCount = 1;
    return rcSUCCESS;
}

static eRESULT utilCLI_Frontend_InputSourceDetect(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)  //A35G2_Simon_0075
{
    UINT16 uiSourceDetect = (psInData->aucData[1] << 8) + psInData->aucData[0];

    sUtilCLICmd_Callback.fpDataPath_InputDetectSetCb(uiSourceDetect);

    //LOG_MSG(db_APP_DATAPATH, "uiSourceDetect = 0x%04X (Last 0x%04X)\n", uiSourceDetect, palDataPath_LastInputDetectGet());

    sUtilCLICmd_Callback.fpDataMgr_OPDInputPlugInStateCb(eOPD_INPUT_DETECT_LOG);

    sUtilCLICmd_Callback.fpDataPath_LastInputDetectSetCb(uiSourceDetect);

    *pwByteCount = 0;
    return rcSUCCESS ;
}

// ==============================================================================
// FUNCTION NAME: utilCLI_LAN_Data_Set
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
// 2018/01/09, Larry Create //A70LV_Larry_0068
// --------------------
// ==============================================================================
static eRESULT utilCLI_GUI_Data_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    eEXEC_CODE eResult = eEXEC_CODE_FAIL;
    sLAN_DATA_FORMAT *sDataIn   = (sLAN_DATA_FORMAT*)psInData->aucData;
    sLAN_DATA_FORMAT *sDataOut  = (sLAN_DATA_FORMAT*)psOutData;
    UINT16 uiDataCode = sDataIn->uiDataCode;
    UINT16 uiLen = 0;
    UINT8  ucType = 0;
    UINT8  ucFreeze = 0;
	//A35G2_Wesley_0137
    UINT8  ucAuto_Focus_Status = 0;
    UINT8  ucAuto_Wall_Color_Status = 0;
    UINT8  ucACU_Status = 0;

    ucType = sUtilCLICmd_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)sDataIn->uiDataCode);

    sDataOut->uiDataCode        = uiDataCode;
    sDataOut->ucType            = ucType;
    sDataOut->ucEnable          = (UINT8)sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)sDataOut->uiDataCode);
    sDataOut->uData.lDataValue  = 0;

    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcIMAGE_FREEZE, edaREAD, &ucFreeze);  //G100_Simon_0030
	//A35G2_Wesley_0137
    palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaREAD, &ucAuto_Focus_Status);
    palDataMgr_Data_Access(edcAUTO_WALL_COLOR_STATUS, edaREAD, &ucAuto_Wall_Color_Status);
    palDataMgr_Data_Access(edcACU_STATUS, edaREAD, &ucACU_Status);

#if 0 //HICC2_Doulas_0134//need to check PIN_Protect
  	//G100_Coda_0095
    if(sUtilCLICmd_Callback.fpGui_PIN_Protect_CheckingCb() == ePASSWORD_PROTECT_LOCKED && sUtilCLICmd_Callback.fpEnvironment_NetworkIsReceivePowerOnCmd_GetCb() == TRUE && uiDataCode != edcPICTURE_MUTE
    		/*&& uiDataCode != edcLENS_CALIBRATION*/ && uiDataCode != edcPIN_PROTECT && uiDataCode != edcWEB_PIN_PROTECT_EXECUTE && uiDataCode != edcWLAN_LINK_STATUS) //A65_OPTOMA_Julie_0085) //A35G2_Coda_0123
    {
        return eEXEC_CODE_FAIL;
    }
#endif
    LOG_MSG(db_APP_CLI_LAN, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    LOG_MSG(db_APP_CLI_LAN, "DataCode[%d] %s\n", sDataIn->uiDataCode, sUtilCLICmd_Callback.fpDataMgr_DataCodeStringGetCb(sDataIn->uiDataCode));
    LOG_MSG(db_APP_CLI_LAN, "Type %d\n", sDataIn->ucType);
    LOG_MSG(db_APP_CLI_LAN, "Enable %d\n", sDataIn->ucEnable);
    if(sDataIn->ucType == DATA_TYPE_STRING)
    {
        LOG_MSG(db_APP_CLI_LAN, "DataValue %s\n", sDataIn->uData.acBuffer);
    }
    else
    {
        LOG_MSG(db_APP_CLI_LAN, "DataValue %d\n", sDataIn->uData.lDataValue);
    }

    if(ucType != DATA_TYPE_NA)
    {
        if((DATA_TYPE_I_DIGIT_64 >= ucType) && (DATA_TYPE_UI_DIGIT_8 <= ucType))
        {
            INT32   iMax = 0;
            INT32   iMin = 0;
            INT32   iValue = sDataIn->uData.lDataValue;

            if((eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMAX, &iMax)) &&
                (eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMIN, &iMin)))
            {
                if(iValue > iMax)
                {
                    iValue = iMax;
                    LOG_MSG(db_APP_GUI, "&Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }
                else if(iValue < iMin)
                {
                    iValue = iMin;
                    LOG_MSG(db_APP_GUI, "&Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }

                uiLen = 4;

                if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
                   sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE )  //G100_Simon_0030  //G100_Simon_0036
                {
                    LOG_MSG(db_APP_GUI, "&Data %d no action (grayout)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucFreeze) &&
                        ((uiDataCode != edcIMAGE_FREEZE &&
                        #if (defined(CUSTOM_OPTOMA))// HICC2_Bruce_0003, for unfreeze
                          uiDataCode != edcCONFIRMATION &&
                        #endif
                          uiDataCode != edcSUPPORT_MESSAGE &&
                          uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                        #ifdef CUSTOM_CHRISTIE// x35G2_Bruce_0004
                          uiDataCode != edcImageReset &&
                        #endif
                          uiDataCode != edcEMERGENCY_MESSAGE_CLEAN &&
                          uiDataCode != edcPICTURE_MUTE && //G100_Larry_0035
                          uiDataCode != edcDIRECT_PICTUREMUTE &&
                          uiDataCode != edcBACKUP_RESTORE_WEB_SAVE) && //G100_Larry_0039
                          uiDataCode != edcLAN_PATH_SWITCH &&   //A35G2_CDS_Simon_0010
                          !(uiDataCode >= edcIPV6_DHCP && uiDataCode <= edcIPV6_DNS) && //HICC2_AC_0055 // HICC2_Bruce_0020
                          !(uiDataCode >= edcLAN_DHCP && uiDataCode <= edcWLAN_SSID) &&
                          !(uiDataCode >= edcCrestron && uiDataCode <= edcHTTP)))
                {
                    LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (freeze)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucAuto_Focus_Status == eAF_STATUS_PROCESSING) &&  //AF //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_FOCUS_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcOSD_SHOW &&
                         uiDataCode != edcGEO_OSD_ON &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_DRAW_FS_CHECKBOARD_ON_OSD &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_WARP_OSD_QUEUE_DRAW &&  //H2PF_Simon_0039
                         uiDataCode != edcNON &&
                         uiDataCode != edcFOCUS_MOVE &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucAuto_Wall_Color_Status == eAWC_STATUS_PROCESSING) &&  //AC //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_WALL_COLOR_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcCONSTANT_POWER_NUMBER &&
                         uiDataCode != edcWALL_COLOR &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcSERVICE_TEST_PATTERN &&
                         uiDataCode != edcSAVE_TO_USER &&
                         !(uiDataCode >= edcHSG_RED_HUE && uiDataCode <= edcHSG_RESET_DEFAULT) &&
                         !(uiDataCode >= edcHSG_RED_RESET_DEFAULT && uiDataCode <= edcHSG_WHITE_GAIN_RESET_DEFAULT)))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucACU_Status == eACU_STATUS_PROCESSING) &&  //ACU //A35G2_Wesley_0137
                        (uiDataCode != edcACU_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_WHITE &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID_CENTER &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_BLACK &&
						 uiDataCode != edcGEO_COLOR_UNIFORMITY_INTERFACE)) //A35G2_Wesley_0157
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if(IS_COMMON_DATA_CODE(uiDataCode))  //G100_Simon_0001
                {
                    if(sUtilCLICmd_Callback.fpDataMapping_CMValueRangeCheckCb(uiDataCode, iValue) == TRUE)   //G100_Simon_0002
                    {
#ifdef CUSTOM_CHRISTIE
						if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
						{
                            if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||   //H2PF_Simon_0071
							   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
							   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
							return eEXEC_CODE_FAIL;
						}
#endif
						if(uiDataCode == edcPICTURE_SETTINGS || uiDataCode == edcUSER_COLOR_MODE || uiDataCode == edcGAMMA || uiDataCode == edcCOLOR_SPACE || uiDataCode == edcSUB_IMAGE_COLOR_SPACE)//HICC2_Julie_0045
                        {
                            if(CommonAPI_CM_FuncIsAvailable(uiDataCode, iValue))
                            {

                            }
                            else
                            {
                                return eEXEC_CODE_FAIL;
                            }
                        }

                        if(CM2GUI(uiDataCode, iValue) >= 0)  //check value
                        {
                            if(psInData->ucSubCmd == eGUI_MSG_DATA)
                            {
                                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                            }
                            else if(psInData->ucSubCmd == eGUI_MSG_DATA_NO_ACTIVE)
                            {
                                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, &iValue);
                            }
                        }
                    }
                    else
                    {
                        LOG_MSG(db_APP_CLI_LAN,"&Data %d (%d over range)\n", uiDataCode, iValue);
                        eResult = eEXEC_CODE_FAIL;
                    }
                }
                else
                {
#ifdef CUSTOM_CHRISTIE
					if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
					{
                        if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||   //H2PF_Simon_0071
						   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
						   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
						return eEXEC_CODE_FAIL;
					}
#endif
                    if(psInData->ucSubCmd == eGUI_MSG_DATA)
                    {
                        eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                    }
                    else if(psInData->ucSubCmd == eGUI_MSG_DATA_NO_ACTIVE)
                    {
                        eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, &iValue);
                    }
                }

                if(eResult != eEXEC_CODE_PASS) //T100_Larry_0050
                {
                    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaREAD, &iValue);  //A35G2_CDS_Simon_0006
                }

                sDataOut->uData.lDataValue = iValue;

                LOG_MSG(db_APP_CLI_LAN,"&Data %d set Dec %d\n", uiDataCode, iValue);
            }

            //sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_GUI, (eDATA_CODE)uiDataCode); //G100_Julie_0003
        }
        else if(ucType == DATA_TYPE_STRING)
        {
            uiLen = strlen(sDataIn->uData.acBuffer);
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)
            {
                LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (grayout)\n", uiDataCode);
            }
            else
            {
                if(psInData->ucSubCmd == eGUI_MSG_DATA)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                }
                else if(psInData->ucSubCmd == eGUI_MSG_DATA_NO_ACTIVE)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, sDataIn->uData.acBuffer);
                }
                LOG_MSG(db_APP_CLI_LAN,"&Data %d set Str (%d)%s\n", uiDataCode, uiLen, sDataIn->uData.acBuffer);
            }

            memcpy(sDataOut->uData.acBuffer, sDataIn->uData.acBuffer, uiLen);
			//sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_GUI, (eDATA_CODE)uiDataCode);

        }
        else if(ucType == DATA_TYPE_STRUCT)
        {
            //LOG_MSG(db_APP_CLI_LAN,"##(%d,%d)\n", sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode)
            //                                      ,sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode)); //HICC2_Doulas_0035
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&    //HICC2_Doulas_0035 Modify
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE) //HICC2_Doulas_0036
            {
                LOG_MSG(db_APP_CLI_LAN,"&Data %d no action (grayout)\n", uiDataCode);
            }
            else if(uiDataCode == edcACTION_KEY_CODE)   //HICC2_Simon_0004
            {
                sKEY_DATA sKey = *(sKEY_DATA *)sDataIn->uData.acBuffer;
                LOG_MSG(db_APP_CLI_LAN, "--Key: edcACTION_KEY_CODE with %d\r\n", sKey.wKeyCode);
                if(palInput_Web_LensShift_Check(sKey.wKeyCode) == FALSE)//HICC2_Julie_0048
                {
                    return eEXEC_CODE_FAIL;
                }
                else
                {
                    palInputProc_InputKeyListID(sKey.wKeyCode);
                    eResult = eEXEC_CODE_FAIL;  //no write datacode
                }
            }
            else
            {
                if(psInData->ucSubCmd == eGUI_MSG_DATA)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                }
                else if(psInData->ucSubCmd == eGUI_MSG_DATA_NO_ACTIVE)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, sDataIn->uData.acBuffer);
                }
                LOG_MSG(db_APP_CLI_LAN,"&Data %d set structure\n", uiDataCode);
            }

			//sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_GUI, (eDATA_CODE)uiDataCode);
        }
        else
        {
            LOG_MSG(db_APP_CLI_LAN,"&Data set not found\n");
        }
    }

    if(edcLENS_CALIBRATION == uiDataCode)
    {
        sUtilCLICmd_Callback.fpGui_Send_LensCalibrationMenuOpenCb();
    }

    if(edcLENS_APPLY_POSITION == uiDataCode)
    {
        sUtilCLICmd_Callback.fpGui_Send_LensMovingMenuOpenCb();
    }

    if(sUtilCLICmd_Callback.fpGui_Get_MenuState_IsOSD_OpenCb() &&
        (
           (edcOSDTEST_PATTERN == uiDataCode) ||
           (edcFACTORY_TEST_PATTERN == uiDataCode) ||
           (edcTWIST_PATTERN == uiDataCode)
       )
       )
    {
        sUtilCLICmd_Callback.fpGui_Send_OSD_ExitCb();
    }

    if(edcWEB_PIN_PROTECT_EXECUTE == uiDataCode) //A65_OPTOMA_Julie_0085 //A35G2_Coda_0123
    {
        UINT8 cData;
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcWEB_PIN_PROTECT_STATE, edaREAD, &cData);
        sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcWEB_PIN_PROTECT_STATE, edaWRITE_THROUGH_NO_ACTION, &cData);
    }

    if(eResult == eEXEC_CODE_PASS)
    {
        *pwByteCount = OFFSET_SUB_DATA_VALUE + uiLen;
    }
    else
    {
        sUtilCLICmd_Callback.fpLANProcSendToLANCb(uiDataCode);  //A35G2_CDS_Simon_0006
        *pwByteCount = 0;
    }

#if 0 //HICC2_Julie_0026,ISS-0023888有設定Auto shutdown時在Pin protect畫面觸發功能後會馬上自動解除倒數.
    if((uiDataCode != edcSHOW_MAINSOURCE_MESSAGES_INFO) &&
       (uiDataCode != edcREAL_DATE_TIME) &&
       (uiDataCode != edcUI_STATUS_IS_OSD_OPEN))    //HICC2_Doulas_0070
    {
        sUtilCLICmd_Callback.fpEnvironment_AutoShutDownClearCb();	//A35G2_BRC_Coda_0015
    }
#endif

    return rcSUCCESS;
}


// ==============================================================================
// FUNCTION NAME: utilCLI_LAN_AckDone_Set
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
// 2018/06/25, Larry Create
// --------------------
// ==============================================================================
static eRESULT utilCLI_GUI_AckDone_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    eEXEC_CODE eResult = eEXEC_CODE_FAIL;
    sLAN_DATA_FORMAT *sDataIn   = (sLAN_DATA_FORMAT*)psInData->aucData;
    sLAN_DATA_FORMAT *sDataOut  = (sLAN_DATA_FORMAT*)psOutData;
    UINT16 uiDataCode = sDataIn->uiDataCode;
    UINT16 uiLen = 0;
    UINT8  ucType = 0;
    UINT8  ucFreeze = 0;
	//A35G2_Wesley_0137
    UINT8  ucAuto_Focus_Status = 0;
    UINT8  ucAuto_Wall_Color_Status = 0;
    UINT8  ucACU_Status = 0;

    ucType = sUtilCLICmd_Callback.fpDataMgr_Data_TypeCb((eDATA_CODE)sDataIn->uiDataCode);

    sDataOut->uiDataCode        = uiDataCode;
    sDataOut->ucType            = ucType;
    sDataOut->ucEnable          = (UINT8)sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)sDataOut->uiDataCode);
    sDataOut->uData.lDataValue  = 0;

    //if((DATA_TYPE_I_DIGIT_64 >= ucType) && (DATA_TYPE_UI_DIGIT_8 <= ucType))
    //{
    //    LOG_MSG(db_ALWAYS, "[Datacode %d Value %d]\n", sDataIn->uiDataCode, sDataIn->uData.lDataValue);
    //}

#if 0 //HICC2_Doulas_0134//need to check PIN_Protect
    if(sUtilCLICmd_Callback.fpGui_PIN_Protect_CheckingCb() == ePASSWORD_PROTECT_LOCKED && sUtilCLICmd_Callback.fpEnvironment_NetworkIsReceivePowerOnCmd_GetCb() == TRUE && uiDataCode != edcPICTURE_MUTE
    		/*&& uiDataCode != edcLENS_CALIBRATION*/ && uiDataCode != edcPIN_PROTECT && uiDataCode != edcWLAN_LINK_STATUS) //G100_Steven_0110 //G100_Coda_00121 //A35G2_BRC_Casper_0048 //A35G2_CDS_Coda_0033
    {
        sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);
        return eEXEC_CODE_FAIL;
    }
#endif
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcIMAGE_FREEZE, edaREAD, &ucFreeze);
	//A35G2_Wesley_0137
    palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaREAD, &ucAuto_Focus_Status);
    palDataMgr_Data_Access(edcAUTO_WALL_COLOR_STATUS, edaREAD, &ucAuto_Wall_Color_Status);
    palDataMgr_Data_Access(edcACU_STATUS, edaREAD, &ucACU_Status);

    LOG_MSG(db_APP_CLI_LAN, "GUI AD DataCode[%d] %s En %d Type %d\n", sDataIn->uiDataCode
                                                                    , sUtilCLICmd_Callback.fpDataMgr_DataCodeStringGetCb(sDataIn->uiDataCode)
                                                                    , sDataIn->ucEnable
                                                                    , sDataIn->ucType);

    if(ucType != DATA_TYPE_NA)
    {
        if((DATA_TYPE_I_DIGIT_64 >= ucType) && (DATA_TYPE_UI_DIGIT_8 <= ucType))
        {
            INT32   iMax = 0;
            INT32   iMin = 0;
            INT32   iValue = sDataIn->uData.lDataValue;

            LOG_MSG(db_APP_CLI_LAN, "AD Data %d\n", sDataIn->uData.lDataValue);

            if((eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMAX, &iMax)) &&    //G100_Simon_0030
                (eEXEC_CODE_PASS == sUtilCLICmd_Callback.fpDataMgr_Data_Range_GetCb((eDATA_CODE)uiDataCode, edrMIN, &iMin)))
            {
                if(iValue > iMax)
                {
                    iValue = iMax;
                    LOG_MSG(db_APP_CLI_LAN,"--&Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }
                else if(iValue < iMin)
                {
                    iValue = iMin;
                    LOG_MSG(db_APP_CLI_LAN,"&--Data %d set over range (%d), recover to (%d)\n", uiDataCode, sDataIn->uData.lDataValue , iValue);
                }

                uiLen = 4;

                if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
                   sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)  //G100_Simon_0030   //G100_Simon_0036
                {
                    LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (grayout)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucFreeze) &&
                        ((uiDataCode != edcIMAGE_FREEZE &&
                        #if (defined(CUSTOM_OPTOMA))// HICC2_Bruce_0003, for unfreeze
                          uiDataCode != edcCONFIRMATION &&
                        #endif
                          uiDataCode != edcSUPPORT_MESSAGE &&
                          uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                        #ifdef CUSTOM_CHRISTIE// x35G2_Bruce_0004
                          uiDataCode != edcImageReset &&
                        #endif
                          uiDataCode != edcEMERGENCY_MESSAGE_CLEAN &&
                          uiDataCode != edcPICTURE_MUTE && //G100_Larry_0035
                          uiDataCode != edcDIRECT_PICTUREMUTE &&
                          uiDataCode != edcBACKUP_RESTORE_WEB_SAVE) && //G100_Larry_0039
                          uiDataCode != edcLAN_PATH_SWITCH &&   //A35G2_CDS_Simon_0010
                          !(uiDataCode >= edcIPV6_DHCP && uiDataCode <= edcIPV6_DNS) && //HICC2_AC_0055 // HICC2_Bruce_0020
                          !(uiDataCode >= edcLAN_DHCP && uiDataCode <= edcWLAN_SSID) &&
                          !(uiDataCode >= edcCrestron && uiDataCode <= edcHTTP)))//lan set function pass
                {
                    LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (freeze)\n", uiDataCode);
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucAuto_Focus_Status == eAF_STATUS_PROCESSING) &&  //AF //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_FOCUS_STATUS &&
                         uiDataCode != edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcOSD_SHOW &&
                         uiDataCode != edcGEO_OSD_ON &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_DRAW_FS_CHECKBOARD_ON_OSD &&  //H2PF_Simon_0039
                         uiDataCode != edcGEO_WARP_OSD_QUEUE_DRAW &&  //H2PF_Simon_0039
                         uiDataCode != edcNON &&
                         uiDataCode != edcFOCUS_MOVE &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucAuto_Wall_Color_Status == eAWC_STATUS_PROCESSING) &&  //AC //A35G2_Wesley_0137
                        (uiDataCode != edcAUTO_WALL_COLOR_STATUS &&
                         uiDataCode !=  edcCAMERA_MODULE_STATUS && //A35G2_Wesley_0157
                         uiDataCode != edcCONSTANT_POWER_NUMBER &&
                         uiDataCode != edcWALL_COLOR &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcSERVICE_TEST_PATTERN &&
                         uiDataCode != edcSAVE_TO_USER &&
                         !(uiDataCode >= edcHSG_RED_HUE && uiDataCode <= edcHSG_RESET_DEFAULT) &&
                         !(uiDataCode >= edcHSG_RED_RESET_DEFAULT && uiDataCode <= edcHSG_WHITE_GAIN_RESET_DEFAULT)))
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if((ucACU_Status == eACU_STATUS_PROCESSING) &&  //ACU //A35G2_Wesley_0137
                        (uiDataCode != edcACU_STATUS &&
                         uiDataCode !=  edcCAMERA_MODULE_STATUS &&
						 uiDataCode != edcGEO_DRAW_SOLID_COLOR &&
                         uiDataCode != edcGEO_TESTPATTERN_OFF &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_WHITE &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_GRID_CENTER &&
                         uiDataCode != edcGEO_TESTPATTERN_ACU_BLACK &&
						 uiDataCode != edcGEO_COLOR_UNIFORMITY_INTERFACE)) //A35G2_Wesley_0157
                {
                    eResult = eEXEC_CODE_FAIL;
                }
                else if(IS_COMMON_DATA_CODE(uiDataCode))  //G100_Simon_0002
                {
                    if(sUtilCLICmd_Callback.fpDataMapping_CMValueRangeCheckCb(uiDataCode, iValue) == TRUE)
                    {
#ifdef CUSTOM_CHRISTIE
						if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
						{
                            if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||     //H2PF_Simon_0071
							   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
							   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
							{
								palLANProcSendAckDone(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eEXEC_CODE_PASS);  //H2PF_Simon_0067
								return eEXEC_CODE_FAIL;
							}
						}
#endif
						if(uiDataCode == edcPICTURE_SETTINGS || uiDataCode == edcUSER_COLOR_MODE || uiDataCode == edcGAMMA || uiDataCode == edcCOLOR_SPACE || uiDataCode == edcSUB_IMAGE_COLOR_SPACE)//HICC2_Julie_0045
                        {
                            if(CommonAPI_CM_FuncIsAvailable(uiDataCode, iValue))
                            {

                            }
                            else
                            {
                                palLANProcSendAckDone(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eEXEC_CODE_PASS);
                                return eEXEC_CODE_FAIL;
                            }
                        }

                        if((INT32)CM2GUI(uiDataCode, iValue) >= 0)
                        {
                            if(psInData->ucSubCmd == eGUI_MSG_ACK_DONE)
                            {
                                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                            }
                            else if(psInData->ucSubCmd == eGUI_MSG_NO_ACTIVE_ACK_DONE)
                            {
                                eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, &iValue);
                            }
                        }
                    }
					else
					{
                        LOG_MSG(db_APP_CLI_LAN,"--&Data %d (%d over range)\n", uiDataCode, iValue);
					    eResult = eEXEC_CODE_FAIL;
					}
                }
				else
                {
#ifdef CUSTOM_CHRISTIE
					if( palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING )	//A35G2_Alan_0011
					{
                        if(uiDataCode == edcPICTURE_MUTE || uiDataCode == edcDIRECT_PICTUREMUTE || uiDataCode == edcPIP_PBP_ENABLE ||     //H2PF_Simon_0071
						   uiDataCode == edcINPUT_KEY || uiDataCode == edcAUTO_ADJUSTMENT_MODE ||
						   uiDataCode == edcOSDTEST_PATTERN || uiDataCode == edcMAIN_INPUT)
						{
							palLANProcSendAckDone(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eEXEC_CODE_PASS);  //H2PF_Simon_0067
							return eEXEC_CODE_FAIL;
						}
					}
#endif
                    if(psInData->ucSubCmd == eGUI_MSG_ACK_DONE)
                    {
                        eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_THROUGH_WITH_ACTION, &iValue);
                    }
                    else if(psInData->ucSubCmd == eGUI_MSG_NO_ACTIVE_ACK_DONE)
                    {
                        eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, &iValue);
                    }
                    //if((eResult == eEXEC_CODE_PASS) && (uiDataCode == edcHSG_AUTO_TEST_PATTERN)) //HICC2_Doulas_0051 remove
                    //{
                    //    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((UINT16)edcHSG_TEST_PATTERN_CTRL, edaREAD, &iValue);
                    //    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb((UINT16)edcHSG_TEST_PATTERN_CTRL, edaWRITE_RAM_ONLY_WITH_ACTION, &iValue);
                    //}
                }

                if(eResult != eEXEC_CODE_PASS) //T100_Larry_0050
                {
                    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(uiDataCode, edaREAD, &iValue); //A35G2_CDS_Simon_0006
                }

                sDataOut->uData.lDataValue = iValue;

                LOG_MSG(db_APP_CLI_LAN,"--&Data %d set Dec %d done\n", uiDataCode, iValue);
            }

            //sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_GUI, (eDATA_CODE)uiDataCode); //G100_Julie_0003
        }
        else if(ucType == DATA_TYPE_STRING)
        {
            uiLen = strlen(sDataIn->uData.acBuffer);
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)
            {
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (grayout)\n", uiDataCode);
            }
            else
            {
                if(psInData->ucSubCmd == eGUI_MSG_ACK_DONE)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                }
                else if(psInData->ucSubCmd == eGUI_MSG_NO_ACTIVE_ACK_DONE)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, sDataIn->uData.acBuffer);
                }
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d set Str (%d)%s done\n", uiDataCode, uiLen, sDataIn->uData.acBuffer);
            }

            memcpy(sDataOut->uData.acBuffer, sDataIn->uData.acBuffer, uiLen);
			//sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_GUI, (eDATA_CODE)uiDataCode);

        }
        else if(ucType == DATA_TYPE_STRUCT)
        {
            if(sUtilCLICmd_Callback.fpDataMgr_DataCode_ControlCb((eDATA_CODE)uiDataCode) != eFUNC_CONTROL_ENABLE &&
               sUtilCLICmd_Callback.fpDataMgr_IsNetworkDataCodeCb((eDATA_CODE)uiDataCode) == FALSE)
            {
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d no action (grayout)\n", uiDataCode);
            }
            else if(uiDataCode == edcACTION_KEY_CODE)   //HICC2_Simon_0004
            {
                sKEY_DATA sKey = *(sKEY_DATA *)sDataIn->uData.acBuffer;
                LOG_MSG(db_APP_CLI_LAN, "--Key: edcACTION_KEY_CODE with %d\r\n", sKey.wKeyCode);
                if(palInput_Web_LensShift_Check(sKey.wKeyCode) == FALSE)//HICC2_Julie_0048
                {
                    return eEXEC_CODE_FAIL;
                }
                else
                {
                    palInputProc_InputKeyListID(sKey.wKeyCode);
                    eResult = eEXEC_CODE_FAIL;  //no write datacode
                }
            }
            else
            {
                if(psInData->ucSubCmd == eGUI_MSG_ACK_DONE)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_THROUGH_WITH_ACTION, sDataIn->uData.acBuffer);
                }
                else if(psInData->ucSubCmd == eGUI_MSG_NO_ACTIVE_ACK_DONE)
                {
                    eResult = sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(sDataOut->uiDataCode, edaWRITE_RAM_ONLY_NO_ACTION, sDataIn->uData.acBuffer);
                }
                LOG_MSG(db_APP_CLI_LAN,"--&Data %d set structure done\n", uiDataCode);
            }

			//sUtilCLICmd_Callback.fpDataMgr_OPDEventCb(eOPD_ACCESS_DATA_GUI, (eDATA_CODE)uiDataCode);
        }
        else
        {
            LOG_MSG(db_APP_CLI_LAN,"-&Data set not found\n");
        }
    }

    #if 0
    if(edcLAN_DHCP == uiDataCode || edcWLAN_ENABLE == uiDataCode || edcCrestron == uiDataCode || edcPJ_Link  == uiDataCode) //A70LV_Larry_0252  //G100_Wilsonj_0059
    {
        sUtilCLICmd_Callback.fpDataMgr_LAN_IP_Copy2_AllCb(); //G100_Larry_0035
    }
    #endif

    if(edcLENS_CALIBRATION == uiDataCode) //T100_Larry_0050
    {
        #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110

        if(palDataMgr_AutoFocusExecute_Get() == eAF_AC_NOT_EXECUTE &&
           palDataMgr_AutoWallColorExecute_Get() == eAF_AC_NOT_EXECUTE &&
           palDataMgr_AutoColorMatchExecute_Get() == eAF_AC_NOT_EXECUTE &&
           palDataMgr_ACU_Execute_Get() == eACU_NOT_EXECUTED &&
           palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)
        {
        	sUtilCLICmd_Callback.fpGui_Send_LensCalibrationMenuOpenCb();
        }
        #else
        sUtilCLICmd_Callback.fpGui_Send_LensCalibrationMenuOpenCb();
        #endif
    }

    if(edcLENS_APPLY_POSITION == uiDataCode) //T100_Larry_0050
    {
        sUtilCLICmd_Callback.fpGui_Send_LensMovingMenuOpenCb();
    }
    else if(edcLIGHT_SENSOR_CALIBRATION_VALUE == uiDataCode)//HICC2_Julie_0040//HICC2_Julie_0042
    {
        if(sDataIn->uData.lDataValue == 0)
        {
            sUtilCLICmd_Callback.fpGui_Send_ABP_Cal_Msg_OpenCb();
        }
    }

    if(sGlobalCfg.sFN_CFG.SupportPin)
    {
        if(edcPIN_PROTECT == uiDataCode) //A35G2_CDS_Coda_0033
        {
            if(sDataIn->uData.lDataValue == 0)
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_PIN_PROTECT_CLEAR, TRUE, NULL);//HICC2_Julie_0021
                //sUtilCLICmd_Callback.fpGui_SendPinProtectClearEventCb();
            }
        }
    }

#if 0 //HICC2_Julie_0026,ISS-0023888有設定Auto shutdown時在Pin protect畫面觸發功能後會馬上自動解除倒數.
    if((uiDataCode != edcSHOW_MAINSOURCE_MESSAGES_INFO) &&
       (uiDataCode != edcREAL_DATE_TIME) &&
       (uiDataCode != edcUI_STATUS_IS_OSD_OPEN))    //HICC2_Doulas_0070
    {
        sUtilCLICmd_Callback.fpEnvironment_AutoShutDownClearCb();	//A35G2_BRC_Coda_0015
    }
#endif

    if(eResult == eEXEC_CODE_PASS)
    {
        sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);  //H2PF_Simon_0067
#if 0
        //sUtilCLICmd_Callback.fpGui_SendUpdateOSDEventCb(); //A70LV_Larry_0250
        //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, eOSD_UPDATE_CLI, NULL); //GuiCb.fpGui_SendUpdateOSD_CLI_EventCb();
        if(palDataMgr_IsUIEventDataCode(uiDataCode) == FALSE)
        {
            LOG_MSG(db_ALWAYS,"(%s, %d)uiDataCode = %d\n", __FUNCTION__, __LINE__, uiDataCode);
            //sUtilCLICmd_Callback.fpLANProcUpdateOSD_SetCb(TRUE);
        }
#endif /* 0 */

        *pwByteCount = 0;
        return rcBUSY;
    }
    else
    {
        sUtilCLICmd_Callback.fpLANProcSendAckDoneCb(uiDataCode, (_eInterface)sDataIn->uiSendFrom, eResult);  //avoid web page always 30s refresh time    //G100_Simon_0051  //H2PF_Simon_0067
        sUtilCLICmd_Callback.fpLANProcSendToLANCb(uiDataCode);   //A35G2_CDS_Simon_0006

        *pwByteCount = 0;
        return rcERROR;
    }
}


//H30K_Tim_0011, add, ***
static eRESULT utilCLI_System_OESN1_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcOESN1, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;
    *pwByteCount = 0;

    return rcSUCCESS ;
}


static eRESULT utilCLI_System_OESN2_Set(sPAYLOAD *psInData,  sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    sUtilCLICmd_Callback.fpDataMgr_Data_AccessCb(edcOESN2, edaWRITE_RAM_ONLY_NO_ACTION, psInData->aucData) ;
    *pwByteCount = 0;

    return rcSUCCESS ;
}
//H30K_Tim_0011, add, &&&


// ==============================================================================
// End Of Static Function
// ==============================================================================
#define CMD_ID(MODULE, SUBCMD, RW) ((MODULE<<1|RW)<<8|SUBCMD)

static const sCMDCFG m_asCmdLut[] =
{
    //{ CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_DATA,                        CMD_READ),      utilCLI_LAN_Data_Get}, //A70LV_Larry_0066
    { CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_DATA,                        CMD_WRITE),     utilCLI_LAN_Data_Set}, //A70LV_Larry_0066
    { CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_ACK_DONE,                    CMD_WRITE),     utilCLI_LAN_AckDone_Set},
    { CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_TELNET_CMD,                  CMD_WRITE),     utilCLI_LAN_TelnetCmd_Set},
    { CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_TIME_CMD,                    CMD_WRITE),     utilCLI_LAN_TimeCmd_Set},//G100_Coda_0016
    { CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_SCHEDULE_CMD,                CMD_WRITE),     utilCLI_LAN_ScheduleCmd_Set},//G100_Coda_0025
    { CMD_ID(eCMD_MODULE_LAN,       eLAN_MSG_EMAIL_CMD,                	  CMD_WRITE),     utilCLI_LAN_EmailNotifyCmd_Set}, //A35G2_Coda_0048


    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_SYSTEM_STATE,             CMD_WRITE),     utilCLI_System_State},
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_SYSTEM_VERSION,           CMD_READ),      utilCLI_System_Version_Get},
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_REMOTE_CODE,              CMD_WRITE),     utilCLI_IR_decode},
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_HOST_READY,               CMD_WRITE),     utilCLI_HostReady},
	//{ CMD_ID(eCMD_MODULE_SYSTEM,	eSYSTEM_MSG_ERROR_LOG,				    CMD_WRITE), 	utilCLI_System_ErrorLog_Update},
	//{ CMD_ID(eCMD_MODULE_SYSTEM,	eSYSTEM_MSG_ERROR_LOG_INDEX,			CMD_WRITE), 	utilCLI_System_ErrorLog_Index},
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_TEMPERATURE,			  CMD_WRITE),     utilCLI_System_Temperature_Update},
	{ CMD_ID(eCMD_MODULE_SYSTEM,	eSYSTEM_MSG_ERROR_LOG_NOTIFY, 		  CMD_WRITE), 	  utilCLI_System_ErrorLog_Notify},  //T100_Simon_0147
//	{ CMD_ID(eCMD_MODULE_SYSTEM,	eSYSTEM_MSG_ROTATION_MODE_UPDATE, 	CMD_WRITE), 	utilCLI_System_G_SENSOR_MODE_Update},  //T100_Simon_0026
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_SYSTEM_POWER_OFF,         CMD_WRITE),     utilCLI_System_PowerOff},
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_HDBASET,                  CMD_WRITE),     utilCLI_System_HDBaseTCmd_Set}, //G100_Owen_0016
	{ CMD_ID(eCMD_MODULE_SYSTEM,	eSYSTEM_MSG_COMMUNICATION_MODE, 	  CMD_WRITE), 	  utilCLI_System_Enter2WMode},
	{ CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_FAST_POWER_ON_NOTIFY,     CMD_WRITE),     utilCLI_System_FastPowerOnNotifyFromMCU},	//G100_Coda_00107
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_LVPS_INFO,                CMD_WRITE),     utilCLI_System_LVPS_Info_Update}, //HICC2_Doulas_0165
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_DIM_POWER,                CMD_WRITE),     utilCLI_System_DimPowerSet},

#ifdef 	PLATFORM_H30_4K
	{ CMD_ID(eCMD_MODULE_SYSTEM,	eSYSTEM_MSG_BURN_IN_CONTROL,   		  CMD_WRITE),	  utilCLI_System_BurnIn_Control},
#endif
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_OESN1,                    CMD_WRITE),     utilCLI_System_OESN1_Set},    //H30K_Tim_0011, add
    { CMD_ID(eCMD_MODULE_SYSTEM,    eSYSTEM_MSG_OESN2,                    CMD_WRITE),     utilCLI_System_OESN2_Set},    //H30K_Tim_0011, add



    { CMD_ID(eCMD_MODULE_GEO,       eGEO_MSG_BLENDER,                     CMD_READ),      utilCLI_Geo_Event},
    { CMD_ID(eCMD_MODULE_GEO,       eGEO_MSG_BLENDER,                     CMD_WRITE),     utilCLI_Geo_Event},
    //FMT
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_FAN_INFO_ALL,                CMD_WRITE),     utilCLI_Fan_Info_Update},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_THERMAL_SENSOR_ALL,          CMD_WRITE),     utilCLI_System_Temperature_Update},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_LIGHT_SENSOR,                CMD_WRITE),     utilCLI_LightSensor_Set},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_LD_INFO_ALL,                 CMD_WRITE),     utilCLI_System_LD_Info_Update},
    { CMD_ID(eCMD_MODULE_FMT,		eFMT_MSG_LIQUID_SPEED,				  CMD_WRITE), 	  utilCLI_Pump_Info_Update},  //HICC2_Doulas_0009
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_TEC_CURRENT,                 CMD_WRITE),     utilCLI_TEC_Current_Update},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_FAN_DIMMING_MODE,            CMD_WRITE),     utilCLI_Dimming_Mode},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_PRESS_SENSOR,                CMD_WRITE),     utilCLI_Press_Update},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_G_SENSOR_DATA,               CMD_WRITE),     utilCLI_GSensorXYZ_Update},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_XFPGA_READY,                 CMD_WRITE),     utilCLI_XFPGA_Ready},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_FAN_SKU_RELOAD,              CMD_WRITE),     utilCLI_TE_ID_Update},
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_TEMP_WARNNING_STATUS,        CMD_WRITE),     utilCLI_System_ProServiceTemperatureSet}, //A35G2_BRC_Casper_0117
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_FAN_WARNNING_STATUS,         CMD_WRITE),     utilCLI_System_ProServiceFanStatusSet}, //A35G2_BRC_Casper_0117
    { CMD_ID(eCMD_MODULE_FMT,       eFMT_MSG_WHEEL_SPEED,                 CMD_WRITE),     utilCLI_WheelSpeedPolling},

    //OPD
    { CMD_ID(eCMD_MODULE_OPD,       eOPD_MSG_UART_STRESS_TEST,            CMD_READ),      utilLI_OPD_UART_GET},
    { CMD_ID(eCMD_MODULE_OPD,       eOPD_MSG_UART_STRESS_TEST,            CMD_WRITE),     utilLI_OPD_UART_SET},

    //FRONTEND
	{ CMD_ID(eCMD_MODULE_FE,        eFE_MSG_INPUT_SOURCE_DETECT,          CMD_WRITE),     utilCLI_Frontend_InputSourceDetect},  //A35G2_Simon_0075

    { CMD_ID(eCMD_MODULE_GUI,       eGUI_MSG_DATA,                        CMD_WRITE),     utilCLI_GUI_Data_Set}, //A70LV_Larry_0066
    { CMD_ID(eCMD_MODULE_GUI,       eGUI_MSG_ACK_DONE,                    CMD_WRITE),     utilCLI_GUI_AckDone_Set},
    { CMD_ID(eCMD_MODULE_GUI,       eGUI_MSG_DATA_NO_ACTIVE,              CMD_WRITE),     utilCLI_GUI_Data_Set},
    { CMD_ID(eCMD_MODULE_GUI,       eGUI_MSG_NO_ACTIVE_ACK_DONE,          CMD_WRITE),     utilCLI_GUI_AckDone_Set},
};

#define SIZE_OF_CMD_LUT sizeof(m_asCmdLut)/sizeof(m_asCmdLut[0])

// A70LH_Jonas_0016, Removed marked code(A70LH_Jonas_0015 marked)

// ==============================================================================
// FUNCTION NAME: UTILCLICMD_EXECUTE
// DESCRIPTION:
//
//
// Params:
// sPAYLOAD *psInData:
// sPAYLOAD *psOutData:
// WORD *pwByteCount:
//
// Returns:
//
//
// modification history
// --------------------
// 2014/03/31, Leo Create
// --------------------
// ==============================================================================
eACK_TYPE utilCLICmd_Execute(sPAYLOAD *psInData, sPAYLOAD *psOutData, UINT16 *pwByteCount)
{
    eRESULT eResult = rcERROR;
    UINT16 wCmdIndex = 0;
    UINT16 wCmd = 0;

    wCmd = ((psInData->uModule.ucModule) << 8) | psInData->ucSubCmd ;

    for(wCmdIndex = 0; wCmdIndex < SIZE_OF_CMD_LUT; wCmdIndex++)
    {
        if(m_asCmdLut[wCmdIndex].wCmdIndex == wCmd)
        {
            ASSERT(m_asCmdLut[wCmdIndex].pFunc != NULL);

            if(m_asCmdLut[wCmdIndex].pFunc != NULL)
            {
                eResult = m_asCmdLut[wCmdIndex].pFunc(psInData, psOutData, pwByteCount);
                LOG_MSG(db_UTL_CMD, "Module %d,SubCmd %d\r\n", (psInData->uModule.ucModule)>>1, psInData->ucSubCmd);
                break;
            }
        }
    }

    // 查無指令
    if(wCmdIndex >= SIZE_OF_CMD_LUT)
    {
        //ASSERT_ALWAYS();
        LOG_MSG(db_UTL_CMD, "\r\nUnknown Cmd - Module %d,SubCmd %d\r\n", (psInData->uModule.ucModule)>>1, psInData->ucSubCmd);
        return eACK_TYPE_UNKNOWN;
    }
    else
    {
        if(rcSUCCESS == eResult)
        {
            // 有資料需要回復
            if(*pwByteCount > 0)
            {
                return eACK_TYPE_FEEDBACK;
            }
            else
            {
                // 回應是否有收到
                return eACK_TYPE_ACK;
            }
        }
        else if(rcERROR == eResult)
        {
            return eACK_TYPE_ERROR;
        }
        else
        {
            return eACK_TYPE_NONACK;
        }
    }
}

eRESULT CheckSumCal_unpacket(BYTE *pcBuffer, WORD wByteCount) //G100_Julie_0029
{
    BYTE ucRxData = 0;

   for(UINT16 uiIndex =0; uiIndex < wByteCount; uiIndex++)
    {
        ucRxData = ucRxData + pcBuffer[uiIndex];
    }

    if(((UINT8)ucRxData & 0xFF) == 0)
	{
	    return rcSUCCESS;
	}
	else
	{
	    return rcERROR;
	}
}

UINT16 CheckSumCal_Packet(uint8 *psOutData, uint8 *psInData, UINT16 wSize) //G100_Julie_0029
{
    UINT8 ucCalValue = 0;
	DWORD dwData = 0;
    if(wSize > I2C_Tx_MAX_SIZE)
    {
        return rcERROR;
    }

    memcpy(psOutData, psInData, wSize);
    for(UINT16 uiIndex =0; uiIndex < wSize; uiIndex++)
    {
		ucCalValue = ucCalValue + psInData[uiIndex];
    }
	psOutData[wSize] = (~ucCalValue) + 1;

    return wSize + 1;
}

