#ifndef PROAV_GENERICSERIALFLASH_H
#define PROAV_GENERICSERIALFLASH_H

#include "dvProAV_Base.h"
#include "stdbool.h"

typedef int (*pfFlashCsrWrite)(uint08 csr, uint32 data);
typedef uint32 (*pfFlashCsrRead)(uint08 csr);

typedef struct {
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
}SfcInfo,*pSfcInfo;

/**
 * @brief Set Serial Flash to 4 byte addressing mode
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiEnter4byteAddressingMode(SfcInfo *sfcinfo);

/**
 * @brief Read Serial Flash Memory
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiReadMemoryCmd(SfcInfo *sfcinfo);

/**
 * @brief Write Serial Flash Memory
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiWriteMemoryCmd(SfcInfo *sfcinfo);

/**
 * @brief Protection All Serial Flash Sector
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiAllSectorProtection(SfcInfo *sfcinfo);

/**
 * @brief Un-protection All Serial Flash Sector
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiAllSectorUnprotect(SfcInfo *sfcinfo);

/**
 * @brief Erase Serial Flash Sector
 * @param SfcInfo: seail flash information
 * @param address: sector address
 * @param length: erase length(bytes)
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiEraseSector(SfcInfo *sfcinfo, uint32_t address, uint32 length);

/**
 * @brief Erase Serial Flash All Sector
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiEraseAll(SfcInfo *sfcinfo);

/**
 * @brief Serial Flash Read Delay Set
 * @param SfcInfo: seail flash information
 * @param ucDly: delay "ucDly" clock cycle
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiReadDlySet(SfcInfo *sfcinfo, uint32 ulDly);

/**
 * @brief Serial Flash Read Delay Set
 * @param SfcInfo: seail flash information
 * @param ulSpiClockRate: seail flash SPI clock rate
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiClkRateSet(SfcInfo *sfcinfo, uint32 ulSpiClockRate);

/**
 * @brief Serial Flash Initialize
 * @param SfcInfo: seail flash information
 * @return rcSUCCESS or rcERROR
 */
int dvProAV_GenericSfiInit(SfcInfo *sfcinfo);

#endif
