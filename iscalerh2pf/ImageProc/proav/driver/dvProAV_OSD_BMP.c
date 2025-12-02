#include "dvProAV_OSD.h"
#include "dvProAV_Base.h"
#include "dvProAV_SerialFlash.h"
#ifndef QT_CPP
    #include "dvProAV.h"
    //#include "Custom_OSD.h"
#endif

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

#ifdef SIMULATOR_ISCALER
#define BMP_BIT_COLOR   8
#define BMP_WIDTH       (1920 * BMP_BIT_COLOR/8)
#define BMP_HIGH        (1080)

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
                fwrite((char*)&raw_img[idx*3 + 2], sizeof(char), sizeof(uint8), pFile);
                fwrite((char*)&raw_img[idx*3 + 1], sizeof(char), sizeof(uint8), pFile);
                fwrite((char*)&raw_img[idx*3 + 0], sizeof(char), sizeof(uint8), pFile);
            }
            else if(bits == 16)
            {
                fwrite((char*)&raw_img[idx*2 + 1], sizeof(char), sizeof(uint8), pFile);
                fwrite((char*)&raw_img[idx*2 + 0], sizeof(char), sizeof(uint8), pFile);
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

    printf("width  = %d\n", info_h.biWidth);
    printf("height = %d\n", info_h.biHeight);
    printf("bits   = %d\n", info_h.biBitCount);

    printf("pBmpRawData   = %d\n", pBmpRawData);

    printf("raw_img   = %d, size = %d\n", raw_img, ImgSize);
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

static int OSD_DMA_SerialFlash_Set( UINT32 ulSflAd, UINT32 ulDestMemAd, UINT8 ucRegsel, UINT32 ulDmaCnt )
{
    int status = rcSUCCESS;





    return (status == rcSUCCESS);
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

static int OSD_Swap_Memplane(void)
{
    int status = rcSUCCESS;

    //START_POINT sDes_position;
    //RECT_SIZE sDes_Size;

    POSD_SAD = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;   // Swap Memory Address

    memset(POSD_SAD, 252, sizeof(DrawBmp));

    //sDes_position.iX = 0;
    //sDes_position.iY = 0;
    //sDes_Size.iHeight = BMP_HIGH;
    //sDes_Size.iWidth = BMP_WIDTH;

    //dvProAV_Paint_Rectangle(sDes_position, sDes_Size, 252);

    ASSERT(status == rcSUCCESS);

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

int OSD_Palette_Set(INT16 iAmount, UINT8 *pucData )
{
    int status = rcSUCCESS;

    INT16 count;
    INT16 iData = 0;

    if (iAmount != PROAV_PALETTE_COLOR)
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

int OSD_CurrentCopy(void) //A70LV_Larry_0067
{
    int status = rcSUCCESS;

    DrawBmp *source_adrs;
    DrawBmp *dest_adrs = POSD_SAD;

    source_adrs = ( POSD_SAD != POSD_SAD0 ) ? POSD_SAD0 : POSD_SAD1;

    memcpy(dest_adrs, source_adrs, sizeof(DrawBmp));

    return (status == rcSUCCESS);
}


// ==============================================================================
// FUNCTION NAME: dvPro_SerialFlash_CheckSum
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
UINT32 dvProAV_SerialFlash_CheckSum(UINT32 ulSflAd, UINT32 ulDataSize)
{
    return 0;
}


// ==============================================================================
// FUNCTION NAME: dvPro_SerialFlash_Erase_All
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
void dvProAV_SerialFlash_Erase_All(void)
{
	return;
}

// ==============================================================================
// FUNCTION NAME: dvPro_SerialFlash_Erase_Sector
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
void dvProAV_SerialFlash_Erase_Sector(UINT32 ulSflAd, UINT32 ulDataSize)
{
    return;
}


// ==============================================================================
// FUNCTION NAME: dvPro_SerialFlash_Write
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
void dvProAV_SerialFlash_Write(UINT32 ulSflAd, UINT8 *pucBuffer, UINT32 ulDataSize)
{

}


/******************  osd_write_inh_ct for C734  *************************
    winh_en      --> OSD write-inhibit color enable (0 or 1)
    inh_color    --> OSD write-inhibit color (color pallet number)
*************************************************************************/
int dvProAV_Inhibit_Color_Set(INT8 cEnable, UINT16 iInhibit_Color)
{
    INT8 status = rcSUCCESS;

    m_iInhitbit_Color = iInhibit_Color;


    return (status == rcSUCCESS);

}

int dvProAV_OSD_SW_Reset(bool bEnable)
{
    int status = rcSUCCESS;
    return (status == rcSUCCESS);
}


int dvProAV_OSD_Enable(bool bEnable)
{
    int status = rcSUCCESS;

    return (status == rcSUCCESS);
}

bool dvProAV_OSD_EnableGet(void)
{
    return (bool)rcSUCCESS;
}

int dvProAV_OSD_On( START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color)
{
    int status = rcSUCCESS;


    bmpWrite("/tmp/output.bmp", POSD_SAD, 1920, BMP_HIGH, BMP_BIT_COLOR);

    //system("eog -w /tmp/output.bmp &");

	SYSTEM_CALL("eog -w /tmp/output.bmp & > /dev/null");

    ASSERT(status == rcSUCCESS);

    OSD_Swap_Memplane();  //Swap memory plane automatically after OSD drawing every time

    return (status == rcSUCCESS);
}

int dvProAV_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color)
{
    int status = rcSUCCESS;
    Imgraw img = {0, 0, 0, NULL};

    FILE *pFile = NULL;
    uint32_t size;

    pFile = fopen(OSD_TEXT_RAWDATA, "rb+");

    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    pTextinfo = malloc(size);

    fread(pTextinfo, sizeof(uint8_t), size, pFile);

    fclose(pFile);

    pFile = fopen(OSD_BITMAP_RAWDATA, "rb+");

    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    pBmpRawData = malloc(size);

    fread(pBmpRawData, sizeof(uint8_t), size, pFile);

    fclose(pFile);

    m_BmpRawHigh = 2448;
    m_BmpRawWidth = 1920;
    m_BmpRawbit = BMP_BIT_COLOR;

    //Imgraw_Read(&img, "./OSD/savedImage_output_index8.bmp");

    OSD_Palette_Set(iPalette_ColorAmount, pucPaletteData);


    POSD_SAD0 = &Draw_SAD0;
    POSD_SAD1 = &Draw_SAD1;

    memset(POSD_SAD0, 0, sizeof(DrawBmp)); //A70LK_Casper_0001
    memset(POSD_SAD1, 0, sizeof(DrawBmp)); //A70LK_Casper_0001

    POSD_SAD = POSD_SAD0;

    return (status == rcSUCCESS);
}


int dvProAV_OSD_ReLoad(void)
{
    return rcSUCCESS;
}


int dvProAV_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
	int status = rcSUCCESS;

	uint32_t RawBmpPosition = 0;
	uint32_t stringH = 0;
	uint8_t  cData;
	uint8_t  r,g,b;

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
                else if(m_BmpRawbit == 16)
                {
                    cData = pBmpRawData[RawBmpPosition++];
                    if(cData != m_iInhitbit_Color)
			        {
                        r = s_ColorPalette[cData].r;
                        g = s_ColorPalette[cData].g;
                        b = s_ColorPalette[cData].b;


			            POSD_SAD->BmpRaw[iY][(iX*2)+0] = (r & 0xF8) | (g >> 5);
			            POSD_SAD->BmpRaw[iY][(iX*2)+1] = ((g & 0x1C) << 3) | (b  >> 3);
			        }
                }
			}
		}
		sBitmap.sSrc_Position.iY = sBitmap.sSrc_Position.iY + 1;
	}


    ASSERT(status == rcSUCCESS);

    return (status == rcSUCCESS);
}



int dvProAV_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index)
{
    int status = rcSUCCESS;
    uint8 wData[6];
    uint8_t r,g,b;

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
            else if(m_BmpRawbit == 16)
            {
                r = (s_ColorPalette[iColor_Index].r >> 3) & 0x1F;
                g = (s_ColorPalette[iColor_Index].g >> 2) & 0x3F;
                b = (s_ColorPalette[iColor_Index].b >> 3) & 0x1F;

                POSD_SAD->BmpRaw[iY][(iX*2)+0] = ((r << 3) & 0xf8) | ((g >> 3) & 0x07);
			    POSD_SAD->BmpRaw[iY][(iX*2)+1] = ((g << 5) & 0xe0) | (b & 0x1F);
            }
        }
    }

    ASSERT(status == rcSUCCESS);

    return (status == rcSUCCESS);
}



int dvProAV_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset)
{
    int status = rcSUCCESS;

    uint8_t stringV = 0;
    uint8_t stringH = 0;
	uint8_t  r,g,b;

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
                else if(m_BmpRawbit == 16)
                {
                    r = (s_ColorPalette[iFont_Color].r >> 3) & 0x1F;
                    g = (s_ColorPalette[iFont_Color].g >> 2) & 0x3F;
                    b = (s_ColorPalette[iFont_Color].b >> 3) & 0x1F;

                    if(((pTextinfo[sString.ulFontOffset + *(sString.puiCharIndex + count + iStringOffset)].t[stringV] >> stringH) & 0x0001))
                    {
                        POSD_SAD->BmpRaw[iY][((sDes_position.iX + stringH)*2)+0] = ((r << 3) & 0xf8)| ((g >> 3) & 0x07);
			            POSD_SAD->BmpRaw[iY][((sDes_position.iX + stringH)*2)+1] = ((g << 5) & 0xe0) | (b & 0x1F);
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
			case ePANEL_ID_1080P_240HZ:
				{
					m_panelWidth = 1920;
					m_panelHeight = 1080;
				}
				break;

			case ePANEL_ID_1080P_120HZ:
				{
					m_panelWidth = 1920;
					m_panelHeight = 1080;
				}
				break;

			case ePANEL_ID_2688x1472_120HZ:
				{
					m_panelWidth = 2688 / 2;
					m_panelHeight = 1472 / 2;

				}
				break;

			case ePANEL_ID_2560x1440_60HZ:
				{
					m_panelWidth = 2560/2;
					m_panelHeight = 1440/2;
				}
				break;

			case ePANEL_ID_3840x2160_50HZ:
			case ePANEL_ID_3840x2160_60HZ:
			case ePANEL_ID_3840x2160_30HZ:
				{
					m_panelWidth = 1920;
					m_panelHeight = 1080;
				}
				break;

			default:
				m_panelWidth = 1920;
				m_panelHeight = 1080;
				break;
		}


    return rcSUCCESS;
}

int dvProAV_OSD_MenuTransparencySet(UINT8 ucValue)     //A70LV_Doulas_0122
{
    int status = rcSUCCESS;

    (void)ucValue;

    return (status == rcSUCCESS);
}


int dvProAV_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe)     //A70LV_Doulas_0122
{
    int status = rcSUCCESS;

    (void)ucEnalbe;

    return (status == rcSUCCESS);
}

int dvProAV_OSD_FillColor(UINT16 iColor_Index)
{
    int status = rcSUCCESS;
    uint8 cData[20];

    if((m_panelHeight == 0) || (m_panelWidth == 0))
    {
        return rcERROR;
    }

    for(INT16 iY = 0; iY < m_panelHeight; iY++)
    {
        for(INT16 iX = 0; iX < m_panelWidth; iX++)
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

void dvProAV_OSD_DoubleSizeSet(bool bDouble, bool bDoubleX, bool bDoubleY)
{
    (void)bDouble;(bool)bDoubleX;(bool)bDoubleY;
}

//-------------------------------------------------------------------------------------------------
// Logo Capture
//-------------------------------------------------------------------------------------------------
void dvProAV_OSD_LogoCaptureLoad(void)
{
    //logo capture
}

#if 1
//-------------------------------------------------------------------------------------------------
// Warp OSD For Entry 4K
//-------------------------------------------------------------------------------------------------
static uint16 m_panelWidth_WP = 3840;
static uint16 m_panelHeight_WP = 2160;

//before warping
static uint32 PWPOSD_SAD_BEFORE;    // PWPOSD_SAD_BEFORE point to current OSD memory plane berfor warping
static uint32 PWPOSD_SAD1;          // OSDSAD0 OSD memory plane 0
static uint32 PWPOSD_SAD2;          // OSDSAD1 OSD memory plane 1

//after warping
static uint32 PWPOSD_SAD_AFTER;     // PWPOSD_SAD_AFTER point to current OSD memory plane after warping
static uint32 PWPOSD_SAD3;          // OSDSAD0 OSD memory plane 0
static uint32 PWPOSD_SAD4;          // OSDSAD1 OSD memory plane 1

/**
 * @brief Get WarpOSD busy state
 * @return BusyState : 0=unbusy, 1=busy
 */
static bool dvProAV_WarpOSD_BusyStateGet_E4K(void)
{
    return 0;
}

/**
 * @brief Inhibit Color Set
 * @param [in] cEnable : Warp OSD write-inhibit color enable (0 or 1)
 * @param [in] iInhibit_Color : Warp OSD write-inhibit color (color pallet number)
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
static INT8 dvProAV_WarpOSD_Inhibit_Color_Set_E4K(INT8 cEnable, uint16 iInhibit_Color)
{
    int status = rcSUCCESS;

    m_iInhitbit_Color = iInhibit_Color;

    (void)cEnable;

    return (status == rcSUCCESS);
}

/**
 * @brief Page Clear
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_PageClear_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    return (status == rcSUCCESS);
}

int dvProAV_WarpOSD_Palette_Set_E4K(INT16 iAmount, UINT8 *pucData)
{
    int status = rcSUCCESS;

    (void)iAmount;
    (void)pucData;
    return (status == rcSUCCESS);
}

/**
 * @brief select warp osd page
 * @param [in] eWPOSDPAGE : select warp osd page
 *                  - eWPOSDPAGE_0 : in eLayerMode_P0BeforeP1AfterWarp mode = before warp
 *                  - eWPOSDPAGE_1 : in eLayerMode_P0BeforeP1AfterWarp mode = after warp
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_MemplaneSel_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;


    return (status == rcSUCCESS);
}

int dvProAV_WarpOSD_Swap_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    return (INT8)status;
}

/**
 * @brief Paint Rectangle
 * @param [in] sDes_position : Rectangle start position
 * @param [in] sDes_Size : Rectangle size
 * @param [in] iColor_Index
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Paint_Rectangle_E4K(eWPOSDPAGE ePage, START_POINT sDes_position, RECT_SIZE sDes_Size, uint16 iColor_Index)
{
    int status = rcSUCCESS;

    return (status == rcSUCCESS);
}

/**
 * @brief 將DRAM中的Bmp複製到OSD畫面的位置
 * @param [in] ulSrc_Addr : setting read base address
 * @param [in] sBitmap : source Bitmap size & start position
 * @param [in] sDes_position : paint Bitmap start position
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_CurrentCopy_E4K(eWPOSDPAGE ePage, OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;

    (void)ePage;
    (void)sBitmap ;
    (void)sDes_position;

    return (status == rcSUCCESS);
}

int dvProAV_WarpOSD_Paint_Bitmap_E4K(OSD_BITMAP sBitmap, START_POINT sDes_position)
{
    int status = rcSUCCESS;

    (void)sBitmap;
    (void)sDes_position;

    return (status == rcSUCCESS);
}

/**
 * @brief 畫單一圓型
 * @param [in] sDes_center : setting circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] iColor_Index : setting circle color
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 * @note 圓最小半徑是10, 小於10會不圓, 不支援半徑大於原心case
 */
int dvProAV_WarpOSD_Paint_SingleCircle_E4K(START_POINT sDes_center, INT16 iRadius, uint16 iColor_Index)
{
    int status = rcSUCCESS;
    (void)sDes_center;
    (void)iRadius;
    (void)iColor_Index;

    return (status == rcSUCCESS);
}

/**
 * @brief 畫多個圓型
 * @param [in] sDes_center : setting top left circle center position
 * @param [in] iRadius : setting circle Radius
 * @param [in] sPitch : setting pitch for circle center
 * @param [in] sLimit : setting paint multi Circle boundary
 * @param [in] iColor_Index : setting circle color
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Paint_MultiCircle_E4K(START_POINT sDes_center, INT16 iRadius, CENTER_PITCH sPitch, START_POINT sLimit, uint16 iColor_Index)
{
    int status = rcSUCCESS;

    (void)sDes_center;
    (void)iRadius;
    (void)sPitch;
    (void)sLimit;
    (void)iColor_Index;


    return (status == rcSUCCESS);
}

/**
 * @brief after & before WarpOSD Mix
 * @param [in] bEnable : true=on false=off
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_MixOn_E4K(bool bEnable)
{
    int status = rcSUCCESS;

    (uint16)bEnable;

    return (status == rcSUCCESS);
}

/**
 * @brief WarpOSD Display
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_On_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    (void)ePage;

    return (status == rcSUCCESS);
}

/**
 * @brief Warp OSD Initial
 * @param [in] iPalette_ColorAmount : 設定調色盤數量
 * @param [in] pucPaletteData : 設定調色盤
 * @param [in] ucInhibit_Color : 設定禁止色
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Init_E4K(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, uint16 ucInhibit_Color)
{
    int status = rcSUCCESS;

    (void)iPalette_ColorAmount;
    (void)pucPaletteData;
    (void)ucInhibit_Color;

    return (status == rcSUCCESS);
}

/**
 * @brief dvProAV_WarpOSD_Set_E4K
 * @param [in] bE4K : true/fales : Warp OSD/Scaler OSD
 * @return status : 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_WarpOSD_Set_E4K(bool bWarpOsd)
{
    int status = rcSUCCESS;

    (void)bWarpOsd;


    return (status == rcSUCCESS);
}

int dvProAV_WarpOSD_MemplaneLastSel_E4K(eWPOSDPAGE ePage)
{
    int status = rcSUCCESS;

    (void)ePage;

    return (status == rcSUCCESS);
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


#endif
void dvProAV_OSD_Buffer_Init(VOID)
{
    // 切換OSD DRAM Buffer
    OSD_Swap_Memplane();  //set OSD memory plane to second and reset that.
    OSD_Swap_Memplane();  //set OSD memory plane to first and reset that

    return;
}

bool dvProAV_OSD_4k3dEn_Get(VOID)
{
    return false;
}

void dvProAV_WarpOSDSubFrameModFlagSet_E4K(bool bSubFrameMod)
{
    (void)bSubFrameMod;
}

void dvProAV_OSD_DbgPrint(void)
{

}

#endif
