// ===============================================================================
// FILE NAME: dvC789_WarpLight.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/12/29, Larry Create
// --------------------
// ===============================================================================

#include "dvC789.h"
#include "dvC789_WarpLight.h"
#include "utilDbgMsg.h"
#include <math.h>


static sDRV_WARPING_INFO m_sDrvWarpingInfo;

#if 0 //A70LV_Larry_0137
void dvC789_DrawGrid(UINT16 uiX, UINT16 uiY, UINT8 ucGridSelX, UINT8 ucGridSelY)// write_grid()
{
#if 0
    UINT16 uiXStart = 0;
    UINT16 uiXWidth = 0;
    UINT16 uiYStart = 0;
    UINT16 uiYWidth = 0;

    if(m_sDrvWarpingInfo.bInit == FALSE)
        return;

    if((uiX == ucGridSelX) && (uiY == ucGridSelY))
    {
        uiXStart = (uiX == 0) ? 0 : (uiX == 16) ? (UINT16)m_sDrvWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                                    (UINT16)m_sDrvWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);
        uiXWidth  = ((uiX == 0) || (uiX == 16)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;
        uiYStart = (uiY == 0) ? 0 : (uiY == 16) ? (UINT16)m_sDrvWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                                    (UINT16)m_sDrvWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);
        uiYWidth  = ((uiY == 0) || (uiY == 16)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;

        dvC789_OSD_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 250, 252);

        uiXStart = (uiX == 0) ? uiXStart : uiXStart + 2;
        uiXWidth  = ((uiX == 0) || (uiX == 16)) ? uiXWidth - 2 : uiXWidth - 4;
        uiYStart = (uiY == 0) ? uiYStart : uiYStart + 2;
        uiYWidth  = ((uiY == 0) || (uiY == 16)) ? uiYWidth - 2 : uiYWidth - 4;

        dvC789_OSD_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 252, 250);
    }

    uiXStart = (uiX == 0) ? 0 : (uiX == 16) ? (UINT16)m_sDrvWarpingInfo.faCursorDefInX[uiX] - DEF_GRID_SIZE :
                        (UINT16)m_sDrvWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE / 2);



    uiXWidth  = DEF_GRID_SIZE;


    uiYStart = (uiY == 0) ? 0 : (uiY == 16) ? (UINT16)m_sDrvWarpingInfo.faCursorDefInY[uiY] - DEF_GRID_SIZE :
                        (UINT16)m_sDrvWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE / 2);
    uiYWidth  = DEF_GRID_SIZE;

    dvC789_OSD_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 250, 252);
#endif /* 0 */

    return;
}


void dvC789_EraseGrid(UINT16 uiX, UINT16 uiY)
{
#if 0

    // fCursorDefInX : define real coordinate in the 3840x2160 canvas
    UINT16 uiXStart = (uiX== 0) ? 0 : (uiX== DEF_GRID_SIZE) ? (UINT16)m_sDrvWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                        (UINT16)m_sDrvWarpingInfo.faCursorDefInX[uiX] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);

    UINT16 uiXWidth  = ((uiX == 0) || (uiX == DEF_GRID_SIZE)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;
    UINT16 uiYStart = (uiY == 0) ? 0 : (uiY == DEF_GRID_SIZE) ? (UINT16)m_sDrvWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE + DEF_GRID_SIZE_O) :
                            (UINT16)m_sDrvWarpingInfo.faCursorDefInY[uiY] - (DEF_GRID_SIZE / 2 + DEF_GRID_SIZE_O);
    UINT16 uiYWidth  = ((uiY== 0) || (uiY == DEF_GRID_SIZE)) ? DEF_GRID_SIZE + DEF_GRID_SIZE_O : DEF_GRID_SIZE + DEF_GRID_SIZE_O * 2;

    if(m_sDrvWarpingInfo.bInit == FALSE)
        return;

    // erase grid
    dvC789_OSD_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, 255, 255);
#endif /* 0 */

    return;
}


void dvC789_ShowGrid(BOOL bGridShow)//change_grid
{
#if 0
    UINT8 ucOsdMode = 0;
    UINT8 ucCH_enable;

    if(m_sDrvWarpingInfo.bInit == FALSE)
        return;

	dvC789_Write( B0_RTCT, RTCT_POVSSTOP, 0 );


    ucOsdMode = (UINT8)dvC789_Read(B3_OSDMODE1CH1, 0);


    if(bGridShow == TRUE)
    {
        ucOsdMode |= 0x07;
        dvC789_Write(B3_OSDMODE1CH1, ucOsdMode, 0);
    }
    else
    {
        ucOsdMode = (ucOsdMode & 0x0B) | 0x03;
        dvC789_Write(B3_OSDMODE1CH1, ucOsdMode, 0);
    }


    dvC789_Write( B0_RTCT, B3_OSDMODE1CH1, 0 );

    dvC789_Write( B0_RTCT, RTCT_NORMAL, 0 );

    if (ucCH_enable & 0x01)  // Ch1 setting
        dvC789_WaitIntct(eMCT_CH1, C789_INT_PO1VS);  //OSD CH1 wait_povs
#endif /* 0 */

    return;
}
#endif /* 0 */

void dvC789_WarpLight_PanelConfig(void)
{
    UINT8 ucV = 0;
    UINT8 ucH = 0;
    UINT8 ucNumCascade = 0;

    m_sDrvWarpingInfo.bInit = 1;

    m_sDrvWarpingInfo.uiPanelHW = dvC789_Read(B2_OACTHW);
    m_sDrvWarpingInfo.uiPanelVW = dvC789_Read(B2_OACTVW);

    //dvC790_GetOutputTiming(ucTblIndex);

    m_sDrvWarpingInfo.uiPixelGridHW = (m_sDrvWarpingInfo.uiPanelHW + DEF_WP_SPACE - 1) / DEF_WP_SPACE; //roundup
    m_sDrvWarpingInfo.uiPixelGridVW = (m_sDrvWarpingInfo.uiPanelVW + DEF_WP_SPACE - 1) / DEF_WP_SPACE; //roundup

    // Warping
    //m_sDrvWarpingInfo.ucWarpColor = 0;//Fix



    for(ucV = 0; ucV < 17; ucV++)
    {
        m_sDrvWarpingInfo.faCursorDefOutY[ucV] = (float)((m_sDrvWarpingInfo.uiPanelVW/*PS_WP_VW*/ * ucV) / 16.0);//Because we have 17 grid, thus div 16
        m_sDrvWarpingInfo.faCursorDefInY[ucV] = (float)((m_sDrvWarpingInfo.uiPanelVW/*PM_IACT_VW*/ * ucV) / 16.0);

        for(ucH = 0; ucH < 17; ucH++)
        {
            if(ucV == 0)
            {
                 m_sDrvWarpingInfo.faCursorDefOutX[ucH] = (float)((m_sDrvWarpingInfo.uiPanelHW/*PS_WP_HW*/ * ucH) / 16.0);
                 m_sDrvWarpingInfo.faCursorDefInX[ucH] = (float)((m_sDrvWarpingInfo.uiPanelHW/*PM_IACT_HW*/ * ucH) / 16.0);
            }

            for(ucNumCascade = 0; ucNumCascade < DEF_NUM_CASC; ucNumCascade++)
            {
                m_sDrvWarpingInfo.saGridPos[ucNumCascade][ucH][ucV].fX = m_sDrvWarpingInfo.faCursorDefOutX[ucH];
                m_sDrvWarpingInfo.saGridPos[ucNumCascade][ucH][ucV].fY = m_sDrvWarpingInfo.faCursorDefOutY[ucV];
            }
        }
    }
}

static void dvC789_SetLut(void)
{
    UINT16 uiX = 0, uiY = 0;
    UINT16 uiIndex = 0;
    UINT8 ucHSclLut = 0;
    UINT8 ucVSclLut = 0;
    float fInStart = 0, fInEnd = 0;
    float fOutStart = 0, fOutEnd = 0;
    double fSclMin = 0, fSclMax = 0, fScl = 0;
    UINT8 aucBuffer[24] = {0}; //A70LV_Larry_0061

    // +++++ HLUT +++++ //
    fSclMin = 65535;
    fSclMax = 0;

    for(uiY = 0; uiY <= m_sDrvWarpingInfo.uiPixelGridVW; uiY++)
    {
        fOutStart = -1;
        fOutEnd = -1;

        uiIndex = DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW;

        for(uiX = DEF_WP_OUT_HGRD; uiX <= (DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW); uiX++)
        {
            uiIndex = uiX;

            if(m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][uiX][uiY] >= 0)
            {
                fInStart = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][uiX][uiY];
                fOutStart = (float)(uiX * DEF_WP_SPACE);
                break;
            }
        }

        for(uiX = (DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW); uiX > uiIndex; uiX--)
        {
            if(m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][uiX][uiY] <= m_sDrvWarpingInfo.uiPanelHW/*PM_IACT_VW*/)
            {
                fInEnd = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][uiX][uiY];
                fOutEnd = (float)(uiX * DEF_WP_SPACE);
                break;
            }
        }

        if((fOutStart >= 0) && (fOutEnd >= 0))
        {
            fScl = (fInEnd - fInStart) / (fOutEnd - fOutStart);

            if(fScl < fSclMin)
            {
                fSclMin = fScl;
            }

            if(fScl > fSclMax)
            {
                fSclMax = fScl;
            }
        }
    }

    fScl = (fSclMin > fSclMax) ? 1 : (fSclMin + fSclMax) / 2;
    ucHSclLut = (UINT8)((fScl >= 1.5) ? 9 : (fScl >= 1.1) ? 8 : (fScl < 0.3) ? 0 : ((fScl * 10) - 3));// -3 -> LUT[0] = cutoff 0.3

    // +++++ VLUT +++++ //
    fSclMin = 65535;
    fSclMax = 0;

    for(uiX = 0; uiX <= m_sDrvWarpingInfo.uiPixelGridHW; uiX++)
    {
        fOutStart = -1;
        fOutEnd = -1;

        uiIndex = DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW;

        for(uiY = DEF_WP_OUT_VGRD; uiY <= (DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW); uiY++)
        {
            uiIndex = uiY;

            if(m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][uiX][uiY] >= 0)
            {
                fInStart = m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][uiX][uiY];
                fOutStart = (float)(uiY * DEF_WP_SPACE);
                break;
            }
        }

        for(uiY = (DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW); uiY > uiIndex; uiY--)
        {
            if(m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][uiX][uiY] <= m_sDrvWarpingInfo.uiPanelHW/*PM_IACT_HW*/)
            {
                fInEnd = m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][uiX][uiY];
                fOutEnd = (float)(uiY * DEF_WP_SPACE);
                break;
            }
        }

        if((fOutStart >= 0) && (fOutEnd >= 0))
        {
            fScl = (fInEnd - fInStart) / (fOutEnd - fOutStart);

            if(fScl < fSclMin)
            {
                fSclMin = fScl;
            }

            if(fScl > fSclMax)
            {
                fSclMax = fScl;
            }
        }
    }

    fScl = (fSclMin > fSclMax) ? 1 : (fSclMin + fSclMax) / 2;
    ucVSclLut = (UINT8)((fScl >= 1.5) ? 9 : (fScl >= 1.1) ? 8 : (fScl < 0.3) ? 0 : (fScl * 10 - 3));// -3 -> LUT[0] = cutoff 0.3

    // +++++ set LUT registers +++++ //

    memcpy(aucBuffer, &m_sPolationTable[ucHSclLut][0], 24);
    dvC789_BurstWrite_AddInc(B6_HLUT0, 24, aucBuffer);

    memcpy(aucBuffer, &m_sPolationTable[ucVSclLut][0], 24);
    dvC789_BurstWrite_AddInc(B6_VLUT0, 24, aucBuffer);



    return;
}

static void dvC789_WriteWarpTable(void)
{
    UINT16 uiX, uiY;
    UINT16 uiXAd, uiYAd;
    UINT16 uiAd;
    UINT16 uiAdShift;
    UINT16 uiXWarpAd;

    UINT16 uiAdStart;
    UINT16 uiYAdShift;
    float fPointX, fPointY;
    double fTemp;
    INT16 iWarpTableX;
    INT16 iWarpTableY;

    // for dummy //
    float fXStartMin, fXEndMax;
    INT16 iWarpTableDummyX = 0;
    INT16 iWarpTableDummyY = 0;
    ///////////////

    UINT32 ulRTCT = 0;
    UINT8  aucData[5] = {0}; //A70LV_Larry_0061
    UINT32  ulAccess_Table = 0;

    ulAccess_Table = dvC789_Read(B5_DTCT);

    //b4 Correction table select
    //DTSEL Operation
    //0 Use A side.
    //1 Use B side. (enabled only when RGBGC=0)

    if(ulAccess_Table & 0x00000010)
    {
        m_sDrvWarpingInfo.ucWarpColor = 0;
    }
    else
    {
        m_sDrvWarpingInfo.ucWarpColor = 1;
    }


    if(m_sDrvWarpingInfo.ucWarpColor == 0)
    {
        if(DEF_WP_SPACE == 32)
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_32PIEXL_A);
            uiAdShift = 6;
            uiAdStart = 2560;
            uiXWarpAd = 64;
            uiYAdShift = 1;
        }
        else
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_A);
            uiAdShift = 7;
            uiAdStart = 9984;
            uiXWarpAd = 128;
            uiYAdShift = 1;
        }
    }
    else
    {
        if(DEF_WP_SPACE == 32)
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_32PIEXL_B);
            uiAdShift = 6;
            uiAdStart = 2560;
            uiXWarpAd = 64;
            uiYAdShift = 1;
        }
        else
        {

            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_ENABLE_16PIEXL_B);
            uiAdShift = 7;
            uiAdStart = 9984;
            uiXWarpAd = 128;
            uiYAdShift = 1;
        }
    }


    ulRTCT = dvC789_Read(BN_RTCT);
    ulRTCT = ulRTCT & RTCT_WARP_TRANSFER_DISABLE;
    dvC789_Write(BN_RTCT, ulRTCT);


    // for dummy area //
    fXStartMin = (float)m_sDrvWarpingInfo.uiPanelHW;// PM_IACT_HW
    fXEndMax = 0;


    // To get fXStartMin and fXEndMax correct value
    for(uiY = DEF_WP_OUT_VGRD; uiY <= (DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW); uiY++)
    {
        fPointX = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][uiY] - (float)m_sDrvWarpingInfo.iDummyHstReset;

        if(fPointX < 0)
        {
            fXStartMin = 0;
        }
        else if(fPointX < fXStartMin)
        {
            fXStartMin = fPointX;
        }

        fPointX = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW][uiY];

        if(fPointX > (float)m_sDrvWarpingInfo.uiPanelHW/*PM_IACT_HW*/)
        {
            fXEndMax = (float)m_sDrvWarpingInfo.uiPanelHW/*PM_IACT_HW*/;
        }
        else if(fPointX > fXEndMax)
        {
            fXEndMax = fPointX;
        }
    }

    ////////////////////
    //

    for(uiY = DEF_WP_OUT_VGRD; uiY <= (DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW); uiY++)
    {
        uiYAd = uiY - DEF_WP_OUT_VGRD;
        uiAd = (uiYAd << uiAdShift);

        aucData[0] = uiAd & 0xFF;
        aucData[1] = (uiAd >> 8) & 0xFF;
        dvC789_BurstWrite_FixedAdd(B5_DTAD, 2, aucData); //A70LV_Larry_0061

        //dvC789_Write_NoSemaphore(B31_DTAD, uiAd & 0xFF, 0);
        //dvC789_Write_NoSemaphore(B31_DTAD, (uiAd >> 8) & 0xFF, 0);

        uiXAd = 0;

        // Dummy area (HST) //
        fPointX = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][uiY] - (float)m_sDrvWarpingInfo.iDummyHstReset;
        fPointY = m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD][uiY];

        fTemp = (fPointY > 3071) ? 3071 : (fPointY < -1024) ? -1024 : fPointY;
        iWarpTableDummyY = (INT16)(fTemp * 16 + 0.5);

#if (DEF_HW_DUMMY_GRD != 0)
        for(uiX = 0; uiX < DEF_HW_DUMMY_GRD; uiX++)
        {
            fTemp = (fPointX < fXStartMin) ? fPointX : fXStartMin + (fPointX - fXStartMin) * uiX / DEF_HW_DUMMY_GRD;
            fTemp = (fTemp > 3068) ? 3068 : (fTemp < -1024) ? -1024 : fTemp;
            iWarpTableDummyX = (INT16)(fTemp * 16 + 0.5);
            // write register

            aucData[0] = iWarpTableDummyX & 0xFF;
            aucData[1] = (iWarpTableDummyX >> 8) & 0xFF;
            aucData[2] = iWarpTableDummyY & 0xFF;
            aucData[3] = (iWarpTableDummyY >> 8) & 0xFF;
            dvC789_BurstWrite_FixedAdd(B5_DTDT, 4, aucData); //A70LV_Larry_0061

            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableDummyX & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableDummyX >> 8) & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableDummyY & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableDummyY >> 8) & 0xFF, 0);//writeb_C790

            uiXAd++;
        }
#endif

        //////////////////////

        for(uiX = DEF_WP_OUT_HGRD; uiX <= (DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW); uiX++)
        {
            if(uiXAd == uiXWarpAd)
            {
                uiAd = uiAdStart + (uiYAd << uiYAdShift);

                aucData[0] = uiAd & 0xFF;
                aucData[1] = (uiAd >> 8) & 0xFF;
                dvC789_BurstWrite_FixedAdd(B5_DTAD, 2, aucData); //A70LV_Larry_0061

                //dvC789_Write_NoSemaphore(B31_DTAD, uiAd & 0xFF, 0);//writeb_C790
                //dvC789_Write_NoSemaphore(B31_DTAD, (uiAd >> 8) & 0xFF, 0);//writeb_C790

            }

            fPointX = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][uiX][uiY] - (float)m_sDrvWarpingInfo.iDummyHstReset;
            fTemp = (fPointX > 3068) ? 3068 : (fPointX < -1024) ? -1024 : fPointX;
            iWarpTableX = (INT16)(fTemp * 16 + 0.5);

            fPointY = m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][uiX][uiY];
            fTemp = (fPointY > 3071) ? 3071 : (fPointY < -1024) ? -1024 : fPointY;
            iWarpTableY = (INT16)(fTemp * 16 + 0.5);
            // write register

            aucData[0] = iWarpTableX & 0xFF;
            aucData[1] = (iWarpTableX >> 8) & 0xFF;
            aucData[2] = iWarpTableY & 0xFF;
            aucData[3] = (iWarpTableY >> 8) & 0xFF;
            dvC789_BurstWrite_FixedAdd(B5_DTDT, 4, aucData); //A70LV_Larry_0061

            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableX & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableX >> 8) & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableY & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableY >> 8) & 0xFF, 0);//writeb_C790

        } // end for x

        // Dummy area (HEND) //
        fPointX = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW][uiY] - (float)m_sDrvWarpingInfo.iDummyHstReset;
        fPointY = m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW][uiY];

        fTemp = (fPointY > 3071) ? 3071 : (fPointY < -1024) ? -1024 : fPointY;
        iWarpTableDummyY = (INT16)(fTemp * 16 + 0.5);

#if (DEF_HW_DUMMY_GRD != 0)
        for(uiX = 0; uiX < DEF_HW_DUMMY_GRD; uiX++)
        {
            if(uiXAd == uiXWarpAd)
            {
                uiAd = uiAdStart + (uiYAd << uiYAdShift);

                aucData[0] = uiAd & 0xFF;
                aucData[1] = (uiAd >> 8) & 0xFF;
                dvC789_BurstWrite_FixedAdd(B5_DTAD, 2, aucData); //A70LV_Larry_0061

                //dvC789_Write_NoSemaphore(B31_DTAD, uiAd & 0xFF, 0);//writeb_C790
                //dvC789_Write_NoSemaphore(B31_DTAD, (uiAd >> 8) & 0xFF, 0);//writeb_C790
            }

            fTemp = (fPointX > fXEndMax) ? fPointX : fPointX + (fXEndMax - fPointX) * (uiX + 1) / DEF_HW_DUMMY_GRD;
            fTemp = (fTemp > 3068) ? 3068 : (fTemp < -1024) ? -1024 : fTemp;
            iWarpTableDummyX = (INT16)(fTemp * 16 + 0.5);

            aucData[0] = iWarpTableDummyX & 0xFF;
            aucData[1] = (iWarpTableDummyX >> 8) & 0xFF;
            aucData[2] = iWarpTableDummyY & 0xFF;
            aucData[3] = (iWarpTableDummyY >> 8) & 0xFF;
            dvC789_BurstWrite_FixedAdd(B5_DTDT, 4, aucData); //A70LV_Larry_0061

            // write register
            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableDummyX & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableDummyX >> 8) & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableDummyY & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableDummyY >> 8) & 0xFF, 0);//writeb_C790


            uiXAd++;
        }
#endif
        if(m_sDrvWarpingInfo.iDummyHendGridReset > 0)
        {
            if(uiXAd == uiXWarpAd)
            {
                uiAd = uiAdStart + (uiYAd << uiYAdShift);

                aucData[0] = uiAd & 0xFF;
                aucData[1] = (uiAd >> 8) & 0xFF;
                dvC789_BurstWrite_FixedAdd(B5_DTAD, 2, aucData); //A70LV_Larry_0061

                //dvC789_Write_NoSemaphore(B31_DTAD, uiAd & 0xFF, 0);
                //dvC789_Write_NoSemaphore(B31_DTAD, (uiAd >> 8) & 0xFF, 0);
            }

            aucData[0] = iWarpTableDummyX & 0xFF;
            aucData[1] = (iWarpTableDummyX >> 8) & 0xFF;
            aucData[2] = iWarpTableDummyY & 0xFF;
            aucData[3] = (iWarpTableDummyY >> 8) & 0xFF;
            dvC789_BurstWrite_FixedAdd(B5_DTDT, 4, aucData); //A70LV_Larry_0061

            // write register
            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableDummyX & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableDummyX >> 8) & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, iWarpTableDummyY & 0xFF, 0);//writeb_C790
            //dvC789_Write_NoSemaphore(B31_DTDT, (iWarpTableDummyY >> 8) & 0xFF, 0);//writeb_C790
        }

        ///////////////////////
    } // end for y

    //

    ulRTCT = ulRTCT | RTCT_WARP_TRANSFER_DISABLE;
    dvC789_Write(BN_RTCT, ulRTCT);

    if(m_sDrvWarpingInfo.ucWarpColor == 0 )
    {
        if(DEF_WP_SPACE == 32)
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_32PIEXL_A);
        }
        else
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_A);
        }

        m_sDrvWarpingInfo.ucWarpColor = 1;
    }
    else
    {
        if(DEF_WP_SPACE == 32)
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_32PIEXL_B);
        }
        else
        {
            dvC789_Write(B5_DTCT, DTCT_BASE_ACCESS_DISABLE_16PIEXL_B);
        }
        m_sDrvWarpingInfo.ucWarpColor = 0;
    }

    //dvC789_Write(B0_RTCT, RTCT_NORMAL, 0 );

    //GV_RTCT = GV_RTCT | RTCT_WARP_TRANSFER_ENABLE;
    ulRTCT = ulRTCT & RTCT_WARP_WPPOVS_ENABLE;
    ulRTCT = ulRTCT | RTCT_WARP_TRANSFER_ENABLE;

    dvC789_Write(BN_RTCT, ulRTCT);

    dvC789_SetLut();

}



static void dvC789_CalmvXCoef(float faNx[])
{
    UINT16 nIndex;
    float fXp[17], fYp[17];
    float fH[19], fYd[19];
    float fA[19], fB[19], fC[19], fD[19], fXk[17];

    // horizontal line
    for(nIndex = 0; nIndex < 17; nIndex++)
    {
        fXp[nIndex] = faNx[nIndex];
        fYp[nIndex] = m_sDrvWarpingInfo.faCursorDefInX[nIndex] - faNx[nIndex];
    }

    for(nIndex = 2 ; nIndex < 18; nIndex++)
    {
        fH[nIndex]  = fXp[nIndex - 1] - fXp[nIndex - 2];
        fYd[nIndex] = (fYp[nIndex - 1] - fYp[nIndex - 2]) / fH[nIndex];
    }

    // dummy
    fH[0] = fH[2];
    fH[1] = fH[2];
    fH[18] = fH[17];
    fYd[0] = fYd[2];
    fYd[1] = fYd[2];
    fYd[18] = fYd[17];

    for(nIndex = 1; nIndex < 18; nIndex++)
    {
        fA[nIndex] = 2 * (fH[nIndex] + fH[nIndex + 1]);
        fB[nIndex] = fH[nIndex + 1];
        fC[nIndex] = fH[nIndex];
        fD[nIndex] = 3 * (fYd[nIndex + 1] - fYd[nIndex]);
    }

    // dummy
    fA[0] = fA[1];
    fA[18] = fA[17];
    fB[0] = fB[1];
    fB[18] = fB[17];
    fC[0] = fC[1];
    fC[18] = fC[17];
    fD[0] = fD[1];
    fD[18] = fD[17];

    for(nIndex = 0; nIndex < 17; nIndex++)
    {
        fXk[nIndex] = (fA[nIndex] * fA[nIndex + 2] * fD[nIndex + 1] - fA[nIndex + 2] *
                fC[nIndex + 1] * fD[nIndex] - fA[nIndex] * fB[nIndex + 1] * fD[nIndex + 2]) /
                (fA[nIndex] * fA[nIndex + 1] * fA[nIndex + 2] - fA[nIndex + 2] * fB[nIndex] *
                fC[nIndex + 1] - fA[nIndex] * fB[nIndex + 1] * fC[nIndex + 2]);
    }

    for(nIndex = 0; nIndex < 16; nIndex++)
    {
        m_sDrvWarpingInfo.saWarpHorLineMove[nIndex].fC = fXk[nIndex];
        m_sDrvWarpingInfo.saWarpHorLineMove[nIndex].fD = (fXk[nIndex + 1] - fXk[nIndex]) / (3 * fH[nIndex + 2]);
        m_sDrvWarpingInfo.saWarpHorLineMove[nIndex].fB = fYd[nIndex + 2] - fH[nIndex + 2] * (m_sDrvWarpingInfo.saWarpHorLineMove[nIndex].fC + m_sDrvWarpingInfo.saWarpHorLineMove[nIndex].fD * fH[nIndex + 2]);
        m_sDrvWarpingInfo.saWarpHorLineMove[nIndex].fA = fYp[nIndex];
    }
}


void dvC789_WarpModeChange(eC789_WARP_MODE eWarpMode)//wpmd_change
{
    UINT16 uiIndex, uiIndex2;

    if(m_sDrvWarpingInfo.bInit == FALSE)
        return;


    m_sDrvWarpingInfo.uiNumHGrid = (eWarpMode == eC789_WARP_MODE_17x17) ? 17 :
                        (eWarpMode == eC789_WARP_MODE_9x9) ? 9 :
                        (eWarpMode == eC789_WARP_MODE_5x5) ? 5 :
                        3;
    m_sDrvWarpingInfo.uiNumVGrid = (eWarpMode == eC789_WARP_MODE_17x17) ? 17 :
                        (eWarpMode == eC789_WARP_MODE_9x9) ? 9 :
                        (eWarpMode == eC789_WARP_MODE_5x5) ? 5 :
                        3;
    m_sDrvWarpingInfo.uiNumHGrid_M1 = m_sDrvWarpingInfo.uiNumHGrid - 1;
    m_sDrvWarpingInfo.uiNumVGrid_M1 = m_sDrvWarpingInfo.uiNumVGrid - 1;
    m_sDrvWarpingInfo.uiNumHGrid_M2 = m_sDrvWarpingInfo.uiNumHGrid - 2;
    m_sDrvWarpingInfo.uiNumVGrid_M2 = m_sDrvWarpingInfo.uiNumVGrid - 2;
    m_sDrvWarpingInfo.uiNumHGrid_P1 = m_sDrvWarpingInfo.uiNumHGrid + 1;
    m_sDrvWarpingInfo.uiNumVGrid_P1 = m_sDrvWarpingInfo.uiNumVGrid + 1;

    for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
    {
        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            if(eWarpMode == eC789_WARP_MODE_2x2)
            {
                if(uiIndex2 == 0)
                {
                    m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] = ((uiIndex == 0) || (uiIndex == 8) || (uiIndex == 16)) ? TRUE : FALSE;
                }

                if(uiIndex == 0)
                {
                    m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 8) || (uiIndex2 == 16)) ? TRUE : FALSE;
                }
            }
            else if(eWarpMode == eC789_WARP_MODE_3x3)
            {
                if(uiIndex2 == 0)
                {
                    m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] = ((uiIndex == 0) || (uiIndex == 8) || (uiIndex == 16)) ? TRUE : FALSE;
                }

                if(uiIndex == 0)
                {
                    m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 8) || (uiIndex2 == 16)) ? TRUE : FALSE;
                }
            }
            else if(eWarpMode == eC789_WARP_MODE_5x5)
            {
                if(uiIndex2 == 0)
                {
                   m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] = ((uiIndex == 0) || (uiIndex == 4) || (uiIndex == 8) ||
                                    (uiIndex == 12) || (uiIndex == 16)) ? TRUE : FALSE;
                }

                if(uiIndex == 0)
                {
                    m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 4) || (uiIndex2 == 8) ||
                                        (uiIndex2 == 12) || (uiIndex2 == 16)) ? TRUE : FALSE;
                }
            }
            else if(eWarpMode == eC789_WARP_MODE_9x9)
            {
                if(uiIndex2 == 0)
                {
                    m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] = ((uiIndex == 0) || (uiIndex == 2) || (uiIndex == 4) || (uiIndex == 6) ||
                                     (uiIndex == 8) || (uiIndex == 10) || (uiIndex == 12) ||
                                     (uiIndex == 14) || (uiIndex == 16)) ? TRUE : FALSE;
                }

                if(uiIndex == 0)
                {
                    m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] = ((uiIndex2 == 0) || (uiIndex2 == 2) || (uiIndex2 == 4) ||
                                    (uiIndex2 == 6) || (uiIndex2 == 8) || (uiIndex2 == 10) ||
                                    (uiIndex2 == 12) || (uiIndex2 == 14) || (uiIndex2 == 16)) ? TRUE : FALSE;
                }
            }
            else
            {
                if(uiIndex2 == 0)
                {
                   m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] = TRUE;
                }

                if(uiIndex == 0)
                {
                    m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] = TRUE;
                }
            }
        }
    }

    return;
}

static void dvC789_CalmvYCoef(float faNy[], UINT16 uiXGrid)
{
    int nIndex;
    float fXp[17], fYp[17];
    float fH[19], fYd[19];
    float fA[19], fB[19], fC[19], fD[19], fXk[17];

    // horizontal line
    for(nIndex = 0; nIndex < 17; nIndex++)
    {
        fXp[nIndex] = faNy[nIndex];
        fYp[nIndex] = m_sDrvWarpingInfo.faCursorDefInY[nIndex] - faNy[nIndex];
    }

    for(nIndex = 2; nIndex < 18; nIndex++)
    {
        fH[nIndex]  = fXp[nIndex - 1] - fXp[nIndex - 2];
        fYd[nIndex] = (fYp[nIndex - 1] - fYp[nIndex - 2]) / fH[nIndex];
    }

    // dummy
    fH[0] = fH[2];
    fH[1] = fH[2];
    fH[18] = fH[17];
    fYd[0] = fYd[2];
    fYd[1] = fYd[2];
    fYd[18] = fYd[17];

    for(nIndex = 1; nIndex < 18; nIndex++)
    {
        fA[nIndex] = 2 * (fH[nIndex] + fH[nIndex + 1]);
        fB[nIndex] = fH[nIndex + 1];
        fC[nIndex] = fH[nIndex];
        fD[nIndex] = 3 * (fYd[nIndex + 1] - fYd[nIndex]);
    }

    // dummy
    fA[0] = fA[1];
    fA[18] = fA[17];
    fB[0] = fB[1];
    fB[18] = fB[17];
    fC[0] = fC[1];
    fC[18] = fC[17];
    fD[0] = fD[1];
    fD[18] = fD[17];

    for(nIndex = 0; nIndex < 17; nIndex++)
    {
        fXk[nIndex] = (fA[nIndex] * fA[nIndex + 2] * fD[nIndex + 1] - fA[nIndex + 2] *
            fC[nIndex + 1] * fD[nIndex] - fA[nIndex] * fB[nIndex + 1] * fD[nIndex + 2]) /
            (fA[nIndex] * fA[nIndex + 1] * fA[nIndex + 2] - fA[nIndex + 2] * fB[nIndex] *
            fC[nIndex + 1] - fA[nIndex] * fB[nIndex + 1] * fC[nIndex + 2]);
    }

    for(nIndex = 0; nIndex < 16; nIndex++)
    {
        m_sDrvWarpingInfo.saWarpVerLineMove[uiXGrid][nIndex].fC = fXk[nIndex];
        m_sDrvWarpingInfo.saWarpVerLineMove[uiXGrid][nIndex].fD = (fXk[nIndex + 1] - fXk[nIndex]) / (3 * fH[nIndex + 2]);
        m_sDrvWarpingInfo.saWarpVerLineMove[uiXGrid][nIndex].fB = fYd[nIndex + 2] - fH[nIndex + 2] * (m_sDrvWarpingInfo.saWarpVerLineMove[uiXGrid][nIndex].fC + m_sDrvWarpingInfo.saWarpVerLineMove[uiXGrid][nIndex].fD * fH[nIndex + 2]);
        m_sDrvWarpingInfo.saWarpVerLineMove[uiXGrid][nIndex].fA = fYp[nIndex];
    }
}


static void dvC789_CalLineCoef(UINT16 uiCascGrid_idx)//calc_line_coef()
{
    UINT16 uiIndex, uiIndex2;
    float fXp[17], fYp[17];
    float fH[19], fYd[19];
    float fA[19], fB[19], fC[19], fD[19], fXk[17];

    // horizontal line
    for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
    {
        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            fXp[uiIndex] = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2].fX;
            fYp[uiIndex] = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2].fY;
        }

        for(uiIndex = 2; uiIndex < 18; uiIndex++)
        {
            fH[uiIndex]  = fXp[uiIndex - 1] - fXp[uiIndex - 2];
            fYd[uiIndex] = (fYp[uiIndex - 1] - fYp[uiIndex - 2]) / fH[uiIndex];
        }

        // dummy
        fH[0] = fH[2];
        fH[1] = fH[2];
        fH[18] = fH[17];
        fYd[0] = fYd[2];
        fYd[1] = fYd[2];
        fYd[18] = fYd[17];

        for(uiIndex = 1; uiIndex < 18; uiIndex++)
        {
            fA[uiIndex] = 2 * (fH[uiIndex] + fH[uiIndex + 1]);
            fB[uiIndex] = fH[uiIndex + 1];
            fC[uiIndex] = fH[uiIndex];
            fD[uiIndex] = 3 * (fYd[uiIndex + 1] - fYd[uiIndex]);
        }

        // dummy
        fA[0] = fA[1];
        fA[18] = fA[17];
        fB[0] = fB[1];
        fB[18] = fB[17];
        fC[0] = fC[1];
        fC[18] = fC[17];
        fD[0] = fD[1];
        fD[18] = fD[17];

        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            fXk[uiIndex] = (fA[uiIndex] * fA[uiIndex + 2] * fD[uiIndex + 1] - fA[uiIndex + 2] *
                fC[uiIndex + 1] * fD[uiIndex] - fA[uiIndex] * fB[uiIndex + 1] * fD[uiIndex + 2]) /
                (fA[uiIndex] * fA[uiIndex + 1] * fA[uiIndex + 2] - fA[uiIndex + 2] * fB[uiIndex] *
                fC[uiIndex + 1] - fA[uiIndex] * fB[uiIndex + 1] * fC[uiIndex + 2]);
        }

        for(uiIndex = 0; uiIndex < 16; uiIndex++)
        {
            m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiIndex2].fC = fXk[uiIndex];
            m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiIndex2].fD = (fXk[uiIndex + 1] - fXk[uiIndex]) / (3 * fH[uiIndex + 2]);
            m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiIndex2].fB = fYd[uiIndex + 2] - fH[uiIndex + 2] *
                    (m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiIndex2].fC +
                    m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiIndex2].fD * fH[uiIndex + 2]);
            m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiIndex2].fA = fYp[uiIndex];
        }
    }

    // vertical line
    for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
    {
        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            fXp[uiIndex] = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiIndex].fY;
            fYp[uiIndex] = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiIndex].fX;
        }

        for(uiIndex = 2; uiIndex < 18; uiIndex++)
        {
            fH[uiIndex]  = fXp[uiIndex - 1] - fXp[uiIndex - 2];
            fYd[uiIndex] = (fYp[uiIndex - 1] - fYp[uiIndex - 2]) / fH[uiIndex];
        }

        // dummy
        fH[0] = fH[2];
        fH[1] = fH[2];
        fH[18] = fH[17];
        fYd[0] = fYd[2];
        fYd[1] = fYd[2];
        fYd[18] = fYd[17];

        for(uiIndex = 1; uiIndex < 18; uiIndex++)
        {
            fA[uiIndex] = 2 * (fH[uiIndex] + fH[uiIndex + 1]);
            fB[uiIndex] = fH[uiIndex + 1];
            fC[uiIndex] = fH[uiIndex];
            fD[uiIndex] = 3 * (fYd[uiIndex + 1] - fYd[uiIndex]);
        }

        // dummy
        fA[0] = fA[1];
        fA[18] = fA[17];
        fB[0] = fB[1];
        fB[18] = fB[17];
        fC[0] = fC[1];
        fC[18] = fC[17];
        fD[0] = fD[1];
        fD[18] = fD[17];

        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            fXk[uiIndex] = (fA[uiIndex] * fA[uiIndex + 2] * fD[uiIndex + 1] - fA[uiIndex + 2] *
                fC[uiIndex + 1] * fD[uiIndex] - fA[uiIndex] * fB[uiIndex + 1] * fD[uiIndex + 2]) /
                (fA[uiIndex] * fA[uiIndex + 1] * fA[uiIndex + 2] - fA[uiIndex + 2] * fB[uiIndex] *
                fC[uiIndex + 1] - fA[uiIndex] * fB[uiIndex + 1] * fC[uiIndex + 2]);
        }

        for(uiIndex = 0; uiIndex < 16; uiIndex++)
        {
            m_sDrvWarpingInfo.saWarpVerLine[uiIndex2][uiIndex].fC = fXk[uiIndex];
            m_sDrvWarpingInfo.saWarpVerLine[uiIndex2][uiIndex].fD = (fXk[uiIndex + 1] - fXk[uiIndex]) / (3 * fH[uiIndex + 2]);
            m_sDrvWarpingInfo.saWarpVerLine[uiIndex2][uiIndex].fB = fYd[uiIndex + 2] - fH[uiIndex + 2] *
                (m_sDrvWarpingInfo.saWarpVerLine[uiIndex2][uiIndex].fC +
                m_sDrvWarpingInfo.saWarpVerLine[uiIndex2][uiIndex].fD * fH[uiIndex + 2]);
            m_sDrvWarpingInfo.saWarpVerLine[uiIndex2][uiIndex].fA = fYp[uiIndex];
        }
    }
}


static void dvC789_CalVLineCoef(UINT16 uiVLine)//calc_v_line_coef()
{
    UINT16 uiIndex;
    float fXp[17], fYp[17];
    float fH[19], fYd[19];
    float fA[19], fB[19], fC[19], fD[19], fXk[17];

    // vertical line
    for(uiIndex = 0; uiIndex < m_sDrvWarpingInfo.uiNumVGrid; uiIndex++)
    {
        fXp[uiIndex] = m_sDrvWarpingInfo.saCalPoint[uiVLine][uiIndex].fY;
        fYp[uiIndex] = m_sDrvWarpingInfo.saCalPoint[uiVLine][uiIndex].fX;
    }

    for(uiIndex = 2; uiIndex < m_sDrvWarpingInfo.uiNumVGrid_P1; uiIndex++)
    {
        fH[uiIndex]  = fXp[uiIndex - 1] - fXp[uiIndex - 2];
        fYd[uiIndex] = (fYp[uiIndex - 1] - fYp[uiIndex - 2]) / fH[uiIndex];
    }

    // dummy
    fH[0] = fH[2];
    fH[1] = fH[2];
    fH[m_sDrvWarpingInfo.uiNumVGrid_P1] = fH[m_sDrvWarpingInfo.uiNumVGrid];
    fYd[0] = fYd[2];
    fYd[1] = fYd[2];
    fYd[m_sDrvWarpingInfo.uiNumVGrid_P1] = fYd[m_sDrvWarpingInfo.uiNumVGrid];

    for(uiIndex = 1; uiIndex < m_sDrvWarpingInfo.uiNumVGrid_P1; uiIndex++)
    {
        fA[uiIndex] = 2 * (fH[uiIndex] + fH[uiIndex + 1]);
        fB[uiIndex] = fH[uiIndex + 1];
        fC[uiIndex] = fH[uiIndex];
        fD[uiIndex] = 3 * (fYd[uiIndex + 1] - fYd[uiIndex]);
    }

    // dummy
    fA[0] = fA[1];
    fA[m_sDrvWarpingInfo.uiNumVGrid_P1] = fA[m_sDrvWarpingInfo.uiNumVGrid];
    fB[0] = fB[1];
    fB[m_sDrvWarpingInfo.uiNumVGrid_P1] = fB[m_sDrvWarpingInfo.uiNumVGrid];
    fC[0] = fC[1];
    fC[m_sDrvWarpingInfo.uiNumVGrid_P1] = fC[m_sDrvWarpingInfo.uiNumVGrid];
    fD[0] = fD[1];
    fD[m_sDrvWarpingInfo.uiNumVGrid_P1] = fD[m_sDrvWarpingInfo.uiNumVGrid];

    for(uiIndex = 0; uiIndex < m_sDrvWarpingInfo.uiNumVGrid; uiIndex++)
    {
        fXk[uiIndex] = (fA[uiIndex] * fA[uiIndex + 2] * fD[uiIndex + 1] -
                fA[uiIndex + 2] * fC[uiIndex + 1] * fD[uiIndex] - fA[uiIndex] * fB[uiIndex + 1] *
                fD[uiIndex + 2]) / (fA[uiIndex] * fA[uiIndex + 1] * fA[uiIndex + 2] -
                fA[uiIndex + 2] * fB[uiIndex] * fC[uiIndex + 1] - fA[uiIndex] *
                fB[uiIndex + 1] * fC[uiIndex + 2]);
    }

    for(uiIndex = 0; uiIndex < m_sDrvWarpingInfo.uiNumVGrid_M1; uiIndex++)
    {
        m_sDrvWarpingInfo.saWarpVerLine[uiVLine][uiIndex].fC = fXk[uiIndex];
        m_sDrvWarpingInfo.saWarpVerLine[uiVLine][uiIndex].fD = (fXk[uiIndex + 1] - fXk[uiIndex]) / (3 * fH[uiIndex + 2]);
        m_sDrvWarpingInfo.saWarpVerLine[uiVLine][uiIndex].fB = fYd[uiIndex + 2] - fH[uiIndex + 2] *
            (m_sDrvWarpingInfo.saWarpVerLine[uiVLine][uiIndex].fC +
            m_sDrvWarpingInfo.saWarpVerLine[uiVLine][uiIndex].fD * fH[uiIndex + 2]);
        m_sDrvWarpingInfo.saWarpVerLine[uiVLine][uiIndex].fA = fYp[uiIndex];
    }
}

static void dvC789_CalHLineCoef(UINT16 uiVLine)//calc_h_line_coef()
{
    UINT16 uiIndex;
    float fXp[17], fYp[17];
    float fH[19], fYd[19];
    float fA[19], fB[19], fC[19], fD[19], fXk[17];

    // horizontal line
    for(uiIndex = 0; uiIndex < m_sDrvWarpingInfo.uiNumHGrid; uiIndex++)
    {
        //While Grid Point Position
        //uiNumHGrid record current how many grid point in H. Ex. 3x3 then uiNumHGrid = 3, 17x17 then uiNumHGrid = 17
        fXp[uiIndex] = m_sDrvWarpingInfo.saCalPoint[uiIndex][uiVLine].fX;
        fYp[uiIndex] = m_sDrvWarpingInfo.saCalPoint[uiIndex][uiVLine].fY;
    }

    for(uiIndex = 2; uiIndex < m_sDrvWarpingInfo.uiNumHGrid_P1; uiIndex++)
    {
        //Get 2 white grid point width
        fH[uiIndex]  = fXp[uiIndex - 1] - fXp[uiIndex - 2];
        //Get line slop ratio
        fYd[uiIndex] = (fYp[uiIndex - 1] - fYp[uiIndex - 2]) / fH[uiIndex];
    }

    // dummy
    fH[0] = fH[2];
    fH[1] = fH[2];
    fH[m_sDrvWarpingInfo.uiNumHGrid_P1] = fH[m_sDrvWarpingInfo.uiNumHGrid];
    fYd[0] = fYd[2];
    fYd[1] = fYd[2];
    fYd[m_sDrvWarpingInfo.uiNumHGrid_P1] = fYd[m_sDrvWarpingInfo.uiNumHGrid];

    for(uiIndex = 1; uiIndex < m_sDrvWarpingInfo.uiNumHGrid_P1; uiIndex++)
    {
        fA[uiIndex] = 2 * (fH[uiIndex] + fH[uiIndex + 1]);
        fB[uiIndex] = fH[uiIndex + 1];
        fC[uiIndex] = fH[uiIndex];
        fD[uiIndex] = 3 * (fYd[uiIndex + 1] - fYd[uiIndex]);
    }

    // dummy
    fA[0] = fA[1];
    fA[m_sDrvWarpingInfo.uiNumHGrid_P1] = fA[m_sDrvWarpingInfo.uiNumHGrid];
    fB[0] = fB[1];
    fB[m_sDrvWarpingInfo.uiNumHGrid_P1] = fB[m_sDrvWarpingInfo.uiNumHGrid];
    fC[0] = fC[1];
    fC[m_sDrvWarpingInfo.uiNumHGrid_P1] = fC[m_sDrvWarpingInfo.uiNumHGrid];
    fD[0] = fD[1];
    fD[m_sDrvWarpingInfo.uiNumHGrid_P1] = fD[m_sDrvWarpingInfo.uiNumHGrid];

    for(uiIndex = 0; uiIndex < m_sDrvWarpingInfo.uiNumHGrid; uiIndex++)
    {
        fXk[uiIndex] = (fA[uiIndex] * fA[uiIndex + 2] *
            fD[uiIndex + 1] - fA[uiIndex + 2] * fC[uiIndex + 1] *
            fD[uiIndex] - fA[uiIndex] * fB[uiIndex + 1] * fD[uiIndex + 2]) /
            (fA[uiIndex] * fA[uiIndex + 1] * fA[uiIndex + 2] - fA[uiIndex + 2] *
            fB[uiIndex] * fC[uiIndex + 1] - fA[uiIndex] * fB[uiIndex + 1] * fC[uiIndex + 2]);
    }

    for(uiIndex = 0; uiIndex <  m_sDrvWarpingInfo.uiNumHGrid_M1; uiIndex++)
    {
        m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiVLine].fC = fXk[uiIndex];
        m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiVLine].fD = (fXk[uiIndex + 1] - fXk[uiIndex]) / (3 * fH[uiIndex + 2]);
        m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiVLine].fB = fYd[uiIndex + 2] - fH[uiIndex + 2] *
            (m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiVLine].fC +
            m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiVLine].fD * fH[uiIndex + 2]);
        m_sDrvWarpingInfo.saWarpHorLine[uiIndex][uiVLine].fA = fYp[uiIndex];
    }
}


static void dvC789_CalPJConv(UINT16 uiCascGrid_idx)// calc_pj_conv()
{
    float fPx[4], fPy[4];
    float fD23, fD24, fD34;
    UINT16 uiIndex, uiIndex2;
    float fD;

    fPx[1] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][0].fX -
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX) / (float)m_sDrvWarpingInfo.uiPanelHW;
    fPy[1] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][0].fY -
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY) / (float)m_sDrvWarpingInfo.uiPanelVW;
    fPx[2] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][16].fX -
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX) / (float)m_sDrvWarpingInfo.uiPanelHW;
    fPy[2] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][16].fY -
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY) / (float)m_sDrvWarpingInfo.uiPanelVW;
    fPx[3] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][16].fX -
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX) / (float)m_sDrvWarpingInfo.uiPanelHW;
    fPy[3] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][16].fY -
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY) / (float)m_sDrvWarpingInfo.uiPanelVW;

    fD23 = fPx[1] * fPy[2] - fPx[2] * fPy[1];
    fD24 = fPx[1] * fPy[3] - fPx[3] * fPy[1];
    fD34 = fPx[2] * fPy[3] - fPx[3] * fPy[2];

    m_sDrvWarpingInfo.faPjcA[0] = fD24 - fD23;
    m_sDrvWarpingInfo.faPjcB[0] = -fD34 - fD23;
    m_sDrvWarpingInfo.faPjcC[0] = fD34 + fD23 - fD24;

    m_sDrvWarpingInfo.faPjcA[1] = fPx[1] * fD34;
    m_sDrvWarpingInfo.faPjcB[1] = fPx[2] * (-fD24);

    m_sDrvWarpingInfo.faPjcA[2] = fPy[1] * fD34;
    m_sDrvWarpingInfo.faPjcB[2] = fPy[2] * (-fD24);

    // h
    uiIndex2 = 0;

    for(uiIndex = 1; uiIndex < 16; uiIndex++)
    {
        if(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] == TRUE)
        {
            uiIndex2 = uiIndex2 + 1;
            fD = (float)uiIndex2 / (float)m_sDrvWarpingInfo.uiNumHGrid_M1;

            // TL ~ TR
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][0].fX =
                    (m_sDrvWarpingInfo.faPjcA[1] * fD) / (m_sDrvWarpingInfo.faPjcA[0] * fD +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelHW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX;
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][0].fY =
                    (m_sDrvWarpingInfo.faPjcA[2] * fD) / (m_sDrvWarpingInfo.faPjcA[0] * fD +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelVW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY;

            // BhL ~ BhR
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][16].fX =
                    (m_sDrvWarpingInfo.faPjcA[1] * fD + m_sDrvWarpingInfo.faPjcB[1]) /
                    (m_sDrvWarpingInfo.faPjcA[0] * fD + m_sDrvWarpingInfo.faPjcB[0] +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelHW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX;
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][16].fY =
                    (m_sDrvWarpingInfo.faPjcA[2] * fD + m_sDrvWarpingInfo.faPjcB[2]) /
                    (m_sDrvWarpingInfo.faPjcA[0] * fD + m_sDrvWarpingInfo.faPjcB[0] +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelVW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY;
        }
    }

    // v
    uiIndex2 = 0;

    for(uiIndex = 1; uiIndex < 16; uiIndex++)
    {
        if(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex] == TRUE)
        {
            uiIndex2 = uiIndex2 + 1;
            fD = (float)uiIndex2 / (float)m_sDrvWarpingInfo.uiNumVGrid_M1;

            // TL ~ BL
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][uiIndex].fX =
                    (m_sDrvWarpingInfo.faPjcB[1] * fD) / (m_sDrvWarpingInfo.faPjcB[0] * fD +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelHW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX;
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][uiIndex].fY =
                    (m_sDrvWarpingInfo.faPjcB[2] * fD) / (m_sDrvWarpingInfo.faPjcB[0] * fD +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelVW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY;

            // TR ~ BR
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][uiIndex].fX =
                    (m_sDrvWarpingInfo.faPjcA[1] + m_sDrvWarpingInfo.faPjcB[1] * fD) /
                    (m_sDrvWarpingInfo.faPjcA[0] + m_sDrvWarpingInfo.faPjcB[0] * fD +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelHW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX;
            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][uiIndex].fY =
                    (m_sDrvWarpingInfo.faPjcA[2] + m_sDrvWarpingInfo.faPjcB[2] * fD) /
                    (m_sDrvWarpingInfo.faPjcA[0] + m_sDrvWarpingInfo.faPjcB[0] * fD +
                     m_sDrvWarpingInfo.faPjcC[0]) * (float)m_sDrvWarpingInfo.uiPanelVW +
                     m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY;
        }
    }
}


static void dvC789_CascadeBiLinear(void)
{
    INT16 iX, iY;
    INT16 iX0, iX1, iY0, iY1;
    float fA, fB;
    float fIxNew, fIyNew;
    float fIxOld00, fIxOld10, fIxOld01, fIxOld11;
    float fIyOld00, fIyOld10, fIyOld01, fIyOld11;
    float fDiff0, fDiff1;

    UINT16 uiHGridMax = DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW + DEF_WP_OUT_HGRD;
    UINT16 uiVGridMax = DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW + DEF_WP_OUT_VGRD;

    for(iY = uiVGridMax; iY >= 0; iY--)
    {
        for(iX = uiHGridMax; iX >= 0; iX--)
        {

            //			ix_new = GV_PX[Def_CASC_NEW_TBL][x][y] / (float)DEF_WP_SPACE + DEF_WP_OUT_HGRD;
            //			iy_new = GV_PY[Def_CASC_NEW_TBL][x][y] / (float)DEF_WP_SPACE + DEF_WP_OUT_VGRD;
            fIxNew = m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][iX][iY] /
                        ((float)DEF_WP_SPACE * (float)m_sDrvWarpingInfo.uiPanelHW/*PM_IACT_HW*/ /
                        (float)m_sDrvWarpingInfo.uiPanelHW/*PS_WP_HW*/) + DEF_WP_OUT_HGRD;
            fIyNew = m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][iX][iY] /
                        ((float)DEF_WP_SPACE * (float)m_sDrvWarpingInfo.uiPanelVW/*PM_IACT_VW*/ /
                        (float)m_sDrvWarpingInfo.uiPanelVW)/*PS_WP_VW*/ + DEF_WP_OUT_VGRD;

            // X-left
            iX0 = (INT16)(fIxNew);
            iX0 = (iX0 < 0) ? 0 : (iX0 > (uiHGridMax - 1)) ? (uiHGridMax - 1) : iX0;
            // X-right
            iX1 = iX0 + 1;
            // Y-top
            iY0 = (INT16)(fIyNew);
            iY0 = (iY0 < 0) ? 0 : (iY0 > (uiVGridMax - 1)) ? (uiVGridMax - 1) : iY0;
            // Y-bottom
            iY1 = iY0 + 1;

            // sub-pixel
            fA  = fIxNew - iX0;
            fB  = fIyNew - iY0;

            fIxOld00 = m_sDrvWarpingInfo.faPosX[DEF_CASC_OLD_TBL][iX0][iY0];
            fIxOld10 = m_sDrvWarpingInfo.faPosX[DEF_CASC_OLD_TBL][iX1][iY0];
            fIxOld01 = m_sDrvWarpingInfo.faPosX[DEF_CASC_OLD_TBL][iX0][iY1];
            fIxOld11 = m_sDrvWarpingInfo.faPosX[DEF_CASC_OLD_TBL][iX1][iY1];
            fIyOld00 = m_sDrvWarpingInfo.faPosY[DEF_CASC_OLD_TBL][iX0][iY0];
            fIyOld10 = m_sDrvWarpingInfo.faPosY[DEF_CASC_OLD_TBL][iX1][iY0];
            fIyOld01 = m_sDrvWarpingInfo.faPosY[DEF_CASC_OLD_TBL][iX0][iY1];
            fIyOld11 = m_sDrvWarpingInfo.faPosY[DEF_CASC_OLD_TBL][iX1][iY1];

            if((fA < -10) || (fA > 10))
            {
                fDiff0 = fabsf(fIxOld10 - fIxOld00);
                fDiff1 = fabsf(fIxOld11 - fIxOld01);

                if(fB < -10)
                {
                    m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][iX][iY] = fIxOld00 + fDiff0 * fA;
                }
                else if(fB > 10)
                {
                    m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][iX][iY] = fIxOld01 + fDiff1 * fA;
                }
                else
                {
                    m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][iX][iY] = fIxOld00 + ((fDiff1 - fDiff0) * fB + fDiff0) * fA;
                }
            }
            else  // bi-linear
            {
                m_sDrvWarpingInfo.faPosX[DEF_CASC_NEW_TBL][iX][iY] = (fIxOld10 * fA + fIxOld00 * (1 - fA)) * (1 - fB) + (fIxOld11 * fA + fIxOld01 * (1 - fA)) * fB;
            }

            if((fB < -10) || (fB > 10))
            {
                fDiff0 = fabsf(fIyOld01 - fIyOld00);
                fDiff1 = fabsf(fIyOld11 - fIyOld10);

                if(fA < -10)
                {
                    m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][iX][iY] = fIyOld00 + fDiff0 * fB;
                }
                else if(fA > 10)
                {
                    m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][iX][iY] = fIyOld10 + fDiff1 * fB;
                }
                else
                {
                    m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][iX][iY] = fIyOld00 + ((fDiff1 - fDiff0) * fA + fDiff0) * fB;
                }
            }
            else  // bi-linear
            {
                m_sDrvWarpingInfo.faPosY[DEF_CASC_NEW_TBL][iX][iY] = (fIyOld01 * fB + fIyOld00 * (1 - fB)) * (1 - fA) + (fIyOld11 * fB + fIyOld10 * (1 - fB)) * fA;
            }
        }
    }

}

static void dvC789_CalWarpTable(UINT16 uiCascGrid_idx, UINT16 uiCascTable_idx, eC789_WARP_MODE eWarpMode, BOOL bInternalWarp)
{
    float fX, fY;
    UINT16 uiIndex, uiIndex2, uiIndex3;
    INT16 iH, iV;
    UINT16 uiW;
    float fVerLine_X[17];
    float fHorLine_Y[DEF_HW_MAX_GRD][17];
    float fX0, fY0;
    UINT16 uiArea;
    float fDx, fDy;
    float fBi, fCu;
    //	float d1;
    float fCalPoint_Hscl[2][15];
    float fCalPoint_Vscl[2][15];

    if(eWarpMode == eC789_WARP_MODE_2x2)    // 2x2 grid mode
    {
        dvC789_CalPJConv(uiCascGrid_idx);
    }


    if(m_sDrvWarpingInfo.uiNumHGrid != 17)
    {
        iV = 0;

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            if(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] == TRUE)
            {
                iH = 0;

                for(uiIndex = 0; uiIndex < 17; uiIndex++)
                {
                    // If 3x3 then uiIndex will be 0, 8, 16
                    if(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] == TRUE)
                    {
                        //The points which is going to caculate the interpolation
                        m_sDrvWarpingInfo.saCalPoint[iH][iV] = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2];
						iH = iH + 1;
                    }
                }

                dvC789_CalHLineCoef(iV);

                iV = iV + 1;
            }
        }


        iV = 0;

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            if(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] == TRUE)
            {
                uiArea = 0;
                iH = 0;

                for(uiIndex = 1; uiIndex < 17; uiIndex++)
                {
                    uiW = 1;

                    while(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] == FALSE)
                    {
                        uiW = uiW + 1;
                        uiIndex = uiIndex + 1;
                    }

                    for(uiIndex3 = 1; uiIndex3 < uiW; uiIndex3++)
                    {
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea + uiIndex3][uiIndex2].fX =
                                    (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2].fX * uiIndex3 +
                                    m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea][uiIndex2].fX * (uiW - uiIndex3)) / uiW;
                        fDx = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea + uiIndex3][uiIndex2].fX -
                                    m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea][uiIndex2].fX;
                        fBi = fDx * fDx;
                        fCu = fBi * fDx;
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea + uiIndex3][uiIndex2].fY =
                                m_sDrvWarpingInfo.saWarpHorLine[iH][iV].fA +
                                m_sDrvWarpingInfo.saWarpHorLine[iH][iV].fB * fDx +
                                m_sDrvWarpingInfo.saWarpHorLine[iH][iV].fC * fBi +
                                m_sDrvWarpingInfo.saWarpHorLine[iH][iV].fD * fCu;

                    }

                    uiArea = uiIndex;
                    iH = iH + 1;
                }

                iV = iV + 1;
            }
        }
    }


    if(m_sDrvWarpingInfo.uiNumVGrid != 17)
    {
        iH = 0;

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            if(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex2] == TRUE)
            {
                iV = 0;

                for(uiIndex = 0; uiIndex < 17; uiIndex++)
                {
                    if(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex] == TRUE)
                    {
                        m_sDrvWarpingInfo.saCalPoint[iH][iV] = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiIndex];
                        iV = iV + 1;
                    }
                }

                dvC789_CalVLineCoef(iH);

                iH = iH + 1;
            }
        }

        iH = 0;

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            if(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex2] == TRUE)
            {
                uiArea = 0;
                iV = 0;

                for(uiIndex = 1; uiIndex < 17; uiIndex++)
                {
                    uiW = 1;

                    while(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex] == FALSE)
                    {
                        uiW = uiW + 1;
                        uiIndex = uiIndex + 1;
                    }

                    for(uiIndex3 = 1; uiIndex3 < uiW; uiIndex3++)
                    {
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea + uiIndex3].fY =
                            (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiIndex].fY * uiIndex3 +
                                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea].fY * (uiW - uiIndex3)) / uiW;
                        fDy = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea + uiIndex3].fY -
                                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea].fY;
                        fBi = fDy * fDy;
                        fCu = fBi * fDy;
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea + uiIndex3].fX =
                                        m_sDrvWarpingInfo.saWarpVerLine[iH][iV].fA +
                                        m_sDrvWarpingInfo.saWarpVerLine[iH][iV].fB * fDy +
                                        m_sDrvWarpingInfo.saWarpVerLine[iH][iV].fC * fBi +
                                        m_sDrvWarpingInfo.saWarpVerLine[iH][iV].fD * fCu;

                    }

                    uiArea = uiIndex;
                    iV = iV + 1;
                }

                iH = iH + 1;
            }
        }
    }

    if((eWarpMode != eC789_WARP_MODE_2x2) && (bInternalWarp == TRUE) &&
            ((m_sDrvWarpingInfo.uiNumHGrid != 17) || (m_sDrvWarpingInfo.uiNumVGrid != 17)))
    {
        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            uiArea = 0;

            if(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex2] == FALSE)
            {
                for(uiIndex = 1; uiIndex < 17; uiIndex++)
                {
                    uiW = 1;

                    while(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex] == FALSE)
                    {
                        uiW = uiW + 1;
                        uiIndex = uiIndex + 1;
                    }

                    for(uiIndex3 = 1; uiIndex3 < uiW; uiIndex3++)
                    {
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea + uiIndex3][uiIndex2].fX =
                            (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2].fX * uiIndex3 +
                                    m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea][uiIndex2].fX * (uiW - uiIndex3)) / uiW;
                    }

                    uiArea = uiIndex;
                }
            }
        }

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            uiArea = 0;

            if(m_sDrvWarpingInfo.baHGridCalEnable[uiIndex2] == FALSE)
            {
                for(uiIndex = 1; uiIndex < 17; uiIndex++)
                {
                    uiW = 1;

                    while(m_sDrvWarpingInfo.baVGridCalEnable[uiIndex] == FALSE)
                    {
                        uiW = uiW + 1;
                        uiIndex = uiIndex + 1;
                    }

                    for(uiIndex3 = 1; uiIndex3 < uiW; uiIndex3++)
                    {
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea + uiIndex3].fY =
                                (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiIndex].fY * uiIndex3 +
                                 m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex2][uiArea].fY * (uiW - uiIndex3)) / uiW;
                    }

                    uiArea = uiIndex;
                }
            }
        }
    }
    else if((eWarpMode == eC789_WARP_MODE_2x2) || (bInternalWarp == FALSE))
    {
        fDx = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][0].fX - m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX;

        for(uiIndex = 1; uiIndex < 16; uiIndex++)
        {
            fCalPoint_Hscl[0][uiIndex - 1] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][0].fX -
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fX) / fDx;
        }

        fDx = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][16].fX -
                    m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][16].fX;

        for(uiIndex = 1; uiIndex < 16; uiIndex++)
        {
            fCalPoint_Hscl[1][uiIndex - 1] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][16].fX -
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][16].fX) / fDx;
        }

        fDy = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][16].fY -
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY;

        for(uiIndex = 1; uiIndex < 16; uiIndex++)
        {
            fCalPoint_Vscl[0][uiIndex - 1] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][uiIndex].fY -
                            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][0].fY) / fDy;
        }

        fDy = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][16].fY - m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][0].fY;

        for(uiIndex = 1; uiIndex < 16; uiIndex++)
        {
            fCalPoint_Vscl[1][uiIndex - 1] = (m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][uiIndex].fY -
                            m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][0].fY) / fDy;
        }

        // internal cursor
        // x
        for(uiIndex2 = 1; uiIndex2 < 16; uiIndex2++)
        {
            for(uiIndex = 1; uiIndex < 16; uiIndex++)
            {
                fDx = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][16][uiIndex2].fX -
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][uiIndex2].fX;
                fDy = m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][16].fY -
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][0].fY;

                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2].fX =
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][0][uiIndex2].fX +
                        fDx * ((float)(16 - uiIndex2) * fCalPoint_Hscl[0][uiIndex - 1] +
                        uiIndex2 * fCalPoint_Hscl[1][uiIndex - 1]) / (float)16;
                m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiIndex2].fY =
                        m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][0].fY +
                        fDy * ((float)(16 - uiIndex) * fCalPoint_Vscl[0][uiIndex2 - 1] +
                        uiIndex * fCalPoint_Vscl[1][uiIndex2 - 1]) / (float)16;
            }
        }
    }

    // spline
    dvC789_CalLineCoef(uiCascGrid_idx);//calc_line_coef(ucCascGrid_idx);

    // table
    for(uiIndex = 0; uiIndex <= m_sDrvWarpingInfo.uiPixelGridHW; uiIndex++)
    {
        fX0 = (float)(uiIndex << DEF_WP_SPACE_BIT);

        for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
        {
            for(uiArea = 0; uiArea < 16; uiArea++)
            {
                if((fX0 < m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea + 1][uiIndex2].fX) || (uiArea == 15))
                {
                    fDx = fX0 - m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiArea][uiIndex2].fX;
                    fBi = fDx * fDx;
                    fCu = fBi * fDx;
                    fHorLine_Y[uiIndex][uiIndex2] =
                                m_sDrvWarpingInfo.saWarpHorLine[uiArea][uiIndex2].fA +
                                m_sDrvWarpingInfo.saWarpHorLine[uiArea][uiIndex2].fB * fDx +
                                m_sDrvWarpingInfo.saWarpHorLine[uiArea][uiIndex2].fC * fBi +
                                m_sDrvWarpingInfo.saWarpHorLine[uiArea][uiIndex2].fD * fCu;

                    break;
                }
            }
        }

        dvC789_CalmvYCoef(fHorLine_Y[uiIndex], uiIndex); //calc_mvy_coef(fHorLine_Y[uiIndex], uiIndex)
    }


    //	for( j = PS_WP_VW_GRD; j >= 0; j-- ) {
    for(iV = (DEF_WP_OUT_VGRD + m_sDrvWarpingInfo.uiPixelGridVW + DEF_WP_OUT_VGRD); iV >= 0; iV--)
    {
        //		y0 = (double)( j << DEF_WP_SPACE_BIT );
        fY0 = (float)((iV - DEF_WP_OUT_VGRD) * DEF_WP_SPACE);

        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            for(uiArea = 0; uiArea < 16; uiArea++)
            {
                if((fY0 < m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiArea + 1].fY) || (uiArea == 15))
                {
                    fDy = fY0 - m_sDrvWarpingInfo.saGridPos[uiCascGrid_idx][uiIndex][uiArea].fY;
                    fBi = fDy * fDy;
                    fCu = fBi * fDy;
                    fVerLine_X[uiIndex] =
                                m_sDrvWarpingInfo.saWarpVerLine[uiIndex][uiArea].fA +
                                m_sDrvWarpingInfo.saWarpVerLine[uiIndex][uiArea].fB * fDy +
                                m_sDrvWarpingInfo.saWarpVerLine[uiIndex][uiArea].fC * fBi +
                                m_sDrvWarpingInfo.saWarpVerLine[uiIndex][uiArea].fD * fCu;
                    break;
                }
            }
        }


        dvC789_CalmvXCoef(fVerLine_X);//calc_mvx_coef(fVerLine_X);

        //		for( i = PS_WP_HW_GRD; i >= 0; i-- ) {
        for(iH = (DEF_WP_OUT_HGRD + m_sDrvWarpingInfo.uiPixelGridHW + DEF_WP_OUT_HGRD); iH >= 0; iH--)
        {
            //			x0 = (double)( i << DEF_WP_SPACE_BIT );
            fX0 = (float)((iH - DEF_WP_OUT_HGRD) * DEF_WP_SPACE);

            for(uiIndex3 = 1; uiIndex3 < 17; uiIndex3++)
            {
                if((fX0 < fVerLine_X[uiIndex3]) || (uiIndex3 == 16))
                {
                    uiArea = uiIndex3 - 1;
                    break;
                }
            }

            fDx = fX0 - fVerLine_X[uiArea];

            fBi = fDx * fDx;
            fCu = fBi * fDx;
            fX = m_sDrvWarpingInfo.saWarpHorLineMove[uiArea].fA +
                 m_sDrvWarpingInfo.saWarpHorLineMove[uiArea].fB * fDx +
                 m_sDrvWarpingInfo.saWarpHorLineMove[uiArea].fC * fBi +
                 m_sDrvWarpingInfo.saWarpHorLineMove[uiArea].fD * fCu;

            //				x = x + (double)( i << DEF_WP_SPACE_BIT );
            fX = fX + fX0;
            //			}
            /*
            x = (x > (PS_WP_HW+128)) ? (PS_WP_HW+128) : (x < -2048) ? -2048 : x;
            GV_WARP_TABLE_X[i][j] = (int)(x * 8 + 0.5);
            */



            m_sDrvWarpingInfo.faPosX[uiCascTable_idx][iH][iV] = fX;

            // y
            /*
            fDy = hor_line_y[i][1] - hor_line_y[i][0];
            d1 = hor_line_y[i][16] - hor_line_y[i][15];
            if( (y0 - hor_line_y[i][0]) < (-fDy) ) {
            	y = -2048;
            }
            else if( (y0 - hor_line_y[i][16]) > d1 ) {
            	y = 6128;
            }
            else {
            */
            uiIndex = iH - DEF_WP_OUT_HGRD;
            uiIndex = (uiIndex <= 0) ? 0 : (uiIndex > m_sDrvWarpingInfo.uiPixelGridHW) ? m_sDrvWarpingInfo.uiPixelGridHW : uiIndex;

            for(uiIndex3 = 1; uiIndex3 < 17; uiIndex3++)
            {
                if((fY0 < fHorLine_Y[uiIndex][uiIndex3]) || (uiIndex3 == 16))
                {
                    uiArea = uiIndex3 - 1;
                    break;
                }
            }



            fDy = fY0 - fHorLine_Y[uiIndex][uiArea];

            fBi = fDy * fDy;
            fCu = fBi * fDy;
            fY = m_sDrvWarpingInfo.saWarpVerLineMove[uiIndex][uiArea].fA +
                 m_sDrvWarpingInfo.saWarpVerLineMove[uiIndex][uiArea].fB * fDy +
                 m_sDrvWarpingInfo.saWarpVerLineMove[uiIndex][uiArea].fC * fBi +
                 m_sDrvWarpingInfo.saWarpVerLineMove[uiIndex][uiArea].fD * fCu;
            //				y = y + (double)( j << DEF_WP_SPACE_BIT );

            fY = fY + fY0;

            //			}
            /*
            y = (y > (PS_WP_VW+128)) ? (PS_WP_VW+128) : (y < -2048) ? -2048 : y;
            GV_WARP_TABLE_Y[i][j] = (int)(y * 8 + 0.5);
            */
            m_sDrvWarpingInfo.faPosY[uiCascTable_idx][iH][iV] = fY;
        }
    }

    /*
    write_wp_table();

    return err;
    */

}


static eC789_WARP_ERROR dvC789_Warp(eC789_WARP_MODE eWarpMode, BOOL bInternalWarp)
{
    INT8 cCascade;// cascade
   // eC790_WARP_MODE eTmpWarpMode;
    eC789_WARP_ERROR eWarpErr = eC789_WARP_OK;

    //eTmpWarpMode = eWarpMode;
    dvC789_WarpModeChange(eWarpMode);
    dvC789_CalWarpTable(DEF_NUM_CASC - 1, DEF_CASC_NEW_TBL, eWarpMode, bInternalWarp);

    dvC789_WarpModeChange(eC789_WARP_MODE_17x17);

    //TODO
    for(cCascade = (DEF_NUM_CASC - 2); cCascade >= 0; cCascade--)
    {
        dvC789_CalWarpTable(cCascade, DEF_CASC_OLD_TBL, eC789_WARP_MODE_17x17, TRUE);
        dvC789_CascadeBiLinear();
    }

    //m_sDrvWarpingInfo.eWarpMode = eTmpWarpMode;
    //dvC790_WarpModeChange(m_sDrvWarpingInfo.eWarpMode);
    dvC789_WarpModeChange(eWarpMode);


#if 0 //Larry mask
#if (DEF_WP_LIM_MODE == 1)
    // Check Limit
    eWarpErr = dvC790_CheckWarpLimit();
#endif
#endif /* 0 */

    if(eWarpErr == eC789_WARP_OK)
    {
        dvC789_WriteWarpTable();
    }

    return eWarpErr;
}


static eC789_WARP_ERROR dvC789_MoveCursor(float fdX, float fdY, UINT8 ucGridSelX, UINT8 ucGridSelY,
                eC789_WARP_MODE eWarpMode, BOOL bInternalGridShow)//move_cur
{
    eC789_WARP_ERROR eWarpErr = eC789_WARP_OK;
#if (DEF_WP_LIM_MODE == 1)
    UINT16 uiIndex, uiIndex2;
#endif

    if((fdX != 0) || (fdY != 0))
    {
        m_sDrvWarpingInfo.saGridPos[DEF_NUM_CASC - 1][ucGridSelX][ucGridSelY].fX =
                        m_sDrvWarpingInfo.saGridPos[DEF_NUM_CASC - 1][ucGridSelX][ucGridSelY].fX + fdX;

        m_sDrvWarpingInfo.saGridPos[DEF_NUM_CASC - 1][ucGridSelX][ucGridSelY].fY =
                         m_sDrvWarpingInfo.saGridPos[DEF_NUM_CASC - 1][ucGridSelX][ucGridSelY].fY + fdY;

    }

    if(eWarpErr == eC789_WARP_OK)
    {
        eWarpErr = dvC789_Warp(eWarpMode, bInternalGridShow);
#if (DEF_WP_LIM_MODE == 1)
        if(eWarpErr != eC789_WARP_OK)
        {
            for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
            {
                for(uiIndex = 0; uiIndex < 17; uiIndex++)
                {
                    m_sDrvWarpingInfo.saGridPos[DEF_NUM_CASC - 1][uiIndex][uiIndex2] = m_sDrvWarpingInfo.saPrevGridPos[uiIndex][uiIndex2];
                }
            }
        }
        else
        {
            for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
            {
                for(uiIndex = 0; uiIndex < 17; uiIndex++)
                {
                    m_sDrvWarpingInfo.saPrevGridPos[uiIndex][uiIndex2] = m_sDrvWarpingInfo.saGridPos[DEF_NUM_CASC - 1][uiIndex][uiIndex2];
                }
            }
        }

#endif
    }

    return eWarpErr;
}


eC789_WARP_ERROR dvC789_MoveWarp(eC789_WARP_DIRECTION eDir, float fGridMovePitch, UINT8 ucGridSelX, UINT8 ucGridSelY, eC789_WARP_MODE eWarpMode, BOOL bInternalGridShow)
{
    eC789_WARP_ERROR eErr = eC789_WARP_OK;

    if(m_sDrvWarpingInfo.bInit == FALSE)
        return eC789_WARP_ERR;


    switch(eDir)
    {
    case eC789_WARP_DIRECTION_UP:
        eErr = dvC789_MoveCursor(0, -fGridMovePitch,
                ucGridSelX, ucGridSelY, eWarpMode, bInternalGridShow);
        break;
    case eC789_WARP_DIRECTION_DOWN:
        eErr = dvC789_MoveCursor(0, fGridMovePitch,
                ucGridSelX, ucGridSelY, eWarpMode, bInternalGridShow);
        break;
    case eC789_WARP_DIRECTION_LEFT:
        eErr = dvC789_MoveCursor(-fGridMovePitch, 0,
                ucGridSelX, ucGridSelY, eWarpMode, bInternalGridShow);
        break;
    case eC789_WARP_DIRECTION_RIGHT:
        eErr = dvC789_MoveCursor(fGridMovePitch, 0,
                ucGridSelX, ucGridSelY, eWarpMode, bInternalGridShow);
        break;
    }

    return eErr;

}

void dvC789_InitWarpTable(void)
{
    UINT16 uiIndex, uiIndex2;
    UINT8 ucCascade;// cascade

    dvC789_WarpLight_PanelConfig(); //A70LV_Larry_0137

    for(uiIndex2 = 0; uiIndex2 < 17; uiIndex2++)
    {
        for(uiIndex = 0; uiIndex < 17; uiIndex++)
        {
            for(ucCascade = 0; ucCascade < DEF_NUM_CASC; ucCascade++)
            {
                m_sDrvWarpingInfo.saGridPos[ucCascade][uiIndex][uiIndex2].fX = m_sDrvWarpingInfo.faCursorDefOutX[uiIndex];
                m_sDrvWarpingInfo.saGridPos[ucCascade][uiIndex][uiIndex2].fY =  m_sDrvWarpingInfo.faCursorDefOutY[uiIndex2];
            }
        }
    }

    dvC789_MoveCursor(0, 0, 0, 0, eC789_WARP_MODE_2x2, FALSE);
}



