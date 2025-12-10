// ===============================================================================
// FILE NAME: dvC789_WarpLight.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/12/29, Larry Create
// --------------------
// ===============================================================================

#ifndef DV_C789_WARPLIGHT_H_

#include "Common.h"

//===== Grid parameter =====//
#define DEF_GRID_SIZE		        8// DEF_GRD_SIZE >= 1
#define DEF_GRID_SIZE_O		        8

//===== Warp Param =====//
#define DEF_WP_LIM_MODE		        0  // 0 : WpLimit OFF, 1 : WpLimit ON
#define DEF_WP_SPACE		        16 // 32, 64
#define DEF_WP_SPACE_BIT	        4

#if(DEF_WP_SPACE == 32)
#define DEF_HW_MAX			        2016
#define DEF_VW_MAX			        1248
#else //16 pixel
#define DEF_HW_MAX			        2032
#define DEF_VW_MAX			        1232
#endif

#define DEF_HW_MAX_GRD		        DEF_HW_MAX/DEF_WP_SPACE//129// = DEF_HW_MAX / DEF_WP_SPACE + 1	[roundup]
#define DEF_VW_MAX_GRD		        DEF_VW_MAX/DEF_WP_SPACE//69// = DEF_VW_MAX / DEF_WP_SPACE + 1	[roundup]

//1920/16 = 120 //125
//1200/16 = 75  //76

#define DEF_WP_OUT_HGRD		        4// Warptable horizontal grid of outside // fix
#define DEF_WP_OUT_VGRD		        4// Warptable vertical grid of outside
#define DEF_WP_OUT_HW		        (DEF_WP_OUT_HGRD << DEF_WP_SPACE_BIT)
#define DEF_WP_OUT_VW		        (DEF_WP_OUT_VGRD << DEF_WP_SPACE_BIT)

#define DEF_GV_PX_HGRD		        (DEF_WP_OUT_HGRD + DEF_HW_MAX_GRD + DEF_WP_OUT_HGRD)// 3 + 128 + 3 = 134
#define DEF_GV_PX_VGRD		        (DEF_WP_OUT_VGRD + DEF_VW_MAX_GRD + DEF_WP_OUT_VGRD)// 27 + 68 + 27 = 122

#define DEF_HW_DUMMY_GRD	        0// fix
#define DEF_HW_DUMMY		        (DEF_HW_DUMMY_GRD << DEF_WP_SPACE_BIT)// Dummy area(Correction to the outside)

#define DEF_NUM_CASC	            3
#define DEF_CASC_OLD_TBL	        0
#define DEF_CASC_NEW_TBL	        1

//===== Polation Table =====//
#define POLATION_MAX 10
#define POLATION_NUMBER 24

#define DTCT_BASE_ACCESS_ENABLE_32PIEXL_A   0x0882  //A35G2_CDS_Simon_0024 , enable WPOUTEN: Overflow correction enable
#define DTCT_BASE_ACCESS_ENABLE_16PIEXL_A   0x0880  //use A side , CPU access B enable
#define DTCT_BASE_ACCESS_ENABLE_32PIEXL_B   0x0852
#define DTCT_BASE_ACCESS_ENABLE_16PIEXL_B   0x0850  //use B side , CPU access A enable
#define DTCT_BASE_ACCESS_DISABLE_32PIEXL_A  0x0812
#define DTCT_BASE_ACCESS_DISABLE_16PIEXL_A  0x0810  //use B side , CPU access disable
#define DTCT_BASE_ACCESS_DISABLE_32PIEXL_B  0x0802
#define DTCT_BASE_ACCESS_DISABLE_16PIEXL_B  0x0800  //use A side , CPU access disable

#define WARPING_DISTORTION_CORRECTION_TABLE_A (0x0000)
#define WARPING_DISTORTION_CORRECTION_TABLE_B (0x0010)
#define WARPING_DISTORTION_ACCESS_TABLE_A     (0x0040)
#define WARPING_DISTORTION_ACCESS_TABLE_B     (0x0080)

#define DTCT_BASE_WPOUTMD_IMPROVEMENT 0x0400

//#define DTCT_BASE_ACCESS_ENABLE_32PIEXL_A   0x0002
//#define DTCT_BASE_ACCESS_ENABLE_16PIEXL_A   0x0040
//#define DTCT_BASE_ACCESS_ENABLE_32PIEXL_B   0x0002
//#define DTCT_BASE_ACCESS_ENABLE_16PIEXL_B   0x0040
//#define DTCT_BASE_ACCESS_DISABLE_32PIEXL_A  0x0002
//#define DTCT_BASE_ACCESS_DISABLE_16PIEXL_A  0x0000
//#define DTCT_BASE_ACCESS_DISABLE_32PIEXL_B  0x0002
//#define DTCT_BASE_ACCESS_DISABLE_16PIEXL_B  0x0000

#define RTCT_WARP_TRANSFER_DISABLE  0xF8
#define RTCT_WARP_TRANSFER_ENABLE   0x05
#define RTCT_WARP_WPPOVS_ENABLE     0x07

typedef enum
{
    eC789_WARP_CROSSHATCH_MODE_OFF,
    eC789_WARP_CROSSHATCH_MODE_SINGLE,
    eC789_WARP_CROSSHATCH_MODE_OVERLAY,
    eC789_WARP_CROSSHATCH_MODE_LAST,
}eC789_WARP_CROSSHATCH_MODE;

typedef struct
{
    UINT32 ulX;
    UINT32 ulY;
}sCOORDINATE_T, *PsCOORDINATE_T;

typedef struct
{
    float fX;
    float fY;
}sCOORDINATE_F, *PsCOORDINATE_F;

typedef struct
{
	float fA;
	float fB;
	float fC;
	float fD;
}sLINE_COEF_T, * PsLINE_COEF_T;


typedef struct
{
    BOOL                        bInit;
    //ePANEL_ID                   ePanelTimingId;           //Pass from up layer to get panel output timing

    //UINT8						ucPanelIndex;             //To save panel table index
    //sOUTPUT_TIMING_INFO         sOutputTimingInfo;

    //UINT32                      ulMclko_Freq;


    //===== parameters 1 =====//
    //UINT16                      uiPanelHst;
    //UINT16                      uiPanelVst;
    UINT16                      uiPanelHW;          // PS_WP_HW, Panel Width, 3840
    UINT16                      uiPanelVW;          // PS_WP_VW, Panel Height, 2160
    UINT16                      uiPixelGridHW;      // PS_WP_HW_GRD, Grid Width / 32 pixel mode or 64 pixel mode = how many interpoation grid size
    UINT16                      uiPixelGridVW;      // PS_WP_VW_GRD, Grid Height / 32 pixel mode or 64 pixel mode = how many interpoation grid size


    UINT8                       ucWarpColor;        // GV_WP_TABLE_COLOR, in 32 pixel space, there A,B,C three table can be used.Refer to B14_DTCT
                                                    //                    in 64 pixel space, there A,B,C,D fourc table can be used.
    //BOOL                        bWPTblSel;




    float                       faCursorDefInX[17];             // PS_CUR_DEF_IX, real coordinate in the 3840x2160 canvas
    float                       faCursorDefInY[17];             // PS_CUR_DEF_IY, real coordinate in the 3840x2160 canvas
    float                       faCursorDefOutX[17];            // PS_CUR_DEF_OX, real coordinate in the 3840x2160 canvas
    float                       faCursorDefOutY[17];            // PS_CUR_DEF_OY, real coordinate in the 3840x2160 canvas

//    UINT16                      uiIACT_HW;// PM_IACT_HW;
//    UINT16                      uiIACT_vW;// PM_IACT_VW;

    //===== Warping ====//

    sCOORDINATE_F               saGridPos[DEF_NUM_CASC][17][17];// PM_GRID, Cascade
    sCOORDINATE_F               saPrevGridPos[17][17];          // PM_GRID_OLD


#if 0
    // EdgeBlending Gain
    UINT16                      uiBlendingEdgeHW;               // PM_EGB_OHW
    UINT16                      uiBlendingEdgeVW;               // PM_EGB_OVW
    BOOL                        bBlendingGainEnable;            // PM_EGB_EN
    BOOL                        bBlendingGainEnable_T;          // PM_EGB_TEN
    BOOL                        bBlendingGainEnable_B;          // PM_EGB_BEN
    BOOL                        bBlendingGainEnable_L;          // PM_EGB_LEN
    BOOL                        bBlendingGainEnable_R;          // PM_EGB_REN

    // EdgeBlending Bias
    BOOL                        bBlendingBiasEnable;            // PM_EBIAS_EN
    UINT8                       ucBlendingBiasCursorX;          // PM_EBIAS_SEL_CUR_X
    UINT8                       ucBlendingBiasCursorY;          // PM_EBIAS_SEL_CUR_Y
    sCOORDINATE_T               saBlendingBiasCursorPos[2][2];  // PM_EBIAS_CUR

    UINT8                       ucaBlendingBiasPalleteColor[16][4]; // PM_EBIAS

    BOOL                        bBlendingBiasAreaTest;              // PM_EBIAS_AREA_TEST
    UINT16                      uiBlendingBiasCursorMoveStep;       // PM_EBIAS_CUR_MV
    UINT8                       ucBlendingBiasColorSel;             // PM_EBIAS_COLOR

    // Blending Gain Gamma
    BOOL                        bBlendingGainGammaEnable;           // PM_EGB_GAMMA_EN
    float                       faBlendingGainGamma[4];  // PM_EGB_GAMMA, R, G, B
    UINT8                       ucBlendingGainGammaColorSel;        // PM_EGB_GAMMA_COLOR
    // Output Gamma
    BOOL                        bOutputGammaEable;                  // PM_GAMMA_EN, Output Gamma
    float                       faOutputGamma[33][3];      // PM_GAMMA, 33 output gamma point
    UINT16                      uiaOutputGammaPixel[33];   // PM_GAMMA_PIX, Each point pixel pos
    UINT16                      uiOutputGammaPixelSel;              // PM_GAMMA_PIX_SEL
    UINT8                       ucOutputGammaColor;                 // PM_GAMMA_COLOR

    // Blending Bias Gamma
    float                       faBlendingBiasGamma[4];              // PM_EBIAS_GAMMA
    UINT8                       ucBlendingGammaColor;                               // PM_EBIAS_GAMMA_COLOR
    UINT16                      uiBlendingBiasGammaData[16][4][16];  //PM_EBIAS_GMDT[16][3][16];
#endif /* 0 */

    INT16                       iDummyHstReset;                                    // PS_DUMMY_HST_REST
    INT16                       iDummyHendGridReset;                                // PS_DUMMY_HEND_GRD_REST

    //===== Warp Table parameters ====//
    float                       faPosX[2][DEF_GV_PX_HGRD][DEF_GV_PX_VGRD];// GV_PX, cascade
    float                       faPosY[2][DEF_GV_PX_HGRD][DEF_GV_PX_VGRD];// GV_PY, cascade


    //===== OSD Cursor parameters ====//
    UINT16                      uiNumHGrid;  // GV_NUM_HGRID, number grid, related to warp mode 2x2, 3x3...
    UINT16                      uiNumVGrid;  // GV_NUM_VGRID, number grid, related to warp mode 2x2, 3x3...
    UINT16                      uiNumHGrid_M1;//GV_NUM_HGRID_m1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_M1;//GV_NUM_VGRID_m1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumHGrid_M2;//GV_NUM_HGRID_m2, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_M2;//GV_NUM_VGRID_m2, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumHGrid_P1;//GV_NUM_HGRID_p1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_P1;//GV_NUM_VGRID_p1, for calc_h_line_coef(), calc_v_line_coef()




    //===== Warp Parameters ====//
    sLINE_COEF_T                saWarpVerLine[17][16];// GV_WP_VER_LINE
    sLINE_COEF_T                saWarpHorLine[16][17];// GV_WP_HOR_LINE
    sLINE_COEF_T                saWarpHorLineMove[16];// GV_WP_HOR_LINE_MV
    sLINE_COEF_T                saWarpVerLineMove[DEF_HW_MAX_GRD][16];// GV_WP_VER_LINE_MV
    sCOORDINATE_F               saCalPoint[17][17];// GV_CP
    BOOL                        baHGridCalEnable[17];// GV_HGRID_CLC_EN[17]
    BOOL                        baVGridCalEnable[17];// GV_VGRID_CLC_EN[17]
    //===== Warp_2x2 Parameters ====//
    float                       faPjcA[3];          // GV_PJC_A
    float                       faPjcB[3];          // GV_PJC_B
    float                       faPjcC[3];          // GV_PJC_C

    //===== Gamma Parameters ====//
    UINT16                      uiTmpGammaTable[1024];// GV_TEMP_GAMMA_TABLE[1024]

    //===== Register parameters =====//
    //UINT32                      ulRegRTCT;          // GV_RTCT
    //UINT16                      uiRegEGBCT;         // GV_EGBCT;


    //===== Test Pattern ====//
    //eC789_WARP_CROSSHATCH_MODE  ePatternGen;        //PM_PATGEN;
    //BOOL                        bEdgeMarkerEnable;  //PM_EGBMK_EN;
    //BOOL                        bOfillEnable;       //PM_OFILL_EN;


}sDRV_WARPING_INFO;

typedef enum

{
    eC789_WARP_MODE_2x2,
    eC789_WARP_MODE_3x3,
    eC789_WARP_MODE_5x5,
    eC789_WARP_MODE_9x9,
    eC789_WARP_MODE_17x17,
}eC789_WARP_MODE;

typedef enum
{
	eC789_WARP_OK,
	eC789_WARP_ERR_ANG_H,// Error of H-slope
	eC789_WARP_ERR_ANG_V,// Error of V-slope
	eC789_WARP_ERR_VSH_LOC,// Error of local V-shrink rate
	eC789_WARP_ERR_VSH_AVE,// Error of average V-shrink rate
	eC789_WARP_ERR_HSH,// Error of H-shrink rate
	eC789_WARP_ERR_OUTSIDE,// Error of outside of ACT
	eC789_WARP_ERR_INTERVAL,// Error of grid interval
	eC789_WARP_NGZ,
	eC789_WARP_CURPOS,
	eC789_WARP_ERR// Error of other reasons


}eC789_WARP_ERROR;

typedef enum
{
    eC789_WARP_DIRECTION_UP,
    eC789_WARP_DIRECTION_DOWN,
    eC789_WARP_DIRECTION_LEFT,
    eC789_WARP_DIRECTION_RIGHT,
    eC789_WARP_DIRECTION_LAST,
}eC789_WARP_DIRECTION;


static const UINT8 m_sPolationTable[POLATION_MAX][POLATION_NUMBER] =
{
    //0 //lutset_6s_c03w00
    {0x02,0x03,0x04,0x04,0x05,0x06,0x07,0x08,0x09,0x09,0x0a,0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x0f,0x11,0x12,0x13,0x13,0x14},
    //1 //lutset_6s_c04w00
    {0xfe,0xff,0xff,0x00,0x01,0x02,0x03,0x05,0x06,0x08,0x09,0x0b,0x0c,0x0e,0x0f,0x10,0x12,0x13,0x14,0x15,0x17,0x16,0x18,0x16},
    //2 //lutset_6s_c05w00
    {0xfa,0xfa,0xfa,0xfb,0xfc,0xfd,0xff,0x00,0x02,0x04,0x06,0x09,0x0c,0x0e,0x11,0x11,0x16,0x18,0x1a,0x1c,0x1e,0x1f,0x1e,0x1e},
    //3 //lutset_6s_c06w00
    {0xfb,0xf9,0xf8,0xf8,0xf7,0xf8,0xf9,0xfb,0xfc,0xff,0x01,0x04,0x08,0x0c,0x10,0x13,0x18,0x1d,0x21,0x24,0x27,0x27,0x28,0x24},
    //4 //lutset_6s_c07w00
    {0x00,0xff,0xfc,0xfa,0xf8,0xf7,0xf6,0xf6,0xf7,0xf8,0xfb,0xfe,0x01,0x06,0x0c,0x12,0x17,0x1d,0x23,0x28,0x2d,0x2f,0x30,0x30},
    //5 //lutset_6s_c08w00
    {0x05,0x04,0x02,0x00,0xfe,0xfb,0xf8,0xf5,0xf5,0xf5,0xf6,0xf8,0xfc,0x00,0x05,0x0d,0x13,0x1a,0x21,0x28,0x2d,0x32,0x36,0x3c},
    //6 //lutset_6s_c09w00
    {0x06,0x07,0x06,0x05,0x03,0x00,0xfe,0xfa,0xf8,0xf6,0xf5,0xf5,0xf7,0xfb,0x00,0x06,0x0e,0x16,0x1e,0x26,0x2d,0x32,0x36,0x40},
    //7 //lutset_6s_c10w00
    {0x02,0x04,0x06,0x07,0x07,0x05,0x03,0x00,0xfd,0xf9,0xf6,0xf4,0xf4,0xf6,0xfa,0x00,0x08,0x11,0x1b,0x25,0x2e,0x37,0x3c,0x40},
    //8 //lutset_6s_c10w10
    {0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0xff,0xfd,0xfb,0xf9,0xf8,0xf9,0xfc,0x00,0x06,0x0f,0x1a,0x26,0x32,0x3a,0x3e,0x40},
    //9 //lutset_6s_c10w15
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xfe,0xfd,0xfc,0xfc,0xfd,0x00,0x05,0x0d,0x18,0x23,0x2e,0x38,0x3e,0x40},
};

void dvC789_WarpLight_PanelConfig(void);
void dvC789_WarpModeChange(eC789_WARP_MODE eWarpMode);
//void dvC789_ShowGrid(BOOL bGridShow);
//void dvC789_EraseGrid(UINT16 uiX, UINT16 uiY);
//void dvC789_DrawGrid(UINT16 uiX, UINT16 uiY, UINT8 ucGridSelX, UINT8 ucGridSelY);
eC789_WARP_ERROR dvC789_MoveWarp(eC789_WARP_DIRECTION eDir, float fGridMovePitch, UINT8 ucGridSelX, UINT8 ucGridSelY, eC789_WARP_MODE eWarpMode, BOOL bInternalGridShow);
void dvC789_InitWarpTable(void);

#endif /* DV_C789_WARPLIGHT_H_ */

