// ==============================================================================
// FILE NAME: UITLMATH.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 02/01/2014, Leo Create
// --------------------
// ==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include "MathAPI.h"

//For compare 64bit data
int QWord_Compare(const void *arg1, const void *arg2)
{
    UINT64 dwArg1 = *(UINT64 *)(arg1);
    UINT64 dwArg2 = *(UINT64 *)(arg2);

    if(dwArg1 < dwArg2)
    {
        return -1;
    }

    if(dwArg1 == dwArg2)
    {
        return 0;
    }

    return 1;
}


// ==============================================================================
// FUNCTION NAME: DWordCompare
// DESCRIPTION:
//
// 快速排序所需之比較程式
//
// Params:
// const void *arg1:
// const void *arg2:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/28, Leo Create
// --------------------
// ==============================================================================
int DWord_Compare(const void *arg1, const void *arg2)
{
    UINT32 dwArg1 = *(UINT32 *)(arg1);
    UINT32 dwArg2 = *(UINT32 *)(arg2);

    if(dwArg1 < dwArg2)
    {
        return -1;
    }

    if(dwArg1 == dwArg2)
    {
        return 0;
    }

    return 1;
}

// ==============================================================================
// FUNCTION NAME: WordCompare
// DESCRIPTION:
//
// 快速排序所需之比較程式
//
// Params:
// const void *arg1:
// const void *arg2:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/28, Leo Create
// --------------------
// ==============================================================================
int Word_Compare(const void *arg1, const void *arg2)
{
    UINT16 wArg1 = *(UINT16 *)(arg1);
    UINT16 wArg2 = *(UINT16 *)(arg2);

    if(wArg1 < wArg2)
    {
        return -1;
    }

    if(wArg1 == wArg2)
    {
        return 0;
    }

    return 1;
}

// ==============================================================================
// FUNCTION NAME: utilQSort
// DESCRIPTION:
//
//
// Params:
// const void *pvSrc:
// WORD wSrcSize:
// WORD wItemSize:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/28, Leo Create
// --------------------
// ==============================================================================
void MathAPI_QSort(void *pvSrc, UINT16 wSrcSize, UINT16 wItemSize)
{
    switch(wItemSize)
    {
        case 2:
            qsort(pvSrc, wSrcSize, wItemSize, Word_Compare);
            break;

        case 4:
            qsort(pvSrc, wSrcSize, wItemSize, DWord_Compare);
            break;

        case 8:
            qsort(pvSrc, wSrcSize, wItemSize, QWord_Compare);
            break;

        default:
            break;
    }
}

// Sine Table
const UINT16 m_awSinTable[10] =
{
    //  10   20   30   40   50   60   70   80   90
    0,  174, 342, 500, 643, 766, 866, 940, 985, 1000,
};

#define SIN_TABLE_COUNT sizeof(m_awSinTable)/sizeof(UINT16)
#define SIN_TABLE_STEP  10

// ==============================================================================
// FUNCTION NAME: util_Asin
// DESCRIPTION:
//
//
// Params:
// INT nSin:
//
// Returns:
//
//
// Modification History
// --------------------
// 2014/09/28, Leo Create
// --------------------
// ==============================================================================
INT16 MathAPI_Asin(INT16 nSin)
{
    BOOL    bNegative = FALSE;
    UINT8   cStartIndex, cEndIndex, cMidIndex;
    INT16   nAngle;

    // If the sin is less than 0, then the angle is negative since
    // sin -x = -sin x
    if(nSin < 0)
    {
        bNegative = TRUE;
        nSin = -nSin;
    }

    // Setup the start and end indexes for the arc sin search
    cStartIndex = 0;
    cEndIndex = SIN_TABLE_COUNT - 1;

    // Perform a binary search to find the arcsin angle
    while(cStartIndex != (cEndIndex - 1))
    {
        // cMidIndex = (cStartIndex + cEndIndex) / 2;
        cMidIndex = (cStartIndex + cEndIndex) >> 1;

        if(nSin >= m_awSinTable[cMidIndex])
        {
            cStartIndex = cMidIndex;
        }
        else
        {
            cEndIndex = cMidIndex;
        }
    }

    // Interpolate between the two closest values
    nAngle = ((INT16)cStartIndex * SIN_TABLE_STEP) +
             (INT16)ROUND((INT16)(nSin - m_awSinTable[cStartIndex]) * (INT16)SIN_TABLE_STEP,
                          (INT16)(m_awSinTable[cEndIndex] - m_awSinTable[cStartIndex])
                         );

    // If the sin was negative, invert the angle
    return (bNegative ? -nAngle : nAngle);
}


//return Key Index at Array
//if not found, return -1
INT32 BinarySearch_32bitData(UINT32 Key, UINT32 *Array, UINT32 ArraySize, UINT32 NextDataByteOffset)
{
   UINT32 first, last, middle;

   first = 0;
   last = ArraySize - 1;
   middle = (first+last)/2;

   while (first <= last)
   {
      //printf("%ul <%d %d %d>\n", *(UINT32 *)((char *)Array+(middle*NextDataByteOffset)), first, middle, last);

      if (*(UINT32 *)((char *)Array+(middle*NextDataByteOffset)) < Key)
      {
         first = middle + 1;
      }
      else if (*(UINT32 *)((char *)Array+(middle*NextDataByteOffset)) == Key)
      {
         //printf("%lu is found at index %d\n", Key, middle);
         return middle;
      }
      else
      {
         last = middle - 1;
      }

      middle = (first + last)/2;
   }

   if (first > last)
   {
      //printf("Not found %u\n", (UINT32)Key);
   }

   return -1;
}

//return Key Index at Array
//if not found, return -1
INT32 BinarySearch_64bitData(UINT64 Key, UINT64 *Array, UINT32 ArraySize, UINT32 NextDataByteOffset)
{
   UINT32 first, last, middle;

   first = 0;
   last = ArraySize - 1;
   middle = (first+last)/2;

   while (first <= last)
   {
      //printf("%ul <%d %d %d>\n", *(UINT64 *)((char *)Array+(middle*NextDataByteOffset)), first, middle, last);

      if (*(UINT64 *)((char *)Array+(middle*NextDataByteOffset)) < Key)
      {
         first = middle + 1;
      }
      else if (*(UINT64 *)((char *)Array+(middle*NextDataByteOffset)) == Key)
      {
         //printf("%lu is found at index %d\n", Key, middle);
         return middle;
      }
      else
      {
         last = middle - 1;
      }

      middle = (first + last)/2;
   }

   if (first > last)
   {
      //printf("Not found %u\n", (UINT32)Key);
   }

   return -1;
}

