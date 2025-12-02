#ifndef _APPDATAITEMAVAILABLEMGR_H_
#define _APPDATAITEMAVAILABLEMGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "utilStorageCfg.h"
#include "Common.h"

eEXEC_CODE palDataItem_Access_AFN_SizePresets(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataItem_Access_AFN_PictureSettings(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataItem_Access_AFN_WarpMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataItem_Access_AFN_BlendMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataItem_Access_AFN_LensMemoryApply(eDATA_ACCESS_MODE eAccessMode, void *pValue);
eEXEC_CODE palDataItem_Access_AFN_Backup_Restore_Restore(eDATA_ACCESS_MODE eAccessMode, void *pValue);
void palDataItemAvailable_Init(void); //HICC2_Simon_0003
void palDataItemAvailable_Poll(void);
eEXEC_CODE palDataItemAvailable_AccessRead(eDATABASE_INFOMATION_INDEX eDataCode, void *pValue);


#endif  //_APPDATAITEMAVAILABLEMGR_H_

