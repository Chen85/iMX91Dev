#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#include "MathAPI.h"
#include "ItemConfigAccess.h"
#include "GeneralFunc.h"

//#define _atoi64(val)     strtoll(val, NULL, 10)

UINT32 ItemCount = 0 ;
UINT8 DbMsgEnable = FALSE;

void ItemConfig_DbMsgEnable(UINT8 Enable)
{
    DbMsgEnable = Enable;
}

void DbMsg(const char *Msg, ...)
{
    if(DbMsgEnable)
    {
        va_list args;
        va_start(args, Msg);

        char String[256] = {'\0'};
        vsnprintf(String, sizeof(String), Msg, args);
        printf(String);
        va_end(args);
    }
}

#define CSV_FILE_FIRST_LINE_STRING "ItemName,ItemID,ParentMenuID,NextMenuID,Style,OperationType,DataCode,DataCodeIndex,Hide,Max,Min,Offset,Step,Value\r\n"

INT8 ItemConfig_CSV_File_LoadToShm(void)
{
    FILE *fp  = fopen(UI_CONFIG_FILE_PATH, "rb");

    if(fp == NULL)
    {
        DbMsg("No File (%s)\n", UI_CONFIG_FILE_PATH);
        return FALSE;
    }

    //get file size
    //UINT32 FileSize = Get_File_Size(UI_CONFIG_FILE_PATH);
    //char *FileBuf = (char *)malloc(FileSize);
    //fread(FileBuf, FileSize, 1, fp);

    //count line number
    ItemCount = Get_File_LineNumber(UI_CONFIG_FILE_PATH);
    ItemCount = ItemCount - 1; //skip first line

    DbMsg("Total Item %d\n", ItemCount);

    if(ItemCount <= 0)
    {
        DbMsg("No Item Info\n");
        return FALSE;
    }

    //malloc ram for copy to shared memory
    sUI_ITEM_CFG *psShmData = (sUI_ITEM_CFG *)malloc(sizeof(sUI_ITEM_CFG) * ItemCount);

    //get data from file (separate by ',')
    rewind(fp);
    char *token ;
    char OneLineData[1024]={'\0'};
    UINT32 Line = 0 ;
    while(fgets(OneLineData, sizeof(OneLineData), fp) != NULL)
    {
        if(Line == 0)   //skip first line
        {
            Line++;
            continue;
        }

        UINT32 ItemIndex = Line - 1;

        token = strtok(OneLineData, ",");
        while(token != NULL)
        {
            for(eFIELD_NAME eField=eItemName; eField<eFieldNameInvalid; eField++)
            {
                switch(eField)
                {
                    case eItemName:         snprintf(psShmData[ItemIndex].ItemName,      80, "%s", token);   break;
                    case eItemID:    	    psShmData[ItemIndex].ItemID  = strtoll(token, NULL, 10);         break;
                    case eParentMenuID:     psShmData[ItemIndex].ParentMenuID   = atoi(token);   break;
                    case eNextMenuID:       psShmData[ItemIndex].NextMenuID     = atoi(token);   break;
                    case eStyle:            psShmData[ItemIndex].Style          = atoi(token);   break;
                    case eOperationType:    psShmData[ItemIndex].OperationType  = atoi(token);   break;
                    case eDataCode:         snprintf(psShmData[ItemIndex].DataCode,      80, "%s", token);   break;
                    case eDataCodeIndex:    psShmData[ItemIndex].DataCodeIndex  = atoi(token);  break;
                    case eHide:             psShmData[ItemIndex].Hide           = atoi(token);  break;
                    case eMax:              psShmData[ItemIndex].Max            = atoi(token);  break;
                    case eMin:              psShmData[ItemIndex].Min            = atoi(token);  break;
                    case eOffset:           psShmData[ItemIndex].Offset         = atoi(token);  break;
                    case eStep:             psShmData[ItemIndex].Step           = atoi(token);  break;
                    case eValue:            psShmData[ItemIndex].Value          = atoi(token);  break;


                    default: DbMsg("!! %s Undefine Field\n", __FUNCTION__);    break;
                }

                token = strtok(NULL, ",");

                if(token == NULL)
                    break;
            }

        }

        Line++;

        if(Line > ItemCount)
            break;
    }

    fclose(fp);

    for(int i=0; i<ItemCount; i++)
    {
        SharedMem_ImportData(eSB_UI_ITEM_INFO, psShmData+i, i);
    }

    ////////////////////////////////////////////////////////////////////////////
    //DbMsg("QSort by ItemID Start\n");
    qsort(psShmData, ItemCount, sizeof(sUI_ITEM_CFG), ItemID_Compare);
    //DbMsg("QSort by ItemID End\n");
    ////////////////////////////////////////////////////////////////////////////

    //copy to shared memory (already sort by ItemID)
    for(int i=0; i<ItemCount; i++)
    {
        SharedMem_ImportData(eSB_UI_ITEM_INFO_SORT_BY_ITEMID, psShmData+i, i);

        //sUI_ITEM_CFG sItemInfo;
        //SharedMem_ReadData_New(eSB_UI_ITEM_INFO_SORT_BY_ITEMID, &sItemInfo, i);
        //DbMsg("<%llu>\n",sItemInfo.ItemID );
    }

    ////////////////////////////////////////////////////////////////////////////
    //DbMsg("QSort by DataCode Start\n");
    qsort(psShmData, ItemCount, sizeof(sUI_ITEM_CFG), Datacode_Compare);
    //DbMsg("QSort by DataCode End1\n");
    ////////////////////////////////////////////////////////////////////////////

    //copy to shared memory (already sort by DataCode)
    for(int i=0; i<ItemCount; i++)
    {
        SharedMem_ImportData(eSB_UI_ITEM_INFO_SORT_BY_DATACODE, psShmData+i, i);

        //sUI_ITEM_CFG sItemInfo;
        //SharedMem_ReadData_New(eSB_UI_ITEM_INFO_SORT_BY_DATACODE, &sItemInfo, i);
        //DbMsg("<%d>\n",sItemInfo.DataCodeIndex );
    }

    //free(FileBuf);
    //FileBuf = NULL;
    free(psShmData);
    psShmData = NULL;

    //DbMsg("ItemConfig_CSV_File_LoadToShm end\n");

    return TRUE;

}

//For qsort compare function (by ItemID)
int ItemID_Compare(const void *arg1, const void *arg2)
{
    sUI_ITEM_CFG wArg1 = *(sUI_ITEM_CFG *)(arg1);
    sUI_ITEM_CFG wArg2 = *(sUI_ITEM_CFG *)(arg2);

    if(wArg1.ItemID < wArg2.ItemID)
    {
        return -1;
    }

    if(wArg1.ItemID == wArg2.ItemID)
    {
        return 0;
    }

    return 1;
}


//For qsort compare function (by Datacode)
int Datacode_Compare(const void *arg1, const void *arg2)
{
    sUI_ITEM_CFG wArg1 = *(sUI_ITEM_CFG *)(arg1);
    sUI_ITEM_CFG wArg2 = *(sUI_ITEM_CFG *)(arg2);

    if(wArg1.DataCodeIndex < wArg2.DataCodeIndex)
    {
        return -1;
    }

    if(wArg1.DataCodeIndex == wArg2.DataCodeIndex)
    {
        return 0;
    }

    return 1;
}

#if 0
int main()
{
    ItemConfig_CSV_File_LoadToShm();

    char *Name[2] = {"IMAGE_TINT\0", "IMAGE_SHARPNESS\0"};
    sUI_ITEM_CFG UI_INFO[2];
    GetItemInfo_ByItemName(2, Name, UI_INFO);
    DbMsg("- <%s><%d>\n",     UI_INFO[0].ItemName, UI_INFO[0].Max);
    DbMsg("- <%s><%d><%d>\n", UI_INFO[1].ItemName, UI_INFO[1].Max, UI_INFO[1].Hide);

    UINT32 ID[2] = {3366869919, 595759901};
    //sUI_ITEM_CFG UI_INFO[2];
    GetItemInfo_ByItemID(2, ID, UI_INFO);
    DbMsg("- <%s><%d>\n",     UI_INFO[0].ItemName, UI_INFO[0].Max);
    DbMsg("- <%s><%d><%d>\n", UI_INFO[1].ItemName, UI_INFO[1].Max, UI_INFO[1].Hide);


    SetItemField_ByItemName("IMAGE_SHARPNESS", eMax, 200);
    SetItemField_ByItemName("IMAGE_SHARPNESS", eMin, -333);
    SetItemField_ByItemName("IMAGE_SHARPNESS", eDataCode, "edcSSSS");
    ItemConfig_SaveToCSVfile_FromShm();

    DbMsg("\n---End--\n");

    return 0;
}
#endif

int strrpc(char *str,char *oldstr,char *newstr)
{
    int findnum = 0;

    UINT32 BufSz = 0;
    if(strlen(str) < strlen(newstr))
    {
        BufSz = strlen(newstr) + 1;
    }
    else
    {
        BufSz = strlen(str)*2;
    }

    char bstr[BufSz];//轉換緩衝區
    memset(bstr, 0, BufSz);

    //DbMsg("<%d,%d,%d>\n", strlen(str), strlen(oldstr), strlen(newstr));

    if(strlen(str) < strlen(oldstr))
    {
        return 0;
    }

    for(int i = 0;i < strlen(str);i++){
        if(!strncmp(str+i,oldstr,strlen(oldstr))){//查詢目標字串
            strcat(bstr,newstr);
            i += strlen(oldstr) - 1;
            findnum++;
        }else{
        	strncat(bstr,str + i,1);//儲存一位元組進緩衝區
	    }
    }

    strcpy(str,bstr);
    return findnum;
}


#if 0
char *strrpc(char *str,char *oldstr,char *newstr)
{
    char bstr[strlen(str)];//轉換緩衝區
    memset(bstr,0,sizeof(bstr));

    for(int i = 0;i < strlen(str);i++){
        if(!strncmp(str+i,oldstr,strlen(oldstr))){//查詢目標字串
            strcat(bstr,newstr);
            i += strlen(oldstr) - 1;
        }else{
        	strncat(bstr,str + i,1);//儲存一位元組進緩衝區
	    }
    }

    strcpy(str,bstr);
    return str;
}
#endif


INT32 FindItemIndex_ByItemID(UINT32 ItemID, UINT8 SortedData)
{
    int search = DATABASE_INDEX_NOT_FIND;

    if(ItemCountCheck() == FALSE)
    {
        return DATABASE_INDEX_NOT_FIND;
    }

    if(SortedData)
    {
        sUI_ITEM_CFG *ShmItemInfoPtr = SharedMem_GetMapPtr(eSB_UI_ITEM_INFO_SORT_BY_ITEMID);

        search = BinarySearch_64bitData((UINT64)ItemID, (UINT64 *)&ShmItemInfoPtr->ItemID, ItemCount, sizeof(sUI_ITEM_CFG));

        if(search == -1)
            DbMsg(">> Error: ItemID %d not found\n", ItemID);
    }
    else
    {
        sUI_ITEM_CFG sItemInfo;

        for(search = 0; search < ItemCount ; search++)
        {
            SharedMem_ReadData_New(eSB_UI_ITEM_INFO, &sItemInfo, search);
            if(ItemID == (UINT32)sItemInfo.ItemID)
                return search;
        }

        if(search == ItemCount)
        {
            search = DATABASE_INDEX_NOT_FIND;
            DbMsg("> Error: ItemID %d not found\n", ItemID);
        }
    }

    return search;

}

INT32 FindItemIndex_ByItemName(char *ItemName)
{
    int search = DATABASE_INDEX_NOT_FIND;

    if(ItemCountCheck() == FALSE)
    {
        return DATABASE_INDEX_NOT_FIND;
    }

    sUI_ITEM_CFG sItemInfo;

    for(search = 0; search < ItemCount ; search++)
    {
        SharedMem_ReadData_New(eSB_UI_ITEM_INFO, &sItemInfo, search);

        if(strncmp(ItemName, sItemInfo.ItemName, 80) == 0)
            return search;
    }

    if(search == ItemCount)
    {
        search = DATABASE_INDEX_NOT_FIND;
        DbMsg("> Error: ItemName %s not found\n", ItemName);
    }

    return search;  //not found
}


INT32 FindItemIndex_ByDatacode(UINT32 DataCode, UINT8 SortedData)
{
    int search = DATABASE_INDEX_NOT_FIND;

    if(ItemCountCheck() == FALSE)
    {
        return DATABASE_INDEX_NOT_FIND;
    }

    //DbMsg("(%s, %d)DataCode:%d\n", __FUNCTION__, __LINE__, DataCode);

    if(SortedData)
    {
        sUI_ITEM_CFG *ShmItemInfoPtr = SharedMem_GetMapPtr(eSB_UI_ITEM_INFO_SORT_BY_DATACODE);

        search = BinarySearch_32bitData(DataCode, &ShmItemInfoPtr->DataCodeIndex, ItemCount, sizeof(sUI_ITEM_CFG));

        if(search == -1)
            DbMsg(">> Error: DataCode %d not found\n", DataCode);
    }
    else
    {
        sUI_ITEM_CFG sItemInfo;

        for(search = 0; search < ItemCount ; search++)
        {
            SharedMem_ReadData_New(eSB_UI_ITEM_INFO, &sItemInfo, search);
            if(DataCode == (UINT32)sItemInfo.DataCode)
                return search;
        }

        if(search == ItemCount)
        {
            search = DATABASE_INDEX_NOT_FIND;
            DbMsg("> Error: DataCode %d not found\n", DataCode);
        }
    }

    return search;

}

INT32 GetItemInfo_ByItemName(UINT16 TotalNum, char **ItemNameSet, sUI_ITEM_CFG *Info)
{
    if(Info == NULL)
    {
        DbMsg("%s Parameter Info is NULL\n", __FUNCTION__);
        return FALSE;
    }

    for(UINT16 i=0; i<TotalNum; i++)
    {
        INT32 index = FindItemIndex_ByItemName(ItemNameSet[i]);

        if(index < 0)  //not find
        {
            memset(Info+i, 0, sizeof(Info));
            continue;
        }


        SharedMem_ReadData_New(eSB_UI_ITEM_INFO, Info+i, index);
    }

    return TRUE;
}

INT32 GetItemInfo_ByItemID(UINT16 TotalNum, UINT32 *ItemIDSet, sUI_ITEM_CFG *Info)
{
    if(Info == NULL)
    {
        DbMsg("%s Parameter Info is NULL\n", __FUNCTION__);
        return FALSE;
    }

    for(UINT16 i=0; i<TotalNum; i++)
    {
        INT32 index = FindItemIndex_ByItemID(ItemIDSet[i], FIND_FROM_SORTED_DATABASE);

        if(index < 0)  //not find
        {
            memset(Info+i, 0, sizeof(Info));
            continue;
        }

        SharedMem_ReadData_New(eSB_UI_ITEM_INFO_SORT_BY_ITEMID, Info+i, index);
    }

    return TRUE;
}


INT32 GetItemInfo_ByDatacode(UINT16 TotalNum, UINT32 *DatacodeSet, sUI_ITEM_CFG *Info)
{
    if(Info == NULL)
    {
        DbMsg("%s Parameter Info is NULL\n", __FUNCTION__);
        return FALSE;
    }

    for(UINT16 i=0; i<TotalNum; i++)
    {
        INT32 index = FindItemIndex_ByDatacode(DatacodeSet[i], FIND_FROM_SORTED_DATABASE);

        if(index < 0)  //not find
        {
            memset(Info+i, 0, sizeof(Info));
            continue;
        }

        SharedMem_ReadData_New(eSB_UI_ITEM_INFO_SORT_BY_DATACODE, Info + i, index);
    }

    return TRUE;
}

//By shared memory data index
INT32 GetItemInfo_ByIndex(UINT16 TotalNum, UINT32 *IndexSet, sUI_ITEM_CFG *Info)
{
    if(Info == NULL)
    {
        DbMsg("%s Parameter Info is NULL\n", __FUNCTION__);
        return FALSE;
    }

    for(UINT16 i=0; i<TotalNum; i++)
    {
        if(IndexSet[i] >= ItemCount)
            return FALSE;

        SharedMem_ReadData_New(eSB_UI_ITEM_INFO, Info + i, IndexSet[i]);
    }

    return TRUE;
}

INT32 SetItemInfo_ByItemName(char *ItemName, sUI_ITEM_CFG *Info)
{
    INT32 index = FindItemIndex_ByItemName(ItemName);

    if(index < 0 || index > ItemCount)
    {
        return FALSE;
    }

    SharedMem_ImportData(eSB_UI_ITEM_INFO, Info, index);

    return TRUE;
}

INT32 SetItemInfo_ByItemID(UINT32 ItemID, sUI_ITEM_CFG *Info)
{
    INT32 index = FindItemIndex_ByItemID(ItemID, FIND_FROM_NOT_SORTED_DATABASE);

    if(index < 0 || index > ItemCount)
    {
        return FALSE;
    }

    SharedMem_ImportData(eSB_UI_ITEM_INFO, Info, index);

    return TRUE;
}

INT32 SetItemField_ByItemID(UINT32 ItemID, eFIELD_NAME Field, ...)
{
    INT32 index = FindItemIndex_ByItemID(ItemID, FIND_FROM_NOT_SORTED_DATABASE);

    if(index < 0 || index > ItemCount)
    {
        return FALSE;
    }

    va_list args;
    va_start(args, Field);

    sUI_ITEM_CFG sItemInfo;
    SharedMem_ReadData_New(eSB_UI_ITEM_INFO, &sItemInfo, index);  //load current info

    switch(Field)
    {
        case eItemName:         snprintf(sItemInfo.ItemName,     80, "%s", va_arg(args, char *));    break;
        case eItemID:    		sItemInfo.ItemID            = va_arg(args, long long);               break;
        case eParentMenuID:     sItemInfo.ParentMenuID      = va_arg(args, int);  break;
        case eNextMenuID:       sItemInfo.NextMenuID        = va_arg(args, int);  break;
        case eStyle:            sItemInfo.Style             = va_arg(args, int);  break;
        case eOperationType:    sItemInfo.OperationType     = va_arg(args, int);  break;
        case eDataCode:         snprintf(sItemInfo.DataCode,     80, "%s", va_arg(args, char *));    break;
        case eDataCodeIndex:    sItemInfo.DataCodeIndex     = va_arg(args, int);  break;
        case eHide:             sItemInfo.Hide              = va_arg(args, int);  break;
        case eMax:              sItemInfo.Max               = va_arg(args, int);  break;
        case eMin:              sItemInfo.Min               = va_arg(args, int);  break;
        case eOffset:           sItemInfo.Offset            = va_arg(args, int);  break;
        case eStep:             sItemInfo.Step              = va_arg(args, int);  break;
        case eValue:            sItemInfo.Value             = va_arg(args, int);  break;

        default:
            break;
    }

    SharedMem_ImportData(eSB_UI_ITEM_INFO, &sItemInfo, index);

    va_end(args);

    return TRUE;

}

INT32 SetItemField_ByItemName(char *ItemName, eFIELD_NAME Field, ...)
{
    INT32 index = FindItemIndex_ByItemName(ItemName);

    if(index < 0 || index > ItemCount)
    {
        return FALSE;
    }

    va_list args;
    va_start(args, Field);

    sUI_ITEM_CFG sItemInfo;
    SharedMem_ReadData_New(eSB_UI_ITEM_INFO, &sItemInfo, index);  //load current info

    switch(Field)
    {
        case eItemName:         snprintf(sItemInfo.ItemName,     80, "%s", va_arg(args, char *));    break;
        case eItemID:    		sItemInfo.ItemID            = va_arg(args, long long);               break;
        case eParentMenuID:     sItemInfo.ParentMenuID      = va_arg(args, int);  break;
        case eNextMenuID:       sItemInfo.NextMenuID        = va_arg(args, int);  break;
        case eStyle:            sItemInfo.Style             = va_arg(args, int);  break;
        case eOperationType:    sItemInfo.OperationType     = va_arg(args, int);  break;
        case eDataCode:         snprintf(sItemInfo.DataCode,     80, "%s", va_arg(args, char *));    break;
        case eDataCodeIndex:    sItemInfo.DataCodeIndex     = va_arg(args, int);  break;
        case eHide:             sItemInfo.Hide              = va_arg(args, int);  break;
        case eMax:              sItemInfo.Max               = va_arg(args, int);  break;
        case eMin:              sItemInfo.Min               = va_arg(args, int);  break;
        case eOffset:           sItemInfo.Offset            = va_arg(args, int);  break;
        case eStep:             sItemInfo.Step              = va_arg(args, int);  break;
        case eValue:            sItemInfo.Value             = va_arg(args, int);  break;

        default:
            break;
    }

    SharedMem_ImportData(eSB_UI_ITEM_INFO, &sItemInfo, index);

    va_end(args);

    return TRUE;

}

//test
INT32 ItemConfig_ReadCSVfile(char *cDatacode, INT32 *iMaxValue, INT32 *iMinValue)
{
    FILE *fp = fopen(UI_CONFIG_FILE_PATH, "r");

    if(fp == NULL)
    {
        DbMsg("Error : can`t open file %s", UI_CONFIG_FILE_PATH);
        return false;
    }

    while (!feof(fp))
    {
		char cStrTemp[1024] = {'\0'};
        sUI_ITEM_CFG sItemInfo;


        char *result = fgets(cStrTemp, 1024, fp);

		if(feof(fp))
		{
			return false;
		}

        sscanf(cStrTemp, "%80[^,],%d,%d,%d,%d,%d,%80[^,],%8[^,],%d,%d,%d,%d,%d,%d"
								, sItemInfo.ItemName
								, &sItemInfo.ItemID
								, &sItemInfo.ParentMenuID
								, &sItemInfo.NextMenuID
								, &sItemInfo.Style
								, &sItemInfo.OperationType
								, sItemInfo.DataCode
								, &sItemInfo.DataCodeIndex
								, &sItemInfo.Hide
								, &sItemInfo.Max
								, &sItemInfo.Min
								, &sItemInfo.Offset
								, &sItemInfo.Step
								, &sItemInfo.Value);

		if(strncmp(sItemInfo.DataCode, cDatacode, sizeof(cDatacode)) == 0)
		{
			*iMaxValue = sItemInfo.Max;
			*iMinValue = sItemInfo.Min;
			//DbMsg("Datacode:%s, UI: max[%d] min[%d]\r\n", sItemInfo.DataCode, *iMaxValue, *iMinValue);

			return true;
		}
    }

    fclose(fp);
}


INT32 ItemConfig_SaveToCSVfile_FromShm(void)
{
    if(ItemCountCheck() == FALSE)
    {
        return FALSE;
    }

    FILE *fp = fopen(UI_CONFIG_FILE_PATH, "wb");

    if(fp == NULL)
    {
        DbMsg("Error : can`t open file %s", UI_CONFIG_FILE_PATH);
        return FALSE;
    }

    char LineData[1024] = {'\0'};

    snprintf(LineData, 1024, "%s", CSV_FILE_FIRST_LINE_STRING);
    fputs(LineData, fp);

    for(int i=0; i<ItemCount; i++)
    {
        sUI_ITEM_CFG sItemInfo;
        SharedMem_ReadData_New(eSB_UI_ITEM_INFO, &sItemInfo, i);

        snprintf(LineData, 1024, "%s,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d\r\n"
                               , sItemInfo.ItemName
                               , sItemInfo.ItemID
                               , sItemInfo.ParentMenuID
                               , sItemInfo.NextMenuID
                               , sItemInfo.Style
                               , sItemInfo.OperationType
                               , sItemInfo.DataCode
							   , sItemInfo.DataCodeIndex
                               , sItemInfo.Hide
                               , sItemInfo.Max
                               , sItemInfo.Min
                               , sItemInfo.Offset
                               , sItemInfo.Step
                               , sItemInfo.Value);

        fputs(LineData, fp);
    }

    fflush(fp);
    fclose(fp);

    return TRUE;
}

//if ItemCount == 0 , return FALSE
INT32 ItemCountCheck(void)
{
    if(ItemCount == 0)
    {
        UINT32 Line = Get_File_LineNumber(UI_CONFIG_FILE_PATH);
        if(Line <= 1)  //no data
            return FALSE;

        ItemCount = Line - 1; //skip first line
    }

    return TRUE;
}

UINT32 ItemCountGet(void)
{
    return ItemCount;
}


INT32 GetItemField(UINT32 ItemID, eFIELD_NAME Field)
{
    INT32 index = FindItemIndex_ByItemID(ItemID, FIND_FROM_SORTED_DATABASE);

    if(index == -1)
        return -1;

    void *ShmPtr = SharedMem_GetMapPtr(eSB_UI_ITEM_INFO_SORT_BY_ITEMID);

    INT32 RetValue = 0;

    switch(Field)
    {
        case eDataCodeIndex:
            {
                UINT8 Ret = 0 ;
                memcpy(&Ret, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, DataCodeIndex), sizeof(INT32));
                return (INT32)Ret;
            }

        case eHide:
            {
                UINT8 Ret = 0 ;
                memcpy(&Ret, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, Hide), 1);
                return (INT32)Ret;
            }

        case eMax:
            memcpy(&RetValue, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, Max), sizeof(INT32));
            return RetValue;

        case eMin:
            memcpy(&RetValue, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, Min), sizeof(INT32));
            return RetValue;

        case eOffset:
            memcpy(&RetValue, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, Offset), sizeof(INT32));
            return RetValue;

        case eStep:
            memcpy(&RetValue, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, Step), sizeof(INT32));
            return RetValue;

        case eValue:
            memcpy(&RetValue, (char *)ShmPtr + index*sizeof(sUI_ITEM_CFG) + offsetof(sUI_ITEM_CFG, Value), sizeof(INT32));
            return RetValue;

        default:
            return -1;
    }
}


