#ifndef TYPE_DEF_H
#define TYPE_DEF_H

#include <stdint.h>

typedef uint8_t     u8_t;
typedef uint16_t    u16_t;
typedef uint32_t    u32_t;
typedef int8_t      s8_t;
typedef int16_t     s16_t;
typedef int32_t     s32_t;
typedef uint8_t     bool_t;

typedef uint32_t    mem_ptr_t;

typedef void        VOID;
typedef uint8_t     BOOLEAN;
typedef int8_t      INT8S;
typedef uint8_t     INT8U;
typedef uint16_t    INT16U;
typedef int16_t     INT16S;
typedef uint32_t    INT32U;
typedef int32_t     INT32S;

typedef uint8_t     BOOL;
typedef uint8_t     UINT8,  *PUINT8,  uint8;   /* 8-bit unsigned  */
typedef uint16_t    UINT16, *PUINT16, uint16;   /* 16-bit unsigned */
typedef uint32_t    UINT32, *PUINT32, uint32;   /* 32-bit unsigned */
typedef uint64_t    UINT64, *PUINT64, uint64;   /* 64-bit unsigned */

typedef int8_t      INT8,  *PINT8;  /* 8-bit unsigned  */
typedef int16_t     INT16, *PINT16; /* 16-bit unsigned */
typedef int32_t     INT32, *PINT32; /* 32-bit unsigned */
typedef int64_t     INT64, *PINT64; /* 64-bit unsigned */
typedef float       FLOAT, *PFLOAT;
typedef double      DOUBLE, *PDOUBLE;

//typedef float       FLOAT;
//typedef double      DOUBLE;
//typedef void        VOID;

typedef char          BYTE;
typedef short int     WORD;
typedef int           DWORD;
typedef char *        PBYTE;

#endif //TYPE_DEF_H
