// ===============================================================================
// FILE NAME: appDataControlMgr.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2024/08/21, Larry Create
// --------------------
// ===============================================================================

#ifdef __APPDATACONTROLMGR_C__

typedef struct
{
    UINT16 uiDataControl;
    eFUNC_CONTROL (*Item_Access_Enable)(void);
} sDATA_CONTROL_LUT_TABLE;

eFUNC_CONTROL palDataMgr_Control_Normal(void)
{
    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_BIST_Enable(void) //HICC2_Steven_0071
{
    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE &&
    		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucBIST!=BIST_STATUS_BUSY)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_System_PowerState(void)
{
    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_SourceDepend(void)         //A70LV_Doulas_0032 modify
{
    if(( palDataPath_IsSourceLock() == TRUE) )//||
       //((palDataPath_IsSourceLockSub() == TRUE) &&
       // (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable == TRUE)) )
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_ResetImageSetting(void) //HICC2_Julie_0059
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucImageFreeze == ets_ON)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE) //HICC2_Doulas_0151
	    {
            if(utilCounterGet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR) == 0)
            {
                return eFUNC_CONTROL_SOURCE_DEPEND;
            }
            else
            {
                return eFUNC_CONTROL_ENABLE;
            }
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
    }
}

eFUNC_CONTROL palDataMgr_Control_Cursor_Fixture_Enable(void)
{
#if 1// (defined(CURSOR_FIXTURE))
    return eFUNC_CONTROL_ENABLE;
#else
    return eFUNC_CONTROL_DISABLE;
#endif
}

eFUNC_CONTROL palDataMgr_Control_Lens_DurationTime_Enable(void)
{
#if 1// (defined(CURSOR_FIXTURE))
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensDurationTime_Enable == ets_OFF)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
#else
    return eFUNC_CONTROL_DISABLE;
#endif
}

eFUNC_CONTROL palDataMgr_Control_AdvWarpingControl(void)    //H2PF_Simon_0088
{
	if(palGeo_ApLinkFlag_Get() == 1 ||
	   palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP ||
	   palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE ||
       palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE
#ifdef SCALER_FPGA_F34
       || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0)
#endif
    )
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_SubSourceDepend(void)  //G100_Coda_00105
{
    //if(palDataPath_IsSourceMonitorSub() == TRUE)
    //if (palDataPath_IsSourceLockSub() == TRUE)
    if((palDataPath_IsSourceLockSub() == TRUE) &&
        (palDataPath_GetDataPathState() > eDATA_PATH_STATE_SUSPENDED) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != FALSE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}


eFUNC_CONTROL palDataMgr_Control_ServiceProtected(void)
{
    return eFUNC_CONTROL_SERCIVE_PROTECT;
}

eFUNC_CONTROL palDataMgr_Control_PIP_PBP_Enable(void)   //A70LV_Doulas_0032
{
    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != FALSE) &&
        (palDataPath_GetDataPathState() > eDATA_PATH_STATE_SUSPENDED))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_Pixel_Track_Phase(void)   //A70LV_Doulas_0032
{
#if 1
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
    #ifdef SCALER_C341 //H30K_Doulas_0044 Modify
       (palImgMgr_IsInterlaced_Get((UINT8)eSOURCE_WINDOW_MAIN) == FALSE) && //interlaced timing gray out
       (palSystem_PanelID_Get() != PANEL_2D_HIGHSPEED) &&
    #else
       (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain == eCM_SOURCE_VGA) && //H30K_Doulas_0041
    #endif
       (palDataMgr_Control_PIP_PBP_Enable() != eFUNC_CONTROL_ENABLE) &&
       (palDataMgr_IS_3D_Enable() == ets_OFF)) //G100_Steven_0026 //G100_Casper_0005
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#else
    return eFUNC_CONTROL_SOURCE_DEPEND;
#endif /* 0 */
}

eFUNC_CONTROL palDataMgr_Control_Scaler(void)   //A70LV_Doulas_0032
{
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       //(palSystem_PanelID_Get() != PANEL_3D_OUTPUT) &&     	 //G100_Doulas_0064 remove//A70LV_Doulas_0155 Add
       (palSystem_PanelID_Get() != PANEL_3D_OUTPUT) &&     //G100_Doulas_0064 Add
       (palSystem_PanelID_Get() != PANEL_2D_HIGHSPEED) &&     //G100_Doulas_0064 Add
       (palDataMgr_IS_3D_Enable() == ets_OFF) &&                 //A70LV_Doulas_0205 Add
       (palDataMgr_Control_PIP_PBP_Enable() != eFUNC_CONTROL_ENABLE))   //A70LV_Doulas_0049 modify
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_AspectRatio(void)
{
    UINT8 ucData;
    UINT8 ucDataUser;
    palDataMgr_Access_PictureSettings(edaREAD, &ucData);
    palDataMgr_Access_PreUserMode(edaREAD, &ucDataUser);

    //printf("(%s, %d) ucData = %d, ucDataUser = %d\n", __FUNCTION__, __LINE__, ucData, ucDataUser);
    if(ucData == eCM_PICTURE_SETTINGS_2DHIGHSPEED ||
    #if defined(CUSTOM_OPTOMA) //H30K_Doulas_0024
       (ucData == eCM_PICTURE_SETTINGS_3D) ||
       (ucData == eCM_PICTURE_SETTINGS_USER && ucDataUser == eCM_PICTURE_SETTINGS_3D) ||
    #endif
      (ucData == eCM_PICTURE_SETTINGS_USER && ucDataUser == eCM_PICTURE_SETTINGS_2DHIGHSPEED))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }

    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (palDataMgr_Control_PIP_PBP_Enable() != eFUNC_CONTROL_ENABLE))
    {
        //printf("(%s, %d)\n", __FUNCTION__, __LINE__);
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_DigitalHorzShift(void)     //A70LV_Doulas_0032
{
    UINT16 uiVal;

    palDataMgr_Access_DigitalHorzZoom(edaREAD,&uiVal);  //A70LV_Doulas_0046 modify
    if((palDataMgr_Control_Scaler() == eFUNC_CONTROL_ENABLE) &&
       (uiVal != DIGITAL_HORZ_ZOOM_DEFAULT))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_DigitalVertShift(void)     //A70LV_Doulas_0032
{
    UINT16 uiVal;

    palDataMgr_Access_DigitalVertZoom(edaREAD,&uiVal);  //A70LV_Doulas_0046 modify
    if((palDataMgr_Control_Scaler() == eFUNC_CONTROL_ENABLE) &&
       (uiVal != DIGITAL_VERT_ZOOM_DEFAULT))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_ContrastEnhancement(void)     //A70LV_Doulas_0046
{
    UINT8 ucVal = 0;
	UINT8 ucConstantBrightness = 0; //G100_Doulas_0022

#if defined(PLATFORM_H30_4K)
	if(palDataMgr_getDimStatus() != DIM_POWER_NUMBER_DEFAULT_VALUE)
	{
		return eFUNC_CONTROL_DISABLE;
	}
#endif
    palDataMgr_Access_PowerMode(edaREAD,&ucVal);
	palDataMgr_Access_Constant_Brightness(edaREAD, &ucConstantBrightness);	//G100_Doulas_0022
	#if defined(CUSTOM_OPTOMA) //A35G2_Coda_0102
	if((ucVal != eCM_POWER_MODE_CUSTOM_MODE) || (ucConstantBrightness != ets_ON))
    #elif defined(CUSTOM_CHRISTIE)
	if(ucVal != eCM_POWER_MODE_CONSTANT_INTENSITY)
	#else
    if((ucVal != eCM_POWER_MODE_CONSTANT_POWER) || (ucConstantBrightness != ets_ON))		//G100_Doulas_0022 Modify
    #endif
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_POWER_MODE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_PowerMode(void)     //A70LV_Doulas_0046
{
#if defined(PLATFORM_H30_4K)
	if(palDataMgr_getDimStatus() != DIM_POWER_NUMBER_DEFAULT_VALUE)
	{
        return eFUNC_CONTROL_DISABLE;
    }
#endif
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_CONTRAST_ENHANCEMENT_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_ConstantPower(void) //A70LV_Larry_0084
{
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A35G2_Coda_0043
    //if( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF && m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucPowerMode == eCM_POWER_MODE_CONSTANT_POWER)
	if( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF &&
		m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucPowerMode == eCM_POWER_MODE_CUSTOM_MODE&&
		m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucConstantBrightness == ets_OFF)
#else
	if( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF &&
		m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucPowerMode == eCM_POWER_MODE_CONSTANT_POWER&&
		m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucConstantBrightness == ets_OFF)	//G100_Doulas_0008 Modify
#endif
	{
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_CONTRAST_ENHANCEMENT_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_HSG_Adjusetment(void)     //A70LV_Doulas_0046
{
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
    return eFUNC_CONTROL_ENABLE;
#elif defined(CUSTOM_CHRISTIE)
    if(palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP)
    {
        return eFUNC_CONTROL_HSG_DEPEND;
    }

    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE)
        || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != FALSE)
        || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucSourceKeyOption != eSOURCE_KEY_CHANGE_SOURCE_AUTO))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        if((palDataPath_GetDataPathState() == eDATA_PATH_STATE_TPG_DISPLAYED)
            || (palDataPath_GetDataPathState() == eDATA_PATH_STATE_BEGIN_SCAN)
            || (palDataPath_GetDataPathState() == eDATA_PATH_STATE_LOOK_FOR_SYNCS))
        {
            UINT8 ucVal;
            // Bit 0 retrn Main Source Status
            // Bit 2 ~ 4 retrn Crrent Main Source ID
            // 00:HDMI1, 01:HDMI2, 02:DVI, 03:3GSDI, 04:HDBASET
            halFrontEndCtrl_videoReady_Get(&ucVal);
            if((ucVal & 0x01) && (((ucVal>>2)&0x07) == (UINT8)CM2GUI(edcMAIN_INPUT, m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain)))
            {
                return eFUNC_CONTROL_ENABLE;
            }
        }
        return eFUNC_CONTROL_HSG_DEPEND;
    }
#else
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHSGAdjustmentEnable == ets_ON)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_HSG_DEPEND;
    }
#endif
}

eFUNC_CONTROL palDataMgr_Control_ColorEnhancement(void)     //A70LV_Doulas_0046
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHSGAdjustmentEnable == ets_OFF)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_HSG_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_FunctionNotReady(void) //A70LV_Larry_0048
{
    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_BlendingTopEnable(void) //A70LV_Larry_0051
{
    if(palGeo_ApLinkFlag_Get() == 0 &&  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingTopEnable)
        #ifdef SCALER_FPGA_F34
        && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable)
        #endif
        )
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}


eFUNC_CONTROL palDataMgr_Control_BlendingBottomEnable(void) //A70LV_Larry_0051
{
    if(palGeo_ApLinkFlag_Get() == 0 &&  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingBottomEnable)
        #ifdef SCALER_FPGA_F34
        && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable)
        #endif
        )
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}


eFUNC_CONTROL palDataMgr_Control_BlendingLeftEnable(void) //A70LV_Larry_0051
{
    if(palGeo_ApLinkFlag_Get() == 0 &&  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingLeftEnable)
        #ifdef SCALER_FPGA_F34
        && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable)
        #endif
        )
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}


eFUNC_CONTROL palDataMgr_Control_BlendingRightEnable(void) //A70LV_Larry_0051
{
    if(palGeo_ApLinkFlag_Get() == 0 &&  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingRightEnable)
        #ifdef SCALER_FPGA_F34
        && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable)
        #endif
        )
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}


eFUNC_CONTROL palDataMgr_Control_WheelIndex2X(void) //A70LV_Doulas_0061
{
    UINT8 ucColorWheelSpeed = 0;

    palDataMgr_Access_ColorWheelSpeed(edaREAD,&ucColorWheelSpeed);
    if(ucColorWheelSpeed == eCM_COLOR_WHEEL_SPEED_2X)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_WheelIndex3X(void)
{
    UINT8 ucColorWheelSpeed = 0;

    palDataMgr_Access_ColorWheelSpeed(edaREAD,&ucColorWheelSpeed);
    if(ucColorWheelSpeed == eCM_COLOR_WHEEL_SPEED_3X)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_ColorSpace(void)   //A70LV_Doulas_0069
{
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (palDataMgr_Control_PIP_PBP_Enable() != eFUNC_CONTROL_ENABLE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_WarpAutoFilter(void) //A70LV_Larry_0072
{
    //if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucWarpAutoFilter) ||
    //   (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucWarpingApplySetting != 0))
    if((palGeo_ApLinkFlag_Get() == 1) ||  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucWarpAutoFilter)
	    #ifdef SCALER_FPGA_F34
        || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0)
        #endif
        )   //G100_Doulas_0027 Modify
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP ||
            palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE) //A35G2_Simon_0115
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_WarpingEnable(void) //A70LV_Larry_0072
{
	if((palGeo_ApLinkFlag_Get() == TRUE) ||  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP)
	    #ifdef SCALER_FPGA_F34
        || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0)
        #endif
        )
    {
        return eFUNC_CONTROL_DISABLE;
    }
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl != WARP_CTRL__BASIC) //A35G2_Simon_0069
    {
        return eFUNC_CONTROL_DISABLE;
    }
#endif
    else if((palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP ||
              palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE) &&
            Syscfg_Value_Get_Typeint(eGeo_BehaviorType) == eGEO_SUPPORT_TWIST_MYSTIQUE)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_ADVANCED &&
            Syscfg_Value_Get_Typeint(eGeo_BehaviorType) == eGEO_SUPPORT_TWIST_MYSTIQUE)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_BlendingEnable(void) //A70LV_Larry_0072
{
	if((palGeo_ApLinkFlag_Get() == TRUE) ||  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP) ||
        #ifdef SCALER_FPGA_F34
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0) ||
        #endif
        (palDataMgr_ACU_Target_Status_Get() == ets_ON))
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP ||
            palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP_MYSTIQUE)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

//Warp Memory Apply grayout condition for OSD/web/rs232
eFUNC_CONTROL palDataMgr_Control_WarpingApply(void) //A70LV_Larry_0301
{
    if((palGeo_ApLinkFlag_Get() == 0) &&  //A35G2_Simon_0086
       (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
       #ifdef SCALER_FPGA_F34
       (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable) &&
       #endif
       ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM1_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM2_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM3_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM4_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM5_IDX].ucWarpingApply != 0)) )		//G100_Doulas_0040 Modify
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

//Warp Memory Apply grayout condition for blending ap
eFUNC_CONTROL palDataMgr_Control_Warping_AP_Apply(void) //A35G2_Simon_0083
{
    if((palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
       ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM1_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM2_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM3_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM4_IDX].ucWarpingApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM5_IDX].ucWarpingApply != 0)) )		//G100_Doulas_0040 Modify
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_WarpingSave(void)  //H2PF_Simon_0154
{
    //A35G2_Simon_0073
    if(palGeo_ApLinkFlag_Get() == 1 ||
       palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP
       #ifdef SCALER_FPGA_F34
       || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0)
       #endif
    )
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(CFG_GEO_BEHAVIORTYPE == eGEO_SUPPORT_TWIST_MYSTIQUE)   //without warp control
    {
        if(palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP ||
           palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE)
        {
            return eFUNC_CONTROL_DISABLE;
        }
    }
    else if(CFG_GEO_BEHAVIORTYPE == eGEO_SUPPORT_CORE_BLEND_AP_AND_BASIC_WARP_NO_MEMORY)   //with warp control , but basic no warp memory
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__BASIC ||
           m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__AP )
        {
            return eFUNC_CONTROL_DISABLE;
        }
    }
    else     //with warp control
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__AP)
        {
            return eFUNC_CONTROL_DISABLE;
        }
    }

    return eFUNC_CONTROL_ENABLE;

}


//Blend Memory Apply grayout condition for OSD/Web/RS232
eFUNC_CONTROL palDataMgr_Control_BlendApply(void) //A70LV_Larry_0301
{
    if((palGeo_ApLinkFlag_Get() == 0) &&  //A35G2_Simon_0086
       (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
       #ifdef SCALER_FPGA_F34
       (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable) &&
       #endif
	   ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM1_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM2_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM3_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM4_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM5_IDX].ucBlendApply != 0)) )		//G100_Doulas_0040 Modify
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

//Blend Memory Apply grayout condition for OSD/Web/RS232
eFUNC_CONTROL palDataMgr_Control_Blend_AP_Apply(void)   //A35G2_Simon_0083
{
    if((palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
	   ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM1_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM2_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM3_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM4_IDX].ucBlendApply != 0) ||
	    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM5_IDX].ucBlendApply != 0)) )		//G100_Doulas_0040 Modify
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_BlendSave(void)  //A35G2_Simon_0081
{
    if(palGeo_ApLinkFlag_Get() == 1 ||      //G100_Doulas_0027 Modify  //A35G2_Simon_0086
       palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP ||
       palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP ||
       palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP_MYSTIQUE
       #ifdef SCALER_FPGA_F34
       || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0)
       #endif
       )
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }

}

eFUNC_CONTROL palDataMgr_Control_WarpingAPSave(void)
{
    if((palGeo_ApLinkFlag_Get() == 0) &&  //A35G2_Simon_0086
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP))
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_WarpingControl(void)    //A35G2_Simon_0069
{
	if(palGeo_ApLinkFlag_Get() == 1 ||  //A35G2_Simon_0086
	   palDataPath_GetDataPathState() == eDATA_PATH_STATE_SPLASH_AT_STARTUP
//#ifdef SCALER_FPGA_F34  //H2PF_Simon_0042 remove
        //|| (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == 0)
//#endif
        )
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_TestPatternDepend(void)         //A70LV_Doulas_0082
{
    //if(palDataPath_IsTestPatternEnable() == FALSE)
    if(palDataPath_GetDataPathState() > eDATA_PATH_STATE_SUSPENDED)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_TEST_PATTERN_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_PIP_PBP_Disable(void)   //A70LV_Doulas_0082
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable == FALSE)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_OSD_TestPattern(void)   //A70LV_Doulas_0086
{
#ifndef CUSTOM_CHRISTIE       //A70LV_Doulas_0230 midify
	if(palGeo_AdvWarpShowOsdPattern_Control() == TRUE)// HICC2_Bruce_0024
    {
        return eFUNC_CONTROL_TEST_PATTERN_DEPEND;
    }
    return eFUNC_CONTROL_ENABLE;
#else
    if(palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_TEST_PATTERN_DEPEND;
    }
#endif
}

eFUNC_CONTROL palDataMgr_Control_Service_TestPattern(void)   //A70LV_Doulas_0086
{
#ifndef CUSTOM_CHRISTIE   //A70LV_Doulas_0230 modify
  #if defined(PLATFORM_H30_4K)
	if(palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)
	{
		return eFUNC_CONTROL_DISABLE;
	}
	if(palSystem_PanelID_Get() == PANEL_3D_OUTPUT)
	{
		return eFUNC_CONTROL_DISABLE;
	}
	if(palSystem_PanelID_Get() == PANEL_2D_HIGHSPEED) //H30K_Doulas_0062 Add
	{
		return eFUNC_CONTROL_DISABLE;
	}
  #endif
	if(palGeo_AdvWarpShowOsdPattern_Control() == TRUE)// HICC2_Bruce_0024
    {
        return eFUNC_CONTROL_TEST_PATTERN_DEPEND;
    }
    return eFUNC_CONTROL_ENABLE;
#else
    if(palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_TEST_PATTERN_DEPEND;
    }
#endif
}

eFUNC_CONTROL palDataMgr_Control_TP_NR(void)   //A70LV_Doulas_0092
{
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (palImgMgr_IsInterlaced_Get((UINT8)eSOURCE_WINDOW_MAIN) == TRUE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }

}

eFUNC_CONTROL palDataMgr_Control_MPEG_NR(void)   //A70LV_Doulas_0092
{
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (palImgMgr_IsInterlaced_Get((UINT8)eSOURCE_WINDOW_MAIN) == TRUE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }

}

eFUNC_CONTROL palDataMgr_Control_USTMode(void)   //ZU860_Energy_0008 //A35G2_BRC_Casper_0088
{
	if(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucUST_LensInstallType == ets_OFF)
	{
		return eFUNC_CONTROL_ENABLE;
	}
	else
	{
		return eFUNC_CONTROL_LENS_LOCK_DEPEND;
	}
}

eFUNC_CONTROL palDataMgr_Control_UST_Pattern(void)   //ZU860_Energy_0008 //A35G2_BRC_Casper_0088
{
	if(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucUST_LensInstallType == ets_ON)
	{
		return eFUNC_CONTROL_ENABLE;
	}
	else
	{
		return eFUNC_CONTROL_LENS_LOCK_DEPEND;
	}
}

eFUNC_CONTROL palDataMgr_Control_UST_Set(void)   //ZU860_Energy_0008 //A35G2_BRC_Casper_0088
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW &&
       m_sAppDataMgrInfo.sSystemValues.ucLensCalibrationDoing == 0)
	{
        if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100)
		|| (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
		&& (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1)) //HICC2_Julie_0070//H30K_Julie_0005
        {
    		return eFUNC_CONTROL_DISABLE;
        }
        else
        {
    		return eFUNC_CONTROL_ENABLE;
        }
	}
	else
	{
		return eFUNC_CONTROL_LENS_LOCK_DEPEND;
	}
}

eFUNC_CONTROL palDataMgr_Control_USTLens_Lock(void)   //ZU860_Energy_0008 //A35G2_BRC_Casper_0088
{
	if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW &&
       m_sAppDataMgrInfo.sSystemValues.ucLensCalibrationDoing == 0 &&
       palDataMgr_Control_USTMode() == eFUNC_CONTROL_ENABLE)
	{
		return eFUNC_CONTROL_ENABLE;
	}
	else
	{
		return eFUNC_CONTROL_LENS_LOCK_DEPEND;
	}
}

eFUNC_CONTROL palDataMgr_Control_LensShift(void)   //SNPLU9000_Energy_0032 //A35G2_BRC_Casper_0088
{
	if((palDataMgr_Control_USTLens_Lock() == eFUNC_CONTROL_ENABLE))//SNPLU9000_Energy_0041
	{
		return eFUNC_CONTROL_ENABLE;
	}
	else
	{
		return eFUNC_CONTROL_LENS_LOCK_DEPEND;
	}
}

eFUNC_CONTROL palDataMgr_Control_Lens_Lock(void)   //A70LV_Doulas_0096 //A35G2_BRC_Casper_0088
{
    if(palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW &&
       m_sAppDataMgrInfo.sSystemValues.ucLensCalibrationDoing == 0 &&
       palDataMgr_Control_USTMode() == eFUNC_CONTROL_ENABLE)
    {
        if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100) ||
             (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
            && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1) //1:unlock ; 0: lock
            )
        {
           return eFUNC_CONTROL_DISABLE;
        }
        #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
        if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
        {
            return eFUNC_CONTROL_DISABLE;
        }
        else if(((palInput_Lens_Type_Get() == eLENS_ID_B20U100) || (palInput_Lens_Type_Get() == eLENS_ID_NVRAM_B20U100)) &&
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1)) //HICC2_Julie_0070//H30K_Julie_0005
        {
            return eFUNC_CONTROL_DISABLE;
        }
        else
        {
        	return eFUNC_CONTROL_ENABLE;
        }
        #else
        return eFUNC_CONTROL_ENABLE;
        #endif
    }
    else
    {
        return eFUNC_CONTROL_LENS_LOCK_DEPEND;
    }

}

eFUNC_CONTROL palDataMgr_Control_Lens_Calibration(void) //G100_Owen_0062 //A35G2_BRC_Casper_0088
{
    #ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0149
    if(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_TYPE_KMUST) //A35G2_BRC_Casper_0149
    {
        return eFUNC_CONTROL_DISABLE;
    }
    #endif
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW &&
        palDataMgr_Control_USTMode() == eFUNC_CONTROL_ENABLE)
    {
        if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100) ||
             (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
            && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1) //1:unlock ; 0: lock
            )
        {
           return eFUNC_CONTROL_DISABLE;
        }

        #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
        if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
        {
            return eFUNC_CONTROL_DISABLE;
        }
        else
        {
        	return eFUNC_CONTROL_ENABLE;
        }
        #else
        return eFUNC_CONTROL_ENABLE;
        #endif
    }
    else
    {
        return eFUNC_CONTROL_LENS_LOCK_DEPEND;
    }

}

eFUNC_CONTROL palDataMgr_Control_LensControl(void)         //HICC2_Doulas_0132
{
    UINT8 ucData = 0;

    if(palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palEnvironment_LensCalFlag_Get() == FALSE)
    {
    	return eFUNC_CONTROL_DISABLE;
    }
    else if(m_sAppDataMgrInfo.sSystemValues.ucLensCalibrationDoing == 1)
    {
        return eFUNC_CONTROL_DISABLE;
    }
#ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
    else if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
    {
        return eFUNC_CONTROL_DISABLE;
    }
#endif
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Lens_Shift(void)
{
    UINT8 ucData = 0;

    if(palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palEnvironment_LensCalFlag_Get() == FALSE)
    {
    	return eFUNC_CONTROL_DISABLE;
    }
    else if(m_sAppDataMgrInfo.sSystemValues.ucLensCalibrationDoing == 1)
    {
        return eFUNC_CONTROL_DISABLE;
    }
#ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
    else if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
    {
        return eFUNC_CONTROL_DISABLE;
    }
#endif
    else if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100) ||
     (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
    && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1) //1:unlock ; 0: lock
    )
    {
       return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}


eFUNC_CONTROL palDataMgr_Control_LANPowerState(void)
{
    UINT8 ucData = 0;

    //wait web warmup 30s
    if(m_sAppDataMgrInfo.sSystemValues.PowerState.PowerStatus == ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_LensType(void)         //G100_Simon_0057 //A35G2_BRC_Casper_0088
{
    UINT8 ucData = 0;

    //if(( eLENS_TYPE_NA == m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType ) ||
        //(palEnvironment_LensCalFlag_Get() == FALSE))    //G100_Owen_0079
    if(palEnvironment_LensCalFlag_Get() == FALSE)
    {
    	return eFUNC_CONTROL_DISABLE;
    }
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW
            && palDataMgr_Control_USTMode() == eFUNC_CONTROL_ENABLE)
    {
        #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
        if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
        {
            return eFUNC_CONTROL_DISABLE;
        }
        #endif
        if(((palInput_Lens_Type_Get() == eLENS_ID_B20U100) || (palInput_Lens_Type_Get() == eLENS_ID_NVRAM_B20U100)) &&
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1)) //HICC2_Julie_0070 //HICC2_Casper_0090
        {
            return eFUNC_CONTROL_DISABLE;
        }


        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_LENS_LOCK_DEPEND;
    }

}

eFUNC_CONTROL palDataMgr_Control_MainOrSubSourceDepend(void)
{
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) || (palDataMgr_Control_SubSourceDepend() == eFUNC_CONTROL_ENABLE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_SyncThresholdDepend(void)         //A70LV_Doulas_0132
{
#if 0
    UINT8 ucMainInput;

    palDataMgr_Access_MainInput(edaREAD,&ucMainInput);

    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (ucMainInput == eINPUT_SOURCE_VGA) &&
       (palImgMgr_VGA_SYNC_TYPE_Get(eSOURCE_WINDOW_MAIN) == eVGA_SYNC_TYPE_SOG))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#else
    return eFUNC_CONTROL_SOURCE_DEPEND;
#endif /* 0 */
}

eFUNC_CONTROL palDataMgr_Control_3D_InputDepend(void)         //A70LV_Doulas_0156
{
    //if(palSystem_PanelID_Get() != PANEL_3D_OUTPUT)

	if((palSystem_PanelID_Get() == PANEL_2D_OUTPUT) //HICC2_Doulas_0133 Modify
#ifdef SCALER_FPGA_F34
        && (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D)
#endif /* SCALER_FPGA_F34 */
    ) 		//G100_Doulas_0064 modify
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_3D_INPUT_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_ColorSpaceSetting(void) //A70LV_Larry_0168
{
    if(palDataPath_IsSourceLock() == TRUE)
    {
        UINT8 ucValue = 0;
        palDataMgr_Access_ColorSpace(edaREAD, &ucValue);

        if(ucValue != eCM_COLOR_SPACE_AUTO)
        {
            return eFUNC_CONTROL_ENABLE;
        }

        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_SubColorSpaceSetting(void)
{
    if(palDataPath_IsSourceLockSub() == TRUE)
    {
        UINT8 ucValue = 0;
        palDataMgr_Access_SubColorSpace(edaREAD, &ucValue);

        if(ucValue != eCM_COLOR_SPACE_AUTO)
        {
            return eFUNC_CONTROL_ENABLE;
        }

        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_Art_Net_On_2x_10x_Disable(void)
{
	if((m_sArtNetInfo.ART_NET_Enable == eCM_ARTNET_SETUP_MODE_ON_2XXX)// HICC2_Bruce_0017
	|| (m_sArtNetInfo.ART_NET_Enable == eCM_ARTNET_SETUP_MODE_ON_10XXX))
	{
    	return eFUNC_CONTROL_DISABLE;
	}
    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_LAN_DHCP(void)
{
	if(palDataMgr_Control_Art_Net_On_2x_10x_Disable() != eFUNC_CONTROL_ENABLE)
	{
    	return eFUNC_CONTROL_DISABLE;
	}
    //H2 wait review
    if(m_sLAN_info.ucLAN_DHCP_Show == 0 || m_sAppDataMgrInfo.sSystemValues.ucNetworkWaitMessage == TRUE) //GuiCb.fpGui_IsNetWork_Wait_MessageCb() == TRUE) ////G100_Coda_0088 //G100 code move to appgui //###
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_IPV6_LAN_DHCP(void) // HICC2_Bruce_0020
{
    if(m_sLAN_info.ucLAN_IPV6_DHCP_Show == 0 || m_sAppDataMgrInfo.sSystemValues.ucNetworkWaitMessage == TRUE) //GuiCb.fpGui_IsNetWork_Wait_MessageCb() == TRUE) ////G100_Coda_0088 //G100 code move to appgui //###
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Twist(void) //A70LV_Larry_0294
{
    if(palEnvironment_NetworkReady() && palIllumination_FirstLightSourceOnGet())
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_HighAltitude(void)
{
    switch(m_sAppDataMgrInfo.sSystemValues.sFAN_INFO.ucHighAltitudeEnable)
    {
        case eFILTER_TYPE_FOG:
        case eFILTER_TYPE_NOFILTER_R:
        case eFILTER_TYPE_NOFILTER_L:
            return eFUNC_CONTROL_DISABLE;

        default:
            return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Backup_Restore(void)   //A70LV_Doulas_0280
{
    /*if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (palSystem_PanelID_Get() != PANEL_3D_OUTPUT) &&
       (palDataMgr_IS_3D_Enable() == ets_OFF) )
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else	//G100_Doulas_0002 remove
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }*/
    return eFUNC_CONTROL_ENABLE;	//G100_Doulas_0002 Modify
}

//G100_Steven_0088
eFUNC_CONTROL palDataMgr_Control_PIPPBP_Sub_ColorSetting(void)
{
    if(palDataPath_IsSourceLockSub() == TRUE && palImgMgr_PIP_PBP_Enable_Get()!=ets_OFF &&
        (palDataPath_GetDataPathState() > eDATA_PATH_STATE_SUSPENDED))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Backup_Restore_Load(void)   //G100_Doulas_0075
{
    if(palDataMgr_GetBackupRestoreExecution())
    {
        return eFUNC_CONTROL_DISABLE;
    }

    if(utilDataMgr_UserDataLoadItemAvailable_CM(eUSER_DATA_LOAD0) ||
	   utilDataMgr_UserDataLoadItemAvailable_CM(eUSER_DATA_LOAD1) ||
	   utilDataMgr_UserDataLoadItemAvailable_CM(eUSER_DATA_LOAD2) ||
	   utilDataMgr_UserDataLoadItemAvailable_CM(eUSER_DATA_LOAD3) ||
	   utilDataMgr_UserDataLoadItemAvailable_CM(eUSER_DATA_LOAD4))	//HICC2_Doulas_0003
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_ModeAdjustment(void)   //A70LV_Doulas_0198
{
#if 0
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
       (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain == eINPUT_SOURCE_VGA) &&
       (palDataMgr_Control_PIP_PBP_Enable() != eFUNC_CONTROL_ENABLE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#else
    return eFUNC_CONTROL_SOURCE_DEPEND;
#endif /* 0 */
}

eFUNC_CONTROL palDataMgr_Control_3D_EnableDepend(void)         //A70LV_Doulas_0239 Modify//A70LV_Doulas_0219
{
	UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;	//G100_Doulas_0020
	BYTE ucLow_Latency = eCM_LOW_LATENCY_MODE_OFF;
	//palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLow_Latency);
    palDataMgr_Access_LowLatencyMode(edaREAD, &ucLow_Latency);

    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if((palDataMgr_Control_BackupInputSW() == eFUNC_CONTROL_ENABLE) && (ets_ON == m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucBackupIputSwitch))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        if((eCM_3D_FORMAT_FRAME_SEQUENTIAL == m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable) &&
            (eSOURCE_STATE_SIGNAL_OUT_OF_RANGE == m_sAppDataMgrInfo.sSystemValues.sSOURCE_INFO.ucSourceState[0]))
        {
            return eFUNC_CONTROL_ENABLE;
        }

        if(utilCounterGet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR) == 0) //for OSD transient //A35G2_CDS_Larry_0055 //A35G2_BRC_Casper_0101
        {
            return eFUNC_CONTROL_SOURCE_DEPEND;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
    }
	else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Mode == eCM_3D_MODE_OFF)			//H30K_Doulas_0030 //G100_Doulas_0020 Add
	{
		return eFUNC_CONTROL_NO_3D_DEPEND;
	}
#ifndef SCALER_FPGA_F34
	else if(ucLow_Latency != eCM_LOW_LATENCY_MODE_OFF)
	{
		return eFUNC_CONTROL_DISABLE;
	}
#endif /* SCALER_FPGA_F34 */
#if defined(CUSTOM_BARCO) //G100_Tim_0057, add, start // no 3D when the Auto HDMI Switch is on
    #if (AUTO_HDMI_SWITCH_DETECT_5V == FALSE) //A35G2_Simon_0079  //A35G2_BRC_Casper_0060
	else if(palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_ON)
	{
		return eFUNC_CONTROL_DISABLE;
	}
	#endif
#endif
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_PictureSetting(void) //A70LV_Larry_0409
{
    if((palDataPath_IsSourceLock() == TRUE))
    {
        if((palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_3D_INPUT_DEPEND) || (palDataMgr_IS_3D_Enable() == ets_ON))
        {
            return eFUNC_CONTROL_NO_3D_DEPEND;
        }

        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }

}

eFUNC_CONTROL palDataMgr_Control_SaveToUserDepend(void)         //A70LV_Doulas_0219 Add
{
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A65_Owen_0002
    UINT8 ucPictureSettings = eCM_PICTURE_SETTINGS_PRESENTATION;
#else
    UINT8 ucPictureSettings = eCM_PICTURE_SETTINGS_VIDEO;                                      //A70LV_Doulas_0292
#endif
    palDataMgr_Access_PictureSettings(edaREAD,(void*)&ucPictureSettings);      //A70LV_Doulas_0292
    if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#if 0 //G100_Steven_0061 fix mantis #15440
    else if(palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_3D_INPUT_DEPEND)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_ON)   //A70LV_Doulas_0239 Modify
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
#endif
    else if(ucPictureSettings == eCM_PICTURE_SETTINGS_USER)     //A70LV_Doulas_0292
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;

    }
}

eFUNC_CONTROL palDataMgr_Control_WheelSpeedDepend(void)         //A70LV_Doulas_0219 Add
{
#ifdef CONFIG_4K_DISPLAY
    return eFUNC_CONTROL_DISABLE;
#else
    if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else if(palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_3D_INPUT_DEPEND)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_ON)      //A70LV_Doulas_0239 Modify
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
#endif /* CONFIG_4K_DISPLAY */
}

eFUNC_CONTROL palDataMgr_Control_ColorTemperatureDepend(void)         //A70LV_Doulas_0219 Add //A70LV_Larry_0327 Add blending mode check
{
    UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_3D_INPUT_DEPEND)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_ON)        //A70LV_Doulas_0239 Modify
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_BLENDING)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER &&  //G100_Steven_0032
       	 m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_BLENDING)
    {
    	return eFUNC_CONTROL_DISABLE;
    }
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_REC709)
    {
        return eFUNC_CONTROL_DISABLE; //A35G2_CDS_Coda_0007
    }
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER &&  //
       	 m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_REC709)
    {
    	return eFUNC_CONTROL_DISABLE; //A35G2_CDS_Coda_0007
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;

    }
}

eFUNC_CONTROL palDataMgr_Control_PIP(void)         //A70LV_Doulas_0226 //G100_Casper_0003
{
	BYTE ucLow_Latency = eCM_LOW_LATENCY_MODE_OFF;
	//palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLow_Latency);
    palDataMgr_Access_LowLatencyMode(edaREAD, &ucLow_Latency);

#if defined(CUSTOM_BARCO) //A35G2_BRC_Casper_0060
    if(((palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_ENABLE) ||
            (palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)) &&     //H2 wait review (eGUI_TEST_PATTERN_OFF)
            (palDataPath_GetDataPathState() > eDATA_PATH_STATE_SUSPENDED) &&
            (palDataMgr_CurTestPatternGet() == eTID_OFF) &&
            (ucLow_Latency == eCM_LOW_LATENCY_MODE_OFF) &&
            (palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_OFF)) //HICC2_Doulas_0133

#else
	if((((palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_ENABLE) && (palDataPath_Last3DEnable_Get() != ets_ON)) ||
	    (palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE) ) &&    //H2 wait review (eGUI_TEST_PATTERN_OFF)
        (palDataPath_GetDataPathState() > eDATA_PATH_STATE_SUSPENDED) &&
        (palDataMgr_CurTestPatternGet() == eTID_OFF) &&
        (ucLow_Latency == eCM_LOW_LATENCY_MODE_OFF))   //HICC2_Doulas_0133
#endif
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_3D_INPUT_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_MenuOffset(void)  //A70LV_Larry_0258
{
    #ifdef SCALER_FPGA_F34
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == ets_OFF)
	{
		return eFUNC_CONTROL_ENABLE;
	}
    #endif

	if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingLeftEnable == FALSE) &&
		(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingRightEnable == FALSE) &&
		(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingTopEnable == FALSE) &&
		(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingBottomEnable == FALSE))
	{
		return eFUNC_CONTROL_ENABLE;
	}
	else
	{
		return eFUNC_CONTROL_DISABLE;
	}
}

eFUNC_CONTROL palDataMgr_Control_3D_InvertDepend(void)         //A70LV_Doulas_0239
{
    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_OFF)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_FrameDelayDepend(void)         //A70LV_Doulas_0239
{
#ifdef SCALER_FPGA_F34
    UINT8 uc3D_Enable = eCM_3D_FORMAT_OFF;
    palDataMgr_Access_3DEnable(edaREAD, &uc3D_Enable);          //A70LV_Doulas_0277
    //palDataMgr_Access_3D_Reference(edaREAD, &uc3D_Reference);   //A70LV_Doulas_0277
    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_OFF)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if((uc3D_Enable == eCM_3D_FORMAT_AUTO) &&
            ((halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
             (halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM) ||
             (halScaler_Input_3D_Format_Get() == eINPUT_3D_TYPE_SIDEBYSIDE)))       //A70LV_Doulas_0277 3D 1.4a and auto mode
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
#else
    UINT8 uc3D_Enable = eCM_3D_FORMAT_OFF;
    UINT8 uc3D_Reference = eCM_3D_LR_REFERENCE_1ST_FRAME;
    palDataMgr_Access_3DEnable(edaREAD, &uc3D_Enable);          //A70LV_Doulas_0277
    palDataMgr_Access_3D_Reference(edaREAD, &uc3D_Reference);   //A70LV_Doulas_0277
    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_OFF)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if((uc3D_Enable == eCM_3D_FORMAT_AUTO)||
            (uc3D_Enable == eCM_3D_FORMAT_FRAME_PACKING)||
            (uc3D_Enable == eCM_3D_FORMAT_SIDE_BY_SIDE)||
            (uc3D_Enable == eCM_3D_FORMAT_TOP_AND_BOTTOM))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#ifdef FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC
    else if(uc3D_Enable == eCM_3D_FORMAT_FRAME_SEQUENTIAL)
    {
        if(palDataMgr_FRAME_SEQUENTIAL_3D_SYNC_Select_Get() == e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC)
        {
            return eFUNC_CONTROL_DISABLE;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
    }
#endif /* FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC */
#ifdef FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC	//G100_Doulas_0071 Modify
#else
    else if(uc3D_Reference == eCM_3D_LR_REFERENCE_1ST_FRAME)          //A70LV_Doulas_0277
    {
        return eFUNC_CONTROL_LR_REFERENCE_DEPEND;
    }
#endif
    else if(palImgMgr_PanelOutputVTotal_Get() < 10)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }

#endif /* SCALER_FPGA_F34 */

}

eFUNC_CONTROL palDataMgr_Control_LR_ReferenceDepend(void)         //A70LV_Doulas_0239
{
	#ifdef FRAME_SEQUENTIAL_USE_C821_OUPUT_3D_SYNC	//G100_Doulas_0071 Modify
	return eFUNC_CONTROL_DISABLE;
	#else
    UINT8 uc3D_Enable = 0;
    palDataMgr_Access_3DEnable(edaREAD, &uc3D_Enable);  //A70LV_Doulas_0252 Add
    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_OFF)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if((uc3D_Enable == eCM_3D_FORMAT_AUTO) &&
            ((palImgMgr_Input_3D_Format_Get() == eINPUT_3D_TYPE_FRAMEPACKING) ||
             (palImgMgr_Input_3D_Format_Get() == eINPUT_3D_TYPE_TOPANDBOTTOM) ||
             (palImgMgr_Input_3D_Format_Get() == eINPUT_3D_TYPE_SIDEBYSIDE)))       //A70LV_Doulas_0252 3D 1.4a and auto mode
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
	#endif
}

eFUNC_CONTROL palDataMgr_Control_3D_SyncInSelect_Depend(void)         //A70LV_Doulas_0239
{
    UINT8 uc3D_Enable = 0;
    palDataMgr_Access_3DEnable(edaREAD, &uc3D_Enable);  //A70LV_Doulas_0252 Add
    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_OFF)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
    else if((uc3D_Enable == eCM_3D_FORMAT_AUTO)||
            (uc3D_Enable == eCM_3D_FORMAT_FRAME_PACKING)||
            (uc3D_Enable == eCM_3D_FORMAT_SIDE_BY_SIDE)||
            (uc3D_Enable == eCM_3D_FORMAT_TOP_AND_BOTTOM))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_InputKey(void) //A70LV_Larry_0298
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }
#ifndef SCALER_FPGA_F34
    else if((palDataMgr_Control_BackupInputSW() == eFUNC_CONTROL_ENABLE) && (ets_ON == m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucBackupIputSwitch))
    {
    	 return eFUNC_CONTROL_DISABLE;
    }
#endif /* SCALER_FPGA_F34 */
#if defined(CUSTOM_BARCO) //G100_Tim_0057, add, start    // no source change when the Auto HDMI switch is on //A35G2_BRC_Casper_0060
    else if( palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_ON)
    {
    	 return eFUNC_CONTROL_DISABLE;
    }
#endif
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Source_PictureMode_Blend(void) //A70LV_Larry_0327
{
    UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(palDataPath_IsSourceLock() == TRUE)  //G100_Simon_0012
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_BLENDING)
        {
        	if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER && //G100_Steven_0032
               m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_BLENDING)
        			return eFUNC_CONTROL_SOURCE_DEPEND;
        	else
        		 return eFUNC_CONTROL_ENABLE;
        }
    }
    else
    {
        if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
        {
            if(palDataPath_IsSourceLockSub() == TRUE)  //G100_Simon_0012
            {
                if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_BLENDING)
                {
                	if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER && //G100_Steven_0032
                       	 m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_BLENDING)
                			return eFUNC_CONTROL_SOURCE_DEPEND;
                	else
                		 return eFUNC_CONTROL_ENABLE;
                }
            }
        }
    }
    return eFUNC_CONTROL_SOURCE_DEPEND;
}

eFUNC_CONTROL palDataMgr_Control_WhitePeaking(void)  //A35G2_CDS_Coda_0016
{
    UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(palDataPath_IsSourceLock() == TRUE)
    {
        if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_BLENDING) &&
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_2DHIGHSPEED) &&
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_3D) &&
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_3D_PASSIVE))
        {
        	if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER) && //G100_Steven_0032
               ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_BLENDING) ||
                (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED) ||
                (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_3D) ||
                (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_3D_PASSIVE)))
        			return eFUNC_CONTROL_SOURCE_DEPEND;
        	else
        		 return eFUNC_CONTROL_ENABLE;
        }
    }

    return eFUNC_CONTROL_SOURCE_DEPEND;
}

eFUNC_CONTROL palDataMgr_Control_Gamma(void) 		//G100_Doulas_0046 Add
{
    UINT8 ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;
	UINT8 ucVal = 0;

	palDataMgr_Access_HDRAutoEnable(edaREAD, &ucVal);

    if(palDataPath_IsSourceLock() == TRUE)  //G100_Simon_0014
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_BLENDING)
        {
        	if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER && //G100_Steven_0032
               	 m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_BLENDING)
            {
        			return eFUNC_CONTROL_SOURCE_DEPEND;
            }
#if defined(PLATFORM_H30_4K)
			else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_3D ||
				(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER &&
               	 m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_3D))
            {
        			return eFUNC_CONTROL_SOURCE_DEPEND;
            }
#endif
        	else
        	{
        	    #if 1
        	    if((ucVal == ets_OFF) || (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_SDR))
        		{
        			return eFUNC_CONTROL_ENABLE;
        		}

                if(palDataMgr_IS_3D_Enable() == ets_ON)
                {
        		    return eFUNC_CONTROL_ENABLE;
        		}

        		#else
        		return eFUNC_CONTROL_ENABLE;	//A35G2_CDS_Coda_0015
        		#endif
        	}
        }
    }
    #if 0 //A35G2_CDS_CODA_0009 : when pip on , gray out same as main source no signal.
    else
    {
        if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
        {
            if(palDataPath_IsSourceLockSub() == TRUE)  //G100_Simon_0014
            {
                if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode != eCM_PICTURE_SETTINGS_BLENDING)
                {
                	if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER && //G100_Steven_0032
                       	 m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode == eCM_PICTURE_SETTINGS_BLENDING)
                    {
                			return eFUNC_CONTROL_SOURCE_DEPEND;
                	}
                	else
                		 return eFUNC_CONTROL_ENABLE;
                }
            }
        }
    }
    #endif
    return eFUNC_CONTROL_SOURCE_DEPEND;
}


eFUNC_CONTROL palDataMgr_Control_LD_Info(void)
{
	if(palSystem_ModelIDGet() - MODEL_ID_0) //LV
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else //LS
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_HDREnable(void) //A70LV_Larry_0072
{
    UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(((ucInputSource == eCM_SOURCE_HDMI1) || (ucInputSource == eCM_SOURCE_HDMI2) || (ucInputSource == eCM_SOURCE_HDBASET)) && //G100_Doulas_0046 Modify
    //    (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable == ets_OFF) &&  	//G100_Doulas_0074 remove
        (palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
        (palDataMgr_IS_3D_Enable() == ets_OFF))	//T100_Clare_0003
    {
        return eFUNC_CONTROL_ENABLE;
    }
//#if defined(CUSTOM_BARCO) //H30K_Doulas_0056//A35G2_BRC_Casper_0101
	else if((palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE) && (utilCounterGet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR) != 0))
	{
        return eFUNC_CONTROL_ENABLE;
	}
//#endif
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_HDR_Status(void)       //G100_Owen_0034
{
	UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;	//G100_Doulas_0046

    if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    /*else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != ets_OFF)	//G100_Doulas_0074 remove//G100_Doulas_0046 Add
	{
		return eFUNC_CONTROL_DISABLE;
	}*/
    else if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHDREnable == ets_OFF) ||
        (palImgMgr_HDR_Get(eSOURCE_WINDOW_MAIN) == eHDR_SETTING_SDR))
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palDataMgr_IS_3D_Enable() == ets_ON)
    {
        return eFUNC_CONTROL_DISABLE;
    }
	else if((ucInputSource != eCM_SOURCE_HDMI1) && (ucInputSource != eCM_SOURCE_HDMI2) && (ucInputSource != eCM_SOURCE_HDBASET))  //G100_Doulas_0046 Add
	{
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_BackupInputSW(void) //G100_Steven_0054
{
#ifdef SCALER_FPGA_F34
    //UINT8 ucBackupInput = ets_MAX_NUMBER;
    //appDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, (PUINT8)&ucBackupInput);

    //if((appDataMgr_Control_BackupInput() == eFUNC_CONTROL_ENABLE) && (ucBackupInput == ets_ON) &&
       //(halScaler_BackupPrimaryInput_Get() != halScaler_BackupSecondaryInput_Get()))
    //{
        //return eFUNC_CONTROL_ENABLE;
    //}

    if(palDataMgr_Control_BackupInput() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    return eFUNC_CONTROL_DISABLE;
#else
    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != 0) ||
        (palDataMgr_IS_3D_Enable() == ets_ON) ||
        (palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_3D_INPUT_DEPEND)) //A35G2_BRC_Casper_0075
    {
        return eFUNC_CONTROL_DISABLE;
    }

#if defined(CUSTOM_BARCO) //G100_Tim_0057, add, start // no Backup Input when Auto HDMI Switch is on //A35G2_BRC_Casper_0060
    if( palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_ON )
    {
        return eFUNC_CONTROL_DISABLE;
    }
#endif

    return eFUNC_CONTROL_ENABLE;

#endif /* SCALER_FPGA_F34 */

} //G100_Steven_0046 end

eFUNC_CONTROL palDataMgr_Control_BackupInput(void) //G100_Steven_0054
{
#ifdef SCALER_FPGA_F34
    if((palSystem_PanelID_Get() != PANEL_3D_OUTPUT) &&
       (palSystem_PanelID_Get() != PANEL_2D_HIGHSPEED) &&
       (palDataMgr_IS_3D_Enable() == ets_OFF) &&
       (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable == ets_OFF))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#else
    if(palDataMgr_Control_BackupInputSW() != eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
#endif /* SCALER_FPGA_F34 */
}

eFUNC_CONTROL palDataMgr_Control_BackupInput_MSRC_Depend(void) //A35G2_BRC_Casper_0075
{
    UINT8 ucBackupInput = ets_MAX_NUMBER;
    palDataMgr_Access_BackupIputSwitch(edaREAD, (PUINT8)&ucBackupInput); //HICC2_Steven_0064 fixed ISS-0026341 //m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucBackupIputSwitch;

    if((ucBackupInput == ets_OFF) || (palDataMgr_Control_BackupInputSW() != eFUNC_CONTROL_ENABLE) || (palDataPath_BKInput_First_Input_Ready() != TRUE))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_BackupInput_SSRC_Depend(void) //A35G2_BRC_Casper_0075
{
    UINT8 ucBackupInput = ets_MAX_NUMBER;
    palDataMgr_Access_BackupIputSwitch(edaREAD, (PUINT8)&ucBackupInput); //HICC2_Steven_0064 fixed ISS-0026341  //m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucBackupIputSwitch;

    if((ucBackupInput == ets_OFF) || (palDataMgr_Control_BackupInputSW() != eFUNC_CONTROL_ENABLE) || (palDataPath_BKInput_Second_Input_Ready() != TRUE))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_DynamicBlack(void)     //A65_Owen_0006
{
    UINT8 ucVal = 0;
	UINT8 ucConstantBrightness = 0;	//G100_Doulas_0021

    palDataMgr_Access_PowerMode(edaREAD, &ucVal);
	palDataMgr_Access_Constant_Brightness(edaREAD, &ucConstantBrightness);	//G100_Doulas_0021
    #if defined(CUSTOM_CHRISTIE)
	if((ucVal != eCM_POWER_MODE_CONSTANT_INTENSITY) &&
    #elif defined(CUSTOM_OPTOMA)
    if((ucVal == eCM_POWER_MODE_CONSTANT_POWER) &&    //H30K_Tim_0015, mod //H30K_Doulas_0064 Modify//H30K_Tim_0010, mod
	#else
    if(((ucVal != eCM_POWER_MODE_CONSTANT_POWER)||(ucConstantBrightness != ets_ON)) &&		//G100_Doulas_0021 Modify
    #endif
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement != eCONTRAST_ENHANCE_REAL_BLACK))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_RealBlack(void)     //A65_Owen_0006
{
    UINT8 ucVal = 0;
	UINT8 ucConstantBrightness = 0;	//G100_Doulas_0021

    palDataMgr_Access_PowerMode(edaREAD, &ucVal);
	palDataMgr_Access_Constant_Brightness(edaREAD, &ucConstantBrightness);	//G100_Doulas_0021
    #if defined(CUSTOM_CHRISTIE)
	if((ucVal != eCM_POWER_MODE_CONSTANT_INTENSITY) &&
    #elif defined(CUSTOM_OPTOMA)
    if(((ucVal != eCM_POWER_MODE_CUSTOM_MODE)||(ucConstantBrightness != ets_ON)) &&  //A35G2_Owen_0003
	#else
    if(((ucVal != eCM_POWER_MODE_CONSTANT_POWER)||(ucConstantBrightness != ets_ON)) &&		//G100_Doulas_0021 Modify
    #endif
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP) &&
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement != eCONTRAST_ENHANCE_DYNAMIC_BLACK))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_DynamicBlack_Enable(void)     //A65_Owen_0006
{
#if defined(PLATFORM_H30_4K)
	if(palDataMgr_getDimStatus() != DIM_POWER_NUMBER_DEFAULT_VALUE)
	{
        return eFUNC_CONTROL_DISABLE;
    }
#endif
	if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_DYNAMIC_BLACK) &&
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_RealBlack_Enable(void)     //A65_Owen_0006
{
#if defined(PLATFORM_H30_4K)
	if(palDataMgr_getDimStatus() != DIM_POWER_NUMBER_DEFAULT_VALUE)
	{
        return eFUNC_CONTROL_DISABLE;
    }
#endif
    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_REAL_BLACK) &&
        (palDataPath_GetDataPathState() != eDATA_PATH_STATE_SPLASH_AT_STARTUP))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_EdgeMask(void)       //ZU860_Doulas_0004
{
    UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(palDataMgr_Control_Scaler() == eFUNC_CONTROL_SOURCE_DEPEND)
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalHorzZoom == DIGITAL_HORZ_ZOOM_DEFAULT) &&
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].uiDigitalVertZoom == DIGITAL_VERT_ZOOM_DEFAULT))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_ConstantBrightness(void) //G100_Doulas_0008
{
#if defined(PLATFORM_H30_4K)
	if(palDataMgr_getDimStatus() != DIM_POWER_NUMBER_DEFAULT_VALUE)
	{
		return eFUNC_CONTROL_DISABLE;
	}
#endif
#if(defined(CUSTOM_BARCO)) //A35G2_Coda_0043
	if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF)
	&& (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucPowerMode == eCM_POWER_MODE_CUSTOM_MODE))
#elif(defined(CUSTOM_OPTOMA))
	UINT8 ucValue = 0;
	palDataMgr_Access_ConstantPower(edaREAD, &ucValue);
	if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF)
	&& (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucPowerMode == eCM_POWER_MODE_CUSTOM_MODE)
	&& ((ucValue < 80) && (ucValue >= 30)))
#else
	if( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF &&
		m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucPowerMode == eCM_POWER_MODE_CONSTANT_POWER )
#endif
	{
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_CONTRAST_ENHANCEMENT_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_LensMemoryApply(void)        //G100_Owen_0030
{
    UINT8 ucData = 0;

    //if( eLENS_TYPE_NA == m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType ) //G100_Steven_0045
    ///{
    	//return eFUNC_CONTROL_DISABLE;
    //}

    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)    //G100_Owen_0041
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW)
        {
            if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100) ||
             (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
            && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1) //1:unlock ; 0: lock
            )
            {
               return eFUNC_CONTROL_DISABLE;
            }

            if(palEnvironment_LensMemorySavingCondition_Get() == 0)
            {
                return eFUNC_CONTROL_DISABLE;
            }

            if(palEnvironment_LensSpecialFlag_Get()) //HICC2_Doulas_0138 run lens special location
            {
                return eFUNC_CONTROL_DISABLE;
            }

            if(palEnvironment_LensCalFlag_Get() == FALSE) //HICC2_Doulas_0138 lens cal or lens moving
            {
                return eFUNC_CONTROL_DISABLE;
            }

            #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
            if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
               palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
               palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
               palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
            {
                return eFUNC_CONTROL_DISABLE;
            }
            #endif

            return eFUNC_CONTROL_ENABLE;
        }
        else
        {
            return eFUNC_CONTROL_LENS_LOCK_DEPEND;
        }
    }

    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_3D_ModeDepend(void)         //G100_Doulas_0020 //G100_Casper_0003
{
#ifdef SCALER_FPGA_F34
    UINT8 uc3D_Enable = eCM_3D_FORMAT_OFF;

    palDataMgr_Access_3DEnable(edaREAD, &uc3D_Enable);
    //palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, (PUINT8)&ucBackupInput);

    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if((palDataMgr_Control_BackupInput() == eFUNC_CONTROL_ENABLE) && (palImgMgr_BackupSwitchGet() == ets_ON) &&
            (halScaler_BackupPrimaryInput_Get() != halScaler_BackupSecondaryInput_Get()))
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else if(uc3D_Enable == eCM_3D_FORMAT_4K3D ||
            uc3D_Enable == eCM_3D_FORMAT_4K3D_DUALPIPE)
    {
        return eFUNC_CONTROL_NO_3D_DEPEND;
    }
#if 1 //A70LK_Larry_0180
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        if(utilCounterGet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR) == 0) //for OSD transient //A35G2_CDS_Larry_0055
        {
            return eFUNC_CONTROL_SOURCE_DEPEND;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
    }
#endif
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }

#else
	UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;	//G100_Doulas_0020
	BYTE ucLow_Latency = eCM_LOW_LATENCY_MODE_OFF;
	//palDataMgr_Data_Access(edcLOW_LATENCY_MODE, edaREAD, &ucLow_Latency);
    palDataMgr_Access_LowLatencyMode(edaREAD, &ucLow_Latency);

    if(palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE)
    {
        return eFUNC_CONTROL_PIP_PBP_DEPEND;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
    {
        if((eCM_3D_FORMAT_FRAME_SEQUENTIAL == m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].uc3D_Enable) &&
            (eSOURCE_STATE_SIGNAL_OUT_OF_RANGE == m_sAppDataMgrInfo.sSystemValues.sSOURCE_INFO.ucSourceState[0]))
        {
            return eFUNC_CONTROL_ENABLE;
        }

        if(utilCounterGet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR) == 0) //for OSD transient //A35G2_BRC_Casper_0101
        {
            return eFUNC_CONTROL_SOURCE_DEPEND;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
    }
    else if(ucLow_Latency != eCM_LOW_LATENCY_MODE_OFF)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
#endif /* SCALER_FPGA_F34 */
}

eFUNC_CONTROL palDataMgr_Control_ImageFreeze(void) //G100_Coda_0001
{
    UINT8 ucFactory_TestPattern = 0;
#ifdef Low_Latency_All
	BOOL ucLow_Latency = TRUE;

	palDataMgr_Access_LowLatencyMode(edaREAD, &ucLow_Latency);

    if(((palDataPath_GetDataPathState() == eDATA_PATH_STATE_MONITOR_SOURCE)
     || (palDataPath_GetDataPathState() == eDATA_PATH_STATE_ATTEMPT_LOCK)
     || (palDataPath_GetPanelChangeState() == ets_ON))
#ifndef SCALER_FPGA_F34
        && (!ucLow_Latency)
#endif /* SCALER_FPGA_F34 */
        && (palDataMgr_CurTestPatternGet() == eTID_OFF) //HICC2_Doulas_0135
        )
	#else
	if((palDataPath_Main_Or_Sub_SourceMonitor() == TRUE)
	    && (palDataMgr_CurTestPatternGet() == eTID_OFF)) //HICC2_Doulas_0135
	    //|| (palDataPath_GetDataPathState() == eDATA_PATH_STATE_ATTEMPT_LOCK && GuiCb.fpGui_IsInstallatioon_Menu_L1Cb() == TRUE))
	#endif	/*Low_Latency_All*/
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {

        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_ApplyUserDepend(void)         //G100_Doulas_0066
{
	UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if((palDataPath_IsSourceLock() == TRUE) &&
	   (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_USER))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

//G100_Tim_0057, add, start //A35G2_BRC_Casper_0060
// ==============================================================================
// DESCRIPTION: Control Auto HDMI Switch Enable/Disable check: no 3D, no PIP/PBP, no Backup Input, no pin enable
// Params: None
// Returns: Control Enable or Disable
//
// Modification History
// --------------------
// 2021/09/01, Tim Chen create
// --------------------
// ==============================================================================
#if (AUTO_HDMI_SWITCH_DETECT_5V == TRUE)
eFUNC_CONTROL palDataMgr_Control_Auto_HDMI_Switch(void)  //A35G2_BRC_Simon_0008
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    UINT8 ucPIPEn = eCM_SCREEN_MODE_NUMBER;
    eEXEC_CODE ePIPResult = palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, (PUINT8)&ucPIPEn);

    UINT8 ucBackupInput = ets_MAX_NUMBER;
    eEXEC_CODE eBackupInputResult = palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, (PUINT8)&ucBackupInput);

    if(
        ( ( ePIPResult == eEXEC_CODE_PASS ) && ( ucPIPEn == eCM_SCREEN_MODE_OFF ) )  &&     // Neither PIP nor PBP
        ( ( eBackupInputResult == eEXEC_CODE_PASS ) && ( ucBackupInput == ets_OFF ) )  &&   // Backup Input off
        (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)                     // PIN protect OK
    ) //A35G2_BRC_Casper_0075
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

#else
eFUNC_CONTROL palDataMgr_Control_Auto_HDMI_Switch(void)
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    UINT8 uc3DEn = ets_ON;
    eEXEC_CODE e3D_En_Result = palDataMgr_Data_Access(edc3D_ENABLE, edaREAD, (PUINT8)&uc3DEn);

    UINT8 uc3DMode = eCM_3D_FORMAT_NUMBER;
    eEXEC_CODE e3D_Mode_Result = palDataMgr_Data_Access(edc3D_MODE, edaREAD, (PUINT8)&uc3DMode);

    UINT8 ucPIPEn = eCM_SCREEN_MODE_NUMBER;
    eEXEC_CODE ePIPResult = palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, (PUINT8)&ucPIPEn);

    UINT8 ucBackupInput = ets_MAX_NUMBER;
    eEXEC_CODE eBackupInputResult = palDataMgr_Data_Access(edcBACKUPINPUT_AUTOSWITCH, edaREAD, (PUINT8)&ucBackupInput);

    if( ( ( e3D_En_Result == eEXEC_CODE_PASS ) &&
           ( ( uc3DEn == ets_OFF ) ||                // 3D disabled
             ( ( uc3DEn == ets_ON ) && ( e3D_Mode_Result == eEXEC_CODE_PASS ) && ( uc3DMode == eCM_3D_FORMAT_AUTO )  )  ) )&&  // On & Auto
        ( ( ePIPResult == eEXEC_CODE_PASS ) && ( ucPIPEn == eCM_SCREEN_MODE_OFF ) )  &&     // Neither PIP nor PBP
        ( ( eBackupInputResult == eEXEC_CODE_PASS ) && ( ucBackupInput == ets_OFF ) )  &&   // Backup Input off
        (palDataMgr_Control_3D_InputDepend() == eFUNC_CONTROL_ENABLE) &&
        (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)                     // PIN protect OK
    ) //A35G2_BRC_Casper_0075
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}
#endif
// ==============================================================================
// DESCRIPTION: Control Main source switch enable or disable
// Params: None
// Returns: Control Enable or Disable
//
// Modification History
// --------------------
// 2021/09/01, Tim Chen create //A35G2_BRC_Casper_0060
// --------------------
// ==============================================================================
eFUNC_CONTROL palDataMgr_Control_Main_Src_Depend(void)
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_TEST_PATTERN_DEPEND;

    if( ( palDataMgr_Control_TestPatternDepend() == eFUNC_CONTROL_ENABLE )      // no test pattern
        && ( palDataMgr_Auto_HDMI_Switch_RAM_Get() == ets_OFF )                 // no Auto HDMI Switch on
    )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_3D_Aspect_Ratio(void)  //G100_Steven_0163 //A35G2_BRC_Casper_0146
{
	if( (palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE) &&
		(palDataMgr_Control_PIP_PBP_Enable() != eFUNC_CONTROL_ENABLE)&&
		( palDataMgr_IS_3D_Enable() == ets_ON || (palSystem_PanelID_Get() >= ePANEL_ID_XGA_120HZ && palSystem_PanelID_Get() <= ePANEL_ID_WUXGA_120HZ)))
	{
		 return eFUNC_CONTROL_ENABLE;
	}

    else
    {
        return eFUNC_CONTROL_DISABLE; //A35G2_Jacky_0037
    }

}

eFUNC_CONTROL palDataMgr_Control_Low_Latency_Mode(void)	//ZU860_Doulas_0131 Add
{
#ifdef Low_Latency_All

#ifdef SCALER_FPGA_F34
	if(palDataPath_IsSourceLock() == TRUE)
    {
		UINT8  ucBackupIputSwitch = 0;
        UINT8 uc3DMode = halScaler_3DEnable_Get();

		palDataMgr_Access_BackupIputSwitch(edaREAD, &ucBackupIputSwitch);
        if(ucBackupIputSwitch == ets_ON)
        {
            return eFUNC_CONTROL_DISABLE;
        }

        //if(appDataMgr_IS_3D_Enable() == ets_ON)
        //{
            //return eFUNC_CONTROL_3D_INPUT_DEPEND;
        //}

        //if((uc3DMode == eCM_3D_FORMAT_DUALPIPE_3D) ||
        //   (uc3DMode == eCM_3D_FORMAT_4K3D_DUALPIPE))
        //{
        //    return eFUNC_CONTROL_DISABLE;
        //}

        if(halScaler_PIP_PBP_Enable_Get() != eCM_SCREEN_MODE_OFF)
        {
            return eFUNC_CONTROL_DISABLE;
        }

        if(halScaler_LowLatencyEnable() == FALSE)
        {
            return eFUNC_CONTROL_SOURCE_DEPEND;
        }

        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
#else
    UINT8 bPIPEn =0, b3DEn =0, b4K=0, b2xHz = 0;	   //G100_Clare_0057	//G100_Clare_0053
	UINT16 wPanelID = ePANEL_ID_WUXGA_60HZ;	//G100_Clare_0062
    char uc24Hz[VERSION_STRING_MAX_LENGTH] = {0};   //G100_Clare_0057
	//Source
	if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)
	{
		#if 1//#if defined(CUSTOM_BARCO) //HICC2_Doulas_0148//A35G2_BRC_Casper_0098
        if(utilCounterGet(eCOUNTER_TYPE_OSD_GRAY_OUT_MONITOR) == 0) //A35G2_BRC_Casper_0101
        {
            return eFUNC_CONTROL_SOURCE_DEPEND;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
		#else
		return eFUNC_CONTROL_SOURCE_DEPEND;
		#endif
	}
	//PIP/PBP
	palDataMgr_Data_Access(edcPIP_PBP_ENABLE, edaREAD, &bPIPEn);

    //Panel 120Hz
    wPanelID = palSystem_PanelID_Get();	//G100_Clare_0062

	//3D
	b3DEn = palDataMgr_IS_3D_Enable();//palImgMgr_Aspect_RatioIs_3D_Mode_Get();

	//4K
	b4K = FrontEnd_Down_Scaling_Get();	//G100_Clare_0053

	//2xHz   //G100_Clare_0057
	palDataMgr_Data_Access(edcMAIN_VERT_REFRESH, edaREAD, &uc24Hz);	//G100_Coda_00107
    if((uc24Hz[0] == '2') && (uc24Hz[2] == '.'))
    {
        b2xHz = TRUE;
    }

    #if defined (CUSTOM_OPTOMA) //H30K_Doulas_0018
    if(bPIPEn || b3DEn || b4K || b2xHz || (wPanelID == PANEL_3D_OUTPUT))
    #else
    if(bPIPEn || b3DEn || b4K || b2xHz || (wPanelID == PANEL_3D_OUTPUT) || (wPanelID == PANEL_2D_HIGHSPEED))	//G100_Clare_0062   //G100_Clare_0057	//G100_Clare_0053
    #endif
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
#endif /* SCALER_FPGA_F34 */
#else
    return eFUNC_CONTROL_ENABLE;
#endif	/*Low_Latency_All*/

}


eFUNC_CONTROL palDataMgr_Control_Background_LogoDepend(void)         //G100_Owen_0048
{
    UINT8 ucBgColor = 0;

    palDataMgr_Access_BackGround_Color(edaREAD, &ucBgColor);
    if(ucBgColor != eCM_BACKGROUND_COLOR_LOGO)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_LogoCaptured_Depend(void)         //G100_Owen_0061
{
    if((palImgMgr_Input_3D_Format_Get() != ets_OFF) || (palDataMgr_Control_PIP_PBP_Enable() == eFUNC_CONTROL_ENABLE) ||   //G100_Owen_0072 //A35G2_BRC_Casper_0036
        (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLowLatency != eCM_LOW_LATENCY_MODE_OFF) ||
        (palSystem_PanelID_Get() == PANEL_3D_OUTPUT) || (palSystem_PanelID_Get() == PANEL_2D_HIGHSPEED)
      #if defined (CUSTOM_OPTOMA)       //A35G2_Tim_0003, add, start
         || ( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucBackgroundColor != eCM_BACKGROUND_COLOR_LOGO )
      #endif                            //A35G2_Tim_0003, add, end
        /* || (GuiCb.fpGui_IsWait_Message_MenuCb() == TRUE)*/)   //G100_Owen_0104 //A35G2_BRC_Casper_0051 //A35G2_BRC_Casper_0093 //A35G2_BRC_Casper_0095 //###
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else if(palDataMgr_Control_SourceDepend() != eFUNC_CONTROL_ENABLE)      //G100_Owen_0060
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}


eFUNC_CONTROL palDataMgr_Control_VGA_Depend(void) //G100_Simon_0038
{
    UINT8 ucMainInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;
    UINT8 ucSubInputSource  = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceSub;

    if(palDataPath_IsSourceLock() == TRUE && ucMainInputSource == eCM_SOURCE_VGA)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else if(palDataPath_IsSourceLockSub() == TRUE && ucSubInputSource == eCM_SOURCE_VGA)
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}


eFUNC_CONTROL palDataMgr_Control_DimmingMode(void)
{
    UINT8 ucDimFlag = 0;
	palDataMgr_Data_Access(edcDIMMING_MODE, edaREAD, &ucDimFlag);

   if(ucDimFlag)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_Basic_Warp(void)    //A35G2_Simon_0069
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CONTROL_BASIC_WARP   )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_Advanced_Warp(void)
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CONTROL_ADVANCED_WARP )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_ADV_BlackLevelEnable(void)			//A65_OPTOMA_Doulas_0020
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
	UINT8 ucBlackLevelArea = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelArea;

    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__ADVANCED) &&
	   (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelEnable[ucBlackLevelArea] == ets_ON))
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_ADV_BlackLevelAddPoint(void)			//A65_OPTOMA_Doulas_0020
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
	UINT8 ucBlackLevelArea = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelArea;

    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__ADVANCED) &&
	   (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelEnable[ucBlackLevelArea] == ets_ON) &&
	   (halAdvBlackLevelIsPointAddable() == TRUE))
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_ADV_BlackLevelRemovePoint(void)			//A65_OPTOMA_Doulas_0020
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
	UINT8 ucBlackLevelArea = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelArea;

    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__ADVANCED) &&
	   (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelEnable[ucBlackLevelArea] == ets_ON) &&
	   (halAdvBlackLevelIsPointDeletable() == TRUE))
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_QUICK_KEY(void)		//A65_OPTOMA_Doulas_0025//A35G2_Coda_0049
{
    if(m_sAppDataMgrInfo.sSystemValues.ucQuickKeyLock > 0)
    {
         return(eFUNC_CONTROL_DISABLE);
    }
    return(eFUNC_CONTROL_ENABLE);
}

eFUNC_CONTROL palDataMgr_Control_Model_Switch_Adjust(void) //A65_OPTOMA_Julie_0064 //A35G2_Coda_0101
{
    if(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucModelSwitchAdjust == eMODEL_SWITCH_NEUTRAL)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_ATW_Set(void) //A35G2_Alan_0001
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
    UINT8 ucCamera_Status = m_sAppDataMgrInfo.sSystemValues.ucCamera_Module_Status;;
	ePANEL_ID ePanelId = palSystem_PanelID_Get();	        //G100_Clare_0024, add
	UINT8 uc3DEn = palDataMgr_IS_3D_Enable();               //G100_Clare_0024, add

    if( ( ucCamera_Status == eCAMERA_MODULE_EXIST ) &&
        ( (ePanelId != PANEL_3D_OUTPUT) && (ePanelId != PANEL_2D_HIGHSPEED) ) &&           //G100_Clare_0024, add
        ( uc3DEn == ets_OFF ) &&        //G100_Clare_0024, add
        (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) &&
         ( m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_AUST ) //A70LK_Jacky_0026
        #ifdef SCALER_FPGA_F34
        && ( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == TRUE )  //A70LK_Simon_0015
        && (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D)   //A70LK_Larry_0180 //H2PF_Simon_0041
        #endif
        )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

	return(eRet);
}

eFUNC_CONTROL palDataMgr_Control_ACU_Calibration(void) //A35G2_Alan_0001
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
    UINT8 ucCamera_Status = m_sAppDataMgrInfo.sSystemValues.ucCamera_Module_Status;;
	ePANEL_ID ePanelId = palSystem_PanelID_Get();	        //G100_Clare_0024, add
	UINT8 uc3DEn = palDataMgr_IS_3D_Enable();               //G100_Clare_0024, add

    if( ( ucCamera_Status == eCAMERA_MODULE_EXIST ) &&
        ( (ePanelId != PANEL_3D_OUTPUT) && (ePanelId != PANEL_2D_HIGHSPEED) ) &&           //G100_Clare_0024, add
        ( uc3DEn == ets_OFF ) &&        //G100_Clare_0024, add
        (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK)  &&     //G100_Coda_00124
        (palDataMgr_CU_Data_Enable_Get()) &&
         ( m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_AUST) //A70LK_Jacky_0026
        #ifdef SCALER_FPGA_F34
        && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == TRUE )    //A70LK_Simon_0015
        && (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D) //A70LK_Larry_0180   //H2PF_Simon_0041
        #endif
        )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

	return(eRet);
}

eFUNC_CONTROL palDataMgr_Control_WARP_Reset(void) //A70Gen2_Steven_0004 fix ISS-0008136
{
    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE && palGeo_ApLinkFlag_Get() == 0)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_ACU_Reset(void) //A35G2_Alan_0001
{
    if(palDataMgr_CU_Data_Enable_Get() &&
      //((ePanelId != ePANEL_ID_1080P_120HZ) && (ePanelId != ePANEL_ID_WUXGA_120HZ) && (ePanelId != PANEL_2D_HIGHSPEED))  &&   //A70LK_Jacky_0022
#ifdef SCALER_FPGA_F34
      (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D) && //A70LK_Larry_0180
#endif /* defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)  */
      ( m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_AUST) //A70LK_Jacky_0026
      #ifdef SCALER_FPGA_F34
      && ( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == TRUE )    //A70LK_Simon_0015
      #endif
      )
    {
        return eFUNC_CONTROL_ENABLE;
    }
    return eFUNC_CONTROL_DISABLE;
}


eFUNC_CONTROL palDataMgr_Control_Light_Sensor_Cal_Depend(void) 	//A70Gen2_Doulas_0015
{
	UINT8 ucVal = 0;
	//UINT8 ucConstantBrightness = 0;
	palDataMgr_Access_PowerMode(edaREAD, &ucVal);
	//appDataMgr_Access_Constant_Brightness(edaREAD, &ucConstantBrightness);

	if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
	{
		if((palDataPath_Last3DEnable_Get() == FALSE) &&    //3D off
		   //((ucVal != eCM_POWER_MODE_CUSTOM_MODE)||(ucConstantBrightness != ets_ON)) &&
		   (ucVal != eCM_POWER_MODE_CONSTANT_INTENSITY) &&
	       (palSystem_PanelID_Get() != PANEL_3D_OUTPUT) &&
	       (palSystem_PanelID_Get() != PANEL_2D_HIGHSPEED) )
	    {
	        return eFUNC_CONTROL_ENABLE;
	    }
	    else
	    {
	        return eFUNC_CONTROL_DISABLE;
	    }

    }
	else
	{
    	return eFUNC_CONTROL_DISABLE;
	}
}


//Is 3D     : return eFUNC_CONTROL_ENABLE
//Is not 3D : return eFUNC_CONTROL_DISABLE
eFUNC_CONTROL palDataMgr_Control_IS_3D_Enable(void)
{
    if(palDataMgr_IS_3D_Enable())
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}


//ACU_Target_Status on   : return eFUNC_CONTROL_ENABLE
//ACU_Target_Status off  : return eFUNC_CONTROL_DISABLE
eFUNC_CONTROL palDataMgr_ACU_Target_Status_Enable(void)
{
    if(palDataMgr_ACU_Target_Status_Get())
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}

//Camera is working     : return eFUNC_CONTROL_DISABLE
//Camera is not working : return eFUNC_CONTROL_ENABLE
eFUNC_CONTROL palDataMgr_Control_Camera_Working_Enable(void) //HICC2_Doulas_0123
{
    if(palDataMgr_Get_Camera_Working_Status() == eCAMERA_STATUS_WORKING)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyWarpMemory1(void)
{
    if(palDataMgr_ApplyWarpMemoryItemAvailable(1) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyWarpMemory2(void)
{
    if(palDataMgr_ApplyWarpMemoryItemAvailable(2) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyWarpMemory3(void)
{
    if(palDataMgr_ApplyWarpMemoryItemAvailable(3) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyWarpMemory4(void)
{
    if(palDataMgr_ApplyWarpMemoryItemAvailable(4) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyWarpMemory5(void)
{
    if(palDataMgr_ApplyWarpMemoryItemAvailable(5) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyBlendMemory1(void)
{
    if(palDataMgr_ApplyBlendMemoryItemAvailable(1) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyBlendMemory2(void)
{
    if(palDataMgr_ApplyBlendMemoryItemAvailable(2) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyBlendMemory3(void)
{
    if(palDataMgr_ApplyBlendMemoryItemAvailable(3) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyBlendMemory4(void)
{
    if(palDataMgr_ApplyBlendMemoryItemAvailable(4) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_ApplyBlendMemory5(void)
{
    if(palDataMgr_ApplyBlendMemoryItemAvailable(5) == 0)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_BlendAPCmdEnable_Control(void)
{
    if((palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE) &&
       (palEnvironment_Fake_Power_Down_Get() == FALSE) &&
       (palGeo_ApLinkFlag_Get() == TRUE)
    )
    {
        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_ColorSpaceSettingRGB(void)
{
    if(palDataPath_IsSourceLock() == TRUE)
    {
        UINT8 ucValue = 0;
        palDataMgr_Access_ColorSpace(edaREAD, &ucValue);

        if(ucValue != eCM_COLOR_SPACE_AUTO)
        {
#ifdef SCALER_FPGA_F34
            if(palImgMgr_PixModIsYUV420(eSOURCE_WINDOW_MAIN, palImgMgr_InputSource_Get(eSOURCE_WINDOW_MAIN)) == FALSE)
            {
                return eFUNC_CONTROL_ENABLE;
            }
            else
            {
                return eFUNC_CONTROL_DISABLE;
            }
#else
            return eFUNC_CONTROL_ENABLE;
#endif /* SCALER_FPGA_F34 */
        }

        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_SubColorSpaceSettingRGB(void)
{
    if(palDataPath_IsSourceLockSub() == TRUE ||
       palImgMgr_3DEnable_Get() == eCM_3D_FORMAT_DUALPIPE_3D || palImgMgr_3DEnable_Get() == eCM_3D_FORMAT_4K3D_DUALPIPE)
    {
            UINT8 ucValue = 0;
            palDataMgr_Access_SubColorSpace(edaREAD, &ucValue);

            if(ucValue != eCM_COLOR_SPACE_AUTO)
            {
#ifdef SCALER_FPGA_F34
                if(palImgMgr_PixModIsYUV420(eSOURCE_WINDOW_SUB, palImgMgr_InputSource_Get(eSOURCE_WINDOW_SUB)) == FALSE)
                {
                    return eFUNC_CONTROL_ENABLE;
                }
                else
                {
                    return eFUNC_CONTROL_DISABLE;
                }
#else
                return eFUNC_CONTROL_ENABLE;
#endif /* SCALER_FPGA_F34 */
            }

        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
    else
    {
        return eFUNC_CONTROL_SOURCE_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_GeometryEnable(void)
{

    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
#ifdef SCALER_FPGA_F34
        eWAPRING_AP_GROUP ucPanelConfig = palDataPath_PanelConfigGet(); //A70LK_Larry_0180

        if((palGeo_ApLinkFlag_Get() == 1) ||
           (ucPanelConfig == eWAPRING_AP_GROUP_4K3D) ||
           (ucPanelConfig == eWAPRING_AP_GROUP_4K3D_DUALPIPE))
        {
            return eFUNC_CONTROL_DISABLE;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
#else
        return eFUNC_CONTROL_ENABLE;
#endif /* SCALER_FPGA_F34 */


    }
    return eFUNC_CONTROL_DISABLE;
}


//ProAV platform only
eFUNC_CONTROL palDataMgr_Control_ACU_3D_Status(void)  //A70LK_Coda_0010
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    #ifdef SCALER_FPGA_F34   //H2PF_Simon_0041

    if(
#ifdef SCALER_FPGA_F34
        (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D) && //A70LK_Larry_0180
#endif /* defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)  */
        ( palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && //G100_Coda_00124   //HICC2_Simon_0012
        ( m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_TYPE_UST ) //A70LK_Jacky_0026
        #ifdef SCALER_FPGA_F34
        && ( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == TRUE )  //A70LK_Simon_0015
        #endif
    )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    #else

    eRet = eFUNC_CONTROL_ENABLE;   //always enable

    #endif

	return(eRet);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Event_Setting(void)			////G100_Coda_0027 //G100_Coda_0028
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    UINT8 ucUserChoiceWeekDay = m_sAppDataMgrInfo.sSystemValues.ucCurrentWeekday;
    UINT8 ucUserChoiceEventIndex = m_sAppDataMgrInfo.sSystemValues.ucCurrentEventListNum;
    UINT8 ucViewUserEventIndex = m_sAppDataMgrInfo.sSystemValues.sScheduleInfo.sWeekday_Info[ucUserChoiceWeekDay].sWeekDayEvent[ucUserChoiceEventIndex].ucEventIndex;

    if(ucViewUserEventIndex != ets_OFF)
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info(UINT8 ucCheckEventListNum)			 //G100_Coda_0029
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    if(palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    UINT8 ucUserChoiceWeekDay = m_sAppDataMgrInfo.sSystemValues.ucCurrentWeekday;
    UINT8 ucViewUserPreviousEventFunctionType = m_sAppDataMgrInfo.sSystemValues.sScheduleInfo.sWeekday_Info[ucUserChoiceWeekDay].sWeekDayEvent[ucCheckEventListNum - 1].ucFunctionType;
    LOG_MSG(db_HAL_RESERVED16, "ctrl %d = %d,\r\n", ucCheckEventListNum - 1, ucViewUserPreviousEventFunctionType); //HICC2_Doulas_0036
    if(ucViewUserPreviousEventFunctionType != 0)
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK1(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(1);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK2(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(2);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK3(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(3);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK4(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(4);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK5(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(5);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK6(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(6);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK7(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(7);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK8(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(8);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK9(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(9);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK10(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(10);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK11(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(11);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK12(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(12);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK13(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(13);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK14(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(14);
}

eFUNC_CONTROL palDataMgr_Control_Schedule_Show_Event_Info_CK15(void)
{
    return palDataMgr_Control_Schedule_Show_Event_Info(15);
}


eFUNC_CONTROL palDataMgr_Control_WarpingMemory(void)  //H2PF_Simon_0042
{
    if((palDataMgr_Control_WarpingApply() == eFUNC_CONTROL_DISABLE) &&
       (palDataMgr_Control_WarpingSave() == eFUNC_CONTROL_DISABLE))
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_BlendingMemory(void)  //H2PF_Simon_0042
{
    if((palDataMgr_Control_BlendApply() == eFUNC_CONTROL_DISABLE) &&
       (palDataMgr_Control_BlendSave() == eFUNC_CONTROL_DISABLE))
    {
        return eFUNC_CONTROL_DISABLE;
    }
    else
    {
        return eFUNC_CONTROL_ENABLE;
    }

}

eFUNC_CONTROL appDataMgr_Control_TAA_WiFi(void) //R70K_AC_0092 //HICC2_Casper_0013
{
    if(m_sLAN_info.ucWLAN_Link_Status)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL appDataMgr_Control_LensSpecialLocation(void) //A70Gen2_Julie_0109//HICC2_Julie_0014
{
    if(palEnvironment_LensCalFlag_Get() == FALSE)
    {
    	return eFUNC_CONTROL_DISABLE;
    }
    return(palDataMgr_Control_Lens_Lock());
}

eFUNC_CONTROL palDataMgr_Control_AutoColorMatch(void) ////HICC2_Ashton_0041//HICC2_Ashton_0004
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
    UINT8 ucCamera_Status = m_sAppDataMgrInfo.sSystemValues.ucCamera_Module_Status;;
    ePANEL_ID ePanelId = palSystem_PanelID_Get();
    UINT8 uc3DEn = palDataMgr_IS_3D_Enable();

    // System flag internal checking
    if( !(( ucCamera_Status == eCAMERA_MODULE_EXIST ) &&
        ( (ePanelId != PANEL_3D_OUTPUT) && (ePanelId != PANEL_2D_HIGHSPEED) ) &&           //G100_Clare_0024, add
        ( uc3DEn == ets_OFF ) &&        //G100_Clare_0024, add
        (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) &&
         ( m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_AUST ) //A70LK_Jacky_0026
        #ifdef SCALER_FPGA_F34
        && ( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == TRUE )  //A70LK_Simon_0015
        && (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D)   //A70LK_Larry_0180 //H2PF_Simon_0041
        #endif
        )
        )
    {
        return eFUNC_CONTROL_DISABLE;
    }

    if(m_sAppDataMgrInfo.sSystemValues.cGroupingStatus != eCM_GROUP_STATUS_MASTER)
    {
        return eFUNC_CONTROL_DISABLE;
    }

    if(m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Cnt > 1)
    {
        UINT8 cCount = 0;
        for(cCount = 0; (cCount < m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Cnt) && (cCount < GROUPING_MENU_SELECT_ITEM); cCount++)
        {
            if(!m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[cCount].CameraStatus) //No Camera
            {
                return eFUNC_CONTROL_DISABLE;
            }
        }
    }

    if(!m_sAppDataMgrInfo.sSystemValues.cGroupingEventAutoColorMatch) //HICC2_Ashton_0005
    {
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_ENABLE;

}

eFUNC_CONTROL palDataMgr_Control_GroupingSelectEnable(void)
{
    if(m_sAppDataMgrInfo.sSystemValues.cGroupingStatus == eCM_GROUP_STATUS_MASTER)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_GroupingFunctionEnable(void)
{
    if((m_sAppDataMgrInfo.sSystemValues.cGroupingStatus != eCM_GROUP_STATUS_STANDBY) &&
       (m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Cnt > 1))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Camera_Lens(void)
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;
    UINT8 ucLensLocked = eLENS_LOCK_ALL_MAX_NUMBER;
    UINT8 ucCamera_Status = m_sAppDataMgrInfo.sSystemValues.ucCamera_Module_Status;;
    UINT8 ucPIPEn = eEXEC_CODE_FAIL;    //G100_Clare_0024, add
    eEXEC_CODE eCode = palDataMgr_Data_Access( edcLOCK_ALL_LENS_MOTORS, edaREAD, &ucLensLocked );
	ePANEL_ID ePanelId = palSystem_PanelID_Get();	        //G100_Clare_0024, add
	UINT8 uc3DEn = palDataMgr_IS_3D_Enable();               //G100_Clare_0024, add

    if( ( ucCamera_Status == eCAMERA_MODULE_EXIST ) &&
        ( (ePanelId != PANEL_3D_OUTPUT) && (ePanelId != PANEL_2D_HIGHSPEED) ) &&           //G100_Clare_0024, add
        ( uc3DEn == ets_OFF ) &&        //G100_Clare_0024, add
        ( eCode == eEXEC_CODE_PASS ) &&
        ( ucLensLocked == eLENS_LOCK_ALL_ALLOW ) &&
        (palDataMgr_PIN_Protect_Checking() == ePASSWORD_PROTECT_UNLOCK) && //G100_Coda_00124
        #ifdef SCALER_FPGA_F34
        (palDataPath_PanelConfigGet() == eWAPRING_AP_GROUP_2D) &&   //A70LK_Larry_0180  //H2PF_Simon_0041
        ( m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable == TRUE ) &&  //A70LK_Simon_0015
        #endif
        ( m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_AUST) //A70LK_Jacky_0026

        )
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_ADV_Warping(void)			//G100_Doulas_0027
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    if(Syscfg_Value_Get_Typeint(eGeo_BehaviorType) == eGEO_SUPPORT_TWIST_MYSTIQUE)
    {
        if(/*(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__ADVANCED) &&  */
            (palGeo_ApLinkFlag_Get() == 0) &&
            palDataMgr_CurrentWarpMemoryType() != WARPING_TYPE_AP &&
            palDataMgr_CurrentWarpMemoryType() != WARPING_TYPE_AP_MYSTIQUE
        )
        {
            eRet = eFUNC_CONTROL_ENABLE;
        }
    }
    else
    {
        if((palGeo_ApLinkFlag_Get() == 0) &&
            m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__ADVANCED
        )
        {
            eRet = eFUNC_CONTROL_ENABLE;
        }
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_ADV_WarpingInner(void)			//G100_Doulas_0027
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl == WARP_CTRL__ADVANCED) &&
	   (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpGridPoints != WARP_POINT__2x2) &&
	   (palGeo_ApLinkFlag_Get() == 0))  //A35G2_Simon_0086
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_DateTime_Mode_Control(void)			//G100_Coda_0038
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    UINT8 ucMode = m_sAppDataMgrInfo.sSystemValues.sDateTimeInfo.ucClockMode;

    if(ucMode != ets_OFF)
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_DateTime_Mode_USE_NTP_Control(void)			//G100_Coda_0038
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    UINT8 ucMode = m_sAppDataMgrInfo.sSystemValues.sDateTimeInfo.ucClockMode;

    if(ucMode == ets_OFF)
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_DateTime_Mode_Manual_Control(void)
{
    eFUNC_CONTROL eRet = eFUNC_CONTROL_DISABLE;

    UINT8 ucMode = m_sAppDataMgrInfo.sSystemValues.sDateTimeInfo.ucClockMode;

    if(ucMode == eCM_DATETIME_CLOCK_MODE_MANUAL && palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
        eRet = eFUNC_CONTROL_ENABLE;
    }

    return (eRet);
}

eFUNC_CONTROL palDataMgr_Control_AutoFocus(void)
{
    if(palDataMgr_Control_Camera_Lens() == eFUNC_CONTROL_ENABLE)
    {
        if(m_sAppDataMgrInfo.sSystemValues.cGroupingStatus != eCM_GROUP_STATUS_STANDBY &&
           m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucImageFreeze == ets_ON)
        {
            return eFUNC_CONTROL_DISABLE;
        }

        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_ScheduleControl(void)
{
    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
        if(m_sAppDataMgrInfo.sSystemValues.cGroupingStatus == eCM_GROUP_STATUS_SLAVE &&
           m_sAppDataMgrInfo.sSystemValues.cGroupingEventSchedule == TRUE)
        {
            return eFUNC_CONTROL_DISABLE;
        }

        return eFUNC_CONTROL_ENABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}

// HICC2_Keven_0009
eFUNC_CONTROL palDataMgr_Control_DisplayModeEnable(void)
{
    UINT8  ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

    if(( palDataPath_IsSourceLock() == TRUE) )//||
       //((palDataPath_IsSourceLockSub() == TRUE) &&
       // (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable == TRUE)) )
    {
        if((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_2DHIGHSPEED) ||
            (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode == eCM_PICTURE_SETTINGS_HDR))
        {
            // if HDR or 2D high speed
            return eFUNC_CONTROL_DISABLE;
        }
        else
        {
            // Normal picture setting
            return eFUNC_CONTROL_ENABLE;
        }
    }
    else
    {
        // source is absent
        return eFUNC_CONTROL_DISABLE;
    }

    return eFUNC_CONTROL_DISABLE;
}
// HICC2_Keven_0009

eFUNC_CONTROL palDataMgr_Control_RemoteIDEnable(void)
{
    return eFUNC_CONTROL_ENABLE;
}

eFUNC_CONTROL palDataMgr_Control_NTPIPEnable(void)
{

    return eFUNC_CONTROL_ENABLE;
}

BOOL palDataMgr_Control_CheckSecurity_Status(void)//A65_OPTOMA_David_0014
{
	if (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucPassWord[0]==0xFF &&
        m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucPassWord[1]==0xFF &&
        m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucPassWord[2]==0xFF &&
        m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucPassWord[3]==0xFF)
	{
	     //LOG_MSG(db_ALWAYS  , "(===Null===CheckSecurity= %d,%d,%d,%d\r\n", ucCurrentPIN[0],ucCurrentPIN[1],ucCurrentPIN[2],ucCurrentPIN[3],ucCurrentPIN[4]  );
	     return FALSE;// Null empty
	}
	else
	{
	    //LOG_MSG(db_ALWAYS  , "(===PassWord===CheckSecurity= %d,%d,%d,%d\r\n", ucCurrentPIN[0],ucCurrentPIN[1],ucCurrentPIN[2],ucCurrentPIN[3],ucCurrentPIN[4]  );
	    return TRUE;//
	}
}

eFUNC_CONTROL palDataMgr_Control_Security_Lock(void) //A65_OPTOMA_David_0014        //A65_OPTOMA_David_0014
{

    if((palDataMgr_Control_CheckSecurity_Status()== TRUE) && (palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
       /*
       if(appGui_CurrentMenuIndexGet()== SECURITY_CHANGE_FIRSTPIN_MENU_ICOUNT && (palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE))
       {
            return eFUNC_CONTROL_ENABLE;
       }
	  else
	  */
	  {
       	return eFUNC_CONTROL_DISABLE;
	  }
    }
}

eFUNC_CONTROL palDataMgr_Control_Energy_Saving(void)
{
	if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucEnergySaving == ets_ON)
		return eFUNC_CONTROL_DISABLE;

    if(palSystem_PowerStateGet() == ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    return eFUNC_CONTROL_DISABLE;
}

eFUNC_CONTROL palDataMgr_Control_BackFocusEnable(void)//HICC2_Julie_0072//H30K_Julie_0005
{
    if(palSystem_PowerStateGet() != ePOWER_STATE_ACTIVE)
    {
        return eFUNC_CONTROL_DISABLE;
    }
    if((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_B20U100)
	&& (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType != eLENS_ID_NVRAM_B20U100))
    {
        return eFUNC_CONTROL_DISABLE;
    }

    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW &&
       m_sAppDataMgrInfo.sSystemValues.ucLensCalibrationDoing == 0)
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1)
        {
            return eFUNC_CONTROL_DISABLE;
        }
        else
        {
            return eFUNC_CONTROL_ENABLE;
        }
    }
    else
    {
        return eFUNC_CONTROL_LENS_LOCK_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_LensMemorySave(void)                 //HICC2_AC_0048
{
    UINT8 ucData = 0;

    if(palEnvironment_LensCalFlag_Get() == FALSE)
    {
    	return eFUNC_CONTROL_DISABLE;
    }
    else if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust == eLENS_LOCK_ALL_ALLOW
            && palDataMgr_Control_USTMode() == eFUNC_CONTROL_ENABLE)
    {
        if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100) ||
             (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
            && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1) //1:unlock ; 0: lock
            )
        {
           return eFUNC_CONTROL_DISABLE;
        }
    #ifdef CUSTOM_CHRISTIE  //A35G2_Wesley_0110
        if(palDataMgr_AutoFocusExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoWallColorExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_AutoColorMatchExecute_Get() != eAF_AC_NOT_EXECUTE ||
           palDataMgr_ACU_Execute_Get() != eACU_NOT_EXECUTED)
        {
            return eFUNC_CONTROL_DISABLE;
        }
    #endif

        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_LENS_LOCK_DEPEND;
    }

}


eFUNC_CONTROL palDataMgr_Control_Light_Source_Settings(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement == eCONTRAST_ENHANCE_OFF)
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_CONTRAST_ENHANCEMENT_DEPEND;
    }
}

eFUNC_CONTROL palDataMgr_Control_Source_KeyInput_Settings(void) //H30K_Doulas_0057
{
    if((palDataMgr_Control_SourceDepend() == eFUNC_CONTROL_ENABLE)
        || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable != FALSE)
        || (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucSourceKeyOption != eSOURCE_KEY_CHANGE_SOURCE_AUTO))
    {
        return eFUNC_CONTROL_ENABLE;
    }
    else
    {
        return eFUNC_CONTROL_DISABLE;
    }
}

eFUNC_CONTROL palDataMgr_Control_Lens_Reset(void)
{
    if(((m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_B20U100) ||
             (m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLensType == eLENS_ID_NVRAM_B20U100))
            && (m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucFujiLensDetect == 1) //1:unlock ; 0: lock
            )
    {
       return eFUNC_CONTROL_DISABLE;
    }
    return eFUNC_CONTROL_ENABLE;

}




const sDATA_CONTROL_LUT_TABLE m_sDataControl_LutTable[] =
{
	/*0000*/ {eacNORAML,                                palDataMgr_Control_Normal},
	/*0001*/ {eacNOT_READY,                             palDataMgr_Control_FunctionNotReady},
	/*0002*/ {eacPOWER_MODE,                            palDataMgr_Control_PowerMode},
	/*0003*/ {eacOSD_TEST_PATTERN,                      palDataMgr_Control_OSD_TestPattern},
	/*0004*/ {eacSCALER,                                palDataMgr_Control_Scaler},
	/*0005*/ {eacDIGITAL_HORZ_SHIFT,                    palDataMgr_Control_DigitalHorzShift},
	/*0006*/ {eacDIGITAL_VERT_SHIFT,                    palDataMgr_Control_DigitalVertShift},
	/*0007*/ {eacWARPING_ENABLE,                        palDataMgr_Control_WarpingEnable},
	/*0008*/ {eacBLENDING_ENABLE,                       palDataMgr_Control_BlendingEnable},
	/*0009*/ {eacWARPING_FILTER,                        palDataMgr_Control_WarpAutoFilter},
	/*0010*/ {eacSOURCE_DEPEND,                         palDataMgr_Control_SourceDepend},
	/*0011*/ {eacCOLOR_SPACE_SETTING,                   palDataMgr_Control_ColorSpaceSetting},
	/*0012*/ {eac3D_ENABLE_DEPEND,                      palDataMgr_Control_3D_EnableDepend},
	/*0013*/ {eac3D_INVERT_DEPEND,                      palDataMgr_Control_3D_InvertDepend},
	/*0014*/ {eacFRAME_DELAY_DEPEND,                    palDataMgr_Control_FrameDelayDepend},
	/*0015*/ {eacTP,                                    palDataMgr_Control_TP_NR},
	/*0016*/ {eacMPEG_NR,                               palDataMgr_Control_MPEG_NR},
	/*0017*/ {eacPICTURE_SETTING,                       palDataMgr_Control_PictureSetting},
	/*0018*/ {eacSAVE_TO_USER,                          palDataMgr_Control_SaveToUserDepend},
	/*0019*/ {eacCONTRAST_ENHANCEMENT,                  palDataMgr_Control_ContrastEnhancement},
	/*0020*/ {eacMAIN_OR_SUB_SOURCE_DEPEND,             palDataMgr_Control_MainOrSubSourceDepend},
	/*0021*/ {eacHDR_ENABLE,                            palDataMgr_Control_HDREnable},
	/*0022*/ {eacSOURCE_PICTUREMODE_BLEND,              palDataMgr_Control_Source_PictureMode_Blend},
	/*0023*/ {eacWHEEL_SPEED_DEPEND,                    palDataMgr_Control_WheelSpeedDepend},
	/*0024*/ {eacLENS_LOCK,                             palDataMgr_Control_Lens_Lock},
	/*0025*/ {eacMENU_OFFSET,                           palDataMgr_Control_MenuOffset},
	/*0026*/ {eacHIGH_ALTITUDE,                         palDataMgr_Control_HighAltitude},
	/*0027*/ {eacLAN_DHCP,                              palDataMgr_Control_LAN_DHCP},
	/*0028*/ {eacHSG_ADJUSETMENT,                       palDataMgr_Control_HSG_Adjusetment},
	/*0029*/ {eacCOLOR_ENHANCEMENT,                     palDataMgr_Control_ColorEnhancement},
	/*0030*/ {eacWHEEL_INDEX_2X,                        palDataMgr_Control_WheelIndex2X},
	/*0031*/ {eacWHEEL_INDEX_3X,                        palDataMgr_Control_WheelIndex3X},
	/*0032*/ {eacLD_INFO,                               palDataMgr_Control_LD_Info},
	/*0033*/ {eacCONSTANT_POWER,                        palDataMgr_Control_ConstantPower},
	/*0034*/ {eacPIP_PBP_ENABLE,                        palDataMgr_Control_PIP_PBP_Enable},
	/*0035*/ {eacTEST_PATTERN_DEPEND,                   palDataMgr_Control_TestPatternDepend},
	/*0036*/ {eacPIP,                                   palDataMgr_Control_PIP},
	/*0037*/ {eacINPUTKEY,                              palDataMgr_Control_InputKey},
	/*0038*/ {eacPIXEL_TRACK_PHASE,                     palDataMgr_Control_Pixel_Track_Phase},
	/*0039*/ {eacBLENDING_TOP_ENABLE,                   palDataMgr_Control_BlendingTopEnable},
	/*0040*/ {eacBLENDING_BOTTOM_ENABLE,                palDataMgr_Control_BlendingBottomEnable},
	/*0041*/ {eacBLENDING_LEFT_ENABLE,                  palDataMgr_Control_BlendingLeftEnable},
	/*0042*/ {eacBLENDING_RIGHT_ENABLE,                 palDataMgr_Control_BlendingRightEnable},
	/*0043*/ {eacSERVICE_TEST_PATTERN,                  palDataMgr_Control_Service_TestPattern},
    /*0044*/ {eacSYNC_THRESHOLD_DEPEND,                 palDataMgr_Control_SyncThresholdDepend},
    /*0045*/ {eacMODE_ADJUSTMENT,                       palDataMgr_Control_ModeAdjustment},
    /*0046*/ {eacWARPING_APPLY,                         palDataMgr_Control_WarpingApply},
    /*0047*/ {eacWARPING_SAVE,                          palDataMgr_Control_WarpingSave},
    /*0048*/ {eacTWIST,                                 palDataMgr_Control_Twist},
    /*0049*/ {eacHDR_STATUS,                            palDataMgr_Control_HDR_Status},
    /*0050*/ {eacLR_REFERENCE_DEPEND,                   palDataMgr_Control_LR_ReferenceDepend},
    /*0051*/ {eacWARPING_AP_ON,                         palDataMgr_Control_WarpingAPSave},
    /*0052*/ {eacBACKUP_RESTORE,                        palDataMgr_Control_Backup_Restore},
    /*0053*/ {eacBLEND_SAVE,                            palDataMgr_Control_BlendSave},
    /*0054*/ {eacCOLOR_TEMPERATURE_DEPEND,              palDataMgr_Control_ColorTemperatureDepend},
    /*0055*/ {eacBACKUP_INPUT,                          palDataMgr_Control_BackupInput},
    /*0056*/ {eacDYNAMIC_BLACK,                         palDataMgr_Control_DynamicBlack},
    /*0057*/ {eacREAL_BLACK,                            palDataMgr_Control_RealBlack},
    /*0058*/ {eacDYNAMIC_BLACK_ENABLE,                  palDataMgr_Control_DynamicBlack_Enable},
    /*0059*/ {eacREAL_BLACK_ENABLE,                     palDataMgr_Control_RealBlack_Enable},
    /*0060*/ {eacEDGE_MASK,                             palDataMgr_Control_EdgeMask},
    /*0061*/ {eacCONSTANT_BRIGHTNESS_DEPEND,            palDataMgr_Control_ConstantBrightness},	//G100_Doulas_0008
    /*0062*/ {eacLENS_MEMORY_APPLY,                     palDataMgr_Control_LensMemoryApply},  //G100_Owen_0030
    /*0063*/ {eac3D_MODE_DEPEND,                        palDataMgr_Control_3D_ModeDepend},		//G100_Doulas_0020
    /*0064*/ {eacIMAGE_FREEZE_DEPEND,                   palDataMgr_Control_ImageFreeze},		//G100_Coda_0001
    /*0065*/ {eacCAMERA_LENS,                           palDataMgr_Control_Camera_Lens},		//G100_Clare_0022
	/*0066*/ {eacADV_WarpingBlending, 		            palDataMgr_Control_ADV_Warping},		//G100_Doulas_0027
	/*0067*/ {eacADV_WarpingInner, 		                palDataMgr_Control_ADV_WarpingInner},		//G100_Doulas_0027
	/*0068*/ {eacSCHEDULE_EVENT_SETTING,                palDataMgr_Control_Schedule_Event_Setting}, //G100_Coda_0027
	/*0069*/ {eacDATE_TIME_MODE_DEPEND,                 palDataMgr_Control_DateTime_Mode_Control}, //G100_Coda_0038
	/*0070*/ {eacDATE_TIME_MODE_USE_NTP_DEPEND,         palDataMgr_Control_DateTime_Mode_USE_NTP_Control}, //G100_Coda_0038
	/*0071*/ {eacLOW_LATENCY,                           palDataMgr_Control_Low_Latency_Mode},	//G100_Clare_0028
	/*0072*/ {eacGAMMA,  					            palDataMgr_Control_Gamma},		//G100_Doulas_0046
    /*0073*/ {eacBACKGROUND_LOGO_DEPEND,                palDataMgr_Control_Background_LogoDepend},  //G100_Owen_0048
    /*0074*/ {eacVGA_DEPEND,                            palDataMgr_Control_VGA_Depend},  //G100_Simon_0038
    /*0075*/ {eacAPPLY_USER_DEPEND,                     palDataMgr_Control_ApplyUserDepend},  	//G100_Doulas_0066
	/*0076*/ {eacBACKUP_INPUT_SW,                       palDataMgr_Control_BackupInputSW}, //G100_Steven_0046
    /*0077*/ {eacLOGO_CAPTURED_DEPEND,                  palDataMgr_Control_LogoCaptured_Depend},  //G100_Owen_0061
	/*0078*/ {eacLENS_CALIBRATION,                      palDataMgr_Control_Lens_Calibration},   //G100_Owen_0062
    /*0079*/ {eacLENS_TYPE,                             palDataMgr_Control_LensType},  //G100_Simon_0057
    /*0080*/ {eacSUB_SOURCE_DEPEND,                     palDataMgr_Control_SubSourceDepend}, //G100_Coda_00105
    /*0081*/ {eacBACKUP_RESTORE_LOAD,                   palDataMgr_Control_Backup_Restore_Load},		//G100_Doulas_0075
	/*0082*/ {eacSUB_COLOR_SPACE_SETTING,               palDataMgr_Control_SubColorSpaceSetting},
    /*0083*/ {eacBLEND_APPLY,                           palDataMgr_Control_BlendApply},
	/*0084*/ {eac3D_SYNC_IN_SELECT_DEPEND,              palDataMgr_Control_3D_SyncInSelect_Depend},
	/*0085*/ {eacPIP_PBP_Sub_ColorSetting,              palDataMgr_Control_PIPPBP_Sub_ColorSetting}, //G100_Steven_0088
	/*0086*/ {eacASPECTRATIO,                           palDataMgr_Control_AspectRatio},
	/*0087*/ {eacDIMMINGMODE,                           palDataMgr_Control_DimmingMode},
	/*0088*/ {eacWHITE_PEAKING_DEPEND,                  palDataMgr_Control_WhitePeaking}, //A35G2_CDS_Coda_0016
    /*0089*/ {eacAUTO_HDMI_SWITCH,                      palDataMgr_Control_Auto_HDMI_Switch},   //G100_Tim_0057, add //A35G2_BRC_Casper_0060
    /*0090*/ {eacMAIN_SRC_DEPEND,                       palDataMgr_Control_Main_Src_Depend},    //G100_Tim_0057, add //A35G2_BRC_Casper_0060
    /*0091*/ {eacBACKUP_INPUT_MSRC_DEPEND,              palDataMgr_Control_BackupInput_MSRC_Depend},     //A35G2_BRC_Casper_0075
    /*0092*/ {eacBACKUP_INPUT_SSRC_DEPEND,              palDataMgr_Control_BackupInput_SSRC_Depend},     //A35G2_BRC_Casper_0075
    /*0093*/ {eacWARP_CONTROL_ADVANCED_WARP,		    palDataMgr_Control_Advanced_Warp},//R70G2_David_0035
	/*0094*/ {eacWARP_CONTROL_BLACK_LEVEL_EN,			palDataMgr_Control_ADV_BlackLevelEnable},		//A65_OPTOMA_Doulas_0020
	/*0095*/ {eacWARP_CONTROL_BLACK_LEVEL_ADD_POINT,	palDataMgr_Control_ADV_BlackLevelAddPoint},		//A65_OPTOMA_Doulas_0020
	/*0096*/ {eacWARP_CONTROL_BLACK_LEVEL_REMOVE_POINT,	palDataMgr_Control_ADV_BlackLevelRemovePoint},	//A65_OPTOMA_Doulas_0020
	/*0097*/ {eacQUICK_KEY,	                            palDataMgr_Control_QUICK_KEY},//A35G2_Coda_0049
    /*0098*/ {eacWARP_CONTROL,                          palDataMgr_Control_WarpingControl},  //A35G2_Simon_0069
    /*0099*/ {eacWARP_CONTROL_BASIC_WARP,               palDataMgr_Control_Basic_Warp}, //A35G2_Simon_0069
	/*0100*/ {eacUST_SET,	    						palDataMgr_Control_UST_Set}, //A35G2_BRC_Casper_0088
	/*0101*/ {eacUST_PATTERN,							palDataMgr_Control_UST_Pattern}, //A35G2_BRC_Casper_0088
	/*0102*/ {eacWARPING_AP_APPLY,	    				palDataMgr_Control_Warping_AP_Apply}, //A35G2_Simon_0083
	/*0103*/ {eacBLEND_AP_APPLY,						palDataMgr_Control_Blend_AP_Apply},   //A35G2_Simon_0083
	/*0104*/ {eacMODEL_SWITCH_ADJUST, 					palDataMgr_Control_Model_Switch_Adjust}, //A35G2_Coda_0101
	/*0105*/ {eacSYSTEM_POWERSTATE,                     palDataMgr_Control_System_PowerState},
	/*0106*/ {eacACU_CALIBRATION,					    palDataMgr_Control_ACU_Calibration}, //A35G2_Alan_0001
	/*0107*/ {eacACU_RESET,							    palDataMgr_Control_ACU_Reset}, //A35G2_Alan_0001
	/*0108*/ {eacATW_SET,							    palDataMgr_Control_ATW_Set}, //A35G2_Alan_0001
    /*0109*/ {eac3D_ASPECT_RATIO,                       palDataMgr_Control_3D_Aspect_Ratio},
    /*0110*/ {eacSCH_SHOW_EVENT_INFO_CK1,				palDataMgr_Control_Schedule_Show_Event_Info_CK1},
    /*0111*/ {eacSCH_SHOW_EVENT_INFO_CK2,				palDataMgr_Control_Schedule_Show_Event_Info_CK2},
    /*0112*/ {eacSCH_SHOW_EVENT_INFO_CK3,				palDataMgr_Control_Schedule_Show_Event_Info_CK3},
    /*0113*/ {eacSCH_SHOW_EVENT_INFO_CK4,				palDataMgr_Control_Schedule_Show_Event_Info_CK4},
    /*0114*/ {eacSCH_SHOW_EVENT_INFO_CK5,				palDataMgr_Control_Schedule_Show_Event_Info_CK5},
    /*0115*/ {eacSCH_SHOW_EVENT_INFO_CK6,				palDataMgr_Control_Schedule_Show_Event_Info_CK6},
    /*0116*/ {eacSCH_SHOW_EVENT_INFO_CK7,				palDataMgr_Control_Schedule_Show_Event_Info_CK7},
    /*0117*/ {eacSCH_SHOW_EVENT_INFO_CK8,				palDataMgr_Control_Schedule_Show_Event_Info_CK8},
    /*0118*/ {eacSCH_SHOW_EVENT_INFO_CK9,				palDataMgr_Control_Schedule_Show_Event_Info_CK9},
    /*0119*/ {eacSCH_SHOW_EVENT_INFO_CK10,				palDataMgr_Control_Schedule_Show_Event_Info_CK10},
    /*0120*/ {eacSCH_SHOW_EVENT_INFO_CK11,				palDataMgr_Control_Schedule_Show_Event_Info_CK11},
    /*0121*/ {eacSCH_SHOW_EVENT_INFO_CK12,				palDataMgr_Control_Schedule_Show_Event_Info_CK12},
    /*0122*/ {eacSCH_SHOW_EVENT_INFO_CK13,				palDataMgr_Control_Schedule_Show_Event_Info_CK13},
    /*0123*/ {eacSCH_SHOW_EVENT_INFO_CK14,				palDataMgr_Control_Schedule_Show_Event_Info_CK14},
    /*0124*/ {eacSCH_SHOW_EVENT_INFO_CK15,				palDataMgr_Control_Schedule_Show_Event_Info_CK15},
    /*0125*/ {eacIS_3D_ENABLE,					        palDataMgr_Control_IS_3D_Enable},
    /*0126*/ {eacACU_TARGET_STATUS_ENABLE,				palDataMgr_ACU_Target_Status_Enable},
    /*0127*/ {eacCAMERA_WORKING_ENABLE,                 palDataMgr_Control_Camera_Working_Enable},
    /*0128*/ {eacAPPLY_WARP_MEMORY_1,                   palDataMgr_Control_ApplyWarpMemory1},
    /*0129*/ {eacAPPLY_WARP_MEMORY_2,                   palDataMgr_Control_ApplyWarpMemory2},
    /*0130*/ {eacAPPLY_WARP_MEMORY_3,                   palDataMgr_Control_ApplyWarpMemory3},
    /*0131*/ {eacAPPLY_WARP_MEMORY_4,                   palDataMgr_Control_ApplyWarpMemory4},
    /*0132*/ {eacAPPLY_WARP_MEMORY_5,                   palDataMgr_Control_ApplyWarpMemory5},
    /*0133*/ {eacAPPLY_BLEND_MEMORY_1,                  palDataMgr_Control_ApplyBlendMemory1},
    /*0134*/ {eacAPPLY_BLEND_MEMORY_2,                  palDataMgr_Control_ApplyBlendMemory2},
    /*0135*/ {eacAPPLY_BLEND_MEMORY_3,                  palDataMgr_Control_ApplyBlendMemory3},
    /*0136*/ {eacAPPLY_BLEND_MEMORY_4,                  palDataMgr_Control_ApplyBlendMemory4},
    /*0137*/ {eacAPPLY_BLEND_MEMORY_5,                  palDataMgr_Control_ApplyBlendMemory5},
	/*0139*/ {eacLIGHT_SENSOR_CAL_DEPEND,   			palDataMgr_Control_Light_Sensor_Cal_Depend},	//A70Gen2_Doulas_0015
	/*0140*/ {eacDATE_TIME_MODE_MANUAL_DEPEND,   		palDataMgr_Control_DateTime_Mode_Manual_Control}, //A70Gen2_Julie_0014
	/*0141*/ {eacGEO_AP_CMD_ENABLE,              		palDataMgr_Control_BlendAPCmdEnable_Control},
	/*0142*/ {eacCOLOR_SPACE_SETTING_RGB,               palDataMgr_Control_ColorSpaceSettingRGB},
	/*0143*/ {eacSUB_COLOR_SPACE_SETTING_RGB,           palDataMgr_Control_SubColorSpaceSettingRGB},
	/*0144*/ {eacGEOMETRY_ENABLE,						palDataMgr_Control_GeometryEnable},
	/*0145*/ {eacACU_3D_STATUS,						    palDataMgr_Control_ACU_3D_Status},//A70LK_Coda_0010
	/*0145*/ {eacTAA_WIFI,               		        appDataMgr_Control_TAA_WiFi}, //R70K_AC_0092 //HICC2_Casper_0013
    /*0146*/ {eacSPECIAL_LOCATION,                      appDataMgr_Control_LensSpecialLocation},// A35G2_Bruce_0008 //A70Gen2_Julie_0109//HICC2_Julie_0014
    /*0147*/ {eacACM_STATUS,                            palDataMgr_Control_AutoColorMatch},// HICC2_Ashton
    /*0148*/ {eacWARPING_MEMORY,                		palDataMgr_Control_WarpingMemory},  //H2PF_Simon_0042
	/*0149*/ {eacBLENDING_MEMORY,               		palDataMgr_Control_BlendingMemory}, //H2PF_Simon_0042
    /*0150*/ {eacGROUPING_SELECT_ENABLE,                palDataMgr_Control_GroupingSelectEnable},
    /*0151*/ {eacGROUPING_FUNCTION_ENABLE,              palDataMgr_Control_GroupingFunctionEnable},
    /*0152*/ {eacLENS_CONTROL,                          palDataMgr_Control_LensControl}, //HICC2_Doulas_0132
    /*0153*/ {eacLAN_POWER_STATUS,                      palDataMgr_Control_LANPowerState},
    /*0154*/ {eacRESET_IMAGE_SETTING,					palDataMgr_Control_ResetImageSetting}, //HICC2_Julie_0059
    /*0155*/ {eacLENS_DURATIONTIME_ENABLE,				palDataMgr_Control_Lens_DurationTime_Enable},
	/*0156*/ {eacBIST_ENABLE,                           palDataMgr_Control_BIST_Enable}, //HICC2_Steven_0071
    /*0157*/ {eacCURSOR_FIXTURE,                        palDataMgr_Control_Cursor_Fixture_Enable},
	/*0158*/ {eacADV_WARP_CONTROL,                      palDataMgr_Control_AdvWarpingControl},
	/*0159*/ {eacAUTO_FOCUS,                            palDataMgr_Control_AutoFocus},
	/*0160*/ {eacSCHEDULE_CONTROL,                      palDataMgr_Control_ScheduleControl},
	/*0161*/ {eacDISPLAYMODE_ENABLE,                    palDataMgr_Control_DisplayModeEnable}, // HICC2_Keven_0009
	/*0162*/ {eacREMOTEID_ENABLE,                       palDataMgr_Control_RemoteIDEnable}, // HICC2_Keven_0014
    /*0163*/ {eacLENS_SHIFT_CONTROL,                    palDataMgr_Control_Lens_Shift},
	/*0164*/ {eacNTP_IP_ENABLE,                         palDataMgr_Control_NTPIPEnable}, // HICC2_Keven_0019
	/*0165*/ {eacBACKFOCUS_ENABLE,                      palDataMgr_Control_BackFocusEnable}, //HICC2_Julie_0072//H30K_Julie_0005
	/*0166*/ {eacLENS_MEMORY_SAVE,                      palDataMgr_Control_LensMemorySave}, // HICC2_Keven_0019
	/*0167*/ {eacHDR_MANUAL,                            palDataMgr_Control_Normal},
	/*0168*/ {eacHDR_CUSTOM,                            palDataMgr_Control_Normal},
	/*0169*/ {eacHDR_AUTO,                              palDataMgr_Control_Normal},
	/*0170*/ {eacHDR_DETECT,                            palDataMgr_Control_Normal},
	/*0171*/ {eacHDR_SDR_CUSTOM,                        palDataMgr_Control_Normal},
	/*0172*/ {eacIPV6_LAN_DHCP,                         palDataMgr_Control_IPV6_LAN_DHCP}, //HICC2_AC_0052 // HICC2_Bruce_0020
	/*0173*/ {eacFILTER_SETTINGS_DEPEND,                palDataMgr_Control_Normal},
	/*0174*/ {eacDUST_FILTER_ENABLE,                    palDataMgr_Control_Normal},
	/*0175*/ {eacFOG_FILTER_ENABLE,                     palDataMgr_Control_Normal},
	/*0176*/ {eacEDIT_REMAINING_TIME,                   palDataMgr_Control_Normal},
    /*0177*/ {eacART_NET_ON_2X_10X_DISABLE,				palDataMgr_Control_Art_Net_On_2x_10x_Disable},
	/*0178*/ {eacLIGHT_OUTPUT_SETTING,                  palDataMgr_Control_Normal},
    /*0179*/ {eacSECURITY_LOCK,                			palDataMgr_Control_Security_Lock},		//A65_OPTOMA_David_0014
    /*0180*/ {eacENERGY_SAVING,                			palDataMgr_Control_Energy_Saving},
    /*0181*/ {eacRASTER_POSITION,                		palDataMgr_Control_Normal},
    /*0182*/ {eacLIGHT_SOURCE_SETTINGS,            		palDataMgr_Control_Light_Source_Settings},
    /*0183*/ {eacDIGITAL_SHIFT_DEPEND,            		palDataMgr_Control_Normal},
    /*0184*/ {eacPANEL_4K2K_DEPEND,            			palDataMgr_Control_Normal},
    /*0185*/ {eacSOURCE_INPUTKEY_DEPEND,            	palDataMgr_Control_Source_KeyInput_Settings}, //H30K_Doulas_0057
    /*0186*/ {eacLENS_LOCK_FOCUS2,            			palDataMgr_Control_Normal},
    /*0187*/ {eacACTUATOR_APPLY,            			palDataMgr_Control_Normal},
    /*0188*/ {eacMENU_TRANSPARENCY,            			palDataMgr_Control_Normal},
    /*0189*/ {eacSHUTTER_FADE,            				palDataMgr_Control_Normal},
    /*0190*/ {eacDAILY_ENABLE,            				palDataMgr_Control_Normal},
    /*0191*/ {eacCAMERA_STATUS,            				palDataMgr_Control_Normal},
    /*0192*/ {eacBURNIN_ENABLE,            	    		palDataMgr_Control_Normal},
    /*0193*/ {eacSTANDARD_LENS,            	    		palDataMgr_Control_Normal},
    /*0194*/ {eacFILTER_DISABLE_ATTITUDE,         		palDataMgr_Control_Normal},
    /*0195*/ {eacLENS_RESET,            	    		palDataMgr_Control_Lens_Reset}

};

VERIFY_SIZE_OF(m_sDataControl_LutTable, sizeof(m_sDataControl_LutTable[0])*eacINVALID);

#endif /* __APPDATACONTROLMGR_C__ */

