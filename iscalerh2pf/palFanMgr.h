#ifndef _PALFANMGR_H_
#define _PALFANMGR_H_

eRESULT palFanMgr_HighAltModeSet(UINT8 cData);
eRESULT palFanMgr_PowerModeSet(UINT8 ucPowerMode);  //unused
eRESULT palFanMgr_Liquid_Speed_Get(UINT8* pcData);
eRESULT palFanMgr_Fan_Speed_Get(UINT8* pcData);
eRESULT palFanMgr_Fan_Lock_Indicate_Get(UINT8* pcData);
eRESULT palFanMgr_Fan_Duty_Get(UINT8* pcData);
eRESULT palFanMgr_Thermal_Sensor_Get(UINT8* pcData);
eRESULT palFanMgr_NoFanControlSet(void);
eRESULT palFanMgr_Fan_Speed_All_Get(UINT8* pcData);
eRESULT palFanMgr_Fan_Duty_All_Get(UINT8* pcData);
eRESULT palFanMgr_Thermal_Sensor_All_Get(UINT8* pcData);
eRESULT palFanMgr_LD_OverTemperatureNumber_Get(UINT8* pcData);
eRESULT palFanMgr_Filter_Get(UINT8* pcData);
eRESULT palFanMgr_Wheel_Speed_Get(UINT8* pcData);


#endif /*_PALFANMGR_H_*/
