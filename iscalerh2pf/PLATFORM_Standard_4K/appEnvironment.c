#include "appEnvironment.h"
#include "appMailBox.h"
#include "appPoll.h"
#include "appDataMgr.h"
#include "appSystem.h"
#include "appLedProcAPI.h"
#include "appIllumination.h"
#include "appDataPath.h"
#include "appLANProcAPI.h"
#include "appDataMgr.h"
#include "appInputProc.h"

#include "palCoreVar.h"
#include "palGui.h"
#include "palMotorMgr.h"
#include "palGeoAPI.h"
#include "palImgMgr.h"
#include "palLDMgr.h"

#include "utilCommon.h"
#include "utilDbgMsg.h"
#include "utilDataMgrAPI.h"
#include "utilOPD_TEST.h"
#include "utilDatabaseAPI.h"
#include "utilCLICmdAPI.h"

#include "GEC_EventTable.h"

#include "halLDProc.h"
#include "halFanCtrlAPI.h" //A70LV_Larry_0041
#include "halMotorCtrlAPI.h"
#include "halFormatter.h"
//#include "GEC_UserOption.h"  //A70LV_John_0008 add basic function and flow of GEC
#include "halScaler.h"
#include "halMCUCtrlAPI.h"

#include "ProjectSettings.h"
#include "palFormatterMgr.h"
#include "palSysCtrlMgr.h"

#define INSTANT_POWEROFF    (10)

static sPAL_ENVIRONMENT_INFORMATION       m_sPalEnvInfo;

#ifndef OE_JIG //A70LV_John_0038 fix OE JIG bug
static UINT8 MonitorFlag = 0; //A70LV_Larry_0031
#endif

extern sGUI_CALLBACK GuiCb;

static eAPP_ADC_STATE     m_AppADCState = eAPP_ADC_STATE_IDLE;    //A70LV_Doulas_0124
static UINT8              m_acRecalibration = 0;
static eADC_MGR_STATE     m_aeState = adcIDLE;
static UINT8              m_acCalStep;
//static ADC_CAL_VALUES     m_asCalData;
static RECT               m_asImageArea[2];
static eNBRESULT          m_aeStatus = nbINVALID;
static eNBRESULT          cCalState = nbINVALID;
static UINT8              ucADC_Poll_Count = 0;
#ifndef OE_JIG //A70LV_John_0038 fix OE JIG bug
static sFAILURE           m_sFailure;
#endif
static UINT8              uc3D_SYNC_Avaliable_Count = 0;        //A70LV_Doulas_0191
#ifndef FRAME_SEQUENTIAL_3D_ULTRA //H30K_Doulas_0003
static UINT8	          uc3D_SYNC_Unavaliable_Count = 0;
#endif
static BOOL               bLightSourceBlankingEn = FALSE;        //A70LV_Doulas_0294
static BOOL               bLightSourceBlankingOn = FALSE;       //A70LV_Doulas_0294
static UINT8              ucLightSourceBlankingCounter = 0;     //A70LV_Doulas_0294
static BOOL               bLensModelCheck = FALSE;	//G100_Clare_0059, add
static UINT8 			  ucLensModelCheckTestMax = 0;	//G100_Clare_0059, add
#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING                   //G100_Tim_0046, add, start
static eUPDATE_POWER_STATUS m_ucUpdate_Power_Status = eUPDATE_POWER_STATUS_MAX;
static UINT8                m_ucDelayCnt = 0;
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING                 //G100_Tim_0046, add, end
#ifdef CAL_ALWAYS_DONE
static UINT8              m_ucMOT_Cal_Flag = TRUE;   //G100_Owen_0076
#else
static UINT8              m_ucMOT_Cal_Flag = FALSE;   //G100_Owen_0076
#endif
static UINT8              m_ucMOT_Special_Flag = FALSE; //A35G2_Wesley_0163 //A70Gen2_Julie_0109//HICC2_Julie_0014
UINT8 aucLensEEPROMVersion[2] = {0}; //HICC2_Steven_0027
static UINT8 			  ucFEOPDCnt = 0;

static BOOL               bSend_PIN_Status = FALSE;        //HICC2_Steven_0078

#if 0
static POINT m_sULSampleArea[2][cstINVALID] =       //A70LV_Doulas_0124
{
    {
        /* ----------------- BEGIN CUSTOM AREA [030] palADCMgr.c ---------------------
         * CONFIGURATION: Portions of image to use for RGB and YUV calibration
         * expressed as tenths of percent of the dislay window */
        // Analog RGB Calibration Points on Gray16 Pattern
        {   25,  500 },     // cstRGB_BLACK
        {  950,  500 },     // cstRGB_WHITE
        {   87,  500 },     // cstRGB_BAR2
        {  915,  500 },     // cstRGB_BAR15

        // YPbPr Calibration Points on SMPTE Color Bar Pattern
        {  900,  850 },     // cstYUV_BLACK
        {  260,  850 },     // cstYUV_WHITE100
        {  900,  500 },     // cstYUV_BLUE75
        {  750,  500 },     // cstYUV_RED75
        {  200,  500 },     // cstYUV_YELLOW75
        {  350,  500 },     // cstYUV_CYAN75
    },
    {
        // Analog RGB Calibration Points on Gray16 Pattern
        {   25,  500 },     // cstRGB_BLACK
        {  950,  500 },     // cstRGB_WHITE
        {   87,  500 },     // cstRGB_BAR2
        {  915,  500 },     // cstRGB_BAR15

        // YPbPr Calibration Points on PAL 100/75 Color Bar Pattern
        {  900,  500 },     // cstYUV_BLACK
        {   50,  500 },     // cstYUV_WHITE100
        {  780,  500 },     // cstYUV_BLUE75
        {  640,  500 },     // cstYUV_RED75
        {  150,  500 },     // cstYUV_YELLOW75
        {  280,  500 },     // cstYUV_CYAN75
        /* ---------------------------- END CUSTOM AREA --------------------------- */
    }
};
#endif

eEXEC_CODE palEnvironment_InstantCooling(void)
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalEnvInfo.bInstantPowerOff = TRUE;

    return eEXEC_CODE_PASS;
}

void palEnvironment_StartCoolTimer(void)
{
    //TODO
    m_sPalEnvInfo.iCoolTimer = m_sPalEnvInfo.psSysConfiguration->Illum.uiCooldownDelay;
}


eEXEC_CODE palEnvironment_PowerStandby(void)
{
    //halBoard_IR_Enable_Set(eIR_ENABLE_ALL, 0); //A70LV_Larry_0023

    LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    return eEXEC_CODE_PASS;
}


eEXEC_CODE palEnvironment_PowerNormal(void)
{
    UINT8   ucData;
    UINT16  uiInput = 0;

    m_sPalEnvInfo.iCoolTimer = 0;
    m_sPalEnvInfo.ucCeilingMountCheck = 0;
    m_sPalEnvInfo.ucCurrentCeilingMount = 99;

    m_sPalEnvInfo.uiAutoShutDown = 0; //A70LV_Larry_0082
    m_sPalEnvInfo.uiSleepTimer = 0;   //A70LV_Larry_0082
    m_sPalEnvInfo.ucSystemCount = 0;  //A70LV_Larry_0082
    m_sPalEnvInfo.ulSystemTimer = 0;  //A70LV_Larry_0082
    m_sPalEnvInfo.ucAutoSourceSearchTimer = 0;   //A70LV_Doulas_0082
    m_sPalEnvInfo.ucAutoSourceWaitTimeToChange = 0;

    m_sPalEnvInfo.iWarmupTimer = 0; //A70LV_Larry_0135
    m_sPalEnvInfo.ucLensID = 99; //A70LV_Larry_0188
    m_sPalEnvInfo.ucLost_12V = 0; //A70LV_Larry_0276
    m_sPalEnvInfo.cNetworkWaitMessage = 0; //A70LV_Larry_0278
    m_sPalEnvInfo.cNetworkStatusMessageCount = 0;
    m_sPalEnvInfo.ucMainSource = 99;
    m_sPalEnvInfo.ucSubSource = 99;
    m_sPalEnvInfo.cNetworkReadyCount = 180; //A70LV_Larry_0411
    m_sPalEnvInfo.bInstantPowerOff = FALSE;

#ifndef OE_JIG //A70LV_John_0038 fix OE JIG bug
    m_sFailure.ucGroup = 0;
    m_sFailure.uiFailure = 0;
    m_sFailure.ucLD_OverTemperature[0] = 0xFF;
    m_sFailure.ucLD_OverTemperature[1] = 0xFF;
    m_sFailure.ucLD_OverTemperature[2] = 0xFF;
    m_sFailure.ucLD_OverTempCount = 0;
#endif
    m_sPalEnvInfo.uiFailureList = eFAILURE_NUMBERS;  //A70LV_Larry_0357
    m_sPalEnvInfo.ucFailureCount = 0;  //A70LV_Larry_0357
    m_sPalEnvInfo.iWaitWarmupTimer = 30;
    m_sPalEnvInfo.ucMonitorFlag = 0;
    m_sPalEnvInfo.ucOPDRunTimeFlag = 3;
    m_sPalEnvInfo.ucLensChanged = FALSE; //HICC2_Julie_0071//H30K_Julie_0005

    palEnvironment_NetworkIsReceivePowerOnCmd_Set(FALSE);

    palEnvironment_Send_PIN_StatusSet(FALSE); //HICC2_Steven_0078

	ucData = FALSE;
	palDataMgr_Data_Access(edcSYSTEM_UPDATE_FLAG, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);

    LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if((BOOL)utilDbgMsg_Get(db_KEEP_UART_SW))
    {
        palDataMgr_Data_Access(edcUART_SWITCH, edaREAD,(void*)&ucData);

        LOG_MSG(db_ALWAYS, "Keep Uart %d \r\n", ucData);

        MS_SLEEP(10);
        palDataMgr_Data_Access(edcUART_SWITCH, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&ucData);
    }
    else
    {
        palDataMgr_Data_Access(edcSERIAL_PORT_PATH, edaREAD,(void*)&ucData); //A70LV_Larry_0216
        palDataMgr_Data_Access(edcSERIAL_PORT_PATH, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);
    }

    palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, (void*)&ucData); //A70LV_Larry_0216

    //HIGH ALTITUDE
    palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaREAD, &ucData);                     //A70LV_Doulas_0148 modify
    if(eFUNC_CONTROL_ENABLE == palDataMgr_Control_HighAltitude())//HICC2_Julie_0020
    {
        palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData); //A70LV_Larry_0105
    }
    else
    {
        palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_THROUGH_NO_ACTION, &ucData); //A70LV_Larry_0105
    }
    palLANProcSendToLAN(edcHIGH_ALTITUDE);//HICC2_Julie_0034

	ucData = 1;
	palDataMgr_Data_Access(edcSLEEP_TIMER_RESET,edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
	palDataMgr_Data_Access(edcAUTO_SHUTDOWN_TIMER_RESET,edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    //12V TRIGGER
    palDataMgr_Data_Access(edc12V_TRIGGER, edaREAD, &ucData);                       //A70LV_Doulas_0148
    palDataMgr_Data_Access(edc12V_TRIGGER, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    //Backlight Preferences
    palDataMgr_Data_Access(edcKEYPAD_BACKLIGHT, edaREAD, &ucData);                      //A70LV_Doulas_0148
    palDataMgr_Data_Access(edcKEYPAD_BACKLIGHT, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcSTATUS_LED, edaREAD, &ucData);
    palDataMgr_Data_Access(edcSTATUS_LED, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    //Menu Offset  //A70LV_Larry_0258
    palDataMgr_Data_Access(edcMEMU_HORZ_OFFSET, edaREAD, &ucData);
    palDataMgr_Data_Access(edcMEMU_HORZ_OFFSET, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
    palDataMgr_Data_Access(edcMEMU_VERT_OFFSET, edaREAD, &ucData);
    palDataMgr_Data_Access(edcMEMU_VERT_OFFSET, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    	//OMS(=pro service) LED status //A65_OPTOMA_Julie_0085 //A35G2_Coda_0124
	ucData = ePROSERVICE_TEMPERATURE_STATUS_GREEN;
	palDataMgr_Data_Access(edcPROSERVICE_TEMPERATURE_STATUS, edaWRITE_THROUGH_WITH_ACTION, &ucData);
	palDataMgr_Data_Access(edcPROSERVICE_FAN_STATUS, edaWRITE_THROUGH_WITH_ACTION, &ucData);

    ucData = ets_OFF;
    palDataMgr_Data_Access(edcCLOSE_LOOP_ENABLE_AXIS0, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    ucData = ets_OFF;
    palDataMgr_Data_Access(edcCLOSE_LOOP_ENABLE_AXIS1, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

    palDataMgr_Data_Access(edcNOTIFICATIONSYSTEMUPDATE, edaREAD, &ucData);   //A35G2_Owen_0005
    if(ucData == ets_ON)     //A35G2_Owen_0005
    {
        m_sPalEnvInfo.ucFotaSystemUpdateTimer = 0;
    }
    else
    {
        m_sPalEnvInfo.ucFotaSystemUpdateTimer = 0xFF;
    }

    /*if(rcSUCCESS == halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiInput)) //H30K_Doulas_0073
    {
        palDataMgr_Data_Access(edcSOURCE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &uiInput);
        palDataPath_InputDetectSet(uiInput);
        palDataMgr_OPDInputPlugInState(eOPD_INPUT_DETECT_LOG);
        palDataPath_LastInputDetectSet(uiInput);
    }*/

    palMotorLens_Init(); //HICC2_Doulas_0147
    palEnvironment_LensSpecialFlag_Set(FALSE); //A35G2_Wesley_0165 //A70Gen2_Julie_0109//HICC2_Julie_0014

    palMotor_LensCenterSettingGet(&ucData);

#if defined(CUSTOM_CHRISTIE)
#if defined (PLATFORM_H60_2K) || defined (PLATFORM_H60_4K)
    if(ucData)
    {
		//????????,???????????,?????lens center??????
		//2024??40????? //Larry
        UINT8 ucSN[24] = {'\0'};

        palDataMgr_Data_Access(edcSERIAL_NUMBER, edaREAD, (void*)ucSN);

        //ex: U24V2435001
        if(ucSN[4] == '2' && ucSN[5] == '4' && //year
           ucSN[6] < '4') //weak < 40
        {
            //lens
            ucData = 0;

            palDataMgr_Data_Access(edcLENS_CENTER_SETTING, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);
        }
    }
#endif
#endif

    palDataMgr_Data_Access(edcLENS_CENTER_SETTING, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&ucData); //HICC2_Casper_0030

    LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    return eEXEC_CODE_PASS;
}



INT16 palEnvironment_Poll(UINT16 uiTick)
{
    BOOL    bLightSourceOn = FALSE;
    UINT8   ucCeilingMount = 99;
    UINT8   ucRear = 99;
    UINT8 ucInputKey = eSOURCE_KEY_CHANGE_SOURCE;   //A70LV_Doulas_0082
    //UINT8 ucPIP_Enable = ets_MAX_NUMBER;          //A70LV_Doulas_0201  //A70LV_Doulas_0082
    eEXEC_CODE eResult = eEXEC_CODE_PASS;           //A70LV_Doulas_0082

    if(utilCounterGet(eCOUNTER_TYPE_ENVIRONMENT) != 0)
    {
        return ENVIRONMENT_PERIOD / POLL_PERIOD;
    }

    utilCounterSet(eCOUNTER_TYPE_ENVIRONMENT, 1000);

    bLightSourceOn = palCoreVar_GetLightSourceOnVar();

    //send system power on state to network until network ready. //A70LV_Larry_0135
    if((palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE) &&
       (m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReady == FALSE) //&&
       /*bLightSourceOn*/) //A70LV_Larry_0323
    {
        m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReady =TRUE ;

        //INT32 ucData = 1 ;
        //palLANProcPowerStateSend(ePOWER_STATE_ACTIVE, 4, (UINT8*)&ucData);
        //halNetwork_SendData(eNETWORK_DATA_POWER_STATE, ePOWER_STATE_ACTIVE, aucDataString, strlen((char *)aucDataString));
    }

    /****************************************************/
    /* Run cooldown timer.                              */
    /*                                                  */
    /* Countdown is inhibited if lamp ever remains lit  */
    /* after cooldown is initiated. Cooldown is started */
    /* by the illumination power-down sequence. This    */
    /* test is a backstop to ensure that cooldown will  */
    /* not complete when the lamp is lit.               */
    /****************************************************/
    //send warmup timer to network //A70LV_Larry_0172

    if(palSystem_PowerStateGet() == ePOWER_STATE_WARMUP ||
       palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE ||
       palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)
    {

		if(palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)
		{
			m_sPalEnvInfo.iWarmupTimer = m_sPalEnvInfo.iWaitWarmupTimer + 1;
		}

        if(m_sPalEnvInfo.iWarmupTimer <= m_sPalEnvInfo.iWaitWarmupTimer)
        {
            LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)WarmupTimer Timer is [%d]\r\n", __FUNCTION__, __LINE__, m_sPalEnvInfo.iWarmupTimer);

            INT32 lWarmupTimer = m_sPalEnvInfo.iWarmupTimer ;

            palLANProcPowerStateSend(ePOWER_STATE_WARMUP, 4, (UINT8*)&lWarmupTimer);

            m_sPalEnvInfo.iWarmupTimer++ ;

            if(m_sPalEnvInfo.iWarmupTimer == m_sPalEnvInfo.iWaitWarmupTimer)
            {
                if(palDataPath_DataPathIsReady() == FALSE)
                {
                    m_sPalEnvInfo.iWarmupTimer = m_sPalEnvInfo.iWaitWarmupTimer - 1;
                }
            }

            if(m_sPalEnvInfo.iWarmupTimer == 1) //A35G2_CDS_Larry_0051
            {
                UINT16 ucLensMemSavingCondition = 0 ;

                palEnvironment_LensCalFlag_Update();

                halMotor_LensMemoryRecord_Get((UINT8 *)&ucLensMemSavingCondition);       //G100_Simon_0048
                palEnvironment_LensMemorySavingCondition_Set(ucLensMemSavingCondition);
                palDataMgr_WarpingMemorySaveConditionUpdate();  //G100_Simon_0048
                palDataMgr_BlendingMemorySaveConditionUpdate();  //G100_Simon_0048

				bSend_PIN_Status = TRUE; //HICC2_Steven_0078
                palDataMgr_PIN_Protect_Status();

				utilOPD_Runtime_Cnt_Rst();//A70Gen2_Julie_0017
            }

            if(m_sPalEnvInfo.iWarmupTimer >= 16  &&
               m_sPalEnvInfo.iWarmupTimer <= 20)
            {
                utilOPD_Runtime_Flag_Set(TRUE);

    			if(palSystem_AC_Voltage_Check() != rcSUCCESS)
    			{
    				LOG_MSG(db_APP_SYSTEM, "palEnvironment_Poll(), AC Voltage Check Fail\r\n");
    			}
            }
        }
        else
        {
            //notify network : system warm up is finish     //T100_Simon_0011
            if(palEnvironment_NetworkIsReceivePowerOnCmd_Get() == FALSE)
            {
                palEnvironment_NetworkIsReceivePowerOnCmd_Set(TRUE);   //temp : only send once

                INT32 ucData = 0 ;

                palLANProcPowerStateSend(ePOWER_STATE_ACTIVE, 4, (UINT8*)&ucData);

                if(palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)  //HICC2_Doulas_0112
                {
                    palLANProcFocusPowerStatusSend();
                }

                utilOPD_Runtime_Flag_Set(TRUE); //G100_Steven_0078
                //palDataMgr_OPDSnapshot(eOPD_POWER_ON_LOG);
            }
        }
    }
    else
    {
        if(m_sPalEnvInfo.iCoolTimer > 0)
        {
            if(!bLightSourceOn)
            {
                m_sPalEnvInfo.iCoolTimer--;

                if(m_sPalEnvInfo.bInstantPowerOff)
                {
                    LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)InstantPowerOff\r\n", __FUNCTION__, __LINE__);
                    m_sPalEnvInfo.bInstantPowerOff = FALSE;
	//Cassper_ProAV
                    m_sPalEnvInfo.iCoolTimer = INSTANT_POWEROFF;
	//Cassper_ProAV
                }
            }
            else
            {
                m_sPalEnvInfo.iCoolTimer = 0;
            }

            //send Cooling timer to network //A70LV_Larry_0135
            if(m_sPalEnvInfo.iCoolTimer > 0)
            {
                INT32 lCoolTimer = m_sPalEnvInfo.iCoolTimer;

                m_sPalEnvInfo.bSourceSetting = FALSE;

                LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)Cool Timer is [%d]\r\n", __FUNCTION__, __LINE__, m_sPalEnvInfo.iCoolTimer);

                palLANProcPowerStateSend(ePOWER_STATE_COOLING, 4, (UINT8*)&lCoolTimer);

                //send current power state (entering standby) to network
#if 0 //HICC2_Steven_0037
                if(m_sPalEnvInfo.iCoolTimer == 1)
                {
                    INT32 ucData = 1 ;

                    palLANProcPowerStateSend(ePOWER_STATE_STANDBY, 4, (UINT8*)&ucData);
                }
#endif
            }

            if(m_sPalEnvInfo.iCoolTimer == 0)
            {
                LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)Cool Timer is complete\r\n", __FUNCTION__, __LINE__);
#if 0

                //Because we do not turn off fan in power up fail until 4th
                if(!corevariable_GetIsPowerUpFailVal())
                {
                    enviro_DisableFan();    /* stop cooling fans */
                }

#endif
                palSystem_CoolDown();      /* inform system of cooldown completion */
                palSystem_Idle();

                for(UINT8 cCount = 0; cCount < 120; cCount++) //HICC2_Steven_0037
                {
                    UINT8 ucWaitMCUState = eSYSTEM_MCU_STATE_POWERDOWN;
    				UINT8 ucStandbyMode = eCM_STANDBY_MODE_COMMUNICATION;

                    palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, &ucStandbyMode);
                    if((ucStandbyMode == eCM_STANDBY_MODE_COMMUNICATION) || (ucStandbyMode == eCM_STANDBY_MODE_COMMUNICATION_20MIN))
                    {
                        ucWaitMCUState = eSYSTEM_MCU_STATE_COMMUNICATION;
                    }
                    LOG_MSG(db_APP_ENVIRONMENT, "ucWaitMCUState = %d\r\n", ucWaitMCUState);
                	if(palSystem_PowerStateGet() == ePOWER_STATE_STANDBY && palSystem_GetMcuState() == ucWaitMCUState)
                    {
                        break;
                    }

                    MS_SLEEP(500);

                }
                {
                    INT32 ucData = 1 ;
				    uOPD_DATA uOPDData = {0};

				    uOPDData.ulMCU_SYSTEM_STATE = palSystem_GetMcuState();
                    utilOPD_EventSet(eOPD_MCU_SYSTEM_STATE_LOG, &uOPDData);
                    LOG_MSG(db_APP_ENVIRONMENT, "ePOWER_STATE_STANDBY\r\n");
                    palLANProcPowerStateSend(ePOWER_STATE_STANDBY, 4, (UINT8*)&ucData);
                } //HICC2_Steven_0037
            }

            if((palSystem_PowerStateGet() == ePOWER_STATE_COOLING) && (m_sPalEnvInfo.bSourceSetting == FALSE))      //G100_Doulas_0019 Save input source
			{
				UINT8 ucMainSource = eCM_SOURCE_HDMI1;
				palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucMainSource);
				palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_NO_ACTION, &ucMainSource);
				m_sPalEnvInfo.bSourceSetting = TRUE;
			}

            if((palSystem_PowerStateGet() == ePOWER_STATE_COOLING) && (m_sPalEnvInfo.ucOPDRunTimeFlag)) //G100_Julie_0047
			{
			    m_sPalEnvInfo.ucOPDRunTimeFlag--;
                if(0 == m_sPalEnvInfo.ucOPDRunTimeFlag)
                {
					utilOPD_I2C_RunTime_RecordEvent(eOPD_RUNTIME_LOG); //G100_Julie_0017 //save i2c retry&error&total count on run time.
					utilOPD_Write2RunTimeFile();			   //G100_Julie_0017
					utilOPD_TotalProjectorHour_CM(); //HICC2_Doulas_0003//G100_Julie_0022
					utilOPD_LD_Hour_CM();
					utilOPD_Runtime_Flag_Set(FALSE); //G100_Steven_0078
					utilOPD_Runtime_Cnt_Rst(); //G100_Steven_0079
                }
			}
        }
    }

    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
        if(palDataPath_GetDataPathState() >= eDATA_PATH_STATE_BEGIN_SCAN)
        {
            static UINT8 ucLast_HDBT_Status = 0xFF;
            UINT8 ucCur_HDBT_Status[eSYSTEM_MSG_SYSTEM_HDBT_STATUS_SZ];

            if(halMCUCtrl_HDBT_Status_Get(ucCur_HDBT_Status) == rcSUCCESS)
            {
                palDataMgr_Data_Access(edcHDBaseT_Status, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucCur_HDBT_Status[0]);
                if((ucLast_HDBT_Status & BIT1) != (ucCur_HDBT_Status[0] & BIT1))   //only Link status change record to OPD
                {
    				uOPD_DATA uOPDData = {0};
                    LOG_MSG(db_APP_ENVIRONMENT, "ucCur_HDBT_Status %d, ucLast_HDBT_Status %d\r\n",ucCur_HDBT_Status[0], ucLast_HDBT_Status);
                    //OPD
                    for(UINT8 ucIndex = 0 ; ucIndex < eSYSTEM_MSG_SYSTEM_HDBT_STATUS_SZ ; ucIndex++)
                    {
                        uOPDData.sSnapshotLog.ulHDBT_Status[ucIndex] = ucCur_HDBT_Status[ucIndex];
                    }
                    utilOPD_EventSet(eOPD_HDBT_STATUS_LOG, &uOPDData);
                }
                ucLast_HDBT_Status = ucCur_HDBT_Status[0];
            }
        }
//A70LV_Larry_0082 [[
        m_sPalEnvInfo.ucSystemCount++;
        if((m_sPalEnvInfo.ucSystemCount % 60) == 0)
        {
            m_sPalEnvInfo.ucSystemCount = 0;
            m_sPalEnvInfo.ulSystemTimer++;

            LOG_MSG(db_APP_ENVIRONMENT, "System Timer %d\r\n", m_sPalEnvInfo.ulSystemTimer);
        }

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
        if(palIllumination_BurnIn_Enable() == 0 && m_sPalEnvInfo.iSecurityPowerDownTimer != 0) //G100_Coda_0071
        {
            m_sPalEnvInfo.iSecurityPowerDownTimer--;

            if(m_sPalEnvInfo.iSecurityPowerDownTimer <= 60)
            {
                palEnvironment_SecurityPowerDownTimerSet(m_sPalEnvInfo.iSecurityPowerDownTimer, TRUE);
            }
            else
            {
                palEnvironment_SecurityPowerDownTimerSet(m_sPalEnvInfo.iSecurityPowerDownTimer, FALSE);
            }

            if(m_sPalEnvInfo.iSecurityPowerDownTimer == 0)
            {
				//G100_Clare_0055, add, >>>
                UINT8 ucData = 0 ;
                palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucData);
                if (ucData == TRUE)
                {
                    palEnvironment_Fake_Power_Down_Set(TRUE);
                }
                else
				//G100_Clare_0055, add, <<<
                {
                    palSystem_PowerDown();
                }
                LOG_MSG(db_APP_ENVIRONMENT, "Security Timer Shut Down\r\n");
            }

            LOG_MSG(db_APP_ENVIRONMENT, "Security Timer %d\r\n", m_sPalEnvInfo.iSecurityPowerDownTimer);
        }
#endif

        if(palIllumination_BurnIn_Enable() == 0 && bLightSourceOn)
        {
            if((palDataPath_IsSourceLock() == TRUE) ||
               (palDataPath_IsSourceLockSub() == TRUE && eFUNC_CONTROL_ENABLE == palDataMgr_DataCode_Control(edcSUB_INPUT)) ||
               (palDataMgr_CurTestPatternGet() != eTID_OFF) //A70LV_Larry_0276
               )
            {
                //palEnvironment_AutoShutDownClear();

                //UINT8 ucReset = 1;
                //palDataMgr_Data_Access(edcAUTO_SHUTDOWN_TIMER_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucReset);
                palEnvironment_AutoShutDownTimerSet(m_sPalEnvInfo.uiAutoShutDownDefault, TRUE);
            }
            else
            {
                if(m_sPalEnvInfo.uiAutoShutDown != 0) //G100_Larry_0019
                {
                    BOOL bCheckDrawOSD = TRUE;

                    m_sPalEnvInfo.uiAutoShutDown--;

                    if(m_sPalEnvInfo.uiSleepTimer != 0)
                    {
                        if(m_sPalEnvInfo.uiAutoShutDown >= m_sPalEnvInfo.uiSleepTimer)
                        {
                            bCheckDrawOSD = FALSE;
                        }
                    }

                    if(bCheckDrawOSD)
                    {
                        if(m_sPalEnvInfo.uiAutoShutDown <= 60)
                        {
                            palEnvironment_AutoShutDownTimerSet(m_sPalEnvInfo.uiAutoShutDown, TRUE);
                        }
                        else
                        {
                            palEnvironment_AutoShutDownTimerSet(m_sPalEnvInfo.uiAutoShutDown, FALSE);
                        }

                        if(m_sPalEnvInfo.uiAutoShutDown == 0)
                        {
							//G100_Clare_0055, add, >>>
                            UINT8 ucData = 0 ;
                            palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucData);
                            if (ucData == TRUE)
                            {
                                palEnvironment_Fake_Power_Down_Set(TRUE);
                            }
                            else
							//G100_Clare_0055, add, <<<
                            {
                                utilDataMgr_WriteGecLog(AutoSystemShutdown);
                                palSystem_PowerDown();
                            }
                            LOG_MSG(db_APP_ENVIRONMENT, "Auto Shut Down Down\r\n");
                        }
                    }

                    LOG_MSG(db_APP_ENVIRONMENT, "Auto Shut Down Timer %d\r\n", m_sPalEnvInfo.uiAutoShutDown);
                }
            }

            if(m_sPalEnvInfo.iReplaceModelPowerDownTimer != 0) //H30K_David_0006
			{
                m_sPalEnvInfo.iReplaceModelPowerDownTimer--;

                if(m_sPalEnvInfo.iReplaceModelPowerDownTimer == 0)
                {
					palSystem_PowerDown();
					LOG_MSG(db_APP_ENVIRONMENT,"PowerDown for Model Replace/Switch.\r\n");
                }

                LOG_MSG(db_APP_ENVIRONMENT, "(%s,%d) Model Replace/Switch Power Down Timer %d\r\n", __FUNCTION__, __LINE__, m_sPalEnvInfo.iReplaceModelPowerDownTimer);
			}

            if(m_sPalEnvInfo.uiSleepTimer != 0)
            {
                BOOL bCheckDrawOSD = TRUE;

                m_sPalEnvInfo.uiSleepTimer--;

                if(m_sPalEnvInfo.uiAutoShutDown != 0)// HICC2_Bruce_0006
                {
                    if(m_sPalEnvInfo.uiSleepTimer > m_sPalEnvInfo.uiAutoShutDown)
                    {
                        bCheckDrawOSD = FALSE;
                    }
                }

                if(bCheckDrawOSD)
                {
                    if(m_sPalEnvInfo.uiSleepTimer <= 60)
                    {
                        #ifdef CUSTOM_OPTOMA //H30K_David_0053
						UINT8 ucFreeze= 0;
    					palDataMgr_Data_Access(edcIMAGE_FREEZE, edaREAD, &ucFreeze);
						if( ucFreeze==FALSE)
                        palLANProcSendToLAN(edcSLEEP_TIMER_COUNT);
						#else
						palLANProcSendToLAN(edcSLEEP_TIMER_COUNT);
      					#endif
                    }

                    if(m_sPalEnvInfo.uiSleepTimer == 0)
                    {
						//G100_Clare_0055, add, >>>
                        UINT8 ucData = 0 ;
                        palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD, &ucData);
                        if (ucData == TRUE)
                        {
                            palEnvironment_Fake_Power_Down_Set(TRUE);
                        }
                        else
						//G100_Clare_0055, add, <<<
                        {
                            utilDataMgr_WriteGecLog(SleepShutdown);
                            palSystem_PowerDown();
                        }
                        LOG_MSG(db_APP_ENVIRONMENT, "Sleep Timer Shut Down\r\n");
                    }
                }

                LOG_MSG(db_APP_ENVIRONMENT, "Sleep Timer %d\r\n", m_sPalEnvInfo.uiSleepTimer);
            }

            if( m_sPalEnvInfo.uiOSD_Unlock_Timer != 0 )	//G100_Clare_0022, add, start
            {
                m_sPalEnvInfo.uiOSD_Unlock_Timer--;

                if(m_sPalEnvInfo.uiOSD_Unlock_Timer == 0)
                {
                    #if (ENABLE_COLOR_UNIFORMITY == TRUE)      //G100_Tim_0012, add, start  //A35G2_CDS_Coda_0027
					UINT8 ucACU_Status = eACU_NOT_EXECUTED;
					palDataMgr_Data_Access(edcACU_EXECUTE, edaREAD, &ucACU_Status);
                    #endif //ENABLE_COLOR_UNIFORMITY           //G100_Tim_0012, add, end

                    palDataMgr_Camera_OSD_Lock_Set(eOSD_NOT_LOCKED);                   //G100_Clare_0024, mod //A35G2_Coda_0052
                    if( palDataMgr_AutoFocusExecute_Get() >= eAF_AC_EXECUTE && palDataMgr_AutoFocusExecute_Get() <= eAF_AC_EXECUTE_NO_UIMSG )		//A65_OPTOMA_Doulas_0039 Modify
                    {
                    	 UINT8 ucStatus = 0;
                         palDataMgr_AutoFocusExecute_Set(eAF_AC_NOT_EXECUTE);
						 ucStatus = eAF_STATUS_TIMEOUT;

                         palDataMgr_Data_Access(edcGEO_TESTPATTERN_OFF, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStatus);

                         palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStatus); //using auto focus timeout message
                         palDataMgr_UI_EventSend(edcUI_EVENT_AF_CAL_FAIL, eAF_STATUS_TIMEOUT, NULL); //using auto focus timeout message
                    }
                    else if( palDataMgr_AutoWallColorExecute_Get() >= eAF_AC_EXECUTE && palDataMgr_AutoWallColorExecute_Get() <= eAF_AC_EXECUTE_NO_UIMSG)
                    {
                        palDataMgr_Auto_Wall_Color_Set(eAF_AC_NOT_EXECUTE);
                        palDataMgr_AutoFocusExecute_Set(eAF_AC_NOT_EXECUTE);
                        #if defined(CUSTOM_OPTOMA) || defined(CUSTOM_CHRISTIE)
                        //appGui_Auto_Wall_Color_Show( eAWC_STATUS_TIMEOUT ); //A35G2_Coda_0055
                        UINT8 ucStatus = 0;
                        ucStatus = eAF_STATUS_TIMEOUT;

                        palDataMgr_Data_Access(edcGEO_TESTPATTERN_OFF, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStatus);
                        palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStatus); //using auto focus timeout message
                        palDataMgr_UI_EventSend(edcUI_EVENT_AF_CAL_FAIL, eAF_STATUS_TIMEOUT, NULL); //using auto focus timeout message
                        #endif
                    }
                    else if(palDataMgr_AutoColorMatchExecute_Get() >= eAF_AC_EXECUTE && palDataMgr_AutoColorMatchExecute_Get() <= eAF_AC_EXECUTE_NO_UIMSG)
                    {
                        UINT8 ucStatus = 0;
                        palDataMgr_AutoColorMatchExecute_Set(eAF_AC_NOT_EXECUTE);
                        palDataMgr_AutoFocusExecute_Set(eAF_AC_NOT_EXECUTE);

                        palDataMgr_Data_Access(edcGEO_TESTPATTERN_OFF, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStatus);
                        palDataMgr_Data_Access(edcAUTO_FOCUS_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStatus); //using auto focus timeout message
                        palDataMgr_UI_EventSend(edcUI_EVENT_AF_CAL_FAIL, eAF_STATUS_TIMEOUT, NULL); //using auto focus timeout message
                    }

#if (ENABLE_COLOR_UNIFORMITY == TRUE)		//G100_Tim_0012, add, start
                    // ACU time out
                    else if( ucACU_Status >= eACU_EXECUTING && ucACU_Status <= eACU_EXECUTING_NO_UIMSG )
                    {
                        ucACU_Status = eACU_STATUS_TIME_OUT;
                        palDataMgr_Data_Access(edcACU_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &ucACU_Status);
                    }
#endif //ENABLE_COLOR_UNIFORMITY			//G100_Tim_0012, add, end
                }

				LOG_MSG(db_APP_ENVIRONMENT, "OSD unlock Timer %d\r\n", m_sPalEnvInfo.uiOSD_Unlock_Timer);
            }	//G100_Clare_0022, add, end

            if(m_sPalEnvInfo.cNetworkWaitMessage != 0) //A70LV_Larry_0278
            {
                m_sPalEnvInfo.cNetworkWaitMessage--;
                if(m_sPalEnvInfo.cNetworkWaitMessage == 0)
                {
                    UINT8 ucNetworkWaitMessage = 0;

                    palDataMgr_Data_Access(edcNETWORK_MESSAGE, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&ucNetworkWaitMessage);
                }
            }
        #ifdef NETWORK_IP_DHCP //HICC2_Julie_0064
            if(m_sPalEnvInfo.cNetworkStatusMessageCount != 0)
            {
                m_sPalEnvInfo.cNetworkStatusMessageCount--;
                if(m_sPalEnvInfo.cNetworkStatusMessageCount == 0)
                {
                    UINT8 cNetworkStatusCount = eCM_NETWORK_STATUS_MESSAGE_IDLE;
                    //LOG_MSG(db_ALWAYS, "%s(eCM_NETWORK_STATUS_MESSAGE_IDLE)\n", __func__);
                    palDataMgr_Data_Access(edcNETWORK_STATUS_MESSAGE, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&cNetworkStatusCount);
                }
            }
        #endif

            if((m_sPalEnvInfo.ucSystemCount % 3) == 0) //A70LV_Larry_0188 //A70LK_Jacky_0026
            {
                UINT8 ucLensSpeed = 0;

                UINT8 ucID = palMotor_LensId_Get();
                UINT8 ucFujiDet = palMotor_LensLock_State_Get();

                if(m_sPalEnvInfo.ucLensID != ucID)
                {
                    m_sPalEnvInfo.ucLensID = ucID;

                    palDataMgr_Data_Access(edcLENS_TYPE, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&m_sPalEnvInfo.ucLensID);

                    palDataMgr_Data_Access(edcLENS_SPEED, edaREAD, &ucLensSpeed);
                    palDataMgr_Data_Access(edcLENS_SPEED, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&ucLensSpeed);

                    aucLensEEPROMVersion[1] = 0;  //HICC2_Steven_0027
                    aucLensEEPROMVersion[0] = 0;
                    palMotor_LENS_EEPROM_Version_Get(aucLensEEPROMVersion);
                    LOG_MSG(db_APP_ENVIRONMENT,"LSM Ver.%02d.%02d\r\n",aucLensEEPROMVersion[0], aucLensEEPROMVersion[1]);
                }

                if(palInput_Fuji_Lens_Detect_Get() != ucFujiDet) //HICC2_Julie_0071//H30K_Julie_0005
                {
                    if((ucID == eLENS_ID_B20U100) || (ucID == eLENS_ID_NVRAM_B20U100))
                    {
                        palDataMgr_Data_Access(edcFUJI_LENS_DETECT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucFujiDet);
                    }
                }
            }

            if((m_sPalEnvInfo.ucSystemCount % 30) == 0) //HICC2_Steven_0032
            {
#ifndef PLATFORM_H30_4K
            	palEnvironment_DMD_Airtight_Get();
#endif
            }

            if(m_sPalEnvInfo.cLensCalibrationTimer != 0) //A70LV_Larry_0129
            {
                m_sPalEnvInfo.cLensCalibrationTimer--;

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0020
                halMotor_LSM_Status_Ask();
                if(m_sPalEnvInfo.cLensCalibrationTimer < (900 - 2)) //HICC2_AC_0020
                {
                    if((palMotorLensCalibrationStatusGet() == 0) || (m_sPalEnvInfo.cLensCalibrationTimer == 0))
#else
                if(m_sPalEnvInfo.cLensCalibrationTimer < (900 - 1)) //HICC2_AC_0020
                {
                    /*
                    if(m_sMenuState.cCurrentMenuIndex != LENS_CALIBRATION_MENU_ICOUNT && m_sMenuState.cCurrentMenuIndex != USTINSTALL_MESSAGE1_MENU_ICOUNT) //A35G2_BRC_Casper_0088)
                    {
                        m_sPalEnvInfo.cLensCalibrationTimer = 0;
                        appGui_Send_LensCalibrationMenuExit();
                        palEnvironment_LensCalFlag_Update();
                    }
                    else */

                    if(((palMotorLensCalibrationStatusGet() == 0) && (palMotorLensMovingStateGet() == 0)) || (m_sPalEnvInfo.cLensCalibrationTimer == 0))
#endif
                    {
                        sLENS_INFO sLensInfo = {0};
                        sLENSMEMORY sLensMem = {0};
                        sMOTOR_LIMIT sFocus = {0};
                        sMOTOR_LIMIT sZoom = {0};
                        sMOTOR_LIMIT sLensBacklash = {0};
                        UINT32 ulLensIDAdc;
                        UINT32 ulStep[2];
                        UINT8  ucLensCal = 0xFF; //is idle
                        UINT8 aucVersion[2] = {0};

                        palMotor_ZoomLimit_Get(&sZoom);
                        palMotor_FocusLimit_Get(&sFocus);
                        palMotorLensPositionGet(&sLensMem);
                        palMotor_Lens_Backlash_Get(&sLensBacklash);
                        palMotor_LensIdADC_Get(&ulLensIDAdc);
                        palMotor_LensCenterGet((BYTE*)&ulStep);
                        palEnvironment_LENS_EEPROM_Version_Get(aucVersion); //HICC2_Julie_0055

                        sLensInfo.wLensHPosition = sLensMem.wLensHPosition;
                        sLensInfo.wLensVPosition = sLensMem.wLensVPosition;
                        sLensInfo.wZoomPosition = sLensMem.wZoomPosition;
                        sLensInfo.wFocusPosition = sLensMem.wFocusPosition;
                        sLensInfo.cDirH = sLensMem.cDirH;  //OSD (Left : 0,Down : 1), Motor Driver (Near Sensor : 1,Far Sensor : 0)
                        sLensInfo.cDirV = sLensMem.cDirV; //OSD (Up : 1,Down : 0), Motor Driver (Near Sensor : 1,Far Sensor : 0)
                        sLensInfo.cDirZoom  = sLensMem.cDirZoom;
                        sLensInfo.cDirFocus = sLensMem.cDirFocus;
                        sLensInfo.wZoomMaximumValue = sZoom.wMaximumValue;
                        sLensInfo.wZoomMinimumValue = sZoom.wMinimumValue;
                        sLensInfo.wFocusMaximumValue = sFocus.wMaximumValue;
                        sLensInfo.wFocusMinimumValue = sFocus.wMinimumValue;
                        sLensInfo.wBacklashH = sLensBacklash.wMaximumValue;
                        sLensInfo.wBacklashV = sLensBacklash.wMinimumValue;
                        sLensInfo.ulLensIDAdcValue = ulLensIDAdc;
                        sLensInfo.ulLensCenterH = ulStep[0];
                        sLensInfo.ulLensCenterV = ulStep[1];
                        sLensInfo.wLensEepromVersion = (aucVersion[0] << 8) | aucVersion[1]; //HICC2_Julie_0055

                        LOG_MSG(db_HAL_LENS, "Lens Pos H %d, V %d, dir H %d, V %d, Backlash H %d, V %d\n", sLensInfo.wLensHPosition, sLensInfo.wLensVPosition, sLensInfo.cDirH, sLensInfo.cDirV, sLensBacklash.wMaximumValue, sLensBacklash.wMinimumValue);
                        LOG_MSG(db_HAL_LENS, "Pos Zoom  %d, Focus %d, Zoom dir %d, Focus dir %d\n", sLensInfo.wZoomPosition, sLensInfo.wFocusPosition, sLensInfo.cDirZoom, sLensInfo.cDirFocus);
                        LOG_MSG(db_HAL_LENS, "Lens Zoom Max %d, Zoom Min %d, Focus Max %d, Focus Min %d\n", sZoom.wMaximumValue, sZoom.wMinimumValue, sFocus.wMaximumValue, sFocus.wMinimumValue);

                        palDataMgr_Data_Access(edcCUR_H_POSITION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensInfo.wLensHPosition); //HICC2_Casper_0030
                        palDataMgr_Data_Access(edcCUR_V_POSITION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensInfo.wLensVPosition); //HICC2_Casper_0030
                        palDataMgr_Data_Access(edcCUR_H_DIR, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensInfo.cDirH); //HICC2_Casper_0030
                        palDataMgr_Data_Access(edcCUR_V_DIR, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensInfo.cDirV); //HICC2_Casper_0030

                        palDataMgr_Data_Access(edcLENS_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &sLensInfo);  //A70LV_Larry_0287

                        #ifdef H30_DEVELOP_TEMP_DEFINE

                        palDataMgr_UI_EventSend(edcUI_EVENT_LENS_CAL_MSG_EXIT, TRUE, NULL);

                        #else
                        palDataMgr_Data_Access(edcLENS_CALIBRATION, edaWRITE_RAM_ONLY_NO_ACTION, &ucLensCal);
                        #endif
                    #if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
                        palMotor_LSM_CalibrationDone_Ask();
                        palMotor_LSM_Factory_Menu(); //HICC2_AC_0010
                    #endif
                        palEnvironment_LensCalFlag_Update();

                        if(m_ucMOT_Cal_Flag)
                        {
                            palDataMgr_OPD_LensCalibration();
                        }

                        if((Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||
                           (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60))    //HICC2_Doulas_0030
                        {
                            UINT8 ucID = palMotor_LensId_Get();

                            palDataMgr_Data_Access(edcLENS_TYPE, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&ucID);
                            //appGui_LensID_Checking();   //G100_Owen_0018
                        }

                        if(m_sPalEnvInfo.cLensCalibrationTimer == 0)
                        {
                            palDataMgr_UI_EventSend(edcUI_EVENT_LENS_CAL_TIME_OUT, TRUE, NULL);
                        }
                        else
                        {
                            palDataMgr_UI_EventSend(edcUI_EVENT_LENS_CAL_MSG_EXIT, TRUE, NULL);
                        }

                        m_sPalEnvInfo.cLensCalibrationTimer = 0;
                    }
                }
            }
        }
//A70LV_Larry_0082]]

        if((m_sPalEnvInfo.uiFailureList == eFAILURE_NUMBERS) && (m_sPalEnvInfo.iWarmupTimer > 15) && (palSystem_SystemCheckGet() == FALSE)) //error handler //A70LV_Larry_0023  //A70LV_Larry_0357
        {
            UINT8  ucShutDown = 0; //A70LV_Larry_0041

        #if defined (SCALER_FPGA_F34)
            if((m_sPalEnvInfo.ucSystemCount % 5) == 0)
            {
                UINT16 uiFPGA_Temp = palImgMgr_FpgaTempGet();

                LOG_MSG(db_APP_ENVIRONMENT, "uiFPGA_Temp %d.%dc \r\n", uiFPGA_Temp/100, uiFPGA_Temp%100);

                palDataMgr_Data_Access(edcTHERMAL_SENSOR_7, edaWRITE_RAM_ONLY_NO_ACTION, (INT16*)&uiFPGA_Temp); //A70LV_Larry_0062

                halMCUCtrl_System_TemperatureSet((UINT8*)&uiFPGA_Temp, 2);

                if(uiFPGA_Temp > 9000)
                {
                    m_sPalEnvInfo.ucFailureCount++;
                    palImgMgr_FpgaTempADCRst(); //double confirm FPGA HW //A70LK_Larry_0100
                }
                else
                {
                    m_sPalEnvInfo.ucFailureCount = 0;
                }

                if(m_sPalEnvInfo.ucFailureCount > 5)
                {
                    ucShutDown = 1;
                    LOG_MSG(db_ALWAYS, "FPGA over temperature 90C\n");
                }
            }
        #endif

            if((ucShutDown == 1)  && (m_sPalEnvInfo.ucMonitorFlag == 0) && (palSystem_NoFanControlFlagGet() == FALSE)) //A70LV_Larry_0031 //A70LV_Larry_0200
            {
                UINT8 ucBurninEnable = 0;
                char cString[128] = {'\0'};

                m_sPalEnvInfo.ucMonitorFlag = 1;

                palLedProc_LED_Behavior_Set(eLED_STATUS_FANLOCK_COOLING);

                palDataMgr_Data_Access(edcBURNIN_ENABLE, edaWRITE_THROUGH_WITH_ACTION, &ucBurninEnable);

                sprintf(cString, "FPGA over temperature 90C");
	//Cassper_ProAV
                //utilDataMgr_AddNewLog(cString);
				utilDataMgr_WriteErrorLog(0xFFFFFFFF, cString);
	//Cassper_ProAV
                palSystem_PowerDown();
            }
        }

        //A70LV_Doulas_0082 Add auto source search
        if((palIllumination_BurnIn_Enable() == 0) &&
           (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) &&
           (palDataPath_GetDataPathState() >= eDATA_PATH_STATE_SCALER_PORT_CONFIG) &&
           (palDataPath_GetDataPathState() <= eDATA_PATH_STATE_MONITOR_SOURCE) &&
           (m_sPalEnvInfo.iWarmupTimer > 15))
        {
            eResult = palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucInputKey);

            if((eResult == eEXEC_CODE_PASS) && (ucInputKey == eSOURCE_KEY_CHANGE_SOURCE_AUTO))
            {
                ePOWER_STATE m_ePowerState = palSystem_PowerStateGet();     //A70LV_Doulas_0233
                static BOOL bSourceSetting = FALSE;                         //A70LV_Doulas_0233
                UINT16 uiInput = palDataPath_LastInputDetectGet();
                UINT8 ucShift = 0;
                UINT8 ucMainSource = eCM_SOURCE_RESERVED;

                palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucMainSource);
                //LOG_MSG(db_ALWAYS, "Power Mode = %d,%d  \r\n",m_ePowerState,palIllumination_FirstLightSourceOnGet());  //A70LV_Doulas_0233
                if((palDataPath_IsSourceLock() == FALSE) &&
                   (m_ePowerState == ePOWER_STATE_ACTIVE) &&
                   #if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Tim_0026, add, start
                   (palDataMgr_ACU_Status_Get() != eACU_STATUS_PROCESSING ) &&
                   #endif //ENABLE_COLOR_UNIFORMITY == TRUE    //G100_Tim_0026, add, end
                   (palIllumination_FirstLightSourceOnGet() == TRUE) &&   //A70LV_Doulas_0233 modify //A70LV_Doulas_0201 modify
                   (palDataMgr_ResetCountGet() == 0)
                )
                {
                    m_sPalEnvInfo.ucAutoSourceSearchTimer++;
                    halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiInput); //HICC2_Doulas_0087
                    ucShift = ucMainSource;
                    //LOG_MSG(db_ALWAYS, "Auto Source (0x%X, 0x%X, %d) \r\n", uiInput, ucShift, ucMainSource);
                    if((uiInput >> ucShift) & 0x01)
                    {
                        UINT8 ucAutoResync = 0;
                        palDataMgr_Data_Access(edcAUTO_SOURCE_RESYNC, edaREAD, &ucAutoResync);

                        if(ucAutoResync == ets_ON)
                        {
                            m_sPalEnvInfo.ucAutoSourceWaitTimeToChange = AUTO_SOURCE_DET_SRC_GO_NEXT_TIME_ENHANCE;
                        }
                        else
                        {
                            m_sPalEnvInfo.ucAutoSourceWaitTimeToChange = AUTO_SOURCE_DET_SRC_GO_NEXT_TIME_NORMAL;
                        }
                    }
                    else
                    {
                        m_sPalEnvInfo.ucAutoSourceWaitTimeToChange = AUTO_SOURCE_NO_SRC_GO_NEXT_TIME;
                    }

                    if(m_sPalEnvInfo.ucAutoSourceSearchTimer >= m_sPalEnvInfo.ucAutoSourceWaitTimeToChange)
                    {
                        UINT8 ucPIP_EN;     //A70LV_Doulas_0272
                        palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &ucPIP_EN) ;     //A70LV_Doulas_0272

                        palEnvironment_AutoSourceSearchTimerClear();
                        if(ucPIP_EN == ets_OFF)        //A70LV_Doulas_0272
                        {
                            palDataMgr_MainInputSourceChangeToNext();
                        }
                    }
                }
                else
                {
                    if((m_ePowerState == ePOWER_STATE_COOLING) && (bSourceSetting == FALSE))      //A70LV_Doulas_0233 Save input source
                    {
                        UINT8 ucMainSource = eCM_SOURCE_HDMI1;
                        palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucMainSource);
                        palDataMgr_Data_Access(edcMAIN_INPUT, edaWRITE_THROUGH_NO_ACTION, &ucMainSource);
                        bSourceSetting = TRUE;
                    }
                    palEnvironment_AutoSourceSearchTimerClear();
                }
                //LOG_MSG(db_ALWAYS, "Auto Timer %d %d\r\n", m_sPalEnvInfo.ucAutoSourceWaitTimeToChange, m_sPalEnvInfo.ucAutoSourceSearchTimer);
            }
        }

        if(sGlobalCfg.sFN_CFG.SupportSourceList)  //HICC2_Doulas_0087
        {
            //appGui_Source_List_Checking();
            UINT32 InputKeySetttings;
            palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &InputKeySetttings);
            if(InputKeySetttings == eCM_INPUT_KEY_LIST_ALL_SOURCE)
            {
                UINT16 uiCurrentInput = 0;
                UINT16 uiInput = 0;
                halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(&uiCurrentInput);
                palDataMgr_Data_Access(edcSOURCE_INFO, edaREAD, &uiInput);
                if((uiInput != uiCurrentInput) && (eResult == rcSUCCESS))
                {
                    palDataMgr_Data_Access(edcSOURCE_INFO, edaWRITE_RAM_ONLY_NO_ACTION, &uiCurrentInput);
                    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                    palDataPath_LastInputDetectSet(uiCurrentInput);
                    palDataPath_InputDetectSet(uiCurrentInput);
                }
            }
        }

        if(bLightSourceOn && appSysIsAsicReadyGet())
        {
            palDataMgr_Data_Access(edcCEILING_MOUNT, edaREAD, (void*)&ucCeilingMount);  //A70LK_Doulas_0003
            //if(m_sPalIllumInfo.bAutoCeilingMountEnabled)
            if((ucCeilingMount == eCM_CEILING_MOUNT_AUTO) && (Board_SingleBoard_Get() == FALSE))  //A70LK_Doulas_0003
            {
                {
                    UINT8 ucCeilingMount_GSensor;
                    UINT8 ucValue;

                    palDataMgr_Data_Access(edcREAR_PROJECTION, edaREAD, (void*)&ucRear);

                    palLDMgr_CeilingMount_Get(eLDBANK_A70LV, &ucCeilingMount_GSensor);  //HICC2_Doulas_0104
                    #if defined(PLATFORM_H30_4K) && defined(OE_JIG)
					if(ucCeilingMount_GSensor)
						ucCeilingMount_GSensor = FALSE;
					else
						ucCeilingMount_GSensor = TRUE;
					#endif
                    //palDataMgr_Data_Access(edcG_SENSOR_MODE, edaREAD, &ucCeilingMount_GSensor); //HICC2_Doulas_0104

                    LOG_MSG(db_APP_ENVIRONMENT, "(%s, %d)Ceiling Mount %d %d\r\n", __FUNCTION__, __LINE__, m_sPalEnvInfo.ucCurrentCeilingMount, ucCeilingMount_GSensor);

                    if(m_sPalEnvInfo.ucCurrentCeilingMount != ucCeilingMount_GSensor)
                    {
                        m_sPalEnvInfo.ucCeilingMountCheck = 0;
                    }

                    if(m_sPalEnvInfo.ucCeilingMountCheck < 5)
                    {
                        UINT8 ucCurrent = 0;
                        UINT8 ucLensType;

                        m_sPalEnvInfo.ucCeilingMountCheck++;

                        palFormatterMgr_CeilingMount_RearProjectGet(&ucValue);
                        palDataMgr_Data_Access(edcLENS_TYPE, edaREAD, &ucLensType);

                        if(ucCeilingMount_GSensor)
                        {
                            if(ucRear)
                                ucCurrent = CEILINGWITHREAR;   //CelingWithRear
                            else
                                ucCurrent = CEILINGPROJECTOR;  //CelingProjector
                        }
                        else
                        {
                            if(ucRear)
                                ucCurrent = NORMALWITHREAR;    //NormalWithRear
                            else
                                ucCurrent = NORMALPROJECTOR;   //NormalProjector
                        }

                        #if 1 //add fuji lens B20U100 //HICC2_Julie_0071//H30K_Julie_0005
                        if((ucLensType == eLENS_ID_B20U100) || (ucLensType == eLENS_ID_NVRAM_B20U100))
                        {
                            ucValue ^= CEILINGWITHREAR;
                            //ucValue ^= CEILINGPROJECTOR; //CelingWithRear->NormalProjector,CelingProjector->NormalWithRear,NormalWithRear->CelingProjector,NormalProjector->CelingWithRear
                        }

                        if((ucCurrent != ucValue) || (m_sPalEnvInfo.ucLensChanged == TRUE) || (m_sPalEnvInfo.ucCurrentCeilingMount == 99))
                        {
                            BOOL bCeiling, bRear;
                            switch(ucCurrent)
                            {
                                case NORMALWITHREAR:
                                    bCeiling = FALSE;
                                    bRear = TRUE;
                                break;

                                case NORMALPROJECTOR:
                                    bCeiling = FALSE;
                                    bRear = FALSE;
                                break;

                                case CEILINGPROJECTOR:
                                    bCeiling = TRUE;
                                    bRear = FALSE;
                                break;

                                case CEILINGWITHREAR:
                                    bCeiling = TRUE;
                                    bRear = TRUE;
                                break;
                            }

                            palMotorEvent_CeilingSet(bCeiling);
                            palMotorEvent_RearSet(bRear);
                            halFormatter_CeilingMount_RearProjectSet(bCeiling, bRear);
                            palLANProcSendToLAN(edcORIENTATION_STATE);
                            m_sPalEnvInfo.ucLensChanged = FALSE;
                            m_sPalEnvInfo.ucCurrentCeilingMount = bCeiling;

                            //halScaler_4K3D_DualPipeOrientationSet(palFormatterMgr_FRC_Bypass_Flag_Get(),palMotorEvent_RearGet(),palMotorEvent_CeilingGet());    //A70LK_Doulas_0017
                            palGeo_ColorUniformityApplyByOrientation();
                        }
                        #else
                        if((ucCurrent != ucValue) || (m_sPalEnvInfo.ucCurrentCeilingMount == 99))
                        {
                            LOG_MSG(db_APP_ENVIRONMENT, "(%s, %d)Ceiling Mount(%d,%d)\r\n", __FUNCTION__, __LINE__, ucCurrent,ucValue);
                            if(ucCeilingMount_GSensor)
                            {
                                palMotorEvent_CeilingSet(TRUE);
                                palMotorEvent_RearSet(ucRear);
                                palFormatterMgr_CeilingMount_RearProjectSet(TRUE,(BOOL)ucRear);
                                palLANProcSendToLAN(edcORIENTATION_STATE);
                                m_sPalEnvInfo.ucCurrentCeilingMount = TRUE;
                            }
                            else
                            {
                                palMotorEvent_CeilingSet(FALSE);
                                palMotorEvent_RearSet(ucRear);
                                palFormatterMgr_CeilingMount_RearProjectSet(FALSE,(BOOL)ucRear);
                                palLANProcSendToLAN(edcORIENTATION_STATE);
                                m_sPalEnvInfo.ucCurrentCeilingMount = FALSE;
                            }
							palImgMgr_4K3D_DualPipeOrientationSet(palFormatterMgr_FRC_Bypass_Flag_Get(),palMotorEvent_RearGet(),palMotorEvent_CeilingGet()); 	//A70LK_Doulas_0017
                            palGeo_ColorUniformityApplyByOrientation();
                        }
                        #endif
                    }
                }
            }
        }

        #if 0
        if(palEnvironment_FEOPD_Cnt_Get() > 0) //G100_Steven_0059 start
        {
        	if((m_sPalEnvInfo.ucSystemCount % 10) == 0)
        	{
        		BYTE cCnt = 0;
        		cCnt = palEnvironment_FEOPD_Cnt_Get();
        		palDataMgr_OPDFE_Snapshot(eOPD_FrontEnd_LOG);
        		cCnt = cCnt-1;
        		palEnvironment_FEOPD_Cnt_Set(cCnt);
        	}
        } //G100_Steven_0059 end
		#endif
        #ifndef FRAME_SEQUENTIAL_3D_ULTRA //H30K_Doulas_0003
        palEnvironment_3D_SYNC_Monitor();
        #endif
    }

#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING               //G100_Tim_0046, add, start
    palEnvironment_Camera_Force_Update_Power_Status_Check();
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING             //G100_Tim_0046, add, end

#ifdef PROSERVICE_ALPHA_ENABLE     //A35G2_Coda_0101
    if(m_sPalEnvInfo.iProServicePowerDownTimer > 0)
    {
        LOG_MSG(db_APP_SCHEDULE, "iProServicePowerDownTimer = %d\r\n", m_sPalEnvInfo.iProServicePowerDownTimer);
        m_sPalEnvInfo.iProServicePowerDownTimer--;
        if(m_sPalEnvInfo.iProServicePowerDownTimer == 0)
        {
            ePROSERVICE_PAIR_STATUS iValue = ePROSERVICE_PAIR_STATUS_IDLE;
            palDataMgr_Data_Access(edcPROSERVICE_PAIR_STATUS, edaWRITE_RAM_ONLY_WITH_ACTION, &iValue);
            palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL);
            palDataMgr_UI_EventSend(edcUI_EVENT_PROSERVICE_PAIR, iValue, NULL);// HICC2_Bruce_0010
            palEnvironment_ProServicePowerDownMessageCountSet(0);
            palEnvironment_ProServicePowerDownTimerSet(0);
        }
    }

    if (m_sPalEnvInfo.iProServicePowerDownCount > 0)
    {
        LOG_MSG(db_APP_SCHEDULE, "iProServicePowerDownCount = %d\r\n", m_sPalEnvInfo.iProServicePowerDownCount);
        m_sPalEnvInfo.iProServicePowerDownCount--;
        palEnvironment_ProServicePowerDownMessageCountSet(m_sPalEnvInfo.iProServicePowerDownCount);
    }
#endif //PROSERVICE_ALPHA_ENABLE

    if(bLightSourceOn && m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReady && palSystem_PowerStateGet() != ePOWER_STATE_UPGRADE) //Polling source state
    {
        UINT8 ucMainSource = 0;
        UINT8 ucSubSource = 0;

        palDataMgr_Data_Access((eDATA_CODE)edcSHOW_MAINSOURCE_MESSAGES, edaREAD, &ucMainSource);
        palDataMgr_Data_Access((eDATA_CODE)edcSHOW_SUBSOURCE_MESSAGES, edaREAD, &ucSubSource);

        if((m_sPalEnvInfo.ucMainSource != ucMainSource) && ((m_sPalEnvInfo.ucMainSource == eSOURCE_STATE_SHOW_SOURCE_RESOLUTION) || (ucMainSource == eSOURCE_STATE_SHOW_SOURCE_RESOLUTION)))
        {
            //char cString[127] = {0};
            UINT16 uiDataCode = 0;

            m_sPalEnvInfo.ucMainSource = ucMainSource;

            uiDataCode = edcMAIN_INPUT;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_ASPECT_RATIO;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_SIGNAL_FORMAT;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_RESOLUTION;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_PIXEL_CLOCK;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_SYNC_TYPE;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_HORZ_REFRESH;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_VERT_REFRESH;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcMAIN_COLOR_SPACE;
            palLANProcSendToLAN(uiDataCode);

            palDataMgr_3DModeConditionUpdate();
        }

        if((m_sPalEnvInfo.ucSubSource != ucSubSource) && ((m_sPalEnvInfo.ucSubSource == eSOURCE_STATE_SHOW_SOURCE_RESOLUTION) || (ucSubSource == eSOURCE_STATE_SHOW_SOURCE_RESOLUTION)))
        {
            //char cString[127] = {0};
            UINT16 uiDataCode = 0;

            m_sPalEnvInfo.ucSubSource = ucSubSource;

            uiDataCode = edcSUB_INPUT;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_ASPECT_RATIO;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_SIGNAL_FORMAT;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_RESOLUTION;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_PIXEL_CLOCK;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_SYNC_TYPE;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_HORZ_REFRESH;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_VERT_REFRESH;
            palLANProcSendToLAN(uiDataCode);

            uiDataCode = edcSUB_COLOR_SPACE;
            palLANProcSendToLAN(uiDataCode);

            palDataMgr_3DModeConditionUpdate();
        }
		appEnvironment_NotificationSystemUpdate();
    }

    return ENVIRONMENT_PERIOD / POLL_PERIOD;
}

eEXEC_CODE palEnvironment_Init(const PsSYSTEM_CONFIGURATION psSysConfiguration)
{
    UINT8 ucUartSw = 0;
    UINT8 ucPowerMode = 0;

    LOG_MSG(db_APP_ENVIRONMENT, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    if(pthread_mutex_init(&m_sPalEnvInfo.xMutex, NULL) != 0)
    {
        LOG_MSG(db_APP_ENVIRONMENT, "%s pthread_mutex_init fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eEXEC_CODE_FAIL;
    }

#else   //freertos

#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    m_sPalEnvInfo.xSemaphore = xSemaphoreCreateMutexStatic(&m_sPalEnvInfo.xMutexBuffer);
#else
    m_sPalEnvInfo.xSemaphore = xSemaphoreCreateMutex();
#endif

    if(m_sPalEnvInfo.xSemaphore == NULL)
    {
        return eEXEC_CODE_FAIL;
    }

#endif

    //palEnvironment_FanInit();

    //palEnvironment_ThermalSensorInit();


    m_sPalEnvInfo.psSysConfiguration = psSysConfiguration;

    /****************************************************/
    /* Initialize cooldown timer.                       */
    /****************************************************/

    m_sPalEnvInfo.bEnableEnvironmentPoll = (BOOL)psSysConfiguration->Environment.ucEnableEnvironmentPoll;
    m_sPalEnvInfo.bCheckFanLock = (BOOL)psSysConfiguration->Environment.ucCheckFanLock;
    m_sPalEnvInfo.bCheckOverTemp = (BOOL)psSysConfiguration->Environment.ucCheckOvertemp;

    m_sPalEnvInfo.iCoolTimer = 0;
    m_sPalEnvInfo.ucCeilingMountCheck = 0;
    m_sPalEnvInfo.ucCurrentCeilingMount = 99;

    m_sPalEnvInfo.uiAutoShutDown = 0; //A70LV_Larry_0082
    m_sPalEnvInfo.uiSleepTimer = 0;   //A70LV_Larry_0082
    m_sPalEnvInfo.ucSystemCount = 0;  //A70LV_Larry_0082
    m_sPalEnvInfo.ulSystemTimer = 0;  //A70LV_Larry_0082
    m_sPalEnvInfo.ucAutoSourceSearchTimer = 0;   //A70LV_Doulas_0082
    m_sPalEnvInfo.iWarmupTimer = 0; //A70LV_Larry_0135
    m_sPalEnvInfo.ucLensID = 99; //A70LV_Larry_0188
    m_sPalEnvInfo.ucLost_12V = 0; //A70LV_Larry_0276
    m_sPalEnvInfo.cNetworkWaitMessage = 0; //A70LV_Larry_0278
    m_sPalEnvInfo.ucMainSource = 99;
    m_sPalEnvInfo.ucSubSource = 99;
    m_sPalEnvInfo.cNetworkReadyCount = 180; //A70LV_Larry_0411
    m_sPalEnvInfo.uiFast_Power_On_Timer = 0;	//G100_Clare_0055
    m_sPalEnvInfo.bFake_Power_Down = 0;

#ifndef OE_JIG //A70LV_John_0038 fix OE JIG bug
    m_sFailure.ucGroup = 0;
    m_sFailure.uiFailure = 0;
    m_sFailure.ucLD_OverTemperature[0] = 0xFF;
    m_sFailure.ucLD_OverTemperature[1] = 0xFF;
    m_sFailure.ucLD_OverTemperature[2] = 0xFF;
	m_sFailure.ucLD_OverTempCount = 0;
#endif
    m_sPalEnvInfo.uiFailureList = eFAILURE_NUMBERS;  //A70LV_Larry_0357
    m_sPalEnvInfo.ucFailureCount = 0;  //A70LV_Larry_0357
    m_sPalEnvInfo.iWaitWarmupTimer = 30;

    m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReady              = FALSE;
    m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReceivePowerOnCmd  = FALSE;
    m_sPalEnvInfo.bSourceSetting = TRUE;
    m_sPalEnvInfo.cLensCalibrationTimer = 0;

    //palMotorLens_Init(); //HICC2_Doulas_0147//HICC2_Casper_0034

    GuiCb = Gui_fpCallbackGet();

    return eEXEC_CODE_PASS;
}

//A70LV_Larry_0135 satrt
void palEnvironment_NetworkState(UINT8 ucData)
{
    m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReady = ucData ;

    LOG_MSG(db_APP_LAN, "\r\nLAN ready\r\n");
}

UINT8 palEnvironment_NetworkReady(void) //A70LV_Larry_0294
{
    return m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReady;
}

void palEnvironment_NetworkIsReceivePowerOnCmd_Set(UINT8 ucData)
{
    m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReceivePowerOnCmd = ucData ;

    LOG_MSG(db_APP_LAN, "\r\nLAN Receive Power On Cmd\r\n");
}

UINT8 palEnvironment_NetworkIsReceivePowerOnCmd_Get(void)
{
    return m_sPalEnvInfo.psSysConfiguration->Network.ucNetworkIsReceivePowerOnCmd ;
}
//A70LV_Larry_0135 end
//ZU860_Clare_0022, >>>
void palEnvironment_SecurityPowerDownTimerSet(UINT32 ulValue, BOOL UpdateToDatabase)
{
    m_sPalEnvInfo.iSecurityPowerDownTimer = ulValue;

    if(UpdateToDatabase)
    {
        palLANProcSendToLAN(edcSECURITY_POWER_DOWN_TIMER);
    }
}

UINT32 palEnvironment_SecurityPowerDownTimerGet(void)
{
    return m_sPalEnvInfo.iSecurityPowerDownTimer;
}
//ZU860_Clare_0022, <<<

UINT32 palEnvironment_AutoShutDownTimerGet(void)	//ZU860_Clare_0143, add
{
    return m_sPalEnvInfo.uiAutoShutDown;
}

void palEnvironment_AutoShutDownTimerSet(UINT32 Value, BOOL UpdateToDatabase)
{
    m_sPalEnvInfo.uiAutoShutDown = Value;

    if(UpdateToDatabase)
    {
        palLANProcSendToLAN(edcAUTO_SHUTDOWN_TIMER_COUNT);
    }
}

void palEnvironment_AutoShutDownDefaultSet(UINT32 Value)
{
    m_sPalEnvInfo.uiAutoShutDownDefault = Value;
}

// ==============================================================================
// FUNCTION NAME: palEnvironment_AutoShutDownClear
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
// 2018/01/16, Larry Create
// --------------------
// ==============================================================================
void palEnvironment_AutoShutDownClear(void)
{
    UINT8 ucTimer = 0;

    //palDataMgr_Data_Access(edcAUTO_SHUTDOWN, edaREAD,(void*)&ucTimer);
    //palDataMgr_Data_Access(edcAUTO_SHUTDOWN, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucTimer);
    palEnvironment_AutoShutDownTimerSet(m_sPalEnvInfo.uiAutoShutDownDefault, TRUE);

    #if 0

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A65_Owen_0006
    if(ucTimer > AUTO_SHUTDOWN_TIMER_MAX_VALUE)
        ucTimer = AUTO_SHUTDOWN_TIMER_MAX_VALUE;

    m_sPalEnvInfo.uiAutoShutDown = 60 * ucTimer; //Sec

    palEnvironment_AutoShutDownTimerSet(m_sPalEnvInfo.uiAutoShutDown, TRUE);

    if(GuiCb.fpGui_IsAutoPowerDown_MenuCb() == TRUE)	//A35G2_BRC_Coda_0015
    {
        #if defined(CUSTOM_OPTOMA)
            if((palEnvironment_SleepTimerGet() > 60 || palEnvironment_SleepTimerGet() == 0)
                && (palEnvironment_SecurityPowerDownTimerGet() > 10 || palEnvironment_SecurityPowerDownTimerGet() == 0))    //A35G2_Coda_0072 //A35G2_Coda_0075
            {
                GuiCb.fpGui_SendAutoPowerOffMenuExitCb();	//A35G2_Coda_0044
            }
        #else
            //GuiCb.fpGui_Send_OSD_ExitCb();
            palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL);
        #endif
    }
#else
    switch(ucTimer)
    {
        case eAUTO_OFF_TIME_5MIN:
            m_sPalEnvInfo.uiAutoShutDown = 60 * 5; //Sec
            break;

        case eAUTO_OFF_TIME_10MIN:
            m_sPalEnvInfo.uiAutoShutDown = 60 * 10; //Sec
            break;

        case eAUTO_OFF_TIME_15MIN:
            m_sPalEnvInfo.uiAutoShutDown = 60 * 15; //Sec
            break;

        case eAUTO_OFF_TIME_20MIN:
            m_sPalEnvInfo.uiAutoShutDown = 60 * 20; //Sec
            break;

        case eAUTO_OFF_TIME_25MIN:
            m_sPalEnvInfo.uiAutoShutDown = 60 * 25; //Sec
            break;

        case eAUTO_OFF_TIME_30MIN:
            m_sPalEnvInfo.uiAutoShutDown = 60 * 30; //Sec
            break;

        case eAUTO_OFF_TIME_NEVER:
        default:
            m_sPalEnvInfo.uiAutoShutDown = 0;
            break;
    }

    if(GuiCb.fpGui_IsAutoPowerDown_MenuCb() == TRUE)	//A35G2_CDS_Coda_0035
    {
        GuiCb.fpGui_SendAutoPowerOffMenuExitCb();
    }
#endif

    #endif
}

// ==============================================================================
// FUNCTION NAME: palEnvironment_SleepTimerGet
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/15, Larry Create
// --------------------
// ==============================================================================
UINT32 palEnvironment_SleepTimerGet(void)
{
    return m_sPalEnvInfo.uiSleepTimer;
}

// ==============================================================================
// FUNCTION NAME: palEnvironment_SleepTimerSet
// DESCRIPTION:
//
//
// Params:
// UINT8 ucTimer:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/15, Larry Create
// --------------------
// ==============================================================================
void palEnvironment_SleepTimerSet(UINT8 ucSleepTimerStep, BOOL UpdateToDatabase) //A70LV_Larry_0130
{
	//LOG_MSG(db_ALWAYS, "SleepTimerSet %d\r\n",ucTimer);

    if(CFG_CUSTOMER_ID == CUSTOMER_ID_OPTOMA || CFG_CUSTOMER_ID == CUSTOMER_ID_BARCO)
    {
        m_sPalEnvInfo.uiSleepTimer = 1800 * (UINT32)ucSleepTimerStep;
    }
    else
    {
        m_sPalEnvInfo.uiSleepTimer = 7200 * (UINT32)ucSleepTimerStep;
    }

    if(UpdateToDatabase)
    {
        palLANProcSendToLAN(edcSLEEP_TIMER_COUNT);
    }

	//LOG_MSG(db_ALWAYS, "SleepTimer %d\r\n",m_sPalEnvInfo.uiSleepTimer);
}

// ==============================================================================
// FUNCTION NAME: palEnvironment_SleepTimerClear
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
// 2018/01/16, Larry Create
// --------------------
// ==============================================================================
void palEnvironment_SleepTimerClear(void)
{
    UINT8 ucTimer = 0;

    palDataMgr_Data_Access(edcSLEEP_TIMER, edaREAD, (void*)&ucTimer);
    palEnvironment_SleepTimerSet(ucTimer, TRUE);
}


//G100_Clare_0055, add, >>>
void palEnvironment_Fast_Power_On_TimerSet(UINT8 ucTimer)
{
	//LOG_MSG(db_ALWAYS, "SleepTimerSet %d\r\n",ucTimer);
   	m_sPalEnvInfo.uiFast_Power_On_Timer = 5400*ucTimer;
    //LOG_MSG(db_ALWAYS, "SleepTimer %d\r\n",m_sPalEnvInfo.uiSleepTimer);
}

void palEnvironment_Fast_Power_On_TimerClear(void)
{
    UINT8 ucTimer = 0;

    palDataMgr_Data_Access(edcFAST_POWER_ON, edaREAD,(void*)&ucTimer);

    //LOG_MSG(db_ALWAYS, "Fast_Power_On_TimerClear %d\r\n",ucTimer);
    palEnvironment_Fast_Power_On_TimerSet(ucTimer);
}
//G100_Clare_0055, add, <<<
// ==============================================================================
// FUNCTION NAME: palEnvironment_AutoSourceSearchTimerClear
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
// 2018/01/24, Doulas Create    //A70LV_Doulas_0082
// --------------------
// ==============================================================================
void palEnvironment_AutoSourceSearchTimerClear(void)
{
    m_sPalEnvInfo.ucAutoSourceSearchTimer = 0;
}


// ==============================================================================
// FUNCTION NAME: palEnvironment_NetworkWaitMessage
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/07/24, Larry Create
// --------------------
// ==============================================================================
void palEnvironment_NetworkWaitMessage(UINT8 ucTimer)
{
    m_sPalEnvInfo.cNetworkWaitMessage = ucTimer;
}

void palEnvironment_NetworkStatusMessage(UINT8 ucTimer)//HICC2_Julie_0064
{
    m_sPalEnvInfo.cNetworkStatusMessageCount = ucTimer;
}

void palEnvironment_LenCalibrationTimeOutSet(void)
{
    if(Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70)
    {
        m_sPalEnvInfo.cLensCalibrationTimer = 360;//need to define.
    }
    else if(Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A35)
    {
        m_sPalEnvInfo.cLensCalibrationTimer = 200;//=LENS_CALIBRATION_TIMEOUT
    }
    else if(Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60) //HICC2_Doulas_0030
    {
        m_sPalEnvInfo.cLensCalibrationTimer = 900;//need to define. //HICC2_AC_0020
    }
}

#if 0
void palEnvironment_ADC_SetupCalImageArea(void)         //A70LV_Doulas_0124
{
    UINT8                cCalStep;
    UINT8                cImageAreas;
    eCAL_SAMPLE_TYPE    eSampleType = cstINVALID;
    UINT16 uiH_Active;
    UINT16 uiV_Active;

    // Get the current calibration step
    cCalStep = m_acCalStep;
    ASSERT(cCalStep < calINVALID);
    ASSERT(cCalStep != calRGB_STORE);
    ASSERT(cCalStep != calYUV_STORE);

    // Use one image area for offset calibrations. Otherwise, use the delta
    // between two image areas (for gain calibrations).
    switch(cCalStep)
    {
        case calRGB_OFFSET:
        case calRGB_OFFSET2:
        case calYUV_OFFSET:
        case calYUV_OFFSET2:
            cImageAreas = 1;
            break;

        default:
            cImageAreas = 2;
            break;
    }

    while(cImageAreas > 0)
    {
        // Decrement area count (this is the index into the image area array)
        cImageAreas--;

        // Determine the sample type for this calibration step
        switch(cCalStep)
        {
            case calRGB_OFFSET:
            case calRGB_OFFSET2:
                eSampleType = cstRGB_BAR2;
                break;

            case calRGB_GAIN:
                eSampleType = (0 == cImageAreas) ? cstRGB_BAR2
                              : cstRGB_BAR15;
                break;

            case calYUV_OFFSET:
            case calYUV_OFFSET2:
                eSampleType = cstYUV_BLACK;
                break;

            case calY_GAIN:
                eSampleType = (0 == cImageAreas) ? cstYUV_100IRE_WHITE
                              : cstYUV_BLACK;
                break;

            case calU_GAIN:
                eSampleType = (0 == cImageAreas) ? cstYUV_75IRE_BLUE
                              : cstYUV_75IRE_YELLOW;
                break;

            case calV_GAIN:
                eSampleType = (0 == cImageAreas) ? cstYUV_75IRE_RED
                              : cstYUV_75IRE_CYAN;
                break;

            case calINVALID:
                ASSERT_ALWAYS();
                eSampleType = cstRGB_BLACK;
                break;
        }

        // The point locations are specified in tenths of percent, for instance
        // 493 = 49.3% of the image dimension.  Calculate the window position
        // of the sample window using the same areas upper left corner and the
        // width and height specified by the constants.
	//Cassper_ProAV
        uiH_Active = palImgMgr_Input_H_Active_Get(eSOURCE_WINDOW_MAIN);
        uiV_Active = palImgMgr_Input_V_Active_Get(eSOURCE_WINDOW_MAIN);
	//Cassper_ProAV
        m_asImageArea[cImageAreas].wLeft   = (UINT16)(((UINT32)m_sULSampleArea[0][eSampleType].wX * (UINT32)uiH_Active) / 1000L);
        m_asImageArea[cImageAreas].wRight  = (UINT16)(m_asImageArea[cImageAreas].wLeft + SAMPLE_AREA_WIDTH);
        m_asImageArea[cImageAreas].wTop    = (UINT16)(((UINT32)m_sULSampleArea[0][eSampleType].wY * (UINT32)uiV_Active) / 1000L);
        m_asImageArea[cImageAreas].wBottom = (UINT16)(m_asImageArea[cImageAreas].wTop + SAMPLE_AREA_HEIGHT);
    }
}

static eNBRESULT palEnvironment_ADC_CalibrateStart(void)
{
    eNBRESULT           eResult = nbERROR;
    UINT8               cStep;

    if( halADCCalibrationCheckFlagGet() == FALSE )  //no input source
    {
        m_aeStatus = nbERROR;
        return nbERROR;
    }


    // Quit if the calibration retry counter has reached zero
    if(m_acRecalibration == 0)
    {
        m_aeStatus = nbERROR;
        return nbERROR;
    }

    // Decrement the calibration retry counter
    m_acRecalibration--;

    // Set counter to first step of calibration (YUV or RGB)
    if(palImgMgr_VGA_SYNC_TYPE_Get(eSOURCE_WINDOW_MAIN) == eVGA_SYNC_TYPE_SEPARATE)
    {
        cStep = CAL_RGB_START;
    }
    else
    {
        cStep = CAL_YUV_START;
    }

    // cStep= CAL_YUV_START;//james force for testing @20111108
    if(adcIDLE == m_aeState)
    {

        m_acCalStep = cStep;

        palEnvironment_ADC_SetupCalImageArea();
        eResult = halADCCtrlImageLevelMeasureStart(cStep,
                                                   m_asImageArea,
                                                   &m_asCalData);

        if(nbPENDING == eResult)
        {
            m_aeState = adcCALIBRATE;
        }
    }

    m_aeStatus = eResult;
    return eResult;
}

static void palEnvironment_ADC_StateChange(eAPP_ADC_STATE eState)
{
    eNBRESULT cResult;
    switch(m_AppADCState)
    {
        case eAPP_ADC_STATE_IDLE:
            if(eState == eAPP_ADC_STATE_CALIBRATION)
            {
                cResult = palEnvironment_ADC_CalibrateStart();

                if(cResult == nbERROR)
                {

                    palEnvironment_ADCCalStatusSet(cResult);
                    GuiCb.fpGui_SendUpdateOSDEventCb() ;   //A70LV_Doulas_0171 Modify //ADC fail
                    return ;
                }
                palImgMgr_InputADC_Cali_CalibrationEnableSet(eSOURCE_WINDOW_MAIN,TRUE);     //init input color config for ADC calibration setting

                palEnvironment_ADCCalStatusSet(cResult);
                GuiCb.fpGui_SendUpdateOSDEventCb() ;   //A70LV_Doulas_0171 Modify   OSD update,ADC doing
            }
            break;

        case eAPP_ADC_STATE_CALIBRATION:

            break;

        default:
            ASSERT_ALWAYS();
    }

    /* ---------------------------- END CUSTOM AREA --------------------------- */
    m_AppADCState = eState;
}

void palEnvironment_ADC_CalibrationEnableSet(void)
{
    m_acRecalibration = MAX_RECALIBRATIONS;
    ucADC_Poll_Count = 0;
    halADCCtrlResetCal_YUV();
    palEnvironment_ADC_StateChange(eAPP_ADC_STATE_CALIBRATION);
}

void palEnvironment_ADC_CalibrationStop(void)
{
    //m_cCalibrationInstance = CALIBRATION_INVALID;
    palEnvironment_ADC_StateChange(eAPP_ADC_STATE_IDLE);
    halADCCtrlOperationCancel();
    palImgMgr_InputADC_Cali_CalibrationEnableSet(eSOURCE_WINDOW_MAIN,FALSE);
    LOG_MSG(db_ADC_CALIBARATION, "palEnvironment_ADC_CalibrationStop \r\n");
}

BOOL palEnvironment_ADC_CalibrationEnableGet(void)
{
    if(m_AppADCState == eAPP_ADC_STATE_CALIBRATION)
        return TRUE;
    else
        return FALSE;
}

void palEnvironment_ADC_StateProcess(void)
{
    eNBRESULT           eResult;
    eEXEC_CODE          eResultEEP = eEXEC_CODE_FAIL;

    switch(m_AppADCState)
    {
        case eAPP_ADC_STATE_IDLE:
            m_aeState = adcIDLE;
            //LOG_MSG(db_ADC_CALIBARATION, "eAPP_ADC_STATE_IDLE \r\n");
            break;

        case eAPP_ADC_STATE_CALIBRATION:
            if(m_aeState != adcIDLE)
            {
                eResult = halADCCtrlOperationStatusGet();
                if(nbSUCCESS == eResult)
                {
                    LOG_MSG(db_ADC_CALIBARATION, "[CalStep:%d] ([ADC]%d,%d,%d)\r\n", m_acCalStep,m_asCalData.awGain[0],m_asCalData.awGain[1],m_asCalData.awGain[2]);
                    // go to the next calibration step
                    m_acCalStep += 1;

                    if((calRGB_STORE == m_acCalStep) ||
                       (calYUV_STORE == m_acCalStep))
                    {
                        // calibration complete, save values
                        if(m_acCalStep == calRGB_STORE)
                            eResultEEP = palDataMgr_Data_Access(edcADC_RGB_GAIN, edaWRITE_THROUGH_NO_ACTION, m_asCalData.awGain);
                        else
                            eResultEEP = palDataMgr_Data_Access(edcADC_YUV_GAIN, edaWRITE_THROUGH_NO_ACTION, m_asCalData.awGain);

                        if(eResultEEP == eEXEC_CODE_PASS )    //save cal to EEPROM ok
                        {
                            eResult = nbSUCCESS;
                            palEnvironment_ADC_CalibrationStop() ;
                        }
                        else
                        {
                            eResult = nbERROR;
                        }
                        LOG_MSG(db_ADC_CALIBARATION, "ADC_Calibration OK %d \r\n",eResult);
                        palEnvironment_ADCCalStatusSet(eResult);// 設定ADC結果
                        GuiCb.fpGui_SendUpdateOSDEventCb() ;   //A70LV_Doulas_0171 Modify //OSD 讀取結果顯示
                    }
                    else
                    {
                        // continue with next calibration step
                        palEnvironment_ADC_SetupCalImageArea();
                        eResult = halADCCtrlImageLevelMeasureStart
                                  (m_acCalStep,
                                   m_asImageArea,
                                   &m_asCalData);
                    }
                }
                else if(nbERROR == eResult)
                {
                    m_aeState = adcIDLE;
                    LOG_MSG(db_ADC_CALIBARATION, "halADCCtrlOperationStatusGet ERROR %d \r\n", m_acCalStep);
                    eResult = palEnvironment_ADC_CalibrateStart();
                    if(eResult == nbERROR)
                    {
                        LOG_MSG(db_ADC_CALIBARATION, "palEnvironment_ADC_CalibrateStart ERROR %d \r\n", m_acCalStep);
                        palEnvironment_ADCCalStatusSet(eResult);    // 設定ADC結果
                        GuiCb.fpGui_SendUpdateOSDEventCb() ;   //A70LV_Doulas_0171 Modify //OSD 讀取結果顯示
                    }
                }

                // update the ADC operation status
                switch(eResult)
                {
                    case nbSUCCESS:
                        m_aeState = adcIDLE;
                        m_aeStatus = nbSUCCESS;
                    break;

                    case nbERROR:
                        m_aeState = adcIDLE;
                        m_aeStatus = nbERROR;
                        palEnvironment_ADC_CalibrationStop() ;
                    break;

                    case nbPENDING:
                        m_aeStatus = nbPENDING;
                    break;

                    default:
                        ASSERT_ALWAYS();
                    break;
                }
            }

            break;

        default:
            ASSERT_ALWAYS();
    }

}

eNBRESULT palEnvironment_ADC_CalibrationStatusGet(void)
{
    return m_aeStatus;
}

INT16 palEnvironment_ADC_Poll(UINT16 uiTick)
{

    if(ucADC_Poll_Count)        //ADC Calibration first on do nothing
    {
        halADCCtrlEventHandler();
        palEnvironment_ADC_StateProcess();
    }
    ucADC_Poll_Count++;
    //LOG_MSG(db_ADC_CALIBARATION, "[%03d]",ucADC_Poll_Count);

    return ENVIRONMENT_ADC_PERIOD / POLL_PERIOD;
}

void palEnvironment_ADCCalStatusSet(eNBRESULT state)
{
    cCalState = state;
    LOG_MSG(db_ADC_CALIBARATION, "palEnvironment_ADCCalStatusSet %d \r\n", state);
}

eNBRESULT palEnvironment_ADCCalStatusget(void)
{
    return cCalState;
}
#endif

eEXEC_CODE palEnvironment_XillinxFPGA_Reset(void)        //A70LV_Doulas_0179
{
    //halBoard_XillinxFPGA_Reset();
    return eEXEC_CODE_PASS;
}

void palEnvironment_3D_SYNC_Monitor(void)   //A70LV_Doulas_0191
{
#ifndef FRAME_SEQUENTIAL_3D_ULTRA //H30K_Doulas_0003
        BOOL b3DSyncAvaliable = FALSE;
        BOOL b3DSyncLost = FALSE;
        UINT8 ucdatapath_state = 0;
        UINT8 uc3D_LR_REFERENCE = 0;
        eRESULT eResult = rcINVALID;
        UINT8 uc3D_Format = 0;
        UINT8 uc3D_Sync_In = 0;


        //#ifdef FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC    //G100_Doulas_0071 Modify
        if(palDataPath_IsSourceMonitor() == TRUE)
        {
            //palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, (void*)&uc3D_Format);
            palDataMgr_Data_Access(edc3D_LR_REFERENCE, edaREAD, (void*)&uc3D_LR_REFERENCE);
            palDataMgr_Data_Access(edc3D_SYNC_TYPE, edaREAD, (void*)&uc3D_Sync_In);


            #if 1 //H30K_Doulas_0034 Modify
            UINT8 uc3D_enable = eCM_3D_FORMAT_OFF;
            UINT8 uc3D_mode = eCM_3D_MODE_OFF;
            palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, (void*)&uc3D_enable);
            palDataMgr_Data_Access(edc3D_MODE, edaREAD, (void*)&uc3D_mode);

            if((uc3D_enable == eCM_3D_FORMAT_FRAME_SEQUENTIAL) &&
                (uc3D_mode != eCM_3D_MODE_OFF))
            {
                uc3D_Format = eINPUT_3D_TYPE_FRAME_SEQUENTIAL;
            }
            #else
            uc3D_Format = halScaler_Input_3D_Format_Get();

            if(uc3D_Format == eINPUT_3D_TYPE_OFF) //A35G2_CDS_Larry_0052
            {
                //check input is 60hz, supprot 3D frame seq.
                UINT8  uc3DTiming = palDataPath_Input3D_Timing_Get();

                if((uc3DTiming == e3DFS_WUXGA_60) || (uc3DTiming == e3DFS_1080P_60))
                {
                   uc3D_Format = eINPUT_3D_TYPE_FRAME_SEQUENTIAL;
                }
            }
            #endif

            if((palDataMgr_IS_3D_Enable() == ets_ON) &&
                (uc3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                (palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Get() == e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC)&&
                (uc3D_Sync_In == eCM_3D_SYNC_TYPE_AUTO) )
            {
                eResult = halFormatter_3D_SYNC_InputMonitorGet(&b3DSyncAvaliable,&b3DSyncLost,&ucdatapath_state);
                uc3D_SYNC_Avaliable_Count = 0;

                LOG_MSG(db_APP_ENVIRONMENT, "DDP1(%d,%d,%d)(%d,%d) \r\n",b3DSyncAvaliable,b3DSyncLost,ucdatapath_state,eResult,uc3D_SYNC_Avaliable_Count);
                if((eResult == rcSUCCESS) &&
                   (ucdatapath_state == DPP_MONITOR_SOURCE) &&
                   (b3DSyncLost == TRUE) &&
                   (b3DSyncAvaliable == FALSE))
                {
                    uc3D_SYNC_Unavaliable_Count ++;
                    if(uc3D_SYNC_Unavaliable_Count > DDP_3D_SYNC_CHECK_TIMER)
                    {
                        halMCU_3D_Sync_In_Select_Set(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                        halFormatter_3D_SYNC_SelectionSet(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                        palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Set(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                        LOG_MSG(db_APP_ENVIRONMENT, "DDP1 sync in C821\r\n");
                    }
                }
                else
                {
                    uc3D_SYNC_Unavaliable_Count = 0;
                }
            }
            else if((palDataMgr_IS_3D_Enable() == ets_ON) &&
                    (uc3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                    (palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Get() == e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC)&&
                    (uc3D_Sync_In == eCM_3D_SYNC_TYPE_AUTO))
            {
                eResult = halFormatter_3D_SYNC_InputMonitorGet(&b3DSyncAvaliable,&b3DSyncLost,&ucdatapath_state);
                uc3D_SYNC_Unavaliable_Count = 0;
                LOG_MSG(db_APP_ENVIRONMENT, "DDP2(%d,%d,%d)(%d,%d) \r\n",b3DSyncAvaliable,b3DSyncLost,ucdatapath_state,eResult,uc3D_SYNC_Avaliable_Count);
                if((eResult == rcSUCCESS) &&
                   (ucdatapath_state == DPP_MONITOR_SOURCE) &&
                   (b3DSyncLost == FALSE) &&
                   (b3DSyncAvaliable == TRUE))
                {
                    uc3D_SYNC_Avaliable_Count ++;
                    if(uc3D_SYNC_Avaliable_Count > DDP_3D_SYNC_CHECK_TIMER)
                    {
                        halMCU_3D_Sync_In_Select_Set(e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC);
                        halFormatter_3D_SYNC_SelectionSet(e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC);
                        palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Set(e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC);
                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                        LOG_MSG(db_APP_ENVIRONMENT, "DDP2 sync in External\r\n");
                    }
                }
                else
                {
                    uc3D_SYNC_Avaliable_Count = 0;
                }
            }
            else if((palDataMgr_IS_3D_Enable() == ets_ON) &&
                    (uc3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                    (palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Get() == e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC)&&
                    (uc3D_Sync_In == eCM_3D_SYNC_TYPE_INTERNAL))
            {
                if(palDataPath_IsSourceLock() == TRUE)
                {
                    uc3D_SYNC_Unavaliable_Count ++;
                    if(uc3D_SYNC_Unavaliable_Count > DDP_3D_SYNC_CHECK_TIMER)
                    {
                        halMCU_3D_Sync_In_Select_Set(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                        halFormatter_3D_SYNC_SelectionSet(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                        palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Set(e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC);
                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                    }
                }
                else
                {
                    uc3D_SYNC_Avaliable_Count = 0;
                }
            }
            else if((palDataMgr_IS_3D_Enable() == ets_ON) &&
                    (uc3D_Format == eINPUT_3D_TYPE_FRAME_SEQUENTIAL) &&
                    (palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Get() == e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC)&&
                    (uc3D_Sync_In == eCM_3D_SYNC_TYPE_EXTERNAL))
            {
                if(palDataPath_IsSourceLock() == TRUE)
                {
                    uc3D_SYNC_Avaliable_Count ++;
                    if(uc3D_SYNC_Avaliable_Count > DDP_3D_SYNC_CHECK_TIMER)
                    {
                        halMCU_3D_Sync_In_Select_Set(e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC);
                        halFormatter_3D_SYNC_SelectionSet(e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC);
                        palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Set(e3D_SYNC_INPUT_SELECTION_EXTERNAL_INPUT_3D_SYNC);
                        palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                    }
                }
                else
                {
                    uc3D_SYNC_Avaliable_Count = 0;
                }
            }
            else
            {
                uc3D_SYNC_Unavaliable_Count = 0;
                uc3D_SYNC_Avaliable_Count = 0;
            }
        }
        else
        {
            uc3D_SYNC_Unavaliable_Count = 0;
            uc3D_SYNC_Avaliable_Count = 0;
        }
#else
    BOOL b3DSyncAvaliable = FALSE;
    BOOL b3DSyncLost = FALSE;
    UINT8 ucdatapath_state = 0;
    UINT8 uc3D_LR_REFERENCE = 0;
    eRESULT eResult = rcINVALID;

    if(palDataPath_IsSourceMonitor() == TRUE)
    {
        palDataMgr_Data_Access(edc3D_LR_REFERENCE, edaREAD, (void*)&uc3D_LR_REFERENCE);
        if((palDataMgr_IS_3D_Enable() == ets_ON) &&
           (uc3D_LR_REFERENCE == eCM_3D_LR_REFERENCE_FIELD_GPIO))
        {
            eResult = halFormatter_3D_SYNC_InputMonitorGet(&b3DSyncAvaliable,&b3DSyncLost,&ucdatapath_state);

            LOG_MSG(db_APP_ENVIRONMENT, "DDP1(%d,%d,%d)(%d,%d) \r\n",b3DSyncAvaliable,b3DSyncLost,ucdatapath_state,eResult,uc3D_SYNC_Avaliable_Count);
            if((eResult == rcSUCCESS) &&
               (ucdatapath_state == DPP_MONITOR_SOURCE) &&
               (b3DSyncLost == TRUE) &&
               (b3DSyncAvaliable == FALSE))
            {
                uc3D_SYNC_Avaliable_Count ++;
                if(uc3D_SYNC_Avaliable_Count > DDP_3D_SYNC_CHECK_TIMER)
                {
                    uc3D_LR_REFERENCE = eCM_3D_LR_REFERENCE_1ST_FRAME;
                    palDataMgr_Data_Access(edc3D_LR_REFERENCE, edaWRITE_THROUGH_WITH_ACTION, (void*)&uc3D_LR_REFERENCE);
                }
            }
            else
            {
                uc3D_SYNC_Avaliable_Count = 0;
            }
        }
        else
        {
            uc3D_SYNC_Avaliable_Count = 0;
        }
    }
    else
    {
        uc3D_SYNC_Avaliable_Count = 0;
    }
#endif
}

void palEnvironment_ReCheckOrientationState(void)   //G100_Simon_0019
{
    m_sPalEnvInfo.ucCeilingMountCheck = 0;
    m_sPalEnvInfo.ucCurrentCeilingMount = 99;
    m_sPalEnvInfo.ucLensChanged = TRUE;
}

void palEnvironment_LensMemorySavingCondition_Set(UINT16 uiItemCondition)   //G100_Simon_0048
{
    m_sPalEnvInfo.ulLensMemorySavingCondition = uiItemCondition ;

    //update to database
    utilDatabase_WriteInformationData(eDI_LENS_MEMORY_SAVE_CONDITION, DATA_TYPE_UI_DIGIT_16, 0, &uiItemCondition);
}

UINT16 palEnvironment_LensMemorySavingCondition_Get(void)   //G100_Simon_0048
{
    return m_sPalEnvInfo.ulLensMemorySavingCondition ;
}

void palEnvironment_Fast_Power_On_to_LAN_Standby_Start(void)
{
    m_sPalEnvInfo.ucFastPowerOnProcessTimer = FAST_PWR_ON_COOLING_START;
}


void palEnvironment_Fast_Power_On_to_LAN_Active_Start(void)
{
    m_sPalEnvInfo.ucFastPowerOnProcessTimer = FAST_PWR_ON_WARMING_START;
}

void palEnvironment_Fake_Power_Down_Set(BOOL bFakePD)
{
    if(m_sPalEnvInfo.bFake_Power_Down != bFakePD)
    {
        UINT8 ucRetry = 0;
        UINT8 ucLDEnable = 0;
        UINT8 ucFadeStep = 0;
        UINT8 ucData;

        m_sPalEnvInfo.bFake_Power_Down = bFakePD;
        if(m_sPalEnvInfo.bFake_Power_Down)
        {
            ucData = eOSD_LOCKED;
            palDataMgr_Data_Access(edcOSD_LOCK, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);
            palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL); //appGui_Send_OSD_Exit();

            do{ //LD off
                palDataMgr_Data_Access(edcFADE_OUT, edaREAD, (void*)&ucFadeStep);
                palFormatterMgr_Smooth_Enable_Set(FALSE, ucFadeStep);
                MS_SLEEP(5);
                palFormatterMgr_Smooth_Enable_Get(&ucLDEnable);
            }while((ucRetry++ < 5) && (ucLDEnable == FALSE));

            palDataMgr_Data_Access(edcPIN_PROTECT, edaREAD, &ucData);
            if(ucData)
            {
                UINT32  ucPin_TotalRemainderTime = 0;

                palDataMgr_Data_Access(edcSECURITY_TIMER, edaREAD, &ucPin_TotalRemainderTime);
                if(ucPin_TotalRemainderTime == 0)
                {
                    UINT8 ucValue = ePASSWORD_PROTECT_UNLOCK;
                    palDataMgr_UI_EventSend(edcUI_EVENT_PIN_PROTECT_ENABLE, FALSE, NULL);
                    palDataMgr_Data_Access(edcSYSTEM_PROTECT_STATUS, edaWRITE_RAM_ONLY_NO_ACTION, &ucValue);
                }
                //appGui_PIN_Protect_Enable(FALSE);
            }

            palEnvironment_Fast_Power_On_to_LAN_Standby_Start();  //G100_Tim_0008, mod //G100_Tim_0007, add

            palEnvironment_Fast_Power_On_TimerSet(1);
            palLedProc_LED_Behavior_Set(eLED_STATUS_POWEROFF);
            palLedProc_Shutter_LED_Set(eLED_SHUTTER_ON); //A35G2_BRC_Casper_0025
        }
        else
        {
            UINT8 ucStartupShutter = 0; //G100_Owen_0101 //A35G2_BRC_Casper_0051

            palDataMgr_Data_Access(edcSTARTUP_SHUTTER, edaREAD, &ucStartupShutter); //G100_Owen_0101
            if(ucStartupShutter != 0)   //G100_Owen_0101
            {
                palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStartupShutter);
                palLedProc_Shutter_LED_Set(eLED_SHUTTER_CLOSE);
            }
            else
            {
                do{ //LD on
                    palDataMgr_Data_Access(edcFADE_IN, edaREAD, (void*)&ucFadeStep);
                    palFormatterMgr_Smooth_Enable_Set(TRUE, ucFadeStep);
                    MS_SLEEP(5);
                    palFormatterMgr_Smooth_Enable_Get(&ucLDEnable);
                }while((ucRetry++ < 5) && (ucLDEnable == FALSE));

                palLedProc_Shutter_LED_Set(eLED_SHUTTER_OFF);
            }
            ucData = eOSD_NOT_LOCKED;
            palDataMgr_Data_Access(edcOSD_LOCK, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);

            palEnvironment_Fast_Power_On_to_LAN_Active_Start();  //G100_Tim_0008, mod //G100_Tim_0007, add

            palEnvironment_Fast_Power_On_TimerSet(0);
            palLedProc_LED_Behavior_Set(eLED_STATUS_NORMAL);
        }
    }
}

BOOL palEnvironment_Fake_Power_Down_Get(void)
{
    return m_sPalEnvInfo.bFake_Power_Down;
}

//G100_Clare_0059, add, >>>
void palEnvironment_LensModelCheck_Set(BOOL bModelCheck)
{
    bLensModelCheck = bModelCheck;
    if(bLensModelCheck == FALSE)
    {
        ucLensModelCheckTestMax = 0;
    }
}

BOOL palEnvironment_LensModelCheck_Get(void)
{
    return bLensModelCheck;
}
//G100_Clare_0059, add <<<

void appEnvironment_NotificationSystemUpdate(void)
{
    UINT8 ucValue = 0;
    palDataMgr_Data_Access(edcNOTIFICATIONSYSTEMUPDATE, edaREAD, &ucValue);

	LOG_MSG(db_APP_ENVIRONMENT, "appEnvironment_NotificationSystemUpdate edcNOTIFICATIONSYSTEMUPDATE = %d\r\n", ucValue);

    if(ucValue == ets_ON)
	{
    	if(appSystem_FW_UpgradeCheckGet() == 0)
	    {
	    	if(m_sPalEnvInfo.ucFotaSystemUpdateTimer < 150)//(90+52)=>90 secs wait for AP reply. System start up to FOTA dbus finish on eco mode needs 52 secs.
	    	{
				if(m_sPalEnvInfo.ucFotaSystemUpdateTimer == 60) //A65_OPTOMA_Julie_0114 //A65_OPTOMA_Julie_0106
				{
					eFOTA_SYSTEM_UPDATE_STATUS SystemUpdateStatus = eFOTA_SYSTEM_UPDATE_STATUS_POWER_ON_QUERY;
					palDataMgr_Data_Access(edcSystemUpdateStatus, edaWRITE_THROUGH_WITH_ACTION, &SystemUpdateStatus);
            		LOG_MSG(db_APP_ENVIRONMENT, "Trigger FOTA!!\r\n");
				}
	        	m_sPalEnvInfo.ucFotaSystemUpdateTimer++ ;
	    	}
	        else if(m_sPalEnvInfo.ucFotaSystemUpdateTimer == 150 && (palSystem_PowerStateGet() != ePOWER_STATE_UPGRADE))
			{
				//appGui_SendKeyEvent(eKEYINPUT_CLI, ekFOTA);
				//palInputProc_KeyEventSend(sInputKey);

				palDataMgr_UI_EventSend(edcUI_EVENT_FOTA, TRUE, NULL);
				m_sPalEnvInfo.ucFotaSystemUpdateTimer++ ;
	    	}

            LOG_MSG(db_APP_ENVIRONMENT, "FOTA Timer (%d)\r\n", m_sPalEnvInfo.ucFotaSystemUpdateTimer);
	    }
    }
}


void palEnvironment_LightSourceBlankingOn_Set(BOOL bBlnakingOn,UINT8 ucTimer)   //A70LV_Doulas_0294
{
    if(bLightSourceBlankingEn)
    {
        bLightSourceBlankingOn = bBlnakingOn;
        //LOG_MSG(db_ALWAYS, "### (%d,%d)\r\n",bBlnakingOn,ucTimer); //H30K_Doulas_0028
        if(bBlnakingOn)
        {
	//Cassper_ProAV
	        #ifndef SCALER_FPGA_F34 //H30K_Doulas_0025
            palLDMgr_LD_SourceBlanking_Set(eLDBANK_A70LV, 0);
            ucLightSourceBlankingCounter = ucTimer*4;
            #else
            ucLightSourceBlankingCounter = ucTimer;
            #endif
        }
        else
        {
            UINT8 ucData = 0;
	//Cassper_ProAV
	        #ifndef SCALER_FPGA_F34 //H30K_Doulas_0025
            palLDMgr_LD_SourceBlanking_Set(eLDBANK_A70LV, 1);
            MS_SLEEP(10);
            palLDMgr_LD_SourceBlanking_Get(eLDBANK_A70LV, &ucData);  //A70LV_Doulas_0306
            #endif
            if(ucData)  //A70LV_Doulas_0306 //blanking off
            {
                ucLightSourceBlankingCounter = 0;
            }
            else
            {
                bLightSourceBlankingOn = TRUE;
                ucLightSourceBlankingCounter = 1;
            }
            //LOG_MSG(db_ALWAYS, "Set0 (%d)\r\n",ucData);
        }
    }
}

void palEnvironment_LightSourceBlankingEn_Set(BOOL bBlnakingEn)     //A70LV_Doulas_0294
{
    bLightSourceBlankingEn = bBlnakingEn;
    if(bBlnakingEn)
    {
        //palLDMgr_LD_SourceBlanking_Set(eLDBANK_A70LV, 1);
    }
    else
    {
	//Cassper_ProAV
	    #ifndef SCALER_FPGA_F34 //H30K_Doulas_0025
        palLDMgr_LD_SourceBlanking_Set(eLDBANK_A70LV, 1);
        #endif
        bLightSourceBlankingOn = FALSE;
        ucLightSourceBlankingCounter = 0;
    }
}

void palEnvironment_LightSourceBlanking_Monitor(void)       //A70LV_Doulas_0294
{
    //DDP_SYSTEM_STATUS sDDPSystemStatus;
    //eRESULT eResult = rcINVALID;

    if(bLightSourceBlankingEn && bLightSourceBlankingOn)
    {
        //LOG_MSG(db_ALWAYS, "palEnvironment_LightSourceBlanking_Monitor (%d)\r\n",ucLightSourceBlankingCounter);
        if(ucLightSourceBlankingCounter > 0)
        {
            ucLightSourceBlankingCounter--;
        }
        else
        {
            #if 0
            eResult = halFormatter_SYS_SystemStatusGet((PUINT8)&sDDPSystemStatus);
            if((eResult == rcSUCCESS) &&
	           (sDDPSystemStatus.DDP_ST_SYSTEM_READY == TRUE)&&
	           (sDDPSystemStatus.DDP_ST_CW_SPINNING == TRUE)&&
	           (sDDPSystemStatus.DDP_ST_SEQ_PHASELOCK == TRUE)&&
	           (sDDPSystemStatus.DDP_ST_SEQ_FREQLOCK == TRUE)&&
	           //(sDDPSystemStatus.DDP_ST_CW_PHASELOCK == TRUE)&&
	           (sDDPSystemStatus.DDP_ST_CW_FREQLOCK == TRUE))
            #endif
            {
                palEnvironment_LightSourceBlankingOn_Set(FALSE,0);
            }

        }
    }
}

BOOL palEnvironment_LightSourceBlankingEn_Get(void)     //A70LV_Doulas_0300
{
    return bLightSourceBlankingEn;
}

BYTE palEnvironment_FEOPD_Cnt_Get(void)
{
	return ucFEOPDCnt;
}

void palEnvironment_FEOPD_Cnt_Set(BYTE cCnt)
{
	ucFEOPDCnt = cCnt;
} //G100_Steven_0059 end

void palEnvironment_LensCalFlag_Update(void)       //G100_Owen_0076
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    m_ucMOT_Cal_Flag = (UINT8)palMotor_LSM_CalibrationDone_Get();
#else
    UINT8 ucRetry = 0, ucLensCal = FALSE;
    eRESULT eResult = rcERROR;

    do
    {
        eResult = halMotor_LensCalDone_Get(&ucLensCal);
    }while((++ucRetry < 3)&&(eResult != rcSUCCESS));

    m_ucMOT_Cal_Flag = ucLensCal;
#endif
    palLANProcSendToLAN(edcLENS_CAL_FLAG);
}


UINT8 palEnvironment_LensCalFlag_Get(void)      //G100_Owen_0076
{

    #ifdef CAL_ALWAYS_DONE
        return TRUE;
    #endif

    return m_ucMOT_Cal_Flag;
}

void palEnvironment_LensCalFlag_Set(BYTE cValue)
{

    #ifdef CAL_ALWAYS_DONE
        m_ucMOT_Cal_Flag = TRUE;
    #endif

    m_ucMOT_Cal_Flag = cValue;

    palLANProcSendToLAN(edcLENS_CAL_FLAG);
    palLANProcSendToLAN(edcMOTOR_LENS_CONTROL);
}

UINT8 palEnvironment_LensSpecialFlag_Get(void) //A35G2_Wesley_0163 //A70Gen2_Julie_0109//HICC2_Julie_0014
{
    return m_ucMOT_Special_Flag;
}

void palEnvironment_LensSpecialFlag_Set(BYTE cValue) //A35G2_Wesley_0163 //A70Gen2_Julie_0109//HICC2_Julie_0014
{
    m_ucMOT_Special_Flag = cValue;
    palLANProcSendToLAN(edcLENS_SPECIAL_LOCATION_FLAG); //HICC2_Doulas_0138
}

UINT8 palEnvironment_Auto_HDMI_Switch_Source_Changing_Get(void)
{
    return 0;
}

void palEnvironment_Auto_HDMI_Switch_Source_Changing_Set(UINT8 ucValue)
{
    return;
}

void palEnvironment_Auto_HDMI_Switch_Source_Monitor(void)
{
    return;
}


//A65_OPTOMA_Jerry_0004 start
// ==============================================================================
// FUNCTION NAME: palEnvironment_ProServicePowerDownMessageCountGet
// DESCRIPTION: Get ProService Power Down Message
// Params: None
// Returns: Power Down Count
// Modification History
// --------------------
// 2020 / 9 / 2
// --------------------
// ==============================================================================
UINT32 palEnvironment_ProServicePowerDownMessageCountGet(void)
{
    return m_sPalEnvInfo.iProServicePowerDownCount;
}

// ==============================================================================
// FUNCTION NAME: palEnvironment_ProServicePowerDownMessageCountSet
// DESCRIPTION: Set ProService Power Down Message
// Params: Power Down Count
// Returns: None
// Modification History
// --------------------
// 2020 / 9 / 2
// --------------------
// ==============================================================================
void palEnvironment_ProServicePowerDownMessageCountSet(UINT32 ucValue)
{
    m_sPalEnvInfo.iProServicePowerDownCount = ucValue;
    palLANProcSendToLAN(edcPROSERVICE_POWERDOWN_COUNT);
}


// ==============================================================================
// FUNCTION NAME: palEnvironment_ProServicePowerDownTimerGet
// DESCRIPTION: Get ProService Power Down Timer
// Params: None
// Returns: Power Down Timer
// Modification History
// --------------------
// 2020 / 9 / 2
// --------------------
// ==============================================================================
UINT32 palEnvironment_ProServicePowerDownTimerGet(void)
{
    return m_sPalEnvInfo.iProServicePowerDownTimer;
}

// ==============================================================================
// FUNCTION NAME: palEnvironment_ProServicePowerDownTimerSet
// DESCRIPTION: Set ProService Power Down Timer
// Params: Power Down Timer
// Returns: None
// Modification History
// --------------------
// 2020 / 9 / 2
// --------------------
// ==============================================================================
void palEnvironment_ProServicePowerDownTimerSet(UINT32 ucValue)
{
    m_sPalEnvInfo.iProServicePowerDownTimer = ucValue;
}
//A65_OPTOMA_Jerry_0004 end

void palEnvironment_ReplaceModel_TimerSet(void) //A65_OPTOMA_Julie_0061
{
	halMCU_Power_Restart_Set(ets_ON);

	m_sPalEnvInfo.iReplaceModelPowerDownTimer = 5; //A35G2_Coda_0063

    //LOG_MSG(db_ALWAYS, "SleepTimer %d\r\n",m_sPalEnvInfo.iReplaceModelPowerDownTimer);
}

UINT32 palEnvironment_SystemTimer_Get(void) //A65_OPTOMA_Julie_0101
{
	return m_sPalEnvInfo.ulSystemTimer;
}

//G100_Clare_0022, add, start
// ==============================================================================
// FUNCTION NAME: palEnvironment_OSD_Lock_Timer_Set
// DESCRIPTION: set unlock timer for OSD lock time out
// Params: ucOSD_Unlock_Time : time out (seconds)
// Returns: none
// Modification History
// --------------------
// 2020/8/20 Create
// --------------------
// ==============================================================================
void palEnvironment_OSD_Lock_Timer_Set(UINT16 ucOSD_Unlock_Time)
{
    m_sPalEnvInfo.uiOSD_Unlock_Timer = ucOSD_Unlock_Time;
}


// ==============================================================================
// FUNCTION NAME: palEnvironment_OSD_Lock_Timer_Get
// DESCRIPTION: get unlock timer
// Params: none
// Returns: unlick timer
// Modification History
// --------------------
// 2020/8/20 Create
// --------------------
// ==============================================================================
UINT16 palEnvironment_OSD_Lock_Timer_Get(void)
{
    return ( m_sPalEnvInfo.uiOSD_Unlock_Timer );
}
//G100_Clare_0022, add, end

#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING                   //G100_Tim_0046, add, start
// ==============================================================================
// DESCRIPTION:  Update Power Active again to sync the power status of the LAN
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/08/05, Tim Chen create
// --------------------
// ==============================================================================
void palEnvironment_Camera_Force_Update_Power_Status_Lunch(void)
{
    palLANProcPowerStateSend(ePOWER_STATE_ACTIVE, 4, &m_ucDelayCnt);
    m_ucUpdate_Power_Status = eUPDATE_POWER_STATUS_MAX;
}


// ==============================================================================
// DESCRIPTION:  Check the power active shall be update or not, in palEnvironment_Poll( ).
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/08/05, Tim Chen create
// --------------------
// ==============================================================================
void palEnvironment_Camera_Force_Update_Power_Status_Check(void)
{
    if( ( m_ucUpdate_Power_Status < eUPDATE_POWER_STATUS_MAX )                  // triggered.
        && ( m_ucDelayCnt > 0 ) )                                               // wait for sending
    {
        m_ucDelayCnt--;

        if( m_ucDelayCnt == 0 )                                                 // send power active now
        {
            palEnvironment_Camera_Force_Update_Power_Status_Lunch();
        }
    }
}


// ==============================================================================
// DESCRIPTION:  Trigger sending power active to LAN, but not right now.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/08/05, Tim Chen create
// --------------------
// ==============================================================================
void palEnvironment_Camera_Force_Update_Power_Status_Set(void)
{
    m_ucUpdate_Power_Status = eUPDATE_POWER_STATUS_ACTIVE;
    m_ucDelayCnt = 2;                   // delay 1 ~ 2 seconds before sending.
}
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING                 //G100_Tim_0046, add, end


INT16 pal_ScalerBlankingLD_Poll(UINT16 uiTick) //HICC2_Doulas_0023
{
    if(palCoreVar_GetLightSourceOnVar())
    {
        palEnvironment_LightSourceBlanking_Monitor();
    }
    return ENVIRONMENT_LIGHTSOURCE_BLANKING_MONITOR_PERIOD / POLL_PERIOD;
}

void palEnvironment_LENS_EEPROM_Version_Get(UINT8* pcData)  //HICC2_Steven_0027
{
	pcData[0] = aucLensEEPROMVersion[0];
	pcData[1] = aucLensEEPROMVersion[1];
}


void palEnvironment_DMD_Airtight_Get(void) //HICC2_Steven_0032
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucStatus = 0;
    UINT8 ucTemp = 0;
    palDataMgr_Data_Access(edcDMD_AIRTIGHT_STATUS, edaREAD, &ucTemp);

    eResult = palSysCtrlMgr_DMD_AirTightGet(&ucStatus);

	//LOG_MSG(db_ALWAYS,"palEnvironment_DMD_Airtight_Get [%d, %d] \n", ucTemp, ucStatus);

    if(eResult != rcERROR && ucTemp!= ucStatus)
    {
    	 palDataMgr_Data_Access(edcDMD_AIRTIGHT_STATUS, edaWRITE_RAM_ONLY_NO_ACTION, &ucStatus);
    }

}

void palEnvironment_LenCalibrationTimerReset(void)
{
    m_sPalEnvInfo.cLensCalibrationTimer = 0;
}

BOOL palEnvironment_Send_PIN_StatusGet(void)
{
	return bSend_PIN_Status;
}

void palEnvironment_Send_PIN_StatusSet(BOOL bValue)
{
	bSend_PIN_Status = bValue;
}



