#include "Common.h"
#include "CommonDef.h"
#include "utilDbgMsg.h"
#include "utilCounterAPI.h"
#include "utilCommon.h"
#include "utilDataMgrAPI.h"

#include "GEC_EventTable.h"
#include "halBoardCtrlAPI.h"
#include "halLDProc.h"
#include "halFanCtrlAPI.h"
#include "halMCUCtrlAPI.h"
#include "palSysCtrlMgr.h"


const UINT32 m_aulBaudrate[eCM_SERIAL_PORT_BAUD_RATE_NUMBER] =
{
    1200,
    2400,
    4800,
    9600,
    14400,
    19200,
    38400,
    57600,
    115200,
};


static UINT8 m_ucBL_VER = 0xff;
static UINT8 m_ucBH_VER = 0xff;
BOOL bXFPAG_SW_RESET_FLAG = FALSE;


eRESULT palSysCtrlMgr_Baudrate_Set(eCM_SERIAL_PORT_BAUD_RATE_ID eBaudrateIdx)
{
    eRESULT eRet = rcSUCCESS;

    if(eBaudrateIdx >= sizeof(m_aulBaudrate)/sizeof(m_aulBaudrate[0]))
        return rcERROR;

    if(rcSUCCESS != halBoard_Baudrate_Set(m_aulBaudrate[eBaudrateIdx]))
    {
        utilDataMgr_WriteGecLog(UARTError);
        return rcERROR;
    }

    eRet = halMCU_Uart_BaudrateSet(eBaudrateIdx);//(m_aulBaudrate[eBaudrateIdx]); //HICC2_Doulas_0086

    return eRet;
}

eRESULT palSysCtrlMgr_DMD_AirTightSet(UINT8 ucValue) //HICC2_Steven_0031
{
    eRESULT eRet = rcSUCCESS;

    eRet = halMCU_DMD_AirTightSet(ucValue);
    return eRet;
}

eRESULT palSysCtrlMgr_DMD_AirTightGet(UINT8 *ucData) //HICC2_Steven_0032
{
	 eRESULT eResult = rcERROR;

	 eResult = halMCU_DMD_AirTightGet(ucData);
	 return eResult;
}

//unused
void palSysCtrlMgr_XillinxFPGA_Reset(void)
{
    halBoard_XillinxFPGA_Reset();
}


//unused
void palSysCtrlMgr_PowerCheckGet(UINT8 *pcData)
{
    halBoard_PowerCheckGet(pcData);
}


//unused
void palSysCtrlMgr_XillinxFPGA_Manual_Mode_Set(UINT8 ucPanel)
{
	#ifndef XFPGA_NEW_AUTO_MODE_V3_3
	MS_SLEEP(100);
	bXFPAG_SW_RESET_FLAG = TRUE;
    halBoard_XillinxFPGA_Manual_Mode_Set();
    bXFPAG_SW_RESET_FLAG = FALSE;
    #endif
}


//unused
void palSysCtrlMgr_XillinxFPGA_Init_Set(UINT8 ucPanel)
{
    UINT8 IsNewVersion = (UINT8)palSysCtrlMgr_XillinxFPGA_NewVersionGet();

    halBoard_XillinxFPGA_Init_Set(ucPanel, IsNewVersion);
}

eRESULT palSysCtrlMgr_XillinxFPGA_VersionRead(UINT8* ucBLV, UINT8* ucBHV)
{
    UINT8 ucBL_VER = 0xff;
    UINT8 ucBH_VER = 0xff;
    eRESULT eResult = rcERROR;
    eRESULT eResult1 = rcERROR;
    eRESULT eResult2 = rcERROR;

    if((m_ucBL_VER == 0xff) &&
       (m_ucBH_VER == 0xff))
    {
        eResult1 = halBoard_XillinxFPGA_Version_BL_Get(&ucBL_VER);
        eResult2 = halBoard_XillinxFPGA_Version_BH_Get(&ucBH_VER);

        if((eResult1 == rcSUCCESS ) &&(eResult2 == rcSUCCESS))
        {
            if((ucBL_VER != 0xff) && (ucBH_VER != 0xff))
            {
                m_ucBL_VER = ucBL_VER;
                m_ucBH_VER = ucBH_VER;
                *ucBLV = m_ucBL_VER;
                *ucBHV = m_ucBH_VER;
                eResult = rcSUCCESS;
            }
        }
    }
    else
    {
        *ucBLV = m_ucBL_VER;
        *ucBHV = m_ucBH_VER;
        eResult = rcSUCCESS;
    }

    return eResult;
}

BOOL palSysCtrlMgr_XillinxFPGA_NewVersionGet(void)
{
    UINT8 ucBL_VER = 0xff;
    UINT8 ucBH_VER = 0xff;
    eRESULT eResult = rcERROR;
    UINT8 ucVal = 0;

    for(; ucVal<3 ; ucVal++)
    {
        eResult  = halBoard_XillinxFPGA_Version_BL_Get(&ucBL_VER);
        eResult &= halBoard_XillinxFPGA_Version_BH_Get(&ucBH_VER);

        if(eResult == rcSUCCESS)
        {
            if((ucBH_VER == 0) &&(ucBL_VER < 3))
            {
                //LOG_MSG(db_ALWAYS, "palSysCtrlMgr_XillinxFPGA_NewVersionGet OLD1 \r\n");
                return FALSE;
            }
            else
            {
                //LOG_MSG(db_ALWAYS, "palSysCtrlMgr_XillinxFPGA_NewVersionGet NEW \r\n");
                return TRUE;
            }
        }
        eResult = rcERROR;
    }
    //LOG_MSG(db_ALWAYS, "palSysCtrlMgr_XillinxFPGA_NewVersionGet OLD2 \r\n");
    return FALSE;
}

BOOL palSysCtrlMgr_XillinxFPGA_SW_Reset_Flag_Get(void)     //A70LV_Doulas_0262
{
    return bXFPAG_SW_RESET_FLAG;
}

BOOL palSysCtrlMgr_XillinxFPGA_Version_BL_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Version_BL_Get(ucData);
    return eResult;
}

BOOL palSysCtrlMgr_XillinxFPGA_Version_BH_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_Version_BH_Get(ucData);
    return eResult;
}

BOOL palSysCtrlMgr_XillinxFPGA_DRP_Status_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_DRP_Status_Get(ucData);
    return eResult;
}

BOOL palSysCtrlMgr_XillinxFPGA_PhaseUnlock_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_PhaseUnlock_Get(ucData);
    return eResult;
}

BOOL palSysCtrlMgr_XillinxFPGA_LDVS_Cover_Get(UINT8 *ucData)  //A35G2_Simon_0065
{
    eRESULT eResult = rcERROR;
    eResult = halBoard_XillinxFPGA_LDVS_Cover_Get(ucData);
    return eResult;
}

void palSysCtrlMgr_PowerOn(void)
{
    halBoard_PowerOn();
}

void palSysCtrlMgr_PowerKeyReset(void)
{
    halBoard_PowerKeyReset();
}

void palSysCtrlMgr_CameraEnableInit(void)
{
    halBoard_CameraEnableInit();
}

void palSysCtrlMgr_CameraEnableSet(UINT8 ucValue)
{
    halBoard_CameraEnableSet(ucValue);
}

//get Model ID from MCU
eRESULT palSysCtrl_ModelID_Get(UINT8* ModelID)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 Data[8] = {0};
    memset(Data, 0, sizeof(Data));

    #ifdef PLATFORM_A35G2
    {
        eResult = halLDCtrl_Version_Get(Data);

        if(eResult == rcSUCCESS)
        {
            *ModelID = Data[2];
        }
        else
        {
            *ModelID = 0;
        }
    }
    #elif defined(PLATFORM_R70K)
    {
         *ModelID = 0x60; // direct assign the model  // HICC2_Keven_0002
    }
    #else  //A70G2 , A70LK
    {
        eResult = halFanCtrl_Platform_Get(Data);

        if(eResult == rcSUCCESS)
        {
            *ModelID = Data[0];
        }
        else
        {
            *ModelID = 0;
        }
    }
    #endif

    return eResult;
}

