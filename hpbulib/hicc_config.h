#ifndef HICC_CONFIG_H
#define HICC_CONFIG_H
//-----------------------------------------------------------------------------------
#define BIT_SET(v, i)       do{ v |= (1 << i); }while(0)
#define BIT_CHECK(v, i)     ((1 << i) & v)
#define BIT_CLEAR(v, i)     do{ v &= ~(1 << i); }while(0)
#define BITS_SET_TRUE(v, i)  do{ v |= (1 << i); }while(i--)
#define BITS_SET_FALSE(v, i) do{ v &= ~(1 << i); }while(i--)

//-----------------------------------------------------------------------------------
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif
#ifndef true
#define true    1
#endif
#ifndef false
#define false   0
#endif

#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80
#define BIT8      0x0100
#define BIT9      0x0200
#define BIT10     0x0400
#define BIT11     0x0800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

#define COMMON_ARRAYSIZE_4      4
#define COMMON_ARRAYSIZE_8      8
#define COMMON_ARRAYSIZE_16     16
#define COMMON_ARRAYSIZE_32     32
#define COMMON_ARRAYSIZE_64     64
#define COMMON_ARRAYSIZE_128    128
#define COMMON_ARRAYSIZE_256    256
#define COMMON_ARRAYSIZE_512    512
#define COMMON_ARRAYSIZE_1024   1024
//-----------------------------------------------------------------------------------
#ifndef NULL
    #define NULL ((void*)0)
#endif
//-----------------------------------------------------------------------------------
#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif
//----------------------------------------------------------------------------------
typedef union
{
    UINT16 Val;
    UINT8  v[2];
    struct
    {
        UINT8 LB;
        UINT8 HB;
    } byte;

    struct
    {
        unsigned int b0: 1;
        unsigned int b1: 1;
        unsigned int b2: 1;
        unsigned int b3: 1;
        unsigned int b4: 1;
        unsigned int b5: 1;
        unsigned int b6: 1;
        unsigned int b7: 1;
        unsigned int b8: 1;
        unsigned int b9: 1;
        unsigned int b10: 1;
        unsigned int b11: 1;
        unsigned int b12: 1;
        unsigned int b13: 1;
        unsigned int b14: 1;
        unsigned int b15: 1;
    } bits;
} UINT16_VAL, UINT16_BITS;

typedef union
{
    UINT32 Val;
    UINT8 v[4];

    struct
    {
        unsigned int b0: 1;
        unsigned int b1: 1;
        unsigned int b2: 1;
        unsigned int b3: 1;
        unsigned int b4: 1;
        unsigned int b5: 1;
        unsigned int b6: 1;
        unsigned int b7: 1;
        unsigned int b8: 1;
        unsigned int b9: 1;
        unsigned int b10: 1;
        unsigned int b11: 1;
        unsigned int b12: 1;
        unsigned int b13: 1;
        unsigned int b14: 1;
        unsigned int b15: 1;
        unsigned int b16: 1;
        unsigned int b17: 1;
        unsigned int b18: 1;
        unsigned int b19: 1;
        unsigned int b20: 1;
        unsigned int b21: 1;
        unsigned int b22: 1;
        unsigned int b23: 1;
        unsigned int b24: 1;
        unsigned int b25: 1;
        unsigned int b26: 1;
        unsigned int b27: 1;
        unsigned int b28: 1;
        unsigned int b29: 1;
        unsigned int b30: 1;
        unsigned int b31: 1;
    } bits;
} UINT32_VAL, UINT32_BITS;
//-----------------------------------------------------------------------------------
//#define SHOW_DEBUGP_MESSAGE
#ifdef SHOW_DEBUGP_MESSAGE
#define DEBUGP printf
#else
#define DEBUGP
#endif

#define HICC_PRINTF(p) printf("[FILE:%s][FUN.:%s][LINE:%d] %s \n", __FILE__, __FUNCTION__, __LINE__, p)
//-----------------------------------------------------------------------------------
#endif //HICC_CONFIG_H
