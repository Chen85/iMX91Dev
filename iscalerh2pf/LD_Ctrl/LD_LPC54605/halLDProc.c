// ===============================================================================
// FILE NAME: halLDProc.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ===============================================================================

#include "Common.h"
#include "utilStorageCfg.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
#include "utilHostAPI.h"
#include "dvMCUDriver.h"
#include "halLDProc.h"
#include "halFormatter.h" //G100_Doulas_0010 add ABP command to DDP


static UINT32 uiLightingSec = 0;
static UINT8  m_aucLDEnable[eLD_BANK_NUMBERS] = {0xFF,0xFF,0xFF}; //A70LV_Larry_0062

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Version_Get
// DESCRIPTION:
//
//
// Params:
// eLDBANK_ID eDriverID:
// BYTE* pcVersion:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Version_Get(UINT8* pcVersion)
{
    eRESULT eResult = rcSUCCESS;

    //eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_MCU_VERSION, eFMT_MSG_MCU_VERSION_SZ, pcVersion);

    eResult = dvLDDriverRegRead(eCMD_MODULE_FMT, eFMT_MSG_MCU_VERSION, eFMT_MSG_MCU_VERSION_SZ, pcVersion);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Voltage_Get
// DESCRIPTION:
//
//
// Params:
// eLDBANK_ID eDriverID:
// BYTE* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Voltage_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_VOLTAGE, eFMT_MSG_LD_CURRENT_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Current_Get
// DESCRIPTION:
//
//
// Params:
// eLDBANK_ID eDriverID:
// BYTE* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Current_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_CURRENT, eFMT_MSG_LD_CURRENT_SZ, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Temperature_Get
// DESCRIPTION:
//
//
// Params:
// eLDBANK_ID eDriverID:
// BYTE* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/26, Casper Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Temperature_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_TEMPERATURE, eFMT_MSG_LD_TEMPERATURE_SZ, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_SourceEnable_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_SourceEnable_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SOURCE_ENABLE, eFMT_MSG_LIGHT_SOURCE_ENABLE_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_SourceEnable_Set
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
// 2020/06/10, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_SourceEnable_Set(eLD_ID eDriverID, UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SOURCE_ENABLE, eFMT_MSG_LIGHT_SOURCE_ENABLE_SZ, &ucData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Fan_RPM_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Fan_RPM_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAN_SPEED, eFMT_MSG_FAN_SPEED_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Tec_Duty_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Tec_Duty_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_TEC_DUTY, eFMT_MSG_TEC_DUTY_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Tec_Current_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Tec_Current_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_TEC_CURRENT, eFMT_MSG_TEC_CURRENT_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Tec_Temperature_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Tec_Temperature_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_TEC_TEMPERATURE, eFMT_MSG_TEC_TEMPERATURE_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Power_Status_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Power_Status_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_POWER_STATUS, eFMT_MSG_POWER_STATUS_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Power_Status_Set
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
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
eRESULT halLDCtrl_Power_Status_Set(eLD_ID eDriverID, UINT8 ucPowerMode)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_POWER_STATUS, eFMT_MSG_POWER_STATUS_SZ, &ucPowerMode);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Failure_Status_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Failure_Status_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_FAILURE_STATUS, eFMT_MSG_FAILURE_STATUS_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_High_Altitude_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_High_Altitude_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_HIGH_ALTITUDE, eFMT_MSG_HIGH_ALTITUDE_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Orientation_Get
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
// 2017/10/19, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Orientation_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_ORIENTATION, eFMT_MSG_ORIENTATION_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Light_Sensor_Get
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
eRESULT halLDCtrl_Light_Sensor_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SENSOR, eFMT_MSG_LIGHT_SENSOR_SZ, pcData);	//G100_Doulas_0010 Modify

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Power_Get
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
// A70LV_Eric.C_0008
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Power_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_PO, eFMT_MSG_LD_PO_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Power_Set
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
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
eRESULT halLDCtrl_LD_Power_Set(eLD_ID eDriverID, UINT8* pucLDPower)  // A70LV_Eric.C_0015 pucLDPower
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LD_PO, eFMT_MSG_LD_PO_SZ, pucLDPower);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_PowerAll_Set
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
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
eRESULT halLDCtrl_LD_PowerAll_Set(eLD_ID eDriverID, UINT8* pucLDPower)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LD_PO_ALL, eFMT_MSG_LD_PO_ALL_SZ, pucLDPower);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Light_Module_Status_Set
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
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
eRESULT halLDCtrl_Light_Module_Status_Set(eLD_ID eDriverID, UINT8 ucLightEnable)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_MODULE_STATUS, eFMT_MSG_LIGHT_MODULE_STATUS_SZ, &ucLightEnable);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Duration_of_Lighting_Get
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
UINT32 halLDCtrl_Duration_of_Lighting_Get(void)
{
    return uiLightingSec;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Duration_of_Lighting_Set
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
void halLDCtrl_Duration_of_Lighting_Set(UINT32 uiSec)
{
    uiLightingSec = uiSec;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_CeilingMount_Get
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
// A70LV_Doulas_0062
// --------------------
// ==============================================================================
eRESULT halLDCtrl_CeilingMount_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_CEILINGMOUNT, eFMT_MSG_CEILINGMOUNT_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Bank_Enable_Set
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
eRESULT halLDCtrl_Bank_Enable_Set(eLD_BANK_ENALE_ITEM eBank, BOOL bEnable)
{
    eLD_BANK_ITEM eBankItem = eLD_BANK_R1;
    UINT8 ucBank = 0;
    UINT8 ucEnable = 0;
    UINT8 aucData[2] = {0};
    eRESULT eResult = rcSUCCESS;

    switch((UINT8)eBank)
    {
        case eLD_BANK_ENALE_01:
        case eLD_BANK_ENALE_02:
        case eLD_BANK_ENALE_03:
        case eLD_BANK_ENALE_04:
            {
                eBankItem = eLD_BANK_B1;
                ucBank = (UINT8)(eBank - eLD_BANK_ENALE_01);
                ucEnable = m_aucLDEnable[eBankItem];

                if(bEnable) //On
                {
                    switch(ucBank)
                    {
                        case 0:
                            ucEnable |= LD_ENABLEBLOCK1MASK;
                            break;

                        case 1:
                            ucEnable |= LD_ENABLEBLOCK2MASK;
                            break;

                        case 2:
                            ucEnable |= LD_ENABLEBLOCK3MASK;
                            break;

                        case 3:
                            ucEnable |= LD_ENABLEBLOCK4MASK;
                            break;
                    }
                }
                else
                {
                    switch(ucBank)
                    {
                        case 0:
                            ucEnable &= (~LD_ENABLEBLOCK1MASK);
                            break;

                        case 1:
                            ucEnable &= (~LD_ENABLEBLOCK2MASK);
                            break;

                        case 2:
                            ucEnable &= (~LD_ENABLEBLOCK3MASK);
                            break;

                        case 3:
                            ucEnable &= (~LD_ENABLEBLOCK4MASK);
                            break;
                    }
                }
            }
            break;

		case eLD_BANK_ENALE_05:
        case eLD_BANK_ENALE_06:
        case eLD_BANK_ENALE_07:
        case eLD_BANK_ENALE_08:
            {
                eBankItem = eLD_BANK_B2;
                ucBank = (UINT8)(eBank - eLD_BANK_ENALE_05);	//A65_Clare_0005
                ucEnable = m_aucLDEnable[eBankItem];

                if(bEnable) //On
                {
                    switch(ucBank)
                    {
                        case 0:
                            ucEnable |= LD_ENABLEBLOCK1MASK;
                            break;

                        case 1:
                            ucEnable |= LD_ENABLEBLOCK2MASK;
                            break;

                        case 2:
                            ucEnable |= LD_ENABLEBLOCK3MASK;
                            break;

                        case 3:
                            ucEnable |= LD_ENABLEBLOCK4MASK;
                            break;
                    }
                }
                else
                {
                    switch(ucBank)
                    {
                        case 0:
                            ucEnable &= (~LD_ENABLEBLOCK1MASK);
                            break;

                        case 1:
                            ucEnable &= (~LD_ENABLEBLOCK2MASK);
                            break;

                        case 2:
                            ucEnable &= (~LD_ENABLEBLOCK3MASK);
                            break;

                        case 3:
                            ucEnable &= (~LD_ENABLEBLOCK4MASK);
                            break;
                    }
                }

            }
            break;

        case eLD_BANK_ENALE_09:	//A65_JIG_Clare_0001
        case eLD_BANK_ENALE_10:	//A65_JIG_Clare_0001
        case eLD_BANK_ENALE_11:
        case eLD_BANK_ENALE_12:
        case eLD_BANK_ENALE_13:
        case eLD_BANK_ENALE_14:
            {
                eBankItem = eLD_BANK_R1;
                ucBank = (UINT8)(eBank - eLD_BANK_ENALE_09);	//A65_Clare_0005
                ucEnable = m_aucLDEnable[eBankItem];

                if(bEnable) //On
                {
                    switch(ucBank)
                    {
                        case 0:
                            ucEnable |= LD_ENABLEBLOCK1MASK;
                            break;

                        case 1:
                            ucEnable |= LD_ENABLEBLOCK2MASK;
                            break;

                        case 2:
                            ucEnable |= LD_ENABLEBLOCK3MASK;
                            break;

                        case 3:
                            ucEnable |= LD_ENABLEBLOCK4MASK;
                            break;
                    }
                }
                else//Off
                {
                    switch(ucBank)
                    {
                        case 0:
                            ucEnable &= (~LD_ENABLEBLOCK1MASK);
                            break;

                        case 1:
                            ucEnable &= (~LD_ENABLEBLOCK2MASK);
                            break;

                        case 2:
                            ucEnable &= (~LD_ENABLEBLOCK3MASK);
                            break;

                        case 3:
                            ucEnable &= (~LD_ENABLEBLOCK4MASK);
                            break;
                    }
                }
            }
            break;
    }

    m_aucLDEnable[eBankItem] =  ucEnable;
    aucData[0] = eBankItem;
    aucData[1] = ucEnable;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LD_BANK_ENABLE, eFMT_MSG_LD_BANK_ENABLE_SZ, aucData);

	return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Bank_Enable_Get
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
BOOL halLDCtrl_Bank_Enable_Get(eLD_BANK_ENALE_ITEM eBank)
{
    eLD_BANK_ITEM eBankItem = eLD_BANK_R1;
    UINT8 ucBank = 0;
    UINT8 ucEnable = 0;

    switch((UINT8)eBank)
    {
        case eLD_BANK_ENALE_01:
        case eLD_BANK_ENALE_02:
        case eLD_BANK_ENALE_03:
        case eLD_BANK_ENALE_04:
            {
                eBankItem = eLD_BANK_B1;
                ucBank = (UINT8)(eBank - eLD_BANK_ENALE_01);
                ucEnable = m_aucLDEnable[eBankItem];

                switch(ucBank)
                {
                    case 0:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK1MASK);
                        break;

                    case 1:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK2MASK);
                        break;

                    case 2:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK3MASK);
                        break;

                    case 3:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK4MASK);
                        break;
                }

            }
            break;

		case eLD_BANK_ENALE_05:
        case eLD_BANK_ENALE_06:
        case eLD_BANK_ENALE_07:
        case eLD_BANK_ENALE_08:
            {
                eBankItem = eLD_BANK_B2;
                ucBank = (UINT8)(eBank - eLD_BANK_ENALE_05);
                ucEnable = m_aucLDEnable[eBankItem];

                switch(ucBank)
                {
                    case 0:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK1MASK);
                        break;

                    case 1:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK2MASK);
                        break;

                    case 2:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK3MASK);
                        break;

                    case 3:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK4MASK);
                        break;
                }
            }
            break;

        case eLD_BANK_ENALE_09:	//G100_Clare_0003
        case eLD_BANK_ENALE_10:	//G100_Clare_0003
        case eLD_BANK_ENALE_11:
        case eLD_BANK_ENALE_12:
        case eLD_BANK_ENALE_13:
        case eLD_BANK_ENALE_14:
            {
                eBankItem = eLD_BANK_R1;
                ucBank = (UINT8)(eBank - eLD_BANK_ENALE_09);	//G100_Clare_0003
                ucEnable = m_aucLDEnable[eBankItem];

                switch(ucBank)
                {
                    case 0:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK1MASK);
                        break;

                    case 1:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK2MASK);
                        break;

                    case 2:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK3MASK);
                        break;

                    case 3:
                        ucEnable = !!(ucEnable & LD_ENABLEBLOCK4MASK);
                        break;
                }

            }
            break;
    }

    return (BOOL)ucEnable;

}

eRESULT halLDCtrl_LD_GroupEnable(UINT8 ucGroup, UINT8 ucEnable) //A70LV_Larry_0142
{
    eRESULT eResult = rcSUCCESS;
    UINT8 aucData[2] = {0};

    aucData[0] = ucGroup;
    aucData[1] = ucEnable;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LD_BANK_ENABLE, eFMT_MSG_LD_BANK_ENABLE_SZ, aucData);

	return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Voltage_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Voltage_All_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

//    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_VOLTAGE_ALL, eFMT_MSG_LD_CURRENT_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Current_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Current_All_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

//    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_CURRENT_ALL, eFMT_MSG_LD_CURRENT_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Temperature_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_Temperature_All_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

//    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_TEMPERATURE_ALL, eFMT_MSG_LD_TEMPERATURE_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Tec_Voltage_All_Get
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
// 2018/05/18, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Tec_Voltage_All_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

//    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_TEC_VOLTAGE_ALL, eFMT_MSG_TEC_VOLTAGE_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Tec_Current_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Tec_Current_All_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

//    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_TEC_CURRENT_ALL, eFMT_MSG_TEC_CURRENT_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_Tec_Temperature_All_Get
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
// 2018/04/09, Larry Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_Tec_Temperature_All_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_TEC_TEMPERATURE_ALL, eFMT_MSG_TEC_TEMPERATURE_ALL_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_ABP_ModeSet
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
eRESULT halLDCtrl_ABP_ModeSet(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

	eResult = halFormatter_ABPINFOSet((eABP_LD_Type)eABP_CONTROL_MODE, (UINT8*)&cData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_TargetBLDLightSet
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
eRESULT halLDCtrl_TargetBLDLightSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;
    //Y R B G
    eResult = halFormatter_ABPINFOSet((eABP_LD_Type)eABP_TARGET_LIGHTSENSOR_BLD, pcData);

	return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_TargetBLDPWMSet
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
eRESULT halLDCtrl_TargetBLDPWMSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    //Y R B G
    eResult = halFormatter_ABPINFOSet((eABP_LD_Type)eABP_DEFAULT_PWM_BLD, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_TargetRLDLightSet
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
eRESULT halLDCtrl_TargetRLDLightSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFormatter_ABPINFOSet((eABP_LD_Type)eABP_TARGET_LIGHTSENSOR_RLD, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_TargetRLDPWMSet
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
eRESULT halLDCtrl_TargetRLDPWMSet(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    //Y R B G
    halFormatter_ABPINFOSet((eABP_LD_Type)eABP_DEFAULT_PWM_RLD, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_ABP_AutoTuning_Get
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
eRESULT halLDCtrl_ABP_AutoTuning_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = halFormatter_ABPINFOGet(pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_DisableShutdown
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
eRESULT halLDCtrl_DisableShutdown(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 acData[2] = {0};

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_SHUTDOWN, eFMT_MSG_SHUTDOWN_SZ, acData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_Count_Get
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
eRESULT halLDCtrl_LD_Count_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_CONT_ON_TIME, eFMT_MSG_LD_CONT_ON_TIME_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_G_Sensor_Data_Get
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
// ZU860_Doulas_0059
// --------------------
// ==============================================================================
eRESULT halLDCtrl_G_Sensor_Data_Get(eLD_ID eDriverID, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_G_SENSOR_DATA, eFMT_MSG_G_SENSOR_DATA_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_SourceBlanking_Get
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
// 2018/12/27, Doulas Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_SourceBlanking_Get(eLD_ID eDriverID, UINT8* pcData)        //A70LV_Doulas_0294
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_ENABLE_BY_SOURCE, eFMT_MSG_LD_ENABLE_BY_SOURCE_SZ, pcData);

    return eResult;
}

eRESULT halLDCtrl_LD_SourceBlanking_Set(eLD_ID eDriverID, UINT8 ucData)   //A70LV_Doulas_0294
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LD_ENABLE_BY_SOURCE, eFMT_MSG_LD_ENABLE_BY_SOURCE_SZ, &ucData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LD_WheelBlanking_Get
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
// 2019/03/28, Doulas Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_LD_WheelBlanking_Get(eLD_ID eDriverID, UINT8* pcData)        //A70LV_Doulas_0306
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LD_ENABLE_BY_WHEEL, eFMT_MSG_LD_ENABLE_BY_WHEEL_SZ, pcData);

    return eResult;
}

eRESULT halLDCtrl_LD_WheelBlanking_Set(eLD_ID eDriverID, UINT8 ucData)   //A70LV_Doulas_0306
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LD_ENABLE_BY_WHEEL, eFMT_MSG_LD_ENABLE_BY_WHEEL_SZ, &ucData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_First_LD_On_Set
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
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
eRESULT halLDCtrl_First_LD_On_Set(eLD_ID eDriverID, UINT8 ucData)
{
    eRESULT eResult = rcSUCCESS;

#if 0
#ifdef ENABLE_MCU_CLI
    eResult = utilHost_SystemSet(eCMD_MODULE_LD, eLD_EVENT_LD_ON, (UINT16)sizeof(UINT8), (UINT8*)&ucData);
#else
    eResult = dvLDDriverRegWrite(eDriverID, eFMT_MSG_LD_FIRST_LD_ENABLE, eFMT_MSG_LD_FIRST_LD_ENABLE_SZ, &ucData);
#endif /* ENABLE_MCU_CLI */
#endif /* 0 */

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LS_RLD_Enable_Set
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
eRESULT halLDCtrl_LS_RLD_Enable_Set(UINT8 cData)		//G100_Doulas_0010
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LS_RLD_ENABLE, eFMT_MSG_LS_RLD_ENABLE_SZ, &cData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LightSensorT1_Set
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
eRESULT halLDCtrl_LightSensorT1_Set(UINT8* pcData)		//G100_Doulas_0012 Set T1
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LS_T1, eFMT_MSG_LS_T1_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LightSensorT0_Set
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
eRESULT halLDCtrl_LightSensorT0_Set(UINT8* pcData)		//G100_Doulas_0012 Set T0
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LS_T0, eFMT_MSG_LS_T0_SZ, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_LightSensorTrigger_Set
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
eRESULT halLDCtrl_LightSensorTrigger_Set(UINT8 cData)		//G100_Doulas_0012 trigger LS quickly receive values (10 times)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SENSOR_TRIGGER, eFMT_MSG_LIGHT_SENSOR_TRIGGER_SZ, &cData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_T1LightSensorValue_Get
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
// 2020/08/31, Doulas Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_T1LightSensorValue_Get(eLD_ID eDriverID, UINT8* pcData)		//G100_Doulas_0012 get light sensor value(for T1)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SENSOR_T1, eFMT_MSG_LIGHT_SENSOR_T1_SZ, pcData);

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halLDCtrl_T0LightSensorValue_Get
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
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
eRESULT halLDCtrl_T0LightSensorValue_Get(eLD_ID eDriverID, UINT8* pcData)		//G100_Doulas_0012 get light sensor value(for T0)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemGet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SENSOR_T0, eFMT_MSG_LIGHT_SENSOR_T0_SZ, pcData);

    return eResult;
}

//G100_Clare_0011, add, >>>
// ==============================================================================
// FUNCTION NAME: halLDCtrl_Fan_Mode_Set
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
eRESULT halLDCtrl_Fan_Mode_Set(UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_FAN_DISPLAY_MODE, eFMT_MSG_FAN_DISPLAY_SZ, pcData);

    return eResult;
}
//G100_Clare_0011, add, <<<

eRESULT halLDCtrl_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8 *pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dvLDDriverRegWrite(eCMD_MODULE_OPD, reg, size, pcData);

    return eResult;
}

eRESULT halLDCtrl_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dvLDDriverRegRead(eCMD_MODULE_OPD, reg, size, pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_ProjectorID_Set
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
eRESULT halLDCtrl_ProjectorID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dvLDDriverRegWrite(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PROJECTOR_ID, 1, &cData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_CustomerID_Set
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
eRESULT halLDCtrl_CustomerID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dvLDDriverRegWrite(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_CUSTOMER_ID, 1, &cData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halLDCtrl_PlatformID_Set
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
eRESULT halLDCtrl_PlatformID_Set(UINT8 cData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dvLDDriverRegWrite(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_PLATFORM_ID, 1, &cData);

    return eResult;
}

eRESULT halLDCtrl_LightSensorPosition_Set(UINT8 pcData)  //HICC2_Jacky_0001
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_SystemSet(eCMD_MODULE_FMT, eFMT_MSG_LIGHT_SENSOR_POSITION, eFMT_MSG_LIGHT_SENSOR_POSITION_SZ, &pcData);

    return eResult;
}

