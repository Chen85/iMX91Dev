#include "dvProAV_Uniformity.h"

/**
 * @brief Set Uniformity Enable / Disbale
 * @param [in] enable
 *             -true: ColorUniformity enable
 *             -false: ColorUniformity disable
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UnifEnableSet(bool bEnable)
{
    return dvProAV_AccessWrite(eUnifEnable, bEnable);
}

/**
 * @brief Get Uniformity Enable / Disbale
 * @return Uniformity Enable / Disbale
 *             -true: ColorUniformity enable
 *             -false: ColorUniformity disable
 */
bool dvProAV_UnifEnableGet(void)
{
    uint32 uRData = 0;

    dvProAV_AccessRead(eUnifEnable, &uRData);
    return (bool)uRData;
}

/**
 * @brief Set Uniformity Data
 * @param [in] u16Offset : 資料起始點
 * @param [in] u8HCount : 水平區塊數
 * @param [in] u8VCount : 垂直區塊數
 * @param [in] u8Data : 寫入的資料
 * @param [in] u16Len : 寫入的長度
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UnifDataSet(uint16 u16Offset, uint08 u8HCount, uint08 u8VCount, uint08 *u8Data, uint16 u16Len)
{
    int status = rcSUCCESS;
    uint08 offsetL = (uint08)(u16Offset & 0xff);
    uint08 offsetH = (uint08)((u16Offset >> 8) & 0xff);

    // Write the value into the buffer enable
    status &= dvProAV_AccessWrite(eUnifHtotBlock, (uint32)u8HCount);
    status &= dvProAV_AccessWrite(eUnifVtotBlock, (uint32)u8VCount);
    status &= dvProAV_AccessWrite(eUnifTableWrite, (uint32)true);  //set buffer to write mode
    status &= dvProAV_AccessWrite(eUnifLutAdrPort, offsetL);
    status &= dvProAV_AccessWrite(eUnifLutAdrPort, offsetH);
    status &= dvProAV_AccessBurstWrite(eUnifLutDataPort, u8Data, u16Len, BURST_FIX_ADDR); // Fixed Address if last parameter is false
    status &= dvProAV_AccessWrite(eUnifTableWrite, (uint32)false); // set buffer to read mode

    return status;
}

/**
 * @brief Set LSB shift of Uniformity Data
 * @param [in] u8ShiftBits : bits of LSB shift
 *             -0: no shift    , (1<<0)
 *             -1: 2 bits shift, (1<<1)
 *             -2: 4 bits shift, (1<<2)
 * @return status : rcSUCCESS / rcERROR / rcINVALID
 */
int dvProAV_UnifDataShiftSet(uint08 u8ShiftBits)
{
    return dvProAV_AccessWrite(eUnifVGofHGGSft, (uint32)u8ShiftBits);
}
