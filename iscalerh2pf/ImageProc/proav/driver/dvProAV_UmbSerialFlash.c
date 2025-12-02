#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include <time.h>
#include <stdint.h>
#include "dvProAV_Base.h"
#include "dvProAV_Scaler.h"
#include "MacronixFlash.h"
#include "dvProAV_UmbSerialFlash.h"
//#include "MirconFlash.h"

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif

#define REG_CONTROL        0x00
#define PACK_REG_CONTROL(four_byte, chip_sel, out_hi_impedance) ((four_byte&0x01)<<8 |(chip_sel&0x0F)<<4 | (out_hi_impedance&0x01))
// Offset 0: Control Register
// [8] = 0:3 byte addressing, 1: 4 byte address
// [7:4] = Chip select
// [0] = output high impedance

#define REG_SPI_CLOCK_BRT  0x01
#define PACK_REG_SPI_CLOCK_BRT(divisor, cs_assert) (divisor&0x0F)
// Offset 1: SPI Clock Baud-rate Register
// [4:0] = Baud rate divisor, 0x00: /2, 0x01:/4 ..., 0x10:/32
#define REG_CS_DELAY_SET   0x02
#define PACK_REG_CS_DELAY_SET(cs_deassert, cs_assert) ((cs_deassert&0x0F)<<4 | (cs_assert&0x0F))
// Offset 2: CS Delay Setting Register
// [7:4] = CS de-assert
// [3:0] = CS assert

#define REG_READ_CAPTURE   0x03
#define PACK_REG_READ_CAPTURE(read_delay) (read_delay & 0x0F)
// Offset 3: Read Capturing Register
// [3:0] = Read delay

#define REG_OP_PROTOC_SET  0x04
#define PACK_REG_OP_PROTOC_SET(read_data, read_addr, write_data, write_addr, instruction) ((read_data & 0x0F)<<16 | (read_addr& 0x03)<<12 | (write_data&0x03)<<8 | (write_addr&0x03)<<4 | (instruction&0x03))
// Offset 4: Operating Protocols Setting Register
// [17:16] = Read data out transfer mode
// [13:12] = Read address transfer mode
// [9:8] = Write data out transfer mode
// [5:4] = Write address transfer mode
// [1:0] = Instruction transfer mode

#define REG_READ_INSTRU    0x05
#define PACK_REG_READ_INSTRU(dummy_cycles, read_opcode) ((dummy_cycles & 0x1FF)<<8 | (read_opcode&0xFF))
// Offset 5: Read Instruction Register
// [12:8] = Dummy cycles
// [7:0] = Read opcode

#define REG_WRITE_INSTRU   0x06
#define PACK_REG_WRITE_INSTRU(polling_opcode, write_opcode) ((polling_opcode & 0xFFF)<<8 | (write_opcode&0xFF))
// Offset 6: Write Instruction Register
// [15:8] = Polling opcode
// [7:0] = Write opcode

#define REG_FLASH_CMD_SET  0x07
#define PACK_REG_FLASH_CMD_SET(dummy_count, data_count, data_type, address_count, opcode) ((dymmy & 0x0F)<<16 | (data_count& 0x0F)<<12 | (data_type&0x01)<<11 | (address_count&0x07)<<8 | (opcode&0xFF))
// Offset 7: Flash Command Setting Register
// [20:16] = Number of dummy cycles
// [15:12] = Number of data bytes
// [11] = Data type
// [10:8] = Number of address bytes
// [7:0] = Opcodee
// 31.. 27.. 23.. 19.. 15.. 11.. 7..4 3..0
// 0000-0000-0000-0000-0100-1000-0000-0101 ==> data count =4,  data type = 1, address byte=0, opcode=1

#define REG_FLASH_CMD_CTL  0x08
#define PACK_REG_FLASH_CMD_CTL(start) (start & 0x01)
// Offset 8: Flash Command Control Register
// [0] = Start
#define REG_FLASH_CMD_ADDR 0x09
#define PACK_REG_FLASH_CMD_ADDR(start_addr) (start_addr)
// Offset 9: Flash Command Address Register
// [31:0] = Stating address
#define REG_FLASH_CMD_WD0  0x0A
#define PACK_REG_FLASH_CMD_WD0(lower_data) (lower_data)
// Offset 0x0A: Flash Command Write Data 0 Register
// [31:0] = Lower 4 bytes write data
#define REG_FLASH_CMD_WD1  0x0B
#define PACK_REG_FLASH_CMD_WD1(upper_data) (upper_data)
// Offset 0x0B: Flash Command Write Data 1 Register
// [31:0] = Upper 4 bytes Read data
#define REG_FLASH_CMD_RD0  0x0C
#define PACK_REG_FLASH_CMD_RD0(lower_addr) (lower_addr)
// Offset 0x0C: Flash Command Read Data 0 Register
// [31:0] = Upper 4 bytes Read data
#define REG_FLASH_CMD_RD1  0x0D
#define PACK_REG_FLASH_CMD_RD1(upper_addr) (upper_addr)
// Offset 0x0D: Flash Command Read Data 1 Register
// [31:0] = Upper 4 bytes Read data


static int _UmbFlashCsrWrite(uint08 csrAddr, uint32 wrData)
{
    int status = rcSUCCESS;
    uint08 ucData[4];
    ucData[0] = wrData & 0xff;
    ucData[1] = (wrData >> 8) & 0xff;
    ucData[2] = (wrData >> 16) & 0xff;
    ucData[3] = (wrData >> 24) & 0xff;

    status &= dvProAV_UmbFlashCsrAddrSet((csrAddr & 0x3F)); // Bit [5:0]
    status &= dvProAV_UmbFlashCsrWrite(4, ucData);
    status &= dvProAV_UmbFlashCsrCmdSet(0x01); // 0x01: Write Mode, add this line to fixed two clock domain issue
    status &= dvProAV_UmbFlashCsrCmdSet(0x03); // 0x03: Write Start
    status &= dvProAV_UmbFlashCsrCmdSet(0x00); // 0x00: Finished
    return status;
}

static int _UmbFlashCsrRead(uint08 csrAddr)
{
    int status = rcSUCCESS;
    uint32 rdData;
    uint08 ucData[4];

    status &= dvProAV_UmbFlashCsrAddrSet((csrAddr & 0x3F)); // Bit [5:0]
    status &= dvProAV_UmbFlashCsrCmdSet(0x00); // 0x00: Read Mode, add this line to fixed two clock domain issue
    status &= dvProAV_UmbFlashCsrCmdSet(0x02); // 0x02: Read Start
    status &= dvProAV_UmbFlashCsrCmdSet(0x00); // 0x00: Finished
    status &= dvProAV_UmbFlashCsrRead(4, ucData);

    rdData = ucData[3];
    rdData <<= 8;
    rdData |= ucData[2];
    rdData <<= 8;
    rdData |= ucData[1];
    rdData <<= 8;
    rdData |= ucData[0];
    return (int)rdData;
}

int dvProAV_FlashCsrReadData(uint08 csrAddr)
{
    return _UmbFlashCsrRead(csrAddr);
}

static int _UmbSfiCmdDataWrite(uint8_t opcode,  uint32_t data, uint8_t dummy_count, uint8_t data_count)
{
    int status = rcSUCCESS;
    uint8_t data_type = 0;
    uint8_t address_count = 0;
    uint32 cmd = (uint32_t)(dummy_count & 0x0F)<<16 | ((uint32_t)(data_count& 0x0F))<<12 | ((uint32_t)(data_type&0x01))<<11 | ((uint32_t)(address_count&0x07))<<8 | (uint32_t)(opcode&0xFF);
    DBMSG("SfiCmdDataWrite:  opcode=0x%02X, data=0x%02X, dummy_count=%d, data_count=%d, address_count = %d\n", opcode, data, dummy_count, data_count, address_count);
    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_SET, cmd);
    if (data_count > 0) status &= _UmbFlashCsrWrite(REG_FLASH_CMD_WD0, data);
    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_CTL, 0x1);

    return status;
}

static int _UmbSfiCmdDataReadDataWrite(uint8_t opcode,  uint32_t *data_ptr, uint8_t dummy_count, uint8_t data_count)
{
    int status = rcSUCCESS;
    uint8_t data_type = 1;
    uint8_t address_count = 0;
    uint32 cmd = (uint32_t)(dummy_count & 0x0F)<<16 | ((uint32_t)(data_count& 0x0F))<<12 | ((uint32_t)(data_type&0x01))<<11 | ((uint32_t)(address_count&0x07))<<8 | (uint32_t)(opcode&0xFF);

    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_SET, cmd);
    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_CTL, 0x1);
    if (data_count > 0)  *data_ptr = _UmbFlashCsrRead(REG_FLASH_CMD_RD0);

    DBMSG("SfiCmdDataRead:  opcode=0x%02X, data=0x%02X, dummy_count=%d, data_count=%d, address_count = %d\n", opcode, *data_ptr, dummy_count, data_count, address_count);
    return status;
}

static int _UmbSfiCmdAddrWrite(uint8_t opcode, uint32_t address, uint8_t dummy_count, uint8_t address_count)
{
    int status = rcSUCCESS;
    uint8_t data_type = 0; //  For Read
    uint8_t data_count = 0;
    uint32 cmd = (uint32_t)(dummy_count & 0x0F)<<16 | ((uint32_t)(data_count& 0x0F))<<12 | ((uint32_t)(data_type&0x01))<<11 | ((uint32_t)(address_count&0x07))<<8 | (uint32_t)(opcode&0xFF);
    DBMSG("SfiCmdAddrWrite:  opcode=0x%02X, address=0x%02X, dummy_count=%d, data_count=%d, address_count = %d\n", opcode, address, dummy_count, data_count, address_count);
    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_SET, cmd);
    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_ADDR, address);
    status &= _UmbFlashCsrWrite(REG_FLASH_CMD_CTL, 0x1);

    return status;
}

#if 0 // unused
static int _SfiCmdAddrRead(SfcInfo *sfcinfo, uint8_t opcode, uint32_t *addr_ptr, uint8_t dummy_count, uint8_t address_count)
{
    uint8_t data_type = 1; // 1 For Read
    uint8_t data_count = 0;
    uint32 cmd = (uint32_t)(dummy_count & 0x0F)<<16 | ((uint32_t)(data_count& 0x0F))<<12 | ((uint32_t)(data_type&0x01))<<11 | ((uint32_t)(address_count&0x07))<<8 | (uint32_t)(opcode&0xFF);
    _UmbFlashCsrWrite(sfcinfo, REG_FLASH_CMD_SET, cmd);
    _UmbFlashCsrWrite(sfcinfo, REG_FLASH_CMD_CTL, 0x1);
    *addr_ptr = _FlashCsrRead(sfcinfo, REG_FLASH_CMD_ADDR);

    DBMSG("SfiCmdAddrRead:  opcode=0x%02X, address=0x%02X, dummy_count=%d, data_count=%d, address_count = %d\n", opcode, *addr_ptr, dummy_count, data_count, address_count);
    return 0;
}
#endif

//Read Memory Commands
static int  _UmbSfiMemoryReadStart(bool bMode, bool b4BytesMode)
{
    int status = rcSUCCESS;
    uint32_t mode;
    uint32_t data;
    if (bMode == 0)
    {
        mode = 0x00000000;
        data = b4BytesMode ? SFLASH_1X4B: SFLASH_1X; // Read opcode:0x03 // for 3 bytes addressing mode, polling opcode = 0x05
    }
    else
    {
        uint32_t dummyCycle = 10;
        mode = 0x00022000;
        data = dummyCycle;
        data <<= 8;
        data |= b4BytesMode ? SFLASH_4X4B: SFLASH_4X; // Read opcode:0x03 // for 3 bytes addressing mode, polling opcode = 0x05
    }
    status &= _UmbFlashCsrWrite(REG_OP_PROTOC_SET, mode);
    status &= _UmbFlashCsrWrite(REG_CONTROL, b4BytesMode ? 0x00000101 : 0x00000001); // 3 or 4 byte address mode , CS0, Enable

    status &= _UmbFlashCsrWrite(REG_READ_INSTRU, data); // Read opcode:0x03 // for 3 bytes addressing mode, polling opcode = 0x05
    //DBMSG("%s, Read Address Mode =%08X, \n", sfcinfo->acName, data);
    return status;
}

//Page Program Commands
static int _UmbSfiMemoryWriteStart(bool bMode, bool b4BytesMode)
{
    int status = rcSUCCESS;
    uint32_t mode;
    uint32_t data;
    if (bMode == 0)
    {
        mode = 0x00000000;
        data = b4BytesMode ? (0x00000500 | SFLASH_PP4B) : (0x00000500 | SFLASH_PP); // Write opcode:0x02/0x12  // for 3 or 4 bytes addressing mode, polling opcode = 0x05
    }
    else
    {
        mode = 0x00000220;
        data = b4BytesMode ? (0x00000500 | SFLASH_PP4X): (0x00000500 | SFLASH_PP4X); //Polling opcode = 0x05
    }
    status &= _UmbFlashCsrWrite(REG_OP_PROTOC_SET, mode);
    status &= _UmbFlashCsrWrite(REG_CONTROL, b4BytesMode ? 0x00000101 : 0x00000001); // 3 or 4 byte address mode , CS0, Enable
    status &= _UmbFlashCsrWrite(REG_WRITE_INSTRU, data);
    //DBMSG("%s, Write Address Mode =%08X, \n", sfcinfo->acName, data);
    return status;
}

inline static int _UmbQuadModeEnable(bool enable)
{
    int status = rcSUCCESS;

    if (enable)
        status &= _UmbSfiCmdDataWrite(SFLASH_EQIO, 0, 0, 0);
    else
        status &= _UmbSfiCmdDataWrite(SFLASH_RSTQIO, 0, 0, 0);
    return status;
}

inline static int _UmbMarconixQuadEnableCmd(bool enable)
{
    int status = rcSUCCESS;
    uint32 data;

    status &= _UmbSfiCmdDataReadDataWrite(SFLASH_RDSR, &data, 0, 1);
    if (enable)
        data |= 0x40;
    else
        data &= ~0x40;

    status &= _UmbSfiCmdDataWrite(SFLASH_WRSR, data, 0, 1);
    return status;
}

inline static int _UmbMicronQuadEnableCmd(bool enable)
{
    int status = rcSUCCESS;
    uint32 data;

    status &= _UmbSfiCmdDataReadDataWrite(0x65, &data, 0, 1); // 0x65 Read Enhanced Volatile Configuration Register
    if (enable)
        data |= 0x80;
    else
        data &= ~0x80;

    status &= _UmbSfiCmdDataWrite(0x61, data, 0, 1); // 0x61 Read Enhanced Volatile Configuration Register
    return status;
}

inline static uint32_t _UmbReadDeviceID()
{
    uint32_t data;
    _UmbSfiCmdDataReadDataWrite(SFLASH_RDID, &data, 0, 4);
    return data;
}

inline static uint32_t _UmbReadStatusRegister(){
    uint32_t data;
    _UmbSfiCmdDataReadDataWrite(SFLASH_RDSR, &data, 0, 2);
    return data;
}

inline static bool _UmbIsWriteInProgress()
{
    uint32_t status = _UmbReadStatusRegister();
    return ((status & MASK_SFLASH_WIP) == MASK_SFLASH_WIP); // 1: Write in progress (WIP) bit ( bit 0 of status register)
}

inline static bool _UmbIsWriteEnableLatch()
{
    uint32_t status = _UmbReadStatusRegister();
    if ((status & MASK_SFLASH_WEL) != MASK_SFLASH_WEL)
    {
        DBMSG("write enable latch do not set\n");
        return false;
    }
    return true;
}

inline static bool _UmbIsAllBlockProtected()
{
    uint32_t status = _UmbReadStatusRegister();
    if ((status & MASK_SFLASH_PRO)!= MASK_SFLASH_PRO)
    {
        DBMSG("All sectors in this configuration device is not protected\n");
        return false;
    }
    DBMSG("All sectors in this configuration device is protected\n");
    return true;
}

inline static int _UmbWriteEnableCmd()
{
    return _UmbSfiCmdDataWrite(SFLASH_WREN, 0, 0, 0);
}

inline static int _UmbWriteDisableCmd()
{
    return _UmbSfiCmdDataWrite(SFLASH_WRDI, 0, 0, 0);
}

//Erase Commands
inline static int _UmbEraseBlock(uint32_t address)
{
     return _UmbSfiCmdAddrWrite(SFLASH_BE, address, 0, 4); // SFLASH_BE: 0xD8 is Micron block erase(64K) command as the Marconix block erase(64K) command
}

inline static int _UmbEraseChip(){
     return _UmbSfiCmdDataWrite(SFLASH_CE, 0, 0, 0);
}

/**
 * @brief Set Serial Flash to 4 byte addressing mode
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiEnter4byteAddressingMode()
{
    return _UmbSfiCmdDataWrite(SFLASH_EN4B, 0, 0, 0);
}

/**
 * @brief Read Serial Flash Memory
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiReadMemoryCmd(bool bMode, bool b4BytesMode)
{
     return _UmbSfiMemoryReadStart(bMode, b4BytesMode);
}

/**
 * @brief Write Serial Flash Memory
 * @param SfcInfo: seail flash information
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiWriteMemoryCmd(bool bMode, bool b4BytesMode){
     return _UmbSfiMemoryWriteStart(bMode, b4BytesMode);
}

//Block Protection Commands
inline static int _UmbAllBlockUnprotectCmd()
{
    int status = rcSUCCESS;
    uint32 data;

    status &= _UmbSfiCmdDataReadDataWrite(SFLASH_RDSR, &data, 0, 1);
    data &= ~0x3C;
    status &= _UmbSfiCmdDataWrite(SFLASH_WRSR, 0x00, 0, 1); //sector unprotect all sector(BP3:BP2:BP1:BP0=0:0:0:0)
    return status;
}

inline static int _UmbAllBlockProtectCmd()
{
    int status = rcSUCCESS;
    uint32 data;

    status &= _UmbSfiCmdDataReadDataWrite(SFLASH_RDSR, &data, 0, 1);
    data |= 0x3C;
    status &= _UmbSfiCmdDataWrite(SFLASH_WRSR, data, 0, 1); //sector protect all sector(BP3:BP2:BP1:BP0=1:1:1:1)
    return status;
}

/**
 * @brief Protection All Serial Flash Sector
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiAllSectorProtection()
{
    uint08 ucTimerId;

    if (TimerRequire(&ucTimerId) == false)
        return rcERROR;

    //DBMSG("Now performing sector protection...\n");

    // a. Performs write enable command.
    _UmbWriteEnableCmd();

    // b. Performs write status register command to clear BP bit and TB bit.
    _UmbAllBlockProtectCmd(); //sector protect all sector(BP3:BP2:BP1:BP0=1:1:1:1)

    // c. Polls WIP bit ( bit 0 of status register) until it returns a 0 (ready).
    ResetTime(ucTimerId);
    while (_UmbIsWriteInProgress()){
        DBMSG("Write register for sector protect in progress...\n");
        if (TimeElapsed(ucTimerId)>100000)
        {
             DBMSG("Write register for sector unprotect over %d msec\n", 100000);
             TimerRelease(ucTimerId);
             return rcERROR;
        }
    }
    TimerRelease(ucTimerId);

    // d. Performs read status register command to check whether BP bit and TB bit has succeeded clear.
    _UmbWriteDisableCmd();
    return rcSUCCESS;
}

/**
 * @brief Un-protection All Serial Flash Sector
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiAllSectorUnprotect()
{
    uint08 ucTimerId;

    if (TimerRequire(&ucTimerId) == false)
        return rcERROR;

   //a. Performs write enable command.
   //b. Performs write status register command to clear BP bit and TB bit.
   //c. Polls WIP bit ( bit 0 of status register) until it returns a 0 (ready).
   //d. Performs read status register command to check whether BP bit and TB bit	has succeeded clear.

    _UmbWriteEnableCmd();

    if (_UmbIsWriteEnableLatch() == false) {
        return rcERROR;
    }

    _UmbAllBlockUnprotectCmd(); //sector unprotect all sector(BP3:BP2:BP1:BP0=0:0:0:0)
    ResetTime(ucTimerId);
    while (_UmbIsWriteInProgress()){
        DBMSG("Write register for sector unprotect in progress...\n");
        if (TimeElapsed(ucTimerId)>100000)
        {
             DBMSG("Write register for sector unprotect over %d msec\n", 100000);
             TimerRelease(ucTimerId);
             return rcERROR;
        }
    }
    TimerRelease(ucTimerId);

    return rcSUCCESS;
}

/**
 * @brief Erase Serial Flash Sector
 * @param address: sector address
 * @param length: erase length(bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiEraseSector(uint32_t address, uint32 length)
{
    uint08 ucTimerId;
    int status = rcSUCCESS;
    if (TimerRequire(&ucTimerId) == false)
        return rcERROR;

    dvProAV_UmbSfiAllSectorUnprotect();

    dvProAV_UmbSfiEnter4byteAddressingMode();

    //a. Performs write enable command.
    _UmbWriteEnableCmd();

    //b. Performs sector erase command.
    // Micron block erase(64K) command as the Marconix block erase(64K) command
    for (uint32_t adr = address; adr<(address+length); adr+=FLASH_SECTOR_SIZE)
    {
        _UmbEraseBlock(adr - (adr%FLASH_SECTOR_SIZE));
    }

    ResetTime(ucTimerId);

    //c. Polls write in progress (WIP) bit ( bit 0 of status register) until it return a 0 (ready).
    while (_UmbIsWriteInProgress()){   //wait sector erase to end before proceed to next
        DBMSG("Sector erase in progress...\n");
        DelayMSec(100);
        if (TimeElapsed(ucTimerId)> 100000)
        {
            DBMSG("Sector erase over %d msec\n", 100000);
            status = rcERROR;
            break;
        }
    }

    //d. Performs read status register to check whether erase operation succeeded or failed.
    TimerRelease(ucTimerId);
    dvProAV_UmbSfiAllSectorProtection();
    return status;
}

/**
 * @brief Erase Serial Flash All Sector
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiEraseAll()
{
    uint08 ucTimerId;
    int status = rcSUCCESS;
    if (TimerRequire(&ucTimerId) == false)
        return rcERROR;

    dvProAV_UmbSfiAllSectorUnprotect();

    //a. Performs write enable command.
    _UmbWriteEnableCmd();

    //b. Performs die erase command.
    _UmbEraseChip();

    //c. Polls write in progress (WIP) bit ( bit 0 of status register) until it return a 0 (ready).
    ResetTime(ucTimerId);
    while (_UmbIsWriteInProgress()){   //wait sector erase to end before proceed to next
        DBMSG("Sector erase all in progress...\n");
        DelayMSec(100);
        if (TimeElapsed(ucTimerId)> 100000)
        {
            DBMSG("Sector erase all  over %d msec\n", 100000);
            status = rcERROR;
            break;
        }

    }
    TimerRelease(ucTimerId);

    //d. Performs read status register to check whether erase operation succeeded or failed.
    dvProAV_UmbSfiAllSectorProtection();
    return status;
}

/**
 * @brief Serial Flash Read Delay Set
 * @param [in] ucDly: delay "ucDly" clock cycle
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiReadDlySet(uint32 ulDly)
{
    int status = rcSUCCESS;
    _UmbFlashCsrWrite(REG_READ_CAPTURE, ulDly);
    return status;
}

/**
 * @brief Serial Flash Read Delay Set
 * @param [in] ulSpiClockRate: serail flash SPI clock rate
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiClkRateSet(uint32 ulSpiClockRate)
{
    int status = rcSUCCESS;
    _UmbFlashCsrWrite(REG_SPI_CLOCK_BRT, ulSpiClockRate); // set SPI clock
    return status;
}

/**
 * @brief Serial Flash Select
 * @param [in] bFlashSel :
 *              0: common FLASH
 *              1: boot FLASH
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiFlashSel(bool bFlashSel)
{
    int status = rcSUCCESS;
    status &= dvProAV_UmbFlashSel(bFlashSel);
    return status;
}

/**
 * @brief Serial Flash Initialize
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbSfiInit(bool bIoMode, uint32 u32CsDelay, uint32 u32SpiClockRate)
{
    uint32_t data;

    dvProAV_UmbFlashIpRst();

    dvProAV_UmbRst(eDramBank0);
    dvProAV_UmbRst(eDramBank1);

    DelayMSec(500);

    _UmbFlashCsrWrite(REG_CS_DELAY_SET, u32CsDelay); // set CS assert & de-assert both delay
    _UmbFlashCsrWrite(REG_SPI_CLOCK_BRT, u32SpiClockRate); // set SPI clock
    data = _UmbReadDeviceID();

    if (data == 0xc23b25c2 || data == 0xc23b95c2) // Mxic 0xc23b25c2:SOP , 0xc23b95c2:BGA
    {
        DBMSG("Mxic flash initialize, id =%08X\n", data);
        if (bIoMode == 1)
        {
            _UmbMarconixQuadEnableCmd(true);
        }
    }
    else if (data == 0x1021bb20)
    {
        DBMSG("Micron flash initialize, id =%08X\n", data);
        if (bIoMode == 1)
        {
            _UmbMicronQuadEnableCmd(true);
        }
    }
    else // Unknow Flash
    {
        DBMSG("Unknow Flash id =%08X\n", data);
    }
    _UmbWriteDisableCmd();

    dvProAV_UmbSfiEnter4byteAddressingMode();

    _UmbIsAllBlockProtected();

#if 0
    _UmbSfiCmdDataReadDataWrite(SFLASH_CMD_TOP, &data, 0, 1);
    m_top_mode = (data & SFLASH_MSK_TOP) ? false : true;
    if (m_top_mode)
        DBMSG("Top Protected Mode\n");
    else
        DBMSG("Bottom Protected Mode\n");

    _UmbSfiCmdDataReadDataWrite(SFLASH_CMD_4BYTE, &data, 0, 1);
    m_4byte_mode = (data & SFLASH_MSK_4BYTE) ? true : false;
    if (m_4byte_mode)
        DBMSG("4 Byte Addressing Mode\n");
    else
        DBMSG("3 Byte Addressing Mode\n");
#else
    // don't check the top protected mode and m_4byte mode
    // the Mircon flash can use the MacronixFlash.h and without change to MirconFlash.h, because the have same cmds' id for the actived commands
    //sfcinfo->bTopMode = true;
    //sfcinfo->b4BytesMode = true;
#endif

    return rcSUCCESS;
}
