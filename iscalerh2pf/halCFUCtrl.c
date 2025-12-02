// ===============================================================================
// FILE NAME: halCFUCtrl.c
// DESCRIPTION:
//
//
// Modification History
// --------------------
// 2018/02/26, Larry Create
// --------------------
// ===============================================================================

#include "halCFUCtrlAPI.h"

#include "dvLDDriver.h"
#include "dvFrontEndDriver.h"
#include "dvMotorBoard.h"
#include "dvMCUDriver.h"

#include "utilCLICmdAPI.h"
#include "utilDbgMsg.h"
#include "utilHostAPI.h"


#define EXT_BIN_DATA_SIZE   (1024 + 4)

eRESULT halCFUCtrl_Cmd_GoBootloader_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_GoBootloader_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegRead(eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            #else
                eResult = utilHost_FrontEndGet(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegRead(eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegRead(eCMD_MODULE_IAP, eIAP_MSG_GO_BOOTLOADER, eIAP_MSG_GO_BOOTLOADER_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BL_Ver_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegRead(eIAP_MSG_BL_VER, eIAP_MSG_BL_VER_SZ, pucData);
            #else
                eResult = utilHost_FrontEndGet(eCMD_MODULE_IAP, eIAP_MSG_BL_VER, eIAP_MSG_BL_VER_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegRead(eIAP_MSG_BL_VER, eIAP_MSG_BL_VER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_BL_VER, eIAP_MSG_BL_VER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_BL_VER, eIAP_MSG_BL_VER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegRead(eCMD_MODULE_IAP, eIAP_MSG_BL_VER, eIAP_MSG_BL_VER_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_App_Ver_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_APP_VER, eIAP_MSG_APP_VER_SZ, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_APP_VER, eIAP_MSG_APP_VER_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_APP_VER,  eIAP_MSG_APP_VER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_APP_VER, eIAP_MSG_APP_VER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_APP_VER, eIAP_MSG_APP_VER_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_APP_VER, eIAP_MSG_APP_VER_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_RunApp_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_RUN_APP, eIAP_MSG_RUN_APP_SZ, pucData);
            #else
            eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_RUN_APP, eIAP_MSG_RUN_APP_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_RUN_APP, eIAP_MSG_RUN_APP_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_RUN_APP, eIAP_MSG_RUN_APP_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_RUN_APP, eIAP_MSG_RUN_APP_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_RUN_APP, eIAP_MSG_RUN_APP_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_IAP_EN_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            #else
            eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_IAP_EN_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegRead(eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            #else
                eResult = utilHost_FrontEndGet(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegRead(eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegRead(eCMD_MODULE_IAP, eIAP_MSG_IAP_ENABLE, eIAP_MSG_IAP_ENABLE_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_App_Info_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_APP_INFO, eIAP_MSG_APP_INFO_SZ, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_APP_INFO, eIAP_MSG_APP_INFO_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_APP_INFO, eIAP_MSG_APP_INFO_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_APP_INFO, eIAP_MSG_APP_INFO_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_APP_INFO, eIAP_MSG_APP_INFO_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_APP_INFO, eIAP_MSG_APP_INFO_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_Bin_Address_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_BIN_ADDRESS, eIAP_MSG_BIN_ADDRESS_SZ, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_BIN_ADDRESS, eIAP_MSG_BIN_ADDRESS_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_BIN_ADDRESS, eIAP_MSG_BIN_ADDRESS_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_BIN_ADDRESS, eIAP_MSG_BIN_ADDRESS_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_BIN_ADDRESS, eIAP_MSG_BIN_ADDRESS_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_BIN_ADDRESS, eIAP_MSG_BIN_ADDRESS_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_Bin_Data_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_BIN_DATA, LPCMCU_BIN_PAGE_BYTE_SIZE, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_BIN_DATA, LPCMCU_BIN_PAGE_BYTE_SIZE, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_BIN_DATA, LPCMCU_BIN_PAGE_BYTE_SIZE, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_BIN_DATA, LPCMCU_BIN_PAGE_BYTE_SIZE, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_BIN_DATA, LPCMCU_BIN_PAGE_BYTE_SIZE, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_BIN_DATA, LPCMCU_BIN_PAGE_BYTE_SIZE, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_ProgrammingFinish_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_PROGRAMING_FINISH, 1, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 1, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_PROGRAMING_FINISH, 1, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 1, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 1, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 1, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_ProgrammingFinish_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegRead(eIAP_MSG_PROGRAMING_FINISH, 4, pucData);
            #else
                eResult = utilHost_FrontEndGet(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 4, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegRead(eIAP_MSG_PROGRAMING_FINISH, 4, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 4, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 4, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegRead(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING_FINISH, 4, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BinCheckSum_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegRead(eIAP_MSG_BIN_CHECK_SUM, eIAP_MSG_BIN_CHECK_SUM_SZ, pucData);
            #else
                eResult = utilHost_FrontEndGet(eCMD_MODULE_IAP, eIAP_MSG_BIN_CHECK_SUM, eIAP_MSG_BIN_CHECK_SUM_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegRead(eIAP_MSG_BIN_CHECK_SUM, eIAP_MSG_BIN_CHECK_SUM_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_BIN_CHECK_SUM, eIAP_MSG_BIN_CHECK_SUM_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_BIN_CHECK_SUM, eIAP_MSG_BIN_CHECK_SUM_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegRead(eCMD_MODULE_IAP, eIAP_MSG_BIN_CHECK_SUM, eIAP_MSG_BIN_CHECK_SUM_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BinToFlash_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegWrite(eIAP_MSG_PROGRAMING, 4, pucData);
            #else
                eResult = utilHost_FrontEndSet(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING, 4, pucData);
            #endif
            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegWrite(eIAP_MSG_PROGRAMING, 4, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING, 4, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING, 4, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegWrite(eCMD_MODULE_IAP, eIAP_MSG_PROGRAMING, 4, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_ExtFlashEnable_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8   ucData = 1;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_OPFU_ENABLE, 1, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_ExtFlashEnable_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_OPFU_ENABLE, 1, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_AppCode_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            #ifndef WORD_REG
                eResult = dvFrontEndDriverRegRead(eIAP_MSG_APP_CODE_READBACK, eIAP_MSG_APP_CODE_READBACK_SZ, pucData);
            #else
                eResult = utilHost_FrontEndGet(eCMD_MODULE_IAP, eIAP_MSG_APP_CODE_READBACK, eIAP_MSG_APP_CODE_READBACK_SZ, pucData);
            #endif

            break;

        case eLPCMCU_INDEX_MOTORBD:
            eResult = dvMotorRegRead(eIAP_MSG_APP_CODE_READBACK, eIAP_MSG_APP_CODE_READBACK_SZ, pucData);
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_APP_CODE_READBACK, eIAP_MSG_APP_CODE_READBACK_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            //eResult = dvFrontEndDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_APP_CODE_READBACK, eIAP_MSG_APP_CODE_READBACK_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FMT_2K:
            eResult = dvMotor2RegRead(eCMD_MODULE_IAP, eIAP_MSG_APP_CODE_READBACK, eIAP_MSG_APP_CODE_READBACK_SZ, pucData);
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_OpfuEnable_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8   ucData = 1;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_OPFU_ENABLE, eIAP_MSG_OPFU_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_OpfuEnable_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_OPFU_ENABLE, eIAP_MSG_OPFU_ENABLE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorSeclect_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_SELECT, eIAP_MSG_EXT_FLASH_SECTOR_SELECT_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorSeclect_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_SELECT, eIAP_MSG_EXT_FLASH_SECTOR_SELECT_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorCheckSum_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_CHECKSUM, eIAP_MSG_EXT_FLASH_SECTOR_CHECKSUM_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorCRC_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_CRC, eIAP_MSG_EXT_FLASH_SECTOR_CRC_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorErase_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_ERASE, eIAP_MSG_EXT_FLASH_SECTOR_ERASE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorErase_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_ERASE, eIAP_MSG_EXT_FLASH_SECTOR_ERASE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_SectorSize_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_EXT_FLASH_SECTOR_SIZE, eIAP_MSG_EXT_FLASH_SECTOR_SIZE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BIN_Write_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    //1024 + 4 //1024 is data 4 is address

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_SECTOR_BIN_WRITE, EXT_BIN_DATA_SIZE, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BIN_CheckSum_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_SECTOR_BIN_CHECKSUM, 4, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BIN_CRC_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_SECTOR_BIN_CRC, 2, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BIN_Write2Flash_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_SECTOR_BIN_WRITE2FLASH, eIAP_MSG_SECTOR_BIN_WRITE2FLASH_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BIN_Write2Flash_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    //0 idel, 1 busy

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_SECTOR_BIN_WRITE2FLASH, eIAP_MSG_SECTOR_BIN_WRITE2FLASH_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_BIN_UpgradeStart_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_UPGRADE_START, eIAP_MSG_UPGRADE_START_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_UpgradeStart_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_UPGRADE_START, eIAP_MSG_UPGRADE_START_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_UpgradeState_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_UPGRADE_STATE, eIAP_MSG_UPGRADE_STATE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_UpgradeState_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_UPGRADE_STATE, eIAP_MSG_UPGRADE_STATE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_ExtUpgradeDone_Set(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegWrite(eCMD_MODULE_IAP, eIAP_MSG_EXT_UPGRADE_DONE, eIAP_MSG_EXT_UPGRADE_DONE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

eRESULT halCFUCtrl_Cmd_UpgradePercentage_Get(eLPCMCU_INDEX eChipIndex, UINT8 *pucData)
{
    eRESULT eResult = rcSUCCESS;

    switch(eChipIndex)
    {
        case eLPCMCU_INDEX_FRONT_2K:
            break;

        case eLPCMCU_INDEX_MOTORBD:
            break;

        case eLPCMCU_INDEX_SYSTEM:
            eResult = dvMCUDriverRegRead(eCMD_MODULE_IAP, eIAP_MSG_PERCENTAGE, eIAP_MSG_PERCENTAGE_SZ, pucData);
            break;

        case eLPCMCU_INDEX_FRONT_4K:
            break;

        case eLPCMCU_INDEX_FMT_2K:
            break;

        default:
            eResult = rcERROR;
    }

    return eResult;
}

