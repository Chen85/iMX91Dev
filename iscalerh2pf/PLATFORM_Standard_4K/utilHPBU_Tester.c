//---------------------------------------------------------------------------
//Module : HPBU_Tester.c / HPUB_Tester.h
//Pupose : Common mudule for Tester application in CPC production line.
//Author : James Chen 20130104
////Common_James_0258 // Add Tester Module code
//---------------------------------------------------------------------------
// A70_Eric.C_0147 update for A70

#include "utilHPBU_Tester.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "utilCommon.h"     //A70LV_Doulas_0138
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilCLICmdAPI.h"

#include "appDataMgr.h"
#include "appIllumination.h"  //G100_Steven_0085
#include "appSystem.h"
#include "appEnvironment.h"

#include "palMotorMgr.h"
#include "palFormatterMgr.h"

#include "halMCUCtrlAPI.h"
#include "halFanCtrlAPI.h"
#include "halMotorCtrlAPI.h"

#include "opdCtrlAPI.h"
#include "Board_Uart.h"
#include "Release.h"
#include "GEC_UserOption.h" //HICC2_Doulas_0054

#include "dvMotorBoard.h"


#ifdef UNITY_CLI
#include "UCLIAPI.h"
#endif
///////////////// callback function start ///////////////
sUtilHPBUTester_CALLBACK sUtilHPBUTester_Callback ;

#ifdef PALDATAMGR_ACCESS_WITHLOG
#define fpDataMgr_Data_AccessCb(a,b,c) fpDataMgr_Data_AccessCb(a,b,c,__FUNCTION__,__LINE__)
#endif

void utilHPBUTest_CLI_RegCallback(sUtilHPBUTester_CALLBACK fpCallback)
{
    sUtilHPBUTester_Callback = fpCallback;
}
////////////////// callback function end ////////////////


#define HPBUTEST_START_HEADER       '*'
#define HPBUTEST_END_HEADER_1       '\n'
#define HPBUTEST_END_HEADER_2       '\r'


static UINT8 m_ucHPBUTestPos = 0;
static UINT8 m_ucHPBUTestrBuffer[_TesterMaxItems + 1] = {'\0'};

#define SN_BUFFEF_LENGTH 24 //A70LH_Larry_0114
#define RTC_BUFFER_LEN 32
#define TIMEZONE_BUFFER_LEN 7
#define PCBA_LENGTH 30

static UINT8  m_ucSNbuf[SN_BUFFEF_LENGTH] = {'\0'};
static UINT8  m_ucRTCBuf[RTC_BUFFER_LEN] = {'\0'};
static UINT8  m_ucTimeZoneBuf[TIMEZONE_BUFFER_LEN] = {'\0'};
static UINT16 m_uiBurnInSetting[3];
static UINT16 m_uiLD_PwmSetting[6];
static UINT8  m_LS_T1T0_Value[3] = {0};			//G100_Doulas_0012 use [0] and [1]
static UINT8  m_LS_T1T0_Start_Value = 0;		//G100_Doulas_0012
static UINT16 m_uiLD_PowerDimPwmSetting[4];		//G50_Alan_0095

//static UINT8  m_3D_MODEL_ID = 0; //G100_Steven_0039
static UINT16 m_auiLS_TargetValue[eLD_SEQ_NUMBER] = {900,900,900,900};    //G100_Owen_0087 //G100_Owen_0115
static UINT8  m_ucPCBAbuf[PCBA_LENGTH] = {'\0'};

static UINT16 m_uiOPD_PERIOD = 0;  //HICC2_Steven_0030

//extern UINT8 palSystem_CustomIDGet(void);
//#define _FIRMWARE_SYSTME //G100_Steven_0002

//---------------------------------------------------------------------------
//never change
static unsigned char cTesterUartMode = 0;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//common code
//---------------------------------------------------------------------------
unsigned char *cpGetRs232OutBuf(void);
void TesterRs232Feedback(unsigned char *outbuf);
////////////////////////////////////////////////////////////////////////////////////////////
//must have this call back function for setup system to normal mode.
unsigned char cTester_NORS(HPBU_TEST_COM_DATA *data);	//This call back function for setup system to normal mode.
//call back function define area///////////////////////////////////////////////////////////
unsigned char cTester_SERS(HPBU_TEST_COM_DATA *data);	//Set serial number command
unsigned char cTester_SERG(HPBU_TEST_COM_DATA *data);	//Get serial number command
unsigned char cTester_MNCG(HPBU_TEST_COM_DATA *data);	//Get version command
unsigned char cTester_VERG(HPBU_TEST_COM_DATA *data);	//Get version command
unsigned char cTester_VEGG(HPBU_TEST_COM_DATA *data);	//Get version command	//G100_Clare_0069, add
unsigned char cTester_MACG(HPBU_TEST_COM_DATA *data);	//Get MAC address command
unsigned char cTester_VGAG(HPBU_TEST_COM_DATA *data);	//Get VGA calibration data command
unsigned char cTester_VGOG(HPBU_TEST_COM_DATA *data);	//Get VGA video calibration data command
unsigned char cTester_BURS(HPBU_TEST_COM_DATA *data);	//Set burn in data command
unsigned char cTester_BURG(HPBU_TEST_COM_DATA *data);	//Get burn in data command
unsigned char cTester_FPMG(HPBU_TEST_COM_DATA *data);	//Get Fan PWM
unsigned char cTester_FRMG(HPBU_TEST_COM_DATA *data);	//Get Fan RPM
unsigned char cTester_GSNG(HPBU_TEST_COM_DATA *data);	//Get G-Sensor
unsigned char cTester_CW2G(HPBU_TEST_COM_DATA *data);	//Get Color wheel index(2X) data command
unsigned char cTester_CW3G(HPBU_TEST_COM_DATA *data);	//Get Color wheel index(3X) data command
unsigned char cTester_NETS(HPBU_TEST_COM_DATA *data);	//Set Network reset command
unsigned char cTester_LDPS(HPBU_TEST_COM_DATA *data);	//Set LD PWM
unsigned char cTester_LDPG(HPBU_TEST_COM_DATA *data);	//Get LD PWM
unsigned char cTester_RBCS(HPBU_TEST_COM_DATA *data);	//Return barcode
unsigned char cTester_LDAV(HPBU_TEST_COM_DATA *data);	//Get LDA Voltage
unsigned char cTester_LDAC(HPBU_TEST_COM_DATA *data);	//Get LDA Current
unsigned char cTester_LDAT(HPBU_TEST_COM_DATA *data);	//Get LDA Temperature
unsigned char cTester_LDAH(HPBU_TEST_COM_DATA *data);	//Get LDA Timer
unsigned char cTester_LDAE(HPBU_TEST_COM_DATA *data);	//Set LDA Bank Enable
unsigned char cTester_LATC(HPBU_TEST_COM_DATA *data);	//Get LDA Temperature compensation(PWM Out)
unsigned char cTester_LDBV(HPBU_TEST_COM_DATA *data);	//Get LDB Voltage
unsigned char cTester_LDBC(HPBU_TEST_COM_DATA *data);	//Get LDB Current
unsigned char cTester_LDBT(HPBU_TEST_COM_DATA *data);	//Get LDB Temperature
unsigned char cTester_LDBH(HPBU_TEST_COM_DATA *data);	//Get LDB Timer
unsigned char cTester_LDBE(HPBU_TEST_COM_DATA *data);	//Set LDB Bank Enable
unsigned char cTester_LBTC(HPBU_TEST_COM_DATA *data);	//Get LDB Temperature compensation(PWM Out)
unsigned char cTester_LDCV(HPBU_TEST_COM_DATA *data);	//Get LDC Voltage
unsigned char cTester_LDCC(HPBU_TEST_COM_DATA *data);	//Get LDC Current
unsigned char cTester_LDCT(HPBU_TEST_COM_DATA *data);	//Get LDC Temperature
unsigned char cTester_LDCH(HPBU_TEST_COM_DATA *data);	//Get LDC Timer
unsigned char cTester_LDCE(HPBU_TEST_COM_DATA *data);	//Set LDC Bank Enable
unsigned char cTester_LCTC(HPBU_TEST_COM_DATA *data);	//Get LDC Temperature compensation(PWM Out)
unsigned char cTester_LDDV(HPBU_TEST_COM_DATA *data);	//Get LDD Voltage
unsigned char cTester_LDDC(HPBU_TEST_COM_DATA *data);	//Get LDD Current
unsigned char cTester_LDDT(HPBU_TEST_COM_DATA *data);	//Get LDD Temperature
unsigned char cTester_LDDH(HPBU_TEST_COM_DATA *data);	//Get LDD Hours
unsigned char cTester_LDDE(HPBU_TEST_COM_DATA *data);	//Set LDD Bank Enable
unsigned char cTester_LDTC(HPBU_TEST_COM_DATA *data);	//Get LDD Temperature compensation(PWM Out)
unsigned char cTester_PRMG(HPBU_TEST_COM_DATA *data);	//Get Pump RPM
unsigned char cTester_PPHG(HPBU_TEST_COM_DATA *data);	//Get Pump Hours
unsigned char cTester_TECV(HPBU_TEST_COM_DATA *data);	//Get Tec Voltage
unsigned char cTester_TECA(HPBU_TEST_COM_DATA *data);	//Get Tec Current
unsigned char cTester_TECS(HPBU_TEST_COM_DATA *data);	//TEC assembled inspection by tec Voltage & Current
unsigned char cTester_LSVG(HPBU_TEST_COM_DATA *data);	//Get Light Sensor
unsigned char cTester_TEPG(HPBU_TEST_COM_DATA *data);	//Get System thermal sensor
unsigned char cTester_FILG(HPBU_TEST_COM_DATA *data);	//Get Filter Type
unsigned char cTester_LSEG(HPBU_TEST_COM_DATA *data);	//Get Lens Enable command
unsigned char cTester_LSDG(HPBU_TEST_COM_DATA *data);	//Focus, Zoom and Backlash
unsigned char cTester_LSPG(HPBU_TEST_COM_DATA *data);   //Lens center position
unsigned char cTester_ABCS(HPBU_TEST_COM_DATA *data);	//Set Auto Brightness calibration
unsigned char cTester_TPHG(HPBU_TEST_COM_DATA *data);	//Get total projector hours
unsigned char cTester_CPRT(HPBU_TEST_COM_DATA *data);	//Get current projector run time
unsigned char cTester_MSGG(HPBU_TEST_COM_DATA *data);	//Error Check(error log, EDID, I2C...)
unsigned char cTester_NUMS(HPBU_TEST_COM_DATA *data);
unsigned char cTester_LSXG(HPBU_TEST_COM_DATA *data);
unsigned char cTester_LSGT(HPBU_TEST_COM_DATA *data);		//G100_Doulas_0012
unsigned char cTester_LSTT(HPBU_TEST_COM_DATA *data);		//G100_Doulas_0012
unsigned char cTester_PRES(HPBU_TEST_COM_DATA *data);	//Get Tec Current
unsigned char cTester_RTCS(HPBU_TEST_COM_DATA *data);   //G100_Owen_0052
unsigned char cTester_RTCG(HPBU_TEST_COM_DATA *data);   //G100_Owen_0053
unsigned char cTester_OPTE(HPBU_TEST_COM_DATA *data);   //G100_Owen_0068
unsigned char cTester_OPTA(HPBU_TEST_COM_DATA *data);   //G100_Owen_0068
unsigned char cTester_OPRA(HPBU_TEST_COM_DATA *data);	//A35G2_Coda_0110
unsigned char cTester_RRWO(HPBU_TEST_COM_DATA *data);   //A35G2_BRC_Casper_0092
unsigned char cTester_PLAT(HPBU_TEST_COM_DATA *data);	//A70Gen2_Julie_0007
unsigned char cTester_LSVS(HPBU_TEST_COM_DATA *data);	//A70Gen2_Doulas_0018
unsigned char cTester_LSCT(HPBU_TEST_COM_DATA *data);	//A70Gen2_Doulas_0018
unsigned char cTester_LSRG(HPBU_TEST_COM_DATA *data);	//A70Gen2_Doulas_0018
unsigned char cTester_LSGG(HPBU_TEST_COM_DATA *data);	//A70Gen2_Doulas_0018
unsigned char cTester_LSOG(HPBU_TEST_COM_DATA *data);	//A70Gen2_Doulas_0018
unsigned char cTester_SPI7(HPBU_TEST_COM_DATA *data);   //G100_Owen_0085
unsigned char cTester_SPI8(HPBU_TEST_COM_DATA *data);   //G100_Owen_0085
unsigned char cTester_USBT(HPBU_TEST_COM_DATA *data);	//A70Gen2_Julie_0048
unsigned char cTester_UIMX(HPBU_TEST_COM_DATA *data);	//A70Gen2_Julie_0048
unsigned char cTester_UEXT(HPBU_TEST_COM_DATA *data);	//A70Gen2_Julie_0048
unsigned char cTester_UDDP(HPBU_TEST_COM_DATA *data);	//A70Gen2_Julie_0048
unsigned char cTester_UKSZ(HPBU_TEST_COM_DATA *data);	//A70Gen2_Julie_0048
unsigned char cTester_OSDR(HPBU_TEST_COM_DATA *data); //HICC2_Doulas_0035
unsigned char cTester_CLAH(HPBU_TEST_COM_DATA *data); //HICC2_Doulas_0035
unsigned char cTester_LDPD(HPBU_TEST_COM_DATA *data);	//Set Bright Mode LD Dim Pwm //G50_Alan_0095
unsigned char cTester_LDPN(HPBU_TEST_COM_DATA *data);	//Set Bright Mode New LD Dim Pwm  //G50_Casper_0008
unsigned char cTester_LDPR(HPBU_TEST_COM_DATA *data);	//Reset WAP flag //G50_Casper_0008
unsigned char cTester_LECA(HPBU_TEST_COM_DATA *data); //Lens Calibration   //HICC2_Doulas_0056
unsigned char cTester_LSAI(HPBU_TEST_COM_DATA *data); //Lens ADC ID        //HICC2_Doulas_0056
unsigned char cTester_SSRH(HPBU_TEST_COM_DATA *data); //Humidity                //HICC2_Doulas_0064
unsigned char cTester_PWPS(HPBU_TEST_COM_DATA *data); //PW photo sensor     //HICC2_Doulas_0066
unsigned char cTester_FACT(HPBU_TEST_COM_DATA *data);
unsigned char cTester_HWVG(HPBU_TEST_COM_DATA *data);
unsigned char cTester_ACTU(HPBU_TEST_COM_DATA *data);
unsigned char cTester_CLSL(HPBU_TEST_COM_DATA *data);
unsigned char cTester_CLSN(HPBU_TEST_COM_DATA *data);

unsigned char cTester_BIST(HPBU_TEST_COM_DATA *data); //HICC2_Steven_0004

#ifdef CUSTOM_BARCO
unsigned char cTester_TEDD(HPBU_TEST_COM_DATA *data);   //G100_Steven_0080
unsigned char cTester_TERA(HPBU_TEST_COM_DATA *data);   //G100_Steven_0080
unsigned char cTester_TERB(HPBU_TEST_COM_DATA *data);   //G100_Steven_0080
unsigned char cTester_GATA(HPBU_TEST_COM_DATA *data);   // CYCLE 0  //G100_Steven_0085
unsigned char cTester_GATB(HPBU_TEST_COM_DATA *data);   // CYCLE 1
unsigned char cTester_GATC(HPBU_TEST_COM_DATA *data);   // CYCLE 2
unsigned char cTester_GATD(HPBU_TEST_COM_DATA *data);   // CYCLE 3
unsigned char cTester_GATE(HPBU_TEST_COM_DATA *data);   // CYCLE 4
unsigned char cTester_GATF(HPBU_TEST_COM_DATA *data);   // CYCLE 5
unsigned char cTester_GATG(HPBU_TEST_COM_DATA *data);   // CYCLE 6
unsigned char cTester_GATH(HPBU_TEST_COM_DATA *data);   // CYCLE 7
unsigned char cTester_GATI(HPBU_TEST_COM_DATA *data);   // CYCLE 8
unsigned char cTester_GATJ(HPBU_TEST_COM_DATA *data);   // CYCLE 9  //G100_Steven_0085
unsigned char cTester_WIXG(HPBU_TEST_COM_DATA *data);   //G100_Owen_0095 //A35G2_BRC_Casper_0048
#endif

#ifdef OE_JIG //A70LV_John_0164 add command for TEC JIG data transfer
unsigned char cTester_TECG(HPBU_TEST_COM_DATA *data); //Tec_Test_Data
#endif
unsigned char cTester_PCBA(HPBU_TEST_COM_DATA *data);
unsigned char cTester_LSPS(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0001
unsigned char cTester_LS2G(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0002
unsigned char cTester_LC2T(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0002
unsigned char cTester_LR2G(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0002
unsigned char cTester_LG2G(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0002
unsigned char cTester_LO2G(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0002
unsigned char cTester_LS2S(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0002
unsigned char cTester_LV2G(HPBU_TEST_COM_DATA *data); //HICC2_Jacky_0003
unsigned char cTester_LOGS(HPBU_TEST_COM_DATA *data); //HICC2_Doulas_0098 L1G sensor
unsigned char cTester_DMDS(HPBU_TEST_COM_DATA *data); //HICC2_Doulas_0098 DMD Humidity/Pressure sensor
unsigned char cTester_SHUM(HPBU_TEST_COM_DATA *data); //HICC2_Doulas_0106

unsigned char cTester_LSMV(HPBU_TEST_COM_DATA *data);  //HICC2_Steven_0027

unsigned char cTester_OPPS(HPBU_TEST_COM_DATA *data);   //HICC2_Steven_0030
unsigned char cTester_LSDT(HPBU_TEST_COM_DATA *data);
unsigned char cTester_TESP(HPBU_TEST_COM_DATA *data); //HICC2_Doulas_0126

unsigned char cTester_XPRE(HPBU_TEST_COM_DATA *data); //HICC2_Steven_0052
unsigned char cTester_XPRZ(HPBU_TEST_COM_DATA *data);


unsigned char cTester_LENV(HPBU_TEST_COM_DATA *data);
unsigned char cTester_O1SN(HPBU_TEST_COM_DATA *data);   //H30K_Tim_0011, add
unsigned char cTester_O2SN(HPBU_TEST_COM_DATA *data);   //H30K_Tim_0011, add
unsigned char cTester_URSW(HPBU_TEST_COM_DATA *data);


unsigned char cTester_LMCR(HPBU_TEST_COM_DATA *data);



//unsigned char cTester_TDMD(HPBU_TEST_COM_DATA *data);  //G100_Steven_0052
//unsigned char cTester_NTDM(HPBU_TEST_COM_DATA *data);  //G100_Steven_0052

//unsigned char cTester_DLDG(HPBU_TEST_COM_DATA *data); //The duration of the LD lighting  // A70LV_Eric.C_0016  //A70LV_Larry_0084 mask

////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UCLI_WRITABLE
#define UCLI_WRITABLE 0
#endif

#ifndef UCLI_READABLE
#define UCLI_READABLE 0
#endif

#ifndef UCLI_NONE
#define UCLI_NONE 0
#endif



//Lookup table define area.
static const  Rs232TesterCommLutTable sTable_TesterLut[] =
{
    {"NORS", cTester_NORS, UCLI_WRITABLE                   }, //This call back function for setup system to normal mode.
    //customer area ////////////////////////////////////////////////
    {"SERS", cTester_SERS, UCLI_WRITABLE                   },
    {"SERG", cTester_SERG, UCLI_READABLE                   },
    {"MNCG", cTester_MNCG, UCLI_READABLE                   },
    {"VERG", cTester_VERG, UCLI_READABLE                   },
    {"MACG", cTester_MACG, UCLI_READABLE                   },
    {"VGAG", cTester_VGAG, UCLI_READABLE                   },
    {"VGOG", cTester_VGOG, UCLI_READABLE                   },
    {"BURS", cTester_BURS, UCLI_WRITABLE                   },
    {"BURG", cTester_BURG, UCLI_READABLE                   },
    {"FPMG", cTester_FPMG, UCLI_READABLE                   },
    {"FRMG", cTester_FRMG, UCLI_READABLE                   },
    {"GSNG", cTester_GSNG, UCLI_READABLE                   },
    {"CW2G", cTester_CW2G, UCLI_READABLE                   },
    {"CW3G", cTester_CW3G, UCLI_READABLE                   },
    {"NETS", cTester_NETS, UCLI_WRITABLE                   },
    {"LDPS", cTester_LDPS, UCLI_WRITABLE                   },
    {"LDPG", cTester_LDPG, UCLI_READABLE                   },
    {"RBCS", cTester_RBCS, UCLI_READABLE                   },
    {"LDAV", cTester_LDAV, UCLI_READABLE                   },
    {"LDAC", cTester_LDAC, UCLI_READABLE                   },
    {"LDAT", cTester_LDAT, UCLI_READABLE                   },
    {"LDAH", cTester_LDAH, UCLI_READABLE                   },
    {"LDAE", cTester_LDAE, UCLI_NONE                       },
    {"LATC", cTester_LATC, UCLI_READABLE                   },
    {"LDBV", cTester_LDBV, UCLI_READABLE                   },
    {"LDBC", cTester_LDBC, UCLI_READABLE                   },
    {"LDBT", cTester_LDBT, UCLI_READABLE                   },
    {"LDBH", cTester_LDBH, UCLI_READABLE                   },
    {"LDBE", cTester_LDBE, UCLI_NONE                       },
    {"LBTC", cTester_LBTC, UCLI_READABLE                   },
    {"LDCV", cTester_LDCV, UCLI_READABLE                   },
    {"LDCC", cTester_LDCC, UCLI_READABLE                   },
    {"LDCT", cTester_LDCT, UCLI_READABLE                   },
    {"LDCH", cTester_LDCH, UCLI_READABLE                   },
    {"LDCE", cTester_LDCE, UCLI_NONE                       },
    {"LCTC", cTester_LCTC, UCLI_READABLE                   },
    {"LDDV", cTester_LDDV, UCLI_READABLE                   },
    {"LDDC", cTester_LDDC, UCLI_READABLE                   },
    {"LDDT", cTester_LDDT, UCLI_READABLE                   },
    {"LDDH", cTester_LDDH, UCLI_READABLE                   },
    {"LDDE", cTester_LDDE, UCLI_NONE                       },
    {"LDTC", cTester_LDTC, UCLI_READABLE                   },
    {"PRMG", cTester_PRMG, UCLI_READABLE                   },
    {"PPHG", cTester_PPHG, UCLI_READABLE                   },
    {"TECV", cTester_TECV, UCLI_READABLE                   },
    {"TECA", cTester_TECA, UCLI_READABLE                   },
	{"TECS", cTester_TECS, UCLI_READABLE                   },
    {"LSVG", cTester_LSVG, UCLI_READABLE                   },
    {"TEPG", cTester_TEPG, UCLI_READABLE                   },
    {"FILG", cTester_FILG, UCLI_READABLE                   },
    {"LSEG", cTester_LSEG, UCLI_READABLE                   },
    {"LSDG", cTester_LSDG, UCLI_READABLE                   },
    {"LSPG", cTester_LSPG, UCLI_READABLE                   },
    {"ABCS", cTester_ABCS, UCLI_NONE                       },
    {"TPHG", cTester_TPHG, UCLI_READABLE                   },
    {"CPRT", cTester_CPRT, UCLI_READABLE                   },
    {"MSGG", cTester_MSGG, UCLI_NONE                       },
    {"NUMS", cTester_NUMS, UCLI_READABLE                   },
    {"LSXG", cTester_LSXG, UCLI_NONE                       },
    {"LSGT", cTester_LSGT, UCLI_READABLE                   },		//G100_Doulas_0012
    {"LSTT", cTester_LSTT, UCLI_WRITABLE                   },		//G100_Doulas_0012
    {"PRES", cTester_PRES, UCLI_READABLE                   },
    {"RTCS", cTester_RTCS, UCLI_WRITABLE                   },
    {"RTCG", cTester_RTCG, UCLI_READABLE                   },
    {"OPTE", cTester_OPTE, UCLI_WRITABLE                   },
    {"OPTA", cTester_OPTA, UCLI_WRITABLE                   },
    {"VEGG", cTester_VEGG, UCLI_READABLE                   },	//G100_Clare_0069, add
    {"OPRA", cTester_OPRA, UCLI_WRITABLE                   },	//A35G2_Coda_0110
    {"RRWO", cTester_RRWO, UCLI_WRITABLE                   }, //A35G2_BRC_Casper_0092
	{"PLAT", cTester_PLAT, UCLI_WRITABLE                   },	//A70Gen2_Julie_0007
	{"LSVS", cTester_LSVS, UCLI_WRITABLE                   },	//A70Gen2_Doulas_0018
	{"LSCT", cTester_LSCT, UCLI_READABLE                   },	//A70Gen2_Doulas_0018
	{"LSRG", cTester_LSRG, UCLI_READABLE                   },	//A70Gen2_Doulas_0018
	{"LSGG", cTester_LSGG, UCLI_READABLE                   },	//A70Gen2_Doulas_0018
	{"LSOG", cTester_LSOG, UCLI_READABLE                   },	//A70Gen2_Doulas_0018
    {"SPI7", cTester_SPI7, UCLI_WRITABLE                   }, //G100_Owen_0085
    {"SPI8", cTester_SPI8, UCLI_WRITABLE                   }, //G100_Owen_0085
    {"USBT", cTester_USBT, UCLI_WRITABLE                   },
    {"UIMX", cTester_UIMX, UCLI_READABLE                   },
    {"UEXT", cTester_UEXT, UCLI_READABLE                   },
    {"UDDP", cTester_UDDP, UCLI_READABLE                   },
    {"UKSZ", cTester_UKSZ, UCLI_READABLE                   },
    {"OSDR", cTester_OSDR, UCLI_READABLE                   }, //HICC2_Doulas_0035
    {"CLAH", cTester_CLAH, UCLI_WRITABLE                   }, //HICC2_Doulas_0035
	{"LDPD", cTester_LDPD, UCLI_READABLE                   }, //G50_Alan_0095
	{"LDPN", cTester_LDPN, UCLI_READABLE                   }, //G50_Casper_0008
	{"LDPR", cTester_LDPR, UCLI_READABLE                   }, //G50_Casper_0008
	{"LECA", cTester_LECA, UCLI_READABLE                   }, // HICC2_Doulas_0056 for new lens calibration command
	{"LSAI", cTester_LSAI, UCLI_READABLE                   }, // HICC2_Doulas_0056 for new lens adc ID command
	{"SSRH", cTester_SSRH, UCLI_READABLE                   }, //HICC2_Doulas_0064
	{"PWPS", cTester_PWPS, UCLI_READABLE                   }, //HICC2_Doulas_0066

    {"FACT", cTester_FACT, UCLI_READABLE                   },
    {"HWVG", cTester_HWVG, UCLI_READABLE                   },
    {"ACTU", cTester_ACTU, UCLI_READABLE                   },
    {"CLSL", cTester_CLSL, UCLI_READABLE                   },
    {"CLSN", cTester_CLSN, UCLI_READABLE                   },

#ifdef CUSTOM_BARCO
	{"TEDD", cTester_TEDD, UCLI_READABLE                   }, //G100_Steven_0080
	{"TERA", cTester_TERA, UCLI_READABLE                   }, //G100_Steven_0080
	{"TERB", cTester_TERB, UCLI_READABLE                   }, //G100_Steven_0080
	{"GATA", cTester_GATA, UCLI_READABLE                   },  //G100_Steven_0085
	{"GATB", cTester_GATB, UCLI_READABLE                   },
	{"GATC", cTester_GATC, UCLI_READABLE                   },
	{"GATD", cTester_GATD, UCLI_READABLE                   },
	{"GATE", cTester_GATE, UCLI_READABLE                   },
	{"GATF", cTester_GATF, UCLI_READABLE                   },
	{"GATG", cTester_GATG, UCLI_READABLE                   },
	{"GATH", cTester_GATH, UCLI_READABLE                   },
	{"GATI", cTester_GATI, UCLI_READABLE                   },
	{"GATJ", cTester_GATJ, UCLI_READABLE                   },  //G100_Steven_0085
    {"WIXG", cTester_WIXG, UCLI_READABLE                   }, //G100_Owen_0095 //A35G2_BRC_Casper_0048
#endif

#ifdef OE_JIG //A70LV_John_0164 add command for TEC JIG data transfer
	{"TECG", cTester_TECG, UCLI_READABLE                   },
#endif
    {"PCBA", cTester_PCBA, UCLI_WRITABLE                   },
    {"LSPS", cTester_LSPS, UCLI_READABLE                   }, //HICC2_Jacky_0001
    {"LS2G", cTester_LS2G, UCLI_READABLE                   }, //HICC2_Jacky_0002
    {"LC2T", cTester_LC2T, UCLI_READABLE                   }, //HICC2_Jacky_0002
    {"LR2G", cTester_LR2G, UCLI_READABLE                   }, //HICC2_Jacky_0002
    {"LG2G", cTester_LG2G, UCLI_READABLE                   }, //HICC2_Jacky_0002
    {"LO2G", cTester_LO2G, UCLI_READABLE                   }, //HICC2_Jacky_0002
    {"LS2S", cTester_LS2S, UCLI_READABLE                   }, //HICC2_Jacky_0002
    {"LV2G", cTester_LV2G, UCLI_READABLE                   }, //HICC2_Jacky_0003
    {"LOGS", cTester_LOGS, UCLI_READABLE                   }, //HICC2_Doulas_0098 L1G sensor
    {"DMDS", cTester_DMDS, UCLI_READABLE                   }, //HICC2_Doulas_0098 DMD Humidity/Pressure sensor
    {"SHUM", cTester_SHUM, UCLI_READABLE                   }, //HICC2_Doulas_0106

	{"BIST", cTester_BIST, UCLI_READABLE                   },    //HICC2_Steven_0004
	{"LSMV", cTester_LSMV, UCLI_READABLE                   },  //HICC2_Steven_0027
	{"OPPS", cTester_OPPS, UCLI_READABLE                   },   //HICC2_Steven_0030
	{"LSDT", cTester_LSDT, UCLI_READABLE                   },
	{"TESP", cTester_TESP, UCLI_WRITABLE                   }, //HICC2_Doulas_0126

	{"XPRE", cTester_XPRE, UCLI_READABLE                   }, //HICC2_Steven_0052
	{"XPRZ", cTester_XPRZ, UCLI_READABLE                   }, //HICC2_Steven_0052
	{"LENV", cTester_LENV, UCLI_READABLE                   },
	{"O1SN", cTester_O1SN, UCLI_READABLE                   },   //H30K_Tim_0011, add
	{"O2SN", cTester_O2SN, UCLI_READABLE                   },   //H30K_Tim_0011, add
	{"URSW", cTester_URSW, UCLI_WRITABLE                   },
	{"LMCR", cTester_LMCR, UCLI_WRITABLE                   },


    //{"TDMD", cTester_TDMD},  //G100_Steven_0052
	//{"NTDM", cTester_NTDM},  //G100_Steven_0052

    //{"DLDG", cTester_DLDG}, // A70LV_Eric.C_0016

    //end of customer area
    /////////////////////////////////////////////////////////////
    {"FFFF", 0           , UCLI_NONE                       }, //Last item, must keep it at last item.
};

#define MAX_TESTLUT_SIZE sizeof(sTable_TesterLut)/sizeof(sTable_TesterLut[0])

////////////////////////////////////////////////////////////////////////////////////////////
//Name : cIsTesterMode()
//Description : check UART mode is Tester mode or not.
//Parameter : void
//Return :  1 : System is Tester mode on UART.
//             0 : System is nomal mode.
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
unsigned char cIsTesterMode(void)
{
    if(cTesterUartMode == 0x5a)
    {
        return 1;
    }

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
//Name : SetTesterMode()
//Description : Set UART mode to Tester mode.
//Parameter : void
//Return :  void
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
void SetTesterMode(void)
{
    // A70LV_Eric.C_0028 Start , Disable all debug message
    UINT64 ulMask = 0;

    utilDbgMsg_Set(db_ALL, 0);
    utilDbgMsgCT_Set(db_ALL, 0); //HICC2_Doulas_0131
    //appGui_DataCode_PointerVar_Set(edcDEBUG_MASK, (UINT8 *)&ulMask);
    // A70LV_Eric.C_0028 End

    cTesterUartMode = 0x5a;
}

////////////////////////////////////////////////////////////////////////////////////////////
//Name : ClearTesterMode()
//Description : Set UART mode to normal mode.
//Parameter : void
//Return :  void
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
void ClearTesterMode(void)
{
    cTesterUartMode = 0x00;
}
////////////////////////////////////////////////////////////////////////////////////////////
//Name : DecodeCommand()
//Description : decode a "Tester" command for HPBU_TEST_COM_DATA structure.
//Parameter :char *src --> Command string that come from PC.
//           HPBU_TEST_COM_DATA *dest : After decode the data will store in *dest structure.
//Return : void
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
void DecodeCommand(char *src, HPBU_TEST_COM_DATA *dest)
{
    unsigned char i;
    char tmp[6];

    //char acString[20];
    //decode command [3 bytes]
    for(i = 0; i < COMMAND_LENGTH; i++)
    {
        dest->chCommand[i] = src[i];
    }

    //acString[i+1] = '\0';

    //dvSerialWrite(SERIAL_DEFAULT_INSTANCE, 20, (PCBYTE)acString);

    //dest->chCommand[3] = '\0';

    i = COMMAND_LENGTH;

    //decode item type
    tmp[0] = src[i++]; //i = 4;
    tmp[1] = src[i++]; //i = 5;
    tmp[2] = '\0';
    dest->cType = atoi(tmp);

    //decode data size
    tmp[0] = src[i++]; //i = 6
    tmp[1] = src[i++]; //i = 7
    tmp[2] = '\0';
    dest->cDataSize = atoi(tmp);
    dest->pcData = (unsigned char *)(i + src); //i = 8

    //get barcode serial number
    //if((src[0]=='S')&&(src[1]=='E')&&(src[2]=='R'))
    if(0 == strncmp(dest->chCommand, "SERS", COMMAND_LENGTH))
    {
        for(i=0;i< dest->cDataSize;i++)
        {
            m_ucSNbuf[i] = src[COMMAND_DATA_START + i];
        }
        for(i= dest->cDataSize; i < SN_BUFFEF_LENGTH;i++)
        {
            m_ucSNbuf[i] = '\0';
        }
    }

    if(0 == strncmp(dest->chCommand, "NUMS", COMMAND_LENGTH))
    {
        for(i=0;i< dest->cDataSize;i++)
        {
            m_ucSNbuf[i] = src[COMMAND_DATA_START + i];
        }
        for(i= dest->cDataSize; i < SN_BUFFEF_LENGTH;i++)
        {
            m_ucSNbuf[i] = '\0';
        }
    }

    if(0 == strncmp(dest->chCommand, "RBCS", COMMAND_LENGTH))
    {
        for(i=0;i< dest->cDataSize;i++)
        {
            m_ucSNbuf[i] = src[COMMAND_DATA_START+i];
        }
        for(i= dest->cDataSize; i < SN_BUFFEF_LENGTH;i++)
        {
            m_ucSNbuf[i] = '\0';
        }
    }

    if(0 == strncmp(dest->chCommand, "OPPS", COMMAND_LENGTH))  //HICC2_Steven_0030
    {
        UINT8 j=0, TotalValue=0;

        tmp[0]= src[COMMAND_DATA_START];
        tmp[1]='\0';
        TotalValue = atoi(tmp);  //TotalValue:3

        for(i = 0; i < TotalValue; i++)
        {
            j = i*5;
            tmp[0]= src[(COMMAND_DATA_START + 1)+j];
            tmp[1]= src[(COMMAND_DATA_START + 2)+j];
            tmp[2]= src[(COMMAND_DATA_START + 3)+j];
            tmp[3]= src[(COMMAND_DATA_START + 4)+j];
            tmp[4]= src[(COMMAND_DATA_START + 5)+j];
            tmp[5]= '\0';
        }
        m_uiOPD_PERIOD = atoi(tmp);
    }

    //get burn in setting
    if(0 == strncmp(dest->chCommand, "BURS", COMMAND_LENGTH))
    {
        UINT8 j=0, TotalValue=0;

        tmp[0]= src[COMMAND_DATA_START];
        tmp[1]='\0';
        TotalValue = atoi(tmp);  //TotalValue:3

        for(i = 0; i < TotalValue; i++)
        {
            j = i*5;
            tmp[0]= src[(COMMAND_DATA_START + 1)+j];
            tmp[1]= src[(COMMAND_DATA_START + 2)+j];
            tmp[2]= src[(COMMAND_DATA_START + 3)+j];
            tmp[3]= src[(COMMAND_DATA_START + 4)+j];
            tmp[4]= src[(COMMAND_DATA_START + 5)+j];
            tmp[5]= '\0';
            m_uiBurnInSetting[i] = atoi(tmp);
        }
    }

	if(0 == strncmp(dest->chCommand, "LDPD", COMMAND_LENGTH) || 0 == strncmp(dest->chCommand, "LDPN", COMMAND_LENGTH))//G50_Alan_0095 //G50_Casper_0008
    {
        UINT8 j=0, TotalValue=0;

        tmp[0]= src[COMMAND_DATA_START];
        tmp[1]='\0';
        TotalValue = atoi(tmp);  //TotalValue:4

        for(i = 0; i < TotalValue; i++)
        {
            j = i*5;
            tmp[0]= src[(COMMAND_DATA_START + 1)+j];
            tmp[1]= src[(COMMAND_DATA_START + 2)+j];
            tmp[2]= src[(COMMAND_DATA_START + 3)+j];
            tmp[3]= src[(COMMAND_DATA_START + 4)+j];
            tmp[4]= src[(COMMAND_DATA_START + 5)+j];
            tmp[5]= '\0';
            m_uiLD_PowerDimPwmSetting[i] = atoi(tmp);
        }
    }

    if(0 == strncmp(dest->chCommand, "LDPS", COMMAND_LENGTH))
    {
        UINT8 j=0, TotalValue=0;

        tmp[0]= src[COMMAND_DATA_START];
        tmp[1]='\0';
        TotalValue = atoi(tmp);  //TotalValue:6

        for(i = 0; i < TotalValue; i++)
        {
            j = i*5;
            tmp[0]= src[(COMMAND_DATA_START + 1)+j];
            tmp[1]= src[(COMMAND_DATA_START + 2)+j];
            tmp[2]= src[(COMMAND_DATA_START + 3)+j];
            tmp[3]= src[(COMMAND_DATA_START + 4)+j];
            tmp[4]= src[(COMMAND_DATA_START + 5)+j];
            tmp[5]= '\0';
            m_uiLD_PwmSetting[i] = atoi(tmp);
        }
    }

	if(0 == strncmp(dest->chCommand, "LSTT", COMMAND_LENGTH))	//G100_Doulas_0012
    {
		UINT8 j=0, TotalValue=0;

        tmp[0]= src[COMMAND_DATA_START];
        tmp[1]='\0';
        TotalValue = atoi(tmp);  //TotalValue:2

        for(i = 0; i < TotalValue; i++)
        {
            j = i*5;
            tmp[0]= src[(COMMAND_DATA_START + 1)+j];
            tmp[1]= src[(COMMAND_DATA_START + 2)+j];
            tmp[2]= src[(COMMAND_DATA_START + 3)+j];
            tmp[3]= src[(COMMAND_DATA_START + 4)+j];
            tmp[4]= src[(COMMAND_DATA_START + 5)+j];
            tmp[5]= '\0';
            m_LS_T1T0_Value[i] = atoi(tmp);
        }
    }

    if(0 == strncmp(dest->chCommand, "RTCS", COMMAND_LENGTH))   //G100_Owen_0052
    {
        for(i=0; i < RTC_BUFFER_LEN; i++)
        {
            if(i < dest->cDataSize)
            {
                if(i < TIMEZONE_BUFFER_LEN)
                {
                    m_ucTimeZoneBuf[i] = src[COMMAND_DATA_START + i];
                    if((i+1) == TIMEZONE_BUFFER_LEN)
                    {
                        m_ucTimeZoneBuf[i] = '\0';
                    }
                }
                else
                {
                    m_ucRTCBuf[i - TIMEZONE_BUFFER_LEN] = src[COMMAND_DATA_START + i];
                }
            }
            else
            {
                m_ucRTCBuf[i] = '\0';
            }
        }
    }


	if(0 == strncmp(dest->chCommand, "LSVS", COMMAND_LENGTH))	//A70Gen2_Doulas_0018
    {
		UINT8 j=0, TotalValue=0;

        tmp[0]= src[COMMAND_DATA_START];
        tmp[1]='\0';
        TotalValue = atoi(tmp);  //TotalValue:4

        for(i = 0; i < TotalValue; i++)
        {
            j = i*5;
            tmp[0]= src[(COMMAND_DATA_START + 1)+j];
            tmp[1]= src[(COMMAND_DATA_START + 2)+j];
            tmp[2]= src[(COMMAND_DATA_START + 3)+j];
            tmp[3]= src[(COMMAND_DATA_START + 4)+j];
            tmp[4]= src[(COMMAND_DATA_START + 5)+j];
            tmp[5]= '\0';
            m_auiLS_TargetValue[i] = atoi(tmp);
        }
    }

    if(0 == strncmp(dest->chCommand, "PCBA", COMMAND_LENGTH))
    {
        for(i=0;i< dest->cDataSize;i++)
        {
            m_ucPCBAbuf[i] = src[COMMAND_DATA_START + i];
        }
        for(i= dest->cDataSize; i < PCBA_LENGTH;i++)
        {
            m_ucPCBAbuf[i] = '\0';
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
//Name : cTesterDecode()
//Description : decode all of "Tester" commands and encode data for feedback to PC.
//Parameter :char *cpComm --> Command string that come from PC.
//
//Return : unsigned char : 0 : decode command pass.
//                         1 : The call back function not exist.
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
unsigned char cTesterDecode(char *cpComm)
{
    unsigned char cData[_TesterMaxData];
    int i, len;
    unsigned char ret = 0;
    unsigned char temp[10];
    unsigned char *cpRs232outbuf;
    HPBU_TEST_COM_DATA sData;
    cpComm += 1;
    memset(&sData, 0, sizeof(HPBU_TEST_COM_DATA));

    //dbPrintf(dbUI,"C=%s",cpComm);
    //clear buffer.
    for(i = 0; i < _TesterMaxData; i++)
    {
        cData[i] = 0;
    }

    //decode command for HPBU_TEST_COM_DATA structure.
    DecodeCommand(cpComm, &sData);
    sData.pcData = &cData[0]; //alloc memory for data point.

    //found out the call back function and execut it.
    for(i = 0; i < MAX_TESTLUT_SIZE; i++)
    {
        if(0 == strncmp(sTable_TesterLut[i].cFuncCode, "FFFF", COMMAND_LENGTH))
        {
#ifdef _BCB
            Debug("not found function");
#endif
            return 1;// The call back function not exist.
            //break;
        }

        if(0 == strncmp(sTable_TesterLut[i].cFuncCode, sData.chCommand, COMMAND_LENGTH))
        {
            if(sTable_TesterLut[i].fpTester_Request != 0)
            {
                ret = sTable_TesterLut[i].fpTester_Request(&sData); //execute call back function.

                if(ret != 0)
                {
                    return ret;
                }

                break;
            }
        }
    }

    //pack data for feedback to PC.////////////////////////////
    len = 0;
    len = strlen((char *)cData);
    cpRs232outbuf = cpGetRs232OutBuf();

    for(i = 0; i < COMMAND_LENGTH; i++)
    {
        cpRs232outbuf[i] = sData.chCommand[i];
    }

    cpRs232outbuf[COMMAND_LENGTH] = '\0';
    sprintf((char *)temp, "%02d%02d", sData.cType, len);
    strcat((char *)cpRs232outbuf, (char *)temp);
    strcat((char *)cpRs232outbuf, (char *)sData.pcData);
    //pack data for feedback to PC.////////////////////////////
    TesterRs232Feedback(cpRs232outbuf);
    return 0; //decode command pass.
}


////////////////////////////////////////////////////////////////////////////////////////////
//Name : cpGetRs232OutBuf(void)
//Description : get ouuput buffer for RS-232
//Parameter : Void
//
//Return : unsigned char* -> Return point for output buffer(RS-232)
//                         1 : Fail.
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
unsigned char *cpGetRs232OutBuf(void)
{
    int i = 0;

    //clear buffer
    for(i = 0; i < 200; i++)
    {
        m_ucHPBUTestrBuffer[i] = 0;
    }

    return m_ucHPBUTestrBuffer; //return point for buffer
}

////////////////////////////////////////////////////////////////////////////////////////////
//Name : TesterRs232Feedback(void)
//Description : Send out data to PC
//Parameter : unsigned char *outbuf --> Point for store feedback data.
//
//Return : void
//
//Author : James Chen 20130104
////////////////////////////////////////////////////////////////////////////////////////////
void TesterRs232Feedback(unsigned char *outbuf)
{
    LOG_MSG(db_ALWAYS, "%s", outbuf);
}

// ==============================================================================
// FUNCTION NAME: cTester_NORS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_NORS(HPBU_TEST_COM_DATA *data)
{
    strcpy((char *)data->pcData, "Normal Mode\r\n");
    ClearTesterMode();
    return 0;
}

// ==============================================================================
// FUNCTION NAME: cTester_SERS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_SERS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

        return 0;//PASS.
#else

        // A70LV_Eric.C_0019 Start
        UINT8 cSN[SN_LENGTH] = {'\0'};

        if(strlen((char *)m_ucSNbuf) < SN_LENGTH)
        {
            sUtilHPBUTester_Callback.fpDataMgr_ServiceModeSetCb(eON);    //A70LV_Doulas_0138
            sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcSERIAL_NUMBER, edaWRITE_THROUGH_WITH_ACTION, (void *)m_ucSNbuf);
            sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcSERIAL_NUMBER, edaREAD, cSN);
            sprintf((char *)data->pcData, "%s\r\n", cSN);
        }
        // A70LV_Eric.C_0019 End


        return 0;//PASS.
#endif
}

#if 0  //G100_Steven_0052
//G100_Steven_0039
unsigned char cTester_TDMD(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

        return 0;//PASS.
#else

       palDataMgr_3DModelID_Set(0x55);

       sprintf((char *)data->pcData, "Pass\r\n");

       return 0;//PASS.
#endif
}

unsigned char cTester_NTDM(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

        return 0;//PASS.
#else

       palDataMgr_3DModelID_Set(0x56);
       sprintf((char *)data->pcData, "Pass\r\n");

       return 0;//PASS.
#endif
}
//G100_Steven_0039
#endif  //G100_Steven_0052
// ==============================================================================
// FUNCTION NAME: cTester_SERG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_SERG(HPBU_TEST_COM_DATA *data)
{

#ifdef _FIRMWARE_SYSTME

        return 0;//PASS.
#else
        // A70LV_Eric.C_0019 Start
        UINT8 cSN[SN_LENGTH] = {'\0'};

        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcSERIAL_NUMBER, edaREAD, cSN);
        sprintf((char *)data->pcData, "%s\r\n", cSN);
        // A70LV_Eric.C_0019 End

        return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_MNCG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_MNCG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

    return 0;  //A35G2_Simon_0070 for cppcheck

#else
    char cModelName[20] = {'\0'}; //A70LV_Larry_0132

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcMODEL_NAME, edaREAD, cModelName);

    // A70LV_Eric.C_0018 Start
    sprintf((char *)data->pcData, "%s\r\n", cModelName);
    // A70LV_Eric.C_0018 End

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_VERG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
//G100_Clare_0069, mod, >>>
unsigned char cTester_VERG(HPBU_TEST_COM_DATA *data) //HICC2_Doulas_0098
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else

#if defined(PLATFORM_R70K)   // HICC20_Keven_0001
    UINT8 aucReleaseVer[16] = {0};
    UINT8 aucFrontEndVer[16] = {0};
    UINT8 aucSTBVer[16] = {0};
    UINT8 aucMotorVer[16] = {0};
    UINT8 aucRX24TVer[16] = {0};
    UINT8 aucLANVer[16] = {0};
    UINT8 aucFormatMCUVer[16] = {0};
    UINT8 aucFPGA1Ver[16] = {0};
    UINT8 aucFPGA2Ver[16] = {0};
    UINT8 aucHDBASETVer[32] = {0};
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcRELEASE_VERSION, edaREAD, aucReleaseVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFRONTEND_VERSION, edaREAD, aucFrontEndVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFMCU_VERSION, edaREAD, aucSTBVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcMOTOR_VERSION, edaREAD, aucMotorVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcPMCU_VERSION, edaREAD, aucRX24TVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLAN_VERSION, edaREAD, aucLANVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFORMATER_VERSION, edaREAD, aucFormatMCUVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFPGA1_VERSION, edaREAD, aucFPGA1Ver);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFPGA2_VERSION, edaREAD, aucFPGA2Ver);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcHDBASET_VERSION, edaREAD, aucHDBASETVer);
    sprintf((char *)data->pcData, "%s-V%02d.%02d-%s-%s-%s-%s-%s-%s-%s-%s-%s\r\n",
        aucReleaseVer,
        VER_MAJOR,VER_MINOR,
        aucFrontEndVer,
        aucSTBVer,
        aucMotorVer,
        aucRX24TVer,
        aucLANVer,
        aucFormatMCUVer,
        aucFPGA1Ver,
        aucFPGA2Ver,
        aucHDBASETVer
        );
#elif defined(PLATFORM_H30_4K)
	UINT8 aucReleaseVer[16] = {0};
	UINT8 aucLANVer[16] = {0};
	UINT8 aucMotorVer[16] = {0};
	UINT8 aucFrontEndVer[16] = {0};
	UINT8 aucFPGA2Ver[16] = {0};

	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcRELEASE_VERSION, edaREAD, aucReleaseVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFRONTEND_VERSION, edaREAD, aucFrontEndVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcMOTOR_VERSION, edaREAD, aucMotorVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLAN_VERSION, edaREAD, aucLANVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFPGA3_VERSION, edaREAD, aucFPGA2Ver);

	sprintf((char *)data->pcData, "%s-%s-V%02d.%02d-%s-%s-%s\r\n",
		aucReleaseVer,
		aucFrontEndVer,
		VER_MAJOR, VER_MINOR,
		aucMotorVer,
		aucLANVer,
		aucFPGA2Ver
		);
#else
    UINT8 aucReleaseVer[16] = {0};
    UINT8 aucFormatMCUVer[16] = {0};
    UINT8 aucKeypadVer[16] = {0};
    UINT8 aucLANVer[16] = {0};
    UINT8 aucMotorVer[16] = {0};
    UINT8 aucFrontEndVer[16] = {0};
    UINT8 aucFPGA2Ver[16] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcRELEASE_VERSION, edaREAD, aucReleaseVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFRONTEND_VERSION, edaREAD, aucFrontEndVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcMOTOR_VERSION, edaREAD, aucMotorVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLD_DRIVER_VERSION, edaREAD, aucFormatMCUVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcKEYPAD_VERSION, edaREAD, aucKeypadVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLAN_VERSION, edaREAD, aucLANVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFPGA3_VERSION, edaREAD, aucFPGA2Ver);

    sprintf((char *)data->pcData, "%s-%s-V%02d.%02d-%s-%s-%s-%s-%s\r\n",
        aucReleaseVer,
        aucFrontEndVer,
        VER_MAJOR, VER_MINOR,
        aucMotorVer,
        aucFormatMCUVer,
        aucKeypadVer,
        aucLANVer,
        aucFPGA2Ver
        );
#endif
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSMV
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
//
// --------------------
// ==============================================================================
unsigned char cTester_LSMV(HPBU_TEST_COM_DATA *data)  //HICC2_Steven_0027
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else
    UINT8 aucVersion[2] = {0};

    palEnvironment_LENS_EEPROM_Version_Get(aucVersion);

    sprintf((char *)data->pcData, "LSM Ver.%02d.%02d\r\n",aucVersion[0], aucVersion[1]);

    return 0;//PASS.
#endif
}

unsigned char cTester_OPPS(HPBU_TEST_COM_DATA *data)   //HICC2_Steven_0030
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else

    if(m_uiOPD_PERIOD <= 600 && m_uiOPD_PERIOD >= 0)
	{
    	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcOPD_PERIOD,    edaWRITE_THROUGH_WITH_ACTION,  &m_uiOPD_PERIOD);
    	sprintf((char *)data->pcData,"%d\r\n", m_uiOPD_PERIOD);
    	return 0;//PASS.
	}
    else
    {
    	sprintf((char *)data->pcData,"Fail");
    	return 0;//PASS.
    }
#endif
}

//G100_Clare_0069, mod, <<<
//G100_Clare_0069, add, >>>
unsigned char cTester_VEGG(HPBU_TEST_COM_DATA *data) //HICC2_Doulas_0098
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else
#ifdef PLATFORM_H30_4K	//Optoma H30K
	UINT8 aucDDPVer[32] = {0};
	UINT8 aucXPRVer[12] = {0};
	UINT8 aucXPRVer_tmp[3] = {0};
	UINT8 aucHDBASETVer[32] = {0};
	UINT8 aucSTBVer[16] = {0};
	UINT8 aucRx24TVer[16] = {0};
	UINT8 aucGeometryVer[16] = {0};
	UINT8 aucHwVer[32] = {0};

	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFORMATER_VERSION, edaREAD, aucDDPVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcXPR_VERSION, edaREAD, aucXPRVer);
	sscanf(aucXPRVer, "%hhu.%hhu.%hhu", &aucXPRVer_tmp[0], &aucXPRVer_tmp[1], &aucXPRVer_tmp[2]);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcHDBASET_VERSION, edaREAD, aucHDBASETVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFMCU_VERSION, edaREAD, aucSTBVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcPMCU_VERSION, edaREAD, aucRx24TVer);
	sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcGEOMETRY_VERSION, edaREAD, aucGeometryVer);

	sprintf((char *)data->pcData, "%s-%d.%d.%d-%s-%s-%s-%s\r\n",

		aucDDPVer,
		aucXPRVer_tmp[0], aucXPRVer_tmp[1], aucXPRVer_tmp[2],
		aucHDBASETVer,
		aucSTBVer,
		//aucLdConvertVer,
		aucRx24TVer,
		aucGeometryVer
		//aucHwVer //A70LK_Coda_0014
		);
#else

    UINT8 aucDDPVer[32] = {0};
    UINT8 aucXFPGAVer[16] = {0};
    UINT8 aucHDBASETVer[32] = {0};
    UINT8 aucSTBVer[16] = {0};
    UINT8 aucRx24TVer[16] = {0};
    UINT8 aucGeometryVer[16] = {0};
    UINT8 aucHwVer[32] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFORMATER_VERSION, edaREAD, aucDDPVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcXFPGA_VERSION, edaREAD, aucXFPGAVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcHDBASET_VERSION, edaREAD, aucHDBASETVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFMCU_VERSION, edaREAD, aucSTBVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcPMCU_VERSION, edaREAD, aucRx24TVer);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcGEOMETRY_VERSION, edaREAD, aucGeometryVer);

    sprintf((char *)data->pcData, "%s-%s-%s-%s-%s-%s\r\n",

        aucDDPVer,
        aucXFPGAVer,
        aucHDBASETVer,
        aucSTBVer,
        //aucLdConvertVer,
        aucRx24TVer,
        aucGeometryVer
        //aucHwVer //A70LK_Coda_0014
        ); //H60_Doulas_0035
#endif
    return 0;//PASS.
#endif
}
//G100_Clare_0069, add, <<<
// ==============================================================================
// FUNCTION NAME: cTester_MACG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_MACG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else //A70LV_Larry_0084
    char cMAC[20] = {'\0'};
    UINT8 acMAC[6] = {0};
    UINT8 ucIndex = 0 ;
    char str[3];
    char *endptr;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLAN_MAC_ADDRESS, edaREAD, cMAC);

    for(ucIndex = 0; ucIndex < 6; ucIndex++)
    {
        str[0] = cMAC[ucIndex * 3 + 0];
        str[1] = cMAC[ucIndex * 3 + 1];
        str[2] = '\0';
        acMAC[ucIndex] = (int) strtol((const char *)str, &endptr, 16) ;
    }

    sprintf((char *)data->pcData, "%02X%02X%02X%02X%02X%02X\r\n",
    acMAC[0], acMAC[1], acMAC[2], acMAC[3], acMAC[4], acMAC[5]);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_VGAG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_VGAG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 uiADC_RGB_GAIN[3] = {0};
    UINT16 uiADC_RGB_OFFSET[3] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcADC_RGB_GAIN,     edaREAD, &uiADC_RGB_GAIN);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcADC_RGB_OFFSET,   edaREAD, &uiADC_RGB_OFFSET);


    sprintf((char *)data->pcData, "6,%d,%d,%d,%d,%d,%d\r\n",
    //sprintf((char *)data->pcData, "%d,%d,%d,%d,%d,%d\r\n",
    uiADC_RGB_GAIN[0],
    uiADC_RGB_GAIN[1],
    uiADC_RGB_GAIN[2],
    uiADC_RGB_OFFSET[0],
    uiADC_RGB_OFFSET[1],
    uiADC_RGB_OFFSET[2]);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_VGOG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_VGOG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT16 uiADC_YUV_GAIN[3] = {0};
    UINT16 uiADC_YUV_OFFSET[3] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcADC_YUV_GAIN,     edaREAD, &uiADC_YUV_GAIN);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcADC_YUV_OFFSET,   edaREAD, &uiADC_YUV_OFFSET);

    sprintf((char *)data->pcData, "6,%d,%d,%d,%d,%d,%d\r\n",
    uiADC_YUV_GAIN[0],
    uiADC_YUV_GAIN[1],
    uiADC_YUV_GAIN[2],
    uiADC_YUV_OFFSET[0],
    uiADC_YUV_OFFSET[1],
    uiADC_YUV_OFFSET[2]);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_BURS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_BURS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

        return 0;  //A35G2_Simon_0070 for cppcheck

#else //A70LV_Larry_0084
        if(m_uiBurnInSetting[0] <= 240 && m_uiBurnInSetting[1] <= 240 && m_uiBurnInSetting[2] <= 999)
        {
            UINT8  ucData  = 0;
            UINT16 uiCycle = 0;

            ucData = (UINT8)m_uiBurnInSetting[0];
            sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_ON,        edaWRITE_THROUGH_WITH_ACTION, &ucData);

            ucData = (UINT8)m_uiBurnInSetting[1];
            sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_OFF,       edaWRITE_THROUGH_WITH_ACTION, &ucData);

            uiCycle = (UINT16)m_uiBurnInSetting[2];
            sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_CYCLE,     edaWRITE_THROUGH_WITH_ACTION, &uiCycle);

            ucData = 1;
            sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_ENABLE,    edaWRITE_THROUGH_WITH_ACTION, &ucData);

            sprintf((char *)data->pcData,"%d,%d,%d\r\n", m_uiBurnInSetting[0], m_uiBurnInSetting[1], m_uiBurnInSetting[2]);
            return 0;//PASS.
        }


        sprintf((char *)data->pcData,"Fail");
        return 0;//PASS.
#endif

}

// ==============================================================================
// FUNCTION NAME: cTester_BURG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/24, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_BURG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

    return 0;  //A35G2_Simon_0070 for cppcheck

#else

    UINT8 ucBurninOn = 0;
    UINT8 ucBurninOff = 0;
    UINT8 ucBurninCycle = 0;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_ON,        edaREAD, &ucBurninOn);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_OFF,       edaREAD, &ucBurninOff);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_CYCLE,     edaREAD, &ucBurninCycle);

    //ucData = 1;
    //sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBURNIN_ENABLE,    edaWRITE_THROUGH_WITH_ACTION, &ucData);

    sprintf((char *)data->pcData,"%d,%d,%d\r\n", ucBurninOn, ucBurninOff, ucBurninCycle);
    return 0;//PASS.


#endif

}

// ==============================================================================
// FUNCTION NAME: cTester_FPMG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_FPMG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT8 FanNum = Syscfg_Value_Get_Typeint(eFan_INFO_Num_defined_in_MCU);

    // A70LV_Eric.C_0004 Start
    UINT16 *auiFanDuty = malloc(FanNum*sizeof(UINT16));
    if(auiFanDuty == NULL)
       return 0;
    memset(auiFanDuty, 0, FanNum*sizeof(UINT16));

    UINT8 ucCount = 0;
    for(ucCount=0; ucCount<FanNum; ucCount++)
    {
        auiFanDuty[ucCount] = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_FAN_DutyCb(ucCount);
//        LOG_MSG(db_ALWAYS, "\r\n Fan_Duty          [%d] 0x%X 0x%X",ucCount, aucData[0],aucData[1]);

        char string[8] = {'\0'};
        snprintf(string, 8, "%d", auiFanDuty[ucCount]);
        strcat((char *)data->pcData, string);

        if(ucCount+1 >= FanNum)
        {
        	char comma_str[5] = "\r\n";		//HICC2_Doulas_0016
            strcat((char *)data->pcData, comma_str); //HICC2_Doulas_0016
            break;
        }
        else
        {
            char comma_str[2] = ",";
            strcat((char *)data->pcData, comma_str);
        }
    }

    free(auiFanDuty);
    auiFanDuty = NULL;

    #if 0
    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        auiFanDuty[0],auiFanDuty[1],auiFanDuty[2],auiFanDuty[3],auiFanDuty[4],
        auiFanDuty[5],auiFanDuty[6],auiFanDuty[7],auiFanDuty[8],auiFanDuty[9],
        auiFanDuty[10],auiFanDuty[11]);
		//,auiFanDuty[12],auiFanDuty[13],auiFanDuty[14],auiFanDuty[15]);
    #endif
    // A70LV_Eric.C_0004 End

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_FRMG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_FRMG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT8 FanNum = Syscfg_Value_Get_Typeint(eFan_INFO_Num_defined_in_MCU);

    // A70LV_Eric.C_0004 Start
    UINT16 *auiFanSpeed = malloc(FanNum*sizeof(UINT16));
    if(auiFanSpeed == NULL)
       return 0;
    memset(auiFanSpeed, 0, FanNum*sizeof(UINT16));

    UINT8 ucCount = 0;
    for(ucCount=0; ucCount<FanNum; ucCount++)
    {
    	auiFanSpeed[ucCount] = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_FAN_RPMCb(ucCount);
//        LOG_MSG(db_ALWAYS, "\r\n Fan_Duty          [%d] 0x%X 0x%X",ucCount, aucData[0],aucData[1]);

        char string[8] = {'\0'};
        snprintf(string, 8, "%d", auiFanSpeed[ucCount]);
        strcat((char *)data->pcData, string);

        if(ucCount+1 >= FanNum)
        {
        	char comma_str[5] = "\r\n";		//HICC2_Doulas_0016
            strcat((char *)data->pcData, comma_str); //HICC2_Doulas_0016
            break;
        }
        else
        {
            char comma_str[2] = ",";
            strcat((char *)data->pcData, comma_str);
        }

    }

    free(auiFanSpeed);
    auiFanSpeed = NULL;

    #if 0
    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
    		auiFanSpeed[0],auiFanSpeed[1],auiFanSpeed[2],auiFanSpeed[3],auiFanSpeed[4],
			auiFanSpeed[5],auiFanSpeed[6],auiFanSpeed[7],auiFanSpeed[8],auiFanSpeed[9],
			auiFanSpeed[10],auiFanSpeed[11]);
			//,auiFanSpeed[12],auiFanSpeed[13],auiFanSpeed[14],auiFanSpeed[15]);
    #endif

    return 0;//PASS.
#endif
}

#ifdef CUSTOM_BARCO
//G100_Steven_0080 start
unsigned char cTester_TEDD(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

	UINT16 eTEC_Tester[eTEC_Data_MaxList]={0,0,0,0,0,0}; //G100_Steven_0091

    halMCUCtrl_TEC_TESTER_DMD_Get((BYTE*)&eTEC_Tester);

    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d\r\n", eTEC_Tester[eTEC_Data_DMD_ENVIR_T],eTEC_Tester[eTEC_Data_DMD_V1],eTEC_Tester[eTEC_Data_DMD_V60],eTEC_Tester[eTEC_Data_DMD_T0],
    		eTEC_Tester[eTEC_Data_DMD_dT], eTEC_Tester[eTEC_Data_DMD_I]);

    return 0;//PASS.
#endif
}

unsigned char cTester_TERA(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Tester[eTEC_Data_MaxList]={0,0,0,0,0,0}; //G100_Steven_0091

    halMCUCtrl_TEC_TESTER_RLD1_Get((UINT8*)&eTEC_Tester);

    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d\r\n", eTEC_Tester[eTEC_Data_DMD_ENVIR_T],eTEC_Tester[eTEC_Data_DMD_V1],eTEC_Tester[eTEC_Data_DMD_V60],eTEC_Tester[eTEC_Data_DMD_T0],
    		eTEC_Tester[eTEC_Data_DMD_dT], eTEC_Tester[eTEC_Data_DMD_I]);

    return 0;//PASS.
#endif
}

unsigned char cTester_TERB(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Tester[eTEC_Data_MaxList]={0,0,0,0,0,0}; //G100_Steven_0091

    halMCUCtrl_TEC_TESTER_RLD2_Get((UINT8*)&eTEC_Tester);

    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d\r\n", eTEC_Tester[eTEC_Data_DMD_ENVIR_T],eTEC_Tester[eTEC_Data_DMD_V1],eTEC_Tester[eTEC_Data_DMD_V60],eTEC_Tester[eTEC_Data_DMD_T0],
    		eTEC_Tester[eTEC_Data_DMD_dT], eTEC_Tester[eTEC_Data_DMD_I]);

    return 0;//PASS.
#endif
}
//G100_Steven_0080 end

 //G100_Steven_0085 start
unsigned char cTester_GATA(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(0);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATB(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(1);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(2);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATD(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(3);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATE(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(4);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATF(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(5);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(6);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATH(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(7);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATI(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(8);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}

unsigned char cTester_GATJ(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 eTEC_Result = 0;

    eTEC_Result = sUtilHPBUTester_Callback.fpIllumination_TEC_Gating_ResultGetCb(9);

    sprintf((char *)data->pcData,"%d\r\n", eTEC_Result);

    return 0;//PASS.
#endif
}
 //G100_Steven_0085 end
#endif

// ==============================================================================
// FUNCTION NAME: cTester_GSNG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_GSNG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "1,0,1,90,40,30\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_CW2G
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_CW2G(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    #if 0
    // A70LV_Eric.C_0020 Start
    UINT16 iCWIndex[2] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcWHEEL_INDEX_2X, edaREAD, iCWIndex);

    sprintf((char *)data->pcData, "%d,%d\r\n",iCWIndex[0],iCWIndex[1]);//PW,FW
    // A70LV_Eric.C_0020 End
    #endif

    //x35 gen2 only 3x pw and fw
    INT16 uiPW_Index = 0 ;
    INT16 uiFW_Index = 0 ;

    //Phosphor Wheel Index get
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcPHOSPHOR_WHEEL_INDEX, edaREAD,    &uiPW_Index) ;

    //Filter Wheel Index
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFILTER_WHEEL_INDEX,   edaREAD,    &uiFW_Index) ;

    sprintf((char *)data->pcData, "%d,%d\r\n",uiPW_Index,uiFW_Index);//PW,FW

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_CW3G
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_CW3G(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    #if 0
    // A70LV_Eric.C_0020 Start
    UINT16 iCWIndex[2] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcWHEEL_INDEX_3X, edaREAD, iCWIndex);

    sprintf((char *)data->pcData, "%d,%d\r\n",iCWIndex[0],iCWIndex[1]);//PW,FW
    // A70LV_Eric.C_0020 End
    #endif
    //x35 gen2 only 3x pw and fw
    INT16 uiPW_Index = 0 ;
    INT16 uiFW_Index = 0 ;

    //Phosphor Wheel Index get
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcPHOSPHOR_WHEEL_INDEX, edaREAD,    &uiPW_Index) ;

    //Filter Wheel Index
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFILTER_WHEEL_INDEX,   edaREAD,    &uiFW_Index) ;

    sprintf((char *)data->pcData, "%d,%d\r\n",uiPW_Index,uiFW_Index);//PW,FW

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_NETS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_NETS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT8 ucData = 1;
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcNETWORK_CONTROL_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucData);    //G100_Simon_0018
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcSNMP_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucData);                   //G100_Tim_0019, add //A35G2_BRC_Casper_0047
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcNETWORK_FACTORY_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucData);

    strcpy((char *)data->pcData,"Done\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDPD
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2023/05/12, Alan Create
// --------------------
// ==============================================================================
unsigned char cTester_LDPD(HPBU_TEST_COM_DATA *data)//G50_Alan_0095
{
#ifdef _FIRMWARE_SYSTME

	return 0;  //A35G2_Simon_0070 for cppcheck

#else //A70LV_Larry_0084
	if(m_uiLD_PowerDimPwmSetting[0] <= 100 && m_uiLD_PowerDimPwmSetting[1] <= 100 && m_uiLD_PowerDimPwmSetting[2] <= 100 && m_uiLD_PowerDimPwmSetting[3] <= 100)
	{
		UINT8 i;
		UINT32 	ulVal = 0;

		GEC_APCCT_Process_HPBU_Tester(0XFF, 0, 0); //WAP Enable Setting
		vTaskDelay(100);

		for(i=0; i<4; i++)
		{
			ulVal = (UINT32)m_uiLD_PowerDimPwmSetting[i];
			GEC_APCCT_Process_HPBU_Tester(1, i, ulVal);
			vTaskDelay(100);
		}

		GEC_APCCT_Process_HPBU_Tester(0XED, 0, 0); //WAP Disable Setting
		//vTaskDelay(2000); //G50_Casper_0008

		sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n", m_uiLD_PowerDimPwmSetting[0], m_uiLD_PowerDimPwmSetting[1], m_uiLD_PowerDimPwmSetting[2], m_uiLD_PowerDimPwmSetting[3]);
		return 0;//PASS.
	}

	sprintf((char *)data->pcData,"Fail");
	return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDPN
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2023/05/18, Casper Create
// --------------------
// ==============================================================================
unsigned char cTester_LDPN(HPBU_TEST_COM_DATA *data) //G50_Casper_0008
{
#ifdef _FIRMWARE_SYSTME

	return 0;  //A35G2_Simon_0070 for cppcheck

#else //A70LV_Larry_0084
	if(m_uiLD_PowerDimPwmSetting[0] <= 1023 && m_uiLD_PowerDimPwmSetting[1] <= 1023 && m_uiLD_PowerDimPwmSetting[2] <= 1023 && m_uiLD_PowerDimPwmSetting[3] <= 1023)
	{
		UINT8 i;
		UINT32 	ulVal = 0;

		GEC_APCCT_Process_HPBU_Tester(0XFF, 0, 0); //WAP Reset default
		vTaskDelay(100);

		for(i=0; i<4; i++)
		{
			ulVal = (UINT32)m_uiLD_PowerDimPwmSetting[i];
			GEC_APCCT_Process_HPBU_Tester(0x21, i, ulVal);
			vTaskDelay(100);
		}

		GEC_APCCT_Process_HPBU_Tester(0XEF, 0, 0); //WAP Apply
		//vTaskDelay(4000);

		sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n", m_uiLD_PowerDimPwmSetting[0], m_uiLD_PowerDimPwmSetting[1], m_uiLD_PowerDimPwmSetting[2], m_uiLD_PowerDimPwmSetting[3]);
		return 0;//PASS.
	}

	sprintf((char *)data->pcData,"Fail");
	return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDPR
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2023/05/18, Casper Create
// --------------------
// ==============================================================================
unsigned char cTester_LDPR(HPBU_TEST_COM_DATA *data) //G50_Casper_0008
{
#ifdef _FIRMWARE_SYSTME

	return 0;  //A35G2_Simon_0070 for cppcheck

#else //A70LV_Larry_0084

	GEC_APCCT_Process_HPBU_Tester(0XFF, 0, 0); //WAP Reset default
	vTaskDelay(100);

	GEC_APCCT_Process_HPBU_Tester(0XEF, 0, 0); //WAP Apply
	vTaskDelay(4000);

	GEC_APCCT_Process_HPBU_Tester(0XFA, 0, 0); //WAP Flag Off

    strcpy((char *)data->pcData,"Done\r\n");
	return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDPS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDPS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else //A70LV_Larry_0084

    palFormatterMgr_PWM_Set(0, m_uiLD_PwmSetting[0]);
    palFormatterMgr_PWM_Set(1, m_uiLD_PwmSetting[1]);
    palFormatterMgr_PWM_Set(2, m_uiLD_PwmSetting[2]);
    palFormatterMgr_PWM_Set(3, m_uiLD_PwmSetting[3]);
    palFormatterMgr_PWM_Set(4, m_uiLD_PwmSetting[4]);
    palFormatterMgr_PWM_Set(5, m_uiLD_PwmSetting[5]);

    sprintf((char *)data->pcData,  "%d,%d,%d,%d,%d,%d\r\n",
        m_uiLD_PwmSetting[0],
        m_uiLD_PwmSetting[1],
        m_uiLD_PwmSetting[2],
        m_uiLD_PwmSetting[3],
        m_uiLD_PwmSetting[4],
        m_uiLD_PwmSetting[5]
        );

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDPG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDPG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    palFormatterMgr_PWM_Update(); //A70LV_Larry_0084

    sprintf((char *)data->pcData,  "%d,%d,%d,%d,%d,%d\r\n",
    palFormatterMgr_PWM_Get(0),
    palFormatterMgr_PWM_Get(1),
    palFormatterMgr_PWM_Get(2),
    palFormatterMgr_PWM_Get(3),
    palFormatterMgr_PWM_Get(4),
    palFormatterMgr_PWM_Get(5)
    );

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_RBCS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_RBCS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    sprintf((char *)data->pcData, "%s", "T30100001\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDAV
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDAV(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    //A70LV_Larry_0084
    UINT8 aucData[eFMT_MSG_LD_VOLTAGE_SZ]={0};
    UINT16 auiVoltage[14]={0};
    UINT8 ucCount = 0;
    UINT16 uiAverage = 0;

    for(ucCount=0; ucCount<14; ucCount++)
    {
        halLDCtrl_LD_Voltage_Get(eLDBANK_A70LV, aucData);
        auiVoltage[aucData[0]] = ((UINT16)aucData[2] << 8) | (UINT16)aucData[1];
    }

    uiAverage = (auiVoltage[0]+auiVoltage[1]+auiVoltage[2]+auiVoltage[3])/4;

    /*
    LD number:
      0~3:  RLD1~RLD4
      4~8:  BLD1_1~BLD1_5
      9~13: BLD2_1~BLD2_5
    */
    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d\r\n",
        auiVoltage[0],auiVoltage[1],auiVoltage[2],auiVoltage[3], uiAverage);
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDAC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDAC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    // A70LV_Eric.C_0006 Start , RLD
    UINT8 aucData[eFMT_MSG_LD_CURRENT_SZ]={0};
    UINT16 auiCurrent[14]={0};
    UINT8 ucCount = 0;
    UINT16 uiAverage = 0;

    for(ucCount=0; ucCount<14; ucCount++)
    {
        halLDCtrl_LD_Current_Get(eLDBANK_A70LV, aucData);
        auiCurrent[aucData[0]] = ((UINT16)aucData[2] << 8) | (UINT16)aucData[1];
    }

    uiAverage = (auiCurrent[0]+auiCurrent[1]+auiCurrent[2]+auiCurrent[3])/4;

    /*
    LD number:
      0~3:  RLD1~RLD4
      4~8:  BLD1_1~BLD1_5
      9~13: BLD2_1~BLD2_5
    */
    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d\r\n",
        auiCurrent[0],auiCurrent[1],auiCurrent[2],auiCurrent[3], uiAverage);
    // A70LV_Eric.C_0006 End

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDAT
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDAT(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else
    // A70LV_Eric.C_0006 Start , BLD1
    UINT8 aucData[eFMT_MSG_LD_TEMPERATURE_SZ]={0};
    INT16 aiTemperature[14]={0};
    UINT8 ucCount = 0;
    UINT16 uiAverage = 0;

    for(ucCount=0; ucCount<14; ucCount++)
    {
        halLDCtrl_LD_Temperature_Get(eLDBANK_A70LV, aucData);
        aiTemperature[aucData[0]] = ((UINT16)aucData[2] << 8) | (UINT16)aucData[1];
    }

    uiAverage = (aiTemperature[0]+aiTemperature[1]+aiTemperature[2]+aiTemperature[3])/4;

    /*
    LD number:
      0~3:  RLD1~RLD4
      4~8:  BLD1_1~BLD1_5
      9~13: BLD2_1~BLD2_5
    */
    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d\r\n",
        aiTemperature[0],aiTemperature[1],aiTemperature[2],aiTemperature[3],uiAverage);
    // A70LV_Eric.C_0006 End

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDAH
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDAH(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulHours = 0; //A70LV_Larry_0316

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLD_HOURS, edaREAD, &ulHours);

    sprintf((char *)data->pcData, "%04u,%02u,%02u\r\n",
            (UINT16)((UINT32)ulHours/(UINT16)1440),
            (UINT8)(((UINT32)ulHours/(UINT16)60)%(UINT16)24),
            (UINT8)((UINT32)ulHours%(UINT16)60));

    return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDAE
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDAE(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "0xFF\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LATC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LATC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "700,800,821,566\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDBV
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDBV(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    //A70LV_Larry_0084
    UINT8 LD_Info_Num = sUtilHPBUTester_Callback.fpDataMgr_LD_Info_Num_GetCb();
    //UINT16 auiVoltage[LD_Info_Num]={0};
    UINT16 *auiVoltage = (UINT16 *)malloc(LD_Info_Num*sizeof(UINT16));
    if(auiVoltage == NULL)
        return 0;
    memset(auiVoltage, 0, LD_Info_Num*sizeof(UINT16));
    UINT8 ucCount = 0;
    char cStringTmp[MAX_ITEMNAME_LENGTH] = ""; //HICC2_Doulas_0016



    for(ucCount=0; ucCount<LD_Info_Num; ucCount++)
    {
        *(auiVoltage+ucCount) = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_LD_VoltageCb(ucCount);

		if(ucCount == (LD_Info_Num - 1)) //HICC2_Doulas_0016
		{
			snprintf(cStringTmp, MAX_ITEMNAME_LENGTH, "%d\r\n",*(auiVoltage+ucCount));
		}
		else
		{
			snprintf(cStringTmp, MAX_ITEMNAME_LENGTH, "%d,",*(auiVoltage+ucCount));
		}
		strcat((char *)data->pcData, cStringTmp);	//HICC2_Doulas_0016
    }
    //sprintf((char *)data->pcData,"%02d.%02dV,%02d.%02dV,%02d.%02dV,%02d.%02dV,%02d.%02dV,%02d.%02dV,%02d.%02dV,%02d.%02dV\r\n",
    //    auiVoltage[0]/100, auiVoltage[0]%100,  auiVoltage[1]/100, auiVoltage[1]%100,
	//	 auiVoltage[2]/100, auiVoltage[2]%100, auiVoltage[3]/100, auiVoltage[3]%100,
	//	 auiVoltage[4]/100, auiVoltage[4]%100,  auiVoltage[5]/100, auiVoltage[5]%100,
	//	 auiVoltage[6]/100, auiVoltage[6]%100,  auiVoltage[7]/100, auiVoltage[7]%100);
    //sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
    //    *(auiVoltage+0),*(auiVoltage+1),*(auiVoltage+2),*(auiVoltage+3),
	//	*(auiVoltage+4),*(auiVoltage+5),*(auiVoltage+6),*(auiVoltage+7),
	//	*(auiVoltage+8),*(auiVoltage+9),*(auiVoltage+10),*(auiVoltage+11),
	//	*(auiVoltage+12),*(auiVoltage+13)); //HICC2_Doulas_0016 remove
    free(auiVoltage);
    auiVoltage = NULL;

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDBC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDBC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    // A70LV_Eric.C_0006 Start , BLD1
    UINT8 LD_Info_Num = sUtilHPBUTester_Callback.fpDataMgr_LD_Info_Num_GetCb();
    //UINT16 auiCurrent[LD_Info_Num]={0};
    UINT16 *auiCurrent = (UINT16 *)malloc(LD_Info_Num*sizeof(UINT16));
    if(auiCurrent == NULL)
        return 0;
    memset(auiCurrent, 0, LD_Info_Num*sizeof(UINT16));
    UINT8 ucCount = 0;
	char cStringTmp[MAX_ITEMNAME_LENGTH] = ""; //HICC2_Doulas_0016



    for(ucCount=0; ucCount<LD_Info_Num; ucCount++)
    {
    	*(auiCurrent+ucCount) = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_LD_CurrentCb(ucCount);

		if(ucCount == (LD_Info_Num - 1)) //HICC2_Doulas_0016
		{
			snprintf(cStringTmp, MAX_ITEMNAME_LENGTH, "%d\r\n",*(auiCurrent+ucCount));
		}
		else
		{
			snprintf(cStringTmp, MAX_ITEMNAME_LENGTH, "%d,",*(auiCurrent+ucCount));
		}
		strcat((char *)data->pcData, cStringTmp);	//HICC2_Doulas_0016
    }
    //sprintf((char *)data->pcData,"%01d.%03dA,%01d.%03dA,%01d.%03dA,%01d.%03dA,%01d.%03dA,%01d.%03dA,%01d.%03dA,%01d.%03dA\r\n",
    //   		auiCurrent[0]/1000, auiCurrent[0]%1000, auiCurrent[1]/1000, auiCurrent[1]%1000,
	//		auiCurrent[2]/1000, auiCurrent[2]%1000, auiCurrent[3]/1000, auiCurrent[3]%1000,
	//		auiCurrent[4]/1000, auiCurrent[4]%1000, auiCurrent[5]/1000, auiCurrent[5]%1000,
	//		auiCurrent[6]/1000, auiCurrent[6]%1000, auiCurrent[7]/1000, auiCurrent[7]%1000);

    //8
    //sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
    //		*(auiCurrent+0),*(auiCurrent+1),*(auiCurrent+2),*(auiCurrent+3),
	//		*(auiCurrent+4),*(auiCurrent+5),*(auiCurrent+6),*(auiCurrent+7),
	//		*(auiCurrent+8),*(auiCurrent+9),*(auiCurrent+10),*(auiCurrent+11),
	//		*(auiCurrent+12),*(auiCurrent+13)); //HICC2_Doulas_0016 remove

    free(auiCurrent);
    auiCurrent = NULL;

    return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDBT
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDBT(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    // A70LV_Eric.C_0006 Start , BLD1

    UINT8 LD_Info_Num = sUtilHPBUTester_Callback.fpDataMgr_LD_Info_Num_GetCb();
    //INT16 aiTemperature[LD_Info_Num]={0};
    INT16 *aiTemperature = (INT16 *)malloc(sizeof(INT16)*LD_Info_Num);
    if(aiTemperature == NULL)
        return 0;
    memset(aiTemperature, 0, (sizeof(INT16)*LD_Info_Num));
    UINT8 ucCount = 0;
    char cStringTmp[MAX_ITEMNAME_LENGTH] = ""; //HICC2_Doulas_0016



    for(ucCount=0; ucCount<LD_Info_Num; ucCount++)
    {
    	*(aiTemperature+ucCount) = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_LD_TemperatureCb(ucCount);

		if(ucCount == (LD_Info_Num - 1)) //HICC2_Doulas_0016
		{
			snprintf(cStringTmp, MAX_ITEMNAME_LENGTH, "%d\r\n",*(aiTemperature+ucCount));
		}
		else
		{
			snprintf(cStringTmp, MAX_ITEMNAME_LENGTH, "%d,",*(aiTemperature+ucCount));
		}
		strcat((char *)data->pcData, cStringTmp);	//HICC2_Doulas_0016
    }
    //sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
    //		*(aiTemperature+0),*(aiTemperature+1),*(aiTemperature+2),*(aiTemperature+3),
	//		*(aiTemperature+4),*(aiTemperature+5),*(aiTemperature+6),*(aiTemperature+7),
	//		*(aiTemperature+8),*(aiTemperature+9),*(aiTemperature+10),*(aiTemperature+11),
	//		*(aiTemperature+12),*(aiTemperature+13)); //HICC2_Doulas_0016 remove

    //sprintf((char *)data->pcData," %02d.%02dT, %02d.%02dT, %02d.%02dT, %02d.%02dT, %02d.%02dT, %02d.%02dT, %02d.%02dT, %02d.%02dT\r\n",
    //		auiTemperature[0]/100, auiTemperature[0]%100, auiTemperature[1]/100, auiTemperature[1]%100,
	//		auiTemperature[2]/100, auiTemperature[2]%100, auiTemperature[3]/100, auiTemperature[3]%100,
	//		auiTemperature[4]/100, auiTemperature[4]%100, auiTemperature[5]/100, auiTemperature[5]%100,
	//		auiTemperature[6]/100, auiTemperature[6]%100, auiTemperature[7]/100, auiTemperature[7]%100);

    free(aiTemperature);
    aiTemperature = NULL;

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDBH
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDBH(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulHours = 0; //A70LV_Larry_0316

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLD_HOURS, edaREAD, &ulHours);

    sprintf((char *)data->pcData, "%04u,%02u,%02u\r\n",
            (UINT16)((UINT32)ulHours/(UINT16)1440),
            (UINT8)(((UINT32)ulHours/(UINT16)60)%(UINT16)24),
            (UINT8)((UINT32)ulHours%(UINT16)60));

    return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDBE
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDBE(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "0xFF\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LBTC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LBTC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "700,800,821,566\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDCV
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDCV(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else
    strcpy((char *)data->pcData, "3211,3212,3213,3214,3215\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDCC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDCC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "4211,4212,4213,4214,4215\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDCT
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDCT(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "2211,2212,2213,2214,2215\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDCH
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDCH(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulHours = 0; //A70LV_Larry_0316

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLD_HOURS, edaREAD, &ulHours);

    sprintf((char *)data->pcData, "%04u,%02u,%02u\r\n",
            (UINT16)((UINT32)ulHours/(UINT16)1440),
            (UINT8)(((UINT32)ulHours/(UINT16)60)%(UINT16)24),
            (UINT8)((UINT32)ulHours%(UINT16)60));

    return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDCE
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDCE(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "0xFF\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LCTC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LCTC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "700,800,821,566\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDDV
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDDV(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT8 BLD_Num = sUtilHPBUTester_Callback.fpDataMgr_BLD_Num_GetCb();
    UINT8 RLD_Num = sUtilHPBUTester_Callback.fpDataMgr_RLD_Num_GetCb();
    //UINT16 auiVoltage[RLD_Num]={0};
    UINT16 *auiVoltage = (UINT16 *)malloc(sizeof(UINT16)*RLD_Num);
    if(auiVoltage == NULL)
        return 0;
    memset(auiVoltage, 0, sizeof(UINT16)*RLD_Num);
    UINT8 ucCount = 0;
    //UINT16 uiAverage = 0;



    for(ucCount=0; ucCount<RLD_Num; ucCount++) //G100_Steven_0010
    {
        *(auiVoltage+ucCount) = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_LD_VoltageCb(ucCount+BLD_Num);
    }

    if(RLD_Num < 4)
        return 0;

    sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n",
            *(auiVoltage+0),*(auiVoltage+1),*(auiVoltage+2),*(auiVoltage+3));

    free(auiVoltage);
    auiVoltage = NULL;

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDDC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDDC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT8 BLD_Num = sUtilHPBUTester_Callback.fpDataMgr_BLD_Num_GetCb();
    UINT8 RLD_Num = sUtilHPBUTester_Callback.fpDataMgr_RLD_Num_GetCb();
    //UINT16 auiCurrent[RLD_Num]={0};
    UINT16 *auiCurrent = (UINT16 *)malloc(sizeof(UINT16)*RLD_Num);
    if(auiCurrent == NULL)
        return 0;
    memset(auiCurrent, 0, sizeof(UINT16)*RLD_Num);
    UINT8 ucCount = 0;
    //UINT16 uiAverage = 0;

    for(ucCount=0; ucCount<RLD_Num; ucCount++) //G100_Steven_0010
    {
        *(auiCurrent+ucCount) = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_LD_CurrentCb(ucCount+BLD_Num);
    }

    if(RLD_Num < 4)
        return 0;

    sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n",
            *(auiCurrent+0),*(auiCurrent+1),*(auiCurrent+2),*(auiCurrent+3));

    free(auiCurrent);
    auiCurrent = NULL;

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDDT
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDDT(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT8 BLD_Num = sUtilHPBUTester_Callback.fpDataMgr_BLD_Num_GetCb();
    UINT8 RLD_Num = sUtilHPBUTester_Callback.fpDataMgr_RLD_Num_GetCb();
    //UINT16 auiTemperature[RLD_Num]={0};
    UINT16 *auiTemperature = (UINT16 *)malloc(sizeof(UINT16)*RLD_Num);
    if(auiTemperature == NULL)
        return 0;
    memset(auiTemperature, 0, sizeof(UINT16)*RLD_Num);
    UINT8 ucCount = 0;
    //UINT16 uiAverage = 0;

    for(ucCount=0; ucCount<RLD_Num; ucCount++) //G100_Steven_0010
    {
        *(auiTemperature+ucCount) = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_LD_CurrentCb(ucCount+BLD_Num);
    }

    if(RLD_Num < 4)
        return 0;

    sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n",
            *(auiTemperature+0),*(auiTemperature+1),*(auiTemperature+2),*(auiTemperature+3));

    free(auiTemperature);
    auiTemperature = NULL;

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDDH
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDDH(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "10:55:44\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDDE
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDDE(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "0xFF\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LDTC
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LDTC(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "700,800,821,566\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_PRMG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_PRMG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT16 auiPumpSpeed={0};

	auiPumpSpeed = palDataMgr_Access_Get_Pump();

    sprintf((char *)data->pcData,"%d\r\n", auiPumpSpeed);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_PPHG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_PPHG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "10:43:40\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_TECV
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_TECV(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT16 auiSensor[3]={0};
    UINT8 ucCount = 0;

    for(ucCount=0; ucCount<3; ucCount++)
    {
    	auiSensor[ucCount] = palDataMgr_Access_Get_TEC_Voltage(ucCount);
    }

    sprintf((char *)data->pcData,"%d,%d\r\n",
    		auiSensor[0],auiSensor[1]);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_TECA
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_TECA(HPBU_TEST_COM_DATA *data)	//G100_Clare_0018
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT16 auiSensor[3]={0};
    UINT8 ucCount = 0;

    for(ucCount=0; ucCount<3; ucCount++)
    {
    	auiSensor[ucCount] = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_TEC_CurrentCb(ucCount);
    }

    sprintf((char *)data->pcData,"%d,%d\r\n",
    		auiSensor[0],auiSensor[1]);

    return 0;//PASS.
#endif
}

unsigned char cTester_TECS(HPBU_TEST_COM_DATA *data)//A70Gen2_Julie_0042
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT16 uiVoltage[3]={0};//DMD、RLD、NA
    UINT16 uiCurrent[3]={0};//DMD、RLD、NA
    UINT8 ucCount = 0;

    for(ucCount=0; ucCount<3; ucCount++)
    {
    	uiVoltage[ucCount] = palDataMgr_Access_Get_TEC_Voltage(ucCount);
		uiCurrent[ucCount] = palDataMgr_Access_Get_TEC_Current(ucCount);
    }

	switch(Board_ModelID_Get()) //A35G2_BRC_Casper_0122
	{
		case MODEL_ID_0:
			{
				if((uiVoltage[0] > 1600 && uiCurrent[0] < 1000) ||
					(uiVoltage[1] > 1600 && uiCurrent[1] < 1000))//16V & 1.0A
				{
					strcpy((char *)data->pcData, "TEC CHECK FAIL\r\n");
				}
				else
				{
					strcpy((char *)data->pcData, "TEC CHECK PASS\r\n");
				}
			}
			break;

		case MODEL_ID_1:
			{
				if((uiVoltage[0] > 1600 && uiCurrent[0] < 1000) ||
					(uiVoltage[1] > 3860 && uiCurrent[1] < 1000))
				{
					strcpy((char *)data->pcData, "TEC CHECK FAIL\r\n");
				}
				else
				{
					strcpy((char *)data->pcData, "TEC CHECK PASS\r\n");
				}
			}
			break;

		case MODEL_ID_2:
		default:
			{
				if((uiVoltage[0] > 3860 && uiCurrent[0] < 1000) ||
					(uiVoltage[1] > 3860 && uiCurrent[1] < 1000))//38.6V & 1.0A
				{
					strcpy((char *)data->pcData, "TEC CHECK FAIL\r\n");
				}
				else
				{
					strcpy((char *)data->pcData, "TEC CHECK PASS\r\n");
				}
			}
			break;
	}
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSVG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LSVG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    // A70LV_Eric.C_0006 Start
    UINT8 aucData[eFMT_MSG_LIGHT_SENSOR_SZ]={0};
    UINT16 auiLightSensorW = 0;
    UINT16 auiLightSensorR = 0;
    UINT16 auiLightSensorG = 0;
    UINT16 auiLightSensorB = 0;
    UINT16 auiLightSensorY = 0;

    halLDCtrl_Light_Sensor_Get(eLDBANK_A70LV, aucData);

    auiLightSensorW = ((UINT16)aucData[1] << 8) | (UINT16)aucData[0];
    auiLightSensorY = ((UINT16)aucData[3] << 8) | (UINT16)aucData[2];
    auiLightSensorR = ((UINT16)aucData[5] << 8) | (UINT16)aucData[4];
    auiLightSensorB = ((UINT16)aucData[7] << 8) | (UINT16)aucData[6];
    auiLightSensorG = ((UINT16)aucData[9] << 8) | (UINT16)aucData[8];

    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d\r\n",
        auiLightSensorW,auiLightSensorY,auiLightSensorR,auiLightSensorB,auiLightSensorG);
    // A70LV_Eric.C_0006 End

    return 0;//PASS.
#endif
}

unsigned char cTester_LS2G(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0002
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    // A70LV_Eric.C_0006 Start
    UINT8 aucData[eFMT_MSG_LIGHT_SENSOR_SZ]={0};
    UINT16 auiLightSensorW = 0;
    UINT16 auiLightSensorR = 0;
    UINT16 auiLightSensorG = 0;
    UINT16 auiLightSensorB = 0;
    UINT16 auiLightSensorY = 0;

    halLDCtrl_Light_Sensor_Get(eLDBANK_A70LV, aucData);

    auiLightSensorW = ((UINT16)aucData[1] << 8) | (UINT16)aucData[0];
    auiLightSensorY = ((UINT16)aucData[3] << 8) | (UINT16)aucData[2];
    auiLightSensorR = ((UINT16)aucData[5] << 8) | (UINT16)aucData[4];
    auiLightSensorB = ((UINT16)aucData[7] << 8) | (UINT16)aucData[6];
    auiLightSensorG = ((UINT16)aucData[9] << 8) | (UINT16)aucData[8];

    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d\r\n",
        auiLightSensorW,auiLightSensorY,auiLightSensorR,auiLightSensorB,auiLightSensorG);
    // A70LV_Eric.C_0006 End

    return 0;//PASS.
#endif
}


// ==============================================================================
// FUNCTION NAME: cTester_TEPG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_TEPG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


	return 0;
#else
#if defined(PLATFORM_H30_4K)
	INT16 aiSensor[7]={0};
	UINT8 ucCount = 0;

	for(ucCount=0; ucCount<7; ucCount++)
	{
		aiSensor[ucCount] = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_Thermal_SensorCb(ucCount);
	}
	sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d\r\n",
			aiSensor[0],aiSensor[1],aiSensor[2],aiSensor[4],aiSensor[5],aiSensor[6]);

#else
	INT16 aiSensor[4]={0};
	UINT8 ucCount = 0;

	for(ucCount=0; ucCount<3; ucCount++)
	{
		aiSensor[ucCount] = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_Thermal_SensorCb(ucCount);
	}
	aiSensor[3] = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_Thermal_SensorCb(4);//HICC2_Doulas_0064 DMD temp

	sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n",
			aiSensor[0],aiSensor[1], aiSensor[2], aiSensor[3]); //HICC2_Doulas_0064
#endif

	return 0;//PASS.
#endif
}


// ==============================================================================
// FUNCTION NAME: cTester_PWPS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_PWPS(HPBU_TEST_COM_DATA *data)  //HICC2_Doulas_0066
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    INT16 aiSensor = 0;

    aiSensor = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_Thermal_SensorCb(5);//PW photo sensor temp
    sprintf((char *)data->pcData,"%d\r\n",aiSensor);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_FILG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_FILG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

        UINT8 ucFanFilter = 0;

        halFanCtrl_Filter_Get(&ucFanFilter);

        sprintf((char *)data->pcData, "%d\r\n", ucFanFilter);

        return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSEG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LSEG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    sMOTOR_LIMIT sLimitZoom;
    sMOTOR_LIMIT sLimitFocus;

    palMotor_ZoomLimit_Get(&sLimitZoom);
    palMotor_FocusLimit_Get(&sLimitFocus);

    if((ABS_DIFF(sLimitFocus.wMaximumValue, sLimitFocus.wMinimumValue) > 100) &&
       (ABS_DIFF(sLimitZoom.wMaximumValue, sLimitZoom.wMinimumValue) > 100))
    {
        strcpy((char *)data->pcData, "Lens Enable\r\n");
    }
    else
    {
        strcpy((char *)data->pcData, "Lens Disable\r\n");
    }

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSDG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LSDG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
#ifdef PLATFORM_H30_4K
		sLENS_INFO sLensInfo = {0};

		palDataMgr_Data_Access(edcLENS_INFO, edaREAD, &sLensInfo);
		sprintf((char *)data->pcData,"%u,%u,%u,%u,%u,%u\r\n"
				, sLensInfo.wZoomMaximumValue, sLensInfo.wZoomMinimumValue
				, sLensInfo.wFocusMaximumValue, sLensInfo.wFocusMinimumValue
				, sLensInfo.wBacklashH, sLensInfo.wBacklashV
			);

#else

    sMOTOR_LIMIT sLimitZoom;
    sMOTOR_LIMIT sLimitFocus;
    sMOTOR_LIMIT sLensBacklash = {0};

    palMotor_ZoomLimit_Get(&sLimitZoom);
    palMotor_FocusLimit_Get(&sLimitFocus);
    palMotor_Lens_Backlash_Get(&sLensBacklash); //A70LV_Larry_0277

    sprintf((char *)data->pcData,"%u,%u,%u,%u,%u,%u\r\n"
            , sLimitZoom.wMaximumValue, sLimitZoom.wMinimumValue
            , sLimitFocus.wMaximumValue, sLimitFocus.wMinimumValue
            , sLensBacklash.wMaximumValue, sLensBacklash.wMinimumValue
        );
#endif
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSPG
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/10/18, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LSPG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else
    DWORD dwStep[2] = {0};

    palMotor_LensCenterGet((BYTE*)&dwStep);
    sprintf((char *)data->pcData,"%d,%d\r\n", dwStep[0], dwStep[1]);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_ABCS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_ABCS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "Pass\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_TPHG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_TPHG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulHours = 0; //A70LV_Larry_0084
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcTOTAL_PROJECTOR_HOURS, edaREAD, &ulHours);

    sprintf((char *)data->pcData, "%04u,%02u,%02u\r\n",
            (UINT16)((UINT32)ulHours/(UINT16)1440),
            (UINT8)(((UINT32)ulHours/(UINT16)60)%(UINT16)24),
            (UINT8)((UINT32)ulHours%(UINT16)60));
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_MSGG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_MSGG(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    strcpy((char *)data->pcData, "Pass\r\n");

    //strcpy((char *)data->pcData, "Fail\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_BIST
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_BIST(HPBU_TEST_COM_DATA *data) //HICC2_Steven_0004
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else

    UINT8  ucBIST = 0;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBIST_CHECK, edaREAD, (void*)&ucBIST);

    if(ucBIST == BIST_STATUS_PASS)   //HICC2_Steven_0012
    {
    	strcpy((char *)data->pcData, "Pass\r\n");
    }
    else
    {
    	strcpy((char *)data->pcData, "Fail\r\n");
    }

    //strcpy((char *)data->pcData, "Fail\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_DLDG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0016
// --------------------
// ==============================================================================
unsigned char cTester_CPRT(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulTime = 0;
    UINT16 uiHour = 0;
    UINT16 uiMin = 0;
    UINT16 uiSec = 0;

    ulTime = sUtilHPBUTester_Callback.fpIllumination_CurrentRunTimeGetCb();

    sprintf((char *)data->pcData,"%04u\r\n",ulTime);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_NUMS
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/01/24, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_NUMS(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    sprintf((char *)data->pcData, "%s\r\n", m_ucSNbuf);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSXG
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/08, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LSXG(HPBU_TEST_COM_DATA *data)
{
#if 1//def _FIRMWARE_SYSTME
    return 0;
#else
    INT16 err = 0;
    UINT16 lightSensor_BD1 = 0;
    UINT16 lightSensor_BD2 = 0;

    UINT8 aucData[10]={0}; //A70LV_Larry_0033 add

    halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0x00);
    halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0x00);

    MS_SLEEP(3000);

    halLDCtrl_Light_Sensor_Get(eLDBANK_A70LV, aucData);
    lightSensor_BD2 = ((UINT16)aucData[1] << 8) | (UINT16)aucData[0];
    lightSensor_BD2 = lightSensor_BD2/10;

    halLDCtrl_LD_GroupEnable(eLD_BANK_B1, 0xFF);
    halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0x00);

    MS_SLEEP(3000);

    halLDCtrl_Light_Sensor_Get(eLDBANK_A70LV, aucData);
    lightSensor_BD1 = ((UINT16)aucData[1] << 8) | (UINT16)aucData[0];
    lightSensor_BD1 = lightSensor_BD1/10;

    halLDCtrl_LD_GroupEnable(eLD_BANK_R1, 0xFF);
    halLDCtrl_LD_GroupEnable(eLD_BANK_B2, 0xFF);

    if(lightSensor_BD1 == 0 || lightSensor_BD2 == 0)
    {
        sprintf((char *)data->pcData, "%d\r\n", 99);
        return 0;
    }

    if(lightSensor_BD1 > lightSensor_BD2)
    {
        err = ROUND( (lightSensor_BD1 - lightSensor_BD2) * 100, lightSensor_BD2 );
    }
    else
    {
        err = ROUND( (lightSensor_BD2 - lightSensor_BD1) * 100, lightSensor_BD1 );
    }

    sprintf((char *)data->pcData, "%d,%d,%d,%d\r\n", 3, lightSensor_BD1, lightSensor_BD2, err);

    return 0;   //PASS.
#endif /* _FIRMWARE_SYSTME */
}       // A70LH_Jonas_0053 End

// ==============================================================================
// FUNCTION NAME: cTester_LSGT  //light sensor 收數據
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
unsigned char cTester_LSGT(HPBU_TEST_COM_DATA *data)		//G100_Doulas_0012
{
	UINT16 auilightSensor_T1[eLD_SEQ_NUMBER] = {0};
	UINT16 auilightSensor_T0[eLD_SEQ_NUMBER] = {0};
	UINT8 aucT1[80] = {0};
	UINT8 aucT0[80] = {0};
	UINT8 aucVal = 0;
	UINT8 aucOffset = 0;
	//UINT16 auilightSensor2_T1[eLD_SEQ_NUMBER] = {0};
	//UINT16 auilightSensor2_T0[eLD_SEQ_NUMBER] = {0};


	halLDCtrl_T1LightSensorValue_Get(eLDBANK_A70LV,aucT1);
	halLDCtrl_T0LightSensorValue_Get(eLDBANK_A70LV,aucT0);

	#if	1
	aucOffset = m_LS_T1T0_Start_Value*8;

	if(m_LS_T1T0_Start_Value < 10)	//Maxmum is 9
	{
		auilightSensor_T1[eLD_SEQ_Y] = ((UINT16)aucT1[aucOffset+1] << 8) | (UINT16)aucT1[aucOffset];
		auilightSensor_T1[eLD_SEQ_R] = ((UINT16)aucT1[aucOffset+3] << 8) | (UINT16)aucT1[aucOffset+2];
		auilightSensor_T1[eLD_SEQ_B] = ((UINT16)aucT1[aucOffset+5] << 8) | (UINT16)aucT1[aucOffset+4];
		auilightSensor_T1[eLD_SEQ_G] = ((UINT16)aucT1[aucOffset+7] << 8) | (UINT16)aucT1[aucOffset+6];

		auilightSensor_T0[eLD_SEQ_Y] = ((UINT16)aucT0[aucOffset+1] << 8) | (UINT16)aucT0[aucOffset];
		auilightSensor_T0[eLD_SEQ_R] = ((UINT16)aucT0[aucOffset+3] << 8) | (UINT16)aucT0[aucOffset+2];
		auilightSensor_T0[eLD_SEQ_B] = ((UINT16)aucT0[aucOffset+5] << 8) | (UINT16)aucT0[aucOffset+4];
		auilightSensor_T0[eLD_SEQ_G] = ((UINT16)aucT0[aucOffset+7] << 8) | (UINT16)aucT0[aucOffset+6];
	}


    sprintf((char *)data->pcData, "T1(%3d)(%4d,%4d,%4d,%4d),T0(%3d)(%4d,%4d,%4d,%4d)\r\n", m_LS_T1T0_Value[0],
																	auilightSensor_T1[eLD_SEQ_R],
																	auilightSensor_T1[eLD_SEQ_G],
																	auilightSensor_T1[eLD_SEQ_B],
																	auilightSensor_T1[eLD_SEQ_Y],
																	m_LS_T1T0_Value[0],
																	auilightSensor_T0[eLD_SEQ_R],
																	auilightSensor_T0[eLD_SEQ_G],
																	auilightSensor_T0[eLD_SEQ_B],
																	auilightSensor_T0[eLD_SEQ_Y]);
	m_LS_T1T0_Start_Value++;
   #else
   aucOffset = m_LS_T1T0_Start_Value*8;

	if(m_LS_T1T0_Start_Value < 10)	//Maxmum is 9
	{
		auilightSensor_T1[eLD_SEQ_Y] = ((UINT16)aucT1[aucOffset+1] << 8) | (UINT16)aucT1[aucOffset];
		auilightSensor_T1[eLD_SEQ_R] = ((UINT16)aucT1[aucOffset+3] << 8) | (UINT16)aucT1[aucOffset+2];
		auilightSensor_T1[eLD_SEQ_B] = ((UINT16)aucT1[aucOffset+5] << 8) | (UINT16)aucT1[aucOffset+4];
		auilightSensor_T1[eLD_SEQ_G] = ((UINT16)aucT1[aucOffset+7] << 8) | (UINT16)aucT1[aucOffset+6];

		auilightSensor_T0[eLD_SEQ_Y] = ((UINT16)aucT0[aucOffset+1] << 8) | (UINT16)aucT0[aucOffset];
		auilightSensor_T0[eLD_SEQ_R] = ((UINT16)aucT0[aucOffset+3] << 8) | (UINT16)aucT0[aucOffset+2];
		auilightSensor_T0[eLD_SEQ_B] = ((UINT16)aucT0[aucOffset+5] << 8) | (UINT16)aucT0[aucOffset+4];
		auilightSensor_T0[eLD_SEQ_G] = ((UINT16)aucT0[aucOffset+7] << 8) | (UINT16)aucT0[aucOffset+6];

		aucOffset = aucOffset+8;
		auilightSensor2_T1[eLD_SEQ_Y] = ((UINT16)aucT1[aucOffset+1] << 8) | (UINT16)aucT1[aucOffset];
		auilightSensor2_T1[eLD_SEQ_R] = ((UINT16)aucT1[aucOffset+3] << 8) | (UINT16)aucT1[aucOffset+2];
		auilightSensor2_T1[eLD_SEQ_B] = ((UINT16)aucT1[aucOffset+5] << 8) | (UINT16)aucT1[aucOffset+4];
		auilightSensor2_T1[eLD_SEQ_G] = ((UINT16)aucT1[aucOffset+7] << 8) | (UINT16)aucT1[aucOffset+6];

		auilightSensor2_T0[eLD_SEQ_Y] = ((UINT16)aucT0[aucOffset+1] << 8) | (UINT16)aucT0[aucOffset];
		auilightSensor2_T0[eLD_SEQ_R] = ((UINT16)aucT0[aucOffset+3] << 8) | (UINT16)aucT0[aucOffset+2];
		auilightSensor2_T0[eLD_SEQ_B] = ((UINT16)aucT0[aucOffset+5] << 8) | (UINT16)aucT0[aucOffset+4];
		auilightSensor2_T0[eLD_SEQ_G] = ((UINT16)aucT0[aucOffset+7] << 8) | (UINT16)aucT0[aucOffset+6];
	}


    sprintf((char *)data->pcData, "T1(%3d)(%4d,%4d,%4d,%4d),T0(%3d)(%4d,%4d,%4d,%4d),T1(%3d)(%4d,%4d,%4d,%4d),T0(%3d)(%4d,%4d,%4d,%4d)\r\n",
																	m_LS_T1T0_Value[0],
																	auilightSensor_T1[eLD_SEQ_R],
																	auilightSensor_T1[eLD_SEQ_G],
																	auilightSensor_T1[eLD_SEQ_B],
																	auilightSensor_T1[eLD_SEQ_Y],
																	m_LS_T1T0_Value[0],
																	auilightSensor_T0[eLD_SEQ_R],
																	auilightSensor_T0[eLD_SEQ_G],
																	auilightSensor_T0[eLD_SEQ_B],
																	auilightSensor_T0[eLD_SEQ_Y],
																	m_LS_T1T0_Value[0],
																	auilightSensor2_T1[eLD_SEQ_R],
																	auilightSensor2_T1[eLD_SEQ_G],
																	auilightSensor2_T1[eLD_SEQ_B],
																	auilightSensor2_T1[eLD_SEQ_Y],
																	m_LS_T1T0_Value[0],
																	auilightSensor2_T0[eLD_SEQ_R],
																	auilightSensor2_T0[eLD_SEQ_G],
																	auilightSensor2_T0[eLD_SEQ_B],
																	auilightSensor2_T0[eLD_SEQ_Y]);
	m_LS_T1T0_Start_Value++;
	m_LS_T1T0_Start_Value++;
   #endif
    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_LSTT   //set T1/T0 value and RLD on/off
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
unsigned char cTester_LSTT(HPBU_TEST_COM_DATA *data)		//G100_Doulas_0012
{
	UINT8 aucData[eFMT_MSG_LS_T1_SZ] = {0};


	aucData[0] = m_LS_T1T0_Value[0];							//set T1/T0 value
	aucData[1] = m_LS_T1T0_Value[0];
	aucData[2] = m_LS_T1T0_Value[0];
	aucData[3] = m_LS_T1T0_Value[0];
	halLDCtrl_LightSensorT1_Set(aucData);						//set T1 value
	halLDCtrl_LightSensorT0_Set(aucData);						//set T0 value
	halLDCtrl_RLD_Bank_Enable((BOOL)m_LS_T1T0_Value[1]);	//set RLD 0(off) or 255(on)
	halLDCtrl_LightSensorTrigger_Set(1);
	m_LS_T1T0_Start_Value = 0;
	MS_SLEEP(150);		//G100_Doulas_0014 Add delay
	sprintf((char *)data->pcData,"%d,%d\r\n", m_LS_T1T0_Value[0],m_LS_T1T0_Value[1]);

    return 0;   //PASS.
}
unsigned char cTester_PRES(HPBU_TEST_COM_DATA *data)		//G100_Doulas_0012
{

	sprintf((char *)data->pcData,"%d\r\n",sUtilHPBUTester_Callback.fpDataMgr_Access_Get_ALTIMETRYCb());	//G100_Clare_0018
    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_RTCS   //set RTC time
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/11/19, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_RTCS(HPBU_TEST_COM_DATA *data)   //G100_Owen_0052
{
    char cRTCStr[RTC_BUFFER_LEN] = {'\0'};
    UINT32 ulYear = 2000;                       //G100_Owen_0070
    UINT32 ulMonth = 1, ulDay = 1;              //G100_Owen_0070
    UINT32 ulHour = 0, ulMin = 0, ulSec = 0;    //G100_Owen_0070
    UINT8 ucData = 1;

    if(strlen((char *)m_ucRTCBuf) < RTC_BUFFER_LEN)
    {
        if((m_ucTimeZoneBuf[0]=='+') || (m_ucTimeZoneBuf[0]=='-'))
        {
            if(m_ucTimeZoneBuf[0]=='+')
            {
                sUtilHPBUTester_Callback.fpDataMgr_DateTime_TimeZoneStr_SetCb(TRUE, &m_ucTimeZoneBuf[1]);
            }
            else
            {
                sUtilHPBUTester_Callback.fpDataMgr_DateTime_TimeZoneStr_SetCb(FALSE, &m_ucTimeZoneBuf[1]);
            }
        }
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_MODE, edaWRITE_THROUGH_WITH_ACTION, &ucData);   //G100_Owen_0071 : set to manual mode

        sscanf(m_ucRTCBuf, "%u/%u/%u %u:%u:%u", &ulYear, &ulMonth, &ulDay, &ulHour, &ulMin, &ulSec);    //G100_Owen_0070

        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_YEAR, edaWRITE_THROUGH_WITH_ACTION, &ulYear);    //G100_Owen_0070
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_MONTH, edaWRITE_THROUGH_WITH_ACTION, &ulMonth);  //G100_Owen_0070
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_DAY, edaWRITE_THROUGH_WITH_ACTION, &ulDay);      //G100_Owen_0070
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_HOUR, edaWRITE_THROUGH_WITH_ACTION, &ulHour);    //G100_Owen_0070
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_MINUTE, edaWRITE_THROUGH_WITH_ACTION, &ulMin);   //G100_Owen_0070

        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcDATE_APPLY, edaWRITE_THROUGH_WITH_ACTION, &ucData);   //G100_Owen_0070

        sprintf((char *)data->pcData, "%s %d/%02d/%02d %02d:%02d:%02d\r\n", m_ucTimeZoneBuf, ulYear, ulMonth, ulDay, ulHour, ulMin, ulSec);
    }

    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_RTCG   //get RTC time
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/11/20, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_RTCG(HPBU_TEST_COM_DATA *data)   //G100_Owen_0053
{
    UINT16 uwYear;
    UINT8 ucMonth;
    UINT8 ucDay;
    UINT8 ucHour;
    UINT8 ucMinute;
    UINT8 ucSecond;
    char cRTCStr[RTC_BUFFER_LEN] = {'\0'};
    char cTimeZoneStr[TIMEZONE_BUFFER_LEN] = {'\0'};
    time_t sTime;
    struct tm *sTemp;

    time(&sTime);
    sTemp = localtime(&sTime);

    uwYear = (1900 + sTemp->tm_year);  //start at 1900 year
    ucMonth = (1 + sTemp->tm_mon);     //start at 0-11 month
    ucDay = sTemp->tm_mday;            //start at 01-31 day
    ucHour = sTemp->tm_hour;
    ucMinute = sTemp->tm_min;
    ucSecond = sTemp->tm_sec;
    sUtilHPBUTester_Callback.fpDataMgr_DateTime_TimeZoneStr_GetCb(cTimeZoneStr);

    sprintf(cRTCStr, "%s %04d/%02d/%02d %02d:%02d:%02d",
            cTimeZoneStr, uwYear, ucMonth, ucDay, ucHour, ucMinute, ucSecond);

    sprintf((char *)data->pcData, "%s\r\n", cRTCStr);

    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_OPTE   //optical center enable set
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/01/12, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_OPTE(HPBU_TEST_COM_DATA *data)   //G100_Owen_0068
{
    palMotor_LensCenterEnable_Set(1);
    strcpy((char *)data->pcData, "Pass\r\n");

    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_OPTA   //optical center apply set
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/01/12, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_OPTA(HPBU_TEST_COM_DATA *data)   //G100_Owen_0068
{
    palMotor_LensCenterApply_Set();
    strcpy((char *)data->pcData, "Pass\r\n");

    return 0;   //PASS.
}


// ==============================================================================
// FUNCTION NAME: cTester_OPDS
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/07/29, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_OPRA(HPBU_TEST_COM_DATA *data)	//A35G2_Coda_0110
{
    utilOPD_ResetAllToDefault();
    utilOPD_Initialization();
    sUtilHPBUTester_Callback.fpDataMgr_OPDRegulatoryInfoCb();

    strcpy((char *)data->pcData, "Pass\r\n");

    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_SPI7   //i-chip c789 stress test
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/04/27, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_SPI7(HPBU_TEST_COM_DATA *data)   //G100_Owen_0085
{
    UINT16 uiTotalCnt = 0, uiErrorCnt = 0;

    ///halC789Ctrl_Cmd_Test(&uiTotalCnt, &uiErrorCnt);

    sprintf((char *)data->pcData,"%d,%d\r\n", uiTotalCnt, uiErrorCnt);

    return 0;
}

// ==============================================================================
// FUNCTION NAME: cTester_SPI8   //i-chip c821 stress test
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/04/27, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_SPI8(HPBU_TEST_COM_DATA *data)   //G100_Owen_0085
{
    UINT16 uiTotalCnt = 0, uiErrorCnt = 0;

    //halScaler_Test(&uiTotalCnt, &uiErrorCnt);

    sprintf((char *)data->pcData,"%d,%d\r\n", uiTotalCnt, uiErrorCnt);

    return 0;
}

// ==============================================================================
// FUNCTION NAME: cTester_WIXG   //wheel index get
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/07/16, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_WIXG(HPBU_TEST_COM_DATA *data)   //G100_Owen_0095 //A35G2_BRC_Casper_0048
{
    UINT16 uiPWIdx = 0, uiFWIdx = 0;

//    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcPHOSPHOR_WHEEL_INDEX, edaREAD, &uiPWIdx);
//    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFILTER_WHEEL_INDEX, edaREAD, &uiFWIdx);
    palFormatterMgr_PW_Index_Get(&uiPWIdx);
    palFormatterMgr_FW_Index_Get(&uiFWIdx);

    sprintf((char *)data->pcData, "%d,%d\r\n", uiPWIdx, uiFWIdx);

    return 0;
}

// ==============================================================================
// FUNCTION NAME: cTester_RRWO   //RTC FW Reset and write reg to OPD file
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/07/16, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_RRWO(HPBU_TEST_COM_DATA *data)   //A70Gen2_Julie_0009//G100_Owen_0095 //A35G2_BRC_Casper_0048
{
    SYSTEM_CALL("%s ResetAll &", RTC_RESET_PATH);
    MS_SLEEP(1000);
    SYSTEM_CALL("%s ReadRegToOpd &", RTC_RESET_PATH);
    strcpy((char *)data->pcData, "RTC Reset and Write Reg into OPD\r\n");

    return 0;
}

unsigned char cTester_PLAT(HPBU_TEST_COM_DATA *data) //A70Gen2_Julie_0007
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else

	UINT8 ucCustomID = 0xFF, ucPlatformID = 0xFF;

	palDataMgr_Model_ID_Set(); //A70Gen2_Julie_0004

	ucCustomID = Board_ModelID_Get();
	ucPlatformID = Board_MCU_ModelID_Get();

	halMCU_PlatformID_Set(ucPlatformID);
	halFanCtrl_Platform_Set(ucCustomID);

    strcpy((char *)data->pcData, "Pass\r\n");
    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSVS   //light sensor target value set & calibration star
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/08/28, Doulas Create
// --------------------
// ==============================================================================
unsigned char cTester_LSVS(HPBU_TEST_COM_DATA *data)		//A70Gen2_Doulas_0018
{
	UINT8  ucFlag = 0;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucFlag);

    sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n",
        m_auiLS_TargetValue[eLD_SEQ_Y], m_auiLS_TargetValue[eLD_SEQ_R], m_auiLS_TargetValue[eLD_SEQ_B], m_auiLS_TargetValue[eLD_SEQ_G]);

	return 0;	//PASS.
}

unsigned char cTester_LS2S(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0002
{
	UINT8  ucFlag = 0;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaWRITE_THROUGH_WITH_ACTION, (void*)&ucFlag);

    sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n",
        m_auiLS_TargetValue[eLD_SEQ_Y], m_auiLS_TargetValue[eLD_SEQ_R], m_auiLS_TargetValue[eLD_SEQ_B], m_auiLS_TargetValue[eLD_SEQ_G]);

	return 0;	//PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_LSCT   //light sensor calibration all
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/07/15, Doulas Create
// --------------------
// ==============================================================================
unsigned char cTester_LSCT(HPBU_TEST_COM_DATA *data)		//A70Gen2_Doulas_0018
{
	UINT8  aucTime[eLD_SEQ_NUMBER] = {0};
	UINT16 auiADC[eLD_SEQ_NUMBER] = {0};
	UINT8  ucError = 0;
	UINT8  ucFlag = 0;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);   //HICC2_Doulas_0037
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_TIME, edaREAD, (void*)aucTime);                 //HICC2_Doulas_0037
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_TARGET_FACTORY, edaREAD, (void*)auiADC);        //HICC2_Doulas_0037

	if(ucFlag)
	{
		ucError = 0;
	}
	else
	{
		ucError = 1;
	}

	sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
		aucTime[0], aucTime[1], aucTime[2], aucTime[3],
		auiADC[0], auiADC[1], auiADC[2], auiADC[3],
		ucError);

	return 0;	//PASS.
}

unsigned char cTester_LC2T(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0002
{
	UINT8  aucTime[eLD_SEQ_NUMBER] = {0};
	UINT16 auiADC[eLD_SEQ_NUMBER] = {0};
	UINT8  ucError = 0;
	UINT8  ucFlag = 0;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHT_SENSOR_CALIBRATION_VALUE, edaREAD, (void*)&ucFlag);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_TIME_SECOND, edaREAD, (void*)aucTime);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_TARGET_FACTORY_SECOND, edaREAD, (void*)auiADC);

	if(ucFlag)
	{
		ucError = 0;
	}
	else
	{
		ucError = 1;
	}

	sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
		aucTime[0], aucTime[1], aucTime[2], aucTime[3],
		auiADC[0], auiADC[1], auiADC[2], auiADC[3],
		ucError);

	return 0;	//PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_LSRG
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/07/07, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_LSRG(HPBU_TEST_COM_DATA *data)		//A70Gen2_Doulas_0018
{
	UINT16 auiRLD[eLD_SEQ_NUMBER] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_TARGET_RLD, edaREAD, (void*)auiRLD); //HICC2_Doulas_0037

	sprintf((char *)data->pcData,"%d,%d\r\n",auiRLD[eLD_SEQ_Y], auiRLD[eLD_SEQ_R]);

    return 0;//PASS.
}

unsigned char cTester_LR2G(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0002
{
	UINT16 auiRLD[eLD_SEQ_NUMBER] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_TARGET_RLD, edaREAD, (void*)auiRLD); //HICC2_Doulas_0037

	sprintf((char *)data->pcData,"%d,%d\r\n",auiRLD[eLD_SEQ_Y], auiRLD[eLD_SEQ_R]);

    return 0;//PASS.
}


// ==============================================================================
// FUNCTION NAME: cTester_LSGG  //light sensor gain get
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/11/03, Owen Create
// --------------------
// ==============================================================================
unsigned char cTester_LSGG(HPBU_TEST_COM_DATA *data)    //A70Gen2_Doulas_0018
{
    float fGain[eLD_SEQ_NUMBER] = {0.0f};

	#if	0	//A65_OPTOMA_Doulas_0214
    halLDCtrl_LightSensorGain_Get((UINT8 *)fGain);
	#else
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_GAIN, edaREAD, (void*)fGain); //HICC2_Doulas_0037
	#endif

    sprintf((char *)data->pcData,"%f,%f,%f,%f\r\n",
        fGain[eLD_SEQ_Y], fGain[eLD_SEQ_R], fGain[eLD_SEQ_B], fGain[eLD_SEQ_G]);

    return 0;   //PASS.
}

unsigned char cTester_LG2G(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0002
{
    float fGain[eLD_SEQ_NUMBER] = {0.0f};

	#if	0	//A65_OPTOMA_Doulas_0214
    halLDCtrl_LightSensorGain_Get((UINT8 *)fGain);
	#else
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_GAIN_SECOND, edaREAD, (void*)fGain); //HICC2_Jacky_0003
	#endif

    sprintf((char *)data->pcData,"%f,%f,%f,%f\r\n",
        fGain[eLD_SEQ_Y], fGain[eLD_SEQ_R], fGain[eLD_SEQ_B], fGain[eLD_SEQ_G]);

    return 0;   //PASS.
}

unsigned char cTester_LSOG(HPBU_TEST_COM_DATA *data)    //A70Gen2_Doulas_0018
{
    UINT16 auiOffset[eLD_SEQ_NUMBER] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_OFFSET, edaREAD, (void*)auiOffset); //HICC2_Doulas_0037

    sprintf((char *)data->pcData,"4,%d,%d,%d,%d\r\n",auiOffset[eLD_SEQ_Y], auiOffset[eLD_SEQ_R], auiOffset[eLD_SEQ_B], auiOffset[eLD_SEQ_G]);

    return 0;   //PASS.
}

unsigned char cTester_LO2G(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0002
{
    UINT16 auiOffset[eLD_SEQ_NUMBER] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHTSENSOR_OFFSET_SECOND, edaREAD, (void*)auiOffset); //HICC2_Jacky_0003

    sprintf((char *)data->pcData,"4,%d,%d,%d,%d\r\n",auiOffset[eLD_SEQ_Y], auiOffset[eLD_SEQ_R], auiOffset[eLD_SEQ_B], auiOffset[eLD_SEQ_G]);

    return 0;   //PASS.
}

unsigned char cTester_USBT(HPBU_TEST_COM_DATA *data) //HICC2_Doulas_0077 Modify
{
    BYTE cimx = 0, cext = 0, cddp = 0, cksz = 0;
	system("/usr/bin/peripherytest");
	SYSTEM_CALL("sync");
    MS_SLEEP(1000);

    utilDataMgr_ReadFile_USBtest_periphery(&cimx,&cext,&cddp,&cksz);

    if((cimx == 1) && (cddp == 1))
	{
		strcpy((char *)data->pcData, "Pass\r\n");
	}
	else
	{
		strcpy((char *)data->pcData, "Fail\r\n");
	}

    return 0;   //PASS.
}

unsigned char cTester_UIMX(HPBU_TEST_COM_DATA *data)
{
	BYTE cimx = 0, cext = 0, cddp = 0, cksz = 0;

	utilDataMgr_ReadFile_USBtest_periphery(&cimx,&cext,&cddp,&cksz);

	if(cimx)
	{
		strcpy((char *)data->pcData, "Pass\r\n");
	}
	else
	{
		strcpy((char *)data->pcData, "Fail\r\n");
	}

    return 0;   //PASS.
}

unsigned char cTester_UEXT(HPBU_TEST_COM_DATA *data)
{
	BYTE cimx = 0, cext = 0, cddp = 0, cksz = 0;

	utilDataMgr_ReadFile_USBtest_periphery(&cimx,&cext,&cddp,&cksz);

	if(cext)
	{
		strcpy((char *)data->pcData, "Pass\r\n");
	}
	else
	{
		strcpy((char *)data->pcData, "Fail\r\n");
	}

    return 0;   //PASS.
}

unsigned char cTester_UDDP(HPBU_TEST_COM_DATA *data)
{
	BYTE cimx = 0, cext = 0, cddp = 0, cksz = 0;

	utilDataMgr_ReadFile_USBtest_periphery(&cimx,&cext,&cddp,&cksz);

	if(cddp)
	{
		strcpy((char *)data->pcData, "Pass\r\n");
	}
	else
	{
		strcpy((char *)data->pcData, "Fail\r\n");
	}

    return 0;   //PASS.
}

unsigned char cTester_UKSZ(HPBU_TEST_COM_DATA *data)
{
	BYTE cimx = 0, cext = 0, cddp = 0, cksz = 0;

	utilDataMgr_ReadFile_USBtest_periphery(&cimx,&cext,&cddp,&cksz);

	if(cksz)
	{
		strcpy((char *)data->pcData, "Pass\r\n");
	}
	else
	{
		strcpy((char *)data->pcData, "Fail\r\n");
	}

    return 0;   //PASS.
}

unsigned char cTester_OSDR(HPBU_TEST_COM_DATA *data) //HICC2_Doulas_0035
{
    UINT8 ucSelect = eSYSTEM_UPGRADE_SELECT_SCALER;

    strcpy((char *)data->pcData,"Reflash\r\n");

    palDataMgr_Data_Access(edcUPGRADE_SELECT, edaWRITE_THROUGH_WITH_ACTION, &ucSelect);

    return 0;   //PASS.
}

unsigned char cTester_CLAH(HPBU_TEST_COM_DATA *data)  	//HICC2_Doulas_0035
{
    UINT8 ucData = 1;

    palDataMgr_ResetAllHoursToDefault();

    strcpy((char *)data->pcData,"Done\r\n");
    return 0;   //PASS.
}

#ifdef OE_JIG //A70LV_John_0164 add command for TEC JIG data transfer
unsigned char cTester_TECG(HPBU_TEST_COM_DATA *data)
{
    //10 = (eLDDRV_CMD_TEC_JIG_DATA_SZ)/sizeof(WORD)
    UINT16 ucData[10]={0};
    halLDCtrl_Tec_JIG_Data_Get(eLDBANK_A70LV,(UINT8*)ucData);
    sprintf((char *)data->pcData,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
            ucData[0],ucData[1],ucData[2],ucData[3],ucData[4],
            ucData[5],ucData[6],ucData[7],ucData[8],ucData[9]);
    return 0;
}
#endif

unsigned char cTester_LSPS(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0001
{
	UINT8  ucFlag = 1;

    palDataMgr_Data_Access(edcLIGHTSENSOR_POSITION, edaWRITE_THROUGH_WITH_ACTION,(void*) &ucFlag);

    strcpy((char *)data->pcData,"Done\r\n");

    return 0;//PASS.
}


// ==============================================================================
// FUNCTION NAME: utilHPBUTest_CLI_Handle
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
// 2017/10/09, Larry Create
// --------------------
// ==============================================================================
UINT8 utilHPBUTest_CLI_Handle(UINT8 ucData)
{
    if(cIsTesterMode() == 0)
        return 0;

//    uiEmpty = Board_Uart_Read(eBOARD_UART_UART1, 1 ,&ucData);    //G100_Wilsonj_0018 ???

    if((ucData == HPBUTEST_START_HEADER || m_ucHPBUTestPos != 0))
    {
        m_ucHPBUTestrBuffer[m_ucHPBUTestPos++] = ucData;

        if(ucData ==  HPBUTEST_END_HEADER_1 || ucData ==  HPBUTEST_END_HEADER_2)
        {
            m_ucHPBUTestrBuffer[m_ucHPBUTestPos] = '\0';

            cTesterDecode((char*)m_ucHPBUTestrBuffer);

            //LOG_MSG(db_APP_CLI, "%s \r\n", m_sCLI_Info[0].ucCLI_DATA);
            m_ucHPBUTestPos = 0;
        }
    }

    return 1;
}

void utilHPBUTest_LS_TargetValue_Get(UINT16 *Data)
{
    memcpy(Data, m_auiLS_TargetValue, sizeof(m_auiLS_TargetValue));
}

#ifdef UNITY_CLI
#include "utilCommonMSSCAPI.h"
UINT8 utilHPBUTest_UNITYCmd_Decode(INT8 *cFuncIn, INT8 *cStringIn, INT8 *BufferOut) //A65_OPTOMA_Julie_0006
{
    INT8 str[256] = {0};
    INT16 i;
    UINT8 ret = 0;
    HPBU_TEST_COM_DATA sData;
    UINT16 wStringLen = 0;

    wStringLen = strlen((INT8 *)cStringIn);

    snprintf(str, 255, "%s%s%02d%s", cFuncIn, "99", wStringLen, cStringIn);
    DecodeCommand(str, &sData);

    for(i = 0; i < MAX_TESTLUT_SIZE; i++)
    {
        if(0 == strncmp(sTable_TesterLut[i].cFuncCode, "FFFF", COMMAND_LENGTH))
        {
            return eCLI_ERROR_CODE_FUNCCODENOTFOUND;// The call back function not exist.
        }

        if(0 == strncmp(sTable_TesterLut[i].cFuncCode, cFuncIn, COMMAND_LENGTH))
        {
            if(sTable_TesterLut[i].fpTester_Request != 0)
            {
                ret = sTable_TesterLut[i].fpTester_Request(&sData); //execute call back function.

                memcpy((INT8 *)BufferOut, (INT8 *)sData.pcData, strlen(sData.pcData)+1);

                if(ret != 0)
                {
                    LOG_MSG(db_APP_CLI, "(func:%s, line:%d) ret[%d]", __FUNCTION__, __LINE__, ret);
                    //return ret;
                    return eCLI_ERROR_CODE_DATAERROR;
                }
                break;
            }
        }
    }
    return eCLI_ERROR_CODE_NO;
}

UINT8 utilHPBUTest_UNITYCmd_GetFlag(INT8 *cHPBUCmd, UINT16 *uiSpecialFlag)
{
    UINT8 ucIndex;
    for(ucIndex = 0; ucIndex < MAX_TESTLUT_SIZE; ucIndex++)
    {
        if(!strncmp(sTable_TesterLut[ucIndex].cFuncCode, "FFFF", COMMAND_LENGTH))
        {
            return eCLI_ERROR_CODE_LookupIndexErr;
        }
        if(!strncmp(sTable_TesterLut[ucIndex].cFuncCode, cHPBUCmd, COMMAND_LENGTH))
        {
            *uiSpecialFlag = sTable_TesterLut[ucIndex].uiSpecialFlag;
            return eCLI_ERROR_CODE_NO;
        }
    }
    return eCLI_ERROR_CODE_LookupIndexErr;
}
#endif // UNITY_CLI

unsigned char cTester_LECA(HPBU_TEST_COM_DATA *data) //HICC2_Doulas_0056
{
	UINT8 ucData = 1;
    // Lens Calibration2 Process
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLENS_CALIBRATION, edaWRITE_THROUGH_WITH_ACTION, &ucData);

	sprintf((char *)data->pcData,"Done\r\n");

    return 0;//PASS.
}

unsigned char cTester_LSAI(HPBU_TEST_COM_DATA *data)  //HICC2_Doulas_0056 lens ID/zoom(max/min)/focus(max/min) values get
{
	sMOTOR_LIMIT sLimitZoom;
	sMOTOR_LIMIT sLimitFocus;
	UINT32 ulLensIDAdc = 0;

	palMotor_ZoomLimit_Get(&sLimitZoom);
	palMotor_FocusLimit_Get(&sLimitFocus);
	palMotor_LensIdADC_Get(&ulLensIDAdc);

	sprintf((char *)data->pcData,"%u,%u,%u,%u,%u\r\n"
		        , ulLensIDAdc
				, sLimitZoom.wMaximumValue, sLimitZoom.wMinimumValue
				, sLimitFocus.wMaximumValue, sLimitFocus.wMinimumValue
			);
	return 0;//PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_SSRH
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/01/04, Doulas Create
// --------------------
// ==============================================================================
unsigned char cTester_SSRH(HPBU_TEST_COM_DATA *data)   //HICC2_Doulas_0064
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT8 aucHumidity[16] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE, edaREAD, (void*)aucHumidity);

    sprintf((char *)data->pcData,"%s\r\n", aucHumidity);

    return 0;//PASS.
#endif
}

unsigned char cTester_FACT(HPBU_TEST_COM_DATA *data)
{
    UINT8 ucData = 1;

	TesterRs232Feedback("FACT0106Done\r\n");    //reply Done before executing.

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcFACTORY_RESET, edaWRITE_THROUGH_WITH_ACTION, &ucData);


    return 0;   //PASS.
}

unsigned char cTester_HWVG(HPBU_TEST_COM_DATA *data)
{
    UINT8 aucVersion[8] = {0};

    palDataMgr_Data_Access(edcHW_PROJECTOR_ID, edaREAD, aucVersion);

    sprintf((char *)data->pcData,"%s\r\n",aucVersion);

    return 0;   //PASS.
}

// ==============================================================================
// FUNCTION NAME: cTester_ACTU
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2023/05/18, Casper Create
// --------------------
// ==============================================================================
unsigned char cTester_ACTU(HPBU_TEST_COM_DATA *data) //H60_Doulas_0033 actuator(Gain/Sub Frame Delay/ Segment Length)
{
#ifdef _FIRMWARE_SYSTME

	return 0;

#else //A70LV_Larry_0084
    UINT8  ucCH0Gain = 0;
    UINT32 ulCH0SubFramedelay = 0;
    UINT16 uiCh0SegmentLength = 0;
    UINT8  ucCH1Gain = 0;
    UINT32 ulCH1SubFramedelay = 0;
    UINT16 uiCh1SegmentLength = 0;

    palDataMgr_Data_Access(edcCH0_DACGAIN, edaREAD, &ucCH0Gain);
    palDataMgr_Data_Access(edcCH0_SFDELAY, edaREAD, &ulCH0SubFramedelay);
    palDataMgr_Data_Access(edcCH0_SEGMENTLEN, edaREAD, &uiCh0SegmentLength);

    palDataMgr_Data_Access(edcCH1_DACGAIN, edaREAD, &ucCH1Gain);
    palDataMgr_Data_Access(edcCH1_SFDELAY, edaREAD, &ulCH1SubFramedelay);
    palDataMgr_Data_Access(edcCH1_SEGMENTLEN, edaREAD, &uiCh1SegmentLength);

    sprintf((char *)data->pcData,  "%d,%d,%d,%d,%d,%d\r\n",
        ucCH0Gain,
        ulCH0SubFramedelay,
        uiCh0SegmentLength,
        ucCH1Gain,
        ulCH1SubFramedelay,
        uiCh1SegmentLength
        );
	return 0;//PASS.

#endif
}

unsigned char cTester_CLSL(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

	return 0;

#else //A70LV_Larry_0084
    UINT32  ucGain0 = 0;
    UINT32 uiSegmentLength0 = 0;
    UINT32  ucGain1 = 0;
    UINT32 uiSegmentLength1 = 0;

    palFormatterMgr_XPR_Gain_of_Axis0_Get(&ucGain0);
    palFormatterMgr_XPR_Segment_Length_of_Axis0_Get(&uiSegmentLength0);

    palFormatterMgr_XPR_Gain_of_Axis1_Get(&ucGain1);
    palFormatterMgr_XPR_Segment_Length_of_Axis1_Get(&uiSegmentLength1);
    sprintf((char *)data->pcData,  "%d,%d,%d,%d\r\n",
        ucGain0,
        uiSegmentLength0,
        ucGain1,
        uiSegmentLength1
        );
	return 0;//PASS.

#endif
}

unsigned char cTester_CLSN(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME

	return 0;

#else //A70LV_Larry_0084
    UINT8 cData[32] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcCLOSE_LOOP_SN, edaREAD, cData);

    snprintf((char *)data->pcData, 32, "%s\r\n", cData);

	return 0;//PASS.

#endif
}

unsigned char cTester_PCBA(HPBU_TEST_COM_DATA *data)
{
    UINT8 cPCBA[PCBA_LENGTH] = {'\0'};

    if(strlen((char *)m_ucPCBAbuf) < PCBA_LENGTH)
    {
        sprintf((char *)data->pcData, "%s\r\n", m_ucPCBAbuf);
    }

    return 0;//PASS.
}

unsigned char cTester_LV2G(HPBU_TEST_COM_DATA *data) //HICC2_Jacky_0003
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT16 auiInfo[eLD_SEQ_NUMBER] = {0};
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcLIGHT_SENSOR_INFO, edaREAD, (void*)auiInfo);


    sprintf((char *)data->pcData,"%d,%d,%d,%d\r\n", auiInfo[0],auiInfo[1],auiInfo[2],auiInfo[3]); //YRBG
    // A70LV_Eric.C_0006 End

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_PWPS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LOGS(HPBU_TEST_COM_DATA *data)  //HICC2_Doulas_0098
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    INT16 aiSensor = 0;

    aiSensor = sUtilHPBUTester_Callback.fpDataMgr_Access_Get_Thermal_SensorCb(7);//L1G sensor temp
    sprintf((char *)data->pcData,"%d\r\n",aiSensor);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_PWPS
// DESCRIPTION:
//
//
// Params:
// HPBU_TEST_COM_DATA *data:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/09/25, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_DMDS(HPBU_TEST_COM_DATA *data)  //HICC2_Doulas_0098 DMD Humidity/Pressure sensor
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT8 aucPressSensor[16] = {0};
    UINT8 aucHumidity[16] = {0};
    UINT8 aucAbsHumidity[16] = {0}; //HICC2_Doulas_0106

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcALTIMETRY_VALUE2, edaREAD, (void*)aucPressSensor);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE2, edaREAD, (void*)aucHumidity);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcABSOLUTE_HUMIDITY2, edaREAD, (void*)aucAbsHumidity); //HICC2_Doulas_0106

    sprintf((char *)data->pcData,"%s,%s,%s\r\n",aucPressSensor, aucHumidity, aucAbsHumidity); //HICC2_Doulas_0106

    return 0;//PASS.
#endif
}

unsigned char cTester_SHUM(HPBU_TEST_COM_DATA *data) //HICC2_Doulas_0106
{
#ifdef _FIRMWARE_SYSTME

    return 0;
#else
    UINT8 aucHumidity[16] = {0};
    UINT8 aucAbsHumidity[16] = {0};

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcBAROMETRY_VALUE, edaREAD, (void*)aucHumidity);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcABSOLUTE_HUMIDITY1, edaREAD, (void*)aucAbsHumidity);

    sprintf((char *)data->pcData,"%s,%s\r\n", aucHumidity, aucAbsHumidity);

    return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSDT
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/07, Larry Create
// --------------------
// ==============================================================================
unsigned char cTester_LSDT(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else

    UINT8 ucLensPlug = palMotor_LensPlug_Get();
    UINT8 ucFujiDet = palMotor_LensLock_State_Get();

    sprintf((char *)data->pcData,"%d,%d\r\n", ucLensPlug, ucFujiDet);

    return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_LSDT
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/020, Doulas Create
// --------------------
// ==============================================================================
unsigned char cTester_TESP(HPBU_TEST_COM_DATA *data)  //HICC2_Doulas_0126
{
#ifdef _FIRMWARE_SYSTME
    return 0;
#else

    UINT8 ucPattern = eCM_TEST_PATTERN_CHECKERBOARD;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcOSDTEST_PATTERN, edaWRITE_THROUGH_WITH_ACTION, &ucPattern);
    sprintf((char *)data->pcData,"%d\r\n", ucPattern);

    return 0;//PASS.

#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_XPRE
// DESCRIPTION:
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2024/08/020, Doulas Create
// --------------------
// ==============================================================================


unsigned char cTester_XPRE(HPBU_TEST_COM_DATA *data) //HICC2_Steven_0052
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulErr0 = 0;
    UINT32 ulErr1 = 0;
    UINT8 ucCLAEnable = TRUE;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcCLOSE_LOOP_ENABLE_AXIS0, edaWRITE_THROUGH_WITH_ACTION, &ucCLAEnable);
    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcCLOSE_LOOP_ENABLE_AXIS1, edaWRITE_THROUGH_WITH_ACTION, &ucCLAEnable);

    palFormatterMgr_XPR_CloseLoop_ErrorCode0(&ulErr0);

    palFormatterMgr_XPR_CloseLoop_ErrorCode1(&ulErr1);

    sprintf((char *)data->pcData, "%d,%d\r\n",ulErr0, ulErr1);

    return 0;//PASS.
#endif
}

unsigned char cTester_XPRZ(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    UINT32 ulZData0 = 0;
    UINT32 ulZData1 = 0;

    palFormatterMgr_XPR_Eeprom_ZData0_Get(&ulZData0);

    palFormatterMgr_XPR_Eeprom_ZData1_Get(&ulZData1);

    sprintf((char *)data->pcData, "%d,%d\r\n",ulZData0, ulZData1);

    return 0;//PASS.
#endif
}

unsigned char cTester_LENV(HPBU_TEST_COM_DATA *data)
{
#ifdef _FIRMWARE_SYSTME


    return 0;
#else
    dvMotorGecCmd(eCLI_GEC_IIC_STATUS, 0, 0);
	vTaskDelay(200);

	if(halMotor_LSM_NVRAM_Status_Get() == TRUE)
    	sprintf((char *)data->pcData, "Pass\r\n");
	else
    	sprintf((char *)data->pcData, "Fail\r\n");

    return 0;//PASS.
#endif
}

//H30K_Tim_0011, add, ***
// ==============================================================================
// FUNCTION NAME: cTester_OSN1
// DESCRIPTION: Get OE SN1
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2025/5/19, Tim Create
// --------------------
// ==============================================================================
unsigned char cTester_O1SN(HPBU_TEST_COM_DATA *data)
{

#ifdef _FIRMWARE_SYSTME

        return 0; //PASS.
#else
        UINT8 cSN[VERSION_STRING_MAX_LENGTH] = {'\0'};

        memset(cSN, 0, VERSION_STRING_MAX_LENGTH);
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcOESN1, edaREAD, cSN);
        sprintf((char *)data->pcData, "%s\r\n", cSN);

        return 0;//PASS.
#endif
}

// ==============================================================================
// FUNCTION NAME: cTester_OSN2
// DESCRIPTION: Get OE SN2
//
//
// Params:
//
// Returns:
//
//
// Modification History
// --------------------
// 2025/5/19, Tim Create
// --------------------
// ==============================================================================
unsigned char cTester_O2SN(HPBU_TEST_COM_DATA *data)
{

#ifdef _FIRMWARE_SYSTME

        return 0; //PASS.
#else
        UINT8 cSN[VERSION_STRING_MAX_LENGTH] = {'\0'};

        memset(cSN, 0, VERSION_STRING_MAX_LENGTH);
        sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcOESN2, edaREAD, cSN);
        sprintf((char *)data->pcData, "%s\r\n", cSN);

        return 0;//PASS.
#endif
}
//H30K_Tim_0011, add, &&&

unsigned char cTester_URSW(HPBU_TEST_COM_DATA *data)
{
    TesterRs232Feedback("URSW0106Pass\r\n");
	vTaskDelay(500);
	halMCU_UartSwitch_Set(0);
}


unsigned char cTester_LMCR(HPBU_TEST_COM_DATA *data)
{
	UINT8 ucData = 1;

    sUtilHPBUTester_Callback.fpDataMgr_Data_AccessCb(edcARTNET_LENS_MOVE_TO_CENTER,    edaWRITE_THROUGH_WITH_ACTION,  &m_uiOPD_PERIOD);

	sprintf((char *)data->pcData,"Done\r\n");

    return 0;//PASS.
}






