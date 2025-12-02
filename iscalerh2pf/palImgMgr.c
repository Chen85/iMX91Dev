
#include "Common.h"
#include "palImgMgr.h"
#include "halScaler.h"
#include "utilDbgMsg.h"
#include "opdCtrlAPI.h"
#include "appPoll.h" //H30K_Doulas_0067

#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif

static bool m_bDump_Register_Flag = 0; //H30K_Doulas_0067

ePAL_IMG_EXEC_CODE palImgMgr_ExeResult(const char* FunctionName, eHAL_SCALER_EXEC_CODE eResult)
{
    ePAL_IMG_EXEC_CODE ePAL_Result;

    switch(eResult)
    {
        case eHAL_SCALER_EXEC_CODE_PASS:
            ePAL_Result = ePAL_IMG_EXEC_CODE_PASS;
            break;

        case eHAL_SCALER_EXEC_CODE_FAIL:
            ePAL_Result = ePAL_IMG_EXEC_CODE_FAIL;
            break;

        case eHAL_SCALER_EXEC_CODE_INIT_FAIL:
            ePAL_Result = ePAL_IMG_EXEC_CODE_INIT_FAIL;
            break;

        case eHAL_SCALER_EXEC_CODE_NOT_INIT:
            ePAL_Result = ePAL_IMG_EXEC_CODE_NOT_INIT;
            break;

        case eHAL_SCALER_EXEC_CH_OVER_RANGE:
            ePAL_Result = ePAL_IMG_EXEC_WINDOW_OVER_RANGE;
            break;


        case eHAL_SCALER_EXEC_CODE_DV_SCALER_INIT_FAIL:
            ePAL_Result = ePAL_IMG_EXEC_CODE_DV_SCALER_INIT_FAIL;
            break;

        case eHAL_SCALER_EXEC_CODE_DV_RX_INIT_FAIL:
            ePAL_Result = ePAL_IMG_EXEC_CODE_DV_RX_INIT_FAIL;
            break;

        case eHAL_SCALER_EXEC_CODE_DV_RX_UNSTABLE:
            ePAL_Result = ePAL_IMG_EXEC_CODE_DV_RX_UNSTABLE;
            break;

        case eHAL_SCALER_EXEC_CODE_NO_AUTO_PHASE:
            ePAL_Result = ePAL_IMG_EXEC_CODE_NO_AUTO_PHASE;
            break;

        case eHAL_SCALER_EXEC_CODE_AUTO_PHASE_GOING:
            ePAL_Result = ePAL_IMG_EXEC_CODE_AUTO_PHASE_GOING;
            break;

        case eHAL_SCALER_EXEC_CODE_HDR_CHANGE:
            ePAL_Result = ePAL_IMG_EXEC_CODE_HDR_CHANGE;
            break;

        default:
            LOG_MSG(db_HAL_SCALER, "undefined eHAL_SCALER_EXEC_CODE %d\n", eResult);
            ePAL_Result = ePAL_IMG_EXEC_CODE_NUMBER;
            break;

    }

    if(ePAL_Result != ePAL_IMG_EXEC_CODE_PASS)
    {
        LOG_MSG(db_ASSERT, "Execute %s Error (ID %d)\n", FunctionName, ePAL_Result);
    }

    return ePAL_Result;
}

#if 0
static inline BOOL palImgMgr_SemaphoreTake(const char *pcFunc)
{
    BOOL bResult = halScaler_SemaphoreTake(TRUE, pcFunc);

    return bResult;
}


static inline BOOL palImgMgr_SemaphoreGive(const char *pcFunc)
{
    BOOL bResult = halScaler_SemaphoreTake(FALSE, pcFunc);

    return bResult;
}
#endif


UINT32 palImgMgr_VersionGet(void)
{
#ifdef SCALER_FPGA_F34

    return halScaler_VersionGet();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT32 palImgMgr_GetScalerErrCode(const UINT8 ucCH)
{
    return halScaler_GetScalerErrCode(ucCH);
}

ePAL_IMG_EXEC_CODE palImgMgr_PowerStandby(uPALIMGMGR_INFO *psInfo)
{

#ifdef SCALER_FPGA_F34
    LOG_PALIMGMGR_PARAMETER_IS_NULL(psInfo);

    eHAL_SCALER_EXEC_CODE eRet = halScaler_PowerStandby(psInfo->sPowerStandby.ucCH);
#else
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PowerStandby();
#endif

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PowerNormal(UINT8 ucCH, const ePANEL_ID ePanelId)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PowerNormal(ucCH, ePanelId);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForDisplay(UINT8 ucCH, const UINT8 ucColorFmt)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureForDisplay(ucCH, ucColorFmt);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForDisplay_ChangePIPLayout(UINT8 ucCH, const UINT8 ucColorFmt)
{
#ifdef SCALER_FPGA_F34
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureForDisplay(ucCH,1);

    //halScaler_SclVopWindowOffSet(ucCH, FALSE);
    halScaler_ScreenOff(ucCH, FALSE);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
#else
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureForDisplay_ChangePIPLayout(ucCH, ucColorFmt);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_ConfigureColor(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureColor(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_ConfigureColorSpace(UINT8 ucCH, UINT8 ucSyncSub)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureColorSpace(ucCH, ucSyncSub);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_DisplaySFG(UINT8 ucCH, const UINT8 ucFillColor, const BOOL bEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DisplaySFG(ucCH, ucFillColor, bEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DisplaySplash(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DisplaySplash();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_SetOverlay(UINT8 ucCH, const BOOL bEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SetOverlay(ucCH, bEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

#if 0
ePAL_IMG_EXEC_CODE palImgMgr_SetOverlayCoef(UINT8 ucCH, const PUINT8 pucOverLay)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SetOverlayCoef(ucCH, pucOverLay);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_PortConfig(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PortConfig(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Init(UINT8 ucCH) //Init 4 channels in once, no matter 4K, 2K
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Init(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Brightness_Set(UINT8 ucCH,UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Brightness_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Contrast_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Contrast_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

INT8 palImgMgr_Dump_Register(uPALIMGMGR_INFO *psInfo)
{
#ifdef SCALER_FPGA_F34
    LOG_PALIMGMGR_PARAMETER_IS_NULL(psInfo);

    INT8 eRet = halScaler_Dump_Register(psInfo->sDump_Register.ucCH);
#else
    INT8 eRet = halScaler_Dump_Register();
#endif

    return eRet;
}

ePAL_IMG_EXEC_CODE palImgMgr_Bypasse_Mode(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Bypasse_Mode();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Test_Mode(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Test_Mode();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HUE_Set(UINT8 ucCH,UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HUE_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Saturation_Set(UINT8 ucCH,UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Saturation_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PIP_PBP_Layout_Set(UINT8 ucLayout)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PIP_PBP_Layout_Set(ucLayout);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PIP_PBP_Size_Set(UINT8 ucSize)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PIP_PBP_Size_Set(ucSize);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_PIP_PBP_Layout_Get(void)
{
    UINT8 ucLayout = halScaler_PIP_PBP_Layout_Get();

    return ucLayout;
}

UINT8 palImgMgr_PIP_PBP_Size_Get(void)
{
    UINT8 ucSize = halScaler_PIP_PBP_Size_Get();

    return ucSize;
}

void palImgMgr_Resync_Init(UINT8 eCH)
{
    halScaler_Resync_Init(eCH);
}

ePAL_IMG_EXEC_CODE palImgMgr_SYNC_Lock_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SYNC_Lock_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_CheckChannelSyncCount(UINT8 *ucIsSimilar)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_CheckChannelSyncCount(ucIsSimilar);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_Freeze_Set(UINT8 ucCH, BOOL bFreezeEn)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Freeze_Set(ucCH, bFreezeEn);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Freeze_Get(UINT8 ucCH, BOOL *bFreezeEn)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Freeze_Get(ucCH, bFreezeEn);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Panel_Set(UINT8 ucCH, ePANEL_ID ePanelId)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Panel_Set(ucCH, ePanelId);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT32 palImgMgr_Panel_Colck_Set(ePANEL_ID ePanelId)
{
#ifdef SCALER_C821_C789

    UINT32 eRet = halScaler_Panel_Colck_Set(ePanelId);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_AutoPhaseStart(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_AutoPhaseStart(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_AutoPhase(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_AutoPhase(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_SetInputSource(UINT8 ucCH, UINT8 ucInputSource)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SetInputSource(ucCH, ucInputSource);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_RedOffset_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RedOffset_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_GreenOffset_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_GreenOffset_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_BlueOffset_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_BlueOffset_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_RedGain_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RedGain_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_GreenGain_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_GreenGain_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_BlueGain_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_BlueGain_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_RedOffset_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RedOffset_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_GreenOffset_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_GreenOffset_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_BlueOffset_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_BlueOffset_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_RedGain_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RedGain_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_GreenGain_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_GreenGain_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_BlueGain_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_BlueGain_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Brightness_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Brightness_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Contrast_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Contrast_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HUE_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HUE_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Saturation_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Saturation_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Init_EEPROM_Setting(sUSER_SYSTEM_SETTING m_UserSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Init_EEPROM_Setting(m_UserSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_SizePresets_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SizePresets_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_SizePresets_SetOnlyValue(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SizePresets_SetOnlyValue(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Overscan_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Overscan_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PixelTrack_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PixelTrack_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PixelPhase_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PixelPhase_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PixelPhase_Action(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PixelPhase_Action();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_OSD_PixelPhase_Get(void)
{
    UINT8 ucPhase = halScaler_OSD_PixelPhase_Get();

    return ucPhase;
}

ePAL_IMG_EXEC_CODE palImgMgr_HorzPosition_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HorzPosition_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_VertPosition_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_VertPosition_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DigitalHorzZoom_Set(UINT16 uiSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DigitalHorzZoom_Set(uiSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DigitalVertZoom_Set(UINT16 uiSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DigitalVertZoom_Set(uiSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DigitalHorzShift_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DigitalHorzShift_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DigitalVertShift_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DigitalVertShift_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_AutoImage_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_AutoImage_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_InputSource_Get(UINT8 ucCH)
{
    UINT8 ucInputSource = halScaler_InputSource_Get(ucCH);

    return ucInputSource;
}

UINT8 palImgMgr_SizePresets_Get(UINT8 ucCH)
{
    UINT8 ucAspectRatio = halScaler_SizePresets_Get(ucCH);

    return ucAspectRatio;
}

UINT8 palImgMgr_PictureSettings_Get(UINT8 ucCH)
{
    UINT8 ucPresetMode = halScaler_PictureSettings_Get(ucCH);

    return ucPresetMode;
}

UINT8 palImgMgr_PictureSettings_PreUser_Get(UINT8 ucCH)
{
    UINT8 ucPreUserMode = halScaler_PictureSettings_PreUser_Get(ucCH);

    return ucPreUserMode;
}

UINT8 palImgMgr_SignalType_Get(UINT8 ucCH)
{
    UINT8 ucSignalType = halScaler_SignalType_Get(ucCH);

    return ucSignalType;
}

ePAL_IMG_EXEC_CODE palImgMgr_ColorSpace_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ColorSpace_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_ColorSpace_Get(UINT8 ucCH)
{
    UINT8 ucColorSpace = halScaler_ColorSpace_Get(ucCH);

    return ucColorSpace;
}

UINT8 palImgMgr_ColorSpace_ScalerValueGet(UINT8 ucCH)
{
    UINT8 ucColorSpace = halScaler_ColorSpace_ScalerValueGet(ucCH);

    return ucColorSpace;
}

ePAL_IMG_EXEC_CODE palImgMgr_Detail_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Detail_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_3DEnable_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = (eHAL_SCALER_EXEC_CODE)halScaler_3DEnable_Set(ucSetting);

    return eRet;
}

UINT8 palImgMgr_3DEnable_Get(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 uc3DEnable = halScaler_3DEnable_Get();

    return uc3DEnable;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_3DMode_Set(UINT8 ucSetting)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = (eHAL_SCALER_EXEC_CODE)halScaler_3DMode_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    //LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    //return 0;
    eHAL_SCALER_EXEC_CODE eRet = (eHAL_SCALER_EXEC_CODE)halScaler_3DMode_Set(ucSetting); //H30K_Doulas_0001

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#endif
}

UINT8 palImgMgr_3DMode_Get(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 uc3DMode = halScaler_3DMode_Get();

    return uc3DMode;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

INT8 palImgMgr_3DInvert_Set(UINT8 ucSetting)
{
#ifdef SCALER_FPGA_F34

    INT8 eRet = halScaler_3DInvert_Set(ucSetting);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

INT8 palImgMgr_3DSyncOut_Set(UINT8 ucSetting)
{
#ifdef SCALER_FPGA_F34

    INT8 eRet = halScaler_3DSyncOut_Set(ucSetting);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_3DSyncOut_Get(UINT8 eCH)
{
#ifdef SCALER_FPGA_F34

    UINT8 uc3SyncOut = halScaler_3DSyncOut_Get(eCH);

    return uc3SyncOut;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

INT8 palImgMgr_3DSyncIn_Set(UINT8 ucSetting)
{
#ifdef SCALER_FPGA_F34

    INT8 eRet = halScaler_3DSyncIn_Set(ucSetting);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_3DSyncIn_Get(UINT8 eCH)
{
#ifdef SCALER_FPGA_F34

    UINT8 uc3DSyncIn = halScaler_3DSyncIn_Get(eCH);

    return uc3DSyncIn;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_IS_3D_Enable(void)
{
#ifdef SCALER_C821_C789

    UINT8 uc3D_Enable = halScaler_IS_3D_Enable();

    return uc3D_Enable;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif

}

UINT8 palImgMgr_3D_Type(void)
{
#ifdef SCALER_C821_C789

    UINT8 uc3D_Type = halScaler_3D_Type();

    return uc3D_Type;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_FrameDelay_Set(UINT16 uiSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrameDelay_Set(uiSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_3DSyncDelay_Set(UINT16 uiSetting)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_3DSyncDelay_Set(uiSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

ePAL_IMG_EXEC_CODE palImgMgr_SkinColor_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SkinColor_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_TemporalNoiseReduction_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_TemporalNoiseReduction_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_MPEGNoiseReduction_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_MPEGNoiseReduction_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DetectFilm_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DetectFilm_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_RGB_Gain_Offset_Set(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RGB_Gain_Offset_Set();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PictureSettings_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PictureSettings_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

void palImgMgr_PreUserMode_Set(UINT8 ucSetting)
{
    halScaler_PreUserMode_Set(ucSetting);
}

ePAL_IMG_EXEC_CODE palImgMgr_ContrastEnhancement_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ContrastEnhancement_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

INT8 palImgMgr_HDREnable_Set(UINT8 ucSetting)
{
#ifdef SCALER_FPGA_F34

    INT8 eRet = halScaler_HDREnable_Set(ucSetting);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

//H2PF_Simon_0193 Start
ePAL_IMG_EXEC_CODE palImgMgr_HDR_Reset(UINT8 ucSetting)
{
#ifdef SCALER_C341

    eHAL_SCALER_EXEC_CODE eRet = halScaler_HDR_Reset();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_PASS;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_HDR10_Process(UINT16 uiHDRContentMaxNit)
{
#ifdef SCALER_C341

    eHAL_SCALER_EXEC_CODE eRet = halScaler_HDR10_Process(uiHDRContentMaxNit);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_PASS;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_HLG_Process(UINT16 uiMonitorMaxNit)
{
#ifdef SCALER_C341

    eHAL_SCALER_EXEC_CODE eRet = halScaler_HLG_Process(uiMonitorMaxNit);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_PASS;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_HDR_Disable(void)
{
#ifdef SCALER_C341

    eHAL_SCALER_EXEC_CODE eRet = halScaler_HDR_Disable();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_PASS;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_SDREnhance_Process(void)
{
#ifdef SCALER_C341

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SDREnhance_Process();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_PASS;

#endif
}
//H2PF_Simon_0193 End


#if 0
INT8 palImgMgr_MEMC_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_MEMC_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_ImageFreeze_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ImageFreeze_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Gamma_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Gamma_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_WhitePeaking_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_WhitePeaking_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ColorTemperature_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ColorTemperature_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

INT8 palImgMgr_EdgeEnhancement_Set(UINT8 ucSetting)
{
    INT8 eRet = halScaler_EdgeEnhancement_Set(ucSetting);

    return eRet;
}

INT8 palImgMgr_ColorWheelSpeed_Get(UINT8 ucCH)
{
#ifdef SCALER_C821_C789

    INT8 cCWSpeed = halScaler_ColorWheelSpeed_Get(ucCH);

    return cCWSpeed;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_ColorWheelSpeed_Set(UINT8 ucCH, UINT8 ucSetting)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_ColorWheelSpeed_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_ColorEnhancement_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ColorEnhancement_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ColorEnhancement_Get(UINT8 ucCH, UINT8 *ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ColorEnhancement_Get(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HSGEnable_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HSGEnable_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HSGEnable_Get(UINT8 *ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HSGEnable_Get(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HSG_Set(UINT8 ucCH, sHSG_SETTING ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HSG_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HSG_All_Set(UINT8 ucInputSource, UINT8 ucPresetMode, sHSG_SETTING ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HSG_All_Set(ucInputSource, ucPresetMode, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_HSG_Get(sHSG_SETTING *m_sHSG_Setting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_HSG_Get(m_sHSG_Setting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PIP_PBP_Enable_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PIP_PBP_Enable_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_PIP_PBP_Enable_Get(void)
{
    UINT8 ucPIPEnable = halScaler_PIP_PBP_Enable_Get();

    return ucPIPEnable;
}

ePAL_IMG_EXEC_CODE palImgMgr_IintChannelSetting(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_IintChannelSetting(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT16 palImgMgr_DigitalHorzZoom_Get(void)
{
    UINT16 uiSetting = halScaler_DigitalHorzZoom_Get();

    return uiSetting;
}

UINT16 palImgMgr_DigitalVertZoom_Get(void)
{
    UINT16 uiSetting = halScaler_DigitalVertZoom_Get();

    return uiSetting;
}

UINT8 palImgMgr_DigitalHorzShift_Get(void)
{
    UINT8 ucSetting = halScaler_DigitalHorzShift_Get();

    return ucSetting;
}

UINT8 palImgMgr_DigitalVertShift_Get(void)
{
    UINT8 ucSetting = halScaler_DigitalVertShift_Get();

    return ucSetting;
}

#if 0
ePAL_IMG_EXEC_CODE palImgMgr_Detail_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Detail_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_Detail_Value_Get(UINT8 ucCH)
{
    UINT8 ucValue = halScaler_Detail_Value_Get(ucCH);

    return ucValue;
}

ePAL_IMG_EXEC_CODE palImgMgr_SkinColor_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SkinColor_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_SkinColor_Value_Get(UINT8 ucCH)
{
    UINT8 ucValue = halScaler_SkinColor_Value_Get(ucCH);

    return ucValue;
}

INT16 palImgMgr_Brightness_Value_Get(UINT8 ucCH)
{
    INT16 iValue = halScaler_Brightness_Value_Get(ucCH);

    return iValue;
}

DOUBLE palImgMgr_Contrast_Value_Get(UINT8 ucCH)
{
    DOUBLE dValue = halScaler_Contrast_Value_Get(ucCH);

    return dValue;
}

INT16 palImgMgr_HUE_Value_Get(UINT8 ucCH)
{
    INT16 iValue = halScaler_HUE_Value_Get(ucCH);

    return iValue;
}

DOUBLE palImgMgr_Saturation_Value_Get(UINT8 ucCH)
{
    DOUBLE dValue = halScaler_Saturation_Value_Get(ucCH);

    return dValue;
}

INT8 palImgMgr_RedOffset_Value_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RedOffset_Value_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

INT8 palImgMgr_GreenOffset_Value_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_GreenOffset_Value_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

INT8 palImgMgr_BlueOffset_Value_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_BlueOffset_Value_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_RedGain_Value_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_RedGain_Value_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_GreenGain_Value_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_GreenGain_Value_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_BlueGain_Value_Get(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_BlueGain_Value_Get(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Temporal_NR_Value_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Temporal_NR_Value_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_MPEG_NR_Value_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_MPEG_NR_Value_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_DetectFilm_Value_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DetectFilm_Value_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_PictureSettings_Value_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_PictureSettings_Value_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_MeasureInput(uPALIMGMGR_INFO *psInfo)
{
    LOG_PALIMGMGR_PARAMETER_IS_NULL(psInfo);

#ifdef SCALER_FPGA_F34
    eHAL_SCALER_EXEC_CODE eRet = halScaler_MeasureInput(psInfo->sMeasureInput.ucCH,
                                                        psInfo->sMeasureInput.ucInput);
#else
    eHAL_SCALER_EXEC_CODE eRet = halScaler_MeasureInput(psInfo->sMeasureInput.ucCH);
#endif

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_MeasureCheck(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_MeasureCheck(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_OSD_TestPattern(const UINT8 ucTestPattern)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_OSD_TestPattern(ucTestPattern);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

#if 0
ePAL_IMG_EXEC_CODE palImgMgr_OSD_TestPattern_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_OSD_TestPattern_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Service_TestPattern(const UINT8 ucTestPattern)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Service_TestPattern(ucTestPattern);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Service_TestPattern_Set(UINT8 ucCH, UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Service_TestPattern_Set(ucCH, ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

UINT8 palImgMgr_PictureSettings_Get_NotReturnUser(UINT8 ucCH)
{
    UINT8 ucValue = halScaler_PictureSettings_Get_NotReturnUser(ucCH);

    return ucValue;
}

ePAL_IMG_EXEC_CODE palImgMgr_InputPixelClock_Get(UINT8 ucCH, UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputPixelClock_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputResolution_Get(UINT8 ucCH, UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputResolution_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputHorzRefresh_Get(UINT8 ucCH, UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputHorzRefresh_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputVertRefresh_Get(UINT8 ucCH, UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputVertRefresh_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputVertRefresh2_Get(UINT8 ucCH, UINT16 *uiValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputVertRefresh2_Get(ucCH, uiValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputVertRefresh2_Get_FromSyncCount(UINT8 ucCH, UINT16 *uiValue)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputVertRefresh2_Get_FromSyncCount(ucCH, uiValue);
    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

ePAL_IMG_EXEC_CODE palImgMgr_InputSignalFormat_Get(UINT8 ucCH, UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputSignalFormat_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputAspectRatio_Get(UINT8 ucCH, UINT8 *ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputAspectRatio_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_FrontEndColorSpace_Set(UINT8 ucCH, UINT8 ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrontEndColorSpace_Set(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_FrontEndColorSpaceInfo_Get(UINT8 ucCH, UINT8* ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrontEndColorSpaceInfo_Get(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_FrontEndScanMode_Set(UINT8 ucCH, UINT8 ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrontEndScanMode_Set(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_FrontEndScanMode_Clear(UINT8 ucCH)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrontEndScanMode_Clear(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

#if 0
BOOL palImgMgr_ForcedSyncResetType_Get(void)
{
    return halScaler_ForcedSyncResetType_Get();
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_ForcedSyncResetDisable_Set(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ForcedSyncResetDisable_Set();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

BOOL palImgMgr_IsInterlaced_Get(UINT8 ucCH)
{
    return halScaler_IsInterlaced_Get(ucCH);
}

ePAL_IMG_EXEC_CODE palImgMgr_FrontEndVideoYUV_Set(UINT8 ucCH, UINT8 ucValue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrontEndVideoYUV_Set(ucCH, ucValue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Horz_Start_Position_Get(UINT8 ucCH, UINT16* uiVal)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Horz_Start_Position_Get(ucCH, uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_Vert_Start_Position_Get(UINT8 ucCH, UINT16* uiVal)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Vert_Start_Position_Get(ucCH, uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_VGA_H_Total_Get(UINT8 ucCH, UINT16* uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_VGA_H_Total_Get(ucCH, uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_VGA_V_Total_Get(UINT8 ucCH, UINT16* uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_VGA_V_Total_Get(ucCH, uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_VGA_SYNC_TYPE_Set(UINT8 ucCH, UINT8 ucVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_VGA_SYNC_TYPE_Set(ucCH, ucVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

#if 0
UINT8 palImgMgr_VGA_SYNC_TYPE_Get(UINT8 ucCH)
{
    UINT8 ucValue = halScaler_VGA_SYNC_TYPE_Get(ucCH);

    return ucValue;
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_SyncThreshold_Set(UINT8 ucSetting)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SyncThreshold_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_SyncThreshold_Get(void)
{
    return halScaler_SyncThreshold_Get();
}

UINT32 palImgMgr_uiHFreq_Get(UINT8 ucCH)
{
    return halScaler_uiHFreq_Get(ucCH);
}

UINT8 palImgMgr_HDR_Get(UINT8 ucCH)
{
    UINT8 ucIsHDR = 0;

#ifdef SCALER_FPGA_F34

    ucIsHDR = halScaler_HDR_Get(ucCH);

#elif defined(SCALER_C821_C789) || defined(SCALER_C341) //HICC2_AC_0011

    //H2 wait review
    //H2 Simon : m_FrontEndVideoFormat need to move to palImgMgr or palFrontendMgr
    #include "appDataPath.h"
    ucIsHDR = palDataPath_HDR_Info_Get();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

    return ucIsHDR;
}



ePAL_IMG_EXEC_CODE palImgMgr_Position_Run(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Position_Run(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_HorzPositionWorkValueGet(UINT8 ucCH)
{
    return halScaler_HorzPositionWorkValueGet(ucCH);
}

UINT8 palImgMgr_VertPositionWorkValueGet(UINT8 ucCH)
{
    return halScaler_VertPositionWorkValueGet(ucCH);
}

ePAL_IMG_EXEC_CODE palImgMgr_AutoPsitionStart(UINT8 ucCH, BOOL bAutoEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_AutoPsitionStart(ucCH, bAutoEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Config_NoSignalOutput(uPALIMGMGR_INFO *psInfo)
{
    LOG_PALIMGMGR_PARAMETER_IS_NULL(psInfo);

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Config_NoSignalOutput(psInfo->sConfig_NoSignalOutput.eWindow,
                                                                 psInfo->sConfig_NoSignalOutput.ucDisplayOutput);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT16 palImgMgr_Input_H_Active_Get(UINT8 ucCH)
{
    return halScaler_Input_H_Active_Get(ucCH);
}

UINT16 palImgMgr_Input_V_Active_Get(UINT8 ucCH)
{
    return halScaler_Input_V_Active_Get(ucCH);
}

#if 0
UINT16 palImgMgr_Input_H_Total_Get(UINT8 ucCH)
{
    return halScaler_Input_H_Total_Get(ucCH);
}

UINT16 palImgMgr_Input_V_Total_Get(UINT8 ucCH)
{
    return halScaler_Input_V_Total_Get(ucCH);
}

UINT16 palImgMgr_Input_H_Start_Get(UINT8 ucCH)
{
    return halScaler_Input_H_Start_Get(ucCH);
}

UINT16 palImgMgr_Input_V_Start_Get(UINT8 ucCH)
{
    return halScaler_Input_V_Start_Get(ucCH);
}

ePAL_IMG_EXEC_CODE palImgMgr_DDR_InputDataAreaTotalGet(UINT8 eCH, RECT DataArea, UINT32 *ulTotalRed, UINT32 *ulTotalGreen, UINT32 *ulTotalBlue)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_DDR_InputDataAreaTotalGet(eCH, DataArea, ulTotalRed, ulTotalGreen, ulTotalBlue);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_InputADC_Cali_CalibrationEnableSet(UINT8 eCH, UINT8 ucADC_CAl_Enable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputADC_Cali_CalibrationEnableSet(eCH, ucADC_CAl_Enable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

#endif


UINT8 palImgMgr_Input_3D_Format_Get(void)
{
    return halScaler_Input_3D_Format_Get();
}

void palImgMgr_Input_3D_Format_Set(UINT8 ucVal)
{
    halScaler_Input_3D_Format_Set(ucVal);
}

#if 0
ePAL_IMG_EXEC_CODE palImgMgr_Input_2D_SyncOut_Enable(UINT8 ucVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Input_2D_SyncOut_Enable(ucVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_3D_InputPort_Set(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_3D_InputPort_Set();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

UINT8 palImgMgr_Input_3D_Format_Config_Get(void)
{
    return halScaler_Input_3D_Format_Config_Get() ;
}

ePAL_IMG_EXEC_CODE palImgMgr_InputHorzPeriod_Get(UINT8 ucCH, UINT32* udVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputHorzPeriod_Get(ucCH, udVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_VGA_ModeTable_Get(UINT8 ucCH, UINT8* ucVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_VGA_ModeTable_Get(ucCH, ucVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_VGA_ModeTableNumber_Get(UINT8 ucCH, UINT16* uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_VGA_ModeTableNumber_Get(ucCH, uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttHorzStart_Get(UINT16* uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ModeAdjusmenttHorzStart_Get(uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttVertStart_Get(UINT16* uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ModeAdjusmenttVertStart_Get(uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttHorzStart_Set(UINT16 uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ModeAdjusmenttHorzStart_Set(uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttVertStart_Set(UINT16 uiVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ModeAdjusmenttVertStart_Set(uiVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttEnableSetting(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ModeAdjusmenttEnableSetting();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ModeAdjusmenttDisableSetting(void)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ModeAdjusmenttDisableSetting() ;

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Init_Mode_Adjustment_EEPROM_Setting(sTIMING_TABLE sTiming_Table)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Init_Mode_Adjustment_EEPROM_Setting(sTiming_Table);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_Input3D_Timing_Get(UINT8 ucCH, UINT8* ucVal)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_Input3D_Timing_Get(ucCH, ucVal);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForPosition(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureForPosition(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_ConfigureForScaler(UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ConfigureForScaler(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

ePAL_IMG_EXEC_CODE palImgMgr_EdgeMask_Set(UINT8 ucSetting)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_EdgeMask_Set(ucSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

void palImgMgr_ProAV_ColorSpaceConversion(UINT8 ucCH, UINT8 ucColorSpace, UINT8 ucPixelMode, UINT8 ucRGBMode)
{
#ifdef SCALER_FPGA_F34

    halScaler_ProAV_ColorSpaceConversion(ucCH, ucColorSpace, ucPixelMode, ucRGBMode);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

eRESULT palImgMgr_ProAV_ColorSpaceSetting(UINT8 ucCH, UINT8 ucColorSpace)
{
#ifdef SCALER_FPGA_F34

    eRESULT eRet = halScaler_ProAV_ColorSpaceSetting(ucCH, ucColorSpace);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return rcERROR;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_Monitor(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Monitor(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_AVIInfoFrame(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_AVIInfoFrame(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_SourceLock(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SourceLock(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

#if 0
ePAL_IMG_EXEC_CODE palImgMgr_SPI_SelectSet(UINT8 ucSelect)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_SPI_SelectSet(ucSelect);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_InputSelectSet(UINT8 ucCH, UINT8 ucSelect, BOOL BlackSource)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputSelectSet(ucCH, ucSelect, BlackSource);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_BackupRxPortSet(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_BackupRxPortSet(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_FrameSyncModeSet(UINT8 ucCH, UINT8 eMode)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrameSyncModeSet(ucCH, eMode);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_FrameSyncModeGet(UINT8* cMode)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_FrameSyncModeGet(cMode);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_InputFrameRateSetting(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputFrameRateSetting(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

#if 0
ePAL_IMG_EXEC_CODE palImgMgr_HDRLevelSetting(UINT8 ucLevel)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_HDRLevelSetting(ucLevel);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_HDRLevelSet(UINT8 ucLevel)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_HDRLevelSet(ucLevel);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

#endif


void palImgMgr_CurrentSourceRxPortSet(UINT8 ucCH, UINT8 ucRxPort)
{
#ifdef SCALER_FPGA_F34

    halScaler_CurrentSourceRxPortSet(ucCH, ucRxPort);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_CurrentSourceRxPortGet(UINT8 ucCH, UINT8 ucSelect)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_CurrentSourceRxPortGet(ucCH, ucSelect);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_RxPortClear(UINT8 ucRxPort)
{
#ifdef SCALER_FPGA_F34

    halScaler_RxPortClear(ucRxPort);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_RxPortCheck(UINT8 ucRxPort, UINT8 ucChangeSourceReset)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_RxPortCheck(ucRxPort, ucChangeSourceReset);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_RxPortCheckPolling(void)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_RxPortCheckPolling();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_RxPortIsReady(UINT8 ucCH, UINT8 ucSelect)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_RxPortIsReady(ucCH, ucSelect);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}


#if 0
ePAL_IMG_EXEC_CODE palImgMgr_RxPortAVI_InfoCopy(UINT8 ucRxPort, UINT8 *pcData)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_RxPortAVI_InfoCopy(ucRxPort, pcData);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_RxPortAVI_Info(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_RxPortAVI_Info(ucCH, ucSelect, pcData);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_RxPortVSI_Info(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_RxPortVSI_Info(ucCH, ucSelect, pcData);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

#endif

void palImgMgr_RxPortDetTiming(UINT8 ucRxPort, UINT8 *psDetTiming)
{
#ifdef SCALER_FPGA_F34

    halScaler_RxPortDetTiming(ucRxPort, psDetTiming);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_InputCompare(UINT8 ucCH, UINT8 ucSelect)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_InputCompare(ucCH, ucSelect);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_SclDatapathSet(UINT8 dataPath)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SclDatapathSet(dataPath);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_SclDatapathCompare(UINT8 dataPath)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SclDatapathCompare(dataPath);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_SclVopWindowOffSet(UINT8 ucCH, bool bOff)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SclVopWindowOffSet(ucCH, bOff);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

#if 0
bool palImgMgr_SclVopWindowOffGet(UINT8 ucCH)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SclVopWindowOffGet(ucCH);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}
#endif

UINT8 palImgMgr_3DModeGet(UINT8 ucRxPort)
{
#ifdef SCALER_FPGA_F34

    UINT8 uc3DMode = halScaler_3DModeGet(ucRxPort);

    return uc3DMode;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}


#if 0
UINT8 palImgMgr_DualPipe3dModeSet(bool bMode)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_DualPipe3dModeSet(bMode);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}
#endif

UINT8 palImgMgr_InputPortIsReady(UINT8 ucCH, UINT8 ucSelect)
{
#ifdef SCALER_FPGA_F34

    UINT8 ucIsReady = halScaler_InputPortIsReady(ucCH, ucSelect);

    return ucIsReady;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}


#if 0
void palImgMgr_InputPortResolution(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    halScaler_InputPortResolution(ucCH, ucSelect, pcData);
}

void palImgMgr_InputPortHorzRate(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    halScaler_InputPortHorzRate(ucCH, ucSelect, pcData);
}

void palImgMgr_RxPortColorSpace(UINT8 ucRxPort, UINT8 *pcData)
{
    halScaler_RxPortColorSpace(ucRxPort, pcData);
}

void palImgMgr_InputPortColorSpace(UINT8 ucCH, UINT8 ucSelect, UINT8 *pcData)
{
    halScaler_InputPortColorSpace(ucCH, ucSelect, pcData);
}
#endif

UINT8 palImgMgr_LogoCaptureSet(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_LogoCaptureSet();

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_LogoDisplayEn(bool bEnable)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_LogoDisplayEn(bEnable);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_ScreenOff(UINT8 ucCH, BOOL bEnable)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_ScreenOff(ucCH, bEnable);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_Dram2Flash(UINT32 dramAddr, UINT32 flashAddr, UINT32 len)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_Dram2Flash(dramAddr, flashAddr, len);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_Flash2Dram(UINT32 dramAddr, UINT32 flashAddr, UINT32 len)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_Flash2Dram(dramAddr, flashAddr, len);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_BackupInputCheck(UINT8 ucMain, UINT8 ucSub)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_BackupInputCheck(ucMain, ucSub);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_PASS;

#endif
}

void palImgMgr_RxPortInfoPrint(void)
{
#ifdef SCALER_FPGA_F34

    halScaler_RxPortInfoPrint();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT16 palImgMgr_FpgaTempGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiFpgaTemp = halScaler_FpgaTempGet();

    return uiFpgaTemp;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_FpgaTempADCRst(void)
{
#ifdef SCALER_FPGA_F34

    halScaler_FpgaTempADCRst();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_BackupPrimaryInput_Set(UINT8 ucInput)
{
#ifdef SCALER_FPGA_F34

    halScaler_BackupPrimaryInput_Set(ucInput);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_BackupPrimaryInput_Get(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 ucValue = halScaler_BackupPrimaryInput_Get();

    return ucValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_BackupSecondaryInput_Set(UINT8 ucInput)
{
#ifdef SCALER_FPGA_F34

    halScaler_BackupSecondaryInput_Set(ucInput);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_BackupSecondaryInput_Get(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 ucValue = halScaler_BackupSecondaryInput_Get();

    return ucValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_CurrentBackupSourceGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 ucValue = halScaler_CurrentBackupSourceGet();

    return ucValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_LogoCapturePanel(UINT16 uiPanel)
{
#ifdef SCALER_FPGA_F34

    UINT8 eRet = halScaler_LogoCapturePanel(uiPanel);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

BOOL palImgMgr_LowLatencyEnable(void)
{
#ifdef SCALER_FPGA_F34

    BOOL Enable = halScaler_LowLatencyEnable();

    return Enable;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}


ePAL_IMG_EXEC_CODE palImgMgr_AntiSmearSet(UINT8 cMode, bool bEnable)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_AntiSmearSet(cMode, bEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_MEMCBypassSet(UINT8 ucEnable)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_MEMCBypassSet(ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_MEMCColorSet(UINT8 ucColor)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_MEMCColorSet(ucColor);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_MEMCMotionSet(UINT8 ucMotion)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_MEMCMotionSet(ucMotion);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_MEMCDemoSet(UINT8 ucDemo)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_MEMCDemoSet(ucDemo);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

void palImgMgr_BackupSwitchSet(UINT8 ucValue)
{
#ifdef SCALER_FPGA_F34

    halScaler_BackupSwitchSet(ucValue);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_BackupSwitchGet(void)
{
#ifdef SCALER_FPGA_F34
    return halScaler_BackupSwitchGet();
#else
    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_BackupSizePresetSetting(void)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_BackupSizePresetSetting();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

BOOL palImgMgr_AutoExt_3D_SyncSetting(void)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_AutoExt_3D_SyncSetting();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_ConfigurePanelSize(uint16 uiPanelID)
{
#ifdef SCALER_FPGA_F34

    halScaler_ConfigurePanelSize(uiPanelID);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

BOOL palImgMgr_Rx_Compare(UINT8 ucCH, UINT8 ucSelect)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Rx_Compare(ucCH, ucSelect);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

#if 0
BOOL palImgMgr_4K3D_Set(UINT8 ucCH, UINT8 ucEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_4K3D_Set(ucCH, ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

BOOL palImgMgr_4K3DLRLR_Set(UINT8 ucCH, UINT8 ucEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_4K3DLRLR_Set(ucCH, ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

BOOL palImgMgr_4K3DLLRR_Set(UINT8 ucCH, UINT8 ucEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_4K3DLLRR_Set(ucCH, ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

BOOL palImgMgr_4K3DLRLR_DualPipe_Set(UINT8 ucCH, UINT8 ucEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_4K3DLRLR_DualPipe_Set(ucCH, ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

BOOL palImgMgr_4K3DLLRR_DualPipe_Set(UINT8 ucCH, UINT8 ucEnable)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_4K3DLLRR_DualPipe_Set(ucCH, ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}
#endif

ePAL_IMG_EXEC_CODE palImgMgr_PixelShiftEnable_Set(UINT8 ucEnable)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_PixelShiftEnable_Set(ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_PixelShiftCalPattern_Set(UINT8 ucEnable)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_PixelShiftCalPattern_Set(ucEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_Default4K3DSEnable_Set(UINT8 ucMode)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Default4K3DSEnable_Set(ucMode);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_ColorSetting_Get(eHAL_COLOR_SETTING  ucNode, UINT8 ucCH)
{
    eHAL_SCALER_EXEC_CODE eRet = halScaler_ColorSetting_Get(ucNode, ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);
}

int palImgMgr_FpgaDataRGBLevelGet(uint16 *RGB_LEVEL_Max, uint16 *RGB_LEVEL_Min)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_FpgaDataRGBLevelGet(RGB_LEVEL_Max, RGB_LEVEL_Min);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_RxPortReset(void)
{
#ifdef SCALER_FPGA_F34

    halScaler_RxPortReset();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_DisableRGB(ePAL_SCALER_RGB_ITEM RGB_Item)
{
#ifdef SCALER_FPGA_F34

    halScaler_DisableRGB((eHAL_SCALER_RGB_ITEM)RGB_Item);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_EnableRGB(void)
{
#ifdef SCALER_FPGA_F34

    halScaler_EnableRGB();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_RGBEnableItemSet(ePAL_SCALER_RGB_ITEM RGB_Item)
{
#ifdef SCALER_FPGA_F34

    halScaler_RGBEnableItemSet((eHAL_SCALER_RGB_ITEM)RGB_Item);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_RGBMaskSet(UINT8 ucMask)
{
#ifdef SCALER_FPGA_F34

    halScaler_RGBMaskSet(ucMask);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_4K3D_DualPipeOrientationSet(UINT8 ucFRC_Bypass,BOOL bRear,BOOL bCeiling)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_4K3D_DualPipeOrientationSet(ucFRC_Bypass, bRear, bCeiling);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

void palImgMgr_RGBSwitchSet(bool bSwitch)
{
#ifdef SCALER_FPGA_F34

    halScaler_RGBSwitchSet(bSwitch);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

BOOL palImgMgr_RGBSwitchGet(void)
{
#ifdef SCALER_FPGA_F34

    BOOL bValue = halScaler_RGBSwitchGet();
    return bValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

eRESULT palImgMgr_Version_Get(uint32 *ulVersiong)
{
#ifdef SCALER_FPGA_F34

    eHAL_SCALER_EXEC_CODE eRet = halScaler_Version_Get(ulVersiong);

    return eRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return rcERROR;

#endif
}

void palImgMgr_PixSftModSet(bool bSwitch)
{
#ifdef SCALER_FPGA_F34

    halScaler_PixSftModSet(bSwitch);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

BOOL palImgMgr_PixModIsYUV420(UINT8 ucCH, UINT8 ucSelect)
{
#ifdef SCALER_FPGA_F34

    BOOL bValue = halScaler_PixModIsYUV420(ucCH, ucSelect);
    return bValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

BOOL palImgMgr_IsVsync120or100Hz_Get(UINT8 ucCH)
{
#ifdef SCALER_C821_C789

    BOOL Ret = halScaler_IsVsync120or100Hz_Get(ucCH);
    return Ret;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

BOOL palImgMgr_IsAspectRation16_9_Get(void)
{
#ifdef SCALER_C821_C789

    BOOL Ret = halScaler_IsAspectRation16_9_Get();

    return Ret;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

BOOL palImgMgr_IsAspectRation16_10_Get(void)
{
#ifdef SCALER_C821_C789

    BOOL Ret = halScaler_IsAspectRation16_10_Get();

    return Ret;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_SourceOutputOff(UINT8 ucCH)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_SourceOutputOff(ucCH);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_OutputVsyncFreqGetForTwistOn(void)
{
#ifdef SCALER_C821_C789

    UINT8 ucRet = halScaler_OutputVsyncFreqGetForTwistOn();

    return ucRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_Low_Latency_Set(eLOW_LATENCY_MODE eLLMode)     //H2PF_Simon_0150
{
#ifdef SCALER_ICHIPS

    eHAL_SCALER_EXEC_CODE eRet = (eHAL_SCALER_EXEC_CODE)halScaler_Low_Latency_Set(eLLMode);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

eLOW_LATENCY_MODE palImgMgr_Low_Latency_Get(void)   //H2PF_Simon_0187
{
#ifdef SCALER_ICHIPS

	return halScaler_Low_Latency_Get();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT32 palImgMgr_Reg0_Read_Get(void)
{
#ifdef SCALER_C821_C789
    UINT32 ulErrorCount = halScaler_Reg0_Read_Get();
    return ulErrorCount;
#else

    UINT32 ulErrorCount = halScaler_Reg0_Read_Get();
    return ulErrorCount;
#endif
}

UINT32 palImgMgr_WarpReg0_Read_Get(void)
{
#ifdef SCALER_C821_C789

    UINT32 ulErrorCount = halC789Ctrl_Reg0_Read_Get();

    return ulErrorCount;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT8 palImgMgr_OPD_Test(UINT8 ucData)
{
#ifdef SCALER_C821_C789

    UINT8 ucRet = halScaler_OPD_Test(ucData);

    return ucRet;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_CaptureImage(void)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_CaptureImage();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_CaptureImage_Show(BOOL bEnable)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_CaptureImage_Show(bEnable);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_CaptureImage_Del(BOOL bSourceLock)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_CaptureImage_Del(bSourceLock);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

ePAL_IMG_EXEC_CODE palImgMgr_ScreenSaveToImage(void)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_ScreenSaveToImage();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

ePAL_IMG_EXEC_CODE palImgMgr_Auto_Fill_Screen_Init(void)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_C821_Auto_Fill_Screen_Init();

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

ePAL_IMG_EXEC_CODE palImgMgr_Auto_Fill_Screen_Setting(BOOL bSetting)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_C821_Auto_Fill_Screen_Setting(bSetting);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

void palImgMgr_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt)
{
#ifdef SCALER_C821_C789

    halScaler_Test(puiTtlCnt, puiErrCnt);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_PIPPBPPaniel_InfoSet(BYTE cCH, UINT16 uiHActive, UINT16 uiVActive, UINT16 uiHStart, UINT16 uiVStart)
{
#ifdef SCALER_C821_C789

    halScaler_PIPPBPPaniel_InfoSet(cCH, uiHActive, uiVActive, uiHStart, uiVStart);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_PowerNormalReadyGet(BYTE ucCH)
{
#ifdef SCALER_C821_C789

    halScaler_PowerNormalReadyGet(ucCH);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_3D_Reset(void)
{
#ifdef SCALER_C821_C789

	halScaler_3D_Reset();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT32 palImgMgr_ICP_Register_Read(UINT32 ulAddr)
{
#ifdef SCALER_C821_C789

    UINT32 ulData = halScaler_ICP_Register_Read(ulAddr);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_ICP_Register_Write(UINT32 ulAddr, UINT32 ucData)
{
#ifdef SCALER_C821_C789

    halScaler_ICP_Register_Write(ulAddr, ucData);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

ePAL_IMG_EXEC_CODE palImgMgr_ClockGen_Init(ePAL_CLOCKGEN_SEL eClockgenSel)
{
#ifdef SCALER_C821_C789

    eHAL_SCALER_EXEC_CODE eRet = halScaler_ClockGen_Init(eClockgenSel);

    return palImgMgr_ExeResult(__FUNCTION__, eRet);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return ePAL_IMG_EXEC_CODE_UNSUPPORTED;

#endif

}

void palImgMgr_PowerNormalDone(BOOL bDone)
{
#ifdef SCALER_C821_C789

    halScaler_PowerNormalDone(bDone);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}


UINT16 palImgMgr_FrameDelay_Max_Get(void)
{
#ifdef SCALER_C821_C789

    return (((halC789Ctrl_Output_V_Total_Get() - 10) / 8) - 1);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return FRAME_DELAY_MAX_VALUE;

#endif
}

UINT16 palImgMgr_PanelOutputVTotal_Get(void)
{
#ifdef SCALER_C821_C789

    return halC789Ctrl_Output_V_Total_Get();

#elif defined(SCALER_C341) //H30K_Doulas_0003
    BOOL   bForceSyncReseVal;
    UINT16 uiH_Toral;
    UINT16 uiV_Toral;
    halScaler_HV_Total_Get(&bForceSyncReseVal,&uiH_Toral,&uiV_Toral);

    return uiV_Toral;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

//Warping Block Black Screen set
UINT8 palImgMgr_WB_Black_Screen_Set(UINT8 ucData)
{
#ifdef SCALER_C821_C789

    return halC789Ctrl_Black_Screen_Set(ucData);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

//Warping Block Black Screen get
UINT8 palImgMgr_WB_Black_Screen_Get(void)
{
#ifdef SCALER_C821_C789

    return halC789Ctrl_Black_Screen_Get();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}


void palImgMgr_WB_OutputEnableSet(UINT8 ucEnable)
{
#ifdef SCALER_C821_C789

    halC789Ctrl_OutputEnableSet(ucEnable);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}


UINT8 palImgMgr_WB_OutputEnableGet(void)
{
#ifdef SCALER_C821_C789

    return halC789Ctrl_OutputEnableGet();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return TRUE;     //HICC2_Simon_0025

#endif
}


void palImgMgr_WB_OutputEnable_Set(UINT8 ucEnable)
{
#ifdef SCALER_C821_C789

    halC789Ctrl_OutputChanged_Set(ucEnable);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}


UINT8 palImgMgr_WB_OutputChenged_Get(void)
{
#ifdef SCALER_C821_C789

    return halC789Ctrl_OutputChanged_Get();

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

//Warping Block Freeze
void palImgMgr_WB_Freeze(UINT8 ucEnable)
{
#ifdef SCALER_C821_C789

    halC789Ctrl_Freeze(ucEnable);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_HDRDemo(void)
{
#ifdef SCALER_FPGA_F34
    halScaler_SetHDRDemo();
#else
    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
#endif

}

BOOL palImgMgr_Get_Dump_Register_Flag(void) //H30K_Doulas_0067
{
    return m_bDump_Register_Flag;
}

void palImgMgr_Set_Dump_Register_Flag(BOOL bDump_Register_Flag) //H30K_Doulas_0067
{
    m_bDump_Register_Flag = bDump_Register_Flag;
}

INT16 palImgMgr_Dump_Register_Poll(UINT16 uiTick) //H30K_Doulas_0067
{
    if(m_bDump_Register_Flag)
    {
        m_bDump_Register_Flag = 0;
        palImgMgr_Dump_Register(NULL);
    }
    return POLL_STARTUP_DELAY / POLL_PERIOD;  //1sec
}


void palImgMgr_HdrMetaDataGet(UINT16 *ucData)
{
#ifdef SCALER_FPGA_F34
    halScaler_HdrMetaDataGet(ucData);
#else
    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
#endif

}

void palImgMgr_HdrMaxMasteringLuminanceGet(UINT16 *ucData)
{
#ifdef SCALER_FPGA_F34
    halScaler_HdrMaxMasteringLuminanceGet(ucData);

#else
    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
#endif

}

void palImgMgr_Hdr10pEnSet(bool bSwitch)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pEnSet(bSwitch);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
#endif
}

BOOL palImgMgr_Hdr10pEnGet(void)
{
#ifdef SCALER_FPGA_F34

    BOOL bValue = halScaler_Hdr10pEnGet();

    return bValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pModeSet(UINT8 ucMode)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pModeSet(ucMode);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_Hdr10pModeGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 ucValue = halScaler_Hdr10pModeGet();

    return ucValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

#if 0

void palImgMgr_Hdr10pSglMaxLumSet(UINT16 SglMax)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pSglMaxLumSet(SglMax);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif

}

UINT16 palImgMgr_Hdr10pSglMaxLumGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pSglMaxLumGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pSglMinLumSet(UINT16 SglMin)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pSglMinLumSet(SglMin);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT16 palImgMgr_Hdr10pSglMinLumGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pSglMinLumGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pDisMaxLumSet(UINT16 DisMax)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pDisMaxLumSet(DisMax);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT16 palImgMgr_Hdr10pDisMaxLumGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pDisMaxLumGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pDisMinLumSet(UINT16 DisMin)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pDisMinLumSet(DisMin);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT16 palImgMgr_Hdr10pDisMinLumGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pDisMinLumGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pHdrGammaSet(UINT16 Gamma)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pHdrGammaSet(Gamma);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT16 palImgMgr_Hdr10pHdrGammaGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pHdrGammaGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}
#endif

void palImgMgr_Hdr10pMaxLevelSet(UINT16 MaxLevel)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pMaxLevelSet(MaxLevel);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT16 palImgMgr_Hdr10pMaxLevelGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pMaxLevelGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pMaxLevelMinSet(UINT16 MaxLevelMin)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pMaxLevelMinSet(MaxLevelMin);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT16 palImgMgr_Hdr10pMaxLevelMinGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pMaxLevelMinGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pFrameAdaptSet(UINT16 FrameAdapt)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pFrameAdaptSet(FrameAdapt);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT16 palImgMgr_Hdr10pFrameAdaptGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pFrameAdaptGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pFrameFeqSet(UINT16 FrameFeq)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pFrameFeqSet(FrameFeq);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT16 palImgMgr_Hdr10pFrameFeqGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pFrameFeqGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}


void palImgMgr_Hdr10pCSCEnSet(bool bSwitch)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pCSCEnSet(bSwitch);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

BOOL palImgMgr_Hdr10pCSCEnGet(void)
{
#ifdef SCALER_FPGA_F34

    BOOL bValue = halScaler_Hdr10pCSCEnGet();

    return bValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_HdrCscCoorTransSet(FLOAT *fValue)
{
#ifdef SCALER_FPGA_F34

    halScaler_HdrCscCoorTransSet(fValue);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_HdrCscCoorTransGet(FLOAT *fValue)
{
#ifdef SCALER_FPGA_F34

    halScaler_HdrCscCoorTransGet(fValue);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

void palImgMgr_HdrLevelModeSet(UINT8 ucLevel)
{
#ifdef SCALER_FPGA_F34

    halScaler_HdrLevelModeSet(ucLevel);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT8 palImgMgr_HdrLevelModeGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT8 ucValue = halScaler_HdrLevelModeGet();

    return ucValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

#if 0

void palImgMgr_Hdr10pSglMaxLum_Cst_Set(UINT16 DisMax)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pSglMaxLum_Cst_Set(DisMax);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

UINT16 palImgMgr_Hdr10pSglMaxLum_Cst_Get(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10pSglMaxLum_Cst_Get();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_Hdr10pInfoMode_Cst_Set(bool bSwitch)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10pInfoMode_Cst_Set(bSwitch);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

BOOL palImgMgr_Hdr10pInfoMode_Cst_Get(void)
{
#ifdef SCALER_FPGA_F34

    bool bValue = halScaler_Hdr10pInfoMode_Cst_Get();

    return bValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

#endif

void palImgMgr_Hdr10MaxLimitSet(UINT16 uiMaxLimit)
{
#ifdef SCALER_FPGA_F34

    halScaler_Hdr10MaxLimitSet(uiMaxLimit);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}

UINT16 palImgMgr_Hdr10MaxLimitGet(void)
{
#ifdef SCALER_FPGA_F34

    UINT16 uiValue = halScaler_Hdr10MaxLimitGet();

    return uiValue;

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
    return 0;

#endif
}

void palImgMgr_HdrG1_SubOffSet(bool bOnOff) //HICC2_AC_0102
{
#ifdef SCALER_FPGA_F34

    halScaler_HdrG1_SubOffSet(bOnOff);

#else

    LOG_PALIMGMGR_FUNC_UNSUPPORTED;

#endif
}



eEXEC_CODE palImg_HDR_Set(UINT8 ucHDRDetect,   //HDR Auto , Disable
                                eCM_HDR_GAMMA_FUNCTION eHDRGamma,
                                eCM_HDR_MODE eHDRMode,
                                UINT16 uiNit,        //mastering max luminance
                                UINT16 uiMaxLimit)   //monitor luminance
{
    eEXEC_CODE eExecResult = eEXEC_CODE_PASS;

    //if(CFG_CUSTOMER_ID == CUSTOMER_ID_FUJIFILM) //HICC2_AC_0112 //HICC2_Casper_0060
    {
        LOG_MSG(db_APP_DATAPATH, "read HDR Nit = %d\r\n", uiNit);
        if(uiNit == 0)
        {
            LOG_MSG(db_APP_DATAPATH, "set HDR Nit min = 1000 \r\n");
            uiNit = 1000;
        }
    }

#ifdef SCALER_FPGA_F34
    halScaler_HDR10_Process(ucHDRDetect, eHDRGamma, eHDRMode, uiNit, uiMaxLimit);
#elif defined(SCALER_C341)

#else
    LOG_PALIMGMGR_FUNC_UNSUPPORTED;
#endif

    return eEXEC_CODE_PASS;
}

