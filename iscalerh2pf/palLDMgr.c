#include "Common.h"
#include "palLDMgr.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "utilStorageCfg.h"
#include "halFormatter.h"
#include "utilDbgMsg.h"

static UINT32 uiLightingSec = 0;
static UINT8  m_aucLDEnable[ePAL_LD_BANK_NUMBERS] = {0xFF,0xFF,0xFF}; //A70LV_Larry_0062


eRESULT palLDMgr_Version_Get(UINT8* pcVersion)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Version_Get(pcVersion);

    return eResult;
}

eRESULT palLDMgr_LD_Voltage_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Voltage_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_LD_Current_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Current_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_LD_Temperature_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Temperature_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_LD_SourceEnable_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_SourceEnable_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_LD_SourceEnable_Set(ePAL_LD_ID eDriverID, UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_SourceEnable_Set(eDriverID, ucData);

    return eResult;
}


eRESULT palLDMgr_Fan_RPM_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Fan_RPM_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_Tec_Duty_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Tec_Duty_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Tec_Current_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Tec_Current_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Tec_Current_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Tec_Temperature_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Tec_Temperature_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Tec_Temperature_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Power_Status_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Power_Status_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Power_Status_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Power_Status_Set
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8 ucPowerMode:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Power_Status_Set(ePAL_LD_ID eDriverID, UINT8 ucPowerMode)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Power_Status_Set(eDriverID, ucPowerMode);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Failure_Status_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Failure_Status_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Failure_Status_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_High_Altitude_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_High_Altitude_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = palLDMgr_High_Altitude_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Orientation_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Orientation_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Orientation_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Light_Sensor_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0007
// --------------------
// ==============================================================================
eRESULT palLDMgr_Light_Sensor_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Light_Sensor_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_Power_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0008
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_Power_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Power_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_Power_Set
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8 ucLDPower:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0008
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_Power_Set(ePAL_LD_ID eDriverID, UINT8* pucLDPower)  // A70LV_Eric.C_0015 pucLDPower
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Power_Set(eDriverID, pucLDPower);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_PowerAll_Set
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pucLDPower:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_PowerAll_Set(ePAL_LD_ID eDriverID, UINT8* pucLDPower)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_PowerAll_Set(eDriverID, pucLDPower);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Light_Module_Status_Set
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8 ucLightEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0011
// --------------------
// ==============================================================================
eRESULT palLDMgr_Light_Module_Status_Set(ePAL_LD_ID eDriverID, UINT8 ucLightEnable)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Light_Module_Status_Set(eDriverID, ucLightEnable);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Duration_of_Lighting_Get
// DESCRIPTION:
//
//
// Params:
//
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0013
// --------------------
// ==============================================================================
UINT32 palLDMgr_Duration_of_Lighting_Get(void)
{
    return halLDCtrl_Duration_of_Lighting_Get();
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Duration_of_Lighting_Set
// DESCRIPTION:
//
//
// Params:
//
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0013
// --------------------
// ==============================================================================
void palLDMgr_Duration_of_Lighting_Set(UINT32 uiSec)
{
    halLDCtrl_Duration_of_Lighting_Set(uiSec);
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_CeilingMount_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0062
// --------------------
// ==============================================================================
eRESULT palLDMgr_CeilingMount_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_CeilingMount_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Bank_Enable_Set
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
// 2018/01/08, Larry Create //A70LV_Larry_0069
// --------------------
// ==============================================================================
eRESULT palLDMgr_Bank_Enable_Set(ePAL_LD_BANK_ENALE_ITEM eBank, BOOL bEnable)
{
    eRESULT eResult = rcSUCCESS;

#if defined(PLATFORM_A35G2)
    eResult = halLDCtrl_Bank_Enable_Set((eLD_BANK_ENALE_ITEM)eBank, bEnable);
#else
	eResult = halLDCtrl_Bank_Enable_Set((eLD_BANK_ITEM)eBank, bEnable);    //HICC2_Doulas_0001 Add
#endif

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Bank_Enable_Get
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
// 2018/01/08, Larry Create //A70LV_Larry_0069
// --------------------
// ==============================================================================
BOOL palLDMgr_Bank_Enable_Get(ePAL_LD_BANK_ENALE_ITEM eBank)
{
    return halLDCtrl_Bank_Enable_Get(eBank);
}

BOOL palLDMgr_BLD_Bank_Enable(BOOL bEnable)
{
    return halLDCtrl_BLD_Bank_Enable(bEnable);
}

BOOL palLDMgr_RLD_Bank_Enable(BOOL bEnable)
{
    return halLDCtrl_RLD_Bank_Enable(bEnable);
}

eRESULT palLDMgr_LD_GroupEnable(UINT8 ucGroup, UINT8 ucEnable) //A70LV_Larry_0142
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_GroupEnable(ucGroup, ucEnable);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_Voltage_All_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_Voltage_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Voltage_All_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_Current_All_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_Current_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Current_All_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_Temperature_All_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_Temperature_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Temperature_All_Get(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Tec_Voltage_All_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/18, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Tec_Voltage_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Tec_Voltage_All_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Tec_Current_All_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Tec_Current_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Tec_Current_All_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_Tec_Temperature_All_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Tec_Temperature_All_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_Tec_Temperature_All_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_ABP_ModeSet
// DESCRIPTION:
//
//
// Params:
// UINT8 cData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_ABP_ModeSet(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_ABP_ModeSet(cData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_TargetBLDLightSet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_TargetBLDLightSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_TargetBLDLightSet(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_TargetBLDPWMSet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_TargetBLDPWMSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_TargetBLDPWMSet(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_TargetRLDLightSet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_TargetRLDLightSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_TargetRLDLightSet(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_TargetRLDPWMSet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/28, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_TargetRLDPWMSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_TargetRLDPWMSet(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_ABP_AutoTuning_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/29, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_ABP_AutoTuning_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_ABP_AutoTuning_Get(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_DisableShutdown
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
// 2018/06/05, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_DisableShutdown(void)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_DisableShutdown();

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_Count_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/08, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_Count_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_Count_Get(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_G_Sensor_Data_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// ZU860_Doulas_0059
// --------------------
// ==============================================================================
eRESULT palLDMgr_G_Sensor_Data_Get(ePAL_LD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_G_Sensor_Data_Get(eDriverID, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_SourceBlanking_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/12/27, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_SourceBlanking_Get(ePAL_LD_ID eDriverID, UINT8* pcData)        //A70LV_Doulas_0294
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_SourceBlanking_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_LD_SourceBlanking_Set(ePAL_LD_ID eDriverID, UINT8 ucData)   //A70LV_Doulas_0294
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_SourceBlanking_Set(eDriverID, ucData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LD_WheelBlanking_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/03/28, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LD_WheelBlanking_Get(ePAL_LD_ID eDriverID, UINT8* pcData)        //A70LV_Doulas_0306
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_WheelBlanking_Get(eDriverID, pcData);

    return eResult;
}


eRESULT palLDMgr_LD_WheelBlanking_Set(ePAL_LD_ID eDriverID, UINT8 ucData)   //A70LV_Doulas_0306
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LD_WheelBlanking_Set(eDriverID, ucData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_First_LD_On_Set
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8 ucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/03/26, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_First_LD_On_Set(ePAL_LD_ID eDriverID, UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_First_LD_On_Set(eDriverID, ucData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LS_RLD_Enable_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 cData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/26, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LS_RLD_Enable_Set(UINT8 cData)		//G100_Doulas_0010
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LS_RLD_Enable_Set(cData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LightSensorT1_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LightSensorT1_Set(UINT8* pcData)		//G100_Doulas_0012 Set T1
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LightSensorT1_Set(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LightSensorT0_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LightSensorT0_Set(UINT8* pcData)		//G100_Doulas_0012 Set T0
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LightSensorT0_Set(pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_LightSensorTrigger_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 cData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/31, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_LightSensorTrigger_Set(UINT8 cData)		//G100_Doulas_0012 trigger LS quickly receive values (10 times)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LightSensorTrigger_Set(cData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_T1LightSensorValue_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/31, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_T1LightSensorValue_Get(ePAL_LD_ID eDriverID, UINT8* pcData)		//G100_Doulas_0012 get light sensor value(for T1)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_T1LightSensorValue_Get(eDriverID, pcData);

    return eResult;
}



// ==============================================================================
// FUNCTION NAME: palLDMgr_T0LightSensorValue_Get
// DESCRIPTION:
//
//
// Params:
// ePAL_LD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_T0LightSensorValue_Get(ePAL_LD_ID eDriverID, UINT8* pcData)		//G100_Doulas_0012 get light sensor value(for T0)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_T0LightSensorValue_Get(eDriverID, pcData);

    return eResult;
}


//G100_Clare_0011, add, >>>
// ==============================================================================
// FUNCTION NAME: palLDMgr_Fan_Mode_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 cData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/09/22, Clare Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_Fan_Mode_Set(eCM_POWER_MODE_ID eLightMode, eCM_PICTURE_SETTINGS_ID ePictureSetting, eCM_PICTURE_SETTINGS_ID eUserModePicSetting)
{
    eRESULT eResult = rcSUCCESS;
	UINT8 cData = eFAN_PWRMODE_SUPER;

	LOG_MSG(db_HAL_FAN, "\r\n edcPICTURE_SETTINGS %d\r\n", ePictureSetting);
	LOG_MSG(db_HAL_FAN, " edcPOWER_MODE %d\r\n", eLightMode);

	if((eLightMode == eCM_POWER_MODE_CONSTANT_POWER) //G100_Steven_0030
		&&((ePictureSetting == eCM_PICTURE_SETTINGS_ENHANCED) || (ePictureSetting == eCM_PICTURE_SETTINGS_USER && eUserModePicSetting == eCM_PICTURE_SETTINGS_ENHANCED)))//eIFC_ApplyUserMode_SUPERBRIGHT)))   //G100_Doulas_0066 Modify
	{
		cData = eFAN_PWRMODE_SUPER;
	}
	else
	{
		if((eLightMode == eCM_POWER_MODE_ECO1) || (eLightMode == eCM_POWER_MODE_ECO2) || (eLightMode == eCM_POWER_MODE_QUIET_MODE))
		{
			cData = eFAN_PWRMODE_ECO;
		}
		else
		{
			cData = eFAN_PWRMODE_FULL;
		}
	}

    eResult = halLDCtrl_Fan_Mode_Set(&cData);

    return eResult;
}
//G100_Clare_0011, add, <<<

eRESULT palLDMgr_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_OPD_Register_Set(reg, size, pcData);

    return eResult;
}


eRESULT palLDMgr_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_OPD_Register_Get(reg, size, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_ProjectorID_Set
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
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_ProjectorID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_ProjectorID_Set(cData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_CustomerID_Set
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
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_CustomerID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_CustomerID_Set(cData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: palLDMgr_PlatformID_Set
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
// 2021/11/04, Larry Create
// --------------------
// ==============================================================================
eRESULT palLDMgr_PlatformID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_PlatformID_Set(cData);

    return eResult;
}

eRESULT palLDMgr_LightSensor_Position_Set(UINT8 cData) //HICC2_Jacky_0001
{
    eRESULT eResult = rcSUCCESS;

    eResult = halLDCtrl_LightSensorPosition_Set(cData);

    return eResult;
}


