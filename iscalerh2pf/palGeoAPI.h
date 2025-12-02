#ifndef _PALGEOAPI_H_
#define _PALGEOAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"
#include "halWarping.h"
#include "cmd_ap.h"

#define  LOG_GEO_FUNC_UNSUPPORTED   LOG_MSG(db_ASSERT, "!!!(funcs:%s,line:%d) function unsupported\r\n", __FUNCTION__, __LINE__);

#define GEO_MEMORY_MAX_FILENAME_LENGTH (128)

typedef enum
{
    ePAL_GEO_EXEC_CODE_PASS,    /* pass */
    ePAL_GEO_EXEC_CODE_FAIL,    /* general fail indication */
    ePAL_GEO_EXEC_CODE_INIT_FAIL,
    ePAL_GEO_EXEC_CODE_NOT_INIT,
    ePAL_GEO_EXEC_CODE_MALLOC_FAIL,
    ePAL_GEO_EXEC_CODE_MUTEX_FAIL,
    ePAL_GEO_EXEC_CODE_INCORRECT_FILE_SIZE,
    ePAL_GEO_EXEC_CODE_FILE_ERROR,
    ePAL_GEO_EXEC_CODE_FILE_CHECKSUM_ERROR,

    ePAL_GEO_EXEC_CODE_NUMBER,
}ePAL_GEO_EXEC_CODE;

typedef enum
{
    eGFT_AP_WARP,
    eGFT_AP_BLEND,
    eGFT_AP_BLACKLEVEL,
    eGFT_OSD_BASIC,
    eGFT_OSD_ADV,
    eGFT_CLI_BLACKLEVEL,

    eGFT_NUMBER,
}ePAL_GEO_FUNCTION_TYPE;


typedef enum
{
    eGMI_MEM_CURRENT = BLENDING_AP_MEM_CUR_IDX,
    eGMI_MEM1 = BLENDING_AP_MEM1_IDX,
    eGMI_MEM2 = BLENDING_AP_MEM2_IDX,
    eGMI_MEM3 = BLENDING_AP_MEM3_IDX,
    eGMI_MEM4 = BLENDING_AP_MEM4_IDX,
    eGMI_MEM5 = BLENDING_AP_MEM5_IDX,

    eGMI_NUMBER
}eGEO_MEM_INDEX;

 //H2PF_Simon_0057
typedef enum
{
    eDRAW_565HIGH_COLOR,   //RGB565
    eDRAW_TRUE_COLOR,      //24-bits (RGB888)

    eDRAW_COLOR_TYPE_INVALID
}eDRAW_COLOR_TYPE;

//advanced warp move pixel
typedef enum
{
	eADV_WARP_MOVE_DIR__UP,
	eADV_WARP_MOVE_DIR__DOWN,
	eADV_WARP_MOVE_DIR__LEFT,
	eADV_WARP_MOVE_DIR__RIGHT,
	eADV_WARP_MOVE_DIR__INVALID,
}eADV_WARP_MOVE_DIR;

typedef enum
{
	eADV_BLEND_DIR__UP,
	eADV_BLEND_DIR__DOWN,
	eADV_BLEND_DIR__LEFT,
	eADV_BLEND_DIR__RIGHT,
	eADV_BLEND_DIR__INVALID,
}eADV_BLEND_DIR;

typedef enum
{
	eADV_BKLV_DIR__UP,
	eADV_BKLV_DIR__DOWN,
	eADV_BKLV_DIR__LEFT,
	eADV_BKLV_DIR__RIGHT,
	eADV_BKLV_DIR__INVALID,
}eADV_BKLV_DIR;

typedef enum
{
    /* 00 */ eGFN_AP_LINK,
    /* 01 */ eGFN_AP_SET_TRANSPARENT_COLOR,

    /* 02 */ eGFN_AP_WARP_FUNCTION_APPLY,    //apply function from current file
    /* 03 */ eGFN_AP_WARP_MEM_APPLY,         //copy to current file and apply
    /* 04 */ eGFN_AP_WARP_SELECT_MEM_APPLY,  //apply specified memory file

    /* 05 */ eGFN_AP_DBDBLEND_FUNCTION_APPLY,    //apply function from current file
    /* 06 */ eGFN_AP_DBDBLEND_MEM_APPLY,         //copy to current file and apply
    /* 07 */ eGFN_AP_DBDBLEND_SELECT_MEM_APPLY,  //apply specified memory file

    /* 08 */ eGFN_AP_BKLV_FUNCTION_APPLY,      //apply function from current file
    /* 09 */ eGFN_AP_BKLV_MEM_APPLY,           //copy to current file and apply
    /* 10 */ eGFN_AP_BKLV_SELECT_MEM_APPLY,    //apply specified memory file

    /* 11 */ eGFN_AP_CLEAR_ALL,

    /* 12 */ eGFN_AP_WARP_CLEAR,
    /* 13 */ eGFN_AP_DBD_BLEND_CLEAR,
    /* 14 */ eGFN_AP_BKLV_CLEAR,
    /* 15 */ eGFN_AP_BKLV_ENABLE,
    /* 16 */ eGFN_AP_BKLV_RESET,
    /* 17 */ eGFN_AP_BKLV_PLT_SET,  //palette set
    /* 18 */ eGFN_CLI_BKLV_AREA_SELECT,
    /* 19 */ eGFN_CLI_BKLV_TL_X,
    /* 20 */ eGFN_CLI_BKLV_TL_Y,
    /* 21 */ eGFN_CLI_BKLV_TR_X,
    /* 22 */ eGFN_CLI_BKLV_TR_Y,
    /* 23 */ eGFN_CLI_BKLV_BL_X,
    /* 24 */ eGFN_CLI_BKLV_BL_Y,
    /* 25 */ eGFN_CLI_BKLV_BR_X,
    /* 26 */ eGFN_CLI_BKLV_BR_Y,
    /* 27 */ eGFN_CLI_BKLV_PLT_R,
    /* 28 */ eGFN_CLI_BKLV_PLT_G,
    /* 29 */ eGFN_CLI_BKLV_PLT_B,
    /* 30 */ eGFN_CLI_BKLV_AREA_ENABLE,
    /* 31 */ eGFN_CLI_BKLV_APPLY,
    /* 32 */ eGFN_CLI_BKLV_APPLY_ALL,
    /* 33 */ eGFN_CLI_BKLV_ENABLE,
    /* 34 */ eGFN_CLI_BKLV_RESET,
    /* 35 */ eGFN_CLI_BKLV_LOAD_DATA,

    /* 36 */ eGFN_DRAW_PNG,
    /* 37 */ eGFN_COLOR_MASK,

    /* 38 */ eGFN_OSD_BASIC,
    /* 39 */ eGFN_OSD_ADV,
    /* 40 */ eGFN_OSD_BLACKLEVEL,

    //Start for proav, use queue to record
    /* 41 */ eGFN_SET_OSD_PALETTE,
    /* 42 */ eGFN_SET_OSD_PALETTES,
    /* 43 */ eGFN_DRAW_RECT,
    /* 44 */ eGFN_DRAW_RECTS,
    /* 45 */ eGFN_DRAW_LINE,
    /* 46 */ eGFN_DRAW_LINES,
    /* 47 */ eGFN_DRAW_CIRCLE,
    /* 48 */ eGFN_DRAW_COPY_OSD_RECT,
    /* 49 */ eGFN_DRAW_CHECKS_ON_OSD,
    /* 50 */ eGFN_DRAW_STRING,
    /* 51 */ eGFN_DRAW_BOXES,

    /* 52 */ eGFN_CLEAR_OSD,
    /* 53 */ eGFN_DRAW_OSD,
    /* 54 */ eGFN_FREEZE_IMAGE,
    //End for proav use queue



    eGFN_NUMBER,
}ePAL_GEO_FUNCTION;


typedef union
{
    struct
    {
        UINT8 APLinkEnable;
    }sGFN_AP_LINK_INFO;

    //========== AP Warp
    struct
    {
        UINT8 PaletteIndex;
    }sGFN_AP_SET_TRANSPARENT_COLOR_INFO;

    struct
    {
        eWARP_FUNC_AP_APPLY_TYPE eType;
    }sGFN_AP_WARP_FUNCTION_APPLY_INFO;

    struct
    {
        UINT8 ApplyIndex;
    }sGFN_AP_WARP_MEM_APPLY_INFO;

    struct
    {
        eGEO_MEM_INDEX eIndex;
        eWARP_FUNC_AP_APPLY_TYPE eType;
    }sGFN_AP_WARP_SELECT_MEM_APPLY_INFO;

    //========== AP DBD
    struct
    {
        eBLEND_FUNC_AP_APPLY_TYPE eType;
    }sGFN_AP_DBDBLEND_FUNCTION_APPLY_INFO;

    struct
    {
        UINT8 ApplyIndex;
    }sGFN_AP_DBDBLEND_MEM_APPLY_INFO;

    struct
    {
        eGEO_MEM_INDEX eIndex;
        eBLEND_FUNC_AP_APPLY_TYPE eType;
    }sGFN_AP_DBDBLEND_SELECT_MEM_APPLY_INFO;

    //========== AP BlackLevel
    struct
    {
        eBLACKLEVEL_FUNC_AP_APPLY_TYPE eType;
    }sGFN_AP_BKLV_FUNCTION_APPLY_INFO;

    struct
    {
        UINT8 ApplyIndex;
    }sGFN_AP_BKLV_MEM_APPLY_INFO;

    struct
    {
        eGEO_MEM_INDEX eIndex;
        eBLACKLEVEL_FUNC_AP_APPLY_TYPE eType;
    }sGFN_AP_BKLV_SELECT_MEM_APPLY_INFO;

    //==========
    #if 0
    struct
    {
    }sGFN_AP_CLEAR_ALL_INFO;

    struct
    {
    }sGFN_AP_WARP_CLEAR_INFO;

    struct
    {
    }sGFN_AP_DBD_BLEND_CLEAR_INFO;

    struct
    {
    }sGFN_AP_BKLV_CLEAR_INFO;
    #endif

    struct
    {
        UINT8 Enable;
    }sGFN_AP_BKLV_ENABLE_INFO;

    struct
    {
        INT32 Val;
    }sGFN_AP_BKLV_PLT_SET_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_AREA_SELECT_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_TL_X_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_TL_Y_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_TR_X_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_TR_Y_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_BL_X_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_BL_Y_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_BR_X_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_BR_Y_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_PLT_R_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_PLT_G_INFO;

    struct
    {
        INT32 Val;
    }sGFN_CLI_BKLV_PLT_B_INFO;

    struct
    {
        INT32 Enable;
    }sGFN_CLI_BKLV_AREA_ENABLE_INFO;

    struct
    {
        INT32 Enable;
    }sGFN_CLI_BKLV_APPLY_INFO;

    struct
    {
        INT32 Enable;
    }sGFN_CLI_BKLV_APPLY_ALL_INFO;

    struct
    {
        INT32 Enable;
    }sGFN_CLI_BKLV_ENABLE_INFO;

    struct
    {
        UINT8 Index;
    }sGFN_CLI_BKLV_LOAD_DATA_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eInsertLocation;
        INT16 PosX;
        INT16 PosY;
        UINT8 *FileName;
    }sGFN_DRAW_PNG_INFO;

    struct
    {
        UINT8 ColorMask;
    }sGFN_COLOR_MASK_INFO;

    struct
    {
        UINT8 PaletteIdx;
        UINT8 Red;
        UINT8 Green;
        UINT8 Blue;
    }sGFN_SET_OSD_PALETTE_INFO;

    struct
    {
        UINT8 Red[256];
        UINT8 Green[256];
        UINT8 Blue[256];
    }sGFN_SET_OSD_PALETTES_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eInsertLocation;
        UINT16 PosX;
        UINT16 PosY;
        UINT16 Width;
        UINT16 Height;
        UINT16 ColorIdx;
    }sGFN_DRAW_RECT_INFO;

    struct
    {
    }sGFN_DRAW_RECTS_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eLocation;
        UINT16 Left;
        UINT16 Top;
        UINT16 Right;
        UINT16 Bottom;
        UINT32 ColorIndex;  //0xRRGGBB (24bits type)
    }sGFN_DRAW_LINE_INFO;

    struct
    {
    }sGFN_DRAW_LINES_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eLocation;
        UINT16 PosX;
        UINT16 PosY;
        UINT16 Radius;
        UINT16 CircleColorIndex;
        UINT16 BGColorIndex;
    }sGFN_DRAW_CIRCLE_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eLocation;
        UINT16 SrcX;
        UINT16 SrcY;
        UINT16 SrcWidth;
        UINT16 SrcHeight;
        UINT16 DestX;
        UINT16 DestY;
    }sGFN_DRAW_COPY_OSD_RECT_INFO;

    struct
    {
        UINT8 ucLayerMode;
        INT16 iBlockSize;
    }sGFN_DRAW_CHECKS_ON_OSD_INFO;

    struct
    {
    }sGFN_DRAW_STRING_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eLocation;
        INT16 PosX;
        INT16 PosY;
        INT16 Width;
        INT16 Height;
        INT16 Red;
        INT16 Green;
        INT16 Blue;
    }sGFN_DRAW_BOXES_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eLocation;
    }sGFN_CLEAR_OSD_INFO;

    struct
    {
        eWARPOSD_INSERT_LOCATION eLocation;
    }sGFN_DRAW_OSD_INFO;

    struct
    {
        UINT8 FreezeEnable;
    }sGFN_FREEZE_IMAGE_INFO;

}uPALGEOAPI_INFO;




typedef struct
{
    ePAL_GEO_FUNCTION eFunc;
    uPALGEOAPI_INFO   uData;
}sDRAW_WAPR_OSD_QUEUE_INFO;

//H2PF_Simon_0057
typedef struct
{
    UINT8 ucColorSpace;   //edcCOLOR_SPACE
    UINT8 ucShowMessage;  //edcSHOW_MESSAGES
    UINT8 ucGeometryEnable;  //edcWARP_TOGGLE
    UINT8 ucAutoSource;  //edcINPUT_KEY
}sDRAW_FRAME_BUFFER_TEMP_DATA;


UINT32 palGeo_TimeElapsedInState(void);
UINT8 palGeo_Is4kWarp(void);
ePAL_GEO_EXEC_CODE palGeo_ExeResult(eHAL_WARPING_EXEC_CODE eResult);
ePAL_GEO_EXEC_CODE palGeo_Initial(void);
ePAL_GEO_EXEC_CODE palGeo_PowerNormal(ePANEL_ID ePanelID);
ePAL_GEO_EXEC_CODE palGeo_ApplyWarpTable(sWarpFileInfo_H2 sFileInfo);
ePAL_GEO_EXEC_CODE palGeo_MemoryFileNameGet(ePAL_GEO_EXEC_CODE eFunction, UINT8 MemIndex, UINT8 *FileName);
ePAL_GEO_EXEC_CODE palGeo_MemoryApplyAP_Warp(UINT8 ucIndex, eWARP_FUNC_AP_APPLY_TYPE eType);
ePAL_GEO_EXEC_CODE palGeo_MemoryApplyAP_Blending(UINT8 ucIndex, eBLEND_FUNC_AP_APPLY_TYPE eType);
ePAL_GEO_EXEC_CODE palGeo_MemoryApplyAP_BlackLevel(UINT8 ucIndex, eBLACKLEVEL_FUNC_AP_APPLY_TYPE eType);
ePAL_GEO_EXEC_CODE palGeo_SetWarp_AutoFilter(UINT8 ucAutoFilter, UINT8 *ucHorzFilter,UINT8 *ucVertFilter);
ePAL_GEO_EXEC_CODE palGeo_SetWarp_ManualFilter(UINT8 ucHorzFilter, UINT8 ucVertFilter);
ePAL_GEO_EXEC_CODE palGeo_SetWarp_AutoFilterParameter(UINT8 ucAutoFilter, UINT8 *ucHorzFilter,UINT8 *ucVertFilter);
ePAL_GEO_EXEC_CODE palGeo_SetWarp_ManualFilterParameter(UINT8 ucHorzFilter, UINT8 ucVertFilter);
ePAL_GEO_EXEC_CODE palGeo_AP_Set_Interpolation_Lut_V_Index(UINT8 ucVertFilterIndex);
ePAL_GEO_EXEC_CODE palGeo_AP_Set_Interpolation_Lut_H_Index(UINT8 ucHorzFilterIndex);
ePAL_GEO_EXEC_CODE palGeo_InterpolationSet(void);
ePAL_GEO_EXEC_CODE palGeo_H_Keystone_Set(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_V_Keystone_Set(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_H_PINBARREL_Set(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_V_PINBARREL_Set(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_TopLeft_X_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_TopLeft_Y_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_TopRight_X_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_TopRight_Y_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_BottomLeft_X_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_BottomLeft_Y_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_BottomRight_X_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_BottomRight_Y_Set(UINT16 Value);
ePAL_GEO_EXEC_CODE palGeo_4corner_Reset(void);
ePAL_GEO_EXEC_CODE palGeo_KeystonePinbarrel4CornerSet(UINT8 *pcKeystoneData,
                                                                  UINT8 *pcPinbarrelData,
                                                                  UINT8 *pc4CornerData);
ePAL_GEO_EXEC_CODE palGeo_WarpingParameterAllGet(UINT8 *pcData);
ePAL_GEO_EXEC_CODE palGeo_Warp_Off(void);
ePAL_GEO_EXEC_CODE palGeo_Egb_Off(void);
ePAL_GEO_EXEC_CODE palGeo_Egb_T_Enable(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_Egb_T_Settings(UINT16 Start, UINT16 Width);
ePAL_GEO_EXEC_CODE palGeo_Egb_B_Enable(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_Egb_B_Settings(UINT16 Start, UINT16 Width);
ePAL_GEO_EXEC_CODE palGeo_Egb_L_Enable(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_Egb_L_Settings(UINT16 Start, UINT16 Width);
ePAL_GEO_EXEC_CODE palGeo_Egb_R_Enable(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_Egb_R_Settings(UINT16 Start, UINT16 Width);
ePAL_GEO_EXEC_CODE palGeo_BlendGamma(UINT8 Value);
ePAL_GEO_EXEC_CODE palGeo_EgbParameterGammaSet(UINT8 *pucEgbParameter,
                                                         UINT8 *pucBlendingGamma);
ePAL_GEO_EXEC_CODE palGeo_BlendParameterAllGet(UINT8 *pcData);
UINT8 palGeo_Color_Uniformity_Init_Status_Get(void);
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Init_Status_Set(UINT8 ucACU_Init);
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Enable_Set(UINT8 ucEnable);
UINT8 palGeo_Color_Uniformity_Enable_Get(void);
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_File_Apply(char *pcFileName);
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Data_Save(UINT8 ucFileNo,  UINT8 *pucData);
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Init(void);
ePAL_GEO_EXEC_CODE palGeo_Color_Uniformity_Reset(void);
ePAL_GEO_EXEC_CODE palGeo_ColorUniformityApplyByOrientation(void);
ePAL_GEO_EXEC_CODE palGeo_ApLinkFlag_Set(BOOL bApLink);
BOOL palGeo_ApLinkFlag_Get(void);
ePAL_GEO_EXEC_CODE palGeo_ClearWarpingTable(void);
ePAL_GEO_EXEC_CODE palGeo_DBD_Blending_Init(void);
ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingTable_Set(UINT16 uiWidth, UINT16 uiHeight, UINT8 *pucDBD_Data);
ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingReset(void);
ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingEnable(UINT8 ucEnable);
ePAL_GEO_EXEC_CODE palGeo_DBD_Blending_EnableGet(UINT8 *pucEnable);
ePAL_GEO_EXEC_CODE palGeo_DBD_BlendingDataClear(void);
ePAL_GEO_EXEC_CODE palGeo_DBD_ClearBlendingTable(void);
ePAL_GEO_EXEC_CODE palGeo_ClearBlackLevel(void);
ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Init(void);
ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Enable(UINT8 ucEnable);
ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Reset(void);
ePAL_GEO_EXEC_CODE palGeo_BlackLevelDataClear(void);
ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Enable(UINT8 ucEnable);
ePAL_GEO_EXEC_CODE palGeo_BlackLevel_Palette_Set(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue);

void palGeo_TwistLinkFlag_Set(BOOL bTwistLink);
BOOL palGeo_TwistLinkFlag_Get(void);

ePAL_GEO_EXEC_CODE palGeo_Save_BasicWarpMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index);
ePAL_GEO_EXEC_CODE palGeo_Apply_BasicWarpMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index);

ePAL_GEO_EXEC_CODE palGeo_Save_BasicBlendMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index);
ePAL_GEO_EXEC_CODE palGeo_Apply_BasicBlendMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index);

ePAL_GEO_EXEC_CODE palGeo_Save_AdvMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index);
ePAL_GEO_EXEC_CODE palGeo_Apply_AdvMemory(eCM_GEO_MEMORY_SAVE_ID CM_Index);

ePAL_GEO_EXEC_CODE palWarpOSD_Config(eGEO_CTRL_INTERFACE eInterface, eWARPOSD_CONFIG eConfig, void *Data, eWARP_OSD_SET_ACTION eAction);
ePAL_GEO_EXEC_CODE palGeo_ColorMask(UINT8 ucColorMask);


ePAL_GEO_EXEC_CODE palGeo_Func_Set(ePAL_GEO_FUNCTION eGeoFn, uPALGEOAPI_INFO *puInfo);
ePAL_GEO_EXEC_CODE palGeo_EnableSet(UINT8 ucEnable);
ePAL_GEO_EXEC_CODE palGeo_CurrentWarpTypeSet(UINT8 ucType);
ePAL_GEO_EXEC_CODE palGeo_Palette_Init(void);

#ifdef CHANGE_PANEL_WORKAROUND
ePAL_GEO_EXEC_CODE palGeo_ADV_WarpingParameterInit(void);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpingInit(ePANEL_ID ucPanel);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpControl(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpShowOsdPattern(UINT8 eType);
ePAT_TYPE palGeo_AdvWarpShowOsdPattern_ParaGet(void);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpShowOsdPattern_ParaSet(ePAT_TYPE eType);
BOOL palGeo_AdvWarpShowOsdPattern_Control(void);// HICC2_Bruce_0024
ePAL_GEO_EXEC_CODE palGeo_Warp_GeometrySet(void);
ePAL_GEO_EXEC_CODE palGeo_Warp_BlendingSet(void);
ePAL_GEO_EXEC_CODE palGeo_BlendingGamma(void);
#endif
ePAL_GEO_EXEC_CODE palGeo_WarpDownScalingIssueWorkAround(void);
ePAL_GEO_EXEC_CODE palGeo_ConfigPanel(const ePANEL_ID ePanelId, const UINT8 ucDualPipe3D);
ePAL_GEO_EXEC_CODE palGeoAdvWarpShowOsdPattern_Unblocked_TwistLink(UINT8 ucValue);   //HICC2_Simon_0008
ePAL_GEO_EXEC_CODE palGeoAdvWarpShowOsdPattern_CheckReApplyBlend(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuOff(void);  //H2PF_Simon_0030
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuWhite(void);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuBlack(void);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuGrid(void);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuGridCenter(void);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuBlackBorder(void);
ePAL_GEO_EXEC_CODE palGeo_TestPatternCuGridSelect(sCOLOR_UNIFORMITY_POINT_SELECT sColorUniformityPoint);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuExitBlackBorderHandle(void);
ePAL_GEO_EXEC_CODE palGeo_TestPatternAcuBlackBorderTargetSelPostionTmpSet(UINT8 ucACU_Target_Tmp);
ePAL_GEO_EXEC_CODE palGeo_OSD_On(eWARPOSD_INSERT_LOCATION eLocation);
ePAL_GEO_EXEC_CODE palGeo_OnePalette_Set(UINT8 PaletteIdx, UINT8 Red, UINT8 Green, UINT8 Blue);
ePAL_GEO_EXEC_CODE palGeo_AllPalette_Set(UINT8 *Red, UINT8 *Green, UINT8 *Blue);
ePAL_GEO_EXEC_CODE palGeo_DrawRect(eWARPOSD_INSERT_LOCATION eLocation,
                                        UINT16 uiPosX,
                                        UINT16 uiPosY,
                                        UINT16 uiWidth,
                                        UINT16 uiHeight,
                                        UINT16 uiColorIdx);

ePAL_GEO_EXEC_CODE palGeo_DrawLine(eWARPOSD_INSERT_LOCATION eLocation,
                                         UINT16 uiLeft,
                                         UINT16 uiTop,
                                         UINT16 uiRight,
                                         UINT16 uiBottom,
                                         UINT32 uiColorIndex);

ePAL_GEO_EXEC_CODE palGeo_DrawCircle(eWARPOSD_INSERT_LOCATION eLocation,
                                     UINT16 uiPosX,
                                     UINT16 uiPosY,
                                     UINT16 uiRadius,
                                     UINT16 uiCircleColorIndex,
                                     UINT16 uiBGColorIndex);

ePAL_GEO_EXEC_CODE palGeo_DrawPNG(eWARPOSD_INSERT_LOCATION eLocation,
                                       UINT16 uiPosX,
                                       UINT16 uiPosY,
                                       UINT8 *FileName);

ePAL_GEO_EXEC_CODE palGeo_DrawMulitSprites(sSPRITES_INFO_DATA *psSpritesInfo);


ePAL_GEO_EXEC_CODE palGeo_DrawMulitSpirites(sSPRITES_INFO_DATA *psSpritesInfo);

ePAL_GEO_EXEC_CODE palGeo_CopyOSDRect(eWARPOSD_INSERT_LOCATION eLocation,
                                             UINT16 uiSrcX,
                                             UINT16 uiSrcY,
                                             UINT16 uiSrcWidth,
                                             UINT16 uiSrcHeight,
                                             UINT16 uiDestX,
                                             UINT16 uiDestY);

ePAL_GEO_EXEC_CODE palGeo_DrawChecksOnOSD(UINT8 ucLayerMode, INT16 iBlockSize);

ePAL_GEO_EXEC_CODE palGeo_DrawCheckboard(eWARPOSD_INSERT_LOCATION eLocation,
                                                 INT16 iSizeX,
                                                 INT16 iSizeY,
                                                 INT16 iShiftX,
                                                 INT16 iShiftY);

ePAL_GEO_EXEC_CODE palGeo_DrawBoxes(eWARPOSD_DRAW_TYPE eType,
                                          INT16 iPositionX,
                                          INT16 iPositionY,
                                          INT16 iWidth,
                                          INT16 iHeight,
                                          INT16 iRed,
                                          INT16 iGreen,
                                          INT16 iBlue);

ePAL_GEO_EXEC_CODE palGeo_FreezeImage(UINT8 ucEnable);
ePAL_GEO_EXEC_CODE palGeo_DrawSolidColor(UINT32 Red, UINT32 Green, UINT32 Blue);
ePAL_GEO_EXEC_CODE palGeo_DrawMultiCircle(sWARPOSD_CIRCLE_GRID_DRAW_INFO sCircleGridInfo);
ePAL_GEO_EXEC_CODE palGeo_AP_Ifill(INT16 iEnable, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue);
ePAL_GEO_EXEC_CODE palGeo_AP_OSD_Init(void);
ePAL_GEO_EXEC_CODE palGeo_InfoPrint(void);
UINT16 palGeo_QueueDataCountGet(void);
eRESULT palGeo_DrawOSDQueueSet(ePAL_GEO_FUNCTION eGeoFn, uPALGEOAPI_INFO uInfo);
eRESULT palGeo_DrawWarpingOSDQueue(void);
ePAL_GEO_EXEC_CODE palGeo_ClearOSD(eWARPOSD_INSERT_LOCATION eLocation);
BOOL palGeo_IsDraw_OSD(eWARPOSD_INSERT_LOCATION eDrawType);   //H2PF_Simon_0040
ePAL_GEO_EXEC_CODE palGeo_Draw_PNG_Pattern(UINT8 ucPatternIndex, eDRAW_COLOR_TYPE eColorType, char *FileName); //H2PF_Simon_0057
ePAL_GEO_EXEC_CODE palGeo_Draw_PNG_PatternChange(UINT8 ucPatternIndex);
ePAL_GEO_EXEC_CODE palGeo_Draw_PNG_Pattern_Off(void); //H2PF_Simon_0057
ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferColorSpace(UINT8 ucColorSpace);
ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferShowMessage(UINT8 ucShowMessage);
ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferGeometryEnable(UINT8 ucGeometryEnable);
ePAL_GEO_EXEC_CODE palGeo_Draw_FrameBufferAutoSource(UINT8 ucAutoSource);
ePAL_GEO_EXEC_CODE palGeo_convertTPtoDTDT(FLOAT *tpx, FLOAT *tpy, INT16 width, INT16 height);
ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearAPTempFile(void);
ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearSaveFile(void);
ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearCurrentFile(void);
ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearCurrentWarpFile(void);
ePAL_GEO_EXEC_CODE palGeo_MemoryAP_ClearCurrentBlendFile(void);
ePAL_GEO_EXEC_CODE palGeo_AdvResetWarping(void);

ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetColIndex(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetColIndex(UINT8 *pValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetRowIndex(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetRowIndex(UINT8 *pValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_Move_Set(eADV_WARP_MOVE_DIR eDir, UINT16 uiPixel);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetColPosition(INT32 *pValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_GetRowPosition(INT32 *pValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetPosition(INT32 lPosX, INT32 lPosY);
ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SettingReset(void);
void palGeo_BlacklevelClearCurrentData(void);
UINT8 palGeo_AdvWarp_GetShowBlendOnWarpPattern(void);
ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SetShowBlendOnWarpPattern(UINT8 ucValue);
UINT8 palGeo_AdvWarp_GetDrawOverlapGridOnWarpPattern(void);
ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SetDrawOverlapGridOnWarpPattern(UINT8 ucValue);
UINT8 palGeo_AdvBlend_GetBlendPatternState(void);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_Enable_Set(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_Enable_Get(UINT8 *pValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_OverlapSize_Set(eADV_BLEND_DIR eDir, UINT16 uiValue);
UINT16 palGeo_AdvBlend_OverlapSize_Get(eADV_BLEND_DIR eDir);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_OverlapOffset_Set(eADV_BLEND_DIR eDir, UINT16 uiValue);
UINT16 palGeo_AdvBlend_OverlapOffset_Get(eADV_BLEND_DIR eDir);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_ConfigReset(void);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SelectControlPoint(eADV_WARP_MOVE_DIR eDir);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_SelectOsdBlendWidth(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_OverlapGridNumber(UINT8 ucValue);    //H2PF_Simon_0108
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGridPoints(UINT8 ucGridPoints, UINT8 ucInner);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpInnerGridPoints(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvResetWarping(void);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpSharpness(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGridColor(UINT8 ucValue, UINT8 ucRedrawPattern);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpBackgroundColor(UINT8 ucValue, UINT8 ucRedrawPattern);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_SetAllPosition(Point2f Data[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V]);
ePAL_GEO_EXEC_CODE palGeo_AdvWarp_SelectControlPoint(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvWarpGP_MoveControlPoint(eADV_WARP_MOVE_DIR eDir);
ePAL_GEO_EXEC_CODE palGeo_AdvBlend_AdjustOsdBlendWidth(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_DrawFrameMemoryTest(UINT16 PosX, UINT16 PosY, UINT16 Width, UINT16 Height, UINT8 Red, UINT8 Green, UINT8 Blue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetEnable(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_AddPoint(void);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_DeletePoint(void);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SelectControlPoint(eADV_BKLV_DIR eDir);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_MoveControlPoint(eADV_BKLV_DIR eDir);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SelectArea(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetRed(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetGreen(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_SetBlue(UINT8 ucValue);
ePAL_GEO_EXEC_CODE palGeo_AdvHide_OSD_Pattern_Set(BOOL bHide);
ePAL_GEO_EXEC_CODE palGeo_AdvBlacklevel_Boundary_Pattern_Hide(BOOL bHide);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_Boundary_Flag_Set(UINT8 ucBoundary);
ePAL_GEO_EXEC_CODE palGeo_AdvBlackLevel_Reset(UINT8 ucResetLevel);
UINT8 halWarping_IsDefaultWarpTable_Get(void);
ePAL_GEO_EXEC_CODE palGeo_FrameMemoryCheck(void);

#ifdef __cplusplus
}
#endif

#endif  //_PALGEOAPI_H_

