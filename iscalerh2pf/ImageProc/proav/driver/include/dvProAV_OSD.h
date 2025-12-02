#ifndef DV_PROAV_OSD_H
#define DV_PROAV_OSD_H

#include "dvProAV_Platform.h"
#include "dvProAV_Base.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef QT_CPP
    #include "Common.h"
#endif

//-------------------------------------------------------------------------------------------------
// Scaler OSD
//-------------------------------------------------------------------------------------------------
/**
 * @brief Paint String
 * @param [in] sDes_position : String start position
 * @param [in] sString : String attribute
 * @param [in] iFont_Color : Font Color
 * @param [in] iBG_Color : Font Back Ground Color
 * @param [in] iStringOffset :
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset);

/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index);
int dvProAV_Page_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset, UINT8 charVNum);

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] sBitmap : Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position);

/**
 * @brief 複製當前顯示OSD畫面到另一個Memory Page
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @todo : 判斷PanelTiming
 */
int OSD_CurrentCopy(void);

/**
 * @brief Inhibit Color Set
 * @param [in] cEnable : OSD write-inhibit color enable (0 or 1)
 * @param [in] iInhibit_Color : OSD write-inhibit color (color pallet number)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Inhibit_Color_Set(INT8 cEnable, UINT16 iInhibit_Color);

/**
 * @brief OSD software reset
 * @param [in] bEnable :
 *              false : software reset disable
 *              true : software reset enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_SW_Reset(bool bEnable);

/**
 * @brief 不顯示OSD畫面
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_Enable(bool bEnable);

bool dvProAV_OSD_EnableGet(void);

/**
 * @brief OSD On
 * @param [in] sDes_position : 設定OSD起始點
 * @param [in] sDes_Size : 設定OSD Size
 * @param [in] cTrap_Enable : 設定OSD畫面Enable / Disable
 * @param [in] sTrap_Color : 設定透明色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_On(START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color);

/**
 * @brief OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color);


int dvProAV_OSD_ReLoad(void);

/**
 * @brief Set OSD Menu Transparency
 * @param [in] ucValue : 傳入透明度比例
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_MenuTransparencySet(UINT8 ucValue);

/**
 * @brief Set OSD Menu Transparency Enable / Disable
 * @param [in] ucEnalbe : Set Menu Transparency
 *              -false : disable
 *              -true : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe);

/**
 * @brief Set OSD Palette
 * @param [in] iAmount :
 * @param [in] pucData :
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int OSD_Palette_Set(INT16 iAmount, UINT8 *pucData);

int dvProAV_OSD_FillColor(UINT16 iColor_Index);

/**
 * @brief OSD Double Size Set
 * @param [in] bDoubl : false = disable, true = enable
 */
void dvProAV_OSD_DoubleSizeSet(bool bDouble, bool bDoubleX, bool bDoubleY);

/**
 * @brief OSD Write Size Set
 * @param [in] u16SizeH : OSD Write size H
 * @param [in] u16SizeV : OSD Write size V
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_WriteSizeSet(uint16 u16SizeH, uint16 u16SizeV);

/**
 * @brief OSD Read Size Set
 * @param [in] u16SizeH : OSD read size H
 * @param [in] u16SizeV : OSD read size V
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_ReadSizeSet(uint16 u16SizeH, uint16 u16SizeV);

/**
 * @brief ReLoad OSD Front & Bitmap Data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_ReLoad(void);

#ifndef QT_CPP
    INT8 dvProAV_OSD_PanelSet(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
    INT8 dvProAV_OSD_PanelChange(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
#endif
//-------------------------------------------------------------------------------------------------
// Logo Capture
//-------------------------------------------------------------------------------------------------
void dvProAV_OSD_LogoCaptureLoad(void);
void dvProAV_OSD_LogoDefaultReplacement(void);
void dvProAV_OSD_LogoUserReplacement(void);

//-------------------------------------------------------------------------------------------------
// Warp OSD For Entry 4K
//-------------------------------------------------------------------------------------------------
/**
 * @brief Page Clear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_PageClear_E4K(eWPOSDPAGE ePage);

/**
 * @brief Set Warp OSD Palette
 * @param [in] iAmount : 傳入調色盤長度
 * @param [in] pucData : 傳入調色盤data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Palette_Set_E4K(INT16 iAmount, UINT8 *pucData);

/**
 * @brief select warp osd page
 * @param [in] eWPOSDPAGE : select warp osd page
 *                  - eWPOSDPAGE_0 : in eLayerMode_P0BeforeP1AfterWarp mode = before warp
 *                  - eWPOSDPAGE_1 : in eLayerMode_P0BeforeP1AfterWarp mode = after warp
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE ePage);


/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Paint_Rectangle_E4K(eWPOSDPAGE ePage, START_POINT sDes_position, RECT_SIZE sDes_Size, uint16 iColor_Index);

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] ulSrc_Addr : setting read base address
 * @param [in] sBitmap : source Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_CurrentCopy_E4K(eWPOSDPAGE ePage, OSD_BITMAP sBitmap, START_POINT sDes_position);

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] sBitmap : Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note 目前設定在不能複製2D mode圖至XPR mode page
 */
int dvProAV_WarpOSD_Paint_Bitmap_E4K(OSD_BITMAP sBitmap, START_POINT sDes_position);

/**
 * @brief 畫單一圓型
 * @param [in] sDes_center : setting circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] iColor_Index : setting circle color
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note 圓最小半徑是10, 小於10會不圓, 不支援半徑大於原心case
 */
int dvProAV_WarpOSD_Paint_SingleCircle_E4K(START_POINT sDes_center, INT16 iRadius, uint16 iColor_Index);

/**
 * @brief 畫多個圓型
 * @param [in] sDes_center : setting top left circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] sPitch : setting pitch for circle center
 * @param [in] sLimit : setting paint multi Circle boundary
 * @param [in] iColor_Index : setting circle color
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Paint_MultiCircle_E4K(START_POINT sDes_center, INT16 iRadius, CENTER_PITCH sPitch, START_POINT sLimit, uint16 iColor_Index);

/**
 * @brief after & before WarpOSD Mix
 * @param [in] bEnable : true=on false=off
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_MixOn_E4K(bool bEnable);

/**
 * @brief WarpOSD Display
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_On_E4K(eWPOSDPAGE ePage);

/**
 * @brief WarpOSD Off For E4K
 * @param [in] ePage
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Off_E4K(eWPOSDPAGE ePage);

/**
 * @brief Warp OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Init_E4K(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, uint16 ucInhibit_Color);

/**
 * @brief dvProAV_WarpOSD_Set_E4K
 * @param [in] bE4K : true/fales : Warp OSD/Scaler OSD
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Set_E4K(bool bWarpOsd);

//-------------------------------------------------------------------------------------------------
// Warp OSD For Entry 4K Sub-Frame Mode
//-------------------------------------------------------------------------------------------------
/**
 * @brief Warp OSD Sub Frame Mode Flag Set
 * @param [in] bSubFrameMod
 *              - false : normal mode
 *              - true : sub frame mode
 */
void dvProAV_WarpOSDSubFrameModFlagSet_E4K(bool bSubFrameMod);

/**
 * @brief Warp OSD Sub Frame Mode Flag Get
 * @return bSubFrameMod
 *              - false : normal mode
 *              - true : sub frame mode
 */
bool dvProAV_OSD_4k3dEn_Get(VOID);

/**
 * @brief Warp OSD Sub Frame Mode Enable
 * @param [in] bSubFrameMod :
 *              - false : normal mode
 *              - true : sub frame mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSDSubFrameModEn_E4K(bool bSubFrameMod);

/**
 * @brief Warp OSD Sub Frame Mode Get
 * @return SubFrameMod :
 *              - false : normal mode
 *              - true : sub frame mode
 */
bool dvProAV_WarpOSDSubFrameModGet_E4K(void);

/**
 * @brief Control Copy Data From Warp OSD DRAM Address At Sub Frame Mode(only use for bitblt and multi-circle)
 * @param [in] bBmpCurrentPage
 *              - false : copy data from other page
 *              - true : copy data from current display page
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSDPaintBmpCtrl_E4K(bool bBmpCurrentPage);

UINT32 dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE ePage);
int dvProAV_WarpOSD_MemplaneLastSel_E4K(eWPOSDPAGE ePage);
void dvProAV_OSD_Buffer_Init(VOID);
void dvProAV_OSD_DbgPrint(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_OSD_H
