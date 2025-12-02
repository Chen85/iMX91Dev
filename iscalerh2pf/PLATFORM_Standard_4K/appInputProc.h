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
#define INPUT_RGB_LEVEL_POLLING_PERIOD (1000)	//A70LK_Doulas_0012

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

#ifdef OE_JIG                           //HICC2_Tim_0004, add, ***
typedef enum
{
    eOE_JIG_CMD_TYPE_NOP,               //normal remote key
    eOE_JIG_CMD_TYPE_PF,                //OE JIG function in iScalerH2PF
    eOE_JIG_CMD_TYPE_CT,                //OE JIG function in iScalerH2CT

    eOE_JIG_CMD_TYPE_NUMBERS,
} eOE_JIG_CMD_TYPE;
#endif //OE_JIG                         //HICC2_Tim_0004, add, &&&

eEXEC_CODE palInputProc_KeyEventSend(sKEY_DATA Key);
eEXEC_CODE palInputProc_Init(void);
eEXEC_CODE palInputProc_PowerNormal(void);      //G100_Owen_0007
INT16 palInputProc_Poll(UINT16 uiTick);
eRESULT palInputProc_BufferInsert(void *pvKey);
eRESULT palInputProc_InputKeycode(UINT32 ucKeyCode); //A70LV_Larry_0135
eRESULT palInputProc_InputKeyListID(eKEY_LIST eKeyListID);
eEXEC_CODE palInputProc_CLIHandler(UINT8 cKey);
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
void palInputProc_USTTestPattern(UINT8 ucTP);
BOOL palInputProc_ADV_WarpBackCheck(void);					//G100_Doulas_0027
BOOL palInputProc_ADV_WarpEnterCheck(void)	;				//G100_Doulas_0027
BOOL palInputProc_ADV_WarpDirectionCheck(UINT8 ucVal);	//G100_Doulas_0027

#ifdef OSD_LOCK_ENABLE //A65_OPTOMA_Jerry_0005 start
void palInputProc_OSD_Unlock_Step_Set(UINT8 ucOSD_Unlock_Step);
UINT8 palInputProc_OSD_Unlock_Step_Get(void);
eCMD_KEY_DATACODE_EXECUTE_STATUS palInputProc_OSD_Lock_Key_Filter(eKEY_INPUT_TYPE eKeyType, UINT32 dwKey);
eCMD_KEY_DATACODE_EXECUTE_STATUS palInputProc_OSD_Lock_Check_Key(eKEY_INPUT_TYPE eKeyType, UINT32 dwKey);
#endif //OSD_LOCK_ENABLE  //A65_OPTOMA_Jerry_0005 end
#if defined(KEYPAD_LOCK_ENABLE)// HICC2_Bruce_0005
BOOL palInputProc_Keypad_Lock_Input_Check(sKEY_DATA *psInputKey);
BOOL palInputProc_Keypad_UnLock_Input_Check(sKEY_DATA *psInputKey);
#endif
#if defined(QUICK_KEY_ENABLE)
BOOL palInputProc_Quick_Key_Lock_Input_Check(sKEY_DATA *psInputKey);
#endif
void palInputProc_OPDKeyEvent(sKEY_DATA sKeyData);  //A35G2_Simon_0075
INT16 palInputProc_RGB_LevelPoll(UINT16 uiTick);	//A70LK_Doulas_0012
BOOL palInputProc_BIST_KeyCheckFun(UINT8 eKey);     //HICC2_Steven_0022
BOOL palInputProc_DDP_PatternCheck(void); //HICC2_Casper_0019
INT8 palInputProc_ACU_Target_Select_KeyCheck(UINT32 eKey);  //H2PF_Simon_0035

UINT32 palInput_Get_H_Current_Position(void); //R70PD_AC_0105 //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
void palInput_Set_H_Current_Position(UINT32 wPosition); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
UINT32 palInput_Get_V_Current_Position(void); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
void palInput_Set_V_Current_Position(UINT32 wPosition); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030 //HICC2_Casper_0040
UINT8 palInput_Get_H_Current_Direction(void);; //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
void palInput_Set_H_Current_Direction(UINT8 bDirection); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
UINT8 palInput_Get_V_Current_Direction(void); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
void palInput_Set_V_Current_Direction(UINT8 bDirection); //R70KPD_VSC_Wesley_0003 //HICC2_Casper_0030
eEXEC_CODE palInputProc_ServiceCodeCheck(sKEY_DATA *psInputKey);//HICC2_Steven_0043 //HICC2_Julie_0020
BOOL palInput_Web_LensShift_Check(UINT32 wData);
BOOL palInputProc_AdvWarpPatternOn_KeyCheck(UINT32 eKey);
void palInput_FOTA_StatusSet(UINT8 ucStatus);
UINT8 palInput_FOTA_StatusGet(void);
void palInput_Lens_Type_Set(UINT8 ucData);
UINT8 palInput_Lens_Type_Get(void);
void palInput_Fuji_Lens_Detect_Set(UINT8 ucData);
UINT8 palInput_Fuji_Lens_Detect_Get(void);

#endif // PALINPUTCTRLAPI_H

