#ifndef DV_PROAV_ACCESS_H
#define DV_PROAV_ACCESS_H

#include "dvProAV_Platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BURST_FIX_ADDR false
#define BURST_INC_ADDR true

typedef enum{
    eAccessIfPcSpi = 0,
    eAccessIfMcuSpi = 1,
    eAccessIfMcuPCIe = 2,
    eAccessIfMax
}AccessInterfaceType;

/**
 * @brief 經由區段值與位移值寫入一個位元組的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataWriteByte(const uint08 bank, const uint08 offset, uint08 data);

/**
 * @brief 經由區段值與位移值讀取一個位元組的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataReadByte(const uint08 bank, const uint08 offset, uint08 *data);

/**
 * @brief 經由區段值與位移值寫入指定長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataWrite(const uint08 bank, const uint08 offset, const uint16 len, uint08 *data);

/**
 * @brief 經由區段值與位移值讀取指定長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRawDataRead(const uint08 bank, const uint08 offset, const uint16 len, uint08 *data);

/**
 * @brief 經由列舉值寫入32位元內所需長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessWrite(const uint32 regEnum, const uint32 data);

/**
 * @brief 經由列舉值讀取32位元內所需長度的原始ProAV暫存器值
 * @param bank 區段值
 * @param offset 位移值
 * @param data 暫存器的值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessRead(const uint32 regEnum, uint32 *data);

/**
 * @brief 設定突發(Burst)模式時寫入暫存器位址為固定或自動增加
 * @param enable
 *        - true: 自動增加暫存器位址
 *        - false: 固定暫存器位址
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstAddrIncEnable(const bool enable);

/**
 * @brief 取得目前突發(Burst)模式時寫入暫存器位址為固定或自動增加的設定值
 * @param enable
 *        - true: 自動增加暫存器位址
 *        - false: 固定暫存器位址
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstAddrIncEnabledGet(bool *enable);

/**
 * @brief 以突發(Burst)模式寫入指定的長度資料與模式
 * @param regEnum 暫存器列舉值
 * @param data 資料緩衝區指標
 * @param count 寫入的資料長度
 * @param autoIncAddrEnabled 暫存器位址為固定或自動增加的設定值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstWrite(const uint32 regEnum, uint08 *data, const uint16 count, const bool incAddrEnabled);

/**
 * @brief 以突發(Burst)模式讀取指定的長度資料與模式
 * @param regEnum 暫存器列舉值
 * @param data 資料緩衝區指標
 * @param count 讀取的資料長度
 * @param autoIncAddrEnabled 暫存器位址為固定或自動增加的設定值
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessBurstRead(const uint32 regEnum, uint08 *data, const uint16 count, const bool incAddrEnabled);

/**
 * @brief 設定ProAV存取的函式與初始值
 * @param nInterfaceType:
 *        0:PcSpi
 *        1:McuSpi
 *        2:McuPCIe(i.mx)
 * @param pfuncRead
 * @param pfuncWrite
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
#ifndef QT_CPP
int dvProAV_AccessInit(int nInterfaceType ,pfuncDeviceRead pfuncRead, pfuncDeviceWrite pfuncWrite, pfuncDeviceBufferWrite pfuncBufferWrite);
#else
int dvProAV_AccessInit(int nInterfaceType ,pfuncDeviceRead pfuncRead, pfuncDeviceWrite pfuncWrite);
#endif
/**
 * @brief 讀取存取介面類型
 * @return interfaceType:
 *        0:PcSpi
 *        1:McuSpi
 *        2:McuPCIe(i.mx)
 */
int dvProAV_AccessInterfaceGet(void);

/**
 * @brief 設定ProAV除錯遮罩
 * @param mask 除錯遮罩
 * @return (無)
 */
void dvProAV_AccessDebugMaskSet(uint08 mask);

/**
 * @brief 設定使用ProAV突發(Burst)模式時為真實突發模式或使用單一模式模擬
 * @param enable
 *        - true : 真實突發模式
 *        - false　:　單一模式模擬
 * @return (無)
 */
void dvProAV_AccessBurstModeEnable(bool enable);

/**
 * @brief 設定使Write Check模式
 * @param enable
 *        - true : Write Check模式
 *        - false　:　無Write Check模式
 * @return (無)
 */
void dvProAV_AccessWriteCheckEnableSet(bool enable);

/**
 * @brief 打開暫存器讀寫紀錄檔
 * @return 成功(rcSUCCESS)或失敗(rcERROR)
 */
int dvProAV_AccessLogOpen(void);

/**
 * @brief 列印到暫存器讀寫紀錄檔
 * @param (同printf)
 * @return 列印資料長度
 */
int dvProAV_AccessPrint(const char *message, ...);

/**
 * @brief 關閉暫存器讀寫紀錄檔
 * @return (無)
 */
void dvProAV_AccessLogClose(void);

int dvProAV_AccessReconfig(void);

#ifdef __cplusplus
}
#endif

#endif // DV_PROAV_ACCESS_H
