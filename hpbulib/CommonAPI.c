#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include "SharedMemCtrl.h"
#include "cmd_ap.h"
#include "CommonAPI.h"
#include "DataMapping.h"
#include "ItemConfigAccess.h"
#include "hicc_config.h"
#include "opdCtrlAPI.h"


INT32 CommonAPI_DataCode_MaxValue_Get(eDATA_CODE eDataCode)
{
    sDATACODE_INFO sDataCodeInfo;
    if(SharedMem_ReadData_New(eSB_DATACODE_INFO, &sDataCodeInfo, (unsigned int)eDataCode) != 1)
    {
        LOG_TO_SYSLOG("Error: %s access shared memory fail\n", __FUNCTION__);
    }

    return sDataCodeInfo.Max;
}

INT32 CommonAPI_DataCode_MinValue_Get(eDATA_CODE eDataCode)
{
    sDATACODE_INFO sDataCodeInfo;
    if(SharedMem_ReadData_New(eSB_DATACODE_INFO, &sDataCodeInfo, (unsigned int)eDataCode) != 1)
    {
        LOG_TO_SYSLOG("Error: %s access shared memory fail\n", __FUNCTION__);
    }

    return sDataCodeInfo.Min;
}

UINT8 CommonAPI_DataCode_IsSupportAckdone_Get(eDATA_CODE eDataCode)
{
    sDATACODE_INFO sDataCodeInfo;
    if(SharedMem_ReadData_New(eSB_DATACODE_INFO, &sDataCodeInfo, (unsigned int)eDataCode) != 1)
    {
        LOG_TO_SYSLOG("Error: %s access shared memory fail\n", __FUNCTION__);
    }

    return sDataCodeInfo.IsSupportAckdone;
}


INT32 CommonAPI_DataCode_DataSize_Get(eDATA_CODE eDataCode)
{
    if(eDataCode >= edcINVALID)
        return 0;

    sDATABASE_ITEM_DATA_FORMAT sDatabaseItemDataShm;
    if(SharedMem_ReadData_New(eSB_DATABASE_ITEM, &sDatabaseItemDataShm, (UINT32)eDataCode) != 1)
    {
        LOG_TO_SYSLOG("Error: %s access shared memory fail\n", __FUNCTION__);
    }

    return (UINT16)sDatabaseItemDataShm.uiValueSize;
}


INT32 CommonAPI_ValueConvert(INT32 iValue, INT32 iValueMax, INT32 iValueMin, INT32 iTransMax, INT32 iTransMin, INT32 *TransValue)
{
	INT32 iValueScale = iValueMax - iValueMin;
	INT32 iTransScale = iTransMax - iTransMin;
	INT64 iTemp = 0;
    INT16 uiRatio = 1000;

    if(iValueScale > 0x7FFF || iTransScale > 0x7FFF)
        uiRatio = 1;

	iTemp = (((INT64)iValue - iValueMin) * iTransScale * uiRatio / iValueScale) + (iTransMin*uiRatio);

	if(iTemp > 0)
		iTemp = ((((iTemp) % (uiRatio)) && (iValueScale < iTransScale)) ? (((iTemp)/(uiRatio)) + 1) : ((iTemp)/(uiRatio)));
	else
		iTemp = ((iTemp) % (uiRatio) ? (((iTemp)/(uiRatio)) - 1) : ((iTemp)/(uiRatio)));

	*TransValue = (INT32)(((iTemp) > (iTransMax)) ? (iTransMax) : (((iTemp) < (iTransMin)) ? (iTransMin) : (iTemp)));
}

INT32 CommonAPI_CM2UI_ValueGet_ByDataCode(eDATA_CODE eDataCode, INT32 *piValue)
{
    sDATABASE_ITEM_DATA_FORMAT sData;
	sUI_ITEM_CFG sUI_Info;
	UINT32 uiDataCode[8] = {0};
	INT32 iDataMax = 0, iDataMin = 0;

	if(eDataCode >= edcINVALID)
		return FALSE;

	uiDataCode[0] = eDataCode;
	if(GetItemInfo_ByDatacode(1, uiDataCode, &sUI_Info) == FALSE)
	{
		return FALSE;
	}

	iDataMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
	iDataMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

    if(SharedMem_ReadData_New(eSB_DATABASE_ITEM, &sData, eDataCode) == TRUE)
    {
        if(IS_COMMON_DATA_CODE(eDataCode))
        {
            *piValue = CM2GUI(eDataCode, sData.uValue.lValue);
        }
		else if((sUI_Info.Max - sUI_Info.Min != 0) && (iDataMax - iDataMin != 0))
		{
			CommonAPI_ValueConvert(sData.uValue.lValue, iDataMax, iDataMin, sUI_Info.Max, sUI_Info.Min, piValue);
		}
	}
	else
	{
	    *piValue = 0;
	}

	return TRUE;
}

INT32 CommonAPI_CM2UI_ValueGet_ByItemID(UINT32 ItemID, INT32 *piValue)
{
    sDATABASE_ITEM_DATA_FORMAT sData;
	sUI_ITEM_CFG sUI_Info;
	UINT32 uiItem[8] = {0};
	INT32 iDataMax = 0, iDataMin = 0;

	uiItem[0] = ItemID;
	if(GetItemInfo_ByItemID(1, uiItem, &sUI_Info) == FALSE)
	{
		return FALSE;
	}

	iDataMax = CommonAPI_DataCode_MaxValue_Get(sUI_Info.DataCodeIndex);
	iDataMin = CommonAPI_DataCode_MinValue_Get(sUI_Info.DataCodeIndex);

    if(SharedMem_ReadData_New(eSB_DATABASE_ITEM, &sData, sUI_Info.DataCodeIndex) == TRUE)
    {
        if(IS_COMMON_DATA_CODE(sUI_Info.DataCodeIndex))
        {
            *piValue = CM2GUI(sUI_Info.DataCodeIndex, sData.uValue.lValue);
        }
		else if((sUI_Info.Max - sUI_Info.Min != 0) && (iDataMax - iDataMin != 0))
		{
			CommonAPI_ValueConvert(sData.uValue.lValue, iDataMax, iDataMin, sUI_Info.Max, sUI_Info.Min, piValue);
		}
	}
	else
	{
	    *piValue = 0;
	}

	return TRUE;
}

INT32 CommonAPI_CM2UI_Range_ByItemID(UINT32 ItemID, INT32 *piMaxValue, INT32 *piMinValue)
{
	sUI_ITEM_CFG sUI_Info;
	UINT32 uiItem[8] = {0};

	uiItem[0] = ItemID;
	if(GetItemInfo_ByItemID(1, uiItem, &sUI_Info) == FALSE)
	{
		return FALSE;
	}

    *piMaxValue = sUI_Info.Max;
    *piMinValue = sUI_Info.Min;

	return TRUE;
}

INT32 CommonAPI_UI2CM_ValueSet_ByDataCode(eDATA_CODE eDataCode, INT32 iValue, INT32 *iTransData)
{
	sUI_ITEM_CFG sUI_Info;
	UINT32 uiDataCode[8] = {0};
	INT32 iDataMax = 0, iDataMin = 0;

	if(eDataCode >= edcINVALID)
		return FALSE;

	uiDataCode[0] = eDataCode;
	if(GetItemInfo_ByDatacode(1, uiDataCode, &sUI_Info) == FALSE)
	{
		return FALSE;
	}

	iDataMax = CommonAPI_DataCode_MaxValue_Get(eDataCode);
	iDataMin = CommonAPI_DataCode_MinValue_Get(eDataCode);

	if(IS_COMMON_DATA_CODE(eDataCode))
	{
	    *iTransData = GUI2CM(eDataCode, iValue);
	}
	else if((sUI_Info.Max - sUI_Info.Min != 0) && (iDataMax - iDataMin != 0))
	{
		CommonAPI_ValueConvert(iValue, sUI_Info.Max, sUI_Info.Min, iDataMax, iDataMin, iTransData);
	}

    return TRUE;
}

INT32 CommonAPI_CM2UI_Range_ByDataCode(eDATA_CODE eDataCode, INT32 *piMaxValue, INT32 *piMinValue)
{
	sUI_ITEM_CFG sUI_Info;
	UINT32 uiDataCode[8] = {0};

	if(eDataCode >= edcINVALID)
		return FALSE;

	uiDataCode[0] = eDataCode;

	if(GetItemInfo_ByDatacode(1, uiDataCode, &sUI_Info) == FALSE)
	{
		return FALSE;
	}

    *piMaxValue = sUI_Info.Max;
    *piMinValue = sUI_Info.Min;

	return TRUE;
}

INT32 CommonAPI_UI2CM_ValueSet_ByItemID(UINT32 ItemID, INT32 iValue, INT32 *iTransData)
{
	sUI_ITEM_CFG sUI_Info;
	UINT32 uiItem[8] = {0};
	INT32 iDataMax = 0, iDataMin = 0;

	uiItem[0] = ItemID;
	if(GetItemInfo_ByItemID(1, uiItem, &sUI_Info) == FALSE)
	{
		return FALSE;
	}

	iDataMax = CommonAPI_DataCode_MaxValue_Get(sUI_Info.DataCodeIndex);
	iDataMin = CommonAPI_DataCode_MinValue_Get(sUI_Info.DataCodeIndex);

	if(IS_COMMON_DATA_CODE(sUI_Info.DataCodeIndex))
	{
	    *iTransData = GUI2CM(sUI_Info.DataCodeIndex, iValue);
	}
	else if((sUI_Info.Max - sUI_Info.Min != 0) && (iDataMax - iDataMin != 0))
	{
		CommonAPI_ValueConvert(iValue, sUI_Info.Max, sUI_Info.Min, iDataMax, iDataMin, iTransData);
	}

    return TRUE;
}



INT32 CommonAPI_CM_FuncIsAvailable(eDATA_CODE DataCode, UINT32 CM_Index)
{
    eDATA_CODE CheckDatacode = edcINVALID;

    switch(DataCode)
    {
        case edcSIZE_PRESETS:
            CheckDatacode = eDI_SIZE_PRESETS;
            break;

        case edcPICTURE_SETTINGS:
            CheckDatacode = eDI_PICTURE_SETTINGS;
            break;

        case edcWARP_MEMORY_APPLY:
            CheckDatacode = eDI_WARP_MEMORY_APPLY;
            break;

        case edcBLEND_MEMORY_APPLY:
            CheckDatacode = eDI_BLEND_MEMORY_APPLY;
            break;

        case edcLENS_APPLY_POSITION:
            CheckDatacode = eDI_LENS_APPLY_POSITION;
            break;

        case edcBACKUP_RESTORE_RESTORE:
            CheckDatacode = eDI_BACKUP_RESTORE_RESTORE;
            break;

        case edcUSER_COLOR_MODE:
            CheckDatacode = eDI_USER_MODE_SETTINGS;
            break;

        case edcGAMMA:
            CheckDatacode = eDI_GAMMA_SETTINGS;
            break;

        case edcMAIN_INPUT:
            CheckDatacode = eDI_MAIN_INPUT_CONDITION;
            break;

        case edcSUB_INPUT:
            CheckDatacode = eDI_SUB_INPUT_CONDITION;
            break;

        case edcCOLOR_SPACE:
            CheckDatacode = eDI_MAIN_COLOR_SPACE;
            break;

        case edcSUB_IMAGE_COLOR_SPACE:
            CheckDatacode = eDI_SUB_COLOR_SPACE;
            break;

        case edcHDR_GAMMA:
            CheckDatacode = eDI_HDR_GAMMA_FUNCTION;
            break;

        case edcDUST_REMAINING_TIME_SELECT:
            CheckDatacode = eDI_DUST_TIME_SELECT;
            break;

        case edcFOG_REMAINING_TIME_SELECT:
            CheckDatacode = eDI_FOG_TIME_SELECT;
            break;

        case edcBACKUPINPUT_PRIMARY_INPUT:
            CheckDatacode = eDI_BACKUPINPUT_PRIMARY_INPUT;
            break;

        case edcBACKUPINPUT_SECONDARY_INPUT:
            CheckDatacode = eDI_BACKUPINPUT_SECONDARY_INPUT;
            break;

        case edc3D_ENABLE:
            CheckDatacode = eDI_3D_MODE_CONDITION;
            break;

        default:
            LOG_TO_SYSLOG("!!!%s undefined DataCode %d\n", __FUNCTION__, DataCode);
            break;
    }

    if(CheckDatacode != edcINVALID)
    {
        sDATABASE_ITEM_DATA_FORMAT sDataCodeInfo;
        if(SharedMem_ReadData_New(eSB_DATABASE_INFOMATION, &sDataCodeInfo, (unsigned int)CheckDatacode) != 1)
        {
            LOG_TO_SYSLOG("Error: %s access shared memory fail\n", __FUNCTION__);
            return 0;
        }

        return !!(BIT_CHECK(sDataCodeInfo.uValue.lValue, CM_Index));
    }

    return 0;
}


INT32 CommonAPI_UI_FuncIsAvailable(eDATA_CODE DataCode, UINT32 UI_Index)
{
    INT32 CM_Value = GUI2CM(DataCode, UI_Index);
    if(CM_Value < 0)
    {
        LOG_TO_SYSLOG("Error: %s map UI value %d fail\n", __FUNCTION__, UI_Index);
        return 0;
    }

    return CommonAPI_CM_FuncIsAvailable(DataCode, CM_Value);
}

UINT32 CommonAPI_UI_ListItem_TotalNum_Get(eDATA_CODE DataCode)
{
    UINT32 TotalNum = DataMapping_GUI_TotalNum_Get(DataCode);

    if(TotalNum > 0)
    {
        return TotalNum;
    }

    return 0;
}


UINT32 CommonAPI_ListItem_NextAvailableValueGet(eDATA_CODE DataCode, UINT32 CurrentIndex)
{
    UINT32 NextIndex = CurrentIndex;

    do
    {
        NextIndex = (++NextIndex) % (CommonAPI_UI_ListItem_TotalNum_Get(DataCode));

        if(NextIndex == CurrentIndex)
            break;

    }while(CommonAPI_UI_FuncIsAvailable(DataCode, NextIndex) == FALSE);

    return NextIndex;
}

UINT32 CommonAPI_ListItem_PreAvailableValueGet(eDATA_CODE DataCode, UINT32 CurrentIndex)
{
    UINT32 PreIndex = CurrentIndex;

    do
    {
        if(PreIndex < 0)
        {
            PreIndex = 0;
            continue;
        }

        PreIndex = (PreIndex == 0) ? (CommonAPI_UI_ListItem_TotalNum_Get(DataCode)-1) : (--PreIndex) ;

        if(PreIndex == CurrentIndex)
            break;

    }while(CommonAPI_UI_FuncIsAvailable(DataCode, PreIndex) == FALSE);

    return PreIndex;
}

UINT16 CommonAPI_LoadErrorLog(UINT16 ucNum, char pcString[][128])
{
    FILE *pFile;
    UINT16 uiReadCount = 0;
    UINT16 uiStart = 0;
    UINT16 uiEnd = 0;
    UINT16 cCount = 0;
    char cLog[MAX_LOG_NUMBER + 1][MAX_LOG_LENGHT] = {'\0'};
	char cTime[MAX_LOG_LENGHT] = {'\0'};
	char cString[MAX_LOG_LENGHT] = {'\0'};

    if(access(ERROR_LOG_TXT,0)==-1)
    {
        return 0;
    }

    //if(utilDataMgr_MutexTake(__FUNCTION__) == TRUE)
    {
        pFile = fopen(ERROR_LOG_TXT, "r");
        flock(fileno(pFile), LOCK_EX);

        if(pFile != NULL)
        {
            while(uiReadCount < MAX_LOG_NUMBER)
            {
                if(fgets(cLog[uiReadCount], MAX_LOG_LENGHT, pFile))
                {
                	memset(cTime,'\0',MAX_LOG_LENGHT);
                	memset(cString,'\0',MAX_LOG_LENGHT);
					sscanf(cLog[uiReadCount], "%127[^,],%127[^,],%*[^ ]", cTime, cString);
					sprintf(cLog[uiReadCount], "%s,%s", cTime, cString);
                    uiReadCount++;
                }
                else
                {
                    break;
                }
            }
            fclose(pFile);

            if(uiReadCount < ucNum)
            {
                uiStart = 0;
                uiEnd = uiReadCount;
            }
            else
            {
                uiStart = uiReadCount - ucNum;
                uiEnd = ucNum;
            }

            for(cCount = 0; cCount < uiEnd; cCount++)
            {
                sprintf(pcString[cCount], "%s", cLog[uiStart + cCount]);
            }
        }
        else
        {
            cCount = 0;
        }

        //utilDataMgr_MutexGive(__FUNCTION__);
        flock(fileno(pFile), LOCK_UN);
    }

    return cCount;
}

UINT16 CommonAPI_LoadErrorLogWithErrorCode(UINT16 ucNum, char pcString[][128])
{
    FILE *pFile;
    UINT16 uiReadCount = 0;
    UINT16 uiStart = 0;
    UINT16 uiEnd = 0;
    UINT16 cCount = 0;
    char cLog[MAX_LOG_NUMBER + 1][MAX_LOG_LENGHT] = {'\0'};
	char cTime[MAX_LOG_LENGHT] = {'\0'};
	char cString[MAX_LOG_LENGHT] = {'\0'};

    if(access(ERROR_LOG_TXT,0)==-1)
    {
        return 0;
    }

    //if(utilDataMgr_MutexTake(__FUNCTION__) == TRUE)
    {
        pFile = fopen(ERROR_LOG_TXT, "r");
        flock(fileno(pFile), LOCK_EX);

        if(pFile != NULL)
        {
            while(uiReadCount < MAX_LOG_NUMBER)
            {
                if(fgets(cLog[uiReadCount], MAX_LOG_LENGHT, pFile))
                {
                    uiReadCount++;
                }
                else
                {
                    break;
                }
            }
            fclose(pFile);

            if(uiReadCount < ucNum)
            {
                uiStart = 0;
                uiEnd = uiReadCount;
            }
            else
            {
                uiStart = uiReadCount - ucNum;
                uiEnd = ucNum;
            }

            for(cCount = 0; cCount < uiEnd; cCount++)
            {
                sprintf(pcString[cCount], "%s", cLog[uiStart + cCount]);
            }
        }
        else
        {
            cCount = 0;
        }

        //utilDataMgr_MutexGive(__FUNCTION__);
        flock(fileno(pFile), LOCK_UN);
    }

    return cCount;
}

eTEST_PATTERN_ID CommonAPI_TestPatternIDGet(eDATA_CODE eDatacode, UINT32 ID)
{
    if(eDatacode == edcOSDTEST_PATTERN ||
       eDatacode == edcSERVICE_TEST_PATTERN ||
       eDatacode == edcTWIST_PATTERN)
    {
        switch(ID)
        {
            case eCM_TEST_PATTERN_OFF:                  return eTID_OFF;
            case eCM_TEST_PATTERN_GRID:                 return eTID_OSD_GRID;
            case eCM_TEST_PATTERN_WHITE:                return eTID_OSD_WHITE;
            case eCM_TEST_PATTERN_BLACK:                return eTID_OSD_BLACK;
            case eCM_TEST_PATTERN_CHECKERBOARD:         return eTID_OSD_CHECKERBOARD;
            case eCM_TEST_PATTERN_COLORBAR:             return eTID_OSD_COLORBAR;
            case eCM_TEST_PATTERN_RED:                  return eTID_OSD_RED;
            case eCM_TEST_PATTERN_GREEN:                return eTID_OSD_GREEN;
            case eCM_TEST_PATTERN_BLUE:                 return eTID_OSD_BLUE;
            case eCM_TEST_PATTERN_YELLOW:               return eTID_OSD_YELLOW;
            case eCM_TEST_PATTERN_MAGENTA:              return eTID_OSD_MAGENTA;
            case eCM_TEST_PATTERN_CYAN:                 return eTID_OSD_CYAN;
            case eCM_TEST_PATTERN_FULL_SCREEN:          return eTID_OSD_FULL_SCREEN;
            case eCM_TEST_PATTERN_4K_FULL_SCREEN:       return eTID_OSD_4K_FULL_SCREEN;
            case eCM_TEST_PATTERN_4K_FULL_SCREEN_2P:    return eTID_OSD_4K_FULL_SCREEN_2P;
            case eCM_TEST_PATTERN_GREEN_GRID:           return eTID_OSD_GREEN_GRID;
            case eCM_TEST_PATTERN_MAGENTA_GRID:         return eTID_OSD_MAGENTA_GRID;
            case eCM_TEST_PATTERN_WHITE_GRID:           return eTID_OSD_WHITE_GRID;
            case eCM_TEST_PATTERN_WHITE_GRID_4CORNER:   return eTID_OSD_WHITE_GRID_4CORNER;
            case eCM_TEST_PATTERN_BORESIGHT:            return eTID_OSD_BORESIGHT;
            case eCM_TEST_PATTERN_GRAY:                 return eTID_OSD_GRAY;
            case eCM_TEST_PATTERN_13POINTS:             return eTID_OSD_13POINTS;
            case eCM_TEST_PATTERN_H_RAMP:               return eTID_OSD_H_RAMP;
            case eCM_TEST_PATTERN_V_RAMP:               return eTID_OSD_V_RAMP;
            case eCM_TEST_PATTERN_RED_GRID:             return eTID_OSD_RED_GRID;
            case eCM_TEST_PATTERN_BLUE_GRID:            return eTID_OSD_BLUE_GRID;
            case eCM_TEST_PATTERN_CHECKERBOARD_2:       return eTID_OSD_CHECKERBOARD_2;
            case eCM_TEST_PATTERN_COLORBAR_2:           return eTID_OSD_COLORBAR_2;
            case eCM_TEST_PATTERN_COLORBAR_3:           return eTID_OSD_COLORBAR_3;
            case eCM_TEST_PATTERN_ASPECT_FRAME:         return eTID_OSD_ASPECT_FRAME;

            default:
            //    LOG_MSG(db_ALWAYS, "(funcs:%s, line:%d)Can`t get Test Pattern ID (%d %d)\n", eDatacode, ID);
                return eTID_INVALID;
        }
    }
    else if(eDatacode == edcFACTORY_TEST_PATTERN)
    {
        switch(ID)
        {
            case eCM_FACTORY_TP_NONE:               return eTID_OFF;
            case eCM_FACTORY_TP_WHITE:              return eTID_DDP_WHITE;
            case eCM_FACTORY_TP_BLACK:              return eTID_DDP_BLACK;
            case eCM_FACTORY_TP_RED:                return eTID_DDP_RED;
            case eCM_FACTORY_TP_GREEN:              return eTID_DDP_GREEN;
            case eCM_FACTORY_TP_BLUE:               return eTID_DDP_BLUE;
            case eCM_FACTORY_TP_CYAN:               return eTID_DDP_CYAN;
            case eCM_FACTORY_TP_YELLOW:             return eTID_DDP_YELLOW;
            case eCM_FACTORY_TP_MAGENTA:            return eTID_DDP_MAGENTA;
            case eCM_FACTORY_TP_BLUE60:             return eTID_DDP_BLUE_60;
            case eCM_FACTORY_TP_GRAY10:             return eTID_DDP_GRAY_10;
            case eCM_FACTORY_TP_WRGB64:             return eTID_DDP_WRGB_64;
            case eCM_FACTORY_TP_FLARE:              return eTID_DDP_FLARE;
            case eCM_FACTORY_TP_FULL_SREEN_W:       return eTID_DDP_FULL_SCREEN_W;
            case eCM_FACTORY_TP_FULL_SREEN_B:       return eTID_DDP_FULL_SCREEN_B;
            case eCM_FACTORY_TP_LATERAL_COLOR:      return eTID_DDP_LATERAL_COLOR;
            case eCM_FACTORY_TP_FOCUS_WORD:         return eTID_DDP_FOCUS_WORD;
            case eCM_FACTORY_TP_13POINTS:           return eTID_DDP_13POINTS;
            case eCM_FACTORY_TP_FULL_COLOR:         return eTID_DDP_FULL_COLOR;
            case eCM_FACTORY_TP_BORESIGHT:          return eTID_DDP_BORESIGHT;
            case eCM_FACTORY_TP_ACTUATOR:           return eTID_DDP_ACTUATOR_CAL;
			case eCM_FACTORY_TP_4K_FULL_SCREEN_1P:  return eTID_OSD_4K_FULL_SCREEN;
			case eCM_FACTORY_TP_4K_FULL_SCREEN_2P:  return eTID_OSD_4K_FULL_SCREEN_2P;

            default:
            //    LOG_MSG(db_ALWAYS, "(funcs:%s, line:%d)Can`t get Test Pattern ID (%d %d)\n", eDatacode, ID);
                return eTID_INVALID;
        }

    }
    else if(eDatacode == edcHSG_TEST_PATTERN_CTRL)
    {
        switch(ID)
        {
            case eCM_TEST_PATTERN_OFF:                  return eTID_OFF;
            case eCM_TEST_PATTERN_WHITE:                return eTID_HSG_WHITE;
            case eCM_TEST_PATTERN_RED:                  return eTID_HSG_RED;
            case eCM_TEST_PATTERN_GREEN:                return eTID_HSG_GREEN;
            case eCM_TEST_PATTERN_BLUE:                 return eTID_HSG_BLUE;
            case eCM_TEST_PATTERN_YELLOW:               return eTID_HSG_YELLOW;
            case eCM_TEST_PATTERN_MAGENTA:              return eTID_HSG_MAGENTA;
            case eCM_TEST_PATTERN_CYAN:                 return eTID_HSG_CYAN;
            default:
            //    LOG_MSG(db_ALWAYS, "(funcs:%s, line:%d)Can`t get Test Pattern ID (%d %d)\n", eDatacode, ID);
                return eTID_INVALID;
        }

    }

    return eTID_INVALID;

}


