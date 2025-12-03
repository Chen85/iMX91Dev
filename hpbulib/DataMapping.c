#include "DataMapping.h"

sDATA_MAPPING_LUT_FORMAT *sDataMapLut = NULL;
sDATA_MAPPING_LUT_FORMAT *sDataMapLutTmp = NULL;    //for realloc
INT32 lDataMapLutSize = 0 ;
eDEBUGMODE_TYPE ucDebugMode = eDEBUG_FATAL_ERROR ;

#include <stdint.h>

//shared memory config
#define SHM_NAME (const char *)"SHM_DataMappingLut"
INT32 lShmSize = 2*1024*1024;

INT32 ShmFd = -1;   //shared memory file descriptor
INT32 *pShmPtr = NULL;   //shared memory mmap pointer

//INT32 ShmReadFd = -1;   //shared memory file descriptor

INT32 ShmDataMappingTableReady = 0;

#define SHM_ONE_MPT_SIZE 1024
#define SHM_MPT_DATA_OFFSET_START (1*1024*1024)
#define SHM_DATAMAPLUT_SIZE_OFFSET_START (lShmSize-8)
#define SHM_DATAMAPLUT_READY_FLAG_OFFSET_START (lShmSize-4)

#define SHM_DATAMAPLUT_READY_FLAG_MAGIC_NUM (0x1234ABCD)
/////////////////////////////


/////////////////////////////////////////////////////////////////////////////
INT32 DataMapping_Init(sDATA_MAPPING_LUT_FORMAT *pSystemDataMapLut, INT32 lSize)
{
    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("DataMapping_Init Start\n");
    }

    if(sDataMapLut == NULL)
    {
        sDataMapLut = (sDATA_MAPPING_LUT_FORMAT *)malloc(sizeof(sDATA_MAPPING_LUT_FORMAT) * lSize);
    }
    else
    {
        sDataMapLutTmp = (sDATA_MAPPING_LUT_FORMAT *)realloc(sDataMapLut, sizeof(sDATA_MAPPING_LUT_FORMAT) * lSize);

        if(sDataMapLutTmp != NULL)
        {
            sDataMapLut = sDataMapLutTmp;
        }
        else
        {
            printf("\n\n!!! DATA_MAP_INIT_FAIL (realloc fail) !!!\n\n");
            free(sDataMapLut);
            return DATA_MAP_INIT_FAIL;
        }
    }

    if(sDataMapLut == NULL)
    {
        if(ucDebugMode >= eDEBUG_FATAL_ERROR)
        {
            printf("\n\n!!! DATA_MAP_INIT_FAIL !!!\n\n");
        }

        return DATA_MAP_INIT_FAIL;
    }

    lDataMapLutSize = lSize;

    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("lDataMapLutSize = %d\n", lDataMapLutSize);
    }

    memset(sDataMapLut, 0, (sizeof(sDATA_MAPPING_LUT_FORMAT) * lSize));
    for(INT32 i = 0 ; i < lSize ; i++)
    {
        memcpy( (sDataMapLut + i) , (pSystemDataMapLut + i) , sizeof(sDATA_MAPPING_LUT_FORMAT) );
    }

    if(DataMapping_ImportSharedMemory() != 1)   //save sDataMapLut to shared memory
    {
        printf("\n\n!!!  DataMapping_ImportSharedMemory Fail  !!!\n\n");
        return DATA_MAP_INIT_FAIL;
    }

    DataMapping_ReadyFlagSet(1);

    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("DataMapping_Init End\n");
    }

    #if 0  //for debug
    {
        FILE *fp;
        UINT32 ulSize = lSize * sizeof(sDATA_MAPPING_LUT_FORMAT);
        fp = fopen( "/tmp/sDataMapLut.dat" , "wb" );
        fwrite(sDataMapLut , 1 , ulSize , fp);
        fclose(fp);

        printf("\nsDataMapLut ulSize = %d\n\n" , ulSize);
    }
    #endif

    return DATA_MAP_PASS;

}

INT32 DataMapping_DeInit(void)
{
    if(sDataMapLut != NULL)
    {
        free(sDataMapLut);
        sDataMapLut = NULL;

        lDataMapLutSize = 0 ;
    }
}

INT32 DataMapping_ImportSharedMemory(void)
{
    /* create the shared memory object */
    ShmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if(ShmFd == -1)  //shm_open()  return -1 if fail
    {
        printf("!! ShmFd == -1 \n");
        return -1;
    }

    /* configure the size of the shared memory object */
    if(ftruncate(ShmFd, lShmSize) == -1)
    {
        printf("!! ShmFd ftruncate fail \n");
        return -1;
    }

    /* memory map the shared memory object */
    pShmPtr = (INT32 *)mmap(0, lShmSize, PROT_WRITE, MAP_SHARED, ShmFd, 0);

    /* save sDataMapLut to shared memory*/
    flock(ShmFd, LOCK_EX);
    memset(pShmPtr, 0, lShmSize);
    memcpy(pShmPtr, sDataMapLut, lDataMapLutSize*sizeof(sDATA_MAPPING_LUT_FORMAT));
    memcpy((INT8 *)pShmPtr+SHM_DATAMAPLUT_SIZE_OFFSET_START, (INT8 *)&lDataMapLutSize, 4);    //save lDataMapLutSize to last 4 bytes of shared memory
    flock(ShmFd, LOCK_UN);

    /* save all psMPT (at sDataMapLut) to shared memory*/
    flock(ShmFd, LOCK_EX);
    for(int index=0 ; index<lDataMapLutSize ; index++)
    {
        uintptr_t mpt_addr = *((uintptr_t *)((INT8 *)(sDataMapLut + index) + offsetof(sDATA_MAPPING_LUT_FORMAT, psMPT)));
        int *Current_MPT_Start_Addr = (INT32 *)mpt_addr;
        int Current_MPT_Total_Bytes = (*(INT32 *)((INT8 *)(sDataMapLut + index) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size))) * eMPT_TYPE_NUMBER * sizeof(INT32);
        memcpy((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(index*SHM_ONE_MPT_SIZE), Current_MPT_Start_Addr, Current_MPT_Total_Bytes);

        if(ucDebugMode >= eDEBUG_LEVEL2)
        {
            printf("Current_MPT_Start_Addr 0x%p\n", Current_MPT_Start_Addr);
            printf("Current_MPT_Total_Bytes %d\n", Current_MPT_Total_Bytes);
            printf("sa 0x%p\n", (INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(index*SHM_ONE_MPT_SIZE));
        }
    }
    flock(ShmFd, LOCK_UN);

    return 1;

}


INT32 DataMapping_ReaderInit(void)
{
    ShmFd = shm_open(SHM_NAME, O_RDONLY, 0666);

    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("DataMapping_ReaderInit Start\n");
    }

    /* memory map the shared memory object */
    pShmPtr = (INT32 *)mmap(0, lShmSize, PROT_READ, MAP_SHARED, ShmFd, 0);

    //retry if not ready
    INT8 Retry = 50;
    while(ShmFd == -1 || DataMapping_ReadyFlagGet() != 1)
    {
        if(Retry-- <= 0)
        {
            printf("DataMapping_ReaderInit Fail (%d %d)\n", ShmFd, DataMapping_ReadyFlagGet());
            return DATA_MAP_NOT_READY;
        }

        usleep(200 * 1000);

        ShmFd = shm_open(SHM_NAME, O_RDONLY, 0666);
        pShmPtr = (INT32 *)mmap(0, lShmSize, PROT_READ, MAP_SHARED, ShmFd, 0);
    }

    //load lDataMapLutSize
    memcpy((INT8 *)&lDataMapLutSize, (INT8 *)pShmPtr+SHM_DATAMAPLUT_SIZE_OFFSET_START, 4);
    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("lDataMapLutSize = %d , pShmPtr %p\n", lDataMapLutSize, pShmPtr);
    }

    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("DataMapping_ReaderInit End\n");
    }

    //load sDataMapLut from shared memory
    #if 0
    if(sDataMapLut == NULL)
    {
        sDataMapLut = (sDATA_MAPPING_LUT_FORMAT *)malloc(lShmSize);
    }
    memcpy((INT8 *)sDataMapLut, (INT8 *)pShmPtr, lShmSize);
    printf("sDataMapLut = 0x%p\n", sDataMapLut);
    #endif

    return 1;
}


INT32 DataMapping_Transform(eMPT_TYPE eType_Before, eMPT_TYPE eType_After, eDATA_CODE eDataCode, INT32 lValue, char *cFuncionName, INT32 llineNum)
{
    INT32 lMappedValue = MPT_INDEX_INVALID;

    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("<<%s %d (%d->%d) %d %d>>\n", cFuncionName, llineNum, eType_Before, eType_After, eDataCode, lValue);
    }

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        return LUT_INDEX_INVALID;
    }

	if(eType_Before == eType_After) //HICC2_Doulas_0003
	{
		return lValue;
	}

    // Get Mapping Table
    //INT32 *pMPT = (INT32 *)*((INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, psMPT))) ;
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    //INT32 lTableSize = *(INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Mapped Value
    INT32 Index = 0;
    while(Index < lTableSize)
    {
        if(ucDebugMode >= eDEBUG_LEVEL2)
        {
            printf("(%d)(%d %d)\n" , eDataCode, *(pMPT + (eMPT_TYPE_NUMBER*Index + eType_Before)) , lValue);
        }

        if( *(pMPT + (eMPT_TYPE_NUMBER*Index + eType_Before)) == lValue )
        {
            lMappedValue = *(pMPT + (eMPT_TYPE_NUMBER*Index + eType_After) );
            break;
        }

        Index++;
    }

    // Check
    if(lMappedValue == MPT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: from %s %d Datacode(%d) %d (%d->%d)\n", cFuncionName , llineNum , eDataCode , lValue , eType_Before , eType_After);
        }

        return MPT_INDEX_INVALID;
    }

    //printf("S: %s Datacode(%d) %d => %d\n", __FUNCTION__ , eDataCode , lValue , lMappedValue);

    return lMappedValue;
}


////////////////////////////////////////////////////////
INT32 DataMapping_GetDataMappingLutIndex(eDATA_CODE DataCode)
{
    INT32 LutIndex = 0;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    while(LutIndex < lDataMapLutSize)
    {
        if(*((INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, eDataCode))) == DataCode)
        {
            return LutIndex ;
        }

        LutIndex++;
    }

    return LUT_INDEX_INVALID;
}

////////////////////////////////////////////////////////
//return Next CM Value
INT32 DataMapping_GetNextCMvalueFromGui(eDATA_CODE eDataCode, INT32 lCMCurrentValue)
{
    INT32 lNextCMValue = MPT_INDEX_INVALID;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: LutIndex == %d %s Datacode(%d) %d\n", LUT_INDEX_INVALID, __FUNCTION__ , eDataCode , lCMCurrentValue);
        }

        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    //INT32 *pMPT = (INT32 *)*((INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, psMPT))) ;
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    //INT32 lTableSize = *(INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Next Value
    INT32 Index = 0;
    while(Index < lTableSize)
    {
        if( *(pMPT + (eMPT_TYPE_NUMBER*Index + eMPT_TYPE_CM)) == lCMCurrentValue )
        {
            INT32 lGuiValue = *( pMPT + (eMPT_TYPE_NUMBER*Index + eMPT_TYPE_GUI) );
            INT32 lNextGuiValue = (lGuiValue + 1) % (DataMapping_GUI_TotalNum_Get(eDataCode));

            INT32 lNextGuiIndex = (++Index) % lTableSize;
            while( *( pMPT + (eMPT_TYPE_NUMBER*lNextGuiIndex + eMPT_TYPE_GUI)) != lNextGuiValue )
            {
                lNextGuiIndex = (++Index) % lTableSize;

                //all not found
                if(*( pMPT + (eMPT_TYPE_NUMBER*lNextGuiIndex + eMPT_TYPE_GUI)) == lGuiValue)
                {
                    printf("%s not found (Datacode %d, lCMCurrentValue %d)\n", __FUNCTION__, eDataCode, lCMCurrentValue);
                    break;
                }
            }

            lNextCMValue = *( pMPT + (eMPT_TYPE_NUMBER*lNextGuiIndex + eMPT_TYPE_CM) );

            break;
        }

        Index++;
    }

    //printf("S: %s Datacode(%d) %d => %d\n", __FUNCTION__ , eDataCode , lCurrentValue , lNextCMValue);

    return lNextCMValue;
}

////////////////////////////////////////////////////////
//return Previous CM value
INT32 DataMapping_GetPreCMvalueFromGui(eDATA_CODE eDataCode, INT32 lCMCurrentValue)
{
    INT32 lPreviousCMValue = MPT_INDEX_INVALID;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: LutIndex == %d %s Datacode(%d) %d\n", LUT_INDEX_INVALID, __FUNCTION__ , eDataCode , lCMCurrentValue);
        }

        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    //INT32 *pMPT = (INT32 *)*((INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, psMPT))) ;
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    //INT32 lTableSize = *(INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Previous Value
    INT32 Index = 0;
    while(Index < lTableSize)
    {
        if( *(pMPT + (eMPT_TYPE_NUMBER*Index + eMPT_TYPE_CM)) == lCMCurrentValue )
        {
            INT32 lGuiValue = *( pMPT + (eMPT_TYPE_NUMBER*Index + eMPT_TYPE_GUI) );

            INT32 lPreGuiValue = 0;
            lPreGuiValue = (lGuiValue <= 0) ? (DataMapping_GUI_TotalNum_Get(eDataCode) - 1) : (lGuiValue - 1) ;

            INT32 lPreGuiIndex = (Index <= 0) ? (lTableSize - 1) : (Index - 1) ;
            while( *( pMPT + (eMPT_TYPE_NUMBER*lPreGuiIndex + eMPT_TYPE_GUI)) != lPreGuiValue )
            {
                lPreGuiIndex = (Index <= 0) ? (lTableSize - 1) : (Index - 1) ;

                //all not found
                if(*( pMPT + (eMPT_TYPE_NUMBER*lPreGuiIndex + eMPT_TYPE_GUI)) == lGuiValue)
                {
                    printf("%s not found (Datacode %d, lCMCurrentValue %d)\n", __FUNCTION__, eDataCode, lCMCurrentValue);
                    break;
                }
            }

            lPreviousCMValue = *( pMPT + (eMPT_TYPE_NUMBER*lPreGuiIndex + eMPT_TYPE_CM) );

            break;
        }

        Index++;
    }

    //printf("S: %s Datacode(%d) %d => %d\n", __FUNCTION__ , eDataCode , lCurrentValue , lPreviousCMValue);

    return lPreviousCMValue;
}

////////////////////////////////////////////////////////
INT32 DataMapping_GetMaxCMvalueFromGui(eDATA_CODE eDataCode)
{
    INT32 lPreviousCMValue = MPT_INDEX_INVALID;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: LutIndex == %d %s Datacode(%d)\n", LUT_INDEX_INVALID, __FUNCTION__ , eDataCode);
        }

        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    //INT32 *pMPT = (INT32 *)*((INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, psMPT))) ;
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    //INT32 lTableSize = *(INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Max Value
    INT32 lMaxCMvalue = *( pMPT + (eMPT_TYPE_NUMBER*(lTableSize-1) + eMPT_TYPE_CM) );

    return lMaxCMvalue;
}

////////////////////////////////////////////////////////
INT32 DataMapping_GetMinCMvalueFromGui(eDATA_CODE eDataCode)
{
    INT32 lPreviousCMValue = MPT_INDEX_INVALID;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: LutIndex == %d %s Datacode(%d)\n", LUT_INDEX_INVALID, __FUNCTION__ , eDataCode);
        }

        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    //INT32 *pMPT = (INT32 *)*((INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, psMPT))) ;
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    //INT32 lTableSize = *(INT32 *)((INT8 *)(sDataMapLut + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Min Value
    INT32 lMinCMvalue = *( pMPT + eMPT_TYPE_CM );

    return lMinCMvalue;
}


////////////////////////////////////////////////////////

INT32 DataMapping_GUI_TotalNum_Get(eDATA_CODE DataCode)
{
    INT32 TotalItemNum = 0;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(DataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Gui Item
    INT32 Index = 0;
    while(Index < lTableSize)
    {
        INT32 GuiValue = *( pMPT + (eMPT_TYPE_NUMBER * Index) + eMPT_TYPE_GUI );
        if(GuiValue == GUI_VALUE_INVALID)  //skip Hide Item
        {
            Index++;
            continue;
        }

        TotalItemNum++;

        Index++;
    }

    return TotalItemNum;
}

INT32 DataMapping_GC_TotalNum_Get(eDATA_CODE DataCode)
{
    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(DataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        return LUT_INDEX_INVALID;
    }

    // Get total GC item size
    INT32 GC_ItemSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size));

    return GC_ItemSize;
}


////////////////////////////////////////////////////////
//return lGuiItemList total number
INT32 DataMapping_GuiItemList_Get(eDATA_CODE eDataCode, INT32 *plGuiItemList)
{
    INT32 TotalItemNum = 0;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: LutIndex == %d %s Datacode(%d)\n", LUT_INDEX_INVALID, __FUNCTION__ , eDataCode);
        }

        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Gui Item
    INT32 Index = 0;
    while(Index < lTableSize)
    {
        INT32 GuiValue = *( pMPT + (eMPT_TYPE_NUMBER * Index) + eMPT_TYPE_GUI );
        if(GuiValue == GUI_VALUE_INVALID)  //skip Hide Item
        {
            Index++;
            continue;
        }

        *(plGuiItemList + TotalItemNum) = *( pMPT + (eMPT_TYPE_NUMBER * Index) + eMPT_TYPE_CM );
        TotalItemNum++;

        Index++;
    }

    return TotalItemNum;
}


//return lGC_ItemList total number
INT32 DataMapping_GC_ItemList_Get(eDATA_CODE eDataCode, INT32 *plGC_ItemList)
{
    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    // Search Data Mapping Lut Index
    INT32 LutIndex = DataMapping_GetDataMappingLutIndex(eDataCode);
    if(LutIndex == LUT_INDEX_INVALID)
    {
        if(ucDebugMode >= eDEBUG_LEVEL1)
        {
            printf("Mapping Fail: LutIndex == %d %s Datacode(%d)\n", LUT_INDEX_INVALID, __FUNCTION__ , eDataCode);
        }

        return LUT_INDEX_INVALID;
    }

    // Get Mapping Table
    INT32 *pMPT = (INT32 *)((INT8 *)pShmPtr+SHM_MPT_DATA_OFFSET_START+(LutIndex*SHM_ONE_MPT_SIZE)) ;   //from shared memory

    // Get Mapping Table Size (GUI item size)
    INT32 lTableSize = *(INT32 *)((INT8 *)((sDATA_MAPPING_LUT_FORMAT *)pShmPtr + LutIndex) + offsetof(sDATA_MAPPING_LUT_FORMAT, lMPT_Size)) ;

    // Get Gui Item
    INT32 Index = 0;
    while(Index < lTableSize)
    {
        *(plGC_ItemList + Index) = *( pMPT + (eMPT_TYPE_NUMBER * Index) + eMPT_TYPE_CM );

        Index++;
    }

    return lTableSize;
}


INT32 DataMapping_Test(void)
{
    INT32 Source = 1;  //eGUI_SOURCE_ID_HDMI1
    printf("GUI2CM = %d\n" , GUI2CM(edcMAIN_INPUT,Source));
    printf("GUI2CLI = %d\n" , GUI2CLI(edcMAIN_INPUT,Source));

    Source = 4;  //eCM_SOURCE_HDMI2 ;
    printf("CM2GUI = %d\n" , CM2GUI(edcMAIN_INPUT,Source));
    printf("CM2CLI = %d\n" , CM2CLI(edcMAIN_INPUT,Source));

    Source = 5 ;
    printf("CLI2GUI = %d\n" , CLI2GUI(edcMAIN_INPUT,Source));
    printf("CLI2CM = %d\n" , CLI2CM(edcMAIN_INPUT,Source));

    printf("GUI_ITEM_SIZE = %d\n" , GUI_ITEM_SIZE(edcMAIN_INPUT));
}


INT32 DataMapping_DataRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value)
{
    INT32 lMax = 0;
    INT32 lMin = 0;

    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        if(lCM_Value == eCM_ID_INVALID)
        {
            return RANGE_CHECK_FAIL;
        }

        // get Gui Max and Min value
        INT32 lGuiValue = CM2GUI(eDataCode, lCM_Value);
        INT32 lMax = GUI_ITEM_SIZE(eDataCode) - 1;
        INT32 lMin = 0;

        if(lMax >= lGuiValue && lGuiValue >= lMin)
        {
            return RANGE_CHECK_PASS;
        }
        else
        {
            return RANGE_CHECK_FAIL;
        }
    }

    //not common data code
    return RANGE_CHECK_FAIL;
}


INT32 DataMapping_IsCommonDataCode(eDATA_CODE eDataCode)
{
    return (DataMapping_GetDataMappingLutIndex(eDataCode) >= 0) ;
}

void DataMapping_DebugModeSet(eDEBUGMODE_TYPE ucEnable)
{
    ucDebugMode = ucEnable;
}

INT32 DataMapping_ReadyFlagGet(void)
{
    if(pShmPtr == NULL)
    {
        return DATA_MAP_NOT_READY;
    }

    INT32 Flag = 0;
    memcpy(&Flag, (INT8 *)pShmPtr+SHM_DATAMAPLUT_READY_FLAG_OFFSET_START, 4);

    if(ucDebugMode >= eDEBUG_LEVEL1)
    {
        printf("DataMapping_ReadyFlagGet %d\n", Flag);
    }

    if(Flag == SHM_DATAMAPLUT_READY_FLAG_MAGIC_NUM)
    {
        return 1;
    }

    return DATA_MAP_NOT_READY;
}

void DataMapping_ReadyFlagSet(INT32 IsReady)
{
    UINT32 Flag = 0 ;

    if(IsReady == 1)
    {
        Flag = SHM_DATAMAPLUT_READY_FLAG_MAGIC_NUM;
    }

    if(pShmPtr != NULL)
    {
        flock(ShmFd, LOCK_EX);
        memcpy((INT8 *)pShmPtr+SHM_DATAMAPLUT_READY_FLAG_OFFSET_START, &Flag,  4);
        msync((INT8 *)pShmPtr+SHM_DATAMAPLUT_READY_FLAG_OFFSET_START, 4, MS_SYNC);
        flock(ShmFd, LOCK_UN);
    }
}


#if 0
INT32 DataMapping_DataRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value)
{
    INT32 lMax = 0;
    INT32 lMin = 0;

    if(IS_COMMON_DATA_CODE(eDataCode))
    {
        // get Gui Max and Min value
        if(appGui_DataCode_MaxValue_Get(eDataCode, &lMax) == eEXEC_CODE_PASS &&
           appGui_DataCode_MinValue_Get(eDataCode, &lMin) == eEXEC_CODE_PASS)
        {
            INT32 lGuiValue = CM2GUI(eDataCode, lCM_Value);

            if(lMax >= lGuiValue && lGuiValue >= lMin)
            {
                return RANGE_CHECK_PASS;
            }
            else
            {
                return RANGE_CHECK_FAIL;
            }
        }
    }
    else   //not common data code
    {
        if(appDataMgr_Data_Range_Get(eDataCode, edrMAX, &lMax) == eEXEC_CODE_PASS &&
           appDataMgr_Data_Range_Get(eDataCode, edrMIN, &lMin) == eEXEC_CODE_PASS)
        {
            if(lMax >= lCM_Value && lCM_Value >= lMin)
            {
                return RANGE_CHECK_PASS;
            }
            else
            {
                return RANGE_CHECK_FAIL;
            }
        }
    }

    return RANGE_CHECK_PASS;
}
#endif

#if 0
INT32 utilDataMapping_SearchFromDBIndex(sMAPPING_TABLE_FORMAT sMPT, INT32 lValue)
{
    INT32 Index = 0;
    UINT32 uiTableSize = sizeof(sMPT) / sizeof(sMAPPING_TABLE_FORMAT);

    while(Index++ < uiTableSize)
    {
        if(sMPT[Index].uiDB_Value == lValue)
        {
            return Index ;
        }
    }

    printf("MPT_INDEX_INVALID (Value %d)" , lValue);

    return MPT_INDEX_INVALID;
}

INT32 utilDataMapping_SearchFromCLIIndex(sMAPPING_TABLE_FORMAT sMPT, INT32 lValue)
{
    INT32 Index = 0;
    UINT32 uiTableSize = sizeof(sMPT) / sizeof(sMAPPING_TABLE_FORMAT);

    while(Index++ < uiTableSize)
    {
        if(sMPT[Index].uiCLI_Value == lValue)
        {
            return Index ;
        }
    }

    printf("MPT_INDEX_INVALID ( Value %d)" , lValue);

    return MPT_INDEX_INVALID;
}
#endif

