#ifndef MIRCON_FLASH_DEF_H
#define MIRCON_FLASH_DEF_H

#define MIRCON_SPI_FLASH
// Macronix serial flash definition
typedef enum{
    SFLASH_WREN        = 0x06, // WriteEnable
    SFLASH_WRDI        = 0x04, // WriteDisable
    SFLASH_RDSR        = 0x05, // ReadStatus
    SFLASH_RDFSR       = 0x70, // read flag status register
    SFLASH_WRSR        = 0x01, // WriteStatus
    SFLASH_RDID        = 0x9F, // ReadID
    SFLASH_QPIID       = 0xaf, // QPI ID read (Multiple I/O Read ID)
    SFLASH_SFDP_READ   = 0x5a, // enter SFDP read mode (Read Serial Flash Discovery Parameter)
    SFLASH_1X          = 0x03, // ReadData
    SFLASH_1X4B        = 0x13, // ReadData by 4 byte address
    SFLASH_FASTREAD1X  = 0x0b, // FastReadData
    SFLASH_FASTREAD4B  = 0x0c, // FastReadData by 4 byte address
    SFLASH_READ2X      = 0xbb, // 2X Read
    SFLASH_READ2X4B    = 0xbc, // 2X Read by 4 byte address
    SFLASH_DREAD       = 0x3b, // Fastread dual output;
    SFLASH_DREAD4B     = 0x3c, // Fastread dual output by 4 byte address;
    SFLASH_4X          = 0xeb, // 4XI/O Read;
    SFLASH_4X4B        = 0xec, // 4XI/O Read by 4 byte address;
    SFLASH_QREAD       = 0x6b, // Fastread quad output;
    SFLASH_QREAD4B     = 0x6c, // Fastread quad output by 4 byte address;
    SFLASH_DTRREAD4X   = 0xed, // Quad DDR read 4XI/O Read;
    SFLASH_DTRREAD4X4B = 0xee, // Quad DDR read 4XI/O Read 4byte;
    SFLASH_PP          = 0x02, // PageProgram
    SFLASH_PP4B        = 0x12, // PageProgram by 4 byte address
    SFLASH_PP4X        = 0x34, // 4 byte quad input fast program
    SFLASH_PP4X4B      = 0x3e, // 4 byte quad input extended fast program
    SFLASH_SE          = 0x20, // Sector(4K Subsector) Erase
    SFLASH_SE4B        = 0x21, // Sector(4K Subsector)Erase by 4 byte address
    SFLASH_BE32K       = 0x52, // 32k block erase
    SFLASH_BE32K4B     = 0x5c, // 32k block erase by 4 byte address
    SFLASH_BE          = 0xd8, // Block(Sector)Erase
    SFLASH_BE4B        = 0xdc, // Block(Sector)Erase by 4 byte address
    SFLASH_CE          = 0xc4, // (Die Erase) Chip Erase 
    SFLASH_EQIO        = 0x35, // enable quad I/O
    SFLASH_RSTQIO      = 0xf5, // reset quad I/O
    SFLASH_EN4B        = 0xb7, // enter 4-byte mode
    SFLASH_EX4B        = 0xe9, // exit 4-byte mode
    SFLASH_RDEAR       = 0xc8, // Read  extended address register;
    SFLASH_WREAR       = 0xc5, // Write extended address register;
    SFLASH_RSTEN       = 0x66, // reset enable
    SFLASH_RST         = 0x99, // reset memory
    SFLASH_NOP         = 0x00, // no operation
    SFLASH_DP          = 0xb9, // DeepPowerDown
    SFLASH_RDP         = 0xab, // ReleaseFromDeepPowerDown
    SFLASH_WRLR        = 0x2c, // write lock register
    SFLASH_RDLR        = 0x2d, // read lock register
    SFLASH_WRPASS      = 0x28, // Write password register;
    SFLASH_PASSULK     = 0x29, // Password unlock;
    SFLASH_RDPASS      = 0x27, // Read password register;
}SPI_FLASH_CMD;

#define MASK_SFLASH_WIP 0x00000001    // set mask to check Write in Progress (WIP) bit to determine device busy or ready
#define MASK_SFLASH_PRO 0x0000003c    // set BP3-0 status register to protect all sectors
#define MASK_SFLASH_WEL 0x00000002    // set mask to check write enable latch

#define SFLASH_CMD_TOP  SFLASH_RDSR
#define SFLASH_MSK_TOP  0x40

#define SFLASH_CMD_4BYTE  SFLASH_RDFSR
#define SFLASH_MSK_4BYTE  0x01

#endif //MIRCON_FLASH_DEF_H
