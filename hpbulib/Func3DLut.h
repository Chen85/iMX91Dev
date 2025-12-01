
#ifndef _FUNC3DLUT_H_
#define _FUNC3DLUT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

//Error Code
#define ERRCODE_NO_ERROR                            (0)
#define ERRCODE_3DLUT_OPEN_CUBE_FILE_FAIL           (1)
#define ERRCODE_3DLUT_LUT_SIZE_INVALID              (2)   //not 33 or 17
#define ERRCODE_3DLUT_CONVERT_TO_C341_FORMAT_FAIL   (3)
#define ERRCODE_3DLUT_OPEN_OUTPUT_BINARY_FILE_FAIL  (4)
#define ERRCODE_3DLUT_CONVERT_TO_FPGA_FORMAT_FAIL   (5)

//For encoding binary data
#define LUT_DATA_OFFSET                             (0x3D)  //for encoding binary data

//系統存檔格式(binary file) : Header(1024bytes) + payload
//header 內容預設值
#define LUT_HEADER_MAGIC                           "C3DL"
#define LUT_HEADER_BINARY_FILE_FMT_VER_MAJOR       1            // 檔案格式版本（非內容版本）
#define LUT_HEADER_BINARY_FILE_FMT_VER_MINOR       0
#define LUT_HEADER_PAYLOAD_OFFSET                  1024         // payload 開始的位置
#define LUT_HEADER_PAYLOAD_SIZE                    (33*33*33)   // payload size
#define LUT_HEADER_CRC32                           0            // 整個 binary file   (含header) 的 CRC
#define LUT_HEADER_CHIP_FMT_ID                     0            // 相容的 chip data : FPGA or C341
#define LUT_HEADER_CUBE_TITLE                      "3DLUT"      // 原始 cube TITLE
#define LUT_HEADER_CUBE_DOMAIN_MIN                 0.0          // 原始 cube DOMAIN_MIN
#define LUT_HEADER_CUBE_DOMAIN_MAX                 1.0          // 原始 cube DOMAIN_MAX
#define LUT_HEADER_CUBE_LUTSIZE                    33           // 原始 cube LUT_3D_SIZE
#define LUT_HEADER_CUBE_FILENAME                   "3DLUT.cube" // 原始 cube 檔名
#define LUT_HEADER_CUBE_VERSION                    "V01"        // 原始 cube 版本
#define LUT_HEADER_CUBE_UPLOAD_TIME                "2025/01/01 00:00:00" // 原始 cube 上傳日期

#pragma pack(push, 1)
typedef union
{
    char Header[1024];

    struct
    {
        UINT8   MagicNumber[4];
        UINT8   MainVersion;            // 檔案格式版本（非內容版本）
        UINT8   SubVersion;
        UINT32  PayloadOffset;          // payload 開始的位置
        UINT32  PayloadSize;            // payload size
        UINT32  CRC32;                  // for CRC32 check
        UINT32  ChipFmtID;              // 相容 chip data 格式 ID (FPGA or C341)

        char    CubeTitleName[128];     // 原始 cube 檔紀錄的資訊, 來自 .cube 之 TITLE 欄位
        FLOAT   CubeDomainMin[3];       // 原始 cube 檔紀錄的資訊, 來自 .cube 之 DOMAIN_MIN R G B欄位
        FLOAT   CubeDomainMax[3];       // 原始 cube 檔紀錄的資訊, 來自 .cube 之 DOMAIN_MAX R G B欄位
        UINT32  CubeLutSize;            // 原始 cube 檔紀錄的資訊, 例如 33 代表 33x33x33, 來自 .cube 之 LUT_3D_SIZE 欄位
        char    CubeFileName[64];       // 原始 cube 檔名
        char    CubeFileVersion[16];    // 原始 cube 版本
        char    CubeFileUploadTime[32]; // 原始 cube 上傳日期 (yyyy/mm/dd hh:mm:ss)

    }sHeader;

} sRGBLUTInfo, *psRGBLUTInfo;   //for output binary file header
#pragma pack(pop)


/*
    CubeFilePath => 輸入 .cube 路徑
    scalerType   => "ICHIP2CH" 走 C341；其他走 FPGA
    outputFile   => 轉換為 Scaler Data format 的 binary file (包含 cube info header) 的輸出檔路徑
*/
int Upload3DLutCube(const char* CubeFilePath, const char* scalerType, const char* outputFile);



#ifdef __cplusplus
}
#endif

#endif //_FUNC3DLUT_H_

