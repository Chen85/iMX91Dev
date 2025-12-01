#ifndef __DATAMAPPING_H__
#define __DATAMAPPING_H__

#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "type_def.h"
#include "SharedMemCtrl.h"
#include "CommonDef.h"

//#define GUI_CHECK_BIT BIT6
//#define CLI_CHECK_BIT BIT7

/*
    x : datacode
    y : value
    return mapped value

    GUI : OSD
    CM : Common , database (shared memory)
    CLI : RS232 cmd
*/
#define GUI2CM(x,y)  DataMapping_Transform(eMPT_TYPE_GUI, eMPT_TYPE_CM,  (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)
#define CM2GUI(x,y)  DataMapping_Transform(eMPT_TYPE_CM,  eMPT_TYPE_GUI, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)

#define CLI2CM(x,y)  DataMapping_Transform(eMPT_TYPE_CLI, eMPT_TYPE_CM,  (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)
#define CM2CLI(x,y)  DataMapping_Transform(eMPT_TYPE_CM,  eMPT_TYPE_CLI, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)

#define GUI2CLI(x,y)  DataMapping_Transform(eMPT_TYPE_GUI, eMPT_TYPE_CLI, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)
#define CLI2GUI(x,y)  DataMapping_Transform(eMPT_TYPE_CLI, eMPT_TYPE_GUI, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)

#define GUI2GM(x,y)  DataMapping_Transform(eMPT_TYPE_GUI, eMPT_TYPE_GM,  (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)
#define GM2GUI(x,y)  DataMapping_Transform(eMPT_TYPE_GM,  eMPT_TYPE_GUI, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)

#define CLI2GM(x,y)  DataMapping_Transform(eMPT_TYPE_CLI, eMPT_TYPE_GM,  (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)
#define GM2CLI(x,y)  DataMapping_Transform(eMPT_TYPE_GM,  eMPT_TYPE_CLI, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)

#define CM2GM(x,y)  DataMapping_Transform(eMPT_TYPE_CM, eMPT_TYPE_GM,   (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)
#define GM2CM(x,y)  DataMapping_Transform(eMPT_TYPE_GM, eMPT_TYPE_CM,   (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)

#define FILE2RAM(x,y)  DataMapping_Transform(eMPT_TYPE_CM, eMPT_TYPE_CM,  (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__) //HICC2_Doulas_0003
#define RAM2FILE(x,y)   DataMapping_Transform(eMPT_TYPE_CM, eMPT_TYPE_CM, (eDATA_CODE)x, (INT32)y, (char *)__FUNCTION__, __LINE__)  //HICC2_Doulas_0003

////////////////////////////////////////////////////////////////////////////////////////////////

#define GUI_ITEM_SIZE(x)  DataMapping_GUI_TotalNum_Get((eDATA_CODE)x)

#define IS_COMMON_DATA_CODE(x)  DataMapping_IsCommonDataCode((eDATA_CODE)x)

#define GUI_VALUE_INVALID   (INT32)(0xFFFFFFFF)   //if item is Hide, HICC1.0 gui value is GUI_VALUE_INVALID
#define CLI_VALUE_INVALID   (INT32)(0xFFFFFFFF)   //if item is Hide, HICC1.0 CLI value is CLI_VALUE_INVALID

#define DATA_MAP_PASS       (INT32)(1)
#define MPT_INDEX_INVALID   (INT32)(-2)
#define DATA_MAP_INIT_FAIL  (INT32)(-3)
#define DATA_MAP_NOT_READY  (INT32)(-4)
#define DATA_CHECK_FAIL     (INT32)(-5)
#define LUT_INDEX_INVALID   (INT32)(-6)

#define RANGE_CHECK_PASS    (INT32)(1)
#define RANGE_CHECK_FAIL    (INT32)(0)

#define MPT_SIZE(x)  sizeof(x)/sizeof(sMAPPING_TABLE_FORMAT)    //Mapping Table Size

/////////////////////////////////////////////////

typedef enum
{
    eDEBUG_OFF,
    eDEBUG_FATAL_ERROR,
    eDEBUG_LEVEL1,
    eDEBUG_LEVEL2,  //most error message

    eDEBUG_NUMBER,

}eDEBUGMODE_TYPE;


/////////////////////////////////////////////////

typedef enum
{
    eMPT_TYPE_GUI,  //GUI , OSD , AP
    eMPT_TYPE_CM,   //Common , database (shared memory)
    eMPT_TYPE_CLI,  //RS232 cmd , CLI
    eMPT_TYPE_GM,   //Gui Maximized (Maximized List Item for HICC2.0)
    eMPT_TYPE_NUMBER,

}eMPT_TYPE;


typedef struct
{
    INT32 lGUI_Value;
    INT32 lCM_Value;
    INT32 lCLI_Value;
    INT32 lGC_Value;

}sMAPPING_TABLE_FORMAT, *psMAPPING_TABLE_FORMAT;


typedef struct
{
    eDATA_CODE eDataCode;
    sMAPPING_TABLE_FORMAT *psMPT;
    INT32 lMPT_Size;
}sDATA_MAPPING_LUT_FORMAT;



/*
HDMI1 / HDMI2 / DP

CM : 3 4 6
GUI : 0 1 2
RS232 : 3 4 5
Key : 3 4 5

*/

INT32 DataMapping_Init(sDATA_MAPPING_LUT_FORMAT *pSystemDataMapLut, INT32 lSize);
INT32 DataMapping_DeInit(void);
INT32 DataMapping_Transform(eMPT_TYPE eType_Before, eMPT_TYPE eType_After, eDATA_CODE eDataCode, INT32 lValue, char *cFuncionName, INT32 llineNum);
INT32 DataMapping_GetDataMappingLutIndex(eDATA_CODE eDataCode);
INT32 DataMapping_GUI_TotalNum_Get(eDATA_CODE DataCode);
INT32 DataMapping_GC_TotalNum_Get(eDATA_CODE DataCode);
INT32 DataMapping_GuiItemList_Get(eDATA_CODE eDataCode, INT32 *plGuiItemList);
INT32 DataMapping_GC_ItemList_Get(eDATA_CODE eDataCode, INT32 *plGC_ItemList);
INT32 DataMapping_GetNextCMvalueFromGui(eDATA_CODE eDataCode, INT32 lCMCurrentValue);
INT32 DataMapping_GetPreCMvalueFromGui(eDATA_CODE eDataCode, INT32 lCMCurrentValue);
INT32 DataMapping_GetMaxCMvalueFromGui(eDATA_CODE eDataCode);
INT32 DataMapping_GetMinCMvalueFromGui(eDATA_CODE eDataCode);
INT32 DataMapping_DataRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value);
INT32 DataMapping_IsCommonDataCode(eDATA_CODE eDataCode);
void DataMapping_DebugModeSet(eDEBUGMODE_TYPE ucEnable);
INT32 DataMapping_Test(void);

INT32 DataMapping_ImportSharedMemory(void);
INT32 DataMapping_ReaderInit(void);

INT32 DataMapping_ReadyFlagGet(void);
void DataMapping_ReadyFlagSet(INT32 Ready);


#endif  //__DATAMAPPING_H__

