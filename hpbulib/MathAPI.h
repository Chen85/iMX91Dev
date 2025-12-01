#ifndef __MATHAPI_H__
#define __MATHAPI_H__

#include "type_def.h"
#include "hicc_config.h"

#define BIT(X ,BITMSK)				((X) & (BITMSK))
#define SETBIT(X, BITMSK)			((X) |=(BITMSK))
#define CLRBIT(X,BITMSK)			((X) &= ~(BITMSK))
#define ASSIGNBIT(X,BITMSK,BIT)	    ((BIT) ? SETBIT(X,BITMSK) : CLRBIT(X,BITMSK))
#define GETBIT(X,BITMSK)			((BIT( X ,BITMSK) == BITMSK) ? 1 : 0)
#define ABS_DIFF(X,Y)               ((X)>=(Y) ? ((X)-(Y)):((Y)-(X)))
#define ROUND(X,Y)                  (((X)+((Y)>>1))/(Y))
#define COMBINE_MSB_LSB(X,Y)        ((BYTE)(X<<8)|(BYTE)(Y))
#define MAX(a, b)                   (((a) > (b)) ? (a) : (b))
#define MIN(a, b)                   (((a) < (b)) ? (a) : (b))
#define CLAMP(X, MIN, MAX)          (((X) > (MAX)) ? (MAX) : (((X) < (MIN)) ? (MIN) : (X)))
#define ABS(x)                      ((x)<0 ? -(x) : (x))
#define ROUND_UP(X,Y)               ((X) % (Y) ? (((X)/(Y)) + 1) : ((X)/(Y)))

int Word_Compare(const void *arg1, const void *arg2);
int DWord_Compare(const void *arg1, const void *arg2);
int QWord_Compare(const void *arg1, const void *arg2);
void MathAPI_QSort(void *pvSrc, UINT16 wSrcSize, UINT16 wItemSize);
INT16 MathAPI_Asin(INT16 nSin);
INT32 BinarySearch_32bitData(UINT32 Key, UINT32 *Array, UINT32 ArraySize, UINT32 NextDataByteOffset);
INT32 BinarySearch_64bitData(UINT64 Key, UINT64 *Array, UINT32 ArraySize, UINT32 NextDataByteOffset);



#endif /* __UTILMATHAPI_H__ */
