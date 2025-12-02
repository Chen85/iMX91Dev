#include <math.h>
#include "dvProAV_Base.h"
#include "dvProAV_SclGammaTable.h"

//#define IN_FILE_DEBUG
#ifndef IN_FILE_DEBUG
    #undef DBMSG
    #define DBMSG(...) do{}while(0)
#endif


#define InputBits     10
#define OutputBits    12
#define GammaColors   3                  // 0: R, 1:G, B:2
#define GammaSteps    1024               // 1 << InputBits
#define GammaStepsMax 4096               // 1 << InputBits
#define OutputBitsMax ((double)4095)     //((double)((1<<OutputBits)-1))
#define InputBitsMax  ((double)1023)     //((double)((1<<InputBits)-1))

static uint16 m_arSclGammaTable[GammaColors][GammaStepsMax];

/**
 * @brief Load ProAV Scaler Gamma table
 * @param [in] u8ColorId: Scaler Color index
 * @param [out] data : ProAV Scaler Gamma Table Pointer
 */
void dvProAV_SclGammaTableSet(uint08 u8ColorId, uint16 *u16Data)
{
    u8ColorId %= 3;

    for (uint16 i= 0 ;i <GammaStepsMax; i++)
    {
        m_arSclGammaTable[u8ColorId][i] = u16Data[i];
    }
}

/**
 * @brief Caculate ProAV Scaler Gamma table by power
 * @param [in] dPower : gamma power
 * @param [in] u9ColorId : color, 0: R, 1:G, B:2
 * @return ProAV Scaler Table Pointer
 */
uint16 *dvProAV_SclGammaTableCaculate(double dPower, uint08 u9ColorId)
{
    u9ColorId %= GammaColors;
    dvProAV_GammaTableCaculate(InputBits, OutputBits, InputBits, dPower, m_arSclGammaTable[u9ColorId]);

    return (uint16 *)m_arSclGammaTable[u9ColorId];
}

/**
 * @brief Caculate Dual Pipe ProAV Scaler Gamma table by power
 * @param [in] dMainPower : main gamma power
 * @param [in] dSubPower : sub gamma power
 * @param [in] u8ColorId : color, 0: R, 1:G, B:2
 * @return ProAV Scaler Table Pointer
 */
uint16 *dvProAV_SclDualGammaTableCaculate(double dMainPower, double dSubPower, uint08 u8ColorId)
{
    u8ColorId %= GammaColors;
    dvProAV_GammaTableCaculate((InputBits - 1), OutputBits, (InputBits - 1), dSubPower, m_arSclGammaTable[u8ColorId]);
    dvProAV_GammaTableCaculate((InputBits - 1), OutputBits, (InputBits - 1), dMainPower, (m_arSclGammaTable[u8ColorId] + (uint32)(1 << (InputBits - 1))));

    return (uint16 *)m_arSclGammaTable[u8ColorId];
}

/**
 * @brief Caculate ProAV Blend Gamma table by power
 * @param [in] ucInputBits: input gamma step bits
 * @param [in] ucOutputBits: output gamma step bits
 * @param [in] ucScaleBits: one step scale bits
 * @param [in] dPower: gamma power
 * @param [in] u8ColorId
 * @return ProAV Scaler Table Pointer
 */
uint16 *dvProAV_BldGammaTableCaculate(uint08 ucInputBits, uint08 ucOutputBits, uint08 ucScaleBits, double dPower, uint08 u8ColorId)
{
    u8ColorId %= GammaColors;
    dvProAV_GammaTableCaculate(ucInputBits, ucOutputBits, ucScaleBits, dPower, m_arSclGammaTable[u8ColorId]);

    return (uint16 *)m_arSclGammaTable[u8ColorId];
}

/**
 * @brief Get ProAV Scaler Gamma table
 * @param [in] colorId
 * @return ProAV Scaler Gamma Table Pointer
 */
uint16 *dvProAV_SclGammaTableGet(uint08 u8ColorId)
{
    u8ColorId %= GammaColors;
    return (uint16 *)m_arSclGammaTable[u8ColorId];
}


/**
 * @brief Get the Scaler Gamma Table Item Count
 * @return Byte Count of ProAV Scaler Gamma table
 */
uint16 dvProAV_SclGammaItemCountGet(void)
{
    return GammaSteps;
}
