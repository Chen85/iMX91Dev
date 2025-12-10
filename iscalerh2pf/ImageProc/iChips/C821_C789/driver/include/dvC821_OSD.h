#ifndef _DVC821_GUI_H_
#define _DVC821_GUI_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"


#define BITMAP_RAW_WIDTH     1920
#define BITMAP_RAW_HEIGHT    2900

//H2 wait review
#define DV_BITMAP_RAW_DATA_SIZE (0x0054F600)
#define DV_TEXT_RAW_DATA_SIZE   (0x00124480)
#define DV_BITMAP_JSON_SIZE     (0x0000445C)
#define DV_TEXT_JSON_SIZE       (0x00482564)
#define DV_OSD_STRUCT_SIZE      (0x00142AA0)


#define DVC821_OSD_SIZE_EXPAND  //Note: not include OSD display area //T100_Sander_0012
#define DVC821_OSD_BITMAP_SKETCHPAD_WIDTH  (BITMAP_RAW_WIDTH / 128) //modify Bitblt read line feed to 0x0F(15x128= 1920) to fit bmp width, this value should read from Json define //T100_Sander_0019

#define ICHIP_OSD_PASS 1
#define ICHIP_OSD_INIT_FAIL -1
#define PALETTE_AMOUNT_ERROR -2


#define ICHIP_PALETTE_COLOR 256//272  //(256 color + 16 blink color)
#define DEF_SFL_OSD_FSAD_2K             (0x00800000)     //Font start address
#define DEF_SFL_OSD_PSAD_2K             (0x00B00000)   //Bitmap start address

#if (LOGO_REPLACE == 1)
#define DEF_SFL_OSD_2ND_LOGO            (0x005C0000) //logo replace size 1920*1200
#define DEF_SFL_OSD_SERVICE_2ND_LOGO    (0x00380000) //service logo replace size 1920*1200
#endif

#define DVC821_OSDCT_IDLE       0x00
#define DVC821_OSDCT_FILL       0x01
#define DVC821_OSDCT_BITBLT     0x02
#define DVC821_OSDCT_CHAR       0x03
#define DVC821_OSDCT_16FILL     0x05
#define DVC821_OSDCT_HIGH_CHAR  0x06
#define DVC821_OSDCT_BIT_BLT    0x07


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


typedef struct
{
    INT16 iX;
    INT16 iY;
} START_POINT;

typedef struct
{
    INT16 iWidth;
    INT16 iHeight;
} RECT_SIZE;

typedef struct
{
    UINT8 ucT_Color1;
    UINT8 ucT_Color2;
    UINT8 ucT_Color3;
    UINT8 ucT_Color4;
} OSD_TRANSPARENCY_COLOR;

typedef struct
{
    INT8 cBias_R;
    INT8 cBias_G;
    INT8 cBias_B;
} OSD_BIAS;

typedef struct
{
    OSD_BIAS sBias_Color1;
    OSD_BIAS sBias_Color2;
    OSD_BIAS sBias_Color3;
    OSD_BIAS sBias_Color4;
} OSD_BIAS_COLOR;

typedef struct
{
    FLOAT fGain_R;
    FLOAT fGain_G;
    FLOAT fGain_B;
} OSD_GAIN;

typedef struct
{
    OSD_GAIN sGain_Color1;
    OSD_GAIN sGain_Color2;
    OSD_GAIN sGain_Color3;
    OSD_GAIN sGain_Color4;
} OSD_GAIN_COLOR;


typedef struct _OSD_BITMAP
{
    START_POINT sSrc_Position;  //use to record bitmap's start position in memory
    RECT_SIZE sBitmap_Size ;  //used to record bitmap size information
} OSD_BITMAP;


//Font set use one byte to indicate his character width
typedef struct _OSD_STRING
{
    UINT8 ucStringMaxHeight;
    UINT16 uiStringLength;
    UINT32 ulFontOffset;
    UINT16 *puiCharIndex;
    UINT8 *pucCharWdith;
} OSD_STRING;


#pragma pack(pop)                          // restore previous alignment


void OSD_CurrentCopy(void); //A70LV_Larry_0067
INT8 dvC821_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color);
INT8 dvC821_OSD_Reload(void);
INT8 dvC821_OSD_Memory_Protect(UINT8 ucEnable);
void dvC821_OSD_On( START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color);
void dvC821_OSD_Off(void);
void dvC821_OSD_Bias_Set(OSD_BIAS_COLOR sBiasSetting);
void dvC821_OSD_Gain_Set( INT8 cEnable, OSD_GAIN_COLOR sGainSetting);
void dvC821_Inhibit_Color_Set( INT8 cEnable, INT16 iInhibit_Color );
INT8 dvC821_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position);
//ZU860_Clare_0132, add, >>>
INT8 dvC821_Paint_Specified_Bitmap(OSD_BITMAP sBitmap, START_POINT sOffset_position, RECT_SIZE sSrc_Size, START_POINT sDes_position);
//ZU860_Clare_0132, add, <<<
INT8 dvC821_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index);
INT8 dvC821_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset);

void dvC821_SerialFlash_Erase_All(void); //A70LV_Larry_0004
void dvC821_SerialFlash_Erase_Sector(UINT32 ulSflAd, UINT32 ulDataSize); //A70LV_Larry_0004
void dvC821_SerialFlash_Write(UINT32 ulSflAd, UINT8 *pucBuffer, UINT32 ulDataSize);
UINT32 dvC821_SerialFlash_CheckSum(UINT32 ulSflAd, UINT32 ulDataSize);
eRESULT dvC821_RAM_Read(UINT32 ulRAMAd, UINT8 *pBuffer, UINT32 ulDataSize);
eRESULT dvC821_RAM_Write(UINT32 ulRAMAd, UINT8 *pBuffer, UINT32 ulDataSize);
void dvC821_OSD_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor);
INT8 dvC821_OSD_PanelSet(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 dvC821_OSD_PanelChange(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 dvC821_OSD_MenuTransparencySet(UINT8 ucValue);     //A70LV_Doulas_0122
INT8 dvC821_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe);     //A70LV_Doulas_0122
void dvC821_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt); //A70LV_Larry_0139
void OSD_DMA_SerialFlash_Set( UINT32 ulSflAd, UINT32 ulDestMemAd, UINT8 ucRegsel, UINT32 ulDmaCnt );
void OSD_DMA_DDR3_WriteTo_Flash( UINT32 ulSrcMemAd, UINT32 ulSflAd, UINT8 ucRegsel, UINT32 ulDmaCnt );
#if (LOGO_REPLACE == 1)
INT8 dvC821_2ND_LOGO_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color, UINT8 ucIndex); //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0068 //A65_OPTOMA_Julie_0067
INT8 dvC821_OSD_2ND_LOGO_Reload(UINT8 ucIndex); //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
#endif
BOOL dvC821_ShowOsdBitmapData(UINT32 ulStartVertPosition);  //A35G2_Simon_0114


#ifdef __cplusplus
}
#endif


#endif
