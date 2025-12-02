#ifndef DV_PROAV_SERIAL_FLASH
#define DV_PROAV_SERIAL_FLASH

#include "dvProAV_Platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief DRAM To Flash Access
 * @param [in] u32DramAddress : Dram Address
 * @param [in] u32FlashAddress : Flash Address
 * @param [in] u32Length : data bytes
 * @param [in] bDram2Flash : false = Flash2Dram / true = Dram2Flash
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiDram2Flash(uint32 u32DramAddress, uint32 u32FlashAddress, uint32 u32Length, bool bDram2Flash);

/**
 * @brief SPI To Flash Access
 * @param [in] flashAddress : Flash Address
 * @param [in/out] ucData : data point
 * @param [in] length : data bytes
 * @param [in] bWrFlash : false = Flash2SPI / true = SPI2Flash
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiSpi2Flash(uint32 u32FlashAddress, uint08 *ucData, uint32 u32Length, bool bWrFlash);

/**
 * @brief Flash to Flash
 * @param [in] u16Length : Data Byte
 * @param [in] ulCpyAddr : Copy DRAM Address
 * @param [in] ulPasAddr : Paste DRAM Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiFlashToFlash(uint16 u16Length, uint32 ulCpyAddr, uint32 ulPasAddr);

/**
 * @brief SPI To DRAM Access
 * @param [in] u32Address : DRAM address
 * @param [in/out] u8Data : data point
 * @param [in] u32Length :
 * @param [in] bWrite : flase = DRAM2SPI / true = SPI2DRAM
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiDramAccess(uint32 u32Address, uint08 *u8Data, uint32 u32Length, bool bWrite);


/**
 * @brief Flash Erase Sector
 * @param u32Address : Flash address
 * @param u32Length : erase sector length
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiEraseSector(uint32 u32Address, uint32 u32Length);

int dvProAV_SfiEraseAll(void);
int dvProAV_SfiReadDlySet(uint32 ulDly);
int dvProAV_SfiClkRateSet(uint32 ulSpiClockRate);

/**
 * @brief Serial Flash Csr Read
 * @param [in] ucCsrAddr
 * @return CSR data
 */
int dvProAV_SfiCsrRead(uint8 ucCsrAddr);

/**
 * @brief Serial Flash Initial
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SfiInit(void);

#ifdef __cplusplus
}
#endif

#endif //DV_PROAV_SERIAL_FLASH
