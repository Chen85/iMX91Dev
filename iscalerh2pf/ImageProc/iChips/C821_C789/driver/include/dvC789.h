#ifndef DVC789_H
#define DVC789_H

#include "Common.h"
#include "utilCommon.h"

//TEST : should remove
#define ICHIPC789_DUMP_ADDR     (0x140C0000)

//BN
#define BN_RGBNK		(0xFF0001)
#define BN_MCT	        (0xFF0101)
#define BN_RTCT	    	(0xFF0301)
#define BN_DDR3CT		(0xFF0401)
#define BN_MEMCT		(0xFF0601)
#define BN_RSTCT		(0xFF0701)

//B0
#define B0_CLKCTRL		(0x000801)
#define B0_CMOSOUTSETUP	(0x000901)
#define B0_POCLKCT		(0x000A01)
#define B0_POPLLCT		(0x000B01)
#define B0_POREFDIV		(0x000C01)
#define B0_POFBDIV		(0x000D01)
#define B0_IPPLLCT		(0x000F01)
#define B0_IPREFDIV		(0x001001)
#define B0_IPFBDIV		(0x001101)
#define B0_LVDSOCLKCT	(0x001201)
#define B0_LVDSOSETUP	(0x001301)
#define B0_LVDSO1CT		(0x001401)
#define B0_LVDSO2CT		(0x001501)
#define B0_LVDSTXCT1	(0x001601)
#define B0_LVDSTXCT2	(0x001701)
#define B0_LVDSI1CT		(0x001801)
#define B0_LVDSI2CT		(0x001901)
#define B0_LVDSRXCT1	(0x001A02)
#define B0_LVDSRXCT2	(0x001C02)
#define B0_DDR3CT2		(0x002006)
#define B0_DDR3CT2_0    (0x002001)
#define B0_DDR3CT2_1    (0x002101)
#define B0_DDR3CT2_2	(0x002201)
#define B0_DDR3CT2_3	(0x002301)
#define B0_DDR3CT2_4	(0x002401)
#define B0_DDR3CT2_5	(0x002501)
#define B0_DDR3OCDDIP	(0x002601)
#define B0_DDR3OCDDIN	(0x002701)
#define B0_DDR3LVLCT	(0x002801)
#define B0_DDR3WRLVLPHCT    (0x002902)
#define B0_DDR3WRTMGCT	(0x002B01)
#define B0_DDR3RDTMGCT	(0x002C02)
#define B0_DDR3VREFCT	(0x002E01)
#define B0_DDR3DLLCT	(0x002F01)
#define B0_DDR3ODTCT	(0x003001)
#define B0_DDR3ZQCT		(0x003101)
#define B0_OCFMT		(0x003501)
#define B0_ICFMT		(0x003602)
#define B0_ISCWCT		(0x003801)
#define B0_OSDCT		(0x003901)
#define B0_BOWINH		(0x003A01)
#define B0_OSDFILL		(0x003B02)
#define B0_CPURAD		(0x003D04)
#define B0_CPUWAD		(0x004104)
#define B0_CPUDT		(0x004501)
#define B0_CPUDTCTL		(0x004601)
#define B0_BOSTAT		(0x004701)
#define B0_BBVDLY		(0x004802)
#define B0_BBVSYCT		(0x004A01)
#define B0_BBWMWI		(0x004B01)
#define B0_BBRMWI		(0x004C01)
#define B0_BOACCT		(0x004D01)
#define B0_BBACTHW		(0x004E02)
#define B0_BBACTVW		(0x005002)
#define B0_CBUFHST		(0x005201)
#define B0_CBUFHW		(0x005301)
#define B0_CBUFVST		(0x005401)
#define B0_CBUFVW		(0x005501)
#define B0_CBUFAD		(0x005602)
#define B0_CBUFDT		(0x005801)
#define B0_CBUFMD		(0x005901)
#define B0_MPROTECT		(0x005A02)
#define B0_RNGBUFWCT	(0x005C01)
#define B0_LINEDRAWCT	(0x005D01)
#define B0_COORDH1		(0x005E02)
#define B0_COORDV1		(0x006002)
#define B0_COORDH2		(0x006202)
#define B0_COORDV2		(0x006402)
#define B0_CPURAD2		(0x006604)
#define B0_BBALUMWI		(0x006A01)
#define B0_BBMODE		(0x006B01)
#define B0_BBBLENDC		(0x006C01)
#define B0_BBMAXMINCT	(0x006D01)
#define B0_BBBINTHR		(0x006E02)

//B1
#define B1_OSDMODE		(0x010801)
#define B1_OSDCT		(0x010901)
#define B1_BOTRANS0		(0x010A01)
#define B1_BOTRANS1		(0x010B01)
#define B1_BOTRANS2		(0x010C01)
#define B1_BOTRANS3		(0x010D01)
#define B1_OSDACTHST	(0x010E02)
#define B1_OSDACTHW		(0x011002)
#define B1_OSDACTVST	(0x011202)
#define B1_OSDACTVW		(0x011402)
#define B1_OSDSAD		(0x011604)
#define B1_OSDMWI		(0x011A01)
#define B1_PLTAD		(0x011B01)
#define B1_PLTDT		(0x011C01)
#define B1_BLINKTIME	(0x011D01)
#define B1_BLINKCYCL	(0x011E01)
#define B1_OSDGAINR0	(0x011F01)
#define B1_OSDGAING0	(0x012001)
#define B1_OSDGAINB0	(0x012101)
#define B1_OSDBIASR0	(0x012201)
#define B1_OSDBIASG0	(0x012301)
#define B1_OSDBIASB0	(0x012401)
#define B1_OSDGAINR1	(0x012501)
#define B1_OSDGAING1	(0x012601)
#define B1_OSDGAINB1	(0x012701)
#define B1_OSDBIASR1	(0x012801)
#define B1_OSDBIASG1	(0x012901)
#define B1_OSDBIASB1	(0x012A01)
#define B1_OSDGAINR2	(0x012B01)
#define B1_OSDGAING2	(0x012C01)
#define B1_OSDGAINB2	(0x012D01)
#define B1_OSDBIASR2	(0x012E01)
#define B1_OSDBIASG2	(0x012F01)
#define B1_OSDBIASB2	(0x013001)
#define B1_OSDGAINR3	(0x013101)
#define B1_OSDGAING3	(0x013201)
#define B1_OSDGAINB3	(0x013301)
#define B1_OSDBIASR3	(0x013401)
#define B1_OSDBIASG3	(0x013501)
#define B1_OSDBIASB3	(0x013601)
#define B1_OSDBLCT		(0x013701)
#define B1_OSDBLENDC	(0x013801)
#define B1_OSDSCRLCT	(0x013901)
#define B1_OSDSCRLSPD	(0x013A01)
#define B1_OSDSCRLDST	(0x013B02)
#define B1_OSDSCRLHW	(0x013D02)
#define B1_OSDSCRLVW	(0x013F02)
#define B1_PGCTRL		(0x014101)
#define B1_PGPTCTRL		(0x014201)
#define B1_HRAMPCNT		(0x014301)
#define B1_VRAMPCNT		(0x014401)
#define B1_HLIMITMIN	(0x014502)
#define B1_VLIMITMIN	(0x014702)
#define B1_HLIMITMAX	(0x014902)
#define B1_VLIMITMAX	(0x014B02)
#define B1_PGCOLR		(0x014D01)
#define B1_PGCOLG		(0x014E01)
#define B1_PGCOLB		(0x014F01)
#define B1_PGCOLLSR		(0x015001)
#define B1_PGCOLLSG		(0x015101)
#define B1_PGCOLLSB		(0x015201)
#define B1_SFLMODE		(0x015401)
#define B1_SFLCT		(0x015501)
#define B1_SFLCMDWR		(0x015601)
#define B1_SFLCMDRD		(0x015701)
#define B1_SFLCMDER		(0x015801)
#define B1_SFLCMDWREN	(0x015901)
#define B1_SFLCMDWRSR	(0x015A01)
#define B1_SFLCMDRDSR	(0x015B01)
#define B1_SFLWRSR		(0x015C01)
#define B1_SFLWRRH		(0x015D01)
#define B1_SFLAD		(0x015E04)
#define B1_SFLDT		(0x016201)
#define B1_SFLDTCTL		(0x016301)
#define B1_SFLSTAT		(0x016401)
#define B1_SFLRDSR		(0x016501)
#define B1_SFLCNT		(0x016604)
#define B1_SFLREGSEL	(0x016A01)
#define B1_SFLTEST		(0x016B01)
#define B1_SFLDVDLY		(0x016C01)
#define B1_SFLSZSEL		(0x016D01)
#define B1_GIOC		    (0x017001)
#define B1_GIOO		    (0x017101)
#define B1_GIOI		    (0x017201)
#define B1_GIOS		    (0x017301)
#define B1_GIOPDCT		(0x017401)
#define B1_INTEN		(0x017602)
#define B1_INTRST		(0x017802)
#define B1_INTSTAT		(0x017A02)
#define B1_TESTG		(0x017F01)

//B2
#define B2_OSYCT		(0x020802)
#define B2_OHCYCL		(0x020A02)
#define B2_OVCYCL		(0x020C02)
#define B2_OSYCT2		(0x020E01)
#define B2_OSYCT3		(0x020F01)
#define B2_OHSCT		(0x021001)
#define B2_OVSCT		(0x021101)
#define B2_OVDLY		(0x021202)
#define B2_OVHDLY		(0x021402)
#define B2_OHDLY		(0x021602)
#define B2_SYRDLY		(0x021801)
#define B2_OVCYCLMIN	(0x021902)
#define B2_SYVRSTCT		(0x021B01)
#define B2_FLDDLY		(0x021C01)
#define	B2_SYEXDLY	    (0x021D01)
#define B2_FLDRTCT		(0x021E01)
#define B2_FLDRTCT2		(0x021F01)
#define B2_OSWP		    (0x022201)
#define B2_OYUVOCT		(0x022301)
#define B2_OINTCT		(0x022401)
#define B2_OEMBCT		(0x022502)
#define B2_OAOI0HST		(0x022802)
#define B2_OAOI0HEND	(0x022A02)
#define B2_OAOI0VST		(0x022C02)
#define B2_OAOI0VEND	(0x022E02)
#define B2_OAOI1HST		(0x023002)
#define B2_OAOI1HEND	(0x023202)
#define B2_OAOI1VST		(0x023402)
#define B2_OAOI1VEND	(0x023602)
#define B2_OAOI2HST		(0x023802)
#define B2_OAOI2HEND	(0x023A02)
#define B2_OAOI2VST		(0x023C02)
#define B2_OAOI2VEND	(0x023E02)
#define B2_OACTHST		(0x024002)
#define B2_OACTHW		(0x024202)
#define B2_OACTVST		(0x024402)
#define B2_OACTVW		(0x024602)
#define B2_OIMGCT		(0x024801)
#define B2_OFILL		(0x024903)
#define B2_OBKGD0		(0x024C03)
#define B2_OBKGD1		(0x024F03)
#define B2_OBKGD2		(0x025203)
#define B2_AUTOFILLCT	(0x025501)
#define B2_OCSCCF00		(0x025602)
#define B2_OCSCCF01		(0x025802)
#define B2_OCSCCF02		(0x025A02)
#define B2_OCSCCF10		(0x025C02)
#define B2_OCSCCF11		(0x025E02)
#define B2_OCSCCF12		(0x026002)
#define B2_OCSCCF20		(0x026202)
#define B2_OCSCCF21		(0x026402)
#define B2_OCSCCF22		(0x026602)
#define B2_OBIASR		(0x026802)
#define B2_OBIASG		(0x026A02)
#define B2_OBIASB		(0x026C02)
#define B2_OVPHCT		(0x026E01)
#define B2_OVADJCT		(0x026F01)
#define B2_GENLOCKCT	(0x027001)
#define B2_GLGAIN		(0x027101)
#define B2_GLTEST		(0x027202)
#define B2_OVPHRDT		(0x027403)
#define B2_ACTCT		(0x027801)
#define B2_ATBGAIN		(0x027902)
#define B2_HISTGRAMCT	(0x027B01)
#define B2_PTLEVELTH	(0x027C01)
#define B2_ATBPI		(0x027D01)

//B3
#define B3_IIMGCT		(0x030801)
#define B3_ISYCT		(0x030901)
#define B3_IVLNDLY		(0x030A02)
#define B3_IVSKDLY		(0x030C02)
#define B3_IHSKDLY		(0x030E02)
#define B3_AUTOFLD		(0x031001)
#define B3_DIFCT		(0x031101)
#define B3_DHCYCL		(0x031202)
#define B3_DVDLY		(0x031402)
#define B3_ICPNCT		(0x031601)
#define B3_ICPNCT2		(0x031701)
#define B3_IACTHST		(0x031802)
#define B3_IACTHW		(0x031A02)
#define B3_IACTVST		(0x031C02)
#define B3_IACTVW		(0x031E02)
#define B3_IAOIHST		(0x032002)
#define B3_IAOIHEND		(0x032202)
#define B3_IAOIVST		(0x032402)
#define B3_IAOIVEND		(0x032602)
#define B3_ISWP		    (0x032801)
#define B3_ICOLORCT		(0x032901)
#define B3_IBIASR		(0x032A02)
#define B3_IBIASG		(0x032C02)
#define B3_IBIASB		(0x032E02)
#define B3_IGAINR		(0x033002)
#define B3_IGAING		(0x033202)
#define B3_IGAINB		(0x033402)
#define B3_ICSC1CF00	(0x033602)
#define B3_ICSC1CF01	(0x033802)
#define B3_ICSC1CF02	(0x033A02)
#define B3_ICSC1CF10	(0x033C02)
#define B3_ICSC1CF11	(0x033E02)
#define B3_ICSC1CF12	(0x034002)
#define B3_ICSC1CF20	(0x034202)
#define B3_ICSC1CF21	(0x034402)
#define B3_ICSC1CF22	(0x034602)
#define B3_ICSC2CF00	(0x034802)
#define B3_ICSC2CF01	(0x034A02)
#define B3_ICSC2CF02	(0x034C02)
#define B3_ICSC2CF10	(0x034E02)
#define B3_ICSC2CF11	(0x035002)
#define B3_ICSC2CF12	(0x035202)
#define B3_ICSC2CF20	(0x035402)
#define B3_ICSC2CF21	(0x035602)
#define B3_ICSC2CF22	(0x035802)
#define B3_UVLPCOEF0	(0x035A01)
#define B3_UVLPCOEF1	(0x035B01)
#define B3_UVLPCOEF2	(0x035C01)
#define B3_IFILLCT		(0x035D02)
#define B3_IFILL		(0x035E03)
#define B3_PMODE		(0x036001)
#define B3_PIH0		    (0x036202)
#define B3_PIV0		    (0x036402)
#define B3_PIH1		    (0x036602)
#define B3_PIV1		    (0x036802)
#define B3_PTH		    (0x036A01)
#define B3_IPIXVALPOSH	(0x036B02)
#define B3_IPIXVALPOSV	(0x036D02)
#define B3_ACTCT		(0x036F01)
#define B3_ATBGAIN		(0x037002)
#define B3_PTLEVELTH	(0x037201)
#define B3_IFLPCT		(0x037301)
#define	B3_ICOLORADJ	(0x037401)
#define B3_IDVLPCT		(0x037601)
#define B3_IDVLPACTHST	(0x037702)
#define B3_IDVLPACTHW	(0x037902)
#define B3_IDVLPACTVST	(0x037B02)
#define B3_IDVLPACTVW	(0x037D02)
#define B3_I420TEST		(0x037F01)

//B4
#define B4_IPOHCYCL		(0x040802)
#define B4_IPOVCYCL		(0x040A02)
#define B4_IPSYRDLY		(0x040C01)
#define B4_IPOACTHST		(0x040D02)
#define B4_IPOACTHW		(0x040F02)
#define B4_IPOACTVST		(0x041102)
#define B4_IPOACTVW		(0x041302)
#define B4_MVNRCT		(0x041501)
#define B4_HMVNRCFA		(0x041601)
#define B4_HMVNRCFB		(0x041701)
#define B4_HMVNRCFC		(0x041801)
#define B4_HMVNRCFD		(0x041901)
#define B4_VFILCFC		(0x041C01)
#define B4_VFILCFD		(0x041D01)
#define B4_HFILCFA		(0x041E01)
#define B4_HFILCFB		(0x041F01)
#define B4_HFILCFC		(0x042001)
#define B4_HFILCFD		(0x042101)
#define B4_XVFILMD		(0x042201)
#define B4_XNRCUT		(0x042301)
#define B4_LUTWAD		(0x042401)
#define B4_LUTCF		(0x042504)
#define B4_LUTWR		(0x042901)
#define B4_IPADJCT		(0x042A01)
#define B4_IPTEST		(0x042B01)
#define B4_PDCTRL1		(0x042C01)
#define B4_PDCTRL2		(0x042D01)
#define B4_PDMV22TH		(0x042E01)
#define B4_PDMV23TH		(0x042F01)
#define B4_PDSQTH		(0x043001)
#define B4_MVACCSEL		(0x043101)
#define B4_PDACTSEL		(0x043201)
#define B4_PD22RSTCT	(0x043301)
#define B4_PD23RSTTH	(0x043401)
#define B4_PD2332CTRL	(0x043501)
#define B4_PD23322CTRL	(0x043601)
#define B4_PD23232CTRL	(0x043701)
#define B4_PD33CTRL		(0x043801)
#define B4_PD55CTRL		(0x043901)
#define B4_PD87CTRL		(0x043A01)
#define B4_COMBCT		(0x043B01)
#define B4_COMBLTH		(0x043C01)
#define B4_COMBHTH		(0x043D01)
#define B4_MVCMBCT		(0x043E01)
#define B4_MVCMBTH		(0x043F01)
#define B4_MVCMBABSTH	(0x044001)
#define B4_MVCMBPREV	(0x044101)
#define B4_PD22ADJ		(0x044201)
#define B4_PDTEST		(0x044301)
#define B4_PDNRCT		(0x044401)
#define B4_PDMVNRCFA	(0x044501)
#define B4_PDMVNRCFB	(0x044601)
#define B4_PDMVNRCFC	(0x044701)
#define B4_PDMVNRCFD	(0x044801)
#define B4_PDMV22NEWTH	(0x044901)
#define B4_PDMV23NEWTH	(0x044A01)
#define B4_PDSCLMD		(0x044B01)
#define B4_DIAGCT0		(0x045001)
#define B4_DIAGCT1		(0x045101)
#define B4_DIAGCT2		(0x045201)
#define B4_DIAGTH0		(0x045301)
#define B4_DIAGTH1		(0x045401)
#define B4_DIAGTH2		(0x045501)
#define B4_DIAGTH3		(0x045601)
#define B4_DIAGTH4		(0x045701)
#define B4_DIAGTH5		(0x045801)
#define B4_DIAGTH6		(0x045901)
#define B4_DIAGTH7		(0x045A01)
#define B4_DIAGTH8		(0x045B01)
#define B4_DIAGTH9		(0x045C01)
#define B4_DIAGTH10		(0x045D01)
#define B4_DIAGTH11		(0x045E01)
#define B4_DIAGTH12		(0x045F01)
#define B4_DIAGAD0		(0x046001)
#define B4_DIAGAD1		(0x046101)
#define B4_DIAGAD2		(0x046201)
#define B4_DIAGAD3		(0x046301)
#define B4_MVNRMODE		(0x046401)
#define B4_MVNRTH		(0x046501)
#define B4_IPHMON		(0x047C01)
#define B4_ACTMSKCT		(0x047D01)
#define B4_IPHSYRDLY	(0x047E02)

//B5
#define B5_DTCT		(0x050802)
#define B5_DTAD		(0x050A01)
#define B5_DTDT		(0x050B01)
#define B5_DTSFTXR		(0x050C01)
#define B5_DTSFTXG		(0x050D01)
#define B5_DTSFTXB		(0x050E01)
#define B5_DTSFTYR		(0x050F01)
#define B5_DTSFTYG		(0x051001)
#define B5_DTSFTYB		(0x051101)
#define B5_HADPCT		(0x051201)
#define B5_HADPCOEF0	(0x051301)
#define B5_HADPCOEF1	(0x051401)
#define B5_HADPPOS0		(0x051502)
#define B5_HADPPOS1		(0x051702)
#define B5_VADPCT		(0x051901)
#define B5_VADPCOEF0	(0x051A01)
#define B5_VADPCOEF1	(0x051B01)
#define B5_VADPPOS0		(0x051C02)
#define B5_VADPPOS1		(0x051E02)
#define B5_WPACCT		(0x052001)
#define B5_FRCCT		(0x052101)
#define B5_WPBKGDCT		(0x052201)
#define B5_GMCT		    (0x052301)
#define B5_GMBNKSEL		(0x052401)
#define B5_GMAD		    (0x052501)
#define B5_GMDT		    (0x052601)
#define B5_ERDFCT		(0x052701)
#define B5_UCCT		    (0x052802)
#define B5_UCAD		    (0x052A01)
#define B5_UCDT		    (0x052B01)
#define B5_ERDFCT2		(0x052C01)
#define B5_WPLGCT		(0x052E02)
#define B5_WPLGACTHW	(0x053001)
#define B5_WPLGACTVW	(0x053101)
#define B5_WPLGHBOUND0	(0x053201)
#define B5_WPLGHBOUND1	(0x053301)
#define B5_WPLGA		(0x053404)
#define B5_WPLGB		(0x053804)
#define B5_WPLGC		(0x053C04)
#define B5_WPLGD		(0x054004)
#define B5_WPLGE		(0x054404)
#define B5_WPLGF		(0x054804)
#define B5_WPLGG		(0x054C04)
#define B5_WPLGH		(0x055004)
#define B5_WPLGI		(0x055404)
#define B5_WCSCT		(0x055801)
#define B5_WCSAD		(0x055901)
#define B5_WCSDT		(0x055A01)
#define B5_GPLCT		(0x055B01)
#define B5_GPLSFT		(0x055C01)
#define B5_GPLCX		(0x055D02)
#define B5_GPLCY		(0x055F02)
#define B5_GPLMULT		(0x056101)
#define B5_3DCTRL		(0x056402)
#define B5_O3DLACTHW	(0x056602)
#define B5_O3DMACTHW	(0x056802)
#define B5_O3DRACTHW	(0x056A02)
#define B5_M3DLACTHST	(0x056C02)
#define B5_M3DLACTHEND	(0x056E02)
#define B5_M3DRACTHST	(0x057002)
#define B5_M3DRACTHEND	(0x057202)
#define B5_O3DSMLACTHW	(0x057402)
#define B5_O3DSMLMSKHW	(0x057602)
#define B5_WPACTHW		(0x057C02)
#define B5_WPACTVW		(0x057E02)

//B6
#define B6_HLUT0		(0x060801)
#define B6_HLUT1		(0x060901)
#define B6_HLUT2		(0x060A01)
#define B6_HLUT3		(0x060B01)
#define B6_HLUT4		(0x060C01)
#define B6_HLUT5		(0x060D01)
#define B6_HLUT6		(0x060E01)
#define B6_HLUT7		(0x060F01)
#define B6_HLUT8		(0x061001)
#define B6_HLUT9		(0x061101)
#define B6_HLUT10		(0x061201)
#define B6_HLUT11		(0x061301)
#define B6_HLUT12		(0x061401)
#define B6_HLUT13		(0x061501)
#define B6_HLUT14		(0x061601)
#define B6_HLUT15		(0x061701)
#define B6_HLUT16		(0x061801)
#define B6_HLUT17		(0x061901)
#define B6_HLUT18		(0x061A01)
#define B6_HLUT19		(0x061B01)
#define B6_HLUT20		(0x061C01)
#define B6_HLUT21		(0x061D01)
#define B6_HLUT22		(0x061E01)
#define B6_HLUT23		(0x061F01)
#define B6_VLUT0		(0x062001)
#define B6_VLUT1		(0x062101)
#define B6_VLUT2		(0x062201)
#define B6_VLUT3		(0x062301)
#define B6_VLUT4		(0x062401)
#define B6_VLUT5		(0x062501)
#define B6_VLUT6		(0x062601)
#define B6_VLUT7		(0x062701)
#define B6_VLUT8		(0x062801)
#define B6_VLUT9		(0x062901)
#define B6_VLUT10		(0x062A01)
#define B6_VLUT11		(0x062B01)
#define B6_VLUT12		(0x062C01)
#define B6_VLUT13		(0x062D01)
#define B6_VLUT14		(0x062E01)
#define B6_VLUT15		(0x062F01)
#define B6_VLUT16		(0x063001)
#define B6_VLUT17		(0x063101)
#define B6_VLUT18		(0x063201)
#define B6_VLUT19		(0x063301)
#define B6_VLUT20		(0x063401)
#define B6_VLUT21		(0x063501)
#define B6_VLUT22		(0x063601)
#define B6_VLUT23		(0x063701)
#define B6_H2LUT0		(0x063801)
#define B6_H2LUT1		(0x063901)
#define B6_H2LUT2		(0x063A01)
#define B6_H2LUT3		(0x063B01)
#define B6_H2LUT4		(0x063C01)
#define B6_H2LUT5		(0x063D01)
#define B6_H2LUT6		(0x063E01)
#define B6_H2LUT7		(0x063F01)
#define B6_H2LUT8		(0x064001)
#define B6_H2LUT9		(0x064101)
#define B6_H2LUT10		(0x064201)
#define B6_H2LUT11		(0x064301)
#define B6_H2LUT12		(0x064401)
#define B6_H2LUT13		(0x064501)
#define B6_H2LUT14		(0x064601)
#define B6_H2LUT15		(0x064701)
#define B6_H2LUT16		(0x064801)
#define B6_H2LUT17		(0x064901)
#define B6_H2LUT18		(0x064A01)
#define B6_H2LUT19		(0x064B01)
#define B6_H2LUT20		(0x064C01)
#define B6_H2LUT21		(0x064D01)
#define B6_H2LUT22		(0x064E01)
#define B6_H2LUT23		(0x064F01)
#define B6_V2LUT0		(0x065001)
#define B6_V2LUT1		(0x065101)
#define B6_V2LUT2		(0x065201)
#define B6_V2LUT3		(0x065301)
#define B6_V2LUT4		(0x065401)
#define B6_V2LUT5		(0x065501)
#define B6_V2LUT6		(0x065601)
#define B6_V2LUT7		(0x065701)
#define B6_V2LUT8		(0x065801)
#define B6_V2LUT9		(0x065901)
#define B6_V2LUT10		(0x065A01)
#define B6_V2LUT11		(0x065B01)
#define B6_V2LUT12		(0x065C01)
#define B6_V2LUT13		(0x065D01)
#define B6_V2LUT14		(0x065E01)
#define B6_V2LUT15		(0x065F01)
#define B6_V2LUT16		(0x066001)
#define B6_V2LUT17		(0x066101)
#define B6_V2LUT18		(0x066201)
#define B6_V2LUT19		(0x066301)
#define B6_V2LUT20		(0x066401)
#define B6_V2LUT21		(0x066501)
#define B6_V2LUT22		(0x066601)
#define B6_V2LUT23		(0x066701)
#define B6_SCLADPCT		(0x066D01)
#define B6_SCLADPAD		(0x066E01)
#define B6_SCLADPDT		(0x066F01)
#define B6_SCLADPDGTH0	(0x067001)
#define B6_SCLADPDGTH1	(0x067101)
#define B6_SCLADPDGTH2	(0x067201)
#define B6_SCLADPDGTH3	(0x067301)
#define B6_SCLADPDGCF0	(0x067401)
#define B6_SCLADPDGCF1	(0x067501)
#define B6_SCLADPDGCF2	(0x067601)
#define B6_SCLADPDGCF3	(0x067701)
#define B6_WPERRCT		(0x067801)
#define B6_WPHSHLIM		(0x067901)
#define B6_WPVSHLIM		(0x067A01)
#define B6_WPHANGLIM	(0x067B01)
#define B6_WPVANGLIM	(0x067C01)

//B7
#define B7_EGBCT		(0x070802)
#define B7_LEGBHST		(0x070A02)
#define B7_LEGBHW		(0x070C02)
#define B7_REGBHST		(0x070E02)
#define B7_REGBHW		(0x071002)
#define B7_TEGBVST		(0x071202)
#define B7_TEGBVW		(0x071402)
#define B7_BEGBVST		(0x071602)
#define B7_BEGBVW		(0x071802)
#define B7_LEGBMKCT		(0x071A01)
#define B7_REGBMKCT		(0x071B01)
#define B7_TEGBMKCT		(0x071C01)
#define B7_BEGBMKCT		(0x071D01)
#define B7_HEGBINIT		(0x071E01)
#define B7_LEGBCOEF		(0x071F02)
#define B7_REGBCOEF		(0x072102)
#define B7_VEGBINIT		(0x072301)
#define B7_TEGBCOEF		(0x072402)
#define B7_BEGBCOEF		(0x072602)
#define B7_EGBGMAD		(0x072801)
#define B7_EGBGMDT		(0x072901)
#define B7_EGBGMMCCT	(0x072A01)
#define B7_EGBIASCT		(0x072B02)
#define B7_EGBIASAD		(0x072D01)
#define B7_EGBIASDT		(0x072E01)

//B8
#define B8_CURCT		(0x080802)
#define B8_CURTLX		(0x080A02)
#define B8_CURTLY		(0x080C02)
#define B8_CURTRX		(0x080E02)
#define B8_CURTRY		(0x081002)
#define B8_CURBLX		(0x081202)
#define B8_CURBLY		(0x081402)
#define B8_CURBRX		(0x081602)
#define B8_CURBRY		(0x081802)
#define B8_CURHW		(0x081A01)
#define B8_CURVW		(0x081B01)
#define B8_CURBLINKTIME	(0x081C01)
#define B8_CURBLINKCYCL	(0x081D01)
#define B8_CURCPUAD		(0x081E01)
#define B8_CURCPUDT		(0x081F01)
#define B8_CURGAINR		(0x082001)
#define B8_CURGAING		(0x082101)
#define B8_CURGAINB		(0x082201)
#define B8_CURBIASR		(0x082301)
#define B8_CURBIASG		(0x082401)
#define B8_CURBIASB		(0x082501)
#define B8_CURPLTDTR0	(0x082601)
#define B8_CURPLTDTR1	(0x082701)
#define B8_CURPLTDTR2	(0x082801)
#define B8_CURPLTDTR3	(0x082901)
#define B8_CURPLTDTR4	(0x082A01)
#define B8_CURPLTDTR5	(0x082B01)
#define B8_CURPLTDTR6	(0x082C01)
#define B8_CURPLTDTR7	(0x082D01)
#define B8_CURPLTDTR8	(0x082E01)
#define B8_CURPLTDTR9	(0x082F01)
#define B8_CURPLTDTR10	(0x083001)
#define B8_CURPLTDTR11	(0x083101)
#define B8_CURPLTDTR12	(0x083201)
#define B8_CURPLTDTR13	(0x083301)
#define B8_CURPLTDTR14	(0x083401)
#define B8_CURPLTDTR15	(0x083501)
#define B8_CURPLTDTG0	(0x083601)
#define B8_CURPLTDTG1	(0x083701)
#define B8_CURPLTDTG2	(0x083801)
#define B8_CURPLTDTG3	(0x083901)
#define B8_CURPLTDTG4	(0x083A01)
#define B8_CURPLTDTG5	(0x083B01)
#define B8_CURPLTDTG6	(0x083C01)
#define B8_CURPLTDTG7	(0x083D01)
#define B8_CURPLTDTG8	(0x083E01)
#define B8_CURPLTDTG9	(0x083F01)
#define B8_CURPLTDTG10	(0x084001)
#define B8_CURPLTDTG11	(0x084101)
#define B8_CURPLTDTG12	(0x084201)
#define B8_CURPLTDTG13	(0x084301)
#define B8_CURPLTDTG14	(0x084401)
#define B8_CURPLTDTG15	(0x084501)
#define B8_CURPLTDTB0	(0x084601)
#define B8_CURPLTDTB1	(0x084701)
#define B8_CURPLTDTB2	(0x084801)
#define B8_CURPLTDTB3	(0x084901)
#define B8_CURPLTDTB4	(0x084A01)
#define B8_CURPLTDTB5	(0x084B01)
#define B8_CURPLTDTB6	(0x084C01)
#define B8_CURPLTDTB7	(0x084D01)
#define B8_CURPLTDTB8	(0x084E01)
#define B8_CURPLTDTB9	(0x084F01)
#define B8_CURPLTDTB10	(0x085001)
#define B8_CURPLTDTB11	(0x085101)
#define B8_CURPLTDTB12	(0x085201)
#define B8_CURPLTDTB13	(0x085301)
#define B8_CURPLTDTB14	(0x085401)
#define B8_CURPLTDTB15	(0x085501)

//B9
#define B9_OSFLD0		(0x090802)
#define B9_OSFLD1		(0x090A02)
#define B9_OSFLD2		(0x090C02)
#define B9_OSFLD3		(0x090E02)
#define B9_ISFLD0		(0x091004)
#define B9_ISFLD1		(0x091404)
#define B9_ISFLD2		(0x091804)
#define B9_ISFLD3		(0x091C04)
#define B9_OMWI		    (0x092001)
#define B9_IMWI		    (0x092101)
#define B9_MRTOSFLD0	(0x092202)
#define B9_MRTOSFLD1	(0x092402)
#define B9_MRTOSFLD2	(0x092602)
#define B9_MRTOSFLD3	(0x092802)
#define B9_MRTISFLD0	(0x092A02)
#define B9_MRTISFLD1	(0x092C02)
#define B9_MRTISFLD2	(0x092E02)
#define B9_MRTISFLD3	(0x093002)
#define B9_MRTMWI		(0x093201)
#define B9_MRTCT		(0x093301)
#define B9_MRTACTHW		(0x093402)
#define B9_MRTACTVW		(0x093602)
#define B9_MRTVDLY		(0x093802)
#define B9_EGBDBDSAD	(0x093A04)
#define B9_EGBDBDMWI	(0x093E01)
#define B9_EBIASSAD		(0x093F04)
#define B9_EBIASMWI		(0x094301)
#define B9_UCDBDSAD		(0x094404)
#define B9_UCDBDMWI		(0x094801)
#define B9_IPOSFLD0		(0x094904)
#define B9_IPOSFLD1		(0x094D04)
#define B9_IPOSFLD2		(0x095104)
#define B9_IPOSFLD3		(0x095504)
#define B9_IPISFLD0		(0x095904)
#define B9_IPISFLD1		(0x095D04)
#define B9_IPISFLD2		(0x096104)
#define B9_IPISFLD3		(0x096504)
#define B9_IPMWI		(0x096901)
#define B9_IPYOFF		(0x096A02)
#define B9_MVFLD		(0x096C04)
#define B9_WTRSADR		(0x097004)
#define B9_WTRSADG		(0x097404)
#define B9_WTRSADB		(0x097804)
#define B9_WTRMWI		(0x097C01)
#define B9_WTRCT		(0x097D01)

//B10
#define B10_IFH		    (0x0A0802)
#define B10_IFV		    (0x0A0A02)
#define B10_IHW		    (0x0A0C01)
#define B10_IVW		    (0x0A0D01)
#define B10_IMFH		(0x0A0E02)
#define B10_IMFV		(0x0A1003)
#define B10_IHVPHDF0	(0x0A1302)
#define B10_IHVPHDF1	(0x0A1502)
#define B10_FLDHIST		(0x0A1701)
#define B10_PSTAT		(0x0A1801)
#define B10_PVAL0		(0x0A1901)
#define B10_PVAL1		(0x0A1A01)
#define B10_PH0		    (0x0A1B02)
#define B10_PV0		    (0x0A1D02)
#define B10_PH1		    (0x0A1F02)
#define B10_PV1		    (0x0A2102)
#define B10_IPIXR		(0x0A2302)
#define B10_IPIXG		(0x0A2502)
#define B10_IPIXB		(0x0A2702)
#define B10_IPTCNT		(0x0A2902)
#define B10_IAPL0		(0x0A2C04)
#define B10_IMAXFILTR	(0x0A3001)
#define B10_IMINFILTR	(0x0A3101)
#define B10_STATE		(0x0A3201)
#define B10_IPFLDMON	(0x0A3301)
#define B10_IFACTHST	(0x0A3402)
#define B10_IFACTHW		(0x0A3602)
#define B10_IFACTVST	(0x0A3802)
#define B10_IFACTVW		(0x0A3A02)
#define B10_OHISTGRAM	(0x0A3C08)
#define B10_OHISTGRAM_0	(0x0A3C01)
#define B10_OHISTGRAM_1	(0x0A3D01)
#define B10_OHISTGRAM_2	(0x0A3E01)
#define B10_OHISTGRAM_3	(0x0A3F01)
#define B10_OHISTGRAM_4	(0x0A4001)
#define B10_OHISTGRAM_5	(0x0A4101)
#define B10_OHISTGRAM_6	(0x0A4201)
#define B10_OHISTGRAM_7	(0x0A4301)
#define B10_OAPL0		(0x0A4404)
#define B10_MAXFILTR	(0x0A4801)
#define B10_MINFILTR	(0x0A4901)
#define B10_OPTCNT		(0x0A4A03)
#define B10_OFV		    (0x0A4D02)
#define B10_IPOFV		(0x0A4F02)
#define B10_OLNUM		(0x0A5102)
#define B10_ILNUM		(0x0A5302)
#define B10_LVDSRXMON	(0x0A5501)
#define B10_DEVICECODE	(0x0A5D01)
#define B10_AUTOFILLSTATE		(0x0A5E01)
#define B10_PD22CNT1	(0x0A5F01)
#define B10_PD23CNT1	(0x0A6001)
#define B10_PD23CNT2	(0x0A6101)
#define B10_PD23CNT3	(0x0A6201)
#define B10_PD23CNT4	(0x0A6301)
#define B10_PD23CNT5	(0x0A6401)
#define B10_PDMON		(0x0A6501)
#define B10_IPMON		(0x0A6601)
#define B10_IPHCNT1		(0x0A6702)
#define B10_IPHCNT2		(0x0A6902)
#define B10_IPHCNT3		(0x0A6B02)
#define B10_OSDSCSAD	(0x0A6D04)
#define B10_DDR3STATE	(0x0A7101)
#define B10_DDR3LVLSTATE	(0x0A7201)
#define B10_DDR3OCDDOP		(0x0A7301)
#define B10_DDR3OCDDON		(0x0A7401)



#define DEF_OSFLD0		            0x00000000//0x00001c80      //A70LV_Doulas_0098 modify
#define DEF_OSFLD1		            0x00000980//0x00000000
#define DEF_OSFLD2	                0x00001300//0x00000980
#define DEF_OSFLD3  		        0x00001c80//0x00001300


#define DEF_ISFLD0		            0x00000000
#define DEF_ISFLD1		            0x01300000
#define DEF_ISFLD2	                0x02600000
#define DEF_ISFLD3  		        0x03900000

// Cascade
#define DEF_NUM_CASC	            3
#define DEF_CASC_OLD_TBL	        0
#define DEF_CASC_NEW_TBL	        1


// Warp OSD insert position
#define DEF_WP_OSD_INS	            0// 0 : Input, 1 : Output



#if 0
//===== Warp Param =====//
#define DEF_WP_LIM_MODE		        0  // 0 : WpLimit OFF, 1 : WpLimit ON
#define DEF_WP_SPACE		        32 // 32, 64
#define DEF_WP_SPACE_BIT	        ((DEF_WP_SPACE == 64) ? 6 : (DEF_WP_SPACE == 32) ? 5 : 4)

#define DEF_HW_MAX			        4096
#define DEF_VW_MAX			        2176
#define DEF_HW_MAX_GRD		        DEF_HW_MAX/DEF_WP_SPACE//129// = DEF_HW_MAX / DEF_WP_SPACE + 1	[roundup]
#define DEF_VW_MAX_GRD		        DEF_VW_MAX/DEF_WP_SPACE//69// = DEF_VW_MAX / DEF_WP_SPACE + 1	[roundup]



#define DEF_WP_OUT_HGRD		        3// Warptable horizontal grid of outside // fix
#define DEF_WP_OUT_VGRD		        27// Warptable vertical grid of outside
#define DEF_WP_OUT_HW		        (DEF_WP_OUT_HGRD << DEF_WP_SPACE_BIT)
#define DEF_WP_OUT_VW		        (DEF_WP_OUT_VGRD << DEF_WP_SPACE_BIT)

#define DEF_GV_PX_HGRD		        (DEF_WP_OUT_HGRD + DEF_HW_MAX_GRD + DEF_WP_OUT_HGRD)// 3 + 128 + 3 = 134
#define DEF_GV_PX_VGRD		        (DEF_WP_OUT_VGRD + DEF_VW_MAX_GRD + DEF_WP_OUT_VGRD)// 27 + 68 + 27 = 122

#define DEF_HW_DUMMY_GRD	        1// fix
#define DEF_HW_DUMMY		        (DEF_HW_DUMMY_GRD << DEF_WP_SPACE_BIT)// Dummy area(Correction to the outside)
#endif /* 0 */


typedef struct {
	int x;
	int y;
} coord_t;


// y= ax + b
typedef struct {
	float a;
	float b;
} line_coef_ab_t;


typedef struct {
	int st;
	int end;
} pos_t;



//===== Initial value =====//
#define DEF_PB_000_INIT	            0
#define DEF_PB_100_GRID_MV_INIT	    5
#define DEF_PB_200_INIT	            0


#define DEF_BLENDING_EDGE_HW	    200// = PM_EGB_HW
#define DEF_BLENDING_EDGE_VW	    200// = PM_EGB_VW


#define DEF_PB_240_INIT	            0
#define DEF_PB_340_INIT	            0
#define DEF_PB_390_INIT	            0
#define DEF_BLENDING_GAIN_GAMMA	    2.2f// = PM_EGB_GAMMA
#define DEF_OUTPUT_GAMMA	        1.0f// = PM_GAMMA
#define DEF_BLENDING_BIAS_GAMMA	    2.2f// = PM_EBIAS_GAMMA
#define DEF_PB_500_INIT	            0
#define DEF_PB_600_INIT	            0

//===== Grid parameter =====//
//#define DEF_GRID_SIZE		        16// DEF_GRD_SIZE >= 1
//#define DEF_GRID_SIZE_O		        16

//===== 4-Cursor =====//
#define DEF_4CURSOR_SIZE	        16


//Memory Address			//G100_Doulas_0027 Add
#define DEF_MWI			    0x20				// 0x20=32, 32*64=2048
#define OSD_BASIC_ADDR		0x06000000	//size=0x300000*2(2048x1200=0x258000; two layers), addr=0x06000000
#define OSD_TEMP_ADDR		0x06600000	//size=0x300000(2048x1200=0x258000; one layers), addr=OSD_BASIC_ADDR+0x300000*2
#define OSD_FONT_ADDR		0x06900000	//size=50k<0x100000, addr=OSD_BASIC_ADDR+0x300000*3
#define OSD_BMP_ADDR		0x06A00000	//size=100k<0x200000, addr=OSD_FONT_ADDR+0x100000
#define RAM_DATA_ADDR		0x06C00000	//size=0x300000(2048x1200=0x258000), addr=OSD_BMP_ADDR+0x200000
#define OSD_BIAS_ADDR		0x06F00000	//size=0x200000(1024x1200=0x12C000), addr=RAM_DATA_ADDR+0x300000
#define OSD_DBD_ADDR		0x07100000	//size=0x300000(2560x1200=0x2EE000, for 10bit:1920/6*8=2560), addr=OSD_BIAS_ADDR+0x200000

//MWI Definition
#define OSD_BIAS_MWI		0x04		//linefeed width is EBIASMWI[7:0]x256 bytes, but 1 byte contain 2 pixels
#define OSD_DBD_8BIT_MWI	0x08		//linefeed width is EGBDBDMWI[7:0]x256 bytes


//===== Address =====//
#define DEF_OSDSAD			        OSD_BASIC_ADDR//0x06000000	//G100_Doulas_0027
#define DEF_OSDSAD_LAYER_OFFSET     0x240000

#define DEF_OSDMWI			        0x0F
//#define DEF_EBIASSAD		        0x05800000
//#define DEF_EBIASMWI		        0x08
#define TWIST_DEF_BLEND_ADDRESS	    OSD_DBD_ADDR//0x05000000		//G100_Doulas_0027
#define TWIST_DEF_OSDSAD      	    OSD_BASIC_ADDR//0x06000000	//G100_Doulas_0027
#define DEF_BLEND_ADDRESS           OSD_DBD_ADDR//0x05000000		//G100_Doulas_0027
//===== Address =====//
#define DEF_EBIASSAD 				OSD_BIAS_ADDR//0x06800000		//G100_Doulas_0027
#define DEF_EBIASMWI 				OSD_BIAS_MWI//0x04			//G100_Doulas_0027


#define DEF_MEMORY_ADDRESS          0


//===== Warp Lim =====//
//#define DEF_WPLIMANG_H		        1.00	// Limitation of H-slope : tan(45deg)
//#define DEF_WPLIMANG_V		        5.67	// Limitation of V-slope : tan(80deg)
//#define DEF_WPLIMVSH_L		        853		// Limitation of local V-shrink rate : //((Def_Wp_Space==64) ? {64/0.3 *8 = 1706} : {32/0.3 *8 = 853})
//#define DEF_WPLIMVSH_A		        512		// Limitation of averate V-shrink rate : //((Def_Wp_Space==64) ? {64/0.5 *8 = 1024} : {32/0.5 *8 = 512})
//#define DEF_WPLIMHSH		        1536 	// Limitation of H-shrink rate : //((Def_Wp_Space==64) ? {64/(1/6) *8 = 3072} : {32/(1/6) *8 = 1536})
//#define DEF_WPLIMITV_H	            2	// Limitation of horizontal interval of grid : DEF_WPLIMITV_H > 0
//#define DEF_WPLIMITV_V	            2	// Limitation of vertical interval of grid : DEF_WPLIMITV_V > 0

//===== Edgeblend Bias =====//
#define DEF_EBIAS_STEP	            1

//===== Edgeblend Gamma =====//
#define DEF_EGBGM_STEP	            0.005

//===== Output Gamma =====//
#define DEF_OGM_NUM	                33 //Interpolation mode, Thirty-One Table Mode
#define DEF_OGM_COEF_STEP	        0.005

//===== Edgeblend Bias Gamma =====//
#define DEF_EBSGM_STEP	            0.005


//===== 4-Cursor =====//
#define Def_4CURSOR_Size	8

//===== Warp Param =====//
#define Def_WpLimitMode		1// 0 : WpLimit OFF, 1 : WpLimit ON
#define Def_Wp_Space		16// 16, 32
#define Def_Wp_Space_Bit	4//((Def_Wp_Space==32) ? 5 : 4)

#define Def_HW_Max			2048
#define Def_VW_Max			1200
#define Def_HW_Max_GRD		133// = (Def_HW_Max + Def_Wp_Dummy * 2 + Def_Wp_Space - 1) / Def_Wp_Space + 1	[rounddown]
#define Def_VW_Max_GRD		76// = (Def_VW_Max + Def_Wp_Space - 1) / Def_Wp_Space + 1	[rounddown]
#define Def_HST_Min			4
#define Def_HW_Min			64
#define Def_VST_Min			4
#define Def_VW_Min			4

// divide by 0
#define Def_DIV0 0xFFFFFFFF

// Buffer Length
#define Def_BUF_MAX	256

//===== Address =====//
//#define DEF_EBIASSAD 0x06800000		//G100_Doulas_0027
//#define DEF_EBIASMWI 0x04				//G100_Doulas_0027


//===== Gamma ====//
extern float PM_EBIAS_GAMMA[3];
extern int PM_EBIAS_GAMMA_COLOR;
extern char PM_EBIAS_GMDT2[16][3][16];



//===== Edgeblend Bias ====//
extern int                         PM_EBIAS_EN;
extern int                         PM_EBIAS_SEL_CUR_X;
extern int                         PM_EBIAS_SEL_CUR_Y;
extern coord_t                     PM_EBIAS_CUR[2][2];
extern int                         PM_EBIAS_PLT;
extern int                         PM_EBIAS[16][3];
extern int                         PM_EBIAS_AREA_TEST;
extern int                         PM_EBIAS_CUR_MV;
extern int                         PM_EBIAS_COLOR;
extern int                         PM_BOARD;

//===== parameters 1 =====//
extern int                         PS_WP_HW;
extern int                         PS_WP_VW;

//From WPLG Project
//#define DEF_PI			            3.14159265
//#define	DEF_WPDMYHW		            96

// for LUT of interpolation filter
//#define DEF_LUTCUT_MAX	            1.2
//#define DEF_LUTCUT_MIN	            0.1
//#define DEF_LUTWIN_MAX	            2.0
//#define DEF_LUTWIN_MIN	            0


//************************* Enum Start *************************//

typedef enum
{
    eC789_EXEC_CODE_PASS,                   /* pass */
    eC789_EXEC_CODE_FAIL,                   /* general fail indication */
    eC789_EXEC_CODE_FATAL,                  /* fatal error; halt application */
    eC789_EXEC_CODE_PANEL_ID_NOT_FOUND,
    eC789_EXEC_CODE_PANEL_TABEL_INDEX_NOT_FOUND,
}eC789_EXEC_CODE;


/*	SFLCMD[3:0]: Serial flash command select
b3-b0 Serial flash command select
SFLCMD
[3:0]
Operation
0   Idle (Use this setting when reading/writing from CPU.)
1   DMA transfer (Register read selected in SFLREGSEL  serial Flash write)
2   DMA transfer (Serial Flash read  Register write selected in SFLREGSEL)
3   DMA transfer (Serial Flash read  FONT data write)
    Use this option when expanding 24-bit FONT data to 32 bits. Keep SLFREGSEL at 0.
4   Sector deletion Issue the command followed by address and pole the status.
5   Bulk erase Issue the command and pole the status.
6-9 Reserved
10  DMA transfer (Serial Flash read to internal register write)
    Format of data on the serial Flash: register address, register value, register
    address, register value …
11-13 Reserved
14  Read operation with SFLGCMD (store read data to SFLGDT with the lowest byte
    first): with SFLCNT, set the number of bytes between 1 and 4.
15  Write operation with SFLGCMD (send out write data from SFLGDT with the lowest
byte first): with SFLCNT, set the number of bytes between 1 and 4.*/

typedef enum
{
    eC789_SFLCT_IDEL            = 0,
    eC789_SFLCT_DMA_REG2FLASH   = 1,
    eC789_SFLCT_DMA_FLASH2REG   = 2,
    eC789_SFLCT_DMA_FLASH2REGF  = 10, //A70LV_Larry_0174

}eC789_SFLCT; //A70LV_Larry_0139

/*
0 DDR3-SDRAM
1 OSD Character Buffer
2 OSD Color Pallete
3 De-inter lacer LUT
4 Distortion Correction LUT
5 Output Image Gamma LUT
6 Output Image Uniformity LUT
7 Adaptive Scale Interpolation Filter LUT
8 Edge Blend Gain Gamma LUT
9 Edge Blend Bias LUT 7 0x2E
A Cursor
*/

typedef enum
{
    eC789_SFLREGSEL_DDR     = 0x00,
    eC789_SFLREGSEL_OCB     = 0x01,
    eC789_SFLREGSEL_OCP     = 0x02,
    eC789_SFLREGSEL_DILL    = 0x03,
    eC789_SFLREGSEL_DCL     = 0x04,
    eC789_SFLREGSEL_OIGL    = 0x05,
    eC789_SFLREGSEL_OIUL    = 0x06,
    eC789_SFLREGSEL_ASIF    = 0x07,
    eC789_SFLREGSEL_EBGL    = 0x08,
    eC789_SFLREGSEL_EBBL    = 0x09,
    eC789_SFLREGSEL_CURSOR  = 0x0A,
    eC789_SFLREGSEL_NA      = 0xFF, //A70LV_Larry_0174

}eC789_SFLREGSEL; //A70LV_Larry_0139

#if 0
typedef enum
{
    eC789_WARP_MODE_2x2,
    eC789_WARP_MODE_3x3,
    eC789_WARP_MODE_5x5,
    eC789_WARP_MODE_9x9,
    eC789_WARP_MODE_17x17,
}eC789_WARP_MODE;

typedef enum
{
    eC789_WARP_CROSSHATCH_MODE_OFF,
    eC789_WARP_CROSSHATCH_MODE_SINGLE,
    eC789_WARP_CROSSHATCH_MODE_OVERLAY,
    eC789_WARP_CROSSHATCH_MODE_LAST,
}eC789_WARP_CROSSHATCH_MODE;

typedef enum
{
    eC789_COLOR_RED,
    eC789_COLOR_GREEN,
    eC789_COLOR_BLUE,
    eC789_COLOR_LAST,
}eC789_COLOR;

typedef enum
{
	eC789_WARP_OK,
	eC789_WARP_ERR_ANG_H,// Error of H-slope
	eC789_WARP_ERR_ANG_V,// Error of V-slope
	eC789_WARP_ERR_VSH_LOC,// Error of local V-shrink rate
	eC789_WARP_ERR_VSH_AVE,// Error of average V-shrink rate
	eC789_WARP_ERR_HSH,// Error of H-shrink rate
	eC789_WARP_ERR_OUTSIDE,// Error of outside of ACT
	eC789_WARP_ERR_INTERVAL,// Error of grid interval
	eC789_WARP_NGZ,
	eC789_WARP_CURPOS,
	eC789_WARP_ERR// Error of other reasons


}eC789_WARP_ERROR;

typedef enum
{
    eC789_WARP_DIRECTION_UP,
    eC789_WARP_DIRECTION_DOWN,
    eC789_WARP_DIRECTION_LEFT,
    eC789_WARP_DIRECTION_RIGHT,
    eC789_WARP_DIRECTION_LAST,
}eC789_WARP_DIRECTION;
#endif /* 0 */

//===================== Struct Start ===================//
typedef struct
{
	UINT8 		        ucFrameRate;
	UINT16		        uiPixClk;
	UINT16		        uiHStart;
	UINT16		        uiHActive;
	UINT16		        uiVStart;
	UINT16		        uiVActive;
	UINT16		        uiHTotal;
	UINT16		        uiVTotal;
	UINT8		        ucHSyncWidth;
	UINT8				ucVSyncWidth;
    UINT8				ucPoRefDiv;
    UINT8				ucPoFbDiv;
#ifdef Low_Latency_All
    UINT8				ucLow_Latency;	//ZU860_Clare_0152
#endif	/*Low_Latency_All*/
}sOUTPUT_TIMING_INFO, *PsOUTPUT_TIMING_INFO;


typedef struct
{
	DOUBLE 		        dFrameRate;
	UINT16		        uiHStart;
	UINT16		        uiHActive;
	UINT16		        uiVStart;
	UINT16		        uiVActive;
    //Input Port Measure Reference CLK by PICLK
	UINT16		        uiHTotal;           //g_ihcycl_i
    //Input Port Measure Reference CLK by MCLK.Depends on B17_DIFCTCH bit[7]
	UINT16		        uiHTotal_MCLK;      //g_ihcycl_m
	UINT16		        uiVTotal;           //g_ivcycl_i

}sINPUT_TIMING_INFO, *PsINPUT_TIMING_INFO;



typedef struct
{
    ePANEL_ID                   ePanelId;
    sOUTPUT_TIMING_INFO         sOutputTimingInfo;
}sPANEL_INFO, *PsPANEL_INFO;



#if 0
typedef struct
{
    UINT32 ulX;
    UINT32 ulY;
}sCOORDINATE_T, *PsCOORDINATE_T;

typedef struct
{
    float fX;
    float fY;
}sCOORDINATE_F, *PsCOORDINATE_F;

typedef struct
{
	float fA;
	float fB;
	float fC;
	float fD;
}sLINE_COEF_T, * PsLINE_COEF_T;
#endif /* 0 */

typedef struct
{
    BOOL                        bInit;
    ePANEL_ID                   ePanelTimingId;           //Pass from up layer to get panel output timing

    UINT8						ucPanelIndex;             //To save panel table index
    sOUTPUT_TIMING_INFO         sOutputTimingInfo;

    UINT32                      ulMclko_Freq;


    //===== parameters 1 =====//
    UINT16                      uiPanelHst;
    UINT16                      uiPanelVst;
    UINT16                      uiPanelHW;          // PS_WP_HW, Panel Width, 3840
    UINT16                      uiPanelVW;          // PS_WP_VW, Panel Height, 2160
    UINT16                      uiPixelGridHW;      // PS_WP_HW_GRD, Grid Width / 32 pixel mode or 64 pixel mode = how many interpoation grid size
    UINT16                      uiPixelGridVW;      // PS_WP_VW_GRD, Grid Height / 32 pixel mode or 64 pixel mode = how many interpoation grid size


    UINT8                       ucWarpColor;        // GV_WP_TABLE_COLOR, in 32 pixel space, there A,B,C three table can be used.Refer to B14_DTCT
                                                    //                    in 64 pixel space, there A,B,C,D fourc table can be used.
    BOOL                        bWPTblSel;




    float                       faCursorDefInX[17];             // PS_CUR_DEF_IX, real coordinate in the 3840x2160 canvas
    float                       faCursorDefInY[17];             // PS_CUR_DEF_IY, real coordinate in the 3840x2160 canvas
    float                       faCursorDefOutX[17];            // PS_CUR_DEF_OX, real coordinate in the 3840x2160 canvas
    float                       faCursorDefOutY[17];            // PS_CUR_DEF_OY, real coordinate in the 3840x2160 canvas

//    UINT16                      uiIACT_HW;// PM_IACT_HW;
//    UINT16                      uiIACT_vW;// PM_IACT_VW;

#if 0
    //===== Warping ====//

    sCOORDINATE_F               saGridPos[DEF_NUM_CASC][17][17];// PM_GRID, Cascade
    sCOORDINATE_F               saPrevGridPos[17][17];          // PM_GRID_OLD
#endif /* 0 */



    // EdgeBlending Gain
    UINT16                      uiBlendingEdgeHW;               // PM_EGB_OHW
    UINT16                      uiBlendingEdgeVW;               // PM_EGB_OVW
    BOOL                        bBlendingGainEnable;            // PM_EGB_EN
    BOOL                        bBlendingGainEnable_T;          // PM_EGB_TEN
    BOOL                        bBlendingGainEnable_B;          // PM_EGB_BEN
    BOOL                        bBlendingGainEnable_L;          // PM_EGB_LEN
    BOOL                        bBlendingGainEnable_R;          // PM_EGB_REN

    // EdgeBlending Bias
    BOOL                        bBlendingBiasEnable;            // PM_EBIAS_EN
    UINT8                       ucBlendingBiasCursorX;          // PM_EBIAS_SEL_CUR_X
    UINT8                       ucBlendingBiasCursorY;          // PM_EBIAS_SEL_CUR_Y
    //sCOORDINATE_T               saBlendingBiasCursorPos[2][2];  // PM_EBIAS_CUR

    //UINT8                       ucaBlendingBiasPalleteColor[16][eC789_COLOR_LAST]; // PM_EBIAS

    BOOL                        bBlendingBiasAreaTest;              // PM_EBIAS_AREA_TEST
    UINT16                      uiBlendingBiasCursorMoveStep;       // PM_EBIAS_CUR_MV
    UINT8                       ucBlendingBiasColorSel;             // PM_EBIAS_COLOR

    // Blending Gain Gamma
    BOOL                        bBlendingGainGammaEnable;           // PM_EGB_GAMMA_EN
    //float                       faBlendingGainGamma[eC789_COLOR_LAST];  // PM_EGB_GAMMA, R, G, B
    UINT8                       ucBlendingGainGammaColorSel;        // PM_EGB_GAMMA_COLOR
    // Output Gamma
    BOOL                        bOutputGammaEable;                  // PM_GAMMA_EN, Output Gamma
    float                       faOutputGamma[DEF_OGM_NUM][3];      // PM_GAMMA, 33 output gamma point
    UINT16                      uiaOutputGammaPixel[DEF_OGM_NUM];   // PM_GAMMA_PIX, Each point pixel pos
    UINT16                      uiOutputGammaPixelSel;              // PM_GAMMA_PIX_SEL
    UINT8                       ucOutputGammaColor;                 // PM_GAMMA_COLOR

    // Blending Bias Gamma
    //float                       faBlendingBiasGamma[eC789_COLOR_LAST];              // PM_EBIAS_GAMMA
    UINT8                       ucBlendingGammaColor;                               // PM_EBIAS_GAMMA_COLOR
    //UINT16                      uiBlendingBiasGammaData[16][eC789_COLOR_LAST][16];  //PM_EBIAS_GMDT[16][3][16];

    INT16                       iDummyHstReset;                                    // PS_DUMMY_HST_REST
    INT16                       iDummyHendGridReset;                                // PS_DUMMY_HEND_GRD_REST

    //===== Warp Table parameters ====//
    //float                       faPosX[2][DEF_GV_PX_HGRD][DEF_GV_PX_VGRD];// GV_PX, cascade
    //float                       faPosY[2][DEF_GV_PX_HGRD][DEF_GV_PX_VGRD];// GV_PY, cascade


    //===== OSD Cursor parameters ====//
    UINT16                      uiNumHGrid;  // GV_NUM_HGRID, number grid, related to warp mode 2x2, 3x3...
    UINT16                      uiNumVGrid;  // GV_NUM_VGRID, number grid, related to warp mode 2x2, 3x3...
    UINT16                      uiNumHGrid_M1;//GV_NUM_HGRID_m1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_M1;//GV_NUM_VGRID_m1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumHGrid_M2;//GV_NUM_HGRID_m2, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_M2;//GV_NUM_VGRID_m2, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumHGrid_P1;//GV_NUM_HGRID_p1, for calc_h_line_coef(), calc_v_line_coef()
    UINT16                      uiNumVGrid_P1;//GV_NUM_VGRID_p1, for calc_h_line_coef(), calc_v_line_coef()




#if 0
    //===== Warp Parameters ====//
    sLINE_COEF_T                saWarpVerLine[17][16];// GV_WP_VER_LINE
    sLINE_COEF_T                saWarpHorLine[16][17];// GV_WP_HOR_LINE
    sLINE_COEF_T                saWarpHorLineMove[16];// GV_WP_HOR_LINE_MV
    sLINE_COEF_T                saWarpVerLineMove[DEF_HW_MAX_GRD][16];// GV_WP_VER_LINE_MV
    sCOORDINATE_F               saCalPoint[17][17];// GV_CP
    BOOL                        baHGridCalEnable[17];// GV_HGRID_CLC_EN[17]
    BOOL                        baVGridCalEnable[17];// GV_VGRID_CLC_EN[17]
    //===== Warp_2x2 Parameters ====//
    float                       faPjcA[3];          // GV_PJC_A
    float                       faPjcB[3];          // GV_PJC_B
    float                       faPjcC[3];          // GV_PJC_C
#endif /* 0 */

    //===== Gamma Parameters ====//
    UINT16                      uiTmpGammaTable[1024];// GV_TEMP_GAMMA_TABLE[1024]

    //===== Register parameters =====//
    UINT16                      uiRegRTCT;          // GV_RTCT
    UINT16                      uiRegEGBCT;         // GV_EGBCT;


    //===== Test Pattern ====//
    //eC789_WARP_CROSSHATCH_MODE  ePatternGen;        //PM_PATGEN;
    BOOL                        bEdgeMarkerEnable;  //PM_EGBMK_EN;
    BOOL                        bOfillEnable;       //PM_OFILL_EN;

    UINT8                       ucRegISYCT;  //A35G2_CDS_Simon_0003

}sDRV_C789_INFO, *PsDRV_C789_INFO;

typedef struct
{
	float	fAxis;
	float	fDist;
	float	fVw;
}sPJPARAM, *PsPJPARAM;

// 4-corner Keystone
typedef struct
{
	UINT16 uiTL_x;
    UINT16 uiTL_y;					// 4(X,Y)
	UINT16 uiTR_x;
    UINT16 uiTR_y;					// 4(X,Y)
	UINT16 uiBL_x;
    UINT16 uiBL_y;					// 4(X,Y)
	UINT16 uiBR_x;
    UINT16 uiBR_y;					// 4(X,Y)
}sWPPOS4C, *PsWPPOS4C;

typedef struct
{
    float fX;
    float fY;
    float fZ;
}s3DPOS, *Ps3DPOS;

typedef struct
{
    float fX;
    float fY;
}s2DPOS, *Ps2DPOS;

//======================================================

BOOL dvC789_SemaphoreTake(BOOL cEnable, const char *pcSemaphore);
UINT32 dvC789_Read(UINT32 ulAddr);
void dvC789_Write(UINT32 ulAddr, UINT32 ulData);
void dvC789_Buffer_Flush(void);
void dvC789_WriteToBuffer(UINT32 ulAddr, UINT32 ulData);
void dvC789_BurstRead_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0139
void dvC789_BurstWrite_FixedAdd(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData);
void dvC789_BurstRead_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0139
void dvC789_BurstWrite_AddInc(const UINT32 ulAddr, UINT16 uiSize, UINT8 *pucData);
#ifdef Low_Latency_All
eC789_EXEC_CODE dvC789_Init(const ePANEL_ID ePanelId, BOOL bbLow_Latency);	//ZU860_Clare_0152//A70LV_Doulas_0036
#else
eC789_EXEC_CODE dvC789_Init(const ePANEL_ID ePanelId);      //A70LV_Doulas_0036
#endif	/*Low_Latency_All*/
void dvC789_InputSwapSet(UINT8 ucVal);
void dvC789_TestPatternSet(UINT8 ucVal);
void dvC789_TestMode(void);
void dvC789_V_Start_Checking(void);  //A70LV_Doulas_0042
void dvC789_ForcedSyncResetValueSet(UINT8 ucForcedSyncReset,UINT16 uiH_Total,UINT16 uiV_Total);     //A70LV_Doulas_0079
void dvC789_OutputEnableSet(UINT8 ucEnable);
UINT8 dvC789_OutputEnableGet(void);
eC789_EXEC_CODE dvC789_Change_Panel(const ePANEL_ID ePanelId);       //A70LV_Doulas_0098
void dvC789_FlashEraseSector(UINT32 ulSflAd, UINT32 ulDataSize); //A70LV_Larry_0139
void dvC789_DMA_Transfer(UINT32 ulAddress, UINT32 ulSize, UINT8 ucRegsel, UINT32 ulDmaCnt); //A70LV_Larry_0139
void dvC789_FlashWrite(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0174
void dvC789_FlashRead(UINT32 ulAddress, UINT16 uiSize, UINT8 *pucData); //A70LV_Larry_0174
void dvC789_FrmaeDelaySet(UINT8 ucValue);       //A70LV_Doulas_0154
UINT16 dvC789_Output_V_Total_Get(void);   //A70LV_Doulas_0154
void dvC789_RegDump(void);      //A70LV_Doulas_0170
UINT8 dvC789_OutputChanged_Get(void);        //A70LV_Doulas_0307
void dvC789_OutputChanged_Set(UINT8 ucVal);        //A70LV_Doulas_0307
void dvC789_ColorUniformity_Control(UINT8 ucEnable, UINT8 ucCPUAccessEnable);
UINT8 dvC789_ColorUniformityEnable_Get(void);    //A65_OPTOMA_CL_0001  //A35G2_CDS_CL_0001
void dvC789_ColorUniformityEnable_Set(UINT8 ucVal);
void dvC789_COMS_Output_Set(UINT8 ucEnable);     //A70LV_Doulas_0361
UINT8 dvC789_OutputV_FreqGet(void);     //A70LV_Doulas_0367
void dvC789_Test(UINT16 *puiTtlCnt, UINT16 *puiErrCnt);
#ifdef Low_Latency_All
void dvC789_Low_Latency_Set(UINT8 ucVal);	//ZU860_Clare_0152
BOOL dvC789_Low_Latency_Get(void);	//ZU860_Clare_0152
#endif	/*Low_Latency_All*/

void dvC789_SetEdgebldBiasCursorEnable(UINT8 ucEnable);
void dvC789_SetEdgebldBiasCursorPos( int selx, int sely, int dx, int dy );
void dvC789_EdgebldBiasEnable(INT8 cEnable);
void dvC789_SetEdgebldBiasPalette( int plt, int r, int g, int b );
void dvC789_BlackLevel_AreaWrite( int plt_sel, coord_t P1, coord_t P2, coord_t P3, coord_t P4 , int iDataWrite);
void WPLT2( int plt_sel, pos_t x, int y );
line_coef_ab_t CalcCoef( const coord_t *p0, const coord_t *p1 );
pos_t CalcLineX( const int *y_crt, const coord_t *p_b, const coord_t *p_t, const line_coef_ab_t *coef );
pos_t XMinMax( const pos_t x[], int num );
pos_t YMinMax( const coord_t p[], int num );
char *FileNameEBAreaBak( int num );
int LoadEgbBiasArea( char fn[] ) ;
void dvC789_CalcEdgebldBiasGammaTable( int color, int plt, int allplt );
void dvC789_EdgebldBiasGammaTable( int plt );
void dvC789_EdgebldBiasGammaTableClear( void );
int dvC789_LoadEgbBiasArea( char fn[] );
void dvC789_Blacklevel_Palette_Set(INT8 cPaletteIdx, UINT8 cRed, UINT8 cGreen, UINT8 cBlue);

char HexToChar( int val );
int CharToHex( char ch );
void dvC789_Dummy(void);
void dvC789_SetOSDMode(bool bFront);
void dvC789_OSDACTStartConfig(bool bFront);
UINT32 dvC789_V_Start_Get(void);    //G100_Owen_0078
void dvC789_ConfigMemAD(void);
void dvC789_ISYCT_Set(UINT8 uiRegValue);  //A35G2_CDS_Simon_0003
UINT8 dvC789_ISYCT_Get(void);  //A35G2_CDS_Simon_0003
void dvC789_AdaptiveScaleFilterLutInit(void); //A65_OPTOMA_CL_0018  //A35G2_Simon_0115
void dvC789_AdaptiveScaleFilterLutEnable(UINT8 ucEnable);  //A35G2_Simon_0115


#endif /* DVC789_H */

