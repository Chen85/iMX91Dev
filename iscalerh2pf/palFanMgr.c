#include "Common.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "halFanCtrlAPI.h"
#include "palFanMgr.h"


eRESULT palFanMgr_HighAltModeSet(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_HighAltModeSet(cData);

    return eResult;
}

//unused
eRESULT palFanMgr_PowerModeSet(UINT8 ucPowerMode)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_PowerModeSet(ucPowerMode);

    return eResult;
}


eRESULT palFanMgr_Liquid_Speed_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Liquid_Speed_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Fan_Speed_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Fan_Speed_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Fan_Lock_Indicate_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Fan_Lock_Indicate_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Fan_Duty_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Fan_Duty_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Thermal_Sensor_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Thermal_Sensor_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_NoFanControlSet(void)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_NoFanControlSet();

    return eResult;
}


eRESULT palFanMgr_Fan_Speed_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Fan_Speed_All_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Fan_Duty_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Fan_Duty_All_Get(pcData);

    return eResult;
}

eRESULT palFanMgr_Thermal_Sensor_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Thermal_Sensor_All_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_LD_OverTemperatureNumber_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_LD_OverTemperatureNumber_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Filter_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Filter_Get(pcData);

    return eResult;
}


eRESULT palFanMgr_Wheel_Speed_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFanCtrl_Wheel_Speed_Get(pcData);

    return eResult;
}



