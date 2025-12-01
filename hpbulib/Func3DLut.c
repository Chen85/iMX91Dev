/*
 * Func3DLut.c
 * 說明：
 *   - 讀取 .cube 3D LUT 檔
 *   - 轉為 FPGA F34 需要的 0~1023 打包格式（uint32_t，B[29:20]G[19:10]R[9:0]）
 *   - 或轉為 C341 27-bit/通道（RGB 共 81-bit）拆成 11 bytes（低位在前）
 *   - 可將結果以二進位寫出，前置一個 sRGBLUTInfo 頭
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "Func3DLut.h"

/* 3D LUT Cube File Table */
typedef struct
{
    float r, g, b;
} sCUBERGBLUT, *psCUBERGBLUT;

/* 3D LUT Table for C341 format，11 bytes/點 */
typedef struct
{
    unsigned char c341Lut[11];
} sC341FORMAT_RGBLUT, *psC341FORMAT_RGBLUT;



static inline int clampi(int v, int lo, int hi)
{
    return (v < lo) ? lo : (v > hi ? hi : v);
}

static inline int lutIndex(int x, int y, int z, int size)
{
    return (z * size * size) + (y * size) + x;
}

/*====================== 讀 .cube ======================*/
/* 讀檔並填滿 lut 與 lutCount；同時解析 LUT_3D_SIZE / TITLE / DOMAIN_MIN / DOMAIN_MAX */
bool readCubeFile(const char* filename, sCUBERGBLUT** outLut, size_t* outCount, sRGBLUTInfo* lutInfo)
{
    FILE* fp = fopen(filename, "r");

    if (!fp)
        return false;

    memset(lutInfo, 0, sizeof(*lutInfo));
    //lutInfo->sHeader.Dimension = 1; /* 預設為 3D LUT */
    lutInfo->sHeader.CubeLutSize = 0;
    lutInfo->sHeader.CubeDomainMin[0] = lutInfo->sHeader.CubeDomainMin[1] = lutInfo->sHeader.CubeDomainMin[2] = 0.0f;
    lutInfo->sHeader.CubeDomainMax[0] = lutInfo->sHeader.CubeDomainMax[1] = lutInfo->sHeader.CubeDomainMax[2] = 1.0f;

    size_t cap = 1024;
    size_t cnt = 0;
    sCUBERGBLUT* lut = (sCUBERGBLUT*)malloc(cap * sizeof(sCUBERGBLUT));
    if (!lut)
    {
        fclose(fp);
        return false;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp))
    {
        /* 跳過註解與空白行 */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        if (strstr(line, "LUT_3D_SIZE") != NULL)
        {
            int size = 0;
            if (sscanf(line, "LUT_3D_SIZE %d", &size) == 1)
            {
                lutInfo->sHeader.CubeLutSize = size;
                //lutInfo->sHeader.Dimension = 1; /* 3D */
            }
            continue;
        }

        if (strstr(line, "TITLE") != NULL)
        {
            /* 解析 "TITLE "xxxxx"" */
            char* first = strchr(line, '\"');
            char* last  = (first ? strrchr(first + 1, '\"') : NULL);
            if (first && last && last > first + 1)
            {
                size_t len = (size_t)(last - first - 1);
                if (len >= sizeof(lutInfo->sHeader.CubeTitleName))
                    len = sizeof(lutInfo->sHeader.CubeTitleName) - 1;
                memcpy(lutInfo->sHeader.CubeTitleName, first + 1, len);
                lutInfo->sHeader.CubeTitleName[len] = '\0';
            }

            continue;
        }

        if (strstr(line, "DOMAIN_MIN") != NULL)
        {
            float r, g, b;
            if (sscanf(line, "DOMAIN_MIN %f %f %f", &r, &g, &b) == 3)
            {
                lutInfo->sHeader.CubeDomainMin[0] = r;
                lutInfo->sHeader.CubeDomainMin[1] = g;
                lutInfo->sHeader.CubeDomainMin[2] = b;
            }
            continue;
        }

        if (strstr(line, "DOMAIN_MAX") != NULL)
        {
            float r, g, b;
            if (sscanf(line, "DOMAIN_MAX %f %f %f", &r, &g, &b) == 3)
            {
                lutInfo->sHeader.CubeDomainMax[0] = r;
                lutInfo->sHeader.CubeDomainMax[1] = g;
                lutInfo->sHeader.CubeDomainMax[2] = b;
            }
            continue;
        }

        /* 嘗試讀取三個浮點 (R G B) */
        float r, g, b;
        if (sscanf(line, "%f %f %f", &r, &g, &b) == 3)
        {
            if (cnt >= cap) //慢慢擴充配置記憶體的byte數 (不夠時再擴充2倍), 1024->2048->4096->8192->...
            {
                cap *= 2;
                sCUBERGBLUT* tmp = (sCUBERGBLUT*)realloc(lut, cap * sizeof(sCUBERGBLUT));
                if (!tmp)
                {
                    free(lut);
                    fclose(fp);
                    return false;
                }

                lut = tmp;

            }

            lut[cnt].r = r;
            lut[cnt].g = g;
            lut[cnt].b = b;
            cnt++;
        }
    }

    fclose(fp);

    *outLut = lut;
    *outCount = cnt;
    return true;
}

/*====================== FPGA 路徑：0~1 轉 0~1023，並打包成 30bits: B[29:20] G[19:10] R[9:0] ======================*/
bool modifyToFpgaLUT(const sCUBERGBLUT* lut, size_t count, uint32_t** outPacked, size_t* outCount)
{
    uint32_t* buf = (uint32_t*)malloc(count * sizeof(uint32_t));

    if (!buf)
        return false;

    for (size_t i = 0; i < count; ++i)
    {
        #if 0
        int r10 = clampi((int)floorf(lut[i].r * 1023.0f + 0.5f), 0, 1023);
        int g10 = clampi((int)floorf(lut[i].g * 1023.0f + 0.5f), 0, 1023);
        int b10 = clampi((int)floorf(lut[i].b * 1023.0f + 0.5f), 0, 1023);
        #endif

        int r10 = clampi((int)floorf(lut[i].r * 1023.0f), 0, 1023);
        int g10 = clampi((int)floorf(lut[i].g * 1023.0f), 0, 1023);
        int b10 = clampi((int)floorf(lut[i].b * 1023.0f), 0, 1023);

        uint32_t packed = ( ((uint32_t)b10) << 20 ) | ( ((uint32_t)g10) << 10 ) | ((uint32_t)r10);
        buf[i] = packed;
    }

    *outPacked = buf;
    *outCount = count;

    return true;
}

bool writeFpgaFormatCubeFile(const char* filename, const uint32_t* packed, size_t count, const sRGBLUTInfo* lutInfo)
{
    FILE* fp = fopen(filename, "wb");

    if (!fp)
        return false;

    /* 先寫 header */
    if (fwrite(lutInfo, sizeof(sRGBLUTInfo), 1, fp) != 1)
    {
        fclose(fp);
        return false;
    }

    /* 再寫資料 */
    if (count > 0)
    {
        if (fwrite(packed, sizeof(uint32_t), count, fp) != count)
        {
            fclose(fp);
            return false;
        }
    }

    fclose(fp);
    return true;
}

/*====================== C341 路徑：33->17 近鄰抽樣；RGB 27bit 拆 11 bytes ======================*/
static void downsampleLUT(    const sCUBERGBLUT* inputLUT, int inputSize, sCUBERGBLUT* outputLUT, int outputSize)
{
    float scale = (float)(inputSize - 1) / (float)(outputSize - 1);

    for (int z = 0; z < outputSize; ++z) {
        for (int y = 0; y < outputSize; ++y) {
            for (int x = 0; x < outputSize; ++x) {

                int srcX = (int)lroundf(x * scale);
                int srcY = (int)lroundf(y * scale);
                int srcZ = (int)lroundf(z * scale);
                srcX = clampi(srcX, 0, inputSize - 1);
                srcY = clampi(srcY, 0, inputSize - 1);
                srcZ = clampi(srcZ, 0, inputSize - 1);

                outputLUT[lutIndex(x, y, z, outputSize)] = inputLUT[lutIndex(srcX, srcY, srcZ, inputSize)];

            }
        }
    }
}

/* 將 0~1 的 float 乘上 2^27-1，再照原始碼位元配置拆成 11 bytes */
bool modifyToC341LUT(    const sCUBERGBLUT* lut, size_t inCount, int CubeLutSize, sC341FORMAT_RGBLUT** outC341, size_t* outCount)
{
    const double scale = 134217727.0; /* 2^27 - 1 */
    sC341FORMAT_RGBLUT* out = NULL;
    size_t count = 0;

    if (CubeLutSize == 33)
    {
        const int outSize = 17;
        count = (size_t)outSize * outSize * outSize;
        out = (sC341FORMAT_RGBLUT*)malloc(count * sizeof(sC341FORMAT_RGBLUT));
        if (!out)
            return false;

        sCUBERGBLUT* tmp17 = (sCUBERGBLUT*)malloc(count * sizeof(sCUBERGBLUT));
        if (!tmp17)
        {
            free(out);
            return false;
        }

        downsampleLUT(lut, 33, tmp17, 17);

        for (size_t i = 0; i < count; ++i)
        {
            int r = (int)floor(tmp17[i].r * scale + 0.5);
            int g = (int)floor(tmp17[i].g * scale + 0.5);
            int b = (int)floor(tmp17[i].b * scale + 0.5);

            /* 依原始位元打包方式拆 bytes（LSB first） */
            out[i].c341Lut[0]  = (unsigned char)( r        & 0xFF);
            out[i].c341Lut[1]  = (unsigned char)((r >> 8)  & 0xFF);
            out[i].c341Lut[2]  = (unsigned char)((r >> 16) & 0xFF);
            out[i].c341Lut[3]  = (unsigned char)(((r >> 24) & 0x07) | ((g << 3) & 0xF8));
            out[i].c341Lut[4]  = (unsigned char)((g >> 5)  & 0xFF);
            out[i].c341Lut[5]  = (unsigned char)((g >> 13) & 0xFF);
            out[i].c341Lut[6]  = (unsigned char)(((g >> 21) & 0x3F) | ((b << 6) & 0xC0));
            out[i].c341Lut[7]  = (unsigned char)((b >> 2)  & 0xFF);
            out[i].c341Lut[8]  = (unsigned char)((b >> 10) & 0xFF);
            out[i].c341Lut[9]  = (unsigned char)((b >> 18) & 0xFF);
            out[i].c341Lut[10] = (unsigned char)((b >> 26) & 0x01);
        }

        free(tmp17);
    }
    else
    {
        /* 直接轉：假設 inCount 已是 CubeLutSize^3 */
        count = inCount;
        out = (sC341FORMAT_RGBLUT*)malloc(count * sizeof(sC341FORMAT_RGBLUT));
        if (!out)
            return false;

        for (size_t i = 0; i < count; ++i)
        {
            int r = (int)floor(lut[i].r * scale + 0.5);
            int g = (int)floor(lut[i].g * scale + 0.5);
            int b = (int)floor(lut[i].b * scale + 0.5);

            out[i].c341Lut[0]  = (unsigned char)( r        & 0xFF);
            out[i].c341Lut[1]  = (unsigned char)((r >> 8)  & 0xFF);
            out[i].c341Lut[2]  = (unsigned char)((r >> 16) & 0xFF);
            out[i].c341Lut[3]  = (unsigned char)(((r >> 24) & 0x07) | ((g << 3) & 0xF8));
            out[i].c341Lut[4]  = (unsigned char)((g >> 5)  & 0xFF);
            out[i].c341Lut[5]  = (unsigned char)((g >> 13) & 0xFF);
            out[i].c341Lut[6]  = (unsigned char)(((g >> 21) & 0x3F) | ((b << 6) & 0xC0));
            out[i].c341Lut[7]  = (unsigned char)((b >> 2)  & 0xFF);
            out[i].c341Lut[8]  = (unsigned char)((b >> 10) & 0xFF);
            out[i].c341Lut[9]  = (unsigned char)((b >> 18) & 0xFF);
            out[i].c341Lut[10] = (unsigned char)((b >> 26) & 0x01);
        }
    }

    *outC341 = out;
    *outCount = count;
    return true;
}

bool writeC341FormatCubeFile(const char* filename, const sC341FORMAT_RGBLUT* c341, size_t count, const sRGBLUTInfo* lutInfo)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp)
        return false;

    if (fwrite(lutInfo, sizeof(sRGBLUTInfo), 1, fp) != 1)
    {
        fclose(fp);
        return false;
    }

    if (count > 0)
    {
        if (fwrite(c341, sizeof(sC341FORMAT_RGBLUT), count, fp) != count)
        {
            fclose(fp);
            return false;
        }
    }

    fclose(fp);
    return true;
}


/*
    CubeFilePath => 輸入 .cube 路徑
    scalerType   => "ICHIP2CH" 走 C341；其他走 FPGA
    outputFile   => 轉換為 Scaler Data format 的 binary file (包含 cube info header) 的輸出檔路徑
*/
int Upload3DLutCube(const char* CubeFilePath, const char* scalerType, const char* outputFile)
{
    sCUBERGBLUT* CubeLut = NULL;
    size_t   lutCount = 0;
    sRGBLUTInfo info;  //out binary file header

    if (!readCubeFile(CubeFilePath, &CubeLut, &lutCount, &info))
    {
        return ERRCODE_3DLUT_OPEN_CUBE_FILE_FAIL;
    }

    if (!(info.sHeader.CubeLutSize == 33 || info.sHeader.CubeLutSize == 17))
    {
        free(CubeLut);
        return ERRCODE_3DLUT_LUT_SIZE_INVALID;
    }

    /* 預設 domain 如未指定 */
    if (info.sHeader.CubeDomainMax[0] == 0 &&
        info.sHeader.CubeDomainMax[1] == 0 &&
        info.sHeader.CubeDomainMax[2] == 0)
    {
        info.sHeader.CubeDomainMax[0] = info.sHeader.CubeDomainMax[1] = info.sHeader.CubeDomainMax[2] = 1.0f;
    }

    if (scalerType && strcmp(scalerType, "ICHIP2CH") == 0)
    {
        /* C341 平台 */
        sC341FORMAT_RGBLUT* C341packed = NULL;
        size_t outCount = 0;
        if (!modifyToC341LUT(CubeLut, lutCount, info.sHeader.CubeLutSize, &C341packed, &outCount))
        {
            free(CubeLut);
            return ERRCODE_3DLUT_CONVERT_TO_C341_FORMAT_FAIL;
        }

        bool ok = writeC341FormatCubeFile(outputFile, C341packed, outCount, &info);

        free(C341packed);
        free(CubeLut);
        return ok ? ERRCODE_NO_ERROR : ERRCODE_3DLUT_OPEN_OUTPUT_BINARY_FILE_FAIL;
    }
    else
    {
        /* FPGA 平台 */
        uint32_t* FPGApacked = NULL;
        size_t outCount = 0;

        if (!modifyToFpgaLUT(CubeLut, lutCount, &FPGApacked, &outCount))
        {
            free(CubeLut);
            return ERRCODE_3DLUT_CONVERT_TO_FPGA_FORMAT_FAIL;
        }

        bool ok = writeFpgaFormatCubeFile(outputFile, FPGApacked, outCount, &info);

        free(FPGApacked);
        free(CubeLut);
        return ok ? ERRCODE_NO_ERROR : ERRCODE_3DLUT_OPEN_OUTPUT_BINARY_FILE_FAIL;
    }
}


