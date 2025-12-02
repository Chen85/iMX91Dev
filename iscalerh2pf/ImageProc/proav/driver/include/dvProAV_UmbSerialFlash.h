#ifndef PROAV_UMBSERIALFLASH_H
#define PROAV_UMBSERIALFLASH_H

#include "dvProAV_Base.h"
#include "stdbool.h"

typedef int (*pfFlashCsrWrite)(uint08 csr, uint32 data);
typedef uint32 (*pfFlashCsrRead)(uint08 csr);

/*typedef struct {
    char acName[32+1];
    uint32 enumIpReset;
    uint32 enumReset;
    uint32 enumBufAddr;
    uint32 enumFlashAddr;
    uint32 enumMoveLen;
    uint32 enumMoveMode;
    uint32 enumMoveStart;
    uint32 enumRestLen;
    uint32 enumCsrAddr;
    uint32 enumCsrCmd;
    uint32 enumCsrWrData;
    uint32 enumCsrRdData;
    uint32 u16LineBytes;
    uint32 u08AddrShift;
    uint32 u32MinBytes;
    uint32 u32BankSize;
    uint32 u32CsDelay;
    uint32 u32SpiClockRate;
    bool bTopMode;
    bool b4BytesMode;
    int iDoneMode;  // 0 by rest length, 1 by busy
    int ioMode;     // 0 1-1-1 mode, 1: 1-4-4 mode (command 1 bits, address 4 bits, data 4 bits)
}SfcInfo,*pSfcInfo;*/

/**
 * @brief dvProAV_FlashCsrRead
 * @param sfcinfo: seail flash information
 * @param csrAddr
 * @return CSR Data
 */
int dvProAV_FlashCsrReadData(uint08 csrAddr);

/**
 * @brief Set Serial Flash to 4 byte addressing mode
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiEnter4byteAddressingMode();

/**
 * @brief Read Serial Flash Memory
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiReadMemoryCmd(bool bMode, bool b4BytesMode);

/**
 * @brief Write Serial Flash Memory
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiWriteMemoryCmd(bool bMode, bool b4BytesMode);

/**
 * @brief Protection All Serial Flash Sector
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiAllSectorProtection();

/**
 * @brief Un-protection All Serial Flash Sector
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiAllSectorUnprotect();

/**
 * @brief Erase Serial Flash Sector
 * @param SfcInfo: seail flash information
 * @param address: sector address
 * @param length: erase length(bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiEraseSector(uint32_t address, uint32 length);

/**
 * @brief Erase Serial Flash All Sector
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiEraseAll();

/**
 * @brief Serial Flash Read Delay Set
 * @param SfcInfo: seail flash information
 * @param ucDly: delay "ucDly" clock cycle
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiReadDlySet(uint32 ulDly);

/**
 * @brief Serial Flash Read Delay Set
 * @param SfcInfo: seail flash information
 * @param ulSpiClockRate: seail flash SPI clock rate
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiClkRateSet(uint32 ulSpiClockRate);

/**
 * @brief Serial Flash Select
 * @param [in] bFlashSel :
 *              0: common FLASH
 *              1: boot FLASH
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiFlashSel(bool bFlashSel);

/**
 * @brief Serial Flash Initialize
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiInit(bool bMode, uint32 u32CsDelay, uint32 u32SpiClockRate);

#endif
