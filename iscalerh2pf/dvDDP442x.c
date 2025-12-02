// ===============================================================================
// FILE NAME: dvDDP442x.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/05/11, Casper Create
// --------------------
// ===============================================================================

#include "dvDDP442x.h"
#include "Board_I2C_Dev_Table.h"
#include "Board_I2C.h"
//#include "Board_GPIO.h"
//#include "utilCounterAPI.h"
#include "utilDbgMsg.h"
#include "utilOPD_TEST.h"
#include "gpio.h"

//#include "utilLogAPI.h"
//#include "utilCounterAPI.h"

#define DDP_BUSY_GPIO GPIO4_IO26_DISP0_DAT5 //GPIO4_IO24 DISP0_DAT3 //EVT->GPIO4_IO26 (122)
#define DDP_BUSY_HIGH '1'
#define DDP_BUSY_LOW  '0'

static BOOL m_bAsicReady = FALSE;
static BOOL m_bUpgrade = FALSE;

static char acLastFuncName[64];
pthread_mutex_t axMutex ;
#define DDP_SEMPHORE_WAIT_DELAY         (5000)
#define DDP_MutexCreate()               pthread_mutex_init(&axMutex, NULL)

static uint8 DDP_MutexTake(const char *cFuncName)
{
    int err;
    struct timespec s_timeout;
    clock_gettime(CLOCK_REALTIME, &s_timeout);
    util_TimespecAddms(&s_timeout, DDP_SEMPHORE_WAIT_DELAY);

    err = pthread_mutex_timedlock(&axMutex, &s_timeout);
    if(err != 0)
    {
        LOG_MSG(db_ALWAYS, "014 %s pthread_mutex_timedlock fail [%s]\n", cFuncName , strerror(err));
        return 0;
    }

    snprintf(acLastFuncName, sizeof(acLastFuncName), "%s", cFuncName);

    return 1;  //pass
}

static uint8 DDP_MutexGive(void)
{
    int err;
    err = pthread_mutex_unlock(&axMutex);
    if(err != 0)
    {
        LOG_MSG(db_ALWAYS, "%s pthread_mutex_unlock fail [%s]\n", __FUNCTION__ , strerror(err));
        return 0;
    }

    return 1;  //pass
}


#if defined(PLATFORM_H60_2K) || defined(PLATFORM_R70K)	//HICC2_Doulas_0001 //HICC2_AC_0003
static const I2C_COMMAND_TABLE DDPCommandTable[] =
{
/****************************************************************************/
/* CmdIndex                       | BytesOut  | BytesIn | SpecialFlag       */
/*0x00***********************************************************************/
    {CMDF_STATUS_READ              ,   0x00,       0x01,        0},
    {CMDF_PROJECTIONMODE           ,   0x01,       0x01,        0},
    {CMDF_SYSTEMMODE               ,   0x02,       0x02,        0},
    {CMDF_UTLCWINDEX               ,   0x03,       0x02,        0},
    {CMDF_INDEXDELAYFINE           ,   0x03,       0x03,        0},
    {CMDF_CWSPEED                  ,   0x02,       0x00,        0},
    {CMDF_CWSPINNINGCONTROL        ,   0x02,       0x02,        0},
    {CMDF_CWCOASTCONTROL           ,   0x02,       0x01,        0},
    {CMDF_CWDEBUGCONTROL           ,   0x02,       0x01,        0},
    {CMDF_CWSTATE                  ,   0x00,       0x03,        0},
    {CMDF_CWVSYNC                  ,   0x02,       0x02,        0},
    {CMDF_CWINDEXCLOCK             ,   0x01,       0x08,        0},
    {CMDF_HWTESTMUX                ,   0x02,       0x02,        0},
    {CMDF_SWTESTMUX                ,   0x02,       0x02,        0},
    {CMDF_APPDEBUG                 ,   0x04,       0x06,        0},
    {CMDF_USBDEBUG                 ,   0x03,       0x00,        0},
/*0x10***********************************************************************/
    {CMDF_VERTICALFLIP             ,   0x01,       0x01,        0},
    {CMDF_TPGPATTERN               ,   0x01,       0x01,        0},
    {CMDF_TPGCSIZE                 ,   0x02,       0x02,        0},
    {CMDF_SFGCOLOR                 ,   0x06,       0x06,        0},
    {CMDF_ALGORITHM                ,   0x02,       0x01,        0},
    {CMDF_I2CREAD                  ,   0x01,       0x00,        0},
    {CMDF_IMPBRIGHTNESS            ,   0x02,       0x02,        0},
    {CMDF_IMPCONTRAST              ,   0x02,       0x02,        0},
    {CMDF_SHARPNESS                ,   0x02,       0x02,        0},
    {CMDF_SOURCECSC                ,   0x02,       0x01,        0},
    {CMDF_IMAGECCA                 ,   0x60,       0x60,        0},
    {CMDF_IMAGEOFF                 ,   0x06,       0x06,        0},
    {CMDF_IMAPPLYHSG               ,   0x2A,       0x2A,        0},
    {CMDF_WHITEPEAKING             ,   0x01,       0x01,        0},
    {CMDF_BGCOLORMODE              ,   0x02,       0x02,        0},
    {CMDF_HORIZONTALFLIP           ,   0x01,       0x01,        0},
/*0x20***********************************************************************/
    {CMDF_GETSYSMODEFEATURES       ,   0x02,       0x0D,        0},
    {CMDF_SFGRESOLUTION            ,   0x00,       0x04,        0},
    {CMDF_SPLASHLOAD               ,   0x02,       0x00,        0},
    {CMDF_DISPLAY                  ,   0x11,       0x11,        0},
    {CMDF_SOURCECONFIG             ,   0x33,       0x33,        0},
    {CMDF_SRCOFFSET                ,   0x06,       0x06,        0},
    {CMDF_SRCPORT                  ,   0x03,       0x02,        0},
    {CMDF_SGSRCPORT_VSYNCPERIOD    ,   0x01,       0x04,        0},
    {CMDF_CHANNELSOURCE            ,   0x01,       0x01,        0},
    {CMDF_FREEZE                   ,   0x01,       0x01,        0},
    {CMDF_CHANNELCONFIG            ,   0x08,       0x07,        0},
    {CMDF_TPGBORDER                ,   0x01,       0x01,        0},
    {CMDF_TPGRESOLUTION            ,   0x04,       0x04,        0},
    {CMDF_GAMMACURVES              ,   0x03,       0x03,        0},
    {CMDF_CALCPIXELCLOCK           ,   0x02,       0x06,        0},
    {CMDF_SRCDESCRIPTION           ,   0x11,       0x11,        0},
/*0x30***********************************************************************/
    {CMDF_UTLPROGRAMMODE           ,   0x01,       0x00,        0},
    {CMDF_LAMPENABLE               ,   0x01,       0x01,        0},
    {CMDF_LAMPENABLEDELAY          ,   0x02,       0x02,        0},
    {CMDF_STRIKERESET              ,   0x01,       0x01,        0},
    {CMDF_LAMPSTRIKERESETDELAY     ,   0x02,       0x02,        0},
    {CMDF_LAMPLIT                  ,   0x00,       0x01,        0},
    {CMDF_LAMPLITSTABLE            ,   0x00,       0x01,        0},
    {CMDF_SYNCCOUNT                ,   0x01,       0x01,        0},
    {CMDF_LAMPBIN                  ,   0x00,       0x01,        0},
    {CMDF_MANUALLAMPBIN            ,   0x01,       0x00,        0},
    {CMDF_MANLAMPBIN               ,   0x01,       0x01,        0},
    {CMDF_COMM                     ,   0x01,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMINDEX     ,   0x01,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMID        ,   0x00,       0x01,        0},
    {CMDF_VARNUMLAMPWAVEFORMS      ,   0x00,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMGAIN      ,   0x01,       0x01,        0},
/*0x40***********************************************************************/
    {CMDF_COLORTEMPPRESET          ,   0x03,       0x03,        0},
    {CMDF_HUECOLOR                 ,   0x04,       0x04,        0},
    {CMDF_DBENABLE                 ,   0x01,       0x01,        0},
    {CMDF_DBPOSITION               ,   0x02,       0x02,        0},
    {CMDF_DBMINMAX                 ,   0x04,       0x04,        0},
    {CMDF_DBMANMODE                ,   0x01,       0x00,        0},
    {CMDF_DBAPERTURE               ,   0x01,       0x02,        0},
    {CMDF_DBAPERTUREINDEX          ,   0x04,       0x04,        0},
    {CMDF_DBBORDERCONF             ,   0x08,       0x08,        0},
    {CMDF_DBBORDERWEIGHT           ,   0x01,       0x01,        0},
    {CMDF_DBCLIPPIXELS             ,   0x02,       0x02,        0},
    {CMDF_DBGAIN                   ,   0x02,       0x02,        0},
    {CMDF_DBNUMSTEPS               ,   0x02,       0x02,        0},
    {CMDF_DBSPEED                  ,   0x02,       0x02,        0},
    {CMDF_DBSTRENGTH               ,   0x01,       0x01,        0},
    {CMDF_DBHIST                   ,   0x88,       0x88,        0},
/*0x50************************************************************************/
    {CMDF_VARWAVEFORMMINGAIN       ,   0x02,       0x01,        0},
    {CMDF_VARWAVEFORMMAXGAIN       ,   0x01,       0x01,        0},
    {CMDF_VARLAMPMANUFACTID        ,   0x00,       0x01,        0},
    {CMDF_VARLAMPBLSTID            ,   0x00,       0x02,        0},
    {CMDF_VARLAMPBLSTSTATUS        ,   0x00,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMSTATUSINFO,   0x00,       0x02,        0},
    {CMDF_LAMPSYNCTYPE             ,   0x03,       0x03,        0},
    {CMDF_LAMPSYNCDELAY            ,   0x02,       0x02,        0},
    {CMDF_VARLAMPBLSTWRITE         ,   0x01,       0x01,        0},
    {CMDF_VARBLSTMAILBOXSTATUS     ,   0x00,       0x03,        0},
    {CMDF_DYNBLSTGAIN              ,   0x01,       0x00,        0},
    {CMDF_DYNBLSTINTECSMODE        ,   0x01,       0x00,        0},
    {CMDF_DYNBLSTSTATUS            ,   0x00,       0x08,        0},
    {CMDF_DBINITIALIZE             ,   0x00,       0x00,        0},
    {CMDF_DBAVAILABLE              ,   0x00,       0x01,        0},
    {CMDF_AUTOLOCKSTATUS           ,   0x01,       0x2c,        0},
/*0x60***********************************************************************/
    {CMDF_ALGCONTROL               ,   0x01,       0x01,        0},
    {CMDF_3DENABLE                 ,   0x01,       0x01,        0},
    {CMDF_FRM                      ,   0x10,       0x10,        0},
    {CMDF_VIEWINGMODE              ,   0x01,       0x01,        0},
    {CMDF_3DFREEZE                 ,   0x01,       0x01,        0},
    {CMDF_3DDLPLINKPULSE           ,   0x01,       0x01,        0},
    {CMDF_3DREFPOLARITY            ,   0x04,       0x04,        0},
    {CMDF_3DSOURCECONFIG           ,   0x0d,       0x0d,        0},
    {CMDF_RESYNC                   ,   0x01,       0x00,        0},
    {CMDF_3DSYNCINPUTSELECT        ,   0x01,       0x01,        0},
    {CMDF_3DSYNCOUTPUTSELECT       ,   0x01,       0x01,        0},
    {CMDF_3DSYNCINPUT_MONITOR      ,   0x00,       0x03,        0},
    {CMDF_3D_FORMAT                ,   0x01,       0x01,        0},
    {CMDF_LIMIT_BLACK_LEVEL        ,   0x01,       0x01,        0},
    {CMDF_LIMIT_BLACK_TOLERANCE    ,   0x01,       0x01,        0},
    {CMDF_LIMIT_BLACK_STATUS       ,   0x00,       0x01,        0},
/*0x70***********************************************************************/
    {CMDF_BRSDISABLEDC             ,   0x01,       0x01,        0},
    {CMDF_BRSDIAGCFG               ,   0x06,       0x06,        0},
    {CMDF_BRSDIAG                  ,   0x01,       0x01,        0},
    {CMDF_BRSMAXGAIN               ,   0x04,       0x04,        0},
    {CMDF_BRSMAXDIGGAIN            ,   0x03,       0x03,        0},
    {CMDF_BRSCLIPPCT               ,   0x03,       0x03,        0},
    {CMDF_BRSSCENECHGRATE          ,   0x01,       0x01,        0},
    {CMDF_BRSMINDC                 ,   0x06,       0x06,        0},
    {CMDF_BRSHISTSHIFT             ,   0x01,       0x01,        0},
    {CMDF_BRSHISTREST              ,   0x00,       0x00,        0},
    {CMDF_BRSDYNLUMASHIFT          ,   0x01,       0x01,        0},
    {CMDF_BRSDYNLUMASHIFTTHR       ,   0x04,       0x04,        0},
    {CMDF_BRSDYNLUMASHIFTSTATIC    ,   0x01,       0x01,        0},
    {CMDF_BRSDYNLUMASHIFTMOVE      ,   0x01,       0x01,        0},
    {CMDF_BRSDYNLUMASHIFTHISTDELAY ,   0x01,       0x01,        0},
    {CMDF_DMDSS                    ,   0x01,       0x01,        0},
/*0x80***********************************************************************/
    {CMDF_CMDDMDDEVTYPE            ,   0x00,       0x02,        0},
    {CMDF_CMDDMDRESO               ,   0x00,       0x04,        0},
    {CMDF_SYSTEMSTATUS             ,   0x00,       0x04,        0},
    {CMDF_SETSLAVEPOWERSTANDBY     ,   0x01,       0x04,        0},
    {CMDF_SETPOWER                 ,   0x01,       0x01,        0},
    {CMDF_APPVERSIONS              ,   0x00,       0x15,        0},
    {CMDF_DMDPOWER                 ,   0x01,       0x01,        0},
    {CMDF_DMDPARK                  ,   0x01,       0x01,        0},
    {CMDF_TSKSSTATE                ,   0x00,       0x01,        0},
    {CMDF_GETTSKSTATE              ,   0x01,       0x0B,        0},
    {CMDF_SETTSKPRIOR              ,   0x02,       0x00,        0},
    {CMDF_SETTSKSUSP               ,   0x02,       0x00,        0},
    {CMDF_SYSSTACK                 ,   0x00,       0x0C,        0},
    {CMDF_MEMPOOL                  ,   0x00,       0x68,        0},
    {CMDF_RESOURCE                 ,   0x00,       0x0C,        0},
    {CMDF_MEMWRITEREAD             ,   0x09,       0x04,        0},
/*0x90***********************************************************************/
    {CMDF_DDP_SWRESET              ,   0x01,       0x00,        0},
    {CMDF_DDPRESETCOUNT            ,   0x00,       0x01,        0},
    {CMDF_DDPRESETCAUSE            ,   0x00,       0x01,        0},
    {CMDF_USBCLOCK                 ,   0x01,       0x01,        0},
    {CMDF_SETGETDDPCLOCK           ,   0x01,       0x01,        0},
    {CMDF_GETDDPCLOCKFREQ          ,   0x01,       0x04,        0},
    {CMDF_DDPGETASICCONFIG         ,   0x00,       0x18,        0},
    {CMDF_GETDDPASICINFO           ,   0x00,       0x06,        0},
    {CMDF_WAP_SETGET               ,   0x02,       0x00,        0},
    {CMDF_WAP                      ,   0x05,       0x0E,        0},
    {CMDF_SCALARBYPASSMODE         ,   0x01,       0x01,        0},
    {CMDF_FRCBYPASSMODE            ,   0x01,       0x01,        0},
    {CMDF_ACTUATOR_PARAMETER       ,   0x06,       0x71,        0}, //HICC2_AC_0003 //PLATFORM_R70K
    {CMDF_WAP_ALLSET_TRPDMD_ESTATUS,   0xC1,       0x01,        0}, //G50_Casper_0006 //HICC2_Casper_0024 (mode:16 * seg : 6 * uint16 size : 2) + 1(level)
    {CMDF_APPCMDDELAY              ,   0x04,       0x00,        0},
    {CMDF_RX24TVERSION             ,   0x01,       0x02,        0}, //H30K_Doulas_0007//HICC2_Doulas_0018
/*0xA0***********************************************************************/
    {CMDF_FANFREQCYCLE             ,   0x04,       0x02,        0},
    {CMDF_FANPARALLEL              ,   0x02,       0x01,        0},
    {CMDF_CWDIRECTION              ,   0x01,       0x01,        0},
    {CMDF_CWDIRECTIONENABLE        ,   0x00,       0x01,        0},
    {CMDF_CWCLOCK                  ,   0x01,       0x00,        0},
    {CMDF_CWBRAKE                  ,   0x01,       0x00,        0},
    {CMDF_CWTYPE                   ,   0x01,       0x01,        0},
    {CMDF_CWMOTORTYPE              ,   0x01,       0x01,        0},
    {CMDF_DUALCWDELAY              ,   0x03,       0x03,        0},
    {CMDF_DUALCWSTRENGTH           ,   0x01,       0x01,        0},
    {CMDF_CWSEQUENCER              ,   0x01,       0x01,        0},
    {CMDF_CWSPOKETEST              ,   0x01,       0x01,        0},
    {CMDF_CWSPOKECONFIG            ,   0x03,       0x03,        0},
    {CMDF_FANDEVICE                ,   0x01,       0x04,        0},
    {CMDF_SET_GETDUALCWDELAY       ,   0x01,       0x00,        0},
    {CMDF_FANNUMSELECT             ,   0x02,       0x00,        0},
/*0xB0***********************************************************************/
    {CMDF_PWMOUTPUTCONFIG          ,   0x08,       0x38,        0},
    {CMDF_PWMINPUTCONFIG           ,   0x06,       0x14,        0},
    {CMDF_DMDLOCK                  ,   0x01,       0x01,        0},
    {CMDF_DISPOFFSET               ,   0x04,       0x04,        0},
    {CMDF_COLORPROFILE             ,   0x01,       0x01,        0},
    {CMDF_CWINDEXPOL               ,   0x03,       0x03,        0},
    {CMDF_DISPKEYENA               ,   0x01,       0x01,        0},
    {CMDF_DISPKEYANG               ,   0x08,       0x08,        0},
    {CMDF_DISPKEYPIT               ,   0x00,       0x04,        0},
    {CMDF_DISPKEYYAW               ,   0x00,       0x04,        0},
    {CMDF_DISPKEYCORNERS           ,   0x10,       0x10,        0},
    {CMDF_DISPKEYANCHOROVERRIDE    ,   0x02,       0x02,        0},
    {CMDF_RESERVED_BC              ,   0x00,       0x00,        0},
    {CMDF_ASPECT_RATIO             ,   0x01,       0x01,        0},
    {CMDF_SEQREVISION              ,   0x00,       0x04,        0},
    {CMDF_EMERGSHUTDOWN            ,   0x01,       0x01,        0},
/*0xC0***********************************************************************/
    {CMDF_SSIC1CURRENTS            ,   0x14,       0x14,        0},
    {CMDF_SSIREDCURRENTS           ,   0x14,       0x14,        0},
    {CMDF_SSIGRNCURRENTS           ,   0x14,       0x14,        0},
    {CMDF_SSIBLUCURRENTS           ,   0x14,       0x14,        0},
    {CMDF_SSIDRIVERSTATUS          ,   0x00,       0x03,        1},
    {CMDF_SSIREDDRIVERTIMING       ,   0x04,       0x04,        1},
    {CMDF_SSIGRNDRIVERTIMING       ,   0x04,       0x04,        1},
    {CMDF_SSIBLUDRIVERTIMI         ,   0x04,       0x04,        1},
    {CMDF_SSIEXTSENSORXY           ,   0x1F,       0x1F,        0},
    {CMDF_SSISENSORCAL             ,   0x15,       0x15,        0},
    {CMDF_SSISENSOR                ,   0x00,       0x0A,        0},
    {CMDF_SSISENSORTIMING          ,   0x0C,       0x0D,        0},
    {CMDF_CCIMODE                  ,   0x01,       0x01,        0},
    {CMDF_CCIENABLE                ,   0x01,       0x01,        0},
    {CMDF_SSIPWMDRIVERLEVELS       ,   0x0C,       0x0C,        0},
    {CMDF_SSIPWMDRIVERLEVELS2      ,   0x0C,       0x0C,        0},
/*0xD0***********************************************************************/
    {CMDF_PWMPARAMS                ,   0x05,       0x04,        0},
    {CMDF_SSILEDMAXCURRENT         ,   0x0a,       0x0a,        0},
    {CMDF_RESERVED_D2              ,   0x00,       0x00,        0},
    {CMDF_RESERVED_D3              ,   0x00,       0x14,        0},
    {CMDF_SSIDCSEQINDEX            ,   0x02,       0x02,        0},
    {CMDF_SSIDCDUMPUART            ,   0x00,       0x00,        0},
    {CMDF_SSIRAMPENABLE            ,   0x02,       0x01,        0},
    {CMDF_SPICONFIG                ,   0x11,       0x0F,        2},
    {CMDF_ALC_RGB_LEVEL            ,   0x00,       0x0C,        0},
    {CMDF_RESERVED_D9              ,   0x00,       0x00,        0},
    {CMDF_4WAYXPR_PARAM            ,   0x06,       0x04,        2}, //HICC2_AC_0014 //PLATFORM_R70K
    {CMDF_ABPINFO                  ,   0x09,       0x01,        0},
    {CMDF_DIMPOWER                 ,   0x02,       0x00,        0},
    {CMDF_REALBLACK_LIGHTSOUTTIMER ,   0x01,       0x00,        0},
    {CMDF_SMOOTH_ON                ,   0x02,       0x01,        0},
    {CMDF_RESERVE_DF               ,   0x00,       0x00,        0},
/*0xE0***********************************************************************/
    {CMDF_MEMORYTDUMP              ,   0x06,       0x00,        0},
    {CMDF_GETMEMORYDUMP            ,   0x08,       0xFF,        0},
    {CMDF_MEMBIST                  ,   0x0b,       0x00,        0},
    {CMDF_UARTCONFIG               ,   0x0a,       0x0a,        0},
    {CMDF_GETUARTPORT              ,   0x01,       0x02,        0},
    {CMDF_UARTRXDSOURCESELECT      ,   0x02,       0x01,        0},
    {CMDF_AUTOUPDATE               ,   0x01,       0x00,        0},
    {CMDF_I2CPASS                  ,   0x0b,       0x03,        0},
    {CMDF_GETI2CPASS               ,   0x0a,       0x00,        0},
    {CMDF_IMAGERGBGAIN             ,   0x06,       0x06,        0},
    {CMDF_EEPROMVER                ,   0x83,       0x00,        0},
    {CMDF_RESERVED_EB              ,   0x00,       0x00,        0},
    {CMDF_BLACKSIGNALSWITCH        ,   0x01,       0x00,        0},
    {CMDF_RESERVED_ED              ,   0x00,       0x00,        0},
    {CMDF_BCCALIBRATE              ,   0x38,       0x00,        0},
    {CMDF_GAMMATABLE               ,   0x01,       0x01,        0},
/*0xF0***********************************************************************/
    {CMDF_FPGACTRL_WRRD            ,   0x00,       0x00,        0}, //HICC2_AC_0003 //PLATFORM_R70K
    {CMDF_RESERVED_F1              ,   0x00,       0x00,        0},
    {CMDF_RESERVED_F2              ,   0x00,       0x00,        0},
    {CMDF_FPGACTRL_SETGET          ,   0x00,       0x00,        0}, //HICC2_AC_0003 //PLATFORM_R70K
    {CMDF_RESERVED_F4              ,   0x00,       0x00,        0},
    {CMDF_RGBGAINADJUSTMENTS       ,   0x12,       0x12,        0},
    {CMDF_APCCT                    ,   0x07,       0x0C,        0},
    {CMDF_SETCOLOROFFSET           ,   0x01,       0x01,        0},
    {CMDF_SPLASH_SCREEN            ,   0x00,       0x00,        0},
    {CMDF_RESERVED_F9              ,   0x00,       0x00,        0},
    {CMDF_SRC_AUTODETECT           ,   0x01,       0x00,        0},
    {CMDF_SEQVERSION               ,   0x00,       0x04,        0},
    {CMDF_IMG_ALGORITHM            ,   0x01,       0x01,        0},
    {CMDF_IMG_ORIENTATION          ,   0x01,       0x01,        0},
    {CMDF_CMPSR_PASSTHRU           ,   0x05,       0x40,        0},
    {CMDF_PARAMETER_SET            ,   0x06,       0x00,        0},
};
#else
//ZU860_John_0013 sync with DDP API 9.0
static const I2C_COMMAND_TABLE DDPCommandTable[] =
{
/****************************************************************************/
/* CmdIndex                       | BytesOut  | BytesIn | SpecialFlag       */
/*0x00***********************************************************************/
    {CMDF_STATUS_READ                         ,   0x00,       0x01,        0},
    {CMDF_PROJECTIONMODE                      ,   0x01,       0x01,        0},
    {CMDF_SYSTEMMODE                          ,   0x02,       0x02,        0},
    {CMDF_UTLCWINDEX                          ,   0x03,       0x02,        0},		//G100_Doulas_0009 Modify
    {CMDF_INDEXDELAYFINE                      ,   0x03,       0x03,        0},
    {CMDF_CWSPEED                             ,   0x02,       0x00,        0},
    {CMDF_CWSPINNINGCONTROL                   ,   0x02,       0x01,        0},
    {CMDF_CWCOASTCONTROL                      ,   0x02,       0x01,        0},
    {CMDF_CWDEBUGCONTROL                      ,   0x02,       0x01,        0},
    {CMDF_CWSTATE                             ,   0x00,       0x03,        0},
    {CMDF_CWVSYNC                             ,   0x02,       0x02,        0},
    {CMDF_CWINDEXCLOCK                        ,   0x01,       0x08,        0},
    {CMDF_HWTESTMUX                           ,   0x02,       0x02,        0},
    {CMDF_SWTESTMUX                           ,   0x02,       0x02,        0},
    {CMDF_APPDEBUG                            ,   0x04,       0x06,        0},
    {CMDF_USBDEBUG                            ,   0x03,       0x00,        0},
/*0x1/**********************************************************************************/
    {CMDF_VERTICALFLIP                        ,   0x01,       0x01,        0},
    {CMDF_TPGPATTERN                          ,   0x01,       0x01,        0},
    {CMDF_TPGCSIZE                            ,   0x02,       0x02,        0},
    {CMDF_SFGCOLOR                            ,   0x06,       0x06,        0},
    {CMDF_ALGORITHM                           ,   0x02,       0x01,        0},
    {CMDF_I2CREAD                             ,   0x01,       0x00,        0},
    {CMDF_IMPBRIGHTNESS                       ,   0x02,       0x02,        0},
    {CMDF_IMPCONTRAST                         ,   0x02,       0x02,        0},
    {CMDF_SHARPNESS                           ,   0x02,       0x02,        0},
    {CMDF_SOURCECSC                           ,   0x02,       0x01,        0},
    {CMDF_IMAGECCA                            ,   0x60,       0x60,        0},
    {CMDF_IMAGEOFF                            ,   0x06,       0x06,        0},
    {CMDF_IMAPPLYHSG                          ,   0x2A,       0x2A,        0},
    {CMDF_WHITEPEAKING                        ,   0x01,       0x01,        0},
    {CMDF_BGCOLORMODE                         ,   0x02,       0x02,        0},
    {CMDF_HORIZONTALFLIP                      ,   0x01,       0x01,        0},
/*0x2/**********************************************************************************/
    {CMDF_GETSYSMODEFEATURES                  ,   0x02,       0x0D,        0},
    {CMDF_SFGRESOLUTION                       ,   0x00,       0x04,        0},
    {CMDF_SPLASHLOAD                          ,   0x02,       0x00,        0},
    {CMDF_DISPLAY                             ,   0x11,       0x11,        0},
    {CMDF_SOURCECONFIG                        ,   0x33,       0x33,        0},
    {CMDF_SRCOFFSET                           ,   0x06,       0x06,        0},
    {CMDF_SRCPORT                             ,   0x03,       0x02,        0},
    {CMDF_SGSRCPORT_VSYNCPERIOD               ,   0x01,       0x04,        0},
    {CMDF_CHANNELSOURCE                       ,   0x01,       0x01,        0},
    {CMDF_FREEZE                              ,   0x01,       0x01,        0},
    {CMDF_CHANNELCONFIG                       ,   0x08,       0x07,        0},
    {CMDF_TPGBORDER                           ,   0x01,       0x01,        0},
    {CMDF_TPGRESOLUTION                       ,   0x04,       0x04,        0},
    {CMDF_GAMMACURVES                         ,   0x03,       0x03,        0},
    {CMDF_CALCPIXELCLOCK                      ,   0x02,       0x06,        0},
    {CMDF_SRCDESCRIPTION                      ,   0x11,       0x11,        0},
/*0x3/**********************************************************************************/
    {CMDF_UTLPROGRAMMODE                      ,   0x01,       0x00,        0},
    {CMDF_LAMPENABLE                          ,   0x01,       0x01,        0},
    {CMDF_LAMPENABLEDELAY                     ,   0x02,       0x02,        0},
    {CMDF_STRIKERESET                         ,   0x01,       0x01,        0},
    {CMDF_LAMPSTRIKERESETDELAY                ,   0x02,       0x02,        0},
    {CMDF_LAMPLIT                             ,   0x00,       0x01,        0},
    {CMDF_LAMPLITSTABLE                       ,   0x00,       0x01,        0},
    {CMDF_SYNCCOUNT                           ,   0x01,       0x01,        0},
    {CMDF_LAMPBIN                             ,   0x00,       0x01,        0},
    {CMDF_MANUALLAMPBIN                       ,   0x01,       0x00,        0},
    {CMDF_MANLAMPBIN                          ,   0x01,       0x01,        0},
    {CMDF_COMM                                ,   0x01,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMINDEX                ,   0x01,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMID                   ,   0x00,       0x01,        0},
    {CMDF_VARNUMLAMPWAVEFORMS                 ,   0x00,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMGAIN                 ,   0x01,       0x01,        0},
/*0x4/**********************************************************************************/
    {CMDF_COLOR_TEMP_PRESET                   ,   0x03,       0x03,        0},
    {CMDF_HUECOLOR                            ,   0x04,       0x04,        0},
    {CMDF_DBENABLE                            ,   0x01,       0x01,        0},
    {CMDF_DBPOSITION                          ,   0x02,       0x02,        0},
    {CMDF_DBMINMAX                            ,   0x04,       0x04,        0},
    {CMDF_DBMANMODE                           ,   0x01,       0x00,        0},
    {CMDF_DBAPERTURE                          ,   0x01,       0x02,        0},
    {CMDF_DBAPERTUREINDEX                     ,   0x04,       0x04,        0},
    {CMDF_DBBORDERCONF                        ,   0x08,       0x08,        0},
    {CMDF_DBBORDERWEIGHT                      ,   0x01,       0x01,        0},
    {CMDF_DBCLIPPIXELS                        ,   0x02,       0x02,        0},
    {CMDF_DBGAIN                              ,   0x02,       0x02,        0},
    {CMDF_DBNUMSTEPS                          ,   0x02,       0x02,        0},
    {CMDF_DBSPEED                             ,   0x02,       0x02,        0},
    {CMDF_DBSTRENGTH                          ,   0x01,       0x01,        0},
    {CMDF_DBHIST                              ,   0x88,       0x88,        0},
/*0x5/***********************************************************************************/
    {CMDF_VARWAVEFORMMINGAIN                  ,   0x02,       0x01,        0},
    {CMDF_VARWAVEFORMMAXGAIN                  ,   0x01,       0x01,        0},
    {CMDF_VARLAMPMANUFACTID                   ,   0x00,       0x01,        0},
    {CMDF_VARLAMPBLSTID                       ,   0x00,       0x02,        0},
    {CMDF_VARLAMPBLSTSTATUS                   ,   0x00,       0x01,        0},
    {CMDF_VARLAMPWAVEFORMSTATUSINFO           ,   0x00,       0x02,        0},
    {CMDF_LAMP_SYNC_TYPE                      ,   0x03,       0x03,        0},
    {CMDF_LAMP_SYNC_DELAY                     ,   0x02,       0x02,        0},
    {CMDF_VARLAMPBLSTWRITE                    ,   0x01,       0x01,        0},
    {CMDF_VARBLSTMAILBOXSTATUS                ,   0x00,       0x03,        0},
    {CMDF_DYNBLSTGAIN                         ,   0x01,       0x00,        0},
    {CMDF_DYNBLSTINTECSMODE                   ,   0x01,       0x00,        0},
    {CMDF_DYNBLSTSTATUS                       ,   0x00,       0x08,        0},
    {CMDF_DBINITIALIZE                        ,   0x00,       0x00,        0},
    {CMDF_DBAVAILABLE                         ,   0x00,       0x01,        0},	//G100_Doulas_0024 Add setting power on splash timeout
    {CMDF_AUTOLOCKSTATUS                      ,   0x01,       0x2c,        0},
/*0x6/**********************************************************************************/
    {CMDF_ALGCONTROL                          ,   0x01,       0x01,        0},
    {CMDF_3DENABLE                            ,   0x01,       0x01,        0},
    {CMDF_FRM                                 ,   0x10,       0x10,        0},
    {CMDF_VIEWINGMODE                         ,   0x01,       0x01,        0},
    {CMDF_3DFREEZE                            ,   0x01,       0x01,        0},
    {CMDF_3DDLPLINKPULSE                      ,   0x01,       0x01,        0},
    {CMDF_3DREFPOLARITY                       ,   0x04,       0x04,        0},
    {CMDF_3DSOURCECONFIG                      ,   0x0d,       0x0d,        0},
    {CMDF_RESYNC                              ,   0x01,       0x00,        0},
    {CMDF_SLAVESYSTEMMODE                     ,   0x02,       0x02,        0},
    {CMDF_SLAVEPOWERSTATE                     ,   0x02,       0x02,        0},
    {CMDF_SLAVEAUTOLOCKSTATE                  ,   0x2c,       0x2c,        0},
    {CMDF_SLAVEPROJECTIONMODE                 ,   0x01,       0x01,        0},
    {CMDF_SLAVEFRM                            ,   0x10,       0x10,        0},
    {CMDF_SLAVEDISPLAY                        ,   0x11,       0x11,        0},
    {CMDF_SETSLAVEDISPLAY                     ,   0x11,       0x11,        0},
/*0x7/**********************************************************************************/
    {CMDF_BRSDISABLEDC                        ,   0x01,       0x01,        0},
    {CMDF_BRSDIAGCFG                          ,   0x06,       0x06,        0},
    {CMDF_BRSDIAG                             ,   0x01,       0x01,        0},
    {CMDF_BRSMAXGAIN                          ,   0x04,       0x04,        0},
    {CMDF_BRSMAXDIGGAIN                       ,   0x03,       0x03,        0},
    {CMDF_BRSCLIPPCT                          ,   0x03,       0x03,        0},
    {CMDF_BRSSCENECHGRATE                     ,   0x01,       0x01,        0},
    {CMDF_BRSMINDC                            ,   0x06,       0x06,        0},
    {CMDF_BRSHISTSHIFT                        ,   0x01,       0x01,        0},
    {CMDF_BRSHISTREST                         ,   0x00,       0x00,        0},
    {CMDF_BRSDYNLUMASHIFT                     ,   0x01,       0x01,        0},
    {CMDF_BRSDYNLUMASHIFTTHR                  ,   0x04,       0x04,        0},
    {CMDF_BRSDYNLUMASHIFTSTATIC               ,   0x01,       0x01,        0},
    {CMDF_BRSDYNLUMASHIFTMOVE                 ,   0x01,       0x01,        0},
    {CMDF_BRSDYNLUMASHIFTHISTDELAY            ,   0x01,       0x01,        0},
    {CMDF_DMDSS                               ,   0x01,       0x01,        0},
/*0x8/**********************************************************************************/
    {CMDF_CMDDMDDEVTYPE                       ,   0x00,       0x02,        0},
    {CMDF_CMDDMDRESO                          ,   0x00,       0x04,        0},
    {CMDF_SYSTEMSTATUS                        ,   0x00,       0x04,        0},
    {CMDF_SETSLAVEPOWERSTANDBY                ,   0x01,       0x04,        0},
    {CMDF_SETPOWER                            ,   0x01,       0x01,        0},
    {CMDF_APPVERSIONS                         ,   0x00,       0x15,        0},
    {CMDF_DMDPOWER                            ,   0x01,       0x01,        0},
    {CMDF_DMDPARK                             ,   0x01,       0x01,        0},
    {CMDF_TSKSSTATE                           ,   0x00,       0x01,        0},
    {CMDF_GETTSKSTATE                         ,   0x01,       0x0B,        0},
    {CMDF_SETTSKPRIOR                         ,   0x02,       0x00,        0},
    {CMDF_SETTSKSUSP                          ,   0x02,       0x00,        0},
    {CMDF_SYSSTACK                            ,   0x00,       0x0C,        0},
    {CMDF_MEMPOOL                             ,   0x00,       0x68,        0},
    {CMDF_RESOURCE                            ,   0x00,       0x0C,        0},
    {CMDF_MEMWRITEREAD                        ,   0x09,       0x04,        0},
/*0x9/**********************************************************************************/
    {CMDF_DDP_SWRESET                         ,   0x01,       0x00,        0},
    {CMDF_DDPRESETCOUNT                       ,   0x00,       0x01,        0},
    {CMDF_DDPRESETCAUSE                       ,   0x00,       0x01,        0},
    {CMDF_USBCLOCK                            ,   0x01,       0x01,        0},
    {CMDF_SETGETDDPCLOCK                      ,   0x01,       0x01,        0},
    {CMDF_GETDDPCLOCKFREQ                     ,   0x01,       0x04,        0},
    {CMDF_DDPGETASICCONFIG                    ,   0x02,       0x18,        0},
    {CMDF_GETDDPASICINFO                      ,   0x00,       0x06,        0},
    {CMDF_3DSYNCINPUTSELECT                   ,   0x01,       0x01,        0},
    {CMDF_3DSYNCOUTPUTSELECT                  ,   0x01,       0x01,        0},
    {CMDF_SCALARBYPASSMODE                    ,   0x01,       0x01,        0},
    {CMDF_FRCBYPASSMODE                       ,   0x01,       0x01,        0},
    {CMDF_TEMPSENSOR                          ,   0x01,       0x01,        0},
	{CMDF_WAP_ALLSET_TRPDMD_ESTATUS           ,   0xCD,       0x01,        0},  //G50_Casper_0006  //HICC2_Steven_0019 //HICC2_Casper_0024 (mode: eCM_PICTURE_SETTINGS_NUMBER * seg : 6 * uint16 size : 2) + 1(level)
    {CMDF_APPCMD_DELAY                        ,   0x04,       0x00,        0},
    {CMDF_RX24TVERSION                        ,   0x01,       0x02,        0},  //H30K_Doulas_0007
/*0xA/**********************************************************************************/
    {CMDF_FANFREQCYCLE                        ,   0x04,       0x02,        0},
    {CMDF_FANPARALLEL                         ,   0x02,       0x01,        0},
    {CMDF_CWDIRECTION                         ,   0x01,       0x01,        0},
    {CMDF_CWDIRECTIONENABLE                   ,   0x00,       0x01,        0},
    {CMDF_CWCLOCK                             ,   0x01,       0x00,        0},
    {CMDF_CWBRAKE                             ,   0x01,       0x00,        0},
    {CMDF_CWTYPE                              ,   0x01,       0x01,        0},
    {CMDF_CWMOTORTYPE                         ,   0x01,       0x01,        0},
    {CMDF_DUALCWDELAY                         ,   0x03,       0x03,        0},		//G100_Doulas_0009 Modify
    {CMDF_DUALCWSTRENGTH                      ,   0x01,       0x01,        0},
    {CMDF_CWSEQUENCER                         ,   0x01,       0x01,        0},
    {CMDF_CWSPOKETEST                         ,   0x01,       0x01,        0},
    {CMDF_CWSPOKECONFIG                       ,   0x03,       0x03,        0},
    {CMDF_FANDEVICE                           ,   0x01,       0x04,        0},	//G100_Doulas_0010 Modify
    {CMDF_SET_GETDUALCWDELAY                  ,   0x01,       0x00,        0}, //ZU860_John_0014 fix wheel index command
    {CMDF_3DSYNCINPUT_MONITOR_FANNUMSELECT    ,   0x02,       0x03,        0},
/*0xB/**********************************************************************************/
    {CMDF_PWMOUTPUTCONFIG                     ,   0x08,       0x38,        0},
    {CMDF_PWMINPUTCONFIG                      ,   0x06,       0x14,        0},
    {CMDF_DMDLOCK                             ,   0x01,       0x01,        0},
    {CMDF_DISPOFFSET                          ,   0x04,       0x04,        0},     //ZU860_Doulas_0046
    {CMDF_COLOR_PROFILE                       ,   0x01,       0x01,        0},
    {CMDF_CW_INDEX_POLARITY                   ,   0x03,       0x03,        0},     //ZU860_Doulas_0046
    {CMDF_DISP_KEY_MINMAX_PIT                 ,   0x00,       0x02,        0},
    {CMDF_DISP_KEY_ENABLE                     ,   0x01,       0x01,        0},     //ZU860_Doulas_0046
    {CMDF_DISP_KEY_ANGLE                      ,   0x06,       0x06,        0},
    {CMDF_3D_FORMAT                           ,   0x01,       0x01,        0},
    {CMDF_LIMIT_BLACK_LEVEL                   ,   0x01,       0x01,        0},
    {CMDF_LIMIT_BLACK_TOLERANCE               ,   0x01,       0x01,        0},
    {CMDF_LIMIT_BLACK_STATUS                  ,   0x01,       0x01,        0},
    {CMDF_WAP                                 ,   0x05,       0x0e,        0},
    {CMDF_SEQREVISION                         ,   0x00,       0x04,        0},
    {CMDF_EMERGSHUTDOWN                       ,   0x01,       0x01,        0},
/*0xC/**********************************************************************************/
    {CMDF_SSIC1CURRENTS                       ,   0x14,       0x14,        0},
    {CMDF_SSIREDCURRENTS                      ,   0x14,       0x14,        0},
    {CMDF_SSIGRNCURRENTS                      ,   0x14,       0x14,        0},
    {CMDF_SSIBLUCURRENTS                      ,   0x14,       0x14,        0},
    {CMDF_SSIDRIVERSTATUS                     ,   0x00,       0x03,        1},
    {CMDF_SSIREDDRIVERTIMING                  ,   0x04,       0x04,        1},
    {CMDF_SSIGRNDRIVERTIMING                  ,   0x04,       0x04,        1},
    {CMDF_SSIBLUDRIVERTIMING                  ,   0x04,       0x04,        1},
    {CMDF_SSIEXTSENSORXY                      ,   0x1F,       0x1F,        0},
    {CMDF_SSISENSORCAL                        ,   0x15,       0x15,        0},
    {CMDF_SSISENSOR                           ,   0x00,       0x0A,        0},
    {CMDF_SSISENSORTIMING                     ,   0x0C,       0x0D,        0},
    {CMDF_CCIMODE                             ,   0x01,       0x01,        0},
    {CMDF_CCIENABLE                           ,   0x01,       0x01,        0},
    {CMDF_SSIPWMDRIVERLEVELS                  ,   0x0C,       0x0C,        0},
    {CMDF_SSIPWMDRIVERLEVELS2                 ,   0x0C,       0x0C,        0},
/*0xD/**********************************************************************************/
    {CMDF_PWMPARAMS                           ,   0x05,       0x04,        0},
    {CMDF_SSIMAX_CURRENT                      ,   0x0a,       0x0a,        0},    //ZU860_Doulas_0119 modify
    {CMDF_REALBLACK_LIGHTSOUTTIMER            ,   0x01,       0x00,        0},// #if(BIST_ENABLE)
    {CMDF_DIMPOWER                            ,   0x02,       0x14,        0},
    {CMDF_SSIDCSEQINDEX                       ,   0x02,       0x02,        0},
    {CMDF_SSIDCDUMPUART                       ,   0x00,       0x00,        0},
    {CMDF_SSIRAMPENABLE                       ,   0x02,       0x01,        0},
    {CMDF_SPICONFIG                           ,   0x11,       0x0F,        2},
    {CMDF_ALC_RGB_LEVEL                       ,   0x00,       0x0C,        0},
    {CMDF_4WAYXPR_WVFM                        ,   0x07,       0x07,        0},
    {CMDF_4WAYXPR_PARAM                       ,   0x06,       0x04,        2},
    {CMDF_4WAYXPR_WVFM_UPDATE_MODE            ,   0x01,       0x01,        0},  //ZU860_John_0024 add ABP command to DDP
    {CMDF_4WAYXPR_USER_UPDATE_WVFM            ,   0x01,       0x01,        0},	//HICC2_Doulas_0001
    {CMDF_4WAYXPR_ACT_INFO                    ,   0x00,       0x18,        0},
    {CMDF_SMOOTH_ON                           ,   0x02,       0x01,        0},	//G100_Clare_0004
    {CMDF_ABPINFO                             ,   0x09,       0x01,        0},			//G100_Doulas_0024
/*0xE*************************************************************************************/
    {CMDF_MEMORYTDUMP                         ,   0x06,       0x00,        0},
    {CMDF_GETMEMORYDUMP                       ,   0x08,       0xFF,        0},
    {CMDF_MEMBIST                             ,   0x0b,       0x00,        0},
    {CMDF_UARTCONFIG                          ,   0x0a,       0x0a,        0},
    {CMDF_GETUARTPORT                         ,   0x01,       0x02,        0},
    {CMDF_UARTRXDSOURCESELECT                 ,   0x02,       0x01,        0},
    {CMDF_AUTOUPDATE                          ,   0x01,       0x00,        0},
    {CMDF_I2CPASS                             ,   0x0b,       0x03,        0},
    {CMDF_GETI2CPASS                          ,   0x0a,       0x00,        0},
    {CMDF_IMAGERGBGAIN                        ,   0x06,       0x06,        0},
    {CMDF_EEPROMVER                           ,   0x83,       0x00,        0},
    {CMDF_DB_USE_PWM                          ,   0x14,       0x14,        0},
    {CMDF_BLACKSIGNALSWITCH                   ,   0x01,       0x00,        0},
    {CMDF_SEGMENTCOLOR                        ,   0x02,       0x02,        0},
    {CMDF_BCCALIBRATE                         ,   0x38,       0x00,        0},
    {CMDF_GAMMATABLE                          ,   0x01,       0x01,        0},
/*0xF/**********************************************************************************/
    {CMDF_FPGACTRL_WRRD                       ,   0x02,       0x08,        0},
    {CMDF_FPGAWRITE                           ,   0x08,       0x00,        0},
    {CMDF_FPGAREAD                            ,   0x00,       0x04,        0},
    {CMDF_FPGACTRL_SETGET                     ,   0x01,       0x00,        0},
    {CMDF_FPGAKEYANGLE                        ,   0x08,       0x00,        0}, //A70LK_Casper_0002
    {CMDF_RGBGAINADJUSTMENTS                  ,   0x12,       0x12,        0},
    {CMDF_APCCT                               ,   0x07,       0x0C,        0},  //ZU860_John_0018 add auto tuning of corrected color temperature
    {CMDF_SETCOLOROFFSET                      ,   0x01,       0x01,        0},
    {CMDF_IMG_ORIENTATION                     ,   0x01,       0x01,        0},     //ZU860_Doulas_0046
    {CMDF_COLOR_ENHANCEMENT                   ,   0x01,       0x00,        0},
    {CMDF_SRC_AUTODETECT                      ,   0x01,       0x00,        0},
    {CMDF_SEQVERSION                          ,   0x00,       0x04,        0},
    {CMDF_IMG_ALGORITHM                       ,   0x01,       0x01,        0},
    {CMDF_VX1_FREEZE_STAT                     ,   0x01,       0x01,        0},
    {CMDF_CMPSR_PASSTHRU                      ,   0x05,       0x40,        0},
    {CMDF_PARAMETER_SET                       ,   0x06,       0x00,        0},
};
#endif

// ==============================================================================
// FUNCTION NAME: dv442x_AsicReadySet
// DESCRIPTION:
//
//
// Params:
// BOOL bBnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/11, Casper Create
// --------------------
// ==============================================================================
void dv442x_AsicReadySet(BOOL bBnable)
{
    m_bAsicReady = bBnable;
}

BOOL dv442x_AsicReadGet(void) //HICC2_Doulas_0085
{
    return m_bAsicReady;
}

// ==============================================================================
// FUNCTION NAME: dv442x_UpgradeSet
// DESCRIPTION:
//
//
// Params:
// BOOL bBnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/03/23, Larry Create
// --------------------
// ==============================================================================
void dv442x_UpgradeSet(BOOL bBnable)
{
    m_bUpgrade = bBnable;
}

// ==============================================================================
// FUNCTION NAME: dv442x_AsicReadySet
// DESCRIPTION:
//
//
// Params:
// BOOL bBnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/30 A70LV_Eric.C_0025
// --------------------
// ==============================================================================
BOOL dvDDP4422_IsAsicReady(void)
{
    UINT16 uiCount = 500;
    UINT8  ucBusy = DDP_BUSY_LOW;
    BOOL b_m_bAsicReady = FALSE; //HICC2_Doulas_0059

    //if(m_bAsicReady == TRUE) //HICC2_Doulas_0059
    //{
    //    return TRUE;
    //}

    if (Board_Stage_Get() > EVT_STAGE)
    {
        do
        {
            if(GPIO_Read(DDP_BUSY_GPIO, &ucBusy) == GPIO_OK)
            {
                if(ucBusy == DDP_BUSY_LOW)
                {
                    b_m_bAsicReady = TRUE; //HICC2_Doulas_0059
                    return b_m_bAsicReady; //HICC2_Doulas_0059
                }
            }

            MS_SLEEP(1);
        }while(uiCount--);

        b_m_bAsicReady = FALSE; //HICC2_Doulas_0059
    }
    else
    {
        b_m_bAsicReady = TRUE; //HICC2_Doulas_0059
    }

    return b_m_bAsicReady; //HICC2_Doulas_0059
}

// ==============================================================================
// FUNCTION NAME: dv442x_Busy
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/11, Casper Create
// --------------------
// ==============================================================================
BOOL dv442x_Busy(void)
{
    UINT16 uiCount = 5000;//500;       //A70LV_Doulas_0186 Modify checking DDP busy timer to avoid lost command.
    UINT8  ucBusy = DDP_BUSY_LOW;


    if(m_bAsicReady == FALSE) //HICC2_Doulas_0041
    {
        return TRUE;
    }

    do
    {
        if(m_bUpgrade)
            return TRUE;

        if(GPIO_Read(DDP_BUSY_GPIO, &ucBusy) == GPIO_OK)
        {
            if(ucBusy == DDP_BUSY_LOW)
            {
                return FALSE;
            }
        }
        MS_SLEEP(1);
    }while(uiCount--);

    return TRUE;
}

// ==============================================================================
// FUNCTION NAME: dv442x_Read
// DESCRIPTION:
//
//
// Params:
// eDDP_CMD_ID eCmd:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/11, Casper Create
// --------------------
// ==============================================================================
eRESULT dv442x_Read_log(eDDP_CMD_ID eCmd, UINT8 *pucData, const char *cFuncName)    //A70LV_Doulas_0006 modify
{
    eRESULT	eResult = rcSUCCESS;
    UINT8	ucRetry = 0;
    BYTE eDevID = eCM_IF_DDP4422;

    if(Board_SingleBoard_Get() == FALSE)
    {
        if(DDP_MutexTake(cFuncName))
        {
            if((dv442x_Busy() == TRUE) || (eCmd > CMDF_PARAMETER_SET))
            {
                //LOG_MSG(db_ALWAYS, "R DDP I2C Fail Reg %02x\r\n", DDPCommandTable[eCmd].uiCmd);
                //ASSERT_ALWAYS();
                DDP_MutexGive();
                return rcERROR;
            }

            do
            {
				dvI2C_Dev_TotalCount(eDevID);

                eResult = Board_I2C_Master_Write(I2C_DDP_BUS,
                                                 I2C_DDP_ADDRESS,
                                                 DDPCommandTable[CMDF_I2CREAD].uiCmd,
                                                 DDPCommandTable[CMDF_I2CREAD].uiWriteNum,
                                                 (UINT8 *)&eCmd,
                                                 I2C_DDP_WRITE_FLAG);

                if(eResult != rcSUCCESS) //G100_Steven_0078
                {
                	dvI2C_Dev_RetryCount(eDevID);
                	utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_DDP4422);
                }
                //MS_SLEEP(1);
            }
            while((ucRetry++ < I2C_DDP_RETRY) && (eResult != rcSUCCESS));

            do
            {
                if(dv442x_Busy() == TRUE)
                {
                    LOG_MSG(db_ALWAYS, "R DDP I2C Fail Reg %02x\r\n", DDPCommandTable[eCmd].uiCmd);
                    ASSERT_ALWAYS();
                    DDP_MutexGive();
                    return rcERROR;
                }
				dvI2C_Dev_TotalCount(eDevID);

                eResult = Board_I2C_Master_Read(I2C_DDP_BUS,
                                                I2C_DDP_ADDRESS,
                                                DDPCommandTable[eCmd].uiCmd,
                                                DDPCommandTable[eCmd].ucReadNum,//uiSize,     //A70LV_Doulas_0006 modify
                                                pucData,
                                                I2C_DDP_READ_FLAG);

                if(eResult != rcSUCCESS) //G100_Steven_0078
                {
                	dvI2C_Dev_RetryCount(eDevID);
                	utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_DDP4422);
                }

                MS_SLEEP(1);
            }
            while((ucRetry++ < I2C_DDP_RETRY) && (eResult != rcSUCCESS));

            DDP_MutexGive();
        }
        else
        {
            LOG_MSG(db_ALWAYS, "(func:%s, line:%d) DDP Mutex Fail, Last handle %s\r\n", __FUNCTION__, __LINE__, acLastFuncName);
        }

        ASSERT(eResult == rcSUCCESS);
    }

    if(eResult != rcSUCCESS)
    {
    	dvI2C_Dev_ErrorCount(eDevID); //G100_Steven_0078
        utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_DDP4422);
        LOG_MSG(db_ALWAYS, "R DDP I2C Fail Reg %02x\r\n", DDPCommandTable[eCmd].uiCmd);
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: dv442x_Write
// DESCRIPTION:
//
//
// Params:
// eDDP_CMD_ID eCmd:
// WORD wSize:
// BYTE *pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/05/11, Casper Create
// --------------------
// ==============================================================================
eRESULT dv442x_Write_log(eDDP_CMD_ID eCmd, UINT8 *pucData, const char *cFuncName)
{
    eRESULT eResult = rcSUCCESS;
    UINT8    ucRetry = 0;
    BYTE eDevID = eCM_IF_DDP4422;

    if(Board_SingleBoard_Get() == FALSE)
    {
        if(DDP_MutexTake(cFuncName))
        {
            if(dv442x_Busy() == TRUE)
            {
                LOG_MSG(db_ALWAYS, "W DDP I2C Fail Reg %02x\r\n", DDPCommandTable[eCmd].uiCmd);
                ASSERT_ALWAYS();
                DDP_MutexGive();
                return rcERROR;
            }

            do
            {
				dvI2C_Dev_TotalCount(eDevID);

                eResult = Board_I2C_Master_Write(I2C_DDP_BUS,
                                                 I2C_DDP_ADDRESS,
                                                 DDPCommandTable[eCmd].uiCmd,
                                                 DDPCommandTable[eCmd].uiWriteNum,
                                                 pucData,
                                                 I2C_DDP_WRITE_FLAG);

    			if(eResult != rcSUCCESS) //G100_Steven_0078
    			{
    				dvI2C_Dev_RetryCount(eDevID);
                    utilOPD_I2C_Warn_Flag_Set(eDEVICE_ERROR_DDP4422);
    			}

                MS_SLEEP(1);
            }
            while((ucRetry++ < I2C_DDP_RETRY) && (eResult != rcSUCCESS));

            DDP_MutexGive();
        }
        else
        {
            LOG_MSG(db_ALWAYS, "(func:%s, line:%d) DDP Mutex Fail, Last handle %s\r\n", __FUNCTION__, __LINE__, acLastFuncName);
        }

        ASSERT(eResult == rcSUCCESS);
    }

    if(eResult != rcSUCCESS)
    {
		dvI2C_Dev_ErrorCount(eDevID); //G100_Steven_0078
        utilOPD_I2C_Error_Flag_Set(eDEVICE_ERROR_DDP4422);
        LOG_MSG(db_ALWAYS, "W DDP I2C Fail Reg %02x\r\n", DDPCommandTable[eCmd].uiCmd);
    }

    return eResult;

}

// ==============================================================================
// FUNCTION NAME: dv442x_BusyPin_Init
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dv442x_BusyPin_Init(void)
{
    DDP_MutexCreate();

    if(GPIO_Open(DDP_BUSY_GPIO, GPIO_IN) != GPIO_OK)
    {
        printf("DDP_BUSY_GPIO open fail\r\n");
        return rcERROR;
    }

    return rcSUCCESS;
}

#if 0
// ==============================================================================
// FUNCTION NAME: dv442x_Test
// DESCRIPTION:
//
//
// Params:
// void:
//
// Returns:
//
//
// Modification History
// --------------------
// 2019/11/19, Larry Create
// --------------------
// ==============================================================================
eRESULT dv442x_Test(void)
{
    UINT16 wVerDDPApp = 0;
    UINT32 dwVerDDPSq = 0;
    UINT8  ucDatax[42] = {0};
    eRESULT eResult = rcINVALID;

    {

        //ucDatax[0] = 0x07;
        //ucDatax[1] = 0x00;  // A70_Eddie_0004. DDP received two bytes, but we may not need so many system modes so fix high byte to 0x00.

        //eResult = dv442x_Write(CMDF_SYSTEMMODE, &ucDatax[0]);


#if 1

        eResult = dv442x_Read(CMDF_APPVERSIONS, &ucDatax[0]);
        ucDatax[2] = (ucDatax[2]/16)*10 + (ucDatax[2]%16);
        wVerDDPApp = (UINT16)(ucDatax[3]<<8) | (UINT16)ucDatax[2];

        sleep(1);


        eResult = dv442x_Read(CMDF_SEQVERSION, &ucDatax[0]);
        dwVerDDPSq = (UINT32)ucDatax[0]+ (UINT32)(ucDatax[1]<<8) +(UINT32)(ucDatax[2]<<16)+(UINT32)(ucDatax[3]<<24);

        sleep(1);

                    printf("DDP      F%02d.%02d(%c%c%02d%d)\n",
                (UINT8)(wVerDDPApp>>8),      //DDP442x       Fxx.xx(HDRxx)
                (UINT8)(wVerDDPApp),
                (UINT8)(dwVerDDPSq>>24),
                (UINT8)(dwVerDDPSq>>16),
                (UINT8)(dwVerDDPSq>>8),
                (UINT8)(dwVerDDPSq));
#endif /* 0 */


        sleep(1);
    }


    return rcSUCCESS;
}
#endif /* 0 */

