// ===============================================================================
// FILE NAME: appDataInitMgr.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2024/08/21, Larry Create
// --------------------
// ===============================================================================

#ifdef __APPDATAINITMGR_C__

eEXEC_CODE palDataMgr_Init(void)
{
    struct sLAYOUT_VERSION sVersion;
    struct sLAYOUT_VERSION sResetAll; //A70LV_Larry_0128
	BOOL	bSWReset = FALSE; //A70LV_Larry_0264
	char	cModelName[24] = {'\0'};
	UINT8   ucValue = 0;	//G100_Doulas_0082

    //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);   //A70LV_Doulas_0017

    //It is used to protect m_sDataMgrInfo.sEepSettings variable
#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

        if(pthread_mutex_init(&m_sAppDataMgrInfo.xMutex, NULL) != 0)
        {
            LOG_MSG(db_ALWAYS, "%s pthread_mutex_init fail [%s]\n", __FUNCTION__ , strerror(errno));
            return eEXEC_CODE_FAIL;
        }

#else
    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        m_sAppDataMgrInfo.xSemaphore = xSemaphoreCreateMutexStatic(&m_sAppDataMgrInfo.xMutexBuffer);
    #else
        m_sAppDataMgrInfo.xSemaphore = xSemaphoreCreateMutex();
    #endif

        if(m_sAppDataMgrInfo.xSemaphore == NULL)
        {
            return eEXEC_CODE_FAIL;
        }
#endif

    if(IS_SINGLE_ISCALER_MODE())
    {
        utilDataMapping_Init(TRUE);
    }
    else  //wait Data Mapping Table Ready
    {
        printf("Wait Data Mapping Table Ready...\n"); //HICC2_Doulas_0084//HICC2_Doulas_0083
        UINT8 RetryCount = 0;
        while(DataMapping_ReadyFlagGet() != TRUE)
        {
            RetryCount++;
            if(RetryCount > 5 && RetryCount < 15)
            {
                printf("Wait Data Mapping Table Ready...\n"); //HICC2_Doulas_0084//HICC2_Doulas_0083
            }

            MS_SLEEP(500);
            utilDataMapping_Init(FALSE);
        }
        printf("Data Mapping Table Ready !\n"); //HICC2_Doulas_0084//HICC2_Doulas_0083
    }
	utilDataMgr_Initialization(&m_sDefSysSettings); //HICC2_Doulas_0003 Modify
    utilOPD_Initialization();
    utilDataMgr_FileToRAM_CM(m_psEEP);	//HICC2_Doulas_0003
    utilDataMgr_LanInit_CM(&m_sLAN_info);	//HICC2_Doulas_0003
    palDataMgr_LAN_IP_Copy1_All(); //G100_Larry_0035
	utilDataMgr_ArtNetInit_CM(&m_sArtNetInfo);// HICC2_Bruce_0017

    palDataMgr_CheckGroupSupported(); //HICC2_Ashton_0003
#ifdef NETWORK_IP_DHCP
    palDataMgr_Default_LanInfo_Get();
    palDataMgr_IPV6_Default_LanInfo_Get(); //HICC2_AC_0078 // HICC2_Bruce_0020
#endif

    //LOG_MSG(db_ALWAYS, "%d, %d\r\n", &m_psEEP->sLayoutVersion, &sVersion);   //A70LV_Doulas_0017

    EE_GETVAR(sLayoutVersion, sVersion);                 /* version from EEPROM */
    //utilStartup_DataMgrVersionGet((UINT16)sizeof(sVersion), (UINT8*)&sResetAll);  //A70LV_Larry_0128 enter power on
    //halMCUCtrl_Model_ID_Get(&m_sAppDataMgrInfo.sSystemValues.ucModelID); //G100_Steven_0039

	if(memcmp(&sVersion, &(m_sDefSysSettings.sLayoutVersion), sizeof(sVersion))) //A70LV_Larry_0264
	{
		bSWReset = TRUE;
	}

    if((sVersion.ucMajor != LAYOUT_VERSION_MAJOR) ||
       (sVersion.ucMinor != LAYOUT_VERSION_MINOR) ||
       (sVersion.ucSubminor != LAYOUT_VERSION_SUBMINOR))
    {
        LOG_MSG(db_ALWAYS, "EEPROM version mismatch:\r\n");   //A70LV_Doulas_0017

        LOG_MSG(db_ALWAYS, "EEPROM ver. %02d.%02d.%02d\r\n",  //A70LV_Doulas_0017
                sVersion.ucMajor,
                sVersion.ucMinor,
                sVersion.ucSubminor);

        LOG_MSG(db_ALWAYS, "Reset to Default EEPROM ver. %02d.%02d.%02d\r\n", //A70LV_Doulas_0017
                m_sDefSysSettings.sLayoutVersion.ucMajor,
                m_sDefSysSettings.sLayoutVersion.ucMinor,
                m_sDefSysSettings.sLayoutVersion.ucSubminor);
        //TODO
        /*
              Start to do factory resest when version is mismatched
              */
        /*=================================================================*/
        /*               don't clear by factory reset                      */
        EE_GETVAR(sSystemDefault.ucSerialNumber, m_sDefSysSettings.sSystemDefault.ucSerialNumber);  //A70LV_Doulas_0093 Add,get SN

        EE_GETVAR(sUserSystemSetting.sCommonSetting.ucModelName, m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucModelName);
        EE_GETVAR(sUserSystemSetting.sCommonSetting.ucCustomCode, m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucCustomCode);

        printf("%s\n", m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucModelName);
        printf("%s\n", m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucCustomCode);

        /*=================================================================*/

        if((sVersion.ucSubminor  == 0x0) &&
           (sVersion.ucMinor     == 0x0) &&
           (sVersion.ucMajor     == 0x0))      //A70LV_Doulas_0268 EEPROM empty
        {
            palDataMgr_Access_MODEL_NAME(edaREAD, (void *)cModelName);

            sprintf(m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucModelName, "%s",cModelName);

            m_sDefSysSettings.sLayoutVersion.ucMajor = LAYOUT_VERSION_MAJOR;
            m_sDefSysSettings.sLayoutVersion.ucMinor = LAYOUT_VERSION_MINOR;
            m_sDefSysSettings.sLayoutVersion.ucSubminor = LAYOUT_VERSION_SUBMINOR;

            utilDataMgr_ResetToDefault(eNVRAM_EVENT_RESETALL);	//HICC2_Doulas_0003
            utilDataMgr_RAMToFileAll_CM(&m_sDefSysSettings);	//HICC2_Doulas_0003
        }
		else if(bSWReset == TRUE) //A70LV_Larry_0264
		{
#if 0
			EE_GETVAR(sSystemDefault.ulTotalProjectorMinute, m_sDefSysSettings.sSystemDefault.ulTotalProjectorMinute);
			EE_GETVAR_SIZE(sSystemDefault.ulLightSourceMinute, 8, m_sDefSysSettings.sSystemDefault.ulLightSourceMinute);
			EE_GETVAR_SIZE(sADC_cal_values, sizeof(sADC_CAL_VALUES), m_sDefSysSettings.sADC_cal_values);
			EE_GETVAR(sSystemDefault.ulTotalProjectorMinute2, m_sDefSysSettings.sSystemDefault.ulTotalProjectorMinute2); //A70LV_Larry_0268
			EE_GETVAR_SIZE(sSystemDefault.ulLightSourceMinute2, 8, m_sDefSysSettings.sSystemDefault.ulLightSourceMinute2); //A70LV_Larry_0268

            EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);    //A70LV_Doulas_0268
            EE_GETVAR(sSystemDefault.uiPWIndex, m_sDefSysSettings.sSystemDefault.uiPWIndex);    //A70LV_Doulas_0268
            EE_GETVAR(sSystemDefault.ucCOLOR_OFFSET, m_sDefSysSettings.sSystemDefault.ucCOLOR_OFFSET);
            EE_GETVAR(sSystemDefault.ucPOWER_OFFSET, m_sDefSysSettings.sSystemDefault.ucPOWER_OFFSET);
            EE_GETVAR_SIZE(sSystemDefault.uiLightSensorFull_BLD, 96, m_sDefSysSettings.sSystemDefault.uiLightSensorFull_BLD);
            EE_GETVAR(sSystemDefault.ucABP_CalibrateFlag, m_sDefSysSettings.sSystemDefault.ucABP_CalibrateFlag);

			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);
			EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);

            EE_GETVAR_SIZE(sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM_CUR_IDX], 50, m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM_CUR_IDX]);
            EE_GETVAR_SIZE(sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM1_IDX], 50, m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM1_IDX]);
            EE_GETVAR_SIZE(sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM2_IDX], 50, m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM2_IDX]);
            EE_GETVAR_SIZE(sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM3_IDX], 50, m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM3_IDX]);
            EE_GETVAR_SIZE(sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM4_IDX], 50, m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory[BLENDING_AP_MEM4_IDX]);

            palDataMgr_Access_MODEL_NAME(edaREAD, (void *)cModelName);
            sprintf(m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucModelName, "%s",cModelName);
#endif /* 0 */
            m_sDefSysSettings.sLayoutVersion.ucMajor = LAYOUT_VERSION_MAJOR;
            m_sDefSysSettings.sLayoutVersion.ucMinor = LAYOUT_VERSION_MINOR;
            m_sDefSysSettings.sLayoutVersion.ucSubminor = LAYOUT_VERSION_SUBMINOR;

            utilDataMgr_ResetToDefault(eNVRAM_EVENT_INITIALIZE);	//HICC2_Doulas_0003
            utilDataMgr_RAMToFileAll_CM(&m_sDefSysSettings);	//HICC2_Doulas_0003

			LOG_MSG(db_ALWAYS, "Hour %08d %08d\r\n",
                m_sDefSysSettings.sSystemDefault.ulTotalProjectorMinute,
                m_sDefSysSettings.sSystemDefault.ulLightSourceMinute[eTIMER_RECORD_BLD]);
		}

        /*=================================================================*/

        memcpy(&m_sAppDataMgrInfo.sEepSettings, &m_sDefSysSettings, sizeof(m_sDefSysSettings));

        //memcpy(&sVersion, &m_sAppDataMgrInfo.sEepSettings.sLayoutVersion, sizeof(sVersion)); //A70LV_Larry_0128

        //m_sAppDataMgrInfo.sEepSettings.sLayoutVersion.ucMajor = LAYOUT_VERSION_MAJOR;
        //m_sAppDataMgrInfo.sEepSettings.sLayoutVersion.ucMinor = LAYOUT_VERSION_MINOR;
        //m_sAppDataMgrInfo.sEepSettings.sLayoutVersion.ucSubminor = LAYOUT_VERSION_SUBMINOR;

        //utilDataMgr_CommonSet_CM(eDF_MAJOR, &m_sAppDataMgrInfo.sEepSettings.sLayoutVersion.ucMajor);
        //utilDataMgr_CommonSet_CM(eDF_MINOR, &m_sAppDataMgrInfo.sEepSettings.sLayoutVersion.ucMinor);
        //utilDataMgr_CommonSet_CM(eDF_SUBMINOR, &m_sAppDataMgrInfo.sEepSettings.sLayoutVersion.ucSubminor);
#if 0
        EE_PUTVAR(sLayoutVersion, m_sAppDataMgrInfo.sEepSettings.sLayoutVersion);
        //EE_PUTVAR(sUserSystemSetting, m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting);
        EE_PUTVAR(sBurin_Information, m_sAppDataMgrInfo.sEepSettings.sBurin_Information);
        EE_PUTVAR(sADC_cal_values, m_sAppDataMgrInfo.sEepSettings.sADC_cal_values);     //A70LV_Doulas_0015
        EE_PUTVAR(sWaveformstate, m_sAppDataMgrInfo.sEepSettings.sWaveformstate);     //A70LV_Doulas_0015
        EE_PUTVAR(sSystemDefault, m_sAppDataMgrInfo.sEepSettings.sSystemDefault);     //A70LV_Doulas_0015
        EE_PUTVAR(sTiming_Table, m_sAppDataMgrInfo.sEepSettings.sTiming_Table);         //A70LV_Doulas_0194 Add

        EE_GETVAR(sLayoutVersion, m_sAppDataMgrInfo.sEepSettings.sLayoutVersion);       //A70LV_Doulas_0018
        EE_GETVAR(sADC_cal_values, m_sAppDataMgrInfo.sEepSettings.sADC_cal_values);
        EE_GETVAR(sWaveformstate, m_sAppDataMgrInfo.sEepSettings.sWaveformstate);
        EE_GETVAR(sBurin_Information, m_sAppDataMgrInfo.sEepSettings.sBurin_Information);
        EE_GETVAR(sSystemDefault, m_sAppDataMgrInfo.sEepSettings.sSystemDefault);
        //EE_GETVAR(sUserSystemSetting, m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting);
        EE_GETVAR(sTiming_Table, m_sAppDataMgrInfo.sEepSettings.sTiming_Table);         //A70LV_Doulas_0194 Add
#endif /* 0 */
        palDataMgr_SystemInit(1); //A70LV_Larry_0217

        //GEC_ErrorCode_Reset(); //A70LV_John_0012 add error log reset while EEPROM version changed
        //utilStartup_DataMgrVersionSet((UINT16)sizeof(sVersion), (UINT8*)&sVersion); //A70LV_Larry_0128

        UINT8 ucSwitch = m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucModelSwitchAdjust;
		palFrontEndMgr_Model_Switch_Enable_Set(ucSwitch); //A65_OPTOMA_Julie_0065 //A35G2_Coda_0063

        //halFrontEndCtrl_EDID_SN_Set(eCM_SOURCE_RESERVED, eCM_EDID_TYPE_V20, m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucSerialNumber); //G100_Doulas_0082 remove//G100_John_0003 fix EDID version change issue
        ucValue = ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHDMI_EDID_1 & EDID_TYPE_MASK)<<HDMI1_EDID_TYPE2P0_SHIFT) +
			      ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHDMI_EDID_2 & EDID_TYPE_MASK)<<HDMI2_EDID_TYPE2P0_SHIFT) +
			      ((m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHDBaseT_EDID & EDID_TYPE_MASK)<<HDBASET_EDID_TYPE2P0_SHIFT);	//HICC2_Doulas_0068//G100_Doulas_0082

        eCM_NATIVE_TIMING NativeTiming ;
        palDataMgr_Access_Proj_Native_Timing(edaREAD, &NativeTiming);

		palFrontEndMgr_EDID_SN_Set(eCM_SOURCE_RESERVED, ucValue, NativeTiming, m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucSerialNumber);						//G100_Doulas_0082 Modify Add HDMI1/HDMI2/HDBaseT type
    }
    else    //A70LV_Doulas_0015
    {
        EE_GETVAR(sLayoutVersion, m_sAppDataMgrInfo.sEepSettings.sLayoutVersion);       //A70LV_Doulas_0018
        EE_GETVAR(sADC_cal_values, m_sAppDataMgrInfo.sEepSettings.sADC_cal_values);
        EE_GETVAR(sWaveformstate, m_sAppDataMgrInfo.sEepSettings.sWaveformstate);
        EE_GETVAR(sBurin_Information, m_sAppDataMgrInfo.sEepSettings.sBurin_Information);
        EE_GETVAR(sSystemDefault, m_sAppDataMgrInfo.sEepSettings.sSystemDefault);
        EE_GETVAR(sUserSystemSetting, m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting);
        EE_GETVAR(sTiming_Table, m_sAppDataMgrInfo.sEepSettings.sTiming_Table);         //A70LV_Doulas_0194 Add
        EE_GETVAR(uCCT_Table, m_sAppDataMgrInfo.sEepSettings.uCCT_Table); //HICC2_Steven_0018 //HICC2_Casper_0014 //HICC2_Casper_0018
        EE_GETVAR(sActuatorSetting, m_sAppDataMgrInfo.sEepSettings.sActuatorSetting); //HICC2_Doulas_0099
        //palDataMgr_sHSG_setting_Factory_Reset();
        //palDataMgr_sColorSetting_Factory_Reset();

        //utilDataMgr_RAMToFileSourceTableHSG(&m_sDefSysSettings);
        ///utilDataMgr_RAMToFileSourceTableColor(&m_sDefSysSettings);
    }

    #ifdef CUSTOM_CHRISTIE
    EE_GETVAR(sUserSystemSetting.sCommonSetting.ucCustomCode, m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucCustomCode);

    if(strcmp(m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucCustomCode, "08010200") == 0)
    {
        Board_MCU_ModelID_Set(MODULE_TYPE_ID1_PLATFORM);
    }
    else if(strcmp(m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucCustomCode, "08010300") == 0)
    {
        Board_MCU_ModelID_Set(MODULE_TYPE_ID2_PLATFORM);
    }
    else
    {
        Board_MCU_ModelID_Set(MODULE_TYPE_ID0_PLATFORM);
    }
    #endif

	#if 0 //Depends on X35Gen2 design
    palDataMgr_Check_ModelID();//G100_Steven_0039
	#endif
    //TODO
    /*
        To get eep data back
    */
    palDataMgr_OPDRegulatoryInfo(); //A65_OPTOMA_Julie_0080

    m_sAppDataMgrInfo.sSystemValues.ucTwistLink = 0; //A70LV_Larry_0168
    m_sAppDataMgrInfo.sSystemValues.ucTwistTestPattern = 0;
    m_sAppDataMgrInfo.sSystemValues.ucNetworkWaitMessage = 0; //A70LV_Larry_0278
    m_sAppDataMgrInfo.sSystemValues.ucNetworkResetFlag = 0; //A70LV_Larry_0278
    //m_sAppDataMgrInfo.sSystemValues.sFAN_INFO.ucHighAltitudeEnable = 0xFF;
    m_sAppDataMgrInfo.sSystemValues.sFAN_INFO.ucFanFilter = eFILTER_TYPE_NOFILTER; //HICC2_Doulas_0060
    m_sAppDataMgrInfo.sSystemValues.sSYSTEM_VARIABLE.ucPictureMute = 1; //G100_Larry_0008
    m_sAppDataMgrInfo.sSystemValues.sSYSTEM_VARIABLE.ucOSD_Show = 1; //G100_Larry_0007
    m_sAppDataMgrInfo.sSystemValues.sLD_INFO.uiLD_INFO_Num = Syscfg_Value_Get_Typeint(eLD_INFO_Num_defined_in_MCU);
    m_sAppDataMgrInfo.sSystemValues.sLD_INFO.uiBLD_Num = Syscfg_Value_Get_Typeint(eBLD_Num_defined_in_MCU);
    m_sAppDataMgrInfo.sSystemValues.sLD_INFO.uiRLD_Num = Syscfg_Value_Get_Typeint(eRLD_Num_defined_in_MCU);
    //G100_Owen_0005
    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHSGTestPatternCtrl = eCM_TEST_PATTERN_OFF; //HICC2_Doulas_0051
    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucImageFreeze = ets_OFF; //HICC2_Doulas_0053
    memset(m_sAppDataMgrInfo.sSystemValues.ucCurrentSecurityCode, ' ', 5);
    memcpy(m_sAppDataMgrInfo.sSystemValues.sVERSION.m_Camera_Version, "Not Supported", 15);
    memset(m_sAppDataMgrInfo.sSystemValues.sVERSION.m_OESN1, 0, VERSION_STRING_MAX_LENGTH); //H30K_Tim_0011, add
    memset(m_sAppDataMgrInfo.sSystemValues.sVERSION.m_OESN2, 0, VERSION_STRING_MAX_LENGTH); //H30K_Tim_0011, add

    m_sAppDataMgrInfo.sSystemValues.ucXPRFilter = 0;
    m_sAppDataMgrInfo.sSystemValues.ucDimPower = DIM_POWER_NUMBER_DEFAULT_VALUE; //HICC2_Doulas_0088
    m_sAppDataMgrInfo.sSystemValues.ucDimPowerFlag = 0;     //HICC2_Doulas_0088
    m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable = 0;
    m_sAppDataMgrInfo.sSystemValues.ucSystem_Busy = 0; //HICC2_Doulas_0119

    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable)
    {
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingLeftEnable)
        {
            m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_LEFT_ENABLE;
        }
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingRightEnable)
        {
            m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_RIGHT_ENABLE;
        }
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingTopEnable)
        {
            m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_TOP_ENABLE;
        }
        if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingBottomEnable)
        {
            m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_BOTTOM_ENABLE;
        }
    }

    m_sAppDataMgrInfo.sSystemValues.ucACU_Exectue             = eACU_NOT_EXECUTED;
    m_sAppDataMgrInfo.sSystemValues.ucACU_Status              = eACU_STATUS_NONE;
    m_sAppDataMgrInfo.sSystemValues.ucACU_Target_Tmp          = ACU_TARGET_DEFAULT;
    m_sAppDataMgrInfo.sSystemValues.ucACU_Reset_Status        = eACU_RESET_NOT_YET;
    m_sAppDataMgrInfo.sSystemValues.ucACU_Target_Status       = ets_OFF;
    m_sAppDataMgrInfo.sSystemValues.ucACU_Apply               = eACU_APPLY_IDLE;
    m_sAppDataMgrInfo.sSystemValues.ucACU_DisableBlend        = eACU_DISABLE_BLEND_NONE;
    m_sAppDataMgrInfo.sSystemValues.ucAuto_Focus_Execute        = eAF_AC_NOT_EXECUTE;
    m_sAppDataMgrInfo.sSystemValues.ucAuto_Wall_Color_Apply     = eAC_APPLY_IDLE;
    m_sAppDataMgrInfo.sSystemValues.ucAuto_Wall_Color_Execute   = eAF_AC_NOT_EXECUTE;
    m_sAppDataMgrInfo.sSystemValues.ucCamera_OSD_Locked         = eOSD_NOT_LOCKED;
    m_sAppDataMgrInfo.sSystemValues.ucCamera_Module_Status      = eCAMERA_MODULE_NOT_EXIST;
    m_sAppDataMgrInfo.sSystemValues.ucAuto_Focus_Status         = eAF_STATUS_TIMEOUT;
    m_sAppDataMgrInfo.sSystemValues.ucAuto_Wall_Color_Status    = eAWC_STATUS_TIMEOUT;
    m_sAppDataMgrInfo.sSystemValues.cGroupingAutoColorSelect    = 0;
    m_sAppDataMgrInfo.sSystemValues.sGroupingAutoColorMatchStatus.clientRet = 0xFF;
    m_sAppDataMgrInfo.sSystemValues.sGroupingAutoColorMatchStatus.functionRet = 0xFF;
    m_sAppDataMgrInfo.sSystemValues.sGroupingAutoFocusStatus.clientRet = 0xFF;
    m_sAppDataMgrInfo.sSystemValues.sGroupingAutoFocusStatus.functionRet = 0xFF;
    m_sAppDataMgrInfo.sSystemValues.ucSystemProtectStatus = ePASSWORD_PROTECT_UNLOCK;//HICC2_Julie_0065

	m_sAppDataMgrInfo.sSystemValues.ucQuickKeyLock = 0;// HICC2_Bruce_0011
	m_sAppDataMgrInfo.sSystemValues.ucEditNumber_5_Digits = 0;
	m_sAppDataMgrInfo.sSystemValues.ucSecurityPollCount = 0;// HICC2_Bruce_0016

    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lInputVoltage = 0;       //HICC2_Doulas_0165
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lPFC_OutputVoltage = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l5V_Voltage = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l12V_Voltage = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l52V_Voltage = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l5V_Current = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l12V_Current = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l52V_Current = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lPFC_MOS_Temperature = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lBridge_Temperature = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lLVPS_Ambient_Temperature = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l12V_SR_MOSFET_Temperature = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l52V_SR_MOSFET_Temperature = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.uiPRIMARY_PROTECTION_STATUS = 0;
    m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.uiPRIMARY_PROTECTION_STATUS = 0;

    utilDbgMsg_ReSet(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ulDbMask, m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ulDbMaskCT);

    GuiCb = Gui_fpCallbackGet();

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataMgr_PowerNormal(void)
{
    if(MutexTake())
    {
        char    cModelName[24] = {'\0'};
        char    cUnknow[13] = "Unknow_Model";
        UINT8   ucValue = 0;
        UINT8   ucValue2 = 0;

        palSystem_Model_ID_Get(); //A35G2_BRC_Casper_0023

		//G100_Clare_0066, add, <<<
        if(m_ucResetAll == TRUE)    //G100_Owen_0046
        {
            m_sDefSysSettings.sLayoutVersion.ucMajor = LAYOUT_VERSION_MAJOR;
            m_sDefSysSettings.sLayoutVersion.ucMinor = LAYOUT_VERSION_MINOR;
            m_sDefSysSettings.sLayoutVersion.ucSubminor = LAYOUT_VERSION_SUBMINOR;

            utilDataMgr_ResetToDefault(eNVRAM_EVENT_RESETALL);	//HICC2_Doulas_0003
            utilDataMgr_RAMToFileAll_CM(&m_sDefSysSettings);	//HICC2_Doulas_0003

            memcpy(&m_sAppDataMgrInfo.sEepSettings, &m_sDefSysSettings, sizeof(m_sDefSysSettings));
            memset(&m_sAppDataMgrInfo.sSystemValues , 0 , sizeof(m_sAppDataMgrInfo.sSystemValues));

            m_sAppDataMgrInfo.sSystemValues.ucTwistLink = 0; //A70LV_Larry_0168
            m_sAppDataMgrInfo.sSystemValues.ucTwistTestPattern = 0;
            m_sAppDataMgrInfo.sSystemValues.ucNetworkWaitMessage = 0; //A70LV_Larry_0278
            m_sAppDataMgrInfo.sSystemValues.ucNetworkResetFlag = 0; //A70LV_Larry_0278
            m_sAppDataMgrInfo.sSystemValues.sSYSTEM_VARIABLE.ucPictureMute = 1; //G100_Larry_0008
            m_sAppDataMgrInfo.sSystemValues.sSYSTEM_VARIABLE.ucOSD_Show = 1; //G100_Larry_0007
            m_sAppDataMgrInfo.sSystemValues.sLD_INFO.uiLD_INFO_Num = Syscfg_Value_Get_Typeint(eLD_INFO_Num_defined_in_MCU);
            m_sAppDataMgrInfo.sSystemValues.sLD_INFO.uiBLD_Num = Syscfg_Value_Get_Typeint(eBLD_Num_defined_in_MCU);
            m_sAppDataMgrInfo.sSystemValues.sLD_INFO.uiRLD_Num = Syscfg_Value_Get_Typeint(eRLD_Num_defined_in_MCU);
            //G100_Owen_0005
            m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHSGTestPatternCtrl = eCM_TEST_PATTERN_OFF; //HICC2_Doulas_0051
            memset(m_sAppDataMgrInfo.sSystemValues.ucCurrentSecurityCode, ' ', 5);
            memcpy(m_sAppDataMgrInfo.sSystemValues.sVERSION.m_Camera_Version, "Not Supported", 15);

            m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable = 0;
            m_sAppDataMgrInfo.sSystemValues.ucLensCenterSetting = 0;

            if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable)
            {
                if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingLeftEnable)
                {
                  m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_LEFT_ENABLE;
                }
                if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingRightEnable)
                {
                  m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_RIGHT_ENABLE;
                }
                if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingTopEnable)
                {
                  m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_TOP_ENABLE;
                }
                if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucBlendingBottomEnable)
                {
                  m_sAppDataMgrInfo.sSystemValues.ucUIBlendingEnable |= BLEND_MENU_BOTTOM_ENABLE;
                }
            }

            utilDataMgr_LanInit_CM(&m_sLAN_info);	//HICC2_Doulas_0003
            palDataMgr_LAN_IP_Copy1_All(); //G100_Larry_0035
			utilDataMgr_ArtNetInit_CM(&m_sArtNetInfo);// HICC2_Bruce_0017
            utilWarp_ADVWarpClearAllFile();     //G100_Doulas_0027
            utilDataMgr_UserDataReset_CM();		//G100_Doulas_0075

            utilDbgMsg_ReSet(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ulDbMask, m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ulDbMaskCT);

			palDataMgr_Access_LensMemory_Clear(edaWRITE_THROUGH_WITH_ACTION, NULL); //A35G2_Larry_0069
			palMotor_LensCenterSettingSet(&m_sAppDataMgrInfo.sSystemValues.ucLensCenterSetting); //HICC2_Casper_0037

            m_ucResetAll = FALSE;

            palLANProc_Init_Reset_All(); //HICC2_Doulas_0062
            palLANProcSendToLAN(edcSNMP_RESET); 			//A35G2_CDS_Larry_0009
            palLANProcSendToLAN(edcNETWORK_CONTROL_RESET);  //A35G2_CDS_Larry_0009
            palLANProcSendToLAN(edcNETWORK_FACTORY_RESET);  //A35G2_CDS_Larry_0009

            LOG_MSG(db_ALWAYS, "Reset All\r\n"); //HICC2_Doulas_0062
        }

        m_sAppDataMgrInfo.sSystemValues.ucXPRFilter = 0;
        m_sAppDataMgrInfo.sSystemValues.ucDimPower = DIM_POWER_NUMBER_DEFAULT_VALUE; //HICC2_Doulas_0088
        m_sAppDataMgrInfo.sSystemValues.ucDimPowerFlag = 0;     //HICC2_Doulas_0088
        m_DimNum = DIM_POWER_NUMBER_DEFAULT_VALUE;  //H2PF_Simon_0068
        utilDataMgr_CommonGet_CM(eDF_WAVEFORMGAIN, (UINT8*)&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucWaveformGain);
        //utilDataMgr_ConstantPowerNumber_StorageGet(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucWaveformGain);  //H2PF_Simon_0068
		m_sAppDataMgrInfo.sSystemValues.sFAN_INFO.ucHighAltitudeEnable = 0xFF; //A70Gen2_Julie_0025
        m_sAppDataMgrInfo.sSystemValues.ucDimPower = DIM_POWER_NUMBER_DEFAULT_VALUE;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucImageFreeze = ets_OFF; //A70Gen2_Julie_0103
        m_sAppDataMgrInfo.sSystemValues.ucDMD_Airtight_Status  = 1; //HICC2_Steven_0032
        m_sAppDataMgrInfo.sSystemValues.ucSystem_Busy = 0; //HICC2_Doulas_0119
        m_sAppDataMgrInfo.sSystemValues.ucActuatorPattern = 0;
        m_sAppDataMgrInfo.sSystemValues.ucActuatorPatternChange = 0;

        m_ucLastInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;

        //HICC2_Julie_0055
        sprintf((char*)m_sAppDataMgrInfo.sSystemValues.sVERSION.m_FPGA1_Version, "XFF.FF");
        sprintf((char*)m_sAppDataMgrInfo.sSystemValues.sVERSION.m_FPGA2_Version, "YFF.FF");
        sprintf((char*)m_sAppDataMgrInfo.sSystemValues.sVERSION.m_FPGA3_Version, "ZFF.FF");
        sprintf((char*)m_sAppDataMgrInfo.sSystemValues.sVERSION.m_KeyPad_Version, "KFF.FF");
        sprintf((char*)m_sAppDataMgrInfo.sSystemValues.sVERSION.m_XFPGA_Version, "QFF.FF");

        #ifndef SCALER_FPGA_F34
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable = 1;   //HICC2_Simon_0011
        #endif

        palEnvironment_LensSpecialFlag_Set(FALSE); //A35G2_Wesley_0163 //A70Gen2_Julie_0109//HICC2_Julie_0014
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLensSpecialLocation = eCM_LENS_MOVE_TOP; //A35G2_Wesley_0165 //A70Gen2_Julie_0109//HICC2_Julie_0014

        {
            UINT32 Result = 0;
            UINT32 ulCount = 0;

            Result = ConfConvCmd_getProjectGroupSearchCmd(&m_sAppDataMgrInfo.sSystemValues.sGroupingSearch);

            if(Result == 0) //pass
			{
                m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.ucCnt = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Cnt;

                for(ulCount = 0; ((ulCount < m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Cnt) && (ulCount < GROUPING_MENU_SEARCH_ITEM)); ulCount++)
                {
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].ucEnable = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].GroupStatus;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].ucCameraStatus = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].CameraStatus;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].ucGroupStatus = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].ProjectorStatus;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].uiFd = (UINT16)m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].Fd;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].uiGroupId = (UINT16)m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].GroupId;

                    snprintf(m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].cName, 32, "%s", m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].ModeName);
                    snprintf(m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSearch.sItem[ulCount].cIP, 20, "%s", m_sAppDataMgrInfo.sSystemValues.sGroupingSearch.Info[ulCount].Ip);
                }
            }

            Result = ConfConvCmd_getProjectGroupSelectCmd(&m_sAppDataMgrInfo.sSystemValues.sGroupingSelect);

            if(Result == 0) //pass
            {
                m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.ucCnt = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Cnt;

                for(ulCount = 0; ((ulCount < m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Cnt) && (ulCount < GROUPING_MENU_SELECT_ITEM)); ulCount++)
                {
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].ucEnable = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].GroupStatus;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].ucCameraStatus = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].CameraStatus;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].ucGroupStatus = (UINT8)m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].ProjectorStatus;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].uiFd = (UINT16)m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].Fd;
                    m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].uiGroupId = (UINT16)m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].GroupId;
                    snprintf(m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].cName, 32, "%s", m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].ModeName);
                    snprintf(m_sAppDataMgrInfo.sSystemValues.sGroupingMenuSelect.sItem[ulCount].cIP, 20, "%s", m_sAppDataMgrInfo.sSystemValues.sGroupingSelect.Info[ulCount].Ip);
                }
            }
            palLANProcSendToLAN(edcGROUPINGMENU_SEARCH_LIST);
            palLANProcSendToLAN(edcGROUPINGMENU_SELECT_LIST);
        }

        m_sAppDataMgrInfo.sSystemValues.ucACU_Exectue             = eACU_NOT_EXECUTED;   //H2PF_Simon_0036 Start
        m_sAppDataMgrInfo.sSystemValues.ucACU_Status              = eACU_STATUS_NONE;
        m_sAppDataMgrInfo.sSystemValues.ucACU_Target_Tmp          = ACU_TARGET_DEFAULT;
        m_sAppDataMgrInfo.sSystemValues.ucACU_Reset_Status        = eACU_RESET_NOT_YET;
        m_sAppDataMgrInfo.sSystemValues.ucACU_Target_Status       = ets_OFF;
        m_sAppDataMgrInfo.sSystemValues.ucACU_Apply               = eACU_APPLY_IDLE;    //HICC2_Simon_0013
        m_sAppDataMgrInfo.sSystemValues.ucACU_DisableBlend        = eACU_DISABLE_BLEND_NONE;
        m_sAppDataMgrInfo.sSystemValues.ucAuto_Focus_Execute        = eAF_AC_NOT_EXECUTE;
        m_sAppDataMgrInfo.sSystemValues.ucAuto_Wall_Color_Apply     = eAC_APPLY_IDLE;		//A35G2_Sammy_0002
        m_sAppDataMgrInfo.sSystemValues.ucAuto_Wall_Color_Execute   = eAF_AC_NOT_EXECUTE;  //H2PF_Simon_0036 End
        m_sAppDataMgrInfo.sSystemValues.ucCamera_OSD_Locked         = eOSD_NOT_LOCKED;
        //m_sAppDataMgrInfo.sSystemValues.ucCamera_Module_Status      = eCAMERA_MODULE_NOT_EXIST;	//HICC2_Steven_0057 fixed ISS-0026345
        m_sAppDataMgrInfo.sSystemValues.ucAuto_Focus_Status         = eAF_STATUS_TIMEOUT;
        m_sAppDataMgrInfo.sSystemValues.ucAuto_Wall_Color_Status    = eAWC_STATUS_TIMEOUT; //A35G2_Coda_0052 //A35G2_Coda_0055
        m_sAppDataMgrInfo.sSystemValues.cGroupingAutoColorSelect    = eAC_APPLY_IDLE;
        m_sAppDataMgrInfo.sSystemValues.sGroupingAutoColorMatchStatus.clientRet = 0xFF;
        m_sAppDataMgrInfo.sSystemValues.sGroupingAutoColorMatchStatus.functionRet = 0xFF;
        m_sAppDataMgrInfo.sSystemValues.sGroupingAutoFocusStatus.clientRet = 0xFF;
        m_sAppDataMgrInfo.sSystemValues.sGroupingAutoFocusStatus.functionRet = 0xFF;
        m_sAppDataMgrInfo.sSystemValues.ucSystemProtectStatus = ePASSWORD_PROTECT_UNLOCK;//HICC2_Julie_0065

		m_sAppDataMgrInfo.sSystemValues.ucQuickKeyLock = 0;// HICC2_Bruce_0011

        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lInputVoltage = 0;       //HICC2_Doulas_0165
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lPFC_OutputVoltage = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l5V_Voltage = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l12V_Voltage = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l52V_Voltage = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l5V_Current = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l12V_Current = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l52V_Current = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lPFC_MOS_Temperature = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lBridge_Temperature = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.lLVPS_Ambient_Temperature = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l12V_SR_MOSFET_Temperature = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.l52V_SR_MOSFET_Temperature = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.uiPRIMARY_PROTECTION_STATUS = 0;
        m_sAppDataMgrInfo.sSystemValues.sLVPS_Info.uiPRIMARY_PROTECTION_STATUS = 0;

        palLANProcSendToLAN(edcACU_TARGET_STATUS); //HICC2_Doulas_0142

        palDataMgr_ACU_Target_Status_Set(m_sAppDataMgrInfo.sSystemValues.ucACU_Target_Status);//HICC2_Julie_0044

        palDataMgr_Data_Access(edcSHOW_MESSAGES, edaREAD, &ucValue);
        palGeo_Draw_FrameBufferShowMessage(ucValue);
        palDataMgr_Data_Access(edcWARP_TOGGLE, edaREAD, &ucValue);
        palGeo_Draw_FrameBufferGeometryEnable(ucValue);
        palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &ucValue);
        palGeo_Draw_FrameBufferAutoSource(ucValue);
        palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, &ucValue);
        palGeo_Draw_FrameBufferColorSpace(ucValue);

        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHSGTestPatternCtrl = eCM_TEST_PATTERN_OFF;
		palLANProcSendToLAN(edcHSG_TEST_PATTERN_CTRL);// HICC2_Bruce_0018

		//G100_Clare_0068, mod, >>>
        //palDataMgr_Check_ModelID();
        //palDataMgr_Access_MODEL_NAME(edaREAD, (void *)cModelName);
        //sprintf(m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucModelName, "%s",cModelName);
		//G100_Clare_0068, mod, <<<
	#if defined(CUSTOM_BARCO) //HICC2_Doulas_0083 Modify//A35G2_Coda_0064
        palDataMgr_sHSG_setting_Factory_Reset();
        palDataMgr_sColorSetting_Factory_Reset();

        utilDataMgr_RAMToFileSourceTableHSG_CM(&m_sDefSysSettings, eDATA_NODE_NUMBER);	//HICC2_Doulas_0003
        utilDataMgr_RAMToFileSourceTableColor_CM(&m_sDefSysSettings, eDATA_NODE_NUMBER);	//HICC2_Doulas_0003
    #else //OPTOMA only reset default HSG value , don't reset user hsg value. //A35G2_Coda_0106
        palDataMgr_sHSG_setting_Init_Default();
	  #ifdef HSG_COLORTEMPERATURE
        palDataMgr_sHSG_ColorTemperature_Init_Default();
	  #endif /* HSG_COLORTEMPERATURE */

        //UINT8 ucOsdLockStatus = eOSD_NOT_LOCKED;                                                    //A35G2_Owen_0011
        //if(eEXEC_CODE_PASS == utilDataMgr_CommonGet(eDF_PROSERVICE_OSD_LOCK, &ucOsdLockStatus))     HICC2_Doulas_0083//A35G2_Owen_0011
        //{
        //    palDataMgr_Access_OSD_Lock(edaWRITE_RAM_ONLY_WITH_ACTION, &ucOsdLockStatus);
        //}
    #endif

        palDataMgr_GetReleaseVersion();
        MutexGive();

		palEnvironment_Send_PIN_StatusSet(FALSE);
        palDataMgr_PIN_Protect_Status();// HICC2_Bruce_0001

		palDataMgr_Set_OSDLockTemp(eOSD_LOCK_NUMBER);
		palDataMgr_Access_OSD_Lock(edaREAD, &ucValue);
		palDataMgr_Access_OSD_Lock(edaWRITE_THROUGH_WITH_ACTION, &ucValue);

#ifndef SIMULATOR_ISCALER
        if(strcmp(cModelName, cUnknow) == 0)	//G100_Clare_0067
        {
            UINT32 ulErrorIndex = 0 ;
            ulErrorIndex = UARTError ;
            utilDataMgr_WriteGecLog_CM(ulErrorIndex);	//HICC2_Doulas_0003
            return eEXEC_CODE_FAIL;
        }
#endif /* SIMULATOR_ISCALER */
#if 0// HICC2_Bruce_0023
		palDataMgr_Data_Access(edcENERGY_SAVING, edaREAD, &ucValue);
		palDataMgr_Data_Access(edcSTANDBY_MODE, edaREAD, &ucValue2);
		if(ucValue == ets_ON && ucValue2 != eCM_STANDBY_MODE_0_5W)
		{
			ucValue2 = eCM_STANDBY_MODE_0_5W;
			palDataMgr_Data_Access(edcSTANDBY_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucValue2);
		}
#endif
        return eEXEC_CODE_PASS;
    }

    return eEXEC_CODE_FAIL;
}

void palDataMgr_BISTToDefault(void)  //HICC2_Steven_0012
{
	UINT8 ucData = 0;

	palDataMgr_Data_Access(edcBIST_CHECK,  edaREAD, &ucData);

	if(ucData == BIST_STATUS_BUSY)
	{
		ucData = BIST_STATUS_INIT;

		//LOG_MSG(db_ALWAYS, "BIST busy fail \r\n");
		palDataMgr_Data_Access(edcBIST_CHECK, edaWRITE_THROUGH_WITH_ACTION, &ucData);
	}
}

void palDataMgr_ResetAllToDefault(void)
{
    if(m_ucResetAll == FALSE)   //G100_Owen_0046
    {
        m_ucResetAll = TRUE;
    }
}

void palDataMgr_Scaler_EEPROM_Iint(void)  //A70LV_Doulas_0013
{
    #if 0
    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucMainLayout = 3;
    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.ucLanguage = 2;
    #endif
    palImgMgr_Init_EEPROM_Setting(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting);   //A70LV_Doulas_0016
}


//A70LV_Doulas_0015
void palDataMgr_sLayoutVersion_Init_Default(void)
{
    UINT8 uci = 0;
    m_sDefSysSettings.sLayoutVersion.ucMajor    = LAYOUT_VERSION_MAJOR;
    m_sDefSysSettings.sLayoutVersion.ucMinor    = LAYOUT_VERSION_MINOR;
    m_sDefSysSettings.sLayoutVersion.ucSubminor = LAYOUT_VERSION_SUBMINOR;
    for(; uci < 17 ; uci++)   //A70LV_Doulas_0017
        m_sDefSysSettings.sLayoutVersion.ucFirmwareID[uci] = 0;
}

void palDataMgr_sADC_cal_values_Init_Default(void)  //A70LV_Doulas_0124 Modify
{
    UINT8 i;
    for(i=0 ; i < 3 ; i++)
    {
        //m_sDefSysSettings.sADC_cal_values.uiRGBGain[i]     = 0;
        m_sDefSysSettings.sADC_cal_values.uiRGBOffset[i]   = 0;
        //m_sDefSysSettings.sADC_cal_values.uiYUVGain[i]     = 0;
        m_sDefSysSettings.sADC_cal_values.uiYUVOffset[i]   = 0;
        //m_sDefSysSettings.sADC_cal_values.uiRGBGain2[i]    = 0;
        m_sDefSysSettings.sADC_cal_values.uiRGBOffset2[i]  = 0;
        //m_sDefSysSettings.sADC_cal_values.uiYUVGain2[i]    = 0;
        m_sDefSysSettings.sADC_cal_values.uiYUVOffset2[i]  = 0;
    }
    m_sDefSysSettings.sADC_cal_values.uiRGBGain[0] = ADC_RGB1_GAIN0_DEFAULT;//802;      //A70LV_Doulas_0171 modify
    m_sDefSysSettings.sADC_cal_values.uiRGBGain[1] = ADC_RGB1_GAIN1_DEFAULT;//804;
    m_sDefSysSettings.sADC_cal_values.uiRGBGain[2] = ADC_RGB1_GAIN2_DEFAULT;//804;

    m_sDefSysSettings.sADC_cal_values.uiYUVGain[0] = ADC_YUV1_GAIN0_DEFAULT;//540;      //A70LV_Doulas_0171 modify
    m_sDefSysSettings.sADC_cal_values.uiYUVGain[1] = ADC_YUV1_GAIN1_DEFAULT;//680;
    m_sDefSysSettings.sADC_cal_values.uiYUVGain[2] = ADC_YUV1_GAIN2_DEFAULT;//535;

    m_sDefSysSettings.sADC_cal_values.uiRGBGain2[0] = ADC_RGB2_GAIN0_DEFAULT;//801;     //A70LV_Doulas_0171 modify
    m_sDefSysSettings.sADC_cal_values.uiRGBGain2[1] = ADC_RGB2_GAIN1_DEFAULT;//801;
    m_sDefSysSettings.sADC_cal_values.uiRGBGain2[2] = ADC_RGB2_GAIN2_DEFAULT;//808;

    m_sDefSysSettings.sADC_cal_values.uiYUVGain2[0] = ADC_YUV2_GAIN0_DEFAULT;//540;     //A70LV_Doulas_0171 modify
    m_sDefSysSettings.sADC_cal_values.uiYUVGain2[1] = ADC_YUV2_GAIN1_DEFAULT;//680;
    m_sDefSysSettings.sADC_cal_values.uiYUVGain2[2] = ADC_YUV2_GAIN2_DEFAULT;//535;

}

void palDataMgr_sWaveformstate_Init_Default(void)
{
    m_sDefSysSettings.sWaveformstate.uiVersion              = 0;
    m_sDefSysSettings.sWaveformstate.ucUpdateStart          = 0;
    m_sDefSysSettings.sWaveformstate.ucUpdateComplete       = 0;
    m_sDefSysSettings.sWaveformstate.ucUpdateStatus         = 0;
    m_sDefSysSettings.sWaveformstate.ucWAVEFORMSTATE_CRC    = WAVEFORMSTATE_RESERVED_NUM;
}

void palDataMgr_sBurin_Information_Init_Default(void)
{
    //Fill burnin structure
    m_sDefSysSettings.sBurin_Information.ucBurnInEnable = ets_OFF;
    m_sDefSysSettings.sBurin_Information.ucLampOn = FW_BURNIN_TIME_ON_DEFAULT_VALUE;
    m_sDefSysSettings.sBurin_Information.ucLampOff = FW_BURNIN_TIME_OFF_DEFAULT_VALUE;
    m_sDefSysSettings.sBurin_Information.uiBurnInCycle = FW_BURNIN_CYCLE_DEFAULT_VALUE;
    m_sDefSysSettings.sBurin_Information.ucBurnInMode = 0;
    m_sDefSysSettings.sBurin_Information.ucBurnInAlwaysOn = 0;
    m_sDefSysSettings.sBurin_Information.ucBurnIn_CRC = BURNIN_RESERVED_NUM;

}

void palDataMgr_sSystemDefault_Init_Default(void)
{
	UINT8 ucData = 0;	//G100_Doulas_0077
    sprintf((char*)m_sDefSysSettings.sSystemDefault.ucSerialNumber, "31211199"); //HICC2_Doulas_0079

    m_sDefSysSettings.sSystemDefault.ucFirstStartupFlag = (UINT8)ets_ON;
    m_sDefSysSettings.sSystemDefault.ucUSTFirstStartup = (UINT8)ets_OFF;

    m_sDefSysSettings.sSystemDefault.ucPINProtect       = (UINT8)ets_OFF;
	m_sDefSysSettings.sSystemDefault.ucPassWord[0]		= 0xFF;//A65_OPTOMA_David_0014
 	m_sDefSysSettings.sSystemDefault.ucPassWord[1]	   	= 0xFF;//A65_OPTOMA_David_0014
	m_sDefSysSettings.sSystemDefault.ucPassWord[2]	   	= 0xFF;//A65_OPTOMA_David_0014
	m_sDefSysSettings.sSystemDefault.ucPassWord[3]	   	= 0xFF;//A65_OPTOMA_David_0014
	m_sDefSysSettings.sSystemDefault.ucPassWord[4]	   	= 0xFF;//A65_OPTOMA_David_0014


    m_sDefSysSettings.sSystemDefault.uiColorWheelIndex[0] = PW_INDEX_DEFAULT_VALUE;   //2x
    m_sDefSysSettings.sSystemDefault.uiColorWheelIndex[1] = PW_INDEX_DEFAULT_VALUE;   //3x
    m_sDefSysSettings.sSystemDefault.uiFWIndex[0] = FW_INDEX_DEFAULT_VALUE;   //2x
    m_sDefSysSettings.sSystemDefault.uiFWIndex[1] = FW_INDEX_DEFAULT_VALUE;   //3x
    m_sDefSysSettings.sSystemDefault.uiPWIndex[0] = PW_INDEX_DEFAULT_VALUE;   //2x
    m_sDefSysSettings.sSystemDefault.uiPWIndex[1] = PW_INDEX_DEFAULT_VALUE;   //3x

    m_sDefSysSettings.sSystemDefault.ucPOWER_OFFSET = 0; //T100_Casper_0003
    m_sDefSysSettings.sSystemDefault.ucCOLOR_OFFSET = 100;

    m_sDefSysSettings.sSystemDefault.ulTotalProjectorMinute = 0;
    m_sDefSysSettings.sSystemDefault.ulLightSourceMinute[eTIMER_RECORD_BLD] = 0;
    m_sDefSysSettings.sSystemDefault.ulLightSourceMinute[eTIMER_RECORD_RLD] = 0;
    m_sDefSysSettings.sSystemDefault.ulWaterCoolingMinute = 0;

    //ABC
//    m_sDefSysSettings.sSystemDefault.uiTargetBright = 0;
//    m_sDefSysSettings.sSystemDefault.ucpwrOff_ABC   = 0;
//    m_sDefSysSettings.sSystemDefault.ucTargetIntensity[0] = 0;
//    m_sDefSysSettings.sSystemDefault.ucTargetIntensity[1] = 0;
//    m_sDefSysSettings.sSystemDefault.ucTargetIntensity[2] = 0;
//    m_sDefSysSettings.sSystemDefault.ucTargetIntensity[3] = 0;
//    m_sDefSysSettings.sSystemDefault.ucTargetIntensity[4] = 0;
//    m_sDefSysSettings.sSystemDefault.ucTargetIntensity[5] = 0;


    m_sDefSysSettings.sSystemDefault.ucABP_CalibrateFlag = (UINT8)ets_OFF; //A70LV_Doulas_0023
    //memset(m_sDefSysSettings.sSystemDefault.uiLightSensorFull_BLD, 0x00, 96);  //G100_Doulas_0077 remove
	for(;ucData < eLD_SEQ_NUMBER;ucData++)			//G100_Doulas_0077 Modify
	{
    	m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorFull_BLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorEco_BLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightFull_PWM_BLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightEco_PWM_BLD[ucData] = 0;

		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorFull_RLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorEco_RLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightFull_PWM_RLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightEco_PWM_RLD[ucData] = 0;

		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorFull_Dynamic_RLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorEco_Dynamic_RLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightFull_PWM_Dynamic_RLD[ucData] = 0;
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightEco_PWM_Dynamic_RLD[ucData] = 0;

        m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucLightSensorTime[ucData] = 0;			//A65_OPTOMA_Doulas_0105
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorTarget[ucData] = 0;			//A65_OPTOMA_Doulas_0105
		m_sAppDataMgrInfo.sEepSettings.sSystemDefault.uiLightSensorTargetRLD[ucData] = 0;		//A65_OPTOMA_Doulas_0105
	}


    //lens
    m_sDefSysSettings.sSystemDefault.ucUST_LensInstallType = ets_OFF;//eUST_LensInstall_Cancel;    //A16 lens //A35G2_BRC_Casper_0088
    m_sDefSysSettings.sSystemDefault.ucLensType = eLENS_ID_NVRAM_ID;

//    m_sDefSysSettings.sSystemDefault.ucDisableSnapshots;
//    m_sDefSysSettings.sSystemDefault.cAutoKeystoneOffset;
//    m_sDefSysSettings.sSystemDefault.ucMCU_PorgramFinish;
    m_sDefSysSettings.sSystemDefault.ulDbMask = db_MSG;
    m_sDefSysSettings.sSystemDefault.ulDbMaskCT = db_MSG; //HICC2_Doulas_0131
//    m_sDefSysSettings.sSystemDefault.ucWF_UpgradeFlag;
    m_sDefSysSettings.sSystemDefault.ucOPFU_Check = FALSE;    //G100_Simon_0064

    //m_sDefSysSettings.sSystemDefault.ucPanel = ePANEL_ID_1080P_60HZ;  //To do...

#if 0
#if defined (DMD_DHD) //A70LV_Larry_0021
    m_sDefSysSettings.sSystemDefault.ucPanel = ePANEL_ID_1080P_60HZ;  //To do...
#elif defined (DMD_WUXGA)
    m_sDefSysSettings.sSystemDefault.ucPanel = ePANEL_ID_WUXGA_60HZ;  //To do...
#else
    m_sDefSysSettings.sSystemDefault.ucPanel = ePANEL_ID_1080P_60HZ;  //To do...
#endif /* DMD_DHD */
#endif /* 0 */
    m_sDefSysSettings.sSystemDefault.ucPanel = PANEL_2D_OUTPUT;

    m_sDefSysSettings.sSystemDefault.ucBIST = eBIST_STATUS_INIT;   //HICC2_Steven_0012
    m_sDefSysSettings.sSystemDefault.ucSYSTEM_DEFAULT_CRC = SYSTEM_DEFAULT_RESERVED_NUM;
}

void appDataMgr_sActuator_Init_Default(void) //HICC2_Doulas_0099
{
#if defined(PLATFORM_R70K) //HICC2_AC_0014 //for DLP7541
    m_sDefSysSettings.sActuatorSetting.ucDAC_AWC0 = 64;
    m_sDefSysSettings.sActuatorSetting.ulSubframeDelay_AWC0 = 1200;
    m_sDefSysSettings.sActuatorSetting.uiSegmentLength_AWC0 = 260;

    m_sDefSysSettings.sActuatorSetting.ucDAC_AWC1 = 90;
    m_sDefSysSettings.sActuatorSetting.ulSubframeDelay_AWC1 = 1250;
    m_sDefSysSettings.sActuatorSetting.uiSegmentLength_AWC1 = 260;
#else
    m_sDefSysSettings.sActuatorSetting.ucDAC_AWC0 = 150;
    m_sDefSysSettings.sActuatorSetting.ulSubframeDelay_AWC0 = 10000; //X10 to ddp
    m_sDefSysSettings.sActuatorSetting.uiSegmentLength_AWC0 = 335; //H60_Doulas_0028 modify

    m_sDefSysSettings.sActuatorSetting.ucDAC_AWC1 = 110; //H60_Doulas_0028 modify
    m_sDefSysSettings.sActuatorSetting.ulSubframeDelay_AWC1 = 10000; //X10 to ddp
    m_sDefSysSettings.sActuatorSetting.uiSegmentLength_AWC1 = 335; //H60_Doulas_0028 modify
#endif
}

void palDataMgr_sUserSystemSetting_Init_Default(void)
{
	palDataMgr_Model_ID_Set();
    palDataMgr_sLightSetting_Init_Default();
    palDataMgr_sOSD_Setting_Init_Default();
    palDataMgr_sNetworkSetting_Init_Default();
    palDataMgr_sImageSetting_Init_Default();
    palDataMgr_sWarpSetting_Init_Default();
    palDataMgr_sCommonSetting_Init_Default();
    palDataMgr_sSourceSetting_Init_Default();
    palDataMgr_sManualAdjustment_Init_Default();
    palDataMgr_sSourceDependSetting_Init_Default();
    palDataMgr_sHSG_setting_Init_Default();
    palDataMgr_sColorSetting_Init_Default();
    palDataMgr_sPWM_setting_Init_Default();
    palDataMgr_sGamma_Init_Default();
    appDataMgr_sActuator_Init_Default(); //HICC2_Doulas_0099
#ifdef HSG_COLORTEMPERATURE
    palDataMgr_sHSG_ColorTemperature_Init_Default();
#endif /* HSG_COLORTEMPERATURE */
}

void palDataMgr_sTiming_Table_Init_Default(void)    //A70LV_Doulas_0194
{
    UINT8 ucVal = 0;
    for(; ucVal < 20 ; ucVal++)
    {
        m_sDefSysSettings.sTiming_Table.ucNewMA_Mode_Flag[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.ucNewMA_Source_ID[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.ucNewMA_Table_ID[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_Mode_ID[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_HPeriod[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_HTotal[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_VTotal[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_HActive[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_VActive[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_HStart[ucVal] = 0;
        m_sDefSysSettings.sTiming_Table.uiNewMA_VStart[ucVal] = 0;
    }
}

void palDataMgr_sLightSetting_Init_Default(void)
{
    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucPowerMode = eCM_POWER_MODE_CONSTANT_POWER;   //light source mode //A70LV_Doulas_0024
    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucWaveformGain = CONSTANT_POWER_NUMBER_DEFAULT_VALUE;     //light strength
//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucLampIntensity;
//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucABPmode;
//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucInstantPowerOff;
    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucLightCalibrationMode = eLIGHT_SENSOR_CALIBRATION_MODE_DEFAULT; //A70LV_Doulas_0024

//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucLampLowPower ;
//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucCurrentLamp;
//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucLampAutoSwitch;
//    m_sDefSysSettings.sUserSystemSetting.sLightSetting.uiLampAutoSwitchTime;
	m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucConstantBrightness = ets_OFF;		//G100_Doulas_0008

    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucLIGHT_SETTING_CRC = USER_LIGHT_SETTING_RESERVED_NUM;
}

void palDataMgr_sOSD_Setting_Init_Default(void)
{
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucMenuLocation = (UINT8)eCM_MENU_LOCATION_CENTER;
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucOSDTranslucency = OSD_TRANSLUCENCY_DEFAULT;
	#if defined(CUSTOM_BARCO) //A35G2_Coda_0058
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucOSDTimeout   = eCM_OSD_TIMEOUT_15_SEC;
    #elif defined(CUSTOM_OPTOMA)
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucOSDTimeout   = eCM_OSD_TIMEOUT_10_SEC;
	#else
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucOSDTimeout   = eCM_OSD_TIMEOUT_60_SEC;
	#endif
	#ifdef CUSTOM_OPTOMA
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucShowMessages = (UINT8)ets_OFF;// BruceLin#20201222, Information Hide
    #else
	m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucShowMessages = (UINT8)ets_ON;
	#endif
	m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucMenuOffsetX  = OSD_MENUOFFSET_X_DEFAULT;
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucMenuOffsetY  = OSD_MENUOFFSET_Y_DEFAULT;
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucMenuLockout  = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucMenuTransparency = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucSearchScreen = (UINT8)ets_OFF;

    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.uiOPDPeriod = OPD_PERIOD_MIN_VALUE;  //HICC2_Steven_0009
    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucOSD_SETTING_CRC = USER_OSD_SETTING_RESERVED_NUM;
}

void palDataMgr_sNetworkSetting_Init_Default(void)
{
    m_sDefSysSettings.sUserSystemSetting.sNetworkSetting.ucShowNetworkMessages = (UINT8)ets_ON;
    m_sDefSysSettings.sUserSystemSetting.sNetworkSetting.ucDHCP = (UINT8)ets_OFF;

    m_sDefSysSettings.sUserSystemSetting.sNetworkSetting.ucNETWORK_SETTING_CRC = USER_NETWORK_SETTING_RESERVED_NUM;
}

void palDataMgr_sImageSetting_Init_Default(void)
{
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHSGAdjustmentEnable = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHSGAdjustmentAutoTestPattern = (UINT8)ets_ON;
	m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucHSGSelected = 0; // Red

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucBlankonSignalSwitch = (UINT8)ets_ON;          //A70LV_Doulas_0294
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucSyncThreshold = SYNC_THRESHOLD_DEFAULT_VALUE;     //A70LV_Doulas_0060 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucContrastEnhancement = eCONTRAST_ENHANCE_OFF;  //0: off ,1:Dynamic Black,2:real black
#ifdef SCALER_FPGA_F34
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_MANDATORY_3D] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_4K3D] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_FS_120] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_FS_OTHER] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_OTHER] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_MANDATORY_3D_P] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_4K3D_P] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_FS_120_P] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_FS_OTHER_P] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_OTHER_P] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3D_Customer_Timing] = 0;

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_MANDATORY_3D] = SYNC_DELAY_3D_DEFAUL_ACTIVE; //A70LK_Jacky_0036
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_4K3D] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_FS_120] = SYNC_DELAY_3D_DEFAUL_ACTIVE; //A70LK_Jacky_0036
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_FS_OTHER] = SYNC_DELAY_3D_DEFAUL_ACTIVE; //A70LK_Jacky_0036
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_OTHER] = SYNC_DELAY_3D_DEFAUL_ACTIVE; //A70LK_Jacky_0038

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_MANDATORY_3D_P] = SYNC_DELAY_3D_DEFAUL_ACTIVE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_4K3D_P]  = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_FS_120_P]  = SYNC_DELAY_3D_DEFAUL_ACTIVE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_FS_OTHER_P] = SYNC_DELAY_3D_DEFAUL_ACTIVE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_OTHER_P] = SYNC_DELAY_3D_DEFAUL_PASSIVE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3D_Customer_Timing] = 0;
#else
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_1080P_120]  = 1;    //A70LV_Doulas_0196 modify//A70LV_Doulas_0049
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_Other_120] = 1;     //A70LV_Doulas_0196 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_1080P_60] = 1;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_1080P_50] = 1;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFS_WUXGA_60] = 1;
	#ifdef SUPPORT_WUXGA_120HZ_3D_PANEL		//G100_Doulas_0064 Modify
	m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DTB] = 1;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFP]  = 1;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DSbS]  = 1;
	#else
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DTB] = 137;//1;         //A70LV_Doulas_0196 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DFP]  = 137;//1;        //A70LV_Doulas_0196 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3DSbS]  = 137;//1;       //A70LV_Doulas_0196 modify
    #endif
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3D_Reserve1] = 1;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3D_Reserve2] = 1;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DFRAME_DELAY_TIMING[e3D_Customer_Timing] = 1;

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_1080P_120]  = 0;    //A70LV_Doulas_0196 modify//A70LV_Doulas_0049
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_Other_120] = 0;     //A70LV_Doulas_0196 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_1080P_60] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_1080P_50] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFS_WUXGA_60] = 0;
#ifdef SUPPORT_WUXGA_120HZ_3D_PANEL     //G100_Doulas_0064 Modify
	m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DTB] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFP]  = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DSbS]  = 0;
#else
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DTB] = 0;//1;         //A70LV_Doulas_0196 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DFP]  = 0;//1;        //A70LV_Doulas_0196 modify
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3DSbS]  = 0;//1;       //A70LV_Doulas_0196 modify
#endif
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3D_Reserve1] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3D_Reserve2] = 0;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ui3DSYNC_DELAY_TIMING[e3D_Customer_Timing] = 0;
#endif /* SCALER_FPGA_F34 */

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucFilm = (UINT8)ets_OFF;
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) //A65_Owen_0002
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucWallColor = (UINT8)eCM_WALL_COLOR_OFF;   //A70LV_Doulas_0049
#else
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucWallColor = (UINT8)eCM_WALL_COLOR_WHITE;   //A70LV_Doulas_0049
#endif
//    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucEdgeEnhancement;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucImageFreeze = (UINT8)ets_OFF;      //A70LV_Doulas_0049

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucCeilingMount = (UINT8)eCM_CEILING_MOUNT_AUTO;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucRearProject = (UINT8)eCM_REAR_PROJECTION_FRONT;

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucLightsOutTimer = (UINT8)LIGHTS_OUT_TIMER_X05_DEFAULT_VALUE; //A70LV_Larry_0235
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucLightsOutSignalLevel = (UINT8)LIGHTS_OUT_SIGNAL_LEVEL_DEFAULT_VALUE; //A70LV_Larry_0235
	m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucLightsOnThreshold = (UINT8)LIGHTS_ON_THRESHOLD_DEFAULT_VALUE;	//A70Gen2_Doulas_0044
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucCompatible_4K  = (UINT8)eCM_EDID_TYPE_V14;


    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDREnable = (UINT8)ets_ON; //T100_Coda_0013
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)         //A35G2_Tim_0001, mod, for Factory Reset //A35G2_BRC_Casper_0038
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDRLevel = (UINT8)eCM_HDR_LEVEL2;
#else
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDRLevel = (UINT8)eCM_HDR_LEVEL1;//eHDR_SDR;		//HICC2_Doulas_0072 Modify//G100_Doulas_0046 Modify
#endif

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucCW_Speed = (UINT8)eCM_COLOR_WHEEL_SPEED_3X;    //A65_OPTOMA_Doulas_0107

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucDBSpeed = (UINT8)DB_SPEED_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucDBStrength = (UINT8)DB_STRENGTH_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucDBLightLevel = (UINT8)DB_LIGHT_LEVEL_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucDBRealBlackEnable = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDMI_EDID_1    = (UINT8)palDataMgr_Data_DefaultValue_Get(edcHDMI_EDID_1);
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDMI_EDID_2    = (UINT8)palDataMgr_Data_DefaultValue_Get(edcHDMI_EDID_2);
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDBaseT_EDID   = (UINT8)palDataMgr_Data_DefaultValue_Get(edcHDBASET_EDID);
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHDMI_OUT   	= (UINT8)eCM_HDMI_OUTPUT_HDMI1;//G100_Doulas_0057
    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHSGAdjustmentAutoTestPattern_2 = (UINT8)ets_ON; //G100_Coda_00109

	m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucHSGTestPatternCtrl = (UINT8)eCM_TEST_PATTERN_OFF; //HICC2_Doulas_0051//A65_OPTOMA_Julie_0040//A35G2_Coda_0103

    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucIMAGE_SETTING_CRC = USER_IMAGE_SETTING_RESERVED_NUM;
}

void palDataMgr_sWarpSetting_Init_Default(void)
{
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpMode = 0;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingAreaOffset;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiNonBlendingAreaOffset;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlending;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucMarker;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucGrid;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucSolidColor;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucImageGamma;
//    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingGamma;

    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucGeometryEnable = 1;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpHorzKeystone = KEYSTONE_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpVertKeystone = KEYSTONE_DEFAULT_VALUE;

	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpHorzPincushion = HORZ_PINCUSHIN_DEFAULT_VALUE; //A70LV_Larry_0051
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpVertPincushion = HORZ_PINCUSHIN_DEFAULT_VALUE; //A70LV_Larry_0051
//	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpPincushionBarrel;
//	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucGeometryPCMode;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerTopLeftHorz     = _4CORNER_TL_HORZ_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerTopLeftVert     = _4CORNER_TL_VERT_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerTopRightHorz    = _4CORNER_TR_HORZ_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerTopRightVert    = _4CORNER_TR_VERT_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerBottomLeftHorz  = _4CORNER_BL_HORZ_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerBottomLeftVert  = _4CORNER_BL_VERT_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerBottomRightHorz = _4CORNER_BR_HORZ_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ui4CornerBottomRightVert = _4CORNER_BR_VERT_DEFAULT_VALUE;

//A70LV_Larry_0051 start
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendingTopEnable           = ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingTopStartPixel       = BLENDING_TOP_START_PIXEL_DEF_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingTopPixelWidth       = (UINT8)palDataMgr_Data_DefaultValue_Get(edcBLENDING_TOP_PIXEL_WIDTH);   //HICC2_Steven_0055;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendingBottomEnable        = ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingBottomStartPixel    = BLENDING_BOTTOM_START_PIXEL_DEF_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingBottomPixelWidth    = (UINT8)palDataMgr_Data_DefaultValue_Get(edcBLENDING_BOTTOM_PIXEL_WIDTH); //HICC2_Steven_0055;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendingLeftEnable          = ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingLeftStartPixel      = BLENDING_LEFT_START_PIXEL_DEF_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingLeftPixelWidth      = (UINT8)palDataMgr_Data_DefaultValue_Get(edcBLENDING_LEFT_PIXEL_WIDTH); //HICC2_Steven_0055;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendingRightEnable         = ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingRightStartPixel     = BLENDING_RIGHT_START_PIXEL_DEF_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.uiBlendingRightPixelWidth     = (UINT8)palDataMgr_Data_DefaultValue_Get(edcBLENDING_RIGHT_PIXEL_WIDTH); //HICC2_Steven_0055;
//A70LV_Larry_0051 end

    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpAutoFilter = 1; //A70LV_Larry_0053
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpFilterH    = DEFAULT_WAPR_FILTER_LUT_H; //A35G2_Simon_0115
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpFilterV    = DEFAULT_WAPR_FILTER_LUT_V; //A35G2_Simon_0115
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucMovePitchMode = 0;

    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpingSaveSetting  = (UINT8)palDataMgr_Data_DefaultValue_Get(edcWARPING_AP_SAVE);
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpingApplySetting = (UINT8)palDataMgr_Data_DefaultValue_Get(edcWARPING_AP_APPLY);
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendSaveSetting  = (UINT8)palDataMgr_Data_DefaultValue_Get(edcBLEND_MEMORY_SAVE);
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendApplySetting = (UINT8)palDataMgr_Data_DefaultValue_Get(edcBLEND_MEMORY_APPLY);

    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendingGamma = eCM_BLENDING_GAMMA_2_2; //A70LV_Larry_0186

	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpControl				    = WARP_CTRL__BASIC;						//G100_Doulas_0027
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpGridPoints			    = WARP_POINT__2x2;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpInner					= ets_ON;//ets_OFF;						//G100_Doulas_0067
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpSharpness				= WARP_SHARPNESS_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpGridColor				= GRID_COLOR__GREEN;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpBackgroundColor		    = BKG_COLOR__BLACK;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendinWidth		  	      	= 1;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendingLeft				    = ADV_BLENDING_LEFT_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendingRight				= ADV_BLENDING_RIGHT_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendingTop				   	= ADV_BLENDING_TOP_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendingBottom			    = ADV_BLENDING_BOTTOM_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendingOverlapGridNumber 	= BLEND_OVERLAP_GRID_NUMBER_DEFAULT_VALUE;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlendingGamma	            = eCM_BLENDING_GAMMA_2_2;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvWarpInner_LastValue          = ets_OFF;  //G100_Tim_0055, add //A35G2_BRC_Casper_0051

	//init Black level
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelArea							 = BLACKLEVEL_AREA__BOTTOM;		//A65_OPTOMA_Doulas_0020
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelEnable[BLACKLEVEL_AREA__TOP]    = ets_OFF;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelEnable[BLACKLEVEL_AREA__BOTTOM] = ets_OFF;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelBrightness					     = BLACKLEVEL_VALUE_R;

	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelRed[BLACKLEVEL_AREA__TOP]    	 = BLACKLEVEL_VALUE_R;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelRed[BLACKLEVEL_AREA__BOTTOM] 	 = BLACKLEVEL_VALUE_R;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelGreen[BLACKLEVEL_AREA__TOP]     = BLACKLEVEL_VALUE_G;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelGreen[BLACKLEVEL_AREA__BOTTOM]  = BLACKLEVEL_VALUE_G;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelBlue[BLACKLEVEL_AREA__TOP]      = BLACKLEVEL_VALUE_B;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelBlue[BLACKLEVEL_AREA__BOTTOM]   = BLACKLEVEL_VALUE_B;
	m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucAdvBlackLevelBoundary	 					 = ets_ON;				//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239

    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWARP_SETTING_CRC = USER_WARP_SETTING_RESERVED_NUM;

    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWarpingLastSaved = 0xFF;
    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucBlendLastSaved = 0xFF;

    memset(m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory, 0, sizeof(m_sDefSysSettings.sUserSystemSetting.sWarpSetting.sWarpMemory)); //A70LV_Larry_0139
}

void palDataMgr_sCommonSetting_Init_Default(void)
{
//    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucModelName[MODEL_NAME_MAX_LENGTH+1]; //A70LV_Doulas_0022

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLanguage = (UINT8)eCM_LANGUAGE_ENGLISH;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucACPowerOn = (UINT8)ets_OFF;
	#if defined(CUSTOM_OPTOMA) //A35G2_Coda_0097:
#ifdef ENERGY_SAVING
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucStandbyPowerSave = eCM_STANDBY_MODE_0_5W;
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucEnergySaving = ets_ON;
#else
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucStandbyPowerSave = eCM_STANDBY_MODE_COMMUNICATION;
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucEnergySaving = ets_OFF;
#endif
	#else
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucStandbyPowerSave = eCM_STANDBY_MODE_COMMUNICATION;
	#endif

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucProjectorAddress = PROJECTOR_ADDRESS_MIN_VALUE;   //A70LV_Doulas_0023
	#ifdef CUSTOM_CHRISTIE //A35G2_BRC_Casper_0007
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucKeypadBacklight = eCM_KEYPAD_BACKLIGHT_5_SECS;
    #else
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucKeypadBacklight = eCM_KEYPAD_BACKLIGHT_ALWAYS_ON;// BruceLin#20201222	//HICC2_Zonic_0010
	#endif
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucStatusLED = eSTATUS_LED_ALWAYS_ON;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucPwrKeyBacklight = (UINT8)ets_ON;

	#ifdef CUSTOM_OPTOMA
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucAutoOffTime = AUTO_SHUTDOWN_TIMER_DEFAULT_VALUE; //A35G2_Coda_0100
	#else
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucAutoOffTime = eAUTO_OFF_TIME_NEVER;
	#endif

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucScreenSaveTime = 0;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSleepTimer = eSLEEP_TIMER_OFF;

//    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucChangePIN;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSerialPortBaudRate = eCM_SERIAL_PORT_BAUD_RATE_115200;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSerialPortEcho = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSerialPortPath = eSERIAL_PORT_RS232;        //HDBaseT or normal RS232

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucIRControl_Top     = (UINT8)ets_ON;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucIRControl_Front   = (UINT8)ets_ON;
	#ifdef CUSTOM_OPTOMA
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucIRControl_Rear	= (UINT8)ets_ON;
	#endif
    #ifdef CUSTOM_CHRISTIE
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucIRControl_HDBaseT = (UINT8)ets_ON;
    #else
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucIRControl_HDBaseT = (UINT8)ets_OFF;
    #endif
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucHDBaseTEnable = (UINT8)ets_ON;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSysErrCode = 0;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uc12VTrigge = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLowLatency = (UINT8)eCM_LOW_LATENCY_MODE_OFF;

//    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLensAdjust;
//    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLensShift;
//    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLensMemoryApply;
//    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLensMemorySave;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucHighAltitude = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucKeypadLock = (UINT8)ets_OFF;// HICC2_Bruce_0001
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucUSB_Power = (UINT8)ets_OFF;// HICC2_Bruce_0004
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiPowerOnCounter = 0;// HICC2_Bruce_0007
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiPowerOffCounter = 0;// HICC2_Bruce_0007
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucFactoryTestPattern = (UINT8)eCM_FACTORY_TP_NONE;    //A70LV_Doulas_0024 //HICC2_Casper_0022
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucServiceMenuTestPattern = (UINT8)eCM_TEST_PATTERN_OFF;   //A70LV_Doulas_0035
    //m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucServiceMenuCustomPattern = 0;
#ifdef DEMO_SNDS   //HICC2_Doulas_0021
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSplashStartupEnable = (UINT8)eCM_LOGO_PATTERN_BLUE;
#elif defined(CURSOR_FIXTURE) //HICC2_Doulas_0056
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSplashStartupEnable = (UINT8)eCM_LOGO_PATTERN_BLUE;
#else
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSplashStartupEnable = (UINT8)eCM_LOGO_PATTERN_FACTORY_LOGO;//eCM_BACKGROUND_COLOR_LOGO;  //G100_Doulas_0038 Modify//A70LV_Doulas_0032 //A70LV_Doulas_0025 //A70LV_Doulas_0024
#endif
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLensDurationTime_Enable = (UINT8)ets_OFF;
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiLensFocusDurationTime = 35;
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiLensZoomDurationTime = 30;
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiLensBackFocusDurationTime = 35;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucActuatorSwitch = (UINT8)ets_ON;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucCoolingDown = 0;   //A70LV_Doulas_0035
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucOSDTestPattern = (UINT8)eCM_TEST_PATTERN_OFF;   //A70LV_Doulas_0035

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucUartSwitch = eUART_SW_LAN;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucBackupRestoreSave = BACKUP_RESTORE_DEFAULT_VALUE;         //A70LV_Doulas_0279
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucBackupRestoreRestore = BACKUP_RESTORE_DEFAULT_VALUE;      //A70LV_Doulas_0279
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLensDetection = 1;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucBackupPrimaryInput = eCM_SOURCE_HDMI1;//eBK_INPUT_SOURCE_HDMI1;     //HICC2_Doulas_0080
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucBackupSecondaryInput = eCM_SOURCE_HDMI2;//eBK_INPUT_SOURCE_HDMI2;   //HICC2_Doulas_0080

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucBackgroundColor = eCM_BACKGROUND_COLOR_LOGO;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSignalPowerOn = ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ulSecurity_TotalRemainMin = 0;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucStartupShutter = ets_OFF;
    #ifdef CUSTOM_OPTOMA
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucAutoSourceResync = ets_ON;	  //A35G2_Coda_0068
	#else
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucAutoSourceResync = ets_OFF;//G100_Doulas_0006
	#endif
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSerialPortOut_BaudRate = eCM_SERIAL_PORT_BAUD_RATE_115200;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucFadeIn_Time = FADE_TIMER_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucFadeOut_Time = FADE_TIMER_DEFAULT_VALUE;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucLogoChange = eCM_CHANGE_LOGO_DEFAULT;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucFast_Power_On = ets_OFF;

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSystemUpdateStatus = eFOTA_SYSTEM_UPDATE_STATUS_IDLE;			//A65_OPTOMA_Doulas_0025//A35G2_Coda_0049
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucNotification_SystemUpdate = ets_OFF;	//A65_OPTOMA_Doulas_0025	//A35G2_Coda_0049
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucOMS_Service_Mode = eCM_OMS_SERVICE_MODE_PRODUCTION;

#ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0060
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucAuto_HDMI_Switch = ets_OFF;   //G100_Tim_0060, add, start
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucInput_Key_Last_Value = eSOURCE_KEY_CHANGE_SOURCE_AUTO;
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uc3D_Mode_Last_Value = ets_ON;  //G100_Tim_0060, add, end
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucProj_Native_Timing = eCM_NATIVE_TIMING_1920X1200;   //G100_Tim_0061, add
#endif

    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiCOMMON_SETTING_CRC = USER_COMMON_SETTING_RESERVED_NUM;    //A70LV_Doulas_0022
#ifdef CUSTOM_OPTOMA	//A35G2_Coda_0049
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucOSD_Lock = (UINT8)eOSD_NOT_LOCKED; //A65_OPTOMA_Jerry_0005
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucFOTA_Enable = (UINT8)ets_ON; //A65_OPTOMA_Jerry_0005
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucQuickKey = QUICK_KEY_DEFAULT;			//A65_OPTOMA_Doulas_0025
	m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucSilentFOTA_SystemUpdate = ets_ON; //A65_OPTOMA_Julie_0086 //A35G2_Coda_0139
    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.ucProj_Native_Timing = eCM_NATIVE_TIMING_3840X2400;// HICC2_Bruce_0022
#endif
}

void palDataMgr_sSourceSetting_Init_Default(void)
{
    #if 1//def CUSTOM_CHRISTIE
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain = eCM_SOURCE_HDMI1;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucInputSourceSub = eCM_SOURCE_HDMI2;
    #else //CUSTOM_BARCO
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain = eCM_SOURCE_VGA;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucInputSourceSub = eCM_SOURCE_HDMI1;
    #endif

    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPIPEnable = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainLayout = eCM_MAIN_LAYOUT_PBP_MAIN_LEFT;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPIPLayout = eCM_MAIN_LAYOUT_PIP_BOTTOM_RIGHT;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPBPLayout = eCM_MAIN_LAYOUT_PBP_MAIN_LEFT;
    #ifdef CUSTOM_OPTOMA  //A35G2_Coda_0085
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPIPSize   = eCM_PIP_SIZE_SMALL;
    #else
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPIPSize   = eCM_PIP_SIZE_LARGE;
    #endif

#ifdef DEMO_SNDS   //HICC2_Doulas_0021
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucSourceKeyOption = eSOURCE_KEY_CHANGE_SOURCE;
#else
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucSourceKeyOption = eSOURCE_KEY_CHANGE_SOURCE_AUTO; //G100_Steven_0070 //A70LV_Doulas_0024
#endif
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucAutoSource = ;        //?
    //m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPowerUpSource = eINPUT_SOURCE_VGA;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucSourceLock = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.uiSourceInfo = 0;        //?
   //m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucPowerUpSource = eINPUT_SOURCE_HDMI1;

//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucEligibleTerminal[INPUT_SOURCE_MAX];
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[INPUT_SOURCE_MAX][SOURCE_NAME_MAX_LENGTH];

   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[0], "Undefined0");    //T100_Simon_0006 Start
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[1], "VGA");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[2], "Undefined2");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[3], "HDMI 1");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[4], "HDMI 2");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[5], "DVI-D");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[6], "DisplayPort");  //G100_Wilsonj_0005
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[7], "3G-SDI");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[8], "HDBaseT");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[9], "12G-SDI");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[10], "Undefined10");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[11], "Undefined11");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[12], "Undefined12");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[13], "Undefined13");
   sprintf((char *)m_sDefSysSettings.sUserSystemSetting.sSourceSetting.cSourceName[14], "Undefined14");       //T100_Simon_0006 End

    #if defined(CUSTOM_CHRISTIE)
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucCustomKey = eCM_HOTKEY_SETTINGS_SIZE_PRESETS;
    #elif defined(CUSTOM_OPTOMA)
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucCustomKey = eCM_HOTKEY_SETTINGS_HDMI1; //HICC2_AC_0009
    #else
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucCustomKey = eCM_HOTKEY_SETTINGS_FREEZE_SCREEN;  //G100_Wilsonj_0051
    #endif

    #if defined(CUSTOM_OPTOMA)
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucCustomKey2 = eCM_HOTKEY_SETTINGS_HDMI2; //HICC2_AC_0009
    #else
    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucCustomKey2 = eCM_HOTKEY_SETTINGS_PROJECTOR_ID;  //G100_Wilsonj_0053
    #endif

    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucBlankKey  = (UINT8)ets_OFF;

    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucSourceHotKeyEnable = (UINT8)ets_ON;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey0 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey1 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey2 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey3 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey4 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey5 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey6 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey7 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey8 = ;
//    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucMainSourceHotKey9 = ;
}

void palDataMgr_sManualAdjustment_Init_Default(void)
{
#if SUPPORT_MANUAL_ADJUSTMENT   //A70LV_Doulas_0023
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.ucManualAdjustmentEnable = (UINT8)ets_OFF;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.ucManualAdjustmentAutoTestPattern = (UINT8)ets_ON;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiRedPartOfRed       = INI_CCN_RED_OF_RED;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiGreenPartOfRed     = INI_CCN_GREEN_OF_RED;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiBluePartOfRed      = INI_CCN_BLUE_OF_RED;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiRedPartOfGreen     = INI_CCN_RED_OF_GREEN;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiGreenPartOfGreen   = INI_CCN_GREEN_OF_GREEN;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiBluePartOfGreen    = INI_CCN_BLUE_OF_GREEN;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiRedPartOfBlue      = INI_CCN_RED_OF_BLUE;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiGreenPartOfBlue    = INI_CCN_GREEN_OF_BLUE;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiBluePartOfBlue     = INI_CCN_BLUE_OF_BLUE;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiRedPartOfWhite     = INI_CCN_RED_OF_WHITE;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiGreenPartOfWhite   = INI_CCN_GREEN_OF_WHITE;
    m_sDefSysSettings.sUserSystemSetting.sManualAdjustment.uiBluePartOfWhite    = INI_CCN_BLUE_OF_WHITE;
#endif
}

void palDataMgr_sSourceDependSetting_Init_Default(void)
{
    UINT8  ucInputSource = 0;
    sSOURCE_DEPEND_SETTING sSource_depend_Setting =
    {
        .ucAspectRatio              = eCM_SCALING_MODE_AUTO,
        .ucOverScan                 = ets_OFF ,
        .ucPhase                    = PHASE_DEFAULT_VALUE ,
        .ucTracking                 = TRACKING_DEFAULT_VALUE ,
        .ucHorzPosition             = HORZ_POSITION_DEFAULT,
        .ucVertPosition             = VERT_POSITION_DEFAULT,
        .uiDigitalHorzZoom          = DIGITAL_HORZ_ZOOM_DEFAULT ,
        .uiDigitalVertZoom          = DIGITAL_VERT_ZOOM_DEFAULT,
        .uiDigitalHorzShift         = DIGITAL_HORZ_SHIFT_DEFAULT,
        .uiDigitalVertShift         = DIGITAL_VERT_SHIFT_DEFAULT,
        .uiCustomDigitalHorzZoom    = DIGITAL_HORZ_ZOOM_DEFAULT ,
        .uiCustomDigitalVertZoom    = DIGITAL_VERT_ZOOM_DEFAULT,
        .uiCustomDigitalHorzShift   = DIGITAL_HORZ_SHIFT_DEFAULT,
        .uiCustomDigitalVertShift   = DIGITAL_VERT_SHIFT_DEFAULT,
    #if defined(CUSTOM_OPTOMA) //H30K_Doulas_0063
        .ucPresetMode               = eCM_PICTURE_SETTINGS_PRESENTATION,
    #else
        .ucPresetMode               = eCM_PICTURE_SETTINGS_BRIGHT,
    #endif
    #ifdef CURSOR_FIXTURE
        .ucPresetMode               = eCM_PICTURE_SETTINGS_BRIGHT,
    #endif
    #ifdef OE_JIG
        .ucPresetMode               = eCM_PICTURE_SETTINGS_BRIGHT,
    #endif
        .ucBackupPreset             = eCM_PICTURE_SETTINGS_BRIGHT,
        .ucSavePreset               = 0,
    #ifdef ENABLE_SINGLE_USER_MODE      //H30K_Tim_0005, add, ***
        .ucPreUserMode              = eCM_PICTURE_SETTINGS_USER,
    #else //ENABLE_SINGLE_USER_MODE     //H30K_Tim_0005, add, &&&
        .ucPreUserMode              = eCM_PICTURE_SETTINGS_BRIGHT,//eCM_PICTURE_SETTINGS_NUMBER, //User mode off //G100_Doulas_0066 Modify//G100_Steven_0029
    #endif //ENABLE_SINGLE_USER_MODE    //H30K_Tim_0005, add
        .uc3D_Enable                = eCM_3D_FORMAT_AUTO,
        .uc3D_Invert                = ets_OFF,
        .uc3D_SyncOut               = eCM_3D_SYNC_OUT_TO_EMITTER,
        .uc3D_SyncIn                = eCM_3D_SYNC_TYPE_AUTO,
        .uc3D_Reference             = eCM_3D_LR_REFERENCE_FIELD_GPIO,
        .uc3D_2D_View               = eCM_3D_2D_3D,
        .ucEdgeMask                 = ets_OFF,
        .uc3D_Mode                  = ets_ON,
      #if defined(CUSTOM_OPTOMA) //A35G2_Coda_0137
        .uc3D_Tech                  = eCM_3D_TECH_DLP_LINK,
      #else
        .uc3D_Tech                  = eCM_3D_TECH_3D_SYNC,
      #endif

        .ucAutoImage                = eAUTO_IMAGE_WIDE,
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
        .ucDigitalZoomProp          = ets_ON,
#else
        .ucDigitalZoomProp          = ets_OFF,
#endif
    };

    for(ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        memcpy(&m_sDefSysSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource],
        &sSource_depend_Setting, sizeof(sSOURCE_DEPEND_SETTING));
    }

    m_sDefSysSettings.sUserSystemSetting.ucSOURCE_DEPEND_SETTING_CRC = SOUCE_DEPEND_RESERVED_NUM;

  #ifdef ENABLE_GO_BACK_TO_LAST_MODE      //H30K_Tim_0006, add, ***
    palDataMgr_Last_Display_Mode_Init();
  #endif //ENABLE_GO_BACK_TO_LAST_MODE    //H30K_Tim_0006, add, &&&
}

void palDataMgr_sHSG_setting_Init_Default(void)
{
#ifndef OE_TOOL
    UINT8 ucInputSource,ucDisplayMode;
    sHSG_SETTING sHSG_TABLE_SETTING[DISPLAY_MODE_MAX];

    if(palSystem_ModelIDGet() == MODEL_ID_0) //A35G2_BRC_Casper_0023
    {
        memcpy(&sHSG_TABLE_SETTING,&sHSG_TABLE_SETTING_MODEL0[0], sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);//A35G2_Coda_0106
    }
    else if(palSystem_ModelIDGet() == MODEL_ID_1) //A35G2_Coda_0106
    {
        memcpy(&sHSG_TABLE_SETTING,&sHSG_TABLE_SETTING_MODEL1[0], sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);
    }
    else
    {
        memcpy(&sHSG_TABLE_SETTING,&sHSG_TABLE_SETTING_MODEL2[0], sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);//A35G2_Coda_0106
    }

    ucInputSource = 0;
    ucDisplayMode = 0;
    for(; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)  //A70LV_Doulas_0047 modify
        {
			m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_R_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_R_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_R_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_G_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_G_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_G_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_B_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_B_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_B_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_C_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_C_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_C_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_M_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_M_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_M_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_R_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_G_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_B_GAIN;


            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_R_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_R_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_R_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_G_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_G_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_G_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_B_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_B_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_B_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_C_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_C_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_C_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_M_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_M_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_M_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_R_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_G_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_B_GAIN;

        }
    }
#else

    UINT8 ucInputSource = 0,ucDisplayMode = 0, cCount = 0;
    sHSG_SETTING sHSG_TABLE_SETTING[DISPLAY_MODE_MAX];
	char cFileName[128] = {'\0'};

    FILE *pFile = NULL;
    uint32_t size;
    UINT8 *pcData = NULL;
    UINT8 cHeader[OE_HEADER_OFFSET] = {0};
    sOE_HSG_SETTING *psHSG_Table = NULL;

    if(access(CONF_SCALER_COLOR_PATH, 0) == -1)
    {
        mkdir(CONF_SCALER_COLOR_PATH, 0777);
    }

    if(palSystem_ModelIDGet() == MODEL_ID_0)
    {
        memcpy(&sHSG_TABLE_SETTING,&sHSG_TABLE_SETTING_MODEL0[0], sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);//A35G2_Coda_0106
        snprintf(cFileName, 128, "%s/hsgSettings_0.bin", CONF_SCALER_COLOR_PATH);
    }
    else if(palSystem_ModelIDGet() == MODEL_ID_1)
    {
        memcpy(&sHSG_TABLE_SETTING,&sHSG_TABLE_SETTING_MODEL1[0], sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);
        snprintf(cFileName, 128, "%s/hsgSettings_1.bin", CONF_SCALER_COLOR_PATH);
    }
    else
    {
        memcpy(&sHSG_TABLE_SETTING,&sHSG_TABLE_SETTING_MODEL2[0], sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);
        snprintf(cFileName, 128, "%s/hsgSettings_2.bin", CONF_SCALER_COLOR_PATH);
    }

    if(CFG_CUSTOMER_ID == CUSTOMER_ID_FUJIFILM)
    {
        UINT8 ucValue;

        palDataMgr_Data_Access(edcHSG_FACTORY_SWITCH, edaREAD, &ucValue);

        if(ucValue)
        {
            if(access(CONF_SCALER_COLOR_PATH_FJ, 0) == -1)
            {
                mkdir(CONF_SCALER_COLOR_PATH_FJ, 0777);
            }

            if(palSystem_ModelIDGet() == MODEL_ID_0)
            {
                snprintf(cFileName, 128, "%s/hsgSettings_0.bin", CONF_SCALER_COLOR_PATH_FJ);
            }
            else if(palSystem_ModelIDGet() == MODEL_ID_1)
            {
                snprintf(cFileName, 128, "%s/hsgSettings_1.bin", CONF_SCALER_COLOR_PATH_FJ);
            }
            else
            {
                snprintf(cFileName, 128, "%s/hsgSettings_2.bin", CONF_SCALER_COLOR_PATH_FJ);
            }
        }
    }

    pFile = fopen(cFileName, "rb+");

    if(pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        pcData = malloc(size);

        if(fread(pcData, sizeof(uint8_t), size, pFile) != size)
        {
            printf("(func:%s, line:%d) fread fail\n", __FUNCTION__, __LINE__);
        }

        fclose(pFile);

        memcpy(&cHeader, pcData, OE_HEADER_OFFSET);

        psHSG_Table = (sOE_HSG_SETTING*)(pcData + OE_HEADER_OFFSET);

        ucDisplayMode = (size - OE_HEADER_OFFSET)/sizeof(sOE_HSG_SETTING);

        for(cCount = 0; cCount < ucDisplayMode; cCount++)
        {
#if 0
            printf("(%d)(%d, %d, %d)(%d, %d, %d)(%d, %d, %d)(%d, %d, %d)(%d, %d, %d)(%d, %d, %d)(%d, %d, %d)\n",
                    psHSG_Table->mode,
                    psHSG_Table->colors.sRed.Hue,
                    psHSG_Table->colors.sRed.Saturation,
                    psHSG_Table->colors.sRed.Gain,
                    psHSG_Table->colors.sGreen.Hue,
                    psHSG_Table->colors.sGreen.Saturation,
                    psHSG_Table->colors.sGreen.Gain,
                    psHSG_Table->colors.sBlue.Hue,
                    psHSG_Table->colors.sBlue.Saturation,
                    psHSG_Table->colors.sBlue.Gain,
                    psHSG_Table->colors.sCyan.Hue,
                    psHSG_Table->colors.sCyan.Saturation,
                    psHSG_Table->colors.sCyan.Gain,
                    psHSG_Table->colors.sMagenta.Hue,
                    psHSG_Table->colors.sMagenta.Saturation,
                    psHSG_Table->colors.sMagenta.Gain,
                    psHSG_Table->colors.sYellow.Hue,
                    psHSG_Table->colors.sYellow.Saturation,
                    psHSG_Table->colors.sYellow.Gain,
                    psHSG_Table->colors.sWhite.RedGain,
                    psHSG_Table->colors.sWhite.GreenGain,
                    psHSG_Table->colors.sWhite.BlueGain);
#endif /* 0 */

            if(psHSG_Table->mode < DISPLAY_MODE_MAX)
            {
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_R_HUE = psHSG_Table->colors.sRed.Hue;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_R_SAT = psHSG_Table->colors.sRed.Saturation;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_R_GAIN = psHSG_Table->colors.sRed.Gain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_G_HUE = psHSG_Table->colors.sGreen.Hue;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_G_SAT = psHSG_Table->colors.sGreen.Saturation;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_G_GAIN = psHSG_Table->colors.sGreen.Gain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_B_HUE = psHSG_Table->colors.sBlue.Hue;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_B_SAT = psHSG_Table->colors.sBlue.Saturation;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_B_GAIN = psHSG_Table->colors.sBlue.Gain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_C_HUE = psHSG_Table->colors.sCyan.Hue;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_C_SAT = psHSG_Table->colors.sCyan.Saturation;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_C_GAIN = psHSG_Table->colors.sCyan.Gain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_M_HUE = psHSG_Table->colors.sMagenta.Hue;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_M_SAT = psHSG_Table->colors.sMagenta.Saturation;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_M_GAIN = psHSG_Table->colors.sMagenta.Gain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_Y_HUE = psHSG_Table->colors.sYellow.Hue;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_Y_SAT = psHSG_Table->colors.sYellow.Saturation;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_Y_GAIN = psHSG_Table->colors.sYellow.Gain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_W_R_GAIN = psHSG_Table->colors.sWhite.RedGain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_W_G_GAIN = psHSG_Table->colors.sWhite.GreenGain;
                sHSG_TABLE_SETTING[psHSG_Table->mode].HSG_W_B_GAIN = psHSG_Table->colors.sWhite.BlueGain;
            }

            psHSG_Table++;
        }

        if(palSystem_ModelIDGet() == MODEL_ID_0)
        {
            memcpy(&sHSG_TABLE_SETTING_MODEL0[0], &sHSG_TABLE_SETTING, sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);
        }
        else if(palSystem_ModelIDGet() == MODEL_ID_1)
        {
            memcpy(&sHSG_TABLE_SETTING_MODEL1[0], &sHSG_TABLE_SETTING, sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);
        }
        else
        {
            memcpy(&sHSG_TABLE_SETTING_MODEL2[0], &sHSG_TABLE_SETTING, sizeof(sHSG_SETTING)*DISPLAY_MODE_MAX);
        }

        free(pcData);
    }
#if 0
    else
    {
        if(palSystem_ModelIDGet() == MODEL_ID_0) //A35G2_BRC_Casper_0023
        {
            printf("(%s, %d) hsgSettings_0.bin open file fail!\n", __FUNCTION__, __LINE__);
        }
        else if(palSystem_ModelIDGet() == MODEL_ID_1) //A35G2_Coda_0106
        {
            printf("(%s, %d) hsgSettings_1.bin open file fail!\n", __FUNCTION__, __LINE__);
        }
        else
        {
            printf("(%s, %d) hsgSettings_2.bin open file fail!\n", __FUNCTION__, __LINE__);
        }
    }
#endif /* 0 */

    ucInputSource = 0;
    ucDisplayMode = 0;

    for(; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)  //A70LV_Doulas_0047 modify
        {
			m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_R_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_R_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_R_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_G_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_G_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_G_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_B_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_B_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_B_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_C_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_C_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_C_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_M_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_M_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_M_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_HUE;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_SAT;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_R_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_G_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_B_GAIN;


            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_R_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_R_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_R_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_R_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_G_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_G_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_G_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_G_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_B_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_B_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_B_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_B_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_C_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_C_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_C_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_C_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_M_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_M_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_M_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_M_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_HUE  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_HUE;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_SAT  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_SAT;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_Y_GAIN = sHSG_TABLE_SETTING[ucDisplayMode].HSG_Y_GAIN;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_R_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_G_GAIN;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = sHSG_TABLE_SETTING[ucDisplayMode].HSG_W_B_GAIN;

        }
    }

#endif /* OE_TOOL */

}

void palDataMgr_sPWM_setting_Init_Default(void)
{
#if 1//ndef OE_TOOL		//H30 PWM bin 放在DDP

#else

    UINT8 ucInputSource = 0,ucDisplayMode = 0, cCount = 0;
	char cFileName[128] = {'\0'};

    FILE *pFile = NULL;
    uint32_t size;
    UINT8 *pcData = NULL;
    UINT8 cHeader[OE_HEADER_OFFSET] = {0};
    sOE_PWM_SETTING *psPWM_Table = NULL;

    if(access(CONF_SCALER_COLOR_PATH, 0) == -1)
    {
        mkdir(CONF_SCALER_COLOR_PATH, 0777);
    }

    if(palSystem_ModelIDGet() == MODEL_ID_0)
    {
        snprintf(cFileName, 128, "%s/pwmSettings_0.bin", CONF_SCALER_COLOR_PATH);
    }
    else if(palSystem_ModelIDGet() == MODEL_ID_1)
    {
        snprintf(cFileName, 128, "%s/pwmSettings_1.bin", CONF_SCALER_COLOR_PATH);
    }
    else if(palSystem_ModelIDGet() == MODEL_ID_2)
    {
        snprintf(cFileName, 128, "%s/pwmSettings_2.bin", CONF_SCALER_COLOR_PATH);
    }
    else
    {
        snprintf(cFileName, 128, "%s/pwmSettings_0.bin", CONF_SCALER_COLOR_PATH);
    }

    pFile = fopen(cFileName, "rb+");

    if(pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        pcData = malloc(size);

        fread(pcData, sizeof(uint8_t), size, pFile);

        fclose(pFile);

        memcpy(&cHeader, pcData, OE_HEADER_OFFSET);

        psPWM_Table = (sOE_PWM_SETTING*)(pcData + OE_HEADER_OFFSET);

        ucDisplayMode = (size - OE_HEADER_OFFSET)/sizeof(sOE_PWM_SETTING);

        for(cCount = 0; cCount < ucDisplayMode; cCount++)
        {
#if 0
            printf("(%d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)(%d, %d, %d, %d)\n",
                    psPWM_Table->mode,
                    psPWM_Table->pwm_value[0].s100.Red,
                    psPWM_Table->pwm_value[0].s100.Green,
                    psPWM_Table->pwm_value[0].s100.Blue,
                    psPWM_Table->pwm_value[0].s100.Yellow,
                    psPWM_Table->pwm_value[0].s90.Red,
                    psPWM_Table->pwm_value[0].s90.Green,
                    psPWM_Table->pwm_value[0].s90.Blue,
                    psPWM_Table->pwm_value[0].s90.Yellow,
                    psPWM_Table->pwm_value[0].s80.Red,
                    psPWM_Table->pwm_value[0].s80.Green,
                    psPWM_Table->pwm_value[0].s80.Blue,
                    psPWM_Table->pwm_value[0].s80.Yellow,
                    psPWM_Table->pwm_value[0].s70.Red,
                    psPWM_Table->pwm_value[0].s70.Green,
                    psPWM_Table->pwm_value[0].s70.Blue,
                    psPWM_Table->pwm_value[0].s70.Yellow,
                    psPWM_Table->pwm_value[0].s60.Red,
                    psPWM_Table->pwm_value[0].s60.Green,
                    psPWM_Table->pwm_value[0].s60.Blue,
                    psPWM_Table->pwm_value[0].s60.Yellow,
                    psPWM_Table->pwm_value[0].s50.Red,
                    psPWM_Table->pwm_value[0].s50.Green,
                    psPWM_Table->pwm_value[0].s50.Blue,
                    psPWM_Table->pwm_value[0].s50.Yellow,
                    psPWM_Table->pwm_value[0].s40.Red,
                    psPWM_Table->pwm_value[0].s40.Green,
                    psPWM_Table->pwm_value[0].s40.Blue,
                    psPWM_Table->pwm_value[0].s40.Yellow,
                    psPWM_Table->pwm_value[0].s30.Red,
                    psPWM_Table->pwm_value[0].s30.Green,
                    psPWM_Table->pwm_value[0].s30.Blue,
                    psPWM_Table->pwm_value[0].s30.Yellow,
                    psPWM_Table->pwm_value[0].s20.Red,
                    psPWM_Table->pwm_value[0].s20.Green,
                    psPWM_Table->pwm_value[0].s20.Blue,
                    psPWM_Table->pwm_value[0].s20.Yellow,
                    psPWM_Table->pwm_value[0].s10.Red,
                    psPWM_Table->pwm_value[0].s10.Green,
                    psPWM_Table->pwm_value[0].s10.Blue,
                    psPWM_Table->pwm_value[0].s10.Yellow);
#endif /* 0 */
                    psPWM_Table++;

        }

        free(pcData);
    }
#if 0
    else
    {
        if(palSystem_ModelIDGet() == MODEL_ID_0) //A35G2_BRC_Casper_0023
        {
            printf("(%s, %d) pwmSettings_0.bin open file fail!\n", __FUNCTION__, __LINE__);
        }
        else if(palSystem_ModelIDGet() == MODEL_ID_1) //A35G2_Coda_0106
        {
            printf("(%s, %d) pwmSettings_1.bin open file fail!\n", __FUNCTION__, __LINE__);
        }
        else
        {
            printf("(%s, %d) pwmSettings_2.bin open file fail!\n", __FUNCTION__, __LINE__);
        }
    }
#endif /* 0 */
#endif /* OE_TOOL */

}

void palDataMgr_sColorSetting_Init_Default(void)    //A70LV_Doulas_0192 modify//A70LV_Doulas_0111 modify OE default
{
#ifndef OE_TOOL
    UINT8  ucInputSource = 0;
    UINT8  ucDisplayMode = 0;
    UINT8  ucSignalType = 0;

    for(ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)
        {
            for(ucSignalType = 0; ucSignalType < SIGNAL_TYPE_MAX ; ucSignalType++)
            {
                memcpy(&m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType],
                        &sColorSetting[ucDisplayMode], sizeof(sCOLOR_SETTING));

                memcpy(&m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType],  //G100_Steven_0047
                        &sColorSetting[ucDisplayMode], sizeof(sCOLOR_SETTING));
            }
        }
    }
#else
    UINT8  ucInputSource = 0;
    UINT8  ucDisplayMode = 0;
    UINT8  ucSignalType = 0;
    UINT8  cCount = 0;
	char cFileName[128] = {'\0'};

    FILE *pFile = NULL;
    uint32_t size;

    sOE_COLOR_SETTING *psColorTable = NULL;
    UINT8 *pcData = NULL;
    UINT8 cHeader[OE_HEADER_OFFSET] = {0};

    if(access(CONF_SCALER_COLOR_PATH, 0) == -1)
    {
        mkdir(CONF_SCALER_COLOR_PATH, 0777);
    }

    //snprintf(cFileName, 128, "%s/colorSettings_0.bin", CONF_SCALER_COLOR_PATH);
	if(palSystem_ModelIDGet() == MODEL_ID_0)
	{
		snprintf(cFileName, 128, "%s/colorSettings_0.bin", CONF_SCALER_COLOR_PATH);
	}
	else if(palSystem_ModelIDGet() == MODEL_ID_1)
	{
		snprintf(cFileName, 128, "%s/colorSettings_1.bin", CONF_SCALER_COLOR_PATH);
	}
	else if(palSystem_ModelIDGet() == MODEL_ID_2)
	{
		snprintf(cFileName, 128, "%s/colorSettings_2.bin", CONF_SCALER_COLOR_PATH);
	}
	else
	{
		snprintf(cFileName, 128, "%s/colorSettings_0.bin", CONF_SCALER_COLOR_PATH);
	}

    pFile = fopen(cFileName, "rb+");

    if(pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        pcData = malloc(size);

        if(fread(pcData, sizeof(uint8_t), size, pFile) != size)
        {
            printf("(func:%s, line:%d) fread fail\n", __FUNCTION__, __LINE__);
        }

        fclose(pFile);

        memcpy(&cHeader, pcData, OE_HEADER_OFFSET);

        psColorTable = (sOE_COLOR_SETTING*)(pcData + OE_HEADER_OFFSET);

        ucDisplayMode = (size - OE_HEADER_OFFSET)/sizeof(sOE_COLOR_SETTING);

        for(cCount = 0; cCount < ucDisplayMode; cCount++)
        {
#if 0
            printf("(%d)(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
                psColorTable->mode,
                psColorTable->color_table.ucCS,
                psColorTable->color_table.ucCT,
                psColorTable->color_table.ucGamma,
                psColorTable->color_table.ucBrillientColorEnabled,
                psColorTable->color_table.ucWhitePeaking,
                psColorTable->color_table.ucColorEnhancement,
                psColorTable->color_table.ucSkinColor,
                psColorTable->color_table.ucSharpness,
                psColorTable->color_table.ucBrightness,
                psColorTable->color_table.ucContrast,
                psColorTable->color_table.ucTint,
                psColorTable->color_table.ucSaturation,
                psColorTable->color_table.ucRedGain,
                psColorTable->color_table.ucGreenGain,
                psColorTable->color_table.ucBlueGain,
                psColorTable->color_table.ucRedOffset,
                psColorTable->color_table.ucGreenOffset,
                psColorTable->color_table.ucBlueOffset,
                psColorTable->color_table.ucWallColorSet);
#endif /* 0 */

            if(psColorTable->mode < DISPLAY_MODE_MAX)
            {
                sColorSetting[psColorTable->mode].ucCS = psColorTable->color_table.ucCS;
                sColorSetting[psColorTable->mode].ucCT = psColorTable->color_table.ucCT;
                sColorSetting[psColorTable->mode].ucGamma = psColorTable->color_table.ucGamma; //palDataMgr_sDDP_Gamma_Transfer(psColorTable->color_table.ucGamma);	//OE tool gamma為DDP gamm index, 需轉換為iScaler index
                sColorSetting[psColorTable->mode].ucBrillientColorEnabled = psColorTable->color_table.ucBrillientColorEnabled;
                sColorSetting[psColorTable->mode].ucWhitePeaking = psColorTable->color_table.ucWhitePeaking;
                sColorSetting[psColorTable->mode].ucColorEnhancement = psColorTable->color_table.ucColorEnhancement;
                sColorSetting[psColorTable->mode].ucSkinColor = psColorTable->color_table.ucSkinColor;
                sColorSetting[psColorTable->mode].ucSharpness = psColorTable->color_table.ucSharpness;
                sColorSetting[psColorTable->mode].ucBrightness = psColorTable->color_table.ucBrightness;
                sColorSetting[psColorTable->mode].ucContrast = psColorTable->color_table.ucContrast;
                sColorSetting[psColorTable->mode].ucTint = psColorTable->color_table.ucTint;
                sColorSetting[psColorTable->mode].ucSaturation = psColorTable->color_table.ucSaturation;
                sColorSetting[psColorTable->mode].ucRedGain = psColorTable->color_table.ucRedGain;
                sColorSetting[psColorTable->mode].ucGreenGain = psColorTable->color_table.ucGreenGain;
                sColorSetting[psColorTable->mode].ucBlueGain = psColorTable->color_table.ucBlueGain;
                sColorSetting[psColorTable->mode].ucRedOffset = psColorTable->color_table.ucRedOffset;
                sColorSetting[psColorTable->mode].ucGreenOffset = psColorTable->color_table.ucGreenOffset;
                sColorSetting[psColorTable->mode].ucBlueOffset = psColorTable->color_table.ucBlueOffset;
                sColorSetting[psColorTable->mode].ucWallColorSet = psColorTable->color_table.ucWallColorSet;
            }

            psColorTable++;
        }

        free(pcData);
    }
#if 0
    else
    {
        printf("(%s, %d) colorSettings_0 open file fail!\n", __FUNCTION__, __LINE__);
    }
#endif /* 0 */


    for(ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)
        {
            for(ucSignalType = 0; ucSignalType < SIGNAL_TYPE_MAX ; ucSignalType++)
            {
                memcpy(&m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType],
                        &sColorSetting[ucDisplayMode], sizeof(sCOLOR_SETTING));

                memcpy(&m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType],  //G100_Steven_0047
                        &sColorSetting[ucDisplayMode], sizeof(sCOLOR_SETTING));
            }
        }
    }

#endif /* OE_TOOL */

    m_sDefSysSettings.sUserSystemSetting.ucCOLOR_DEPEND_SETTING_CRC = COLOR_SETTING_RESERVED_NUM;

}

void palDataMgr_sHSG_ColorTemperature_Init_Default(void)
{
    UINT8 ucCount = 0, ucCount2 = 0;

    memset((UINT8*)&m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature, 127, sizeof(m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature));

#ifndef OE_TOOL
#else

    UINT8 ucInputSource = 0,ucDisplayMode = 0, ucColorTempMode;
    sOE_CT_SETTING_DATA sOE_CT_Table;
    char cFileName[128] = {'\0'};

    FILE *pcOECT_File = NULL, *pcCT_File = NULL;
    uint32_t size;
    UINT8 *pcOE_CT_Data = NULL, *pcCT_Data = NULL;
    UINT8 cHeader[OE_HEADER_OFFSET] = {0};
    sOE_CT_TABLE *psOE_CT_Table = NULL;
    sOE_CUSTOM_CT_TABLE *psCT_Table = NULL;

    if(access(CONF_SCALER_COLOR_PATH, 0) == -1)
    {
        mkdir(CONF_SCALER_COLOR_PATH, 0777);
    }

    snprintf(cFileName, 128, "%s/Color_temperture_setting.bin", CONF_SCALER_COLOR_PATH);

    pcOECT_File = fopen(cFileName, "rb+");

    snprintf(cFileName, 128, "%s/Custom_color_temperture_setting.bin", CONF_SCALER_COLOR_PATH);

    pcCT_File = fopen(cFileName, "rb+");

    if(pcOECT_File != NULL && pcCT_File != NULL)
    {
        fseek(pcOECT_File, 0, SEEK_END);
        size = ftell(pcOECT_File);
        fseek(pcOECT_File, 0, SEEK_SET);

        pcOE_CT_Data = malloc(size);

        if(fread(pcOE_CT_Data, sizeof(uint8_t), size, pcOECT_File) != size)
        {
            printf("(func:%s, line:%d) fread fail\n", __FUNCTION__, __LINE__);
        }

        fclose(pcOECT_File);

        memcpy(&cHeader, pcOE_CT_Data, OE_HEADER_OFFSET);

        psOE_CT_Table = (sOE_CT_TABLE*)(pcOE_CT_Data + OE_HEADER_OFFSET);

        ucDisplayMode = cHeader[OE_HEADER_PIC_NUM];
        ucColorTempMode = cHeader[OE_HEADER_CT_NUM];

        for(ucCount = 0; ucCount < ucColorTempMode; ucCount++)
        {
            for(ucCount2 = 0; ucCount2 < ucDisplayMode; ucCount2++)
            {
                if(psOE_CT_Table->colortemp < OE_COLORTEMPERATURE_SIZE && psOE_CT_Table->picturemode[ucCount2].pic_index < OE_PICTUREMODE_SIZE)
                {
                    sOE_CT_Table.setting[psOE_CT_Table->colortemp].picturemode[psOE_CT_Table->picturemode[ucCount2].pic_index].colors.RedGain = psOE_CT_Table->picturemode[ucCount2].colors.RedGain;
                    sOE_CT_Table.setting[psOE_CT_Table->colortemp].picturemode[psOE_CT_Table->picturemode[ucCount2].pic_index].colors.GreenGain = psOE_CT_Table->picturemode[ucCount2].colors.GreenGain;
                    sOE_CT_Table.setting[psOE_CT_Table->colortemp].picturemode[psOE_CT_Table->picturemode[ucCount2].pic_index].colors.BlueGain = psOE_CT_Table->picturemode[ucCount2].colors.BlueGain;
                }
                else
                {
                    ASSERT_ALWAYS();
                }
            }
            psOE_CT_Table = (sOE_CT_TABLE*)(pcOE_CT_Data + OE_HEADER_OFFSET + (ucCount+1)*(2 + 8*ucDisplayMode)); //picture mode + HSG RGB
        }

        fseek(pcCT_File, 0, SEEK_END);
        size = ftell(pcCT_File);
        fseek(pcCT_File, 0, SEEK_SET);

        pcCT_Data = malloc(size);

        if(fread(pcCT_Data, sizeof(uint8_t), size, pcCT_File) != size)
        {
            printf("(func:%s, line:%d) fread fail\n", __FUNCTION__, __LINE__);
        }

        fclose(pcCT_File);

        memcpy(&cHeader, pcCT_Data, OE_HEADER_OFFSET);

        psCT_Table = (sOE_CUSTOM_CT_TABLE*)(pcCT_Data + OE_HEADER_OFFSET);

        ucDisplayMode = cHeader[OE_HEADER_PIC_NUM];
        ucColorTempMode = cHeader[OE_HEADER_CUSTOM_CT_NUM];

        for(ucCount = 0; ucCount < ucDisplayMode; ucCount++)
        {
            for(ucCount2 = 0; ucCount2 < ucColorTempMode; ucCount2++)
            {
                if(psCT_Table->picturemode < DISPLAY_MODE_MAX && ucCount2 < HSG_CT_MAX)
                {
#if 0
                    printf("(%s, %d) picture  = %d, colortemp = %d (%3d %3d %3d)\n", __FUNCTION__, __LINE__,
                        psCT_Table->picturemode,
                        psCT_Table->colortemp[ucCount2],
                        sOE_CT_Table.setting[psCT_Table->colortemp[ucCount2]].picturemode[psCT_Table->picturemode].colors.RedGain,
                        sOE_CT_Table.setting[psCT_Table->colortemp[ucCount2]].picturemode[psCT_Table->picturemode].colors.GreenGain,
                        sOE_CT_Table.setting[psCT_Table->colortemp[ucCount2]].picturemode[psCT_Table->picturemode].colors.BlueGain);
#endif /* 0 */
                    m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[psCT_Table->picturemode][ucCount2].ucRedGain =  sOE_CT_Table.setting[psCT_Table->colortemp[ucCount2]].picturemode[psCT_Table->picturemode].colors.RedGain;
                    m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[psCT_Table->picturemode][ucCount2].ucGreenGain = sOE_CT_Table.setting[psCT_Table->colortemp[ucCount2]].picturemode[psCT_Table->picturemode].colors.GreenGain;
                    m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[psCT_Table->picturemode][ucCount2].ucBlueGain = sOE_CT_Table.setting[psCT_Table->colortemp[ucCount2]].picturemode[psCT_Table->picturemode].colors.BlueGain;
                }
                else
                {
                    ASSERT_ALWAYS();
                }
            }

            psCT_Table = (sOE_CUSTOM_CT_TABLE*)(pcCT_Data + OE_HEADER_OFFSET + (ucCount+1)*(2 + 2*ucColorTempMode)); //picture mode + color temperature
        }

        free(pcOE_CT_Data);
        free(pcCT_Data);
    }


#if 0
    for(ucCount = 0; ucCount < DISPLAY_MODE_MAX; ucCount++)
    {
        printf("picture mode = %d\n", ucCount);
        for(ucCount2 = 0; ucCount2 < HSG_CT_MAX; ucCount2++)
        {
            printf("CT = %d, R = %3d G = %3d B = %3d\n", ucCount2,
            m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucCount][ucCount2].ucRedGain,
            m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucCount][ucCount2].ucGreenGain,
            m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucCount][ucCount2].ucBlueGain);
        }
    }
#endif /* 0 */

    for(UINT8 ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(UINT8 ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)  //A70LV_Doulas_0047 modify
        {
            UINT8 ucCT = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][0].ucCT;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucRedGain;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucGreenGain;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucBlueGain;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucRedGain;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucGreenGain;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucBlueGain;
        }
    }
#endif /* OE_TOOL */
}

void palDataMgr_sGamma_Init_Default(void)
{
#ifdef OE_TOOL
	char cFileName[128] = {'\0'};
    FILE *pFile = NULL;
    UINT8 *pcData = NULL;
    uint32_t size;
    UINT8 cHeader[OE_HEADER_OFFSET] = {0};
    sOE_GAMMA_SETTING *psGamma_Table = NULL;
    UINT8 cCount = 0, ucGammaNumber = 0;

    if(access(CONF_SCALER_COLOR_PATH, 0) == -1)
    {
        mkdir(CONF_SCALER_COLOR_PATH, 0777);
    }

    if(palSystem_ModelIDGet() == MODEL_ID_2)
    {
        snprintf(cFileName, 128, "%s/GammaSettings_2.bin", CONF_SCALER_COLOR_PATH);
    }
    else if(palSystem_ModelIDGet() == MODEL_ID_1)
    {
        snprintf(cFileName, 128, "%s/GammaSettings_1.bin", CONF_SCALER_COLOR_PATH);
    }
    else
    {
        snprintf(cFileName, 128, "%s/GammaSettings_0.bin", CONF_SCALER_COLOR_PATH);
    }

    pFile = fopen(cFileName, "rb+");

    if(pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        pcData = malloc(size);

        if(fread(pcData, sizeof(uint8_t), size, pFile) != size)
        {
            printf("(func:%s, line:%d) fread fail\n", __FUNCTION__, __LINE__);
        }

        fclose(pFile);

        memcpy(&cHeader, pcData, OE_HEADER_OFFSET);

        psGamma_Table = (sOE_GAMMA_SETTING*)(pcData + OE_HEADER_OFFSET);

        ucGammaNumber = (size - OE_HEADER_OFFSET)/sizeof(sOE_GAMMA_SETTING);

        for(cCount = 0; cCount < ucGammaNumber; cCount++)
        {
#if 0
            printf("gamma_cm %d -> gamma_oe %d\n",
                    psGamma_Table->gamma_cm,
                    psGamma_Table->gamma_oe);
#endif /* 0 */

            if(psGamma_Table->gamma_cm < eCM_GAMMA_NUMBER)
            {
                Gamma_Mapping[psGamma_Table->gamma_cm] = (UINT8)psGamma_Table->gamma_oe;
            }

            psGamma_Table++;
        }

        free(pcData);
    }

#else
    #warning "OE_TOOL not define, gamma default is 2.2"
#endif /* OE_TOOL */

}

void palDataMgr_sADC_CAL_VALUES_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sADC_cal_values, &m_sDefSysSettings.sADC_cal_values, sizeof(sADC_CAL_VALUES));
}

void palDataMgr_sWaveformstate_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sWaveformstate, &m_sDefSysSettings.sWaveformstate, sizeof(sWAVEFORMSTATE));
}

void palDataMgr_sBurin_Information_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sBurin_Information, &m_sDefSysSettings.sBurin_Information, sizeof(sBURNIN_INFORMATION));
}

void palDataMgr_sSystemDefault_Factory_Reset(void)
{
    EE_GETVAR(sSystemDefault.ucSerialNumber, m_sDefSysSettings.sSystemDefault.ucSerialNumber);  //A70LV_Doulas_0093 Add,get SN

    EE_GETVAR(sSystemDefault.ulTotalProjectorMinute, m_sDefSysSettings.sSystemDefault.ulTotalProjectorMinute); //A70LV_Larry_0268
    EE_GETVAR_SIZE(sSystemDefault.ulLightSourceMinute, 8, m_sDefSysSettings.sSystemDefault.ulLightSourceMinute); //A70LV_Larry_0268
    EE_GETVAR_SIZE(sSystemDefault.uiLightSensorFull_BLD, 96, m_sDefSysSettings.sSystemDefault.uiLightSensorFull_BLD);
    EE_GETVAR(sSystemDefault.ucABP_CalibrateFlag, m_sDefSysSettings.sSystemDefault.ucABP_CalibrateFlag);
    EE_GETVAR(sSystemDefault.ucCOLOR_OFFSET, m_sDefSysSettings.sSystemDefault.ucCOLOR_OFFSET);
    EE_GETVAR(sSystemDefault.ucPOWER_OFFSET, m_sDefSysSettings.sSystemDefault.ucPOWER_OFFSET);
	EE_GETVAR_SIZE(sSystemDefault.ucLightSensorTime, 4, m_sDefSysSettings.sSystemDefault.ucLightSensorTime);			//A65_OPTOMA_Doulas_0105
    EE_GETVAR_SIZE(sSystemDefault.uiLightSensorTarget, 8, m_sDefSysSettings.sSystemDefault.uiLightSensorTarget);		//A65_OPTOMA_Doulas_0105
    EE_GETVAR_SIZE(sSystemDefault.uiLightSensorTargetRLD, 8, m_sDefSysSettings.sSystemDefault.uiLightSensorTargetRLD);	//A65_OPTOMA_Doulas_0105


    EE_GETVAR(sSystemDefault.uiFWIndex, m_sDefSysSettings.sSystemDefault.uiFWIndex);    //A70LV_Doulas_0268
    EE_GETVAR(sSystemDefault.uiPWIndex, m_sDefSysSettings.sSystemDefault.uiPWIndex);    //A70LV_Doulas_0268

    EE_GETVAR(sSystemDefault.ucUST_LensInstallType, m_sDefSysSettings.sSystemDefault.ucUST_LensInstallType); //A35G2_BRC_Casper_0089

    EE_GETVAR(sSystemDefault.ucBIST, m_sDefSysSettings.sSystemDefault.ucBIST); //HICC2_Steven_0004

    memcpy(&m_sAppDataMgrInfo.sEepSettings.sSystemDefault,
        &m_sDefSysSettings.sSystemDefault, sizeof(sSYSTEM_DEFAULT));
}

void palDataMgr_sLightSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting,
        &m_sDefSysSettings.sUserSystemSetting.sLightSetting, sizeof(sLIGHT_SETTING));
}

void palDataMgr_sOSD_Setting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sOSD_Setting,
        &m_sDefSysSettings.sUserSystemSetting.sOSD_Setting, sizeof(sOSD_SETTING));
}

void palDataMgr_sNetworkSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sNetworkSetting,
        &m_sDefSysSettings.sUserSystemSetting.sNetworkSetting, sizeof(sNETWORK_SETTING));
}

void palDataMgr_sImageSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting,
        &m_sDefSysSettings.sUserSystemSetting.sImageSetting, sizeof(sIMAGE_SETTING));
}

void palDataMgr_sWarpSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting,
        &m_sDefSysSettings.sUserSystemSetting.sWarpSetting, sizeof(sWARP_SETTING));
}
void palDataMgr_sCommonSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting,
        &m_sDefSysSettings.sUserSystemSetting.sCommonSetting, sizeof(sCOMMON_SETTING));
}
void palDataMgr_sSourceSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting,
        &m_sDefSysSettings.sUserSystemSetting.sSourceSetting, sizeof(sSOURCE_SETTING));
}
void palDataMgr_sManualAdjustment_Factory_Reset(void)
{
#if SUPPORT_MANUAL_ADJUSTMENT   //A70LV_Doulas_0023
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sManualAdjustment,
        &m_sDefSysSettings.sUserSystemSetting.sManualAdjustment, sizeof(sMANUAL_ADJUSTMENT));
#endif
}
void palDataMgr_sSourceDependSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[0],
        &m_sDefSysSettings.sUserSystemSetting.sSourceDependSetting[0], sizeof(sSOURCE_DEPEND_SETTING)*INPUT_SOURCE_MAX);
}
void palDataMgr_sHSG_setting_Factory_Reset(void)
{
    palDataMgr_sHSG_setting_Init_Default(); //A35G2_BRC_Casper_0023

#ifdef HSG_COLORTEMPERATURE
    for(UINT8 ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(UINT8 ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)
        {
            UINT8 ucCT = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][0].ucCT;

            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucRedGain;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucGreenGain;
            m_sDefSysSettings.sUserSystemSetting.sHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucBlueGain;
        }
    }
#endif /* HSG_COLORTEMPERATURE */

    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sHSG_setting[0][0],
        &m_sDefSysSettings.sUserSystemSetting.sHSG_setting[0][0], sizeof(sHSG_SETTING)*INPUT_SOURCE_MAX*DISPLAY_MODE_MAX);
}
void palDataMgr_UserMode_sHSG_setting_Factory_Reset(void) //G100_Steven_0038
{
#ifdef HSG_COLORTEMPERATURE
    for(UINT8 ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX ; ucInputSource++)
    {
        for(UINT8 ucDisplayMode = 0; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)
        {
            UINT8 ucCT = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][0].ucCT;

            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_R_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucRedGain;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_G_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucGreenGain;
            m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[ucInputSource][ucDisplayMode].HSG_W_B_GAIN  = m_sDefSysSettings.sUserSystemSetting.sHSG_ColorTemperature[ucDisplayMode][ucCT].ucBlueGain;

        }
    }
#endif /* HSG_COLORTEMPERATURE */

    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserHSG_setting[0][0],
        &m_sDefSysSettings.sUserSystemSetting.sUserHSG_setting[0][0], sizeof(sHSG_SETTING)*INPUT_SOURCE_MAX*DISPLAY_MODE_MAX);
}

void palDataMgr_sColorSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[0][0][0],
        &m_sDefSysSettings.sUserSystemSetting.sColorSetting[0][0][0], sizeof(sCOLOR_SETTING)*INPUT_SOURCE_MAX*DISPLAY_MODE_MAX*SIGNAL_TYPE_MAX);
} //G100_Steven_0030 start
void palDataMgr_UserMode_sColorSetting_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[0][0][0],
        &m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[0][0][0], sizeof(sCOLOR_SETTING)*INPUT_SOURCE_MAX*DISPLAY_MODE_MAX*SIGNAL_TYPE_MAX);
} //G100_Steven_0030 end

void palDataMgr_sTiming_Table_Factory_Reset(void)   //A70LV_Doulas_0194
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sTiming_Table, &m_sDefSysSettings.sTiming_Table, sizeof(sTIMING_TABLE));
}

void palDataMgr_sActuator_Factory_Reset(void)
{
    memcpy(&m_sAppDataMgrInfo.sEepSettings.sActuatorSetting, &m_sDefSysSettings.sActuatorSetting, sizeof(sACTUATOR_SETTING));
}

void palDataMgr_sWaveformstate_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sWaveformstate.ucWAVEFORMSTATE_CRC !=
                    m_sDefSysSettings.sWaveformstate.ucWAVEFORMSTATE_CRC)
    {
        palDataMgr_sWaveformstate_Factory_Reset();

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    }
}

void palDataMgr_sBurin_Information_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sBurin_Information.ucBurnIn_CRC !=
                    m_sDefSysSettings.sBurin_Information.ucBurnIn_CRC)
    {
        palDataMgr_sBurin_Information_Factory_Reset();

        utilDataMgr_RAMToFileBurnin_CM(&m_sDefSysSettings, eDATA_NODE_BURNIN);	//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sSystemDefault_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sSystemDefault.ucSYSTEM_DEFAULT_CRC !=
                    m_sDefSysSettings.sSystemDefault.ucSYSTEM_DEFAULT_CRC)
    {
        palDataMgr_sSystemDefault_Factory_Reset();

        utilDataMgr_RAMToFileProjectorInfo_CM(&m_sDefSysSettings, eDATA_NODE_PROJECTORINFO);	//HICC2_Doulas_0003
        utilDataMgr_RAMToFileSystemCal_CM(&m_sDefSysSettings, eDATA_NODE_SYSTEMCAL);		//HICC2_Doulas_0003
        utilDataMgr_RAMToFileSystemHours_CM(&m_sDefSysSettings, eDATA_NODE_SYSTEMHOURS);	//HICC2_Doulas_0003
        utilDataMgr_RAMToFileAutoBrightness_CM(&m_sDefSysSettings, eDATA_NODE_AUTOBRIGHTNESS);	//HICC2_Doulas_0003
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);		//HICC2_Doulas_0003
        utilDataMgr_RAMToFileWarping_CM(&m_sDefSysSettings, eDATA_NODE_WARPING);	//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sLightSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sLightSetting.ucLIGHT_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sLightSetting.ucLIGHT_SETTING_CRC)
    {
        palDataMgr_sLightSetting_Factory_Reset();
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);		//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sOSD_Setting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sOSD_Setting.ucOSD_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sOSD_Setting.ucOSD_SETTING_CRC)
    {
        palDataMgr_sOSD_Setting_Factory_Reset();
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);	//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sNetworkSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sNetworkSetting.ucNETWORK_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sNetworkSetting.ucNETWORK_SETTING_CRC)
    {
        palDataMgr_sNetworkSetting_Factory_Reset();
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);	//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sImageSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sImageSetting.ucIMAGE_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sImageSetting.ucIMAGE_SETTING_CRC)
    {
        palDataMgr_sImageSetting_Factory_Reset();
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);		//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sWarpSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sWarpSetting.ucWARP_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sWarpSetting.ucWARP_SETTING_CRC)
    {
        palDataMgr_sWarpSetting_Factory_Reset();
        utilDataMgr_RAMToFileWarping_CM(&m_sDefSysSettings, eDATA_NODE_WARPING);	//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sCommonSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sCommonSetting.uiCOMMON_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sCommonSetting.uiCOMMON_SETTING_CRC)   //A70LV_Doulas_0022
    {
        palDataMgr_sCommonSetting_Factory_Reset();
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);		//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sSourceSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucSOURCE_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.sSourceSetting.ucSOURCE_SETTING_CRC)
    {
        palDataMgr_sSourceSetting_Factory_Reset();
        utilDataMgr_RAMToFileCommonGui_CM(&m_sDefSysSettings, eDATA_NODE_COMMONGUI);		//HICC2_Doulas_0003

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sSourceDependSetting_Reset_Check(void)
{
    if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.ucSOURCE_DEPEND_SETTING_CRC !=
                    m_sDefSysSettings.sUserSystemSetting.ucSOURCE_DEPEND_SETTING_CRC)
    {
        palDataMgr_sSourceDependSetting_Factory_Reset();

        utilDataMgr_RAMToFileSourceTableCommon_CM(&m_sDefSysSettings, eDATA_NODE_NUMBER);		//HICC2_Doulas_0003

        //EE_PUTVAR(sUserSystemSetting.sSourceDependSetting, m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting);

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

void palDataMgr_sColorSetting_Reset_Check(void)
{
    //if(m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.ucCOLOR_DEPEND_SETTING_CRC !=
                    //m_sDefSysSettings.sUserSystemSetting.ucCOLOR_DEPEND_SETTING_CRC)
    {

        utilDataMgr_RAMToFileSourceTableColor_CM(&m_sDefSysSettings, eDATA_NODE_NUMBER);	//HICC2_Doulas_0003

        //EE_PUTVAR(sUserSystemSetting.sColorSetting, m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting);

        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    }
}

UINT8 palDataMgr_Reset_RGB_Gain_Offset(void)   //A70LV_Doulas_0022
{
    UINT8  ucInputSource = 0;
    UINT8  ucDisplayMode;
    UINT8  ucSignalType;

#if	1		//HICC2_Steven_0049 //A70LK_Nina_0008
    UINT8  ucUser_Color_Mode = 0;
    ucInputSource = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceSetting.ucInputSourceMain;
    ucDisplayMode = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPresetMode;
    ucSignalType = palDataMgr_SignalType_Get();

	if(ucDisplayMode != eCM_PICTURE_SETTINGS_USER)
	{
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain     = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain   = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain    = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset   = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset  = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset;
	}
	else
	{
		ucUser_Color_Mode = m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sSourceDependSetting[ucInputSource].ucPreUserMode;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucRedGain     = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucRedGain;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucGreenGain   = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucGreenGain;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucBlueGain    = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucBlueGain;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucRedOffset   = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucRedOffset;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucGreenOffset = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucGreenOffset;
        m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucBlueOffset  = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucUser_Color_Mode][ucSignalType].ucBlueOffset;
	}
#else
    for(ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX; ucInputSource++)
    {
        for(ucDisplayMode = 0 ; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)
        {
            for(ucSignalType = 0 ; ucSignalType < SIGNAL_TYPE_MAX ; ucSignalType++)
                {
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain     = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain   = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain    = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset   = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset  = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset;
//G100_Steven_0045 start
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain     = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain   = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain    = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset   = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset  = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset;
//G100_Steven_0045 end
                }
        }
    }
#endif //HICC2_Steven_0049 end
    utilDataMgr_RAMToFileSourceTableColor_CM(&m_sAppDataMgrInfo.sEepSettings, eDATA_NODE_NUMBER);	//HICC2_Doulas_0003

    return eEXEC_CODE_PASS;
}

UINT8 palDataMgr_Reset_All_RGB_Gain_Offset(void)   //HICC2_Steven_0049  //A70LV_Doulas_0022
{
    UINT8  ucInputSource = 0;
    UINT8  ucDisplayMode;
    UINT8  ucSignalType;

    for(ucInputSource = 0; ucInputSource < INPUT_SOURCE_MAX; ucInputSource++)
    {
        for(ucDisplayMode = 0 ; ucDisplayMode < DISPLAY_MODE_MAX ; ucDisplayMode++)
        {
            for(ucSignalType = 0 ; ucSignalType < SIGNAL_TYPE_MAX ; ucSignalType++)
                {
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain     = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain   = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain    = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset   = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset  = m_sDefSysSettings.sUserSystemSetting.sColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset;
//G100_Steven_0045 start
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain     = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain   = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain    = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueGain;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset   = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucRedOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucGreenOffset;
                    m_sAppDataMgrInfo.sEepSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset  = m_sDefSysSettings.sUserSystemSetting.sUserColorSetting[ucInputSource][ucDisplayMode][ucSignalType].ucBlueOffset;
//G100_Steven_0045 end
                }
        }
    }

    utilDataMgr_RAMToFileSourceTableColor_CM(&m_sAppDataMgrInfo.sEepSettings, eDATA_NODE_NUMBER);	//HICC2_Doulas_0003

    return eEXEC_CODE_PASS;
}

#endif /* __APPDATAINITMGR_C__ */
