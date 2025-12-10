#include "dvC821.h"
#include "dvC821_OSD.h"
#include "utilDbgMsg.h"
//	#ifdef CUSTOM_BARCO     //A65_Owen_0002
//	#include "OSD_BARCO/Custom_OSD.h"
//	#else
//	#include "OSD/Custom_OSD.h"
//	#endif
#ifdef SIMULATOR_ISCALER

#include "Custom_OSD.h"

#define BMP_WIDTH       1920
#define BMP_HIGH        1200
#define BMP_BIT_COLOR   8

// reference https://gist.github.com/hunandy14/143d6036a846d1d53c9cff2be807cf1d

// 檔案結構
#pragma pack(2)
typedef struct
{
    uint16_t bfTybe;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}BmpFileHeader;

typedef struct
{
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes; // 1=defeaul, 0=custom
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter; // 72dpi=2835, 96dpi=3780
    uint32_t biYPelsPerMeter; // 120dpi=4724, 300dpi=11811
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}BmpInfoHeader;

#pragma pack()

#pragma pack(1)

typedef struct
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t Alpha;
}BmpInfoColorPalette;

typedef struct
{
    uint32_t t[32];
}TextInfo;

typedef struct
{
    uint8_t BmpRaw[BMP_HIGH][BMP_WIDTH];
}DrawBmp;


#pragma pack()


// 圖像結構
typedef struct Imgraw {
    uint32_t width, height;
    uint16_t bits;
    uint8* data;
} Imgraw;


BmpInfoColorPalette s_ColorPalette[256];

TextInfo *pTextinfo;
uint8 *pBmpRawData;

DrawBmp Draw_SAD0;
DrawBmp Draw_SAD1;


DrawBmp *POSD_SAD; // OSDSAD(current) point to current OSD memory plane
DrawBmp *POSD_SAD0; // OSDSAD0  OSD memory plane 1
DrawBmp *POSD_SAD1; // OSDSAD1 OSD memory plane 2

uint16_t m_BmpRawHigh  = 0;
uint16_t m_BmpRawWidth = 0;
uint16_t m_BmpRawbit  = 0;



#ifdef CONFIG_4K_DISPLAY
static UINT16 m_panelWidth = 1920;
static UINT16 m_panelHeight = 1080;
#else
static UINT16 m_panelWidth = 1920;
static UINT16 m_panelHeight = 1080;
#endif /* CONFIG_4K_DISPLAY */

static BOOL m_OSDTransparencyEn = false;
static UINT8 m_ucOSDTransparency = false;


int PS_PANEL_1CH_HW;
INT16 m_iInhitbit_Color = 0;
static TickType_t ulOSDStartTicks;

ePANEL_ID OSDePanelTimingId;


void bmpWrite(const char* name, const uint8* raw_img, uint32_t width, uint32_t height, uint16_t bits)
{
    if(!(name && raw_img))
    {
        printf("Error bmpWrite.");
        return;
    }
    // 檔案資訊
    BmpFileHeader file_h =
    {
        .bfTybe=0x4d42,
        .bfReserved1=0,
        .bfReserved2=0,
        .bfOffBits=54,
    };

    file_h.bfSize = file_h.bfOffBits + width*height * bits/8;

    if(bits==8)
    {
        file_h.bfSize+= 1024;
        file_h.bfOffBits+= 1024;
    }

    // 圖片資訊
    BmpInfoHeader info_h =
    {
        .biSize=40,
        .biPlanes=1,
        .biCompression=0,
        .biXPelsPerMeter=0,
        .biYPelsPerMeter=0,
        .biClrUsed=0,
        .biClrImportant=0,
    };

    info_h.biWidth = width;
    info_h.biHeight = height;
    info_h.biBitCount = bits;
    info_h.biSizeImage = width*height * bits/8;

    if(bits == 8)
    {
        info_h.biClrUsed=256;
    }
    // 寫入檔頭
    FILE *pFile = NULL;
    pFile = fopen(name,"wb+");

    if(!pFile)
    {
        printf("Error opening file.");
        return;
    }

    fwrite((char*)&file_h, sizeof(char), sizeof(file_h), pFile);
    fwrite((char*)&info_h, sizeof(char), sizeof(info_h), pFile);
    // 寫調色盤
    if(bits == 8)
    {
        fwrite((char*)&s_ColorPalette, sizeof(s_ColorPalette), 1, pFile);
#if 0
        for(unsigned i = 0; i < 256; ++i)
        {
            uch c = i;
            fwrite((char*)&c, sizeof(char), sizeof(uch), pFile);
            fwrite((char*)&c, sizeof(char), sizeof(uch), pFile);
            fwrite((char*)&c, sizeof(char), sizeof(uch), pFile);
            fwrite("", sizeof(char), sizeof(uch), pFile);
        }
#endif /* 0 */
    }

    // 寫入圖片資訊
    size_t alig = ((width*bits/8)*3) % 4;
    for(int j = height-1; j >= 0; --j)
    {
        for(unsigned i = 0; i < width; ++i)
        {
            uint32_t idx = j*width +i;

            if(bits == 24)
            {
                fwrite((char*)&raw_img[idx*3 +2], sizeof(char), sizeof(uint8), pFile);
                fwrite((char*)&raw_img[idx*3 +1], sizeof(char), sizeof(uint8), pFile);
                fwrite((char*)&raw_img[idx*3 +0], sizeof(char), sizeof(uint8), pFile);
            }
            else if(bits == 8)
            {
                fwrite((char*)&raw_img[idx], sizeof(char), sizeof(uint8), pFile);
            }
        }
        // 對齊4byte
        for(size_t i = 0; i < alig; ++i)
        {
            fwrite("", sizeof(char), sizeof(uint8), pFile);
        }
    }
    fclose(pFile);
}

void bmpRead(const char* name, uint8** raw_img, uint32_t* width, uint32_t* height, uint16_t* bits)
{
    if(!(name && raw_img && width && height && bits))
    {
        printf("Error bmpRead.");
        return;
    }
    // 檔案資訊
    BmpFileHeader file_h;
    // 圖片資訊
    BmpInfoHeader info_h;
    // 讀取檔頭
    FILE *pFile = NULL;
    pFile = fopen(name, "rb+");
    //fopen_s(&pFile, name, "rb+");
    if(!pFile)
    {
        system("Error opening file.");
        return;
    }
    fread((char*)&file_h, sizeof(char), sizeof(file_h), pFile);
    fread((char*)&info_h, sizeof(char), sizeof(info_h), pFile);
    // 讀取長寬
    *width = info_h.biWidth;
    *height = info_h.biHeight;
    *bits = info_h.biBitCount;
    size_t ImgSize = ((size_t)*width) * ((size_t)*height) * 3;

    *raw_img = (uint8*)calloc(ImgSize, sizeof(uint8));

    m_BmpRawHigh = (UINT16)info_h.biHeight;
    m_BmpRawWidth = (UINT16)info_h.biWidth;
    m_BmpRawbit = (UINT16)info_h.biBitCount;
    pBmpRawData = *raw_img;

    printf("width  = %d\n", (int)info_h.biWidth);
    printf("height = %d\n", (int)info_h.biHeight);
    printf("bits   = %d\n", (int)info_h.biBitCount);

    printf("pBmpRawData   = %d\n", (int)pBmpRawData);

    printf("raw_img   = %d, size = %d\n", (int)raw_img, ImgSize);
    // 讀取讀片資訊轉RAW檔資訊
    if(info_h.biBitCount == 8)
    {
        //fread((char*)&s_ColorPalette, sizeof(s_ColorPalette), 1, pFile);

        //for(int count = 0; count < 256; count++)
        //{
            //printf("%3d, %3d, %3d, %3d\n",s_ColorPalette[count].r, s_ColorPalette[count].g, s_ColorPalette[count].b,  s_ColorPalette[count].Alpha);
        //}
    }
    else
    {
        fseek(pFile, file_h.bfOffBits, SEEK_SET);
    }

    size_t alig = ((info_h.biWidth*info_h.biBitCount/8)*3) % 4;

    for(int j = *height-1; j >= 0; --j)
    {
        for(unsigned i = 0; i < *width; ++i)
        {
            uint32_t idx = j*(*width)+i;

            if(*bits == 24)
            { // RGB圖片
                fread((char*)&(*raw_img)[idx*3 +2],
                    sizeof(char), sizeof(uint8), pFile);
                fread((char*)&(*raw_img)[idx*3 +1],
                    sizeof(char), sizeof(uint8), pFile);
                fread((char*)&(*raw_img)[idx*3 +0],
                    sizeof(char), sizeof(uint8), pFile);
            }
            else if(*bits == 8)
            {
                fread((char*)&(*raw_img)[idx], sizeof(char), sizeof(uint8), pFile);
            }
        }
        fseek(pFile , (long)alig , SEEK_CUR);
    }
    fclose(pFile);
}

void Imgraw_Read(const Imgraw* _this, const char* name)
{
    const Imgraw* p = _this;
    bmpRead(name, &p->data, &p->width, &p->height, &p->bits);
}

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

void OSD_DMA_SerialFlash_Set( UINT32 ulSflAd, UINT32 ulDestMemAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
    LOG_MSG(db_DV_SCALER, "DMA_SerialFlash Done (X%08X)(X%08X)(%02d)(X%08X)\r\n", ulSflAd, ulDestMemAd, ucRegsel, ulDmaCnt);
}

void OSD_DMA_DDR3_WriteTo_Flash( UINT32 ulSrcMemAd, UINT32 ulSflAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
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


}


/******************  osd_acct for C734  *************************
    ucControlData     --> OSD access control
*************************************************************************/
static void OSD_AccessControl( UINT8 ucControlData )
{

}


static void OSD_Swap_Memplane(void)
{
    int status = rcSUCCESS;

    POSD_SAD = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;   // Swap Memory Address

    memset(POSD_SAD, sizeof(DrawBmp), m_iInhitbit_Color);

    return (status == rcSUCCESS);

}


/******************  osd_blink_ct for C734  ************************
    cEnable            ->  blink enable
    on_period     ->  blink ON period(0.1sec yp 10sec on POCLK freq=150MHz)
    off_period    ->  blink OFF period
    cBlinkTp_Enable       ->  blink transparent color enable
*************************************************************************/
void OSD_Blink_Control(INT8 cEnable, DOUBLE dOn_period, DOUBLE dOff_period, INT8 cBlinkTp_Enable)
{

}


INT8 OSD_Palette_Set(INT16 iAmount, UINT8 *pucData )
{
    int status = rcSUCCESS;

    INT16 count;
    INT16 iData = 0;

    if (iAmount != ICHIP_PALETTE_COLOR)
    {
       return rcERROR;
    }


    for( count=0; count < iAmount; count++ )
    {
        s_ColorPalette[count].r = *(pucData+iData++);
        s_ColorPalette[count].g = *(pucData+iData++);
        s_ColorPalette[count].b = *(pucData+iData++);
        s_ColorPalette[count].Alpha = 0;
        //printf("%3d, %3d, %3d, %3d\n",s_ColorPalette[count].r, s_ColorPalette[count].g, s_ColorPalette[count].b,  s_ColorPalette[count].Alpha);
    }


    return (status == rcSUCCESS);
}



/******************  osd_copy for C734  ************************
    source_adrs   ->  start address
    dest_adrs     ->  destination address
    bbhw          ->  bbhw
    bbvw          ->  bbvw
*************************************************************************/
void OSD_Copy(UINT32 source_adrs, UINT32 dest_adrs, INT16 ibbHor, INT16 ibbVer)
{
}

void OSD_CurrentCopy(void) //A70LV_Larry_0067
{
    int status = rcSUCCESS;

    DrawBmp *source_adrs;
    DrawBmp *dest_adrs = POSD_SAD;

    source_adrs = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;

    memcpy(dest_adrs, source_adrs, sizeof(DrawBmp));

    return (status == rcSUCCESS);
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
    return 0;
}

eRESULT dvC821_RAM_Read(UINT32 ulRAMAd, UINT8 *pData, UINT32 ulDataSize) //A35G2_CDS_Larry_0050
{
    return rcSUCCESS;
}

eRESULT dvC821_RAM_Write(UINT32 ulRAMAd, UINT8 *pData, UINT32 ulDataSize) //A35G2_CDS_Larry_0050
{
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

}


/******************  osd_write_inh_ct for C734  *************************
    winh_en      --> OSD write-inhibit color enable (0 or 1)
    inh_color    --> OSD write-inhibit color (color pallet number)
*************************************************************************/
void dvC821_Inhibit_Color_Set( INT8 cEnable, INT16 iInhibit_Color )
{
    INT8 status = rcSUCCESS;

    m_iInhitbit_Color = iInhibit_Color;

    return (status == rcSUCCESS);

}


void dvC821_OSD_Off(void)
{

}

void dvC821_OSD_On( START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color)
{
    int status = rcSUCCESS;

    bmpWrite("/tmp/output.bmp", POSD_SAD, BMP_WIDTH, BMP_HIGH, 8);


    system("eog -w /tmp/output.bmp &");


    ASSERT(status == rcSUCCESS);

    OSD_Swap_Memplane();  //Swap memory plane automatically after OSD drawing every time

    //return (status == rcSUCCESS);
}


/******************  osd_bias for C734  *************************/
/****************************************************************
    rbias     ->  Red   bias (-128 to 127)
    gbias     ->  Green bias (-128 to 127)
    bbias     ->  Blue  bias (-128 to 127)
 ****************************************************************/
void dvC821_OSD_Bias_Set(OSD_BIAS_COLOR sBiasSetting)
{


}


/******************  osd_gain for C734  *************************
//  en        ->  gain enable
    rgain     ->  gain for red   (0 to 3.99)
    ggain     ->  gain for green (0 to 3.99)
    bgain     ->  gaub fir blue  (0 to 3.99)
*************************************************************************/
void dvC821_OSD_Gain_Set( INT8 cEnable, OSD_GAIN_COLOR sGainSetting)
{

}



INT8 dvC821_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color)
{
    int status = rcSUCCESS;
    Imgraw img = {0, 0, 0, NULL};

    FILE *pFile = NULL;
    uint32_t size;

    pFile = fopen(OSD_TEXT_RAWDATA, "rb+");

    if(pFile == NULL)  //G100_Simon_0092
    {
        LOG_MSG(db_DV_SCALER_OSD, "Can not fopen %s\n", OSD_TEXT_RAWDATA);
        status &= rcERROR;
    }
    else
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        pTextinfo = malloc(size);

        fread(pTextinfo, sizeof(uint8_t), size, pFile);

        fclose(pFile);
    }

    pFile = fopen(OSD_BITMAP_RAWDATA, "rb+");

    if(pFile == NULL)  //G100_Simon_0092
    {
        LOG_MSG(db_DV_SCALER_OSD, "Can not fopen %s\n", OSD_BITMAP_RAWDATA);
        status &= rcERROR;
    }
    else
    {
        fseek(pFile, 0, SEEK_END);
        size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        pBmpRawData = malloc(size);

        fread(pBmpRawData, sizeof(uint8_t), size, pFile);

        fclose(pFile);
    }

    m_BmpRawHigh = BITMAP_RAW_HEIGHT;
    m_BmpRawWidth = BITMAP_RAW_WIDTH;
    m_BmpRawbit = 8;

    //Imgraw_Read(&img, "./OSD/savedImage_output_index8.bmp");

    OSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);


    POSD_SAD0 = &Draw_SAD0;
    POSD_SAD1 = &Draw_SAD1;

    memset(POSD_SAD0, 0, sizeof(DrawBmp));  //G100_Simon_0058
    memset(POSD_SAD1, 0, sizeof(DrawBmp));

    POSD_SAD = POSD_SAD0;

    return (status == rcSUCCESS);
}


INT8 dvC821_OSD_Reload(void)
{
    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_Memory_Protect(UINT8 ucEnable) //A35G2_CDS_Larry_0050
{
    return ICHIP_OSD_PASS;
}

#if (LOGO_REPLACE == 1)
INT8 dvC821_2ND_LOGO_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color, UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0068 //A65_OPTOMA_Julie_0067
{
    int status = rcSUCCESS;

    return (status == rcSUCCESS);
}

INT8 dvC821_OSD_2ND_LOGO_Reload(UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
{
    return ICHIP_OSD_PASS;
}
#endif

INT8 dvC821_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;

    uint32_t RawBmpPosition = 0;
    uint32_t stringH = 0;
    uint8_t  cData;

    for(INT16 iY = sDes_position.iY; iY < (sDes_position.iY + sBitmap.sBitmap_Size.iHeight); iY++)
    {
       RawBmpPosition = (sBitmap.sSrc_Position.iY * m_BmpRawWidth) + sBitmap.sSrc_Position.iX;

       for(INT16 iX = sDes_position.iX; iX < (sDes_position.iX + sBitmap.sBitmap_Size.iWidth); iX++)
       {
	        if((iX < m_panelWidth) && (iY < m_panelHeight))
			{
                if(m_BmpRawbit == 8)
                {
                    cData = pBmpRawData[RawBmpPosition++];
                    if(cData != m_iInhitbit_Color)
                    {
                        POSD_SAD->BmpRaw[iY][iX] = cData;
                    }
	            }
            }
       }
       sBitmap.sSrc_Position.iY = sBitmap.sSrc_Position.iY + 1;
    }


    ASSERT(status == rcSUCCESS);

    return (status == rcSUCCESS);
}


INT8 dvC821_Paint_Specified_Bitmap(OSD_BITMAP sBitmap, START_POINT sOffset_position, RECT_SIZE sSrc_Size, START_POINT sDes_position)
{
    INT16 iBmhw, iBmvw, iSrc_x, iSrc_y;
    uint32_t RawBmpPosition = 0;
    uint8_t  cData;

    iSrc_x = sBitmap.sSrc_Position.iX + sOffset_position.iX;
    iSrc_y = sBitmap.sSrc_Position.iY + sOffset_position.iY;
    iBmhw = sSrc_Size.iWidth;
    iBmvw = sSrc_Size.iHeight;

    for(INT16 iY = sDes_position.iY; iY < (sDes_position.iY + iBmvw); iY++)
    {
        RawBmpPosition = (iSrc_y * m_BmpRawWidth) + iSrc_x;

        for(INT16 iX = sDes_position.iX; iX < (sDes_position.iX + iBmhw); iX++)
        {
             if((iX < m_panelWidth) && (iY < m_panelHeight))
			{
                if(m_BmpRawbit == 8)
                {
                    cData = pBmpRawData[RawBmpPosition++];
                    if(cData != m_iInhitbit_Color)
                    {
                        POSD_SAD->BmpRaw[iY][iX] = cData;
                    }
	            }
            }
        }
        iSrc_y = iSrc_y + 1;
    }

    return ICHIP_OSD_PASS;
}
INT8 dvC821_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index)
{
    int status = rcSUCCESS;
    uint8 wData[6];

    if((sDes_Size.iWidth == 0) || (sDes_Size.iHeight == 0))
    {
        return rcERROR;
    }

    for(INT16 iY = sDes_position.iY; iY < (sDes_position.iY + sDes_Size.iHeight); iY++)
    {
        for(INT16 iX = sDes_position.iX; iX < (sDes_position.iX + sDes_Size.iWidth); iX++)
        {
            if(m_BmpRawbit == 8)
            {
                POSD_SAD->BmpRaw[iY][iX] = (uint8_t)iColor_Index;
            }
        }
    }

    ASSERT(status == rcSUCCESS);

    return (status == rcSUCCESS);
}



INT8 dvC821_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset)
{
    int status = rcSUCCESS;

    uint8_t stringV = 0;
    uint8_t stringH = 0;

    for(uint16_t count = 0; count < sString.uiStringLength; count++)
    {
        stringV = 0;

        for(INT16 iY = sDes_position.iY; iY < (sDes_position.iY + sString.ucStringMaxHeight); iY++)
        {
            for(stringH = 0;  stringH < *(sString.pucCharWdith + count + iStringOffset); stringH++)
            {
                if(m_BmpRawbit == 8)
                {
                    if(((pTextinfo[sString.ulFontOffset + *(sString.puiCharIndex + count + iStringOffset)].t[stringV] >> stringH) & 0x0001))
                    {
                        POSD_SAD->BmpRaw[iY][sDes_position.iX + stringH] = (uint8_t)iFont_Color;
                        //printf("@");
                    }
                    else
                    {
                        //BmpRaw[iY][sDes_position.iX + stringH] = (uint8_t)iBG_Color;
                        //printf("_");
                    }
                }
            }

            stringV++;
        }

        sDes_position.iX = sDes_position.iX + *(sString.pucCharWdith + count + iStringOffset);

    }


    ASSERT(status == rcSUCCESS);

    return (status == rcSUCCESS);
}


void dvC821_OSD_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor) //A70LV_Larry_0055
{


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
            m_panelWidth = 1920;
            m_panelHeight = 1200;
            break;


        case ePANEL_ID_1080P_60HZ:
            m_panelWidth = 1920;
            m_panelHeight = 1080;
            break;

        case ePANEL_ID_1080P_120HZ:
            m_panelWidth = 1920;
            m_panelHeight = 1080;
            break;

        default:
            m_panelWidth = 1920;
            m_panelHeight = 1080;
            break;
    }
    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_MenuTransparencySet(UINT8 ucValue)     //A70LV_Doulas_0122
{
    return ICHIP_OSD_PASS;
}

INT8 dvC821_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe)     //A70LV_Doulas_0122
{
    return ICHIP_OSD_PASS;
}
#endif

