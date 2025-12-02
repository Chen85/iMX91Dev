#ifndef _DVCDCE913_H_
#define _DVCDCE913_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"


/*+-----------------------------+*/
/*|	CDCE9XX                     |*/
/*+-----------------------------+*/
#define CDCE9XX_INPUT_FREQ      27000000	/*27MHz*/
#define CDCE9XX_MAX_FREQ        230000000	/*MAX230MHz*/
#define CDCE9XX_MIN_FREQ        630000		/*MIN630kHz*/
#define CDCE9XX_MAX_VCO_FREQ    230000000	/*MAX230MHz*/

struct cdce_param {
    unsigned int div;
    unsigned int n;
    unsigned int p;
    unsigned int q;
    unsigned int r;
    unsigned int vco_range;
};

typedef enum
{
    eCDCE913_EXEC_CODE_PASS,                   /* pass */
    eCDCE913_EXEC_CODE_FAIL,                   /* general fail indication */
    eCDCE913_EXEC_CODE_FATAL,                  /* fatal error; halt application */
    eCDCE913_EXEC_CODE_NO_CHIP_SELECT,
}eCDCE913_EXEC_CODE;

typedef enum
{
    eCDCE913_SEL_0,
    eCDCE913_SEL_1,
    eCDCE913_SEL_2,

    eCDCE913_SEL_NUMBER,
}eCDCE913_SEL;

//************************* Enum End *************************//



//===================== Struct Start ===================//
#if 0
typedef struct
{
    UINT8 ucAddr;
    UINT8 ucMask;
    UINT8 ucValue;
}sREG_VCXO_INFO, *PsREG_VCXO_INFO;


typedef struct
{
    BOOL         bInit;

}sDRV_VCXO_INFO, *PsDRV_VCXO_INFO;
#endif

//===================== Struct End ===================//



eRESULT dvCDCE913_I2C_Write(UINT8 ucReg, UINT8 *ucData, eCDCE913_SEL eCDCE913_Sel);
eRESULT dvCDCE913_I2C_Read(UINT8 ucReg, UINT8 *ucData, eCDCE913_SEL eCDCE913_Sel);
void dvCDCE913_I2C_Init(void);
eCDCE913_EXEC_CODE write_CDCE913(unsigned int frq, eCDCE913_SEL eCDCE913_Sel);

#ifdef __cplusplus
}
#endif


#endif  //_DVCDCE913_H_

