/*
 * CustomerDef.h
 *
 *  Created on: Sep 5, 2018
 *      Author: bennytseng
 */

#ifndef _CUSTOMERDEF_H_
#define _CUSTOMERDEF_H_
//==============================================================================
#ifdef CHRISTIE
    #define CUSTOMER_CHRISTIE
    #define CUSTOMER_ID (0x01)
#elif defined OPTOMA
    #define CUSTOMER_OPTOMA
    #define CUSTOMER_ID (0x02)
#elif defined BASIL
    #define CUSTOMER_BARCO
    #define CUSTOMER_ID (0x03)
#elif defined DONVIEW
    #define CUSTOMER_DONVIEW
    #define CUSTOMER_ID (0x04)
#elif defined EIKI
    #define CUSTOMER_EIKI
    #define CUSTOMER_ID (0x05)
#elif defined RICOH
    #define CUSTOMER_RICOH
    #define CUSTOMER_ID (0x06)
#elif defined KRINDA
    #define CUSTOMER_KRINDA
    #define CUSTOMER_ID (0x07)
#elif defined WINCOMN
    #define CUSTOMER_WINCOMN
    #define CUSTOMER_ID (0x08)
#elif defined AVIC
    #define CUSTOMER_AVIC
    #define CUSTOMER_ID (0x09)
#elif defined DHN
    #define CUSTOMER_DHN
    #define CUSTOMER_ID (0x0A)
#else
    #define CUSTOMER_GENERAL
    #define CUSTOMER_ID (0xFF)
#endif
//==============================================================================
#ifdef T100
    #define PROJECT_T100
    #define PROJECT_ID  (0x01)
#elif defined A70
    #define PROJECT_A70
    #define PROJECT_ID  (0x02)
#elif defined B35LC
    #define PROJECT_B35LC
    #define PROJECT_ID  (0x03)
#elif defined B35LBPLUS
    #define PROJECT_B35LBPLUS
    #define PROJECT_ID  (0x04)
#elif defined A65
    #define PROJECT_A65
    #define PROJECT_ID  (0x05)
#elif defined R70
    #define PROJECT_R70
    #define PROJECT_ID  (0x06)
#elif defined T50
    #define PROJECT_T50
    #define PROJECT_ID  (0x07)
#else
    #define PROJECT_GENERAL
    #define PROJECT_ID  (0xFF)
#endif
//==============================================================================
// T100
// Christie : LH/LD
// Optoma   : ZK1050/ZK750
#ifdef LH
    #define MODEL_ID    (0x01)
#elif defined LD
    #define MODEL_ID    (0x02)
#elif defined ZK1050
    #define MODEL_ID    (0x01)
#elif defined ZK750
    #define MODEL_ID    (0x02)
#endif

// A70
// Christie : WU/HD/LHR
// Krinda   : CWU2000L
#ifdef WU
    #define MODEL_ID    (0x01)
#elif defined HD
    #define MODEL_ID    (0x02)
#elif defined LHR
    #define MODEL_ID    (0x03)
#elif defined CWU2000L
    #define MODEL_ID    (0x01)
#endif

// B35LC
// Optoma   : ZU860
// EIKI     : EK812U/EK818U
// Donview  : SNPLU9000
#ifdef ZU860
    #define MODEL_ID    (0x01)
#elif defined EK812U
    #define MODEL_ID    (0x01)
#elif defined EK818U
    #define MODEL_ID    (0x02)
#elif defined SNPLU9000
    #define MODEL_ID    (0x01)
#endif

// B35LBPLUS
// Optoma   : ZU750/ZU750AGC
// Basil    : G60W7/G60W8/G60W8AGC/G60W10
// Donview  : SNPLU7500/SNPLU7500AGC/SNPLU8001AGC/SNPLU8001USAAGC/SNPLU8001EUAGC
// AVIC     : LP60SLWU
#ifdef ZU750
    #define MODEL_ID    (0x01)
#elif defined ZU750AGC
    #define MODEL_ID    (0x02)
#elif defined G60W7
    #define MODEL_ID    (0x01)
#elif defined G60W8
    #define MODEL_ID    (0x02)
#elif defined G60W8AGC
    #define MODEL_ID    (0x03)
#elif defined G60W10
    #define MODEL_ID    (0x04)
#elif defined SNPLU7500
    #define MODEL_ID    (0x01)
#elif defined SNPLU7500AGC
    #define MODEL_ID    (0x02)
#elif defined SNPLU8001AGC
    #define MODEL_ID    (0x03)
#elif defined SNPLU8001USAAGC
    #define MODEL_ID    (0x04)
#elif defined SNPLU8001EUAGC
    #define MODEL_ID    (0x05)
#elif defined LP60SLWU
    #define MODEL_ID    (0x01)
#endif

// A65
// Basil   : G100W16/G100W163D/G100W20/G100W203D
#ifdef G100W16
    #define MODEL_ID    (0x01)
#elif defined G100W163D
    #define MODEL_ID    (0x02)
#elif defined G100W20
    #define MODEL_ID    (0x03)
#elif defined G100W203D
    #define MODEL_ID    (0x04)
#endif

// R70
#ifdef DU7100
    #define MODEL_ID    (0x01)
#elif defined DU8200
    #define MODEL_ID    (0x02)
#endif

// Krinda T50
#ifdef KRINDAT50
    #define MODEL_ID    (0x01)
#endif

#ifndef MODEL_ID
    #define MODEL_ID    (0xFF)
#endif
//==============================================================================
#ifndef FLAG_ID
    #define FLAG_ID     (0x00)
#endif
//==============================================================================
#define PROJECTOR_ID    ((PROJECT_ID << 24) | (CUSTOMER_ID << 16) | (MODEL_ID << 8) | (FLAG_ID << 0))
//==============================================================================
#endif /* _CUSTOMERDEF_H_ */
