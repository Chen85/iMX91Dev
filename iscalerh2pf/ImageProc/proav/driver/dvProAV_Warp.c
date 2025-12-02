#include "dvProAV_Warp.h"
#ifdef QT_CPP
#include "dvProAV_WarpTable.h"
#else
#include "dvProAV.h"
#include "dvProAV_PixelShiftWith3d.h"
#endif

//define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
    #define DBMSG_ASSERT() ASSERT_ALWAYS()
#endif

#ifndef QT_CPP
static uint16 m_uiWarpingPanel = 0;
#else
//#define DEF_SAVE_WARP_TABLE
//#define DEF_SAVE_AUTO_FILTER_TABLE
#endif
static bool m_bWarpingbypass = 0;

typedef struct
{
    uint16  u16X;
    uint16  u16Y;
} sWARP_TABLE_DATA;

//-------------------------------------------------------------------------------------------------
// Pre-Warp Filter Analyze For Test
//-------------------------------------------------------------------------------------------------
#if 0
int dvProAV_WarpPreAnalyze(uint8 *u8WarpTb, uint16 u16CntH, uint16 u16CntV, bool b32x32)
{
    int status = rcSUCCESS;

    const uint8 u8EmptyValue = 0xff;
    uint8 ucWpSpace = (b32x32)? DEF_WP_SPACE_4K : DEF_WP_SPACE_2K;
    uint16 u16TbValueX = 0, u16TbValueY = 0, u16TbValuePastY = 0;
    uint8 u8CoorY = 0, u8CoorX = 0;
    const uint16 u16OutSideValueY = ucWpSpace * u16CntV, u16OutSideValueX = ucWpSpace * u16CntH;

    // auto warp filter table buffer
    uint8 **u8WarpFilterTb = (uint8**)malloc(u16CntV * sizeof(uint8*));
    uint8 *u8TbSpace = (uint8*)malloc(u16CntV * (u16CntH / 2) * sizeof(uint8*));
    for (uint8 i=0; i<u16CntV; i++)
    {
        u8WarpFilterTb[i] = u8TbSpace + i * (u16CntH / 2);
    }

        // warp table 格式轉型
        sWARP_TABLE_DATA **u8WpTb = (sWARP_TABLE_DATA**)malloc((u16CntV + 1) * sizeof(sWARP_TABLE_DATA*));
        for(uint8 i=0; i<(u16CntV + 1); i++)
            u8WpTb[i] = (sWARP_TABLE_DATA *)u8WarpTb + i * (u16CntH + 1);

        // calculate block coor buffer X
        uint8 **i8CoorBufX = (uint8**)malloc((u16CntH + 1) * sizeof(uint8*));
        uint8 *i8CoorBufSpaceX = (uint8*)malloc((u16CntH + 1) * (u16CntV + 1) * sizeof(uint8*));
        for(uint8 i=0; i<(u16CntH + 1); i++)
            i8CoorBufX[i] = i8CoorBufSpaceX + i * (u16CntV + 1);

        // calculate block coor buffer Y
        uint8 **i8CoorBufY = (uint8**)malloc((u16CntH + 1) * sizeof(uint8*));
        uint8 *i8CoorBufSpaceY = (uint8*)malloc((u16CntH + 1) * (u16CntV + 1) * sizeof(uint8*));
        for(uint8 i=0; i<(u16CntH + 1); i++)
            i8CoorBufY[i] = i8CoorBufSpaceY + i * (u16CntV + 1);

        // calculate block ratio index buffer
        uint8 **u8RatioIdxBuf = (uint8**)malloc(u16CntH * sizeof(uint8*));
        uint8 *u8RatioIdxBufSpace = (uint8*)malloc(u16CntH * u16CntV * sizeof(uint8*));
        for(uint8 i=0; i<u16CntH; i++)
            u8RatioIdxBuf[i] = u8RatioIdxBufSpace + i * u16CntV;

#if 0
        // default buffer value
        memset(i8CoorBufX[0], u8EmptyValue, (u16CntH + 1) * (u16CntV + 1));     // shift buffer reset
        memset(i8CoorBufY[0], u8EmptyValue, (u16CntH + 1) * (u16CntV + 1));     // shift buffer reset
        memset(u8RatioIdxBuf[0], u8EmptyValue, u16CntH * u16CntV);              // ratio buffer reset

        // process, 計算原始座標
        for(uint8 u8IndexX=0; u8IndexX<=u16CntH; u8IndexX++)
        {
            for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++) // 偏移量計算
            {
                u16TbValueY = (u8WpTb[u8IndexY][u8IndexX].u16Y >> ((b32x32)? 3:4));
                u8CoorY = u16TbValueY / ucWpSpace;
                u16TbValueX = (u8WpTb[u8IndexY][u8IndexX].u16X >> ((b32x32)? 3:4));
                u8CoorX = u16TbValueX / ucWpSpace;
                // ratio
                if((u8IndexX != 0) && (u8IndexY != 0))
                {
                    if((u16TbValueY > u16OutSideValueY) || (u16TbValuePastY > u16OutSideValueY) || (u8CoorY == 0))
                    {
                        //u8RatioIdxBuf[bOdd][u8IndexY - 1] = u8EmptyValue;
                    }
                    else
                    {
                        double fWarpRatio = (double)((double)ucWpSpace / ((double)(u16TbValueY - ((u16TbValuePastY > u16TbValueY)? 0 : u16TbValuePastY))));
                        u8RatioIdxBuf[u8IndexX - 1][u8IndexY - 1] = (fWarpRatio > 0.975)?   12 :
                                                               (fWarpRatio > 0.95)?    11 :
                                                               (fWarpRatio > 0.925)?   10 :
                                                               (fWarpRatio > 0.9)?     9 :
                                                               (fWarpRatio > 0.85)?   8 :
                                                               (fWarpRatio > 0.8)?    7 :
                                                               (fWarpRatio > 0.75)?   6 :
                                                               (fWarpRatio > 0.7)?     5 :
                                                               (fWarpRatio > 0.65)?    4 :
                                                               (fWarpRatio > 0.6)?     3 :
                                                               (fWarpRatio > 0)?   2 : 15;
                    }
                }
                // shift
                if(u16TbValueY <= u16OutSideValueY)      // warp Y coor is inside
                {
                    i8CoorBufY[u8IndexX][u8IndexY] = u8CoorY;
                }
                if(u16TbValueX <= u16OutSideValueX)      // warp X coor is inside
                {
                    i8CoorBufX[u8IndexX][u8IndexY] = u8CoorX;
                }
                u16TbValuePastY = u16TbValueY;
            }
        }

#if 1   // save to file for debug
        uint8 *u8Print = (uint8*)malloc((u16CntV + 1) * (((u16CntH + 1) * 3) + 1));   // hex轉字串空間
        FILE* foutX = fopen("WarpCoorX.txt", "wb");
        FILE* foutY = fopen("WarpCoorY.txt", "wb");
        FILE* foutF = fopen("WarpRatioY.txt", "wb");
        uint8 u8Data = 0;
        uint16 u16RatioTableCharCnt = 0;

        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=u16CntH; u16CntX++)
            {
                u8Data = i8CoorBufX[u16CntX][u8IndexY];
                u8Print[u16RatioTableCharCnt++] = (((u8Data & 0xf0) >> 4) < 10)? (((u8Data & 0xf0) >> 4) + 0x30) : ((((u8Data & 0xf0) >> 4) % 10) + 0x41);
                u8Print[u16RatioTableCharCnt++] = ((u8Data & 0x0f) < 10)? ((u8Data & 0x0f) + 0x30) : (((u8Data & 0x0f) % 10) + 0x41);
                u8Print[u16RatioTableCharCnt++] = ' ';
            }
            u8Print[u16RatioTableCharCnt++] = '\n';
        }
        fwrite(u8Print, 1, u16RatioTableCharCnt, foutX);
        fclose(foutX);

        u16RatioTableCharCnt = 0;
        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=u16CntH; u16CntX++)
            {
                u8Data = i8CoorBufY[u16CntX][u8IndexY];
                u8Print[u16RatioTableCharCnt++] = (((u8Data & 0xf0) >> 4) < 10)? (((u8Data & 0xf0) >> 4) + 0x30) : ((((u8Data & 0xf0) >> 4) % 10) + 0x41);
                u8Print[u16RatioTableCharCnt++] = ((u8Data & 0x0f) < 10)? ((u8Data & 0x0f) + 0x30) : (((u8Data & 0x0f) % 10) + 0x41);
                u8Print[u16RatioTableCharCnt++] = ' ';
            }
            u8Print[u16RatioTableCharCnt++] = '\n';
        }

        fwrite(u8Print, 1, u16RatioTableCharCnt, foutY);
        fclose(foutY);

        u16RatioTableCharCnt = 0;
        for(uint8 u8IndexY=0; u8IndexY<=(u16CntV - 1); u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=(u16CntH - 1); u16CntX++)
            {
                u8Data = u8RatioIdxBuf[u16CntX][u8IndexY];
                u8Print[u16RatioTableCharCnt++] = (((u8Data & 0xf0) >> 4) < 10)? (((u8Data & 0xf0) >> 4) + 0x30) : ((((u8Data & 0xf0) >> 4) % 10) + 0x41);
                u8Print[u16RatioTableCharCnt++] = ((u8Data & 0x0f) < 10)? ((u8Data & 0x0f) + 0x30) : (((u8Data & 0x0f) % 10) + 0x41);
                u8Print[u16RatioTableCharCnt++] = ' ';
            }
            u8Print[u16RatioTableCharCnt++] = '\n';
        }

        fwrite(u8Print, 1, u16RatioTableCharCnt, foutF);
        fclose(foutF);

        free(u8Print);
#endif
#endif

#if 1
        // default buffer value
        memset(i8CoorBufX[0], u8EmptyValue, (u16CntH + 1) * (u16CntV + 1));     // shift buffer reset
        memset(i8CoorBufY[0], u8EmptyValue, (u16CntH + 1) * (u16CntV + 1));     // shift buffer reset
        memset(u8RatioIdxBuf[0], u8EmptyValue, u16CntH * u16CntV);              // ratio buffer reset

        // process, Y座標校正
        for(uint8 u8IndexX=0; u8IndexX<=u16CntH; u8IndexX++)
        {
            for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++) // 偏移量計算
            {
                u16TbValueY = (u8WpTb[u8IndexY][u8IndexX].u16Y >> ((b32x32)? 3:4));
                u8CoorY = u16TbValueY / ucWpSpace;
                u16TbValueX = (u8WpTb[u8IndexY][u8IndexX].u16X >> ((b32x32)? 3:4));
                u8CoorX = u16TbValueX / ucWpSpace;
                // ratio
                if((u8IndexX != 0) && (u8IndexY != 0))
                {
                    if((u16TbValueY > u16OutSideValueY) || (u16TbValuePastY > u16OutSideValueY) || (u8CoorY == 0))
                    {
                        //u8RatioIdxBuf[bOdd][u8IndexY - 1] = u8EmptyValue;
                    }
                    else
                    {
                        double fWarpRatio = (double)((double)ucWpSpace / ((double)(u16TbValueY - ((u16TbValuePastY > u16TbValueY)? 0 : u16TbValuePastY))));
                        u8RatioIdxBuf[u8IndexX - 1][u8CoorY - 1] = (fWarpRatio > 0.975)?   12 :
                                                                        (fWarpRatio > 0.95)?    11 :
                                                                        (fWarpRatio > 0.925)?   10 :
                                                                        (fWarpRatio > 0.9)?     9 :
                                                                        (fWarpRatio > 0.85)?   8 :
                                                                        (fWarpRatio > 0.8)?    7 :
                                                                        (fWarpRatio > 0.75)?   6 :
                                                                        (fWarpRatio > 0.7)?     5 :
                                                                        (fWarpRatio > 0.65)?    4 :
                                                                        (fWarpRatio > 0.6)?     3 :
                                                                        (fWarpRatio > 0)?   2 : 15;
                    }
                }
                // Coor
                if(u16TbValueY <= u16OutSideValueY)      // warp Y coor is inside
                {
                    i8CoorBufY[u8IndexX][u8CoorY] = u8CoorY;
                    if(u16TbValueX <= u16OutSideValueX)      // warp X coor is inside
                    {
                        i8CoorBufX[u8IndexX][u8CoorY] = u8CoorX;
                    }
                }
                u16TbValuePastY = u16TbValueY;
            }
        }
#if 1   // save to file for debug
        uint8 *u8Print_CalY = (uint8*)malloc((u16CntV + 1) * (((u16CntH + 1) * 3) + 1));   // hex轉字串空間
        FILE* foutX_CalY = fopen("WarpCoorX_CalY.txt", "wb");
        FILE* foutY_CalY = fopen("WarpCoorY_CalY.txt", "wb");
        FILE* foutF_CalY = fopen("WarpRatioY_CalY.txt", "wb");
        uint8 u8Data_CalY = 0;
        uint16 u16TbCharCnt_CalY = 0;

        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=u16CntH; u16CntX++)
            {
                u8Data_CalY = i8CoorBufX[u16CntX][u8IndexY];
                u8Print_CalY[u16TbCharCnt_CalY++] = (((u8Data_CalY & 0xf0) >> 4) < 10)? (((u8Data_CalY & 0xf0) >> 4) + 0x30) : ((((u8Data_CalY & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalY[u16TbCharCnt_CalY++] = ((u8Data_CalY & 0x0f) < 10)? ((u8Data_CalY & 0x0f) + 0x30) : (((u8Data_CalY & 0x0f) % 10) + 0x41);
                u8Print_CalY[u16TbCharCnt_CalY++] = ' ';
            }
            u8Print_CalY[u16TbCharCnt_CalY++] = '\n';
        }
        fwrite(u8Print_CalY, 1, u16TbCharCnt_CalY, foutX_CalY);
        fclose(foutX_CalY);

        u16TbCharCnt_CalY = 0;
        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=u16CntH; u16CntX++)
            {
                u8Data_CalY = i8CoorBufY[u16CntX][u8IndexY];
                u8Print_CalY[u16TbCharCnt_CalY++] = (((u8Data_CalY & 0xf0) >> 4) < 10)? (((u8Data_CalY & 0xf0) >> 4) + 0x30) : ((((u8Data_CalY & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalY[u16TbCharCnt_CalY++] = ((u8Data_CalY & 0x0f) < 10)? ((u8Data_CalY & 0x0f) + 0x30) : (((u8Data_CalY & 0x0f) % 10) + 0x41);
                u8Print_CalY[u16TbCharCnt_CalY++] = ' ';
            }
            u8Print_CalY[u16TbCharCnt_CalY++] = '\n';
        }

        fwrite(u8Print_CalY, 1, u16TbCharCnt_CalY, foutY_CalY);
        fclose(foutY_CalY);

        u16TbCharCnt_CalY = 0;
        for(uint8 u8IndexY=0; u8IndexY<=(u16CntV - 1); u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=(u16CntH - 1); u16CntX++)
            {
                u8Data_CalY = u8RatioIdxBuf[u16CntX][u8IndexY];
                u8Print_CalY[u16TbCharCnt_CalY++] = (((u8Data_CalY & 0xf0) >> 4) < 10)? (((u8Data_CalY & 0xf0) >> 4) + 0x30) : ((((u8Data_CalY & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalY[u16TbCharCnt_CalY++] = ((u8Data_CalY & 0x0f) < 10)? ((u8Data_CalY & 0x0f) + 0x30) : (((u8Data_CalY & 0x0f) % 10) + 0x41);
                u8Print_CalY[u16TbCharCnt_CalY++] = ' ';
            }
            u8Print_CalY[u16TbCharCnt_CalY++] = '\n';
        }

        fwrite(u8Print_CalY, 1, u16TbCharCnt_CalY, foutF_CalY);
        fclose(foutF_CalY);

        free(u8Print_CalY);
#endif
#endif

#if 1
        // process, Y方向Ration Index內差, 將EMPTY_VALUE轉換為正確數值
        for(uint8 u8IndexX=0; u8IndexX<u16CntH; u8IndexX++)
        {
            if(u8RatioIdxBuf[u8IndexX][0] == u8EmptyValue)  // 上邊界block的ratio等於EMPTY_VALUE時的處理
            {
                for(uint8 i=1; i<u16CntV; i++)
                {
                    if(u8RatioIdxBuf[u8IndexX][i] != u8EmptyValue)
                    {
                        u8RatioIdxBuf[u8IndexX][0] = 2 * i / (i + 1) + u8RatioIdxBuf[u8IndexX][i] * 1 / (i + 1);
                        break;
                    }
                }
            }
            if(u8RatioIdxBuf[u8IndexX][u16CntV - 1] == u8EmptyValue)    // 下邊界block的ratio等於EMPTY_VALUE時的處理
            {
                for(uint8 i=1; i<u16CntV; i++)
                {
                    if(u8RatioIdxBuf[u8IndexX][u16CntV - 1 - i] != u8EmptyValue)
                    {
                        u8RatioIdxBuf[u8IndexX][u16CntV - 1] = 2 * i / (i + 1) + u8RatioIdxBuf[u8IndexX][u16CntV - 1 - i] * 1 / (i + 1);
                        break;
                    }
                }
            }
            for(uint8 u8IndexY=0; u8IndexY<u16CntV; u8IndexY++)    // 中間區域ratio遇到EMPTY_VALUE時的處理
            {
                // ratio
                if(u8RatioIdxBuf[u8IndexX][u8IndexY] == u8EmptyValue)
                {
                    for(uint8 i=1; i<u16CntV; i++)
                    {
                        if((u8RatioIdxBuf[u8IndexX][u8IndexY + i] != u8EmptyValue) && ((u8IndexY + i) < u16CntV))
                        {
                            u8RatioIdxBuf[u8IndexX][u8IndexY] = u8RatioIdxBuf[u8IndexX][u8IndexY - 1] * i / (i + 1) + u8RatioIdxBuf[u8IndexX][u8IndexY + i] * 1 / (i + 1);
                            break;
                        }
                    }
                }
            }
        }

#if 1   // save to file for debug
        uint8 *u8Print_CalY_InDif = (uint8*)malloc((u16CntV + 1) * (((u16CntH + 1) * 3) + 1));   // hex轉字串空間
        FILE* foutF_CalY_InDif = fopen("WarpRatioY_CalY_InDif.txt", "wb");
        uint8 u8Data_CalY_InDif = 0;
        uint16 u16TbCharCnt_CalY_InDif = 0;

        u16TbCharCnt_CalY_InDif = 0;
        for(uint8 u8IndexY=0; u8IndexY<=(u16CntV - 1); u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=(u16CntH - 1); u16CntX++)
            {
                u8Data_CalY_InDif = u8RatioIdxBuf[u16CntX][u8IndexY];
                u8Print_CalY_InDif[u16TbCharCnt_CalY_InDif++] = (((u8Data_CalY_InDif & 0xf0) >> 4) < 10)? (((u8Data_CalY_InDif & 0xf0) >> 4) + 0x30) : ((((u8Data_CalY_InDif & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalY_InDif[u16TbCharCnt_CalY_InDif++] = ((u8Data_CalY_InDif & 0x0f) < 10)? ((u8Data_CalY_InDif & 0x0f) + 0x30) : (((u8Data_CalY_InDif & 0x0f) % 10) + 0x41);
                u8Print_CalY_InDif[u16TbCharCnt_CalY_InDif++] = ' ';
            }
            u8Print_CalY_InDif[u16TbCharCnt_CalY_InDif++] = '\n';
        }

        fwrite(u8Print_CalY_InDif, 1, u16TbCharCnt_CalY_InDif, foutF_CalY_InDif);
        fclose(foutF_CalY_InDif);

        free(u8Print_CalY_InDif);
#endif
#endif

#if 1
        // X方向X座標內差
        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            for(uint8 u8IndexX=0; u8IndexX<=u16CntH; u8IndexX++)
            {
                if(i8CoorBufX[u8IndexX][u8IndexY] == u8EmptyValue)
                {
                    if((u8IndexX == 0) || ((u8IndexX > 0) && (i8CoorBufX[u8IndexX - 1][u8IndexY] == u8EmptyValue)))
                    {
                        for(uint8 i=1; i<u16CntV; i++)
                        {
                            if((u8IndexX + i) >= u16CntH)
                                break;
                            else if(i8CoorBufX[u8IndexX + i][u8IndexY] != u8EmptyValue)
                            {
                                i8CoorBufX[u8IndexX][u8IndexY] = ((i8CoorBufX[u8IndexX + i][u8IndexY] - i) >= 0)?(i8CoorBufX[u8IndexX + i][u8IndexY] - i) : u8EmptyValue;
                                break;
                            }
                        }
                    }
                    else
                    {
                        i8CoorBufX[u8IndexX][u8IndexY] = ((i8CoorBufX[u8IndexX - 1][u8IndexY] + 1) < u16CntH)? (i8CoorBufX[u8IndexX - 1][u8IndexY] + 1) : 0xff;
                    }
                }
            }
        }
#if 1   // save to file for debug
        uint8 *u8Print_CalY_XInDif = (uint8*)malloc((u16CntV + 1) * (((u16CntH + 1) * 3) + 1));   // hex轉字串空間
        FILE* foutX_CalY_XInDif = fopen("WarpCoorX_CalY_InDif.txt", "wb");
        uint8 u8Data_CalY_XInDif = 0;
        uint16 u16TbCharCnt_CalY_XInDif = 0;

        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=u16CntH; u16CntX++)
            {
                u8Data_CalY_XInDif = i8CoorBufX[u16CntX][u8IndexY];
                u8Print_CalY_XInDif[u16TbCharCnt_CalY_XInDif++] = (((u8Data_CalY_XInDif & 0xf0) >> 4) < 10)? (((u8Data_CalY_XInDif & 0xf0) >> 4) + 0x30) : ((((u8Data_CalY_XInDif & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalY_XInDif[u16TbCharCnt_CalY_XInDif++] = ((u8Data_CalY_XInDif & 0x0f) < 10)? ((u8Data_CalY_XInDif & 0x0f) + 0x30) : (((u8Data_CalY_XInDif & 0x0f) % 10) + 0x41);
                u8Print_CalY_XInDif[u16TbCharCnt_CalY_XInDif++] = ' ';
            }
            u8Print_CalY_XInDif[u16TbCharCnt_CalY_XInDif++] = '\n';
        }
        fwrite(u8Print_CalY_XInDif, 1, u16TbCharCnt_CalY_XInDif, foutX_CalY_XInDif);
        fclose(foutX_CalY_XInDif);

        free(u8Print_CalY_XInDif);
#endif
#endif

#if 1
        // process, Ratio Index Table X座標校正
        uint8 u8u8RatioIdxLineBuf[120];
        for(uint8 u8IndexY=0; u8IndexY<u16CntV; u8IndexY++)
        {
            memset(u8u8RatioIdxLineBuf, u8EmptyValue, sizeof(u8u8RatioIdxLineBuf));
            for(uint8 u8IndexX=0; u8IndexX<u16CntH; u8IndexX++) // 偏移量計算
            {
                // ratio
                //if((u8IndexX != 0) && (u8IndexY != 0))
                {
                    u8CoorX = i8CoorBufX[u8IndexX][u8IndexY];
                    if(u8u8RatioIdxLineBuf[u8IndexX] != u8EmptyValue)
                    {
                    }
                    else
                    {
                        uint8 u8Test = u8RatioIdxBuf[u8IndexX][u8IndexY];
                        u8u8RatioIdxLineBuf[(u8CoorX < u16CntH)? u8CoorX : (u8CoorX - 1)] = u8Test;
                    }
                }
            }
            //if(u8IndexY > 0)
                for(uint8 i=0;i<sizeof(u8u8RatioIdxLineBuf); i++)
                    u8RatioIdxBuf[i][u8IndexY] = u8u8RatioIdxLineBuf[i];
        }
#if 1   // save to file for debug
        uint8 *u8Print_CalYX = (uint8*)malloc((u16CntV + 1) * (((u16CntH + 1) * 3) + 1));   // hex轉字串空間
        FILE* foutF_CalYX = fopen("WarpRatioY_CalYX.txt", "wb");
        uint8 u8Data_CalYX = 0;
        uint16 u16TbCharCnt_CalYX = 0;

        u16TbCharCnt_CalYX = 0;
        for(uint8 u8IndexY=0; u8IndexY<=(u16CntV - 1); u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=(u16CntH - 1); u16CntX++)
            {
                u8Data_CalYX = u8RatioIdxBuf[u16CntX][u8IndexY];
                u8Print_CalYX[u16TbCharCnt_CalYX++] = (((u8Data_CalYX & 0xf0) >> 4) < 10)? (((u8Data_CalYX & 0xf0) >> 4) + 0x30) : ((((u8Data_CalYX & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalYX[u16TbCharCnt_CalYX++] = ((u8Data_CalYX & 0x0f) < 10)? ((u8Data_CalYX & 0x0f) + 0x30) : (((u8Data_CalYX & 0x0f) % 10) + 0x41);
                u8Print_CalYX[u16TbCharCnt_CalYX++] = ' ';
            }
            u8Print_CalYX[u16TbCharCnt_CalYX++] = '\n';
        }

        fwrite(u8Print_CalYX, 1, u16TbCharCnt_CalYX, foutF_CalYX);
        fclose(foutF_CalYX);

        free(u8Print_CalYX);
#endif
#endif

#if 1
    // process, X方向Ration Index內差, 將EMPTY_VALUE轉換為正確數值
    for(uint8 u8IndexY=0; u8IndexY<u16CntV; u8IndexY++)
    {
        if(u8RatioIdxBuf[0][u8IndexY] == u8EmptyValue)  // 上邊界block的ratio等於EMPTY_VALUE時的處理
        {
            for(uint8 i=1; i<u16CntH; i++)
            {
                if(u8RatioIdxBuf[i][u8IndexY] != u8EmptyValue)
                {
                    u8RatioIdxBuf[0][u8IndexY] = 2 * i / (i + 1) + u8RatioIdxBuf[i][u8IndexY] * 1 / (i + 1);
                    break;
                }
            }
        }
        if(u8RatioIdxBuf[u16CntH - 1][u8IndexY] == u8EmptyValue)    // 下邊界block的ratio等於EMPTY_VALUE時的處理
        {
            for(uint8 i=1; i<u16CntH; i++)
            {
                if(u8RatioIdxBuf[u16CntH - 1 - i][u8IndexY] != u8EmptyValue)
                {
                    u8RatioIdxBuf[u16CntH - 1][u8IndexY] = 2 * i / (i + 1) + u8RatioIdxBuf[u16CntH - 1 - i][u8IndexY] * 1 / (i + 1);
                    break;
                }
            }
        }
        for(uint8 u8IndexX=0; u8IndexX<u16CntH; u8IndexX++)    // 中間區域ratio遇到EMPTY_VALUE時的處理
        {
            // ratio
            if(u8RatioIdxBuf[u8IndexX][u8IndexY] == u8EmptyValue)
            {
                for(uint8 i=1; i<u16CntH; i++)
                {
                    if((u8RatioIdxBuf[u8IndexX + i][u8IndexY] != u8EmptyValue) && ((u8IndexX + i) < u16CntH))
                    {
                        u8RatioIdxBuf[u8IndexX][u8IndexY] = u8RatioIdxBuf[u8IndexX - 1][u8IndexY] * i / (i + 1) + u8RatioIdxBuf[u8IndexX + i][u8IndexY] * 1 / (i + 1);
                        break;
                    }
                }
            }
        }
    }
#if 1   // save to file for debug
        uint8 *u8Print_CalYX_InDif = (uint8*)malloc((u16CntV + 1) * (((u16CntH + 1) * 3) + 1));   // hex轉字串空間
        FILE* foutF_CalYX_InDif = fopen("WarpRatioY_CalYX_InDif.txt", "wb");
        uint8 u8Data_CalYX_InDif = 0;
        uint16 u16TbCharCnt_CalYX_InDif = 0;

        u16TbCharCnt_CalYX_InDif = 0;
        for(uint8 u8IndexY=0; u8IndexY<=(u16CntV - 1); u8IndexY++)
        {
            for(uint16 u16CntX=0; u16CntX<=(u16CntH - 1); u16CntX++)
            {
                u8Data_CalYX_InDif = u8RatioIdxBuf[u16CntX][u8IndexY];
                u8Print_CalYX_InDif[u16TbCharCnt_CalYX_InDif++] = (((u8Data_CalYX_InDif & 0xf0) >> 4) < 10)? (((u8Data_CalYX_InDif & 0xf0) >> 4) + 0x30) : ((((u8Data_CalYX_InDif & 0xf0) >> 4) % 10) + 0x41);
                u8Print_CalYX_InDif[u16TbCharCnt_CalYX_InDif++] = ((u8Data_CalYX_InDif & 0x0f) < 10)? ((u8Data_CalYX_InDif & 0x0f) + 0x30) : (((u8Data_CalYX_InDif & 0x0f) % 10) + 0x41);
                u8Print_CalYX_InDif[u16TbCharCnt_CalYX_InDif++] = ' ';
            }
            u8Print_CalYX_InDif[u16TbCharCnt_CalYX_InDif++] = '\n';
        }

        fwrite(u8Print_CalYX_InDif, 1, u16TbCharCnt_CalYX_InDif, foutF_CalYX_InDif);
        fclose(foutF_CalYX_InDif);

        free(u8Print_CalYX_InDif);
#endif
#endif

    free(u8WpTb);
    free(i8CoorBufY);
    free(i8CoorBufSpaceY);
    free(i8CoorBufX);
    free(i8CoorBufSpaceX);
    free(u8RatioIdxBuf);
    free(u8RatioIdxBufSpace);
    free(u8WarpFilterTb);
    free(u8TbSpace);

    return status;
}
#endif
//-------------------------------------------------------------------------------------------------
// Auto-Warp Filter & Warp
//-------------------------------------------------------------------------------------------------
/**
 * @brief Auto Warp With Filter Set
 * @param [in] u8WarpTb : warp table data point
 * @param [in] u16CntH : warp table H block count
 * @param [in] u16CntV : warp table V block count
 * @param [in] b32x32 : warp block size is 32x32?
 *              - false : Block Size = 16x16
 *              - true : Block Size = 32x32
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpAutoWarpAndFrtSet(uint8 *u8WarpTb, uint16 u16CntH, uint16 u16CntV, bool b32x32, uint8 *cCoeffTb)
{
    int status = rcSUCCESS;

    const uint8 u8EmptyValue = 0xff;
    uint8 ucWpSpace = (b32x32)? DEF_WP_SPACE_4K : DEF_WP_SPACE_2K;
    uint16 u16TbValueX = 0, u16TbValueY = 0, u16TbValuePastX = 0, u16TbValuePastY = 0;
    uint8 u8CoorY = 0, u8CoorX = 0;
    const uint16 u16OutSideValueY = ucWpSpace * u16CntV, u16OutSideValueX = ucWpSpace * u16CntH;
    //double fWarpMinRatioX = 1;
    double /*fWarpRatioX = 1,*/ fWarpRatioY = 1;
    bool bOrgWarpX = true, bOrgWarpY = true;

    // auto warp filter table buffer
    uint8 **u8WarpFilterTb = (uint8**)malloc(u16CntV * sizeof(uint8*));
    uint8 *u8TbSpace = (uint8*)malloc(u16CntV * (u16CntH / 2) * sizeof(uint8));
    // warp table 格式轉型
    sWARP_TABLE_DATA **u8WpTb = (sWARP_TABLE_DATA**)malloc((u16CntV + 1) * sizeof(sWARP_TABLE_DATA*));
    // calculate block coor buffer X
    uint8 **i8CoorBufX = (uint8**)malloc((u16CntH + 1) * sizeof(uint8*));
    uint8 *i8CoorBufSpaceX = (uint8*)malloc((u16CntH + 1) * (u16CntV + 1) * sizeof(uint8));
    // calculate block ratio index buffer
    uint8 **u8RatioIdxBuf = (uint8**)malloc(u16CntH * sizeof(uint8*));
    uint8 *u8RatioIdxBufSpace = (uint8*)malloc(u16CntH * u16CntV * sizeof(uint8));
	uint8 *u8u8RatioIdxLineBuf = (uint8*)malloc(u16CntH * sizeof(uint8));

    for (uint8 i=0; i<u16CntV; i++)
    {
        u8WarpFilterTb[i] = u8TbSpace + i * (u16CntH / 2);
    }

    for(uint8 i=0; i<(u16CntV + 1); i++)
    {
        u8WpTb[i] = (sWARP_TABLE_DATA *)u8WarpTb + i * (u16CntH + 1);
    }

    for(uint8 i=0; i<(u16CntH + 1); i++)
    {
        i8CoorBufX[i] = i8CoorBufSpaceX + i * (u16CntV + 1);
    }

    for(uint8 i=0; i<u16CntH; i++)
    {
        u8RatioIdxBuf[i] = u8RatioIdxBufSpace + i * u16CntV;
    }

    // default buffer value
    memset(i8CoorBufSpaceX, u8EmptyValue, (u16CntH + 1) * (u16CntV + 1));     // shift buffer reset
    memset(u8RatioIdxBufSpace, u8EmptyValue, u16CntH * u16CntV);              // ratio buffer reset

    // process, 一次處理, Y座標校正
    for(uint8 u8IndexX=0; u8IndexX<=u16CntH; u8IndexX++)
    {
        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++) // 偏移量計算
        {
            u16TbValueY = (u8WpTb[u8IndexY][u8IndexX].u16Y >> ((b32x32)? 3:4));
            u8CoorY = u16TbValueY / ucWpSpace;
            u16TbValueX = (u8WpTb[u8IndexY][u8IndexX].u16X >> ((b32x32)? 3:4));
            u8CoorX = u16TbValueX / ucWpSpace;
            // ratio
            if((u8IndexX != 0) && (u8IndexY != 0))
            {
                if((u16TbValueY > u16OutSideValueY) || (u16TbValuePastY > u16OutSideValueY) || (u8CoorY == 0))
                {
                    //u8RatioIdxBuf[bOdd][u8IndexY - 1] = u8EmptyValue;
                }
                else
                {
                    //fWarpRatioX = (double)(((double)(u16TbValueX - ((u16TbValuePastX > u16TbValueX)? 0 : u16TbValuePastX))) / (double)ucWpSpace);
                    //fWarpMinRatioX = (fWarpMinRatioX < fWarpRatioX)? fWarpMinRatioX : fWarpRatioX;
                    if((double)(((double)(u16TbValueX - ((u16TbValuePastX > u16TbValueX)? 0 : u16TbValuePastX))) / (double)ucWpSpace) != 1)
                        bOrgWarpX = false;
                    fWarpRatioY = (double)((double)ucWpSpace / ((double)(u16TbValueY - ((u16TbValuePastY > u16TbValueY)? 0 : u16TbValuePastY))));
                    if(fWarpRatioY != 1)
                        bOrgWarpY = false;
#if 1
                    u8RatioIdxBuf[u8IndexX - 1][u8CoorY - 1] = (fWarpRatioY > 0.975)?   12 :
                                                                   (fWarpRatioY > 0.95)?    11 :
                                                                   (fWarpRatioY > 0.925)?   10 :
                                                                   (fWarpRatioY > 0.9)?     9 :
                                                                   (fWarpRatioY > 0.85)?   8 :
                                                                   (fWarpRatioY > 0.8)?    7 :
                                                                   (fWarpRatioY > 0.75)?   6 :
                                                                   (fWarpRatioY > 0.7)?     5 :
                                                                   (fWarpRatioY > 0.65)?    4 :
                                                                   (fWarpRatioY > 0.6)?     3 :
                                                                   (fWarpRatioY > 0)?   2 : 15;
#else
                    u8RatioIdxBuf[u8IndexX - 1][u8CoorY - 1] =
                          (fWarpRatioY > 0.98)?   14 :
                          (fWarpRatioY > 0.96)?    13 :
                          (fWarpRatioY > 0.94)?   12 :
                          (fWarpRatioY > 0.92)?     11 :
                          (fWarpRatioY > 0.90)?     10 :
                          (fWarpRatioY > 0.875)?     9 :
                          (fWarpRatioY > 0.85)?    8 :
                          (fWarpRatioY > 0.825)?    7 :
                          (fWarpRatioY > 0.8)?    6 :
                          (fWarpRatioY > 0.775)?    5 :
                          (fWarpRatioY > 0.75)?   4 :
                          (fWarpRatioY > 0.7)?     3 :
                          (fWarpRatioY > 0.65)?    2 :
                          (fWarpRatioY > 0.6)?     1 :
                          (fWarpRatioY > 0)?   0: 15;
#endif
                }
            }
            // Coor
            if(u16TbValueY <= u16OutSideValueY)      // warp Y coor is inside
            {
                if(u16TbValueX <= u16OutSideValueX)      // warp X coor is inside
                {
                    i8CoorBufX[u8IndexX][u8CoorY] = u8CoorX;
                }
            }
            u16TbValuePastY = u16TbValueY;
        }
        // Y方向Ration Index內差, 將EMPTY_VALUE轉換為正確數值
        if(u8IndexX != 0)
        {
            if(u8RatioIdxBuf[u8IndexX - 1][0] == u8EmptyValue)  // 上邊界block的ratio等於EMPTY_VALUE時的處理
            {
                for(uint8 i=1; i<u16CntV; i++)
                {
                    if(u8RatioIdxBuf[u8IndexX - 1][i] != u8EmptyValue)
                    {
                        u8RatioIdxBuf[u8IndexX - 1][0] = 2 * i / (i + 1) + u8RatioIdxBuf[u8IndexX - 1][i] * 1 / (i + 1);
                        break;
                    }
                }
            }
            if(u8RatioIdxBuf[u8IndexX - 1][u16CntV - 1] == u8EmptyValue)    // 下邊界block的ratio等於EMPTY_VALUE時的處理
            {
                for(uint8 i=1; i<u16CntV; i++)
                {
                    if(u8RatioIdxBuf[u8IndexX - 1][u16CntV - 1 - i] != u8EmptyValue)
                    {
                        u8RatioIdxBuf[u8IndexX - 1][u16CntV - 1] = 2 * i / (i + 1) + u8RatioIdxBuf[u8IndexX - 1][u16CntV - 1 - i] * 1 / (i + 1);
                        break;
                    }
                }
            }
            for(uint8 u8IndexY=0; u8IndexY<u16CntV; u8IndexY++)    // 中間區域ratio遇到EMPTY_VALUE時的處理
            {
                // ratio
                if(u8RatioIdxBuf[u8IndexX - 1][u8IndexY] == u8EmptyValue)
                {
                    for(uint8 i=1; i<u16CntV; i++)
                    {
                        if((u8RatioIdxBuf[u8IndexX - 1][u8IndexY + i] != u8EmptyValue) && ((u8IndexY + i) < u16CntV))
                        {
                            u8RatioIdxBuf[u8IndexX - 1][u8IndexY] = u8RatioIdxBuf[u8IndexX - 1][u8IndexY - 1] * i / (i + 1) + u8RatioIdxBuf[u8IndexX - 1][u8IndexY + i] * 1 / (i + 1);
                            break;
                        }
                    }
                }
            }
        }
        u16TbValuePastX = u16TbValueX;
    }
    free(u8WpTb);

    if(bOrgWarpY)// 如判斷為原圖
    {
        memset(u8TbSpace, 0xff, u16CntV * (u16CntH / 2));      // ratio buffer reset
    }
    else    // process, 二次處理, X方向校正
    {
        for(uint8 u8IndexY=0; u8IndexY<=u16CntV; u8IndexY++)
        {
            memset(u8u8RatioIdxLineBuf, u8EmptyValue, u16CntH);
            for(uint8 u8IndexX=0; u8IndexX<=u16CntH; u8IndexX++)
            {
                // X座標內差
                if(i8CoorBufX[u8IndexX][u8IndexY] == u8EmptyValue)
                {
                    if((u8IndexX == 0) || ((u8IndexX > 0) && (i8CoorBufX[u8IndexX - 1][u8IndexY] == u8EmptyValue)))
                    {
                        for(uint8 i=1; i<u16CntV; i++)
                        {
                            if((u8IndexX + i) >= u16CntH)
                                break;
                            else if(i8CoorBufX[u8IndexX + i][u8IndexY] != u8EmptyValue)
                            {
                                i8CoorBufX[u8IndexX][u8IndexY] = ((i8CoorBufX[u8IndexX + i][u8IndexY] - i) >= 0)?(i8CoorBufX[u8IndexX + i][u8IndexY] - i) : u8EmptyValue;
                                break;
                            }
                        }
                    }
                    else
                    {
                        i8CoorBufX[u8IndexX][u8IndexY] = ((i8CoorBufX[u8IndexX - 1][u8IndexY] + 1) < u16CntH)? (i8CoorBufX[u8IndexX - 1][u8IndexY] + 1) : u8EmptyValue;
                    }
                }
                // Ratio Index Table X座標校正
                u8CoorX = i8CoorBufX[u8IndexX][u8IndexY];
                if((u8u8RatioIdxLineBuf[u8IndexX] == u8EmptyValue) && (u8IndexX < u16CntH) && (u8IndexY < u16CntV))
                {
                    uint8 u8Ind = (u8CoorX < u16CntH)? u8CoorX : (u8CoorX - 1);

                    if((u8u8RatioIdxLineBuf[u8IndexX] == u8EmptyValue) && (u8Ind < u16CntH))
                    {
                        u8u8RatioIdxLineBuf[u8Ind] = u8RatioIdxBuf[u8IndexX][u8IndexY];
                    }
                }
            }
            // X方向Ration Index內差, 將EMPTY_VALUE轉換為正確數值
            if(u8u8RatioIdxLineBuf[0] == u8EmptyValue)  // 上邊界block的ratio等於EMPTY_VALUE時的處理
            {
                for(uint8 i=1; i<u16CntH; i++)
                {
                    if(u8u8RatioIdxLineBuf[i] != u8EmptyValue)
                    {
                        u8u8RatioIdxLineBuf[0] = 2 * i / (i + 1) + u8u8RatioIdxLineBuf[i] * 1 / (i + 1);
                        break;
                    }
                }
            }
            if(u8u8RatioIdxLineBuf[u16CntH - 1] == u8EmptyValue)    // 下邊界block的ratio等於EMPTY_VALUE時的處理
            {
                for(uint8 i=1; i<u16CntH; i++)
                {
                    if(u8u8RatioIdxLineBuf[u16CntH - 1 - i] != u8EmptyValue)
                    {
                        u8u8RatioIdxLineBuf[u16CntH - 1] = 2 * i / (i + 1) + u8u8RatioIdxLineBuf[u16CntH - 1 - i] * 1 / (i + 1);
                        break;
                    }
                }
            }
            for(uint8 u8IndexX=0; u8IndexX<u16CntH; u8IndexX++)    // 中間區域ratio遇到EMPTY_VALUE時的處理
            {
                // ratio
                if(u8u8RatioIdxLineBuf[u8IndexX] == u8EmptyValue)
                {
                    for(uint8 i=1; i<u16CntH; i++)
                    {
                        if((u8u8RatioIdxLineBuf[u8IndexX + i] != u8EmptyValue) && ((u8IndexX + i) < u16CntH))
                        {
                            u8u8RatioIdxLineBuf[u8IndexX] = u8u8RatioIdxLineBuf[u8IndexX - 1] * i / (i + 1) + u8u8RatioIdxLineBuf[u8IndexX + i] * 1 / (i + 1);
                            break;
                        }
                    }
                }
            }
            // Ratio Line buffer to buffer packing
            if(u8IndexY < u16CntV)
            {
                for(uint8 i=0; i<u16CntH; i++)
                {
                    // buffer資料存至warp filter table
                    if((i % 2) == 0)
                    {
                        u8WarpFilterTb[u8IndexY][(i / 2)] = u8u8RatioIdxLineBuf[i];
                    }
                    else
                    {
                        u8WarpFilterTb[u8IndexY][(i / 2)] |= (u8u8RatioIdxLineBuf[i] << 4) & 0xf0;
                    }
                }
            }
        }
    }
    free(u8u8RatioIdxLineBuf);
    free(i8CoorBufX);
    free(i8CoorBufSpaceX);
    free(u8RatioIdxBuf);
    free(u8RatioIdxBufSpace);

#ifdef DEF_SAVE_AUTO_FILTER_TABLE   // save to file for debug
    uint8 *u8WarpFilterTbPrint = (uint8*)malloc(u16CntV * (u16CntH * 2 + 1));   // hex轉字串空間
    FILE* fout = fopen("WarpFltTb_debug.txt", "wb");
    uint8 u8Data = 0;
    uint16 u16RatioTableCharCnt = 0;

    for(uint8 u8IndexY=0; u8IndexY<u16CntV; u8IndexY++)
    {
        for(uint16 u16CntX=0; u16CntX<(u16CntH/2); u16CntX++)
        {
            u8Data = u8WarpFilterTb[u8IndexY][u16CntX];//u8RatioIdxBuf[u8IndexY / 2][(u8IndexY % 2) * (u16CntH / 2) + u16CntX];//
            u8WarpFilterTbPrint[u16RatioTableCharCnt++] = ((u8Data & 0x0f) < 10)? ((u8Data & 0x0f) + 0x30) : (((u8Data & 0x0f) % 10) + 0x41);
            u8WarpFilterTbPrint[u16RatioTableCharCnt++] = (((u8Data & 0xf0) >> 4) < 10)? (((u8Data & 0xf0) >> 4) + 0x30) : ((((u8Data & 0xf0) >> 4) % 10) + 0x41);
        }
        u8WarpFilterTbPrint[u16RatioTableCharCnt++] = '\n';
    }
    fwrite(u8WarpFilterTbPrint, 1, u16RatioTableCharCnt, fout);
    free(u8WarpFilterTbPrint);
    fclose(fout);
#endif

    /*WarpCoeffTable eCoeffTb = (fWarpMinRatioX == 1)? eWarpCoeff_20 :
                                  (fWarpMinRatioX > 0.975)?   eWarpCoeff_16 :
                                  (fWarpMinRatioX > 0.95)?    eWarpCoeff_15 :
                                  (fWarpMinRatioX > 0.925)?   eWarpCoeff_14 :
                                  (fWarpMinRatioX > 0.9)?     eWarpCoeff_13 :
                                  (fWarpMinRatioX > 0.85)?   eWarpCoeff_12 :
                                  (fWarpMinRatioX > 0.8)?    eWarpCoeff_11 :
                                  (fWarpMinRatioX > 0.75)?   eWarpCoeff_10 :
                                  (fWarpMinRatioX > 0.7)?     eWarpCoeff_9 :
                                  (fWarpMinRatioX > 0.65)?    eWarpCoeff_8 :
                                  (fWarpMinRatioX > 0.6)?     eWarpCoeff_7 :
                                  (fWarpMinRatioX > 0)?   eWarpCoeff_6 : eWarpCoeff_16;*/

#ifdef QT_CPP
    status &= dvProAV_WarpTableDl2Chip((u16CntH + 1) * (u16CntV + 1) * 4, u8WarpTb);
#endif /* QT_CPP */
    status &= dvProAV_WarpPreFtIdxTbSet((u16CntH * u16CntV) / 2, (uint8 *)&u8WarpFilterTb[0][0], b32x32);
    status &= dvProAV_WarpCoeffSet((bOrgWarpX)? eWarpCoeff_20 : eWarpCoeff_16);

    if(cCoeffTb != NULL)
    {
        *cCoeffTb = (bOrgWarpX)? eWarpCoeff_20 : eWarpCoeff_16;
    }

    free(u8WarpFilterTb);
    free(u8TbSpace);

    return status;
}

//-------------------------------------------------------------------------------------------------
// Pre-Warp Filter
//-------------------------------------------------------------------------------------------------
/**
 * @brief Auto Warp With Filter Set
 * @param [in] u16CntH : warp table H block count
 * @param [in] u16CntV : warp table V block count
 * @param [in] u8FilterIdx : Pre-Warp Filter Index
 *              - u8FilterIdx <= 15 : all page use same Pre-Warp Filter, 15->0 : sharp->blur
 * @param [in] b32x32 : warp block size is 32x32?
 *              - false : Block Size = 16x16
 *              - true : Block Size = 32x32
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_PreFtSet(uint16 u16CntH, uint16 u16CntV, uint8 u8FilterIdx, bool b32x32)
{
    int status = rcSUCCESS;

    const uint8 u8FilterMax_Y = 15;
    uint8 **u8WarpFilterTb = (uint8**)malloc(u16CntV * sizeof(uint8*));
    uint8 *u8TbSpace = (uint8*)malloc(u16CntV * (u16CntH / 2) * sizeof(uint8*));
    for (uint8 i=0; i<u16CntV; i++)
        u8WarpFilterTb[i] = u8TbSpace + i * (u16CntH / 2);
    if(u8FilterIdx > u8FilterMax_Y)
        u8FilterIdx = u8FilterMax_Y;
    uint8 u8Idx = (u8FilterIdx << 4) | u8FilterIdx;
    memset(u8TbSpace, u8Idx, u16CntV * (u16CntH / 2));

    status &= dvProAV_WarpPreFtIdxTbSet((u16CntH * u16CntV) / 2, (uint8 *)&u8WarpFilterTb[0][0], b32x32);

    return status;
}

/**
 * @brief Pre-Warp Coefficient Y Table set
 * @param [in] ucPreCoeff : pre-warp coeff table, 一次須寫滿16組coefficient table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpPreCoeffYSet(uint8 *ucPreCoeff)
{
    int status = rcSUCCESS;

    status &= dvProAV_WarpPreFilterYEn(false);

    status &= dvProAV_AccessWrite(eWftyFltCoefWr_E4k, true);
    status &= dvProAV_AccessBurstWrite(eWftyFltCoefPort_E4k, ucPreCoeff, PRE_WARP_Y_COEFF_SIZE, BURST_FIX_ADDR);
    status &= dvProAV_AccessWrite(eWftyFltCoefWr_E4k, false);

    status &= dvProAV_WarpPreFilterYEn(true);

    return status;
}

/**
 * @brief Pre-Warp Filter Index Table Set
 * @param [in] ulSize : warp index table size
 * @param [in] ucIdxTb : warp index table
 * @param [in] b32x32 :
 *              false : 16x16 table
 *              true : 32x32 table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpPreFtIdxTbSet(uint32 ulSize, uint8 *ucIdxTb, bool b32x32)
{
    int status = rcSUCCESS;

    status &= dvProAV_WarpPreFilterYEn(false);
    status &= dvProAV_AccessWrite(eWftyFltWr_E4k, true);
    status &= dvProAV_AccessBurstWrite(eWftyFltPort_E4k, ucIdxTb, (uint16)ulSize, BURST_FIX_ADDR);
    status &= dvProAV_AccessWrite(eWftyFltWr_E4k, false);
    status &= dvProAV_AccessWrite(eWftyTable16_E4k, !b32x32);
    status &= dvProAV_WarpPreFilterYEn(true);

    return status;
}

/**
 * @brief Warp Pre-filter Y Direction Enable
 * @param [in] bFilterYEn :
 *              flase : warp filter Y direction enable
 *              true : warp filter Y direction disable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpPreFilterYEn(bool bFilterYEn)
{
    return dvProAV_AccessWrite(eWftyFltEn_E4k, bFilterYEn);
}

//-------------------------------------------------------------------------------------------------
// Warp
//-------------------------------------------------------------------------------------------------
/**
 * @brief Warp Filter Enable
 * @param [in] bFilterEn : kernel warp filter X Y enable
 *              flase : warp kernel filter disable
 *              true : warp kernel filter enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpFilterEn(bool bFilterEn)
{
    return dvProAV_AccessWrite(eWarpWftEn, bFilterEn);
}

/**
 * @brief Warping Table Download to Chip
 * @param [in] u32WarpTbSize : warp table size
 * @param [in] u8WarpTb : warp table data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpTableDl2Chip(uint32 u32WarpTbSize, uint8 *u8WarpTb)
{
    int status = rcSUCCESS;
    uint32 ul32x32Mode = 0, ulYRatio = 0;

#ifdef QT_CPP
    // set 16x16/32x32 mode
    status &= dvProAV_AccessRead(eWarpWpBlk32x32, &ul32x32Mode);
    status &= dvProAV_AccessWrite(eWarpFrac3bits, (u32WarpTbSize >= dvProAV_WarpTableBlockSizeGet(e4KSize))? true : ul32x32Mode);   // 傳入4K table禁用32x32Mode
#endif /* QT_CPP */
    // set warping delay parameter
    status &= dvProAV_AccessRead(eWarpWpManualYsd, &ulYRatio);
    status &= dvProAV_WarpDelaySet(u32WarpTbSize, (WarpScalingRatio)ulYRatio, (bool)ul32x32Mode);
#if 0   // SPI->SRAM
    uint32 ulCount = 0;
    status &= dvProAV_AccessWrite(eWarpWpTableWrite, true);

    if(ulWarpTableSize < 65536)
    {
        status &= dvProAV_AccessBurstWrite(eWarpWpTablePort, warpTable, (uint16)ulWarpTableSize, BURST_FIX_ADDR);
    }
    else
    {
        uint32 ulWiteLen = 0;
        ulCount = ulWarpTableSize;

        dvProAV_AccessBurstModeEnable(true);
        while(ulCount!=0)
        {
            ulWiteLen = (ulCount > 65535)? 65535 : ulCount;
            status &= dvProAV_AccessBurstWrite(eWarpWpTablePort, (warpTable + (ulWarpTableSize - ulCount)), (uint16)ulWiteLen, BURST_FIX_ADDR);
            ulCount -= ulWiteLen;
        }
        dvProAV_AccessBurstModeEnable(true);
    }
#else   // SPI->DRAM->SRAM
#ifdef DEF_SAVE_WARP_TABLE   // save warp table for debug
    FILE *fptr;
#if 1   // save to .txt File
    fptr = fopen("WarpTable.txt","w");
    uint32 u32AsciiTableSize = u32WarpTbSize * 3;
    uint8 u8BlkCntH = 120;
    char *pAsciiArray = (char *)malloc(u32AsciiTableSize);
    for(uint32 i=0; i<u32WarpTbSize; i++)     // hex to ASCii
    {
        pAsciiArray[3 * i] = hextoascii((u8WarpTb[i] >> 4) & 0x0f);
        pAsciiArray[(3 * i) + 1] = hextoascii(u8WarpTb[i] & 0x0f);
        if((i % ((u8BlkCntH + 1)* 4)) == (((u8BlkCntH + 1)* 4) - 1))
            pAsciiArray[(3 * i) + 2] = '\n';
        else
            pAsciiArray[(3 * i) + 2] = ' ';
    }
    fwrite(&pAsciiArray[0], 1, u32AsciiTableSize, fptr);
    free(pAsciiArray);
#else   // save to .dat File
    fptr = fopen("WarpTable.dat","w");
    fwrite(u8WarpTb, 1, u32WarpTbSize, fptr);
#endif
    fclose(fptr);
#endif
    status &= dvProAV_LineBufAccess(eDramBank1, DRAM_WRP_TABLE_ADDR, u8WarpTb, u32WarpTbSize, IO_WRITE);
    status &= dvProAV_WarpTableDMA(u32WarpTbSize, DRAM_WRP_TABLE_ADDR);
#endif
    return status;
}

/**
 * @brief Warping Image Size Set
 * @param [in] Timing : Output Timing Info
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
#ifndef QT_CPP
int dvProAV_WarpImageSizeSet(uint16 uiPanelID)
{
    int status = rcSUCCESS;
    uint16 uiIndex = 0;
    uint32 uiHsize= 0;
    uint32 uiVsize= 0;
    uint8  ucRetry = 60;

    WarpPanelTiming sPanelInfo = {0};


    if(ePROAV_EXEC_CODE_PASS == dvPro_GetPanelInfo(uiPanelID, (UINT8*)&sPanelInfo))
    {
        DBMSG("========WarpTableSizeSet========\n");
        DBMSG(" eVOPHTot %d \n",     sPanelInfo.HTotal);
        DBMSG(" eVOPHsWidth %d \n",  sPanelInfo.HSync);
        DBMSG(" eVOPHDspSt %d \n",   sPanelInfo.HStart);
        DBMSG(" eVOPHSize %d \n",    sPanelInfo.HSize);
        DBMSG(" eVOPVTot %d \n",     sPanelInfo.VTotal);
        DBMSG(" eVOPVsWidth %d \n",  sPanelInfo.VSync);
        DBMSG(" eVOPVDspSt %d \n",   sPanelInfo.VStart);
        DBMSG(" eVOPVSize %d \n",    sPanelInfo.VSize);
        DBMSG("===============================\n");

        uint16 u16Hsize = ((sPanelInfo.HSize + PROAV_DRAM_LINE_BYTES - 1) / PROAV_DRAM_LINE_BYTES) * PROAV_DRAM_LINE_BYTES;
        status &= dvProAV_AccessWrite(eWcuMb2HsWidth, sPanelInfo.HSync);     // Hsync Width
        status &= dvProAV_AccessWrite(eWcuMb2VsWidth, sPanelInfo.VSync);     // Vsync Width
        status &= dvProAV_AccessWrite(eWarpWarpingHSize, u16Hsize);  // Hsize for Warping
        status &= dvProAV_AccessWrite(eWarpWarpingVSize, sPanelInfo.VSize);  // Vsize for Warping
        status &= dvPro_PixelShift_Setting(ePIXEL_SHIFT_NORMAL);

        switch(sPanelInfo.id)
        {
            case ePANEL_ID_1080P_60HZ:
            case ePANEL_ID_1080P_120HZ:
            case ePANEL_ID_1080P_240HZ:
            case ePANEL_ID_WUXGA_60HZ:
            case ePANEL_ID_WUXGA_120HZ:
                dvProAV_PixSftWcuEn(false);
                dvProAV_WarpBlkSizeSet(false);
                dvProAV_AccessWrite(eWarpFrac3bits, 0);
                dvProAV_WarpDelayTableSet(eWarpDelayTable1d3_2DRAM, 9);
#ifdef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSftWcuEn(false);
                }
#endif /* VPD_XPR_ENABLE */
                break;

            case ePANEL_ID_WUXGA_240HZ:
                dvProAV_PixSftWcuEn(false);
#ifdef WUXGA240_32X32
                dvProAV_WarpBlkSizeSet(true);
#else
                dvProAV_WarpBlkSizeSet(false);
#endif /* WUXGA240_32X32 */
                dvProAV_AccessWrite(eWarpFrac3bits, 0);
                dvProAV_WarpDelayTableSet(eWarpDelayTable1d3_2DRAM, 9);
#ifdef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSftWcuEn(false);
                }
#endif /* VPD_XPR_ENABLE */
                break;

           case ePANEL_ID_1920x2400_60HZ: //for 4K3D dual pipe
                dvProAV_PixSftWcuEn(true);
                dvProAV_WarpBlkSizeSet(true);
                dvProAV_AccessWrite(eWarpFrac3bits, 0);
                dvProAV_WarpDelayTableSet(eWarpDelayTable1d3_2DRAM, 9);
                dvProAV_AccessWrite(eVopHsWidth, sPanelInfo.HSync);

#ifdef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSftWcuEn(false);
                }
#endif /* VPD_XPR_ENABLE */
                break;

            case ePANEL_ID_2688x1472_120HZ:
                dvProAV_WarpBlkSizeSet(true);
                dvProAV_AccessWrite(eWarpFrac3bits, 1);
                dvProAV_WarpDelayTableSet(eWarpDelayTable1d3_2DRAM, 9);
#ifdef VPD_XPR_ENABLE
               if(Board_SingleBoard_Get() == FALSE)
               {
                   dvProAV_PixSftWcuEn(false);
               }
#endif /* VPD_XPR_ENABLE */
                break;

            default:
                dvProAV_PixSftWcuEn(false);
                dvProAV_WarpBlkSizeSet(true);
                dvProAV_AccessWrite(eWarpFrac3bits, 1);
                dvProAV_WarpDelayTableSet(eWarpDelayTable1d3_2DRAM, 6);
#ifdef VPD_XPR_ENABLE
                if(Board_SingleBoard_Get() == FALSE)
                {
                    dvProAV_PixSftWcuEn(true);
                }
#else
                //dvProAV_PixSftWcuEn(false);
#endif /* VPD_XPR_ENABLE */
                break;
        }

        m_uiWarpingPanel = sPanelInfo.id;

        do
        {
            MS_SLEEP(10);

            status &= dvProAV_AccessRead(eWcuMb2HSize, &uiHsize);
            status &= dvProAV_AccessRead(eWcuMb2VSize, &uiVsize);
            ucRetry--;
        }while(((uiHsize != sPanelInfo.HSize) || (uiVsize != sPanelInfo.VSize)) && ucRetry);

        return status;
    }
    else
    {
        DBMSG("Warping table not find uiPanelID = %d\n",uiPanelID);
    }

    return rcERROR;

}
#else
int dvProAV_WarpImageSizeSet(const WarpPanelTiming *Timing)
{
    int status = rcSUCCESS;
    uint16 u16Hsize = ((Timing->HSize + PROAV_DRAM_LINE_BYTES - 1) / PROAV_DRAM_LINE_BYTES) * PROAV_DRAM_LINE_BYTES;

    status &= dvProAV_AccessWrite(eWcuMb2HsWidth, Timing->HSync);     // Hsync Width
    status &= dvProAV_AccessWrite(eWcuMb2VsWidth, Timing->VSync);     // Vsync Width
    status &= dvProAV_AccessWrite(eWarpWarpingHSize, u16Hsize);       // Hsize for Warping
    status &= dvProAV_AccessWrite(eWarpWarpingVSize, Timing->VSize);  // Vsize for Warping

    DBMSG("========WarpTableSizeSet========\n");
    DBMSG(" eVOPHTot %d \n",    Timing->HTotal);
    DBMSG(" eVOPHsWidth %d \n",  Timing->HSync);
    DBMSG(" eVOPHDspSt %d \n",   Timing->HStart);
    DBMSG(" eVOPHSize %d (%d) \n",    Timing->HSize, u16Hsize);
    DBMSG(" eVOPVTot %d \n",    Timing->VTotal);
    DBMSG(" eVOPVsWidth %d \n",  Timing->VSync);
    DBMSG(" eVOPVDspSt %d \n",   Timing->VStart);
    DBMSG(" eVOPVSize %d \n",    Timing->VSize);
    DBMSG("===============================\n");

    return status;
}
#endif

/**
 * @brief Warp Scaling Set
 * @param [in] XRatio : warp X direction scaling ratio
 * @param [in] YRatio : warp Y direction scaling ratio
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpScalingSet(WarpScalingRatio XRatio, WarpScalingRatio YRatio)
{
    int status = rcSUCCESS;

    status &= dvProAV_AccessWrite(eWarpWpManualXsd, XRatio);
    status &= dvProAV_AccessWrite(eWarpWpManualYsd, YRatio);

    return status;
}

/**
* @brief Warp Auto Scaling Enable / Disable Set
* @param [in] bEnable
*             - false : Disable Warp Auto Scaling
*             - true : Enable Warp Auto Scaling
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_WarpAutoScalingEn(bool bEnable)
{
   return dvProAV_AccessWrite(eWarpWpAutosd, bEnable);
}

/**
 * @brief Warping Freeze Set
 * @param [in] bFreeze :
 *              - false : unfreeze
 *              - true : freeze
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpFreeze(bool bFreeze)
{
    return dvProAV_AccessWrite(eMwiu2WrReqMaskWfo, (bFreeze != true)? 0x00 : 0x03);
}

/**
 * @brief Warping Bypass Set
 * @param [in] bBypass :
 *              - false : with warp
 *              - true : bypass warp
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpBypassSet(bool bBypass)
{
    m_bWarpingbypass = bBypass;
    return dvProAV_AccessWrite(eWcuBypsWarp, bBypass);
}

/**
 * @brief Warp Bypass Status Get
 * @return m_bWarpingbypass : warp bypass status
 */
uint8 dvProAV_WarpBypassGet(void)
{
    return m_bWarpingbypass;
}

/**
 * @brief Warp X Coefficient Table Set
 * @param [in] ucWarpTableSel : warp coeff select
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpCoeffSet(WarpCoeffTable ucWarpCoeffSel)
{
    int status = rcSUCCESS;
    if(ucWarpCoeffSel >= eWarpCoeffTableMax)
    {
        DBMSG_ASSERT();
        return rcINVALID;
    }

    status &= dvProAV_WarpFilterEn(false);
    status &= dvProAV_AccessWrite(eWarpWftWr, true);
    status &= dvProAV_AccessBurstWrite(eWarpWftPort, (uint08 *)(&warpCoeff[ucWarpCoeffSel][0]), WARP_COEFF_BLOCK_SIZE, BURST_FIX_ADDR);
    status &= dvProAV_AccessWrite(eWarpWftWr, false);

    status &= dvProAV_WarpFilterEn(true);

    return status;
}

/**
 * @brief Warp Y Coefficient Table Set
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpYCoeffSet_E4K(void)
{
    int status = rcSUCCESS;

    status &= dvProAV_WarpFilterEn(false);

    status &= dvProAV_AccessWrite(eWarpWftYWr, true);
    status &= dvProAV_AccessBurstWrite(eWarpWftYPort_E4k, (uint08 *)(&WarpYCoeff[0]), WARP_2TP_COEFF_BLOCK_SIZE, BURST_FIX_ADDR);
    status &= dvProAV_AccessWrite(eWarpWftYWr, false);

    status &= dvProAV_WarpFilterEn(true);

    return status;
}

/**
* @brief Warp Block Size Mode Set
* @param [in] bMode
*             - false : Block Size = 16x16
*             - true : Block Size = 32x32
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_WarpBlkSizeSet(bool bMode)
{
   return dvProAV_AccessWrite(eWarpWpBlk32x32, bMode);
}

/**
 * @brief DRAM Warp Table To SRAM
 * @param [in] ulWarpTableSize : Warp table Size
 * @param [in] u32DramAddr : Warp table DRAM address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpTableDMA(uint32 ulWarpTableSize, uint32 u32DramAddr)
{
    int status = rcSUCCESS;
    uint32 DMAEn = 0;
    uint8  ucRetry = 20;

    status &= dvProAV_AccessWrite(eWarpDmaBaseAdr, u32DramAddr >> 1);
    status &= dvProAV_AccessWrite(eWarpTotalNum, (ulWarpTableSize / 4));
    status &= dvProAV_AccessWrite(eWarpWarpingDmaEn, true);     // DRAM Write to SRAM

    do
    {
        status &= dvProAV_AccessRead(eWarpWarpingVSize, &DMAEn);
        MS_SLEEP(1);
    }while((DMAEn == 1) && (ucRetry--));

    // temporary solution for 2688x1472 warp table DMA issue
    //status &= dvProAV_AccessWrite(eWarpWpTableWrite, true);
    //MS_SLEEP(20);
    //status &= dvProAV_AccessWrite(eWarpWpTableWrite, false);

    return status;
}

/**
 * @brief Warp Delay Set
 * @param [in] ulWarpTableSize : warp table size
 * @param [in] sYRatio : warp Y direction scaling ratio
 * @param [in] b32x32Mode
 *             - false : Block Size = 16x16
 *             - true : Block Size = 32x32, 2K Warp Table to 4K
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpDelaySet(uint32 ulWarpTableSize, WarpScalingRatio sYRatio, bool b32x32Mode)
{
    int status = rcSUCCESS;
    uint8 ucTableIndex = 0;
    WarpDelayTable ucWarpDelayTable = eWarpDelayTable1d3_2DRAM;

    ucTableIndex = (ulWarpTableSize >= WARP_TABLE_4K_SIZE)? eTable4k_16x16 : (b32x32Mode == false)? eTable2k : eTable4k_32x32;
    ucTableIndex *= eWarpTableTypeMax;
    ucTableIndex += sYRatio;

#if 1
#ifndef QT_CPP
    if((m_uiWarpingPanel == ePANEL_ID_2688x1472_120HZ) ||
       (m_uiWarpingPanel == ePANEL_ID_1080P_120HZ) ||
       (m_uiWarpingPanel == ePANEL_ID_1080P_240HZ) ||
       (m_uiWarpingPanel == ePANEL_ID_WUXGA_120HZ) ||
       (m_uiWarpingPanel == ePANEL_ID_WUXGA_240HZ))
    {
        ucTableIndex = 9;
    }
    else
    {
        if(Board_SingleBoard_Get() == FALSE)
        {
            ucTableIndex = 6;
        }
        else
        {
            ucTableIndex = 9; //default delay 設最大，避免OSD調整warping畫面閃黑
        }
    }
#endif /* QT_CPP */
#endif /* 0 */

    status &= dvProAV_WarpDelayTableSet(ucWarpDelayTable, ucTableIndex);

    return status;
}

/**
 * @brief Warp Delay Table Set
 * @param [in] ucWarpDelayTable
 *              - 0 : eWarpDelayTable1d3_2DRAM
 *              - 1 : eWarpDelayTable1d3_3DRAM
 *              - 2 : eWarpDelayTable1d9
 * @param [in] ucTableIndex : 0 ~ 9
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpDelayTableSet(WarpDelayTable ucWarpDelayTable, uint8 ucTableIndex)
{
    int status = rcSUCCESS;
    double dWarpDelayRatio = 1;
    uint32 ulVSize = 0;

    if((ucWarpDelayTable >= eWarpDelayTableMax) || (ucTableIndex > 9))
    {
        DBMSG("(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);
        return rcINVALID;
    }

    // calculate warp delay ratio
    status &= dvProAV_AccessRead(eWarpWarpingVSize, &ulVSize);
    dWarpDelayRatio = (double)ulVSize / (double)WARP_DELAY_REF_V_SIZE;

    // Warp delay setting
    status &= dvProAV_AccessWrite(eWcuWarpLnDly, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WarpLnDly * dWarpDelayRatio));  // Line delay from WIP to Warping
    //status &= dvProAV_AccessWrite(eWcuWopLnDly, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WopLnDly * dWarpDelayRatio));  // Line delay from WIP to WOP
    status &= dvProAV_WarpWipLineDlySet((uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WopLnDly * dWarpDelayRatio));
    status &= dvProAV_AccessWrite(eWcuWarpAdaLnDly1, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WarpAdaLnDly1 * dWarpDelayRatio));  // Line delay from WIP to Warping
    status &= dvProAV_AccessWrite(eWcuWopAdaLnDly1, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WopAdaLnDly1 * dWarpDelayRatio));  // Line delay from WIP to WOP
    status &= dvProAV_AccessWrite(eWcuWarpAdaLnDly2, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WarpAdaLnDly2 * dWarpDelayRatio));  // Line delay from WIP to Warping
    status &= dvProAV_AccessWrite(eWcuWopAdaLnDly2, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].WopAdaLnDly2 * dWarpDelayRatio));  // Line delay from WIP to WOP
    status &= dvProAV_AccessWrite(eWarpKernalHoldEn, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].KernalHoldEn));  // Line delay from WIP to Warping
    status &= dvProAV_AccessWrite(eWarpTampdisp4En, (uint32)(warpDelayTable[ucWarpDelayTable][ucTableIndex].Tampdisp4En));  // Line delay from WIP to WOP

    return status;
}

/**
* @brief Warp Auto Delay Enable / Disable Set
* @param [in] bEnable
*             - false : Disable Warp Auto Delay
*             - true : Enable Warp Auto Delay
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_WarpAutoDelayEn(bool bEnable)
{
   return dvProAV_AccessWrite(eWcuAdaWarpDlyEn, bEnable);
}

/**
* @brief Auto Warp Delay Mode Set
* @param [in] bMode
*             - false : Choose delay as short as possible in 3 warping delay mode
*             - true : Add increments to output delay if warping not finish in one frame mode
* @return status : rcSUCCESS / rcERROR / rcINVALID
*/
int dvProAV_WarpDelayMode(bool bMode)
{
   return dvProAV_AccessWrite(eWcuAdaWarpDlyMd, bMode);
}

/**
 * @brief Line Delay From WIP to WOP Set
 * @param [in] uiDelay : delay value
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpWipLineDlySet(uint16 uiDelay)
{
    int status = rcSUCCESS;
    uint32 ulPixelMode = 0;
    uint32 ulVTotal = 0;
    uint16 uiDelayValue = uiDelay;

    status &= dvProAV_AccessRead(REG_WcuCfgA, &ulPixelMode);
    if((ulPixelMode & 0x87) > 0)
    {
        status &= dvProAV_AccessRead(eWcuMb2VTot, &ulVTotal);
        switch((ulPixelMode & 0x8E))
        {
            case 0x80 :     // 2D with pixel shift (2K240 out)
                uiDelayValue = (uint16)(ulVTotal * 0.75);
                break;
            case 0x02 :     // 4K 3D B' (4K60 out)
                uiDelayValue = (uint16)(ulVTotal);
                break;
            case 0x82 :     // 4K 3D B' (2k240 out)
                {
                    uint32 ulFrSeqDb = 0;
                    status &= dvProAV_AccessRead(eVopFrSeqDb, &ulFrSeqDb);
                    if((bool)ulFrSeqDb) // LLRR
                        uiDelayValue = (uint16)(ulVTotal * 1.25);
                    else    // LRLR
                        uiDelayValue = (uint16)(ulVTotal * 1.5);
                }
                break;
            case 0x04 :     // 4K 3D B
            case 0x08 :     // 4K 3D B+ 2Loop (2k240 out) & 4K 3D B+ 4Loop (2k240 out)
                uiDelayValue = (uint16)(ulVTotal * 1.5);
                break;
        }
    }
    status &= dvProAV_AccessWrite(eWcuWopLnDly, uiDelayValue);

    return status;
}

/**
 * @brief Warp Initial
 * @param uiPanelID
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_Warp_Init(uint16 uiPanelID)
{
    int status = rcSUCCESS;

    status &= dvProAV_WarpBypassSet(false);
    status &= dvProAV_WarpScalingSet(eOriRatio, eOriRatio);
    status &= dvProAV_WarpCoeffSet(eWarpCoeff_16);
    status &= dvProAV_WarpYCoeffSet_E4K();
    status &= dvProAV_WarpPreCoeffYSet((uint8 *)WarpPreYCoeff);
    status &= dvProAV_WarpPreFilterYEn(true);
    status &= dvProAV_AccessWrite(eWcuAdaWarpDlyEn, true);      // warp adaptive delay enable

#ifndef QT_CPP
#if 0
#ifdef CONFIG_4K_DISPLAY

    m_uiWarpingPanel = uiPanelID;

    if(m_uiWarpingPanel == PANEL_2D_HIGHSPEED)
    {
        status &= dvProAV_WarpBlkSizeSet(false);
        status &= dvProAV_AccessWrite(eWarpFrac3bits, 0);
        status &= dvProAV_WarpDelaySet(WARP_TABLE_2K_SIZE, (WarpScalingRatio)0, (bool)0);
    }
    else
    {
        status &= dvProAV_WarpBlkSizeSet(true);
        status &= dvProAV_AccessWrite(eWarpFrac3bits, 1);
        status &= dvProAV_WarpDelaySet(WARP_TABLE_4K_SIZE, (WarpScalingRatio)0, (bool)1);
    }
#else
        m_uiWarpingPanel = PANEL_2D_OUTPUT;
        status &= dvProAV_WarpBlkSizeSet(false);
        status &= dvProAV_AccessWrite(eWarpFrac3bits, 0);
        status &= dvProAV_WarpDelaySet(WARP_TABLE_2K_SIZE, (WarpScalingRatio)0, (bool)0);
#endif /* CONFIG_4K_DISPLAY */
#endif /* 0 */

    status &= dvProAV_WarpImageSizeSet(uiPanelID);

#else
    (void)uiPanelID;
#endif /* QT_CPP */

    return status;
}

/**
 * @brief Get Warp Size
 * @param [out] u16WarpSizeH : warp H size
 * @param [out] u16WarpSizeV : warp V size
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WarpSizeGet(uint16 *u16WarpSizeH, uint16 *u16WarpSizeV)
{
    int status = rcSUCCESS;
    uint32 ulGetOutputSize;

    // Read Output Size
    status &= dvProAV_AccessRead(eWcuMb2HSize, &ulGetOutputSize);
    *u16WarpSizeH = (uint16)ulGetOutputSize;
    status &= dvProAV_AccessRead(eWcuMb2VSize, &ulGetOutputSize);
    *u16WarpSizeV = (uint16)ulGetOutputSize;

    return status;
}

int dvProAV_WarpDownScalingIssueWorkAround(void)
{
    int status = rcSUCCESS;

    // temporary solution for 2688x1472 warp table DMA issue
    //status &= dvProAV_AccessWrite(eWarpWpTableWrite, true);
    //MS_SLEEP(20);
    //status &= dvProAV_AccessWrite(eWarpWpTableWrite, false);

    return status;
}

//-------------------------------------------------------------------------------------------------
// WCU
//-------------------------------------------------------------------------------------------------
/**
 * @brief RGB Mask Of Warping Output Set
 * @param [in] ucMask : RGB Mask
 *              -bit0 : R channel Mask
 *              -bit1 : G channel Mask
 *              -bit2 : B channel Mask
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_WcuRgbMaskSet(uint8 ucMask)
{
    return dvProAV_AccessWrite(eWcuRGBMask, ucMask);
}
