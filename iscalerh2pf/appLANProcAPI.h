// ===============================================================================
// FILE NAME: appLANProcAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2018/05/15, Larry Create
// --------------------
// ===============================================================================


#ifndef APPLANPROCAPI_H
#define APPLANPROCAPI_H

#include "Common.h"
#include "utilCommon.h"

#define LAN_POLLING_PERIOD 10 //A70LV_Larry_0257 300 -> 10

typedef enum    //T100_Simon_0039
{
    eLAN_SNMP_OVER_TEMPERATURE,
    eLAN_SNMP_LIGHT_SOURCE_OVER_LIFE,
    eLAN_SNMP_FAN_LOCK_01,
    eLAN_SNMP_FAN_LOCK_02,
    eLAN_SNMP_FAN_LOCK_03,
    eLAN_SNMP_FAN_LOCK_04,
    eLAN_SNMP_FAN_LOCK_05,
    eLAN_SNMP_FAN_LOCK_06,
    eLAN_SNMP_FAN_LOCK_07,
    eLAN_SNMP_FAN_LOCK_08,
    eLAN_SNMP_FAN_LOCK_09,
    eLAN_SNMP_FAN_LOCK_10,
    eLAN_SNMP_FAN_LOCK_11,
    eLAN_SNMP_FAN_LOCK_12,
    eLAN_SNMP_FAN_LOCK_13,
    eLAN_SNMP_FAN_LOCK_14,
    eLAN_SNMP_FAN_LOCK_15,
    eLAN_SNMP_FAN_LOCK_16,

    eLAN_SNMP_MAX_NUMBER,
}eLAN_SNMP;

typedef enum
{
    eACK_DONE_SEND_STATE_PENDQ,
    eACK_DONE_SEND_STATE_PROCESS,

    eACK_DONE_SEND_STATE_NUMBER,
}eACK_DONE_SEND_STATE;  //G100_Owen_0088	//A35G2_CDS_Coda_0034

//#define LAN_FORMAT_DATA_LEN    4 //ucType + ucEnable + uiDataCode

eEXEC_CODE palLANProc_Init(void);
eEXEC_CODE palLANProc_Init_Reset_All(void); //HICC2_Doulas_0062
INT16 palLANProc_Poll(UINT16 uiTick);
void palLANProcSendToLAN(UINT16 uiDataCode);
void palLANProcSendAckDone(UINT16 uiDataCode, _eInterface eSendFrom ,eEXEC_CODE ExecResult); //A70LV_Larry_0238   //H2PF_Simon_0067
void palLANProcPowerStateSend(UINT16 uiData, UINT16 uiSize, UINT8 *pucData);
void palLANProcInfoItemSend(UINT16 uiIndex, UINT16 uiSize, UINT8 *pucData);
void palLANProcStartSet(UINT8 ucData);
UINT8 palLANProcStartGet(void);
void palLANProcTelnetReply(UINT8 *aucString, UINT16 uiStringLength); //A70LV_Larry_0190
void palLANProcTelnetReplyStringGet(UINT8 *String);
void palLANProc_UpgradePercentage(UINT8 ucData); //A70LV_Larry_0189
eEXEC_CODE palLANProc_Reset(void);
void palLANProcSendLog(UINT32 ulEvent, UINT32 ulTime);
static void palLANProcGrayoutConditionCheck(void);
void palLANProcSturctDataSend(UINT16 uiSturctType, UINT16 uiSize, UINT8 *pucData);//G100_Coda_0016
void palLANProcScheduleSend(void);//G100_Coda_0025
void palLANProcScheduleSend_Update(void);

void palLANProcForceCheckUpdateData(void);
void palLANProcForceCheckUpdateDataSub(void);   //HICC2_Doulas_0113
void palLANProcUpdateOSD_Set(BOOL bFlag);
void palLANProcPowerNormal(void); //A35G2_CDS_Larry_0028
void palLANProcFocusPowerStatusSend(void);

#endif /* APPLANPROCAPI_H */

