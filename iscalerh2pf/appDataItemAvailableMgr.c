#include <stddef.h>

#include "appDataMgr.h"
#include "appDataItemAvailableMgr.h"
#include "appLedProcAPI.h"
#include "appIllumination.h"
#include "appEnvironment.h"
#include "appSystem.h"
#include "appDataPath.h"
#include "appCLICmd.h"
#include "appLANProcAPI.h"
#include "appInputProc.h"

#include "palGui.h"
#include "palMotorMgr.h"
#include "palFrontEndMgr.h"
#include "palFormatterMgr.h"
#include "palImgMgr.h"
#include "palGeoAPI.h"

#include "halFormatter.h" //OE CPC
#include "halFanCtrlAPI.h"
#include "palLDMgr.h"
#include "halBoardCtrlAPI.h"
#include "halMotorCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "halFrontEndCtrlAPI.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"         //A70LV_Doulas_0154
#endif
#include "halScaler.h"

#include "utilDataMgrAPI.h"
#include "utilDbgMsg.h"
#include "utilDatabaseAPI.h"
#include "utilDataMapping.h"
#include "utilCLICmdAPI.h"
#include "utilCounterAPI.h"
#include "utilMisc.h"
#include "utilCommon.h"
#include "utilHostAPI.h"
#include "PNGUtility.h"

#include "Release.h"
#include "cmd_ap.h"				//G100_Doulas_0032
#include "GEC_EventTable.h"
#include "GEC_CoreFunction.h" //A70LK_Casper_0007 //G50_Casper_0006

#include <sys/time.h>  //G100_Wilsonj_0068


#include "utilDataMapping.h"      //H2 wait review
#include "ProjectSettings.h"

#include "utilBIST.h"  //HICC2_Steven_0008

sAFN_INFO sAFNInfo;

typedef struct
{
    UINT16 eDI_ITEM;
    eEXEC_CODE (*DataMgr_Access)(eDATA_ACCESS_MODE eAccessMode, void *pValue);

} sDATA_ITEM_AVAILABLE_LUT_TABLE;

eEXEC_CODE palDataItemAvailable_Null(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    return eEXEC_CODE_FAIL;
}

eEXEC_CODE palDataItem_Access_AFN_MainInput(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_MainInput;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        UINT8 ucPIPEn = palImgMgr_PIP_PBP_Enable_Get();

        NewVal = 0xFFFF;

        if(ucPIPEn)
        {
            UINT8 ucSubSource = 0;
            UINT8 ucCount = eCM_SOURCE_NUMBER - 1;

            BITS_SET_TRUE(NewVal, ucCount);

            palDataMgr_Data_Access(edcSUB_INPUT, edaREAD, &ucSubSource);

            for(ucCount = 0; ucCount < eCM_SOURCE_NUMBER; ucCount++)
            {
                if(ucSubSource == ucCount)
                {
                    BIT_CLEAR(NewVal, ucCount);
                    break;
                }
            }
        }

        sAFNInfo.AFN_MainInput = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataItem_Access_AFN_SubInput(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_SubInput;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        UINT8 ucPIPEn = palImgMgr_PIP_PBP_Enable_Get();

        if(ucPIPEn)
        {
            UINT8 ucMainSource = 0;
            UINT8 ucCount = eCM_SOURCE_NUMBER - 1;

            BITS_SET_TRUE(NewVal, ucCount);

            palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucMainSource);

            for(ucCount = 0; ucCount < eCM_SOURCE_NUMBER; ucCount++)
            {
                if(ucMainSource == ucCount)
                {
                    BIT_CLEAR(NewVal, ucCount);
                    break;
                }
            }
        }
        else
        {
            NewVal = 0;
        }

        sAFNInfo.AFN_SubInput = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

//Available function for edcSIZE_PRESETS
eEXEC_CODE palDataItem_Access_AFN_SizePresets(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_SizePresets;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcSIZE_PRESETS);

        UINT8 uc3D_Enable;
        uc3D_Enable = palDataMgr_IS_3D_Enable();

        for(UINT32 index = 0; index < eCM_SCALING_MODE_NUMBER; index++)
        {
            if(uc3D_Enable == ets_ON)  //3D on
            {
                if((index == eCM_SCALING_MODE_3D) && (eEnable == eFUNC_CONTROL_ENABLE))
                {
                    BIT_SET(NewVal, eCM_SCALING_MODE_3D);
                }
                else
                {
                    BIT_CLEAR(NewVal, index);
                }
            }
            else
            {
                if((index != eCM_SCALING_MODE_3D) && (eEnable == eFUNC_CONTROL_ENABLE))
                {
                    //psMenuInfo->ppsMenuItemList[iCount]->ItemHide = 0;
                    BIT_SET(NewVal, index);
                }
                else
                {
                    //psMenuInfo->ppsMenuItemList[iCount]->ItemHide = 1;
                    BIT_CLEAR(NewVal, index);
                }
            }
        }

        sAFNInfo.AFN_SizePresets = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}


//Available function for edcPICTURE_SETTINGS
eEXEC_CODE palDataItem_Access_AFN_PictureSettings(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_PictureSetttings;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcPICTURE_SETTINGS);

        UINT8 PicSettings;
        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &PicSettings);

        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            switch(PicSettings)
            {
                case eCM_PICTURE_SETTINGS_HDR:
                case eCM_PICTURE_SETTINGS_HLG: //HICC2_AC_0015
                case eCM_PICTURE_SETTINGS_3D:
                case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                    BIT_SET(NewVal, PicSettings);
                    if(PicSettings == eCM_PICTURE_SETTINGS_3D_PASSIVE)
                    {
                        BIT_SET(NewVal, eCM_PICTURE_SETTINGS_3D);
                    }

                #ifdef ENABLE_SINGLE_USER_MODE              //H30K_Tim_0014, add, ***
                    //grayout user mode.
                #else //ENABLE_SINGLE_USER_MODE             //H30K_Tim_0014, add, &&&
                    BIT_SET(NewVal, eCM_PICTURE_SETTINGS_USER); //HICC2_Casper_0017
                #endif //ENABLE_SINGLE_USER_MODE            //H30K_Tim_0014, add

                    break;

            #ifdef ENABLE_SINGLE_USER_MODE                  //H30K_Tim_0014, add, ***

                // No User-XXX

            #else //ENABLE_SINGLE_USER_MODE                 //H30K_Tim_0014, add, &&&

                case eCM_PICTURE_SETTINGS_USER:
                    {
                        UINT8 UserMode;
                        palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaREAD, &UserMode);
                        switch(UserMode)
                        {
                            case eCM_PICTURE_SETTINGS_HDR:
                            case eCM_PICTURE_SETTINGS_HLG: //HICC2_AC_0015
                            case eCM_PICTURE_SETTINGS_3D:
                            case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                            case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                                BIT_SET(NewVal, UserMode);
                                if(UserMode == eCM_PICTURE_SETTINGS_3D_PASSIVE)
                                {
                                    BIT_SET(NewVal, eCM_PICTURE_SETTINGS_3D);
                                }
                                BIT_SET(NewVal, eCM_PICTURE_SETTINGS_USER); //HICC2_Casper_0017
                                break;

                            default:
                                for(int i=0; i<eCM_PICTURE_SETTINGS_NUMBER; i++)
                                {
                                    switch(i)
                                    {
                                        case eCM_PICTURE_SETTINGS_HDR:
                                        case eCM_PICTURE_SETTINGS_HLG: //HICC2_AC_0015
                                        case eCM_PICTURE_SETTINGS_3D:
                                        case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                                        case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                                            BIT_CLEAR(NewVal, i);
                                            break;

                                        default:
                                            BIT_SET(NewVal, i);
                                            break;
                                    }
                                }
                                break;
                        }
                    }
                    break;

            #endif //ENABLE_SINGLE_USER_MODE                //H30K_Tim_0014, add

                default:
                    for(int i=0; i<eCM_PICTURE_SETTINGS_NUMBER; i++)
                    {
                        switch(i)
                        {
                            case eCM_PICTURE_SETTINGS_HDR:
                            case eCM_PICTURE_SETTINGS_HLG: //HICC2_AC_0015
                            case eCM_PICTURE_SETTINGS_3D:
                            case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                            case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                                BIT_CLEAR(NewVal, i);
                                break;

                            default:
                                BIT_SET(NewVal, i);
                                break;
                        }
                    }
                    break;
            }
        }
        else
        {
            NewVal = 0;
        }

        sAFNInfo.AFN_PictureSetttings = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}


eEXEC_CODE palDataItem_Access_AFN_WarpMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_WarpMemoryApply;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcWARP_MEMORY_APPLY);

        for(UINT32 index=eCM_GEO_MEMORY_APPLY_OFF; index<eCM_GEO_MEMORY_APPLY_NUMBER; index++)
        {
            if(eEnable == eFUNC_CONTROL_ENABLE)
            {
                if(index == eCM_GEO_MEMORY_APPLY_OFF)
                {
                    BIT_SET(NewVal, eCM_GEO_MEMORY_APPLY_OFF);
                    continue;
                }
                else
                {
                    if(palDataMgr_ApplyWarpMemoryItemAvailable(index) == 0)
                    {
                        BIT_CLEAR(NewVal, index);
                    }
                    else
                    {
                        BIT_SET(NewVal, index);
                    }
                }
            }
            else
            {
                BIT_CLEAR(NewVal, index);
            }
        }

        sAFNInfo.AFN_WarpMemoryApply = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}


eEXEC_CODE palDataItem_Access_AFN_BlendMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_BlendMemoryApply;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcBLEND_MEMORY_APPLY);

        for(UINT32 index=eCM_GEO_MEMORY_APPLY_OFF; index<eCM_GEO_MEMORY_APPLY_NUMBER; index++)
        {
            if(eEnable == eFUNC_CONTROL_ENABLE)
            {
                if(index == eCM_GEO_MEMORY_APPLY_OFF)
                {
                    BIT_SET(NewVal, eCM_GEO_MEMORY_APPLY_OFF);
                    continue;
                }
                else
                {
                    if(palDataMgr_ApplyBlendMemoryItemAvailable(index) == 0)
                    {
                        BIT_CLEAR(NewVal, index);
                    }
                    else
                    {
                        BIT_SET(NewVal, index);
                    }
                }
            }
            else
            {
                BIT_CLEAR(NewVal, index);
            }
        }

        sAFNInfo.AFN_BlendMemoryApply = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}


eEXEC_CODE palDataItem_Access_AFN_LensMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_LensMemoryApply;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcLENS_APPLY_POSITION);

        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            NewVal = palEnvironment_LensMemorySavingCondition_Get();
        }
        else
        {
            NewVal = 0 ;
        }

        sAFNInfo.AFN_LensMemoryApply = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

//Available function for edcBACKUP_RESTORE_RESTORE
eEXEC_CODE palDataItem_Access_AFN_Backup_Restore_Restore(eDATA_ACCESS_MODE eAccessMode, void *pValue) //HICC2_Doulas_0039
{
    UINT32 CurVal = sAFNInfo.AFN_BackupRestoreRestore;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcBACKUP_RESTORE_RESTORE);

        for(UINT32 index=BACKUP_RESTORE_MIN_VALUE; index<=BACKUP_RESTORE_MAX_VALUE; index++)
        {
            if(utilDataMgr_UserDataLoadItemAvailable_CM(index) == TRUE)
            {
                //psMenuInfo->ppsMenuItemList[iCount]->ItemHide = 0;
                BIT_SET(NewVal, index);
            }
            else
            {
                //psMenuInfo->ppsMenuItemList[iCount]->ItemHide = 1;
                BIT_CLEAR(NewVal, index);
            }
        }

        sAFNInfo.AFN_BackupRestoreRestore = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataItem_Access_AFN_UserMode(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_UserMode;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcUSER_COLOR_MODE);

        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            UINT8 PicSettings;
            palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaREAD, &PicSettings);

            switch(PicSettings)
            {
                case eCM_PICTURE_SETTINGS_HDR:
                case eCM_PICTURE_SETTINGS_HLG:
                case eCM_PICTURE_SETTINGS_3D:
                case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                    for(UINT8 ucIndex = 0; ucIndex < eCM_PICTURE_SETTINGS_NUMBER; ucIndex++)
                    {
                        if(PicSettings == eCM_PICTURE_SETTINGS_HDR && ucIndex == eCM_PICTURE_SETTINGS_HDR)
                        {
                            BIT_SET(NewVal, ucIndex);
                        }
						else if(PicSettings == eCM_PICTURE_SETTINGS_HLG && ucIndex == eCM_PICTURE_SETTINGS_HLG)
                        {
                            BIT_SET(NewVal, ucIndex);
                        }
                        else if(PicSettings == eCM_PICTURE_SETTINGS_3D && ucIndex == eCM_PICTURE_SETTINGS_3D)
                        {
                            BIT_SET(NewVal, ucIndex);
                        }
                        else if(PicSettings == eCM_PICTURE_SETTINGS_3D_PASSIVE && ucIndex == eCM_PICTURE_SETTINGS_3D_PASSIVE)
                        {
                            BIT_SET(NewVal, ucIndex);
                        }
                        else if(PicSettings == eCM_PICTURE_SETTINGS_2DHIGHSPEED && ucIndex == eCM_PICTURE_SETTINGS_2DHIGHSPEED)
                        {
                            BIT_SET(NewVal, ucIndex);
                        }
                        else
                        {
                            BIT_CLEAR(NewVal, ucIndex);
                        }
                    }
                    break;

                default:
                    for(int i=0; i<eCM_PICTURE_SETTINGS_NUMBER; i++)
                    {
                        switch(i)
                        {
                            case eCM_PICTURE_SETTINGS_HDR:
                			case eCM_PICTURE_SETTINGS_HLG:
                            case eCM_PICTURE_SETTINGS_3D:
                            case eCM_PICTURE_SETTINGS_2DHIGHSPEED:
                            case eCM_PICTURE_SETTINGS_3D_PASSIVE:
                                BIT_CLEAR(NewVal, i);
                                break;

                            default:
                                BIT_SET(NewVal, i);
                                break;
                        }
                    }
                    break;
            }
        }
        else
        {
            NewVal = 0;
        }

        sAFNInfo.AFN_UserMode = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataItem_Access_AFN_Gamma(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_Gamma;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcGAMMA);

        UINT8 PicSettings;
        UINT8 PicUserMode;

        palDataMgr_Data_Access(edcPICTURE_SETTINGS, edaREAD, &PicSettings);
        palDataMgr_Data_Access(edcUSER_COLOR_MODE, edaREAD, &PicUserMode);


        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            UINT8 ucGamma;
            palDataMgr_Data_Access(edcGAMMA, edaREAD, &ucGamma);

            if((PicSettings == eCM_PICTURE_SETTINGS_HDR) ||
               (PicSettings == eCM_PICTURE_SETTINGS_USER && PicUserMode == eCM_PICTURE_SETTINGS_HDR) ||
               (PicSettings == eCM_PICTURE_SETTINGS_HLG) ||
               (PicSettings == eCM_PICTURE_SETTINGS_USER && PicUserMode == eCM_PICTURE_SETTINGS_HLG) )
            {
                for(UINT8 ucIndex = 0; ucIndex < eCM_GAMMA_NUMBER; ucIndex++)
                {
                    if(ucIndex >= eCM_GAMMA_HDR_STANDARD && ucIndex <= eCM_GAMMA_HLG_Detail_HC)
                    {
                        BIT_SET(NewVal, ucIndex);
                    }
                    else
                    {
                        BIT_CLEAR(NewVal, ucIndex);
                    }
                }
            }
            else
            {
                for(UINT8 ucIndex = 0; ucIndex < eCM_GAMMA_NUMBER; ucIndex++)
                {
#if defined(PLATFORM_H30_4K)
					if(ucIndex != eCM_GAMMA_HDR_STANDARD && ucIndex != eCM_GAMMA_3D)
                    {
                        BIT_SET(NewVal, ucIndex);
                    }
                    else
                    {
                        BIT_CLEAR(NewVal, ucIndex);
                    }
#else
                    if(ucIndex < eCM_GAMMA_HDR_STANDARD)
                    {
                        BIT_SET(NewVal, ucIndex);
                    }
                    else
                    {
                        BIT_CLEAR(NewVal, ucIndex);
                    }
#endif
                }
            }
        }
        else
        {
            NewVal = 0;
        }

        sAFNInfo.AFN_Gamma = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataItem_Access_AFN_MainColorSpace(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_MainColorSpace;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
#ifdef SCALER_FPGA_F34
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcCOLOR_SPACE);

        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            UINT8 ucCount = eCM_COLOR_SPACE_NUMBER - 1;

            BITS_SET_TRUE(NewVal, ucCount);

            if(halScaler_PixModIsYUV420(eSOURCE_WINDOW_MAIN, halScaler_InputSource_Get(eSOURCE_WINDOW_MAIN)) == TRUE)
            {
                BIT_CLEAR(NewVal, eCM_COLOR_SPACE_RGB_FULL);
                BIT_CLEAR(NewVal, eCM_COLOR_SPACE_RGB_LIMITED);
            }
        }
        else
        {
            NewVal = 0;
        }
#else
        NewVal = 0x1F; // bit0~4 = 1
#endif /* SCALER_FPGA_F34 */

        sAFNInfo.AFN_MainColorSpace = NewVal;
        *((UINT32*)pValue) = NewVal;
    }


    return eEXEC_CODE_PASS;
}

eEXEC_CODE palDataItem_Access_AFN_SubColorSpace(eDATA_ACCESS_MODE eAccessMode, void *pValue)
{
    UINT32 CurVal = sAFNInfo.AFN_SubColorSpace;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
#ifdef SCALER_FPGA_F34
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edcSUB_IMAGE_COLOR_SPACE);

        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            UINT8 ucCount = eCM_COLOR_SPACE_NUMBER - 1;

            BITS_SET_TRUE(NewVal, ucCount);

            if(halScaler_PixModIsYUV420(eSOURCE_WINDOW_SUB, halScaler_InputSource_Get(eSOURCE_WINDOW_SUB)) == TRUE)
            {
                BIT_CLEAR(NewVal, eCM_COLOR_SPACE_RGB_FULL);
                BIT_CLEAR(NewVal, eCM_COLOR_SPACE_RGB_LIMITED);
            }
        }
        else
        {
            NewVal = 0;
        }
#else
        NewVal = 0x1F; // bit0~4 = 1
#endif /* SCALER_FPGA_F34 */

        sAFNInfo.AFN_SubColorSpace = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}

//Available function for edc3D_ENABLE
eEXEC_CODE palDataItem_Access_AFN_3D_Enable(eDATA_ACCESS_MODE eAccessMode, void *pValue) //H30K_Doulas_0068
{
    UINT32 CurVal = sAFNInfo.AFN_3DEnable;
    UINT32 NewVal = *((UINT32*)pValue);

    if(edaREAD == eAccessMode)
    {
        *((UINT32*)pValue) = CurVal;
    }
    else
    {
        NewVal = 0;
        eFUNC_CONTROL eEnable = palDataMgr_DataCode_Control(edc3D_ENABLE);

        if(eEnable == eFUNC_CONTROL_ENABLE)
        {
            UINT8 ucCount = eCM_3D_FORMAT_NUMBER - 1;
            UINT16 uiV_Freq;
            #ifdef SCALER_FPGA_F34
            UINT32 ucSource = 0;
            UINT32 ucGeometryEnable = 0;
            UINT8 uc3DMode = 0;
            //UINT16 uiItemCondition = 0xFFFF;
            #endif

            BITS_SET_TRUE(NewVal, ucCount);
            halScaler_InputVertRefresh2_Get(eSOURCE_WINDOW_MAIN,&uiV_Freq);

            #ifdef SCALER_FPGA_F34
            palDataMgr_Data_Access(edcMAIN_INPUT, edaREAD, &ucSource);
            palDataMgr_Data_Access(edcWARP_TOGGLE, edaREAD, &ucGeometryEnable);
            palDataMgr_Data_Access(edc3D_MODE, edaREAD, &uc3DMode);

            ucSource = CM2GUI(edcMAIN_INPUT, ucSource);
            ucGeometryEnable = CM2GUI(edcWARP_TOGGLE, ucGeometryEnable);

            if(ucSource != eGUI_SOURCE_ID_HDMI1 && ucSource != eGUI_SOURCE_ID_HDMI2) //Dual Pipe, 4K3D Dual Pipe gray out
            {
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_DUALPIPE_3D);
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_4K3D_DUALPIPE);
            }

            if(ucGeometryEnable == 0 || uc3DMode == eCM_3D_MODE_ON) //4K3D,4K3D Dual Pipe gray out
            {
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_4K3D);
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_4K3D_DUALPIPE);
            }
            #endif

            if((palImgMgr_Input_H_Active_Get(eSOURCE_WINDOW_MAIN) > 3800) &&
                (palImgMgr_Input_V_Active_Get(eSOURCE_WINDOW_MAIN) > 2150)) //4K input
            {
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_FRAME_PACKING);
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_SIDE_BY_SIDE);
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_TOP_AND_BOTTOM);
            }

            if(uiV_Freq > 9800) //98hz
            {
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_FRAME_PACKING);
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_SIDE_BY_SIDE);
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_TOP_AND_BOTTOM);
            }

            if(uiV_Freq > 19800) //198hz
            {
                BIT_CLEAR(NewVal, eCM_3D_FORMAT_FRAME_SEQUENTIAL);
            }

        }
        else
        {
            NewVal = 0;
        }

        sAFNInfo.AFN_3DEnable = NewVal;
        *((UINT32*)pValue) = NewVal;
    }

    return eEXEC_CODE_PASS;
}



sDATA_ITEM_AVAILABLE_LUT_TABLE m_sDataItem_Available_LutTable[] =
{
/*0000*/  {eDI_POWER_STATUS,                    palDataItemAvailable_Null},
/*0001*/  {eDI_WARMUP_COUNT,                    palDataItemAvailable_Null},
/*0002*/  {eDI_PIN_STATUS,                      palDataItemAvailable_Null},
/*0003*/  {eDI_SERVICE_MODE_STATUS,             palDataItemAvailable_Null},
/*0004*/  {eDI_PANEL_H_RESOLUTION,              palDataItemAvailable_Null},
/*0005*/  {eDI_PANEL_V_RESOLUTION,              palDataItemAvailable_Null},
/*0006*/  {eDI_3D_ENABLE,                       palDataItemAvailable_Null},
/*0007*/  {eDI_UPGRADE_STATUS,                  palDataItemAvailable_Null},
/*0008*/  {eDI_UPGRADE_PERCENTAGE,              palDataItemAvailable_Null},
/*0009*/  {eDI_ORIENTATION_STATE,               palDataItemAvailable_Null},
/*0010*/  {eDI_LENS_MEMORY_SAVE_CONDITION,      palDataItemAvailable_Null},
/*0011*/  {eDI_WARPING_MEMORY_SAVE_CONDITION,   palDataItemAvailable_Null},
/*0012*/  {eDI_BLENDING_MEMORY_SAVE_CONDITION,  palDataItemAvailable_Null},
/*0013*/  {eDI_SCREEN_CAPTURE_STATE,            palDataItemAvailable_Null},
/*0014*/  {eDI_DUMP_REG_STATE,                  palDataItemAvailable_Null},
/*0015*/  {eDI_WARP_MEMORY1_FLAG,               palDataItemAvailable_Null},
/*0016*/  {eDI_WARP_MEMORY2_FLAG,               palDataItemAvailable_Null},
/*0017*/  {eDI_WARP_MEMORY3_FLAG,               palDataItemAvailable_Null},
/*0018*/  {eDI_WARP_MEMORY4_FLAG,               palDataItemAvailable_Null},
/*0019*/  {eDI_WARP_MEMORY5_FLAG,               palDataItemAvailable_Null},
/*0020*/  {eDI_BLEND_MEMORY1_FLAG,              palDataItemAvailable_Null},
/*0021*/  {eDI_BLEND_MEMORY2_FLAG,              palDataItemAvailable_Null},
/*0022*/  {eDI_BLEND_MEMORY3_FLAG,              palDataItemAvailable_Null},
/*0023*/  {eDI_BLEND_MEMORY4_FLAG,              palDataItemAvailable_Null},
/*0024*/  {eDI_BLEND_MEMORY5_FLAG,              palDataItemAvailable_Null},
/*0025*/  {eDI_GEOMETRY_CMD_ACK_DONE,           palDataItemAvailable_Null},
/*0026*/  {eDI_BLENDING_DATA_SYNC_STATUS,       palDataItemAvailable_Null},
/*0027*/  {eDI_3D_MODE_CONDITION,               palDataItem_Access_AFN_3D_Enable}, //H30K_Doulas_0068
/*0028*/  {eDI_USER_DATA_CONDITION,             palDataItemAvailable_Null},
/*0029*/  {eDI_PICTURE_MODE_CONDITION,          palDataItemAvailable_Null},
/*0030*/  {eDI_MAIN_INPUT_CONDITION,            palDataItem_Access_AFN_MainInput},
/*0031*/  {eDI_SUB_INPUT_CONDITION,             palDataItem_Access_AFN_SubInput},
/*0032*/  {eDI_BLEND_MEMORY_APPLY,              palDataItem_Access_AFN_BlendMemoryApply},
/*0033*/  {eDI_SIZE_PRESETS,                    palDataItem_Access_AFN_SizePresets},
/*0034*/  {eDI_PICTURE_SETTINGS,                palDataItem_Access_AFN_PictureSettings},
/*0035*/  {eDI_WARP_MEMORY_APPLY,               palDataItem_Access_AFN_WarpMemoryApply},
/*0036*/  {eDI_LENS_APPLY_POSITION,             palDataItem_Access_AFN_LensMemoryApply},
/*0037*/  {eDI_BACKUP_RESTORE_RESTORE,          palDataItem_Access_AFN_Backup_Restore_Restore},
/*0038*/  {eDI_USER_MODE_SETTINGS,              palDataItem_Access_AFN_UserMode},
/*0039*/  {eDI_GAMMA_SETTINGS,                  palDataItem_Access_AFN_Gamma},
/*0040*/  {eDI_MAIN_COLOR_SPACE,                palDataItem_Access_AFN_MainColorSpace},
/*0041*/  {eDI_SUB_COLOR_SPACE,                 palDataItem_Access_AFN_SubColorSpace},
/*0042*/  {eDI_HDR_GAMMA_FUNCTION,              palDataItemAvailable_Null},
/*0043*/  {eDI_DUST_TIME_SELECT,                palDataItemAvailable_Null},
/*0044*/  {eDI_FOG_TIME_SELECT,                 palDataItemAvailable_Null},
/*0045*/  {eDI_BACKUPINPUT_PRIMARY_INPUT,       palDataItemAvailable_Null},
/*0046*/  {eDI_BACKUPINPUT_SECONDARY_INPUT,     palDataItemAvailable_Null}
};

#define DATAITEM_AVAILABLE_TABLE_SIZE sizeof(m_sDataItem_Available_LutTable)/sizeof(m_sDataItem_Available_LutTable[0])   //HICC2_Simon_0003

VERIFY_SIZE_OF(m_sDataItem_Available_LutTable, sizeof(m_sDataItem_Available_LutTable[0])*sDI_INVALID);


void palDataItemAvailable_Init(void)  //HICC2_Simon_0003
{
    UINT16 uiCount = 0;
    UINT32 InitVal = 0;
    size_t table_size = DATAITEM_AVAILABLE_TABLE_SIZE;

    for(uiCount = 0; uiCount < sDI_INVALID; uiCount++)
    {
        if(uiCount >= table_size)
        {
            printf("!!! Dataitem Size Error [PF %zu] [LIB %d]\r\n", table_size, sDI_INVALID);
            break;
        }

        utilDatabase_WriteInformationData(uiCount, DATA_TYPE_UI_DIGIT_32, 0, (void*)&InitVal);
    }
}

void palDataItemAvailable_Poll(void)
{
    UINT16 uiCount = 0;
    UINT32 CurVal = 0, NewVal = 0;
    size_t table_size = DATAITEM_AVAILABLE_TABLE_SIZE;  //HICC2_Simon_0003

    for(uiCount = 0; uiCount < sDI_INVALID; uiCount++)
    {
        if(uiCount >= table_size)  //HICC2_Simon_0003
        {
            printf("!!! Dataitem Size Error [PF %zu] [LIB %d]\r\n", table_size, sDI_INVALID);
            break;
        }

        CurVal = 0;
        if(eEXEC_CODE_PASS == m_sDataItem_Available_LutTable[uiCount].DataMgr_Access(edaREAD, (void*)&CurVal))
        {
            m_sDataItem_Available_LutTable[uiCount].DataMgr_Access(edaWRITE_RAM_ONLY_NO_ACTION, (void*)&NewVal);

            if(CurVal != NewVal)
            {
                //printf("(%s, %d) count = %d, NewVal = %d\n", __FUNCTION__, __LINE__, uiCount, NewVal);
                utilDatabase_WriteInformationData(uiCount, DATA_TYPE_UI_DIGIT_32, 0, (void*)&NewVal);
            }
        }
    }
}

eEXEC_CODE palDataItemAvailable_AccessRead(eDATABASE_INFOMATION_INDEX eDataCode, void *pValue)
{
    eEXEC_CODE eResult = eEXEC_CODE_PASS;
    UINT16 uiCount = 0;
    UINT32 CurVal = 0, NewVal = 0;

    if(eDataCode < sDI_INVALID)
    {
        if(m_sDataItem_Available_LutTable[eDataCode].DataMgr_Access != NULL)
        {
            eResult = m_sDataItem_Available_LutTable[eDataCode].DataMgr_Access(edaREAD, pValue);
        }
    }

    return eResult;
}


