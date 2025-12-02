#include "dvProAV_WarpOSD.h"


static uint16 m_panelWidth = 1920;  // for test
static uint16 m_panelHeight = 1080; // for test

static uint32 PWPOSD_SAD;           // PWPOSD_SAD(current) point to current OSD memory plane

//before warping
static uint32 PWPOSD_SAD_BEFORE;    // PWPOSD_SAD_BEFORE point to current OSD memory plane berfor warping
static uint32 PWPOSD_SAD1;          // OSDSAD0 OSD memory plane 0
static uint32 PWPOSD_SAD2;          // OSDSAD1 OSD memory plane 1

//after warping
static uint32 PWPOSD_SAD_AFTER;     // PWPOSD_SAD_AFTER point to current OSD memory plane after warping
static uint32 PWPOSD_SAD3;          // OSDSAD0 OSD memory plane 0
static uint32 PWPOSD_SAD4;          // OSDSAD1 OSD memory plane 1

static uint16 m_iInhitbit_Color = 0xFFFF;

/**
 * @brief Get WarpOSD busy state
 * @return BusyState : false=unbusy, true=busy
 */
static bool dvProAV_WarpOSD_BusyStateGet(void)
{
    uint32 BusyState = 0;
    dvProAV_AccessRead(eWrpOsdBusy, &BusyState);
    return (bool)BusyState;
}

/**
 * @brief Page Clear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_PageClear(void)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    status &= dvProAV_WarpOSD_Inhibit_Color_Set(false, m_iInhitbit_Color);  // 關閉禁止色
    #if 0   // 矩形畫透明色
        START_POINT_WP sStart;
        RECT_SIZE_WP sSize;

        sStart.iX = 0;
        sStart.iY = 0;
        sSize.iWidth = (int16)m_panelWidth;
        sSize.iHeight = (int16)m_panelHeight;

        status &= dvProAV_WarpOSD_Paint_Rectangle(sStart, sSize, m_iInhitbit_Color);
    #else   // 透明色功能
        uint32 uiTranNum = (((m_panelWidth * m_panelHeight) + (32 * PROAV_DRAM_LINE_BYTES) - 1) / (32 * PROAV_DRAM_LINE_BYTES)) * 2;     // (Hsize*Vsize)/(一次寫入32個DRAM位置*DRAM位置Byte數), 無條件進位
        status &= dvProAV_AccessWrite(eWrpOsdMemWrAdr, PWPOSD_SAD);
        status &= dvProAV_AccessWrite(eWrpOsdFillColorNum, uiTranNum);
        status &= dvProAV_AccessWrite(eWrpOsdBmpFillColor, (uint8)m_iInhitbit_Color);
        status &= dvProAV_AccessWrite(eWrpOsdTrig, eWPOSD_TransparentTrigger);
    #endif

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        MS_SLEEP(1);
        retry--;
    }

    status &= dvProAV_WarpOSD_Inhibit_Color_Set(true, m_iInhitbit_Color);  // 開啟禁止色

    return status;
}

/**
 * @brief Set Warp OSD Palette
 * @param [in] iAmount : 傳入調色盤長度
 * @param [in] u8PucData : 傳入調色盤data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Palette_Set(INT16 iAmount, UINT8 *u8PucData)
{
    int status = rcSUCCESS;

    if (iAmount != PROAV_PALETTE_COLOR)
    {
       return rcSUCCESS;
    }
    // Enable調色盤
    status &= dvProAV_AccessWrite(eWrpOsdPaletteEn , true);
    // 寫入調色盤Data
    #ifdef PaintStringSingleMode    // Single mode
        for(INT16 iCount=0; iCount<(iAmount*3); iCount++)
        {
           //LOG_MSG(db_DV_C734_OSD, "(func:%s, line:%d)Ch1(%d)=%d\r\n", __FUNCTION__, __LINE__, iCount, *(pucData+iCount));
           status &= dvProAV_AccessWrite(eOWOsdPalete, *(pucData+iCount));
        }
    #else   // Burst mode
        status &= dvProAV_AccessBurstWrite(eWrpOsdPalette , u8PucData, (uint16)(iAmount*3), BURST_FIX_ADDR);
    #endif

    return status;
}

/**
 * @brief select warp osd page
 * @param [in] eWPOSDPAGE : select warp osd page
 *                  - eWPOSDPAGE_0 : in eLayerMode_P0BeforeP1AfterWarp mode = before warp
 *                  - eWPOSDPAGE_1 : in eLayerMode_P0BeforeP1AfterWarp mode = after warp
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_MemplaneSel(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    if(ePage >= eWPOSDPAGE_Max)
        return rcERROR;


    if(ePage == eWPOSDPAGE_0)
    {
        PWPOSD_SAD = (PWPOSD_SAD_BEFORE == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
    }
    else
    {
        PWPOSD_SAD = (PWPOSD_SAD_AFTER == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
    }

    // 設定寫入位置，設定background address為寫入OSD的位置
    status &= dvProAV_AccessWrite(eWrpOsdStartAdr, PWPOSD_SAD);
    //printf("eWrpOsdStartAdr = 3 0x%08x\n", PWPOSD_SAD);

    return status;
}

int dvProAV_WarpOSD_Swap(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    if(ePage == eWPOSDPAGE_0)
    {
        PWPOSD_SAD_BEFORE = (PWPOSD_SAD_BEFORE == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
    }
    else
    {
        PWPOSD_SAD_AFTER = (PWPOSD_SAD_AFTER == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
    }

    status &= dvProAV_WarpOSD_MemplaneSel(ePage);
    return status;
}

/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @Todo
 */
int dvProAV_WarpOSD_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, uint16 iColor_Index)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if((sDes_Size.iWidth == 0) || (sDes_Size.iHeight == 0))
    {
        return rcERROR;
    }

    #if 0   // single mode
        status &= dvProAV_AccessWrite(eWrpOsdBmpHeight,(sDes_Size.iHeight & 0xffff));        // 設定矩形高度
        status &= dvProAV_AccessWrite(eWrpOsdBmpWidth,(sDes_Size.iWidth & 0xffff));          // 設定矩形寬度
        status &= dvProAV_AccessWrite(eWrpOsdBmpVStart, (uint16)sDes_position.iY);           // 設定矩形起始位置Y
        status &= dvProAV_AccessWrite(eWrpOsdBmpHStart, (uint16)sDes_position.iX);           // 設定矩形起始位置X
        status &= dvProAV_AccessWrite(eWrpOsdBmpFillColor,iColor_Index & 0xffff);            // 設定矩形顏色
    #else   // burst mode
        uint8 wData[10];
        wData[0] = sDes_Size.iHeight & 0xff;                                        // 設定矩形高度
        wData[1] = (sDes_Size.iHeight >> 8) & 0xff;
        wData[2] = sDes_Size.iWidth & 0xff;                                         // 設定矩形寬度
        wData[3] = (sDes_Size.iWidth >> 8) & 0xff;
        wData[4] = sDes_position.iY & 0xff;                                         // 設定矩形起始位置Y
        wData[5] = (sDes_position.iY >> 8) & 0xff;
        wData[6] = sDes_position.iX & 0xff;                                         // 設定矩形起始位置X
        wData[7] = (sDes_position.iX >> 8) & 0xff;
        wData[8] = iColor_Index & 0xff;                                             // 設定矩形顏色
        wData[9] = (iColor_Index >> 8) & 0xff;
        status &= dvProAV_AccessBurstWrite(eWrpOsdBmpHeight , (uint08 *)wData, 10, BURST_INC_ADDR); // Fixed Address if last parameter is false
    #endif

    status &= dvProAV_AccessWrite(eWrpOsdTrig, eWPOSD_RectangleTrigger);              // 資料寫入DRAM

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        #ifdef OSDBUSYTIMEOUT
            if (TimeElapsed(eTimerUser0) > OSDBUSYTIMEOUT)
            {
                DBMSG(" OSD Busy... \n");
                break;
            }
        #endif
        MS_SLEEP(1);
        retry--;
    }

    return status;
}

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] ulSrc_Addr : setting read base address
 * @param [in] sBitmap : source Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_CurrentCopy(eWPOSDPAGE ePage, OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    // set read BMP size
    status &= dvProAV_AccessWrite(eWrpOsdBmpWidth, sBitmap.sBitmap_Size.iWidth & 0xffff);  //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    status &= dvProAV_AccessWrite(eWrpOsdBmpHeight, sBitmap.sBitmap_Size.iHeight & 0xffff);  //BitBLT vertical direction width register

    // set read BMP position //copy當下在畫的DRAM
    status &= dvProAV_AccessWrite(eWrpOsdBltAdr, PWPOSD_SAD);                       // 設定Blt Function Address
    status &= dvProAV_AccessWrite(eWrpOsdBmpHStart, sBitmap.sSrc_Position.iX & 0xffff);
    status &= dvProAV_AccessWrite(eWrpOsdBmpVStart, sBitmap.sSrc_Position.iY & 0xffff);
    // set write BMP position
    //status &= dvProAV_AccessWrite(eOWMemWrAdr, PWPOSD_SAD_BEFORE);                     // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eWrpOsdBltVStart, sDes_position.iY & 0xffff);
    status &= dvProAV_AccessWrite(eWrpOsdBltHStart, sDes_position.iX & 0xffff);

    status &= dvProAV_AccessWrite(eWrpOsdTrig, eWPOSD_BitBLTTrigger);                //OSD control register  ==> BitBLT

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        #ifdef OSDBUSYTIMEOUT
            if (TimeElapsed(eTimerUser0) > OSDBUSYTIMEOUT)
            {
                DBMSG(" OSD Busy... \n");
                break;
            }
        #endif
        MS_SLEEP(1);
        retry--;
    }

    return status;
}

int dvProAV_WarpOSD_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    // set read BMP size
    status &= dvProAV_AccessWrite(eWrpOsdBmpWidth, sBitmap.sBitmap_Size.iWidth & 0xffff);  //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    status &= dvProAV_AccessWrite(eWrpOsdBmpHeight, sBitmap.sBitmap_Size.iHeight & 0xffff);  //BitBLT vertical direction width register

    // set read BMP position
    status &= dvProAV_AccessWrite(eWrpOsdBltAdr, DRAM_WRPOSD_BMP0_ADDR & 0xffffffff/*PWPOSD_SAD_BEFORE*/);                       // 設定Blt Function Address
    status &= dvProAV_AccessWrite(eWrpOsdBmpHStart, sBitmap.sSrc_Position.iX & 0xffff);
    status &= dvProAV_AccessWrite(eWrpOsdBmpVStart, sBitmap.sSrc_Position.iY & 0xffff);
    // set write BMP position
    //status &= dvProAV_AccessWrite(eOWMemWrAdr, PWPOSD_SAD_BEFORE);                     // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eWrpOsdBltVStart, sDes_position.iY & 0xffff);
    status &= dvProAV_AccessWrite(eWrpOsdBltHStart, sDes_position.iX & 0xffff);

    status &= dvProAV_AccessWrite(eWrpOsdTrig, eWPOSD_BitBLTTrigger);                //OSD control register  ==> BitBLT

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        #ifdef OSDBUSYTIMEOUT
            if (TimeElapsed(eTimerUser0) > OSDBUSYTIMEOUT)
            {
                DBMSG(" OSD Busy... \n");
                break;
            }
        #endif
        MS_SLEEP(1);
        retry--;
    }

    return status;
}

/**
 * @brief 畫單一圓型
 * @param [in] sDes_center : setting circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] iColor_Index : setting circle color
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note 圓最小半徑是10, 小於10會不圓, 不支援半徑大於原心case
 */
int dvProAV_WarpOSD_Paint_SingleCircle(START_POINT sDes_center, INT16 iRadius, uint16 iColor_Index)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if(iRadius == 0)
    {
        return rcERROR;
    }

    status &= dvProAV_AccessWrite(eWrpOsdCirRadius,(iRadius & 0xffff));         // 設定圓半徑
    status &= dvProAV_AccessWrite(eWrpOsdCirHCen, (uint16)sDes_center.iX);       // 設定圓心位置X
    status &= dvProAV_AccessWrite(eWrpOsdCirVCen, (uint16)sDes_center.iY);       // 設定圓心位置Y
    status &= dvProAV_AccessWrite(eWrpOsdBmpFillColor,iColor_Index & 0xffff);    // 設定顏色

    status &= dvProAV_AccessWrite(eWrpOsdTrig, eWPOSD_SigleCircleTrigger);    // 資料寫入DRAM

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        #ifdef OSDBUSYTIMEOUT
            if (TimeElapsed(eTimerUser0) > OSDBUSYTIMEOUT)
            {
                DBMSG(" OSD Busy... \n");
                break;
            }
        #endif
        MS_SLEEP(1);
        retry--;
    }

    return status;
}

/**
 * @brief 畫多個圓型
 * @param [in] sDes_center : setting top left circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] sPitch : setting pitch for circle center
 * @param [in] sLimit : setting paint multi Circle boundary
 * @param [in] iColor_Index : setting circle color
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Paint_MultiCircle(START_POINT sDes_center, INT16 iRadius, CENTER_PITCH sPitch, START_POINT sLimit, uint16 iColor_Index)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if(iRadius == 0)
    {
        return rcERROR;
    }

    status &= dvProAV_AccessWrite(eWrpOsdCirRadius,(iRadius & 0xffff));     // 設定圓半徑
    status &= dvProAV_AccessWrite(eWrpOsdCirHCen, (uint16)sDes_center.iX);  // 設定圓心位置X
    status &= dvProAV_AccessWrite(eWrpOsdCirVCen, (uint16)sDes_center.iY);  // 設定圓心位置Y
    status &= dvProAV_AccessWrite(eWrpOsdCirHP, (uint16)sPitch.idX);        // 設定X軸圓心之間Pitch
    status &= dvProAV_AccessWrite(eWrpOsdCirVP, (uint16)sPitch.idY);        // 設定Y軸圓心之間Pitch
    status &= dvProAV_AccessWrite(eWrpOsdCirHL, (uint16)sLimit.iX);         // 設定畫MultiCircle區域邊界X
    status &= dvProAV_AccessWrite(eWrpOsdCirVL, (uint16)sLimit.iY);         // 設定畫MultiCircle區域邊界Y
    status &= dvProAV_AccessWrite(eWrpOsdBmpFillColor,iColor_Index & 0xffff); // 設定顏色

    status &= dvProAV_AccessWrite(eWrpOsdTrig, eWPOSD_MultiCircleTrigger);// 資料寫入DRAM

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        #ifdef OSDBUSYTIMEOUT
            if (TimeElapsed(eTimerUser0) > OSDBUSYTIMEOUT)
            {
                DBMSG(" OSD Busy... \n");
                break;
            }
        #endif
        MS_SLEEP(1);
        retry--;
    }

    return status;
}

/**
 * @brief 畫點
 * @param [in] sDes_position : setting paint dot position
 * @param [in] cPointCount : setting count for paint X direction dot
 * @param [in] iColor_Index : setting color
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Paint_Dot(START_POINT sDes_position, UINT16 cPointCount, uint16 iColor_Index)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eWrpOsdBmpVStart, (uint16)sDes_position.iY);  // 設定矩形起始位置Y
    status &= dvProAV_AccessWrite(eWrpOsdBmpHStart, (uint16)sDes_position.iX);  // 設定矩形起始位置X
    for(uint16 i=0; i<cPointCount; i++)
    {
        status &= dvProAV_AccessWrite(eWrpOsdCpuData, (uint32)iColor_Index);
    }
    //status &= dvProAV_AccessBurstWrite(eWrpOsdCpuData , iColor_Index, cPointCount, BURST_FIX_ADDR);

    return status;
}

/**
 * @brief after & before WarpOSD Mix
 * @param [in] bEnable : true=on false=off
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_MixOn(bool bEnable)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eWrpOsdDbChEn, (uint16)bEnable);
    status &= dvProAV_AccessWrite(eDbdPathMode, (uint16)bEnable);

    return status;
}

/**
 * @brief WarpOSD Display
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_On(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if(ePage == eWPOSDPAGE_0)
    {
        PWPOSD_SAD_BEFORE = (PWPOSD_SAD_BEFORE == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
        status &= dvProAV_AccessWrite(eWrpOsdMemRdAdr, PWPOSD_SAD_BEFORE);

#if 0
        if(PWPOSD_SAD_BEFORE == PWPOSD_SAD1)
        {
            printf("PWPOSD_SAD_BEFORE = 1 0x%08x\n", PWPOSD_SAD1);
        }
        else
        {
            printf("PWPOSD_SAD_BEFORE = 2 0x%08x\n", PWPOSD_SAD2);
        }
#endif /* 0 */

    }
    else
    {
        PWPOSD_SAD_AFTER = (PWPOSD_SAD_AFTER == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
        status &= dvProAV_AccessWrite(eWrpOsdMem2RdAdr, PWPOSD_SAD_AFTER);

#if 0
        if(PWPOSD_SAD_AFTER == PWPOSD_SAD3)
        {
            printf("PWPOSD_SAD_AFTER = 3 0x%08x\n", PWPOSD_SAD3);
        }
        else
        {
            printf("PWPOSD_SAD_AFTER = 4 0x%08x\n", PWPOSD_SAD4);
        }
#endif /* 0 */
    }

    status &= dvProAV_AccessWrite(eWrpOsdEnable, (uint16)true);
    status &= dvProAV_AccessWrite(eWrpOsdTrig, (uint16)eWPOSD_DisplayOSDData);

    while((dvProAV_WarpOSD_BusyStateGet() != 0) && retry)  // busy
    {
        MS_SLEEP(1);
        retry--;
    }

    dvProAV_WarpOSD_MemplaneSel(ePage);
    dvProAV_WarpOSD_PageClear();

    return status;
}

/**
 * @brief Warp OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, uint16 ucInhibit_Color)
{
    int status = rcSUCCESS;
    uint32 ulGetOutputSize = 0;

    // 設定Memory位置
    PWPOSD_SAD = DRAM_WRPOSD_MEM1_ADDR;
    PWPOSD_SAD_BEFORE = DRAM_WRPOSD_MEM1_ADDR;     // WarpOSDSAD(current) point to current WarpOSD memory plane
    PWPOSD_SAD_AFTER = DRAM_WRPOSD_MEM3_ADDR;     // WarpOSDSAD(current) point to current WarpOSD memory plane
    PWPOSD_SAD1 = DRAM_WRPOSD_MEM1_ADDR;    // OSDSAD0 WarpOSD memory plane 0
    PWPOSD_SAD2 = DRAM_WRPOSD_MEM2_ADDR;    // OSDSAD1 WarpOSD memory plane 1
    PWPOSD_SAD3 = DRAM_WRPOSD_MEM3_ADDR;    // OSDSAD0 WarpOSD memory plane 0
    PWPOSD_SAD4 = DRAM_WRPOSD_MEM4_ADDR;    // OSDSAD1 WarpOSD memory plane 1

    // Read Output Size
    status &= dvProAV_AccessRead(eWcuMb2HSize, &ulGetOutputSize);
    m_panelWidth = (uint16)ulGetOutputSize;
    status &= dvProAV_AccessRead(eWcuMb2VSize, &ulGetOutputSize);
    m_panelHeight = (uint16)ulGetOutputSize;
    // High Color mode setting
    status &= dvProAV_WarpOSD_HighColorModeEn(false);
    // 設定調色盤
    status &= dvProAV_WarpOSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);
    // 清除Warp OSD畫面
    //status &= dvProAV_AccessRawDataWriteByte(0x17, 0x2a, 0);    // Disable Warp OSD Function trigger
    status &= dvProAV_AccessWrite(eWrpOsdEnable, false);
    status &= dvProAV_WarpOSD_LayerModeSet(eLayerMode_P0BeforeP1AfterWarp);
    status &= dvProAV_WarpOSD_MixOn(true);
    // 設定讀取Warp OSD資料的DRAM位置
    status &= dvProAV_AccessWrite(eWrpOsdStartAdr, PWPOSD_SAD_BEFORE);
    status &= dvProAV_AccessWrite(eWrpOsdMemRdAdr, PWPOSD_SAD_BEFORE);
    status &= dvProAV_AccessWrite(eWrpOsdMem2RdAdr, PWPOSD_SAD_AFTER);

    // 設定OSD Line Buffer Size display panel size
    status &= dvProAV_AccessWrite(eWrpOsdLineBufCnt, (m_panelWidth / PROAV_DRAM_LINE_BYTES));
    // 設定OSD Size
    status &= dvProAV_AccessWrite(eWrpOsdVidWidth, m_panelWidth);
    status &= dvProAV_AccessWrite(eWrpOsdVidHeight, m_panelHeight);
    // 設定禁止色
    status &= dvProAV_WarpOSD_Inhibit_Color_Set(true, ucInhibit_Color);
    // 設定透明色
    status &= dvProAV_AccessWrite(eWrpOsdTranColor0, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdTranColor1, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdTranColor2, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdTranColor3, ucInhibit_Color);

    // 切換OSD DRAM Buffer

    status &= dvProAV_WarpOSD_Swap(eWPOSDPAGE_0);
    status &= dvProAV_WarpOSD_PageClear();
    status &= dvProAV_WarpOSD_Swap(eWPOSDPAGE_0);
    status &= dvProAV_WarpOSD_PageClear();
    status &= dvProAV_WarpOSD_Swap(eWPOSDPAGE_1);
    status &= dvProAV_WarpOSD_PageClear();
    status &= dvProAV_WarpOSD_Swap(eWPOSDPAGE_1);
    status &= dvProAV_WarpOSD_PageClear();

    return status;
}

/**
 * @brief Inhibit Color Set
 * @param [in] cEnable : Warp OSD write-inhibit color enable (0 or 1)
 * @param [in] iInhibit_Color : Warp OSD write-inhibit color (color pallet number)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Inhibit_Color_Set(INT8 cEnable, uint16 iInhibit_Color)
{
    int status = rcSUCCESS;

    m_iInhitbit_Color = iInhibit_Color;

    status &= dvProAV_AccessWrite(eWrpOsdInhiColor, m_iInhitbit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdInhiColorEn, (uint16)cEnable);

    return status;
}

/**
 * @brief Set Layer Mode
 * @param [in] eMode :
 *              - eLayerMode_BothBeforeWarp
 *              - eLayerMode_P0BeforeP1AfterWarp
 *              - eLayerMode_PageAfterWarp
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_LayerModeSet(eLayerMode eMode)
{
    if(eMode >= eLayerMode_Max)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    return dvProAV_AccessWrite(eWrpOsdChSel, eMode);
}

/**
 * @brief High Color Mode Enable / Disable
 * @param [in] eMode :
 *              - false: Disable
 *              - true : Enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_HighColorModeEn(bool eMode)
{
    return dvProAV_AccessWrite(eWrpOsdHighColorEn, eMode);
}

void dvProAV_WarpOSD_DbgPrint(void)  //H2PF_Simon_0038
{
    LOG_MSG(db_ALWAYS, "\r\n======= WarpOSD =======\r\n");
    MS_SLEEP(5);

    LOG_MSG(db_ALWAYS, "PWPOSD_SAD 0x%X\r\n\r\n", PWPOSD_SAD);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "PWPOSD_SAD_BEFORE 0x%X\r\n", PWPOSD_SAD_BEFORE);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "PWPOSD_SAD1 0x%X\r\n", PWPOSD_SAD1);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "PWPOSD_SAD2 0x%X\r\n\r\n", PWPOSD_SAD2);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "PWPOSD_SAD_AFTER 0x%X\r\n", PWPOSD_SAD_AFTER);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "PWPOSD_SAD3 0x%X\r\n", PWPOSD_SAD3);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "PWPOSD_SAD4 0x%X\r\n", PWPOSD_SAD4);
    MS_SLEEP(5);

    LOG_MSG(db_ALWAYS, "===================\r\n\r\n");
    MS_SLEEP(5);

}

