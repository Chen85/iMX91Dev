#ifndef HDMISTANDARD_H
#define HDMISTANDARD_H

#include "dvProAV_Platform.h"

#pragma pack(push)  /* push current alignment to stack */       //A70LV_Doulas_0123
#pragma pack(1)     /* set alignment to 1 byte boundary */      //A70LV_Doulas_0123

typedef struct{ // low bit first
    uint08 S:2;
    // S1 S0 Scan Information
    // 0  0  No Data
    // 0  1  Composed for a overscanned display, where some Actvie Pixels and lines at the edges are ant display
    // 1  0  Composed for a underscanned display, where some Actvie Pixels and lines at the edges are ant display
    // 1  1  Reserved
    uint08 B:2;
    // B1 B0 Bar Data Present
    // 0  0  Bar Data not Present
    // 0  1  Vertical Bar Info present
    // 1  0  Horizontal Bar Info present
    // 1  1  Vertical and Horizontal Bar Info present
    uint08 A:1;
    // A0 Active Format Information Present
    // 0  No Active Format Information
    // 1  Active Format(R3..R0)
    uint08 Y:3;
    // Y2 Y1 Y0 RGB or YCbCr
    // 0  0  0  RGB(default)
    // 0  0  1  YCbCr422
    // 0  1  0  YCbCr444
    // 0  1  1  YCbCr420
    // 1  0  0  Reserved
    // 1  0  1  Reserved
    // 1  1  0  Reserved
    // 1  1  1  IDO-Defined
}AVI_B1;

typedef struct{ // low bit first
    uint08 R:4;
    // R3 R2 R1 R0 Active Portion Ascpect Ratio
    // 1  0  0  0  Same as Picture Aspect Ratio
    // 1  0  0  1  4:3(Center)
    // 1  0  1  0  16:9(Center)
    // 1  0  1  0  14:9(Center)
    // others      Varies. See Annex H.
    uint08 M:2;
    // M1 M0 Picture Aspect Ratio
    // 0  0  No Data
    // 0  1  4:3
    // 1  0  16:9
    // 1  1  Reserved
    uint08 C:2;
    // C1 C0 Colorimetry
    // 0  0  No Data
    // 0  1  SMTPE 170M
    // 1  0  ITU-R BT.709
    // 1  1  Extended Colorimetry Information Vaild(colorimetry indicated in bitd EC0, EC1, EC2)
}AVI_B2;

typedef struct{ // low bit first
    uint08 SC:2;
    // SC1 SC0 Non-Uniform Picture Scaling
    // 0   0   No Known non-uniform scaling
    // 0   1   Pictuer has been scaled horizontally
    // 1   0   Pictuer has been scaled vertically
    // 1   1   Pictuer has been scaled horizontally and vertically
    uint08 Q:2;
    // Q1 Q0 RGB Quantization Range
    // 0  0  Default(depends on video format)
    // 0  1  Limited Range
    // 1  0  Full Range
    // 1  1  Reserved
    uint08 EC:3;
    // EC2 EC1 EC0 Extended Colorimetry
    // 0   0   0   xvYCC601
    // 0   0   1   xvYCC709
    // 0   1   0   sYCC601
    // 0   1   1   AdobeYCC601
    // 1   0   0   AdobeRGB
    // 1   0   1   ITU-R BT.2020 Y'cC'bcC'rc
    // 1   1   0   ITU-R BT.2020 R'G'B' or Y'C'bC'r
    // 1   1   1   Reserved
    uint08 ITC:1;
    // ITC
    // 0   No Data
    // 1   IT content(Byte 5 CN bits valid)
}AVI_B3;

typedef struct{ // low bit first
    uint08 YQ:2;
    // YQ1 YQ0 YCC Quantization Range
    // 0 0 Limited Range
    // 0 1 Full Range
    // 1 0 Reserved
    // 1 1 Reserved
    uint08 CN:2;
    // CN1 CN0 IT Content Type
    // 0 0 Graphics
    // 0 1 Photo
    // 1 0 Cinema
    // 1 1 Game
    uint08 PR:4;
    // PR3 PR2 PR1 PR0 Pixel Repetition Factor
    // 0 0 0 0 No Repetition (i.e., pixel date sent once)
    // 0 0 0 1 Pixel Data sent 2 times (i.e., repeated once)
    // 0 0 1 0 Pixel Data sent 3 times
    // 0 0 1 1 Pixel Data sent 4 times
    // 0 1 0 0 Pixel Data sent 5 times
    // 0 1 0 1 Pixel Data sent 6 times
    // 0 1 1 0 Pixel Data sent 7 times
    // 0 1 1 1 Pixel Data sent 8 times
    // 1 0 0 0 Pixel Data sent 9 times
    // 1 0 0 1 Pixel Data sent 10 times
    // 0x0A-0x0F Reserved
}AVI_B5;

typedef struct{
    uint08 checksum;    // Byte  0: checksum
    AVI_B1 byte1;       // Byte  1: [Y2] Y1 Y0 A0 B1 B0 S1 S0
    AVI_B2 byte2;       // Byte  2: C1 C0 M1 M0 R3 R2 R1 R0
    AVI_B3 byte3;       // Byte  3: ITC EC2 EC1 EC0 Q1 Q0 SC1 SC0
    uint08 vic;         // Byte  4: [VIC7] VIC6 VIC5 VIC4 VIC3 VIC2 VIC1 VIC0;
    AVI_B5 byte5;       // Byte  5: YQ1 YQ0 CN1 CN0 PR3 PR2 PR1 PR0
    uint08 linenumETBL; // Byte  6: ETB07-ETB00 (Line Number of End of Top Bar – lower 8 bits)
    uint08 linenumETBH; // Byte  7: ETB15-ETB08 (Line Number of End of Top Bar – upper 8 bits)
    uint08 linenumSBBL; // Byte  8: SBB07-SBB00 (Line Number of Start of Bottom Bar – lower 8 bits)
    uint08 linenumSBBH; // Byte  9: SBB15-SBB08 (Line Number of Start of Bottom Bar – upper 8 bits)
    uint08 linenumELBL; // Byte 10: ELB07-ELB00 (Pixel Number of End of Left Bar – lower 8 bits)
    uint08 linenumELBH; // Byte 11: ELB15-ELB08 (Pixel Number of End of Left Bar – upper 8 bits)
    uint08 linenumSRBL; // Byte 12: SRB07-SRB00 (Pixel Number of Start of Right Bar – lower 8 bits)
    uint08 linenumSRBH; // Byte 13: SRB15-SRB08 (Pixel Number of Start of Right Bar – upper 8 bits)
                        // Byte 14~27: Reserved
}AVI_INFOFRAME; // Auxiliary Video Information

typedef struct{
    uint08 eotf;        //Data Byte 1 Electro-Optical Transfer Function
                        //     bit [2:0]
                        //     0 Traditional gamma - SDR Luminance
                        //     1 Traditional gamma - HDR Luminance
                        //     2 SMPTE ST 2084
                        //     3 Future EOTF
                        //     4~7 Reserved for future use

    uint08 metaID;      //Data Byte 2 Static_Metadata_Descriptor_ID
                        //     bit [2:0]
                        //     0 Static Metadata Type 1
                        //     1~7 Reserved for future use

    // Data Bytes 3 – 18: ROUND(HEX2DEC("C350")*0.00002,4)
    //--- 顯示器的xyz 值(色域圖上上RGB的三點)以及白色的色座標值
    uint08 prim_x0L;    //Data Byte 3 display_primaries_x[0], LSB
    uint08 prim_x0H;    //Data Byte 4 display_primaries_x[0], MSB
    uint08 prim_y0;     //Data Byte 5 display_primaries_y[0], LSB
    uint08 prim_y0H;    //Data Byte 6 display_primaries_y[0], MSB
    uint08 prim_x1L;    //Data Byte 7 display_primaries_x[1], LSB
    uint08 prim_x1H;    //Data Byte 8 display_primaries_x[1], MSB
    uint08 prim_y1L;    //Data Byte 9 display_primaries_y[1], LSB
    uint08 prim_y1H;    //Data Byte 10 display_primaries_y[1], MSB
    uint08 prim_x2L;    //Data Byte 11 display_primaries_x[2], LSB
    uint08 prim_x2H;    //Data Byte 12 display_primaries_x[2], MSB
    uint08 prim_y2L;    //Data Byte 13 display_primaries_y[2], LSB
    uint08 prim_y2H;    //Data Byte 14 display_primaries_y[2], MSB
    uint08 white_pxL;   //Data Byte 15 white_point_x, LSB
    uint08 white_pxH;   //Data Byte 16 white_point_x, MSB
    uint08 white_pyL;   //Data Byte 17 white_point_y, LSB
    uint08 white_pyH;   //Data Byte 18 white_point_y, MSB

    // Data Bytes 19 – 20: 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2
    //---- 顯示器的的白色最大輝度值
    uint08 maxMstLumL;   //Data Byte 19 max_display_mastering_lumniance LSB
    uint08 maxMstLumH;   //Data Byte 20 max_display_mastering_lumniance MSB

    // Data Bytes 21 – 22: 0x0001 represents 0.0001 cd/m2 and 0xFFFF represents 6.5535 cd/m2
    //---- 顯示器的的白色最小輝度值
    uint08 minMstLumL;   //Data Byte 21 min_display_mastering_lumniance LSB
    uint08 minMstLumH;   //Data Byte 22 min_display_mastering_lumniance MSB

    // Data Bytes 23 – 24: 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2
    //--- 影片內容的最大亮度值
    uint08 maxContLvL;  //Data Byte 23 max_content_light_level LSB
    uint08 maxContLvH;  //Data Byte 24 max_content_light_level MSB

    // Data Bytes 25 – 26: 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2
    //--- 在一個畫面中，最大亮度的平均值
    uint08 maxFrmAvLvL; //Data Byte 25 Maximum Frame Average Light Level LSB
    uint08 maxFrmAvLvH; //Data Byte 26 Maximum Frame Average Light Level MSB
    uint08 Reserved[2];    //Reserved
}HDR_INFOFRAME; // HDR Information

#define HDR_INFOFRAME_SIZE sizeof(HDR_INFOFRAME)/sizeof(uint08)

typedef struct{
    uint08 eotf : 3;        //Data Byte 1 Electro-Optical Transfer Function
        //     bit [2:0]
        //     0 Traditional gamma - SDR Luminance
        //     1 Traditional gamma - HDR Luminance
        //     2 SMPTE ST 2084
        //     3 Future EOTF
        //     4~7 Reserved for future use
    uint08      : 0;
    uint08 metaID : 1;      //Data Byte 2 Static_Metadata_Descriptor_ID
        //     bit [2:0]
        //     0 Static Metadata Type 1
        //     1~7 Reserved for future use
    uint08      : 0;
    // Data Bytes 3 – 18: ROUND(HEX2DEC("C350")*0.00002,4)
    //--- 顯示器的xyz 值(色域圖上上RGB的三點)以及白色的色座標值
    uint16 prim_x0;    //Data Byte 3 display_primaries_x[0], LSB
    uint16 prim_y0;    //Data Byte 5 display_primaries_y[0], LSB
    uint16 prim_x1;    //Data Byte 7 display_primaries_x[1], LSB
    uint16 prim_y1;    //Data Byte 9 display_primaries_y[1], LSB
    uint16 prim_x2;    //Data Byte 11 display_primaries_x[2], LSB
    uint16 prim_y2;    //Data Byte 13 display_primaries_y[2], LSB
    uint16 white_px;   //Data Byte 15 white_point_x, LSB
    uint16 white_py;   //Data Byte 17 white_point_y, LSB

    // Data Bytes 19 – 20: 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2
    //---- 顯示器的的白色最大輝度值
    uint16 maxMstLum;   //Data Byte 19 max_display_mastering_lumniance LSB

    // Data Bytes 21 – 22: 0x0001 represents 0.0001 cd/m2 and 0xFFFF represents 6.5535 cd/m2
    //---- 顯示器的的白色最小輝度值
    uint16 minMstLum;   //Data Byte 21 min_display_mastering_lumniance LSB

    // Data Bytes 23 – 24: 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2
    //--- 影片內容的最大亮度值
    uint16 maxContLv;  //Data Byte 23 max_content_light_level LSB

    // Data Bytes 25 – 26: 0x0001 represents 1 cd/m2 and 0xFFFF represents 65535 cd/m2
    //--- 在一個畫面中，最大亮度的平均值
    uint16 maxFrmAvLv; //Data Byte 25 Maximum Frame Average Light Level LSB
    uint08 Reserved[2];    //Reserved
}HDR_INFOFRAME_PROAV; // HDR Information

#define HDR_INFOFRAME_PROAV_SIZE sizeof(HDR_INFOFRAME_PROAV)/sizeof(uint08)

typedef struct{ // low bit first
    uint08 VideoFmt:3;
    uint08 Reserved:5;
}VSI_B4;

typedef struct{ // low bit first
    uint08 Structure:4;
    uint08 Reserved:4;
}VSI_B5_STRU;

typedef struct{ // low bit first
    union {
        VSI_B5_STRU stru;
        uint08 vic;
    };
}VSI_B5;

typedef struct{ // low bit first
    uint08 ExtData:4;
    uint08 Reserved:4;
}VSI_B6;

typedef struct{
    uint08 byte0;       // Byte  0: checksum
    uint08 byte1;       // Byte  1: Low byte of 24bit IEEE Registration Indentifier (0x000C03), LSB first
    uint08 byte2;       // Byte  2: Middle byte of 24bit IEEE Registration Indentifier (0x000C03), LSB first
    uint08 byte3;       // Byte  3: High byte of 24bit IEEE Registration Indentifier (0x000C03), LSB first
    VSI_B4 videoFmt;    // Byte  4: HDMI video format;
    VSI_B5 byte5;       // Byte  5: HDMI VIC or 3D structure
    VSI_B6 extData;     // Byte  6: 3D Extend Data
}VSI_INFOFRAME;         // Vendor Specific Infoframe

#pragma pack(pop)

/*typedef struct{
    uint08 Length:5;        // Byte  0: checksum
    uint08 Checksum:8;
    uint32 IEEE:24;         // Byte  1: Low byte of 24bit IEEE Registration Indentifier (0x000C03), LSB first
    uint08 Reserved1:5;
    uint08 VideoFormat:3;
    uint08 VIC:8;
    uint08 Reserved2:5;
    uint08 Ext3D:3;
    uint08 Reserved3:3;
}VSI_INFOFRAME_PROAV;*/       // Vendor Specific Infoframe

typedef enum
{
    eAVI_PIXEL_MODE_RGB,
    eAVI_PIXEL_MODE_YCbCr422,
    eAVI_PIXEL_MODE_YCbCr444,
    eAVI_PIXEL_MODE_YCbCr420,
} eAVI_PIXEL_MODE;

typedef enum
{
    eAVI_RGB_RANGE_DEFAULT,
    eAVI_RGB_RANGE_LIMITED,
    eAVI_RGB_RANGE_FULL,
} eAVI_RGB_RANGE;

typedef enum
{
    eAVI_COLORIMETRY_NODATA,
    eAVI_COLORIMETRY_SMTPE_170M,
    eAVI_COLORIMETRY_ITUR_BT709,
    eAVI_COLORIMETRY_EXCOLORIMETRY,
} eAVI_COLORIMETRY;

typedef enum
{
    eAVI_EXCOLORIMETRY_xvYCC601,
    eAVI_EXCOLORIMETRY_xvYCC709,
    eAVI_EXCOLORIMETRY_sYCC601,
    eAVI_EXCOLORIMETRY_AdobeYCC601,
    eAVI_EXCOLORIMETRY_AdobeRGB,
    eAVI_EXCOLORIMETRY_ITUR_BT2020_YCBCR,
    eAVI_EXCOLORIMETRY_ITUR_BT2020_RGB_OR_YCBCR,
    eAVI_EXCOLORIMETRY_Reserved,
} eAVI_EXCOLORIMETRY;

typedef enum{
    eNoHDMIFormat = 0,
    e2DFormat,
    e3DFormat,
    eHDMIVideoFormatMax
} HDMIVideoFormat;

typedef enum{
    eScl3DMode_Off = 0,
    eScl3DMode_FramePacking,
    eScl3DMode_SideBySide,
    eScl3DMode_TopAndBottom

} Scl3DMode;

#endif // _HDMISTANDARD_H
