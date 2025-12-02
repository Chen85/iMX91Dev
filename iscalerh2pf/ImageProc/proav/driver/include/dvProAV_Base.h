#ifndef DV_PROAV_BASE_H
#define DV_PROAV_BASE_H

#include "dvProAV_Platform.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"

#define ACCESS_WRITE_CHECK_ENABLED (1)
#define IO_READ  0
#define IO_WRITE 1

#define MEMORYBUFMAXBYTE 32768  // 512x64(byte)

//-------------------------------------------------------------------------------------------------
// MEM Calcualte Marco
//-------------------------------------------------------------------------------------------------
#define CAL_MEM_BASE(start, useSize, min) ((((start) + (useSize)) + (min) - 1) / (min) * (min) * 1UL)
#define CAL_MEM_SIZE(useSize, min) (((useSize) + (min) - 1)/(min) * (min) * 1UL)

//-------------------------------------------------------------------------------------------------
// RSU SRAM Parameter definition
//-------------------------------------------------------------------------------------------------
#define RSU_SRAM_DATA_WIDTH    32     // 32 bits
#define RSU_SRAM_LINE_BYTES    (RSU_SRAM_DATA_WIDTH / 8)
#define RSU_SRAM_ADDR_SHIFT    0
#define RSU_SRAM_MIN_BYTES     (RSU_SRAM_LINE_BYTES << RSU_SRAM_ADDR_SHIFT)
#define RSU_SRAM_BANK_SIZE     (4 * 1024UL)                     // 4096 Bytes

//-------------------------------------------------------------------------------------------------
// DRAM Parameter definition
//-------------------------------------------------------------------------------------------------
#define PROAV_DRAM_DATA_WIDTH    512     // 512 bits
#define PROAV_DRAM_LINE_BYTES    (PROAV_DRAM_DATA_WIDTH / 8)
#define PROAV_DRAM_ADDR_SHIFT    1
#define PROAV_DRAM_MIN_BYTES     (PROAV_DRAM_LINE_BYTES << PROAV_DRAM_ADDR_SHIFT)    // since the dram addrees is from bit 1(shift 1bit)
#define PROAV_DRAM_BANK_SIZE     (2 * 1024 * 1024 * 1024UL)                          // 2G Bytes => address = 0x2000000(0x0000000 ~ 0x1FFFFFF), 2^25(A0 ~ A24), Data Bus

//-------------------------------------------------------------------------------------------------
// FLASH Parameter definition
//-------------------------------------------------------------------------------------------------
#define FLASH_SECTOR_SIZE  0x10000       // 64 KB
#define FLASH_ADDRESS_BASE 64
#define FLASH_MEM_SIZE     0x08000000    // 128 MB
#define FLASH_INFO_SIZE    0x400         // 1 KB

//-------------------------------------------------------------------------------------------------
// Data size definition (Byte base)
//-------------------------------------------------------------------------------------------------
#define OSD_H_MAX               4096
#define OSD_V_MAX               2400
#define OSD_PIX_MAX             (OSD_H_MAX * OSD_V_MAX)
#define WPOSD_H_MAX             4096
#define WPOSD_V_MAX             2400
#define WPOSD_PIX_MAX           (WPOSD_H_MAX * WPOSD_V_MAX)
#define OSD_COLOR_BYTES         (16/8)
#define WND_H_MAX               4096
#define WND_V_MAX               2400
#define WND_PIX_MAX             (WND_H_MAX * WND_V_MAX)
#define WND_3D_H_MAX            2712
#define WND_3D_V_MAX            1528
#define WND_3D_PIX_MAX          (WND_3D_H_MAX * WND_3D_V_MAX)
#define I_WND_H_MAX             2048
#define I_WND_V_MAX             1200
#define I_WND_PIX_MAX           (I_WND_H_MAX * I_WND_V_MAX)
#define VIDEO_COLOR_BYTES       (CAL_MEM_SIZE(3 * 10, 8) / 8)  // 30(RGB 10bits) -> 32
#define LOGO_CAP_COLOR_BYTES    (16/8)
#define OSD_BITMAP_RAW_WIDTH    1920
#define OSD_BITMAP_RAW_HEIGHT   2448
#define WPOSD_BITMAP_RAW_WIDTH  3840

#define OSD_FNT_SIZE            CAL_MEM_SIZE(0x300000, PROAV_DRAM_LINE_BYTES)       // 3 MB
#define OSD_BMP_SIZE            CAL_MEM_SIZE(0x600000, PROAV_DRAM_LINE_BYTES)       // 9 MB (4096x2160 = 8.5MB)
#define WARP_TABLE_SIZE         CAL_MEM_SIZE((WND_H_MAX/16+1) * (WND_V_MAX/16+2) * 4, PROAV_DRAM_MIN_BYTES)
#define HDR_GAMMA_SIZE          CAL_MEM_SIZE(1024 * 4, PROAV_DRAM_LINE_BYTES)
#define HDR_TABLE_SIZE          (HDR_GAMMA_SIZE * 3)
#define GAMMA_TABLE_SIZE        CAL_MEM_SIZE(0x400 * 2 * 3, PROAV_DRAM_LINE_BYTES)  // 0x400 * 2byte * 3 colors Bytes
#define DBD_WEIGHT_TABLE_SIZE   CAL_MEM_SIZE(0x2200000UL, PROAV_DRAM_LINE_BYTES)    // 34 MB
#define SCALING_TABLE_SIZE      CAL_MEM_SIZE(0x800, PROAV_DRAM_LINE_BYTES)          // 2 KB
#define MAIN_WND_SIZE           CAL_MEM_SIZE(WND_PIX_MAX * VIDEO_COLOR_BYTES, PROAV_DRAM_MIN_BYTES)           // Main Window Size = Full Window Size
#define SUB_WND_SIZE            CAL_MEM_SIZE(WND_PIX_MAX * VIDEO_COLOR_BYTES, PROAV_DRAM_MIN_BYTES)           // Sub Window Size = Main Window /2
#define MTN_MEM_SIZE            CAL_MEM_SIZE((I_WND_PIX_MAX * VIDEO_COLOR_BYTES + 5) / 2, PROAV_DRAM_MIN_BYTES) // FACT SIZE = 1920(H) x 1080(V) x 32bit(for 30bit RGB) / 512bit(DRAM Line Size) / 2
#define OSD_MEM_SIZE            CAL_MEM_SIZE(OSD_PIX_MAX * OSD_COLOR_BYTES, PROAV_DRAM_MIN_BYTES)             // 4096 x 2160 x 8bit(for 8bit/256 color) / 512bit
#define LOGO_CAPTURE_2D_SIZE    CAL_MEM_SIZE(WND_PIX_MAX * LOGO_CAP_COLOR_BYTES, PROAV_DRAM_MIN_BYTES)        // Main Window Size = Full Window Size
#define LOGO_CAPTURE_3D_SIZE    CAL_MEM_SIZE(WND_3D_PIX_MAX * LOGO_CAP_COLOR_BYTES, PROAV_DRAM_MIN_BYTES)     // Main Window 3D Size


//-------------------------------------------------------------------------------------------------
// Flash Memory Map
//-------------------------------------------------------------------------------------------------
#define FLASH_OSDTEXT_SIZE OSD_FNT_SIZE
#define FLASH_OSDBMP0_SIZE OSD_BMP_SIZE
#define FLASH_OSDBMP1_SIZE OSD_BMP_SIZE
#define FLASH_OSDBMP2_SIZE OSD_BMP_SIZE

#define FLASH_DATINFO_ADDR          0UL
#define FLASH_OSDTEXT_ADDR          CAL_MEM_BASE(FLASH_DATINFO_ADDR, FLASH_INFO_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_OSDBMP0_ADDR          CAL_MEM_BASE(FLASH_OSDTEXT_ADDR, FLASH_OSDTEXT_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_OSDBMP1_ADDR          CAL_MEM_BASE(FLASH_OSDBMP0_ADDR, FLASH_OSDBMP0_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_OSDBMP2_ADDR          CAL_MEM_BASE(FLASH_OSDBMP1_ADDR, FLASH_OSDBMP1_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_LOGO0_2D_ADDR         CAL_MEM_BASE(FLASH_OSDBMP2_ADDR, FLASH_OSDBMP2_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_LOGO1_2D_ADDR         CAL_MEM_BASE(FLASH_LOGO0_2D_ADDR, LOGO_CAPTURE_2D_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_LOGO2_2D_ADDR         CAL_MEM_BASE(FLASH_LOGO1_2D_ADDR, LOGO_CAPTURE_2D_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_LOGO0_3D_ADDR         CAL_MEM_BASE(FLASH_LOGO2_2D_ADDR, LOGO_CAPTURE_2D_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_LOGO1_3D_ADDR         CAL_MEM_BASE(FLASH_LOGO0_3D_ADDR, LOGO_CAPTURE_3D_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_LOGO2_3D_ADDR         CAL_MEM_BASE(FLASH_LOGO1_3D_ADDR, LOGO_CAPTURE_3D_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_WRPTBL0_ADDR          CAL_MEM_BASE(FLASH_LOGO2_3D_ADDR, LOGO_CAPTURE_3D_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_WRPTBL1_ADDR          CAL_MEM_BASE(FLASH_WRPTBL0_ADDR, WARP_TABLE_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_WRPTBL2_ADDR          CAL_MEM_BASE(FLASH_WRPTBL1_ADDR, WARP_TABLE_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_WRPTBL3_ADDR          CAL_MEM_BASE(FLASH_WRPTBL2_ADDR, WARP_TABLE_SIZE, FLASH_SECTOR_SIZE)
#define FLASH_WRPTBL4_ADDR          CAL_MEM_BASE(FLASH_WRPTBL3_ADDR, WARP_TABLE_SIZE, FLASH_SECTOR_SIZE)
//#define FLASH_BDBDWT0_ADDR CAL_MEM_BASE(FLASH_BLDGMAB_ADDR, GAMMA_TABLE_SIZE, FLASH_SECTOR_SIZE)

#define FLASH_LAST_SIZE  WARP_TABLE_SIZE
#define FLASH_LAST_ADDR  FLASH_WRPTBL4_ADDR
#define FLASH_USED_SIZE  CAL_MEM_BASE(FLASH_LAST_ADDR, FLASH_LAST_SIZE, FLASH_SECTOR_SIZE)

#if (FLASH_USED_SIZE > FLASH_MEM_SIZE)
#error "Flash Memory size is overflow" OSD_BMP_SIZE FLASH_MEM_SIZE
#endif

//-------------------------------------------------------------------------------------------------
// Data size definition
//-------------------------------------------------------------------------------------------------
#define DRAM_BANK0_SIZE         PROAV_DRAM_BANK_SIZE
#define DRAM_WND_MEM_SIZE       (MAIN_WND_SIZE * 2)
#define DRAM_SWND_MEM_SIZE      (SUB_WND_SIZE * 2)
#define DRAM_MTN_MEM_SIZE       MTN_MEM_SIZE
#define DRAM_SMTN_MEM_SIZE      MTN_MEM_SIZE
#define DRAM_HDR_TABLE_SIZE     HDR_TABLE_SIZE
#define DRAM_LOGO_CAPTURE_SIZE  LOGO_CAPTURE_2D_SIZE
#define WPOSD_MEM_SIZE          CAL_MEM_SIZE(WPOSD_PIX_MAX * OSD_COLOR_BYTES, PROAV_DRAM_MIN_BYTES)   // 4096 x 2160 x 8bit(for 8bit/256 color) / 512bit
#define DRAM_BANK0_LAST_MEM_SIZE    WPOSD_MEM_SIZE

#define DRAM_BANK1_SIZE         PROAV_DRAM_BANK_SIZE
#define DRAM_WIP_SIZE           CAL_MEM_SIZE((64 * 0x88000 * 2), PROAV_DRAM_MIN_BYTES)                // 0x88000 DRAM Size, x2才不會蓋到後面位置但尚不知原因
#define DRAM_WOP_SIZE           (DRAM_WIP_SIZE * 4)
#define DRAM_WRP_TABLE_SIZE     WARP_TABLE_SIZE
#define DRAM_DBDBLD_GAIN_SIZE   CAL_MEM_SIZE(64 * WND_PIX_MAX / 48, PROAV_DRAM_MIN_BYTES)             // 8 Bit => WND_PIX_MAX/64, 10 Bit =>  WND_PIX_MAX/48, 取最大值
#define DRAM_DBDBLD_BIAS_SIZE   CAL_MEM_SIZE(64 * WND_PIX_MAX / 128, PROAV_DRAM_MIN_BYTES)            // Main Scaler Window Size x 2
#define DRAM_BANK1_LAST_MEM_SIZE      DRAM_DBDBLD_BIAS_SIZE

//-------------------------------------------------------------------------------------------------
// Dram Bank#0 Memory Map (Scaler)
//-------------------------------------------------------------------------------------------------
#define DRAM_WND1_MEM_ADDR              0UL
#define DRAM_WND2_MEM_ADDR              CAL_MEM_BASE(DRAM_WND1_MEM_ADDR   , DRAM_WND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)    // 補足起始位置1024倍數
#define DRAM_WND3_MEM_ADDR              CAL_MEM_BASE(DRAM_WND2_MEM_ADDR   , DRAM_WND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)    // 補足起始位置1024倍數
#define DRAM_WND4_MEM_ADDR              CAL_MEM_BASE(DRAM_WND3_MEM_ADDR   , DRAM_WND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)    // 補足起始位置1024倍數
#define DRAM_WND5_MEM_ADDR              CAL_MEM_BASE(DRAM_WND4_MEM_ADDR   , DRAM_WND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)    // 補足起始位置1024倍數
#define DRAM_WND6_MEM_ADDR              CAL_MEM_BASE(DRAM_WND5_MEM_ADDR   , DRAM_WND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)    // 補足起始位置1024倍數
#define DRAM_SWND1_MEM_ADDR             CAL_MEM_BASE(DRAM_WND6_MEM_ADDR   , DRAM_WND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)    // 補足起始位置1024倍數
#define DRAM_SWND2_MEM_ADDR             CAL_MEM_BASE(DRAM_SWND1_MEM_ADDR  , DRAM_SWND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_SWND3_MEM_ADDR             CAL_MEM_BASE(DRAM_SWND2_MEM_ADDR  , DRAM_SWND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_SWND4_MEM_ADDR             CAL_MEM_BASE(DRAM_SWND3_MEM_ADDR  , DRAM_SWND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_SWND5_MEM_ADDR             CAL_MEM_BASE(DRAM_SWND4_MEM_ADDR  , DRAM_SWND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_SWND6_MEM_ADDR             CAL_MEM_BASE(DRAM_SWND5_MEM_ADDR  , DRAM_SWND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_MTN_MEM_ADDR               CAL_MEM_BASE(DRAM_SWND6_MEM_ADDR  , DRAM_SWND_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_SMTN_MEM_ADDR              CAL_MEM_BASE(DRAM_MTN_MEM_ADDR    , MTN_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)         // 補足起始位置1024倍數
#define DRAM_GAMMA_ADDR                 CAL_MEM_BASE(DRAM_SMTN_MEM_ADDR   , MTN_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)         // 補足起始位置1024倍數
#define DRAM_OSD_MEM1_ADDR              CAL_MEM_BASE(DRAM_GAMMA_ADDR      , GAMMA_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)     // 補足起始位置1024倍數
#define DRAM_OSD_MEM2_ADDR              CAL_MEM_BASE(DRAM_OSD_MEM1_ADDR   , OSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_OSD_MEM3_ADDR              CAL_MEM_BASE(DRAM_OSD_MEM2_ADDR   , OSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_OSD_TEXT_ADDR              CAL_MEM_BASE(DRAM_OSD_MEM3_ADDR   , OSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_OSD_BMP_ADDR               CAL_MEM_BASE(DRAM_OSD_TEXT_ADDR   , OSD_FNT_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_HDRPQ1_ADDR                CAL_MEM_BASE(DRAM_OSD_BMP_ADDR    , OSD_BMP_SIZE / PROAV_DRAM_LINE_BYTES, 1024)   // 補足起始位置1024倍數
#define DRAM_HDRPQ2_ADDR                CAL_MEM_BASE(DRAM_HDRPQ1_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_HDRPQ3_ADDR                CAL_MEM_BASE(DRAM_HDRPQ2_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_HDRPQ4_ADDR                CAL_MEM_BASE(DRAM_HDRPQ3_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_HDRHL1_ADDR                CAL_MEM_BASE(DRAM_HDRPQ4_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_HDRHL2_ADDR                CAL_MEM_BASE(DRAM_HDRHL1_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_HDRHL3_ADDR                CAL_MEM_BASE(DRAM_HDRHL2_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_HDRHL4_ADDR                CAL_MEM_BASE(DRAM_HDRHL3_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_CAPTURE_ADDR          CAL_MEM_BASE(DRAM_HDRHL4_ADDR     , DRAM_HDR_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_DISPLAY0_2D_ADDR      CAL_MEM_BASE(DRAM_LOGO_CAPTURE_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_DISPLAY1_2D_ADDR      CAL_MEM_BASE(DRAM_LOGO_DISPLAY0_2D_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_DISPLAY2_2D_ADDR      CAL_MEM_BASE(DRAM_LOGO_DISPLAY1_2D_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_DISPLAY0_3D_ADDR      CAL_MEM_BASE(DRAM_LOGO_DISPLAY2_2D_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_DISPLAY1_3D_ADDR      CAL_MEM_BASE(DRAM_LOGO_DISPLAY0_3D_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_LOGO_DISPLAY2_3D_ADDR      CAL_MEM_BASE(DRAM_LOGO_DISPLAY1_3D_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)  // 補足起始位置1024倍數
#define DRAM_WRPOSD_MEM1_ADDR           CAL_MEM_BASE(DRAM_LOGO_DISPLAY2_3D_ADDR , DRAM_LOGO_CAPTURE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_WRPOSD_MEM2_ADDR           CAL_MEM_BASE(DRAM_WRPOSD_MEM1_ADDR  , WPOSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_WRPOSD_MEM3_ADDR           CAL_MEM_BASE(DRAM_WRPOSD_MEM2_ADDR  , WPOSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_WRPOSD_MEM4_ADDR           CAL_MEM_BASE(DRAM_WRPOSD_MEM3_ADDR  , WPOSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_WRPOSD_BMP0_ADDR           CAL_MEM_BASE(DRAM_WRPOSD_MEM4_ADDR  , WPOSD_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024)

#define DRAM_BANK0_LAST_ADDR    DRAM_WRPOSD_BMP0_ADDR
#define DRAM_BANK0_USED_SIZE    (CAL_MEM_BASE(DRAM_BANK0_LAST_ADDR, DRAM_BANK0_LAST_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024) * PROAV_DRAM_LINE_BYTES)

#if (DRAM_BANK0_USED_SIZE > DRAM_BANK0_SIZE)
#error "Dram Bank 0 size is overflow"
#endif

//-------------------------------------------------------------------------------------------------
// Dram Bank#1 Memory Map (Warping Buffer/Warpper/Blender)
//-------------------------------------------------------------------------------------------------
#define DRAM_WIP_ADDR           0UL
#define DRAM_WOP1_ADDR          CAL_MEM_BASE(DRAM_WIP_ADDR          , DRAM_WIP_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_WOP2_ADDR          CAL_MEM_BASE(DRAM_WOP1_ADDR         , DRAM_WOP_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_WRP_TABLE_ADDR     CAL_MEM_BASE(DRAM_WOP2_ADDR         , DRAM_WOP_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_GAIN_ADDR0  CAL_MEM_BASE(DRAM_WRP_TABLE_ADDR    , DRAM_WRP_TABLE_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_BIAS_ADDR0  CAL_MEM_BASE(DRAM_DBDBLD_GAIN_ADDR0 , DRAM_DBDBLD_GAIN_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_GAIN_ADDR1  CAL_MEM_BASE(DRAM_DBDBLD_BIAS_ADDR0 , DRAM_DBDBLD_BIAS_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_BIAS_ADDR1  CAL_MEM_BASE(DRAM_DBDBLD_GAIN_ADDR1 , DRAM_DBDBLD_GAIN_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_GAIN_ADDR2  CAL_MEM_BASE(DRAM_DBDBLD_BIAS_ADDR1 , DRAM_DBDBLD_BIAS_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_BIAS_ADDR2  CAL_MEM_BASE(DRAM_DBDBLD_GAIN_ADDR2 , DRAM_DBDBLD_GAIN_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_GAIN_ADDR3  CAL_MEM_BASE(DRAM_DBDBLD_BIAS_ADDR2 , DRAM_DBDBLD_BIAS_SIZE / PROAV_DRAM_LINE_BYTES, 1024)
#define DRAM_DBDBLD_BIAS_ADDR3  CAL_MEM_BASE(DRAM_DBDBLD_GAIN_ADDR3 , DRAM_DBDBLD_GAIN_SIZE / PROAV_DRAM_LINE_BYTES, 1024)


#define DRAM_BANK1_LAST_ADDR    DRAM_DBDBLD_BIAS_ADDR3
#define DRAM_BANK1_USED_SIZE    (CAL_MEM_BASE(DRAM_BANK1_LAST_ADDR, DRAM_BANK1_LAST_MEM_SIZE / PROAV_DRAM_LINE_BYTES, 1024) * PROAV_DRAM_LINE_BYTES)

#if (DRAM_BANK1_USED_SIZE > DRAM_BANK1_SIZE)
#error "Dram Bank 2 size is overflow"
#endif

//-------------------------------------------------------------------------------------------------
// EXTERNAL PLL LMK03328
//-------------------------------------------------------------------------------------------------
#define VCXO_REF_CLK 270000  // 27MHz
#define HDMI_REF_CLK_6000 1485000  // 148.5MHz
#define HDMI_REF_CLK_5994 1484500

#ifdef Tx_Vx1_148M
#define VX1_REF_CLK 1485000
#define VX1_REF_CLK_6000 1485000    // 150MHz //2200*1125*60.00	148500000
#define VX1_REF_CLK_5994 1483515    // 150MHz //2200*1125*29.97	148351500
#else
#define VX1_REF_CLK 1500000
#define VX1_REF_CLK_6000 1499616 //1499994    // 150MHz //4064×2460×60.00	599846400, 599846400/4 = 149961600
#define VX1_REF_CLK_5994 1500000 //1498116 //1499976    // 150MHz //4064×2462×29.97	299866872, 299723251/2 = 149983464
#endif /* Tx_Vx1_148M */



/* 1499135 = (149907542 + 149919530)/2
//4560*2194*60.00    600278400, 600278400/4 = 150069600
//4560*2194*59.94    599678122, 599678122/4 = 149919530
//3080*1624*120.00 = 600230400, 600230400/4 = 150057600
//3080*1624*119.88 = 599630170, 599630170/4 = 149907542
*/
#define PLL_LMK03328_INPUT_CLK_MIN 750000   // 75MHz
#define PLL_LMK03328_INPUT_CLK_MAX 1250000   // 125MHz
#define PLL_LMK03328_RDIVIDER 0 // devide 1
#define PLL_LMK03328_PLL_OUT_DIVIDER 1  // devide 2
#define PLL_LMK03328_OUTPUT_DIVIDER 17

typedef enum
{
    eLMK03328SOURCE_PRIR,    // primary refence source
    eLMK03328SOURCE_SECR,    // secondary refence source

    eLMK03328SOURCE_MAX,
} eLMK03328SOURCE;

typedef enum
{
    eLMK03328PLL_CH1,
    eLMK03328PLL_CH2,

    eLMK03328PLL_CHMAX,
} eLMK03328PLL_CH;

typedef enum
{
    eLMK03328OUTPUT_CH0,
    eLMK03328OUTPUT_CH1,
    eLMK03328OUTPUT_CH2,
    eLMK03328OUTPUT_CH3,
    eLMK03328OUTPUT_CH4,
    eLMK03328OUTPUT_CH5,
    eLMK03328OUTPUT_CH6,
    eLMK03328OUTPUT_CH7,

    eLMK03328OUTPUT_CHMAX,
} eLMK03328OUTPUT_CH;

typedef enum
{
    eLMK03328OUTPUT_REF_SELECT_PLL1,
    eLMK03328OUTPUT_REF_SELECT_PLL2,
    eLMK03328OUTPUT_REF_SELECT_PRIR,
    eLMK03328OUTPUT_REF_SELECT_SECR,

    eLMK03328OUTPUT_REF_SELECT_CHMAX,
} eLMK03328OUTPUT_REF_SELECT;

typedef enum
{
    eLMK03328PLL_REF_SELECT_AUTO,
    eLMK03328PLL_REF_SELECT_EXPIN,
    eLMK03328PLL_REF_SELECT_PRIR,    // primary refence source
    eLMK03328PLL_REF_SELECT_SECR,    // secondary refence source

    eLMK03328PLL_REF_SELECT_MAX,
} eLMK03328PLL_REF_SELECT;

//-------------------------------------------------------------------------------------------------
// Base
//-------------------------------------------------------------------------------------------------
typedef enum{
    vrc148p5Mhz = 0,
    vrc270Mhz,
    vrcMax
} VopRefClock;

typedef enum{
                           // "0000" = not indicated
                           // "0001" ~ "0011" = Reserved
    eColorDepth_bpp24 = 4, // "0100" = 8 bpc
    eColorDepth_bpp30 = 5, // "0101" = 10 bpc
    eColorDepth_bpp36 = 6, // "0110" = 12 bpc
    eColorDepth_bpp48 = 7  // "0111" = 16 bpc
                           // "1111" = Reserved
} ColorDepth;

typedef enum{
    eSclEntity_Main = 0,
    eSclEntity_Sub,
    eSclEntity_Max
} SclEntity;

typedef enum{
    eSclSrcIn_Main = 0,
    eSclSrcIn_Sub,
    eSclSrcIn_MainSub,
    eSclSrcIn_Max,
} SclSrcIn;

typedef enum{
    eRxPort_0 = 0,
    eRxPort_1,
    eRxPort_2,
    eRxPort_3,
    eRxPort_Max
} RxPort;

typedef struct
{
    uint16 uiX;
    uint16 uiY;
    uint16 uiWidth;
    uint16 uiHeight;
} UI16Rect;

typedef struct
{
    uint16 Total;  //Total
    uint16 Start;  //Start
    uint16 Sync;   //Sync Width
    uint16 Size;   //Active size
} SyncTmg;

typedef struct
{
    SyncTmg Hs;  //H detected
    SyncTmg Vs;  //V detected
    bool Interlace; // Interlace
    bool Polarity; // Polarity
} DetTmg;

typedef struct
{
    uint08 VSI[8];
    uint08 AVI[14];
    uint08 HDR[28];
} DetInfoFrame;

typedef struct
{
    double dRx; // R CIE coordinate x
    double dRy; // R CIE coordinate y
    double dGx; // G CIE coordinate x
    double dGy; // G CIE coordinate y
    double dBx; // B CIE coordinate x
    double dBy; // B CIE coordinate y
    double dWx; // W CIE coordinate x
    double dWy; // W CIE coordinate y
    double dW_Y;// W的亮度Y

} CIE_Coor;

typedef enum{
    eScalerSPI_PC,
    eScalerSPI_MCU,

    eScalerSPISelMax
} ScalerSPISel;

typedef enum{
    eHdrCsc2020to709,
    eHdrCsc2020toAdobe,
    eHdrCsc2020toP3,
    eHdrCscP3to709,
    eHdrCscAdobeto709,

    eHdrCscTypeMax
}HdrCscType;

typedef enum{
    eEotfType_Sdr,       // SDR
    eEotfType_TraditionalGammaHDR,
    eEotfType_St2084Hdr, // HDR
    eEotfType_Hlg,       // HLG

    eEotfTypeMax
}HdrEotfType;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 重置state machine
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_StateMachineReset(void);

//-------------------------------------------------------------------------------------------------
// Reference Clock
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定Reference Clock
 * @param [in] ref : Reference Clock
 *              -vrc148p5Mhz
 *              -vrc270Mhz
 */
void dvProAV_VopRefClockSet(VopRefClock ref);

/**
 * @brief 讀取Ref Clock
 * @return : 回傳當前Reference Clock
 *          -vrc148p5Mhz
 *          -vrc270Mhz
 */
VopRefClock dvProAV_VopRefClockGet(void);

//-------------------------------------------------------------------------------------------------
// SPI Control
//-------------------------------------------------------------------------------------------------
/**
 * @brief 設定ProAV Scaler SPI控制權
 * @param [in] signal : SPI source
 */
int dvProAV_ScalerSPISel(ScalerSPISel signal);

/**
 * @brief ProAV SPI MISO Delay Control Set
 * @param [in] eSpiCh : SPI channel
 *              - eScalerSPI_PC
 *              - eScalerSPI_MCU
 * @param [in] ucDelay : 0~7 * 3.3 delay time, only valid when SPI Boost enable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_SpiMisoDelaySet(ScalerSPISel eSpiCh, uint8 ucDelay);

/**
 * @brief Caculate Gamma Table
 * @param [in]  ucInputBits: input gamma bits
 * @param [in]  ucOutputBits: output gamma bits
 * @param [in]  ucScaleBits: one step scale bits
 * @param [in]  power: gamma power
 * @param [out] data: Gamma Table Pointer
 * @note
 * fn = ROUNDUP(POWER(index/InputBitsMax,power)*OutputBitsMax,0)
 */
void dvProAV_GammaTableCaculate(uint08 ucInputBits, uint08 ucOutputBits, uint08 ucScaleBits, double power, void *data);

/**
 * @brief Get the write error count for SPI interface
 * @return  write error count
 */
uint08 dvProAV_InterfaceWriteErrorCountGet(void);

//-------------------------------------------------------------------------------------------------
// OSD Common
//-------------------------------------------------------------------------------------------------
#define PROAV_PALETTE_COLOR 256

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct
{
    UINT16 iX;
    UINT16 iY;
} START_POINT;

typedef struct
{
    UINT16 iWidth;
    UINT16 iHeight;
} RECT_SIZE;

typedef struct
{
    UINT8 ucT_Color1;
    UINT8 ucT_Color2;
    UINT8 ucT_Color3;
    UINT8 ucT_Color4;
} OSD_TRANSPARENCY_COLOR;

typedef struct _OSD_BITMAP
{
    START_POINT sSrc_Position;  //use to record bitmap's start position in memory
    RECT_SIZE sBitmap_Size ;  //used to record bitmap size information
} OSD_BITMAP;

//Font set use one byte to indicate his character width
typedef struct _OSD_STRING
{
    UINT8 ucStringMaxHeight;
    UINT16 uiStringLength;
    UINT32 ulFontOffset;
    UINT16 *puiCharIndex;
    UINT8 *pucCharWdith;
} OSD_STRING;

typedef struct
{
    INT16 idX;
    INT16 idY;
} CENTER_PITCH;

#pragma pack(pop)                          // restore previous alignment


typedef enum
{
    eWPOSD_TransparentTrigger,
    eWPOSD_SigleCircleTrigger,
    eWPOSD_MultiCircleTrigger,
    eWPOSD_RectangleTrigger,
    eWPOSD_BitBLTTrigger,
    eWPOSD_DisplayOSDData
} eWPOSD_TRIG;

typedef enum
{
    eWPOSDPAGE_0,   // in eLayerMode_P0BeforeP1AfterWarp mode = before warp
    eWPOSDPAGE_1,   // in eLayerMode_P0BeforeP1AfterWarp mode = after warp

    eWPOSDPAGE_Max
} eWPOSDPAGE;

//-------------------------------------------------------------------------------------------------
// Line Buffer
//-------------------------------------------------------------------------------------------------
typedef enum{
    eDramBank0,
    eDramBank1,

    eDramBankMax
} DramBank;

typedef enum{
    eDramDataType_Pixel,
    eDramDataType_Data,

    eDramDataTypeMax
} DramDataType;

/**
 * @brief 設定Dram Bank
 * @param [in] bank Dram Bank
 * @return
 */
void dvProAV_DramBankSet(DramBank eBank);

/**
 * @brief 讀取目前Dram Bank
 * @return Dram Bank
 */
DramBank dvProAV_DramBankGet(void);

/**
 * @brief 寫入資料到Warp Line Buffer
 * @param [in] bBank : DRAM Bank
 *             - 0 : Scaler DRAM Bank
 *             - 1 : Warp DRAM Bank
 * @param [in] u32Addr : 起始位置
 * @param [in] u8Data : 資料指標
 * @param [in] u32DataLen : 資料長度(Byte)
 * @param [in] bWrite :
 *              - false : read
 *              - true : write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_LineBufAccess(DramBank eBank, uint32 u32Addr, uint08 *u8Data, uint32 u32DataLen, bool bWrite);

/**
 * @brief 寫入指定長度的資料到Warp Line Buffer
 * @param [in] bBank : DRAM Bank
 *             - 0 : Scaler DRAM Bank
 *             - 1 : Warp DRAM Bank
 * @param [in] u32Addr : 起始位置
 * @param [in] u16Len : 資料長度 = len(1~256)*64byte(DRAM 資料寬度為512 Bits = 64Byte)
 * @param [in] u8Data : 資料指標
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_LineBufWrite(DramBank eBank, uint32 u32Addr, uint16 u16Len, uint08 *u8Data);

/**
 * @brief 讀取指定長度的資料到Warp Line Buffer
 * @param [in] bBank : DRAM Bank
 *             - 0 : Scaler DRAM Bank
 *             - 1 : Warp DRAM Bank
 * @param [in] u32Addr : 起始位置 = address
 * @param [in] u16Len : 資料長度 = len(1~256)*64byte(DRAM 資料寬度為512 Bits = 64Byte)
 * @param [in] u8Data : 資料
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_LineBufRead(DramBank eBank, uint32 u32Addr, uint16 u16Len, uint08 *u8Data);

int dvProAV_DramWriteEanle(DramBank eBank, uint32 addr, uint8 WrEn);
int dvProAV_DramReadEanle(DramBank eBank, uint32 addr, uint8 ReadEn);

//-------------------------------------------------------------------------------------------------
// DRAM Clear
//-------------------------------------------------------------------------------------------------
/**
 * @brief dvProAV_ClearDram
 * @param [in] bBank : DRAM Bank
                - 0 : Scaler DRAM Bank
                - 1 : Warp DRAM Bank
 * @param [in] u32Addr : Dram start address
 * @param [in] u32LenByte : byets
 * @param [in] u8Data : Dram data value
 * @return
 */
int dvProAV_ClearDram(DramBank eBank, uint32 u32Addr, uint32 u32LenByte, uint8 u8Data);

//-----------------------------------------------------------------------------
// Universal Memory Buffer-Scaler
//-----------------------------------------------------------------------------
typedef enum{
    eUmbFunc_DramToDram = 0x00,
    eUmbFunc_SpiToDram = 0x01,
    eUmbFunc_FlashToDram = 0x02,
    eUmbFunc_DramToSpi = 0x10,
    eUmbFunc_SpiToSpi = 0x11,
    eUmbFunc_FlashToSpi = 0x12,
    eUmbFunc_DramToFlash = 0x20,
    eUmbFunc_SpiToFlash = 0x21,
    eUmbFunc_FlashToFlash = 0x22,
    eUmbFunc_DramToGamma = 0x30,
    eUmbFunc_SpiToGamma = 0x31,
    eUmbFunc_FlashToGamma = 0x32,
    eUmbFunc_Max
} eUmbFunc;

typedef enum{
    eUmbActionMode_Continuous = 0x01,
    eUmbActionMode_Stage = 0x02,
} eUmbActionMode;

typedef enum{
    eUmbGammaType_VopGamma,
    eUmbGammaType_Hdr1,
    eUmbGammaType_Hdr30,
    eUmbGammaType_Hdr31,
    eUmbGammaType_Hdr4,
    eUmbGammaType_Max
} eUmbGammaType;

/**
 * @brief Universal Memory Buffer reset
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbRst(DramBank eBank);

/**
 * @brief Universal Memory Buffer Action mode
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] eMode : Action mode
 *              -eUmbActionMode_Continuous
 *              -eUmbActionMode_Stage
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbAction(DramBank eBank, eUmbActionMode eMode);

/**
 * @brief Universal Memory Buffer function select
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] eFunc
 *              -eUmbFunc_DramToDram = 0x00,
 *              -eUmbFunc_CpuToDram = 0x01,
 *              -eUmbFunc_FlashToDram = 0x02,
 *              -eUmbFunc_DramToCpu = 0x10,
 *              -eUmbFunc_CpuToCpu = 0x11,
 *              -eUmbFunc_FlashToCpu = 0x12,
 *              -eUmbFunc_DramToFlash = 0x20,
 *              -eUmbFunc_CpuToFlash = 0x21,
 *              -eUmbFunc_FlashToFlash = 0x22,
 *              -eUmbFunc_DramToGamma = 0x30,
 *              -eUmbFunc_CpuToGamma = 0x31,
 *              -eUmbFunc_FlashToGamma = 0x32,
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFuncSel(DramBank eBank, eUmbFunc eFunc);

/**
 * @brief Universal Memory Buffer Length Set
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] uiOutLen : Buffer output length, buffer max size 512bit x 512
 * @param [in] uiInLen : Buffer input length, buffer max size 512bit x 512
 *              DRAM -> 1 pkc*n, 1 pkc represent 64 bytes, length max is 512
 *              CPU, FLASH, gamma -> 13 pkc*n, 13 pkc represent 12x5 bytes + 4 bytes, length max is 6656
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbLenSet(DramBank eBank, uint16 uiOutLen, uint16 uiInLen);

/**
 * @brief Get Universal Memory Buffer state
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @return ulStatus : status
 *          -bit 0 : buffer core busy
 *          -bit 1 : writing (FSM)
 *          -bit 2 : waiting (FSM)
 *          -bit 3 : reading (FSM)
 */
uint08 dvProAV_UmbStatusGet(DramBank eBank);

/**
 * @brief dvProAV_UmbDataFmtSet
 * @param [in] bBigEnd :
 *              - 0 : little-endian
 *              - 1 : big-endian
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDataFmtSet(bool bBigEnd);

/**
 * @brief Universal Memory Buffer Data Write
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulWLen : write data byte
 * @param [in] ucData : write data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDataWrite(DramBank eBank, uint16 ulWLen, uint08 *ucData);

/**
 * @brief Universal Memory Buffer Data Read
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulWLen : read data byte
 * @param [out] ucData : read data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDataRead(DramBank eBank, uint16 ulRLen, uint08 *ucData);

/**
 * @brief DRAM Address Set
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulDramAddr : DRAM Address, 64byte data pack
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDramAddrSet(DramBank eBank, uint32 ulDramAddr);

/**
 * @brief dvProAV_UmbDramToDram
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] u16len : Data Byte
 * @param [in] ulCpyAddr : Copy DRAM Address
 * @param [in] ulPasAddr : Paste DRAM Address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbDramToDram(DramBank eBank, uint16 u16len, uint32 ulCpyAddr, uint32 ulPasAddr);
//-----------------------------------------------------------------------------
// Universal Memory Buffer - Flash
//-----------------------------------------------------------------------------
/**
 * @brief serial flash Ip reset
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashIpRst(void);

/**
 * @brief Serial Flash Select
 * @param [in] bFlashSel :
 *              -0 : common FLASH
 *              -1 : boot FLASH
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashSel(bool bFlashSel);

/**
 * @brief Flash Csr Command Set
 * @param [in] ucCmd : CSR command
 *              -0X : finish
 *              -10 : read
 *              -11 : write
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrCmdSet(uint08 ucCmd);

/**
 * @brief Flash Csr Address Set
 * @param [in] ucAddr : CSR address
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrAddrSet(uint08 ucAddr);

/**
 * @brief Flash Csr Data Write
 * @param [in] ulWLen : write csr data byte
 * @param [in] ucData : write csr data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrWrite(uint16 ulWLen, uint08 *ucData);

/**
 * @brief Flash Csr Data Read
 * @param [in] ulRLen : read csr data byte
 * @param [out] ucData : read csr data array
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashCsrRead(uint16 ulRLen, uint08 *ucData);

/**
 * @brief Flash Address Set
 * @param [in] bBank : DRAM Bank
 *              - 0 : Scaler DRAM Bank
 *              - 1 : Warp DRAM Bank
 * @param [in] ulFlashAddr : Flash Address, 4byte data pack
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbFlashAddrSet(DramBank eBank, uint32 ulFlashAddr);

//-----------------------------------------------------------------------------
// Universal Memory Buffer - Gamma
//-----------------------------------------------------------------------------
int dvProAV_UmbSpiToSpi(uint16 u16Len, uint08 *ucWData, uint08 *ucRData);
/**
 * @brief Universal Memory Buffer Hdr Type Set
 * @param ucHdrType : HDR type
 *          -0: HDR gamma 1
 *          -1: HDR gamma 30
 *          -2: HDR gamma 31
 *          -3: HDR gamma 4
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbHdrTypeSet(uint08 ucHdrType);

/**
 * @brief Universal Memory Buffer Gamma Rgb Same Table Set
 * @param bTableType : (HDR mode not applicable)
 *          -false : RGB use the same
 *          -true : different table
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbRgbTableTypeSet(bool bTableType);

/**
 * @brief Universal Memory Buffer Gamma Type Set
 * @param bGammaType : Package of gamma table type
 *          -false : General image gamma table (take 12 bits for every 2 bytes)
 *          -true : HDR gamma table (take 26 bits for every 4 bytes)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbGammaTypeSet(bool bGammaType);

/**
 * @brief Gamma Table Write
 * @param [in] eFunc : only accept the following three functions
 *              - eUmbFunc_DramToGamma
 *              - eUmbFunc_SpiToGamma
 *              - eUmbFunc_FlashToGamma
 * @param [in] ulAddr : different functions of "eFunc" have different meanings
 *              - eUmbFunc_DramToGamma : ulAddr = Dram Address
 *              - eUmbFunc_SpiToGamma : ulAddr = Invalid
 *              - eUmbFunc_FlashToGamma : ulAddr = Flash Address
 * @param [in] eGammaType
 *              - eUmbGammaType_VopGamma
 *              - eUmbGammaType_Hdr1
 *              - eUmbGammaType_Hdr30
 *              - eUmbGammaType_Hdr31
 *              - eUmbGammaType_Hdr4
 * @param [in] len : gamma table bytes
 * @param [in] ucData : data point
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UmbGammaWrite(eUmbFunc eFunc, uint32 ulAddr, eUmbGammaType eGammaType, uint16 len, uint08 *ucData);


#ifdef __cplusplus
}
#endif


#endif // DV_PROAV_BASE_H
