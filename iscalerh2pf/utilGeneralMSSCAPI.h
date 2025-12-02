// ===============================================================================
// FILE NAME: utilGeneralMSSCAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================

#ifndef UTILGENERALMSSCAPI_H
#define UTILGENERALMSSCAPI_H

#include "Common.h"

#ifdef GENERAL_MSSC_CLI
#ifdef __cplusplus
extern "C" {
#endif

#define GENERAL_CMD_HEADER_1       '['
#define GENERAL_CMD_END_1          ']'

#define GENERAL_CMD_TEXT_SIZE      (210)

#define GENERAL_MAIN_CMD_LEN       (4)
#define GENERAL_SUB_CMD_LEN        (4)
#define GENERAL_CHECKSUM_LEN       (3)
#define GENERAL_ADDRESS_LEN_MAX    (5)

#if CUSTOM_OPTOMA
#define AMXB_STR    "AMXB<-SDKClass=VideoProjector><-Make=OPTOMA><-Model=%s><-Revision=1.0.0>\r"
#else
#define AMXB_STR    "AMXB<-SDKClass=VideoProjector><-Make=BARCO><-Model=%s><-Revision=1.0.0>\r"
#endif

#define AMXB_NEUTRAL_STR    "AMXB<-SDKClass=VideoProjector><-Make=General><-Model=%s><-Revision=1.0.0>\r" //A35G2_Coda_0063

typedef struct
{
    char cMainCode[GENERAL_MAIN_CMD_LEN + 1]; //+1 for '\0'
    char cSubCode[GENERAL_SUB_CMD_LEN + 1]; //+1 for '\0'

    UINT8 ucCmdFrom; //from PC or telnet ...    //T100_Simon_0020
    char  cTextString[GENERAL_CMD_TEXT_SIZE + 1]; //store text prameter in Data. //+1 for '\0'
    INT32 lData;
    float fData;
    UINT8 ucHasSubCode;
    UINT8 ucDataType; // 0--> Decimal, 1:String.
    UINT8 ucIsRead;  //Write, Read
    UINT8 ucPrefixCharType; //$, #, &

    char uiProjectorAddress[GENERAL_ADDRESS_LEN_MAX + 1]; //+1 for '\0'
    UINT32 ulProjectorAddressID;

    UINT8 ucHasHost;
    char uiProjectorHost[GENERAL_ADDRESS_LEN_MAX + 1]; //+1 for '\0'
    UINT32 ulProjectorHostID;

    UINT16 uiFuncID;
}sCLI_GENERAL_FORMAT;

void utilGeneral_CLI_Init(void);
void utilGeneral_CLI_AddressID(UINT32 ulAddress);  //G100_Wilsonj_0050
UINT8 utilGeneral_CLI_Handle(UINT8 cCh, UINT8 ucData);
UINT8 utilGeneral_CLI_Process(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_GENERAL_FORMAT* sCmdFormat, UINT16 uiType);
void utilGeneral_CLI_AMX(UINT8 ucData);
UINT8 utilGeneral_CheckBit(UINT8 cValue, UINT8 cbit ); //G100_Steven_0101 //A35G2_BRC_Casper_0047
void utilGeneral_CLI_SerialPortEcho(UINT8 eCh, UINT8 ucEcho);

#ifdef __cplusplus
}
#endif
#endif /* GENERAL_MSSC_CLI */

#endif /* UTILGENERALMSSCAPI_H */



