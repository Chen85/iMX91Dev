#ifndef PALINPUTPROC_H
#define PALINPUTPROC_H
// ==============================================================================
// FILE NAME: APPINPUTCTRLAPI.H
// DESCRIPTION:
//
//
// modification history
// --------------------
// 22/04/2013, Leohong written
// --------------------
// ==============================================================================


#include "Common.h"
#include "halKeyCodeList.h"   //T100_Simon_0022
#include "halInputCtrlAPI.h" //A65_OPTOMA_Jerry_0005
#include "utilCommon.h" //A65_OPTOMA_Jerry_0005

//#include "utilDataMgrAPI.h"
//#include "halBoardCtrlAPI.h" //A70_Larry_0043

#define INPUT_POLLING_PERIOD (10)

//A65_OPTOMA_Jerry_0005 start
#ifdef OSD_LOCK_ENABLE
#define OSD_UNLOCK_PROCEDURE_STEP1 1
#define OSD_UNLOCK_PROCEDURE_STEP2 2
#define OSD_UNLOCK_PROCEDURE_STEP3 3
#define OSD_UNLOCK_PROCEDURE_STEP4 4
#define OSD_UNLOCK_PROCEDURE_DONE  5
#endif //OSD_LOCK_ENABLE
//A65_OPTOMA_Jerry_0005 end

typedef enum
{
    eINPUT_EVENT_KEY,
    eINPUT_EVENT_POWER_ON,
    eINPUT_EVENT_POWER_OFF,

    eINPUT_EVENT_NUMBERS,
} eINPUT_EVENT;

#ifdef PLATFORM_B35LH //B35LH_Casper_0008
typedef enum
{
    eINPUT_PARAM_REMOTE_CODE,
    eINPUT_PARAM_DEFAULT_REMOTE_CODE,
    eINPUT_PARAM_IR_POWER_KEY_CODE,
    eINPUT_PARAM_UARTSET,
    eINPUT_PARAM_BAUDRATE,
    eINPUT_PARAM_ERROR_LOG_ADDRESS,

    eINPUT_PARAM_NUMBERS,
} eINPUT_PARAM;
#else
typedef enum
{
    eINPUT_PARAM_REMOTE_ID,
    eINPUT_PARAM_UARTSET,
    eINPUT_PARAM_BAUDRATE,

    eINPUT_PARAM_NUMBERS,
} eINPUT_PARAM; //A70_Larry_0043
#endif
#if 0
typedef enum
{
    eINPUT_BAUDRATE_1200,
    eINPUT_BAUDRATE_2400,
    eINPUT_BAUDRATE_4800,
    eINPUT_BAUDRATE_9600,
    eINPUT_BAUDRATE_14400,
    eINPUT_BAUDRATE_19200,
    eINPUT_BAUDRATE_38400,
    eINPUT_BAUDRATE_57600,
    eINPUT_BAUDRATE_115200,

    eINPUT_BAUDRATE_NUMBERS,
} eINPUT_BAUDRATE;
#endif /* 0 */

eEXEC_CODE palInputProc_Init(void);
eEXEC_CODE palInputProc_PowerNormal(void);      //G100_Owen_0007
INT16 palInputProc_Poll(UINT16 uiTick);
eRESULT palInputProc_BufferInsert(void *pvKey);
eRESULT palInputProc_InputKeycode(UINT32 ucKeyCode); //A70LV_Larry_0135
eRESULT palInputProc_InputKeyListID(eKEY_LIST eKeyListID);
eEXEC_CODE palInputProc_CLIHandler(UINT8 cKey);
//void palInputProc_NodeCallback(eNVRAM_NODE eNodeID, eNVRAM_NODE_EVENT eNodeEvent);
//void palInputProc_Param_Get(eINPUT_PARAM eParam, void *pvData);
//void palInputProc_Param_Set(eINPUT_PARAM eParam, void *pvData);
//void palInputProc_UartSwitchSet(eUART_SWITCH eMode);
//void palInputProc_UartSwitchReCfg(void);
#ifdef PLATFORM_B35LH //B35LH_Casper_0008
void palInputProc_Remote_CustomCode_Set(UINT16 wCustomCode);
void palInputProc_Default_Remote_CustomCode_Set(UINT16 wCustomCode);
void palInputProc_IR_PowerKeyCode_Set(UINT16 wPowerKeyCode);
#else
void palInputProc_Remote_CustomId_Set(UINT16 wRemoteId); //A70LV_Larry_0001
#endif
void palInputProc_LensMotorLockSet(UINT8 ucEnable); //A70LV_Larry_0129
UINT8 palInputProc_LensMotorLockGet(void); //A70LV_Larry_0129
void palInputProc_DisableKey(UINT8 ucEnable); //A70LV_Larry_0191
//void palInputProc_TestPattern(UINT8 ucTP); //A70LV_Larry_0196
void palInputProc_USTTestPattern(UINT8 ucTP);//ZU860_Energy_0009 //A35G2_BRC_Casper_0088
BOOL palInputProc_ADV_WarpBackCheck(void);					//G100_Doulas_0027
BOOL palInputProc_ADV_WarpEnterCheck(void)	;				//G100_Doulas_0027
BOOL palInputProc_ADV_WarpDirectionCheck(UINT8 ucVal);	//G100_Doulas_0027

#ifdef OSD_LOCK_ENABLE //A65_OPTOMA_Jerry_0005 start
void palInputProc_OSD_Unlock_Step_Set(UINT8 ucOSD_Unlock_Step);
UINT8 palInputProc_OSD_Unlock_Step_Get(void);
eCMD_KEY_DATACODE_EXECUTE_STATUS palInputProc_OSD_Lock_Key_Filter(eKEY_INPUT_TYPE eKeyType, UINT32 dwKey);
eCMD_KEY_DATACODE_EXECUTE_STATUS palInputProc_OSD_Lock_Check_Key(eKEY_INPUT_TYPE eKeyType, UINT32 dwKey);
#endif //OSD_LOCK_ENABLE  //A65_OPTOMA_Jerry_0005 end

void palInputProc_OPDKeyEvent(sKEY_DATA sKeyData);  //A35G2_Simon_0075
eEXEC_CODE palInputProc_KeyEventSend(sKEY_DATA Key); //HICC2_Steven_0010
INT8 palInputProc_ACU_Target_Select_KeyCheck(UINT32 eKey);
BOOL palInputProc_BIST_KeyCheckFun(UINT8 eKey); //HICC2_Steven_0002
BOOL palInputProc_DDP_PatternCheck(void); //HICC2_Casper_0019

UINT32 palInput_Get_H_Current_Position(void); //R70PD_AC_0105 //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
void palInput_Set_H_Current_Position(UINT32 wPosition); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
UINT32 palInput_Get_V_Current_Position(void); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
void palInput_Set_V_Current_Position(UINT32 wPosition); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
UINT8 palInput_Get_H_Current_Direction(void);; //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
void palInput_Set_H_Current_Direction(UINT8 bDirection); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
UINT8 palInput_Get_V_Current_Direction(void); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
void palInput_Set_V_Current_Direction(UINT8 bDirection); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
BOOL palInput_Web_LensShift_Check(UINT32 wData);

#endif // PALINPUTCTRLAPI_H

