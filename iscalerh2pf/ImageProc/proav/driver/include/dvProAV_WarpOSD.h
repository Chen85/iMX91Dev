#ifndef DV_PROAV_WARPOSD_H
#define DV_PROAV_WARPOSD_H

#include "dvProAV_Platform.h"
#include "dvProAV_Base.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    eLayerMode_BothBeforeWarp,
    eLayerMode_P0BeforeP1AfterWarp,
    eLayerMode_PageAfterWarp,

    eLayerMode_Max
}eLayerMode;

/**
 * @brief Page Clear
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_PageClear(void);

/**
 * @brief Set Warp OSD Palette
 * @param [in] iAmount : 傳入調色盤長度
 * @param [in] u8PucData : 傳入調色盤data
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Palette_Set(INT16 iAmount, UINT8 *u8PucData);

/**
 * @brief select warp osd page
 * @param [in] eWPOSDPAGE : select warp osd page
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE ePage);

int dvProAV_WarpOSD_Swap(eWPOSDPAGE ePage);

/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 * @Todo
 */
int dvProAV_WarpOSD_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, uint16 iColor_Index);

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] ulSrc_Addr : setting read base address
 * @param [in] sBitmap : source Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 * @ToCheck Addr是否需在此設定?
 */
int dvProAV_WarpOSD_CurrentCopy(eWPOSDPAGE ePage, OSD_BITMAP sBitmap, START_POINT sDes_position);

/**
 * @brief 從BMP address copy
 * @param [in] ulSrc_Addr : setting read base address
 * @param [in] sBitmap : source Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 * @ToCheck Addr是否需在此設定?
 */
int dvProAV_WarpOSD_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position);

/**
 * @brief 畫單一圓型
 * @param [in] sDes_center : setting circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] iColor_Index : setting circle color
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 * @note 圓最小半徑是10, 小於10會不圓
 */
int dvProAV_WarpOSD_Paint_SingleCircle(START_POINT sDes_center, INT16 iRadius, uint16 iColor_Index);

/**
 * @brief 畫多個圓型
 * @param [in] sDes_center : setting top left circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] sPitch : setting pitch for circle center
 * @param [in] sLimit : setting paint multi Circle boundary
 * @param [in] iColor_Index : setting circle color
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Paint_MultiCircle(START_POINT sDes_center, INT16 iRadius, CENTER_PITCH sPitch, START_POINT sLimit, uint16 iColor_Index);

/**
 * @brief 畫點
 * @param [in] sDes_position : setting paint dot position
 * @param [in] cPointCount : setting count for paint X direction dot
 * @param [in] iColor_Index : setting color
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Paint_Dot(START_POINT sDes_position, UINT16 cPointCount, uint16 iColor_Index);

/**
 * @brief after & before WarpOSD Mix
 * @param [in] bEnable : true=on false=off
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_MixOn(bool bEnable);

/**
 * @brief WarpOSD Display
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_On(eWPOSDPAGE ePage);

/**
 * @brief OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, uint16 ucInhibit_Color);

/**
 * @brief Inhibit Color Set
 * @param [in] cEnable : Warp OSD write-inhibit color enable (0 or 1)
 * @param [in] iInhibit_Color : Warp OSD write-inhibit color (color pallet number)
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Inhibit_Color_Set(INT8 cEnable, uint16 iInhibit_Color);

/**
 * @brief Set Layer Mode
 * @param [in] eMode :
 *              - eLayerMode_BothBeforeWarp
 *              - eLayerMode_P0BeforeP1AfterWarp
 *              - eLayerMode_PageAfterWarp
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_LayerModeSet(eLayerMode eMode);

/**
 * @brief High Color Mode Enable / Disable
 * @param [in] eMode :
 *              - false: Disable
 *              - true : Enable
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_HighColorModeEn(bool eMode);
void dvProAV_WarpOSD_DbgPrint(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_WARPOSD_H
