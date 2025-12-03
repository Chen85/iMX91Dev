//#include "dvC790.h" //A70LV_Larry_0001
#include "utilDbgMsg.h"
#include <math.h>
#include "utilCRCAPI.h"
//#include "GenericTypeDefs.h"
#include "Common.h"
#include "utilCommon.h"
#ifdef C341_WARPING_ENABLE
#include "dvC341.h"
#include "dvC341_Geo.h"
#endif /* C341_WARPING_ENABLE */
#ifdef C789_WARPING_ENABLE
#include "dvC789_WarpLight.h"
#include "dvC789.h"
#endif /* C789_WARPING_ENABLE */
#include "halGui.h"
#include "halWarping.h"
#include "cmd_ap.h"
#include "utilDatabaseAPI.h"
#include "utilMisc.h"
#include "halScaler.h"		//G100_Doulas_0045
//#include "appDataMgr.h"                 //G100_Tim_0012, add  //A35G2_CDS_Coda_0027
#include "utilWarpDemo.h"
#include "opdCtrlAPI.h"
#include "appDataMgr.h"

//A70LV_Larry_0051 start


//===== Struct defines =====//
typedef struct
{
    INT16 tl_x, tl_y, tr_x, tr_y, bl_x, bl_y, br_x, br_y;
} str_wppos4c;  // 4-corner position

typedef struct
{
    float x, y, z;
} str_pos3d;

typedef struct
{
    float x, y;
} str_pos2d;

typedef struct
{
    int dist, vw, axis;
} str_pjparam;  // Projection system parameter

typedef struct
{
    float ang_h, ang_v, vsh_l, vsh_a, hsh;
} str_wplimit;  // Warping limit

typedef struct
{
    float Px[DEF_WPTBLNUM][DEF_GV_PX_HGRD+1][DEF_GV_PX_VGRD+1];
    float Py[DEF_WPTBLNUM][DEF_GV_PX_HGRD+1][DEF_GV_PX_VGRD+1];
}sWARP_TABLE;

typedef struct
{
    float PM_WPKEYANG_H;
    float PM_WPKEYANG_V;
    float WPKEYANG_H_OLD;
    float WPKEYANG_V_OLD;
    float PM_WPPINWID_H;
    float PM_WPPINWID_V;
    INT16 PM_WPKEY4C_TLX;
    INT16 PM_WPKEY4C_TLY;
    INT16 PM_WPKEY4C_TRX;
    INT16 PM_WPKEY4C_TRY;
    INT16 PM_WPKEY4C_BLX;
    INT16 PM_WPKEY4C_BLY;
    INT16 PM_WPKEY4C_BRX;
    INT16 PM_WPKEY4C_BRY;
    UINT8 TABLE_COLOR;

    UINT8 POLATION_H;   //auto calculated value
    UINT8 POLATION_V;   //auto calculated value
    UINT8 m_cWarpFilterSelect_H;  //OSD select value (manual)
    UINT8 m_cWarpFilterSelect_V;  //OSD select value (manual)
    UINT8 m_cWarpFilterApValue_H;  //AP value                   //A35G2_Simon_0110
    UINT8 m_cWarpFilterApValue_V;  //AP value                   //A35G2_Simon_0110
    BOOL m_cWarpAutoFilter;    //OSD auto filter on/off
    BOOL m_cWarpAutoFilterAp;  //AP  auto filter on/off         //A35G2_Simon_0110

    str_wppos4c WPPOS4C;
    str_pos3d pos_i[4];
    str_pos3d pos_r;
    str_pos3d pos_a[4];
    str_pos2d pos_o[4];

}sWARP_GEO_PARA;

typedef struct
{
    UINT8   ucBlending_T_Enable;
    UINT8   ucBlending_B_Enable;
    UINT8   ucBlending_L_Enable;
    UINT8   ucBlending_R_Enable;
    UINT16  uiBlending_T_St;
    UINT16  uiBlending_B_St;
    UINT16  uiBlending_L_St;
    UINT16  uiBlending_R_St;
    UINT16  uiBlending_T_Width;
    UINT16  uiBlending_B_Width;
    UINT16  uiBlending_L_Width;
    UINT16  uiBlending_R_Width;
    UINT8   ucBlendingGamma; //A70LV_Larry_0171
    UINT32  ulEGBCT;
}sBLENDING_PARA;
//A70LV_Larry_0051 end


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
    UINT16                          uiWarp_HW_1CH;
    UINT16                          uiWarp_VW;
    UINT16                          uiWarp_Is120HzPanel;    //A35G2_Simon_0093
    UINT16                          uiPixelGridHW;      // PS_WP_HW_GRD, Grid Width / 32 pixel mode or 64 pixel mode = how many interpoation grid size
    UINT16                          uiPixelGridVW;      // PS_WP_VW_GRD, Grid Height / 32 pixel mode or 64 pixel mode = how many interpoation grid size
    INT16                           iDummyHstReset;                                    // PS_DUMMY_HST_REST
    INT16                           iDummyHendGridReset;                                // PS_DUMMY_HEND_GRD_REST
    float                           fWarp_PHWHF;
    float                           fWarp_PVWHF;

    float                           faCursorDefInX[17];             // PS_CUR_DEF_IX, real coordinate in the 3840x2160 canvas //A70LV_Larry_0137
    float                           faCursorDefInY[17];             // PS_CUR_DEF_IY, real coordinate in the 3840x2160 canvas //A70LV_Larry_0137

    UINT8                           ucIs4KWarp;
    UINT16                          uiWpSpace;
    UINT16                          uiWpSpaceBitShift;
    eWARPDATA_DECIMAL_PART_RES      eWarpDecimalPartRes;   //refer to C341 B145_WPDTCT bit12 DTSPMD

    sWARP_TABLE                     sWarpTable; //A70LV_Larry_0051

    UINT8                           ucFreeze;

    //Warp OSD
    sWARPOSD_CONFIG_INFO            sOSD_WarpOSD_Info;
    sWARPOSD_CONFIG_INFO            sAp_WarpOSD_Info;
    //UINT8                           aucPalette[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT];
    UINT32                          aulSpritePalette[WARP_OSD_TOTAL_PALETTE];
    UINT32                          aulMapSpritePalette[WARP_OSD_TOTAL_PALETTE];
    UINT16                          uiSpritePaletteCount;
    UINT16                          uiMapSpritePaletteCount;
    #if 0
    UINT8                           ucTransparentColorEnable;
    UINT8                           ucTransparentColor[4];
    UINT8                           ucBlinkEnable;
    UINT8                           ucBlinkTime;
    UINT8                           ucBlinkCycle;
    #endif

    //Black Level
    UINT8                           aucBL_Palette[16][3];
    UINT8                           aucBL_GammaTable[16][3][16];
    FLOAT                           aucBL_GammaCoef;    //2.2

    UINT8                           ucColorMask;
    UINT8                           ucPixIndexed[4096 * 10];

    UINT8                           ucBitBLT_MWI;
    UINT8                           ucBLEND_GAIN_DBD_MWI;  //Dot-by-bot blending
    UINT8                           ucBLEND_BIAS_MWI;  //black level

    UINT8                           ucCurrentWarpingType;  //WARPING_TYPE_ADVANCED / WARPING_TYPE_AP  //A35G2_Simon_0115
    UINT8                           ucIsDefaultWarpTable;  //H2PF_Simon_0131
}sHAL_WARPING_INFORMATION, *PsHAL_WARPING_INFORMATION;

static TickType_t ulhalwarpStartTicks;

static void halwarp_ResetTimeInState(void)
{
    ulhalwarpStartTicks = xTaskGetTickCount();
    return;
}

UINT32 halwarp_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(ulhalwarpStartTicks > ulTicks)
    {
        return (0xFFFFFFFF - ulhalwarpStartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - ulhalwarpStartTicks) / portTICK_RATE_MS;
    }
}

static sHAL_WARPING_INFORMATION m_sHalWarpingInfo;
static sWARP_GEO_PARA m_sWArpGeoPara; //A70LV_Larry_0051
static sBLENDING_PARA m_sBlendingPara; //A70LV_Larry_0051
static sBLACKLEVEL_PARA m_sBlackLevelPara[MAX_BLACKLEVEL_AREA_NUMBER];  //A35G2_Simon_0093
UINT8 ucBlacklevelAreaSel = 0;
static UINT8 ucTempPalette[768] ;   //G100_Simon_0007
static UINT8 ucTempTranparentColor[4] ;   //G100_Simon_0007  //A35G2_CDS_Simon_0057

static BOOL m_bWarpTwistLinkFlag = FALSE; //A70LV_Larry_0319
static BOOL ucAdvWarpingInit = FALSE;		//G100_Doulas_0063

INT32 m_OSD_MEM_LAYER0;      //H2PF_Simon_0107
INT32 m_OSD_MEM_LAYER1;      //H2PF_Simon_0107
INT32 m_OSD_CURRENT_LAYER;   //H2PF_Simon_0107

BOOL halWarping_SemaphoreTake(const char *pcFunc)
{
    BOOL bResult = dvC341_SemaphoreTake(TRUE, pcFunc);

    return bResult;
}

BOOL halWarping_SemaphoreGive(const char *pcFunc)
{
    BOOL bResult = dvC341_SemaphoreTake(FALSE, pcFunc);

    return bResult;
}

#if 1
eHAL_WARPING_EXEC_CODE halWarping_SetWarp_AutoFilter(UINT8 ucAutoFilter,UINT8 *ucHorzFilter,UINT8 *ucVertFilter)
{
    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    m_sWArpGeoPara.m_cWarpAutoFilter = ucAutoFilter; //A70LV_Larry_0060

#if (ADAPTIVE_SCALE_WARP_FILTER_LUT == FALSE)   //A35G2_Simon_0115
    *ucHorzFilter = m_sWArpGeoPara.m_cWarpFilterSelect_H; //A70LV_Larry_0060
    *ucVertFilter = m_sWArpGeoPara.m_cWarpFilterSelect_V; //A70LV_Larry_0060
#else
    m_sWArpGeoPara.m_cWarpFilterSelect_H = *ucHorzFilter;
    m_sWArpGeoPara.m_cWarpFilterSelect_V = *ucVertFilter;
#endif

#ifdef WARP_ENABLE
    halWarp_InterpolationSet(); //A70LV_Larry_0060
#endif /* WARP_ENABLE */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarp_ManualFilter(UINT8 ucHorzFilter,UINT8 ucVertFilter)
{
    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    m_sWArpGeoPara.m_cWarpAutoFilter = FALSE; //A70LV_Larry_0060
    m_sWArpGeoPara.m_cWarpFilterSelect_H = ucHorzFilter; //A70LV_Larry_0060
    m_sWArpGeoPara.m_cWarpFilterSelect_V = ucVertFilter; //A70LV_Larry_0060
#ifdef WARP_ENABLE
    halWarp_InterpolationSet(); //A70LV_Larry_0060
#endif /* WARP_ENABLE */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarp_AutoFilterParameter(UINT8 ucAutoFilter,UINT8 *ucHorzFilter,UINT8 *ucVertFilter)	//G100_Doulas_0027
{
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    m_sWArpGeoPara.m_cWarpAutoFilter = ucAutoFilter; //A70LV_Larry_0060
    *ucHorzFilter = m_sWArpGeoPara.m_cWarpFilterSelect_H; //A70LV_Larry_0060
    *ucVertFilter = m_sWArpGeoPara.m_cWarpFilterSelect_V; //A70LV_Larry_0060

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarp_ManualFilterParameter(UINT8 ucHorzFilter,UINT8 ucVertFilter)		//G100_Doulas_0027
{
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    m_sWArpGeoPara.m_cWarpAutoFilter = FALSE; //A70LV_Larry_0060
    m_sWArpGeoPara.m_cWarpFilterSelect_H = ucHorzFilter; //A70LV_Larry_0060
    m_sWArpGeoPara.m_cWarpFilterSelect_V = ucVertFilter; //A70LV_Larry_0060

    return eHAL_WARPING_EXEC_CODE_PASS;
}

#endif /* 0 */



#if 1 //A70LV_Larry_0051
static int WPMODE;     // Warping mode
static int WPON;       // for cascade
static int WPHIST[4];  // for cascade

//static UINT16 PS_PHW;
//static UINT16 PS_PVW;
//static int PHW16;  // ((PS_PHW%16)==0) ? PS_PHW/16 : PS_PHW/16+1;
//static int PVW16;  // ((PS_PVW%16)==0) ? PS_PVW/16 : PS_PVW/16+1;
//static float PHWHF;    // PS_PHW/2.0;
//static float PVWHF;    // PS_PVW/2.0;

//===== Warping parameters =====//
str_wplimit PS_WPLIMIT = {DEF_WPLIMANG_H, DEF_WPLIMANG_V, DEF_WPLIMVSH_L, DEF_WPLIMVSH_A, DEF_WPLIMHSH};
str_pjparam PS_PJPARAM = {DEF_PJDIST, DEF_PJVW, DEF_PJAXIS};


//===== function defines =====//
void halWarp_CalcHomography(str_wppos4c *p4c, UINT8 ucTblIdx);
UINT8 halWarp_Check_curpos(str_wppos4c *p4c);
UINT8 halWarp_FourCornerShift(str_wppos4c *p4c);
UINT8 halWarp_SetWpKeyAng(float AngH, float AngV);
void halWarp_Calc_Aspect(str_pos2d *pi, float angh, float angv, str_pos3d *po, UINT8 *CP);
void halWarp_Rotate(str_pos3d *pi, float angh, float angv, UINT8 rev, str_pos3d *po);
void halWarp_Angle_to_coord(str_pos3d *pi, float angh, float angv, str_wppos4c *p4c);
void halWarp_Calc4Corner(float AngH, float AngV, const str_pjparam *pjp, str_wppos4c *p4c);

#ifdef _CASCADE_
void halWarp_CalcNoWarp(UINT8 ucTblIdx);
void halWarp_Casc_bilinear(void);
void halWarp_CalcPin(float WidH, float WidV, int TblIdx);
void halWarp_Calc_oldtable(int mode);
int halWarp_CheckWpLimit(const str_wplimit *lim);
void halWarp_CascWpTable(int Premode);
#endif

UINT8 halWarp_Calclut(UINT8 *pcLUT_H, UINT8 *pcLUT_V)
{
    int i, j;
    float scl;
    float scl_min = 1;

    // v06_casc start  //
    // hlut
    scl_min = 1;
    for (j = DEF_WP_OUT_VGRD; j <= (DEF_WP_OUT_VGRD + m_sHalWarpingInfo.uiPixelGridVW); j++)
    {
        for (i = DEF_WP_OUT_HGRD; i < (DEF_WP_OUT_HGRD + m_sHalWarpingInfo.uiPixelGridHW); i++)
        {
            if ((m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i][j] < 0) || (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i][j] > (float)m_sHalWarpingInfo.uiWarp_HW))
                continue;

            if ((m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i + 1][j] < 0) || (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i + 1][j] > (float)m_sHalWarpingInfo.uiWarp_HW))
                continue;

            if (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i][j] >= m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i + 1][j])
                continue;

            scl = m_sHalWarpingInfo.uiWpSpace / (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i + 1][j] - m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][i][j]);
            scl_min = (scl < scl_min) ? scl : scl_min;
        }
    }

    // vlut
    for (i = DEF_WP_OUT_HGRD; i <= (DEF_WP_OUT_HGRD + m_sHalWarpingInfo.uiPixelGridHW); i++)
    {
        for (j = DEF_WP_OUT_VGRD; j < (DEF_WP_OUT_VGRD + m_sHalWarpingInfo.uiPixelGridVW); j++)
        {
            if ((m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j] < 0) || (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j] > (float)m_sHalWarpingInfo.uiWarp_VW))
                continue;

            if ((m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j + 1] < 0) || (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j + 1] > (float)m_sHalWarpingInfo.uiWarp_VW))
                continue;

            if (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j] >= m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j + 1])
                continue;

            scl = m_sHalWarpingInfo.uiWpSpace  / (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j + 1] - m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][i][j]);
            scl_min = (scl < scl_min) ? scl : scl_min;
        }
    }
    // v06_casc finish //

    // +++++ set LUT registers +++++ //
    scl_min = (scl_min >= 1) ? 9
        : (scl_min > 0.96) ? 8
        : (scl_min > 0.93) ? 7
        : (scl_min < 0.3) ? 0
        : scl_min * 10 - 3;

    *pcLUT_H = scl_min; //ucHSclLut;
    *pcLUT_V = scl_min; //ucVSclLut;
}

eHAL_WARPING_EXEC_CODE halWarp_SetLut(void)
{
    halWarp_Calclut(&m_sWArpGeoPara.POLATION_H, &m_sWArpGeoPara.POLATION_V);

#ifdef WARP_ENABLE
    halWarp_InterpolationSet();
#endif /* WARP_ENABLE */

    return eHAL_WARPING_EXEC_CODE_PASS;
}


//A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarp_AP_Set_Interpolation_Lut_H_Index(UINT8 ucHorzFilterIndex)
{
    if(m_sWArpGeoPara.m_cWarpAutoFilterAp == FALSE)  //only for manual
    {
        LOG_MSG(db_HAL_WARPING, "%s, Set Interpolation Lut H index %d\n\n" , __FUNCTION__, ucHorzFilterIndex);
        m_sWArpGeoPara.m_cWarpFilterApValue_H = ucHorzFilterIndex;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarp_AP_Set_Interpolation_Lut_V_Index(UINT8 ucVertFilterIndex)
{
    if(m_sWArpGeoPara.m_cWarpAutoFilterAp == FALSE)  //only for manual
    {
        LOG_MSG(db_HAL_WARPING, "%s, Set Interpolation Lut V index %d\n\n" , __FUNCTION__, ucVertFilterIndex);
        m_sWArpGeoPara.m_cWarpFilterApValue_V= ucVertFilterIndex;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarp_WriteWpTable(void) //A70LH_Larry_0124 add write check sum    //G100_Simon_0089
{
    UINT32 uiX, uiY;
    UINT32 uiXAd = 0, uiYAd = 0;
    UINT32 uiAd;
    UINT16 uiAdShift;
    UINT16 uiXAd_4K;

    UINT16 uiAdStart_4K;
    UINT16 uiYAdShift_4K;
    float fPointX, fPointY;
    float fTemp;
    INT16 iWarpTableX;
    INT16 iWarpTableY;
    UINT8 GV_RTCT = 0;
    UINT16 DTCT = 0;

    // for dummy //
    float fXStartMin, fXEndMax;
    INT16 iWarpTableDummyX = 0;
    INT16 iWarpTableDummyY = 0;
    ///////////////
    UINT8 aucData[6] = {0}; //A70LV_Larry_0061
    UINT16 uiDTCT = 0;
    UINT8 *pcDTDT = malloc(DEF_WPTBLMAX_V * DEF_WPTBLMAX_H * 4 * 2);
    UINT32 uiDTCTCount = 0;

    {
        //===== for writing warp table =====//

        GV_RTCT = (UINT8)dvC341_Read(B0_RTCT6, 0);

        //GV_RTCT = 0;
        dvC341_Write(B0_RTCT6, 0, 0);
#if 1
        uiDTCT = dvC341_Read(B145_WPDTCTCH1, 0);

        if(m_sHalWarpingInfo.uiWpSpace == 32)
        {
            uiAdShift = 7;   // 2^7 = 128
            uiXAd_4K = 128;
            uiAdStart_4K = 17664;
            uiYAdShift_4K = 1;

            if(!(uiDTCT & DTCT_BASE_ACCESS_TABLE_B))
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_ENABLE_32PIEXL_A, 0);
            }
            else
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_ENABLE_32PIEXL_B, 0);
            }
        }
        else//16
        {
            uiAdShift = 8;  // 2^8 = 256
            uiXAd_4K = 256;
            uiAdStart_4K = 35328;
            uiYAdShift_4K = 1;

            #ifdef DTCT_16PIEXL_ALWAYS_USE_TABLE_A
            dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_ENABLE_ALWAYS_TABLE_A, 0);
            #else
            if(!(uiDTCT & DTCT_BASE_ACCESS_TABLE_B))
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_ENABLE_16PIEXL_A, 0);
            }
            else
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_ENABLE_16PIEXL_B, 0);
            }
            #endif
        }

#endif /* 0 */

        dvC341_Write(B145_WPDTADCH1, 0x00, 0);
        dvC341_Write(B145_WPDTADCH1, 0x00, 0);
        dvC341_Write(B145_WPDTADCH1, 0x00, 0);

        // for dummy area //
        fXStartMin = 1000000; ;// v03_1 //(float)(m_sDrvWarpingInfo.uiPixelGridHW << DEF_WP_SPACE_BIT);// PM_IACT_HW
        fXEndMax = 0;

        // To get fXStartMin and fXEndMax correct value
        for(uiY = DEF_WP_OUT_VGRD; uiY <= (DEF_WP_OUT_VGRD + m_sHalWarpingInfo.uiPixelGridVW); uiY++)
        {
            fPointX = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][uiY] - (float)m_sHalWarpingInfo.iDummyHstReset;

            if(fPointX < 0)
            {
                fXStartMin = 0;
            }
            else if(fPointX < fXStartMin)
            {
                fXStartMin = fPointX;
            }

            fPointX = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD + m_sHalWarpingInfo.uiPixelGridHW][uiY];

            if(fPointX > (float)(m_sHalWarpingInfo.uiPixelGridHW << m_sHalWarpingInfo.uiWpSpaceBitShift)/*PM_IACT_HW*/)
            {
                fXEndMax = (float)(m_sHalWarpingInfo.uiPixelGridHW << m_sHalWarpingInfo.uiWpSpaceBitShift)/*PM_IACT_HW*/;
            }
            else if(fPointX > fXEndMax)
            {
                fXEndMax = fPointX;
            }
        }

        ////////////////////

        uiDTCTCount = 0;

        uiYAd = 0;
        uiAd = (UINT32)(uiYAd << uiAdShift);

        aucData[0] = uiAd & 0xff;
        aucData[1] = (uiAd >> 8) & 0xff;
        aucData[2] = (uiAd >> 16) & 0xff;
        dvC341_BurstWrite_FixedAdd(B145_WPDTADCH1, 3, aucData);


        for(uiY = DEF_WP_OUT_VGRD; uiY <= (DEF_WP_OUT_VGRD + m_sHalWarpingInfo.uiPixelGridVW); uiY++)
        {
            //uiYAd = uiY - DEF_WP_OUT_VGRD;
            //uiAd = (UINT32)(uiYAd << uiAdShift);

            //aucData[0] = uiAd & 0xff;
            //aucData[1] = (uiAd >> 8) & 0xff;
            //aucData[2] = (uiAd >> 16) & 0xff;
            //dvC341_BurstWrite_FixedAdd(B145_WPDTADCH1, 3, aucData);

            //fprintf(pDumpFile, "uiY = %d, Add = %d\n", uiY, uiAd);

            //////////////////////

            for(uiX = DEF_WP_OUT_HGRD; uiX <= (DEF_WP_OUT_HGRD + m_sHalWarpingInfo.uiPixelGridHW); uiX++)
            {
#if 0
                if(uiXAd == uiXAd_4K)
                {
                    uiAd = (UINT32)(uiAdStart_4K + (uiYAd << uiYAdShift_4K));

                    aucData[0] = uiAd & 0xff;
                    aucData[1] = (uiAd >> 8) & 0xff;
                    aucData[2] = (uiAd >> 16) & 0xff;
                    dvC341_BurstWrite_FixedAdd(B145_WPDTADCH1, 3, aucData);
                }
#endif /* 0 */

                fPointX = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][uiX][uiY] - (float)m_sHalWarpingInfo.iDummyHstReset;
                //iWarpTableX = (fPointX > 6143) ? 0xBFFF : (fPointX < -2048) ? 0xC000 : (INT16)(fPointX * 8 + (float)0.5);

                fPointY = m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][uiX][uiY];
                //iWarpTableY = (fPointY > 6143) ? 0xBFFF : (fPointY < -2048) ? 0xC000 : (INT16)(fPointY * 8 + (float)0.5);

                if(halWarping_WarpDataDecimalPartRes_Get() == eWDP_3BIT)
                {
                    if(fPointX > 6143)
                    {
                        iWarpTableX = 0xbfff;
                    }
                    else if(fPointX < -2048)
                    {
                        iWarpTableX = 0xc000;
                    }
                    else
                    {
                        iWarpTableX = (int)(fPointX*8+0.5f);
                    }

                    if(fPointY > 6143)
                    {
                        iWarpTableY = 0xbfff;
                    }
                    else if(fPointY < -2048)
                    {
                        iWarpTableY = 0xc000;
                    }
                    else
                    {
                        iWarpTableY = (int)(fPointY*8+0.5f);
                    }
                }
                else
                {
                    // clip
                    if(fPointX > 2060)
                    {
                        iWarpTableX = 0x80c0;
                    }
                    else if(fPointX < -1024)
                    {
                        iWarpTableX = 0xc000;
                    }
                    else
                    {
                        iWarpTableX = (int)(fPointX * 16 + 0.5f);
                    }

                    if(fPointY > 3071)
                    {
                        iWarpTableY = 0xbfff;
                    }
                    else if(fPointY < -1024)
                    {
                        iWarpTableY = 0xc000;
                    }
                    else
                    {
                        iWarpTableY = (int)(fPointY * 16 + 0.5f);
                    }
                }

                pcDTDT[uiDTCTCount++] = iWarpTableX & 0xFF;
                pcDTDT[uiDTCTCount++] = (iWarpTableX >> 8) & 0xFF;
                pcDTDT[uiDTCTCount++] = iWarpTableY & 0xFF, 0;
                pcDTDT[uiDTCTCount++] = (iWarpTableY >> 8) & 0xFF;

                //fprintf(pDumpFile, "A(%d, %d) ", iWarpTableX, iWarpTableY);

                uiXAd++;// v02
            } // end for x

#if 1
            // Dummy area (HEND) //
            fPointX = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD + m_sHalWarpingInfo.uiPixelGridHW][uiY] - (float)m_sHalWarpingInfo.iDummyHstReset;
            fPointY = m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD + m_sHalWarpingInfo.uiPixelGridHW][uiY];

            //iWarpTableDummyY = (fPointY > 6143) ? 0xBFFF : (fPointY < -2048) ? 0xC000 : (INT16)(fPointY * 8 + (float)0.5);

            if(halWarping_WarpDataDecimalPartRes_Get() == eWDP_3BIT)
            {
                if(fPointY > 6143)
                {
                    iWarpTableDummyY = 0xbfff;
                }
                else if(fPointY < -2048)
                {
                    iWarpTableDummyY = 0xc000;
                }
                else
                {
                    iWarpTableDummyY = (int)(fPointY*8+0.5f);
                }
            }
            else
            {
                // clip
                if(fPointY > 3071)
                {
                    iWarpTableDummyY = 0xbfff;
                }
                else if(fPointY < -1024)
                {
                    iWarpTableDummyY = 0xc000;
                }
                else
                {
                    iWarpTableDummyY = (int)(fPointY * 16 + 0.5f);
                }
            }

            for(uiX = 0; uiX < DEF_HW_DUMMY_GRD; uiX++)
            {
#if 0
                if(uiXAd == uiXAd_4K)
                {
                    uiAd = (UINT32)(uiAdStart_4K + (uiYAd << uiYAdShift_4K));

                    aucData[0] = uiAd & 0xff;
                    aucData[1] = (uiAd >> 8) & 0xff;
                    aucData[2] = (uiAd >> 16) & 0xff;
                    dvC341_BurstWrite_FixedAdd(B145_WPDTADCH1, 3, aucData);
                    printf("(%s, %d)\n", __FUNCTION__, __LINE__);
                }
#endif /* 0 */

    			fTemp = ((fPointY< 0) || (fPointY > (float)(m_sHalWarpingInfo.uiPixelGridVW<<m_sHalWarpingInfo.uiWpSpaceBitShift)) || (fPointX > (float)(m_sHalWarpingInfo.uiPixelGridHW<<m_sHalWarpingInfo.uiWpSpaceBitShift))) ? fPointX : fPointX + (fXEndMax - fPointX) * (uiX + 1) / DEF_HW_DUMMY_GRD;
                //iWarpTableDummyX = (fPointX > 6143) ? 0xBFFF : (fPointX < -2048) ? 0xC000 : (INT16)(fPointX * 8 + (float)0.5);

                if(halWarping_WarpDataDecimalPartRes_Get() == eWDP_3BIT)
                {
                    if(fTemp > 6143)
                    {
                        iWarpTableDummyX = 0xbfff;
                    }
                    else if(fPointX < -2048)
                    {
                        iWarpTableDummyX = 0xc000;
                    }
                    else
                    {
                        iWarpTableDummyX = (int)(fPointX*8+0.5f);
                    }
                }
                else
                {
                    // clip
                    if(fTemp > 2060)
                    {
                        iWarpTableDummyX = 0x80c0;
                    }
                    else if(fPointX < -1024)
                    {
                        iWarpTableDummyX = 0xc000;
                    }
                    else
                    {
                        iWarpTableDummyX = (int)(fPointX * 16 + 0.5f);
                    }
                }
                // write register

                pcDTDT[uiDTCTCount++] = iWarpTableDummyX & 0xFF;
                pcDTDT[uiDTCTCount++] = (iWarpTableDummyX >> 8) & 0xFF;
                pcDTDT[uiDTCTCount++] = iWarpTableDummyY & 0xFF;
                pcDTDT[uiDTCTCount++] = (iWarpTableDummyY >> 8) & 0xFF;
                //fprintf(pDumpFile, "D(%d, %d) ", iWarpTableDummyX, iWarpTableDummyY);

                //dvC341_BurstWrite_FixedAdd(B145_WPDTDTCH1, 4, aucData);

                uiXAd++;
            }


            while((uiXAd % uiXAd_4K))
            {
                //Dummy area
                pcDTDT[uiDTCTCount++] = iWarpTableDummyX & 0xFF;
                pcDTDT[uiDTCTCount++] = (iWarpTableDummyX >> 8) & 0xFF;
                pcDTDT[uiDTCTCount++] = iWarpTableDummyY & 0xFF;
                pcDTDT[uiDTCTCount++] = (iWarpTableDummyY >> 8) & 0xFF;
                uiXAd++;
            }

            if(m_sHalWarpingInfo.iDummyHendGridReset > 0)
            {
#if 0
                if(uiXAd == uiXAd_4K)
                {
                    uiAd = (UINT32)(uiAdStart_4K + (uiYAd << uiYAdShift_4K));

                    aucData[0] = uiAd & 0xff;
                    aucData[1] = (uiAd >> 8) & 0xff;
                    aucData[2] = (uiAd >> 16) & 0xff;
                    dvC341_BurstWrite_FixedAdd(B145_WPDTADCH1, 3, aucData);
                    printf("(%s, %d)\n", __FUNCTION__, __LINE__);

                    //fprintf(pDumpFile, "Add ", uiAd);
                }
#endif /* 0 */

                // write register
                pcDTDT[uiDTCTCount++] = (iWarpTableDummyX & 0xff);
                pcDTDT[uiDTCTCount++] = ((iWarpTableDummyX >> 8) & 0xff);
                pcDTDT[uiDTCTCount++] = (iWarpTableDummyY & 0xff);
                pcDTDT[uiDTCTCount++] = ((iWarpTableDummyY >> 8) & 0xff);
                //dvC341_BurstWrite_FixedAdd(B145_WPDTDTCH1, 4, aucData);

                //fprintf(pDumpFile, "E(%d, %d) ", iWarpTableDummyX, iWarpTableDummyY);
            }
#endif /* 0 */

            //fprintf(pDumpFile, "\n");

            ///////////////////////
        } // end for y

        //UINT32 StartTime = xTaskGetTickCount();
        UINT16 CRC16 = utilCRC16Calc(pcDTDT, uiDTCTCount);  //H2PF_Simon_0131
        //printf("CRC16 = %d (%d)\r\n", CRC16, m_sHalWarpingInfo.ucIsDefaultWarpTable);

        if(CRC16 == DEFAULT_WARP_TABLE_CRC_3840x2400)  //H2PF_Simon_0131
        {
            halWarping_FrameMemoryProcessCheck(TRUE);
        }
        else
        {
            halWarping_FrameMemoryProcessCheck(FALSE);
        }

        dvC341_BurstWrite_FixedAdd(B145_WPDTDTCH1, uiDTCTCount, pcDTDT);

        free(pcDTDT);

        //fclose(pDumpFile);

        dvC341_Write(B0_RTCT6, 0xFF, 0);
#if 1
        if(m_sHalWarpingInfo.uiWpSpace == 32)
        {
            if(!(uiDTCT & DTCT_BASE_ACCESS_TABLE_B))
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_DISABLE_32PIEXL_A, 0);
                dvC341_Write(B150_WPDTCTCH2, DTCT_BASE_ACCESS_DISABLE_32PIEXL_A, 0);
            }
            else
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_DISABLE_32PIEXL_B, 0);
                dvC341_Write(B150_WPDTCTCH2, DTCT_BASE_ACCESS_DISABLE_32PIEXL_B, 0);
            }
        }
        else
        {
            #ifdef DTCT_16PIEXL_ALWAYS_USE_TABLE_A
            dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B, 0);
            dvC341_Write(B150_WPDTCTCH2, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B, 0);
            #else
            if(!(uiDTCT & DTCT_BASE_ACCESS_TABLE_B))
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_DISABLE_16PIEXL_A, 0);
                dvC341_Write(B150_WPDTCTCH2, DTCT_BASE_ACCESS_DISABLE_16PIEXL_A, 0);
            }
            else
            {
                dvC341_Write(B145_WPDTCTCH1, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B, 0);
                dvC341_Write(B150_WPDTCTCH2, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B, 0);
            }
            #endif
        }
#endif /* 0 */
        dvC341_Write(B0_RTCT6, GV_RTCT, 0);

        //halWarping_SemaphoreGive(__FUNCTION__);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}


void halWarp_CalcHomography(str_wppos4c *p4c, UINT8 ucTblIdx)
{
    int     x, y;
    float   FHW, FVW;
    float   hg_sx, hg_sy, hg_dx1, hg_dx2, hg_dy1, hg_dy2, hg_z, hg_g, hg_h;
    float   hg_sys0, hg_sys1, hg_sys2, hg_sys3, hg_sys4, hg_sys5, hg_sys6, hg_sys7;
    float   hg_inv0, hg_inv1, hg_inv2, hg_inv3, hg_inv4, hg_inv5;
    float   WX, WY, tmp;

    FHW = (float)m_sHalWarpingInfo.uiWarp_HW;
    FVW = (float)m_sHalWarpingInfo.uiWarp_VW;

    //  X1 = p4c->tl_x;
    //  X2 = p4c->tr_x;
    //  X3 = p4c->br_x; //!
    //  X4 = p4c->bl_x; //!
    //  Y1 = p4c->tl_y;
    //  Y2 = p4c->tr_y;
    //  Y3 = p4c->br_y; //!
    //  Y4 = p4c->bl_y; //!

    //===== calculate homography coefficient =====//
    hg_sx   = (float)((p4c->tl_x - p4c->tr_x) + (p4c->br_x - p4c->bl_x));
    hg_sy   = (float)((p4c->tl_y - p4c->tr_y) + (p4c->br_y - p4c->bl_y));
    hg_dx1  = (float)(p4c->tr_x - p4c->br_x);
    hg_dx2  = (float)(p4c->bl_x - p4c->br_x);
    hg_dy1  = (float)(p4c->tr_y - p4c->br_y);
    hg_dy2  = (float)(p4c->bl_y - p4c->br_y);
    hg_z    = (hg_dx1 * hg_dy2) - (hg_dy1 * hg_dx2);
    hg_g    = ((hg_sx * hg_dy2) - (hg_sy * hg_dx2)) / hg_z;
    hg_h    = ((hg_sy * hg_dx1) - (hg_sx * hg_dy1)) / hg_z;

    hg_sys0 = p4c->tr_x - p4c->tl_x + hg_g * p4c->tr_x;
    hg_sys1 = p4c->bl_x - p4c->tl_x + hg_h * p4c->bl_x;
    hg_sys2 = p4c->tl_x;
    hg_sys3 = p4c->tr_y - p4c->tl_y + hg_g * p4c->tr_y;
    hg_sys4 = p4c->bl_y - p4c->tl_y + hg_h * p4c->bl_y;
    hg_sys5 = p4c->tl_y;
    hg_sys6 = hg_g;
    hg_sys7 = hg_h;

    //===== generate warping table =====//
    for(y = (DEF_GV_PX_VGRD); y >= 0; y--)
    {
        for(x = (DEF_GV_PX_HGRD); x >= 0; x--)
        {
            hg_inv0 = hg_sys7 * ((x - DEF_WP_OUT_HGRD) * m_sHalWarpingInfo.uiWpSpace) - hg_sys1;
            hg_inv1 = hg_sys0 - hg_sys6 * ((x - DEF_WP_OUT_HGRD) * m_sHalWarpingInfo.uiWpSpace);
            hg_inv1 = (hg_inv1 < 0.000001f) ? 0.000001f : hg_inv1;
            hg_inv2 = ((x - DEF_WP_OUT_HGRD) * m_sHalWarpingInfo.uiWpSpace) - hg_sys2;
            hg_inv3 = hg_sys6 * ((y - DEF_WP_OUT_VGRD) * m_sHalWarpingInfo.uiWpSpace) - hg_sys3;
            hg_inv4 = hg_sys4 - hg_sys7 * ((y - DEF_WP_OUT_VGRD) * m_sHalWarpingInfo.uiWpSpace);
            hg_inv4 = (hg_inv4 < 0.000001f) ? 0.000001f : hg_inv4;
            hg_inv5 = ((y - DEF_WP_OUT_VGRD) * m_sHalWarpingInfo.uiWpSpace) - hg_sys5;

            tmp = 1 - (hg_inv3 * hg_inv0) / (hg_inv4 * hg_inv1);
            tmp = (tmp < 0.000001f) ? 0.000001f : tmp;

            WY  = FVW * (hg_inv3 * hg_inv2 + hg_inv1 * hg_inv5) / (hg_inv4 *hg_inv1) / tmp;
            WX  = (FHW / hg_inv1) * ((hg_inv0 * WY / FVW) + hg_inv2);

            m_sHalWarpingInfo.sWarpTable.Px[ucTblIdx][x][y] = (WX > 32767) ? 32767 : (WX < -32768) ? -32768 : WX;
            m_sHalWarpingInfo.sWarpTable.Py[ucTblIdx][x][y] = (WY > 32767) ? 32767 : (WY < -32768) ? -32768 : WY;
        }
    }

    return;
}


UINT8 halWarp_Check_curpos(str_wppos4c *p4c)
{
    return ((p4c->tl_x >= p4c->tr_x) || (p4c->tl_x <  0)        ||
            (p4c->tl_y >= p4c->bl_y) || (p4c->tl_y <  0)        ||
            (p4c->tr_x >  m_sHalWarpingInfo.uiWarp_HW)   || (p4c->tr_x <= p4c->tl_x) ||
            (p4c->tr_y >= p4c->br_y) || (p4c->tr_y <  0)        ||
            (p4c->bl_x >= p4c->br_x) || (p4c->bl_x <  0)        ||
            (p4c->bl_y >  m_sHalWarpingInfo.uiWarp_VW)   || (p4c->bl_y <= p4c->tl_y) ||
            (p4c->br_x >  m_sHalWarpingInfo.uiWarp_HW)   || (p4c->br_x <= p4c->bl_x) ||
            (p4c->br_y >  m_sHalWarpingInfo.uiWarp_VW)   || (p4c->br_y <= p4c->tr_y));
}

// ==============================================================================
// FUNCTION NAME: FourCornerShift
// DESCRIPTION:
//
//
// Params:
// str_wppos4c *p4c:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/08/16, Larry Create
// --------------------
// ==============================================================================
UINT8 halWarp_FourCornerShift(str_wppos4c *p4c)
{
    p4c->tl_x = p4c->tl_x + (m_sWArpGeoPara.PM_WPKEY4C_TLX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->tl_y = p4c->tl_y + (m_sWArpGeoPara.PM_WPKEY4C_TLY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
    p4c->tr_x = p4c->tr_x - (m_sWArpGeoPara.PM_WPKEY4C_TRX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->tr_y = p4c->tr_y + (m_sWArpGeoPara.PM_WPKEY4C_TRY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
    p4c->bl_x = p4c->bl_x + (m_sWArpGeoPara.PM_WPKEY4C_BLX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->bl_y = p4c->bl_y - (m_sWArpGeoPara.PM_WPKEY4C_BLY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
    p4c->br_x = p4c->br_x - (m_sWArpGeoPara.PM_WPKEY4C_BRX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->br_y = p4c->br_y - (m_sWArpGeoPara.PM_WPKEY4C_BRY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);

    return halWarp_Check_curpos(p4c);

}

UINT8 halWarp_SetWpKeyAng(float AngH, float AngV)
{
    UINT8 ucWperr = 0;
    //BYTE cCount = 0; //A70LH_Larry_0124 add

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    halWarp_Calc4Corner((float)AngH, (float)AngV, &PS_PJPARAM, &m_sWArpGeoPara.WPPOS4C);

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    ucWperr = halWarp_FourCornerShift(&m_sWArpGeoPara.WPPOS4C); //A70LH_Larry_0141

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    if(ucWperr)
    {
        return 0;
    }

    halWarp_CalcHomography(&m_sWArpGeoPara.WPPOS4C, DEF_WPTBLIDX_NEW);

    //LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
#ifdef _CASCADE_
    halWarp_CascWpTable(WPHIST[1]); // pre1
    //LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    halWarp_CascWpTable(WPHIST[2]); // pre2
    //LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
#endif

    return (ucWperr);
}

void halWarp_Calc_Aspect(str_pos2d *pi, float angh, float angv, str_pos3d *po, UINT8 *CP)
{
    float   P12a, P12b;
    float   P13a, P13b;
    float   P24a, P24b;
    float   P34a, P34b;

    float   P1xout, P1yout;
    float   P2xout, P2yout;
    float   P3xout, P3yout;
    float   P4xout, P4yout;

    int     i;
    int     hflg, vflg;
    int     Bp;
    UINT8   Bpp;
    float   tba;
    int     Bpv;
    float   Bpvx, Bpvy;
    float   xa, xb, ya, yb;
    int     Bpr;
    float   Bpry;
    float   va, vb, ha, hb, da, db;
    float   Bprv;
    float   oldnH;
    float   yc = 0, xc, yv, vV, vH, xh, hH;  //A35G2_Simon_0070 for cppcheck
    float   minH, minV;
    float   dvx, dvH, dhx, dhH;
    float   np1x, np4x, np1y, np4y;
    float   ny1, ny2;
    float   ny, nx, nH;
    float   next = 0, next2 = 0;
    int     flga, flgb, flgc;
    float   Bpvrx, Bpvry;
    float   Bx;

    //line P1,P2(AB)
    P12a = ((pi + 1)->x == pi->x) ? DEF_MAX_LONG : ((pi + 1)->y - pi->y) / ((pi + 1)->x - pi->x);
    P12b = pi->y - pi->x * P12a;
    //line P1,P3(AC)
    P13a = ((pi + 2)->x == pi->x) ? DEF_MAX_LONG : ((pi + 2)->y - pi->y) / ((pi + 2)->x - pi->x);
    P13b = pi->y - pi->x * P13a;
    //line P2,P4(BD)
    P24a = ((pi + 3)->x == (pi + 1)->x) ? DEF_MAX_LONG : ((pi + 3)->y - (pi +1)->y) / ((pi + 3)->x - (pi + 1)->x);
    P24b = (pi + 1)->y - (pi + 1)->x * P24a;
    //line P3,P4(CD)
    P34a = ((pi + 3)->x == (pi + 2)->x) ? DEF_MAX_LONG : ((pi + 3)->y - (pi +2)->y) / ((pi + 3)->x - (pi + 2)->x);
    P34b = (pi + 2)->y - (pi + 2)->x * P34a;

    //calc
    hflg = (angh >= 0) ? 1 : 0;
    vflg = (angv >  0) ? 1 : 0;
    Bp   = ((hflg == 0) && (vflg == 0)) ? 2
           : ((hflg == 0) && (vflg == 1)) ? 4
           : ((hflg == 1) && (vflg == 0)) ? 1
           : 3;

    tba  = ((Bp == 1) | (Bp == 2)) ? P34a : P12a;
    Bpp  = ((Bp == 1) & (tba > 0)) ? 3
           : ((Bp == 2) & (tba < 0)) ? 4
           : ((Bp == 3) & (tba < 0)) ? 1
           : ((Bp == 4) & (tba > 0)) ? 2
           : Bp;

    va   = ((Bpp == 1) | (Bpp == 3)) ? P13a : P24a;
    Bpv  = ((Bpp == 1) | (Bpp == 3)) ? ((va >= 0) ? 1 : 3) : ((va >= 0) ? 4 :2);
    Bpvx = (Bpv == 1) ? pi->x
           : (Bpv == 2) ? (pi + 1)->x
           : (Bpv == 3) ? (pi + 2)->x
           : (pi + 3)->x;
    Bpvy = (Bpv == 1) ? pi->y
           : (Bpv == 2) ? (pi + 1)->y
           : (Bpv == 3) ? (pi + 2)->y
           : (pi + 3)->y;

    xa   = ((Bpv == 1) | (Bpv == 2)) ? P12a : P34a;
    xb   = ((Bpv == 1) | (Bpv == 2)) ? P12b : P34b;
    ya   = ((Bpv == 1) | (Bpv == 3)) ? P13a : P24a;
    yb   = ((Bpv == 1) | (Bpv == 3)) ? P13b : P24b;

    Bpr  = (Bpv == 1) ? 3
           : (Bpv == 2) ? 4
           : (Bpv == 3) ? 1
           : 2;

    Bpry = (Bpr == 1) ? pi->y
           : (Bpr == 2) ? (pi + 1)->y
           : (Bpr == 3) ? (pi + 2)->y
           : (pi + 3)->y;

    va   = ((Bpv == 1) | (Bpv == 2)) ? P34a : P12a;
    vb   = ((Bpv == 1) | (Bpv == 2)) ? P34b : P12b;
    ha   = ((Bpv == 1) | (Bpv == 3)) ? P24a : P13a;
    hb   = ((Bpv == 1) | (Bpv == 3)) ? P24b : P13b;
    da   = ((Bpv == 1) | (Bpv == 4)) ? -(float)m_sHalWarpingInfo.uiWarp_VW / (float)m_sHalWarpingInfo.uiWarp_HW : (float)m_sHalWarpingInfo.uiWarp_VW / (float)m_sHalWarpingInfo.uiWarp_HW;
    //  db   = Bpvy - Bpvx * da;
    Bprv = fabsf(Bpry - Bpvy);

    oldnH = 0;
    nx = 0;
    ny = 0;

    for(i = 0; i < 30; i++)
    {
        yc    = (i == 0) ? Bpvy : yc + next2;
        xc    = (i == 0) ? Bpvx : (ya == 0) ? powf(10.0, 20) : (yc - yb) / ya;
        yv    = va * xc + vb;
        vV    = fabsf(yc - yv);
        vH    = (float)m_sHalWarpingInfo.uiWarp_HW / (float)m_sHalWarpingInfo.uiWarp_VW * vV;
        xh    = (ha == 0) ? powf(10.0, 20) : (yc - hb) / ha;
        hH    = fabsf(xc - xh);
        minH  = (vH > hH) ? hH : vH;
        db    = yc - da * xc;
        dvx   = (da == va) ? powf(10.0, 20) : (vb - db) / (da - va);
        dvH   = fabsf(xc - dvx);
        minH  = (dvH > minH) ? minH : dvH;
        dhx   = (da == ha) ? powf(10.0, 20) : (hb - db) / (da - ha);
        dhH   = fabsf(xc - dhx);
        minH  = (dhH > minH) ? minH : dhH;
        minV  = (float)m_sHalWarpingInfo.uiWarp_VW / (float)m_sHalWarpingInfo.uiWarp_HW * minH;
        np1x  = ((Bpv == 1) | (Bpv == 3)) ? xc : xc - minH;
        np4x  = ((Bpv == 2) | (Bpv == 4)) ? xc : xc + minH;
        np1y  = ((Bpv == 1) | (Bpv == 2)) ? yc : yc + minV;
        np4y  = ((Bpv == 3) | (Bpv == 4)) ? yc : yc - minV;
        ny1   = ((Bpv == 1) | (Bpv == 2)) ? np1y : np4y;
        ny2   = ((Bpv == 1) | (Bpv == 3)) ? xa * np4x + xb : xa * np1x + xb;
        nx    = ((Bpv == 1) | (Bpv == 2)) ? ((ny2 >= ny1) ? xc : nx) : ((ny2<= ny1) ? xc : nx);
        ny    = ((Bpv == 1) | (Bpv == 2)) ? ((ny2 >= ny1) ? yc : ny) : ((ny2<= ny1) ? yc : ny);
        nH    = ((Bpv == 1) | (Bpv == 2)) ? ((ny2 >= ny1) ? minH : oldnH) : ((ny2 <= ny1) ? minH : oldnH);
        next  = (i == 0) ? (nH > 0) ? 0 : Bprv / 2
        : ((Bpv == 1) | (Bpv == 2)) ? (ny2 >= ny1) ? (nH == oldnH) ? 0 : next/ 2 : next / 2
        : (ny2 <= ny1) ? (nH == oldnH) ? 0 : next / 2 : next / 2;
        next2 = ((Bpv == 1) | (Bpv == 2)) ? ((nH > oldnH) ? next : -next) : ((nH > oldnH) ? -next : next);
        oldnH = nH;
    }

    Bpvry = (Bpv == 1) ? (pi + 1)->y
            : (Bpv == 2) ? pi->y
            : (Bpv == 3) ? (pi + 3)->y
            : (pi + 2)->y;
    flga  = (((Bpv == 1) | (Bpv == 2)) & (Bpvy > Bpvry)) ? 1 : 0;
    flgb  = (((Bpv == 3) | (Bpv == 4)) & (Bpvy < Bpvry)) ? 1 : 0;
    flgc  = (vH > hH) ? 1 : 0;

    if(flgc & (flga | flgb))
    {
        Bpvrx = (Bpv == 1) ? (pi + 1)->x
                : (Bpv == 2) ? pi->x
                : (Bpv == 3) ? (pi + 3)->x
                : (pi + 2)->x;
        Bx    = (ya == 0) ? DEF_MAX_LONG : (Bpvry - yb) / ya;
        nx    = Bx;
        ny    = Bpvry;
        minH  = fabsf(Bpvrx - Bx);
        minV  = minH * (float)m_sHalWarpingInfo.uiWarp_VW / (float)m_sHalWarpingInfo.uiWarp_HW;
    }

    //output coordinate
    P1xout = ((Bpv == 1) || (Bpv == 3)) ? nx : nx - minH;
    P1yout = ((Bpv == 1) || (Bpv == 2)) ? ny : ny + minV;

    P2xout = ((Bpv == 2) || (Bpv == 4)) ? nx : nx + minH;
    P2yout = P1yout;

    P3xout = P1xout;
    P3yout = ((Bpv == 3) || (Bpv == 4)) ? ny : ny - minV;

    P4xout = P2xout;
    P4yout = P3yout;

    //output
    po->x     = P1xout;
    po->y     = P1yout;
    (po + 1)->x = P2xout;
    (po + 1)->y = P2yout;
    (po + 2)->x = P3xout;
    (po + 2)->y = P3yout;
    (po + 3)->x = P4xout;
    (po + 3)->y = P4yout;

    *CP = Bpp;

    return;
}


void halWarp_Rotate(str_pos3d *pi, float angh, float angv, UINT8 rev, str_pos3d *po)
{
    float   x1, y1, z1;
    float   rad_h, rad_v;

    rad_h  = DEF_PI * angh / 180.0f;
    rad_v  = DEF_PI * angv / 180.0f;

    if(rev == 0)
    {
        // X ax rotation
        x1 =  pi->x;
        y1 =  pi->y * cosf(-rad_v) - pi->z * sinf(-rad_v);
        z1 =  pi->y * sinf(-rad_v) + pi->z * cosf(-rad_v);
        // Y ax rotation
        po->x =  x1 * cosf(rad_h) + z1 * sinf(rad_h);
        po->y =  y1;
        po->z = -x1 * sinf(rad_h) + z1 * cosf(rad_h);
    }
    else
    {
        // Y ax rotation
        x1 =  pi->x * cosf(-rad_h) + pi->z * sinf(-rad_h);
        y1 =  pi->y;
        z1 = -pi->x * sinf(-rad_h) + pi->z * cosf(-rad_h);
        // X ax rotation
        po->x =  x1;
        po->y =  y1 * cosf(rad_v) - z1 * sinf(rad_v);
        po->z =  y1 * sinf(rad_v) + z1 * cosf(rad_v);
    }

    return;
}

void halWarp_Angle_to_coord(str_pos3d *pi, float angh, float angv, str_wppos4c *p4c)
{
    float   fp1x, fp1y, fp2x, fp2y, fp3x, fp3y, fp4x, fp4y;
    int     tymax, bymin;
    UINT8   ucCP;

    //rotate.c
    halWarp_Rotate(pi  , angh, angv, 0, &m_sWArpGeoPara.pos_r);
    m_sWArpGeoPara.pos_o[0].x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    m_sWArpGeoPara.pos_o[0].y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;
    m_sWArpGeoPara.pos_a[0].z = pi->z;     //!

    halWarp_Rotate(pi + 1, angh, angv, 0, &m_sWArpGeoPara.pos_r);
    m_sWArpGeoPara.pos_o[1].x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    m_sWArpGeoPara.pos_o[1].y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;
    m_sWArpGeoPara.pos_a[1].z = (pi + 1)->z;   //!

    halWarp_Rotate(pi + 2, angh, angv, 0, &m_sWArpGeoPara.pos_r);
    m_sWArpGeoPara.pos_o[2].x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    m_sWArpGeoPara.pos_o[2].y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;
    m_sWArpGeoPara.pos_a[2].z = (pi + 2)->z;   //!m_sWArpGeoPara.

    halWarp_Rotate(pi + 3, angh, angv, 0, &m_sWArpGeoPara.pos_r);
    m_sWArpGeoPara.pos_o[3].x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    m_sWArpGeoPara.pos_o[3].y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;
    m_sWArpGeoPara.pos_a[3].z = (pi + 3)->z;   //!

    /*aspect conversion*/
    halWarp_Calc_Aspect(m_sWArpGeoPara.pos_o, angh, angv, m_sWArpGeoPara.pos_a, &ucCP);

    //rotate_rev.c
    halWarp_Rotate(m_sWArpGeoPara.pos_a  , angh, angv, 1, &m_sWArpGeoPara.pos_r);
    fp1x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    fp1y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;

    halWarp_Rotate(m_sWArpGeoPara.pos_a + 1, angh, angv, 1, &m_sWArpGeoPara.pos_r);
    fp2x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    fp2y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;

    halWarp_Rotate(m_sWArpGeoPara.pos_a + 2, angh, angv, 1, &m_sWArpGeoPara.pos_r);
    fp3x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    fp3y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;

    halWarp_Rotate(m_sWArpGeoPara.pos_a + 3, angh, angv, 1, &m_sWArpGeoPara.pos_r);
    fp4x = m_sWArpGeoPara.pos_r.x * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.x;
    fp4y = m_sWArpGeoPara.pos_r.y * (pi->z - m_sWArpGeoPara.pos_r.z) / m_sWArpGeoPara.pos_r.z + m_sWArpGeoPara.pos_r.y;

    //round off
    fp1x = floorf(fp1x + 0.5f);
    fp1y = floorf(fp1y + 0.5f);
    fp2x = floorf(fp2x + 0.5f);
    fp2y = floorf(fp2y + 0.5f);
    fp3x = floorf(fp3x + 0.5f);
    fp3y = floorf(fp3y + 0.5f);
    fp4x = floorf(fp4x + 0.5f);
    fp4y = floorf(fp4y + 0.5f);

    tymax = (fp2y > fp1y) ? (int)fp2y : (int)fp1y;
    bymin = (fp3y > fp4y) ? (int)fp4y : (int)fp3y;


    //Movement of center
    p4c->tl_x = (int)(m_sHalWarpingInfo.fWarp_PHWHF + fp1x);
    p4c->tr_x = (int)(m_sHalWarpingInfo.fWarp_PHWHF + fp2x);
    p4c->bl_x = (int)(m_sHalWarpingInfo.fWarp_PHWHF + fp3x);
    p4c->br_x = (int)(m_sHalWarpingInfo.fWarp_PHWHF + fp4x);
    p4c->tl_y = ((ucCP == 1) || (ucCP == 2)) ? tymax - (int)fp1y : m_sHalWarpingInfo.uiWarp_VW + (bymin - (int)fp1y);
    p4c->tr_y = ((ucCP == 1) || (ucCP == 2)) ? tymax - (int)fp2y : m_sHalWarpingInfo.uiWarp_VW + (bymin - (int)fp2y);
    p4c->bl_y = ((ucCP == 1) || (ucCP == 2)) ? tymax - (int)fp3y : m_sHalWarpingInfo.uiWarp_VW + (bymin - (int)fp3y);
    p4c->br_y = ((ucCP == 1) || (ucCP == 2)) ? tymax - (int)fp4y : m_sHalWarpingInfo.uiWarp_VW + (bymin - (int)fp4y);

    return;
}

void halWarp_Calc4Corner(float AngH, float AngV, const str_pjparam *pjp, str_wppos4c *p4c)
{
    float   dnratio, upratio;
    float   pz;

    //dnratio = (pjp->axis == 0) ? 20 : 0; //offset 120%
    //upratio = 100 + dnratio; //1.2
    dnratio = (pjp->axis == 0) ? 50 : 0; //offset 100%
    upratio = 100 - dnratio; //1.0

    pz = (float)m_sHalWarpingInfo.uiWarp_VW / (float)pjp->vw * (float)pjp->dist;

    m_sWArpGeoPara.pos_i[0].x = (float)(-m_sHalWarpingInfo.uiWarp_HW / 2);
    m_sWArpGeoPara.pos_i[0].y = (float)(m_sHalWarpingInfo.uiWarp_VW) * upratio / 100.0f;
    m_sWArpGeoPara.pos_i[0].z = pz;
    m_sWArpGeoPara.pos_i[1].x = (float)(m_sHalWarpingInfo.uiWarp_HW / 2);
    m_sWArpGeoPara.pos_i[1].y = m_sWArpGeoPara.pos_i[0].y;
    m_sWArpGeoPara.pos_i[1].z = pz;
    m_sWArpGeoPara.pos_i[2].x = m_sWArpGeoPara.pos_i[0].x;
    m_sWArpGeoPara.pos_i[2].y = (float)(-m_sHalWarpingInfo.uiWarp_VW) * dnratio / 100.0f;
    m_sWArpGeoPara.pos_i[2].z = pz;
    m_sWArpGeoPara.pos_i[3].x = m_sWArpGeoPara.pos_i[1].x;
    m_sWArpGeoPara.pos_i[3].y = m_sWArpGeoPara.pos_i[2].y;
    m_sWArpGeoPara.pos_i[3].z = pz;

    halWarp_Angle_to_coord(m_sWArpGeoPara.pos_i, AngH, AngV, p4c);

    return;
}

#ifdef _CASCADE_
void halWarp_CalcNoWarp(UINT8 ucTblIdx)
{
    INT32 uiX, uiY;

    for(uiY = (DEF_GV_PX_VGRD); uiY >= 0; uiY--)
    {
        for(uiX = (DEF_GV_PX_HGRD); uiX >= 0; uiX--)
        {
            m_sHalWarpingInfo.sWarpTable.Px[ucTblIdx][uiX][uiY] = ((uiX - DEF_WP_OUT_HGRD) * m_sHalWarpingInfo.uiWpSpace);
            m_sHalWarpingInfo.sWarpTable.Py[ucTblIdx][uiX][uiY] = ((uiY - DEF_WP_OUT_VGRD) * m_sHalWarpingInfo.uiWpSpace);
        }
    }

    return;
}

void halWarp_Casc_bilinear(void)
{
    int     x, y;
    int     x0, x1, y0, y1;
    float   a, b;
    float   fIxNew, fIyNew;

    float   ix_old00, ix_old10, ix_old01, ix_old11;
    float   iy_old00, iy_old10, iy_old01, iy_old11;
    float   diff0, diff1;

    for(y = DEF_GV_PX_VGRD; y >= 0; y--)
    {
        for(x = DEF_GV_PX_HGRD; x >= 0; x--)
        {

            fIxNew = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y] /
                        ((float)m_sHalWarpingInfo.uiWpSpace * (float)m_sHalWarpingInfo.uiWarp_HW/*PM_IACT_HW*/ /
                        (float)m_sHalWarpingInfo.uiWarp_HW/*PS_WP_HW*/) + DEF_WP_OUT_HGRD;
            fIyNew = m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y] /
                        ((float)m_sHalWarpingInfo.uiWpSpace * (float)m_sHalWarpingInfo.uiWarp_VW/*PM_IACT_VW*/ /
                        (float)m_sHalWarpingInfo.uiWarp_VW)/*PS_WP_VW*/ + DEF_WP_OUT_VGRD;


            // X-left
            x0 = (int)(fIxNew);
            x0 = (x0 < 0) ? 0 : (x0 > (DEF_GV_PX_HGRD - 1)) ? (DEF_GV_PX_HGRD - 1) : x0;
            // X-right
            x1 = x0 + 1;
            // Y-top
            y0 = (int)(fIyNew);
            y0 = (y0 < 0) ? 0 : (y0 > (DEF_GV_PX_VGRD - 1)) ? (DEF_GV_PX_VGRD - 1) : y0;
            // Y-bottom
            y1 = y0 + 1;

            // sub-pixel
            a  = fIxNew - x0;
            b  = fIyNew - y0;

            ix_old00 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_OLD][x0][y0];
            ix_old10 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_OLD][x1][y0];
            ix_old01 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_OLD][x0][y1];
            ix_old11 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_OLD][x1][y1];
            iy_old00 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_OLD][x0][y0];
            iy_old10 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_OLD][x1][y0];
            iy_old01 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_OLD][x0][y1];
            iy_old11 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_OLD][x1][y1];

            if((a < -10) || (a > 10))
            {
                diff0 = fabsf(ix_old10 - ix_old00);
                diff1 = fabsf(ix_old11 - ix_old01);

                if(b < -10)
                {
                    m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y] = ix_old00 + diff0 * a;
                }
                else if(b > 10)
                {
                    m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y] = ix_old01 + diff1 * a;
                }
                else
                {
                    m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y] = ix_old00 + ((diff1 - diff0) * b + diff0) * a;
                }
            }
            else  // bi-linear
            {
                m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y] = (ix_old10 * a + ix_old00 * (1 - a)) * (1 - b) + (ix_old11 * a + ix_old01 * (1 - a)) * b;
            }

            if((b < -10) || (b > 10))
            {
                diff0 = fabsf(iy_old01 - iy_old00);
                diff1 = fabsf(iy_old11 - iy_old10);

                if(a < -10)
                {
                    m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y] = iy_old00 + diff0 * b;
                }
                else if(a > 10)
                {
                    m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y] = iy_old10 + diff1 * b;
                }
                else
                {
                    m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y] = iy_old00 + (( diff1 - diff0) * a + diff0) * b;
                }
            }
            else  // bi-linear
            {
                m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y] = (iy_old01 * b + iy_old00 * (1 - b)) * (1 - a) + (iy_old11 * b + iy_old10 * (1 - b)) * a;
            }

        }
    }

    return;
}

void halWarp_CalcPin(float WidH, float WidV, int TblIdx)
{
    int     x, y;
    float   tWidH, tWidV, tWidH2, tWidV2;
    float   sa, dist;
    float   sclx[DEF_GV_PX_VGRD+1], scly[DEF_GV_PX_HGRD+1];
    float   FHW, FVW;

    //printf("(uiWarp_HW = %d, uiWarp_VW = %d, fWarp_PHWHF = %f, fWarp_PVWHF = %f, WidH = %f, WidV = %f)\n", m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW, m_sHalWarpingInfo.fWarp_PHWHF,  m_sHalWarpingInfo.fWarp_PVWHF, WidH, WidV);

    FHW = (float)m_sHalWarpingInfo.uiWarp_HW;
    FVW = (float)m_sHalWarpingInfo.uiWarp_VW;

    //===== Vertical =====//
    tWidV = FVW * (FVW / (FVW - 2 * fabsf(WidV)) - 1);
    tWidV = (WidV < 0) ? -tWidV : tWidV;

    if(tWidV < 0)
    {
        tWidV2 = -tWidV;
    }
    else
    {
        tWidV2 = 0;
    }

    sa = tWidV / (m_sHalWarpingInfo.fWarp_PHWHF * m_sHalWarpingInfo.fWarp_PHWHF);


    for(x = (DEF_GV_PX_HGRD); x >= 0; x--)
    {
        dist = sa * (((x - DEF_WP_OUT_HGRD) << m_sHalWarpingInfo.uiWpSpaceBitShift) - m_sHalWarpingInfo.fWarp_PHWHF) * (((x - DEF_WP_OUT_HGRD) << m_sHalWarpingInfo.uiWpSpaceBitShift) - m_sHalWarpingInfo.fWarp_PHWHF) + FVW + tWidV2;
        scly[x] = dist / FVW;
    }

    //===== Horizontal =====//
    tWidH = FHW * (FHW / (FHW - 2 * fabsf(WidH)) - 1);
    tWidH = (WidH < 0) ? -tWidH : tWidH;

    if(tWidH < 0)
    {
        tWidH2 = -tWidH;
    }
    else
    {
        tWidH2 = 0;
    }

    sa = tWidH / (m_sHalWarpingInfo.fWarp_PVWHF * m_sHalWarpingInfo.fWarp_PVWHF);

    for(y = (DEF_GV_PX_VGRD); y >= 0; y--)
    {
        dist = sa * (((y - DEF_WP_OUT_VGRD) << m_sHalWarpingInfo.uiWpSpaceBitShift) - m_sHalWarpingInfo.fWarp_PVWHF) * (((y - DEF_WP_OUT_VGRD) << m_sHalWarpingInfo.uiWpSpaceBitShift) - m_sHalWarpingInfo.fWarp_PVWHF) + FHW + tWidH2;
        sclx[y] = dist / FHW;
    }

    for(y = (DEF_GV_PX_VGRD); y >= 0; y--)
    {
        for(x = (DEF_GV_PX_HGRD); x >= 0; x--)
        {
            m_sHalWarpingInfo.sWarpTable.Px[TblIdx][x][y] = (((x - DEF_WP_OUT_HGRD) << m_sHalWarpingInfo.uiWpSpaceBitShift) - m_sHalWarpingInfo.fWarp_PHWHF) * sclx[y] + m_sHalWarpingInfo.fWarp_PHWHF;// + 3*32;
            m_sHalWarpingInfo.sWarpTable.Py[TblIdx][x][y] = (((y - DEF_WP_OUT_VGRD) << m_sHalWarpingInfo.uiWpSpaceBitShift) - m_sHalWarpingInfo.fWarp_PVWHF) * scly[x] + m_sHalWarpingInfo.fWarp_PVWHF;// + 27*32;
        }
    }
    return;
}

void halWarp_Calc_oldtable(int mode)
{
    switch(mode)
    {
        case DEF_WPMODE_TH   :
            halWarp_CalcNoWarp(DEF_WPTBLIDX_OLD);
            break;

        case DEF_WPMODE_KANG :
            //printf("(%s, %d)KANG\r\n", __FUNCTION__, __LINE__);
            halWarp_CalcHomography(&m_sWArpGeoPara.WPPOS4C, DEF_CASC_OLD_TBL);
            break;

        case DEF_WPMODE_K4C  :
            //printf("(%s, %d) K4C\r\n", __FUNCTION__, __LINE__);
            halWarp_CalcHomography(&m_sWArpGeoPara.WPPOS4C, DEF_CASC_OLD_TBL);
            break;

        case DEF_WPMODE_PIN  :
            //printf("(%s, %d) PIN\r\n", __FUNCTION__, __LINE__);
            halWarp_CalcPin(m_sWArpGeoPara.PM_WPPINWID_H, m_sWArpGeoPara.PM_WPPINWID_V, DEF_CASC_OLD_TBL);
            break;

        case DEF_WPMODE_ARC  :
            //CalcArc(PM_WPARCWID_L, PM_WPARCWID_R, PM_WPARCWID_T,PM_WPARCWID_B, DEF_WPTBLIDX_OLD);
            break;

        case DEF_WPMODE_ROT  :
            //CalcAffine(PM_WPROTANG, DEF_WPTBLIDX_OLD);
            break;

        default:
            ASSERT_ALWAYS();
            break;
    }

    return;
}

int halWarp_CheckWpLimit(const str_wplimit *lim)
{
    int     x, y;
    float   diffx_x, diffy_x, diffx_y, diffy_y, diffy_y_sum, idiffy_y;
    float   tan_h = 0;
    float   tan_v = 0;
    float   divn = 0;
	float   hw_grd_max = (float)(m_sHalWarpingInfo.uiPixelGridHW<<m_sHalWarpingInfo.uiWpSpaceBitShift);// v03_1
	float   vw_grd_max = (float)(m_sHalWarpingInfo.uiPixelGridVW<<m_sHalWarpingInfo.uiWpSpaceBitShift);// v03_1

	// v03_1 start
	if( m_sHalWarpingInfo.uiWarp_HW > 4032 )
	{
		for( x = (DEF_WP_OUT_HGRD+m_sHalWarpingInfo.uiPixelGridHW); x >= DEF_WP_OUT_HGRD; x-- )
		{
			if( (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][DEF_WP_OUT_VGRD] >= 0) && (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][DEF_WP_OUT_VGRD] <= hw_grd_max) && (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][DEF_WP_OUT_VGRD] > 0))
			{
				return eC341_WARP_ERR_OUTSIDE;
			}
		}
	}
	// v03_1 finish

	for( y = (DEF_WP_OUT_VGRD+m_sHalWarpingInfo.uiPixelGridVW); y >= DEF_WP_OUT_VGRD ; y-- )
	{
		divn = 0;
		diffy_y_sum = 0;

		// v03_1 start
		if( (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][y] >= 0) && (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][y] <= vw_grd_max) && (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][y] > (float)(DEF_HW_DUMMY * 6)) )
		{
			return eC341_WARP_ERR_OUTSIDE;
		}
		if( (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD+m_sHalWarpingInfo.uiPixelGridHW][y] >= 0) && (m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD+m_sHalWarpingInfo.uiPixelGridHW][y] <= vw_grd_max) && (m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD+m_sHalWarpingInfo.uiPixelGridHW][y] < (hw_grd_max - (float)(DEF_HW_DUMMY * 6))))
		{
			return eC341_WARP_ERR_OUTSIDE;
		}
		// v03_1 finish

		for( x = (DEF_WP_OUT_HGRD+m_sHalWarpingInfo.uiPixelGridHW); x >= DEF_WP_OUT_HGRD; x-- )
		{
            if((x < m_sHalWarpingInfo.uiPixelGridHW) || (y < m_sHalWarpingInfo.uiPixelGridVW))
            {
                // Valid area
                if(!(((m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y]<0)&&(m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x+1][y]<0)&&(m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y+1]<0)&&(m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x+1][y+1]<0))
                   ||((m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y]<0)&&(m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x+1][y]<0)&&(m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y+1]<0)&&(m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x+1][y+1]<0))
                   ||((m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y]>hw_grd_max)&&(m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x+1][y]>hw_grd_max)&&(m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y+1]>hw_grd_max)&&(m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x+1][y+1]>hw_grd_max))
                   ||((m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y]>vw_grd_max)&&(m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x+1][y]>vw_grd_max)&&(m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y+1]>vw_grd_max)&&(m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x+1][y+1]>vw_grd_max))))
                {
					if( x < m_sHalWarpingInfo.uiPixelGridHW )
					{
						diffx_x = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x+1][y] - m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y];
						diffy_x = m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x+1][y] - m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y];

						// Horizontal slope
						tan_h = diffy_x/diffx_x;
						if( (tan_h < ((-1)*DEF_WPLIMANG_H)) || (tan_h > DEF_WPLIMANG_H) )
						{
							return eC341_WARP_ERR_ANG_H;
						}
                        // Horizontal shrink rate
                        if( diffx_x > (float)((m_sHalWarpingInfo.uiWpSpace / (1.0 / DEF_WPLIMHSH)) * 8)) // v03_1
                        {
                            return eC341_WARP_ERR_HSH;
                        }
						// Horizontal interval of grid
						if( diffx_x < DEF_WPLIMITV_H )
						{
							return eC341_WARP_ERR_INTERVAL;
						}
                    } // end if(x<PS_WP_HW_GRD)
                    else
                    {
    					diffx_x = 1;
    					diffy_x = 0;// -> diffy_x / diffx_x = 0
    				}

                    if( y < m_sHalWarpingInfo.uiPixelGridVW )
                    {
                        diffx_y = m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y+1] - m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y];
                        diffy_y = m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y+1] - m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y];

                        // Vertical local shrink rate
                        idiffy_y = diffy_y + diffx_y * ( (-1) * diffy_x/diffx_x );// theta = arctan(diffy_x/diffx_x), tan(theta) = tan( arctan(diffy_x/diffx_x) ) = diffy_x/diffx_x
                        if( idiffy_y > (float)(m_sHalWarpingInfo.uiWpSpace / 0.3) ) // v03_1
                        {
                            return eC341_WARP_ERR_VSH_LOC;
                        }

                        // Vertical slope
                        tan_v = diffx_y / diffy_y;
                        if( (tan_v < ((-1) * DEF_WPLIMANG_V)) || (tan_v > DEF_WPLIMANG_V) )
                        {
                            return eC341_WARP_ERR_ANG_V;
                        }

						// Vertical interval of grid
						if( diffy_y < DEF_WPLIMITV_V )
						{
							return eC341_WARP_ERR_INTERVAL;
						}

						diffy_y_sum = diffy_y_sum + idiffy_y;
						divn = divn + 1;
				    }
				} // end if(y<PS_WP_VW_GRD)
            }
        }
    } // end for y
    // Vertical average shrink rate
	if( (diffy_y_sum) > ((float)(m_sHalWarpingInfo.uiWpSpace / 0.5) * divn) )// v03_1
	{
        return eC341_WARP_ERR_VSH_AVE;
    }
    return eC341_WARP_OK;
}

void halWarp_CascWpTable(int Premode)
{
    halWarp_Calc_oldtable(Premode);
    halWarp_Casc_bilinear();

    return;
}

#endif


// ==============================================================================
// FUNCTION NAME: appC786GeometrySet
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2015/02/26, Larry Create
// --------------------
// ==============================================================================
eRESULT halWarp_GeometrySet(void)
{
    UINT8 ucWperr = 0;

    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return rcSUCCESS;
    }

    //WPMODE = WPMODE;    //A70LV_Doulas_0088   //G100_Simon_0060
    //WPON = WPON;        //A70LV_Doulas_0088
    WPMODE = DEF_WPMODE_KANG;

    WPHIST[2] = DEF_WPMODE_TH;
    WPHIST[1] = DEF_WPMODE_PIN;
    WPHIST[0] = DEF_WPMODE_KANG;


    //LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    ucWperr = halWarp_SetWpKeyAng(m_sWArpGeoPara.PM_WPKEYANG_H, m_sWArpGeoPara.PM_WPKEYANG_V);

    //LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    #ifndef CUSTOM_OPTOMA   //optoma no check      //H2PF_Simon_0130
    ucWperr = halWarp_CheckWpLimit(&PS_WPLIMIT);
    #endif

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    if(!ucWperr)
    {
        halWarp_WriteWpTable();
        halWarp_SetLut();
    }

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    if(ucWperr)
    {
        LOG_MSG(db_HAL_WARPING, "Over limits!! [ERR_CODE=%d]\r\n", ucWperr);
        m_sWArpGeoPara.PM_WPKEYANG_H = m_sWArpGeoPara.WPKEYANG_H_OLD;
        m_sWArpGeoPara.PM_WPKEYANG_V = m_sWArpGeoPara.WPKEYANG_V_OLD;
    }
    else
    {
        WPON = 1;
        m_sWArpGeoPara.WPKEYANG_H_OLD = m_sWArpGeoPara.PM_WPKEYANG_H;
        m_sWArpGeoPara.WPKEYANG_V_OLD = m_sWArpGeoPara.PM_WPKEYANG_V;
    }

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
#ifdef CUSTOM_CHRISTIE
 	halBasicWarpSettingSet();	//G100_Doulas_0027
#else
 	halBasicWarpSettingSet();
 	halBasicBlendSettingSet();
#endif
    return (ucWperr == 0) ? rcSUCCESS : rcERROR;
}

// ==============================================================================
// FUNCTION NAME: appC786ParameterSet
// DESCRIPTION:
//
//
// Params:
// eCOMMAND_ID eType:
// int *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/12/15, Larry Create
// //A70LV_Larry_0053 modify
// --------------------
// ==============================================================================
void halWarp_GeometryParameterSet(eWARP_EVENT eType, void *pcData)
{
    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return;
    }

    switch(eType)
    {
        case eWARP_EVENT_WARPOFF:
            {
                UINT8 ucTemp =  m_sWArpGeoPara.TABLE_COLOR; //A70LV_Larry_0074

                memset((UINT8*)&m_sWArpGeoPara, 0, sizeof(m_sWArpGeoPara));
                m_sWArpGeoPara.PM_WPKEYANG_H = 0;
                m_sWArpGeoPara.PM_WPKEYANG_V = 0;
                m_sWArpGeoPara.PM_WPPINWID_H = 0;
                m_sWArpGeoPara.PM_WPPINWID_V = 0;
                m_sWArpGeoPara.PM_WPKEY4C_TLX = 0;
                m_sWArpGeoPara.PM_WPKEY4C_TLY = 0;
                m_sWArpGeoPara.PM_WPKEY4C_TRX = 0;
                m_sWArpGeoPara.PM_WPKEY4C_TRY = 0;
                m_sWArpGeoPara.PM_WPKEY4C_BLX = 0;
                m_sWArpGeoPara.PM_WPKEY4C_BLY = 0;
                m_sWArpGeoPara.PM_WPKEY4C_BRX = 0;
                m_sWArpGeoPara.PM_WPKEY4C_BRY = 0;

                m_sWArpGeoPara.TABLE_COLOR = ucTemp; //A70LV_Larry_0074

                m_sWArpGeoPara.POLATION_H = DEFAULT_WAPR_FILTER_LUT_H;  //A35G2_Simon_0115
                m_sWArpGeoPara.POLATION_V = DEFAULT_WAPR_FILTER_LUT_V;  //A35G2_Simon_0115
                m_sWArpGeoPara.m_cWarpFilterSelect_H = DEFAULT_WAPR_FILTER_LUT_H;  //A35G2_Simon_0115
                m_sWArpGeoPara.m_cWarpFilterSelect_V = DEFAULT_WAPR_FILTER_LUT_V;  //A35G2_Simon_0115
                m_sWArpGeoPara.m_cWarpAutoFilter = TRUE;
                #if (WARPING_AP_AUTO_FILTER == TRUE)            //A35G2_Simon_0110
                m_sWArpGeoPara.m_cWarpAutoFilterAp = TRUE;
                m_sWArpGeoPara.m_cWarpFilterApValue_H = DEFAULT_WAPR_FILTER_LUT_H;
                m_sWArpGeoPara.m_cWarpFilterApValue_V = DEFAULT_WAPR_FILTER_LUT_V;
                #else
                m_sWArpGeoPara.m_cWarpAutoFilterAp = FALSE;
                m_sWArpGeoPara.m_cWarpFilterApValue_H = DEFAULT_WAPR_FILTER_LUT_H;
                m_sWArpGeoPara.m_cWarpFilterApValue_V = DEFAULT_WAPR_FILTER_LUT_V;
                #endif

                if((m_sHalWarpingInfo.uiWarp_HW > 1920) || (m_sHalWarpingInfo.uiWarp_VW > 1200))
                {
                    m_sHalWarpingInfo.ucIs4KWarp = 1;
                    m_sHalWarpingInfo.uiWpSpace = DEF_WP_SPACE_32P;
                    m_sHalWarpingInfo.uiWpSpaceBitShift = DEF_WP_SPACE_BIT_32P;
                }
                else
                {
                    m_sHalWarpingInfo.ucIs4KWarp = 0;
                    m_sHalWarpingInfo.uiWpSpace = DEF_WP_SPACE_16P;
                    m_sHalWarpingInfo.uiWpSpaceBitShift = DEF_WP_SPACE_BIT_16P;
                }

                m_sHalWarpingInfo.uiPixelGridHW = (m_sHalWarpingInfo.uiWarp_HW + m_sHalWarpingInfo.uiWpSpace - 1) / m_sHalWarpingInfo.uiWpSpace; //roundup
                m_sHalWarpingInfo.uiPixelGridVW = (m_sHalWarpingInfo.uiWarp_VW + m_sHalWarpingInfo.uiWpSpace - 1) / m_sHalWarpingInfo.uiWpSpace; //roundup

                m_sHalWarpingInfo.fWarp_PHWHF = (m_sHalWarpingInfo.uiWarp_HW / 2.0);
                m_sHalWarpingInfo.fWarp_PVWHF = (m_sHalWarpingInfo.uiWarp_VW / 2.0);
                WPMODE = DEF_WPMODE_TH;
                // for cascade function
                WPON = 0;
                WPHIST[0] = DEF_WPMODE_TH; // new
                WPHIST[1] = DEF_WPMODE_TH; // old
                WPHIST[2] = DEF_WPMODE_TH; // older
                WPHIST[3] = DEF_WPMODE_TH; // oldest
                halWarp_CalcNoWarp(0);
                halWarp_CalcNoWarp(1);
            }
            break;

        case eWARP_EVENT_KEYSTONE_H:
            {
                INT8 ucKeystone_H = *(INT8 *)pcData;
                ucKeystone_H = ucKeystone_H - KEYSTONE_BASED;
                m_sWArpGeoPara.PM_WPKEYANG_H = (float)(ucKeystone_H * 2);
            }
            break;

        case eWARP_EVENT_KEYSTONE_V:
            {
                INT8 ucKeystone_V = *(INT8 *)pcData;
                ucKeystone_V = ucKeystone_V - KEYSTONE_BASED;
                m_sWArpGeoPara.PM_WPKEYANG_V = (float)ucKeystone_V;
            }
            break;

        case eWARP_EVENT_KEYSTONE_HV:
            {
                INT8 ucKeystone_H = *(INT8 *)pcData;
                INT8 ucKeystone_V = *((INT8 *)pcData + 1);

                ucKeystone_H = ucKeystone_H - KEYSTONE_BASED;
                ucKeystone_V = ucKeystone_V - KEYSTONE_BASED;

                m_sWArpGeoPara.PM_WPKEYANG_H = (float)(ucKeystone_H * 2);
                m_sWArpGeoPara.PM_WPKEYANG_V = (float)ucKeystone_V;
            }
            break;

            //A70_Eva_0007 End
        case eWARP_EVENT_PINBARREL_H:
            {
                INT8 ucPin_H = *(INT8 *)pcData;
                ucPin_H = ucPin_H - PINBARREL_BASED;
                m_sWArpGeoPara.PM_WPPINWID_H = (float)(ucPin_H * 2 * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H));
            }
            break;

        case eWARP_EVENT_PINBARREL_V:
            {
                INT8 ucPin_V = *(INT8 *)pcData;
                ucPin_V = ucPin_V - PINBARREL_BASED;
                m_sWArpGeoPara.PM_WPPINWID_V = (float)(ucPin_V * 2 * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V));
            }
            break;

        case eWARP_EVENT_PINBARREL_HV:
            {
                INT8 ucPin_H = *(INT8 *)pcData;
                INT8 ucPin_V = *((INT8 *)pcData+1);

                ucPin_H = ucPin_H - PINBARREL_BASED;
                ucPin_V = ucPin_V - PINBARREL_BASED;

                m_sWArpGeoPara.PM_WPPINWID_H = (float)(ucPin_H * 2 * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H));
                m_sWArpGeoPara.PM_WPPINWID_V = (float)(ucPin_V * 2 * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V));
            }
            break;

        case eWARP_EVENT_4C_TL_X:
            m_sWArpGeoPara.PM_WPKEY4C_TLX = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_TL_Y:
             m_sWArpGeoPara.PM_WPKEY4C_TLY = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_TR_X:
             m_sWArpGeoPara.PM_WPKEY4C_TRX = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_TR_Y:
             m_sWArpGeoPara.PM_WPKEY4C_TRY = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_BL_X:
             m_sWArpGeoPara.PM_WPKEY4C_BLX = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_BL_Y:
             m_sWArpGeoPara.PM_WPKEY4C_BLY = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_BR_X:
             m_sWArpGeoPara.PM_WPKEY4C_BRX = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_BR_Y:
             m_sWArpGeoPara.PM_WPKEY4C_BRY = *(INT16 *)pcData;
            break;

        case eWARP_EVENT_4C_ALL:
            {
               m_sWArpGeoPara.PM_WPKEY4C_TLX = *((INT16 *)pcData + 0);
               m_sWArpGeoPara.PM_WPKEY4C_TLY = *((INT16 *)pcData + 1);
               m_sWArpGeoPara.PM_WPKEY4C_TRX = *((INT16 *)pcData + 2);
               m_sWArpGeoPara.PM_WPKEY4C_TRY = *((INT16 *)pcData + 3);
               m_sWArpGeoPara.PM_WPKEY4C_BLX = *((INT16 *)pcData + 4);
               m_sWArpGeoPara.PM_WPKEY4C_BLY = *((INT16 *)pcData + 5);
               m_sWArpGeoPara.PM_WPKEY4C_BRX = *((INT16 *)pcData + 6);
               m_sWArpGeoPara.PM_WPKEY4C_BRY = *((INT16 *)pcData + 7);
            }
            break;

        case eWARP_EVENT_WARP_H_FILTER_USER_SEL:
            m_sWArpGeoPara.m_cWarpFilterSelect_H = *(UINT8 *)pcData;
            break;

        case eWARP_EVENT_WARP_V_FILTER_USER_SEL:
            m_sWArpGeoPara.m_cWarpFilterSelect_V = *(UINT8 *)pcData;
            break;

        case eWARP_EVENT_WARP_H_FILTER_AP_SEL:
            m_sWArpGeoPara.m_cWarpFilterApValue_H = *(UINT8 *)pcData;
            break;

        case eWARP_EVENT_WARP_V_FILTER_AP_SEL:
            m_sWArpGeoPara.m_cWarpFilterApValue_V = *(UINT8 *)pcData;
            break;

        case eWARP_EVENT_USER_WARP_FILTER_AUTO_ENABLE:
            m_sWArpGeoPara.m_cWarpAutoFilter = *(BOOL *)pcData;
            break;

        case eWARP_EVENT_AP_WARP_FILTER_AUTO_ENABLE:
            m_sWArpGeoPara.m_cWarpAutoFilterAp = *(BOOL *)pcData;
            break;

        default:
            ASSERT_ALWAYS();
            break;
    }
}

void halWarp_GeometryParameterGet(eWARP_EVENT eType, void *pcData) //A70LV_Larry_0220
{
    switch(eType)
    {
        case eWARP_EVENT_DEFAULT_FILTER_H:
            *(UINT8 *)pcData = (UINT8)m_sWArpGeoPara.POLATION_H;
            break;

        case eWARP_EVENT_DEFAULT_FILTER_V:
            *(UINT8 *)pcData = (UINT8)m_sWArpGeoPara.POLATION_V;
            break;

        case eWARP_EVENT_WARP_H_FILTER_USER_SEL:
            *(UINT8 *)pcData = (UINT8)m_sWArpGeoPara.m_cWarpFilterSelect_H;
            break;

        case eWARP_EVENT_WARP_V_FILTER_USER_SEL:
            *(UINT8 *)pcData = (UINT8)m_sWArpGeoPara.m_cWarpFilterSelect_V;
            break;

        case eWARP_EVENT_WARP_H_FILTER_AP_SEL:
            *(UINT8 *)pcData = (UINT8)m_sWArpGeoPara.m_cWarpFilterApValue_H;
            break;

        case eWARP_EVENT_WARP_V_FILTER_AP_SEL:
            *(UINT8 *)pcData = (UINT8)m_sWArpGeoPara.m_cWarpFilterApValue_V;
            break;

        case eWARP_EVENT_USER_WARP_FILTER_AUTO_ENABLE:
            *(BOOL *)pcData = (BOOL)m_sWArpGeoPara.m_cWarpAutoFilter;
            break;

        case eWARP_EVENT_AP_WARP_FILTER_AUTO_ENABLE:
            *(BOOL *)pcData = (BOOL)m_sWArpGeoPara.m_cWarpAutoFilterAp;
            break;

        case eWARP_EVENT_WARP_PARA:
            {
                sWARP_BASIC *sWarpBasic = (sWARP_BASIC*)pcData;

                sWarpBasic->PM_WPKEYANG_H   = m_sWArpGeoPara.PM_WPKEYANG_H;
                sWarpBasic->PM_WPKEYANG_V   = m_sWArpGeoPara.PM_WPKEYANG_V;
                sWarpBasic->WPKEYANG_H_OLD  = m_sWArpGeoPara.WPKEYANG_H_OLD;
                sWarpBasic->WPKEYANG_V_OLD  = m_sWArpGeoPara.WPKEYANG_V_OLD;
                sWarpBasic->PM_WPPINWID_H   = m_sWArpGeoPara.PM_WPPINWID_H / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);  //H2PF_Simon_0152
                sWarpBasic->PM_WPPINWID_V   = m_sWArpGeoPara.PM_WPPINWID_V / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);  //H2PF_Simon_0152
                sWarpBasic->PM_WPKEY4C_TLX  = m_sWArpGeoPara.PM_WPKEY4C_TLX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
                sWarpBasic->PM_WPKEY4C_TLY  = m_sWArpGeoPara.PM_WPKEY4C_TLY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
                sWarpBasic->PM_WPKEY4C_TRX  = m_sWArpGeoPara.PM_WPKEY4C_TRX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
                sWarpBasic->PM_WPKEY4C_TRY  = m_sWArpGeoPara.PM_WPKEY4C_TRY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
                sWarpBasic->PM_WPKEY4C_BLX  = m_sWArpGeoPara.PM_WPKEY4C_BLX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
                sWarpBasic->PM_WPKEY4C_BLY  = m_sWArpGeoPara.PM_WPKEY4C_BLY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
                sWarpBasic->PM_WPKEY4C_BRX  = m_sWArpGeoPara.PM_WPKEY4C_BRX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
                sWarpBasic->PM_WPKEY4C_BRY  = m_sWArpGeoPara.PM_WPKEY4C_BRY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
                sWarpBasic->TABLE_COLOR     = m_sWArpGeoPara.TABLE_COLOR;

                sWarpBasic->POLATION_H      = m_sWArpGeoPara.POLATION_H;
                sWarpBasic->POLATION_V      = m_sWArpGeoPara.POLATION_V;
                sWarpBasic->m_cWarpFilterSelect_H   = m_sWArpGeoPara.m_cWarpFilterSelect_H;
                sWarpBasic->m_cWarpFilterSelect_V   = m_sWArpGeoPara.m_cWarpFilterSelect_V;
                sWarpBasic->m_cWarpFilterApValue_H  = m_sWArpGeoPara.m_cWarpFilterApValue_H;
                sWarpBasic->m_cWarpFilterApValue_V  = m_sWArpGeoPara.m_cWarpFilterApValue_V;
                sWarpBasic->m_cWarpAutoFilter       = m_sWArpGeoPara.m_cWarpAutoFilter;
                sWarpBasic->m_cWarpAutoFilterAp     = m_sWArpGeoPara.m_cWarpAutoFilterAp;
            }
            break;

        default:
            ASSERT_ALWAYS();
            break;
    }
}

// ==============================================================================
// FUNCTION NAME: halWarp_InterpolationSet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/12/19, Larry Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarp_InterpolationSet(void)  //G100_Simon_0089     //A35G2_Simon_0110
{
#ifdef WARP_ENABLE
    // +++++ set LUT registers +++++ //
    UINT32 GV_RTCT = 0;
    UINT8 aucBuffer[POLATION_NUMBER] = {0}; //A70LV_Larry_0061
    UINT8 ucWarpFilterLutIndex_H = 0;
    UINT8 ucWarpFilterLutIndex_V = 0;
	UINT8 ucWriteLutEnable = 1;	//A70Gen2_Doulas_0056
    UINT8 ch, bank_ofst;


    //if(halWarping_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        GV_RTCT = (UINT8)dvC341_Read(B0_RTCT6, 0);
        dvC341_Write(B0_RTCT6, 0xFF, 0);

        LOG_MSG(db_HAL_WARPING, "(%s, %d) start\n", __FUNCTION__, __LINE__);

        //Use AP value
        #ifdef CUSTOM_CHRISTIE
        if(halWarping_TwistLinkFlag_Get() == TRUE || m_sHalWarpingInfo.ucCurrentWarpingType == WARPING_TYPE_AP_MYSTIQUE)  //A35G2_Simon_0115
        #else
        if(utilWarp_GetWarpCtrl() == WARP_CTRL__AP)
        #endif
        {
            ucWarpFilterLutIndex_H = m_sWArpGeoPara.m_cWarpFilterApValue_H;
            ucWarpFilterLutIndex_V = m_sWArpGeoPara.m_cWarpFilterApValue_V;

            LOG_MSG(db_HAL_WARPING, "WarpFilterSelect Ap (%d %d)\n", ucWarpFilterLutIndex_H, ucWarpFilterLutIndex_V);
        }
        //Use OSD value
        else if((m_sWArpGeoPara.m_cWarpAutoFilter == FALSE) &&
                (m_sWArpGeoPara.m_cWarpFilterSelect_H < POLATION_MAX) &&
                (m_sWArpGeoPara.m_cWarpFilterSelect_V < POLATION_MAX))
        {
            ucWarpFilterLutIndex_H = m_sWArpGeoPara.m_cWarpFilterSelect_H;
            ucWarpFilterLutIndex_V = m_sWArpGeoPara.m_cWarpFilterSelect_V;

    	    //dvC341Geo_AdaptiveScaleFilterLutEnable(FALSE);   //A35G2_Simon_0115

            LOG_MSG(db_HAL_WARPING, "WarpFilterSelect OSD Manual (%d %d)\n", ucWarpFilterLutIndex_H, ucWarpFilterLutIndex_V);
        }
        //Auto
        else
        {
            ucWarpFilterLutIndex_H = m_sWArpGeoPara.POLATION_H;
            ucWarpFilterLutIndex_V = m_sWArpGeoPara.POLATION_V;

            m_sWArpGeoPara.m_cWarpFilterSelect_H = m_sWArpGeoPara.POLATION_H;
            m_sWArpGeoPara.m_cWarpFilterSelect_V = m_sWArpGeoPara.POLATION_V;

            LOG_MSG(db_HAL_WARPING, "WarpFilterSelect Auto (%d %d)\n", ucWarpFilterLutIndex_H, ucWarpFilterLutIndex_V);
        }

    	if(ucWriteLutEnable)
    	{
    	    for(ch = eC341_CH1; ch < OUTPUT_CH_NUM; ch++)
            {
                if(ch == eC341_CH1)
                {
            	    memcpy(aucBuffer, &m_sPolationTable[ucWarpFilterLutIndex_H][0], POLATION_NUMBER);
            	    dvC341_BurstWrite_AddInc(B146_WPHLUT0CH1, POLATION_NUMBER, aucBuffer);
            	    memcpy(aucBuffer, &m_sPolationTable[ucWarpFilterLutIndex_V][0], POLATION_NUMBER);
            	    dvC341_BurstWrite_AddInc(B146_WPVLUT0CH1, POLATION_NUMBER, aucBuffer);
                }
                else if(ch == eC341_CH2)
                {
            	    memcpy(aucBuffer, &m_sPolationTable[ucWarpFilterLutIndex_H][0], POLATION_NUMBER);
            	    dvC341_BurstWrite_AddInc(B151_WPHLUT0CH2, POLATION_NUMBER, aucBuffer);
            	    memcpy(aucBuffer, &m_sPolationTable[ucWarpFilterLutIndex_V][0], POLATION_NUMBER);
                    dvC341_BurstWrite_AddInc(B151_WPVLUT0CH2, POLATION_NUMBER, aucBuffer);
                }
            }
    	}

        dvC341_Write(B0_RTCT6, GV_RTCT, 0);
        //halWarping_SemaphoreGive(__FUNCTION__);

        LOG_MSG(db_HAL_WARPING, "(%s, %d) end\n", __FUNCTION__, __LINE__);
    }

#ifdef CUSTOM_CHRISTIE
    halBasicWarpSettingSet();
#endif

    return eHAL_WARPING_EXEC_CODE_PASS;

#else  // #ifdef WARP_ENABLE

    return eHAL_WARPING_EXEC_CODE_PASS;

#endif

}

// ==============================================================================
// FUNCTION NAME: halWarp_BlendingParameterSet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/12/19, Larry Create
// --------------------
// ==============================================================================
void halWarp_BlendingParameterSet(eBLENDING_EVENT eType, void *pcData)
{
#if 1
    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return;
    }

    switch(eType)
    {
        case eBLENDING_EVENT_OFF:
            m_sBlendingPara.ucBlending_T_Enable = 0;
            m_sBlendingPara.ucBlending_B_Enable = 0;
            m_sBlendingPara.ucBlending_L_Enable = 0;
            m_sBlendingPara.ucBlending_R_Enable = 0;
            m_sBlendingPara.uiBlending_T_St = BLENDING_TOP_START_PIXEL_DEF_VALUE;    //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_B_St = BLENDING_BOTTOM_START_PIXEL_DEF_VALUE; //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_L_St = BLENDING_LEFT_START_PIXEL_DEF_VALUE;   //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_R_St = BLENDING_RIGHT_START_PIXEL_DEF_VALUE;  //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_T_Width = BLENDING_TOP_PIXEL_WIDTH_DEF_VALUE;    //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_B_Width = BLENDING_BOTTOM_PIXEL_WIDTH_DEF_VALUE; //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_L_Width = BLENDING_LEFT_PIXEL_WIDTH_DEF_VALUE;   //HICC2_Doulas_0026
            m_sBlendingPara.uiBlending_R_Width = BLENDING_RIGHT_PIXEL_WIDTH_DEF_VALUE;  //HICC2_Doulas_0026
            m_sBlendingPara.ucBlendingGamma = (UINT8)eCM_BLENDING_GAMMA_2_2; //A70LV_Larry_0171
            break;

        case eBLENDING_EVENT_T_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_sBlendingPara.ucBlending_T_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_B_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_sBlendingPara.ucBlending_B_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_L_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_sBlendingPara.ucBlending_L_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_R_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_sBlendingPara.ucBlending_R_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_T_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_T_St = uiValue;
            }
            break;

        case eBLENDING_EVENT_B_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_B_St = uiValue;

                //LOG_MSG(db_ALWAYS, "m_sBlendingPara.uiBlending_B_St %d\r\n", m_sBlendingPara.uiBlending_B_St);
            }
            break;

        case eBLENDING_EVENT_L_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_L_St = uiValue;
            }
            break;

        case eBLENDING_EVENT_R_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_R_St = uiValue;
            }
            break;

        case eBLENDING_EVENT_T_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_T_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_B_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_B_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_L_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_L_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_R_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_sBlendingPara.uiBlending_R_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_GAMMA: //A70LV_Larry_0171
            {
                UINT8 ucGamma = *(UINT8 *)pcData;

                m_sBlendingPara.ucBlendingGamma = ucGamma;
            }
            break;

        case eBLENDING_EVENT_GAMMA_DMA: //A70LV_Larry_0171
            m_sBlendingPara.ucBlendingGamma = BLENDING_GAMMA_DMA;
            break;

        case eBLENDING_EVENT_ALL: //A70LV_Larry_0139
            {
                UINT8 *Data = (UINT8 *)pcData;
                m_sBlendingPara.ucBlending_T_Enable =   *(Data + 0);
                m_sBlendingPara.uiBlending_T_St =       *(Data + 2) << 8 | *(Data + 1);
                m_sBlendingPara.uiBlending_T_Width =    *(Data + 4) << 8 | *(Data + 3);
                m_sBlendingPara.ucBlending_B_Enable =   *(Data + 5);
                m_sBlendingPara.uiBlending_B_St =       *(Data + 7) << 8 | *(Data + 6);
                m_sBlendingPara.uiBlending_B_Width =    *(Data + 9) << 8 | *(Data + 8);
                m_sBlendingPara.ucBlending_L_Enable =   *(Data + 10);
                m_sBlendingPara.uiBlending_L_St =       *(Data + 12) << 8 | *(Data + 11);
                m_sBlendingPara.uiBlending_L_Width =    *(Data + 14) << 8 | *(Data + 13);
                m_sBlendingPara.ucBlending_R_Enable =   *(Data + 15);
                m_sBlendingPara.uiBlending_R_St =       *(Data + 17) << 8 | *(Data + 16);
                m_sBlendingPara.uiBlending_R_Width =    *(Data + 19) << 8 | *(Data + 18);
            }
            break;

        default:
            ASSERT_ALWAYS();
            break;
   }
#endif /* 0 */
}

// ==============================================================================
// FUNCTION NAME: halWarp_BlendingSet
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/12/19, Larry Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarp_BlendingSet(void)   //G100_Simon_0089
{
    UINT16 uiEGBCT = 0;
    INT32  iCoef = 0;
    float  fCoef = 0;
    UINT16 uiWidth = 0;
    int GV_RTCT = 0;
    sEDGE_BLENDING_PARA sEdgeBlending = {0};

    //GV_EGBCT = ( PM_EGB_GAMMA_EN == 0 ) ? 0x3000 : 0x3010;

    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    LOG_MSG(db_HAL_WARPING, "(func:%s) Start\r\n", __func__);

    if(!(uiEGBCT & 0x0010)) //b4:Edge blend gamma enable
    {
        halWarp_BlendingGammaSet();
    }

    sEdgeBlending.ucBlending_T_Enable = m_sBlendingPara.ucBlending_T_Enable;
    sEdgeBlending.ucBlending_B_Enable = m_sBlendingPara.ucBlending_B_Enable;
    sEdgeBlending.ucBlending_L_Enable = m_sBlendingPara.ucBlending_L_Enable;
    sEdgeBlending.ucBlending_R_Enable = m_sBlendingPara.ucBlending_R_Enable;
    sEdgeBlending.uiBlending_T_St = m_sBlendingPara.uiBlending_T_St;
    sEdgeBlending.uiBlending_B_St = m_sBlendingPara.uiBlending_B_St;
    sEdgeBlending.uiBlending_L_St = m_sBlendingPara.uiBlending_L_St;
    sEdgeBlending.uiBlending_R_St = m_sBlendingPara.uiBlending_R_St;
    sEdgeBlending.uiBlending_T_Width = m_sBlendingPara.uiBlending_T_Width;
    sEdgeBlending.uiBlending_B_Width = m_sBlendingPara.uiBlending_B_Width;
    sEdgeBlending.uiBlending_L_Width = m_sBlendingPara.uiBlending_L_Width;
    sEdgeBlending.uiBlending_R_Width = m_sBlendingPara.uiBlending_R_Width;

    uiEGBCT = halWarp_EdgeBlendingSet(sEdgeBlending);

#ifdef CUSTOM_CHRISTIE
    halBasicBlendSettingSet();
#else
    halBasicWarpSettingSet();
    halBasicBlendSettingSet();
#endif

    return eHAL_WARPING_EXEC_CODE_PASS;
}

UINT16 halWarp_EdgeBlendingSet(sEDGE_BLENDING_PARA sEdgeBlending)
{
    UINT16 uiEGBCT = 0;
    INT32  iCoef = 0;
    float  fCoef = 0;
    UINT16 uiWidth = 0;
    int GV_RTCT = 0;

    //GV_EGBCT = ( PM_EGB_GAMMA_EN == 0 ) ? 0x3000 : 0x3010;

    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    LOG_MSG(db_HAL_WARPING, "(func:%s) Start\r\n", __func__);

#ifdef C789_WARPING_ENABLE

    uiEGBCT = (UINT16)dvC789_Read(B7_EGBCT);

    uiEGBCT |= 0x1080;
    uiEGBCT &= 0xFFF0;

    GV_RTCT = dvC789_Read(BN_RTCT);
    GV_RTCT = GV_RTCT & RTCT_WARP_TRANSFER_DISABLE;
    dvC789_WriteToBuffer( BN_RTCT, GV_RTCT);

    //==========Left==========
    if(sEdgeBlending.ucBlending_L_Enable)
    {
       uiWidth = sEdgeBlending.uiBlending_L_Width;
       fCoef = ((sEdgeBlending.ucBlending_L_Enable == 0) || (uiWidth < 4) ) ? 0 : 1 / ((float)uiWidth - 1) * 65536;
       iCoef = (INT32)fCoef;

       if( fCoef > (float)iCoef)
       {
           iCoef += 1;
       }

       if(iCoef > 0x7fff )
       {
           iCoef = 0x7fff;
       }
    }

    dvC789_WriteToBuffer( B7_LEGBHST, sEdgeBlending.uiBlending_L_St);
    if((sEdgeBlending.ucBlending_L_Enable == 0) || (iCoef == 0))
    {
        dvC789_WriteToBuffer( B7_HEGBINIT, 0x80);
        dvC789_WriteToBuffer( B7_LEGBHW, 0x0000);
        dvC789_WriteToBuffer( B7_LEGBCOEF, 0x0000);
    }
    else
    {
        uiEGBCT = uiEGBCT | 0x0011;
        dvC789_WriteToBuffer( B7_HEGBINIT, 0x00);
        dvC789_WriteToBuffer( B7_LEGBHW, uiWidth);
        dvC789_WriteToBuffer( B7_LEGBCOEF, (iCoef & 0xFFFF));
    }

    //========================


    //=========Right==========
    if(sEdgeBlending.ucBlending_R_Enable)
    {
        uiWidth = sEdgeBlending.uiBlending_R_Width;
        fCoef = ((sEdgeBlending.ucBlending_R_Enable == 0) || (uiWidth < 4) ) ? 0 : ( 1 - 1 / ((float)uiWidth - 1) ) * 65536;
        iCoef = (INT32)fCoef;

        if(iCoef < 0x8000)
        {
            iCoef = 0x8000;
        }
    }

    dvC789_WriteToBuffer( B7_REGBHST, (m_sHalWarpingInfo.uiWarp_HW - sEdgeBlending.uiBlending_R_St -uiWidth));
    if(sEdgeBlending.ucBlending_R_Enable == 0 )
    {
        dvC789_WriteToBuffer( B7_REGBHW, 0x0000);
        dvC789_WriteToBuffer( B7_REGBCOEF, 0x0000);
    }
    else
    {
        uiEGBCT = uiEGBCT | 0x0012;
        dvC789_WriteToBuffer( B7_REGBHW, uiWidth);
        dvC789_WriteToBuffer( B7_REGBCOEF, (iCoef & 0xFFFF));
    }
    //========================

    //==========TOP===========
    if(sEdgeBlending.ucBlending_T_Enable)
    {
        uiWidth = sEdgeBlending.uiBlending_T_Width;
        fCoef = ((sEdgeBlending.ucBlending_T_Enable == 0) || (uiWidth < 4) ) ? 0 : 1 / ((float)uiWidth - 1) * 65536;
        iCoef = (INT32)fCoef;

        if( fCoef > (float)iCoef)
        {
            iCoef += 1;
        }

        if(iCoef > 0x7fff)
        {
            iCoef = 0x7fff;
        }
    }

    dvC789_WriteToBuffer( B7_TEGBVST, sEdgeBlending.uiBlending_T_St);
    if((sEdgeBlending.ucBlending_T_Enable == 0) || (iCoef == 0))
    {
        dvC789_WriteToBuffer( B7_VEGBINIT, 0x80);
        dvC789_WriteToBuffer( B7_TEGBVW, 0x0000);
        dvC789_WriteToBuffer( B7_TEGBCOEF, 0x0000);
    }
    else
    {
        uiEGBCT = uiEGBCT | 0x0014;
        dvC789_WriteToBuffer( B7_VEGBINIT, 0x00);
        dvC789_WriteToBuffer( B7_TEGBVW, uiWidth);
        dvC789_WriteToBuffer( B7_TEGBCOEF, (iCoef & 0xFFFF));
    }

    //========================

    //=========Bottom=========
    if(sEdgeBlending.ucBlending_B_Enable)
    {
        uiWidth = sEdgeBlending.uiBlending_B_Width;
        fCoef = ((sEdgeBlending.ucBlending_B_Enable == 0) || (uiWidth < 4) ) ? 0 : ( 1 - 1 / ((float)uiWidth - 1) ) * 65536;
        iCoef = (INT32)fCoef;

        if(iCoef < 0x8000)
        {
            iCoef = 0x8000;
        }
    }
    dvC789_WriteToBuffer( B7_BEGBVST, (m_sHalWarpingInfo.uiWarp_VW - sEdgeBlending.uiBlending_B_St - uiWidth));
    if(sEdgeBlending.ucBlending_B_Enable == 0)
    {
        dvC789_WriteToBuffer( B7_BEGBVW, 0x0000);
        dvC789_WriteToBuffer( B7_BEGBCOEF, 0x0000);
    }
    else
    {
        uiEGBCT = uiEGBCT | 0x0018;
        dvC789_WriteToBuffer( B7_BEGBVW,  uiWidth);
        dvC789_WriteToBuffer( B7_BEGBCOEF, (iCoef & 0xFFFF));
    }
    //========================

    dvC789_WriteToBuffer( B7_EGBCT, uiEGBCT);

    GV_RTCT = GV_RTCT | RTCT_WARP_WPPOVS_ENABLE;
    dvC789_WriteToBuffer( BN_RTCT, GV_RTCT);

    GV_RTCT = GV_RTCT & RTCT_WARP_TRANSFER_DISABLE;
    GV_RTCT = GV_RTCT | RTCT_WARP_TRANSFER_ENABLE;

    dvC789_WriteToBuffer( BN_RTCT, GV_RTCT);
    dvC789_Buffer_Flush();

#endif /* C789_WARPING_ENABLE */


#ifdef C341_WARPING_ENABLE

    GV_RTCT = dvC341_Read(B0_RTCT0, 0);
    //GV_RTCT = GV_RTCT & RTCT_WARP_TRANSFER_DISABLE;  //C341 wait review
    dvC341_WriteToBuffer( B0_RTCT0, 0x00, 0);

    for(eC341_CH eCH = 0; eCH < OUTPUT_CH_NUM ; eCH++)
    {
        uiEGBCT = (UINT16)dvC341Geo_Read(B147_WPEGBCTCH1, 0);
        uiEGBCT |= 0x1000;
        uiEGBCT &= 0xFF70;

        //==========Left==========
        if(eCH == eC341_CH1)  //Left only for CH1
        {
            if(sEdgeBlending.ucBlending_L_Enable)
            {
               uiWidth = sEdgeBlending.uiBlending_L_Width;
               fCoef = ((sEdgeBlending.ucBlending_L_Enable == 0) || (uiWidth < 4) ) ? 0 : 1 / ((float)uiWidth - 1) * 65536;
               iCoef = (INT32)fCoef;

               if( fCoef > (float)iCoef)
               {
                   iCoef += 1;
               }

               if(iCoef > 0x7fff )
               {
                   iCoef = 0x7fff;
               }
            }

            dvC341Geo_WriteToBuffer( B147_WPLEGBHSTCH1, sEdgeBlending.uiBlending_L_St, eCH);
            if((sEdgeBlending.ucBlending_L_Enable == 0) || (iCoef == 0))
            {
                dvC341Geo_WriteToBuffer( B147_WPHEGBINITCH1, 0x80, eCH);
                dvC341Geo_WriteToBuffer( B147_WPLEGBHWCH1, 0x0000, eCH);
                dvC341Geo_WriteToBuffer( B147_WPLEGBCOEFCH1, 0x0000, eCH);
            }
            else
            {
                uiEGBCT = uiEGBCT | 0x0011;
                dvC341Geo_WriteToBuffer( B147_WPHEGBINITCH1, 0x00, eCH);
                dvC341Geo_WriteToBuffer( B147_WPLEGBHWCH1, uiWidth, eCH);
                dvC341Geo_WriteToBuffer( B147_WPLEGBCOEFCH1, (iCoef & 0xFFFF), eCH);
            }
        }
        else
        {
            dvC341Geo_WriteToBuffer( B147_WPLEGBHSTCH1, 0, eCH);
            dvC341Geo_WriteToBuffer( B147_WPHEGBINITCH1, 0x80, eCH);
            dvC341Geo_WriteToBuffer( B147_WPLEGBHWCH1, 0x0000, eCH);
            dvC341Geo_WriteToBuffer( B147_WPLEGBCOEFCH1, 0x0000, eCH);
        }
        //========================


        //=========Right==========
        if(eCH == eC341_CH2)  //Right only for CH2
        {
            if(sEdgeBlending.ucBlending_R_Enable)
            {
                uiWidth = sEdgeBlending.uiBlending_R_Width;
                fCoef = ((sEdgeBlending.ucBlending_R_Enable == 0) || (uiWidth < 4) ) ? 0 : ( 1 - 1 / ((float)uiWidth - 1) ) * 65536;
                iCoef = (INT32)fCoef;

                if(iCoef < 0x8000)
                {
                    iCoef = 0x8000;
                }
            }

            dvC341Geo_WriteToBuffer( B147_WPREGBHSTCH1, (m_sHalWarpingInfo.uiWarp_HW_1CH - sEdgeBlending.uiBlending_R_St -uiWidth), eCH);
            if(sEdgeBlending.ucBlending_R_Enable == 0 )
            {
                dvC341Geo_WriteToBuffer( B147_WPREGBHWCH1, 0x0000, eCH);
                dvC341Geo_WriteToBuffer( B147_WPREGBCOEFCH1, 0x0000, eCH);
            }
            else
            {
                uiEGBCT = uiEGBCT | 0x0012;
                dvC341Geo_WriteToBuffer( B147_WPREGBHWCH1, uiWidth, eCH);
                dvC341Geo_WriteToBuffer( B147_WPREGBCOEFCH1, (iCoef & 0xFFFF), eCH);
            }
        }
        else
        {
            dvC341Geo_WriteToBuffer( B147_WPREGBHSTCH1, 0, eCH);
            dvC341Geo_WriteToBuffer( B147_WPREGBHWCH1, 0x0000, eCH);
            dvC341Geo_WriteToBuffer( B147_WPREGBCOEFCH1, 0x0000, eCH);
        }
        //========================

        //==========TOP===========
        //for CH1, CH2
        if(sEdgeBlending.ucBlending_T_Enable)
        {
            uiWidth = sEdgeBlending.uiBlending_T_Width;
            fCoef = ((sEdgeBlending.ucBlending_T_Enable == 0) || (uiWidth < 4) ) ? 0 : 1 / ((float)uiWidth - 1) * 65536;
            iCoef = (INT32)fCoef;

            if( fCoef > (float)iCoef)
            {
                iCoef += 1;
            }

            if(iCoef > 0x7fff)
            {
                iCoef = 0x7fff;
            }
        }

        dvC341Geo_WriteToBuffer( B147_WPTEGBVSTCH1, sEdgeBlending.uiBlending_T_St, eCH);
        if((sEdgeBlending.ucBlending_T_Enable == 0) || (iCoef == 0))
        {
            dvC341Geo_WriteToBuffer( B147_WPVEGBINITCH1, 0x80, eCH);
            dvC341Geo_WriteToBuffer( B147_WPTEGBVWCH1, 0x0000, eCH);
            dvC341Geo_WriteToBuffer( B147_WPTEGBCOEFCH1, 0x0000, eCH);
        }
        else
        {
            uiEGBCT = uiEGBCT | 0x0014;
            dvC341Geo_WriteToBuffer( B147_WPVEGBINITCH1, 0x00, eCH);
            dvC341Geo_WriteToBuffer( B147_WPTEGBVWCH1, uiWidth, eCH);
            dvC341Geo_WriteToBuffer( B147_WPTEGBCOEFCH1, (iCoef & 0xFFFF), eCH);
        }

        //========================

        //=========Bottom=========
        //for CH1, CH2
        if(sEdgeBlending.ucBlending_B_Enable)
        {
            uiWidth = sEdgeBlending.uiBlending_B_Width;
            fCoef = ((sEdgeBlending.ucBlending_B_Enable == 0) || (uiWidth < 4) ) ? 0 : ( 1 - 1 / ((float)uiWidth - 1) ) * 65536;
            iCoef = (INT32)fCoef;

            if(iCoef < 0x8000)
            {
                iCoef = 0x8000;
            }
        }
        dvC341Geo_WriteToBuffer( B147_WPBEGBVSTCH1, (m_sHalWarpingInfo.uiWarp_VW - sEdgeBlending.uiBlending_B_St - uiWidth), eCH);
        if(sEdgeBlending.ucBlending_B_Enable == 0)
        {
            dvC341Geo_WriteToBuffer( B147_WPBEGBVWCH1, 0x0000, eCH);
            dvC341Geo_WriteToBuffer( B147_WPBEGBCOEFCH1, 0x0000, eCH);
        }
        else
        {
            uiEGBCT = uiEGBCT | 0x0018;
            dvC341Geo_WriteToBuffer( B147_WPBEGBVWCH1,  uiWidth, eCH);
            dvC341Geo_WriteToBuffer( B147_WPBEGBCOEFCH1, (iCoef & 0xFFFF), eCH);
        }
        //========================

        dvC341Geo_WriteToBuffer( B147_WPEGBCTCH1, uiEGBCT, eCH);
    }

    dvC341_Write( B0_RTCT0, GV_RTCT, 0);

#endif /* C789_WARPING_ENABLE */

    return uiEGBCT;
}

eHAL_WARPING_EXEC_CODE halWarp_BlendingGammaSet(void) //A70LV_Larry_0171    //G100_Simon_0089
{
    float fCoef = 2.2;

    switch(m_sBlendingPara.ucBlendingGamma)
    {
        case eCM_BLENDING_GAMMA_1_8:
            fCoef = 1.8;
            break;

        case eCM_BLENDING_GAMMA_1_9:
            fCoef = 1.9;
            break;

        case eCM_BLENDING_GAMMA_2_0:
            fCoef = 2.0;
            break;

        case eCM_BLENDING_GAMMA_2_1:
            fCoef = 2.1;
            break;

        case eCM_BLENDING_GAMMA_2_2:
            fCoef = 2.2;
            break;

        case eCM_BLENDING_GAMMA_2_3:
            fCoef = 2.3;
            break;

        case eCM_BLENDING_GAMMA_2_4:
            fCoef = 2.4;
            break;

        case BLENDING_GAMMA_DMA: //not to do anything
            return rcSUCCESS;

        default:
            break;
    }


    halWarp_BlendingGammaCoefSet(fCoef);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarp_BlendingGammaCoefSet(float fCoef)
{
	float fGm;
    UINT16 uiGammaTable[1024] = {0};
    int uiCount = 0;
    UINT16 uiEGBCT = 0;
    UINT32 ulRTCT  = 0;

    if(fCoef == 0)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    //LOG_MSG(db_ALWAYS, "BlendingGamma fCoef %.3f \r\n", fCoef);

    for ( uiCount = 0; uiCount < 1024; uiCount++ )
    {
        fGm  = pow( ((float)uiCount / 1024), 1 / fCoef ) * 65536 + 0.5;
        uiGammaTable[uiCount] = ( fGm < 0 ) ? 0 : ( fGm > 65535 ) ? 65535 : (int)fGm;
    }

#ifdef C789_WARPING_ENABLE

    uiEGBCT = (UINT16)dvC789_Read(B7_EGBCT);

    ulRTCT = dvC789_Read(BN_RTCT);

    ulRTCT = ulRTCT & RTCT_WARP_TRANSFER_DISABLE;

    dvC789_WriteToBuffer( BN_RTCT, ulRTCT);

    //dvC789_Write( B7_EGBCT, ((uiEGBCT & 0xf0ff) | 0x0e00));
    dvC789_WriteToBuffer( B7_EGBCT, 0x1180 |  (uiEGBCT & 0x000f));

    dvC789_WriteToBuffer( B7_EGBGMAD, 0x00);
    dvC789_WriteToBuffer( B7_EGBGMAD, 0x00);
    dvC789_Buffer_Flush();

    dvC789_BurstWrite_FixedAdd(B7_EGBGMDT, 2048, (UINT8 *)&uiGammaTable[0]);

    uiEGBCT = (uiEGBCT & 0xffcf) | 0x0010;

    dvC789_WriteToBuffer(B7_EGBCT, uiEGBCT);

    dvC789_Buffer_Flush();

    ulRTCT = ulRTCT | RTCT_WARP_TRANSFER_ENABLE;

    dvC789_Write(BN_RTCT, ulRTCT);
#endif /* 0 */


#ifdef C341_WARPING_ENABLE
    ulRTCT = dvC341_Read(B0_RTCT0, 0);
    dvC341_Write( B0_RTCT0, 0, 0);

    for(eC341_CH eCH = eC341_CH1; eCH<OUTPUT_CH_NUM;  eCH++)
    {
        uiEGBCT = (UINT16)dvC341Geo_Read(B147_WPEGBCTCH1, eCH);

        //dvC341Geo_Write( B147_WPEGBCTCH1, ((uiEGBCT & 0xf0ff) | 0x0e00));
    	dvC341Geo_WriteToBuffer( B147_WPEGBCTCH1, 0x1100 |  (uiEGBCT & 0x00ff), eCH);
        dvC341Geo_WriteToBuffer( B147_WPEGBGMADCH1, 0x00, eCH);
        dvC341Geo_WriteToBuffer( B147_WPEGBGMADCH1, 0x00, eCH);
        dvC341Geo_Buffer_Flush();

        if(eCH == eC341_CH1)
        {
            dvC341Geo_BurstWrite_FixedAdd(B147_WPEGBGMDTCH1, sizeof(uiGammaTable), (UINT8 *)&uiGammaTable[0]);  //2048
        }
        else if(eCH == eC341_CH2)
        {
            dvC341Geo_BurstWrite_FixedAdd(B152_WPEGBGMDTCH2, sizeof(uiGammaTable), (UINT8 *)&uiGammaTable[0]);  //2048
        }

        uiEGBCT = (uiEGBCT & 0xfe4f) | 0x0010;

        dvC341Geo_WriteToBuffer(B147_WPEGBCTCH1, uiEGBCT, eCH);
    }

    dvC341Geo_Buffer_Flush();

    dvC341_Write(B0_RTCT0, ulRTCT, 0);

#endif /* 0 */
}

void halWarp_BlendingParameterGet(eBLENDING_EVENT eType, void *pcData)
{
#if 1
    switch(eType)
    {
        case eBLENDING_EVENT_ALL: //A70LV_Larry_0139
            {

                sBLENDING_BASIC *sBlendungBasic = (sBLENDING_BASIC*)pcData;

                sBlendungBasic->ucBlending_T_Enable = m_sBlendingPara.ucBlending_T_Enable;
                sBlendungBasic->uiBlending_T_St     = m_sBlendingPara.uiBlending_T_St;
                sBlendungBasic->uiBlending_T_Width  = m_sBlendingPara.uiBlending_T_Width;
                sBlendungBasic->ucBlending_B_Enable = m_sBlendingPara.ucBlending_B_Enable;
                sBlendungBasic->uiBlending_B_St     = m_sBlendingPara.uiBlending_B_St;
                sBlendungBasic->uiBlending_B_Width  = m_sBlendingPara.uiBlending_B_Width;
                sBlendungBasic->ucBlending_L_Enable = m_sBlendingPara.ucBlending_L_Enable;
                sBlendungBasic->uiBlending_L_St     = m_sBlendingPara.uiBlending_L_St;
                sBlendungBasic->uiBlending_L_Width  = m_sBlendingPara.uiBlending_L_Width;
                sBlendungBasic->ucBlending_R_Enable = m_sBlendingPara.ucBlending_R_Enable;
                sBlendungBasic->uiBlending_R_St     = m_sBlendingPara.uiBlending_R_St;
                sBlendungBasic->uiBlending_R_Width  = m_sBlendingPara.uiBlending_R_Width;
                sBlendungBasic->ucBlendingGamma  = m_sBlendingPara.ucBlendingGamma;
            }
            break;

        default:
            ASSERT_ALWAYS();
            break;
   }
#endif /* 0 */
}

#endif /* 0 */ //A70LV_Larry_0051


//G100_Tim_0024, add, start
// ==============================================================================
// FUNCTION: halWarping_Color_Uniformity_Disable( )
// DESCRIPTION: Disable Color Uniformity function.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/07/06, Tim Chen Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Disable(void)
{
    dvC341Geo_ColorUniformity_Control(0, 0);		//A65_OPTOMA_Doulas_0153 Modify
    return eHAL_WARPING_EXEC_CODE_PASS;
}
//G100_Tim_0024, add, end
//A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
// ==============================================================================
// FUNCTION: halWarping_Color_Uniformity_Enable( )
// DESCRIPTION: Enable Color Uniformity function.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/09/23, CL Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Enable(void)
{
    dvC341Geo_ColorUniformity_Control(1, 0);
    return eHAL_WARPING_EXEC_CODE_PASS;
}



//A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
UINT8 halWarping_Color_Uniformity_Enable_Get(void)
{
    return dvC341Geo_ColorUniformityEnable_Get();
}

// ==============================================================================
// FUNCTION NAME:
// DESCRIPTION: Set the data to the iChip C789
// Params: *pcData : the write data
// Params: uiSize : the write length
// Returns: eHAL_WARPING_EXEC_CODE_PASS : OK
// Returns: eHAL_WARPING_EXEC_CODE_MUTEX_FAIL : NG
// Modification History
//
// --------------------
// 2019/07/25, Larry Create
// 2021/06/21, Tim Modify
// --------------------
// ==============================================================================

#if 0 //for C789 format to C341
#define COLOR_UNIFORMITY_RAWDATA_X      11
#define COLOR_UNIFORMITY_RAWDATA_Y      9
#define COLOR_UNIFORMITY_RAWDATA_COLOR  3 //RGB

#define COLOR_UNIFORMITY_C341_X     17
#define COLOR_UNIFORMITY_C341_Y     13
#define COLOR_UNIFORMITY_X_OFFSET   5

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    UINT8 D0;
    UINT8 D1;
    UINT8 D2;
    UINT8 D3;
    UINT8 D4;
}sColorUniformityDataFormat;

typedef struct
{
    sColorUniformityDataFormat sX[COLOR_UNIFORMITY_RAWDATA_X]; //11
    sColorUniformityDataFormat sY[COLOR_UNIFORMITY_RAWDATA_Y];  //20
    sColorUniformityDataFormat sGinit[COLOR_UNIFORMITY_RAWDATA_COLOR]; //23
    sColorUniformityDataFormat sDgDv[COLOR_UNIFORMITY_RAWDATA_Y*COLOR_UNIFORMITY_RAWDATA_COLOR]; //50
    sColorUniformityDataFormat sDgDh[COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR]; //83
    sColorUniformityDataFormat sDgDhDv[COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_Y*COLOR_UNIFORMITY_RAWDATA_COLOR];
}sColorUniformityRawData;

#pragma pack(pop)                          // restore previous alignment
#endif /* 0 */

eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Set(UINT8 *pcData, UINT16 uiSize)  //G100_Owen_0019
{
#if 0
    UINT16 uiData = 0;
    UINT16 uiCountX = 0;
    UINT16 uiCountY = 0;
    UINT16 uiCountColor = 0;
    sColorUniformityRawData sRawDataCH1 = {0};
    sColorUniformityRawData sRawDataCH2 = {0};
#endif /* 0 */

    UINT16 uiCount = 0;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) set data len: %d\r\n", __FUNCTION__, __LINE__, uiSize);

    dvC341Geo_ColorUniformity_Control(0, 1);

    //CH1
    //////////////////////////////////////
    dvC341_Write(B13_UCADCH1, 0x00, 0);
    dvC341_Write(B13_UCADCH1, 0x00, 0);

    //write to C341 Ch0
    //burst write has size limit 508, but color  uniformity size is 3930

#if 1
    for(uiCount = 0; uiCount < 3930; uiCount++)
    {
        dvC341_WriteToBuffer(B13_UCDTCH1, (UINT8)pcData[uiCount], 0);
    }
#else
    memcpy((BYTE*)&sRawDataCH1, pcData, sizeof(sColorUniformityRawData));

    for(uiCountX = 0; uiCountX < COLOR_UNIFORMITY_C341_X; uiCountX++)
    {
        if(uiCountX < COLOR_UNIFORMITY_X_OFFSET)
        {
            uiData = (UINT16)sRawDataCH1.sX[uiCountX].D1;
            uiData = uiData<<8;
            uiData += (UINT16)sRawDataCH1.sX[uiCountX].D0;

            dvC341_WriteToBuffer(B13_UCDTCH1, uiData & 0xFF, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, (uiData>>8) & 0xFF, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sX[uiCountX].D2, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sX[uiCountX].D3, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sX[uiCountX].D4, 0);
        }
        else
        {
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x0F, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
        }
    }


    for(uiCountY = 0; uiCountY < COLOR_UNIFORMITY_C341_Y; uiCountY++)
    {
        if(uiCountY < COLOR_UNIFORMITY_RAWDATA_Y)
        {
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sY[uiCountY].D0, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sY[uiCountY].D1, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sY[uiCountY].D2, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sY[uiCountY].D3, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sY[uiCountY].D4, 0);
            uiData = (UINT16)sRawDataCH1.sY[uiCountY].D1;
            uiData = uiData<<8;
            uiData += (UINT16)sRawDataCH1.sY[uiCountY].D0;
        }
        else
        {
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x60, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x09, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
        }
    }

    for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
    {
        dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sGinit[uiCountColor].D0, 0);
        dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sGinit[uiCountColor].D1, 0);
        dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sGinit[uiCountColor].D2, 0);
        dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sGinit[uiCountColor].D3, 0);
        dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sGinit[uiCountColor].D4, 0);
    }

    for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
    {
        for(uiCountY = 0; uiCountY < COLOR_UNIFORMITY_C341_Y; uiCountY++)
        {
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
        }
    }

    for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
    {
        for(uiCountX = 0; uiCountX < COLOR_UNIFORMITY_C341_X; uiCountX++)
        {
            if(uiCountX < COLOR_UNIFORMITY_X_OFFSET)
            {
                dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D0, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D1, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D2, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D3, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D4, 0);
            }
            else
            {
                dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
            }
        }
    }

    for(uiCountY = 0; uiCountY < COLOR_UNIFORMITY_C341_Y; uiCountY++)
    {
        for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
        {
            for(uiCountX = 0; uiCountX < COLOR_UNIFORMITY_C341_X; uiCountX++)
            {
                if(uiCountX < COLOR_UNIFORMITY_X_OFFSET && uiCountY < COLOR_UNIFORMITY_RAWDATA_Y)
                {
                    dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D0, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D1, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D2, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D3, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, sRawDataCH1.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D4, 0);
                }
                else
                {
                    dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                    dvC341_WriteToBuffer(B13_UCDTCH1, 0x00, 0);
                }
            }
        }
    }
#endif /* 0 */
    dvC341_Buffer_Flush();
    //////////////////////////////////////


    //CH2
    //////////////////////////////////////
    dvC341_Write(B29_UCADCH2, 0x00, 0);
    dvC341_Write(B29_UCADCH2, 0x00, 0);

    //write to C341 Ch1
    //burst write has size limit 508, but color  uniformity size is 3930

#if 1
    for(uiCount = 0; uiCount < 3930; uiCount++)
    {
        dvC341_WriteToBuffer(B29_UCDTCH2, (UINT8)pcData[uiCount + 3930], 0);
    }
#else
    memcpy((BYTE*)&sRawDataCH2, &pcData[3930], sizeof(sColorUniformityRawData));

    for(uiCountX = 0; uiCountX < COLOR_UNIFORMITY_C341_X; uiCountX++)
    {
        if(uiCountX <= COLOR_UNIFORMITY_X_OFFSET)
        {
            uiData = (UINT16)sRawDataCH2.sX[uiCountX].D1;
            uiData = uiData<<8;
            uiData += (UINT16)sRawDataCH2.sX[uiCountX].D0;
            uiData -= 1920;

            dvC341_WriteToBuffer(B29_UCDTCH2, uiData & 0xFF, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, (uiData>>8) & 0xFF, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sX[uiCountX].D2, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sX[uiCountX].D3, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sX[uiCountX].D4, 0);

            printf("CH2 X = %d\n", uiData);
        }
        else
        {
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x0F, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
        }
    }

    for(uiCountY = 0; uiCountY < COLOR_UNIFORMITY_C341_Y; uiCountY++)
    {
        if(uiCountY < COLOR_UNIFORMITY_RAWDATA_Y)
        {
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sY[uiCountY].D0, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sY[uiCountY].D1, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sY[uiCountY].D2, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sY[uiCountY].D3, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sY[uiCountY].D4, 0);
            uiData = (UINT16)sRawDataCH2.sY[uiCountY].D1;
            uiData = uiData<<8;
            uiData += (UINT16)sRawDataCH2.sY[uiCountY].D0;
            printf("CH2 Y = %d\n", uiData);
        }
        else
        {
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x60, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x09, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
        }
    }

    for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
    {
        dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sGinit[uiCountColor].D0, 0);
        dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sGinit[uiCountColor].D1, 0);
        dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sGinit[uiCountColor].D2, 0);
        dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sGinit[uiCountColor].D3, 0);
        dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sGinit[uiCountColor].D4, 0);
    }

    for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
    {
        for(uiCountY = 0; uiCountY < COLOR_UNIFORMITY_C341_Y; uiCountY++)
        {
            if(uiCountY < COLOR_UNIFORMITY_RAWDATA_Y)
            {
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDv[uiCountY + uiCountColor*COLOR_UNIFORMITY_RAWDATA_Y].D0, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDv[uiCountY + uiCountColor*COLOR_UNIFORMITY_RAWDATA_Y].D1, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDv[uiCountY + uiCountColor*COLOR_UNIFORMITY_RAWDATA_Y].D2, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDv[uiCountY + uiCountColor*COLOR_UNIFORMITY_RAWDATA_Y].D3, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDv[uiCountY + uiCountColor*COLOR_UNIFORMITY_RAWDATA_Y].D4, 0);
            }
            else
            {
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            }
        }
    }

    for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
    {
        for(uiCountX = 0; uiCountX < COLOR_UNIFORMITY_C341_X; uiCountX++)
        {
            if(uiCountX <= COLOR_UNIFORMITY_X_OFFSET)
            {
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D0, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D1, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D2, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D3, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDh[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X].D4, 0);
            }
            else
            {
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
            }
        }
    }

    for(uiCountY = 0; uiCountY < COLOR_UNIFORMITY_C341_Y; uiCountY++)
    {
        for(uiCountColor = 0; uiCountColor < COLOR_UNIFORMITY_RAWDATA_COLOR; uiCountColor++)
        {
            for(uiCountX = 0; uiCountX < COLOR_UNIFORMITY_C341_X; uiCountX++)
            {
                if(uiCountX <= COLOR_UNIFORMITY_X_OFFSET && uiCountY < COLOR_UNIFORMITY_RAWDATA_Y)
                {
                    dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D0, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D1, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D2, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D3, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, sRawDataCH2.sDgDhDv[uiCountX + uiCountColor*COLOR_UNIFORMITY_RAWDATA_X + uiCountY*COLOR_UNIFORMITY_RAWDATA_X*COLOR_UNIFORMITY_RAWDATA_COLOR].D4, 0);
                }
                else
                {
                    dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                    dvC341_WriteToBuffer(B29_UCDTCH2, 0x00, 0);
                }
            }
        }
    }
#endif /* 1 */
    dvC341_Buffer_Flush();


    //////////////////////////////////////

    dvC341Geo_ColorUniformity_Control(1, 0);

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) OK\r\n", __FUNCTION__, __LINE__);
    return eHAL_WARPING_EXEC_CODE_PASS;

}


//A65_OPTOMA_CL_0002  //A65_OPTOMA_CL_0004
eHAL_WARPING_EXEC_CODE halWarping_Set_OSD_Transparent( UINT8 ucPaletteIndex )
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    halWarping_OSDWriteTransparentColor(TRUE,
                                        ucPaletteIndex,
                                        ucPaletteIndex,
                                        ucPaletteIndex,
                                        ucPaletteIndex);

    return (eRet);
}

eHAL_WARPING_EXEC_CODE halWarping_PowerNormal(const ePANEL_ID ePanelId) //A70LV_Larry_0051
{
    BOOL bIs4K = FALSE;
    //eHAL_WARPING_EXEC_CODE eExeRet;
    halWarping_IsDefaultWarpTable_Set(TRUE);  //H2PF_Simon_0131
    dvC341Geo_Init(ePanelId, halScaler_Low_Latency_Get());

//#ifdef WARP_ENABLE
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //bIs4K = halWarping_Is4KPanel(ePanelId);

    //reset Global variable
    memset(&m_sHalWarpingInfo.sWarpTable, 0, sizeof(sWARP_TABLE));
    memset(ucTempPalette, 0, sizeof(ucTempPalette));   //G100_Simon_0007
    memset(ucTempTranparentColor, 0, sizeof(ucTempTranparentColor));   //G100_Simon_0007
    m_sHalWarpingInfo.ucColorMask = 0;

    halWarping_Panel_Update(ePanelId);
    halWarping_OSD_ON(); //A35G2_CDS_Simon_0044
    halWarping_FreezeImage(FALSE);
    halWarpOSD_ClearOSD();
    halWarpOSD_DisableOSD();
    //halWarping_CursorPanelConfig(); //A70LV_Larry_0137
    halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
    halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
    halWarp_BlendingGammaSet();  //R70G2_Simon_0001
    halWarp_BlacklevelParameterSet(eBKLV_EVENT_RESET, TRUE);
    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);


    halWarp_GeometrySet();
    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    //halWarping_Color_Uniformity_Disable();
    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    //halWarping_Color_Uniformity_Init();

    #if (ADAPTIVE_SCALE_WARP_FILTER_LUT == TRUE)
    //halWarping_AdaptiveScaleFilterLutInit();    //A35G2_Simon_0115
    #endif

    LOG_MSG(db_HAL_WARPING, "<%s %d>\r\n", __FUNCTION__, __LINE__);
    return eHAL_WARPING_EXEC_CODE_PASS;
}


//C341 wait review
eHAL_WARPING_EXEC_CODE halWarping_Init(void)     //(const eHAL_WARPING_WARP_MODE eMode, const eHAL_WARPING_MOVE_PITCH eMoveIdx) //A70LV_Larry_0051
{
    //UINT8 ucIndex, ucIndex2;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    m_OSD_MEM_LAYER0 = DEF_OSDSAD_LAYER3_4K;   //H2PF_Simon_0107
    m_OSD_MEM_LAYER1 = DEF_OSDSAD_LAYER4_4K;   //H2PF_Simon_0107
    m_OSD_CURRENT_LAYER = m_OSD_MEM_LAYER0;    //H2PF_Simon_0107

    m_sHalWarpingInfo.bWarpingInit = TRUE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(pthread_mutex_init(&m_sHalWarpingInfo.xSemaphore, NULL) != 0)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_init fail [%s]\n", __FUNCTION__ , strerror(errno));
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }
#else
    #if(CURRENT_RTOS_TYPE == RTOS_STATIC)
        m_sHalWarpingInfo.xSemaphore = xSemaphoreCreateMutexStatic(&m_sHalWarpingInfo.xMutexBuffer);
    #else
        m_sHalWarpingInfo.xSemaphore = xSemaphoreCreateMutex();
    #endif

    if(m_sHalWarpingInfo.xSemaphore == NULL)
        return eHAL_WARPING_EXEC_CODE_FAIL;
#endif


    //TODO
    //To get eeprom data from upper layer and set to m_sHalWarpingInfo parameter
    //m_sHalWarpingInfo.eWarpMode = eMode;
    m_sHalWarpingInfo.bGridShow = FALSE;//In the init status you never show grid
    m_sHalWarpingInfo.bInternalGridShow = TRUE;

    m_sHalWarpingInfo.ucGridSelX = 0;
    m_sHalWarpingInfo.ucGridSelY = 0;

    m_sHalWarpingInfo.ucPrevGridSelX = 0;
    m_sHalWarpingInfo.ucPrevGridSelY = 0;

    m_sHalWarpingInfo.eWarpDecimalPartRes = eWDP_3BIT;

    halWarping_IsDefaultWarpTable_Set(TRUE);  //H2PF_Simon_0131

#if 0 //A70LV_Larry_0137 mask
    for(ucIndex2 = 0; ucIndex2 < 17; ucIndex2++)
    {
        for(ucIndex = 0; ucIndex < 17; ucIndex++)
        {
            m_sHalWarpingInfo.baGridCursorDisplayed[ucIndex][ucIndex2] = FALSE;// Clear all grid cursor displayed flag
        }
    }
#endif /* 0 */

    m_sHalWarpingInfo.ePatternGen = eHAL_WARPING_CROSSHATCH_MODE_OFF;
    m_sHalWarpingInfo.uiBlendingEdgeHW = DEF_BLENDING_EDGE_HW;
    m_sHalWarpingInfo.uiBlendingEdgeVW = DEF_BLENDING_EDGE_VW;

#ifdef C341_WARPING_ENABLE
    //dvC341_InitWarpTable();
#endif /* C341_WARPING_ENABLE */
#ifdef C789_WARPING_ENABLE
    //dvC341Geo_InitWarpTable();
#endif /* C789_WARPING_ENABLE */

    memset(&m_sHalWarpingInfo.sWarpTable, 0, sizeof(sWARP_TABLE));
    memset(ucTempPalette, 0, sizeof(ucTempPalette));   //G100_Simon_0007
    memset(ucTempTranparentColor, 0, sizeof(ucTempTranparentColor));   //G100_Simon_0007

    //halWarping_Blacklevel_Init();   //H2PF_Simon_0107

    m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColorEnable = TRUE;

    #if 0
    {
    //  unsigned int rdt;
        int oaoihst;
        int oaoivst;
        int syren;
        int ch, bank_ofst;

        oaoihst = dvC341_Read(B144_WPOAOI0HSTCH1, 0);
        oaoivst = dvC341_Read(B144_WPOAOI0VSTCH1, 0);
        syren = ((dvC341_Read(B144_WPOSYCTCH1, 0) & 0x0002) == 0) ? 0 : 1;

        for (ch = 0; ch < 4; ch++) {
            if (ch < 2) {// CH1-2
                ///// Warping
                bank_ofst = (ch * 5);
                // Warping
                if (ch == 0) {// CH1
                    dvC341_Write(B144_WPMCTCH1, 0x11, bank_ofst);
                    dvC341_Write(B144_WPOSYCTCH1, (0x0001 + (syren << 1)), bank_ofst);
                    dvC341_Write(B145_WPWTRCTCH1, 0x00, bank_ofst);
                    dvC341_Write(B145_WPLBFCTCH1, 0x00, bank_ofst);
                }
                else {// CH2
                    dvC341_Write(B144_WPMCTCH1, 0x11, bank_ofst);
                    dvC341_Write(B144_WPOSYCTCH1, (0x1011 + (syren << 1)), bank_ofst);
                    dvC341_Write(B145_WPWTRCTCH1, 0x00, bank_ofst);
                    dvC341_Write(B145_WPLBFCTCH1, 0x00, bank_ofst);
                }
                dvC341_Write(B148_WPOSFLD0CH1, PS_WPOSFLD[0], bank_ofst);
                dvC341_Write(B148_WPOSFLD1CH1, PS_WPOSFLD[1], bank_ofst);
                dvC341_Write(B148_WPOSFLD2CH1, PS_WPOSFLD[2], bank_ofst);
                dvC341_Write(B148_WPOSFLD3CH1, PS_WPOSFLD[3], bank_ofst);
                ///// Output side
                // OSYNC
                dvC341_Write(B144_WPFLDDLYCH1, (oaoivst + 3840 * 2 / 3), bank_ofst);
                // OACT
                dvC341_Write(B144_WPOACTHSTCH1, oaoihst, bank_ofst);
                dvC341_Write(B144_WPOACTHWCH1, 1920, bank_ofst);
                dvC341_Write(B144_WPOACTVSTCH1, oaoivst, bank_ofst);
                dvC341_Write(B144_WPOACTVWCH1, 2400, bank_ofst);
                dvC341_Write(B144_WPOAOI0HENDCH1, (oaoihst + (3840/2)), bank_ofst);
                dvC341_Write(B144_WPOAOI0VENDCH1, (oaoivst + 2400), bank_ofst);
                ///// Input side
                bank_ofst = (ch * 16);
                // Measure Input
                /*
                write_C341(B16_PTHCH1, 0x80, bank_ofst);
                rdt = read_C341(B16_DIFCTCH1, bank_ofst);
                write_C341(B16_DIFCTCH1, (rdt & 0xfe), bank_ofst);
                rdt = read_C341(B16_PIH0CH1, bank_ofst);
                write_C341(B16_PIH0CH1, (rdt & 0xbfff), bank_ofst);
                */
                // Warping Cursor

            }
            else {// CH3-4
                ///// Warping
                bank_ofst = (ch * 5);
                // Warping
                dvC341_Write(B144_WPMCTCH1, 0x00, bank_ofst);
                dvC341_Write(B144_WPOSYCTCH1, 0x0000, bank_ofst);
                dvC341_Write(B145_WPDTCTCH1, 0x0000, bank_ofst);
                dvC341_Write(B145_WPWTRCTCH1, 0x00, bank_ofst);
                dvC341_Write(B145_WPLBFCTCH1, 0x00, bank_ofst);
                dvC341_Write(B145_WPIACTHWCH1, 0x0000, bank_ofst);
                dvC341_Write(B145_WPIACTVWCH1, 0x0000, bank_ofst);
                ///// Output side
                // OACT
                dvC341_Write(B144_WPOACTHWCH1, 0, bank_ofst);
                dvC341_Write(B144_WPOACTVWCH1, 0, bank_ofst);
            }
        }

    }
    #endif


    return eHAL_WARPING_EXEC_CODE_PASS;
}

void halWarping_TwistLinkFlag_Set(BOOL bTwistLink) //A70LV_Larry_0319
{
    m_bWarpTwistLinkFlag = bTwistLink;
}

BOOL halWarping_TwistLinkFlag_Get(void) //A70LV_Larry_0319
{
    return m_bWarpTwistLinkFlag;
}

eHAL_WARPING_EXEC_CODE halWarping_EnableSet(UINT8 ucEnable)
{
    return eHAL_WARPING_EXEC_CODE_PASS;
}

// AP API Start
static void halWarping_DBD_WriteDummy(void)
{
    #if 0   //C341 wait review
    UINT8 ucIdx;

    dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_BLEND_ADDRESS + ((OSD_DBD_8BIT_MWI * 256) * m_sHalWarpingInfo.uiWarp_VW));     //R70G2_Simon_0003
    for(ucIdx=0; ucIdx<8; ucIdx++)
    {
        dvC341Geo_WriteToBuffer(B4_CPUDT, 0xFF);
    }
    #endif
}

//#define AP_WARPING_TABLE_LOG
//C341 wait review
eHAL_WARPING_EXEC_CODE halWarping_convertTPtoDTDT(FLOAT *tpx, FLOAT *tpy, UINT16 width, UINT16 height)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT16 uiPixelGridHW = (width-1);
    UINT16 uiPixelGridVW = (height-1);

    for(int y = DEF_WP_OUT_VGRD + uiPixelGridVW; y >= DEF_WP_OUT_VGRD; y--)
    {
        for(int x = DEF_WP_OUT_HGRD + uiPixelGridHW; x >= DEF_WP_OUT_HGRD; x--)
        {
            m_sHalWarpingInfo.sWarpTable.Px[DEF_CASC_NEW_TBL][x][y] = *(tpx + (y-DEF_WP_OUT_VGRD)*(uiPixelGridHW+1) + (x-DEF_WP_OUT_HGRD));
            m_sHalWarpingInfo.sWarpTable.Py[DEF_CASC_NEW_TBL][x][y] = *(tpy + (y-DEF_WP_OUT_VGRD)*(uiPixelGridHW+1) + (x-DEF_WP_OUT_HGRD));
        }
    }

    halWarp_WriteWpTable();

    //A35G2_Simon_0110
    /**** dvC341Geo_SetLut start ****/
    if(m_sWArpGeoPara.m_cWarpAutoFilterAp == TRUE)
    {
        halWarp_Calclut(&m_sWArpGeoPara.m_cWarpFilterApValue_H, &m_sWArpGeoPara.m_cWarpFilterApValue_V);
    }

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;
}


//C341 wait review
eHAL_WARPING_EXEC_CODE halWarping_ClearWarpingTable(void)
{
#if 0
    UINT8 i,j;
    UINT8 ucWidth, ucHeight;

    ucWidth = (m_sHalWarpingInfo.uiWarp_HW + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1;
    ucHeight = (m_sHalWarpingInfo.uiWarp_VW + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1;

    LOG_MSG(db_HAL_WARPING, "%s %d %d", __FUNCTION__, ucWidth, ucHeight);

    static float PxAry[((1920 + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1) * ((1200 + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1)];
    static float PyAry[((1920 + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1) * ((1200 + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1)];

    memset(PxAry, 0x00, sizeof(PxAry));
    memset(PyAry, 0x00, sizeof(PyAry));

    for(i = 0 ; i < ucHeight ; i++)
    {
        for(j = 0 ; j < ucWidth ; j++)
        {
            PxAry[i * ucWidth + j] = (float)(j * DEF_WP_SPACE);
            PyAry[i * ucWidth + j] = (float)(i * DEF_WP_SPACE);
        }
    }
#endif /* 0 */

    halWarp_CalcNoWarp(DEF_CASC_NEW_TBL);
    halWarp_WriteWpTable();

    if(m_sWArpGeoPara.m_cWarpAutoFilterAp == TRUE)
    {
        halWarp_Calclut(&m_sWArpGeoPara.m_cWarpFilterApValue_H, &m_sWArpGeoPara.m_cWarpFilterApValue_V);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_EdgeBlending_Disable(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT32 RegEGBCT_CH1 = 0;
    RegEGBCT_CH1 = dvC341Geo_Read(B147_WPEGBCTCH1, eC341_CH1);
    RegEGBCT_CH1 &= 0xFFF0;
    dvC341Geo_Write(B147_WPEGBCTCH1, RegEGBCT_CH1, eC341_CH1);

    UINT32 RegEGBCT_CH2 = 0;
    RegEGBCT_CH2 = dvC341Geo_Read(B147_WPEGBCTCH1, eC341_CH2);
    RegEGBCT_CH2 &= 0xFFF0;
    dvC341Geo_Write(B147_WPEGBCTCH1, RegEGBCT_CH2, eC341_CH2);

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DBD_BlendingTable_Set(UINT16 uiWidth, UINT16 uiHeight, UINT8 *pucDBD_Data)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT8 ucFastWrite = 0;
    UINT8 ucRetryCnt = 200;
    INT32 nEGBCT = 0xD090;
    dvC341Geo_WriteToBuffer(B148_WPEGBDBDSADCH1, DEF_WPEGBDBDSAD0, 0);
    dvC341Geo_WriteToBuffer(B148_WPEGBDBDMWICH1, OSD_DBD_8BIT_MWI, 0);
    dvC341Geo_WriteToBuffer(B148_WPEGBDBDSADCH1, DEF_WPEGBDBDSAD1, 1);
    dvC341Geo_WriteToBuffer(B148_WPEGBDBDMWICH1, OSD_DBD_8BIT_MWI, 1);

    dvC341Geo_WriteToBuffer(B147_WPEGBCTCH1, nEGBCT, 0);
    dvC341Geo_WriteToBuffer(B147_WPEGBCTCH1, nEGBCT, 1);
    dvC341Geo_Buffer_Flush();

    //Cindy: change RTCT to avoid blending not shown
    int GV_RTCT = dvC341Geo_Read(B0_RTCT6, 0); //0x0105;
    //dvC341Geo_WriteToBuffer( BN_RTCT, 0x0105);  //0x0105 is the initial value of RTCT   //C341 wait review

    UINT16 RegBBACTHW = uiWidth / OUTPUT_CH_NUM ;

#ifdef OSDCT_FASTEN_ON
    if (uiWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        uiWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        RegBBACTHW = uiWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
#endif

    //Clear blending
    dvC341Geo_WriteToBuffer(B4_OSDCT, 0x00, 0);
    dvC341Geo_WriteToBuffer(B4_OSDFILL, 0x00FF, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTHW, RegBBACTHW - 1, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTVW, uiHeight - 1, 0);
    dvC341Geo_WriteToBuffer(B4_BBWMWI, ((OSD_DBD_8BIT_MWI * EGBDBD_LINEFEED_BASE)/OSD_LINEFEED_BASE) & 0xff, 0);
    dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEGBDBDSAD0, 0);
    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
    }
    dvC341Geo_Buffer_Flush();

    while(((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) != 0) && (ucRetryCnt--))
    {
        usleep(10 * 1000);
    }
    dvC341Geo_Write(B4_OSDCT, 0x00, 0);

    dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEGBDBDSAD1, 0);
    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
    }
    dvC341Geo_Buffer_Flush();

    ucRetryCnt = 200;
    while(((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) != 0) && (ucRetryCnt--))
    {
        usleep(10 * 1000);
    }
    dvC341Geo_Write(B4_OSDCT, 0x00, 0);

    //Write blending
    int address;

    for(int idx = 0 ; idx < uiHeight ; idx++)
    {
        int nCount = 0, bWrite = 0;

        //first pixel of each line
        dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEGBDBDSAD0 + idx * 2048, 0); // BBWMWI * 128
        dvC341Geo_WriteToBuffer(B4_CPUDT, pucDBD_Data[uiWidth * idx + 0], 0);

        for(int j = 1 ; j < uiWidth/2 ; j++)
        {
            if(pucDBD_Data[uiWidth * idx + j] != 0xFF)
            {
                address = DEF_WPEGBDBDSAD0 + idx * 2048 + j;

                if(pucDBD_Data[uiWidth * idx + j-1] == 0xFF)    //if it is a start, then set address
                    dvC341Geo_WriteToBuffer(B4_CPUWAD, address, 0);

                dvC341Geo_WriteToBuffer(B4_CPUDT, pucDBD_Data[uiWidth * idx + j], 0);
            }
        }

        dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEGBDBDSAD1 + idx * 2048, 0); // BBWMWI * 128
        dvC341Geo_WriteToBuffer(B4_CPUDT, pucDBD_Data[uiWidth * idx + uiWidth/2], 0);      //H2PF_Simon_0100

        for(int j = uiWidth/2 + 1 ; j < uiWidth ; j++)   //H2PF_Simon_0100
        {
            if(pucDBD_Data[uiWidth * idx + j] != 0xFF)
            {
                address = DEF_WPEGBDBDSAD1 + idx * 2048 + (j - uiWidth/2);

                if(pucDBD_Data[uiWidth * idx + j-1] == 0xFF)    //if it is a start, then set address
                    dvC341Geo_WriteToBuffer(B4_CPUWAD, address, 0);

                dvC341Geo_WriteToBuffer(B4_CPUDT, pucDBD_Data[uiWidth * idx + j], 0);
            }
        }
        dvC341Geo_Buffer_Flush();
    }

    halWarping_DBD_WriteDummy();

    dvC341Geo_Buffer_Flush();

    //Restore RTCT
    dvC341Geo_Write( B0_RTCT6, GV_RTCT, 0);  //C341 wait review

    //reset BBWMWI to OSD MWI
    dvC341Geo_Write(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH / OSD_LINEFEED_BASE) & 0xff, 0);

    m_sBlendingPara.ulEGBCT = nEGBCT;

    eResult = eHAL_WARPING_EXEC_CODE_PASS;
    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DBD_ClearBlendingTable(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    UINT8 ucFastWrite = 0;
    INT32 lEGBCT = 0xD090;
    UINT8 ucRetryCnt = 200;
    UINT16 uiWidth = m_sHalWarpingInfo.uiWarp_HW_1CH;

    dvC341Geo_WriteToBuffer(B148_WPEGBDBDSADCH1, DEF_BLEND_ADDRESS, 0);
    dvC341Geo_WriteToBuffer(B148_WPEGBDBDMWICH1, OSD_DBD_8BIT_MWI, 0);

    dvC341Geo_WriteToBuffer(B147_WPEGBCTCH1, lEGBCT, 0);
    dvC341Geo_WriteToBuffer(B147_WPEGBCTCH1, lEGBCT, 1);


#ifdef OSDCT_FASTEN_ON
    if (uiWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        uiWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0 )
    {
        uiWidth = uiWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
#endif
    dvC341Geo_WriteToBuffer(B4_OSDCT, 0x00, 0);
    dvC341Geo_WriteToBuffer(B4_OSDFILL, 0xFF, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTHW, uiWidth - 1, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTVW, m_sHalWarpingInfo.uiWarp_VW - 1, 0);
    dvC341Geo_WriteToBuffer(B4_BBWMWI, ((OSD_DBD_8BIT_MWI * EGBDBD_LINEFEED_BASE)/OSD_LINEFEED_BASE) & 0xff, 0);

    dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEGBDBDSAD0, 0);
    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
    }
    dvC341Geo_Buffer_Flush();

    while(((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) != 0) && (ucRetryCnt--))
    {
        MS_SLEEP(10);
    }

    dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEGBDBDSAD1, 0);
    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
    }
    dvC341Geo_Buffer_Flush();

    while(((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) != 0) && (ucRetryCnt--))
    {
        MS_SLEEP(10);
    }

    dvC341Geo_Write(B4_OSDCT, 0x00, 0);

    //reset BBWMWI to OSD MWI
    dvC341Geo_Write(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH / OSD_LINEFEED_BASE) & 0xff, 0);

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DBD_Blending_Enable(UINT8 ucEnable)  //H2PF_Simon_0086
{
    UINT16 RegEGBCT_CH1 = (UINT16)dvC341Geo_Read(B147_WPEGBCTCH1, eC341_CH1);
    UINT16 RegEGBCT_CH2 = (UINT16)dvC341Geo_Read(B147_WPEGBCTCH1, eC341_CH2);  //H2PF_Simon_0164

    if(ucEnable)
    {
        RegEGBCT_CH1 |= BIT14;
        RegEGBCT_CH2 |= BIT14;
    }
    else
    {
        RegEGBCT_CH1 &= (~BIT14);
        RegEGBCT_CH2 &= (~BIT14);
    }

    dvC341Geo_Write(B147_WPEGBCTCH1, (UINT32)RegEGBCT_CH1, eC341_CH1);
    dvC341Geo_Write(B147_WPEGBCTCH1, (UINT32)RegEGBCT_CH2, eC341_CH2);

    return eHAL_WARPING_EXEC_CODE_PASS;
}


BOOL halWarping_DBD_Blending_EnableGet(void)  //H2PF_Simon_0086
{
    UINT32 RegEGBCT = dvC341_Read(B147_WPEGBCTCH1, 0);

    return  (RegEGBCT & BIT14) ? TRUE : FALSE ;
}

eHAL_WARPING_EXEC_CODE halWarping_clearBlackLevel(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

	for(int i=0; i<16; i++)
	{
		m_sHalWarpingInfo.aucBL_Palette[i][0] = 0;
		m_sHalWarpingInfo.aucBL_Palette[i][1] = 0;
		m_sHalWarpingInfo.aucBL_Palette[i][2] = 0;
    	halWarping_BlackLevel_Palette_Set(i, 0, 0, 0);
	}

    UINT16 BBACTHW = (2048 / 2) - 1;
    UINT16 BBACTVW = m_sHalWarpingInfo.uiWarp_VW - 1;
    UINT16 BBMWI = (2048 / 2) / OSD_LINEFEED_BASE;

    INT32 iRegBBWMWI = dvC341Geo_Read(B4_BBWMWI, 0);

    dvC341_Write(B4_CPUWAD, DEF_WPEBIASSAD0, 0);
    dvC341_Write(B4_OSDFILL, 0x00, 0);
    dvC341_Write(B4_BBACTHW, (BBACTHW) & 0xffff, 0);
    dvC341_Write(B4_BBACTVW, (BBACTVW) & 0x7fff, 0);
    dvC341_Write(B4_BBWMWI, BBMWI & 0xff, 0);
    dvC341_Write(B4_OSDCT, 0x01, 0);

    UINT8 ucRetryCnt = 200;
    while(((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) != 0) && (ucRetryCnt--))  //A35G2_Simon_0093
    {
        MS_SLEEP(10);
    }

    //for CH2
    dvC341Geo_Write(B148_WPEBIASSADCH1, DEF_WPEBIASSAD1, eC341_CH2);
    dvC341Geo_Write(B148_WPEBIASMWICH1, DEF_EBIASMWI&0xff, eC341_CH2);
    dvC341_Write(B4_CPUWAD, DEF_WPEBIASSAD1, 0);
    dvC341_Write(B4_OSDCT, 0x01, 0);
    ucRetryCnt = 200;
    while(((dvC341_Read(B4_BOSTAT, 0) & 0x01) != 0) && (ucRetryCnt--))  //A35G2_Simon_0093
    {
        MS_SLEEP(10);
    }


    dvC341_Write(B4_OSDCT, 0x00, 0);

    dvC341_Write(B4_BBWMWI, iRegBBWMWI, 0);  //A35G2_Simon_0093

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Enable(UINT8 ucEnable)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "%s %d\n\n" , __FUNCTION__, ucEnable);
    dvC341Geo_EdgebldBiasEnable(ucEnable);
    MS_SLEEP(100);   //A35G2_Simon_0103

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;
}

#define FAST_BKLV_TABLE_SET
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Table_Set(UINT8 *ucData, UINT16 uiWidth, UINT16 uiHeight)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT32 ulDataSize = (uiWidth * uiHeight / 2) ;
    UINT8 *ucTableData = (UINT8 *)malloc(ulDataSize);

    LOG_MSG(db_HAL_WARPING, "(%s %d) %dx%d Start\n", __FUNCTION__, __LINE__ , uiWidth, uiHeight);

    if(ucTableData == NULL)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(uiWidth % 2 != 0)  //Only can be used in even number
    {
        free(ucTableData);
        LOG_MSG(db_HAL_WARPING, "(%s %d) data error (width %d)\n", __FUNCTION__, __LINE__ , uiWidth);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    memset(ucTableData, 0x00, ulDataSize);

    for(int h = 0 ; h < uiHeight ; h++)
    {
        for(int w = 0 ; w < uiWidth ; w += 2)
        {
            uint8 pixel1 = (ucData[h * uiWidth + w + 0]) & 0x0F;
            uint8 pixel2 = (ucData[h * uiWidth + w + 1]) & 0x0F;

            ucTableData[h * uiWidth / 2 + w / 2] = (pixel2 << 4) + pixel1;  //1個byte包含2個pixel的data
        }
    }

    dvC341Geo_WriteToBuffer(B148_WPEBIASSADCH1, DEF_WPEBIASSAD0,   eC341_CH1);
    dvC341Geo_WriteToBuffer(B148_WPEBIASMWICH1, DEF_EBIASMWI&0xff, eC341_CH1);
    dvC341Geo_WriteToBuffer(B148_WPEBIASSADCH1, DEF_WPEBIASSAD1,   eC341_CH2);
    dvC341Geo_WriteToBuffer(B148_WPEBIASMWICH1, DEF_EBIASMWI&0xff, eC341_CH2);

    UINT32 ulLineWrite = 0 ;
    dvC341Geo_WriteToBuffer(B4_OSDCT, 0x00, 0);

    //H2PF_Simon_0095
    #ifdef FAST_BKLV_TABLE_SET   //約3.5秒     //先把CH1寫完，再寫CH2
    {
        #define EACH_BURST_WRITE_SIZE 16384 //H2PF_Simon_0115  //因需要整除改為16384 (一次寫16row，V為2400或1200時都可以整除，下面迴圈暫不做最後無法整除的處理) //用 61440 3.4秒  //用 32768 3.6秒 //用 16384 3.8秒
        UINT16 DataSize_1CH = m_sHalWarpingInfo.uiWarp_HW_1CH / 2 ;  //960
        UINT16 EachWriteSize = EACH_BURST_WRITE_SIZE;
        UINT16 EachWriteLineSize = (DEF_EBIASMWI * EBIAS_LINEFEED_BASE);  //1024
        UINT16 EachWriteLineNumber = EachWriteSize / EachWriteLineSize;  //16
        UINT8 EachData[EACH_BURST_WRITE_SIZE] = {0};

        //CH1
        for(UINT32 y = 0 ; y < uiHeight/EachWriteLineNumber ; y++ )  //2400/16 == 150  (一次寫 16 row，共寫 150 次)
        {
            dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEBIASSAD0 + y*EachWriteSize , 0);

            for(UINT16 LineIndex = 0 ; LineIndex < EachWriteLineNumber ; LineIndex++)  //一次寫 16 row
            {
                //EachData 1row 為 1024
                //ucTableData 1row 為 960
                memcpy(EachData + (EachWriteLineSize*LineIndex), ucTableData+((uiWidth/2)*(EachWriteLineNumber*y + LineIndex)), DataSize_1CH);
            }

            dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, sizeof(EachData), EachData);

        }

        //CH2
        for(UINT32 y = 0 ; y < uiHeight/EachWriteLineNumber ; y++ )  //2400/16 == 150  (一次寫 16 row，共寫 150 次)
        {
            dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEBIASSAD1 + y*EachWriteSize , 0);

            for(UINT16 LineIndex = 0 ; LineIndex < EachWriteLineNumber ; LineIndex++)  //一次寫 16 row
            {
                memcpy(EachData + (EachWriteLineSize*LineIndex), ucTableData+(DataSize_1CH + (uiWidth/2)*(EachWriteLineNumber*y + LineIndex)), DataSize_1CH);
            }

            dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, sizeof(EachData), EachData);

        }
    }
    #elif defined(FAST_BKLV_TABLE_SET_2) //約12秒   //先把CH1寫完，再寫CH2
    {
        UINT16 DataSize_1CH = m_sHalWarpingInfo.uiWarp_HW_1CH / 2 ;  //960

        //CH1
        dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEBIASSAD0, 0);
        for(UINT32 ulLineWrite = 0 ; ulLineWrite < uiHeight; ulLineWrite++)
        {
            for(UINT16 PosX = 0 ; PosX < (DEF_EBIASMWI * EBIAS_LINEFEED_BASE) ; PosX++)
            {
                if(PosX < DataSize_1CH)
                    dvC341Geo_WriteToBuffer(B4_CPUDT, *(ucTableData + (ulLineWrite*uiWidth/2 + PosX)), 0);
                else
                    dvC341Geo_WriteToBuffer(B4_CPUDT, 0, 0);
            }
        }


        //CH2
        dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEBIASSAD1, 0);
        for(UINT32 ulLineWrite = 0 ; ulLineWrite < uiHeight; ulLineWrite++)
        {
            for(UINT16 PosX = 0 ; PosX < (DEF_EBIASMWI * EBIAS_LINEFEED_BASE) ; PosX++)
            {
                if(PosX < DataSize_1CH)
                    dvC341Geo_WriteToBuffer(B4_CPUDT, *(ucTableData + (DataSize_1CH + ulLineWrite*uiWidth/2 + PosX)), 0);
                else
                    dvC341Geo_WriteToBuffer(B4_CPUDT, 0, 0);
            }
        }

    }
    #else  //約12秒    //normal 整個畫面1row寫完再寫下1row , 所以寫入時會 CH1->CH2->CH1->CH2.... 須一直重新設定 CPUWAD 會較慢
    {
        UINT16 DataSize_1CH = m_sHalWarpingInfo.uiWarp_HW_1CH / 2 ;  //960

        //整個畫面1個row寫完再寫下1個row
        for(UINT32 index = 0 ; index < ulDataSize; index += uiWidth/2)
        {
            //CH1 : need write CPUWAD when index ==   0/1920/3840...
            //CH2 : need write CPUWAD when index == 960/2880/5760...  (+ DataSize_1CH)

            //for CH1
            dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEBIASSAD0 + (DEF_EBIASMWI * EBIAS_LINEFEED_BASE * ulLineWrite), 0);
            dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, DataSize_1CH, ucTableData+index);

            //for CH2
            dvC341Geo_WriteToBuffer(B4_CPUWAD, DEF_WPEBIASSAD1 + (DEF_EBIASMWI * EBIAS_LINEFEED_BASE * ulLineWrite), 0);
            dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, DataSize_1CH, ucTableData+index+DataSize_1CH);

            ulLineWrite++;
        }
    }
    #endif


    dvC341Geo_Buffer_Flush();

    //dvC341Geo_Write( B4_OSDCT, 0x00 );

    free(ucTableData);

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Palette_Set(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    FLOAT ebgmLevel[16] = { 0, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024 };
    int lv, ad;
    int rtct_org = 0;  //= 0x0105; C341 wait review
    int GV_RTCT = 0x0000;

    m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][0] = ucRed;
    m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][1] = ucGreen;
    m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][2] = ucBlue;

    m_sHalWarpingInfo.aucBL_GammaCoef = 2.2;  //R70G2_Simon_0001

    for (UINT8 i=0; i<16; i++)
    {
        for (UINT8 col=0; col<3; col++)
        {
            FLOAT bias = (FLOAT)m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][col];
            for (UINT8 lv = 0; lv < 16; lv++ )
            {
                FLOAT gm = pow( (pow( ebgmLevel[lv], m_sHalWarpingInfo.aucBL_GammaCoef ) + pow( bias, m_sHalWarpingInfo.aucBL_GammaCoef )), 1/m_sHalWarpingInfo.aucBL_GammaCoef ) - ebgmLevel[lv];
                m_sHalWarpingInfo.aucBL_GammaTable[ucPaletteIdx][col][lv] = ( gm < 0 ) ? 0 : ( gm > 255 ) ? 255 : (int)gm;
            }
        }
    }

    INT32 iRegEGBIASCT  = dvC341Geo_Read(B147_WPEGBIASCTCH1, 0); //A35G2_Simon_0095
    LOG_MSG(db_HAL_WARPING, "%s %d %d %d %d (0x%X)\n", __FUNCTION__, ucPaletteIdx, ucRed, ucGreen, ucBlue, iRegEGBIASCT);

    dvC341Geo_Write(B0_RTCT6, GV_RTCT, 0);  //C341 wait reivew

    for(eC341_CH eCH = eC341_CH1 ; eCH<OUTPUT_CH_NUM ; eCH++)
    {
        for (int rgb=0; rgb<3; rgb++)
        {
            dvC341Geo_WriteToBuffer( B147_WPEGBIASCTCH1, ((iRegEGBIASCT|0x02) & 0xFFF3) | (rgb<<2), eCH);
            for (int sel_plt = 0; sel_plt < 16; sel_plt++)
            {
                ad = ((sel_plt < 16) && (sel_plt >= 0)) ? (sel_plt << 4) : 0;
                dvC341Geo_WriteToBuffer( B147_WPEGBIASADCH1, ad & 0xff, eCH);

                for ( lv = 0; lv < 16; lv++ )
                {
                    dvC341Geo_WriteToBuffer( B147_WPEGBIASDTCH1, m_sHalWarpingInfo.aucBL_GammaTable[sel_plt][rgb][lv] & 0xff, eCH);
                }
            }
        }
    }

    iRegEGBIASCT &= ~(0x02) ;
    dvC341Geo_WriteToBuffer( B147_WPEGBIASCTCH1, iRegEGBIASCT, eC341_CH1);
    dvC341Geo_WriteToBuffer( B147_WPEGBIASCTCH1, iRegEGBIASCT, eC341_CH2);

    dvC341Geo_Buffer_Flush();

    dvC341Geo_Write(B0_RTCT6, rtct_org, 0);  //A35G2_Simon_0093   //C341 wait review

    eResult = eHAL_WARPING_EXEC_CODE_PASS;
    //eResult |= halWarping_BlackLevel_Enable(TRUE);  //A35G2_Simon_0093

    return eResult;
}

//HICC2 no need
#if 0
//R70G2_Simon_0003
eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Palette_SaveFile(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //save blacklevel palette to file
        FILE *fp_palette = fopen(BLENDING_AP_BKLEVEL_PALETTE_CURRENT_FILENAME, "wb");
        if(fp_palette != NULL)
        {
            fwrite(&m_sHalWarpingInfo.aucBL_Palette , sizeof(m_sHalWarpingInfo.aucBL_Palette) , 1 , fp_palette);
            fflush(fp_palette);
            fclose(fp_palette);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif

//A65_OPTOMA_CL_0005
eHAL_WARPING_EXEC_CODE halWarpOSD_AP_Color_Initial( UINT8 ucInitPalette )
{
    halWarpOSD_Reload_AP_Config(eWOR_Blink);
    halWarpOSD_Reload_AP_Config(eWOR_Transparent);

    //init 256 color palettes for blender
    if(ucInitPalette)
    {
        halWarpOSD_Reload_AP_Config(eWOR_Palette);
    }

    dvC341_Buffer_Flush();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//A65_OPTOMA_CL_0006
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Group_Set(eCOLOR_PALETTE_GROUP eColorPaletteGroup)
{
    utilWarp_SetColorPaletteGroupIndex(eColorPaletteGroup);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Set(INT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    sPaletteSingleInfo sPlt = {uiIndex, ucRed, ucGreen, ucBlue};

    halWarpOSD_Config(eGCI_AP, eWOC_Palette_Single, &sPlt, eWRTIE_TO_CHIP);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_SetAll(UINT8 *aucRed, UINT8 *aucGreen, UINT8 *aucBlue)
{
    UINT8 Palette[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT];

    for(UINT16 Index = 0 ; Index < WARP_OSD_TOTAL_PALETTE ; Index++)
    {
        Palette[Index][0] = *(aucRed   + Index);
        Palette[Index][1] = *(aucGreen + Index);
        Palette[Index][2] = *(aucBlue  + Index);
    }

    halWarpOSD_Config(eGCI_AP, eWOC_Palette_All, Palette, eWRTIE_TO_CHIP);

    return eHAL_WARPING_EXEC_CODE_PASS;
}


void halWarping_OSDPalette_32bitsDataTo256x3Data(UINT32 *ulSrcPalette, UINT8 aucDestPalette[256][WARP_OSD_PALETTE_ELEMENT])
{
    UINT32 red = 0, green = 0, blue = 0;

    for (int i=0 ; i<256 ; i++)
    {
        red   = ulSrcPalette[i] & 0xFF0000;
        red   = red >> 16;
        green = ulSrcPalette[i] & 0x00FF00;
        green = green >> 8;
        blue  = ulSrcPalette[i] & 0x0000FF;

        aucDestPalette[i][0] = red;
        aucDestPalette[i][1] = green;
        aucDestPalette[i][2] = blue;
    }
}


//Palette : Array[256][3]
void halWarpOSD_AP_PLT_Set(UINT8 StartIdx, UINT8 Number, eWARP_OSD_SET_ACTION eAction)
{
    if(eAction == eWRTIE_TO_BUFFER)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette, StartIdx, Number, FALSE);
    }
    else if(eAction == eWRTIE_TO_CHIP)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette, StartIdx, Number, TRUE);
    }
}

void halWarpOSD_AP_Transparent_Set(void)
{
    halWarping_OSDWriteTransparentColor(m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColorEnable,
                                        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0],
                                        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[1],
                                        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[2],
                                        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[3]);
}


void halWarpOSD_AP_Blink_Set(eWARP_OSD_SET_ACTION eAction)
{
    if(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkEnable)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkColorSet0,  0xF0, WARP_OSD_BLINK_COLOR_MAX, (eAction == eWRTIE_TO_CHIP) ? (TRUE) : (FALSE));
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkColorSet1, 0x100, WARP_OSD_BLINK_COLOR_MAX, (eAction == eWRTIE_TO_CHIP) ? (TRUE) : (FALSE));
    }

    halWarping_OSDWriteBlink(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkEnable,
                             m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkTime,
                             m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkCycle,
                             (eAction == eWRTIE_TO_CHIP) ? (TRUE) : (FALSE)
                             );


}

//Palette : Array[256][3]
void halWarpOSD_OSD_PLT_Set(UINT8 *Palette, UINT16 StartIdx, UINT16 Number, eWARP_OSD_SET_ACTION eAction)
{
    memcpy(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette+(StartIdx*WARP_OSD_PALETTE_ELEMENT), Palette, (Number*WARP_OSD_PALETTE_ELEMENT));

    if(eAction == eWRTIE_TO_BUFFER)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette+(StartIdx*WARP_OSD_PALETTE_ELEMENT), StartIdx, Number, FALSE);
    }
    else if(eAction == eWRTIE_TO_CHIP)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette+(StartIdx*WARP_OSD_PALETTE_ELEMENT), StartIdx, Number, TRUE);
    }

}


void halWarpOSD_OSD_Transparent_Set(BOOL bEnable, UINT8 PLTIndex0, UINT8 PLTIndex1, UINT8 PLTIndex2, UINT8 PLTIndex3)
{
    m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColorEnable = bEnable;
    m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[0] = PLTIndex0;
    m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[1] = PLTIndex1;
    m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[2] = PLTIndex2;
    m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[3] = PLTIndex3;

    halWarping_OSDWriteTransparentColor(m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColorEnable,
                                        m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[0],
                                        m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[1],
                                        m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[2],
                                        m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[3]);
}

void halWarpOSD_OSD_Blink_Set(BOOL bEnable, UINT8 ucTime, UINT8 ucCycle, UINT8 BlinkSet0[WARP_OSD_BLINK_COLOR_MAX][WARP_OSD_PALETTE_ELEMENT], UINT8 BlinkSet1[WARP_OSD_BLINK_COLOR_MAX][WARP_OSD_PALETTE_ELEMENT], eWARP_OSD_SET_ACTION eAction)
{
    m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkEnable = bEnable;

    if(bEnable)
    {
        m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkTime = ucTime;
        m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkCycle = ucCycle;
        memcpy(m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkColorSet0, BlinkSet0, sizeof(WARP_OSD_BLINK_COLOR_MAX*WARP_OSD_PALETTE_ELEMENT));
        memcpy(m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkColorSet1, BlinkSet1, sizeof(WARP_OSD_BLINK_COLOR_MAX*WARP_OSD_PALETTE_ELEMENT));

        halWarping_OSDWritePalette(BlinkSet0,  0xF0, WARP_OSD_BLINK_COLOR_MAX, (eAction == eWRTIE_TO_CHIP) ? (TRUE) : (FALSE));
        halWarping_OSDWritePalette(BlinkSet1, 0x100, WARP_OSD_BLINK_COLOR_MAX, (eAction == eWRTIE_TO_CHIP) ? (TRUE) : (FALSE));
    }

    halWarping_OSDWriteBlink(m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkEnable,
                             m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkTime,
                             m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkCycle,
                             (eAction == eWRTIE_TO_CHIP) ? (TRUE) : (FALSE)
                             );


}

eHAL_WARPING_EXEC_CODE halWarpOSD_Reload_AP_Config(eWARPOSD_RELOAD_TYPE eType)
{
    if(eType == eWOR_Palette || eType == eWOR_ALL)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette,
                                   0,
                                   WARP_OSD_TOTAL_PALETTE,
                                   FALSE);
    }

    if(eType == eWOR_Blink || eType == eWOR_ALL)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkColorSet0,  0xF0, WARP_OSD_BLINK_COLOR_MAX, FALSE);
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkColorSet1, 0x100, WARP_OSD_BLINK_COLOR_MAX, FALSE);

        halWarping_OSDWriteBlink(m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkEnable,
                                 m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkTime,
                                 m_sHalWarpingInfo.sAp_WarpOSD_Info.ucBlinkCycle,
                                 FALSE);
    }

    if(eType == eWOR_Transparent || eType == eWOR_ALL)
    {
        halWarping_OSDWriteTransparentColor(m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColorEnable,
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0],
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[1],
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[2],
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[3]);
    }

    dvC341_Buffer_Flush();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void halWarpOSD_Reload_OSD_Config(eWARPOSD_RELOAD_TYPE eType)
{
    if(eType == eWOR_Palette || eType == eWOR_ALL)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette,
                                   0,
                                   WARP_OSD_TOTAL_PALETTE,
                                   FALSE);
    }

    if(eType == eWOR_Blink || eType == eWOR_ALL)
    {
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkColorSet0,  0xF0, WARP_OSD_BLINK_COLOR_MAX, FALSE);
        halWarping_OSDWritePalette(m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkColorSet1, 0x100, WARP_OSD_BLINK_COLOR_MAX, FALSE);

        halWarping_OSDWriteBlink(m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkEnable,
                                 m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkTime,
                                 m_sHalWarpingInfo.sOSD_WarpOSD_Info.ucBlinkCycle,
                                 FALSE);
    }

    if(eType == eWOR_Transparent || eType == eWOR_ALL)
    {
        halWarping_OSDWriteTransparentColor(m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColorEnable,
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[0],
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[1],
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[2],
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[3]);
    }

    dvC341_Buffer_Flush();

}




//ucPalette : Array[256][3]
eHAL_WARPING_EXEC_CODE halWarping_OSDWritePalette(UINT8 Palette[][WARP_OSD_PALETTE_ELEMENT], UINT16 StartWritePLTidx, UINT16 Number, BOOL FlushToChip)
{
    UINT32 red = 0, green = 0, blue = 0;

    dvC341_RegAccessMode(eRA_MLTBNKO12);

    if(StartWritePLTidx > 255)
    {
	    dvC341Geo_WriteToBuffer(B9_PLTAD1CH1, 0xFF, 0);
        dvC341Geo_WriteToBuffer(B9_PLTDT1CH1, 0, 0);
        dvC341Geo_WriteToBuffer(B9_PLTDT1CH1, 0, 0);
        dvC341Geo_WriteToBuffer(B9_PLTDT1CH1, 0, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B9_PLTAD1CH1, StartWritePLTidx, 0);
    }

    for (int i = StartWritePLTidx ; i < StartWritePLTidx + Number; i++)
    {
        red   = Palette[i-StartWritePLTidx][0] ;
        green = Palette[i-StartWritePLTidx][1] ;
        blue  = Palette[i-StartWritePLTidx][2] ;

        LOG_MSG(db_HAL_WARPING, "halWarping_OSDWritePalette %d %d %d %d\n", i-StartWritePLTidx, red, green, blue);

        dvC341Geo_WriteToBuffer(B9_PLTDT1CH1, red, 0);
        dvC341Geo_WriteToBuffer(B9_PLTDT1CH1, green, 0);
        dvC341Geo_WriteToBuffer(B9_PLTDT1CH1, blue, 0);
    }

    dvC341Geo_WriteToBuffer(B9_PLTAD1CH1, 0x00, 0);  //dummy

    if(FlushToChip)
    {
        dvC341Geo_Buffer_Flush();
    }

    dvC341_RegAccessMode(eRA_NORMAL);

    return eHAL_WARPING_EXEC_CODE_PASS;

}

//write TransparentColor index only when (bEnable == 1)
eHAL_WARPING_EXEC_CODE halWarping_OSDWriteTransparentColor(BOOL bEnable, UINT8 PLTIndex0, UINT8 PLTIndex1, UINT8 PLTIndex2, UINT8 PLTIndex3)
{
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) Enable %d, Transparent Color[%d %d %d %d]\r\n",__FUNCTION__
                                                                                              ,__LINE__
                                                                                              , bEnable
                                                                                              , PLTIndex0
                                                                                              , PLTIndex1
                                                                                              , PLTIndex2
                                                                                              , PLTIndex3);


    UINT16 ucOSDMODE = (UINT16)dvC341Geo_Read(B9_OSDMODE1CH1, 0);
    ucOSDMODE &= ~(BIT0);

    dvC341_RegAccessMode(eRA_MLTBNKO12);

    if(bEnable)
    {
        ucOSDMODE |= (BIT0);
        dvC341Geo_WriteToBuffer(B9_BOTRANS01CH1, PLTIndex0, 0);
        dvC341Geo_WriteToBuffer(B9_BOTRANS11CH1, PLTIndex1, 0);
        dvC341Geo_WriteToBuffer(B9_BOTRANS21CH1, PLTIndex2, 0);
        dvC341Geo_WriteToBuffer(B9_BOTRANS31CH1, PLTIndex3, 0);
    }

    dvC341Geo_WriteToBuffer(B9_OSDMODE1CH1, (UINT32)ucOSDMODE, 0);
    dvC341Geo_Buffer_Flush();

    dvC341_RegAccessMode(eRA_NORMAL);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_OSD_TransparentColor_Get(eTRANSPARENT_COLOR_SLOT eSlotIndex, UINT32 *ulColor)
{
    switch(eSlotIndex)
    {
        case eTC_SLOT0:
            *ulColor = dvC341_Read(B9_BOTRANS01CH1, 0);
            break;

        case eTC_SLOT1:
            *ulColor = dvC341_Read(B9_BOTRANS11CH1, 0);
            break;

        case eTC_SLOT2:
            *ulColor = dvC341_Read(B9_BOTRANS21CH1, 0);
            break;

        case eTC_SLOT3:
            *ulColor = dvC341_Read(B9_BOTRANS31CH1, 0);
            break;

        default:
            *ulColor = 0;
            return eHAL_WARPING_EXEC_CODE_FAIL;

    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//write Blink time/cycle only when (bEnable == 1)
eHAL_WARPING_EXEC_CODE halWarping_OSDWriteBlink(BOOL bEnable, UINT8 ucTime, UINT8 ucCycle, BOOL FlushToChip)
{
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) Enable %d, Blink[%d %d]\r\n",__FUNCTION__
                                                                            ,__LINE__
                                                                            , bEnable
                                                                            , ucTime
                                                                            , ucCycle);

    dvC341_RegAccessMode(eRA_MLTBNKO12);

    if(bEnable)
    {
        dvC341Geo_WriteToBuffer(B9_BLINKTIME1CH1, ucTime, 0);
        dvC341Geo_WriteToBuffer(B9_BLINKCYCL1CH1, ucCycle, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B9_BLINKTIME1CH1, 0, 0);
        dvC341Geo_WriteToBuffer(B9_BLINKCYCL1CH1, 0, 0);
    }

    if(FlushToChip)
    {
        dvC341Geo_Buffer_Flush();
    }

    dvC341_RegAccessMode(eRA_NORMAL);

    return eHAL_WARPING_EXEC_CODE_PASS;
}





eHAL_WARPING_EXEC_CODE halWarpOSD_ClearOSD(void)
{
    int GV_RTCT = 0;  //= 0x0105;  //C341 wait review
    int lRetry = 1000;
    UINT16 uiWidth = DEF_OSD_PANEL_WIDTH;
    UINT8 ucFastWrite = 0;
    INT32 lTransparentColor = dvC341Geo_Read(B9_BOTRANS01CH1, 0);
    INT32 lOSD_MODE_CH1 = dvC341_Read(B9_OSDMODE1CH1, 0); //A35G2_CDS_Simon_0044
    INT32 lOSD_MODE_CH2 = dvC341_Read(B9_OSDMODE1CH1, CH_BANK_OFFSET); //A35G2_CDS_Simon_0044

    LOG_MSG(db_HAL_WARPING, "%s Start %d %d %d (OSDMODE 0x%04X 0x%04X)\n", __FUNCTION__ , m_sHalWarpingInfo.uiWarp_HW , m_sHalWarpingInfo.uiWarp_VW, lTransparentColor, lOSD_MODE_CH1, lOSD_MODE_CH2);

    dvC341_Write( B9_OSDMODE1CH1, 0x00, 0);  //0x03:0x07
    dvC341_Write( B9_OSDMODE1CH1, 0x00, CH_BANK_OFFSET);  //0x03:0x07

#ifdef OSDCT_FASTEN_ON
    if (uiWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        uiWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        uiWidth = uiWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
#endif

    //GV_RTCT = GV_RTCT | 0x0007;  //C341 wait review
    dvC341Geo_Write( B4_BBWMWI, (DEF_OSD_PANEL_WIDTH/OSD_LINEFEED_BASE)&0xff, 0); //Cindy 20180801-223959
    dvC341Geo_Write( B0_RTCT6, GV_RTCT, 0);  //C341 wait review
    dvC341Geo_Write( B4_OSDFILL, lTransparentColor, 0);
    dvC341Geo_Write( B4_BBACTHW, (uiWidth - 1)&0xfff, 0);
    dvC341Geo_Write( B4_BBACTVW, (DEF_OSD_PANEL_HEIGHT - 1)&0xfff, 0);
    dvC341Geo_Write( B4_BBWMWI, (DEF_OSD_PANEL_WIDTH/OSD_LINEFEED_BASE), 0);
    dvC341Geo_Write( B4_CPUWAD, m_OSD_MEM_LAYER0 & CPUWAD_MASK, 0);    //A65_OPTOMA_Simon_0001
    if(ucFastWrite)
    {
        dvC341Geo_Write( B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_Write( B4_OSDCT, 0x01, 0);
    }
    //GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;  //C341 wait review
    dvC341Geo_Write( B0_RTCT6, GV_RTCT, 0);  //C341 wait review

    while((dvC341Geo_Read(B4_BOSTAT, 0)&0x01) == 0x01)    //G100_Simon_0007
    {
        if(lRetry <= 0)
        {
            LOG_MSG(db_HAL_WARPING, "%s B4_BOSTAT timeout\n", __FUNCTION__);
            break;
        }

        lRetry--;
        MS_SLEEP(1);
    }

    dvC341Geo_Write(B4_OSDCT, 0x00, 0);

    dvC341Geo_Write(B4_BBACTHW, (uiWidth - 1)&0xfff, 0);
    dvC341Geo_Write(B4_BBACTVW, (DEF_OSD_PANEL_HEIGHT - 1)&0xfff, 0);
    dvC341Geo_Write(B4_OSDFILL, lTransparentColor, 0);
    dvC341Geo_Write(B4_CPUWAD, m_OSD_MEM_LAYER1 & CPUWAD_MASK, 0);
    if(ucFastWrite)
    {
        dvC341Geo_Write( B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_Write( B4_OSDCT, 0x01, 0);
    }

    lRetry = 1000;
    while( (dvC341Geo_Read(B4_BOSTAT, 0)&0x01) == 0x01 )    //R70G2_Simon_0007
    {
        if(lRetry <= 0)
        {
            LOG_MSG(db_HAL_WARPING, "%s B4_BOSTAT timeout\n", __FUNCTION__);
            break;
        }

        lRetry--;
        MS_SLEEP(1);
    }
    dvC341Geo_Write(B4_OSDCT, 0x00, 0);

    //GV_RTCT = GV_RTCT | 0x0007;   //C341 wait review
    dvC341Geo_Write(B0_RTCT6, GV_RTCT, 0);
    dvC341Geo_Write(B4_OSDCT, 0x00, 0);
    dvC341_Write(B9_OSDMODE1CH1, lOSD_MODE_CH1, 0);  //0x03:0x07 //A35G2_CDS_Simon_0044
    dvC341_Write(B9_OSDMODE1CH1, lOSD_MODE_CH2, CH_BANK_OFFSET);

    //GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;   //C341 wait review
    dvC341Geo_Write(B0_RTCT6, GV_RTCT, 0);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_AP_ClearMemoryOSD(void)
{
	if(halWarping_GetFreezeImageState() == TRUE)
	{
		halWarping_FreezeImage(FALSE);
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_AP_ClearOSD(eWARPOSD_DRAW_TYPE eType)  //R70G2_Simon_0001
{
    //halWarpOSD_Palette_Group_Set(COLOR_PALETTE__AP);  //A65_OPTOMA_CL_0010

    LOG_MSG(db_HAL_WARPING, "%s (%d)\n", __FUNCTION__ , eType);

    if(eType == eWDT_BEFORE_WARP)
    {
        halWarpOSD_AP_ClearMemoryOSD();
    }
    else if(eType == eWDT_AFTER_WARP)
    {
        UINT32 ulTransparentColor = dvC341Geo_Read(B9_BOTRANS01CH1, 0);

        #if 0  //remove
        if( utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)//A65_OPTOMA_CL_0010
        {
            if(utilWarp_GetOsdPatternType() == PAT_TYPE__TESTPATTERN_BLACK_BORDER_MULTIPLESELECT ||
               utilWarp_GetOsdPatternType() == PAT_TYPE__DRAW_CIRCLE_GRID)
            {
                halAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__OFF);
            }
            else
            {
                utilWarp_ShowOsdPattern(PAT_TYPE__OFF);
            }
        }
        #endif

        halWarping_AP_DrawRect(eWDT_AFTER_WARP,  0, 0, m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW, ulTransparentColor);
        halWarping_OSD_ON();    //R70G2_Simon_0002
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

#if 0
eHAL_WARPING_EXEC_CODE halWarpOSD_ClearOSD_NoSemaphore(void)
{
    int GV_RTCT = 0x0105;
    int lRetry = 1000;

    INT32 lTransparentColor = dvC341Geo_Read(B9_BOTRANS02CH1);
    INT32 lOSD_MODE = dvC341Geo_Read(B9_OSDMODE2CH1); //A35G2_CDS_Simon_0044

    dvC341Geo_Write( B9_OSDMODE2CH1, 0x00 );  //0x03:0x07

    LOG_MSG(db_HAL_WARPING, "%s Start %d %d %d\n", __FUNCTION__ , m_sHalWarpingInfo.uiWarp_HW , m_sHalWarpingInfo.uiWarp_VW, lTransparentColor);
    GV_RTCT = GV_RTCT | 0x0007;
    dvC341Geo_Write( B4_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW_1CH / OSD_LINEFEED_BASE)&0xff ); //Cindy 20180801-223959
    dvC341Geo_Write( BN_RTCT, GV_RTCT );
    dvC341Geo_Write( B4_OSDFILL, lTransparentColor );
    dvC341Geo_Write( B4_BBACTHW, (m_sHalWarpingInfo.uiWarp_HW - 1)&0xfff );
    dvC341Geo_Write( B4_BBACTVW, (m_sHalWarpingInfo.uiWarp_VW - 1)&0xfff );
    dvC341Geo_Write( B4_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW_1CH / OSD_LINEFEED_BASE) );
    dvC341Geo_Write( B4_CPUWAD, m_OSD_MEM_LAYER0 & CPUWAD_MASK );    //A65_OPTOMA_Simon_0001
    dvC341Geo_Write( B4_OSDCT, 0x01 );
    GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
    dvC341Geo_Write( BN_RTCT, GV_RTCT );

    while( (dvC341Geo_Read(B4_BOSTAT)&0x01) == 0x01 )
    {
        if(lRetry <= 0)
        {
            LOG_MSG(db_HAL_WARPING, "%s B4_BOSTAT timeout\n", __FUNCTION__);
            break;
        }

        lRetry--;
        MS_SLEEP(1);
    }

    dvC341Geo_Write( B4_OSDCT, 0x00 );


    dvC341Geo_Write( B4_CPUWAD, m_OSD_MEM_LAYER1 & CPUWAD_MASK );    //R70G2_Simon_0007
    dvC341Geo_Write( B4_OSDCT, 0x01 );

    lRetry = 1000;
    while( (dvC341Geo_Read(B4_BOSTAT)&0x01) == 0x01 )
    {
        if(lRetry <= 0)
        {
            LOG_MSG(db_HAL_WARPING, "%s B4_BOSTAT timeout\n", __FUNCTION__);
            break;
        }

        lRetry--;
        MS_SLEEP(1);
    }
    dvC341Geo_Write( B4_OSDCT, 0x00 );


    GV_RTCT = GV_RTCT | 0x0007;
    dvC341Geo_Write( BN_RTCT, GV_RTCT );
    dvC341Geo_Write( B4_OSDCT, 0x00 );
    dvC341Geo_Write( B9_OSDMODE2CH1, lOSD_MODE );  //0x03:0x07 //A35G2_CDS_Simon_0044
    GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
    dvC341Geo_Write( BN_RTCT, GV_RTCT );

    return eHAL_WARPING_EXEC_CODE_PASS;

}
#endif


eHAL_WARPING_EXEC_CODE halWarpOSD_DisableOSD(void)
{
    UINT32 GV_RTCT = dvC341_Read(B0_RTCT0, 0);
    UINT16 OSDMODE = 0;
    dvC341_Write(B0_RTCT0, 0x00, 0);
    OSDMODE = dvC341Geo_Read( B9_OSDMODE1CH1, 0);
    OSDMODE &= 0xFFFD;
    dvC341_Write(B9_OSDMODE1CH1, OSDMODE, 0);
    dvC341_Write(B9_OSDMODE1CH1, OSDMODE, CH_BANK_OFFSET);
    dvC341_Write(B0_RTCT0, GV_RTCT, 0);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarp_BlacklevelParameterSet(eBLACKLEVEL_PARA eParameter, INT32 lData)
{
    switch(eParameter)
    {
        case eBKLV_EVENT_AREA_SEL:
            ucBlacklevelAreaSel = lData;
            m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_SEL = lData;
            break;

        //coordinate
        case eBKLV_EVENT_CUR_TL_X:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_X = lData;
            break;

        case eBKLV_EVENT_CUR_TL_Y:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_Y = lData;
            break;

        case eBKLV_EVENT_CUR_TR_X:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_X = lData;
            break;

        case eBKLV_EVENT_CUR_TR_Y:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_Y = lData;
            break;

        case eBKLV_EVENT_CUR_BL_X:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_X = lData;
            break;

        case eBKLV_EVENT_CUR_BL_Y:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_Y = lData;
            break;

        case eBKLV_EVENT_CUR_BR_X:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_X = lData;
            break;

        case eBKLV_EVENT_CUR_BR_Y:
            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_Y = lData;
            break;


        //palette R,G,B
        case eBKLV_EVENT_PLT_RED:
            if(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_R == (UINT8)lData)
            {
                break;
            }

            m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_R = (UINT8)lData;

            if(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_ACTIVE == TRUE)  //A35G2_Simon_0106
            {
                halWarping_Blacklevel_Palette_Set(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_SEL,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_R,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_G,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_B);

                halWarping_Blacklevel_SaveData(BLACKLEVEL_CURRENT_INDEX);  //A35G2_Simon_0106
            }
            break;

        case eBKLV_EVENT_PLT_GREEN:
            if(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_G == (UINT8)lData)
            {
                break;
            }

            m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_G = (UINT8)lData;

            if(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_ACTIVE == TRUE)  //A35G2_Simon_0106
            {

                halWarping_Blacklevel_Palette_Set(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_SEL,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_R,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_G,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_B);

                halWarping_Blacklevel_SaveData(BLACKLEVEL_CURRENT_INDEX);  //A35G2_Simon_0106
            }
            break;

        case eBKLV_EVENT_PLT_BLUE:
            if(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_B == (UINT8)lData)
            {
                break;
            }

            m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_B = (UINT8)lData;

            if(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_ACTIVE == TRUE)  //A35G2_Simon_0106
            {
                halWarping_Blacklevel_Palette_Set(m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_SEL,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_R,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_G,
                                                  m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_B);

                halWarping_Blacklevel_SaveData(BLACKLEVEL_CURRENT_INDEX);  //A35G2_Simon_0106
            }
            break;


        case eBKLV_EVENT_AREA_ENABLE:
            m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_AreaEnable = (UINT8)lData;
            break;

        case eBKLV_EVENT_APPLY:
            halWarping_Blacklevel_AreaApply(ucBlacklevelAreaSel,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_Y,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_Y,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_Y,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_Y);
            break;


        case eBKLV_EVENT_APPLY_ALL_AREA:
            halWarping_Blacklevel_ApplyAllArea();
            halWarping_Blacklevel_SaveData(BLACKLEVEL_CURRENT_INDEX);  //A35G2_Simon_0093
            break;

        case eBKLV_EVENT_ENABLE:
            halWarping_BlackLevel_Enable(lData);
            break;

        case eBKLV_EVENT_RESET:
            halWarping_Blacklevel_Reset();
            break;

        default:
            LOG_MSG(db_HAL_WARPING, "%s para set error\n\n", __FUNCTION__);
            break;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}


INT32 halWarp_BlacklevelParameterGet(eBLACKLEVEL_PARA eParameter)
{
    switch(eParameter)
    {
        case eBKLV_EVENT_AREA_SEL:
            return ucBlacklevelAreaSel;

        //coordinate
        case eBKLV_EVENT_CUR_TL_X:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_X;

        case eBKLV_EVENT_CUR_TL_Y:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_Y;

        case eBKLV_EVENT_CUR_TR_X:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_X;

        case eBKLV_EVENT_CUR_TR_Y:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_Y;

        case eBKLV_EVENT_CUR_BL_X:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_X;

        case eBKLV_EVENT_CUR_BL_Y:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_Y;

        case eBKLV_EVENT_CUR_BR_X:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_X;

        case eBKLV_EVENT_CUR_BR_Y:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_Y;


        //palette R,G,B
        case eBKLV_EVENT_PLT_RED:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_R;

        case eBKLV_EVENT_PLT_GREEN:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_G;

        case eBKLV_EVENT_PLT_BLUE:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_B;


        case eBKLV_EVENT_AREA_ENABLE:
            return m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_AreaEnable;

        case eBKLV_EVENT_APPLY:
            return FALSE;

        case eBKLV_EVENT_APPLY_ALL_AREA:
            return FALSE;

        case eBKLV_EVENT_ENABLE:
            return FALSE;  //wait review

        default:
            LOG_MSG(db_HAL_WARPING, "%s para get error\n\n", __FUNCTION__);
            return 0;
    }

}


void halWarping_Blacklevel_Init(void)
{
    dvC341Geo_EdgebldBiasEnable(FALSE);

    memset(m_sBlackLevelPara, 0, sizeof(m_sBlackLevelPara));
}

eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_Reset(void)
{
    LOG_MSG(db_HAL_WARPING, "%s Start\n\n", __FUNCTION__);

    dvC341Geo_EdgebldBiasEnable(FALSE);

    #if (CMD_ONLY_BLACK_LEVEL == TRUE)  //A35G2_Simon_0093
    {
        memset(m_sBlackLevelPara, 0, sizeof(m_sBlackLevelPara));

        dvC341Geo_EdgebldBiasGammaTableClear();

        for(int i=0 ; i<15 ; i++)
        {
            dvC341Geo_Blacklevel_Palette_Set(i,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_B);
        }
        coord_t sCursor_TL = {0,                               0};
        coord_t sCursor_TR = {m_sHalWarpingInfo.uiWarp_HW - 1, 0};
        coord_t sCursor_BL = {0,                               m_sHalWarpingInfo.uiWarp_VW - 1};
        coord_t sCursor_BR = {m_sHalWarpingInfo.uiWarp_HW - 1, m_sHalWarpingInfo.uiWarp_VW - 1};
        dvC341Geo_BlackLevel_AreaWrite(0, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, TRUE);

        dvC341Geo_EdgebldBiasEnable(FALSE);

        if(access(BLACKLEVEL_TMP_FILE, R_OK) == 0)  //A35G2_Simon_0093
        {
            if(access("/mnt/configs/scaler/blending/blacklevel_tmp.dat", R_OK) == 0)
                SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel_tmp.dat");
        }
    }
    #endif

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void halWarping_Blacklevel_AreaApply(              INT8 cPaletteSelect,
                                                   INT16 iCursor_TL_X,
                                                   INT16 iCursor_TL_Y,
                                                   INT16 iCursor_TR_X,
                                                   INT16 iCursor_TR_Y,
                                                   INT16 iCursor_BL_X,
                                                   INT16 iCursor_BL_Y,
                                                   INT16 iCursor_BR_X,
                                                   INT16 iCursor_BR_Y)
{
    coord_t sCursor_TL = {iCursor_TL_X, iCursor_TL_Y};
    coord_t sCursor_TR = {iCursor_TR_X, iCursor_TR_Y};
    coord_t sCursor_BL = {iCursor_BL_X, iCursor_BL_Y};
    coord_t sCursor_BR = {iCursor_BR_X, iCursor_BR_Y};

    dvC341Geo_Blacklevel_Palette_Set(cPaletteSelect,
                                  m_sBlackLevelPara[cPaletteSelect].ucBlackLevel_PLT_R,
                                  m_sBlackLevelPara[cPaletteSelect].ucBlackLevel_PLT_G,
                                  m_sBlackLevelPara[cPaletteSelect].ucBlackLevel_PLT_B
                                  );

    dvC341Geo_EdgebldBiasEnable(TRUE);

    dvC341Geo_BlackLevel_AreaWrite(cPaletteSelect, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, TRUE);

}

void halWarping_Blacklevel_ApplyAllArea(void)
{
    //clear
    coord_t sCursor_TL = {0,                               0};
    coord_t sCursor_TR = {m_sHalWarpingInfo.uiWarp_HW - 1, 0};
    coord_t sCursor_BL = {0,                               m_sHalWarpingInfo.uiWarp_VW - 1};
    coord_t sCursor_BR = {m_sHalWarpingInfo.uiWarp_HW - 1, m_sHalWarpingInfo.uiWarp_VW - 1};
    //dvC341Geo_BlackLevel_AreaWrite(0, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, FALSE);

    //set palette
    for(int i=0 ; i<15 ; i++)
    {
        if(i > 0 && m_sBlackLevelPara[i].ucBlackLevel_AreaEnable == FALSE)  //area 0 don`t care "Enable"      //A35G2_Simon_0106
        {
            m_sBlackLevelPara[i].ucBlackLevel_ACTIVE = FALSE;
            continue;
        }

        dvC341Geo_Blacklevel_Palette_Set(m_sBlackLevelPara[i].ucBlackLevel_PLT_SEL,
                                         m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                         m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                         m_sBlackLevelPara[i].ucBlackLevel_PLT_B);
    }

    dvC341Geo_EdgebldBiasEnable(TRUE);

    for(int i=0 ; i<15 ; i++)  //G100_Simon_0005
    {
        if(i > 0 && m_sBlackLevelPara[i].ucBlackLevel_AreaEnable == FALSE)  //area 0 don`t care coordinate (always full screen)     //A35G2_Simon_0106
        {
            m_sBlackLevelPara[i].ucBlackLevel_ACTIVE = FALSE;
            continue;
        }

        coord_t sCursor_TL = {m_sBlackLevelPara[i].uiBlackLevel_CUR_TL_X, m_sBlackLevelPara[i].uiBlackLevel_CUR_TL_Y};
        coord_t sCursor_TR = {m_sBlackLevelPara[i].uiBlackLevel_CUR_TR_X, m_sBlackLevelPara[i].uiBlackLevel_CUR_TR_Y};
        coord_t sCursor_BL = {m_sBlackLevelPara[i].uiBlackLevel_CUR_BL_X, m_sBlackLevelPara[i].uiBlackLevel_CUR_BL_Y};
        coord_t sCursor_BR = {m_sBlackLevelPara[i].uiBlackLevel_CUR_BR_X, m_sBlackLevelPara[i].uiBlackLevel_CUR_BR_Y};

        //A35G2_Simon_0106
        if(i > 0 &&
           (
           m_sBlackLevelPara[i].uiBlackLevel_CUR_TL_X >= m_sBlackLevelPara[i].uiBlackLevel_CUR_TR_X ||
           m_sBlackLevelPara[i].uiBlackLevel_CUR_BL_X >= m_sBlackLevelPara[i].uiBlackLevel_CUR_BR_X ||
           m_sBlackLevelPara[i].uiBlackLevel_CUR_TL_Y >= m_sBlackLevelPara[i].uiBlackLevel_CUR_BL_Y ||
           m_sBlackLevelPara[i].uiBlackLevel_CUR_TR_Y >= m_sBlackLevelPara[i].uiBlackLevel_CUR_BR_Y
           )
        )
        {
            LOG_MSG(db_HAL_WARPING, "black level coordinate error <area %d>\n", i);
            m_sBlackLevelPara[i].ucBlackLevel_ACTIVE = FALSE;
            continue;
        }

        dvC341Geo_BlackLevel_AreaWrite(m_sBlackLevelPara[i].ucBlackLevel_PLT_SEL, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, FALSE);

        m_sBlackLevelPara[i].ucBlackLevel_ACTIVE = TRUE;  //area 0 always Active
    }

    dvC341Geo_EdgebldBiasEnable(TRUE);

    dvC341Geo_LoadEgbBiasArea(BLACKLEVEL_TMP_FILE /*"ebarea0.bak"*/);
}


void halWarping_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue)
{
    dvC341Geo_Blacklevel_Palette_Set(cPaletteIdx, cRed, cGreen, cBlue);

    dvC341Geo_EdgebldBiasEnable(TRUE);
}

eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_SaveData(UINT8 ucIndex)  //A35G2_Simon_0093
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    if(ucIndex > WARPING_APPLY_MAX_VALUE && ucIndex != BLACKLEVEL_CURRENT_INDEX)
    {
	    LOG_MSG(db_HAL_WARPING, "Error : Blacklevel SaveData %d\n", ucIndex);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    if(access(BLACKLEVEL_TMP_FILE, R_OK) != 0)
    {
	    LOG_MSG(db_HAL_WARPING, "Blacklevel no tmp file to save\n");
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    UINT8 ucOneLineBuf[m_sHalWarpingInfo.uiWarp_HW + 5];
    memset(ucOneLineBuf, 0, m_sHalWarpingInfo.uiWarp_HW + 5);

    LOG_MSG(db_HAL_WARPING, "Save Blacklevel Start\n");

    sBlackLevelFileData sFile;
    sFile.ucEnableFlag = TRUE;
    sFile.uc3DFlag = m_sHalWarpingInfo.uiWarp_Is120HzPanel;
    sFile.ucWidth  = m_sHalWarpingInfo.uiWarp_HW;
    sFile.ucHeight = m_sHalWarpingInfo.uiWarp_VW;

    #if 0  //for test  //write all 1920x1200 data
    FILE *fp = fopen(BLACKLEVEL_TMP_FILE, "r");
    if(fp == NULL)
    {
        LOG_MSG(db_HAL_WARPING, "No Blacklevel temp file\n");
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    for(int line=0 ; line<m_sHalWarpingInfo.uiWarp_VW ; line++)
    {
        if(fgets( ucOneLineBuf, sizeof(ucOneLineBuf), fp ) != NULL)
        {
            LOG_MSG(db_HAL_WARPING, "%s <%d>(%c)\n\n" , ucOneLineBuf, strlen(ucOneLineBuf)-1 , ucOneLineBuf[strlen(ucOneLineBuf) - 2]);
        }

        UINT16 ucLength = strlen(ucOneLineBuf) - 1;  // last char is '\n'

        if(ucLength < m_sHalWarpingInfo.uiWarp_HW && ucLength >= 1)
        {
            for(int i=ucLength ; i<m_sHalWarpingInfo.uiWarp_HW ; i++)
            {
                ucOneLineBuf[i] = ucOneLineBuf[ucLength - 1] ;
            }
        }

        for(int i=0 ; i<m_sHalWarpingInfo.uiWarp_HW ; i++)
        {
            sFile.aucTable[i + (line * m_sHalWarpingInfo.uiWarp_HW)] = ucOneLineBuf[i];
        }

    }

    fclose(fp);

    //for debug
    {
        FILE *fp_tmp;
        UINT32 ulSize = m_sHalWarpingInfo.uiWarp_HW * m_sHalWarpingInfo.uiWarp_VW;
        fp_tmp = fopen( "./BK_Table.dat" , "w" );
        fwrite(sFile.aucTable , 1 , ulSize , fp_tmp);
        fclose(fp_tmp);

        LOG_MSG(db_HAL_WARPING, "\nBK_Table Size = %d\n\n" , ulSize);
    }
    #endif

    #if 0   //only write to file
    //write to DMA
    {
    	dvC341Geo_Write(B4_CPUWAD, DEF_EBIASSAD);
    	dvC341Geo_Write(B4_CPURAD, DEF_EBIASSAD);

    	// erase Black Level Group
        LOG_MSG(db_HAL_WARPING, "Erase...\n");
    	UINT32 ulAddress = ucIndex * WARPING_MEMORY_GROUP_SIZE + WARPING_TWIST_BIAS_TABLE_ADDRESS;
    	dvC341Geo_FlashEraseSector(ulAddress, WARPING_TWIST_BIAS_TABLE_SIZE) ;
        LOG_MSG(db_HAL_WARPING, "Erase Done\n");

        dvC341Geo_DMA_Transfer(ulAddress, WARPING_TWIST_BIAS_TABLE_SIZE, eC789_SFLREGSEL_DDR, eC789_SFLCT_DMA_REG2FLASH);
        LOG_MSG(db_HAL_WARPING, "Write Flash Done\n");
    }
    #endif

    //save blacklevel palette
    memset(sFile.aucBiasPalette, 0, sizeof(sFile.aucBiasPalette));
    for(int i=0 ; i<MAX_BLACKLEVEL_AREA_NUMBER ; i++)
    {
        sFile.aucBiasPalette[0 + i*3] = m_sBlackLevelPara[i].ucBlackLevel_PLT_R;
        sFile.aucBiasPalette[1 + i*3] = m_sBlackLevelPara[i].ucBlackLevel_PLT_G;
        sFile.aucBiasPalette[2 + i*3] = m_sBlackLevelPara[i].ucBlackLevel_PLT_B;
    }

    for(int i=0 ; i<sizeof(sFile.aucBiasPalette) ; i++)
    {
        if(i % 3 == 0)
        {
            LOG_MSG(db_HAL_WARPING, "\n");
        }

        LOG_MSG(db_HAL_WARPING, "<%d>", sFile.aucBiasPalette[i]);
    }

    //save BlackLevelPara
    memset(sFile.asBlackLevelPara, 0, sizeof(sFile.asBlackLevelPara));
    for(int i=0 ; i<MAX_BLACKLEVEL_AREA_NUMBER ; i++)
    {
        memcpy(&sFile.asBlackLevelPara[i], &m_sBlackLevelPara[i], sizeof(sFile.asBlackLevelPara[0]));
    }


    //save raw data to file
    char cRawDataFileName[64];
    snprintf(cRawDataFileName, 64, BLACKLEVEL_RAW_DATA_FILE, ucIndex);
    snprintf((char *)sFile.aucRawDataFileName, 64, "%s", cRawDataFileName);
    SYSTEM_CALL("cp %s %s", BLACKLEVEL_TMP_FILE, cRawDataFileName);

    //write to file
    char cWriteFileName[128];
    sprintf(cWriteFileName, BLACKLEVEL_FILE, ucIndex);

    FILE *fp_save = fopen(cWriteFileName, "wb");
    if(fp_save == NULL)
    {
        LOG_MSG(db_HAL_WARPING, "Can not open file %s\n", cWriteFileName);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }
    else
    {
        fwrite(&sFile , sizeof(sBlackLevelFileData) , 1 , fp_save);
        fflush(fp_save);
        fclose(fp_save);
    }

    LOG_MSG(db_HAL_WARPING, "Save Blacklevel End\n");

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_LoadData(UINT8 ucIndex)  //A35G2_Simon_0093
{
    if(utilWarp_GetWarpCtrl() != WARP_CTRL__ADVANCED)  //G100_Simon_0005
    {
	    LOG_MSG(db_HAL_WARPING, "Error : Blacklevel_LoadData (not Advanced)\n", utilWarp_GetWarpCtrl());
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    if(ucIndex > WARPING_APPLY_MAX_VALUE && ucIndex != BLACKLEVEL_CURRENT_INDEX)
    {
	    LOG_MSG(db_HAL_WARPING, "Error : Blacklevel_LoadData %d\n", ucIndex);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    halWarping_clearBlackLevel();


    LOG_MSG(db_HAL_WARPING, "Load Blacklevel %d Start \n", ucIndex);

    char cFileName[128];

    // load file
    UINT32 ulFileSize = sizeof(sBlackLevelFileData);
    UINT8 *pucData = (UINT8 *)malloc(ulFileSize);

	memset(pucData, 0, ulFileSize); //A35G2_Alan_0039

	if(pucData == NULL)
    {
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    sprintf(cFileName, BLACKLEVEL_FILE, ucIndex);
    FILE *fp_read = fopen(cFileName, "rb");
    if(fp_read == NULL)
    {
        free(pucData);
        LOG_MSG(db_HAL_WARPING, "Can not open file %s\n", cFileName);

        //remove current data
        halWarping_Blacklevel_ClearCurrentData();
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }
    else
    {
        size_t ret = fread(pucData, 1, ulFileSize, fp_read);
        fclose(fp_read);
    }

    sBlackLevelFileData *sBlackLevelInfo = (sBlackLevelFileData *)pucData;

    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag %d\n",               offsetof(sBlackLevelFileData, ucEnableFlag),    sBlackLevelInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",                   offsetof(sBlackLevelFileData, uc3DFlag),        sBlackLevelInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",                    offsetof(sBlackLevelFileData, ucWidth),         sBlackLevelInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",                   offsetof(sBlackLevelFileData, ucHeight),        sBlackLevelInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X BiasPalette %d %d %d\n",        offsetof(sBlackLevelFileData, aucBiasPalette),  sBlackLevelInfo->aucBiasPalette[0], sBlackLevelInfo->aucBiasPalette[1], sBlackLevelInfo->aucBiasPalette[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X BlackLevelPara %d %d %d\n",     offsetof(sBlackLevelFileData, asBlackLevelPara),sBlackLevelInfo->asBlackLevelPara[0], sBlackLevelInfo->asBlackLevelPara[1], sBlackLevelInfo->asBlackLevelPara[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X RawDataFileName %c%c\n",        offsetof(sBlackLevelFileData, aucRawDataFileName),sBlackLevelInfo->aucRawDataFileName[0],sBlackLevelInfo->aucRawDataFileName[1]);

    if(sBlackLevelInfo->uc3DFlag != m_sHalWarpingInfo.uiWarp_Is120HzPanel)
    {
        LOG_MSG(db_HAL_WARPING, "Panel not match, skip Blacklevel (%d %d)\n", sBlackLevelInfo->uc3DFlag, m_sHalWarpingInfo.uiWarp_Is120HzPanel);
		free(pucData); //A35G2_Alan_0039
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    for(int i=0 ; i<MAX_BLACKLEVEL_AREA_NUMBER ; i++)
    {
        //load blacklevel palette
        m_sBlackLevelPara[i].ucBlackLevel_PLT_R = sBlackLevelInfo->aucBiasPalette[0 + i*3] ;
        m_sBlackLevelPara[i].ucBlackLevel_PLT_G = sBlackLevelInfo->aucBiasPalette[1 + i*3] ;
        m_sBlackLevelPara[i].ucBlackLevel_PLT_B = sBlackLevelInfo->aucBiasPalette[2 + i*3] ;

        dvC341Geo_Blacklevel_Palette_Set(i,
                                      m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                      m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                      m_sBlackLevelPara[i].ucBlackLevel_PLT_B);

        LOG_MSG(db_HAL_WARPING, "%d load R%d G%d B%d\n" , i,
                                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_B);

        //load BlackLevelPara
        memcpy(&m_sBlackLevelPara[i], &sBlackLevelInfo->asBlackLevelPara[i], sizeof(m_sBlackLevelPara[0]));
    }

    // load Black Level Table
    char cRawDataFileName[64];
    snprintf(cRawDataFileName, 64, BLACKLEVEL_RAW_DATA_FILE, ucIndex);

    char cCurrentRawDataFileName[64];
    snprintf(cCurrentRawDataFileName, 64, BLACKLEVEL_RAW_DATA_FILE, BLACKLEVEL_CURRENT_INDEX);

    //load to current file
    if(access(cRawDataFileName, R_OK) == 0 && ucIndex != BLACKLEVEL_CURRENT_INDEX)
    {
        //copy raw data to current
        SYSTEM_CALL("cp %s %s", cRawDataFileName, cCurrentRawDataFileName);

        //copy blacklevel info to current
        char cCurrentInfoFileName[64];
        sprintf(cCurrentInfoFileName, BLACKLEVEL_FILE, BLACKLEVEL_CURRENT_INDEX);
        SYSTEM_CALL("cp %s %s", cFileName, cCurrentInfoFileName);
    }

    //load raw data file to tmp
    if(access(cRawDataFileName, R_OK) == 0)
    {
        SYSTEM_CALL("cp %s %s", cRawDataFileName, BLACKLEVEL_TMP_FILE);
    }

    dvC341Geo_LoadEgbBiasArea(cCurrentRawDataFileName);

    free(pucData);

    #if 0
    //load DMA
    dvC341Geo_Write(B4_OSDCT, 0x00);
    dvC341Geo_Write(B4_CPUWAD, DEF_EBIASSAD);

    UINT32 ulAddress = WARPING_TWIST_BIAS_TABLE_ADDRESS + WARPING_MEMORY_GROUP_SIZE * ucIndex;
    dvC341Geo_DMA_Transfer(ulAddress, WARPING_TWIST_BIAS_TABLE_SIZE, eC789_SFLREGSEL_DDR, eC789_SFLCT_DMA_FLASH2REG);
    #endif

    //Enable black level
    dvC341Geo_EdgebldBiasEnable(TRUE);


    LOG_MSG(db_HAL_WARPING, "Load Blacklevel %d End\n", ucIndex);


    return eHAL_WARPING_EXEC_CODE_PASS;
}



eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ClearAllData(void)  //A35G2_Simon_0093
{
    //halWarping_Blacklevel_Reset();

    if(access("/mnt/configs/scaler/blending/blacklevel0.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel0.dat");
    if(access("/mnt/configs/scaler/blending/blacklevel0_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel0_raw_data.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel1.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel1.dat");
    if(access("/mnt/configs/scaler/blending/blacklevel1_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel1_raw_data.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel2.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel2.dat");
    if(access("/mnt/configs/scaler/blending/blacklevel2_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel2_raw_data.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel3.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel3.dat");
    if(access("/mnt/configs/scaler/blending/blacklevel3_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel3_raw_data.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel4.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel4.dat");
    if(access("/mnt/configs/scaler/blending/blacklevel4_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel4_raw_data.dat");


    if(access("/mnt/configs/scaler/blending/blacklevel_tmp.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel_tmp.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel_handling.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel_handling.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel5.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel5.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel5_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel5_raw_data.dat");

    SYSTEM_CALL("sync");

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ClearCurrentData(void)  //A35G2_Simon_0093
{
    if(access("/mnt/configs/scaler/blending/blacklevel_tmp.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel_tmp.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel_handling.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel_handling.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel5.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel5.dat");

    if(access("/mnt/configs/scaler/blending/blacklevel5_raw_data.dat", R_OK) == 0)
        SYSTEM_CALL("rm /mnt/configs/scaler/blending/blacklevel5_raw_data.dat");

    SYSTEM_CALL("sync");

    return eHAL_WARPING_EXEC_CODE_PASS;

}

void halWarping_Blacklevel_Debug(void)
{
    int i = 0 ;

    LOG_MSG(db_HAL_WARPING, "\n");


    LOG_MSG(db_HAL_WARPING, "\n\nucBlacklevelAreaSel = %d\n\n", ucBlacklevelAreaSel);

    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", i);

    LOG_MSG(db_HAL_WARPING, "\n>> ucBlackLevel_AreaEnable\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].ucBlackLevel_AreaEnable);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_TL_X\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_TL_X);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_TL_Y\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_TL_Y);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_TR_X\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_TR_X);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_TR_Y\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_TR_Y);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_BL_X\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_BL_X);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_BL_Y\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_BL_Y);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_BR_X\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_BR_X);

    LOG_MSG(db_HAL_WARPING, "\n>> uiBlackLevel_CUR_BR_Y\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].uiBlackLevel_CUR_BR_Y);

    LOG_MSG(db_HAL_WARPING, "\n>> ucBlackLevel_PLT_SEL\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].ucBlackLevel_PLT_SEL);

    LOG_MSG(db_HAL_WARPING, "\n>> ucBlackLevel_PLT_R\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].ucBlackLevel_PLT_R);

    LOG_MSG(db_HAL_WARPING, "\n>> ucBlackLevel_PLT_G\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].ucBlackLevel_PLT_G);

    LOG_MSG(db_HAL_WARPING, "\n>> ucBlackLevel_PLT_B\n");
    for(i=0 ; i<15 ; i++)
        LOG_MSG(db_HAL_WARPING, "%5d", m_sBlackLevelPara[i].ucBlackLevel_PLT_B);



    LOG_MSG(db_HAL_WARPING, "\n");
}


INT16 halWarping_HResGet(void)
{
    return m_sHalWarpingInfo.uiWarp_HW;
}

INT16 halWarping_VResGet(void)
{
    return m_sHalWarpingInfo.uiWarp_VW;
}

//return total color number
int CountColors(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend, int *colorSet)
{
    unsigned int red = 0, green = 0, blue = 0;
    unsigned int uiColor = 0;
    unsigned int ColorNumber = 0;

    LOG_MSG(db_HAL_WARPING, "Count Colors\n");

    int number=3*nWidth*nHeight;

    for (int index = 0; index < number ; index=index+3)
    {
        red = paucBlend[index];
        green = paucBlend[index + 1];
        blue = paucBlend[index + 2];
        // Format: 0xRRGGBB. For example: color=0x123456, red is 0x12, green=0x34, blue=0x56
        uiColor = (red <<16) + (green <<8) + blue;

        int ulColorSearchIndex = 0;
        int find = FALSE ;
        do
        {
            if(colorSet[ulColorSearchIndex] == uiColor)
            {
                find = TRUE;
                break;
            }

            ulColorSearchIndex++ ;

            if(ulColorSearchIndex > 255)
            {
                ulColorSearchIndex = 256;
                LOG_MSG(db_HAL_WARPING, "Stop caculation. More than 256 colors\n");
                break;
            }

        }while(ulColorSearchIndex <= 255 && ulColorSearchIndex <= ColorNumber);

        if(find == FALSE && ulColorSearchIndex <= 255)
        {
            colorSet[ulColorSearchIndex - 1] = uiColor ;
            ColorNumber = ulColorSearchIndex;
        }

        if(ulColorSearchIndex > 255)
        {
            ColorNumber = ulColorSearchIndex;  //A35G2_Simon_0067
            break;
        }

    }

    LOG_MSG(db_HAL_WARPING, " %d colors\n", ColorNumber);


    for(int i=0 ; i<ColorNumber ; i++)
    {
        LOG_MSG(db_HAL_WARPING, "C%d 0x%X\n", i, colorSet[i]);
    }

    return ColorNumber;

}

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnOSD(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend)
{
    int ulColorset[256] ;

    memset(ulColorset, 0xFF , sizeof(ulColorset));

    LOG_MSG(db_HAL_WARPING, "%s (%d %d %d %d)\n", __FUNCTION__, xPos, yPos, ulWidth, ulHeight);

    int numberOfColors = CountColors(xPos, yPos, ulWidth, ulHeight, paucBlend, ulColorset);
    LOG_MSG(db_HAL_WARPING, "PNG total color %d\n",  numberOfColors);

    //printf("numberOfColors is %d\n", numberOfColors);
    //printf("In DrawPNGOnOSD, colorSet size is %d\n", colorSet.size());

    // Cindy: change RTCT to avoid PNG not shown
    int GV_RTCT = dvC341Geo_Read(B0_RTCT6, 0);      // 0x0105;

    //dvC341Geo_Write(BN_RTCT, 0x0105);     // 0x0105 is the initial value of RTCT
    dvC341Geo_Write(B0_RTCT6, 0x0, 0);
	dvC341Geo_Write(B4_OSDCT, 0x00, 0);

    if(numberOfColors > 255)
    {
        DrawPNGOnOSDReducedColor(xPos, yPos, ulWidth, ulHeight, paucBlend);
    }
    else
    {
        DrawPNGOnOSDOriginalColor(xPos, yPos, ulWidth, ulHeight, paucBlend, ulColorset, numberOfColors);
    }

	halWarping_DBD_WriteDummy();

    // Restore RTCT
    dvC341Geo_Write(B0_RTCT6, GV_RTCT, 0);

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnMemory(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend)
{
    LOG_MSG(db_HAL_WARPING, "Start %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());

    if(halWarping_GetFreezeImageState() == FALSE)
    {
        halWarping_FreezeImage(TRUE);
    }


    int cpuwad = 0, offset = 0;
    int cpuwad_previous = -1;

    INT32 lHRes = halWarping_HResGet();
    INT32 lVRes = halWarping_VResGet();

    int x = xPos;
    int y = yPos;

    int tmpWidth =  (xPos + ulWidth) > lHRes ? lHRes : ulWidth;
    int tmpHeight = (yPos + ulHeight) > lVRes ? lVRes : ulHeight;

    //uint8 *pBuffer = malloc(0x10000);
    uint32 count = 0;
    uint8 idxTemp = 9;
    uint32 starty = 0;
    uint32 startx = 0;

    //C341 wait review
    //C341 wait review
    //C341 wait review

#if 0 //A35G2_CDS_Larry_0029 //先處理line 4的倍數
    {
        tmpHeight = (tmpHeight/4)*4;

        for (y = yPos; y < tmpHeight; y++)
        {
            if((y % 4) == 0)
            {
                cpuwad = (y / 4) * (2 - 0) * 32768 + (y % 4) * 16;

                dvC341Geo_BurstWrite_AddInc(B4_CPUWAD, 4, (UINT8*)&cpuwad);

                memset(pBuffer, 0, 0x10000);
                count = 0;
            }

            for (int idx = 0; idx < 3; idx++)
            {
                for (x = xPos; x < tmpWidth; x++)
                {
                    if (x % 12 < 6)
                        offset = x % 12;
                    else
                        offset = x % 12 + 2;

                    if (x % 12 == 0 || x == xPos || (idxTemp != idx))
                    {
                        idxTemp = idx;

                        cpuwad =
                            (y / 4) * (2 - 0) * 32768 + (y % 4) * 16 +
                            (x / 12) * 256 + offset;
                        cpuwad = cpuwad + 64 * idx;

                        starty = cpuwad%0x10000;
                        startx = 0;

                    }
                    else if (x % 12 == 6)   //dummy//A35G2_CDS_Simon_0028
                    {
                        startx++;
                        startx++;
                    }

                    pBuffer[starty + startx] = paucBlend[tmpWidth * 3 * y + 3 * x + idx];
                    startx++;

                    if(count < starty + startx)
                    {
                        count = starty + startx;
                    }
                }
            }

            if((y % 4) == 3)
            {
                //dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, count, pBuffer);
            }

        }
    }

    free(pBuffer);
#endif /* 0 */


#if 0 //A35G2_CDS_Larry_0029 //處理line 4的餘數
    {
        tmpHeight = (yPos + ulHeight) > lVRes ? lVRes : ulHeight;
        tmpHeight = yPos + y + (tmpHeight%4);
		idxTemp = 9;

        for (; y < tmpHeight; y++)  //A35G2_Simon_0070 for cppcheck
        {
            for (int idx = 0; idx < 3; idx++)
            {
                for (x = xPos; x < tmpWidth; x++)
                {
                    if (x % 12 < 6)
                        offset = x % 12;
                    else
                        offset = x % 12 + 2;

                    if (x % 12 == 0 || x == xPos || (idxTemp != idx))
                    {
                        idxTemp = idx;

                        cpuwad =
                            (y / 4) * (2 - 0) * 32768 + (y % 4) * 16 +
                            (x / 12) * 256 + offset;
                        cpuwad = cpuwad + 64 * idx;
                        //dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad);
                    }
                    else if (x % 12 == 6)   //A35G2_CDS_Simon_0028
                    {
                        //dvC341Geo_WriteToBuffer(B4_CPUDT, 0x00);
                        //dvC341Geo_WriteToBuffer(B4_CPUDT, 0x00);
                    }

                    //dvC341Geo_WriteToBuffer(B4_CPUDT, paucBlend[tmpWidth * 3 * y + 3 * x + idx]);
                }
            }

        }
    }
#endif /* 0 */

    dvC341Geo_Buffer_Flush();

    LOG_MSG(db_HAL_WARPING, "End %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void DrawPNGOnOSDOriginalColor(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend, int *colorSet, int colorset_size)
{
    LOG_MSG(db_HAL_WARPING, "%s Start\n", __FUNCTION__);

    // 1.
    // Reduce color and calcuate palette
    unsigned char *pixelIndexed = (unsigned char *)malloc(1920 * 1200 + 100);
    memset(pixelIndexed, 0, 1920 * 1200 + 100);
    UINT32 ulPalette256[256];  //vector < unsigned int >vPalette256;
    memset(ulPalette256, 0, sizeof(ulPalette256));

    //paucBlend : BYTE 0 : Pixel0 R
    //            BYTE 1 : Pixel0 G
    //            BYTE 2 : Pixel0 B
    //            BYTE 3 : Pixel1 R
    //            BYTE 4 : Pixel1 G
    //            BYTE 5 : Pixel1 B .....
    CalculatePaletteAndTransformRGB888(nWidth, nHeight, paucBlend, pixelIndexed, ulPalette256, colorSet, colorset_size);

    // 2.
    // Draw
    LOG_MSG(db_HAL_WARPING, "Write Palette...\n");
    for(int i=0; i<sizeof(ulPalette256)/sizeof(ulPalette256[0]) ; i++)
    {
        //LOG_MSG(db_HAL_WARPING, "%d 0x%X\n", i, ulPalette256[i]);
    }

    halWarpOSD_DisableOSD();    //G100_Simon_0007
    //halWarping_OSDTransparentColorSet(0xFF);  //G100_Simon_0078
    UINT8 PLT[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT] = {0};
    halWarping_OSDPalette_32bitsDataTo256x3Data(ulPalette256, PLT);
    halWarping_OSDWritePalette(PLT, 0, 256, FALSE);
    halWarping_OSDWriteBlink(FALSE,0,0,FALSE);
    halWarping_OSDWriteTransparentColor(TRUE, 0,0,0,0);
    //dvC341Geo_Buffer_Flush(); //A35G2_CDS_Simon_0044
    halWarpOSD_ClearOSD();
    halWarping_DrawSpriteBuffer(xPos, yPos, nWidth, nHeight, pixelIndexed);
    dvC341Geo_Buffer_Flush(); //A35G2_CDS_Simon_0044
    dvC341Geo_SetOSDMode(FALSE);  //G100_Simon_0088 //A35G2_CDS_Simon_0044

#ifdef PNG_OSD_NO_TRANSPARENT_COLOR
    //Disable transparent color
    halWarping_OSDWriteTransparentColor(FALSE,0,0,0,0);
#endif

    free(pixelIndexed);
}

void CalculatePaletteAndTransformRGB888(int nWidth, int nHeight, unsigned char *paucBlend, unsigned char *pixelIndexed,
    unsigned int *ulPalette256, int *colorSet, int colorset_size)

{
    LOG_MSG(db_HAL_WARPING, "%s Start\n", __FUNCTION__);

    // 1. Get colorSet to map and vector
    unsigned int transparency = 0;
    ulPalette256[0] = transparency;

    //index 0 is reserved for transparency
    int colorIndex=1;
    LOG_MSG(db_HAL_WARPING, "colorset_size = %d\n", colorset_size /*colorSet.size()*/);

    for(int i=0 ; i<colorset_size ; i++)
    {
        unsigned int color = colorSet[i] ;
        LOG_MSG(db_HAL_WARPING, "%d-%X\n",colorIndex, color);

        if(colorIndex >= 256) //over size
        {
            LOG_MSG(db_HAL_WARPING, "colorset over size\n");
            break;
        }

        ulPalette256[colorIndex] = color ;
        colorIndex++;
    }

    // 2. Transform
    unsigned short red = 0, green = 0, blue = 0;
    unsigned int uiColor;
    int nPos;

    for (int y = 0; y < nHeight; y++)
    {
        for (int x = 0; x < nWidth; x++)
        {
            nPos = y * nWidth + x;
            red   = paucBlend[nWidth * 3 * y + 3 * x + 0];
            green = paucBlend[nWidth * 3 * y + 3 * x + 1];
            blue  = paucBlend[nWidth * 3 * y + 3 * x + 2];
            // Format: 0xRRGGBB. For example: color=0x123456, red is 0x12, green=0x34, blue=0x56
            uiColor = red * 65536 + green * 256 + blue;
            //printf("> uiColor=0x%X\n", uiColor);
            int findindex=0 ;
#ifdef PNG_OSD_NO_TRANSPARENT_COLOR
            for(findindex=0 ; findindex<256 ; findindex++)  //ulPalette256[0] is black color      //A35G2_Simon_0067
#else
            for(findindex=1 ; findindex<256 ; findindex++)  //ulPalette256[0] is transparent color  //A35G2_CDS_Simon_0045
#endif
            {
                if (ulPalette256[findindex] == uiColor)
                {
                    // cout<<"Find, the value is"<<iter->second<<endl;
                    pixelIndexed[nPos] = (unsigned char)findindex;

                    break;
                }
            }

            if(findindex == 256)  //not find
            {
                //printf("Error! Ran out color palette. It cannot be drawn\n");
                pixelIndexed[nPos] = 0;
            }

         }
     }
}





void DrawPNGOnOSDReducedColor(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend)
{
    LOG_MSG(db_HAL_WARPING, "%s Start\n", __FUNCTION__);

    // 1.
    // Reduce color and calcuate palette
    unsigned char *pixelIndexed = (unsigned char *)malloc(1920 * 1200 + 100);  //A35G2_Simon_0067
    memset(pixelIndexed, 0, 1920 * 1200 + 100);
    UINT32 ulPalette256[256];
    memset(ulPalette256, 0, sizeof(ulPalette256));

    ColorUniformQuantization884(nWidth, nHeight, paucBlend, pixelIndexed, ulPalette256);
    // ColorUniformQuantization676(nWidth, nHeight, paucBlend, pixelIndexed,
    // vPalette256);

    // 2.
    // Draw
    /*
       printf("print palette"); for(int i=0; i< vPalette256.size() ; i++) {
       printf("%d: 0x%X \n", i, vPalette256.at(i)); } */
    halWarpOSD_DisableOSD();    //G100_Simon_0007
    //halWarping_OSDTransparentColorSet(0xFF);  //G100_Simon_0078
    UINT8 PLT[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT] = {0};
    halWarping_OSDPalette_32bitsDataTo256x3Data(ulPalette256, PLT);
    halWarping_OSDWritePalette(PLT, 0, 256, FALSE);
    halWarping_OSDWriteBlink(FALSE,0,0,FALSE);
    halWarping_OSDWriteTransparentColor(TRUE, 0,0,0,0);
    //dvC341Geo_Buffer_Flush();
    halWarpOSD_ClearOSD();
    halWarping_DrawSpriteBuffer(xPos, yPos, nWidth, nHeight, pixelIndexed);
    dvC341Geo_Buffer_Flush(); //A35G2_CDS_Simon_0044
    dvC341Geo_SetOSDMode(FALSE);  //G100_Simon_0088 //A35G2_CDS_Simon_0044

#ifdef PNG_OSD_NO_TRANSPARENT_COLOR
    //Disable transparent color
    halWarping_OSDWriteTransparentColor(FALSE,0,0,0,0);
#endif

    free(pixelIndexed);
}


void ColorUniformQuantization884(int nWidth,
                                             int nHeight,
                                             unsigned char *paucBlend,
                                             unsigned char *pixelIndexed,
                                             UINT32 *ulPalette256)
{
    // idx 0 is red, idx 1 is green, index 2 is blue
    // Step 1: get indexed data
    unsigned char red = 0, green = 0, blue = 0;

    for (int y = 0; y < nHeight; y++)
    {
        for (int x = 0; x < nWidth; x++)
        {
            red = paucBlend[nWidth * 3 * y + 3 * x + 0];
            green = paucBlend[nWidth * 3 * y + 3 * x + 1];
            blue = paucBlend[nWidth * 3 * y + 3 * x + 2];

            red = red / 32;
            green = green / 32;
            blue = blue / 64;

            #ifndef PNG_OSD_NO_TRANSPARENT_COLOR      //A35G2_Simon_0067
            if ((red == 0) && (green == 0) && (blue == 0))
            {
                red = 1;        // Preserve 0 to transparency
            }
            #endif

            pixelIndexed[nWidth * y + x] = red * 32 + green * 4 + blue;
        }
    }

    // Step 2: get palette
    const unsigned int level4[4] = { 0, 85, 170, 255 };
    const unsigned int level8[8] = { 0, 36, 72, 109, 145, 182, 218, 255 };
    unsigned int paletteR = 0, paletteG = 0, paletteB = 0, color32 = 0;
    unsigned int ulPaletteIndex = 0;

    for (int index_r = 0; index_r < 8; index_r++)
    {
        for (int index_g = 0; index_g < 8; index_g++)
        {
            for (int index_b = 0; index_b < 4; index_b++)
            {

                paletteR = level8[index_r];
                paletteG = level8[index_g];
                paletteB = level4[index_b];

                // Format: color=0x123456, red is 0x12, green=0x34, blue=0x56
                color32 = paletteR * 65536 + paletteG * 256 + paletteB;
                // printf("paletteR=0x%X, paletteG=0x%X, paletteB=0x%X,
                // color32=0x%X\n", paletteR, paletteG, paletteB, color32);
                ulPalette256[ulPaletteIndex] = color32;
                ulPaletteIndex++;
            }
        }
    }

    ulPalette256[0] = 0x0;    // Preserve to transparency

}


void CalculatePaletteAndTransformRGB5551(sPixmapData *pasPixmap,
                                                        UINT16 uiPixmapNumber,
                                                        unsigned int *ulmapPalette255,
                                                        unsigned int *ulPalette256)  //vPalette256
{
	LOG_MSG(db_HAL_WARPING, "Start CalculatePaletteAndTransformRGB5551\n");

    // Init first transparency
    unsigned short transparency = 0;

    //UINT16 ulMapPalette256_Count = 0;
    //UINT16 ulPalette256_Count = 0;

    ulPalette256[0] = transparency;

    unsigned short red = 0, green = 0, blue = 0;

    for (int idx = 0; idx < uiPixmapNumber; idx++)
    {
        int nPos = 0;

        for (int i = 0; i < pasPixmap[idx].sPixmap_OnOSD.nHeight; i++)
        {
            for (int j = 0; j < pasPixmap[idx].sPixmap_OnOSD.nWidth; j++)
            {
                nPos = i * pasPixmap[idx].sPixmap_OnOSD.nWidth + j;

                // printf("ucPixData 0x%X,0x%X ",pvPixmap[idx].ucPixData[2 *
                // nPos + 1], pvPixmap[idx].ucPixData[2 * nPos]);
                unsigned short usColor = (unsigned short)(pasPixmap[idx].sPixmap_OnOSD.ucPixData[2 * nPos + 1] << 8) +
                                         (unsigned short)(pasPixmap[idx].sPixmap_OnOSD.ucPixData[2 * nPos]);

                //printf("usColor=0x%X \n",usColor);

                // Check if it is transparent or no
                if ((usColor & 1 << 15) == 0)
                {
                    m_sHalWarpingInfo.ucPixIndexed[nPos] = transparency;
                    //printf("transparency...usColor=%x\n", usColor);
                }
				else
				{
                    int findindex=0 ;

                    for(findindex=1 ; findindex<255 ; findindex++)  //A35G2_CDS_Simon_0045
                    {
                        if (ulmapPalette255[findindex] == usColor)
                        {
                            m_sHalWarpingInfo.ucPixIndexed[nPos] = (unsigned char)findindex + 1;
                            break;
                        }
                    }

	                if( findindex < 255 )   //find
	                {
	                    // cout<<"Find, the value is"<<iter->second<<endl;
	                  //  printf("Find, the value is %d\n", iter->second);
                        //printf("find usColor=%x (%d)\n", usColor , findindex + 1);
	                    m_sHalWarpingInfo.ucPixIndexed[nPos] = (unsigned char)findindex + 1;
	                }
	                else
	                {
	                    // cout<<"Do not Find"<<endl;
                        m_sHalWarpingInfo.uiSpritePaletteCount++;

	                    if (m_sHalWarpingInfo.uiSpritePaletteCount < 256)
	                    {
	                       // printf("Add palette: usColor=0x%X \n",usColor);

	                        red = (unsigned int)(usColor & 0x001F);
	                        green = (unsigned int)(usColor & 0x03E0);
	                        green = green >> 5;
	                        blue = (unsigned int)(usColor & 0x7C00);
	                        blue = blue >> 10;

	                        unsigned int color32 = 0;       // Format:
	                                                        // color=0x123456, red
	                                                        // is 0x12,
	                                                        // green=0x34,
	                                                        // blue=0x56
	                        color32 = red * 8 * 65536 + green * 8 * 256 + blue * 8;
	                        // unsigned int color32=0; //Format: color=0x123456,
	                        // red is 0x56, green=0x34, green=0x12
	                        // color32= red*8+ green*8*256 + blue*8*65536;

	                      //  printf("r g  b color32 0x%X 0x%X 0x%X 0x%X\n", red,
	                      //         green, blue, color32);

 	                        LOG_MSG(db_HAL_WARPING, "rgb%d(%d %d %d)\n",m_sHalWarpingInfo.uiSpritePaletteCount, red, green, blue);

	                        // Add new color
	                        ulmapPalette255[m_sHalWarpingInfo.uiMapSpritePaletteCount] = usColor;
	                        m_sHalWarpingInfo.uiMapSpritePaletteCount++;

                            ulPalette256[m_sHalWarpingInfo.uiSpritePaletteCount] = color32 ;
	                        // Transform color
	                        m_sHalWarpingInfo.ucPixIndexed[nPos] = (unsigned char)ulPalette256[m_sHalWarpingInfo.uiSpritePaletteCount];

	                    }
	                    else
	                    {
	                        //LOG_MSG(db_HAL_WARPING, "Error! Ran out color palette. It cannot be drawn\n");
	                        LOG_MSG(db_HAL_WARPING, "OC\n");
	                        m_sHalWarpingInfo.ucPixIndexed[nPos] = 0;
	                    }
	                }
				}
            }
        }

        LOG_MSG(db_HAL_WARPING, "idx %d , Total Pixel = %d\n" , idx , nPos);
    }


	LOG_MSG(db_HAL_WARPING, "End %s (%d %d)\n", __FUNCTION__, m_sHalWarpingInfo.uiMapSpritePaletteCount, m_sHalWarpingInfo.uiSpritePaletteCount);

}


eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnMemory(INT16 iTotalNumber,
                                                                   INT16 iSpritePid,
                                                                   INT16 iSpriteTop,
                                                                   INT16 iSpriteLeft,
                                                                   sPixmapData *psPixmap)
{
    LOG_MSG(db_HAL_WARPING, "Start %s\n", __FUNCTION__);


    #if 0
    LOG_MSG(db_HAL_WARPING, "%s (%d %d %d %d)\n", __FUNCTION__,
                                                  iTotalNumber,
                                                  iSpritePid,
                                                  iSpriteTop,
                                                  iSpriteLeft
                                                  );
    #endif

    if(halWarping_GetFreezeImageState() == FALSE)
    {
        halWarping_FreezeImage(TRUE);
    }

    int cpuwad = 0, offset = 0;

    for (int idx = 0; idx < 3; idx++)
    {
        for (int i = 0; i < iTotalNumber; i++)
        {
            //int nPID = iSpritePid;
            int nPID = 0 ;  //always one sPixmap

            psPixmap = psPixmap + i;

            int tmpWidth  = (iSpriteLeft + psPixmap->sPixmap_OnMemory.nWidth)  > halWarping_HResGet() ? halWarping_HResGet() : (iSpriteLeft + psPixmap->sPixmap_OnMemory.nWidth);
            int tmpHeight = (iSpriteTop  + psPixmap->sPixmap_OnMemory.nHeight) > halWarping_VResGet() ? halWarping_VResGet() : (iSpriteTop  + psPixmap->sPixmap_OnMemory.nHeight);

            // dvC790_WriteBuffer(B4_CPUWAD, 0x00, 0);

            //C341 wait review
            //C341 wait review
            //C341 wait review
            #if 0
            for (int y = iSpriteTop ; y < tmpHeight ; y++)
            {
                for (int x = iSpriteLeft ; x < tmpWidth ; x++)
                {
                    if (x % 12 < 6)
                        offset = x % 12;
                    else
                        offset = x % 12 + 2;

                    if (x % 12 == 0 || x == iSpriteLeft)
                    {
                        cpuwad = (y / 4) * (2 - 0) * 32768 + (y % 4) * 16 + (x / 12) * 256 + offset;
                        cpuwad = cpuwad + 64 * idx;
                        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad);
                    }
                    else if (x % 12 == 6)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, 0x00);
                        dvC341Geo_WriteToBuffer(B4_CPUDT, 0x00);
                    }

                    if(idx == 0)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, psPixmap->sPixmap_OnMemory.ucPixDataR[psPixmap->sPixmap_OnMemory.nWidth * (y - iSpriteTop) + (x - iSpriteLeft)] * 8);
                    }
                    else if(idx == 1)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, psPixmap->sPixmap_OnMemory.ucPixDataG[psPixmap->sPixmap_OnMemory.nWidth * (y - iSpriteTop) + (x - iSpriteLeft)] * 8);
                    }
                    else if(idx == 2)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, psPixmap->sPixmap_OnMemory.ucPixDataB[psPixmap->sPixmap_OnMemory.nWidth * (y - iSpriteTop) + (x - iSpriteLeft)] * 8);
                    }
                }
            }
            #endif
        }


        // !!!!!!!!!!!!!!!!

        //dvC341Geo_Buffer_Flush();    //notice : remove Flush (for speed up), so need to Flush when finishing working

        // !!!!!!!!!!!!!!!!


    }

    dvC341_Buffer_Flush();  //A35G2_CDS_Simon_0058

    LOG_MSG(db_HAL_WARPING, "End %s\n", __FUNCTION__);

    return eHAL_WARPING_EXEC_CODE_PASS;

}



eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnOSD(INT16 iTotalNumber,
                                                              INT16 iSpritePid,
                                                              INT16 iSpriteTop,
                                                              INT16 iSpriteLeft,
                                                              sPixmapData *psPixmap)
{
    //printf("DrawSpritesOnOSD start time: %ld ms\n", currentTimeMillis());

    // First step: make palette data and transform to indexed data
    // mapPalette255--> first: color, second: palette index

    //unsigned int ulSpritePalette256[256];
    //unsigned int mapSpritePalette255[256];
    //memset(ulSpritePalette256,  0, sizeof(ulSpritePalette256));
    //memset(mapSpritePalette255, 0, sizeof(mapSpritePalette255));
    #if 0
    LOG_MSG(db_HAL_WARPING, "%s (%d %d %d %d)\n", __FUNCTION__,
                                                  iTotalNumber,
                                                  iSpritePid,
                                                  iSpriteTop,
                                                  iSpriteLeft
                                                  );
    #endif

    CalculatePaletteAndTransformRGB5551(psPixmap, iTotalNumber, m_sHalWarpingInfo.aulMapSpritePalette, m_sHalWarpingInfo.aulSpritePalette);

    // Cindy: change RTCT to avoid PNG not shown
    //int GV_RTCT = dvC341Geo_Read(BN_RTCT);      // 0x0105;
    //dvC341Geo_WriteToBuffer(BN_RTCT, 0x0105);     // 0x0105 is the initial value of RTCT
	//dvC341Geo_WriteToBuffer(B4_OSDCT, 0x00);
	//dvC341Geo_Buffer_Flush();

	// Second step: Write Palette
    UINT8 PLT[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT] = {0};
    halWarping_OSDPalette_32bitsDataTo256x3Data(m_sHalWarpingInfo.aulSpritePalette, PLT);
    halWarping_OSDWritePalette(PLT, 0, m_sHalWarpingInfo.uiSpritePaletteCount + 1, FALSE);
    halWarping_OSDWriteBlink(FALSE,0,0,FALSE);
    halWarping_OSDWriteTransparentColor(TRUE, 0,0,0,0);

    // Third step: Draw Sprites

    // Cindy: change RTCT to avoid blending not shown
    //dvC341Geo_Write(BN_RTCT, 0x0105);     // 0x0105 is the initial value
                                               // of RTCT
    int nPID = 0;

    for (int idx = 0; idx < iTotalNumber; idx++)
    {
        // printf("PID : %d Top : %d Left : %d \n", pvSprites[idx].nPID,
        // pvSprites[idx].nTop, pvSprites[idx].nLeft);

        psPixmap = psPixmap + idx ;

        nPID = psPixmap->sPixmap_OnOSD.nPID;

        halWarping_DrawSpriteBuffer(iSpriteLeft,  //the same
                                    iSpriteTop,   //the same
                                    psPixmap->sPixmap_OnOSD.nWidth,
                                    psPixmap->sPixmap_OnOSD.nHeight,
                                    m_sHalWarpingInfo.ucPixIndexed);
    }
    //dvC341Geo_Buffer_Flush();

    // dummy start
    halWarping_DBD_WriteDummy();
    // dummy end

    // Restore RTCT
    //dvC341Geo_Write(BN_RTCT, GV_RTCT);

    dvC341_Buffer_Flush();  //A35G2_CDS_Simon_0058

    return eHAL_WARPING_EXEC_CODE_PASS;

}



void halWarping_DrawSpriteBuffer(int startX,
                                             int startY,
                                             int width,
                                             int height,
                                             unsigned char *paucBlend)
{
    LOG_MSG(db_HAL_WARPING, "Start %s (%d %d %d %d)\n", __FUNCTION__, startX, startY, width, height);

    int xEnd = startX + width;
    xEnd=(xEnd < halWarping_HResGet())? xEnd:halWarping_HResGet();

    int yEnd = startY + height;
    yEnd=(yEnd < halWarping_VResGet())? yEnd:halWarping_VResGet();

#if 1
    INT32 lIndex = 0 ;

    dvC341Geo_SetOSDMode(FALSE);  //A35G2_Simon_0067

    for(int y = startY ; y < yEnd ; y++)
    {
        int cpuwad = m_OSD_CURRENT_LAYER + startX + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * y;
        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad, 0);

        for(int x = startX ; x < xEnd ; x++)
        {
            if(x < 0 || y < 0)
            {
                dvC341Geo_WriteToBuffer(B4_CPUDT, 0, 0);
            }
            else
            {
                dvC341Geo_WriteToBuffer(B4_CPUDT, paucBlend[lIndex], 0);
            }

            lIndex++ ;
        }
    }

    //dvC341Geo_Buffer_Flush();
#endif

    #if 0
    for (int y = startY; y < yEnd; y++)
    {
        if( xEnd-startX <= dvC341Geo_BurstModeMaxSize() )
        {
            int cpuwad = m_OSD_CURRENT_LAYER + startX + DEF_OSDMWI * 128 * y;   //A65_OPTOMA_Simon_0001
            dvC341Geo_Write(B4_CPUWAD, cpuwad);
            dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, xEnd-startX , (UINT8 *)&paucBlend[startX + (y-startY)*width]);
        }
        else  //G100_Simon_0003
        {
            UINT32 ulBlockIndex = 0;
            UINT32 ulBlockSize  = dvC341Geo_BurstModeMaxSize();
            UINT32 ulOffset = 0;
            UINT32 ulRemainSize = xEnd - startX ;

            while(ulRemainSize > 0)
            {
                ulOffset = ulBlockIndex * ulBlockSize ;

                int cpuwad = m_OSD_CURRENT_LAYER + (startX + ulOffset) + DEF_OSDMWI * 128 * y;    //A65_OPTOMA_Simon_0001
                dvC341Geo_Write(B4_CPUWAD, cpuwad);

                if(ulRemainSize >= dvC341Geo_BurstModeMaxSize())
                {
                    dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, dvC341Geo_BurstModeMaxSize() , (UINT8 *)&paucBlend[startX + ulOffset + (y-startY)*width]);
                    ulRemainSize = ulRemainSize - dvC341Geo_BurstModeMaxSize();
                }
                else   //last block
                {
                    dvC341Geo_BurstWrite_FixedAdd(B4_CPUDT, ulRemainSize , (UINT8 *)&paucBlend[startX + ulOffset + (y-startY)*width]);
                    ulRemainSize = 0;
                    break;
                }

                ulBlockIndex++;

            }
        }
    }
    #endif

    LOG_MSG(db_HAL_WARPING, "End %s\n", __FUNCTION__);

}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSDModeSet(UINT8 ucFront)  //G100_Simon_0088
{
    if(ucFront)   //insert warp OSD before warp
    {
        dvC341Geo_SetOSDMode(TRUE);
    }
    else   //insert warp OSD after warp
    {
        dvC341Geo_SetOSDMode(FALSE);
    }

	return eHAL_WARPING_EXEC_CODE_PASS;
}


//G100_Simon_0079
eHAL_WARPING_EXEC_CODE halWarping_DrawRect(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    UINT8 ucFastWrite = 0;
	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    if( eType >= eWDT_INVALID ||
        iPositionX < 0 ||
        iPositionY < 0 ||
        iPositionX > m_sHalWarpingInfo.uiWarp_HW ||
        iPositionY > m_sHalWarpingInfo.uiWarp_VW ||
        iWidth  > m_sHalWarpingInfo.uiWarp_HW ||
        iHeight > m_sHalWarpingInfo.uiWarp_VW ||
        iColorIndex > 255
    )
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) Error %d %d %d %d %d %d\n", __FUNCTION__
    	                                                           , __LINE__
    	                                                           , eType
    	                                                           , iPositionX
    	                                                           , iPositionY
    	                                                           , iWidth
    	                                                           , iHeight
    	                                                           , iColorIndex);

        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
        return eResult;
    }


    if(eType == eWDT_BEFORE_WARP)   //insert warp OSD before warp
    {
        dvC341Geo_SetOSDMode(TRUE);  //A65_OPTOMA_Simon_0001
    }
    else   //insert warp OSD after warp
    {
        dvC341Geo_SetOSDMode(FALSE);
    }

    int cpuwad;
    int GV_RTCT_BAK = dvC341Geo_Read(B0_RTCT6, 0); //0x0105;

    //Cindy: change RTCT to avoid blending not shown
    int GV_RTCT = 0;  //= 0x0105;  //C341 wait review

    iWidth = iWidth/OUTPUT_CH_NUM;

#ifdef OSDCT_FASTEN_ON
    if (iWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        iWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0 &&
        iPositionX % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        iWidth = iWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
#endif

    cpuwad = m_OSD_CURRENT_LAYER + iPositionX + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iPositionY;  //A65_OPTOMA_Simon_0001

    LOG_MSG(db_HAL_WARPING, "cpuwad 0x%X\n\n", cpuwad);

    //GV_RTCT = GV_RTCT | 0x0007;  //C341 wait review
    dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
    dvC341Geo_WriteToBuffer(B4_OSDFILL, iColorIndex, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTHW, (iWidth -1)&0xfff, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTVW, (iHeight-1)&0xfff, 0);
    dvC341Geo_WriteToBuffer(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH/OSD_LINEFEED_BASE), 0);
    dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad&CPUWAD_MASK, 0);
    dvC341Geo_WriteToBuffer(B9_OSDSAD2CH1, m_OSD_CURRENT_LAYER, 0);  //A65_OPTOMA_Simon_0001
    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
    }
    //GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;  //C341 wait review
    dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
    dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT_BAK, 0);
    dvC341Geo_Buffer_Flush();

    do { } while ( (dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) == 0x01 );

	eResult = eHAL_WARPING_EXEC_CODE_PASS;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);

	return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_DrawCheckboard(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                           INT16 iRectSize)
{
    INT16 iHorzRes = m_sHalWarpingInfo.uiWarp_HW ;
    INT16 iVertRes = m_sHalWarpingInfo.uiWarp_VW ;

    LOG_MSG(db_HAL_WARPING, "%s %d %d\n" ,__FUNCTION__, ucLayerMode ,iRectSize);

    if(iRectSize <= 0 || ucLayerMode >= eWDT_INVALID)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }


    if(ucLayerMode == eWDT_AFTER_WARP)  //A65_OPTOMA_Simon_0001
    {
        dvC341Geo_SetOSDMode(FALSE);
    }
    else
    {
        dvC341Geo_SetOSDMode(TRUE);
    }

    //write palette
    UINT8 aucWhite[3] = {255,255,255};
    UINT8 aucBlack[3] = {0,0,0};
    dvC341Geo_Write(B9_PLTAD1CH1, COLOR_IDX__WHITE, 0);  //A35G2_CDS_Simon_0046
    dvC341Geo_BurstWrite_FixedAdd(B9_PLTDT1CH1, 3, aucWhite);  // 0
    //halWarping_PaletteUpdate((UINT16)COLOR_IDX__WHITE, aucWhite[0], aucWhite[1], aucWhite[2]);  //A35G2_CDS_Simon_0046
    dvC341Geo_Write(B9_PLTAD1CH1, COLOR_IDX__BLACK, 0);  //A35G2_CDS_Simon_0046
    dvC341Geo_BurstWrite_FixedAdd(B9_PLTDT1CH1, 3, aucBlack);  // 1
    //halWarping_PaletteUpdate((UINT16)COLOR_IDX__BLACK, aucBlack[0], aucBlack[1], aucBlack[2]);  //A35G2_CDS_Simon_0046

    //draw full screen black color
    halWarping_DrawRect(ucLayerMode, 0, 0, iHorzRes, iVertRes, COLOR_IDX__BLACK );  //A35G2_CDS_Simon_0046

    //draw first white rect
    halWarping_DrawRect(ucLayerMode, 0, 0, iRectSize, iRectSize, COLOR_IDX__WHITE );  //A35G2_CDS_Simon_0046

    //draw all white rect by memory copy
    for(int y = 0 ; y < iVertRes ; y+=iRectSize)
    {
        for(int x = 0 ; x < iHorzRes ; x+=(iRectSize*2) )
        {
            if( ((y/iRectSize) % 2) == 0 )
            {
                halWarping_CopyOSDRect(ucLayerMode, 0, 0, x, y, iRectSize, iRectSize );
            }
            else
            {
                halWarping_CopyOSDRect(ucLayerMode, 0, 0, x+iRectSize , y, iRectSize, iRectSize );
            }
        }
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarping_DrawCheckboards(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                             INT16 iSizeX,
                                                             INT16 iSizeY,
                                                             INT16 iShiftX,
                                                             INT16 iShiftY)
{
    INT16 iHorzRes = m_sHalWarpingInfo.uiWarp_HW ;
    INT16 iVertRes = m_sHalWarpingInfo.uiWarp_VW ;

    LOG_MSG(db_HAL_WARPING, "%s (%d %d %d %d %d)\n" ,__FUNCTION__, ucLayerMode ,iSizeX, iSizeY, iShiftX, iShiftY);

    if(ucLayerMode >= eWDT_INVALID ||
       iSizeX  <= 0 ||
       iSizeY  <= 0 ||
       iShiftX < 0 ||
       iShiftY < 0
    )
    {
        LOG_MSG(db_HAL_WARPING, "%s fail\n" ,__FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }


    INT32 lPalette256[256];
    memset(lPalette256, 0, sizeof(lPalette256));

    unsigned int transparency = 0x0;
    unsigned int white = 0xFFFFFF;

    lPalette256[0] = 0;  //transparency
    lPalette256[1] = 0xFFFFFF; //white

    UINT8 PLT[WARP_OSD_TOTAL_PALETTE][WARP_OSD_PALETTE_ELEMENT] = {0};
    halWarping_OSDPalette_32bitsDataTo256x3Data(lPalette256, PLT);
    halWarping_OSDWritePalette(PLT, 0, 256, FALSE);
    halWarping_OSDWriteBlink(FALSE,0,0,FALSE);
    halWarping_OSDWriteTransparentColor(TRUE, 0,0,0,0);

    int nDouble, nFirstColor = 0, nTmpColor = 0;        // 0 : white, 1 : black(or transparent)
    if(iShiftX < 0) iShiftX = iSizeX + (iShiftX % iSizeX);
    if(iShiftY < 0) iShiftY = iSizeY + (iShiftY % iSizeY);
    int xGrid= (iShiftX / iSizeX) % 2;
    int yGrid= (iShiftY / iSizeY) % 2;
    nFirstColor = xGrid ^ yGrid;

    iShiftX = iSizeX - (iShiftX % iSizeX);
    iShiftY = iSizeY - (iShiftY % iSizeY);


    int nHight = 2 * iSizeY < iVertRes ? 2 * iSizeY : iVertRes;

    for (int y = 0; y < nHight;)
    {
        if (y != 0 && (y == iShiftY || y == iShiftY + iSizeY))
            nFirstColor = nFirstColor == 0 ? 0x01 : 0x00;

        int cpuwad = m_OSD_CURRENT_LAYER + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * y;

        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad, 0);

        unsigned char ucColor = 0, ucPLT = 0;
        int nWidth = iHorzRes + iShiftX;

        for (int x = 0; x < iHorzRes; x++)
        {
            if (x < iShiftX)
            {
                ucPLT = nFirstColor == 0 ? 0x01 : 0x00;
            }
            else if (((x - iShiftX) / iSizeX) % 2 == 0)
            {
                if (iShiftX == 0)
                    ucPLT = nFirstColor == 0 ? 0x01 : 0x00;
                else
                    ucPLT = nFirstColor == 0 ? 0x00 : 0x01;
            }
            else
            {
                if (iShiftX == 0)
                    ucPLT = nFirstColor == 0 ? 0x00 : 0x01;
                else
                    ucPLT = nFirstColor == 0 ? 0x01 : 0x00;
            }

            dvC341Geo_WriteToBuffer(B4_CPUDT, ucPLT, 0);

        }

        dvC341Geo_Buffer_Flush();

        int nYsize = 0;

        if (y == 0)
            nYsize = iShiftY != 0 ? iShiftY : iSizeY;
        else if (iShiftY != 0 && y == iShiftY + iSizeY)
            nYsize = iSizeY - iShiftY;
        else
            nYsize = iSizeY;

        for (int nHight = 1; nHight < nYsize; nHight *= 2)
        {
            // printf("Height : %d \n", nHight);

            if (nHight + nHight < iSizeY)
                halWarping_CopyOSDRect(eWDT_AFTER_WARP, 0, y, 0, y + nHight, iHorzRes, nHight);
            else
                halWarping_CopyOSDRect(eWDT_AFTER_WARP, 0, y, 0, y + nHight, iHorzRes, nYsize - nHight);
        }
        y += nYsize;
        // printf("y : %d \n", y);
    }

    dvC341Geo_Buffer_Flush();

    nDouble = 1;
    for (int nHight = 2 * iSizeY; nHight < iVertRes;
         nHight = 2 * iSizeY * nDouble)
    {
        // printf("Height : %d iHorzRes - nHight = %d \n", nHight,
        // iHorzRes - nHight);
        if (nHight + 2 * iSizeY * nDouble < iVertRes)
            halWarping_CopyOSDRect(eWDT_AFTER_WARP, 0, 0, 0, nHight, iHorzRes, 2 * iSizeY * nDouble);
        else
            halWarping_CopyOSDRect(eWDT_AFTER_WARP, 0, 0, 0, nHight, iHorzRes, iVertRes - nHight);

        nDouble *= 2;
    }


    return eHAL_WARPING_EXEC_CODE_PASS;

}


eHAL_WARPING_EXEC_CODE halAdvWarpControl(UINT8 ucValue)		//G100_Doulas_0027
{
    #if 0   //A65_OPTOMA_CL_0016 //A35G2_BRC_Casper_0139
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
    #endif

    //black level
    switch((eWARP_CTRL)ucValue)  //G100_Simon_0005
    {
        case WARP_CTRL__BASIC:
            halWarping_DBD_Blending_Enable(FALSE);
            break;

        case WARP_CTRL__ADVANCED:
            halWarping_DBD_Blending_Enable(FALSE);
            dvC341Geo_AdaptiveScaleFilterLutEnable(FALSE);
            break;

        case WARP_CTRL__AP:         //G100_Simon_0080
            #if (CMD_ONLY_BLACK_LEVEL == TRUE)  //A35G2_Simon_0088
            halWarping_BlackLevel_Enable(TRUE);  //A35G2_Simon_0093
            #endif
            break;

        default:
            halWarping_BlackLevel_Enable(FALSE);
            break;
    }

    BOOL bVal = utilWarp_SetWarpCtrl((eWARP_CTRL)ucValue);
	if(bVal)
	{
	//	SaveWarpConfig();//kenton_temp_check
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halAdvWarpGridPoints(UINT8 ucGridPorins, UINT8 ucInner)		//G100_Doulas_0027  //A35G2_BRC_Simon_0001
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetWarpPoint((eWARP_POINT)ucGridPorins);
    BOOL bInnerVal = utilWarp_SetWarpInner(ucInner);
	if(bVal || bInnerVal)
	{
		SaveWarpConfig();//kenton_temp_check
	}

	utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);


    return eHAL_WARPING_EXEC_CODE_PASS;
}

#if 1   //A35G2_BRC_Simon_0001 remove //A35G2_BRC_Casper_0051
eHAL_WARPING_EXEC_CODE halAdvWarpInnerGridPoints(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetWarpInner(ucValue);
	if(bVal)
	{
		SaveWarpConfig();//kenton_temp_check
	}

	utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif

eHAL_WARPING_EXEC_CODE halAdvWarpShowOsdPattern(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_ShowOsdPattern((ePAT_TYPE)ucValue);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

#if 0  //remove
eHAL_WARPING_EXEC_CODE halAdvWarpShowOsdPattern_Unblocked_TwistLink(UINT8 ucValue) //A65_OPTOMA_Julie_0050  //A35G2_CDS_Coda_0027
{
    utilWarp_SetApBlendApply(FALSE);  //A65_OPTOMA_CL_0010
    utilWarp_SetApBlacklevelApply(FALSE);  //A65_OPTOMA_CL_0010
	utilWarp_ShowOsdPattern((ePAT_TYPE)ucValue);

    INT8 ucWarpingApplySetting;  //A65_OPTOMA_CL_0010

    //AP need keep blending/blacklevel effect, so check blending/blacklevel has been applied.
    if(utilWarp_GetApBlendApply() || utilWarp_GetApBlacklevelApply())
    {
        LOG_MSG(db_HAL_WARPING, "%s, ===>apply blend = %d, apply blacklevel = %d\r\n", __FUNCTION__, utilWarp_GetApBlendApply(), utilWarp_GetApBlacklevelApply());
        utilWarp_SetApBlendApply(FALSE);
        utilWarp_SetApBlacklevelApply(FALSE);
        if(access(BLENDING_AP_DBD_TABLE_CURRENT_FILENAME, R_OK) == 0 ||
           access(BLENDING_AP_BKLEVEL_TABLE_CURRENT_FILENAME, R_OK) == 0 ||
           access(BLENDING_AP_BKLEVEL_PALETTE_CURRENT_FILENAME, R_OK) == 0 )
        {
            LOG_MSG(db_HAL_WARPING, "Apply Current AP setting of DBD Blending\n");
            //halWarping_MemoryApplyAP_CurrentSetting();
        }
        else
        {
            palDataMgr_Access_Warping_Apply(edaREAD, &ucWarpingApplySetting);
            halWarping_MemoryApplyAP_Blending(ucWarpingApplySetting);
            halWarping_MemoryApplyAP_BlackLevel(ucWarpingApplySetting);
        }
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif

eHAL_WARPING_EXEC_CODE halAdvWarpSelectControlPoint(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_SelectControlPoint((eDIR)ucValue);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpMoveControlPoint(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_MoveControlPoint((eDIR)ucValue);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpSelectOsdBlendWidth(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_SelectOsdBlendWidth((eDIR)ucValue);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpAdjustOsdBlendWidth(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	eADJ eADJVal;
	if((ucValue == DIR__UP) || (ucValue == DIR__RIGHT))
		eADJVal = ADJ__PLUS;
	else
		eADJVal = ADJ__MINUS;
	utilWarp_AdjustOsdBlendWidth(eADJVal);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpSharpness(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetOsdWarpSharpness(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		SaveWarpConfig();	//G100_Doulas_0029 Add
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGridColor(UINT8 ucValue, UINT8 ucRedrawPattern)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetOsdGridColor((eCOLOR_IDX)ucValue, ucRedrawPattern);
	if(bVal)
	{
		SaveWarpConfig();//kenton_temp_check
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpBackgroundColor(UINT8 ucValue, UINT8 ucRedrawPattern)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetOsdBkgColor((eBKG_COLOR)ucValue, ucRedrawPattern);
	if(bVal)
	{
		SaveWarpConfig();//kenton_temp_check
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingOverlapGridNumber(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetOverlapGridNum(ucValue);
	if(bVal)
	{
        utilWarp_ReDrawWarpPattern(); //A35G2_BRC_Casper_0047
		SaveWarpConfig();//kenton_temp_check
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingGamma(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_SetOsdBlendGamma(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		SaveWarpConfig();//kenton_temp_check
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpingInit(ePANEL_ID ucPanel)		//G100_Doulas_0027
{
	UINT8 ucNewPanel;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	if(ucAdvWarpingInit == FALSE)	//G100_Doulas_0063
	{
		ucAdvWarpingInit = TRUE;
	}
	switch(ucPanel)
	{
		case ePANEL_ID_WUXGA_60HZ:
			ucNewPanel = RES_ID__WUXGA;
			break;

		case ePANEL_ID_1080P_60HZ:
			ucNewPanel = RES_ID__1080P60;
			break;

		case ePANEL_ID_WUXGA_120HZ:
			ucNewPanel = RES_ID__WUXGA120;//RES_ID__WUXGA;	//G100_Doulas_0063 Modify
			break;

		case ePANEL_ID_1080P_120HZ:
			ucNewPanel = RES_ID__1080P120;
			break;

		case ePANEL_ID_WXGA_120HZ:			//R70G2_Doulas_0004
			ucNewPanel = RES_ID__WXGA120;
			break;

		case ePANEL_ID_3840x2400_60HZ:
		default:
			ucNewPanel = RES_ID__WQUXGA60;
			break;

		case ePANEL_ID_3840x2160_60HZ:
			ucNewPanel = RES_ID__UHD60;
			break;

		case ePANEL_ID_WUXGA_240HZ:
			ucNewPanel = RES_ID__WUXGA240;
			break;

		case ePANEL_ID_1080P_240HZ:
			ucNewPanel = RES_ID__1080P240;
			break;
	}
	halAdvWarpVstartOffsetSet(ucPanel);	//G100_Doulas_0043 Add
	utilWarp_SysInit(ucNewPanel,ucNewPanel);		//G100_Doulas_0064 Modify


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingLeft(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halAdvBlendingRight(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingTop(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingBottom(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halBasicWarpSettingSet(void)		//G100_Doulas_0027
{
	sWARP_BASIC stWarpingBasic;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(ucAdvWarpingInit == FALSE)	//G100_Doulas_0063
	{
		return eHAL_WARPING_EXEC_CODE_PASS;
	}

	memcpy(&(stWarpingBasic), &(m_sWArpGeoPara), sizeof(sWARP_BASIC));
    utilBasicWarpSettingSet(stWarpingBasic);

	//LOG_MSG(db_ALWAYS, "halBasicWarpSettingSet \r\n");
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halBasicBlendSettingSet(void)		//G100_Doulas_0027
{
	sBLENDING_BASIC stBlendingBasic;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	if(ucAdvWarpingInit == FALSE)	//G100_Doulas_0063
	{
		return eHAL_WARPING_EXEC_CODE_PASS;
	}

	memcpy(&(stBlendingBasic), &(m_sBlendingPara), sizeof(sBLENDING_BASIC));
    utilBasicBlendSettingSet(stBlendingBasic);

	//LOG_MSG(db_ALWAYS, "halBasicBlendSettingSet \r\n");
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halBasicWarpSettingGet(void)		//G100_Doulas_0027
{
	sWARP_BASIC stWarpingBasic;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilBasicWarpSettingGet(&stWarpingBasic);
   	memcpy(&(m_sWArpGeoPara),&(stWarpingBasic), sizeof(sWARP_BASIC));

	//LOG_MSG(db_ALWAYS, "halBasicWarpSettingGet \r\n");
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halBasicBlendSettingGet(void)		//G100_Doulas_0027
{
	sBLENDING_BASIC stBlendingBasic;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilBasicBlendSettingGet(&stBlendingBasic);
	memcpy(&(m_sBlendingPara),&(stBlendingBasic), sizeof(sBLENDING_BASIC));

	//LOG_MSG(db_ALWAYS, "halBasicWarpSettingGet \r\n");
    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halADVWarpSettingGet(OSD_WARP *stADVWarp)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
	utilWarp_GetADVWarpConfig(stADVWarp);

	//LOG_MSG(db_ALWAYS, "halADVWarpSettingGet \r\n");
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halADVWarpSettingSet(OSD_WARP stADVWarp)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

   	utilWarp_SetADVWarpConfig(stADVWarp);
	//LOG_MSG(db_ALWAYS, "halADVWarpSettingSet \r\n");

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halBasicSaveWarpMemory(UINT8 ucValue)
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    UINT8 ucdata;
	ucdata = utilWarp_SaveBasicWarpMemoryPreset(ucValue);
	if(ucdata != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);	//G100_Doulas_0047 Add
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}

    return eResult;
}


eHAL_WARPING_EXEC_CODE halBasicSaveBlendMemory(UINT8 ucValue)
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    UINT8 ucdata;
	ucdata = utilWarp_SaveBasicBlendMemoryPreset(ucValue);
	if(ucdata != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);	//G100_Doulas_0047 Add
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}

    return eResult;
}


eHAL_WARPING_EXEC_CODE halAdvSaveMemory(UINT8 ucValue)		//G100_Doulas_0027
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    LOG_MSG(db_HAL_WARPING, "halAdvSaveMemory %d \r\n", ucValue);

    UINT8 ucdata;
	ucdata = utilWarp_SaveMemoryPreset(ucValue);
	if(ucdata != FLASH_ACCESS__PASS)
	{
		LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);	//G100_Doulas_0047 Add
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}

    return eResult;
}

eHAL_WARPING_EXEC_CODE halBasicApplyWarpMemory(UINT8 ucValue)
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    UINT8 ucdata;
	UINT8 ucWarpmode;

	ucdata = utilWarp_LoadBasicWarpMemoryPreset(ucValue);

	if(ucdata == FLASH_ACCESS__PASS)
	{
		ucWarpmode = utilWarp_GetWarpCtrl();
		/*if(ucWarpmode == WARP_CTRL__BASIC)
		{
			halBasicWarpSettingGet();
			halWarp_GeometrySet();
			halWarp_BlendingSet();
		}*/
	}
	else
	{
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}

    return eResult;
}

eHAL_WARPING_EXEC_CODE halBasicApplyBlendMemory(UINT8 ucValue)
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }


    UINT8 ucdata;
	UINT8 ucWarpmode;

	ucdata = utilWarp_LoadBasicBlendMemoryPreset(ucValue);

	if(ucdata == FLASH_ACCESS__PASS)
	{
		ucWarpmode = utilWarp_GetWarpCtrl();
		/*if(ucWarpmode == WARP_CTRL__BASIC)
		{
			halBasicWarpSettingGet();
			halWarp_GeometrySet();
			halWarp_BlendingSet();
		}*/
	}
	else
	{
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvApplyMemory(UINT8 ucValue)		//G100_Doulas_0027
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    LOG_MSG(db_HAL_WARPING, "halAdvApplyMemory %d\r\n", ucValue);

    UINT8 ucdata;
	UINT8 ucWarpmode;

	ucdata = utilWarp_LoadMemoryPreset(ucValue);

	LOG_MSG(db_HAL_WARPING, "<%d>\r\n", ucdata);

	if(ucdata == FLASH_ACCESS__PASS)
	{
		ucWarpmode = utilWarp_GetWarpCtrl();
		/*if(ucWarpmode == WARP_CTRL__BASIC)
		{
			halBasicWarpSettingGet();
			halWarp_GeometrySet();
			halWarp_BlendingSet();
		}*/
	}
	else
	{
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvClearMemory(void)		//G100_Doulas_0027
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    UINT8 ucdata;
	ucdata = utilWarp_ClearOsdPreset();
	if(ucdata != FLASH_ACCESS__PASS)
	{
		eResult = eHAL_WARPING_EXEC_CODE_FAIL;
	}
	//LOG_MSG(db_ALWAYS, "halAdvClearMemory\r\n");

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvResetWarping(void)		//G100_Doulas_0027
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_ResetWarpConfig();
	//LOG_MSG(db_ALWAYS, "halAdvResetWarping\r\n");

    return eResult;
}

eHAL_WARPING_EXEC_CODE halTest(void)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	LOG_MSG(db_ALWAYS, "halTest1(%f,%f)(%f,%f)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d)(%d,%d,%d)\n", m_sWArpGeoPara.PM_WPKEYANG_H,m_sWArpGeoPara.PM_WPKEYANG_V,
												m_sWArpGeoPara.PM_WPPINWID_H,m_sWArpGeoPara.PM_WPPINWID_V,
												m_sWArpGeoPara.PM_WPKEY4C_TLX,m_sWArpGeoPara.PM_WPKEY4C_TLY,
												m_sWArpGeoPara.PM_WPKEY4C_TRX ,m_sWArpGeoPara.PM_WPKEY4C_TRY,
												m_sWArpGeoPara.PM_WPKEY4C_BLX,m_sWArpGeoPara.PM_WPKEY4C_BLY,
												m_sWArpGeoPara.PM_WPKEY4C_BRX ,m_sWArpGeoPara.PM_WPKEY4C_BRY,
												m_sWArpGeoPara.POLATION_H,m_sWArpGeoPara.POLATION_V,
												m_sWArpGeoPara.m_cWarpAutoFilter ,m_sWArpGeoPara.m_cWarpFilterSelect_H ,m_sWArpGeoPara.m_cWarpFilterSelect_V);


	LOG_MSG(db_ALWAYS, "halTest2 (%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,0x%x)  \r\n",
		m_sBlendingPara.ucBlending_T_Enable,m_sBlendingPara.ucBlending_B_Enable,m_sBlendingPara.ucBlending_L_Enable,m_sBlendingPara.ucBlending_R_Enable,
		m_sBlendingPara.uiBlending_T_St,    m_sBlendingPara.uiBlending_B_St,m_sBlendingPara.uiBlending_L_St,m_sBlendingPara.uiBlending_R_St,
		m_sBlendingPara.uiBlending_T_Width,m_sBlendingPara.uiBlending_B_Width,m_sBlendingPara.uiBlending_L_Width,m_sBlendingPara.uiBlending_R_Width,
		m_sBlendingPara.ucBlendingGamma,m_sBlendingPara.ulEGBCT);
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpVstartOffsetSet(ePANEL_ID ePanelTimingId)      //G100_Doulas_0043
{
    switch(ePanelTimingId)
    {
        case ePANEL_ID_WUXGA_60HZ:
            if(halScaler_PIP_PBP_Enable_Get())		//G100_Doulas_0045 Modify warning
            {
                //PS_PANEL_VST = OSD_V_START_WUGA_60 - 2;//20;//32;
                utilWarp_Panel_Vstart_Offset_Set(0);//(-2); //A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                //PS_PANEL_VST = OSD_V_START_WUGA_60;//20;//32;
                utilWarp_Panel_Vstart_Offset_Set(0);
            }
            break;

        case ePANEL_ID_1080P_60HZ:
            if(halScaler_PIP_PBP_Enable_Get())		//G100_Doulas_0045 Modify warning
            {
                //PS_PANEL_VST = OSD_V_START_1080P_60 - 2;//41;
                utilWarp_Panel_Vstart_Offset_Set(0);//(-2); //A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                //PS_PANEL_VST = OSD_V_START_1080P_60;//41;
                utilWarp_Panel_Vstart_Offset_Set(0);
            }
            break;

	 	case ePANEL_ID_WXGA_120HZ:		//R70G2_Doulas_0004
            if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            {
                utilWarp_Panel_Vstart_Offset_Set(0);
            }
            else
            {

                utilWarp_Panel_Vstart_Offset_Set(0);
            }
            break;

        case ePANEL_ID_1080P_120HZ:
	 	case ePANEL_ID_WUXGA_120HZ:
            if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)	//G100_Doulas_0045 Modify warning
            {
                //PS_PANEL_VST = OSD_V_START_1080P_120 + 2;//41;
                utilWarp_Panel_Vstart_Offset_Set(0);
            }
            else
            {
                //PS_PANEL_VST = OSD_V_START_1080P_120;//41;
                utilWarp_Panel_Vstart_Offset_Set(-2);
            }
            break;
        default:
            break;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//G100_Simon_0073
void halWarping_BurstWrite_FixedAdd(UINT32 ulCPUAddrReg, UINT32 ulCPUDataReg, UINT8 *ucData, UINT32 ulDataSize, UINT32 ulMaxBurstWriteSize)
{
#if 0
    if( ulDataSize <= ulMaxBurstWriteSize )
    {
        dvC341Geo_Write(B4_CPUWAD, ulCPUAddrReg);
        dvC341Geo_BurstWrite_FixedAdd(ulCPUDataReg, ulDataSize , (UINT8 *)ucData);
    }
    else
    {
        UINT32 ulBlockIndex = 0;
        UINT32 ulBlockSize  = ulMaxBurstWriteSize;
        UINT32 ulOffset = 0;
        UINT32 ulRemainSize = ulDataSize ;

        LOG_MSG(db_HAL_WARPING, "[%d %d]\n", ulMaxBurstWriteSize, ulRemainSize);

        while(ulRemainSize > 0)
        {
            ulOffset = ulBlockIndex * ulBlockSize ;

            LOG_MSG(db_HAL_WARPING, "b<0x%08X><%d>\n", ulCPUAddrReg + ulOffset, *((UINT8 *)ucData + ulOffset));

            dvC341Geo_Write(B4_CPUWAD, ulCPUAddrReg + ulOffset);

            if(ulRemainSize >= ulMaxBurstWriteSize)
            {
                //dvC341Geo_BurstWrite_FixedAdd(ulCPUDataReg, ulMaxBurstWriteSize, (UINT8 *)ucData + ulOffset);
                for(int index = 0 ; index < ulMaxBurstWriteSize; index++)
                {
                    dvC341Geo_WriteToBuffer(B4_CPUDT, ucData[index + ulOffset]);
                }
                ulRemainSize = ulRemainSize - ulMaxBurstWriteSize;
            }
            else   //last block
            {
                //dvC341Geo_BurstWrite_FixedAdd(ulCPUDataReg, ulRemainSize , (UINT8 *)ucData + ulOffset);
                for(int index = 0 ; index < ulRemainSize; index++)
                {
                    dvC341Geo_WriteToBuffer(B4_CPUDT, ucData[index + ulOffset]);
                }
                ulRemainSize = 0;
                break;
            }

            ulBlockIndex++;

            dvC341Geo_Buffer_Flush();

        }
    }
#endif /* 0 */

}

//A65_OPTOMA_CL_0020
//use scaler ofill?
eHAL_WARPING_EXEC_CODE halWarping_AP_Ifill(INT16 iEnable, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) enable = %d, (r,g,b) = (%d,%d,%d)\n", __FUNCTION__, __LINE__, iEnable, ucRed, ucGreen, ucBlue);


    #if 0  //using scaler block OFILL , but can not display when no input source
    static UINT32 Reg_OIMGCT = 0;
    static UINT32 Reg_OSDMODE = 0;
    static UINT8 PreEnable = 0;

    if(iEnable == 0)
    {
        if(PreEnable == 1)
        {
        	dvC341_Write(B8_OIMGCTCH1,    Reg_OIMGCT, 0);
        	dvC341_Write(B8_OIMGCTCH1,    Reg_OIMGCT, CH_BANK_OFFSET);

        	dvC341_Write(B9_OSDMODE1CH1,   Reg_OSDMODE, 0);
        	dvC341_Write(B9_OSDMODE1CH1,   Reg_OSDMODE, CH_BANK_OFFSET);

        	PreEnable = 0;
        }
    }
    else
    {
        if(PreEnable == 0)
        {
            Reg_OIMGCT = dvC341_Read(B8_OIMGCTCH1, 0);
            Reg_OSDMODE = dvC341_Read(B9_OSDMODE1CH1, 0);
            PreEnable = 1;
        }

    	dvC341_Write(0x00084901,    (UINT32)ucRed, 0);//R
    	dvC341_Write(0x00084A01,    (UINT32)ucGreen, 0);//G
    	dvC341_Write(0x00084B01,    (UINT32)ucBlue, 0);//B
    	dvC341_Write(0x00084901,    (UINT32)ucRed, CH_BANK_OFFSET);//R
    	dvC341_Write(0x00084A01,    (UINT32)ucGreen, CH_BANK_OFFSET);//G
    	dvC341_Write(0x00084B01,    (UINT32)ucBlue, CH_BANK_OFFSET);//B

    	dvC341_Write(B8_OIMGCTCH1,    Reg_OIMGCT | 0x01, 0);
    	dvC341_Write(B8_OIMGCTCH1,    Reg_OIMGCT | 0x01, CH_BANK_OFFSET);

    	dvC341_Write(B9_OSDMODE1CH1,    0, 0);
    	dvC341_Write(B9_OSDMODE1CH1,    0, CH_BANK_OFFSET);

    }
    #endif

	eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return  eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_AP_DrawRect(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    LOG_MSG(db_HAL_WARPING, "==> %d %d %d %d %d %d\n" , eType
                                                      , iPositionX
                                                      , iPositionY
                                                      , iWidth
                                                      , iHeight
                                                      , iColorIndex);

#if 1  //C341 wait review first
    if( eType >= eWDT_INVALID ||
        iPositionX < 0 ||
        iPositionY < 0 ||
        iPositionX > m_sHalWarpingInfo.uiWarp_HW ||
        iPositionY > m_sHalWarpingInfo.uiWarp_VW ||
        iWidth  > m_sHalWarpingInfo.uiWarp_HW ||
        iHeight > m_sHalWarpingInfo.uiWarp_VW ||
        iColorIndex < 0 || iColorIndex > 255
    )
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) Error %d %d %d %d %d %d\n", __FUNCTION__
    	                                                           , __LINE__
    	                                                           , eType
    	                                                           , iPositionX
    	                                                           , iPositionY
    	                                                           , iWidth
    	                                                           , iHeight
    	                                                           , iColorIndex);

        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
        return eResult;
    }

    if((m_sHalWarpingInfo.ucFreeze == FALSE) && (eType == eWDT_BEFORE_WARP))
    {
        halWarping_FreezeImage(TRUE);
    }

    if(eType == eWDT_BEFORE_WARP)   //insert warp OSD before warp
    {
        int x=0;
        int y=0;
        int cpuwad = 0, offset = 0;
        int cpuwad_previous = -1;
        UINT8 Is30BitProcess = dvc341Geo_IsFrameMemoryProc30Bits();  //H2PF_Simon_0131
        UINT32 WPISFLD = dvC341Geo_FrameMemoryDrawStartAddrGet();


        halwarp_ResetTimeInState();
        UINT32 ulBOSTAT;
        do
        {
            ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

            if(halwarp_TimeElapsedInState() >= 1000)
            {
                break;
            }
        }while((ulBOSTAT & 0x01) == 0x01);
        dvC341_Write(B4_OSDCT, 0x00, 0);   //H2PF_Simon_0151

        for(y = iPositionY ; y < (iPositionY + iHeight) ; y++)
        {
            for(x = iPositionX ; x < (iPositionX + iWidth) ; x++)
            {
                //#ifdef DEF_WARPBLOCK_24BIT_PROCESS

                //24bit for BMP format
                if(Is30BitProcess == FALSE)  //H2PF_Simon_0131
                {
                    cpuwad = WPISFLD + 2*32768*(y/4) + 64*(y%4) + 2*128*(x/20) + 16*(x%20)/5;
                    if( (cpuwad) != (cpuwad_previous+3) )  //24bit
                    {
                        dvC341_WriteToBuffer(B4_CPUWAD, cpuwad, 0);

                        //LOG_MSG(db_HAL_WARPING, ">%d %d 0x%X\n", x, y, cpuwad );

                        //24bit for BMP format set cpuwad when..
                        //x0 y0
                        //x5 y0
                        //x10 y0
                        //x15 y0
                        //x20 y0
                    }

                    dvC341_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][0], 0); //R
                    dvC341_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][1], 0); //G
                    dvC341_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][2], 0); //B

                }
                else
                {
                    //30bit for BMP format
                    cpuwad = WPISFLD + 2*32768*(y/4) + 64*(y%4) + 2*128*(x/16) + 4*(x%16);
                    if( (cpuwad) != (cpuwad_previous+3) )  //30bit
                    {
                        dvC341_WriteToBuffer(B4_CPUWAD, cpuwad | (BIT31), 0);

                        //30bit for BMP format set cpuwad when...
                        //x0 y0
                        //x16 y0
                        //x32 y0
                        //x48 y0
                        //x64 y0
                    }

                    dvC341_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][2], 0); //B
                    dvC341_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][1], 0); //G
                    dvC341_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][0], 0); //R
                }

                cpuwad_previous = cpuwad;


                //dvC341_WriteToBuffer(B4_CPUWAD, 0x00, 0);
            }
        }

        dvC341Geo_Buffer_Flush();

    }
    else   //insert warp OSD after warp
    {
        UINT8 ucFastWrite = 0;
        dvC341Geo_SetOSDMode(FALSE);

        //if OSD 2X
        if(dvC341Geo_IsMagnifyOSD2X())
        {
            iPositionX = iPositionX / 2;
            iPositionY = iPositionY / 2;
            iWidth  = (iWidth  + 1) / 2 ;
            iHeight = (iHeight + 1) / 2 ;
        }

        int cpuwad;
        int GV_RTCT_BAK = dvC341Geo_Read(B0_RTCT6, 0); //0x0105;

        //Cindy: change RTCT to avoid blending not shown
        int GV_RTCT = 0xFF;
        cpuwad = m_OSD_CURRENT_LAYER + iPositionX + (DEF_OSD_PANEL_WIDTH/128) * 128 * iPositionY;

#ifdef OSDCT_FASTEN_ON
        if (iWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
            iWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0 &&
            iPositionX % OSDCT_FASTEN_TRANSFER_UNIT == 0)
        {
            iWidth = iWidth / OSDCT_FASTEN_TRANSFER_UNIT;
            ucFastWrite = 1;
        }
#endif

        dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
        dvC341Geo_WriteToBuffer(B4_OSDFILL, iColorIndex, 0);
        dvC341Geo_WriteToBuffer(B4_BBACTHW, (iWidth -1)&0xfff, 0);
        dvC341Geo_WriteToBuffer(B4_BBACTVW, (iHeight-1)&0xfff, 0);
        dvC341Geo_WriteToBuffer(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH / OSD_LINEFEED_BASE), 0);
        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad & CPUWAD_MASK, 0);
        if(ucFastWrite)
        {
            dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
        }
        else
        {
            dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
        }
        GV_RTCT = 0;
        dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
        dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT_BAK, 0);
        dvC341Geo_Buffer_Flush();

        do { } while ( (dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) == 0x01 );

    }
#endif /* 0 */

	eResult = eHAL_WARPING_EXEC_CODE_PASS;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);

	return eResult;
}


//draw on memory by 24bit color (0xRRGGBB)
eHAL_WARPING_EXEC_CODE halWarping_AP_DrawBox(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iRed, INT16 iGreen, INT16 iBlue)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    LOG_MSG(db_HAL_WARPING, "==> %d %d %d %d %d - %d %d %d\n" , eType
                                                              , iPositionX
                                                              , iPositionY
                                                              , iWidth
                                                              , iHeight
                                                              , iRed
                                                              , iGreen
                                                              , iBlue);

#if 0 //C341 wait review first
    if( eType != eWDT_BEFORE_WARP ||
        iPositionX < 0 ||
        iPositionY < 0 ||
        iPositionX > m_sHalWarpingInfo.uiWarp_HW ||
        iPositionY > m_sHalWarpingInfo.uiWarp_VW ||
        iWidth  > m_sHalWarpingInfo.uiWarp_HW ||
        iHeight > m_sHalWarpingInfo.uiWarp_VW ||
        iRed < 0   || iRed > 255 ||
        iGreen < 0 || iGreen > 255 ||
        iBlue < 0  || iBlue > 255
    )
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) Error Box data\n", __FUNCTION__ , __LINE__);

        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
        return eResult;
    }

    if((m_sHalWarpingInfo.ucFreeze == FALSE) && (eType == eWDT_BEFORE_WARP))
    {
        halWarping_FreezeImage(TRUE);
    }

    if(eType == eWDT_BEFORE_WARP)   //insert warp OSD before warp  //draw on memory
    {
        int x=0;
        int y=0;
        int cpuwad = 0, offset = 0;
        int cpuwad_previous = -1;

        for(x = iPositionX ; x < (iPositionX + iWidth) ; x++)
        {
            for(y = iPositionY ; y < (iPositionY + iHeight) ; y++)
            {
                for(int idx = 0 ; idx < 3 ; idx++)
                {
                    if(x%12 < 6)
                    {
                        offset=x%12;
                    }

                    if(x%12 >= 6)
                    {
                        offset=x%12+2;
                    }

                    cpuwad_previous = cpuwad;
                    cpuwad = (y/4)*(2-0)*32768+(y%4)*16+(x/12)*256+offset;
                    cpuwad = cpuwad + 64 * idx;

                    if(cpuwad != cpuwad_previous + 1) //not continuous
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad);
                    }

                    if(idx == 0)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, iRed);
                    }
                    else if(idx == 1)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, iGreen);
                    }
                    else if(idx == 2)
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUDT, iBlue);
                    }
                }
            }
        }

        dvC341Geo_Buffer_Flush();

    }
#endif /* 0 */

	eResult = eHAL_WARPING_EXEC_CODE_PASS;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);

	return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_AP_DrawLine(eWARPOSD_DRAW_TYPE eType, INT16 iLeft, INT16 iTop, INT16 iRight, INT16 iBottom, INT32 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT32 cpuwad = 0, offset = 0;
    UINT32 cpuwad_previous = 0;
    UINT16 x,y = 0;


	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    LOG_MSG(db_HAL_WARPING, "line data %d %d %d %d %d 0x%X", eType
                                                           , iLeft
                                                           , iTop
                                                           , iRight
                                                           , iBottom
                                                           , iColorIndex
                                                          );

    if(eType   >= eWDT_INVALID ||
       iLeft   < 0 ||
       iTop    < 0 ||
       iRight  >= halWarping_HResGet() ||
       iBottom >= halWarping_VResGet()
    )
    {
        LOG_MSG(db_HAL_WARPING, "error line data eType:%d Left:%d Top:%d Right:%d Bottom:%d Color:%d", eType
                                                                                                     , iLeft
                                                                                                     , iTop
                                                                                                     , iRight
                                                                                                     , iBottom
                                                                                                     , iColorIndex
                                                                                                    );
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    if(m_sHalWarpingInfo.ucFreeze == FALSE)
    {
        halWarping_FreezeImage(TRUE);
    }

    if(eType == eWDT_BEFORE_WARP)   //insert warp OSD before warp
    {
        UINT32 WPISFLD = dvC341Geo_FrameMemoryDrawStartAddrGet();
        UINT8 Is30BitProcess = dvc341Geo_IsFrameMemoryProc30Bits();  //H2PF_Simon_0131

        for(y=iTop; y<=iBottom; y++)
        {
            x = iLeft;

            for(x=iLeft; x<=iRight; x++)
            {
                //#ifdef DEF_WARPBLOCK_24BIT_PROCESS

                //24bit for BMP format
                if(Is30BitProcess == FALSE)  //H2PF_Simon_0131
                {
                    cpuwad = WPISFLD + 2*32768*(y/4) + 64*(y%4) + 2*128*(x/20) + 16*(x%20)/5;
                    if( (cpuwad) != (cpuwad_previous+3) )  //24bit
                    {
                        dvC341_WriteToBuffer(B4_CPUWAD, cpuwad, 0);

                        //24bit for BMP format set cpuwad when..
                        //x0 y0
                        //x5 y0
                        //x10 y0
                        //x15 y0
                        //x20 y0
                    }

                    dvC341_WriteToBuffer(B4_CPUDT, (iColorIndex >> 16 ) & 0xFF, 0);  //R
                    dvC341_WriteToBuffer(B4_CPUDT, (iColorIndex >> 8 ) & 0xFF, 0);   //G
                    dvC341_WriteToBuffer(B4_CPUDT, (iColorIndex >> 0) & 0xFF, 0);    //B
                }
                else
                {
                    //30bit for BMP format
                    cpuwad = WPISFLD + 2*32768*(y/4) + 64*(y%4) + 2*128*(x/16) + 4*(x%16);
                    if( (cpuwad) != (cpuwad_previous+3) )  //30bit
                    {
                        dvC341_WriteToBuffer(B4_CPUWAD, cpuwad | (BIT31), 0);

                        //30bit for BMP format set cpuwad when...
                        //x0 y0
                        //x16 y0
                        //x32 y0
                        //x48 y0
                        //x64 y0
                    }

                    dvC341_WriteToBuffer(B4_CPUDT, (iColorIndex >> 0 ) & 0xFF, 0);  //B
                    dvC341_WriteToBuffer(B4_CPUDT, (iColorIndex >> 8 ) & 0xFF, 0);  //G
                    dvC341_WriteToBuffer(B4_CPUDT, (iColorIndex >> 16) & 0xFF, 0);  //R
                }

                cpuwad_previous = cpuwad;

                //dvC341_WriteToBuffer(B4_CPUWAD, 0x00, 0);
            }
        }

        dvC341Geo_Buffer_Flush();

    }
    else   //insert warp OSD after warp
    {
        dvC341Geo_SetOSDMode(FALSE);
    }

	eResult = eHAL_WARPING_EXEC_CODE_PASS;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);

	return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawRect_WriteByCPUDT(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

#if 0  //unused
	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    if( eType >= eWDT_INVALID ||
        iPositionX < 0 ||
        iPositionY < 0 ||
        iPositionX > m_sHalWarpingInfo.uiWarp_HW ||
        iPositionY > m_sHalWarpingInfo.uiWarp_VW ||
        iWidth  > m_sHalWarpingInfo.uiWarp_HW ||
        iHeight > m_sHalWarpingInfo.uiWarp_VW ||
        iColorIndex > 255
    )
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) Error %d %d %d %d %d %d\n", __FUNCTION__
    	                                                           , __LINE__
    	                                                           , eType
    	                                                           , iPositionX
    	                                                           , iPositionY
    	                                                           , iWidth
    	                                                           , iHeight
    	                                                           , iColorIndex);

        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
        return eResult;
    }


    if(m_sHalWarpingInfo.ucFreeze == FALSE)
    {
        halWarping_FreezeImage(TRUE);
    }

    if(eType == eWDT_BEFORE_WARP)   //insert warp OSD before warp
    {
        int x=0;
        int y=0;
        int cpuwad = 0, offset = 0;
        int cpuwad_previous = -1;

        for(x = iPositionX ; x < (iPositionX + iWidth) ; x++)
        {
            for(y = iPositionY ; y < (iPositionY + iHeight) ; y++)
            {
                for(int idx = 0 ; idx < 3 ; idx++)
                {
                    if(x%12 < 6)
                    {
                        offset=x%12;
                    }

                    if(x%12 >= 6)
                    {
                        offset=x%12+2;
                    }

                    cpuwad_previous = cpuwad;
                    cpuwad = (y/4)*(2-0)*32768+(y%4)*16+(x/12)*256+offset;
                    cpuwad = cpuwad + 64 * idx;

                    if(cpuwad != cpuwad_previous + 1) //not continuous
                    {
                        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad);
                    }

                    dvC341Geo_WriteToBuffer(B4_CPUDT, m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[iColorIndex][idx]);
                }
            }
        }
        dvC341Geo_Buffer_Flush();

    }
    else   //insert warp OSD after warp
    {
        dvC341Geo_SetOSDMode(FALSE);

        dvC341Geo_WriteToBuffer(BN_RTCT, 0x0105);     // 0x0105 is the initial value of RTCT

        dvC341Geo_WriteToBuffer(B1_OSDMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);
        dvC341Geo_WriteToBuffer(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH / OSD_LINEFEED_BASE) & 0xff);

        for (UINT32 y = iPositionY; y < iPositionY + iHeight; y++)
        {
            UINT32 cpuwad = m_OSD_CURRENT_LAYER + iPositionX + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * y;

            dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad);

            for (UINT32 x = iPositionX; x < iPositionX + iWidth; x++)
            {
                dvC341Geo_WriteToBuffer(B4_CPUDT, iColorIndex);
            }
        }

        dvC341Geo_Buffer_Flush();
    }
#endif /* 0 */

	eResult = eHAL_WARPING_EXEC_CODE_PASS;

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);

	return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_FreezeImage(UINT8 ucEnable)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    m_sHalWarpingInfo.ucFreeze = ucEnable;

    LOG_MSG(db_HAL_WARPING, "(%s@%d) %d\n", __FUNCTION__, __LINE__, ucEnable);

    if(m_sHalWarpingInfo.ucFreeze)
    {
    	MS_SLEEP(100); // 800 ms, wait signal stable

    	dvC341Geo_Freeze(TRUE);
        dvC341Geo_Buffer_Flush();

        MS_SLEEP(100);
	}
	else //unfreeze
	{
    	dvC341Geo_Freeze(FALSE);  //H2PF_Simon_0131
	}

    dvC341Geo_Buffer_Flush();

	return eHAL_WARPING_EXEC_CODE_PASS;
}


UINT8 halWarping_GetFreezeImageState(void)
{
    return m_sHalWarpingInfo.ucFreeze;
}

eHAL_WARPING_EXEC_CODE halWarping_OSD_Swap_Memplane(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    UINT8 ucFastWrite = 0;
    UINT16 uiWidth = DEF_OSD_PANEL_WIDTH;
    LOG_MSG(db_HAL_WARPING, "%s 0x%X\n", __FUNCTION__, m_OSD_CURRENT_LAYER );

    m_OSD_CURRENT_LAYER = ( m_OSD_CURRENT_LAYER != m_OSD_MEM_LAYER0 ) ? m_OSD_MEM_LAYER0 : m_OSD_MEM_LAYER1;

    LOG_MSG(db_HAL_WARPING, "%s 0x%X\n", __FUNCTION__, m_OSD_CURRENT_LAYER );

    //dvC341Geo_Write(BN_RTCT, 0x0105);     // 0x0105 is the initial value of RTCT     //C341 wait review

    INT32 lTransparentColor = dvC341Geo_Read(B9_BOTRANS01CH1, 0);

	int cpuwad = m_OSD_CURRENT_LAYER;

	int GV_RTCT_BAK = dvC341Geo_Read(B0_RTCT6, 0); //0x0105;

	//Cindy: change RTCT to avoid blending not shown
	int GV_RTCT = 0;  //0x0105; //0x0105; //C341 wait review
	//GV_RTCT = GV_RTCT | 0x0007;  //C341 wait review

#ifdef OSDCT_FASTEN_ON
    if (uiWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        uiWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0 )
    {
        uiWidth = uiWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
#endif

	dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
	dvC341Geo_WriteToBuffer(B4_OSDFILL, lTransparentColor, 0);
	dvC341Geo_WriteToBuffer(B4_BBACTHW, (uiWidth-1)&0xfff, 0);
	dvC341Geo_WriteToBuffer(B4_BBACTVW, (DEF_OSD_PANEL_HEIGHT-1)&0xfff, 0);
	dvC341Geo_WriteToBuffer(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH/OSD_LINEFEED_BASE), 0);
	dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad&CPUWAD_MASK, 0);
    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x11, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x01, 0);
    }
	//GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005; //C341 wait review
	dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
	dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT_BAK, 0);
	dvC341Geo_Buffer_Flush();

	do { } while ((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) == 0x01 );

    dvC341Geo_WriteToBuffer(B4_OSDCT, 0x00, 0);

    //GV_RTCT = GV_RTCT | 0x0007; //C341 wait review
    dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
    dvC341Geo_WriteToBuffer(B4_OSDCT, 0x00, 0);
    //dvC341Geo_WriteToBuffer( B9_OSDMODE2CH1, 0x07 );  //0x03:0x07 //A35G2_CDS_Simon_0044
    //GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005; //C341 wait review
    dvC341Geo_WriteToBuffer(B0_RTCT6, GV_RTCT, 0);
	dvC341Geo_Buffer_Flush();

    eResult = eHAL_WARPING_EXEC_CODE_PASS;

    return eResult;

}


eHAL_WARPING_EXEC_CODE halWarping_OSD_ON(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;  //A65_OPTOMA_Simon_0001

    dvC341_Write(B9_OSDSAD1CH1, m_OSD_CURRENT_LAYER, 0);
    dvC341_Write(B9_OSDSAD1CH1, m_OSD_CURRENT_LAYER + (DEF_OSD_PANEL_WIDTH/OUTPUT_CH_NUM), CH_BANK_OFFSET);

    //OSD 1X
    if(m_sHalWarpingInfo.uiWarp_HW == DEF_OSD_PANEL_WIDTH)
    {
        dvC341_Write(B9_OSDMODE1CH1, (BIT14|BIT1|BIT0), 0);  //H2PF_Simon_0153
        dvC341_Write(B9_OSDMODE1CH1, (BIT14|BIT1|BIT0), CH_BANK_OFFSET);
    }
    else //OSD 2X
    {
        dvC341_Write(B9_OSDMODE1CH1, (BIT14|BIT12|BIT11|BIT1|BIT0), 0);  //H2PF_Simon_0153
        dvC341_Write(B9_OSDMODE1CH1, (BIT14|BIT12|BIT11|BIT1|BIT0), CH_BANK_OFFSET);
    }

    eResult = halWarping_OSD_Swap_Memplane();

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_OSD_PalettePrint(void)
{
    UINT8 aucWarpOSD_Pallette[768+3+3]; //add 2 blink color
    memset(aucWarpOSD_Pallette, 0, sizeof(aucWarpOSD_Pallette));

    dvC341Geo_Write(B9_PLTAD2CH1, 0, 0);
    dvC341Geo_BurstRead_FixedAdd(B9_PLTDT2CH1, 768 + 3 + 3, aucWarpOSD_Pallette); //add 2 blink color

    for(int i=0 ; i<256 + 2 ; i++)
    {
        LOG_MSG(db_HAL_WARPING, "OSD1 %3d (%3d %3d %3d)\n", i, aucWarpOSD_Pallette[i*3 + 0], aucWarpOSD_Pallette[i*3 + 1], aucWarpOSD_Pallette[i*3 + 2]);
        MS_SLEEP(3);
    }

    dvC341Geo_Write(B9_PLTAD1CH1, 0, 0);
    dvC341Geo_BurstRead_FixedAdd(B9_PLTDT1CH1, 768 + 3 + 3, aucWarpOSD_Pallette); //add 2 blink color

    for(int i=0 ; i<256 + 2 ; i++)
    {
        LOG_MSG(db_HAL_WARPING, "OSD2 %3d (%3d %3d %3d)\n", i, aucWarpOSD_Pallette[i*3 + 0], aucWarpOSD_Pallette[i*3 + 1], aucWarpOSD_Pallette[i*3 + 2]);
        MS_SLEEP(3);
    }

    UINT8 aucWarpOSD_TranparentColor[4];
    memset(aucWarpOSD_TranparentColor, 0, sizeof(aucWarpOSD_TranparentColor));
    aucWarpOSD_TranparentColor[0] = dvC341Geo_Read(B9_BOTRANS02CH1, 0);
    aucWarpOSD_TranparentColor[1] = dvC341Geo_Read(B9_BOTRANS12CH1, 0);
    aucWarpOSD_TranparentColor[2] = dvC341Geo_Read(B9_BOTRANS22CH1, 0);
    aucWarpOSD_TranparentColor[3] = dvC341Geo_Read(B9_BOTRANS32CH1, 0);
    LOG_MSG(db_HAL_WARPING, "OSD1 Transparent %d %d %d %d\n\n", aucWarpOSD_TranparentColor[0], aucWarpOSD_TranparentColor[1], aucWarpOSD_TranparentColor[2], aucWarpOSD_TranparentColor[3]);
    MS_SLEEP(3);
    aucWarpOSD_TranparentColor[0] = dvC341Geo_Read(B9_BOTRANS01CH1, 0);
    aucWarpOSD_TranparentColor[1] = dvC341Geo_Read(B9_BOTRANS11CH1, 0);
    aucWarpOSD_TranparentColor[2] = dvC341Geo_Read(B9_BOTRANS21CH1, 0);
    aucWarpOSD_TranparentColor[3] = dvC341Geo_Read(B9_BOTRANS31CH1, 0);
    LOG_MSG(db_HAL_WARPING, "OSD2 Transparent %d %d %d %d\n\n", aucWarpOSD_TranparentColor[0], aucWarpOSD_TranparentColor[1], aucWarpOSD_TranparentColor[2], aucWarpOSD_TranparentColor[3]);
    MS_SLEEP(3);
    LOG_MSG(db_HAL_WARPING, "m_OSD_CURRENT_LAYER => 0x%X\n\n", m_OSD_CURRENT_LAYER);
    LOG_MSG(db_HAL_WARPING, "CPU OSDSAD => 0x%X\n\n", dvC341Geo_Read(B9_OSDSAD1CH1, 0));
    LOG_MSG(db_HAL_WARPING, "B9_OSDMODE2CH1 => 0x%X\n\n", dvC341Geo_Read(B9_OSDMODE1CH1, 0));
    LOG_MSG(db_HAL_WARPING, "GetWarpCtrl %d\n\n", utilWarp_GetWarpCtrl());


    LOG_MSG(db_HAL_WARPING, "DEF_OSDSAD_LAYER0_4K 0x%08X\n\n", DEF_OSDSAD_LAYER0_4K);
    LOG_MSG(db_HAL_WARPING, "DEF_OSDSAD_LAYER1_4K 0x%08X\n\n", DEF_OSDSAD_LAYER1_4K);
    LOG_MSG(db_HAL_WARPING, "DEF_OSDSAD_LAYER3_4K 0x%08X\n\n", DEF_OSDSAD_LAYER3_4K);
    LOG_MSG(db_HAL_WARPING, "DEF_OSDSAD_LAYER4_4K 0x%08X\n\n", DEF_OSDSAD_LAYER4_4K);
    LOG_MSG(db_HAL_WARPING, "DEF_BITMAPAD_4K 0x%08X\n\n", DEF_BITMAPAD_4K);
    LOG_MSG(db_HAL_WARPING, "DEF_FONTAD_4K 0x%08X\n\n", DEF_FONTAD_4K);
    LOG_MSG(db_HAL_WARPING, "DEF_GEO_FONTAD_4K 0x%08X\n\n", DEF_GEO_FONTAD_4K);


    utilWarp_PrintWarpConfig();

    return eHAL_WARPING_EXEC_CODE_PASS;
}


//C341 wait review first
eHAL_WARPING_EXEC_CODE halWarping_DrawCircle(eWARPOSD_DRAW_TYPE eType, INT16 iX, INT16 iY, INT16 iRadius, UINT16 uiCircleColorIndex, UINT16 uiBGColorIndex)  //R70G2_Simon_0001
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d %d)\n", __FUNCTION__, eType, iX, iY, iRadius, uiCircleColorIndex, uiBGColorIndex);

	INT16 iHRes = halWarping_HResGet();
	INT16 iVRes = halWarping_VResGet();

    if(iRadius > iVRes/2)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    UINT8 *aucCircle = (UINT8 *)malloc(iHRes * iVRes);
    if(aucCircle == NULL)
    {
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    memset(aucCircle, uiBGColorIndex, (iHRes * iVRes));

#if 0
    if(eType == eWDT_BEFORE_WARP)
    {
        halWarping_WarpOSDModeSet(TRUE);
    }
    else if(eType == eWDT_AFTER_WARP)
    {
        halWarping_WarpOSDModeSet(FALSE);
    }
    else
    {
        free(aucCircle);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }
#endif

    halWarping_WarpOSDModeSet(FALSE);

	INT32 iCenterX = (INT32)iX;
	INT32 iCenterY = (INT32)iY;

	INT32 iStartX, iEndX;
	INT32 iStartY, iEndY;
	iStartX = ((iCenterX-iRadius)>0)      ? (iCenterX-iRadius-1) : 0;
	iEndX   = ((iCenterX+iRadius)<iHRes)  ? (iCenterX+iRadius+1) : iHRes;   //R70G2_Simon_0007
	iStartY = ((iCenterY-iRadius)>0)      ? (iCenterY-iRadius-1) : 0;
	iEndY   = ((iCenterY+iRadius)<iVRes)  ? (iCenterY+iRadius+1) : iVRes;   //R70G2_Simon_0007

    LOG_MSG(db_HAL_WARPING, "(%d %d %d %d)\n", iStartX, iEndX, iStartY, iEndY);

	for(INT32 y = iStartY; y < iEndY; y++)
	{
		for(INT32 x = iStartX; x < iEndX; x++)
		{
			if((x-iCenterX)*(x-iCenterX)+(y-iCenterY)*(y-iCenterY) <= (INT32)((INT32)iRadius*iRadius))
			{
				aucCircle[ x + y*iHRes ] = uiCircleColorIndex;
			}
			else
			{
				aucCircle[ x + y*iHRes ] = uiBGColorIndex;
			}
		}
	}


    INT32 cpuwad, lineStart;

    for(INT32 j=0; j<iVRes; j++)
    {
        cpuwad = m_OSD_CURRENT_LAYER + 0 + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * j;    //R70G2_Simon_0002
        lineStart = cpuwad;
        dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad&0x07ffffff, 0);

        //Write first byte
        if(aucCircle[0 + j*iHRes] == uiCircleColorIndex)   //R70G2_Simon_0007
        {
            dvC341Geo_WriteToBuffer(B4_CPUDT, aucCircle[0 + j*iHRes], 0);    //R70G2_Simon_0002
        }

        for(INT32 k = 1 ; k < iHRes ; k++)
        {
            if(aucCircle[k + j*iHRes] == uiCircleColorIndex)  //skip gain=1 0xff
            {
                cpuwad = lineStart + k;

                if(aucCircle[k-1 + j*iHRes] != uiCircleColorIndex)   //if it is a start, then set address
                    dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad&0x07ffffff, 0);

                dvC341Geo_WriteToBuffer(B4_CPUDT, aucCircle[k + j*iHRes], 0);
            }
        }
    }
    dvC341Geo_Buffer_Flush();

    free(aucCircle);

    return eResult;
}


//G100_Simon_0079
eHAL_WARPING_EXEC_CODE halWarping_CopyOSDRect(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                       INT16 ixSource,
                                                       INT16 iySource,
                                                       INT16 ixDestination,
                                                       INT16 iyDestination,
                                                       INT16 iWidth,
                                                       INT16 iHeight)
{
    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d %d %d)\n", __FUNCTION__
                                                        , ucLayerMode
                                                        , ixSource
                                                        , iySource
                                                        , ixDestination
                                                        , iyDestination
                                                        , iWidth
                                                        , iHeight);


    int cpurad = m_OSD_CURRENT_LAYER + ixSource + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iySource;     //R70G2_Simon_0002
    int cpuwad = m_OSD_CURRENT_LAYER + ixDestination + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iyDestination;
    //LOG_MSG(db_HAL_WARPING, "cpurad 0x%X wad 0x%X\n", cpurad, cpuwad);
    UINT8 ucFastWrite = 0;
    UINT16 uiWidth = iWidth/OUTPUT_CH_NUM;
#ifdef OSDCT_FASTEN_ON
    if (uiWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        uiWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0 &&
        ixSource % OSDCT_FASTEN_TRANSFER_UNIT == 0 &&
        ixDestination % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        uiWidth = uiWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
#endif

    dvC341Geo_WriteToBuffer(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH/OSD_LINEFEED_BASE) & 0xff, 0);
    dvC341Geo_WriteToBuffer(B4_BBRMWI, (DEF_OSD_PANEL_WIDTH/OSD_LINEFEED_BASE) & 0xff, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTHW, uiWidth - 1, 0);
    dvC341Geo_WriteToBuffer(B4_BBACTVW, iHeight - 1, 0);
    dvC341Geo_WriteToBuffer(B4_CPURAD, cpurad & 0x07ffffff, 0);
    dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad & 0x07ffffff, 0);

    if(ucFastWrite)
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x12, 0);
    }
    else
    {
        dvC341Geo_WriteToBuffer(B4_OSDCT, 0x02, 0); // bitblt
    }
    dvC341Geo_Buffer_Flush();

    do
    {
    }
    while ((dvC341Geo_Read(B4_BOSTAT, 0) & 0x01) == 0x01);
    dvC341Geo_Write(B4_OSDCT, 0x00, 0);


    return eHAL_WARPING_EXEC_CODE_PASS;
}

#if 0
eHAL_WARPING_EXEC_CODE halWarping_CopyOSDRect_NoSemaphore(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                                        INT16 ixSource,
                                                                        INT16 iySource,
                                                                        INT16 ixDestination,
                                                                        INT16 iyDestination,
                                                                        INT16 iWidth,
                                                                        INT16 iHeight)
{
    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d %d %d)", __FUNCTION__
                                                      , ucLayerMode
                                                      , ixSource
                                                      , iySource
                                                      , ixDestination
                                                      , iyDestination
                                                      , iWidth
                                                      , iHeight);


    int cpurad = m_OSD_CURRENT_LAYER + ixSource + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iySource;     //R70G2_Simon_0002
    int cpuwad = m_OSD_CURRENT_LAYER + ixDestination + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iyDestination;
    LOG_MSG(db_HAL_WARPING, "cpurad 0x%X , cpuwad 0x%X\n\n", cpurad, cpuwad);

    dvC341Geo_WriteToBuffer(B4_BBWMWI, (DEF_OSD_PANEL_WIDTH / OSD_LINEFEED_BASE) & 0xff);
    dvC341Geo_WriteToBuffer(B4_BBRMWI, (DEF_OSD_PANEL_WIDTH / OSD_LINEFEED_BASE) & 0xff);
    dvC341Geo_WriteToBuffer(B4_BBACTHW, iWidth - 1);
    dvC341Geo_WriteToBuffer(B4_BBACTVW, iHeight - 1);
    dvC341Geo_WriteToBuffer(B4_CPURAD, cpurad & 0x07ffffff);
    dvC341Geo_WriteToBuffer(B4_CPUWAD, cpuwad & 0x07ffffff);
    dvC341Geo_WriteToBuffer(B4_OSDCT, 0x02);      // bitblt
    dvC341Geo_Buffer_Flush();

    do
    {
    }
    while ((dvC341Geo_Read(B4_BOSTAT) & 0x01) == 0x01);
    dvC341Geo_Write(B4_OSDCT, 0x00);

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif

#if 0
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_CurrentSetting(void)
{
    LOG_MSG(db_HAL_WARPING, "Panel = %dx%d\n", m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW);

    sCurrentFileInfo sCurrentInfo;  //A35G2_Simon_0110
    halWarping_AP_LoadCurrentFileInfo(&sCurrentInfo);  //A35G2_Simon_0110
    halWarp_AP_Set_Interpolation_Lut_H_Index(sCurrentInfo.sInfo.ucManualWarpFilterH);    //A35G2_Simon_0110
    halWarp_AP_Set_Interpolation_Lut_V_Index(sCurrentInfo.sInfo.ucManualWarpFilterV);    //A35G2_Simon_0110

    if(sCurrentInfo.sInfo.ucPanelID != m_sHalWarpingInfo.ePanelTimingId)  //A65_OPTOMA_Simon_0002  //A35G2_Simon_0110
    {
        LOG_MSG(db_HAL_WARPING, "Panel not match, skip (%d)(%d)\n", sCurrentInfo.sInfo.ucPanelID, m_sHalWarpingInfo.ePanelTimingId);  //A35G2_Simon_0110
        halWarping_ClearWarpingTable();
        halWarp_InterpolationSet();  //A35G2_Simon_0110
        halWarping_DBD_ClearBlendingTable();
        halWarping_EdgeBlending_Disable();
        halWarping_clearBlackLevel();
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    INT16 iHRes = m_sHalWarpingInfo.uiWarp_HW;
    INT16 iVRes = m_sHalWarpingInfo.uiWarp_VW;

    UINT16 uiWarpWidth  = (m_sHalWarpingInfo.uiWarp_HW + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1;
    UINT16 uiWarpHeight = (m_sHalWarpingInfo.uiWarp_VW + DEF_WP_SPACE - 1) / DEF_WP_SPACE + 1;

    //Apply Warp
    UINT16 uiSize = iHRes * iVRes ;

    FILE *pFile_TPx = NULL;
    FILE *pFile_TPy = NULL;
    pFile_TPx = fopen(BLENDING_AP_WARP_TPX_CURRENT_FILENAME, "rb");
    pFile_TPy = fopen(BLENDING_AP_WARP_TPY_CURRENT_FILENAME, "rb");

    if(pFile_TPx == NULL || pFile_TPy == NULL)
    {
        if(pFile_TPx != NULL)
            fclose(pFile_TPx);

        if(pFile_TPy != NULL)
            fclose(pFile_TPy);

        LOG_MSG(db_HAL_WARPING, "\nNo temp warp table (%s)(%s)\n", BLENDING_AP_WARP_TPX_CURRENT_FILENAME , BLENDING_AP_WARP_TPY_CURRENT_FILENAME);

        halWarping_ClearWarpingTable();
        halWarp_InterpolationSet();  //A35G2_Simon_0110
    }
    else
    {
        FLOAT *pucData_TPx = (FLOAT *)malloc(uiSize * sizeof(FLOAT));
        FLOAT *pucData_TPy = (FLOAT *)malloc(uiSize * sizeof(FLOAT));
        if(pucData_TPx == NULL || pucData_TPy == NULL)
        {
            if(pucData_TPx != NULL)
                free(pucData_TPx);

            if(pucData_TPy != NULL)
                free(pucData_TPy);

            if(pFile_TPx != NULL)
                fclose(pFile_TPx);

            if(pFile_TPy != NULL)
                fclose(pFile_TPy);

            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(pucData_TPx, 0xFF, (uiSize * sizeof(FLOAT)));
        memset(pucData_TPy, 0xFF, (uiSize * sizeof(FLOAT)));
        fread(pucData_TPx, sizeof(FLOAT), uiSize, pFile_TPx);
        fread(pucData_TPy, sizeof(FLOAT), uiSize, pFile_TPy);

        halWarping_convertTPtoDTDT(pucData_TPx, pucData_TPy, uiWarpWidth, uiWarpHeight);
        halWarp_InterpolationSet();  //A35G2_Simon_0110
        free(pucData_TPx);
        free(pucData_TPy);
        fclose(pFile_TPx);
        fclose(pFile_TPy);
    }


    //apply temp DBD table
    FILE *pDBDFile = NULL;
    pDBDFile = fopen(BLENDING_AP_DBD_TABLE_CURRENT_FILENAME, "rb");

    if(pDBDFile == NULL)
    {
        LOG_MSG(db_HAL_WARPING, "\nNo temp DBD table\n");

        halWarping_DBD_ClearBlendingTable();
        halWarping_EdgeBlending_Disable();
    }
    else
    {
        UINT32 ulSize = (UINT32)iHRes * iVRes ;
        UINT8 *pucDBD_Data = (UINT8 *)malloc(ulSize);
        if(pucDBD_Data == NULL)
        {
            fclose(pDBDFile);
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(pucDBD_Data, 0xFF, ulSize);
        fread(pucDBD_Data, 1, ulSize, pDBDFile);

        halWarping_DBD_BlendingTable_Set(iHRes, iVRes, pucDBD_Data);

        free(pucDBD_Data);
        fclose(pDBDFile);
    }

    //apply blacklevel
    #ifndef CUSTOM_CHRISTIE
    FILE *pFile = NULL;
    pFile = fopen(BLENDING_AP_BKLEVEL_TABLE_CURRENT_FILENAME, "rb");
    if(pFile == NULL)
    {
        LOG_MSG(db_HAL_WARPING, "\nNo temp blacklevel table\n");
        halWarping_clearBlackLevel();
    }
    else
    {
        UINT32 ulSize = (UINT32)iHRes * iVRes;
        UINT8 *pucTableData = (UINT8 *)malloc(ulSize);
        if(pucTableData == NULL)
        {
            fclose(pFile);
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }

        memset(pucTableData, 0xFF, ulSize);
        fread(pucTableData, 1, (ulSize), pFile);

        halWarping_BlackLevel_Table_Set(pucTableData, iHRes, iVRes);
        halWarping_BlackLevel_Enable(TRUE);

        fclose(pFile);
        free(pucTableData);
    }

    //load blacklevel palette from file
    FILE *fp_palette = fopen(BLENDING_AP_BKLEVEL_PALETTE_CURRENT_FILENAME, "rb");
    if(fp_palette != NULL)
    {
        fread(m_sHalWarpingInfo.aucBL_Palette , sizeof(m_sHalWarpingInfo.aucBL_Palette) , 1 , fp_palette);
        fflush(fp_palette);
        fclose(fp_palette);

        for(UINT8 ucIndex = 0 ; ucIndex < 16 ; ucIndex++)
        {
            UINT8 ucRed   = m_sHalWarpingInfo.aucBL_Palette[ucIndex][0] ;
            UINT8 ucGreen = m_sHalWarpingInfo.aucBL_Palette[ucIndex][1] ;
            UINT8 ucBlue  = m_sHalWarpingInfo.aucBL_Palette[ucIndex][2] ;
            halWarping_BlackLevel_Palette_Set(ucIndex, ucRed, ucGreen, ucBlue);
        }
    }
    #endif

    return eHAL_WARPING_EXEC_CODE_PASS;

}
#endif

#if 0
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_Warp(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    INT8 cFileName[128] = {0};
    snprintf((char *)cFileName, 128, "%s/Warp%d", BLENDING_AP_HICC2_SAVING_PATH, ucIndex);

    UINT32 ulSize = sizeof(sWarpFileInfo);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
        halWarp_GeometrySet();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == NO_FILE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
        halWarp_GeometrySet();
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    sWarpFileInfo *sWarpInfo = (sWarpFileInfo *)pucData;
    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag %d\n",         offsetof(sWarpFileInfo, ucEnableFlag),  sWarpInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",             offsetof(sWarpFileInfo, uc3DFlag),      sWarpInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",              offsetof(sWarpFileInfo, ucWidth),       sWarpInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",             offsetof(sWarpFileInfo, ucHeight),      sWarpInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table_TpX %f %f %f\n",    offsetof(sWarpFileInfo, afTable_TpX),   sWarpInfo->afTable_TpX[0], sWarpInfo->afTable_TpX[1], sWarpInfo->afTable_TpX[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table_TpY %f %f %f\n",    offsetof(sWarpFileInfo, afTable_TpY),   sWarpInfo->afTable_TpY[0], sWarpInfo->afTable_TpY[1], sWarpInfo->afTable_TpY[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X LutTable H%d V%d\n",      offsetof(sWarpFileInfo, aucLutTable),   sWarpInfo->aucLutTable[0], sWarpInfo->aucLutTable[1]);  //A35G2_Simon_0110

    //check ucEnableFlag
    if(sWarpInfo->ucEnableFlag != WARPING_TYPE_AP && sWarpInfo->ucEnableFlag != WARPING_TYPE_AP_MYSTIQUE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
        halWarp_GeometrySet();
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check panel
    if(sWarpInfo->uc3DFlag == FALSE)  //warp data is 2D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply warp %d %d\n", sWarpInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_GeometrySet();
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }
    else  //warp data is 3D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_3D_OUTPUT)  //not 3D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply warp %d %d\n", sWarpInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_GeometrySet();
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }

    //check resolution               //A35G2_Simon_0120 Start
    UINT16 uiApplyWidth  = sWarpInfo->ucWidth;
    UINT16 uiApplyHeight = sWarpInfo->ucHeight;

    //if file data error, use panel resolution
    if(uiApplyWidth  > 127 ||
       uiApplyHeight > 76
    )
    {
        //use panel resolution
        uiApplyWidth  = (m_sHalWarpingInfo.uiWarp_HW + m_sHalWarpingInfo.uiWpSpace - 1) / m_sHalWarpingInfo.uiWpSpace + 1;
        uiApplyHeight = (m_sHalWarpingInfo.uiWarp_VW + m_sHalWarpingInfo.uiWpSpace - 1) / m_sHalWarpingInfo.uiWpSpace + 1;

        //OPD
        UINT8 aucString[256];
        snprintf((char *)aucString, sizeof(aucString), "WarpMemory%d,%dx%d,%dx%d", ucIndex+1
                                                                                 , sWarpInfo->ucWidth
                                                                                 , sWarpInfo->ucHeight
                                                                                 , uiApplyWidth
                                                                                 , uiApplyHeight);
        halWarping_OPDEvent(aucString);
        LOG_MSG(db_HAL_WARPING, "%s\n", aucString);
    }

    INT32 Start = TMO_GetSysRunTime();

    eResult |= halWarping_convertTPtoDTDT(sWarpInfo->afTable_TpX, sWarpInfo->afTable_TpY, uiApplyWidth, uiApplyHeight);   //A35G2_Simon_0120 End

    if(m_sWArpGeoPara.m_cWarpAutoFilterAp == FALSE)  // load AP manual value       //A35G2_Simon_0110
    {
        if(sWarpInfo->aucLutTable[0] < WAPR_FILTER_H_MAX) //H index
        {
            m_sWArpGeoPara.m_cWarpFilterApValue_H = sWarpInfo->aucLutTable[0];
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "warp filter error H %d\n", sWarpInfo->aucLutTable[0]);
        }

        if(sWarpInfo->aucLutTable[1] < WAPR_FILTER_V_MAX) //V index
        {
            m_sWArpGeoPara.m_cWarpFilterApValue_V = sWarpInfo->aucLutTable[1];
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "warp filter error V %d\n", sWarpInfo->aucLutTable[1]);
        }
    }

    halWarp_InterpolationSet(); //A35G2_Simon_0110

    INT32 End = TMO_GetSysRunTime();
    LOG_MSG(db_HAL_WARPING, "\nhalWarping_MemoryApplyAP_Warp time %d\n", End-Start);

    free(pucData);

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_Blending(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    INT8 cFileName[128] = {0};
    snprintf((char *)cFileName, 128, "%s/Blend%d", BLENDING_AP_HICC2_SAVING_PATH, ucIndex);

    UINT32 ulSize = sizeof(sBlendFileInfo);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        //halWarping_DBD_Blending_Enable(FALSE);
        halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
        halWarp_BlendingSet();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == NO_FILE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        //halWarping_DBD_Blending_Enable(FALSE);
        halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
        halWarp_BlendingSet();
        return eHAL_WARPING_EXEC_CODE_PASS;
    }


    sBlendFileInfo *sBlendInfo = (sBlendFileInfo *)pucData;
    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag %d\n",     offsetof(sBlendFileInfo, ucEnableFlag), sBlendInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",         offsetof(sBlendFileInfo, uc3DFlag),     sBlendInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",          offsetof(sBlendFileInfo, ucWidth),      sBlendInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",         offsetof(sBlendFileInfo, ucHeight),     sBlendInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table %d %d %d\n",    offsetof(sBlendFileInfo, aucTable),     sBlendInfo->aucTable[0], sBlendInfo->aucTable[1], sBlendInfo->aucTable[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X EdgeBlend %d %d %d\n",offsetof(sBlendFileInfo, aucEdgeBlend), sBlendInfo->aucEdgeBlend[0], sBlendInfo->aucEdgeBlend[1], sBlendInfo->aucEdgeBlend[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X EGBCT %d %d %d\n",    offsetof(sBlendFileInfo, aucEGBCT),     sBlendInfo->aucEGBCT[0], sBlendInfo->aucEGBCT[1], sBlendInfo->aucEGBCT[2]);

    //check ucEnableFlag
    if(sBlendInfo->ucEnableFlag != WARPING_TYPE_AP && sBlendInfo->ucEnableFlag != WARPING_TYPE_AP_MYSTIQUE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        //halWarping_DBD_Blending_Enable(FALSE);
        halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
        halWarp_BlendingSet();
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check panel
    if(sBlendInfo->uc3DFlag == FALSE)  //blending data is 2D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blend %d %d\n", sBlendInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            //eResult |= halWarping_DBD_Blending_Enable(FALSE);
            halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
            halWarp_BlendingSet();
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }
    else  //blending data is 3D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_3D_OUTPUT)  //not 3D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blend %d %d\n", sBlendInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            //eResult |= halWarping_DBD_Blending_Enable(FALSE);
            halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
            halWarp_BlendingSet();
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }

    //LOG_MSG(db_HAL_WARPING, "Need Rewrite Check %d %d\n", m_sHalWarpingInfo.ucCurrent_DBD_Data_Index, ucIndex);

    INT32 Start = TMO_GetSysRunTime();

    #if 0
    if(m_sHalWarpingInfo.ucCurrent_DBD_Data_Index == ucIndex)  //Data already write into CPU
    {
        eResult |= halWarping_DBD_Blending_Enable(TRUE);
    }
    else
    {
        m_sHalWarpingInfo.ucCurrent_DBD_Data_Index = ucIndex;
    }

    halWarping_BlendingGamma(TRUE);
    #endif

    //check resolution          //A35G2_Simon_0120 Start
    UINT16 uiApplyWidth  = sBlendInfo->ucWidth;
    UINT16 uiApplyHeight = sBlendInfo->ucHeight;

    //if file data error, use panel resolution
    if(uiApplyWidth  > 3840 ||
       uiApplyHeight > 2160
    )
    {
        //use panel resolution
        uiApplyWidth  = m_sHalWarpingInfo.uiWarp_HW;
        uiApplyHeight = m_sHalWarpingInfo.uiWarp_VW;

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

    return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_BlackLevel(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    INT8 cFileName[128] = {0};
    snprintf((char *)cFileName, 128, "%s/BlackLevel%d", BLENDING_AP_HICC2_SAVING_PATH, ucIndex);

    UINT32 ulSize = sizeof(sBlackLevelFileInfo);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == NO_FILE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    sBlackLevelFileInfo *sBlackLevelInfo = (sBlackLevelFileInfo *)pucData;
    LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag %d\n",         offsetof(sBlackLevelFileInfo, ucEnableFlag),    sBlackLevelInfo->ucEnableFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",             offsetof(sBlackLevelFileInfo, uc3DFlag),        sBlackLevelInfo->uc3DFlag);
    LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",              offsetof(sBlackLevelFileInfo, ucWidth),         sBlackLevelInfo->ucWidth);
    LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",             offsetof(sBlackLevelFileInfo, ucHeight),        sBlackLevelInfo->ucHeight);
    LOG_MSG(db_HAL_WARPING, "0x%08X Table %d %d %d\n",        offsetof(sBlackLevelFileInfo, aucTable),        sBlackLevelInfo->aucTable[0], sBlackLevelInfo->aucTable[1], sBlackLevelInfo->aucTable[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X BiasPalette %d %d %d\n",  offsetof(sBlackLevelFileInfo, aucBiasPalette),  sBlackLevelInfo->aucBiasPalette[0], sBlackLevelInfo->aucBiasPalette[1], sBlackLevelInfo->aucBiasPalette[2]);
    LOG_MSG(db_HAL_WARPING, "0x%08X EGBCT %d %d %d\n",        offsetof(sBlackLevelFileInfo, aucEGBIASCT),     sBlackLevelInfo->aucEGBIASCT[0], sBlackLevelInfo->aucEGBIASCT[1], sBlackLevelInfo->aucEGBIASCT[2]);

    //check ucEnableFlag
    if(sBlackLevelInfo->ucEnableFlag != WARPING_TYPE_AP && sBlackLevelInfo->ucEnableFlag != WARPING_TYPE_AP_MYSTIQUE)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check panel
    if(sBlackLevelInfo->uc3DFlag == FALSE)  //black level data is 2D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blacklevel %d %d\n", sBlackLevelInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            eResult |= halWarping_BlackLevel_Enable(FALSE);
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }
    else  //black level data is 3D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_3D_OUTPUT)  //not 3D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blacklevel %d %d\n", sBlackLevelInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            eResult |= halWarping_BlackLevel_Enable(FALSE);
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }


    //LOG_MSG(db_HAL_WARPING, "Need Rewrite Check %d %d\n", m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index, ucIndex);

    INT32 Start = TMO_GetSysRunTime();

    #if 0
    if(m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index == ucIndex)  //Data already write into CPU
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

        m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index = ucIndex;
    }
    #endif

    for(UINT8 ucIndex = 0 ; ucIndex < 16 ; ucIndex++)
    {
        UINT8 ucRed = sBlackLevelInfo->aucBiasPalette[ucIndex*3+0] ;
        UINT8 ucGreen = sBlackLevelInfo->aucBiasPalette[ucIndex*3+1] ;
        UINT8 ucBlue = sBlackLevelInfo->aucBiasPalette[ucIndex*3+2] ;
        eResult |= halWarping_BlackLevel_Palette_Set(ucIndex, ucRed, ucGreen, ucBlue);
    }

    //check resolution               //A35G2_Simon_0120 Start
    UINT16 uiApplyWidth  = sBlackLevelInfo->ucWidth;
    UINT16 uiApplyHeight = sBlackLevelInfo->ucHeight;

    //if file data error, use panel resolution
    if(uiApplyWidth  > 3840 ||
       uiApplyHeight > 2160
    )
    {
        //use panel resolution
        uiApplyWidth  = m_sHalWarpingInfo.uiWarp_HW;
        uiApplyHeight = m_sHalWarpingInfo.uiWarp_VW;

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

    eResult |= halWarping_BlackLevel_Table_Set(sBlackLevelInfo->aucTable, uiApplyWidth, uiApplyHeight);    //A35G2_Simon_0120 End
    halWarping_BlackLevel_Enable(TRUE);

    INT32 End = TMO_GetSysRunTime();

    LOG_MSG(db_HAL_WARPING, "\nhalWarping_MemoryApplyAP_BlackLevel time %d\n", End-Start);

    free(pucData);

    return eResult;
}
#endif

eHAL_WARPING_EXEC_CODE halWarping_MemoryAP_ClearSaveFile(void)
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

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_MemoryAP_ClearCurrentFile(void)
{
    char cFilename[64] = {0};

    sprintf(cFilename, BLENDING_AP_HICC2_WARP_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);
    sprintf(cFilename, BLENDING_AP_HICC2_DBD_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);
    sprintf(cFilename, BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);

    SYSTEM_CALL("sync");

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_MemoryAP_ClearCurrentWarpFile(void)
{
    char cFilename[64] = {0};

    sprintf(cFilename, BLENDING_AP_HICC2_WARP_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);

    SYSTEM_CALL("sync");

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_MemoryAP_ClearCurrentBlendFile(void)
{
    char cFilename[64] = {0};

    sprintf(cFilename, BLENDING_AP_HICC2_DBD_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);
    sprintf(cFilename, BLENDING_AP_HICC2_BKLEVEL_SAVE_FILENAME , BLENDING_AP_MEM_CUR_IDX);
    remove(cFilename);

    SYSTEM_CALL("sync");

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_MemoryAP_ClearAPTempFile(void)
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

    return eHAL_WARPING_EXEC_CODE_PASS;
}

#if 0
eHAL_WARPING_EXEC_CODE halWarping_AP_OSD_Init(void)  //A65_OPTOMA_Simon_0001
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    //clear OSD
    eResult = halWarpOSD_ClearOSD();

    if(eResult != eHAL_WARPING_EXEC_CODE_PASS)
    {
        return eResult;
    }

    //init OSD layer
    m_OSD_CURRENT_LAYER = DEF_OSDSAD2;   //H2PF_Simon_0107
    halWarping_OSDWriteTransparentColor(TRUE,0,0,0,0);
    halWarping_OSDWriteBlink(FALSE,0,0,TRUE);

    eResult = halWarping_OSD_ON();

    return eResult;
}
#endif

eHAL_WARPING_EXEC_CODE halWarping_OSDLayerReset(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    //init OSD layer
    m_OSD_CURRENT_LAYER = DEF_OSDSAD2;   //H2PF_Simon_0107

    return eResult;
}


eHAL_WARPING_EXEC_CODE halAdvBlackLevelSelectControlPoint(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	#if BLACKLEVEL_SEL_MODE			//A65_OPTOMA_Doulas_0146 Modify
	if((eDIR)ucValue < DIR__INVALID)
	{
		utilWarp_OsdBlackLevel_SelectOnThePattern((eDIR)ucValue);
	}
	#else
	if((eDIR)ucValue == DIR__LEFT)
	{
		utilWarp_OsdBlackLevel_SelectOnThePattern(SEQ__COUNTERCLOCKWISE);
	}
	else if((eDIR)ucValue == DIR__RIGHT)
	{
		utilWarp_OsdBlackLevel_SelectOnThePattern(SEQ__CLOCKWISE);
	}
	#endif

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelMoveControlPoint(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_OsdBlackLevel_MovePoint((eDIR)ucValue);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelAddPoint(void)				//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_OsdBlackLevel_AddPoint();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelDeletePoint(void)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_OsdBlackLevel_DeletePoint();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSelectArea(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_OsdBlackLevel_SetArea(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_PREVIEW);
		utilWarp_ShowOsdPattern(PAT_TYPE__OFF);					//A65_OPTOMA_Doulas_0023
	}


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetEnable(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_OsdBlackLevel_SetEnable(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		SaveWarpConfig();
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetRed(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_OsdBlackLevel_SetRed(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
		{
			SaveWarpConfig();
		}
	}


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetGreen(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_OsdBlackLevel_SetGreen(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
		{
			SaveWarpConfig();
		}
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetBlue(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    BOOL bVal = utilWarp_OsdBlackLevel_SetBlue(ucValue);
	if(bVal)
	{
	//	LOG_MSG(db_ALWAYS, "changed\r\n");
		if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
		{
			SaveWarpConfig();
		}
	}


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelReset(UINT8 ucResetLevel)		//A65_OPTOMA_Doulas_0029 Modify//A65_OPTOMA_Doulas_0020
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	if(ucResetLevel == eBLACK_LEVEL_RESET_ALL)
		utilWarp_OsdBlackLevel_ResetConfig();
	else if(ucResetLevel == eBLACK_LEVEL_RESET_BOTTOM)
		utilWarp_OsdBlackLevel_ResetConfigBottom();
	else if(ucResetLevel == eBLACK_LEVEL_RESET_TOP)
		utilWarp_OsdBlackLevel_ResetConfigTop();

	//LOG_MSG(db_ALWAYS, "halAdvResetWarping\r\n");

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelShowAddPointPattern(void)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	if(utilWarp_OsdBlackLevel_IsPointAddable())
	{
		utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_ADD_POINT);
	}
	else
	{
	//	LOG_MSG(db_ALWAYS, "Point number is 32, can not be added any more!!\r\n");
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelShowRemovePointPattern(void)		//A65_OPTOMA_Doulas_0020
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	if(utilWarp_OsdBlackLevel_IsPointDeletable())
	{
		utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_DEL_POINT);
	}
	else
	{
	//	LOG_MSG(db_ALWAYS, "Point number is 4, can not be deleted any more!!\r\n");
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

BOOL halAdvBlackLevelIsPointDeletable(void)		//A65_OPTOMA_Doulas_0020
{
    return utilWarp_OsdBlackLevel_IsPointDeletable();
}

BOOL halAdvBlackLevelIsPointAddable(void)		//A65_OPTOMA_Doulas_0020
{
    return utilWarp_OsdBlackLevel_IsPointAddable();
}


eHAL_WARPING_EXEC_CODE halWarping_Panel_Update(ePANEL_ID ePanelId)
{
    m_sHalWarpingInfo.ePanelTimingId = ePanelId;

    switch(m_sHalWarpingInfo.ePanelTimingId)
    {
        case ePANEL_ID_XGA_60HZ:
        case ePANEL_ID_XGA_120HZ:
            m_sHalWarpingInfo.uiWarp_HW = 1024;
            m_sHalWarpingInfo.uiWarp_VW = 768;
            break;

        case ePANEL_ID_WXGA_60HZ:
        case ePANEL_ID_WXGA_120HZ:
            m_sHalWarpingInfo.uiWarp_HW = 1280;
            m_sHalWarpingInfo.uiWarp_VW = 800;//768;        //R70G2_Doulas_0004 Modify
            break;

        case ePANEL_ID_1600x1200_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 1600;
            m_sHalWarpingInfo.uiWarp_VW = 1200;
            break;

        case ePANEL_ID_WUXGA_60HZ:
        case ePANEL_ID_WUXGA_120HZ:  //A70LV_Doulas_0377 Add
        case ePANEL_ID_WUXGA_240HZ:
            m_sHalWarpingInfo.uiWarp_HW = 1920;
            m_sHalWarpingInfo.uiWarp_VW = 1200;
            break;
        case ePANEL_ID_WQXGA_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 2560;
            m_sHalWarpingInfo.uiWarp_VW = 1600;
            break;

        case ePANEL_ID_720P_50HZ:
        case ePANEL_ID_720P_60HZ:
        case ePANEL_ID_720P_120HZ:
            m_sHalWarpingInfo.uiWarp_HW = 1280;
            m_sHalWarpingInfo.uiWarp_VW = 720;
            break;

        case ePANEL_ID_1080P_50HZ:
        case ePANEL_ID_1080P_60HZ:
        case ePANEL_ID_1080P_120HZ:
        case ePANEL_ID_1080P_240HZ:
        case ePANEL_ID_LAST:
            default:
            m_sHalWarpingInfo.uiWarp_HW = 1920;
            m_sHalWarpingInfo.uiWarp_VW = 1080;
            break;

        case ePANEL_ID_3840x2160_50HZ:
        case ePANEL_ID_3840x2160_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 3840;
            m_sHalWarpingInfo.uiWarp_VW = 2160;
            break;

        case ePANEL_ID_3840x2400_50HZ:
        case ePANEL_ID_3840x2400_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 3840;
            m_sHalWarpingInfo.uiWarp_VW = 2400;
            break;

        case ePANEL_ID_4096x2160_50HZ:
        case ePANEL_ID_4096x2160_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 4096;
            m_sHalWarpingInfo.uiWarp_VW = 2160;
            break;

        case ePANEL_ID_960x2160_50HZ:
        case ePANEL_ID_960x2160_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 960;
            m_sHalWarpingInfo.uiWarp_VW = 2160;
            break;

        case ePANEL_ID_1280x2160_50HZ:
        case ePANEL_ID_1280x2160_60HZ:
            m_sHalWarpingInfo.uiWarp_HW = 1280;
            m_sHalWarpingInfo.uiWarp_VW = 2160;
            break;

        case ePANEL_ID_2688x1472_120HZ:
            m_sHalWarpingInfo.uiWarp_HW = 2688;
            m_sHalWarpingInfo.uiWarp_VW = 1472;
            break;

        case ePANEL_ID_2716x1528_120HZ:
            m_sHalWarpingInfo.uiWarp_HW = 2716;
            m_sHalWarpingInfo.uiWarp_VW = 1528;
            break;
    }

    //G100_Simon_0004
    //write panel H/V to Database
    utilDatabase_WriteInformationData(eDI_PANEL_H_RESOLUTION, DATA_TYPE_UI_DIGIT_16, 0, &m_sHalWarpingInfo.uiWarp_HW);
    utilDatabase_WriteInformationData(eDI_PANEL_V_RESOLUTION, DATA_TYPE_UI_DIGIT_16, 0, &m_sHalWarpingInfo.uiWarp_VW);
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) %d*%d \r\n", __FUNCTION__, __LINE__, m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW);

    if((m_sHalWarpingInfo.uiWarp_HW > 1920) || (m_sHalWarpingInfo.uiWarp_VW > 1200))
    {
        m_sHalWarpingInfo.ucIs4KWarp = 1;
        m_sHalWarpingInfo.uiWpSpace = DEF_WP_SPACE_32P;
        m_sHalWarpingInfo.uiWpSpaceBitShift = DEF_WP_SPACE_BIT_32P;
    }
    else
    {
        m_sHalWarpingInfo.ucIs4KWarp = 0;
        m_sHalWarpingInfo.uiWpSpace = DEF_WP_SPACE_16P;
        m_sHalWarpingInfo.uiWpSpaceBitShift = DEF_WP_SPACE_BIT_16P;
    }

    m_sHalWarpingInfo.uiWarp_HW_1CH = dvC341_OutputActiveHW_1CH();

    m_sHalWarpingInfo.uiPixelGridHW = (m_sHalWarpingInfo.uiWarp_HW + m_sHalWarpingInfo.uiWpSpace - 1) / m_sHalWarpingInfo.uiWpSpace; //roundup
    m_sHalWarpingInfo.uiPixelGridVW = (m_sHalWarpingInfo.uiWarp_VW + m_sHalWarpingInfo.uiWpSpace - 1) / m_sHalWarpingInfo.uiWpSpace; //roundup

    m_sHalWarpingInfo.fWarp_PHWHF = (m_sHalWarpingInfo.uiWarp_HW / 2.0);
    m_sHalWarpingInfo.fWarp_PVWHF = (m_sHalWarpingInfo.uiWarp_VW / 2.0);

    m_sHalWarpingInfo.iDummyHstReset = ((DEF_HW_DUMMY % 3) == 0) ? 0 : 3 - (DEF_HW_DUMMY % 3);
    m_sHalWarpingInfo.iDummyHendGridReset = ((((m_sHalWarpingInfo.uiPixelGridHW + DEF_HW_DUMMY_GRD + DEF_HW_DUMMY_GRD) * m_sHalWarpingInfo.uiWpSpace) % 3) == 0) ? 0 : 1;


    UINT32 ul3D = FALSE;
    if(ePanelId == PANEL_3D_OUTPUT)
    {
        ul3D = TRUE;
        utilDatabase_WriteInformationData(eDI_3D_ENABLE, DATA_TYPE_UI_DIGIT_32, 0, &ul3D);
        m_sHalWarpingInfo.uiWarp_Is120HzPanel = TRUE;  //A35G2_Simon_0093
    }
    else
    {
        ul3D = FALSE;
        utilDatabase_WriteInformationData(eDI_3D_ENABLE, DATA_TYPE_UI_DIGIT_32, 0, &ul3D);
        m_sHalWarpingInfo.uiWarp_Is120HzPanel = FALSE;  //A35G2_Simon_0093
    }
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) 3D flag %d \r\n", __FUNCTION__, __LINE__, ul3D);

    //set MWI
    //m_sHalWarpingInfo.ucBitBLT_MWI         = m_sHalWarpingInfo.uiWarp_HW / 128 ;
    //m_sHalWarpingInfo.ucBLEND_GAIN_DBD_MWI = m_sHalWarpingInfo.uiWarp_HW / 256 + 1;
    //m_sHalWarpingInfo.ucBLEND_BIAS_MWI     = (m_sHalWarpingInfo.uiWarp_HW / 256 + 1) / 2;

    //init OSD
    m_OSD_CURRENT_LAYER = DEF_OSDSAD2;   //H2PF_Simon_0107

    LOG_MSG(db_HAL_WARPING, "m_sHalWarpingInfo.uiWarp_HW_1CH %d \r\n", m_sHalWarpingInfo.uiWarp_HW_1CH);
    LOG_MSG(db_HAL_WARPING, "m_sHalWarpingInfo.uiPixelGridHW %d \r\n", m_sHalWarpingInfo.uiPixelGridHW);
    LOG_MSG(db_HAL_WARPING, "m_sHalWarpingInfo.uiPixelGridVW %d \r\n", m_sHalWarpingInfo.uiPixelGridVW);
    LOG_MSG(db_HAL_WARPING, "m_sHalWarpingInfo.uiWpSpace %d \r\n", m_sHalWarpingInfo.uiWpSpace);


    return eHAL_WARPING_EXEC_CODE_PASS ;
}


eHAL_WARPING_EXEC_CODE halWarping_ColorMask(UINT8 ucColorMask)
{
    LOG_MSG(db_HAL_WARPING, "\nColorMask Set %d\n", ucColorMask);

    m_sHalWarpingInfo.ucColorMask = ucColorMask;

    INT32 lICOLORCT = dvC341Geo_Read(B16_ICOLORCTCH1, 0);
    if(m_sHalWarpingInfo.ucColorMask == 0 || m_sHalWarpingInfo.ucColorMask == 8) // transpancy || White
    {
        dvC341Geo_Write(B16_ICOLORCTCH1, lICOLORCT & 0xF7, 0);
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    // Others Color
    dvC341Geo_WriteToBuffer(B16_ICOLORCTCH1, lICOLORCT | 0x08, 0);

    //Cindy 20180820-213120 //default value is 512
    int nRedMask   = m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[m_sHalWarpingInfo.ucColorMask][0] == 0x00 ? 0 : 512;
    int nGreenMask = m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[m_sHalWarpingInfo.ucColorMask][1] == 0x00 ? 0 : 512;
    int nBlueMask  = m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[m_sHalWarpingInfo.ucColorMask][2] == 0x00 ? 0 : 512;

    LOG_MSG(db_HAL_WARPING, "Mask RGB %d %d %d\n", nRedMask, nGreenMask, nBlueMask);

    dvC341Geo_WriteToBuffer(B16_IGAINRCH1, nRedMask, 0);
    dvC341Geo_WriteToBuffer(B16_IGAINGCH1, nGreenMask, 0);
    dvC341Geo_WriteToBuffer(B16_IGAINBCH1, nBlueMask, 0);
    dvC341Geo_Buffer_Flush();

    return eHAL_WARPING_EXEC_CODE_PASS ;
}

eHAL_WARPING_EXEC_CODE halWarping_InitTestPalette(void)
{
#if 0
    UINT8 ColorPallet[10][3] = {
	{ 0x00, 0x00, 0x00 }, // transparency
	{ 0x00, 0x00, 0x00 }, // black
	{ 0xFF, 0x00, 0x00 }, // red
	{ 0x00, 0xFF, 0x00 }, // green
	{ 0x00, 0x00, 0xFF }, // blue
	{ 0xFF, 0xFF, 0x00 }, // yellow
	{ 0x00, 0xFF, 0xFF }, // cyan
	{ 0xFF, 0x00, 0xFF }, // magenta
	{ 0xFF, 0xFF, 0xFF }, // white
	{ 0x6F, 0x9A, 0xD3 }  // Grey
    };


    dvC341Geo_WriteToBuffer( B9_PLTAD2CH1, 0x00 );
	for(int idx = 0 ; idx < 10 ; idx++)
	{
	    sPaletteSingleInfo sPlt = {idx, ColorPallet[idx][0], ColorPallet[idx][1], ColorPallet[idx][2]};
		halWarpOSD_Config(eGCI_AP, eWOC_Palette_Single, &sPlt, eWRTIE_TO_BUFFER);
	}

	dvC341Geo_WriteToBuffer( B9_PLTAD2CH1, 0x00 ); //dummy

    dvC341Geo_Buffer_Flush();
#endif /* 0 */


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_AP_SaveInfoForCurrentFile(void)  //A65_OPTOMA_Simon_0002   //A35G2_Simon_0110
{
    sCurrentFileInfo sFileInfo;
    memset(&sFileInfo, 0, sizeof(sFileInfo));
    sFileInfo.sInfo.ucPanelID = (UINT8)m_sHalWarpingInfo.ePanelTimingId;
    sFileInfo.sInfo.ucManualWarpFilterH = (UINT8)m_sWArpGeoPara.m_cWarpFilterApValue_H;
    sFileInfo.sInfo.ucManualWarpFilterV = (UINT8)m_sWArpGeoPara.m_cWarpFilterApValue_V;

    FILE *fp = fopen(BLENDING_AP_CURRENT_INFO_FILENAME, "wb");

    if(fp == NULL)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    fwrite(&sFileInfo , sizeof(sFileInfo) , 1 , fp);
    fflush(fp);
    fclose(fp);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_AP_LoadCurrentFileInfo(sCurrentFileInfo *psCurrentInfo)  //A65_OPTOMA_Simon_0002  //A35G2_Simon_0110
{
    FILE *fp = fopen(BLENDING_AP_CURRENT_INFO_FILENAME, "rb");

    if(fp == NULL)
    {
        psCurrentInfo->sInfo.ucPanelID = ePANEL_ID_LAST;
        psCurrentInfo->sInfo.ucManualWarpFilterH = DEFAULT_WAPR_FILTER_LUT_H;
        psCurrentInfo->sInfo.ucManualWarpFilterV = DEFAULT_WAPR_FILTER_LUT_V;
        LOG_MSG(db_HAL_WARPING, "Error: Can`t read current file info\n" );
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    size_t ret = fread(psCurrentInfo , sizeof(sCurrentFileInfo) , 1 , fp);
    fclose(fp);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//A35G2_Simon_0110
eHAL_WARPING_EXEC_CODE halWarping_AP_SaveWarpFilterForCurrentFile(void)  //A65_OPTOMA_Simon_0002      //A35G2_Simon_0110
{
    sCurrentFileInfo sFileInfo;
    memset(&sFileInfo, 0, sizeof(sFileInfo));
    sFileInfo.sInfo.ucPanelID = (UINT8)m_sHalWarpingInfo.ePanelTimingId;

    FILE *fp = fopen(BLENDING_AP_CURRENT_INFO_FILENAME, "wb");

    if(fp == NULL)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    fwrite(&sFileInfo , sizeof(sFileInfo) , 1 , fp);
    fflush(fp);
    fclose(fp);

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_HST_Set(UINT16 uiData)
{
    dvC341Geo_Write(B9_OSDACTHST1CH1, uiData, 0);

	return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_HST_Get(UINT16 *uiData)
{
    *uiData = dvC341Geo_Read(B9_OSDACTHST1CH1, 0);

	return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_VST_Set(UINT16 uiData)
{
    dvC341Geo_Write(B9_OSDACTVST1CH1, uiData, 0);

	return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_VST_Get(UINT16 *uiData)
{
    *uiData = dvC341Geo_Read(B9_OSDACTVST1CH1, 0);

	return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvHide_OSD_Pattern_Set(BOOL bHide)				//A65_OPTOMA_Doulas_0211
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	//if(utilWarp_OsdBlackLevel_IsPointAddable())
	{
		utilWarp_HideOsdPattern(bHide);
	}

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlack_Level_Boundary_Pattern_Hide(BOOL bHide)  //H2PF_Simon_0155
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_HideBlacklevelBoundaryPattern(bHide);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetColIndex(UINT8 ucValue)  //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eRet;
    }

    if(utilWarp_GridPoint_SetColIndex(ucValue) == FALSE)
    {
        eRet = eHAL_WARPING_EXEC_CODE_FAIL;
    }

    return eRet;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetRowIndex(UINT8 ucValue)  //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eRet;
    }

    if(utilWarp_GridPoint_SetRowIndex(ucValue) == FALSE)
    {
        eRet = eHAL_WARPING_EXEC_CODE_FAIL;
    }

    return eRet;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGP_Move(eDIR eDir, UINT16 uiPixel) //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    BOOL bRet = FALSE;
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    switch(eDir)
    {
        case DIR__UP:
            bRet = utilWarp_GridPoint_MoveUp(uiPixel);
        break;

        case DIR__DOWN:
            bRet = utilWarp_GridPoint_MoveDown(uiPixel);
        break;

        case DIR__LEFT:
            bRet = utilWarp_GridPoint_MoveLeft(uiPixel);
        break;

        case DIR__RIGHT:
            bRet = utilWarp_GridPoint_MoveRight(uiPixel);
        break;
    }

    if(bRet == FALSE)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetPositionMove(INT32 lPosX, INT32 lPosY)   //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eRet;
    }

    if(utilWarp_GridPoint_SetPositionMove(lPosX, lPosY) == FALSE)
    {
        eRet = eHAL_WARPING_EXEC_CODE_FAIL;
    }


    return eRet;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetAllPosition(Point2f Data[DEF_NUM_CUR_MAX_H][DEF_NUM_CUR_MAX_V])
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eRet;
    }

    if(utilWarp_GridPoint_SetAllPosition(Data) == FALSE)
    {
         eRet = eHAL_WARPING_EXEC_CODE_FAIL; //over limit
    }

    return eRet;
}

eHAL_WARPING_EXEC_CODE halAdvWarpSettingReset(void) //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    utilWarp_ShowOsdPattern(PAT_TYPE__OFF); //G100_Owen_0118 //A35G2_BRC_Casper_0132
    utilWarp_ResetWarpSetting();

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingEnable_Set(BOOL bEnable)   //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eRet;
    }

    if(utilWarp_SetBlendEnable(bEnable) == FALSE)
    {
        eRet = eHAL_WARPING_EXEC_CODE_FAIL;
    }
    else
    {
        SaveWarpConfig();
    }


    return eRet;
}

eHAL_WARPING_EXEC_CODE halAdvBlending_Width(eDIR eDir, UINT16 uiPixel)  //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    BOOL bRet = FALSE;
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    switch(eDir)
    {
        case DIR__UP:
            bRet = utilWarp_SetOsdBlendWidthTop(uiPixel);
        break;

        case DIR__DOWN:
            bRet = utilWarp_SetOsdBlendWidthDown(uiPixel);
        break;

        case DIR__LEFT:
            bRet = utilWarp_SetOsdBlendWidthLeft(uiPixel);
        break;

        case DIR__RIGHT:
            bRet = utilWarp_SetOsdBlendWidthRight(uiPixel);
        break;
    }

    if(bRet == FALSE)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    SaveWarpConfig();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlending_Offset(eDIR eDir, UINT16 uiPixel) //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    BOOL bRet = FALSE;
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    switch(eDir)
    {
        case DIR__UP:
            bRet = utilWarp_SetOsdBlendOffsetTop(uiPixel);
        break;

        case DIR__DOWN:
            bRet = utilWarp_SetOsdBlendOffsetDown(uiPixel);
        break;

        case DIR__LEFT:
            bRet = utilWarp_SetOsdBlendOffsetLeft(uiPixel);
        break;

        case DIR__RIGHT:
            bRet = utilWarp_SetOsdBlendOffsetRight(uiPixel);
        break;
    }

    if(bRet == FALSE)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    SaveWarpConfig();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendConfigReset(void) //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    utilWarp_ShowOsdPattern(PAT_TYPE__OFF); //G100_Owen_0118 //A35G2_BRC_Casper_0132
    utilWarp_ResetBlendConfig();

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvBlack_Level_Boundary_Flag_Set(UINT8 ucBoundary)					//A35G2_Owen_0002 //A65_OPTOMA_Doulas_0239
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

	utilWarp_Black_Level_Boundary_Flag_Set(ucBoundary);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//Initial Adaptive Scale Interpolation Filter LUT
eHAL_WARPING_EXEC_CODE halWarping_AdaptiveScaleFilterLutInit(void)//A65_OPTOMA_CL_0018         //A35G2_Simon_0115
{
	dvC341Geo_AdaptiveScaleFilterLutInit();

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//Auto Warp filter : new type (Adaptive Scale Interpolation Filter LUT)
eHAL_WARPING_EXEC_CODE halWarping_AdaptiveScaleFilterLutEnable(UINT8 ucEnable) //A65_OPTOMA_CL_0018 //A35G2_Simon_0115
{
	dvC341Geo_AdaptiveScaleFilterLutEnable(ucEnable);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_CurrentWarpTypeSet(UINT8 ucType)   //A35G2_Simon_0115
{
	m_sHalWarpingInfo.ucCurrentWarpingType = ucType;

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void halWarping_OPDEvent(UINT8 *aucString)  //A35G2_Simon_0120
{
    uOPD_DATA uOPDData;
    memset(&uOPDData, 0x00, sizeof(uOPD_DATA));

    snprintf(uOPDData.cString, 256, "%s", aucString);

    utilOPD_EventSet(eOPD_GEOMETRY_LOG, &uOPDData);
}


UINT32 halWarpOSD_Palette_TotalDataNumber_Get(void)
{
    return (WARP_OSD_TOTAL_PALETTE * WARP_OSD_PALETTE_ELEMENT);
}

ePANEL_ID halWarpOSD_PanelID_Get(void)
{
    return m_sHalWarpingInfo.ePanelTimingId;
}


eHAL_WARPING_EXEC_CODE halWarpOSD_Draw_InsertLocation(eWARPOSD_DRAW_TYPE eDrawType)
{
    if(eDrawType == eWDT_BEFORE_WARP)
    {
        dvC341Geo_SetOSDMode(TRUE);
    }
    else
    {
        dvC341Geo_SetOSDMode(FALSE);
    }
}

//set Palette , Transparent Color , Blink
eHAL_WARPING_EXEC_CODE halWarpOSD_Config(eGEO_CTRL_INTERFACE eInterface, eWARPOSD_CONFIG eConfig, void *Data, eWARP_OSD_SET_ACTION eAction)
{
    if(eInterface >= eGCI_INVALID ||
       eConfig >= eWOC_Number ||
       eAction >= eWRTIE_NUMBER)
    {
        LOG_MSG(db_HAL_WARPING, "(func:%s,line:%d) Error: eInterface:%d, eConfig:%d eAction:%d\n", __FUNCTION__, __LINE__, eInterface, eConfig, eAction);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    sWARPOSD_CONFIG_INFO *psWarpOSDInfo = NULL;

    if(eInterface == eGCI_OSD)
    {
        psWarpOSDInfo = &m_sHalWarpingInfo.sOSD_WarpOSD_Info;
    }
    else if(eInterface == eGCI_AP)
    {
        psWarpOSDInfo = &m_sHalWarpingInfo.sAp_WarpOSD_Info;
    }

    if(psWarpOSDInfo == NULL)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    switch(eConfig)
    {
        case eWOC_Palette_Single:
            {
                sPaletteSingleInfo sPlt = *(sPaletteSingleInfo *)Data;
                psWarpOSDInfo->Palette[sPlt.index][0] = sPlt.red;
                psWarpOSDInfo->Palette[sPlt.index][1] = sPlt.green;
                psWarpOSDInfo->Palette[sPlt.index][2] = sPlt.blue;

                if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
                {
                    UINT8 SinglePLT[1][WARP_OSD_PALETTE_ELEMENT] = {
                                                                        {psWarpOSDInfo->Palette[sPlt.index][0],
                                                                         psWarpOSDInfo->Palette[sPlt.index][1],
                                                                         psWarpOSDInfo->Palette[sPlt.index][2]}
                                                                   };

                    halWarping_OSDWritePalette(SinglePLT,
                                               sPlt.index,
                                               1,
                                               (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
                }
            }
            break;

        case eWOC_Palette_All:
            memcpy(psWarpOSDInfo->Palette, Data , WARP_OSD_TOTAL_PALETTE*WARP_OSD_PALETTE_ELEMENT);

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->Palette,
                                           0,
                                           WARP_OSD_TOTAL_PALETTE,
                                           (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;


        case eWOC_TransparentColorEnable:
            psWarpOSDInfo->TransparentColorEnable = *(BOOL *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteTransparentColor(psWarpOSDInfo->TransparentColorEnable,
                                                    psWarpOSDInfo->TransparentColor[0],
                                                    psWarpOSDInfo->TransparentColor[1],
                                                    psWarpOSDInfo->TransparentColor[2],
                                                    psWarpOSDInfo->TransparentColor[3]);
            }
            break;

        case eWOC_TransparentColor0:
            psWarpOSDInfo->TransparentColor[0] = *(UINT8 *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteTransparentColor(psWarpOSDInfo->TransparentColorEnable,
                                                    psWarpOSDInfo->TransparentColor[0],
                                                    psWarpOSDInfo->TransparentColor[1],
                                                    psWarpOSDInfo->TransparentColor[2],
                                                    psWarpOSDInfo->TransparentColor[3]);
            }
            break;

        case eWOC_TransparentColor1:
            psWarpOSDInfo->TransparentColor[1] = *(UINT8 *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteTransparentColor(psWarpOSDInfo->TransparentColorEnable,
                                                    psWarpOSDInfo->TransparentColor[0],
                                                    psWarpOSDInfo->TransparentColor[1],
                                                    psWarpOSDInfo->TransparentColor[2],
                                                    psWarpOSDInfo->TransparentColor[3]);
            }
            break;

        case eWOC_TransparentColor2:
            psWarpOSDInfo->TransparentColor[2] = *(UINT8 *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteTransparentColor(psWarpOSDInfo->TransparentColorEnable,
                                                    psWarpOSDInfo->TransparentColor[0],
                                                    psWarpOSDInfo->TransparentColor[1],
                                                    psWarpOSDInfo->TransparentColor[2],
                                                    psWarpOSDInfo->TransparentColor[3]);
            }
            break;

        case eWOC_TransparentColor3:
            psWarpOSDInfo->TransparentColor[3] = *(UINT8 *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteTransparentColor(psWarpOSDInfo->TransparentColorEnable,
                                                    psWarpOSDInfo->TransparentColor[0],
                                                    psWarpOSDInfo->TransparentColor[1],
                                                    psWarpOSDInfo->TransparentColor[2],
                                                    psWarpOSDInfo->TransparentColor[3]);
            }
            break;


        case eWOC_BlinkEnable:
            psWarpOSDInfo->ucBlinkEnable = *(BOOL *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteBlink(psWarpOSDInfo->ucBlinkEnable,
                                         psWarpOSDInfo->ucBlinkTime,
                                         psWarpOSDInfo->ucBlinkCycle,
                                         (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;

        case eWOC_BlinkTime:
            psWarpOSDInfo->ucBlinkTime = *(UINT8 *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteBlink(psWarpOSDInfo->ucBlinkEnable,
                                         psWarpOSDInfo->ucBlinkTime,
                                         psWarpOSDInfo->ucBlinkCycle,
                                         (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;

        case eWOC_BlinkCycle:
            psWarpOSDInfo->ucBlinkCycle = *(UINT8 *)Data;

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWriteBlink(psWarpOSDInfo->ucBlinkEnable,
                                         psWarpOSDInfo->ucBlinkTime,
                                         psWarpOSDInfo->ucBlinkCycle,
                                         (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;

        case eWOC_BlinkPaletteSet0_0:
            memcpy(psWarpOSDInfo->ucBlinkColorSet0, Data , WARP_OSD_PALETTE_ELEMENT);

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->ucBlinkColorSet0,
                                           0xF0,
                                           2,
                                           (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;

        case eWOC_BlinkPaletteSet0_1:
            memcpy(psWarpOSDInfo->ucBlinkColorSet0+WARP_OSD_PALETTE_ELEMENT, Data , WARP_OSD_PALETTE_ELEMENT);

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->ucBlinkColorSet0,
                                           0xF0,
                                           2,
                                           (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;

        case eWOC_BlinkPaletteSet1_0:
            memcpy(psWarpOSDInfo->ucBlinkColorSet1, Data , WARP_OSD_PALETTE_ELEMENT);

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->ucBlinkColorSet1,
                                           0x100,
                                           2,
                                           (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;

        case eWOC_BlinkPaletteSet1_1:
            memcpy(psWarpOSDInfo->ucBlinkColorSet1+WARP_OSD_PALETTE_ELEMENT, Data , WARP_OSD_PALETTE_ELEMENT);

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->ucBlinkColorSet1,
                                           0x100,
                                           2,
                                           (eAction == eWRTIE_TO_CHIP) ? (TRUE):(FALSE));
            }
            break;


        default:
            LOG_MSG(db_ALWAYS, "(func:%s.line:%d) undefined eConfig %d\n", __FUNCTION__, __LINE__, eConfig);
            break;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

UINT8 halWarping_Is4KWarp(void)
{
    return m_sHalWarpingInfo.ucIs4KWarp;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Init(void)
{
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Reset(void)
{
    halAdvBlackLevelReset(eBLACK_LEVEL_RESET_ALL);  //H30K_Doulas_0027
    return eHAL_WARPING_EXEC_CODE_PASS;
}

void halWarpingOsd_DrawFrameMemory(UINT16 PosX, UINT16 PosY, UINT16 Width, UINT16 Height, UINT8 Red, UINT8 Green, UINT8 Blue)
{
    UINT32 x,y;
    UINT32 cpuwad_previous = 0;
    UINT32 cpuwad = 0;
    dvC341Geo_Freeze(TRUE);
    UINT32 WPISFLD = dvC341Geo_FrameMemoryDrawStartAddrGet();
    UINT8 Is30BitProcess = dvc341Geo_IsFrameMemoryProc30Bits();  //H2PF_Simon_0131

    //#ifdef DEF_WARPBLOCK_24BIT_PROCESS

    //if 24bit process
    if(Is30BitProcess == FALSE)  //H2PF_Simon_0131
    {
        for(y=PosY; y<PosY+Height ; y++)
        {
            for(x=PosX; x<PosX+Width ; x++)
            {
                //24bit for BMP format
                cpuwad = WPISFLD + 2*32768*(y/4) + 64*(y%4) + 2*128*(x/20) + 16*(x%20)/5;

                if( (cpuwad) != (cpuwad_previous+3) )
                {
                    dvC341_WriteToBuffer(B4_CPUWAD, cpuwad, 0);
                }

                cpuwad_previous = cpuwad;

                dvC341_WriteToBuffer(B4_CPUDT, Red, 0);  //R
                dvC341_WriteToBuffer(B4_CPUDT, Green, 0);  //G
                dvC341_WriteToBuffer(B4_CPUDT, Blue, 0);  //B
            }
        }

        //dummy
        dvC341_Write(B4_CPUWAD, cpuwad+3, 0);
    }
    else
    {
        //30bit
        for(y=PosY; y<PosY+Height ; y++)
        {
            for(x=PosX; x<PosX+Width ; x++)
            {
                //30bit for BMP format
                UINT32 cpuwad = WPISFLD + 2*32768*(y/4) + 64*(y%4) + 2*128*(x/16) + 4*(x%16);

                if( (cpuwad) != (cpuwad_previous+3) )  //30bit
                {
                    //30bit for BMP format
                    dvC341_WriteToBuffer(B4_CPUWAD, cpuwad | (BIT31), 0);
                }

                cpuwad_previous = cpuwad;

                dvC341_WriteToBuffer(B4_CPUDT, Blue, 0);  //B
                dvC341_WriteToBuffer(B4_CPUDT, Green, 0);  //G
                dvC341_WriteToBuffer(B4_CPUDT, Red, 0);  //R
            }
        }

        //dummy
        dvC341_Write(B4_CPUWAD, cpuwad+3, 0);
    }

    dvC341_Buffer_Flush();

    LOG_MSG(db_ALWAYS, "Draw End\r\n");

}


eHAL_WARPING_EXEC_CODE halWarping_TransferCtrl_Through(void)
{
    return eHAL_WARPING_EXEC_CODE_PASS;
}

UINT16 halWarping_DEF_WP_SPACE_Get(void)
{
    return m_sHalWarpingInfo.uiWpSpace;
}

UINT16 halWarping_DEF_WP_SPACE_BITS_Get(void)
{
    return m_sHalWarpingInfo.uiWpSpaceBitShift;
}

eWARPDATA_DECIMAL_PART_RES halWarping_WarpDataDecimalPartRes_Get(void)
{
    return m_sHalWarpingInfo.eWarpDecimalPartRes;
}

void halWarping_WarpDataDecimalPartRes_Set(eWARPDATA_DECIMAL_PART_RES Value)
{
    m_sHalWarpingInfo.eWarpDecimalPartRes = Value;
}

void halWarping_IsDefaultWarpTable_Set(UINT8 IsDafault)  //H2PF_Simon_0131
{
    m_sHalWarpingInfo.ucIsDefaultWarpTable = IsDafault ;
    dvc341Geo_IsDefaultWarpTable_Set(IsDafault);
}

UINT8 halWarping_IsDefaultWarpTable_Get(void)  //H2PF_Simon_0131
{
    return m_sHalWarpingInfo.ucIsDefaultWarpTable ;
}


void halWarping_FrameMemoryProcessCheck(UINT8 IsDefaultWarpTable)  //H2PF_Simon_0131
{
    halWarping_IsDefaultWarpTable_Set(IsDefaultWarpTable);

    LOG_MSG(db_HAL_WARPING, "[%d %d %d]\r\n", IsDefaultWarpTable, dvC341_PIP_PBP_Enable(), m_sHalWarpingInfo.ePanelTimingId);

    if(dvC341_ScanMode_Get(0) == eSCAN_MODE_NO_SIGNAL && dvC341_ScanMode_Get(1) == eSCAN_MODE_NO_SIGNAL)
    {
        return;
    }

    //30-bit handle case
    if(m_sHalWarpingInfo.ucIsDefaultWarpTable == TRUE &&
       dvC341_PIP_PBP_Enable() == FALSE &&
       m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT &&
       m_bWarpTwistLinkFlag == FALSE &&
       utilWarp_GetWarpCtrl() != WARP_CTRL__AP)
    {
        if(dvc341Geo_IsFrameMemoryProc30Bits() == FALSE || dvC341_ScalerFrameMemoeyFormat_Get() != 30)
        {
            LOG_MSG(db_HAL_WARPING, "config 30\r\n");

            //set Scaler/Warping Block frame memory 30bit process

            UINT16 CurOsdMode  = (UINT16)dvC341_Read(B9_OSDMODE2CH1, 0);
            UINT16 CurOsd2Mode = (UINT16)dvC341_Read(B9_OSDMODE1CH1, 0);
            dvC341_Write( B0_RTCT0, (RTCT_OP_STOP<<4)|RTCT_OP_STOP, 0 );
            if(CurOsdMode | (BIT1))  //if osd on, need to disable osd first
            {
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode & (~(BIT1)), 0);
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode & (~(BIT1)), CH_BANK_OFFSET);
            }
            if(CurOsd2Mode | (BIT1))
            {
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode & (~(BIT1)), 0);
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode & (~(BIT1)), CH_BANK_OFFSET);
            }
            dvC341_Write( B0_RTCT0, (RTCT_OP_POVSCH1CH2<<4)|RTCT_OP_POVSCH1CH2, 0 );
            UINT8 CurMCT = (UINT8)dvC341_Read(B0_MCT1, 0);
            dvC341_Write(B0_MCT1, CurMCT & (~(BIT0)), 0);
            dvC341_Write(B0_MCT2, CurMCT & (~(BIT0)), 0);
            MS_SLEEP(100);

            dvC341_Write( B0_RTCT6, 0xFF, 0);
            dvC341_Write(B144_WPMCTCH1,   0x10, 0);
            dvC341_Write(B144_WPMCTCH1,   0x10, CH_WPBANK_OFFSET);
            dvC341_Write( B0_RTCT6, 0, 0);

            //Scaler block
            dvC341_ScalerFrameMemoryProcSet(TRUE);

            //Warp block
            dvc341Geo_FrameMemoryProc30Bits(TRUE);


            dvC341_Write( B0_RTCT0, (RTCT_OP_STOP<<4)|RTCT_OP_STOP, 0 );
            if(CurOsdMode | (BIT1))  //if osd on, need to disable osd first
            {
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode, 0);
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode, CH_BANK_OFFSET);
            }
            if(CurOsd2Mode | (BIT1))
            {
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode , 0);
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode , CH_BANK_OFFSET);
            }
            dvC341_Write( B0_RTCT0, (RTCT_OP_POVSCH1CH2<<4)|RTCT_OP_POVSCH1CH2, 0 );
            dvC341_Write(B0_MCT1, CurMCT , 0);
            dvC341_Write(B0_MCT2, CurMCT , 0);

            MS_SLEEP(50);

            dvC341_Write( B0_RTCT6, 0xFF, 0);
            dvC341_Write(B144_WPMCTCH1,   0x11, 0);
            dvC341_Write(B144_WPMCTCH1,   0x11, CH_WPBANK_OFFSET);
            dvC341_Write( B0_RTCT6, 0, 0);
        }
    }
    else
    {
        //24-bit handle case
        if((WARP_ON_USE_FRAME_MEMORY_YUV422 == TRUE && (dvc341Geo_IsFrameMemoryProc30Bits() == FALSE || dvC341_ScalerFrameMemoeyFormat_Get() == 30)) ||    //H2PF_Simon_0137
           (WARP_ON_USE_FRAME_MEMORY_YUV422 == FALSE && (dvc341Geo_IsFrameMemoryProc30Bits() == TRUE || dvC341_ScalerFrameMemoeyFormat_Get() == 30))     //current is 30-bit
        )
        {
            LOG_MSG(db_HAL_WARPING, "config 24\r\n");

            //set Scaler/Warping Block frame memory 24bit process

            UINT16 CurOsdMode  = (UINT16)dvC341_Read(B9_OSDMODE2CH1, 0);
            UINT16 CurOsd2Mode = (UINT16)dvC341_Read(B9_OSDMODE1CH1, 0);
            dvC341_Write( B0_RTCT0, (RTCT_OP_STOP<<4)|RTCT_OP_STOP, 0 );
            if(CurOsdMode | (BIT1))  //osd on, need to disable osd first
            {
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode & (~(BIT1)), 0);
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode & (~(BIT1)), CH_BANK_OFFSET);
            }
            if(CurOsd2Mode | (BIT1))  //osd on, need to disable osd first
            {
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode & (~(BIT1)), 0);
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode & (~(BIT1)), CH_BANK_OFFSET);
            }

            dvC341_Write( B0_RTCT0, (RTCT_OP_POVSCH1CH2<<4)|RTCT_OP_POVSCH1CH2, 0 );

            UINT8 CurMCT = (UINT8)dvC341_Read(B0_MCT1, 0);
            dvC341_Write(B0_MCT1, CurMCT & (~(BIT0)), 0);
            dvC341_Write(B0_MCT2, CurMCT & (~(BIT0)), 0);

            MS_SLEEP(100);
            dvC341_Write( B0_RTCT6, 0xFF, 0);
            dvC341_Write(B144_WPMCTCH1,   0x10, 0);
            dvC341_Write(B144_WPMCTCH1,   0x10, CH_WPBANK_OFFSET);
            dvC341_Write( B0_RTCT6, 0, 0);

            //Scaler block
            dvC341_ScalerFrameMemoryProcSet(FALSE);

            //Warp block
            if(WARP_ON_USE_FRAME_MEMORY_YUV422 == TRUE)  //H2PF_Simon_0137
            {
                dvc341Geo_FrameMemoryProc30Bits(TRUE);  //scaler YUV20BIT(YUV422) , warp 30bit
            }
            else
            {
                dvc341Geo_FrameMemoryProc30Bits(FALSE);
            }

            dvC341_Write(B0_RTCT0, (RTCT_OP_STOP<<4)|RTCT_OP_STOP, 0 );
            if(CurOsdMode | (BIT1))
            {
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode, 0);
                dvC341_Write(B9_OSDMODE2CH1, CurOsdMode, CH_BANK_OFFSET);
            }
            if(CurOsd2Mode | (BIT1))
            {
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode , 0);
                dvC341_Write(B9_OSDMODE1CH1, CurOsd2Mode , CH_BANK_OFFSET);
            }
            dvC341_Write(B0_RTCT0, (RTCT_OP_POVSCH1CH2<<4)|RTCT_OP_POVSCH1CH2, 0 );
            dvC341_Write(B0_MCT1, CurMCT, 0);
            dvC341_Write(B0_MCT2, CurMCT, 0);

            MS_SLEEP(50);

            dvC341_Write(B0_RTCT6, 0xFF, 0);
            dvC341_Write(B144_WPMCTCH1,   0x11, 0);
            dvC341_Write(B144_WPMCTCH1,   0x11, CH_WPBANK_OFFSET);
            dvC341_Write(B0_RTCT6, 0, 0);
        }
    }


}


