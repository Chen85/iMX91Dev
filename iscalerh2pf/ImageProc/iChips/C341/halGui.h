#ifndef _HALGUI_H_
#define _HALGUI_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "dvC341_OSD.h"
#include "Common.h"


INT8 halGui_Init(void);
INT8 halGui_HWInit(INT16 iPalette_ColorAmount, UINT8 *pucPaletteData, UINT8 ucInhibit_Color);
INT8 halGui_OSDReload(void);
#if (LOGO_REPLACE == 1)	//ZU860_Clare_0124
void halGui_LogoPaletteFlagInit(void);
void halGui_ServiceLogoPaletteFlagInit(void); //A65_OPTOMA_Julie_0076
INT8 halGui_SecondLogoHWInit(void);
INT8 halGui_Service_SecondLogoHWInit(void);   //A65_OPTOMA_Julie_0076
INT8 halGui_SecondLogoPaletteStore(char *fileName, UINT8 ucIndex);
#endif


INT8 halGui_OSD_Copy(void); //A70LV_Larry_0067
INT8 halGui_OSD_On(COORDINATE sStart_Position, BLOCK_SIZE_INFO sDraw_Size, INT8 cTrap_Enable, OSD_TRANSPARENCY_COLOR_INFO sTrap_Color);
INT8 halGui_OSD_Off(void);
INT8 halGui_OSD_Set_InhitbitColor( INT8 cEnable, INT16 iInhibit_Color );
INT8 halGui_Language_Set(UINT8 ucLanguage);
INT8 halGui_Paint_Bitmap(OSD_BITMAP_INFO sBitmap, START_POINT_INFO sDes_position);
INT8 halGui_Paint_Specified_Bitmap(OSD_BITMAP_INFO sBitmap, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Position, BLOCK_SIZE_INFO sDraw_Size);
INT8 halGui_Paint_Rectangle(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index);
INT8 halGui_Paint_Text( UINT32 ulTextID, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment );
INT8 halGui_Paint_ASCIIString( UINT8 *pucString, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment );
INT8 halGui_Paint_Number(INT32 iNumber, eNUMERIC_SIGN eNumMode, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment);
INT8 halGui_Paint_TextHightLight(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index);	//ZU860_Clare_0019,
INT8 halGui_Paint_SubString( OSD_STRING_INFO sOsdString, START_POINT_INFO sDes_position, INT16 iTextColor, INT16 iStringOffset);

MENU_INFO* halGui_MenuStructPoint_Get(void);
UINT32 halGui_MenuCount_Get(void);


INT8 halGui_GuiData_EraseAll(void);
INT8 halGui_GuiData_FlashEraseSector(UINT32 ulAddr, UINT32 ulSize);
INT8 halGui_GuiData_FlashWrite(UINT32 ulAddr, UINT8 *pucData, UINT32 ulSize);
//UINT32 halGui_GuiData_PartialWrite(char *fileName, UINT32 ulAddr);
UINT32 halGui_GuiData_PartialWrite(char *fileName, UINT32 ulFlashAddr, UINT32 ulRAMAddr, UINT8 ucFocus);
UINT32 halGui_Upgrade(UINT8 ucFocus);
INT8 halGui_UpdateTextGuiData(void);
INT8 halGui_UpdateBitmapGuiData(void);
INT8 halGui_InitSemaphore(void);     //A70LV_Doulas_0014

void halGui_Upgrade_Second_Logo_Replace(UINT8 ucFocus); //A65_OPTOMA_Julie_0102 //A35G2_Coda_0067
void halGui_Upgrade_Service_Second_Logo_Replace(UINT8 ucFocus); //A65_OPTOMA_Julie_0102 //A35G2_Coda_0067

INT8 halGui_Paint_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sStart_Position); //T100_Sander_0013
INT8 halGui_Paint_Specified_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Start_Position, BLOCK_SIZE_INFO sDraw_Size);	//ZU860_Clare_0132
INT8 halGui_PanelSet(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 halGui_PanelChange(ePANEL_ID ePanelTimingId);      //A70LV_Doulas_0105
INT8 halGui_OSD_MenuTransparencySet(UINT8 ucValue);         //A70LV_Doulas_0122
INT8 halGui_OSD_MenuTransparencyEnableSet(UINT8 ucEnable);         //A70LV_Doulas_0122
INT8 halGui_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor); //A70LV_Larry_0137
INT8 halGui_Paint_Line(COORDINATE sStart_Position, UINT8 width, INT16 iColor_Index);    //ZU860_Clare_0001, add
#ifdef LOGO_REPLACE  //SNPLU9000_Energy_0018
UINT32 halGui_GuiData_Resolved_Logo_PartialWrite(UINT32 ulExtAddr, UINT32 ulAddr, UINT32 ulSize);	//ZU860_Clare_0131
#endif
void halGui_UpgradeAccess_Set(BOOL bEnable);		//A65_OPTOMA_Doulas_0126
BOOL halGui_UpgradeAccess_Get(void);				//A65_OPTOMA_Doulas_0126

INT8 halGui_2ND_LOGO_OSDReload(UINT8 ucIndex);	//A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
void halGui_ShowOsdBitmapData(UINT32 ulStartVertPosition);    //A35G2_Simon_0114
INT8 halGui_OSD_Memory_Protect(UINT8 Enable);


#ifdef __cplusplus
}
#endif


#endif
