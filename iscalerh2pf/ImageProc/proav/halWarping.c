#include "utilDbgMsg.h"
#include "utilMisc.h"
#include "math.h"
#include "ProcessMutexData.h"
#include "utilCommon.h"
#include "utilQueueAPI.h"
#include "utilCLICmdAPI.h"
#include "utilMathAPI.h"
#include "timeout.h"
#include "cmd_ap.h"
#include "utilDatabaseAPI.h"
#include "opdCtrlAPI.h"

#include "utilWarpDemoProAV.h"
#include "halScaler.h"
#include "halGui.h"
#include "halMotorCtrlAPI.h"
#include "halWarping.h"

#include "appDataMgr.h"
#include "palMotorMgr.h"

#include "dvProAV_RegTable.h"
#include "dvProAV_Blend.h"
#include "dvProAV_SerialFlash.h"
#include "dvProAV_OSD.h"
#include "dvProAV_Uniformity.h"
#include "dvProAV_Base.h"
#include "dvProAV_PixelShiftWith3d.h"
#include "dvProAV_Warp.h"
#ifdef FPGA_ENTRY_4K
#include "dvProAV_WarpOSD.h"
#endif /* FPGA_ENTRY_4K */


#define BLENDING_GAMMA_TYPE_FRONT 0
#define BLENDING_GAMMA_TYPE_BACK  1

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
    float Px[DEF_WPTBLNUM][DEF_WPTBLMAX_H][DEF_WPTBLMAX_V];
    float Py[DEF_WPTBLNUM][DEF_WPTBLMAX_H][DEF_WPTBLMAX_V];
}sWARP_TABLE;

typedef struct
{
    str_wppos4c WPPOS4C;
    str_pos3d pos_i[4];
    str_pos3d pos_r;
    str_pos3d pos_a[4];
    str_pos2d pos_o[4];
}sWARP_4C_PARA;

typedef struct
{
    ePANEL_ID                       ePanelTimingId;
    BOOL                            bWarpingInit;
    UINT32                          ulWarpingErrCode;
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
    //BOOL                            baGridCursorEnabled[17][17];    // GV_GRID_EN, to indicate in any warp mode which cursor can be shown

    UINT16                          uiWarp_HW;
    UINT16                          uiWarp_VW;
    int                             uiWarp_HW16;
    int                             uiWarp_VW16;
    float                           fWarp_PHWHF;
    float                           fWarp_PVWHF;

    UINT16                          uiBlending_HW;
    UINT16                          uiBlending_VW;

    //UINT16                          uiWarp_FourCornerHW;
    //UINT16                          uiWarp_FourCornerVW;
    UINT8                           ucIs4KWarp;

    //float                           faCursorDefInX[17];             // PS_CUR_DEF_IX, real coordinate in the 3840x2160 canvas //A70LV_Larry_0137
    //float                           faCursorDefInY[17];             // PS_CUR_DEF_IY, real coordinate in the 3840x2160 canvas //A70LV_Larry_0137

    sWARP_TABLE                     sWarpTable; //A70LV_Larry_0051
    UINT8                           acWarpTable[1024*40];

    UINT8                           ucFreeze;

    //Warp OSD
    sWARPOSD_CONFIG_INFO            sOSD_WarpOSD_Info;  //H2PF_Simon_0038
    sWARPOSD_CONFIG_INFO            sAp_WarpOSD_Info;
    //Warp OSD
    //UINT8                           aucPalette[256][3];
    UINT32                          aulSpritePalette[256];
    UINT32                          aulMapSpritePalette[256];
    UINT16                          uiSpritePaletteCount;
    UINT16                          uiMapSpritePaletteCount;
    //UINT16                          uiInhibit_Color;
    INT16                           iPalette_ColorAmount;

    //Scaler OSD (before warp)  //A70LK_CL_0004
    eSCALER_OSD_PALETTE                           ucScalerOsdSwitch;  // 0 : main menu, 1 : use by warp function
    UINT8                           aucScalerOsdPalette[256][3];
    UINT32                          aulScalerOsdSpritePalette[256];
    UINT32                          aulScalerOsdMapSpritePalette[256];
    UINT16                          uiScalerOsdSpritePaletteCount;
    UINT16                          uiScalerOsdMapSpritePaletteCount;
    UINT16                          uiScalerOsdInhibit_Color;
    INT16                           iScalerOsdPalette_ColorAmount;
    UINT8                           ucScalerOsdPixIndexed[2048 * 1200];  //scaler osd is 1920x1200 then enlarge to 3840x2160 for A70LK FPGA
    UINT8                           ucScalerOsdDrawShow;
    sWARPOSD_RECT_DRAW_INFO         asScalerOsdRect[4096];
    UINT16                          uiScalerOsdRectCount;
    sWARPOSD_RECT_DRAW_INFO         asWarpOsdRect[4096];
    UINT16                          uiWarpOsdRectCount;
    UINT8                           ucDrawPngOnBgEnable;

    //Black Level
    UINT8                           aucBL_Palette[16][3];
    UINT8                           aucBL_GammaTable[16][3][16];
    FLOAT                           aucBL_GammaCoef;    //2.2

    UINT8                           ucColorMask;
    UINT8                           ucPixIndexed[4096 * 10];

    UINT8                           ucWarpingCheck;
    UINT8                           ucCurrent_DBD_Data_Index;  //record the last apply blending data index (already write into CPU)
    UINT8                           ucCurrent_Blacklevel_Data_Index;  //record the last apply blacklevel data index (already write into CPU)

	UINT8                           ucCurrentWarpingType;  //WARPING_TYPE_OSD / WARPING_TYPE_AP  //A35G2_Simon_0115
    UINT8                           ucDrawOSD[eWDT_INVALID];
    pthread_mutex_t                 OSDQueueDrawMutex;  //A70LK_Simon_0009

}sHAL_WARPING_INFORMATION, *PsHAL_WARPING_INFORMATION;



static sHAL_WARPING_INFORMATION m_sHalWarpingInfo;
//static sWARP_BASIC      m_sWArpGeoPara; //A70LV_Larry_0051
//static sBLENDING_BASIC  m_sBlendingPara; //A70LV_Larry_0051
static sWARP_4C_PARA    m_sWArp4CPara;
static UINT8 m_pnHeader[4] = {0xcc, 0x77, 0x88, 0x78};
static WARP_CONFIG m_stWarpConfig = {0};
static BOOL m_bWarpTwistLinkFlag = FALSE; //A70LV_Larry_0319
static UINT8 m_ucACU_Target_Tmp = ACU_TARGET_DEFAULT;
static BOOL ucAdvWarpingInit = FALSE;

//ACU test pattern  //A70LK_CL_0001
uint8 m_ucCUShow;
extern uint8 m_pnCUSelectEnableTable[ACU_TARGET_X_MAX][ACU_TARGET_Y_MAX] ;
extern const float HPosRatio[ACU_TARGET_X_MAX] ;
extern const float VPosRatio[ACU_TARGET_Y_MAX] ;
extern const float HCenterPosRatio[3] ;
extern const float VCenterPosRatio[3] ;
uint08 ucSpriteDataBuffer[3840 * 2400];  //A70LK_CL_0004
UINT16 ucSpriteDataBufferRGB[3840 * 2400];  //A70LK_CL_0004
UINT8   ucDrawOSDAckDone = 0;// R70K_Bruce_0022
static UINT8 m_ucACU_Init_Status = WARPING_ACU_INIT_NOT_READY;

#ifdef FPGA_ENTRY_4K
typedef struct
{
    UINT8               ucBuffer[1024];
}sWarpingFormat;

//static sQUEUE m_asWarpingOSD_Ring_Buffer;    //H2PF_Simon_0038  move to palGeo.c
//static sPAYLOAD m_sWarping_Buffer[64];  //A70LK_Simon_0001
#endif /* FPGA_ENTRY_4K */


#if 1
BOOL halWarping_SemaphoreTake(const char *pcSemaphore)
{
    BOOL bResult = dvProAV_SemaphoreTake(TRUE, pcSemaphore);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s) Semaphore Take %d \r\n", pcSemaphore, bResult);

    ASSERT(bResult == TRUE);

    return bResult;
}

BOOL halWarping_SemaphoreGive(const char *pcSemaphore)
{
    BOOL bResult = dvProAV_SemaphoreTake(FALSE, pcSemaphore);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s)Semaphore Give\r\n", pcSemaphore);

    ASSERT(bResult == TRUE);

    return bResult;
}
#endif


#define halWarping_SemaphoreTake(x) TRUE
#define halWarping_SemaphoreGive(x)




#if 1

BOOL halWarping_Is4KPanel(const ePANEL_ID ePanelId)     //A70LV_Doulas_0072 modify
{
    BOOL bIs4K = FALSE;

    bIs4K = (ePanelId == ePANEL_ID_3840x2160_60HZ) ? TRUE
        : (ePanelId == ePANEL_ID_3840x2160_30HZ) ? TRUE
       : (ePanelId == ePANEL_ID_3840x2160_50HZ) ? TRUE
       : (ePanelId == ePANEL_ID_2688x1472_120HZ) ? TRUE
       : (ePanelId == ePANEL_ID_2716x1528_120HZ) ? TRUE
       : (ePanelId == ePANEL_ID_3840x2400_60HZ) ? TRUE    //A70LK_Simon_0001
       : FALSE ;

  return bIs4K;
}

eHAL_WARPING_EXEC_CODE halWarping_PowerStandby(void)
{
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarpKeystoneAngle(float fAngH, float fAngV)
{
    //sPJPARAM sPJ; //A70LV_Larry_0001

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return eHAL_WARPING_EXEC_CODE_FAIL;

    m_sHalWarpingInfo.eWarpMode = HAL_WARPING_WARP_MODE_2x2;

    // Fill the Projector keystone parameters
    //sPJ.fAxis = 50; //A70LV_Larry_0001
    //sPJ.fDist = 3000; //A70LV_Larry_0001
    //sPJ.fVw = 1000; //A70LV_Larry_0001

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_SetWarpArc(INT16 iWidL, INT16 iWidR, INT16 iWidT, INT16 iWidB)
{
    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return eHAL_WARPING_EXEC_CODE_FAIL;

    return eHAL_WARPING_EXEC_CODE_PASS;
}


void halWarping_MoveWarp(eHAL_WARPING_DIRECTION eDir)
{
    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

#ifdef C821_WARPING_ENABLE
    dvPro_MoveWarp((ePROAV_WARP_DIRECTION)eDir, m_sHalWarpingInfo.fGridMovePitch,
            m_sHalWarpingInfo.ucGridSelX, m_sHalWarpingInfo.ucGridSelY,
            (ePROAV_WARP_MODE)m_sHalWarpingInfo.eWarpMode, m_sHalWarpingInfo.bInternalGridShow);

#endif
#ifdef C789_WARPING_ENABLE
    //dvC789_MoveWarp((eC789_WARP_DIRECTION)eDir, m_sHalWarpingInfo.fGridMovePitch,
            //m_sHalWarpingInfo.ucGridSelX, m_sHalWarpingInfo.ucGridSelY,
            //(eC789_WARP_MODE)m_sHalWarpingInfo.eWarpMode, m_sHalWarpingInfo.bInternalGridShow);

#endif
}

void halWarping_MovePitchChange(eHAL_WARPING_MOVE_PITCH eMoveIdx)
{
    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    if(eMoveIdx >= eHAL_WARPING_MOVE_PITCH_LAST)
        eMoveIdx = eHAL_WARPING_MOVE_PITCH_100;

    m_sHalWarpingInfo.eMovePitchIndex = eMoveIdx;

  	//Move Pitch
    m_sHalWarpingInfo.fGridMovePitch = (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_100) ? 100
                                         : (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_50) ? 50
                                         : (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_10) ? 10
                                         : (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_5) ? 5
                                         : (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_1) ? 1
                                         : (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_0P5) ? 0.5
                                         : (m_sHalWarpingInfo.eMovePitchIndex == eHAL_WARPING_MOVE_PITCH_0P1) ? 0.1
                                         : 0.0625;
}




///////////////////////////////////////////////////////////////////////////////////
///@brief halWarping_ChangeGrid: Draw change warp grid OSD
///@param BOOL bCursorOn - Display cursor or not
///@return void -
///@author Roger Chen
///////////////////////////////////////////////////////////////////////////////////

#ifdef WARP_ENABLE
void halWarping_OSD_DrawGrid(UINT16 uiX, UINT16 uiY, UINT8 ucGridSelX, UINT8 ucGridSelY)// write_grid() //A70LV_Larry_0137
{
    (void)uiX;
    (void)uiY;
    (void)ucGridSelX;
    (void)ucGridSelY;
#if 0
    UINT16 uiXStart = 0;
    UINT16 uiXWidth = 0;
    UINT16 uiYStart = 0;
    UINT16 uiYWidth = 0;

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    if((uiX == ucGridSelX) && (uiY == ucGridSelY))
    {
        uiXStart = (uiX == 0) ? 0 : (uiX == 16) ? (UINT16)m_sHalWarpingInfo.faCursorDefInX[uiX] - 1 - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                                    (UINT16)m_sHalWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);
        uiXWidth  = ((uiX == 0) || (uiX == 16)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;
        uiYStart = (uiY == 0) ? 0 : (uiY == 16) ? (UINT16)m_sHalWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                                    (UINT16)m_sHalWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);
        uiYWidth  = ((uiY == 0) || (uiY == 16)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;

        halGui_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 250, 252);

        uiXStart = (uiX == 0) ? uiXStart : uiXStart + 2;
        uiXWidth  = ((uiX == 0) || (uiX == 16)) ? uiXWidth - 2 : uiXWidth - 4;
        uiYStart = (uiY == 0) ? uiYStart : uiYStart + 2;
        uiYWidth  = ((uiY == 0) || (uiY == 16)) ? uiYWidth - 2 : uiYWidth - 4;

        halGui_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 252, 250);
    }

    uiXStart = (uiX == 0) ? 0 : (uiX == 16) ? (UINT16)m_sHalWarpingInfo.faCursorDefInX[uiX] - 1 -DEF_GRID_SIZE :
                        (UINT16)m_sHalWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE / 2);



    uiXWidth  = DEF_GRID_SIZE;


    uiYStart = (uiY == 0) ? 0 : (uiY == 16) ? (UINT16)m_sHalWarpingInfo.faCursorDefInY[uiY] - DEF_GRID_SIZE :
                        (UINT16)m_sHalWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE / 2);
    uiYWidth  = DEF_GRID_SIZE;

    halGui_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 250, 252);
#endif /* 0 */

    return;
}


void halWarping_OSD_EraseGrid(UINT16 uiX, UINT16 uiY)
{
    (void)uiX;
    (void)uiY;

#if 0
    // fCursorDefInX : define real coordinate in the 3840x2160 canvas
    UINT16 uiXStart = (uiX== 0) ? 0 : (uiX== DEF_GRID_SIZE) ? (UINT16)m_sHalWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                        (UINT16)m_sHalWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);

    UINT16 uiXWidth  = ((uiX == 0) || (uiX == DEF_GRID_SIZE)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;
    UINT16 uiYStart = (uiY == 0) ? 0 : (uiY == DEF_GRID_SIZE) ? (UINT16)m_sHalWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                            (UINT16)m_sHalWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);
    UINT16 uiYWidth  = ((uiY== 0) || (uiY == DEF_GRID_SIZE)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    // erase grid
    halGui_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 255, 255);
#endif /* 0 */

    return;
}
#endif /* WARP_ENABLE */


static void halWarping_DrawGrid(BOOL bGridShow)//change_grid
{
#if 1
    (void)bGridShow;
#else
#ifdef WARP_ENABLE
    COORDINATE sStart_Position;
    BLOCK_SIZE sDraw_Size;


    UINT8 ucIndex = 0, ucIndex2 = 0;

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    if(bGridShow == 0)
        return;

    for(ucIndex2 = 0; ucIndex2 < 17; ucIndex2++)
    {
        for(ucIndex = 0; ucIndex < 17; ucIndex++)
        {
            // baGridCursorEnabled : define in any warp mode which cursor can be shown
            // baGridCursorDisplayed : define in any warp mode which cursor already shown
            if(m_sHalWarpingInfo.baGridCursorEnabled[ucIndex][ucIndex2])
            {
			    halWarping_OSD_DrawGrid(ucIndex, ucIndex2, m_sHalWarpingInfo.ucGridSelX, m_sHalWarpingInfo.ucGridSelY); //A70LV_Larry_0137
            }
            else
            {
                halWarping_OSD_EraseGrid(ucIndex, ucIndex2); //A70LV_Larry_0137
            }
        }
    }

    m_sHalWarpingInfo.ucPrevGridSelX = m_sHalWarpingInfo.ucGridSelX;
    m_sHalWarpingInfo.ucPrevGridSelY = m_sHalWarpingInfo.ucGridSelY;


    sStart_Position.iX = 0;
    sStart_Position.iY = 0;
    sDraw_Size.iWidth = 1920;
    sDraw_Size.iHeight = 1080;
    halGui_OSD_On(sStart_Position, sDraw_Size);  //turn on full screen for splash display


    //dvPro_ShowGrid(bGridShow);

#endif
#endif /* 0 */

    return;
}

static void halWarping_WarpModeGridChange(INT8 cXDir, INT8 cYDir)//wpmd_grid_change
{
#if 1
(void)cXDir;
(void)cYDir;
#else
    UINT16 uiIndex, uiIndex2;
    BOOL bModeChange = ((cXDir == 0) && (cYDir == 0)) ? TRUE : FALSE;

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    if(bModeChange == TRUE)
    {
        // In the dvC790_WarpModeChange() already changed the baGridCursorEnabled setting.
        // In the old mode the current selected grid might visible, but in the new mode might not visible
        // Thus, change grid back to (0,0) to search again the first visible grid
        if(m_sHalWarpingInfo.baGridCursorEnabled[m_sHalWarpingInfo.ucGridSelX][m_sHalWarpingInfo.ucGridSelY] == FALSE)
        {
            m_sHalWarpingInfo.ucGridSelX = 0;
            m_sHalWarpingInfo.ucGridSelY = 0;
        }
    }
    else
    {
        do
        {
            uiIndex = (cXDir == 0) ? m_sHalWarpingInfo.ucGridSelX :
                        (cXDir > 0) ? (m_sHalWarpingInfo.ucGridSelX == 16) ? 16 : m_sHalWarpingInfo.ucGridSelX + 1 :
                        (m_sHalWarpingInfo.ucGridSelX == 0) ? 0 : m_sHalWarpingInfo.ucGridSelX - 1;
            uiIndex2 = (cYDir == 0) ? m_sHalWarpingInfo.ucGridSelY :
                        (cYDir > 0) ? (m_sHalWarpingInfo.ucGridSelY == 16) ? 16 : m_sHalWarpingInfo.ucGridSelY + 1 :
                        (m_sHalWarpingInfo.ucGridSelY == 0) ? 0 : m_sHalWarpingInfo.ucGridSelY - 1;

            if((uiIndex == m_sHalWarpingInfo.ucGridSelX) && (uiIndex2 == m_sHalWarpingInfo.ucGridSelY))
            {
                break;
            }

            m_sHalWarpingInfo.ucGridSelX = (UINT8)uiIndex;
            m_sHalWarpingInfo.ucGridSelY = (UINT8)uiIndex2;
        }
        // To search until the first grid point can be displayed found
        while(m_sHalWarpingInfo.baGridCursorEnabled[m_sHalWarpingInfo.ucGridSelX][m_sHalWarpingInfo.ucGridSelY] == FALSE);
    }
#endif /* 0 */

    return;
}

void halWarping_SelectGridChange(eHAL_WARPING_DIRECTION eDir)
{
    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    if(m_sHalWarpingInfo.bGridShow == FALSE)
        return;
#if 0
    switch(eDir)
    {
    case eHAL_WARPING_DIRECTION_UP:
        halWarping_WarpModeGridChange(0, -1);
        break;
    case eHAL_WARPING_DIRECTION_DOWN:
        halWarping_WarpModeGridChange(0, 1);
        break;
    case eHAL_WARPING_DIRECTION_LEFT:
        halWarping_WarpModeGridChange(-1, 0);
        break;
    case eHAL_WARPING_DIRECTION_RIGHT:
        halWarping_WarpModeGridChange(1, 0);
        break;
    }
#endif

#ifdef WARP_ENABLE
    halWarping_DrawGrid(m_sHalWarpingInfo.bGridShow);
#endif
}

#if 0
void halWarping_WarpModeChange(eHAL_WARPING_WARP_MODE eMode, BOOL bGridShow)
{
    UINT16 uiIndex, uiIndex2;
    BOOL bInternalCurEnable;

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    m_sHalWarpingInfo.eWarpMode = eMode;
    m_sHalWarpingInfo.bGridShow = bGridShow;

#ifdef C821_WARPING_ENABLE
    dvPro_WarpModeChange((ePROAV_WARP_MODE)eMode);

    for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
    {
        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            // To find if internal grid need to be diaplayed
            bInternalCurEnable = ((m_sHalWarpingInfo.bInternalGridShow == FALSE) &&
                (uiIndex > 0) && (uiIndex < 16) && (uiIndex2 > 0) && (uiIndex2 < 16)) ? FALSE : TRUE;

            if(m_sHalWarpingInfo.eWarpMode == ePROAV_WARP_MODE_2x2)
            {
                m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 16)) ? TRUE : FALSE;
                m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 16)) ?
                            (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;

            }
            else if(m_sHalWarpingInfo.eWarpMode == ePROAV_WARP_MODE_3x3)
            {
                if(bInternalCurEnable == FALSE)
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                }
                else
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 8) || (uiIndex == 16)) ? TRUE : FALSE;
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 8) || (uiIndex2 == 16)) ?
                                    (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;

                }
            }
            else if(m_sHalWarpingInfo.eWarpMode == ePROAV_WARP_MODE_5x5)
            {
                if(bInternalCurEnable == FALSE)
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                }
                else
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 4) || (uiIndex == 8) ||
                                        (uiIndex == 12) || (uiIndex == 16)) ? TRUE : FALSE;
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 4) || (uiIndex2 == 8) ||
                                        (uiIndex2 == 12) || (uiIndex2 == 16)) ?
                                        (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;
                }
            }
            else if(m_sHalWarpingInfo.eWarpMode == ePROAV_WARP_MODE_9x9)
            {
                if(bInternalCurEnable == FALSE)
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                }
                else
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 2) || (uiIndex == 4) ||
                                        (uiIndex == 6) || (uiIndex == 8) || (uiIndex == 10) || (uiIndex == 12) ||
                                        (uiIndex == 14) || (uiIndex == 16)) ? TRUE : FALSE;
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 2) || (uiIndex2 == 4) ||
                        (uiIndex2 == 6) || (uiIndex2 == 8) || (uiIndex2 == 10) || (uiIndex2 == 12) ||
                        (uiIndex2 == 14) || (uiIndex2 == 16)) ?
                                        (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;

                }
            }
            else
            {
                if(bInternalCurEnable == FALSE)
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                }
                else
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = TRUE;
                }
            }
        }
    }
#else
		(void)uiIndex;
		(void)uiIndex2;
        (void)bInternalCurEnable;
#endif /* 0 */
#ifdef C789_WARPING_ENABLE
        //dvC789_WarpModeChange((eC789_WARP_MODE)eMode);

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            for(uiIndex = 0; uiIndex < 17; uiIndex++)
            {
                // To find if internal grid need to be diaplayed
                bInternalCurEnable = ((m_sHalWarpingInfo.bInternalGridShow == FALSE) &&
                    (uiIndex > 0) && (uiIndex < 16) && (uiIndex2 > 0) && (uiIndex2 < 16)) ? FALSE : TRUE;

                if(m_sHalWarpingInfo.eWarpMode == eC789_WARP_MODE_2x2)
                {
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 16)) ? TRUE : FALSE;
                    m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 16)) ?
                                (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;

                }
                else if(m_sHalWarpingInfo.eWarpMode == eC789_WARP_MODE_3x3)
                {
                    if(bInternalCurEnable == FALSE)
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                    }
                    else
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 8) || (uiIndex == 16)) ? TRUE : FALSE;
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 8) || (uiIndex2 == 16)) ?
                                        (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;

                    }
                }
                else if(m_sHalWarpingInfo.eWarpMode == eC789_WARP_MODE_5x5)
                {
                    if(bInternalCurEnable == FALSE)
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                    }
                    else
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 4) || (uiIndex == 8) ||
                                            (uiIndex == 12) || (uiIndex == 16)) ? TRUE : FALSE;
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 4) || (uiIndex2 == 8) ||
                                            (uiIndex2 == 12) || (uiIndex2 == 16)) ?
                                            (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;
                    }
                }
                else if(m_sHalWarpingInfo.eWarpMode == eC789_WARP_MODE_9x9)
                {
                    if(bInternalCurEnable == FALSE)
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                    }
                    else
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex == 0) || (uiIndex == 2) || (uiIndex == 4) ||
                                            (uiIndex == 6) || (uiIndex == 8) || (uiIndex == 10) || (uiIndex == 12) ||
                                            (uiIndex == 14) || (uiIndex == 16)) ? TRUE : FALSE;
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 2) || (uiIndex2 == 4) ||
                            (uiIndex2 == 6) || (uiIndex2 == 8) || (uiIndex2 == 10) || (uiIndex2 == 12) ||
                            (uiIndex2 == 14) || (uiIndex2 == 16)) ?
                                            (m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] ? TRUE : FALSE) : FALSE;

                    }
                }
                else
                {
                    if(bInternalCurEnable == FALSE)
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = FALSE;
                    }
                    else
                    {
                        m_sHalWarpingInfo.baGridCursorEnabled[uiIndex][uiIndex2] = TRUE;
                    }
                }
            }
        }
#else
        (void)uiIndex;
        (void)uiIndex2;
        (void)bInternalCurEnable;
#endif /* 0 */


    halWarping_WarpModeGridChange(0, 0);
    halWarping_DrawGrid(bGridShow);

}
#endif

void halWarping_CrossHatchEnable(eHAL_WARPING_CROSSHATCH_MODE eMode, UINT16 uiHW, UINT16 uiVW)
{
    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return;

    m_sHalWarpingInfo.ePatternGen = eMode;
    m_sHalWarpingInfo.uiBlendingEdgeHW = uiHW;
    m_sHalWarpingInfo.uiBlendingEdgeVW= uiVW;
}


void halWarping_BlankOutput(UINT8 ucEnable)
{
#if 0
    dvC790_OutputFillSet(ucEnable) ;
#endif
}


INT16 halWarping_MovePitch_Get(void)    //A70LV_Doulas_0008
{
    return (INT16)m_sHalWarpingInfo.eMovePitchIndex;
}


INT16 halWarping_WarpMode_Get(void)     //A70LV_Doulas_0008
{
    return (INT16)m_sHalWarpingInfo.eWarpMode;
}


INT16 halWarping_WarpMode_GridShow_Get(void)    //A70LV_Doulas_0008
{
    return (INT16)m_sHalWarpingInfo.bGridShow;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarpPincushion(UINT8 ucPincushionH, UINT8 ucPincushionV)   //A70LV_Doulas_0020
{

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarp_4Corner(UINT16 uiTopLeftHorz,
                                                  UINT16 uiTopLeftVert,
                                                  UINT16 uiTopRightHorz,
                                                  UINT16 uiTopRightVert,
                                                  UINT16 uiBottomLeftHorz,
                                                  UINT16 uiBottomLeftVert,
                                                  UINT16 uiBottomRightHorz,
                                                  UINT16 uiBottomRightVert)
{

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_SetWarp_AutoFilter(UINT8 ucAutoFilter,UINT8 *ucHorzFilter,UINT8 *ucVertFilter)
{
    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter = ucAutoFilter; //A70LV_Larry_0060
//HICC2_Steven_0041 start
    //*ucHorzFilter = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H; //A70LV_Larry_0060
    //*ucVertFilter = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V; //A70LV_Larry_0060

#if (ADAPTIVE_SCALE_WARP_FILTER_LUT == FALSE)   //A35G2_Simon_0115
    *ucHorzFilter =  m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H; //A70LV_Larry_0060
    *ucVertFilter =  m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V; //A70LV_Larry_0060
#else
    m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H = *ucHorzFilter;
    m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V = *ucVertFilter;
#endif
//HICC2_Steven_0041 end
    //m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H = *ucHorzFilter;
    //m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V = *ucVertFilter;

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

    m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter = FALSE; //A70LV_Larry_0060
    m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H = ucHorzFilter; //A70LV_Larry_0060
    m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V = ucVertFilter; //A70LV_Larry_0060
    #ifdef WARP_ENABLE
    halWarp_InterpolationSet(); //A70LV_Larry_0060
    #endif /* WARP_ENABLE */
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
void halWarp_SetLut(void);

#ifdef _CASCADE_
void halWarp_CalcNoWarp(UINT8 ucTblIdx);
void halWarp_Casc_bilinear(void);
void halWarp_CalcPin(float WidH, float WidV, int TblIdx);
void halWarp_Calc_oldtable(int mode);
int halWarp_CheckWpLimit(const str_wplimit *lim);
void halWarp_CascWpTable(int Premode);
#endif

#if 0
  UINT8 halWarp_Calclut(float fRatio, UINT8 ucSymbol, UINT8 *pucLUT) //A70LV_Larry_0060
  {
      (void)pucLUT;
      if(fRatio < 0.4f)
      {
          return 0;
      }
      else if(fRatio < 0.5f)
      {
          return 1;
      }
      else if(fRatio < 0.6f)
      {
          return 2;
      }
      else if(fRatio < 0.7f)
      {
          return 3;
      }
      else if(fRatio < 0.8f)
      {
          return 4;
      }
      else if(fRatio < 0.9f)
      {
          return 5;
      }
      else if(fRatio < 1.0f)
      {
          return 6;
      }
      else if(fRatio < 1.1f)
      {
          return 7;
      }
      else if(fRatio < 1.5f)
      {
          return 8;
      }
      else
      {
          return ucSymbol;
      }

  }
#endif /* 0 */

#if 1
UINT8 halWarp_Calclut(float fRatioH, float fRatioV)
{
    float fRatio = (fRatioH < fRatioV) ? fRatioH:fRatioV;

    if(fRatio < 0.5f)
    {
        return 10;
    }
    else if(fRatio < 0.6f)
    {
        return 11;
    }
    else if(fRatio < 0.7f)
    {
        return 12;
    }
    else if(fRatio < 0.8f)
    {
        return 13;
    }
    else if(fRatio < 0.9f)
    {
        return 14;
    }
    else if(fRatio <= 1.0f)
    {
        return 15;
    }
    else if(fRatio < 1.1f)
    {
        return 16;
    }
    else
    {
        return 17;
    }
}
#endif /* 0 */

void halWarp_SetLut(void)
{
#if 0
    //UINT8   ucHLUT[24], ucVLUT[24];
    INT16  uiCount = 0;
    float   fHscl, fVscl;//, hwin, vwin;
    float   fMax, fR;

    // +++++ HLUT +++++ //
    fMax = 1;

    for(uiCount = m_sHalWarpingInfo.uiWarp_VW16; uiCount >= 0; uiCount--)
    {
        fR = fabsf(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][uiCount] - m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][uiCount]);

        if(fMax < fR)
        {
            fMax = fR;
        }
    }

    fHscl = (float)(m_sHalWarpingInfo.uiWarp_HW / fMax);
    fHscl = (fHscl > DEF_LUTCUT_MAX) ? DEF_LUTCUT_MAX : (fHscl < DEF_LUTCUT_MIN)? DEF_LUTCUT_MIN : fHscl; // limit
    //hwin = (hscl < 1.0f) ? 0 : hscl / 2.0f + 0.5f;
    //hwin = (hwin > DEF_LUTWIN_MAX) ? DEF_LUTWIN_MAX : (hwin < DEF_LUTWIN_MIN)? DEF_LUTWIN_MIN : hwin; // limit

    ///m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H = halWarp_Calclut(fHscl, DEF_LUTSYM, NULL); //A70LV_Larry_0060

    // +++++ VLUT +++++ //
    fMax = 1;

    for(uiCount = m_sHalWarpingInfo.uiWarp_HW16; uiCount >= 0; uiCount--)
    {
        fR = fabsf(m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][uiCount][m_sHalWarpingInfo.uiWarp_VW16] - m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][uiCount][0]);

        if(fMax < fR)
        {
            fMax = fR;
        }
    }

    fVscl = (float)(m_sHalWarpingInfo.uiWarp_VW / fMax);
    fVscl = (fVscl > DEF_LUTCUT_MAX) ? DEF_LUTCUT_MAX : (fVscl < DEF_LUTCUT_MIN)? DEF_LUTCUT_MIN : fVscl; // limit
    //vwin = (vscl < 1.0f) ? 0 : vscl / 2.0f + 0.5f;
    //vwin = (vwin > DEF_LUTWIN_MAX) ? DEF_LUTWIN_MAX : (vwin < DEF_LUTWIN_MIN)? DEF_LUTWIN_MIN : vwin; // limit

    //m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_V = halWarp_Calclut(fVscl, DEF_LUTSYM, NULL); //A70LV_Larry_0060

    m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H = halWarp_Calclut(fHscl, fVscl);
#endif /* 0 */

#ifdef WARP_ENABLE
    halWarp_InterpolationSet();
#endif /* WARP_ENABLE */

    return;
}

#define DRAM2WARPING

UINT16 halWarp_WriteWpTable(void) //A70LH_Larry_0124 add write check sum
{
#ifdef WARP_ENABLE
    int x, y;
    int dtdtx, dtdty;
    int     xmax, phwdmy, dmy16, xsft;
    float   tx, ty;
    float   phw_ru16;
    float   xstmin, xedmax;
    float   frx, brx, phwdmy16;


    UINT8 aucData[6] = {0}; //A70LV_Larry_0061
    UINT32 ulTableSize = 0;

    //===== for dummy area process =====//
    phw_ru16 = (float)(m_sHalWarpingInfo.uiWarp_HW16 << 4);
    xstmin   = 1000000;
    xedmax   = 0;

    for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
    {
        // x = 0
        if(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][y] < xstmin)
        {
            if(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][y] < 0)
            {
                xstmin = 0;
            }
            else
            {
                xstmin = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][y];
            }
        }

        // x = m_sHalWarpingInfo.uiWarp_HW16
        if(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y] > xedmax)
        {
            if(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y] > phw_ru16)
            {
                xedmax = phw_ru16;
            }
            else
            {
                xedmax = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y];
            }
        }
    }

    //===== for writing warp table =====//

   	////dvPro_Write(B0_RTCT, 0x000000, 0 );


    ////dvPro_Write(B0_RTCT, (RTCT_POVSSTOP & RTCT_WARP_TRANSFER_DISABLE), 0);
    //RTCT_STOP

    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        LOG_MSG(db_DV_PROAV_WARPING, "Warping Table Start 4K\r\n");
    }
    else
    {
        LOG_MSG(db_DV_PROAV_WARPING, "Warping Table Start 2K\r\n");
    }


    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

#ifndef DRAM2WARPING
        dvProAV_AccessBurstAddrIncEnable(FALSE);
#endif /* DRAM2WARPING */

        frx      = (float)(DEF_WPDMYHWF >> 4);
        brx      = (float)(DEF_WPDMYHWB >> 4);
        xmax     = m_sHalWarpingInfo.uiWarp_HW16 + (DEF_WPDMYHWF >> 4) + (DEF_WPDMYHWB >> 4);
        phwdmy   = m_sHalWarpingInfo.uiWarp_HW + DEF_WPDMYHWF;
        phwdmy16 = m_sHalWarpingInfo.uiWarp_HW16  + frx;
        dmy16    = (DEF_WPDMYHWF >> 4);

        if(m_sHalWarpingInfo.ucIs4KWarp)
        {
            dvProAV_AccessWrite(eWarpFrac3bits, 1); //4K
        }
        else
        {
            dvProAV_AccessWrite(eWarpFrac3bits, 0); //2K
        }
#ifndef DRAM2WARPING
        dvProAV_AccessWrite(eWarpWpTableWrite, true);
#endif /* DRAM2WARPING */
        //dvProAV_WarpScalingSet(eOriRatio, eOriRatio);
        //dvProAV_AccessWrite(eWarpWpManualYsd,   0);
        //dvProAV_AccessWrite(eWarpWpManualXsd,   0);

        //taskENTER_CRITICAL();

        for(y = 0; y <= m_sHalWarpingInfo.uiWarp_VW16; y++)
        {

            for(x = 0; x <= xmax; x++)
            {
                //===== for dummy area process =====//
                if((x << 4) < DEF_WPDMYHWF)
                {
                    if(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][y] < 0)
                    {
                        tx = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][y];
                    }
                    else
                    {
                        tx = (m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][0][y] - xstmin)/ frx * x + xstmin;
                    }

                    ty = (m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][0][y]);
                }
                else if((x << 4) > phwdmy)
                {
                    if(m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y] > m_sHalWarpingInfo.uiWarp_HW)
                    {
                        tx = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y];
                    }
                    else
                    {
                        tx = (xedmax - m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y]) / brx * (x - phwdmy16) + m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y];
                    }

                    ty = (m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][m_sHalWarpingInfo.uiWarp_HW16][y]);
                }
                else
                {
                    xsft = x - dmy16;
                    tx = (m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][xsft][y]);
                    ty = (m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][xsft][y]);
                }

                //==================================//

                if(m_sHalWarpingInfo.ucIs4KWarp)
                {
                    if(tx> 6143)
                    {
                        dtdtx = 0xbfff;
                    }
                    else if(tx<-2048)
                    {
                        dtdtx = 0xc000;
                    }
                    else
                    {
                        dtdtx = (int)(tx*8+0.5f);
                    }

                    if(ty> 6143)
                    {
                        dtdty = 0xbfff;
                    }
                    else if(ty<-2048)
                    {
                        dtdty = 0xc000;
                    }
                    else
                    {
                        dtdty = (int)(ty*8+0.5f);
                    }
                }
                else
                {
                    dtdtx = (int)(tx * 16 + 0.5f);
                    dtdty = (int)(ty * 16 + 0.5f);

                    // clip
                    if(tx > 2060)
                    {
                        dtdtx = 0x80c0;
                    }

                    if(ty > 3071)
                    {
                        dtdty = 0xbfff;
                    }

                    if(tx < -1024)
                    {
                        dtdtx = 0xc000;
                    }

                    if(ty < -1024)
                    {
                        dtdty = 0xc000;
                    }
                }

                aucData[0] = (dtdtx & 0xff);
                aucData[1] = ((dtdtx >> 8) & 0xff);
                aucData[2] = (dtdty & 0xff);
                aucData[3] = ((dtdty >> 8) & 0xff);

#ifndef DRAM2WARPING
                //dvProAV_AccessBurstWrite2(eWarpWpTablePort,  aucData, 4, BURST_FIX_ADDR);
                dvProAV_AccessBurstWrite(eWarpWpTablePort,  aucData, 4, BURST_FIX_ADDR);
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[0];
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[1];
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[2];
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[3];

#else
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[0];
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[1];
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[2];
                m_sHalWarpingInfo.acWarpTable[ulTableSize++] = aucData[3];
#endif /* DRAM2WARPING */

                //dvProAV_AccessWrite(eWarpWpTablePort,  (UINT32)aucData[0]);
                //dvProAV_AccessWrite(eWarpWpTablePort,  (UINT32)aucData[1]);
                //dvProAV_AccessWrite(eWarpWpTablePort,  (UINT32)aucData[2]);
                //dvProAV_AccessWrite(eWarpWpTablePort,  (UINT32)aucData[3]);


                //if(utilDbgMsg_Get(db_DV_PROAV_WARPING))
                //{
                    //LOG_MSG(db_DV_PROAV_WARPING, "0x%02x 0x%02x 0x%02x 0x%02x\r\n", aucData[0], aucData[1], aucData[2], aucData[3]);
                    //utilDelayMs(10);
                //}

            } // end for x
        } // end for y

        //taskEXIT_CRITICAL();

#ifndef DRAM2WARPING
        dvProAV_AccessWrite(eWarpWpTableWrite, false);
#else
        dvProAV_WarpTableDl2Chip(ulTableSize, &m_sHalWarpingInfo.acWarpTable[0]);
#endif /* DRAM2WARPING */

        LOG_MSG(db_DV_PROAV_WARPING, "Warping Table End\r\n");

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        ASSERT_ALWAYS();
    }
#endif /* WARP_ENABLE */

    return 0;
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

    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
        {
            for(x = m_sHalWarpingInfo.uiWarp_HW16; x >= 0; x--)
            {
                hg_inv0 = hg_sys7 * (x << DEF_WP_SPACE_4K_BITS) - hg_sys1;
                hg_inv1 = hg_sys0 - hg_sys6 * (x << DEF_WP_SPACE_4K_BITS);
                hg_inv1 = (hg_inv1 < 0.000001f) ? 0.000001f : hg_inv1;
                hg_inv2 = (x << DEF_WP_SPACE_4K_BITS) - hg_sys2;
                hg_inv3 = hg_sys6 * (y << DEF_WP_SPACE_4K_BITS) - hg_sys3;
                hg_inv4 = hg_sys4 - hg_sys7 * (y << DEF_WP_SPACE_4K_BITS);
                hg_inv4 = (hg_inv4 < 0.000001f) ? 0.000001f : hg_inv4;
                hg_inv5 = (y << DEF_WP_SPACE_4K_BITS) - hg_sys5;

                tmp = 1 - (hg_inv3 * hg_inv0) / (hg_inv4 * hg_inv1);
                tmp = (tmp < 0.000001f) ? 0.000001f : tmp;

                WY  = FVW * (hg_inv3 * hg_inv2 + hg_inv1 * hg_inv5) / (hg_inv4 *hg_inv1) / tmp;
                WX  = (FHW / hg_inv1) * ((hg_inv0 * WY / FVW) + hg_inv2);

                m_sHalWarpingInfo.sWarpTable.Px[ucTblIdx][x][y] = (WX > 65536) ? 65535 : (WX < -65536) ? -65536 : WX; // 4096*16 = 65536
                m_sHalWarpingInfo.sWarpTable.Py[ucTblIdx][x][y] = (WY > 65536) ? 65535 : (WY < -65536) ? -65536 : WY; // 4096*16 = 65536
            }
        }
    }
    else
    {
        for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
        {
            for(x = m_sHalWarpingInfo.uiWarp_HW16; x >= 0; x--)
            {
                hg_inv0 = hg_sys7 * (x << DEF_WP_SPACE_2K_BITS) - hg_sys1;
                hg_inv1 = hg_sys0 - hg_sys6 * (x << DEF_WP_SPACE_2K_BITS);
                hg_inv1 = (hg_inv1 < 0.000001f) ? 0.000001f : hg_inv1;
                hg_inv2 = (x << DEF_WP_SPACE_2K_BITS) - hg_sys2;
                hg_inv3 = hg_sys6 * (y << DEF_WP_SPACE_2K_BITS) - hg_sys3;
                hg_inv4 = hg_sys4 - hg_sys7 * (y << DEF_WP_SPACE_2K_BITS);
                hg_inv4 = (hg_inv4 < 0.000001f) ? 0.000001f : hg_inv4;
                hg_inv5 = (y << DEF_WP_SPACE_2K_BITS) - hg_sys5;

                tmp = 1 - (hg_inv3 * hg_inv0) / (hg_inv4 * hg_inv1);
                tmp = (tmp < 0.000001f) ? 0.000001f : tmp;

                WY  = FVW * (hg_inv3 * hg_inv2 + hg_inv1 * hg_inv5) / (hg_inv4 *hg_inv1) / tmp;
                WX  = (FHW / hg_inv1) * ((hg_inv0 * WY / FVW) + hg_inv2);

                m_sHalWarpingInfo.sWarpTable.Px[ucTblIdx][x][y] = (WX > 32767) ? 32767 : (WX < -32768) ? -32768 : WX; // 2048 = 65536
                m_sHalWarpingInfo.sWarpTable.Py[ucTblIdx][x][y] = (WY > 32767) ? 32767 : (WY < -32768) ? -32768 : WY; // 2048= 65536
            }
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
    p4c->tl_x = p4c->tl_x + (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->tl_y = p4c->tl_y + (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
    p4c->tr_x = p4c->tr_x - (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->tr_y = p4c->tr_y + (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
    p4c->bl_x = p4c->bl_x + (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->bl_y = p4c->bl_y - (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
    p4c->br_x = p4c->br_x - (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX * m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
    p4c->br_y = p4c->br_y - (m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY * m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);

    LOG_MSG(db_HAL_WARPING, "(%s, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)\r\n", __FUNCTION__, __LINE__, p4c->tl_x,p4c->tl_y,p4c->tr_x,p4c->tr_y,p4c->bl_x,p4c->bl_y,p4c->br_x,p4c->br_y);


    return halWarp_Check_curpos(p4c);

}

UINT8 halWarp_SetWpKeyAng(float AngH, float AngV)
{
    UINT8 ucWperr = 0;
    //BYTE cCount = 0; //A70LH_Larry_0124 add

    halWarp_Calc4Corner((float)AngH, (float)AngV, &PS_PJPARAM, &m_sWArp4CPara.WPPOS4C);

    ucWperr = halWarp_FourCornerShift(&m_sWArp4CPara.WPPOS4C); //A70LH_Larry_0141

    if(ucWperr)
    {
        return 0;
    }

    halWarp_CalcHomography(&m_sWArp4CPara.WPPOS4C, DEF_WPTBLIDX_NEW);

#ifdef _CASCADE_
    halWarp_CascWpTable(WPHIST[1]); // pre1
    halWarp_CascWpTable(WPHIST[2]); // pre2
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
    float   yc, xc, yv, vV, vH, xh, hH;
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
    yc = 0; //A70LK_Casper_0001

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
    halWarp_Rotate(pi  , angh, angv, 0, &m_sWArp4CPara.pos_r);
    m_sWArp4CPara.pos_o[0].x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    m_sWArp4CPara.pos_o[0].y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;
    m_sWArp4CPara.pos_a[0].z = pi->z;     //!

    halWarp_Rotate(pi + 1, angh, angv, 0, &m_sWArp4CPara.pos_r);
    m_sWArp4CPara.pos_o[1].x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    m_sWArp4CPara.pos_o[1].y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;
    m_sWArp4CPara.pos_a[1].z = (pi + 1)->z;   //!

    halWarp_Rotate(pi + 2, angh, angv, 0, &m_sWArp4CPara.pos_r);
    m_sWArp4CPara.pos_o[2].x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    m_sWArp4CPara.pos_o[2].y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;
    m_sWArp4CPara.pos_a[2].z = (pi + 2)->z;   //!m_stWarpConfig.stOsdBasic.stWArpGeoPara.

    halWarp_Rotate(pi + 3, angh, angv, 0, &m_sWArp4CPara.pos_r);
    m_sWArp4CPara.pos_o[3].x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    m_sWArp4CPara.pos_o[3].y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;
    m_sWArp4CPara.pos_a[3].z = (pi + 3)->z;   //!

    /*aspect conversion*/
    halWarp_Calc_Aspect(m_sWArp4CPara.pos_o, angh, angv, m_sWArp4CPara.pos_a, &ucCP);

    //rotate_rev.c
    halWarp_Rotate(m_sWArp4CPara.pos_a  , angh, angv, 1, &m_sWArp4CPara.pos_r);
    fp1x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    fp1y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;

    halWarp_Rotate(m_sWArp4CPara.pos_a + 1, angh, angv, 1, &m_sWArp4CPara.pos_r);
    fp2x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    fp2y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;

    halWarp_Rotate(m_sWArp4CPara.pos_a + 2, angh, angv, 1, &m_sWArp4CPara.pos_r);
    fp3x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    fp3y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;

    halWarp_Rotate(m_sWArp4CPara.pos_a + 3, angh, angv, 1, &m_sWArp4CPara.pos_r);
    fp4x = m_sWArp4CPara.pos_r.x * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.x;
    fp4y = m_sWArp4CPara.pos_r.y * (pi->z - m_sWArp4CPara.pos_r.z) / m_sWArp4CPara.pos_r.z + m_sWArp4CPara.pos_r.y;

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
    dnratio = (pjp->axis == 0) ? 50 : 0; //offset 120%
    upratio = 100 - dnratio; //1.2

    pz = (float)m_sHalWarpingInfo.uiWarp_VW / (float)pjp->vw * (float)pjp->dist;

    m_sWArp4CPara.pos_i[0].x = (float)(-m_sHalWarpingInfo.uiWarp_HW / 2);
    m_sWArp4CPara.pos_i[0].y = (float)(m_sHalWarpingInfo.uiWarp_VW) * upratio / 100.0f;
    m_sWArp4CPara.pos_i[0].z = pz;
    m_sWArp4CPara.pos_i[1].x = (float)(m_sHalWarpingInfo.uiWarp_HW / 2);
    m_sWArp4CPara.pos_i[1].y = m_sWArp4CPara.pos_i[0].y;
    m_sWArp4CPara.pos_i[1].z = pz;
    m_sWArp4CPara.pos_i[2].x = m_sWArp4CPara.pos_i[0].x;
    m_sWArp4CPara.pos_i[2].y = (float)(-m_sHalWarpingInfo.uiWarp_VW) * dnratio / 100.0f;
    m_sWArp4CPara.pos_i[2].z = pz;
    m_sWArp4CPara.pos_i[3].x = m_sWArp4CPara.pos_i[1].x;
    m_sWArp4CPara.pos_i[3].y = m_sWArp4CPara.pos_i[2].y;
    m_sWArp4CPara.pos_i[3].z = pz;

    halWarp_Angle_to_coord(m_sWArp4CPara.pos_i, AngH, AngV, p4c);

    return;
}

#ifdef _CASCADE_
void halWarp_CalcNoWarp(UINT8 ucTblIdx)
{
    INT32 uiX, uiY;
    UINT8 ucSfiftBit = 0;

    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        ucSfiftBit = DEF_WP_SPACE_4K_BITS;
    }
    else
    {
        ucSfiftBit = DEF_WP_SPACE_2K_BITS;
    }

    for(uiY = (DEF_WPTBLMAX_V -1); uiY >= 0; uiY--)
    {
        for(uiX = (DEF_WPTBLMAX_H -1); uiX >= 0; uiX--)
        {
            m_sHalWarpingInfo.sWarpTable.Px[ucTblIdx][uiX][uiY] = (uiX << ucSfiftBit);
            m_sHalWarpingInfo.sWarpTable.Py[ucTblIdx][uiX][uiY] = (uiY << ucSfiftBit);
        }
    }

    return;
}

void halWarp_Casc_bilinear(void)
{
    int     x, y;
    int     x0, x1, y0, y1;
    float   a, b;

    float   ix_new16, iy_new16;
    float   ix_old00, ix_old10, ix_old01, ix_old11;
    float   iy_old00, iy_old10, iy_old01, iy_old11;
    float   diff0, diff1;
    float   dDefSpace = 0.0;

    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        dDefSpace = (float)DEF_WP_SPACE_4K;
    }
    else
    {
        dDefSpace = (float)DEF_WP_SPACE_2K;
    }

    for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
    {
        for(x = m_sHalWarpingInfo.uiWarp_HW16; x >= 0; x--)
        {

            ix_new16 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y] / dDefSpace;
            iy_new16 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y] / dDefSpace;

            // X-left
            x0 = (int)(ix_new16);
            x0 = (x0 < 0) ? 0 : (x0 > (m_sHalWarpingInfo.uiWarp_HW16 - 1)) ? (m_sHalWarpingInfo.uiWarp_HW16 - 1) : x0;
            // X-right
            x1 = x0 + 1;
            // Y-top
            y0 = (int)(iy_new16);
            y0 = (y0 < 0) ? 0 : (y0 > (m_sHalWarpingInfo.uiWarp_VW16 - 1)) ? (m_sHalWarpingInfo.uiWarp_VW16 - 1) : y0;
            // Y-bottom
            y1 = y0 + 1;

            // sub-pixel
            a  = ix_new16 - x0;
            b  = iy_new16 - y0;

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
    float   sclx[DEF_WPTBLMAX_V] = {0}, scly[DEF_WPTBLMAX_H] = {0}; //A70LK_Casper_0001
    float   FHW, FVW;
    UINT8   ucSfiftBit = 0;

    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        ucSfiftBit = DEF_WP_SPACE_4K_BITS;
    }
    else
    {
        ucSfiftBit = DEF_WP_SPACE_2K_BITS;
    }

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

    for(x = m_sHalWarpingInfo.uiWarp_HW16; x >= 0; x--)
    {
        dist = sa * ((x << ucSfiftBit) - m_sHalWarpingInfo.fWarp_PHWHF) * ((x << ucSfiftBit) - m_sHalWarpingInfo.fWarp_PHWHF) + FVW + tWidV2;
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

    for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
    {
        dist = sa * ((y << ucSfiftBit) - m_sHalWarpingInfo.fWarp_PVWHF) * ((y << ucSfiftBit) - m_sHalWarpingInfo.fWarp_PVWHF) + FHW + tWidH2;
        sclx[y] = dist / FHW;
    }

    for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
    {
        for(x = m_sHalWarpingInfo.uiWarp_HW16; x >= 0; x--)
        {
    	     m_sHalWarpingInfo.sWarpTable.Px[TblIdx][x][y] = ((x << ucSfiftBit) - m_sHalWarpingInfo.fWarp_PHWHF) * sclx[y] + m_sHalWarpingInfo.fWarp_PHWHF;
             m_sHalWarpingInfo.sWarpTable.Py[TblIdx][x][y] = ((y << ucSfiftBit) - m_sHalWarpingInfo.fWarp_PVWHF) * scly[x] + m_sHalWarpingInfo.fWarp_PVWHF;
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
            halWarp_CalcHomography(&m_sWArp4CPara.WPPOS4C, DEF_WPTBLIDX_OLD);
            break;

        case DEF_WPMODE_K4C  :
            halWarp_CalcHomography(&m_sWArp4CPara.WPPOS4C, DEF_WPTBLIDX_OLD);
            break;

        case DEF_WPMODE_PIN  :
            halWarp_CalcPin(m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H, m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V, DEF_WPTBLIDX_OLD);
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
    int     x, y, x1, y1;
    float   ix_new00, ix_new10, ix_new01, ix_new11;
    float   iy_new00, iy_new10, iy_new01, iy_new11;
    float   diffx_x, diffy_x, diffx_y, diffy_y, diffy_y_sum;
    float   tan_h, tan_v;
    float   divn;

    for(y = m_sHalWarpingInfo.uiWarp_VW16; y >= 0; y--)
    {
        divn = 0;
        diffy_y_sum = 0;

        for(x = m_sHalWarpingInfo.uiWarp_HW16; x >= 0; x--)
        {
            if((x < m_sHalWarpingInfo.uiWarp_HW16) || (y < m_sHalWarpingInfo.uiWarp_VW16))
            {
                x1 = (x < m_sHalWarpingInfo.uiWarp_HW16) ? x + 1 : x;
                y1 = (y < m_sHalWarpingInfo.uiWarp_VW16) ? y + 1 : y;

                ix_new00 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y];
                ix_new10 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x1][y];
                ix_new01 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x][y1];
                ix_new11 = m_sHalWarpingInfo.sWarpTable.Px[DEF_WPTBLIDX_NEW][x1][y1];
                iy_new00 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y];
                iy_new10 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x1][y];
                iy_new01 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x][y1];
                iy_new11 = m_sHalWarpingInfo.sWarpTable.Py[DEF_WPTBLIDX_NEW][x1][y1];

                // Valid area
                if(!(((ix_new00 < 0) && (ix_new10 < 0) && (ix_new01 < 0) && (ix_new11 < 0))
                     || ((iy_new00 < 0) && (iy_new10 < 0) && (iy_new01 < 0)&& (iy_new11 < 0))
                     || ((ix_new00 > m_sHalWarpingInfo.uiWarp_HW) && (ix_new10 > m_sHalWarpingInfo.uiWarp_HW) && (ix_new01 > m_sHalWarpingInfo.uiWarp_HW) && (ix_new11 > m_sHalWarpingInfo.uiWarp_HW))
                     || ((iy_new00 > m_sHalWarpingInfo.uiWarp_VW) && (iy_new10 > m_sHalWarpingInfo.uiWarp_VW) && (iy_new01 > m_sHalWarpingInfo.uiWarp_VW) && (iy_new11 > m_sHalWarpingInfo.uiWarp_VW))))
                {
                    if(x < m_sHalWarpingInfo.uiWarp_HW16)
                    {
                        diffx_x = ix_new10 - ix_new00;
                        diffy_x = iy_new10 - iy_new00;

                        // Horizontal slope
                        tan_h = diffy_x / diffx_x;

                        if((tan_h < (-lim->ang_h)) || (tan_h > lim->ang_h))
                        {
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y * 16), ix_new00, iy_new00, (x1* 16), (y * 16), ix_new10, iy_new10);
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y1 * 16), ix_new01, iy_new01, (x1* 16), (y1 * 16), ix_new11, iy_new11);
                            //LOG_MSG(eDB_MSK_LIST_C786, "diffx_x=%f, diffy_x=%f, tan_h=%f\r\n", diffx_x, diffy_x, tan_h);
                            return DEF_WPERRANG_H;
                        }

                        // Horizontal shrink rate
                        if(diffx_x > lim->hsh)
                        {
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y * 16), ix_new00, iy_new00, (x1* 16), (y * 16), ix_new10, iy_new10);
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y1 * 16), ix_new01, iy_new01, (x1* 16), (y1 * 16), ix_new11, iy_new11);
                            //LOG_MSG(eDB_MSK_LIST_C786, "diffx_x=%f\r\n",diffx_x);
                            return DEF_WPERRHSH;
                        }
                    } // end if(x<m_sHalWarpingInfo.uiWarp_HW16)

                    if(y < m_sHalWarpingInfo.uiWarp_VW16)
                    {
                        diffx_y = ix_new01 - ix_new00;
                        diffy_y = iy_new01 - iy_new00;

                        // Vertical local shrink rate
                        if(diffy_y > lim->vsh_l)
                        {
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y * 16), ix_new00, iy_new00, (x1* 16), (y * 16), ix_new10, iy_new10);
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y1 * 16), ix_new01, iy_new01, (x1* 16), (y1 * 16), ix_new11, iy_new11);
                            //LOG_MSG(eDB_MSK_LIST_C786, "diffy_y=%f\r\n",diffy_y);
                            return DEF_WPERRVSH_L;
                        }

                        // Vertical slope
                        tan_v = diffx_y / diffy_y;

                        if((tan_v < (-lim->ang_v)) || (tan_v > lim->ang_v))
                        {
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y * 16), ix_new00, iy_new00, (x1* 16), (y * 16), ix_new10, iy_new10);
                            //LOG_MSG(eDB_MSK_LIST_C786, "(%4d,%4d)=(%6.2f,%6.2f), (%4d,%4d)=(%6.2f,%6.2f)\r\n", (x * 16), (y1 * 16), ix_new01, iy_new01, (x1* 16), (y1 * 16), ix_new11, iy_new11);
                            //LOG_MSG(eDB_MSK_LIST_C786, "diffy_y=%f, diffx_y=%f, tan_v=%f\r\n", diffy_y, diffx_y, tan_v);
                            return DEF_WPERRANG_V;
                        }

                        diffy_y_sum = diffy_y_sum + diffy_y;
                        divn = divn + 1;
                    } // end if(y<m_sHalWarpingInfo.uiWarp_VW16)
                } // end if(Px,Py)
            } // end if((x<m_sHalWarpingInfo.uiWarp_HW16)||(y<m_sHalWarpingInfo.uiWarp_VW16))
        } // end for x

        // Vertical average shrink rate
        if((diffy_y_sum / divn) > lim->vsh_a)
        {
            //LOG_MSG(eDB_MSK_LIST_C786, "diffy_y_sum=%f, divn=%f\r\n",diffy_y_sum, divn);
            return DEF_WPERRVSH_A;
        }
    } // end for y

    return 0;
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

    WPMODE = WPMODE;    //A70LV_Doulas_0088
    WPON = WPON;        //A70LV_Doulas_0088
    WPMODE = DEF_WPMODE_KANG;

    WPHIST[2] = DEF_WPMODE_TH;
    WPHIST[1] = DEF_WPMODE_PIN;
    WPHIST[0] = DEF_WPMODE_KANG;


    ucWperr = halWarp_SetWpKeyAng(m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H, m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V);

    //ucWperr = halWarp_CheckWpLimit(&PS_WPLIMIT);

    if(!ucWperr)
    {
        halWarp_WriteWpTable();
        halWarp_SetLut();
    }

    if(ucWperr)
    {
        LOG_MSG(db_HAL_WARPING, "Over limits!! [ERR_CODE=%d]\r\n", ucWperr);
        m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H = m_stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_H_OLD;
        m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V = m_stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_V_OLD;
    }
    else
    {
        WPON = 1;
        m_stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_H_OLD = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H;
        m_stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_V_OLD = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V;
    }

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
            UINT8 ucTemp =  m_stWarpConfig.stOsdBasic.stWArpGeoPara.TABLE_COLOR; //A70LV_Larry_0074

            memset((UINT8*)&m_stWarpConfig.stOsdBasic.stWArpGeoPara, 0, sizeof(m_stWarpConfig.stOsdBasic.stWArpGeoPara));
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY = 0;

            m_stWarpConfig.stOsdBasic.stWArpGeoPara.TABLE_COLOR = ucTemp; //A70LV_Larry_0074

            m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_V = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V = 0;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter = TRUE;

#ifdef WUXGA240_32X32
            if((m_sHalWarpingInfo.uiWarp_HW > DEF_WARPING_SIZE_H) || (m_sHalWarpingInfo.uiWarp_VW > DEF_WARPING_SIZE_V) || (ePANEL_ID_WUXGA_240HZ == m_sHalWarpingInfo.ePanelTimingId))
#else
            if(m_sHalWarpingInfo.uiWarp_HW > DEF_WARPING_SIZE_H || m_sHalWarpingInfo.uiWarp_VW > DEF_WARPING_SIZE_V)
#endif
            {
                m_sHalWarpingInfo.uiWarp_HW16 = ((m_sHalWarpingInfo.uiWarp_HW % DEF_WP_SPACE_4K) == 0) ? m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_4K : m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_4K + 1;
                m_sHalWarpingInfo.uiWarp_VW16 = ((m_sHalWarpingInfo.uiWarp_VW % DEF_WP_SPACE_4K) == 0) ? m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_4K : m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_4K + 1;
            }
            else
            {
                m_sHalWarpingInfo.uiWarp_HW16 = ((m_sHalWarpingInfo.uiWarp_HW % DEF_WP_SPACE_2K) == 0) ? m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_2K : m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_2K + 1;
                m_sHalWarpingInfo.uiWarp_VW16 = ((m_sHalWarpingInfo.uiWarp_VW % DEF_WP_SPACE_2K) == 0) ? m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_2K : m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_2K + 1;
            }

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
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H = (float)(ucKeystone_H * 2);
        }
        break;

        case eWARP_EVENT_KEYSTONE_V:
        {
            INT8 ucKeystone_V = *(INT8 *)pcData;
            ucKeystone_V = ucKeystone_V - KEYSTONE_BASED;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V = (float)ucKeystone_V;

        }
        break;

        case eWARP_EVENT_KEYSTONE_HV:
        {
            INT8 ucKeystone_H = *(INT8 *)pcData;
            INT8 ucKeystone_V = *((INT8 *)pcData + 1);

            ucKeystone_H = ucKeystone_H - KEYSTONE_BASED;
            ucKeystone_V = ucKeystone_V - KEYSTONE_BASED;

            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H = (float)(ucKeystone_H * 2);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V = (float)ucKeystone_V;
        }
        break;

            //A70_Eva_0007 End
        case eWARP_EVENT_PINBARREL_H:
        {
            INT8 ucPin_H = *(INT8 *)pcData;
            ucPin_H = ucPin_H - PINBARREL_BASED;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H = (float)(ucPin_H * 2 * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H));
        }
        break;

        case eWARP_EVENT_PINBARREL_V:
        {
            INT8 ucPin_V = *(INT8 *)pcData;
            ucPin_V = ucPin_V - PINBARREL_BASED;
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V = (float)(ucPin_V * 2 * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V));
        }
        break;

        case eWARP_EVENT_PINBARREL_HV:
        {
            INT8 ucPin_H = *(INT8 *)pcData;
            INT8 ucPin_V = *((INT8 *)pcData+1);

            ucPin_H = ucPin_H - PINBARREL_BASED;
            ucPin_V = ucPin_V - PINBARREL_BASED;

            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H = (float)(ucPin_H * 2 * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H));
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V = (float)(ucPin_V * 2 * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V));

        }
        break;

        case eWARP_EVENT_4C_TL_X:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
        break;

        case eWARP_EVENT_4C_TL_Y:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
        break;

        case eWARP_EVENT_4C_TR_X:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
        break;

        case eWARP_EVENT_4C_TR_Y:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
        break;

        case eWARP_EVENT_4C_BL_X:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
        break;

        case eWARP_EVENT_4C_BL_Y:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
        break;

        case eWARP_EVENT_4C_BR_X:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
        break;

        case eWARP_EVENT_4C_BR_Y:
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY = *(INT16 *)pcData;// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
        break;

        case eWARP_EVENT_4C_ALL:
        {
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX = *((INT16 *)pcData + 0);// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY = *((INT16 *)pcData + 1);// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX = *((INT16 *)pcData + 2);// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY = *((INT16 *)pcData + 3);// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX = *((INT16 *)pcData + 4);// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY = *((INT16 *)pcData + 5);// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX = *((INT16 *)pcData + 6);// * (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY = *((INT16 *)pcData + 7);// * (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
        }
        break;

        case eWARP_EVENT_CONFIG_PANEL:
#ifdef WUXGA240_32X32
            if((m_sHalWarpingInfo.uiWarp_HW > DEF_WARPING_SIZE_H) || (m_sHalWarpingInfo.uiWarp_VW > DEF_WARPING_SIZE_V) || (ePANEL_ID_WUXGA_240HZ == m_sHalWarpingInfo.ePanelTimingId))
#else
            if(m_sHalWarpingInfo.uiWarp_HW > DEF_WARPING_SIZE_H || m_sHalWarpingInfo.uiWarp_VW > DEF_WARPING_SIZE_V)
#endif /* WUXGA240_32X32 */
            {
                m_sHalWarpingInfo.uiWarp_HW16 = ((m_sHalWarpingInfo.uiWarp_HW % DEF_WP_SPACE_4K) == 0) ? m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_4K : m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_4K + 1;
                m_sHalWarpingInfo.uiWarp_VW16 = ((m_sHalWarpingInfo.uiWarp_VW % DEF_WP_SPACE_4K) == 0) ? m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_4K : m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_4K + 1;
            }
            else
            {
                m_sHalWarpingInfo.uiWarp_HW16 = ((m_sHalWarpingInfo.uiWarp_HW % DEF_WP_SPACE_2K) == 0) ? m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_2K : m_sHalWarpingInfo.uiWarp_HW / DEF_WP_SPACE_2K + 1;
                m_sHalWarpingInfo.uiWarp_VW16 = ((m_sHalWarpingInfo.uiWarp_VW % DEF_WP_SPACE_2K) == 0) ? m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_2K : m_sHalWarpingInfo.uiWarp_VW / DEF_WP_SPACE_2K + 1;
            }
            m_sHalWarpingInfo.fWarp_PHWHF = (m_sHalWarpingInfo.uiWarp_HW / 2.0);
            m_sHalWarpingInfo.fWarp_PVWHF = (m_sHalWarpingInfo.uiWarp_VW / 2.0);
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
            *(UINT8 *)pcData = (UINT8)m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H;
        break;

        case eWARP_EVENT_DEFAULT_FILTER_V:
            *(UINT8 *)pcData = (UINT8)m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_V;
        break;

        case eWARP_EVENT_WARP_PARA:
        {
            sWARP_BASIC *sWarpBasic = (sWARP_BASIC*)pcData;

            sWarpBasic->PM_WPKEYANG_H   = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_H;
            sWarpBasic->PM_WPKEYANG_V   = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEYANG_V;
            sWarpBasic->WPKEYANG_H_OLD  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_H_OLD;
            sWarpBasic->WPKEYANG_V_OLD  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.WPKEYANG_V_OLD;
            sWarpBasic->PM_WPPINWID_H   = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_H / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            sWarpBasic->PM_WPPINWID_V   = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPPINWID_V / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            sWarpBasic->PM_WPKEY4C_TLX  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            sWarpBasic->PM_WPKEY4C_TLY  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TLY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            sWarpBasic->PM_WPKEY4C_TRX  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            sWarpBasic->PM_WPKEY4C_TRY  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_TRY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            sWarpBasic->PM_WPKEY4C_BLX  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            sWarpBasic->PM_WPKEY4C_BLY  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BLY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            sWarpBasic->PM_WPKEY4C_BRX  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRX;//  / (m_sHalWarpingInfo.uiWarp_HW/DEF_WARPING_SIZE_H);
            sWarpBasic->PM_WPKEY4C_BRY  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.PM_WPKEY4C_BRY;//  / (m_sHalWarpingInfo.uiWarp_VW/DEF_WARPING_SIZE_V);
            sWarpBasic->TABLE_COLOR     = m_stWarpConfig.stOsdBasic.stWArpGeoPara.TABLE_COLOR;

            sWarpBasic->POLATION_H      = m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H;
            sWarpBasic->POLATION_V      = m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_V;
            sWarpBasic->m_cWarpFilterSelect_H   = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H;
            sWarpBasic->m_cWarpFilterSelect_V   = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_V;
            sWarpBasic->m_cWarpFilterApValue_H  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_H;
            sWarpBasic->m_cWarpFilterApValue_V  = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterApValue_V;
            sWarpBasic->m_cWarpAutoFilter       = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter;
            sWarpBasic->m_cWarpAutoFilterAp     = m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilterAp;
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
eHAL_WARPING_EXEC_CODE halWarp_InterpolationSet(void)
{
    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpAutoFilter == TRUE)
        {
            dvProAV_WarpAutoWarpAndFrtSet((uint8*)&m_sHalWarpingInfo.acWarpTable[0], m_sHalWarpingInfo.uiWarp_HW16, m_sHalWarpingInfo.uiWarp_VW16, m_sHalWarpingInfo.ucIs4KWarp, &m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H);
            m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H = m_stWarpConfig.stOsdBasic.stWArpGeoPara.POLATION_H;
        }
        else
        {
            dvProAV_WarpCoeffSet(m_stWarpConfig.stOsdBasic.stWArpGeoPara.m_cWarpFilterSelect_H);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }

#ifdef CUSTOM_CHRISTIE
    halBasicWarpSettingSet();
#endif

    return eHAL_WARPING_EXEC_CODE_PASS;
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
            m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St = 0;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width = BLENDING_TOP_PIXEL_WIDTH_DEF_VALUE;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width = BLENDING_BOTTOM_PIXEL_WIDTH_DEF_VALUE;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width = BLENDING_LEFT_PIXEL_WIDTH_DEF_VALUE;
            m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width = BLENDING_RIGHT_PIXEL_WIDTH_DEF_VALUE;
            m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma = (UINT8)eCM_BLENDING_GAMMA_2_2;//eBLENDING_GAMMA_2_2; //A70LV_Larry_0171
            break;

        case eBLENDING_EVENT_T_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_B_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_L_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_R_ENABLE:
            {
                UINT8 ucEnable = *(UINT8 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable = ucEnable;
            }
            break;

        case eBLENDING_EVENT_T_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St = uiValue;
            }
            break;

        case eBLENDING_EVENT_B_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St = uiValue;

                //LOG_MSG(db_ALWAYS, "m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St %d\r\n", m_sBlendingPara.uiBlending_B_St);
            }
            break;

        case eBLENDING_EVENT_L_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St = uiValue;
            }
            break;

        case eBLENDING_EVENT_R_ST:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St = uiValue;
            }
            break;

        case eBLENDING_EVENT_T_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_B_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_L_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_R_WIDTH:
            {
                UINT16 uiValue = *(UINT16 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width = uiValue;
            }
            break;

        case eBLENDING_EVENT_GAMMA: //A70LV_Larry_0171
            {
                UINT8 ucGamma = *(UINT8 *)pcData;

                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma = ucGamma;
            }
            break;

        case eBLENDING_EVENT_GAMMA_DMA: //A70LV_Larry_0171
            m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma = BLENDING_GAMMA_DMA;
            break;

        case eBLENDING_EVENT_ALL: //A70LV_Larry_0139
            {
                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable =   *((UINT8 *)pcData + 0);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St =       *((UINT8 *)pcData + 2) << 8 | *((UINT8 *)pcData + 1);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width =    *((UINT8 *)pcData + 4) << 8 | *((UINT8 *)pcData + 3);
                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable =   *((UINT8 *)pcData + 5);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St =       *((UINT8 *)pcData + 7) << 8 | *((UINT8 *)pcData + 6);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width =    *((UINT8 *)pcData + 9) << 8 | *((UINT8 *)pcData + 8);
                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable =   *((UINT8 *)pcData + 10);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St =       *((UINT8 *)pcData + 12) << 8 | *((UINT8 *)pcData + 11);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width =    *((UINT8 *)pcData + 14) << 8 | *((UINT8 *)pcData + 13);
                m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable =   *((UINT8 *)pcData + 15);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St =       *((UINT8 *)pcData + 17) << 8 | *((UINT8 *)pcData + 16);
                m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width =    *((UINT8 *)pcData + 19) << 8 | *((UINT8 *)pcData + 18);
            }
            break;

        default:
            ASSERT_ALWAYS();
            break;
   }
#endif /* 0 */
}

// ==============================================================================
// FUNCTION NAME: halWarp_BlendingParameterGet
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
// 2022/10/17, Larry Create
// --------------------
// ==============================================================================
void halWarp_BlendingParameterGet(eBLENDING_EVENT eType, void *pcData)
{
#if 1
    switch(eType)
    {
        case eBLENDING_EVENT_ALL: //A70LV_Larry_0139
            {

                sBLENDING_BASIC *sBlendungBasic = (sBLENDING_BASIC*)pcData;

                sBlendungBasic->ucBlending_T_Enable = m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable;
                sBlendungBasic->uiBlending_T_St     = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St;
                sBlendungBasic->uiBlending_T_Width  = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width;
                sBlendungBasic->ucBlending_B_Enable = m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable;
                sBlendungBasic->uiBlending_B_St     = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St;
                sBlendungBasic->uiBlending_B_Width  = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width;
                sBlendungBasic->ucBlending_L_Enable = m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable;
                sBlendungBasic->uiBlending_L_St     = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St;
                sBlendungBasic->uiBlending_L_Width  = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width;
                sBlendungBasic->ucBlending_R_Enable = m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable;
                sBlendungBasic->uiBlending_R_St     = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St;
                sBlendungBasic->uiBlending_R_Width  = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width;
                sBlendungBasic->ucBlendingGamma  = m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma;
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
eRESULT halWarp_BlendingSet(void)
{
    UINT16 uiEdgeBlending = 0;
    UINT16 uiCoef = 0;
    UINT16 uiWidth = 0, uiWidthCal = 0;
    double power[7] = {1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4};

    //GV_EGBCT = ( PM_EGB_GAMMA_EN == 0 ) ? 0x3000 : 0x3010;

    if(halWarping_TwistLinkFlag_Get() == TRUE) //A70LV_Larry_0319
    {
        return rcSUCCESS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvProAV_BldUpdatingEnable(false);

        //==========Left==========
        if(m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable)
        {
            uiWidth = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_Width;
            uiWidthCal = uiWidth;
            if(((uiWidthCal % 2) == 0) && (uiWidthCal > 0))
            {
                uiWidthCal--;
            }

            uiCoef = ((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable == 0) || (uiWidth < 5) ) ? 0 : (UINT16)(((131072 - 1) / (uiWidthCal - 1)) + 1); //131072 = 2^17
        }

        dvProAV_AccessWrite(eEgblHLSt, (uint32)m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_L_St);

        if((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable == 0) || (uiCoef == 0))
        {
            dvProAV_AccessWrite(eEgblHinit, 0x80);
            dvProAV_AccessWrite(eEgblHLW, 0x0000);
            dvProAV_AccessWrite(eEgblHLGrad, 0x0000);
        }
        else
        {
            uiEdgeBlending = uiEdgeBlending | 0x08;
            dvProAV_AccessWrite(eEgblHinit, 0x00);
            dvProAV_AccessWrite(eEgblHLW, uiWidth);
            dvProAV_AccessWrite(eEgblHLGrad, (uiCoef & 0xFFFF));
        }
        //========================

        //=========Right==========
        if(m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable)
        {
            uiWidth = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_Width;
            uiWidthCal = uiWidth;
            if(((uiWidthCal % 2) == 0) && (uiWidthCal > 0))
            {
                uiWidthCal--;
            }

            uiCoef = ((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable == 0) || (uiWidth < 5) ) ? 0 : (UINT16)((~(UINT16)((131072 - 1)/(uiWidthCal - 1) + 1)) + 1);

        }

        dvProAV_AccessWrite(eEgblHRSt, (uint32)(m_sHalWarpingInfo.uiBlending_HW - m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_R_St -uiWidth));

        if(m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable == 0 )
        {
            dvProAV_AccessWrite(eEgblHRW, 0x0000);
            dvProAV_AccessWrite(eEgblHRGrad, 0x0000);
        }
        else
        {
            uiEdgeBlending = uiEdgeBlending | 0x04;
            dvProAV_AccessWrite(eEgblHRW, uiWidth);
            dvProAV_AccessWrite(eEgblHRGrad, (uiCoef & 0xFFFF));
        }
        //========================

        //==========TOP===========
        if(m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable)
        {
            uiWidth = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_Width;
            uiCoef = ((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable == 0) || (uiWidth < 5) ) ? 0 : (UINT16)(((131072 - 1) / (uiWidth - 1)) + 1);
        }

        dvProAV_AccessWrite(eEgblVTSt, m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_T_St);
        if((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable == 0) || (uiCoef == 0))
        {
            dvProAV_AccessWrite(eEgblVinit, 0x80);
            dvProAV_AccessWrite(eEgblVTW,   0x0000);
            dvProAV_AccessWrite(eEgblVTGrad, 0x0000);
        }
        else
        {
            uiEdgeBlending = uiEdgeBlending | 0x02;
            dvProAV_AccessWrite(eEgblVinit, 0x00);
            dvProAV_AccessWrite(eEgblVTW, uiWidth);
            dvProAV_AccessWrite(eEgblVTGrad, (uiCoef & 0xFFFF));
        }
        //========================

        //=========Bottom=========
        if(m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable)
        {
            uiWidth = m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_Width;
            uiCoef = ((m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable == 0) || (uiWidth < 5) ) ? 0 : (UINT16)((~(UINT16)((131072 - 1)/(uiWidth - 1) + 1)) + 1);
        }
        dvProAV_AccessWrite(eEgblVBSt, (m_sHalWarpingInfo.uiBlending_VW - m_stWarpConfig.stOsdBasic.stBlendingPara.uiBlending_B_St - uiWidth));
        if(m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable == 0)
        {
            dvProAV_AccessWrite(eEgblVBW, 0x0000);
            dvProAV_AccessWrite(eEgblVBGrad, 0x0000);
        }
        else
        {
            uiEdgeBlending = uiEdgeBlending | 0x01;
            dvProAV_AccessWrite(eEgblVBW, uiWidth);
            dvProAV_AccessWrite(eEgblVBGrad, (uiCoef & 0xFFFF));
        }
        //========================

        dvProAV_BldEnableSet(uiEdgeBlending);


        if(uiEdgeBlending == 0)
        {
            dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, FALSE);
            dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, FALSE);
            //dvProAV_AccessWrite(eVopGammaEn, (uint32)false);
            dvProAV_BldDBDEnableSet(false);
            dvProAV_BldDBDBiasEnableSet(false);
            dvProAV_BldUpdatingEnable(true);
        }
        else
        {
            if((FALSE == dvProAV_BldGammaEnableGet(BLENDING_GAMMA_TYPE_FRONT)) ||
               (FALSE == dvProAV_BldGammaEnableGet(BLENDING_GAMMA_TYPE_FRONT)))
            {
                dvProAV_BldDBDEnableSet(false);
                dvProAV_BldDBDBiasEnableSet(false);

                dvProAV_BldGammaPowerSet(BLENDING_GAMMA_TYPE_FRONT, power[m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma]);
                dvProAV_BldGammaPowerSet(BLENDING_GAMMA_TYPE_BACK, 1/power[m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma]);

                LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) Gamma %d\r\n", __FUNCTION__, __LINE__, m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);

                if(!dvProAV_BldGammaEnableGet(BLENDING_GAMMA_TYPE_FRONT)) //gamma enable
                {

                    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
                    dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, TRUE);
                }

                LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

                if(!dvProAV_BldGammaEnableGet(BLENDING_GAMMA_TYPE_BACK)) //gamma enable
                {
                    dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, TRUE);
                }
           }

           dvProAV_BldUpdatingEnable(true);
        }

#ifdef CUSTOM_CHRISTIE
    	halBasicBlendSettingSet();
#else
    	halBasicWarpSettingSet();
    	halBasicBlendSettingSet();
#endif

        halWarping_SemaphoreGive(__FUNCTION__);
    }



    return rcSUCCESS;
}

eRESULT halWarp_BlendingGamma(void) //A70LV_Larry_0171
{
    double power[7] = {1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4};

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) Gamma %d\r\n", __FUNCTION__, __LINE__ , m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma);

        dvProAV_BldUpdatingEnable(false);

        dvProAV_BldGammaPowerSet(BLENDING_GAMMA_TYPE_FRONT, power[m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma]);
        //dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, TRUE);

        dvProAV_BldGammaPowerSet(BLENDING_GAMMA_TYPE_BACK, 1/power[m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma]);
        //dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, TRUE);

        dvProAV_BldUpdatingEnable(true);

#ifdef CUSTOM_CHRISTIE
    	halBasicBlendSettingSet();
#endif

        halWarping_SemaphoreGive(__FUNCTION__);
    }

    return rcSUCCESS;
}


#endif /* 0 */ //A70LV_Larry_0051

eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_EN(UINT8 ucEn, UINT8 ucAcen)
{
#if (ENABLE_COLOR_UNIFORMITY == FALSE)   //G100_Simon_0090
    return eHAL_WARPING_EXEC_CODE_PASS;
#endif

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        dvProAV_UnifEnableSet(TRUE);
        halWarping_SemaphoreGive(__FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s mutex fail\n\n" , __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
}

eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_EN_NoSemaphore(UINT8 ucEn, UINT8 ucAcen)
{
#if (ENABLE_COLOR_UNIFORMITY == FALSE)   //G100_Simon_0090
    return eHAL_WARPING_EXEC_CODE_PASS;
#endif

    //int GV_RTCT = dvC789_Read(BN_RTCT);
    LOG_MSG(db_HAL_WARPING, "Uniformity_EN_NoSemaphore == (%d,%d)\n", ucEn,ucAcen);	//A65_OPTOMA_Doulas_0136


    return eHAL_WARPING_EXEC_CODE_PASS;
}

//G100_Tim_0023, add, start
// ==============================================================================
// Function: halACU_OSD_Color_Initial()
// DESCRIPTION: Take samaphore and initial the OSD color
// DESCRIPTION: to prevent the wrong OSD color after Auto Focus
// Params: None
// Returns: eHAL_WARPING_EXEC_CODE_PASS : success
// Returns: eHAL_WARPING_EXEC_CODE_MUTEX_FAIL : fail, no samaphore
//
// Modification History
// --------------------
// 2021/07/07, Tim Create
// --------------------
// ==============================================================================
/*  //A65_OPTOMA_CL_0006
eHAL_WARPING_EXEC_CODE halACU_OSD_Color_Initial( void )
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    UINT8 ucRetry = 0;

	#ifdef ENABLE_ACU_TWIST_LINK_CHECK
	if( halWarping_TwistLinkFlag_Get() == TRUE )
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
	#endif

    for( ; ucRetry < 3; ucRetry++ )
    {
        if( TRUE == halWarping_SemaphoreTake(__FUNCTION__) )
    	{
            #if 0  //A65_OPTOMA_CL_0006
			LOG_MSG(db_HAL_WARPING, "(%s) Start %d %d\n", __FUNCTION__ , m_sHalWarpingInfo.uiWarp_HW , m_sHalWarpingInfo.uiWarp_VW);
			dvC789_Write( B0_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW/128)&0xff ); //avoid pattern incorrect. //A65_OPTOMA_Julie_0050

            OSD_Color_Init();
            #else
            utilWarp_CheckWarpOsdColorPalette(COLOR_PALETTE__OSD);
            #endif
            halWarping_SemaphoreGive(__FUNCTION__);
            eRet = eHAL_WARPING_EXEC_CODE_PASS;
            break;
        }
        MS_SLEEP(2);
    }

    return (eRet);
}
*/

UINT8 halWarping_Color_Uniformity_Init_Status_Get( void )
{
    return m_ucACU_Init_Status;
}

void halWarping_Color_Uniformity_Init_Status_Set( UINT8 ucACU_Init )
{
    m_ucACU_Init_Status = ucACU_Init;
}
//G100_Tim_0020, add, end

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
    {
#if (ENABLE_COLOR_UNIFORMITY == FALSE)   //G100_Simon_0090
        return eHAL_WARPING_EXEC_CODE_PASS;
#endif

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            dvProAV_UnifEnableSet(FALSE);
            halWarping_SemaphoreGive(__FUNCTION__);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "%s mutex fail\n\n" , __FUNCTION__);
            return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        }
    }

    MS_SLEEP(20);
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
    halWarping_Color_Uniformity_EN(1, 0);
    MS_SLEEP(20);
    return eHAL_WARPING_EXEC_CODE_PASS;
}

//A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
UINT8 halWarping_Color_Uniformity_Enable_Get(void)
{
#if (ENABLE_COLOR_UNIFORMITY == FALSE)   //G100_Simon_0090
    return eHAL_WARPING_EXEC_CODE_PASS;
#endif
    BOOL cEnable = FALSE;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        cEnable = dvProAV_UnifEnableGet();
        halWarping_SemaphoreGive(__FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s mutex fail\n\n" , __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return cEnable;
}

//A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
void halWarping_Color_Uniformity_Enable_Set(UINT8 ucData)
{
    if(ucData == 0)
    {
        halWarping_Color_Uniformity_Disable();
    }
    else if(ucData == 1)
    {
        halWarping_Color_Uniformity_Enable();
    }
}


//A65_OPTOMA_CL_0002  //A65_OPTOMA_CL_0004
eHAL_WARPING_EXEC_CODE halWarping_Set_OSD_Transparent( UINT8 ucPaletteIndex )
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    UINT8 ucRetry = 0;

	#ifdef ENABLE_ACU_TWIST_LINK_CHECK
	if( halWarping_TwistLinkFlag_Get() == TRUE )
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
	#endif

    for( ; ucRetry < 3; ucRetry++ )
    {
        if( TRUE == halWarping_SemaphoreTake(__FUNCTION__) )
    	{
            halWarping_SemaphoreGive(__FUNCTION__);
            eRet = eHAL_WARPING_EXEC_CODE_PASS;
            break;
        }
        MS_SLEEP(2);
    }

    return (eRet);
}


#if (ENABLE_COLOR_UNIFORMITY == TRUE)  //G100_Tim_0012, add, start
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

//halWarping_ColorUniformitySet
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Set(UINT8 *pcData, UINT16 uiSize)  //G100_Owen_0019  //H2PF_Simon_0035
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d)\r\n", __FUNCTION__, __LINE__ , uiSize);

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvProAV_UnifDataShiftSet(1);
        dvProAV_UnifEnableSet(FALSE);
        dvProAV_UnifDataSet(0, 10, 8, pcData, uiSize);  //A70LK_CL_0001 (11,9) ==> (10,8)
        dvProAV_UnifEnableSet(TRUE);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}


// ==============================================================================
// FUNCTION NAME:
// DESCRIPTION: Set the data to the iChip C789 from the file
// Params: *pcFileName : the file name
// Returns: eHAL_WARPING_EXEC_CODE_PASS : OK
// Returns: eHAL_WARPING_EXEC_CODE_MUTEX_FAIL : NG
//
// Modification History
// --------------------
// 2020/08/11, Owen Create
// 2021/06/21, Tim Modify
// --------------------
// ==============================================================================

// ==> palGeo_Color_Uniformity_File_Apply
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Table_Set(char *pcFileName)
{
    FILE *pFile = NULL;
    pFile = fopen(pcFileName, "rb");
    UINT8 ucCnt = 0;
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_FAIL;

	if(pFile == NULL)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    UINT8 *pucData = (UINT8 *)malloc(WARPING_COLOR_UNIFORMITY_SIZE);
    if(pucData == NULL)
    {
		fclose(pFile);
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }
    memset(pucData, 0xFF, WARPING_COLOR_UNIFORMITY_SIZE);
    fread(pucData, 1, WARPING_COLOR_UNIFORMITY_SIZE, pFile);

    for( ; ucCnt < 3; ucCnt++ )
    {
        eRet = halWarping_Color_Uniformity_Set( pucData, WARPING_COLOR_UNIFORMITY_SIZE );  //A70LK_CL_0001  //H2PF_Simon_0035
        if( eRet == eHAL_WARPING_EXEC_CODE_PASS )
        {
            break;
        }
    }

    free(pucData);
	fclose(pFile);

    return eRet;
}

#else //ENABLE_COLOR_UNIFORMITY         //G100_Tim_0012, add, end

// ==============================================================================
// FUNCTION NAME: halWarping_Color_Uniformity_Set
// DESCRIPTION:
//
//
// Params:
// UINT8 *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/07/25, Larry Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Set(UINT8 *pcData, UINT16 uiSize)  //G100_Owen_0019  //H2PF_Simon_0035
{
#if (ENABLE_COLOR_UNIFORMITY == FALSE)   //G100_Simon_0090
    return eHAL_WARPING_EXEC_CODE_PASS;
#endif

    return eHAL_WARPING_EXEC_CODE_PASS;
}

// ==============================================================================
// FUNCTION NAME: halWarping_Color_Uniformity_Table_Set
// DESCRIPTION:
//
//
// Params:
// char *pcFileName;
//
// Returns:
// eHAL_WARPING_EXEC_CODE
//
// Modification History
// --------------------
// 2020/08/11, Owen Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarping_Color_Uniformity_Table_Set(char *pcFileName)  //H2PF_Simon_0035
{
#if (ENABLE_COLOR_UNIFORMITY == FALSE)   //G100_Simon_0090
    return eHAL_WARPING_EXEC_CODE_PASS;
#endif

    return eHAL_WARPING_EXEC_CODE_PASS;
}

#endif //ENABLE_COLOR_UNIFORMITY        //G100_Tim_0012, add

eHAL_WARPING_EXEC_CODE halWarping_PowerNormal(const ePANEL_ID ePanelId) //A70LV_Larry_0051
{
    BOOL bIs4K = FALSE;
    //eHAL_WARPING_EXEC_CODE eExeRet;
    UINT16 uiDelay = 0;

#ifdef WARP_ENABLE
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d)\r\n", __FUNCTION__, __LINE__ , ePanelId);

    m_sHalWarpingInfo.ucWarpingCheck = TRUE;
    m_sHalWarpingInfo.ucCurrent_DBD_Data_Index        = DBD_BLENDING_NO_DATA;
    m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index = BLACKLEVEL_NO_DATA;

    //bIs4K = halWarping_Is4KPanel(ePanelId);
	m_sHalWarpingInfo.ePanelTimingId = ePanelId;

    //if(xSemaphoreTake(m_sHalWarpingInfo.xSemaphore,(TickType_t )SEMPHORE_WAIT_DELAY/portTICK_RATE_MS) == pdTRUE)
    //{
        //if(!m_sHalWarpingInfo.bWarpingInit)
        //{//
            //LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) : Please call halWarping_Init() before this call\r\n", __FUNCTION__, __LINE__);
            //xSemaphoreGive(m_sHalWarpingInfo.xSemaphore);
            //return eHAL_WARPING_EXEC_CODE_NOT_INIT;
        //}

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
                    m_sHalWarpingInfo.uiWarp_VW = 768;
                    break;

                case ePANEL_ID_1600x1200_60HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 1600;
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

                case ePANEL_ID_WUXGA_60HZ:
                case ePANEL_ID_WUXGA_120HZ:
                case ePANEL_ID_WUXGA_240HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 1920;
                    m_sHalWarpingInfo.uiWarp_VW = 1200;
                    break;

                case ePANEL_ID_3840x2160_50HZ:
                case ePANEL_ID_3840x2160_60HZ:
                case ePANEL_ID_3840x2160_30HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 3840;
                    m_sHalWarpingInfo.uiWarp_VW = 2160;
                    break;

                case ePANEL_ID_3840x2400_50HZ:
                case ePANEL_ID_3840x2400_60HZ:
                case ePANEL_ID_3840x2400_30HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 3840;
                    m_sHalWarpingInfo.uiWarp_VW = 2400;
                    break;

                case ePANEL_ID_2560x1440_60HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 2560;
                    m_sHalWarpingInfo.uiWarp_VW = 1440;
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

        m_sHalWarpingInfo.uiBlending_HW = m_sHalWarpingInfo.uiWarp_HW;
        m_sHalWarpingInfo.uiBlending_VW = m_sHalWarpingInfo.uiWarp_VW;

#ifdef WUXGA240_32X32
        if((m_sHalWarpingInfo.uiWarp_HW > 1920) || (m_sHalWarpingInfo.uiWarp_VW > 1200) || (ePANEL_ID_WUXGA_240HZ == m_sHalWarpingInfo.ePanelTimingId))
#else
        if((m_sHalWarpingInfo.uiWarp_HW > 1920) || (m_sHalWarpingInfo.uiWarp_VW > 1200))
#endif /* WUXGA240_32X32 */
        {
            m_sHalWarpingInfo.ucIs4KWarp = 1;
        }
        else
        {
            m_sHalWarpingInfo.ucIs4KWarp = 0;
        }

#if 0
        if(m_sHalWarpingInfo.uiWarp_HW > 1920)
        {
            m_sHalWarpingInfo.uiWarp_FourCornerHW = 3840;
        }
        else
        {
            m_sHalWarpingInfo.uiWarp_FourCornerHW = 1920;
        }

        if(m_sHalWarpingInfo.uiWarp_VW > 1080)
        {
            m_sHalWarpingInfo.uiWarp_FourCornerVW = 2160;
        }
        else
        {
            m_sHalWarpingInfo.uiWarp_FourCornerVW = 1080;
        }
#endif /* 0 */

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            dvProAV_Warp_Init(m_sHalWarpingInfo.ePanelTimingId);

            // Warp delay setting
            /*uiDelay = (((m_sHalWarpingInfo.uiWarp_VW / 2) + 127) / 128)*128;   // ProAV_Rex_0046
            dvProAV_AccessWrite(eWcuWarpLnDly, uiDelay);  // Line delay from WIP to Warping
            dvProAV_AccessWrite(eWcuWopLnDly, uiDelay + 128);  // Line delay from WIP to WOP*/

            dvProAV_AccessWrite(eWarpWftEn, 0x01);

            //halWarping_CursorPanelConfig(); //A70LV_Larry_0137
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);

#ifdef FPGA_ENTRY_4K
            //Warp OSD
            //dvProAV_WarpOSD_Init_E4K(m_sHalWarpingInfo.iPalette_ColorAmount, m_sHalWarpingInfo.aucPalette, m_sHalWarpingInfo.uiInhibit_Color);
            //dvProAV_WarpOSD_On_E4K(eWPOSDPAGE_0);
            //dvProAV_WarpOSD_On_E4K(eWPOSDPAGE_1);
#else
            //Warp OSD
            dvProAV_WarpOSD_Init(m_sHalWarpingInfo.iPalette_ColorAmount, m_sHalWarpingInfo.aucPalette, m_sHalWarpingInfo.uiInhibit_Color);
            dvProAV_WarpOSD_On(eWPOSDPAGE_0);
            dvProAV_WarpOSD_On(eWPOSDPAGE_1);
#endif /* FPGA_ENTRY_4K */

            halWarping_SemaphoreGive(__FUNCTION__);
        }

        //halWarping_DBD_Blending_Init();
        //halWarping_BlackLevel_Init();

        halWarp_GeometrySet();
        //halWarping_Color_Uniformity_Init();  //A70LK_CL_0001

        m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable = 0;
        m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable = 0;
        m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable = 0;
        m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable = 0;
        m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma = eCM_BLENDING_GAMMA_2_2;

        halWarp_BlendingSet();

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, FALSE);
            dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, FALSE);

            halWarping_SemaphoreGive(__FUNCTION__);
        }

        //write panel H/V to Database
        utilDatabase_WriteInformationData(eDI_PANEL_H_RESOLUTION, DATA_TYPE_UI_DIGIT_16, 0, &m_sHalWarpingInfo.uiWarp_HW);
        utilDatabase_WriteInformationData(eDI_PANEL_V_RESOLUTION, DATA_TYPE_UI_DIGIT_16, 0, &m_sHalWarpingInfo.uiWarp_VW);
        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) %d*%d \r\n", __FUNCTION__, __LINE__, m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW);

#endif /* 0 */
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_ConfigPanel(const ePANEL_ID ePanelId, const UINT8 ucDualPipe3D)
{
    BOOL bIs4K = FALSE;
    //eHAL_WARPING_EXEC_CODE eExeRet;
    UINT16 uiDelay = 0;
    BOOL bDBDEnable = FALSE;

#ifdef WARP_ENABLE
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d)\r\n", __FUNCTION__, __LINE__ , ePanelId);

    //bIs4K = halWarping_Is4KPanel(ePanelId);

	m_sHalWarpingInfo.ePanelTimingId = ePanelId;
    m_sHalWarpingInfo.ucWarpingCheck = TRUE; //need to re-apply warping

    //if(xSemaphoreTake(m_sHalWarpingInfo.xSemaphore,(TickType_t )SEMPHORE_WAIT_DELAY/portTICK_RATE_MS) == pdTRUE)
    //{
        //if(!m_sHalWarpingInfo.bWarpingInit)
        //{//
            //LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) : Please call halWarping_Init() before this call\r\n", __FUNCTION__, __LINE__);
            //xSemaphoreGive(m_sHalWarpingInfo.xSemaphore);
            //return eHAL_WARPING_EXEC_CODE_NOT_INIT;
        //}

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
                    m_sHalWarpingInfo.uiWarp_VW = 768;
                    break;

                case ePANEL_ID_1600x1200_60HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 1600;
                    m_sHalWarpingInfo.uiWarp_VW = 1200;
                    break;

                case ePANEL_ID_WUXGA_60HZ:
                case ePANEL_ID_WUXGA_120HZ:
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
                case ePANEL_ID_3840x2160_30HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 3840;
                    m_sHalWarpingInfo.uiWarp_VW = 2160;
                    break;

                case ePANEL_ID_3840x2400_50HZ:
                case ePANEL_ID_3840x2400_60HZ:
                case ePANEL_ID_3840x2400_30HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 3840;
                    m_sHalWarpingInfo.uiWarp_VW = 2400;
                    break;

                case ePANEL_ID_2560x1440_60HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 2560;
                    m_sHalWarpingInfo.uiWarp_VW = 1440;
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

                case ePANEL_ID_1920x2400_60HZ:
                    m_sHalWarpingInfo.uiWarp_HW = 1920;
                    m_sHalWarpingInfo.uiWarp_VW = 2400;
                    break;
        }

        m_sHalWarpingInfo.uiBlending_HW = m_sHalWarpingInfo.uiWarp_HW;
        m_sHalWarpingInfo.uiBlending_VW = m_sHalWarpingInfo.uiWarp_VW;

        if(ucDualPipe3D)
        {
            if(m_sHalWarpingInfo.ePanelTimingId == ePANEL_ID_1920x2400_60HZ)
            {
                m_sHalWarpingInfo.uiBlending_HW = m_sHalWarpingInfo.uiBlending_HW*2;
                //m_sHalWarpingInfo.uiBlending_VW = m_sHalWarpingInfo.uiBlending_VW;
                //m_sHalWarpingInfo.uiWarp_VW = m_sHalWarpingInfo.uiWarp_VW * 2;
            }
        }

#ifdef WUXGA240_32X32
        if((m_sHalWarpingInfo.uiWarp_HW > 1920) || (m_sHalWarpingInfo.uiWarp_VW > 1200) || (ePANEL_ID_WUXGA_240HZ == m_sHalWarpingInfo.ePanelTimingId))
#else
        if((m_sHalWarpingInfo.uiWarp_HW > 1920) || (m_sHalWarpingInfo.uiWarp_VW > 1200))
#endif /* WUXGA240_32X32 */
        {
            m_sHalWarpingInfo.ucIs4KWarp = 1;
        }
        else
        {
            m_sHalWarpingInfo.ucIs4KWarp = 0;
        }

        //A70LK_Simon_0010
        //notify current panel (4k or 1080p) for AP
        if(m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT)
        {
            UINT32 ulData = 0;
            utilDatabase_WriteInformationData(eDI_3D_ENABLE, DATA_TYPE_UI_DIGIT_32, 0, &ulData);
        }
        else
        {
            UINT32 ulData = 1;
            utilDatabase_WriteInformationData(eDI_3D_ENABLE, DATA_TYPE_UI_DIGIT_32, 0, &ulData);
        }

#if 0
        if(m_sHalWarpingInfo.uiWarp_HW > 1920)
        {
            m_sHalWarpingInfo.uiWarp_FourCornerHW = 3840;
        }
        else
        {
            m_sHalWarpingInfo.uiWarp_FourCornerHW = 1920;
        }

        if(m_sHalWarpingInfo.uiWarp_VW > 1080)
        {
            m_sHalWarpingInfo.uiWarp_FourCornerVW = 2160;
        }
        else
        {
            m_sHalWarpingInfo.uiWarp_FourCornerVW = 1080;
        }
#endif /* 0 */


        //write panel H/V to Database
        utilDatabase_WriteInformationData(eDI_PANEL_H_RESOLUTION, DATA_TYPE_UI_DIGIT_16, 0, &m_sHalWarpingInfo.uiWarp_HW);
        utilDatabase_WriteInformationData(eDI_PANEL_V_RESOLUTION, DATA_TYPE_UI_DIGIT_16, 0, &m_sHalWarpingInfo.uiWarp_VW);

        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) %d*%d \r\n", __FUNCTION__, __LINE__, m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW);

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            dvProAV_WarpImageSizeSet(ePanelId);   // set warping size // ProAV_Rex_0033

            halWarp_GeometryParameterSet(eWARP_EVENT_CONFIG_PANEL, NULL);
            halWarping_SemaphoreGive(__FUNCTION__);
        }

        halWarp_GeometrySet();

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            MS_SLEEP(50);
            dvProAV_WarpDownScalingIssueWorkAround();

            halWarping_SemaphoreGive(__FUNCTION__);
        }

        halWarp_BlendingSet();

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, FALSE);
            dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, FALSE);

            MS_SLEEP(100);
#ifdef FPGA_ENTRY_4K
            //A70LK_CL_0004 for 3D 1080p output panel to change Warp OSD resolution
            dvProAV_WarpOSD_Init_E4K(m_sHalWarpingInfo.iPalette_ColorAmount, (UINT8 *)m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette, m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0]);  //H2PF_Simon_0038
            dvProAV_WarpOSD_On_E4K(eWPOSDPAGE_0);
            dvProAV_WarpOSD_On_E4K(eWPOSDPAGE_1);
#else
            dvProAV_WarpOSD_Init(m_sHalWarpingInfo.iPalette_ColorAmount, m_sHalWarpingInfo.aucPalette, m_sHalWarpingInfo.uiInhibit_Color);
            dvProAV_WarpOSD_On(eWPOSDPAGE_0);
            dvProAV_WarpOSD_On(eWPOSDPAGE_1);
#endif /* FPGA_ENTRY_4K */

            bDBDEnable = dvProAV_BldDBDEnableGet();

            halWarping_SemaphoreGive(__FUNCTION__);
        }

        if((bDBDEnable) ||
           (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_L_Enable) ||
           (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_R_Enable) ||
           (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_T_Enable) ||
           (m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlending_B_Enable))
        {
            halWarp_BlendingGamma();

            if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
            {
                dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, TRUE);
                dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, TRUE);
                halWarping_SemaphoreGive(__FUNCTION__);
            }
            //LOG_MSG(db_ALWAYS, "%s \n", __FUNCTION__);
        }

        //MS_SLEEP(80);

#endif /* 0 */
    return eHAL_WARPING_EXEC_CODE_PASS;
}

ePANEL_ID halWarping_PanelGet(void)
{
    return m_sHalWarpingInfo.ePanelTimingId;
}

eHAL_WARPING_EXEC_CODE halWarping_Init(void)//(const eHAL_WARPING_WARP_MODE eMode, const eHAL_WARPING_MOVE_PITCH eMoveIdx) //A70LV_Larry_0051
{
    //UINT8 ucIndex, ucIndex2;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

#ifdef FPGA_ENTRY_4K
    //utilQueueInitial(&m_asWarpingOSD_Ring_Buffer, 64, sizeof(sPAYLOAD), (UINT8*)&m_sWarping_Buffer);  //A70LK_Simon_0001  //H2PF_Simon_0038 move to palGeo.c
#endif /* FPGA_ENTRY_4K */

    m_sHalWarpingInfo.bWarpingInit = TRUE;

#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
    if(pthread_mutex_init(&m_sHalWarpingInfo.xSemaphore, NULL) == -1)
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

    //halWarping_OsdQueueDrawSemaphoreCreate();  //A70LK_Simon_0008  //H2PF_Simon_0038 move to palGeo.c

    //TODO
    //To get eeprom data from upper layer and set to m_sHalWarpingInfo parameter
    m_sHalWarpingInfo.eWarpMode = HAL_WARPING_WARP_MODE_2x2;
    m_sHalWarpingInfo.bGridShow = FALSE;//In the init status you never show grid
    m_sHalWarpingInfo.bInternalGridShow = TRUE;

    m_sHalWarpingInfo.ucGridSelX = 0;
    m_sHalWarpingInfo.ucGridSelY = 0;

    m_sHalWarpingInfo.ucPrevGridSelX = 0;
    m_sHalWarpingInfo.ucPrevGridSelY = 0;

    //halWarping_MovePitchChange( m_sHalWarpingInfo.eMovePitchIndex); //A70LV_Larry_0137

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

#ifdef C821_WARPING_ENABLE
    //dvPro_InitWarpTable();
#endif /* C821_WARPING_ENABLE */
#ifdef C789_WARPING_ENABLE
    //dvC789_InitWarpTable();
#endif /* C789_WARPING_ENABLE */


    memset(&m_sHalWarpingInfo.sWarpTable, 0, sizeof(sWARP_TABLE));
    memset(&m_sHalWarpingInfo.sAp_WarpOSD_Info,  0, sizeof(m_sHalWarpingInfo.sAp_WarpOSD_Info));  //H2PF_Simon_0038
    memset(&m_sHalWarpingInfo.sOSD_WarpOSD_Info, 0, sizeof(m_sHalWarpingInfo.sOSD_WarpOSD_Info));

    //Warp OSD
    m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0] = 255;  //H2PF_Simon_0038
    m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[1] = 255;
    m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[2] = 255;
    m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[3] = 255;
    m_sHalWarpingInfo.iPalette_ColorAmount = 256;
    memset(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette,  0, sizeof(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette));  //H2PF_Simon_0038
    memset(m_sHalWarpingInfo.asWarpOsdRect, 0, sizeof(m_sHalWarpingInfo.asWarpOsdRect));
    m_sHalWarpingInfo.uiWarpOsdRectCount = 0;
    memset(m_sHalWarpingInfo.asScalerOsdRect, 0, sizeof(m_sHalWarpingInfo.asScalerOsdRect));
    m_sHalWarpingInfo.uiScalerOsdRectCount = 0;
    m_sHalWarpingInfo.ucDrawPngOnBgEnable= 0;

    m_sHalWarpingInfo.ucWarpingCheck = TRUE;
    m_sHalWarpingInfo.ucCurrent_DBD_Data_Index        = DBD_BLENDING_NO_DATA;
    m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index = BLACKLEVEL_NO_DATA;

    return eHAL_WARPING_EXEC_CODE_PASS;
}



void halWarp_Palette_Init(void)
{
    LOG_MSG(db_HAL_WARPING, "(%s %d)\n", __FUNCTION__, __LINE__);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0] = DEF_TRANSPARENT_COLOR_IDX;  //H2PF_Simon_0038
        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[1] = DEF_TRANSPARENT_COLOR_IDX;
        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[2] = DEF_TRANSPARENT_COLOR_IDX;
        m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[3] = DEF_TRANSPARENT_COLOR_IDX;
        m_sHalWarpingInfo.iPalette_ColorAmount = 256;
        memset(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette, 0, sizeof(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette));
        halWarping_16SystemColorsPalette((UINT8 *)m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette);
        halWarping_216ColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[16][0]);
        halWarping_24GreyColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[232][0]);
        halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);

        //A70LK_CL_0004
        m_sHalWarpingInfo.ucScalerOsdSwitch = SCALER_OSD_PALETTE__MENU_TYPE;    //scaler osd's palette was init by gui (0), by warp (1)
        m_sHalWarpingInfo.uiScalerOsdInhibit_Color = 255;
        m_sHalWarpingInfo.iScalerOsdPalette_ColorAmount = 256;
        m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount = 0;
        m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount = 0;
        memset(m_sHalWarpingInfo.aucScalerOsdPalette, 0, sizeof(m_sHalWarpingInfo.aucScalerOsdPalette));
        memset(m_sHalWarpingInfo.aulScalerOsdSpritePalette  , 0, sizeof(m_sHalWarpingInfo.aulScalerOsdSpritePalette));
        memset(m_sHalWarpingInfo.aulScalerOsdMapSpritePalette, 0, sizeof(m_sHalWarpingInfo.aulScalerOsdMapSpritePalette));
        halWarping_ClearSpriteBuffer();

#ifdef FPGA_ENTRY_4K
//        halWarping_PrintWarpOsdColorPalette();  //debug
//        LOG_MSG(db_HAL_WARPING, "(%s %d)====> init Warp OSD's palette in 4K mode \n", __FUNCTION__, __LINE__);
        dvProAV_WarpOSD_Init_E4K(m_sHalWarpingInfo.iPalette_ColorAmount, (UINT8 *)m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette, m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0]);  //H2PF_Simon_0038
#else
        dvProAV_WarpOSD_Init(m_sHalWarpingInfo.iPalette_ColorAmount, (UINT8 *)m_sHalWarpingInfo.aucPalette, m_sHalWarpingInfo.uiInhibit_Color);
#endif /* FPGA_ENTRY_4K */
        //dvProAV_WarpOSD_On(eWPOSDPAGE_0);
        //dvProAV_WarpOSD_On(eWPOSDPAGE_1);


        halWarping_SemaphoreGive(__FUNCTION__);
    }
}


// ==============================================================================
// FUNCTION NAME: halWarping_WarplightReset
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
// 2018/03/26, Larry Create
// --------------------
// ==============================================================================
void halWarping_WarplightReset(void)
{
#ifdef C821_WARPING_ENABLE
    dvPro_InitWarpTable();
#endif /* C821_WARPING_ENABLE */
#ifdef C789_WARPING_ENABLE
    //dvC789_InitWarpTable();
#endif /* C789_WARPING_ENABLE */
}

void halWarping_CursorPanelConfig(void) //A70LV_Larry_0137
{
#if 0
    UINT8 ucV = 0;
    UINT8 ucH = 0;

    for(ucV = 0; ucV < 17; ucV++)
    {
        m_sHalWarpingInfo.faCursorDefInY[ucV] = (float)((m_sHalWarpingInfo.uiWarp_VW/*PM_IACT_VW*/ * ucV) / 16.0);

        for(ucH = 0; ucH < 17; ucH++)
        {
            if(ucV == 0)
            {
                 m_sHalWarpingInfo.faCursorDefInX[ucH] = (float)((m_sHalWarpingInfo.uiWarp_HW/*PM_IACT_HW*/ * ucH) / 16.0);
            }
        }
    }
#endif /* 0 */
}


// ==============================================================================
// FUNCTION NAME: halWarp_MemoryInterpolationSet
// DESCRIPTION:
//
//
// Params:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/26, Larry Create
// --------------------
// ==============================================================================
void halWarp_MemoryInterpolationSet(UINT8 *pucData)
{
    //UINT32 GV_RTCT = 0;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        halWarping_SemaphoreGive(__FUNCTION__);
    }
}

void halWarp_MemoryInterpolationGet(UINT8 *pucData)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        halWarping_SemaphoreGive(__FUNCTION__);
    }
}

// ==============================================================================
// FUNCTION NAME: halWarping_MemorySave
// DESCRIPTION:
//
//
// Params:
// UINT8 ucIndex:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/26, Larry Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarping_MemorySaveOSD_Warp(UINT8 ucIndex)   //save OSD warping
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    UINT32 ulFlash_Addr = 0x00000000;
    UINT32 ulSize = WARP_TABLE_SIZE;
    UINT32 ulBlock = 0;
    UINT32 ulCount = 0;

    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //UINT8 Temp[16385] = {0};

    //save warping type , panel flag to file
    UINT8 aucData[2] = {
                            WARPING_TYPE_OSD,
                            (m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT) ? 0 : 1   //A70LK_Simon_0010
                       };
    UINT8 aucFileName[128] = {0};
    snprintf((char *)aucFileName, 128, "/mnt/configs/Warp%d", ucIndex+1);
    FILE *fp = fopen( (char *)aucFileName , "wb" );
    fwrite(&aucData , 1 , sizeof(aucData) , fp);
    fclose(fp);

    switch(ucIndex)
    {
        case 0:
            ulFlash_Addr = FLASH_WRPTBL0_ADDR;
            break;

        case 1:
            ulFlash_Addr = FLASH_WRPTBL1_ADDR;
            break;

        case 2:
            ulFlash_Addr = FLASH_WRPTBL2_ADDR;
            break;

        case 3:
            ulFlash_Addr = FLASH_WRPTBL3_ADDR;
            break;

        case 4:
            ulFlash_Addr = FLASH_WRPTBL4_ADDR;
            break;
    }


    if((ulSize % 0x10000 == 0))
    {
        ulBlock = ulSize/0x10000;
    }
    else
    {
        ulBlock = (ulSize/0x10000) + 1;
    }

    LOG_MSG(db_HAL_WARPING,"warp save start Address = 0x%08x, Block = %d\n", ulFlash_Addr, ulBlock);
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(m_sHalWarpingInfo.ucIs4KWarp)
        {
            dvProAV_AccessWrite(eWarpFrac3bits, 1); //4K
        }
        else
        {
            dvProAV_AccessWrite(eWarpFrac3bits, 0); //2K
        }

        //dvProAV_WarpScalingSet(eOriRatio, eOriRatio);
        //dvProAV_AccessWrite(eWarpWpManualYsd,   0);
        //dvProAV_AccessWrite(eWarpWpManualXsd,   0);

        dvProAV_WarpTableDl2Chip(40*1024, &m_sHalWarpingInfo.acWarpTable[0]);

        for(ulCount = 0; ulCount < ulBlock; ulCount++)
        {
            dvProAV_SfiEraseSector((ulFlash_Addr + (0x10000*ulCount)), 0x10000);
        }


        dvProAV_DramWriteEanle(eDramBank1, DRAM_WRP_TABLE_ADDR, 1);

        dvProAV_DramBankSet(1);

        for(ulCount = 0; ulCount < ulBlock; ulCount++)
        {
            dvProAV_SfiDram2Flash((DRAM_WRP_TABLE_ADDR + (0x10000*ulCount)) * PROAV_DRAM_LINE_BYTES, (ulFlash_Addr + (0x10000*ulCount)), 0x10000, IO_WRITE);
        }

        dvProAV_DramWriteEanle(eDramBank1, DRAM_WRP_TABLE_ADDR, 0);

        dvProAV_DramBankSet(0);

        halWarping_SemaphoreGive(__FUNCTION__);
        LOG_MSG(db_HAL_WARPING,"warp save end\n");
    }

    return eResult;

}

eHAL_WARPING_EXEC_CODE halWarping_MemorySaveOSD_Blending(UINT8 ucIndex)   //save OSD Edge Blending
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //save blending type , 3D flag to file
    UINT8 aucData[2] = {
                            WARPING_TYPE_OSD,
                            (m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT) ? 0 : 1
                       };

    UINT8 aucFileName[128] = {0};
    snprintf((char *)aucFileName, 128, "/mnt/configs/Blend%d", ucIndex+1);
    FILE *fp = fopen( (char *)aucFileName , "wb" );
    fwrite(&aucData , 1 , sizeof(aucData) , fp);
    fclose(fp);

    //save blacklevel type , 3D flag to file
    snprintf((char *)aucFileName, 128, "/mnt/configs/Blacklevel%d", ucIndex+1);
    fp = fopen( (char *)aucFileName , "wb" );
    fwrite(&aucData , 1 , sizeof(aucData) , fp);
    fclose(fp);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halWarping_MemoryApply
// DESCRIPTION:
//
//
// Params:
// UINT8 ucIndex:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/26, Larry Create
// --------------------
// ==============================================================================
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyOSD_Warp(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    UINT32 ulFlash_Addr = 0x00000000;
    UINT32 ulSize = WARP_TABLE_SIZE;
    UINT32 ulBlock = 0;
    UINT32 ulCount = 0;
    //UINT8 Temp[16385] = {0};
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    //check panel
    INT8 cFileName[128] = {0};
    snprintf((char *)cFileName, 128, "/mnt/configs/Warp%d", ucIndex+1);
    UINT8 aucData[2] = {0};
    if(utilMisc_GetFileData(cFileName, aucData, sizeof(aucData)) == UTILMISC_NO_FILE)
    {
        ASSERT_ALWAYS();
        halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
        halWarp_GeometrySet();
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check ucEnableFlag
    UINT8 ucEnableFlag = aucData[0];
    if(ucEnableFlag != WARPING_TYPE_OSD)
    {
        ASSERT_ALWAYS();
        halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
        halWarp_GeometrySet();
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //check panel
    UINT8 uc3D_Flag = aucData[1];
    if(uc3D_Flag == FALSE)  //warp data is 2D
    {
        if(m_sHalWarpingInfo.ePanelTimingId != PANEL_2D_OUTPUT)  //not 2D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply OSD warp %d %d\n", uc3D_Flag, m_sHalWarpingInfo.ePanelTimingId);
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_GeometrySet();
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }
    }
    else  //warp data is 3D
    {
        if(m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT)  //not 3D panel   //A70LK_Simon_0010
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply OSD warp %d %d\n", uc3D_Flag, m_sHalWarpingInfo.ePanelTimingId);
            halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
            halWarp_GeometrySet();
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }
    }

    switch(ucIndex)
    {
        case 0:
            ulFlash_Addr = FLASH_WRPTBL0_ADDR;
            break;

        case 1:
            ulFlash_Addr = FLASH_WRPTBL1_ADDR;
            break;

        case 2:
            ulFlash_Addr = FLASH_WRPTBL2_ADDR;
            break;

        case 3:
            ulFlash_Addr = FLASH_WRPTBL3_ADDR;
            break;

        case 4:
            ulFlash_Addr = FLASH_WRPTBL4_ADDR;
            break;
    }

    //if((ulSize % 0x10000 == 0))
    //{
        //ulBlock = ulSize/0x10000;
    //}
    ///else
    //{
        //ulBlock = (ulSize/0x10000) + 1;
    //}

    LOG_MSG(db_HAL_WARPING,"warp applystart Address 0x%08x\n", ulFlash_Addr);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(m_sHalWarpingInfo.ucIs4KWarp)
        {
            dvProAV_AccessWrite(eWarpFrac3bits, 1); //4K
        }
        else
        {
            dvProAV_AccessWrite(eWarpFrac3bits, 0); //2K
        }

        //dvProAV_WarpScalingSet(eOriRatio, eOriRatio);
        //dvProAV_AccessWrite(eWarpWpManualYsd, 0);
        //dvProAV_AccessWrite(eWarpWpManualXsd, 0);

        dvProAV_DramWriteEanle(eDramBank1, DRAM_WRP_TABLE_ADDR, 1);

        //dvProAV_WarpDramWriteEanle(DRAM_WRP_TABLE_ADDR, 1);
        //dvProAV_WarpLineBufModeSet(TRUE);

        dvProAV_DramBankSet(1);

        //for(ulCount = 0; ulCount < ulBlock; ulCount++)
        //{
            dvProAV_SfiDram2Flash(DRAM_WRP_TABLE_ADDR* PROAV_DRAM_LINE_BYTES, ulFlash_Addr, WARP_TABLE_SIZE, IO_READ);
        //}

        dvProAV_WarpTableDMA(WARP_TABLE_2K_SIZE, DRAM_WRP_TABLE_ADDR);

        //dvProAV_WarpDramWriteEanle(DRAM_WRP_TABLE_ADDR, 0);
        dvProAV_DramWriteEanle(eDramBank1, DRAM_WRP_TABLE_ADDR, 0);

        dvProAV_DramBankSet(0);

        halWarping_SemaphoreGive(__FUNCTION__);

        LOG_MSG(db_HAL_WARPING,"warp apply end\n");
    }

    return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyOSD_Blending(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
    halWarping_DBD_Blending_Enable(FALSE);
    halWarping_BlackLevel_Enable(FALSE);

    halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
    halWarp_BlendingSet();

    //need to add edge blending here

    return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_ALL(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(halWarping_MemoryApplyAP_Warp(ucIndex) != eHAL_WARPING_EXEC_CODE_PASS)
    {
        LOG_MSG(db_HAL_WARPING, "Apply AP Warp Fail\n");
    }

    if(halWarping_MemoryApplyAP_Blending(ucIndex) != eHAL_WARPING_EXEC_CODE_PASS)
    {
        LOG_MSG(db_HAL_WARPING, "Apply AP Blending Fail\n");
    }

    if(halWarping_MemoryApplyAP_BlackLevel(ucIndex) != eHAL_WARPING_EXEC_CODE_PASS)
    {
        LOG_MSG(db_HAL_WARPING, "Apply AP BlackLevel Fail\n");
    }

    return eResult;
}

#if 0
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_Warp(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    INT8 cFileName[128] = {0};
    snprintf((char *)cFileName, 128, "/mnt/configs/Warp%d", ucIndex+1);
    UINT32 ulSize = sizeof(sWarpFileInfo);

    //Check file is from OSD or AP (record at first byte)
    UINT8 ucType = WARPING_TYPE_AP ;
    if(utilMisc_GetFileData(cFileName, &ucType, 1) == NO_FILE)
    {
        ASSERT_ALWAYS();
        LOG_MSG(db_HAL_WARPING, "No File (%s)\n", cFileName);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    switch(ucType)
    {
        case WARPING_TYPE_OSD: //file is from OSD save
            {
                LOG_MSG(db_HAL_WARPING, "Apply OSD Warp (%s)\n", cFileName);

                halWarping_MemoryApplyOSDWarp(ucIndex);
            }
            break;

        case WARPING_TYPE_AP:  //file is from AP save
            {
                LOG_MSG(db_HAL_WARPING, "Apply AP Warp (%s)\n", cFileName);

                UINT8 *pucData = (UINT8 *)malloc(ulSize);
                if(pucData == NULL)
                {
                    ASSERT_ALWAYS();
                    return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
                }

                if(utilMisc_GetFileData(cFileName, pucData, ulSize) == 1)
                {
                    ASSERT_ALWAYS();
                    free(pucData);
                    return eHAL_WARPING_EXEC_CODE_FAIL;
                }

                sWarpFileInfo *sWarpInfo = (sWarpFileInfo *)pucData;
                LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag %d\n",         offsetof(sWarpFileInfo, ucEnableFlag),  sWarpInfo->ucEnableFlag);
                LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",             offsetof(sWarpFileInfo, uc3DFlag),      sWarpInfo->uc3DFlag);
                LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",              offsetof(sWarpFileInfo, ucWidth),       sWarpInfo->ucWidth);
                LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",             offsetof(sWarpFileInfo, ucHeight),      sWarpInfo->ucHeight);
                LOG_MSG(db_HAL_WARPING, "0x%08X Table_TpX %f %f %f\n",    offsetof(sWarpFileInfo, afTable_TpX),   sWarpInfo->afTable_TpX[0], sWarpInfo->afTable_TpX[1], sWarpInfo->afTable_TpX[2]);
                LOG_MSG(db_HAL_WARPING, "0x%08X Table_TpY %f %f %f\n",    offsetof(sWarpFileInfo, afTable_TpY),   sWarpInfo->afTable_TpY[0], sWarpInfo->afTable_TpY[1], sWarpInfo->afTable_TpY[2]);
                LOG_MSG(db_HAL_WARPING, "0x%08X LutTable %d\n",           offsetof(sWarpFileInfo, aucLutTable),   sWarpInfo->aucLutTable[0]);

                INT32 Start = TMO_GetSysRunTime();
                eResult |= halWarping_convertTPtoDTDT(sWarpInfo->afTable_TpX, sWarpInfo->afTable_TpY, sWarpInfo->ucWidth, sWarpInfo->ucHeight);
                INT32 End = TMO_GetSysRunTime();
                LOG_MSG(db_HAL_WARPING, "\nhalWarping_convertTPtoDTDT time %d\n", End-Start);

                free(pucData);
            }
            break;

        default:
            {
                LOG_MSG(db_HAL_WARPING, "Unknown File Type (%s)(%d)\n", cFileName, ucType);
                eResult = eHAL_WARPING_EXEC_CODE_FAIL;
            }
            break;
    }


    return eResult;
}
#endif

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
    if((halWarping_TwistLinkFlag_Get() == TRUE) && (ucEnable == FALSE))
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(ucEnable)
        {
            dvProAV_WarpBypassSet(false);
        }
        else
        {
            dvProAV_WarpBypassSet(true);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

BOOL halWarping_EnableGet(void)
{
    if(dvProAV_WarpBypassGet())
    {
       return FALSE;
    }
    else
    {
        return TRUE;
    }
}


eHAL_WARPING_EXEC_CODE halWarping_convertTPtoDTDT(FLOAT *tpx, FLOAT *tpy, INT16 width, INT16 height)
{
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
#ifdef WUXGA240_32X32
    INT16 nWarpTableShift = (m_sHalWarpingInfo.uiWarp_HW >= 1920 && m_sHalWarpingInfo.ucIs4KWarp == 1) ? 8 : 16;  //8
#else
    INT16 nWarpTableShift = (m_sHalWarpingInfo.uiWarp_HW > 1920 ) ? 8 : 16;  //8
#endif /* WUXGA240_32X32 */
    UINT32 nWarpBlockSize = width * height * 4;
    UINT8 *aucWarpTable = (UINT8 *)malloc(nWarpBlockSize) ;
    UINT8 *aucTx = (UINT8 *)malloc(nWarpBlockSize) ;
    UINT8 *aucTy = (UINT8 *)malloc(nWarpBlockSize) ;

#ifdef WUXGA240_32X32
    UINT32 ulWarpTableFormat = (m_sHalWarpingInfo.uiWarp_HW >= 1920 && m_sHalWarpingInfo.ucIs4KWarp == 1) ? 32 : 16 ;
#else
    UINT32 ulWarpTableFormat = (m_sHalWarpingInfo.uiWarp_HW > 1920) ? 32 : 16 ;
#endif /* WUXGA240_32X32 */
    UINT32 ulWidth   = ROUND_UP(m_sHalWarpingInfo.uiWarp_HW , ulWarpTableFormat) + 1; //121
    UINT32 ulnHeight = ROUND_UP(m_sHalWarpingInfo.uiWarp_VW , ulWarpTableFormat) + 1; //69

    if(ulWidth < width || ulnHeight < height)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }


    if(aucWarpTable == NULL || aucTx == NULL || aucTy == NULL)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    memset(aucWarpTable, 0, nWarpBlockSize);

    LOG_MSG(db_HAL_WARPING, "nWarpBlockSize = (%d %d %d) \n", width, height, nWarpBlockSize);

    memcpy(aucTx, tpx, nWarpBlockSize);
    memcpy(aucTy, tpy, nWarpBlockSize);

    for(INT16 nHeightIdx = 0 ; nHeightIdx < height ; nHeightIdx++)
    {
        for(INT16 nWidthIdx = 0 ; nWidthIdx < width ; nWidthIdx++)
        {
             //printf("%d %d %f %f \n", nWidthIdx, nHeightIdx, tpx[nHeightIdx * width + nWidthIdx], tpy[nHeightIdx * width + nWidthIdx]);
             //LOG_MSG(db_ALWAYS, "x:%d y:%d Px:%f Py:%f (%d)\n", nWidthIdx
             //                                                 , nHeightIdx
             //                                                 , tpx[nHeightIdx * width + nWidthIdx]
             //                                                 , tpy[nHeightIdx * width + nWidthIdx]
             //                                                 , nHeightIdx * width * 4 + nWidthIdx * 4 + 3);

             //INT16 nsPx = (INT16)(*((FLOAT *)(aucTx+(sizeof(FLOAT)*(nHeightIdx * width + nWidthIdx)))) * nWarpTableShift);
             //INT16 nsPy = (INT16)(*((FLOAT *)(aucTy+(sizeof(FLOAT)*(nHeightIdx * width + nWidthIdx)))) * nWarpTableShift);

             FLOAT TempX = *((FLOAT *)(aucTx+(sizeof(FLOAT)*(nHeightIdx * width + nWidthIdx)))) ;
             FLOAT TempY = *((FLOAT *)(aucTy+(sizeof(FLOAT)*(nHeightIdx * width + nWidthIdx)))) ;
             TempX = (TempX >  6143.0) ?  6143.0 : TempX;
             TempX = (TempX < -2048.0) ? -2048.0 : TempX;
             TempY = (TempY >  6143.0) ?  6143.0 : TempY;
             TempY = (TempY < -2048.0) ? -2048.0 : TempY;

             INT16 nsPx = (INT16)(TempX * nWarpTableShift);
             INT16 nsPy = (INT16)(TempY * nWarpTableShift);

             aucWarpTable[nHeightIdx * width * 4 + nWidthIdx * 4 + 0] = (UINT8)(nsPx & 0x00FF);
             aucWarpTable[nHeightIdx * width * 4 + nWidthIdx * 4 + 1] = (UINT8)((nsPx >> 8) & 0x00FF);
             aucWarpTable[nHeightIdx * width * 4 + nWidthIdx * 4 + 2] = (UINT8)(nsPy & 0x00FF);
             aucWarpTable[nHeightIdx * width * 4 + nWidthIdx * 4 + 3] = (UINT8)((nsPy >> 8) & 0x00FF);
        }
    }

    //dvProAV_WarpBypass(false);
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        INT32 Start = TMO_GetSysRunTime();

        memcpy(&m_sHalWarpingInfo.acWarpTable[0], &aucWarpTable[0], nWarpBlockSize);

        if(dvProAV_WarpTableDl2Chip(nWarpBlockSize, &m_sHalWarpingInfo.acWarpTable[0]) != rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_FAIL;
        }
        else
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

        //default value
        dvProAV_WarpAutoWarpAndFrtSet((uint8*)&m_sHalWarpingInfo.acWarpTable[0], m_sHalWarpingInfo.uiWarp_HW16, m_sHalWarpingInfo.uiWarp_VW16, m_sHalWarpingInfo.ucIs4KWarp, NULL);

        INT32 End = TMO_GetSysRunTime();
        LOG_MSG(db_HAL_WARPING, "\ndvProAV_WarpTableDl2Chip time %d\n", End-Start);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    free(aucTx);
    free(aucTy);
    free(aucWarpTable);

    LOG_MSG(db_HAL_WARPING, "\n Exit %s \n", __FUNCTION__);

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_ClearWarpingTable(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
#ifdef WUXGA240_32X32
    UINT32 ulWarpTableFormat = (m_sHalWarpingInfo.uiWarp_HW >= 1920 && m_sHalWarpingInfo.ucIs4KWarp == 1) ? 32 : 16 ;
#else
    UINT32 ulWarpTableFormat = (m_sHalWarpingInfo.uiWarp_HW > 1920) ? 32 : 16 ;
#endif /* WUXGA240_32X32 */
    UINT32 ulWidth   = ROUND_UP(m_sHalWarpingInfo.uiWarp_HW , ulWarpTableFormat) + 1; //121
    UINT32 ulnHeight = ROUND_UP(m_sHalWarpingInfo.uiWarp_VW , ulWarpTableFormat) + 1; //69

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d %d)\r\n", __FUNCTION__, __LINE__, ulWidth, ulnHeight);

    FLOAT PxAry[ulnHeight * ulWidth], PyAry[ulnHeight * ulWidth];

    memset(PxAry, 0x00, sizeof(PxAry));
    memset(PyAry, 0x00, sizeof(PyAry));

    for(UINT32 i = 0 ; i < ulnHeight ; i++)
    {
        for(UINT32 j = 0 ; j < ulWidth ; j++)
        {
            PxAry[i * ulWidth + j] = (float)(j * ulWarpTableFormat) ;
            PyAry[i * ulWidth + j] = (float)(i * ulWarpTableFormat) ;
        }
    }

    eResult = halWarping_convertTPtoDTDT(PxAry, PyAry, ulWidth, ulnHeight) ;

	return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_DBD_Blending_Init(void)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;
    //UINT16 uiHorzResolution = 3840;  //temp
    //UINT16 uiVertResolution = 2160;  //temp

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult &= dvProAV_BldDBDInit();
        eDvResult &= dvProAV_BldDBDReset(0, 0, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDReset(1, 0, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDEnableSet(FALSE);  //A70LK_Simon_0004

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_DBD_BlendingTable_Set(UINT16 uiWidth, UINT16 uiHeight, UINT8 *pucDBD_Data)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT32 uiHorzResolution = (UINT32)uiWidth;  //temp
    UINT32 uiVertResolution = (UINT32)uiHeight;  //temp

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d %d)\r\n", __FUNCTION__, __LINE__ , uiWidth , uiHeight );

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //appSystem_Suspend();

        INT32 Start = TMO_GetSysRunTime();
        eDvResult &= dvProAV_BldDBDInit();
        eDvResult &= dvProAV_BldDBDReset(0, 0, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDReset(1, 0, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDEnableSet(TRUE);
        eDvResult &= dvProAV_BldDbdDataModeSet(0);
        eDvResult &= dvProAV_BldDBDBlockSizeSet(uiHorzResolution, (uiHorzResolution > 1920 ? 4 : 8));
        eDvResult &= dvProAV_BldDBDReadSizeSet(uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDGainBaseAddrSet(0);
        eDvResult &= dvProAV_BldDBDBlockAddrSet(0);

        if(dvProAV_PixSftSubFrameModGet() && halWarping_PanelGet() == PANEL_2D_OUTPUT)
        {
            eDvResult &= dvProAV_BldDBDDataSet_4k3d(0, pucDBD_Data, uiHorzResolution * uiVertResolution, uiHorzResolution, uiVertResolution);
        }
        else
        {
            eDvResult &= dvProAV_BldDBDDataSet(1, 0, pucDBD_Data, uiHorzResolution * uiVertResolution);
        }

        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) time %d\n", __FUNCTION__, __LINE__ , TMO_GetSysRunTime() -  Start);

        //appSystem_Resume();

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    halWarping_BlendingGamma(TRUE);

    return eResult;

}


eHAL_WARPING_EXEC_CODE halWarping_DBD_Blending_Reset(void)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d %d)\r\n", __FUNCTION__, __LINE__ , uiHorzResolution, uiVertResolution);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult &= dvProAV_BldDBDInit();
        eDvResult &= dvProAV_BldDBDReset(0, 0, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDReset(1, 0, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDEnableSet(FALSE);
        eDvResult &= dvProAV_BldDbdDataModeSet(0);
        eDvResult &= dvProAV_BldDBDBlockSizeSet(uiHorzResolution, (uiHorzResolution > 1920 ? 4 : 8));
        eDvResult &= dvProAV_BldDBDReadSizeSet(uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDGainBaseAddrSet(0);
        eDvResult &= dvProAV_BldDBDBlockAddrSet(0);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DBD_Blending_Enable(UINT8 ucEnable)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d %d)(%d)\r\n", __FUNCTION__, __LINE__ , uiHorzResolution, uiVertResolution, ucEnable);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult &= dvProAV_BldDBDEnableSet(ucEnable);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(ucEnable)
    {
        halWarping_BlendingGamma(TRUE);
    }

    return eResult;
}

BOOL halWarping_DBD_Blending_EnableGet(void)
{
    BOOL bEnable = FALSE;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        bEnable = dvProAV_BldDBDEnableGet();

        halWarping_SemaphoreGive(__FUNCTION__);
    }

    return bEnable;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Init(void)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;
    //UINT16 uiHorzResolution = 3840;
    //UINT16 uiVertResolution = 2160;

    m_sHalWarpingInfo.aucBL_GammaCoef = (FLOAT)2.2;
    memset(m_sHalWarpingInfo.aucBL_Palette, 0, 16*3);
    memset(m_sHalWarpingInfo.aucBL_GammaTable, 0, 16*3*16);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult &= dvProAV_BldDBDInit();
        eDvResult &= dvProAV_BldDBDReset(0, 2, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDReset(1, 2, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDBiasEnableSet(FALSE);  //A70LK_Simon_0004

        //init black level palette
        #if 0
        UINT8 biasTable[16] = { 0 };
        for(UINT8 i = 0 ; i < 16 ; i++)
        {
            for(UINT8 j = 0 ; j < 16 ; j++)
                biasTable[j] = 17 * i;

            eDvResult &= dvProAV_BldDBDBiasDl2Chip(i, biasTable, 3);
        }
        #endif

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Palette_Set(UINT8 ucPaletteIdx, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    FLOAT ebgmLevel[16] = { 0, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024 };

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

#ifdef BLACKLEVEL_SET_GAMMA
        if(ucRed != 0 || ucGreen != 0 || ucBlue != 0)
        {
            m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][0] = 1;
            m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][1] = 1;
            m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][2] = 1;
        }
        else
        {
            m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][0] = 0;
            m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][1] = 0;
            m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][2] = 0;
        }
#else
        m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][0] = ucRed;
        m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][1] = ucGreen;
        m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][2] = ucBlue;
#endif

        m_sHalWarpingInfo.aucBL_GammaCoef = (FLOAT)2.2;

        for(UINT8 col=0 ; col<3 ; col++)
        {
            FLOAT fBias = m_sHalWarpingInfo.aucBL_Palette[ucPaletteIdx][col];

            for(UINT8 lv=0 ; lv<16 ; lv++)
            {
                FLOAT gm = pow( (pow( ebgmLevel[lv], m_sHalWarpingInfo.aucBL_GammaCoef ) + pow( fBias, m_sHalWarpingInfo.aucBL_GammaCoef )), 1/m_sHalWarpingInfo.aucBL_GammaCoef ) - ebgmLevel[lv];
                m_sHalWarpingInfo.aucBL_GammaTable[ucPaletteIdx][col][lv] = ( gm < 0 ) ? 0 : ( gm > 255 ) ? 255 : (UINT8)gm;
            }
        }

        for(UINT8 rgb=0 ; rgb<3 ; rgb++)
        {
    		eDvResult &= dvProAV_BldDBDBiasDl2Chip(ucPaletteIdx, m_sHalWarpingInfo.aucBL_GammaTable[ucPaletteIdx][rgb], rgb);
        }


        #if 0
        printf("\n\nPalette %d\n", ucPaletteIdx);
        for(int x=0 ; x<16 ; x++)
        {
    		printf("%d [%d][%d][%d]\n" , x, m_sHalWarpingInfo.aucBL_GammaTable[ucPaletteIdx][0][x], m_sHalWarpingInfo.aucBL_GammaTable[ucPaletteIdx][1][x], m_sHalWarpingInfo.aucBL_GammaTable[ucPaletteIdx][2][x]);
        }
        #endif



        #if 0
        uint8 biasTable[16] = { 0 };
        for(int i = 0 ; i < 16 ; i++)
        {
            for(int j = 0 ; j < 16 ; j++)
                biasTable[j] = 17 * i;

            dvProAV_BldDBDBiasDl2Chip(i, biasTable, 3);
        }

        for(UINT16 index = 0 ; index < 16 ; index++)
        {
            UINT8 ucBiasTable[16] = {0};
            memcpy(ucBiasTable, (ucData+(16*index)) , 16);
            eDvResult &= dvProAV_BldDBDBiasDl2Chip(index, ucBiasTable, 3);
        }
        #endif

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


#ifdef BLACKLEVEL_SET_GAMMA
    //set blending back gamma
    if(ucRed != 0 || ucGreen != 0 || ucBlue != 0)
    {
        halWarping_SetBackGammaData(TRUE, ucRed, ucGreen, ucBlue);
        halWarping_BlendingGamma(1);
    }
#endif


    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}


#ifdef BLACKLEVEL_SET_GAMMA
eHAL_WARPING_EXEC_CODE halWarping_SetBackGammaData(UINT8 Enable, UINT8 Red, UINT8 Green, UINT8 Blue)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvProAV_SetBldBackGmaRgbSep();
        dvPro_BackGammaSet(Enable, 1, Red, Green, Blue);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Table_Set(UINT8 *ucData, UINT16 uiWidth, UINT16 uiHeight)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT32 uiHorzResolution = (UINT32)m_sHalWarpingInfo.uiWarp_HW;
    UINT32 uiVertResolution = (UINT32)m_sHalWarpingInfo.uiWarp_VW;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d %d)\r\n", __FUNCTION__, __LINE__ , uiHorzResolution , uiVertResolution);

    UINT8 *ucTableData = (UINT8 *)malloc(uiHorzResolution * uiVertResolution / 2);
    if(ucTableData == NULL)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    for(int h = 0 ; h < uiVertResolution ; h++)
    {
        for(int w = 0 ; w < uiHorzResolution ; w += 2)
        {
            uint8 pixel1 = (ucData[h * uiHorzResolution + w + 0]) & 0x0F;
            uint8 pixel2 = (ucData[h * uiHorzResolution + w + 1]) & 0x0F;
            ucTableData[h * uiHorzResolution / 2 + w / 2] = (pixel2 << 4) + pixel1;
        }
    }

    LOG_MSG(db_HAL_WARPING, "\n BL ucTableData 0x%X 0x%X 0x%X 0x%X\n" , ucTableData[0] , ucTableData[1] , ucTableData[2] , ucTableData[3] );

    #if 0
    {
        FILE *fp;
        UINT32 ulSize = (uiHorzResolution * uiVertResolution / 2);
        fp = fopen( "/tmp/FinalBK_Table.dat" , "wb" );
        fwrite(ucTableData , 1 , ulSize , fp);
        fclose(fp);
    }
    #endif


    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        INT32 Start = TMO_GetSysRunTime();


        eDvResult &= dvProAV_BldDbdDataModeSet(2);
        eDvResult &= dvProAV_BldDBDBiasBaseAddrSet(0);
        eDvResult &= dvProAV_BldDBDBlockSizeSet(uiHorzResolution, (uiHorzResolution > 1920 ? 4 : 8));
        eDvResult &= dvProAV_BldDBDReadSizeSet(uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDBiasEnableSet(TRUE);

        eDvResult &= dvProAV_BldDBDDataSet(1, 2, ucTableData, (uiHorzResolution * uiVertResolution / 2));
        //eDvResult &= dvProAV_BldDBDDataSet_4k3d(0, ucTableData, uiHorzResolution * uiVertResolution, uiHorzResolution, uiVertResolution);



        INT32 End = TMO_GetSysRunTime();
        LOG_MSG(db_HAL_WARPING, "halWarping_BlackLevel_Table_Set time %d\n", End-Start);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    free(ucTableData);

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Reset(void)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d %d)\r\n", __FUNCTION__, __LINE__ , uiHorzResolution, uiVertResolution);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult &= dvProAV_BldDBDReset(0, 2, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDReset(1, 2, uiHorzResolution, uiVertResolution);
        eDvResult &= dvProAV_BldDBDBiasEnableSet(FALSE);
        eDvResult &= dvProAV_BldDbdDataModeSet(2);
        eDvResult &= dvProAV_BldDBDBiasBaseAddrSet(0);
        eDvResult &= dvProAV_BldDBDBlockSizeSet(uiHorzResolution, (uiHorzResolution > 1920 ? 4 : 8));
        //eDvResult &= dvProAV_BldDBDBlockSizeSet(uiHorzResolution, 8);
        eDvResult &= dvProAV_BldDBDReadSizeSet(uiHorzResolution, uiVertResolution);

        //re-init palette
        #if 0
        UINT8 biasTable[16] = { 0 };
        for(UINT8 i = 0 ; i < 16 ; i++)
        {
            for(UINT8 j = 0 ; j < 16 ; j++)
                biasTable[j] = 17 * i;

            dvProAV_BldDBDBiasDl2Chip(i, biasTable, 3);
        }
        #endif

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_BlackLevel_Enable(UINT8 ucEnable)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)(%d)\r\n", __FUNCTION__, __LINE__ , ucEnable);

    UINT16 uiHorzResolution = m_sHalWarpingInfo.uiWarp_HW;
    UINT16 uiVertResolution = m_sHalWarpingInfo.uiWarp_VW;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult &= dvProAV_BldDBDBiasEnableSet(ucEnable);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_ColorUniformityApplyByOrientation(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    UINT32 ulFileSize = sizeof(sColorUniformityFileInfo);

    //open file
    char cFileName[64] = "/mnt/configs/ColorUniforimty";

    // if file is not exist
    if(access(cFileName, 0) < 0)
    {
        //LOG_MSG(db_HAL_WARPING, "ColorUniformity file is not exist!\n");
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    FILE *pFile = fopen((char *)cFileName, "rb");
    UINT8 *pucColorUniformityData = (UINT8 *)malloc(ulFileSize);

    if(pFile != NULL)
    {
        if(pucColorUniformityData != NULL)
        {
            memset(pucColorUniformityData, 0, ulFileSize);
            fread(pucColorUniformityData, 1, ulFileSize, pFile);

            sColorUniformityFileInfo *ColorUniformityInfo = (sColorUniformityFileInfo *)pucColorUniformityData;

            //check current orientation
            UINT8 ucCeilingMount = palMotorEvent_CeilingGet();
            UINT8 ucRear = palMotorEvent_RearGet();

            if(ucCeilingMount == 0 && ucRear == 0)  //table top
            {
                eResult |= halWarping_Color_Uniformity_Set(ColorUniformityInfo->aucTable_TableTop, 1900);  //H2PF_Simon_0035
            }
            else if(ucCeilingMount == 1 && ucRear == 0)  //ceiling mount
            {
                eResult |= halWarping_Color_Uniformity_Set(ColorUniformityInfo->aucTable_CeilingMount, 1900);  //H2PF_Simon_0035
            }
            else if(ucCeilingMount == 0 && ucRear == 1)  //rear projection
            {
                eResult |= halWarping_Color_Uniformity_Set(ColorUniformityInfo->aucTable_RearProjection, 1900);  //H2PF_Simon_0035
            }
            else if(ucCeilingMount == 1 && ucRear == 1)  //ceiling mount + rear projection
            {
                eResult |= halWarping_Color_Uniformity_Set(ColorUniformityInfo->aucTable_RearCeiling, 1900);  //H2PF_Simon_0035
            }

            LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)<c:%d r:%d>\r\n", __FUNCTION__, __LINE__, ucCeilingMount, ucRear);

        }
    }

    if(pucColorUniformityData == NULL)
    {
        fclose(pFile);
        free(pucColorUniformityData);
        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)malloc fail\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(pFile == NULL)
    {
        free(pucColorUniformityData);
        LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)%s can not open\n", __FUNCTION__, __LINE__ , cFileName);
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    free(pucColorUniformityData);

    return eResult;

}



eHAL_WARPING_EXEC_CODE halWarping_ColorUniformityReset(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT16 uiDataSize = 1900;
        UINT8 *pucData = (UINT8 *)malloc(uiDataSize);

        //Disable color uniformity and clear memory
        dvProAV_UnifDataShiftSet(1);
        dvProAV_UnifEnableSet(FALSE);
        dvProAV_UnifDataSet(0, 10, 8, pucData, uiDataSize);  //A70LK_CL_0001 (11,9) ==> (10,8)

        //remove Color Uniformity file
        char cFilename[64] = "/mnt/configs/ColorUniforimty" ;
        remove(cFilename);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_ColorUniformityDisable(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //Disable color uniformity
        dvProAV_UnifDataShiftSet(1);
        dvProAV_UnifEnableSet(FALSE);
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eResult;
}


int halWarping_RGBColorToIndex(uint8 paucPalette[][3], uint32 uRGBColor)
{
    for(int idx = 0 ; idx < 256 ; idx++)
    {
        if(paucPalette[idx][0] >= (uint8)((uRGBColor >> 0) & 0x000000FF))
            if(paucPalette[idx][1] >= (uint8)((uRGBColor >> 8) & 0x000000FF))
                if(paucPalette[idx][2] >= (uint8)((uRGBColor >> 16) & 0x000000FF))
                    return idx;
    }

    return 0;
}

static UINT8 current_osd = 99;
//UINT8 m_Current_OSD_Tyoe = eWDT_AFTER_WARP;

eHAL_WARPING_EXEC_CODE halWarping_DrawRect(eWARPOSD_DRAW_TYPE eType, INT16 iX, INT16 iY, INT16 iWidth, INT16 iHeight, UINT16 uiColorIndex)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    START_POINT sDes_position = {iX, iY};
    RECT_SIZE sDes_Size = {iWidth, iHeight};

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d %d)\n", __FUNCTION__
                                                         , eType
                                                         , sDes_position.iX
                                                         , sDes_position.iY
                                                         , sDes_Size.iWidth
                                                         , sDes_Size.iHeight
                                                         , uiColorIndex);
#ifdef FPGA_ENTRY_4K

        if(current_osd != eType)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eType);
            current_osd = eType;
        }

        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K((eWPOSDPAGE)eType);
        eDvResult &= dvProAV_WarpOSD_Paint_Rectangle_E4K((eWPOSDPAGE)eType, sDes_position, sDes_Size, uiColorIndex);
        halWarping_IsDraw_OSD_Set(eType, TRUE);   //H2PF_Simon_0040

        //m_Current_OSD_Tyoe = eType;
        if(current_osd == eWDT_AFTER_WARP)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eWDT_BEFORE_WARP);
            current_osd = eWDT_BEFORE_WARP;
        }
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel((eWPOSDPAGE)eType);
        eDvResult &= dvProAV_WarpOSD_Paint_Rectangle(sDes_position, sDes_Size, uiColorIndex);
#endif /* FPGA_ENTRY_4K */
        //eDvResult &= dvProAV_WarpOSD_On(eType);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        return eResult;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawCircle(eWARPOSD_DRAW_TYPE eType, INT16 iX, INT16 iY, INT16 iRadius, UINT16 uiColorIndex, UINT16 uiBGColorIndex)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d)\n", __FUNCTION__, eType, iX, iY, iRadius, uiColorIndex);

    START_POINT sDes_position = {iX, iY};

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#ifdef FPGA_ENTRY_4K
        if(current_osd != eType)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eType);
            current_osd = eType;
        }

        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K((eWPOSDPAGE)eType);
        eDvResult &= dvProAV_WarpOSD_Paint_SingleCircle_E4K(sDes_position, iRadius, uiColorIndex);
        halWarping_IsDraw_OSD_Set(eType, TRUE);   //H2PF_Simon_0040

        //m_Current_OSD_Tyoe = eType;
        if(current_osd == eWDT_AFTER_WARP)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eWDT_BEFORE_WARP);
            current_osd = eWDT_BEFORE_WARP;
        }
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel((eWPOSDPAGE)eType);
        eDvResult &= dvProAV_WarpOSD_Paint_SingleCircle(sDes_position, iRadius, uiColorIndex);
#endif
        //eDvResult &= dvProAV_WarpOSD_On();

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        return eResult;
    }

    return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_DrawCircleGrid(eWARPOSD_DRAW_TYPE eType,   //HICC2_Simon_0026
                                                          INT16 iX,
                                                          INT16 iY,
                                                          INT16 iRadius,
                                                          INT16 iPitchX,
                                                          INT16 iPitchY,
                                                          UINT16 uLimitX,
                                                          UINT16 uLimitY,
                                                          UINT16 uiColorIndex)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d)\n", __FUNCTION__, eType, iX, iY, iRadius, uiColorIndex);
    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d)\n", __FUNCTION__, iPitchX, iPitchY, uLimitX, uLimitY);

    START_POINT sDes_position = {iX, iY};
    CENTER_PITCH sPitch = {iPitchX, iPitchY};
    START_POINT sLimit = {uLimitX, uLimitY};

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(current_osd != eType)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eType);
            current_osd = eType;
        }

        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K((eWPOSDPAGE)eType);
        eDvResult &= dvProAV_WarpOSD_Paint_MultiCircle_E4K(sDes_position, iRadius, sPitch, sLimit, uiColorIndex);

        halWarping_IsDraw_OSD_Set(eType, TRUE);   //H2PF_Simon_0040

        //m_Current_OSD_Tyoe = eType;
        if(current_osd == eWDT_AFTER_WARP)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eWDT_BEFORE_WARP);
            current_osd = eWDT_BEFORE_WARP;
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        return eResult;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_CopyOSD(eWARPOSD_DRAW_TYPE eType, INT16 iSrcX, INT16 iSrcY, INT16 iSrcWidth, INT16 iSrcHeight, INT16 iDesX, INT16 iDesY)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    LOG_MSG(db_HAL_WARPING, "%s(%d %d %d %d %d %d %d)\n", __FUNCTION__, eType, iSrcX, iSrcY, iSrcWidth, iSrcHeight, iDesX, iDesY);

    OSD_BITMAP sBitmap = { {iSrcX,iSrcY},{iSrcWidth,iSrcHeight} };
    START_POINT sDes_position = {iDesX, iDesY};

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#ifdef FPGA_ENTRY_4K
        if(current_osd != eType)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eType);
            current_osd = eType;
        }

        if(dvProAV_WarpOSD_CurrentCopy_E4K(eType, sBitmap, sDes_position) == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

        //m_Current_OSD_Tyoe = eType;
        if(current_osd == eWDT_AFTER_WARP)
        {
            eDvResult &= dvProAV_WarpOSD_Set_E4K(eWDT_BEFORE_WARP);
            current_osd = eWDT_BEFORE_WARP;
        }
#else
        if(dvProAV_WarpOSD_CurrentCopy(eType, sBitmap, sDes_position) == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }
#endif /* FPGA_ENTRY_4K */

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        return eResult;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend)
{
    //extern UINT8 aucPalette[256 * 3];  //temp
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    INT32 m_nHorzResolution = halWarping_HResGet();  //A70LK_Simon_0001
    INT32 m_nVertResolution = halWarping_VResGet();

    UINT32 ulSpriteOsdAddress;

    UINT8 ucTmpData[m_nHorzResolution];
    memset(ucTmpData, 0, m_nHorzResolution);


    LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);

    UINT8 *paucData = (UINT8 *)malloc(halWarping_HResGet() * halWarping_VResGet());  //A70LK_Simon_0001
    if(paucData == NULL)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }
    memset(paucData, 0xFF, (halWarping_HResGet() * halWarping_VResGet()));  //A70LK_Simon_0001

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_1);

#ifdef FPGA_ENTRY_4K
        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_1);
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE_1);
#endif /* FPGA_ENTRY_4K */

        INT32 tmpWidth  = (xPos + ulWidth)  > m_nHorzResolution ? m_nHorzResolution : ulWidth  + xPos;
        INT32 tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight + yPos;

        for (int y = yPos; y < tmpHeight; y++)
        {
//            memset(ucTmpData, 0x10, sizeof(ucTmpData)); // nInhibitColor
            memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // transparent
            for (int x = xPos; x < tmpWidth; x++)
            {
                uint32 nXPos = x - xPos;
                uint32 nYPos = y - yPos;
                uint32 uiRedColor   = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 0];
                uint32 uiGreenColor = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 1];
                uint32 uiBlueColor  = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 2];

                uint32 uiColor = uiBlueColor * 65536 + uiGreenColor * 256 + uiRedColor;
                paucData[nYPos * m_nHorzResolution + nXPos] = (uint8)halWarping_RGBColorToIndex(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette, uiColor);  //H2PF_Simon_0038
                ucTmpData[x] = paucData[nYPos * m_nHorzResolution + nXPos];
//                printf("uiColor : %X ColorIdx : %d \n", uiColor, paucData[nYPos * m_nHorzResolution + nXPos]);
            }

            eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + y * m_nHorzResolution / 64, m_nHorzResolution / 64, ucTmpData);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        free(paucData);
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        return eResult;
    }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

    free(paucData);

    return eResult;
}

#if 0    //H2PF_Simon_0038 removea
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Set(INT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        m_sHalWarpingInfo.aucPalette[uiIndex][0] = ucRed;
        m_sHalWarpingInfo.aucPalette[uiIndex][1]  = ucGreen;
        m_sHalWarpingInfo.aucPalette[uiIndex][2] = ucBlue;

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Update(void)  //UPDATE warp OSD palette
{
    LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#ifdef FPGA_ENTRY_4K
        dvProAV_WarpOSD_Palette_Set_E4K((INT16)256, (UINT8 *)m_sHalWarpingInfo.aucPalette);
#else
        dvProAV_WarpOSD_Palette_Set((INT16)256, (UINT8 *)m_sHalWarpingInfo.aucPalette);
#endif /* FPGA_ENTRY_4K */

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif


void halWarping_16SystemColorsPalette(UINT8 *paucPalette)
{
#if 0
    UINT8 auc16SystemColorsPalette[] = {
        0x00, 0x00, 0x00,    // 000 Black
        0x00, 0x00, 0x00,    // 001 Maroon
        0x00, 0x00, 0x00,    // 002 Green
        0x00, 0x00, 0x00,    // 003 Olive
        0x00, 0x00, 0x00,    // 004 Navy
        0x80, 0x00, 0x00,    // 005 Purple
        0xFF, 0x00, 0x00,    // 006 Teal
        0x00, 0x00, 0x00,    // 007 Silver
        0x00, 0x00, 0x00,    // 008 Grey
        0xFF, 0x00, 0x00,    // 009 Red
        0x00, 0xFF, 0x00,    // 010 Lime
        0xFF, 0xFF, 0x00,    // 011 Yellow
        0x00, 0x00, 0xFF,    // 012 Blue
        0xFF, 0x00, 0xFF,    // 013 Fuchsia
        0x00, 0xFF, 0xFF,    // 014 Aqua
        0xFF, 0xFF, 0xFF     // 015 White
    };
#endif

    UINT8 auc16SystemColorsPalette[] = {
        0x00, 0x00, 0x00,    // 000 Black
        0xFF, 0x00, 0x00,    // 001 Maroon
        0x00, 0xFF, 0x00,    // 002 Green
        0x00, 0x00, 0xFF,    // 003 Olive
        0x00, 0x00, 0x00,    // 004 Navy
        0x80, 0x00, 0x00,    // 005 Purple
        0xFF, 0x00, 0x00,    // 006 Teal
        0x00, 0x00, 0x00,    // 007 Silver
        0x00, 0x00, 0x00,    // 008 Grey
        0xFF, 0x00, 0x00,    // 009 Red
        0x00, 0xFF, 0x00,    // 010 Lime
        0xFF, 0xFF, 0x00,    // 011 Yellow
        0x00, 0x00, 0xFF,    // 012 Blue
        0xFF, 0x00, 0xFF,    // 013 Fuchsia
        0x00, 0xFF, 0xFF,    // 014 Aqua
        0xFF, 0xFF, 0xFF     // 015 White
    };


    memcpy(paucPalette, auc16SystemColorsPalette, sizeof(auc16SystemColorsPalette));

}

void halWarping_216ColorsPalette(UINT8 *paucPalette)
{
    UINT8 auc216ColorsPalette[216 * 3] = { 0 };

    for(UINT8 ucRedIdx = 0 ; ucRedIdx < 6 ; ucRedIdx++)
    {
        for(UINT8 ucGreenIdx = 0 ; ucGreenIdx < 6 ; ucGreenIdx++)
        {
            for(UINT8 ucBlueIdx = 0 ; ucBlueIdx < 6 ; ucBlueIdx++)
            {
                auc216ColorsPalette[(ucRedIdx * 36 + ucGreenIdx * 6 + ucBlueIdx) * 3 + 0] = ucRedIdx * 40   + (ucRedIdx == 0 ? 0 : 55);
                auc216ColorsPalette[(ucRedIdx * 36 + ucGreenIdx * 6 + ucBlueIdx) * 3 + 1] = ucGreenIdx * 40 + (ucGreenIdx == 0 ? 0 : 55);
                auc216ColorsPalette[(ucRedIdx * 36 + ucGreenIdx * 6 + ucBlueIdx) * 3 + 2] = ucBlueIdx * 40  + (ucBlueIdx == 0 ? 0: 55);
            }
        }
    }

    //for(int idx = 0 ; idx < 216 ; idx++)
    //{
    //    printf("%d rgb(%d, %d, %d) \n", idx+16, auc216ColorsPalette[idx * 3 + 0], auc216ColorsPalette[idx * 3 + 1], auc216ColorsPalette[idx * 3 + 2]);
    //}

    memcpy(paucPalette, auc216ColorsPalette, sizeof(auc216ColorsPalette));

}

void halWarping_24GreyColorsPalette(UINT8 *paucPalette)
{
    UINT8 auc24GreyColorsPalette[24 * 3] = { 0 };

    for(UINT8 idx = 0 ; idx < 24 ; idx++)
    {
        auc24GreyColorsPalette[idx * 3 + 0] = 8 + idx * 10;
        auc24GreyColorsPalette[idx * 3 + 1] = 8 + idx * 10;
        auc24GreyColorsPalette[idx * 3 + 2] = 8 + idx * 10;
    }

    memcpy(paucPalette, auc24GreyColorsPalette, sizeof(auc24GreyColorsPalette));
}

//A65_OPTOMA_CL_0005
eHAL_WARPING_EXEC_CODE halWarpOSD_Color_Initial( UINT8 ucInitPalette )
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    UINT8 ucRetry = 0;
#if 0  //A65_OPTOMA_CL_0006
        for( ; ucRetry < 3; ucRetry++ )
        {
            if( TRUE == halWarping_SemaphoreTake(__FUNCTION__) )
        	{
                //init 256 color palettes for blender
                for(UINT16 uiIndex = 0 ; uiIndex < 256 ; uiIndex++)
                {
                    dvC789_WriteToBuffer(B1_PLTAD, uiIndex);
                    dvC789_WriteToBuffer(B1_PLTDT, m_sHalWarpingInfo.aucPalette[uiIndex][0]);
                    dvC789_WriteToBuffer(B1_PLTDT, m_sHalWarpingInfo.aucPalette[uiIndex][1]);
                    dvC789_WriteToBuffer(B1_PLTDT, m_sHalWarpingInfo.aucPalette[uiIndex][2]);
                }
                dvC789_WriteToBuffer(B1_BOTRANS0, 0);  //Transparent color set to index 0
                dvC789_WriteToBuffer(B1_BOTRANS1, 0);
                dvC789_WriteToBuffer(B1_BOTRANS2, 0);
                dvC789_WriteToBuffer(B1_BOTRANS3, 0);

                dvC789_WriteToBuffer(B1_BLINKTIME, 0);  //no blink
                dvC789_WriteToBuffer(B1_BLINKCYCL, 0);

                dvC789_Buffer_Flush();

                halWarping_SemaphoreGive(__FUNCTION__);
                eRet = eHAL_WARPING_EXEC_CODE_PASS;
                break;
            }
            MS_SLEEP(2);
        }
#else
        //init 256 color palettes for blender
        if(ucInitPalette)
        {
            for(UINT16 uiIndex = 0 ; uiIndex < 256 ; uiIndex++)
            {

            }
        }

        eRet = eHAL_WARPING_EXEC_CODE_PASS;
#endif
    return (eRet);
}

#if 0
//A65_OPTOMA_CL_0006
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Group_Set(eCOLOR_PALETTE_GROUP eColorPaletteGroup)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //utilWarp_SetColorPaletteGroupIndex(eColorPaletteGroup);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_SetAll(UINT8 *aucRed, UINT8 *aucGreen, UINT8 *aucBlue)  //H2PF_Simon_0038 remove
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        for(UINT16 uiIndex = 0 ; uiIndex < 256 ; uiIndex++)
        {
            m_sHalWarpingInfo.aucPalette[uiIndex][0] = aucRed[uiIndex];
            m_sHalWarpingInfo.aucPalette[uiIndex][1] = aucGreen[uiIndex];
            m_sHalWarpingInfo.aucPalette[uiIndex][2] = aucBlue[uiIndex];

            halWarping_PaletteUpdate(uiIndex, aucRed[uiIndex], aucGreen[uiIndex], aucBlue[uiIndex]);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif /* 0 */

eHAL_WARPING_EXEC_CODE halWarpOSD_ClearOSD(eWARPOSD_DRAW_TYPE eType)
{
    LOG_MSG(db_HAL_WARPING, "(%s %d) eType = %d\n", __FUNCTION__, __LINE__, eType);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#ifdef FPGA_ENTRY_4K
        if(current_osd != eType)
        {
            dvProAV_WarpOSD_Set_E4K(eType);
            current_osd = eType;
        }

        dvProAV_WarpOSD_MemplaneSel_E4K(eType);
        dvProAV_WarpOSD_PageClear_E4K(eType);
        dvProAV_WarpOSD_On_E4K(eType);
        halWarping_IsDraw_OSD_Set(eType, FALSE);   //H2PF_Simon_0040

        //m_Current_OSD_Tyoe = eType;
        if(current_osd == eWDT_AFTER_WARP)
        {
            dvProAV_WarpOSD_Set_E4K(eWDT_BEFORE_WARP);
            current_osd = eWDT_BEFORE_WARP;
        }

        if(utilWarp_GetWarpCtrl() == WARP_CTRL__ADVANCED)
        {
            utilWarp_SetColorPaletteGroupIndex(COLOR_PALETTE__SCALER_OSD);
            //halWarpOSD_SetLastAddress_E4K(eWDT_BEFORE_WARP);
        }
#else
        dvProAV_WarpOSD_MemplaneSel(eType);
        dvProAV_WarpOSD_PageClear();
        dvProAV_WarpOSD_On(eType);
#endif /* FPGA_ENTRY_4K */
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_DrawOSD(eWARPOSD_DRAW_TYPE eType)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        LOG_MSG(db_HAL_RESERVED19, "(%s %d) eType = %d (cur %d)\n", __FUNCTION__, __LINE__, eType, current_osd);

        //dvProAV_Inhibit_Color_Set(GUI_ON, GUI_INHITBIT_COLOR);  //A70LK_CL_0004
#ifdef FPGA_ENTRY_4K

        if(current_osd != eType)
        {
            dvProAV_WarpOSD_Set_E4K(eType);
            current_osd = eType;
        }

        dvProAV_WarpOSD_On_E4K(eType);
        halWarping_IsDraw_OSD_Set(eType, TRUE);   //H2PF_Simon_0040
        //m_Current_OSD_Tyoe = eType;
        if(current_osd == eWDT_AFTER_WARP)
        {
            dvProAV_WarpOSD_Set_E4K(eWDT_BEFORE_WARP);
            current_osd = eWDT_BEFORE_WARP;
        }

#else
        dvProAV_WarpOSD_On(eType);
#endif /* FPGA_ENTRY_4K */
        //dvProAV_Inhibit_Color_Set(GUI_OFF, GUI_INHITBIT_COLOR);  //A70LK_CL_0004
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Set_E4K(eWARPOSD_DRAW_TYPE eType)
{
    LOG_MSG(db_HAL_WARPING, "(%s %d) eType = %d\n", __FUNCTION__, __LINE__, eType);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#ifdef FPGA_ENTRY_4K
        dvProAV_WarpOSD_Set_E4K(eType);
#endif /* FPGA_ENTRY_4K */
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

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
    UINT32 ulTransparentColor = 0;

    //halWarpOSD_Palette_Group_Set(COLOR_PALETTE__AP);  //A65_OPTOMA_CL_0010

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //ulTransparentColor = dvC789_Read(B1_BOTRANS0);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail", __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "%s (%d)\n", __FUNCTION__ , eType);

    if(eType == eWDT_BEFORE_WARP)
    {
        halWarpOSD_AP_ClearMemoryOSD();
    }
    else if(eType == eWDT_AFTER_WARP)
    {
#if 0
        if( utilWarp_GetOsdPatternType() != PAT_TYPE__OFF)//A65_OPTOMA_CL_0010
        {
            if(utilWarp_GetOsdPatternType() == PAT_TYPE__TESTPATTERN_BLACK_BORDER_MULTIPLESELECT ||
               utilWarp_GetOsdPatternType() == PAT_TYPE__DRAW_CIRCLE_GRID)
            {
                halAdvWarpShowOsdPattern_Unblocked_TwistLink(PAT_TYPE__OFF);
            }
            else
            {
                if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
                {
                    utilWarp_ShowOsdPattern(PAT_TYPE__OFF);
                    halWarping_SemaphoreGive(__FUNCTION__);
                }
                else
                {
                    LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail", __FUNCTION__);
                    return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
                }
            }
        }
#endif /* 0 */
        halWarping_AP_DrawRect(eWDT_AFTER_WARP,  0, 0, m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW, ulTransparentColor);
        halWarping_OSD_ON(eWDT_AFTER_WARP);    //R70G2_Simon_0002  //H2PF_Simon_0035
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarpOSD_SetLastAddress_E4K(eWARPOSD_DRAW_TYPE eType)
{
    LOG_MSG(db_HAL_WARPING, "(%s %d) eType = %d\n", __FUNCTION__, __LINE__, eType);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#ifdef FPGA_ENTRY_4K
        dvProAV_WarpOSD_MemplaneLastSel_E4K(eType);
#endif /* FPGA_ENTRY_4K */
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_DisableOSD(void)
{
    //dvC789_Write( B1_OSDMODE, 0x00 );

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarp_BlacklevelParameterSet(eBLACKLEVEL_PARA eParameter, INT32 lData)
{
    switch(eParameter)
    {
        case eBKLV_EVENT_AREA_SEL:
            //ucBlacklevelAreaSel = lData;
            //m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_PLT_SEL = lData;
            break;

        //coordinate
        case eBKLV_EVENT_CUR_TL_X:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_X = lData;
            break;

        case eBKLV_EVENT_CUR_TL_Y:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_Y = lData;
            break;

        case eBKLV_EVENT_CUR_TR_X:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_X = lData;
            break;

        case eBKLV_EVENT_CUR_TR_Y:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_Y = lData;
            break;

        case eBKLV_EVENT_CUR_BL_X:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_X = lData;
            break;

        case eBKLV_EVENT_CUR_BL_Y:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_Y = lData;
            break;

        case eBKLV_EVENT_CUR_BR_X:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_X = lData;
            break;

        case eBKLV_EVENT_CUR_BR_Y:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_Y = lData;
            break;


        //palette R,G,B
        case eBKLV_EVENT_PLT_RED:
#if 0
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
#endif /* 0 */
            break;

        case eBKLV_EVENT_PLT_GREEN:
#if 0
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
#endif /* 0 */
            break;

        case eBKLV_EVENT_PLT_BLUE:
#if 0
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
#endif /* 0 */
            break;


        case eBKLV_EVENT_AREA_ENABLE:
            //m_sBlackLevelPara[ucBlacklevelAreaSel].ucBlackLevel_AreaEnable = (UINT8)lData;
            break;

        case eBKLV_EVENT_APPLY:
#if 0
            halWarping_Blacklevel_AreaApply(ucBlacklevelAreaSel,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TL_Y,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_TR_Y,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BL_Y,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_X,
                                            m_sBlackLevelPara[ucBlacklevelAreaSel].uiBlackLevel_CUR_BR_Y);
#endif /* 0 */
            break;


        case eBKLV_EVENT_APPLY_ALL_AREA:
            halWarping_Blacklevel_ApplyAllArea();
            halWarping_Blacklevel_SaveData(BLACKLEVEL_CURRENT_INDEX);  //A35G2_Simon_0093
            break;

        case eBKLV_EVENT_ENABLE:
            halWarping_BlackLevel_Enable(lData);
            break;

        case eBKLV_EVENT_RESET:
            halWarping_BlackLevel_Reset();  //wait review
            break;

        default:
            LOG_MSG(db_HAL_WARPING, "%s para set error\n\n", __FUNCTION__);
            break;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}


INT32 halWarp_BlacklevelParameterGet(eBLACKLEVEL_PARA eParameter)
{
#if 0
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
#endif /* 0 */

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
#if 0
    coord_t sCursor_TL = {iCursor_TL_X, iCursor_TL_Y};
    coord_t sCursor_TR = {iCursor_TR_X, iCursor_TR_Y};
    coord_t sCursor_BL = {iCursor_BL_X, iCursor_BL_Y};
    coord_t sCursor_BR = {iCursor_BR_X, iCursor_BR_Y};

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvC789_Blacklevel_Palette_Set(cPaletteSelect,
                                      m_sBlackLevelPara[cPaletteSelect].ucBlackLevel_PLT_R,
                                      m_sBlackLevelPara[cPaletteSelect].ucBlackLevel_PLT_G,
                                      m_sBlackLevelPara[cPaletteSelect].ucBlackLevel_PLT_B
                                      );

        dvC789_EdgebldBiasEnable(TRUE);

        dvC789_BlackLevel_AreaWrite(cPaletteSelect, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, TRUE);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX FAIL\n" , __FUNCTION__);
    }
#endif /* 0 */
}

void halWarping_Blacklevel_ApplyAllArea(void)
{
#if 0
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //clear
        coord_t sCursor_TL = {0,                               0};
        coord_t sCursor_TR = {m_sHalWarpingInfo.uiWarp_HW - 1, 0};
        coord_t sCursor_BL = {0,                               m_sHalWarpingInfo.uiWarp_VW - 1};
        coord_t sCursor_BR = {m_sHalWarpingInfo.uiWarp_HW - 1, m_sHalWarpingInfo.uiWarp_VW - 1};
        dvC789_BlackLevel_AreaWrite(0, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, FALSE);

        //set palette
        for(int i=0 ; i<15 ; i++)
        {
            if(i > 0 && m_sBlackLevelPara[i].ucBlackLevel_AreaEnable == FALSE)  //area 0 don`t care "Enable"      //A35G2_Simon_0106
            {
                m_sBlackLevelPara[i].ucBlackLevel_ACTIVE = FALSE;
                continue;
            }

            dvC789_Blacklevel_Palette_Set(m_sBlackLevelPara[i].ucBlackLevel_PLT_SEL,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_B);
        }

        dvC789_EdgebldBiasEnable(TRUE);

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

            dvC789_BlackLevel_AreaWrite(m_sBlackLevelPara[i].ucBlackLevel_PLT_SEL, sCursor_TL, sCursor_TR, sCursor_BL, sCursor_BR, FALSE);

            m_sBlackLevelPara[i].ucBlackLevel_ACTIVE = TRUE;  //area 0 always Active
        }

        dvC789_EdgebldBiasEnable(TRUE);

        dvC789_LoadEgbBiasArea(BLACKLEVEL_TMP_FILE /*"ebarea0.bak"*/);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX FAIL\n" , __FUNCTION__);
    }
#endif /* 0 */
}


void halWarping_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //dvC789_Blacklevel_Palette_Set(cPaletteIdx, cRed, cGreen, cBlue);

        //dvC789_EdgebldBiasEnable(TRUE);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX FAIL\n" , __FUNCTION__);
    }
}


#if 0
eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ChangeToFullData(char *cFileName)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT8 ucOneLineBuf[m_sHalWarpingInfo.uiWarp_HW + 5];
        memset(ucOneLineBuf, 0, m_sHalWarpingInfo.uiWarp_HW + 5);

        UINT8 *aucTable = (UINT8 *)malloc(m_sHalWarpingInfo.uiWarp_HW * m_sHalWarpingInfo.uiWarp_VW);
        if(aucTable == NULL)
        {
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }

        FILE *fp = fopen(cFileName, "r");
        if(fp == NULL)
        {
            LOG_MSG(db_HAL_WARPING, "No Blacklevel temp file\n");
            free(aucTable);
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
                aucTable[i + (line * m_sHalWarpingInfo.uiWarp_HW)] = ucOneLineBuf[i];
            }

        }

        fclose(fp);

        {
            FILE *fp_tmp;
            UINT32 ulSize = m_sHalWarpingInfo.uiWarp_HW * m_sHalWarpingInfo.uiWarp_VW;
            fp_tmp = fopen( cFileName , "w" );
            if(fp_tmp != NULL)
            {
                fwrite(aucTable , 1 , ulSize , fp_tmp);
                fclose(fp_tmp);
            }

            LOG_MSG(db_HAL_WARPING, "\n Table Size = %d\n\n" , ulSize);
        }

        free(aucTable);

        halWarping_SemaphoreGive(__FUNCTION__);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_WriteFullData(UINT8 ucIndex)
{
    halWarping_Blacklevel_ChangeToFullData("./blacklevel_raw_table0.dat");

    halWarping_Blacklevel_Reset();

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        // load file
        UINT32 ulFileSize = sizeof(sBlackLevelFileData);
        UINT8 *pucData = (UINT8 *)malloc(ulFileSize);
        if(pucData == NULL)
        {
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }

        char cFileName[128];
        sprintf(cFileName, BLACKLEVEL_FILE, 0);
        FILE *fp_read = fopen(cFileName, "rb");
        if(fp_read == NULL)
        {
            free(pucData);
            LOG_MSG(db_HAL_WARPING, "Can not open file %s\n", cFileName);
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }
        else
        {
            fread(pucData, 1, ulFileSize, fp_read);
            fclose(fp_read);
        }

        sBlackLevelFileData *sBlackLevelInfo = (sBlackLevelFileData *)pucData;

        LOG_MSG(db_HAL_WARPING, "0x%08X EnableFlag %d\n",         offsetof(sBlackLevelFileData, ucEnableFlag),    sBlackLevelInfo->ucEnableFlag);
        LOG_MSG(db_HAL_WARPING, "0x%08X 3DFlag %d\n",             offsetof(sBlackLevelFileData, uc3DFlag),        sBlackLevelInfo->uc3DFlag);
        LOG_MSG(db_HAL_WARPING, "0x%08X Width %d\n",              offsetof(sBlackLevelFileData, ucWidth),         sBlackLevelInfo->ucWidth);
        LOG_MSG(db_HAL_WARPING, "0x%08X Height %d\n",             offsetof(sBlackLevelFileData, ucHeight),        sBlackLevelInfo->ucHeight);
        LOG_MSG(db_HAL_WARPING, "0x%08X BiasPalette %d %d %d\n",  offsetof(sBlackLevelFileData, aucBiasPalette),  sBlackLevelInfo->aucBiasPalette[0], sBlackLevelInfo->aucBiasPalette[1], sBlackLevelInfo->aucBiasPalette[2]);

        for(int i=0 ; i<MAX_BLACKLEVEL_AREA_NUMBER ; i++)
        {
            m_sBlackLevelPara[i].ucBlackLevel_PLT_R = sBlackLevelInfo->aucBiasPalette[0 + i*3] ;
            m_sBlackLevelPara[i].ucBlackLevel_PLT_G = sBlackLevelInfo->aucBiasPalette[1 + i*3] ;
            m_sBlackLevelPara[i].ucBlackLevel_PLT_B = sBlackLevelInfo->aucBiasPalette[2 + i*3] ;

            dvC789_Blacklevel_Palette_Set(i,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                          m_sBlackLevelPara[i].ucBlackLevel_PLT_B);

            LOG_MSG(db_HAL_WARPING, "%d load R%d G%d B%d\n" , i,
                                                              m_sBlackLevelPara[i].ucBlackLevel_PLT_R,
                                                              m_sBlackLevelPara[i].ucBlackLevel_PLT_G,
                                                              m_sBlackLevelPara[i].ucBlackLevel_PLT_B);

        }

        free(pucData);

        dvC789_EdgebldBiasEnable(TRUE);

        UINT8 *pucRawData = (UINT8 *)malloc(1920 * 1200);
        FILE *fp_rawtable = fopen("./blacklevel_raw_table0.dat", "rb");
        if(fp_rawtable == NULL)
        {
            LOG_MSG(db_HAL_WARPING, "Can not open file %s\n");
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }
        else
        {
            fread(pucRawData, 1, 1920 * 1200, fp_rawtable);
            fclose(fp_rawtable);
        }

        UINT8 *ucTableData = (UINT8 *)malloc(1920 * 1200 / 2);
        for(int y = 0 ; y < 1200 ; y++)
        {
            for(int x = 0 ; x < 1920 ; x += 2)
            {
                uint8 pixel1 = (pucRawData[y * 1920 + x + 0]) & 0x0F;
                uint8 pixel2 = (pucRawData[y * 1920 + x + 1]) & 0x0F;
                ucTableData[y * 1920 / 2 + x / 2] = (pixel2 << 4) + pixel1;
            }

    		int ad = DEF_EBIASSAD + y * DEF_EBIASMWI * 256;
    		dvC789_Write( B0_CPUWAD, ad );
    		//dvC789_BurstWrite_FixedAdd( B0_CPUDT, 1920/2 , &ucTableData[y * 1920 / 2] );
    		UINT32 ulWriteSize = 1920;
    		for(int WriteStart=0 ; WriteStart < ulWriteSize ; WriteStart+=500)
    		{
                //write last block
        		if(WriteStart + 500 >= ulWriteSize)
        		{
            		dvC789_Write( B0_CPUWAD, ad + WriteStart );
            		dvC789_BurstWrite_FixedAdd( B0_CPUDT, (ulWriteSize - WriteStart) , &ucTableData[y * 1920 / 2 + WriteStart] );
            		break;
        		}

        		dvC789_Write( B0_CPUWAD, ad + WriteStart );
        		dvC789_BurstWrite_FixedAdd( B0_CPUDT, 500 , &ucTableData[y * 1920 / 2 + WriteStart] );

       		}
        }

        free(ucTableData);
        free(pucRawData);

        halWarping_SemaphoreGive(__FUNCTION__);
    }

}
#endif


eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_SaveData(UINT8 ucIndex)  //A35G2_Simon_0093
{
#if 0
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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
        	dvC789_Write(B0_CPUWAD, DEF_EBIASSAD);
        	dvC789_Write(B0_CPURAD, DEF_EBIASSAD);

        	// erase Black Level Group
            LOG_MSG(db_HAL_WARPING, "Erase...\n");
        	UINT32 ulAddress = ucIndex * WARPING_MEMORY_GROUP_SIZE + WARPING_TWIST_BIAS_TABLE_ADDRESS;
        	dvC789_FlashEraseSector(ulAddress, WARPING_TWIST_BIAS_TABLE_SIZE) ;
            LOG_MSG(db_HAL_WARPING, "Erase Done\n");

            dvC789_DMA_Transfer(ulAddress, WARPING_TWIST_BIAS_TABLE_SIZE, eC789_SFLREGSEL_DDR, eC789_SFLCT_DMA_REG2FLASH);
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
        snprintf((char *)sFile.aucRawDataFileName, 64, cRawDataFileName);
        SYSTEM_CALL("cp %s %s", BLACKLEVEL_TMP_FILE, cRawDataFileName);

        //write to file
        char cWriteFileName[128];
        sprintf(cWriteFileName, BLACKLEVEL_FILE, ucIndex);

        FILE *fp_save = fopen(cWriteFileName, "wb");
        if(fp_save == NULL)
        {
            LOG_MSG(db_HAL_WARPING, "Can not open file %s\n", cWriteFileName);
            halWarping_SemaphoreGive(__FUNCTION__);
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }
        else
        {
            fwrite(&sFile , sizeof(sBlackLevelFileData) , 1 , fp_save);
            fflush(fp_save);
            fclose(fp_save);
        }

	    LOG_MSG(db_HAL_WARPING, "Save Blacklevel End\n");

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_LoadData(UINT8 ucIndex)  //A35G2_Simon_0093
{
#if 0
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
	    LOG_MSG(db_HAL_WARPING, "Load Blacklevel %d Start \n", ucIndex);

        char cFileName[128];

        // load file
        UINT32 ulFileSize = sizeof(sBlackLevelFileData);
        UINT8 *pucData = (UINT8 *)malloc(ulFileSize);
        if(pucData == NULL)
        {
            halWarping_SemaphoreGive(__FUNCTION__);
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }

        sprintf(cFileName, BLACKLEVEL_FILE, ucIndex);
        FILE *fp_read = fopen(cFileName, "rb");
        if(fp_read == NULL)
        {
            free(pucData);
            LOG_MSG(db_HAL_WARPING, "Can not open file %s\n", cFileName);
            halWarping_SemaphoreGive(__FUNCTION__);

            //remove current data
            halWarping_Blacklevel_ClearCurrentData();
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }
        else
        {
            fread(pucData, 1, ulFileSize, fp_read);
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
            halWarping_SemaphoreGive(__FUNCTION__);
            return eHAL_WARPING_EXEC_CODE_FAIL;
        }

        for(int i=0 ; i<MAX_BLACKLEVEL_AREA_NUMBER ; i++)
        {
            //load blacklevel palette
            m_sBlackLevelPara[i].ucBlackLevel_PLT_R = sBlackLevelInfo->aucBiasPalette[0 + i*3] ;
            m_sBlackLevelPara[i].ucBlackLevel_PLT_G = sBlackLevelInfo->aucBiasPalette[1 + i*3] ;
            m_sBlackLevelPara[i].ucBlackLevel_PLT_B = sBlackLevelInfo->aucBiasPalette[2 + i*3] ;

            dvC789_Blacklevel_Palette_Set(i,
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

        dvC789_LoadEgbBiasArea(cCurrentRawDataFileName);

        free(pucData);

        #if 0
        //load DMA
        dvC789_Write(B0_OSDCT, 0x00);
        dvC789_Write(B0_CPUWAD, DEF_EBIASSAD);

        UINT32 ulAddress = WARPING_TWIST_BIAS_TABLE_ADDRESS + WARPING_MEMORY_GROUP_SIZE * ucIndex;
        dvC789_DMA_Transfer(ulAddress, WARPING_TWIST_BIAS_TABLE_SIZE, eC789_SFLREGSEL_DDR, eC789_SFLCT_DMA_FLASH2REG);
        #endif

        //Enable black level
        dvC789_EdgebldBiasEnable(TRUE);


	    LOG_MSG(db_HAL_WARPING, "Load Blacklevel %d End\n", ucIndex);


        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif
    return eHAL_WARPING_EXEC_CODE_PASS;
}



eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ClearAllData(void)  //A35G2_Simon_0093
{
    //halWarping_Blacklevel_Reset();

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarping_Blacklevel_ClearCurrentData(void)  //A35G2_Simon_0093
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
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

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

void halWarping_Blacklevel_Debug(void)
{
#if 0
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
#endif /* 0 */
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

/*
    for(int i=0 ; i<ColorNumber ; i++)
    {
        LOG_MSG(db_HAL_WARPING, "C%d 0x%X\n", i, colorSet[i]);
    }
*/
    return ColorNumber;

}

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnOSD(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend)
{
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;


    LOG_MSG(db_HAL_WARPING, "Start %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());
        halWarpOSD_ClearOSD(eWDT_AFTER_WARP);  //clear Warp OSD
//    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
//    {
        LOG_MSG(db_HAL_WARPING, "using Draw PNG on OSD method, x, y = (%d, %d), width/height = (%d, %d)\n", xPos, yPos, ulWidth, ulHeight);

        int ulColorset[256] ;
        memset(ulColorset, 0xFF , sizeof(ulColorset));

        int numberOfColors = CountColors(xPos, yPos, ulWidth, ulHeight, paucBlend, ulColorset);

        LOG_MSG(db_HAL_WARPING, "PNG total color %d\n",  numberOfColors);

        //force do original color first for test
        unsigned char *pixelIndexed = (unsigned char *)malloc(3840 * 2400 + 100);
        if(pixelIndexed == NULL)
        {
            ASSERT_ALWAYS();
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(pixelIndexed, 0xFF, 3840 * 2400 + 100);
        UINT32 ulPalette256[256];  //vector < unsigned int >vPalette256;
        UINT8 ucColor884 = 0;  //A70LK_CL_0006
        memset(ulPalette256, 0xFF, sizeof(ulPalette256));

        if(numberOfColors > 255)
        {
            ColorUniformQuantization884(ulWidth, ulHeight, paucBlend, pixelIndexed, ulPalette256);
            ucColor884 = 1;  //A70LK_CL_0006
        }
        else
        {
            CalculatePaletteAndTransformRGB888(ulWidth, ulHeight, paucBlend, pixelIndexed, ulPalette256, ulColorset, numberOfColors);
        }

        halWarping_OSDWritePaletteWithNumber(ulPalette256, 255);  //ignore index 255 of transparent color
        LOG_MSG(db_HAL_WARPING, "===>end count color and write palette (%d)\n", TMO_GetSysRunTime());

        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();

        int tmpWidth =  (xPos + ulWidth) > m_nHorzResolution ? m_nHorzResolution : ulWidth;
        int tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight;

        LOG_MSG(db_HAL_WARPING, "start x,y = (%d, %d), H/V = (%d, %d), Panel H/V = (%d, %d), paint H/V = (%d, %d)\n"
        , xPos, yPos, ulWidth, ulHeight, m_nHorzResolution, m_nVertResolution, tmpWidth, tmpHeight);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT32 ulSpriteOsdAddress;
        //A70LK_CL_0006
        UINT8 ucRowNumber = MEMORYBUFMAXBYTE / m_nHorzResolution;
        UINT8 ucTmpData[m_nHorzResolution*ucRowNumber];

        ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_1);

#ifdef FPGA_ENTRY_4K
        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_1);
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE_1);
#endif /* FPGA_ENTRY_4K */

        halWarping_IsDraw_OSD_Set(eWDT_AFTER_WARP, TRUE);  //H2PF_Simon_0066

        INT32 lIndex = 0 ;
        //A70LK_CL_0006
        INT16 iCount = 0;
        UINT8 ucRow = 0;
        memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write

        for (int y = yPos; y < tmpHeight; y++)
        {
            ucRow = y%ucRowNumber;  //A70LK_CL_0006
            for (int x = xPos; x < tmpWidth; x++)
            {

                if(x >= 0 && y >= 0)
                {
                    if((ucColor884 == 1) && (pixelIndexed[lIndex] == 255))  //A70LK_CL_0006
                    {
                        ucTmpData[x + tmpWidth * ucRow] = 254;      //work around to avoid transparent color
                    }
                    else
                    {
                        ucTmpData[x + tmpWidth * ucRow] = pixelIndexed[lIndex];
                    }
                }
/*
                if(y == yPos+1)
                {
                    printf("x,y = (%d, %d)  ColorIdx : %d, value = %d \n", x, y, lIndex, pixelIndexed[lIndex]);
                }
                */
                lIndex++ ;
            }
            //A70LK_CL_0006
            if((y!=0) && (y%ucRowNumber ==ucRowNumber-1))
            {
//                printf("y = (%d) iCount : %d, uRow = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, (m_nHorzResolution / 64) * ucRowNumber, ((iCount * m_nHorzResolution / 64) *ucRowNumber), ((iCount * m_nHorzResolution / 64) *ucRowNumber));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + ((iCount * m_nHorzResolution / 64) *ucRowNumber), (m_nHorzResolution / 64) * ucRowNumber, ucTmpData);
                iCount ++;
                memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write
            }
            if((y == tmpHeight -1) && (tmpHeight/ucRowNumber * iCount != tmpHeight) )
            {
//                printf("y = (%d) iCount : %d, uRow = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, (m_nHorzResolution / 64) * (ucRow+1), ((iCount * m_nHorzResolution / 64)*ucRowNumber), ((iCount * m_nHorzResolution / 64)*ucRowNumber));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + ((iCount * m_nHorzResolution / 64)*ucRowNumber), (m_nHorzResolution / ucRowNumber) * (ucRow+1), ucTmpData);
                memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write
            }

        }

        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

        free(pixelIndexed);
        halWarping_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "End %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void ShiftLeftArrayBit(UINT8 *Array, UINT32 ArraySize, UINT32 ShiftLeftNum) //H2PF_Simon_0057
{
    UINT8 carry = 0;  // 用於儲存進位位元

    while(ShiftLeftNum--)  //要 shift left 的次數
    {
        for (int i = 0 ; i < ArraySize ; i++)
        {
            UINT8 next_carry = (Array[i] & 0x80) ? 1 : 0;  // 取出最高位作為下一個進位
            Array[i] = (Array[i] << 1) | carry;  // 左移並加上進位
            carry = next_carry;  // 更新進位
        }
    }
}

void bit_memcpy(unsigned char *dest, int dest_bit_offset, const unsigned char *src, int src_bit_offset, int num_bits)   //H2PF_Simon_0057
{
    // 目標與來源的位元位置及位元掩碼
    int dest_byte_offset = dest_bit_offset / 8;
    int dest_bit_pos = dest_bit_offset % 8;
    int src_byte_offset = src_bit_offset / 8;
    int src_bit_pos = src_bit_offset % 8;

    while (num_bits > 0)
    {
        // 計算本次迴圈要複製的位元數
        int bits_to_copy = 8 - dest_bit_pos;
        if (bits_to_copy > 8 - src_bit_pos) bits_to_copy = 8 - src_bit_pos;
        if (bits_to_copy > num_bits) bits_to_copy = num_bits;

        // 準備源和目的地的位元掩碼
        unsigned char src_mask = ((1 << bits_to_copy) - 1) << src_bit_pos;
        unsigned char dest_mask = ((1 << bits_to_copy) - 1) << dest_bit_pos;

        // 提取源資料，移位並清理目標位置，再將資料置入目標
        unsigned char src_bits = (src[src_byte_offset] & src_mask) >> src_bit_pos;
        dest[dest_byte_offset] = (dest[dest_byte_offset] & ~dest_mask) | (src_bits << dest_bit_pos);

        // 更新位元位置及剩餘位元數
        num_bits -= bits_to_copy;
        dest_bit_pos += bits_to_copy;
        src_bit_pos += bits_to_copy;

        if (dest_bit_pos == 8)
        {
            dest_bit_pos = 0;
            dest_byte_offset++;
        }

        if (src_bit_pos == 8)
        {
            src_bit_pos = 0;
            src_byte_offset++;
        }
    }
}


/////////////////////////////
// 測試bitcpy函數
/*
int main()
{
    unsigned char src[] = {0b10101010, 0b11001100}; // 源資料
    unsigned char dest[2] = {0}; // 目標資料

    // 從src的第1位開始複製6位到dest的第3位
    bitcpy(dest, 3, src, 1, 6);

    // 打印結果
    printf("結果: %02x %02x\n", dest[0], dest[1]); // 應該輸出: a8 00

    return 0;
}
*/
/////////////////////////////

UINT8 W_FB_START = 0;

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnFrameBuffer(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend) //H2PF_Simon_0057
{
    UINT32 StartTime = TMO_GetSysRunTime();
    LOG_MSG(db_HAL_WARPING, "Start %s (%d) (%d %d %d %d)\n", __FUNCTION__ , TMO_GetSysRunTime(), xPos, yPos, ulWidth, ulHeight);
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);  //H2PF_Simon_0038
    //halGui_OSD_Off();  //Disable OSD
    m_sHalWarpingInfo.ucDrawPngOnBgEnable = 1;

    //*paucBlend = (INT8 *)malloc(3840*2400*sizeof(INT32));
    //memset(paucBlend, 0xFF, (3840*2400*sizeof(INT32)));

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvProAV_AccessWrite(eIpmBaseAdr, 0);
        dvProAV_AccessWrite(eOpmBaseAdr, 0);
        dvProAV_AccessWrite(eIpmBaseAdr2, 0);
        dvProAV_AccessWrite(eOpmBaseAdr2, 0);
        dvProAV_AccessWrite(eIpmBaseAdr3, 0);
        dvProAV_AccessWrite(eOpmBaseAdr3, 0);
        dvProAV_AccessWrite(eIpmBaseAdr4, 0);
        dvProAV_AccessWrite(eOpmBaseAdr4, 0);
        dvProAV_AccessWrite(eIpmBaseAdr5, 0);
        dvProAV_AccessWrite(eOpmBaseAdr5, 0);
        dvProAV_AccessWrite(eIpmBaseAdr6, 0);
        dvProAV_AccessWrite(eOpmBaseAdr6, 0);

        dvProAV_SclInputFreezeSet(eSclSrcIn_MainSub, TRUE);
        dvProAV_SclVopScreenOffSet(false);
        dvPro_RGBLSMdSet(TRUE);
        //dvProAV_OSD_Enable(false);  //H2PF_Simon_0038

        LOG_MSG(db_HAL_WARPING, "Draw PNG on Frame Buffer, width/height = (%d, %d)\n", ulWidth, ulHeight);

        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();

        UINT32 ulBgBaseAddress = DRAM_WND1_MEM_ADDR;

        int tmpWidth =  (xPos + ulWidth) > m_nHorzResolution ? m_nHorzResolution : ulWidth;
        int tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight;

        LOG_MSG(db_HAL_WARPING, "start x,y = (%d, %d), H/V = (%d, %d), Panel H/V = (%d, %d), paint H/V = (%d, %d)\n"
        , xPos, yPos, ulWidth, ulHeight, m_nHorzResolution, m_nVertResolution, tmpWidth, tmpHeight);

        //每個 pixel 為 30bit
        //一次寫入單位為 16 個 pixel
        //共 30x16=480bits 資料，但DRAM一次要寫512bits
        //所以第 481 ~ 512 的 bit 要補上 0

        //line buf 一次可寫 MEMORYBUFMAXBYTE (32KB) 的資料
        //ucRowNumber ：用 line buf, 一次可以寫幾 row ?
        //ucTmpData : line buf 一次會寫的 data
        UINT8 ucRowNumber = MEMORYBUFMAXBYTE / m_nHorzResolution/ 4;  //32768 / 3840 / 4 = 2    //Line Buf 一次可以寫的 row 數量   //其中4  為一個 pixel是 4 個byte
        UINT8 *ucTmpData = (unsigned char *)malloc((m_nHorzResolution * ucRowNumber)*4); //malloc 3840*4*2 = 30720   //Line Buf 一次要寫入的 data
        UINT8 *uc16PixelData = (unsigned char *)malloc(64);
        memset(ucTmpData, 0, ((m_nHorzResolution * ucRowNumber)*4));
        memset(uc16PixelData, 0, 64);

        if(ucTmpData == NULL)
        {
            ASSERT_ALWAYS();
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(ucTmpData, 0xff, sizeof(ucTmpData));

        INT16 iCount = 0;
        UINT8 ucRow = 0;
        UINT8 PixelIdx = 0;
        UINT16 PixelGroupIdx = 0;

        //W_FB_START = 1;
        UINT8 OneDramData[64] = {0};

        for (int y = yPos; y < tmpHeight; y++)
        {
            ucRow = y%ucRowNumber;

            for (int x = xPos; x < tmpWidth; x++)
            {
                uint32 nXPos = x - xPos;
                uint32 nYPos = y - yPos;
                uint8 uiRedColor   = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 2];
                uint8 uiGreenColor = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 1];
                uint8 uiBlueColor  = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 0];

                UINT32 uiColor = (((UINT32)uiBlueColor << 2 ) << 20) |
                                 (((UINT32)uiGreenColor<< 2 ) << 10) |
                                 ( (UINT32)uiRedColor  << 2 );

                uiColor = uiColor & 0x3FFFFFFF;  //remove MSB 2 bits

                UINT8 Pixel[4] = {0};
                Pixel[0] = (uiColor & 0x000000ff);
                Pixel[1] = (uiColor & 0x0000ff00) >> 8;
                Pixel[2] = (uiColor & 0x00ff0000) >> 16;
                Pixel[3] = (uiColor & 0xff000000) >> 24;
                PixelIdx++;

                bit_memcpy(OneDramData, 30*(PixelIdx-1), Pixel, 0, 30);

                //ucTmpData 共存放 480組 pixel 的資料，每組有16個pixel，每個 pixel 有 4 bytes
                //DRAW 一次要寫 512bit ，一次寫 16pixel，481~512 bit 補0
                if(PixelIdx >= 16)
                {
                    memcpy(ucTmpData+((PixelGroupIdx)*64), OneDramData, 64);
                    PixelGroupIdx++;
                    PixelIdx = 0;
                }
            }

            if((y!=0) && (y%ucRowNumber ==ucRowNumber-1))
            {
                //printf("1 y%d - %d\r\n", y, y%ucRowNumber);
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulBgBaseAddress + ((iCount * m_nHorzResolution / 64) *ucRowNumber*4), (m_nHorzResolution / 64) * ucRowNumber*4, ucTmpData);
                PixelGroupIdx = 0;
                iCount ++;
            }
            if((y == tmpHeight -1) && (tmpHeight/ucRowNumber * iCount != tmpHeight) )
            {
                //printf("2 y%d - %d > %d\r\n", y, tmpHeight/ucRowNumber * iCount , iCount);
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulBgBaseAddress + ((iCount * m_nHorzResolution / 64)*ucRowNumber*4), (m_nHorzResolution / ucRowNumber) * (ucRow+1)*4, ucTmpData);
                PixelGroupIdx = 0;
            }

        }
        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

//        printf("Enable BG!!!\n");
        //dvProAV_SclVopScreenOffSet(true);
        //dvProAV_SclVopScreenOffSet(false);
        //dvProAV_SclLogoDisplayEn(true);

        free(ucTmpData);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        free(paucBlend);
        paucBlend = NULL;
        //W_FB_START = 0;
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "End %s Time(%d ms)\n", __FUNCTION__ , TMO_GetPeriod(StartTime));

    free(paucBlend);
    paucBlend = NULL;

    //W_FB_START = 0;

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnBackground(UINT8 ucPatternIndex, INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend) //H2PF_Simon_0057
{
    UINT32 StartTime = TMO_GetSysRunTime();
    LOG_MSG(db_HAL_WARPING, "Start %s (%d)\n", __FUNCTION__ , StartTime);
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    UINT32 ulBgBaseAddress;

    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);  //H2PF_Simon_0038
    dvProAV_SclVopScreenOffSet(true);  //disable display source
    dvProAV_SclLogoDisplayEn(true);    //enable display background

    m_sHalWarpingInfo.ucDrawPngOnBgEnable = 1;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        LOG_MSG(db_HAL_WARPING, "Start Draw PNG on Background - width/height = (%d, %d)\n", ulWidth, ulHeight);

        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();

        if(halWarping_PanelGet() == PANEL_2D_OUTPUT)
        {
            if(0 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY0_2D_ADDR;
            }
            else if(1 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY1_2D_ADDR;
            }
            else
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY2_2D_ADDR;
            }
        }
        else
        {
            if(0 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY0_3D_ADDR;
            }
            else if(1 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY1_3D_ADDR;
            }
            else
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY2_3D_ADDR;
            }
        }

        dvProAV_AccessWrite(eLogoLogoBasAdr, (ulBgBaseAddress / 2));

        int tmpWidth =  (xPos + ulWidth) > m_nHorzResolution ? m_nHorzResolution : ulWidth;
        int tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight;

        LOG_MSG(db_HAL_WARPING, "start x,y = (%d, %d), H/V = (%d, %d), Panel H/V = (%d, %d), paint H/V = (%d, %d)\n"
        , xPos, yPos, ulWidth, ulHeight, m_nHorzResolution, m_nVertResolution, tmpWidth, tmpHeight);

        UINT8 ucRowNumber = MEMORYBUFMAXBYTE / m_nHorzResolution / 2;  //32768 / 3840 / 2 = 4
        UINT8 *ucTmpData = (unsigned char *)malloc((m_nHorzResolution * ucRowNumber)*2); //malloc 3840*4*2 = 30720

        if(ucTmpData == NULL)
        {
            ASSERT_ALWAYS();
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }

        INT16 iCount = 0;
        UINT8 ucRow = 0;

        for (int y = yPos; y < tmpHeight; y++)
        {
            ucRow = y%ucRowNumber;
            for (int x = xPos; x < tmpWidth; x++)
            {
                uint32 nXPos = x - xPos;
                uint32 nYPos = y - yPos;
                uint8 uiRedColor   = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 0];
                uint8 uiGreenColor = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 1];
                uint8 uiBlueColor  = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 2];

                uint16 uiColor = ((uiRedColor & 0b11111000) << 8) |
                                 ((uiGreenColor & 0b11111100) << 3) |
                                 (uiBlueColor >> 3);

                ucTmpData[tmpWidth * ucRow*2 + x*2 + 0] = (uint8)(uiColor & 0x00ff);
                ucTmpData[tmpWidth * ucRow*2 + x*2 + 1] = (uint8)((uiColor & 0xff00) >> 8);
            }
            if((y!=0) && (y%ucRowNumber ==ucRowNumber-1))
            {
                //printf("y = (%d) iCount : %d, uRow = %d, shift = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, uiShift, (m_nHorzResolution / 64) * ucRowNumber*2, ((iCount * m_nHorzResolution / 64) *ucRowNumber*2), ((iCount * m_nHorzResolution / 64) *ucRowNumber*2));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulBgBaseAddress + ((iCount * m_nHorzResolution / 64) *ucRowNumber*2), (m_nHorzResolution / 64) * ucRowNumber*2, ucTmpData);
                iCount ++;
            }
            if((y == tmpHeight -1) && (tmpHeight/ucRowNumber * iCount != tmpHeight) )
            {
//                printf("Base address of LOGO_DISPLAY = 0x%08x\n",ulBgBaseAddress);
//                printf("y = (%d) iCount : %d, uRow = %d, shift = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, uiShift, (m_nHorzResolution / 64) * (ucRow+1)*2, ((iCount * m_nHorzResolution / 64)*ucRowNumber*2), ((iCount * m_nHorzResolution / 64)*ucRowNumber*2));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulBgBaseAddress + ((iCount * m_nHorzResolution / 64)*ucRowNumber*2), (m_nHorzResolution / ucRowNumber) * (ucRow+1)*2, ucTmpData);
            }

        }
        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }


        free(ucTmpData);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "End %s (%d ms)\n", __FUNCTION__ , TMO_GetPeriod(StartTime));

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_ChangeIndex(UINT8 ucPatternIndex)
{
    UINT32 StartTime = TMO_GetSysRunTime();
    LOG_MSG(db_HAL_WARPING, "Start %s (%d)\n", __FUNCTION__ , StartTime);
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    UINT32 ulBgBaseAddress;

    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);  //H2PF_Simon_0038
    dvProAV_SclVopScreenOffSet(true);  //disable display source
    dvProAV_SclLogoDisplayEn(true);    //enable display background

    m_sHalWarpingInfo.ucDrawPngOnBgEnable = 1;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();

        if(halWarping_PanelGet() == PANEL_2D_OUTPUT)
        {
            if(0 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY0_2D_ADDR;
            }
            else if(1 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY1_2D_ADDR;
            }
            else
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY2_2D_ADDR;
            }
        }
        else
        {
            if(0 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY0_3D_ADDR;
            }
            else if(1 == ucPatternIndex)
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY1_3D_ADDR;
            }
            else
            {
                ulBgBaseAddress = DRAM_LOGO_DISPLAY2_3D_ADDR;
            }
        }

        dvProAV_AccessWrite(eLogoLogoBasAdr, (ulBgBaseAddress / 2));

        halWarping_SemaphoreGive(__FUNCTION__);
    }


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_Off(void) //H2PF_Simon_0057
{
    dvProAV_SclVopScreenOffSet(false);  //disable display source
    dvProAV_SclLogoDisplayEn(false);    //enable display background

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnMemoryTest(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend)
{

    LOG_MSG(db_HAL_WARPING, "Start %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    UINT32 ulBgBaseAddress;

    halWarpOSD_ClearOSD(eWDT_AFTER_WARP);  //H2PF_Simon_0038
    dvProAV_OSD_Enable(false);  //H2PF_Simon_0038
    //halGui_OSD_Off();  //Disable OSD
    m_sHalWarpingInfo.ucDrawPngOnBgEnable = 1;

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        halWarping_IsDraw_OSD_Set(eWDT_BEFORE_WARP, TRUE);  //H2PF_Simon_0066

        LOG_MSG(db_HAL_WARPING, "using Draw PNG on Background OSD method, width/height = (%d, %d)\n", ulWidth, ulHeight);

        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();
        if(m_nHorzResolution>1920 && m_nVertResolution > 1080)
        {
            ulBgBaseAddress = DRAM_LOGO_DISPLAY1_2D_ADDR;
        }
        else
        {
            ulBgBaseAddress = DRAM_LOGO_DISPLAY1_3D_ADDR;
        }

        int tmpWidth =  (xPos + ulWidth) > m_nHorzResolution ? m_nHorzResolution : ulWidth;
        int tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight;

        LOG_MSG(db_HAL_WARPING, "start x,y = (%d, %d), H/V = (%d, %d), Panel H/V = (%d, %d), paint H/V = (%d, %d)\n"
        , xPos, yPos, ulWidth, ulHeight, m_nHorzResolution, m_nVertResolution, tmpWidth, tmpHeight);

        UINT8 ucRowNumber = MEMORYBUFMAXBYTE / m_nHorzResolution/2;  //32768 / 3840 / 2 = 4
        UINT8 *ucTmpData = (unsigned char *)malloc((m_nHorzResolution * ucRowNumber)*2); //malloc 3840*4*2 = 30720

        if(ucTmpData == NULL)
        {
            ASSERT_ALWAYS();
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(ucTmpData, 0xff, sizeof(ucTmpData));

        INT16 iCount = 0;
        UINT8 ucRow = 0;

        for (int y = yPos; y < tmpHeight; y++)
        {
            ucRow = y%ucRowNumber;
            for (int x = xPos; x < tmpWidth; x++)
            {
                uint32 nXPos = x - xPos;
                uint32 nYPos = y - yPos;
                uint8 uiRedColor   = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 0];
                uint8 uiGreenColor = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 1];
                uint8 uiBlueColor  = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 2];

                uint16 uiColor = ((uiRedColor & 0b11111000) <<8) |((uiGreenColor & 0b11111100) <<3) | (uiBlueColor >>3);
                ucTmpData[tmpWidth * ucRow*2 + x*2 + 0] = (uint8)(uiColor & 0x00ff);
                ucTmpData[tmpWidth * ucRow*2 + x*2 + 1] = (uint8)((uiColor & 0xff00) >> 8);
            }
            if((y!=0) && (y%ucRowNumber ==ucRowNumber-1))
            {
                //printf("y = (%d) iCount : %d, uRow = %d, shift = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, uiShift, (m_nHorzResolution / 64) * ucRowNumber*2, ((iCount * m_nHorzResolution / 64) *ucRowNumber*2), ((iCount * m_nHorzResolution / 64) *ucRowNumber*2));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulBgBaseAddress + ((iCount * m_nHorzResolution / 64) *ucRowNumber*2), (m_nHorzResolution / 64) * ucRowNumber*2, ucTmpData);
                iCount ++;
                memset(ucTmpData, 0xFF, sizeof(ucTmpData));
            }
            if((y == tmpHeight -1) && (tmpHeight/ucRowNumber * iCount != tmpHeight) )
            {
//                printf("Base address of LOGO_DISPLAY = 0x%08x\n",ulBgBaseAddress);
//                printf("y = (%d) iCount : %d, uRow = %d, shift = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, uiShift, (m_nHorzResolution / 64) * (ucRow+1)*2, ((iCount * m_nHorzResolution / 64)*ucRowNumber*2), ((iCount * m_nHorzResolution / 64)*ucRowNumber*2));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulBgBaseAddress + ((iCount * m_nHorzResolution / 64)*ucRowNumber*2), (m_nHorzResolution / ucRowNumber) * (ucRow+1)*2, ucTmpData);
                memset(ucTmpData, 0xFF, sizeof(ucTmpData));
            }

        }
        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

//        printf("Enable BG!!!\n");
        dvProAV_SclVopScreenOffSet(true);
        dvProAV_SclLogoDisplayEn(true);

        free(ucTmpData);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "End %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());

    return eHAL_WARPING_EXEC_CODE_PASS;
}



eHAL_WARPING_EXEC_CODE halWarping_DrawPNG_OnMemory(INT32 xPos, INT32 yPos, INT32 ulWidth, INT32 ulHeight, UINT8 *paucBlend)
{
    LOG_MSG(db_HAL_WARPING, "Start %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    if(halWarping_GetFreezeImageState() == FALSE)
    {
        halWarping_FreezeImage(TRUE);
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

#if 1 //A70LK_CL_0004 use draw png on osd method
        LOG_MSG(db_HAL_WARPING, "using Draw PNG on OSD method, width/height = (%d, %d)\n", ulWidth, ulHeight);


        UINT8 *paucResize = (unsigned char *)malloc((3840 * 2400)*3 + 1024);
        if(paucResize == NULL)
        {
            ASSERT_ALWAYS();
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(paucResize, 0xff, (3840 * 2400)*3 + 1024);
        if(ulWidth > 1920 || ulHeight > 1080)
        {
            halWarping_resizePixels(paucResize, paucBlend,ulWidth,ulHeight,1920,1080);
        }
        else
        {
            memcpy(paucResize, paucBlend, sizeof(UINT8)*((3840 * 2400)*3 + 1024));
        }

        int ulColorset[256] ;
        UINT8 ucColor884 = 0;  //A70LK_CL_0006
        memset(ulColorset, 0xFF , sizeof(ulColorset));
//        int numberOfColors = CountColors(xPos, yPos, ulWidth, ulHeight, paucBlend, ulColorset);
        int numberOfColors = CountColors(xPos, yPos, ulWidth, ulHeight, paucResize, ulColorset);
        LOG_MSG(db_HAL_WARPING, "PNG total color %d\n",  numberOfColors);
        //force do original color first for test
        unsigned char *pixelIndexed = (unsigned char *)malloc(3840 * 2400 + 100);
        if(pixelIndexed == NULL)
        {
            ASSERT_ALWAYS();
            return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
        }
        memset(pixelIndexed, 0xFF, 3840 * 2400 + 100);
        UINT32 ulPalette256[256];  //vector < unsigned int >vPalette256;
        memset(ulPalette256, 0xFF, sizeof(ulPalette256));

        if(numberOfColors > 255)
        {
//            ColorUniformQuantization884(ulWidth, ulHeight, paucBlend, pixelIndexed, ulPalette256);
            ColorUniformQuantization884(ulWidth, ulHeight, paucResize, pixelIndexed, ulPalette256);
            ucColor884 = 1;  //A70LK_CL_0006
        }
        else
        {
//            CalculatePaletteAndTransformRGB888(ulWidth, ulHeight, paucBlend, pixelIndexed, ulPalette256, ulColorset, numberOfColors);
            CalculatePaletteAndTransformRGB888(ulWidth, ulHeight, paucResize, pixelIndexed, ulPalette256, ulColorset, numberOfColors);
        }

        dvProAV_OSD_Enable(false);  //A70LK_CL_0007

        halWarping_OSDWritePaletteWithNumberScalerOsd(ulPalette256, 256);  //ignore index 255 of transparent color
        LOG_MSG(db_HAL_WARPING, "===>end count color and write palette (%d)\n", TMO_GetSysRunTime());

        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();

        if(halWarping_HResGet()>1920 && halWarping_VResGet() > 1080)
        {
            m_nHorzResolution = m_nHorzResolution/2;
            m_nVertResolution = m_nVertResolution/2;
        }

        int tmpWidth =  (xPos + ulWidth) > m_nHorzResolution ? m_nHorzResolution : ulWidth;
        int tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight;

        LOG_MSG(db_HAL_WARPING, "start x,y = (%d, %d), H/V = (%d, %d), Panel H/V = (%d, %d), paint H/V = (%d, %d)\n"
        , xPos, yPos, ulWidth, ulHeight, m_nHorzResolution, m_nVertResolution, tmpWidth, tmpHeight);

        UINT32 ulSpriteOsdAddress;
        //A70LK_CL_0006
        UINT8 ucRowNumber = MEMORYBUFMAXBYTE / m_nHorzResolution;
        UINT8 ucTmpData[m_nHorzResolution*ucRowNumber];

        ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_0);

#ifdef FPGA_ENTRY_4K
        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_0);
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE_0);
#endif /* FPGA_ENTRY_4K */

        INT32 lIndex = 0 ;
        //A70LK_CL_0006
        INT16 iCount = 0;
        UINT8 ucRow = 0;
        memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write

        for (int y = yPos; y < tmpHeight; y++)
        {
            ucRow = y%ucRowNumber;  //A70LK_CL_0006
            for (int x = xPos; x < tmpWidth; x++)
            {

                if(x >= 0 && y >= 0)
                {
                    if((ucColor884 == 1) && (pixelIndexed[lIndex] == 255))  //A70LK_CL_0006
                    {
                        ucTmpData[x + tmpWidth * ucRow] = 254;      //work around to avoid transparent color
                    }
                    else
                    {
                        ucTmpData[x + tmpWidth * ucRow] = pixelIndexed[lIndex];
                    }
                }
/*
                if(y == yPos+1)
                {
                    printf("x,y = (%d, %d)  ColorIdx : %d, value = %d \n", x, y, lIndex, pixelIndexed[lIndex]);
                }
                */
                lIndex++ ;
            }
            //A70LK_CL_0006
            if((y!=0) && (y%ucRowNumber ==ucRowNumber-1))
            {
//                printf("y = (%d) iCount : %d, uRow = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, (m_nHorzResolution / 64) * ucRowNumber, ((iCount * m_nHorzResolution / 64) *ucRowNumber), ((iCount * m_nHorzResolution / 64) *ucRowNumber));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + ((iCount * m_nHorzResolution / 64) *ucRowNumber), (m_nHorzResolution / 64) * ucRowNumber, ucTmpData);
                iCount ++;
                memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write
            }
            if((y == tmpHeight -1) && (tmpHeight/ucRowNumber * iCount != tmpHeight) )
            {
//                printf("y = (%d) iCount : %d, uRow = %d, size = %d, address offset = 0x%08x (%d)\n", y, iCount, ucRow, (m_nHorzResolution / 64) * (ucRow+1), ((iCount * m_nHorzResolution / 64)*ucRowNumber), ((iCount * m_nHorzResolution / 64)*ucRowNumber));
                eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + ((iCount * m_nHorzResolution / 64)*ucRowNumber), (m_nHorzResolution / ucRowNumber) * (ucRow+1), ucTmpData);
                memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write
            }
        }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
        free(paucResize);
        free(pixelIndexed);
        halWarping_SemaphoreGive(__FUNCTION__);

#else  //original method

        LOG_MSG(db_HAL_WARPING, "Original Draw PNG on memory method\n");

        INT32 m_nHorzResolution = halWarping_HResGet()/2;
        INT32 m_nVertResolution = halWarping_VResGet()/2;

        int tmpWidth =  (xPos + ulWidth) > m_nHorzResolution ? m_nHorzResolution : ulWidth;
        int tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight;

//        LOG_MSG(db_HAL_WARPING, "start x,y = (%d, %d), H/V = (%d, %d), Panel H/V = (%d, %d), paint H/V = (%d, %d)\n"
//        , xPos, yPos, ulWidth, ulHeight, m_nHorzResolution, m_nVertResolution, tmpWidth, tmpHeight);

        UINT32 ulSpriteOsdAddress;

        UINT8 ucTmpData[m_nHorzResolution];
        memset(ucTmpData, 0, m_nHorzResolution);

    UINT8 *paucData = (UINT8 *)malloc(halWarping_HResGet() * halWarping_VResGet());  //A70LK_Simon_0001
    if(paucData == NULL)
    {
        ASSERT_ALWAYS();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }
    memset(paucData, 0xFF, (halWarping_HResGet() * halWarping_VResGet()));  //A70LK_Simon_0001

        ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_0);

#ifdef FPGA_ENTRY_4K
        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_0);
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE_0);
#endif /* FPGA_ENTRY_4K */

        for (int y = yPos; y < tmpHeight; y++)
        {
            memset(ucTmpData, 0xFF, sizeof(ucTmpData)); // only transparent color in line buffer write
            for (int x = xPos; x < tmpWidth; x++)
            {
                uint32 nXPos = x - xPos;
                uint32 nYPos = y - yPos;
                uint32 uiRedColor   = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 0];
                uint32 uiGreenColor = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 1];
                uint32 uiBlueColor  = paucBlend[ulWidth * 3 * nYPos + 3 * nXPos + 2];

                uint32 uiColor = uiBlueColor * 65536 + uiGreenColor * 256 + uiRedColor;
                UINT16 uiColorIndex = halGui_RGBColorToIndex(uiColor);
                uiColorIndex = (uiColorIndex>255) ? 249 : uiColorIndex;  //use YELLO color(249) if no matched index was found in scaler osd palette

                paucData[nYPos * m_nHorzResolution + nXPos] = (uint8)uiColorIndex;
                ucTmpData[x] = paucData[nYPos * m_nHorzResolution + nXPos];
//                printf("uiColor : %X ColorIdx : %d \n", uiColor, paucData[nYPos * m_nHorzResolution + nXPos]);
            }

            eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + y * m_nHorzResolution / 64, m_nHorzResolution / 64, ucTmpData);
        }

    if(eDvResult == rcSUCCESS)
    {
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
        free(paucData);
        halWarping_SemaphoreGive(__FUNCTION__);
#endif

    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "End %s (%d)\n", __FUNCTION__ , TMO_GetSysRunTime());

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void DrawPNGOnOSDOriginalColor(int xPos, int yPos, int nWidth, int nHeight, unsigned char *paucBlend, int *colorSet, int colorset_size)
{
    LOG_MSG(db_HAL_WARPING, "%s Start\n", __FUNCTION__);

    // 1.
    // Reduce color and calcuate palette
    unsigned char *pixelIndexed = (unsigned char *)malloc(3840 * 2400 + 100);
    if(pixelIndexed == NULL)
    {
        ASSERT_ALWAYS();
        return ;
    }
    memset(pixelIndexed, 0, 3840 * 2400 + 100);
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
    halWarping_OSDTransparentColorSet(0xFF);  //G100_Simon_0078
    //halWarping_OSDWritePalette(ulPalette256);  //H2 wait review
    //dvC789_Buffer_Flush(); //A35G2_CDS_Simon_0044
    halWarping_DrawSpriteBuffer(xPos, yPos, nWidth, nHeight, pixelIndexed);
    //dvC789_Buffer_Flush(); //A35G2_CDS_Simon_0044
    //dvC789_SetOSDMode(FALSE);  //G100_Simon_0088 //A35G2_CDS_Simon_0044

#ifdef PNG_OSD_NO_TRANSPARENT_COLOR
    //Disable transparent color
    halWarping_OSD_TransparentColor_Set(FALSE, 0);  //A35G2_Simon_0067
#endif

    free(pixelIndexed);
}

void CalculatePaletteAndTransformRGB888(int nWidth, int nHeight, unsigned char *paucBlend, unsigned char *pixelIndexed,
    unsigned int *ulPalette256, int *colorSet, int colorset_size)

{
    LOG_MSG(db_HAL_WARPING, "%s Start\n", __FUNCTION__);

    // 1. Get colorSet to map and vector
    unsigned int transparency = 0;
    ulPalette256[255] = transparency;

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
    unsigned char *pixelIndexed = (unsigned char *)malloc(3840 * 2400 + 100);  //A35G2_Simon_0067
    if(pixelIndexed == NULL)
    {
        ASSERT_ALWAYS();
        return ;
    }
    memset(pixelIndexed, 0, 3840 * 2400 + 100);
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
    halWarping_OSDTransparentColorSet(0xFF);  //G100_Simon_0078
    //halWarping_OSDWritePalette(ulPalette256);  //H2 wait review
    //dvC789_Buffer_Flush();
    halWarping_DrawSpriteBuffer(xPos, yPos, nWidth, nHeight, pixelIndexed);
    //dvC789_Buffer_Flush(); //A35G2_CDS_Simon_0044
    //dvC789_SetOSDMode(FALSE);  //G100_Simon_0088 //A35G2_CDS_Simon_0044

#ifdef PNG_OSD_NO_TRANSPARENT_COLOR
    //Disable transparent color
    halWarping_OSD_TransparentColor_Set(FALSE, 0);    //A35G2_Simon_0067
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

//    ulPalette256[255] = 0x0;    // Preserve to transparency  //A70LK_CL_0006

}


void CalculatePaletteAndTransformRGB5551(sPixmapData *pasPixmap,
                                                        UINT16 uiPixmapNumber,
                                                        unsigned int *ulmapPalette255,
                                                        unsigned int *ulPalette256)  //vPalette256
{
#if 1
	LOG_MSG(db_HAL_WARPING, "Start CalculatePaletteAndTransformRGB5551\n");

    // Init first transparency
    unsigned short transparency = 0;

    //UINT16 ulMapPalette256_Count = 0;
    //UINT16 ulPalette256_Count = 0;

    ulPalette256[255] = transparency;

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
                    //printf("transparency...(pos = %d) usColor =%x\n", nPos, usColor);
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
                        //printf("find (pos = %d) usColor=%x (index = %d)\n", nPos, usColor , findindex + 1);
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
#endif /* 0 */

}

#if 0  //H2 wait review
void halWarping_OSDWritePalette(UINT32 *ulPalette256)
{
#if 0
    // vPalette256 is unsigned short color, format is RGBA5551
    unsigned int red = 0, green = 0, blue = 0;

    // printf("Enter halWarping_OSDWritePalette\n");
    //printf("Palette size is %d \n", vPalette256.size());

    dvC789_WriteToBuffer(B1_PLTAD, 0x00);
    for (int i = 0; i < 256; i++)
    {
        //printf("vPalette256.at(i) is 0x%X ", vPalette256.at(i));
        // printf("in halWarping_OSDWritePalette\n");
        red   = ulPalette256[i] & 0xFF0000;
        red   = red >> 16;
        green = ulPalette256[i] & 0xFF00;
        green = green >> 8;
        blue  = ulPalette256[i] & 0x00FF;

        //LOG_MSG(db_HAL_WARPING, "%d %d %d %d\n", i , red, green, blue);

        //dvC789_WriteToBuffer(B1_PLTDT, red);
        //dvC789_WriteToBuffer(B1_PLTDT, green);
        //dvC789_WriteToBuffer(B1_PLTDT, blue);

        halWarping_PaletteUpdate((UINT16)i, red, green, blue);
    }

    //dvC789_WriteToBuffer(B1_BOTRANS0, 0);
    //dvC789_WriteToBuffer(B1_BOTRANS1, 0);
    //dvC789_WriteToBuffer(B1_BOTRANS2, 0);
    //dvC789_WriteToBuffer(B1_BOTRANS3, 0);  //A35G2_CDS_Simon_0057

    //dvC789_WriteToBuffer(B1_PLTAD, 0x00);  //dummy

    //dvC789_Buffer_Flush();
#endif /* 0 */

}
#endif

void halWarping_OSDWritePaletteWithNumber(UINT32 *ulPalette256, UINT16 uiNumber)
{
    // vPalette256 is unsigned short color, format is RGBA5551
    unsigned int red = 0, green = 0, blue = 0;

    // printf("Enter halWarping_OSDWritePalette\n");
    //printf("Palette size is %d \n", vPalette256.size());

    for (int i = 0; i < uiNumber; i++)
    {
        //printf("vPalette256.at(i) is 0x%X ", vPalette256.at(i));
        // printf("in halWarping_OSDWritePalette\n");
        red   = ulPalette256[i] & 0xFF0000;
        red   = red >> 16;
        green = ulPalette256[i] & 0xFF00;
        green = green >> 8;
        blue  = ulPalette256[i] & 0x00FF;

        //LOG_MSG(db_HAL_WARPING, "%d %d %d %d\n", i , red, green, blue);

        halWarpOSD_Palette_SetToRam((UINT16)i, red, green, blue);  //H2PF_Simon_0038
    }
    halWarpOSD_Palette_RamToChip();  //write to FPGA  //H2PF_Simon_0038

}

eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnMemoryModify(INT16 iTotalNumber,
                                                                   INT16 iSpritePid,
                                                                   INT16 iSpriteTop,
                                                                   INT16 iSpriteLeft,
                                                                   sPixmapData *psPixmap)
{
   // LOG_MSG(db_HAL_WARPING, "Start %s\n", __FUNCTION__);

//Draw on Scaler OSD. size limit 1920x1080
//reduce color like draw on warp OSD.
//debug CL 20221117        CalculatePaletteAndTransformRGB5551ScalerOsd(psPixmap, iTotalNumber, m_sHalWarpingInfo.aulScalerOsdMapSpritePalette, m_sHalWarpingInfo.aulScalerOsdSpritePalette);


    	// Second step: Write Palette
        //halWarping_OSDWritePaletteWithNumberScalerOsd(m_sHalWarpingInfo.aulScalerOsdSpritePalette, m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount + 1);
        //halWarping_OSDWritePalette(m_sHalWarpingInfo.aulSpritePalette);
        //halWarping_PrintWarpOsdColorPalette();// Debug to print current warp osd color palette
        // Third step: Draw Sprites

        int nPID = 0;

        for (int idx = 0; idx < iTotalNumber; idx++)
        {
            psPixmap = psPixmap + idx ;

            nPID = psPixmap->sPixmap_OnMemory.nPID;

            m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].ucLayerMode
                = eWDT_BEFORE_WARP;
            if(halWarping_HResGet()>1920 && halWarping_VResGet() > 1080)
            {
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iX
                    = iSpriteLeft/2;
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iY
                    = iSpriteTop/2;
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iWidth
                    = (psPixmap->sPixmap_OnMemory.nWidth)/2;
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iHeight
                    = (psPixmap->sPixmap_OnMemory.nHeight)/2;
            }
            else
            {
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iX
                    = iSpriteLeft;
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iY
                    = iSpriteTop;
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iWidth
                    = psPixmap->sPixmap_OnMemory.nWidth;
                m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iHeight
                    = psPixmap->sPixmap_OnMemory.nHeight;
            }
            m_sHalWarpingInfo.asScalerOsdRect[m_sHalWarpingInfo.uiScalerOsdRectCount].iColorIndex
                = (psPixmap->sPixmap_OnMemory.ucPixDataR[0] & 0x1F)
                    + ((psPixmap->sPixmap_OnMemory.ucPixDataG[0] & 0x1F) << 5)
                    + ((psPixmap->sPixmap_OnMemory.ucPixDataB[0] & 0x1F) << 10)
                    + ((psPixmap->sPixmap_OnMemory.ucPixDataR[0] & 0x80) << 8);
            m_sHalWarpingInfo.uiScalerOsdRectCount++;
        }

    return eHAL_WARPING_EXEC_CODE_PASS;
}



eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnMemory(INT16 iTotalNumber,
                                                                   INT16 iSpritePid,
                                                                   INT16 iSpriteTop,
                                                                   INT16 iSpriteLeft,
                                                                   sPixmapData *psPixmap)
{
   // LOG_MSG(db_HAL_WARPING, "Start %s\n", __FUNCTION__);

//Draw on Scaler OSD. size limit 1920x1080
//reduce color like draw on warp OSD.
#if 1
        CalculatePaletteAndTransformRGB5551ScalerOsd(psPixmap, iTotalNumber, m_sHalWarpingInfo.aulScalerOsdMapSpritePalette, m_sHalWarpingInfo.aulScalerOsdSpritePalette);


    	// Second step: Write Palette
        //halWarping_OSDWritePaletteWithNumberScalerOsd(m_sHalWarpingInfo.aulScalerOsdSpritePalette, m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount + 1);
        //halWarping_OSDWritePalette(m_sHalWarpingInfo.aulSpritePalette);
        //halWarping_PrintWarpOsdColorPalette();// Debug to print current warp osd color palette
        // Third step: Draw Sprites

        int nPID = 0;

        for (int idx = 0; idx < iTotalNumber; idx++)
        {
             //printf("PID : %d Top : %d Left : %d \n", pvSprites[idx].nPID,
             //pvSprites[idx].nTop, pvSprites[idx].nLeft);

            psPixmap = psPixmap + idx ;

            nPID = psPixmap->sPixmap_OnMemory.nPID;

            halWarping_DrawSpriteBufferScalerOsd(iSpriteLeft/2,  //the same
                                        iSpriteTop/2,   //the same
                                        (psPixmap->sPixmap_OnMemory.nWidth)/2,
                                        (psPixmap->sPixmap_OnMemory.nHeight)/2,
                                        m_sHalWarpingInfo.ucScalerOsdPixIndexed,
                                        psPixmap);

        }
#else
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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

                // dvC790_WriteBuffer(B0_CPUWAD, 0x00, 0);
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
                            dvC789_WriteToBuffer(B0_CPUWAD, cpuwad);
                        }
                        else if (x % 12 == 6)
                        {
                            dvC789_WriteToBuffer(B0_CPUDT, 0x00);
                            dvC789_WriteToBuffer(B0_CPUDT, 0x00);
                        }

                        if(idx == 0)
                            dvC789_WriteToBuffer(B0_CPUDT, psPixmap->sPixmap_OnMemory.ucPixDataR[psPixmap->sPixmap_OnMemory.nWidth * (y - iSpriteTop) + (x - iSpriteLeft)] * 8);
                        else if(idx == 1)
                            dvC789_WriteToBuffer(B0_CPUDT, psPixmap->sPixmap_OnMemory.ucPixDataG[psPixmap->sPixmap_OnMemory.nWidth * (y - iSpriteTop) + (x - iSpriteLeft)] * 8);
                        else if(idx == 2)
                            dvC789_WriteToBuffer(B0_CPUDT, psPixmap->sPixmap_OnMemory.ucPixDataB[psPixmap->sPixmap_OnMemory.nWidth * (y - iSpriteTop) + (x - iSpriteLeft)] * 8);
                    }
                }
            }


            // !!!!!!!!!!!!!!!!

            //dvC789_Buffer_Flush();    //notice : remove Flush (for speed up), so need to Flush when finishing working

            // !!!!!!!!!!!!!!!!


        }

        dvC789_Buffer_Flush();  //A35G2_CDS_Simon_0058

        halWarping_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail", __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    LOG_MSG(db_HAL_WARPING, "End %s\n", __FUNCTION__);
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;

}


eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnOSDModify(INT16 iTotalNumber,
                                                              INT16 iSpritePid,
                                                              INT16 iSpriteTop,
                                                              INT16 iSpriteLeft,
                                                              sPixmapData *psPixmap)
{
        int nPID = 0;

        for (int idx = 0; idx < iTotalNumber; idx++)
        {
            psPixmap = psPixmap + idx ;

            nPID = psPixmap->sPixmap_OnOSD.nPID;

            m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].ucLayerMode
                = eWDT_AFTER_WARP;
            m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iX
                = iSpriteLeft;
            m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iY
                = iSpriteTop;

            if(dvProAV_OSD_4k3dEn_Get() && halWarping_PanelGet() == PANEL_2D_HIGHSPEED)
            {
                if(m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iX > 0)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iX = iSpriteLeft * 2;
                }

                if(m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iY > 0)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iY = iSpriteTop * 2;
                }

                if(m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iX >= (psPixmap->sPixmap_OnOSD.nWidth/2))
                {
                    m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iX += (psPixmap->sPixmap_OnOSD.nWidth/2);
                }

                if(m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iY >= (psPixmap->sPixmap_OnOSD.nHeight/2))
                {
                    m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iY += (psPixmap->sPixmap_OnOSD.nHeight/2);
                }
            }

            m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iWidth
                = psPixmap->sPixmap_OnOSD.nWidth;
            m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iHeight
                = psPixmap->sPixmap_OnOSD.nHeight;
            m_sHalWarpingInfo.asWarpOsdRect[m_sHalWarpingInfo.uiWarpOsdRectCount].iColorIndex
                = (unsigned short)(psPixmap->sPixmap_OnOSD.ucPixData[1] << 8)
                    + (unsigned short)(psPixmap->sPixmap_OnOSD.ucPixData[0]);
            m_sHalWarpingInfo.uiWarpOsdRectCount++;

        }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesOnOSD(INT16 iTotalNumber,
                                                              INT16 iSpritePid,
                                                              INT16 iSpriteTop,
                                                              INT16 iSpriteLeft,
                                                              sPixmapData *psPixmap)
{
#if 1
//    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        CalculatePaletteAndTransformRGB5551(psPixmap, iTotalNumber, m_sHalWarpingInfo.aulMapSpritePalette, m_sHalWarpingInfo.aulSpritePalette);

    	// Second step: Write Palette
        halWarping_OSDWritePaletteWithNumber(m_sHalWarpingInfo.aulSpritePalette, m_sHalWarpingInfo.uiSpritePaletteCount + 1);
        // Third step: Draw Sprites

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

//        halWarping_SemaphoreGive(__FUNCTION__);
    }
//    else
//    {
//        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail", __FUNCTION__);
//        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
//    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarping_DrawSpritesInit(void)
{

#if 0
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //reset palette
        memset(m_sHalWarpingInfo.aulSpritePalette,  0, sizeof(m_sHalWarpingInfo.aulSpritePalette));
        memset(m_sHalWarpingInfo.aulMapSpritePalette, 0, sizeof(m_sHalWarpingInfo.aulMapSpritePalette));

        halWarping_OSDWritePalette(m_sHalWarpingInfo.aulSpritePalette);
        dvC789_Buffer_Flush();

        m_sHalWarpingInfo.uiSpritePaletteCount = 0;
        m_sHalWarpingInfo.uiMapSpritePaletteCount = 0;

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail", __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}



//G100_Simon_0007
void halWarping_OSDTransparentColorSet(UINT8 ucTransparentColorIndex)
{
    //dvC789_Write(B1_BOTRANS0, ucTransparentColorIndex);
    //dvC789_Write(B1_BOTRANS1, ucTransparentColorIndex);
    //dvC789_Write(B1_BOTRANS2, ucTransparentColorIndex);
    //dvC789_Write(B1_BOTRANS3, ucTransparentColorIndex);  //A35G2_CDS_Simon_0057
}


//G100_Simon_0007
eHAL_WARPING_EXEC_CODE halWarping_ReadPaletteToTemp(void)
{
#if 0
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvC789_Write(B1_PLTAD, 0);
        dvC789_BurstRead_FixedAdd(B1_PLTDT, 300, ucTempPalette);
        dvC789_Write(B1_PLTAD, 100);
        dvC789_BurstRead_FixedAdd(B1_PLTDT, 300, &ucTempPalette[300]);
        dvC789_Write(B1_PLTAD, 200);
        dvC789_BurstRead_FixedAdd(B1_PLTDT, 168, &ucTempPalette[600]);

        for(int i=0 ; i<256 ; i++)
        {
            LOG_MSG(db_HAL_WARPING, "%d - %d %d %d\n", i, ucTempPalette[i*3 + 0], ucTempPalette[i*3 + 1], ucTempPalette[i*3 + 2]);
        }

        ucTempTranparentColor[0] = dvC789_Read(B1_BOTRANS0);
        ucTempTranparentColor[1] = dvC789_Read(B1_BOTRANS1);
        ucTempTranparentColor[2] = dvC789_Read(B1_BOTRANS2);
        ucTempTranparentColor[3] = dvC789_Read(B1_BOTRANS3);  //A35G2_CDS_Simon_0057

        LOG_MSG(db_HAL_WARPING, "ucTempTranparentColor %d %d %d %d\n", ucTempTranparentColor[0], ucTempTranparentColor[1], ucTempTranparentColor[2], ucTempTranparentColor[3]);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//G100_Simon_0007
eHAL_WARPING_EXEC_CODE halWarping_TempPaletteWriteToChip(void)
{
#if 0
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        halWarpOSD_DisableOSD();

        dvC789_Write(B1_PLTAD, 0);
        dvC789_BurstWrite_FixedAdd(B1_PLTDT, 300, ucTempPalette);
        dvC789_Write(B1_PLTAD, 100);
        dvC789_BurstWrite_FixedAdd(B1_PLTDT, 300, &ucTempPalette[300]);
        dvC789_Write(B1_PLTAD, 200);
        dvC789_BurstWrite_FixedAdd(B1_PLTDT, 168, &ucTempPalette[600]);


        LOG_MSG(db_HAL_WARPING, "halWarping_TempPaletteWriteToChip %d\n" , ucTempTranparentColor[0]);

        for(int i=0 ; i<256 ; i++)
        {
            halWarping_PaletteUpdate((UINT16)i, ucTempPalette[i*3 + 0], ucTempPalette[i*3 + 1], ucTempPalette[i*3 + 2]);
            LOG_MSG(db_HAL_WARPING, "%d - %d %d %d\n", i, ucTempPalette[i*3 + 0], ucTempPalette[i*3 + 1], ucTempPalette[i*3 + 2]);
        }

        dvC789_Write(B1_BOTRANS0, ucTempTranparentColor[0]);
        dvC789_Write(B1_BOTRANS1, ucTempTranparentColor[1]);
        dvC789_Write(B1_BOTRANS2, ucTempTranparentColor[2]);
        dvC789_Write(B1_BOTRANS3, ucTempTranparentColor[3]);  //A35G2_CDS_Simon_0057

        MS_SLEEP(100);

        halWarpOSD_ClearOSD_NoSemaphore();
        dvC789_SetOSDMode(TRUE);  //G100_Simon_0088

        halWarping_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_DrawSpriteBuffer(int startX,
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


#if 1  //draw on dram of Warp OSD copy from DrawPNG function
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    int xPos = startX;
    int yPos = startY;
    int ulWidth = width;
    int ulHeight = height;

    INT32 m_nHorzResolution = halWarping_HResGet();  //A70LK_Simon_0001
    INT32 m_nVertResolution = halWarping_VResGet();

    uint08 ucTmpData[m_nHorzResolution];
    memset(ucTmpData, 0, m_nHorzResolution);

    UINT32 ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_1);

if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
{
    LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);

#ifdef FPGA_ENTRY_4K
        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_1);
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE_1);
#endif /* FPGA_ENTRY_4K */

        INT32 tmpWidth  = (xPos + ulWidth)  > m_nHorzResolution ? m_nHorzResolution : ulWidth  + xPos;
        INT32 tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight + yPos;

        INT32 lIndex = 0 ;

        for (int y = yPos; y < tmpHeight; y++)
        {
            //memset(ucTmpData, 0x10, sizeof(ucTmpData)); // nInhibitColor
            memset(ucTmpData, 0xff, sizeof(ucTmpData)); // transparent color
            dvProAV_LineBufRead(eDramBank0, ulSpriteOsdAddress + y * m_nHorzResolution / 64, m_nHorzResolution / 64, ucTmpData);  //get current displayed data
            for (int x = xPos; x < tmpWidth; x++)
            {
                if(x >= 0 && y >= 0)
                    ucTmpData[x] = paucBlend[lIndex];

//                printf("x,y = (%d, %d) nX,nY = (%d, %d),  ColorIdx : %d, value = %d \n", x, y,nXPos, nYPos, lIndex, paucBlend[lIndex]);
                lIndex++ ;
            }
            eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + y * m_nHorzResolution / 64, m_nHorzResolution / 64, ucTmpData);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
        LOG_MSG(db_HAL_WARPING, "(%s %d) PWPOSD_SAD_AFTER = 0x%08x\n", __FUNCTION__, __LINE__, ulSpriteOsdAddress);

        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

}
#endif

    LOG_MSG(db_HAL_WARPING, "End %s\n", __FUNCTION__);
    return eResult;

}

#if 0
eHAL_WARPING_EXEC_CODE halWarping_WarpOSDModeSet(UINT8 ucFront)  //G100_Simon_0088
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        halWarping_SemaphoreGive(__FUNCTION__);
    	return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
}
#endif /* 0 */

eHAL_WARPING_EXEC_CODE halWarping_DrawCheckboard(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                           INT16 iRectSize)
{
#if 0
    INT16 iHorzRes = m_sHalWarpingInfo.uiWarp_HW ;
    INT16 iVertRes = m_sHalWarpingInfo.uiWarp_VW ;

    LOG_MSG(db_HAL_WARPING, "%s %d %d\n" ,__FUNCTION__, ucLayerMode ,iRectSize);

    if(iRectSize <= 0 || ucLayerMode >= eWDT_INVALID)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#if 0
        if(ucLayerMode == eWDT_AFTER_WARP)  //A65_OPTOMA_Simon_0001
        {
            dvC789_SetOSDMode(FALSE);
        }
        else
        {
            dvC789_SetOSDMode(TRUE);
        }

        //write palette
        UINT8 aucWhite[3] = {255,255,255};
        UINT8 aucBlack[3] = {0,0,0};
        dvC789_Write(B1_PLTAD, COLOR_IDX__WHITE);  //A35G2_CDS_Simon_0046
        dvC789_BurstWrite_FixedAdd(B1_PLTDT, 3, aucWhite);  // 0
        halWarping_PaletteUpdate((UINT16)COLOR_IDX__WHITE, aucWhite[0], aucWhite[1], aucWhite[2]);  //A35G2_CDS_Simon_0046
        dvC789_Write(B1_PLTAD, COLOR_IDX__BLACK);  //A35G2_CDS_Simon_0046
        dvC789_BurstWrite_FixedAdd(B1_PLTDT, 3, aucBlack);  // 1
        halWarping_PaletteUpdate((UINT16)COLOR_IDX__BLACK, aucBlack[0], aucBlack[1], aucBlack[2]);  //A35G2_CDS_Simon_0046
#endif /* 0 */

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

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
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;

}

eHAL_WARPING_EXEC_CODE halWarping_DrawCheckboards(eWARPOSD_DRAW_TYPE ucLayerMode,
                                                             INT16 iSizeX,
                                                             INT16 iSizeY,
                                                             INT16 iShiftX,
                                                             INT16 iShiftY)
{
    LOG_MSG(db_HAL_WARPING, "%s (%d %d %d %d %d)\n" ,__FUNCTION__, ucLayerMode ,iSizeX, iSizeY, iShiftX, iShiftY);
    //printf("%s (%d %d %d %d %d)\n" ,__FUNCTION__, ucLayerMode ,iSizeX, iSizeY, iShiftX, iShiftY);

    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    INT16 iHorzRes = halWarping_HResGet();
    INT16 iVertRes = halWarping_VResGet();

    if(dvProAV_OSD_4k3dEn_Get() && halWarping_PanelGet() == PANEL_2D_HIGHSPEED)
    {
        iHorzRes *= 2;
        iVertRes *= 2;
        iSizeX *= 2;
        iSizeY *= 2;
        iShiftX *= 2;
        iShiftY *= 2;
    }
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
        UINT32 ulSpriteOsdAddress, ulSpriteOsdAddress_old;
        //do
        UINT16 uiColorIndex;
        //INT32 lPalette256[256];
        //memset(lPalette256, 0, sizeof(lPalette256));

//        unsigned int transparency = 0x0;
//        unsigned int white = 0xFFFFFF;

        //lPalette256[0] = 0;  //transparency, use black instead
        //lPalette256[1] = 0xFFFFFF; //white

        //halWarping_OSDWritePaletteWithNumber(lPalette256, 2);
        //update color palette
        halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);  //H2PF_Simon_0038
        halWarpOSD_Palette_RamToChip();  //H2PF_Simon_0038

        int nDouble, nFirstColor = 0, nTmpColor = 0;        // 0 : white, 1 : black(or transparent)
        if(iShiftX < 0) iShiftX = iSizeX + (iShiftX % iSizeX);
        if(iShiftY < 0) iShiftY = iSizeY + (iShiftY % iSizeY);
        int xGrid= (iShiftX / iSizeX) % 2;
        int yGrid= (iShiftY / iSizeY) % 2;
        nFirstColor = xGrid ^ yGrid;

        iShiftX = iSizeX - (iShiftX % iSizeX);
        iShiftY = iSizeY - (iShiftY % iSizeY);

        //printf("==>modify value Size x,y  Shift x,y (%d %d %d %d)\n" ,iSizeX, iSizeY, iShiftX, iShiftY);

        int nHight = 2 * iSizeY < iVertRes ? 2 * iSizeY : iVertRes;
        INT16 iDrawSizeX;
        INT16 iDrawSizeY;

        ulSpriteOsdAddress_old = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_1);

        for (int y = 0; y < nHight;)
        {
            if (y != 0 && (y == iShiftY || y == iShiftY + iSizeY))
                nFirstColor = nFirstColor == 0 ? 0x01 : 0x00;

            iDrawSizeY = iSizeY;

            unsigned char ucPLT = 0;
            for (int x = 0; x < iHorzRes; x = x+iDrawSizeX)
            {
                iDrawSizeX = iSizeX;
                if (x < iShiftX)
                {
                    ucPLT = nFirstColor == 0 ? 0x01 : 0x00;
                    iDrawSizeX = iShiftX;
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

                uiColorIndex = ucPLT == 0 ? AP_COLOR_IDX__BLACK : AP_COLOR_IDX__WHITE;

                if(x + iDrawSizeX > iHorzRes)
                    iDrawSizeX = iHorzRes - x;

//                if(x < 100)
//                printf("x, y, colorIndex(%d %d %d %d)\n" ,x , y , uiColorIndex, iDrawSizeX);
                iDrawSizeY = (y + iDrawSizeY) > iVertRes ? (iVertRes-y) : iDrawSizeY;

                eResult = halWarping_DrawRect(eWDT_AFTER_WARP, x, y, iDrawSizeX, iDrawSizeY, uiColorIndex );
                if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                    return eResult;

                ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_1);
                if(ulSpriteOsdAddress_old != ulSpriteOsdAddress)
                {
                    x = 0;
                }

            }


            int nYsize = 0;

            if (y == 0)
                nYsize = iShiftY != 0 ? iShiftY : iSizeY;
            else if (iShiftY != 0 && y == iShiftY + iSizeY)
                nYsize = iSizeY - iShiftY;
            else
                nYsize = iSizeY;
#if 0
            for (int nHight = 1; nHight < nYsize; nHight *= 2)
            {
                // printf("Height : %d \n", nHight);

                if (nHight + nHight < iSizeY)
                {
                    eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, y, iHorzRes, nHight, 0, y + nHight);
                    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                        return eResult;
                }
                else
                {
                    eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, y, iHorzRes, nYsize - nHight, 0, y + nHight);
                    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                        return eResult;
                }
            }
#endif
            y += nYsize;
            // printf("y : %d \n", y);
        }

        nDouble = 1;
        for (int nHight = 2 * iSizeY; nHight < iVertRes;
             nHight = 2 * iSizeY * nDouble)
        {
            // printf("Height : %d iHorzRes - nHight = %d \n", nHight,
            // iHorzRes - nHight);
            if (nHight + 2 * iSizeY * nDouble < iVertRes)
            {
                eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, 0, iHorzRes, 2 * iSizeY * nDouble, 0, nHight);
                if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                    return eResult;
            }
            else
            {
                eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, 0, iHorzRes, iVertRes - nHight, 0, nHight);
                if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                    return eResult;
            }
            nDouble *= 2;
        }

    return eHAL_WARPING_EXEC_CODE_PASS;

}



eHAL_WARPING_EXEC_CODE halAdvWarpControl(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    //black level
    #if 0
    switch((eWARP_CTRL)ucValue)  //G100_Simon_0005
    {
        case WARP_CTRL__ADVANCED:
        case WARP_CTRL__AP:         //G100_Simon_0080
            #if (CMD_ONLY_BLACK_LEVEL == TRUE)  //A35G2_Simon_0088
            //halWarping_Blacklevel_Enable(TRUE);  //A35G2_Simon_0093
            #endif
            break;

        default:
            halWarping_Blacklevel_Enable(FALSE);
            break;
    }
    #endif

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_SetWarpCtrl((eWARP_CTRL)ucValue);
		if(bVal)
		{
            SaveWarpConfig();//kenton_temp_check
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halAdvWarpGridPoints(UINT8 ucGridPorins, UINT8 ucInner)		//G100_Doulas_0027  //A35G2_BRC_Simon_0001
{

	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    LOG_MSG(db_HAL_WARPING, "%s (Grid %d Inner %d)\r\n", __FUNCTION__, ucGridPorins, ucInner);

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_SetWarpPoint((eWARP_POINT)ucGridPorins);
        BOOL bInnerVal = utilWarp_SetWarpInner(ucInner);
		if(bVal || bInnerVal)
		{
			SaveWarpConfig();//kenton_temp_check
		}

		//utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpInnerGridPoints(UINT8 ucValue)		//G100_Doulas_0027
{

	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_SetWarpInner(ucValue);
		if(bVal)
		{
			SaveWarpConfig();//kenton_temp_check
		}

		//utilWarp_ShowOsdPattern(PAT_TYPE__WARP_SEL_CTRL_POINT);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpShowOsdPattern(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_ShowOsdPattern((ePAT_TYPE)ucValue);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpSelectControlPoint(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_SelectControlPoint((eDIR)ucValue);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpMoveControlPoint(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_MoveControlPoint((eDIR)ucValue);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpSelectOsdBlendWidth(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_SelectOsdBlendWidth((eDIR)ucValue);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpAdjustOsdBlendWidth(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{

		eADJ eADJVal;
		if((ucValue == DIR__UP) || (ucValue == DIR__RIGHT))
			eADJVal = ADJ__PLUS;
		else
			eADJVal = ADJ__MINUS;
		utilWarp_AdjustOsdBlendWidth(eADJVal);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpSharpness(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        BOOL bVal = utilWarp_SetOsdWarpSharpness(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			SaveWarpConfig();	//G100_Doulas_0029 Add
		}


        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGridColor(UINT8 ucValue, UINT8 ucRedrawPattern)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        BOOL bVal = utilWarp_SetOsdGridColor((eCOLOR_IDX)ucValue, ucRedrawPattern);
		if(bVal)
		{
			SaveWarpConfig();//kenton_temp_check
		}


        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpBackgroundColor(UINT8 ucValue, UINT8 ucRedrawPattern)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        BOOL bVal = utilWarp_SetOsdBkgColor((eBKG_COLOR)ucValue, ucRedrawPattern);
		if(bVal)
		{
			SaveWarpConfig();//kenton_temp_check
		}


        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingOverlapGridNumber(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        BOOL bVal = utilWarp_SetOverlapGridNum(ucValue);
		if(bVal)
		{
            utilWarp_ReDrawWarpPattern(); //A35G2_BRC_Casper_0047
			SaveWarpConfig();//kenton_temp_check
		}


        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingGamma(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        BOOL bVal = utilWarp_SetOsdBlendGamma(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			SaveWarpConfig();//kenton_temp_check
		}


        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingLeft(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halAdvBlendingRight(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingTop(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlendingBottom(UINT8 ucValue)		//G100_Doulas_0027
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

	memcpy(&(stWarpingBasic), &(m_stWarpConfig.stOsdBasic.stWArpGeoPara), sizeof(sWARP_BASIC));
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

	memcpy(&(stBlendingBasic), &(m_stWarpConfig.stOsdBasic.stBlendingPara), sizeof(sBLENDING_BASIC));
    utilBasicBlendSettingSet(stBlendingBasic);

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
   	memcpy(&(m_stWarpConfig.stOsdBasic.stWArpGeoPara),&(stWarpingBasic), sizeof(sWARP_BASIC));

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
	memcpy(&(m_stWarpConfig.stOsdBasic.stBlendingPara),&(stBlendingBasic), sizeof(sBLENDING_BASIC));

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
    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halBasicSaveWarpMemory(UINT8 ucValue)
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT8 ucdata;
		ucdata = utilWarp_SaveBasicWarpMemoryPreset(ucValue);
		if(ucdata != FLASH_ACCESS__PASS)
		{
			LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);	//G100_Doulas_0047 Add
			eResult = eHAL_WARPING_EXEC_CODE_FAIL;
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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

		//LOG_MSG(db_ALWAYS, "utilWarp_LoadMemoryPreset 0,Return %d,Warp %d\r\n",ucdata,ucWarpmode);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT8 ucdata;
		ucdata = utilWarp_SaveBasicBlendMemoryPreset(ucValue);
		if(ucdata != FLASH_ACCESS__PASS)
		{
			LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);	//G100_Doulas_0047 Add
			eResult = eHAL_WARPING_EXEC_CODE_FAIL;
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eResult;
}

eHAL_WARPING_EXEC_CODE halBasicApplyBlendMemory(UINT8 ucValue)
{
    eRESULT eDvResult = rcSUCCESS;
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    UINT8 ucdata;

	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        eDvResult |= dvProAV_BldDBDEnableSet(false);
        eDvResult |= dvProAV_BldDBDBiasEnableSet(false);

        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

        utilWarp_LoadBasicBlendMemoryPreset(ucValue);

        UINT8 ucdata;
		UINT8 ucWarpmode;

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

		//LOG_MSG(db_ALWAYS, "utilWarp_LoadMemoryPreset 0,Return %d,Warp %d\r\n",ucdata,ucWarpmode);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT8 ucdata;
		ucdata = utilWarp_SaveMemoryPreset(ucValue);
		if(ucdata != FLASH_ACCESS__PASS)
		{
			LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);	//G100_Doulas_0047 Add
			eResult = eHAL_WARPING_EXEC_CODE_FAIL;
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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

		//LOG_MSG(db_ALWAYS, "utilWarp_LoadMemoryPreset 0,Return %d,Warp %d\r\n",ucdata,ucWarpmode);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT8 ucdata;
		ucdata = utilWarp_ClearOsdPreset();
		if(ucdata != FLASH_ACCESS__PASS)
		{
			//eResult = eHAL_WARPING_EXEC_CODE_FAIL;
		}

        dvProAV_BldUpdatingEnable(false);

        dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, FALSE);
        dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, FALSE);
        dvProAV_BldDBDEnableSet(false);
        dvProAV_BldDBDBiasEnableSet(false);

        dvProAV_BldUpdatingEnable(true);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvResetWarping(void)		//G100_Doulas_0027
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
		utilWarp_ResetWarpConfig();

		//LOG_MSG(db_ALWAYS, "halAdvResetWarping\r\n");
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvWarpVstartOffsetSet(ePANEL_ID ePanelTimingId)      //G100_Doulas_0043
{
    switch(ePanelTimingId)
    {
        case ePANEL_ID_WUXGA_60HZ:
            if(halScaler_PIP_PBP_Enable_Get())		//G100_Doulas_0045 Modify warning
            {
                //PS_PANEL_VST = OSD_V_START_WUGA_60 - 2;//20;//32;
                //utilWarp_Panel_Vstart_Offset_Set(0);//(-2); //A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                //PS_PANEL_VST = OSD_V_START_WUGA_60;//20;//32;
                //utilWarp_Panel_Vstart_Offset_Set(0);
            }
            break;

        case ePANEL_ID_1080P_60HZ:
            if(halScaler_PIP_PBP_Enable_Get())		//G100_Doulas_0045 Modify warning
            {
                //PS_PANEL_VST = OSD_V_START_1080P_60 - 2;//41;
                //utilWarp_Panel_Vstart_Offset_Set(0);//(-2); //A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                //PS_PANEL_VST = OSD_V_START_1080P_60;//41;
                //utilWarp_Panel_Vstart_Offset_Set(0);
            }
            break;

	 	case ePANEL_ID_WXGA_120HZ:		//R70G2_Doulas_0004
            if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            {
                //utilWarp_Panel_Vstart_Offset_Set(0);
            }
            else
            {

                //utilWarp_Panel_Vstart_Offset_Set(0);
            }
            break;

        case ePANEL_ID_1080P_120HZ:
	 	case ePANEL_ID_WUXGA_120HZ:
            if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)	//G100_Doulas_0045 Modify warning
            {
                //PS_PANEL_VST = OSD_V_START_1080P_120 + 2;//41;
                //utilWarp_Panel_Vstart_Offset_Set(0);
            }
            else
            {
                //PS_PANEL_VST = OSD_V_START_1080P_120;//41;
                //utilWarp_Panel_Vstart_Offset_Set(-2);
            }
            break;
        default:
            break;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_AP_DrawRect(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

#if 0
	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    LOG_MSG(db_HAL_WARPING, "==> %d %d %d %d %d %d\n" , eType
                                                      , iPositionX
                                                      , iPositionY
                                                      , iWidth
                                                      , iHeight
                                                      , iColorIndex);

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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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
                            dvC789_WriteToBuffer(B0_CPUWAD, cpuwad);
                        }

                        dvC789_WriteToBuffer(B0_CPUDT, m_sHalWarpingInfo.aucPalette[iColorIndex][idx]);
                    }
                }
            }
            dvC789_Buffer_Flush();

        }
        else   //insert warp OSD after warp
        {
            dvC789_SetOSDMode(FALSE);

            int cpuwad;
            int GV_RTCT_BAK = dvC789_Read(BN_RTCT); //0x0105;

            //Cindy: change RTCT to avoid blending not shown
            int GV_RTCT = 0x0105;
            cpuwad = m_OSD_CURRENT_LAYER + iPositionX + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iPositionY;

            GV_RTCT = GV_RTCT | 0x0007;
            dvC789_WriteToBuffer( BN_RTCT, GV_RTCT );
            dvC789_WriteToBuffer( B0_OSDFILL, iColorIndex );
            dvC789_WriteToBuffer( B0_BBACTHW, (iWidth -1)&0xfff );
            dvC789_WriteToBuffer( B0_BBACTVW, (iHeight-1)&0xfff );
            dvC789_WriteToBuffer( B0_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW/128) );
            dvC789_WriteToBuffer( B0_CPUWAD, cpuwad&0x1fffffff );
            dvC789_WriteToBuffer( B0_OSDCT, 0x01 );
            GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
            dvC789_WriteToBuffer( BN_RTCT, GV_RTCT );
            dvC789_WriteToBuffer( BN_RTCT, GV_RTCT_BAK );
            dvC789_Buffer_Flush();

            do { } while ( (dvC789_Read(B0_BOSTAT) & 0x01) == 0x01 );

        }

        halWarping_SemaphoreGive(__FUNCTION__);
    	eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    }

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);
#endif /* 0 */

	return eResult;
}


//draw on memory by 24bit color (0xRRGGBB)
eHAL_WARPING_EXEC_CODE halWarping_AP_DrawBox(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iRed, INT16 iGreen, INT16 iBlue)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

#if 0
	LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    LOG_MSG(db_HAL_WARPING, "==> %d %d %d %d %d - %d %d %d\n" , eType
                                                              , iPositionX
                                                              , iPositionY
                                                              , iWidth
                                                              , iHeight
                                                              , iRed
                                                              , iGreen
                                                              , iBlue);

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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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
                            dvC789_WriteToBuffer(B0_CPUWAD, cpuwad);
                        }

                        if(idx == 0)
                        {
                            dvC789_WriteToBuffer(B0_CPUDT, iRed);
                        }
                        else if(idx == 1)
                        {
                            dvC789_WriteToBuffer(B0_CPUDT, iGreen);
                        }
                        else if(idx == 2)
                        {
                            dvC789_WriteToBuffer(B0_CPUDT, iBlue);
                        }
                    }
                }
            }

            dvC789_Buffer_Flush();

        }

        halWarping_SemaphoreGive(__FUNCTION__);
    	eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    }

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);
#endif /* 0 */

	return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_AP_DrawLine(eWARPOSD_DRAW_TYPE eType, INT16 iLeft, INT16 iTop, INT16 iRight, INT16 iBottom, INT32 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

    iRight = iRight == 0 ? 1 : iRight;
    iBottom = iBottom == 0 ? 1 : iBottom;
    LOG_MSG(db_HAL_WARPING, "(%s@%d) Start\n", __FUNCTION__, __LINE__);

    UINT16 uiColorIndex = halGui_RGBColorToIndex(iColorIndex);
    LOG_MSG(db_HAL_WARPING, "use Palette Index = %d\n", uiColorIndex);
    uiColorIndex = (uiColorIndex>255) ? 255 : uiColorIndex;  //use transparent color(255) if no matched index was found in scaler osd palette

    halWarping_DrawRect(eType, iLeft, iTop, iRight, iBottom, uiColorIndex);
#if 0
    int cpuwad_previous = 1;
    int cpuwad = 0, offset = 0;

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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(eType == eWDT_BEFORE_WARP)   //insert warp OSD before warp
        {
            for (int idx = 0; idx < 3; idx++)
            {
                for (int y = iTop ; y <= iBottom ; y++)
                {
                    for (int x = iLeft ; x <= iRight ; x++)
                    {
                        if (x % 12 < 6)
                            offset = x % 12;
                        else
                            offset = x % 12 + 2;

                        if (x % 12 == 0 || x == iLeft)
                        {
                            cpuwad =
                                (y / 4) * (2 - 0) * 32768 + (y % 4) * 16 +
                                (x / 12) * 256 + offset;
                            cpuwad = cpuwad + 64 * idx;
                            dvC789_WriteToBuffer(B0_CPUWAD, cpuwad);
                        }
                        else if (x % 12 == 6)   //A35G2_CDS_Simon_0028
                        {
                            dvC789_WriteToBuffer(B0_CPUDT, 0x00);
                            dvC789_WriteToBuffer(B0_CPUDT, 0x00);
                        }

                        // (plt_fc >> (2 - idx) * 8) & 0x0000FF
                        dvC789_WriteToBuffer(B0_CPUDT, (iColorIndex >> (idx * 8)) & 0x0000FF);

                        //LOG_MSG(db_HAL_WARPING, "iColorIndex %X\n" , (iColorIndex >> (idx * 8)) & 0x0000FF);
                        // dvC789_WriteBuffer(B0_CPUDT, paucBlend[nWidth*3*y+3*x+idx],
                        // 0);
                    }
                }
            }

            dvC789_Buffer_Flush();

        }
        else   //insert warp OSD after warp
        {
            dvC789_SetOSDMode(FALSE);
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    	eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    }

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);
#endif /* 0 */

	return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_DrawRect_WriteByCPUDT(eWARPOSD_DRAW_TYPE eType, INT16 iPositionX, INT16 iPositionY, INT16 iWidth, INT16 iHeight, INT16 iColorIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

#if 0
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
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
                            dvC789_WriteToBuffer(B0_CPUWAD, cpuwad);
                        }

                        dvC789_WriteToBuffer(B0_CPUDT, m_sHalWarpingInfo.aucPalette[iColorIndex][idx]);
                    }
                }
            }
            dvC789_Buffer_Flush();

        }
        else   //insert warp OSD after warp
        {
            dvC789_SetOSDMode(FALSE);

            dvC789_WriteToBuffer(BN_RTCT, 0x0105);     // 0x0105 is the initial value of RTCT

            dvC789_WriteToBuffer(B1_OSDMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);
            dvC789_WriteToBuffer(B0_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);

            for (UINT32 y = iPositionY; y < iPositionY + iHeight; y++)
            {
                UINT32 cpuwad = m_OSD_CURRENT_LAYER + iPositionX + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * y;

                dvC789_WriteToBuffer(B0_CPUWAD, cpuwad);

                for (UINT32 x = iPositionX; x < iPositionX + iWidth; x++)
                {
                    dvC789_WriteToBuffer(B0_CPUDT, iColorIndex);
                }
            }

            dvC789_Buffer_Flush();
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    	eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    }

	LOG_MSG(db_HAL_WARPING, "(%s@%d) End\n", __FUNCTION__, __LINE__);
#endif /* 0 */

	return eResult;
}


eHAL_WARPING_EXEC_CODE halWarping_FreezeImage(UINT8 ucEnable)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    m_sHalWarpingInfo.ucFreeze = ucEnable;
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if((m_sHalWarpingInfo.ucDrawPngOnBgEnable == 1)
            && (ucEnable == 0))
        {
            m_sHalWarpingInfo.ucDrawPngOnBgEnable = 0;
            dvProAV_SclVopScreenOffSet(false);
            dvProAV_SclLogoDisplayEn(false);
            dvProAV_OSD_Enable(true);
            LOG_MSG(db_HAL_WARPING, "%s disable BG (%d) and enable OSD\n", __FUNCTION__, m_sHalWarpingInfo.ucDrawPngOnBgEnable);
        }
        halWarping_SemaphoreGive(__FUNCTION__);
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail\n", __FUNCTION__);
    }

	return eResult;
}


UINT8 halWarping_GetFreezeImageState(void)
{
    return m_sHalWarpingInfo.ucFreeze;
}

eHAL_WARPING_EXEC_CODE halWarping_OSD_Swap_Memplane(void)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

#if 0
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        LOG_MSG(db_HAL_WARPING, "%s 0x%X\n", __FUNCTION__, m_OSD_CURRENT_LAYER );

        m_OSD_CURRENT_LAYER = ( m_OSD_CURRENT_LAYER != m_OSD_MEM_LAYER0 ) ? m_OSD_MEM_LAYER0 : m_OSD_MEM_LAYER1;

        LOG_MSG(db_HAL_WARPING, "%s 0x%X\n", __FUNCTION__, m_OSD_CURRENT_LAYER );

        dvC789_Write(BN_RTCT, 0x0105);     // 0x0105 is the initial value of RTCT

        INT32 lTransparentColor = dvC789_Read(B1_BOTRANS0);

    	int cpuwad = m_OSD_CURRENT_LAYER;

    	int GV_RTCT_BAK = dvC789_Read(BN_RTCT); //0x0105;

    	//Cindy: change RTCT to avoid blending not shown
    	int GV_RTCT = 0x0105; //0x0105;
    	GV_RTCT = GV_RTCT | 0x0007;
    	dvC789_WriteToBuffer( BN_RTCT, GV_RTCT );
    	dvC789_WriteToBuffer( B0_OSDFILL, lTransparentColor );
    	dvC789_WriteToBuffer( B0_BBACTHW, (m_sHalWarpingInfo.uiWarp_HW-1)&0xfff );
    	dvC789_WriteToBuffer( B0_BBACTVW, (m_sHalWarpingInfo.uiWarp_VW-1)&0xfff );
    	dvC789_WriteToBuffer( B0_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW/128));
    	dvC789_WriteToBuffer( B0_CPUWAD, cpuwad&0x1fffffff );
    	dvC789_WriteToBuffer( B0_OSDCT, 0x01 );
    	GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
    	dvC789_WriteToBuffer( BN_RTCT, GV_RTCT );
    	dvC789_WriteToBuffer( BN_RTCT, GV_RTCT_BAK );
    	dvC789_Buffer_Flush();

    	do { } while ( (dvC789_Read( B0_BOSTAT ) & 0x01) == 0x01 );

        dvC789_WriteToBuffer( B0_OSDCT, 0x00 );

        GV_RTCT = GV_RTCT | 0x0007;
        dvC789_WriteToBuffer( BN_RTCT, GV_RTCT );
        dvC789_WriteToBuffer( B0_OSDCT, 0x00 );
        //dvC789_WriteToBuffer( B1_OSDMODE, 0x07 );  //0x03:0x07 //A35G2_CDS_Simon_0044
        GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
        dvC789_WriteToBuffer( BN_RTCT, GV_RTCT );
    	dvC789_Buffer_Flush();

        halWarping_SemaphoreGive(__FUNCTION__);
        eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
        eResult = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail\n", __FUNCTION__);
    }
#endif /* 0 */

    return eResult;

}


eHAL_WARPING_EXEC_CODE halWarping_OSD_ON(eWARPOSD_DRAW_TYPE eType)  //H2PF_Simon_0035
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

    LOG_MSG(db_HAL_WARPING, "(funs:%s, line:%d) - (%d)\r\n",__FUNCTION__, __LINE__, eType);

    halWarpOSD_DrawOSD(eType);

    #if 0
    if(*eType == eWDT_AFTER_WARP)
    {
        appGui_Send_OSD_Exit();
    }
    #endif

    //halWarping_Set_Draw_OSD_On(1);// R70K_Bruce_0022

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_OSD_PalettePrint(void)
{

#if 1
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        UINT8 OsdPalette[768] = {0};
        utilProcMutexData_OsdPalette_Get(OsdPalette);

        LOG_MSG(db_HAL_WARPING, "\r\n");
        for(int i=0 ; i<256 ; i++)
        {
            LOG_MSG(db_HAL_WARPING, "Plt %3d Scaler OSD(%3d %3d %3d) Osd_Ram(%3d %3d %3d) AP_Ram(%3d %3d %3d)\n", i
                                                                                                                , OsdPalette[i*3 + 0], OsdPalette[i*3 + 1], OsdPalette[i*3 + 2]
                                                                                                                , m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette[i][0], m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette[i][1], m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette[i][2]
                                                                                                                , m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[i][0],  m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[i][1],  m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[i][2]);
            MS_SLEEP(6);
        }

        UINT32 aucWarpOSD_TranparentColor[4];
        memset(aucWarpOSD_TranparentColor, 0, sizeof(aucWarpOSD_TranparentColor));
        dvProAV_AccessRead(eWrpOsdTranColor0, &aucWarpOSD_TranparentColor[0]);
        dvProAV_AccessRead(eWrpOsdTranColor1, &aucWarpOSD_TranparentColor[1]);
        dvProAV_AccessRead(eWrpOsdTranColor2, &aucWarpOSD_TranparentColor[2]);
        dvProAV_AccessRead(eWrpOsdTranColor3, &aucWarpOSD_TranparentColor[3]);
        LOG_MSG(db_HAL_WARPING, "\nTransparent Color %d %d %d %d\n\n", aucWarpOSD_TranparentColor[0], aucWarpOSD_TranparentColor[1], aucWarpOSD_TranparentColor[2], aucWarpOSD_TranparentColor[3]);
        MS_SLEEP(5);

        UINT32 WrpOsdInhiColorEn, WrpOsdInhiColor;
        dvProAV_AccessRead(eWrpOsdInhiColorEn, &WrpOsdInhiColorEn);
        dvProAV_AccessRead(eWrpOsdInhiColor, &WrpOsdInhiColor);
        LOG_MSG(db_HAL_WARPING, "Inhibit Color En? %d\n", WrpOsdInhiColorEn);
        LOG_MSG(db_HAL_WARPING, "Inhibit Color Idx %d\n\n", WrpOsdInhiColor);
        MS_SLEEP(10);

        dvProAV_OSD_DbgPrint();

        dvProAV_WarpOSD_DbgPrint();

        MS_SLEEP(5);
        LOG_MSG(db_HAL_WARPING, "Process Mutex Data :\n");
        LOG_MSG(db_HAL_WARPING, "SHM_BLEND_AP_LINK_STATE : %d\n", SHM_BLEND_AP_LINK_STATE);
        LOG_MSG(db_HAL_WARPING, "SHM_DRAW_WARPOSD_STATE  : %d\n", SHM_DRAW_WARPOSD_STATE(eWIL_BEFORE_WARP));

        //LOG_MSG(db_HAL_WARPING, "m_OSD_CURRENT_LAYER => 0x%X\n\n", m_OSD_CURRENT_LAYER);
        //LOG_MSG(db_HAL_WARPING, "CPU OSDSAD => 0x%X\n\n", dvC789_Read(B1_OSDSAD));

        halWarping_SemaphoreGive(__FUNCTION__);

    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail\n\n", __FUNCTION__);
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//A35G2_Simon_0067
eHAL_WARPING_EXEC_CODE halWarping_OSD_TransparentColor_Set(UINT8 ucEnable, UINT8 ucColorIndex)
{
    //A70LK_CL_0002
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d) warp osd transparent color index = %d\n", __FUNCTION__, __LINE__, ucColorIndex);
        dvProAV_AccessWrite(eWrpOsdTranColor0, ucColorIndex);
        dvProAV_AccessWrite(eWrpOsdTranColor1, ucColorIndex);
        dvProAV_AccessWrite(eWrpOsdTranColor2, ucColorIndex);
        dvProAV_AccessWrite(eWrpOsdTranColor3, ucColorIndex);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#if 0
    UINT8 ucOSDMODE = (UINT8)dvC789_Read(B1_OSDMODE);
    ucOSDMODE &= ~(BIT1|BIT0);
    if(ucEnable)
    {
        ucOSDMODE |= (BIT1|BIT0);

        dvC789_WriteToBuffer(B1_BOTRANS0, ucColorIndex);
        dvC789_WriteToBuffer(B1_BOTRANS1, ucColorIndex);
        dvC789_WriteToBuffer(B1_BOTRANS2, ucColorIndex);
        dvC789_WriteToBuffer(B1_BOTRANS3, ucColorIndex);
    }
    else
    {
        ucOSDMODE |= (BIT1);
    }

    LOG_MSG(db_HAL_WARPING, "set OSDMODE %d\r\n", ucOSDMODE);
    dvC789_WriteToBuffer(B1_OSDMODE, ucOSDMODE);
    dvC789_Buffer_Flush();
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_OSD_TransparentColor_Get(eTRANSPARENT_COLOR_SLOT eSlotIndex, UINT32 *ulColor)
{
    //only eWrpOsdTranColor0
    dvProAV_AccessRead(eWrpOsdTranColor0, ulColor);

    #if 0
    switch(eSlotIndex)
    {
        case eTC_SLOT0:
            dvProAV_AccessRead(eWrpOsdTranColor0, ulColor);
            break;

        case eTC_SLOT1:
            dvProAV_AccessRead(eWrpOsdTranColor1, ulColor);
            break;

        case eTC_SLOT2:
            dvProAV_AccessRead(eWrpOsdTranColor2, ulColor);
            break;

        case eTC_SLOT3:
            dvProAV_AccessRead(eWrpOsdTranColor3, ulColor);
            break;

        default:
            *ulColor = 0;
            return eHAL_WARPING_EXEC_CODE_FAIL;

    }
    #endif

    return eHAL_WARPING_EXEC_CODE_PASS;
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

#if 0
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        int cpurad = m_OSD_CURRENT_LAYER + ixSource + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iySource;     //R70G2_Simon_0002
        int cpuwad = m_OSD_CURRENT_LAYER + ixDestination + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iyDestination;
        //LOG_MSG(db_HAL_WARPING, "cpurad 0x%X wad 0x%X\n", cpurad, cpuwad);

        dvC789_WriteToBuffer(B0_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);
        dvC789_WriteToBuffer(B0_BBRMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);
        dvC789_WriteToBuffer(B0_BBACTHW, iWidth - 1);
        dvC789_WriteToBuffer(B0_BBACTVW, iHeight - 1);
        dvC789_WriteToBuffer(B0_CPURAD, cpurad & 0x07ffffff);
        dvC789_WriteToBuffer(B0_CPUWAD, cpuwad & 0x07ffffff);
        dvC789_WriteToBuffer(B0_OSDCT, 0x02);      // bitblt
        dvC789_Buffer_Flush();

        do
        {
        }
        while ((dvC789_Read(B0_BOSTAT) & 0x01) == 0x01);
        dvC789_Write(B0_OSDCT, 0x00);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

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


#if 0
    int cpurad = m_OSD_CURRENT_LAYER + ixSource + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iySource;     //R70G2_Simon_0002
    int cpuwad = m_OSD_CURRENT_LAYER + ixDestination + (m_sHalWarpingInfo.uiWarp_HW/128) * 128 * iyDestination;
    LOG_MSG(db_HAL_WARPING, "cpurad 0x%X , cpuwad 0x%X\n\n", cpurad, cpuwad);

    dvC789_WriteToBuffer(B0_BBWMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);
    dvC789_WriteToBuffer(B0_BBRMWI, (m_sHalWarpingInfo.uiWarp_HW/128) & 0xff);
    dvC789_WriteToBuffer(B0_BBACTHW, iWidth - 1);
    dvC789_WriteToBuffer(B0_BBACTVW, iHeight - 1);
    dvC789_WriteToBuffer(B0_CPURAD, cpurad & 0x07ffffff);
    dvC789_WriteToBuffer(B0_CPUWAD, cpuwad & 0x07ffffff);
    dvC789_WriteToBuffer(B0_OSDCT, 0x02);      // bitblt
    dvC789_Buffer_Flush();

    do
    {
    }
    while ((dvC789_Read(B0_BOSTAT) & 0x01) == 0x01);
    dvC789_Write(B0_OSDCT, 0x00);
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}



eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_CurrentSetting(void)
{
    LOG_MSG(db_HAL_WARPING, "Panel = %dx%d\n", m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW);

#if 0
    UINT8 ePanel ;
    halWarping_AP_LoadPanelIDFromCurrentFile(&ePanel);

    if(ePanel != m_sHalWarpingInfo.ePanelTimingId)  //A65_OPTOMA_Simon_0002
    {
        LOG_MSG(db_HAL_WARPING, "Panel not match, skip (%d)(%d)\n", ePanel, m_sHalWarpingInfo.ePanelTimingId);
        halWarping_ClearWarpingTable();
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
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;

}

//R70G2_Simon_0001
eHAL_WARPING_EXEC_CODE halWarping_MemoryApplyAP_Warp(UINT8 ucIndex)
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
    LOG_MSG(db_HAL_WARPING, "\r\n(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    INT8 cFileName[128] = {0};
    snprintf((char *)cFileName, 128, "%s/Warp%d", BLENDING_AP_SAVING_PATH, ucIndex+1);

    UINT32 ulSize = sizeof(sWarpFileInfo);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        halWarp_GeometryParameterSet(eWARP_EVENT_WARPOFF, NULL);
        halWarp_GeometrySet();
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
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
    LOG_MSG(db_HAL_WARPING, "0x%08X LutTable %d\n",           offsetof(sWarpFileInfo, aucLutTable),   sWarpInfo->aucLutTable[0]);

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
        if(m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT)  //not 3D panel  //A70LK_Simon_0010
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
		UINT16 uiDefSpace = DEF_WP_SPACE_4K;	//A70LK_Sammy_0001
		if(m_sHalWarpingInfo.ucIs4KWarp)
		{
			uiDefSpace = DEF_WP_SPACE_4K;
		}
		else
		{
			uiDefSpace = DEF_WP_SPACE_2K;
		}
        uiApplyWidth  = (m_sHalWarpingInfo.uiWarp_HW + uiDefSpace - 1) / uiDefSpace + 1;
        uiApplyHeight = (m_sHalWarpingInfo.uiWarp_VW + uiDefSpace - 1) / uiDefSpace + 1;

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
    snprintf((char *)cFileName, 128, "%s/Blend%d", BLENDING_AP_SAVING_PATH, ucIndex+1);

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

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
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
            eResult |= halWarping_DBD_Blending_Enable(FALSE);    //A70LK_Simon_0012
            halWarp_BlendingParameterSet(eBLENDING_EVENT_OFF, NULL);
            halWarp_BlendingSet();
            free(pucData);
            return eHAL_WARPING_EXEC_CODE_PASS;
        }
    }
    else  //blending data is 3D
    {
        if(m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT)  //not 3D panel
        {
            LOG_MSG(db_HAL_WARPING, "Panel not match , skip apply blend %d %d\n", sBlendInfo->uc3DFlag, m_sHalWarpingInfo.ePanelTimingId);
            eResult |= halWarping_DBD_Blending_Enable(FALSE);    //A70LK_Simon_0012
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
    snprintf((char *)cFileName, 128, "%s/BlackLevel%d", BLENDING_AP_SAVING_PATH, ucIndex+1);

    UINT32 ulSize = sizeof(sBlackLevelFileInfo);
    UINT8 *pucData = (UINT8 *)malloc(ulSize);
    if(pucData == NULL)
    {
        ASSERT_ALWAYS();
        free(pucData);
        halWarping_BlackLevel_Enable(FALSE);
        return eHAL_WARPING_EXEC_CODE_MALLOC_FAIL;
    }

    if(utilMisc_GetFileData(cFileName, pucData, ulSize) == UTILMISC_NO_FILE)
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
        if(m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT)  //not 3D panel  //A70LK_Simon_0010
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
    eResult |= halWarping_BlackLevel_Table_Set(sBlackLevelInfo->aucTable, m_sHalWarpingInfo.uiWarp_HW, m_sHalWarpingInfo.uiWarp_VW);
    halWarping_BlackLevel_Enable(TRUE);

    INT32 End = TMO_GetSysRunTime();

    LOG_MSG(db_HAL_WARPING, "\nhalWarping_MemoryApplyAP_BlackLevel time %d\n", End-Start);

    free(pucData);

    return eResult;
}

eHAL_WARPING_EXEC_CODE halWarping_AP_OSD_Init(void)  //A65_OPTOMA_Simon_0001
{
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;

#if 0
    //clear OSD
    eResult = halWarpOSD_ClearOSD();

    if(eResult != eHAL_WARPING_EXEC_CODE_PASS)
    {
        return eResult;
    }

    //init OSD layer
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        m_OSD_CURRENT_LAYER = DEF_OSDSAD;

        dvC789_WriteToBuffer(B1_BOTRANS0, 0);
        dvC789_WriteToBuffer(B1_BOTRANS1, 0);
        dvC789_WriteToBuffer(B1_BOTRANS2, 0);
        dvC789_WriteToBuffer(B1_BOTRANS3, 0);  //A35G2_CDS_Simon_0057
        dvC789_WriteToBuffer(B1_BLINKTIME, 0);  //A65_OPTOMA_Simon_0004
        dvC789_WriteToBuffer(B1_BLINKCYCL, 0);
    	dvC789_Buffer_Flush();

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail\n\n", __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    eResult = halWarping_OSD_ON();

    if(eResult != eHAL_WARPING_EXEC_CODE_PASS)
    {
        return eResult;
    }
#endif /* 0 */

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSelectControlPoint(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
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
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelMoveControlPoint(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_OsdBlackLevel_MovePoint((eDIR)ucValue);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelAddPoint(void)				//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_OsdBlackLevel_AddPoint();
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelDeletePoint(void)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		utilWarp_OsdBlackLevel_DeletePoint();
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSelectArea(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_OsdBlackLevel_SetArea(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_PREVIEW);
			utilWarp_ShowOsdPattern(PAT_TYPE__OFF);					//A65_OPTOMA_Doulas_0023
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetEnable(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_OsdBlackLevel_SetEnable(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			SaveWarpConfig();
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetRed(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_OsdBlackLevel_SetRed(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
			{
				SaveWarpConfig();
			}
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetGreen(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_OsdBlackLevel_SetGreen(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
			{
				SaveWarpConfig();
			}
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelSetBlue(UINT8 ucValue)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        BOOL bVal = utilWarp_OsdBlackLevel_SetBlue(ucValue);
		if(bVal)
		{
		//	LOG_MSG(db_ALWAYS, "changed\r\n");
			if(utilWarp_GetOsdPatternType() == PAT_TYPE__OFF)
			{
				SaveWarpConfig();
			}
		}

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelReset(UINT8 ucResetLevel)		//A65_OPTOMA_Doulas_0029 Modify//A65_OPTOMA_Doulas_0020
{
	eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_PASS;
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
    	if(ucResetLevel == eBLACK_LEVEL_RESET_ALL)
			utilWarp_OsdBlackLevel_ResetConfig();
		else if(ucResetLevel == eBLACK_LEVEL_RESET_BOTTOM)
			utilWarp_OsdBlackLevel_ResetConfigBottom();
		else if(ucResetLevel == eBLACK_LEVEL_RESET_TOP)
			utilWarp_OsdBlackLevel_ResetConfigTop();

		//LOG_MSG(db_ALWAYS, "halAdvResetWarping\r\n");
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eResult;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelShowAddPointPattern(void)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		if(utilWarp_OsdBlackLevel_IsPointAddable())
		{
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_ADD_POINT);
		}
		else
		{
		//	LOG_MSG(db_ALWAYS, "Point number is 32, can not be added any more!!\r\n");
		}
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvBlackLevelShowRemovePointPattern(void)		//A65_OPTOMA_Doulas_0020
{
#if 0
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		if(utilWarp_OsdBlackLevel_IsPointDeletable())
		{
			utilWarp_ShowOsdPattern(PAT_TYPE__BLACKLEVEL_DEL_POINT);
		}
		else
		{
		//	LOG_MSG(db_ALWAYS, "Point number is 4, can not be deleted any more!!\r\n");
		}
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

BOOL halAdvBlackLevelIsPointDeletable(void)		//A65_OPTOMA_Doulas_0020
{
    return FALSE;
    //return utilWarp_OsdBlackLevel_IsPointDeletable();
}

BOOL halAdvBlackLevelIsPointAddable(void)		//A65_OPTOMA_Doulas_0020
{
    return FALSE;
    //return utilWarp_OsdBlackLevel_IsPointAddable();
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvC789_WriteToBuffer( B1_PLTAD, 0x00 );
    	for(int idx = 0 ; idx < 10 ; idx++)
    	{
    		dvC789_WriteToBuffer( B1_PLTDT, ColorPallet[idx][0]);
    		dvC789_WriteToBuffer( B1_PLTDT, ColorPallet[idx][1]);
    		dvC789_WriteToBuffer( B1_PLTDT, ColorPallet[idx][2]);
            halWarping_PaletteUpdate((UINT16)idx, ColorPallet[idx][0], ColorPallet[idx][1], ColorPallet[idx][2]);
    	}

    	dvC789_WriteToBuffer( B1_PLTAD, 0x00 ); //dummy

        dvC789_Buffer_Flush();

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX Fail\n\n", __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
#endif /* 0 */

    return eHAL_WARPING_EXEC_CODE_PASS;
}

#if 0    //H2PF_Simon_0038  remove
eHAL_WARPING_EXEC_CODE halWarping_PaletteUpdate(UINT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)    //H2PF_Simon_0038 remove
{
    m_sHalWarpingInfo.aucPalette[uiIndex][0] = ucRed;
    m_sHalWarpingInfo.aucPalette[uiIndex][1] = ucGreen;
    m_sHalWarpingInfo.aucPalette[uiIndex][2] = ucBlue;

    return eHAL_WARPING_EXEC_CODE_PASS;
}
#endif

eHAL_WARPING_EXEC_CODE halWarping_AP_SavePanelIDForCurrentFile(void)  //A65_OPTOMA_Simon_0002
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

eHAL_WARPING_EXEC_CODE halWarping_AP_LoadPanelIDFromCurrentFile(UINT8 *ucPanelID)  //A65_OPTOMA_Simon_0002
{
    UINT8 ucIs3D_Panel = (m_sHalWarpingInfo.ePanelTimingId == PANEL_2D_OUTPUT) ? FALSE : TRUE ;  //A70LK_Simon_0010

    sCurrentFileInfo sReadFileInfo;

    FILE *fp = fopen(BLENDING_AP_CURRENT_INFO_FILENAME, "rb");

    if(fp == NULL)
    {
        *ucPanelID = ePANEL_ID_LAST;
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    fread(&sReadFileInfo , sizeof(sReadFileInfo) , 1 , fp);
    fclose(fp);

    *ucPanelID = sReadFileInfo.sInfo.ucPanelID;

    return eHAL_WARPING_EXEC_CODE_PASS;
}


eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_HST_Set(UINT16 uiData)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        halWarping_SemaphoreGive(__FUNCTION__);
    	return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_HST_Get(UINT16 *uiData)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

        halWarping_SemaphoreGive(__FUNCTION__);
    	return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_VST_Set(UINT16 uiData)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        halWarping_SemaphoreGive(__FUNCTION__);
    	return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
}

eHAL_WARPING_EXEC_CODE halWarping_WarpOSD_VST_Get(UINT16 *uiData)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        //*uiData = dvC789_Read(B1_OSDACTVST);

        halWarping_SemaphoreGive(__FUNCTION__);
    	return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
    	LOG_MSG(db_HAL_WARPING, "(%s@%d) MUTEX FAIL\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }
}


eHAL_WARPING_EXEC_CODE halAdvHide_OSD_Pattern_Set(BOOL bHide)				//A65_OPTOMA_Doulas_0211
{
	if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
#if 0
 		//if(utilWarp_OsdBlackLevel_IsPointAddable())
 		{
 			utilWarp_HideOsdPattern(bHide);
 		}
#endif /* 0 */
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halAdvWarpGP_SetColIndex(UINT8 ucValue)  //G100_Owen_0089 //A35G2_BRC_Casper_0046
{
    eHAL_WARPING_EXEC_CODE eRet = eHAL_WARPING_EXEC_CODE_PASS;

    if(halWarping_TwistLinkFlag_Get() == TRUE)
    {
        return eRet;
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(utilWarp_GridPoint_SetColIndex(ucValue) == FALSE)
        {
            eRet = eHAL_WARPING_EXEC_CODE_FAIL;
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(utilWarp_GridPoint_SetRowIndex(ucValue) == FALSE)
        {
            eRet = eHAL_WARPING_EXEC_CODE_FAIL;
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(utilWarp_GridPoint_SetPositionMove(lPosX, lPosY) == FALSE)
        {
            eRet = eHAL_WARPING_EXEC_CODE_FAIL;
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        if(utilWarp_GridPoint_SetAllPosition(Data) == FALSE)
        {
            eRet = eHAL_WARPING_EXEC_CODE_FAIL; //over limit
        }

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
#if 0
        if(utilWarp_SetBlendEnable(bEnable) == FALSE)
        {
            eRet = eHAL_WARPING_EXEC_CODE_FAIL;
        }
        else
        {
            SaveWarpConfig();
        }
#endif /* 0 */

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        eRet = eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
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

    halWarping_SemaphoreGive(__FUNCTION__);

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

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
	{
		//utilWarp_Black_Level_Boundary_Flag_Set(ucBoundary);

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

UINT8 halWarping_WarpingCheckFlagGet(void)
{
    return m_sHalWarpingInfo.ucWarpingCheck;
}

void halWarping_WarpingCheckFlagSet(UINT8 ucNeedCheck)
{
    LOG_MSG(db_HAL_WARPING, "ucWarpingCheck = %d" , ucNeedCheck);
    m_sHalWarpingInfo.ucWarpingCheck = ucNeedCheck;
}

UINT8 halWarping_DBD_Data_Index_Get(void)
{
    return m_sHalWarpingInfo.ucCurrent_DBD_Data_Index;
}

void halWarping_DBD_Data_Index_Set(UINT8 uIndex)
{
    m_sHalWarpingInfo.ucCurrent_DBD_Data_Index = uIndex;
}

UINT8 halWarping_BlackLevel_Data_Index_Get(void)
{
    return m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index;
}

void halWarping_BlackLevel_Data_Index_Set(UINT8 uIndex)
{
    m_sHalWarpingInfo.ucCurrent_Blacklevel_Data_Index = uIndex;
}


eHAL_WARPING_EXEC_CODE halWarping_BlendingGamma(UINT8 ucEnable)
{
    if(ucEnable)
    {
        m_stWarpConfig.stOsdBasic.stBlendingPara.ucBlendingGamma = eCM_BLENDING_GAMMA_2_2;
        halWarp_BlendingGamma();
    }

    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_FRONT, ucEnable);
        dvProAV_BldGammaEnableSet(BLENDING_GAMMA_TYPE_BACK, ucEnable);
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }


    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_WarpDownScalingIssueWorkAround(void)
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
        dvProAV_WarpDownScalingIssueWorkAround();
        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

#ifdef FPGA_ENTRY_4K
#if 0    //H2PF_Simon_0038 move to palGeo.c
eRESULT halWarping_DrawOSDQueueSet(UINT8* pucBuffer)
{
    eRESULT eResult = rcERROR;

    if(m_sHalWarpingInfo.bWarpingInit == FALSE)
        return eResult;

    if(halWarping_OsdQueueDrawSemaphoreTake())  //A70LK_Simon_0008
    {
        eResult = utilQueueWrite(&m_asWarpingOSD_Ring_Buffer, pucBuffer);
        halWarping_OsdQueueDrawSemaphoreGive();
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d) mutex fail\n", __FUNCTION__, __LINE__);
        eResult = rcERROR;
    }

    ASSERT(eResult == rcSUCCESS);

    return eResult;
}

eRESULT halWarping_DrawWarpingOSD(void)
{
    eRESULT eResult = rcERROR;
    sPAYLOAD sPayload = {0};
    UINT16 wByteCount = 0;
    UINT16 uiItemID;

    if(halWarping_OsdQueueDrawSemaphoreTake())  //A70LK_Simon_0008
    {
        while(rcSUCCESS == utilQueueRead(&m_asWarpingOSD_Ring_Buffer, (UINT8*)&sPayload))
        {
            memcpy((UINT8*)&uiItemID, (UINT8*)&sPayload.aucData[2], 2);

            utilCLI_Geo_DrawOSDFeature(&sPayload, NULL, &wByteCount);

            if((uiItemID == eBLENDER_DRAW_OSD) || (uiItemID == eBLENDER_CLEAR_OSD))
            {
                halWarping_OsdQueueDrawSemaphoreGive();
                return rcSUCCESS;
            }

            memset((UINT8*)&sPayload, 0, sizeof(sPayload));
        }

        halWarping_OsdQueueDrawSemaphoreGive();
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d) mutex fail\n", __FUNCTION__, __LINE__);
        return rcERROR;
    }

    return rcSUCCESS;
}
#endif

#endif /* FPGA_ENTRY_4K */

eHAL_WARPING_EXEC_CODE halWarping_CurrentWarpTypeSet(UINT8 ucType)   //A35G2_Simon_0115
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {
    	m_sHalWarpingInfo.ucCurrentWarpingType = ucType;

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "%s MUTEX FAIL\n" , __FUNCTION__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//A70LK_CL_0001
//The available index for user define color pallete on Scaler OSD are 237, 238 in Christie model.
//These 2 are used for logo capture in Optoma model.
//I can use one between 17~231 on Warp OSD for user define color pallete
void halWarping_SelectCUGrid(sCOLOR_UNIFORMITY_POINT_SELECT *psCUPointSelect)
{
	uint8 i;

	m_ucCUShow = psCUPointSelect->ucShow;
	for(i = 0; i < 63; i++)
	{
	    m_pnCUSelectEnableTable[i%9][i/9] = psCUPointSelect->ucPoint[i];
	}

}

void  halWarping_DrawOsdRectBorder(uint16 nHStart, uint16 nVStart, uint16 nWidth, uint16 nHeight, UINT16 eColor, uint8 nLineWidth)  //H2PF_Simon_0037
{
	//Left
	//DrawOsdRect(OSD_LAYER_TYPE__MAIN, nHStart, nVStart, GRID_LINE_WIDTH, nHeight, eColor);
    halWarping_DrawRect(eWDT_AFTER_WARP, nHStart, nVStart, nLineWidth, nHeight, eColor);
	//Right Edge
	halWarping_DrawRect(eWDT_AFTER_WARP, nHStart+nWidth-nLineWidth, nVStart, nLineWidth, nHeight, eColor);
	//Top Edge
	halWarping_DrawRect(eWDT_AFTER_WARP, nHStart, nVStart, nWidth, nLineWidth, eColor);
	//Bottom Edge
	halWarping_DrawRect(eWDT_AFTER_WARP, nHStart, nVStart+nHeight-nLineWidth, nWidth, nLineWidth, eColor);
}

//CheckerBoard pattern
eHAL_WARPING_EXEC_CODE halWarping_DrawCheckerBoardPattern(eWARPOSD_DRAW_TYPE ucLayerMode, INT16 iRectSize)
{

    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    INT16 iHorzRes = halWarping_HResGet();
    INT16 iVertRes = halWarping_VResGet();


    LOG_MSG(db_HAL_WARPING, "%s %d size = %d,  H/V = (%d, %d)\n" ,__FUNCTION__, eWDT_AFTER_WARP ,iRectSize, iHorzRes, iVertRes);

    //update color palette
    halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);  //H2PF_Simon_0038
    halWarpOSD_Palette_RamToChip();  //H2PF_Simon_0038

    //draw full screen black color
    eResult = halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0, iHorzRes, iVertRes, AP_COLOR_IDX__BLACK );
    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
        return eResult;

    if((iHorzRes == 1920 && iVertRes== 1080) ||
       (iHorzRes == 1920 && iVertRes== 1200)  //A70LK_CL_0005 this will have same picture in 4K and 2k panel output  //H2PF_Simon_0039
    )
    {
        iRectSize = iRectSize/2;
    }

    //step 1 draw first row by half, then copy first half to 2nd half
    //step 2 copy 1st row to 2nd row by shift one rectangle size
    //step 3 copy row1 and row 2 to fullfill rest of screen.
    //step 1 draw (6 times)
    for(int x = 0 ; x < iHorzRes/4 ; x+=(iRectSize*2) )
    {
    eResult = halWarping_DrawRect(eWDT_AFTER_WARP, x, 0, iRectSize, iRectSize, AP_COLOR_IDX__WHITE);
    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
        return eResult;
    }
    //step 1 copy 1/4 to 1/2
   eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, 0, iHorzRes/4, iRectSize, iHorzRes/4,0);
    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
        return eResult;
    //copy 1/2 to row 1
   eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, 0, iHorzRes/2, iRectSize, iHorzRes/2,0);
    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
        return eResult;
    //step 2 copy row 1 to row 2
   eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, 0, (iHorzRes - iRectSize), iRectSize, iRectSize,iRectSize);
    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
        return eResult;
    //step 3 copy(13 times) by 2 rows each time
    for(int y = 0 ; y < (iVertRes/iRectSize)/2 ; y++)
    {
        eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, 0, 0, iHorzRes, iRectSize*2, 0, (y+1)*iRectSize*2);
        if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
            return eResult;
    }
    return eResult;
}

//ACU grid pattern
//target select function with white background (draw black border of target)
eHAL_WARPING_EXEC_CODE halWarping_DrawOsdAcuSelectPosition(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE_4K/2;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

#if 0
	for(j = 0; j < 7; j++)
	{
		for(i = 0; i < 9; i++)
		{
			if(m_pnCUSelectEnableTable[i][j] != 0)
			{
			        //pink as selected
			         halWarping_DrawOsdRectBorder((uint16)(HPosRatio[i]*halWarping_HResGet()) - nGridOffset, (uint16)(VPosRatio[j]*halWarping_VResGet()) - nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__MAGENTA, GRID_LINE_WIDTH);
			}
			else
			{
			    if(!m_ucCUShow)  // 0:show all point, 1:show selected point only
			         halWarping_DrawOsdRectBorder((uint16)(HPosRatio[i]*halWarping_HResGet()) - nGridOffset, (uint16)(VPosRatio[j]*halWarping_VResGet()) - nGridOffset, OSD_ACU_GRID_SIZE, OSD_ACU_GRID_SIZE, COLOR_IDX__BLACK, GRID_LINE_WIDTH);
			}
		}
	}
#else
        //draw all points  (0:show all point, 1:show selected point only)
        if(!m_ucCUShow)
        {
            //draw 1st row and copy to 2nd row
            halWarping_DrawOsdRectBorder((uint16)(HPosRatio[0]*halWarping_HResGet()) - nGridOffset, (uint16)(VPosRatio[0]*halWarping_VResGet()) - nGridOffset, OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K, AP_COLOR_IDX__GREY192, GRID_LINE_WIDTH);  //H2PF_Simon_0037
            for(i = 0; i < 9; i++)
            {
                if(i != 0)
                {
                    //copy 1st rectangle to rest rectangle of row 1
                    eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, (HPosRatio[0]*halWarping_HResGet())- nGridOffset, (VPosRatio[0]*halWarping_VResGet())- nGridOffset
                    , OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K
                    , (HPosRatio[i]*halWarping_HResGet())- nGridOffset,(VPosRatio[0]*halWarping_VResGet()) - nGridOffset);
                    if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                        return eResult;
                }
                //copy 1st row to 2nd row by each rectangle
                eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, (HPosRatio[i]*halWarping_HResGet())- nGridOffset, (VPosRatio[0]*halWarping_VResGet())- nGridOffset
                , OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K
                , (HPosRatio[i]*halWarping_HResGet())- nGridOffset,(VPosRatio[1]*halWarping_VResGet()) - nGridOffset);
                if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                    return eResult;
            }
            //copy row 1 to row 7
            eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, (HPosRatio[0]*halWarping_HResGet())- nGridOffset, (VPosRatio[0]*halWarping_VResGet())- nGridOffset
            , halWarping_HResGet()-OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K
            , (HPosRatio[0]*halWarping_HResGet())- nGridOffset,(VPosRatio[6]*halWarping_VResGet()) - nGridOffset);
            if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                return eResult;

            //copy row 2 to row 3~6
            for(i = 2; i < 6; i++)
            {
               eResult = halWarping_CopyOSD(eWDT_AFTER_WARP, (HPosRatio[0]*halWarping_HResGet())- nGridOffset, (VPosRatio[1]*halWarping_VResGet())- nGridOffset
                , halWarping_HResGet()-OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K
                , (HPosRatio[0]*halWarping_HResGet())- nGridOffset,(VPosRatio[i]*halWarping_VResGet()) - nGridOffset);
                if (eResult != eHAL_WARPING_EXEC_CODE_PASS)
                    return eResult;
            }

        }
        //draw selected
	for(j = 0; j < 7; j++)
	{
		for(i = 0; i < 9; i++)
		{
			if(m_pnCUSelectEnableTable[i][j] != 0)
			{
			        //pink as selected
			         halWarping_DrawOsdRectBorder((uint16)(HPosRatio[i]*halWarping_HResGet()) - nGridOffset, (uint16)(VPosRatio[j]*halWarping_VResGet()) - nGridOffset, OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K, AP_COLOR_IDX__BLACK, GRID_LINE_WIDTH_SEL);  //H2PF_Simon_0037
			}
		}
	}

#endif
}
//target select function with white background (draw white background)
void halWarping_DrawOsdAcuBlackBorderMultipleSelectPattern(void)
{
    //update color palette
    halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);  //H2PF_Simon_0038
    halWarpOSD_Palette_RamToChip();  //H2PF_Simon_0038

	//fill full white
    halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0, halWarping_HResGet(), halWarping_VResGet(), AP_COLOR_IDX__WHITE);
    halWarping_DrawOsdAcuSelectPosition();
}

// ==============================================================================
// Function:
// DESCRIPTION: Draw the 9x7 white blocks in the black pattern for ACU calibration.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, CL create
// --------------------
// ==============================================================================
//full acu grid with black background for ACU calibration
void  halWarping_DrawOsdAcuGridPattern(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE_4K/2;
	//A65_OPTOMA_CL_0001 use percentage * native panel resolution to get x, y coordinate instead of fix number

    //update color palette
    halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);  //H2PF_Simon_0038
    halWarpOSD_Palette_RamToChip();  //H2PF_Simon_0038

	//fill full black
	halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0, halWarping_HResGet(), halWarping_VResGet(), AP_COLOR_IDX__BLACK);  //A65_OPTOMA_CL_0002

	for(i = 0; i < 9; i++)
	{
		for(j = 0; j < 7; j++)
		{
			halWarping_DrawRect(eWDT_AFTER_WARP, (uint16)(HPosRatio[i]*halWarping_HResGet())-nGridOffset, (uint16)(VPosRatio[j]*halWarping_VResGet())-nGridOffset, OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K, AP_COLOR_IDX__WHITE);
		}
	}
}

// ==============================================================================
// Function:
// DESCRIPTION: Draw the 3 white blocks in the black pattern for ACU calibration.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, CL create
// --------------------
// ==============================================================================
//draw 3 acu grids with black background for ACU calibration
void  halWarping_DrawOsdAcuCenterGridPattern(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE_4K/2;
	//A65_OPTOMA_CL_0001 use percentage * native panel resolution to get x, y coordinate instead of fix number

     //update color palette
     halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);  //H2PF_Simon_0038
     halWarpOSD_Palette_RamToChip();  //H2PF_Simon_0038

	//fill full black
	halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0, halWarping_HResGet(), halWarping_VResGet(), AP_COLOR_IDX__BLACK);  //A65_OPTOMA_CL_0002

	for(i = 0; i < 3; i++)
	{
		halWarping_DrawRect(eWDT_AFTER_WARP, (uint16)(HCenterPosRatio[i]*halWarping_HResGet())-nGridOffset, (uint16)(VCenterPosRatio[i]*halWarping_VResGet())-nGridOffset, OSD_ACU_GRID_SIZE_4K, OSD_ACU_GRID_SIZE_4K, AP_COLOR_IDX__WHITE);
	}
}

void halWarping_ACU_Target_Status_Set(UINT8 ucValue)
{
    m_ucACU_Target_Tmp = ucValue;
}

// ==============================================================================
// Function:
// DESCRIPTION: Draw the 9x7 black borders in the white pattern for ACU target selection.
// Params: None
// Returns: None
//
// Modification History
// --------------------
// 2021/06/15, Tim create
// --------------------
// ==============================================================================
void  halWarping_DrawOsdAcuBlackBorderPattern(void)
{
	uint8 i, j;
	uint8 nGridOffset = OSD_ACU_GRID_SIZE_4K/2;
	//A65_OPTOMA_CL_0001 use percentage * native panel resolution to get x, y coordinate instead of fix number
	//uint8 acTS = appDataMgr_ACU_Target_Select_Tmp_Get();
    UINT8 aucTS[TARGET_SELECT_LEN + 1] = {0, 0, 0};
    //CL add ucShow = 0 for trigger from OSD, ap set this value from another way
    m_ucCUShow = 0;

    aucTS[0] = m_ucACU_Target_Tmp / 10;
    aucTS[1] = m_ucACU_Target_Tmp % 10;

	for(i = 0; i < 9; i++)
	{
		for(j = 0; j < 7; j++)
		{
            if( ( ( aucTS[0] - 1 ) == i ) && ( ( aucTS[1] - 1 ) == j ) )
            {
                m_pnCUSelectEnableTable[i][j] = 1;
            }
            else
            {
                m_pnCUSelectEnableTable[i][j] = 0;
            }
		}
	}

    //update color palette
    halWarping_APColorsPalette(&m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[240][0]);  //H2PF_Simon_0038
    halWarpOSD_Palette_RamToChip();
    //fill full white
    halWarping_DrawRect(eWDT_AFTER_WARP, 0, 0, halWarping_HResGet(), halWarping_VResGet(), AP_COLOR_IDX__WHITE);
    halWarping_DrawOsdAcuSelectPosition();
    halWarpOSD_DrawOSD(eWDT_AFTER_WARP);  //show warp OSD
}

void  halWarping_PrintWarpOsdColorPalette(void)
{
        LOG_MSG(db_HAL_WARPING, "WarpOSD color palette number = %d\n", m_sHalWarpingInfo.iPalette_ColorAmount);
        LOG_MSG(db_HAL_WARPING, "WarpOSD color palette inhibit color number = %d\n", m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0]);  //H2PF_Simon_0038

        for(UINT8 idx = 0 ; idx < 16 ; idx++)
        {
            LOG_MSG(db_HAL_WARPING, "WarpOSD color palette index = %d, (R,G,B) = (%d, %d, %d)\n", idx
            , m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][0], m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][1]
            , m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][2]);
        }
        for(UINT8 idx = 240 ; idx < 254 ; idx++)
        {
            LOG_MSG(db_HAL_WARPING, "WarpOSD color palette index = %d, (R,G,B) = (%d, %d, %d)\n", idx
            , m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][0], m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][1]
            , m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][2]);
        }
}
//A70LK_CL_0001 end
//A70LK_CL_0002 map RGB to index of color palette of warp OSD
UINT16 halWarping_RGBColorToWarpOsdIndex(uint32 uRGBColor)
{
    UINT16 uiErrorIdx = 256;  //can not match in scaler's OSD palette
    //0x00RRGGBB
    //m_sHalWarpingInfo.aucPalette[idx][0]  //R
    //m_sHalWarpingInfo.aucPalette[idx][1]  //G
    //m_sHalWarpingInfo.aucPalette[idx][2]  //B
    for(int idx = 0 ; idx < 256 ; idx++)
    {
        if((m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][0] == (uint8)((uRGBColor >> 0) & 0x000000FF))
            &&(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][1] == (uint8)((uRGBColor >> 8) & 0x000000FF))
            &&(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette[idx][2] == (uint8)((uRGBColor >> 16) & 0x000000FF)))
            {
                    return idx;
            }
    }

    return uiErrorIdx;
}
//A70LK_CL_0002
void halWarping_APColorsPalette(UINT8 *paucPalette)
{
    UINT8 aucApColorsPalette[9][3];  //H2PF_Simon_0037
    UINT8 ucIndex;

    for (int idx = 0; idx < 8; idx++)
    {
        ucIndex = 240+idx;
        aucApColorsPalette[idx][0] =
                (ucIndex == AP_COLOR_IDX__WHITE || ucIndex == AP_COLOR_IDX__RED
                        || ucIndex == AP_COLOR_IDX__YELLOW
                        || ucIndex == AP_COLOR_IDX__MAGENTA) ? 255 : 0;
        aucApColorsPalette[idx][1] =
                (ucIndex == AP_COLOR_IDX__WHITE || ucIndex == AP_COLOR_IDX__GREEN
                        || ucIndex == AP_COLOR_IDX__YELLOW || ucIndex == AP_COLOR_IDX__CYAN) ?
                        255 : 0;
        aucApColorsPalette[idx][2] =
                (ucIndex == AP_COLOR_IDX__WHITE || ucIndex == AP_COLOR_IDX__BLUE
                        || ucIndex == AP_COLOR_IDX__CYAN || ucIndex == AP_COLOR_IDX__MAGENTA) ?
                        255 : 0;
    }

    aucApColorsPalette[8][0] = 192;  //H2PF_Simon_0037
    aucApColorsPalette[8][1] = 192;
    aucApColorsPalette[8][2] = 192;

    memcpy(paucPalette, aucApColorsPalette, sizeof(aucApColorsPalette));
}

//A70LK_CL_0004
eHAL_WARPING_EXEC_CODE halWarping_SetScalerOsdSwitch(eSCALER_OSD_PALETTE ePaletteType)
{
    if(m_sHalWarpingInfo.ucScalerOsdSwitch == ePaletteType)
    {
        return eHAL_WARPING_EXEC_CODE_PASS;
    }
    else
    {
        m_sHalWarpingInfo.ucScalerOsdSwitch = ePaletteType;
        //this is palette group switch for Scaler OSD
        // 0 : Scaler menu
        // 1 : Draw before warp like draw png on memory, draw sprite on memory
        switch(ePaletteType)
        {
            case SCALER_OSD_PALETTE__MENU_TYPE: //scaler osd palette
//                halGui_HWInit();  //A70LK_CL_0007
                //halGui_OSD_Off();
                dvProAV_OSD_Enable(false);  //H2PF_Simon_0038
                halGui_RestorePalette();
                break;

            case SCALER_OSD_PALETTE__DRAW_TYPE: //scaler osd palette
                break;

            default:
//                halGui_HWInit();  //A70LK_CL_0007
                //halGui_OSD_Off();
                dvProAV_OSD_Enable(false);  //H2PF_Simon_0038
                halGui_RestorePalette();
                break;
        }
    }
        return eHAL_WARPING_EXEC_CODE_PASS;
}
eHAL_WARPING_EXEC_CODE halWarping_PaletteUpdateScalerOsd(UINT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)
{
    m_sHalWarpingInfo.aucScalerOsdPalette[uiIndex][0] = ucRed;
    m_sHalWarpingInfo.aucScalerOsdPalette[uiIndex][1] = ucGreen;
    m_sHalWarpingInfo.aucScalerOsdPalette[uiIndex][2] = ucBlue;

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_UpdateScalerOsd(UINT16 uiNumber)  //UPDATE scaler OSD palette
{
    LOG_MSG(db_HAL_WARPING, "\n(%s %d)\n", __FUNCTION__, __LINE__);
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;

//    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
//    {
        eDvResult &= OSD_Palette_Set(256, (UINT8 *)m_sHalWarpingInfo.aucScalerOsdPalette);  //A70LK_CL_0007
        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }
//        halWarping_SemaphoreGive(__FUNCTION__);
//    }
//    else
//    {
//        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
//        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
//    }

    return eResult;
}

void halWarping_OSDWritePaletteWithNumberScalerOsd(UINT32 *ulPalette256, UINT16 uiNumber)
{
    // vPalette256 is unsigned short color, format is RGBA5551
    unsigned int red = 0, green = 0, blue = 0;

    for (int i = 0; i < uiNumber; i++)
    {
        red   = ulPalette256[i] & 0xFF0000;
        red   = red >> 16;
        green = ulPalette256[i] & 0xFF00;
        green = green >> 8;
        blue  = ulPalette256[i] & 0x00FF;

        //LOG_MSG(db_HAL_WARPING, "%d %d %d %d\n", i , red, green, blue);

        halWarping_PaletteUpdateScalerOsd((UINT16)i, red, green, blue);
    }
    halWarpOSD_Palette_UpdateScalerOsd(uiNumber);  //write to FPGA

}

void CalculatePaletteAndTransformRGB5551ScalerOsd(sPixmapData *pasPixmap,
                                                        UINT16 uiPixmapNumber,
                                                        unsigned int *ulmapPalette255,
                                                        unsigned int *ulPalette256)  //vPalette256
{
#if 1
//	LOG_MSG(db_HAL_WARPING, "Start CalculatePaletteAndTransformRGB5551 Scaler OSD\n");

    // Init first transparency
    unsigned short transparency = 0;

    ulPalette256[255] = transparency;

    unsigned short red = 0, green = 0, blue = 0;

    for (int idx = 0; idx < uiPixmapNumber; idx++)
    {
        int nPos = 0;
        //printf("memory idx = %d,  ==>Width, Height = %d %d\n", idx, pasPixmap[idx].sPixmap_OnMemory.nWidth, pasPixmap[idx].sPixmap_OnMemory.nHeight);

        for (int i = 0; i < pasPixmap[idx].sPixmap_OnMemory.nHeight/2; i++)
        {
            for (int j = 0; j < pasPixmap[idx].sPixmap_OnMemory.nWidth/2; j++)
            {
                nPos = i * pasPixmap[idx].sPixmap_OnMemory.nWidth/2 + j;

                unsigned short usColor = (pasPixmap[idx].sPixmap_OnMemory.ucPixDataR[nPos] & 0x1F)
                                                        + ((pasPixmap[idx].sPixmap_OnMemory.ucPixDataG[nPos] & 0x1F) << 5)
                                                        + ((pasPixmap[idx].sPixmap_OnMemory.ucPixDataB[nPos] & 0x1F) << 10)
                                                        + ((pasPixmap[idx].sPixmap_OnMemory.ucPixDataR[nPos] & 0x80) << 8);

                //printf("usColor=0x%X \n",usColor);

                // Check if it is transparent or no
                if ((usColor & 0x8000) == 0)
                {
                    m_sHalWarpingInfo.ucScalerOsdPixIndexed[nPos] = transparency;
                    //printf("transparency...(pos = %d) usColor =%x\n", nPos, usColor);
                }
		else
		{
                    int findindex=0 ;

                    for(findindex=1 ; findindex<255 ; findindex++)  //A35G2_CDS_Simon_0045
                    {
                        if (ulmapPalette255[findindex] == usColor)
                        {
                            m_sHalWarpingInfo.ucScalerOsdPixIndexed[nPos] = (unsigned char)findindex + 1;
                            break;
                        }
                    }

	                if( findindex < 255 )   //find
	                {
	                    // cout<<"Find, the value is"<<iter->second<<endl;
	                  //  printf("Find, the value is %d\n", iter->second);
                        //printf("find (pos = %d) usColor=%x (index = %d)\n", nPos, usColor , findindex + 1);
	                    m_sHalWarpingInfo.ucScalerOsdPixIndexed[nPos] = (unsigned char)findindex + 1;
	                }
	                else
	                {
	                    // cout<<"Do not Find"<<endl;
                        m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount++;

	                    if (m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount < 256)
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

 	                        //LOG_MSG(db_HAL_WARPING, "rgb%d(%d %d %d)\n",m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount, red, green, blue);

	                        // Add new color
	                        ulmapPalette255[m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount] = usColor;
	                        m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount++;

                            ulPalette256[m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount] = color32 ;
	                        // Transform color
	                        m_sHalWarpingInfo.ucScalerOsdPixIndexed[nPos] = (unsigned char)ulPalette256[m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount];

	                    }
	                    else
	                    {
	                        //LOG_MSG(db_HAL_WARPING, "Error! Ran out color palette. It cannot be drawn\n");
	                        LOG_MSG(db_HAL_WARPING, "OC\n");
	                        m_sHalWarpingInfo.ucScalerOsdPixIndexed[nPos] = 0;
	                    }
	                }
		}
            }
        }

        //LOG_MSG(db_HAL_WARPING, "idx %d , Total Pixel = %d\n" , idx , nPos);
    }


	//LOG_MSG(db_HAL_WARPING, "End %s (%d %d)\n", __FUNCTION__, m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount, m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount);
#endif /* 0 */

}

eHAL_WARPING_EXEC_CODE halWarping_DrawSpriteBufferScalerOsd(int startX,
                                             int startY,
                                             int width,
                                             int height,
                                             unsigned char *paucBlend,
                                             sPixmapData *pasPixmap)
{
    //LOG_MSG(db_HAL_WARPING, "Start %s (%d %d %d %d)\n", __FUNCTION__, startX, startY, width, height);

    int xEnd = startX + width;
    xEnd=(xEnd < halWarping_HResGet()/2)? xEnd:halWarping_HResGet()/2;

    int yEnd = startY + height;
    yEnd=(yEnd < halWarping_VResGet()/2)? yEnd:halWarping_VResGet()/2;


#if 1  //draw on dram of Scaler OSD copy from DrawPNG function
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    int xPos = startX;
    int yPos = startY;
    int ulWidth = width;
    int ulHeight = height;

    INT32 m_nHorzResolution = halWarping_HResGet()/2;  //A70LK_Simon_0001
    INT32 m_nVertResolution = halWarping_VResGet()/2;

    UINT8 ucTmpData[m_nHorzResolution];
    //memset(ucTmpData, 0, m_nHorzResolution);

if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
{
    //LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        INT32 tmpWidth  = (xPos + ulWidth)  > m_nHorzResolution ? m_nHorzResolution : ulWidth  + xPos;
        INT32 tmpHeight = (yPos + ulHeight) > m_nVertResolution ? m_nVertResolution : ulHeight + yPos;

        INT32 lIndex = 0 ;

        for (int y = yPos; y < tmpHeight; y++)
        {
            //memset(ucTmpData, 0x10, sizeof(ucTmpData)); // nInhibitColor
//            memset(ucTmpData, 0xFF, sizeof(ucTmpData)); //transparent
            memset(ucTmpData, 0, sizeof(ucTmpData)); //palette index 0 as black
            for (int x = xPos; x < tmpWidth; x++)
            {
                if(x >= 0 && y >= 0)
                    ucTmpData[x] = paucBlend[lIndex];
/*
                ucSpriteDataBufferRGB[y*m_nHorzResolution + x] = pasPixmap.sPixmap_OnMemory.ucPixDataR[y*m_nHorzResolution + x] & 0x1F)
                                                        + ((pasPixmap.sPixmap_OnMemory.ucPixDataG[y*m_nHorzResolution + x] & 0x1F) << 5)
                                                        + ((pasPixmap.sPixmap_OnMemory.ucPixDataB[y*m_nHorzResolution + x] & 0x1F) << 10);
*/
                ucSpriteDataBuffer[y*m_nHorzResolution + x] = ucTmpData[x];

                //printf("x,y = (%d, %d) nX,nY = (%d, %d),  ColorIdx : %d, value = %d \n", x, y,nXPos, nYPos, lIndex, paucBlend[lIndex]);
                lIndex++ ;
            //if(y == tmpHeight -1)
                //printf("x,y = (%d, %d)  ColorIdx : %d, value = %d \n", x, y,lIndex, paucBlend[lIndex]);
            }

        }

        halWarping_SemaphoreGive(__FUNCTION__);
        //LOG_MSG(db_HAL_WARPING, "(%s %d) PWPOSD_SAD_AFTER = 0x%08x\n", __FUNCTION__, __LINE__, DRAM_WRPOSD_MEM3_ADDR);

        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }

}
#endif

    //LOG_MSG(db_HAL_WARPING, "End %s\n", __FUNCTION__);
    return eResult;

}

void halWarping_ClearSpriteBuffer(void)
{
        memset(ucSpriteDataBuffer, 0x00, sizeof(ucSpriteDataBuffer)); // transparent color
//        memset(ucSpriteDataBufferRGB, 0x0000, sizeof(ucSpriteDataBufferRGB)); // transparent color
}

eHAL_WARPING_EXEC_CODE halWarping_ShowSpriteBuffer(eWARPOSD_DRAW_TYPE eDrawType)
{
    UINT32 ulSpriteOsdAddress;
    eRESULT eDvResult = rcSUCCESS;
    eHAL_WARPING_EXEC_CODE eResult = eHAL_WARPING_EXEC_CODE_FAIL;
    INT32 m_nHorzResolution = halWarping_HResGet();  //A70LK_Simon_0001
    INT32 m_nVertResolution = halWarping_VResGet();

    LOG_MSG(db_HAL_WARPING, "Start %s (draw type = %d, (h,v) = (%d, %d))\n", __FUNCTION__, eDrawType, m_nHorzResolution, m_nVertResolution);

//update palette
//    if(eDrawType == eWDT_BEFORE_WARP)
//    {
        //update Scaler OSD palette
//        halWarping_OSDWritePaletteWithNumberScalerOsd(m_sHalWarpingInfo.aulScalerOsdSpritePalette, m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount + 1);
//    }

//draw
if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
{

    if(eDrawType == eWDT_BEFORE_WARP)  //on scaler OSD
    {
        halWarping_OSDWritePaletteWithNumberScalerOsd(m_sHalWarpingInfo.aulScalerOsdSpritePalette, m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount + 1);
        //debug to show value of sprite on memory
        /*
        printf("\n=====> SHOW VALUE <=====\n");
        for(int i = 0; i <m_nVertResolution/2; i++)
        {
            printf("index (%d) value = 0x%x, ", i, ucSpriteDataBuffer[i]);
            if(((m_nVertResolution/2) % (m_nVertResolution/20)) == 0)  // 108 character as one line
                printf("\n");
        }

        printf("\n=====> END <=====\n");
        */
        ulSpriteOsdAddress = dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE_0);
#ifdef FPGA_ENTRY_4K
        eDvResult &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_0);
#else
        eDvResult &= dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE_0);
#endif /* FPGA_ENTRY_4K */
        for(int i = 0; i <m_nVertResolution/2; i++ )
            eDvResult &= dvProAV_LineBufWrite(eDramBank0, ulSpriteOsdAddress + i * (m_nHorzResolution /2/64), (m_nHorzResolution /2/64), &ucSpriteDataBuffer[i*(m_nHorzResolution/2)]);

        if(eDvResult == rcSUCCESS)
        {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
        }
    }
    else
    {
            eResult = eHAL_WARPING_EXEC_CODE_PASS;
    }

        halWarping_SemaphoreGive(__FUNCTION__);

}

    halWarping_ClearSpriteBuffer();
    LOG_MSG(db_HAL_WARPING, "End %s\n", __FUNCTION__);

    return eResult;

}

void halWarping_SetShowScalerOsdAck(UINT8 ucShow)
{
    m_sHalWarpingInfo.ucScalerOsdDrawShow = ucShow;
}

UINT8 halWarping_GetShowScalerOSDAck(void)
{
    return m_sHalWarpingInfo.ucScalerOsdDrawShow;
}
/*
w1 - the width of the original image
h1 - the height of the original image
pixels - an array of int with the pixel data
w2 - desired width
h2 - desired height
retval - this is the returned value, it is a new pixel array which contains the manipulated image.
*/
void halWarping_resizePixels(UINT8* retval, UINT8* pixels,int w1,int h1,int w2,int h2)
{
    // EDIT: added +1 to remedy an early rounding problem
    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;

    int x2, y2 ;
    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            retval[(i*w2)*3+j*3 + 0] = pixels[(y2*w1)*3+x2*3 + 0] ;  //Red
            retval[(i*w2)*3+j*3 + 1] = pixels[(y2*w1)*3+x2*3 + 1] ;  //Green
            retval[(i*w2)*3+j*3 + 2] = pixels[(y2*w1)*3+x2*3 + 2] ;  //Blue
        }
    }
}

eHAL_WARPING_EXEC_CODE halWarping_ShowSpriteOnMemory(void)
{
        //scaler osd is 1920x1080
        //printf("==>sprite memory counts = %d\n", m_sHalWarpingInfo.uiScalerOsdRectCount);
        unsigned short red = 0, green = 0, blue = 0;
        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();


        if(halWarping_HResGet()>1920 && halWarping_VResGet() > 1080)
        {
            m_nHorzResolution = m_nHorzResolution/2;
            m_nVertResolution = m_nVertResolution/2;
        }

/*
        for(int i = 0; i< m_sHalWarpingInfo.uiScalerOsdRectCount; i++)
        {
            printf("==>layer %d, x = %d, y = %d, width = %d, height = %d, color = 0x%x\n"
                , m_sHalWarpingInfo.asScalerOsdRect[i].ucLayerMode
                , m_sHalWarpingInfo.asScalerOsdRect[i].iX
                , m_sHalWarpingInfo.asScalerOsdRect[i].iY
                , m_sHalWarpingInfo.asScalerOsdRect[i].iWidth
                , m_sHalWarpingInfo.asScalerOsdRect[i].iHeight
                , m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex);
        }
*/

        //color palette
        for(int i = 0; i< m_sHalWarpingInfo.uiScalerOsdRectCount; i++)
        {
            int findindex=0 ;
            unsigned short usColor = (unsigned short)m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex;

            for(findindex=1 ; findindex<255 ; findindex++)
            {
                if (m_sHalWarpingInfo.aulScalerOsdMapSpritePalette[findindex] == usColor)
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex = (unsigned char)findindex + 1;
                    //printf("find color index = %d\n",m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex);
                    break;
                }
            }

            if( findindex < 255 )   //find
            {
                m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex = (unsigned char)findindex + 1;
                //printf("find color index = %d\n",m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex);
            }
            else
            {
                m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount++;

                if (m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount < 256)
                {
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

                    // Add new color
                    m_sHalWarpingInfo.aulScalerOsdMapSpritePalette[m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount] = usColor;
                    m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount++;

                    m_sHalWarpingInfo.aulScalerOsdSpritePalette[m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount] = color32 ;
                    // Transform color
                    m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex = (unsigned char)m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount;
                    //printf("assign new color index = %d\n",m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex);
                    //printf("color palette count = %d\n",m_sHalWarpingInfo.uiScalerOsdMapSpritePaletteCount);

                }
                else
                {
                    //LOG_MSG(db_HAL_WARPING, "Error! Ran out color palette. It cannot be drawn\n");
//                    LOG_MSG(db_HAL_WARPING, "OC\n");
                    m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex = 255;
                    //printf("run out of color palette, assign 255 to color\n");
                }
            }
        }

		//A70LK_Casper_0004
        //find current index of twist test pattern and write it to index 0
        //red    green  blue
//    { 0x00, 0x00, 0x00 }, // black
//    { 0xFF, 0xFF, 0xFF }, // white
//    { 0x7F, 0x7F, 0x7F }  // Grey
//    (0 : off, 1 : grid, 2 : white, 3 : black, 4 : checker board, 5 : color bars, 18 : gray
        UINT8 ucTestPattern = halWarping_GetShowScalerOSDAck();
        if(ucTestPattern == 2)
        {
            //white
            m_sHalWarpingInfo.aulScalerOsdSpritePalette[0] = 0x00ffffff;
        }
        else if (ucTestPattern == 18)
        {
            //grey
            m_sHalWarpingInfo.aulScalerOsdSpritePalette[0] = 0x007f7f7f;
        }
        else
        {
            //black
            m_sHalWarpingInfo.aulScalerOsdSpritePalette[0] = 0;
        }

        //halGui_OSD_Off();  //A70LK_CL_0007
        dvProAV_OSD_Enable(false);  //H2PF_Simon_0038

        if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
        {
            halWarping_OSDWritePaletteWithNumberScalerOsd(m_sHalWarpingInfo.aulScalerOsdSpritePalette, m_sHalWarpingInfo.uiScalerOsdSpritePaletteCount + 1);
            halWarping_SemaphoreGive(__FUNCTION__);
        }
        else
        {
            LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
            return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
        }
        //draw

        //background
        halWarping_DrawRect(eWDT_BEFORE_WARP,
            0,
            0,
            m_nHorzResolution,
            m_nVertResolution,
            0);  //use black pattern or current one

        //data
        for(int i = 0; i< m_sHalWarpingInfo.uiScalerOsdRectCount; i++)
        {
            if(m_sHalWarpingInfo.asScalerOsdRect[i].iX < 0)
            {
                if(m_sHalWarpingInfo.asScalerOsdRect[i].iWidth + m_sHalWarpingInfo.asScalerOsdRect[i].iX >0)
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iWidth += m_sHalWarpingInfo.asScalerOsdRect[i].iX;
                }
                else
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iWidth = 0;
                }
                m_sHalWarpingInfo.asScalerOsdRect[i].iX = 0;
            }
            if(m_sHalWarpingInfo.asScalerOsdRect[i].iX + m_sHalWarpingInfo.asScalerOsdRect[i].iWidth > m_nHorzResolution)
            {
                if(m_nHorzResolution - m_sHalWarpingInfo.asScalerOsdRect[i].iX > 0)
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iWidth = m_nHorzResolution - m_sHalWarpingInfo.asScalerOsdRect[i].iX;
                }
                else
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iWidth = 0;
                }
            }
            if(m_sHalWarpingInfo.asScalerOsdRect[i].iY < 0)
            {
                if(m_sHalWarpingInfo.asScalerOsdRect[i].iHeight + m_sHalWarpingInfo.asScalerOsdRect[i].iY > 0)
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iHeight += m_sHalWarpingInfo.asScalerOsdRect[i].iY;
                }
                else
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iHeight = 0;
                }
                m_sHalWarpingInfo.asScalerOsdRect[i].iY = 0;
            }
            if(m_sHalWarpingInfo.asScalerOsdRect[i].iY + m_sHalWarpingInfo.asScalerOsdRect[i].iHeight > m_nVertResolution)
            {
                if(m_nVertResolution - m_sHalWarpingInfo.asScalerOsdRect[i].iY > 0)
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iHeight = m_nVertResolution - m_sHalWarpingInfo.asScalerOsdRect[i].iY;
                }
                else
                {
                    m_sHalWarpingInfo.asScalerOsdRect[i].iHeight = 0;
                }
            }

            halWarping_DrawRect(m_sHalWarpingInfo.asScalerOsdRect[i].ucLayerMode,
                m_sHalWarpingInfo.asScalerOsdRect[i].iX,
                m_sHalWarpingInfo.asScalerOsdRect[i].iY,
                m_sHalWarpingInfo.asScalerOsdRect[i].iWidth,
                m_sHalWarpingInfo.asScalerOsdRect[i].iHeight,
                m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex);
/*
            printf("Modify==>layer %d, x = %d, y = %d, width = %d, height = %d, color = %d\n"
                , m_sHalWarpingInfo.asScalerOsdRect[i].ucLayerMode
                , m_sHalWarpingInfo.asScalerOsdRect[i].iX
                , m_sHalWarpingInfo.asScalerOsdRect[i].iY
                , m_sHalWarpingInfo.asScalerOsdRect[i].iWidth
                , m_sHalWarpingInfo.asScalerOsdRect[i].iHeight
                , m_sHalWarpingInfo.asScalerOsdRect[i].iColorIndex);
*/

        }
//        memset(m_sHalWarpingInfo.asScalerOsdRect, 0, sizeof(sWARPOSD_RECT_DRAW_INFO)*m_sHalWarpingInfo.uiScalerOsdRectCount);
        m_sHalWarpingInfo.uiScalerOsdRectCount = 0;

    return eHAL_WARPING_EXEC_CODE_PASS;

}




void halWarping_ShowSpriteOnOSD(void)
{
        //printf("==>sprite osd counts = %d\n", m_sHalWarpingInfo.uiWarpOsdRectCount);
        unsigned short red = 0, green = 0, blue = 0;
        INT32 m_nHorzResolution = halWarping_HResGet();
        INT32 m_nVertResolution = halWarping_VResGet();

        if(dvProAV_OSD_4k3dEn_Get() && halWarping_PanelGet() == PANEL_2D_HIGHSPEED)
        {
            m_nHorzResolution *= 2;
            m_nVertResolution *= 2;
        }
/*
        for(int i = 0; i< m_sHalWarpingInfo.uiWarpOsdRectCount; i++)
        {
            printf("==>layer %d, x = %d, y = %d, width = %d, height = %d, color = 0x%x\n"
                , m_sHalWarpingInfo.asWarpOsdRect[i].ucLayerMode
                , m_sHalWarpingInfo.asWarpOsdRect[i].iX
                , m_sHalWarpingInfo.asWarpOsdRect[i].iY
                , m_sHalWarpingInfo.asWarpOsdRect[i].iWidth
                , m_sHalWarpingInfo.asWarpOsdRect[i].iHeight
                , m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex);
        }
*/
        //color palette
        for(int i = 0; i< m_sHalWarpingInfo.uiWarpOsdRectCount; i++)
        {
            int findindex=0 ;
            unsigned short usColor = (unsigned short)m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex;

            //printf("usColor = 0x%x\n",usColor);

            for(findindex=1 ; findindex<255 ; findindex++)
            {
                if (m_sHalWarpingInfo.aulMapSpritePalette[findindex] == usColor)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex = (unsigned char)findindex + 1;
                    //printf("find color index = %d\n",m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex);
                    break;
                }
            }

            if( findindex < 255 )   //find
            {
                m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex = (unsigned char)findindex + 1;
                //printf("find color index = %d\n",m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex);
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

                    //printf("rgb%d(%d %d %d), 0x%x\n",m_sHalWarpingInfo.uiSpritePaletteCount, red, green, blue, color32);

                    // Add new color
                    m_sHalWarpingInfo.aulMapSpritePalette[m_sHalWarpingInfo.uiMapSpritePaletteCount] = usColor;
                    m_sHalWarpingInfo.uiMapSpritePaletteCount++;

                    m_sHalWarpingInfo.aulSpritePalette[m_sHalWarpingInfo.uiSpritePaletteCount] = color32 ;
                    // Transform color
//                    m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex = (unsigned char)m_sHalWarpingInfo.aulSpritePalette[m_sHalWarpingInfo.uiSpritePaletteCount];
                    m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex = (unsigned char)m_sHalWarpingInfo.uiSpritePaletteCount;
                    //printf("assign new color index = %d\n",m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex);
                    //printf("color palette count = %d\n",m_sHalWarpingInfo.uiMapSpritePaletteCount);

                }
                else
                {
                    //LOG_MSG(db_HAL_WARPING, "Error! Ran out color palette. It cannot be drawn\n");
                    //LOG_MSG(db_HAL_WARPING, "OC\n");
                    m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex = 255;  //as transparent
                    printf("run out of color palette, assign 255 to color\n");
                }
            }
        }
        halWarping_OSDWritePaletteWithNumber(m_sHalWarpingInfo.aulSpritePalette, m_sHalWarpingInfo.uiSpritePaletteCount + 1);

        //draw

        //background
        halWarping_DrawRect(eWDT_AFTER_WARP,
            0,
            0,
            m_nHorzResolution,
            m_nVertResolution,
            255);


        for(int i = 0; i< m_sHalWarpingInfo.uiWarpOsdRectCount; i++)
        {
            if(m_sHalWarpingInfo.asWarpOsdRect[i].iX < 0)
            {
                if(m_sHalWarpingInfo.asWarpOsdRect[i].iWidth + m_sHalWarpingInfo.asWarpOsdRect[i].iX >0)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iWidth += m_sHalWarpingInfo.asWarpOsdRect[i].iX;
                }
                else
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iWidth = 0;
                }
                m_sHalWarpingInfo.asWarpOsdRect[i].iX = 0;
            }
            if(m_sHalWarpingInfo.asWarpOsdRect[i].iX + m_sHalWarpingInfo.asWarpOsdRect[i].iWidth > m_nHorzResolution)
            {
                if(m_nHorzResolution - m_sHalWarpingInfo.asWarpOsdRect[i].iX > 0)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iWidth = m_nHorzResolution - m_sHalWarpingInfo.asWarpOsdRect[i].iX;
                }
                else
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iWidth = 0;
                }
            }
            if(m_sHalWarpingInfo.asWarpOsdRect[i].iY < 0)
            {
                if(m_sHalWarpingInfo.asWarpOsdRect[i].iHeight + m_sHalWarpingInfo.asWarpOsdRect[i].iY > 0)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iHeight += m_sHalWarpingInfo.asWarpOsdRect[i].iY;
                }
                else
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iHeight = 0;
                }
                m_sHalWarpingInfo.asWarpOsdRect[i].iY = 0;
            }
            if(m_sHalWarpingInfo.asWarpOsdRect[i].iY + m_sHalWarpingInfo.asWarpOsdRect[i].iHeight > m_nVertResolution)
            {
                if(m_nVertResolution - m_sHalWarpingInfo.asWarpOsdRect[i].iY > 0)
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iHeight = m_nVertResolution - m_sHalWarpingInfo.asWarpOsdRect[i].iY;
                }
                else
                {
                    m_sHalWarpingInfo.asWarpOsdRect[i].iHeight = 0;
                }
            }

            halWarping_DrawRect(m_sHalWarpingInfo.asWarpOsdRect[i].ucLayerMode,
                m_sHalWarpingInfo.asWarpOsdRect[i].iX,
                m_sHalWarpingInfo.asWarpOsdRect[i].iY,
                m_sHalWarpingInfo.asWarpOsdRect[i].iWidth,
                m_sHalWarpingInfo.asWarpOsdRect[i].iHeight,
                m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex);
/*
            printf("Modify==>layer %d, x = %d, y = %d, width = %d, height = %d, color = %d\n"
                , m_sHalWarpingInfo.asWarpOsdRect[i].ucLayerMode
                , m_sHalWarpingInfo.asWarpOsdRect[i].iX
                , m_sHalWarpingInfo.asWarpOsdRect[i].iY
                , m_sHalWarpingInfo.asWarpOsdRect[i].iWidth
                , m_sHalWarpingInfo.asWarpOsdRect[i].iHeight
                , m_sHalWarpingInfo.asWarpOsdRect[i].iColorIndex);
*/

        }
//        memset(m_sHalWarpingInfo.asWarpOsdRect, 0, sizeof(sWARPOSD_RECT_DRAW_INFO)*m_sHalWarpingInfo.uiWarpOsdRectCount);
        m_sHalWarpingInfo.uiWarpOsdRectCount = 0;

}

void halWarping_OPDEvent(UINT8 *aucString)  //A35G2_Simon_0120
{
    uOPD_DATA uOPDData;
    memset(&uOPDData, 0x00, sizeof(uOPD_DATA));

    snprintf(uOPDData.cString, 256, "%s", aucString);

    utilOPD_EventSet(eOPD_GEOMETRY_LOG, &uOPDData);
}

//UINT8 halWarping_Current_OSD_Type(void)
//{
//    return m_Current_OSD_Tyoe;
//}

BOOL halWarping_IsDraw_OSD(eWARPOSD_DRAW_TYPE eDrawType)
{
    LOG_MSG(db_HAL_RESERVED16, "<%d-%d>\r\n",eDrawType, m_sHalWarpingInfo.ucDrawOSD[eDrawType] );
    return m_sHalWarpingInfo.ucDrawOSD[eDrawType];
}

void halWarping_IsDraw_OSD_Set(eWARPOSD_DRAW_TYPE eDrawType, UINT8 IsDraw)   //H2PF_Simon_0040
{
    if(eDrawType >= eWDT_INVALID)
    {
        LOG_MSG(db_HAL_WARPING, "%s Fatal Error (%d %d)\r\n", __FUNCTION__, eDrawType, IsDraw );
        return ;
    }

    m_sHalWarpingInfo.ucDrawOSD[eDrawType] = IsDraw;

    //write into shared memory
    SHM_DRAW_WARPOSD_STATE(eDrawType) = IsDraw;
}

void halWarping_Set_Draw_OSD_On(UINT8 ucAckDone)
{// R70K_Bruce_0022
    //printf("Function : %s, Line number : %d, set ucAckDone = %d\n", __FUNCTION__, __LINE__, ucAckDone);
    ucDrawOSDAckDone = ucAckDone;
}

UINT8 halWarping_Get_Draw_OSD_On(void)
{// R70K_Bruce_0022
    //printf("Function : %s, Line number : %d, get ucAckDone = %d\n", __FUNCTION__, __LINE__, ucDrawOSDAckDone);
    return ucDrawOSDAckDone;
}


eHAL_WARPING_EXEC_CODE halWarpOSD_Draw_InsertLocation(eWARPOSD_DRAW_TYPE eDrawType)
{
    if(eDrawType == eWDT_BEFORE_WARP)
    {
        //dvC789_SetOSDMode(TRUE);
    }
    else
    {
        //dvC789_SetOSDMode(FALSE);
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

#if 1
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
                    if(eInterface == eGCI_AP)
                    {
                        halWarping_OSDWritePalette_AfterWarpOsd(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
                    }
                    else
                    {
                        halWarping_OSDWritePalette_BeforeWarpOsd(psWarpOSDInfo->Palette);
                    }
                }
            }
            break;

        case eWOC_Palette_All:
            memcpy(psWarpOSDInfo->Palette, Data , WARP_OSD_TOTAL_PALETTE*WARP_OSD_PALETTE_ELEMENT);

            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                if(eInterface == eGCI_AP)
                {
                    halWarping_OSDWritePalette_AfterWarpOsd(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
                }
                else
                {
                    halWarping_OSDWritePalette_BeforeWarpOsd(psWarpOSDInfo->Palette);
                }
            }
            break;

        case eWOC_Palette_RamDataToChip:
            {
                if(eInterface == eGCI_AP)
                {
                    halWarping_OSDWritePalette_AfterWarpOsd(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
                }
                else
                {
                    halWarping_OSDWritePalette_BeforeWarpOsd(psWarpOSDInfo->Palette);
                }
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

            #if 0
            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
            }
            #endif
            break;

        case eWOC_BlinkPaletteSet0_1:
            memcpy(psWarpOSDInfo->ucBlinkColorSet0+WARP_OSD_PALETTE_ELEMENT, Data , WARP_OSD_PALETTE_ELEMENT);

            #if 0
            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
            }
            #endif
            break;

        case eWOC_BlinkPaletteSet1_0:
            memcpy(psWarpOSDInfo->ucBlinkColorSet1, Data , WARP_OSD_PALETTE_ELEMENT);

            #if 0
            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
            }
            #endif
            break;

        case eWOC_BlinkPaletteSet1_1:
            memcpy(psWarpOSDInfo->ucBlinkColorSet1+WARP_OSD_PALETTE_ELEMENT, Data , WARP_OSD_PALETTE_ELEMENT);

            #if 0
            if(eAction == eWRTIE_TO_BUFFER || eAction == eWRTIE_TO_CHIP)
            {
                halWarping_OSDWritePalette(psWarpOSDInfo->Palette);  //H2PF_Simon_0038
            }
            #endif
            break;


        default:
            LOG_MSG(db_ALWAYS, "(func:%s.line:%d) undefined eConfig %d\n", __FUNCTION__, __LINE__, eConfig);
            break;
    }
#endif

    return eHAL_WARPING_EXEC_CODE_PASS;
}

ePANEL_ID halWarpOSD_PanelID_Get(void)
{
    return m_sHalWarpingInfo.ePanelTimingId;
}

eHAL_WARPING_EXEC_CODE halWarpOSD_Reload_AP_Config(eWARPOSD_RELOAD_TYPE eType)
{
    if(eType == eWOR_Palette || eType == eWOR_ALL)
    {
        halWarping_OSDWritePalette_AfterWarpOsd(m_sHalWarpingInfo.sAp_WarpOSD_Info.Palette);
    }

    if(eType == eWOR_Blink || eType == eWOR_ALL)
    {
    }

    if(eType == eWOR_Transparent || eType == eWOR_ALL)
    {
        halWarping_OSDWriteTransparentColor(m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColorEnable,
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[0],
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[1],
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[2],
                                            m_sHalWarpingInfo.sAp_WarpOSD_Info.TransparentColor[3]);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

void halWarpOSD_Reload_OSD_Config(eWARPOSD_RELOAD_TYPE eType)
{
    if(eType == eWOR_Palette || eType == eWOR_ALL)
    {
        halWarping_OSDWritePalette_BeforeWarpOsd(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette);
    }

    if(eType == eWOR_Blink || eType == eWOR_ALL)
    {
    }

    if(eType == eWOR_Transparent || eType == eWOR_ALL)
    {
        halWarping_OSDWriteTransparentColor(m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColorEnable,
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[0],
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[1],
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[2],
                                            m_sHalWarpingInfo.sOSD_WarpOSD_Info.TransparentColor[3]);
    }

}

//實際寫入chip
eHAL_WARPING_EXEC_CODE halWarping_OSDWritePalette_AfterWarpOsd(UINT8 Palette[][WARP_OSD_PALETTE_ELEMENT])  //H2PF_Simon_0038
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

#ifdef FPGA_ENTRY_4K
        dvProAV_WarpOSD_Palette_Set_E4K((INT16)256, (UINT8 *)Palette);
#else
        dvProAV_WarpOSD_Palette_Set((INT16)256, (UINT8 *)m_sHalWarpingInfo.aucPalette);
#endif /* FPGA_ENTRY_4K */

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

//????chip (write Scaler OSD)
eHAL_WARPING_EXEC_CODE halWarping_OSDWritePalette_BeforeWarpOsd(UINT8 Palette[][WARP_OSD_PALETTE_ELEMENT])  //H2PF_Simon_0038
{
    if(TRUE == halWarping_SemaphoreTake(__FUNCTION__))
    {

#ifdef FPGA_ENTRY_4K
        OSD_Palette_Set(256, (UINT8 *)Palette);
#else
        dvProAV_WarpOSD_Palette_Set((INT16)256, (UINT8 *)m_sHalWarpingInfo.aucPalette);
#endif /* FPGA_ENTRY_4K */

        halWarping_SemaphoreGive(__FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_WARPING, "\n!!!(%s %d)\n", __FUNCTION__, __LINE__);
        return eHAL_WARPING_EXEC_CODE_MUTEX_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;

}

//實際寫入chip
//write TransparentColor index only when (bEnable == 1)
eHAL_WARPING_EXEC_CODE halWarping_OSDWriteTransparentColor(BOOL bEnable, UINT8 PLTIndex0, UINT8 PLTIndex1, UINT8 PLTIndex2, UINT8 PLTIndex3)  //H2PF_Simon_0038
{
    LOG_MSG(db_HAL_WARPING, "(func:%s, line:%d) Enable %d, Transparent Color[%d %d %d %d]\r\n",__FUNCTION__
                                                                                              ,__LINE__
                                                                                              , bEnable
                                                                                              , PLTIndex0
                                                                                              , PLTIndex1
                                                                                              , PLTIndex2
                                                                                              , PLTIndex3);

    INT32 status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eWrpOsdTranColor0, PLTIndex0);
    //status &= dvProAV_AccessWrite(eWrpOsdTranColor1, PLTIndex1);
    //status &= dvProAV_AccessWrite(eWrpOsdTranColor2, PLTIndex2);
    //status &= dvProAV_AccessWrite(eWrpOsdTranColor3, PLTIndex3);

    status &= dvProAV_AccessWrite(eOsdTranColor0, PLTIndex0);
    //status &= dvProAV_AccessWrite(eOsdTranColor1, PLTIndex1);
    //status &= dvProAV_AccessWrite(eOsdTranColor2, PLTIndex2);
    //status &= dvProAV_AccessWrite(eOsdTranColor3, PLTIndex3);

    if(status != rcSUCCESS)
    {
        return eHAL_WARPING_EXEC_CODE_FAIL;
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//實際寫入chip
//write Blink time/cycle only when (bEnable == 1)
eHAL_WARPING_EXEC_CODE halWarping_OSDWriteBlink(BOOL bEnable, UINT8 ucTime, UINT8 ucCycle, BOOL FlushToChip)  //H2PF_Simon_0038
{
    LOG_MSG(db_HAL_WARPING, "!! OSD Blink not available (func:%s, line:%d) Enable %d, Blink[%d %d]\r\n",__FUNCTION__
                                                                            ,__LINE__
                                                                            , bEnable
                                                                            , ucTime
                                                                            , ucCycle);


    return eHAL_WARPING_EXEC_CODE_PASS;
}

//僅寫入記憶體
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_SetToRam(INT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)  //H2PF_Simon_0038
{
    sPaletteSingleInfo sPlt = {uiIndex, ucRed, ucGreen, ucBlue};

    halWarpOSD_Config(eGCI_AP, eWOC_Palette_Single, &sPlt, eWRTIE_TO_RAM);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//將記憶體資料寫入chip
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_RamToChip(void)  //H2PF_Simon_0038
{
    halWarpOSD_Config(eGCI_AP, eWOC_Palette_RamDataToChip, NULL, eWRTIE_TO_CHIP);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//同時寫入ram和chip
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_Set(INT16 uiIndex, UINT8 ucRed, UINT8 ucGreen, UINT8 ucBlue)  //H2PF_Simon_0038
{
    sPaletteSingleInfo sPlt = {uiIndex, ucRed, ucGreen, ucBlue};

    halWarpOSD_Config(eGCI_AP, eWOC_Palette_Single, &sPlt, eWRTIE_TO_CHIP);

    return eHAL_WARPING_EXEC_CODE_PASS;
}

//同時寫入ram和chip
eHAL_WARPING_EXEC_CODE halWarpOSD_Palette_SetAll(UINT8 *aucRed, UINT8 *aucGreen, UINT8 *aucBlue)  //H2PF_Simon_0038
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

void halWarpOSD_OSD_PLT_Set(UINT8 *Palette, UINT16 StartIdx, UINT16 Number, eWARP_OSD_SET_ACTION eAction)
{
    memcpy(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette+(StartIdx*WARP_OSD_PALETTE_ELEMENT), Palette, (Number*WARP_OSD_PALETTE_ELEMENT));

    if(eAction == eWRTIE_TO_BUFFER)
    {
        halWarping_OSDWritePalette_BeforeWarpOsd(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette);
    }
    else if(eAction == eWRTIE_TO_CHIP)
    {
        halWarping_OSDWritePalette_BeforeWarpOsd(m_sHalWarpingInfo.sOSD_WarpOSD_Info.Palette);
    }

}

eHAL_WARPING_EXEC_CODE halWarpOSD_AP_Color_Initial( UINT8 ucInitPalette )
{
    halWarpOSD_Reload_AP_Config(eWOR_Blink);
    halWarpOSD_Reload_AP_Config(eWOR_Transparent);

    //init 256 color palettes for blender
    if(ucInitPalette)
    {
        halWarpOSD_Reload_AP_Config(eWOR_Palette);
    }

    return eHAL_WARPING_EXEC_CODE_PASS;
}

eHAL_WARPING_EXEC_CODE halWarping_TransferCtrl_Through(void)
{
    return eHAL_WARPING_EXEC_CODE_PASS;
}


UINT16 halWarping_DEF_WP_SPACE_Get(void)
{
    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        return (UINT16)DEF_WP_SPACE_4K;
    }

    return (UINT16)DEF_WP_SPACE_2K;

}

UINT16 halWarping_DEF_WP_SPACE_BITS_Get(void)
{
    if(m_sHalWarpingInfo.ucIs4KWarp)
    {
        return (UINT16)DEF_WP_SPACE_4K_BITS;
    }

    return (UINT16)DEF_WP_SPACE_2K_BITS;

}

UINT8 halWarping_Is4KWarp(void)
{
    return m_sHalWarpingInfo.ucIs4KWarp;
}

