//#include <string.h>
#include <stdint.h>

#include "dvC341.h"
#include "dvC341_OSD.h"
#include "utilDbgMsg.h"
#include "opdCtrlAPI.h"
//	#ifdef CUSTOM_BARCO     //A65_Owen_0002
//	#include "OSD_BARCO/Custom_OSD.h"
//	#else
//	#include "OSD/Custom_OSD.h"
//	#endif

#define EVB_BOARD_FLASH

//#define FLASH_3_BYTES_ADDR

#define FLASH_SIZE_256Mb        0
#define FLASH_SIZE_512Mb        1
#define FLASH_SIZE              FLASH_SIZE_512Mb


#if (FLASH_SIZE == FLASH_SIZE_512Mb)
UINT8 REG_SFLSZSEL = 0x02;
#else
UINT8 REG_SFLSZSEL = 0x01;
#endif

#ifdef FLASH_3_BYTES_ADDR
UINT8 REG_SFLCMDWR = 0x02;
UINT8 REG_SFLCMDRD = 0x0b;	// Fast Read = 0xB, normal read = 0x03
#else
UINT8 REG_SFLCMDWR = 0x12;
UINT8 REG_SFLCMDRD = 0x0c;	// Fast Read = 0xC, normal read = 0x13
#endif

#ifdef EVB_BOARD_FLASH
UINT8 REG_SFLCMDER = 0xc7;	// erase all : 0xC7, erase sector : 0xD8(3-byte mode_, 0xDC(4-byte mode)
#else
UINT8 REG_SFLCMDER = 0xD7;
#endif


int PS_PANEL_HW = 3840;
int PS_PANEL_VW = 2160;
int PS_PANEL_1CH_HW = 1920;
//for compiler error ////////////////////////////////////
#define m_iInhitbit_Color C341_m_iInhitbit_Color
#define PS_PANEL_VST C341_PS_PANEL_VST
#define PS_PANEL_HST C341_PS_PANEL_HST
/////////////////////////////////////////////////////////

//declare_gPOSD.h  ________________________________
 unsigned int POSD_SAD;// OSDSAD(current) point to current OSD memory plane
 unsigned int POSD_SAD0;// OSDSAD0  OSD memory plane 1
 unsigned int POSD_SAD1;// OSDSAD1 OSD memory plane 2
 unsigned int POSD_MWI;// MWI
 unsigned int POSD_REG_MWI;// OSDMWI
 unsigned int POSD_MPROC;// MPROC

 unsigned int POSD_PSAD;// OSD Part SAD of Bitmap
 unsigned int POSD_FSAD;// OSD Font SAD of Font
#if 0
unsigned int POSD_SAD_CH1;// OSDSAD1
unsigned int POSD_SAD_CH2;// OSDSAD1
unsigned int POSD_SAD_CH3;// OSDSAD1
unsigned int POSD_SAD_CH4;// OSDSAD1
#endif
 //char POSD_PART[400][400];
 //char POSD_SENTENCE[400][400];

 unsigned int POSD_HST;
// unsigned int POSD_HW;
 unsigned int POSD_VST;
// unsigned int POSD_VW;

// Serial Flash
 unsigned int POSD_SFL_PSAD;// OSD Part SAD
 unsigned int POSD_SFL_FSAD;// OSD Font SAD

//
 int POSD_DISP_CH;// for output channel swap

 //declare_gPS.h  __________________________________
 //A70LV_Doulas_0105 modify
int PS_PANEL_HST = 192;
int PS_PANEL_VST = 41;
#if 0
#if defined (DMD_DHD) //A70LV_Larry_0021
     int PS_PANEL_HST = 192;
     int PS_PANEL_VST = 41;
#elif defined (DMD_WUXGA)
     int PS_PANEL_HST = 112;
     int PS_PANEL_VST = 32;
#else
     int PS_PANEL_HST = 192;
     int PS_PANEL_VST = 41;
#endif /* DMD_DHD */
#endif  //A70LV_Doulas_0105 end


INT16 m_iInhitbit_Color = 0;
static TickType_t ulOSDStartTicks;

ePANEL_ID OSDePanelTimingId;    //A70LV_Doulas_0105

sOSD_INFO sOSDInfo = {
                        TRUE,
                     };

static void OSD_ResetTimeInState(void)
{
    ulOSDStartTicks = xTaskGetTickCount();
    return;
}

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

void dvC341_OSD_OPDLogOut(const char *pcStr, ...)   //G100_Owen_0080
{
    uOPD_DATA uOPDData = {0};
    va_list args;

    va_start(args, pcStr);

    vsnprintf(uOPDData.sINTERFACE.cString, 256, pcStr, args);
    va_end(args);

    utilOPD_EventSet((UINT8)eOPD_INTERFACE_LOG, &uOPDData);
}


//DMA transfer
//(Serial Flash read -> Register write selected in SFLREGSEL)
void OSD_DMA_SerialFlash_Set( UINT32 ulSflAd, UINT32 ulDestMemAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
    //dvC341_Write( B29_GIOS, 0x000f, 0 );

    UINT8 ucSFLSTAT = 0;

    //LOG_MSG(db_DV_SCALER, "C341 DMA_SerialFlash Start\r\n");

	dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

    dvC341_Write(B4_SFLCT, 0x00,0);
    dvC341_Write(B4_SFLMODE,0x80,0);

    if ((ucRegsel==0x01))
    {
        #ifdef FLASH_3_BYTES_ADDR
        dvC341_WriteToBuffer(B4_SFLMODE,0x17,0);
        #else
        dvC341_WriteToBuffer(B4_SFLMODE,0x37,0);
        #endif
    }
    else
    {
        #ifdef FLASH_3_BYTES_ADDR
        dvC341_WriteToBuffer(B4_SFLMODE,0x13,0);
        #else
        dvC341_WriteToBuffer(B4_SFLMODE,0x33,0);
        #endif
    }

    #ifdef FLASH_3_BYTES_ADDR
	dvC341_WriteToBuffer(B4_SFLSZSEL, REG_SFLSZSEL, 0);// 3byte Address: 0x00, 4byte Address: 0x01
	dvC341_WriteToBuffer(B4_SFLCMDWR, REG_SFLCMDWR, 0);// 3byte Address: 0x02, 4byte Address: 0x12
	dvC341_WriteToBuffer(B4_SFLCMDRD, REG_SFLCMDRD, 0);// 3byte Address: 0x0b, 4byte Address: 0x0c
	dvC341_WriteToBuffer(B4_SFLCMDWREN, 0x06, 0);
	dvC341_WriteToBuffer(B4_SFLCMDRDSR, 0x05, 0);
	dvC341_WriteToBuffer(B4_SFLAD, ulSflAd & 0xffffffff, 0);
	dvC341_WriteToBuffer(B4_SFLCNT, ulDmaCnt & 0xffffffff | 0x80000000, 0);//b31:wait
	dvC341_WriteToBuffer(B4_SFLREGSEL, ucRegsel & 0x0f, 0);
	#else
    dvC341_WriteToBuffer(B4_SFLCMDWR,REG_SFLCMDWR,0); //Serial Flash write command setting
    dvC341_WriteToBuffer(B4_SFLCMDRD,REG_SFLCMDRD,0); //Serial Flash read command setting
    dvC341_WriteToBuffer(B4_SFLCMDWREN,0x06,0); //Serial Flash WREN command setting
    dvC341_WriteToBuffer(B4_SFLCMDRDSR,0x05,0); //Serial Flash RDSR command setting
    dvC341_WriteToBuffer(B4_SFLSZSEL, REG_SFLSZSEL, 0);
    dvC341_WriteToBuffer(B4_SFLAD, ulSflAd&0xffffffff,0);
    dvC341_WriteToBuffer(B4_SFLCNT, ulDmaCnt&0xffffff,0);
    dvC341_WriteToBuffer(B4_SFLREGSEL, ucRegsel & 0x0f,0); //DDR3-SDRAM
    #endif

    if (ucRegsel==0x00)
    {
        dvC341_WriteToBuffer(B4_OSDCT, 0, 0);
        dvC341_WriteToBuffer(B4_CPUWAD, ulDestMemAd&0xffffffff,0);
    }
    else if (ucRegsel==0x04)
    { // PLT0
        dvC341_WriteToBuffer(B9_PLTAD1CH1,ulDestMemAd&0xff,0);
    }
    else if (ucRegsel==0x05)
    { // PLT0
        dvC341_WriteToBuffer(B25_PLTAD2CH2,ulDestMemAd&0xff,0);
    }
    else if (ucRegsel==0x01)
    { // CBUF
        dvC341_WriteToBuffer(B4_CBUFAD,ulDestMemAd&0xffff,0);
    }

    dvC341_WriteToBuffer(B4_SFLCT, 0x02,0);
    dvC341_Buffer_Flush();


    OSD_ResetTimeInState();

    do
    {
        ucSFLSTAT = dvC341_Read( B4_SFLSTAT, 0 );

        // NOP
		if(OSD_TimeElapsedInState() >= 3000)
        {
            LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) : time out\r\n", __FUNCTION__, __LINE__);
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) : time out\r\n", __FUNCTION__, __LINE__);
            break;
        }

    }while((ucSFLSTAT & 0x01) != 0 );


    dvC341_Write(B4_SFLCT,0x00,0);
    LOG_MSG(db_DV_SCALER_OSD, "OSD_TimeElapsedInState (%d)\r\n", OSD_TimeElapsedInState());

    LOG_MSG(db_DV_SCALER_OSD, "DMA_SerialFlash Done (X%08X)(X%08X)(%02d)(X%08X)\r\n", ulSflAd, ulDestMemAd, ucRegsel, ulDmaCnt);
}

//DMA transfer
//(Register read selected in SFLREGSEL --> serial Flash write)
void OSD_DMA_DDR3_WriteTo_Flash( UINT32 ulSrcMemAd, UINT32 ulSflAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
    //dvC341_Write( B29_GIOS, 0x000f, 0 );
    UINT8 ucSFLDTCTL = 0xff;
    UINT8 ucSFLSTAT = 0xff;
    UINT8 ucSFLRDSR = 0xff;

    LOG_MSG(db_DV_SCALER_OSD, "DMA_SerialFlash Start\r\n");

    dvC341_Write(B4_SFLCT,0x00,0);
    dvC341_Write(B4_SFLMODE,0x80,0);

    if ((ucRegsel==0x01)||(ucRegsel==0x02))
    {
    	#ifdef FLASH_3_BYTES_ADDR
        dvC341_Write(B4_SFLMODE,0x17,0);
    	#else
        dvC341_Write(B4_SFLMODE,0x37,0);
    	#endif
    }
    else
    {
    	#ifdef FLASH_3_BYTES_ADDR
        dvC341_Write(B4_SFLMODE,0x13,0);
    	#else
        dvC341_Write(B4_SFLMODE,0x33,0);
    	#endif
    }


    #ifdef FLASH_3_BYTES_ADDR
	dvC341_WriteToBuffer(B4_SFLSZSEL, REG_SFLSZSEL, 0);// 3byte Address: 0x00, 4byte Address: 0x01
	dvC341_WriteToBuffer(B4_SFLCMDWR, REG_SFLCMDWR, 0);// 3byte Address: 0x02, 4byte Address: 0x12
	//dvC341_WriteToBuffer(B4_SFLCMDRD, REG_SFLCMDRD, 0);// 3byte Address: 0x0b, 4byte Address: 0x0c
	dvC341_WriteToBuffer(B4_SFLCMDWREN, 0x06, 0);
	dvC341_WriteToBuffer(B4_SFLCMDRDSR, 0x05, 0);
	dvC341_WriteToBuffer(B4_SFLAD, ulSflAd & 0xffffffff, 0);
	dvC341_WriteToBuffer(B4_SFLCNT, ulDmaCnt & 0xffffffff | 0x80000000, 0);//b31:wait
	dvC341_WriteToBuffer(B4_SFLREGSEL, ucRegsel & 0x0f, 0);
	#else
    dvC341_Write(B4_SFLCMDWR,REG_SFLCMDWR,0); //Serial Flash write command setting
    //dvC341_Write(B4_SFLCMDRD,REG_SFLCMDRD,0); //Serial Flash read command setting
    dvC341_Write(B4_SFLCMDWREN,0x06,0); //Serial Flash WREN command setting
    dvC341_Write(B4_SFLCMDRDSR,0x05,0); //Serial Flash RDSR command setting
    dvC341_Write(B4_SFLSZSEL, REG_SFLSZSEL, 0);
    dvC341_Write(B4_SFLAD, ulSflAd&0xffffffff,0);
    dvC341_Write(B4_SFLCNT, ulDmaCnt&0xffffff,0);
    dvC341_Write(B4_SFLREGSEL, ucRegsel & 0x0f,0); //DDR3-SDRAM
    #endif

    if (ucRegsel==0x00)
    {
        dvC341_Write(B4_CPURAD, ulSrcMemAd&0xffffffff,0);
        dvC341_Write(B4_OSDCT, 0, 0);
    }
    else if ((ucRegsel==0x01)||(ucRegsel==0x02))
    { // PLT0
        dvC341_Write(B9_PLTAD1CH1,ulSrcMemAd&0xff,0);
    }
    else if (ucRegsel==0x03)
    { // CBUF
        dvC341_Write(B4_CBUFAD,ulSrcMemAd&0xffff,0);
    }

    dvC341_Write(B4_SFLCT, 0x01,0);
    //dvC341_Buffer_Flush();

    OSD_ResetTimeInState();

    do
    {
        MS_SLEEP(100);
        ucSFLSTAT = dvC341_Read( B4_SFLSTAT, 0 );

        // NOP
        if(OSD_TimeElapsedInState() >= 30000)
        {
            LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) ucSFLSTAT:0x%X time out\r\n", __FUNCTION__, __LINE__, ucSFLSTAT);
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) ucSFLSTAT:0x%X time out\r\n", __FUNCTION__, __LINE__, ucSFLSTAT);
            break;
        }

    }while((ucSFLSTAT & 0x01) != 0 );
    LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) OSD_TimeElapsedInState (%d)\r\n", __FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    //dvC341_Write(B4_SFLCT,0x00,0);

    LOG_MSG(db_DV_SCALER_OSD, "DMA_SerialFlash Done (X%08X)(X%08X)(%02d)(X%08X)\r\n", ulSrcMemAd, ulSflAd, ucRegsel, ulDmaCnt);
}

/******************  osd_mproc for C734  *************************
    cEnable           --> OSD memory protect enable(0,1)
    ulReg_mproc    --> OSD memory protect address
*************************************************************************/
static void OSD_Memory_Protect( INT8 cEnable, UINT32 ulReg_mproc )
{
    UINT32 ulMemProtecth;

    ulMemProtecth = ( cEnable == 1 ) ? ulReg_mproc | 0x8000 : ulReg_mproc & 0x7fff;

    dvC341_Write( B4_MPROTECT, ulMemProtecth & 0xffff, 0 );
}


/******************  osd_acct for C734  *************************
    ucControlData     --> OSD access control
*************************************************************************/
static void OSD_AccessControl( UINT8 ucControlData )
{
    dvC341_Write( B4_BOACCT, ucControlData&0xff, 0 );
}


static void OSD_Swap_Memplane(void)
{
    START_POINT sStart;
    RECT_SIZE sSize;

    POSD_SAD = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;

    sStart.iX = 0;
    sStart.iY = 0;
    sSize.iWidth = 1920;
    sSize.iHeight = 1200;

    dvC341_Inhibit_Color_Set(0, m_iInhitbit_Color);
    dvC341_Paint_Rectangle(sStart, sSize, m_iInhitbit_Color);
}


/******************  osd_blink_ct for C734  ************************
    cEnable            ->  blink enable
    on_period     ->  blink ON period(0.1sec yp 10sec on POCLK freq=150MHz)
    off_period    ->  blink OFF period
    cBlinkTp_Enable       ->  blink transparent color enable
*************************************************************************/
void OSD_Blink_Control(INT8 cEnable, DOUBLE dOn_period, DOUBLE dOff_period, INT8 cBlinkTp_Enable)
{
    UINT32 ulOSDmode;
    INT16 iBlinkTime, iBlinkCycl;
    UINT8 ucCh_ofst = 0;


    #if 0
    ucCh_ofst = ( POSD_DISP_CH == 0 ) ? 0
        : ( POSD_DISP_CH == 1 ) ? 4
        : ( POSD_DISP_CH == 2 ) ? 8
        : 12;
    #endif

    ulOSDmode = dvC341_Read( B9_OSDMODE2CH1, ucCh_ofst ) & 0xffff;

    if(cBlinkTp_Enable == 1)
    {
        ulOSDmode = ulOSDmode | (BIT5);    // blink transparent
    }
    else
    {
        ulOSDmode = ulOSDmode & (~(BIT5));
    }

    iBlinkTime = (int)(dOn_period * 150000000 / 2097152);

    if(cEnable == 0)
    {
        iBlinkCycl = 0;
    }
    else
    {
        iBlinkCycl = (int)((dOn_period + dOff_period) * 150000000 / 2097152);
    }

    dvC341_WriteToBuffer( B9_OSDMODE2CH1,    ulOSDmode & 0xffff, 0 );
    dvC341_WriteToBuffer( B9_BLINKTIME2CH1,  iBlinkTime & 0xff, 0 );
    dvC341_WriteToBuffer( B9_BLINKCYCL2CH1,  iBlinkCycl & 0xff, 0 );
    dvC341_WriteToBuffer( B25_OSDMODE2CH2,   ulOSDmode & 0xffff, 0 );
    dvC341_WriteToBuffer( B25_BLINKTIME2CH2, iBlinkTime & 0xff, 0 );
    dvC341_WriteToBuffer( B25_BLINKCYCL2CH2, iBlinkCycl & 0xff, 0 );
    dvC341_Buffer_Flush();
}


INT8 OSD_Palette_Set(INT16 iAmount, UINT8 *pucData )
{
    INT16 iCount = 0;

    if (iAmount != ICHIP_PALETTE_COLOR)
    {
       return PALETTE_AMOUNT_ERROR;
    }

    dvC341_WriteToBuffer( B9_PLTAD2CH1,  0, 0);
    dvC341_BurstWrite_FixedAdd( B9_PLTDT2CH1,  (iAmount*3), pucData+iCount);
    dvC341_WriteToBuffer( B25_PLTAD2CH2, 0, 0);
    dvC341_BurstWrite_FixedAdd( B25_PLTDT2CH2, (iAmount*3), pucData+iCount);

    UINT32 OSDMODE = dvC341_Read(B9_OSDMODE2CH1, 0 );

    OSDMODE |= BIT14;  //Use color palette

    if(sOSDInfo.ucMagnifyOSD2X)
    {
        OSDMODE |= BIT12;  //OSD horizontal double size
        OSDMODE |= BIT11;  //OSD vertical   double size
    }

    dvC341_WriteToBuffer( B9_OSDMODE2CH1,  OSDMODE, 0 );
    dvC341_WriteToBuffer( B25_OSDMODE2CH2, OSDMODE, 0 );

    if(dvC341_Read(B9_OSDBLENDC2CH1, 0) == 0)     //A70LV_Doulas_0122 check Menu Transparency init
    {
        dvC341_WriteToBuffer( B9_OSDBLENDC2CH1    , 0x80 ,0);    //OSD blend coefficient(Menu Transparency value)
        dvC341_WriteToBuffer( B25_OSDBLENDC2CH2   , 0x80 ,0);    //OSD blend coefficient(Menu Transparency value)
    }
    dvC341_Buffer_Flush();

    return ICHIP_OSD_PASS;
}


/******************  osd_copy for C734  ************************
    source_adrs   ->  start address
    dest_adrs     ->  destination address
    bbhw          ->  bbhw
    bbvw          ->  bbvw
*************************************************************************/
void OSD_Copy(UINT32 source_adrs, UINT32 dest_adrs, INT16 ibbHor, INT16 ibbVer)
{
    int osdct;
    UINT32 ulBOSTAT = 0;
    UINT8 ucFastWrite = 0;

    osdct = dvC341_Read(B4_OSDCT, 0);

    //ulRad = POSD_PSAD + ( /*POSD_MWI*/0x0a * 128 * iSrc_y + iSrc_x );// bitblt source address  //0x0a* 128(source total bitmap width) should read from JSon file define
    //ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;  // bitblt destination address  //POSD_MWI = output OSD plane width

    //dvC734_Write( B4_BBWMWI, 0x08, 0 );  //8x128 = 1024
    //dvC734_Write( B4_BBRMWI, POSD_REG_MWI, 0 );  //0x20 x128 = 4096

    #ifdef OSDCT_FASTEN_ON
    if (ibbHor >= OSDCT_FASTEN_TRANSFER_UNIT &&
        ibbHor % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        ibbHor = ibbHor / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
    #endif

    dvC341_WriteToBuffer(B4_BBACTHW, (ibbHor-1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC341_WriteToBuffer(B4_BBACTVW, (ibbVer-1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC341_WriteToBuffer(B4_CPURAD, source_adrs & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC341_WriteToBuffer(B4_CPUWAD, dest_adrs & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    if(ucFastWrite)
    {
        dvC341_WriteToBuffer( B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_BITBLT | DVC341_OSDCT_FASTEN, 0 ); //OSD control register  ==> BitBLT
    }
    else
    {
        dvC341_WriteToBuffer( B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_BITBLT, 0 ); //OSD control register  ==> BitBLT
    }

    dvC341_Buffer_Flush();
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            return;
        }

    }while((ulBOSTAT & 0x01) != 0 );

}

void OSD_CurrentCopy(void) //A70LV_Larry_0067
{
    UINT32 osdct;
    UINT32 source_adrs = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;
    UINT32 dest_adrs = POSD_SAD;
    UINT16 uibbHor = 0;
    UINT16 uibbVer = 0;
    UINT32 ulBOSTAT = 0;
    UINT8 ucFastWrite = 0;

    if(sOSDInfo.ucMagnifyOSD2X)
    {
        uibbHor = PS_PANEL_HW / 2;
        uibbVer = PS_PANEL_VW / 2;
    }
    else
    {
        uibbHor = PS_PANEL_HW;
        uibbVer = PS_PANEL_VW;
    }

    #ifdef OSDCT_FASTEN_ON
    if (uibbHor >= OSDCT_FASTEN_TRANSFER_UNIT &&
        uibbHor % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        uibbHor = uibbHor / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
    #endif

    osdct = dvC341_Read(B4_OSDCT, 0);

    dvC341_WriteToBuffer(B4_BBACTHW, (uibbHor-1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC341_WriteToBuffer(B4_BBACTVW, (uibbVer-1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC341_WriteToBuffer(B4_CPURAD, source_adrs & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC341_WriteToBuffer(B4_CPUWAD, dest_adrs & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    if(ucFastWrite)
    {
        dvC341_WriteToBuffer( B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_BITBLT | DVC341_OSDCT_FASTEN, 0 ); //OSD control register  ==> BitBLT
    }
    else
    {
        dvC341_WriteToBuffer( B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_BITBLT, 0 ); //OSD control register  ==> BitBLT
    }

    dvC341_Buffer_Flush();
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            return;
        }
    }while((ulBOSTAT & 0x01) != 0 );
}

// ==============================================================================
// FUNCTION NAME: dvC341_SerialFlash_CheckSum
// DESCRIPTION:
//
//
// Params:
// UINT32 ulSflAd:
// UINT32 ulDataSize:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/31, Larry Create
// --------------------
// ==============================================================================
UINT32 dvC341_SerialFlash_CheckSum(UINT32 ulSflAd, UINT32 ulDataSize)
{
    UINT8   ucSFLDTCTL = 0xff;
    //UINT8   ucFLSTAT = 0xff;
    //UINT8   ucSFLRDSR = 0xff;
    //UINT8   ucData = 0;
    UINT16  uiCount = 0;
    UINT32  ulBLOCK = ulDataSize/0x100; //A70LV_Larry_0054
    UINT32  ulCount = 0;
    UINT32  ulTotoalCheckSum = 0, ulCheckSum = 0;
    UINT32  ulReadFlashAdd = ulSflAd;
    UINT32  ulSectorSize = 0; //A70LV_Larry_0158
    UINT8 ucaData[0x100] = {0};

	dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

    dvC341_Write(B4_SFLCT,0x00,0);
    dvC341_Write(B4_SFLMODE,0x80,0); //Serial Flash mode setting //Reset

    #ifdef FLASH_3_BYTES_ADDR
    dvC341_Write(B4_SFLMODE,0x13,0);
    #else
    dvC341_Write(B4_SFLMODE,0x33,0); //A70LV_Larry_0102
    #endif

    dvC341_Write(B4_SFLCMDRD, REG_SFLCMDRD, 0);
    dvC341_Write(B4_SFLSZSEL, REG_SFLSZSEL,0); //Specify the size of the serial Flash to be connected. //256 Mbit
    dvC341_Write(B4_SFLAD,(ulSflAd&0xFFFFFFFF),0); //Serial Flash address
    dvC341_Write(B4_SFLDTCTL, 0x01, 0);


    LOG_MSG(db_DV_SCALER_OSD, "Read Add 0x%08x-0x%08x\n", ulSflAd, ulSflAd + ulDataSize);

    for(ulCount = 0; ulCount < ulBLOCK; ulCount++) //A70LV_Larry_0054
    {
        ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL, 0);
        OSD_ResetTimeInState();

        while((ucSFLDTCTL & 0x80))
        {
            ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL, 0);

            if(OSD_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                return ulTotoalCheckSum;
            }
        }

        dvC341_BurstRead_FixedAdd(B4_SFLDT, 0x100, ucaData);

        for(uiCount = 0; uiCount < 0x100; uiCount++)
        {
            ulTotoalCheckSum = ulTotoalCheckSum + (ucaData[uiCount] * (ulSectorSize + 1));
            ulCheckSum = ulCheckSum + (ucaData[uiCount] * (ulSectorSize + 1));
            ulReadFlashAdd++;
            ulSectorSize++;
        }

        // //A70LV_Larry_0052

        if((ulCount&0xFF) == 0xFF) //A70LV_Larry_0054
        {
            LOG_MSG(db_DV_SCALER_OSD, "Add(0x%08x) 0x%08x-0x%08x CheckSum 0x%08x(0x%08x)\n", ulReadFlashAdd, ulCount - 0xffff, ulCount, ulCheckSum, ulTotoalCheckSum);
            ulCheckSum = 0;
        }


    }


    dvC341_Write(B4_SFLAD,0,0);

    dvC341_Write(B4_SFLDTCTL, 0x00, 0);

    LOG_MSG(db_DV_SCALER_OSD, "Total CheckSum 0x%08x\n", ulTotoalCheckSum);

    return ulTotoalCheckSum;
}

eRESULT dvC341_RAM_Read(UINT32 ulRAMAd, UINT8 *pData, UINT32 ulDataSize) //A35G2_CDS_Larry_0050
{
    UINT32  ulCount = 0;
    UINT8 *pBuffer = malloc(ulDataSize + 0x1000);
    UINT8 ucVAl = 0;

    if(pBuffer != NULL)
    {
        memset(pBuffer, 0xff, ulDataSize);

        dvC341_Write(B4_CPURAD, ulRAMAd, 0);
        dvC341_Write(B4_CPUDTCTL, 0x01 ,0);

        OSD_ResetTimeInState();

        ucVAl = 0x08;
        while((ucVAl & 0x08) != 0)
        {
            ucVAl = (UINT8)dvC341_Read(B4_CPUDTCTL,0);

            if(OSD_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                free(pBuffer);
                return rcERROR;
            }
        }

        for(ulCount = 0; ulCount < ulDataSize; (ulCount = ulCount + 0x1000))
        {
            dvC341_BurstRead_FixedAdd(B4_CPUDT, 0x1000, &pBuffer[ulCount]);
        }

        if(pData != NULL)
        {
            memcpy(pData, pBuffer, ulDataSize);
        }

        free(pBuffer);
    }

    return rcSUCCESS;
}

eRESULT dvC341_RAM_Write(UINT32 ulRAMAd, UINT8 *pData, UINT32 ulDataSize) //A35G2_CDS_Larry_0050
{
    UINT32  ulCount = 0;

    dvC341_Write(B4_CPUWAD, ulRAMAd, 0);

    for(ulCount = 0; ulCount < ulDataSize; (ulCount = ulCount + 0x1000))
    {
        dvC341_BurstWrite_FixedAdd(B4_CPUDT, 0x1000, &pData[ulCount]);
    }
    //在下一次CPUWAD，確保CPUDT資料有被更新
    ulRAMAd = 0;
    dvC341_Write(B4_CPUWAD, ulRAMAd, 0);

    return rcSUCCESS;
}


// ==============================================================================
// FUNCTION NAME: dvC341_SerialFlash_Erase_All
// DESCRIPTION:
//
//
// Params:
// :
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/31, Larry Create
// --------------------
// ==============================================================================
void dvC341_SerialFlash_Erase_All(void)
{
    unsigned int wsad;
    int RDTCHK;

    wsad = 0x00000000;
    //dvC734_Write(B29_GIOS, 0x000f, 0);
    LOG_MSG(db_DV_SCALER, "FlasEraseAll_Start\n");

	dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

    dvC341_Write(B4_SFLSZSEL, REG_SFLSZSEL, 0);
    dvC341_Write(B4_SFLCT, 0x00, 0);
    dvC341_Write(B4_SFLMODE, 0x80, 0);
    #ifdef FLASH_3_BYTES_ADDR
    dvC341_Write(B4_SFLMODE, 0x03, 0);
    #else
    dvC341_Write(B4_SFLMODE, 0x23, 0); //A35G2_CDS_Larry_0032 0x33 -> 0x23
    #endif
    dvC341_Write(B4_SFLCT, 0x00, 0);
    dvC341_Write(B4_SFLCMDWR, REG_SFLCMDWR, 0);  //Set the page write command to the serial Flash connected
    dvC341_Write(B4_SFLCMDRD, REG_SFLCMDRD, 0);  //Set a read or fast read command to be executed for the serial Flash connected
    dvC341_Write(B4_SFLCMDER, REG_SFLCMDER, 0);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
    dvC341_Write(B4_SFLCMDWREN, 0x06, 0);  //Set the value of WREN command of the serial Flash connected
    dvC341_Write(B4_SFLCMDRDSR, 0x05, 0);  //Set the value of RDSR command of the serial Flash connected
    dvC341_Write(B4_SFLAD, wsad, 0);  //Specify the serial Flash address. After accessing, auto increment starts
    dvC341_Write(B4_SFLCT, 0x05, 0);



    OSD_ResetTimeInState();
    do
    {
        RDTCHK = dvC341_Read(B4_SFLSTAT, 0);

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
        LOG_MSG(db_DV_SCALER, ".");

    } while ((RDTCHK & 0x02)==0x02);
    LOG_MSG(db_DV_SCALER, "\nWait B4_SFLSTAT Done\n");

    OSD_ResetTimeInState();
    do
    {
        RDTCHK = dvC341_Read(B4_SFLRDSR, 0);

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }while ((RDTCHK & 0x03)!=0x00);
    LOG_MSG(db_DV_SCALER, "\nWait B4_SFLRDSR Done\n");

    LOG_MSG(db_DV_SCALER, "FlasEraseAll_Done\n");

	return;
}

// ==============================================================================
// FUNCTION NAME: dvC341_SerialFlash_Erase_Sector
// DESCRIPTION:
//
//
// Params:
// UINT32 ulSflAd:
// UINT32 ulDataSize:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/31, Larry Create
// --------------------
// ==============================================================================
void dvC341_SerialFlash_Erase_Sector(UINT32 ulSflAd, UINT32 ulDataSize)
{
    UINT32 ulEraseAddrStart = ulSflAd;
    UINT32 ulEraseAddrEnd   = ulSflAd + ulDataSize;
    UINT8  ucRDTCHK;

    if((ulEraseAddrStart & 0x0000FFFF) || (ulEraseAddrEnd & 0x0000FFFF))
    {
        LOG_MSG(db_DV_SCALER_OSD, "Not matching 64k\n");
        return;
    }

    LOG_MSG(db_DV_SCALER_OSD, "Sector Erase Start\n");

	dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

    for(ulEraseAddrStart = ulSflAd; ulEraseAddrStart < ulEraseAddrEnd; ulEraseAddrStart += 0x00010000) //64k
    {
        LOG_MSG(db_DV_SCALER_OSD, "Erase Sector 0x%08x - 0x%08x\n", ulEraseAddrStart, ulEraseAddrStart + 0x0000FFFF);

         //A70LV_Larry_0052

        dvC341_Write(B4_SFLSZSEL, REG_SFLSZSEL, 0);
        dvC341_Write(B4_SFLCT, 0x00, 0);
        dvC341_Write(B4_SFLMODE, 0x80, 0);
        #ifdef FLASH_3_BYTES_ADDR
        dvC341_Write(B4_SFLMODE, 0x03, 0);	//0x13 --> 0x03 (Normal Read)
        #else
        dvC341_Write(B4_SFLMODE, 0x23, 0); //A35G2_CDS_Larry_0032 0x33 -> 0x23
        #endif
        dvC341_Write(B4_SFLCT, 0x00, 0);
        dvC341_Write(B4_SFLCMDWR, REG_SFLCMDWR, 0);  //Set the page write command to the serial Flash connected
        dvC341_Write(B4_SFLCMDRD, 0x13 /*REG_SFLCMDRD*/, 0);  //Set a read or fast read command to be executed for the serial Flash connected
        dvC341_Write(B4_SFLCMDER, 0xDC /*REG_SFLCMDER*/, 0);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
        dvC341_Write(B4_SFLCMDWREN, 0x06, 0);  //Set the value of WREN command of the serial Flash connected
        dvC341_Write(B4_SFLCMDRDSR, 0x05, 0);  //Set the value of RDSR command of the serial Flash connected
        dvC341_Write(B4_SFLAD, ulEraseAddrStart, 0);  //Specify the serial Flash address. After accessing, auto increment starts
        dvC341_Write(B4_SFLCT, 0x04, 0);

        OSD_ResetTimeInState();

        do
        {
            ucRDTCHK = dvC341_Read(B4_SFLSTAT, 0); //A70LV_Larry_0054
            MS_SLEEP(50);
            if(OSD_TimeElapsedInState() >= 2000)
            {
                LOG_MSG(db_DV_SCALER_OSD, "Erase Time Out 1\n");
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }

        } while((ucRDTCHK & 0x02)==0x02);

        OSD_ResetTimeInState();
        do
        {
            ucRDTCHK = dvC341_Read(B4_SFLRDSR, 0); //A70LV_Larry_0054
            MS_SLEEP(50);
            if(ucRDTCHK & 0x02)
            {
                dvC341_Write(B4_SFLCMDER, 0x04, 0);
                dvC341_Write(B4_SFLCT, 0x05, 0);
            }

            if(OSD_TimeElapsedInState() >= 2000)
            {
                LOG_MSG(db_DV_SCALER_OSD, "Erase Time Out 2\n");
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }

        } while((ucRDTCHK & 0x03) !=0x00 );

         //A70LV_Larry_0052
    }

    LOG_MSG(db_DV_SCALER_OSD, "Sector Erase End\n");

    return;
}


// ==============================================================================
// FUNCTION NAME: dvC341_SerialFlash_Write
// DESCRIPTION:
//
//
// Params:
// UINT32 ulSflAd:
// UINT8 *pucBuffer:
// UINT32 ulDataSize:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/10/31, Larry Create
// --------------------
// ==============================================================================
void dvC341_SerialFlash_Write(UINT32 ulSflAd, UINT8 *pucBuffer, UINT32 ulDataSize)
{
    UINT8   ucSFLDTCTL = 0xff;
    UINT8   ucSFLSTAT = 0xff;
    //UINT8   ucSFLRDSR = 0xff;
    UINT8   ucRDTCHK = 0xff;
    UINT32  ulBLOCK = ulDataSize;
    UINT16  uiCount = 0;
    UINT8   aucData[256] = {0};
    UINT32  ulADDRCount = 0;
    UINT32  ulWriteADDR = ulSflAd;
    UINT32  ulCount = 0;
    UINT8*  pucData = pucBuffer;
    UINT32  ulTotoalCheckSum = 0;
    UINT32  ulCheckSum = 0;
    uintptr_t ulReadFlashAdd = (uintptr_t)pucBuffer;
    UINT32  ulSectorSize = 0; //A70LV_Larry_0158

    //matching 64k
    if(ulBLOCK & 0x0000FFFF)
    {
        ulBLOCK += 0x00010000;
        ulBLOCK &= 0xFFFF0000;
        ulBLOCK = ulBLOCK / 0x100;
    }
    else
    {
        ulBLOCK = ulBLOCK / 0x100;
    }

    if(ulWriteADDR & 0x0000FFFF)
    {
        LOG_MSG(db_DV_SCALER_OSD, "Not matching 64k\n");
        return;
    }

    LOG_MSG(db_DV_SCALER_OSD, "Address 0x%08x\n", ulWriteADDR);
    LOG_MSG(db_DV_SCALER_OSD, "Size    0x%08x\n", ulBLOCK*0x100);
    LOG_MSG(db_DV_SCALER_OSD, "Data    0x%08x\n", ulReadFlashAdd);

    dvC341_Write(B5_GIOS, 0x000055, 0);//2bits for each GIO[X]
	dvC341_Write(B5_GIOPDCT,0x03FF, 0);//Enable pulldown for all GIO
	dvC341_Write(B5_GIOC, 0x000300, 0);//Drive   GIO[9:8](WP#,HLD#)
	dvC341_Write(B5_GIOO, 0x000300, 0);//High to GIO[9:8](WP#,HLD#)
	dvC341_Write(B4_OSDCT, 0x00, 0);

#if 1
    dvC341_Write(B4_SFLCT, 0x00,0);
    dvC341_Write(B4_SFLMODE, 0x80,0); //Serial Flash mode setting //Reset

    #ifdef FLASH_3_BYTES_ADDR
    dvC341_Write(B4_SFLMODE, 0x13,0); //A70LV_Larry_0102
    #else
    dvC341_Write(B4_SFLMODE, 0x33,0); //A70LV_Larry_0102
    #endif

    dvC341_Write(B4_SFLCMDWR, REG_SFLCMDWR, 0); //Serial Flash write command setting
    dvC341_Write(B4_SFLCMDWREN, 0x06, 0); //Serial Flash WREN command setting
    dvC341_Write(B4_SFLCMDRDSR, 0x05, 0); //Serial Flash RDSR command setting
    dvC341_Write(B4_SFLSZSEL, REG_SFLSZSEL, 0); //Specify the size of the serial Flash to be connected. //256 Mbit
    dvC341_Write(B4_SFLAD, (ulWriteADDR & 0xffffffff), 0); //Serial Flash address

    LOG_MSG(db_DV_SCALER_OSD, "Flash Copy start\n");

    for(ulCount = 0; ulCount < ulBLOCK; ulCount++)
    {
         //A70LV_Larry_0052
		OSD_ResetTimeInState();	//ZU860_Clare_0118, add

        do
		{
			ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL,0);
			if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
			{
				LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 1\n");
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
				break;
			}
		} while ((ucSFLDTCTL & 0x20)==0x00);
        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
        do
		{
			ucSFLSTAT = dvC341_Read(B4_SFLSTAT, 0);
			if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
			{
				LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 2\n");
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
				break;
			}
		}  while ((ucSFLSTAT & 0x02)==0x02);

        for (uiCount = 0; uiCount < 0x100; uiCount++)
        {
            aucData[uiCount] = *pucData++;//acData[cCount];
            ulCheckSum += (aucData[uiCount] * (ulSectorSize + 1)); //A70LV_Larry_0158
            ulTotoalCheckSum += (aucData[uiCount] * (ulSectorSize + 1)); //A70LV_Larry_0158
            ulSectorSize++; //A70LV_Larry_0158
//            dvC341_Write_NoSemaphore(B4_SFLDT, (ucData & 0xFF), 0); //A70LV_Larry_0052

            if((ulADDRCount&0xFFFF) == 0xFFFF)
            {
                //MS_SLEEP(10);

                LOG_MSG(db_DV_SCALER_OSD, "Add(0x%08x) 0x%08x-0x%08x CheckSum 0x%08x\n", ulReadFlashAdd, ulADDRCount - 0x0FFF, ulADDRCount, ulCheckSum);
                ulCheckSum = 0;
            }

            ulADDRCount = ulADDRCount + 1;
//            if((ulADDRCount & 0xff)==0)
//            {
//                do{ ucSFLDTCTL = dvC341_Read_NoSemaphore(B4_SFLDTCTL,0);} while ((ucSFLDTCTL & 0x20)==0x00);
//            }

        }
        dvC341_BurstWrite_FixedAdd(B4_SFLDT, 0x100, aucData);

        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
        do
        {
            ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL,0);
            if(OSD_TimeElapsedInState() >= 1000)    //ZU860_Clare_0118, add
            {
                LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 3\n");
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }
        } while ((ucSFLDTCTL & 0x20)==0x00);

        if((ulADDRCount & 0xff)!=0)
        {
            dvC341_Write(B4_SFLAD, ((ulWriteADDR + ulADDRCount) & 0xffffffff), 0);
            OSD_ResetTimeInState(); //ZU860_Clare_0118, add
            do
			{
				ucSFLDTCTL = dvC341_Read(B4_SFLDTCTL,0);
				if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
				{
					LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 4\n");
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
					break;
				}
			} while ((ucSFLDTCTL & 0x20)==0x00);
	        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
            do
			{
				ucSFLSTAT = dvC341_Read(B4_SFLSTAT, 0);
				if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
				{
					LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 5\n");
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
					break;
				}
			}  while ((ucSFLSTAT & 0x02)==0x02);
	        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
            do
			{
				ucRDTCHK = dvC341_Read(B4_SFLRDSR, 0);
				if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
				{
					LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 6\n");
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
					break;
				}
			}  while ((ucRDTCHK & 0x03)!=0x00);
			OSD_ResetTimeInState();	//ZU860_Clare_0118, add
        }

        ulReadFlashAdd = ulReadFlashAdd + 0x100;

         //A70LV_Larry_0052

    }

    LOG_MSG(db_DV_SCALER_OSD, "Flash Copy End\n");

    LOG_MSG(db_DV_SCALER_OSD, "Write Total CheckSum 0x%08x\n", ulTotoalCheckSum);

    MS_SLEEP(10);
#endif
    if(ulTotoalCheckSum != dvC341_SerialFlash_CheckSum(ulSflAd, ulBLOCK*0x100))
    {
        LOG_MSG(db_DV_SCALER_OSD, "Flash Copy Fail\n");
    }
    else
    {
        LOG_MSG(db_DV_SCALER_OSD, "Flash Copy Pass\n");
    }

}


/******************  osd_write_inh_ct for C734  *************************
    winh_en      --> OSD write-inhibit color enable (0 or 1)
    inh_color    --> OSD write-inhibit color (color pallet number)
*************************************************************************/
void dvC341_Inhibit_Color_Set( INT8 cEnable, INT16 iInhibit_Color )
{
    UINT8 ucOsdct;

    ucOsdct = (cEnable ==1) ? 0x20 : 0x00;

    m_iInhitbit_Color = iInhibit_Color;

    dvC341_WriteToBuffer( B4_OSDCT, ucOsdct & 0xff, 0 );
    dvC341_WriteToBuffer( B4_BOWINH, m_iInhitbit_Color & 0xff, 0 );
    dvC341_Buffer_Flush();
}


void dvC341_OSD_Off(void)
{
    UINT32 ulOSDmode, ulOSDmode2, ulOSDmode3, ulOSDmode4;
    UINT32 ulRtct;

    ulOSDmode = dvC341_Read( B9_OSDMODE2CH1,  0) & (~(BIT1));
    ulOSDmode2= dvC341_Read( B25_OSDMODE2CH2, 0) & (~(BIT1));
    ulOSDmode3= dvC341_Read( B41_OSDMODE2CH3, 0) & (~(BIT1));
    ulOSDmode4= dvC341_Read( B57_OSDMODE2CH4, 0) & (~(BIT1));

    ulRtct = dvC341_Read( B0_RTCT0, 0 ) & 0xffffffff;

    dvC341_WriteToBuffer( B0_RTCT0, (RTCT_OP_STOP<<4)|RTCT_OP_STOP, 0 );

    dvC341_WriteToBuffer( B9_OSDMODE2CH1,  ulOSDmode  & 0xffff, 0);
    dvC341_WriteToBuffer( B25_OSDMODE2CH2, ulOSDmode2 & 0xffff, 0);
    dvC341_WriteToBuffer( B41_OSDMODE2CH3, ulOSDmode3 & 0xffff, 0);
    dvC341_WriteToBuffer( B57_OSDMODE2CH4, ulOSDmode4 & 0xffff, 0);

    dvC341_WriteToBuffer( B0_RTCT0, (RTCT_OP_POVSCH3CH4<<4)|RTCT_OP_POVSCH1CH2, 0 );
    dvC341_Buffer_Flush();

    dvC341_wait1_povs(eC341_CH_V0);  //wait_povs
    //dvC341_wait1_povs(eC341_CH_V1);  //wait_povs

    dvC341_Write( B0_RTCT0, ulRtct, 0 );
}

void dvC341_OSD_On( START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color)
{
	UINT32 ulOSDmode1;//, ulOSDmode2, ulOSDmode3, ulOSDmode4;
	UINT32 ulRddt1_read;//, ulRddt2_read, ulRddt3_read, ulRddt4_read;
	UINT32 ulOSDmode2;      //A70LV_Doulas_0098
	UINT32 ulRddt2_read;    //A70LV_Doulas_0098
	//UINT32 ulRtct;
    UINT8 ucCh1_ofst = 0;
    UINT8 ucCh2_ofst = CH_BANK_OFFSET;

    //UINT8 ucCh3_ofst;
    //UINT8 ucCh4_ofst;

    START_POINT sCH2_start;
    //START_POINT sCH3_start;
    //START_POINT sCH4_start;
    RECT_SIZE sHalf_Size;
    UINT8 ucCH_enable;  //Bit0:CH1 enable, Bit1:CH2 enable, Bit2:CH2 enable, Bit3:CH4 enable
    ePANEL_ID eOutputPanel = dvC341_PanelGet();     //A70LV_Doulas_0098

    //ucCh3_ofst = 8;
    //ucCh4_ofst = 12;

    //sCH2_start.iX = 960;
    //sCH2_start.iY = 0;
    //sCH3_start.iX = 1920;
    //sCH3_start.iY = 0;
    //sCH4_start.iX = 2880;
   // sCH4_start.iY = 0;

    sHalf_Size.iWidth  = PS_PANEL_HW / 2;
    sHalf_Size.iHeight = PS_PANEL_VW;

    if(sOSDInfo.ucMagnifyOSD2X)
    {
        sDes_position.iX  *= 2;
        sDes_position.iY  *= 2;
        sDes_Size.iHeight *= 2;
        sDes_Size.iWidth  *= 2;
    }

#if 0
    switch(OSDePanelTimingId)   //A70LV_Doulas_0105 Modify
    {
        case ePANEL_ID_WUXGA_60HZ:
        case ePANEL_ID_WUXGA_120HZ:
        case ePANEL_ID_WUXGA_240HZ:
            sHalf_Size.iWidth = 1920/2;
            sHalf_Size.iHeight = 1200;
            break;

        case ePANEL_ID_1080P_60HZ:
        case ePANEL_ID_1080P_120HZ:
        case ePANEL_ID_1080P_240HZ:
            sHalf_Size.iWidth = 1920/2;
            sHalf_Size.iHeight = 1080;
            break;

        case ePANEL_ID_3840x2160_60HZ:
        case ePANEL_ID_3840x2160_60HZ:
            sHalf_Size.iWidth = 3840/2;
            sHalf_Size.iHeight = 2160;
            break;

        case ePANEL_ID_3840x2400_60HZ:
            sHalf_Size.iWidth = 3840/2;
            sHalf_Size.iHeight = 2400;
            break;

        default:
            sHalf_Size.iWidth = 1920/2;
            sHalf_Size.iHeight = 1080;
            break;
    }
#endif

    ucCH_enable = 0;
    ucCH_enable = ucCH_enable | 0x01;  //enable ch1 first

    sCH2_start.iX = PS_PANEL_1CH_HW * 1;
    sCH2_start.iY = 0;
    if (sDes_position.iX+sDes_Size.iWidth >= sCH2_start.iX)
    {
        ucCH_enable = ucCH_enable | 0x03;  //enable ch1 ~ ch2
    }

    //sCH2_start.iX = sDes_position.iX + sDes_Size.iWidth;
    //sCH2_start.iY = 0;

    #if 0
    if((eOutputPanel == ePANEL_ID_1080P_120HZ) ||
	   (eOutputPanel == ePANEL_ID_WUXGA_120HZ))  //G100_Doulas_0064 Modify//A70LV_Doulas_0098
    {
        sCH2_start.iX = 960;
        sCH2_start.iY = 0;
        if (sDes_position.iX+sDes_Size.iWidth >= sCH2_start.iX)
        {
            ucCH_enable = ucCH_enable | 0x03;  //enable ch1 ~ ch2
        }

        if (sDes_position.iX >= sCH2_start.iX)
        {
            ucCH_enable = ucCH_enable & 0xfe;  //disable ch1
        }
    }
    else
    {
        sCH2_start.iX = sDes_position.iX + sDes_Size.iWidth;
        sCH2_start.iY = 0;
    }
    #endif

    ulRddt1_read = dvC341_Read( B9_OSDMODE2CH1, ucCh1_ofst ) & 0xffff;
    ulRddt2_read = dvC341_Read( B9_OSDMODE2CH1, ucCh2_ofst ) & 0xffff;    //A70LV_Doulas_0098

	//ulRtct = dvC341_Read( B0_RTCT, 0 ) & 0xffffffff;

    if( cTrap_Enable == 1 )
    {
        ulOSDmode1 = ulRddt1_read | 0x0007; //A70LV_Larry_0055 //for warping 0x03->0x07
        ulOSDmode2 = ulRddt2_read | 0x0007;       //A70LV_Doulas_0098
    }
    else
    {
        ulOSDmode1 = (ulRddt1_read & 0xfffc) | 0x0002;
        ulOSDmode2 = (ulRddt2_read & 0xfffc) | 0x0002;  //A70LV_Doulas_0098
    }

	dvC341_WriteToBuffer( B0_RTCT0, (RTCT_OP_STOP<<4)|RTCT_OP_STOP, 0 );

    if (ucCH_enable & 0x01)  // Ch1 setting
    {
        INT16 iDes_width;
        iDes_width = sDes_Size.iWidth - (sDes_position.iX + sDes_Size.iWidth - sCH2_start.iX);    //A70LV_Doulas_0098 modify
        if (iDes_width < 0)
            iDes_width = sDes_Size.iWidth;
        else if (iDes_width > sHalf_Size.iWidth)
            iDes_width = sHalf_Size.iWidth;

        //LOG_MSG(db_DV_SCALER_OSD, "OSD ON(%d,%d) size(%d,%d)\r\n", sDes_position.iX, sDes_position.iY, sDes_Size.iWidth, sDes_Size.iHeight);

        #if 0
        dbmsg("\nCh1 setting\n");
        dbmsg("\niDes_width = %d\n", iDes_width);
        dbmsg("sDes_Size.iWidth  = %d\n", sDes_Size.iWidth);
        dbmsg("sDes_Size.iHeight = %d\n", sDes_Size.iHeight);
        dbmsg("sDes_position.iX  = %d\n", sDes_position.iX);
        dbmsg("sDes_position.iY  = %d\n", sDes_position.iY);

        dbmsg("POSD_HST  = %d\n", POSD_HST);
        dbmsg("POSD_VST  = %d\n", POSD_VST);

        dbmsg("\nB9_OSDACTHST1CH1 = %d\n", (POSD_HST+sDes_position.iX));
        dbmsg("B9_OSDACTHW1CH1  = %d\n", iDes_width);
        dbmsg("B9_OSDACTVST1CH1 = %d\n", (POSD_VST+sDes_position.iY));
        dbmsg("B9_OSDACTVW1CH1  = %d\n\n", (sDes_Size.iHeight));
        #endif

    	dvC341_WriteToBuffer( B9_OSDSAD2CH1, POSD_SAD/*POSD_SAD_CH1*/ & 0xffffffff, ucCh1_ofst );  //Always set start address to POSD_SAD, because OSD memory plane always start paint fucntion at (0,0)
    	dvC341_WriteToBuffer( B9_OSDACTHST2CH1, (POSD_HST+sDes_position.iX) & 0xffff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_OSDACTHW2CH1, ((iDes_width)) & 0xffff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_OSDACTVST2CH1, (POSD_VST+sDes_position.iY) & 0xffff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_OSDACTVW2CH1, (sDes_Size.iHeight) & 0xffff, ucCh1_ofst );

        dvC341_WriteToBuffer( B9_OSDMWI2CH1, POSD_REG_MWI & 0xff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_OSDMODE2CH1, ulOSDmode1 & 0xffff, ucCh1_ofst );
        //dvC341_Write( B3_OSDMODE1CH1, 0x22, ucCh1_ofst );

    	dvC341_WriteToBuffer( B9_BOTRANS02CH1, sTrap_Color.ucT_Color1 & 0xff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_BOTRANS12CH1, sTrap_Color.ucT_Color2 & 0xff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_BOTRANS22CH1, sTrap_Color.ucT_Color3 & 0xff, ucCh1_ofst );
    	dvC341_WriteToBuffer( B9_BOTRANS32CH1, sTrap_Color.ucT_Color4 & 0xff, ucCh1_ofst );


    }

    if (ucCH_enable & 0x02)  // Ch2 setting     //A70LV_Doulas_0098
    {
        INT16 iDes_x;
        INT16 iDes_width;

        if (sCH2_start.iX <= sDes_position.iX)
        {
            iDes_x = sDes_position.iX - sCH2_start.iX;
        }
        else
        {
            iDes_x = 0;
        }

        if (iDes_x>0)
        {
            iDes_width = sDes_Size.iWidth;
        }
        else
        {
            iDes_width = sDes_position.iX + sDes_Size.iWidth - sCH2_start.iX;
        }



        //LOG_MSG(db_DV_SCALER_OSD, "OSD ON(%d,%d) size(%d,%d)\r\n", sDes_position.iX, sDes_position.iY, sDes_Size.iWidth, sDes_Size.iHeight);
        if (sCH2_start.iX <= sDes_position.iX)
        {
            dvC341_WriteToBuffer( B9_OSDSAD2CH1, POSD_SAD & 0xffffffff, ucCh2_ofst );
        }
        else
        {
            if(sOSDInfo.ucMagnifyOSD2X)
            {
			    dvC341_WriteToBuffer(B9_OSDSAD2CH1, (POSD_SAD + (sCH2_start.iX - sDes_position.iX /*+ sDes_position.iY * POSD_REG_MWI * 128*/) / 2) & 0xffffffff, ucCh2_ofst);
			}
			else
			{
			    dvC341_WriteToBuffer(B9_OSDSAD2CH1, (POSD_SAD + (sCH2_start.iX - sDes_position.iX /*+ sDes_position.iY * POSD_REG_MWI * 128*/)) & 0xffffffff, ucCh2_ofst);
			}
        }

        #if 0
        dbmsg("\nCh2 setting\n");
        dbmsg("\niDes_width = %d\n", iDes_width);
        dbmsg("sDes_Size.iWidth  = %d\n", sDes_Size.iWidth);
        dbmsg("sDes_Size.iHeight = %d\n", sDes_Size.iHeight);
        dbmsg("sDes_position.iX  = %d\n", sDes_position.iX);
        dbmsg("sDes_position.iY  = %d\n", sDes_position.iY);

        dbmsg("POSD_HST  = %d\n", POSD_HST);
        dbmsg("POSD_VST  = %d\n", POSD_VST);

        dbmsg("\nB25_OSDACTHST1CH2 = (POSD_HST +iDes_x)          = %d\n", (POSD_HST+iDes_x));
        dbmsg("B25_OSDACTHW1CH2  = (iDes_width)                = %d\n", iDes_width);
        dbmsg("B25_OSDACTVST1CH2 = (POSD_VST+sDes_position.iY) = %d\n", (POSD_VST+sDes_position.iY));
        dbmsg("B25_OSDACTVW1CH2  = (sDes_Size.iHeight)         = %d\n\n", (sDes_Size.iHeight));
        #endif

    	dvC341_WriteToBuffer( B9_OSDACTHST2CH1, (POSD_HST+iDes_x) & 0xffff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_OSDACTHW2CH1, ((iDes_width)) & 0xffff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_OSDACTVST2CH1, (POSD_VST+sDes_position.iY) & 0xffff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_OSDACTVW2CH1, (sDes_Size.iHeight) & 0xffff, ucCh2_ofst );

        dvC341_WriteToBuffer( B9_OSDMWI2CH1, POSD_REG_MWI & 0xff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_OSDMODE2CH1, ulOSDmode2 & 0xffff, ucCh2_ofst );
        //dvC341_Write( B3_OSDMODE1CH1, 0x22, ucCh1_ofst );

    	dvC341_WriteToBuffer( B9_BOTRANS02CH1, sTrap_Color.ucT_Color1 & 0xff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_BOTRANS12CH1, sTrap_Color.ucT_Color2 & 0xff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_BOTRANS22CH1, sTrap_Color.ucT_Color3 & 0xff, ucCh2_ofst );
    	dvC341_WriteToBuffer( B9_BOTRANS32CH1, sTrap_Color.ucT_Color4 & 0xff, ucCh2_ofst );


    }

	//dvC341_Write( B0_RTCT, B3_OSDMODE1CH1, 0 ); //A70LV_Larry_0173 ??

	dvC341_WriteToBuffer( B0_RTCT0, (RTCT_OP_POVSCH1CH2<<4)|RTCT_OP_POVSCH1CH2, 0 );
    dvC341_Buffer_Flush();

    if (ucCH_enable & 0x01)  // Ch1 setting
	    dvC341_wait1_povs(eC341_CH_V0); //OSD CH1 wait_povs

    //if (ucCH_enable & 0x02)  // Ch2 setting //A70LV_Doulas_0098
	//    dvC341_wait1_povs(eC341_CH_V1);  //OSD CH1 wait_povs

    OSD_Swap_Memplane();  //Swap memory plane automatically after OSD drawing every time
}

/******************  osd_bias for C734  *************************/
/****************************************************************
    rbias     ->  Red   bias (-128 to 127)
    gbias     ->  Green bias (-128 to 127)
    bbias     ->  Blue  bias (-128 to 127)
 ****************************************************************/
void dvC341_OSD_Bias_Set(OSD_BIAS_COLOR sBiasSetting)
{
    UINT8 ucCh_ofst = 0;

    #if 0
    ucCh_ofst = ( POSD_DISP_CH == 0 ) ? 0
        : ( POSD_DISP_CH == 1 ) ? 4
        : ( POSD_DISP_CH == 2 ) ? 8
        : 12;
    #endif

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ch_v = 0 ;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
        dvC341_WriteToBuffer(B9_OSDBIASR02CH1, sBiasSetting.sBias_Color1.cBias_R & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASG02CH1, sBiasSetting.sBias_Color1.cBias_G & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASB02CH1, sBiasSetting.sBias_Color1.cBias_B & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASR12CH1, sBiasSetting.sBias_Color2.cBias_R & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASG12CH1, sBiasSetting.sBias_Color2.cBias_G & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASB12CH1, sBiasSetting.sBias_Color2.cBias_B & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASR22CH1, sBiasSetting.sBias_Color3.cBias_R & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASG22CH1, sBiasSetting.sBias_Color3.cBias_G & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASB22CH1, sBiasSetting.sBias_Color3.cBias_B & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASR32CH1, sBiasSetting.sBias_Color4.cBias_R & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASG32CH1, sBiasSetting.sBias_Color4.cBias_G & 0xff, ucRegOffset );
        dvC341_WriteToBuffer(B9_OSDBIASB32CH1, sBiasSetting.sBias_Color4.cBias_B & 0xff, ucRegOffset );
    }

    dvC341_Buffer_Flush();

}


/******************  osd_gain for C734  *************************
//  en        ->  gain enable
    rgain     ->  gain for red   (0 to 3.99)
    ggain     ->  gain for green (0 to 3.99)
    bgain     ->  gaub fir blue  (0 to 3.99)
*************************************************************************/
void dvC341_OSD_Gain_Set( INT8 cEnable, OSD_GAIN_COLOR sGainSetting)
{
    INT16 iRed0, iGreen0, iBlue0;
    INT16 iRed1, iGreen1, iBlue1;
    INT16 iRed2, iGreen2, iBlue2;
    INT16 iRed3, iGreen3, iBlue3;
    UINT32 ulOSDmode;
    UINT8 ucCh_ofst = 0;

    #if 0
    ucCh_ofst = ( POSD_DISP_CH == 0 ) ? 0
        : ( POSD_DISP_CH == 1 ) ? 4
        : ( POSD_DISP_CH == 2 ) ? 8
        : 12;
    #endif

    iRed0   = (INT16)( sGainSetting.sGain_Color1.fGain_R * 64 );
    iGreen0 = (INT16)( sGainSetting.sGain_Color1.fGain_G * 64 );
    iBlue0  = (INT16)( sGainSetting.sGain_Color1.fGain_B * 64 );
    iRed1   = (INT16)( sGainSetting.sGain_Color2.fGain_R * 64 );
    iGreen1 = (INT16)( sGainSetting.sGain_Color2.fGain_G * 64 );
    iBlue1  = (INT16)( sGainSetting.sGain_Color2.fGain_B * 64 );
    iRed2   = (INT16)( sGainSetting.sGain_Color3.fGain_R * 64 );
    iGreen2 = (INT16)( sGainSetting.sGain_Color3.fGain_G * 64 );
    iBlue2  = (INT16)( sGainSetting.sGain_Color3.fGain_B * 64 );
    iRed3   = (INT16)( sGainSetting.sGain_Color4.fGain_R * 64 );
    iGreen3 = (INT16)( sGainSetting.sGain_Color4.fGain_G * 64 );
    iBlue3  = (INT16)( sGainSetting.sGain_Color4.fGain_B * 64 );

    ulOSDmode = dvC341_Read( B9_OSDMODE2CH1, ucCh_ofst ) & 0xffff;

    if( cEnable == 1 )
    {
        ulOSDmode = ulOSDmode | 0x10;
    }
    else
    {
        ulOSDmode = ulOSDmode & 0xffef;
    }

    for(UINT8 ch=0; ch<OUTPUT4K_CH_NUM; ch++)
    {
        UINT8 BankOffset = 16;
        UINT8 ch_v = 0 ;
        UINT8 ucRegOffset = ((ch_v * 2) + ch) * BankOffset;
        dvC341_WriteToBuffer( B9_OSDGAINR02CH1, iRed0,      ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAING02CH1, iGreen0,    ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINB02CH1, iBlue0,     ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINR12CH1, iRed1,      ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAING12CH1, iGreen1,    ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINB12CH1, iBlue1,     ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINR22CH1, iRed2,      ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAING22CH1, iGreen2,    ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINB22CH1, iBlue2,     ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINR32CH1, iRed3,      ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAING32CH1, iGreen3,    ucRegOffset );
        dvC341_WriteToBuffer( B9_OSDGAINB32CH1, iBlue3,     ucRegOffset );

        dvC341_WriteToBuffer( B9_OSDMODE1CH1, ulOSDmode & 0xffff, ucRegOffset );
    }

    dvC341_Buffer_Flush();
}



INT8 dvC341_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color)
{
    INT8 cRet = ICHIP_OSD_PASS;
    OSD_BIAS_COLOR sBias_Color;
    LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);


    POSD_REG_MWI = DEF_OSDMWI_4K;
    POSD_SAD0 = DEF_OSDSAD_LAYER0_4K & 0x3fffffff;
    POSD_SAD1 = DEF_OSDSAD_LAYER1_4K & 0x3fffffff;
    POSD_PSAD = DEF_BITMAPAD_4K & 0x3fffffff;
    POSD_FSAD = DEF_FONTAD_4K & 0x3fffffff;
#if 0
    POSD_SAD_CH1 = DEF_OSDSAD_CH1_4K & 0x3fffffff;
    POSD_SAD_CH2 = DEF_OSDSAD_CH2_4K & 0x3fffffff;
    POSD_SAD_CH3 = DEF_OSDSAD_CH3_4K & 0x3fffffff;
    POSD_SAD_CH4 = DEF_OSDSAD_CH4_4K & 0x3fffffff;
#endif

    POSD_MPROC = (DEF_MPROTECT_4K & 0x7fff);

    POSD_SFL_PSAD = DEF_SFL_OSD_PSAD_2K;
    POSD_SFL_FSAD = DEF_SFL_OSD_FSAD_2K;

    POSD_SAD = POSD_SAD0;
    POSD_MWI = POSD_REG_MWI * 128; //0x20*128

    switch(OSDePanelTimingId)   //A70LV_Doulas_0226 modify//A70LV_Doulas_0105
    {
        case ePANEL_ID_WUXGA_60HZ:
            PS_PANEL_HST = OSD_H_START_WUGA_60;//56;
            if(dvC341_PIP_PBP_Enable() == TRUE)
            {
                PS_PANEL_VST = OSD_V_START_WUGA_60;// - 2;//20;//32;	//A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_WUGA_60;//20;//32;  A70LV_Doulas_0170 modify
            }
            break;

        case ePANEL_ID_1080P_60HZ:
            PS_PANEL_HST = OSD_H_START_1080P_60;//192;
            if(dvC341_PIP_PBP_Enable() == TRUE)
            {
                PS_PANEL_VST = OSD_V_START_1080P_60;// - 2;//41;		//A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_1080P_60;//41;
            }
            break;

        case ePANEL_ID_1080P_120HZ:
            PS_PANEL_HST = OSD_H_START_1080P_120;//24;
            PS_PANEL_VST = OSD_V_START_1080P_120;//41;
            break;

		case ePANEL_ID_WUXGA_120HZ:     //G100_Doulas_0064
            PS_PANEL_HST = OSD_H_START_WUXGA_120;
            PS_PANEL_VST = OSD_V_START_WUXGA_120;
            break;

		case ePANEL_ID_3840x2160_60HZ:
            PS_PANEL_HST = 384/2 + 1;
            PS_PANEL_VST = 80 + 2;
            break;

		case ePANEL_ID_3840x2160_60HZ_594MHz:
            PS_PANEL_HST = 384/2 + 1;
            PS_PANEL_VST = 80 + 2;
            break;

		case ePANEL_ID_3840x2400_60HZ:
            PS_PANEL_HST = 80/2 + 1;
            PS_PANEL_VST = 20;
            break;

        default:
            {
                dbmsg("!!! OSD H/V Start undefined.... use default\r\n");
                PS_PANEL_HST = 80/2 + 1;
                PS_PANEL_VST = 20;
            }
            break;
    }

    POSD_HST = (PS_PANEL_HST - 5);// OSDACTHST offset = 5  //A70LV_Doulas_0105 //A70LV_Larry_0054 5->7
    POSD_VST = (PS_PANEL_VST + 0);// OSDACTVST offset = -1 //A70LV_Doulas_0105

    POSD_DISP_CH = 0;  //( PS_OCH_SWP == 1 ) ? 2 : 0;// for output channel swap  //PS_OCH_SWP(m_sChannelInfo[eCH].bOutputChSwap)

    cRet = OSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);

    dvC341_WriteToBuffer( B4_OSDCT, 0x00, 0 );
    dvC341_WriteToBuffer( B9_OSDMWI2CH1, POSD_REG_MWI,0); //0x20
    dvC341_WriteToBuffer( B4_BBWMWI, POSD_REG_MWI, 0 );
    dvC341_WriteToBuffer( B4_BBRMWI, DVC341_OSD_BITMAP_SKETCHPAD_WIDTH/*POSD_REG_MWI*/, 0 );
    dvC341_Buffer_Flush();

    OSD_DMA_SerialFlash_Set( POSD_SFL_PSAD, POSD_PSAD, 0, DV_BITMAP_RAW_DATA_SIZE );// OSD Part  //modify dma size to fit new bitmap data
    OSD_DMA_SerialFlash_Set( POSD_SFL_FSAD, POSD_FSAD, 0, DV_TEXT_RAW_DATA_SIZE );// OSD Font  //modify dma size to fit new font data
    OSD_Memory_Protect( 1, POSD_MPROC );// v12 //A70LV_Larry_0415
    dvC341_Inhibit_Color_Set( 1, ucInhibit_Color );
    dbmsg("!!! check B4_BOACCT\r\n");
    OSD_AccessControl( 0x33 );// v12
    //dvC341_OSD_Off();

    sBias_Color.sBias_Color1.cBias_R = 0;//128; //127;
    sBias_Color.sBias_Color1.cBias_G = 0;//128; //127;
    sBias_Color.sBias_Color1.cBias_B = 0;//128; //127;
    sBias_Color.sBias_Color2.cBias_R = 0;
    sBias_Color.sBias_Color2.cBias_G = 0;
    sBias_Color.sBias_Color2.cBias_B = 0;
    sBias_Color.sBias_Color3.cBias_R = 0;
    sBias_Color.sBias_Color3.cBias_G = 0;
    sBias_Color.sBias_Color3.cBias_B = 0;
    sBias_Color.sBias_Color4.cBias_R = 0;
    sBias_Color.sBias_Color4.cBias_G = 0;
    sBias_Color.sBias_Color4.cBias_B = 0;
    dvC341_OSD_Bias_Set(sBias_Color);

    OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    OSD_Swap_Memplane();  //set OSD memory plane to first and reset that

    LOG_MSG(db_DV_SCALER_OSD, "dvC341_OSD_Init reg init complete\r\n");

    return cRet;
}

INT8 dvC341_OSD_Reload(void)
{
    LOG_MSG(db_DV_SCALER_OSD, "dvC341_OSD_Reload Start\r\n");
    dvC341_OSD_Memory_Protect(0);

    OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_PSAD_2K, (DEF_BITMAPAD_4K & 0x3fffffff), 0, DV_BITMAP_RAW_DATA_SIZE );// OSD Part  //modify dma size to fit new bitmap data
    OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_FSAD_2K, (DEF_FONTAD_4K & 0x3fffffff),   0, DV_TEXT_RAW_DATA_SIZE );// OSD Font  //modify dma size to fit new font data

    LOG_MSG(db_DV_SCALER_OSD, "dvC341_OSD_Reload Done\r\n");

    dvC341_OSD_Memory_Protect(1);

    return ICHIP_OSD_PASS;
}

INT8 dvC341_OSD_Memory_Protect(UINT8 ucEnable) //A35G2_CDS_Larry_0050
{
    OSD_Memory_Protect( ucEnable, (DEF_MPROTECT_4K & 0x7fff));// v12

    return ICHIP_OSD_PASS;
}

#if (LOGO_REPLACE == 1)
INT8 dvC341_2ND_LOGO_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color, UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0068 //A65_OPTOMA_Julie_0067
{
    int status = rcSUCCESS;

    OSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);

	if(ucIndex == eCM_USER_LOGO_SERVICE_2D)
	{
		OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_SERVICE_2ND_LOGO, POSD_PSAD, 0, OSD_2ND_LOGO_BITMAP_SIZE); //modify dma size to fit new bitmap data
	}
	else
	{
	    OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_2ND_LOGO, POSD_PSAD, 0, OSD_2ND_LOGO_BITMAP_SIZE); //modify dma size to fit new bitmap data
	}

    dvC341_Inhibit_Color_Set( 1, ucInhibit_Color );
    OSD_AccessControl( 0x33 );// v12

    OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    OSD_Swap_Memplane();  //set OSD memory plane to first and reset that

    LOG_MSG(db_DV_SCALER_OSD, "2ND logo index[%d], dvC341_2ND_LOGO_OSD_Init complete\r\n", ucIndex);

    return (status == rcSUCCESS);
}

INT8 dvC341_OSD_2ND_LOGO_Reload(UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
{
    POSD_PSAD = DEF_BITMAPAD_4K & 0x3fffffff;
    LOG_MSG(db_UPGRADE, "%s(%d)[0x%08x, 0x%08x] Start\r\n", __func__, ucIndex, POSD_MPROC, POSD_PSAD);
    dvC341_OSD_Memory_Protect(0);

	if(ucIndex == eCM_USER_LOGO_SERVICE_2D) //A65_OPTOMA_Julie_0076
	{
		OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_SERVICE_2ND_LOGO, POSD_PSAD, 0, OSD_2ND_LOGO_BITMAP_SIZE); //modify dma size to fit new bitmap data
	}
	else
	{
	    OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_2ND_LOGO, POSD_PSAD, 0, OSD_2ND_LOGO_BITMAP_SIZE); //modify dma size to fit new bitmap data
	}

    dvC341_OSD_Memory_Protect(1);
    LOG_MSG(db_UPGRADE, "%s(%d)[0x%08x, 0x%08x] done\r\n", __func__, ucIndex, POSD_MPROC, POSD_PSAD);
    return ICHIP_OSD_PASS;
}
#endif

INT8 dvC341_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    INT16 iBmhw, iBmvw, iSrc_x, iSrc_y;
    UINT32 ulOsdct, ulRad, ulWad;
    UINT32 ulBOSTAT;
    UINT8 ucFastWrite = 0 ;

    ulOsdct = dvC341_Read( B4_OSDCT, 0 ) & 0xff;

    iSrc_x  = sBitmap.sSrc_Position.iX;
    iSrc_y  = sBitmap.sSrc_Position.iY;
    iBmhw = sBitmap.sBitmap_Size.iWidth;
    iBmvw = sBitmap.sBitmap_Size.iHeight;

    ulRad = POSD_PSAD + ( /*POSD_MWI*/DVC341_OSD_BITMAP_SKETCHPAD_WIDTH * 128 * iSrc_y + iSrc_x );// bitblt source address  //0x0a* 128(source total bitmap width) should read from JSon file define
	ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;  // bitblt destination address  //POSD_MWI = output OSD plane width

    #ifdef OSDCT_FASTEN_ON
    if (iBmhw >= OSDCT_FASTEN_TRANSFER_UNIT &&
        iBmhw % OSDCT_FASTEN_TRANSFER_UNIT == 0 &&
        sDes_position.iX % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        iBmhw = iBmhw / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
    #endif

    dvC341_WriteToBuffer(B4_BBACTHW, (iBmhw - 1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC341_WriteToBuffer(B4_BBACTVW, (iBmvw - 1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC341_WriteToBuffer(B4_CPURAD, ulRad & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC341_WriteToBuffer(B4_CPUWAD, ulWad & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    if(ucFastWrite)
    {
        dvC341_WriteToBuffer(B4_OSDCT, (ulOsdct & 0xE0) | DVC341_OSDCT_BITBLT | DVC341_OSDCT_FASTEN, 0 ); //OSD control register  ==> BitBLT
    }
    else
    {
        dvC341_WriteToBuffer(B4_OSDCT, (ulOsdct & 0xE0) | DVC341_OSDCT_BITBLT , 0 ); //OSD control register  ==> BitBLT
    }

    dvC341_Buffer_Flush();


    OSD_ResetTimeInState();

    do
    {
        if(OSD_TimeElapsedInState() >= 1000)
        {
            LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) Time Out\n", __FUNCTION__, __LINE__);
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }

        ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

#if 0 //G100_Larry_0020
        if((ulBOSTAT & 0x01) != 0 )
        {
            MS_SLEEP(1);
        }
#endif /* 0 */

    } while ( (ulBOSTAT & 0x01) != 0 );

    return ICHIP_OSD_PASS;
}

INT8 dvC341_Paint_Specified_Bitmap(OSD_BITMAP sBitmap, START_POINT sOffset_position, RECT_SIZE sSrc_Size, START_POINT sDes_position)
{
    INT16 iBmhw, iBmvw, iSrc_x, iSrc_y;
    UINT32 ulOsdct, ulRad, ulWad;
	UINT32 ulBOSTAT;

    ulOsdct = dvC341_Read( B4_OSDCT, 0 ) & 0xff;

    iSrc_x = sBitmap.sSrc_Position.iX + sOffset_position.iX;
    iSrc_y = sBitmap.sSrc_Position.iY + sOffset_position.iY;
    iBmhw = sSrc_Size.iWidth;
    iBmvw = sSrc_Size.iHeight;

    ulRad = POSD_PSAD + ( /*POSD_MWI*/DVC341_OSD_BITMAP_SKETCHPAD_WIDTH * 128 * iSrc_y + iSrc_x );// bitblt source address  //0x0a* 128(source total bitmap width) should read from JSon file define
	ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;  // bitblt destination address  //POSD_MWI = output OSD plane width

    dvC341_WriteToBuffer(B4_BBACTHW, (iBmhw - 1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC341_WriteToBuffer(B4_BBACTVW, (iBmvw - 1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC341_WriteToBuffer(B4_CPURAD, ulRad & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC341_WriteToBuffer(B4_CPUWAD, ulWad & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    dvC341_WriteToBuffer(B4_OSDCT, (ulOsdct & 0xE0) | DVC341_OSDCT_BITBLT, 0 ); //OSD control register  ==> BitBLT
    dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
    OSD_ResetTimeInState();

	do
    {
        ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }while((ulBOSTAT & 0x01) != 0 );
#endif /* SCALER_BOARD_STAGE */

    return ICHIP_OSD_PASS;
}
INT8 dvC341_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index)
{
    UINT32 ulWad;
    UINT32 ulOsdct;
    UINT32 ulBOSTAT;
    UINT8 ucFastWrite = 0 ;

    ulOsdct = dvC341_Read( B4_OSDCT, 0 ) & 0xff;

    //LOG_MSG(db_DV_SCALER_OSD, "Rectangle (%d,%d) size(%d,%d) color=%d\r\n", sDes_position.iX, sDes_position.iY, sDes_Size.iWidth, sDes_Size.iHeight, iColor_Index);
    ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;// write address

    #ifdef OSDCT_FASTEN_ON
    if (sDes_Size.iWidth >= OSDCT_FASTEN_TRANSFER_UNIT &&
        sDes_Size.iWidth % OSDCT_FASTEN_TRANSFER_UNIT == 0 &&
        sDes_position.iX % OSDCT_FASTEN_TRANSFER_UNIT == 0)
    {
        sDes_Size.iWidth = sDes_Size.iWidth / OSDCT_FASTEN_TRANSFER_UNIT;
        ucFastWrite = 1;
    }
    #endif

    dvC341_WriteToBuffer(B4_OSDFILL, iColor_Index & 0xff, 0 );
    dvC341_WriteToBuffer(B4_BBACTHW, (sDes_Size.iWidth - 1) & 0xffff, 0 );
    dvC341_WriteToBuffer(B4_BBACTVW, (sDes_Size.iHeight - 1) & 0xffff, 0 );
    dvC341_WriteToBuffer(B4_CPUWAD, ulWad & 0xffffffff, 0 );

    if(ucFastWrite)
    {
        dvC341_WriteToBuffer(B4_OSDCT, (ulOsdct & 0xE0) | DVC341_OSDCT_FILL | DVC341_OSDCT_FASTEN, 0 );
    }
    else
    {
        dvC341_WriteToBuffer(B4_OSDCT, (ulOsdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
    }

    dvC341_Buffer_Flush();


#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }

    } while ( (ulBOSTAT & 0x01) != 0);
#endif /* SCALER_BOARD_STAGE */

    return ICHIP_OSD_PASS;
}


INT8 dvC341_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset)
{
    INT16 iCount;
    UINT32 ulOSDct;
    UINT32 ulRad;
    UINT32 ulWad;
    UINT32 ulBOSTAT;
    UINT8  ucCharTemp[5] = {0}; //A70LV_Larry_0409

    ulOSDct = dvC341_Read( B4_OSDCT, 0 ) & 0xff;

    dvC341_WriteToBuffer(B4_CBUFAD, 0x0a00, 0); //0xa00 -> 2560deci  //set address to 0xa00(2560) to set first line attribute.
    dvC341_WriteToBuffer(B4_CBUFDT, sString.ucStringMaxHeight & 0xff, 0); //Specify a data of character buffer to be accessed, set first line vertical height.
    dvC341_WriteToBuffer(B4_CBUFAD, 0x0000, 0); // reset character buffer address to 0x0000 prepare for character data writing



    for(iCount=0; iCount<(sString.uiStringLength); iCount++)
    {

#if 1 //A70LV_Larry_0409
        //LOG_MSG(db_HAL_GUI, "%d-th of total=%d char fill\n", iCount, (sString.uiStringLength-iStringOffset));
        dvC341_WriteToBuffer(B4_CBUFDT, (sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) & 0xff, 0);
        dvC341_WriteToBuffer(B4_CBUFDT, ((sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) >> 8) & 0xff, 0);
        dvC341_WriteToBuffer(B4_CBUFDT, iFont_Color & 0xff, 0);
        dvC341_WriteToBuffer(B4_CBUFDT, iBG_Color & 0xff, 0);
        dvC341_WriteToBuffer(B4_CBUFDT, *(sString.pucCharWdith+iCount+iStringOffset) & 0xff, 0); //set each character width.
#else
        ucCharTemp[0] = (sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) & 0xff;
        ucCharTemp[1] = ((sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) >> 8) & 0xff;
        ucCharTemp[2] = iFont_Color & 0xff;
        ucCharTemp[3] = iBG_Color & 0xff;
        ucCharTemp[4] = *(sString.pucCharWdith+iCount+iStringOffset) & 0xff;
        dvC341_BurstWriteAsync_FixedAdd(B4_CBUFDT, 5, ucCharTemp);
#endif

    }



    dvC341_WriteToBuffer(B4_CBUFHST, 0x00, 0); //Character buffer horizontal direction start point
    dvC341_WriteToBuffer(B4_CBUFHW, (sString.uiStringLength - 1) & 0xff, 0); //Character buffer horizontal direction width
    dvC341_WriteToBuffer(B4_CBUFVST, 0x00, 0); //Character buffer vertical direction start point
    dvC341_WriteToBuffer(B4_CBUFVW, 0x00, 0); //Character buffer vertical direction width


    ulRad = POSD_FSAD;  //font_data address
    ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;//ST_OSDAD + wDes_x + wDes_y * PS_OSDMWI * 128;
    dvC341_WriteToBuffer(B4_CPURAD, ulRad & 0xffffffff, 0);
    dvC341_WriteToBuffer(B4_CPUWAD, ulWad & 0xffffffff, 0);
    dvC341_WriteToBuffer(B4_OSDCT, (ulOSDct&0xE0) | DVC341_OSDCT_CHAR, 0); //OSD command select => Character development

    dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
    } while ( (ulBOSTAT & 0x01) != 0 );
#endif /* SCALER_BOARD_STAGE */

    return ICHIP_OSD_PASS;
}

void dvC341_OSD_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor) //A70LV_Larry_0055
{
    UINT32 ulCpuWAd;
    UINT16 uiXStart_Div3_rd;
    UINT16 uiXStart_Div3_rd_rem;
    UINT16 uiXEnd_Div3_rd;
    UINT16 uiXEnd_Div3_rd_rem;
    UINT16 uiXWidth_Div3;
    UINT8  osdct = 0;
    UINT16 uiColor = 0;
    UINT32 ulBOSTAT = 0;


    if((uiXWidth < 1) || (uiYWidth < 1))
    {
        return;
    }

    osdct = dvC341_Read(B4_OSDCT, 0);

    uiXStart_Div3_rd = uiXStart;

    uiXStart_Div3_rd_rem = uiXStart - uiXStart_Div3_rd;

    uiXEnd_Div3_rd = (uiXStart + uiXWidth);

    uiXEnd_Div3_rd_rem = (uiXStart + uiXWidth) - uiXEnd_Div3_rd;

    ulCpuWAd = POSD_SAD + uiXStart_Div3_rd + POSD_MWI * uiYStart;

    uiXWidth_Div3 = uiXEnd_Div3_rd - uiXStart_Div3_rd;


    if(ucForeColor == ucBackColor)
    {
        uiColor = ucBackColor;



        dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
        dvC341_WriteToBuffer(B4_BBACTHW, uiXWidth_Div3 & 0xFFFF, 0);
        dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
        dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
        dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
        dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
        OSD_ResetTimeInState();

        do
        {
            ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

            if(OSD_TimeElapsedInState() >= 1000)
            {
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }
        }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


        dvC341_Write(B4_OSDCT, 0x00, 0);

    }
    else if(uiXWidth_Div3 == 0)
    {

        dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
        dvC341_WriteToBuffer(B4_BBACTHW, 0x0000, 0);
        dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
        dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
        dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
        dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
        OSD_ResetTimeInState();

        do
        {
            ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

           if(OSD_TimeElapsedInState() >= 1000)
            {
                dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }
        }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


        dvC341_Write(B4_OSDCT, 0x00, 0);

    }
    else if(uiXWidth_Div3 > 0)
    {
        if(uiXStart_Div3_rd_rem > 0)
        {


            uiColor = ucForeColor;
            dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC341_WriteToBuffer(B4_BBACTHW, 0x0000, 0);
            dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
            dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE  */


            dvC341_Write(B4_OSDCT, 0x00, 0);


            uiXStart_Div3_rd++;
            ulCpuWAd = POSD_SAD + uiXStart_Div3_rd + POSD_MWI * uiYStart;
            uiXWidth_Div3 = uiXEnd_Div3_rd - uiXStart_Div3_rd;
        }

        if(uiXWidth_Div3 == 0)
        {
            uiColor = ucForeColor;
            dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC341_WriteToBuffer(B4_BBACTHW, 0x0000, 0);
            dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
            dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


            dvC341_Write(B4_OSDCT, 0x00, 0);

        }
        else if(uiXEnd_Div3_rd_rem == 2)
        {

            uiColor = ucForeColor;
            dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC341_WriteToBuffer(B4_BBACTHW, uiXWidth_Div3 & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
            dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


            dvC341_Write(B4_OSDCT, 0x00, 0);

        }
        else
        {

            uiColor = ucForeColor;
            dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC341_WriteToBuffer(B4_BBACTHW, (uiXWidth_Div3 - 1) & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
            dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAG */

            dvC341_WriteToBuffer(B4_OSDCT, 0x00, 0);

            ulCpuWAd = POSD_SAD + uiXEnd_Div3_rd + POSD_MWI * uiYStart;

            uiColor = ucForeColor;
            dvC341_WriteToBuffer(B4_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC341_WriteToBuffer(B4_BBACTHW, 0x0000, 0);
            dvC341_WriteToBuffer(B4_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC341_WriteToBuffer(B4_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC341_WriteToBuffer(B4_OSDCT, (osdct & 0xE0) | DVC341_OSDCT_FILL, 0 );
            dvC341_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC341_Read( B4_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC341_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */

            dvC341_Write(B4_OSDCT, 0x00, 0);

        }
    }
}

INT8 dvC341_OSD_PanelSet(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    OSDePanelTimingId = ePanelTimingId;
    return ICHIP_OSD_PASS;
}

INT8 dvC341_OSD_PanelChange(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0226 modify//A70LV_Doulas_0105
{
    OSDePanelTimingId = ePanelTimingId;

    PS_PANEL_HW = 3840;
    PS_PANEL_VW = 2160;
    PS_PANEL_1CH_HW = 1920;

    switch(OSDePanelTimingId)
    {
        case ePANEL_ID_WUXGA_60HZ:
            PS_PANEL_HST = OSD_H_START_WUGA_60;//56;
            if(dvC341_PIP_PBP_Enable() == TRUE)
            {
                PS_PANEL_VST = OSD_V_START_WUGA_60;// - 2;//20;//32; //A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_WUGA_60;//20;//32;      //A70LV_Doulas_0170 mdoify
            }
            PS_PANEL_HW = 1920;
            PS_PANEL_VW = 1200;
            PS_PANEL_1CH_HW = 960;
            break;

        case ePANEL_ID_1080P_60HZ:
            PS_PANEL_HST = OSD_H_START_1080P_60;//192;
            if(dvC341_PIP_PBP_Enable() == TRUE)
            {
                PS_PANEL_VST = OSD_V_START_1080P_60;// - 2;//41;		//A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_1080P_60;//41;
            }
            PS_PANEL_HW = 1920;
            PS_PANEL_VW = 1080;
            PS_PANEL_1CH_HW = 960;
            break;

        case ePANEL_ID_1080P_120HZ:
            PS_PANEL_HST = OSD_H_START_1080P_120;//24;
            //if(e3D_Type == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            //{
            //    PS_PANEL_VST = OSD_V_START_1080P_120 + 2;//41;
            //}
            //else
            {
                PS_PANEL_VST = OSD_V_START_1080P_120;//41;
            }
            PS_PANEL_HW = 1920;
            PS_PANEL_VW = 1080;
            PS_PANEL_1CH_HW = 960;
            break;

		case ePANEL_ID_WUXGA_120HZ:     //G100_Doulas_0064 Add
            PS_PANEL_HST = OSD_H_START_WUXGA_120;
            //if(e3D_Type == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            //{
            //    PS_PANEL_VST = OSD_V_START_WUXGA_120 +2;
            //}
            //else
            {
                PS_PANEL_VST = OSD_V_START_WUXGA_120;
            }
            PS_PANEL_HW = 1920;
            PS_PANEL_VW = 1200;
            PS_PANEL_1CH_HW = 960;
            break;

		case ePANEL_ID_1080P_240HZ:
		    PS_PANEL_HST = OSD_H_START_1080P_120;
            PS_PANEL_VST = OSD_V_START_1080P_120;
            PS_PANEL_HW = 1920;
            PS_PANEL_VW = 1080;
            PS_PANEL_1CH_HW = 960;
            break;

		case ePANEL_ID_3840x2160_60HZ:
		    PS_PANEL_HST = 384/2 + 1;
            PS_PANEL_VST = 80 + 2;
            PS_PANEL_HW = 3840;
            PS_PANEL_VW = 2160;
            PS_PANEL_1CH_HW = 1920;
            break;

		case ePANEL_ID_3840x2160_60HZ_594MHz:  //need modify
		    PS_PANEL_HST = 384/2 + 1;
            PS_PANEL_VST = 80 + 2;
            PS_PANEL_HW = 3840;
            PS_PANEL_VW = 2160;
            PS_PANEL_1CH_HW = 1920;
            break;

		case ePANEL_ID_3840x2400_60HZ:  //need modify
		    PS_PANEL_HST = 80/2 + 1;
            PS_PANEL_VST = 20;
            PS_PANEL_HW = 3840;
            PS_PANEL_VW = 2400;
            PS_PANEL_1CH_HW = 1920;
            break;

        default:
            dbmsg("!!! PS_PANEL_HST/PS_PANEL_VST undefined, use default");
		    PS_PANEL_HST = 80/2 + 1;
            PS_PANEL_VST = 20;
            PS_PANEL_HW = 3840;
            PS_PANEL_VW = 2400;
            PS_PANEL_1CH_HW = 1920;
            break;
    }

    POSD_HST = (PS_PANEL_HST - 5);
    POSD_VST = (PS_PANEL_VST + 0);

    return ICHIP_OSD_PASS;
}

INT8 dvC341_OSD_MenuTransparencySet(UINT8 ucValue)     //A70LV_Doulas_0122
{
    UINT8 ucMenuTransparency;   //128 OSD no Transparency, 0 no OSD
    ucMenuTransparency = 128 - (ucValue/MENU_TRANSPARENCY_STEP_VALUE)*4;

    dvC341_WriteToBuffer(B9_OSDBLENDC2CH1    , ucMenuTransparency ,0);
    dvC341_WriteToBuffer(B25_OSDBLENDC2CH2   , ucMenuTransparency ,0);
    dvC341_Buffer_Flush();


    LOG_MSG(db_DV_SCALER_OSD, "dvC341_OSD_MenuTransparencySet %d\r\n",ucMenuTransparency);
    return ICHIP_OSD_PASS;
}

INT8 dvC341_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe)     //A70LV_Doulas_0122
{
    UINT32 ulVal;

    ulVal = dvC341_Read(B9_OSDMODE2CH1, 0);

    if(ulVal == 0)
    {
        return ICHIP_OSD_PASS;
    }

    if(ucEnalbe)
    {
        ulVal = ulVal | (BIT9);   //OSD blend enable(Menu Transparency Enable)
    }
    else
    {
        ulVal = ulVal & (~BIT9);   //OSD blend disable(Menu Transparency disable)
    }

    dvC341_WriteToBuffer( B9_OSDMODE2CH1,  ulVal & 0xffff, 0 );
    dvC341_WriteToBuffer( B25_OSDMODE2CH2, ulVal & 0xffff, 0 );
    dvC341_Buffer_Flush();

    LOG_MSG(db_DV_SCALER_OSD, "dvC341_OSD_MenuTransparencyEnableSet %d\r\n",ucEnalbe);
    return ICHIP_OSD_PASS;
}

//only for debug
BOOL dvC341_ShowOsdBitmapData(UINT32 ulStartVertPosition)  //A35G2_Simon_0114
{
    //not ready
    UINT32 ulAddr = POSD_PSAD + (ulStartVertPosition * POSD_MWI)  ;
    dvC341_Write(B9_OSDSAD2CH1, ulAddr, 0);
}

BOOL dvC341_IsMagnifyOSD2X(void)
{
    return sOSDInfo.ucMagnifyOSD2X;
}

