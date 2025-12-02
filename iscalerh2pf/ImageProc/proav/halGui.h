#ifndef _HALGUI_H_
#define _HALGUI_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "Common.h"
//#include "./OSD/Custom_OSD.h"

//define for OSD object read status
#define NO_SPECIFIC_READING 0
#define MENU_READING        1
#define ITEM_READING        2
#define BITMAP_READING      3
#define TEXT_READING        4
#define TEXT_STRING_READING 5


//API Return Status
#define GUI_PASS  1
#define GUI_FOUND 1
#define GUI_FAIL  0
#define ERROR_GUI_NOT_FOUND -1
#define ERROR_GUI_ITEM_SIZE_OVER_LIMIT -2
#define ERROR_GUI_EMPTY_STRING -3
#define ERROR_GUI_NOT_ALLOW_NEGATIVE -4
#define ERROR_GUI_SEMAPHORE_FAIL -5    //A70LV_Doulas_0014


#define GUI_ON 1
#define GUI_OFF 0

#define CJSON_GET_PASS  1
#define ERROR_CJSON_GET_FAIL  -1
#define ERROR_CJSON_PARSER_FAIL  -2
#define ERROR_CJSON_ITEM_IS_EMPTY  -3
#define ERROR_CJSON_NO_DEFINE_NAMESTRING  -4
#define ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT  -5
#define ERROR_CJSON_PALETTE_SIZE_NOT_MATCH  -6
#define ERROR_CJSON_FILE_OPEN_FAIL  -7
#define ERROR_OSD_STRUCT_LINK_FAIL  -8


//define for test useage color
#define GUI_C_RED 246
#define GUI_C_GREEN 247
#define GUI_C_BLUE 248
#define GUI_C_CYAN 253
#define GUI_C_YELLOW 249
#define GUI_C_MAGENTA 254
#define GUI_C_WHITE 250
#define GUI_C_BLACK 252 //A70LV_Larry_0046
#define GUI_C_GRAY 239  //T100_Simon_0028

//#define GUI_TRANSPARENT_COLOR_1 253
//#define GUI_TRANSPARENT_COLOR_2 254

#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA) || defined(CUSTOM_KRINDA)	//ZU860_Clare_0001
#define GUI_C_LIGHT_BLACK 240
#define GUI_C_ACT_MENU_COLOR 241
#define GUI_C_ACT_ITEM_COLOR 242
#define GUI_C_INA_MENU_COLOR 243
#define GUI_C_INA_ITEM_COLOR 244
#define GUI_C_FOCUS_BAR 245
#define GUI_C_SLIDER_BAR 250                //Slider Bar [#aaaaaa; 170,170,170]
#define GUI_C_SLIDER_BAR_BACKGROUND 252     //Slider Bar Background [#000000; 0,0,0]
#define GUI_C_RED_LINE 246
#define GUI_C_MENU_GUIDE 252 //241                //Menu Guide [#332f2f; 51,47,47]
#define GUI_C_MAIN_BACKGROUND 242           //Main Menu Background   [#6f6f6f; 111,111,111]
#define GUI_C_ITEM_ON_FOCUS 238             //Item On Focus [#007bc7; 0,123,199]
#define GUI_C_MSG_BACKGROUND 240            //Message Background     [#aaaaaa; 170,170,170]
#define GUI_C_CONFIRM_BACKGROUND 242            //Message Background     [#aaaaaa; 170,170,170]
#define GUI_C_SUB_MENU_ITEM 242             //Sub Menu Item          [#393635; 57,54,53]
#define GUI_C_SUB_MENU_BACKGROUND 242       //Sub Menu Background    [#3e3a39; 62,58,57]
#else
#define GUI_C_LOGO_COLOR 240       //logo color
#define GUI_C_ACT_MENU_COLOR 241   //submenu item highlight
#define GUI_C_ACT_ITEM_COLOR 242   //main item highlight and submenu item background
#define GUI_C_INA_MENU_COLOR 243   //submenu title
#define GUI_C_INA_ITEM_COLOR 244   //main item background and submenu item grayout color
#define GUI_C_FOCUS_BAR 245        //
#define GUI_C_SLIDER_BAR 251//250                // white
#define GUI_C_SLIDER_BAR_BACKGROUND 250//239     // [137, 137, 137]
#define GUI_C_RED_LINE 246
#endif /* CUSTOM_OPTOMA */

#define GUI_INHITBIT_COLOR 255
#define GUI_STRING_ASCII_FONT_OFFSET 0

/////////////////////////////////////////////////////////////////////
///ENUM Object Align Type
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
///Item Infomation
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
///Menu Information
/////////////////////////////////////////////////////////////////////


INT8 halGui_Init(void);
INT8 halGui_HWInit(void);
INT8 halGui_LogoReplacementInit(void);
INT8 halGui_OSDReload(void);
#ifdef LOGO_REPLACE	//ZU860_Clare_0124
INT8 halGui_SecondLogoHWInit(void);
#endif
INT8 halGui_OSD_Copy(void); //A70LV_Larry_0067
INT8 halGui_OSD_On(COORDINATE sStart_Position, BLOCK_SIZE_INFO sDraw_Size);
INT8 halGui_OSD_Off(void);
INT8 halGui_OSD_Set_InhitbitColor( INT8 cEnable, INT16 iInhibit_Color );
INT8 halGui_Language_Set(UINT8 ucLanguage);
INT8 halGui_Paint_Bitmap(UINT32 ulBitmapID, COORDINATE sStart_Position);
INT8 halGui_Paint_Rectangle(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index);
INT8 halGui_Paint_RectanglewithBoarder(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index, INT16 iColor_Background, INT16 iBoarder_Index); ////T100_Sander_0002
INT8 halGui_Paint_RectangFrame(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index, INT16 iColor_Background);
INT8 halGui_Paint_Text( UINT32 ulTextID, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment );
INT8 halGui_Paint_ASCIIString( UINT8 *pucString, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment );
INT8 halGui_Paint_Number(INT32 iNumber, eNUMERIC_SIGN eNumMode, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment);
INT8 halGui_Paint_TextHightLight(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index);	//ZU860_Clare_0019,
INT8 halGui_Paint_TestPattern(COORDINATE sDrawStartPosition, BLOCK_SIZE_INFO sDraw_Size, UINT16 uiIndex);

MENU_INFO* halGui_MenuStructPoint_Get(void);
UINT32 halGui_MenuCount_Get(void);


INT8 halGui_GuiData_EraseAll(void);
INT8 halGui_GuiData_FlashEraseSector(UINT32 ulAddr, UINT32 ulSize);
INT8 halGui_GuiData_FlashWrite(UINT32 ulDRAM_Addr, UINT32 ulFlash_Addr, UINT32 ulSize, UINT8* pcData);
INT8 halGui_GuiData_FlashRead(UINT32 ulDRAM_Addr, UINT32 ulSflAd, UINT32 ulSize, UINT8* pcData);
UINT32 halGui_GuiData_PartialWrite(char *fileName, UINT32 ulFlashAddr, UINT32 ulRAMAddr, UINT8 ucFocus);
INT8 halGui_UpdateTextGuiData(void);
INT8 halGui_UpdateBitmapGuiData(void);
INT8 halGui_InitSemaphore(void);     //A70LV_Doulas_0014

INT8 halGui_Paint_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sStart_Position); //T100_Sander_0013
INT8 halGui_Paint_Specified_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Start_Position, BLOCK_SIZE_INFO sDraw_Size);	//ZU860_Clare_0132
INT8 halGui_PanelSet(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 halGui_PanelChange(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 halGui_OSD_MenuTransparencySet(UINT8 ucValue);         //A70LV_Doulas_0122
INT8 halGui_OSD_MenuTransparencyEnableSet(UINT8 ucEnable);         //A70LV_Doulas_0122
INT8 halGui_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor); //A70LV_Larry_0137
INT8 halGui_Paint_Line(COORDINATE sStart_Position, UINT8 width, INT16 iColor_Index);    //ZU860_Clare_0001, add
UINT8 halGui_LogoReplacementEnable(UINT16 uiPanel);
UINT16 halGui_RGBColorToIndex(uint32 uRGBColor);  //A70LK_CL_0002
INT8 halGui_RestorePalette(void);  //A70LK_CLK_0007
INT8 halGui_Buffer_Init(void);
UINT16 halGui_MaxStringW_Get(UINT32 ulTextID); //A70LK_Jacky_0053

////////////////////////////
//only for compiler error
INT8 halGui_Paint_Specified_Bitmap(OSD_BITMAP_INFO sBitmap, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Position, BLOCK_SIZE_INFO sDraw_Size);  //H2 wait review
INT8 halGui_Paint_SubString( OSD_STRING_INFO sOsdString, START_POINT_INFO sDes_position, INT16 iTextColor, INT16 iStringOffset);
void halGui_Upgrade(UINT8 ucFocus);
void halGui_UpgradeAccess_Set(BOOL bEnable);
BOOL halGui_UpgradeAccess_Get(void);
void halGui_ShowOsdBitmapData(UINT32 ulStartVertPosition);
////////////////////////////



#ifdef __cplusplus
}
#endif


#endif
