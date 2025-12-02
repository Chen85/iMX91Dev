#ifndef OE_TOOLPARAMETER_H
#define OE_TOOLPARAMETER_H

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#define OE_HEADER_OFFSET            32
#define OE_HEADER_MAIN_VER          0
#define OE_HEADER_SUB_VER           1
#define OE_HEADER_PIC_NUM           2   //picture mode number
#define OE_HEADER_CUSTOM_CT_NUM     3   //OSD color Temperature number
#define OE_HEADER_CT_NUM            4   //OE color Temperature number

#define OE_PICTUREMODE_SIZE         20
#define OE_PWMTABLE_NUMBER          1 //BLD, RLD, GLD...
#define OE_COLORTEMPERATURE_SIZE    128
#define OE_CUSTOM_CT_SIZE           256
#define OE_GAMMA_CM_SIZE            38

#define SYSTEM_MODE_DEFAULT 0
#define SYSTEM_MODE_WARMEST 1
#define SYSTEM_MODE_WARM    2
#define SYSTEM_MODE_COOL    3
#define SYSTEM_MODE_OE_TOOL 7

//PWM table
typedef struct {
    int16_t Red;
    int16_t Green;
    int16_t Blue;
    int16_t Yellow;
} sOE_PWM_TABLE;

typedef struct {
    int16_t WheelSpeed; //0->(2x), 1->(3X), 2->(4X)
    int16_t System_mode[SYSTEM_MODE_OE_TOOL + 1];
}sOE_SYSTEM_INFO;

typedef struct {
    sOE_PWM_TABLE s100;
    sOE_PWM_TABLE s90;
    sOE_PWM_TABLE s80;
    sOE_PWM_TABLE s70;
    sOE_PWM_TABLE s60;
    sOE_PWM_TABLE s50;
    sOE_PWM_TABLE s40;
    sOE_PWM_TABLE s30;
    sOE_PWM_TABLE s20;
    sOE_PWM_TABLE s10;
    sOE_PWM_TABLE sEco1; // 80%
    sOE_PWM_TABLE sEco2; // 50%
} sOE_PWMTable;

typedef struct {
    int16_t mode;
    //sOE_PWM_INFO pwm_info;
    sOE_PWMTable pwm_value[OE_PWMTABLE_NUMBER];
} sOE_PWM_SETTING;

typedef struct {
    int8_t header[OE_HEADER_OFFSET];
    sOE_PWM_SETTING setting[OE_PICTUREMODE_SIZE];
} sOE_PWM_SETTING_DATA;

//Color Table
typedef struct {
    uint8_t ucCS;
    uint8_t ucCT;
    uint8_t ucGamma;
    uint8_t ucBrillientColorEnabled;
    uint8_t ucWhitePeaking;
    uint8_t ucColorEnhancement;
    uint8_t ucSkinColor;
    uint8_t ucSharpness;
    uint8_t ucBrightness;
    uint8_t ucContrast;
    uint8_t ucTint;
    uint8_t ucSaturation;
    uint8_t ucRedGain;
    uint8_t ucGreenGain;
    uint8_t ucBlueGain;
    uint8_t ucRedOffset;
    uint8_t ucGreenOffset;
    uint8_t ucBlueOffset;
    uint8_t ucWallColorSet;
} sOE_COLOR_TABLE;

typedef struct {
    int16_t mode;
    //int16_t WheelSpeed; //0->(2x), 1->(3X), 2->(4X)
    sOE_COLOR_TABLE color_table;
} sOE_COLOR_SETTING;

typedef struct {
    uint8 header[OE_HEADER_OFFSET];
    sOE_COLOR_SETTING setting[OE_PICTUREMODE_SIZE];
} sOE_COLOR_SETTING_DATA;

//HSG
typedef struct {
    int16_t Hue;
    int16_t Saturation;
    int16_t Gain;
} sOE_HSG_COLOR;

typedef struct {
    int16_t RedGain;
    int16_t GreenGain;
    int16_t BlueGain;
} sOE_HSG_WHITE_GAIN;

typedef struct {
    sOE_HSG_COLOR sRed;
    sOE_HSG_COLOR sGreen;
    sOE_HSG_COLOR sBlue;
    sOE_HSG_COLOR sCyan;
    sOE_HSG_COLOR sMagenta;
    sOE_HSG_COLOR sYellow;
    sOE_HSG_WHITE_GAIN sWhite;
} sOE_HSGTable;

typedef struct {
    int16_t mode;
    int16_t WheelSpeed; //NA //0->(2x), 1->(3X), 2->(4X)
    sOE_HSGTable colors;
} sOE_HSG_SETTING;

typedef struct {
    uint8 header[OE_HEADER_OFFSET];
    sOE_HSG_SETTING setting[OE_PICTUREMODE_SIZE];
} sOE_HSG_SETTING_DATA;

//Color Temerature
typedef struct {
    int16_t colortemp;

    struct
    {
        int16_t pic_index;
        sOE_HSG_WHITE_GAIN colors;
    } picturemode[OE_PICTUREMODE_SIZE];
} sOE_CT_TABLE;

typedef struct {
    uint8 header[OE_HEADER_OFFSET];
    sOE_CT_TABLE setting[OE_COLORTEMPERATURE_SIZE];
} sOE_CT_SETTING_DATA;

typedef struct {
    int16_t picturemode;
    int16_t colortemp[OE_COLORTEMPERATURE_SIZE];
} sOE_CUSTOM_CT_TABLE;

typedef struct {
    uint8 header[OE_HEADER_OFFSET];
    sOE_CUSTOM_CT_TABLE setting[OE_PICTUREMODE_SIZE];
} sOE_CUSTOM_CT_SETTING_DATA;

typedef struct {
    int16_t mode;
    sOE_SYSTEM_INFO system_mode_info;
} sOE_SYSTEMMODE_SETTING;

typedef struct {
    uint8 header[OE_HEADER_OFFSET];
    sOE_SYSTEMMODE_SETTING setting[OE_PICTUREMODE_SIZE];
} sOE_SYSTEM_SETTING_DATA;

typedef struct {
    uint16 gamma_cm;
    uint16 gamma_oe;
} sOE_GAMMA_SETTING;

typedef struct {
    uint8 header[OE_HEADER_OFFSET];
    sOE_GAMMA_SETTING setting[OE_GAMMA_CM_SIZE];
} sOE_GAMMA_SETTING_DATA;

#pragma pack(pop)

#endif /* OE_TOOLPARAMETER_H */

