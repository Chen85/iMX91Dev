#include <math.h>
#include "dvCDCE913.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"

//static sDRV_VCXO_INFO m_DrvVCXOInfo[2];   //A70LV_Doulas_0072 modify

//#define SET_CAPACITOR_10PF

////////////////// I2C config ////////////////////
#if (CURRENT_MAIN_BOARD == CORE_BOARD)
#include "Board_I2C_Dev_Table.h"
int lCDCE913_I2C_Bus[eCDCE913_SEL_NUMBER];
int lCDCE913_I2C_Addr[eCDCE913_SEL_NUMBER];
int lCDCE913_I2C_Flag[eCDCE913_SEL_NUMBER];
int lCDCE913_I2C_Retry[eCDCE913_SEL_NUMBER];

#if 0
#define CDCE913_0_I2C_BUS            eBOARD_I2C_BUS2
#define CDCE913_0_I2C_ADDRESS        (0xD8)
#define CDCE913_0_I2C_FLAG           (I2C_DEFAULT|I2C_REG_NEED)
#define CDCE913_0_I2C_RETRY_COUNT    (10)

#define CDCE913_1_I2C_BUS            (eBOARD_I2C_BUS0)
#define CDCE913_1_I2C_ADDRESS        (0xD8)
#define CDCE913_1_I2C_FLAG           (I2C_DEFAULT|I2C_REG_NEED)
#define CDCE913_1_I2C_RETRY_COUNT    (10)

#define CDCE913_2_I2C_BUS            eBOARD_I2C_BUS2
#define CDCE913_2_I2C_ADDRESS        (0xD8)
#define CDCE913_2_I2C_FLAG           (I2C_DEFAULT|I2C_REG_NEED)
#define CDCE913_2_I2C_RETRY_COUNT    (10)
#endif
#endif
////////////////// I2C config ////////////////////


void dvCDCE913_I2C_Init(void)
{
    lCDCE913_I2C_Bus[eCDCE913_SEL_0]    = VCXO_0_I2C_BUS;
    lCDCE913_I2C_Bus[eCDCE913_SEL_1]    = VCXO_1_I2C_BUS;
    lCDCE913_I2C_Bus[eCDCE913_SEL_2]    = VCXO_2_I2C_BUS;
    lCDCE913_I2C_Addr[eCDCE913_SEL_0]   = VCXO_0_I2C_ADDR;
    lCDCE913_I2C_Addr[eCDCE913_SEL_1]   = VCXO_1_I2C_ADDR;
    lCDCE913_I2C_Addr[eCDCE913_SEL_2]   = VCXO_2_I2C_ADDR;
    lCDCE913_I2C_Flag[eCDCE913_SEL_0]   = VCXO_I2C_FLAG;
    lCDCE913_I2C_Flag[eCDCE913_SEL_1]   = VCXO_I2C_FLAG;
    lCDCE913_I2C_Flag[eCDCE913_SEL_2]   = VCXO_I2C_FLAG;
    lCDCE913_I2C_Retry[eCDCE913_SEL_0]  = VCXO_I2C_RETRY;
    lCDCE913_I2C_Retry[eCDCE913_SEL_1]  = VCXO_I2C_RETRY;
    lCDCE913_I2C_Retry[eCDCE913_SEL_2]  = VCXO_I2C_RETRY;
}


eRESULT dvCDCE913_I2C_Write(UINT8 ucReg, UINT8 *ucData, eCDCE913_SEL eCDCE913_Sel)
{
    eRESULT eRet = rcSUCCESS ;
    UINT8 ucRetry = 0;
	BYTE eDevID = eCM_IF_CLOCK_GENERATOR;

    if( eCDCE913_Sel >= eCDCE913_SEL_NUMBER ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Bus)/sizeof(lCDCE913_I2C_Bus[0]))  ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Addr)/sizeof(lCDCE913_I2C_Addr[0])) ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Flag)/sizeof(lCDCE913_I2C_Flag[0])) ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Retry)/sizeof(lCDCE913_I2C_Retry[0]))
    )
    {
        LOG_MSG(db_DV_VCXO, "(funcs:%d, line:%d) Dev Sel Error %d\r\n", __FUNCTION__, __LINE__, eCDCE913_Sel);
        return rcERROR;
    }

    do
    {
		dvI2C_Dev_TotalCount(eDevID);

        eRet = Board_I2C_Master_Write( lCDCE913_I2C_Bus[eCDCE913_Sel],
                                       lCDCE913_I2C_Addr[eCDCE913_Sel],
                                       ucReg,
                                       1,
                                       ucData,
                                       lCDCE913_I2C_Flag[eCDCE913_Sel]) ;

        ucRetry++ ;

        if(eRet != rcSUCCESS)
        {
		    dvI2C_Dev_RetryCount(eDevID);
            utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_VCXO);
        }
    }
    while( (eRet == rcERROR) && (ucRetry < lCDCE913_I2C_Retry[eCDCE913_Sel]) );


    //ASSERT(eRet == rcSUCCESS);
	if(eRet != rcSUCCESS)
	{
	    dvI2C_Dev_ErrorCount(eDevID);
		utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_VCXO);
		LOG_MSG(db_ALWAYS, "VCXO Write Error : 0x%02X %s: %d\r\n", ucReg, __FILE__, __LINE__);
	}

    return eRet ;

}


eRESULT dvCDCE913_I2C_Read(UINT8 ucReg, UINT8 *ucData, eCDCE913_SEL eCDCE913_Sel)
{
    eRESULT eRet = rcSUCCESS ;
    UINT8 ucRetry = 0;
    BYTE eDevID = eCM_IF_CLOCK_GENERATOR;

    if( eCDCE913_Sel >= eCDCE913_SEL_NUMBER ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Bus)/sizeof(lCDCE913_I2C_Bus[0]))  ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Addr)/sizeof(lCDCE913_I2C_Addr[0])) ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Flag)/sizeof(lCDCE913_I2C_Flag[0])) ||
        eCDCE913_Sel >= (sizeof(lCDCE913_I2C_Retry)/sizeof(lCDCE913_I2C_Retry[0]))
    )
    {
        LOG_MSG(db_DV_VCXO, "(funcs:%d, line:%d) Dev Sel Error %d\r\n", __FUNCTION__, __LINE__, eCDCE913_Sel);
        return rcERROR;
    }


    do
    {
		dvI2C_Dev_TotalCount(eDevID);

        eRet = Board_I2C_Master_Read( lCDCE913_I2C_Bus[eCDCE913_Sel],
                                      lCDCE913_I2C_Addr[eCDCE913_Sel],
                                      ucReg,
                                      1,
                                      ucData,
                                      lCDCE913_I2C_Flag[eCDCE913_Sel]) ;

        ucRetry++ ;

        if(eRet != rcSUCCESS)
        {
		    dvI2C_Dev_RetryCount(eDevID);
            utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_VCXO);
        }
    }
    while( (eRet == rcERROR) && (ucRetry < lCDCE913_I2C_Retry[eCDCE913_Sel]) );


    //ASSERT(eRet == rcSUCCESS);
	if(eRet != rcSUCCESS)
	{
        dvI2C_Dev_ErrorCount(eDevID);
		utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_VCXO);
		LOG_MSG(db_ALWAYS, "VCXO Read Error : 0x%02X %s: %d\r\n", ucReg, __FILE__, __LINE__);
	}


    return eRet ;

}


unsigned long dvCDCE9xx_get_clkgen_bitstream(double fout, struct cdce_param *param)
{

	short	div;
	double	fvco;
	double	error, min_error;
	double	nm_approx;
	int	i;
	unsigned long	n, m, fine_n, fine_m;
	unsigned long	nd, p, q, r;
	unsigned long	set = 0;

	if ((fout <= CDCE9XX_MIN_FREQ) || (fout > CDCE9XX_MAX_FREQ))
	{
		param->div = 0;
		param->n = 4;
		param->p = 2;
		param->q = 16;
		param->r = 0;
		param->vco_range = 0;
		return 0;
	}

	for (i = 127; i > 0; i--) {
		fvco = fout * i;
		if (fvco <= CDCE9XX_MAX_VCO_FREQ) {
			break;
		}
	}

	div = i;

	nm_approx = fvco / (double)CDCE9XX_INPUT_FREQ;

	min_error = 99999999;
	fine_m = 0;
	fine_n = 0;

	for (m = 1; m < 512; m++) {
		n = (unsigned long)(nm_approx * (double)m);

		for (i = 0; i < 2; i++, n++) {
			if (n < 1 || n > 4095) continue;

			error = (double)n / (double)m - nm_approx;

			if (error < 0) error = -error;

			if (error < min_error) {
				fine_n = n;
				fine_m = m;
				min_error = error;
			}
		}
	}

	p = 4 - (int)(log(fine_n / fine_m) / log(2));
	if (p < 0) p = 0;
	nd = (int)(fine_n*pow(2, p));
	q = (int)(nd / fine_m);
	r = nd - fine_m * q;

	param->div = div;
	param->n = fine_n;
	param->p = p;
	param->q = q;
	param->r = r;
	if (fvco < 125000000) param->vco_range = 0;
	else if (fvco < 150000000) param->vco_range = 1;
	else if (fvco < 175000000) param->vco_range = 2;
	else param->vco_range = 3;

	return 0;
}


eCDCE913_EXEC_CODE write_CDCE913(unsigned int frq, eCDCE913_SEL eCDCE913_Sel)
{
	int		i;
	struct	cdce_param param;
	unsigned char cfgbuf[6], pllbuf[16];

    if(eCDCE913_Sel >= eCDCE913_SEL_NUMBER)
    {
        LOG_MSG(db_DV_VCXO, "VXCO Sel %d Error\r\n", eCDCE913_Sel);
        return eCDCE913_EXEC_CODE_NO_CHIP_SELECT;
    }

	//if (ch > 10) {
	//	return;
	//}

	dvCDCE9xx_get_clkgen_bitstream((double)frq, &param);

	cfgbuf[0] = 0x01;
	cfgbuf[1] = 0x94;
	cfgbuf[2] = 0;
	cfgbuf[3] = 0x02;
	cfgbuf[4] = 0x50;
	cfgbuf[5] = 0xa0; // not write

	pllbuf[0] = 0;
	pllbuf[1] = 0;
	pllbuf[2] = 0;
	pllbuf[3] = 0;
	pllbuf[4] = 0x6d;
	pllbuf[5] = 0x02;
	pllbuf[6] = param.div & 0x7f;
	pllbuf[7] = 0;
	pllbuf[8] = (param.n >> 4) & 0xff;
	pllbuf[9] = ((param.n << 4) & 0xf0) | ((param.r >> 5) & 0x0f);
	pllbuf[10] = ((param.r << 3) & 0xf8) | ((param.q >> 3) & 0x07);
	pllbuf[11] = ((param.q << 5) & 0xe0) | ((param.p << 2) & 0x1c) | (param.vco_range & 0x03);
	pllbuf[12] = pllbuf[8];
	pllbuf[13] = pllbuf[9];
	pllbuf[14] = pllbuf[10];
	pllbuf[15] = pllbuf[11];
// for cdce913
	for (i = 0; i<5; i++) {
		dvCDCE913_I2C_Write(i + 1 + 0x80, &cfgbuf[i], eCDCE913_Sel);
	}

	for (i = 0; i<16; i++) {
		dvCDCE913_I2C_Write(i + 16 + 0x80, &pllbuf[i], eCDCE913_Sel);
	}
// for cdce925
    #if 0
	cfgbuf[0] = 0x00;
	for (i = 0; i < 5; i++) {
		write_I2C(0x64, i + 1 + 0x80, cfgbuf[i], ch);
	}

	for (i = 0; i < 16; i++) {
		write_I2C(0x64, i + 16 + 0x80, pllbuf[i], ch);
	}
	#endif

}


static UINT8 dvCDCE913_RegWrite(UINT8 ucRegAddr, UINT8 ucDataIn, eCDCE913_SEL eCDCE913_Sel)
{
#if 0
    if(eCDCE913_Sel >= eVCXO_SEL_LAST)
        return eVCXO_EXEC_CODE_NO_CHIP_SELECT;

    switch(eCDCE913_Sel)
    {
        case eVCXO_SEL_C734:
        case eVCXO_SEL_C821_HD:     //A70LV_Doulas_2000
        case eVCXO_SEL_C821_WU:        //A70LV_Doulas_2000
        case eVCXO_SEL_C821_720P:        //A70LV_Doulas_2000
        case eVCXO_SEL_C821_720P120:    //A70LV_Doulas_0094
        case eVCXO_SEL_C821_800x600_120: //A70LV_Doulas_0094
        case eVCXO_SEL_C821_1080P120:    //A70LV_Doulas_0094
        case eVCXO_SEL_C821_WUXGA120:    //A70LV_Doulas_0094

#if (CURRENT_MAIN_BOARD == NO_BOARD)
            return 0;

#elif (CURRENT_MAIN_BOARD == CORE_BOARD)
        {
            eRESULT eRet = rcERROR ;
            eRet = dvCDCE913_I2C_Write( ucRegAddr|0x80, &ucDataIn, eCDCE913_Sel ) ;

            if( eRet == rcSUCCESS )
            {
                return eCDCE913_EXEC_CODE_PASS;
            }
            else
            {
                LOG_MSG(db_DV_VCXO, "CDCE913(%d) I2C write fail: reg(%d)\r\n", eCDCE913_Sel , ucRegAddr);
                return eVCXO_EXEC_CODE_FAIL;
            }
        }
#else
        return 0;
#endif

    //    break;        //A70LV_Doulas_0072 modify


    case eVCXO_SEL_C790:

#if (CURRENT_MAIN_BOARD == NO_BOARD)
        return 0;

#elif (CURRENT_MAIN_BOARD == CORE_BOARD)
        {
            eRESULT eRet = rcERROR ;
            eRet = dvCDCE913_I2C_Write( ucRegAddr|0x80, &ucDataIn, eCDCE913_Sel ) ;

            if( eRet == rcSUCCESS )
            {
                return eCDCE913_EXEC_CODE_PASS;
            }
            else
            {
                LOG_MSG(db_DV_VCXO, "CDCE913(%d) I2C write fail: reg(%d)\r\n", eCDCE913_Sel , ucRegAddr);
                return eVCXO_EXEC_CODE_FAIL;
            }
        }
#else
        return 0;
#endif

    //    break;    //A70LV_Doulas_0072 modify

    }
#endif

    return eCDCE913_EXEC_CODE_PASS;
}

static UINT8 dvCDCE913_Write_InitTbl(eCDCE913_SEL eCDCE913_Sel)
{
    #if 0
    UINT32 ulCnt = 0;
    eVCXO_EXEC_CODE eRet = eCDCE913_EXEC_CODE_PASS;
    const sREG_VCXO_INFO *RegTable = m_InitVCXOTable_83MHz ;
    UINT8 ucRegTableSize = 0 ;

    switch(eCDCE913_Sel)
    {

        default:
            return eVCXO_EXEC_CODE_NO_CHIP_SELECT;
    }


    while(ulCnt < ucRegTableSize)
    {
        eRet = (eVCXO_EXEC_CODE)dvCDCE913_RegWrite(RegTable[ulCnt].ucAddr, RegTable[ulCnt].ucValue, eCDCE913_Sel) ;  //A70LV_Doulas_0072 modify
        LOG_MSG(db_DV_VCXO, "(%02X) [%02X]\r\n", RegTable[ulCnt].ucAddr, RegTable[ulCnt].ucValue);

        if(eRet != eCDCE913_EXEC_CODE_PASS)
        {
            LOG_MSG(db_ALWAYS, "\r\n\r\nInit CDCE913(%d) Fail!!\r\n\r\n" , eCDCE913_Sel);
            break;
        }

        ulCnt++;
    }

    return eRet;
    #endif

    return 0;
}




