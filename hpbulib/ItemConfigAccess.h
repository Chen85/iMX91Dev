#ifndef _ITEMCONFIGACCESS_
#define _ITEMCONFIGACCESS_

#include <stdio.h>
#include <stdlib.h>
#include "type_def.h"
#include "hicc_config.h"
#include "SharedMemCtrl.h"


#ifndef __UBUNTU_SIMULATOR__
#define UI_CONFIG_FILE_PATH "/usr/configs/ProjectSettings/appGuiItemInfo.csv"
#else
#define UI_CONFIG_FILE_PATH "../hpbulib/ProjectSettings/appGuiItemInfo.csv"
#endif /* __UBUNTU_SIMULATOR__ */

#define FIND_FROM_SORTED_DATABASE (TRUE)
#define FIND_FROM_NOT_SORTED_DATABASE (FALSE)

#define DATABASE_INDEX_NOT_FIND (-1)

#pragma pack(push)
#pragma pack(1)



#pragma pack(pop)


typedef enum
{
    eItemName,
	eItemID,
    eParentMenuID,
    eNextMenuID,
    eStyle,
    eOperationType,
    eDataCode,
    eDataCodeIndex,
    eHide,
    eMax,
    eMin,
    eOffset,
    eStep,
    eValue,

    eFieldNameInvalid,

}eFIELD_NAME;


//char *strrpc(char *str,char *oldstr,char *newstr);
INT8 ItemConfig_CSV_File_LoadToShm(void);
int ItemID_Compare(const void *arg1, const void *arg2);
int Datacode_Compare(const void *arg1, const void *arg2);
int strrpc(char *str,char *oldstr,char *newstr);
INT32 FindItemIndex_ByItemID(UINT32 ItemID, UINT8 SortedData);
INT32 FindItemIndex_ByItemName(char *ItemName);
INT32 FindItemIndex_ByDatacode(UINT32 DataCode, UINT8 SortedData);
UINT8 ItemHideStatusGet(UINT32 ItemID);
INT32 ItemInfoGet_Max(UINT32 ItemID);
INT32 ItemInfoGet_Min(UINT32 ItemID);
UINT32 ItemInfoGet_Step(UINT32 ItemID);
UINT32 ItemInfoGet_Value(UINT32 ItemID);
INT32 GetItemField(UINT32 ItemID, eFIELD_NAME Field);
INT32 GetItemInfo_ByItemName(UINT16 TotalNum, char **ItemNameSet, sUI_ITEM_CFG *Info);
INT32 GetItemInfo_ByItemID(UINT16 TotalNum, UINT32 *ItemIDSet, sUI_ITEM_CFG *Info);
INT32 GetItemInfo_ByDatacode(UINT16 TotalNum, UINT32 *DatacodeSet, sUI_ITEM_CFG *Info);
INT32 GetItemInfo_ByIndex(UINT16 TotalNum, UINT32 *IndexSet, sUI_ITEM_CFG *Info);
INT32 SetItemInfo_ByItemName(char *ItemName, sUI_ITEM_CFG *Info);
INT32 SetItemInfo_ByItemID(UINT32 ItemID, sUI_ITEM_CFG *Info);
INT32 SetItemField_ByItemID(UINT32 ItemID, eFIELD_NAME Field, ...);
INT32 SetItemField_ByItemName(char *ItemName, eFIELD_NAME Field, ...);
INT32 ItemCountCheck(void);
UINT32 ItemCountGet(void);
INT32 ItemConfig_SaveToCSVfile_FromShm(void);
INT32 GetItemField(UINT32 ItemID, eFIELD_NAME Field);

//test
INT32 ItemConfig_ReadCSVfile(char *cDatacode, INT32 *iMaxValue, INT32 *iMinValue);
void ItemConfig_DbMsgEnable(UINT8 Enable);
void DbMsg(const char *Msg, ...);

#endif //_ITEMCONFIGACCESS_


