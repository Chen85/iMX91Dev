#ifndef UTILDATABASEAPI_H
#define UTILDATABASEAPI_H


////////  callback function start  ////////
typedef char* (*fpDataMgr_DataCodeStringGet)(eDATA_CODE eDataCode);

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    fpDataMgr_DataCodeStringGet fpDataMgr_DataCodeStringGetCb;

}sUTILDATABASE_CALLBACK;

#pragma pack(pop)   /* restore original alignment from stack */

void utilDatabase_RegCallback(sUTILDATABASE_CALLBACK fpCallback);
/////////  callback function end  /////////


#define DATABASE_ACCESS_PASS (INT32)(1)       //A35G2_CDS_Simon_0011
#define DATABASE_ACCESS_FAIL (INT32)(-1)

INT32 utilDatabase_Init(void);
INT32 utilDatabase_WriteData(eSHARED_MEMORY_BLOCK ucDatabaseIndex, void *Data, UINT32 ulIndex);
INT32 utilDatabase_ReadData(eSHARED_MEMORY_BLOCK ucDatabaseIndex, void *Data, UINT32 ulIndex);
INT32 utilDatabase_WriteInformationData(UINT32 ulIndex, UINT8 ucType, UINT8 ucGrayoutStatus, void *Data);
void utilDatabase_ItemPrint(void);
void utilDatabase_InformationPrint(void);


#endif  //UTILDATABASEAPI_H

