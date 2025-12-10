#ifndef _HALWARPING_H_
#define _HALWARPING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"
#include "utilWarpDemo.h"		//G100_Doulas_0027
#include "cmd_ap.h"


#define DEF_BLENDING_EDGE_HW	    200
#define DEF_BLENDING_EDGE_VW	    200

#define DEF_TRANSPARENT_COLOR_IDX	0  //H2PF_Simon_0038

#define BLENDING_GAMMA_DMA	        99

#define PNG_OSD_NO_TRANSPARENT_COLOR  //A35G2_Simon_0067

//A70LV_Larry_0139 start
#define WARPING_MEMORY_GROUP_NUMBER      (5)
#define WARPING_MEMORY_GROUP_SIZE        0x00400000  //0x00010000
#define WARPING_DISTORTION_TABLE_ADDRESS 0x00000000
#define WARPING_DISTORTION_TABLE_SIZE    0x0000A000
#define WARPING_BLENDING_TABLE_ADDRESS   0x0000B000
#define WARPING_BLENDING_TABLE_SIZE      0x00000800
#define WARPING_BLENDING_EDGE_ADDRESS    0x0000BB00
#define WARPING_BLENDING_EDGE_SIZE       0x00000042
#define WARPING_INT_TABLE_ADDRESS        0x0000C000
#define WARPING_INT_TABLE_SIZE           0x00000064
#define WARPING_SAVE_TYPE_ADDRESS        0x0000FFF0

#define WARPING_BLD_GROUP_MYSTIQUE_NUMBER    (5)
#define WARPING_BLD_GROUP_MYSTIQUE_SIZE      0x00300000
#define WARPING_BLD_TABLE_MYSTIQUE_ADDRESS   0x012F0000
#define WARPING_BLD_TABLE_MYSTIQUE_SIZE      0x00000800
#define WARPING_BLD_EDGE_MYSTIQUE_ADDRESS    0x012FBB00
#define WARPING_BLD_EDGE_MYSTIQUE_SIZE       0x00000042
#define WARPING_BLD_EDGE_MYSTIQUE_TYPE       0x012FFFF0

#define WARPING_TYPE_NA                     (0)
#define WARPING_TYPE_OSD                    (0xB0)    //Advanced
#define WARPING_TYPE_AP                     (0xA0)    //R70G2_Simon_0001
#define WARPING_TYPE_OSD_MYSTIQUE           (0xB1)    //Basic
#define WARPING_TYPE_AP_MYSTIQUE            (0xA1)    //Twist

#define WARPING_TWIST_GROUP_NUMBER          (5)
#define WARPING_TWIST_ADDRESS               0x01000000
#define WARPING_TWIST_GROUP_SIZE            0x00300000

#define WARPING_TWIST_BLEND_ADDRESS         0x01000000
#define WARPING_TWIST_BLEND_SIZE            0x00260000
#define WARPING_TWIST_GAMMA_TABLE_ADDRESS   0x012FB000
#define WARPING_TWIST_GAMMA_TABLE_SIZE      0x00000800
#define WARPING_TWIST_EGBCT_ADDRESS         0x012FBC00
#define WARPING_TWIST_EGBCT_SIZE            0x00000006
#define WARPING_TWIST_BLEND_BLOCK           0x012F0000
#define WARPING_TWIST_BLEND_TYPE            0x012FFFF0

#define WARPING_ACU_INIT_NOT_READY          0               //G100_Tim_0020, add
#define WARPING_ACU_INIT_DONE               1               //G100_Tim_0020, add

#define BLENDING_ONLY_COLOR_UNIFORMITY //A70LV_Julie_0047 enable //A70LV_John_0133 disabled until ready//A70LV_John_0126 Color Uniformity is only enabled in blending mode
#define WARPING_COLOR_UNIFORMITY_ADDR       0x01F00000
#define WARPING_COLOR_UNIFORMITY_SIZE       0x0000076C
#define WARPING_COLOR_UNIFORMITY_UCCT_ADDR  0x01F01000
#define WARPING_COLOR_UNIFORMITY_UCCT_SIZE  0x00000006
#define WARPING_COLOR_UNIFORMITY_UCCT_TYPE  0x01F0FFF0
#define WARPING_COLOR_UNIFORMITY_UCCT_TAG   (0xA1)

#define WARPING_COLOR_UNIFORMITY_Offset_0   0x00000000      // Normal
#define WARPING_COLOR_UNIFORMITY_Offset_1   0x00010000      // Ceil On ( Top Left to Bottom Right )
#define WARPING_COLOR_UNIFORMITY_Offset_2   0x00020000      // Rear On ( Left to Right )
#define WARPING_COLOR_UNIFORMITY_Offset_3   0x00030000      // Both On ( Top to Bottom )
#define WARPING_COLOR_UNIFORMITY_Offset_F   0x000F0000      // Unknown


//Black Level
#define WARPING_TWIST_BIAS_TABLE_ADDRESS    0x00250000  //0x00270000//0x00250000
#define WARPING_TWIST_BIAS_TABLE_SIZE       0x00140000  //0x0012C000//0x00130000

#define WARPING_TWIST_BIAS_GAMMA_SIZE       0x00000100
#define WARPING_TWIST_BIAS_GAMMA_ADDR_R     0x003AA000//0x0038A000
#define WARPING_TWIST_BIAS_GAMMA_ADDR_G     WARPING_TWIST_BIAS_GAMMA_ADDR_R + WARPING_TWIST_BIAS_GAMMA_SIZE
#define WARPING_TWIST_BIAS_GAMMA_ADDR_B     WARPING_TWIST_BIAS_GAMMA_ADDR_G + WARPING_TWIST_BIAS_GAMMA_SIZE

#define WARPING_TWIST_BIAS_EGBIASCT_ADDRESS 0x003AB000//0x0038B000
#define WARPING_TWIST_BIAS_EGBIASCT_SIZE    0x00000006
#define WARPING_TWIST_BIAS_FLAG_ADD         0x003A0000//0x00380000
#define WARPING_TWIST_BIAS_FLAG             0x003AFFF0//0x0038FFF0


#ifdef C821_WARPING_ENABLE
#define WARPING_DISTORTION_CORRECTION_TABLE_A (0x0000)
#define WARPING_DISTORTION_CORRECTION_TABLE_B (0x0010)
#define WARPING_DISTORTION_ACCESS_TABLE_A     (0x0100)
#define WARPING_DISTORTION_ACCESS_TABLE_B     (0x0200)
#endif /* C821_WARPING_ENABLE */

#ifdef C789_WARPING_ENABLE
#define WARPING_DISTORTION_CORRECTION_TABLE_A (0x0000)
#define WARPING_DISTORTION_CORRECTION_TABLE_B (0x0010)
#define WARPING_DISTORTION_ACCESS_TABLE_A     (0x0040)
#define WARPING_DISTORTION_ACCESS_TABLE_B     (0x0080)
#endif /* C789_WARPING_ENABLE */
//A70LV_Larry_0139 end

//A70LV_Larry_0051 start
#define _CASCADE_

//===== Projection system parameters =====//
#define DEF_PJDIST      2080  //1220    // Projection distance [mm] //A35G2_Coda_0129
#define DEF_PJVW        1000  // Height of the image [mm]
#define DEF_PJAXIS      0   // Optical axis (0:center, 1:bottom)

#define DEF_MAX_LONG    0x7fffffffL
#define DEF_PI          3.14159265f

//===== Warping dummy area =====//
#define DEF_WPDMYHWF    0      // warp DUMMY area(front)
#define DEF_WPDMYHWB    0       // warp DUMMY area(back)
#define DEF_WP_SPACE    16

//===== LUT =====//
#define DEF_LUTSYM      9           // LUT symbol default
#define DEF_LUTCUT_MAX  1.2f        // LUT cutoff max
#define DEF_LUTCUT_MIN  0.3f        // LUT cutoff min
#define DEF_LUTWIN_MAX  2.0f        // LUT window max
#define DEF_LUTWIN_MIN  0           // LUT window min

//===== Warping table =====//
#ifdef _CASCADE_
#define DEF_WPTBLNUM        2   // Warping table number
#define DEF_WPTBLIDX_OLD    0   // Old table
#define DEF_WPTBLIDX_NEW    1   // New table
#else
#define DEF_WPTBLNUM        1
#define DEF_WPTBLIDX_OLD    0
#define DEF_WPTBLIDX_NEW    0
#endif
#define DEF_WPTBLMAX_H  128     // Horizontal maximum table size
#define DEF_WPTBLMAX_V  80      // Vertical maximum table size

//===== Warping mode =====//
#define DEF_WPMODE_TH   0       // No warp
#define DEF_WPMODE_KANG 1       // Keystone(Angle)
#define DEF_WPMODE_K4C  2       // Keystone(4-corner)
#define DEF_WPMODE_PIN  3       // Pincushion/Barrel
#define DEF_WPMODE_ARC  4       // ARC
#define DEF_WPMODE_ROT  5       // Rotation

//===== Warping limit =====//
//#define DEF_WPLIMANG_H  0.70        // Limitation of H-slope : tan(35deg)
#define DEF_WPLIMANG_H  1.00        // Limitation of H-slope : tan(45deg)
#define DEF_WPLIMANG_V  5.67        // Limitation of V-slope : tan(80deg)
#define DEF_WPLIMVSH_L  53      // Limitation of local V-shrink rate : 16/0.33
//#define DEF_WPLIMVSH_A  32      // Limitation of average V-shrink rate : 16/0.5
#define DEF_WPLIMVSH_A  48      // Limitation of average V-shrink rate : 16/0.33
#define DEF_WPLIMHSH    192         // Limitation of H-shrink rate : 16/(1/12)
#define DEF_WPERRANG_H  1       // Error of H-slope
#define DEF_WPERRANG_V  2       // Error of V-slope
#define DEF_WPERRVSH_L  3       // Error of local V-shrink rate
#define DEF_WPERRVSH_A  4       // Error of average V-shrink rate
#define DEF_WPERRHSH    5       // Error of H-shrink rate
#ifdef _CASCADE_
#define DEF_WPTBLNUM        2   // Warping table number
#define DEF_WPTBLIDX_OLD    0   // Old table
#define DEF_WPTBLIDX_NEW    1   // New table
#else
#define DEF_WPTBLNUM        1
#define DEF_WPTBLIDX_OLD    0
#define DEF_WPTBLIDX_NEW    0
#endif

//===== 4 Corner Based =====//
#define FOUR_CORNER_Based_H 1920
#define FOUR_CORNER_Based_V 1200

//===== Calualte based =====//
#define KEYSTONE_BASED  20.0
#define PINBARREL_BASED 50.0
//A70LV_Larry_0051 end

#define WARP_TABLE_A    0
#define WARP_TABLE_B    1
//#define DEF_BLEND_ADDRESS   0x05000000
#define MAX_HORZ_RES    1920
#define MAX_VERT_RES    1200
//#define DEF_EBIASSAD        0x06800000
//#define DEF_EBIASMWI        0x04

#define DEFAULT_WAPR_FILTER_LUT_H   (7)     //A35G2_Simon_0110
#define DEFAULT_WAPR_FILTER_LUT_V   (7)

#define WAPR_FILTER_H_MAX POLATION_MAX
#define WAPR_FILTER_V_MAX POLATION_MAX

#define MAX_BLACKLEVEL_AREA_NUMBER      (15)
#define BLACKLEVEL_CURRENT_INDEX        (WARPING_APPLY_MAX_VALUE + 1)  //A35G2_Simon_0093


typedef struct
{
    UINT8 index;
    UINT8 red;
    UINT8 green;
    UINT8 blue;
}sPaletteSingleInfo;

typedef enum
{
    eWOR_ALL,
    eWOR_Palette,
    eWOR_Transparent,
    eWOR_Blink,

    eWOR_Number
}eWARPOSD_RELOAD_TYPE;

typedef enum
{
    eTC_SLOT0,
    eTC_SLOT1,
    eTC_SLOT2,
    eTC_SLOT3,

    eTC_SLOT_INVALID,
}eTRANSPARENT_COLOR_SLOT;

//for Barco command only Blacklevel
typedef struct
{
    UINT8   ucBlackLevel_AreaEnable;
    INT16   uiBlackLevel_CUR_TL_X;  //top Left cursor X
    INT16   uiBlackLevel_CUR_TL_Y;  //top Left cursor Y
    INT16   uiBlackLevel_CUR_TR_X;  //top right cursor X
    INT16   uiBlackLevel_CUR_TR_Y;  //top right cursor Y
    INT16   uiBlackLevel_CUR_BL_X;  //bottom Left cursor X
    INT16   uiBlackLevel_CUR_BL_Y;  //bottom Left cursor Y
    INT16   uiBlackLevel_CUR_BR_X;  //bottom right cursor X
    INT16   uiBlackLevel_CUR_BR_Y;  //bottom right cursor Y
    INT8    ucBlackLevel_PLT_SEL;
    UINT8   ucBlackLevel_PLT_R;
    UINT8   ucBlackLevel_PLT_G;
    UINT8   ucBlackLevel_PLT_B;
    UINT8   ucBlackLevel_ACTIVE;

}sBLACKLEVEL_PARA;


typedef struct
{
	float	fAxis;
	float	fDist;
	float	fVw;
}sHALWARPING_PJPARAM, *PsHALWARPING_PJPARAM;


//======================= Error Code Start ======================//
typedef enum
{
    eHAL_WARPING_EXEC_CODE_PASS,    /* pass */
    eHAL_WARPING_EXEC_CODE_FAIL,    /* general fail indication */
    eHAL_WARPING_EXEC_CODE_INIT_FAIL,
    eHAL_WARPING_EXEC_CODE_NOT_INIT,
    eHAL_WARPING_EXEC_CODE_MALLOC_FAIL,
    eHAL_WARPING_EXEC_CODE_MUTEX_FAIL,

    eHAL_WARPING_EXEC_CODE_NUMBER,
}eHAL_WARPING_EXEC_CODE;

typedef enum
{
    eHAL_WARPING_CROSSHATCH_MODE_OFF,
    eHAL_WARPING_CROSSHATCH_MODE_SINGLE,
    eHAL_WARPING_CROSSHATCH_MODE_OVERLAY,
    eHAL_WARPING_CROSSHATCH_MODE_LAST,
}eHAL_WARPING_CROSSHATCH_MODE;

typedef enum
{
    HAL_WARPING_WARP_MODE_2x2,
    HAL_WARPING_WARP_MODE_3x3,
    HAL_WARPING_WARP_MODE_5x5,
    HAL_WARPING_WARP_MODE_9x9,
    HAL_WARPING_WARP_MODE_17x17,
}eHAL_WARPING_WARP_MODE;

typedef enum
{
    eHAL_WARPING_DIRECTION_UP,
    eHAL_WARPING_DIRECTION_DOWN,
    eHAL_WARPING_DIRECTION_LEFT,
    eHAL_WARPING_DIRECTION_RIGHT,
    eHAL_WARPING_DIRECTION_LAST,
}eHAL_WARPING_DIRECTION;

typedef enum
{
    eHAL_WARPING_MOVE_PITCH_100,
    eHAL_WARPING_MOVE_PITCH_50,
    eHAL_WARPING_MOVE_PITCH_10,
    eHAL_WARPING_MOVE_PITCH_5,
    eHAL_WARPING_MOVE_PITCH_1,
    eHAL_WARPING_MOVE_PITCH_0P5,
    eHAL_WARPING_MOVE_PITCH_0P1,
    eHAL_WARPING_MOVE_PITCH_0P0625,
    eHAL_WARPING_MOVE_PITCH_LAST,
}eHAL_WARPING_MOVE_PITCH; //A70LV_Larry_0137


//A70LV_Larry_0051 start
typedef enum
{
    eWARP_EVENT_WARPOFF,  //Reset //Init
    eWARP_EVENT_KEYSTONE_H,
    eWARP_EVENT_KEYSTONE_V,
    eWARP_EVENT_KEYSTONE_HV,  //A70_Eva_0007
    eWARP_EVENT_PINBARREL_H,
    eWARP_EVENT_PINBARREL_V,
    eWARP_EVENT_PINBARREL_HV,
    eWARP_EVENT_4C_TL_X,
    eWARP_EVENT_4C_TL_Y,
    eWARP_EVENT_4C_TR_X,
    eWARP_EVENT_4C_TR_Y,
    eWARP_EVENT_4C_BL_X,
    eWARP_EVENT_4C_BL_Y,
    eWARP_EVENT_4C_BR_X,
    eWARP_EVENT_4C_BR_Y,
    eWARP_EVENT_4C_ALL,
    eWARP_EVENT_CURVE_L,
    eWARP_EVENT_CURVE_R,
    eWARP_EVENT_CURVE_T,
    eWARP_EVENT_CURVE_B,
    eWARP_EVENT_ROTATION,

    eWARP_EVENT_DEFAULT_FILTER_H, //A70LV_Larry_0220
    eWARP_EVENT_DEFAULT_FILTER_V, //A70LV_Larry_0220

    eWARP_EVENT_WARP_H_FILTER_USER_SEL,
    eWARP_EVENT_WARP_V_FILTER_USER_SEL,
    eWARP_EVENT_WARP_H_FILTER_AP_SEL,
    eWARP_EVENT_WARP_V_FILTER_AP_SEL,
    eWARP_EVENT_USER_WARP_FILTER_AUTO_ENABLE,
    eWARP_EVENT_AP_WARP_FILTER_AUTO_ENABLE,
    eWARP_EVENT_WARP_PARA,

    eWARP_EVENT_NUMBER,
} eWARP_EVENT;

typedef enum
{
    eBLENDING_EVENT_OFF,
    eBLENDING_EVENT_T_ENABLE,
    eBLENDING_EVENT_B_ENABLE,
    eBLENDING_EVENT_L_ENABLE,
    eBLENDING_EVENT_R_ENABLE,
    eBLENDING_EVENT_T_ST,
    eBLENDING_EVENT_B_ST,
    eBLENDING_EVENT_L_ST,
    eBLENDING_EVENT_R_ST,
    eBLENDING_EVENT_T_WIDTH,
    eBLENDING_EVENT_B_WIDTH,
    eBLENDING_EVENT_L_WIDTH,
    eBLENDING_EVENT_R_WIDTH,
    eBLENDING_EVENT_GAMMA,
    eBLENDING_EVENT_GAMMA_DMA,
    eBLENDING_EVENT_ALL,

    eBLENDING_EVENT_NUMBER,
}eBLENDING_EVENT;
//A70LV_Larry_0051 end


typedef enum
{
    eBKLV_EVENT_AREA_SEL,

    eBKLV_EVENT_CUR_TL_X,
    eBKLV_EVENT_CUR_TL_Y,
    eBKLV_EVENT_CUR_TR_X,
    eBKLV_EVENT_CUR_TR_Y,
    eBKLV_EVENT_CUR_BL_X,
    eBKLV_EVENT_CUR_BL_Y,
    eBKLV_EVENT_CUR_BR_X,
    eBKLV_EVENT_CUR_BR_Y,

    eBKLV_EVENT_PLT_RED,
    eBKLV_EVENT_PLT_GREEN,
    eBKLV_EVENT_PLT_BLUE,

    eBKLV_EVENT_AREA_ENABLE,

    eBKLV_EVENT_APPLY,
    eBKLV_EVENT_APPLY_ALL_AREA,

    eBKLV_EVENT_ENABLE,
    eBKLV_EVENT_RESET,

}eBLACKLEVEL_PARA;




//==================================================================================

typedef struct
{
    UINT8 ucEnableFlag;
    UINT8 uc3DFlag;
    UINT16 ucWidth;
    UINT16 ucHeight;
    UINT8 aucBiasPalette[48];
    sBLACKLEVEL_PARA asBlackLevelPara[MAX_BLACKLEVEL_AREA_NUMBER];   //A35G2_Simon_0093
    UINT8 aucRawDataFileName[64];   //A35G2_Simon_0093
}sBlackLevelFileData;

//R70G2_Simon_0001
typedef enum
{
    eWDT_BEFORE_WARP,
    eWDT_AFTER_WARP,

    eWDT_INVALID,
}eWARPOSD_DRAW_TYPE;


//A65_OPTOMA_Simon_0002
typedef union
{
    UINT8 ucData[32];

    struct
    {
        UINT8 ucPanelID;
        UINT8 ucManualWarpFilterH;    //A35G2_Simon_0110
        UINT8 ucManualWarpFilterV;
    }sInfo;

}sCurrentFileInfo;





BOOL halWarping_SemaphoreTake(const char *pcFunc);
BOOL halWarping_SemaphoreGive(const char *pcFunc);

//======================= Error Code End =======================//
eHAL_WARPING_EXEC_CODE halWarping_Init(void);   //(const eHAL_WARPING_WARP_MODE eMode,  const eHAL_WARPING_MOVE_PITCH eMoveIdx);//Init 4 channels in once, no matter 4K, 2K


eHAL_WARPING_EXEC_CODE halWarping_PowerStandby(void);
eHAL_WARPING_EXEC_CODE halWarping_PowerNormal(const ePANEL_ID ePanelId);

eHAL_WARPING_EXEC_CODE halWarping_SetWarp_AutoFilter(UINT8 ucAutoFilter,UINT8 *ucHorzFilter,UINT8 *ucVertFilter);
eHAL_WARPING_EXEC_CODE halWarping_SetWarp_ManualFilter(UINT8 ucHorzFilter,UINT8 ucVertFilter);
eHAL_WARPING_EXEC_CODE halWarping_SetWarp_AutoFilterParameter(UINT8 ucAutoFilter,UINT8 *ucHorzFilter,UINT8 *ucVertFilter);	//G100_Doulas_0027
eHAL_WARPING_EXEC_CODE halWarping_SetWarp_ManualFilterParameter(UINT8 ucHorzFilter,UINT8 ucVertFilter);		//G100_Doulas_0027


eRESULT halWarp_GeometrySet(void); //A70LV_Larry_0051
void halWarp_GeometryParameterSet(eWARP_EVENT eType, void *pcData); //A70LV_Larry_0051
void halWarp_GeometryParameterGet(eWARP_EVENT eType, void *pcData); //A70LV_Larry_0220
eHAL_WARPING_EXEC_CODE halWarp_InterpolationSet(void); //A70LV_Larry_0051  //G100_Simon_0089

void halWarp_BlendingParameterSet(eBLENDING_EVENT eType, void *pcData); //A70LV_Larry_0051
void halWarp_BlendingParameterGet(eBLENDING_EVENT eType, void *pcData);
eHAL_WARPING_EXEC_CODE halWarp_BlendingSet(void); //A70LV_Larry_0051  //G100_Simon_0089
eHAL_WARPING_EXEC_CODE halWarp_BlendingGamma(void); //A70LV_Larry_0171  //G100_Simon_0089
//void halWarping_CursorPanelConfig(void); //A70LV_Larry_0137

void halWarping_TwistLinkFlag_Set(BOOL bTwistLink); //A70LV_Larry_0319
BOOL halWarping_TwistLinkFlag_Get(void); //A70LV_Larry_0319
eHAL_WARPING_EXEC_CODE halWarping_EnableSet(UINT8 ucEnable);
void halWarping_Color_Uniformity_Save(void);
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Set(UINT8 *pcData, UINT16 uiSize);
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Table_Set(char *pcFileName);

// AP API Start
eHAL_WARPING_EXEC_CODE halWarping_convertTPtoDTDT(FLOAT *tpx, FLOAT *tpy, UINT16 width, UINT16 height);
eHAL_WARPING_EXEC_CODE halWarping_ClearWarpingTable(void);
eHAL_WARPING_EXEC_CODE halWarping_EdgeBlending_Disable(void);
eHAL_WARPING_EXEC_CODE halWarping_DBD_BlendingTable_Set(UINT16 uiWidth, UINT16 uiHeight, UINT8 *pucDBD_Data);
eHAL_WARPING_EXEC_CODE halWarping_DBD_ClearBlendingTable(void);
eHAL_WARPING_EXEC_CODE halWarping_clearBlackLevel(void);
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Enable(UINT8 ucEnable);
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Table_Set(UINT8 *ucData, UINT16 uiWidth, UINT16 uiHeight);
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Palette_Set(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue);
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Set(INT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue);
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_SetAll(UINT8 *aucRed, UINT8 *aucGreen, UINT8 *aucBlue);
eHAL_WARPING_EXEC_CODE halWarpOSD_ClearOSD(void);
eHAL_WARPING_EXEC_CODE halWarpOSD_DisableOSD(void);
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Palette_SaveFile(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue);

eHAL_WARPING_EXEC_CODE halWarp_BlacklevelParameterSet(eBLACKLEVEL_PARA eParameter, INT32 lData);
INT32 halWarp_BlacklevelParameterGet(eBLACKLEVEL_PARA eParameter);
void halWarping_Blacklevel_AreaApply(              INT8 cPaletteSelect,
                                                   INT16 iCursor_TL_X,
                                                   INT16 iCursor_TL_Y,
                                                   INT16 iCursor_TR_X,
                                                   INT16 iCursor_TR_Y,
                                                   INT16 iCursor_BL_X,
                                                   INT16 iCursor_BL_Y,
                                                   INT16 iCursor_BR_X,
                                                   INT16 iCursor_BR_Y);

void halWarping_Blacklevel_ApplyAllArea(void);
void halWarping_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue);
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Reset(void);
void halWarping_Blacklevel_Debug(void);
void halWarping_Blacklevel_Init(void);
eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_SaveData(UINT8 ucIndex);
eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_LoadData(UINT8 ucIndex);
eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ClearAllData(void);
eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ClearCurrentData(void);  //A35G2_Simon_0093
INT16 halWarping_HResGet(void);
INT16 halWarping_VResGet(void);

int CountColors(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend, int *colorSet);
eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnOSD(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend);
eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnMemory(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend);
void DrawPNGOnOSDOriginalColor(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend, int *colorSet, int colorset_size);
void CalculatePaletteAndTransformRGB888(int nWidth, int nHeight, unsigned char *paucBlend, unsigned char *pixelIndexed,
    unsigned int *ulPalette256, int *colorSet, int colorset_size);
void DrawPNGOnOSDReducedColor(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend);
void ColorUniformQuantization884(int nWidth,
                                             int nHeight,
                                             unsigned char *paucBlend,
                                             unsigned char *pixelIndexed,
                                             UINT32 *ulPalette256);
void CalculatePaletteAndTransformRGB5551(sPixmapData *pasPixmap,
                                                        UINT16 uiPixmapNumber,
                                                        unsigned int *ulmapPalette255,
                                                        unsigned int *ulPalette256);


void halWarping_OSDPalette_32bitsDataTo256x3Data(UINT32 *ulSrcPalette, UINT8 aucDestPalette[256][WARP_OSD_PALETTE_ELEMENT]);
eHAL_WARPING_EXEC_CODE halWarping_OSDWritePalette(UINT8 Palette[][WARP_OSD_PALETTE_ELEMENT], UINT16 StartWritePLTidx, UINT16 Number, BOOL FlushToChip);
eHAL_WARPING_EXEC_CODE halWarping_OSDWriteTransparentColor(BOOL bEnable, UINT8 PLTIndex0, UINT8 PLTIndex1, UINT8 PLTIndex2, UINT8 PLTIndex3);
eHAL_WARPING_EXEC_CODE halWarping_OSDWriteBlink(BOOL bEnable, UINT8 ucTime, UINT8 ucCycle, BOOL FlushToChip);


void halWarping_DrawSpriteBuffer(int startX, int startY,
                                             int width, int height,
                                             unsigned char *paucBlend);
eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnOSD(INT16 iTotalNumber,
                                                              INT16 iSpritePid,
                                                              INT16 iSpriteTop,
                                                              INT16 iSpriteLeft,
                                                              sPixmapData *psPixmap);
eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnMemory(INT16 iTotalNumber,
                                                                   INT16 iSpritePid,
                                                                   INT16 iSpriteTop,
                                                                   INT16 iSpriteLeft,
                                                                   sPixmapData *psPixmap);
eHAL_WARPING_EXEC_CODE halWarping_AP_DrawBox(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iRed, INT16 iGreen, INT16 iBlue);
eHAL_WARPING_EXEC_CODE halWarping_TempPaletteWriteToChip(void);

eHAL_WARPING_EXEC_CODE halWarping_WarpOSDModeSet(UINT8 ucFront);
eHAL_WARPING_EXEC_CODE halWarpOSD_ClearOSD(void);
//eHAL_WARPING_EXEC_CODE halWarpOSD_ClearOSD_NoSemaphore(void);
eHAL_WARPING_EXEC_CODE halWarp_SetLut(void);


//G100_Doulas_0027 start
uint8 SaveWarpConfig(void);
eHAL_WARPING_EXEC_CODE halAdvWarpControl(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpGridPoints(UINT8 ucGridPorins, UINT8 ucInner);
eHAL_WARPING_EXEC_CODE halAdvWarpInnerGridPoints(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpShowOsdPattern(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpSelectControlPoint(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpMoveControlPoint(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpSelectOsdBlendWidth(UINT8 ucValue)	;
eHAL_WARPING_EXEC_CODE halAdvWarpAdjustOsdBlendWidth(UINT8 ucValue)	;
eHAL_WARPING_EXEC_CODE halAdvWarpSharpness(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpGridColor(UINT8 ucValue, UINT8 ucRedrawPattern);
eHAL_WARPING_EXEC_CODE halAdvWarpBackgroundColor(UINT8 ucValue, UINT8 ucRedrawPattern);
eHAL_WARPING_EXEC_CODE halAdvBlendingOverlapGridNumber(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvBlendingGamma(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpingInit(ePANEL_ID ucPanel);
eHAL_WARPING_EXEC_CODE halAdvBlendingLeft(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvBlendingRight(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvBlendingTop(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvBlendingBottom(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halBasicWarpSettingSet(void);
eHAL_WARPING_EXEC_CODE halBasicWarpSettingGet(void);
eHAL_WARPING_EXEC_CODE halBasicBlendSettingSet(void);
eHAL_WARPING_EXEC_CODE halBasicBlendSettingGet(void);
eHAL_WARPING_EXEC_CODE halBasicSaveWarpMemory(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halBasicSaveBlendMemory(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halBasicApplyWarpMemory(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halBasicApplyBlendMemory(UINT8 ucValue);

eHAL_WARPING_EXEC_CODE halADVWarpSettingGet(OSD_WARP *stADVWarp);
eHAL_WARPING_EXEC_CODE halADVWarpSettingSet(OSD_WARP stADVWarp);
eHAL_WARPING_EXEC_CODE halAdvSaveMemory(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvApplyMemory(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvClearMemory(void);
eHAL_WARPING_EXEC_CODE halAdvResetWarping(void);
eHAL_WARPING_EXEC_CODE halTest(void);
//G100_Doulas_0027 end
eHAL_WARPING_EXEC_CODE halAdvWarpVstartOffsetSet(ePANEL_ID ePanelTimingId);      //G100_Doulas_0043
void halWarping_BurstWrite_FixedAdd(UINT32 ulCPUAddrReg, UINT32 ulCPUDataReg, UINT8 *ucData, UINT32 ulDataSize, UINT32 ulMaxBurstWriteSize);
eHAL_WARPING_EXEC_CODE halWarping_DrawRect(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex);
eHAL_WARPING_EXEC_CODE halWarping_FreezeImage(UINT8 ucEnable);
UINT8 halWarping_GetFreezeImageState(void);
eHAL_WARPING_EXEC_CODE halWarping_OSD_ON(void);
eHAL_WARPING_EXEC_CODE halWarping_OSD_Swap_Memplane(void);
eHAL_WARPING_EXEC_CODE halWarpOSD_AP_ClearMemoryOSD(void);
eHAL_WARPING_EXEC_CODE halWarpOSD_AP_ClearOSD(eWARPOSD_DRAW_TYPE eType);
eHAL_WARPING_EXEC_CODE halWarping_AP_Ifill(INT16 iEnable, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue);
eHAL_WARPING_EXEC_CODE halWarping_AP_DrawRect(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex);
eHAL_WARPING_EXEC_CODE halWarping_AP_DrawLine(eWARPOSD_DRAW_TYPE eType, INT16 iLeft, INT16 iTop, INT16 iRight, INT16 iBottom, INT32 iColorIndex);
eHAL_WARPING_EXEC_CODE halWarping_OSD_PalettePrint(void);
eHAL_WARPING_EXEC_CODE halWarping_DrawCircle(eWARPOSD_DRAW_TYPE eType, INT16 iX, INT16 iY, INT16 iRadius, UINT16 uiCircleColorIndex, UINT16 uiBGColorIndex);
eHAL_WARPING_EXEC_CODE halWarping_CopyOSDRect(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                       INT16 ixSource,
                                                       INT16 iySource,
                                                       INT16 ixDestination,
                                                       INT16 iyDestination,
                                                       INT16 iWidth,
                                                       INT16 iHeight);
#if 0
eHAL_WARPING_EXEC_CODE halWarping_CopyOSDRect_NoSemaphore(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                                        INT16 ixSource,
                                                                        INT16 iySource,
                                                                        INT16 ixDestination,
                                                                        INT16 iyDestination,
                                                                        INT16 iWidth,
                                                                        INT16 iHeight);
#endif

eHAL_WARPING_EXEC_CODE halWarping_DrawCheckboard(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                           INT16 iRectSize);
eHAL_WARPING_EXEC_CODE halWarping_DrawCheckboards(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                             INT16 iSizeX,
                                                             INT16 iSizeY,
                                                             INT16 iShiftX,
                                                             INT16 iShiftY);
//G100_Owen_0089 PToolSet Start //A35G2_BRC_Casper_0046
eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetColIndex(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetRowIndex(UINT8 ucValue);
eHAL_WARPING_EXEC_CODE halAdvWarpGP_Move(eDIR eDir, UINT16 uiPixel);
eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetPositionMove(INT32 lPosX, INT32 lPosY);
eHAL_WARPING_EXEC_CODE halAdvWarpSettingReset(void);
eHAL_WARPING_EXEC_CODE halAdvBlendingEnable_Set(BOOL bEnable);
eHAL_WARPING_EXEC_CODE halAdvBlending_Width(eDIR eDir, UINT16 uiPixel);
eHAL_WARPING_EXEC_CODE halAdvBlending_Offset(eDIR eDir, UINT16 uiPixel);
eHAL_WARPING_EXEC_CODE halAdvBlendConfigReset(void);
//G100_Owen_0089 PToolSet End

//eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_CurrentSetting(void);
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_Warp(UINT8 ucIndex);
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_Blending(UINT8 ucIndex);
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_BlackLevel(UINT8 ucIndex);
//eHAL_WARPING_EXEC_CODE halWarping_AP_OSD_Init(void);
eHAL_WARPING_EXEC_CODE halWarping_OSDLayerReset(void);

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSelectControlPoint(UINT8 ucValue);	//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelMoveControlPoint(UINT8 ucValue);		//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelAddPoint(void);							//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelDeletePoint(void);						//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelSelectArea(UINT8 ucValue);				//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetEnable(UINT8 ucValue);					//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetRed(UINT8 ucValue);						//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetGreen(UINT8 ucValue);					//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetBlue(UINT8 ucValue);					//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelReset(UINT8 ucResetLevel);					//A65_OPTOMA_Doulas_0029//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelShowAddPointPattern(void);			//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halAdvBlackLevelShowRemovePointPattern(void);		//A65_OPTOMA_Doulas_0020
BOOL halAdvBlackLevelIsPointDeletable(void);									//A65_OPTOMA_Doulas_0020
BOOL halAdvBlackLevelIsPointAddable(void);										//A65_OPTOMA_Doulas_0020
eHAL_WARPING_EXEC_CODE halWarping_Panel_Update(ePANEL_ID ePanelId);
eHAL_WARPING_EXEC_CODE halWarping_ColorMask(UINT8 ucColorMask);
eHAL_WARPING_EXEC_CODE halWarping_InitTestPalette(void);
eHAL_WARPING_EXEC_CODE halWarping_PaletteUpdate(UINT16 uiIndex, UINT8 R, UINT8 G, UINT8 B);
eHAL_WARPING_EXEC_CODE halWarping_AP_SaveInfoForCurrentFile(void);  //A65_OPTOMA_Simon_0002  //A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarping_AP_LoadCurrentFileInfo(sCurrentFileInfo *psCurrentInfo);  //A65_OPTOMA_Simon_0002  //A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_HST_Set(UINT16 uiData);
eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_HST_Get(UINT16 *uiData);
eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_VST_Set(UINT16 uiData);
eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_VST_Get(UINT16 *uiData);

eHAL_WARPING_EXEC_CODE halAdvHide_OSD_Pattern_Set(BOOL bHide);				//A65_OPTOMA_Doulas_0211


#if (ENABLE_COLOR_UNIFORMITY == TRUE)   //G100_Tim_0020, add, start
//eHAL_WARPING_EXEC_CODE halACU_OSD_Color_Initial( void );                    //G100_Tim_0023, add  //A65_OPTOMA_CL_0006
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Disable(void);         //G100_Tim_0024, add
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Enable(void);    //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
#endif //ENABLE_COLOR_UNIFORMITY        //G100_Tim_0020, add, end
eHAL_WARPING_EXEC_CODE halAdvWarpShowOsdPattern_Unblocked_TwistLink(UINT8 ucValue); //A65_OPTOMA_Julie_0050
UINT8 halWarping_Color_Uniformity_Enable_Get(void);    //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
eHAL_WARPING_EXEC_CODE halWarping_Set_OSD_Transparent( UINT8 ucPaletteIndex );  //A65_OPTOMA_CL_0002  //A65_OPTOMA_CL_0004
eHAL_WARPING_EXEC_CODE halWarpOSD_AP_Color_Initial( UINT8 ucInitPalette );  //A65_OPTOMA_CL_0005
void halWarpOSD_AP_PLT_Set(UINT8 StartIdx, UINT8 Number, eWARP_OSD_SET_ACTION eAction);
void halWarpOSD_AP_Transparent_Set(void);
void halWarpOSD_AP_Blink_Set(eWARP_OSD_SET_ACTION eAction);
void halWarpOSD_OSD_PLT_Set(UINT8 *Palette, UINT16 StartIdx, UINT16 Number, eWARP_OSD_SET_ACTION eAction);
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Group_Set(eCOLOR_PALETTE_GROUP eColorPaletteGroup);  //A65_OPTOMA_CL_0006
eHAL_WARPING_EXEC_CODE halAdvBlack_Level_Boundary_Flag_Set(UINT8 ucBoundary);   //A35G2_Owen_0002
eHAL_WARPING_EXEC_CODE halWarp_AP_Set_Interpolation_Lut_H_Index(UINT8 ucHorzPolationIdx);  //A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarp_AP_Set_Interpolation_Lut_V_Index(UINT8 ucVertPolationIdx);  //A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarping_AdaptiveScaleFilterLutInit(void);     //A65_OPTOMA_CL_0018 //A35G2_Simon_0115
eHAL_WARPING_EXEC_CODE halWarping_AdaptiveScaleFilterLutEnable(UINT8 ucEnable); //A65_OPTOMA_CL_0018 //A35G2_Simon_0115
eHAL_WARPING_EXEC_CODE halWarping_CurrentWarpTypeSet(UINT8 ucType); //A35G2_Simon_0115
eHAL_WARPING_EXEC_CODE halWarpOSD_Reload_AP_Config(eWARPOSD_RELOAD_TYPE eType);
eHAL_WARPING_EXEC_CODE halWarpOSD_Config(eGEO_CTRL_INTERFACE eInterface, eWARPOSD_CONFIG eConfig, void *Data, eWARP_OSD_SET_ACTION eAction);
eHAL_WARPING_EXEC_CODE halWarpOSD_Draw_InsertLocation(eWARPOSD_DRAW_TYPE eDrawType);
ePANEL_ID halWarpOSD_PanelID_Get(void);
void halWarping_OPDEvent(UINT8 *aucString);
eHAL_WARPING_EXEC_CODE halWarping_TransferCtrl_Through(void);
eHAL_WARPING_EXEC_CODE halWarping_OSD_TransparentColor_Get(eTRANSPARENT_COLOR_SLOT eSlotIndex, UINT32 *ulColor);
eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetAllPosition(Point2f Data[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V]);
UINT8 halWarping_Is4KWarp(void);

#ifdef __cplusplus
}
#endif


#endif

