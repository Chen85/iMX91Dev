//#include <string.h>

#include "dvC821.h"
#include "dvC821_OSD.h"
#include "utilDbgMsg.h"
#include "opdCtrlAPI.h"
//	#ifdef CUSTOM_BARCO     //A65_Owen_0002
//	#include "OSD_BARCO/Custom_OSD.h"
//	#else
//	#include "OSD/Custom_OSD.h"
//	#endif


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

int PS_PANEL_HW;
int PS_PANEL_VW;


int PS_PANEL_1CH_HW;
INT16 m_iInhitbit_Color = 0;
static TickType_t ulOSDStartTicks;

ePANEL_ID OSDePanelTimingId;    //A70LV_Doulas_0105

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

void dvC821_OSD_OPDLogOut(const char *pcStr, ...)   //G100_Owen_0080
{
    uOPD_DATA uOPDData = {0};
    va_list args;

    va_start(args, pcStr);

    vsnprintf(uOPDData.sINTERFACE.cString, 256, pcStr, args);
    va_end(args);

    utilOPD_EventSet((UINT8)eOPD_INTERFACE_LOG, &uOPDData);
}

void OSD_DMA_SerialFlash_Set( UINT32 ulSflAd, UINT32 ulDestMemAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
    //dvC821_Write( B29_GIOS, 0x000f, 0 );

    UINT8 ucSFLSTAT = 0;

    LOG_MSG(db_DV_SCALER, "DMA_SerialFlash Start\r\n");


    dvC821_Write(B1_SFLCT, 0,0);
    dvC821_Write(B1_SFLMODE,0x80,0);

    if ((ucRegsel==0x01)||(ucRegsel==0x02))
    {
        dvC821_WriteToBuffer(B1_SFLMODE,0x37,0);
    }
    else
    {
        dvC821_WriteToBuffer(B1_SFLMODE,0x33,0);
    }


    dvC821_WriteToBuffer(B1_SFLCMDWR,0x12,0); //Serial Flash write command setting
    dvC821_WriteToBuffer(B1_SFLCMDRD,0x0C,0); //Serial Flash read command setting
    dvC821_WriteToBuffer(B1_SFLCMDWREN,0x06,0); //Serial Flash WREN command setting
    dvC821_WriteToBuffer(B1_SFLCMDRDSR,0x05,0); //Serial Flash RDSR command setting
    dvC821_WriteToBuffer(B1_SFLSZSEL, 0x01, 0);
    dvC821_WriteToBuffer(B1_SFLAD, ulSflAd&0xffffffff,0);
    dvC821_WriteToBuffer(B1_SFLCNT, ulDmaCnt&0xffffff,0);
    dvC821_WriteToBuffer(B1_SFLREGSEL, ucRegsel & 0x0f,0); //DDR3-SDRAM

    if (ucRegsel==0x00)
    {
        dvC821_WriteToBuffer(B0_OSDCT, 0, 0);
        dvC821_WriteToBuffer(B0_CPUWAD, ulDestMemAd&0xffffffff,0);
    }
    else if ((ucRegsel==0x01)||(ucRegsel==0x02))
    { // PLT0
        dvC821_WriteToBuffer(B3_PLTAD1CH1,ulDestMemAd&0xff,0);
    }
    else if (ucRegsel==0x03)
    { // CBUF
        dvC821_WriteToBuffer(B0_CBUFAD,ulDestMemAd&0xffff,0);
    }

    dvC821_WriteToBuffer(B1_SFLCT, 0x02,0);
    dvC821_Buffer_Flush();


    OSD_ResetTimeInState();

    do
    {
        ucSFLSTAT = dvC821_Read( B1_SFLSTAT, 0 );

        // NOP
		if(OSD_TimeElapsedInState() >= 3000)
        {
            LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) : time out\r\n", __FUNCTION__, __LINE__);
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) : time out\r\n", __FUNCTION__, __LINE__);
            break;
        }

    }while((ucSFLSTAT & 0x01) != 0 );


    dvC821_Write(B1_SFLCT,0x00,0);
    LOG_MSG(db_DV_SCALER_OSD, "OSD_TimeElapsedInState (%d)\r\n", OSD_TimeElapsedInState());

    LOG_MSG(db_DV_SCALER_OSD, "DMA_SerialFlash Done (X%08X)(X%08X)(%02d)(X%08X)\r\n", ulSflAd, ulDestMemAd, ucRegsel, ulDmaCnt);
}

void OSD_DMA_DDR3_WriteTo_Flash( UINT32 ulSrcMemAd, UINT32 ulSflAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
    //dvC821_Write( B29_GIOS, 0x000f, 0 );
    UINT8 ucSFLDTCTL = 0xff;
    UINT8 ucSFLSTAT = 0xff;
    UINT8 ucSFLRDSR = 0xff;

    LOG_MSG(db_DV_SCALER_OSD, "DMA_SerialFlash Start\r\n");

    dvC821_Write(B1_SFLCT,0x00,0);
    dvC821_Write(B1_SFLMODE,0x80,0);

    if ((ucRegsel==0x01)||(ucRegsel==0x02))
    {
        dvC821_Write(B1_SFLMODE,0x37,0);
    }
    else
    {
        dvC821_Write(B1_SFLMODE,0x33,0);
    }


    dvC821_Write(B1_SFLCMDWR,0x12,0); //Serial Flash write command setting
    //dvC821_Write(B1_SFLCMDRD,0x0C,0); //Serial Flash read command setting
    dvC821_Write(B1_SFLCMDWREN,0x06,0); //Serial Flash WREN command setting
    dvC821_Write(B1_SFLCMDRDSR,0x05,0); //Serial Flash RDSR command setting
    dvC821_Write(B1_SFLSZSEL, 0x01, 0);
    dvC821_Write(B1_SFLAD, ulSflAd&0xffffffff,0);
    dvC821_Write(B1_SFLCNT, ulDmaCnt&0xffffff,0);
    dvC821_Write(B1_SFLREGSEL, ucRegsel & 0x0f,0); //DDR3-SDRAM

    if (ucRegsel==0x00)
    {
        dvC821_Write(B0_CPURAD, ulSrcMemAd&0xffffffff,0);
        dvC821_Write(B0_OSDCT, 0, 0);
    }
    else if ((ucRegsel==0x01)||(ucRegsel==0x02))
    { // PLT0
        dvC821_Write(B3_PLTAD1CH1,ulSrcMemAd&0xff,0);
    }
    else if (ucRegsel==0x03)
    { // CBUF
        dvC821_Write(B0_CBUFAD,ulSrcMemAd&0xffff,0);
    }

    dvC821_Write(B1_SFLCT, 0x01,0);
    //dvC821_Buffer_Flush();

    OSD_ResetTimeInState();

    do
    {
        MS_SLEEP(100);
        ucSFLSTAT = dvC821_Read( B1_SFLSTAT, 0 );

        // NOP
        if(OSD_TimeElapsedInState() >= 30000)
        {
            LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) ucSFLSTAT:0x%X time out\r\n", __FUNCTION__, __LINE__, ucSFLSTAT);
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) ucSFLSTAT:0x%X time out\r\n", __FUNCTION__, __LINE__, ucSFLSTAT);
            break;
        }

    }while((ucSFLSTAT & 0x01) != 0 );
    LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) OSD_TimeElapsedInState (%d)\r\n", __FUNCTION__, __LINE__, OSD_TimeElapsedInState());

    //dvC821_Write(B1_SFLCT,0x00,0);

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

    dvC821_Write( B0_MPROTECT, ulMemProtecth & 0xffff, 0 );
}


/******************  osd_acct for C734  *************************
    ucControlData     --> OSD access control
*************************************************************************/
static void OSD_AccessControl( UINT8 ucControlData )
{
    dvC821_Write( B0_BOACCT, ucControlData&0xff, 0 );
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

    dvC821_Inhibit_Color_Set(0, m_iInhitbit_Color);
    dvC821_Paint_Rectangle(sStart, sSize, m_iInhitbit_Color);
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
    UINT8 ucCh_ofst;



    ucCh_ofst = ( POSD_DISP_CH == 0 ) ? 0
        : ( POSD_DISP_CH == 1 ) ? 4
        : ( POSD_DISP_CH == 2 ) ? 8
        : 12;

    ulOSDmode = dvC821_Read( B3_OSDMODE1CH1, ucCh_ofst ) & 0xff;

    if(cBlinkTp_Enable == 1)
    {
        ulOSDmode = ulOSDmode | 0x20;    // blink transparent
    }
    else
    {
        ulOSDmode = ulOSDmode & 0xdf;
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

    dvC821_WriteToBuffer( B3_OSDMODE1CH1, ulOSDmode & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer( B3_BLINKTIME1CH1, iBlinkTime & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer( B3_BLINKCYCL1CH1, iBlinkCycl & 0xff, ucCh_ofst );
    dvC821_Buffer_Flush();
}


INT8 OSD_Palette_Set(INT16 iAmount, UINT8 *pucData )
{
    INT16 iCount;

    if (iAmount != ICHIP_PALETTE_COLOR)
    {
       return PALETTE_AMOUNT_ERROR;
    }

    //Ch1 setting
    dvC821_WriteToBuffer( B3_PLTAD1CH1, 0, 0);
    dvC821_WriteToBuffer( B3_PLTAD1CH1, 0, 13); //A70LV_Larry_0173
    for( iCount=0; iCount<(iAmount*3); iCount++ )
    {
       //LOG_MSG(db_DV_C734_OSD, "(func:%s, line:%d)Ch1(%d)=%d\r\n", __FUNCTION__, __LINE__, iCount, *(pucData+iCount));
       dvC821_WriteToBuffer( B3_PLTDT1CH1, *(pucData+iCount), 0 );
       dvC821_WriteToBuffer( B3_PLTDT1CH1, *(pucData+iCount), 13); //A70LV_Larry_0173
    }
    dvC821_WriteToBuffer( B3_OSDCT1CH1, 0x40, 0 );
    dvC821_WriteToBuffer( B3_OSDCT1CH1, 0x40, 13);  //A70LV_Doulas_0122 Add

    if(dvC821_Read(B3_OSDBLENDC1CH1,0) == 0)     //A70LV_Doulas_0122 check Menu Transparency init
    {
        dvC821_WriteToBuffer( B3_OSDBLENDC1CH1   , 0x80 ,0);    //OSD blend coefficient(Menu Transparency value)
        dvC821_WriteToBuffer( B3_OSDBLENDC1CH1   , 0x80 ,13);   //OSD blend coefficient(Menu Transparency value)
    }
    dvC821_Buffer_Flush();

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

    osdct = dvC821_Read(B0_OSDCT, 0);

    //ulRad = POSD_PSAD + ( /*POSD_MWI*/0x0a * 128 * iSrc_y + iSrc_x );// bitblt source address  //0x0a* 128(source total bitmap width) should read from JSon file define
    //ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;  // bitblt destination address  //POSD_MWI = output OSD plane width

    //dvC734_Write( B0_BBWMWI, 0x08, 0 );  //8x128 = 1024
    //dvC734_Write( B0_BBRMWI, POSD_REG_MWI, 0 );  //0x20 x128 = 4096

    dvC821_WriteToBuffer(B0_BBACTHW, (ibbHor-1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC821_WriteToBuffer(B0_BBACTVW, (ibbVer-1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC821_WriteToBuffer(B0_CPURAD, source_adrs & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC821_WriteToBuffer(B0_CPUWAD, dest_adrs & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    dvC821_WriteToBuffer( B0_OSDCT, (osdct & 0xf0) | 0x02, 0 ); //OSD control register  ==> BitBLT

    dvC821_Buffer_Flush();
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
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

    switch(OSDePanelTimingId)   //A70LV_Doulas_0105 Modify
    {
        case ePANEL_ID_WUXGA_60HZ:
		case ePANEL_ID_WUXGA_120HZ: 	//G100_Doulas_0064
            uibbHor = 1920;
            uibbVer = 1200;
            break;

        case ePANEL_ID_1080P_60HZ:
            uibbHor = 1920;
            uibbVer = 1080;
            break;

        case ePANEL_ID_1080P_120HZ:
            uibbHor = 1920;
            uibbVer = 1080;
            break;
        default:
            uibbHor = 1920;
            uibbVer = 1080;
            break;
    }




    osdct = dvC821_Read(B0_OSDCT, 0);

    dvC821_WriteToBuffer(B0_BBACTHW, (uibbHor-1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC821_WriteToBuffer(B0_BBACTVW, (uibbVer-1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC821_WriteToBuffer(B0_CPURAD, source_adrs & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC821_WriteToBuffer(B0_CPUWAD, dest_adrs & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    dvC821_WriteToBuffer( B0_OSDCT, (osdct & 0xf0) | 0x02, 0 ); //OSD control register  ==> BitBLT

    dvC821_Buffer_Flush();
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            ASSERT_ALWAYS();
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            return;
        }
    }while((ulBOSTAT & 0x01) != 0 );
}

// ==============================================================================
// FUNCTION NAME: dvC821_SerialFlash_CheckSum
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
UINT32 dvC821_SerialFlash_CheckSum(UINT32 ulSflAd, UINT32 ulDataSize)
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



    dvC821_Write(B1_SFLCT,0x00,0);
    dvC821_Write(B1_SFLMODE,0x80,0); //Serial Flash mode setting //Reset
    dvC821_Write(B1_SFLMODE,0x33,0); //A70LV_Larry_0102

    dvC821_Write(B1_SFLCMDRD, 0x0C, 0);
    dvC821_Write(B1_SFLSZSEL,0x01,0); //Specify the size of the serial Flash to be connected. //256 Mbit
    dvC821_Write(B1_SFLAD,(ulSflAd&0xFFFFFFFF),0); //Serial Flash address
    dvC821_Write(B1_SFLDTCTL, 0x01, 0);


    LOG_MSG(db_DV_SCALER_OSD, "Read Add 0x%08x-0x%08x\n", ulSflAd, ulSflAd + ulDataSize);

    for(ulCount = 0; ulCount < ulBLOCK; ulCount++) //A70LV_Larry_0054
    {
        ucSFLDTCTL = dvC821_Read(B1_SFLDTCTL, 0);
        OSD_ResetTimeInState();

        while((ucSFLDTCTL & 0x80))
        {
            ucSFLDTCTL = dvC821_Read(B1_SFLDTCTL, 0);

            if(OSD_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                return ulTotoalCheckSum;
            }
        }

        dvC821_BurstRead_FixedAdd(B1_SFLDT, 0x100, ucaData);

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


    dvC821_Write(B1_SFLAD,0,0);

    dvC821_Write(B1_SFLDTCTL, 0x00, 0);

    LOG_MSG(db_DV_SCALER_OSD, "Total CheckSum 0x%08x\n", ulTotoalCheckSum);

    return ulTotoalCheckSum;
}

eRESULT dvC821_RAM_Read(UINT32 ulRAMAd, UINT8 *pData, UINT32 ulDataSize) //A35G2_CDS_Larry_0050
{
    UINT32  ulCount = 0;
    UINT8 *pBuffer = malloc(ulDataSize + 0x1000);
    UINT8 ucVAl = 0;

    if(pBuffer != NULL)
    {
        memset(pBuffer, 0xff, ulDataSize);

        dvC821_Write(B0_CPURAD, ulRAMAd, 0);
        dvC821_Write(B0_CPUDTCTL, 0x01 ,0);

        OSD_ResetTimeInState();

        ucVAl = 0x08;
        while((ucVAl & 0x08) != 0)
        {
            ucVAl = (UINT8)dvC821_Read(B0_CPUDTCTL,0);

            if(OSD_TimeElapsedInState() >= 1000)
            {
                ASSERT_ALWAYS();
                free(pBuffer);
                return rcERROR;
            }
        }

        for(ulCount = 0; ulCount < ulDataSize; (ulCount = ulCount + 0x1000))
        {
            dvC821_BurstRead_FixedAdd(B0_CPUDT, 0x1000, &pBuffer[ulCount]);
        }

        if(pData != NULL)
        {
            memcpy(pData, pBuffer, ulDataSize);
        }

        free(pBuffer);
    }

    return rcSUCCESS;
}

eRESULT dvC821_RAM_Write(UINT32 ulRAMAd, UINT8 *pData, UINT32 ulDataSize) //A35G2_CDS_Larry_0050
{
    UINT32  ulCount = 0;

    dvC821_Write(B0_CPUWAD, ulRAMAd, 0);

    for(ulCount = 0; ulCount < ulDataSize; (ulCount = ulCount + 0x1000))
    {
        dvC821_BurstWrite_FixedAdd(B0_CPUDT, 0x1000, &pData[ulCount]);
    }
    //在下一次CPUWAD，確保CPUDT資料有被更新
    ulRAMAd = 0;
    dvC821_Write(B0_CPUWAD, ulRAMAd, 0);

    return rcSUCCESS;
}


// ==============================================================================
// FUNCTION NAME: dvC821_SerialFlash_Erase_All
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
void dvC821_SerialFlash_Erase_All(void)
{
    unsigned int wsad;
    int RDTCHK;

    wsad = 0x00000000;
    //dvC734_Write(B29_GIOS, 0x000f, 0);
    LOG_MSG(db_DV_SCALER, "FlasEraseAll_Start\n");



    dvC821_Write(B1_SFLSZSEL, 0x01, 0);
    dvC821_Write(B1_SFLCT, 0x00, 0);
    dvC821_Write(B1_SFLMODE, 0x80, 0);
    dvC821_Write(B1_SFLMODE, 0x23, 0); //A35G2_CDS_Larry_0032 0x33 -> 0x23
    dvC821_Write(B1_SFLCT, 0x00, 0);
    dvC821_Write(B1_SFLCMDWR, 0x12, 0);  //Set the page write command to the serial Flash connected
    dvC821_Write(B1_SFLCMDRD, 0x0C, 0);  //Set a read or fast read command to be executed for the serial Flash connected
    dvC821_Write(B1_SFLCMDER, 0xC7, 0);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
    dvC821_Write(B1_SFLCMDWREN, 0x06, 0);  //Set the value of WREN command of the serial Flash connected
    dvC821_Write(B1_SFLCMDRDSR, 0x05, 0);  //Set the value of RDSR command of the serial Flash connected
    dvC821_Write(B1_SFLAD, wsad, 0);  //Specify the serial Flash address. After accessing, auto increment starts
    dvC821_Write(B1_SFLCT, 0x05, 0);



    OSD_ResetTimeInState();
    do
    {
        RDTCHK = dvC821_Read(B1_SFLSTAT, 0);

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
        LOG_MSG(db_DV_SCALER, ".");

    } while ((RDTCHK & 0x02)==0x02);
    LOG_MSG(db_DV_SCALER, "\nWait B1_SFLSTAT Done\n");

    OSD_ResetTimeInState();
    do
    {
        RDTCHK = dvC821_Read(B1_SFLRDSR, 0);

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }while ((RDTCHK & 0x03)!=0x00);
    LOG_MSG(db_DV_SCALER, "\nWait B1_SFLRDSR Done\n");

    LOG_MSG(db_DV_SCALER, "FlasEraseAll_Done\n");

	return;
}

// ==============================================================================
// FUNCTION NAME: dvC821_SerialFlash_Erase_Sector
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
void dvC821_SerialFlash_Erase_Sector(UINT32 ulSflAd, UINT32 ulDataSize)
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

    for(ulEraseAddrStart = ulSflAd; ulEraseAddrStart < ulEraseAddrEnd; ulEraseAddrStart += 0x00010000) //64k
    {
        LOG_MSG(db_DV_SCALER_OSD, "Erase Sector 0x%08x - 0x%08x\n", ulEraseAddrStart, ulEraseAddrStart + 0x0000FFFF);

         //A70LV_Larry_0052

        dvC821_Write(B1_SFLSZSEL, 0x01, 0);
        dvC821_Write(B1_SFLCT, 0x00, 0);
        dvC821_Write(B1_SFLMODE, 0x80, 0);
        dvC821_Write(B1_SFLMODE, 0x23, 0); //A35G2_CDS_Larry_0032 0x33 -> 0x23
        dvC821_Write(B1_SFLCT, 0x00, 0);
        dvC821_Write(B1_SFLCMDWR, 0x12, 0);  //Set the page write command to the serial Flash connected
        dvC821_Write(B1_SFLCMDRD, 0x0C, 0);  //Set a read or fast read command to be executed for the serial Flash connected
        dvC821_Write(B1_SFLCMDER, 0xDC, 0);  //Set a selector erase or bulk (chip) erase command to be executed for the serial Flash connected
        dvC821_Write(B1_SFLCMDWREN, 0x06, 0);  //Set the value of WREN command of the serial Flash connected
        dvC821_Write(B1_SFLCMDRDSR, 0x05, 0);  //Set the value of RDSR command of the serial Flash connected
        dvC821_Write(B1_SFLAD, ulEraseAddrStart, 0);  //Specify the serial Flash address. After accessing, auto increment starts
        dvC821_Write(B1_SFLCT, 0x04, 0);

        OSD_ResetTimeInState();

        do
        {
            ucRDTCHK = dvC821_Read(B1_SFLSTAT, 0); //A70LV_Larry_0054
            MS_SLEEP(50);
            if(OSD_TimeElapsedInState() >= 2000)
            {
                LOG_MSG(db_DV_SCALER_OSD, "Erase Time Out 1\n");
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }

        } while((ucRDTCHK & 0x02)==0x02);

        OSD_ResetTimeInState();
        do
        {
            ucRDTCHK = dvC821_Read(B1_SFLRDSR, 0); //A70LV_Larry_0054
            MS_SLEEP(50);

            if(ucRDTCHK & 0x02)
            {
                dvC821_Write(B1_SFLCMDER, 0x04, 0);
                dvC821_Write(B1_SFLCT, 0x05, 0);
            }

            if(OSD_TimeElapsedInState() >= 2000)
            {
                LOG_MSG(db_DV_SCALER_OSD, "Erase Time Out 2\n");
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }

        } while((ucRDTCHK & 0x03) !=0x00 );

         //A70LV_Larry_0052
    }

    LOG_MSG(db_DV_SCALER_OSD, "Sector Erase End\n");

    return;
}


// ==============================================================================
// FUNCTION NAME: dvC821_SerialFlash_Write
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
void dvC821_SerialFlash_Write(UINT32 ulSflAd, UINT8 *pucBuffer, UINT32 ulDataSize)
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
    UINT32  ulReadFlashAdd = (UINT32)pucBuffer;
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
#if 1
    dvC821_Write(B1_SFLCT, 0x00,0);
    dvC821_Write(B1_SFLMODE, 0x80,0); //Serial Flash mode setting //Reset
    dvC821_Write(B1_SFLMODE, 0x33,0); //A70LV_Larry_0102

    dvC821_Write(B1_SFLCMDWR, 0x12, 0); //Serial Flash write command setting
    dvC821_Write(B1_SFLCMDWREN, 0x06, 0); //Serial Flash WREN command setting
    dvC821_Write(B1_SFLCMDRDSR, 0x05, 0); //Serial Flash RDSR command setting
    dvC821_Write(B1_SFLSZSEL, 0x01, 0); //Specify the size of the serial Flash to be connected. //256 Mbit
    dvC821_Write(B1_SFLAD, (ulWriteADDR & 0xffffffff), 0); //Serial Flash address

    LOG_MSG(db_DV_SCALER_OSD, "Flash Copy start\n");

    for(ulCount = 0; ulCount < ulBLOCK; ulCount++)
    {
         //A70LV_Larry_0052
		OSD_ResetTimeInState();	//ZU860_Clare_0118, add

        do
		{
			ucSFLDTCTL = dvC821_Read(B1_SFLDTCTL,0);
			if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
			{
				LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 1\n");
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
				break;
			}
		} while ((ucSFLDTCTL & 0x20)==0x00);
        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
        do
		{
			ucSFLSTAT = dvC821_Read(B1_SFLSTAT, 0);
			if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
			{
				LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 2\n");
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
				break;
			}
		}  while ((ucSFLSTAT & 0x02)==0x02);

        for (uiCount = 0; uiCount < 0x100; uiCount++)
        {
            aucData[uiCount] = *pucData++;//acData[cCount];
            ulCheckSum += (aucData[uiCount] * (ulSectorSize + 1)); //A70LV_Larry_0158
            ulTotoalCheckSum += (aucData[uiCount] * (ulSectorSize + 1)); //A70LV_Larry_0158
            ulSectorSize++; //A70LV_Larry_0158
//            dvC821_Write_NoSemaphore(B1_SFLDT, (ucData & 0xFF), 0); //A70LV_Larry_0052

            if((ulADDRCount&0xFFFF) == 0xFFFF)
            {
                //MS_SLEEP(10);

                LOG_MSG(db_DV_SCALER_OSD, "Add(0x%08x) 0x%08x-0x%08x CheckSum 0x%08x\n", ulReadFlashAdd, ulADDRCount - 0x0FFF, ulADDRCount, ulCheckSum);
                ulCheckSum = 0;
            }

            ulADDRCount = ulADDRCount + 1;
//            if((ulADDRCount & 0xff)==0)
//            {
//                do{ ucSFLDTCTL = dvC821_Read_NoSemaphore(B1_SFLDTCTL,0);} while ((ucSFLDTCTL & 0x20)==0x00);
//            }

        }
        dvC821_BurstWrite_FixedAdd(B1_SFLDT, 0x100, aucData);

        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
        do
        {
            ucSFLDTCTL = dvC821_Read(B1_SFLDTCTL,0);
            if(OSD_TimeElapsedInState() >= 1000)    //ZU860_Clare_0118, add
            {
                LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 3\n");
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }
        } while ((ucSFLDTCTL & 0x20)==0x00);

        if((ulADDRCount & 0xff)!=0)
        {
            dvC821_Write(B1_SFLAD, ((ulWriteADDR + ulADDRCount) & 0xffffffff), 0);
            OSD_ResetTimeInState(); //ZU860_Clare_0118, add
            do
			{
				ucSFLDTCTL = dvC821_Read(B1_SFLDTCTL,0);
				if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
				{
					LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 4\n");
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
					break;
				}
			} while ((ucSFLDTCTL & 0x20)==0x00);
	        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
            do
			{
				ucSFLSTAT = dvC821_Read(B1_SFLSTAT, 0);
				if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
				{
					LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 5\n");
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
					break;
				}
			}  while ((ucSFLSTAT & 0x02)==0x02);
	        OSD_ResetTimeInState();	//ZU860_Clare_0118, add
            do
			{
				ucRDTCHK = dvC821_Read(B1_SFLRDSR, 0);
				if(OSD_TimeElapsedInState() >= 1000)	//ZU860_Clare_0118, add
				{
					LOG_MSG(db_DV_SCALER_OSD, "Write Time Out 6\n");
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
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
    if(ulTotoalCheckSum != dvC821_SerialFlash_CheckSum(ulSflAd, ulBLOCK*0x100))
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
void dvC821_Inhibit_Color_Set( INT8 cEnable, INT16 iInhibit_Color )
{
    UINT8 ucOsdct;

    ucOsdct = (cEnable ==1) ? 0x20 : 0x00;

    m_iInhitbit_Color = iInhibit_Color;

    dvC821_WriteToBuffer( B0_OSDCT, ucOsdct & 0xff, 0 );
    dvC821_WriteToBuffer( B0_BOWINH, m_iInhitbit_Color & 0xff, 0 );
    dvC821_Buffer_Flush();
}


void dvC821_OSD_Off(void)
{
    UINT32 ulOSDmode, ulOSDmode2;//, ulOSDmode3, ulOSDmode4;
    UINT32 ulRtct;

    ulOSDmode = dvC821_Read( B3_OSDMODE1CH1, 0) & 0xfd;
    ulOSDmode2= dvC821_Read( B3_OSDMODE1CH1, 13) & 0xfd;

    ulRtct = dvC821_Read( B0_RTCT, 0 ) & 0xffffffff;

    dvC821_WriteToBuffer( B0_RTCT, RTCT_POVSSTOP, 0 );

    dvC821_WriteToBuffer( B3_OSDMODE1CH1, ulOSDmode & 0xff, 0);
    dvC821_WriteToBuffer( B3_OSDMODE1CH1, ulOSDmode2 & 0xff, 13);

    dvC821_WriteToBuffer( B0_RTCT, RTCT_NORMAL, 0 );
    dvC821_Buffer_Flush();

    dvC821_WaitIntct((eMCT)0, C821_INT_PO1VS );  //wait_povs
    dvC821_WaitIntct((eMCT)1, C821_INT_PO2VS );  //wait_povs

    dvC821_Write( B0_RTCT, ulRtct, 0 );
}

void dvC821_OSD_On( START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color)
{
	UINT32 ulOSDmode1;//, ulOSDmode2, ulOSDmode3, ulOSDmode4;
	UINT32 ulRddt1_read;//, ulRddt2_read, ulRddt3_read, ulRddt4_read;
	UINT32 ulOSDmode2;      //A70LV_Doulas_0098
	UINT32 ulRddt2_read;    //A70LV_Doulas_0098
	//UINT32 ulRtct;
    UINT8 ucCh1_ofst;
    UINT8 ucCh2_ofst;
    //UINT8 ucCh3_ofst;
    //UINT8 ucCh4_ofst;

    START_POINT sCH2_start;
    //START_POINT sCH3_start;
    //START_POINT sCH4_start;
    RECT_SIZE sQuarter_Size;
    UINT8 ucCH_enable;  //Bit0:CH1 enable, Bit1:CH2 enable, Bit2:CH2 enable, Bit3:CH4 enable
    ePANEL_ID eOutputPanel = dvC821_PanelGet();     //A70LV_Doulas_0098

    ucCh1_ofst = 0;
    ucCh2_ofst = 13;    //A70LV_Doulas_0098
    //ucCh3_ofst = 8;
    //ucCh4_ofst = 12;

    //sCH2_start.iX = 960;
    //sCH2_start.iY = 0;
    //sCH3_start.iX = 1920;
    //sCH3_start.iY = 0;
    //sCH4_start.iX = 2880;
   // sCH4_start.iY = 0;

    switch(OSDePanelTimingId)   //A70LV_Doulas_0105 Modify
    {
        case ePANEL_ID_WUXGA_60HZ:
            sQuarter_Size.iWidth = 1920;
            sQuarter_Size.iHeight = 1200;
            break;

        case ePANEL_ID_1080P_60HZ:
            sQuarter_Size.iWidth = 1920;
            sQuarter_Size.iHeight = 1080;
            break;

        case ePANEL_ID_1080P_120HZ:
            sQuarter_Size.iWidth = 960; //A70LV_Larry_0173
            sQuarter_Size.iHeight = 1080;
            break;

		case ePANEL_ID_WUXGA_120HZ: 	//G100_Doulas_0064
			sQuarter_Size.iWidth = 960;
            sQuarter_Size.iHeight = 1200;
            break;

        default:
            sQuarter_Size.iWidth = 1920;
            sQuarter_Size.iHeight = 1080;
            break;
    }


    ucCH_enable = 0;

    ucCH_enable = ucCH_enable | 0x01;  //enable ch1 only

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

    ulRddt1_read = dvC821_Read( B3_OSDMODE1CH1, ucCh1_ofst ) & 0xff;
    ulRddt2_read = dvC821_Read( B3_OSDMODE1CH1, ucCh2_ofst ) & 0xff;    //A70LV_Doulas_0098

	//ulRtct = dvC821_Read( B0_RTCT, 0 ) & 0xffffffff;

    if( cTrap_Enable == 1 )
    {
        ulOSDmode1 = ulRddt1_read | 0x07; //A70LV_Larry_0055 //for warping 0x03->0x07
        ulOSDmode2 = ulRddt2_read | 0x07;       //A70LV_Doulas_0098
    }
    else
    {
        ulOSDmode1 = (ulRddt1_read & 0xfc) | 0x02;
        ulOSDmode2 = (ulRddt2_read & 0xfc) | 0x02;  //A70LV_Doulas_0098
    }

	dvC821_WriteToBuffer( B0_RTCT, RTCT_POVSSTOP, 0 );



    if (ucCH_enable & 0x01)  // Ch1 setting
    {
        INT16 iDes_width;
        iDes_width = sDes_Size.iWidth - (sDes_position.iX + sDes_Size.iWidth - sCH2_start.iX);    //A70LV_Doulas_0098 modify
        if (iDes_width < 0)
            iDes_width = sDes_Size.iWidth;
        else if (iDes_width > sQuarter_Size.iWidth)
            iDes_width = sQuarter_Size.iWidth;

        //LOG_MSG(db_DV_SCALER_OSD, "OSD ON(%d,%d) size(%d,%d)\r\n", sDes_position.iX, sDes_position.iY, sDes_Size.iWidth, sDes_Size.iHeight);



    	dvC821_WriteToBuffer( B3_OSDSAD1CH1, POSD_SAD/*POSD_SAD_CH1*/ & 0xffffffff, ucCh1_ofst );  //Always set start address to POSD_SAD, because OSD memory plane always start paint fucntion at (0,0)
    	dvC821_WriteToBuffer( B3_OSDACTHST1CH1, (POSD_HST+sDes_position.iX) & 0xffff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_OSDACTHW1CH1, ((iDes_width)) & 0xffff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_OSDACTVST1CH1, (POSD_VST+sDes_position.iY) & 0xffff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_OSDACTVW1CH1, (sDes_Size.iHeight) & 0xffff, ucCh1_ofst );

        dvC821_WriteToBuffer( B3_OSDMWI1CH1, POSD_REG_MWI & 0xff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_OSDMODE1CH1, ulOSDmode1 & 0xff, ucCh1_ofst );
        //dvC821_Write( B3_OSDMODE1CH1, 0x22, ucCh1_ofst );

    	dvC821_WriteToBuffer( B3_BOTRANS01CH1, sTrap_Color.ucT_Color1 & 0xff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_BOTRANS11CH1, sTrap_Color.ucT_Color2 & 0xff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_BOTRANS21CH1, sTrap_Color.ucT_Color3 & 0xff, ucCh1_ofst );
    	dvC821_WriteToBuffer( B3_BOTRANS31CH1, sTrap_Color.ucT_Color4 & 0xff, ucCh1_ofst );


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
            dvC821_WriteToBuffer( B3_OSDSAD1CH1, POSD_SAD & 0xffffffff, ucCh2_ofst );
        }
        else
        {
			dvC821_WriteToBuffer(B3_OSDSAD1CH1, (POSD_SAD + (sCH2_start.iX - sDes_position.iX))/*POSD_SAD_CH4*/ & 0xffffffff, ucCh2_ofst);
        }


    	dvC821_WriteToBuffer( B3_OSDACTHST1CH1, (POSD_HST+iDes_x) & 0xffff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_OSDACTHW1CH1, ((iDes_width)) & 0xffff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_OSDACTVST1CH1, (POSD_VST+sDes_position.iY) & 0xffff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_OSDACTVW1CH1, (sDes_Size.iHeight) & 0xffff, ucCh2_ofst );

        dvC821_WriteToBuffer( B3_OSDMWI1CH1, POSD_REG_MWI & 0xff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_OSDMODE1CH1, ulOSDmode2 & 0xff, ucCh2_ofst );
        //dvC821_Write( B3_OSDMODE1CH1, 0x22, ucCh1_ofst );

    	dvC821_WriteToBuffer( B3_BOTRANS01CH1, sTrap_Color.ucT_Color1 & 0xff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_BOTRANS11CH1, sTrap_Color.ucT_Color2 & 0xff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_BOTRANS21CH1, sTrap_Color.ucT_Color3 & 0xff, ucCh2_ofst );
    	dvC821_WriteToBuffer( B3_BOTRANS31CH1, sTrap_Color.ucT_Color4 & 0xff, ucCh2_ofst );


    }

	//dvC821_Write( B0_RTCT, B3_OSDMODE1CH1, 0 ); //A70LV_Larry_0173 ??

	dvC821_WriteToBuffer( B0_RTCT, RTCT_NORMAL, 0 );
    dvC821_Buffer_Flush();

    if (ucCH_enable & 0x01)  // Ch1 setting
	    dvC821_WaitIntct(eMCT_CH1, C821_INT_PO1VS);  //OSD CH1 wait_povs

    if (ucCH_enable & 0x02)  // Ch2 setting //A70LV_Doulas_0098
	    dvC821_WaitIntct(eMCT_CH2, C821_INT_PO2VS);  //OSD CH1 wait_povs

    OSD_Swap_Memplane();  //Swap memory plane automatically after OSD drawing every time
}

/******************  osd_bias for C734  *************************/
/****************************************************************
    rbias     ->  Red   bias (-128 to 127)
    gbias     ->  Green bias (-128 to 127)
    bbias     ->  Blue  bias (-128 to 127)
 ****************************************************************/
void dvC821_OSD_Bias_Set(OSD_BIAS_COLOR sBiasSetting)
{
    UINT8 ucCh_ofst;

    ucCh_ofst = ( POSD_DISP_CH == 0 ) ? 0
        : ( POSD_DISP_CH == 1 ) ? 4
        : ( POSD_DISP_CH == 2 ) ? 8
        : 12;

    dvC821_WriteToBuffer(B3_OSDBIASR01CH1, sBiasSetting.sBias_Color1.cBias_R & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASG01CH1, sBiasSetting.sBias_Color1.cBias_G & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASB01CH1, sBiasSetting.sBias_Color1.cBias_B & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASR11CH1, sBiasSetting.sBias_Color2.cBias_R & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASG11CH1, sBiasSetting.sBias_Color2.cBias_G & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASB11CH1, sBiasSetting.sBias_Color2.cBias_B & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASR21CH1, sBiasSetting.sBias_Color3.cBias_R & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASG21CH1, sBiasSetting.sBias_Color3.cBias_G & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASB21CH1, sBiasSetting.sBias_Color3.cBias_B & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASR31CH1, sBiasSetting.sBias_Color4.cBias_R & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASG31CH1, sBiasSetting.sBias_Color4.cBias_G & 0xff, ucCh_ofst );
    dvC821_WriteToBuffer(B3_OSDBIASB31CH1, sBiasSetting.sBias_Color4.cBias_B & 0xff, ucCh_ofst );
    dvC821_Buffer_Flush();

}


/******************  osd_gain for C734  *************************
//  en        ->  gain enable
    rgain     ->  gain for red   (0 to 3.99)
    ggain     ->  gain for green (0 to 3.99)
    bgain     ->  gaub fir blue  (0 to 3.99)
*************************************************************************/
void dvC821_OSD_Gain_Set( INT8 cEnable, OSD_GAIN_COLOR sGainSetting)
{
    INT16 iRed0, iGreen0, iBlue0;
    INT16 iRed1, iGreen1, iBlue1;
    INT16 iRed2, iGreen2, iBlue2;
    INT16 iRed3, iGreen3, iBlue3;
    UINT32 ulOSDmode;
    UINT8 ucCh_ofst;

    ucCh_ofst = ( POSD_DISP_CH == 0 ) ? 0
        : ( POSD_DISP_CH == 1 ) ? 4
        : ( POSD_DISP_CH == 2 ) ? 8
        : 12;

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

    ulOSDmode = dvC821_Read( B3_OSDMODE1CH1, ucCh_ofst ) & 0xff;

    if( cEnable == 1 )
    {
        ulOSDmode = ulOSDmode | 0x10;
    }
    else
    {
        ulOSDmode = ulOSDmode & 0xef;
    }

    dvC821_WriteToBuffer( B3_OSDGAINR01CH1, iRed0, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAING01CH1, iGreen0, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINB01CH1, iBlue0, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINR11CH1, iRed1, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAING11CH1, iGreen1, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINB11CH1, iBlue1, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINR21CH1, iRed2, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAING21CH1, iGreen2, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINB21CH1, iBlue2, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINR31CH1, iRed3, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAING31CH1, iGreen3, ucCh_ofst );
    dvC821_WriteToBuffer( B3_OSDGAINB31CH1, iBlue3, ucCh_ofst );

    dvC821_WriteToBuffer( B3_OSDMODE1CH1, ulOSDmode & 0xff, ucCh_ofst );
    dvC821_Buffer_Flush();
}



INT8 dvC821_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color)
{
    INT8 cRet = ICHIP_OSD_PASS;
    OSD_BIAS_COLOR sBias_Color;
    LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);


    POSD_REG_MWI = DEF_OSDMWI_2K;
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
    POSD_MPROC = DEF_MPROTECT_2K & 0x0fff;

    POSD_SFL_PSAD = DEF_SFL_OSD_PSAD_2K;
    POSD_SFL_FSAD = DEF_SFL_OSD_FSAD_2K;

    POSD_SAD = POSD_SAD0;
    POSD_MWI = POSD_REG_MWI * 128; //0F*128

    switch(OSDePanelTimingId)   //A70LV_Doulas_0226 modify//A70LV_Doulas_0105
    {
        case ePANEL_ID_WUXGA_60HZ:
            PS_PANEL_HST = OSD_H_START_WUGA_60;//56;
            if(dvC821_PIP_PBP_Enable() == TRUE)
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
            if(dvC821_PIP_PBP_Enable() == TRUE)
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

        default:
            break;
    }

    POSD_HST = (PS_PANEL_HST - 5);// OSDACTHST offset = 5  //A70LV_Doulas_0105 //A70LV_Larry_0054 5->7
    POSD_VST = (PS_PANEL_VST + 0);// OSDACTVST offset = -1 //A70LV_Doulas_0105

    POSD_DISP_CH = 0;  //( PS_OCH_SWP == 1 ) ? 2 : 0;// for output channel swap  //PS_OCH_SWP(m_sChannelInfo[eCH].bOutputChSwap)

    cRet = OSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);

    dvC821_WriteToBuffer( B0_OSDCT, 0x00, 0 );
    dvC821_WriteToBuffer( B3_OSDMWI1CH1, POSD_REG_MWI,0); //0x0F
    dvC821_WriteToBuffer( B0_BBWMWI, POSD_REG_MWI, 0 );
    dvC821_WriteToBuffer( B0_BBRMWI, DVC821_OSD_BITMAP_SKETCHPAD_WIDTH/*POSD_REG_MWI*/, 0 );
    dvC821_Buffer_Flush();

    OSD_DMA_SerialFlash_Set( POSD_SFL_PSAD, POSD_PSAD, 0, DV_BITMAP_RAW_DATA_SIZE );// OSD Part  //modify dma size to fit new bitmap data
    OSD_DMA_SerialFlash_Set( POSD_SFL_FSAD, POSD_FSAD, 0, DV_TEXT_RAW_DATA_SIZE );// OSD Font  //modify dma size to fit new font data
    OSD_Memory_Protect( 1, POSD_MPROC );// v12 //A70LV_Larry_0415
    dvC821_Inhibit_Color_Set( 1, ucInhibit_Color );
    OSD_AccessControl( 0x33 );// v12
    //dvC821_OSD_Off();

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
    dvC821_OSD_Bias_Set(sBias_Color);

    OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    OSD_Swap_Memplane();  //set OSD memory plane to first and reset that

    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_Init reg init complete\r\n");

    return cRet;
}

INT8 dvC821_OSD_Reload(void)
{
    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_Reload Start\r\n");
    OSD_Memory_Protect( 0, POSD_MPROC );// v12 //A70LV_Larry_0415

    OSD_DMA_SerialFlash_Set( POSD_SFL_PSAD, POSD_PSAD, 0, DV_BITMAP_RAW_DATA_SIZE );// OSD Part  //modify dma size to fit new bitmap data
    OSD_DMA_SerialFlash_Set( POSD_SFL_FSAD, POSD_FSAD, 0, DV_TEXT_RAW_DATA_SIZE );// OSD Font  //modify dma size to fit new font data

    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_Reload Done\r\n");

    OSD_Memory_Protect( 1, POSD_MPROC );// v12 //A70LV_Larry_0415

    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_Memory_Protect(UINT8 ucEnable) //A35G2_CDS_Larry_0050
{
    OSD_Memory_Protect( ucEnable, (DEF_MPROTECT_2K & 0x0fff));// v12

    return ICHIP_OSD_PASS;
}

#if (LOGO_REPLACE == 1)
INT8 dvC821_2ND_LOGO_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color, UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0068 //A65_OPTOMA_Julie_0067
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

    dvC821_Inhibit_Color_Set( 1, ucInhibit_Color );
    OSD_AccessControl( 0x33 );// v12

    OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    OSD_Swap_Memplane();  //set OSD memory plane to first and reset that

    LOG_MSG(db_DV_SCALER_OSD, "2ND logo index[%d], dvC821_2ND_LOGO_OSD_Init complete\r\n", ucIndex);

    return (status == rcSUCCESS);
}

INT8 dvC821_OSD_2ND_LOGO_Reload(UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
{
    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_2ND_LOGO_Reload Start, 2ND logo index[%d], \r\n", ucIndex);
    OSD_Memory_Protect( 0, POSD_MPROC );// v12 //A70LV_Larry_0415

	if(ucIndex == eCM_USER_LOGO_SERVICE_2D) //A65_OPTOMA_Julie_0076
	{
		OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_SERVICE_2ND_LOGO, POSD_PSAD, 0, OSD_2ND_LOGO_BITMAP_SIZE); //modify dma size to fit new bitmap data
	}
	else
	{
	    OSD_DMA_SerialFlash_Set( DEF_SFL_OSD_2ND_LOGO, POSD_PSAD, 0, OSD_2ND_LOGO_BITMAP_SIZE); //modify dma size to fit new bitmap data
	}

    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_2ND_LOGO_Reload Done\r\n");
    OSD_Memory_Protect( 1, POSD_MPROC );// v12 //A70LV_Larry_0415

    return ICHIP_OSD_PASS;
}
#endif

INT8 dvC821_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    INT16 iBmhw, iBmvw, iSrc_x, iSrc_y;
    UINT32 ulOsdct, ulRad, ulWad;
    UINT32 ulBOSTAT;

    ulOsdct = dvC821_Read( B0_OSDCT, 0 ) & 0xff;

    iSrc_x  = sBitmap.sSrc_Position.iX;
    iSrc_y  = sBitmap.sSrc_Position.iY;
    iBmhw = sBitmap.sBitmap_Size.iWidth;
    iBmvw = sBitmap.sBitmap_Size.iHeight;

    ulRad = POSD_PSAD + ( /*POSD_MWI*/DVC821_OSD_BITMAP_SKETCHPAD_WIDTH * 128 * iSrc_y + iSrc_x );// bitblt source address  //0x0a* 128(source total bitmap width) should read from JSon file define
	ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;  // bitblt destination address  //POSD_MWI = output OSD plane width

    dvC821_WriteToBuffer(B0_BBACTHW, (iBmhw - 1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC821_WriteToBuffer(B0_BBACTVW, (iBmvw - 1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC821_WriteToBuffer(B0_CPURAD, ulRad & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC821_WriteToBuffer(B0_CPUWAD, ulWad & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    dvC821_WriteToBuffer(B0_OSDCT, (ulOsdct & 0xf0) | DVC821_OSDCT_BITBLT, 0 ); //OSD control register  ==> BitBLT
    dvC821_Buffer_Flush();


    OSD_ResetTimeInState();

    do
    {
        if(OSD_TimeElapsedInState() >= 1000)
        {
            LOG_MSG(db_DV_SCALER_OSD, "(func:%s, line:%d) Time Out\n", __FUNCTION__, __LINE__);
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }

        ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

#if 0 //G100_Larry_0020
        if((ulBOSTAT & 0x01) != 0 )
        {
            MS_SLEEP(1);
        }
#endif /* 0 */

    } while ( (ulBOSTAT & 0x01) != 0 );

    return ICHIP_OSD_PASS;
}

INT8 dvC821_Paint_Specified_Bitmap(OSD_BITMAP sBitmap, START_POINT sOffset_position, RECT_SIZE sSrc_Size, START_POINT sDes_position)
{
    INT16 iBmhw, iBmvw, iSrc_x, iSrc_y;
    UINT32 ulOsdct, ulRad, ulWad;
	UINT32 ulBOSTAT;

    ulOsdct = dvC821_Read( B0_OSDCT, 0 ) & 0xff;

    iSrc_x = sBitmap.sSrc_Position.iX + sOffset_position.iX;
    iSrc_y = sBitmap.sSrc_Position.iY + sOffset_position.iY;
    iBmhw = sSrc_Size.iWidth;
    iBmvw = sSrc_Size.iHeight;

    ulRad = POSD_PSAD + ( /*POSD_MWI*/DVC821_OSD_BITMAP_SKETCHPAD_WIDTH * 128 * iSrc_y + iSrc_x );// bitblt source address  //0x0a* 128(source total bitmap width) should read from JSon file define
	ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;  // bitblt destination address  //POSD_MWI = output OSD plane width

    dvC821_WriteToBuffer(B0_BBACTHW, (iBmhw - 1) & 0xffff, 0); //BitBLT horizontal direction width, Specify the horizontal width of BitBTL
    dvC821_WriteToBuffer(B0_BBACTVW, (iBmvw - 1) & 0xffff, 0); //BitBLT vertical direction width register
    dvC821_WriteToBuffer(B0_CPURAD, ulRad & 0xffffffff, 0); //CPU read address, CPURAD[28:0]: Use at CPU read access for font development, bitblt and image memory
    dvC821_WriteToBuffer(B0_CPUWAD, ulWad & 0xffffffff, 0); //CPU write address, CPUWAD[28:0]: Use at CPU write access for font development, bitblt, fill and image memory

    dvC821_WriteToBuffer(B0_OSDCT, (ulOsdct & 0xf0) | DVC821_OSDCT_BITBLT, 0 ); //OSD control register  ==> BitBLT
    dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
    OSD_ResetTimeInState();

	do
    {
        ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
    }while((ulBOSTAT & 0x01) != 0 );
#endif /* SCALER_BOARD_STAGE */

    return ICHIP_OSD_PASS;
}
INT8 dvC821_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index)
{
    UINT32 ulWad;
    UINT32 ulOsdct;
    UINT32 ulBOSTAT;

    ulOsdct = dvC821_Read( B0_OSDCT, 0 ) & 0xff;

    //LOG_MSG(db_DV_SCALER_OSD, "Rectangle (%d,%d) size(%d,%d) color=%d\r\n", sDes_position.iX, sDes_position.iY, sDes_Size.iWidth, sDes_Size.iHeight, iColor_Index);
    ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;// write address

    dvC821_WriteToBuffer(B0_OSDFILL, iColor_Index & 0xff, 0 );
    dvC821_WriteToBuffer(B0_BBACTHW, (sDes_Size.iWidth - 1) & 0xffff, 0 );
    dvC821_WriteToBuffer(B0_BBACTVW, (sDes_Size.iHeight - 1) & 0xffff, 0 );
    dvC821_WriteToBuffer(B0_CPUWAD, ulWad & 0xffffffff, 0 );
    dvC821_WriteToBuffer(B0_OSDCT, (ulOsdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
    dvC821_Buffer_Flush();


#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }

    } while ( (ulBOSTAT & 0x01) != 0);
#endif /* SCALER_BOARD_STAGE */

    return ICHIP_OSD_PASS;
}


INT8 dvC821_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset)
{
    INT16 iCount;
    UINT32 ulOSDct;
    UINT32 ulRad;
    UINT32 ulWad;
    UINT32 ulBOSTAT;
    UINT8  ucCharTemp[5] = {0}; //A70LV_Larry_0409

    ulOSDct = dvC821_Read( B0_OSDCT, 0 ) & 0xff;

    dvC821_WriteToBuffer(B0_CBUFAD, 0x0a00, 0); //0xa00 -> 2560deci  //set address to 0xa00(2560) to set first line attribute.
    dvC821_WriteToBuffer(B0_CBUFDT, sString.ucStringMaxHeight & 0xff, 0); //Specify a data of character buffer to be accessed, set first line vertical height.
    dvC821_WriteToBuffer(B0_CBUFAD, 0x0000, 0); // reset character buffer address to 0x0000 prepare for character data writing



    for(iCount=0; iCount<(sString.uiStringLength); iCount++)
    {

#if 1 //A70LV_Larry_0409
        //LOG_MSG(db_HAL_GUI, "%d-th of total=%d char fill\n", iCount, (sString.uiStringLength-iStringOffset));
        dvC821_WriteToBuffer(B0_CBUFDT, (sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) & 0xff, 0);
        dvC821_WriteToBuffer(B0_CBUFDT, ((sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) >> 8) & 0xff, 0);
        dvC821_WriteToBuffer(B0_CBUFDT, iFont_Color & 0xff, 0);
        dvC821_WriteToBuffer(B0_CBUFDT, iBG_Color & 0xff, 0);
        dvC821_WriteToBuffer(B0_CBUFDT, *(sString.pucCharWdith+iCount+iStringOffset) & 0xff, 0); //set each character width.
#else
        ucCharTemp[0] = (sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) & 0xff;
        ucCharTemp[1] = ((sString.ulFontOffset + *(sString.puiCharIndex+iCount+iStringOffset)) >> 8) & 0xff;
        ucCharTemp[2] = iFont_Color & 0xff;
        ucCharTemp[3] = iBG_Color & 0xff;
        ucCharTemp[4] = *(sString.pucCharWdith+iCount+iStringOffset) & 0xff;
        dvC821_BurstWriteAsync_FixedAdd(B0_CBUFDT, 5, ucCharTemp);
#endif

    }



    dvC821_WriteToBuffer(B0_CBUFHST, 0x00, 0); //Character buffer horizontal direction start point
    dvC821_WriteToBuffer(B0_CBUFHW, (sString.uiStringLength - 1) & 0xff, 0); //Character buffer horizontal direction width
    dvC821_WriteToBuffer(B0_CBUFVST, 0x00, 0); //Character buffer vertical direction start point
    dvC821_WriteToBuffer(B0_CBUFVW, 0x00, 0); //Character buffer vertical direction width


    ulRad = POSD_FSAD;  //font_data address
    ulWad = POSD_SAD + sDes_position.iX + sDes_position.iY * POSD_MWI;//ST_OSDAD + wDes_x + wDes_y * PS_OSDMWI * 128;
    dvC821_WriteToBuffer(B0_CPURAD, ulRad & 0xffffffff, 0);
    dvC821_WriteToBuffer(B0_CPUWAD, ulWad & 0xffffffff, 0);
    dvC821_WriteToBuffer(B0_OSDCT, (ulOSDct&0xf0) | DVC821_OSDCT_CHAR, 0); //OSD command select => Character development

    dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
    OSD_ResetTimeInState();

    do
    {
        ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

        if(OSD_TimeElapsedInState() >= 1000)
        {
            dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
            break;
        }
    } while ( (ulBOSTAT & 0x01) != 0 );
#endif /* SCALER_BOARD_STAGE */

    return ICHIP_OSD_PASS;
}

void dvC821_OSD_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor) //A70LV_Larry_0055
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

    osdct = dvC821_Read(B0_OSDCT, 0);

    uiXStart_Div3_rd = uiXStart;

    uiXStart_Div3_rd_rem = uiXStart - uiXStart_Div3_rd;

    uiXEnd_Div3_rd = (uiXStart + uiXWidth);

    uiXEnd_Div3_rd_rem = (uiXStart + uiXWidth) - uiXEnd_Div3_rd;

    ulCpuWAd = POSD_SAD + uiXStart_Div3_rd + POSD_MWI * uiYStart;

    uiXWidth_Div3 = uiXEnd_Div3_rd - uiXStart_Div3_rd;


    if(ucForeColor == ucBackColor)
    {
        uiColor = ucBackColor;



        dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
        dvC821_WriteToBuffer(B0_BBACTHW, uiXWidth_Div3 & 0xFFFF, 0);
        dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
        dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
        dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
        dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
        OSD_ResetTimeInState();

        do
        {
            ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

            if(OSD_TimeElapsedInState() >= 1000)
            {
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }
        }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


        dvC821_Write(B0_OSDCT, 0x00, 0);

    }
    else if(uiXWidth_Div3 == 0)
    {

        dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
        dvC821_WriteToBuffer(B0_BBACTHW, 0x0000, 0);
        dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
        dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
        dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
        dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
        OSD_ResetTimeInState();

        do
        {
            ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

           if(OSD_TimeElapsedInState() >= 1000)
            {
                dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                break;
            }
        }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


        dvC821_Write(B0_OSDCT, 0x00, 0);

    }
    else if(uiXWidth_Div3 > 0)
    {
        if(uiXStart_Div3_rd_rem > 0)
        {


            uiColor = ucForeColor;
            dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC821_WriteToBuffer(B0_BBACTHW, 0x0000, 0);
            dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
            dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE  */


            dvC821_Write(B0_OSDCT, 0x00, 0);


            uiXStart_Div3_rd++;
            ulCpuWAd = POSD_SAD + uiXStart_Div3_rd + POSD_MWI * uiYStart;
            uiXWidth_Div3 = uiXEnd_Div3_rd - uiXStart_Div3_rd;
        }

        if(uiXWidth_Div3 == 0)
        {
            uiColor = ucForeColor;
            dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC821_WriteToBuffer(B0_BBACTHW, 0x0000, 0);
            dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
            dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


            dvC821_Write(B0_OSDCT, 0x00, 0);

        }
        else if(uiXEnd_Div3_rd_rem == 2)
        {

            uiColor = ucForeColor;
            dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC821_WriteToBuffer(B0_BBACTHW, uiXWidth_Div3 & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
            dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */


            dvC821_Write(B0_OSDCT, 0x00, 0);

        }
        else
        {

            uiColor = ucForeColor;
            dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC821_WriteToBuffer(B0_BBACTHW, (uiXWidth_Div3 - 1) & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
            dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAG */

            dvC821_WriteToBuffer(B0_OSDCT, 0x00, 0);

            ulCpuWAd = POSD_SAD + uiXEnd_Div3_rd + POSD_MWI * uiYStart;

            uiColor = ucForeColor;
            dvC821_WriteToBuffer(B0_OSDFILL, (uiColor & 0xFFFF), 0);
            dvC821_WriteToBuffer(B0_BBACTHW, 0x0000, 0);
            dvC821_WriteToBuffer(B0_BBACTVW, (uiYWidth - 1) & 0xFFFF, 0);
            dvC821_WriteToBuffer(B0_CPUWAD, ulCpuWAd & 0xFFFFFFFF, 0);
            dvC821_WriteToBuffer(B0_OSDCT, (osdct & 0xe0) | DVC821_OSDCT_FILL, 0 );
            dvC821_Buffer_Flush();

#if (SCALER_BOARD_STAGE > MOCKUP_STAGE)
            OSD_ResetTimeInState();

            do
            {
                ulBOSTAT = dvC821_Read( B0_BOSTAT, 0 );

                if(OSD_TimeElapsedInState() >= 1000)
                {
                    dvC821_OSD_OPDLogOut("(func:%s, line:%d) time out\r\n", __FUNCTION__, __LINE__);
                    break;
                }
            }while((ulBOSTAT & 0x01) == 0x01);
#endif /* SCALER_BOARD_STAGE */

            dvC821_Write(B0_OSDCT, 0x00, 0);

        }
    }
}

INT8 dvC821_OSD_PanelSet(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    OSDePanelTimingId = ePanelTimingId;
    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_PanelChange(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0226 modify//A70LV_Doulas_0105
{
    OSDePanelTimingId = ePanelTimingId;

    switch(OSDePanelTimingId)
    {
        case ePANEL_ID_WUXGA_60HZ:
            PS_PANEL_HST = OSD_H_START_WUGA_60;//56;
            if(dvC821_PIP_PBP_Enable() == TRUE)
            {
                PS_PANEL_VST = OSD_V_START_WUGA_60;// - 2;//20;//32; //A65_OPTOMA_Doulas_0060 Modify
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_WUGA_60;//20;//32;      //A70LV_Doulas_0170 mdoify
            }
            break;

        case ePANEL_ID_1080P_60HZ:
            PS_PANEL_HST = OSD_H_START_1080P_60;//192;
            if(dvC821_PIP_PBP_Enable() == TRUE)
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
            if(dvC821_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            {
                PS_PANEL_VST = OSD_V_START_1080P_120 + 2;//41;
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_1080P_120;//41;
            }
            break;

		case ePANEL_ID_WUXGA_120HZ:     //G100_Doulas_0064 Add
            PS_PANEL_HST = OSD_H_START_WUXGA_120;
            if(dvC821_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
            {
                PS_PANEL_VST = OSD_V_START_WUXGA_120 +2;
            }
            else
            {
                PS_PANEL_VST = OSD_V_START_WUXGA_120;
            }
            break;

        default:
			PS_PANEL_HST = OSD_H_START_1080P_60;    //G100_Doulas_0064
            PS_PANEL_VST = OSD_V_START_1080P_60;    //G100_Doulas_0064
            break;
    }

    POSD_HST = (PS_PANEL_HST - 5);
    POSD_VST = (PS_PANEL_VST + 0);
    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_MenuTransparencySet(UINT8 ucValue)     //A70LV_Doulas_0122
{
    UINT8 ucMenuTransparency;   //128 OSD no Transparency, 0 no OSD
    ucMenuTransparency = 128 - (ucValue/MENU_TRANSPARENCY_STEP_VALUE)*4;

    dvC821_WriteToBuffer(B3_OSDBLENDC1CH1   , ucMenuTransparency ,0);
    dvC821_WriteToBuffer(B3_OSDBLENDC1CH1   , ucMenuTransparency ,13);
    dvC821_Buffer_Flush();


    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_MenuTransparencySet %d\r\n",ucMenuTransparency);
    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe)     //A70LV_Doulas_0122
{
    UINT8 ucVal;

    ucVal = dvC821_Read(B3_OSDCT1CH1,0);
    if(ucEnalbe )
    {
        ucVal = ucVal | 0x02;   //OSD blend enable(Menu Transparency Enable)
    }
    else
    {
        ucVal = ucVal & 0xFD;   //OSD blend disable(Menu Transparency disable)
    }

    dvC821_WriteToBuffer( B3_OSDCT1CH1, ucVal, 0 );
    dvC821_WriteToBuffer( B3_OSDCT1CH1, ucVal, 13 );
    dvC821_Buffer_Flush();

    LOG_MSG(db_DV_SCALER_OSD, "dvC821_OSD_MenuTransparencyEnableSet %d\r\n",ucEnalbe);
    return ICHIP_OSD_PASS;
}

//only for debug
BOOL dvC821_ShowOsdBitmapData(UINT32 ulStartVertPosition)  //A35G2_Simon_0114
{
    UINT32 ulAddr = POSD_PSAD + (ulStartVertPosition * POSD_MWI)  ;
    dvC821_Write(B3_OSDSAD1CH1, ulAddr, 0);
}


