#include <string.h>

#include "halGui.h"
#include "cJSON.h"
#include "dvProAV_OSD.h" //A70LV_Larry_0001
#include "utilDbgMsg.h"
//#include "lwip_fs.h"
//#include "ff.h"
//#include "MemMap.h"
#include "halScaler.h"      //A70LV_Doulas_0014
#include "PNGUtility.h"

#include "dvProAV_Base.h"
#include "dvProAV_Scaler.h"
#include "dvProAV_SerialFlash.h"

#include "utilCRCAPI.h"
#include "ProcessMutexData.h"

//#define STANDARD_C_MALLOC
#define CHAR_LINE_BUF_MAX 16

/* Read a cJSON item/entity/structure to embedded. */
char JSON_Get(const cJSON *item, int iReadStatus);


//SemaphoreHandle_t            xGuiSemaphore;
//#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
//StaticSemaphore_t            xGuiMutexBuffer;
//#endif


//char *m_pcBitmapStruct = (char*)BITMAP_JSON_ADDR;
//char *m_pcTextStruct = (char*)TEXT_JSON_ADDR;
//char *m_pcOSDStruct = (char*)OSD_STRUCT_ADDR; //A70LV_Larry_0001


//	static int iBitmapTotalWidth = 0;
//	static int iBitmapTotalHeight = 0;
//static int iBitmapPaletteUsed = 768;
extern UINT8 aiBitmapPalette[768];


static INT16 iTotalLanguageUsed = 2;
static INT16 iActiveLanguageIndex = 0;
static INT16 *piLanguageUsed;

static int iTextStringCount = 0;
static int iTextStringReadCount = 0;


//UINT32 ulAllFontCount = 0;  //use to record how many font be load into system
UINT8 *mpucAllFontWidthInfo;  //use to record all load font's width information
//UINT32 ulAsciiFontCount = 0;  //use to record how many font be load into system
UINT8 *mucAsciiFontMaxHeight; //T100_Sander_0021

//UINT32 ulAllTextCount = 0;
static int iTextReadCount = 0;
TEXT_INFO *mspAllTextInfo;

//UINT32 ulAllBitmapCount = 0;
static int iBitmapReadCount = 0;
BITMAP_INFO *mspAllBitmapInfo;

//UINT32 ulAllMenuCount = 0;
static int iMenuReadCount = 0;
MENU_INFO *mspAllMenuInfo;

//UINT32 ulAllItemCount = 0;
static int iItemReadCount = 0;
ITEM_INFO *mspAllItemInfo;

#define SPLASH_SUPPORT_NUM 10
#define CTM_LOGO_INDEX 9


static UINT32 SPLASH_LOGO_ID[SPLASH_SUPPORT_NUM] = {0}; //T100_Sander_0013

static BOOL halGui_SemaphoreTake(const UINT8 ucCH, const char *pcSemaphore)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);
    BOOL bResult = dvProAV_SemaphoreTake(TRUE, pcSemaphore);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s) Semaphore %d \r\n", pcSemaphore, bResult);

    if(bResult != TRUE)
    {
        ASSERT(bResult == TRUE);
    }

    return bResult;
}


static BOOL halGui_SemaphoreGive(const UINT8 ucCH)
{
    //xSemaphoreGive(m_sHalScalerInfo[ucCH].xSemaphore);

    BOOL bResult = dvProAV_SemaphoreTake(FALSE, __FUNCTION__);

    LOG_MSG(db_HAL_SEMAPHORE, "(func:%s)\r\n", __FUNCTION__);

    if(bResult != TRUE)
    {
        ASSERT(bResult == TRUE);
    }

    return bResult;
}

#if 0
char JSON_Get_String(const cJSON *item)
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


char JSON_Get_Number(const cJSON *item)
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


char JSON_Get_Menu_Number(const cJSON *item)
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


char JSON_Get_Item_Number(const cJSON *item)
{
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
        if(item->valueint == CTM_LOGO_INDEX) //T100_Sander_0013
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

    }
    else
    {
        //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : %d with No define Name_String\n", __FUNCTION__, __LINE__, item->valueint);
        return (char)ERROR_CJSON_NO_DEFINE_NAMESTRING;
    }

    return CJSON_GET_PASS;
}


char JSON_Get_Bitmap_Number(const cJSON *item)
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


char JSON_Get_Text_Number(const cJSON *item)
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


char JSON_Get_TextString_Number(const cJSON *item)
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


char JSON_Get_ObjectArray(const cJSON *item, int iReadStatus)
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

    //travel all cJSON child item and call JSON_Get_Object to extract all OSD Menu/Item properties
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

        cRet = JSON_Get(child, iReadStatus);
        child = child->next;
    }

    //printf("JSON_Get_ObjectArray ret=%d\n", cRet);
    return cRet;
}


char JSON_Get_ItemBitmapArray(const cJSON *item)
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
    #ifdef STANDARD_C_MALLOC
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulBitmapID = (UINT32*)malloc(numentries*sizeof(UINT32));
    #else
    mspAllItemInfo[iItemReadCount-1].sItemDrawingInfo.pulBitmapID = (UINT32*)pvPortMalloc(numentries*sizeof(UINT32));
    #endif
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

char JSON_Get_ItemBrushArray(const cJSON *item) //T100_Sander_0002
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


	//Cassper_ProAV
static char JSON_Get_ItemRectangleIDArray(const cJSON *item)
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
	//Cassper_ProAV

char JSON_Get_ItemTextArray(const cJSON *item)
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


char JSON_Get_Palette_Array(const cJSON *item)
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


char JSON_Get_LanguageUsed_Array(const cJSON *item)
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
char JSON_Get_ASCIIFontMaxHeight_Array(const cJSON *item)
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


char JSON_Get_AllFontWidthInfo_Array(const cJSON *item)
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


char JSON_Get_StringFont_Array(const cJSON *item)
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


char JSON_Get_Object(const cJSON *item, int iReadStatus)
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

        cRet = JSON_Get(child, iReadStatus);
        child = child->next;

        //printf("JSON_Get_Object ret=%d\n", cRet);
    }

    return cRet;
}


char JSON_Get(const cJSON *item, int iReadStatus)
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
                    cRet = JSON_Get_Menu_Number(item);
                    break;

                case ITEM_READING:
                    cRet = JSON_Get_Item_Number(item);
                    break;

                case BITMAP_READING:
                    cRet = JSON_Get_Bitmap_Number(item);
                    break;

                case TEXT_READING:
                    cRet = JSON_Get_Text_Number(item);
                    break;

                case TEXT_STRING_READING:
                    cRet = JSON_Get_TextString_Number(item);
                    break;

                default:
                    cRet = JSON_Get_Number(item);
                    break;
            }
            break;

        case cJSON_String:
            cRet = JSON_Get_String(item);
            break;

        case cJSON_Array:
            if (!strcmp("MenuObjectArray",item->string))
            {
                iSpecificRead = MENU_READING;
                iMenuReadCount = 0;
                cRet = JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("ItemObjectArray",item->string))
            {
                iSpecificRead = ITEM_READING;
                iItemReadCount = 0;
                cRet = JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("ItemBitmapIDArray",item->string))
            {
                cRet = JSON_Get_ItemBitmapArray(item);
            }
	//Cassper_ProAV
            else if (!strcmp("ItemRectangleIDArray",item->string))
            {
                cRet = JSON_Get_ItemRectangleIDArray(item);
            }
	//Cassper_ProAV
            else if (!strcmp("ItemBrushIDArray",item->string))  //T100_Sander_0002
            {
                cRet = JSON_Get_ItemBrushArray(item);
            }
            else if (!strcmp("ItemTextIDArray",item->string))
            {
                cRet = JSON_Get_ItemTextArray(item);
            }
            else if (!strcmp("BitmapObjectArray",item->string))
            {
                iSpecificRead = BITMAP_READING;
                iBitmapReadCount = 0;
                cRet = JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("BitmapPaletteArray",item->string))
            {
                cRet = JSON_Get_Palette_Array(item);
            }
            else if (!strcmp("TextObjectArray",item->string))
            {
                iSpecificRead = TEXT_READING;
                iTextReadCount = 0;
                cRet = JSON_Get_ObjectArray(item, TEXT_READING);
            }
            else if (!strcmp("StringObjectArray",item->string))
            {
                iSpecificRead = TEXT_STRING_READING;
                iTextStringReadCount = 0;
                cRet = JSON_Get_ObjectArray(item, iSpecificRead);
            }
            else if (!strcmp("ASCIIFontMaxHeightArray", item->string)) //T100_Sander_0021
            {
                cRet = JSON_Get_ASCIIFontMaxHeight_Array(item);
            }
            else if (!strcmp("LanguageUsedArray",item->string))
            {
                cRet = JSON_Get_LanguageUsed_Array(item);
            }
            else if (!strcmp("AllFontWidthInformationArray",item->string))
            {
                cRet = JSON_Get_AllFontWidthInfo_Array(item);
            }
            else if (!strcmp("FontArray",item->string))
            {
                cRet = JSON_Get_StringFont_Array(item);
            }
            else
            {
                //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)ERROR : Read array with no define Name_String\n", __FUNCTION__, __LINE__);
                return (char)ERROR_CJSON_GET_FAIL;
            }
            break;

        case cJSON_Object:
            cRet = JSON_Get_Object(item, iReadStatus);
            break;
    }

    return cRet;
}



/*
Parse JSON string then convert that into to Gui object
 */
void JSON_Convertion(char *text)
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
        JSON_Get(json, NO_SPECIFIC_READING);
        cJSON_Delete(json);
        #endif
    }
}


/* Read a JSON file, parse it*/
INT8 JSON_FileOpen(char *filename)
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
    fclose(pFile);

    pFile = fopen(filename, "rb");

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) size %d\r\n", __FUNCTION__, __LINE__, size);

    cbuf = (char *) pvPortMalloc(size);

    memset(cbuf, '\0', size);

    fread(cbuf, 1, size, pFile);

    fclose(pFile);

    JSON_Convertion(cbuf);
    vPortFree(cbuf);

    return CJSON_GET_PASS;
}



//function define to get CJson result


INT8 Bitmap_Get(UINT32 ulBitmapID, OSD_BITMAP *psRetBitmap)
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
        *(pucRet_char_width+uiCount) = *(mpucAllFontWidthInfo + ulFontOffset + *(puiChar_index+uiCount)) + 1;
        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Copy %d-th width %d to %d\n", __FUNCTION__, __LINE__, (uiCount+1), *(mpucAllFontWidthInfo + ulFontOffset + *(puiChar_index+uiCount)), *(pucRet_char_width+uiCount));
    }

    return GUI_PASS;
}

INT8 String_Get(UINT32 ulTextID, OSD_STRING * psRetString, UINT16 *puiStringWidth)
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
                //*puiStringWidth = *puiStringWidth + psRetString->uiStringLength;
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
#endif

UINT16 halGui_MaxStringW_Get(UINT32 ulTextID) //A70LK_Jacky_0053
{
#if 0
    INT16 iCount;
    STRING_INFO *psString;
    //searching
    for (iCount=0; iCount<ulAllTextCount; iCount++)
    {
        //if TextID match
        if (ulTextID == mspAllTextInfo[iCount].ulTextID)
        {
            psString = (mspAllTextInfo[iCount].psString + iActiveLanguageIndex);
            return psString->uiStringWidth + psString->uiStringLength;
        }
    }
#endif
    return 0;
}

#if 0
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

#endif

// ---------------------------------------------------------------------------
// color definition data strucutre
// ---------------------------------------------------------------------------

//typedef const RGB *PCRGB;

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


INT8 Gui_Data_DMA_To_Flash(UINT8 *pucDataBuf, UINT32 ulDataSize, UINT32 ulSflAd)
{
    return GUI_PASS;
}

INT8 Gui_Paint_SubString( OSD_STRING sIchipString, START_POINT sDes_position, INT16 iTextColor, INT16 iStringOffset)
{
    INT8 cResult = 1;

    INT16 iCount = 0;
    UINT16 uiCharWdith = 0;
    START_POINT sSubDes_position = sDes_position;
    OSD_STRING sSubString = sIchipString;

    for(iCount = 0; iCount < sIchipString.uiStringLength; iCount = iCount + CHAR_LINE_BUF_MAX)
    {
        sSubString.uiStringLength = ((sIchipString.uiStringLength - iCount) > CHAR_LINE_BUF_MAX) ? CHAR_LINE_BUF_MAX : (sIchipString.uiStringLength - iCount);

        //dvProAV_Inhibit_Color_Set(TRUE, GUI_INHITBIT_COLOR);
        cResult &= dvProAV_Paint_String(sSubDes_position, sSubString, iTextColor, GUI_INHITBIT_COLOR, iCount);
        //dvProAV_Inhibit_Color_Set(FALSE, GUI_INHITBIT_COLOR);

        if((sIchipString.uiStringLength - iCount) > CHAR_LINE_BUF_MAX)
        {
            for(uiCharWdith = 0; uiCharWdith < sSubString.uiStringLength ; uiCharWdith++)
            {
                sSubDes_position.iX = sSubDes_position.iX + sIchipString.pucCharWdith[uiCharWdith + iCount];
            }
        }
    }

    return cResult;
}

#if 0
INT8 halGui_Init(void)
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
    if (CJSON_GET_PASS != JSON_FileOpen(BITMAP_STRUCT_JASON))
    {
        if(CJSON_GET_PASS != JSON_FileOpen(CUSTOM_BITMAP_STRUCT_JASON))
        {
            return ERROR_CJSON_FILE_OPEN_FAIL;
        }
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) OSDStruct.json open\r\n", __FUNCTION__, __LINE__);
    if (CJSON_GET_PASS != JSON_FileOpen(OSD_STRUCT_JASON))
    {
       if(CJSON_GET_PASS != JSON_FileOpen(CUSTOM_OSD_STRUCT_JASON))
       {
           return ERROR_CJSON_FILE_OPEN_FAIL;
       }
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) TextStruct.json open\r\n", __FUNCTION__, __LINE__);
    if (CJSON_GET_PASS != JSON_FileOpen(TEXT_STRUCT_JASON))
    {
        if(CJSON_GET_PASS != JSON_FileOpen(CUSTOM_TEXT_STRUCT_JASON))
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
#endif


INT8 halGui_HWInit(void)
{
    if(halGui_InitSemaphore() != GUI_PASS)      //A70LV_Doulas_0014
        return ERROR_GUI_SEMAPHORE_FAIL;

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_OSD_Init(256, aiBitmapPalette, GUI_INHITBIT_COLOR);  //need to get amount of palette color and palette data point

        halGui_SemaphoreGive(0);
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) complete!\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}

INT8 halGui_LogoReplacementInit(void)
{
    if(halGui_InitSemaphore() != GUI_PASS)      //A70LV_Doulas_0014
        return ERROR_GUI_SEMAPHORE_FAIL;

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {

#ifdef ENABLE_LOGO_REPLACEMENT
        dvProAV_OSD_LogoDefaultReplacement();
#endif /* ENABLE_LOGO_REPLACEMENT */

        halGui_SemaphoreGive(0);
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) complete!\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}

INT8 halGui_OSDReload(void)
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_OSD_ReLoad();

        halGui_SemaphoreGive(0);
    }
    LOG_MSG(db_ALWAYS, "halGui_OSDReload\n");

    return GUI_PASS;
}

INT8 halGui_OSD_Copy(void) //A70LV_Larry_0067
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {

        OSD_CurrentCopy();

        halGui_SemaphoreGive(0);
    }

    return GUI_PASS;
}

INT8 halGui_OSD_On(COORDINATE sStart_Position, BLOCK_SIZE_INFO sDraw_Size)
{
    START_POINT sDes_Position;
    RECT_SIZE sDes_Size;
    OSD_TRANSPARENCY_COLOR sTra_Color;

    //sDes_Position.iX = sStart_Position.iX;
    //sDes_Position.iY = sStart_Position.iY;
    memcpy(&sDes_Position, &sStart_Position, sizeof(START_POINT));
    //sDes_Size.iWidth = sDraw_Size.iWidth;
    //sDes_Size.iHeight = sDraw_Size.iHeight;
    memcpy(&sDes_Size, &sDraw_Size, sizeof(RECT_SIZE));
    sTra_Color.ucT_Color1 = GUI_INHITBIT_COLOR;//GUI_TRANSPARENT_COLOR_1;
    sTra_Color.ucT_Color2 = GUI_INHITBIT_COLOR;
    sTra_Color.ucT_Color3 = GUI_INHITBIT_COLOR;
    sTra_Color.ucT_Color4 = GUI_INHITBIT_COLOR;
    //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)(%d,%d)(%d,%d)\n", __FUNCTION__, __LINE__,sStart_Position.iX,sStart_Position.iY, sDraw_Size.iWidth ,sDraw_Size.iHeight);    //A70LV_Doulas_0068 test
    LOG_MSG(db_HAL_PROAV_GUI, "halGui_OSD_On (func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_OSD_On(sDes_Position, sDes_Size, GUI_ON, sTra_Color);  //disable transparency color for  first stage

        halGui_SemaphoreGive(0);
    }
    return GUI_PASS;
}


INT8 halGui_OSD_Off(void)
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
#if 1 //A70LV_Larry_0001
        dvProAV_OSD_Enable(false);
#else
        dvC734_OSD_Off();
#endif /* 1 */

        halGui_SemaphoreGive(0);
    }
    return GUI_PASS;
}


INT8 halGui_OSD_Set_InhitbitColor( INT8 cEnable, INT16 iInhibit_Color )
{
    LOG_MSG(db_HAL_PROAV_GUI, "OSD_Set_InhitbitColor (func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_Inhibit_Color_Set(cEnable, iInhibit_Color);

        halGui_SemaphoreGive(0);
    }

    return GUI_PASS;
}


INT8 halGui_Language_Set(UINT8 ucLanguage)
{
    if ((int)ucLanguage < iTotalLanguageUsed)
    {
        iActiveLanguageIndex = (int)ucLanguage;
        return GUI_PASS;
    }

    return ERROR_GUI_NOT_FOUND;
}

MENU_INFO* halGui_MenuStructPoint_Get(void)
{
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) Get All Menu struct point\r\n", __FUNCTION__, __LINE__);
    return mspAllMenuInfo;
}

#if 0
UINT32 halGui_MenuCount_Get(void)
{
    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) Get All menu count\r\n", __FUNCTION__, __LINE__);
    return ulAllMenuCount;
}
#endif

INT8 halGui_Paint_Bitmap(UINT32 ulBitmapID, COORDINATE sStart_Position)
{
#if 0
    //BITMAP_INFO sRetBitmap;
    OSD_BITMAP sBitmap;
    if( GUI_FOUND == Bitmap_Get(ulBitmapID, &sBitmap) )
    {
        START_POINT sDes_position;
        memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
        //sBitmap.sSrc_Position.iX = sRetBitmap.sSrc_Coordinate.iX;
        //sBitmap.sSrc_Position.iY = sRetBitmap.sSrc_Coordinate.iY;
        //sBitmap.sBitmap_Size.iWidth = sRetBitmap.sBitmap_Size.iWidth;
        //sBitmap.sBitmap_Size.iHeight = sRetBitmap.sBitmap_Size.iHeight;

        if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
        {
            //dvProAV_Inhibit_Color_Set(TRUE, GUI_INHITBIT_COLOR);
            dvProAV_Paint_Bitmap(sBitmap, sDes_position);  //need to design return type from driver to Application at here
            //dvProAV_Inhibit_Color_Set(FALSE, GUI_INHITBIT_COLOR);

            halGui_SemaphoreGive(0);
        }
        return GUI_FOUND;
    }
#endif
    return ERROR_GUI_NOT_FOUND;
}

//T100_Sander_0013
INT8 halGui_Paint_SPLASH(UINT8 ulBitmapIDIndex, COORDINATE sStart_Position)
{
    LOG_MSG(db_HAL_PROAV_GUI, "halGui_Paint_SPLASH (func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    //halGui_SemaphoreTake(0, 400);
    halGui_Paint_Bitmap(*(SPLASH_LOGO_ID + ulBitmapIDIndex), sStart_Position);
    //halGui_SemaphoreGive(0);

    return GUI_FOUND;
}

	//Cassper_ProAV
//ZU860_Clare_0019, >>>
INT8 halGui_Paint_TextHightLight(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)      // for OPTOMA IP/PIN Text
{
    START_POINT sDes_position;
    RECT_SIZE sDes_size;
    INT8 iVal = 0;  //ZU860_Doulas_0136

	if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
	{
		sDes_position.iX = sStart_Position.iX;
		sDes_position.iY = sStart_Position.iY;
		sDes_size.iWidth = sSize.iWidth;
		sDes_size.iHeight = sSize.iHeight + 1;
#if 0//def CUSTOM_OPTOMA	//ZU860_Clare_0093
		dvC821_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR); 	// EK816U_626U_Energy_0019
#endif
		sDes_size.iHeight = sSize.iHeight;
		//iVal = dvC821_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);
		halGui_SemaphoreGive(0);          //ZU860_Doulas_0136
	}
    return iVal;
}
	//Cassper_ProAV

//T100_Sander_0006
INT8 halGui_Paint_Rectangle(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index)
{
    INT8 cResult = 0;
    START_POINT sDes_position;
    RECT_SIZE sDes_size;

    memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
    memcpy(&sDes_size, &sSize, sizeof(RECT_SIZE));

    LOG_MSG(db_HAL_PROAV_GUI, "halGui_Paint_Rectangle (func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR);

    	sDes_position.iX = sStart_Position.iX + 1;
    	sDes_position.iY = sStart_Position.iY + 1;
    	sDes_size.iWidth = sSize.iWidth - 1;
    	sDes_size.iHeight = sSize.iHeight - 1;

        dvProAV_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR_BACKGROUND);

    	sDes_position.iX = sStart_Position.iX + 1;
    	sDes_position.iY = sStart_Position.iY + 1;
    	sDes_size.iWidth = sSize.iWidth - 2;
    	sDes_size.iHeight = sSize.iHeight - 2;

        cResult = dvProAV_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        halGui_SemaphoreGive(0);
    }

    return cResult;
}

INT8 halGui_Paint_RectanglewithBoarder(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index, INT16 iColor_Background, INT16 iBoarder_Index)  //T100_Sander_0002
{
    INT8 cResult = 0;
    START_POINT sDes_position;
    RECT_SIZE sDes_size;

    memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
    memcpy(&sDes_size, &sSize, sizeof(RECT_SIZE));

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        if(iBoarder_Index != ebiNON)
        {
            dvProAV_Paint_Rectangle(sDes_position, sDes_size, iColor_Background);

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

                dvProAV_Paint_Rectangle(sDes_position, sDes_size, GUI_C_SLIDER_BAR);

    			sDes_size.iWidth = sSize.iWidth - 2;
    			sDes_size.iHeight = sSize.iHeight - 2;
    		}
        }

        cResult = dvProAV_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);

        halGui_SemaphoreGive(0);
    }

    return cResult;

}


INT8 halGui_Paint_RectangFrame(COORDINATE sStart_Position, BLOCK_SIZE_INFO sSize, INT16 iColor_Index, INT16 iColor_Background) //A70LV_Larry_0378
{
    INT8 cResult = 0;
    START_POINT sDes_position;
    RECT_SIZE   sDes_size;
    UINT16      uiLayoutWide = 2;

    memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
    memcpy(&sDes_size, &sSize, sizeof(RECT_SIZE));

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_Paint_Rectangle(sDes_position, sDes_size, iColor_Background);

        if((uiLayoutWide != 0) && (iColor_Background != iColor_Index))
        {
            if(((sSize.iWidth - uiLayoutWide*2) > 0) &&
               ((sSize.iHeight - uiLayoutWide*2) > 0))
            {
                sDes_position.iX = sStart_Position.iX + uiLayoutWide;
                sDes_position.iY = sStart_Position.iY + uiLayoutWide;
                sDes_size.iWidth = sSize.iWidth - uiLayoutWide*2;
                sDes_size.iHeight = sSize.iHeight - uiLayoutWide*2;
            }
            else
            {
                halGui_SemaphoreGive(0);
                return cResult;
            }

            dvProAV_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);
        }

        halGui_SemaphoreGive(0);
    }

    return cResult;

}


INT8 halGui_Paint_Text( UINT32 ulTextID, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment )
{
#if 0
    //STRING_INFO sString;
    OSD_STRING sIchipString;
    UINT16 uiStringWidth;

    LOG_MSG(db_HAL_PROAV_GUI, "halGui_Paint_Text (func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Paint TextID=%d, start_x=%d, BoxWidth=%d\n", __FUNCTION__, __LINE__, ulTextID, sStart_Position.iX, sTextBoxSize.iWidth);
    if(ulTextID == 0) //Tahoma,12,75,0 //A70LV_Larry_0211
    {
        return GUI_PASS;
    }

    if( GUI_FOUND == String_Get(ulTextID, &sIchipString, &uiStringWidth) )
    {
        START_POINT sDes_position;

		if(uiStringWidth > sTextBoxSize.iWidth) //A70LV_Larry_0267
		{
			LOG_MSG(db_HAL_GUI_CK, "[%03d]Over String Box width %d string width %d\n", ulTextID, sTextBoxSize.iWidth, uiStringWidth);

			while(uiStringWidth > sTextBoxSize.iWidth)
			{
				sIchipString.uiStringLength -= 1;
				if(uiStringWidth - sIchipString.pucCharWdith[sIchipString.uiStringLength])
				{
					uiStringWidth -= sIchipString.pucCharWdith[sIchipString.uiStringLength];
				}
			}

			//LOG_MSG(db_ALWAYS, "reduce uiStringWidth %d\n", uiStringWidth);
		}
        //ToDo eAlignment : adjust string start position according to TextBoxSize and string width
        switch (eAlignment)
        {
            case eoaLEFT:
                memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
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
                memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
            break;
        }

        //LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Des_(x,y)=(%d,%d) str_width=%d \n", __FUNCTION__, __LINE__, sDes_position.iX, sDes_position.iY, uiStringWidth);

        if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
        {
            Gui_Paint_SubString(sIchipString, sDes_position, iTextColor, 0);
            halGui_SemaphoreGive(0);
        }

        //free that all malloc memory size in this function
        vPortFree(sIchipString.puiCharIndex);
        vPortFree(sIchipString.pucCharWdith);

    }
    else
    {
        LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)ERROR : TextID=%d not found!\n", __FUNCTION__, __LINE__, ulTextID);
        return ERROR_GUI_NOT_FOUND;
    }
#endif /* 0 */
    return GUI_PASS;
}


INT8 halGui_Paint_ASCIIString( UINT8 *pucString, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment )
{
#if 0
    INT16 iCount;
    UINT16 uiStringWidth;
    OSD_STRING sIchipString;
    START_POINT sDes_position;

    sIchipString.ulFontOffset = GUI_STRING_ASCII_FONT_OFFSET;
    sIchipString.ucStringMaxHeight = mucAsciiFontMaxHeight[iActiveLanguageIndex + 1]; //T100_Sander_0021
    sIchipString.uiStringLength = strlen((char *)pucString);  //need to implemnet recursive function to paint string if string length over 32

    if(pucString[(sIchipString.uiStringLength - 1)] == '\n' || pucString[(sIchipString.uiStringLength - 1)] == '\r')
    {
        sIchipString.uiStringLength = sIchipString.uiStringLength - 1;
    }

    if (sIchipString.uiStringLength < 1)
        return ERROR_GUI_EMPTY_STRING;

    sIchipString.puiCharIndex = (UINT16 *)pvPortMalloc(sIchipString.uiStringLength*2);
    for (iCount=0; iCount<sIchipString.uiStringLength; iCount++)
    {
        *(sIchipString.puiCharIndex+iCount) = (UINT16)*(pucString+iCount);
    }

    sIchipString.pucCharWdith = (UINT8*)pvPortMalloc(sIchipString.uiStringLength);
    String_FillWidthInfo( sIchipString.uiStringLength, sIchipString.ulFontOffset, sIchipString.puiCharIndex, sIchipString.pucCharWdith);

    uiStringWidth = 0;  //calculate ASCII string width
    for (iCount=0; iCount<sIchipString.uiStringLength; iCount++)
    {
        uiStringWidth = uiStringWidth + *(sIchipString.pucCharWdith + iCount);
    }
    uiStringWidth = uiStringWidth + sIchipString.uiStringLength; //add one pixel for each character for C734 OSD drawing

    switch (eAlignment)
    {
        case eoaLEFT:
            memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
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
            memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));
            break;
    }

    LOG_MSG(db_HAL_PROAV_GUI, "halGui_Paint_Text (func:%s, line:%d)\n", __FUNCTION__, __LINE__);

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        Gui_Paint_SubString(sIchipString, sDes_position, iTextColor, 0);  //String background color default set to inhibit color for first test version
        halGui_SemaphoreGive(0);
    }

    vPortFree(sIchipString.puiCharIndex);
    vPortFree(sIchipString.pucCharWdith);

    //free that all malloc memory size in this function
#endif /* 0 */
    return GUI_PASS;
}


INT8 halGui_Paint_Number(INT32 iNumber, eNUMERIC_SIGN eNumMode, COORDINATE sStart_Position, BLOCK_SIZE_INFO sTextBoxSize, INT16 iTextColor, eOBJECT_ALIGN eAlignment)
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

    halGui_Paint_ASCIIString(aucVal_Str, sStart_Position, sTextBoxSize, iTextColor, eAlignment);

    return GUI_PASS;
}

INT8 halGui_Paint_TestPattern(COORDINATE sDrawStartPosition, BLOCK_SIZE_INFO sDraw_Size, UINT16 uiIndex)
{
#if 0
	COORDINATE sDrawStartPosition2;
	BLOCK_SIZE_INFO sDraw_Size2;
	UINT8 ucX = 0;
	UINT8 ucY = 0;
	UINT8 ucVal;

    switch(uiIndex)
	{
		case eCM_TEST_PATTERN_GRID:
			sDrawStartPosition.iX = 0;
			sDrawStartPosition.iY = 0;
			halGui_Paint_SPLASH(7, sDrawStartPosition);
			break;

		case eCM_TEST_PATTERN_GREEN_GRID:    //ZU860_Doulas_0094 Modify//ZU860_Doulas_0031
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
			ucVal = sDraw_Size.iWidth / 96;
			sDraw_Size2.iWidth  = 1;
			sDraw_Size2.iHeight = sDraw_Size.iHeight;
			for(; ucX < ucVal ; ucX++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX + ucX * 96;
				sDrawStartPosition2.iY = sDrawStartPosition.iY ;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_GREEN, GUI_C_GREEN, ebiNON);
			}
			sDrawStartPosition2.iX = sDrawStartPosition.iX + sDraw_Size.iWidth - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_GREEN, GUI_C_GREEN, ebiNON);

			ucVal = sDraw_Size.iHeight / 60;
			sDraw_Size2.iWidth  = sDraw_Size.iWidth;
			sDraw_Size2.iHeight = 1;
			for(; ucY < ucVal ; ucY++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX;
				sDrawStartPosition2.iY = sDrawStartPosition.iY + ucY * 60;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_GREEN, GUI_C_GREEN, ebiNON);
			}
			sDrawStartPosition2.iY = sDrawStartPosition.iY + sDraw_Size.iHeight - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_GREEN, GUI_C_GREEN, ebiNON);
			break;

		case eCM_TEST_PATTERN_MAGENTA_GRID:    //ZU860_Doulas_0094 Modify//ZU860_Doulas_0031
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
			ucVal = sDraw_Size.iWidth / 96;
			sDraw_Size2.iWidth  = 1;
			sDraw_Size2.iHeight = sDraw_Size.iHeight;
			for(; ucX < ucVal ; ucX++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX + ucX * 96;
				sDrawStartPosition2.iY = sDrawStartPosition.iY ;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_MAGENTA, GUI_C_MAGENTA, ebiNON);
			}
			sDrawStartPosition2.iX = sDrawStartPosition.iX + sDraw_Size.iWidth - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_MAGENTA, GUI_C_MAGENTA, ebiNON);

			ucVal = sDraw_Size.iHeight / 60;
			sDraw_Size2.iWidth  = sDraw_Size.iWidth;
			sDraw_Size2.iHeight = 1;
			for(; ucY < ucVal ; ucY++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX;
				sDrawStartPosition2.iY = sDrawStartPosition.iY + ucY * 60;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_MAGENTA, GUI_C_MAGENTA, ebiNON);
			}
			sDrawStartPosition2.iY = sDrawStartPosition.iY + sDraw_Size.iHeight - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_MAGENTA, GUI_C_MAGENTA, ebiNON);
			break;

		case eCM_TEST_PATTERN_WHITE_GRID:    //ZU860_Doulas_0094 Modify//ZU860_Doulas_0031
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
			ucVal = sDraw_Size.iWidth / 96;
			sDraw_Size2.iWidth  = 1;
			sDraw_Size2.iHeight = sDraw_Size.iHeight;
			for(; ucX < ucVal ; ucX++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX + ucX * 96;
				sDrawStartPosition2.iY = sDrawStartPosition.iY ;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			}
			sDrawStartPosition2.iX = sDrawStartPosition.iX + sDraw_Size.iWidth - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);

			ucVal = sDraw_Size.iHeight / 60;
			sDraw_Size2.iWidth  = sDraw_Size.iWidth;
			sDraw_Size2.iHeight = 1;
			for(; ucY < ucVal ; ucY++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX;
				sDrawStartPosition2.iY = sDrawStartPosition.iY + ucY * 60;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			}
			sDrawStartPosition2.iY = sDrawStartPosition.iY + sDraw_Size.iHeight - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			break;

		case eCM_TEST_PATTERN_WHITE:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			break;

		case eCM_TEST_PATTERN_BLACK:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
			break;

		case eCM_TEST_PATTERN_CHECKERBOARD:
			ucVal = 4;
			sDraw_Size2.iWidth  = sDraw_Size.iWidth / ucVal;
			sDraw_Size2.iHeight = sDraw_Size.iHeight / ucVal;
			sDrawStartPosition2.iX = sDrawStartPosition.iX;
			sDrawStartPosition2.iY = sDrawStartPosition.iY;
			for(; ucY < ucVal ; ucY++)
			{
				for(; ucX < ucVal ; ucX++)
				{
					sDrawStartPosition2.iX = sDrawStartPosition.iX + sDraw_Size2.iWidth * ucX;
					sDrawStartPosition2.iY = sDrawStartPosition.iY + sDraw_Size2.iHeight * ucY;
					if(((ucX + ucY) % 2) == 0)
						halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
					else
						halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
				}
				ucX = 0;
			}
			break;

		case eCM_TEST_PATTERN_COLORBAR:
			sDraw_Size2.iWidth  = sDraw_Size.iWidth / 8;
			sDraw_Size2.iHeight = sDraw_Size.iHeight;
			sDrawStartPosition2.iX = sDrawStartPosition.iX;
			sDrawStartPosition2.iY = sDrawStartPosition.iY;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_YELLOW, GUI_C_YELLOW, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_CYAN, GUI_C_CYAN, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_GREEN, GUI_C_GREEN, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_MAGENTA, GUI_C_MAGENTA, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_RED, GUI_C_RED, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_BLUE, GUI_C_BLUE, ebiNON);
			sDrawStartPosition2.iX += sDraw_Size2.iWidth;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
			break;

		case eCM_TEST_PATTERN_RED:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_RED, GUI_C_RED, ebiNON);
			break;

		case eCM_TEST_PATTERN_GREEN:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_GREEN, GUI_C_GREEN, ebiNON);
			break;

		case eCM_TEST_PATTERN_BLUE:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLUE, GUI_C_BLUE, ebiNON);
			break;

		case eCM_TEST_PATTERN_YELLOW:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_YELLOW, GUI_C_YELLOW, ebiNON);
			break;

		case eCM_TEST_PATTERN_MAGENTA:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_MAGENTA, GUI_C_MAGENTA, ebiNON);
			break;

		case eCM_TEST_PATTERN_CYAN:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_CYAN, GUI_C_CYAN, ebiNON);
			break;

		case eCM_TEST_PATTERN_BORESIGHT:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLACK, GUI_C_BLACK, ebiNON);
			sDrawStartPosition.iX = 9;
			sDrawStartPosition.iY = 9;
			halGui_Paint_SPLASH(3, sDrawStartPosition);
			sDrawStartPosition.iX = 1894;
			sDrawStartPosition.iY = 9;
			halGui_Paint_SPLASH(3, sDrawStartPosition);
			sDrawStartPosition.iX = 11;
			sDrawStartPosition.iY = 1052;
			halGui_Paint_SPLASH(3, sDrawStartPosition);
			sDrawStartPosition.iX = 1892;
			sDrawStartPosition.iY = 1052;
			halGui_Paint_SPLASH(3, sDrawStartPosition);
			sDrawStartPosition.iX = 952;
			sDrawStartPosition.iY = 529;
			halGui_Paint_SPLASH(3, sDrawStartPosition);

			sDrawStartPosition.iX = 345;
			sDrawStartPosition.iY = 0;
			halGui_Paint_SPLASH(4, sDrawStartPosition);

			sDrawStartPosition.iX = 1475;
			sDrawStartPosition.iY = 0;
			halGui_Paint_SPLASH(5, sDrawStartPosition);

			sDrawStartPosition.iX = 910;
			sDrawStartPosition.iY = 980;
			halGui_Paint_SPLASH(6, sDrawStartPosition);
			break;

		case eCM_TEST_PATTERN_FULL_SCREEN:
		{
			OSD_BITMAP sRetBitmap;

			if(GUI_FOUND == Bitmap_Get(*(SPLASH_LOGO_ID + 1), &sRetBitmap) )
			{
				INT16 iX = 0;
				INT16 iY = 0;

				halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_WHITE, GUI_C_WHITE, ebiNON);

				for(iX = sDrawStartPosition.iX; iX < sDraw_Size.iWidth; iX = iX+sRetBitmap.sBitmap_Size.iWidth)
				{
					for(iY = sDrawStartPosition.iY; iY < sDraw_Size.iHeight; iY = iY+sRetBitmap.sBitmap_Size.iHeight)
					{
						sDrawStartPosition2.iX = iX;
						sDrawStartPosition2.iY = iY;
						halGui_Paint_SPLASH(1, sDrawStartPosition2);
					}
				}
			}
		}
		break;


		case eCM_TEST_PATTERN_GRAY:	//T100_Simon_0028
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_GRAY, GUI_C_GRAY, ebiNON);
			break;

        case eCM_TEST_PATTERN_WHITE_GRID_4CORNER:    //ZU860_Doulas_0094
			//halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_BLACK, GUI_C_BLACK, ebiNON);      //ZU860_Doulas_0124 remove boader
			ucVal = sDraw_Size.iWidth / 96;
			sDraw_Size2.iWidth  = 1;
			sDraw_Size2.iHeight = sDraw_Size.iHeight;
			for(; ucX < ucVal ; ucX++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX + ucX * 96;
				sDrawStartPosition2.iY = sDrawStartPosition.iY ;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			}
			sDrawStartPosition2.iX = sDrawStartPosition.iX + sDraw_Size.iWidth - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);

			ucVal = sDraw_Size.iHeight / 60;
			sDraw_Size2.iWidth  = sDraw_Size.iWidth;
			sDraw_Size2.iHeight = 1;
			for(; ucY < ucVal ; ucY++)
			{
				sDrawStartPosition2.iX = sDrawStartPosition.iX;
				sDrawStartPosition2.iY = sDrawStartPosition.iY + ucY * 60;
				halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			}
			sDrawStartPosition2.iY = sDrawStartPosition.iY + sDraw_Size.iHeight - 1;
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);


            ucVal = sDraw_Size.iWidth / 96;
			sDraw_Size2.iWidth  = 1;
			sDraw_Size2.iHeight = 1;
			for(ucX = 0; ucX < ucVal ; ucX++)
            {
                for(ucY = 0; ucY < sDraw_Size.iHeight / 60 ; ucY++)
                {

                    sDrawStartPosition2.iX = sDrawStartPosition.iX + ucX * 96 + 96/2;
				    sDrawStartPosition2.iY = sDrawStartPosition.iY + ucY * 60 + 60/2;
				    halGui_Paint_RectanglewithBoarder(sDrawStartPosition2, sDraw_Size2, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
                }
            }
			break;

		default:
			halGui_Paint_RectanglewithBoarder(sDrawStartPosition, sDraw_Size, GUI_C_WHITE, GUI_C_WHITE, ebiNON);
			break;
	}
#endif /* 0 */
    return GUI_PASS;
}

INT8 halGui_GuiData_EraseAll(void) //A70LV_Larry_0004
{
    //halGui_SemaphoreTake(0, 411);

    //halGui_SemaphoreGive(0);

    //LOG_MSG(db_DV_SCALER, "(func:%s, line:%d)Erase spi-flash complete!\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}

INT8 halGui_GuiData_FlashEraseSector(UINT32 ulAddr, UINT32 ulSize) //A70LV_Larry_0004
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_SfiEraseSector(ulAddr, ulSize);

        halGui_SemaphoreGive(0);
    }


    return GUI_PASS;
}

INT8 halGui_GuiData_FlashWrite(UINT32 ulDRAM_Addr, UINT32 ulFlash_Addr, UINT32 ulSize, UINT8* pcData)
{
    int status = rcSUCCESS;

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_DramBankSet(0);

        dvProAV_SfiDramAccess(ulDRAM_Addr, pcData, ulSize, IO_WRITE);

        dvProAV_SfiDram2Flash(ulDRAM_Addr, ulFlash_Addr, ulSize, IO_WRITE);

        halGui_SemaphoreGive(0);
    }

    return GUI_PASS;
}

INT8 halGui_GuiData_FlashRead(UINT32 ulDRAM_Addr, UINT32 ulSflAd, UINT32 ulSize, UINT8* pcData)
{
#ifdef FPGA_ENTRY_4K
    UINT32  ulBlock = 0;
    UINT32  ulCount = 0;

    ulBlock = (ulSize%0x8000 == 0) ? (ulSize/0x8000):(ulSize/0x8000) + 1;

    UINT8   *pBuf = NULL;
    pBuf = (UINT8 *)malloc(ulBlock*0x8000);

    if(pBuf == NULL)
    {
        return GUI_FAIL;
    }

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_DramBankSet(0);
        dvProAV_SfiDram2Flash(ulDRAM_Addr, ulSflAd, ulSize, IO_READ);

        for(ulCount = 0; ulCount < ulBlock; ulCount++)
        {
            dvProAV_SfiDramAccess(ulDRAM_Addr, &pBuf[ulCount*0x8000], 0x8000, IO_READ);

            ulDRAM_Addr += 0x8000;

            halGui_SemaphoreGive(FALSE);
        }


        halGui_SemaphoreGive(FALSE);
    }

#else
    UINT32  ulBlock = 0;
    UINT32  ulCount = 0;

    ulBlock = (ulSize%0x4000 == 0) ? (ulSize/0x4000):((ulSize/0x4000) + 1);

    UINT8   *pBuf = NULL;
    pBuf = (UINT8 *)malloc(ulBlock*0x4000);

    if(pBuf == NULL)
    {
        return GUI_FAIL;
    }

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_DramBankSet(0);
        dvProAV_SfiDram2Flash(ulDRAM_Addr, ulSflAd, ulSize, IO_READ);

        for(ulCount = 0; ulCount < ulBlock; ulCount++)
        {
            dvProAV_SfiDramAccess(ulDRAM_Addr, &pBuf[ulCount*0x4000], 0x4000, IO_READ);

            ulDRAM_Addr += 0x4000;

            halGui_SemaphoreGive(FALSE);
        }


        halGui_SemaphoreGive(FALSE);
    }

#endif /* FPGA_ENTRY_4K */

    LOG_MSG(db_DV_PROAV_OSD, "Read Add 0x%08x 0x%08x-0x%08x\n", ulDRAM_Addr, ulSflAd, ulSflAd + ulSize);

    memcpy(pcData, pBuf, ulSize);

    free(pBuf);

    return GUI_PASS;

}

UINT32 halGui_GuiData_FlashRead_CheckSum(UINT32 ulDRAM_Addr, UINT32 ulSflAd, UINT32 ulDataSize)
{
#ifdef FPGA_ENTRY_4K
    UINT16  uiCount = 0;
    UINT32  ulBLOCK = ulDataSize/0x8000;
    UINT32  ulCount = 0;
    UINT32  ulTotoalCheckSum = 0, ulCheckSum = 0;
    UINT32  ulReadFlashAdd = ulSflAd;
    UINT32  ulSectorSize = 0;

    //UINT8   ucaData[0x8000] = {0};
    UINT8   *pBuf = NULL;
    pBuf = (UINT8 *)malloc(0x8000);


    dvProAV_DramBankSet(0);

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_SfiDram2Flash(ulDRAM_Addr, ulSflAd, ulDataSize, IO_READ);

        halGui_SemaphoreGive(FALSE);
    }

    LOG_MSG(db_DV_PROAV_OSD, "Read Add 0x%08x 0x%08x-0x%08x\n", ulDRAM_Addr, ulSflAd, ulSflAd + ulDataSize);

    for(ulCount = 0; ulCount < ulBLOCK; ulCount++)
    {
        if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
        {
            dvProAV_SfiDramAccess(ulDRAM_Addr, pBuf, 0x8000, IO_READ);

            ulDRAM_Addr += 0x8000;

            halGui_SemaphoreGive(FALSE);
        }

        for(uiCount = 0; uiCount < 0x8000; uiCount++)
        {
            ulTotoalCheckSum = ulTotoalCheckSum + (pBuf[uiCount] * (ulSectorSize + 1));
            ulCheckSum = ulCheckSum + (pBuf[uiCount] * (ulSectorSize + 1));
            ulReadFlashAdd++;
            ulSectorSize++;
        }
    }

    LOG_MSG(db_DV_PROAV_OSD, "Total CheckSum 0x%08x\n", ulTotoalCheckSum);

    free(pBuf);

    return ulTotoalCheckSum;
#else
    UINT16  uiCount = 0;
    UINT32  ulBLOCK = ulDataSize/0x4000;
    UINT32  ulCount = 0;
    UINT32  ulTotoalCheckSum = 0, ulCheckSum = 0;
    UINT32  ulReadFlashAdd = ulSflAd;
    UINT32  ulSectorSize = 0;

    //UINT8   ucaData[0x4000] = {0};
    UINT8   *pBuf = NULL;
    pBuf = (UINT8 *)malloc(0x4000);


    dvProAV_DramBankSet(0);

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_SfiDram2Flash(ulDRAM_Addr, ulSflAd, ulDataSize, IO_READ);

        halGui_SemaphoreGive(FALSE);
    }

    LOG_MSG(db_DV_PROAV_OSD, "Read Add 0x%08x 0x%08x-0x%08x\n", ulDRAM_Addr, ulSflAd, ulSflAd + ulDataSize);

    for(ulCount = 0; ulCount < ulBLOCK; ulCount++)
    {
        if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
        {
            dvProAV_SfiDramAccess(ulDRAM_Addr, pBuf, 0x4000, IO_READ);

            ulDRAM_Addr += 0x4000;

            halGui_SemaphoreGive(FALSE);
        }

        for(uiCount = 0; uiCount < 0x4000; uiCount++)
        {
            ulTotoalCheckSum = ulTotoalCheckSum + (pBuf[uiCount] * (ulSectorSize + 1));
            ulCheckSum = ulCheckSum + (pBuf[uiCount] * (ulSectorSize + 1));
            ulReadFlashAdd++;
            ulSectorSize++;
        }
    }

    LOG_MSG(db_DV_PROAV_OSD, "Total CheckSum 0x%08x\n", ulTotoalCheckSum);

    free(pBuf);
#endif /* FPGA_ENTRY_4K */

    return ulTotoalCheckSum;
}

#define FLASH_SECTION_SIZE 0x10000

UINT32 halGui_GuiData_PartialWrite(char *filename, UINT32 ulFlash_Addr, UINT32 ulDRAM_Addr, BOOL cFocus) //A70LV_Larry_0134
{
    UINT8  cResult = rcSUCCESS;
    UINT32 ulBlock = 0;
    UINT32 ulCount = 0;
    UINT32 ulReadCRC = 0, ulReadCRC2 = 0;
    UINT32 ulExtCRC = 0;
    UINT32 ulSectorSize = 0; //A70LV_Larry_0158
    //UINT8 *pucData = NULL;
    FILE *pFile = NULL;
    UINT32 ulFileSize;
    char *cbuf = NULL;
    UINT32 ulAddr = 0;
    UINT32 ulSize = 0;
    UINT8 cRetry = 0;
    UINT8 cRetryStep = 0;
    UINT8 *pcReadBuffer = NULL;

    pFile = fopen(filename, "rb");

    if(pFile == NULL) //A70LV_Larry_0001
    {
        return 0;
    }
    else //計算.dat szie
    {
        fseek(pFile, 0, SEEK_END);
        ulSize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
    }

    ulBlock = (ulSize % FLASH_SECTION_SIZE == 0) ? (ulSize/FLASH_SECTION_SIZE):(ulSize/FLASH_SECTION_SIZE) + 1;
    ulFileSize = ulBlock*FLASH_SECTION_SIZE;

    cbuf = (uint8 *)malloc(ulFileSize);

    memset(cbuf, 0xFF, ulFileSize);
    fread(cbuf, 1, ulSize, pFile);
    fclose(pFile);

    pcReadBuffer = (UINT8 *)malloc(FLASH_SECTION_SIZE + 0x1000);

    for(ulCount = 0; ulCount < ulBlock; ulCount++)
    {
        ulReadCRC = 0;
        ulReadCRC2 = 0;
        ulSectorSize = 0;
        ulExtCRC = 0;

        memset(pcReadBuffer, 0, FLASH_SECTION_SIZE);
        halGui_GuiData_FlashRead(ulDRAM_Addr, ulFlash_Addr, FLASH_SECTION_SIZE, pcReadBuffer);
        ulReadCRC = utilCRC16Calc(pcReadBuffer, FLASH_SECTION_SIZE);
        ulExtCRC = utilCRC16Calc((cbuf + ulCount * FLASH_SECTION_SIZE), FLASH_SECTION_SIZE);


        if((ulReadCRC != ulExtCRC) || cFocus)
        {
            cRetry = 0;
            cResult = rcSUCCESS;

            do
            {
                if(rcSUCCESS == halGui_GuiData_FlashEraseSector(ulFlash_Addr, FLASH_SECTION_SIZE))
                {
                    halGui_GuiData_FlashWrite(ulDRAM_Addr, ulFlash_Addr, FLASH_SECTION_SIZE, &cbuf[ulCount*FLASH_SECTION_SIZE]);

                    memset(pcReadBuffer, 0, FLASH_SECTION_SIZE);
                    halGui_GuiData_FlashRead(ulDRAM_Addr, ulFlash_Addr, FLASH_SECTION_SIZE, pcReadBuffer);
                    ulReadCRC2 = utilCRC16Calc(pcReadBuffer, FLASH_SECTION_SIZE);

                    if(ulExtCRC != ulReadCRC2)
                    {
                        cResult = rcERROR;
                        LOG_MSG(db_UPGRADE, "Flash Copy Fail[%d]\n", cRetry);
                    }
                    else
                    {
                        cResult = rcSUCCESS;
                        LOG_MSG(db_UPGRADE, "Flash Copy Pass[%d]\n", cRetry);
                    }

                }
                else
                {
                    cResult = rcERROR;
                    LOG_MSG(db_UPGRADE, "Flash erase fail\n");
                }

                cRetry++;
                if(cRetry == 5)
                {
                    cRetry = 0;
                    cRetryStep++;
                    switch(cRetryStep)
                    {
                        case 1: // read clock delay 1T
                            dvProAV_SfiReadDlySet(1);
                            LOG_MSG(db_ALWAYS, "Retry setp : flash read clock delay 1T\n");
                            break;
                        case 2: // read clock delay 2T
                            dvProAV_SfiReadDlySet(2);
                            LOG_MSG(db_ALWAYS, "Retry setp  : flash read clock delay 2T\n");
                            break;
                        case 3: // clock rate 133M/6  & read clock delay 0T
                            dvProAV_SfiReadDlySet(0);
                            dvProAV_SfiClkRateSet(3);
                            LOG_MSG(db_ALWAYS, "Retry setp  : flash clock rate 133M/6  & read clock delay 0T\n");
                            break;
                        case 4: // clock rate 133M/6 & read clock delay 1T
                            dvProAV_SfiReadDlySet(1);
                            LOG_MSG(db_ALWAYS, "Retry setp  : flash clock rate 133M/6  & read clock delay 1T\n");
                            break;
                        case 5: // clock rate 133M/6 & read clock delay 2T
                            dvProAV_SfiReadDlySet(2);
                            LOG_MSG(db_ALWAYS, "Retry setp  : flash clock rate 133M/6  & read clock delay 2T\n");
                            break;
                        case 6: // flash r/w fail, set to default value
                            cRetry = 5;
                            dvProAV_SfiReadDlySet(0);
                            dvProAV_SfiClkRateSet(2);
                            LOG_MSG(db_ALWAYS, "Retry setp  : flash r/w fail, set to default value\n");
                            break;
                    }
                }

            }while((ulExtCRC != ulReadCRC2) && (cRetry < 5));

            if(cRetry == 5)
            {
                free(cbuf);
                free(pcReadBuffer);

                return rcERROR;
            }
        }

        LOG_MSG(db_UPGRADE, "Copy Address %03d 0x%08x-0x%08x [0x%04x][0x%04x][0x%04x]\n", ulCount, ulFlash_Addr, ulFlash_Addr + (FLASH_SECTION_SIZE - 1), ulExtCRC, ulReadCRC, ulReadCRC2);

        ulDRAM_Addr = ulDRAM_Addr + FLASH_SECTION_SIZE;
        ulFlash_Addr = ulFlash_Addr + FLASH_SECTION_SIZE;

        //MS_SLEEP(10);
    }

    free(cbuf);

    return cResult;
}

INT8 halGui_InitSemaphore(void)     //A70LV_Doulas_0014
{
//#if(CURRENT_RTOS_TYPE == RTOS_STATIC)
    //xGuiSemaphore = halScaler_xSemaphore_Get();
//#else
    //xGuiSemaphore = halScaler_xSemaphore_Get();
//#endif

    //if(xGuiSemaphore == NULL)
        //return ERROR_GUI_SEMAPHORE_FAIL;

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d)Init complete!\n", __FUNCTION__, __LINE__);
    return GUI_PASS;
}

INT8 halGui_PanelSet(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    dvProAV_OSD_PanelSet(ePanelTimingId);
    return GUI_PASS;
}

INT8 halGui_PanelChange(ePANEL_ID ePanelTimingId)      //A70LV_Doulas_0105
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        //ProAV_OSD_FillColor(GUI_C_BLACK);

        dvProAV_OSD_Enable(false);
        MS_SLEEP(10);
        dvProAV_OSD_PanelChange(ePanelTimingId);

        halGui_SemaphoreGive(0);
    }

    return GUI_PASS;
}

INT8 halGui_OSD_MenuTransparencySet(UINT8 ucValue)         //A70LV_Doulas_0122
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_OSD_MenuTransparencySet(ucValue);
        halGui_SemaphoreGive(0);
    }

    return GUI_PASS;
}

INT8 halGui_OSD_MenuTransparencyEnableSet(UINT8 ucEnable)         //A70LV_Doulas_0122
{
    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_OSD_MenuTransparencyEnableSet(ucEnable);
        halGui_SemaphoreGive(0);
    }

    return GUI_PASS;
}

INT8 halGui_DrawRect(UINT16 uiXStart, UINT16 uiXWidth, UINT16 uiYStart, UINT16 uiYWidth, UINT8 ucForeColor, UINT8 ucBackColor) //A70LV_Larry_0137
{
    //halGui_SemaphoreTake(0, 417);
    //dvProAV_OSD_DrawRect(uiXStart, uiXWidth, uiYStart, uiYWidth, ucForeColor, ucBackColor);
    //halGui_SemaphoreGive(0);

    return GUI_PASS;
}

	//Cassper_ProAV
INT8 halGui_Paint_Line(COORDINATE sStart_Position, UINT8 width, INT16 iColor_Index)   //ZU860_Clare_0001, add
{
    START_POINT sDes_position;
    RECT_SIZE sDes_size;
    INT8 iVal = 0;  //ZU860_Doulas_0136

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
	{
		memcpy(&sDes_position, &sStart_Position, sizeof(START_POINT));

		sDes_position.iX = sStart_Position.iX;
		sDes_position.iY = sStart_Position.iY;
		sDes_size.iWidth = width;
		sDes_size.iHeight = 1;

		//iVal = dvC821_Paint_Rectangle(sDes_position, sDes_size, iColor_Index);
		halGui_SemaphoreGive(0);          //ZU860_Doulas_0136
	}
    return iVal;
}

UINT8 halGui_LogoReplacementEnable(UINT16 uiPanel)
{
    UINT8 cResult = rcSUCCESS;

#ifndef SIMULATOR_ISCALER
    int status = rcSUCCESS;
    UINT8 *pFileColor = NULL;
    UINT16 *pFileRGB565 = NULL;
    INT16 iWidth, iHeight;
    UINT32 ulLen = 0;
    UINT32 ulAddr = 0;
    RGB   *pRGB = NULL;
    FILE *fp = NULL;
    PanelTiming sPanelInfo = {0};

    if(ePROAV_EXEC_CODE_PASS == dvPro_GetPanelInfo(uiPanel, (UINT8*)&sPanelInfo))
    {
        if(uiPanel == PANEL_2D_OUTPUT)
        {
            if(access(LOGO_REPLACEMENT_DEFAULT_2D, 0) == -1) //A70LK_Jacky_0011
            {
                return rcERROR;
            }
            read_png_file(LOGO_REPLACEMENT_DEFAULT_2D, (int*)&iWidth, (int*)&iHeight);
        }
        else if(uiPanel == PANEL_3D_OUTPUT)
        {
            if(access(LOGO_REPLACEMENT_DEFAULT_3D, 0) == -1) //A70LK_Jacky_0011
            {
                return rcERROR;
            }
            read_png_file(LOGO_REPLACEMENT_DEFAULT_3D, (int*)&iWidth, (int*)&iHeight);
        }
        else
        {
           return rcERROR;
        }

        //確認圖片size和output panel是否一致
        if(sPanelInfo.HSize != iWidth || sPanelInfo.VSize != iHeight)
        {
            return rcERROR;
        }
    }
    else
    {
        return rcERROR;
    }


    pFileColor = (UINT8 *)malloc(sPanelInfo.HSize*sPanelInfo.VSize*3+1024);

    if(pFileColor == NULL)
    {
        return rcERROR;
    }

    pFileRGB565 = (UINT16 *)malloc(sPanelInfo.HSize*sPanelInfo.VSize*2+1024);

    if(pFileRGB565 == NULL)
    {
        free(pFileColor); //A70LK_Casper_0001
        return rcERROR;
    }

    if(uiPanel == PANEL_2D_OUTPUT)
    {
        fp = fopen(LOGO_REPLACEMENT_DEFAULT_2D_RGB565, "w");

        if(fp == NULL)
        {
            free(pFileColor);
            free(pFileRGB565);
            return rcERROR;
        }

        read_png_file(LOGO_REPLACEMENT_DEFAULT_2D, (int*)&iWidth, (int*)&iHeight);
        process_file(pFileColor, iWidth, iHeight);

        for(UINT16 i = 0; i < iHeight; i++)
        {
            pRGB =(RGB*)&pFileColor[iWidth*3*i];

            for(UINT16 j = 0; j < iWidth; j++)
            {
                pFileRGB565[ulLen] =  (UINT16)RGB888TORGB565(pRGB->ucRed, pRGB->ucGreen, pRGB->ucBlue);

                pRGB++;
                ulLen++;
            }
        }

        ulLen = ulLen*2;
        ulAddr = DRAM_LOGO_DISPLAY1_2D_ADDR;
    }
    else if(uiPanel == PANEL_3D_OUTPUT)
    {
         fp = fopen(LOGO_REPLACEMENT_DEFAULT_3D_RGB565, "w");

        if(fp == NULL)
        {
            free(pFileColor);
            free(pFileRGB565);
            return rcERROR;
        }

        read_png_file(LOGO_REPLACEMENT_DEFAULT_3D, (int*)&iWidth, (int*)&iHeight);
        process_file(pFileColor, iWidth, iHeight);

        for(UINT16 i = 0; i < iHeight; i++)
        {
            pRGB =(RGB*)&pFileColor[iWidth*3*i];

            for(UINT16 j = 0; j < iWidth; j++)
            {
                pFileRGB565[ulLen] = (UINT16)RGB888TORGB565(pRGB->ucRed, pRGB->ucGreen, pRGB->ucBlue);

                pRGB++;
                ulLen++;
            }
        }

        ulLen = ulLen*2;
        ulAddr = DRAM_LOGO_DISPLAY1_3D_ADDR;
    }


    free(pFileColor);

    fwrite(pFileRGB565, sPanelInfo.HSize*sPanelInfo.VSize*2, 1, fp);
    fclose(fp);

#if 1
    if(halGui_SemaphoreTake(0, __FUNCTION__) == TRUE)
    {
        status &= dvProAV_LineBufAccess(eDramBank0, ulAddr, (uint08 *)&pFileRGB565[0], ulLen, IO_WRITE);

        free(pFileRGB565);

        halGui_SemaphoreGive(0);
    }

    if(uiPanel == PANEL_2D_OUTPUT)
    {
        cResult &= halScaler_Dram2Flash(DRAM_LOGO_DISPLAY1_2D_ADDR, FLASH_LOGO1_2D_ADDR, ulLen);
    }
    else if(uiPanel == PANEL_3D_OUTPUT)
    {
        cResult &= halScaler_Dram2Flash(DRAM_LOGO_DISPLAY1_3D_ADDR, FLASH_LOGO1_3D_ADDR, ulLen);
    }

#endif /* 0 */

#endif /* SIMULATOR_ISCALER */

    return cResult;
}


void halGui_ShowTest(void)
#if 1
{
    COORDINATE sStart_Position = {100, 100};
    BLOCK_SIZE_INFO sSize = {960, 500};
    char ucString[127] = {'\0'};

    UINT8 *pucString = {"ABCDEFGHIJKLMN"};
    //INT16 iColor_Index,
    //INT16 iColor_Background,
    //INT16 iBoarder_Index
    static UINT16 test = 0;

    //sSize = strlen(ucString) + 1;



    halGui_Paint_RectanglewithBoarder(sStart_Position, sSize, GUI_C_GREEN, GUI_C_RED, ebiHIGHLIGHT);

    sStart_Position.iX = 120;
    sStart_Position.iY = 120;

    sSize.iWidth  = 200;
    sSize.iHeight = 200;

    halGui_Paint_ASCIIString(pucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);


    sSize.iWidth  = 200;
    sSize.iHeight = 32;

    sStart_Position.iY = sStart_Position.iY + 32;

    test++;
    sprintf(ucString, "%d%d%d%d%d%d",test, test, test, test, test, test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString((UINT8 *)ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);


    sStart_Position.iY = sStart_Position.iY + 32;

    test++;
    sprintf(ucString, "%d%d%d%d%d%d",test, test, test, test, test, test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString((UINT8 *)ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);

    sStart_Position.iY = sStart_Position.iY + 32;

    test++;
    sprintf(ucString, "%d%d%d%d%d%d",test, test, test, test, test, test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString((UINT8 *)ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);

    sStart_Position.iY = sStart_Position.iY + 32;

    test++;
    sprintf(ucString, "%d%d%d%d%d%d",test, test, test, test, test, test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString((UINT8 *)ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);


    sStart_Position.iX = 120;
    sStart_Position.iY = 400;

    sSize.iWidth  = 200;
    sSize.iHeight = 200;


    halGui_Paint_RectanglewithBoarder(sStart_Position, sSize, GUI_INHITBIT_COLOR, GUI_INHITBIT_COLOR, ebiHIGHLIGHT);
#if 0
    test++;

    sprintf(ucString, "%d\0\n", test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString(ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);

    sStart_Position.iY = sStart_Position.iY + 32;
    test++;

    sprintf(ucString, "%d\0\n", test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString(ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);

    sStart_Position.iY = sStart_Position.iY + 32;
    test++;

    sprintf(ucString, "%d\0\n", test);
    //sSize = strlen(ucString) + 1;
    halGui_Paint_ASCIIString(ucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);
#endif /* 0 */

    sStart_Position.iX = 0;
    sStart_Position.iY = 0;

    sSize.iWidth  = 1920;
    sSize.iHeight = 1080;

    halGui_OSD_On(sStart_Position,sSize);


}
#else
{
    COORDINATE sStart_Position = {100, 100};
    BLOCK_SIZE sSize = {500, 500};
    UINT8 *pucString = {"ABCDEFGHIJKLMN"};
	UINT8 *pucString2 = {"OPQRSDGGKLAOEMV"};
    //INT16 iColor_Index,
    //INT16 iColor_Background,
    //INT16 iBoarder_Index


    halGui_Paint_RectanglewithBoarder(sStart_Position, sSize, GUI_C_GREEN, GUI_C_RED, ebiHIGHLIGHT);

    sStart_Position.iX = 120;
    sStart_Position.iY = 120;

    sSize.iWidth  = 200;
    sSize.iHeight = 200;

    halGui_Paint_ASCIIString(pucString, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);

	sStart_Position.iX = 120;
    sStart_Position.iY = 160;

    sSize.iWidth  = 200;
    sSize.iHeight = 200;

    halGui_Paint_ASCIIString(pucString2, sStart_Position, sSize, GUI_C_YELLOW, eoaCENTERED);


    sStart_Position.iX = 0;
    sStart_Position.iY = 0;

    sSize.iWidth  = 1920;
    sSize.iHeight = 1080;

    halGui_OSD_On(sStart_Position,sSize);


}

#endif /* 0 */

//A70LK_CL_0002 map RGB to index of color palette of scaler OSD
UINT16 halGui_RGBColorToIndex(uint32 uRGBColor)
{
    UINT16 uiErrorIdx = 256;  //can not match in scaler's OSD palette

    UINT8 OsdPalette[768] = {0};
    utilProcMutexData_OsdPalette_Get(OsdPalette);

    for(int idx = 0 ; idx < 256 ; idx++)
    {
        if((OsdPalette[0+(idx*3)] == (uint8)((uRGBColor >> 0) & 0x000000FF))
            &&(OsdPalette[1+(idx*3)] == (uint8)((uRGBColor >> 8) & 0x000000FF))
            &&(OsdPalette[2+(idx*3)] == (uint8)((uRGBColor >> 16) & 0x000000FF)))
            {
                    return idx;
            }
    }

    return uiErrorIdx;
}

//A70LK_CL_0007 restore color palette of scaler OSD
INT8 halGui_RestorePalette(void)
{
    //if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        UINT8 OsdPalette[768] = {0};
        utilProcMutexData_OsdPalette_Get(OsdPalette);  //H2PF_Simon_0038
        OSD_Palette_Set(256, OsdPalette);
        //dvProAV_OSD_Buffer_Init();

        //halGui_SemaphoreGive(0);
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) complete!\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}

INT8 halGui_Buffer_Init(void)
{
    if(halGui_InitSemaphore() != GUI_PASS)      //A70LV_Doulas_0014
        return ERROR_GUI_SEMAPHORE_FAIL;

    if(TRUE == halGui_SemaphoreTake(0, __FUNCTION__))
    {
        dvProAV_OSD_Buffer_Init();
        halGui_SemaphoreGive(0);
    }

    LOG_MSG(db_HAL_GUI, "(func:%s, line:%d) complete!\n", __FUNCTION__, __LINE__);

    return GUI_PASS;
}

////////////////////////////
//only for compiler error
////////////////////////////
INT8 halGui_Paint_Specified_Bitmap(OSD_BITMAP_INFO sBitmap, COORDINATE sSrc_Offset_Position, COORDINATE sDraw_Position, BLOCK_SIZE_INFO sDraw_Size)  //H2 wait review
{
    return 1;
}

INT8 halGui_Paint_SubString( OSD_STRING_INFO sOsdString, START_POINT_INFO sDes_position, INT16 iTextColor, INT16 iStringOffset)
{
    return 1;
}

void halGui_Upgrade(UINT8 ucFocus)
{
    return;
}

void halGui_UpgradeAccess_Set(BOOL bEnable)
{
    return;
}

BOOL halGui_UpgradeAccess_Get(void)
{
    return 0;
}

void halGui_ShowOsdBitmapData(UINT32 ulStartVertPosition)
{
    return;
}

