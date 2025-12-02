#include "dvProAV_OSD.h"
#include "dvProAV_SerialFlash.h"
#ifndef QT_CPP
    #include "dvProAV.h"
#endif
#include "ProcessMutexData.h"     //H2PF_Simon_0040

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

#define PROAV_DISABLE_QUICK_OSD (1)

#define PROAV_OSD_ENABLE
#define OSDBUSYTIMEOUT 1000

#ifdef USE_SHM_POSD_SAD     //H2PF_Simon_0040
#define POSD_SAD  *utilProcMutexData_POSD_SAD_ShmPtrGet()
#else
static UINT32 POSD_SAD;     // OSDSAD(current) point to current OSD memory plane
#endif
static UINT32 POSD_SAD0;    // OSDSAD0 OSD memory plane 0
static UINT32 POSD_SAD1;    // OSDSAD1 OSD memory plane 1

#ifdef DMD_WUXGA
static UINT16 m_panelWidth = 1920;
static UINT16 m_panelHeight = 1200;
#else
static UINT16 m_panelWidth = 1920;
static UINT16 m_panelHeight = 1080;
#endif /* DMD_WUXGA */

static BOOL m_OSDTransparencyEn = false;
static UINT8 m_ucOSDTransparency = false;
static UINT16 m_iInhitbit_Color = 0xFFFF;

#ifdef FPGA_ENTRY_4K
//-------------------------------------------------------------------------------------------------
// Warp OSD For Entry 4K
//-------------------------------------------------------------------------------------------------
#ifdef DMD_WUXGA
static uint16 m_panelWidth_WP = 3840;
static uint16 m_panelHeight_WP = 2400;
#else
static uint16 m_panelWidth_WP = 3840;
static uint16 m_panelHeight_WP = 2160;
#endif /* DMD_WUXGA */

#ifdef USE_SHM_POSD_SAD   //H2PF_Simon_0040
#define PWPOSD_SAD *utilProcMutexData_PWPOSD_SAD_ShmPtrGet()
#else
static uint32 PWPOSD_SAD;    // PWPOSD_SAD_BEFORE point to current OSD memory plane berfor warping
#endif

//before warping
#ifdef USE_SHM_POSD_SAD   //H2PF_Simon_0040
#define PWPOSD_SAD_BEFORE *utilProcMutexData_PWPOSD_SAD_BEFORE_ShmPtrGet()
#else
static uint32 PWPOSD_SAD_BEFORE;    // PWPOSD_SAD_BEFORE point to current OSD memory plane berfor warping
#endif
static uint32 PWPOSD_SAD1;          // OSDSAD0 OSD memory plane 0
static uint32 PWPOSD_SAD2;          // OSDSAD1 OSD memory plane 1

//after warping
#ifdef USE_SHM_POSD_SAD   //H2PF_Simon_0040
#define PWPOSD_SAD_AFTER *utilProcMutexData_PWPOSD_SAD_AFTER_ShmPtrGet()
#else
static uint32 PWPOSD_SAD_AFTER;     // PWPOSD_SAD_AFTER point to current OSD memory plane after warping
#endif
static uint32 PWPOSD_SAD3;          // OSDSAD0 OSD memory plane 0
static uint32 PWPOSD_SAD4;          // OSDSAD1 OSD memory plane 1

static bool m_b4k3dEn = 0;
#endif /* FPGA_ENTRY_4K */

#ifndef QT_CPP
    // Serial Flash
    UINT32 POSD_SFL_PSAD;// OSD Part SAD
    UINT32 POSD_SFL_FSAD;// OSD Font SAD

    ePANEL_ID OSDePanelTimingId;    //A70LV_Doulas_0105

    static TickType_t ulOSDStartTicks;
#endif

//-------------------------------------------------------------------------------------------------
// Scaler OSD
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get OSD busy state
 * @return ulBusyStatus : 0=unbusy, 1=busy
 */
static bool dvProAV_OSD_BusyGet(void)
{
    UINT32 ulBusyStatus = 0;
#ifndef NO_INTERFACE
    int status = rcSUCCESS;
    status &= dvProAV_AccessRead(eOsdBusy, &ulBusyStatus);
#endif /* 0 */
    return (bool)ulBusyStatus;
}

/**
 * @brief Get OSD Vsync Wait state
 * @return ulBusyStatus : 0=unbusy, 1=busy
 */
static BOOL dvProAV_OSD_Vs_WaitStateGet(void)
{
    UINT32 ulBusyStatus = 0;
#ifndef NO_INTERFACE
    int status = rcSUCCESS;
    status &= dvProAV_AccessRead(eOsdVSBusy, &ulBusyStatus);
#endif /* 0 */
    return (BOOL)ulBusyStatus;
}

static void OSD_ResetTimeInState(void)
{
#ifndef QT_CPP
    ulOSDStartTicks = xTaskGetTickCount();
    return;
#endif
}

#ifndef QT_CPP
UINT32 OSD_TimeElapsedInState(void)
{
    TickType_t ulTicks;

    ulTicks = xTaskGetTickCount();

    if(ulOSDStartTicks > ulTicks)
    {
        return (0xFFFFFFFF - ulOSDStartTicks + 1 + ulTicks) / portTICK_RATE_MS;
    }
    else
    {
        return (ulTicks - ulOSDStartTicks) / portTICK_RATE_MS;
    }
}
#endif

static int OSD_DMA_SerialFlash_Set(UINT32 ulSflAd, UINT32 ulDestMemAd, UINT32 ulDmaCnt)
{
    int status = rcSUCCESS;
    if(ulDmaCnt == 0)
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    DBMSG("B %08x Add %08x, size %08x\n", ulSflAd, ulDestMemAd, ulDmaCnt);
    status &= dvProAV_SfiDram2Flash(ulDestMemAd, ulSflAd, ulDmaCnt, IO_READ);
    DBMSG("F %08x Add %08x, size %08x\n", ulSflAd, ulDestMemAd, ulDmaCnt);

    return status;
}

/**
 * @brief Swap DRAM buffer
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#if PROAV_DISABLE_QUICK_OSD
static int OSD_Swap_Memplane(void)
{
    int status = rcSUCCESS;
    UINT8  cData[5];

    POSD_SAD = (POSD_SAD != POSD_SAD0)? POSD_SAD0 : POSD_SAD1;      // Swap Memory Address
    PWPOSD_SAD_BEFORE = POSD_SAD;

    //LOG_MSG(db_HAL_WARPING, "(%s) POSD_SAD = %08x\n", __FUNCTION__, POSD_SAD);

#ifdef PROAV_OSD_ENABLE

    // 畫透明色
    status &= dvProAV_Inhibit_Color_Set(false, m_iInhitbit_Color);  // 關閉禁止色
#if 0   // 矩形方式畫透明色
    START_POINT sStart;
    RECT_SIZE sSize;

    sStart.iX = 0;
    sStart.iY = 0;
    sSize.iWidth = (int16)m_panelWidth;
    sSize.iHeight = (int16)m_panelHeight;

    status &= dvProAV_Paint_Rectangle(sStart, sSize, m_iInhitbit_Color);
#else
    uint32 uiTranNum = (((OSD_H_MAX * OSD_V_MAX) + (32 * PROAV_DRAM_LINE_BYTES) - 1) / (32 * PROAV_DRAM_LINE_BYTES));     // (Hsize*Vsize)/(一次寫入32個DRAM位置*DRAM位置Byte數), 無條件進位
    status &= dvProAV_AccessWrite(eOsdTranColor0, (uint16)m_iInhitbit_Color);       // 設定透明色顏色
    status &= dvProAV_AccessWrite(eOsdBmpFillClr, (uint16)m_iInhitbit_Color);       // 設定填入顏色
    status &= dvProAV_AccessWrite(eOsdTranNum, uiTranNum);                          // 設定尺寸
#if 0   // signal mode
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);                          // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eOsdTranTrig, true);                              // 開始寫入
#else   // burst mode
    cData[0] = (POSD_SAD & 0xff);                                                   // 設定寫入DRAM address
    cData[1] = (POSD_SAD >> 8) & 0xff;
    cData[2] = (POSD_SAD >> 16) & 0xff;
    cData[3] = (POSD_SAD >> 24) & 0xff;
    cData[4] = (eOsdTranTrig) & 0xff;                                               // 開始寫入
    status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
#endif
    OSD_ResetTimeInState();

    while((dvProAV_OSD_Vs_WaitStateGet() != 0) || (dvProAV_OSD_BusyGet() != 0))  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("OSD_Swap_Memplane Time Out\n");
           break;
        }
#endif
     }

#endif

#endif /* PROAV_OSD_ENABLE */
    ASSERT(status == rcSUCCESS);

    status &= dvProAV_Inhibit_Color_Set(true, m_iInhitbit_Color);  // 開啟禁止色

    return status;
}
#else
static int OSD_Swap_Memplane(void)
{
    int status = rcSUCCESS;
    UINT8  cData[22];

    POSD_SAD = (POSD_SAD != POSD_SAD0)? POSD_SAD0 : POSD_SAD1;      // Swap Memory Address
    PWPOSD_SAD_BEFORE = POSD_SAD;

    //LOG_MSG(db_HAL_WARPING, "(%s) POSD_SAD = %08x\n", __FUNCTION__, POSD_SAD);

#ifdef PROAV_OSD_ENABLE

    // 畫透明色
    status &= dvProAV_Inhibit_Color_Set(false, m_iInhitbit_Color);  // 關閉禁止色
#if 0   // 矩形方式畫透明色
    START_POINT sStart;
    RECT_SIZE sSize;

    sStart.iX = 0;
    sStart.iY = 0;
    sSize.iWidth = (int16)OSD_H_MAX;
    sSize.iHeight = (int16)OSD_V_MAX;

    status &= dvProAV_Paint_Rectangle(sStart, sSize, m_iInhitbit_Color);
#else
    uint32 uiTranNum = (((OSD_H_MAX * OSD_V_MAX) + (32 * PROAV_DRAM_LINE_BYTES) - 1) / (32 * PROAV_DRAM_LINE_BYTES));     // (Hsize*Vsize)/(一次寫入32個DRAM位置*DRAM位置Byte數), 無條件進位
    cData[0] = ((eOsdTranColor0 >> 8) & 0xff);
    cData[1] = m_iInhitbit_Color & 0xff;
    cData[2] = (((eOsdTranColor0 >> 8) + 1) & 0xff);
    cData[3] = (m_iInhitbit_Color >> 8) & 0xff;
    cData[4] = ((eOsdBmpFillClr >> 8) & 0xff);
    cData[5] = m_iInhitbit_Color & 0xff;
    cData[6] = (((eOsdBmpFillClr >> 8) + 1) & 0xff);
    cData[7] = (m_iInhitbit_Color >> 8) & 0xff;
    cData[8] = ((eOsdTranNum >> 8) & 0xff);
    cData[9] = uiTranNum & 0xff;
    cData[10] = (((eOsdTranNum >> 8) + 1) & 0xff);
    cData[11] = (uiTranNum >> 8) & 0xff;
    //status &= dvProAV_AccessWrite(eOsdTranColor0, (uint16)m_iInhitbit_Color);       // 設定透明色顏色
    //status &= dvProAV_AccessWrite(eOsdBmpFillClr, (uint16)m_iInhitbit_Color);       // 設定填入顏色
    //status &= dvProAV_AccessWrite(eOsdTranNum, uiTranNum);                          // 設定尺寸
#if 0   // signal mode
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);                          // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eOsdTranTrig, true);                              // 開始寫入
#else   // burst mode
    cData[12] = (eOsdMemWrAdr >> 8) & 0xff;
    cData[13] = (POSD_SAD & 0xff);                                                   // 設定寫入DRAM address
    cData[14] = ((eOsdMemWrAdr >> 8) + 1) & 0xff;
    cData[15] = (POSD_SAD >> 8) & 0xff;
    cData[16] = ((eOsdMemWrAdr >> 8) + 2) & 0xff;
    cData[17] = (POSD_SAD >> 16) & 0xff;
    cData[18] = ((eOsdMemWrAdr >> 8) + 3) & 0xff;
    cData[19] = (POSD_SAD >> 24) & 0xff;
    cData[20] = (eOsdTranTrig >> 8) & 0xff;
    cData[21] = (eOsdTranTrig) & 0xff;                                               // 開始寫入
    status &= dvProAV_AccessBurstWrite(eOsdQuickWr_E4k, cData, sizeof (cData), BURST_FIX_ADDR);
#endif
    OSD_ResetTimeInState();

    while((dvProAV_OSD_Vs_WaitStateGet() != 0) || (dvProAV_OSD_BusyGet() != 0))  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("OSD_Swap_Memplane Time Out\n");
           break;
        }
#endif
     }

#endif

#endif /* PROAV_OSD_ENABLE */
    ASSERT(status == rcSUCCESS);

    status &= dvProAV_Inhibit_Color_Set(true, m_iInhitbit_Color);  // 開啟禁止色

    return status;
}
#endif

/**
 * @brief Paint String
 * @param [in] sDes_position : String start position
 * @param [in] sString : String attribute
 * @param [in] iFont_Color : Font Color
 * @param [in] iBG_Color : Font Back Ground Color
 * @param [in] iStringOffset :
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#if PROAV_DISABLE_QUICK_OSD
int dvProAV_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset)
{
    int status = rcSUCCESS;
    UINT8 cVNum = 1;
    UINT16 iCount;
    UINT32 ulWad;
    UINT16 dynamicMemorySize = (sString.uiStringLength * 5);
    UINT16 uiCharIndex = 0;
    uint08 *pCharData = NULL;
    UINT16 uiLen = 0;
    UINT16 uiTotalLen = 0;
    UINT8 cData[5];

    if(dynamicMemorySize == 0)
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

#ifdef PROAV_OSD_ENABLE
    #if 0   // signal mode
    // 設定寫字串區域Size
    status &= dvProAV_AccessWrite(eOsdCharVNum, cVNum);                                 // Character buffer vertical direction width
    status &= dvProAV_AccessWrite(eOsdCharHNum, (uint8)(sString.uiStringLength + 1));   // Character buffer horizontal direction width
    status &= dvProAV_AccessWrite(eOsdCharHeight, (uint8)sString.ucStringMaxHeight);    // 設定字串高度
    // 設定寫字串位置
    ulWad = POSD_SAD + ((uint16)sDes_position.iX / PROAV_DRAM_LINE_BYTES) + ((uint16)sDes_position.iY * (m_panelWidth / PROAV_DRAM_LINE_BYTES));
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, (uint32)ulWad);     // 設定寫入位置
    ulWad = sDes_position.iX % PROAV_DRAM_LINE_BYTES;
    status &= dvProAV_AccessWrite(eOsdFontBlank, (uint8)ulWad);     // 設定字串偏移位置
    status &= dvProAV_AccessWrite(eOsdFontAttriRst, true);          // reset character attribute
    // Write character attribute
    for(iCount=0; iCount<(sString.uiStringLength); iCount++)
    {
        dvProAV_AccessWrite(eOsdCharBufData, (sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) & 0xff);
        dvProAV_AccessWrite(eOsdCharBufData, ((sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) >> 8) & 0xff);
        dvProAV_AccessWrite(eOsdCharBufData, iFont_Color & 0xff);
        dvProAV_AccessWrite(eOsdCharBufData, iBG_Color & 0xff);
        dvProAV_AccessWrite(eOsdCharBufData, *(sString.pucCharWdith + iCount + iStringOffset) & 0xff); //set each character width.
    }
    uiCharIndex = 64;
    // 補空白
    dvProAV_AccessWrite(eOsdCharBufData, uiCharIndex & 0xff);
    dvProAV_AccessWrite(eOsdCharBufData, (uiCharIndex >> 8) & 0xff);
    dvProAV_AccessWrite(eOsdCharBufData, iFont_Color & 0xff);
    dvProAV_AccessWrite(eOsdCharBufData, iBG_Color & 0xff);
    dvProAV_AccessWrite(eOsdCharBufData, 5);        //set each character width.

    status &= dvProAV_AccessWrite(eOsdMemWrAdr, (uint32)ulWad);     // 設定寫入位置
    status &= dvProAV_AccessWrite(eOsdFontTrig, true);              // 字串資料寫入DRAM
    #else   // burst mode
    // 設定寫字串區域Size
    cData[0] = cVNum;                               // eOsdCharVNum, Character buffer vertical direction width
    cData[1] = (sString.uiStringLength + 1) & 0xff; // eOsdCharHNum, Character buffer horizontal direction width
    cData[2] = sString.ucStringMaxHeight & 0xff;    // eOsdCharHeight, 設定字串高度
    // 設定寫字串位置
    ulWad = POSD_SAD + ((uint16)sDes_position.iX / PROAV_DRAM_LINE_BYTES) + ((uint16)sDes_position.iY * (m_panelWidth / PROAV_DRAM_LINE_BYTES));
    cData[3] = (uint8)(sDes_position.iX % PROAV_DRAM_LINE_BYTES); // eOsdFontBlank, 設定字串偏移位置
    cData[4] = 0x01;                                // eOsdFontAttriRst, reset character attribute
    status &= dvProAV_AccessBurstWrite(eOsdCharVNum , (uint08 *)cData, 5, BURST_INC_ADDR);

    // Write character attribute
    pCharData = (UINT8 *)malloc(dynamicMemorySize + 10);
    if(pCharData == NULL)
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

    for(iCount = 0; iCount < (sString.uiStringLength); iCount++)
    {
        uiCharIndex = (UINT16)(sString.ulFontOffset + *(sString.puiCharIndex + iCount + iStringOffset));
        #ifndef QT_CPP
        uiCharIndex = uiCharIndex * 2;
        #endif
        pCharData[(iCount * 5) + 0] = uiCharIndex & 0xff;
        pCharData[(iCount * 5) + 1] = (uiCharIndex >> 8) & 0xff;
        pCharData[(iCount * 5) + 2] = iFont_Color & 0xff;
        pCharData[(iCount * 5) + 3] = iBG_Color & 0xff;
        pCharData[(iCount * 5) + 4] = *(sString.pucCharWdith + iCount + iStringOffset) & 0xff;
    }

    iCount = iCount * 5;
    uiCharIndex = 64;
    // 補空白
    pCharData[iCount++] = uiCharIndex & 0xff;
    pCharData[iCount++] = (uiCharIndex >> 8) & 0xff;
    pCharData[iCount++] = iFont_Color & 0xff;
    pCharData[iCount++] = iBG_Color & 0xff;
    pCharData[iCount++] = 5;
    uiTotalLen = iCount;
    ASSERT(uiTotalLen < (dynamicMemorySize + 10));
    status &= dvProAV_AccessBurstWrite(eOsdCharBufData, &pCharData[0], uiTotalLen, BURST_FIX_ADDR);

    // 設定寫入位置
    cData[0] = (ulWad & 0xff);
    cData[1] = (ulWad >> 8) & 0xff;
    cData[2] = (ulWad >> 16) & 0xff;
    cData[3] = (ulWad >> 24) & 0xff;
    // 字串資料寫入DRAM
    cData[4] = (eOsdFontTrig) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
    #endif
    free(pCharData);
    OSD_ResetTimeInState();
    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
        #ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_String Time Out\n");
           break;
        }
        #endif
    }
#endif /* PROAV_OSD_ENABLE */

    return status;
}
#else
INT8 dvProAV_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset)
{
    int status = rcSUCCESS;
    UINT8 cVNum = 1;
    UINT16 iCount = 0;
    UINT32 ulWad;
    UINT16 dynamicMemorySize = (sString.uiStringLength * 5);
    UINT16 uiCharIndex = 0;
    uint08 *pCharData = NULL;
    UINT16 uiTotalLen = 0;

#if 1
    if(dynamicMemorySize == 0)
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

    // 設定寫字串位置
    ulWad = POSD_SAD + ((uint16)sDes_position.iX / PROAV_DRAM_LINE_BYTES) + ((uint16)sDes_position.iY * (m_panelWidth / PROAV_DRAM_LINE_BYTES));

    // Write character attribute
    pCharData = (UINT8 *)malloc((dynamicMemorySize + 10 + 10) * 2);

    pCharData[iCount++] = 0x22;        // eOsdCharVNum, Character buffer vertical direction width
    pCharData[iCount++] = cVNum;
    pCharData[iCount++] = 0x23;        // eOsdCharHNum, Character buffer horizontal direction width
    pCharData[iCount++] = (sString.uiStringLength + 1) & 0xff;
    pCharData[iCount++] = 0x24;        // eOsdCharHeight, 設定字串高度
    pCharData[iCount++] = sString.ucStringMaxHeight & 0xff;
    pCharData[iCount++] = 0x25;        // eOsdFontBlank, 設定字串偏移位置
    pCharData[iCount++] = (uint8)(sDes_position.iX % PROAV_DRAM_LINE_BYTES);
    pCharData[iCount++] = 0x26;        // eOsdFontAttriRst, reset character attribute
    pCharData[iCount++] = 0x01;

    for(UINT16 iCnt = 0; iCnt < (sString.uiStringLength); iCnt++)
    {
        uiCharIndex = (UINT16)(sString.ulFontOffset + *(sString.puiCharIndex + iCnt + iStringOffset));
        #ifndef QT_CPP
        uiCharIndex = uiCharIndex * 2;
        #endif
        pCharData[iCount++] = 0x2C;
        pCharData[iCount++] = uiCharIndex & 0xff;
        pCharData[iCount++] = 0x2C;
        pCharData[iCount++] = (uiCharIndex >> 8) & 0xff;
        pCharData[iCount++] = 0x2C;
        pCharData[iCount++] = iFont_Color & 0xff;
        pCharData[iCount++] = 0x2C;
        pCharData[iCount++] = iBG_Color & 0xff;
        pCharData[iCount++] = 0x2C;
        pCharData[iCount++] = *(sString.pucCharWdith + iCnt + iStringOffset) & 0xff;
    }

    //iCount = iCount * 10;
    uiCharIndex = 64;
    // 補空白
    pCharData[iCount++] = 0x2C;
    pCharData[iCount++] = uiCharIndex & 0xff;
    pCharData[iCount++] = 0x2C;
    pCharData[iCount++] = (uiCharIndex >> 8) & 0xff;
    pCharData[iCount++] = 0x2C;
    pCharData[iCount++] = iFont_Color & 0xff;
    pCharData[iCount++] = 0x2C;
    pCharData[iCount++] = iBG_Color & 0xff;
    pCharData[iCount++] = 0x2C;
    pCharData[iCount++] = 5;
    // 設定寫入位置
    pCharData[iCount++] = 0x43;
    pCharData[iCount++] = (ulWad & 0xff);
    pCharData[iCount++] = 0x44;
    pCharData[iCount++] = (ulWad >> 8) & 0xff;
    pCharData[iCount++] = 0x45;
    pCharData[iCount++] = (ulWad >> 16) & 0xff;
    pCharData[iCount++] = 0x46;
    pCharData[iCount++] = (ulWad >> 24) & 0xff;
    // 字串資料寫入DRAM
    pCharData[iCount++] = 0x47;
    pCharData[iCount++] = (eOsdFontTrig) & 0xff;

    status &= dvProAV_AccessBurstWrite(eOsdQuickWr_E4k, &pCharData[0], iCount, BURST_FIX_ADDR);

    //printf("(%s, %d)Time %d\n",__FUNCTION__, __LINE__, OSD_TimeElapsedInState());
    free(pCharData);
    OSD_ResetTimeInState();
    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
        #ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_String Time Out\n");
           break;
        }
        #endif
    }
    //printf("(%s, %d)Time %d\n",__FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    return status;
#endif
}
#endif

/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#if PROAV_DISABLE_QUICK_OSD
int dvProAV_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index)
{
    int status = rcSUCCESS;
    uint8 cData[10];

    if((sDes_Size.iWidth == 0) || (sDes_Size.iHeight == 0))
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

#ifdef PROAV_OSD_ENABLE
#if 0   // single mode
    status &= dvProAV_AccessWrite(eOsdBmpFillClr,iColor_Index & 0xff);          // 設定矩形顏色
    status &= dvProAV_AccessWrite(eOsdBmpHeight,(sDes_Size.iHeight & 0xffff));  // 設定矩形高度
    status &= dvProAV_AccessWrite(eOsdBmpWidth,(sDes_Size.iWidth & 0xffff));    // 設定矩形寬度
    status &= dvProAV_AccessWrite(eOsdBmpVStart, (uint16)sDes_position.iY);     // 設定矩形位置Y
    status &= dvProAV_AccessWrite(eOsdBmpHStart, (uint16)sDes_position.iX);     // 設定矩形位置X
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);                      // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eOsdRectTrig, true);                          // 矩形資料寫入DRAM
#else   // burst mode
    cData[0] = iColor_Index & 0xff;                                             // 設定矩形顏色
    cData[1] = (iColor_Index >> 8) & 0xff;
    cData[2] = sDes_Size.iHeight & 0xff;                                        // 設定矩形高度
    cData[3] = (sDes_Size.iHeight >> 8) & 0xff;
    cData[4] = sDes_Size.iWidth & 0xff;                                         // 設定矩形寬度
    cData[5] = (sDes_Size.iWidth >> 8) & 0xff;
    cData[6] = sDes_position.iY & 0xFF;                                         // 設定矩形位置Y
    cData[7] = (sDes_position.iY >> 8) & 0xff;
    cData[8] = sDes_position.iX & 0xFF;                                         // 設定矩形位置X
    cData[9] = (sDes_position.iX >> 8) & 0xff;

    status &= dvProAV_AccessBurstWrite(eOsdBmpFillClr , (uint08 *)cData, 10, BURST_INC_ADDR); // Fixed Address if last parameter is false

    cData[0] = (POSD_SAD & 0xff);                                               // 設定寫入DRAM address
    cData[1] = (POSD_SAD >> 8) & 0xff;
    cData[2] = (POSD_SAD >> 16) & 0xff;
    cData[3] = (POSD_SAD >> 24) & 0xff;
    cData[4] = (eOsdRectTrig) & 0xff;                                           // 矩形資料寫入DRAM

    status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
#endif
    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_Rectangle Time Out 4\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */

    ASSERT(status == rcSUCCESS);

    return status;
}
#else
INT8 dvProAV_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index)
{
#if 1
    int status = rcSUCCESS;
    uint8 cData[30];

    if((sDes_Size.iWidth == 0) || (sDes_Size.iHeight == 0))
    {
        ASSERT_ALWAYS();
        return rcERROR;
    }

#ifdef PROAV_OSD_ENABLE
#if 0   // single mode
    status &= dvProAV_AccessWrite(eOsdBmpFillClr,iColor_Index & 0xff);          // 設定矩形顏色
    status &= dvProAV_AccessWrite(eOsdBmpHeight,(sDes_Size.iHeight & 0xffff));  // 設定矩形高度
    status &= dvProAV_AccessWrite(eOsdBmpWidth,(sDes_Size.iWidth & 0xffff));    // 設定矩形寬度
    status &= dvProAV_AccessWrite(eOsdBmpVStart, (uint16)sDes_position.iY);     // 設定矩形位置Y
    status &= dvProAV_AccessWrite(eOsdBmpHStart, (uint16)sDes_position.iX);     // 設定矩形位置X
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);                      // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eOsdRectTrig, true);                          // 矩形資料寫入DRAM
#else   // burst mode
    cData[0] = (eOsdBmpFillClr >> 8) & 0xff;
    cData[1] = iColor_Index & 0xff;                                             // 設定矩形顏色
    cData[2] = ((eOsdBmpFillClr >> 8) + 1) & 0xff;
    cData[3] = (iColor_Index >> 8) & 0xff;

    cData[4] = (eOsdBmpHeight >> 8) & 0xff;
    cData[5] = sDes_Size.iHeight & 0xff;                                        // 設定矩形高度
    cData[6] = ((eOsdBmpHeight >> 8) + 1) & 0xff;
    cData[7] = (sDes_Size.iHeight >> 8) & 0xff;

    cData[8] = (eOsdBmpWidth >> 8) & 0xff;
    cData[9] = sDes_Size.iWidth & 0xff;                                         // 設定矩形寬度
    cData[10] = ((eOsdBmpWidth >> 8) + 1) & 0xff;
    cData[11] = (sDes_Size.iWidth >> 8) & 0xff;

    cData[12] = (eOsdBmpVStart >> 8) & 0xff;
    cData[13] = sDes_position.iY & 0xFF;                                         // 設定矩形位置Y
    cData[14] = ((eOsdBmpVStart >> 8) + 1) & 0xff;
    cData[15] = (sDes_position.iY >> 8) & 0xff;

    cData[16] = (eOsdBmpHStart >> 8) & 0xff;
    cData[17] = sDes_position.iX & 0xFF;                                         // 設定矩形位置X
    cData[18] = ((eOsdBmpHStart >> 8) + 1) & 0xff;
    cData[19] = (sDes_position.iX >> 8) & 0xff;

    //status &= dvProAV_AccessBurstWrite(eOsdBmpFillClr , (uint08 *)cData, 10, BURST_INC_ADDR);

    cData[20] = (eOsdMemWrAdr >> 8) & 0xff;
    cData[21] = (POSD_SAD & 0xff);                                               // 設定寫入DRAM address
    cData[22] = ((eOsdMemWrAdr >> 8) + 1) & 0xff;
    cData[23] = (POSD_SAD >> 8) & 0xff;
    cData[24] = ((eOsdMemWrAdr >> 8) + 2) & 0xff;
    cData[25] = (POSD_SAD >> 16) & 0xff;
    cData[26] = ((eOsdMemWrAdr >> 8) + 3) & 0xff;
    cData[27] = (POSD_SAD >> 24) & 0xff;
    cData[28] = (eOsdRectTrig >> 8) & 0xff;
    cData[29] = (eOsdRectTrig) & 0xff;                                           // 矩形資料寫入DRAM

    status &= dvProAV_AccessBurstWrite(eOsdQuickWr_E4k, &cData[0], sizeof (cData), BURST_FIX_ADDR);
#endif
    //printf("(%s, %d)Time %d\n",__FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_Rectangle Time Out 4\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */
    //printf("(%s, %d)Time %d\n",__FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    ASSERT(status == rcSUCCESS);

    return status;
#endif
}
#endif
/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] sBitmap : Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#if PROAV_DISABLE_QUICK_OSD
int dvProAV_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;
    UINT16 iSrc_x = 0, iSrc_y = 0;
    UINT32 ulRad = 0;
    UINT32 ulWad = 0;
    uint8  cData[16];
    UINT16 iWidth = 0, iHeight = 0;

#ifdef PROAV_OSD_ENABLE

    iSrc_x = sBitmap.sSrc_Position.iX;
    iSrc_y = sBitmap.sSrc_Position.iY;

	iWidth = (sDes_position.iX+sBitmap.sBitmap_Size.iWidth <= m_panelWidth)? sBitmap.sBitmap_Size.iWidth : (sDes_position.iX+sBitmap.sBitmap_Size.iWidth-m_panelWidth);
	iHeight = (sDes_position.iY+sBitmap.sBitmap_Size.iHeight <= m_panelHeight)? sBitmap.sBitmap_Size.iHeight : (sDes_position.iY+sBitmap.sBitmap_Size.iHeight-m_panelHeight);
#if 0   // siganl mode
    // set size
    status &= dvProAV_AccessWrite(eOsdBmpHeight, sBitmap.sBitmap_Size.iHeight & 0xffff);    // set Bmp Height
    status &= dvProAV_AccessWrite(eOsdBmpWidth, sBitmap.sBitmap_Size.iWidth & 0xffff);      // set Bmp Width
    // set read position
    ulRad = DRAM_OSD_BMP_ADDR + ((uint32)sBitmap.sSrc_Position.iX / PROAV_DRAM_LINE_BYTES) + ((uint32)sBitmap.sSrc_Position.iY * (OSD_BITMAP_RAW_WIDTH / PROAV_DRAM_LINE_BYTES));
    iSrc_x = iSrc_x % PROAV_DRAM_LINE_BYTES;
    iSrc_y = 0;
    status &= dvProAV_AccessWrite(eOsdBmpVStart, iSrc_y & 0xffff);          // set Bmp read V start
    status &= dvProAV_AccessWrite(eOsdBmpHStart, iSrc_x & 0xffff);          // set Bmp read H start
    status &= dvProAV_AccessWrite(eOsdBltAdr, ulRad);                       // set Bmp read DRAM address
    // set write position
    ulWad = POSD_SAD + ((uint16)sDes_position.iX / PROAV_DRAM_LINE_BYTES) + ((uint16)sDes_position.iY * (m_panelWidth / PROAV_DRAM_LINE_BYTES));
    sDes_position.iX = sDes_position.iX % PROAV_DRAM_LINE_BYTES;
    sDes_position.iY = 0;
    status &= dvProAV_AccessWrite(eOsdBltHStart, sDes_position.iX & 0xffff);// set Bmp write V start
    status &= dvProAV_AccessWrite(eOsdBltVStart, sDes_position.iY & 0xffff);// set Bmp write H start
    // enable BMP size
    status &= dvProAV_AccessWrite(eOsdImageEn, true);   // 特殊OSD size從圖庫讀取需讀取原圖size, 否則OSD畫面會錯
    // set write addr
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, ulWad);                     // set Bmp write DRAM address
    // BLT trigger
    status &= dvProAV_AccessWrite(eOsdBmpTrig, true);   // OSD control register  ==> BitBLT
#else   // burst mode
    // set size
    cData[0] = iHeight & 0xff;         // set Bmp Height
    cData[1] = (iHeight >> 8) & 0xff;
    cData[2] = iWidth & 0xff;          // set Bmp Width
    cData[3] = (iWidth >> 8) & 0xff;
    // set read position
    ulRad = DRAM_OSD_BMP_ADDR;
    cData[4] = iSrc_y & 0xff;           // set Bmp read V start
    cData[5] = (iSrc_y >> 8) & 0xff;
    cData[6] = iSrc_x & 0xff;           // set Bmp read H start
    cData[7] = (iSrc_x >> 8) & 0xff;
    // set read addr
    cData[8] = ulRad & 0xff;            // set Bmp read DRAM Address
    cData[9] = (ulRad >> 8) & 0xff;
    cData[10] = (ulRad >> 16) & 0xff;
    cData[11] = (ulRad >> 24) & 0xff;
    // set write position
    ulWad = POSD_SAD;
    cData[12] = sDes_position.iY & 0xFF;                // set Bmp write V start
    cData[13] = (sDes_position.iY >> 8) & 0xff;
    cData[14] = sDes_position.iX & 0xFF;                // set Bmp write H start
    cData[15] = (sDes_position.iX >> 8) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdBmpHeight , (uint08 *)cData, 16, BURST_INC_ADDR);

    // enable BMP size
    status &= dvProAV_AccessWrite(eOsdImageEn, true);   // 特殊OSD size從圖庫讀取需讀取原圖size, 否則OSD畫面會錯

    // set write addr
    cData[0] = (ulWad & 0xff);
    cData[1] = (ulWad >> 8) & 0xff;
    cData[2] = (ulWad >> 16) & 0xff;
    cData[3] = (ulWad >> 24) & 0xff;
    // BLT trigger
    cData[4] = (eOsdBmpTrig) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
#endif

    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_Bitmap Time Out 3\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */
    ASSERT(status == rcSUCCESS);
    //printf("(%s, %d)Time %d\n",__FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    status &= dvProAV_AccessWrite(eOsdImageEn, false);  // BitBLT完成須關閉, 否則會影響其他OSD功能

    return status;
}
#else
INT8 dvProAV_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;
    UINT16 iSrc_x = 0, iSrc_y = 0;
    UINT32 ulRad = 0;
    UINT32 ulWad = 0;
    uint8  cData[44];
    UINT16 iWidth = 0, iHeight = 0;

#ifdef PROAV_OSD_ENABLE

    iSrc_x = sBitmap.sSrc_Position.iX;
    iSrc_y = sBitmap.sSrc_Position.iY;

    iWidth = (sDes_position.iX+sBitmap.sBitmap_Size.iWidth <= m_panelWidth)? sBitmap.sBitmap_Size.iWidth : (sDes_position.iX+sBitmap.sBitmap_Size.iWidth-m_panelWidth);
    iHeight = (sDes_position.iY+sBitmap.sBitmap_Size.iHeight <= m_panelHeight)? sBitmap.sBitmap_Size.iHeight : (sDes_position.iY+sBitmap.sBitmap_Size.iHeight-m_panelHeight);
#if 0   // siganl mode
    // set size
    status &= dvProAV_AccessWrite(eOsdBmpHeight, sBitmap.sBitmap_Size.iHeight & 0xffff);    // set Bmp Height
    status &= dvProAV_AccessWrite(eOsdBmpWidth, sBitmap.sBitmap_Size.iWidth & 0xffff);      // set Bmp Width
    // set read position
    ulRad = DRAM_OSD_BMP_ADDR + ((uint32)sBitmap.sSrc_Position.iX / PROAV_DRAM_LINE_BYTES) + ((uint32)sBitmap.sSrc_Position.iY * (OSD_BITMAP_RAW_WIDTH / PROAV_DRAM_LINE_BYTES));
    iSrc_x = iSrc_x % PROAV_DRAM_LINE_BYTES;
    iSrc_y = 0;
    status &= dvProAV_AccessWrite(eOsdBmpVStart, iSrc_y & 0xffff);          // set Bmp read V start
    status &= dvProAV_AccessWrite(eOsdBmpHStart, iSrc_x & 0xffff);          // set Bmp read H start
    status &= dvProAV_AccessWrite(eOsdBltAdr, ulRad);                       // set Bmp read DRAM address
    // set write position
    ulWad = POSD_SAD + ((uint16)sDes_position.iX / PROAV_DRAM_LINE_BYTES) + ((uint16)sDes_position.iY * (m_panelWidth / PROAV_DRAM_LINE_BYTES));
    sDes_position.iX = sDes_position.iX % PROAV_DRAM_LINE_BYTES;
    sDes_position.iY = 0;
    status &= dvProAV_AccessWrite(eOsdBltHStart, sDes_position.iX & 0xffff);// set Bmp write V start
    status &= dvProAV_AccessWrite(eOsdBltVStart, sDes_position.iY & 0xffff);// set Bmp write H start
    // enable BMP size
    status &= dvProAV_AccessWrite(eOsdImageEn, true);   // 特殊OSD size從圖庫讀取需讀取原圖size, 否則OSD畫面會錯
    // set write addr
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, ulWad);                     // set Bmp write DRAM address
    // BLT trigger
    status &= dvProAV_AccessWrite(eOsdBmpTrig, true);   // OSD control register  ==> BitBLT
#else   // burst mode
    // set size
    cData[0] = (eOsdBmpHeight >> 8) & 0xff;
    cData[1] = iHeight & 0xff;         // set Bmp Height
    cData[2] = ((eOsdBmpHeight >> 8) + 1) & 0xff;
    cData[3] = (iHeight >> 8) & 0xff;
    cData[4] = (eOsdBmpWidth >> 8) & 0xff;
    cData[5] = iWidth & 0xff;          // set Bmp Width
    cData[6] = ((eOsdBmpWidth >> 8) + 1) & 0xff;
    cData[7] = (iWidth >> 8) & 0xff;
    // set read position
    ulRad = DRAM_OSD_BMP_ADDR;
    cData[8] = (eOsdBmpVStart >> 8) & 0xff;
    cData[9] = iSrc_y & 0xff;           // set Bmp read V start
    cData[10] = ((eOsdBmpVStart >> 8) + 1) & 0xff;
    cData[11] = (iSrc_y >> 8) & 0xff;
    cData[12] = (eOsdBmpHStart >> 8) & 0xff;
    cData[13] = iSrc_x & 0xff;           // set Bmp read H start
    cData[14] = ((eOsdBmpHStart >> 8) + 1) & 0xff;
    cData[15] = (iSrc_x >> 8) & 0xff;
    // set read addr
    cData[16] = (eOsdBltAdr >> 8) & 0xff;
    cData[17] = ulRad & 0xff;            // set Bmp read DRAM Address
    cData[18] = ((eOsdBltAdr >> 8) + 1) & 0xff;
    cData[19] = (ulRad >> 8) & 0xff;
    cData[20] = ((eOsdBltAdr >> 8) + 2) & 0xff;
    cData[21] = (ulRad >> 16) & 0xff;
    cData[22] = ((eOsdBltAdr >> 8) + 3) & 0xff;
    cData[23] = (ulRad >> 24) & 0xff;
    // set write position
    ulWad = POSD_SAD;
    cData[24] = (eOsdBltHStart >> 8) & 0xff;
    cData[25] = sDes_position.iX & 0xFF;                // set Bmp write V start
    cData[26] = ((eOsdBltHStart >> 8) + 1) & 0xff;
    cData[27] = (sDes_position.iX >> 8) & 0xff;
    cData[28] = (eOsdBltVStart >> 8) & 0xff;
    cData[29] = sDes_position.iY & 0xFF;                // set Bmp write H start
    cData[30] = ((eOsdBltVStart >> 8) + 1) & 0xff;
    cData[31] = (sDes_position.iY >> 8) & 0xff;
    //status &= dvProAV_AccessBurstWrite(eOsdBmpHeight , (uint08 *)cData, 16, BURST_INC_ADDR);

    // enable BMP size
    //status &= dvProAV_AccessWrite(eOsdImageEn, true);   // 特殊OSD size從圖庫讀取需讀取原圖size, 否則OSD畫面會錯
    cData[32] = (eOsdImageEn >> 8) & 0xff;
    cData[33] = 0x01;

    // set write addr
    cData[34] = (eOsdMemWrAdr >> 8) & 0xff;
    cData[35] = (ulWad & 0xff);
    cData[36] = ((eOsdMemWrAdr >> 8) + 1) & 0xff;
    cData[37] = (ulWad >> 8) & 0xff;
    cData[38] = ((eOsdMemWrAdr >> 8) + 2) & 0xff;
    cData[39] = (ulWad >> 16) & 0xff;
    cData[40] = ((eOsdMemWrAdr >> 8) + 3) & 0xff;
    cData[41] = (ulWad >> 24) & 0xff;
    // BLT trigger
    cData[42] = (eOsdBmpTrig >> 8) & 0xff;
    cData[43] = (eOsdBmpTrig) & 0xff;
    //status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
    status &= dvProAV_AccessBurstWrite(eOsdQuickWr_E4k, &cData[0], sizeof (cData), BURST_FIX_ADDR);
#endif
    //printf("(%s, %d)Time %d\n",__FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_Bitmap Time Out 3\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */
    ASSERT(status == rcSUCCESS);

    status &= dvProAV_AccessWrite(eOsdImageEn, false);  // BitBLT完成須關閉, 否則會影響其他OSD功能

    return status;
}
#endif

/**
 * @brief 複製當前顯示OSD畫面到另一個Memory Page
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @todo : 判斷PanelTiming
 */
#if PROAV_DISABLE_QUICK_OSD
int OSD_CurrentCopy(void) //A70LV_Larry_0067
{
    int status = rcSUCCESS;
    uint8  cData[16];

    UINT32 source_adrs = (POSD_SAD != POSD_SAD0) ? POSD_SAD0 : POSD_SAD1;
    UINT32 dest_adrs = POSD_SAD;
    UINT16 uibbHor = m_panelWidth;
    UINT16 uibbVer = m_panelHeight;

#ifdef PROAV_OSD_ENABLE
#if 0   // signal mode
    // set size
    status &= dvProAV_AccessWrite(eOsdBmpHeight, uibbVer);    // set Bmp Height
    status &= dvProAV_AccessWrite(eOsdBmpWidth, uibbHor);     // set Bmp Width
    // set read position
    status &= dvProAV_AccessWrite(eOsdBmpVStart, 0);          // set Bmp read V start
    status &= dvProAV_AccessWrite(eOsdBmpHStart, 0);          // set Bmp read H start
    // set Read Adr
    status &= dvProAV_AccessWrite(eOsdBltAdr, source_adrs);   // set Bmp read DRAM address
    // set write position
    status &= dvProAV_AccessWrite(eOsdBltVStart, 0);          // set Bmp write V start
    status &= dvProAV_AccessWrite(eOsdBltHStart, 0);          // set Bmp write H start
    // set Write Adr
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, dest_adrs);   // set Bmp write DRAM address
    // trigger
    status &= dvProAV_AccessWrite(eOsdBmpTrig, true);         // OSD control register ==> BitBLT
#else   // burst mode
    // set size
    cData[0] = uibbVer & 0xff;          // set Bmp Height
    cData[1] = (uibbVer >> 8) & 0xff;
    cData[2] = uibbHor & 0xff;          // set Bmp Width
    cData[3] = (uibbHor >> 8) & 0xff;
    // set read position
    cData[4] = 0;                       // set Bmp read V start
    cData[5] = 0;
    cData[6] = 0;                       // set Bmp read H start
    cData[7] = 0;
    // set Read Adr
    cData[8] = source_adrs & 0xff;      // set Bmp read DRAM address
    cData[9] = (source_adrs >> 8) & 0xff;
    cData[10] = (source_adrs >> 16) & 0xff;
    cData[11] = (source_adrs >> 24) & 0xff;
    // set write position
    cData[12] = 0;                      // set Bmp write V start
    cData[13] = 0;
    cData[14] = 0;                      // set Bmp write H start
    cData[15] = 0;
    status &= dvProAV_AccessBurstWrite(eOsdBmpHeight, (uint08 *)cData, 16, BURST_INC_ADDR);

    // set Write Adr
    cData[0] = (dest_adrs & 0xff);
    cData[1] = (dest_adrs >> 8) & 0xff;
    cData[2] = (dest_adrs >> 16) & 0xff;
    cData[3] = (dest_adrs >> 24) & 0xff;
    // BLT trigger
    cData[4] = (eOsdBmpTrig) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
#endif
    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("OSD_CurrentCopy Time Out\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */

    return status;
}
#else
int OSD_CurrentCopy(void) //A70LV_Larry_0067
{
    int status = rcSUCCESS;
    uint8  cData[42];

    UINT32 source_adrs = (POSD_SAD != POSD_SAD0) ? POSD_SAD0 : POSD_SAD1;
    UINT32 dest_adrs = POSD_SAD;
    UINT16 uibbHor = m_panelWidth;
    UINT16 uibbVer = m_panelHeight;

#ifdef PROAV_OSD_ENABLE
#if 0   // signal mode
    // set size
    status &= dvProAV_AccessWrite(eOsdBmpHeight, uibbVer);    // set Bmp Height
    status &= dvProAV_AccessWrite(eOsdBmpWidth, uibbHor);     // set Bmp Width
    // set read position
    status &= dvProAV_AccessWrite(eOsdBmpVStart, 0);          // set Bmp read V start
    status &= dvProAV_AccessWrite(eOsdBmpHStart, 0);          // set Bmp read H start
    // set Read Adr
    status &= dvProAV_AccessWrite(eOsdBltAdr, source_adrs);   // set Bmp read DRAM address
    // set write position
    status &= dvProAV_AccessWrite(eOsdBltVStart, 0);          // set Bmp write V start
    status &= dvProAV_AccessWrite(eOsdBltHStart, 0);          // set Bmp write H start
    // set Write Adr
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, dest_adrs);   // set Bmp write DRAM address
    // trigger
    status &= dvProAV_AccessWrite(eOsdBmpTrig, true);         // OSD control register ==> BitBLT
#else   // burst mode
    // set size
    cData[0] = (eOsdBmpHeight >> 8) & 0xff;
    cData[1] = uibbVer & 0xff;          // set Bmp Height
    cData[2] = ((eOsdBmpHeight >> 8) + 1) & 0xff;
    cData[3] = (uibbVer >> 8) & 0xff;
    cData[4] = (eOsdBmpWidth >> 8) & 0xff;
    cData[5] = uibbHor & 0xff;          // set Bmp Width
    cData[6] = ((eOsdBmpWidth >> 8) + 1) & 0xff;
    cData[7] = (uibbHor >> 8) & 0xff;
    // set read position
    cData[8] = (eOsdBmpVStart >> 8) & 0xff;
    cData[9] = 0;                       // set Bmp read V start
    cData[10] = ((eOsdBmpVStart >> 8) + 1) & 0xff;
    cData[11] = 0;
    cData[12] = (eOsdBmpHStart >> 8) & 0xff;
    cData[13] = 0;                       // set Bmp read H start
    cData[14] = ((eOsdBmpHStart >> 8) + 1) & 0xff;
    cData[15] = 0;
    // set Read Adr
    cData[16] = (eOsdBltAdr >> 8) & 0xff;
    cData[17] = source_adrs & 0xff;      // set Bmp read DRAM address
    cData[18] = ((eOsdBltAdr >> 8) + 1) & 0xff;
    cData[19] = (source_adrs >> 8) & 0xff;
    cData[20] = ((eOsdBltAdr >> 8)  + 2) & 0xff;
    cData[21] = (source_adrs >> 16) & 0xff;
    cData[22] = ((eOsdBltAdr >> 8)  + 3) & 0xff;
    cData[23] = (source_adrs >> 24) & 0xff;
    // set write position
    cData[24] = (eOsdBltVStart >> 8) & 0xff;
    cData[25] = 0;                       // set Bmp write V start
    cData[26] = ((eOsdBltVStart >> 8) + 1) & 0xff;
    cData[27] = 0;
    cData[28] = (eOsdBltHStart >> 8) & 0xff;
    cData[29] = 0;                       // set Bmp write H start
    cData[30] = ((eOsdBltHStart >> 8) + 1) & 0xff;
    cData[31] = 0;
    //status &= dvProAV_AccessBurstWrite(eOsdBmpHeight, (uint08 *)cData, 16, BURST_INC_ADDR);

    // set Write Adr
    cData[32] = (eOsdMemWrAdr >> 8) & 0xff;
    cData[33] = (dest_adrs & 0xff);
    cData[34] = ((eOsdMemWrAdr >> 8) + 1) & 0xff;
    cData[35] = (dest_adrs >> 8) & 0xff;
    cData[36] = ((eOsdMemWrAdr >> 8) + 2) & 0xff;
    cData[37] = (dest_adrs >> 16) & 0xff;
    cData[38] = ((eOsdMemWrAdr >> 8) + 3) & 0xff;
    cData[39] = (dest_adrs >> 24) & 0xff;
    // BLT trigger
    cData[40] = (eOsdBmpTrig >> 8) & 0xff;
    cData[41] = (eOsdBmpTrig) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdQuickWr_E4k, cData, sizeof (cData), BURST_FIX_ADDR);
#endif
    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("OSD_CurrentCopy Time Out\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */

    return status;
}
#endif

/**
 * @brief Inhibit Color Set
 * @param [in] cEnable : OSD write-inhibit color enable (0 or 1)
 * @param [in] iInhibit_Color : OSD write-inhibit color (color pallet number)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Inhibit_Color_Set(INT8 cEnable, UINT16 iInhibit_Color)
{
    int status = rcSUCCESS;

#ifdef PROAV_OSD_ENABLE
    status &= dvProAV_AccessWrite(eOsdInhiColorEn, (uint16)cEnable);
    if(m_iInhitbit_Color != iInhibit_Color)
    {
        m_iInhitbit_Color = iInhibit_Color;
        status &= dvProAV_AccessWrite(eOsdInhiColor, (uint32)m_iInhitbit_Color);
    }
#endif /* PROAV_OSD_ENABLE */

    return status;
}

/**
 * @brief OSD software reset
 * @param [in] bEnable :
 *              false : software reset disable
 *              true : software reset enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_SW_Reset(bool bEnable)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eOsdSWRst, bEnable);
    MS_SLEEP(1);
    return status;
}

/**
 * @brief 不顯示OSD畫面
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_Enable(bool bEnable)
{
    return dvProAV_AccessWrite(eOsdEnable, bEnable);     // osd Enable off
}

bool dvProAV_OSD_EnableGet(void)
{
    UINT32 ulEnale = 0;
    dvProAV_AccessRead(eOsdEnable, &ulEnale);
    return (bool)ulEnale;
}

/**
 * @brief OSD On
 * @param [in] sDes_position : 設定OSD起始點
 * @param [in] sDes_Size : 設定OSD Size
 * @param [in] cTrap_Enable : 設定OSD畫面Enable / Disable
 * @param [in] sTrap_Color : 設定透明色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#if PROAV_DISABLE_QUICK_OSD
int dvProAV_OSD_On(START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color)
{
    int status = rcSUCCESS;
    uint8  cData[9];

#ifdef PROAV_OSD_ENABLE
#if 0   // signal mode
    // 設定OSD Size
    status &= dvProAV_AccessWrite(eOsdVidWidth, (uint32)sDes_Size.iWidth);
    status &= dvProAV_AccessWrite(eOsdVidHeight, (uint32)sDes_Size.iHeight);
    // 設定起始位置
    status &= dvProAV_AccessWrite(eOsdHorzShift, (uint16)sDes_position.iX); // OSD ACT horizontal direction start point 1 CH1
    status &= dvProAV_AccessWrite(eOsdVertShift, (uint16)sDes_position.iY); // OSD ACT vertical direction start point 1 CH1
    // 設定透明色
    status &= dvProAV_AccessWrite(eOsdTranColor0, sTrap_Color.ucT_Color1 & 0xff);

    // OSD Display to panel
    status &= dvProAV_AccessWrite(eOsdEnable, (uint16)cTrap_Enable);// 顯示OSD畫面

    // 設定OSD讀取位置
    status &= dvProAV_AccessWrite(eOsdMemRdAdr, (uint32)POSD_SAD);          //OSD start address 1 CH1
    // display trigger
    status &= dvProAV_AccessWrite(eOsdDispTrig, true);              // OSD display

#else   // burst mode
    // 設定OSD Size
    cData[0] = sDes_Size.iHeight & 0xff;
    cData[1] = (sDes_Size.iHeight >> 8) & 0xff;
    cData[2] = sDes_Size.iWidth & 0xff;
    cData[3] = (sDes_Size.iWidth >> 8) & 0xff;
    // 設定起始位置
    cData[4] = sDes_position.iX & 0xff;
    cData[5] = (sDes_position.iX >> 8) & 0xff;
    cData[6] = sDes_position.iY & 0xff;
    cData[7] = (sDes_position.iY >> 8) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdVidHeight, (uint08 *)cData, 8, BURST_INC_ADDR);
    // 設定透明色
    cData[0] = sTrap_Color.ucT_Color1 & 0xff;
    cData[1] = 0;
    cData[2] = sTrap_Color.ucT_Color2 & 0xff;
    cData[3] = 0;
    cData[4] = sTrap_Color.ucT_Color3 & 0xff;
    cData[5] = 0;
    cData[6] = sTrap_Color.ucT_Color4 & 0xff;
    cData[7] = 0;
    status &= dvProAV_AccessBurstWrite(eOsdTranColor0, (uint08 *)cData, 8, BURST_INC_ADDR);

    // OSD Display to panel
    status &= dvProAV_AccessWrite(eOsdEnable, (uint16)cTrap_Enable);// 顯示OSD畫面

    // 設定OSD讀取位置
    cData[0] = (POSD_SAD & 0xff);
    cData[1] = (POSD_SAD >> 8) & 0xff;
    cData[2] = (POSD_SAD >> 16) & 0xff;
    cData[3] = (POSD_SAD >> 24) & 0xff;
    // 設定OSD寫入位置
    cData[4] = 0;
    cData[5] = 0;
    cData[6] = 0;
    cData[7] = 0;
    // display trigger
    cData[8] = (eOsdDispTrig) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdMemRdAdr, (uint08 *)cData, 9, BURST_INC_ADDR);
#endif
    OSD_ResetTimeInState();

    while(dvProAV_OSD_Vs_WaitStateGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("OSD_On Time Out\n");
           break;
        }
#endif
    }
#endif
    ASSERT(status == rcSUCCESS);

    OSD_Swap_Memplane();  //Swap memory plane automatically after OSD drawing every time

    return status;
}
#else
int dvProAV_OSD_On(START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color)
{
    int status = rcSUCCESS;
    uint8  cData[40];

#ifdef PROAV_OSD_ENABLE
#if 0   // signal mode
    // 設定OSD Size
    status &= dvProAV_AccessWrite(eOsdVidWidth, (uint32)sDes_Size.iWidth);
    status &= dvProAV_AccessWrite(eOsdVidHeight, (uint32)sDes_Size.iHeight);
    // 設定起始位置
    status &= dvProAV_AccessWrite(eOsdHorzShift, (uint16)sDes_position.iX); // OSD ACT horizontal direction start point 1 CH1
    status &= dvProAV_AccessWrite(eOsdVertShift, (uint16)sDes_position.iY); // OSD ACT vertical direction start point 1 CH1
    // 設定透明色
    status &= dvProAV_AccessWrite(eOsdTranColor0, sTrap_Color.ucT_Color1 & 0xff);

    // OSD Display to panel
    status &= dvProAV_AccessWrite(eOsdEnable, (uint16)cTrap_Enable);// 顯示OSD畫面

    // 設定OSD讀取位置
    status &= dvProAV_AccessWrite(eOsdMemRdAdr, (uint32)POSD_SAD);          //OSD start address 1 CH1
    // display trigger
    status &= dvProAV_AccessWrite(eOsdDispTrig, true);              // OSD display

#else   // burst mode
    // 設定OSD Size
    cData[0] = (eOsdVidWidth >> 8) & 0xff;
    cData[1] = sDes_Size.iWidth & 0xff;
    cData[2] = ((eOsdVidWidth >> 8) + 1) & 0xff;
    cData[3] = (sDes_Size.iWidth >> 8) & 0xff;
    cData[4] = (eOsdVidHeight >> 8)& 0xff;
    cData[5] = sDes_Size.iHeight & 0xff;
    cData[6] = ((eOsdVidHeight >> 8) + 1) & 0xff;
    cData[7] = (sDes_Size.iHeight >> 8) & 0xff;
    // 設定起始位置
    cData[8] = (eOsdHorzShift >> 8) & 0xff;
    cData[9] = sDes_position.iX & 0xff;
    cData[10] = ((eOsdHorzShift >> 8) + 1) & 0xff;
    cData[11] = (sDes_position.iX >> 8) & 0xff;
    cData[12] = (eOsdVertShift >> 8) & 0xff;
    cData[13] = sDes_position.iY & 0xff;
    cData[14] = ((eOsdVertShift >> 8) + 1) & 0xff;
    cData[15] = (sDes_position.iY >> 8) & 0xff;
    //status &= dvProAV_AccessBurstWrite(eOsdVidHeight, (uint08 *)cData, 8, BURST_INC_ADDR);
    // 設定透明色
    cData[16] = (eOsdTranColor0 >> 8) & 0xff;
    cData[17] = sTrap_Color.ucT_Color1 & 0xff;
    cData[18] = ((eOsdTranColor0 >> 8) + 1) & 0xff;
    cData[19] = (sTrap_Color.ucT_Color1 >> 8) & 0xff;
    //status &= dvProAV_AccessBurstWrite(eOsdTranColor0, (uint08 *)cData, 8, BURST_INC_ADDR);
    cData[20] = (eOsdEnable >> 8)& 0xff;
    cData[21] = cTrap_Enable & 0xff;
    // OSD Display to panel
    //status &= dvProAV_AccessWrite(eOsdEnable, (uint16)cTrap_Enable);// 顯示OSD畫面

    // 設定OSD讀取位置
    cData[22] = (eOsdMemRdAdr >> 8) & 0xff;
    cData[23] = (POSD_SAD & 0xff);
    cData[24] = ((eOsdMemRdAdr >> 8) + 1) & 0xff;
    cData[25] = (POSD_SAD >> 8) & 0xff;
    cData[26] = ((eOsdMemRdAdr >> 8) + 2) & 0xff;
    cData[27] = (POSD_SAD >> 16) & 0xff;
    cData[28] = ((eOsdMemRdAdr >> 8) + 3) & 0xff;
    cData[29] = (POSD_SAD >> 24) & 0xff;

    cData[30] = (eOsdMemWrAdr >> 8) & 0xff;
    cData[31] = 0;
    cData[32] = ((eOsdMemWrAdr >> 8) + 1) & 0xff;
    cData[33] = 0;
    cData[34] = ((eOsdMemWrAdr >> 8) + 2) & 0xff;
    cData[35] = 0;
    cData[36] = ((eOsdMemWrAdr >> 8) + 3) & 0xff;
    cData[37] = 0;

    // display trigger
    cData[38] = (eOsdDispTrig >> 8)& 0xff;
    cData[39] = (eOsdDispTrig) & 0xff;
    status &= dvProAV_AccessBurstWrite(eOsdQuickWr_E4k, cData, sizeof (cData), BURST_FIX_ADDR);
#endif
    OSD_ResetTimeInState();

    while(dvProAV_OSD_Vs_WaitStateGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("OSD_On Time Out\n");
           break;
        }
#endif
    }
#endif
    ASSERT(status == rcSUCCESS);

    OSD_Swap_Memplane();  //Swap memory plane automatically after OSD drawing every time

    return status;
}
#endif

/**
 * @brief OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color)
{
    int status = rcSUCCESS;

    m_OSDTransparencyEn = false;

    // 設定Memory位置
    POSD_SAD = DRAM_OSD_MEM1_ADDR;          // OSDSAD(current) point to current OSD memory plane
    POSD_SAD0 = DRAM_OSD_MEM1_ADDR;         // OSDSAD0 OSD memory plane 0
    POSD_SAD1 = DRAM_OSD_MEM2_ADDR;         // OSDSAD1 OSD memory plane 1

#ifndef QT_CPP    //Flash & DMA Memory位置
    //POSD_SFL_PSAD = BITMAP_RAW_DATA_ADDR;
    //POSD_SFL_FSAD = TEXT_RAW_DATA_ADDR;
#endif
#ifdef PROAV_OSD_ENABLE
    // OSD soft reset
    status &= dvProAV_OSD_SW_Reset(true);
    status &= dvProAV_OSD_SW_Reset(false);
    // 設定調色盤
    status &= OSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);
    // 清除OSD畫面
    status &= dvProAV_AccessWrite(REG_OsdFunc, 0);
    status &= dvProAV_AccessWrite(eOsdEnable, false);
    // 設定讀取OSD資料的DRAM位置
    status &= dvProAV_AccessWrite(eOsdMemRdAdr, POSD_SAD);
    // 設定OSD Line Buffer Size display panel size
    status &= dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
    // 設定OSD Size
    status &= dvProAV_OSD_WriteSizeSet(m_panelWidth, m_panelHeight);
    status &= dvProAV_OSD_ReadSizeSet(m_panelWidth, m_panelHeight);
    // 設定OSD 圖庫 Size
    status &= dvProAV_AccessWrite(eOsdImageWidth, OSD_BITMAP_RAW_WIDTH);    // 設定OSD原圖Hsize
    status &= dvProAV_AccessWrite(eOsdImageHeight, OSD_BITMAP_RAW_HEIGHT);  // 設定OSD原圖Vsize

#ifndef NO_INTERFACE        // 將字庫及BMP data放至DRAM
    DBMSG("OSD DMA Start\n");
    OSD_DMA_SerialFlash_Set(FLASH_OSDTEXT_ADDR, DRAM_OSD_TEXT_ADDR * PROAV_DRAM_LINE_BYTES, OSD_FNT_SIZE);// OSD Font  //modify dma size to fit new font dat

    DBMSG("DMA Bitmap Front\n");
    OSD_DMA_SerialFlash_Set(FLASH_OSDBMP0_ADDR, DRAM_OSD_BMP_ADDR * PROAV_DRAM_LINE_BYTES, OSD_BMP_SIZE);// OSD Part  //modify dma size to fit new bitmap data

    DBMSG("DMA Bitmap Done\n");
#endif /* PROAV_OSD_ENABLE */

    // 設定讀取字庫base address
    status &= dvProAV_AccessWrite(eOsdCharBufAdr, DRAM_OSD_TEXT_ADDR);
    // 設定禁止色
    m_iInhitbit_Color = (UINT16)ucInhibit_Color;
    status &= dvProAV_Inhibit_Color_Set(true, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eOsdInhiColorEn, true);
    status &= dvProAV_AccessWrite(eOsdInhiColor, m_iInhitbit_Color);
    status &= dvProAV_AccessWrite(eVopOsdBldLvl, 0);

    // 切換OSD DRAM Buffer
    status &= OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    status &= OSD_Swap_Memplane();  //set OSD memory plane to first and reset that
#endif /* PROAV_OSD_ENABLE */

    return status;
}

/**
 * @brief ReLoad OSD Front & Bitmap Data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_ReLoad(void)
{
    int status = rcSUCCESS;
    DBMSG("OSD DMA Start\n");
    status &= OSD_DMA_SerialFlash_Set(FLASH_OSDTEXT_ADDR, DRAM_OSD_TEXT_ADDR * PROAV_DRAM_LINE_BYTES, OSD_FNT_SIZE);// OSD Font  //modify dma size to fit new font dat

    DBMSG("DMA Bitmap Front\n");
    status &= OSD_DMA_SerialFlash_Set(FLASH_OSDBMP0_ADDR, DRAM_OSD_BMP_ADDR * PROAV_DRAM_LINE_BYTES, OSD_BMP_SIZE);// OSD Part  //modify dma size to fit new bitmap data

    DBMSG("DMA Bitmap Done\n");

    return status;
}

/**
 * @brief Set OSD Menu Transparency
 * @param [in] ucValue : 傳入透明度比例
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_MenuTransparencySet(UINT8 ucValue)     //A70LV_Doulas_0122
{
    int status = rcSUCCESS;
    UINT16 ucVal;
    ucVal = ((ucValue * 165) / 100);    // 0-100 to 0-165
    m_ucOSDTransparency = (ucVal & 0xff);
#ifdef PROAV_OSD_ENABLE
    if(m_OSDTransparencyEn)
    {
        status &= dvProAV_AccessWrite(eVopOsdBldLvl, m_ucOSDTransparency);
    }
    else
    {
        return 1;
    }
#endif /* PROAV_OSD_ENABLE */
    return status;
}

/**
 * @brief Set OSD Menu Transparency Enable / Disable
 * @param [in] ucEnalbe : Set Menu Transparency
 *              -false : disable
 *              -true : enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe)
{
    int status = rcSUCCESS;
    UINT8 ucVal;
    m_OSDTransparencyEn = ucEnalbe;
	//for H60K HICC2
    //ucVal =(ucEnalbe)? m_ucOSDTransparency : false;
    //status &= dvProAV_AccessWrite(eVopOsdBldLvl, ucVal);
    return status;
}

/**
 * @brief Set OSD Palette
 * @param [in] iAmount :
 * @param [in] pucData :
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int OSD_Palette_Set(INT16 iAmount, UINT8 *pucData)
{
    int status = rcSUCCESS;

#ifdef PROAV_OSD_ENABLE
    if (iAmount != PROAV_PALETTE_COLOR)
    {
       return rcSUCCESS;
    }

    // Enable調色盤
    status &= dvProAV_AccessWrite(eOsdPaleteEn, true);
    DBMSG("OSD_Palette_Set\n");

    // 寫入調色盤Data
    #ifndef QT_CPP    // Single mode
        for(INT16 iCount=0; iCount<(iAmount*3); iCount++)
        {
            status &= dvProAV_AccessWrite(eOsdPalete, *(pucData+iCount));

            //DBMSG_ALWAYS("[%02x]\n", *(pucData+iCount));
        }
    #else   // Burst mode
        status &= dvProAV_AccessBurstWrite(eOsdPalete , pucData, (uint16)(iAmount*3), BURST_FIX_ADDR);
    #endif
#endif /* PROAV_OSD_ENABLE */
    return status;
}

#ifndef QT_CPP
INT8 dvProAV_OSD_PanelSet(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    OSDePanelTimingId = ePanelTimingId;
    return rcSUCCESS;
}

INT8 dvProAV_OSD_PanelChange(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0226 modify//A70LV_Doulas_0105
{
    OSDePanelTimingId = ePanelTimingId;

    switch(OSDePanelTimingId)
    {
        case ePANEL_ID_1080P_60HZ:
        case ePANEL_ID_1080P_120HZ:
        case ePANEL_ID_1080P_240HZ:
            {
                m_panelWidth = 1920;
                m_panelHeight = 1080;
                dvProAV_OSD_DoubleSizeSet(0, 0, 0);
                dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
            }
            break;

        case ePANEL_ID_WUXGA_60HZ:
        case ePANEL_ID_WUXGA_120HZ:
        case ePANEL_ID_WUXGA_240HZ:
            {
                m_panelWidth = 1920;
                m_panelHeight = 1200;
                dvProAV_OSD_DoubleSizeSet(0, 0, 0);
                dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
            }
            break;

        case ePANEL_ID_2688x1472_120HZ:
            {
                m_panelWidth = 2688 / 2;
                m_panelHeight = 1472 / 2;

                dvProAV_OSD_DoubleSizeSet(1, 1, 1);

                dvProAV_AccessWrite(eOsdLineBufCnt, m_panelWidth / 32);
            }
            break;

        case ePANEL_ID_2560x1440_60HZ:
            {
                UINT16 uiOSDLineCnt = 0;

                m_panelWidth = 2560;
                m_panelHeight = 1440;

                dvProAV_OSD_DoubleSizeSet(0, 0, 0);

                uiOSDLineCnt = m_panelWidth/32;

                dvProAV_AccessWrite(eOsdLineBufCnt, uiOSDLineCnt);
            }
            break;

        case ePANEL_ID_3840x2160_50HZ:
        case ePANEL_ID_3840x2160_60HZ:
        case ePANEL_ID_3840x2160_30HZ:
            {
                m_panelWidth = 1920;
                m_panelHeight = 1080;
                dvProAV_OSD_DoubleSizeSet(1, 1, 1);
                dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
            }
            break;

        case ePANEL_ID_3840x2400_50HZ:
        case ePANEL_ID_3840x2400_60HZ:
        case ePANEL_ID_3840x2400_30HZ:
            {
                m_panelWidth = 1920;
                m_panelHeight = 1200;
                dvProAV_OSD_DoubleSizeSet(1, 1, 1);
                dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
            }
            break;

        case ePANEL_ID_1920x2400_60HZ:
            {
                m_panelWidth = 1920;
                m_panelHeight = 2400;
                dvProAV_OSD_DoubleSizeSet(1, 0, 1);
                dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
            }
            break;


        default:
#if 0
            if(dvPro_PIP_PBP_Enable() == TRUE)
            {
                PS_PANEL_VST = OSD_V_START_1080P_60 - 2;//41;
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_1080P_60;//41;
            }
#endif /* 0 */
            m_panelWidth = 1920;
            m_panelHeight = 1080;
            dvProAV_OSD_DoubleSizeSet(0, 0, 0);
            dvProAV_AccessWrite(eOsdLineBufCnt, 0x3C);
            break;
    }

    if(ePanelTimingId == PANEL_2D_OUTPUT)
    {
        dvProAV_AccessWrite(eLogoLogoBasAdr, (DRAM_LOGO_DISPLAY1_2D_ADDR / 2));
    }
    else if(ePanelTimingId == PANEL_3D_OUTPUT || ePanelTimingId == PANEL_2D_HIGHSPEED)
    {
        dvProAV_AccessWrite(eLogoLogoBasAdr, (DRAM_LOGO_DISPLAY1_3D_ADDR / 2));
    }

#ifdef FPGA_ENTRY_4K
    // 設定OSD Read Size
    dvProAV_AccessWrite(eOsdVidRdWidth_E4k, m_panelWidth);
    dvProAV_AccessWrite(eOsdVidRdHeight_E4k, m_panelHeight);
#endif /* FPGA_ENTRY_4K */
    // 設定OSD Write Size
    dvProAV_OSD_WriteSizeSet(m_panelWidth, m_panelHeight);

    return rcSUCCESS;
}
#endif

int dvProAV_OSD_FillColor(UINT16 iColor_Index)
{
    int status = rcSUCCESS;
    uint8 cData[20];

    if((m_panelHeight == 0) || (m_panelWidth == 0))
    {
        return rcERROR;
    }

#ifdef PROAV_OSD_ENABLE
    //status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);                    // 設定寫入DRAM address
    //eOsdBmpFillClr
    cData[0] = iColor_Index & 0xff;                                             // 設定矩形顏色
    cData[1] = (iColor_Index >> 8) & 0xff;
    //eOsdBmpHeight
    cData[2] = m_panelHeight & 0xff;                                            // 設定矩形高度
    cData[3] = (m_panelHeight >> 8) & 0xff;
    //eOsdBmpWidth
    cData[4] = m_panelWidth & 0xff;                                             // 設定矩形寬度
    cData[5] = (m_panelWidth >> 8) & 0xff;
    //eOsdBmpVStart
    cData[6] = 0;                                                               // 設定矩形位置Y
    cData[7] = 0;
    //eOsdBltHStart
    cData[8] = 0;                                                               // 設定矩形位置X
    cData[9] = 0;

    status &= dvProAV_AccessBurstWrite(eOsdBmpFillClr , (uint08 *)cData, 10, BURST_INC_ADDR); // Fixed Address if last parameter is false

    cData[0] = (POSD_SAD & 0xff);
    cData[1] = (POSD_SAD >> 8) & 0xff;
    cData[2] = (POSD_SAD >> 16) & 0xff;
    cData[3] = (POSD_SAD >> 24) & 0xff;
    cData[4] = (eOsdRectTrig) & 0xff;

    status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);
    //status &= dvProAV_AccessWrite(eOsdRectTrig, true);                                // 矩形資料寫入DRAM
    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_Rectangle Time Out 7\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */

    ASSERT(status == rcSUCCESS);

    return status;
}

/**
 * @brief OSD Double Size Set
 * @param [in] bDoubl : false = disable, true = enable
 */
void dvProAV_OSD_DoubleSizeSet(bool bDouble, bool bDoubleX, bool bDoubleY)
{
    dvProAV_AccessWrite(eOsd4KEn, bDouble);    // OSD 2K to 4K Enable / Disable

    if(bDouble == TRUE)
    {
        dvProAV_AccessWrite(eOsd4KEn_X, bDoubleX);
        dvProAV_AccessWrite(eOsd4KEn_Y, bDoubleY);
    }
}

/**
 * @brief OSD Write Size Set
 * @param [in] u16SizeH : OSD Write size H
 * @param [in] u16SizeV : OSD Write size V
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_WriteSizeSet(uint16 u16SizeH, uint16 u16SizeV)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eOsdVidWidth, u16SizeH);
    status &= dvProAV_AccessWrite(eOsdVidHeight, u16SizeV);
    return status;
}

/**
 * @brief OSD Read Size Set
 * @param [in] u16SizeH : OSD read size H
 * @param [in] u16SizeV : OSD read size V
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_OSD_ReadSizeSet(uint16 u16SizeH, uint16 u16SizeV)
{
    int status = rcSUCCESS;
    status &= dvProAV_AccessWrite(eOsdVidRdWidth_E4k, u16SizeH);
    status &= dvProAV_AccessWrite(eOsdVidRdHeight_E4k, u16SizeV);
    return status;
}

//-------------------------------------------------------------------------------------------------
// Logo Capture
//-------------------------------------------------------------------------------------------------
void dvProAV_OSD_LogoCaptureLoad(void)
{
    //logo capture
    OSD_DMA_SerialFlash_Set(FLASH_LOGO0_2D_ADDR, DRAM_LOGO_DISPLAY0_2D_ADDR * PROAV_DRAM_LINE_BYTES, LOGO_CAPTURE_2D_SIZE);
    OSD_DMA_SerialFlash_Set(FLASH_LOGO0_3D_ADDR, DRAM_LOGO_DISPLAY0_3D_ADDR * PROAV_DRAM_LINE_BYTES, LOGO_CAPTURE_3D_SIZE);
}

//-------------------------------------------------------------------------------------------------
// Logo default Replacement
//-------------------------------------------------------------------------------------------------
void dvProAV_OSD_LogoDefaultReplacement(void)
{
    //logo default replacement
    OSD_DMA_SerialFlash_Set(FLASH_LOGO1_2D_ADDR, DRAM_LOGO_DISPLAY1_2D_ADDR * PROAV_DRAM_LINE_BYTES, LOGO_CAPTURE_2D_SIZE);
    OSD_DMA_SerialFlash_Set(FLASH_LOGO1_3D_ADDR, DRAM_LOGO_DISPLAY1_3D_ADDR * PROAV_DRAM_LINE_BYTES, LOGO_CAPTURE_3D_SIZE);
}

//-------------------------------------------------------------------------------------------------
// Logo user Replacement
//-------------------------------------------------------------------------------------------------
void dvProAV_OSD_LogoUserReplacement(void)
{
    //logo capture
    OSD_DMA_SerialFlash_Set(FLASH_LOGO2_2D_ADDR, DRAM_LOGO_DISPLAY2_2D_ADDR * PROAV_DRAM_LINE_BYTES, LOGO_CAPTURE_2D_SIZE);
    OSD_DMA_SerialFlash_Set(FLASH_LOGO2_2D_ADDR, DRAM_LOGO_DISPLAY2_3D_ADDR * PROAV_DRAM_LINE_BYTES, LOGO_CAPTURE_3D_SIZE);
}

#ifdef FPGA_ENTRY_4K
//-------------------------------------------------------------------------------------------------
// Warp OSD For Entry 4K
//-------------------------------------------------------------------------------------------------
/**
 * @brief Get WarpOSD busy state
 * @return BusyState : 0=unbusy, 1=busy
 */
static bool dvProAV_WarpOSD_BusyStateGet_E4K(void)
{
    return dvProAV_OSD_BusyGet();
}

/**
 * @brief Inhibit Color Set
 * @param [in] cEnable : Warp OSD write-inhibit color enable (0 or 1)
 * @param [in] iInhibit_Color : Warp OSD write-inhibit color (color pallet number)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
static INT8 dvProAV_WarpOSD_Inhibit_Color_Set_E4K(INT8 cEnable, uint16 iInhibit_Color)
{
    int status = rcSUCCESS;

    m_iInhitbit_Color = iInhibit_Color;

    //status &= dvProAV_Inhibit_Color_Set(false, m_iInhitbit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdInhiColor, m_iInhitbit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdInhiColorEn, (uint16)cEnable);

    return status;
}

/**
 * @brief Page Clear
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_PageClear_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if(m_b4k3dEn)
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(m_b4k3dEn);

    status &= dvProAV_WarpOSD_Inhibit_Color_Set_E4K(false, m_iInhitbit_Color);  // 關閉禁止色
    #if 0   // 矩形畫透明色
        START_POINT sStart;
        RECT_SIZE sSize;

        sStart.iX = 0;
        sStart.iY = 0;
        sSize.iWidth = (int16)WPOSD_H_MAX;
        sSize.iHeight = (int16)WPOSD_V_MAX;

        status &= dvProAV_Paint_Rectangle(sStart, sSize, m_iInhitbit_Color);
    #else   // 透明色功能
        uint32 uiTranNum = (((WPOSD_H_MAX * WPOSD_V_MAX) + (32 * PROAV_DRAM_LINE_BYTES) - 1) / (32 * PROAV_DRAM_LINE_BYTES));     // (Hsize*Vsize)/(一次寫入32個DRAM位置*DRAM位置Byte數), 無條件進位

        if(ePage == eWPOSDPAGE_0)
        {
            status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);                      // 設定寫入DRAM address
        }
        else
        {
            status &= dvProAV_AccessWrite(eOsdMemWrAdr, PWPOSD_SAD);                    // 設定寫入DRAM address
        }

        status &= dvProAV_AccessWrite(eOsdTranColor0, (uint16)m_iInhitbit_Color);       // 設定透明色顏色
        status &= dvProAV_AccessWrite(eOsdBmpFillClr, (uint16)m_iInhitbit_Color);       // 設定填入顏色
        status &= dvProAV_AccessWrite(eOsdTranNum, uiTranNum);                          // 設定尺寸
        status &= dvProAV_AccessWrite(eOsdTranTrig, true);                              // 開始寫入
    #endif

    while((dvProAV_OSD_Vs_WaitStateGet() != 0) || (dvProAV_OSD_BusyGet() != 0))
    {
        MS_SLEEP(1);
        retry--;
    }

    status &= dvProAV_Inhibit_Color_Set(true, m_iInhitbit_Color);
    status &= dvProAV_WarpOSD_Inhibit_Color_Set_E4K(true, m_iInhitbit_Color);  // 開啟禁止色

    if(m_b4k3dEn)
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(false);

    return status;
}

/**
 * @brief Set Warp OSD Palette
 * @param [in] iAmount : 傳入調色盤長度
 * @param [in] pucData : 傳入調色盤data
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Palette_Set_E4K(INT16 iAmount, UINT8 *pucData)
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
           status &= dvProAV_AccessWrite(eWrpOsdPalette, *(pucData+iCount));
        }
    #else   // Burst mode
        status &= dvProAV_AccessBurstWrite(eWrpOsdPalette , pucData, (uint16)(iAmount * 3), BURST_FIX_ADDR);
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
int dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    if(ePage >= eWPOSDPAGE_Max)
        return rcERROR;

    // 設定寫入位置，設定background address為寫入OSD的位置
    if(ePage == eWPOSDPAGE_0)
    {
        POSD_SAD = (PWPOSD_SAD_BEFORE == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;

        status &= dvProAV_AccessWrite(eOsdMemWrAdr, POSD_SAD);

        LOG_MSG(db_HAL_RESERVED19, "(%s, %d) POSD_SAD = %08x\n", __FUNCTION__,__LINE__, POSD_SAD);
    }
    else
    {
        PWPOSD_SAD = (PWPOSD_SAD_AFTER == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;

        status &= dvProAV_AccessWrite(eOsdMemWrAdr, PWPOSD_SAD);

        LOG_MSG(db_HAL_RESERVED19, "(%s, %d) PWPOSD_SAD = %08x\n", __FUNCTION__,__LINE__, PWPOSD_SAD);
    }

    return status;
}

/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Paint_Rectangle_E4K(eWPOSDPAGE ePage, START_POINT sDes_position, RECT_SIZE sDes_Size, uint16 iColor_Index)
{
    int status = rcSUCCESS;

    if(ePage == eWPOSDPAGE_0)
    {
        status = dvProAV_Paint_Rectangle(sDes_position, sDes_Size, iColor_Index);
    }
    else
    {
        if(m_b4k3dEn)
            status &= dvProAV_WarpOSDSubFrameModEn_E4K(m_b4k3dEn);

        uint8 cData[10];

        if((sDes_Size.iWidth == 0) || (sDes_Size.iHeight == 0))
        {
            return rcERROR;
        }

#ifdef PROAV_OSD_ENABLE

        cData[0] = iColor_Index & 0xff;                                             // 設定矩形顏色
        cData[1] = (iColor_Index >> 8) & 0xff;
        cData[2] = sDes_Size.iHeight & 0xff;                                        // 設定矩形高度
        cData[3] = (sDes_Size.iHeight >> 8) & 0xff;
        cData[4] = sDes_Size.iWidth & 0xff;                                         // 設定矩形寬度
        cData[5] = (sDes_Size.iWidth >> 8) & 0xff;
        cData[6] = sDes_position.iY & 0xFF;                                         // 設定矩形位置Y
        cData[7] = (sDes_position.iY >> 8) & 0xff;
        cData[8] = sDes_position.iX & 0xFF;                                         // 設定矩形位置X
        cData[9] = (sDes_position.iX >> 8) & 0xff;
        status &= dvProAV_AccessBurstWrite(eOsdBmpFillClr , (uint08 *)cData, 10, BURST_INC_ADDR); // Fixed Address if last parameter is false


        cData[0] = (PWPOSD_SAD & 0xff);                                             // 設定寫入DRAM address
        cData[1] = (PWPOSD_SAD >> 8) & 0xff;
        cData[2] = (PWPOSD_SAD >> 16) & 0xff;
        cData[3] = (PWPOSD_SAD >> 24) & 0xff;
        cData[4] = (eOsdRectTrig) & 0xff;                                           // 矩形資料寫入DRAM
        status &= dvProAV_AccessBurstWrite(eOsdMemWrAdr, &cData[0], 5, BURST_INC_ADDR);

        OSD_ResetTimeInState();

        while(dvProAV_OSD_BusyGet() != 0)  // check busy
        {
#ifndef QT_CPP
            if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
            {
               DBMSG_ALWAYS("Paint_Rectangle Time Out 4\n");
               break;
            }
#endif
        }
#endif /* PROAV_OSD_ENABLE */

        ASSERT(status == rcSUCCESS);

        if(m_b4k3dEn)
            status &= dvProAV_WarpOSDSubFrameModEn_E4K(false);
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
int dvProAV_WarpOSD_CurrentCopy_E4K(eWPOSDPAGE ePage, OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;
    uint08 eTimerUser0 = 0;

    if(m_b4k3dEn)
    {
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(m_b4k3dEn);
        status &= dvProAV_WarpOSDPaintBmpCtrl_E4K(m_b4k3dEn);
    }

    // set read BMP size
    status &= dvProAV_AccessWrite(eOsdBmpWidth, sBitmap.sBitmap_Size.iWidth & 0xffff);  //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    status &= dvProAV_AccessWrite(eOsdBmpHeight, sBitmap.sBitmap_Size.iHeight & 0xffff);//BitBLT vertical direction width register

    // set read BMP position //copy當下在畫的DRAM
    status &= dvProAV_AccessWrite(eOsdBltAdr, PWPOSD_SAD);                               // 設定Blt Function Address
    status &= dvProAV_AccessWrite(eOsdBmpHStart, sBitmap.sSrc_Position.iX & 0xffff);
    status &= dvProAV_AccessWrite(eOsdBmpVStart, sBitmap.sSrc_Position.iY & 0xffff);
    // set write BMP position
    //status &= dvProAV_AccessWrite(eOWMemWrAdr, PWPOSD_SAD_BEFORE);                     // 設定寫入DRAM address
    status &= dvProAV_AccessWrite(eOsdBltVStart, sDes_position.iY & 0xffff);
    status &= dvProAV_AccessWrite(eOsdBltHStart, sDes_position.iX & 0xffff);

    status &= dvProAV_AccessWrite(eOsdBmpTrig, true);                //OSD control register  ==> BitBLT

    ResetTime(eTimerUser0);
    while((dvProAV_OSD_BusyGet() != 0) && retry)  // busy
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

    if(m_b4k3dEn)
    {
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(false);
        status &= dvProAV_WarpOSDPaintBmpCtrl_E4K(false);
    }

    return status;
}

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] sBitmap : Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 * @note 目前設定在不能複製2D mode圖至XPR mode page
 */
int dvProAV_WarpOSD_Paint_Bitmap_E4K(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;
    UINT16 iSrc_x = 0, iSrc_y = 0;
    UINT32 ulRad = 0;
    UINT32 ulWad = 0;
    uint8  cData[16];

#ifdef PROAV_OSD_ENABLE
    if(m_b4k3dEn)
    {
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(m_b4k3dEn);
        status &= dvProAV_WarpOSDPaintBmpCtrl_E4K(m_b4k3dEn);
    }
    //DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    iSrc_x = sBitmap.sSrc_Position.iX;
    iSrc_y = sBitmap.sSrc_Position.iY;

#if 0   // siganl mode
    // set size
    status &= dvProAV_AccessWrite(eOsdBmpHeight, sBitmap.sBitmap_Size.iHeight & 0xffff);    // set Bmp Height
    status &= dvProAV_AccessWrite(eOsdBmpWidth, sBitmap.sBitmap_Size.iWidth & 0xffff);      // set Bmp Width
    // set read position
    ulRad = DRAM_OSD_BMP_ADDR + ((uint32)sBitmap.sSrc_Position.iX / PROAV_DRAM_LINE_BYTES) + ((uint32)sBitmap.sSrc_Position.iY * (OSD_BITMAP_RAW_WIDTH / PROAV_DRAM_LINE_BYTES));
    iSrc_x = iSrc_x % PROAV_DRAM_LINE_BYTES;
    iSrc_y = 0;
    status &= dvProAV_AccessWrite(eOsdBmpVStart, iSrc_y & 0xffff);          // set Bmp read V start
    status &= dvProAV_AccessWrite(eOsdBmpHStart, iSrc_x & 0xffff);          // set Bmp read H start
    status &= dvProAV_AccessWrite(eOsdBltAdr, ulRad);                       // set Bmp read DRAM address
    // set write position
    ulWad = POSD_SAD + ((uint16)sDes_position.iX / PROAV_DRAM_LINE_BYTES) + ((uint16)sDes_position.iY * (m_panelWidth / PROAV_DRAM_LINE_BYTES));
    sDes_position.iX = sDes_position.iX % PROAV_DRAM_LINE_BYTES;
    sDes_position.iY = 0;
    status &= dvProAV_AccessWrite(eOsdBltHStart, sDes_position.iX & 0xffff);// set Bmp write V start
    status &= dvProAV_AccessWrite(eOsdBltVStart, sDes_position.iY & 0xffff);// set Bmp write H start
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, ulWad);                     // set Bmp write DRAM address
#else   // burst mode
    cData[0] = sBitmap.sBitmap_Size.iHeight & 0xff;         // set Bmp Height
    cData[1] = (sBitmap.sBitmap_Size.iHeight >> 8) & 0xff;
    cData[2] = sBitmap.sBitmap_Size.iWidth & 0xff;          // set Bmp Width
    cData[3] = (sBitmap.sBitmap_Size.iWidth >> 8) & 0xff;
    // set read position
    ulRad = DRAM_WRPOSD_BMP0_ADDR;
    cData[4] = iSrc_y & 0xff;           // set Bmp read V start
    cData[5] = (iSrc_y >> 8) & 0xff;
    cData[6] = iSrc_x & 0xff;           // set Bmp read H start
    cData[7] = (iSrc_x >> 8) & 0xff;
    cData[8] = ulRad & 0xff;            // set Bmp read DRAM Address
    cData[9] = (ulRad >> 8) & 0xff;
    cData[10] = (ulRad >> 16) & 0xff;
    cData[11] = (ulRad >> 24) & 0xff;
    // set write position
    ulWad = PWPOSD_SAD;
    cData[12] = sDes_position.iY & 0xFF;                // set Bmp write V start
    cData[13] = (sDes_position.iY >> 8) & 0xff;
    cData[14] = sDes_position.iX & 0xFF;                // set Bmp write H start
    cData[15] = (sDes_position.iX >> 8) & 0xff;
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, ulWad); // set Bmp write DRAM address
    status &= dvProAV_AccessBurstWrite(eOsdBmpHeight , (uint08 *)cData, 16, BURST_INC_ADDR);
#endif
    status &= dvProAV_AccessWrite(eOsdImageEn, true);   // 特殊OSD size從圖庫讀取需讀取原圖size, 否則OSD畫面會錯
    status &= dvProAV_AccessWrite(eOsdBmpTrig, true);   // OSD control register  ==> BitBLT

    OSD_ResetTimeInState();

    while(dvProAV_OSD_BusyGet() != 0)  // check busy
    {
        MS_SLEEP(1);
#ifndef QT_CPP
        if(OSD_TimeElapsedInState() >= OSDBUSYTIMEOUT)
        {
           DBMSG_ALWAYS("Paint_Bitmap Time Out 3\n");
           break;
        }
#endif
    }
#endif /* PROAV_OSD_ENABLE */
    ASSERT(status == rcSUCCESS);

    status &= dvProAV_AccessWrite(eOsdImageEn, false);  // BitBLT完成須關閉, 否則會影響其他OSD功能

    if(m_b4k3dEn)
    {
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(false);
        status &= dvProAV_WarpOSDPaintBmpCtrl_E4K(false);
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
int dvProAV_WarpOSD_Paint_SingleCircle_E4K(START_POINT sDes_center, INT16 iRadius, uint16 iColor_Index)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;
    uint08 eTimerUser0 = 0;

    if(iRadius == 0)
    {
        return rcERROR;
    }

    if(m_b4k3dEn)
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(m_b4k3dEn);
    status &= dvProAV_AccessWrite(eOsdCirRadius_E4k,(iRadius & 0xffff));    // 設定圓半徑
    status &= dvProAV_AccessWrite(eOsdCirHCen_E4k, (uint16)sDes_center.iX); // 設定圓心位置X
    status &= dvProAV_AccessWrite(eOsdCirVCen_E4k, (uint16)sDes_center.iY); // 設定圓心位置Y
    status &= dvProAV_AccessWrite(eOsdBmpFillClr, iColor_Index & 0xffff);   // 設定顏色

    status &= dvProAV_AccessWrite(eOsdSCirTri, true);    // 資料寫入DRAM

    ResetTime(eTimerUser0);
    while((dvProAV_OSD_BusyGet() != 0) && retry)  // busy
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

    if(m_b4k3dEn)
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(false);

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
int dvProAV_WarpOSD_Paint_MultiCircle_E4K(START_POINT sDes_center, INT16 iRadius, CENTER_PITCH sPitch, START_POINT sLimit, uint16 iColor_Index)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;
    uint08 eTimerUser0 = 0;

    if(iRadius == 0)
    {
        return rcERROR;
    }

    if(m_b4k3dEn)
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(m_b4k3dEn);

    status &= dvProAV_WarpOSDPaintBmpCtrl_E4K(true);                        // 開啟同頁貼上模式for sub frame mode
    status &= dvProAV_AccessWrite(eOsdCirRadius_E4k,(iRadius & 0xffff));    // 設定圓半徑
    status &= dvProAV_AccessWrite(eOsdCirHCen_E4k, (uint16)sDes_center.iX); // 設定圓心位置X
    status &= dvProAV_AccessWrite(eOsdCirVCen_E4k, (uint16)sDes_center.iY); // 設定圓心位置Y
    status &= dvProAV_AccessWrite(eOsdCirHP_E4k, (uint16)sPitch.idX);       // 設定X軸圓心之間Pitch
    status &= dvProAV_AccessWrite(eOsdCirVP_E4k, (uint16)sPitch.idY);       // 設定Y軸圓心之間Pitch
    status &= dvProAV_AccessWrite(eOsdCirHL_E4k, (uint16)sLimit.iX);        // 設定畫MultiCircle區域邊界X
    status &= dvProAV_AccessWrite(eOsdCirVL_E4k, (uint16)sLimit.iY);        // 設定畫MultiCircle區域邊界Y
    status &= dvProAV_AccessWrite(eOsdBmpFillClr, iColor_Index & 0xffff);   // 設定顏色

    status &= dvProAV_AccessWrite(eOsdMuCirTri, true);  // 資料寫入DRAM

    ResetTime(eTimerUser0);
    while((dvProAV_OSD_BusyGet() != 0) && retry)  // busy
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

    if(m_b4k3dEn)
        status &= dvProAV_WarpOSDSubFrameModEn_E4K(false);
    status &= dvProAV_WarpOSDPaintBmpCtrl_E4K(false);    // 關閉同頁貼上模式

    return status;
}

/**
 * @brief after & before WarpOSD Mix
 * @param [in] bEnable : true=on false=off
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_MixOn_E4K(bool bEnable)
{
    //status &= dvProAV_AccessWrite(eWrpOsdDbChEn, (uint16)bEnable);
    return dvProAV_AccessWrite(eDbdPathMode, (uint16)bEnable);
}

/**
 * @brief WarpOSD Display
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_On_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if(ePage == eWPOSDPAGE_0)
    {
        //status &= dvProAV_AccessWrite(eWrpOsdMemRdAdr, PWPOSD_SAD_BEFORE);

        status &= dvProAV_AccessWrite(eOsdMemRdAdr, POSD_SAD);

        LOG_MSG(db_HAL_RESERVED19, "(%s %d) PWPOSD_SAD_BEFORE = 0x%08x\n", __FUNCTION__, __LINE__, PWPOSD_SAD_BEFORE);
        LOG_MSG(db_HAL_RESERVED19, "(%s %d) POSD_SAD = 0x%08x\n", __FUNCTION__, __LINE__, POSD_SAD);

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
        status &= dvProAV_AccessWrite(eOsdEnable, (uint16)true);        // 顯示OSD畫面
        status &= dvProAV_AccessWrite(eOsdDispTrig, true);              // OSD display

        OSD_ResetTimeInState();
        while(dvProAV_OSD_Vs_WaitStateGet() != 0)  // check busy
        {
            MS_SLEEP(1);
    #ifndef QT_CPP
            if(OSD_TimeElapsedInState() >= (OSDBUSYTIMEOUT / 2))
            {
               DBMSG_ALWAYS("OSD_On Time Out\n");
               break;
            }
    #endif
        }

        //清除下一塊待畫的RAM buffer
        POSD_SAD = (POSD_SAD == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
        dvProAV_WarpOSD_PageClear_E4K(ePage);

        POSD_SAD = (POSD_SAD == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
        PWPOSD_SAD_BEFORE = POSD_SAD;

        LOG_MSG(db_HAL_RESERVED19, "(%s %d) PWPOSD_SAD_BEFORE = 0x%08x\n", __FUNCTION__, __LINE__, PWPOSD_SAD_BEFORE);
        LOG_MSG(db_HAL_RESERVED19, "(%s %d) POSD_SAD = 0x%08x\n", __FUNCTION__, __LINE__, POSD_SAD);

    }
    else
    {

        status &= dvProAV_AccessWrite(eWrpOsdMem2RdAdr, PWPOSD_SAD);

        PWPOSD_SAD_AFTER = PWPOSD_SAD;

        LOG_MSG(db_HAL_RESERVED19, "(%s %d) PWPOSD_SAD_AFTER = 0x%08x\n", __FUNCTION__, __LINE__, PWPOSD_SAD_AFTER);
        LOG_MSG(db_HAL_RESERVED19, "(%s %d) PWPOSD_SAD = 0x%08x\n", __FUNCTION__, __LINE__, PWPOSD_SAD);

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
        status &= dvProAV_AccessWrite(eWrpOsdEnable, (uint16)true);
        status &= dvProAV_AccessWrite(eWrpOsdTrig, (uint16)eWPOSD_DisplayOSDData);

        while((dvProAV_WarpOSD_BusyStateGet_E4K() != 0) && retry)  // busy
        {
            MS_SLEEP(1);
            retry--;
        }

        //清除下一塊待畫的RAM buffer
        PWPOSD_SAD = (PWPOSD_SAD == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
        dvProAV_WarpOSD_PageClear_E4K(ePage);

        PWPOSD_SAD = (PWPOSD_SAD == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
        PWPOSD_SAD_AFTER = PWPOSD_SAD;
    }

    return status;
}

/**
 * @brief WarpOSD Off For E4K
 * @param [in] ePage
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Off_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;
    uint16 retry = 1000;

    if(ePage == eWPOSDPAGE_0)
    {
        //status &= dvProAV_AccessWrite(eWrpOsdMemRdAdr, PWPOSD_SAD_BEFORE);

        status &= dvProAV_AccessWrite(eOsdMemRdAdr, POSD_SAD);

        LOG_MSG(db_HAL_WARPING, "(%s %d) PWPOSD_SAD_BEFORE = 0x%08x\n", __FUNCTION__, __LINE__, PWPOSD_SAD_BEFORE);
        LOG_MSG(db_HAL_WARPING, "(%s %d) POSD_SAD = 0x%08x\n", __FUNCTION__, __LINE__, POSD_SAD);

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
        status &= dvProAV_AccessWrite(eOsdDispTrig, true);              // OSD display

        OSD_ResetTimeInState();
        while(dvProAV_OSD_Vs_WaitStateGet() != 0)  // check busy
        {
            MS_SLEEP(1);
    #ifndef QT_CPP
            if(OSD_TimeElapsedInState() >= (OSDBUSYTIMEOUT / 2))
            {
               DBMSG_ALWAYS("OSD_On Time Out\n");
               break;
            }
    #endif
        }

        //清除下一塊待畫的RAM buffer
        POSD_SAD = (POSD_SAD == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
        dvProAV_WarpOSD_PageClear_E4K(ePage);

        POSD_SAD = (POSD_SAD == PWPOSD_SAD1)? PWPOSD_SAD2 : PWPOSD_SAD1;
        PWPOSD_SAD_BEFORE = POSD_SAD;

        status &= dvProAV_AccessWrite(eOsdEnable, (uint16)false);        // 顯示OSD畫面
    }
    else
    {


        status &= dvProAV_AccessWrite(eWrpOsdMem2RdAdr, PWPOSD_SAD);

        PWPOSD_SAD_AFTER = PWPOSD_SAD;
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
        //status &= dvProAV_AccessWrite(eWrpOsdEnable, (uint16)true);
        status &= dvProAV_AccessWrite(eWrpOsdTrig, (uint16)eWPOSD_DisplayOSDData);

        while((dvProAV_WarpOSD_BusyStateGet_E4K() != 0) && retry)  // busy
        {
            MS_SLEEP(1);
            retry--;
        }

        //清除下一塊待畫的RAM buffer
        PWPOSD_SAD = (PWPOSD_SAD == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
        dvProAV_WarpOSD_PageClear_E4K(ePage);

        PWPOSD_SAD = (PWPOSD_SAD == PWPOSD_SAD3)? PWPOSD_SAD4 : PWPOSD_SAD3;
        PWPOSD_SAD_AFTER = PWPOSD_SAD;


        status &= dvProAV_AccessWrite(eWrpOsdEnable, (uint16)false);        // 顯示OSD畫面
    }

    return status;
}

/**
 * @brief Warp OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Init_E4K(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, uint16 ucInhibit_Color)
{
    int status = rcSUCCESS;
    uint32 ulGetOutputSize = 0;
#ifndef QT_CPP
    uint32 u32SclSubFrmMod, u32WarpSubFrmMod;
#endif

    // 設定Memory位置
    POSD_SAD/*PWPOSD_SAD */ = DRAM_WRPOSD_MEM1_ADDR;
    PWPOSD_SAD_BEFORE = DRAM_WRPOSD_MEM1_ADDR;     // WarpOSDSAD(current) point to current WarpOSD memory plane
    PWPOSD_SAD_AFTER = DRAM_WRPOSD_MEM3_ADDR;     // WarpOSDSAD(current) point to current WarpOSD memory plane

    PWPOSD_SAD1 = DRAM_OSD_MEM1_ADDR;         // OSDSAD0 OSD memory plane 0
    PWPOSD_SAD2 = DRAM_OSD_MEM2_ADDR;         // OSDSAD1 OSD memory plane 1

    PWPOSD_SAD = DRAM_WRPOSD_MEM3_ADDR;
    PWPOSD_SAD3 = DRAM_WRPOSD_MEM3_ADDR;    // OSDSAD0 WarpOSD memory plane 0
    PWPOSD_SAD4 = DRAM_WRPOSD_MEM4_ADDR;    // OSDSAD1 WarpOSD memory plane 1

    // Scaler OSD setting
    //status &= dvProAV_WarpOSD_Set_E4K(true);
    // Read Output Size
    status &= dvProAV_AccessRead(eWcuMb2HSize, &ulGetOutputSize);
    m_panelWidth_WP = (uint16)ulGetOutputSize;
    status &= dvProAV_AccessRead(eWcuMb2VSize, &ulGetOutputSize);
    m_panelHeight_WP = (uint16)ulGetOutputSize;

#ifndef QT_CPP
    status &= dvProAV_AccessRead(eWcuSbfSpt, &u32WarpSubFrmMod);
    status &= dvProAV_AccessRead(eDcuSclSbfSpt, &u32SclSubFrmMod);

    if(((u32WarpSubFrmMod & 0x01) || (u32SclSubFrmMod & 0x01)) && (OSDePanelTimingId == PANEL_2D_HIGHSPEED))
    {
        m_panelWidth_WP *= 2;
        m_panelHeight_WP *= 2;
    }
#endif

    // High Color mode setting
    //status &= dvProAV_WarpOSD_HighColorModeEn(false);
    // 設定調色盤
    status &= dvProAV_WarpOSD_Palette_Set_E4K(iPalette_ColorAmount, pucPaletteData);
    // 清除Warp OSD畫面
    status &= dvProAV_AccessWrite(eWrpOsdEnable, false);
    //status &= dvProAV_WarpOSD_LayerModeSet(eLayerMode_P0BeforeP1AfterWarp);
    status &= dvProAV_WarpOSD_MixOn_E4K(true);
    // 設定讀取Warp OSD資料的DRAM位置
    status &= dvProAV_AccessWrite(eOsdMemWrAdr, PWPOSD_SAD_BEFORE);
    status &= dvProAV_AccessWrite(eOsdMemRdAdr, PWPOSD_SAD_BEFORE);
    status &= dvProAV_AccessWrite(eWrpOsdMem2RdAdr, PWPOSD_SAD_AFTER);

    // 設定OSD Line Buffer Size display panel size
    status &= dvProAV_AccessWrite(eWrpOsdLineBufCnt, (m_panelWidth_WP / PROAV_DRAM_LINE_BYTES));
    // 設定OSD Size
    status &= dvProAV_AccessWrite(eWrpOsdVidWidth, m_panelWidth_WP);
    status &= dvProAV_AccessWrite(eWrpOsdVidHeight, m_panelHeight_WP);
    // 設定禁止色
    dvProAV_WarpOSD_Inhibit_Color_Set_E4K(true, ucInhibit_Color);
    // 設定透明色
    status &= dvProAV_AccessWrite(eWrpOsdTranColor0, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdTranColor1, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdTranColor2, ucInhibit_Color);
    status &= dvProAV_AccessWrite(eWrpOsdTranColor3, ucInhibit_Color);

    // 切換OSD DRAM Buffer
    status &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_1);
    status &= dvProAV_WarpOSD_PageClear_E4K(eWPOSDPAGE_1);
    status &= dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE_1);
    status &= dvProAV_WarpOSD_PageClear_E4K(eWPOSDPAGE_1);

    return status;
}

/**
 * @brief dvProAV_WarpOSD_Set_E4K
 * @param [in] bE4K : true/fales : Warp OSD/Scaler OSD
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSD_Set_E4K(bool bWarpOsd)
{
    int status = rcSUCCESS;

    status &= dvProAV_WarpOSD_MemplaneSel_E4K(bWarpOsd);
    if(bWarpOsd)
    {
        if(!dvProAV_WarpOSDSubFrameModGet_E4K())
        status &= dvProAV_OSD_WriteSizeSet(m_panelWidth_WP, m_panelHeight_WP);
        else
            status &= dvProAV_OSD_WriteSizeSet(3840, 2400);
    }
    else
    {
        status &= dvProAV_OSD_WriteSizeSet(m_panelWidth, m_panelHeight);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// Warp OSD For Entry 4K Sub-Frame Mode
//-------------------------------------------------------------------------------------------------
/**
 * @brief Warp OSD Sub Frame Mode Flag Set
 * @param [in] bSubFrameMod
 *              - false : normal mode
 *              - true : sub frame mode
 */
void dvProAV_WarpOSDSubFrameModFlagSet_E4K(bool bSubFrameMod)
{
    m_b4k3dEn = bSubFrameMod;
}

/**
 * @brief Warp OSD Sub Frame Mode Flag Get
 * @return bSubFrameMod
 *              - false : normal mode
 *              - true : sub frame mode
 */
bool dvProAV_OSD_4k3dEn_Get(VOID)
{
    return m_b4k3dEn;
}

/**
 * @brief Warp OSD Sub Frame Mode Enable
 * @param [in] bSubFrameMod :
 *              - false : normal mode
 *              - true : sub frame mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSDSubFrameModEn_E4K(bool bSubFrameMod)
{
    return dvProAV_AccessWrite(eOsdXPROn, bSubFrameMod);
}
/**
 * @brief Warp OSD Sub Frame Mode Get
 * @return SubFrameMod :
 *              - false : normal mode
 *              - true : sub frame mode
 */
bool dvProAV_WarpOSDSubFrameModGet_E4K(void)
{
    uint32 u32SubFrameMod = 0;
    dvProAV_AccessRead(eOsdXPROn, &u32SubFrameMod);
    return (bool)u32SubFrameMod;
}

/**
 * @brief Control Copy Data From Warp OSD DRAM Address At Sub Frame Mode(only use for bitblt and multi-circle)
 * @param [in] bBmpCurrentPage
 *              - false : copy data from non-XPR mode page
 *              - true : copy data from XPR display mode page
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpOSDPaintBmpCtrl_E4K(bool bBmpCurrentPage)
{
    return dvProAV_AccessWrite(eOsdXPRbitbmpInOsd, bBmpCurrentPage);
}

UINT32 dvProAV_OSD_AddressGet_E4K(eWPOSDPAGE ePage)    //A70LK_CL_0002
{
    UINT32 ulOsdAddress;

    if(ePage == eWPOSDPAGE_0)   //before warp
    {
        ulOsdAddress = (PWPOSD_SAD_BEFORE == PWPOSD_SAD1) ? PWPOSD_SAD2 : PWPOSD_SAD1;
    }
    else  //after warp
    {
        ulOsdAddress = (PWPOSD_SAD_AFTER == PWPOSD_SAD3) ? PWPOSD_SAD4 : PWPOSD_SAD3;
    }
    LOG_MSG(db_HAL_WARPING, "(%s %d) OSD address for dram write = 0x%08x\n", __FUNCTION__, __LINE__, ulOsdAddress);
    return ulOsdAddress;
}

int dvProAV_WarpOSD_MemplaneLastSel_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    if(ePage >= eWPOSDPAGE_Max)
        return rcERROR;

    // 設定寫入位置，設定background address為寫入OSD的位置
    if(ePage == eWPOSDPAGE_0)
    {
        POSD_SAD = PWPOSD_SAD_BEFORE;
        status &= dvProAV_AccessWrite(eOsdMemWrAdr, PWPOSD_SAD_BEFORE);

        LOG_MSG(db_HAL_WARPING, "(%s, %d) POSD_SAD = %08x\n", __FUNCTION__,__LINE__, PWPOSD_SAD_BEFORE);
    }
    else
    {
        status &= dvProAV_AccessWrite(eOsdMemWrAdr, PWPOSD_SAD_AFTER);

        LOG_MSG(db_HAL_WARPING, "(%s, %d) PWPOSD_SAD = %08x\n", __FUNCTION__,__LINE__, PWPOSD_SAD_AFTER);
    }

    return status;
}

void dvProAV_OSD_Buffer_Init(VOID)
{
    // 切換OSD DRAM Buffer
    OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    OSD_Swap_Memplane();  //set OSD memory plane to first and reset that

    return;
}

#endif

void dvProAV_OSD_DbgPrint(void)  //H2PF_Simon_0038
{
    LOG_MSG(db_ALWAYS, "\r\n======= OSD =======\r\n");
    MS_SLEEP(5);

    LOG_MSG(db_ALWAYS, "POSD_SAD 0x%X\r\n\r\n", POSD_SAD);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "POSD_SAD0 0x%X\r\n", POSD_SAD0);
    MS_SLEEP(5);
    LOG_MSG(db_ALWAYS, "POSD_SAD1 0x%X\r\n", POSD_SAD1);
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

