#ifndef _PALGUI_H_
#define _PALGUI_H_

#ifdef __cplusplus
extern "C" {
#endif
//#include "dvC821_OSD.h"
#include "Common.h"


////////  callback function start  ////////
typedef void (*fpGuiStyle_LanguageSet)(UINT8 Language);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_VGA_Position_Warning)(void);
typedef BOOL (*fpGui_IsInstallatioon_Menu_L1)(void);
typedef eEXEC_CODE (*fpGui_PIN_Protect_Enable)(BOOL Condition);
typedef void (*fpGuiStyle_MenuOffsetSet)(UINT8 Offset);
typedef void (*fpGuiStyle_CurrentSplashSet)(UINT8 Style);
typedef void (*fpGui_Send_WaitProcessMessageExit)(UINT8 Back);
typedef void (*fpGui_LenMovingSet)(void);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_Lens_Moving_Message)(void);
typedef void (*fpGui_SendLensShiftExitEvent)(void);
typedef void (*fpGui_LenCalibrationSet)(void);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_Lens_Calibration_Message)(void);
typedef void (*fpGui_Len_Run_Border_Set)(void);
typedef void (*fpGuiStyle_HorzOffsetSet)(UINT8 Offset);
typedef void (*fpGuiStyle_VertOffsetSet)(UINT8 Offset);
typedef void (*fpGui_MenuTimeout_Set)(UINT8 OSDTimeoutIndex);
typedef void (*fpGui_MenuTimeout_Refresh)(void);
typedef BOOL (*fpGui_IsPower_Consumption_Message)(void);
typedef BOOL (*fpGui_IsStandbyModeSetting_Menu)(void);
typedef eEXEC_CODE (*fpGui_Send_PowerIncreaseMessage)(UINT8 Back);
typedef BOOL (*fpGui_IsNetWork_Wait_Message)(void);
typedef void (*fpGui_Send_NetworkWaitMessageExit)(UINT8 Back);
typedef void (*fpGui_HSG_TestPattern_ON_Check)(void);
typedef void (*fpGui_HSG_TestPattern_Off)(UINT32 MenuIndex);

typedef eEXEC_CODE (*fpGui_Send_MenuOpen_Wait_Message)(void);
typedef BOOL (*fpGui_IsCustom_White_NextMenu_L3)(void);
typedef void (*fpGui_IsHSG_TestPattern_OFF_Custom_White_NextMenu_L3)(void);
typedef void (*fpGui_IsHSG_TestPattern_ON_Custom_White_NextMenu_L3)(void);
typedef void (*fpGui_TestPattern)(UINT8 TP);
typedef UINT8 (*fpGui_HSG_TestPatternEnableGet)(void);
typedef UINT8 (*fpGui_HSG_TestPatternNumberGet)(void);
typedef void (*fpGui_Emergency_Show)(UINT8 Enable);
typedef void (*fpGui_SupportMessageSet)(UINT8 Index);

typedef void (*fpGui_SendDisconnectTwistMessage)(UINT8 Show);
typedef void (*fpGui_SendUpdateOSDEvent)(void);
typedef eEXEC_CODE (*fpGui_HSG_TestPattern_Set)(UINT8 TP);
typedef UINT8 (*fpGui_PIN_Protect_Checking)(void);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_WarningMenuPowerDown_Message)(void);
typedef BOOL (*fpGui_IsWait_Message_Menu)(void);
typedef void (*fpGui_Send_OSD_Exit)(void);
typedef void (*fpGui_SendDrawPIPLayoutEvent)(INT8 Value);
typedef BOOL (*fpGui_Get_MenuState_IsOSD_Open)(void);
typedef eEXEC_CODE (*fpGui_Auto_Focus_Message)(void);
typedef eEXEC_CODE (*fpGui_Auto_Color_Uniformity_Message)(void);
typedef UINT8 (*fpGui_TestPatternGet)(void);
typedef void (*fpGui_OSD_Send_Open_Freeze_Menu_Message)(void);
typedef void (*fpGui_DrawPIPLayout_Position)(UINT8 Layout, UINT8 Size);

typedef void (*fpGui_OSD_ON_Set)(UINT8 Val);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_Auto_Wall_Color_Message)(void);
typedef void (*fpGui_IsPIPOn_Set)(UINT8 Enable);
typedef void (*fpGui_Send_MenuState_SetIsSourceMenuOpen)(INT8 Value);
typedef eEXEC_CODE (*fpGui_PanelChange)(ePANEL_ID ePanelTimingId);
typedef INT8 (*fpGui_BurnIn_TestPatteren_Update)(void);
typedef BOOL (*fpGui_IsCustom_White_Reset_ConfirmDialog)(void);
typedef void (*fpGui_IsHSG_TestPattern_ON_Custom_White_Reset_ConfirmDialog)(void);
typedef void (*fpGuiStyle_MenuOffsetBlending)(UINT8 Enable);
typedef void (*fpGuiStyle_MenuOffsetTopBlending)(INT16 High);
typedef void (*fpGuiStyle_MenuOffsetBottomBlending)(INT16 High);
typedef void (*fpGuiStyle_MenuOffsetLeftBlending)(INT16 High);
typedef void (*fpGui_SendRestoreFailMessage)(void);
typedef eEXEC_CODE (*fpGui_Auto_Color_Uniformity_Target_Select)(void);
typedef void (*fpGui_SendSplashEvent)(INT8 Value);
typedef ePANEL_ID (*fpGui_PanelGet)(void);
typedef INT8 (*fpGui_IsSplash_On_Get)(void);
typedef void (*fpGui_SendSubSourceInfoMessage)(void);
typedef void (*fpGuiStyle_CurrentTestPatternSet)(UINT8 Style);
typedef INT8 (*fpGui_OSD_MenuTransparencyEnableSet)(UINT8 Enable);

typedef UINT8 (*fpGui_OSD_MenuTransparencyEnableGet)(void);
typedef BOOL (*fpGui_IsFirstStartupMenu)(void);
typedef BOOL (*fpGui_IsPower_Off_Menu)(void);
typedef eEXEC_CODE (*fpGui_BackupInput_Inactive_Message)(void);
typedef INT8 (*fpGui_HWInit)(void);
typedef BOOL (*fpGui_IsAutoPowerDown_Menu)(void);
typedef void (*fpGui_SendAutoPowerOffMenuExit)(void);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_UST_Message2)(void);
typedef UINT8 (*fpGuiStyle_CurrentTestPatternGet)(void);
typedef eEXEC_CODE (*fpGui_SecurityShutDown_Query_Menu)(void);
typedef void (*fpGui_Send_SleepTimer_Query_Menu)(void);
typedef eEXEC_CODE (*fpGui_AutoShutDown_Query_Menu)(void);
typedef void (*fpGui_Send_ABC_Result)(void);
typedef void (*fpGui_SendOSDPollEvent)(void);
typedef UINT8 (*fpGui_MsgQueueRemainSizeIsLow)(void);
typedef eEXEC_CODE (*fpGui_DataCode_Value_Set)(UINT16 uiDataCode, UINT8 ucAccessMode, tDATA_CODE iValue);
typedef void (*fpGui_SendKeyEvent)(UINT8 ucType, eKEY_LIST eKey);
typedef eEXEC_CODE (*fpGui_SpecialMenu_Check)(UINT32 KeyCode);
typedef BOOL (*fpGui_IsService_Code_Input_Menu)(void);
typedef BOOL (*fpGui_IsLens_Shift_Menu)(void);
typedef BOOL (*fpGui_IsFocus_Menu)(void);
typedef BOOL (*fpGui_IsZoom_Menu)(void);
typedef void (*fpGui_TestPatternFocusIndexEvent)(UINT8 Type, eKEY_LIST Key);
typedef void (*fpGui_HSG_TestPatternEnableSet)(UINT8 TP);
typedef void (*fpGui_SendSourceMessage)(INT8 Value);
typedef BOOL (*fpGui_Is_NotSuported_Menu)(void);

typedef void (*fpGui_SendUpdateOSD_CLI_Event)(void);
typedef eEXEC_CODE (*fpGui_PowerStandby)(void);
typedef void (*fpGui_Task_Suspend)(void);
typedef void (*fpGui_Task_Resume)(void);
typedef eEXEC_CODE (*fpGui_Init)(ePANEL_ID PanelTimingId);
typedef eEXEC_CODE (*fpGui_PowerNormal)(ePANEL_ID PanelTimingId);
typedef eEXEC_CODE (*fpGui_DataCode_Value_Get)(UINT16 DataCode, tDATA_CODE *Value);
typedef eEXEC_CODE (*fpGui_DataCode_PointerVar_Set)(UINT16 DataCode, UINT8 *Data);
typedef eEXEC_CODE (*fpGui_DataCode_String_Set)(UINT16 DataCode, char *DynamicString);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_Schedule_NextMenu_L2)(void);
typedef BOOL (*fpGui_LenCalibrationDoing)(void);
typedef eEXEC_CODE (*fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialog)(void);
typedef void (*fpGui_USTInstall_Start)(void);
typedef void (*fpGui_Send_LensMovingMenuOpen)(void);
typedef void (*fpGui_Send_LensCalibrationMenuOpen)(void);
typedef void (*fpGui_SendPinProtectClearEvent)(void);

typedef struct
{
    fpGuiStyle_LanguageSet fpGuiStyle_LanguageSetCb;
    fpGui_Send_MenuOpen_VGA_Position_Warning fpGui_Send_MenuOpen_VGA_Position_WarningCb;
    fpGui_IsInstallatioon_Menu_L1 fpGui_IsInstallatioon_Menu_L1Cb;
    fpGui_PIN_Protect_Enable fpGui_PIN_Protect_EnableCb;
    fpGuiStyle_MenuOffsetSet fpGuiStyle_MenuOffsetSetCb;
    fpGuiStyle_CurrentSplashSet fpGuiStyle_CurrentSplashSetCb;
    fpGui_Send_WaitProcessMessageExit fpGui_Send_WaitProcessMessageExitCb;
    fpGui_LenMovingSet fpGui_LenMovingSetCb;
    fpGui_Send_MenuOpen_Lens_Moving_Message fpGui_Send_MenuOpen_Lens_Moving_MessageCb;
    fpGui_SendLensShiftExitEvent fpGui_SendLensShiftExitEventCb;
    fpGui_LenCalibrationSet fpGui_LenCalibrationSetCb;
    fpGui_Send_MenuOpen_Lens_Calibration_Message fpGui_Send_MenuOpen_Lens_Calibration_MessageCb;
    fpGui_Len_Run_Border_Set fpGui_Len_Run_Border_SetCb;
    fpGuiStyle_HorzOffsetSet fpGuiStyle_HorzOffsetSetCb;
    fpGuiStyle_VertOffsetSet fpGuiStyle_VertOffsetSetCb;
    fpGui_MenuTimeout_Set fpGui_MenuTimeout_SetCb;
    fpGui_MenuTimeout_Refresh fpGui_MenuTimeout_RefreshCb;
    fpGui_IsPower_Consumption_Message fpGui_IsPower_Consumption_MessageCb;
    fpGui_IsStandbyModeSetting_Menu fpGui_IsStandbyModeSetting_MenuCb;
    fpGui_Send_PowerIncreaseMessage fpGui_Send_PowerIncreaseMessageCb;
    fpGui_IsNetWork_Wait_Message fpGui_IsNetWork_Wait_MessageCb;
    fpGui_Send_NetworkWaitMessageExit fpGui_Send_NetworkWaitMessageExitCb;
    fpGui_HSG_TestPattern_ON_Check fpGui_HSG_TestPattern_ON_CheckCb;
    fpGui_HSG_TestPattern_Off fpGui_HSG_TestPattern_OffCb;

    fpGui_Send_MenuOpen_Wait_Message fpGui_Send_MenuOpen_Wait_MessageCb;
    fpGui_IsCustom_White_NextMenu_L3 fpGui_IsCustom_White_NextMenu_L3Cb;
    fpGui_IsHSG_TestPattern_OFF_Custom_White_NextMenu_L3 fpGui_IsHSG_TestPattern_OFF_Custom_White_NextMenu_L3Cb;
    fpGui_IsHSG_TestPattern_ON_Custom_White_NextMenu_L3 fpGui_IsHSG_TestPattern_ON_Custom_White_NextMenu_L3Cb;
    fpGui_TestPattern fpGui_TestPatternCb;
    fpGui_HSG_TestPatternEnableGet fpGui_HSG_TestPatternEnableGetCb;
    fpGui_HSG_TestPatternNumberGet fpGui_HSG_TestPatternNumberGetCb;
    fpGui_Emergency_Show fpGui_Emergency_ShowCb;
    fpGui_SupportMessageSet fpGui_SupportMessageSetCb;


    fpGui_SendDisconnectTwistMessage fpGui_SendDisconnectTwistMessageCb;
    fpGui_SendUpdateOSDEvent fpGui_SendUpdateOSDEventCb;
    fpGui_HSG_TestPattern_Set fpGui_HSG_TestPattern_SetCb;
    fpGui_PIN_Protect_Checking fpGui_PIN_Protect_CheckingCb;
    //fpGui_Send_MenuOpen_WarningMenuPowerDown_Message fpGui_Send_MenuOpen_WarningMenuPowerDown_MessageCb;
    fpGui_IsWait_Message_Menu fpGui_IsWait_Message_MenuCb;
    fpGui_Send_OSD_Exit fpGui_Send_OSD_ExitCb;
    fpGui_SendDrawPIPLayoutEvent fpGui_SendDrawPIPLayoutEventCb;
    fpGui_Get_MenuState_IsOSD_Open fpGui_Get_MenuState_IsOSD_OpenCb;
    fpGui_Auto_Focus_Message fpGui_Auto_Focus_MessageCb;
    fpGui_Auto_Color_Uniformity_Message fpGui_Auto_Color_Uniformity_MessageCb;
    fpGui_TestPatternGet fpGui_TestPatternGetCb;
    fpGui_OSD_Send_Open_Freeze_Menu_Message fpGui_OSD_Send_Open_Freeze_Menu_MessageCb;
    fpGui_DrawPIPLayout_Position fpGui_DrawPIPLayout_PositionCb;

    fpGui_OSD_ON_Set fpGui_OSD_ON_SetCb;
    fpGui_Send_MenuOpen_Auto_Wall_Color_Message fpGui_Send_MenuOpen_Auto_Wall_Color_MessageCb;
    fpGui_IsPIPOn_Set fpGui_IsPIPOn_SetCb;
    fpGui_Send_MenuState_SetIsSourceMenuOpen fpGui_Send_MenuState_SetIsSourceMenuOpenCb;
    fpGui_PanelChange fpGui_PanelChangeCb;
    fpGui_BurnIn_TestPatteren_Update fpGui_BurnIn_TestPatteren_UpdateCb;
    fpGui_IsCustom_White_Reset_ConfirmDialog fpGui_IsCustom_White_Reset_ConfirmDialogCb;
    fpGui_IsHSG_TestPattern_ON_Custom_White_Reset_ConfirmDialog fpGui_IsHSG_TestPattern_ON_Custom_White_Reset_ConfirmDialogCb;
    fpGuiStyle_MenuOffsetBlending fpGuiStyle_MenuOffsetBlendingCb;
    fpGuiStyle_MenuOffsetTopBlending fpGuiStyle_MenuOffsetTopBlendingCb;
    fpGuiStyle_MenuOffsetBottomBlending fpGuiStyle_MenuOffsetBottomBlendingCb;
    fpGuiStyle_MenuOffsetLeftBlending fpGuiStyle_MenuOffsetLeftBlendingCb;
    fpGui_SendRestoreFailMessage fpGui_SendRestoreFailMessageCb;
    fpGui_Auto_Color_Uniformity_Target_Select fpGui_Auto_Color_Uniformity_Target_SelectCb;
    fpGui_SendSplashEvent fpGui_SendSplashEventCb;
    fpGui_PanelGet fpGui_PanelGetCb;
    fpGui_IsSplash_On_Get fpGui_IsSplash_On_GetCb;
    fpGui_SendSubSourceInfoMessage fpGui_SendSubSourceInfoMessageCb;
    fpGuiStyle_CurrentTestPatternSet fpGuiStyle_CurrentTestPatternSetCb;
    fpGui_OSD_MenuTransparencyEnableSet fpGui_OSD_MenuTransparencyEnableSetCb;

    fpGui_OSD_MenuTransparencyEnableGet fpGui_OSD_MenuTransparencyEnableGetCb;
    fpGui_IsFirstStartupMenu fpGui_IsFirstStartupMenuCb;
    fpGui_IsPower_Off_Menu fpGui_IsPower_Off_MenuCb;
    fpGui_BackupInput_Inactive_Message fpGui_BackupInput_Inactive_MessageCb;
    fpGui_HWInit fpGui_HWInitCb;
    fpGui_IsAutoPowerDown_Menu fpGui_IsAutoPowerDown_MenuCb;
    fpGui_SendAutoPowerOffMenuExit fpGui_SendAutoPowerOffMenuExitCb;
    fpGui_Send_MenuOpen_UST_Message2 fpGui_Send_MenuOpen_UST_Message2Cb;
    fpGuiStyle_CurrentTestPatternGet fpGuiStyle_CurrentTestPatternGetCb;
    fpGui_SecurityShutDown_Query_Menu fpGui_SecurityShutDown_Query_MenuCb;
    fpGui_Send_SleepTimer_Query_Menu fpGui_Send_SleepTimer_Query_MenuCb;
    fpGui_AutoShutDown_Query_Menu fpGui_AutoShutDown_Query_MenuCb;
    fpGui_Send_ABC_Result fpGui_Send_ABC_ResultCb;
    fpGui_SendOSDPollEvent fpGui_SendOSDPollEventCb;
    fpGui_MsgQueueRemainSizeIsLow fpGui_MsgQueueRemainSizeIsLowCb;
    fpGui_DataCode_Value_Set fpGui_DataCode_Value_SetCb;
    fpGui_SendKeyEvent fpGui_SendKeyEventCb;
    fpGui_SpecialMenu_Check fpGui_SpecialMenu_CheckCb;
    fpGui_IsService_Code_Input_Menu fpGui_IsService_Code_Input_MenuCb;
    fpGui_IsLens_Shift_Menu fpGui_IsLens_Shift_MenuCb;
    fpGui_IsFocus_Menu fpGui_IsFocus_MenuCb;
    fpGui_IsZoom_Menu fpGui_IsZoom_MenuCb;
    fpGui_TestPatternFocusIndexEvent fpGui_TestPatternFocusIndexEventCb;
    fpGui_HSG_TestPatternEnableSet fpGui_HSG_TestPatternEnableSetCb;
    fpGui_SendSourceMessage fpGui_SendSourceMessageCb;
    fpGui_Is_NotSuported_Menu fpGui_Is_NotSuported_MenuCb;

    //fpGui_SendUpdateOSD_CLI_Event fpGui_SendUpdateOSD_CLI_EventCb;
    fpGui_PowerStandby fpGui_PowerStandbyCb;
    fpGui_Task_Suspend fpGui_Task_SuspendCb;
    fpGui_Task_Resume fpGui_Task_ResumeCb;
    fpGui_Init fpGui_InitCb;
    fpGui_PowerNormal fpGui_PowerNormalCb;
    fpGui_DataCode_Value_Get fpGui_DataCode_Value_GetCb;
    fpGui_DataCode_PointerVar_Set fpGui_DataCode_PointerVar_SetCb;
    fpGui_DataCode_String_Set fpGui_DataCode_String_SetCb;
    fpGui_Send_MenuOpen_Schedule_NextMenu_L2  fpGui_Send_MenuOpen_Schedule_NextMenu_L2Cb;
    fpGui_LenCalibrationDoing fpGui_LenCalibrationDoingCb;
    fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialog  fpGui_Send_MenuOpen_Lens_Calibration_ConfirmDialogCb;
    fpGui_USTInstall_Start fpGui_USTInstall_StartCb;
    fpGui_Send_LensMovingMenuOpen fpGui_Send_LensMovingMenuOpenCb;
    fpGui_Send_LensCalibrationMenuOpen fpGui_Send_LensCalibrationMenuOpenCb;
    fpGui_SendPinProtectClearEvent fpGui_SendPinProtectClearEventCb;

}sGUI_CALLBACK;

extern sGUI_CALLBACK GuiCb;

void GUI_RegCallback(sGUI_CALLBACK fpCallback);
sGUI_CALLBACK Gui_fpCallbackGet(void);
/////////  callback function end  /////////

INT8 Bitmap_Get(UINT32 ulBitmapID, OSD_BITMAP_INFO *psRetBitmap);
INT8 String_FillWidthInfo(UINT16 uiString_length, UINT32 ulFontOffset, UINT16 *puiChar_index, UINT8 *pucRet_char_width);
INT8 String_Get(UINT32 ulTextID, OSD_STRING_INFO * psRetString, UINT16 *puiStringWidth);
INT8 palGui_Init(void);
INT8 palGui_HWInit(void);
INT8 palGui_OSDReload(void);
#if (LOGO_REPLACE == 1)	//ZU860_Clare_0124
void palGui_LogoPaletteFlagInit(void);
void palGui_ServiceLogoPaletteFlagInit(void); //A65_OPTOMA_Julie_0076
INT8 palGui_SecondLogoHWInit(void);
INT8 palGui_Service_SecondLogoHWInit(void);   //A65_OPTOMA_Julie_0076
INT8 palGui_SecondLogoPaletteStore(char *fileName, UINT8 ucIndex);
#endif


INT8 palGui_OSD_Copy(void); //A70LV_Larry_0067
INT8 palGui_OSD_On(COORDINATE sStart_Position, BLOCK_SIZE_INFO sDraw_Size);
INT8 palGui_OSD_Off(void);
INT8 palGui_OSD_Set_InhitbitColor( INT8 cEnable, INT16 iInhibit_Color );
INT8 palGui_Language_Set(UINT8 ucLanguage);
INT8 palGui_Paint_Bitmap(UINT32 ulBitmapID, COORDINATE sStart_Position);
INT8 palGui_Paint_Specified_Bitmap(UINT32 ulBitmapID, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Position, BLOCK_SIZE_INFO sDraw_Size);
INT8 palGui_Paint_Rectangle(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index);
INT8 palGui_Paint_RectanglewithBoarder(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index, INT16 iColor_Background, INT16 iBoarder_Index); ////T100_Sander_0002
INT8 palGui_Paint_RectangFrame(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index); //A70LV_Larry_0378
INT8 palGui_Paint_Text( UINT32 ulTextID, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment );
INT8 palGui_Paint_ASCIIString( UINT8 *pucString, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment );
INT8 palGui_Paint_Number(INT32 iNumber, eNUMERIC_SIGN eNumMode, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment);
INT8 palGui_Paint_TextHightLight(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index);	//ZU860_Clare_0019,

MENU_INFO* palGui_MenuStructPoint_Get(void);
UINT32 palGui_MenuCount_Get(void);


INT8 palGui_GuiData_EraseAll(void);
INT8 palGui_GuiData_FlashEraseSector(UINT32 ulAddr, UINT32 ulSize);
INT8 palGui_GuiData_FlashWrite(UINT32 ulAddr, UINT8 *pucData, UINT32 ulSize);
//UINT32 palGui_GuiData_PartialWrite(char *fileName, UINT32 ulAddr);
UINT32 palGui_GuiData_PartialWrite(char *fileName, UINT32 ulFlashAddr, UINT32 ulRAMAddr, UINT8 ucFocus);
UINT32 palGui_Upgrade(UINT8 ucFocus);
INT8 palGui_UpdateTextGuiData(void);
INT8 palGui_UpdateBitmapGuiData(void);
INT8 palGui_InitSemaphore(void);     //A70LV_Doulas_0014

void palGui_Upgrade_Second_Logo_Replace(UINT8 ucFocus); //A65_OPTOMA_Julie_0102 //A35G2_Coda_0067
void palGui_Upgrade_Service_Second_Logo_Replace(UINT8 ucFocus); //A65_OPTOMA_Julie_0102 //A35G2_Coda_0067

INT8 palGui_Paint_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sStart_Position); //T100_Sander_0013
INT8 palGui_Paint_Specified_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Start_Position, BLOCK_SIZE_INFO sDraw_Size);	//ZU860_Clare_0132
INT8 palGui_PanelSet(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 palGui_PanelChange(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 palGui_OSD_MenuTransparencySet(UINT8 ucValue);         //A70LV_Doulas_0122
INT8 palGui_OSD_MenuTransparencyEnableSet(UINT8 ucEnable);         //A70LV_Doulas_0122
INT8 palGui_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor); //A70LV_Larry_0137
INT8 palGui_Paint_Line(COORDINATE sStart_Position, UINT8 width, INT16 iColor_Index);    //ZU860_Clare_0001, add
#ifdef LOGO_REPLACE  //SNPLU9000_Energy_0018
UINT32 palGui_GuiData_Resolved_Logo_PartialWrite(UINT32 ulExtAddr, UINT32 ulAddr, UINT32 ulSize);	//ZU860_Clare_0131
#endif
void palGui_UpgradeAccess_Set(BOOL bEnable);		//A65_OPTOMA_Doulas_0126
BOOL palGui_UpgradeAccess_Get(void);				//A65_OPTOMA_Doulas_0126

INT8 palGui_2ND_LOGO_OSDReload(UINT8 ucIndex);	//A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
void palGui_ShowOsdBitmapData(UINT32 ulStartVertPosition);    //A35G2_Simon_0114


#ifdef __cplusplus
}
#endif


#endif  //_PALGUI_H_

