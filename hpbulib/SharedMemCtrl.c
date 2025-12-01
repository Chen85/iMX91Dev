#include "SharedMemCtrl.h"
#include "cmd_ap.h"

#define ASSERT_ALWAYS()           syslog(LOG_INFO, "ASSERT_ALWAYS: %s: %d\r\n", __FUNCTION__, __LINE__)
#define LOG_MSG(mask, fmt, ...)   syslog(LOG_INFO, fmt, ##__VA_ARGS__)


sSHM_PARAMETER sShmParameter[eSB_MAX_NUMBER] =
  {
      { (edcINVALID*sizeof(sDATABASE_ITEM_DATA_FORMAT)),        "DB_Item",                      -1, NULL },
      { (sDI_INVALID*sizeof(sDATABASE_INFOMATION_DATA_FORMAT)), "DB_Info",                      -1, NULL },
      { (64*1024*1024),                                         "DB_Blending_Data_0",           -1, NULL },
      { (64*1024*1024),                                         "DB_Blending_Data_1",           -1, NULL },
      { (5*1024*1024),                                          "UI_ITEM_INFO",                 -1, NULL },  //UI Item Config
      { (5*1024*1024),                                          "UI_ITEM_INFO_SortByItemID",    -1, NULL },  //UI Item Config sort by ItemID
      { (5*1024*1024),                                          "UI_ITEM_INFO_SortByDataCode",  -1, NULL },  //UI Item Config sort by Datacode
      { (sizeof(sPROCESS_MUTEX_DATA)),                          "ProcessMutexData",             -1, NULL },  //for Scaler(C821/ProAV) process mutex (On Linux, a pthread_mutex_t occupies 24 bytes on 32-bit machines and 40 bytes on 64-bit machines.)
      { (sizeof(sITEM_CONTROL_TABLE)*eacINVALID),               "ItemControlTable",             -1, NULL },  //gray out condition (Item Control selected by OSD TOOL)
      { (edcINVALID*sizeof(sDATACODE_INFO)),                    "DatacodeInfo",                 -1, NULL },  //for Datacode info (Value Max , Min , Step)
      { (5*1024*1024),                                          "CommonData",                   -1, NULL }   //Common Data (Model Name...)
  };


int lSharedMemory_Reader_fd[eSB_MAX_NUMBER] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

int SharedMem_Create(void)
{
    for(unsigned char x = 0 ; x < eSB_MAX_NUMBER ; x++ )
    {
        /* create the shared memory object */
        sShmParameter[x].lSharedMemory_fd = shm_open(sShmParameter[x].cSharedMemory_Name, O_CREAT | O_RDWR, 0666);

        if(sShmParameter[x].lSharedMemory_fd < 0)  //shm_open()  return -1 if fail
        {
            ASSERT_ALWAYS();
            return -1;
        }

        /* configure the size of the shared memory object */
        if(ftruncate(sShmParameter[x].lSharedMemory_fd, sShmParameter[x].lSharedMemory_Size) == -1)
        {
            ASSERT_ALWAYS();
            return -1;
        }

        /* memory map the shared memory object */
        sShmParameter[x].ptr_shm = mmap(0, sShmParameter[x].lSharedMemory_Size, PROT_WRITE, MAP_SHARED, sShmParameter[x].lSharedMemory_fd, 0);

        memset(sShmParameter[x].ptr_shm ,0 , sShmParameter[x].lSharedMemory_Size);

        LOG_MSG(0, "Shared Memory [%s] created, fd [%d], Addr [%p]\n" , sShmParameter[x].cSharedMemory_Name , sShmParameter[x].lSharedMemory_fd, sShmParameter[x].ptr_shm);
    }

    return 1;
}

void* SharedMem_CreatorMappingPtrGet(eSHARED_MEMORY_BLOCK eDatabase)
{
//    LOG_MSG(0, "get <<%p>>\n", sShmParameter[eDatabase].ptr_shm);
//    LOG_MSG(0, "get <<%d>>\n", sShmParameter[eDatabase].lSharedMemory_Size);
//    LOG_MSG(0, "get <<%s>>\n", sShmParameter[eDatabase].cSharedMemory_Name);

    return (void *)sShmParameter[eDatabase].ptr_shm;
}


int SharedMem_ImportData(eSHARED_MEMORY_BLOCK eDatabase, void *Data, unsigned int ulIndex)
{
    /* check database is already exist */
    if(sShmParameter[eDatabase].lSharedMemory_fd < 0 ||
       sShmParameter[eDatabase].ptr_shm == NULL ||
       sShmParameter[eDatabase].ptr_shm == MAP_FAILED)
    {
        ASSERT_ALWAYS();
        return -1;
    }

    unsigned int ulOffset ;
    unsigned int ulSize ;
    switch(eDatabase)
    {
        case eSB_DATABASE_ITEM:
            if(ulIndex >= edcINVALID)
            {
                printf("Data Code %d Over flow, INVALID = %d\n", ulIndex, edcINVALID);
                ASSERT_ALWAYS();
                return -1;
            }
            ulSize = sizeof(sDATABASE_ITEM_DATA_FORMAT);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_DATABASE_INFOMATION:
            ulSize = sizeof(sDATABASE_ITEM_DATA_FORMAT);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_DATABASE_BLENDING_DATA_0:
        case eSB_DATABASE_BLENDING_DATA_1:
            ulSize = sizeof(sSPRITE_INFO_DATA);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_UI_ITEM_INFO:
        case eSB_UI_ITEM_INFO_SORT_BY_ITEMID:
        case eSB_UI_ITEM_INFO_SORT_BY_DATACODE:
            ulSize = sizeof(sUI_ITEM_CFG);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_PROCESS_MUTEX_DATA:
            ulSize = sizeof(sPROCESS_MUTEX_DATA);
            ulOffset = 0;
            break;

        case eSB_ITEM_CONTROL_TABLE:
            ulSize = sizeof(sITEM_CONTROL_TABLE);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_DATACODE_INFO:
            ulSize = sizeof(sDATACODE_INFO);
            ulOffset = ulIndex * ulSize;
            break;

        default:
            return -1;
    }

    flock(sShmParameter[eDatabase].lSharedMemory_fd, LOCK_EX);
    memcpy( ((char *)sShmParameter[eDatabase].ptr_shm)+ulOffset, Data, ulSize);
    msync( ((char *)sShmParameter[eDatabase].ptr_shm)+ulOffset, ulSize, MS_SYNC);
    flock(sShmParameter[eDatabase].lSharedMemory_fd, LOCK_UN);

    return 1;
}


//////////////////////////////////////////////////////////////////
//old code, please use "SharedMem_ReaderInit_New" function
//////////////////////////////////////////////////////////////////
void* SharedMem_ReaderInit(eSHARED_MEMORY_BLOCK eDatabase)
{
    /* Opening an existing shared memory object , and return file descriptor*/
    lSharedMemory_Reader_fd[eDatabase] = shm_open(sShmParameter[eDatabase].cSharedMemory_Name, O_RDONLY, 0666);

    if(lSharedMemory_Reader_fd[eDatabase] == -1)
    {
        ASSERT_ALWAYS();
        return NULL;
    }

    LOG_MSG(0, "get Shared Memory file descriptor [%d]\n" , lSharedMemory_Reader_fd[eDatabase]);

    /* get memory map the shared memory object */
    void *shm_ptr = mmap(0, sShmParameter[eDatabase].lSharedMemory_Size, PROT_READ, MAP_SHARED, lSharedMemory_Reader_fd[eDatabase], 0);

    LOG_MSG(0, "get Shared Memory map address [%p]\n" , shm_ptr);

    return shm_ptr;

}


//////////////////////////////////////////////////////////////////
//old code (only for datacode), please use "SharedMem_ReadData_New" function
//////////////////////////////////////////////////////////////////
int SharedMem_ReadData(void *shm_ptr, void *Data, unsigned int ulIndex)
{
    if(shm_ptr == NULL)
    {
        ASSERT_ALWAYS();
        return -1;
    }

    unsigned int ulOffset ;
    unsigned int ulSize ;

    ulSize = sizeof(sDATABASE_ITEM_DATA_FORMAT);
    ulOffset = ulIndex * ulSize;

    memcpy(Data, shm_ptr+(ulOffset) , ulSize);

    return 1;
}


void* SharedMem_ReaderInit_New(eSHARED_MEMORY_BLOCK eDatabase)
{
    /* Opening an existing shared memory object , and return file descriptor*/
    sShmParameter[eDatabase].lSharedMemory_fd = shm_open(sShmParameter[eDatabase].cSharedMemory_Name, O_RDONLY, 0666);

    if(sShmParameter[eDatabase].lSharedMemory_fd < 0)
    {
        ASSERT_ALWAYS();
        return NULL;
    }

    LOG_MSG(0, "get Shared Memory file descriptor [%d]\n" , sShmParameter[eDatabase].lSharedMemory_fd);

    /* get memory map the shared memory object */
    sShmParameter[eDatabase].ptr_shm = mmap(0, sShmParameter[eDatabase].lSharedMemory_Size, PROT_READ, MAP_SHARED, sShmParameter[eDatabase].lSharedMemory_fd, 0);

    if(sShmParameter[eDatabase].ptr_shm == MAP_FAILED)
    {
        ASSERT_ALWAYS();
        return NULL;
    }

    LOG_MSG(0, "get Shared Memory map address [%p]\n" , sShmParameter[eDatabase].ptr_shm);

    return sShmParameter[eDatabase].ptr_shm;

}

int SharedMem_ReadData_New(eSHARED_MEMORY_BLOCK eDatabase, void *Data, unsigned int ulIndex)
{
    /* check database is already exist */
    if(sShmParameter[eDatabase].lSharedMemory_fd < 0 ||
       sShmParameter[eDatabase].ptr_shm == NULL ||
       sShmParameter[eDatabase].ptr_shm == MAP_FAILED )
    {
        ASSERT_ALWAYS();
        LOG_MSG(0, "SHM ASSERT %d <%d 0x%p>\n", eDatabase, sShmParameter[eDatabase].lSharedMemory_fd, sShmParameter[eDatabase].ptr_shm);
        return -1;
    }

    unsigned int ulOffset ;
    unsigned int ulSize ;
    switch(eDatabase)
    {
        case eSB_DATABASE_ITEM:
            ulSize = sizeof(sDATABASE_ITEM_DATA_FORMAT);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_DATABASE_INFOMATION:
            ulSize = sizeof(sDATABASE_ITEM_DATA_FORMAT);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_DATABASE_BLENDING_DATA_0:
        case eSB_DATABASE_BLENDING_DATA_1:
            ulSize = sizeof(sSPRITE_INFO_DATA);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_UI_ITEM_INFO:
        case eSB_UI_ITEM_INFO_SORT_BY_ITEMID:
        case eSB_UI_ITEM_INFO_SORT_BY_DATACODE:
            ulSize = sizeof(sUI_ITEM_CFG);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_PROCESS_MUTEX_DATA:
            ulSize = sizeof(sPROCESS_MUTEX_DATA);
            ulOffset = 0;
            break;

        case eSB_ITEM_CONTROL_TABLE:
            ulSize = sizeof(sITEM_CONTROL_TABLE);
            ulOffset = ulIndex * ulSize;
            break;

        case eSB_DATACODE_INFO:
            ulSize = sizeof(sDATACODE_INFO);
            ulOffset = ulIndex * ulSize;
            break;

        default:
            {
                ASSERT_ALWAYS();
                LOG_MSG(0, "undefined DB %d\n", eDatabase);
                return -1;
            }
    }

    flock(sShmParameter[eDatabase].lSharedMemory_fd, LOCK_EX);
    memcpy(Data, sShmParameter[eDatabase].ptr_shm +(ulOffset) , ulSize);
    flock(sShmParameter[eDatabase].lSharedMemory_fd, LOCK_UN);

    return 1;
}


void* SharedMem_WriterInit(eSHARED_MEMORY_BLOCK eDatabase)
{
    //LOG_MSG(0, " get Shared Memory file descriptor [%d]\n" , sShmParameter[eDatabase].lSharedMemory_fd);
    //LOG_MSG(0, "get Shared Memory map ptr [%p]\n" , sShmParameter[eDatabase].ptr_shm);

    if(sShmParameter[eDatabase].lSharedMemory_fd >= 0)  //already init
    {
        return NULL;
    }

    /* Opening an existing shared memory object , and return file descriptor*/
    sShmParameter[eDatabase].lSharedMemory_fd = shm_open(sShmParameter[eDatabase].cSharedMemory_Name, O_RDWR, 0666);

    LOG_MSG(0, "W get Shared Memory file descriptor [%d]\n" , sShmParameter[eDatabase].lSharedMemory_fd);

    if(sShmParameter[eDatabase].lSharedMemory_fd < 0)
    {
        ASSERT_ALWAYS();
        return NULL;
    }

    /* get memory map the shared memory object */
    sShmParameter[eDatabase].ptr_shm = mmap(0, sShmParameter[eDatabase].lSharedMemory_Size, PROT_READ|PROT_WRITE, MAP_SHARED, sShmParameter[eDatabase].lSharedMemory_fd, 0);

    if(sShmParameter[eDatabase].ptr_shm == MAP_FAILED)
    {
        ASSERT_ALWAYS();
        return NULL;
    }

    LOG_MSG(0, "get Shared Memory map address [%p]\n" , sShmParameter[eDatabase].ptr_shm);

    return sShmParameter[eDatabase].ptr_shm;

}

int SharedMem_GetFd(eSHARED_MEMORY_BLOCK eDatabase)
{
    return sShmParameter[eDatabase].lSharedMemory_fd ;
}

void* SharedMem_GetMapPtr(eSHARED_MEMORY_BLOCK eDatabase)
{
    return sShmParameter[eDatabase].ptr_shm ;
}

int SharedMem_GetSize(eSHARED_MEMORY_BLOCK eDatabase)
{
    return sShmParameter[eDatabase].lSharedMemory_Size ;
}


