/*********************************************************************************
 * FILE NAME: GEC_CLITABLE.H
 * DESCRIPTION:
 *
 *
 * Date: 26 June, 2017
 * Author: John.Chung
*********************************************************************************/
#ifndef GEC_CLITABLE_H
#define GEC_CLITABLE_H

/*
 * Verify CLI name is the same as output CLI name but capital character
 */

static const sGEC_CLI sGEC_CommandLUT[eGEC_COMMAND_INVALID] =
{   //item No.                  output CLI name
    {eGEC_GetModelName,         "GEC_GETMODELNAME"},
    {eGEC_GetFwVersion,         "GEC_GETFWVERSION"},
    {eGEC_GetSerialNumber,      "GEC_GETSERIALNUMBER"},
    {eGEC_GetGecInfo,           "GEC_GETGECINFO"},
    {eGEC_GetGecVersion,        "GEC_GETGECVERSION"},
    {eGEC_GetGecEncryption,     "GEC_GETGECENCRYPTION"},
    {eGEC_GetErrorCode,         "GEC_GETERRORCODE"},
    {eGEC_SetErrorCodeClear,    "GEC_SETERRORCODECLEAR"},
    {eGEC_GetAccCountClear,     "GEC_GETACCCOUNTCLEAR"},
    {eGEC_SetAccCountClear,     "GEC_SETACCCOUNTCLEAR"},
    {eGEC_GetGecTable,          "GEC_GETGECTABLE"},
    {eGEC_GetErrorCount,        "GEC_GETERRORCOUNT"},
    {eGEC_SetErrorCodeMode,     "GEC_SETERRORCODE"},
    {eGEC_SetNVRAM,             "GEC_SETNVRAM"},
    {eGEC_GetNVRAM,             "GEC_GETNVRAM"},
    {eGEC_APCCT,                "APCCT"} //A70LV_John_0077 add CCT function for AP
};
#endif

