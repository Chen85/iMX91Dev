#ifndef __UTILMATHAPI_H__
#define __UTILMATHAPI_H__


//#include "Common.h"

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

//int WordCompare(const void *arg1, const void *arg2);
//int DWordCompare(const void *arg1, const void *arg2);
//void utilQSort(void *pvSrc, UINT16 wSrcSize, UINT16 wItemSize);
//INT16 util_Asin(INT16 nSin);
UINT16 util_PositiveInterPlacement(UINT32 dwY_Hi, UINT32 dwY_Lo, UINT32 dwX_In, UINT32 dwX_Hi, UINT32 dwX_Lo); //A70LV_Larry_0142
UINT32 CalcChecksum(UINT8 *pnData, UINT32 nSize);

#endif /* __UTILMATHAPI_H__ */

