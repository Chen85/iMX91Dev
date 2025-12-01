#ifndef _COMMONAPI_
#define _COMMONAPI_

#include "type_def.h"
#include "SharedMemCtrl.h"
#include "CommonType.h"

#define LOG_TO_SYSLOG(fmt, ...)   syslog(LOG_INFO, fmt, ##__VA_ARGS__)

#define MAX_LOG_NUMBER (100)
#define MAX_LOG_LENGHT (128)


INT32 CommonAPI_DataCode_MaxValue_Get(eDATA_CODE eDataCode);
INT32 CommonAPI_DataCode_MinValue_Get(eDATA_CODE eDataCode);
UINT8 CommonAPI_DataCode_IsSupportAckdone_Get(eDATA_CODE eDataCode);
INT32 CommonAPI_DataCode_DataSize_Get(eDATA_CODE eDataCode);
INT32 CommonAPI_CM2UI_ValueGet_ByDataCode(eDATA_CODE eDataCode, INT32 *piValue);
INT32 CommonAPI_CM2UI_ValueGet_ByItemID(UINT32 ItemID, INT32 *piValue);
INT32 CommonAPI_CM2UI_Range_ByItemID(UINT32 ItemID, INT32 *piMaxValue, INT32 *piMinValue);
INT32 CommonAPI_UI2CM_ValueSet_ByDataCode(eDATA_CODE eDataCode, INT32 iValue, INT32 *iTransData);
INT32 CommonAPI_CM2UI_Range_ByDataCode(eDATA_CODE eDataCode, INT32 *piMaxValue, INT32 *piMinValue);
INT32 CommonAPI_UI2CM_ValueSet_ByItemID(UINT32 ItemID, INT32 iValue, INT32 *iTransData);
INT32 CommonAPI_CM_FuncIsAvailable(eDATA_CODE DataCode, UINT32 CM_Index);
INT32 CommonAPI_UI_FuncIsAvailable(eDATA_CODE DataCode, UINT32 UI_Index);
UINT32 CommonAPI_UI_ListItem_TotalNum_Get(eDATA_CODE DataCode);
UINT32 CommonAPI_ListItem_NextAvailableValueGet(eDATA_CODE DataCode, UINT32 CurrentIndex);
UINT32 CommonAPI_ListItem_PreAvailableValueGet(eDATA_CODE DataCode, UINT32 CurrentIndex);
UINT16 CommonAPI_LoadErrorLog(UINT16 ucNum, char pcString[][128]);
UINT16 CommonAPI_LoadErrorLogWithErrorCode(UINT16 ucNum, char pcString[][128]);
eTEST_PATTERN_ID CommonAPI_TestPatternIDGet(eDATA_CODE eDatacode, UINT32 ID);




#endif //_COMMONAPI_

