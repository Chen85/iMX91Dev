/*********************************************************************************
 * FILE NAME: GEC_EVENTTABLE.H
 * DESCRIPTION:
 *
 *
 * Date: 26 June, 2017
 * Author: John.Chung
*********************************************************************************/
#ifndef GEC_EVENTTABLE_H
#define GEC_EVENTTABLE_H


//This define is copy from generated file, do not edit directly
#define BurnInTime                   0x00010100
#define Reset                        0x00020100
#define IPv4                         0x00030100
#define IPv6                         0x00030200
#define SourceInfo                   0x00040000
#define SourceInfo01                 0x00040001
#define SourceInfo02                 0x00040002
#define SourceInfo03                 0x00040003
#define SourceInfo04                 0x00040004
#define SourceInfo05                 0x00040005
#define SourceInfo06                 0x00040006
#define SourceInfo07                 0x00040007
#define PictureMode                  0x00050000
#define PictureMode01                0x00050001
#define PictureMode02                0x00050002
#define PictureMode03                0x00050003
#define PictureMode04                0x00050004
#define PictureMode05                0x00050005
#define PictureMode06                0x00050006
#define PictureMode07                0x00050007
#define PictureMode08                0x00050008
#define OrientationAngle             0x00060000
#define HighAltitude                 0x00070000
#define LDSaturation                 0x00080000
#define LDSaturation01               0x00080001
#define LDSaturation02               0x00080002
#define LDSaturation03               0x00080003
#define LDSaturation04               0x00080004
#define LDSaturation05               0x00080005
#define LDSaturation06               0x00080006
#define LDSaturation07               0x00080007
#define LDSaturation08               0x00080008
#define NTP_GEC                      0x00F00000
#define GPS_GEC                      0x00F10000
#define SourceDetVGA                 0x00090100
#define SourceDetHDMI1               0x00098200
#define SourceDetHDMI2               0x00098201
#define SourceDetDVI                 0x00098100
#define SourceDetDP                  0x00098300
#define SourceDetHDBASET             0x00098500
#define SourceDetSDI                 0x00098400
#define SourceDetSlot1               0x00098600
#define SourceDetSlot2               0x00098601
#define SourceLostVGA                0x000A0100
#define SourceLostHDMI1              0x000A8200
#define SourceLostHDMI2              0x000A8201
#define SourceLostDVI                0x000A8100
#define SourceLostDP                 0x000A8300
#define SourceLostHDBASET            0x000A8500
#define SourceLostSDI                0x000A8400
#define SourceLostSlot1              0x000A8600
#define SourceLostSlot2              0x000A8601
#define PowerNotMatch                0x01010300
#define ACLost                       0x01020100
#define LVPSError                    0x01030200
#define Volt220VDetect               0x01040000
#define Volt110VDetect               0x01050000
#define Volt50VLost                  0x01060100
#define Volt12VLost                  0x01070100
#define Volt3p3VLost                 0x01080100
#define InterLockSwLost				 0x01090100 //HICC2_Doulas_0013
#define ThermalSwLost				 0x010A0100 //HICC2_Doulas_0013
#define LightModuleBroken            0x02010100
#define LightModuleBroken01          0x02010101
#define LightModuleBroken02          0x02010102
#define LightModuleBroken03          0x02010103
#define LightModuleBroken04          0x02010104
#define LightModuleBroken05          0x02010105
#define LightModuleBroken06          0x02010106
#define LightModuleBroken07          0x02010107
#define LightModuleBroken08          0x02010108
#define LightModuleBroken09          0x02010109
#define LightModuleBroken10          0x0201010A
#define LightModuleBroken11          0x0201010B
#define LightModuleBroken12          0x0201010C
#define LightModuleBroken13          0x0201010D
#define LightModuleBroken14          0x0201010E
#define LightModuleBroken15          0x0201010F
#define LightModuleCommunicationFail 0x02010300
#define LightModuleIgnitionFail      0x02010400
#define LightModuleLightUpFail       0x02010500
#define LightModuleOverCurrent       0x02010700
#define LightModuleOverCurrent01     0x02010701
#define LightModuleOverCurrent02     0x02010702
#define LightModuleOverCurrent03     0x02010703
#define LightModuleOverCurrent04     0x02010704
#define LightModuleOverCurrent05     0x02010705
#define LightModuleOverCurrent06     0x02010706
#define LightModuleOverCurrent07     0x02010707
#define LightModuleOverCurrent08     0x02010708
#define LightModuleOverCurrent09     0x02010709
#define LightModuleOverCurrent10     0x0201070A
#define LightModuleOverCurrent11     0x0201070B
#define LightModuleOverCurrent12     0x0201070C
#define LightModuleOverCurrent13     0x0201070D
#define LightModuleOverCurrent14     0x0201070E
#define LightModuleOverCurrent15     0x0201070F
#define LightModuleOverVoltage       0x02010900
#define LightModuleOverVoltage01     0x02010901
#define LightModuleOverVoltage02     0x02010902
#define LightModuleOverVoltage03     0x02010903
#define LightModuleOverVoltage04     0x02010904
#define LightModuleOverVoltage05     0x02010905
#define LightModuleOverVoltage06     0x02010906
#define LightModuleOverVoltage07     0x02010907
#define LightModuleOverVoltage08     0x02010908
#define LightModuleOverVoltage09     0x02010909
#define LightModuleOverVoltage10     0x0201090A
#define LightModuleOverVoltage11     0x0201090B
#define LightModuleOverVoltage12     0x0201090C
#define LightModuleOverVoltage13     0x0201090D
#define LightModuleOverVoltage14     0x0201090E
#define LightModuleOverVoltage15     0x0201090F
#define LightModuleShuntCurrent      0x02010B00
#define LightModuleShuntCurrent01    0x02010B01
#define LightModuleShuntCurrent02    0x02010B02
#define LightModuleShuntCurrent03    0x02010B03
#define LightModuleShuntCurrent04    0x02010B04
#define LightModuleShuntCurrent05    0x02010B05
#define LightModuleShuntCurrent06    0x02010B06
#define LightModuleShuntCurrent07    0x02010B07
#define LightModuleShuntCurrent08    0x02010B08
#define LightModuleShuntCurrent09    0x02010B09
#define LightModuleShuntCurrent10    0x02010B0A
#define LightModuleShuntCurrent11    0x02010B0B
#define LightModuleShuntCurrent12    0x02010B0C
#define LightModuleShuntCurrent13    0x02010B0D
#define LightModuleShuntCurrent14    0x02010B0E
#define LightModuleShuntCurrent15    0x02010B0F
#define LightModuleShuntCurrent16    0x02010B10
#define LightModuleShuntCurrent17    0x02010B11
#define LightModuleShuntCurrent18    0x02010B12
#define LightModuleShuntCurrent19    0x02010B13
#define LightModuleShuntCurrent20    0x02010B14
#define LightModuleShuntCurrent21    0x02010B15
#define LightModuleShuntCurrent22    0x02010B16
#define LightModuleShuntCurrent23    0x02010B17
#define LightModuleShuntCurrent24    0x02010B18
#define LightModuleShuntCurrent25    0x02010B19
#define LightModuleShuntCurrent26    0x02010B1A
#define LightModuleShuntCurrent27    0x02010B1B
#define LightModuleShuntCurrent28    0x02010B1C
#define LightModuleShuntCurrent29    0x02010B1D
#define LightModuleShuntCurrent30    0x02010B1E
#define LightModuleShuntCurrent31    0x02010B1F
#define LightModuleOCP               0x02010800
#define LightModuleOCP01             0x02010801
#define LightModuleOCP02             0x02010802
#define LightModuleOCP03             0x02010803
#define LightModuleOCP04             0x02010804
#define LightModuleOCP05             0x02010805
#define LightModuleOCP06             0x02010806
#define LightModuleOCP07             0x02010807
#define LightSensorError             0x02020600
#define ShutterFail                  0x02030200
#define AmbientOverTemp              0x03010300
#define AmbientHighTemp              0x03010400
#define SystemOverTemp               0x03020300
#define SystemHighTemp               0x03020400
#define LightModuleOverTemp          0x03030300
#define LightModuleOverTemp01        0x03030301
#define LightModuleOverTemp02        0x03030302
#define LightModuleOverTemp03        0x03030303
#define LightModuleOverTemp04        0x03030304
#define LightModuleOverTemp05        0x03030305
#define LightModuleOverTemp06        0x03030306
#define LightModuleOverTemp07        0x03030307
#define LightModuleOverTemp08        0x03030308
#define LightModuleOverTemp09        0x03030309
#define LightModuleOverTemp10        0x0303030A
#define LightModuleOverTemp11        0x0303030B
#define LightModuleOverTemp12        0x0303030C
#define LightModuleOverTemp13        0x0303030D
#define LightModuleOverTemp14        0x0303030E
#define LightModuleOverTemp15        0x0303030F
#define LightModuleWarnTemp          0x03030400
#define LightModuleWarnTemp01        0x03030401
#define LightModuleWarnTemp02        0x03030402
#define LightModuleWarnTemp03        0x03030403
#define LightModuleWarnTemp04        0x03030404
#define LightModuleWarnTemp05        0x03030405
#define LightModuleWarnTemp06        0x03030406
#define LightModuleWarnTemp07        0x03030407
#define LightModuleWarnTemp08        0x03030408
#define LightModuleWarnTemp09        0x03030409
#define LightModuleWarnTemp10        0x0303040A
#define LightModuleWarnTemp11        0x0303040B
#define LightModuleWarnTemp12        0x0303040C
#define LightModuleWarnTemp13        0x0303040D
#define LightModuleWarnTemp14        0x0303040E
#define LightModuleWarnTemp15        0x0303040F
#define FanCalibration               0x03040500
#define FanLock                      0x03040100
#define FanLock01                    0x03040101
#define FanLock02                    0x03040102
#define FanLock03                    0x03040103
#define FanLock04                    0x03040104
#define FanLock05                    0x03040105
#define FanLock06                    0x03040106
#define FanLock07                    0x03040107
#define FanLock08                    0x03040108
#define FanLock09                    0x03040109
#define FanLock10                    0x0304010A
#define FanLock11                    0x0304010B
#define FanLock12                    0x0304010C
#define FanLock13                    0x0304010D
#define FanLock14                    0x0304010E
#define FanLock15                    0x0304010F
#define FanLock16                    0x03040110
#define FanLock17                    0x03040111   //HICC2_Doulas_0013
#define FanStallError                0x03040200
#define FanStallError01              0x03040201
#define FanStallError02              0x03040202
#define FanStallError03              0x03040203
#define FanStallError04              0x03040204
#define FanStallError05              0x03040205
#define FanStallError06              0x03040206
#define FanStallError07              0x03040207
#define FanStallError08              0x03040208
#define FanStallError09              0x03040209
#define FanStallError10              0x0304020A
#define FanStallError11              0x0304020B
#define FanStallError12              0x0304020C
#define FanStallError13              0x0304020D
#define FanStallError14              0x0304020E
#define FanStallError15              0x0304020F
#define FanStallError16              0x03040210
#define FanStallError17              0x03040211   //HICC2_Doulas_0013
#define FanStallWarn                 0x03040600
#define FanStallWarn01               0x03040601
#define FanStallWarn02               0x03040602
#define FanStallWarn03               0x03040603
#define FanStallWarn04               0x03040604
#define FanStallWarn05               0x03040605
#define FanStallWarn06               0x03040606
#define FanStallWarn07               0x03040607
#define FanStallWarn08               0x03040608
#define FanStallWarn09               0x03040609
#define FanStallWarn10               0x0304060A
#define FanStallWarn11               0x0304060B
#define FanStallWarn12               0x0304060C
#define FanStallWarn13               0x0304060D
#define FanStallWarn14               0x0304060E
#define FanStallWarn15               0x0304060F
#define LCSError                     0x03050100
#define LCSError01                   0x03050101
#define LCSError02                   0x03050102
#define LCSError03                   0x03050103
#define LCSError04                   0x03050104
#define LCSStall                     0x03050200
#define LCSStall01                   0x03050201
#define LCSStall02                   0x03050202
#define LCSStall03                   0x03050203
#define LCSStall04                   0x03050204
#define DMDOverTemp                  0x03060300
#define DMDOverTemp01                0x03060301
#define DMDOverTemp02                0x03060400
#define DMDOverTemp03                0x03060401
#define TECOverTemp                  0x03070300
#define TECOverTemp01                0x03070301
#define TECOverTemp02                0x03070400
#define TECOverTemp03                0x03070401
#define TECAbnormal01                0x03070500
#define TECAbnormal02                0x03070501
#define TECAbnormal03                0x03070502
#define FMTENVIRAbnormal01           0x03070600
#define FMTENVIRAbnormal02           0x03070601
#define FMTENVIRAbnormal03           0x03070602
#define FMTENVIRAbnormal04 			 0x03070603
#define WheelStop                    0x04010100
#define WheelStop01                  0x04010101
#define WheelStop02                  0x04010102
#define WheelStop03                  0x04010103
#define WheelStop04                  0x04010104
#define WheelStop05                  0x04010105
#define WheelStop06                  0x04010106
#define LensShiftSwitchFail          0x04020200
#define InterLockSwitchFail          0x04030200
#define LidOpen                      0x04040300
#define DDPFail                      0x04050200
#define DDPFail01                    0x04050201 //HICC2_Steven_0006
#define DDPFail02                    0x04050202
#define DDPFail03                    0x04050203
#define DDPFail04                    0x04050204
#define DDPFail05                    0x04050205
#define DDPFail06                    0x04050206
#define DDPFail07                    0x04050207
#define DDPFail08                    0x04050208
#define DDPFail09                    0x04050209
#define DDPFail10                    0x0405020A
#define DDPFail11                    0x0405020B
#define DDPFail12                    0x0405020C
#define DDPFail13                    0x0405020D
#define DDPFail14                    0x0405020E
#define DDPFail15                    0x0405020F
#define DDPFail16                    0x04050210
#define DDPFail17                    0x04050211
#define DDPFail18                    0x04050212
#define DDPFail19                    0x04050213
#define UpgradeFail                  0x05010100
#define UpgradeFail01                0x05010101
#define UpgradeFail02                0x05010102
#define UpgradeFail03                0x05010103
#define UpgradeFail04                0x05010104
#define UpgradeFail05                0x05010105
#define UpgradeFail06                0x05010106
#define UpgradeFail07                0x05010107
#define UpgradeFail08                0x05010108
#define UpgradeFail09                0x05010109
#define UpgradeFail10                0x0501010A
#define UpgradeFail11                0x0501010B
#define UpgradeFail12                0x0501010C
#define UpgradeFail13                0x0501010D
#define UpgradeFail14                0x0501010E
#define UpgradeFail15                0x0501010F
#define MultimediaFail               0x05020100
#define WatchDogReset                0x05030200
#define WatchDogReset01              0x05030201
#define WatchDogReset02              0x05030202
#define WatchDogReset03              0x05030203
#define WatchDogReset04              0x05030204
#define AutoSystemShutdown           0x05040300
#define SleepShutdown                0x05050300
#define RTCLowBattery                0x06030100
#define RTCFail                      0x06030200
#define LANFailthenrestart           0x07000000
#define PowerOffCount                0xFF010100
#define PowerOnCount                 0xFF020100
#define RetryCoolingCount            0xFF030100
#define AnalogSourceLockFail         0xFF040400
#define AnalogSourceLockFail01       0xFF040401
#define AnalogSourceLockFail02       0xFF040402
#define AnalogSourceLockFail03       0xFF040403
#define AnalogSourceLockFail04       0xFF040404
#define AnalogSourceLockFail05       0xFF040405
#define AnalogSourceLockFail06       0xFF040406
#define AnalogSourceLockFail07       0xFF040407
#define DigitalSourceLockFail        0xFF050400
#define DigitalSourceLockFail01      0xFF050401
#define DigitalSourceLockFail02      0xFF050402
#define DigitalSourceLockFail03      0xFF050403
#define DigitalSourceLockFail04      0xFF050404
#define DigitalSourceLockFail05      0xFF050405
#define DigitalSourceLockFail06      0xFF050406
#define DigitalSourceLockFail07      0xFF050407
#define FogFilter                    0xFF060000
#define FogFilter01                  0xFF060001
#define FogFilter02                  0xFF060002
#define FogFilter03                  0xFF060003
#define FogFilter04                  0xFF060004
#define I2CError                     0xFF070200
#define I2CError01                   0xFF070201
#define I2CError02                   0xFF070202
#define I2CError03                   0xFF070203
#define I2CError04                   0xFF070204
#define I2CError05                   0xFF070205
#define I2CError06                   0xFF070206
#define I2CError07                   0xFF070207
#define I2CError08                   0xFF070208
#define I2CError09                   0xFF070209
#define I2CError10                   0xFF07020A
#define I2CError11                   0xFF07020B
#define I2CError12                   0xFF07020C
#define I2CError13                   0xFF07020D
#define I2CError14                   0xFF07020E
#define I2CError15                   0xFF07020F
#define I2CError16                   0xFF070210
#define I2CError17                   0xFF070211
#define I2CError18                   0xFF070212
#define I2CError19                   0xFF070213
#define I2CError20                   0xFF070214
#define I2CError21                   0xFF070215
#define I2CError22                   0xFF070216
#define I2CError23                   0xFF070217
#define I2CError24                   0xFF070218
#define I2CError25                   0xFF070219
#define I2CError26                   0xFF07021A
#define I2CError27                   0xFF07021B
#define I2CError28                   0xFF07021C
#define I2CError29                   0xFF07021D
#define I2CError30                   0xFF07021E
#define I2CError31                   0xFF07021F
#define I2CError32                   0xFF070220
#define UARTError                    0xFF080200
#define SPIError                     0xFF090200
#define NVRAMRecovery                0xFF0A0300
#define HDCPError                    0xFF0B0200
#define HDCPError01                  0xFF0B0201
#define HDCPError02                  0xFF0B0202
#define HDCPError03                  0xFF0B0203
#define HDCPError04                  0xFF0B0204
#define HDCPError05                  0xFF0B0205
#define HDCPError06                  0xFF0B0206
#define HDCPError07                  0xFF0B0207
#define FPGALockFail                 0xFF0C0400
#define FPGAPhaseLockFail            0xFF0C0500
#define I2Cwarn                      0xFF070600
#define I2Cwarn01                    0xFF070601
#define I2Cwarn02                    0xFF070602
#define I2Cwarn03                    0xFF070603
#define I2Cwarn04                    0xFF070604
#define I2Cwarn05                    0xFF070605
#define I2Cwarn06                    0xFF070606
#define I2Cwarn07                    0xFF070607
#define I2Cwarn08                    0xFF070608
#define I2Cwarn09                    0xFF070609
#define I2Cwarn10                    0xFF07060A
#define I2Cwarn11                    0xFF07060B
#define I2Cwarn12                    0xFF07060C
#define I2Cwarn13                    0xFF07060D
#define I2Cwarn14                    0xFF07060E
#define I2Cwarn15                    0xFF07060F
#define I2Cwarn16                    0xFF070610
#define I2Cwarn17                    0xFF070611
#define I2Cwarn18                    0xFF070612
#define I2Cwarn19                    0xFF070613
#define I2Cwarn20                    0xFF070614
#define I2Cwarn21                    0xFF070615
#define I2Cwarn22                    0xFF070616
#define I2Cwarn23                    0xFF070617
#define I2Cwarn24                    0xFF070618
#define I2Cwarn25                    0xFF070619
#define I2Cwarn26                    0xFF07061A
#define I2Cwarn27                    0xFF07061B
#define I2Cwarn28                    0xFF07061C
#define I2Cwarn29                    0xFF07061D
#define I2Cwarn30                    0xFF07061E
#define I2Cwarn31                    0xFF07061F
#define I2Cwarn32                    0xFF070620

#define GEC_NO_ERROR 0
#define GEC_ERROR 1
#define GEC_WARNING 2
#define GEC_REMINDER 3
typedef enum  //This enum is copy from xls file, do not edit directly
{
    eBurnInTime,
    eReset,
    eIPv4,
    eIPv6,
    eSourceInfo,
    eSourceInfo01,
    eSourceInfo02,
    eSourceInfo03,
    eSourceInfo04,
    eSourceInfo05,
    eSourceInfo06,
    eSourceInfo07,
    ePictureMode,
    ePictureMode01,
    ePictureMode02,
    ePictureMode03,
    ePictureMode04,
    ePictureMode05,
    ePictureMode06,
    ePictureMode07,
    ePictureMode08,
    eOrientationAngle,
    eHighAltitude,
    eLDSaturation,
    eLDSaturation01,
    eLDSaturation02,
    eLDSaturation03,
    eLDSaturation04,
    eLDSaturation05,
    eLDSaturation06,
    eLDSaturation07,
    eLDSaturation08,
    eNTP,
    eGPS,
    eSourceDetVGA,
    eSourceDetHDMI1,
    eSourceDetHDMI2,
    eSourceDetDVI,
    eSourceDetDP,
    eSourceDetHDBASET,
    eSourceDetSDI,
    eSourceDetSlot1,
    eSourceDetSlot2,
    eSourceLostVGA,
    eSourceLostHDMI1,
    eSourceLostHDMI2,
    eSourceLostDVI,
    eSourceLostDP,
    eSourceLostHDBASET,
    eSourceLostSDI,
    eSourceLostSlot1,
    eSourceLostSlot2,
    ePowerNotMatch,
    eACLost,
    eLVPSError,
    eVolt220VDetect,
    eVolt110VDetect,
    eVolt50VLost,
    eVolt12VLost,
    eVolt3p3VLost,
    eInterLockSwLost,   //HICC2_Doulas_0013
    eThermalSwLost,     //HICC2_Doulas_0013
    eLightModuleBroken,
    eLightModuleBroken01,
    eLightModuleBroken02,
    eLightModuleBroken03,
    eLightModuleBroken04,
    eLightModuleBroken05,
    eLightModuleBroken06,
    eLightModuleBroken07,
    eLightModuleBroken08,
    eLightModuleBroken09,
    eLightModuleBroken10,
    eLightModuleBroken11,
    eLightModuleBroken12,
    eLightModuleBroken13,
    eLightModuleBroken14,
    eLightModuleBroken15,
    eLightModuleCommunicationFail,
    eLightModuleIgnitionFail,
    eLightModuleLightUpFail,
    eLightModuleOverCurrent,
    eLightModuleOverCurrent01,
    eLightModuleOverCurrent02,
    eLightModuleOverCurrent03,
    eLightModuleOverCurrent04,
    eLightModuleOverCurrent05,
    eLightModuleOverCurrent06,
    eLightModuleOverCurrent07,
    eLightModuleOverCurrent08,
    eLightModuleOverCurrent09,
    eLightModuleOverCurrent10,
    eLightModuleOverCurrent11,
    eLightModuleOverCurrent12,
    eLightModuleOverCurrent13,
    eLightModuleOverCurrent14,
    eLightModuleOverCurrent15,
    eLightModuleOverVoltage,
    eLightModuleOverVoltage01,
    eLightModuleOverVoltage02,
    eLightModuleOverVoltage03,
    eLightModuleOverVoltage04,
    eLightModuleOverVoltage05,
    eLightModuleOverVoltage06,
    eLightModuleOverVoltage07,
    eLightModuleOverVoltage08,
    eLightModuleOverVoltage09,
    eLightModuleOverVoltage10,
    eLightModuleOverVoltage11,
    eLightModuleOverVoltage12,
    eLightModuleOverVoltage13,
    eLightModuleOverVoltage14,
    eLightModuleOverVoltage15,
    eLightModuleShuntCurrent,
    eLightModuleShuntCurrent01,
    eLightModuleShuntCurrent02,
    eLightModuleShuntCurrent03,
    eLightModuleShuntCurrent04,
    eLightModuleShuntCurrent05,
    eLightModuleShuntCurrent06,
    eLightModuleShuntCurrent07,
    eLightModuleShuntCurrent08,
    eLightModuleShuntCurrent09,
    eLightModuleShuntCurrent10,
    eLightModuleShuntCurrent11,
    eLightModuleShuntCurrent12,
    eLightModuleShuntCurrent13,
    eLightModuleShuntCurrent14,
    eLightModuleShuntCurrent15,
    eLightModuleShuntCurrent16,
    eLightModuleShuntCurrent17,
    eLightModuleShuntCurrent18,
    eLightModuleShuntCurrent19,
    eLightModuleShuntCurrent20,
    eLightModuleShuntCurrent21,
    eLightModuleShuntCurrent22,
    eLightModuleShuntCurrent23,
    eLightModuleShuntCurrent24,
    eLightModuleShuntCurrent25,
    eLightModuleShuntCurrent26,
    eLightModuleShuntCurrent27,
    eLightModuleShuntCurrent28,
    eLightModuleShuntCurrent29,
    eLightModuleShuntCurrent30,
    eLightModuleShuntCurrent31,
    eLightModuleOCP,
    eLightModuleOCP01,
    eLightModuleOCP02,
    eLightModuleOCP03,
    eLightModuleOCP04,
    eLightModuleOCP05,
    eLightModuleOCP06,
    eLightModuleOCP07,
    eLightSensorError,
    eShutterFail,
    eAmbientOverTemp,
    eAmbientHighTemp,
    eSystemOverTemp,
    eSystemHighTemp,
    eLightModuleOverTemp,
    eLightModuleOverTemp01,
    eLightModuleOverTemp02,
    eLightModuleOverTemp03,
    eLightModuleOverTemp04,
    eLightModuleOverTemp05,
    eLightModuleOverTemp06,
    eLightModuleOverTemp07,
    eLightModuleOverTemp08,
    eLightModuleOverTemp09,
    eLightModuleOverTemp10,
    eLightModuleOverTemp11,
    eLightModuleOverTemp12,
    eLightModuleOverTemp13,
    eLightModuleOverTemp14,
    eLightModuleOverTemp15,
    eLightModuleWarnTemp,
    eLightModuleWarnTemp01,
    eLightModuleWarnTemp02,
    eLightModuleWarnTemp03,
    eLightModuleWarnTemp04,
    eLightModuleWarnTemp05,
    eLightModuleWarnTemp06,
    eLightModuleWarnTemp07,
    eLightModuleWarnTemp08,
    eLightModuleWarnTemp09,
    eLightModuleWarnTemp10,
    eLightModuleWarnTemp11,
    eLightModuleWarnTemp12,
    eLightModuleWarnTemp13,
    eLightModuleWarnTemp14,
    eLightModuleWarnTemp15,
    eFanCalibration,
    eFanLock,
    eFanLock01,
    eFanLock02,
    eFanLock03,
    eFanLock04,
    eFanLock05,
    eFanLock06,
    eFanLock07,
    eFanLock08,
    eFanLock09,
    eFanLock10,
    eFanLock11,
    eFanLock12,
    eFanLock13,
    eFanLock14,
    eFanLock15,
    eFanLock16,
    eFanLock17,  //HICC2_Doulas_0013
    eFanStallError,
    eFanStallError01,
    eFanStallError02,
    eFanStallError03,
    eFanStallError04,
    eFanStallError05,
    eFanStallError06,
    eFanStallError07,
    eFanStallError08,
    eFanStallError09,
    eFanStallError10,
    eFanStallError11,
    eFanStallError12,
    eFanStallError13,
    eFanStallError14,
    eFanStallError15,
    eFanStallError16,  //HICC2_Doulas_0013
    eFanStallError17,  //HICC2_Doulas_0013
    eFanStallWarn,
    eFanStallWarn01,
    eFanStallWarn02,
    eFanStallWarn03,
    eFanStallWarn04,
    eFanStallWarn05,
    eFanStallWarn06,
    eFanStallWarn07,
    eFanStallWarn08,
    eFanStallWarn09,
    eFanStallWarn10,
    eFanStallWarn11,
    eFanStallWarn12,
    eFanStallWarn13,
    eFanStallWarn14,
    eFanStallWarn15,
    eLCSError,
    eLCSError01,
    eLCSError02,
    eLCSError03,
    eLCSError04,
    eLCSStall,
    eLCSStall01,
    eLCSStall02,
    eLCSStall03,
    eLCSStall04,
    eDMDOverTemp,
    eDMDOverTemp01,
    eDMDOverTemp02,
    eDMDOverTemp03,
    eTECOverTemp,
    eTECOverTemp01,
    eTECOverTemp02,
    eTECOverTemp03,
    eTECAbnormal01,
    eTECAbnormal02,
    eTECAbnormal03,
    eFMTENVIRAbnormal01,  //HICC2_Doulas_0013
    eFMTENVIRAbnormal02,  //HICC2_Doulas_0013
    eFMTENVIRAbnormal03,  //HICC2_Doulas_0013
    eFMTENVIRAbnormal04,  //HICC2_Doulas_0013
    eWheelStop,
    eWheelStop01,
    eWheelStop02,
    eWheelStop03,
    eWheelStop04,
    eWheelStop05,
    eWheelStop06,
    eLensShiftSwitchFail,
    eInterLockSwitchFail,
    eLidOpen,
    eDDPFail,
 	eDDPFail01,
 	eDDPFail02,
 	eDDPFail03,
 	eDDPFail04,
 	eDDPFail05,
 	eDDPFail06,
 	eDDPFail07,
 	eDDPFail08,
 	eDDPFail09,
 	eDDPFail10,
 	eDDPFail11,
 	eDDPFail12,
 	eDDPFail13,
 	eDDPFail14,
 	eDDPFail15,
 	eDDPFail16,
 	eDDPFail17,
 	eDDPFail18,
 	eDDPFail19,
    eUpgradeFail,
    eUpgradeFail01,
    eUpgradeFail02,
    eUpgradeFail03,
    eUpgradeFail04,
    eUpgradeFail05,
    eUpgradeFail06,
    eUpgradeFail07,
    eUpgradeFail08,
    eUpgradeFail09,
    eUpgradeFail10,
    eUpgradeFail11,
    eUpgradeFail12,
    eUpgradeFail13,
    eUpgradeFail14,
    eUpgradeFail15,
    eMultimediaFail,
    eWatchDogReset,
    eWatchDogReset01,
    eWatchDogReset02,
    eWatchDogReset03,
    eWatchDogReset04,
    eAutoSystemShutdown,
    eSleepShutdown,
    eRTCLowBattery,
    eRTCFail,
    eLANFailthenrestart,
    ePowerOffCount,
    ePowerOnCount,
    eRetryCoolingCount,
    eAnalogSourceLockFail,
    eAnalogSourceLockFail01,
    eAnalogSourceLockFail02,
    eAnalogSourceLockFail03,
    eAnalogSourceLockFail04,
    eAnalogSourceLockFail05,
    eAnalogSourceLockFail06,
    eAnalogSourceLockFail07,
    eDigitalSourceLockFail,
    eDigitalSourceLockFail01,
    eDigitalSourceLockFail02,
    eDigitalSourceLockFail03,
    eDigitalSourceLockFail04,
    eDigitalSourceLockFail05,
    eDigitalSourceLockFail06,
    eDigitalSourceLockFail07,
    eFogFilter,
    eFogFilter01,
    eFogFilter02,
    eFogFilter03,
    eFogFilter04,
    eI2CError,
    eI2CError01,
    eI2CError02,
    eI2CError03,
    eI2CError04,
    eI2CError05,
    eI2CError06,
    eI2CError07,
    eI2CError08,
    eI2CError09,
    eI2CError10,
    eI2CError11,
    eI2CError12,
    eI2CError13,
    eI2CError14,
    eI2CError15,
    eI2CError16,
    eI2CError17,
    eI2CError18,
    eI2CError19,
    eI2CError20,
    eI2CError21,
    eI2CError22,
    eI2CError23,
    eI2CError24,
    eI2CError25,
    eI2CError26,
    eI2CError27,
    eI2CError28,
    eI2CError29,
    eI2CError30,
    eI2CError31,
    eI2CError32,
    eUARTError,
    eSPIError,
    eNVRAMRecovery,
    eHDCPError,
    eHDCPError01,
    eHDCPError02,
    eHDCPError03,
    eHDCPError04,
    eHDCPError05,
    eHDCPError06,
    eHDCPError07,
    eFPGALockFail,
    eFPGAPhaseLockFail,
    eI2Cwarn,
    eI2Cwarn01,
    eI2Cwarn02,
    eI2Cwarn03,
    eI2Cwarn04,
    eI2Cwarn05,
    eI2Cwarn06,
    eI2Cwarn07,
    eI2Cwarn08,
    eI2Cwarn09,
    eI2Cwarn10,
    eI2Cwarn11,
    eI2Cwarn12,
    eI2Cwarn13,
    eI2Cwarn14,
    eI2Cwarn15,
    eI2Cwarn16,
    eI2Cwarn17,
    eI2Cwarn18,
    eI2Cwarn19,
    eI2Cwarn20,
    eI2Cwarn21,
    eI2Cwarn22,
    eI2Cwarn23,
    eI2Cwarn24,
    eI2Cwarn25,
    eI2Cwarn26,
    eI2Cwarn27,
    eI2Cwarn28,
    eI2Cwarn29,
    eI2Cwarn30,
    eI2Cwarn31,
    eI2Cwarn32,

    eERROR_LIST_INVALID//Last for nothing
}eERROR_LIST;
    
typedef enum
{
    eHEX,
    eASCII
}eGEC_DATATYPE;


typedef struct gec_errorname
{
    UINT32 uiErrorIndex;
    eERROR_LIST eErrorList;
    char *ERROREVENT;               //Error Type in Generic error code
    char *CustomizeEvent;           //Error Type in Customize error code  //A70LV_John_0046 add customized error code name
    eGEC_DATATYPE fgAuxForm;        //Error Body Part M
    UINT8 ucWriteCheck;
}sGEC_ERRORTABLE;

//This table is copy from generated file, do not edit directly.
static const sGEC_ERRORTABLE sGEC_ERRORCODE_TABLE[]=
{
    {BurnInTime,                   eBurnInTime,                   "",                                "", eHEX,   0},
    {Reset,                        eReset,                        "",                                "", eHEX,   0},
    {IPv4,                         eIPv4,                         "",                                "", eHEX,   0},
    {IPv6,                         eIPv6,                         "",                                "", eHEX,   0},
    {SourceInfo,                   eSourceInfo,                   "",                                "", eHEX,   0},
    {SourceInfo01,                 eSourceInfo01,                 "",                                "", eHEX,   0},
    {SourceInfo02,                 eSourceInfo02,                 "",                                "", eHEX,   0},
    {SourceInfo03,                 eSourceInfo03,                 "",                                "", eHEX,   0},
    {SourceInfo04,                 eSourceInfo04,                 "",                                "", eHEX,   0},
    {SourceInfo05,                 eSourceInfo05,                 "",                                "", eHEX,   0},
    {SourceInfo06,                 eSourceInfo06,                 "",                                "", eHEX,   0},
    {SourceInfo07,                 eSourceInfo07,                 "",                                "", eHEX,   0},
    {PictureMode,                  ePictureMode,                  "",                                "", eHEX,   0},
    {PictureMode01,                ePictureMode01,                "",                                "", eHEX,   0},
    {PictureMode02,                ePictureMode02,                "",                                "", eHEX,   0},
    {PictureMode03,                ePictureMode03,                "",                                "", eHEX,   0},
    {PictureMode04,                ePictureMode04,                "",                                "", eHEX,   0},
    {PictureMode05,                ePictureMode05,                "",                                "", eHEX,   0},
    {PictureMode06,                ePictureMode06,                "",                                "", eHEX,   0},
    {PictureMode07,                ePictureMode07,                "",                                "", eHEX,   0},
    {PictureMode08,                ePictureMode08,                "",                                "", eHEX,   0},
    {OrientationAngle,             eOrientationAngle,             "",                                "", eHEX,   0},
    {HighAltitude,                 eHighAltitude,                 "",                                "", eHEX,   0},
    {LDSaturation,                 eLDSaturation,                 "",                                "", eHEX,   0},
    {LDSaturation01,               eLDSaturation01,               "",                                "", eHEX,   0},
    {LDSaturation02,               eLDSaturation02,               "",                                "", eHEX,   0},
    {LDSaturation03,               eLDSaturation03,               "",                                "", eHEX,   0},
    {LDSaturation04,               eLDSaturation04,               "",                                "", eHEX,   0},
    {LDSaturation05,               eLDSaturation05,               "",                                "", eHEX,   0},
    {LDSaturation06,               eLDSaturation06,               "",                                "", eHEX,   0},
    {LDSaturation07,               eLDSaturation07,               "",                                "", eHEX,   0},
    {LDSaturation08,               eLDSaturation08,               "",                                "", eHEX,   0},
    {NTP_GEC,                      eNTP,                          "",                                "", eHEX,   0},
    {GPS_GEC,                      eGPS,                          "",                                "", eHEX,   0},
    {SourceDetVGA,                 eSourceDetVGA,                 "VGA Detected",                    "", eHEX,   GEC_REMINDER},
    {SourceDetHDMI1,               eSourceDetHDMI1,               "HDMI 1 Detected",                 "", eHEX,   GEC_REMINDER},
    {SourceDetHDMI2,               eSourceDetHDMI2,               "HDMI 2 Detected",                 "", eHEX,   GEC_REMINDER},
    {SourceDetDVI,                 eSourceDetDVI,                 "DVI Detected",                    "", eHEX,   GEC_REMINDER},
    {SourceDetDP,                  eSourceDetDP,                  "DP Detected",                     "", eHEX,   GEC_REMINDER},
    {SourceDetHDBASET,             eSourceDetHDBASET,             "HDBASET Detected",                "", eHEX,   GEC_REMINDER},
    {SourceDetSDI,                 eSourceDetSDI,                 "3GSDI  Detected",                 "", eHEX,   GEC_REMINDER},
    {SourceDetSlot1,               eSourceDetSlot1,               "Slot 1 Detected",                 "", eHEX,   GEC_REMINDER},
    {SourceDetSlot2,               eSourceDetSlot2,               "Slot 2 Detected",                 "", eHEX,   GEC_REMINDER},
    {SourceLostVGA,                eSourceLostVGA,                "VGA Lost",                        "", eHEX,   GEC_REMINDER},
    {SourceLostHDMI1,              eSourceLostHDMI1,              "HDMI 1 Lost",                     "", eHEX,   GEC_REMINDER},
    {SourceLostHDMI2,              eSourceLostHDMI2,              "HDMI 2 Lost",                     "", eHEX,   GEC_REMINDER},
    {SourceLostDVI,                eSourceLostDVI,                "DVI Lost",                        "", eHEX,   GEC_REMINDER},
    {SourceLostDP,                 eSourceLostDP,                 "DP Lost",                         "", eHEX,   GEC_REMINDER},
    {SourceLostHDBASET,            eSourceLostHDBASET,            "HDBASET Lost",                    "", eHEX,   GEC_REMINDER},
    {SourceLostSDI,                eSourceLostSDI,                "3GSDI  Lost",                     "", eHEX,   GEC_REMINDER},
    {SourceLostSlot1,              eSourceLostSlot1,              "Slot 1 Lost",                     "", eHEX,   GEC_REMINDER},
    {SourceLostSlot2,              eSourceLostSlot2,              "Slot 2 Lost",                     "", eHEX,   GEC_REMINDER},
    {PowerNotMatch,                ePowerNotMatch,                "Power Not Match",                 "", eHEX,   GEC_ERROR},
    {ACLost,                       eACLost,                       "AC Lost",                         "", eHEX,   GEC_ERROR},
    {LVPSError,                    eLVPSError,                    "LVPS Not Ready",                  "", eHEX,   GEC_ERROR},
    {Volt220VDetect,               eVolt220VDetect,               "Volt 220V Detect",                "", eHEX,   GEC_ERROR},
    {Volt110VDetect,               eVolt110VDetect,               "Volt 110V Detect",                "", eHEX,   GEC_ERROR},
    {Volt50VLost,                  eVolt50VLost,                  "Volt 50V Lost",                   "", eHEX,   GEC_ERROR},
    {Volt12VLost,                  eVolt12VLost,                  "Volt 12V Lost",                   "", eHEX,   GEC_ERROR},
    {Volt3p3VLost,                 eVolt3p3VLost,                 "Volt 3p3V Lost",                  "", eHEX,   GEC_ERROR},
    {InterLockSwLost,              eInterLockSwLost,              "Interlock Switch Open",           "", eHEX,   GEC_ERROR}, //HICC2_Doulas_0013
    {ThermalSwLost,                eThermalSwLost,                "Thermal Switch Open",             "", eHEX,   GEC_ERROR}, //HICC2_Doulas_0013
    {LightModuleBroken,            eLightModuleBroken,            "Light Module Broken",             "", eHEX,   GEC_ERROR},
    {LightModuleBroken01,          eLightModuleBroken01,          "Light Module Broken 01",          "", eHEX,   GEC_ERROR},
    {LightModuleBroken02,          eLightModuleBroken02,          "Light Module Broken 02",          "", eHEX,   GEC_ERROR},
    {LightModuleBroken03,          eLightModuleBroken03,          "Light Module Broken 03",          "", eHEX,   GEC_ERROR},
    {LightModuleBroken04,          eLightModuleBroken04,          "Light Module Broken 04",          "", eHEX,   GEC_ERROR},
    {LightModuleBroken05,          eLightModuleBroken05,          "LD 1 Unexpected off",             "", eHEX,   GEC_ERROR},
    {LightModuleBroken06,          eLightModuleBroken06,          "LD 2 Unexpected off",             "", eHEX,   GEC_ERROR},
    {LightModuleBroken07,          eLightModuleBroken07,          "LD 3 Unexpected off",             "", eHEX,   GEC_ERROR},
    {LightModuleBroken08,          eLightModuleBroken08,          "LD 4 Unexpected off",             "", eHEX,   GEC_ERROR},
    {LightModuleBroken09,          eLightModuleBroken09,          "",                                "", eHEX,   0},
    {LightModuleBroken10,          eLightModuleBroken10,          "",                                "", eHEX,   0},
    {LightModuleBroken11,          eLightModuleBroken11,          "",                                "", eHEX,   0},
    {LightModuleBroken12,          eLightModuleBroken12,          "",                                "", eHEX,   0},
    {LightModuleBroken13,          eLightModuleBroken13,          "",                                "", eHEX,   0},
    {LightModuleBroken14,          eLightModuleBroken14,          "",                                "", eHEX,   0},
    {LightModuleBroken15,          eLightModuleBroken15,          "",                                "", eHEX,   0},
    {LightModuleCommunicationFail, eLightModuleCommunicationFail, "Light Module Communication Fail", "", eHEX,   GEC_ERROR},
    {LightModuleIgnitionFail,      eLightModuleIgnitionFail,      "Light Module Ignition Fail",      "", eHEX,   GEC_ERROR},
    {LightModuleLightUpFail,       eLightModuleLightUpFail,       "Light Module Light Up Fail",      "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent,       eLightModuleOverCurrent,       "Light Module Over Current",       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent01,     eLightModuleOverCurrent01,     "LD OCP 01",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent02,     eLightModuleOverCurrent02,     "LD OCP 02",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent03,     eLightModuleOverCurrent03,     "LD OCP 03",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent04,     eLightModuleOverCurrent04,     "LD OCP 04",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent05,     eLightModuleOverCurrent05,     "LD OCP 05",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent06,     eLightModuleOverCurrent06,     "LD OCP 06",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent07,     eLightModuleOverCurrent07,     "LD OCP 07",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent08,     eLightModuleOverCurrent08,     "LD OCP 08",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent09,     eLightModuleOverCurrent09,     "LD OCP 09",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent10,     eLightModuleOverCurrent10,     "LD OCP 10",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent11,     eLightModuleOverCurrent11,     "LD OCP 11",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent12,     eLightModuleOverCurrent12,     "LD OCP 12",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent13,     eLightModuleOverCurrent13,     "LD OCP 13",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent14,     eLightModuleOverCurrent14,     "LD OCP 14",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverCurrent15,     eLightModuleOverCurrent15,     "LD OCP 15",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage,       eLightModuleOverVoltage,       "Light Module Over Voltage",       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage01,     eLightModuleOverVoltage01,     "LD OVP 01",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage02,     eLightModuleOverVoltage02,     "LD OVP 02",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage03,     eLightModuleOverVoltage03,     "LD OVP 03",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage04,     eLightModuleOverVoltage04,     "LD OVP 04",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage05,     eLightModuleOverVoltage05,     "LD OVP 05",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage06,     eLightModuleOverVoltage06,     "LD OVP 06",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage07,     eLightModuleOverVoltage07,     "LD OVP 07",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage08,     eLightModuleOverVoltage08,     "LD OVP 08",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage09,     eLightModuleOverVoltage09,     "LD OVP 09",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage10,     eLightModuleOverVoltage10,     "LD OVP 10",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage11,     eLightModuleOverVoltage11,     "LD OVP 11",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage12,     eLightModuleOverVoltage12,     "LD OVP 12",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage13,     eLightModuleOverVoltage13,     "LD OVP 13",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage14,     eLightModuleOverVoltage14,     "LD OVP 14",                       "", eHEX,   GEC_ERROR},
    {LightModuleOverVoltage15,     eLightModuleOverVoltage15,     "LD OVP 15",                       "", eHEX,   GEC_ERROR},
    {LightModuleShuntCurrent,      eLightModuleShuntCurrent,      "Light Module Shunt Current",      "", eHEX,   GEC_ERROR},
    {LightModuleShuntCurrent01,    eLightModuleShuntCurrent01,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent02,    eLightModuleShuntCurrent02,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent03,    eLightModuleShuntCurrent03,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent04,    eLightModuleShuntCurrent04,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent05,    eLightModuleShuntCurrent05,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent06,    eLightModuleShuntCurrent06,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent07,    eLightModuleShuntCurrent07,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent08,    eLightModuleShuntCurrent08,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent09,    eLightModuleShuntCurrent09,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent10,    eLightModuleShuntCurrent10,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent11,    eLightModuleShuntCurrent11,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent12,    eLightModuleShuntCurrent12,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent13,    eLightModuleShuntCurrent13,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent14,    eLightModuleShuntCurrent14,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent15,    eLightModuleShuntCurrent15,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent16,    eLightModuleShuntCurrent16,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent17,    eLightModuleShuntCurrent17,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent18,    eLightModuleShuntCurrent18,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent19,    eLightModuleShuntCurrent19,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent20,    eLightModuleShuntCurrent20,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent21,    eLightModuleShuntCurrent21,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent22,    eLightModuleShuntCurrent22,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent23,    eLightModuleShuntCurrent23,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent24,    eLightModuleShuntCurrent24,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent25,    eLightModuleShuntCurrent25,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent26,    eLightModuleShuntCurrent26,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent27,    eLightModuleShuntCurrent27,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent28,    eLightModuleShuntCurrent28,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent29,    eLightModuleShuntCurrent29,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent30,    eLightModuleShuntCurrent30,    "",                                "", eHEX,   0},
    {LightModuleShuntCurrent31,    eLightModuleShuntCurrent31,    "",                                "", eHEX,   0},
    {LightModuleOCP,               eLightModuleOCP,               "Light Module OCP",                "", eHEX,   GEC_ERROR},
    {LightModuleOCP01,             eLightModuleOCP01,             "BLD1 OCP",                        "", eHEX,   GEC_ERROR},
    {LightModuleOCP02,             eLightModuleOCP02,             "BLD2 OCP",                        "", eHEX,   GEC_ERROR},
    {LightModuleOCP03,             eLightModuleOCP03,             "RLD OCP",                         "", eHEX,   GEC_ERROR},
    {LightModuleOCP04,             eLightModuleOCP04,             "Light Module OCP 04",             "", eHEX,   GEC_ERROR},
    {LightModuleOCP05,             eLightModuleOCP05,             "Light Module OCP 05",             "", eHEX,   GEC_ERROR},
    {LightModuleOCP06,             eLightModuleOCP06,             "Light Module OCP 06",             "", eHEX,   GEC_ERROR},
    {LightModuleOCP07,             eLightModuleOCP07,             "Light Module OCP 07",             "", eHEX,   GEC_ERROR},
    {LightSensorError,             eLightSensorError,             "Light Sensor Error",              "", eHEX,   GEC_ERROR},
    {ShutterFail,                  eShutterFail,                  "Shutter Fail",                    "", eHEX,   GEC_ERROR},
    {AmbientOverTemp,              eAmbientOverTemp,              "Ambient Over Temp",               "", eHEX,   GEC_ERROR},
    {AmbientHighTemp,              eAmbientHighTemp,              "Ambient High Temp",               "", eHEX,   GEC_WARNING},
    {SystemOverTemp,               eSystemOverTemp,               "System Over Temp",                "", eHEX,   GEC_ERROR},
    {SystemHighTemp,               eSystemHighTemp,               "System High Temp",                "", eHEX,   GEC_WARNING},
    {LightModuleOverTemp,          eLightModuleOverTemp,          "BLD OverTemp shutdown",           "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp01,        eLightModuleOverTemp01,        "LD Bank Over Temp 01",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp02,        eLightModuleOverTemp02,        "LD Bank Over Temp 02",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp03,        eLightModuleOverTemp03,        "LD Bank Over Temp 03",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp04,        eLightModuleOverTemp04,        "LD Bank Over Temp 04",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp05,        eLightModuleOverTemp05,        "LD Bank Over Temp 05",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp06,        eLightModuleOverTemp06,        "LD Bank Over Temp 06",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp07,        eLightModuleOverTemp07,        "LD Bank Over Temp 07",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp08,        eLightModuleOverTemp08,        "LD Bank Over Temp 08",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp09,        eLightModuleOverTemp09,        "LD Bank Over Temp 09",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp10,        eLightModuleOverTemp10,        "LD Bank Over Temp 10",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp11,        eLightModuleOverTemp11,        "LD Bank Over Temp 11",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp12,        eLightModuleOverTemp12,        "LD Bank Over Temp 12",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp13,        eLightModuleOverTemp13,        "LD Bank Over Temp 13",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp14,        eLightModuleOverTemp14,        "LD Bank Over Temp 14",            "", eHEX,   GEC_ERROR},
    {LightModuleOverTemp15,        eLightModuleOverTemp15,        "LD Bank Over Temp 15",            "", eHEX,   GEC_ERROR},
    {LightModuleWarnTemp,          eLightModuleWarnTemp,          "Light Module Warn Temp",          "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp01,        eLightModuleWarnTemp01,        "LD Bank Warn Temp 01",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp02,        eLightModuleWarnTemp02,        "LD Bank Warn Temp 02",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp03,        eLightModuleWarnTemp03,        "LD Bank Warn Temp 03",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp04,        eLightModuleWarnTemp04,        "LD Bank Warn Temp 04",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp05,        eLightModuleWarnTemp05,        "LD Bank Warn Temp 05",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp06,        eLightModuleWarnTemp06,        "LD Bank Warn Temp 06",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp07,        eLightModuleWarnTemp07,        "LD Bank Warn Temp 07",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp08,        eLightModuleWarnTemp08,        "LD Bank Warn Temp 08",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp09,        eLightModuleWarnTemp09,        "LD Bank Warn Temp 09",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp10,        eLightModuleWarnTemp10,        "LD Bank Warn Temp 10",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp11,        eLightModuleWarnTemp11,        "LD Bank Warn Temp 11",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp12,        eLightModuleWarnTemp12,        "LD Bank Warn Temp 12",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp13,        eLightModuleWarnTemp13,        "LD Bank Warn Temp 13",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp14,        eLightModuleWarnTemp14,        "LD Bank Warn Temp 14",            "", eHEX,   GEC_WARNING},
    {LightModuleWarnTemp15,        eLightModuleWarnTemp15,        "LD Bank Warn Temp 15",            "", eHEX,   GEC_WARNING},
    {FanCalibration,               eFanCalibration,               "Fan Calibration",                 "", eHEX,   GEC_ERROR},
    {FanLock,                      eFanLock,                      "Fan Lock",                        "", eHEX,   GEC_ERROR},
    {FanLock01,                    eFanLock01,                    "Fan Lock 01",                     "", eHEX,   GEC_ERROR},
    {FanLock02,                    eFanLock02,                    "Fan Lock 02",                     "", eHEX,   GEC_ERROR},
    {FanLock03,                    eFanLock03,                    "Fan Lock 03",                     "", eHEX,   GEC_ERROR},
    {FanLock04,                    eFanLock04,                    "Fan Lock 04",                     "", eHEX,   GEC_ERROR},
    {FanLock05,                    eFanLock05,                    "Fan Lock 05",                     "", eHEX,   GEC_ERROR},
    {FanLock06,                    eFanLock06,                    "Fan Lock 06",                     "", eHEX,   GEC_ERROR},
    {FanLock07,                    eFanLock07,                    "Fan Lock 07",                     "", eHEX,   GEC_ERROR},
    {FanLock08,                    eFanLock08,                    "Fan Lock 08",                     "", eHEX,   GEC_ERROR},
    {FanLock09,                    eFanLock09,                    "Fan Lock 09",                     "", eHEX,   GEC_ERROR},
    {FanLock10,                    eFanLock10,                    "Fan Lock 10",                     "", eHEX,   GEC_ERROR},
    {FanLock11,                    eFanLock11,                    "Fan Lock 11",                     "", eHEX,   GEC_ERROR},
    {FanLock12,                    eFanLock12,                    "Fan Lock 12",                     "", eHEX,   GEC_ERROR},
    {FanLock13,                    eFanLock13,                    "Fan Lock 13",                     "", eHEX,   GEC_ERROR},
    {FanLock14,                    eFanLock14,                    "Fan Lock 14",                     "", eHEX,   GEC_ERROR},
    {FanLock15,                    eFanLock15,                    "Fan Lock 15",                     "", eHEX,   GEC_ERROR},
    {FanLock16,                    eFanLock16,                    "Fan Lock 16",                     "", eHEX,   GEC_ERROR}, //HICC2_Doulas_0013
    {FanLock17,                    eFanLock17,                    "Fan Lock 17",                     "", eHEX,   GEC_ERROR}, //HICC2_Doulas_0013
    {FanStallError,                eFanStallError,                "Fan Stall Error",                 "", eHEX,   GEC_ERROR},
    {FanStallError01,              eFanStallError01,              "Fan Error 01",                    "", eHEX,   GEC_ERROR},
    {FanStallError02,              eFanStallError02,              "Fan Error 02",                    "", eHEX,   GEC_ERROR},
    {FanStallError03,              eFanStallError03,              "Fan Error 03",                    "", eHEX,   GEC_ERROR},
    {FanStallError04,              eFanStallError04,              "Fan Error 04",                    "", eHEX,   GEC_ERROR},
    {FanStallError05,              eFanStallError05,              "Fan Error 05",                    "", eHEX,   GEC_ERROR},
    {FanStallError06,              eFanStallError06,              "Fan Error 06",                    "", eHEX,   GEC_ERROR},
    {FanStallError07,              eFanStallError07,              "Fan Error 07",                    "", eHEX,   GEC_ERROR},
    {FanStallError08,              eFanStallError08,              "Fan Error 08",                    "", eHEX,   GEC_ERROR},
    {FanStallError09,              eFanStallError09,              "Fan Error 09",                    "", eHEX,   GEC_ERROR},
    {FanStallError10,              eFanStallError10,              "Fan Error 10",                    "", eHEX,   GEC_ERROR},
    {FanStallError11,              eFanStallError11,              "Fan Error 11",                    "", eHEX,   GEC_ERROR},
    {FanStallError12,              eFanStallError12,              "Fan Error 12",                    "", eHEX,   GEC_ERROR},
    {FanStallError13,              eFanStallError13,              "Fan Error 13",                    "", eHEX,   GEC_ERROR},
    {FanStallError14,              eFanStallError14,              "Fan Error 14",                    "", eHEX,   GEC_ERROR},
    {FanStallError15,              eFanStallError15,              "Fan Error 15",                    "", eHEX,   GEC_ERROR},
    {FanStallError16,              eFanStallError16,              "Fan Error 16",                    "", eHEX,   GEC_ERROR}, //HICC2_Doulas_0013
    {FanStallError17,              eFanStallError17,              "Fan Error 17",                    "", eHEX,   GEC_ERROR}, //HICC2_Doulas_0013
    {FanStallWarn,                 eFanStallWarn,                 "Fan Stall Warn",                  "", eHEX,   GEC_WARNING},
    {FanStallWarn01,               eFanStallWarn01,               "Fan Warn 01",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn02,               eFanStallWarn02,               "Fan Warn 02",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn03,               eFanStallWarn03,               "Fan Warn 03",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn04,               eFanStallWarn04,               "Fan Warn 04",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn05,               eFanStallWarn05,               "Fan Warn 05",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn06,               eFanStallWarn06,               "Fan Warn 06",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn07,               eFanStallWarn07,               "Fan Warn 07",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn08,               eFanStallWarn08,               "Fan Warn 08",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn09,               eFanStallWarn09,               "Fan Warn 09",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn10,               eFanStallWarn10,               "Fan Warn 10",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn11,               eFanStallWarn11,               "Fan Warn 11",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn12,               eFanStallWarn12,               "Fan Warn 12",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn13,               eFanStallWarn13,               "Fan Warn 13",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn14,               eFanStallWarn14,               "Fan Warn 14",                     "", eHEX,   GEC_WARNING},
    {FanStallWarn15,               eFanStallWarn15,               "Fan Warn 15",                     "", eHEX,   GEC_WARNING},
    {LCSError,                     eLCSError,                     "",                                "", eHEX,   0},
    {LCSError01,                   eLCSError01,                   "Pump Lock 01",                    "", eHEX,   GEC_ERROR},
    {LCSError02,                   eLCSError02,                   "Pump Lock 02",                    "", eHEX,   GEC_ERROR},
    {LCSError03,                   eLCSError03,                   "",                                "", eHEX,   0},
    {LCSError04,                   eLCSError04,                   "",                                "", eHEX,   0},
    {LCSStall,                     eLCSStall,                     "",                                "", eHEX,   0},
    {LCSStall01,                   eLCSStall01,                   "Pump Error 01",                   "", eHEX,   GEC_ERROR},
    {LCSStall02,                   eLCSStall02,                   "",                                "", eHEX,   0},
    {LCSStall03,                   eLCSStall03,                   "Pump Warn 01",                    "", eHEX,   GEC_WARNING},
    {LCSStall04,                   eLCSStall04,                   "",                                "", eHEX,   0},
    {DMDOverTemp,                  eDMDOverTemp,                  "DMD Over Temp 01",                "", eHEX,   GEC_ERROR},
    {DMDOverTemp01,                eDMDOverTemp01,                "DMD Over Temp 02",                "", eHEX,   GEC_ERROR},
    {DMDOverTemp02,                eDMDOverTemp02,                "DMD High Temp 01",                "", eHEX,   GEC_WARNING},
    {DMDOverTemp03,                eDMDOverTemp03,                "DMD High Temp 02",                "", eHEX,   GEC_WARNING},
    {TECOverTemp,                  eTECOverTemp,                  "TEC Over Temp",                   "", eHEX,   GEC_ERROR},
    {TECOverTemp01,                eTECOverTemp01,                "RLD Over Temp",                   "", eHEX,   GEC_ERROR},
    {TECOverTemp02,                eTECOverTemp02,                "RLD HIGH Temp",                   "", eHEX,   GEC_WARNING},
    {TECOverTemp03,                eTECOverTemp03,                "BLD HIGH Temp",                   "", eHEX,   GEC_WARNING},
    {TECAbnormal01,                eTECAbnormal01,                "TEC Abnormal 01",                 "", eHEX,   GEC_ERROR},
    {TECAbnormal02,                eTECAbnormal02,                "TEC Abnormal 02",                 "", eHEX,   GEC_ERROR},
    {TECAbnormal03,                eTECAbnormal03,                "TEC Abnormal 03",                 "", eHEX,   GEC_ERROR},
    {FMTENVIRAbnormal01,           eFMTENVIRAbnormal01,           "HIGH AMBIENT WARNING",             "",eHEX,   GEC_WARNING}, //HICC2_Doulas_0013
    {FMTENVIRAbnormal02,           eFMTENVIRAbnormal02,           "HIGH ALTITUDE WARNING",            "",eHEX,   GEC_WARNING}, //HICC2_Doulas_0013
    {FMTENVIRAbnormal03,           eFMTENVIRAbnormal03,           "HIGH HUMIDITY WARNING",            "",eHEX,   GEC_WARNING}, //HICC2_Doulas_0013
    {FMTENVIRAbnormal04,           eFMTENVIRAbnormal04,           "HIGH AMBIENT AND ALTITUDE WARNING","",eHEX,   GEC_WARNING}, //HICC2_Doulas_0013
    {WheelStop,                    eWheelStop,                    "Wheel Stop",                      "", eHEX,   GEC_ERROR},
    {WheelStop01,                  eWheelStop01,                  "Phosphor Wheel Stop",             "", eHEX,   GEC_ERROR},
    {WheelStop02,                  eWheelStop02,                  "Filter Wheel Stop",               "", eHEX,   GEC_ERROR},
    {WheelStop03,                  eWheelStop03,                  "",                                "", eHEX,   0},
    {WheelStop04,                  eWheelStop04,                  "",                                "", eHEX,   0},
    {WheelStop05,                  eWheelStop05,                  "",                                "", eHEX,   0},
    {WheelStop06,                  eWheelStop06,                  "",                                "", eHEX,   0},
    {LensShiftSwitchFail,          eLensShiftSwitchFail,          "Lens Shift Switch Fail",          "", eHEX,   GEC_ERROR},
    {InterLockSwitchFail,          eInterLockSwitchFail,          "Inter Lock Switch Fail",          "", eHEX,   GEC_ERROR},
    {LidOpen,                      eLidOpen,                      "Lid Open",                        "", eHEX,   GEC_ERROR},
    {DDPFail,                      eDDPFail,                      "DDP Asic Fail",                   "", eHEX,   GEC_ERROR},
	{DDPFail01,                    eDDPFail01,                    "DDP LLFAULT FLASH TABLE",         "", eHEX,   GEC_ERROR},
	{DDPFail02,                    eDDPFail02,                    "DDP LLFAULT MEM INIT",            "", eHEX,   GEC_ERROR},
	{DDPFail03,                    eDDPFail03,                    "DDP LLFAULT ARM INIT",            "", eHEX,   GEC_ERROR},
	{DDPFail04,                    eDDPFail04,                    "DDP LLFAULT MEM POOL",            "", eHEX,   GEC_ERROR},
	{DDPFail05,                    eDDPFail05,                    "DDP LLFAULT INIT TASK",           "", eHEX,   GEC_ERROR},
	{DDPFail06,                    eDDPFail06,                    "DDP LLFAULT ASIC CFG",            "", eHEX,   GEC_ERROR},
	{DDPFail07,                    eDDPFail07,                    "DDP LLFAULT SEQ CFG",             "", eHEX,   GEC_ERROR},
	{DDPFail08,                    eDDPFail08,                    "DDP LLFAULT SYS CONFIG",          "", eHEX,   GEC_ERROR},
	{DDPFail09,                    eDDPFail09,                    "DDP LLFAULT SYS FATAL INIT",      "", eHEX,   GEC_ERROR},
	{DDPFail10,                    eDDPFail10,                    "DDP LLFAULT SYS INIT",            "", eHEX,   GEC_ERROR},
	{DDPFail11,                    eDDPFail11,                    "DDP LLFAULT SYS ASIC",            "", eHEX,   GEC_ERROR},
	{DDPFail12,                    eDDPFail12,                    "DDP LLFAULT SYS I2C ",            "", eHEX,   GEC_ERROR},
	{DDPFail13,                    eDDPFail13,                    "DDP LLFAULT SYS CW SEQ",          "", eHEX,   GEC_ERROR},
	{DDPFail14,                    eDDPFail14,                    "DDP LLFAULT SYS DMD",             "", eHEX,   GEC_ERROR},
	{DDPFail15,                    eDDPFail15,                    "DDP LLFAULT SYS DMD FATAL",       "", eHEX,   GEC_ERROR},
	{DDPFail16,                    eDDPFail16,                    "DDP LLFAULT SYS ULPM",            "", eHEX,   GEC_ERROR},
	{DDPFail17,                    eDDPFail17,                    "DDP LLFAULT EX DATA",             "", eHEX,   GEC_ERROR},
	{DDPFail18,                    eDDPFail18,                    "DDP LLFAULT EX PREFETCH",         "", eHEX,   GEC_ERROR},
	{DDPFail19,                    eDDPFail19,                    "DDP LLFAULT EX INSTRUCT",         "", eHEX,   GEC_ERROR},
	{UpgradeFail,                  eUpgradeFail,                  "Upgrade Fail",                    "", eHEX,   GEC_ERROR},
    {UpgradeFail01,                eUpgradeFail01,                "A-MCU Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail02,                eUpgradeFail02,                "M-MCU Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail03,                eUpgradeFail03,                "L-MCU Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail04,                eUpgradeFail04,                "K-MCU Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail05,                eUpgradeFail05,                "F-MCU Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail06,                eUpgradeFail06,                "Formatter Upgrade Fail",          "", eHEX,   GEC_ERROR},
    {UpgradeFail07,                eUpgradeFail07,                "MStar Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail08,                eUpgradeFail08,                "FPGA0 Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail09,                eUpgradeFail09,                "FPGA1 Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail10,                eUpgradeFail10,                "FPGA2 Upgrade Fail",              "", eHEX,   GEC_ERROR},
    {UpgradeFail11,                eUpgradeFail11,                "Xillinx FPGA Upgrade Fail",       "", eHEX,   GEC_ERROR},
    {UpgradeFail12,                eUpgradeFail12,                "",                                "", eHEX,   0},
    {UpgradeFail13,                eUpgradeFail13,                "",                                "", eHEX,   0},
    {UpgradeFail14,                eUpgradeFail14,                "",                                "", eHEX,   0},
    {UpgradeFail15,                eUpgradeFail15,                "",                                "", eHEX,   0},
    {MultimediaFail,               eMultimediaFail,               "Multimedia Fail",                 "", eHEX,   GEC_ERROR},
    {WatchDogReset,                eWatchDogReset,                "Watch Dog Reset",                 "", eHEX,   GEC_ERROR},
    {WatchDogReset01,              eWatchDogReset01,              "",                                "", eHEX,   0},
    {WatchDogReset02,              eWatchDogReset02,              "",                                "", eHEX,   0},
    {WatchDogReset03,              eWatchDogReset03,              "",                                "", eHEX,   0},
    {WatchDogReset04,              eWatchDogReset04,              "",                                "", eHEX,   0},
    {AutoSystemShutdown,           eAutoSystemShutdown,           "Auto System Shutdown",            "", eHEX,   GEC_ERROR},
    {SleepShutdown,                eSleepShutdown,                "Sleep Shutdown",                  "", eHEX,   GEC_ERROR},
    {RTCLowBattery,                eRTCLowBattery,                "RTC Low Battery",                 "", eHEX,   GEC_ERROR},
    {RTCFail,                      eRTCFail,                      "RTC Fail",                        "", eHEX,   GEC_ERROR},
    {LANFailthenrestart,           eLANFailthenrestart,           "LAN Fail then restart",           "", eHEX,   GEC_ERROR},
    {PowerOffCount,                ePowerOffCount,                "",                                "", eHEX,   0},
    {PowerOnCount,                 ePowerOnCount,                 "",                                "", eHEX,   0},
    {RetryCoolingCount,            eRetryCoolingCount,            "Retry Cooling Count",             "", eHEX,   GEC_ERROR},
    {AnalogSourceLockFail,         eAnalogSourceLockFail,         "",                                "", eHEX,   0},
    {AnalogSourceLockFail01,       eAnalogSourceLockFail01,       "",                                "", eHEX,   0},
    {AnalogSourceLockFail02,       eAnalogSourceLockFail02,       "",                                "", eHEX,   0},
    {AnalogSourceLockFail03,       eAnalogSourceLockFail03,       "",                                "", eHEX,   0},
    {AnalogSourceLockFail04,       eAnalogSourceLockFail04,       "",                                "", eHEX,   0},
    {AnalogSourceLockFail05,       eAnalogSourceLockFail05,       "",                                "", eHEX,   0},
    {AnalogSourceLockFail06,       eAnalogSourceLockFail06,       "",                                "", eHEX,   0},
    {AnalogSourceLockFail07,       eAnalogSourceLockFail07,       "",                                "", eHEX,   0},
    {DigitalSourceLockFail,        eDigitalSourceLockFail,        "",                                "", eHEX,   0},
    {DigitalSourceLockFail01,      eDigitalSourceLockFail01,      "",                                "", eHEX,   0},
    {DigitalSourceLockFail02,      eDigitalSourceLockFail02,      "",                                "", eHEX,   0},
    {DigitalSourceLockFail03,      eDigitalSourceLockFail03,      "",                                "", eHEX,   0},
    {DigitalSourceLockFail04,      eDigitalSourceLockFail04,      "",                                "", eHEX,   0},
    {DigitalSourceLockFail05,      eDigitalSourceLockFail05,      "",                                "", eHEX,   0},
    {DigitalSourceLockFail06,      eDigitalSourceLockFail06,      "",                                "", eHEX,   0},
    {DigitalSourceLockFail07,      eDigitalSourceLockFail07,      "",                                "", eHEX,   0},
    {FogFilter,                    eFogFilter,                    "Fog Filter",                      "", eHEX,   GEC_ERROR},
    {FogFilter01,                  eFogFilter01,                  "Fog Filter 01",                   "", eHEX,   GEC_ERROR},
    {FogFilter02,                  eFogFilter02,                  "Fog Filter 02",                   "", eHEX,   GEC_ERROR},
    {FogFilter03,                  eFogFilter03,                  "Fog Filter 03",                   "", eHEX,   GEC_ERROR},
    {FogFilter04,                  eFogFilter04,                  "Fog Filter 04",                   "", eHEX,   GEC_ERROR},
    {I2CError,                     eI2CError,                     "",                                "", eHEX,   0},
    {I2CError01,                   eI2CError01,                   "FrontEnd I2C Fail",               "", eHEX,   GEC_REMINDER},
    {I2CError02,                   eI2CError02,                   "MotorBoard I2C Fail",             "", eHEX,   GEC_REMINDER},
    {I2CError03,                   eI2CError03,                   "FMT Control I2C Fail",            "", eHEX,   GEC_REMINDER},
    {I2CError04,                   eI2CError04,                   "DDP4422 I2C Fail",                "", eHEX,   GEC_REMINDER},
    {I2CError05,                   eI2CError05,                   "MCU I2C Fail",                    "", eHEX,   GEC_REMINDER},
    {I2CError06,                   eI2CError06,                   "VCXO949 I2C Fail",                "", eHEX,   GEC_REMINDER},
    {I2CError07,                   eI2CError07,                   "Xillinx FPGA I2C Fail",           "", eHEX,   GEC_REMINDER},
    {I2CError08,                   eI2CError08,                   "",                                "", eHEX,   0},
    {I2CError09,                   eI2CError09,                   "",                                "", eHEX,   0},
    {I2CError10,                   eI2CError10,                   "",                                "", eHEX,   0},
    {I2CError11,                   eI2CError11,                   "",                                "", eHEX,   0},
    {I2CError12,                   eI2CError12,                   "",                                "", eHEX,   0},
    {I2CError13,                   eI2CError13,                   "",                                "", eHEX,   0},
    {I2CError14,                   eI2CError14,                   "",                                "", eHEX,   0},
    {I2CError15,                   eI2CError15,                   "",                                "", eHEX,   0},
    {I2CError16,                   eI2CError16,                   "",                                "", eHEX,   0},
    {I2CError17,                   eI2CError17,                   "",                                "", eHEX,   0},
    {I2CError18,                   eI2CError18,                   "",                                "", eHEX,   0},
    {I2CError19,                   eI2CError19,                   "",                                "", eHEX,   0},
    {I2CError20,                   eI2CError20,                   "",                                "", eHEX,   0},
    {I2CError21,                   eI2CError21,                   "",                                "", eHEX,   0},
    {I2CError22,                   eI2CError22,                   "",                                "", eHEX,   0},
    {I2CError23,                   eI2CError23,                   "",                                "", eHEX,   0},
    {I2CError24,                   eI2CError24,                   "",                                "", eHEX,   0},
    {I2CError25,                   eI2CError25,                   "",                                "", eHEX,   0},
    {I2CError26,                   eI2CError26,                   "",                                "", eHEX,   0},
    {I2CError27,                   eI2CError27,                   "",                                "", eHEX,   0},
    {I2CError28,                   eI2CError28,                   "",                                "", eHEX,   0},
    {I2CError29,                   eI2CError29,                   "",                                "", eHEX,   0},
    {I2CError30,                   eI2CError30,                   "",                                "", eHEX,   0},
    {I2CError31,                   eI2CError31,                   "",                                "", eHEX,   0},
    {I2CError32,                   eI2CError32,                   "",                                "", eHEX,   0},
    {UARTError,                    eUARTError,                    "",                                "", eHEX,   0},
    {SPIError,                     eSPIError,                     "",                                "", eHEX,   0},
    {NVRAMRecovery,                eNVRAMRecovery,                "",                                "", eHEX,   0},
    {HDCPError,                    eHDCPError,                    "",                                "", eHEX,   0},
    {HDCPError01,                  eHDCPError01,                  "",                                "", eHEX,   0},
    {HDCPError02,                  eHDCPError02,                  "",                                "", eHEX,   0},
    {HDCPError03,                  eHDCPError03,                  "",                                "", eHEX,   0},
    {HDCPError04,                  eHDCPError04,                  "",                                "", eHEX,   0},
    {HDCPError05,                  eHDCPError05,                  "",                                "", eHEX,   0},
    {HDCPError06,                  eHDCPError06,                  "",                                "", eHEX,   0},
    {HDCPError07,                  eHDCPError07,                  "",                                "", eHEX,   0},
    {FPGALockFail,                 eFPGALockFail,                 "",                                "", eHEX,   0},
    {FPGAPhaseLockFail,            eFPGAPhaseLockFail,            "",                                "", eHEX,   0},
    {I2Cwarn,                      eI2Cwarn,                      "",                                "", eHEX,   0},
    {I2Cwarn01,                    eI2Cwarn01,                    "FrontEnd I2C Retry",              "", eHEX,   GEC_REMINDER},
    {I2Cwarn02,                    eI2Cwarn02,                    "MotorBoard I2C Retry",            "", eHEX,   GEC_REMINDER},
    {I2Cwarn03,                    eI2Cwarn03,                    "FMT Control I2C Retry",           "", eHEX,   GEC_REMINDER},
    {I2Cwarn04,                    eI2Cwarn04,                    "DDP4422 I2C Retry",               "", eHEX,   GEC_REMINDER},
    {I2Cwarn05,                    eI2Cwarn05,                    "MCU I2C Retry",                   "", eHEX,   GEC_REMINDER},
    {I2Cwarn06,                    eI2Cwarn06,                    "VCXO949 I2C Retry",               "", eHEX,   GEC_REMINDER},
    {I2Cwarn07,                    eI2Cwarn07,                    "Xillinx FPGA I2C Retry",          "", eHEX,   GEC_REMINDER},
    {I2Cwarn08,                    eI2Cwarn08,                    "",                                "", eHEX,   0},
    {I2Cwarn09,                    eI2Cwarn09,                    "",                                "", eHEX,   0},
    {I2Cwarn10,                    eI2Cwarn10,                    "",                                "", eHEX,   0},
    {I2Cwarn11,                    eI2Cwarn11,                    "",                                "", eHEX,   0},
    {I2Cwarn12,                    eI2Cwarn12,                    "",                                "", eHEX,   0},
    {I2Cwarn13,                    eI2Cwarn13,                    "",                                "", eHEX,   0},
    {I2Cwarn14,                    eI2Cwarn14,                    "",                                "", eHEX,   0},
    {I2Cwarn15,                    eI2Cwarn15,                    "",                                "", eHEX,   0},
    {I2Cwarn16,                    eI2Cwarn16,                    "",                                "", eHEX,   0},
    {I2Cwarn17,                    eI2Cwarn17,                    "",                                "", eHEX,   0},
    {I2Cwarn18,                    eI2Cwarn18,                    "",                                "", eHEX,   0},
    {I2Cwarn19,                    eI2Cwarn19,                    "",                                "", eHEX,   0},
    {I2Cwarn20,                    eI2Cwarn20,                    "",                                "", eHEX,   0},
    {I2Cwarn21,                    eI2Cwarn21,                    "",                                "", eHEX,   0},
    {I2Cwarn22,                    eI2Cwarn22,                    "",                                "", eHEX,   0},
    {I2Cwarn23,                    eI2Cwarn23,                    "",                                "", eHEX,   0},
    {I2Cwarn24,                    eI2Cwarn24,                    "",                                "", eHEX,   0},
    {I2Cwarn25,                    eI2Cwarn25,                    "",                                "", eHEX,   0},
    {I2Cwarn26,                    eI2Cwarn26,                    "",                                "", eHEX,   0},
    {I2Cwarn27,                    eI2Cwarn27,                    "",                                "", eHEX,   0},
    {I2Cwarn28,                    eI2Cwarn28,                    "",                                "", eHEX,   0},
    {I2Cwarn29,                    eI2Cwarn29,                    "",                                "", eHEX,   0},
    {I2Cwarn30,                    eI2Cwarn30,                    "",                                "", eHEX,   0},
    {I2Cwarn31,                    eI2Cwarn31,                    "",                                "", eHEX,   0},
    {I2Cwarn32,                    eI2Cwarn32,                    "",                                "", eHEX,   0}
};

#endif

