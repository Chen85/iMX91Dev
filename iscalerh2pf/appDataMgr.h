#ifndef _APPDATAMGR_H_
#define _APPDATAMGR_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "utilStorageCfg.h"
#include "Common.h"

//temp
//#define DISABLE_TP_DATACODE

#define LAYOUT_VERSION_MAJOR        0x00  //A70LV_Doulas_0015
#define LAYOUT_VERSION_MINOR        0x02
#define LAYOUT_VERSION_SUBMINOR     0x17    //G100_Doulas_0066 //A70LV_Doulas_0124

#define APP_TEC_CONFIG_V321  (0x65)//101 //Cu width 1.3mm, size 62*62.
#define APP_TEC_CONFIG_V4    (0x68)//104 //Cu width 6.5mm, size reduce to 62*45.

//#define DATAMGR_ACCESS_TABLE_ENTRIES palDataMgr_DataCode_Total_Number_Get()

#define ERROR_APP_DATAMGR_FAIL -1
#define ERROR_APP_DATAMGR_DATA_OUT_OF_RANGE -2

#define SIZE_BY_DATATYPE ((UINT16)0xFFFF)

typedef struct
{
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    pthread_mutex_t              xMutex ;
#else  //freertos
    SemaphoreHandle_t            xSemaphore;

#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    StaticSemaphore_t            xMutexBuffer;
#endif
#endif

    sEEPROM_SETTINGS            sEepSettings;

    sSYSTEM_VALUES              sSystemValues;
}sAPP_DATAMGR_INFORMATION, *PsAPP_DATAMGR_INFORMATION;



typedef enum
{
    eEXE_TP_PASS,
    eEXE_TP_PROTECTED,  //need service code
    eEXE_TP_BLOCKED,

    eEXE_TP_INVALID
}eEXE_TP_RESULT;


typedef enum
{
    eACU_DISABLE_BLEND_NONE = 0,
    eACU_DISABLE_BLEND_DBD = 0x01,
    eACU_DISABLE_BLEND_TOP = 0x02,
    eACU_DISABLE_BLEND_BOTTOM = 0x04,
    eACU_DISABLE_BLEND_LEFT = 0x08,
    eACU_DISABLE_BLEND_RIGHT = 0x10,
}eACU_FORCE_DISABLE_TYPE;// R70K_Bruce_0052

typedef enum
{
    eDIM_MODE_NORMAL = 0,
    eDIM_MODE_AMBIENT_OVERTEMP,
    eDIM_MODE_DMD_OVERTEMP,

    eDIM_MODE__NUMBERS,
} eDIM_MODE_LIST;

eEXEC_CODE palDataMgr_Init(void);
eEXEC_CODE palDataMgr_PowerNormal(void);
void palDataMgr_ResetAllToDefault(void);    //G100_Owen_0046
void palSystem_DefaultValueInit(void);
void palDataMgr_DefaultSettingConfig(void);
psEEPROM_SETTINGS palDataMgr_GetDefaultSettingConfig(void);
void palDataMgr_OPDEvent(UINT8 ucEvent, eDATA_CODE eDataCode);
void palDataMgr_AccessOPDEvent(UINT8 ucEvent, eDATA_CODE eDataCode, void *pValue);  //A35G2_Simon_0075
UINT8 palDataMgr_OPDAccessLogSkip(eDATA_CODE eDataCode, eDATA_ACCESS_MODE eAccessMode);  //A35G2_Simon_0076
void palDataMgr_OPDFE_Snapshot(UINT8 ucEvent); //G100_John_0004 fix OPDFE warning
void palDataMgr_OPDSnapshot(UINT8 ucEvent);
UINT8 palDataMgr_Data_Type(eDATA_CODE eDataCode); //A70LV_Larry_0065
eFUNC_CONTROL palDataMgr_MenuItem_Control(eDATA_CONTROL eDataControl);
eFUNC_CONTROL palDataMgr_DataCode_Control(eDATA_CODE eDataCode);
BOOL palDataMgr_NetworkCommandSkip(eDATA_CODE eDataCode); //A70LV_Larry_0172
INT8 palDataMgr_IsNetworkDataCode(eDATA_CODE eDataCode); //G100_Simon_0036
INT8 palDataMgr_IsArtNetDataCode_WithoutArtNetReset(eDATA_CODE eDataCode);
INT8 palDataMgr_IsArtNetDataCode(eDATA_CODE eDataCode);
INT8 palDataMgr_IsUIEventDataCode(eDATA_CODE eDataCode);
#ifdef PALDATAMGR_ACCESS_WITHLOG
eEXEC_CODE palDataMgr_Data_Access_WithLog(eDATA_CODE eDataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue, const char *cFuncName, UINT32 ulLineNum);
#else
eEXEC_CODE palDataMgr_Data_Access(eDATA_CODE eDataCode, eDATA_ACCESS_MODE eAccessMode, void *pValue);
#endif
char* palDataMgr_DataCodeStringGet(eDATA_CODE eDataCode);
eEXEC_CODE palDataMgr_Data_Range_Get(eDATA_CODE eDataCode, eDATA_RANGE_MODE eRange, tDATA_CODE *pValue);

eEXEC_CODE palDataMgr_Data_ReadDefaultWrite(eDATA_CODE eDataCode);// R70K_Bruce_0065
eEXEC_CODE palDataMgr_Data_ReadDefaultWriteNoAction(eDATA_CODE eDataCode);//HICC2_Doulas_0109
eEXEC_CODE palDataMgr_Data_ReadCurrentWrite(eDATA_CODE eDataCode);// R70K_Bruce_0065
eEXEC_CODE palDataMgr_ColorSetting_toEEPROM_bySource(UINT8 ucInputSource);// R70K_Bruce_0065
eEXEC_CODE palDataMgr_HSG_toEEPROM_bySource(UINT8 ucInputSource);// R70K_Bruce_0065
eEXEC_CODE palDataMgr_UserColorSetting_toEEPROM_bySource(UINT8 ucInputSource);// R70K_Bruce_0069 //A70Gen2_Julie_0092
eEXEC_CODE palDataMgr_UserHSG_toEEPROM_bySource(UINT8 ucInputSource);// R70K_Bruce_0069 //A70Gen2_Julie_0092

void palDataMgr_Scaler_EEPROM_Iint(void);  //A70LV_Doulas_0013

//A70LV_Doulas_0015
void palDataMgr_sLayoutVersion_Init_Default(void);
void palDataMgr_sADC_cal_values_Init_Default(void);
void palDataMgr_sWaveformstate_Init_Default(void);
void palDataMgr_sBurin_Information_Init_Default(void);
void palDataMgr_sSystemDefault_Init_Default(void);
void palDataMgr_sUserSystemSetting_Init_Default(void);
void palDataMgr_sTiming_Table_Init_Default(void);    //A70LV_Doulas_0194
void palDataMgr_sLightSetting_Init_Default(void);
void palDataMgr_sOSD_Setting_Init_Default(void);
void palDataMgr_sNetworkSetting_Init_Default(void);
void palDataMgr_sImageSetting_Init_Default(void);
void palDataMgr_sWarpSetting_Init_Default(void);
void palDataMgr_sCommonSetting_Init_Default(void);
void palDataMgr_sSourceSetting_Init_Default(void);
void palDataMgr_sManualAdjustment_Init_Default(void);
void palDataMgr_sSourceDependSetting_Init_Default(void);
void palDataMgr_sHSG_setting_Init_Default(void);
void palDataMgr_sColorSetting_Init_Default(void);
void palDataMgr_sPWM_setting_Init_Default(void);
void palDataMgr_sHSG_ColorTemperature_Init_Default(void);
void palDataMgr_sGamma_Init_Default(void);

void palDataMgr_sADC_CAL_VALUES_Factory_Reset(void);
void palDataMgr_sWaveformstate_Factory_Reset(void);
void palDataMgr_sBurin_Information_Factory_Reset(void);
void palDataMgr_sSystemDefault_Factory_Reset(void);
void palDataMgr_sLightSetting_Factory_Reset(void);
void palDataMgr_sOSD_Setting_Factory_Reset(void);
void palDataMgr_sNetworkSetting_Factory_Reset(void);
void palDataMgr_sImageSetting_Factory_Reset(void);
void palDataMgr_sWarpSetting_Factory_Reset(void);
void palDataMgr_sCommonSetting_Factory_Reset(void);
void palDataMgr_sSourceSetting_Factory_Reset(void);
void palDataMgr_sManualAdjustment_Factory_Reset(void);
void palDataMgr_sSourceDependSetting_Factory_Reset(void);
void palDataMgr_sHSG_setting_Factory_Reset(void);
void palDataMgr_sColorSetting_Factory_Reset(void);
void palDataMgr_sTiming_Table_Factory_Reset(void);   //A70LV_Doulas_0194

void palDataMgr_sWaveformstate_Reset_Check(void);
void palDataMgr_sBurin_Information_Reset_Check(void);
void palDataMgr_sSystemDefault_Reset_Check(void);
void palDataMgr_sLightSetting_Reset_Check(void);
void palDataMgr_sOSD_Setting_Reset_Check(void);
void palDataMgr_sNetworkSetting_Reset_Check(void);
void palDataMgr_sImageSetting_Reset_Check(void);
void palDataMgr_sWarpSetting_Reset_Check(void);
void palDataMgr_sCommonSetting_Reset_Check(void);
void palDataMgr_sSourceSetting_Reset_Check(void);
void palDataMgr_sSourceDependSetting_Reset_Check(void);
void palDataMgr_sColorSetting_Reset_Check(void);

UINT8 palDataMgr_Reset_RGB_Gain_Offset(void);        //A70LV_Doulas_0022
UINT8 palDataMgr_Reset_All_RGB_Gain_Offset(void);    //HICC2_Steven_0049
UINT8 palDataMgr_3D_Timing_Get(void);
UINT8 palDataMgr_SignalType_Get(void);
UINT8 palDataMgr_CopyUserColorSetting(void);
UINT8 palDataMgr_SaveUserColorSettingToEEPROM(void);

//eEXEC_CODE palDataMgr_Access_Factory_Reset_ALL(eDATA_ACCESS_MODE eAccessMode, void *pValue);     //A70LV_Doulas_0023
eEXEC_CODE palDataMgr_Access_TotalProjectorHours(eDATA_ACCESS_MODE eAccessMode, void *pValue); //A70LV_John_0009 add timer for GEC
eEXEC_CODE palDataMgr_Access_LD_Info(eDATA_ACCESS_MODE eAccessMode, void *pValue);

eFUNC_CONTROL palDataMgr_LanApplyAvalibleCheck(void);     //A70LV_Doulas_0025
eFUNC_CONTROL palDataMgr_WLanApplyAvalibleCheck(void); //A70LV_Larry_0135
void palDataMgr_LanArgCopy1(void);
void palDataMgr_LanArgCopy2(void);
void palDataMgr_WLanArgCopy1(void); //A70LV_Larry_0135
void palDataMgr_WLanArgCopy2(void); //A70LV_Larry_0135
void palDataMgr_CrestronArgCopy1(void);  //G100_Wilsonj_0055
void palDataMgr_CrestronArgCopy2(void);  //G100_Wilsonj_0055
void palDataMgr_PJLinkArgCopy1(void);  //G100_Wilsonj_0059
void palDataMgr_PJLinkArgCopy2(void);  //G100_Wilsonj_0059
void palDataMgr_LAN_IP_Copy1_All(void); //G100_Larry_0035
void palDataMgr_LAN_IP_Copy2_All(void); //G100_Larry_0035
void palDataMgr_LanArg_Send_Mcu(UINT16 uiDataCode);	//A35Gen2_Coda

void palDataMgr_IPV6_LanArgCopy1(void); // HICC2_Bruce_0020
void palDataMgr_IPV6_LanArgCopy2(void);
void palDataMgr_IPV6LanArg_Send_Mcu(UINT16 uiDataCode);

eEXEC_CODE palDataMgr_Access_ChangePIN(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_SubSourceChecking(void);     //A70LV_Doulas_0030

UINT8 palDataMgr_Format_Init(void);
UINT8 palDataMgr_Format_Normal_WithSemaphoreSet(void);
UINT8 palDataMgr_Format_PictureMode_Set(void);
UINT8 palDataMgr_Formatter_HSG_CE_Set(void);
UINT8 palDataMgr_Formatter_3D_Set(void);
UINT8 palDataMgr_OSDResetExecuteActionfunction(void);
UINT8 palDataMgr_FactoryResetExecuteActionfunction(void);   //A70LV_Doulas_0049 Add
UINT8 palDataMgr_Formatter_ForcePictureMode_Set(UINT8 ucPictureMode);
UINT8 palDataMgr_Formatter_Parameter_Set(UINT8 ucForce);   //A70LV_Doulas_0054
UINT8 palDataMgr_LightSource_Set(UINT8 ucPowerMode, UINT8 ucPowerNumber); //A70LV_Larry_0087
eEXEC_CODE palDataMgr_MainInputSourceChangeToNext(void);      //A70LV_Doulas_0082
eFUNC_CONTROL palDataMgr_Control_SourceDepend(void);  //A70LV_Larry_0108
eFUNC_CONTROL palDataMgr_Control_MainOrSubSourceDepend(void);
eFUNC_CONTROL palDataMgr_Control_PIP_PBP_Enable(void); //A70LV_Larry_0350
eFUNC_CONTROL palDataMgr_Control_ContrastEnhancement(void); //A70LV_Larry_0111
eFUNC_CONTROL palDataMgr_Control_Scaler(void); //A70LV_Larry_0111
UINT8 palDataMgr_ResetPhaseRam(void);   //A70LV_Doulas_0117
UINT8 palDataMgr_ResetDigitalZoomShiftRam(void);   //A70LV_Doulas_0117
void palDataMgr_ResetDigitalZoomShift(void);
UINT8 palDataMgr_ResetAllHoursToDefault(void);      //A70LV_Doulas_0128
void palDataMgr_SourceName_Get(UINT8 ucSource, char *cDynamicString);
void palDataMgr_ServiceModeSet(UINT8 ucVal);      //A70LV_Doulas_0138
UINT8 palDataMgr_ServiceModeGet(void);
UINT8 palDataMgr_IS_3D_Enable(void);         //A70LV_Doulas_0153
void palDataMgr_Scaler_Mode_Adjustment_EEPROM_Iint(void);  //A70LV_Doulas_0195
BOOL palDataMgr_Is_Formatter_Setting(void);     //A70LV_Doulas_0206
UINT8 palDataMgr_UserSrcID_To_SystemSrcID(UINT8 ucUserSourceID);  //T100_Simon_0038
UINT8 palDataMgr_SystemSrcID_To_UserSrcID(UINT8 ucSysSourceID);   //T100_Simon_0038
void palDataMgr_XFPGA_LVDS_COVER_Set(UINT8 ucLVDS_COVER_En);      //A70LV_Doulas_0262
void palDataMgr_HV_Start_Position_Reset(void);                    //A70LV_Doulas_0269
UINT8 palDataMgr_ResetCountGet(void); //A70LV_Larry_0354
void palDataMgr_ConfigurationSave(UINT8 ucIndex);            //A70LV_Doulas_0279
sBACKUP_RESTORE palDataMgr_ConfigurationRead(void);    //A70LV_Doulas_0280
//sBACKUP_RESTORE palDataMgr_ConfigurationGet(UINT8 ucIndex);    //A70LV_Doulas_0279
//void palDataMgr_ConfigurationRestore(UINT8 ucIndex,sBACKUP_RESTORE sBackRestore);         //A70LV_Doulas_0279
void palDataMgr_DisplayModeChecking(void);         //ZU860_Doulas_0045
void palDataMgr_3D_Check(void);        //ZU860_Doulas_0138
void palDataMgr_DataSyncToDatabase(void);
void palDataMgr_BeforeDisplayModeToDefault(void);
UINT8 palDataMgr_BeforeDisplayModeGet(void);
void palDataMgr_ImportAllDataToDatabase(void);
void palDataMgr_ImportDataToDatabase(eDATA_CODE eDatacode);
void palDataMgr_TecGatingInfo_Get(sTEC_GATING_INFO *psEEP);
void palDataMgr_TecGatingInfo_Set(sTEC_GATING_INFO *psEEP);
UINT8 palDataMgr_Access_Get_FAN_Duty(UINT8 FAN_Index); //G100_Steven_0003
UINT16 palDataMgr_Access_Get_FAN_RPM(UINT8 FAN_Index);
UINT16 palDataMgr_Access_Get_LD_Voltage(UINT8 ucIndex);
UINT16 palDataMgr_Access_Get_LD_Current(UINT8 ucIndex);
UINT16 palDataMgr_Access_Get_ALTIMETRY(void);	//G100_Clare_0018
UINT16 palDataMgr_Access_Get_TEC_Current(UINT8 ucIndex);	//G100_Clare_0018
UINT16 palDataMgr_Access_Get_TEC_Voltage(UINT8 ucIndex); //HICC2_Doulas_0054
INT16 palDataMgr_Access_Get_LD_Temperature(UINT8 ucIndex);
INT16 palDataMgr_Access_Get_Thermal_Sensor(UINT8 ucIndex);
UINT16 palDataMgr_Access_Get_Pump(void); //A70Gen2_Julie_0006
UINT16 palDataMgr_Access_Get_FanFilter(void); //A70Gen2_Julie_0034
eEXEC_CODE palDataMgr_Access_Color_Uniformity_Table_Offset_Get(UINT32 *dwOffset);
UINT8 palDataMgr_ColorSetting_Get(UINT8  ucNode); //G100_Steven_0016
void palDataMgr_ColorSetting_Set(UINT8  ucNode, UINT8 ucValue); //G100_Steven_0033
//void palDataMgr_sColorSetting_UserMode_Init_Default(void);         //G100_Steven_0018
void palDataMgr_sHSG_UserMode_setting_Init_Default(void);           //G100_Steven_0018
void palDataMgr_UpdatePreUser(void);   //G100_Steven_0018
UINT8 palDataMgr_HSG_Selected_Get(UINT16 ucNode);
void palDataMgr_HSG_Selected_Set(UINT16 ucNode, eDATA_ACCESS_MODE eAccessMode, UINT8 ucVal);
void palDataMgr_LAN_IP_ADDRESS(void);   //G100_Julie_0014
void palDataMgr_OPDOperation(UINT8 ucEvent); //G100_Julie_0016
void palDataMgr_OPDSourceInfo(UINT8 ucCH);   //G100_Julie_0016
eEXEC_CODE palDataMgr_Access_HDMI_OUT(eDATA_ACCESS_MODE eAccessMode, void *pValue); //G100_Steven_0021
UINT8 palDataMgr_Init_FrontEnd_BackUpInput(void);  //G100_Steven_0036

eEXEC_CODE palDataMgr_Access_HSG_Red_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);	 //G100_Coda_00110
eEXEC_CODE palDataMgr_Access_HSG_Green_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_HSG_Blue_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_HSG_Cyan_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_HSG_Magenta_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_HSG_Yellow_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_HSG_WhiteGain_ResetDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Apply_User_Mode(eDATA_ACCESS_MODE eAccessMode, void *pValue);		//G100_Doulas_0066
eEXEC_CODE palDataMgr_Access_Date_Time_Zone_Edit_Num(eDATA_ACCESS_MODE eAccessMode, void *pValue); //G100_Coda_0036
eEXEC_CODE palDataMgr_Access_Real_DateTime(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_Schedule_All_Data_Get(sSCHEDULE * sSchudeleData);
void palDataMgr_Schedule_Unpack_LanPacket(sLAN_SCHEDULE_PACKET * sLanSchudelePacket);
void palDataMgr_Schedule_Build_LanPacket(sLAN_SCHEDULE_PACKET *sLanSchudelePacket);
void palDataMgr_DateTime_Unpack_LanPacket(_sDateTime * sLanDateTimePacket);
void palDataMgr_DateTime_TempData_Get(sDATE_TIME_INFO * sTempTimeDateInfo);
void palDataMgr_DateTime_TempData_Set(sDATE_TIME_INFO * sTempTimeDateInfo);
void palDataMgr_DateTime_Build_LanPacket(_sDateTime *sDateTimePacket);
void palDataMgr_DateTime_TimeZoneStr_Get(char *pcTimeZoneStr);  //G100_Owen_0053
void palDataMgr_DateTime_TimeZoneStr_Set(BOOL bSinged, char *pcTimeZoneStr);    //G100_Owen_0053
eEXEC_CODE palDataMgr_Access_DateTimeApply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eFUNC_CONTROL palDataMgr_Control_DateTime_Mode_Control(void);
eFUNC_CONTROL palDataMgr_Control_DateTime_Mode_USE_NTP_Control(void);
void palDataMgr_DateTime_Build_LanPacket_ResetDefault(void);	 //G100_Coda_00110
eEXEC_CODE palDataMgr_Access_DateTimeReset(eDATA_ACCESS_MODE eAccessMode, void *pValue);	 //G100_Coda_00110
eEXEC_CODE palDataMgr_AccessADV_WarpInnerGridPoints(eDATA_ACCESS_MODE eAccessMode, void *pValue);	//A65_OPTOMA_Doulas_0132
#ifdef CUSTOM_OPTOMA
eEXEC_CODE palDataMgr_Access_HSGColor(eDATA_ACCESS_MODE eAccessMode, void *pValue);	//A65_OPTOMA_Doulas_0138
#endif
eEXEC_CODE palDataMgr_Access_Model_Switch_Adjust(eDATA_ACCESS_MODE eAccessMode, void *pValue);//A35G2_Coda_0063



//G100_Clare_0022, add, >>>
void palDataMgr_Camera_OSD_Lock_Set(UINT8 ucVal);
UINT8 palDataMgr_Camera_OSD_Lock_Get(void);
eEXEC_CODE palDataMgr_Access_OSD_Lock(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_FOTA_Enable(eDATA_ACCESS_MODE eAccessMode, void *pValue);//A65_OPTOMA_Jerry_0005
void palDataMgr_Camera_Module_Status_Set(UINT8 ucVal);
UINT8 palDataMgr_Camera_Module_Status_Get(void);
eEXEC_CODE palDataMgr_Access_Camera_Module_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_AutoFocusExecute_Set(UINT8 ucVal);
UINT8 palDataMgr_AutoFocusExecute_Get(void);
eEXEC_CODE palDataMgr_Access_Auto_Focus(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_Auto_Wall_Color_Set(UINT8 ucVal);
UINT8 palDataMgr_AutoWallColorExecute_Get(void);
void palDataMgr_AutoColorMatchExecute_Set(UINT8 ucVal);
UINT8 palDataMgr_AutoColorMatchExecute_Get(void);
eEXEC_CODE palDataMgr_Access_Auto_Wall_Color(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_AC_Apply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Auto_Focus_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Auto_Wall_Color_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eFUNC_CONTROL palDataMgr_Control_Camera_Lens(void);
eEXEC_CODE palDataMgr_Access_LogoDelete(eDATA_ACCESS_MODE eAccessMode, void *pValue);   //G100_Owen_0048
//G100_Clare_0022, add, <<<
eFUNC_CONTROL palDataMgr_Control_ADV_Warping(void);			//G100_Doulas_0027
eFUNC_CONTROL palDataMgr_Control_ADV_WarpingInner(void);		//G100_Doulas_0027
void palDataMgr_ADV_WarpingParameterInit(void);				//G100_Doulas_0027
void palDataMgr_ADV_Blend_LfRtTpBm_Update(void);			//G100_Doulas_0027
UINT8 palDataMgr_ApplyWarpMemoryItemAvailable(UINT8 ucIdx);			//G100_Doulas_0040
UINT8 palDataMgr_ApplyBlendMemoryItemAvailable(UINT8 ucIdx);
void palDataMgr_ImageSetting_EDID_Type_Set(UINT8 cSource, UINT8 cType); //G100_John_0003 fix EDID version change issue
UINT8 palDataMgr_User_Mode_CM_Get(void); //G100_Steven_0027
eEXEC_CODE palDataMgr_Access_LogoChange(eDATA_ACCESS_MODE eAccessMode, void *pValue);   //G100_Owen_0048
//UINT8 palDataMgr_User_Mode_Trans2_Color_Mode(UINT8 cUserMode); //G100_Steven_0029
//UINT8 palDataMgr_Color_Mode_Trans2_User_Mode(UINT8 cColorMode); //G100_Steven_0029
void palDataMgr_UserMode_sHSG_setting_Factory_Reset(void); //G100_Steven_0030
void palDataMgr_UserMode_sColorSetting_Factory_Reset(void); //G100_Steven_0030
void palDataMgr_ColorSetting_toEEPROM(UINT8 ucNode, UINT8 ucVal);//G100_Steven_0033
void palDataMgr_HSG_SetAction(void);//G100_Steven_0033
UINT8 palDataMgr_Check_User_Color_Mode(UINT8 ucColor); //G100_Steven_0037
UINT16 palDataMgr_HSG_RAM_Get(UINT8 ucNode);           //G100_Steven_0037
void palDataMgr_sColorSetting_EEPROM_Reset(void); //G100_Steven_0038
eEXEC_CODE palDataMgr_Access_AC_Voltage_Info_Get(eDATA_ACCESS_MODE eAccessMode, void *pValue);	//G100_Clare_0042
#if 0 //Depends on X35Gen2 design
void palDataMgr_Check_ModelID(void); //G100_Steven_0039
#endif
//void palDataMgr_3DModelID_Set(UINT8 ucData);  //G100_Steven_0052
eFUNC_CONTROL palDataMgr_Control_PIP(void);
void palDataMgr_WarpingMemorySaveConditionUpdate(void);
void palDataMgr_BlendingMemorySaveConditionUpdate(void);
#ifdef FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC	//G100_Doulas_0071
void palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Set(UINT8 ucData);
UINT8 palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Get(void);
#endif
eEXEC_CODE palDataMgr_Access_Fast_Power_On(eDATA_ACCESS_MODE eAccessMode, void *pValue);	//G100_Clare_0055
UINT8 palDataMgr_Fast_Power_On_Get(void);	//G100_Clare_0055
eFUNC_CONTROL palDataMgr_Control_Lens_Calibration(void); //G100_Owen_0063
UINT8 palDataMgr_BKInput_Trans2_CMInput(UINT8 ucBKInput); //G100_Steven_0055
eEXEC_CODE palDataMgr_Access_Backup_Restore_Restore(eDATA_ACCESS_MODE eAccessMode, void *pValue);		//G100_Doulas_0075
eFUNC_CONTROL palDataMgr_Control_Backup_Restore_Load(void);   //G100_Doulas_0075
int palDataMgr_IP_Conflict_Verify(void);   //G100_Julie_0035
void palDataMgr_WriteSSTinfo(void);   //G100_Julie_0036
void palDataMgr_WarppingControlAdvanceSet(void);		//G100_Doulas_0079
#if 0
void palDataMgr_SKUTypeSet(void);	//G100_Doulas_0080
#endif
void palDataMgr_OPDFWversion(UINT8 ucEvent); //G100_Julie_0044
void palDataMgr_OPDRegulatoryInfo(void);
void palDataMgr_SetBackupRestoreExecution(UINT8 ucData); //G100_Steven_0064
UINT8 palDataMgr_GetBackupRestoreExecution(void);        //G100_Steven_0064
void palDataMgr_ExecuteStandbyMode(UINT8 ucData); //G100_Steven_0072
void palDataMgr_ExecuteNetworkFactoryReset(UINT8 ucData); //G100_Steven_0072
void palDataMgr_ExecuteLanIPApply(UINT8 ucData); //G100_Steven_0072
#ifdef CUSTOM_OPTOMA
eFUNC_CONTROL palDataMgr_Control_3D_InvertDepend(void);// BruceLin#20210111
#endif
void palDataMgr_SubColorSetting_Set(UINT8  ucNode, UINT8 ucValu); //G100_Steven_0079
UINT8 palDataMgr_SubColorSetting_Get(UINT8  ucNode);
void palDataMgr_SubColorSetting_toEEPROM(UINT8 ucNode, UINT8 ucVal); //G100_Steven_0079
eFUNC_CONTROL palDataMgr_Control_PIPPBP_Sub_ColorSetting(void); //G100_Steven_0088

eEXEC_CODE palDataMgr_AccessADV_BlendingOverlapGridNumber(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_AccessADV_BlendingGamma(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_CurrentWarpMemoryType(void);
UINT8 palDataMgr_CurrentBlendMemoryType(void);
void palDataMgr_MenuOffsetUpdateForEdgeBlending(void);
void palDataMgr_AdvReset_Execution(UINT8 ucSelect); //G100_Owen_0091 //A35G2_BRC_Casper_0046
eEXEC_CODE palDataMgr_Access_SNMP_Reset(eDATA_ACCESS_MODE eAccessMode, void *pValue);   //G100_Tim_0019, add //A35G2_BRC_Casper_0047
eEXEC_CODE palDataMgr_Access_LensMemory_Clear(eDATA_ACCESS_MODE eAccessMode, void *pValue);
INT8 palDataMgr_HSGSelectedValue_Default_Get(eDATA_CODE eDataCode, tDATA_CODE *pValue);
//G100_Tim_0057, add, start //A35G2_BRC_Casper_0060
UINT8 palDataMgr_Input_Key_Last_Value_RAM_Get( void );
void  palDataMgr_Input_Key_Last_Value_RAM_Set( UINT8 ucValue );
UINT8 palDataMgr_3D_Mode_Last_Value_RAM_Get( void );
void  palDataMgr_3D_Mode_Last_Value_RAM_Set( UINT8 ucValue );
void  palDataMgr_Auto_HDMI_Switch_Change_Input_Key( eSOURCE_KEY_ITEM eInput_Key);
void  palDataMgr_Auto_HDMI_Switch_Change_3D_Mode( eTOGGLE_SETTING e3D_On_Off);
void  palDataMgr_Auto_HDMI_Switch_ReConfig(UINT8 ucStatus);
UINT8 palDataMgr_Auto_HDMI_Switch_RAM_Get(void);
void  palDataMgr_Auto_HDMI_Switch_RAM_Set( UINT8 ucValue );
eEXEC_CODE palDataMgr_Access_Auto_HDMI_Switch(eDATA_ACCESS_MODE eAccessMode, void *pValue);
//G100_Tim_0057, add, end
//G100_Tim_0058, add, start //A35G2_BRC_Casper_0060
eEXEC_CODE palDataMgr_Access_Proj_Native_Timing(eDATA_ACCESS_MODE eAccessMode, void *pValue);
//G100_Tim_0058, add, end

UINT8 palDataMgr_ACU_Execute_Get( void );
void palDataMgr_ACU_Execute_Set( UINT8	ucData );
eEXEC_CODE palDataMgr_Access_ACU_Execute(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_ACU_Apply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_ACM_Select(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_ACU_Status_Get( void );
void palDataMgr_ACU_Status_Set( UINT8  ucData );
UINT8 palDataMgr_ACU_Target_Status_Get( void );         //A65_OPTOMA_Julie_0082  //A35G2_CDS_Coda_0027
void palDataMgr_ACU_Target_Status_Set( UINT8  ucData ); //A65_OPTOMA_Julie_0082
eEXEC_CODE palDataMgr_Access_ACU_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_ACU_Target_Select_Get(void);
UINT8 palDataMgr_ACU_Target_Select_Tmp_Get(void);
eEXEC_CODE palDataMgr_ACU_Target_Select_Set(UINT8 ucValue);
void palDataMgr_ACU_Target_Select_Tmp_Set(UINT8 ucValue);
void palDataMgr_ACU_Target_Select_Sync(void);
eEXEC_CODE palDataMgr_Access_ACU_Target_Select(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_ACU_Reset_Get(void);
void palDataMgr_ACU_Reset_Set(UINT8 ucValue);
UINT8 palDataMgr_ACU_DisableBlend_Get(void);
void palDataMgr_ACU_DisableBlend_Set(UINT8 ucData, BOOL bSemaphore);
eEXEC_CODE palDataMgr_Access_ACU_Reset(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_CU_Data_Status_Get(void);  //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
UINT8 palDataMgr_CU_Data_Enable_Get(void);  //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
void palDataMgr_CU_Data_Enable_Set(UINT8  ucData);  //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
eEXEC_CODE palDataMgr_Access_CU_Data_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
eEXEC_CODE palDataMgr_Access_CU_Data_Enable(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
#ifdef ENABLE_ACU_CHECK_DISPLAY_MODE
eEXEC_CODE palDataMgr_ACU_Display_Mode_Check( void );
#endif //ENABLE_ACU_CHECK_DISPLAY_MODE
eEXEC_CODE palDataMgr_Access_ACU_Save(eDATA_ACCESS_MODE eAccessMode, void *pValue);

eEXEC_CODE palDataMgr_Access_ProService_Pair_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue); //A65_OPTOMA_Jerry_0004 start
eEXEC_CODE palDataMgr_Access_ProService_Binding_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Proservice_PairCode(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_ProService_Account(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_ProServiceErrorCode(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_ProServiceLocation(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_UI_Event_BroadcastMsg(eDATA_ACCESS_MODE eAccessMode, void *pValue); //A65_OPTOMA_Jerry_0004 end

#if (LOGO_REPLACE == 1)
eEXEC_CODE palDataMgr_Access_2nd_Logo_Replaced(eDATA_ACCESS_MODE eAccessMode, void *pValue);
#endif

UINT8 palDataMgr_Model_Replace_Check(void);          //A65_OPTOMA_Julie_0056
void palDataMgr_Model_Replace_PowerOff_Restart(void);//A65_OPTOMA_Julie_0056
eEXEC_CODE palDataMgr_Access_BackGround_Color(eDATA_ACCESS_MODE eAccessMode, void *pValue); //A65_OPTOMA_Julie_0064
eEXEC_CODE palDataMgr_CustomerSplah(eDATA_ACCESS_MODE eAccessMode, void *pValue);   //A65_OPTOMA_Julie_0065

#ifdef NO_POWER_OFF_DURING_CAMERA_WORKING                   //G100_Tim_0046, add, start
eCAMERA_STATUS palDataMgr_Get_Camera_Working_Status(void);
#endif //NO_POWER_OFF_DURING_CAMERA_WORKING                 //G100_Tim_0046, add, end

UINT8 palDataMgr_LogoReplace_DisplayCheck(void); //A65_OPTOMA_Julie_0079
void palDataMgr_3DModeConditionUpdate(void); //A70LK_Jacky_0014
int palDataMgr_CopyTar2Usb(void);
void palDataMgr_OPD_LensCalibration(void);
void palDataMgr_OPD_LensMemorySet(UINT8 ucIndex);
void palDataMgr_OPD_LensOpticaCenter(void);
void palDataMgr_OPDMutexLock(UINT16 uiEvent, eDATA_CODE eCurrent, eDATA_CODE eLast);  //A35G2_Simon_0075
void palDataMgr_OPDInputPlugInState(UINT8 ucEvent);  //A35G2_Simon_0075
eDMD_TYPE palDataMgr_DMD_Type_Get(void); //A35G2_Coda_0090
BOOL palDataMgr_IsSecuritySettingDefault(void); //A35G2_Coda_0098
eEXEC_CODE palDataMgr_Access_Focus_Move(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //A65_OPTOMA_CL_0008
eEXEC_CODE palDataMgr_Access_Warping_Apply(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //A65_OPTOMA_CL_0010
eEXEC_CODE palDataMgr_Access_DMD_199(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //B35LC_Tim_0111, add  //A65_OPTOMA_CL_0011
void palDataMgr_Access_3D_Frame_Sequential_OSDFrmae_Upated(void);
UINT8 palDataMgr_BackupRestoreExecuteActionfunction(void);//A35G2_Coda_0107
eEXEC_CODE palDataMgr_Access_ProServiceTemperatureStatus(eDATA_ACCESS_MODE eAccessMode, void *pValue);   //A35G2_BRC_Casper_0117
eEXEC_CODE palDataMgr_Access_ProServiceFanStatus(eDATA_ACCESS_MODE eAccessMode, void *pValue);           //A35G2_BRC_Casper_0117
eEXEC_CODE palDataMgr_Access_PictureSettings(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Apply_User_Mode(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_PowerMode(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_TestPatternSet(eCM_TEST_PATTERN_ID ePatternID);
void palDataMgr_PictureModeGet(UINT8 *CurrentPictureMode);
eEXEC_CODE palDataMgr_Access_AutoShutdownTimer_Reset(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_UI_EventSend_Debug(eDATA_CODE DataCode, UINT32 IntData, void *VoidData, const char *func, UINT32 LineNum);
eEXEC_CODE palDataMgr_Access_SystemPower(eDATA_ACCESS_MODE eAccessMode, void *pValue);    //B35G2_Ashton_0008
PLATFORM_ID_INDEX palDataMgr_Platform_ID_Get(void);
eFUNC_CONTROL palDataMgr_Control_DateTime_Mode_Manual_Control(void);
void palDataMgr_Model_ID_Set(void);
UINT16 palDataMgr_DataSizeGet(eDATA_CODE eDataCode);
BOOL palDataMgr_IsOsdOpen(void);
eEXEC_CODE palDataMgr_Access_LIGHT_SENSOR_INFO_1(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LIGHT_SENSOR_INFO_2(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LIGHT_SENSOR_INFO_3(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LIGHT_SENSOR_INFO_4(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LIGHT_SENSOR_INFO_5(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_LD_Info_Num_Get(void);
UINT8 palDataMgr_BLD_Num_Get(void);
UINT8 palDataMgr_RLD_Num_Get(void);
void palDataMgr_WriteSSTinfo(void);
int utilDataMgr_ReadFile_USBtest_periphery(BYTE *cimx, BYTE *cext, BYTE *cddp, BYTE *cksz); //A70Gen2_Julie_0048
eEXEC_CODE palDataMgr_Access_TestPattern_List(UINT32 eDatacode, eDATA_ACCESS_MODE eAccessMode, void *cCMValue);
//eEXEC_CODE palDataMgr_Access_AFN_WarpMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
//eEXEC_CODE palDataMgr_Access_AFN_BlendMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
//eEXEC_CODE palDataMgr_Access_AFN_Backup_Restore_Restore(eDATA_ACCESS_MODE eAccessMode, void *pValue); //HICC2_Doulas_0039
//eTEST_PATTERN_ID palDataMgr_TestPatternIDGet(eDATA_CODE eDatacode, UINT32 ID); //HICC2_Doulas_0049
eEXE_TP_RESULT palDataMgr_TestPatternHandle(eTEST_PATTERN_ID eTP_ID);
eTEST_PATTERN_ID palDataMgr_CurTestPatternGet(void);
eEXEC_CODE palDataMgr_Access_System_Protect_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_PIN_Protect_Status(void);
UINT8 palDataMgr_PIN_Protect_Checking(void);
eEXEC_CODE palDataMgr_Access_SUPPORT_MESSAGE(eDATA_ACCESS_MODE eAccessMode, void *pValue); //HICC2_Doulas_0053
eEXEC_CODE palDataMgr_Access_DataCodeResetToDefault(eDATA_ACCESS_MODE eAccessMode, void *pValue); //HICC2_Doulas_0054
INT8 palDataMgr_DataCode_Default_Get(eDATA_CODE eDataCode, tDATA_CODE *pValue); //HICC2_Doulas_0054
void palDataMgr_CCT_Get(uCCT_TABLE *psEEP);
void palDataMgr_CCT_Set(uCCT_TABLE *psEEP);
UINT16 palDataMgr_OPD_Period_Get(void);  //HICC2_Steven_0009
UINT8  palDataMgr_BIST_Status_Get(void);   //HICC2_Steven_0012
void palDataMgr_BISTToDefault(void);       //HICC2_Steven_0012
void palDataMgr_GetReleaseVersion(void);
eFUNC_CONTROL palDataMgr_Control_HighAltitude(void);//HICC2_Julie_0020
void palDataMgr_Schedule_Sort_Event_List_By_ExecuteTime(void);  //G100_Coda_0029
void palDataMgr_ApplyMemoryAvailablePrint(void);
eEXEC_CODE palDataMgr_Access_Group_Id(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Group_Status(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_GroupFunctionProcessed(UINT16 uiDataCode);
void palDataMgr_CheckGroupSupported(); // HICC2_Ashton_0003
BOOL palDataMgr_IsGroupSupported(); // HICC2_Ashton_0003
eEXEC_CODE palDataMgr_Access_ColorSpace(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_MenuShowMessages(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_InputKey(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_DataSyncStatus(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_UI_DataCodeAndUpdateOSDEvent(UINT16 uiData);
eEXEC_CODE palDataMgr_NotifyGroupingEvent(sDATABASE_ITEM_DATA_FORMAT sData);
void palDataMgr_WheelSpeedPollingSetting(UINT16 uiFWSpeed, UINT16 uiPWSpeed);
void palDataMgr_UI_EVENT_ABP_Cal_Msg_Open(void);
void palDataMgr_UI_EVENT_Pin_Protect_Clear(void);
UINT8 palDataMgr_AcuSelect_HDR_Gamma_Check(void); //HICC2_Julie_0052
void palDataMgr_LanShowArgCopyDefault(void);//R70CDS_David_0017//HICC2_Julie_0057
eEXEC_CODE palDataMgr_Default_LanInfo_Get(void);//HICC2_Julie_0057
void palDataMgr_IPV6_LanShowArgCopyDefault(void);// HICC2_Bruce_0020
eEXEC_CODE palDataMgr_IPV6_Default_LanInfo_Get(void);
eEXEC_CODE palDataMgr_NetworkStatusMessage(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT32 palDataMgr_Schedule_CommonScheduleEventIndex_Translate_GuiEventIndexValue(UINT8 CommonEventType, UINT8 CommonEventIndex);
eEXEC_CODE palDataMgr_Access_USB_Power(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Power_On_Counter(void);// HICC2_Bruce_0007
eEXEC_CODE palDataMgr_Power_Off_Counter(void);// HICC2_Bruce_0007
eEXEC_CODE palDataMgr_Access_Lens_Focus_DurationTime(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Lens_Zoom_DurationTime(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Lens_BackFocus_DurationTime(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Lens_DurationTime_Enable(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LSM_Boundary_Calibration(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LSM_Sensor_State_H(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_LSM_Sensor_State_V(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_TotalProjectorHours_Factory(eDATA_ACCESS_MODE eAccessMode, void *pValue);//A35G2_David_0010
eEXEC_CODE palDataMgr_Access_LD_Hours_Factory(eDATA_ACCESS_MODE eAccessMode, void *pValue);//A35G2_David_0010
eEXEC_CODE palDataMgr_Access_UI_Event_LogoReplace(eDATA_ACCESS_MODE eAccessMode, void *pValue);// HICC2_Bruce_0013
eEXEC_CODE palDataMgr_Access_ConstantPower(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Real_Black(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataMgr_Access_Dynamic_Black(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_getDimStatus(void);
eFUNC_CONTROL palDataMgr_Control_Security_Lock(void);
void palDataMgr_SecurityTimer_Poll(void);// HICC2_Bruce_0016
eEXEC_CODE palDataMgr_Access_System_Update_Flag(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataMgr_Set_OSDLockTemp(UINT8 ucVal);
eEXEC_CODE palDataMgr_Access_Energy_Saving(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT32 utilMath_Bcd2UINT32 (INT32 lNumber, void *pucBcd);//H30K_David_0038
eEXEC_CODE palDataMgr_AccessADV_WarpControlSet(UINT8 ucAdvWarpControl);

#ifdef ENABLE_GO_BACK_TO_LAST_MODE      //H30K_Tim_0006, add, ***
  void palDataMgr_Last_Display_Mode_Init(void);
  UINT8 palDataMgr_Last_Display_Mode_Get(void);
  UINT8 palDataMgr_Last_Display_Mode_Set(UINT8 ucSource, UINT8 ucLast_Display_Mode);
#endif //ENABLE_GO_BACK_TO_LAST_MODE    //H30K_Tim_0006, add, &&&
eEXEC_CODE palDataMgr_Access_OE_SN_1(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //H30K_Tim_0011, add
eEXEC_CODE palDataMgr_Access_OE_SN_2(eDATA_ACCESS_MODE eAccessMode, void *pValue);  //H30K_Tim_0011, add
UINT8 palDataMgr_sDDP_Gamma_Transfer(UINT8 ucGamma);
eEXEC_CODE palDataMgr_Access_StandbyMode(eDATA_ACCESS_MODE eAccessMode, void *pValue);
UINT8 palDataMgr_DefaultDisplayModeGet(void); //H30K_Doulas_0081


#ifdef __cplusplus
}
#endif


#endif  //_APPDATAMGR_H_

