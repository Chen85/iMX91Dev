#include <string.h>

#include "palGui.h"
#include "halGui.h"
#ifdef SCALER_C821_C789
#include "halC789CtrlAPI.h"
#endif
#include "cJSON.h"
#include "utilDbgMsg.h"
//#include "lwip_fs.h"
//#include "ff.h"
//#include "MemMap.h"
//#include "halScaler.h"      //A70LV_Doulas_0014
#include "utilCRCAPI.h"

#ifdef SCALER_C341
#include "dvC341.h"
#endif

//#define STANDARD_C_MALLOC
#define CHAR_LINE_BUF_MAX 32


///////////////// callback function start ///////////////
sGUI_CALLBACK sGuiCb ;
sGUI_CALLBACK GuiCb;

void Gui_RegCallback(sGUI_CALLBACK fpCallback)
{
    sGuiCb = fpCallback;
}

sGUI_CALLBACK Gui_fpCallbackGet(void)
{
    return sGuiCb;
}

////////////////// callback function end ////////////////



/* Read a cJSON item/entity/structure to embedded. */
char palGui_JSON_Get(const cJSON *item, int iReadStatus);

//#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)
//pthread_mutex_t             xGuiSemaphore;
//#else
//SemaphoreHandle_t            xGuiSemaphore;
//#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
//    StaticSemaphore_t            xGuiMutexBuffer;
//#endif
//#endif

//char *m_pcBitmapStruct = (char*)BITMAP_JSON_ADDR;
//char *m_pcTextStruct = (char*)TEXT_JSON_ADDR;
//char *m_pcOSDStruct = (char*)OSD_STRUCT_ADDR; //A70LV_Larry_0001


//	static int iBitmapTotalWidth = 0;
//	static int iBitmapTotalHeight = 0;
//static int iBitmapPaletteUsed = 768;
UINT8 aiBitmapPalette[768];


static INT16 iTotalLanguageUsed = 11;
static INT16 iActiveLanguageIndex = 0;
static INT16 *piLanguageUsed;

static int iTextStringCount = 0;
static int iTextStringReadCount = 0;


UINT32 ulAllFontCount = 0;  //use to record how many font be load into system
UINT8 *mpucAllFontWidthInfo;  //use to record all load font's width information
UINT32 ulAsciiFontCount = 0;  //use to record how many font be load into system
UINT8 *mucAsciiFontMaxHeight; //T100_Sander_0021

UINT32 ulAllTextCount = 0;
static int iTextReadCount = 0;
TEXT_INFO *mspAllTextInfo;

UINT32 ulAllBitmapCount = 0;
static int iBitmapReadCount = 0;
BITMAP_INFO *mspAllBitmapInfo;

UINT32 ulAllMenuCount = 0;
static int iMenuReadCount = 0;
MENU_INFO *mspAllMenuInfo;

UINT32 ulAllItemCount = 0;
static int iItemReadCount = 0;
ITEM_INFO *mspAllItemInfo;

#define SPLASH_SUPPORT_NUM 10
static UINT32 SPLASH_LOGO_ID[SPLASH_SUPPORT_NUM] = {0}; //T100_Sander_0013

static inline BOOL palGui_SemaphoreTake(const char *pcFunc)
{
    #if 0    //H2 wait review
    BOOL bResult = halGui_SemaphoreTake(TRUE, pcFunc);

    return bResult;
    #endif

    return TRUE;
}


static inline BOOL palGui_SemaphoreGive(const char *pcFunc)
{
    #if 0     //H2 wait review
    BOOL bResult = halGui_SemaphoreTake(FALSE, pcFunc);

    return bResult;
    #endif

    return TRUE;
}


char palGui_JSON_Get_String(const cJSON *item)
{
    if (!strcmp("Brand", item->string))
    {
        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Brand = %s\n", __FUNCTION__, __LINE__, item->valuestring);
    }
    else if (!strcmp("Model", item->string))
    {
        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Model = %s\n", __FUNCTION__, __LINE__, item->valuestring);
    }
    else
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : \"%s\" with No define Name_String\n", __FUNCTION__, __LINE__, item->valuestring);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Number(const cJSON *item)
{
    if (!strcmp("APVersion", item->string))
    {
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)OSD: APVersion = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else if (!strcmp("BitmapTotalWidth", item->string))
    {
        //iBitmapTotalWidth = (int)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Bitmap: BitmapTotalWidth = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else if (!strcmp("BitmapTotalHeight", item->string))
    {
        //iBitmapTotalHeight = (int)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Bitmap: BitmapTotalHeight = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else
    {
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String\n", __FUNCTION__, __LINE__, item->valueint);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Menu_Number(const cJSON *item)
{
    if (iMenuReadCount > ulAllMenuCount)
    {
        return (char)ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT;
    }


    if (!strcmp("MenuID", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].ulMenuID = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d ID=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuItemCount", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].uiItemCount = (UINT16)item->valueint;
        #ifdef STANDARD_C_MALLOC
        mspAllMenuInfo[iMenuReadCount-1].ppsMenuItemList = (ITEM_INFO **)malloc(item->valueint*sizeof(ITEM_INFO *));
        #else
        mspAllMenuInfo[iMenuReadCount-1].ppsMenuItemList = (ITEM_INFO **)pvPortMalloc(item->valueint*sizeof(ITEM_INFO *));
        #endif

        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuItemCount=%d, ITEM_INFO*_UnitSize=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint, sizeof(ITEM_INFO *));
    }
    else if (!strcmp("MenuOnePageMaxItem", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].uiOnePageMaxItem = (UINT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuOnePageMaxItem=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuDisplayStyle", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.uiDisplayStyle = (UINT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuDisplayStyle=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("XofLTR", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.sStartPosition.sLTR.iX = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d XofLTR=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("YofLTR", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.sStartPosition.sLTR.iY = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d YofLTR=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("XofRTL", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.sStartPosition.sRTL.iX = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d XofRTL=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("YofRTL", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.sStartPosition.sRTL.iY = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d YofRTL=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuWidth", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.sSizeInfo.iWidth = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuWidth=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuHeight", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.sSizeInfo.iHeight = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuHeight=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuChildItemGap", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.iItemGap = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuChildItemGap=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuStringOnFocusColor", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.ucString_OnFocus_Color = (UINT8)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuStringOnFocusColor=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuStringNonFocusColor", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.ucString_NonFocus_Color = (UINT8)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuStringNonFocusColor=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuItemOnFocusColor", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.ucItem_OnFocus_Color = (UINT8)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuItemOnFocusColor=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else if (!strcmp("MenuItemNonFocusColor", item->string))
    {
        mspAllMenuInfo[iMenuReadCount-1].sMenuDrawingInfo.ucItem_NonFocus_Color = (UINT8)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Menu_%d MenuItemNonFocusColor=%d\n", __FUNCTION__, __LINE__, iMenuReadCount, item->valueint);
    }
    else
    {
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String\n", __FUNCTION__, __LINE__, item->valueint);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Item_Number(const cJSON *item)
{
#if 0
    if (iItemReadCount > ulAllItemCount)
    {
        return (char)ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT;
    }


    if (!strcmp("ItemID", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].ulItemID = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemID=%d\r\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemDataCode", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].iDataCode = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemDataCode=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ParentMenuID", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].ulParentMenuID = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ParentMenuID=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemOperateType", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].iOperateType = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemOperateType=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemListBitmapNumber1", item->string)) //T100_Sander_0012
    {
        mspAllItemInfo[iItemReadCount-1].ItemListBitmapNumber1 = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemListBitmapNumber1=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemListBitmapNumber2", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].ItemListBitmapNumber2 = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemListBitmapNumber2=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemListTextNumber1", item->string)) //T100_Sander_0012
    {
        mspAllItemInfo[iItemReadCount-1].ItemListTextNumber1 = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemListTextNumber1=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemListTextNumber2", item->string)) //T100_Sander_0012
    {
        mspAllItemInfo[iItemReadCount-1].ItemListTextNumber2 = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemListTextNumber2=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemHide", item->string)) //T100_Sander_0016
    {
        mspAllItemInfo[iItemReadCount-1].ItemHide = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemHide=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("NextMenuID", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].ulNextMenuID = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d NextMenuID=%d\r\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemDisplayStyle", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.uiDisplayStyle = (UINT16)item->valueint;
		//LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemDisplayStyle=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
        //if(item->valueint == eidCTM_LOGO) //T100_Sander_0013
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
        if(item->valueint == eidCTM_LOGO || item->valueint == eidCTM_OPTOMA_SECOND_LOGO) //T100_Sander_0013
#else
        if(item->valueint == eidCTM_LOGO) //T100_Sander_0013
#endif
        {
        	INT8 Index = 0;
        	while(Index < SPLASH_SUPPORT_NUM)
        	{
        		if((SPLASH_LOGO_ID[Index]) == 0)
        		{
					SPLASH_LOGO_ID[Index] = mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulBitmapID[0];
					if((Index + 1) < SPLASH_SUPPORT_NUM)
						SPLASH_LOGO_ID[Index + 1] = 0;
					break;
        		}
        		Index++;
        	}
		}
    }
    else if (!strcmp("XofLTR", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.sStartPosition.sLTR.iX = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d XofLTR=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("YofLTR", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.sStartPosition.sLTR.iY = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d YofLTR=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("XofRTL", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.sStartPosition.sRTL.iX = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d XofRTL=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("YofRTL", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.sStartPosition.sRTL.iY = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d YofRTL=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemWidth", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.sSizeInfo.iWidth = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemWidth=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if (!strcmp("ItemHeight", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.sSizeInfo.iHeight = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemHeight=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else if(!strcmp("ItemControl", item->string))
    {
        mspAllItemInfo[iItemReadCount-1].ItemControl = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d ItemHeight=%d\n", __FUNCTION__, __LINE__, iItemReadCount, item->valueint);
    }
    else
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String\n", __FUNCTION__, __LINE__, item->valueint);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }
#endif
    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Bitmap_Number(const cJSON *item)
{
    if (!strcmp("BitmapID", item->string))
    {
        mspAllBitmapInfo[iBitmapReadCount-1].ulBitmapID = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): BitmapID = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else if (!strcmp("XofSource", item->string))
    {
        mspAllBitmapInfo[iBitmapReadCount-1].sSrc_Coordinate.iX = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): XofSource = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else if (!strcmp("YofSource", item->string))
    {
        mspAllBitmapInfo[iBitmapReadCount-1].sSrc_Coordinate.iY = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): YofSource = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else if (!strcmp("BitmapWidth", item->string))
    {
        mspAllBitmapInfo[iBitmapReadCount-1].sBitmap_Size.iWidth = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): BitmapWidth = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else if (!strcmp("BitmapHeight", item->string))
    {
        mspAllBitmapInfo[iBitmapReadCount-1].sBitmap_Size.iHeight = (INT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): BitmapHeight = %d\n", __FUNCTION__, __LINE__, item->valueint);
    }
    else
    {
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String\n", __FUNCTION__, __LINE__, item->valueint);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Text_Number(const cJSON *item)
{
    if (!strcmp("TextID", item->string))
    {
        mspAllTextInfo[iTextReadCount-1].ulTextID = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): TextID = %d\n", __FUNCTION__, __LINE__, (mspAllTextInfo+(iTextReadCount-1))->ulTextID);
    }
    else if (!strcmp("TextMaxWidth", item->string))
    {
        mspAllTextInfo[iTextReadCount-1].uiTextMaxWidth = (UINT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): TextMaxWidth = %d\n", __FUNCTION__, __LINE__, (mspAllTextInfo+(iTextReadCount-1))->uiTextMaxWidth);
    }
    else if (!strcmp("TextMaxHeight", item->string))
    {
        mspAllTextInfo[iTextReadCount-1].ucTextMaxHeight = (UINT8)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): TextMaxHeight = %d\n", __FUNCTION__, __LINE__, (mspAllTextInfo+(iTextReadCount-1))->ucTextMaxHeight);
    }
    else
    {
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String=%s\n", __FUNCTION__, __LINE__, item->valueint, item->string);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_TextString_Number(const cJSON *item)
{
    if (!strcmp("StringMaxHeight", item->string))
    {
        mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].ucStringMaxHeight = (UINT8)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): MaxHeight = %d\n", __FUNCTION__, __LINE__, ((mspAllTextInfo+(iTextReadCount-1))->psString + (iTextStringReadCount-1))->ucStringMaxHeight);
    }
    else if (!strcmp("StringWidth", item->string))
    {
        mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].uiStringWidth = (UINT16)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Width = %d\n", __FUNCTION__, __LINE__, ((mspAllTextInfo+(iTextReadCount-1))->psString + (iTextStringReadCount-1))->uiStringWidth);
    }
    else if (!strcmp("FontOffset", item->string))
    {
        mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].ulFontOffset = (UINT32)item->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): FontOffset = %d\n", __FUNCTION__, __LINE__, ((mspAllTextInfo+(iTextReadCount-1))->psString + (iTextStringReadCount-1))->ulFontOffset);
    }
    else
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String\n", __FUNCTION__, __LINE__, item->valueint);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_ObjectArray(const cJSON *item, int iReadStatus)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    /* Explicitly handle empty object case */
    if (!numentries)
    {
        //Empty child
        return (char)ERROR_CJSON_ITEM_IS_EMPTY;
    }

    switch (iReadStatus)
    {
        case MENU_READING:
            //malloc memory size for all Menu object needed
            ulAllMenuCount = numentries;
            #ifdef STANDARD_C_MALLOC
            mspAllMenuInfo = (MENU_INFO*)malloc(ulAllMenuCount * sizeof(MENU_INFO));
            #else
            mspAllMenuInfo = (MENU_INFO*)pvPortMalloc(ulAllMenuCount * sizeof(MENU_INFO));
            #endif

            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)OSD: Malloc Menu size=%d MallocSize=%d, UnitSize=%d\n", __FUNCTION__, __LINE__, ulAllMenuCount, ulAllItemCount * sizeof(MENU_INFO), sizeof(MENU_INFO));
            break;

        case ITEM_READING:
            //malloc memory size for all Item object needed
            ulAllItemCount = numentries;
            #ifdef STANDARD_C_MALLOC
            mspAllItemInfo = (ITEM_INFO*)malloc(ulAllItemCount * sizeof(ITEM_INFO));
            #else
            mspAllItemInfo = (ITEM_INFO*)pvPortMalloc(ulAllItemCount * sizeof(ITEM_INFO));
            #endif

            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)OSD: Malloc Item size=%d MallocSize=%d, UnitSize=%d\n", __FUNCTION__, __LINE__, ulAllItemCount, ulAllItemCount * sizeof(ITEM_INFO), sizeof(ITEM_INFO));
            break;

        case BITMAP_READING:
            //malloc memory size for all Bitmap object needed
            ulAllBitmapCount = numentries;
            #ifdef STANDARD_C_MALLOC
            mspAllBitmapInfo = (BITMAP_INFO*)malloc(ulAllBitmapCount * sizeof(BITMAP_INFO));
            #else
            mspAllBitmapInfo = (BITMAP_INFO*)pvPortMalloc(ulAllBitmapCount * sizeof(BITMAP_INFO));
            #endif

            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Bitmap: Malloc Bitmap size=%d MallocSize=%d, UnitSize=%d\n", __FUNCTION__, __LINE__, ulAllBitmapCount, ulAllBitmapCount * sizeof(BITMAP_INFO), sizeof(BITMAP_INFO));
            break;

        case TEXT_READING:
            //malloc memory size for all Text object needed
            ulAllTextCount = numentries;
            #ifdef STANDARD_C_MALLOC
            mspAllTextInfo = (TEXT_INFO*)malloc(ulAllTextCount * sizeof(TEXT_INFO));
            #else
            mspAllTextInfo = (TEXT_INFO*)pvPortMalloc(ulAllTextCount * sizeof(TEXT_INFO));
            #endif

            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Text: Malloc Text size=%d MallocSize=%d, UnitSize=%d\n", __FUNCTION__, __LINE__, ulAllTextCount, ulAllTextCount * sizeof(TEXT_INFO), sizeof(TEXT_INFO));
            break;

        case TEXT_STRING_READING:
            //malloc memory size for all String object of this TextID needed
            iTextStringCount = numentries;
            #ifdef STANDARD_C_MALLOC
            mspAllTextInfo[iTextReadCount-1].psString = (STRING_INFO *)malloc(iTextStringCount*sizeof(STRING_INFO));
            #else
            mspAllTextInfo[iTextReadCount-1].psString = (STRING_INFO *)pvPortMalloc(iTextStringCount*sizeof(STRING_INFO));
            #endif
            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Text: Malloc %d-th Text string size=%d MallocSize=%d\n", __FUNCTION__, __LINE__, iTextReadCount, iTextStringCount, iTextStringCount*sizeof(STRING_INFO));
            break;

        default :
            LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : with No define read status\n", __FUNCTION__, __LINE__);
            return (char)ERROR_CJSON_GET_FAIL;
    }

    //travel all cJSON child item and call palGui_JSON_Get_Object to extract all OSD Menu/Item properties
    //printf("JSON_Get_ObjectArray try to get first object of Menu total entries=%d\n", numentries);
    child = item->child;
    cRet = CJSON_GET_PASS;
    while (child && cRet == CJSON_GET_PASS)
    {
        switch (iReadStatus)
        {
            case MENU_READING:
                //Increase the Menu read count at this read action
                iMenuReadCount++;
                break;

            case ITEM_READING:
                //Increase the Item read count at this read action
                iItemReadCount++;
                break;

            case BITMAP_READING:
                //Increase the Item read count at this read action
                iBitmapReadCount++;
                break;

            case TEXT_READING:
                //Increase the Item read count at this read action
                iTextReadCount++;
                break;

            case TEXT_STRING_READING:
                //Increase the Item read count at this read action
                iTextStringReadCount++;
                break;
        }

        cRet = palGui_JSON_Get(child, iReadStatus);
        child = child->next;
    }

    //printf("JSON_Get_ObjectArray ret=%d\n", cRet);
    return cRet;
}


char palGui_JSON_Get_ItemBitmapArray(const cJSON *item)
{
    //travel all cJSON child item and copy this value to corresponding OSD Item object
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    if (iItemReadCount > ulAllItemCount)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : read count over array size limitation\n", __FUNCTION__, __LINE__);
        return (char)ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT;
    }

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    if (numentries <= 0)
    {
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d with empty Bitmap\n", __FUNCTION__, __LINE__, iItemReadCount);
        return CJSON_GET_PASS;
    }

    //malloc memory for this item Bitmap needed
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.ucBitmapCount = (UINT8)numentries;
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulBitmapID = (UINT32*)malloc(numentries*sizeof(UINT32));
    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d size=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries);

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulBitmapID[numentries] = (UINT32)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d BitmapID[%d]=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries, mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulBitmapID[numentries]);

        numentries++;
        child = child->next;
    }

    return CJSON_GET_PASS;
}

char palGui_JSON_Get_ItemBrushArray(const cJSON *item) //T100_Sander_0002
{
    //travel all cJSON child item and copy this value to corresponding OSD Item object
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    if (iItemReadCount > ulAllItemCount)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : read count over array size limitation\n", __FUNCTION__, __LINE__);
        return (char)ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT;
    }

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    if (numentries <= 0)
    {
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d with empty Bitmap\n", __FUNCTION__, __LINE__, iItemReadCount);
        return CJSON_GET_PASS;
    }

    //malloc memory for this item Bitmap needed
    #ifdef STANDARD_C_MALLOC
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulSliderColor = (UINT16*)malloc(numentries*sizeof(UINT16));
    #else
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulSliderColor = (UINT16*)pvPortMalloc(numentries*sizeof(UINT16));
    #endif
    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d size=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries);

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulSliderColor[numentries] = (UINT16)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d BitmapID[%d]=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries, mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulSliderColor[numentries]);

        numentries++;
        child = child->next;
    }

    return CJSON_GET_PASS;
}


static char palGui_JSON_Get_ItemRectangleIDArray(const cJSON *item)
{
    //travel all cJSON child item and copy this value to corresponding OSD Item object
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    if (iItemReadCount > ulAllItemCount)
    {
        LOG_MSG(db_ALWAYS, "(func:%s,#%d)ERROR : read count over array size limitation\n", __FUNCTION__, __LINE__);
        return (char)ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT;
    }

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    if (numentries <= 0)
    {
        LOG_MSG(db_HAL_GUI, "(func:%s,#%d): Item_%d with empty Bitmap\n", __FUNCTION__, __LINE__, iItemReadCount);
        return CJSON_GET_PASS;
    }

    //malloc memory for this item Bitmap needed
    #ifdef STANDARD_C_MALLOC
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulRectangleID = (UINT32*)malloc(numentries*sizeof(UINT32));
    #else
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulRectangleID = (UINT32*)pvPortMalloc(numentries*sizeof(UINT32));
    #endif
    //LOG_MSG(db_HAL_GUI, "(func:%s,#%d): Item_%d size=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries);

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulRectangleID[numentries] = (UINT32)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s,#%d): Item_%d RectangleID[%d]=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries,  mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulRectangleID[numentries]);

        numentries++;
        child = child->next;
    }

    return CJSON_GET_PASS;
}

char palGui_JSON_Get_ItemTextArray(const cJSON *item)
{
    //travel all cJSON child item and copy this value to corresponding OSD Item object
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    if (iItemReadCount > ulAllItemCount)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : read count over array size limitation\n", __FUNCTION__, __LINE__);
        return (char)ERROR_CJSON_OSD_READ_SIZE_OVER_LIMIT;
    }

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    if (numentries <= 0)
    {
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d with empty Text\n", __FUNCTION__, __LINE__, iItemReadCount);
        return CJSON_GET_PASS;
    }

    //malloc memory for this item StringID needed
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.ucTextCount = (UINT8)numentries;
    #ifdef STANDARD_C_MALLOC
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulTextID = (UINT32*)malloc(numentries*sizeof(UINT32));
    #else
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulTextID = (UINT32*)pvPortMalloc(numentries*sizeof(UINT32));
    #endif
    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d read-in TextID MallocSize=%d, pulTextIDAddr=0x%x\n", __FUNCTION__, __LINE__, iItemReadCount, numentries, mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulTextID);

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulTextID[numentries] = (UINT32)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Item_%d TextID[%d]=%d\n", __FUNCTION__, __LINE__, iItemReadCount, numentries, mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulTextID[numentries] );

        numentries++;
        child = child->next;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Palette_Array(const cJSON *item)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    if (numentries != 768)  //256 color x RGB = 768
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : palette size wrong\n", __FUNCTION__, __LINE__);
        return (char)ERROR_CJSON_PALETTE_SIZE_NOT_MATCH;
    }

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        aiBitmapPalette[numentries] = (UINT8)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Bitmap: Palette(%d)=%d\n", __FUNCTION__, __LINE__, numentries, child->valueint);

        numentries++;
        child = child->next;
    }

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_LanguageUsed_Array(const cJSON *item)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    //malloc memory for Language used array needed
    iTotalLanguageUsed = numentries;
    #ifdef STANDARD_C_MALLOC
    piLanguageUsed = (INT16 *)malloc(numentries*sizeof(int));
    #else
    piLanguageUsed = (INT16 *)pvPortMalloc(numentries*sizeof(int));
    #endif
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Language used=%d MallocSize=%d\n", __FUNCTION__, __LINE__, numentries, numentries*sizeof(int));

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        piLanguageUsed[numentries] = (int)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d-th Language_ID=%d\n", __FUNCTION__, __LINE__, (numentries+1), child->valueint);

        numentries++;
        child = child->next;
    }

    return CJSON_GET_PASS;
}

//T100_Sander_0021
char palGui_JSON_Get_ASCIIFontMaxHeight_Array(const cJSON *item)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    //malloc memory for all font needed
    ulAsciiFontCount = numentries;
    #ifdef STANDARD_C_MALLOC
    mucAsciiFontMaxHeight = (UINT8*)malloc(ulAsciiFontCount);
    #else
    mucAsciiFontMaxHeight = (UINT8*)pvPortMalloc(ulAsciiFontCount);
    #endif
    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Total AsciiFontHeight =%d\n", __FUNCTION__, __LINE__, numentries);

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mucAsciiFontMaxHeight[numentries] = (UINT8)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d-th font Height=%d\n", __FUNCTION__, __LINE__, (numentries+1), child->valueint);

        numentries++;
        child = child->next;
    }

    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d fonts read complete\n", __FUNCTION__, __LINE__, numentries);

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_AllFontWidthInfo_Array(const cJSON *item)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    //malloc memory for all font needed
    ulAllFontCount = numentries;
    #ifdef STANDARD_C_MALLOC
    mpucAllFontWidthInfo = (UINT8*)malloc(ulAllFontCount);
    #else
    mpucAllFontWidthInfo = (UINT8*)pvPortMalloc(ulAllFontCount);
    #endif
    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): Total font size=%d\n", __FUNCTION__, __LINE__, numentries);

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mpucAllFontWidthInfo[numentries] = (UINT8)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d-th font width=%d\n", __FUNCTION__, __LINE__, (numentries+1), child->valueint);

        numentries++;
        child = child->next;
    }

    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d fonts read complete\n", __FUNCTION__, __LINE__, numentries);

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_StringFont_Array(const cJSON *item)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* How many entries in the array? */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    //malloc memory for string font needed
    mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].uiStringLength = (UINT16)numentries;
    #ifdef STANDARD_C_MALLOC
    mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].puiCharIndex = (UINT16 *)malloc(numentries*sizeof(UINT16));
    #else
    mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].puiCharIndex = (UINT16 *)pvPortMalloc(numentries*sizeof(UINT16));
    #endif
//    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d-th-Text %d-th-String length=%d MallocSize=%d\n", __FUNCTION__, __LINE__, iTextReadCount, iTextStringReadCount, numentries, numentries*sizeof(UINT16));

    child = item->child;
    cRet = CJSON_GET_PASS;
    numentries = 0;
    while (child && cRet == CJSON_GET_PASS)
    {
        //read the array value into corresponding object directly
        mspAllTextInfo[iTextReadCount-1].psString[iTextStringReadCount-1].puiCharIndex[numentries] = (UINT16)child->valueint;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)%d, ", __FUNCTION__, __LINE__, child->valueint);

        numentries++;
        child = child->next;
    }
//    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d): %d-th-Text %d-th-String %d char read in\n", __FUNCTION__, __LINE__, iTextReadCount, iTextStringReadCount, numentries);

    return CJSON_GET_PASS;
}


char palGui_JSON_Get_Object(const cJSON *item, int iReadStatus)
{
    //travel all cJSON child item and recursive cJSON_Get to get all properties
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    cJSON *child = item->child;
    int numentries = 0;

    /* Count the number of entries. */
    while (child)
    {
        numentries++;
        child = child->next;
    }

    //printf("JSON_Get_Object try to get first object of total entries=%d\n", numentries);
    //set child point to item first child again to exetract data
    child = item->child;
    cRet = CJSON_GET_PASS;
    while (child && cRet == CJSON_GET_PASS)
    {
        #if 0
        if (child->string)
        {
            printf("JSON_Get_Object Name String=%s\n", child->string);
        }
        #endif

        cRet = palGui_JSON_Get(child, iReadStatus);
        child = child->next;

        //printf("JSON_Get_Object ret=%d\n", cRet);
    }

    return cRet;
}


char palGui_JSON_Get(const cJSON *item, int iReadStatus)
{
    char cRet = (char)ERROR_CJSON_GET_FAIL;
    int iSpecificRead = NO_SPECIFIC_READING;
    if (!item)
    {
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : empty item\n", __FUNCTION__, __LINE__);
        return (char)ERROR_CJSON_ITEM_IS_EMPTY;
    }

    //printf("JSON_Get type=%d, O_read=%d\n", item->type, iObjectReadStatus);

    switch ((item->type) & 0xFF)
    {
        case cJSON_NULL:
            //cRet = cJSON_strdup("null");
            cRet = (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
            break;

        case cJSON_False:
            //cRet = cJSON_strdup("false");
            cRet = (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
            break;

        case cJSON_True:
            //cRet = cJSON_strdup("true");
            cRet = (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
            break;

        case cJSON_Number:
            switch (iReadStatus)
            {
                case MENU_READING:
                    cRet = palGui_JSON_Get_Menu_Number(item);
                    break;

                case ITEM_READING:
                    cRet = palGui_JSON_Get_Item_Number(item);
                    break;

                case BITMAP_READING:
                    cRet = palGui_JSON_Get_Bitmap_Number(item);
                    break;

                case TEXT_READING:
                    cRet = palGui_JSON_Get_Text_Number(item);
                    break;

                case TEXT_STRING_READING:
                    cRet = palGui_JSON_Get_TextString_Number(item);
                    break;

                default:
                    cRet = palGui_JSON_Get_Number(item);
                    break;
            }
            break;

        case cJSON_String:
            cRet = palGui_JSON_Get_String(item);
            break;

        case cJSON_Array:
            if (!strcmp("MenuObjectArray",item->string))
            {
                iSpecificRead = MENU_READING;
                iMenuReadCount = 0;
                cRet = palGui_JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("ItemObjectArray",item->string))
            {
                iSpecificRead = ITEM_READING;
                iItemReadCount = 0;
                cRet = palGui_JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("ItemBitmapIDArray",item->string))
            {
                cRet = palGui_JSON_Get_ItemBitmapArray(item);
            }
            else if (!strcmp("ItemRectangleIDArray",item->string))
            {
                cRet = palGui_JSON_Get_ItemRectangleIDArray(item);
            }
            else if (!strcmp("ItemBrushIDArray",item->string))  //T100_Sander_0002
            {
                cRet = palGui_JSON_Get_ItemBrushArray(item);
            }
            else if (!strcmp("ItemTextIDArray",item->string))
            {
                cRet = palGui_JSON_Get_ItemTextArray(item);
            }
            else if (!strcmp("BitmapObjectArray",item->string))
            {
                iSpecificRead = BITMAP_READING;
                iBitmapReadCount = 0;
                cRet = palGui_JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("BitmapPaletteArray",item->string))
            {
                cRet = palGui_JSON_Get_Palette_Array(item);
            }
            else if (!strcmp("TextObjectArray",item->string))
            {
                iSpecificRead = TEXT_READING;
                iTextReadCount = 0;
                cRet = palGui_JSON_Get_ObjectArray(item, TEXT_READING);
            }
            else if (!strcmp("StringObjectArray",item->string))
            {
                iSpecificRead = TEXT_STRING_READING;
                iTextStringReadCount = 0;
                cRet = palGui_JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("ASCIIFontMaxHeightArray", item->string)) //T100_Sander_0021
            {
                cRet = palGui_JSON_Get_ASCIIFontMaxHeight_Array(item);
            }
            else if (!strcmp("LanguageUsedArray",item->string))
            {
                cRet = palGui_JSON_Get_LanguageUsed_Array(item);
            }
            else if (!strcmp("AllFontWidthInformationArray",item->string))
            {
                cRet = palGui_JSON_Get_AllFontWidthInfo_Array(item);
            }
            else if (!strcmp("FontArray",item->string))
            {
                cRet = palGui_JSON_Get_StringFont_Array(item);
            }
            else
            {
                //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : Read array with no define Name_String\n", __FUNCTION__, __LINE__);
                return (char)ERROR_CJSON_GET_FAIL;
            }
            break;

        case cJSON_Object:
            cRet = palGui_JSON_Get_Object(item, iReadStatus);
            break;
    }

    return cRet;
}



/*
Parse JSON string then convert that into to Gui object
 */
void palGui_JSON_Convertion(char *text)
{
    //char *out;
    cJSON *json;

    json=cJSON_Parse(text);
    if (!json)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)Error before: [%s]\n", __FUNCTION__, __LINE__, cJSON_GetErrorPtr());
    }
    else
    {
        #if 0
        out=cJSON_Print(json);
        cJSON_Delete(json);
        printf("%s\n",out);
        free(out);
        #else  //get CJson file data to OSD structure
        palGui_JSON_Get(json, NO_SPECIFIC_READING);
        cJSON_Delete(json);
        #endif

        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) palGui_JSON_Convertion Pass\n", __FUNCTION__, __LINE__);
    }
}


/* Read a JSON file, parse it*/
INT8 palGui_JSON_FileOpen(char *filename)
{
    FILE *pFile = NULL;
    UINT32 size;
    UINT32 br; //bw, br, i;
    char *cbuf = NULL;

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    pFile = fopen(filename, "rb");

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    if(pFile == NULL) //A70LV_Larry_0001
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) %s lost\r\n", __FUNCTION__, __LINE__, filename);
        return ERROR_CJSON_FILE_OPEN_FAIL;
    }

    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
//    fclose(pFile);
//
//    pFile = fopen(filename, "rb");

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) size %d\r\n", __FUNCTION__, __LINE__, size);

    cbuf = (char *) pvPortMalloc(size);

    memset(cbuf, '\0', size);

    fread(cbuf, size, 1, pFile);

    fclose(pFile);

    palGui_JSON_Convertion(cbuf);
    vPortFree(cbuf);

    return CJSON_GET_PASS;
}



//function define to get CJson result


INT8 Bitmap_Get(UINT32 ulBitmapID, OSD_BITMAP_INFO *psRetBitmap)
{
    INT16 iCount;

    //search Bitmap
    for (iCount=0; iCount<ulAllBitmapCount; iCount++)
    {
        //if BitmapID match
        if (ulBitmapID == mspAllBitmapInfo[iCount].ulBitmapID)
        {
            //psRetBitmap = (mspAllBitmapInfo+iCount);
            psRetBitmap->sSrc_Position.iX = mspAllBitmapInfo[iCount].sSrc_Coordinate.iX;
            psRetBitmap->sSrc_Position.iY = mspAllBitmapInfo[iCount].sSrc_Coordinate.iY;
            psRetBitmap->sBitmap_Size.iWidth = mspAllBitmapInfo[iCount].sBitmap_Size.iWidth;
            psRetBitmap->sBitmap_Size.iHeight = mspAllBitmapInfo[iCount].sBitmap_Size.iHeight;
            return GUI_FOUND;
        }
    }

    return ERROR_GUI_NOT_FOUND;
}


INT8 String_FillWidthInfo(UINT16 uiString_length, UINT32 ulFontOffset, UINT16 *puiChar_index, UINT8 *pucRet_char_width)
{
    UINT16 uiCount;

    for (uiCount=0; uiCount<uiString_length; uiCount++)
    {
        *(pucRet_char_width+uiCount) = *(mpucAllFontWidthInfo + ulFontOffset + *(puiChar_index+uiCount));
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Copy %d-th width %d to %d\n", __FUNCTION__, __LINE__, (uiCount+1), *(mpucAllFontWidthInfo + ulFontOffset + *(puiChar_index+uiCount)), *(pucRet_char_width+uiCount));
    }

    return GUI_PASS;
}


INT8 String_Get(UINT32 ulTextID, OSD_STRING_INFO * psRetString, UINT16 *puiStringWidth)
{
    INT16 iCount;
    STRING_INFO *psString;

    //searching
    for (iCount=0; iCount<ulAllTextCount; iCount++)
    {
        //if TextID match
        if (ulTextID == mspAllTextInfo[iCount].ulTextID)
        {
            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) TextID=%d found!\n", __FUNCTION__, __LINE__, (mspAllTextInfo+iCount)->ulTextID);
            psString = (mspAllTextInfo[iCount].psString+iActiveLanguageIndex);

            psRetString->ucStringMaxHeight = psString->ucStringMaxHeight;
            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)string maxheight=%d\n", __FUNCTION__, __LINE__, psRetString->ucStringMaxHeight);
            psRetString->uiStringLength = psString->uiStringLength;  //need to implemnet recursive function to paint string if string length over 32
            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Length=%d\n", __FUNCTION__, __LINE__, psRetString->uiStringLength);
            psRetString->ulFontOffset = psString->ulFontOffset;
            //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Fontoffset=%d\n", __FUNCTION__, __LINE__, psRetString->ulFontOffset);

            psRetString->puiCharIndex = (UINT16 *)pvPortMalloc(psRetString->uiStringLength*2);
            memcpy(psRetString->puiCharIndex, psString->puiCharIndex, psRetString->uiStringLength*2);

            #if 0
            {
                int i;
                for (i=0; i<psRetString->uiStringLength; i++)
                {
                    LOG_MSG(db_DV_C734, "(func:%s, line:%d)%d-th charIdx=%d, oriCharIdx=%d\n", __FUNCTION__, __LINE__, i, *(psRetString->puiCharIndex+i), *(psString->puiCharIndex+i));
                }
            }
            #endif

            psRetString->pucCharWdith = (UINT8*)pvPortMalloc(psRetString->uiStringLength);
            String_FillWidthInfo( psRetString->uiStringLength, psRetString->ulFontOffset, psRetString->puiCharIndex, psRetString->pucCharWdith);
            //LOG_MSG(db_DV_C734, "(func:%s, line:%d)test\n", __FUNCTION__, __LINE__);

            #if 1
            {
                INT16 i;
                //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)str_length=%d, offset=%d\n", __FUNCTION__, __LINE__, psRetString->uiStringLength, psRetString->ulFontOffset);
                *puiStringWidth = 0;
                for (i=0; i<psRetString->uiStringLength; i++)
                {
                    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)%d-th, idx=%d, width=%d\n", __FUNCTION__, __LINE__, i+1, *(psRetString->puiCharIndex+i), *(psRetString->pucCharWdith+i));
                    *puiStringWidth = *puiStringWidth + psRetString->pucCharWdith[i];
                }
                *puiStringWidth = *puiStringWidth + psRetString->uiStringLength;
            }
            #else

                LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) ActLang=%d length=%d, FontOffset=%d!\n", __FUNCTION__, __LINE__, iActiveLanguageIndex, psRetString->uiStringLength, psRetString->ulFontOffset);
                *puiStringWidth = psString->uiStringWidth + psString->uiStringLength;  //set the active string width
            #endif
            return GUI_FOUND;
        }
    }

    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) string not found\n", __FUNCTION__, __LINE__);
    return ERROR_GUI_NOT_FOUND;
}


INT8 Gui_ItemStructure_Link()
{
    INT16 iItemCount;
    INT16 iMenuCount;
    INT16 iItemCopy_Count;
    //INT16 i;

    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Total: %d Menu, %d Item be loaded\n", __FUNCTION__, __LINE__, ulAllMenuCount, ulAllItemCount);

    iItemCopy_Count = 0;
    //It maybe include empty item node when Item number is not match menu define, so this should be still use linked-list to implement Item link funciton at stage 2
    for (iMenuCount=0; iMenuCount<ulAllMenuCount; iMenuCount++)
    {
        iItemCopy_Count = 0;  //reset Item read count when menu ID increase
        for (iItemCount=0; iItemCount<ulAllItemCount; iItemCount++)
        {
            if ( mspAllItemInfo[iItemCount].ulParentMenuID == mspAllMenuInfo[iMenuCount].ulMenuID)
            {

                if (iItemCopy_Count< mspAllMenuInfo[iMenuCount].uiItemCount)  // these item number should be match menu info
                {
                    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)%d-th Item find parent %d-th MenuID=%d add to ppsMenuItemList[%d]\n", __FUNCTION__, __LINE__, iItemCount, iMenuCount, mspAllMenuInfo[iMenuCount].ulMenuID, iItemCopy_Count);
                    mspAllMenuInfo[iMenuCount].ppsMenuItemList[iItemCopy_Count] = &mspAllItemInfo[iItemCount];
                    iItemCopy_Count++;
                    #if 0
                    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)mspAllItemInfo[%d].sItemDrawingInfo.pulTextID Addr=0x%x\n", __FUNCTION__, __LINE__, iItemCount, mspAllItemInfo[iItemCount].sItemDrawingInfo.pulTextID);
                    for (i=0; i<mspAllItemInfo[iItemCount].sItemDrawingInfo.ucTextCount; i++)
                    {
                        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)mspAllItemInfo[%d].sItemDrawingInfo.pulTextID[%d]=%d\n", __FUNCTION__, __LINE__, iItemCount, i, mspAllItemInfo[iItemCount].sItemDrawingInfo.pulTextID[i]);
                    }
                    #endif
                }
                else
                {
                    return ERROR_GUI_ITEM_SIZE_OVER_LIMIT;
                }
            }
        }
    }

    return GUI_PASS;
}


//	RGB m_asPalette[] =	//T100_Sander_0016
//	{
//	{12, 110, 180},		// index 240 Christie Logo background
//	{19, 54, 89},		// index 241 Active Menu Title background
//	{45, 90, 135},		// index 242 Active Menu Item background
//	{61, 84, 108},     	// index  243 Inactive Menu Title background
//	{160, 160, 160},    // index  244 Inactive Menu Item background
//	{33, 100, 168},     // index  245 Focus Bar
//	{255, 0, 0},     	// index  246 Red
//	{0, 255, 0},     	// index  247 Green
//	{0, 0, 255},     	// index  248 Blue
//	{255, 255, 0},     	// index  249 Yellow
//	{255, 255, 255},    // index  250 White
//	{9, 45, 82},    	// index  251 Slider Bar
//	{255, 255, 255},    // index  252 Slider Bar background
//	{252, 252, 252},    // index  253 Transparent Color 1
//	{253, 253, 253},    // index  254 Transparent Color 2
//	{254, 0, 254}     	// index  255 Inhibit color
//	};
//	#define NUM_PALETTE_TABLE_ENTRIES (sizeof(m_asPalette)/sizeof(RGB))


//	INT8 Gui_ColorPalette_Replace()
//	{
//	    INT16 iCount;
//	    INT16 iIdx;
//
//	    LOG_MSG(db_ALWAYS, "(func:%s, line:%d)replace 240~256 colors\n", __FUNCTION__, __LINE__, ulAllMenuCount, ulAllItemCount);
//	    iIdx = 0;
//	    for (iCount=240; iCount<256; iCount++)
//	    {
//	        aiBitmapPalette[iCount*3] = m_asPalette[iIdx].ucRed;
//	        aiBitmapPalette[iCount*3+1] = m_asPalette[iIdx].ucGreen;
//	        aiBitmapPalette[iCount*3+2] = m_asPalette[iIdx].ucBlue;
//	        iIdx++;
//	    }
//
//	    return GUI_PASS;
//	}


//	INT8 Gui_Data_Load(char *filename, UINT32 ulSflAd)
//	{
//	    FIL Fil;	/* File object */
//	    FRESULT rc;     /* Result code */
//
//	    UINT32 br; //bw, br, i;
//	    char *cbuf = NULL;
//
//
//	    rc = f_open(&Fil, filename, FA_READ);
//	    cbuf = (char *) pvPortMalloc(f_size(&Fil));
//
//	    LOG_MSG(db_ALWAYS, "(func:%s, line:%d) after fs_open called len=%d\n", __FUNCTION__, __LINE__, f_size(&Fil));
//
//	    rc = f_read(&Fil, (void *)cbuf, f_size(&Fil), &br);
//
//	    if(rc || !br)
//	    {
//
//	        LOG_MSG(db_ALWAYS, "(func:%s, line:%d)Error %s f_read Fail !\n", __FUNCTION__, __LINE__, filename);
//	        f_close(&Fil);
//	        return ERROR_GUI_NOT_FOUND;
//	        /* Error or end of file */
//	    }
//
//	    f_close(&Fil);
//
//	    //LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Write %s data\n", __FUNCTION__, __LINE__, filename);
//	    //call C734 SPI-flahs write fucntion
//	    dvC734_SerialFlash_Write(ulSflAd, (UINT8 *)cbuf, f_size(&Fil));
//
//	    vPortFree(cbuf);
//
//	    return GUI_PASS;
//	}


INT8 palGui_Init(void)
{
    //UINT32 *pulExtFlashTag = (UINT32*)EXTERNAL_FLASH_TAG_ADDR;

	//*SPLASH_LOGO_ID = 0; //T100_Sander_0013

    //if(*pulExtFlashTag != EXTERNAL_FLASH_TAG)
    //{
        //LOG_MSG(db_APP_GUI, "JSON External Flash tag error!\r\n");
        //return eEXEC_CODE_FAIL;
    //}

    // No SD-MMC Card Hardware
    //JSON_Convertion((char *)m_pcBitmapStruct);
    //JSON_Convertion((char *)m_pcOSDStruct);
    //JSON_Convertion((char *)m_pcTextStruct);

    //JSON_FileOpen("BitmapStruct.json");
    //JSON_FileOpen("OSDStruct.json");
    //JSON_FileOpen("TextStruct.json");

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) BitmapStruct.json open\r\n", __FUNCTION__, __LINE__);
    if (CJSON_GET_PASS != palGui_JSON_FileOpen(BITMAP_STRUCT_JASON))
    {
        if(CJSON_GET_PASS != palGui_JSON_FileOpen(CUSTOM_BITMAP_STRUCT_JASON)) //A35G2_Larry_0067 for RD debug
        {
            return ERROR_CJSON_FILE_OPEN_FAIL;
        }
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) OSDStruct.json open\r\n", __FUNCTION__, __LINE__);
    if (CJSON_GET_PASS != palGui_JSON_FileOpen(OSD_STRUCT_JASON))
    {
        if(CJSON_GET_PASS != palGui_JSON_FileOpen(CUSTOM_OSD_STRUCT_JASON)) //A35G2_Larry_0067 for RD debug
        {
            return ERROR_CJSON_FILE_OPEN_FAIL;
        }
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) TextStruct.json open\r\n", __FUNCTION__, __LINE__);
    if (CJSON_GET_PASS != palGui_JSON_FileOpen(TEXT_STRUCT_JASON))
    {
        if(CJSON_GET_PASS != palGui_JSON_FileOpen(CUSTOM_TEXT_STRUCT_JASON)) //A35G2_Larry_0067 for RD debug
        {
            return ERROR_CJSON_FILE_OPEN_FAIL;
        }
    }

    //for test insert private palette setting into index 240~255
    //Gui_ColorPalette_Replace();

    if (GUI_PASS != Gui_ItemStructure_Link())
    {
        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Error Menu link failed!\n", __FUNCTION__, __LINE__);
        return ERROR_OSD_STRUCT_LINK_FAIL;
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Init complete!\n", __FUNCTION__, __LINE__);
    return GUI_PASS;
}

#if 0//(LOGO_REPLACE == 1)// R70G2_Bruce#0023
INT8 LogoPaletteFlag = -1;
INT8 ServiceLogoPaletteFlag = -1;

void palGui_LogoPaletteFlagInit(void)
{
    LogoPaletteFlag = -1;
}

void palGui_ServiceLogoPaletteFlagInit(void) //A65_OPTOMA_Julie_0076
{
    ServiceLogoPaletteFlag = -1;
}

INT8 palGui_SecondLogoHWInit(void) //T100_Optoma_Coda:
{
    if (LogoPaletteFlag == 1)
        return 0;
	if(palGui_InitSemaphore() != GUI_PASS)	   //A70LV_Doulas_0014
		return ERROR_GUI_SEMAPHORE_FAIL;
#ifdef SCALER_C821_C789
	halC789Ctrl_OutputEnableSet(0);
#endif
    UINT16 ret = 0;
    UINT8 Palette_toWrite[768] = {0};
    UINT8 Palette_2ndLogo[OSD_2ND_LOGO_PALETTE_SIZE] = {0};
    FILE *pFile = fopen(OSD_2ND_LOGO_PALETTE_FILE, "rb");
    if(pFile == NULL)
    {
        LOG_MSG(db_UPGRADE, "%s() fopen(OSD_2ND_LOGO_PALETTE_FILE) failed!\r\n", __FUNCTION__);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }
    //Get Second Logo Palette 0 ~ 118 total 119
    ret = fread(Palette_2ndLogo, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);
    if (ret != OSD_2ND_LOGO_PALETTE_SIZE)
    {
        LOG_MSG(db_UPGRADE, "%s() fread(%d) failed!\r\n", __FUNCTION__, ret);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }

    // copy aiBitmapPalette[768] to Palette_toWrite[768]
    memcpy(Palette_toWrite, aiBitmapPalette, sizeof(Palette_toWrite));
    // copy Palette_2ndLogo[119*3] to Palette_toWrite[768]
    memcpy(Palette_toWrite, Palette_2ndLogo, OSD_2ND_LOGO_PALETTE_SIZE);

    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
    #ifdef SCALER_C821_C789
    	dvC821_2ND_LOGO_OSD_Init(256, Palette_toWrite, GUI_INHITBIT_COLOR, eCM_USER_LOGO_2D); //A65_OPTOMA_Julie_0067
    #endif
        palGui_SemaphoreGive(__FUNCTION__);
        LOG_MSG(db_HAL_GUI, "%s() pass.\n", __FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_GUI, "%s() fail!\n", __FUNCTION__);
    }
    LogoPaletteFlag = 1;
	ServiceLogoPaletteFlag = 0;
#ifdef SCALER_C821_C789
	halC789Ctrl_OutputEnableSet(1);
#endif
    LOG_MSG(db_UPGRADE, "%s(%d)\r\n", __FUNCTION__, LogoPaletteFlag);
    return GUI_PASS;
}

INT8 palGui_Service_SecondLogoHWInit(void) //A65_OPTOMA_Julie_0076
{
    if (ServiceLogoPaletteFlag == 1)
        return 0;
	if(palGui_InitSemaphore() != GUI_PASS)	   //A70LV_Doulas_0014
		return ERROR_GUI_SEMAPHORE_FAIL;
#ifdef SCALER_C821_C789
	halC789Ctrl_OutputEnableSet(0);
#endif
    UINT16 ret = 0;
    UINT8 Palette_toWrite[768] = {0};
    UINT8 Palette_2ndLogo[OSD_2ND_LOGO_PALETTE_SIZE] = {0};
    FILE *pFile = fopen(OSD_2ND_LOGO_SERVICE_PALETTE_FILE, "rb");
    if(pFile == NULL)
    {
        LOG_MSG(db_UPGRADE, "(%s,%d) fopen(%s) failed!\r\n", __FUNCTION__, __LINE__, OSD_2ND_LOGO_SERVICE_PALETTE_FILE);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }
    //Get Second Logo Palette 0 ~ 118 total 119
    ret = fread(Palette_2ndLogo, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);
    if (ret != OSD_2ND_LOGO_PALETTE_SIZE)
    {
        LOG_MSG(db_UPGRADE, "%s() fread(%d) failed!\r\n", __FUNCTION__, ret);// db_ALWAYS
        return (ERROR_GUI_EMPTY_STRING);
    }

    // copy aiBitmapPalette[768] to Palette_toWrite[768]
    memcpy(Palette_toWrite, aiBitmapPalette, sizeof(Palette_toWrite));
    // copy Palette_2ndLogo[119*3] to Palette_toWrite[768]
    memcpy(Palette_toWrite, Palette_2ndLogo, OSD_2ND_LOGO_PALETTE_SIZE);

    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
    #ifdef SCALER_C821_C789
    	dvC821_2ND_LOGO_OSD_Init(256, Palette_toWrite, GUI_INHITBIT_COLOR, eCM_USER_LOGO_SERVICE_2D); //A65_OPTOMA_Julie_0067
    #endif
        palGui_SemaphoreGive(__FUNCTION__);
        LOG_MSG(db_HAL_GUI, "%s() pass.\n", __FUNCTION__);
    }
    else
    {
        LOG_MSG(db_HAL_GUI, "%s() fail!\n", __FUNCTION__);
    }
    ServiceLogoPaletteFlag = 1;
	LogoPaletteFlag = 0;
#ifdef SCALER_C821_C789
	halC789Ctrl_OutputEnableSet(1);
#endif
    LOG_MSG(db_UPGRADE, "%s(%d)\r\n", __FUNCTION__, ServiceLogoPaletteFlag);
    return GUI_PASS;
}


INT8 palGui_SecondLogoPaletteStore(char *fileName, UINT8 ucIndex) //A65_OPTOMA_Julie_0076 // R70G2_Bruce#0023
{
    FILE *pFile = NULL;
    UINT8 Palette[512] = {0};
    UINT16 r = 0, w = 0;

    pFile = fopen(fileName, "rb");
    if (pFile == NULL)
    {
        LOG_MSG(db_UPGRADE, "%s() fopen(TMP_2ND_LOGO_PALETTE_FILE) failed!\r\n", __func__);
        return (ERROR_GUI_EMPTY_STRING);
    }
    r = fread(Palette, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);

	if(ucIndex == eCM_USER_LOGO_SERVICE_2D) //A65_OPTOMA_Julie_0076
	{
	    pFile = fopen(OSD_2ND_LOGO_SERVICE_PALETTE_FILE, "wb+");
	    if (pFile == NULL)
	    {
	        LOG_MSG(db_UPGRADE, "%s() fopen(OSD_2ND_LOGO_SERVICE_PALETTE_FILE) failed!\r\n", __func__);
	        return (ERROR_GUI_EMPTY_STRING);
	    }
	}
	else
	{
	    pFile = fopen(OSD_2ND_LOGO_PALETTE_FILE, "wb+");
	    if (pFile == NULL)
	    {
	        LOG_MSG(db_UPGRADE, "%s() fopen(OSD_2ND_LOGO_PALETTE_FILE) failed!\r\n", __func__);
	        return (ERROR_GUI_EMPTY_STRING);
	    }
	}
    w = fwrite(Palette, 1, OSD_2ND_LOGO_PALETTE_SIZE, pFile);
    fclose(pFile);

    if ((r != OSD_2ND_LOGO_PALETTE_SIZE) || (w != OSD_2ND_LOGO_PALETTE_SIZE))
    {
        LOG_MSG(db_UPGRADE, "%s() r = %d, w = %d!\r\n", __func__, r, w);
        return (ERROR_GUI_EMPTY_STRING);
    }
    return GUI_PASS;
}
#endif
#if 0
INT8 palGui_HWInit(void)
{
#if (LOGO_REPLACE == 1)// R70G2_Bruce#0023
	if (LogoPaletteFlag == 0 && ServiceLogoPaletteFlag == 0) //A65_OPTOMA_Julie_0076
	{
		return 0;
	}
	else
	{
	#ifdef SCALER_C821_C789
		halC789Ctrl_OutputEnableSet(0);
	#endif
	}
#endif
    if(palGui_InitSemaphore() != GUI_PASS)      //A70LV_Doulas_0014
        return ERROR_GUI_SEMAPHORE_FAIL;

#ifdef SCALER_FPGA_F34
    halGui_HWInit();
#else
    halGui_HWInit(256, aiBitmapPalette, GUI_INHITBIT_COLOR);  //need to get amount of palette color and palette data point
#endif

#if (LOGO_REPLACE == 1)// R70G2_Bruce#0023
	{
		LogoPaletteFlag = 0;
		ServiceLogoPaletteFlag = 0;
	#ifdef SCALER_C821_C789
		halC789Ctrl_OutputEnableSet(1);
	#endif
		LOG_MSG(db_UPGRADE, "%s(%d,%d)\r\n", __FUNCTION__, LogoPaletteFlag, ServiceLogoPaletteFlag);
	}
#endif

    return GUI_PASS;
}

INT8 palGui_OSDReload(void)
{
    halGui_OSDReload();

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}
#endif
#if (LOGO_REPLACE == 1)
INT8 palGui_2ND_LOGO_OSDReload(UINT8 ucIndex) //A65_OPTOMA_Julie_0076 //A65_OPTOMA_Julie_0067
{
    halGui_2ND_LOGO_OSDReload(ucIndex);

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}
#endif

INT8 palGui_OSD_Copy(void) //A70LV_Larry_0067
{
    halGui_OSD_Copy();

    return GUI_PASS;
}

INT8 palGui_OSD_On(COORDINATE sStart_Position, BLOCK_SIZE_INFO sDraw_Size)
{
    COORDINATE sDes_Position;
    BLOCK_SIZE_INFO sDes_Size;
    OSD_TRANSPARENCY_COLOR_INFO sTra_Color;

    //sDes_Position.iX = sStart_Position.iX;
    //sDes_Position.iY = sStart_Position.iY;
    memcpy(&sDes_Position, &sStart_Position, sizeof(COORDINATE));
    //sDes_Size.iWidth = sDraw_Size.iWidth;
    //sDes_Size.iHeight = sDraw_Size.iHeight;
    memcpy(&sDes_Size, &sDraw_Size, sizeof(BLOCK_SIZE_INFO));
    sTra_Color.ucT_Color1 = GUI_INHITBIT_COLOR;//GUI_TRANSPARENT_COLOR_1;
    sTra_Color.ucT_Color2 = GUI_INHITBIT_COLOR;
    sTra_Color.ucT_Color3 = GUI_INHITBIT_COLOR;
    sTra_Color.ucT_Color4 = GUI_INHITBIT_COLOR;

#ifdef SCALER_FPGA_F34
    halGui_OSD_On(sDes_Position, sDes_Size);
#else
    halGui_OSD_On(sDes_Position, sDes_Size, GUI_ON, sTra_Color);  //disable transparency color for  first stage
#endif

    return GUI_PASS;
}


INT8 palGui_OSD_Off(void)
{
    halGui_OSD_Off();

    return GUI_PASS;
}


INT8 palGui_OSD_Set_InhitbitColor( INT8 cEnable, INT16 iInhibit_Color )
{
    halGui_OSD_Set_InhitbitColor(cEnable, iInhibit_Color);

    return GUI_PASS;
}


INT8 palGui_Language_Set(UINT8 ucLanguage)
{
    if ((int)ucLanguage < iTotalLanguageUsed)
    {
        iActiveLanguageIndex = (int)ucLanguage;
        return GUI_PASS;
    }

    return ERROR_GUI_NOT_FOUND;
}

MENU_INFO* palGui_MenuStructPoint_Get(void)
{
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) Get All Menu struct point\r\n", __FUNCTION__, __LINE__);
    return mspAllMenuInfo;
}

UINT32 palGui_MenuCount_Get(void)
{
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) Get All menu count\r\n", __FUNCTION__, __LINE__);
    return ulAllMenuCount;
}

INT8 palGui_Paint_Bitmap(UINT32 ulBitmapID, COORDINATE sStart_Position)
{
    //BITMAP_INFO sRetBitmap;
    OSD_BITMAP_INFO sBitmap;
    if( GUI_FOUND == Bitmap_Get(ulBitmapID, &sBitmap) )
    {
        START_POINT_INFO sDes_position;
        memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT_INFO));

        if(ulBitmapID != 40 && sDes_position.iX == 0 && sDes_position.iY == 0) //G100_Coda_00106
        {
            LOG_MSG(db_HAL_GUI, "NG Bitmap %d : X %d , Y %d\r\n",ulBitmapID, sDes_position.iX, sDes_position.iY);
        }

        //halGui_Paint_Bitmap(sBitmap, sDes_position);  //need to design return type from driver to Application at here

        return GUI_FOUND;
    }

    return ERROR_GUI_NOT_FOUND;
}
//ZU860_Clare_0132, add, >>>
#if 1
INT8 palGui_Paint_Specified_Bitmap(UINT32 ulBitmapID, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Position, BLOCK_SIZE_INFO sDraw_Size)
{
    OSD_BITMAP_INFO sBitmap;
    if( GUI_FOUND == Bitmap_Get(ulBitmapID, &sBitmap) )
    {
		COORDINATE sBmp_position;
		BLOCK_SIZE_INFO sBmp_Size;
		COORDINATE sDes_position;
		memcpy(&sBmp_position, &sSrc_Offset_Position, sizeof(COORDINATE));
		memcpy(&sBmp_Size, &sDraw_Size, sizeof(BLOCK_SIZE_INFO));
		memcpy(&sDes_position, &sDraw_Position, sizeof(COORDINATE));

        halGui_Paint_Specified_Bitmap(sBitmap, sBmp_position, sDes_position, sBmp_Size);

		return GUI_FOUND;
	}
    return ERROR_GUI_NOT_FOUND;
}
#else
INT8 palGui_Paint_Specified_Bitmap(COORDINATE sSrc_Position, BLOCK_SIZE_INFO sDraw_Size, COORDINATE sStart_Position)
{
	START_POINT_INFO sBmp_position;
	RECT_SIZE_INFO sBmp_Size;
	START_POINT_INFO sDes_position;
	memcpy(&sBmp_position, &sSrc_Position, sizeof(START_POINT_INFO));
	memcpy(&sBmp_Size, &sDraw_Size, sizeof(BLOCK_SIZE_INFO));
	memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT_INFO));

	palGui_SemaphoreTake(__FUNCTION__);
	dvC821_Paint_Specified_Bitmap(sBmp_position, sBmp_Size, sDes_position);  //need to design return type from driver to Application at her
	palGui_SemaphoreGive(__FUNCTION__);

	return GUI_FOUND;
}
#endif
//ZU860_Clare_0132, add, <<<

//T100_Sander_0013
INT8 palGui_Paint_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sStart_Position)
{
    palGui_Paint_Bitmap(*(SPLASH_LOGO_ID + ulBitmapIDIndex), sStart_Position);

    return GUI_FOUND;
}

//ZU860_Clare_0132, add, >>>
INT8 palGui_Paint_Specified_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Start_Position, BLOCK_SIZE_INFO sDraw_Size)
{
    palGui_Paint_Specified_Bitmap(*(SPLASH_LOGO_ID + ulBitmapIDIndex), sSrc_Offset_Position, sDraw_Start_Position, sDraw_Size);
    return GUI_FOUND;
}
//ZU860_Clare_0132, add, <<<

//ZU860_Clare_0019, >>>
INT8 palGui_Paint_TextHightLight(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)      // for OPTOMA IP/PIN Text
{
    COORDINATE sDes_position;
    BLOCK_SIZE_INFO sDes_size;
    INT8 iVal = 0;  //ZU860_Doulas_0136

    sDes_position.iX = sStart_Position.iX;
    sDes_position.iY = sStart_Position.iY;
    sDes_size.iWidth = sSize.iWidth;
    sDes_size.iHeight = sSize.iHeight;

#ifdef CUSTOM_OPTOMA_ZU860	//ZU860_Clare_0093
    halGui_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR); 	// EK816U_626U_Energy_0019
#endif
    sDes_size.iHeight = sSize.iHeight;
    iVal = halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

    return iVal;
}
	//ZU860_Clare_0019, <<<
//T100_Sander_0006
INT8 palGui_Paint_Rectangle(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)
{
    INT8 cResult = 0;

    cResult = halGui_Paint_Rectangle(sStart_Position, sSize, iColor_Index);

    return cResult;
}

INT8 palGui_Paint_RectanglewithShadow(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)
{
    INT8 cResult = 0;
    COORDINATE sDes_position;
    BLOCK_SIZE_INFO sDes_size;

    memcpy(&sDes_position, &sStart_Position, sizeof(COORDINATE));
    memcpy(&sDes_size,     &sSize,           sizeof(BLOCK_SIZE_INFO));

    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        halGui_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR);

        sDes_position.iX = sStart_Position.iX + 1;
        sDes_position.iY = sStart_Position.iY + 1;
        sDes_size.iWidth = sSize.iWidth - 1;
        sDes_size.iHeight = sSize.iHeight - 1;

        halGui_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR_BACKGROUND);
        sDes_position.iX = sStart_Position.iX + 1;
        sDes_position.iY = sStart_Position.iY + 1;
        sDes_size.iWidth = sSize.iWidth - 2;
        sDes_size.iHeight = sSize.iHeight - 2;

        cResult = halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        palGui_SemaphoreGive(__FUNCTION__);
    }

    return cResult;


}


INT8 palGui_Paint_RectanglewithBoarder(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index, INT16 iColor_Background, INT16 iBoarder_Index)  //T100_Sander_0002
{
    INT8 cResult = 0;
    COORDINATE sDes_position;
    BLOCK_SIZE_INFO sDes_size;

    memcpy(&sDes_position, &sStart_Position, sizeof(COORDINATE));
    memcpy(&sDes_size, &sSize, sizeof(BLOCK_SIZE_INFO));

    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        if(iBoarder_Index != ebiNON)
        {
            cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Background);

            if(iBoarder_Index == ebiFULL)
            {
                sDes_position.iX = sStart_Position.iX + 1;
                sDes_position.iY = sStart_Position.iY + 1;
                sDes_size.iWidth = sSize.iWidth - 2;
                sDes_size.iHeight = sSize.iHeight - 2;
            }
            else if(iBoarder_Index == ebiHIGHLIGHT)
            {
                sDes_position.iX = sStart_Position.iX + 1;
                sDes_position.iY = sStart_Position.iY + 1;
                sDes_size.iWidth = sSize.iWidth - 1;
                sDes_size.iHeight = sSize.iHeight - 1;

                cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR);

                sDes_size.iWidth = sSize.iWidth - 2;
                sDes_size.iHeight = sSize.iHeight - 2;
            }
        }

        cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        palGui_SemaphoreGive(__FUNCTION__);
    }

    return cResult;


}


INT8 palGui_Paint_RectangFrame(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index) //A70LV_Larry_0378
{
    INT8 cResult = 0;
    COORDINATE sDes_position;
    BLOCK_SIZE_INFO sDes_size;
    UINT16     uiLayoutWide = 5;

    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        //TOP
        sDes_position.iX = sStart_Position.iX;
        sDes_position.iY = sStart_Position.iY;
        sDes_size.iWidth  = sSize.iWidth;
        sDes_size.iHeight = uiLayoutWide;
        cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        //Left
        sDes_position.iX = sStart_Position.iX;
        sDes_position.iY = sStart_Position.iY;
        sDes_size.iWidth  = uiLayoutWide;
        sDes_size.iHeight = sSize.iHeight;
        cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        //Right
        sDes_position.iX = sStart_Position.iX + sSize.iWidth - uiLayoutWide;
        sDes_position.iY = sStart_Position.iY;
        sDes_size.iWidth  = uiLayoutWide;
        sDes_size.iHeight = sSize.iHeight;
        cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        //Bottom
        sDes_position.iX = sStart_Position.iX;
        sDes_position.iY = sStart_Position.iY + sSize.iHeight - uiLayoutWide;
        sDes_size.iWidth  = sSize.iWidth;
        sDes_size.iHeight = uiLayoutWide;

        cResult &= halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        palGui_SemaphoreGive(__FUNCTION__);
    }

    return cResult;

}


INT8 palGui_Paint_Text( UINT32 ulTextID, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment )
{
    //STRING_INFO sString;
    OSD_STRING_INFO sOsdString;
    UINT16 uiStringWidth;

    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Paint TextID=%d, start_x=%d, BoxWidth=%d\n", __FUNCTION__, __LINE__, ulTextID, sStart_Position.iX, sTextBoxSize.iWidth);
    if(ulTextID == 0) //Tahoma,12,75,0 //A70LV_Larry_0211
    {
        return GUI_PASS;
    }

    #if 0     //H2 wait review
	#ifdef CUSTOM_BARCO //A35G2_BRC_Casper_0091
    if((iActiveLanguageIndex != eGUI_LANGUAGE_ENGLISH) && (iActiveLanguageIndex != eGUI_LANGUAGE_S_CHINESE))
    {
        sStart_Position.iY += 4;
    }
	#endif
	#endif

    if( GUI_FOUND == String_Get(ulTextID, &sOsdString, &uiStringWidth) )
    {
        START_POINT_INFO sDes_position;

		if(uiStringWidth > sTextBoxSize.iWidth) //A70LV_Larry_0267
		{
			LOG_MSG(db_HAL_GUI_CK, "[%03d]Over String Box width %d string width %d\n", ulTextID, sTextBoxSize.iWidth, uiStringWidth);

			while(uiStringWidth > sTextBoxSize.iWidth)
			{
				sOsdString.uiStringLength -= 1;
				if(uiStringWidth - sOsdString.pucCharWdith[sOsdString.uiStringLength])
				{
					uiStringWidth -= sOsdString.pucCharWdith[sOsdString.uiStringLength];
				}
			}

			//LOG_MSG(db_ALWAYS, "reduce uiStringWidth %d\n", uiStringWidth);
		}
        //ToDo eAlignment : adjust string start position according to TextBoxSize and string width
        switch (eAlignment)
        {
            case eoaLEFT:
                memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT_INFO));
                break;

            case eoaCENTERED:
                {
                    INT16 iX_start;
                    iX_start = sStart_Position.iX + (sTextBoxSize.iWidth - uiStringWidth)/2;
                    if (iX_start >  0)
                    {
                        sDes_position.iX = iX_start;
                    }
                    else
                    {
                        sDes_position.iX = sStart_Position.iX;
                    }
                    sDes_position.iY = sStart_Position.iY;
                }
                break;

            case eoaRIGHT:
                {
                    INT16 iX_start;
                    iX_start = sStart_Position.iX + (sTextBoxSize.iWidth - uiStringWidth);
                    if (iX_start >  0)
                    {
                        sDes_position.iX = iX_start;
                    }
                    else
                    {
                        sDes_position.iX = sStart_Position.iX;
                    }
                    sDes_position.iY = sStart_Position.iY;
                }
                break;

            default:
                memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT_INFO));
                break;
        }

        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Des_(x,y)=(%d,%d) str_width=%d \n", __FUNCTION__, __LINE__, sDes_position.iX, sDes_position.iY, uiStringWidth);

        halGui_Paint_SubString(sOsdString, sDes_position, iTextColor, 0);

        //free that all malloc memory size in this function
        vPortFree(sOsdString.puiCharIndex);
        vPortFree(sOsdString.pucCharWdith);

    }
    else
    {
        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)ERROR : TextID=%d not found!\n", __FUNCTION__, __LINE__, ulTextID);
        return ERROR_GUI_NOT_FOUND;
    }
    return GUI_PASS;
}


INT8 palGui_Paint_ASCIIString( UINT8 *pucString, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment )
{
    INT16 iCount;
    UINT16 uiStringWidth;
    OSD_STRING_INFO sOsdString;
    START_POINT_INFO sDes_position;

    sOsdString.ulFontOffset = GUI_STRING_ASCII_FONT_OFFSET;
    sOsdString.ucStringMaxHeight = mucAsciiFontMaxHeight[iActiveLanguageIndex + 1]; //T100_Sander_0021
    sOsdString.uiStringLength = strlen((char *)pucString);  //need to implemnet recursive function to paint string if string length over 32

    #if 0     //H2 wait review
    #if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)
    sStart_Position.iY +=6; //osd tool v53 always fixed height 12,but g100 wordings height = 16, so need adjust by code. //G100_Coda_0063
    #endif
    #endif

    if (sOsdString.uiStringLength < 1)
        return ERROR_GUI_EMPTY_STRING;

    sOsdString.puiCharIndex = (UINT16 *)pvPortMalloc(sOsdString.uiStringLength*2);
    for (iCount=0; iCount<sOsdString.uiStringLength; iCount++)
    {
        *(sOsdString.puiCharIndex+iCount) = (UINT16)*(pucString+iCount);
    }

    sOsdString.pucCharWdith = (UINT8*)pvPortMalloc(sOsdString.uiStringLength);
    String_FillWidthInfo( sOsdString.uiStringLength, sOsdString.ulFontOffset, sOsdString.puiCharIndex, sOsdString.pucCharWdith);

    uiStringWidth = 0;  //calculate ASCII string width
    for (iCount=0; iCount<sOsdString.uiStringLength; iCount++)
    {
        uiStringWidth = uiStringWidth + *(sOsdString.pucCharWdith + iCount);
    }
    uiStringWidth = uiStringWidth + sOsdString.uiStringLength; //add one pixel for each character for C734 OSD drawing

    if(uiStringWidth > sTextBoxSize.iWidth)
    {
        LOG_MSG(db_HAL_GUI_CK, "Over String Box width %d string width %d\n", sTextBoxSize.iWidth, uiStringWidth);
        LOG_MSG(db_HAL_GUI_CK, "%s\n", pucString);

        while(uiStringWidth > sTextBoxSize.iWidth)
        {
            sOsdString.uiStringLength -= 1;
            if(uiStringWidth - sOsdString.pucCharWdith[sOsdString.uiStringLength])
            {
                uiStringWidth -= sOsdString.pucCharWdith[sOsdString.uiStringLength];
            }
        }

        //LOG_MSG(db_ALWAYS, "reduce uiStringWidth %d\n", uiStringWidth);
    }

    switch (eAlignment)
    {
        case eoaLEFT:
            memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT_INFO));
            break;

        case eoaCENTERED:
            {
                INT16 iX_start;
                iX_start = sStart_Position.iX + (sTextBoxSize.iWidth - uiStringWidth)/2;
                if (iX_start >  0)
                {
                    sDes_position.iX = iX_start;
                }
                else
                {
                    sDes_position.iX = sStart_Position.iX;
                }
                sDes_position.iY = sStart_Position.iY;
            }
            break;

        case eoaRIGHT:
            {
                INT16 iX_start;
                iX_start = sStart_Position.iX + (sTextBoxSize.iWidth - uiStringWidth);
                if (iX_start >  0)
                {
                    sDes_position.iX = iX_start;
                }
                else
                {
                    sDes_position.iX = sStart_Position.iX;
                }
                sDes_position.iY = sStart_Position.iY;
            }
            break;

        default:
            memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT_INFO));
            break;
    }

    halGui_Paint_SubString(sOsdString, sDes_position, iTextColor, 0);  //String background color default set to inhibit color for first test version

    vPortFree(sOsdString.puiCharIndex);
    vPortFree(sOsdString.pucCharWdith);

    //free that all malloc memory size in this function
    return GUI_PASS;
}


INT8 palGui_Paint_Number(INT32 iNumber, eNUMERIC_SIGN eNumMode, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment)
{
    UINT8 ucIsPrefix;
    UINT8 aucVal_Str[8];
    UINT8 ucStr_lenght;
    INT32 iTnum;
    INT16 iCount;


    ucStr_lenght = 0;
    if ((eNumMode == ensNON) || (eNumMode == ensSTRING))  //T100_Sander_0002
    {
        ucIsPrefix = 0;
        if (iNumber < 0)
        {
            return ERROR_GUI_NOT_ALLOW_NEGATIVE;
        }
    }
    else
    {
        ucIsPrefix = 1;
    }

    if ((eNumMode == ensNEGATIVE) & (iNumber < 0))
    {
        aucVal_Str[0] = '-';//0x2d; //set the minus sign char "-" ASCII code
    }

    if ((eNumMode == ensNEGATIVE) & (iNumber >= 0))
    {
        aucVal_Str[0] = ' ';//0x20; //set the space char " " ASCII code
    }

    if ((eNumMode == ensDUO) & (iNumber < 0))
    {
        aucVal_Str[0] = '-';//0x2d;//14; //set the minus sign char "-" ASCII code
    }

    if ((eNumMode == ensDUO) & (iNumber >= 0))
    {
        aucVal_Str[0] = '+';//0x2b;//12; //set the plus sign "+" ASCII code
    }

    if ((eNumMode != ensNON) & (iNumber < 0))
    {
        iNumber = -1 * iNumber;
    }

    iTnum = iNumber;
    while (iTnum >= 10)
    {
        iTnum = iTnum/10;
        ucStr_lenght++;
    }
    ucStr_lenght++;

    for (iCount=ucStr_lenght; iCount>0; iCount--)
    {
        //fill in the number character to array for numeric string display.
        aucVal_Str[ucIsPrefix + iCount - 1] = iNumber % 10 + '0';//0x30; //0x30 is '0' ASCII code.
        iNumber = iNumber / 10;
    }
    if (eNumMode == ensSTRING)  //T100_Sander_0002
    {
    	aucVal_Str[ucIsPrefix + ucStr_lenght] = '.';
    	aucVal_Str[ucIsPrefix + ucStr_lenght + 1] = '\0';
    }
	else
	{
    	aucVal_Str[ucIsPrefix + ucStr_lenght] = '\0';
	}

    palGui_Paint_ASCIIString(aucVal_Str, sStart_Position, sTextBoxSize, iTextColor, eAlignment);
    return GUI_PASS;
}

INT8 palGui_GuiData_EraseAll(void) //A70LV_Larry_0004
{
    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        //call C734 SPI-flash erase fucntion
        halGui_GuiData_EraseAll();

        LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)Erase spi-flash complete!\n", __FUNCTION__, __LINE__);

        palGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 palGui_GuiData_FlashEraseSector(UINT32 ulAddr, UINT32 ulSize) //A70LV_Larry_0004
{
    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        halGui_GuiData_FlashEraseSector(ulAddr, ulSize);

        palGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

INT8 palGui_GuiData_FlashWrite(UINT32 ulAddr, UINT8 *pucData, UINT32 ulSize) //A70LV_Larry_0004
{
    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        //H2 wait reivew
        //halGui_GuiData_FlashWrite(ulAddr, pucData, ulSize);

        palGui_SemaphoreGive(__FUNCTION__);
    }

    return GUI_PASS;
}

#define FLASH_SECTION_SIZE 0x10000

#if 0
UINT32 palGui_GuiData_PartialWrite(char *fileName, UINT32 ulAddr) //A70LV_Larry_0134
{
    UINT32 ulBlock = 0;
    UINT32 ulCount = 0;
    UINT32 ulCheckSum = 0;
    UINT32 ulExtSheckSum = 0;
    UINT32 ulSectorSize = 0; //A70LV_Larry_0158
    UINT32 ulFileSize = 0;
    FILE *pFile = NULL;
    UINT8 *pcExtAddr = NULL;
    UINT8 *pcReadAddr = NULL;

    pFile = fopen(fileName, "rb");

    if(pFile == NULL)
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) Open %s failed!\r\n", __FUNCTION__, __LINE__, fileName);
        return 0;
    }
    else //計算.dat szie //A35G2_CDS_Larry_0036
    {
        fseek(pFile, 0, SEEK_END);
        ulSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
    }

    if((ulSize % 0x10000 == 0))
    {
        ulBlock = ulSize/0x10000;
    }
    else
    {
        ulBlock = (ulSize/0x10000) + 1;
    }

    ulFileSize = ulBlock*0x10000;

    pcExtAddr = (UINT8 *)malloc(ulFileSize);
    memset(pcExtAddr, 0xFF, ulFileSize);
    fread(pcExtAddr, ulSize, 1, pFile);
    fclose(pFile);
    pcReadAddr = pcExtAddr;

    for(ulCount = 0; ulCount < ulBlock; ulCount++)
    {
        ulCheckSum = 0;
        ulSectorSize = 0; //A70LV_Larry_0158
        ulExtSheckSum = 0;

        if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
        {
            ulCheckSum = dvC821_SerialFlash_CheckSum(ulAddr, 0x10000);

            palGui_SemaphoreGive(__FUNCTION__);
        }

        do
        {
            ulExtSheckSum = ulExtSheckSum + (*(pcReadAddr++) * (ulSectorSize + 1));
            ulSectorSize++; //A70LV_Larry_0158
        }while(ulSectorSize < 0x10000); //64K


        if(ulCheckSum != ulExtSheckSum)
        {
            palGui_GuiData_FlashEraseSector(ulAddr, 0x10000);
            palGui_GuiData_FlashWrite(ulAddr, (pcExtAddr + ulCount * 0x10000), 0x10000);
            MS_SLEEP(10);

            if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
            {
                ulCheckSum = dvC821_SerialFlash_CheckSum(ulAddr, 0x10000);

                palGui_SemaphoreGive(__FUNCTION__);
            }
            MS_SLEEP(10);

            if(ulExtSheckSum != ulCheckSum)
            {
                LOG_MSG(db_UPGRADE, "Flash Copy Fail\n");
            }
            else
            {
                LOG_MSG(db_UPGRADE, "Flash Copy Pass\n");
            }
        }

        ulAddr = ulAddr + 0x10000;
//        ulExtAddr = ulExtAddr + 0x10000;
//        pucData = (UINT8 *)ulExtAddr;

        LOG_MSG(db_UPGRADE, "Copy Address %03d 0x%08x-0x%08x [0x%08x][0x%08x]\n", ulCount, ulAddr,ulAddr+0xFFFF, ulExtSheckSum, ulCheckSum);
        //MS_SLEEP(10);
    }

    free(pcExtAddr);

    return GUI_PASS;
}
#endif /* 0 */

#if 1
UINT32 palGui_GuiData_PartialWrite(char *fileName, UINT32 ulFlashAddr, UINT32 ulRAMAddr, UINT8 ucFocus) //A35G2_CDS_Larry_0050
{
    UINT32 cResult = 0;

    cResult = halGui_GuiData_PartialWrite(fileName, ulFlashAddr, ulRAMAddr, ucFocus);

    return cResult;
}

#endif /* 0 */

UINT32 palGui_Upgrade(UINT8 ucFocus) //A35G2_CDS_Larry_0050
{
    return halGui_Upgrade(ucFocus);
}

#if 0//(LOGO_REPLACE == 1)
void palGui_Upgrade_Second_Logo_Replace(UINT8 ucFocus)  //A35G2_Coda_0067
{
    halGui_OSD_Memory_Protect(FALSE);

	halGui_SecondLogoPaletteStore(TMP_2ND_LOGO_PALETTE_FILE, eCM_USER_LOGO_2D);
 	palGui_GuiData_PartialWrite(TMP_2ND_LOGO_RAWDATA_FILE, DEF_SFL_OSD_2ND_LOGO, (DEF_BITMAPAD_4K & 0x3fffffff), ucFocus);
 }

void palGui_Upgrade_Service_Second_Logo_Replace(UINT8 ucFocus)  //A35G2_Coda_0067
{
    halGui_OSD_Memory_Protect(FALSE);

	halGui_SecondLogoPaletteStore(TMP_2ND_LOGO_PALETTE_FILE, eCM_USER_LOGO_SERVICE_2D);
	palGui_GuiData_PartialWrite(TMP_2ND_LOGO_RAWDATA_FILE, DEF_SFL_OSD_SERVICE_2ND_LOGO, (DEF_BITMAPAD_4K & 0x3fffffff), ucFocus);
}
#endif

INT8 palGui_InitSemaphore(void)     //A70LV_Doulas_0014
{
//#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    //xGuiSemaphore = palImage_Scaler_xSemaphore_Get();
//#else
//    xGuiSemaphore = palImage_Scaler_xSemaphore_Get();
//#endif

//    if(xGuiSemaphore == NULL)
//        return ERROR_GUI_SEMAPHORE_FAIL;

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Init complete!\n", __FUNCTION__, __LINE__);
    return GUI_PASS;
}

INT8 palGui_PanelSet(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    halGui_PanelSet(ePanelTimingId);

    return GUI_PASS;
}

INT8 palGui_PanelChange(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    halGui_PanelChange(ePanelTimingId);

    return GUI_PASS;
}

INT8 palGui_OSD_MenuTransparencySet(UINT8 ucValue)         //A70LV_Doulas_0122
{
    halGui_OSD_MenuTransparencySet(ucValue);

    return GUI_PASS;
}

INT8 palGui_OSD_MenuTransparencyEnableSet(UINT8 ucEnable)         //A70LV_Doulas_0122
{
    halGui_OSD_MenuTransparencyEnableSet(ucEnable);

    return GUI_PASS;
}

INT8 palGui_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor) //A70LV_Larry_0137
{
    halGui_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, ucForeColor, ucBackColor);

    return GUI_PASS;
}

INT8 palGui_Paint_Line(COORDINATE sStart_Position, UINT8 width, INT16 iColor_Index)   //ZU860_Clare_0001, add
{
    COORDINATE sDes_position;
    BLOCK_SIZE_INFO sDes_size;
    INT8 iVal = 0;  //ZU860_Doulas_0136

    memcpy(&sDes_position, &sStart_Position, sizeof(COORDINATE));

    sDes_position.iX = sStart_Position.iX;
    sDes_position.iY = sStart_Position.iY;
    sDes_size.iWidth = width;
    sDes_size.iHeight = 1;

    iVal = halGui_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

    return iVal;
}

void palGui_ShowTest(void)
{
#if 0
    COORDINATE sStart_Position = {100, 100};
    BLOCK_SIZE_INFO sSize = {500, 500};
    UINT8 *pucString = {"ABCDEFGHIJKLMN"};
	UINT8 *pucString2 = {"OPQRSDGGKLAOEMV"};
    //INT16 iColor_Index,
    //INT16 iColor_Background,
    //INT16 iBoarder_Index

    if(palGui_SemaphoreTake(__FUNCTION__) == TRUE)
    {
        palGui_Paint_RectanglewithBoarder(sStart_Position, sSize, GUI_C_GREEN, GUI_C_RED, ebiHIGHLIGHT);

        sStart_Position.iX = 120;
        sStart_Position.iY = 120;

        sSize.iWidth  = 200;
        sSize.iHeight = 200;

        palGui_Paint_ASCIIString(pucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);

    	sStart_Position.iX = 120;
        sStart_Position.iY = 160;

        sSize.iWidth  = 200;
        sSize.iHeight = 200;

        palGui_Paint_ASCIIString(pucString2, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);


        sStart_Position.iX = 0;
        sStart_Position.iY = 0;

        sSize.iWidth  = 1920;
        sSize.iHeight = 1080;

        palGui_OSD_On(sStart_Position,sSize);

        palGui_SemaphoreGive(__FUNCTION__);
    }
#endif
}

void palGui_UpgradeAccess_Set(BOOL bEnable)	//A65_OPTOMA_Doulas_0126
{
    halGui_UpgradeAccess_Set(bEnable);
}

BOOL palGui_UpgradeAccess_Get(void)	//A65_OPTOMA_Doulas_0126
{
    return halGui_UpgradeAccess_Get();
}

//only for debug
void palGui_ShowOsdBitmapData(UINT32 ulStartVertPosition)	//A35G2_Simon_0114
{
    halGui_ShowOsdBitmapData(ulStartVertPosition);
}

