#ifndef __UTILDATAMAPPING_H__
#define __UTILDATAMAPPING_H__

#include "type_def.h"
#include "SharedMemCtrl.h"
#include "Common.h"

#include "CustomDef.h"      //H2 wait review
#include "CustomDefCLI.h"   //H2 wait review

#define DATA_RANGE_CHECK_PASS (1)
#define DATA_RANGE_CHECK_FAIL (0)


#if 0
#define IS_COMMON_DATA_CODE(x)  ((eDATA_CODE)x == edcMAIN_INPUT || \
                                 (eDATA_CODE)x == edcSUB_INPUT || \
                                 (eDATA_CODE)x == edcBACKUPINPUT_PRIMARY_INPUT || \
                                 (eDATA_CODE)x == edcBACKUPINPUT_SECONDARY_INPUT || \
                                 (eDATA_CODE)x == edcOSDTEST_PATTERN || \
                                 (eDATA_CODE)x == edcSERVICE_TEST_PATTERN || \
                                 (eDATA_CODE)x == edcTWIST_PATTERN || \
                                 (eDATA_CODE)x == edcPICTURE_SETTINGS || \
                                 (eDATA_CODE)x == edcFORCE_PICTURE_SETTINGS || \
                                 (eDATA_CODE)x == edcLANGUAGE || \
                                 (eDATA_CODE)x == edcSIZE_PRESETS || \
                                 (eDATA_CODE)x == edcMAIN_ASPECT_RATIO || \
                                 (eDATA_CODE)x == edcSUB_ASPECT_RATIO || \
                                 (eDATA_CODE)x == edcPRE_USER_MODE)
#endif

INT32 utilDataMapping_Init(UINT8 IsCreator);
INT32 utilDataMapping_DeInit(void);
INT32 utilDataMapping_CMValueRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value);
INT32 utilDataMapping_GuiValueRangeCheck(eDATA_CODE eDataCode, INT32 lCM_Value);

#endif  //__UTILDATAMAPPING_H__

