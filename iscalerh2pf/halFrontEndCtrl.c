// ==============================================================================
// FILE NAME: HALFRONTENDCTRL.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 11/27/2017  A70LV_Eric.C_0023
// --------------------
// ==============================================================================

#include "halFrontEndCtrlAPI.h"
#include "utilCLICmdAPI.h"
#include "utilDbgMsg.h"
#include "utilHostAPI.h"
#include "utilOPD_TEST.h"

#if(BIST_ENABLE) //HICC2_Steven_0001
#include "utilBIST.h"

void halFrontEnd_Bist_Cmd_Send(void *pvData)  //H30K_Steven_0001
{
    eRESULT eResult = rcSUCCESS;

    if(utilBIST_Status_Get()!= eBIST_STATUS_IDLE) //write cmd
    {
    	UINT8 pcData[eBIST_MSG_SET_SZ];

    	sBIST_DEV *psDev = (sBIST_DEV*)pvData;
    	sBIST_CMD sSendCmd = utilBIST_CmdInfo_Get();

    	memset(pcData, 0, eBIST_MSG_SET_SZ);

    	eResult = utilHost_FrontEndSet(eCMD_MODULE_BIST, eBIST_MSG_SET_VIA_I2C, eBIST_MSG_SET_SZ, (UINT8*)&sSendCmd);
    }
    else   // get data
    {
    	UINT8 pcData[eBIST_MSG_GET_SZ];

    	UINT8 ucIndex =0, temp =0;

    	//sBIST_DATA sGetData =  (sBIST_DATA*)pvData;
    	sBIST_DATA *psGetData = (sBIST_DATA*)pvData;
    	sBIST_CMD sSendCmd = utilBIST_CmdInfo_Get();
    	memset(pcData, 0, eBIST_MSG_GET_SZ);

    	for(ucIndex =0; ucIndex<=10; ucIndex++)
    	{

    		MS_SLEEP(sSendCmd.lCount * 10);

    		utilHost_FrontEndGet(eCMD_MODULE_BIST, 0, 1, &temp);

    		if(temp == SUB_LAYER_FINISH)
    		{
    			break;
    		}
    	}

    	if(temp != SUB_LAYER_FINISH)
    	{
    		LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() Sub Cmd not finish \n", __FILE__, __LINE__, __func__);
    	}
    	else
    	{
    		eResult = utilHost_FrontEndGet(eCMD_MODULE_BIST, (UINT8)sSendCmd.lBistId, eBIST_MSG_GET_SZ, pcData);
    	}
    	memcpy(psGetData, pcData, sizeof(sBIST_DATA));
    }
}

eRESULT halFrontEndCtrl_BIST_Set(UINT8 ucID_Index, UINT16 ucTestCount, UINT8 ucModule, UINT8 ucLayer) //A70LK_Steven_0007
{
    eRESULT eResult = rcINVALID;
    UINT8 pcData[eBIST_MSG_SET_SZ]; //A70LK_Steven_0007

    pcData[0] = ucID_Index;
    pcData[1] = ucTestCount >> 8;
    pcData[2] = ucTestCount & 0xFF;
    pcData[3] = ucModule;  //A70LK_Steven_0004
    pcData[4] = ucLayer;  //A70LK_Steven_0007

    eResult = utilHost_FrontEndSet(eCMD_MODULE_BIST, eBIST_MSG_SET_VIA_I2C, eBIST_MSG_SET_SZ, pcData);

    return(eResult);
}
eRESULT halFrontEndCtrl_BIST_Get(UINT8 ucID_Index, UINT8* pcData)
{
    eRESULT eResult = rcINVALID;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_BIST, ucID_Index, eBIST_MSG_GET_SZ, pcData);

    return(eResult);
}
eRESULT halFrontEndCtrl_BIST_Status_Set(UINT8 cCmd)//HICC2_Steven_0007
{
    UINT8 pcData = 0;
    eRESULT eResult = utilHost_FrontEndSet(eCMD_MODULE_BIST, cCmd, 1, &pcData);
    return(eResult);
}

#endif //HICC2_Steven_0001

// ==============================================================================
// FUNCTION NAME: halFanCtrl_Liquid_Speed_Get
// DESCRIPTION:
//
//
// Params:
// eLD_ID eDriverID:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0023
// --------------------
// ==============================================================================
#if 1 //HICC2_Doulas_0010 modify//Depends on X35Gen2 design
eRESULT halFrontEndCtrl_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eSYSTEM_MSG_SYSTEM_VERSION, eFE_MSG_VERSION_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_SYSTEM, eSYSTEM_MSG_SYSTEM_VERSION, eFE_MSG_VERSION_SZ, pcData);
    #endif

    return eResult;
}
#endif
// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_HDBaseTVersion_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0031
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_HDBaseTVersion_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_HDBASET_VER, eFE_MSG_HDBASET_VER_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_HDBASET_VER, eFE_MSG_HDBASET_VER_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_videoReady_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0030
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_videoReady_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_READY, eFE_MSG_VIDEO_READY_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_READY, eFE_MSG_VIDEO_READY_SZ, pcData);
    #endif

    #if 0//def H30_DEVELOP_TEMP_DEFINE	//HICC2_Zonic_0002
    *pcData = 1;
	#endif

    return eResult;
}

eRESULT halFrontEndCtrl_BackupInput_videoReady_Get(UINT8* pcData) //G100_Steven_0065
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_BACKUP_INPUT_VIDEO_READY, eFE_MSG_BACKUP_INPUT_VIDEO_READY_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_BACKUP_INPUT_VIDEO_READY, eFE_MSG_BACKUP_INPUT_VIDEO_READY_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_videoFormat_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0030
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_videoFormat_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_FORMAT_MAIN, eFE_MSG_VIDEO_FORMAT_MAIN_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_FORMAT_MAIN, eFE_MSG_VIDEO_FORMAT_MAIN_SZ, pcData);
    #endif

    return eResult;
}


eRESULT halFrontEndCtrl_HDR_EOTF_STREAM_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_HDR_EOTF_STREAM, eFE_MSG_HDR_EOTF_STREAM_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_HDR_EOTF_STREAM, eFE_MSG_HDR_EOTF_STREAM_SZ, pcData);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_HDR_LuminanceInfo_Get(UINT8* pcData)  //H2PF_Simon_0193
{
    eRESULT eResult = rcSUCCESS;

#ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_HDR_LUMINANCE_INFO, eFE_MSG_HDR_LUMINANCE_INFO_SZ, pcData);
#else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_HDR_LUMINANCE_INFO, eFE_MSG_HDR_LUMINANCE_INFO_SZ, pcData);
#endif

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_OPDInfo_Get //G100_Steven_0059
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0030
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_OPDInfo_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_OPD_INFO, eFE_MSG_OPD_INFO_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_OPD_INFO, eFE_MSG_OPD_INFO_SZ, pcData);
    #endif

    return eResult;
}
//G100_Steven_0059
// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_mainInput_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0032
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_MainInput_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_INPUT_MAIN, eFE_MSG_VIDEO_INPUT_MAIN_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_INPUT_MAIN, eFE_MSG_VIDEO_INPUT_MAIN_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_mainInput_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0032
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_MainInput_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VIDEO_INPUT_MAIN, eFE_MSG_VIDEO_INPUT_MAIN_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VIDEO_INPUT_MAIN, eFE_MSG_VIDEO_INPUT_MAIN_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_SubInput_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_SubInput_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_INPUT_SUB, eFE_MSG_VIDEO_INPUT_SUB_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_INPUT_SUB, eFE_MSG_VIDEO_INPUT_SUB_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_SubInput_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_SubInput_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VIDEO_INPUT_SUB, eFE_MSG_VIDEO_INPUT_SUB_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VIDEO_INPUT_SUB, eFE_MSG_VIDEO_INPUT_SUB_SZ, pcData);
    #endif

    return eResult;
}
 //G100_Steven_0016 start
// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_Backup_AutoSW_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0032
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_Backup_AutoSW_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_BACKUP_INPUT_AUTO_SW, eFE_MSG_BACKUP_INPUT_AUTO_SW_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_BACKUP_INPUT_AUTO_SW, eFE_MSG_BACKUP_INPUT_AUTO_SW_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_Backup_First_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0032
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_Backup_First_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_BACKUP_INPUT_FIRST, eFE_MSG_BACKUP_INPUT_FIRST_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_BACKUP_INPUT_FIRST, eFE_MSG_BACKUP_INPUT_FIRST_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_Backup_First_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Eric.C_0032
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_Backup_Second_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_BACKUP_INPUT_SECOND, eFE_MSG_BACKUP_INPUT_SECOND_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_BACKUP_INPUT_SECOND, eFE_MSG_BACKUP_INPUT_SECOND_SZ, pcData);
    #endif

    return eResult;
}
 //G100_Steven_0016 end
//G100_Steven_0028
eRESULT halFrontEndCtrl_Backup_Status_Get(UINT8* pcData) //bit0:Active ; bit1:SWAP
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_BACKUP_INPUT_STATUS, eFE_MSG_BACKUP_INPUT_STATUS_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_BACKUP_INPUT_STATUS, eFE_MSG_BACKUP_INPUT_STATUS_SZ, pcData);
    #endif

    return eResult;
}

//A35G2_CDS_Simon_0041
eRESULT halFrontEndCtrl_BackupInput_ScalerTimingCheck_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_BACKUP_INPUT_VIDEO_SCALER_CHECK, eFE_MSG_BACKUP_INPUT_VIDEO_SCALER_CHECK_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_BACKUP_INPUT_VIDEO_SCALER_CHECK, eFE_MSG_BACKUP_INPUT_VIDEO_SCALER_CHECK_SZ, pcData);
    #endif

    return eResult;
}

//G100_Steven_0021
// ==============================================================================
eRESULT halFrontEndCtrl_HDMI_OUT_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_HDMI_OUT, eFE_MSG_HDMI_OUT_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_HDMI_OUT, eFE_MSG_HDMI_OUT_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_PIP_Enable_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_PIP_Enable_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_PIPPBP_ENABLE, eFE_MSG_PIPPBP_ENABLE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_PIPPBP_ENABLE, eFE_MSG_PIPPBP_ENABLE_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_PIP_Enable_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_PIP_Enable_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucEnable = FALSE;

    if(*pcData) //G100_Owen_0012
    {
        ucEnable = TRUE;
    }

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_PIPPBP_ENABLE, eFE_MSG_PIPPBP_ENABLE_SZ, &ucEnable);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_PIPPBP_ENABLE, eFE_MSG_PIPPBP_ENABLE_SZ, &ucEnable);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_INPUT_SOURCE_DETECT_Get
// DESCRIPTION:
//
//
// Params:
// UINT16* uiData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_INPUT_SOURCE_DETECT_Get(UINT16* uiData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[2];

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_INPUT_SOURCE_DETECT, eFE_MSG_INPUT_SOURCE_DETECT_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT_SOURCE_DETECT, eFE_MSG_INPUT_SOURCE_DETECT_SZ, pcData);
    #endif

    *uiData = pcData[0] + pcData[1] * 256;
    //LOG_MSG(db_ALWAYS, "halFrontEndCtrl_INPUT_SOURCE_DETECT_Get 0x%x\r\n",*uiData);     //A70LV_Doulas_0129

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_H_Total_Get
// DESCRIPTION:
//
//
// Params:
// UINT16* uiData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_H_Total_Get(UINT16* uiData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[2];

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VGA_HTOTAL, eFE_MSG_VGA_HTOTAL_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VGA_HTOTAL, eFE_MSG_VGA_HTOTAL_SZ, pcData);
    #endif

    *uiData = pcData[0] + pcData[1] * 256;

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_H_Total_Set
// DESCRIPTION:
//
//
// Params:
// UINT16* uiData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_H_Total_Set(UINT16* uiData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[2];

    pcData[0] = (UINT8)(*uiData & 0xff);
    pcData[1] = (UINT8)(*uiData >>8);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VGA_HTOTAL, eFE_MSG_VGA_HTOTAL_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VGA_HTOTAL, eFE_MSG_VGA_HTOTAL_SZ, pcData);
    #endif

    return eResult;
}


// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Phase_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Phase_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VGA_PHASE, eFE_MSG_VGA_PHASE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VGA_PHASE, eFE_MSG_VGA_PHASE_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Phase_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0102
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Phase_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VGA_PHASE, eFE_MSG_VGA_PHASE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VGA_PHASE, eFE_MSG_VGA_PHASE_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_videoFormatSub_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0112
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_videoFormatSub_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_FORMAT_SUB, eFE_MSG_VIDEO_FORMAT_SUB_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_FORMAT_SUB, eFE_MSG_VIDEO_FORMAT_SUB_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Sync_Threshold_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0112
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Sync_Threshold_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #if 0 // No VGA for X35Gen2
    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VGA_SYNC_THRESHOLD, eFE_MSG_VGA_SYNC_THRESHOLD_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VGA_SYNC_THRESHOLD, eFE_MSG_VGA_SYNC_THRESHOLD_SZ, pcData);
    #endif
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Sync_Threshold_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0112
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Sync_Threshold_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #if 0 // No VGA for X35Gen2
    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VGA_SYNC_THRESHOLD, eFE_MSG_VGA_SYNC_THRESHOLD_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VGA_SYNC_THRESHOLD, eFE_MSG_VGA_SYNC_THRESHOLD_SZ, pcData);
    #endif
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Pixel_Clock_Set
// DESCRIPTION:
//
//
// Params:
// UINT32 udData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0115
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Pixel_Clock_Set(UINT32 udData)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[4];

    pcData[0] = (UINT8)(udData & 0xFF);
    pcData[1] = (UINT8)((udData>>8) & 0xFF);
    pcData[2] = (UINT8)((udData>>16) & 0xFF);
    pcData[3] = (UINT8)((udData>>24) & 0xFF);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VGA_PIXEL_CLOCK, eFE_MSG_PIXEL_CLOCK_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VGA_PIXEL_CLOCK, eFE_MSG_PIXEL_CLOCK_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Gain_Get
// DESCRIPTION:
//
//
// Params:
// UINT16* uiRed:
// UINT16* uiGreen:
// UINT16* uiBlue:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0124
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Gain_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[6];

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_ADC7604_GAIN, eFE_MSG_ADC7604_GAIN_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_ADC7604_GAIN, eFE_MSG_ADC7604_GAIN_SZ, pcData);
    #endif

    *uiRed   = (UINT16)pcData[0] + ((UINT16)pcData[1]<<8);
    *uiGreen = (UINT16)pcData[2] + ((UINT16)pcData[3]<<8);
    *uiBlue  = (UINT16)pcData[4] + ((UINT16)pcData[5]<<8);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Gain_Set
// DESCRIPTION:
//
//
// Params:
// UINT16 uiRed:
// UINT16 uiGreen:
// UINT16 uiBlue:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0124
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Gain_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[6];

    pcData[0] = (UINT8)(uiRed & 0xFF);
    pcData[1] = (UINT8)((uiRed>>8) & 0xFF);
    pcData[2] = (UINT8)(uiGreen & 0xFF);
    pcData[3] = (UINT8)((uiGreen>>8) & 0xFF);
    pcData[4] = (UINT8)(uiBlue & 0xFF);
    pcData[5] = (UINT8)((uiBlue>>8) & 0xFF);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_ADC7604_GAIN, eFE_MSG_ADC7604_GAIN_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_ADC7604_GAIN, eFE_MSG_ADC7604_GAIN_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Offset_Get
// DESCRIPTION:
//
//
// Params:
// UINT16* uiRed:
// UINT16* uiGreen:
// UINT16* uiBlue:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0124
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Offset_Get(UINT16* uiRed,UINT16* uiGreen,UINT16* uiBlue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[6];

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_ADC7604_OFFSET, eFE_MSG_ADC7604_OFFSET_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_ADC7604_OFFSET, eFE_MSG_ADC7604_OFFSET_SZ, pcData);
    #endif

    *uiRed   = (UINT16)pcData[0] + ((UINT16)pcData[1]<<8);
    *uiGreen = (UINT16)pcData[2] + ((UINT16)pcData[3]<<8);
    *uiBlue  = (UINT16)pcData[4] + ((UINT16)pcData[5]<<8);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_Offset_Set
// DESCRIPTION:
//
//
// Params:
// UINT16 uiRed:
// UINT16 uiGreen:
// UINT16 uiBlue:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0124
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_Offset_Set(UINT16 uiRed,UINT16 uiGreen,UINT16 uiBlue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[6];

    pcData[0] = (UINT8)(uiRed & 0xFF);
    pcData[1] = (UINT8)((uiRed>>8) & 0xFF);
    pcData[2] = (UINT8)(uiGreen & 0xFF);
    pcData[3] = (UINT8)((uiGreen>>8) & 0xFF);
    pcData[4] = (UINT8)(uiBlue & 0xFF);
    pcData[5] = (UINT8)((uiBlue>>8) & 0xFF);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_ADC7604_OFFSET, eFE_MSG_ADC7604_OFFSET_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_ADC7604_OFFSET, eFE_MSG_ADC7604_OFFSET_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_FPGA_Version_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0176
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_FPGA_Version_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_FPGA_VER, eFE_MSG_FPGA_VIDEO_VER_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_FPGA_VER, eFE_MSG_FPGA_VIDEO_VER_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_Main_Timing_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0176
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_Main_Timing_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_TIMING_MAIN, eFE_MSG_VIDEO_TIMING_MAIN_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_TIMING_MAIN, eFE_MSG_VIDEO_TIMING_MAIN_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_Sub_Timing_Get
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0176
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_Sub_Timing_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_VIDEO_TIMING_SUB, eFE_MSG_VIDEO_TIMING_SUB_SZ, pcData);
    #else
    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_VIDEO_TIMING_SUB, eFE_MSG_VIDEO_TIMING_SUB_SZ, pcData);
    #endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_EDID_SN_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData
//
// Returns:
//
//
// Modification History
// --------------------
// G100_John_0003
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_EDID_SN_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EDID_UPGRADE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_UPGRADE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #endif

    return eResult;
}
//G100_John_0003 end

#if 0
// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_SerialNumber_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0200
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_SerialNumber_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_SERIALNUMBER_EDID, eFE_MSG_SERIALNUMBER_EDID_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_SERIALNUMBER_EDID, eFE_MSG_SERIALNUMBER_EDID_SZ, pcData);
    #endif

    return eResult;
}
#endif

// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_VGA_TIMING_Set
// DESCRIPTION:
//
//
// Params:
// UINT16 uiH_Active:
// UINT16 uiV_Active:
// UINT16 uiV_Freq:
//
// Returns:
//
//
// Modification History
// --------------------
// A70LV_Doulas_0200
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_VGA_TIMING_Set(UINT16 uiH_Active,UINT16 uiV_Active,UINT16 uiV_Freq)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 pcData[6];

    pcData[0] = (UINT8)(uiH_Active & 0xFF);
    pcData[1] = (UINT8)((uiH_Active>>8) & 0xFF);
    pcData[2] = (UINT8)(uiV_Active & 0xFF);
    pcData[3] = (UINT8)((uiV_Active>>8) & 0xFF);
    pcData[4] = (UINT8)(uiV_Freq & 0xFF);
    pcData[5] = (UINT8)((uiV_Freq >>8) & 0xFF);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_VGA_TIMING, eFE_MSG_VGA_TIMING_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_VGA_TIMING, eFE_MSG_VGA_TIMING_SZ, pcData);
    #endif

    return eResult;
}

#if 0
// ==============================================================================
// FUNCTION NAME: halFrontEndCtrl_EDID_Upgrade_Set
// DESCRIPTION:
//
//
// Params:
// UINT8* pcData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/07/27, Larry Create
// --------------------
// ==============================================================================
eRESULT halFrontEndCtrl_EDID_Upgrade_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EDID_UPGRADE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_UPGRADE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #endif

    return eResult;
}

//G100_Steven_0005
eRESULT halFrontEndCtrl_EDID_Upgrade_Single_Set(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;


    //LOG_MSG(db_ALWAYS, "EDID %s\r\n",pcData);

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EDID_UPGRADE_SINGLE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_UPGRADE_SINGLE, eFE_MSG_EDID_UPGRADE_SZ, pcData);
    #endif

    return eResult;
}
#endif

eRESULT halFrontEndCtrl_EQMode_HDMI1_Set(UINT8 ucValue)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EQ_MODE_HDMI1, eFE_MSG_EQ_MODE_HDMI1_SZ, &ucValue);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EQ_MODE_HDMI1, eFE_MSG_EQ_MODE_HDMI1_SZ, &ucValue);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_EQMode_HDMI2_Set(UINT8 ucValue)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EQ_MODE_HDMI2, eFE_MSG_EQ_MODE_HDMI2_SZ, &ucValue);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EQ_MODE_HDMI2, eFE_MSG_EQ_MODE_HDMI2_SZ, &ucValue);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_EQMode_DVI_Set(UINT8 ucValue)
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EQ_MODE_DVI, eFE_MSG_EQ_MODE_DVI_SZ, &ucValue);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EQ_MODE_DVI, eFE_MSG_EQ_MODE_DVI_SZ, &ucValue);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_HotPlugEnable_Set(UINT8 ucEnable)		//G100_Doulas_0006
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_HOT_PLUG_ENABLE, eFE_MSG_HOT_PLUG_ENABLE_SZ, &ucEnable);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_HOT_PLUG_ENABLE, eFE_MSG_HOT_PLUG_ENABLE_SZ, &ucEnable);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_ForceHotPlug_Set(UINT8 ucForce)		//G100_Doulas_0007
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_FORCE_HOT_PLUG, eFE_MSG_FORCE_HOT_PLUG_SZ, &ucForce);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_FORCE_HOT_PLUG, eFE_MSG_FORCE_HOT_PLUG_SZ, &ucForce);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_Model_Name_Set(UINT8* pcData) //A35G2_CDS_Coda_0032
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegRead(eFE_MSG_EDID_MODEL_NAME_SET, eFE_MSG_MODEL_NAME_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_MODEL_NAME_SET, eFE_MSG_MODEL_NAME_SZ, pcData);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_OPD_Register_Set(UINT8 reg, UINT8 size, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndSet(eCMD_MODULE_OPD, reg, size, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_OPD_Register_Get(UINT8 reg, UINT8 size, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, reg, size, pcData);

    return eResult;
}


eRESULT halFrontEndCtrl_IST_Set(UINT8 ucCount) //G100_Julie_0001
{
    eRESULT eResult = rcSUCCESS;

    //LOG_MSG(db_HAL_MCU,"(func:%s, line:%d) System i2c[%d]\r\n", __FUNCTION__, __LINE__, ucCount);
	#ifndef WORD_REG
	#else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_OPD, eOPD_MSG_I2C_STRESS_TEST, 1, &ucCount);
	#endif

	return eResult;
}

//G100_Julie_0002, start.
eRESULT halFrontEndCtrl_IST_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_STRESS_TEST, 0, ucCount);

    return eResult;
}

eRESULT halFrontEndCtrl_OPD_State_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_STRESS_STATE, eOPD_MSG_I2C_STRESS_STATE_SZ, ucCount);

    return eResult;
}

eRESULT halFrontEndCtrl_OPD_Work_Get(UINT8 *ucCount)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_STRESS_WORK, eOPD_MSG_I2C_STRESS_WORK_SZ, ucCount);

    return eResult;
}

eRESULT halFrontEndCtrl_OPD_Device_Get(UINT8 ucValue, UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, ucValue, 5, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_Error_Reset(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucCount = 0xFF;

    eResult = utilHost_FrontEndSet(eCMD_MODULE_OPD, eOPD_MSG_EVENT_RESET, eOPD_MSG_EVENT_RESET_SZ, &ucCount);
    return eResult;
}
//G100_Julie_0002, end.

eRESULT halFrontEndCtrl_SPI_GS2961_Set(void) //G100_Julie_0004
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucCount = 0xFF;

    eResult = utilHost_FrontEndSet(eCMD_MODULE_OPD, eOPD_MSG_SPI_STRESS_TEST, eOPD_MSG_SPI_STRESS_TEST_SZ, &ucCount);
    return eResult;
}

eRESULT halFrontEndCtrl_SPI_GS2961_Get(UINT8 *ucCount) //G100_Julie_0004
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_SPI_STRESS_TEST, eOPD_MSG_SPI_STRESS_TEST_SZ, ucCount);

    return eResult;
}

eRESULT halFrontEndCtrl_I2C_Retry_Get(UINT16 uiSize, UINT8* pcData) //G100_Julie_0017
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_RUN_TIME_RETRY, uiSize, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_I2C_Error_Get(UINT16 uiSize,UINT8* pcData) //G100_Julie_0017
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_RUN_TIME_ERROR, uiSize, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_I2C_Total_Get(UINT8* pcData) //G100_Julie_0017
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_OPD, eOPD_MSG_I2C_RUN_TIME_TOTAL, eOPD_MSG_I2C_RUN_TIME_TOTAL_SZ, pcData);

    return eResult;
}


eRESULT halFrontEndCtrl_AutoHDMISwitch_Set(UINT8* pcData) //A35G2_BRC_Simon_0006
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_AUTO_HDMI_SWITCH_SET, eFE_MSG_AUTO_HDMI_SWITCH_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_AUTO_HDMI_SWITCH_SET, eFE_MSG_AUTO_HDMI_SWITCH_SZ, pcData);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_CustomizedEDID_Set(UINT8* pcData) //A35G2_Simon_0091
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_CUSTOMIZE_EDID_SET, eFE_MSG_CUSTOMIZE_EDID_SZ, pcData);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_CUSTOMIZE_EDID_SET, eFE_MSG_CUSTOMIZE_EDID_SZ, pcData);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX0_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT0_AVIINFO_RX_0, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX1_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT0_AVIINFO_RX_1, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX2_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT0_AVIINFO_RX_2, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input0RX3_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT0_AVIINFO_RX_3, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX0_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT1_AVIINFO_RX_0, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX1_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT1_AVIINFO_RX_1, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX2_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT1_AVIINFO_RX_2, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_AVIInfoFrame_Input1RX3_Get(UINT8* pcData)
{
    eRESULT eResult = rcSUCCESS;

    eResult = utilHost_FrontEndGet(eCMD_MODULE_FE, eFE_MSG_INPUT1_AVIINFO_RX_3, eFE_MSG_AVI_INFO_SZ, pcData);

    return eResult;
}

eRESULT halFrontEndCtrl_Model_Switch_Enable_Set(UINT8 ucSwitch) //A65_OPTOMA_Julie_0065 //A35G2_Coda_0063
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_MODEL_SWITCH_ENABLE, eFE_MSG_MODEL_SWITCH_ENABLE_SZ, &ucSwitch);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_MODEL_SWITCH_ENABLE, eFE_MSG_MODEL_SWITCH_ENABLE_SZ, &ucSwitch);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_AudioVol_Set(UINT8 vol)		//R70G2_Sammy_0004
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_AUDIO_VOL_SET, eFE_MSG_CMD_AUDIO_VOL_SZ, &vol);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_AudioMute_Set(UINT8 mute)		//R70G2_Sammy_0004
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_AUDIO_MUTE_SET, eFE_MSG_AUDIO_MUTE_SZ, &mute);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_AudioPlay_Set(UINT8* pcPlay)		//R70G2CDS_Sammy_00010
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_AUDIO_PLAY_SET, eFE_MSG_AUDIO_PLAY_SZ, pcPlay);
    #endif

    return eResult;
}

eRESULT halFrontEndCtrl_Factory_Reset_Set(UINT8 cReset)//HICC2_Julie_0008
{
    eRESULT eResult = rcSUCCESS;

    #ifndef WORD_REG
    eResult = dvFrontEndDriverRegWrite(eFE_MSG_EDID_FACTORY_RESET, eSYSTEM_MSG_FACTORY_RESET_MODE_SZ, &cReset);
    #else
    eResult = utilHost_FrontEndSet(eCMD_MODULE_FE, eFE_MSG_EDID_FACTORY_RESET, eSYSTEM_MSG_FACTORY_RESET_MODE_SZ, &cReset);
    #endif

    return eResult;
}

void halFrontEnd_Bist(void *pvData)  //H30K_Steven_0001
{
	eRESULT lResult = rcSUCCESS;
	UINT8 aucVersion[8] = {0};
	sBIST_DEV *psDev = (sBIST_DEV*)pvData;

	psDev->lValid = 1;

	lResult = halFrontEndCtrl_Version_Get(aucVersion);
    psDev->lCommuError = 0;

    if(lResult == rcSUCCESS)
    {
        psDev->lError      = 0;
        psDev->lData       = I2C_SUCCESS_VALUE;
    }
    else
    {
        psDev->lError      = 1;
        psDev->lData       = I2C_ERROR_VALUE;
    }
}

