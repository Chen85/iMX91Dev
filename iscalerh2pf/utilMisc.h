#ifndef __UTILMISC_H__
#define __UTILMISC_H__


#include "./inc/Common.h"

#define UTILMISC_EXEC_PASS      0
#define UTILMISC_NO_FILE        1
#define UTILMISC_DATA_ERROR     2

#define MAX_CONF_ITEM                        2048
#define MAX_CONF_ITEM_VALUE_STR_LENGTH       128

INT8 utilMisc_GetFileData(INT8 *cFileName, UINT8 *paucData, UINT32 ulSize);
INT8 utilMisc_GetReleaseVersion(UINT8 *aucString);
INT8 utilMisc_GetStringFromConf(const char *FilePath, const char *FieldName, UINT8 *aucDataString);

#endif

