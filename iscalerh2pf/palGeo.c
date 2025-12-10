#include "Common.h"
#include "utilCommon.h"
#include "appDataMgr.h"
#include "appLANProcAPI.h"
#include "appDataPath.h"

#include "halGui.h"
#include "halWarping.h"
#include "halScaler.h"

#ifdef SCALER_C821_C789
#include "dvC789_WarpLight.h"
#include "dvC789.h"
#include "utilWarpDemo.h"
#elif defined(SCALER_FPGA_F34)
#include "utilWarpDemoProAV.h"
#endif

#include "palGeoAPI.h"
#include "utilDatabaseAPI.h"
#include "utilMisc.h"
#include "utilDbgMsg.h"
#include "utilQueueAPI.h"
#include "ProcessMutexData.h"

#include "opdCtrlAPI.h"
#include "PNGUtility.h"
#include "GeneralFunc.h"
#include <math.h>

typedef struct
{
    ePANEL_ID                       ePanelTimingId;
    BOOL                            bWarpingInit;
    //UINT32                          ulWarpingErrCode;
    SemaphoreHandle_t               xSemaphore;

#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    StaticSemaphore_t               xMutexBuffer;
#endif


    eHAL_WARPING_CROSSHATCH_MODE    ePatternGen;
    UINT16                          uiBlendingEdgeHW;
    UINT16                          uiBlendingEdgeVW;

    eHAL_WARPING_WARP_MODE          eWarpMode;

    eHAL_WARPING_MOVE_PITCH         eMovePitchIndex;               // PB_100_GRID_MV        *
    float                           fGridMovePitch;                 // PM_GRID_MV           *


    BOOL                            bGridShow;
    BOOL                            bInternalGridShow;          // PM_CUR_INTERNAL_ON

    UINT8                           ucGridSelX;                 // PM_SEL_GRID_X
    UINT8                           ucGridSelY;                 // PM_SEL_GRID_Y
    UINT8                           ucPrevGridSelX;                 // GV_PREV_SEL_GRID_X
    UINT8                           ucPrevGridSelY;                 // GV_PREV_SEL_GRID_Y

    //BOOL                            baGridCursorDisplayed[17][17];  // GV_GRID_DISP, to indicate any warp mode which cursor already shown on screen
    BOOL                            baGridCursorEnabled[17][17];    // GV_GRID_EN, to indicate in any warp mode which cursor can be shown


    UINT16                          uiWarp_HW;
    UINT16                          uiWarp_VW;
    UINT16                          uiWarp_Is120HzPanel;    //A35G2_Simon_0093
    int                             uiWarp_HW16;
    int                             uiWarp_VW16;
    float                           fWarp_PHWHF;
    float                           fWarp_PVWHF;

    UINT8                           ucFreeze;

    //Warp OSD
    UINT8                           aucPalette[256][3];
    UINT32                          aulSpritePalette[256];
    UINT32                          aulMapSpritePalette[256];
    UINT16                          uiSpritePaletteCount;
    UINT16                          uiMapSpritePaletteCount;
    //UINT16                          uiInhibit_Color;
    //INT16                           iPalette_ColorAmount;

    //Black Level
    UINT8                           aucBL_Palette[16][3];
    UINT8                           aucBL_GammaTable[16][3][16];
    FLOAT                           aucBL_GammaCoef;    //2.2

    UINT8                           ucColorMask;
    UINT8                           ucPixIndexed[4096 * 10];

    UINT8                           ucCurrentWarpingType;  //WARPING_TYPE_OSD / WARPING_TYPE_AP  //A35G2_Simon_0115

    UINT8                           m_ucACU_Init_Status;

    pthread_mutex_t                 OSDQueueDrawMutex;  //H2PF_Simon_0038
    sQUEUE                          sDrawWarpOsd_Queue;
    sDRAW_WAPR_OSD_QUEUE_INFO       sDrawWarpOsd_QueueBuffer[2048];
}sPAL_GEO_INFORMATION, *PsPAL_GEO_INFORMATION;


static sPAL_GEO_INFORMATION m_sPalGeoInfo;
static TickType_t ulpalGeoStartTicks;

static void palGeo_ResetTimeInState(void)
{
    ulpalGeoStartTicks = xTaskGetTickCount();
}

UINT32 palGeo_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(ulpalGeoStartTicks > ulTicks)
    {
        return (0xFFFFFFFF - ulpalGeoStartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - ulpalGeoStartTicks) / portTICK_RATE_MS;
    }
}


static BOOL palGeo_SemaphoreTake(const char *pcFunc)
{
    BOOL bResult = halWarping_SemaphoreTake(pcFunc);

    return bResult;
}

static BOOL palGeo_SemaphoreGive(const char *pcFunc)
{
    BOOL bResult = halWarping_SemaphoreGive(pcFunc);

    return bResult;
}

static UINT8 LastHandleFuncName[64] = {'\0'};
#define PALGEO_SEMAPHORE_TAKE if(palGeo_SemaphoreTake(__FUNCTION__))\
                              {\
                                  snprintf(LastHandleFuncName, sizeof(LastHandleFuncName), "%s", __FUNCTION__);

#define PALGEO_SEMAPHORE_GIVE     palGeo_SemaphoreGive(__FUNCTION__);\
                              }\
                              else \
                              {\
                                  LOG_MSG(db_HAL_WARPING, "!!!(func:%s,line:%d) Mutex Fail (Last Function: %s)\n", __FUNCTION__, __LINE__, LastHandleFuncName);\
                                  return ePAL_GEO_EXEC_CODE_MUTEX_FAIL;\
                              }

#define PALGEO_SEMAPHORE_GIVE_ONLY     palGeo_SemaphoreGive(__FUNCTION__);

#if 0
#define PALGEO_SEMAPHORE_ELOG else \
                              {\
                                  LOG_MSG(db_HAL_WARPING, "!!!(func:%s,line:%d) mutex fail\n", __FUNCTION__, __LINE__);\
                                  return ePAL_GEO_EXEC_CODE_MUTEX_FAIL;\
                              }
#endif


//for ProAV
#define OSD_QUEUE_DRAW_MUTEX_WAIT_DELAY (20000)
static ePAL_GEO_EXEC_CODE palGeo_OsdQueueDrawSemaphoreCreate(void)  //H2PF_Simon_0038
{
    if(pthread_mutex_init(&m_sPalGeoInfo.OSDQueueDrawMutex, NULL) == -1)
    {
        LOG_MSG(db_HAL_WARPING, "!OSDQueueDrawMutex Init Fail\n");
        return eHAL_WARPING_EXEC_CODE_INIT_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "%s Pass\r\n", __FUNCTION__);

    return ePAL_GEO_EXEC_CODE_PASS;
}

static BOOL palGeo_OsdQueueDrawSemaphoreTake(void)    //H2PF_Simon_0038
{
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, OSD_QUEUE_DRAW_MUTEX_WAIT_DELAY);

    INT32 lResult = pthread_mutex_timedlock(&m_sPalGeoInfo.OSDQueueDrawMutex, &s_timeout);
    if(lResult != 0)
    {
        LOG_MSG(db_HAL_WARPING, "%s pthread_mutex_timedlock fail [%s]\n", __FUNCTION__ , strerror(lResult));
        return FALSE;
    }

    return TRUE;
}

static BOOL palGeo_OsdQueueDrawSemaphoreGive(void)    //H2PF_Simon_0038
{
    INT32 lResult = pthread_mutex_unlock(&m_sPalGeoInfo.OSDQueueDrawMutex);

    if(lResult != 0)
    {
        LOG_MSG(db_HAL_WARPING, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(lResult));
        return FALSE;
    }

    return TRUE;
}


ePAL_GEO_EXEC_CODE palGeo_ExeResult(eHAL_WARPING_EXEC_CODE eResult)
{
    ePAL_GEO_EXEC_CODE ePAL_Result;

    switch(eResult)
    {
        case eHAL_WARPING_EXEC_CODE_PASS:
            ePAL_Result = ePAL_GEO_EXEC_CODE_PASS;
            break;

        case eHAL_WARPING_EXEC_CODE_FAIL:
            ePAL_Result = ePAL_GEO_EXEC_CODE_FAIL;
            break;

        case eHAL_WARPING_EXEC_CODE_INIT_FAIL:
            ePAL_Result = ePAL_GEO_EXEC_CODE_INIT_FAIL;
            break;

        case eHAL_WARPING_EXEC_CODE_NOT_INIT:
            ePAL_Result = ePAL_GEO_EXEC_CODE_NOT_INIT;
            break;

        case eHAL_WARPING_EXEC_CODE_MALLOC_FAIL:
            ePAL_Result = ePAL_GEO_EXEC_CODE_MALLOC_FAIL;
            break;

        case eHAL_WARPING_EXEC_CODE_MUTEX_FAIL:
            ePAL_Result = ePAL_GEO_EXEC_CODE_MUTEX_FAIL;
            break;

        default:
            LOG_MSG(db_HAL_WARPING, "undefined eHAL_WARPING_EXEC_CODE %d", eResult);
            ePAL_Result = ePAL_GEO_EXEC_CODE_NUMBER;
            break;

    }

    return ePAL_Result;
}


ePAL_GEO_EXEC_CODE palGeo_Initial(void)
{
    PALGEO_SEMAPHORE_TAKE;
    m_sPalGeoInfo.m_ucACU_Init_Status = WARPING_ACU_INIT_NOT_READY;
    halWarping_Init();

    utilQueueInitial(&m_sPalGeoInfo.sDrawWarpOsd_Queue,
                     sizeof(m_sPalGeoInfo.sDrawWarpOsd_QueueBuffer)/sizeof(m_sPalGeoInfo.sDrawWarpOsd_QueueBuffer[0]),
                     sizeof(sDRAW_WAPR_OSD_QUEUE_INFO),
                     (UINT8*)m_sPalGeoInfo.sDrawWarpOsd_QueueBuffer);  //H2PF_Simon_0038

    palGeo_OsdQueueDrawSemaphoreCreate();

    m_sPalGeoInfo.bWarpingInit = TRUE;  //H2PF_Simon_0038
    PALGEO_SEMAPHORE_GIVE;


    return ePAL_GEO_EXEC_CODE_PASS;
}

UINT16 palGeo_QueueDataCountGet(void)  //H2PF_Simon_0038
{
    return utilQueueDataCountGet(&m_sPalGeoInfo.sDrawWarpOsd_Queue);
}

UINT8 palGeo_Is4kWarp(void)
{
    return halWarping_Is4KWarp();
}

ePAL_GEO_EXEC_CODE palGeo_PowerNormal(ePANEL_ID ePanelID)
{
    PALGEO_SEMAPHORE_TAKE;
    halWarping_PowerNormal(ePanelID);
    PALGEO_SEMAPHORE_GIVE;

    //return ePAL_GEO_EXEC_CODE_PASS;   //for test

    palGeo_Color_Uniformity_Init();  //apply CU data

#ifdef CUSTOM_CHRISTIE
    {
        palGeo_AdvWarpingInit(ePanelID);
        palGeo_ADV_WarpingParameterInit();
        palGeo_BlackLevel_Enable(FALSE);

        if( palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_AP_MYSTIQUE ||
            palDataMgr_CurrentWarpMemoryType() == WARPING_TYPE_ADVANCED)
        {
            UINT8 ucApplyIndex = 0;
            palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaREAD, &ucApplyIndex);
            palDataMgr_Data_Access(edcWARP_MEMORY_APPLY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucApplyIndex);
        }
        else
        {
            UINT8 WarpControl = WARP_CTRL__BASIC ;
            palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaWRITE_THROUGH_WITH_ACTION, &WarpControl);
            palGeo_Warp_GeometrySet();
        }

        if( palDataMgr_CurrentBlendMemoryType() == WARPING_TYPE_AP_MYSTIQUE )
        {
            UINT8 ucApplyIndex = 0;
            palDataMgr_Data_Access(edcBLEND_MEMORY_APPLY, edaREAD, &ucApplyIndex);
            palDataMgr_Data_Access(edcBLEND_MEMORY_APPLY, edaWRITE_RAM_ONLY_WITH_ACTION, &ucApplyIndex);
        }
        else
        {
            palGeo_Warp_BlendingSet();
            palDataMgr_ADV_Blend_LfRtTpBm_Update();
            ///palDataMgr_MenuOffsetUpdateForEdgeBlending();

            #if 0  //H2PF_Simon_0050 , set blend set datacode will set "Apply Blend Memory"" to "Off""
            UINT16 ucPixel = 0;
            palDataMgr_Data_Access(edcBLENDING_TOP_START_PIXEL, edaREAD, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_TOP_START_PIXEL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_BOTTOM_START_PIXEL, edaREAD, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_BOTTOM_START_PIXEL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_LEFT_START_PIXEL, edaREAD, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_LEFT_START_PIXEL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_RIGHT_START_PIXEL, edaREAD, &ucPixel);
            palDataMgr_Data_Access(edcBLENDING_RIGHT_START_PIXEL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucPixel);
            #endif
        }
    }
#elif defined(CUSTOM_BARCO)
    {
        UINT8 ucData = 0 ;
        palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);                          //G100_Doulas_0027 Add
        palGeo_AdvWarpingInit(m_sPalDataPathInfo.ePanelTimingId);                                   //G100_Doulas_0027 init ADV
        palGeo_ADV_WarpingParameterInit();                                                  //G100_Doulas_0027 init Parameter
        if(ucData == WARP_CTRL__ADVANCED)   //G100_Doulas_0079 Modify
        {
            halWarping_clearBlackLevel();  //A35G2_Simon_0093
            palDataMgr_WarppingControlAdvanceSet();
        }
        else
        {
            palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);    //G100_Doulas_0027 Modify
        }
        palDataMgr_ADV_Blend_LfRtTpBm_Update();
    }
#else  //for blender
    {
        UINT8 ucData = 0 ;
        palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaREAD, &ucData);                          //G100_Doulas_0027 Add
        LOG_MSG(db_HAL_WARPING, "edcADV_WARP_CONTROL %d\n" ,  ucData);
        palGeo_AdvWarpingInit(ePanelID);                                   //G100_Doulas_0027 init ADV
        palGeo_ADV_WarpingParameterInit();                                                  //G100_Doulas_0027 init Parameter
        palGeo_DBD_Blending_Init();
        palGeo_BlackLevel_Init();
        if(ucData == WARP_CTRL__ADVANCED)   //G100_Doulas_0079 Modify
        {
            palDataMgr_WarppingControlAdvanceSet();
        }
        else
        {
            if(ucData == WARP_CTRL__AP)   //A65_OPTOMA_Simon_0006
            {
                palGeo_Func_Set(eGFN_AP_CLEAR_ALL, NULL);  //H2PF_Simon_0038
            }

            palDataMgr_Data_Access(edcADV_WARP_CONTROL, edaWRITE_RAM_ONLY_WITH_ACTION, &ucData);    //G100_Doulas_0027 Modify
        }
        palDataMgr_ADV_Blend_LfRtTpBm_Update();                                                 //G100_Doulas_0027
    }
#endif

    #ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);
    PALGEO_SEMAPHORE_GIVE;
    #else
    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_ClearOSD();
    PALGEO_SEMAPHORE_GIVE;
    #endif

    return ePAL_GEO_EXEC_CODE_PASS;

}


ePAL_GEO_EXEC_CODE palGeo_InfoPrint(void)  //H2PF_Simon_0038
{
    PALGEO_SEMAPHORE_TAKE;
    halWarping_OSD_PalettePrint();
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_ApplyWarpTable(sWarpFileInfo_H2 sFileInfo)
{
    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_MemoryFileNameGet(ePAL_GEO_EXEC_CODE eFunction, UINT8 MemIndex, UINT8 *FileName)
{
    switch(eFunction)
    {
        case eGFT_AP_WARP:
            snprintf(FileName, GEO_MEMORY_MAX_FILENAME_LENGTH, BLENDING_AP_HICC2_WARP_SAVE_FILENAME, MemIndex);
            break;

        case eGFT_AP_BLEND:
            snprintf(FileName, GEO_MEMORY_MAX_FILENAME_LENGTH, BLENDING_AP_HICC2_DBD_SAVE_FILENAME, MemIndex);
            break;

        case eGFT_AP_BLACKLEVEL:
            snprintf(FileName, GEO_MEMORY_MAX_FILENAME_LENGTH, BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME, MemIndex);
            break;

        default:
            LOG_MSG(db_ASSERT, "!!! (func:%s, line:%d) undefined Geo Function %d\n", __FUNCTION__, __LINE__, eFunction);
            break;
    }

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_MemoryFileVerifyChecksum(ePAL_GEO_EXEC_CODE eFunction, UINT8 *aucData)
{
    UINT16 sum = 0;
    UINT32 DataSize = 0;
    UINT16 FileChecksum = 0;  //checksum value at file record

    switch(eFunction)
    {
        case eGFT_AP_WARP:
            DataSize = sizeof(sWarpFileInfo_H2);
            break;

        case eGFT_AP_BLEND:
            DataSize = sizeof(sBlendFileInfo_H2);
            break;

        case eGFT_AP_BLACKLEVEL:
            DataSize = sizeof(sBlackLevelFileInfo_H2);
            break;

        default:
            LOG_MSG(db_ASSERT, "!!! (func:%s, line:%d) undefined Geo Function %d\n", __FUNCTION__, __LINE__, eFunction);
            break;
    }

    FileChecksum = *(UINT16*)((UINT8*)aucData + DataSize - sizeof(UINT16));  //checksum record at last 2 bytes

    for(UINT32 count=0; count<DataSize-sizeof(UINT16); count++)
    {
        sum += *((UINT8 *)aucData + count);
    }

    if( (UINT16)(sum + FileChecksum) == 0 )
    {
        return ePAL_GEO_EXEC_CODE_PASS;
    }

    LOG_MSG(db_HAL_WARPING, "checksum error (sum == 0x%04X, FileChecksum == 0x%04X)\n", sum, FileChecksum);

    return ePAL_GEO_EXEC_CODE_FAIL;
}


//ucIndex == 0 : current
ePAL_GEO_EXEC_CODE palGeo_MemoryApplyAP_Warp(UINT8 ucIndex, eWARP_FUNC_AP_APPLY_TYPE eType)
{
    ePAL_GEO_EXEC_CODE eResult = ePAL_GEO_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d) %d\r\n", __FUNCTION__, __LINE__, ucIndex);

    PALGEO_SEMAPHORE_TAKE;
    INT8 cFileName[GEO_MEMORY_MAX_FILENAME_LENGTH] = {0};
    palGeo_MemoryFileNameGet(eGFT_AP_WARP, ucIndex, cFileName);

    UINT32 ulMemFileSize = Get_File_Size(cFileName);
    UINT32 ulSize = sizeof(sWarpFileInfo_H2);

    //check file size
    if(ulMemFileSize != ulSize)
    {
        LOG_MSG(db_HAL_WARPING, "Memory %d File Size Error (File %d, Need %d)\n", ucIndex, ulMemFileSize, ulSize);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_INCORRECT_FILE_SIZE;
    }

    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        //halWarping_ClearWarpingTable();
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_MUTEX_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
    {
        ASSERT_ALWAYS();
        halWarping_ClearWarpingTable();  //HICC2_Simon_0015
        free(pucData);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FAIL;
    }

    //check checksum
    if(palGeo_MemoryFileVerifyChecksum(eGFT_AP_WARP, pucData) != ePAL_GEO_EXEC_CODE_PASS)
    {
        ASSERT_ALWAYS();
        free(pucData);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FILE_CHECKSUM_ERROR;
    }

    sWarpFileInfo_H2 *sWarpInfo = (sWarpFileInfo_H2 *)pucData;
    LOG_MSG(db_HAL_WARPING, "0x%08X Header 0x%04X\n",         offsetof(sWarpFileInfo_H2, Header),        sWarpInfo->Header);
    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag 0x%02X\n",     offsetof(sWarpFileInfo_H2, ucEnableFlag),  sWarpInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",             offsetof(sWarpFileInfo_H2, uc3DFlag),      sWarpInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",              offsetof(sWarpFileInfo_H2, ucWidth),       sWarpInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",             offsetof(sWarpFileInfo_H2, ucHeight),      sWarpInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table_TpX %f %f %f\n",    offsetof(sWarpFileInfo_H2, afTable_TpX),   sWarpInfo->afTable_TpX[0], sWarpInfo->afTable_TpX[1], sWarpInfo->afTable_TpX[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table_TpY %f %f %f\n",    offsetof(sWarpFileInfo_H2, afTable_TpY),   sWarpInfo->afTable_TpY[0], sWarpInfo->afTable_TpY[1], sWarpInfo->afTable_TpY[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X HorzFilterIdx %d\n",      offsetof(sWarpFileInfo_H2, HorzFilterIdx), sWarpInfo->HorzFilterIdx);
    LOG_MSG(db_HAL_WARPING, "0x%08X VertFilterIdx %d\n",      offsetof(sWarpFileInfo_H2, VertFilterIdx), sWarpInfo->VertFilterIdx);

    //check Header
    if(sWarpInfo->Header != HICC2_GEO_FILE_HEADER)
    {
        ASSERT_ALWAYS();
        free(pucData);
        LOG_MSG(db_HAL_WARPING, "Warp file header error 0x%04X\n", sWarpInfo->Header);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FAIL;
    }

    //check ucEnableFlag
    if(sWarpInfo->ucEnableFlag != WARPING_TYPE_AP && sWarpInfo->ucEnableFlag != WARPING_TYPE_AP_MYSTIQUE)
    {
        LOG_MSG(db_HAL_WARPING, "Clear Warping\n");
        halWarping_ClearWarpingTable();  //HICC2_Simon_0015
        free(pucData);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_PASS;
    }

    //check panel
    if(sWarpInfo->uc3DFlag == FALSE)  //warp data is 2D
    {
        if(halWarpOSD_PanelID_Get() != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply warp %d %d\n", sWarpInfo->uc3DFlag, halWarpOSD_PanelID_Get());  //H2PF_Simon_0038
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_GeometrySet();
            free(pucData);
            PALGEO_SEMAPHORE_GIVE_ONLY;
            return ePAL_GEO_EXEC_CODE_PASS;
        }
    }
    else  //warp data is 3D
    {
        if(halWarpOSD_PanelID_Get() == PANEL_2D_OUTPUT)  //not 3D panel  //H2PF_Simon_0038
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply warp %d %d\n", sWarpInfo->uc3DFlag, halWarpOSD_PanelID_Get());  //H2PF_Simon_0038
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_GeometrySet();
            free(pucData);
            PALGEO_SEMAPHORE_GIVE_ONLY;
            return ePAL_GEO_EXEC_CODE_PASS;
        }
    }

    INT32 Start = TMO_GetSysRunTime();
    if(eType == eWAT_WARP_TABLE || eType == eWAT_ALL)
    {
        eResult |= halWarping_convertTPtoDTDT(sWarpInfo->afTable_TpX, sWarpInfo->afTable_TpY, sWarpInfo->ucWidth, sWarpInfo->ucHeight);
    }

    //H2 wait review , need to read system config
    BYTE ApWarpFilterAutoEnable = 0;
    halWarp_GeometryParameterGet(eWARP_EVENT_AP_WARP_FILTER_AUTO_ENABLE, &ApWarpFilterAutoEnable);

    //H2 wait review , need to read system config
    if(ApWarpFilterAutoEnable == FALSE)  // load AP manual value       //A35G2_Simon_0110
    {
        if(sWarpInfo->HorzFilterIdx < WAPR_FILTER_H_MAX) //H index
        {
            halWarp_GeometryParameterSet(eWARP_EVENT_WARP_H_FILTER_AP_SEL, &sWarpInfo->HorzFilterIdx);
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "warp filter error H %d\n", sWarpInfo->HorzFilterIdx);
        }

        if(sWarpInfo->VertFilterIdx < WAPR_FILTER_V_MAX) //V index
        {
            halWarp_GeometryParameterSet(eWARP_EVENT_WARP_V_FILTER_AP_SEL, &sWarpInfo->VertFilterIdx);
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "warp filter error V %d\n", sWarpInfo->VertFilterIdx);
        }
    }

    halWarp_InterpolationSet(); //A35G2_Simon_0110

    INT32 End = TMO_GetSysRunTime();
    LOG_MSG(db_HAL_WARPING, "\n%s time:%d\n", __FUNCTION__, End-Start);

    free(pucData);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


//ucIndex == 0 : current
ePAL_GEO_EXEC_CODE palGeo_MemoryApplyAP_Blending(UINT8 ucIndex, eBLEND_FUNC_AP_APPLY_TYPE eType)
{
    ePAL_GEO_EXEC_CODE eResult = ePAL_GEO_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d) %d\r\n", __FUNCTION__, __LINE__, ucIndex);

    PALGEO_SEMAPHORE_TAKE;

    INT8 cFileName[GEO_MEMORY_MAX_FILENAME_LENGTH] = {0};
    palGeo_MemoryFileNameGet(eGFT_AP_BLEND, ucIndex, cFileName);

    UINT32 ulMemFileSize = Get_File_Size(cFileName);
    UINT32 ulSize = sizeof(sBlendFileInfo_H2);

    //check file size
    if(ulMemFileSize != ulSize)
    {
        LOG_MSG(db_HAL_WARPING, "Memory %d File Size Error (File %d, Need %d)\n", ucIndex, ulMemFileSize, ulSize);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_INCORRECT_FILE_SIZE;
    }

    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        //halWarping_DBD_Blending_Enable(FALSE);
        halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
        halWarp_BlendingSet();
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        //halWarping_DBD_Blending_Enable(FALSE);
        halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
        halWarp_BlendingSet();
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check checksum
    if(palGeo_MemoryFileVerifyChecksum(eGFT_AP_BLEND, pucData) != ePAL_GEO_EXEC_CODE_PASS)
    {
        ASSERT_ALWAYS();
        free(pucData);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FILE_CHECKSUM_ERROR;
    }

    sBlendFileInfo_H2 *sBlendInfo = (sBlendFileInfo_H2 *)pucData;
    LOG_MSG(db_HAL_WARPING, "0x%08X Header 0x%04X\n",     offsetof(sBlendFileInfo_H2, Header),       sBlendInfo->Header);
    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag 0x%02X\n", offsetof(sBlendFileInfo_H2, ucEnableFlag), sBlendInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",         offsetof(sBlendFileInfo_H2, uc3DFlag),     sBlendInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",          offsetof(sBlendFileInfo_H2, ucWidth),      sBlendInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",         offsetof(sBlendFileInfo_H2, ucHeight),     sBlendInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table %d %d %d\n",    offsetof(sBlendFileInfo_H2, aucTable),     sBlendInfo->aucTable[0], sBlendInfo->aucTable[1], sBlendInfo->aucTable[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X EdgeBlend %d %d %d\n",offsetof(sBlendFileInfo_H2, aucEdgeBlend), sBlendInfo->aucEdgeBlend[0], sBlendInfo->aucEdgeBlend[1], sBlendInfo->aucEdgeBlend[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X EGBCT %d %d %d\n",    offsetof(sBlendFileInfo_H2, aucEGBCT),     sBlendInfo->aucEGBCT[0], sBlendInfo->aucEGBCT[1], sBlendInfo->aucEGBCT[2]);

    //check Header
    if(sBlendInfo->Header != HICC2_GEO_FILE_HEADER)
    {
        ASSERT_ALWAYS();
        free(pucData);
        LOG_MSG(db_HAL_WARPING, "DBD Blend file header error 0x%04X\n", sBlendInfo->Header);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FAIL;
    }

    //check ucEnableFlag
    if(sBlendInfo->ucEnableFlag == 0)  //disable
    {
        halWarping_DBD_ClearBlendingTable();   //HICC2_Simon_0016
        free(pucData);
        //halWarping_DBD_Blending_Enable(FALSE);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else if(sBlendInfo->ucEnableFlag != WARPING_TYPE_AP && sBlendInfo->ucEnableFlag != WARPING_TYPE_AP_MYSTIQUE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        //halWarping_DBD_Blending_Enable(FALSE);
        halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
        halWarp_BlendingSet();
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check panel
    if(sBlendInfo->uc3DFlag == FALSE)  //blending data is 2D
    {
        if(halWarpOSD_PanelID_Get() != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blend %d %d\n", sBlendInfo->uc3DFlag, halWarpOSD_PanelID_Get());
            eResult |= halWarping_DBD_Blending_Enable(FALSE);  //H2PF_Simon_0099
            halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
            halWarp_BlendingSet();
            free(pucData);
            PALGEO_SEMAPHORE_GIVE_ONLY;
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }
    else  //blending data is 3D
    {
        if(halWarpOSD_PanelID_Get() == PANEL_2D_OUTPUT)  //not 3D panel  //H2PF_Simon_0038
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blend %d %d\n", sBlendInfo->uc3DFlag, halWarpOSD_PanelID_Get());
            eResult |= halWarping_DBD_Blending_Enable(FALSE);  //H2PF_Simon_0099
            halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
            halWarp_BlendingSet();
            free(pucData);
            PALGEO_SEMAPHORE_GIVE_ONLY;
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }

    //LOG_MSG(db_HAL_WARPING, "Need Rewrite Check %d %d\n", m_sPalGeoInfo.ucCurrent_DBD_Data_Index, ucIndex);

    INT32 Start = TMO_GetSysRunTime();

    #if 0
    if(m_sPalGeoInfo.ucCurrent_DBD_Data_Index == ucIndex)  //Data already write into CPU
    {
        eResult |= halWarping_DBD_Blending_Enable(TRUE);
    }
    else
    {
        m_sPalGeoInfo.ucCurrent_DBD_Data_Index = ucIndex;
    }

    halWarping_BlendingGamma(TRUE);
    #endif

    //check resolution          //A35G2_Simon_0120 Start
    UINT16 uiApplyWidth  = sBlendInfo->ucWidth;
    UINT16 uiApplyHeight = sBlendInfo->ucHeight;

    //if file data error, use panel resolution
    if(uiApplyWidth  > MAX_DBD_BLEND_WIDTH ||
       uiApplyHeight > MAX_DBD_BLEND_HEIGHT
    )
    {
        //use panel resolution
        uiApplyWidth  = m_sPalGeoInfo.uiWarp_HW;
        uiApplyHeight = m_sPalGeoInfo.uiWarp_VW;

        //OPD
        UINT8 aucString[256];
        snprintf((char *)aucString, sizeof(aucString), "BlendMemory%d,%dx%d,%dx%d", ucIndex+1
                                                                                  , sBlendInfo->ucWidth
                                                                                  , sBlendInfo->ucHeight
                                                                                  , uiApplyWidth
                                                                                  , uiApplyHeight);

        halWarping_OPDEvent(aucString);
        LOG_MSG(db_HAL_WARPING, "%s\n", aucString);
    }

    eResult |= halWarping_DBD_BlendingTable_Set(uiApplyWidth, uiApplyHeight, sBlendInfo->aucTable); //A35G2_Simon_0120 End

    INT32 End = TMO_GetSysRunTime();


    LOG_MSG(db_HAL_WARPING, "\nhalWarping_MemoryApplyAP_Blending time %d\n", End-Start);

    free(pucData);

    PALGEO_SEMAPHORE_GIVE;

    return eResult;
}


//ucIndex == 0 : current
ePAL_GEO_EXEC_CODE palGeo_MemoryApplyAP_BlackLevel(UINT8 ucIndex, eBLACKLEVEL_FUNC_AP_APPLY_TYPE eType)
{
    ePAL_GEO_EXEC_CODE eResult = ePAL_GEO_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d) %d type %d\r\n", __FUNCTION__, __LINE__, ucIndex, eType );

    PALGEO_SEMAPHORE_TAKE;

    INT8 cFileName[GEO_MEMORY_MAX_FILENAME_LENGTH] = {0};
    palGeo_MemoryFileNameGet(eGFT_AP_BLACKLEVEL, ucIndex, cFileName);

    UINT32 ulMemFileSize = Get_File_Size(cFileName);
    UINT32 ulSize = sizeof(sBlackLevelFileInfo_H2);

    //check file size
    if(ulMemFileSize != ulSize)
    {
        LOG_MSG(db_HAL_WARPING, "Memory %d File Size Error (File %d, Need %d)\n", ucIndex, ulMemFileSize, ulSize);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_INCORRECT_FILE_SIZE;
    }

    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_PASS;
    }

    //check checksum
    if(palGeo_MemoryFileVerifyChecksum(eGFT_AP_BLACKLEVEL, pucData) != ePAL_GEO_EXEC_CODE_PASS)
    {
        ASSERT_ALWAYS();
        free(pucData);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FILE_CHECKSUM_ERROR;
    }

    sBlackLevelFileInfo_H2 *sBlackLevelInfo = (sBlackLevelFileInfo_H2 *)pucData;
    LOG_MSG(db_HAL_WARPING, "0x%08X Header 0x%04X\n",         offsetof(sBlackLevelFileInfo_H2, Header),          sBlackLevelInfo->Header);
    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag 0x%02X\n",     offsetof(sBlackLevelFileInfo_H2, ucEnableFlag),    sBlackLevelInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",             offsetof(sBlackLevelFileInfo_H2, uc3DFlag),        sBlackLevelInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",              offsetof(sBlackLevelFileInfo_H2, ucWidth),         sBlackLevelInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",             offsetof(sBlackLevelFileInfo_H2, ucHeight),        sBlackLevelInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table %d %d %d\n",        offsetof(sBlackLevelFileInfo_H2, aucTable),        sBlackLevelInfo->aucTable[0], sBlackLevelInfo->aucTable[1], sBlackLevelInfo->aucTable[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X BiasPalette %d %d %d\n",  offsetof(sBlackLevelFileInfo_H2, aucBiasPalette),  sBlackLevelInfo->aucBiasPalette[0], sBlackLevelInfo->aucBiasPalette[1], sBlackLevelInfo->aucBiasPalette[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X EGBCT %d %d %d\n",        offsetof(sBlackLevelFileInfo_H2, aucEGBIASCT),     sBlackLevelInfo->aucEGBIASCT[0], sBlackLevelInfo->aucEGBIASCT[1], sBlackLevelInfo->aucEGBIASCT[2]);

    //check Header
    if(sBlackLevelInfo->Header != HICC2_GEO_FILE_HEADER)
    {
        ASSERT_ALWAYS();
        free(pucData);
        LOG_MSG(db_HAL_WARPING, "Blacklevel file header error 0x%04X\n", sBlackLevelInfo->Header);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_FAIL;
    }

    //check ucEnableFlag
    if(sBlackLevelInfo->ucEnableFlag == 0)  //disable
    {
        halWarping_BlackLevel_Enable(FALSE);
        free(pucData);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else if(sBlackLevelInfo->ucEnableFlag != WARPING_TYPE_AP && sBlackLevelInfo->ucEnableFlag != WARPING_TYPE_AP_MYSTIQUE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return ePAL_GEO_EXEC_CODE_PASS;
    }

    //check panel
    if(sBlackLevelInfo->uc3DFlag == FALSE)  //black level data is 2D
    {
        if(halWarpOSD_PanelID_Get() != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blacklevel %d %d\n", sBlackLevelInfo->uc3DFlag, halWarpOSD_PanelID_Get());
            eResult |= halWarping_BlackLevel_Enable(FALSE);
            free(pucData);
            PALGEO_SEMAPHORE_GIVE_ONLY;
            return ePAL_GEO_EXEC_CODE_PASS;
        }
    }
    else  //black level data is 3D
    {
        if(halWarpOSD_PanelID_Get() == PANEL_2D_OUTPUT)  //not 3D panel  //H2PF_Simon_0038
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blacklevel %d %d\n", sBlackLevelInfo->uc3DFlag, halWarpOSD_PanelID_Get());
            eResult |= halWarping_BlackLevel_Enable(FALSE);
            free(pucData);
            PALGEO_SEMAPHORE_GIVE_ONLY;
            return ePAL_GEO_EXEC_CODE_PASS;
        }
    }


    //LOG_MSG(db_HAL_WARPING, "Need Rewrite Check %d %d\n", m_sPalGeoInfo.ucCurrent_Blacklevel_Data_Index, ucIndex);

    INT32 Start = TMO_GetSysRunTime();

    #if 0
    if(m_sPalGeoInfo.ucCurrent_Blacklevel_Data_Index == ucIndex)  //Data already write into CPU
    {
        eResult |= halWarping_BlackLevel_Enable(TRUE);
    }
    else
    {
        for(UINT8 ucIndex = 0 ; ucIndex < 16 ; ucIndex++)
        {
            UINT8 ucRed = sBlackLevelInfo->aucBiasPalette[ucIndex*3+0] ;
            UINT8 ucGreen = sBlackLevelInfo->aucBiasPalette[ucIndex*3+1] ;
            UINT8 ucBlue = sBlackLevelInfo->aucBiasPalette[ucIndex*3+2] ;
            eResult |= halWarping_BlackLevel_Palette_Set(ucIndex, ucRed, ucGreen, ucBlue);
        }
        eResult |= halWarping_BlackLevel_Table_Set(sBlackLevelInfo->aucTable);

        m_sPalGeoInfo.ucCurrent_Blacklevel_Data_Index = ucIndex;
    }
    #endif

    if(eType == eLAT_BLACKLEVEL_PALETTE || eType == eLAT_ALL)
    {
        for(UINT8 ucIndex = 0 ; ucIndex < 16 ; ucIndex++)
        {
            UINT8 ucRed   = sBlackLevelInfo->aucBiasPalette[ucIndex*3+0] ;
            UINT8 ucGreen = sBlackLevelInfo->aucBiasPalette[ucIndex*3+1] ;
            UINT8 ucBlue  = sBlackLevelInfo->aucBiasPalette[ucIndex*3+2] ;
            eResult |= halWarping_BlackLevel_Palette_Set(ucIndex, ucRed, ucGreen, ucBlue);
        }
    }
    else if(eType >= eLAT_BLACKLEVEL_PALETTE0 && eType <= eLAT_BLACKLEVEL_PALETTE15)
    {
        UINT8 ucRed   = sBlackLevelInfo->aucBiasPalette[(eType-eLAT_BLACKLEVEL_PALETTE0)*3+0] ;
        UINT8 ucGreen = sBlackLevelInfo->aucBiasPalette[(eType-eLAT_BLACKLEVEL_PALETTE0)*3+1] ;
        UINT8 ucBlue  = sBlackLevelInfo->aucBiasPalette[(eType-eLAT_BLACKLEVEL_PALETTE0)*3+2] ;
        eResult |= halWarping_BlackLevel_Palette_Set((eType-eLAT_BLACKLEVEL_PALETTE0), ucRed, ucGreen, ucBlue);
    }

    //check resolution               //A35G2_Simon_0120 Start
    UINT16 uiApplyWidth  = sBlackLevelInfo->ucWidth;
    UINT16 uiApplyHeight = sBlackLevelInfo->ucHeight;

    //if file data error, use panel resolution
    if(uiApplyWidth  > MAX_BLACKLEVEL_WIDTH ||
       uiApplyHeight > MAX_BLACKLEVEL_HEIGHT
    )
    {
        //use panel resolution
        uiApplyWidth  = m_sPalGeoInfo.uiWarp_HW;
        uiApplyHeight = m_sPalGeoInfo.uiWarp_VW;

        //OPD
        UINT8 aucString[256];
        snprintf((char *)aucString, sizeof(aucString), "BlacklevelMemory%d,%dx%d,%dx%d", ucIndex+1
                                                                                       , sBlackLevelInfo->ucWidth
                                                                                       , sBlackLevelInfo->ucHeight
                                                                                       , uiApplyWidth
                                                                                       , uiApplyHeight);

        halWarping_OPDEvent(aucString);
        LOG_MSG(db_HAL_WARPING, "%s\n", aucString);
    }

    if(eType == eLAT_BLACKLEVEL_TABLE || eType == eLAT_ALL)
    {
        eResult |= halWarping_BlackLevel_Table_Set(sBlackLevelInfo->aucTable, uiApplyWidth, uiApplyHeight);    //A35G2_Simon_0120 End
    }

    halWarping_BlackLevel_Enable(TRUE);

    INT32 End = TMO_GetSysRunTime();

    LOG_MSG(db_HAL_WARPING, "\n%s end time %d\n", __func__, End-Start);

    free(pucData);

    PALGEO_SEMAPHORE_GIVE;

    return eResult;
}

ePAL_GEO_EXEC_CODE palGeo_SetWarp_AutoFilter(UINT8 ucAutoFilter, UINT8 *ucHorzFilter,UINT8 *ucVertFilter)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halWarping_SetWarp_AutoFilter(ucAutoFilter, ucHorzFilter, ucVertFilter);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_SetWarp_ManualFilter(UINT8 ucHorzFilter, UINT8 ucVertFilter)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halWarping_SetWarp_ManualFilter(ucHorzFilter, ucVertFilter);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_SetWarp_AutoFilterParameter(UINT8 ucAutoFilter, UINT8 *ucHorzFilter,UINT8 *ucVertFilter)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

#ifdef SCALER_C821_C789

    eRet = halWarping_SetWarp_AutoFilterParameter(ucAutoFilter, ucHorzFilter, ucVertFilter);

#else

    LOG_GEO_FUNC_UNSUPPORTED;

#endif

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_SetWarp_ManualFilterParameter(UINT8 ucHorzFilter, UINT8 ucVertFilter)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

#ifdef SCALER_C821_C789

    eRet = halWarping_SetWarp_ManualFilterParameter(ucHorzFilter, ucVertFilter);

#else

    LOG_GEO_FUNC_UNSUPPORTED;

#endif


    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AP_Set_Interpolation_Lut_V_Index(UINT8 ucVertFilterIndex)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

#ifdef SCALER_C821_C789

    eRet = halWarp_AP_Set_Interpolation_Lut_V_Index(ucVertFilterIndex);

#else

    LOG_GEO_FUNC_UNSUPPORTED;

#endif

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AP_Set_Interpolation_Lut_H_Index(UINT8 ucHorzFilterIndex)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

#ifdef SCALER_C821_C789

    eRet = halWarp_AP_Set_Interpolation_Lut_H_Index(ucHorzFilterIndex);

#else

    LOG_GEO_FUNC_UNSUPPORTED;

#endif

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_InterpolationSet(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_InterpolationSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_H_Keystone_Set(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_KEYSTONE_H, &Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_V_Keystone_Set(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_KEYSTONE_V, &Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_H_PINBARREL_Set(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_PINBARREL_H, &Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_V_PINBARREL_Set(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_PINBARREL_V, &Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_4corner_TopLeft_X_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_TL_X, &Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_4corner_TopLeft_Y_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_TL_Y, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_4corner_TopRight_X_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_TR_X, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_4corner_TopRight_Y_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_TR_Y, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_4corner_BottomLeft_X_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_BL_X, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_4corner_BottomLeft_Y_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_BL_Y, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_4corner_BottomRight_X_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_BR_X, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_4corner_BottomRight_Y_Set(UINT16 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_4C_BR_Y, (UINT8 *)&Value);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_4corner_Reset(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    INT16 aiData[8] = {0};

    PALGEO_SEMAPHORE_TAKE;

    aiData[0] = _4CORNER_TL_HORZ_DEFAULT_VALUE;
    aiData[1] = _4CORNER_TL_VERT_DEFAULT_VALUE;
    aiData[2] = _4CORNER_TR_HORZ_DEFAULT_VALUE;
    aiData[3] = _4CORNER_TR_VERT_DEFAULT_VALUE;
    aiData[4] = _4CORNER_BL_HORZ_DEFAULT_VALUE;
    aiData[5] = _4CORNER_BL_VERT_DEFAULT_VALUE;
    aiData[6] = _4CORNER_BR_HORZ_DEFAULT_VALUE;
    aiData[7] = _4CORNER_BR_VERT_DEFAULT_VALUE;

    halWarp_GeometryParameterSet(eWARP_EVENT_4C_ALL, (UINT8 *)&aiData[0]);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_KeystonePinbarrel4CornerSet(UINT8 *pcKeystoneData,
                                                                  UINT8 *pcPinbarrelData,
                                                                  UINT8 *pc4CornerData)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
	halWarp_GeometryParameterSet(eWARP_EVENT_KEYSTONE_HV, pcKeystoneData);
	halWarp_GeometryParameterSet(eWARP_EVENT_PINBARREL_HV, pcPinbarrelData);
	halWarp_GeometryParameterSet(eWARP_EVENT_4C_ALL, pc4CornerData);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_WarpingParameterAllGet(UINT8 *pcData)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterGet(eWARP_EVENT_WARP_PARA, (void *)pcData);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Warp_Off(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
    halWarp_GeometrySet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_Off(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_T_Enable(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_T_ENABLE, &Value);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_T_Settings(UINT16 Start, UINT16 Width)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_T_ST, &Start);
    halWarp_BlendingParameterSet(eBLENDING_EVENT_T_WIDTH, &Width);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_Egb_B_Enable(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_B_ENABLE, &Value);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_B_Settings(UINT16 Start, UINT16 Width)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_B_ST, &Start);
    halWarp_BlendingParameterSet(eBLENDING_EVENT_B_WIDTH, &Width);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_L_Enable(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_L_ENABLE, &Value);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_L_Settings(UINT16 Start, UINT16 Width)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_L_ST, &Start);
    halWarp_BlendingParameterSet(eBLENDING_EVENT_L_WIDTH, &Width);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_Egb_R_Enable(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_R_ENABLE, &Value);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Egb_R_Settings(UINT16 Start, UINT16 Width)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_R_ST, &Start);
    halWarp_BlendingParameterSet(eBLENDING_EVENT_R_WIDTH, &Width);
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_BlendGamma(UINT8 Value)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_GAMMA, &Value);
    halWarp_BlendingGammaSet();
    halBasicBlendSettingSet();  //H2PF_Simon_0165
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_EgbParameterGammaSet(UINT8 *pucEgbParameter,
                                                         UINT8 *pucBlendingGamma)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterSet(eBLENDING_EVENT_ALL, pucEgbParameter);
    halWarp_BlendingParameterSet(eBLENDING_EVENT_GAMMA, pucBlendingGamma); //A70LV_Larry_0171
    halWarp_BlendingSet();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_BlendParameterAllGet(UINT8 *pcData)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halWarp_BlendingParameterGet(eBLENDING_EVENT_ALL, (void*)pcData);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}



UINT8 palGeo_Color_Uniformity_Init_Status_Get(void)
{
    return m_sPalGeoInfo.m_ucACU_Init_Status;
}

ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Init_Status_Set(UINT8 ucACU_Init)
{
    PALGEO_SEMAPHORE_TAKE;
    m_sPalGeoInfo.m_ucACU_Init_Status = ucACU_Init;
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Enable_Set(UINT8 ucEnable)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "Color_Uniformity_Enable_Set %d\r\n", ucEnable);

    PALGEO_SEMAPHORE_TAKE;
    if(ucEnable == 0)
    {
        eRet = halWarping_Color_Uniformity_Disable();
    }
    else if(ucEnable == 1)
    {
        eRet = halWarping_Color_Uniformity_Enable();
    }
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

UINT8 palGeo_Color_Uniformity_Enable_Get(void)
{
    UINT8 Enable = 0;

    PALGEO_SEMAPHORE_TAKE;
    Enable = halWarping_Color_Uniformity_Enable_Get();
    PALGEO_SEMAPHORE_GIVE;

    return Enable;
}


//halWarping_Color_Uniformity_Table_Set
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_File_Apply(char *pcFileName)
{
    FILE *pFile = NULL;
    pFile = fopen(pcFileName, "rb");
    UINT8 ucCnt = 0;
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_FAIL;

	PALGEO_SEMAPHORE_TAKE;
    if(pFile == NULL)
    {
        //ASSERT_ALWAYS();
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return palGeo_ExeResult(eHAL_WARPING_EXEC_CODE_FAIL);
    }

    UINT8 *pucData = (UINT8 *)malloc(WARPING_COLOR_UNIFORMITY_SIZE);
    if(pucData == NULL)
    {
		fclose(pFile);
        ASSERT_ALWAYS();
        PALGEO_SEMAPHORE_GIVE_ONLY;
        return palGeo_ExeResult(eHAL_WARPING_EXEC_CODE_MALLOC_FAIL);
    }
    memset(pucData, 0xFF, WARPING_COLOR_UNIFORMITY_SIZE);
    if(fread(pucData, 1, WARPING_COLOR_UNIFORMITY_SIZE, pFile) != WARPING_COLOR_UNIFORMITY_SIZE)
    {
        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) fread fail\n", __FUNCTION__, __LINE__);
    }

    for( ; ucCnt < 3; ucCnt++ )
    {
        eRet = halWarping_Color_Uniformity_Set( pucData, WARPING_COLOR_UNIFORMITY_SIZE );
        if( eRet == eHAL_WARPING_EXEC_CODE_PASS )
        {
            break;
        }
    }

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) %s\n", __FUNCTION__, __LINE__, pcFileName);

    free(pucData);
	fclose(pFile);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Data_Save(UINT8 ucFileNo,  UINT8 *pucData)
{
    FILE *pFile = NULL;
    UINT8 ucFile_Valid = eACU_FILE_INVALID;

    PALGEO_SEMAPHORE_TAKE;
    LOG_MSG(db_HAL_WARPING, "(func:%s, line%d) Save %d\r\n", __FUNCTION__, __LINE__, ucFileNo);

    switch( ucFileNo )
    {
        case eACU_SAVE_FILE_0:  pFile = fopen(COLOR_UNIFORMITY_DATA0, "wb");  break;
        case eACU_SAVE_FILE_1:  pFile = fopen(COLOR_UNIFORMITY_DATA1, "wb");  break;
        case eACU_SAVE_FILE_2:  pFile = fopen(COLOR_UNIFORMITY_DATA2, "wb");  break;
        case eACU_SAVE_FILE_3:  pFile = fopen(COLOR_UNIFORMITY_DATA3, "wb");  break;

        default:
            PALGEO_SEMAPHORE_GIVE_ONLY
            LOG_MSG(db_HAL_WARPING, "%s invalid file no.: %d\r\n", __FUNCTION__, ucFileNo);
            return ePAL_GEO_EXEC_CODE_FAIL;
    }

    if(pFile == NULL)
    {
        ASSERT_ALWAYS();
        PALGEO_SEMAPHORE_GIVE_ONLY
        return ePAL_GEO_EXEC_CODE_FILE_ERROR;
    }

    fwrite(pucData , 1 , WARPING_COLOR_UNIFORMITY_SIZE , pFile);
    fclose(pFile);
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Reset(void) //H2PF_Simon_0054
{
    #ifdef SCALER_FPGA_F34
    {
        PALGEO_SEMAPHORE_TAKE;
        halWarping_ColorUniformityReset();
        PALGEO_SEMAPHORE_GIVE;
    }
    #else
    {
    }
    #endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Init(void)
{
    ePAL_GEO_EXEC_CODE eResult= ePAL_GEO_EXEC_CODE_PASS;
    UINT8  ucType = 0;
    UINT32 dwID = eACU_FILE_INVALID;

    if(palDataMgr_Access_Color_Uniformity_Table_Offset_Get(&dwID) == eEXEC_CODE_PASS)
    {
        switch( dwID )
        {
            case eACU_FILE_0_VALID:
                palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA0);
                break;

            case eACU_FILE_1_VALID:
                palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA1);
                break;

            case eACU_FILE_2_VALID:
                palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA2);
                break;

            case eACU_FILE_3_VALID:
                palGeo_Color_Uniformity_File_Apply(COLOR_UNIFORMITY_DATA3);
                break;

            default:
                palGeo_Color_Uniformity_Enable_Set(FALSE);
                LOG_MSG(db_HAL_WARPING, "palGeo_Color_Uniformity_Init(), Fail, File ID invalid: %d\r\n", dwID); //G100_Tim_0018, mod
                eResult = ePAL_GEO_EXEC_CODE_FAIL;
                break;
        }
    }
    else
    {
        palGeo_Color_Uniformity_Enable_Set(FALSE);
    }

    palGeo_Color_Uniformity_Init_Status_Set(WARPING_ACU_INIT_DONE); //G100_Tim_0020, add

	if(palDataMgr_CU_Data_Enable_Get() && (palDataMgr_CU_Data_Status_Get()))//A35G2_Alan_0007
        palGeo_Color_Uniformity_Enable_Set(TRUE);
	else
		palGeo_Color_Uniformity_Enable_Set(FALSE);

    //LOG_MSG(db_HAL_WARPING, "palGeo_Color_Uniformity_Init() %d, File: %d\r\n", ((eResult == ePAL_GEO_EXEC_CODE_FAIL) ? "OK":"NG") , dwID);

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_ColorUniformityApplyByOrientation(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    #ifdef SCALER_FPGA_F34

    PALGEO_SEMAPHORE_TAKE;
    halWarping_ColorUniformityApplyByOrientation();
    PALGEO_SEMAPHORE_GIVE;

    #endif

    return palGeo_ExeResult(eResult);
}




ePAL_GEO_EXEC_CODE palGeo_ApLinkFlag_Set(BOOL bApLink)
{
    PALGEO_SEMAPHORE_TAKE;
    LOG_MSG(db_HAL_WARPING, "(func:%s,line:%d) %d\n", __FUNCTION__, __LINE__, bApLink);

    SHM_BLEND_AP_LINK_STATE = bApLink;   //H2PF_Simon_0040
    halWarping_TwistLinkFlag_Set(bApLink);

    palLANProcSendToLAN(edcBLENDING_AP_LINK_STATUS);  //HICC2_Simon_0027

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

BOOL palGeo_ApLinkFlag_Get(void)
{
    return halWarping_TwistLinkFlag_Get();
}


ePAL_GEO_EXEC_CODE palGeo_ClearWarpingTable(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_ClearWarpingTable();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_DBD_Blending_Init(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_DBD_Blending_Init();
    PALGEO_SEMAPHORE_GIVE;
#endif /* SCALER_FPGA_F34 */

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingTable_Set(UINT16 uiWidth, UINT16 uiHeight, UINT8 *pucDBD_Data)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_DBD_BlendingTable_Set(uiWidth, uiHeight, pucDBD_Data);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingReset(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_DBD_Blending_Reset();
    PALGEO_SEMAPHORE_GIVE;
#endif /* SCALER_FPGA_F34 */

    return palGeo_ExeResult(eResult);
}


ePAL_GEO_EXEC_CODE palGeo_DBD_Blending_EnableGet(UINT8 *pucEnable)  //H2PF_Simon_0035   //H2PF_Simon_0086
{
    PALGEO_SEMAPHORE_TAKE;
    *pucEnable = halWarping_DBD_Blending_EnableGet();
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingEnable(UINT8 ucEnable)  //H2PF_Simon_0086
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) %d\r\n", __FUNCTION__, __LINE__, ucEnable);

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_DBD_Blending_Enable(ucEnable);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingDataClear(void)
{
#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    halWarping_DBD_Data_Index_Set(DBD_BLENDING_NO_DATA);
    PALGEO_SEMAPHORE_GIVE;
#endif /* SCALER_FPGA_F34 */

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_DBD_ClearBlendingTable(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_DBD_ClearBlendingTable();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_ClearBlackLevel(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_clearBlackLevel();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Init(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_BlackLevel_Init();
    PALGEO_SEMAPHORE_GIVE;
#endif /* SCALER_FPGA_F34 */

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Enable(UINT8 ucEnable)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_BlackLevel_Enable(ucEnable);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Reset(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halWarping_BlackLevel_Reset();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_BlackLevelDataClear(void)
{
#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    halWarping_BlackLevel_Data_Index_Set(BLACKLEVEL_NO_DATA);
    PALGEO_SEMAPHORE_GIVE;
#endif /* SCALER_FPGA_F34 */

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Palette_Set(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
#ifdef SCALER_C341
    PALGEO_SEMAPHORE_TAKE;
    halWarping_BlackLevel_Palette_Set(ucPaletteIdx, ucRed, ucGreen, ucBlue);
    PALGEO_SEMAPHORE_GIVE;
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_MemoryFileCopy(ePAL_GEO_FUNCTION_TYPE eFunc, UINT8 SrcIndex, UINT8 DestIndex)
{
    char src[GEO_MEMORY_MAX_FILENAME_LENGTH];
    char dest[GEO_MEMORY_MAX_FILENAME_LENGTH];

    palGeo_MemoryFileNameGet(eFunc, SrcIndex, src);
    palGeo_MemoryFileNameGet(eFunc, DestIndex, dest);
    SYSTEM_CALL("cp -f %s %s;sync", src, dest);

    return ePAL_GEO_EXEC_CODE_PASS;
}



INT16 palGeo_HResGet(void)
{
    return halWarping_HResGet();
}


INT16 palGeo_VResGet(void)
{
    return halWarping_VResGet();
}


ePAL_GEO_EXEC_CODE palGeo_Save_BasicWarpMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halBasicSaveWarpMemory(CM_Index - 1);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_Apply_BasicWarpMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halBasicApplyWarpMemory(CM_Index - 1);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}


ePAL_GEO_EXEC_CODE palGeo_Save_BasicBlendMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halBasicSaveBlendMemory(CM_Index - 1);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_Apply_BasicBlendMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halBasicApplyBlendMemory(CM_Index - 1);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_Save_AdvMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halAdvSaveMemory(CM_Index - 1);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}

ePAL_GEO_EXEC_CODE palGeo_Apply_AdvMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eResult = halAdvApplyMemory(CM_Index - 1);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eResult);
}


ePAL_GEO_EXEC_CODE palGeo_OnePalette_Set(UINT8 PaletteIdx, UINT8 Red, UINT8 Green, UINT8 Blue)
{
    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_Palette_Group_Set(COLOR_PALETTE__AP);
    halWarpOSD_Palette_Set(PaletteIdx, Red, Green, Blue);
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_AllPalette_Set(UINT8 *Red, UINT8 *Green, UINT8 *Blue)
{
    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_Palette_Group_Set(COLOR_PALETTE__AP);
    halWarpOSD_Palette_SetAll(Red, Green, Blue);
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_ClearOSD(eWARPOSD_INSERT_LOCATION eLocation)   //HICC2_Simon_0008
{

    //halWarpOSD_Palette_Group_Set(COLOR_PALETTE__AP);
#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_ClearOSD((eWARPOSD_DRAW_TYPE)eLocation);
    PALGEO_SEMAPHORE_GIVE;
#else

    if( utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)//A65_OPTOMA_CL_0010
    {
        if(utilWarp_GetOsdPatternType() == PAT_TYPE__TESTPATTERN_BLACK_BORDER_MULTIPLESELECT ||
           utilWarp_GetOsdPatternType() == PAT_TYPE__DRAW_CIRCLE_GRID)
        {
            palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__OFF);
        }
        else
        {
            PALGEO_SEMAPHORE_TAKE;
            utilWarp_ShowOsdPattern(PAT_TYPE__OFF);
            PALGEO_SEMAPHORE_GIVE;
        }
    }

    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_AP_ClearOSD( (eLocation == eWIL_BEFORE_WARP) ? eWDT_BEFORE_WARP : eWDT_AFTER_WARP);
    PALGEO_SEMAPHORE_GIVE;
#endif


    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DrawRect(eWARPOSD_INSERT_LOCATION eLocation,
                                   UINT16 uiPosX,
                                   UINT16 uiPosY,
                                   UINT16 uiWidth,
                                   UINT16 uiHeight,
                                   UINT16 uiColorIdx)
{
    INT16 iHRes = halWarping_HResGet();
    INT16 iVRes = halWarping_VResGet();

    LOG_MSG(db_HAL_WARPING, "RECT Data = %d %d %d %d %d %d\n", eLocation,
                                                               uiPosX,
                                                               uiPosY,
                                                               uiWidth,
                                                               uiHeight,
                                                               uiColorIdx);

    if( eLocation >= eWIL_INVALID ||
        uiPosX  >= iHRes ||
        uiPosY  >= iVRes ||
        uiWidth  > iHRes ||
        uiHeight > iVRes ||
        uiColorIdx > 255
      )
    {
        return ePAL_GEO_EXEC_CODE_FAIL;
    }

    #ifdef SCALER_FPGA_F34  //H2PF_Simon_0038
    {
        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawRect(eLocation,
                            uiPosX,
                            uiPosY,
                            uiWidth,
                            uiHeight,
                            uiColorIdx);
        PALGEO_SEMAPHORE_GIVE;
    }
    #else
    {
        if(eLocation == eWIL_AFTER_WARP)  //on OSD  //A65_OPTOMA_CL_0006
        {
            palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_RECT);   //HICC2_Simon_0008
        }

        PALGEO_SEMAPHORE_TAKE;
        halWarping_AP_DrawRect(eLocation,
                               uiPosX,
                               uiPosY,
                               uiWidth,
                               uiHeight,
                               uiColorIdx);
        PALGEO_SEMAPHORE_GIVE;
    }
    #endif

    return ePAL_GEO_EXEC_CODE_PASS;

}



ePAL_GEO_EXEC_CODE palGeo_DrawMultiRect(eWARPOSD_INSERT_LOCATION eLocation,
                                                UINT16 uiPosX,
                                                UINT16 uiPosY,
                                                UINT16 uiWidth,
                                                UINT16 uiHeight,
                                                UINT16 uiColorIdx)
{

#if 0

    PALGEO_SEMAPHORE_TAKE;

    if(eLocation == eWIL_AFTER_WARP)  //on OSD  //A65_OPTOMA_CL_0007
    {
        halAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_RECTS);
    }

    for(UINT16 uiIndex = 0 ; uiIndex < sRectsInfo->ucTotalNumber ; uiIndex++ )
    {
        sWARPOSD_RECTS_DRAW_INFO *sSingleRectAddr = sRectsInfo + uiIndex;  //A65_OPTOMA_CL_0007
        sWARPOSD_RECT_DRAW_INFO *sRectInfo = &sSingleRectAddr->sRect ;

        if( sRectInfo->ucLayerMode >= eWDT_INVALID ||
            sRectInfo->iX < 0 ||
            sRectInfo->iY < 0 ||
            sRectInfo->iX >= halWarping_HResGet() ||
            sRectInfo->iY >= halWarping_VResGet() ||
            sRectInfo->iWidth  > halWarping_HResGet() ||
            sRectInfo->iHeight > halWarping_VResGet()
           )
        {
            LOG_MSG(db_HAL_WARPING, "Error RECT Data = %d %d %d %d %d %d\n", sRectInfo->ucLayerMode,
                                                                             sRectInfo->iX,
                                                                             sRectInfo->iY,
                                                                             sRectInfo->iWidth,
                                                                             sRectInfo->iHeight,
                                                                             sRectInfo->iColorIndex );

            continue;
        }
        LOG_MSG(db_HAL_WARPING, "Rect Info = %d %d %d %d %d %d\n", sRectInfo->ucLayerMode,
                                                                   sRectInfo->iX,
                                                                   sRectInfo->iY,
                                                                   sRectInfo->iWidth,
                                                                   sRectInfo->iHeight,
                                                                   sRectInfo->iColorIndex );

        halWarping_AP_DrawRect(sRectInfo->ucLayerMode,
                               sRectInfo->iX,
                               sRectInfo->iY,
                               sRectInfo->iWidth,
                               sRectInfo->iHeight,
                               sRectInfo->iColorIndex);

    }

    PALGEO_SEMAPHORE_GIVE;
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DrawLine(eWARPOSD_INSERT_LOCATION eLocation,
                                         UINT16 uiLeft,
                                         UINT16 uiTop,
                                         UINT16 uiRight,
                                         UINT16 uiBottom,
                                         UINT32 uiColorIndex)  //H2PF_Simon_0038
{
    PALGEO_SEMAPHORE_TAKE;

    INT16 iHRes = halWarping_HResGet();
    INT16 iVRes = halWarping_VResGet();

    LOG_MSG(db_HAL_WARPING, "Line Data = %u %u %u %u %u 0x%X\n", eLocation,
                                                                 uiLeft,
                                                                 uiTop,
                                                                 uiRight,
                                                                 uiBottom,
                                                                 uiColorIndex);

    if( eLocation >= eWIL_INVALID || eLocation == eWIL_AFTER_WARP ||
        uiRight  >= halWarping_HResGet() ||
        uiBottom >= halWarping_VResGet()
    )
    {
        LOG_MSG(db_HAL_WARPING, "Error Line Data = %u %u %u %u %u 0x%X\n", eLocation,
                                                                           uiLeft,
                                                                           uiTop,
                                                                           uiRight,
                                                                           uiBottom,
                                                                           uiColorIndex);

        if(eLocation == eWIL_AFTER_WARP)
        {
            LOG_MSG(db_HAL_WARPING, "Draw Line not support draw after warp\n");
        }

        return ePAL_GEO_EXEC_CODE_FAIL;
    }


    #ifdef SCALER_FPGA_F34  //H2PF_Simon_0038
    UINT16 NewLeft = 0;
    UINT16 NewTop = 0;
    UINT16 NewRight = 0;
    UINT16 NewBottom = 0;
    if(eLocation == eWDT_BEFORE_WARP)
    {
        if(halWarping_HResGet() > 1920 && halWarping_VResGet() > 1080)
        {
            NewLeft = (uiLeft/2);
            NewTop = (uiTop/2);
            NewRight  = ((uiRight - uiLeft) + 1)/2;
            NewBottom = ((uiBottom - uiTop) + 1)/2;
        }
        else
        {
            NewLeft = uiLeft;
            NewTop = uiTop;
            NewRight  = uiRight - uiLeft;
            NewBottom = uiBottom - uiTop;
        }
    }
    else
    {
        NewLeft = uiLeft;
        NewTop = uiTop;
        NewRight  = uiRight - uiLeft;
        NewBottom = uiBottom - uiTop;
    }

    uiLeft = NewLeft;
    uiTop = NewTop;
    uiRight = NewRight;
    uiBottom = NewBottom;
    #endif

    //only draw on memory (before warp)
    halWarping_AP_DrawLine(eLocation,
                           uiLeft,
                           uiTop,
                           uiRight,
                           uiBottom,
                           (INT32)uiColorIndex
                           );


    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;

}


ePAL_GEO_EXEC_CODE palGeo_DrawMultiLine(eWARPOSD_INSERT_LOCATION eLocation,
                                                UINT16 uiLeft,
                                                UINT16 uiTop,
                                                UINT16 uiRight,
                                                UINT16 uiBottom,
                                                UINT16 uiColorIndex)
{


    return ePAL_GEO_EXEC_CODE_PASS;

}

ePAL_GEO_EXEC_CODE palGeo_DrawCircle(eWARPOSD_INSERT_LOCATION eLocation,
                                            UINT16 uiPosX,
                                            UINT16 uiPosY,
                                            UINT16 uiRadius,
                                            UINT16 uiCircleColorIndex,
                                            UINT16 uiBGColorIndex)
{
    PALGEO_SEMAPHORE_TAKE;

    palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_CIRCLE);   //HICC2_Simon_0008

    halWarping_DrawCircle((eLocation == eWIL_BEFORE_WARP) ? eWDT_BEFORE_WARP : eWDT_AFTER_WARP,
                           uiPosX,
                           uiPosY,
                           uiRadius,
                           uiCircleColorIndex,
                           uiBGColorIndex);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DrawMultiCircle(sWARPOSD_CIRCLE_GRID_DRAW_INFO sCircleGridInfo)    //HICC2_Simon_0026
{
    LOG_MSG(db_HAL_WARPING, "Draw Circle new method !!\n");

    #ifdef SCALER_FPGA_F34
    {
        PALGEO_SEMAPHORE_TAKE;
        INT32 m_nHorzResolution = halWarping_HResGet(); //R70K_AC_0102
        INT32 m_nVertResolution = halWarping_VResGet();

    	uint8 nForeRColor   = sCircleGridInfo.color_red;
    	uint8 nForeGColor   = sCircleGridInfo.color_green;
    	uint8 nForeBColor   = sCircleGridInfo.color_blue;
    	uint16 nRadius      = sCircleGridInfo.radius;
    	uint16 nSpaceX      = sCircleGridInfo.space;     //相鄰兩個圓的圓心，水平間隔 nSpaceX pixel
    	uint16 nSpaceY      = sCircleGridInfo.space;     //相鄰兩個圓的圓心，垂直間隔 nSpaceY pixel
    	uint16 nOffsetX     = sCircleGridInfo.x_offset;  //第一個圓，X 方向 offset 多少開始畫
    	uint16 nOffsetY     = sCircleGridInfo.y_offset;  //第一個圓，Y 方向 offset 多少開始畫
    	uint16 nCircleXS    = sCircleGridInfo.x_start;   //從第幾個圓開始畫 (例如 X 方向總共可以畫 41 個圓，則 nCircleXS 20 表示從第20個圓當起始點開始畫)
    	uint16 nCircleXE    = sCircleGridInfo.x_end;
    	uint16 nCircleYS    = sCircleGridInfo.y_start;   //從第幾個圓開始畫 (例如 Y 方向總共可以畫 23 個圓，則 nCircleYS 11 表示從第11個圓當起始點開始畫)
    	uint16 nCircleYE    = sCircleGridInfo.y_end;
    	uint16 nStepX       = sCircleGridInfo.x_step;
    	uint16 nStepY       = sCircleGridInfo.y_step;
    	BOOL bShowLastSpot  = sCircleGridInfo.bShowLast;
    	BOOL bClearOsd      = sCircleGridInfo.bClearOSD;

        //background
        halWarping_DrawRect(eWDT_AFTER_WARP,
                            0,
                            0,
                            m_nHorzResolution,
                            m_nVertResolution,
                            (UINT16)AP_COLOR_IDX__BLACK);

        halWarpOSD_Palette_Set(USER_PALETTE_INDEX, nForeRColor, nForeGColor, nForeBColor);
        //halWarpOSD_Palette_Update();

        LOG_MSG(db_HAL_WARPING, "DrawCircleGrid R(%d), S(%d %d), O(%d %d), XSXE(%d %d), YSYE(%d %d), (%d %d), C(%d)!!\r\n",
                                nRadius,
                                nSpaceX,    nSpaceY,
                                nOffsetX,   nOffsetY,
                                nCircleXS,  nCircleXE,
                                nCircleYS,  nCircleYE,
                                nStepX,     nStepY,
                                USER_PALETTE_INDEX);


        halWarping_DrawCircleGrid(eWDT_AFTER_WARP,
                                  (nSpaceX*nCircleXS) + nRadius + nOffsetX,   //第一個圓的圓心 X 座標
                                  (nSpaceY*nCircleYS) + nRadius + nOffsetY,   //第一個圓的圓心 Y 座標
                                  nRadius,
                                  nSpaceX,
                                  nSpaceY,
                                  ((nSpaceX*nCircleXE) + nRadius + nOffsetX + nRadius + 1) > (m_nHorzResolution) ? m_nHorzResolution : ((nSpaceX*nCircleXE) + nRadius + nOffsetX + nRadius + 1),  //H2PF_Simon_0031
                                  ((nSpaceY*nCircleYE) + nRadius + nOffsetY + nRadius + 1) > (m_nVertResolution) ? m_nVertResolution : ((nSpaceY*nCircleYE) + nRadius + nOffsetY + nRadius + 1),  //H2PF_Simon_0031
                                  USER_PALETTE_INDEX);


        halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD

        PALGEO_SEMAPHORE_GIVE;

    }
    #else
    {
        PALGEO_SEMAPHORE_TAKE;
        utilWarp_CopyCircleInfo(&sCircleGridInfo);
        PALGEO_SEMAPHORE_GIVE;

    	palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_CIRCLE_GRID);   //HICC2_Simon_0008
    }
    #endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_DrawPNG(eWARPOSD_INSERT_LOCATION eLocation,
                                       UINT16 uiPosX,
                                       UINT16 uiPosY,
                                       UINT8 *FileName)
{
    PALGEO_SEMAPHORE_TAKE;

    LOG_MSG(db_HAL_WARPING, "Start %s\n", __FUNCTION__);
    LOG_MSG(db_HAL_WARPING, "ucLayerMode %d\n", eLocation);
    LOG_MSG(db_HAL_WARPING, "iX %d\n", uiPosX);
    LOG_MSG(db_HAL_WARPING, "iY %d\n", uiPosY);
    LOG_MSG(db_HAL_WARPING, "FileName %s\n", FileName);

    INT16 iWidth, iHeight;
    UINT8 *pauctmpFullColor = (UINT8 *)malloc(halWarping_HResGet() * halWarping_VResGet() * 3 + 1024);
    if(pauctmpFullColor == NULL)
    {
        ASSERT_ALWAYS();
        return ePAL_GEO_EXEC_CODE_MALLOC_FAIL;
    }
    memset(pauctmpFullColor, 0, (halWarping_HResGet() * halWarping_VResGet() * 3 + 1024));

#ifndef SIMULATOR_ISCALER
    //read PNG file
    read_png_file((char*)FileName, (int*)&iWidth, (int*)&iHeight);
    LOG_MSG(db_HAL_WARPING, "\nPNG FileName=%s, Width=%d, Height=%d\n", FileName, iWidth, iHeight);

    //process file
    process_file(pauctmpFullColor, halWarping_HResGet(), halWarping_VResGet());
    LOG_MSG(db_HAL_WARPING, "process_file done\n");

    #if 0
    //clear OSD
    #ifdef SCALER_FPGA_F34  //for compile error
    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);
    #else
    halWarpOSD_ClearOSD();
    #endif
    #endif

    //insert Warp OSD After/Before warp
    if(eLocation == eWIL_BEFORE_WARP)
    {
        #ifdef SCALER_FPGA_F34  //H2PF_Simon_0038
        {
            halWarping_DrawPNG_OnMemoryTest(uiPosX,
                                            uiPosY,
                                            iWidth,
                                            iHeight,
                                            pauctmpFullColor);
        }
        #else
        {
            halWarpOSD_ClearOSD();
            halWarpOSD_AP_ClearOSD(eWDT_AFTER_WARP);   //H2PF_Simon_0029
            halWarping_DrawPNG_OnMemory(uiPosX,
                                        uiPosY,
                                        iWidth,
                                        iHeight,
                                        pauctmpFullColor);
        }
        #endif
    }
    else
    {
        #ifdef SCALER_FPGA_F34  //H2PF_Simon_0038
        {
            halWarping_DrawPNG_OnOSD(uiPosX,
                                     uiPosY,
                                     iWidth,
                                     iHeight,
                                     pauctmpFullColor);
        }
        #else
        {
            halWarpOSD_ClearOSD();
            //palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_PNG);  //H2PF_Simon_0072
            halWarping_WarpOSDModeSet(FALSE);
            halWarping_DrawPNG_OnOSD(uiPosX,
                                     uiPosY,
                                     iWidth,
                                     iHeight,
                                     pauctmpFullColor);
        }
        #endif
    }

#endif /* SIMULATOR_ISCALER */

    free(pauctmpFullColor);

    LOG_MSG(db_HAL_WARPING, "%s End\n", __FUNCTION__);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_DrawMulitSprites(sSPRITES_INFO_DATA *psSpritesInfo)  //H2PF_Simon_0038  //H2PF_Simon_0049
{
    eSHARED_MEMORY_BLOCK eShmBlockSelect = eSB_DATABASE_BLENDING_DATA_0;
    eDATA_SYNC_STATUS eDataSyncFeedback = eDATA_SYNC_STATUS_BLOCK0_RECEIVER_DONE;
    eDATA_SYNC_STATUS eDataSyncFailFeedback = eDATA_SYNC_STATUS_BLOCK0_RECEIVER_FAIL;
    void *shm_ptr_BlendData = SharedMem_CreatorMappingPtrGet(eSB_DATABASE_BLENDING_DATA_0);
    eWARPOSD_DRAW_TYPE etype = eWDT_AFTER_WARP;

    //check Data Sync Status
    eDATA_SYNC_STATUS eDataSyncStatus = eDATA_SYNC_STATUS_INVALID;

    PALGEO_SEMAPHORE_TAKE;

    LOG_MSG(db_HAL_WARPING, "Start %s (Total %d)(Block %d)\n", __FUNCTION__ , psSpritesInfo->iTotalNumber, psSpritesInfo->ucBlock);

    if(psSpritesInfo->ucBlock > 1)
    {
        LOG_MSG(db_HAL_WARPING, "\n\n !!! error data\n\n");
        return eEXEC_CODE_FAIL;
    }

    palDataMgr_Data_Access(edcDATA_SYNC_STATUS, edaREAD, &eDataSyncStatus);

    if(psSpritesInfo->ucBlock == 0 && eDataSyncStatus != eDATA_SYNC_STATUS_BLOCK0_SENDER_DONE)
    {
        LOG_MSG(db_HAL_WARPING, "\n\n !!! Sprite Data not ready (%d)\n\n", eDataSyncStatus);
        return eEXEC_CODE_FAIL;
    }
    else if(psSpritesInfo->ucBlock == 1 && eDataSyncStatus != eDATA_SYNC_STATUS_BLOCK1_SENDER_DONE)
    {
        LOG_MSG(db_HAL_WARPING, "\n\n !!! Sprite Data not ready (%d)\n\n", eDataSyncStatus);
        return eEXEC_CODE_FAIL;
    }

    if(psSpritesInfo->ucBlock == 0)
    {
        eShmBlockSelect = eSB_DATABASE_BLENDING_DATA_0;
        shm_ptr_BlendData = SharedMem_CreatorMappingPtrGet(eShmBlockSelect);
        eDataSyncFeedback = eDATA_SYNC_STATUS_BLOCK0_RECEIVER_DONE;
        eDataSyncFailFeedback = eDATA_SYNC_STATUS_BLOCK0_RECEIVER_FAIL;
    }
    else if(psSpritesInfo->ucBlock == 1)
    {
        eShmBlockSelect = eSB_DATABASE_BLENDING_DATA_1;
        shm_ptr_BlendData = SharedMem_CreatorMappingPtrGet(eShmBlockSelect);
        eDataSyncFeedback = eDATA_SYNC_STATUS_BLOCK1_RECEIVER_DONE;
        eDataSyncFailFeedback = eDATA_SYNC_STATUS_BLOCK1_RECEIVER_FAIL;
    }

    #ifdef SCALER_ICHIPS
    PALGEO_SEMAPHORE_GIVE;
    #endif

    for(UINT32 index = 0 ; index < psSpritesInfo->iTotalNumber ; index++)
    {
        void *shm_ptr_CurrentBlendData = (char *)shm_ptr_BlendData + (index * sizeof(sSPRITE_INFO_DATA));

        //utilDatabase_ReadData(eShmBlockSelect, (void *)psSingleSpriteData, index);
#if 1
        LOG_MSG(db_HAL_RESERVED16, "SD%d (%d - %d - %d %d %d)\n", index
                                                                , eDataSyncStatus
                                                                , *((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, ucLayerMode))
                                                                , *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nPid))
                                                                , *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nTop))
                                                                , *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nLeft))
                                                                //, psSingleSpriteData->sPixmapData.nPID
                                                                //, psSingleSpriteData->sPixmapData.nWidth
                                                                //, psSingleSpriteData->sPixmapData.nHeight
                                                                );
#endif

        if(*((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, ucLayerMode)) == eWDT_AFTER_WARP)
        {
            if(index == 0 && *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nPid)) == 0)
            {
                //halWarping_DrawSpritesInit();
            }


            #ifdef SCALER_FPGA_F34
            halWarping_DrawSpritesOnOSDModify(1,
                                        *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nPid)),
                                        *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nTop)),
                                        *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nLeft)),
                                        (sPixmapData *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, sData)));
            #else

            palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_SPRITES);  //A65_OPTOMA_CL_0006   //HICC2_Simon_0008


            PALGEO_SEMAPHORE_TAKE;
            halWarping_DrawSpritesOnOSD(1,
                                        *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nPid)),
                                        *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nTop)),
                                        *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nLeft)),
                                        (sPixmapData *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, sData)));
            PALGEO_SEMAPHORE_GIVE;
            #endif
        }
        else if(*((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, ucLayerMode)) == eWDT_BEFORE_WARP)
        {
            #ifdef SCALER_FPGA_F34
            halWarping_SetScalerOsdSwitch(SCALER_OSD_PALETTE__DRAW_TYPE);
            etype = eWDT_BEFORE_WARP;
            halWarping_DrawSpritesOnMemoryModify(1,
                                           *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nPid)),
                                           *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nTop)),
                                           *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nLeft)),
                                           (sPixmapData *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, sData)));
            #else
            PALGEO_SEMAPHORE_TAKE;
            halWarping_DrawSpritesOnMemory(1,
                                           *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nPid)),
                                           *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nTop)),
                                           *(INT16 *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, nLeft)),
                                           (sPixmapData *)((char *)shm_ptr_CurrentBlendData + offsetof(sSPRITE_INFO_DATA, sData)));
            PALGEO_SEMAPHORE_GIVE;
            #endif
        }

    }

    palDataMgr_Access_DataSyncStatus(edaWRITE_THROUGH_WITH_ACTION, &eDataSyncFeedback);  //H2PF_Simon_0062

    //C821+C789 平台 : 每次收到 sprite command 都直接畫
    //F34 平台 : 收到 sprite command 後，先存起來，直到收到一筆 Total Number 不等於 500 時，
    //           再全部一次畫完 (Total Number 為 500 表示 sprite 還沒送完，還會有下一筆 sprite command)

    #ifdef SCALER_FPGA_F34
    //draw all
    if(psSpritesInfo->iTotalNumber != 500)  //500 means there is more packets on the way
    {	//A70LK_Casper_0004
        UINT8 ucTwistPattern = 0;
        palDataMgr_Data_Access(edcTWIST_PATTERN, edaREAD, &ucTwistPattern);
        halWarping_SetShowScalerOsdAck(ucTwistPattern);
        LOG_MSG(db_HAL_WARPING, "Twist Test Pattern = %d\n", ucTwistPattern);

        if(etype == eWDT_BEFORE_WARP)
        {
            //halWarping_ShowSpriteBuffer(etype);
            //halWarping_SetShowScalerOsdAck(1);
            halWarpOSD_ClearOSD(eWDT_BEFORE_WARP);  //A70LK_CL_0007
            halWarping_ShowSpriteOnMemory();
            halWarping_OSD_ON(eWDT_BEFORE_WARP);  //H2PF_Simon_0065
        }
        else if(etype == eWDT_AFTER_WARP)
        {
            halWarping_ShowSpriteOnOSD();
        }
    }

    PALGEO_SEMAPHORE_GIVE;
    #endif


    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_CopyOSDRect(eWARPOSD_INSERT_LOCATION eLocation,
                                             UINT16 uiSrcX,
                                             UINT16 uiSrcY,
                                             UINT16 uiSrcWidth,
                                             UINT16 uiSrcHeight,
                                             UINT16 uiDestX,
                                             UINT16 uiDestY)

{
    PALGEO_SEMAPHORE_TAKE;

    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    #ifdef SCALER_FPGA_F34    //H2PF_Simon_0038
    {
        eResult = halWarping_CopyOSD(eLocation,
                                     uiSrcX,
                                     uiSrcY,
                                     uiSrcWidth,
                                     uiSrcHeight,
                                     uiDestX,
                                     uiDestY);
    }
    #else
    {
        eResult = halWarping_CopyOSDRect(eLocation,
                                         uiSrcX,
                                         uiSrcY,
                                         uiDestX,
                                         uiDestY,
                                         uiSrcWidth,
                                         uiSrcHeight);
    }
    #endif

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DrawChecksOnOSD(UINT8 ucLayerMode, INT16 iBlockSize)  //H2PF_Simon_0035
{
    if(iBlockSize <= 0 || ucLayerMode >= eWDT_INVALID)
    {
        return ePAL_GEO_EXEC_CODE_FAIL;
    }

    #ifdef SCALER_FPGA_F34
    {
        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawCheckerBoardPattern(ucLayerMode, iBlockSize);
        PALGEO_SEMAPHORE_GIVE;
    }
    #else
    {
        eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

        LOG_MSG(db_HAL_WARPING, "%s %d %d\n", __FUNCTION__, ucLayerMode, iBlockSize);

        if(iBlockSize <= 0 || ucLayerMode >= eWDT_INVALID)  //A65_OPTOMA_CL_0015
        {

            return eEXEC_CODE_FAIL;
        }

        sWARPOSD_CHECKBOARD_DRAW_INFO sDrawCheckboardInfo = {.ucLayerMode = ucLayerMode,
                                                             .iBlockSize  = iBlockSize};
        utilWarp_CopyChecksInfo(&sDrawCheckboardInfo);  //A65_OPTOMA_CL_0015
        eResult = palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_CHECKBOARD);  //A65_OPTOMA_CL_0006   //HICC2_Simon_0008
        //eResult = halWarping_DrawCheckboard(xDrawCheckboardInfo->ucLayerMode,    //A65_OPTOMA_CL_0015
        //                                    xDrawCheckboardInfo->iBlockSize);
    }
    #endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DrawSolidColor(UINT32 Red, UINT32 Green, UINT32 Blue)  //H2PF_Simon_0035
{
    LOG_MSG(db_HAL_WARPING, "Draw Solid Color (%d %d %d)\r\n", Red, Green, Blue);

#ifdef SCALER_FPGA_F34

    if(palDataMgr_ACU_DisableBlend_Get() == eACU_DISABLE_BLEND_NONE)  //turn off blending until finishing camara function     //H2PF_Simon_0036
    {
        palDataMgr_ACU_DisableBlend_Set(ets_ON, FALSE);
    }

    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_Palette_Set(USER_PALETTE_INDEX, Red, Green, Blue);
    halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0,halWarping_HResGet(),halWarping_VResGet(), USER_PALETTE_INDEX);
    halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
    PALGEO_SEMAPHORE_GIVE;

#else

    PALGEO_SEMAPHORE_TAKE;
    sPaletteSingleInfo sPlt = {COLOR_IDX__USER_DEFINE, Red, Green, Blue};
    halWarpOSD_Config(eGCI_OSD, eWOC_Palette_Single, &sPlt, eWRTIE_TO_CHIP);
    PALGEO_SEMAPHORE_GIVE;

    palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_SOLID_COLOR);  //HICC2_Simon_0008

#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_OSD_On(eWARPOSD_INSERT_LOCATION eLocation)  //H2PF_Simon_0035
{
#ifdef SCALER_FPGA_F34

    PALGEO_SEMAPHORE_TAKE;
    halWarping_OSD_ON((eWARPOSD_DRAW_TYPE)eLocation);
    PALGEO_SEMAPHORE_GIVE;

#else

    PALGEO_SEMAPHORE_TAKE;
    halWarping_OSD_ON();
    PALGEO_SEMAPHORE_GIVE;

#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_DrawCheckboard(eWARPOSD_INSERT_LOCATION eLocation,
                                                 INT16 iSizeX,
                                                 INT16 iSizeY,
                                                 INT16 iShiftX,
                                                 INT16 iShiftY)
{

    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    PALGEO_SEMAPHORE_TAKE;
    LOG_MSG(db_HAL_WARPING, "%s (%d %d %d %d %d)\n", __FUNCTION__
                                                   , eLocation
                                                   , iSizeX
                                                   , iSizeY
                                                   , iShiftX
                                                   , iShiftY);

    PALGEO_SEMAPHORE_GIVE;

    palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__DRAW_CHECKBOARD_TWIST);  //A65_OPTOMA_CL_0006  //HICC2_Simon_0008
    eResult = halWarping_DrawCheckboards((eLocation == eWIL_BEFORE_WARP) ? eWDT_BEFORE_WARP : eWDT_AFTER_WARP,
                                         iSizeX,
                                         iSizeY,
                                         iShiftX,
                                         iShiftY);



    return ePAL_GEO_EXEC_CODE_PASS;
}


//only draw on memory (before warp)
ePAL_GEO_EXEC_CODE palGeo_DrawBoxes(eWARPOSD_DRAW_TYPE eType,
                                          INT16 iPositionX,
                                          INT16 iPositionY,
                                          INT16 iWidth,
                                          INT16 iHeight,
                                          INT16 iRed,
                                          INT16 iGreen,
                                          INT16 iBlue)  //H2PF_Simon_0038

{
    PALGEO_SEMAPHORE_TAKE;

    halWarping_AP_DrawBox(eType,
                          iPositionX,
                          iPositionY,
                          iWidth,
                          iHeight,
                          iRed,
                          iGreen,
                          iBlue);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_FreezeImage(UINT8 ucEnable)
{
    PALGEO_SEMAPHORE_TAKE;

    #ifdef SCALER_FPGA_F34
    {
        halWarping_FreezeImage(ucEnable);

        if(ucEnable == 0)   //H2PF_Simon_0040
        {
            if((!palDataPath_IsSourceLock()) && (!palDataPath_IsSourceLockSub()))
            {//R70K_Bruce_0041, CL modify 3DE issue 13621
                //appDatapath_TestPatternEntry();
            }

            if(halWarping_IsDraw_OSD(eWDT_BEFORE_WARP) == TRUE)  //H2PF_Simon_0044
            {
                halWarpOSD_ClearOSD(eWDT_BEFORE_WARP);  //A70LK_Simon_0011
            }
            halWarping_SetScalerOsdSwitch(SCALER_OSD_PALETTE__MENU_TYPE);

            if((!palDataPath_IsSourceLock()) && (!palDataPath_IsSourceLockSub())
            && (palDataMgr_CurTestPatternGet() == eTID_OFF))
            {//R70K_Bruce_0041, CL modify 3DE issue 13621
                //appDataPath_OSD_TestPattern_Set(eCM_TEST_PATTERN_OFF);
                palDataMgr_TestPatternHandle(eTID_OFF);
                //palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL); // CL modify 3DE issue 13621
            }
        }
    }
    #else
    {
        halWarping_FreezeImage(ucEnable);
    }
    #endif

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_ColorMask(UINT8 ucColorMask)    //H2PF_Simon_0038
{
    #ifdef SCALER_FPGA_F34
    {
        UINT8 ucMask = 0;

        switch(ucColorMask)   //A70LK_Simon_0017
        {
            case 2: //only red
                ucMask = ~(SCALER_RGB_MASK_R);
                break;

            case 3: //only green
                ucMask = ~(SCALER_RGB_MASK_G);
                break;

            case 4: //only blue
                ucMask = ~(SCALER_RGB_MASK_B);
                break;

            case 5: //only yellow
                ucMask = ~(SCALER_RGB_MASK_R | SCALER_RGB_MASK_G);
                break;

            case 6: //only cyan
                ucMask = ~(SCALER_RGB_MASK_G | SCALER_RGB_MASK_B);
                break;

            case 7: //only megenta
                ucMask = ~(SCALER_RGB_MASK_R | SCALER_RGB_MASK_B);
                break;

            default:
                ucMask = 0;
                break;
        }

        halScaler_RGBMaskSet(ucMask & 0x07);
    }
    #else
    {
        PALGEO_SEMAPHORE_TAKE;
        halWarping_ColorMask(ucColorMask);
        PALGEO_SEMAPHORE_GIVE;
    }
    #endif

    return ePAL_GEO_EXEC_CODE_PASS;
}



ePAL_GEO_EXEC_CODE palGeo_AP_OSD_Init(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

    //clear OSD
    eResult |= halWarpOSD_DisableOSD();

    //init OSD layer
    eResult |= halWarping_OSDLayerReset();

    UINT8 Data = TRUE;
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_TransparentColorEnable, &Data, eWRTIE_TO_BUFFER);
    Data = DEF_TRANSPARENT_COLOR_IDX;  //H2PF_Simon_0038
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_TransparentColor0, &Data, eWRTIE_TO_BUFFER);
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_TransparentColor1, &Data, eWRTIE_TO_BUFFER);
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_TransparentColor2, &Data, eWRTIE_TO_BUFFER);
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_TransparentColor3, &Data, eWRTIE_TO_BUFFER);

    Data = 0;  //H2PF_Simon_0038
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_BlinkEnable, &Data, eWRTIE_TO_BUFFER);
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_BlinkTime,   &Data, eWRTIE_TO_BUFFER);
    eResult |= halWarpOSD_Config(eGCI_AP, eWOC_BlinkCycle,  &Data, eWRTIE_TO_CHIP);

    eResult |= halWarpOSD_Palette_Group_Set(COLOR_PALETTE__AP);

    #ifdef SCALER_FPGA_F34
    eResult |= halWarpOSD_ClearOSD(eWDT_AFTER_WARP);
    eResult |= halWarping_OSD_ON(eWDT_AFTER_WARP);  //H2PF_Simon_0035
    #else
    eResult |= halWarpOSD_ClearOSD();
    eResult |= halWarping_OSD_ON();  //H2PF_Simon_0035
    #endif

    eResult |= halWarpOSD_Draw_InsertLocation(eWDT_AFTER_WARP);

    PALGEO_SEMAPHORE_GIVE;

    if(eResult != eHAL_WARPING_EXEC_CODE_PASS)
    {
        return ePAL_GEO_EXEC_CODE_FAIL;
    }


    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_ADV_WarpingParameterInit(void)
{
    PALGEO_SEMAPHORE_TAKE;

    palDataMgr_ADV_WarpingParameterInit();

    PALGEO_SEMAPHORE_GIVE;
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpingInit(ePANEL_ID ucPanel)
{
    PALGEO_SEMAPHORE_TAKE;

    halAdvWarpingInit(ucPanel);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpControl(UINT8 ucValue)
{
    PALGEO_SEMAPHORE_TAKE;

    //black level
    switch((eWARP_CTRL)ucValue)   //H2PF_Simon_0102
    {
        case WARP_CTRL__BASIC:
            halWarping_DBD_Blending_Enable(FALSE);
            halWarping_BlackLevel_Enable(FALSE);
            break;

        case WARP_CTRL__ADVANCED:
            halWarping_DBD_Blending_Enable(FALSE);

            if(CFG_GEO_BLACKLEVELTYPE == eGEO_BLACKLEVEL_OSD)
            {
                if(utilWarp_OsdBlackLevel_GetEnable())
                {
                    //halWarping_BlackLevel_Enable(TRUE);  //H2PF_Simon_0157
                }
                else
                {
                    halWarping_BlackLevel_Enable(FALSE);
                }
            }
            else
            {
                halWarping_BlackLevel_Enable(FALSE);
            }
            break;

        case WARP_CTRL__AP:
            break;

        default:
            halWarping_DBD_Blending_Enable(FALSE);
            halWarping_BlackLevel_Enable(FALSE);
            break;
    }


    halAdvWarpControl(ucValue);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarpShowOsdPattern(UINT8 ucValue)
{
    PALGEO_SEMAPHORE_TAKE;

    halAdvWarpShowOsdPattern(ucValue);

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAT_TYPE palGeo_AdvWarpShowOsdPattern_ParaGet(void)
{
    return utilWarp_GetOsdPatternType();
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpShowOsdPattern_ParaSet(ePAT_TYPE eType)
{
    utilWarp_SetOsdPatternType(eType);

    return ePAL_GEO_EXEC_CODE_PASS;
}

BOOL palGeo_AdvWarpShowOsdPattern_Control(void)// HICC2_Bruce_0024
{
	switch(utilWarp_GetOsdPatternType())
	{
		case PAT_TYPE__SEL_BLEND_WIDTH:
		case PAT_TYPE__ADJ_BLEND_WIDTH:
		case PAT_TYPE__BLACKLEVEL_SEL_POINT:
		case PAT_TYPE__BLACKLEVEL_PREVIEW:
		case PAT_TYPE__BLACKLEVEL_BOUNDARY:
			return(TRUE);
	}
    return(FALSE);
}

ePAL_GEO_EXEC_CODE palGeo_Warp_GeometrySet(void)
{
    PALGEO_SEMAPHORE_TAKE;

    halWarp_GeometrySet();

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Warp_BlendingSet(void)
{
    PALGEO_SEMAPHORE_TAKE;

    halWarp_BlendingSet();

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_BlendingGamma(void)
{
    PALGEO_SEMAPHORE_TAKE;

    halWarp_BlendingGammaSet();

    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_WarpDownScalingIssueWorkAround(void)
{
#ifdef SCALER_FPGA_F34
    PALGEO_SEMAPHORE_TAKE;

    halWarping_WarpDownScalingIssueWorkAround();

    PALGEO_SEMAPHORE_GIVE;
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_EnableSet(UINT8 ucEnable)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halWarping_EnableSet(ucEnable);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_CurrentWarpTypeSet(UINT8 ucType)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halWarping_CurrentWarpTypeSet(ucType);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePANEL_ID palGeo_PanelGet(void)
{
#if defined(SCALER_FPGA_F34)

    return halWarping_PanelGet();

#else

    return halWarpOSD_PanelID_Get();

#endif

}

void palGeo_WarpingCheckFlagSet(UINT8 ucNeedCheck)
{
#if defined(SCALER_FPGA_F34)

    halWarping_WarpingCheckFlagSet(ucNeedCheck);

#else



#endif
}


ePAL_GEO_EXEC_CODE palGeo_Palette_Init(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

#ifdef FPGA_ENTRY_4K
    halWarp_Palette_Init();
#endif /* FPGA_ENTRY_4K */

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_ConfigPanel(const ePANEL_ID ePanelId, const UINT8 ucDualPipe3D)
{
   {
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

#ifdef FPGA_ENTRY_4K
    eRet |= halWarping_ConfigPanel(ePanelId, ucDualPipe3D);
    eRet |= halAdvWarpingInit(ePanelId);
#endif /* FPGA_ENTRY_4K */

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}
}


ePAL_GEO_EXEC_CODE palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(UINT8 ucValue) //A65_OPTOMA_Julie_0050  //A35G2_CDS_Coda_0027   //HICC2_Simon_0008
{
#ifdef SCALER_FPGA_F34


#else

    PALGEO_SEMAPHORE_TAKE;
    utilWarp_SetApBlendApply(FALSE);  //A65_OPTOMA_CL_0010
    utilWarp_SetApBlacklevelApply(FALSE);  //A65_OPTOMA_CL_0010
	utilWarp_ShowOsdPattern((ePAT_TYPE)ucValue);
    PALGEO_SEMAPHORE_GIVE;

    INT8 ucWarpingApplySetting;  //A65_OPTOMA_CL_0010

    //AP need keep blending/blacklevel effect, so check blending/blacklevel has been applied.
    if(utilWarp_GetApBlendApply() || utilWarp_GetApBlacklevelApply())
    {
        LOG_MSG(db_HAL_WARPING, "%s, ===>apply blend = %d, apply blacklevel = %d\r\n", __FUNCTION__, utilWarp_GetApBlendApply(), utilWarp_GetApBlacklevelApply());
        utilWarp_SetApBlendApply(FALSE);
        utilWarp_SetApBlacklevelApply(FALSE);

        uPALGEOAPI_INFO uWarpInfo = {.sGFN_AP_WARP_FUNCTION_APPLY_INFO.eType = eWAT_ALL};
        palGeo_Func_Set(eGFN_AP_WARP_FUNCTION_APPLY, &uWarpInfo);

        uPALGEOAPI_INFO uBlendInfo = {.sGFN_AP_DBDBLEND_FUNCTION_APPLY_INFO.eType = eBAT_ALL};
        palGeo_Func_Set(eGFN_AP_DBDBLEND_FUNCTION_APPLY, &uBlendInfo);
        //halWarping_MemoryApplyAP_CurrentSetting();
    }
#endif


    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeoAdvWarpShowOsdPattern_CheckReApplyBlend(UINT8 ucValue)
{
//#ifdef SCALER_FPGA_F34


//#else

    PALGEO_SEMAPHORE_TAKE;
    utilWarp_SetApBlendApply(FALSE);  //A65_OPTOMA_CL_0010
    utilWarp_SetApBlacklevelApply(FALSE);  //A65_OPTOMA_CL_0010
	utilWarp_ShowOsdPattern((ePAT_TYPE)ucValue);
    PALGEO_SEMAPHORE_GIVE;

    INT8 ucWarpingApplySetting;  //A65_OPTOMA_CL_0010

    //AP need keep blending/blacklevel effect, so check blending/blacklevel has been applied.
    if(utilWarp_GetApBlendApply() || utilWarp_GetApBlacklevelApply())
    {
        LOG_MSG(db_HAL_WARPING, "%s, ===>apply blend = %d, apply blacklevel = %d\r\n", __FUNCTION__, utilWarp_GetApBlendApply(), utilWarp_GetApBlacklevelApply());
        utilWarp_SetApBlendApply(FALSE);
        utilWarp_SetApBlacklevelApply(FALSE);

        uPALGEOAPI_INFO uBlendInfo = {.sGFN_AP_DBDBLEND_FUNCTION_APPLY_INFO.eType = eBAT_ALL};
        palGeo_Func_Set(eGFN_AP_DBDBLEND_FUNCTION_APPLY, &uBlendInfo);
        //halWarping_MemoryApplyAP_CurrentSetting();
    }
//#endif


    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuOff(void)  //H2PF_Simon_0030
{
    LOG_MSG(db_HAL_WARPING, "Test Pattern off !!\n");

#ifdef SCALER_FPGA_F34
    {
        if(palDataMgr_ACU_DisableBlend_Get() != eACU_DISABLE_BLEND_NONE)  //when camera function finish, turn on blending  //H2PF_Simon_0036
        {
            palDataMgr_ACU_DisableBlend_Set(ets_OFF, FALSE);
        }

        PALGEO_SEMAPHORE_TAKE;
        halWarpOSD_ClearOSD(eWDT_AFTER_WARP);
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__OFF);
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuWhite(void)
{
#ifdef SCALER_FPGA_F34
    {
        if(palDataMgr_ACU_DisableBlend_Get() == eACU_DISABLE_BLEND_NONE)  //turn off blending until finishing camara function  //H2PF_Simon_0036
        {
            palDataMgr_ACU_DisableBlend_Set(ets_ON, FALSE);
        }

        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0,halWarping_HResGet(),halWarping_VResGet(), (UINT16)AP_COLOR_IDX__WHITE);
        halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__TESTPATTERN_WHITE);   //HICC2_Simon_0008
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuBlack(void)
{
#ifdef SCALER_FPGA_F34
    {
        if(palDataMgr_ACU_DisableBlend_Get() == eACU_DISABLE_BLEND_NONE)  //turn off blending until finishing camara function  //H2PF_Simon_0036
        {
            palDataMgr_ACU_DisableBlend_Set(ets_ON, FALSE);
        }

        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0,halWarping_HResGet(),halWarping_VResGet(), (UINT16)AP_COLOR_IDX__BLACK);
        halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__TESTPATTERN_BLACK);   //HICC2_Simon_0008
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuGrid(void)
{
#ifdef SCALER_FPGA_F34
    {
        if(palDataMgr_ACU_DisableBlend_Get() == eACU_DISABLE_BLEND_NONE)  //turn off blending until finishing camara function  //H2PF_Simon_0036
        {
            palDataMgr_ACU_DisableBlend_Set(ets_ON, FALSE);
        }

        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawOsdAcuGridPattern();
        halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__TESTPATTERN_GRID);   //HICC2_Simon_0008
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuGridCenter(void)
{
#ifdef SCALER_FPGA_F34
    {
        if(palDataMgr_ACU_DisableBlend_Get() == eACU_DISABLE_BLEND_NONE)  //turn off blending until finishing camara function  //H2PF_Simon_0036
        {
            palDataMgr_ACU_DisableBlend_Set(ets_ON, FALSE);
        }

        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawOsdAcuCenterGridPattern();
        halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__TESTPATTERN_GRIDCENTER);   //HICC2_Simon_0008
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuBlackBorder(void)
{
#ifdef SCALER_FPGA_F34
    {
        PALGEO_SEMAPHORE_TAKE;
        halWarping_DrawOsdAcuBlackBorderPattern();
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__TESTPATTERN_BLACK_BORDER);   //HICC2_Simon_0008
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_TestPatternCuGridSelect(sCOLOR_UNIFORMITY_POINT_SELECT sColorUniformityPoint)
{
    LOG_MSG(db_HAL_WARPING, "Test Pattern CU Grid Select !!\n");

#ifdef SCALER_FPGA_F34
    {
        PALGEO_SEMAPHORE_TAKE;
        halWarping_SelectCUGrid(&sColorUniformityPoint);
        halWarping_DrawOsdAcuBlackBorderMultipleSelectPattern();
        halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
        PALGEO_SEMAPHORE_GIVE;
    }
#else
    {
        PALGEO_SEMAPHORE_TAKE;
        utilWarp_SelectCUGrid(&sColorUniformityPoint);
        PALGEO_SEMAPHORE_GIVE;
        palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__TESTPATTERN_BLACK_BORDER_MULTIPLESELECT);
    }
#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuExitBlackBorderHandle(void)
{
#ifdef SCALER_FPGA_F34

    PALGEO_SEMAPHORE_TAKE;
    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);
    halWarpOSD_SetLastAddress_E4K(eWDT_BEFORE_WARP);
    PALGEO_SEMAPHORE_GIVE;

#elif defined(SCALER_ICHIPS)     //H2PF_Simon_0087

    PALGEO_SEMAPHORE_TAKE;
    halAdvWarpShowOsdPattern(PAT_TYPE__OFF);
    PALGEO_SEMAPHORE_GIVE;

#else

    LOG_MSG(db_HAL_WARPING, "\r\n!!! %s is not implemented\r\n\r\n", __FUNCTION__);

#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

//for drawing current sel position when pattern on
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuBlackBorderTargetSelPostionTmpSet(UINT8 ucACU_Target_Tmp)
{
#ifdef SCALER_FPGA_F34

    PALGEO_SEMAPHORE_TAKE;
    halWarping_ACU_Target_Status_Set(ucACU_Target_Tmp);
    PALGEO_SEMAPHORE_GIVE;

#endif

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_AP_Ifill(INT16 iEnable, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)  //HICC2_Simon_0024
{
#ifdef SCALER_FPGA_F34

#else
    PALGEO_SEMAPHORE_TAKE;
    halWarping_AP_Ifill(iEnable, ucRed, ucGreen, ucBlue);
    PALGEO_SEMAPHORE_GIVE;
#endif /* SCALER_FPGA_F34 */

    return ePAL_GEO_EXEC_CODE_PASS;
}


eRESULT palGeo_DrawOSDQueueSet(ePAL_GEO_FUNCTION eGeoFn, uPALGEOAPI_INFO uInfo)  //H2PF_Simon_0038
{
    eRESULT eResult = rcERROR;

    if(m_sPalGeoInfo.bWarpingInit == FALSE)
    {
        LOG_MSG(db_HAL_WARPING, "palGeo_DrawOSDQueueSet (%d) fail, Warping not init \r\n", eGeoFn);
        return eResult;
    }

    sDRAW_WAPR_OSD_QUEUE_INFO sDrawInfo = {eGeoFn, uInfo};

    if(palGeo_OsdQueueDrawSemaphoreTake())  //A70LK_Simon_0008
    {
        LOG_MSG(db_HAL_WARPING, "==> (%s %d) %d\r\n\r\n", __FUNCTION__, __LINE__, eGeoFn);
        eResult = utilQueueWrite(&m_sPalGeoInfo.sDrawWarpOsd_Queue, (UINT8 *)&sDrawInfo);
        palGeo_OsdQueueDrawSemaphoreGive();
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d) mutex fail\n", __FUNCTION__, __LINE__);
        eResult = rcERROR;
    }

    ASSERT(eResult == rcSUCCESS);

    return eResult;
}

eRESULT palGeo_DrawWarpingOSDQueue(void)  //H2PF_Simon_0038
{
    eRESULT eResult = rcERROR;
    sDRAW_WAPR_OSD_QUEUE_INFO sDrawInfo;
    memset((UINT8*)&sDrawInfo, 0, sizeof(sDrawInfo));

    LOG_MSG(db_HAL_WARPING, "\r\n\r\n  >>>>> <%s>\r\n", __FUNCTION__);

    if(palGeo_OsdQueueDrawSemaphoreTake())
    {
        while(rcSUCCESS == utilQueueRead(&m_sPalGeoInfo.sDrawWarpOsd_Queue, (UINT8*)&sDrawInfo))
        {
            LOG_MSG(db_HAL_WARPING, "Queue Fn %d\r\n\r\n", sDrawInfo.eFunc);
            palGeo_Func_Set(sDrawInfo.eFunc, &sDrawInfo.uData);

            if((sDrawInfo.eFunc == eGFN_DRAW_OSD) || (sDrawInfo.eFunc == eGFN_CLEAR_OSD))
            {
                palGeo_OsdQueueDrawSemaphoreGive();
                return rcSUCCESS;
            }

            memset((UINT8*)&sDrawInfo, 0, sizeof(sDrawInfo));
        }

        palGeo_OsdQueueDrawSemaphoreGive();
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d) mutex fail\n", __FUNCTION__, __LINE__);
        return rcERROR;
    }

    return rcSUCCESS;
}


BOOL palGeo_IsDraw_OSD(eWARPOSD_INSERT_LOCATION eDrawType)   //H2PF_Simon_0040
{
#ifdef SCALER_FPGA_F34
    {
        return halWarping_IsDraw_OSD((eWARPOSD_DRAW_TYPE)eDrawType);
    }
#elif defined(SCALER_ICHIPS)
    {
        if(eDrawType == eWIL_BEFORE_WARP)  //H2PF_Simon_0070
        {
            return halWarping_GetFreezeImageState();
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "palGeo_IsDraw_OSD get Fail\r\n" );
            return FALSE;
        }
    }
#endif /* SCALER_FPGA_F34 */

    return FALSE;
}


INT8 palGeo_RestorePalette(void)
{

#if defined(SCALER_FPGA_F34)

    return halGui_RestorePalette();

#else

    return 1;

#endif

}



sDRAW_FRAME_BUFFER_TEMP_DATA sDrawFrmaeBufferTempData;   //H2PF_Simon_0057

ePAL_GEO_EXEC_CODE palGeo_Draw_PNG_Pattern(UINT8 ucPatternIndex, eDRAW_COLOR_TYPE eColorType, char *FileName) //H2PF_Simon_0057
{
#ifndef SIMULATOR_ISCALER

    UINT8 Data = FALSE;
    INT16 iWidth, iHeight;
    UINT8 *pauctmpFullColor = (UINT8 *)malloc(halWarping_HResGet() * halWarping_VResGet() * 3 + 1024);

    //char *FileName = "/usr/bin/A1.png";
    if(pauctmpFullColor == NULL)
    {
        ASSERT_ALWAYS();
        return ePAL_GEO_EXEC_CODE_MALLOC_FAIL;
    }
    memset(pauctmpFullColor, 0, (halWarping_HResGet() * halWarping_VResGet() * 3 + 1024));

    //turn off splash
    //palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
    //MS_SLEEP(300);  //wait OSD event done

    //read PNG file
    LOG_MSG(db_HAL_WARPING, "\nStart read PNG FileName %s\n", FileName);

    if(access(FileName, R_OK ) != 0)
    {
        LOG_MSG(db_HAL_WARPING, "\n\n\nFile:%s is not exist\n\n", FileName);
        return ePAL_GEO_EXEC_CODE_FILE_ERROR;
    }

    read_png_file((char*)FileName, (int*)&iWidth, (int*)&iHeight);
    LOG_MSG(db_HAL_WARPING, "End read PNG Width=%d, Height=%d\n", FileName, iWidth, iHeight);

    //process file
    LOG_MSG(db_HAL_WARPING, "Start process_file\n");
    process_file(pauctmpFullColor, halWarping_HResGet(), halWarping_VResGet());
    LOG_MSG(db_HAL_WARPING, "End process_file\n");

    //palDataMgr_Data_Access(edcSHOW_MESSAGES, edaREAD, &sDrawFrmaeBufferTempData.ucShowMessage);
    //palDataMgr_Data_Access(edcWARP_TOGGLE, edaREAD, &sDrawFrmaeBufferTempData.ucGeometryEnable);
    //palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &sDrawFrmaeBufferTempData.ucAutoSource);

    //Disable Auto Source
    Data = eCM_INPUT_KEY_CHANGE_SOURCE;
    palDataMgr_Data_Access(edcINPUT_KEY, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);

    //Disable OSD Message
    Data = FALSE;
    palDataMgr_Data_Access(edcSHOW_MESSAGES, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);

    //Disable Geometry Enable
    palGeo_EnableSet(FALSE);    //H2PF_Simon_0058

    if(eColorType == eDRAW_TRUE_COLOR)
    {
        //record current color space
        //palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, &sDrawFrmaeBufferTempData.ucColorSpace);

        //set RGB color space
        Data = eCM_COLOR_SPACE_RGB_FULL;
        palDataMgr_Data_Access(edcCOLOR_SPACE, edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
    }


    PALGEO_SEMAPHORE_TAKE;
    if(eColorType == eDRAW_565HIGH_COLOR)
    {
        #ifdef SCALER_FPGA_F34
        halWarping_DrawPNG_OnBackground(ucPatternIndex, 0,0,halWarping_HResGet(),halWarping_VResGet(),pauctmpFullColor);
        #endif
    }
    else if(eColorType == eDRAW_TRUE_COLOR)
    {
        #ifdef SCALER_FPGA_F34
        //start draw PNG data on Frame Buffer
        halWarping_DrawPNG_OnFrameBuffer(0,0,halWarping_HResGet(),halWarping_VResGet(),pauctmpFullColor);
        #endif
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "ColorType Error %d\r\n", eColorType);
    }
    PALGEO_SEMAPHORE_GIVE;

    free(pauctmpFullColor);
#endif /* SIMULATOR_ISCALER */

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Draw_PNG_PatternChange(UINT8 ucPatternIndex)
{
    UINT8 Data = FALSE;

    //turn off splash
    palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
    MS_SLEEP(300);  //wait OSD event done

    //palDataMgr_Data_Access(edcSHOW_MESSAGES, edaREAD, &sDrawFrmaeBufferTempData.ucShowMessage);
    //palDataMgr_Data_Access(edcWARP_TOGGLE, edaREAD, &sDrawFrmaeBufferTempData.ucGeometryEnable);
    //palDataMgr_Data_Access(edcINPUT_KEY, edaREAD, &sDrawFrmaeBufferTempData.ucAutoSource);


    //Disable Auto Source
    Data = eCM_INPUT_KEY_CHANGE_SOURCE;
    palDataMgr_Access_InputKey(edaWRITE_RAM_ONLY_WITH_ACTION, &Data);

    //Disable OSD Message
    Data = FALSE;
    palDataMgr_Access_MenuShowMessages(edaWRITE_RAM_ONLY_WITH_ACTION, &Data);

    //Disable Geometry Enable
    palGeo_EnableSet(FALSE);    //H2PF_Simon_0058

#if 0
    if(eColorType == eDRAW_TRUE_COLOR)
    {
        //record current color space
        palDataMgr_Data_Access(edcCOLOR_SPACE, edaREAD, &sDrawFrmaeBufferTempData.ucColorSpace);

        //set RGB color space
        Data = eCM_COLOR_SPACE_RGB_FULL;
        palDataMgr_Access_ColorSpace(edaWRITE_RAM_ONLY_WITH_ACTION, &Data);
    }
#endif /* 0 */

    PALGEO_SEMAPHORE_TAKE;

    #ifdef SCALER_FPGA_F34
    halWarping_DrawPNG_ChangeIndex(ucPatternIndex);
    #endif

    PALGEO_SEMAPHORE_GIVE;


    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_Draw_PNG_Pattern_Off(void) //H2PF_Simon_0057
{
    PALGEO_SEMAPHORE_TAKE;
    #ifdef SCALER_FPGA_F34
    halWarping_DrawPNG_Off();
    #endif
    PALGEO_SEMAPHORE_GIVE;

    if(palDataPath_IsSourceLock())
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 0, NULL);
    }
    else
    {
        palDataMgr_UI_EventSend(edcUI_EVENT_SPLASH, 1, NULL);
    }

    //reload Auto Source
    //palDataMgr_Access_InputKey(edaWRITE_RAM_ONLY_WITH_ACTION, &sDrawFrmaeBufferTempData.ucAutoSource);
    //palLANProcSendToLAN(edcINPUT_KEY);

    //reload edcSHOW_MESSAGES
    palDataMgr_Access_MenuShowMessages(edaWRITE_RAM_ONLY_WITH_ACTION, &sDrawFrmaeBufferTempData.ucShowMessage);
    palLANProcSendToLAN(edcSHOW_MESSAGES);
    palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD, TRUE, NULL);   //H2PF_Simon_0058

    //reload edcWARP_TOGGLE
    UINT8 GeoEnable = 0;
    palDataMgr_Data_Access(edcWARP_TOGGLE, edaREAD, &GeoEnable);
    if(GeoEnable == TRUE) //H2PF_Simon_0058
    {
        palGeo_EnableSet(TRUE);
    }

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferColorSpace(UINT8 ucColorSpace)
{
#ifdef SCALER_FPGA_F34
    sDrawFrmaeBufferTempData.ucColorSpace = ucColorSpace;
#endif /* SCALER_FPGA_F34 */
	return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferShowMessage(UINT8 ucShowMessage)
{
#ifdef SCALER_FPGA_F34
    sDrawFrmaeBufferTempData.ucShowMessage = ucShowMessage;
#endif /* SCALER_FPGA_F34 */
	return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferGeometryEnable(UINT8 ucGeometryEnable)
{
#ifdef SCALER_FPGA_F34
    sDrawFrmaeBufferTempData.ucGeometryEnable = ucGeometryEnable;
#endif /* SCALER_FPGA_F34 */
	return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferAutoSource(UINT8 ucAutoSource)
{
#ifdef SCALER_FPGA_F34
    sDrawFrmaeBufferTempData.ucAutoSource = ucAutoSource;
#endif /* SCALER_FPGA_F34 */
	return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_convertTPtoDTDT(FLOAT *tpx, FLOAT *tpy, INT16 width, INT16 height)
{
    eHAL_WARPING_EXEC_CODE eHAL_Ret = eHAL_WARPING_EXEC_CODE_NUMBER;
    ePAL_GEO_EXEC_CODE ePAL_Ret = ePAL_GEO_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

    eHAL_Ret = halWarping_convertTPtoDTDT(tpx, tpy, width, height);

    PALGEO_SEMAPHORE_GIVE;

    if(eHAL_Ret != eHAL_WARPING_EXEC_CODE_NUMBER)
    {
        ePAL_Ret = palGeo_ExeResult(eHAL_Ret);
    }

    return ePAL_Ret;
}

ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearAPTempFile(void)
{
    remove("/mnt/configs/BlendCurrent");
    remove("/mnt/configs/BlendMemory1");
    remove("/mnt/configs/BlendMemory2");
    remove("/mnt/configs/BlendMemory3");
    remove("/mnt/configs/BlendMemory4");
    remove("/mnt/configs/WarpCurrent");
    remove("/mnt/configs/WarpMemory1");
    remove("/mnt/configs/WarpMemory2");
    remove("/mnt/configs/WarpMemory3");
    remove("/mnt/configs/WarpMemory4");

    SYSTEM_CALL("sync");

    return ePAL_GEO_EXEC_CODE_PASS;
}


ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearSaveFile(void)
{
    char cFilename[64] = {0};

    for(UINT8 ucIndex = BLENDING_AP_MEM1_IDX ; ucIndex <= BLENDING_AP_MEM5_IDX ; ucIndex++)
    {
        sprintf(cFilename, BLENDING_AP_HICC2_WARP_SAVE_FILENAME , ucIndex);
        remove(cFilename);
        sprintf(cFilename, BLENDING_AP_HICC2_DBD_SAVE_FILENAME , ucIndex);
        remove(cFilename);
        sprintf(cFilename, BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME , ucIndex);
        remove(cFilename);
    }

    SYSTEM_CALL("sync");

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearCurrentFile(void)
{
    char cFilename[64] = {0};

    sprintf(cFilename, BLENDING_AP_HICC2_WARP_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);
    sprintf(cFilename, BLENDING_AP_HICC2_DBD_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);
    sprintf(cFilename, BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);

    SYSTEM_CALL("sync");

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearCurrentWarpFile(void)
{
    char cFilename[64] = {0};

    sprintf(cFilename, BLENDING_AP_HICC2_WARP_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);

    SYSTEM_CALL("sync");

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearCurrentBlendFile(void)
{
    char cFilename[64] = {0};

    sprintf(cFilename, BLENDING_AP_HICC2_DBD_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);
    sprintf(cFilename, BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);

    SYSTEM_CALL("sync");

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGridPoints(UINT8 ucGridPoints, UINT8 ucInner)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpGridPoints(ucGridPoints, ucInner);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarpInnerGridPoints(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpInnerGridPoints(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvResetWarping(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

    remove(WEB_PAGE_WARP_TMP_FILENAME);
    SYSTEM_CALL("sync");

    eRet = halAdvResetWarping();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpSharpness(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpSharpness(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGridColor(UINT8 ucValue, UINT8 ucRedrawPattern)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpGridColor(ucValue, ucRedrawPattern);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpBackgroundColor(UINT8 ucValue, UINT8 ucRedrawPattern)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpBackgroundColor(ucValue, ucRedrawPattern);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


//*** for PToolSet function
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetColIndex(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpGP_SetColIndex(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetColIndex(UINT8 *pValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    *pValue = utilWarp_GridPoint_GetColIndex();

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetRowIndex(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpGP_SetRowIndex(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetRowIndex(UINT8 *pValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    *pValue = utilWarp_GridPoint_GetRowIndex();

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_Move_Set(eADV_WARP_MOVE_DIR eDir, UINT16 uiPixel)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    if( utilWarp_GetOsdPatternType() != PAT_TYPE__WARP_SEL_CTRL_POINT )
    {
        halAdvWarpShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
    }

    eRet = halAdvWarpGP_Move((eDIR)eDir, uiPixel);
    if(eRet == eHAL_WARPING_EXEC_CODE_PASS)
    {
        SaveWarpConfig();
    }
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetColPosition(INT32 *pValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    *pValue = utilWarp_GridPoint_GetColPosition();

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetRowPosition(INT32 *pValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    *pValue = utilWarp_GridPoint_GetRowPosition();

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetPosition(INT32 lPosX, INT32 lPosY)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    LOG_MSG(db_HAL_WARPING, "(%s@%d)[%d %d]\r\n\r\n", __FUNCTION__, __LINE__,  lPosX, lPosY);
    if( utilWarp_GetOsdPatternType() != PAT_TYPE__WARP_SEL_CTRL_POINT )
    {
        halAdvWarpShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
    }

    eRet = halAdvWarpGP_SetPositionMove(lPosX, lPosY);

    if(eRet == eHAL_WARPING_EXEC_CODE_PASS)
    {
        SaveWarpConfig();
    }
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetAllPosition(Point2f Data[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V])
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    LOG_MSG(db_HAL_WARPING, "(%s@%d)\r\n", __FUNCTION__, __LINE__);

    eRet = halAdvWarpGP_SetAllPosition(Data);

    if(eRet == eHAL_WARPING_EXEC_CODE_PASS)
    {
        SaveWarpConfig();
    }
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SelectControlPoint(eADV_WARP_MOVE_DIR eDir)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    utilWarp_SelectControlPoint((eDIR)eDir);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_MoveControlPoint(eADV_WARP_MOVE_DIR eDir)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halAdvWarpMoveControlPoint((eDIR)eDir);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SettingReset(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpSettingReset();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

void palGeo_BlacklevelClearCurrentData(void)
{
    halWarping_Blacklevel_ClearCurrentData();
}

UINT8 palGeo_AdvWarp_GetShowBlendOnWarpPattern(void)
{
    UINT8 ucValue = (UINT8)utilWarp_GetShowBlendOnWarpPattern();

    return ucValue;
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SetShowBlendOnWarpPattern(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    utilWarp_SetShowBlendOnWarpPattern(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


UINT8 palGeo_AdvWarp_GetDrawOverlapGridOnWarpPattern(void)
{
    UINT8 ucValue = (UINT8)utilWarp_GetDrawOverlapGridOnWarpPattern();

    return ucValue;
}


ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SetDrawOverlapGridOnWarpPattern(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    utilWarp_SetDrawOverlapGridOnWarpPattern(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SelectControlPoint(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    halAdvWarpSelectControlPoint(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

UINT8 palGeo_AdvBlend_GetBlendPatternState(void)
{
    UINT8 ucValue = (UINT8)utilWarp_GetBlendPatternState();

    return ucValue;
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_Enable_Set(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlendingEnable_Set(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_Enable_Get(UINT8 *pValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    *pValue = utilWarp_GetBlendEnable();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_OverlapSize_Set(eADV_BLEND_DIR eDir, UINT16 uiValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

    eRet |= halAdvBlending_Width((eDIR)eDir,  uiValue);

    if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
    {
        eRet |= halAdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
    }

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


UINT16 palGeo_AdvBlend_OverlapSize_Get(eADV_BLEND_DIR eDir)
{
    switch(eDir)
    {
        case eADV_BLEND_DIR__UP:
            return utilWarp_GetOsdBlendWidthTop();

        case eADV_BLEND_DIR__DOWN:
            return utilWarp_GetOsdBlendWidthDown();

        case eADV_BLEND_DIR__LEFT:
            return utilWarp_GetOsdBlendWidthLeft();

        case eADV_BLEND_DIR__RIGHT:
            return utilWarp_GetOsdBlendWidthRight();
    }

    return 0;
}


ePAL_GEO_EXEC_CODE palGeo_AdvBlend_OverlapOffset_Set(eADV_BLEND_DIR eDir, UINT16 uiValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;

    eRet |= halAdvBlending_Offset((eDIR)eDir,  uiValue);

    if( utilWarp_GetOsdPatternType() == PAT_TYPE__SEL_BLEND_WIDTH )
    {
        eRet |= halAdvWarpShowOsdPattern(PAT_TYPE__SEL_BLEND_WIDTH);
    }

    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


UINT16 palGeo_AdvBlend_OverlapOffset_Get(eADV_BLEND_DIR eDir)
{
    switch(eDir)
    {
        case eADV_BLEND_DIR__UP:
            return utilWarp_GetOsdBlendOffsetTop();

        case eADV_BLEND_DIR__DOWN:
            return utilWarp_GetOsdBlendOffsetDown();

        case eADV_BLEND_DIR__LEFT:
            return utilWarp_GetOsdBlendOffsetLeft();

        case eADV_BLEND_DIR__RIGHT:
            return utilWarp_GetOsdBlendOffsetRight();
    }

    return 0;
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_ConfigReset(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlendConfigReset();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_SelectOsdBlendWidth(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpSelectOsdBlendWidth(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_AdjustOsdBlendWidth(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvWarpAdjustOsdBlendWidth(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlend_OverlapGridNumber(UINT8 ucValue)    //H2PF_Simon_0108
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlendingOverlapGridNumber(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetEnable(UINT8 ucValue)    //H2PF_Simon_0106
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelSetEnable(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_AddPoint(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelAddPoint();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}


ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_DeletePoint(void)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelDeletePoint();
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SelectControlPoint(eADV_BKLV_DIR eDir)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelSelectControlPoint((eDIR)eDir);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_MoveControlPoint(eADV_BKLV_DIR eDir)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelMoveControlPoint((eDIR)eDir);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SelectArea(UINT8 ucValue)  //H2PF_Simon_0106  Start
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelSelectArea(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetRed(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelSetRed(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetGreen(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelSetGreen(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetBlue(UINT8 ucValue)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelSetBlue(ucValue);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvHide_OSD_Pattern_Set(BOOL bHide)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvHide_OSD_Pattern_Set(bHide);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlacklevel_Boundary_Pattern_Hide(BOOL bHide)   //H2PF_Simon_0155
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlack_Level_Boundary_Pattern_Hide(bHide);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_Boundary_Flag_Set(UINT8 ucBoundary)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlack_Level_Boundary_Flag_Set(ucBoundary);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}

ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_Reset(UINT8 ucResetLevel)
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    PALGEO_SEMAPHORE_TAKE;
    eRet = halAdvBlackLevelReset(ucResetLevel);
    PALGEO_SEMAPHORE_GIVE;

    return palGeo_ExeResult(eRet);
}                                                                                  //H2PF_Simon_0106 End

INT32 palGeo_Func_Data_Get(ePAL_GEO_FUNCTION eGeoFn)
{
    switch(eGeoFn)
    {
        case eGFN_AP_LINK:
            return (INT32)palGeo_ApLinkFlag_Get();
            break;

        case eGFN_CLI_BKLV_AREA_SELECT:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_AREA_SEL);

        //coordinate
        case eGFN_CLI_BKLV_TL_X:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TL_X);

        case eGFN_CLI_BKLV_TL_Y:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TL_Y);

        case eGFN_CLI_BKLV_TR_X:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TR_X);

        case eGFN_CLI_BKLV_TR_Y:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_TR_Y);

        case eGFN_CLI_BKLV_BL_X:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BL_X);

        case eGFN_CLI_BKLV_BL_Y:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BL_Y);

        case eGFN_CLI_BKLV_BR_X:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BR_X);

        case eGFN_CLI_BKLV_BR_Y:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_CUR_BR_Y);


        //palette R,G,B
        case eGFN_CLI_BKLV_PLT_R:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_PLT_RED);

        case eGFN_CLI_BKLV_PLT_G:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_PLT_GREEN);

        case eGFN_CLI_BKLV_PLT_B:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_PLT_BLUE);


        case eGFN_CLI_BKLV_AREA_ENABLE:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_AREA_ENABLE);

        case eGFN_CLI_BKLV_APPLY:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_APPLY);

        case eGFN_CLI_BKLV_APPLY_ALL:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_APPLY_ALL_AREA);

        case eGFN_CLI_BKLV_ENABLE:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_ENABLE);

        case eGFN_CLI_BKLV_RESET:
            return halWarp_BlacklevelParameterGet(eBKLV_EVENT_RESET);

        default:
            LOG_MSG(db_HAL_WARPING, "palGeo_Func_Data_Get %d undefined\n", eGeoFn);
            return 0;
    }

    return 0;

}


ePAL_GEO_EXEC_CODE palGeo_Func_Set(ePAL_GEO_FUNCTION eGeoFn, uPALGEOAPI_INFO *puInfo)
{
    eHAL_WARPING_EXEC_CODE eHAL_Ret = eHAL_WARPING_EXEC_CODE_NUMBER;
    ePAL_GEO_EXEC_CODE ePAL_Ret = ePAL_GEO_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "palGeo_Func_Set %d\r\n", eGeoFn);

    switch(eGeoFn)
    {
        case eGFN_AP_LINK:
            palGeo_ApLinkFlag_Set(puInfo->sGFN_AP_LINK_INFO.APLinkEnable);
            break;

        case eGFN_AP_SET_TRANSPARENT_COLOR:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarping_Set_OSD_Transparent(puInfo->sGFN_AP_SET_TRANSPARENT_COLOR_INFO.PaletteIndex);
            PALGEO_SEMAPHORE_GIVE;
            break;

        ////////////////////////////////
        //Apply Warping
        ////////////////////////////////
        case eGFN_AP_WARP_FUNCTION_APPLY:
            ePAL_Ret = palGeo_MemoryApplyAP_Warp(BLENDING_AP_MEM_CUR_IDX, puInfo->sGFN_AP_WARP_FUNCTION_APPLY_INFO.eType);
            break;

        case eGFN_AP_WARP_MEM_APPLY:
            {
                UINT8 ApplyIndex = puInfo->sGFN_AP_WARP_MEM_APPLY_INFO.ApplyIndex;

                //memory file copy to current memory
                if(ApplyIndex != BLENDING_AP_MEM_CUR_IDX)
                {
                    //remove origin file    //H2PF_Simon_0094
                    char CurrentFileName[GEO_MEMORY_MAX_FILENAME_LENGTH] = {0};
                    palGeo_MemoryFileNameGet(eGFT_AP_WARP, BLENDING_AP_MEM_CUR_IDX, CurrentFileName);
                    SYSTEM_CALL("rm -r %s", CurrentFileName);

                    //copy memory file to current
                    palGeo_MemoryFileCopy(eGFT_AP_WARP,
                                          ApplyIndex,
                                          BLENDING_AP_MEM_CUR_IDX);
                }
                //apply current memory
                ePAL_Ret = palGeo_MemoryApplyAP_Warp(BLENDING_AP_MEM_CUR_IDX,
                                                     eWAT_ALL);
            }
            break;

        case eGFN_AP_WARP_SELECT_MEM_APPLY:
            ePAL_Ret = palGeo_MemoryApplyAP_Warp(puInfo->sGFN_AP_WARP_SELECT_MEM_APPLY_INFO.eIndex,
                                                 puInfo->sGFN_AP_WARP_SELECT_MEM_APPLY_INFO.eType);
            break;


        ////////////////////////////////
        //Apply DBD Blending
        ////////////////////////////////
        case eGFN_AP_DBDBLEND_FUNCTION_APPLY:
            ePAL_Ret = palGeo_MemoryApplyAP_Blending(BLENDING_AP_MEM_CUR_IDX,
                                                     puInfo->sGFN_AP_DBDBLEND_FUNCTION_APPLY_INFO.eType);
            break;

        case eGFN_AP_DBDBLEND_MEM_APPLY:
            {
                UINT8 ApplyIndex = puInfo->sGFN_AP_DBDBLEND_MEM_APPLY_INFO.ApplyIndex;

                //memory file copy to current memory
                if(ApplyIndex != BLENDING_AP_MEM_CUR_IDX)
                {
                    //remove origin file    //H2PF_Simon_0094
                    char CurrentFileName[GEO_MEMORY_MAX_FILENAME_LENGTH] = {0};
                    palGeo_MemoryFileNameGet(eGFT_AP_BLEND, BLENDING_AP_MEM_CUR_IDX, CurrentFileName);
                    SYSTEM_CALL("rm -r %s", CurrentFileName);

                    //copy memory file to current
                    palGeo_MemoryFileCopy(eGFT_AP_BLEND,
                                          ApplyIndex,
                                          BLENDING_AP_MEM_CUR_IDX);
                }
                //apply current memory
                ePAL_Ret = palGeo_MemoryApplyAP_Blending(BLENDING_AP_MEM_CUR_IDX,
                                                         eBAT_ALL);
            }
            break;

        case eGFN_AP_DBDBLEND_SELECT_MEM_APPLY:
            ePAL_Ret = palGeo_MemoryApplyAP_Blending(puInfo->sGFN_AP_DBDBLEND_SELECT_MEM_APPLY_INFO.eIndex,
                                                     puInfo->sGFN_AP_DBDBLEND_SELECT_MEM_APPLY_INFO.eType);
            break;


        ////////////////////////////////
        //Apply Black level
        ////////////////////////////////
        case eGFN_AP_BKLV_FUNCTION_APPLY:
            ePAL_Ret = palGeo_MemoryApplyAP_BlackLevel(BLENDING_AP_MEM_CUR_IDX,
                                                       puInfo->sGFN_AP_BKLV_FUNCTION_APPLY_INFO.eType);
            break;

        case eGFN_AP_BKLV_MEM_APPLY:
            {
                UINT8 ApplyIndex = puInfo->sGFN_AP_BKLV_MEM_APPLY_INFO.ApplyIndex;

                //memory file copy to current memory
                if(ApplyIndex != BLENDING_AP_MEM_CUR_IDX)
                {
                    //remove origin file    //H2PF_Simon_0094
                    char CurrentFileName[GEO_MEMORY_MAX_FILENAME_LENGTH] = {0};
                    palGeo_MemoryFileNameGet(eGFT_AP_BLACKLEVEL, BLENDING_AP_MEM_CUR_IDX, CurrentFileName);
                    SYSTEM_CALL("rm -r %s", CurrentFileName);

                    //copy memory file to current
                    palGeo_MemoryFileCopy(eGFT_AP_BLACKLEVEL,
                                          ApplyIndex,
                                          BLENDING_AP_MEM_CUR_IDX);
                }
                //apply current memory
                ePAL_Ret = palGeo_MemoryApplyAP_BlackLevel(BLENDING_AP_MEM_CUR_IDX,
                                                           eLAT_ALL);
            }
            break;

        case eGFN_AP_BKLV_SELECT_MEM_APPLY:
            ePAL_Ret = palGeo_MemoryApplyAP_BlackLevel(puInfo->sGFN_AP_BKLV_SELECT_MEM_APPLY_INFO.eIndex,
                                                       puInfo->sGFN_AP_BKLV_SELECT_MEM_APPLY_INFO.eType);
            break;

        case eGFN_AP_CLEAR_ALL:
            eHAL_Ret = eHAL_WARPING_EXEC_CODE_PASS;
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret |= halWarping_ClearWarpingTable();
            eHAL_Ret |= halWarp_InterpolationSet();
            eHAL_Ret |= halWarping_EdgeBlending_Disable();
            eHAL_Ret |= halWarping_DBD_ClearBlendingTable();
            eHAL_Ret |= halWarping_clearBlackLevel();
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_AP_WARP_CLEAR:
            eHAL_Ret = palGeo_ClearWarpingTable();   //HICC2_Simon_0017
            break;

        case eGFN_AP_DBD_BLEND_CLEAR:
            eHAL_Ret = palGeo_DBD_ClearBlendingTable();   //HICC2_Simon_0017
            break;

        case eGFN_AP_BKLV_CLEAR:
            eHAL_Ret = palGeo_ClearBlackLevel();   //HICC2_Simon_0017
            break;

        case eGFN_AP_BKLV_ENABLE:
            ePAL_Ret = palGeo_BlackLevel_Enable(puInfo->sGFN_AP_BKLV_ENABLE_INFO.Enable);
            break;

        case eGFN_AP_BKLV_RESET:
            ePAL_Ret = palGeo_BlackLevel_Reset();
            break;

        case eGFN_AP_BKLV_PLT_SET:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_AREA_SEL, puInfo->sGFN_AP_BKLV_PLT_SET_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_AREA_SELECT:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_AREA_SEL, puInfo->sGFN_CLI_BKLV_AREA_SELECT_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_TL_X:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TL_X, puInfo->sGFN_CLI_BKLV_TL_X_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_TL_Y:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TL_Y, puInfo->sGFN_CLI_BKLV_TL_Y_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_TR_X:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TR_X, puInfo->sGFN_CLI_BKLV_TR_X_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_TR_Y:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_TR_Y, puInfo->sGFN_CLI_BKLV_TR_Y_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_BL_X:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BL_X, puInfo->sGFN_CLI_BKLV_BL_X_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_BL_Y:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BL_Y, puInfo->sGFN_CLI_BKLV_BL_Y_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_BR_X:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BR_X, puInfo->sGFN_CLI_BKLV_BR_X_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_BR_Y:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_CUR_BR_Y, puInfo->sGFN_CLI_BKLV_BR_Y_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_PLT_R:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_PLT_RED,   puInfo->sGFN_CLI_BKLV_PLT_R_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_PLT_G:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_PLT_GREEN, puInfo->sGFN_CLI_BKLV_PLT_G_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_PLT_B:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_PLT_BLUE,  puInfo->sGFN_CLI_BKLV_PLT_B_INFO.Val);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_AREA_ENABLE:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_AREA_ENABLE, puInfo->sGFN_CLI_BKLV_AREA_ENABLE_INFO.Enable);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_APPLY:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_APPLY, puInfo->sGFN_CLI_BKLV_APPLY_INFO.Enable);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_APPLY_ALL:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_AREA_ENABLE, puInfo->sGFN_CLI_BKLV_APPLY_ALL_INFO.Enable);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_ENABLE:
            PALGEO_SEMAPHORE_TAKE;   //HICC2_Simon_0017
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_ENABLE, puInfo->sGFN_CLI_BKLV_ENABLE_INFO.Enable);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_RESET:
            PALGEO_SEMAPHORE_TAKE;
            eHAL_Ret = halWarp_BlacklevelParameterSet(eBKLV_EVENT_RESET, TRUE);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_CLI_BKLV_LOAD_DATA:
            PALGEO_SEMAPHORE_TAKE;
            eHAL_Ret = halWarping_Blacklevel_LoadData(puInfo->sGFN_CLI_BKLV_LOAD_DATA_INFO.Index);
            PALGEO_SEMAPHORE_GIVE;
            break;

        case eGFN_DRAW_PNG:
            ePAL_Ret = palGeo_DrawPNG(puInfo->sGFN_DRAW_PNG_INFO.eInsertLocation,
                                      puInfo->sGFN_DRAW_PNG_INFO.PosX,
                                      puInfo->sGFN_DRAW_PNG_INFO.PosY,
                                      puInfo->sGFN_DRAW_PNG_INFO.FileName);
            break;

        case eGFN_COLOR_MASK:
            ePAL_Ret = palGeo_ColorMask(puInfo->sGFN_COLOR_MASK_INFO.ColorMask);
            break;

        //////////////////////////////////////
        case eGFN_SET_OSD_PALETTE:  //H2PF_Simon_0038
            ePAL_Ret = palGeo_OnePalette_Set(puInfo->sGFN_SET_OSD_PALETTE_INFO.PaletteIdx,
                                             puInfo->sGFN_SET_OSD_PALETTE_INFO.Red,
                                             puInfo->sGFN_SET_OSD_PALETTE_INFO.Green,
                                             puInfo->sGFN_SET_OSD_PALETTE_INFO.Blue);
            break;

        case eGFN_SET_OSD_PALETTES:  //H2PF_Simon_0038
            ePAL_Ret = palGeo_AllPalette_Set(puInfo->sGFN_SET_OSD_PALETTES_INFO.Red,
                                             puInfo->sGFN_SET_OSD_PALETTES_INFO.Green,
                                             puInfo->sGFN_SET_OSD_PALETTES_INFO.Blue);
            break;

        case eGFN_DRAW_RECT:  //H2PF_Simon_0038
            ePAL_Ret = palGeo_DrawRect(puInfo->sGFN_DRAW_RECT_INFO.eInsertLocation,
                                       puInfo->sGFN_DRAW_RECT_INFO.PosX,
                                       puInfo->sGFN_DRAW_RECT_INFO.PosY,
                                       puInfo->sGFN_DRAW_RECT_INFO.Width,
                                       puInfo->sGFN_DRAW_RECT_INFO.Height,
                                       puInfo->sGFN_DRAW_RECT_INFO.ColorIdx);
            break;

        case eGFN_DRAW_RECTS:
            break;

        case eGFN_DRAW_LINE:  //H2PF_Simon_0038
            ePAL_Ret = palGeo_DrawLine(puInfo->sGFN_DRAW_LINE_INFO.eLocation,
                                       puInfo->sGFN_DRAW_LINE_INFO.Left,
                                       puInfo->sGFN_DRAW_LINE_INFO.Top,
                                       puInfo->sGFN_DRAW_LINE_INFO.Right,
                                       puInfo->sGFN_DRAW_LINE_INFO.Bottom,
                                       puInfo->sGFN_DRAW_LINE_INFO.ColorIndex);
            break;

        case eGFN_DRAW_LINES:
            break;

        case eGFN_DRAW_CIRCLE:  //H2PF_Simon_0038
            palGeo_DrawCircle(puInfo->sGFN_DRAW_CIRCLE_INFO.eLocation,
                              puInfo->sGFN_DRAW_CIRCLE_INFO.PosX,
                              puInfo->sGFN_DRAW_CIRCLE_INFO.PosY,
                              puInfo->sGFN_DRAW_CIRCLE_INFO.Radius,
                              puInfo->sGFN_DRAW_CIRCLE_INFO.CircleColorIndex,
                              puInfo->sGFN_DRAW_CIRCLE_INFO.BGColorIndex);
            break;

        case eGFN_DRAW_COPY_OSD_RECT:  //H2PF_Simon_0038
            palGeo_CopyOSDRect(puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.eLocation,
                               puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.SrcX,
                               puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.SrcY,
                               puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.SrcWidth,
                               puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.SrcHeight,
                               puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.DestX,
                               puInfo->sGFN_DRAW_COPY_OSD_RECT_INFO.DestY);
            break;

        case eGFN_DRAW_CHECKS_ON_OSD:  //H2PF_Simon_0038
            palGeo_DrawChecksOnOSD(puInfo->sGFN_DRAW_CHECKS_ON_OSD_INFO.ucLayerMode,
                                   puInfo->sGFN_DRAW_CHECKS_ON_OSD_INFO.iBlockSize);

            break;

        case eGFN_DRAW_STRING:
            break;

        case eGFN_DRAW_BOXES:  //H2PF_Simon_0038
            palGeo_DrawBoxes(puInfo->sGFN_DRAW_BOXES_INFO.eLocation,
                             puInfo->sGFN_DRAW_BOXES_INFO.PosX,
                             puInfo->sGFN_DRAW_BOXES_INFO.PosY,
                             puInfo->sGFN_DRAW_BOXES_INFO.Width,
                             puInfo->sGFN_DRAW_BOXES_INFO.Height,
                             puInfo->sGFN_DRAW_BOXES_INFO.Red,
                             puInfo->sGFN_DRAW_BOXES_INFO.Green,
                             puInfo->sGFN_DRAW_BOXES_INFO.Blue);
            break;

        case eGFN_CLEAR_OSD:  //H2PF_Simon_0038
            palGeo_ClearOSD(puInfo->sGFN_CLEAR_OSD_INFO.eLocation);
            break;

        case eGFN_DRAW_OSD:  //H2PF_Simon_0038
            palGeo_OSD_On(puInfo->sGFN_DRAW_OSD_INFO.eLocation);
            break;

        case eGFN_FREEZE_IMAGE:  //H2PF_Simon_0038
            palGeo_FreezeImage(puInfo->sGFN_FREEZE_IMAGE_INFO.FreezeEnable);
            break;

        //////////////////////////////////////

        default:
            LOG_MSG(db_HAL_WARPING, "!!! (func:%s, line:%d) unknown function ID %d\n", __FUNCTION__, __LINE__, eGeoFn);
            break;
    }

    if(eHAL_Ret != eHAL_WARPING_EXEC_CODE_NUMBER)
    {
        ePAL_Ret = palGeo_ExeResult(eHAL_Ret);
    }

    if(ePAL_Ret != ePAL_GEO_EXEC_CODE_PASS)
    {
        LOG_MSG(db_HAL_WARPING, "palGeo_Func_Set %d Fail\n", eGeoFn);
    }

    return ePAL_Ret;
}

#if 0
ePAL_GEO_EXEC_CODE palGeo_WarpTableTest(void)
{
    PALGEO_SEMAPHORE_TAKE;
    CalcOsdWarpTable();
    WriteWarpTable(32);
    PALGEO_SEMAPHORE_GIVE;
}
#endif


ePAL_GEO_EXEC_CODE palGeo_DrawFrameMemoryTest(UINT16 PosX, UINT16 PosY, UINT16 Width, UINT16 Height, UINT8 Red, UINT8 Green, UINT8 Blue)
{
    PALGEO_SEMAPHORE_TAKE;
    halWarpingOsd_DrawFrameMemory(PosX, PosY, Width, Height, Red, Green, Blue);
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}

ePAL_GEO_EXEC_CODE palGeo_FrameMemoryCheck(void)  //H2PF_Simon_0131
{
    PALGEO_SEMAPHORE_TAKE;
    halWarping_FrameMemoryProcessCheck(halWarping_IsDefaultWarpTable_Get());
    PALGEO_SEMAPHORE_GIVE;

    return ePAL_GEO_EXEC_CODE_PASS;
}


