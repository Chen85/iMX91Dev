#ifndef _DVC341_GUI_H_
#define _DVC341_GUI_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"
#include "Ichipc341_MemTable.h"

#define BITMAP_RAW_WIDTH     1920
#define BITMAP_RAW_HEIGHT    2900

//for compiler error ////////////////////////////////////
#define OSD_ResetTimeInState C341_OSD_ResetTimeInState
#define OSD_TimeElapsedInState C341_OSD_TimeElapsedInState
#define OSD_DMA_SerialFlash_Set C341_OSD_DMA_SerialFlash_Set
#define OSD_DMA_DDR3_WriteTo_Flash C341_OSD_DMA_DDR3_WriteTo_Flash
#define OSD_Memory_Protect C341_OSD_Memory_Protect
#define OSD_AccessControl C341_OSD_AccessControl
#define OSD_Swap_Memplane C341_OSD_Swap_Memplane
#define OSD_Blink_Control C341_OSD_Blink_Control
#define OSD_Palette_Set C341_OSD_Palette_Set
#define OSD_Copy C341_OSD_Copy
#define OSD_CurrentCopy C341_OSD_CurrentCopy
/////////////////////////////////////////////////////////////

//#define BITMAP_RAW_WIDTH     1920
//#define BITMAP_RAW_HEIGHT    2900

#ifndef SIMULATOR_ISCALER
#define OSDCT_FASTEN_ON
#define OSDCT_FASTEN_TRANSFER_UNIT 64
#endif /* SIMULATOR_ISCALER */

//H2 wait review
#define DV_BITMAP_RAW_DATA_SIZE (0x00A00000)
#define DV_TEXT_RAW_DATA_SIZE   (0x002F0000)


#define DVC341_OSD_SIZE_EXPAND  //Note: not include OSD display area //T100_Sander_0012
#define DVC341_OSD_BITMAP_SKETCHPAD_WIDTH  (BITMAP_RAW_WIDTH / 128) //modify Bitblt read line feed to 0x0F(15x128= 1920) to fit bmp width, this value should read from Json define //T100_Sander_0019

#define ICHIP_OSD_PASS 1
#define ICHIP_OSD_INIT_FAIL -1
#define PALETTE_AMOUNT_ERROR -2


#define ICHIP_PALETTE_COLOR 256//272  //(256 color + 16 blink color)
#define DEF_SFL_OSD_FSAD_2K (0x00010000)// (0x00800000)// Font start address
#define DEF_SFL_OSD_PSAD_2K (0x00300000)// (0x00B00000)// Bitmap start address

#if 0//(LOGO_REPLACE == 1)
#define DEF_SFL_OSD_2ND_LOGO    		(0x01000000) // logo replace size = 1920*1200 = 0x00232800 => 0x00240000
#define DEF_SFL_OSD_SERVICE_2ND_LOGO    (0x01240000) // service logo replace size 1920*1200
#endif

#define DVC341_OSDCT_IDLE       0x00
#define DVC341_OSDCT_FILL       0x01
#define DVC341_OSDCT_BITBLT     0x02
#define DVC341_OSDCT_CHAR       0x03
#define DVC341_OSDCT_16FILL     0x05
#define DVC341_OSDCT_HIGH_CHAR  0x06
#define DVC341_OSDCT_BIT_BLT    0x07

#define DVC341_OSDCT_FASTEN     0x10   //only for OSDCT OSDCMD == 1/2/4/5/8/9

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */



typedef struct
{
    UINT8 ucMagnifyOSD2X;
}sOSD_INFO;

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
INT8 dvC341_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color);
INT8 dvC341_OSD_Reload(void);
INT8 dvC341_OSD_Memory_Protect(UINT8 ucEnable);
void dvC341_OSD_On( START_POINT sDes_position, RECT_SIZE sDes_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR sTrap_Color);
void dvC341_OSD_Off(void);
void dvC341_OSD_Bias_Set(OSD_BIAS_COLOR sBiasSetting);
void dvC341_OSD_Gain_Set( INT8 cEnable, OSD_GAIN_COLOR sGainSetting);
void dvC341_Inhibit_Color_Set( INT8 cEnable, INT16 iInhibit_Color );
INT8 dvC341_Paint_Bitmap(OSD_BITMAP sBitmap, START_POINT sDes_position);
//ZU860_Clare_0132, add, >>>
INT8 dvC341_Paint_Specified_Bitmap(OSD_BITMAP sBitmap, START_POINT sOffset_position, RECT_SIZE sSrc_Size, START_POINT sDes_position);
//ZU860_Clare_0132, add, <<<
INT8 dvC341_Paint_Rectangle(START_POINT sDes_position, RECT_SIZE sDes_Size, INT16 iColor_Index);
INT8 dvC341_Paint_String(START_POINT sDes_position, OSD_STRING sString, INT16 iFont_Color, INT16 iBG_Color, INT16 iStringOffset);

void dvC341_SerialFlash_Erase_All(void); //A70LV_Larry_0004
void dvC341_SerialFlash_Erase_Sector(UINT32 ulSflAd, UINT32 ulDataSize); //A70LV_Larry_0004
void dvC341_SerialFlash_Write(UINT32 ulSflAd, UINT8 *pucBuffer, UINT32 ulDataSize);
UINT32 dvC341_SerialFlash_CheckSum(UINT32 ulSflAd, UINT32 ulDataSize);
eRESULT dvC341_RAM_Read(UINT32 ulRAMAd, UINT8 *pBuffer, UINT32 ulDataSize);
eRESULT dvC341_RAM_Write(UINT32 ulRAMAd, UINT8 *pBuffer, UINT32 ulDataSize);
void dvC341_OSD_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor);
INT8 dvC341_OSD_PanelSet(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 dvC341_OSD_PanelChange(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 dvC341_OSD_MenuTransparencySet(UINT8 ucValue);     //A70LV_Doulas_0122
INT8 dvC341_OSD_MenuTransparencyEnableSet(UINT8 ucEnalbe);     //A70LV_Doulas_0122
void dvC341_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt); //A70LV_Larry_0139
void OSD_DMA_SerialFlash_Set( UINT32 ulSflAd, UINT32 ulDestMemAd, UINT8 ucRegsel, UINT32 ulDmaCnt );
void OSD_DMA_DDR3_WriteTo_Flash( UINT32 ulSrcMemAd, UINT32 ulSflAd, UINT8 ucRegsel, UINT32 ulDmaCnt );
#if (LOGO_REPLACE == 1)
INT8 dvC341_2ND_LOGO_OSD_Init(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color, UINT8 ucIndex); //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0068 //A65_OPTOMA_Julie_0067
INT8 dvC341_OSD_2ND_LOGO_Reload(UINT8 ucIndex); //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
#endif
BOOL dvC341_ShowOsdBitmapData(UINT32 ulStartVertPosition);  //A35G2_Simon_0114
BOOL dvC341_IsMagnifyOSD2X(void);

#ifdef SIMULATOR_ISCALER
INT8 dvC341_WarpingDeomOSD_Palette_Set(INT16 iAmount, UINT8 *pucData);
void dvC341_WarpingDeomOSDInit(UINT32 uiTextSize, UINT8 *pcText, UINT32 uiCharWidthSize, UINT8 *pcCharWidth);
void dvC341_WarpingDeomOSDString(uint8* pStr, uint8 nCharNum, uint16 nHStart, uint16 nVStart, INT16 eCharColor, INT16 eBkgColor);
void dvC341_WarpingDeomOSDRectangle(uint16 nHStart, uint16 nVStart, uint16 nWidth, uint16 nHeight, uint16 eColor);
void dvC341_WarpingDeomOSD_Copy2Temp(uint16 nSrcHStart, uint16 nSrcVStart, uint16 nDstHStart, uint16 nDstVStart, uint16 nWidth, uint16 nHeight, BOOL bToTemp);
void dvC341_WarpingDeomOSDOn(void);
#endif /* SIMULATOR_ISCALER */

#ifdef __cplusplus
}
#endif


#endif
