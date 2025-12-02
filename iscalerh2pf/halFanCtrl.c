// ===============================================================================
// FILE NAME: halFanCtrl.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/12/02, Larry Create
// --------------------
// ===============================================================================

#include "halFanCtrlAPI.h"
#include "dvLDDriver.h"   // A70LV_Eric.C_0005
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"

// ==============================================================================
// FUNCTION NAME: halFanCtrl_HighAltModeSet
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
// 2017/12/01, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_HighAltModeSet(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_HIGH_ALTITUDE, eFMT_MSG_HIGH_ALTITUDE_SZ, (UINT8*)&cData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFan_PowerModeSet
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
// 2019/08/26, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_PowerModeSet(UINT8 ucPowerMode)
{
    eRESULT eResult = rcSUCCESS;

#ifdef ENABLE_MCU_CLI
//    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eLD_EVENT_LIGHTSOURCE_MODE, (UINT16)sizeof(UINT8), (UINT8*)&ucPowerMode);
#endif /* ENABLE_MCU_CLI */

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Liquid_Speed_Get
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0005
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Liquid_Speed_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LIQUID_SPEED, eFMT_MSG_LIQUID_SPEED_SZ, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halFanCtrl_Fan_Speed_Get
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0003
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Fan_Speed_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAN_SPEED, eFMT_MSG_FAN_SPEED_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Fan_Lock_Indicate_Get
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0003
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Fan_Lock_Indicate_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAN_LOCK_INDICATE, eFMT_MSG_FAN_LOCK_INDICATE_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Fan_Duty_Get
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0003
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Fan_Duty_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAN_DUTY, eFMT_MSG_FAN_DUTY_RPM_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Thermal_Sensor_Get
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
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
eRESULT halFanCtrl_Thermal_Sensor_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_THERMAL_SENSOR, eFMT_MSG_THERMAL_SENSOR_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_NoFanControl
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
eRESULT halFanCtrl_NoFanControlSet(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucData = 1;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_FAN_NO_FAN_CTRL, eFMT_MSG_FAN_NO_FAN_CTRL_SZ, &ucData);

    return eResult;
}

eRESULT halFanCtrl_OperationModeSet(void)//HICC2_Julie_0046
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucData = 0;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_FAN_NO_FAN_CTRL, eFMT_MSG_FAN_NO_FAN_CTRL_SZ, &ucData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Fan_Speed_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Fan_Speed_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAN_SPEED_ALL, eFMT_MSG_FAN_SPEED_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Fan_Duty_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Fan_Duty_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAN_DUTY_ALL, eFMT_MSG_FAN_DUTY_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Thermal_Sensor_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Thermal_Sensor_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_THERMAL_SENSOR_ALL, eFMT_MSG_THERMAL_SENSOR_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_LD_OverTemperature_Get
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
// 2018/07/06, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_LD_OverTemperatureNumber_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_OVER_TEMPERATURE, eFMT_MSG_LD_OVER_TEMPERATURE_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Filter_Get
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
// 2018/07/27, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Platform_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_PLATFORM_ID, eFMT_MSG_PLATFORM_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Filter_Get
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
// 2018/07/27, Larry Create
// --------------------
// ==============================================================================
eRESULT halFanCtrl_Filter_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FILTER_TYPE, 1, pcData);

    return eResult;
}

eRESULT halFanCtrl_Wheel_Speed_Get(UINT8* pcData)   //G100_Julie_0014
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_WHEEL_SPEED, eFMT_MSG_WHEEL_SPEED_SZ, pcData);

    return eResult;
}

eRESULT halFanCtrl_Platform_Set(UINT8 cData) //A70Gen2_Julie_0001
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_PLATFORM_ID, eFMT_MSG_PLATFORM_SZ, (UINT8*)&cData);

    if(eResult == rcSUCCESS)
    {
        MS_SLEEP(50);
        //等待系統設定完成
    }
    return eResult;
}


