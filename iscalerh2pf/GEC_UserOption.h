/*********************************************************************************
 * FILE NAME: GEC_USEROPTION.H
 * DESCRIPTION:
 * To use GEC function, this is the only .h file you need to include
 * Modification History
 *
 *
 * Date: 26 June, 2017
 * Author: John.Chung
*********************************************************************************/

//A70LV_John_0008 start add basic function and flow of GEC
/*
 * File structure:
 * 1. include
 * 2. define
 * 3. enum for function usage
 * 4. GEC functions
 */
#ifndef GEC_USEROPTION_H
#define GEC_USEROPTION_H

#if 0 //TURN ON this define for demonstration
#define GEC_DEMO //GEC_John_Reserved
#endif

#define GECVersion "G01.03"     //Version control
#define GEC_RESETVERSION 10103  //EEPROM Reset Control
#ifdef GEC_DEMO
#define GEC_ModeName    "MCU54113"
#define GEC_FwVersion   "M01.00" //For 54113
#endif

//User Only need to modify this part of include
//User include
#include "utilDbgMsg.h"  //A70LV_John_0003 modify GEC code common define
#include "utilCounterAPI.h"
#include "utilCommon.h"     //A70LV_John_0077 add CCT function for AP
#include "utilStorageCfg.h" //A70LV_John_0077 add CCT function for AP
#include "halFormatter.h"   //A70LV_John_0077 add CCT function for AP
#include "halLDProc.h"      //A70LV_John_0077 add CCT function for AP
#include "palFormatterMgr.h"

#define GEC_printf(...) LOG_MSG(db_ALWAYS, __VA_ARGS__)

typedef enum
{
    eGEC_GetModelName,
    eGEC_GetFwVersion,
    eGEC_GetSerialNumber,
    eGEC_GetGecInfo,
    eGEC_GetGecVersion,
    eGEC_GetGecEncryption,
    eGEC_GetErrorCode,
    eGEC_SetErrorCodeClear,
    eGEC_GetAccCountClear,
    eGEC_SetAccCountClear,
    eGEC_GetGecTable,
    eGEC_GetErrorCount,
    eGEC_SetErrorCodeMode,
    eGEC_SetNVRAM,  //for debug usage
    eGEC_GetNVRAM,  //for debug usage

    eGEC_APCCT,  //A70LV_John_0077 add CCT function for AP

    eGEC_COMMAND_INVALID
}eGEC_COMMAND;

typedef struct
{
    eGEC_COMMAND cCLI_INDEX_Command;

    UINT32 cCLI_INDEX_SubID;
    UINT8 cCLI_INDEX_DataLength;

    UINT32 cCLI_INDEX_DataNumber;
    UINT8 *cCLI_INDEX_Data;
}sCLI_INDEX;

typedef struct
{
    eGEC_COMMAND CommandIndex;
    char *GEC_Command;
}sGEC_CLI;

#define GEC_CLI_MAX_LENGTH      255

void GEC_AccessMode_Set(UINT8 ucvalue);
UINT8 GEC_AccessMode_Get(void);
void GEC_APCCT_Process(UINT32 Data_X, UINT32 Data_Y, UINT32 Data_Z);  //A70LV_John_0077 add CCT function for AP
void GEC_EventHandler(UINT8 *pcInput);
void GEC_APCCT_Process_HPBU_Tester(UINT32 Data_X, UINT32 Data_Y, UINT32 Data_Z); //G50_Alan_0095//R70G2_RIC_AC_0034

#endif //define GEC_UserOpton_h
