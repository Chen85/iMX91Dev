#include "appIllumination.h"
#include "appMailBox.h"
#include "utilDbgMsg.h"
#include "appDataPath.h"
#include "appEnvironment.h"
#include "appDataMgr.h"
#include "appPoll.h"
//#include "appGui.h"
#include "appLedProcAPI.h" //A70LV_Larry_0020
#include "appLANProcAPI.h"

#include "appSystem.h"
#include "utilCommon.h"
#include "utilHPBU_Tester.h"    //A70G2_Owen_0001
#include "halFormatter.h"
#include "palLDMgr.h"
#include "halFanCtrlAPI.h"
#include "halFrontEndCtrlAPI.h"
#include "halMotorCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "utilCRCAPI.h" //G100_Steven_0011
#include "GEC_EventTable.h"   //HICC2_Doulas_0117
#include "GEC_CoreFunction.h" //G50_Casper_0006

#ifdef Low_Latency_All
#include "halScaler.h"
#endif	/*Low_Latency_All*/
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif

//#include "dvLpcPW_Protect.h"
//#include "dvLPCIOExp.h"
#include "utilMathAPI.h"
#include "utilCLICmdAPI.h"
//#include "utilOPD_TEST.h"
#include "utilDataMgrAPI.h"
#ifdef CUSTOM_OPTOMA
#include "utilOptomaMSSCAPI.h"
#endif

#include "palCoreVar.h"
#include "palGui.h"
#include "palMotorMgr.h"
#include "palFormatterMgr.h"

//	#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
//	#include "./OSD_BARCO/Custom_OSD.h"
//	#else
//	#include "./OSD/Custom_OSD.h"
//	#endif
#define LIGHHT_SENSOR_GET_TARGET_NOT_RLD_OFF	//A70Gen2_Doulas_0035

static sPAL_ILLUMINATION_INFORMATION       m_sPalIllumInfo;

extern sGUI_CALLBACK GuiCb;

static UINT8 m_cWorkCounterCheck = PHASE_LOCK_COUNT_DOWN;     //A70LV_Doulas_0212
static UINT8 m_cPhaseLockMaxCounterCheck = 0;                 //A70LV_Doulas_0212
//static sLIGHT_SENSOR m_sLightSensor;
static sLIGHT_SENSOR sLightSenosr_RLD_ON;					//G100_Doulas_0023
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)	//A35G2_OTM_Coda_0002
static UINT16 cTEC_Gating_PassCnt = 0; //G100_Steven_0084
static BOOL b_TEC_Check_F = 0;
static BOOL b_TEC_First_F = 0; //G100_Steven_0087
static sTEC_GATING_INFO  sTEC_GATING = {0};
static BOOL b2D3DFlag = TRUE; //A35G2_BRC_Casper_0077

UINT16 TEC_GATING_TEMP[TEC_Gating_NUMBER] = {2400, 2900, 3700, 4200, 4600, 5300};
UINT16 TEC_GATING_LU_DMDTEMP[TEC_Gating_NUMBER] = {4100, 4100, 4100, 4700, 5300, 5500};
UINT16 TEC_GATING_LU_DMDCurrent[TEC_Gating_NUMBER] = {1000, 2000, 2100, 2800, 1500, 1500};
UINT16 TEC_GATING_LV_DMDTEMP[TEC_Gating_NUMBER] = {4000, 4000, 4400, 4700, 5300, 5500};
UINT16 TEC_GATING_LV_DMDCurrent[TEC_Gating_NUMBER] = {1000, 1500, 2000, 2500, 1500, 1500};
//G100_Steven_0084

#if TEC_TESTER_JIG //G100_Steven_0080
static UINT8 m_cTEC_Tester_LDEn_Cnt = 0;
#endif
#endif

#ifdef OE_JIG //A70LV_Larry_0322
static UINT8 m_ucFan14ErrorCount = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan1ErrorCount  = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan2ErrorCount  = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan3ErrorCount  = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan7ErrorCount  = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan8ErrorCount  = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan10ErrorCount = 0;   //HICC2_Doulas_0034
static UINT8 m_ucFan11ErrorCount = 0;   //HICC2_Doulas_0032
static UINT8 m_ucFan15ErrorCount = 0;   //HICC2_Doulas_0032
static UINT8 m_ucFan12ErrorCount = 0;
static UINT8 m_ucFan17ErrorCount = 0;   //HICC2_Doulas_0032
static UINT8 m_ucPumpErrorCount = 0;
static UINT8 m_ucTecDMDErrorCount = 0;
static UINT8 m_ucTecRLDErrorCount = 0;
static UINT8 m_ucDMDTempErrorCount = 0; //HICC2_Doulas_0032
static UINT8 m_ucOE_JIG_Error = 0;
#endif /* OE_JIG */

static UINT8 m_OPD_EngineLog_Index = eOPD_ENGINE_LOG; //A35G2_CDS_Larry_0020 //A65_OPTOMA_Julie_0028

static UINT32  m_dwOPDPeriodCnt = 0;  //HICC2_Steven_0009

#define SMOOTH_ON_DELAY (6) //A70LV_Larry_0255
#define SMOOTH_ON_READY (5) //A70LV_Larry_0340
#define SMOOTH_ON_DONE  (0)

void palIllumination_OPDSnapshotPowerOnDelay(UINT8 cDelay);

eEXEC_CODE palIllumination_Init(void)
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    UINT8 ucFanFilter = 0;

    //m_sPalIllumInfo.ulLampPeriod = 0;
    palDataMgr_Data_Access(edcLD_HOURS,   edaREAD, &m_sPalIllumInfo.ulLampPeriod);     //A70LV_Doulas_0061 modify

    m_sPalIllumInfo.uiLampUpdateCount = 0;
    m_sPalIllumInfo.ulStartUpLampPeriod = 0;
    m_sPalIllumInfo.bIllumEnabled = FALSE;
    m_sPalIllumInfo.bFirstLightSourceOn = FALSE; //A70LV_Larry_0020
    //m_sPalIllumInfo.ucSmoothOn = 0; //A70LV_Larry_0131
    palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS,   edaREAD, &m_sPalIllumInfo.ulTotalProjectorHours);    // A70LV_Doulas_0061
    utilOPD_TotalProjector_Set(palIllumination_TotalProjectSec_Get()); //A70Gen2_Julie_0019//A65_OPTOMA_Julie_0080
    m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount = 0;       //A70LV_Doulas_0061
    m_sPalIllumInfo.ucFanPollCount = 0; //A70LV_Larry_0251
    m_sPalIllumInfo.ucSmoothOn = SMOOTH_ON_DELAY; //A70LV_Larry_0255
    m_sPalIllumInfo.cLensPlug = 99; //A70LV_Larry_0340
    m_sPalIllumInfo.ucRentalModeEnable = 0;
    m_sPalIllumInfo.ucLensDetection = 1;

    m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable = 0;
    m_sPalIllumInfo.sBurnIn_Info.ucLampOn = 0;
    m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle= 0;

    m_sPalIllumInfo.ulCurrentRunTime = 0;

    palDataMgr_Data_Access(edcLENS_DETECTION, edaREAD, &m_sPalIllumInfo.ucLensDetection);

#ifdef CUSTOM_BARCO
    palIllumination_TEC_Gating_InfoGet(); //G100_Steven_0084
#endif

    GuiCb = Gui_fpCallbackGet();

    //halBoard_FPGA_Reset_Set(LOW);
    //MS_SLEEP(500);
    //halBoard_FPGA_Reset_Set(HIGH);

    #if 0   //A35G2_Simon_0107 remove
    #ifdef QUICKLY_POWER_ON //G100_Doulas_0024
    halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
    #else
    halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
    #endif
    #endif

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palIllumination_PowerNormal(void)
{
    UINT8 ucData = 0, ucFlag = 0;
    //UINT8 ucDimModeFlag = 0;		//A70Gen2_Doulas_0057

    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //palSystem_VersionCheck(FALSE); //A70LV_Larry_0112 //G100_Simon_0056
    //palDataMgr_Data_Access(edcDIMMING_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucDimModeFlag);	//A70Gen2_Doulas_0057

    ucData = 1; //default LD off //G100_Larry_0008
    palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_NO_ACTION, &ucData);
#ifdef OE_JIG

    m_ucFan1ErrorCount  = 0;   //HICC2_Doulas_0034
    m_ucFan2ErrorCount  = 0;   //HICC2_Doulas_0034
    m_ucFan3ErrorCount  = 0;   //HICC2_Doulas_0034
    m_ucFan7ErrorCount  = 0;   //HICC2_Doulas_0034
    m_ucFan8ErrorCount  = 0;   //HICC2_Doulas_0034
    m_ucFan10ErrorCount = 0;   //HICC2_Doulas_0034
    m_ucFan11ErrorCount = 0; //HICC2_Doulas_0032
    m_ucFan12ErrorCount = 0;
    m_ucFan14ErrorCount = 0;   //HICC2_Doulas_0034
    m_ucFan15ErrorCount = 0; //HICC2_Doulas_0032
    m_ucFan17ErrorCount = 0; //HICC2_Doulas_0032
    m_ucPumpErrorCount = 0;
    m_ucTecDMDErrorCount = 0;
    m_ucDMDTempErrorCount = 0; //HICC2_Doulas_0032
    m_ucTecRLDErrorCount = 0;
    m_ucOE_JIG_Error = 0;

     m_sPalIllumInfo.ucLensDetection = 0;
     LOG_MSG(db_ALWAYS, "Enter no fan control\r\n");

     //halFanCtrl_Platform_Set(eMODEL_TYPE_DWU23HS); //HICC2_Doulas_0032 remove

    halFanCtrl_NoFanControlSet();
    palLDMgr_DisableShutdown();

    ucData = eCM_POWER_MODE_CONSTANT_POWER; //A70LV_Larry_0322
    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&ucData);

    ucData = 1; //A70LV_Larry_0322
    palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&ucData);

    //ucData = 1; //HICC2_Doulas_0034 remove
    //palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData); //HICC2_Doulas_0034 remove

    //ucData = eCM_COLOR_WHEEL_SPEED_3X; //A70LV_Larry_0322
    //palDataMgr_Data_Access(edcCOLOR_WHEEL_SPEED, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&ucData);
#endif

    palCoreVar_SetLightSourceOnVar(TRUE);
    m_sPalIllumInfo.bIllumEnabled = TRUE;
    m_sPalIllumInfo.bFirstLightSourceOn = FALSE; //A70LV_Larry_0020
    m_sPalIllumInfo.ucLightSourceMode = 99; //A70LV_Larry_0181
    m_sPalIllumInfo.ucABPDelayStart = 10; //A70LV_Larry_0181
    m_sPalIllumInfo.ucConstantBrightness = ets_OFF;		//G100_Doulas_0008
    m_sPalIllumInfo.ucLVPS_110VDet = 0;

    m_sPalIllumInfo.ulCurrentRunTime = 0;
    m_sPalIllumInfo.ucFanFilter = 99;
    m_sPalIllumInfo.ulCurrentRunTime = 0;
    m_sPalIllumInfo.ucSnapshotPowerOn = 0;

    //read burnin information for lamp time count with burnin function
    palDataMgr_Data_Access(edcBURNIN_ENABLE, edaREAD, &m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable);
    palDataMgr_Data_Access(edcBURNIN_ON, edaREAD, &m_sPalIllumInfo.sBurnIn_Info.ucLampOn);
    //palDataMgr_Data_Access(edcBURNIN_OFF, edaREAD, &m_sPalIllumInfo.sBurnIn_Info.ucLampOff);
    palDataMgr_Data_Access(edcBURNIN_CYCLE, edaREAD, &m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle);

    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)ucBurnInEnable=%d, ucBurnInCycle=%d\r\n", __FUNCTION__, __LINE__, m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable, m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle);

	halMCUCtrl_Standby_Error_Get(&ucFlag); //G100_Julie_0022
#ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0076
    palImgMgr_WB_Black_Screen_Set(ets_OFF);   //G100_Owen_0121
#endif
    return eEXEC_CODE_PASS;

}

#if defined(CUSTOM_CHRISTIE) || defined(CUSTOM_OPTOMA) //A35G2_Coda_0135
void palIllumination_LightSourceMode_Init(void)
{
    m_sPalIllumInfo.ucLightSourceMode = 99;
    return;
}
#endif

eEXEC_CODE palIllumination_UpgradeCooling(void) //A70LV_Larry_0112
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //TODO
    /*
       1.Disable Lamp
       2.DMD Park
       3.CW Stop
    */
    if(m_sPalIllumInfo.bIllumEnabled)
    {
		UINT8 ucRetry = 0;	//G100_Clare_0004
		UINT8 ucLDOff = 0;	//G100_Clare_0004
        palCoreVar_SetLightSourceOnVar(FALSE);
		do{	//G100_Clare_0004
			palFormatterMgr_Smooth_Enable_Set(FALSE, FADE_OUT_TIMER_QUICK); //A65_Clare_0002   //G100_Simon_0086//HICC2_Julie_0009
			//MS_SLEEP(FADE_TIMER_MIN_VALUE*100 + 1000);  //G100_Simon_0086//HICC2_Julie_0009
			MS_SLEEP(5);
			palFormatterMgr_Smooth_Enable_Get(&ucLDOff);
		}while((ucRetry++ < 5)&&(ucLDOff != FALSE));  //G100_Simon_0086
        //palLDMgr_LD_SourceEnable_Set(eLDBANK_A70LV, 0);
		//LD off
    }

    m_sPalIllumInfo.bIllumEnabled = FALSE;
    m_sPalIllumInfo.bFirstLightSourceOn = FALSE;


    return eEXEC_CODE_PASS;
}


eEXEC_CODE palIllumination_PowerStandby(void)
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //TODO
    /*
       1.Disable Lamp
       2.DMD Park
       3.CW Stop
    */
    if(m_sPalIllumInfo.bIllumEnabled)
    {
		UINT8 ucRetry = 0;	//G100_Clare_0004
		UINT8 ucLDOff = 0;	//G100_Clare_0004
		UINT8 ucFade_Out_Step = 0;	//G100_Clare_0004
        palCoreVar_SetLightSourceOnVar(FALSE);

		do{	//G100_Clare_0004
			palDataMgr_Data_Access(edcFADE_OUT, edaREAD, (void*)&ucFade_Out_Step);
        #ifdef CUSTOM_CHRISTIE
            ucFade_Out_Step = FADE_OUT_TIMER_QUICK;
        #endif
			palFormatterMgr_Smooth_Enable_Set(FALSE, ucFade_Out_Step);
			MS_SLEEP(5);
			palFormatterMgr_Smooth_Enable_Get(&ucLDOff);
		}while((ucRetry++ < 5)&&(ucLDOff == FALSE));
        //palLDMgr_LD_SourceEnable_Set(eLDBANK_A70LV, 0);
        halMCU_SystemCooling();

        ucLDOff = 1; //default LD off //G100_Larry_0008
        palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_NO_ACTION, &ucLDOff);

		//LD off
    }
    palEnvironment_StartCoolTimer();

    m_sPalIllumInfo.bIllumEnabled = FALSE;
    m_sPalIllumInfo.bFirstLightSourceOn = FALSE;

    m_sPalIllumInfo.ucFanPollCount = 0; //A70LV_Larry_0251
	m_sPalIllumInfo.ucSmoothOn = SMOOTH_ON_DELAY; //A70LV_Larry_0255
    m_sPalIllumInfo.cLensPlug = 99; //A70LV_Larry_0340
    m_sPalIllumInfo.ucRentalModeEnable = 0;
    m_sPalIllumInfo.ucLensDetection = 1;
    m_sPalIllumInfo.ulStartUpLampPeriod = 0;
    m_sPalIllumInfo.uiLampUpdateCount = 0;

    m_cWorkCounterCheck = PHASE_LOCK_COUNT_DOWN;
    m_cPhaseLockMaxCounterCheck = 0;

    palDataMgr_WheelSpeedPollingSetting(0,0); //clear

    return eEXEC_CODE_PASS;
}

//static BOOL m_bFirstRead = TRUE; //for init //A70LV_Larry_0098

BOOL palIllumination_FirstLightSourceOnGet(void) //A70LV_Larry_0098
{
    //if(m_bFirstRead == TRUE) //A70LV_Larry_0236 mask
    //{
        //m_bFirstRead = FALSE;
        //return TRUE;
    //}
    return m_sPalIllumInfo.bFirstLightSourceOn;
}

void palIllumination_AbpLightSensor(sLIGHT_SENSOR *psLightSenosr) //A70LV_Larry_0142 //A70G2_Owen_0003
{

        UINT8 aucData[eFMT_MSG_LIGHT_SENSOR_SZ]={0};

        palLDMgr_Light_Sensor_Get(eLDBANK_A70LV, aucData);

        psLightSenosr->uiW = ((UINT16)aucData[1] << 8) | (UINT16)aucData[0];
        psLightSenosr->uiY = ((UINT16)aucData[3] << 8) | (UINT16)aucData[2];
        psLightSenosr->uiR = ((UINT16)aucData[5] << 8) | (UINT16)aucData[4];
        psLightSenosr->uiB = ((UINT16)aucData[7] << 8) | (UINT16)aucData[6];
        psLightSenosr->uiG = ((UINT16)aucData[9] << 8) | (UINT16)aucData[8];

        LOG_MSG(db_APP_ILLUMINATION, "Light Sensor %d,%d,%d,%d,%d\r\n",
         psLightSenosr->uiW,psLightSenosr->uiR,psLightSenosr->uiG,psLightSenosr->uiB,psLightSenosr->uiY);

        //appGui_SendOSDPollEvent();
}

void palIllumination_LghtSourceMode_ConstantIntensity(void) //T100_Larry_0022 //A70G2_Owen_0003
{
    UINT16 uiFull_Dynamic_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiEco_Dynamic_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiFull_Dynamic_RLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiEco_Dynamic_RLD_PWM[eLD_SEQ_NUMBER] = {0};

    UINT16 uiTarget_BLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiTarget_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};


    UINT16 uiTargetRLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiCurrent_PWM[eLD_SEQ_NUMBER] = {0};

    UINT16 uiDefaultPWM[8] = {0};
    UINT8  ucCount = 0;

    LOG_MSG(db_APP_ILLUMINATION, "Start constant intensity\r\n");

    //BLD Y/R/B/G, RLD Y/R/B/G
    palDataMgr_Data_Access(edcCONSTANT_INTENSITY_PWM, edaREAD, (void*)&uiDefaultPWM);

    //Set BLD
    uiCurrent_PWM[eLD_SEQ_Y] = uiDefaultPWM[0];
    uiCurrent_PWM[eLD_SEQ_R] = uiDefaultPWM[1];
    uiCurrent_PWM[eLD_SEQ_B] = uiDefaultPWM[2];
    uiCurrent_PWM[eLD_SEQ_G] = uiDefaultPWM[3];

    palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_BLD, edaREAD, (void*)&uiTarget_BLD_LightSensor);

    LOG_MSG(db_APP_ILLUMINATION, "Target BLD PWM %d %d %d %d\r\n", uiCurrent_PWM[eLD_SEQ_Y],
                                                                   uiCurrent_PWM[eLD_SEQ_R],
                                                                   uiCurrent_PWM[eLD_SEQ_B],
                                                                   uiCurrent_PWM[eLD_SEQ_G]);

    LOG_MSG(db_APP_ILLUMINATION, "Target BLD Light %d %d %d %d\r\n", uiTarget_BLD_LightSensor[eLD_SEQ_Y],
                                                                     uiTarget_BLD_LightSensor[eLD_SEQ_R],
                                                                     uiTarget_BLD_LightSensor[eLD_SEQ_B],
                                                                     uiTarget_BLD_LightSensor[eLD_SEQ_G]);

    palLDMgr_TargetBLDLightSet((UINT8 *)&uiTarget_BLD_LightSensor); //A70LV_Larry_0179
    palLDMgr_TargetBLDPWMSet((UINT8 *)&uiCurrent_PWM); //A70LV_Larry_0179

#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    //Set RLD
    palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaREAD, (void*)&uiTarget_RLD_LightSensor);
    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaREAD, (void*)&uiFull_Dynamic_RLD_LightSensor);
    palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaREAD, (void*)&uiEco_Dynamic_RLD_LightSensor);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaREAD, (void*)&uiFull_Dynamic_RLD_PWM);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaREAD, (void*)&uiEco_Dynamic_RLD_PWM);

	LOG_MSG(db_APP_ILLUMINATION, "LIGHTSENSOR_FULL(%d %d) Eco(%d %d)\r\n",uiFull_Dynamic_RLD_LightSensor[eLD_SEQ_R],uiFull_Dynamic_RLD_LightSensor[eLD_SEQ_Y],
																          uiEco_Dynamic_RLD_LightSensor[eLD_SEQ_R],uiEco_Dynamic_RLD_LightSensor[eLD_SEQ_Y]);		//G100_Doulas_0023
	LOG_MSG(db_APP_ILLUMINATION, "PWM_FULL(%d %d) Eco(%d %d)\r\n",uiFull_Dynamic_RLD_PWM[eLD_SEQ_R],uiFull_Dynamic_RLD_PWM[eLD_SEQ_Y],
														          uiEco_Dynamic_RLD_PWM[eLD_SEQ_R],uiEco_Dynamic_RLD_PWM[eLD_SEQ_Y]);		//G100_Doulas_0023

    //Calculate RLD target PWM
	for(ucCount = eLD_SEQ_R; ucCount <= eLD_SEQ_R; ucCount++)
	{
		if(util_RLD_ValueCheck(uiFull_Dynamic_RLD_PWM[ucCount],
                               uiEco_Dynamic_RLD_PWM[ucCount],
                               uiTarget_RLD_LightSensor[ucCount],
                               uiFull_Dynamic_RLD_LightSensor[ucCount],
                               uiEco_Dynamic_RLD_LightSensor[ucCount]) == eEXEC_CODE_PASS) //G100_Doulas_0023 Modify
		{
       		uiTargetRLD_PWM[ucCount] = util_PositiveInterPlacement(uiFull_Dynamic_RLD_PWM[ucCount],
                                                              	   uiEco_Dynamic_RLD_PWM[ucCount],
                                                              	   uiTarget_RLD_LightSensor[ucCount],
                                                                   uiFull_Dynamic_RLD_LightSensor[ucCount],
                                                                   uiEco_Dynamic_RLD_LightSensor[ucCount]);
		}
		else
		{
			uiTargetRLD_PWM[ucCount] = uiDefaultPWM[4+ucCount];
		}
	}
	uiTargetRLD_PWM[eLD_SEQ_Y] = 1;//uiDefaultPWM[4];	//G100_Doulas_0023//G100_Doulas_0010 Modify
	//uiTargetRLD_PWM[eLD_SEQ_R] = uiDefaultPWM[5];	//G100_Doulas_0023//G100_Doulas_0010 Modify
    uiTargetRLD_PWM[eLD_SEQ_B] = 1;
    uiTargetRLD_PWM[eLD_SEQ_G] = 1;

    LOG_MSG(db_APP_ILLUMINATION, "Target RLD PWM %d %d %d %d\r\n", uiTargetRLD_PWM[eLD_SEQ_Y],
                                                                   uiTargetRLD_PWM[eLD_SEQ_R],
                                                                   uiTargetRLD_PWM[eLD_SEQ_B],
                                                                   uiTargetRLD_PWM[eLD_SEQ_G]);

    LOG_MSG(db_APP_ILLUMINATION, "Target RLD Light %d %d %d %d\r\n", uiTarget_RLD_LightSensor[eLD_SEQ_Y],
                                                                     uiTarget_RLD_LightSensor[eLD_SEQ_R],
                                                                     uiTarget_RLD_LightSensor[eLD_SEQ_B],
                                                                     uiTarget_RLD_LightSensor[eLD_SEQ_G]);
    palLDMgr_TargetRLDLightSet((UINT8 *)&uiTarget_RLD_LightSensor); //A70LV_Larry_0179
    palLDMgr_TargetRLDPWMSet((UINT8 *)&uiTargetRLD_PWM); //A70LV_Larry_0179
#endif


    memcpy(m_sPalIllumInfo.uiABPTargetBLDLight, uiTarget_BLD_LightSensor, 8);
#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    memcpy(m_sPalIllumInfo.uiABPTargetRLDLight, uiTarget_RLD_LightSensor, 8);
#endif

    LOG_MSG(db_APP_ILLUMINATION, "Keep LD Light start\r\n");
}

void palIllumination_LghtSourceMode_RentalMode(void) //T100_Larry_0022
{
    UINT16 uiFull_BLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiFull_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiECO_BLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiECO_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};

    UINT16 uiFull_BLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiFull_RLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiECO_BLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiECO_RLD_PWM[eLD_SEQ_NUMBER] = {0};

    UINT16 uiFull_Dynamic_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiEco_Dynamic_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiFull_Dynamic_RLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiEco_Dynamic_RLD_PWM[eLD_SEQ_NUMBER] = {0};

    UINT16 uiTarget_BLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiTarget_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};

    UINT16 uiCurrent_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiTargetRLD_PWM[eLD_SEQ_NUMBER] = {0};

    UINT8  ucCount = 0;

    halFormatter_PWM_Update();

    LOG_MSG(db_APP_ILLUMINATION, "Start Rental Mode\r\n");

    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_BLD, edaREAD, (void*)&uiFull_BLD_LightSensor);
    //LOG_MSG(db_APP_ILLUMINATION, "FULL_BLD %d %d %d %d\r\n",uiFull_BLD_LightSensor[0], uiFull_BLD_LightSensor[1], uiFull_BLD_LightSensor[2], uiFull_BLD_LightSensor[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_BLD, edaREAD, (void*)&uiFull_BLD_PWM);
    //LOG_MSG(db_APP_ILLUMINATION, "PWM_FULL_BLD %d %d %d %d\r\n",uiFull_BLD_PWM[0], uiFull_BLD_PWM[1], uiFull_BLD_PWM[2], uiFull_BLD_PWM[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_RLD, edaREAD, (void*)&uiFull_RLD_LightSensor);
    //LOG_MSG(db_APP_ILLUMINATION, "FULL_RLD %d %d %d %d\r\n",uiFull_RLD_LightSensor[0], uiFull_RLD_LightSensor[1], uiFull_RLD_LightSensor[2], uiFull_RLD_LightSensor[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_RLD, edaREAD, (void*)&uiFull_RLD_PWM);
    //LOG_MSG(db_APP_ILLUMINATION, "PWM_FULL_RLD %d %d %d %d\r\n",uiFull_RLD_PWM[0], uiFull_RLD_PWM[1], uiFull_RLD_PWM[2], uiFull_RLD_PWM[3]);

    palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_BLD, edaREAD, (void*)&uiECO_BLD_LightSensor);
    //LOG_MSG(db_APP_ILLUMINATION, "ECO_BLD %d %d %d %d\r\n",uiECO_BLD_LightSensor[0], uiECO_BLD_LightSensor[1], uiECO_BLD_LightSensor[2], uiECO_BLD_LightSensor[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_BLD, edaREAD, (void*)&uiECO_BLD_PWM);
    //LOG_MSG(db_APP_ILLUMINATION, "PWM_ECO_BLD %d %d %d %d\r\n",uiECO_BLD_PWM[0], uiECO_BLD_PWM[1], uiECO_BLD_PWM[2], uiECO_BLD_PWM[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_RLD, edaREAD, (void*)&uiECO_RLD_LightSensor);
    //LOG_MSG(db_APP_ILLUMINATION, "ECO_RLD %d %d %d %d\r\n",uiECO_RLD_LightSensor[0], uiECO_RLD_LightSensor[1], uiECO_RLD_LightSensor[2], uiECO_RLD_LightSensor[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_RLD, edaREAD, (void*)&uiECO_RLD_PWM);
    //LOG_MSG(db_APP_ILLUMINATION, "PWM_ECO_RLD %d %d %d %d\r\n",uiECO_RLD_PWM[0], uiECO_RLD_PWM[1], uiECO_RLD_PWM[2], uiECO_RLD_PWM[3]);


    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaREAD, (void*)&uiFull_Dynamic_RLD_LightSensor);
    //LOG_MSG(db_APP_ILLUMINATION, "FULL_DYNAMIC_RLD %d %d %d %d\r\n",uiFull_Dynamic_RLD_LightSensor[0], uiFull_Dynamic_RLD_LightSensor[1], uiFull_Dynamic_RLD_LightSensor[2], uiFull_Dynamic_RLD_LightSensor[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaREAD, (void*)&uiEco_Dynamic_RLD_LightSensor);
    //LOG_MSG(db_APP_ILLUMINATION, "ECO_DYNAMIC_RLD %d %d %d %d\r\n",uiEco_Dynamic_RLD_LightSensor[0], uiEco_Dynamic_RLD_LightSensor[1], uiEco_Dynamic_RLD_LightSensor[2], uiEco_Dynamic_RLD_LightSensor[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaREAD, (void*)&uiFull_Dynamic_RLD_PWM);
    //LOG_MSG(db_APP_ILLUMINATION, "PWM_FULL_DYNAMIC_RLD %d %d %d %d\r\n",uiFull_Dynamic_RLD_PWM[0], uiFull_Dynamic_RLD_PWM[1], uiFull_Dynamic_RLD_PWM[2], uiFull_Dynamic_RLD_PWM[3]);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaREAD, (void*)&uiEco_Dynamic_RLD_PWM);
    //LOG_MSG(db_APP_ILLUMINATION, "PWM_ECO_DYNAMIC_RLD %d %d %d %d\r\n",uiEco_Dynamic_RLD_PWM[0], uiEco_Dynamic_RLD_PWM[1], uiEco_Dynamic_RLD_PWM[2], uiEco_Dynamic_RLD_PWM[3]);

    //Calculate BLD target Light sensor value
    uiCurrent_PWM[eLD_SEQ_Y] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_Y);
    uiCurrent_PWM[eLD_SEQ_R] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_R);
    uiCurrent_PWM[eLD_SEQ_B] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_B);
    uiCurrent_PWM[eLD_SEQ_G] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_G);


    for(ucCount = 0; ucCount < eLD_SEQ_NUMBER; ucCount++)
    {
        uiTarget_BLD_LightSensor[ucCount] = util_PositiveInterPlacement(uiFull_BLD_LightSensor[ucCount],
                                                                        uiECO_BLD_LightSensor[ucCount],
                                                                        uiCurrent_PWM[ucCount],
                                                                        uiFull_BLD_PWM[ucCount],
                                                                        uiECO_BLD_PWM[ucCount]);
    }


    palLDMgr_TargetBLDLightSet((UINT8 *)&uiTarget_BLD_LightSensor); //A70LV_Larry_0179
    palLDMgr_TargetBLDPWMSet((UINT8 *)&uiCurrent_PWM); //A70LV_Larry_0179

    LOG_MSG(db_APP_ILLUMINATION, "Target BLD PWM %d %d %d %d\r\n", uiCurrent_PWM[eLD_SEQ_Y],
                                                                   uiCurrent_PWM[eLD_SEQ_R],
                                                                   uiCurrent_PWM[eLD_SEQ_B],
                                                                   uiCurrent_PWM[eLD_SEQ_G]);

    LOG_MSG(db_APP_ILLUMINATION, "Target BLD Light %d %d %d %d\r\n", uiTarget_BLD_LightSensor[eLD_SEQ_Y],
                                                                     uiTarget_BLD_LightSensor[eLD_SEQ_R],
                                                                     uiTarget_BLD_LightSensor[eLD_SEQ_B],
                                                                     uiTarget_BLD_LightSensor[eLD_SEQ_G]);

    //Calculate RLD target Light sensor value

    uiCurrent_PWM[eLD_SEQ_Y] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_Y);
    uiCurrent_PWM[eLD_SEQ_R] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_R);
    uiCurrent_PWM[eLD_SEQ_B] = 1;
    uiCurrent_PWM[eLD_SEQ_G] = 1;

    for(ucCount = eLD_SEQ_Y; ucCount <= eLD_SEQ_R; ucCount++)
    {
        uiTarget_RLD_LightSensor[ucCount] = util_PositiveInterPlacement(uiFull_RLD_LightSensor[ucCount],
                                                                        uiECO_RLD_LightSensor[ucCount],
                                                                        uiCurrent_PWM[ucCount],
                                                                        uiFull_RLD_PWM[ucCount],
                                                                        uiECO_RLD_PWM[ucCount]);
    }

    //LOG_MSG(db_APP_ILLUMINATION, "Target BLD %d %d %d %d\r\n",uiTarget_BLD_LightSensor[0], uiTarget_BLD_LightSensor[1], uiTarget_BLD_LightSensor[2], uiTarget_BLD_LightSensor[3]);
    //LOG_MSG(db_APP_ILLUMINATION, "Target RLD %d %d %d %d\r\n",uiTarget_RLD_LightSensor[0], uiTarget_RLD_LightSensor[1], uiTarget_RLD_LightSensor[2], uiTarget_RLD_LightSensor[3]);

    palLDMgr_TargetRLDLightSet((UINT8 *)&uiTarget_RLD_LightSensor); //A70LV_Larry_0179

    //Calculate RLD target PWM
    for(ucCount = eLD_SEQ_Y; ucCount <= eLD_SEQ_R; ucCount++)
    {
       uiTargetRLD_PWM[ucCount] = util_PositiveInterPlacement(uiFull_Dynamic_RLD_PWM[ucCount],
                                                              uiEco_Dynamic_RLD_PWM[ucCount],
                                                              uiTarget_RLD_LightSensor[ucCount],
                                                              uiFull_Dynamic_RLD_LightSensor[ucCount],
                                                              uiEco_Dynamic_RLD_LightSensor[ucCount]);
    }
    uiTargetRLD_PWM[eLD_SEQ_B] = 1;
    uiTargetRLD_PWM[eLD_SEQ_G] = 1;

    palLDMgr_TargetRLDPWMSet((UINT8 *)&uiTargetRLD_PWM); //A70LV_Larry_0179

    LOG_MSG(db_APP_ILLUMINATION, "Target RLD PWM %d %d %d %d\r\n", uiTargetRLD_PWM[eLD_SEQ_Y],
                                                                   uiTargetRLD_PWM[eLD_SEQ_R],
                                                                   uiTargetRLD_PWM[eLD_SEQ_B],
                                                                   uiTargetRLD_PWM[eLD_SEQ_G]);

    LOG_MSG(db_APP_ILLUMINATION, "Target RLD Light %d %d %d %d\r\n", uiTarget_RLD_LightSensor[eLD_SEQ_Y],
                                                                     uiTarget_RLD_LightSensor[eLD_SEQ_R],
                                                                     uiTarget_RLD_LightSensor[eLD_SEQ_B],
                                                                     uiTarget_RLD_LightSensor[eLD_SEQ_G]);

    memcpy(m_sPalIllumInfo.uiABPTargetBLDLight, uiTarget_BLD_LightSensor, 8);
    memcpy(m_sPalIllumInfo.uiABPTargetRLDLight, uiTarget_RLD_LightSensor, 8);

    LOG_MSG(db_APP_ILLUMINATION, "Keep LD Light start\r\n");
}

void palIllumination_ReduceLighthandler(void) //T100_Larry_0022 //A70G2_Owen_0003
{
    UINT16 uiFull_BLD_LightSensor[eLD_SEQ_NUMBER] = {0};

#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    UINT16 uiFull_Dynamic_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiEco_Dynamic_RLD_LightSensor[eLD_SEQ_NUMBER] = {0};
    UINT16 uiFull_Dynamic_RLD_PWM[eLD_SEQ_NUMBER] = {0};
    UINT16 uiEco_Dynamic_RLD_PWM[eLD_SEQ_NUMBER] = {0};
#endif
    UINT16 uiTarget_BLD_LightSensor[eLD_SEQ_NUMBER] = {0};

   // UINT16 uiCurrent_PWM[eLD_SEQ_NUMBER] = {0};
#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    UINT16 uiTargetRLD_PWM[eLD_SEQ_NUMBER] = {0};
#endif
    UINT16 uiReduceLight = 0;

    UINT32 ulTargetLight = 0;

    UINT8  ucCount = 0;
	UINT16 uiDefaultPWM[8] = {0};		//G100_Doulas_0010

    LOG_MSG(db_APP_ILLUMINATION, "Reduce Light handler\r\n");
	palDataMgr_Data_Access(edcCONSTANT_INTENSITY_PWM, edaREAD, (void*)&uiDefaultPWM);		//G100_Doulas_0010
    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_BLD, edaREAD, (void*)&uiFull_BLD_LightSensor);

#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaREAD, (void*)&uiFull_Dynamic_RLD_LightSensor);
    palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaREAD, (void*)&uiEco_Dynamic_RLD_LightSensor);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaREAD, (void*)&uiFull_Dynamic_RLD_PWM);
    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaREAD, (void*)&uiEco_Dynamic_RLD_PWM);
#endif

	uiFull_BLD_LightSensor[eLD_SEQ_G] = DDP_PWM_MAX_VALUE;//1000;	//G100_Doulas_0023//G100_Doulas_0010 Add
    uiReduceLight = uiFull_BLD_LightSensor[eLD_SEQ_G]/200;

    uiTarget_BLD_LightSensor[eLD_SEQ_G] = m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G] - uiReduceLight;

    //BLD
    ulTargetLight = (UINT32)((uiTarget_BLD_LightSensor[eLD_SEQ_G]*m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_R])/m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G]);
    m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_R] = (UINT16)ulTargetLight;

    ulTargetLight = (UINT32)((uiTarget_BLD_LightSensor[eLD_SEQ_G]*m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_B])/m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G]);
    m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_B] = (UINT16)ulTargetLight;

    ulTargetLight = (UINT32)((uiTarget_BLD_LightSensor[eLD_SEQ_G]*m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_Y])/m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G]); //HICC2_Doulas_0048//HICC2_Doulas_0045
    m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_Y] = (UINT16)ulTargetLight;  //HICC2_Doulas_0045


#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    //RLD
    ulTargetLight = (UINT32)((uiTarget_BLD_LightSensor[eLD_SEQ_G]*m_sPalIllumInfo.uiABPTargetRLDLight[eLD_SEQ_R])/m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G]);
    m_sPalIllumInfo.uiABPTargetRLDLight[eLD_SEQ_R] = (UINT16)ulTargetLight;

//    ulTargetLight = (UINT32)((uiTarget_BLD_LightSensor[eLD_SEQ_Y]*m_sPalIllumInfo.uiABPTargetRLDLight[eLD_SEQ_Y])/m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_Y]);
//    m_sPalIllumInfo.uiABPTargetRLDLight[eLD_SEQ_Y] = (UINT16)ulTargetLight;
#endif

    m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G] = uiTarget_BLD_LightSensor[eLD_SEQ_G];

#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    //Calculate RLD target PWM
	for(ucCount = eLD_SEQ_R; ucCount <= eLD_SEQ_R; ucCount++)
	{
		if(util_RLD_ValueCheck(uiFull_Dynamic_RLD_PWM[ucCount],
						       uiEco_Dynamic_RLD_PWM[ucCount],
						       m_sPalIllumInfo.uiABPTargetRLDLight[ucCount],
						       uiFull_Dynamic_RLD_LightSensor[ucCount],
						       uiEco_Dynamic_RLD_LightSensor[ucCount]) == eEXEC_CODE_PASS) 	//G100_Doulas_0023 Modify
		{
       		uiTargetRLD_PWM[ucCount] = util_PositiveInterPlacement(uiFull_Dynamic_RLD_PWM[ucCount],
                                                              	   uiEco_Dynamic_RLD_PWM[ucCount],
                                                              	   m_sPalIllumInfo.uiABPTargetRLDLight[ucCount],
                                                              	   uiFull_Dynamic_RLD_LightSensor[ucCount],
                                                              	   uiEco_Dynamic_RLD_LightSensor[ucCount]);
    	}
		else
		{
			uiTargetRLD_PWM[ucCount] = uiDefaultPWM[4+ucCount];
		}
	}
	uiTargetRLD_PWM[eLD_SEQ_Y] = 1;//uiDefaultPWM[4];	//G100_Doulas_0023//G100_Doulas_0010 Modify
	//uiTargetRLD_PWM[eLD_SEQ_R] = uiDefaultPWM[5];	//G100_Doulas_0023//G100_Doulas_0010 Modify
    uiTargetRLD_PWM[eLD_SEQ_B] = 1;
    uiTargetRLD_PWM[eLD_SEQ_G] = 1;
#endif


    LOG_MSG(db_APP_ILLUMINATION, "Target BLD Light %d %d %d %d\r\n", m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_Y],
                                                                     m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_R],
                                                                     m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_B],
                                                                     m_sPalIllumInfo.uiABPTargetBLDLight[eLD_SEQ_G]);


#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    LOG_MSG(db_APP_ILLUMINATION, "Target RLD Light %d %d\r\n", m_sPalIllumInfo.uiABPTargetRLDLight[eLD_SEQ_Y],
                                                               m_sPalIllumInfo.uiABPTargetRLDLight[eLD_SEQ_R]);

    LOG_MSG(db_APP_ILLUMINATION, "Target RLD PWM %d %d\r\n", uiTargetRLD_PWM[eLD_SEQ_Y],
                                                             uiTargetRLD_PWM[eLD_SEQ_R]);
#endif

    palLDMgr_TargetBLDLightSet((UINT8 *)&m_sPalIllumInfo.uiABPTargetBLDLight);
#if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
    palLDMgr_TargetRLDLightSet((UINT8 *)&m_sPalIllumInfo.uiABPTargetRLDLight); //T100_Larry_0020
    palLDMgr_TargetRLDPWMSet((UINT8 *)&uiTargetRLD_PWM); //T100_Larry_0020
#endif
}


void palIllumination_LightSourceMode_Polling(void) //T100_Larry_0018
{
#if 1  //G100_Doulas_0008 Modify
    UINT8 ucLightSourceMode = 0;
	UINT8 ucConstantBrightness = 0;

    palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, &ucLightSourceMode);
	palDataMgr_Data_Access(edcCONSTANT_BRIGHTNESS, edaREAD, &ucConstantBrightness);

    if((ucLightSourceMode != m_sPalIllumInfo.ucLightSourceMode) ||
	   (ucConstantBrightness != m_sPalIllumInfo.ucConstantBrightness))
    {
        switch(ucLightSourceMode)
        {
            case eCM_POWER_MODE_CONSTANT_INTENSITY:
                {
					UINT8 ucFlag = 0;	//A70Gen2_Doulas_0015
					palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);	//A70Gen2_Doulas_0015

					if(ucFlag)	//A70Gen2_Doulas_0015 calibration ok
					{
                    	//MS_SLEEP(500); //A70LV_Larry_0230 //A35G2_CDS_Casper_0001
                    	palIllumination_LghtSourceMode_ConstantIntensity();
                    	palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_START); //A70LV_Larry_0179
                        m_sPalIllumInfo.ucABPAutoTuningCount = 5;               //H30K_Tim_0008, add
					}
                }
                break;

            case eCM_POWER_MODE_CUSTOM_MODE:
                {
#if defined (CUSTOM_OPTOMA)
                    if(ucConstantBrightness == ets_ON)  //A35G2_Owen_0003
                    {
                        UINT8 ucFlag = 0;
                        palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);

                        if(ucFlag)	//cal success
                        {
                            palIllumination_LghtSourceMode_ConstantIntensity();
                            palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_START);
                            m_sPalIllumInfo.ucABPAutoTuningCount = 5;           //H30K_Tim_0008, add
                        }
                    }
#endif
                }
                break;

            case eCM_POWER_MODE_RENTAL_MODE:
                {
                    UINT8 ucFlag = 0;

                    //MS_SLEEP(500); //A70LV_Larry_0230 //A35G2_CDS_Casper_0001
                    palIllumination_LghtSourceMode_RentalMode();
                    palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);

                    if(ucFlag)
                    {
                        palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_START);//A70LV_Larry_0179
                        m_sPalIllumInfo.ucRentalModeEnable = 1;
                        m_sPalIllumInfo.ucABPAutoTuningCount = 5;               //H30K_Tim_0008, add
                    }
                    else
                    {
                        m_sPalIllumInfo.ucRentalModeEnable = 0;
                    }
                }
                break;

            case eCM_POWER_MODE_CONSTANT_POWER:
            case eCM_POWER_MODE_ECO1:
            case eCM_POWER_MODE_ECO2:
            default:
                if((m_sPalIllumInfo.ucLightSourceMode == eCM_POWER_MODE_CONSTANT_INTENSITY) ||
				   (m_sPalIllumInfo.ucLightSourceMode == eCM_POWER_MODE_RENTAL_MODE) ||
				   (m_sPalIllumInfo.ucLightSourceMode == eCM_POWER_MODE_CONSTANT_POWER && m_sPalIllumInfo.ucConstantBrightness == ets_ON))
                {
                    palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_IDLE); //A70LV_Larry_0179
                //    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucLightSourceMode); //A70LV_Larry_0230
                }
                break;
        }
		m_sPalIllumInfo.ucConstantBrightness = ucConstantBrightness;
		m_sPalIllumInfo.ucLightSourceMode = ucLightSourceMode;
    }

#else
    UINT8 ucLightSourceMode = 0;

    palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, &ucLightSourceMode);

    if(ucLightSourceMode != m_sPalIllumInfo.ucLightSourceMode)
    {
        switch(ucLightSourceMode)
        {
            case eCM_POWER_MODE_CONSTANT_INTENSITY:
                {
                    MS_SLEEP(500); //A70LV_Larry_0230
                    palIllumination_LghtSourceMode_ConstantIntensity();
                    palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_START); //A70LV_Larry_0179
                    m_sPalIllumInfo.ucLightSourceMode = ucLightSourceMode;
                }
                break;

            case eCM_POWER_MODE_RENTAL_MODE:
                {
                    UINT8 ucFlag = 0;

                    MS_SLEEP(500); //A70LV_Larry_0230
                    palIllumination_LghtSourceMode_RentalMode();
                    palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);

                    if(ucFlag)
                    {
                        palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_START);//A70LV_Larry_0179
                        m_sPalIllumInfo.ucRentalModeEnable = 1;
                    }
                    else
                    {
                        m_sPalIllumInfo.ucRentalModeEnable = 0;
                    }
                    m_sPalIllumInfo.ucLightSourceMode = ucLightSourceMode;
                }
                break;

            case eCM_POWER_MODE_CONSTANT_POWER:
            case eCM_POWER_MODE_ECO1:
            case eCM_POWER_MODE_ECO2:
            default:
                if(m_sPalIllumInfo.ucLightSourceMode == eCM_POWER_MODE_CONSTANT_INTENSITY || m_sPalIllumInfo.ucLightSourceMode == eCM_POWER_MODE_RENTAL_MODE) //A70LV_Larry_0230
                {
                    palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_IDLE); //A70LV_Larry_0179
                    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucLightSourceMode); //A70LV_Larry_0230
                    m_sPalIllumInfo.ucLightSourceMode = ucLightSourceMode;
                }
                break;
        }
    }
#endif
}


void palIllumination_ABP_AutoTuning_Polling(void) //T100_Larry_0022
{
    if(m_sPalIllumInfo.ucABPAutoTuningCount == 0)
    {
        switch(m_sPalIllumInfo.ucLightSourceMode)
        {
            case eCM_POWER_MODE_RENTAL_MODE:
                if(!m_sPalIllumInfo.ucRentalModeEnable)
                {
                    m_sPalIllumInfo.ucABPAutoTuningCount = 20;
                    return;
                }
            case eCM_POWER_MODE_CONSTANT_INTENSITY:
                {
                    UINT8 ucABPAutoTuningStatus = 0;
                    if(rcSUCCESS == palLDMgr_ABP_AutoTuning_Get(&ucABPAutoTuningStatus))
                    {
                        LOG_MSG(db_APP_ILLUMINATION, "ucABPAutoTuningStatus %d\r\n", ucABPAutoTuningStatus);
                        switch(ucABPAutoTuningStatus)
                        {
                            case eABP_AUTOTUNING_STATE_OVER_RANGE:
                                break;

        	                case eABP_AUTOTUNING_STATE_SATURATED:
        	                    palIllumination_ReduceLighthandler();
                                palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_LDRELOAD);

                                m_sPalIllumInfo.ucABPAutoTuningCount = 20;
        	                    break;

                            //H30K_Tim_0008, add, ***
                            case eABP_AUTOTUNING_STATE_STOP:
                                {
                                    UINT8 ucPowerMode = eCM_POWER_MODE_CONSTANT_POWER;
                                    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPowerMode);
                                    m_sPalIllumInfo.ucABPAutoTuningCount = 20;
                                }
                                break;
                            //H30K_Tim_0008, add, &&&

        	                default:
                                m_sPalIllumInfo.ucABPAutoTuningCount = 5;
                                break;
                        }
                    }
                }
                break;

            case eCM_POWER_MODE_CUSTOM_MODE:	//G100_Doulas_0008 Add
                {
                    UINT8 ucABPAutoTuningStatus = 0;

					if(m_sPalIllumInfo.ucConstantBrightness == ets_ON)
					{
                    	if(rcSUCCESS == palLDMgr_ABP_AutoTuning_Get(&ucABPAutoTuningStatus))
                    	{
                        	LOG_MSG(db_APP_ILLUMINATION, "ucABPAutoTuningStatus %d\r\n", ucABPAutoTuningStatus);
                        	switch(ucABPAutoTuningStatus)
                        	{
                            	case eABP_AUTOTUNING_STATE_OVER_RANGE:
                                	break;

        	                	case eABP_AUTOTUNING_STATE_SATURATED:
        	                    	palIllumination_ReduceLighthandler();
                                	palLDMgr_ABP_ModeSet((UINT8)eABP_STATE_LDRELOAD);

                                	m_sPalIllumInfo.ucABPAutoTuningCount = 20;
        	                    	break;

                                //H30K_Tim_0008, add, ***
                                case eABP_AUTOTUNING_STATE_STOP:
                                    {
                                        UINT8 ucValue = ets_OFF;
                                        palDataMgr_Data_Access(edcCONSTANT_BRIGHTNESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);
                                        m_sPalIllumInfo.ucABPAutoTuningCount = 20;
                                    }
                                    break;
                                //H30K_Tim_0008, add, &&&

        	                	default:
                                	m_sPalIllumInfo.ucABPAutoTuningCount = 5;
                                	break;
                        	}
                    	}
                    }
					else
					{
						m_sPalIllumInfo.ucABPAutoTuningCount = 0xff;
					}
                }
                break;

            default:
                m_sPalIllumInfo.ucABPAutoTuningCount = 0xff;
                break;
        }
    }
    else
    {
        m_sPalIllumInfo.ucABPAutoTuningCount--;
    }
}

#if 0
void palIllumination_LightSensorValueSet(UINT16 *puiValue)
{
    m_sLightSensor.uiW = puiValue[0];
    m_sLightSensor.uiR = puiValue[1];
    m_sLightSensor.uiG = puiValue[2];
    m_sLightSensor.uiB = puiValue[3];
    m_sLightSensor.uiY = puiValue[4];

    LOG_MSG(db_APP_ILLUMINATION, "Light Sensor %d,%d,%d,%d,%d\r\n",
    m_sLightSensor.uiW,m_sLightSensor.uiR,m_sLightSensor.uiG,m_sLightSensor.uiB,m_sLightSensor.uiY);
}
#endif /* 0 */

void palIllumination_LightSensorCal_Set(UINT8 ucIndex) //T100_Larry_0018
{
    m_sPalIllumInfo.ucLightSensorCalStatus = ucIndex;

    switch(m_sPalIllumInfo.ucLightSensorCalStatus)
    {
        case eLIGHT_SENSOR_CAL_IDEL:
            //if(appGui_CurrentMenuIndexGet() == LIGHT_SENSOR_CALIBRATION_STATUS_MENU_ICOUNT) //T100_Larry_0019
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_OSD_EXIT, TRUE, NULL);
            }
            break;

        case eLIGHT_SENSOR_CAL_FULL_START:
            {
                palLDMgr_BLD_Bank_Enable(TRUE);
                palLDMgr_RLD_Bank_Enable(TRUE);
                m_sPalIllumInfo.ucABPTimer = 1;

                //palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

        case eLIGHT_SENSOR_CAL_BLD1_FULL:
            {
                UINT8 ucProcessBar = 0;

                halLDCtrl_BLD_Bank_Enable(TRUE);
                halLDCtrl_RLD_Bank_Enable(FALSE);

                m_sPalIllumInfo.ucABPTimer = 20;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

        case eLIGHT_SENSOR_CAL_BLD2_FULL:
            break;

        case eLIGHT_SENSOR_CAL_RLD1_FULL:
            {
                UINT8 ucProcessBar = 25;

                halLDCtrl_BLD_Bank_Enable(FALSE);
                halLDCtrl_RLD_Bank_Enable(TRUE);

                m_sPalIllumInfo.ucABPTimer = 10;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

        case eLIGHT_SENSOR_CAL_ECO_START:
            {
                UINT16 uiData = 1;
                UINT8 ucProcessBar = 50;

                #if defined(CUSTOM_OPTOMA) || defined(CUSTOM_BARCO) //A35G2_Coda_0113

                uiData = eCM_POWER_MODE_CUSTOM_MODE;
                palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiData);

                uiData = CONSTANT_POWER_NUMBER_MIN_VALUE;
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiData);

                #else
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uiData);
                #endif
                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);

                m_sPalIllumInfo.ucABPTimer = 1;

            }
            break;

        case eLIGHT_SENSOR_CAL_BLD1_ECO:
            {
                UINT8 ucProcessBar = 50;

                halLDCtrl_BLD_Bank_Enable(TRUE);
                halLDCtrl_RLD_Bank_Enable(FALSE);

                m_sPalIllumInfo.ucABPTimer = 20;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

        case eLIGHT_SENSOR_CAL_BLD2_ECO:
            break;

        case eLIGHT_SENSOR_CAL_RLD1_ECO:
            {
                UINT8 ucProcessBar = 75;

                halLDCtrl_BLD_Bank_Enable(FALSE);
                halLDCtrl_RLD_Bank_Enable(TRUE);

                m_sPalIllumInfo.ucABPTimer = 10;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

        case eLIGHT_SENSOR_CAL_DONE:
            {
                UINT8 ucProcessBar = 100;
                UINT8 uiData = 100;

                halLDCtrl_BLD_Bank_Enable(TRUE);
                halLDCtrl_RLD_Bank_Enable(TRUE);

                //palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaREAD, &uiData);
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &uiData); //A70LV_Larry_0201

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);

                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;

        case eLIGHT_SENSOR_CAL_SUCCESS:
            {
                UINT8 ucFlag = 1;
                UINT8 uiData = 100;

                halLDCtrl_BLD_Bank_Enable(TRUE);
                halLDCtrl_RLD_Bank_Enable(TRUE);

                //palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaREAD, &uiData);
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &uiData); //A70LV_Larry_0201

                palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucFlag);
                palDataMgr_UI_EventSend(edcUI_EVENT_ABC_RESULT, TRUE, NULL); //GuiCb.fpGui_Send_ABC_ResultCb();

				palDataMgr_OPDOperation(eOPD_PROJECTOR_LOG);
				palDataMgr_OPDSnapshot(eOPD_LIGHT_SENSOR_CALIBRATION); //A70Gen2_Julie_0047

                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SUCCESS\r\n");

                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;

        case eLIGHT_SENSOR_CAL_ERROR:
            {
                UINT8 ucFlag = 0;
                UINT16 uiData = 100;

                halLDCtrl_BLD_Bank_Enable(TRUE);
                halLDCtrl_RLD_Bank_Enable(TRUE);

                //palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaREAD, &uiData);
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &uiData); //A70LV_Larry_0201

                palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucFlag);
                palDataMgr_UI_EventSend(edcUI_EVENT_ABC_RESULT, TRUE, NULL); //GuiCb.fpGui_Send_ABC_ResultCb();

                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_ERROR\r\n");

                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_START:	//A65_OPTOMA_Doulas_0106
            {
                //halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0xFF);
                //halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0xFF);
                //halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0xFF);
                m_sPalIllumInfo.ucABPTimer = 1;
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT:	//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucProcessBar = 0;

                m_sPalIllumInfo.ucABPTimer = 3;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_T_VALUE:	//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucProcessBar = 20;

				m_sPalIllumInfo.ucABPTimer = 15;    //G100_Owen_0134
				halLDCtrl_LightSensorTrigger_Set(eLSP_STATE_CALIB);	//calib start

				UINT16 TargetValue[eLD_SEQ_NUMBER] = {0};
                utilHPBUTest_LS_TargetValue_Get(TargetValue);
				halLDCtrl_LS_TargetValue_Set((UINT8 *)TargetValue);//G100_Owen_0115 //Expected light sensor value on the cal sample time

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_BLD_VALUE:	//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucProcessBar = 50;

                m_sPalIllumInfo.ucABPTimer = 5;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_RLD_VALUE:	//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucProcessBar = 80;

				halLDCtrl_LS_RLD_Enable_Set(FALSE);
                m_sPalIllumInfo.ucABPTimer = 5;

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_DONE:	//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucProcessBar = 100;;

                halLDCtrl_LS_RLD_Enable_Set(TRUE);

                palDataMgr_Data_Access(edcLIGHTSENSOPROCESS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucProcessBar);
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);

                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;

        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_SUCCESS:		//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucFlag = 1;
                UINT8 uiData = 100;


				halLDCtrl_LS_RLD_Enable_Set(TRUE);

                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &uiData);

                palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucFlag);
                //appGui_Send_ABC_Result();  //H2 wait review
                palDataMgr_UI_EventSend(edcUI_EVENT_ABC_RESULT, TRUE, NULL);

				palDataMgr_OPDOperation(eOPD_PROJECTOR_LOG);
				palDataMgr_OPDSnapshot(eOPD_LIGHT_SENSOR_CALIBRATION); //A70Gen2_Julie_0047

                //LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SAMPLE_TIME_SUCCESS\r\n");

                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;

        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR:		//A65_OPTOMA_Doulas_0106
            {
                UINT8 ucFlag = 0;
                UINT16 uiData = 100;


                halLDCtrl_LS_RLD_Enable_Set(TRUE);

                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, &uiData);

                palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaWRITE_THROUGH_NO_ACTION, (void*)&ucFlag);
                //appGui_Send_ABC_Result();  //H2 wait review

                //LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR\r\n");

                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;
        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_COLLECT_INFO: //HICC2_Jacky_0003
            {
                m_sPalIllumInfo.ucABPTimer = 5;
            }
            break;

        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT_CHANGE_POS: //HICC2_Jacky_0003
            {
                m_sPalIllumInfo.ucABPTimer = 3;
            }
            break;


        default:
            break;
    }
}

void palIllumination_LightSensorCal_Process(void) //T100_Larry_0018
{
    static UINT8 uc3D_ENABLE = eCM_3D_FORMAT_OFF;
    static UINT8 ucCONTRAST_ENHANCEMENT = 0;
    static UINT8 ucHIGH_ALTITUDE = 0;
    static UINT8 ucPICTURE_SETTINGS = eCM_PICTURE_SETTINGS_BRIGHT;
    static UINT8 ucPOWER_MODE = eCM_POWER_MODE_CONSTANT_POWER;
    static UINT8 ucCONSTANT_POWER_NUMBER = 100;

    if(m_sPalIllumInfo.ucABPTimer != 0)
    {
        m_sPalIllumInfo.ucABPTimer--;
        return;
    }

    switch(m_sPalIllumInfo.ucLightSensorCalStatus)
    {
        case eLIGHT_SENSOR_CAL_IDEL:
            palIllumination_ABP_StatusSet(eABP_STATUS_IDEL);
            break;

        case eLIGHT_SENSOR_CAL_FULL_START:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                UINT8  ucData = 0;
                sLIGHT_SENSOR sLightSenosr;

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                uiData[eLD_SEQ_R] = sLightSenosr.uiR;
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_Y; ucCount < eLD_SEQ_NUMBER; ucCount++)
                {
                    if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                    {
                        ucError |= 1;
                    }
                }

                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ERROR);
                }
                else
                {

                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_BLD1_FULL);

                    ucData = eCM_3D_FORMAT_OFF;
                    palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, (void*)&uc3D_ENABLE);
                    palDataMgr_Data_Access(edc3D_ENABLE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);

                    ucData = 0;
                    palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaREAD, (void*)&ucCONTRAST_ENHANCEMENT);
                    palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);
                    palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaREAD, (void*)&ucHIGH_ALTITUDE);
                    palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData); //T100_Larry_0019

                    ucData = eCM_PICTURE_SETTINGS_BRIGHT;
                    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, (void*)&ucPICTURE_SETTINGS);
                    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);

                    ucData = eCM_POWER_MODE_CONSTANT_POWER;
                    palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, (void*)&ucPOWER_MODE);
                    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);

                    ucData = 100;
                    palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaREAD, (void*)&ucCONSTANT_POWER_NUMBER);
                    palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucData);
                }
            }
            break;

        case eLIGHT_SENSOR_CAL_BLD1_FULL:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;

                LOG_MSG(db_APP_ILLUMINATION, "Full BLD1\r\n");

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                uiData[eLD_SEQ_R] = sLightSenosr.uiR;
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_Y; ucCount < eLD_SEQ_NUMBER; ucCount++)
                {
                    if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                    {
                        ucError |= 1;
                    }
                }

                halFormatter_PWM_Update();

                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ERROR);
                }
                else
                {
                    //palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_RLD1_FULL);
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ECO_START);
                }

                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_BLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                uiData[eLD_SEQ_Y] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_Y);
                uiData[eLD_SEQ_R] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_R);
                uiData[eLD_SEQ_B] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_B);
                uiData[eLD_SEQ_G] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_G);

                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_BLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

            }
            break;

        case eLIGHT_SENSOR_CAL_BLD2_FULL:
            break;

        case eLIGHT_SENSOR_CAL_RLD1_FULL:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;

                LOG_MSG(db_APP_ILLUMINATION, "Full RLD1\r\n");

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                uiData[eLD_SEQ_R] = sLightSenosr.uiR;
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_Y; ucCount <= eLD_SEQ_R; ucCount++)
                {
                    if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                    {
                        ucError |= 1;
                    }
                }

                halFormatter_PWM_Update();

                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ERROR);
                }
                else
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ECO_START);
                }


                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                uiData[eLD_SEQ_Y] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_Y);
                uiData[eLD_SEQ_R] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_R);
                uiData[eLD_SEQ_B] = 1;
                uiData[eLD_SEQ_G] = 1;

                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
            }
            break;

        case eLIGHT_SENSOR_CAL_ECO_START:
           {
               palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_BLD1_ECO);
           }
           break;

        case eLIGHT_SENSOR_CAL_BLD1_ECO:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;

                LOG_MSG(db_APP_ILLUMINATION, "Eco BLD1\r\n");

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                uiData[eLD_SEQ_R] = sLightSenosr.uiR;
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_Y; ucCount < eLD_SEQ_NUMBER; ucCount++)
                {
                   if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                   {
                       ucError |= 1;
                   }
                }

                halFormatter_PWM_Update();

                if(ucError)
                {
                   palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ERROR);
                }
                else
                {
                   //palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_RLD1_ECO);
                   palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_DONE);
                }

                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_BLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                uiData[eLD_SEQ_Y] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_Y);
                uiData[eLD_SEQ_R] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_R);
                uiData[eLD_SEQ_B] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_B);
                uiData[eLD_SEQ_G] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_G);

                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_BLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

            }
            break;

        case eLIGHT_SENSOR_CAL_BLD2_ECO:
            break;

        case eLIGHT_SENSOR_CAL_RLD1_ECO:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;

                LOG_MSG(db_APP_ILLUMINATION, "Eco RLD1\r\n");

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                uiData[eLD_SEQ_R] = sLightSenosr.uiR;
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_Y; ucCount <= eLD_SEQ_R; ucCount++)
                {
                   if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                   {
                       ucError |= 1;
                   }
                }

                halFormatter_PWM_Update();

                if(ucError)
                {
                   palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_ERROR);
                }
                else
                {
                   palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_DONE);
                }

                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                uiData[eLD_SEQ_Y] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_Y);
                uiData[eLD_SEQ_R] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_R);
                uiData[eLD_SEQ_B] = 1;
                uiData[eLD_SEQ_G] = 1;

                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
            }
            break;

        case eLIGHT_SENSOR_CAL_DONE:
            {
                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SUCCESS);
            }
            break;

        case eLIGHT_SENSOR_CAL_SUCCESS:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};

                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_IDEL);

                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "FULL_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM_FULL_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "FULL_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM FULL_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "ECO_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM ECO_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "ECO_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM ECO_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucCONSTANT_POWER_NUMBER);
                palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPOWER_MODE);
                palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucHIGH_ALTITUDE);
                palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucCONTRAST_ENHANCEMENT);
                palDataMgr_Data_Access(edc3D_ENABLE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uc3D_ENABLE);
                palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPICTURE_SETTINGS);

                LOG_MSG(db_APP_ILLUMINATION, "Light sensor cal Success\r\n");
            }
            break;

        case eLIGHT_SENSOR_CAL_ERROR:
            {

                UINT16 uiData[eLD_SEQ_NUMBER] = {0};

                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_IDEL);

                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "FULL_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM_FULL_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "FULL_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_FULL_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM FULL_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "ECO_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM ECO_BLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "ECO_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_ECO_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "PWM ECO_RLD %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucCONSTANT_POWER_NUMBER);
                palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPOWER_MODE);
                palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucHIGH_ALTITUDE);
                palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucCONTRAST_ENHANCEMENT);
                palDataMgr_Data_Access(edc3D_ENABLE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&uc3D_ENABLE);
                palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPICTURE_SETTINGS);

                LOG_MSG(db_APP_ILLUMINATION, "Light sensor cal Error\r\n");
            }
            break;

			case eLIGHT_SENSOR_CAL_SAMPLE_TIME_START:	//A65_OPTOMA_Doulas_0106
            {
#if 0   //G100_Owen_0138
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                UINT8  ucData = 0;
                sLIGHT_SENSOR sLightSenosr;

                palIllumination_AbpLightSensor(&sLightSenosr);

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                uiData[eLD_SEQ_R] = sLightSenosr.uiR;
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_Y; ucCount < eLD_SEQ_NUMBER; ucCount++)
                {
                    if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                    {
                        ucError |= 1;
                    }
                }

                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR);
                }
                else
#else
                UINT8  ucData = 0;
#endif
                {
                	palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT);
                    ucData = 0;
//                    palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);  //A70G2_Owen_0002 remove
                    palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);
                    palDataMgr_Data_Access(edcHIGH_ALTITUDE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);

                    ucData = eCM_PICTURE_SETTINGS_BRIGHT;
                    palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);

                    ucData = ets_OFF;
                    palDataMgr_Data_Access(edcCONSTANT_BRIGHTNESS, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);

                    ucData = eCM_POWER_MODE_CONSTANT_POWER;
                    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);

                    ucData = 100;
                    palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);

                }
            }
            break;

			case eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT:		//A65_OPTOMA_Doulas_0106
			{
                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT\r\n");

				palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_T_VALUE);
            }
			break;

			case eLIGHT_SENSOR_CAL_SAMPLE_TIME_T_VALUE:		//A65_OPTOMA_Doulas_0106
            {
                float fGain[eLD_SEQ_NUMBER] = {0.0f};   //G100_Owen_0138
                UINT16 auiOffset[eLD_SEQ_NUMBER] = {0}; //G100_Owen_0138
                UINT8 aucTime[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                UINT8  ucLS_Cal_Stautus = eLSP_STATE_IDLE;

                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SAMPLE_TIME_T_VALUE\r\n");

                halLDCtrl_LightSensorT0_Get(aucTime);	//0~255
                halLDCtrl_LightSensorTrigger_Get(&ucLS_Cal_Stautus); //LS status
                halLDCtrl_LightSensorOffset_Get((UINT8 *)auiOffset);    //G100_Owen_0138
                halLDCtrl_LightSensorGain_Get((UINT8 *)fGain);  //G100_Owen_0138

                for(ucCount = eLD_SEQ_Y; ucCount < eLD_SEQ_NUMBER; ucCount++) //HICC2_Doulas_0045
                {
                    if(ABC_LIGHT_SENSOR_SAMPLE_TIME_T_MINIMUM > aucTime[ucCount])
                    {
                        ucError |= 1;
                    }
                }

				if(ucLS_Cal_Stautus == eLSP_STATE_CALIB)
				{
					ucError |= 1;
				}

				LOG_MSG(db_APP_ILLUMINATION, "LS Cal T (%d,%d,%d,%d)\r\n",aucTime[0],aucTime[1],aucTime[2],aucTime[3]);
                LOG_MSG(db_APP_ILLUMINATION, "LS Cal Offset (%d,%d,%d,%d)\r\n",auiOffset[0],auiOffset[1],auiOffset[2],auiOffset[3]);    //G100_Owen_0138
                LOG_MSG(db_APP_ILLUMINATION, "LS Cal Gain (%f,%f,%f,%f)\r\n",fGain[0],fGain[1],fGain[2],fGain[3]);  //G100_Owen_0138
				LOG_MSG(db_APP_ILLUMINATION, "LS Cal Status %d\r\n",ucLS_Cal_Stautus);

                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR);
                }
                else
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_BLD_VALUE);
                }
                UINT8 ucPosition = 0; //HICC2_Jacky_0003 start

                palDataMgr_Data_Access(edcLIGHTSENSOR_POSITION, edaREAD, (void*)&ucPosition);
                if(ucPosition)
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_TIME_SECOND, edaWRITE_THROUGH_NO_ACTION, (void*)aucTime);
                    palDataMgr_Data_Access(edcLIGHTSENSOR_OFFSET_SECOND, edaWRITE_THROUGH_NO_ACTION, (void*)auiOffset);
                    palDataMgr_Data_Access(edcLIGHTSENSOR_GAIN_SECOND, edaWRITE_THROUGH_NO_ACTION, (void*)fGain);
                }
                else
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_TIME, edaWRITE_THROUGH_NO_ACTION, (void*)aucTime);     //A70Gen2_Larry_0008
                    palDataMgr_Data_Access(edcLIGHTSENSOR_OFFSET, edaWRITE_THROUGH_NO_ACTION, (void*)auiOffset); //A70Gen2_Larry_0008
                    palDataMgr_Data_Access(edcLIGHTSENSOR_GAIN, edaWRITE_THROUGH_NO_ACTION, (void*)fGain);       //A70Gen2_Larry_0008
                }//HICC2_Jacky_0003 end
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_BLD_VALUE:	//A65_OPTOMA_Doulas_0106
            {
				UINT16 auiADC[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;

                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SAMPLE_TIME_BLD_VALUE\r\n");


    			halLDCtrl_T0LightSensorValue_Get(eLDBANK_A70LV, (UINT8 *)auiADC);
//LOG_MSG(db_APP_ILLUMINATION, "auiBLD (%d,%d,%d,%d)\r\n",auiADC[0],auiADC[1],auiADC[2],auiADC[3]); //HICC2_Doulas_0007

                for(ucCount = eLD_SEQ_Y; ucCount < eLD_SEQ_NUMBER; ucCount++) //HICC2_Doulas_0045
                {
                    if(ABC_LIGHT_SENSOR_CAL_BLD_MINIMUM > auiADC[ucCount] || ABC_LIGHT_SENSOR_CAL_BLD_MAXIMUM < auiADC[ucCount])
                    {
                        ucError |= 1;
                    }
                }


                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR);
                }
                else
                {
                    UINT8 ucPosition = 0; //HICC2_Jacky_0003 start
                    palDataMgr_Data_Access(edcLIGHTSENSOR_POSITION, edaREAD, (void*)&ucPosition);
                    if(ucPosition)
                    {
                        palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_FACTORY_SECOND, edaWRITE_THROUGH_NO_ACTION, (void*)auiADC);
                    }
                    else
                    {
                        palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_FACTORY, edaWRITE_THROUGH_NO_ACTION, (void*)auiADC);
                    }//HICC2_Jacky_0003 end
#if 1   //A70G2_Owen_0002
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_RLD_VALUE);
#else
                    switch(palDataMgr_Model_ID_Get())   //G100_Owen_0115
                    {
                        case eG100_MODEL_W22:
                        case eG100_MODEL_W22_3D:
                            palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_RLD_VALUE);
                            break;

                        case eG100_MODEL_W19:
                        case eG100_MODEL_W19_3D:
                        case eG100_MODEL_W16:
                        case eG100_MODEL_W16_3D:
                            palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_DONE);
                            break;

                        default:
                            palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR);
                            break;
                    }
#endif
                }
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_RLD_VALUE:	//A65_OPTOMA_Doulas_0106 //A70G2_Owen_0002
            {
				UINT16 auiADC[eLD_SEQ_NUMBER] = {0};
    			UINT16 auiRLD[eLD_SEQ_NUMBER] = {0};
                //UINT8  ucCount = 0;
                UINT8  ucError = 0;
                INT32 wLightSensorInfoType = 0; //HICC2_Doulas_0037

                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_CAL_SAMPLE_TIME_RLD_VALUE\r\n");

                palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_FACTORY, edaREAD, (void*)auiRLD); //A70Gen2_Larry_0008
    			halLDCtrl_T0LightSensorValue_Get(eLDBANK_A70LV, (UINT8 *)auiADC);
    			halLDCtrl_LS_RLD_Enable_Set(TRUE);

    			Syscfg_Value_Get(eLightSensorSegmentType, &wLightSensorInfoType); //HICC2_Doulas_0037
//LOG_MSG(db_APP_ILLUMINATION, "auiRLD (%d,%d,%d,%d)\r\n",auiRLD[0],auiRLD[1],auiRLD[2],auiRLD[3]); //HICC2_Doulas_0007
//LOG_MSG(db_APP_ILLUMINATION, "auiADC (%d,%d,%d,%d)\r\n",auiADC[0],auiADC[1],auiADC[2],auiADC[3]); //HICC2_Doulas_0007

                if(wLightSensorInfoType == eDC_LIGHT_SENSOR_SEGMENT_TYPE_0)   //HICC2_Doulas_0037 //R/G/B/RR
                {
    			    //auiRLD[eLD_SEQ_Y] -= auiADC[eLD_SEQ_Y];
    			    auiRLD[eLD_SEQ_R] -= auiADC[eLD_SEQ_R];

                    if(ABC_LIGHT_SENSOR_CAL_RLD_MINIMUM > auiRLD[eLD_SEQ_R] || ABC_LIGHT_SENSOR_CAL_RLD_MAXIMUM < auiRLD[eLD_SEQ_R])
                    {
                        ucError |= 1;
                    }
                }
                else  //R/G/B/Y ,no RLD
                {
                    auiRLD[eLD_SEQ_R] = 0;
                }

                if(ucError)
                {
                    palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR);
                }
                else
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)auiRLD); //A70Gen2_Larry_0008

                    UINT8 ucPosition = 0; //HICC2_Jacky_0003

                    palDataMgr_Data_Access(edcLIGHTSENSOR_POSITION, edaREAD, (void*)&ucPosition);
                    if(ucPosition)
                    {
                        palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_COLLECT_INFO);
                    }
                    else
                    {
                        palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_DONE);
                    }//HICC2_Jacky_0003 end
                }
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_DONE:	//A65_OPTOMA_Doulas_0106
            {
                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_SUCCESS);
            }
            break;

        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_SUCCESS:		//A65_OPTOMA_Doulas_0106
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
				UINT8 aucTime[eLD_SEQ_NUMBER] = {0};

                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_IDEL);

                palDataMgr_Data_Access(edcLIGHTSENSOR_TIME, edaREAD, (void*)aucTime);
				LOG_MSG(db_APP_ILLUMINATION, "Sample Time %d %d %d %d\r\n",aucTime[0], aucTime[1], aucTime[2], aucTime[3]);

                palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_FACTORY, edaREAD, (void*)uiData);
				LOG_MSG(db_APP_ILLUMINATION, "BLD Sensor %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_RLD, edaREAD, (void*)uiData);
				LOG_MSG(db_APP_ILLUMINATION, "RLD Sensor %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);


                LOG_MSG(db_APP_ILLUMINATION, "Light sensor sample time cal Success\r\n");
            }
            break;

		case eLIGHT_SENSOR_CAL_SAMPLE_TIME_ERROR:	//A65_OPTOMA_Doulas_0106
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
				UINT8 aucTime[eLD_SEQ_NUMBER] = {0};

                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_IDEL);

                palDataMgr_Data_Access(edcLIGHTSENSOR_TIME, edaREAD, (void*)aucTime);
				LOG_MSG(db_APP_ILLUMINATION, "Sample Time %d %d %d %d\r\n",aucTime[0], aucTime[1], aucTime[2], aucTime[3]);

                palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_FACTORY, edaREAD, (void*)uiData);
				LOG_MSG(db_APP_ILLUMINATION, "BLD Sensor %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                palDataMgr_Data_Access(edcLIGHTSENSOR_TARGET_RLD, edaREAD, (void*)uiData);
				LOG_MSG(db_APP_ILLUMINATION, "RLD Sensor %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3]);

                LOG_MSG(db_APP_ILLUMINATION, "Light sensor sample time cal Error\r\n");
            }
            break;

        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_COLLECT_INFO: //HICC2_Jacky_0003
            {
                UINT8  ucPosition = 0;
                sLIGHT_SENSOR sLightSensor;
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};

                palIllumination_AbpLightSensor(&sLightSensor);

                uiData[eLD_SEQ_Y] = sLightSensor.uiY;
                uiData[eLD_SEQ_R] = sLightSensor.uiR;
                uiData[eLD_SEQ_B] = sLightSensor.uiB;
                uiData[eLD_SEQ_G] = sLightSensor.uiG;

                palDataMgr_Data_Access(edcLIGHT_SENSOR_INFO, edaWRITE_THROUGH_WITH_ACTION,(void*) &uiData);

                palDataMgr_Data_Access(edcLIGHTSENSOR_POSITION, edaWRITE_THROUGH_WITH_ACTION,(void*) &ucPosition);

                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT_CHANGE_POS);
            }
            break;

        case eLIGHT_SENSOR_CAL_SAMPLE_TIME_WAIT_CHANGE_POS: //HICC2_Jacky_0003
            {
                palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_START);
            }
            break;

        default:
            break;
    }
}

#if 0
void palIllumination_LightSensor_RLDDynamic_Cal_Set(UINT8 ucIndex) //T100_Larry_0019
{
    m_sPalIllumInfo.ucLightSensorRLDCalStatus = ucIndex;

    switch(m_sPalIllumInfo.ucLightSensorRLDCalStatus)
    {
        case eLIGHT_SENSOR_RLD_CAL_IDEL:
            m_sPalIllumInfo.ucABPTimer = 0;
            if(m_sPalIllumInfo.ucABPStatus == eABP_STATUS_DYNAMIC_RLD_CAL_SHUTDOWN)
            {
                m_sPalIllumInfo.bFirstLightSourceOn = FALSE;
                palSystem_PowerDown();
            }
            break;

        case eLIGHT_SENSOR_RLD_CAL_START: //A70LV_Larry_0363 modify
            {
                UINT8 ucData = 0;
                #if 1   //ZU860_Doulas_0006 modify
                palDataMgr_Data_Access(edcREAL_BLACK, edaREAD, (void*)&ucData);
                if(ucData != 0)
                {
                    ucData = 0;
                    palDataMgr_Data_Access(edcREAL_BLACK, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);
                }

                palDataMgr_Data_Access(edcDYNAMIC_BLACK, edaREAD, (void*)&ucData);
                if(ucData != 0)
                {
                    ucData = 0;
                    palDataMgr_Data_Access(edcDYNAMIC_BLACK, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);
                }
				halFormatter_PWM_Update();		//G100_Doulas_0023
				halFormatter_RLD_PWM_Set(700,700); 	//G100_Doulas_0023
                #else
                palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaREAD, (void*)&ucData);

                if(ucData != 0)
                {
                    ucData = 0;
                    palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);
                }
                #endif
            }
            m_sPalIllumInfo.ucABPTimer = 2;
            break;

        case eLIGHT_SENSOR_RLD_CAL_FULL:
            //halLDCtrl_BLD_Bank_Enable(FALSE);
            //halLDCtrl_RLD_Bank_Enable(TRUE);
            #if	1	//G100_Doulas_0023 Modify
			halFormatter_RLD_PWM_Set(700,700);
			halLDCtrl_LS_RLD_Enable_Set(FALSE);
			#else
            //halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0xFF); //A70LV_Larry_0179
            halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0x00); //A70LV_Larry_0179
            halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0x00); //A70LV_Larry_0179
            halFormatter_RLD_PWM_Set(800); //A70LV_Larry_0363
            #endif
            m_sPalIllumInfo.ucABPTimer = 2;
            break;

		case eLIGHT_SENSOR_RLD_CAL_ECO_START:	//G100_Doulas_0023 Modify
			halLDCtrl_LS_RLD_Enable_Set(TRUE);
			halFormatter_RLD_PWM_Set(400,400);
            m_sPalIllumInfo.ucABPTimer = 2;
            break;

        case eLIGHT_SENSOR_RLD_CAL_ECO:
			#if 1	//G100_Doulas_0023 Modify
			halLDCtrl_LS_RLD_Enable_Set(FALSE);
			#else
            halFormatter_RLD_PWM_Set(500); //A70LV_Larry_0363
            #endif
            m_sPalIllumInfo.ucABPTimer = 2;
            break;

        case eLIGHT_SENSOR_RLD_CAL_DONE:
			halLDCtrl_LS_RLD_Enable_Set(TRUE);						//G100_Doulas_0023
            if(m_sPalIllumInfo.ucABPStatus == eABP_STATUS_DYNAMIC_RLD_CAL_SHUTDOWN)
            {
                halLDCtrl_BLD_Bank_Enable(FALSE);
                halLDCtrl_RLD_Bank_Enable(FALSE);
            }
            else
            {
                halLDCtrl_BLD_Bank_Enable(TRUE);
                halLDCtrl_RLD_Bank_Enable(TRUE);
            }
			m_sPalIllumInfo.ucABPTimer = 0;
            break;
    }
}

void palIllumination_LightSensor_RLDDynamic_Cal_Process(void) //T100_Larry_0019
{
    if(m_sPalIllumInfo.ucABPTimer != 0)
    {
        m_sPalIllumInfo.ucABPTimer--;
        return;
    }

    switch(m_sPalIllumInfo.ucLightSensorRLDCalStatus)
    {
        case eLIGHT_SENSOR_RLD_CAL_IDEL:
            palIllumination_ABP_StatusSet(eABP_STATUS_IDEL);
            break;

        case eLIGHT_SENSOR_RLD_CAL_START:
            {
                UINT8  ucError = 0;
                //UINT8  ucData = 0;
                sLIGHT_SENSOR sLightSenosr;

                palIllumination_AbpLightSensor(&sLightSenosr_RLD_ON);	//G100_Doulas_0023

                LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_RLD_CAL_START \r\n"); //G100_Doulas_0023

                if(//(ABC_LIGHT_SENSOR_MINIMUM > sLightSenosr_RLD_ON.uiY || ABC_LIGHT_SENSOR_MAXIMUM < sLightSenosr_RLD_ON.uiY) || //Y seg
                   (ABC_LIGHT_SENSOR_MINIMUM > sLightSenosr_RLD_ON.uiR || ABC_LIGHT_SENSOR_MAXIMUM < sLightSenosr_RLD_ON.uiR))	//G100_Doulas_0023 Modify
                {
                    LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_RLD_CAL_START Error\r\n");
                    ucError |= 1;
                }

                if(ucError)
                {
                    palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_DONE);
                }
                else
                {
                    //ucData = 0;

                    //palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);


                    palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_FULL);
                }
            }

            break;

        case eLIGHT_SENSOR_RLD_CAL_FULL:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;
				UINT16 uiRLD_Light[2];	//G100_Doulas_0023

				halLDCtrl_LS_RLD_Enable_Set(TRUE);	//G100_Doulas_0023
                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;//sLightSenosr_RLD_ON.uiY - sLightSenosr.uiY;		//G100_Doulas_0023
                uiData[eLD_SEQ_R] = sLightSenosr_RLD_ON.uiR - sLightSenosr.uiR;		//G100_Doulas_0023
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

               // if((ABC_LIGHT_SENSOR_MINIMUM > uiData[eLD_SEQ_Y] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_Y]) || //Y seg
               //    (ABC_LIGHT_SENSOR_MINIMUM > uiData[eLD_SEQ_R] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_R]) || //R seg
               //    (ABC_LIGHT_SENSOR_MINIMUM < uiData[eLD_SEQ_B]) || //B Seg
               //    (ABC_LIGHT_SENSOR_MINIMUM < uiData[eLD_SEQ_G])    //G Seg
               //    )
                if(//(ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_Y]) || //Y seg
                   (ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_R])) //R seg   //G100_Doulas_0023 Modify
                {
                    ucError |= 1;
                }

                halFormatter_PWM_Update();
				halFormatter_GetRLD_Light(uiRLD_Light);		//G100_Doulas_0023
				//LOG_MSG(db_ALWAYS, "RLD PWM1 Get %d,%d\r\n",uiRLD_Light[0],uiRLD_Light[1]); //G100_Doulas_0023

                if(ucError)
                {
                    palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_DONE);
                }
                else
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                    uiData[eLD_SEQ_Y] = 1;//uiRLD_Light[1];//palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_Y);	//G100_Doulas_0023 Modify
                    uiData[eLD_SEQ_R] = uiRLD_Light[0];//palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_R);	//G100_Doulas_0023 Modify
                    uiData[eLD_SEQ_B] = 1;
                    uiData[eLD_SEQ_G] = 1;

                    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);



                    palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_ECO_START);	//G100_Doulas_0023
                }
            }
            break;

		case eLIGHT_SENSOR_RLD_CAL_ECO_START:	//G100_Doulas_0023
            {
                UINT8  ucError = 0;

                palIllumination_AbpLightSensor(&sLightSenosr_RLD_ON);


                if(//(ABC_LIGHT_SENSOR_MINIMUM > sLightSenosr_RLD_ON.uiY || ABC_LIGHT_SENSOR_MAXIMUM < sLightSenosr_RLD_ON.uiY) || //Y seg
                   (ABC_LIGHT_SENSOR_MINIMUM > sLightSenosr_RLD_ON.uiR || ABC_LIGHT_SENSOR_MAXIMUM < sLightSenosr_RLD_ON.uiR))
                {
                    LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_RLD_CAL_ECO_START Error\r\n");
                    ucError |= 1;
                }

                if(ucError)
                {
                    palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_DONE);
                }
                else
                {
                    palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_ECO);
                }
            }
            break;

        case eLIGHT_SENSOR_RLD_CAL_ECO:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;
				UINT16 uiRLD_Light[2];						//G100_Doulas_0023

				UINT16 uiRLD_PWM_F[eLD_SEQ_NUMBER];				//G100_Doulas_0023
				UINT16 uiRLD_Sensor_F[eLD_SEQ_NUMBER] = {0};	//G100_Doulas_0023
				UINT16 uiRLD_PWM_E[eLD_SEQ_NUMBER];				//G100_Doulas_0023
				UINT16 uiRLD_Sensor_E[eLD_SEQ_NUMBER] = {0};	//G100_Doulas_0023
				UINT16 uiRLD_PWM_T[8];							//G100_Doulas_0023
				UINT16 uiRLD_Sensor_T[eLD_SEQ_NUMBER] = {0};	//G100_Doulas_0023

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[eLD_SEQ_Y] = sLightSenosr.uiY;//sLightSenosr_RLD_ON.uiY - sLightSenosr.uiY;	//G100_Doulas_0023
                uiData[eLD_SEQ_R] = sLightSenosr_RLD_ON.uiR - sLightSenosr.uiR;	//G100_Doulas_0023
                uiData[eLD_SEQ_B] = sLightSenosr.uiB;
                uiData[eLD_SEQ_G] = sLightSenosr.uiG;

                //if((ABC_LIGHT_SENSOR_MINIMUM > uiData[eLD_SEQ_Y] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_Y]) || //Y seg
                //   (ABC_LIGHT_SENSOR_MINIMUM > uiData[eLD_SEQ_R] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_R]) || //R seg
                //   (ABC_LIGHT_SENSOR_MINIMUM < uiData[eLD_SEQ_B]) || //B Seg
                //   (ABC_LIGHT_SENSOR_MINIMUM < uiData[eLD_SEQ_G])    //G Seg
                //   )
                if(//(ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_Y]) || //Y seg
                   (ABC_LIGHT_SENSOR_MAXIMUM < uiData[eLD_SEQ_R])) //R seg	//G100_Doulas_0023 Modify
                {
                    ucError |= 1;
                }

                halFormatter_PWM_Update();
				halFormatter_GetRLD_Light(uiRLD_Light);		//G100_Doulas_0023
				//LOG_MSG(db_ALWAYS, "RLD PWM2 Get %d,%d\r\n",uiRLD_Light[0],uiRLD_Light[1]); //G100_Doulas_0023

                if(!ucError)
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                    uiData[eLD_SEQ_Y] = 1;//uiRLD_Light[1];//palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_Y);	//G100_Doulas_0023 Modify
                    uiData[eLD_SEQ_R] = uiRLD_Light[0];//palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_R);	//G100_Doulas_0023 Modify
                    uiData[eLD_SEQ_B] = 1;
                    uiData[eLD_SEQ_G] = 1;

                    palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);
                }

                palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_DONE);
				#if 1	//G100_Doulas_0023 debug
				palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaREAD, (void*)&uiRLD_Sensor_F);
				palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaREAD, (void*)&uiRLD_PWM_F);

				palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaREAD, (void*)&uiRLD_Sensor_E);
				palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaREAD, (void*)&uiRLD_PWM_E);

				palDataMgr_Data_Access(edcCONSTANT_INTENSITY_PWM, edaREAD, (void*)&uiRLD_PWM_T);
				palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaREAD, (void*)&uiRLD_Sensor_T);

				LOG_MSG(db_APP_ILLUMINATION, "PWM1(%d,%d) Sensor(%d,%d)\r\n",uiRLD_PWM_F[eLD_SEQ_R],uiRLD_PWM_F[eLD_SEQ_Y],
																   			 uiRLD_Sensor_F[eLD_SEQ_R],uiRLD_Sensor_F[eLD_SEQ_Y]); //G100_Doulas_0023

				LOG_MSG(db_APP_ILLUMINATION, "PWM2(%d,%d) Sensor(%d,%d)\r\n",uiRLD_PWM_E[eLD_SEQ_R],uiRLD_PWM_E[eLD_SEQ_Y],
																   			 uiRLD_Sensor_E[eLD_SEQ_R],uiRLD_Sensor_E[eLD_SEQ_Y]); //G100_Doulas_0023



				LOG_MSG(db_APP_ILLUMINATION, "PWMx(%d,%d) Sensor(%d,%d)\r\n",uiRLD_PWM_T[4+eLD_SEQ_R],uiRLD_PWM_T[4+eLD_SEQ_Y],
																   			 uiRLD_Sensor_T[eLD_SEQ_R],uiRLD_Sensor_T[eLD_SEQ_Y]); //G100_Doulas_0023
				UINT8 ucCount;
				UINT16 uiTargetRLD_PWM[eLD_SEQ_NUMBER] = {0};
				for(ucCount = eLD_SEQ_Y; ucCount <= eLD_SEQ_R; ucCount++)
    			{
       				uiTargetRLD_PWM[ucCount] = util_PositiveInterPlacement(uiRLD_PWM_F[ucCount],
                                                              uiRLD_PWM_E[ucCount],
                                                              uiRLD_Sensor_T[ucCount],
                                                              uiRLD_Sensor_F[ucCount],
                                                              uiRLD_Sensor_E[ucCount]);
    			}
				LOG_MSG(db_APP_ILLUMINATION, "Target PWM(%d,%d) \r\n",uiTargetRLD_PWM[eLD_SEQ_R],uiTargetRLD_PWM[eLD_SEQ_Y]); //G100_Doulas_0023
				#endif
            }
            break;

        case eLIGHT_SENSOR_RLD_CAL_DONE:
            {
                UINT8  ucData = 0;
                UINT8  ucCurrent = 0;

                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaREAD, (void*)&ucCurrent);

                if(ucCurrent == 1)
                {
                    ucData = 2;
                }
                else
                {
                    ucData = (ucCurrent-1);
                }
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucData);
                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucCurrent);
                palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_IDEL);
				LOG_MSG(db_APP_ILLUMINATION, "eLIGHT_SENSOR_RLD_CAL_DONE \r\n"); //G100_Doulas_0023
            }
            break;
    }
}
#endif /* 0 */

void palIllumination_LightSensor_ConstantIntensity_Set(UINT8 ucIndex) //T100_Larry_0019
{
    m_sPalIllumInfo.ucLightSensorConstantIntensity = ucIndex;

#if 0
    char *psString[] = {
        STRINGER(eLIGHT_SENSOR_INTESSITY_IDEL),
        STRINGER(eLIGHT_SENSOR_INTESSITY_START),
        STRINGER(eLIGHT_SENSOR_INTESSITY_BLD),
        STRINGER(eLIGHT_SENSOR_INTESSITY_RLD),
        STRINGER(eLIGHT_SENSOR_INTESSITY_DONE),
        STRINGER(eLIGHT_SENSOR_INTESSITY_ERROR)
    };

    LOG_MSG(db_APP_ILLUMINATION, "(%s, %d)ABP %s\r\n", __FUNCTION__, __LINE__, psString[ucIndex]);
#endif

    switch(m_sPalIllumInfo.ucLightSensorConstantIntensity)
    {
        case eLIGHT_SENSOR_INTESSITY_IDEL:
            m_sPalIllumInfo.ucABPTimer = 0;
            break;

        case eLIGHT_SENSOR_INTESSITY_START:
            m_sPalIllumInfo.ucABPTimer = 5;//1;		//A70Gen2_Doulas_0022 Modify
            break;

        case eLIGHT_SENSOR_INTESSITY_BLD:
            halLDCtrl_BLD_Bank_Enable(TRUE);
			#ifdef LIGHHT_SENSOR_GET_TARGET_NOT_RLD_OFF	//A70Gen2_Doulas_0035
			halLDCtrl_RLD_Bank_Enable(TRUE);
			#else
            halLDCtrl_RLD_Bank_Enable(FALSE);
			#endif
            //halLDCtrl_LS_RLD_Enable_Set(FALSE);				//G100_Doulas_0010
            //halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0x00);  //G100_Doulas_0010 remove//A70LV_Larry_0179
            //halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0xFF);
            //halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0xFF);
            m_sPalIllumInfo.ucABPTimer = 4; //G100_Owen_0093
            break;

        case eLIGHT_SENSOR_INTESSITY_RLD:
            halLDCtrl_BLD_Bank_Enable(TRUE);
            halLDCtrl_RLD_Bank_Enable(TRUE);
            //halLDCtrl_LS_RLD_Enable_Set(TRUE);				//G100_Doulas_0010
            //halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0xFF);  //G100_Doulas_0010 remove//A70LV_Larry_0179
            //halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0xFF);
            //halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0xFF);
            break;

        case eLIGHT_SENSOR_INTESSITY_DONE:
        case eLIGHT_SENSOR_INTESSITY_ERROR:
            halLDCtrl_BLD_Bank_Enable(TRUE);
            halLDCtrl_RLD_Bank_Enable(TRUE);
            m_sPalIllumInfo.ucABPTimer = 0; //A70LV_Larry_0230
            break;

    }
}

void palIllumination_LightSensor_ConstantIntensity_Process(void) //T100_Larry_0019
{
    if(m_sPalIllumInfo.ucABPTimer != 0)
    {
        m_sPalIllumInfo.ucABPTimer--;
        return;
    }

    LOG_MSG(db_APP_ILLUMINATION, "LightSensor Constant Intensity[%d]\r\n", m_sPalIllumInfo.ucLightSensorConstantIntensity);

    switch(m_sPalIllumInfo.ucLightSensorConstantIntensity)
    {
        case eLIGHT_SENSOR_INTESSITY_IDEL:
            palIllumination_ABP_StatusSet(eABP_STATUS_IDEL);
            break;

        case eLIGHT_SENSOR_INTESSITY_START:
            {
                UINT16 uiData[8] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;
				UINT16 uiRLD_Light[2];//G100_Doulas_0010

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));

                uiData[0] = sLightSenosr.uiY;
                uiData[1] = sLightSenosr.uiR;
                uiData[2] = sLightSenosr.uiB;
                uiData[3] = sLightSenosr.uiG;

                for(ucCount = eLD_SEQ_R; ucCount < eLD_SEQ_NUMBER; ucCount++)   //A70G2_Owen_0003
                {
                   if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                   {
                       LOG_MSG(db_APP_ILLUMINATION, "uiData[%d] = %d\r\n", ucCount, uiData[ucCount]);
                       ucError |= 1;
                   }
                }
				halFormatter_GetRLD_Light(uiRLD_Light);		//G100_Doulas_0010
                halFormatter_PWM_Update();

                if(ucError)
                {
                   palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_ERROR);
                }
                else
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                    uiData[0] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_Y);
                    uiData[1] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_R);
                    uiData[2] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_B);
                    uiData[3] = palFormatterMgr_PWM_Get(ePAL_LD_SEG_BLD_G);
                    uiData[4] = uiRLD_Light[1];//palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_Y);		//G100_Doulas_0010 Modify, get target light
                    uiData[5] = uiRLD_Light[0];//palFormatterMgr_PWM_Get(ePAL_LD_SEG_RLD_R);		//G100_Doulas_0010 Modify, get target light

                    LOG_MSG(db_APP_ILLUMINATION, "PWM Get[Y,R,B,G,RY,RR], %d, %d, %d, %d, %d, %d\r\n", uiData[0], uiData[1], uiData[2], uiData[3], uiData[4], uiData[5]);
                    palDataMgr_Data_Access(edcCONSTANT_INTENSITY_PWM, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                    palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_BLD);
                }
            }
            break;

        case eLIGHT_SENSOR_INTESSITY_BLD:
            {
                UINT16 uiData[eLD_SEQ_NUMBER] = {0};
                UINT8  ucCount = 0;
                UINT8  ucError = 0;
                sLIGHT_SENSOR sLightSenosr;
                INT32 wLightSensorInfoType = 0; //HICC2_Doulas_0045

    			Syscfg_Value_Get(eLightSensorSegmentType, &wLightSensorInfoType); //HICC2_Doulas_0045

                palIllumination_AbpLightSensor(&sLightSenosr);

                //memcpy(&sLightSenosr, &m_sLightSensor, sizeof(sLIGHT_SENSOR));
                palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaREAD, (void*)&uiData);  //G100_Owen_0092


                LOG_MSG(db_APP_ILLUMINATION, "BLD Get[Y,R,B,G], %d, %d, %d, %d\r\n", uiData[eLD_SEQ_Y], uiData[eLD_SEQ_R], uiData[eLD_SEQ_B], uiData[eLD_SEQ_G]);
                if(wLightSensorInfoType == eDC_LIGHT_SENSOR_SEGMENT_TYPE_0)   //HICC2_Doulas_0045 //R/G/B/RR
                {
                    uiData[eLD_SEQ_Y] = sLightSenosr.uiY;
                    uiData[eLD_SEQ_R] = sLightSenosr.uiR;
//                uiData[eLD_SEQ_B] = sLightSenosr.uiB; //G100_Owen_0092 remove
//                uiData[eLD_SEQ_G] = sLightSenosr.uiG; //G100_Owen_0092 remove

                    for(ucCount = eLD_SEQ_R; ucCount < eLD_SEQ_B; ucCount++)   //A70G2_Owen_0003
                    {
                        if(ABC_LIGHT_SENSOR_MINIMUM > uiData[ucCount] || ABC_LIGHT_SENSOR_MAXIMUM < uiData[ucCount])
                        {
                            ucError |= 1;
                        }
                    }
                }
                else  //R/G/B/Y ,no RLD
                {

                }


                if(ucError)
                {
                   palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_ERROR);
                }
                else
                {
                    palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_BLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiData);

                    #if 1//def RLD_TYPE //A35G2_CDS_Casper_0001
                        palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_RLD);
                    #else
                        palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_DONE);
                    #endif
                }
            }
            break;

        case eLIGHT_SENSOR_INTESSITY_RLD:
            {
                UINT16 uiBLD[eLD_SEQ_NUMBER] = {0};
                UINT16 uiRLD[eLD_SEQ_NUMBER] = {0};
                INT32 wLightSensorInfoType = 0; //HICC2_Doulas_0045

                Syscfg_Value_Get(eLightSensorSegmentType, &wLightSensorInfoType); //HICC2_Doulas_0045
                palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaREAD, (void*)&uiRLD);
                palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_BLD, edaREAD, (void*)&uiBLD);
                LOG_MSG(db_APP_ILLUMINATION, "BRLD - BLD YR, [%d, %d], [%d, %d]\r\n", uiRLD[0], uiBLD[0], uiRLD[1], uiBLD[1]); //G100_Owen_0092

                if(wLightSensorInfoType == eDC_LIGHT_SENSOR_SEGMENT_TYPE_0)   //HICC2_Doulas_0045 //R/G/B/RR
                {
                    uiRLD[eLD_SEQ_Y] = ABS(uiRLD[eLD_SEQ_Y] - uiBLD[eLD_SEQ_Y]); //Y    //G100_Owen_0092
                    uiRLD[eLD_SEQ_R] = ABS(uiRLD[eLD_SEQ_R] - uiBLD[eLD_SEQ_R]); //R    //G100_Owen_0092
                }
                else
                {
                    uiRLD[eLD_SEQ_Y] = 0;
                    uiRLD[eLD_SEQ_R] = 0;
                }
                uiRLD[eLD_SEQ_B] = 0; //G
                uiRLD[eLD_SEQ_G] = 0; //B

                LOG_MSG(db_APP_ILLUMINATION, "[RR,RY] %d, %d\r\n", uiRLD[eLD_SEQ_R], uiRLD[eLD_SEQ_Y]); //G100_Owen_0092

                palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiRLD);

                palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_DONE);

				uiRLD[eLD_SEQ_Y] = 0;	//G100_Doulas_0023
				uiRLD[eLD_SEQ_R] = 0;	//G100_Doulas_0023
				uiRLD[eLD_SEQ_B] = 0; 	//G100_Doulas_0023
                uiRLD[eLD_SEQ_G] = 0; 	//G100_Doulas_0023
				palDataMgr_Data_Access(edcLIGHTSENSOR_FULL_DYNAMIC_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiRLD);	 	//G100_Doulas_0023 clear
    			palDataMgr_Data_Access(edcLIGHTSENSOR_ECO_DYNAMIC_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiRLD);		//G100_Doulas_0023
    			palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiRLD);	//G100_Doulas_0023
    			palDataMgr_Data_Access(edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD, edaWRITE_THROUGH_NO_ACTION, (void*)&uiRLD);	//G100_Doulas_0023
            }
            break;

        case eLIGHT_SENSOR_INTESSITY_DONE:
            {
                UINT16 uiData[8] = {0};
                palDataMgr_Data_Access(edcCONSTANT_INTENSITY_PWM, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "Constant Intensity PWM %d %d %d %d %d %d\r\n",uiData[0], uiData[1], uiData[2], uiData[3], uiData[4], uiData[5]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_BLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "Constant Intensity BLD %d %d %d %d\r\n", uiData[0], uiData[1], uiData[2], uiData[3]);
                palDataMgr_Data_Access(edcLIGHTSENSOR_INTENSITY_RLD, edaREAD, (void*)&uiData);
                LOG_MSG(db_APP_ILLUMINATION, "Constant Intensity RLD %d %d %d %d\r\n", uiData[0], uiData[1], uiData[2], uiData[3]);

                palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_IDEL);
            }
            break;

        case eLIGHT_SENSOR_INTESSITY_ERROR:
            {
                UINT8 ucPOWER_MODE = eCM_POWER_MODE_CONSTANT_POWER;

                palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, (void*)&ucPOWER_MODE);
                if(ucPOWER_MODE == eCM_POWER_MODE_CONSTANT_INTENSITY)
                {
                    ucPOWER_MODE = eCM_POWER_MODE_CONSTANT_POWER;
                    palDataMgr_Data_Access(edcPOWER_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucPOWER_MODE);
                }
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);
                palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_IDEL);
            }
            break;
    }
}

void palIllumination_ABP_StatusSet(UINT8 ucIndex) //T100_Larry_0018
{
    if(m_sPalIllumInfo.bFirstLightSourceOn == FALSE ||
       m_sPalIllumInfo.ucABPStatus == ucIndex)
    {
        return;
    }

    switch(ucIndex)
    {
        case eABP_STATUS_IDEL:
            if(m_sPalIllumInfo.ucABPStatus != eABP_STATUS_IDEL)
            {
            	halLDCtrl_BLD_Bank_Enable(TRUE);
	            halLDCtrl_RLD_Bank_Enable(TRUE);
                //halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0xFF);
                //halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0xFF);
                //halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0xFF);
            }

            m_sPalIllumInfo.ucABPStatus = ucIndex;
            break;

        case eABP_STATUS_FACTORY_CAL:
            //palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_FULL_START);			//A65_OPTOMA_Doulas_0106 Modify
			palIllumination_LightSensorCal_Set(eLIGHT_SENSOR_CAL_SAMPLE_TIME_START);	//A65_OPTOMA_Doulas_0106 Modify
            halLDCtrl_ABP_ModeSet((UINT8)eABP_STATE_IDLE);
            m_sPalIllumInfo.ucABPStatus = ucIndex;
            break;

        case eABP_STATUS_DYNAMIC_RLD_CAL_SHUTDOWN: //T100_Larry_0023 //A70LV_Larry_0363 modify
            //{
            //    INT32 lCoolTimer = 60 ;

                //halC789Ctrl_OutputEnableSet(0); //A70LV_Larry_0372
            //    appLANProcPowerStateSend(ePOWER_STATE_COOLING, 4, (UINT8*)&lCoolTimer);
            //}
        case eABP_STATUS_DYNAMIC_RLD_CAL:
            //palIllumination_LightSensor_RLDDynamic_Cal_Set(eLIGHT_SENSOR_RLD_CAL_START);
            //halLDCtrl_ABP_ModeSet((UINT8)eABP_STATE_IDLE);
            break;

        case eABP_STATUS_CONSTANT_INTENSITY:
            palIllumination_LightSensor_ConstantIntensity_Set(eLIGHT_SENSOR_INTESSITY_START);
            halLDCtrl_ABP_ModeSet((UINT8)eABP_STATE_IDLE);
            m_sPalIllumInfo.ucABPStatus = ucIndex;
            break;

        default:
            break;
    }
}

UINT8 palIllumination_ABP_StatusGet(void) //T100_Larry_0019
{
    return m_sPalIllumInfo.ucABPStatus;
}


void palIllumination_ABP_StatusProcess(void) //T100_Larry_0018
{
    switch(m_sPalIllumInfo.ucABPStatus)
    {
        case eABP_STATUS_IDEL:
            palIllumination_LightSourceMode_Polling();
            palIllumination_ABP_AutoTuning_Polling(); //T100_Larry_0022
            break;

        case eABP_STATUS_FACTORY_CAL:
            palIllumination_LightSensorCal_Process();
            break;

        case eABP_STATUS_DYNAMIC_RLD_CAL:
        case eABP_STATUS_DYNAMIC_RLD_CAL_SHUTDOWN:
            //palIllumination_LightSensor_RLDDynamic_Cal_Process();
            break;

        case eABP_STATUS_CONSTANT_INTENSITY:
            palIllumination_LightSensor_ConstantIntensity_Process();
            break;

        default:
            break;
    }
}
 //G100_Steven_0011 start
// ==============================================================================
// FUNCTION NAME: palIllumination_CCT_Set
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
// 2019/02/20, Larry Create
// --------------------
// ==============================================================================
void palIllumination_CCT_Set(void)
{
    uCCT_TABLE  uCCTTable = {0};
    //UINT16      uiCRC = 0;
    UINT16      uiCount = 0;
    UINT8       ucCount = 0;

    utilDataMgr_CCT_StorageGet(&uCCTTable);	//HICC2_Doulas_0003 //HICC2_Casper_0014

    GEC_WAP_Flag_Set(uCCTTable.sDataStruct.ulCCT_Tag); //G50_Casper_0006  //HICC2_Steven_0019
    LOG_MSG(db_HAL_FORMATTER, "WAP setup start, CCT tag = %lu\r\n", uCCTTable.sDataStruct.ulCCT_Tag);
    if(uCCTTable.sDataStruct.ulCCT_Tag == CCT_NEW_CALIBRATION_TAG) //G50_Casper_0006 //HICC2_Steven_0019
    {
    	palFormatterMgr_WAP_AllSet(eWAP_POWERLEVEL_100,(UINT16*)uCCTTable.sDataStruct.uiPWM100Data);
    	palFormatterMgr_WAP_AllSet(eWAP_POWERLEVEL_50,(UINT16*)uCCTTable.sDataStruct.uiPWM50Data);
    	palFormatterMgr_WAP_AllSet(eWAP_POWERLEVEL_30,(UINT16*)uCCTTable.sDataStruct.uiPWM30Data);
    	palFormatterMgr_WAP_AllSet(eWAP_POWERLEVEL_10,(UINT16*)uCCTTable.sDataStruct.uiPWM10Data);
    	for(uiCount = 0; uiCount < eCM_PICTURE_SETTINGS_NUMBER; uiCount++)
    	{
    		LOG_MSG(db_HAL_FORMATTER, "PWM100 : %d, %d, %d, %d, %d, %d\r\n",
    				uCCTTable.sDataStruct.uiPWM100Data[uiCount][0],
					uCCTTable.sDataStruct.uiPWM100Data[uiCount][1],
					uCCTTable.sDataStruct.uiPWM100Data[uiCount][2],
					uCCTTable.sDataStruct.uiPWM100Data[uiCount][3],
					uCCTTable.sDataStruct.uiPWM100Data[uiCount][4],
					uCCTTable.sDataStruct.uiPWM100Data[uiCount][5]);
    		LOG_MSG(db_HAL_FORMATTER, "PWM50 : %d, %d, %d, %d, %d, %d\r\n",
    				uCCTTable.sDataStruct.uiPWM50Data[uiCount][0],
					uCCTTable.sDataStruct.uiPWM50Data[uiCount][1],
					uCCTTable.sDataStruct.uiPWM50Data[uiCount][2],
					uCCTTable.sDataStruct.uiPWM50Data[uiCount][3],
					uCCTTable.sDataStruct.uiPWM50Data[uiCount][4],
					uCCTTable.sDataStruct.uiPWM50Data[uiCount][5]);
    		LOG_MSG(db_HAL_FORMATTER, "PWM30 : %d, %d, %d, %d, %d, %d\r\n",
    				uCCTTable.sDataStruct.uiPWM30Data[uiCount][0],
					uCCTTable.sDataStruct.uiPWM30Data[uiCount][1],
					uCCTTable.sDataStruct.uiPWM30Data[uiCount][2],
					uCCTTable.sDataStruct.uiPWM30Data[uiCount][3],
					uCCTTable.sDataStruct.uiPWM30Data[uiCount][4],
					uCCTTable.sDataStruct.uiPWM30Data[uiCount][5]);
    		LOG_MSG(db_HAL_FORMATTER, "PWM10 : %d, %d, %d, %d, %d, %d\r\n",
    				uCCTTable.sDataStruct.uiPWM10Data[uiCount][0],
					uCCTTable.sDataStruct.uiPWM10Data[uiCount][1],
					uCCTTable.sDataStruct.uiPWM10Data[uiCount][2],
					uCCTTable.sDataStruct.uiPWM10Data[uiCount][3],
					uCCTTable.sDataStruct.uiPWM10Data[uiCount][4],
					uCCTTable.sDataStruct.uiPWM10Data[uiCount][5]);
    	}
    }
    else
    {
        LOG_MSG(db_HAL_FORMATTER, "CCT Tag NA!\r\n");
    }
}
//G100_Steven_0011 end


#ifdef CUSTOM_BARCO
//G100_Steven_0084 start
void palIllumination_TEC_Gating_InfoGet(void)
{
    palIapProc_TECGATING_StorageGet_CM(&sTEC_GATING);	//HICC2_Doulas_0003
}

void palIllumination_TEC_Gating_InfoSet( UINT16 cCycleData)
{

	sTEC_GATING.ucTEC_GATING_CYCLE[sTEC_GATING.ucTEC_GATING_CNT] = cCycleData;

	LOG_MSG(db_APP_ILLUMINATION, "TEC_Gating_InfoSet cnt %d, cycle %d\r\n",sTEC_GATING.ucTEC_GATING_CNT, cCycleData);

	sTEC_GATING.ucTEC_GATING_CNT = sTEC_GATING.ucTEC_GATING_CNT +1;

	if(sTEC_GATING.ucTEC_GATING_CNT >= FW_BURNIN_CYCLE_DEFAULT_VALUE)
	{
		sTEC_GATING.ucTEC_GATING_CNT = 0;
	}

    palIapProc_TECGATING_StorageSet_CM(&sTEC_GATING);	//HICC2_Doulas_0003

}

void palIllumination_TEC_Gating_InfoClean(void)
{
	UINT8 cIndex = 0;
	sTEC_GATING.ucTEC_GATING_CNT = 0;

	for( cIndex=0; cIndex < FW_BURNIN_CYCLE_DEFAULT_VALUE; cIndex++)
	{
		sTEC_GATING.ucTEC_GATING_CYCLE[cIndex] = 0;
	}

    palIapProc_TECGATING_StorageSet_CM(&sTEC_GATING);	//HICC2_Doulas_0003

}
//G100_Steven_0084 end
#endif




// ==============================================================================
// FUNCTION NAME: palIlluminationLightSourceCount
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
// 2020/09/07, Larry Create
// --------------------
// ==============================================================================
static void palIlluminationLightSourceCount(void)
{
    UINT8  ucPowerMode = 0;
    UINT32 ulVal = 0;
    tDATA_CODE ulMaxRangeValue = 0;

    palDataMgr_Data_Access(edcLD_HOURS, edaREAD, &m_sPalIllumInfo.ulLampPeriod);
    palDataMgr_Data_Range_Get(edcLD_HOURS, edrMAX, &ulMaxRangeValue);	//A35G2_CDS_CODA_0005
    if(m_sPalIllumInfo.ulLampPeriod >= ulMaxRangeValue)	//A35G2_CDS_CODA_0005
    {
        m_sPalIllumInfo.ulLampPeriod = ulMaxRangeValue;
    }
    else
    {
        m_sPalIllumInfo.ulLampPeriod += 1;
    }
    palDataMgr_Data_Access(edcLD_HOURS, edaWRITE_THROUGH_WITH_ACTION, &m_sPalIllumInfo.ulLampPeriod);

    palDataMgr_Data_Access(edcPOWER_MODE, edaREAD, &ucPowerMode);

    if(ucPowerMode == eCM_POWER_MODE_CONSTANT_POWER)
    {
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_NORAML, edaREAD, &ulVal);
        ulVal = ulVal + 1;
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_NORAML, edaWRITE_THROUGH_NO_ACTION, &ulVal);
    }

    if(ucPowerMode == eCM_POWER_MODE_ECO1)
    {
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_ECO, edaREAD, &ulVal);
        ulVal = ulVal + 1;
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_ECO, edaWRITE_THROUGH_NO_ACTION, &ulVal);
    }

    if(ucPowerMode == eCM_POWER_MODE_QUIET_MODE)
    {
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_QUIET, edaREAD, &ulVal);
        ulVal = ulVal + 1;
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_QUIET, edaWRITE_THROUGH_NO_ACTION, &ulVal);
    }

    if(ucPowerMode == eCM_POWER_MODE_CUSTOM_MODE)
    {
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_CUSTOM, edaREAD, &ulVal);
        ulVal = ulVal + 1;
        palDataMgr_Data_Access(edcLIGHT_SOURCE_HOURS_CUSTOM, edaWRITE_THROUGH_NO_ACTION, &ulVal);
    }

}

INT16 palIllumination_Poll(UINT16 uiTick)
{
    BOOL bIsLightSourceOn = FALSE;
    BOOL bDDP_AsicReady = FALSE; //A70LV_Larry_0098

    DDP_SYSTEM_STATUS sDDPSystemStatus = {0};                   //A70LV_Doulas_0212
    eRESULT eResult = rcINVALID;                                //A70LV_Doulas_0212

    UINT8 cLensPlug = 0; //A70LV_Larry_0340

    UINT16 wOPD_Period = palDataMgr_OPD_Period_Get(); //HICC2_Steven_0009

    if(palSystem_PowerStateGet() == ePOWER_STATE_STANDBY)
    {
        return ILLUMINATION_PERIOD/POLL_PERIOD;
    }
    else if(palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE)
    {
        return (ILLUMINATION_PERIOD*60)/POLL_PERIOD;
    }

	bIsLightSourceOn = palCoreVar_GetLightSourceOnVar();

	//DDP asic ready
	if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
	    bDDP_AsicReady = appSysInitAfterAsicReady(); //A70LV_Larry_0098
    }

    if(Board_Stage_Get() > EVT_STAGE)
    {
        if(bDDP_AsicReady)
        {
            if(bIsLightSourceOn && (m_sPalIllumInfo.bFirstLightSourceOn == FALSE)) //A70LV_Larry_0025 //A70LV_Larry_0087
            {
        		if(m_sPalIllumInfo.ucSmoothOn == SMOOTH_ON_DELAY) //A70LV_Larry_0255
        	    {
        	        eResult = halFormatter_SYS_SystemStatusGet((PUINT8)&sDDPSystemStatus);      //A70LV_Doulas_0212 DDP phase lock check
        	        LOG_MSG(db_APP_ILLUMINATION,"PhaseLock1 %d(%d,%d)(%d,%d)(%d)\r\n",
					                                     m_cPhaseLockMaxCounterCheck
        	                                            ,sDDPSystemStatus.DDP_ST_SYSTEM_READY
        	                                            ,sDDPSystemStatus.DDP_ST_CW_SPINNING
        	                                            ,sDDPSystemStatus.DDP_ST_CW_PHASELOCK
        	                                            ,sDDPSystemStatus.DDP_ST_CW_FREQLOCK
        	                                            ,eResult);
        	        if((eResult == rcSUCCESS) &&
        	        //   (sDDPSystemStatus.DDP_ST_SYSTEM_READY == TRUE)&&		//A70Gen2_Doulas_0001 remove
        	           (sDDPSystemStatus.DDP_ST_CW_SPINNING == TRUE)&&
#ifdef PLATFORM_H60_2K //HICC2_Doulas_0019 Rx24t issue
#else
        	           (sDDPSystemStatus.DDP_ST_CW_PHASELOCK == TRUE)&&
#endif
        	           (sDDPSystemStatus.DDP_ST_CW_FREQLOCK == TRUE))
        	        {
                        m_cWorkCounterCheck--;
        	        }
        	        else
        	        {
                        m_cWorkCounterCheck = PHASE_LOCK_COUNT_DOWN;
        	        }
                    LOG_MSG(db_APP_ILLUMINATION,"m_cWorkCounterCheck %d \r\n",m_cWorkCounterCheck);
                    if(m_cPhaseLockMaxCounterCheck % 10 == 8)
                    {
                        halMCU_DDP_PowerSet(1);
                    }
        	        m_cPhaseLockMaxCounterCheck++;

        	        if((m_cWorkCounterCheck <= 0) || (m_cPhaseLockMaxCounterCheck > PHASE_LOCK_MAX_TIMER))     //A70LV_Doulas_0212 DDP phase lock check
                    {
        	            UINT8 ucColorOffset = 0;
                        UINT8 ucValue = 0;
                        UINT8 ucRear = 0;
                        UINT8 ucV110Det = 0;
                        UINT8 ucTempDimPower = 0;

                        while(eEXEC_CODE_PASS != palDataPath_GetPathReady()) //等DataPath ready在點燈，套twist blending會比較久 //H2PF_Simon_0048
                        {
                            MS_SLEEP(100);
                        }

        	            //m_sPalIllumInfo.bFirstLightSourceOn = TRUE;
        	            m_sPalIllumInfo.ucABPDelayStart = 10;
        				m_sPalIllumInfo.ucSmoothOn = SMOOTH_ON_READY; //A70LV_Larry_0340
					    #ifdef Low_Latency_All
					    if(halScaler_3D_Type() == eINPUT_3D_TYPE2_OFF)
					    {
						    halFormatter_FRCByPassModeSet(TRUE);
					    }
					    #endif	/*Low_Latency_All*/
                        //palDataMgr_Data_Access(edcCOLOR_OFFSET, edaREAD, (void*)&ucColorOffset);
                        //halFormatter_COLOR_OFFSET_Set(ucColorOffset);

                        //palDataMgr_ColorConfig_Parameter_Set();
                        palDataMgr_Format_PictureMode_Set(); //T100_Casper_0087
#ifdef OE_JIG
                        palDataMgr_LightSource_Set(13, 100); //13 is eCM_POWER_MODE_20
#endif /* OE_JIG */
#if OPEN_WAP
                        palIllumination_CCT_Set();  //G100_Steven_0011
#endif

                        //palDataMgr_Formatter_Parameter_Set();

						m_sPalIllumInfo.cLensPlug = 99;//A70Gen2_Doulas_0033


#ifndef OE_JIG
						halFormatter_DimPower_SetDefault();

						if(rcSUCCESS == halMCUCtrl_AC_Voltage_Info_Get(&ucV110Det))
                        {
                            m_sPalIllumInfo.ucLVPS_110VDet = ucV110Det;

                            #if defined (PLATFORM_H60_2K)  //HICC2_Doulas_0102
                            if(Board_ModelID_Get() == MODEL_ID_1)
                            #else
                            if(Board_ModelID_Get() == MODEL_ID_2)
                            #endif
                            {
                                if(ucV110Det)
                                {
                                    LOG_MSG(db_HAL_FORMATTER, "LVPS DimPower for 110V\r\n");
                                    ucValue = LVPS_110V_DIMPOWER_LEVEL;
                                }
                                else
                                {
                                    ucValue = LVPS_220V_DIMPOWER_LEVEL;
                                }

                                UINT8 ucConstantPowerNumber;
                                palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaREAD, (void*)&ucConstantPowerNumber); //H2PF_Simon_0068

                                palDataMgr_Data_Access(edcDIMMING_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);

                                if(ucConstantPowerNumber > ucValue) //HICC2_Doulas_0102 Add   //H2PF_Simon_0068
								{
                                    palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);
								}
                            }
                        }

						if(Board_ModelID_Get() == MODEL_ID_2)
						{
							if(rcSUCCESS == halLDCtrl_EnvironmentDimPower_Get(&ucTempDimPower))
							{
								if(ucTempDimPower)
								{
									LOG_MSG(db_HAL_FORMATTER, "Environment DimPower\r\n");
									ucValue = TEMP_DIMPOWER_LEVEL;
									palDataMgr_Data_Access(edcDIMMING_MODE, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue);
									palDataMgr_Data_Access(edcCONSTANT_POWER_NUMBER, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucValue); //HICC2_Doulas_0102
								}
							}
						}
#endif /* OE_JIG */

						LOG_MSG(db_APP_ILLUMINATION,"1 m_sPalIllumInfo.ucSmoothOn %d\r\n",m_sPalIllumInfo.ucSmoothOn);
        	        }
        		}
        		else
        		{
        			if(m_sPalIllumInfo.ucSmoothOn == SMOOTH_ON_DONE) //A35G2_BRC_Casper_0086 //A35G2_BRC_Casper_0116
        			{
                        UINT8 ucFlag = FALSE;   //A70G2_Owen_0002
                        UINT8 ucNum = 0;

                        m_sPalIllumInfo.bFirstLightSourceOn = TRUE; //HICC2_Steven_0082, #ISS-0031940 "power off cmd" can only be executed after LD on
                        palIllumination_OPDSnapshotPowerOnDelay(20);
                        halLDCtrl_LD_Enable_Default();

                        for(ucNum = 0 ; ucNum < 16 ; ucNum++)
                        {
                            palLANProcSendToLAN((eDATA_CODE)(edcLD_ENALE_01+ucNum));
                        }

        				if(palDataMgr_DataCode_Control(edcCONTRAST_ENHANCEMENT) == eFUNC_CONTROL_ENABLE)
        			    {
        			    	UINT8 ucEnhancement = 0;

        	                palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaREAD, (void*)&ucEnhancement);
        	                palDataMgr_Data_Access(edcCONTRAST_ENHANCEMENT, edaWRITE_RAM_ONLY_WITH_ACTION, (void*)&ucEnhancement);
        					LOG_MSG(db_APP_ILLUMINATION,"2 m_sPalIllumInfo.ucSmoothOn %d\r\n",m_sPalIllumInfo.ucSmoothOn);
        			    }
        			    #ifdef QUICKLY_POWER_ON	//G100_Doulas_0024
    					halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
    					halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
    					halFormatter_SplashAtStartupTimeoutSet(0);			//Splash timeout(disable splash)
    					#endif

                        //setting LS    //A70G2_Owen_0002 Start
                        palDataMgr_Data_Access(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);
                        if(ucFlag == TRUE)
                        {
                            float fGain[eLD_SEQ_NUMBER] = {0.0f};   //G100_Owen_0132
                            UINT16 auiOffset[eLD_SEQ_NUMBER] = {0};
                            UINT8 aucTime[eLD_SEQ_NUMBER] = {0};

                            palDataMgr_Data_Access(edcLIGHTSENSOR_TIME, edaREAD, (void*)aucTime);     //A70Gen2_Larry_0008
                            palDataMgr_Data_Access(edcLIGHTSENSOR_GAIN, edaREAD, (void*)fGain);       //A70Gen2_Larry_0008
                            palDataMgr_Data_Access(edcLIGHTSENSOR_OFFSET, edaREAD, (void*)auiOffset); //A70Gen2_Larry_0008

                            LOG_MSG(db_APP_ILLUMINATION, "LS read T (YRBG) (%d,%d,%d,%d)\r\n",aucTime[0],aucTime[1],aucTime[2],aucTime[3]);
                            LOG_MSG(db_APP_ILLUMINATION, "LS read Gain (YRBG) (%f,%f,%f,%f)\r\n",fGain[0],fGain[1],fGain[2],fGain[3]);
                            LOG_MSG(db_APP_ILLUMINATION, "LS read Offset (YRBG) (%d,%d,%d,%d)\r\n",auiOffset[0],auiOffset[1],auiOffset[2],auiOffset[3]);
                            halLDCtrl_LightSensorT0_Set(aucTime);//halLDCtrl_LightSensorTime_Set(aucTime);
                            halLDCtrl_LightSensorT1_Set((UINT8 *)fGain);//halLDCtrl_LightSensorGain_Set((UINT8 *)fGain);
                            halLDCtrl_LightSensorOffset_Set((UINT8 *)auiOffset);
                        }

                        palDataPath_SmoothOnReadyCheck_Set(1);
                    }
                    else if(m_sPalIllumInfo.ucSmoothOn == SMOOTH_ON_READY) //A70LV_Larry_0340
                    {
                        if((Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||
                           (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60))   //HICC2_Doulas_0030
                        {
                            LOG_MSG(db_APP_ILLUMINATION,"1 bLensPlug %d %d\r\n",m_sPalIllumInfo.cLensPlug , cLensPlug);
                        }

                        palLedProc_LED_Behavior_Set(eLED_STATUS_NORMAL);    //G100_Owen_0032

                        if(m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable)
                        {
                            UINT8 ucLDOn = 0;
                            UINT8 ucRetry = 0;
        			    	UINT8 ucFade_In_Step = 0;	//G100_Clare_0004
                            UINT8 ucPictureMute = 0;

                            halLDCtrl_Light_Module_Status_Set(eLDBANK_A70LV, 1);
    						#ifdef QUICKLY_POWER_ON		//G100_Doulas_0024 Modify
    						#else
                            do{	//G100_Clare_0004
    							palDataMgr_Data_Access(edcFADE_IN, edaREAD, (void*)&ucFade_In_Step);
                            #ifdef CUSTOM_CHRISTIE
                                ucFade_In_Step = FADE_IN_TIMER_QUICK;
                            #endif
                                palFormatterMgr_Smooth_Enable_Set(TRUE, ucFade_In_Step);
                                MS_SLEEP(5);
                                palFormatterMgr_Smooth_Enable_Get(&ucLDOn);
                            }while((ucRetry++ < 5)&&(ucLDOn == FALSE));
                            #endif
                            //halLDCtrl_LD_SourceEnable_Set(eLDBANK_A70LV, 1);
        	                palLedProc_Shutter_LED_Set(eLED_SHUTTER_OFF); //A70LV_Larry_0081
                            palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_NO_ACTION, &ucPictureMute);

                            m_sPalIllumInfo.ucSmoothOn--;
                        }
                        else
                        {
                    	    UINT8 ucC789_C821_InitReady = palDataPath_C789_C821_InitReadyGet();		//G100_Doulas_0076

                            if((Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||
                                (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60))   //HICC2_Doulas_0030
                            {
                                cLensPlug = palIllumination_LensDetectionGet();
                            }

    						LOG_MSG(db_APP_ILLUMINATION, "C789_C821_InitReady = %d\r\n", ucC789_C821_InitReady);	//G100_Doulas_0076
    						if(ucC789_C821_InitReady)																//G100_Doulas_0076 appDataPath_StartDisplay 跑完
    						{
							    palDataPath_C789_C821_InitReadySet(ucC789_C821_InitReady+1);
    						}
                            if(ucC789_C821_InitReady == 3)//HICC2_Julie_0037
                            {
                                palDataPath_SmoothOnReadyCheck_Set(1);
                            }

    						if( ((m_sPalIllumInfo.cLensPlug != cLensPlug) &&
    						   (ucC789_C821_InitReady > 2) &&
    						   Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70)
    						   ||
    						    #if !defined(CUSTOM_OPTOMA) && !defined(CUSTOM_CHRISTIE)
    						    (ucC789_C821_InitReady > 2 && Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A35)
    						    #else
    						    (ucC789_C821_InitReady && Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A35)
    						    #endif
    						   ||
    						   ((m_sPalIllumInfo.cLensPlug != cLensPlug) &&
    						   (ucC789_C821_InitReady > 2) &&
    						   Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60) //HICC2_Doulas_0030
    						)	//G100_Doulas_0076 Modify 等待appDataPath_StartDisplay 跑完加上2秒後點燈
                            {
                                #if 0 //ndef QUICKLY_POWER_ON
                                halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
                                #endif
                                #ifdef CUSTOM_OPTOMA		//A65_OPTOMA_Doulas_0069 //A35G2_Coda_0062
    							utilOptoma_OutputInfoMsg(sOptoma_System_Auto_Send_Lut[eOPT_SYSTEM_READY].cINFO_String);
                    			#endif

                        	    palDataPath_C789_C821_InitReadySet(0);	//G100_Doulas_0076

                                m_sPalIllumInfo.cLensPlug = cLensPlug;
                                if((m_sPalIllumInfo.cLensPlug && Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||//LD off
                                   (m_sPalIllumInfo.cLensPlug && Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60)) //HICC2_Doulas_0030
                                {
								    m_sPalIllumInfo.ucSmoothOn--;
                                    palLedProc_Shutter_LED_Set(eLED_SHUTTER_ON);
    								halLDCtrl_LD_SourceEnable_Set(eLDBANK_A70LV, 0);	//G100_Clare_0005
                                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_NO_ACTION, &m_sPalIllumInfo.cLensPlug);	//G100_Clare_0005
#ifndef OE_JIG
                                    if(palSystem_SystemCheckGet() == FALSE)//HICC2_Julie_0046
                                    {
                                        halFanCtrl_OperationModeSet();
                                    }
#endif
                                }
                                else //LD on
                                { //A35G2_BRC_Casper_0116
    	                            UINT8 ucStartupShutter = 0;
                                    UINT8 ucPictureMute = 0;

                                    #if TEC_TESTER_JIG//G100_Steven_0080
                                    ucStartupShutter = ets_ON;
                                    m_cTEC_Tester_LDEn_Cnt = 1;
                                    //LOG_MSG(db_ALWAYS, "Startup shutter = %d\r\n", ucStartupShutter);
                                    #else
    	                            palDataMgr_Data_Access(edcSTARTUP_SHUTTER, edaREAD, &ucStartupShutter);
                                    #endif
                                    palDataMgr_Data_Access(edcPICTURE_MUTE, edaREAD, &ucPictureMute);

    	                            LOG_MSG(db_APP_ILLUMINATION, "Startup shutter = %d, Picture mute = %d\r\n", ucStartupShutter, ucPictureMute);

                                    m_sPalIllumInfo.ucSmoothOn--;

                                    if(ucPictureMute == 1) //預設應為1，0代表user已經在smooth on過程中按shutter點燈
                                    {

                                        #ifdef QUICKLY_POWER_ON     //G100_Doulas_0024 Modify
                                        #else
                                        palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucStartupShutter);  //G100_Clare_0005
                                        #endif
                                        if(ucStartupShutter != 0)   //G100_Owen_0032
                                        {
                                            palLedProc_Shutter_LED_Set(eLED_SHUTTER_ON);
#ifndef OE_JIG
                                            if(palSystem_SystemCheckGet() == FALSE)//HICC2_Julie_0046
                                            {
                                                halFanCtrl_OperationModeSet();
                                            }
#endif
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        m_sPalIllumInfo.ucSmoothOn--;
                        LOG_MSG(db_APP_ILLUMINATION,"m_sPalIllumInfo.ucSmoothOn %d\r\n",m_sPalIllumInfo.ucSmoothOn);
                    }
                }
            }
        }
    }
    else
    {
        m_sPalIllumInfo.ucSmoothOn = SMOOTH_ON_DONE;
        m_sPalIllumInfo.bFirstLightSourceOn = TRUE;
        palDataPath_SmoothOnReadyCheck_Set(1);
    }

	//write opd engine log delay 30s when shutter and high attitude happened.
	if((m_sPalIllumInfo.ucSnapshotUpdatePeriod > 0) && (palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)) //A35G2_CDS_Larry_0020
	{
		m_sPalIllumInfo.ucSnapshotUpdatePeriod--;

		if(m_sPalIllumInfo.ucSnapshotUpdatePeriod == 0)
		{
			if((m_OPD_EngineLog_Index >= eOPD_SHUTTER_ON_LOG) && (m_OPD_EngineLog_Index <= eOPD_HIGH_ALTITUDE_ON))
			{
				//LOG_MSG(db_ALWAYS, "(%s,%d) m_OPD_EngineLog_Index[%d](SHUTTER_ON=27, HIGH_ALTITUDE_ON=30)\r\n", __FUNCTION__, __LINE__, m_OPD_EngineLog_Index);
				palDataMgr_OPDSnapshot(m_OPD_EngineLog_Index);
			}
		}
	}

    if(m_sPalIllumInfo.ucSnapshotPowerOn)
    {
        m_sPalIllumInfo.ucSnapshotPowerOn--;
        if(m_sPalIllumInfo.ucSnapshotPowerOn == 0)
        {
            palDataMgr_OPDSnapshot(eOPD_POWER_ON_LOG);
        }
    }

	if(wOPD_Period != 0 && (m_sPalIllumInfo.ulCurrentRunTime >= 2))  //HICC2_Steven_0009
	{
		m_dwOPDPeriodCnt++;
		//LOG_MSG(db_ALWAYS,"palDataMgr_OPDSnapshot (%d, %d)\r\n", m_dwOPDPeriodCnt , m_sPalIllumInfo.ulCurrentRunTime);

		if( (palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE) && (m_dwOPDPeriodCnt >= wOPD_Period))
		{
			 //LOG_MSG(db_ALWAYS,"palDataMgr_OPDSnapshot \r\n");
			 m_dwOPDPeriodCnt = 0;
			 palDataMgr_OPDSnapshot(eOPD_ENGINE_LOG);
			 palDataMgr_OPDRegulatoryInfo();
		}
	}


    if( ++m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount >= LAMP_PERIOD_UPDATE)   //A70LV_Doulas_0061 Add,Total projector hours (One minute){
    {
        palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS, edaREAD, &m_sPalIllumInfo.ulTotalProjectorHours);
        m_sPalIllumInfo.ulTotalProjectorHours += 1;
        palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS, edaWRITE_THROUGH_WITH_ACTION, &m_sPalIllumInfo.ulTotalProjectorHours);

        LOG_MSG(db_APP_ILLUMINATION,"TotalProjectorHours %d(min.)\r\n", m_sPalIllumInfo.ulTotalProjectorHours);

        m_sPalIllumInfo.ulCurrentRunTime += 1;

        if((palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE) && (m_sPalIllumInfo.ulCurrentRunTime%10 == 0) && (wOPD_Period== 0))  //HICC2_Steven_0009
        {
            palDataMgr_OPDSnapshot(eOPD_ENGINE_LOG);
        }
        palDataMgr_OPDRegulatoryInfo(); //A65_OPTOMA_Julie_0080

        m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount = 0;
    }

    utilOPD_TotalProjector_Set(palIllumination_TotalProjectSec_Get()); //A65_OPTOMA_Julie_0080

    if((Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||
        (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60))   //HICC2_Doulas_0030
    {
        if((m_sPalIllumInfo.cLensPlug == 0) && (m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable == 0))
        {
            UINT8 ucID = palMotor_LensId_Get();

            cLensPlug = palIllumination_LensDetectionGet();

            if(m_sPalIllumInfo.cLensPlug != cLensPlug)
            {
                m_sPalIllumInfo.cLensPlug = cLensPlug;
                LOG_MSG(db_APP_ILLUMINATION,"2 bLensPlug %d\r\n",m_sPalIllumInfo.cLensPlug);

                palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &m_sPalIllumInfo.cLensPlug);
                palDataMgr_Data_Access(edcLENS_TYPE, edaWRITE_RAM_ONLY_WITH_ACTION,(void*)&ucID);
                //appGui_LensID_Checking();   //G100_Owen_0068
            }
        }
    }

    if(bIsLightSourceOn)//A70LV_Doulas_0061 modify
    {
        if( ++m_sPalIllumInfo.uiLampUpdateCount >= LAMP_PERIOD_UPDATE )  //One minute //A70LV_Doulas_0061 modify
        {
        	palIlluminationLightSourceCount();

            m_sPalIllumInfo.ulStartUpLampPeriod += 1;
            m_sPalIllumInfo.uiLampUpdateCount = 0;

            LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)Illum Time [%d]\r\n", __FUNCTION__, __LINE__, m_sPalIllumInfo.ulLampPeriod);

            if (m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable)
            {
			#ifdef CUSTOM_BARCO
            	palIllumination_TEC_Gating(m_sPalIllumInfo.ulStartUpLampPeriod); //G100_Steven_0084
			#endif
                if (m_sPalIllumInfo.ulStartUpLampPeriod >= m_sPalIllumInfo.sBurnIn_Info.ucLampOn)
                {
				#ifdef CUSTOM_BARCO
                    palIllumination_TEC_Result(); //G100_Steven_0084
				#endif
                    if (m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle > 1)
                    {
				#ifdef CUSTOM_BARCO
                    	cTEC_Gating_PassCnt = 0; //G100_Steven_0084
                    	b_TEC_Check_F = 0;
                    	b_TEC_First_F = 0;  //G100_Steven_0087
				#endif
                        m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle = m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle - 1;
                        palDataMgr_Data_Access(edcBURNIN_CYCLE, edaWRITE_THROUGH_WITH_ACTION, &m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle);
                        LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d) remain ucBurnInCycle[%d]\r\n", __FUNCTION__, __LINE__, m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle);
                    }
                    else
                    {
				#ifdef CUSTOM_BARCO
                    	cTEC_Gating_PassCnt = 0; //G100_Steven_0084
                    	b_TEC_Check_F = 0;
                    	b_TEC_First_F = 0;  //G100_Steven_0087
				#endif
                        m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle = 0;         //H30K_Tim_0012, mod, 1 //for OSD Range
                        palDataMgr_Data_Access(edcBURNIN_CYCLE, edaWRITE_THROUGH_WITH_ACTION, &m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle);
                        m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable = ets_OFF;
                        palDataMgr_Data_Access(edcBURNIN_ENABLE, edaWRITE_THROUGH_WITH_ACTION, &m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable);
                        LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d) Burnin time is up, shut down and disable Burninflag[%d]\r\n", __FUNCTION__, __LINE__, m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable);
                    }
                    //send system power off event
                    palSystem_PowerDown();
                }
            }
        }
    }

    if(bIsLightSourceOn && (0 == m_sPalIllumInfo.uiLampUpdateCount%5) && m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable)
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_BURNIN_PATTERN_UPDATE, TRUE, NULL);
    }

#ifdef OE_JIG //A70LV_Larry_0322
    //if((bIsLightSourceOn) && (m_sPalIllumInfo.uiLampUpdateCount%10 == 0) && (palSystem_SystemCheckGet() == FALSE)) //HICC2_Doulas_0032
    if((bIsLightSourceOn) && (m_sPalIllumInfo.uiLampUpdateCount%10 == 0)) //HICC2_Doulas_0032 Modify
    {
        UINT16 uiFan1  = palDataMgr_Access_Get_FAN_RPM(0); //HICC2_Doulas_0034
        UINT16 uiFan2  = palDataMgr_Access_Get_FAN_RPM(1); //HICC2_Doulas_0034
        UINT16 uiFan3  = palDataMgr_Access_Get_FAN_RPM(2); //HICC2_Doulas_0034
        UINT16 uiFan7  = palDataMgr_Access_Get_FAN_RPM(6); //HICC2_Doulas_0034
        UINT16 uiFan8  = palDataMgr_Access_Get_FAN_RPM(7); //HICC2_Doulas_0034
        UINT16 uiFan10 = palDataMgr_Access_Get_FAN_RPM(9); //HICC2_Doulas_0034

        UINT16 uiFan11 = palDataMgr_Access_Get_FAN_RPM(10);
        UINT16 uiFan12 = palDataMgr_Access_Get_FAN_RPM(11);
        UINT16 uiFan14 = palDataMgr_Access_Get_FAN_RPM(13); //HICC2_Doulas_0034
        UINT16 uiFan15 = palDataMgr_Access_Get_FAN_RPM(14);
        UINT16 uiFan17 = palDataMgr_Access_Get_FAN_RPM(16);
        UINT16 uiPump = palDataMgr_Access_Get_Pump();
        UINT16 uiTecDMD = palDataMgr_Access_Get_TEC_Current(0);
        INT16  iDMDTemp = palDataMgr_Access_Get_Thermal_Sensor(1);
        //UINT16 uiTecRLD = palDataMgr_Access_Get_TEC_Current(1);
        //UINT16 uiRLDCurrent11 = palDataMgr_Access_Get_LD_Current(10);
        //UINT16 uiRLDCurrent12 = palDataMgr_Access_Get_LD_Current(11);
        //UINT16 uiRLDCurrent13 = palDataMgr_Access_Get_LD_Current(12);
        //UINT16 uiRLDCurrent14 = palDataMgr_Access_Get_LD_Current(13);

        LOG_MSG(db_APP_MONITOR, "Fan1~3   = %d,%d,%d \n", uiFan1,uiFan2,uiFan3); //HICC2_Doulas_0034
        LOG_MSG(db_APP_MONITOR, "Fan7~10  = %d,%d,%d \n", uiFan7,uiFan8,uiFan10);
        LOG_MSG(db_APP_MONITOR, "Fan11~14 = %d,%d,%d \n", uiFan11,uiFan12,uiFan14);
        LOG_MSG(db_APP_MONITOR, "Fan15~17 = %d,%d \n", uiFan15,uiFan17);
        LOG_MSG(db_APP_MONITOR, "Pump = %d\n", uiPump);
        LOG_MSG(db_APP_MONITOR, "TecDMD = %d\n", uiTecDMD);
        LOG_MSG(db_APP_MONITOR, "DMDTemp = %d\n", iDMDTemp);
        //LOG_MSG(db_APP_MONITOR, "TecRLD = %d\n", uiTecRLD);
        //LOG_MSG(db_APP_MONITOR, "RLD Cuurent = %d %d %d %d\n", uiRLDCurrent11, uiRLDCurrent12, uiRLDCurrent13, uiRLDCurrent14);


        //check fan lock
        if( (uiFan1  < 500) || (uiFan2 < 500)  || (uiFan3  < 500) || (uiFan7 < 500) ||
            (uiFan8  < 500) || (uiFan10 < 500) || (uiFan14 < 500) ||
            (uiFan11 < 500) || (uiFan12 < 500) || (uiFan15 < 500) || (uiFan17 < 500)) //HICC2_Doulas_0034//HICC2_Doulas_0032
        {
            if(uiFan1 < 500) //HICC2_Doulas_0034
            {
                m_ucFan1ErrorCount++;
                if(m_ucFan1ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock01);
                }
            }
            else
            {
                m_ucFan1ErrorCount = 0;
            }

            if(uiFan2 < 500) //HICC2_Doulas_0034
            {
                m_ucFan2ErrorCount++;
                if(m_ucFan2ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock02);
                }
            }
            else
            {
                m_ucFan2ErrorCount = 0;
            }

            if(uiFan3 < 500) //HICC2_Doulas_0034
            {
                m_ucFan3ErrorCount++;
                if(m_ucFan3ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock03);
                }
            }
            else
            {
                m_ucFan3ErrorCount = 0;
            }

            if(uiFan7 < 500) //HICC2_Doulas_0034
            {
                m_ucFan7ErrorCount++;
                if(m_ucFan7ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock07);
                }
            }
            else
            {
                m_ucFan7ErrorCount = 0;
            }

            if(uiFan8 < 500) //HICC2_Doulas_0034
            {
                m_ucFan8ErrorCount++;
                if(m_ucFan8ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock08);
                }
            }
            else
            {
                m_ucFan8ErrorCount = 0;
            }

            if(uiFan10 < 500) //HICC2_Doulas_0034
            {
                m_ucFan10ErrorCount++;
                if(m_ucFan10ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock10);
                }
            }
            else
            {
                m_ucFan10ErrorCount = 0;
            }

            if(uiFan11 < 500)
            {
                m_ucFan11ErrorCount++;
                if(m_ucFan11ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock11);
                }
            }
            else
            {
                m_ucFan11ErrorCount = 0;
            }

            if(uiFan12 < 500)
            {
                m_ucFan12ErrorCount++;
                if(m_ucFan12ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock12);
                }
            }
            else
            {
                m_ucFan12ErrorCount = 0;
            }

            if(uiFan14 < 500)   //HICC2_Doulas_0034
            {
                m_ucFan14ErrorCount++;
                if(m_ucFan14ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock14);
                }
            }
            else
            {
                m_ucFan14ErrorCount = 0;
            }

            if(uiFan15 < 500)
            {
                m_ucFan15ErrorCount++;
                if(m_ucFan15ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock15);
                }
            }
            else
            {
                m_ucFan15ErrorCount = 0;
            }

            if(uiFan17 < 500)
            {
                m_ucFan17ErrorCount++;
                if(m_ucFan17ErrorCount >= 5) //HICC2_Doulas_0117
                {
                    utilDataMgr_WriteGecLog(FanLock17);
                }
            }
            else
            {
                m_ucFan17ErrorCount = 0;
            }

            if( m_ucFan1ErrorCount  >= 5 || m_ucFan2ErrorCount  >= 5 || m_ucFan3ErrorCount  >= 5 || m_ucFan7ErrorCount >= 5 ||
                m_ucFan8ErrorCount  >= 5 || m_ucFan10ErrorCount >= 5 || m_ucFan14ErrorCount >= 5 ||
                m_ucFan11ErrorCount >= 5 || m_ucFan12ErrorCount >= 5 || m_ucFan15ErrorCount >= 5 || m_ucFan17ErrorCount >= 5) //HICC2_Doulas_0034
            {
                m_ucOE_JIG_Error = 1;

                LOG_MSG(db_ASSERT, "Fan Lock = (%d %d %d %d)(%d %d %d %d)(%d %d %d)\n", uiFan1, uiFan2, uiFan3, uiFan7, uiFan8, uiFan10, uiFan11, uiFan12, uiFan14, uiFan15, uiFan17);
            }
        }
        else
        {
            m_ucFan1ErrorCount = 0;  //HICC2_Doulas_0044 Modify
            m_ucFan2ErrorCount = 0;
            m_ucFan3ErrorCount = 0;
            m_ucFan7ErrorCount = 0;
            m_ucFan8ErrorCount = 0;
            m_ucFan10ErrorCount = 0;
            m_ucFan11ErrorCount = 0;
            m_ucFan12ErrorCount = 0;
            m_ucFan14ErrorCount = 0;
            m_ucFan15ErrorCount = 0;
            m_ucFan17ErrorCount = 0;
        }


        //check pump
        if((uiPump < 500) || (uiPump >5000))
        {
            m_ucPumpErrorCount++;

            if(m_ucPumpErrorCount >= 5)
            {
                m_ucOE_JIG_Error = 1;
                if(uiPump < 500)
                    utilDataMgr_WriteGecLog(LCSError01); //HICC2_Doulas_0117 pump lock
                else
                    utilDataMgr_WriteGecLog(LCSStall01); //HICC2_Doulas_0117 pump stall

                LOG_MSG(db_ASSERT, "Pump Fail\n");
            }
        }
        else
        {
            m_ucPumpErrorCount = 0;
        }

        //check DMD tec
        if(uiTecDMD < 300)  //HICC2_Doulas_0117 Modify
        {
            m_ucTecDMDErrorCount++;

            if(m_ucTecDMDErrorCount >= 7)
            {
                m_ucOE_JIG_Error = 1;
                utilDataMgr_WriteGecLog(TECAbnormal01); //HICC2_Doulas_0117 TEC current error
                LOG_MSG(db_ASSERT, "DMD Tec Fail\n");
            }
        }
        else
        {
            m_ucTecDMDErrorCount = 0;
        }

        //check DMD Temp
        if(iDMDTemp > 5000)//7500) //HICC2_Doulas_0064 Modify//HICC2_Doulas_0032
        {
            m_ucDMDTempErrorCount++;

            if(m_ucDMDTempErrorCount >= 5)
            {
                m_ucOE_JIG_Error = 1;
                utilDataMgr_WriteGecLog(DMDOverTemp); //HICC2_Doulas_0117 DMD over temp
                LOG_MSG(db_ASSERT, "DMD Temp Fail\n");
            }
        }
        else
        {
            m_ucDMDTempErrorCount = 0;
        }

        //check RLD tec
        #if 0 //HICC2_Doulas_0032
        if((uiRLDCurrent11 > 500) || (uiRLDCurrent12 > 500) || (uiRLDCurrent13 > 500) || (uiRLDCurrent14 > 500))
        {
            if(uiTecRLD < 500)
            {
                m_ucTecRLDErrorCount++;

                if(m_ucTecRLDErrorCount >= 7)
                {
                    m_ucOE_JIG_Error = 1;
                    LOG_MSG(db_ASSERT, "RLD Tec Fail\n");
                }
            }
            else
            {
                m_ucTecRLDErrorCount = 0;
            }
        }
        #endif

        if(m_ucOE_JIG_Error)
        {
            palSystem_PowerDown();
        }
    }


#endif /* OE_JIG */
    //G100_Steven_0080 start
   #if TEC_TESTER_JIG
       if(m_cTEC_Tester_LDEn_Cnt > 0 && m_cTEC_Tester_LDEn_Cnt < TEC_TESTER_COUNT)
       {
    	   m_cTEC_Tester_LDEn_Cnt++;

    	   if(TEC_TESTER_En == m_cTEC_Tester_LDEn_Cnt)
    	   {
    		   UINT8 cEnable = ets_ON;
    		   halMCU_TEC_TESTER_En_Set(cEnable);
    	   }

    	   //LOG_MSG(db_ALWAYS, "\r\nm_cTEC_Tester_LDEn_Cnt =  %d \r\n", m_cTEC_Tester_LDEn_Cnt);
       }
       else if (m_cTEC_Tester_LDEn_Cnt >= TEC_TESTER_COUNT)
       {
           UINT8 ucShutter = ets_OFF;
           m_cTEC_Tester_LDEn_Cnt = 0;
           palDataMgr_Data_Access(edcPICTURE_MUTE, edaWRITE_RAM_ONLY_WITH_ACTION, &ucShutter);
       }
   #endif
    //G100_Steven_0080 end

    if(bIsLightSourceOn) //A70LV_Larry_0075
    {
        UINT16 uiDataCode = 0;
		UINT8 ucFanFilter  = 0;
		UINT8 ucFanNum = Syscfg_Value_Get_Typeint(eFan_INFO_Num_defined_in_MCU); //HICC2_Doulas_0012
		UINT8 ucNum = 0;	//HICC2_Doulas_0012

        m_sPalIllumInfo.ucFanPollCount++;

        if(m_sPalIllumInfo.ucFanPollCount%10 == 0)
        {
            m_sPalIllumInfo.ucFanPollCount = 0;
			#if 1	//HICC2_Doulas_0012
			for(ucNum = 0 ; ucNum < ucFanNum ; ucNum++)
			{
				if(ucNum <= 15)
					palLANProcSendToLAN((eDATA_CODE)(edcFAN_RPM_01+ucNum));
				else if(ucNum <= 19)
					palLANProcSendToLAN((eDATA_CODE)(edcFAN_RPM_17+ucNum-16));
			}
			#else
            for(uiDataCode = edcFAN_RPM_01; uiDataCode <= edcFAN_RPM_16; uiDataCode++)
            {
                palLANProcSendToLAN((eDATA_CODE)uiDataCode);
            }
			#endif

            for(uiDataCode = edcLD_INFO_01; uiDataCode <= edcLD_INFO_14; uiDataCode++)
            {
                palLANProcSendToLAN((eDATA_CODE)uiDataCode);
            }

            for(uiDataCode = edcTHERMAL_SENSOR_1; uiDataCode <= edcTHERMAL_SENSOR_6; uiDataCode++)  //HICC2_Doulas_0066//HICC2_Doulas_0058 Modify//G100_Owen_0031
            {
                palLANProcSendToLAN((eDATA_CODE)uiDataCode);
            }

            for(uiDataCode = edcTECCURRENT_1; uiDataCode <= edcPUMP_RPM; uiDataCode++)
            {
                palLANProcSendToLAN((eDATA_CODE)uiDataCode);
            }

			palLANProcSendToLAN(edcSYSTEM_TEMPERATURE); //A70Gen2_Julie_0034
			palLANProcSendToLAN(edcAMBIENT_TEMPERATURE);

			halFanCtrl_Filter_Get(&ucFanFilter); //A70Gen2_Julie_0023
            if(m_sPalIllumInfo.ucFanFilter != ucFanFilter)
            {
                //avoid OPD log is too large
                m_sPalIllumInfo.ucFanFilter = ucFanFilter;
                palDataMgr_Data_Access(edcFAN_FILTER, edaWRITE_RAM_ONLY_NO_ACTION, &ucFanFilter);
            }

			if(palDataMgr_Access_Get_FanFilter() != ucFanFilter)
			{
				if((ucFanFilter == eFILTER_TYPE_NOFILTER_L) || (ucFanFilter == eFILTER_TYPE_NOFILTER_R)) //HICC2_Doulas_0060
				{
                    UINT32 ulMessage = ets_ON;
                    palDataMgr_Data_Access(edcUI_EVENT_UNKNOW_FILTER_MESSAGE, edaWRITE_THROUGH_WITH_ACTION, &ulMessage);
				}
			}

			for(uiDataCode = edcLIGHT_SENSOR_INFO_1; uiDataCode <= edcLIGHT_SENSOR_INFO_5; uiDataCode++)  //HICC2_Doulas_0008
            {
                palLANProcSendToLAN((eDATA_CODE)uiDataCode);
            }

			for(uiDataCode = edcFACTORY_FORMATTER_BLD_PWM_R; uiDataCode <= edcFACTORY_FORMATTER_RLD_PWM_Y; uiDataCode++)  //HICC2_Doulas_0008
            {
                palLANProcSendToLAN((eDATA_CODE)uiDataCode);
            }
        }
    }

#ifndef OE_JIG //A70LV_Larry_0322
    if(bIsLightSourceOn) //A70LV_Larry_0142 add
    {
        if(m_sPalIllumInfo.ucABPDelayStart == 0)
        {
            palIllumination_ABP_StatusProcess();
        }
        else
        {
            m_sPalIllumInfo.ucABPDelayStart--;
        }
    }
#endif /* OE_JIG */


#if 0      //H2 wait review
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A65_Owen_0002
    if(appGui_CurrentMenuIndexGet() == SERVICE_MODE_ADJUSTMENT_MENU_ICOUNT)     //ZU860_Clare_0084
#else
    if(appGui_IsMode_Adjust_Menu() == TRUE)     //ZU860_Clare_0084
#endif
    {
        if(eFUNC_CONTROL_ENABLE == palDataMgr_DataCode_Control(edcMODE_ADJ_TABLE_NUMBER))
        {
            palDataPath_ModeAdjusmenttEnableSetting();
        }
    }
    else
    {
        //palDataPath_ModeAdjusmenttDisableSetting();
    }
#endif

    return ILLUMINATION_PERIOD/POLL_PERIOD;
}


eEXEC_CODE palIllumination_BurnIn_Count_Start(UINT8 ucBurnin_enable, UINT8 ucLampOnTime, UINT16 uiBurnin_Cycle)
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    //reset ulStartUpLampPeriod for burn in lamp on count
    m_sPalIllumInfo.ulStartUpLampPeriod = 0;

    //update burnin finformation for lamp time count with burnin function
    m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable = ucBurnin_enable;
    m_sPalIllumInfo.sBurnIn_Info.ucLampOn = ucLampOnTime;
    m_sPalIllumInfo.sBurnIn_Info.uiBurnInCycle = uiBurnin_Cycle;
    halMCU_Auto_Power_On_Set(ucBurnin_enable);
    palLANProcSendToLAN(edcBURN_IN_STATE);

	return eEXEC_CODE_PASS;
}


eEXEC_CODE palIllumination_BurnIn_Count_Stop(void)
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    //reset ulStartUpLampPeriod for burn in lamp on count
    m_sPalIllumInfo.ulStartUpLampPeriod = 0;

    //update burnin finformation for lamp time count with burnin function
    m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable = ets_OFF;
    halMCU_Auto_Power_On_Set(ets_OFF);
    palLANProcSendToLAN(edcBURN_IN_STATE);

	return eEXEC_CODE_PASS;
}


UINT8 palIllumination_BurnIn_Enable(void) //A70LV_Larry_0016
{
	return m_sPalIllumInfo.sBurnIn_Info.ucBurnInEnable;
}

// ==============================================================================
// FUNCTION NAME: palIllumination_Shutter
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
// 2018/01/15, Larry Create
// --------------------
// ==============================================================================
UINT8 palIllumination_Shutter(UINT8 ucEnable)
{
//    if(m_sPalIllumInfo.ucSmoothOn)
//    {
//        return 1;
//    }

    if(ucEnable)
    {
		UINT8 ucRetry = 0;	//G100_Clare_0004
		UINT8 ucLDOff = 0;	//G100_Clare_0004
		UINT8 ucFade_Out_Step = 0;	//G100_Clare_0004
        if(palDataPath_GetPanelChangeState() != ets_ON) //A35G2_BRC_Casper_0068
        {
            palLedProc_Shutter_LED_Set(eLED_SHUTTER_ON);
        }
        palCoreVar_SetLightSourceOnVar(FALSE);
        //halLDCtrl_LD_SourceEnable_Set(eLDBANK_A70LV, 0);
		do{	//G100_Clare_0004
			palDataMgr_Data_Access(edcFADE_OUT, edaREAD, (void*)&ucFade_Out_Step);
        #if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)//A35G2_BRC_Casper_0077//A35G2_OTM_Coda_0002
            if(palDataPath_GetPanelChangeState() == ets_ON)
            {
                ucFade_Out_Step = FADE_OUT_TIMER_QUICK;
                b2D3DFlag = FALSE;
            }
        #else
            if(ucFade_Out_Step == FADE_TIMER_MIN_VALUE) //HICC2_Doulas_0071
            {
                ucFade_Out_Step = FADE_OUT_TIMER_QUICK;
            }
            else
            {
            	ucFade_Out_Step = ucFade_Out_Step*FADE_TIMER_STEP_VALUE;
            }
        #endif
			palFormatterMgr_Smooth_Enable_Set(FALSE, ucFade_Out_Step);
			MS_SLEEP(5);
			palFormatterMgr_Smooth_Enable_Get(&ucLDOff);
		}while((ucRetry++ < 5)&&(ucLDOff == FALSE));

        return 1;
    }
    else
    {
        if(
            ((palIllumination_LensDetectionGet() == 0) && (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70))   //Type 70 case
            || ((palIllumination_LensDetectionGet() == 0) && (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60)) //HICC2_Doulas_0030 H60 case
            || (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A35)  //Type 35 case //HICC2_Doulas_0030
        )
        {
			UINT8 ucLDOn = 0;
			UINT8 ucRetry = 0;
			UINT8 ucFade_In_Step = 0;	//G100_Clare_0004
            palLedProc_Shutter_LED_Set(eLED_SHUTTER_OFF);
			#if 0//def OE_JIG	//G100_Clare_0006
			halFormatter_PWM_Set(0, 100);
			halFormatter_PWM_Set(1, 100);
			halFormatter_PWM_Set(2, 100);
			halFormatter_PWM_Set(3, 100);
			halFormatter_PWM_Set(4, 100);
			halFormatter_PWM_Set(5, 100);
			#endif /* OE_JIG */
            palCoreVar_SetLightSourceOnVar(TRUE);
			do{	//G100_Clare_0004
				palDataMgr_Data_Access(edcFADE_IN, edaREAD, (void*)&ucFade_In_Step);
	        #if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A35G2_BRC_Casper_0077//A35G2_OTM_Coda_0002
                if(!b2D3DFlag)
                {
                    ucFade_In_Step = FADE_IN_TIMER_QUICK;
                    b2D3DFlag = TRUE;
                }
            #else
                if(ucFade_In_Step == FADE_TIMER_MIN_VALUE) //HICC2_Doulas_0071
                {
                	ucFade_In_Step = FADE_IN_TIMER_QUICK;
                }
                else
                {
                	ucFade_In_Step = ucFade_In_Step*FADE_TIMER_STEP_VALUE;
                }
            #endif
                if(m_sPalIllumInfo.ucSmoothOn != SMOOTH_ON_DONE) //smooth on in first power on
                {
                    ucFade_In_Step = 4*FADE_TIMER_STEP_VALUE;
                }

				palFormatterMgr_Smooth_Enable_Set(TRUE, ucFade_In_Step);
				MS_SLEEP(5);
				palFormatterMgr_Smooth_Enable_Get(&ucLDOn);
			}while((ucRetry++ < 5)&&(ucLDOn == FALSE));
            //halLDCtrl_LD_SourceEnable_Set(eLDBANK_A70LV, 1);

            if((Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||
                (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60))   //HICC2_Doulas_0030
            {
                m_sPalIllumInfo.cLensPlug = 0;
            }

            return 0;
        }

        return 1;
    }
}

UINT8 palIllumination_DirectShutter(UINT8 ucEnable)
{
    if(m_sPalIllumInfo.ucSmoothOn >= SMOOTH_ON_READY)
    {
        //還在smooth on階段,禁用shutter
        return 1;
    }

    if(ucEnable)
    {
        //if(palDataPath_GetPanelChangeState() != ets_ON) //A35G2_BRC_Casper_0068
        {
            palLedProc_Shutter_LED_Set(eLED_SHUTTER_ON);
        }
        palCoreVar_SetLightSourceOnVar(FALSE);
        palFormatterMgr_Smooth_Enable_Set(FALSE, 0);

        return 1;
    }
    else
    {
        if(((palIllumination_LensDetectionGet() == 0) && (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70))   //Type 70 case
            || ((palIllumination_LensDetectionGet() == 0) && (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60)) //HICC2_Doulas_0030 H60 case
            || (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A35)  //Type 35 case //HICC2_Doulas_0030
        )
        {
            palLedProc_Shutter_LED_Set(eLED_SHUTTER_OFF);
            palCoreVar_SetLightSourceOnVar(TRUE);
            palFormatterMgr_Smooth_Enable_Set(TRUE, 0);

            if((m_sPalIllumInfo.ucSmoothOn == SMOOTH_ON_DONE) &&
                ((Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_A70) ||
                 (Syscfg_Value_Get_Typeint(eMotorLensType) == eMOTOR_LENSTYPE_H60))
                )   //HICC2_Doulas_0030
            {
                m_sPalIllumInfo.cLensPlug = 0;
            }

            return 0;
        }

        return 1;
    }
}

eEXEC_CODE palIllumination_Init_LDHoursSet(UINT32 ulHours)   //A70LV_Doulas_0128 //A70LV_Larry_0268
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalIllumInfo.ulLampPeriod   = ulHours;

    //if(bWriteEEPROM)
    //{
        //palDataMgr_Data_Access(edcLD_HOURS, edaWRITE_THROUGH_NO_ACTION, &ulHours);
        //palDataMgr_Access_LD_Hours(edaWRITE_THROUGH_NO_ACTION, &ulHours);
    //}

    m_sPalIllumInfo.uiLampUpdateCount = 0;
    m_sPalIllumInfo.ulStartUpLampPeriod = 0;

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palIllumination_Init_TotalProjectHoursSet(UINT32 ulHours)   //A70LV_Doulas_0128 //A70LV_Larry_0268
{
    LOG_MSG(db_APP_ILLUMINATION, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_sPalIllumInfo.ulTotalProjectorHours   = ulHours;

    //if(bWriteEEPROM)
    //{
        //palDataMgr_Data_Access(edcTOTAL_PROJECTOR_HOURS, edaWRITE_THROUGH_NO_ACTION, &ulHours);
        //palDataMgr_Access_TotalProjectorHours(edaWRITE_THROUGH_NO_ACTION, &ulHours);
    //}

    m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount = 0;

    return eEXEC_CODE_PASS;
}

UINT32 palIllumination_TotalProjectSec_Get(void) //A70LV_Larry_0276
{
    return m_sPalIllumInfo.ulTotalProjectorHours*60 + m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount;

#if 0
    if(m_sPalIllumInfo.ulTotalProjectorHours > m_sPalIllumInfo.ulTotalProjectorHours_2)
    {
        return m_sPalIllumInfo.ulTotalProjectorHours*60 + m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount;
    }
    else
    {
        return m_sPalIllumInfo.ulTotalProjectorHours_2*60 + m_sPalIllumInfo.ulTotalProjectorHoursUpdateCount;
    }
#endif /* 0 */
}

void palIllumination_LensDetectionSet(UINT8 ucEnable)
{
    m_sPalIllumInfo.ucLensDetection = ucEnable;
}

UINT8 palIllumination_LensDetectionGet(void)
{
    // 1 no lens
    // 0 has lens

    UINT8 cLensPlug = 0;

    if(m_sPalIllumInfo.ucLensDetection)
    {
        cLensPlug = halMotor_LensPlug_Get();
    }
    else
    {
        cLensPlug = 0;
    }

    return cLensPlug;
}

void palIllumination_InstantOff(void)
{
    if(Board_Stage_Get() > EVT_STAGE)   //G100_Owen_0076
    {
        palSystem_12VPwrLostSet(TRUE);
        m_sPalIllumInfo.bIllumEnabled = FALSE;
        palCoreVar_SetLightSourceOnVar(FALSE);
    }
}

eEXEC_CODE util_RLD_ValueCheck(UINT32 dwY_Hi, UINT32 dwY_Lo, UINT32 dwX_In, UINT32 dwX_Hi, UINT32 dwX_Lo) 	//G100_Doulas_0023
{
    if(dwX_Hi == dwX_Lo)
    {   // avoid overflow
        return eEXEC_CODE_FAIL;
    }
	else if((dwY_Hi == 0) ||
		    (dwY_Lo == 0) ||
		    (dwX_In == 0) ||
		    (dwX_Hi == 0) ||
		    (dwX_Lo == 0) )
	{
		return eEXEC_CODE_FAIL;
	}
	else if((dwX_Hi > ABC_LIGHT_SENSOR_MAXIMUM) || (dwX_Lo > ABC_LIGHT_SENSOR_MAXIMUM))
	{
		return eEXEC_CODE_FAIL;
	}

    return eEXEC_CODE_PASS;
}

UINT32 palIllumination_CurrentRunTimeGet(void)
{
    return m_sPalIllumInfo.ulCurrentRunTime;
}


#ifdef CUSTOM_BARCO
//G100_Steven_0084
void palIllumination_TEC_Gating(UINT32 dwLD_On)
{
	UINT8 cGating_STEP = 0;
	BOOL bCheckPass = TRUE;

	if( ((dwLD_On >= 10 && dwLD_On % 10 == 0) && b_TEC_First_F == 1) || (dwLD_On >= 9 && dwLD_On % 9 == 0)) //G100_Steven_0087
	{
		UINT16 wSystemTemp = palDataMgr_Access_Get_Thermal_Sensor(2);
		UINT16 wDMDTemp = palDataMgr_Access_Get_Thermal_Sensor(0);
		UINT16 wDMDTEC_Current = palDataMgr_Access_Get_TEC_Current(0);

		LOG_MSG(db_APP_ILLUMINATION, "TEC_Gating: SysTemp :%d, DMDTEC :%d, TECCurrent :%d\r\n", wSystemTemp, wDMDTemp, wDMDTEC_Current);
		//cTEC_Gating_Cnt++;

		if( wSystemTemp < TEC_GATING_TEMP[TEC_Gating_STEP0])
		{
			cGating_STEP = TEC_Gating_STEP0;
		}
		else if( wSystemTemp >= TEC_GATING_TEMP[TEC_Gating_STEP0] && wSystemTemp < TEC_GATING_TEMP[TEC_Gating_STEP1])
		{
			cGating_STEP = TEC_Gating_STEP1;
		}
		else if( wSystemTemp >= TEC_GATING_TEMP[TEC_Gating_STEP1] && wSystemTemp < TEC_GATING_TEMP[TEC_Gating_STEP2])
		{
			cGating_STEP = TEC_Gating_STEP2;
		}
		else if( wSystemTemp >= TEC_GATING_TEMP[TEC_Gating_STEP2] && wSystemTemp < TEC_GATING_TEMP[TEC_Gating_STEP3])
		{
			cGating_STEP = TEC_Gating_STEP3;
		}
		else if( wSystemTemp >= TEC_GATING_TEMP[TEC_Gating_STEP3] && wSystemTemp < TEC_GATING_TEMP[TEC_Gating_STEP4])
		{
			cGating_STEP = TEC_Gating_STEP4;
		}
		else if( wSystemTemp >= TEC_GATING_TEMP[TEC_Gating_STEP4] && wSystemTemp < TEC_GATING_TEMP[TEC_Gating_STEP5])
		{
			cGating_STEP = TEC_Gating_STEP5;
		}
		else
		{
			cGating_STEP = TEC_Gating_NUMBER;
		}

		if(cGating_STEP < TEC_Gating_NUMBER)
		{

			if(palDataMgr_Model_ID_Get() == MODULE_TYPE_ID2_PLATFORM)
			{	//LU
				if( wDMDTemp > TEC_GATING_LU_DMDTEMP[cGating_STEP] )
				{
					bCheckPass = FALSE;
				}
				else if( wDMDTEC_Current > TEC_GATING_LU_DMDCurrent[cGating_STEP] )
				{
					bCheckPass = FALSE;
				}
			}
		}

		if(bCheckPass == FALSE)
		{
			if(b_TEC_First_F == 1)  //G100_Steven_0087 start
			{
				LOG_MSG(db_APP_ILLUMINATION, "TEC_Gating fail 2...\r\n");

				if( cTEC_Gating_PassCnt >=1 )
				{
					b_TEC_Check_F = 1;
				}
				cTEC_Gating_PassCnt = 0;
				b_TEC_First_F = 0;
			}
			else
			{
				LOG_MSG(db_APP_ILLUMINATION, "TEC_Gating fail 1...\r\n");
				b_TEC_First_F = 1;
			}  //G100_Steven_0087 end
		}
		else
		{
			cTEC_Gating_PassCnt++;
			b_TEC_First_F = 0;
			LOG_MSG(db_APP_ILLUMINATION, "TEC_Gating pass %d\r\n", cTEC_Gating_PassCnt);

		}
	}
}

void palIllumination_TEC_Result(void)//save result to eeprom
{

	UINT16 wResult = 0;

	//LOG_MSG(db_APP_ILLUMINATION, "----TEC_Result CYCLE: %d, Result: %d \r\n", cTEC_Gating_PassCnt,  wResult);

	if(cTEC_Gating_PassCnt == 0) // fail
	{
		wResult = 0;
	}
	else if( (0 < cTEC_Gating_PassCnt && cTEC_Gating_PassCnt  < 5) || b_TEC_Check_F == 1) // not enough 5 time
	{
		wResult = 9;
	}
	else if( cTEC_Gating_PassCnt >= 5) // pass
	{
		wResult = 1;
	}

	LOG_MSG(db_APP_ILLUMINATION, "TEC_Result CYCLE: %d, Result: %d, CheckF: %d \r\n", cTEC_Gating_PassCnt,  wResult, b_TEC_Check_F);

	palIllumination_TEC_Gating_InfoSet(wResult);

}//G100_Steven_0084


UINT16 palIllumination_TEC_Gating_ResultGet( UINT8 cIndex) //G100_Steven_0085
{
	return sTEC_GATING.ucTEC_GATING_CYCLE[cIndex];
}
#endif

void palIllumination_OPDSnapshotPowerOnDelay(UINT8 cDelay)
{
    m_sPalIllumInfo.ucSnapshotPowerOn = cDelay;
}

void palIllumination_OPDSnapshot_Recode(UINT8 cIndex) //A35G2_CDS_Larry_0020 //A65_OPTOMA_Julie_0028
{
	m_OPD_EngineLog_Index = cIndex;
	m_sPalIllumInfo.ucSnapshotUpdatePeriod = 30;
}

UINT8 palIllumination_LVPS_110VDetGet(void)
{
    return m_sPalIllumInfo.ucLVPS_110VDet;
}


