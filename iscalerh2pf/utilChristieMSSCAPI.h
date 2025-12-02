#if defined(CUSTOM_CHRISTIE)
// ===============================================================================
// FILE NAME: utilChristieMSSCAPI.h
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2017/10/08, Larry Create
// --------------------
// ===============================================================================

#ifndef UTILCHRISTIEMSSCAPI_H
#define UTILCHRISTIEMSSCAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Common.h"

#define CHRISTIE_CMD_HEADER_1       '('
#define CHRISTIE_CMD_HEADER_2       '<'

#define CHRISTIE_CMD_END_1          ')'
#define CHRISTIE_CMD_END_2          '>'

#define CHRISTIE_CMD_TEXT_SIZE      (210)

#define CHRISTIE_MAIN_CMD_LEN       (3)
#define CHRISTIE_SUB_CMD_LEN        (4)
#define CHRISTIE_CHECKSUM_LEN       (3)
#define CHRISTIE_ADDRESS_LEN_MAX    (5)

#define AMXB_STR_CHRISTIE    "AMXB<-SDKClass=VideoProjector><-Make=CHRISTIE><-Model=%s><-Revision=1.0.0>\r"

typedef struct
{
    char cMainCode[CHRISTIE_MAIN_CMD_LEN + 1]; //+1 for '\0'
    char cSubCode[CHRISTIE_SUB_CMD_LEN + 1]; //+1 for '\0'

    UINT8 ucCmdFrom; //from PC or telnet ...    //T100_Simon_0020
    char  cTextString[CHRISTIE_CMD_TEXT_SIZE + 1]; //store text prameter in Data. //+1 for '\0'
    INT32 lData;
    float fData;
    UINT8 ucHasSubCode;
    UINT8 ucDataType; // 0--> Decimal, 1:String.
    UINT8 ucIsRead;  //Write, Read
    UINT8 ucPrefixCharType; //$, #, &

    char uiProjectorAddress[CHRISTIE_ADDRESS_LEN_MAX + 1]; //+1 for '\0'
    UINT32 ulProjectorAddressID; //A70LV_Larry_0385

    UINT8 ucHasHost; //A70LV_Larry_0385
    char uiProjectorHost[CHRISTIE_ADDRESS_LEN_MAX + 1]; //+1 for '\0'
    UINT32 ulProjectorHostID; //A70LV_Larry_0385

    UINT16 uiFuncID;
}sCLI_CHRISTIE_FORMAT;

void utilChristie_CLI_Init(void);
void utilChristie_CLI_AddressID(UINT32 ulAddress); //A70LV_Larry_0385
UINT8 utilChristie_CLI_Handle(UINT8 eCh, UINT8 *pcData);
void utilChristie_CLI_SerialPortEcho(UINT8 eCh, UINT8 ucEcho); //A70LV_Larry_0060
void utilChristie_CMD_Reply(UINT8 ucChannel, UINT8 ucErrorCode, sCLI_CHRISTIE_FORMAT* sCmdFormat, char *ucReturnString);
UINT8 utilChristie_CMD_Decode(UINT8* pcString, sCLI_CHRISTIE_FORMAT* sCmdFormat);
UINT8 utilChristie_CMD_Handle(sCLI_CHRISTIE_FORMAT* sCmdFormat);
UINT8 utilChristie_CLI_Process(UINT8 cCh, UINT8* pcString, char* pcReturnString, sCLI_CHRISTIE_FORMAT* sCmdFormat, UINT16 uiType); //A70LV_Larry_0388
void utilChristie_CLI_AMX(UINT8 ucData);

#ifdef __cplusplus
}
#endif

#endif /* UTILCHRISTIEMSSCAPI_H */

#endif //defined(CUSTOM_CHRISTIE)


