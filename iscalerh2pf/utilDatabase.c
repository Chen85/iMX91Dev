#include "utilDbgMsg.h"
#include "utilDatabaseAPI.h"
#include "utilIPCAPI.h"
#include "Common.h"
#include "cmd_ap.h"

void *shm_ptr_item = NULL;
void *shm_ptr_infomation = NULL;
void *shm_ptr_BlendingData_0 = NULL;
void *shm_ptr_BlendingData_1 = NULL;
void *shm_ptr_UI_item_info = NULL;
void *shm_ptr_ProcessMutexData = NULL;
void *shm_ptr_ItemControl = NULL;
void *shm_ptr_DatacodeInfo = NULL;

///////////////// callback function start ///////////////
sUTILDATABASE_CALLBACK sUtilDatabase_Callback;
void utilDatabase_RegCallback(sUTILDATABASE_CALLBACK fpCallback)
{
    sUtilDatabase_Callback = fpCallback;
}
////////////////// callback function end ////////////////


INT32 utilDatabase_Init(void)
{
    if(SharedMem_Create() == -1)
    {
        ASSERT_ALWAYS();
        return DATABASE_ACCESS_FAIL;
    }

    shm_ptr_item             = SharedMem_CreatorMappingPtrGet(eSB_DATABASE_ITEM);
    shm_ptr_infomation       = SharedMem_CreatorMappingPtrGet(eSB_DATABASE_INFOMATION);
    shm_ptr_BlendingData_0   = SharedMem_CreatorMappingPtrGet(eSB_DATABASE_BLENDING_DATA_0);
    shm_ptr_BlendingData_1   = SharedMem_CreatorMappingPtrGet(eSB_DATABASE_BLENDING_DATA_1);
    shm_ptr_UI_item_info     = SharedMem_CreatorMappingPtrGet(eSB_UI_ITEM_INFO);
    shm_ptr_ProcessMutexData = SharedMem_CreatorMappingPtrGet(eSB_PROCESS_MUTEX_DATA);
    shm_ptr_ItemControl      = SharedMem_CreatorMappingPtrGet(eSB_ITEM_CONTROL_TABLE);
    shm_ptr_DatacodeInfo     = SharedMem_CreatorMappingPtrGet(eSB_DATACODE_INFO);

    //printf("<< shm_ptr_item %p >>\n", shm_ptr_item );
    //printf("<< shm_ptr_infomation %p >>\n", shm_ptr_infomation );
    //printf("<< shm_ptr_BlendingData_0 %p >>\n", shm_ptr_BlendingData_0 );
    //printf("<< shm_ptr_BlendingData_1 %p >>\n", shm_ptr_BlendingData_1 );

    return DATABASE_ACCESS_PASS;
}

INT32 utilDatabase_WriteData(eSHARED_MEMORY_BLOCK ucDatabaseIndex, void *Data, UINT32 ulIndex)
{
    if(SharedMem_ImportData(ucDatabaseIndex, Data, ulIndex) != TRUE)
    {
        return DATABASE_ACCESS_FAIL;
    }

    return DATABASE_ACCESS_PASS;
}

INT32 utilDatabase_ReadData(eSHARED_MEMORY_BLOCK ucDatabaseIndex, void *Data, UINT32 ulIndex)
{
    switch(ucDatabaseIndex)
    {
        case eSB_DATABASE_ITEM:
            memcpy(Data, (char *)shm_ptr_item + (sizeof(sDATABASE_ITEM_DATA_FORMAT) * ulIndex), sizeof(sDATABASE_ITEM_DATA_FORMAT));
            return TRUE;

        case eSB_DATABASE_INFOMATION:
            memcpy(Data, (char *)shm_ptr_infomation + (sizeof(sDATABASE_ITEM_DATA_FORMAT) * ulIndex), sizeof(sDATABASE_ITEM_DATA_FORMAT));
            return TRUE;

        case eSB_DATABASE_BLENDING_DATA_0:
            memcpy(Data, (char *)shm_ptr_BlendingData_0 + (sizeof(sSPRITE_INFO_DATA) * ulIndex), sizeof(sSPRITE_INFO_DATA));
            return TRUE;

        case eSB_DATABASE_BLENDING_DATA_1:
            memcpy(Data, (char *)shm_ptr_BlendingData_1 + (sizeof(sSPRITE_INFO_DATA) * ulIndex), sizeof(sSPRITE_INFO_DATA));
            return TRUE;

        case eSB_ITEM_CONTROL_TABLE:
            memcpy(Data, (char *)shm_ptr_ItemControl + (sizeof(sITEM_CONTROL_TABLE) * ulIndex), sizeof(sITEM_CONTROL_TABLE));
            return TRUE;

        case eSB_DATACODE_INFO:
            memcpy(Data, (char *)shm_ptr_DatacodeInfo + (sizeof(sDATACODE_INFO) * ulIndex), sizeof(sDATACODE_INFO));
            return TRUE;

        default:
            return DATABASE_ACCESS_FAIL;
    }

    return DATABASE_ACCESS_PASS;
}


INT32 utilDatabase_WriteInformationData(UINT32 ulIndex, UINT8 ucType, UINT8 ucGrayoutStatus, void *Data)
{
    sDATABASE_ITEM_DATA_FORMAT sWriteData ;

    sWriteData.ucType = ucType;
    sWriteData.ucGrayoutStatus = ucGrayoutStatus;
    sWriteData.uiItemIndex = ulIndex;
    sWriteData.uiValueSize = sizeof(INT32);

    switch(ucType)
    {
        case DATA_TYPE_STRING:
            snprintf((char *)sWriteData.uValue.acString, 255, "%s", (char *)Data);
            break;

        case DATA_TYPE_UI_DIGIT_8:
            sWriteData.uValue.lValue = (INT32)(*((UINT8 *)Data));
            break;

        case DATA_TYPE_UI_DIGIT_16:
            sWriteData.uValue.lValue = (INT32)(*((UINT16 *)Data));
            break;

        case DATA_TYPE_UI_DIGIT_32:
            sWriteData.uValue.lValue = (INT32)(*((UINT32 *)Data));
            break;

        case DATA_TYPE_I_DIGIT_8:
            sWriteData.uValue.lValue = (INT32)(*((INT8 *)Data));
            break;

        case DATA_TYPE_I_DIGIT_16:
            sWriteData.uValue.lValue = (INT32)(*((INT16 *)Data));
            break;

        case DATA_TYPE_I_DIGIT_32:
            sWriteData.uValue.lValue = (INT32)(*((INT32 *)Data));
            break;

        case DATA_TYPE_FLOAT:
            sWriteData.uValue.lValue = (INT32)(*((FLOAT *)Data));
            break;

        default:
            LOG_MSG(db_UTL_DATABASE, "%s (%d) error\n", __FUNCTION__, ucType);
            return DATABASE_ACCESS_FAIL;
    }

    LOG_MSG(db_UTL_DATABASE, "%s %d (%d)\n", __FUNCTION__, ulIndex, sWriteData.uValue.lValue);

    if(SharedMem_ImportData(eSB_DATABASE_INFOMATION, &sWriteData, ulIndex) != TRUE)
    {
        return DATABASE_ACCESS_FAIL;
    }

    //for notify
    UINT16 auiUpdateInfoItem[1] = {(UINT16)ulIndex};
    utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, 0, auiUpdateInfoItem, eEXEC_CODE_PASS, sizeof(UINT16)*1);    //A35G2_CDS_Simon_0011

    return DATABASE_ACCESS_PASS;
}



void utilDatabase_ItemPrint(void)
{
    LOG_MSG(db_ALWAYS, "Item shm ptr = %p\n", shm_ptr_item);

    for(UINT16 Index = 0 ; Index < edcINVALID ; Index++ )
    {
        sDATABASE_ITEM_DATA_FORMAT DATA;
        memcpy((void *)&DATA, (void *)shm_ptr_item + (sizeof(sDATABASE_ITEM_DATA_FORMAT) * Index), sizeof(sDATABASE_ITEM_DATA_FORMAT));  //G100_Simon_0031
        if(DATA.ucType == DATA_TYPE_STRING)
        {
            LOG_MSG(db_ALWAYS, "%s -> %d %d %d %d %s\n" , sUtilDatabase_Callback.fpDataMgr_DataCodeStringGetCb(Index), DATA.ucType, DATA.ucGrayoutStatus, DATA.uiItemIndex, DATA.uiValueSize, DATA.uValue.acString);
        }
        else if(DATA.ucType == DATA_TYPE_FLOAT)
        {
            LOG_MSG(db_ALWAYS, "%s -> %d %d %d %d %f\n" , sUtilDatabase_Callback.fpDataMgr_DataCodeStringGetCb(Index), DATA.ucType, DATA.ucGrayoutStatus, DATA.uiItemIndex, DATA.uiValueSize, DATA.uValue.lValue);
        }
        else if(DATA.ucType == DATA_TYPE_STRUCT)
        {
            LOG_MSG(db_ALWAYS, "%s -> %d %d %d %d \n"   , sUtilDatabase_Callback.fpDataMgr_DataCodeStringGetCb(Index), DATA.ucType, DATA.ucGrayoutStatus, DATA.uiItemIndex, DATA.uiValueSize);
            for(INT32 i=0; i<DATA.uiValueSize; i++)
            {
                LOG_MSG(db_ALWAYS, "%02X ", DATA.uValue.acString[i]);
            }
        }
        else
        {
            LOG_MSG(db_ALWAYS, "%s -> %d %d %d %d %d\n" , sUtilDatabase_Callback.fpDataMgr_DataCodeStringGetCb(Index), DATA.ucType, DATA.ucGrayoutStatus, DATA.uiItemIndex, DATA.uiValueSize, DATA.uValue.lValue);
        }

        #ifdef APP_BACKGROUND
        MS_SLEEP(6);
        #else
        MS_SLEEP(3);
        #endif
    }
}


void utilDatabase_InformationPrint(void)
{
    LOG_MSG(db_ALWAYS, "Information shm ptr = %p\n", shm_ptr_infomation);

    LOG_MSG(db_ALWAYS, "Index -> ucType GrayoutStatus ItemIndex Size String\n");

    for(UINT16 Index = 0 ; Index < sDI_INVALID ; Index++ )
    {
        //LOG_MSG(db_ALWAYS, "Information shm ptr = %p\n", (char *)shm_ptr_infomation+(sizeof(sDATABASE_INFOMATION_DATA_FORMAT) * Index));

        sDATABASE_INFOMATION_DATA_FORMAT DATA;
        memcpy((void *)&DATA, (char *)shm_ptr_infomation+(sizeof(sDATABASE_INFOMATION_DATA_FORMAT) * Index), sizeof(sDATABASE_INFOMATION_DATA_FORMAT));
        if(DATA.ucType == DATA_TYPE_STRING)
        {
            LOG_MSG(db_ALWAYS, "%d -> %d %d %d %d %s\n" , Index , DATA.ucType , DATA.ucGrayoutStatus, DATA.uiItemIndex, DATA.uiValueSize, DATA.uValue.acString);
        }
        else
        {
            LOG_MSG(db_ALWAYS, "%d -> %d %d %d %d %d\n" , Index , DATA.ucType , DATA.ucGrayoutStatus, DATA.uiItemIndex, DATA.uiValueSize, DATA.uValue.lValue);
        }

        MS_SLEEP(3);
    }
}


