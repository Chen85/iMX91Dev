#include "Common.h"
#include "utilStorageCfg.h"
#include "palFormatterMgr.h"
#include "halFormatter.h"
#include "utilDbgMsg.h"
#include "ProjectSettings.h"

///////////////// callback function start ///////////////
sFORMATTER_MGR_CALLBACK sFormatterMgr_Callback =
{
    .fpGetPictureModeCb = NULL,
};

void palFormatterMgr_RegCallback(sFORMATTER_MGR_CALLBACK fpCallback)
{
    sFormatterMgr_Callback = fpCallback ;
}
////////////////// callback function end ////////////////

typedef struct
{
    eORIENTATION_STATE ucOrientationState;
}sPAL_FORMATTER_INFO;

sPAL_FORMATTER_INFO sFormatterInfo;


eRESULT palFormatterMgr_GetSSIPWMDriverLevels_1(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_GetSSIPWMDriverLevels_1(uSSI);

    return eResult;
}

eRESULT palFormatterMgr_SetSSIPWMDriverLevels_1(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SetSSIPWMDriverLevels_1(uSSI);

	return eResult;
}

eRESULT palFormatterMgr_SetDisplaySFGColor(UINT16 uiSFG_Red,UINT16 uiSFG_Green,UINT16 uiSFG_Blue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SetDisplaySFGColor(uiSFG_Red, uiSFG_Green, uiSFG_Blue);

    return eResult;
}

eRESULT palFormatterMgr_GetDisplaySFGColor(UINT16 *uiSFG_Red,UINT16 *uiSFG_Green,UINT16 *uiSFG_Blue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_GetDisplaySFGColor(uiSFG_Red, uiSFG_Green, uiSFG_Blue);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_Switch_Set(UINT16 ucActuator)
{
    eRESULT eResult = rcINVALID;

#ifdef CURSOR_FIXTURE
	ucActuator = ets_OFF;
#endif
    eResult = halFormatter_Actuator_Switch_Set(ucActuator);

    return eResult;
}


eRESULT palFormatterMgr_PW_Index_Set(UINT16 uiPWIndex)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_PW_Index_Set(uiPWIndex);

    return eResult;
}

eRESULT palFormatterMgr_PW_Index_Get(UINT16 *uiPWIndex)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_PW_Index_Get(uiPWIndex);

    return eResult;
}

eRESULT palFormatterMgr_FW_Index_Set(UINT16 uiFWIndex)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_FW_Index_Set(uiFWIndex);

    return eResult;
}

eRESULT palFormatterMgr_FW_Index_Get(UINT16 *uiFWIndex)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_FW_Index_Get(uiFWIndex);

    return eResult;
}

eRESULT palFormatterMgr_PowerMode_Set(UINT8 ucPowerMode)
{
    eRESULT eResult = rcINVALID;
    UINT32 dwVerDDPSq = 0; //A35G2_BRC_Casper_0122
	//UINT16	uiDatax[13] = {0};
    uSSI_DRIVER uSSI = {0};

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Increase ucPowerMode = %d\r\n", __FUNCTION__, __LINE__, ucPowerMode);

    halFormatter_SEQ_RevisionGet(&dwVerDDPSq); //A35G2_BRC_Casper_0122

	if(ucPowerMode == 0)
	{
	    #if defined(PLATFORM_H60_2K)
        {
	        uSSI.sLD.uiRed = 200;
	        uSSI.sLD.uiGreen = 200;
	        uSSI.sLD.uiBlue = 200;
	        uSSI.sLD.uiYellow = 200;
	        uSSI.sLD.uiRLD_Red = 0;
	        uSSI.sLD.uiRLD_Yellow = 0;
        }
        #elif defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)    //HICC2_Doulas_0100
        {
	        uSSI.sLD.uiRed = 200;
	        uSSI.sLD.uiGreen = 200;
	        uSSI.sLD.uiBlue = 200;
	        uSSI.sLD.uiYellow = 200;
	        uSSI.sLD.uiRLD_Red = 0;
	        uSSI.sLD.uiRLD_Yellow = 0;
        }
		#elif defined(PLATFORM_A70G2)
        {
	        uSSI.sLD.uiRed = 133;
	        uSSI.sLD.uiGreen = 133;
	        uSSI.sLD.uiBlue = 133;
	        uSSI.sLD.uiYellow = 0;
	        uSSI.sLD.uiRLD_Red = 0;
	        uSSI.sLD.uiRLD_Yellow = 0;
        }
		#endif

		eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI);
	}
	else if(ucPowerMode == 1)
	{
		uSSI.sLD.uiRed = 300;
	    uSSI.sLD.uiGreen = 300;
	    uSSI.sLD.uiBlue = 300;
	    uSSI.sLD.uiYellow = 300;
	    uSSI.sLD.uiRLD_Red = 0;
	    uSSI.sLD.uiRLD_Yellow = 0;
		eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI);
	}
	else if(ucPowerMode == 2)
    {
        switch(Board_ModelID_Get())
        {
            default:
            case MODEL_ID_0:
                #if defined(PLATFORM_H60_2K)
                {
            		uSSI.sLD.uiRed        = 800;    //HICC2_Doulas_0100
            	    uSSI.sLD.uiGreen      = 760;
            	    uSSI.sLD.uiBlue       = 700;    //HICC2_Doulas_0100
            	    uSSI.sLD.uiYellow     = 700;    //HICC2_Doulas_0100
            	    uSSI.sLD.uiRLD_Red    = 600;
            	    uSSI.sLD.uiRLD_Yellow = 0;
	                eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI); //HICC2_Doulas_0064 Modify//1800
	                LOG_MSG(db_HAL_FORMATTER, "1800 PWM Set\r\n");
           		}
           		#elif defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)    //HICC2_Doulas_0124//HICC2_Doulas_0100
           		{
            		uSSI.sLD.uiRed        = 670;
            	    uSSI.sLD.uiGreen      = 700;
            	    uSSI.sLD.uiBlue       = 630;
            	    uSSI.sLD.uiYellow     = 740;
            	    uSSI.sLD.uiRLD_Red    = 0;
            	    uSSI.sLD.uiRLD_Yellow = 0;
	                eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI);
	                LOG_MSG(db_HAL_FORMATTER, "1600 PWM Set\r\n");
           		}
                #elif defined(PLATFORM_A70G2)
                {
            		uSSI.sLD.uiRed        = 930;
            	    uSSI.sLD.uiGreen      = 920;
            	    uSSI.sLD.uiBlue       = 850;
            	    uSSI.sLD.uiYellow     = 910;
            	    uSSI.sLD.uiRLD_Red    = 0;
            	    uSSI.sLD.uiRLD_Yellow = 0;
        		    eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI);
                    LOG_MSG(db_HAL_FORMATTER, "Model 0 PWM Set\r\n");
           		}
                #endif

                break;

            case MODEL_ID_1:
                #if defined(PLATFORM_H60_2K)
                {
            		uSSI.sLD.uiRed        = 760;
            	    uSSI.sLD.uiGreen      = 725;
            	    uSSI.sLD.uiBlue       = 675; //HICC2_Doulas_0124
            	    uSSI.sLD.uiYellow     = 675;
            	    uSSI.sLD.uiRLD_Red    = 600;
            	    uSSI.sLD.uiRLD_Yellow = 0;
                    eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI); //HICC2_Doulas_0064 Modify//2400
               		LOG_MSG(db_HAL_FORMATTER, "2400 PWM Set\r\n");
                }
                #elif defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)    //HICC2_Doulas_0124//HICC2_Doulas_0100
           		{
            		uSSI.sLD.uiRed        = 680;
            	    uSSI.sLD.uiGreen      = 730;
            	    uSSI.sLD.uiBlue       = 640;
            	    uSSI.sLD.uiYellow     = 800;
            	    uSSI.sLD.uiRLD_Red    = 0;
            	    uSSI.sLD.uiRLD_Yellow = 0;
	                eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI);
	                LOG_MSG(db_HAL_FORMATTER, "2100 PWM Set\r\n");
           		}
                #elif defined(PLATFORM_A70G2)
                {
                    if((UINT8)(dwVerDDPSq>>24) == 'B' && (UINT8)(dwVerDDPSq>>16) == 'T')
                    {
                		uSSI.sLD.uiRed        = 800;
                	    uSSI.sLD.uiGreen      = 850;
                	    uSSI.sLD.uiBlue       = 790;
                	    uSSI.sLD.uiYellow     = 885;
                	    uSSI.sLD.uiRLD_Red    = 0;
                	    uSSI.sLD.uiRLD_Yellow = 0;
        		        eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI); //9.5k ////A35G2_BRC_Casper_0127
                        LOG_MSG(db_HAL_FORMATTER, "9.5K Barco W11 PWM Set\r\n");
                    }
                    else
                    {
                		uSSI.sLD.uiRed        = 940;
                	    uSSI.sLD.uiGreen      = 910;
                	    uSSI.sLD.uiBlue       = 790;
                	    uSSI.sLD.uiYellow     = 750;
                	    uSSI.sLD.uiRLD_Red    = 0;
                	    uSSI.sLD.uiRLD_Yellow = 0;
        		        eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI); //9.5k //A35G2_BRC_Casper_0131
                        LOG_MSG(db_HAL_FORMATTER, "9.5K Christie DWU1100 PWM Set\r\n");
                    }
                }
                #endif
                break;

            case MODEL_ID_2:  //HICC2_Doulas_0100
        		uSSI.sLD.uiRed        = 600;
        	    uSSI.sLD.uiGreen      = 600;
        	    uSSI.sLD.uiBlue       = 600;
        	    uSSI.sLD.uiYellow     = 600;
        	    uSSI.sLD.uiRLD_Red    = 0;
        	    uSSI.sLD.uiRLD_Yellow = 0;
    		    eResult= halFormatter_SetSSIPWMDriverLevels_1(&uSSI); //11k
        		LOG_MSG(db_HAL_FORMATTER, "Model 2 PWM Set\r\n");
                break;
            break;
        }
    }
	else
	{
		eResult = rcERROR;
		LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Fail\r\n", __FUNCTION__, __LINE__);
	}

    return eResult;
}

eRESULT palFormatterMgr_Actuator_Switch(UINT8 ucActuator)
{
    eRESULT eResult = rcINVALID;
	
#ifdef CURSOR_FIXTURE	
	ucActuator = ets_OFF;
#endif

    eResult = halFormatter_Actuator_Switch(ucActuator);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_FixedOut_Set(UINT8 ucCh, UINT8 ucFixedOut)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_FixedOut_Set(ucCh, ucFixedOut);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_FixedOut_Get(UINT8 ucCh, UINT8 *ucFixedOut)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_FixedOut_Get(ucCh, ucFixedOut);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_Gain_Set(UINT8 ucCh, UINT8 ucActuatorGain)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_Gain_Set(ucCh, ucActuatorGain);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_Gain_Get(UINT8 ucCh, UINT8 *ucActuatorGain)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_Gain_Get(ucCh, ucActuatorGain);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_Segment_Length_Set(UINT8 ucCh, UINT16 uiSeg)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_Segment_Length_Set(ucCh, uiSeg);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_Segment_Length_Get(UINT8 ucCh, UINT16 *uiSeg)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_Segment_Length_Get(ucCh, uiSeg);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_SubFrameDelay_Set(UINT8 ucCh, UINT32 ulDelay)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_SubFrameDelay_Set(ucCh, ulDelay);

    return eResult;
}

eRESULT palFormatterMgr_Actuator_SubFrameDelay_Get(UINT8 ucCh, UINT32 *ulDelay)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Actuator_SubFrameDelay_Get(ucCh, ulDelay);

    return eResult;
}

eRESULT palFormatterMgr_Standby(void) //A70LV_Larry_0212
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_Standby();

    return eResult;
}

eRESULT palFormatterMgr_TestPattern(UINT8 ucPattern)
{
    eRESULT eResult = rcSUCCESS;

    halFormatter_TestPattern(ucPattern);

    BOOL bActuator_Switch = TRUE;
	palDataMgr_Data_Access(edcACTUATOR_SWITCH, edaREAD, &bActuator_Switch) ;
    palFormatterMgr_Actuator_Switch_Set(bActuator_Switch);

    return eResult;
}

eRESULT palFormatterMgr_HorFlip(BOOL bH_Flip)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_HorFlip(bH_Flip);

    return eResult;
}

eRESULT palFormatterMgr_VerFlip(BOOL bV_Flip)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_VerFlip(bV_Flip);

    return eResult;
}

void palFormatterMgr_AsicReadySet(BOOL bBnable)
{
    halFormatter_AsicReadySet(bBnable);
}

eRESULT palFormatterMgr_HorFlipGet(BOOL *bH_Flip)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_HorFlipGet(bH_Flip);

    return eResult;
}

eRESULT palFormatterMgr_VerFlipGet(BOOL *bV_Flip)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_VerFlipGet(bV_Flip);

    return eResult;
}

eRESULT palFormatterMgr_GetSSIPWMDriverLevels_2(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_GetSSIPWMDriverLevels_2(uSSI);

    return eResult;
}

eRESULT palFormatterMgr_SetSSIPWMDriverLevels_2(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SetSSIPWMDriverLevels_2(uSSI);

    return eResult;
}

eRESULT palFormatterMgr_SystemModeSet(UINT8 cSystemMode)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SystemModeSet(cSystemMode);

    return eResult;
}

eRESULT palFormatterMgr_SystemModeGet(UINT8 *cSystemMode)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SystemModeGet(cSystemMode);

    return eResult;
}

eRESULT palFormatterMgr_SplashLoad(UINT8 cNum)
{
    eRESULT eResult = rcINVALID;

    #if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    if(cNum > 0)
    {
        cNum = 2*cNum - 1;
    }
    #endif

    eResult = halFormatter_SplashLoad(cNum);

    return eResult;
}

eRESULT palFormatterMgr_dv4422_TestPattern(UINT8 cTestPattern)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_dv4422_TestPattern(cTestPattern);

    return eResult;
}

eRESULT palFormatterMgr_CW_SpokeTest(UINT8 cEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_CW_SpokeTest(cEnable);

    return eResult;
}

eRESULT palFormatterMgr_ColorOffsetSet(UINT8 ucColorOffset)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ColorOffsetSet(ucColorOffset);

    return eResult;
}

eRESULT palFormatterMgr_IMG_AlgorithmSet(IMG_ALGORITHMS eAlgo, BOOL bState)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_IMG_AlgorithmSet(eAlgo, bState);

    return eResult;
}

eRESULT palFormatterMgr_IMG_GammaLutSet(UINT8 cGamma)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_IMG_GammaLutSet(cGamma);

    return eResult;
}

eRESULT palFormatterMgr_IMG_GammaLutGet(UINT8* cGamma)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_IMG_GammaLutGet(cGamma);

    return eResult;
}

eRESULT palFormatterMgr_DB_EnableSet(UINT8 cDB)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_EnableSet(cDB);

    return eResult;
}

eRESULT palFormatterMgr_DB_EnableGet(UINT8 *cDB)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_EnableGet(cDB);

    return eResult;
}

eRESULT palFormatterMgr_DB_SetAperture(UINT16 cPosition)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_SetAperture(cPosition);

    return eResult;
}

eRESULT palFormatterMgr_DB_GetAperture(UINT16 *cPosition)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_GetAperture(cPosition);

    return eResult;
}

eRESULT palFormatterMgr_DB_SetMinMaxAperture(UINT16 ucMinPosition,UINT16 ucMaxPosition)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_SetMinMaxAperture(ucMinPosition, ucMaxPosition);

    return eResult;
}

eRESULT palFormatterMgr_DB_GetMinMaxAperture(UINT16 *ucMinPosition,UINT16 *ucMaxPosition)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_GetMinMaxAperture(ucMinPosition, ucMaxPosition);

    return eResult;
}

eRESULT palFormatterMgr_DB_SetOpenCloseAperture(UINT8 cOpenClose)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_SetOpenCloseAperture(cOpenClose);

    return eResult;
}

eRESULT palFormatterMgr_DB_GetCurrentAperture(UINT16 *ucCurrentAperture)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_GetCurrentAperture(ucCurrentAperture);

    return eResult;
}

eRESULT palFormatterMgr_SYS_AppVersionGet(UINT16 *wDDP)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SYS_AppVersionGet(wDDP);

    return eResult;
}

eRESULT palFormatterMgr_VersionSaveGet(UINT8 *pcData)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_VersionSaveGet(pcData);

    return eResult;
}

eRESULT palFormatterMgr_SEQ_RevisionGet(UINT32 *ucData)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SEQ_RevisionGet(ucData);

    return eResult;
}


eRESULT palFormatterMgr_3D_3DModeSet(BOOL cEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_3DModeSet(cEnable);

    return eResult;
}


eRESULT palFormatterMgr_3D_3DModeGet(BOOL *pbEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_3DModeGet(pbEnable);

    return eResult;
}


eRESULT palFormatterMgr_IMG_WhitePeakingSet(UINT8 cWhitePeak)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_IMG_WhitePeakingSet(cWhitePeak);

    return eResult;
}

eRESULT palFormatterMgr_IMG_WhitePeakingGet(UINT8 *cWhitePeak)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_IMG_WhitePeakingGet(cWhitePeak);

    return eResult;
}

eRESULT palFormatterMgr_3D_RefPolaritySet( BOOL bRef3D, BOOL bOddEven, BOOL bTopField, BOOL bInvert )
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_RefPolaritySet(bRef3D, bOddEven, bTopField, bInvert );

    return eResult;
}


eRESULT palFormatterMgr_3D_RefPolarityGet(BOOL *bRef3D, BOOL *bOddEven, BOOL *bTopField, BOOL *bInvert )
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_RefPolarityGet(bRef3D, bOddEven, bTopField, bInvert);

    return eResult;
}


eRESULT palFormatterMgr_3D_DLPLinkPulseSet(BOOL bEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_DLPLinkPulseSet(bEnable);

    return eResult;
}


eRESULT palFormatterMgr_3D_DLPLinkPulseGet(BOOL *pbEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_DLPLinkPulseGet(pbEnable);

    return eResult;
}


eRESULT palFormatterMgr_DISP_FreezeSet(UINT8 cFreeze)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_FreezeSet(cFreeze);

    return eResult;
}


eRESULT palFormatterMgr_DISP_FreezeGet(UINT8 *pcFreeze)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_FreezeGet(pcFreeze);

    return eResult;
}


eRESULT palFormatterMgr_DB_SetBorderCfg(UINT16 ucScalerOutHS,UINT16 ucScalerOutVS,UINT16 ucScalerOutHW,UINT16 ucScalerOutVW)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_SetBorderCfg(ucScalerOutHS, ucScalerOutVS, ucScalerOutHW, ucScalerOutVW);

    return eResult;
}

eRESULT palFormatterMgr_DB_SetClipPixels(UINT16 ucClipPixels)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DB_SetClipPixels(ucClipPixels);

    return eResult;
}

eRESULT palFormatterMgr_IsColorWheelSpinning(BOOL *cSpin)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_IsColorWheelSpinning(cSpin);

    return eResult;
}


eRESULT palFormatterMgr_HSGSet(UINT8* hsgdata)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_HSGSet(hsgdata);

    return eResult;
}


INT16 palFormatterMgr_HSGdata_Trans(INT8 cHSG_OSDvalue, BOOL bHue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_HSGdata_Trans(cHSG_OSDvalue, bHue);

    return eResult;
}


eRESULT palFormatterMgr_HSGValueSet(sHSG_SETTING hsgdata)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_HSGValueSet(hsgdata);

    return eResult;
}


eRESULT palFormatterMgr_CCADataSend(UINT8 *data, UINT16 wSize)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_CCADataSend(data, wSize);

    return eResult;
}


eRESULT palFormatterMgr_CCADataGet(UINT8 *data)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_CCADataGet(data);

    return eResult;
}


eRESULT palFormatterMgr_HSG_UnitGainSet(void)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_HSG_UnitGainSet();

    return eResult;
}


eRESULT palFormatterMgr_SYS_SystemStatusGet(UINT8 *pcStatus)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SYS_SystemStatusGet(pcStatus);

    return eResult;
}


eRESULT palFormatterMgr_DISP_BlackLevelSet(UINT8 bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_BlackLevelSet(bValue);

    return eResult;
}


eRESULT palFormatterMgr_DISP_BlackLevelGet(UINT8 *bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_BlackLevelGet(bValue);

    return eResult;
}


eRESULT palFormatterMgr_DISP_BlackLevelThresholdSet(UINT8 bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_BlackLevelThresholdSet(bValue);

    return eResult;
}


eRESULT palFormatterMgr_DISP_BlackLevelThresholdGet(UINT8 *bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_BlackLevelThresholdGet(bValue);

    return eResult;
}

eRESULT palFormatterMgr_DISP_BlackStatusSet(UINT8 bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_BlackStatusSet(bValue);

    return eResult;
}

eRESULT palFormatterMgr_DISP_BlackPowerStatusGet(UINT8 *bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DISP_BlackPowerStatusGet(bValue);

    return eResult;
}


eRESULT palFormatterMgr_ILL_AuxSyncSet(BOOL bEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ILL_AuxSyncSet(bEnable);

    return eResult;
}


eRESULT palFormatterMgr_ILL_AuxSyncGet(BOOL *pbEnable)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ILL_AuxSyncGet(pbEnable);

    return eResult;
}


eRESULT palFormatterMgr_ILL_AuxSyncTypeSet(UINT8 *pcSyncType)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ILL_AuxSyncTypeSet(pcSyncType);

    return eResult;
}


eRESULT palFormatterMgr_ILL_AuxSyncTypeGet(UINT8 *pcSyncType)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ILL_AuxSyncTypeGet(pcSyncType);

    return eResult;
}


eRESULT palFormatterMgr_ILL_BoostRLDEnableSet(UINT8 cEn)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ILL_BoostRLDEnableSet(cEn);

    return eResult;
}


eRESULT palFormatterMgr_ILL_BoostRLDEnableGet(UINT8 *pcEn)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ILL_BoostRLDEnableGet(pcEn);

    return eResult;
}


eRESULT palFormatterMgr_DMD_BGDisplayModeSet(UINT8 cMode)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DMD_BGDisplayModeSet(cMode);

    return eResult;
}


eRESULT palFormatterMgr_DMD_BGDisplayModeGet(UINT8 *pcMode)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DMD_BGDisplayModeGet(pcMode);

    return eResult;
}


eRESULT palFormatterMgr_SensorTimingGet(UINT16 *SensorStartDelay, UINT16 *RedSensorTiming ,UINT16 *GreenSensorTiming,UINT16 *BlueSensorTiming,UINT16 *YellowSensorTiming)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SensorTimingGet(SensorStartDelay, RedSensorTiming, GreenSensorTiming, BlueSensorTiming, YellowSensorTiming);

    return eResult;
}


eRESULT palFormatterMgr_SensorTimingSet(UINT16 SensorStartDelay, UINT16 RedSensorTiming ,UINT16 GreenSensorTiming,UINT16 BlueSensorTiming,UINT16 YellowSensorTiming)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SensorTimingSet(SensorStartDelay, RedSensorTiming, GreenSensorTiming, BlueSensorTiming, YellowSensorTiming);

    return eResult;
}


eRESULT palFormatterMgr_SensorDataGet(UINT16 *RedSensorData ,UINT16 *GreenSensorData,UINT16 *BlueSensorData,UINT16 *YellowSensorData)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SensorDataGet(RedSensorData, GreenSensorData, BlueSensorData, YellowSensorData);

    return eResult;
}


eRESULT palFormatterMgr_ImageRGBGainGet(UINT16 *CSCRedGain ,UINT16 *CSCGreenGain,UINT16 *CSCBlueGain)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ImageRGBGainGet(CSCRedGain, CSCGreenGain, CSCBlueGain);

    return eResult;
}


eRESULT palFormatterMgr_ImageRGBGainSet(UINT16 CSCRedGain ,UINT16 CSCGreenGain,UINT16 CSCBlueGain)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ImageRGBGainSet(CSCRedGain, CSCGreenGain, CSCBlueGain);

    return eResult;
}


eRESULT palFormatterMgr_DynamicBlackUsePWMGet(UINT16 *PWM_FULL, UINT16 *PWM_ECO)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DynamicBlackUsePWMGet(PWM_FULL, PWM_ECO);

    return eResult;
}


eRESULT palFormatterMgr_DynamicBlackUsePWMSet(UINT16 *PWM_FULL, UINT16 *PWM_ECO)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DynamicBlackUsePWMSet(PWM_FULL, PWM_ECO);

    return eResult;
}


eRESULT palFormatterMgr_DynamicBlackTableGet(UINT8 *DA_Table)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DynamicBlackTableGet(DA_Table);

    return eResult;
}


eRESULT palFormatterMgr_DynamicBlackTableSet(UINT8 DA_Table)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DynamicBlackTableSet(DA_Table);

    return eResult;
}


eRESULT palFormatterMgr_BCCalibrateSet(BOOL bEnable,UINT32 dRed,UINT32 dGreen,UINT32 dBlue,UINT32 dYellow)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_BCCalibrateSet(bEnable, dRed, dGreen, dBlue, dYellow);

    return eResult;
}


eRESULT palFormatterMgr_SegmentColorSet(UINT8 cSegmentColorEnabled,UINT8 cSegmentColor)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SegmentColorSet(cSegmentColorEnabled, cSegmentColor);

    return eResult;
}


eRESULT palFormatterMgr_SegmentColorGet(UINT8 *cSegmentColorEnabled,UINT8 *cSegmentColor)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_SegmentColorGet(cSegmentColorEnabled, cSegmentColor);

    return eResult;
}


eRESULT palFormatterMgr_DBSpeedSet(UINT16 cDB_Speed)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DBSpeedSet(cDB_Speed);

    return eResult;
}


eRESULT palFormatterMgr_DBSpeedGet(UINT16 *cDB_Speed)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_DBSpeedGet(cDB_Speed);

    return eResult;
}


eRESULT palFormatterMgr_ALC_RGB_LevelGet(UINT16 *RGBLevel)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ALC_RGB_LevelGet(RGBLevel);

    return eResult;
}


eRESULT palFormatterMgr_CWIndexClockGet(UINT32 *dClockPeriod,UINT32 *dCW1_Frequence)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_CWIndexClockGet(dClockPeriod, dCW1_Frequence);

    return eResult;
}


//3D SYNC Selection:
// [0] : 3D sync input
// [1] : MST8535 3D sync output
// [2] : None 3D SYNC (Auto sync off)
eRESULT palFormatterMgr_3D_SYNC_SelectionSet(UINT8 c3D_SYNC_Selection)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_SYNC_SelectionSet(c3D_SYNC_Selection);

    return eResult;
}


eRESULT palFormatterMgr_3D_SYNC_SelectionGet(UINT8 *c3D_SYNC_Selection)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_SYNC_SelectionGet(c3D_SYNC_Selection);

    return eResult;
}


//3D SYNC Out:
// [0] : To Emitter
// [1] : To Next Projector
eRESULT palFormatterMgr_3D_SYNC_OutSelectionSet(UINT8 c3D_SYNC_Out)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_SYNC_OutSelectionSet(c3D_SYNC_Out);

    return eResult;
}


eRESULT palFormatterMgr_3D_SYNC_OutSelectionGet(UINT8 *c3D_SYNC_Out)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_SYNC_OutSelectionGet(c3D_SYNC_Out);

    return eResult;
}


eRESULT palFormatterMgr_3D_SYNC_InputMonitorGet(BOOL *b3DSyncAvaliable,BOOL *b3DSyncLost,UINT8 *cdatapath_state)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_3D_SYNC_InputMonitorGet(b3DSyncAvaliable, b3DSyncLost, cdatapath_state);

    return eResult;
}


eRESULT palFormatterMgr_3D_ViewingModeSet(UINT8 cViewingMode)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_3D_ViewingModeSet(cViewingMode);

    return eResult;
}

eRESULT palFormatterMgr_3D_ViewingModeGet(UINT8 *cViewingMode)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_3D_ViewingModeGet(cViewingMode);

    return eResult;
}

eRESULT palFormatterMgr_ConstantPowerSet(UINT8 cPower)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ConstantPowerSet(cPower);

    return eResult;
}


eRESULT palFormatterMgr_PWM_Set(UINT8 cIndex, UINT16 uiValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_PWM_Set(cIndex, uiValue);

    return eResult;
}

//H2 wait review
UINT16 palFormatterMgr_PWM_Get(UINT8 cIndex)
{
    return halFormatter_PWM_Get(cIndex);
}

eRESULT palFormatterMgr_PWM_Update(void)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_PWM_Update();

    return eResult;
}


UINT16 palFormatterMgr_RLD_PWM_Set(UINT16 uiRValue,UINT16 uiYValue)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_RLD_PWM_Set(uiRValue, uiYValue);

    return eResult;
}

BOOL palDDP4422_IsAsicReady(void)
{
    return halDDP4422_IsAsicReady();
}

eRESULT palFormatterMgr_ColorEnhancementSet(UINT8 ucColorEnhancement)
{
    eRESULT eResult = rcINVALID;

    eResult = halFormatter_ColorEnhancementSet(ucColorEnhancement);

    return eResult;
}

//H2 wait review
eRESULT palFormatterMgr_WallColorSet(UINT8 ucWallColor)
{
    eRESULT  eResult = rcINVALID;
    UINT16 uiRedGain,uiGreenGain,uiBlueGain;
    UINT8  ucPresetMode;
    UINT8  ucGuiPresetMode;
    UINT8  ucGuiWallColor;

    sFormatterMgr_Callback.fpGetPictureModeCb(&ucPresetMode);

    //ucGuiPresetMode = CM2GUI(edcPICTURE_SETTINGS, ucPresetMode); //HICC2_Casper_0015
    if((INT8)ucPresetMode < 0 )
    {
        ucPresetMode = 0 ;
    }

    //ucGuiWallColor = CM2GUI(edcWALL_COLOR, ucWallColor); //HICC2_Casper_0015
    if((INT8)ucWallColor < 0 )
    {
        ucWallColor = 0 ;
    }

    uiRedGain = sWallColorValues[ucPresetMode][ucWallColor].RedGain ;
    uiGreenGain = sWallColorValues[ucPresetMode][ucWallColor].GreenGain ;
    uiBlueGain = sWallColorValues[ucPresetMode][ucWallColor].BlueGain ;

    LOG_MSG(db_HAL_FORMATTER, "palFormatterMgr_WallColorSet RGB Gain : %d,%d,%d \r\n", uiRedGain,uiGreenGain,uiBlueGain);

    eResult = halFormatter_ImageRGBGainSet(uiRedGain, uiGreenGain, uiBlueGain);

    return eResult;
}

#if 0
eRESULT palFormatterMgr_WallColorSet(UINT16 uiRedGain ,UINT16 uiGreenGain, UINT16 uiBlueGain)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_ImageRGBGainSet(uiRedGain, uiGreenGain, uiBlueGain);

    return eResult;
}
#endif

eRESULT palFormatterMgr_ParameterSet(UINT8 ucPowerMode, UINT8 ucPowerModeNumber, UINT8 ucPictureMode, UINT8 ucColorTemperature, UINT8 ucColorWheelSpeed, UINT8 ucInputSource)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_ParameterSet(ucPowerMode, ucPowerModeNumber, ucPictureMode, ucColorTemperature, ucColorWheelSpeed, ucInputSource);

    return eResult;
}

eRESULT palFormatterMgr_CeilingMount_RearProjectSet(BOOL bCeilingMount, BOOL bRearProject)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_CeilingMount_RearProjectSet(bCeilingMount, bRearProject);

    //send to database
    if(eResult == rcSUCCESS)
    {
        UINT8 ucDB_Data = eOS_INVALID;

        if(bCeilingMount == 0 && bRearProject == 0)
        {
            ucDB_Data = eOS_TABLE_TOP;
        }
        else if(bCeilingMount == 1 && bRearProject == 0)
        {
            ucDB_Data = eOS_CEILING_MOUNT;
        }
        else if(bCeilingMount == 0 && bRearProject == 1)
        {
            ucDB_Data = eOS_REAR_PROJECTION;
        }
        else if(bCeilingMount == 1 && bRearProject == 1)
        {
            ucDB_Data = eOS_CEILING_REAR;
        }

        sFormatterInfo.ucOrientationState = ucDB_Data;

        //utilDatabase_WriteInformationData(eDI_ORIENTATION_STATE, DATA_TYPE_UI_DIGIT_8, 0, &ucDB_Data);
    }

    return eResult;
}

eRESULT palFormatterMgr_CeilingMount_RearProjectGet(UINT8 *pucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_CeilingMount_RearProjectGet(pucValue);

    return eResult;
}


eRESULT palFormatterMgr_COLOR_ENHANCEMENT_ValueGet(UINT8 ucDispalyMode, UINT8 ucCE, sHSG_SETTING *sHSG)
{
    eRESULT  eResult = rcSUCCESS;

    //ucDispalyMode = CM2GUI(edcPICTURE_SETTINGS, ucDispalyMode);

    if((INT8)ucDispalyMode < 0 )
    {
        ucDispalyMode = 0 ;
    }

#if 0
	if(palDataMgr_Model_ID_Get() == MODULE_TYPE_ID0_PLATFORM)
	{
		memcpy(sHSG, &sCE_TABLE_SETTING_W16[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}
	else
    {
    	memcpy(sHSG, &sCE_TABLE_SETTING_W20[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}
#endif /* 0 */

    if(ucDispalyMode < eCM_PICTURE_SETTINGS_NUMBER)
    {
        memcpy(sHSG, &sCE_TABLE_SETTING_DEFAULT[ucDispalyMode], sizeof(sHSG_SETTING));
    }

    return eResult;
}

eRESULT palFormatterMgr_CE_Add_HSG_Set(UINT8 ucDispalyMode, UINT8 ucCE, sHSG_SETTING sHSG)
{
    eRESULT  eResult = rcSUCCESS;
    sHSG_SETTING  sCE_HSG;

    //ucDispalyMode = CM2GUI(edcPICTURE_SETTINGS, ucDispalyMode);

    if((INT8)ucDispalyMode < 0 )
    {
        ucDispalyMode = 0 ;
    }

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): ucDispalyMode %d ,ucCE %d \r\n", __FUNCTION__, __LINE__,ucDispalyMode,ucCE);

#if 0
	if(palDataMgr_Model_ID_Get() == MODULE_TYPE_ID0_PLATFORM)	//G100_Clare_0015
	{
		memcpy(&sCE_HSG,&sCE_TABLE_SETTING_W16[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}
	else
	{
		memcpy(&sCE_HSG,&sCE_TABLE_SETTING_W20[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}
#endif /* 0 */

    if(ucDispalyMode < eCM_PICTURE_SETTINGS_NUMBER)
    {
        memcpy(&sCE_HSG, &sCE_TABLE_SETTING_DEFAULT[ucDispalyMode], sizeof(sHSG_SETTING));
    }

    //Red
    if(( (INT16)sHSG.HSG_R_HUE + (INT16)sCE_HSG.HSG_R_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_R_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_R_HUE + (INT16)sCE_HSG.HSG_R_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_R_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_R_HUE += sHSG.HSG_R_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_R_SAT + (INT16)sCE_HSG.HSG_R_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_R_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_R_SAT + (INT16)sCE_HSG.HSG_R_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_R_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_R_SAT += sHSG.HSG_R_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_R_GAIN + (INT16)sCE_HSG.HSG_R_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_R_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_R_GAIN + (INT16)sCE_HSG.HSG_R_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_R_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_R_GAIN += sHSG.HSG_R_GAIN - HSG_DEFAULT_VALUE;


    //Green
    if(( (INT16)sHSG.HSG_G_HUE + (INT16)sCE_HSG.HSG_G_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_G_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_G_HUE + (INT16)sCE_HSG.HSG_G_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_G_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_G_HUE += sHSG.HSG_G_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_G_SAT + (INT16)sCE_HSG.HSG_G_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_G_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_G_SAT + (INT16)sCE_HSG.HSG_G_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_G_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_G_SAT += sHSG.HSG_G_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_G_GAIN + (INT16)sCE_HSG.HSG_G_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_G_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_G_GAIN + (INT16)sCE_HSG.HSG_G_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_G_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_G_GAIN += sHSG.HSG_G_GAIN - HSG_DEFAULT_VALUE;


    //Blue
    if(( (INT16)sHSG.HSG_B_HUE + (INT16)sCE_HSG.HSG_B_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_B_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_B_HUE + (INT16)sCE_HSG.HSG_B_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_B_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_B_HUE += sHSG.HSG_B_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_B_SAT + (INT16)sCE_HSG.HSG_B_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_B_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_B_SAT + (INT16)sCE_HSG.HSG_B_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_B_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_B_SAT += sHSG.HSG_B_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_B_GAIN + (INT16)sCE_HSG.HSG_B_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_B_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_B_GAIN + (INT16)sCE_HSG.HSG_B_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_B_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_B_GAIN += sHSG.HSG_B_GAIN - HSG_DEFAULT_VALUE;


    //Cyan
    if(( (INT16)sHSG.HSG_C_HUE + (INT16)sCE_HSG.HSG_C_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_C_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_C_HUE + (INT16)sCE_HSG.HSG_C_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_C_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_C_HUE += sHSG.HSG_C_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_C_SAT + (INT16)sCE_HSG.HSG_C_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_C_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_C_SAT + (INT16)sCE_HSG.HSG_C_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_C_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_C_SAT += sHSG.HSG_C_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_C_GAIN + (INT16)sCE_HSG.HSG_C_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_C_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_C_GAIN + (INT16)sCE_HSG.HSG_C_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_C_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_C_GAIN += sHSG.HSG_C_GAIN - HSG_DEFAULT_VALUE;


    //Magenta
    if(( (INT16)sHSG.HSG_M_HUE + (INT16)sCE_HSG.HSG_M_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_M_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_M_HUE + (INT16)sCE_HSG.HSG_M_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_M_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_M_HUE += sHSG.HSG_M_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_M_SAT + (INT16)sCE_HSG.HSG_M_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_M_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_M_SAT + (INT16)sCE_HSG.HSG_M_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_M_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_M_SAT += sHSG.HSG_M_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_M_GAIN + (INT16)sCE_HSG.HSG_M_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_M_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_M_GAIN + (INT16)sCE_HSG.HSG_M_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_M_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_M_GAIN += sHSG.HSG_M_GAIN - HSG_DEFAULT_VALUE;


    //Yellow
    if(( (INT16)sHSG.HSG_Y_HUE + (INT16)sCE_HSG.HSG_Y_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_Y_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_Y_HUE + (INT16)sCE_HSG.HSG_Y_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_Y_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_Y_HUE += sHSG.HSG_Y_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_Y_SAT + (INT16)sCE_HSG.HSG_Y_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_Y_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_Y_SAT + (INT16)sCE_HSG.HSG_Y_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_Y_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_Y_SAT += sHSG.HSG_Y_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_Y_GAIN + (INT16)sCE_HSG.HSG_Y_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_Y_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_Y_GAIN + (INT16)sCE_HSG.HSG_Y_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_Y_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_Y_GAIN += sHSG.HSG_Y_GAIN - HSG_DEFAULT_VALUE;


    //White
    if(( (INT16)sHSG.HSG_W_R_GAIN + (INT16)sCE_HSG.HSG_W_R_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_W_R_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_W_R_GAIN + (INT16)sCE_HSG.HSG_W_R_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_W_R_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_W_R_GAIN += sHSG.HSG_W_R_GAIN - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_W_G_GAIN + (INT16)sCE_HSG.HSG_W_G_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_W_G_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_W_G_GAIN + (INT16)sCE_HSG.HSG_W_G_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_W_G_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_W_G_GAIN += sHSG.HSG_W_G_GAIN - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_W_B_GAIN + (INT16)sCE_HSG.HSG_W_B_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_W_B_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_W_B_GAIN + (INT16)sCE_HSG.HSG_W_B_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_W_B_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_W_B_GAIN += sHSG.HSG_W_B_GAIN - HSG_DEFAULT_VALUE;

    halFormatter_HSGValueSet(sCE_HSG);

    return eResult;
}



eRESULT palFormatterMgr_Upgrade_Set(UINT8 ucEnable)
{
    halFormatter_Upgrade_Set(ucEnable);

    return rcSUCCESS;
}


//3D Format:
// [0] : first frame
// [1] : FIELD GPIO
eRESULT palFormatterMgr_3D_FormatSet(UINT8 c3D_Format)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_3D_FormatSet(c3D_Format);

    return eResult;
}


eRESULT palFormatterMgr_3D_FormatGet(UINT8 *c3D_Format)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_3D_FormatGet(c3D_Format);

    return eResult;
}


eRESULT palFormatterMgr_ChannelSourceSet(UINT8 cSource)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_ChannelSourceSet(cSource);

    return eResult;
}


eRESULT palFormatterMgr_LightsOutTimerSet(UINT8 cLightOutTimer)
{
    eRESULT  eResult = rcINVALID;

    #if defined(CUSTOM_OPTOMA)
    cLightOutTimer = cLightOutTimer;
    #else
    cLightOutTimer = cLightOutTimer*2;
    #endif

    eResult = halFormatter_LightsOutTimerSet(cLightOutTimer);

    return eResult;
}

eRESULT palFormatterMgr_ABPINFOSet(UINT8 ucType, UINT8 *pucData)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_ABPINFOSet(ucType, pucData);

    return eResult;
}


eRESULT palFormatterMgr_ABPINFOGet(UINT8 *ucStatus)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_ABPINFOGet(ucStatus);

    return eResult;
}


eRESULT palFormatterMgr_IMG_AlgorithmEnableSet(UINT8* cAlg)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_IMG_AlgorithmEnableSet(cAlg);

    return eResult;
}


eRESULT palFormatterMgr_IMG_AlgorithmEnableGet(UINT8* cAlg)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_IMG_AlgorithmEnableGet(cAlg);

    return eResult;
}


eRESULT palFormatterMgr_Projection_Mode_Set(UINT8 ucProjection_Mode)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_Projection_Mode_Set(ucProjection_Mode);

    return eResult;
}

eRESULT palFormatterMgr_Projection_Mode_Get(UINT8 *ucProjection_Mode)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_Projection_Mode_Get(ucProjection_Mode);

    return eResult;
}

eRESULT palFormatterMgr_SlaveProjection_Mode_Get(UINT8 *ucProjection_Mode)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_SlaveProjection_Mode_Get(ucProjection_Mode);

    return eResult;
}

eRESULT palFormatterMgr_APCCT_Set(UINT8* cCCT)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_APCCT_Set(cCCT);

    return eResult;
}


eRESULT palFormatterMgr_APCCT_Get(UINT8* cCCT)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_APCCT_Get(cCCT);

    return eResult;
}

eRESULT palFormatterMgr_WAP_SetGet(UINT8 ucWap_Mode, eWAP_POWERLEVEL eWap_PowerLevel)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_WAP_SetGet(ucWap_Mode, eWap_PowerLevel);

    return eResult;
}

eRESULT palFormatterMgr_WAP_AllSet(eWAP_POWERLEVEL eWap_PowerLevel, UINT16 *ucPWM)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_WAP_AllSet((UINT8)eWap_PowerLevel, ucPWM);

    return eResult;
}

eRESULT palFormatterMgr_WAP_Set(UINT8 ucWap_Mode, eWAP_POWERLEVEL eWap_PowerLevel, UINT8 ucWap_LDSeq, UINT16 ucPWM)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_WAP_Set(ucWap_Mode, eWap_PowerLevel, ucWap_LDSeq, ucPWM);

    return eResult;
}

eRESULT palFormatterMgr_WAP_Get(UINT16* ucPWM)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_WAP_Get(ucPWM);

    return eResult;
}

eRESULT palFormatterMgr_DB_StrengthSet(UINT8 ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_DB_StrengthSet(ucValue);

    return eResult;
}


eRESULT palFormatterMgr_DB_StrengthGet(UINT8 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_DB_StrengthGet(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_FRC_BypassGet(UINT8 *ucEnable)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_FRC_BypassGet(ucEnable);

    return eResult;
}

UINT8 palFormatterMgr_FRC_Bypass_Flag_Get(void)
{
    return halFormatter_FRC_Bypass_Flag_Get();
}

eRESULT palFormatterMgr_BlankSignalSwitchSet(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_BlankSignalSwitchSet(ucEnable);

    return eResult;
}


eRESULT palFormatterMgr_BlankSignalSwitchGet(UINT8 *ucEnable)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_BlankSignalSwitchGet(ucEnable);

    return eResult;
}


eRESULT palFormatterMgr_AspectRatio_Set(UINT8* ucVAl)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_AspectRatio_Set(ucVAl);

    return eResult;
}


eRESULT palFormatterMgr_AspectRatio_Get(UINT8* ucVAl)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_AspectRatio_Get(ucVAl);

    return eResult;
}


eRESULT palFormatterMgr_Smooth_Enable_Set(BOOL bEnable, UINT8 ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_Smooth_Enable(bEnable, ucValue);

    return eResult;
}

eRESULT palFormatterMgr_Smooth_Enable_Get(UINT8* ucVAl)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_Smooth_Enable_Get(ucVAl);

    return eResult;
}


eRESULT palFormatterMgr_GetRLD_Light(UINT16 *RLD_Light)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_GetRLD_Light(RLD_Light);

    return eResult;
}

eRESULT palFormatterMgr_FRCByPassModeSet(BOOL ucVAl)
{
    eRESULT  eResult = rcINVALID;

#ifdef Low_Latency_All
    eResult = halFormatter_FRCByPassModeSet(ucVAl);
#endif /* Low_Latency_All */

    return eResult;
}


eRESULT palFormatterMgr_FRCByPassModeGet(UINT8* ucVAl)
{
    eRESULT  eResult = rcINVALID;

#ifdef Low_Latency_All
    eResult = halFormatter_FRCByPassModeGet(ucVAl);
#endif /* Low_Latency_All */

    return eResult;
}


eRESULT palFormatterMgr_LDFadeInStartupSet(UINT8 ucShutterStartup, UINT8 ucShutterFadeIn)			//G100_Doulas_0024
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_LDFadeInStartupSet(ucShutterStartup, ucShutterFadeIn);

    return eResult;
}


eRESULT palFormatterMgr_SplashAtStartupTimeoutSet(UINT8 ucTimeout)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_SplashAtStartupTimeoutSet(ucTimeout);

    return eResult;
}


//H2 wait review
eRESULT palFormatterMgr_PowerModeTable(UINT8 ucPowerMode)
{
}


eORIENTATION_STATE palFormatterMgr_OrientationStateGet(void)
{
    return sFormatterInfo.ucOrientationState;
}

eRESULT palFormatterMgr_FRC_BypassSet(UINT8 ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_FRC_BypassSet(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Set(ucEnable);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Filter_Set(UINT8 ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Filter_Set(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Filter_Get(UINT8 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Filter_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Close_Loop_Enable_Axis0_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Close_Loop_Enable_Axis0_Set(ucEnable);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Close_Loop_Enable_Axis0_Get(UINT8 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Close_Loop_Enable_Axis0_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Close_Loop_Enable_Axis1_Set(UINT8 ucDatax)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Close_Loop_Enable_Axis1_Set(ucDatax);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Close_Loop_Enable_Axis1_Get(UINT8 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Close_Loop_Enable_Axis1_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Gain_of_Axis0_Get(UINT32 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Gain_of_Axis0_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Gain_of_Axis1_Get(UINT32 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Gain_of_Axis1_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Segment_Length_of_Axis0_Get(UINT32 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Segment_Length_of_Axis0_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Segment_Length_of_Axis1_Get(UINT32 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Segment_Length_of_Axis1_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_SN_Get(UINT8 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_SN_Get(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_EepromTargetAdcAxis0_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_TargerAdc_Axis0_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_EepromTargetAdcAxis1_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_TargerAdc_Axis1_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Eeprom_ZData0_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_ZData0_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Eeprom_ZData1_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_ZData1_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Eeprom_XData0_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_XData0_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Eeprom_XData1_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_XData1_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Eeprom_YData0_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_YData0_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Eeprom_YData1_Get(UINT32 *ulValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Eeprom_YData1_Get(ulValue);

    return eResult;
}

eRESULT palFormatterMgr_AutoLockResync(void)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_AutoLockResync();

    return eResult;
}

eRESULT palFormatterMgr_GetI2C_Diag(UINT8 *pdata)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_GetI2C_Diag(pdata);

    return eResult;
}

eRESULT palFormatterMgr_XPR_CloseLoop_ErrorCode0(UINT32 *ucValue) //HICC2_Steven_0051 start
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_CloseLoop_ErrorCode0(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_CloseLoop_ErrorCode1(UINT32 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_CloseLoop_ErrorCode1(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_CloseLoop_Valid(UINT8 *ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_CloseLoop_Valid(ucValue);

    return eResult;
}

eRESULT palFormatterMgr_XPR_Close_Loop_Error_Reset_Set(UINT8 ucValue)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Close_Loop_Error_Reset_Set(ucValue);

    return eResult;
} //HICC2_Steven_0051 end



eRESULT palFormatterMgr_XPR_Gain_Write_EEPROM(void)
{
    eRESULT  eResult = rcINVALID;

    eResult = halFormatter_XPR_Gain_Write_EEPROM();

    return eResult;
}
