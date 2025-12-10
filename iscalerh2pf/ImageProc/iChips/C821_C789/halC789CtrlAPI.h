// ===============================================================================
// FILE NAME: halC789CtrlAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/09/13, Larry Create
// --------------------
// ===============================================================================


#ifndef HALC789CTRL_H
#define HALC789CTRL_H

#include "Common.h"

typedef enum
{
  eC789_TIMING_CONFIG_1080_P60 = 0,
  eC789_TIMING_CONFIG_1080_P50,
  eC789_TIMING_CONFIG_WUXGA_P60,
  eC789_TIMING_CONFIG_WUXGA_P50,

  eC789_TIMING_CONFIG_NUMBERS,

}eC789_TIMING_CONFIG;

BOOL halC789Ctrl_SemaphoreTake(const char *pcFunc);
BOOL halC789Ctrl_SemaphoreGive(const char *pcFunc);
void halC789Ctrl_BankSync(void); //A70LV_Larry_0318
#ifdef Low_Latency_All
void halC789Ctrl_Init(const ePANEL_ID ePanelId, BOOL bLow_Latency);
#else
void halC789Ctrl_Init(const ePANEL_ID ePanelId);
#endif	/*Low_Latency_All*/
void halC789Ctrl_BypassMode(void);
void halC789Ctrl_TestPattern(void);     //A70LV_Doulas_0036
void halC789Ctrl_Init_Default_Register(void);
void halC789Ctrl_V_Start_Checking(void);  //A70LV_Doulas_0042
void halC789Ctrl_OutputEnableSet(UINT8 ucEnable);  //A70LV_Doulas_0079
UINT8 halC789Ctrl_OutputEnableGet(void);
void halC789Ctrl_ForcedSyncResetValueSet(UINT8 ucForcedSyncReset,UINT16 uiH_Total,UINT16 uiV_Total);
void halC789Ctrl_Change_Panel(const ePANEL_ID ePanelId);     //A70LV_Doulas_0098
void halC789Ctrl_FrmaeDelaySet(UINT8 ucVlaue);      //A70LV_Doulas_0154
UINT16 halC789Ctrl_Output_V_Total_Get(void);     //A70LV_Doulas_0154

void halC789Ctrl_TwistLinkFlag_Set(BOOL bbTwistLink);   //A70LV_Doulas_0262
BOOL halC789Ctrl_TwistLinkFlag_Get(void);               //A70LV_Doulas_0262

UINT8 halC789Ctrl_OutputChanged_Get(void);   //A70LV_Doulas_0307
void halC789Ctrl_OutputChanged_Set(UINT8 ucVal);   //A70LV_Doulas_0307
void halC789Ctrl_COMS_Output_Set(UINT8 ucVal);   //A70LV_Doulas_0361
UINT8 halC789Ctrl_OutputV_FreqGet(void);   //A70LV_Doulas_0367
UINT32 halC789Ctrl_Reg0_Read_Get(void); //G100_Julie_0004
UINT8 halC789Ctrl_OPD_Test(UINT8 ucData); //G100_Julie_0004
void halC789Ctrl_Cmd_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt);
#ifdef Low_Latency_All
void halC789Ctrl_Low_Latency_Set(UINT8 ucEnable);
#endif	/*Low_Latency_All*/
void halC789Ctrl_RegDump(void); 	//G100_Doulas_0027
UINT8 halC789Ctrl_Black_Screen_Get(void);			//G100_Doulas_0072
UINT8 halC789Ctrl_Black_Screen_Set(UINT8 ucData);	//G100_Doulas_0072
void halC789Ctrl_V_Start_Test(void); //G100_Owen_0078
UINT8 halC789Ctrl_ScreenSaveToImage(UINT32 xPos, UINT32 yPos, UINT32 nWidth, UINT32 nHeight);
void halC789Ctrl_Freeze(UINT8 ucEnable);  //A35G2_CDS_Simon_0003

UINT32 halC789Ctrl_ICP_Register_Read(UINT32 ulAddr);               //A65_OPTOMA_Julie_0052
void halC789Ctrl_ICP_Register_Write(UINT32 ulAddr, UINT32 ucData); //A65_OPTOMA_Julie_0052


#endif /* HALC789CTRL_H */

