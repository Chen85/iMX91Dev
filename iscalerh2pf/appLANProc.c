// ==============================================================================
// FILE NAME: APPLEDPROC.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 27/05/2013, Leohong written
// 02/10/2013 Larry fix  //A70_Larry_0038
// --------------------
// ==============================================================================


#include "appPoll.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
//nclude "utilCounterAPI.h"
#include "appDataMgr.h"
#include "appDataItemAvailableMgr.h"
#include "appSystem.h"
#include "appLANProcAPI.h"
#include "appEnvironment.h"
//#include "appIapProcAPI.h"
//nclude "GEC_UserOption.h"  //A70LV_John_0076 reallocate GEC function place
#include "palGui.h"
#include "palGeoAPI.h"
#include "palFormatterMgr.h"

#include "halFormatter.h"
#include "halMotorCtrlAPI.h"

#include "utilQueueAPI.h"
#include "utilDatabaseAPI.h"

#ifndef SIMULATOR_ISCALER
#include "utilIPCAPI.h"
#endif

#include "cmd_ap.h"
#include "CommonAPI.h"

extern sGUI_CALLBACK GuiCb;


typedef struct
{
    UINT16  uiDataCode;
    UINT16  uiSendAckDoneDelay;
    UINT8   ucExecResult;
}sLAN_AckBuff;	//A35G2_CDS_Coda_0034


typedef struct
{
    UINT8                        ucLAN_Start;
    UINT8                        ucUpgradePercentage; //A70LV_Larry_0304
    UINT16                       uiLAN_TimerCount;

    PsSYSTEM_CONFIGURATION       psSysConfiguration;

    UINT8 ucDataCurrent[DATAMGR_ACCESS_TABLE_ENTRIES];
    UINT8 ucDataBackup[DATAMGR_ACCESS_TABLE_ENTRIES];

    UINT8 ucItemCtrlCurrent[eacINVALID];
    //UINT8 ucItemCtrlBackup[eacINVALID];

    eACK_DONE_SEND_STATE eAckDoneSendState;	//A35G2_CDS_Coda_0034
    sLAN_AckBuff  sSendAckDone;	//A35G2_CDS_Coda_0034
}sLAN_CFG;

static sLAN_CFG m_sLANCfg = {0};

#define MAX_LAN_PKT     2048
#define MAX_LAN_WEB_ACK 32    //A70LV_Larry_0238

//static RINGBUFF_T   m_asSendToLan;
//static UINT16       m_auiSendToLanBuffer[MAX_LAN_PKT];

//static RINGBUFF_T   m_asSendAckDone;
//static UINT16       m_auiSendToAckDoneBuffer[MAX_LAN_WEB_ACK];

static sQUEUE m_sUpdateDatacodeQueue;
static UINT16 m_asUpdateDatacodeBuffer[MAX_LAN_PKT] = {0};

static sQUEUE m_asSendAckDoneQueue;
static sLAN_AckBuff m_sSendToAckDoneBuffer[MAX_LAN_WEB_ACK] = {0};  //G100_Owen_0088//A35G2_CDS_Coda_0034

static UINT16 m_asSendAckDoneDelay;

static UINT8 ucForceCheckUpdateData = FALSE;   //G100_Simon_0035
static UINT8 ucForceCheckUpdateDataSub = FALSE;   //HICC2_Doulas_0113
static BOOL m_bUpdateOSD = FALSE;   //G100_Owen_0076

static UINT8 TelnetReplyString[256] = {'\0'};

BOOL palLANProc_DatacodeUpdate_Check(UINT16 uiDataCode, UINT8 ucInit)
{
    BOOL bSendData = FALSE;

    switch(uiDataCode)
    {
        case edcLAN_APPLY:
        case edcWLAN_APPLY:
        case edcCRESTRON_APPLY:
        case edcPJLINK_APPLY:
        case edcNETWORK_FACTORY_RESET:
        case edcLAN_DHCP:
        case edcLAN_IP_ADDRESS:
        case edcLAN_SUBNET_MASK:
        case edcLAN_DEFAULT_GATEWAY:
        case edcLAN_PRIMARY_DNS:
        case edcLAN_SECOND_DNS:
        case edcWLAN_ENABLE:
        case edcWLAN_START_IP:
        case edcWLAN_END_IP:
        case edcWLAN_SUBNET_MASK:
        case edcWLAN_DEFAULT_GATEWAY:
        //case edcLAN:
        //case edcWLAN:
        case edcWLAN_SSID:
        case edcLAN_IP_ADDRESS1:
        case edcLAN_SUBNET_MASK1:
        case edcLAN_DEFAULT_GATEWAY1:
        case edcLAN_IP_ADDRESS2:
        case edcLAN_SUBNET_MASK2:
        case edcLAN_DEFAULT_GATEWAY2:
        case edcLAN_IP_ADDRESS3:
        case edcLAN_SUBNET_MASK3:
        case edcLAN_DEFAULT_GATEWAY3:
        case edcLAN_IP_ADDRESS4:
        case edcLAN_SUBNET_MASK4:
        case edcLAN_DEFAULT_GATEWAY4:
        case edcWLAN_START_IP1:
        case edcWLAN_END_IP1:
        case edcWLAN_SUBNET_MASK1:
        case edcWLAN_DEFAULT_GATEWAY1:
        case edcWLAN_START_IP2:
        case edcWLAN_END_IP2:
        case edcWLAN_SUBNET_MASK2:
        case edcWLAN_DEFAULT_GATEWAY2:
        case edcWLAN_START_IP3:
        case edcWLAN_END_IP3:
        case edcWLAN_SUBNET_MASK3:
        case edcWLAN_DEFAULT_GATEWAY3:
        case edcWLAN_START_IP4:
        case edcWLAN_END_IP4:
        case edcWLAN_SUBNET_MASK4:
        case edcWLAN_DEFAULT_GATEWAY4:
        case edcLAN_PRIMARY_DNS_1:
        case edcLAN_PRIMARY_DNS_2:
        case edcLAN_PRIMARY_DNS_3:
        case edcLAN_PRIMARY_DNS_4:
        case edcLAN_SECOND_DNS_1:
        case edcLAN_SECOND_DNS_2:
        case edcLAN_SECOND_DNS_3:
        case edcLAN_SECOND_DNS_4:
        case edcCRESTRON_ADDRESS:   //G100_Wilsonj_0059
        case edcCRESTRON_ADDRESS1:  //G100_Wilsonj_0059
        case edcCRESTRON_ADDRESS2:  //G100_Wilsonj_0059
        case edcCRESTRON_ADDRESS3:  //G100_Wilsonj_0059
        case edcCRESTRON_ADDRESS4:  //G100_Wilsonj_0059
        case edcPJLINK_ADDRESS:     //G100_Wilsonj_0059
        case edcPJLINK_ADDRESS1:    //G100_Wilsonj_0059
        case edcPJLINK_ADDRESS2:    //G100_Wilsonj_0059
        case edcPJLINK_ADDRESS3:    //G100_Wilsonj_0059
        case edcPJLINK_ADDRESS4:    //G100_Wilsonj_0059
        case edcLIGHTSENSOR_FULL_BLD:
        case edcLIGHTSENSOR_ECO_BLD:
        case edcLIGHTSENSOR_PWM_FULL_BLD:
        case edcLIGHTSENSOR_PWM_ECO_BLD:
        case edcLIGHTSENSOR_FULL_RLD:
        case edcLIGHTSENSOR_ECO_RLD:
        case edcLIGHTSENSOR_PWM_FULL_RLD:
        case edcLIGHTSENSOR_PWM_ECO_RLD:
        case edcLIGHTSENSOR_FULL_DYNAMIC_RLD:
        case edcLIGHTSENSOR_ECO_DYNAMIC_RLD:
        case edcLIGHTSENSOR_PWM_DYNAMIC_FULL_RLD:
        case edcLIGHTSENSOR_PWM_DYNAMIC_ECO_RLD:
        case edcLIGHTSENSOR_INTENSITY_BLD:
        case edcLIGHTSENSOR_INTENSITY_RLD:
        case edcNETWORK_CONTROL_RESET:  //G100_Owen_0047
        case edcLD_INFORMATION:         //G100_Owen_0048
        case edcSNMP_RESET:             //A35G2_CDS_Larry_0009
        case edcPJLINK_SECRET_ENABLE:	//A35G2_Coda_0049
        case edcPJLINK_SECRET_PASSWORD:
        case edcPROJECTOR_GROUP_ENABLE_DONE:
        case edcPROJECTOR_GROUP_SEARCH_DONE:
        case edcPROJECTOR_GROUP_SEARCH_SAVE_DONE:
        case edcPROJECTOR_GROUP_SELECT_DONE:
        case edcPROJECTOR_GROUP_SELECT_SAVE_DONE:
        case edcPROJECTOR_GROUP_RESET_DONE:
        case edcPROJECTOR_GROUP_IDENTIFY_DONE:
        case edcPROJECTOR_GROUP_ADD_MEMBER_DONE:
        case edcPROJECTOR_GROUP_STATUS:
        case edcPROJECTOR_GROUP_PROJECTOR_NAME:
        case edcPROJECTOR_GROUP_NAME:
        case edcPROJECTOR_GROUP_ID:
        case edcGROUP_EVENT_FREEZE:
        case edcGROUP_EVENT_SHUTTER:
        case edcGROUP_EVENT_DYNAMIC_BLACK:
        case edcGROUP_EVENT_SCHEDULE:
        case edcGROUP_EVENT_AUTO_FOCUS:
        case edcGROUP_EVENT_MUTLI_COLOR_MATCH:
        case edcPROJECTOR_GROUP_SET_LEADER_DONE:
        case edcSTRUCT_EMAIL: //HICC2_Steven_0047
        case edcIPV6_DHCP: //HICC2_AC_0050 // HICC2_Bruce_0020
        case edcIPV6_IP_ADDRESS:
        case edcIPV6_PREFIX_LENGTH:
        case edcIPV6_DEFAULT_GATEWAY:
        case edcIPV6_DNS:
        case edcIPV6_APPLY:
        case edcIPV6_IP_ADDRESS_1:
        case edcIPV6_IP_ADDRESS_2:
        case edcIPV6_IP_ADDRESS_3:
        case edcIPV6_IP_ADDRESS_4:
        case edcIPV6_IP_ADDRESS_5:
        case edcIPV6_IP_ADDRESS_6:
        case edcIPV6_IP_ADDRESS_7:
        case edcIPV6_IP_ADDRESS_8:
        case edcIPV6_DEFAULT_GATEWAY_1:
        case edcIPV6_DEFAULT_GATEWAY_2:
        case edcIPV6_DEFAULT_GATEWAY_3:
        case edcIPV6_DEFAULT_GATEWAY_4:
        case edcIPV6_DEFAULT_GATEWAY_5:
        case edcIPV6_DEFAULT_GATEWAY_6:
        case edcIPV6_DEFAULT_GATEWAY_7:
        case edcIPV6_DEFAULT_GATEWAY_8:
        case edcIPV6_DNS_1:
        case edcIPV6_DNS_2:
        case edcIPV6_DNS_3:
        case edcIPV6_DNS_4:
        case edcIPV6_DNS_5:
        case edcIPV6_DNS_6:
        case edcIPV6_DNS_7:
        case edcIPV6_DNS_8:
#if defined(CUSTOM_OPTOMA)
        case edcNETWORK_RESET_ONLY:	//HICC2_Zonic_0013
#endif
            bSendData = FALSE;
            break; //edcSTRUCT_EMAIL

        case edcSTRUCT_SCHEDULE: //HICC2_Steven_0046
        {
        	if(palSystem_PowerStateGet() == ePOWER_STATE_STANDBY || palSystem_PowerStateGet() == ePOWER_STATE_RESET )
        	{
        		 bSendData = FALSE;
        	}
        	else
        	{
        		 bSendData = TRUE;
        	}
        }
        break;
        case edcMAIN_INPUT: //T100_Simon_0038
        case edcSUB_INPUT:
            LOG_MSG(db_APP_LAN, "UpdateItem to ipc(%d)\r\n", uiDataCode);
            bSendData = TRUE;
            break;

        case edcMAIN_ASPECT_RATIO: //A70LV_Larry_0325
        case edcSUB_ASPECT_RATIO: //A70LV_Larry_0325
            LOG_MSG(db_APP_LAN, "UpdateItem to ipc(%d)\r\n", uiDataCode);
            bSendData = TRUE;
            break;

		case edcART_NET:// HICC2_Bruce_0017
		case edcARTNET_NET:
		case edcARTNET_SUBNET:
		case edcARTNET_UNIVERSE:
		case edcARTNET_START_ADDRESS:
		case edcARTNET_CHANNEL_SETTINGS:
		case edcARTNET_USER1_CHANNEL_1:
		case edcARTNET_USER1_CHANNEL_2:
		case edcARTNET_USER1_CHANNEL_3:
		case edcARTNET_USER1_CHANNEL_4:
		case edcARTNET_USER1_CHANNEL_5:
		case edcARTNET_USER1_CHANNEL_6:
		case edcARTNET_USER1_CHANNEL_7:
		case edcARTNET_USER1_CHANNEL_8:
		case edcARTNET_USER1_CHANNEL_9:
		case edcARTNET_USER1_CHANNEL_10:
		case edcARTNET_USER1_CHANNEL_11:
		case edcARTNET_USER1_CHANNEL_12:
		case edcARTNET_USER1_CHANNEL_13:
		case edcARTNET_USER1_CHANNEL_14:
		case edcARTNET_USER1_CHANNEL_15:
		case edcARTNET_USER1_CHANNEL_16:
		case edcARTNET_USER2_CHANNEL_1:
		case edcARTNET_USER2_CHANNEL_2:
		case edcARTNET_USER2_CHANNEL_3:
		case edcARTNET_USER2_CHANNEL_4:
		case edcARTNET_USER2_CHANNEL_5:
		case edcARTNET_USER2_CHANNEL_6:
		case edcARTNET_USER2_CHANNEL_7:
		case edcARTNET_USER2_CHANNEL_8:
		case edcARTNET_USER2_CHANNEL_9:
		case edcARTNET_USER2_CHANNEL_10:
		case edcARTNET_USER2_CHANNEL_11:
		case edcARTNET_USER2_CHANNEL_12:
		case edcARTNET_USER2_CHANNEL_13:
		case edcARTNET_USER2_CHANNEL_14:
		case edcARTNET_USER2_CHANNEL_15:
		case edcARTNET_USER2_CHANNEL_16:// HICC2_Bruce_0017
		case edcARTNET_USER1_RESET:
		case edcARTNET_USER2_RESET:
		case edcARTNET_RESET:
			if(ucInit)
			{
				bSendData = FALSE;
			}
			else
			{
				bSendData = TRUE;
			}
			break;

        default:
            LOG_MSG(db_APP_LAN, "UpdateItem to ipc(%d)\r\n", uiDataCode);
            bSendData = TRUE;
            break;
    }

    return bSendData;
}

// ==============================================================================
// FUNCTION NAME: palLANProc_Init
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
// 2020/02/29, Larry Create
// --------------------
// ==============================================================================
eEXEC_CODE palLANProc_Init(void)
{
    UINT16  uiDataCode = 0;
    UINT8   ucItemControl = 0;
    UINT16  auiUpdateItem[MAX_LAN_PKT] = {0};
    UINT16  uiUpdateCount = 0;
    UINT16  uiCount = 0;

    m_sLANCfg.ucLAN_Start = 0;
    m_sLANCfg.ucUpgradePercentage = 0; //A70LV_Larry_0304
    m_sLANCfg.eAckDoneSendState = eACK_DONE_SEND_STATE_PENDQ;	//A35G2_CDS_Coda_0034
    ucForceCheckUpdateData = FALSE;    //G100_Simon_0035
    ucForceCheckUpdateDataSub = FALSE;   //HICC2_Doulas_0113
    m_bUpdateOSD = FALSE;           //G100_Owen_0076

    palDataItemAvailable_Init();  //HICC2_Simon_0003

    //for(uiDataCode = 0; uiDataCode < DATAMGR_ACCESS_TABLE_ENTRIES; uiDataCode++) //A70LV_Larry_0289
    //{
    //    m_sLANCfg.ucDataCurrent[uiDataCode] = 0xFF;
    //    m_sLANCfg.ucDataBackup[uiDataCode] = m_sLANCfg.ucDataCurrent[uiDataCode];
    //}

    memset(m_sLANCfg.ucDataCurrent, 0xFF, sizeof(m_sLANCfg.ucDataCurrent)/sizeof(m_sLANCfg.ucDataCurrent[0]));
    memset(m_sLANCfg.ucDataBackup,  0xFF, sizeof(m_sLANCfg.ucDataBackup) /sizeof(m_sLANCfg.ucDataBackup[0]));

    //memset(m_sLANCfg.ucItemCtrlCurrent, 0xFF, sizeof(m_sLANCfg.ucItemCtrlCurrent)/sizeof(m_sLANCfg.ucItemCtrlCurrent[0]));
    //memset(m_sLANCfg.ucItemCtrlBackup,  0xFF, sizeof(m_sLANCfg.ucItemCtrlBackup) /sizeof(m_sLANCfg.ucItemCtrlBackup[0]));

    utilQueueInitial(&m_sUpdateDatacodeQueue, MAX_LAN_PKT,     2, (UINT8*)m_asUpdateDatacodeBuffer);
    utilQueueInitial(&m_asSendAckDoneQueue,   MAX_LAN_WEB_ACK, sizeof(sLAN_AckBuff), (UINT8*)m_sSendToAckDoneBuffer);	//A35G2_CDS_Coda_0034

    for(uiDataCode = 0; uiDataCode < DATAMGR_ACCESS_TABLE_ENTRIES; uiDataCode++)
    {
        //if(palDataMgr_NetworkCommandSkip((eDATA_CODE)uiDataCode) == FALSE &&
        //   palDataMgr_IsUIEventDataCode((eDATA_CODE)uiDataCode) == FALSE)
        if(CommonAPI_DataCode_IsSupportAckdone_Get((eDATA_CODE)uiDataCode) == TRUE)
        {
            if(palDataMgr_Data_Type((eDATA_CODE)uiDataCode) != DATA_TYPE_NA)
            {
                m_sLANCfg.ucDataCurrent[uiDataCode] = palDataMgr_DataCode_Control((eDATA_CODE)uiDataCode);

                if(m_sLANCfg.ucDataCurrent[uiDataCode] != m_sLANCfg.ucDataBackup[uiDataCode])
                {
                    m_sLANCfg.ucDataBackup[uiDataCode] = m_sLANCfg.ucDataCurrent[uiDataCode];
                    palDataMgr_ImportDataToDatabase(uiDataCode);

                    if(palLANProc_DatacodeUpdate_Check(uiDataCode, 1) == TRUE)
                    {
                        if(palDataMgr_Data_Type(uiDataCode) == DATA_TYPE_STRUCT)
                        {
                            utilIpc_NotifyStructureDatatypeDatacode(uiDataCode);
                        }
                        else
                        {
                            auiUpdateItem[uiUpdateCount++] = uiDataCode;
                        }
                    }
                }
            }
        }
    }

    do
    {
        if((uiUpdateCount - uiCount) > 100)
        {
            if(rcSUCCESS != utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &auiUpdateItem[uiCount], eEXEC_CODE_PASS, sizeof(UINT16)*100))
            {
                ASSERT_ALWAYS();
            }
            uiCount += 100;
        }
        else
        {
            if(rcSUCCESS != utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &auiUpdateItem[uiCount], eEXEC_CODE_PASS, sizeof(UINT16)*(uiUpdateCount - uiCount)))
            {
                ASSERT_ALWAYS();
            }

            uiCount = uiUpdateCount;
        }

        MS_SLEEP(50);
        //LOG_MSG(db_ALWAYS, "(%s, %d) UpdateCount = %d\r\n", __FUNCTION__, __LINE__, uiCount);
    }while (uiUpdateCount > uiCount);

    GuiCb = Gui_fpCallbackGet();

    return eEXEC_CODE_PASS ;
}

eEXEC_CODE palLANProc_Init_Reset_All(void) //HICC2_Doulas_0062
{
    UINT16  uiDataCode = 0;
    UINT8   ucItemControl = 0;
    UINT16  auiUpdateItem[MAX_LAN_PKT] = {0};
    UINT16  uiUpdateCount = 0;
    UINT16  uiCount = 0;

    for(uiDataCode = 0; uiDataCode < DATAMGR_ACCESS_TABLE_ENTRIES; uiDataCode++)
    {
        if(CommonAPI_DataCode_IsSupportAckdone_Get((eDATA_CODE)uiDataCode) == TRUE)
        {
            if(palDataMgr_Data_Type((eDATA_CODE)uiDataCode) != DATA_TYPE_NA)
            {
                m_sLANCfg.ucDataCurrent[uiDataCode] = palDataMgr_DataCode_Control((eDATA_CODE)uiDataCode);

                //if(m_sLANCfg.ucDataCurrent[uiDataCode] != m_sLANCfg.ucDataBackup[uiDataCode])
                {
                    m_sLANCfg.ucDataBackup[uiDataCode] = m_sLANCfg.ucDataCurrent[uiDataCode];
                    palDataMgr_ImportDataToDatabase(uiDataCode);

                    //if(palLANProc_DatacodeUpdate_Check(uiDataCode) == TRUE)
                    if(palDataMgr_NetworkCommandSkip(uiDataCode) == FALSE)
                    {
                        if(palDataMgr_Data_Type(uiDataCode) == DATA_TYPE_STRUCT)
                        {
                            utilIpc_NotifyStructureDatatypeDatacode(uiDataCode);
                        }
                        else
                        {
                            auiUpdateItem[uiUpdateCount++] = uiDataCode;
                        }
                        //LOG_MSG(db_ALWAYS, "UpdateItem to ipc(%d)\r\n", uiDataCode);
                    }
                }
            }
        }
    }

	do
	{
		if((uiUpdateCount - uiCount) > 100)
		{
		    if(rcSUCCESS != utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &auiUpdateItem[uiCount], eEXEC_CODE_PASS, sizeof(UINT16)*100))
		    {
		        ASSERT_ALWAYS();
		    }
		    uiCount += 100;
		}
		else
		{
		    if(rcSUCCESS != utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, &auiUpdateItem[uiCount], eEXEC_CODE_PASS, sizeof(UINT16)*(uiUpdateCount - uiCount)))
		    {
		        ASSERT_ALWAYS();
		    }
		    uiCount = uiUpdateCount;
		}

		MS_SLEEP(50);
		//LOG_MSG(db_ALWAYS, "(%s, %d) UpdateCount = %d\r\n", __FUNCTION__, __LINE__, uiCount);
	}while (uiUpdateCount > uiCount);

    //GuiCb = Gui_fpCallbackGet();

    return eEXEC_CODE_PASS ;
}



// ==============================================================================
// FUNCTION NAME: palLANProc_Poll
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
// 2020/02/29, Larry Create
// --------------------
// ==============================================================================
INT16 palLANProc_Poll(UINT16 uiTick)
{
    static UINT8 sucDataPrev = 0;	//A35G2_CDS_Coda_0034
    char    cDataType = 0;
    UINT16  uiDataCode = 0;
    UINT16  auiUpdateItem[MAX_LAN_PKT] = {0};
    UINT16  uiUpdateCount = 0;
    UINT16  uiUpdateStructureTypeCount = 0;

    if(m_sLANCfg.ucLAN_Start)
    {
        UINT16 uiCount = 0;

        if(m_sLANCfg.eAckDoneSendState == eACK_DONE_SEND_STATE_PENDQ)
        {
            if(rcSUCCESS == (eRESULT)utilQueueRead(&m_asSendAckDoneQueue, (UINT8*)&m_sLANCfg.sSendAckDone))
            {
                m_sLANCfg.eAckDoneSendState = eACK_DONE_SEND_STATE_PROCESS;
            }
        }

        if(m_sLANCfg.uiLAN_TimerCount >= 200 ||
           ucForceCheckUpdateData == TRUE ||
           (m_sLANCfg.eAckDoneSendState == eACK_DONE_SEND_STATE_PROCESS && m_sLANCfg.sSendAckDone.uiSendAckDoneDelay == 0) //A70LV_Larry_0257 //G100_Simon_0035
        )
        {
            palLANProcGrayoutConditionCheck();
            m_sLANCfg.uiLAN_TimerCount = 0;

            if(ucForceCheckUpdateData == TRUE)
            {
                ucForceCheckUpdateData = FALSE;
            }
        }

        if(ucForceCheckUpdateDataSub == TRUE) //HICC2_Doulas_0113
        {
            palLANProcGrayoutConditionCheck();
            m_sLANCfg.uiLAN_TimerCount = 0;

            if(ucForceCheckUpdateDataSub == TRUE)
            {
                ucForceCheckUpdateDataSub = FALSE;
            }
        }

        if((m_sLANCfg.uiLAN_TimerCount%100 == 0) &&
            m_bUpdateOSD &&
            palDataMgr_ResetCountGet() == 0
        )
        {
            if( (palGeo_QueueDataCountGet() == 0) &&
                #ifdef SCALER_FPGA_F34
                (palGeo_AdvWarpShowOsdPattern_ParaGet() == PAT_TYPE__OFF) &&
				#endif
                (FALSE == palGeo_ApLinkFlag_Get() || (TRUE == palGeo_ApLinkFlag_Get() && (FALSE == palGeo_IsDraw_OSD(eWDT_BEFORE_WARP))))     //H2PF_Simon_0040
            )
            {
                palDataMgr_UI_EventSend(edcUI_EVENT_UPDATE_OSD_CLI, TRUE, NULL); //GuiCb.fpGui_SendUpdateOSD_CLI_EventCb();
            }

            m_bUpdateOSD = FALSE;
        }
        m_sLANCfg.uiLAN_TimerCount++;

        while(rcSUCCESS == (eRESULT)utilQueueRead(&m_sUpdateDatacodeQueue, (UINT8*)&uiDataCode))
        {
            palDataMgr_ImportDataToDatabase(uiDataCode);

            if(palLANProc_DatacodeUpdate_Check(uiDataCode, 0) == TRUE)
            {
                if(palDataMgr_Data_Type(uiDataCode) == DATA_TYPE_STRUCT)
                {
                    utilIpc_NotifyStructureDatatypeDatacode(uiDataCode);
                }
                else
                {
                    if(palDataMgr_IsUIEventDataCode(uiDataCode) && palDataMgr_ResetCountGet()!=0)
                    {
                        //printf("(%s, %d) uiDataCode = %d\n", __FUNCTION__, __LINE__, uiDataCode);
                    }
                    else if(palDataMgr_GroupFunctionProcessed(uiDataCode))
                    {
                        //printf("(%s, %d) uiDataCode = %d\n", __FUNCTION__, __LINE__, uiDataCode);
                    }
                    else
                    {
                        auiUpdateItem[uiUpdateCount++] = uiDataCode;
                    }
                }
            }

            switch(uiDataCode)
            {
                case edcLAN_APPLY:
                {
                    UINT8  cData[256] = {0};
                    UINT8  ucDHCP = 0;
                    UINT8  ucPath = 0;          //G100_Owen_0026
                    char   cIP[18] = {'\0'};
                    char   cSub[18] = {'\0'};
                    char   cGatway[18] = {'\0'};
                    char   cPrimaryDNS[18] = {'\0'};
                    char   cSecondaryDNS[18] = {'\0'};

                    palDataMgr_Data_Access(edcLAN_DHCP, edaREAD, &ucDHCP);
                    palDataMgr_Data_Access(edcLAN_IP_ADDRESS, edaREAD, cIP);
                    palDataMgr_Data_Access(edcLAN_SUBNET_MASK, edaREAD, cSub);
                    palDataMgr_Data_Access(edcLAN_DEFAULT_GATEWAY, edaREAD, cGatway);
                    palDataMgr_Data_Access(edcLAN_PRIMARY_DNS, edaREAD, cPrimaryDNS);
                    palDataMgr_Data_Access(edcLAN_SECOND_DNS,  edaREAD, cSecondaryDNS);
                    palDataMgr_Data_Access(edcLAN_PATH_SWITCH,  edaREAD, &ucPath);          //G100_Owen_0026

                    sprintf((char *)cData ,"%d %d %s %s %s %s %s", ucPath, ucDHCP, cIP, cSub ,cGatway ,cPrimaryDNS ,cSecondaryDNS);   //G100_Owen_0026
                    LOG_MSG(db_APP_ACK_LAN, "LAN_APPLY str %s\n", cData);

                    utilIpc_SendData(eIPC_SEND_DATA_LAN_APPLY, edcLAN_APPLY, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);  //G100_Simon_0053
                } break;

                case edcWLAN_APPLY:
                {
                    UINT8  cData[256] = {0};
                    UINT8  cWLAN_Enable = 0;
                    char   cStart_IP[18] = {'\0'};
                    char   cEnd_IP[18] = {'\0'};
                    char   cMask[18] = {'\0'};
                    char   cGatway[18] = {'\0'};
                    char   cSSID[32] = {'\0'};

                    palDataMgr_Data_Access(edcWLAN_ENABLE, edaREAD, &cWLAN_Enable);
                    palDataMgr_Data_Access(edcWLAN_START_IP, edaREAD, cStart_IP);
                    palDataMgr_Data_Access(edcWLAN_END_IP, edaREAD, cEnd_IP);
                    palDataMgr_Data_Access(edcWLAN_SUBNET_MASK, edaREAD, cMask);
                    palDataMgr_Data_Access(edcWLAN_DEFAULT_GATEWAY, edaREAD, cGatway);
                    palDataMgr_Data_Access(edcWLAN_SSID, edaREAD, cSSID);

                    sprintf((char *)cData ,"%d %s 12345678 %s %s %s %s",cWLAN_Enable, cSSID, cStart_IP, cEnd_IP, cMask, cGatway);
                    LOG_MSG(db_APP_ACK_LAN, "WLAN_APPLY str %s\n", cData);

                    utilIpc_SendData(eIPC_SEND_DATA_LAN_APPLY, edcWLAN_APPLY, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);  //G100_Simon_0053
                } break;

                //G100_Wilsonj_0055 Start
                case edcCRESTRON_APPLY:
                {
                    UINT8  cData[256] = {0};
                    char   cIP[18] = {'\0'};
                    UINT16 ucPID = 0;
                    UINT16 ucPORT = 0;

                    palDataMgr_Data_Access(edcCRESTRON_ADDRESS, edaREAD, cIP);
                    palDataMgr_Data_Access(edcCRESTRON_PID, edaREAD, &ucPID);
                    palDataMgr_Data_Access(edcCRESTRON_PORT, edaREAD, &ucPORT);

                    sprintf((char *)cData ,"%s %d %d", cIP, ucPID, ucPORT);
                    LOG_MSG(db_APP_ACK_LAN, "CRESTRON_APPLY str %s\n", cData);

                    utilIpc_SendData(eIPC_SEND_DATA_LAN_APPLY, edcCRESTRON_APPLY, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);
                } break;

                case edcPJLINK_APPLY:
                {
                    UINT8  cData[256] = {0};
                    char   cIP[18] = {'\0'};
					UINT8  cSecurityEn = 0;
					char  cPassword[16] = {0};

                    palDataMgr_Data_Access(edcPJLINK_ADDRESS, edaREAD, cIP);
                    palDataMgr_Data_Access(edcPJLINK_SECRET_ENABLE, edaREAD, &cSecurityEn); //A65_OPTOMA_Julie_0034//A35G2_Coda_0049
                    palDataMgr_Data_Access(edcPJLINK_SECRET_PASSWORD, edaREAD, &cPassword); //A65_OPTOMA_Julie_0035

                    sprintf((char *)cData ,"%s %d %s", cIP, cSecurityEn, cPassword);
                    LOG_MSG(db_APP_ACK_LAN, "PJLINK_APPLY str %s\n", cData);

                    utilIpc_SendData(eIPC_SEND_DATA_LAN_APPLY, edcPJLINK_APPLY, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);
                } break;
                //G100_Wilsonj_0059 End

                case edcNETWORK_FACTORY_RESET:
                {
                    UINT8 ucData = 1;
                    utilIpc_SendData(eIPC_SEND_DATA_LAN_RESET, edcNETWORK_FACTORY_RESET, &ucData, eEXEC_CODE_PASS, sizeof(ucData));
                } break;

                case edcNETWORK_RESET_ONLY:
                {
                    UINT8 ucData = 1;
                    utilIpc_SendData(eIPC_SEND_DATA_LAN_RESET, edcNETWORK_RESET_ONLY, &ucData, eEXEC_CODE_PASS, sizeof(ucData));
                } break;

                case edcNETWORK_CONTROL_RESET:  //G100_Owen_0047
                {
                    UINT8 ucData = 1;
                    utilIpc_SendData(eIPC_SEND_DATA_LAN_RESET, edcNETWORK_CONTROL_RESET, &ucData, eEXEC_CODE_PASS, sizeof(ucData));
                } break;

                case edcSNMP_RESET: //A35G2_CDS_Larry_0009
                {
                    UINT8 ucData = 1;
                    utilIpc_SendData(eIPC_SEND_DATA_LAN_RESET, edcSNMP_RESET, &ucData, eEXEC_CODE_PASS, sizeof(ucData));
                } break;

                case edcCONFIG_BACKUP_RESTART_WEB: //A65_OPTOMA_Julie_0123 //A35G2_Coda_0123
                {
                    UINT8 ucData = 0;
                    UINT8 cData[256] = {0};

                    palDataMgr_Data_Access(edcCONFIG_BACKUP_RESTART_WEB, edaREAD, &ucData);
                    sprintf((char *)cData ,"%d", ucData);
                    //LOG_MSG(db_ALWAYS, "CONFIG_BACKUP_RESTART_WEB str (%d)(%s)\n", ucData, cData);

                    utilIpc_SendData(eIPC_SEND_DATA_LAN_APPLY, edcCONFIG_BACKUP_RESTART_WEB, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);
                } break;

                case edcACTION_TELNET_CLI:
                {
                    UINT8 cData[256] = {0};
                    palDataMgr_Data_Access(edcACTION_TELNET_CLI, edaREAD, cData);

                    utilIpc_SendData(eIPC_SEND_DATA_TELNET_REPLY, edcACTION_TELNET_CLI, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);
                } break;

				case edcIPV6_APPLY: //HICC2_AC_0050 // HICC2_Bruce_0020
				{
					UINT8 cData[256] = {0};
					UINT8 ucDHCP = 0;
					char cIP[64] = {'\0'};
					UINT8 ucPrefix_Length = 0;
					char cGatway[64] = {'\0'};
					char cDNS[64] = {'\0'};

					palDataMgr_Data_Access(edcIPV6_DHCP, edaREAD, &ucDHCP);
					palDataMgr_Data_Access(edcIPV6_IP_ADDRESS, edaREAD, cIP);
					palDataMgr_Data_Access(edcIPV6_PREFIX_LENGTH, edaREAD, &ucPrefix_Length);
					palDataMgr_Data_Access(edcIPV6_DEFAULT_GATEWAY, edaREAD, cGatway);
					palDataMgr_Data_Access(edcIPV6_DNS, edaREAD, cDNS);

					sprintf((char *)cData ,"%d %s %d %s %s", ucDHCP, cIP ,ucPrefix_Length ,cGatway ,cDNS);
					LOG_MSG(db_APP_ACK_LAN, "edcIPV6_APPLY str %s\n", cData);
					utilIpc_SendData(eIPC_SEND_DATA_LAN_APPLY, edcIPV6_APPLY, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);
				} break;
            }


            //For update datacode IPC command, use 2 bytes to record one datacode.
            //And for each IPC data length is 2048(MAX_SIZE_DATABASE_DATA) bytes.
            //Therefore, the update datacode IPC command notify up to 2048/2 datacode.
            if(uiUpdateCount >= MAX_SIZE_DATABASE_DATA/sizeof(UINT16))
            {
                break;
            }
        }

        if(uiUpdateCount)
        {
            //for(int i=0 ; i<uiUpdateCount ; i++)
            //    LOG_MSG(db_ALWAYS, "> send Update DC %d\n", auiUpdateItem[i]);


            palDataItemAvailable_Poll();

			if(rcSUCCESS != utilIpc_SendData(eIPC_SEND_DATA_UPDATE_DATACODE_ITEM, 0, auiUpdateItem, eEXEC_CODE_PASS, sizeof(UINT16)*uiUpdateCount))
			{
				ASSERT_ALWAYS();
			}
        }

        if(m_sLANCfg.eAckDoneSendState == eACK_DONE_SEND_STATE_PROCESS)
        {
            if(m_sLANCfg.sSendAckDone.uiSendAckDoneDelay == 0)
            {
                //if(palDataMgr_NetworkCommandSkip((eDATA_CODE)m_sLANCfg.sSendAckDone.uiDataCode) == FALSE &&
                //   palDataMgr_IsUIEventDataCode((eDATA_CODE)m_sLANCfg.sSendAckDone.uiDataCode) == FALSE)
                if(CommonAPI_DataCode_IsSupportAckdone_Get((eDATA_CODE)m_sLANCfg.sSendAckDone.uiDataCode) == TRUE)
                {
                    UINT8 cData[256] = {0};
                    LOG_MSG(db_APP_CLI_LAN, "ACK_LAN Done %d %d\n", m_sLANCfg.sSendAckDone.uiDataCode, m_sLANCfg.sSendAckDone.ucExecResult);

                    #if 0
                    //AacDone前置處理，例如schedule OSD更新，要先更新Grayout Condition
                    switch(m_sLANCfg.sSendAckDone.uiDataCode)
                    {
                        case edcSCHEDULE_VIEW_WEEKDAY:
                        case edcWARP_TOGGLE:
                        case edcLOCK_ALL_LENS_MOTORS:
                        case edcLENS_CALIBRATION:
                        case edcAUTO_WARP_FILTER:
                        case edcWARP_RESET: //HICC2_Jacky_0007
                        case edcADV_WARP_CONTROL:
                            palLANProcGrayoutConditionCheck();
                            break;
                    }
                    #endif

                    palDataMgr_ImportDataToDatabase(m_sLANCfg.sSendAckDone.uiDataCode); //G100_Larry_0030

                    switch(m_sLANCfg.sSendAckDone.uiDataCode)
                    {
                        case edcMAIN_INPUT: //T100_Simon_0038
                        case edcSUB_INPUT:
                        {
                            UINT8 ucSourceID = 0;
                            palDataMgr_Data_Access((eDATA_CODE)m_sLANCfg.sSendAckDone.uiDataCode, edaREAD, &ucSourceID);
                            utilIpc_SendData(eIPC_SEND_DATA_ACK_DONE, m_sLANCfg.sSendAckDone.uiDataCode, &ucSourceID, m_sLANCfg.sSendAckDone.ucExecResult, sizeof(ucSourceID));

                            //LOG_MSG(db_APP_ACK_LAN, "Dec %d\n", ucSourceID);
                        }
                        break;

                        //case edcBACKUP_RESTORE_SAVE: //HICC2_Doulas_0039 remove//A70LV_Larry_0382
                        //    break;

                        default:
                        {
                            cDataType = palDataMgr_Data_Type((eDATA_CODE)m_sLANCfg.sSendAckDone.uiDataCode);
                            if(DATA_TYPE_NA != cDataType)
                            {
                                palDataMgr_Data_Access((eDATA_CODE)m_sLANCfg.sSendAckDone.uiDataCode, edaREAD, cData);

                                if(DATA_TYPE_STRING == cDataType)
                                {
                                    //LOG_MSG(db_APP_ACK_LAN, "str %s\n", cData);
                                    utilIpc_SendData(eIPC_SEND_DATA_ACK_DONE, m_sLANCfg.sSendAckDone.uiDataCode, cData, m_sLANCfg.sSendAckDone.ucExecResult, strlen((char*)cData));
                                }
                                else
                                {
                                    //LOG_MSG(db_APP_ACK_LAN, "dec %d\n", cData);
                                    utilIpc_SendData(eIPC_SEND_DATA_ACK_DONE, m_sLANCfg.sSendAckDone.uiDataCode, cData, m_sLANCfg.sSendAckDone.ucExecResult, 1);
                                }
                            }
                        }
                        break;
                    }
                }
                m_sLANCfg.eAckDoneSendState = eACK_DONE_SEND_STATE_PENDQ;
            }
            else
            {
                m_sLANCfg.sSendAckDone.uiSendAckDoneDelay--;
                //G100_Owen_0089
                if(m_sLANCfg.sSendAckDone.uiSendAckDoneDelay%10 == 0)
                {
                    UINT8 ucData[256] = {0};
                    switch(m_sLANCfg.sSendAckDone.uiDataCode)
                    {
                        case edcPICTURE_MUTE:
                        {
                            ucData[0] = 1;
                            palFormatterMgr_Smooth_Enable_Get(ucData);
                            LOG_MSG(db_APP_ACK_LAN, "Smooth En = %d\n", ucData[0]);
                            if(ucData[0] == 0)
                            {
                                LOG_MSG(db_APP_ACK_LAN, "uiSendAckDoneDelay = %d\n", m_sLANCfg.sSendAckDone.uiSendAckDoneDelay);
                                m_sLANCfg.sSendAckDone.uiSendAckDoneDelay = 0;
                            }
                        }
                        break;

                        case edcFOCUS_MOVE:  //A65_OPTOMA_CL_0008
                        {
                            ucData[0] = 1;
                            ucData[0] = halMotorFocusGet();
                            LOG_MSG(db_APP_ACK_LAN, "Focus Motor Status = %d\n", ucData[0]);
                            if(ucData[0] == 0)
                            {
                                LOG_MSG(db_APP_ACK_LAN, "Focus Motor uiSendAckDoneDelay = %d\n", m_sLANCfg.sSendAckDone.uiSendAckDoneDelay);
                                m_sLANCfg.sSendAckDone.uiSendAckDoneDelay = 0;
                            }
                        }
                            break;

                        case edcLENS_TOP_BOTTOM_CENTER: //A35G2_Wesley_0163 //A70Gen2_Julie_0109//HICC2_Julie_0015
                        {
                            if(palEnvironment_LensSpecialFlag_Get() == FALSE)
                            {
                                m_sLANCfg.sSendAckDone.uiSendAckDoneDelay = 0;
                            }
                        }
                            break;

                        default:
                            break;
                    }
                }

                #if 0
                if(m_sLANCfg.sSendAckDone.uiSendAckDoneDelay == 0)
                {
                    palLANProcGrayoutConditionCheck();
                }
                #endif
            }
        }

        if(palSystem_PowerStateGet() == ePOWER_STATE_UPGRADE) //A70LV_Larry_0304
        {
            if(m_sLANCfg.ucUpgradePercentage >= 100)
            {
                m_asSendAckDoneDelay++;

                if(m_asSendAckDoneDelay > 100)
                {
                    palLANProc_UpgradePercentage(m_sLANCfg.ucUpgradePercentage);
                    m_asSendAckDoneDelay = 0;
                }
            }
        }
    }

    return LAN_POLLING_PERIOD / POLL_PERIOD;
}

// ==============================================================================
// FUNCTION NAME: palLANProcSendToLAN
// DESCRIPTION:
//
//
// Params:
// UINT16 uiDataCode:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/15, Larry Create
// --------------------
// ==============================================================================
void palLANProcSendToLAN(UINT16 uiDataCode)
{
	//reduce IPC command (don`t send if data is the same)
	#if 0
    if(uiDataCode != edcACTION_KEY_CODE)
    {
        if(palDataMgr_Data_Type(uiDataCode) >= DATA_TYPE_UI_DIGIT_8 &&
           palDataMgr_Data_Type(uiDataCode) <= DATA_TYPE_I_DIGIT_32 )
        {
            sDATABASE_ITEM_DATA_FORMAT sDatabaseData;
            utilDatabase_ReadData(eSB_DATABASE_ITEM, &sDatabaseData, uiDataCode);

            UINT32 Value;
            palDataMgr_Data_Access(uiDataCode, edaREAD, &Value);

            UINT32 Control;
            Control = palDataMgr_DataCode_Control(uiDataCode);

            if(sDatabaseData.uValue.lValue   == Value &&
               sDatabaseData.ucGrayoutStatus == Control )
            {
                return ;
            }
        }
    }
	#endif

    if(uiDataCode < DATAMGR_ACCESS_TABLE_ENTRIES && palDataMgr_Data_Type((eDATA_CODE)uiDataCode) != DATA_TYPE_NA)
    {
        m_sLANCfg.ucDataCurrent[uiDataCode] = palDataMgr_DataCode_Control((eDATA_CODE)uiDataCode);
        m_sLANCfg.ucDataBackup[uiDataCode] = m_sLANCfg.ucDataCurrent[uiDataCode];
    }
    if(palSystem_PowerStateGet() != ePOWER_STATE_UPGRADE && m_sLANCfg.ucLAN_Start)
    {
        if(rcSUCCESS != utilQueueWrite(&m_sUpdateDatacodeQueue, (UINT8 *)&uiDataCode))
        {
            ASSERT_ALWAYS();
        }
    }
}

// ==============================================================================
// FUNCTION NAME: palLANProcSendAckDone
// DESCRIPTION:
//
//
// Params:
// UINT16 uiDataCode:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/25, Larry Create
// --------------------
// ==============================================================================
void palLANProcSendAckDone(UINT16 uiDataCode, _eInterface eSendFrom ,eEXEC_CODE ExecResult)  //H2PF_Simon_0067
{
    sLAN_AckBuff sSendAckBuff = {0};	//A35G2_CDS_Coda_0034
    UINT16 uiSendDelay = 0;	//A35G2_CDS_Coda_0034

    if(palSystem_PowerStateGet() != ePOWER_STATE_UPGRADE && m_sLANCfg.ucLAN_Start)
    {
        if(eSendFrom == CMD_INTERFACE_UI_PROCESS)  //from iScalerH2CT
        {
            switch(uiDataCode)
            {
                //因設定 edcCOLOR_SPACE_RGB 或 edcCOLOR_SPACE_YUV 時
                //會同時設定 edcCOLOR_SPACE
                //而 edcCOLOR_SPACE 又會同時設定 edcCOLOR_SPACE_RGB , edcCOLOR_SPACE_YUV
                //為了不考慮順序問題，等待 100ms 讓相關 color space 的 datacode 都寫入 shared memory 後
                //再回 Ackdone , 以確保此時 shared memory 的資料都已更新
                case edcCOLOR_SPACE_AUTO:
                case edcCOLOR_SPACE_RGB:
                case edcCOLOR_SPACE_YUV:
                    uiSendDelay = 100 / LAN_POLLING_PERIOD; // 100ms
                    break;

                //此 datacode 因傳送 CT 再回傳給 PF 執行 queue 裡全部內容
                //測試後，先暫時以 300ms 作為預估時間
                case edcGEO_CLEAR_WARP_OSD:
                case edcGEO_OSD_ON:
                    uiSendDelay = 300 / LAN_POLLING_PERIOD;
                    break;

                //edcGEO_FREEZE_IMAGE 因傳送 CT 再回傳給 PF 執行 queue 裡全部內容
                //測試後，先暫時以 500ms 作為預估時間
                case edcGEO_FREEZE_IMAGE:
                    uiSendDelay = 500 / LAN_POLLING_PERIOD;
                    break;

                //為了操作 OSD 時不卡
                //預設為 datacode function 執行完，就回 Ackdone
                default:
                    uiSendDelay = 0 / LAN_POLLING_PERIOD;
                    break;
            }
        }
        else  //from webpage or other...
        {
            switch(uiDataCode)
    		{
    		    case edcCOLOR_SPACE_AUTO:
                case edcCOLOR_SPACE_RGB:
                case edcCOLOR_SPACE_YUV:
                    uiSendDelay = 100 / LAN_POLLING_PERIOD; // 100ms
                    break;

                case edcGEO_CLEAR_WARP_OSD:
                case edcGEO_OSD_ON:
                    uiSendDelay = 300 / LAN_POLLING_PERIOD;
                    break;

                case edcGEO_FREEZE_IMAGE:
                    uiSendDelay = 500 / LAN_POLLING_PERIOD;
                    break;

    			case edcPICTURE_SETTINGS:
                case edcFORCE_PICTURE_SETTINGS:
    			case edcIMAGE_FREEZE:
                case edcSIZE_PRESETS:
                case edcHDR_AUTOENABLE: //A35G2_Coda_0131
                case edcFOCUS_MOVE:  //A65_OPTOMA_CL_0008
    				uiSendDelay = 2000/LAN_POLLING_PERIOD; // 2s
    				break;

                case edcLOW_LATENCY_MODE:
    				uiSendDelay = 3000/LAN_POLLING_PERIOD;
                    break;

                case edcCUSTOMIZE_EDID_HDMI1:
                case edcCUSTOMIZE_EDID_HDMI2:
    				uiSendDelay = 5000/LAN_POLLING_PERIOD; //5s
    				break;

    			case edcPIP_PBP_ENABLE:
    			case edcSWAP:
    			case edcSIZE:
    			case edcMAIN_LAYOUT:
                case edcHSG_RESET_DEFAULT:
    			case edcBACKUP_RESTORE_WEB_SAVE:	//G100_Doulas_0030
    			case edcBACKUP_RESTORE_SAVE:		//G100_Doulas_0030
    			case edcBACKUPINPUT_AUTOSWITCH:       //A35G2_CDS_Simon_0025
    			case edcBACKUPINPUT_PRIMARY_INPUT:    //A35G2_CDS_Simon_0025
    			case edcBACKUPINPUT_SECONDARY_INPUT:  //A35G2_CDS_Simon_0025
    			case edcPICTURE_MUTE:               //G100_Owen_0089  //A35G2_CDS_Coda_0027
    				uiSendDelay = 7000/LAN_POLLING_PERIOD; //7s
    				break;

    			case edcFACTORY_RESET:
    			case edcBACKUP_RESTORE_WEB_RESTORE:		//G100_Doulas_0030
    			case edcBACKUP_RESTORE_RESTORE:			//G100_Doulas_0030
    				uiSendDelay = 8000/LAN_POLLING_PERIOD;
    				break;

                case edc3D_ENABLE:
                case edc3D_MODE:            //G100_Simon_0024
                case edc3D_Reset:           //G100_Simon_0034
                case edcINSTALLATION_RESET: //G100_Simon_0034
                case edcImageReset:         //G100_Simon_0034
                case edcSYSTEM_RESET:       //G100_Simon_0034
    			    uiSendDelay = 12000/LAN_POLLING_PERIOD;
    			    break;

    			//case edcLENS_TOP_BOTTOM_CENTER: //HICC2_Doulas_0138//A35G2_Wesley_0163 //A70Gen2_Julie_0109//HICC2_Julie_0015
    			//    uiSendDelay = 60000/LAN_POLLING_PERIOD;// 60s
    			//	break;

    			case edcADV_WARP_GRID_POINTS:
    			case edcADV_WARP_INNER:
    			case edcADV_WARP_GRID_COLOR:
    			case edcADV_WARP_GRID_BACKGROUND:
    			case edcAW_WARP_POINT_ALL_POSITION:
    			    {
    			        UINT8 GridPoints = 0;
    			        palDataMgr_Data_Access(edcADV_WARP_GRID_POINTS, edaREAD, &GridPoints);
    			        if(GridPoints == WARP_POINT__17x17)
    			        {
    			            uiSendDelay = 1000/LAN_POLLING_PERIOD;
    			        }
    			        else if(GridPoints == WARP_POINT__33x33)
    			        {
    			            uiSendDelay = 2000/LAN_POLLING_PERIOD;
    			        }
    			    }
    			    break;

    			default:
    				uiSendDelay = 0/LAN_POLLING_PERIOD;
    				break;
    		}
        }


        sSendAckBuff.uiDataCode = uiDataCode;			//A35G2_CDS_Coda_0034
        sSendAckBuff.uiSendAckDoneDelay = uiSendDelay;
        sSendAckBuff.ucExecResult = (UINT8)ExecResult;

        if(rcSUCCESS != utilQueueWrite(&m_asSendAckDoneQueue, (UINT8 *)&sSendAckBuff))
        {
            ASSERT_ALWAYS();
        }

        LOG_MSG(db_APP_ACK_LAN, "ACK_LAN %d (%dms,%d)\n", uiDataCode, (uiSendDelay*10), sSendAckBuff.ucExecResult);
    }
}

// ==============================================================================
// FUNCTION NAME: palLANProcPowerStateSend
// DESCRIPTION:
//
//
// Params:
// UINT16 uiData:
// UINT16 uiSize:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/15, Larry Create
// --------------------
// ==============================================================================
void palLANProcPowerStateSend(UINT16 uiData, UINT16 uiSize, UINT8 *pucData)
{
    sDST_POWER_STATE sPowerState = {
                                    uiData,
                                    (UINT32)*pucData
                                   };


    palDataMgr_Data_Access(edcPOWER_STATUS, edaWRITE_THROUGH_WITH_ACTION, &sPowerState);
    //utilIpc_SendData(eIPC_SEND_DATA_POWER_STATE, uiData, pucData, uiSize);
}

// ==============================================================================
// FUNCTION NAME: palLANProcInfoItemSend
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
// 2020/07/01, Larry Create
// --------------------
// ==============================================================================
void palLANProcInfoItemSend(UINT16 uiIndex, UINT16 uiSize, UINT8 *pucData)
{
    utilIpc_SendData(eIPC_SEND_DATA_UPDATE_INFO_ITEM, uiIndex, pucData, eEXEC_CODE_PASS ,uiSize);
}

// ==============================================================================
// FUNCTION NAME: palLANProcTelnetReply
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
// 2020/02/29, Larry Create
// --------------------
// ==============================================================================
void palLANProcTelnetReply(UINT8 *aucString, UINT16 uiStringLength)        //T100_Simon_0020
{
    if(m_sLANCfg.ucLAN_Start) //A70LV_Larry_0408
    {
        UINT8 cData[1024] = {0};
        UINT32 eDataCode = edcACTION_TELNET_CLI;

        snprintf(TelnetReplyString, sizeof(TelnetReplyString), "%s", aucString);

        palDataMgr_Data_Access(eDataCode, edaREAD, cData);
        utilIpc_SendData(eIPC_SEND_DATA_TELNET_REPLY, eDataCode, cData, eEXEC_CODE_PASS, strlen((char*)cData)+1);

        //UINT16 uiCmdID = 0xFFFF ; //Don`t care
        //utilIpc_SendData(eIPC_SEND_DATA_TELNET_REPLY, uiCmdID, aucString, eEXEC_CODE_PASS, uiStringLength);
    }
}

// ==============================================================================
// FUNCTION NAME: palLANProcTelnetReplyStringGet
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
// 2024/07/24, Larry Create
// --------------------
// ==============================================================================
void palLANProcTelnetReplyStringGet(UINT8 *String)
{
    snprintf(String, sizeof(TelnetReplyString), "%s", TelnetReplyString);
}
// ==============================================================================
// FUNCTION NAME: palLANProcSendLog
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
// 2020/02/29, Larry Create
// --------------------
// ==============================================================================
void palLANProcSendLog(UINT32 ulEvent, UINT32 ulTime) //A70LV_Larry_0352
{
    sDST_ACTION_LOG sLog = {0};

    sLog.ulEvent = ulEvent;
    sLog.ulTime = ulTime;

    utilIpc_SendData(eIPC_SEND_DATA_SNMP, 0, &sLog, eEXEC_CODE_PASS, sizeof(sDST_ACTION_LOG));
}

// ==============================================================================
// FUNCTION NAME: palLANProcStart
// DESCRIPTION:
//
//
// Params:
// UINT8 ucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/16, Larry Create
// --------------------
// ==============================================================================
void palLANProcStartSet(UINT8 ucData)
{
    m_sLANCfg.ucLAN_Start = ucData;
}

// ==============================================================================
// FUNCTION NAME: palLANProcStartGet
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
// 2019/01/30, Larry Create
// --------------------
// ==============================================================================
UINT8 palLANProcStartGet(void)
{
    return m_sLANCfg.ucLAN_Start;
}

// ==============================================================================
// FUNCTION NAME: palLANProc_Reset
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
// 2020/03/18, Larry Create
// --------------------
// ==============================================================================
eEXEC_CODE palLANProc_Reset(void)
{
	m_sLANCfg.ucLAN_Start = 1;

    return eEXEC_CODE_PASS ;
}

// ==============================================================================
// FUNCTION NAME: palLANProc_UpgradePercentage
// DESCRIPTION:
//
//
// Params:
// UINT8 ucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/05/31, Larry Create
// --------------------
// ==============================================================================
void palLANProc_UpgradePercentage(UINT8 ucData)
{
    m_sLANCfg.ucUpgradePercentage = ucData; //A70LV_Larry_0304

    //utilCLICmd_EventWriteToLAN(eCMD_MODULE_IAP, eIAP_CMD_PERCENTAGE, 1, (UINT8 *)&ucData);
}


static void palLANProcGrayoutConditionCheck(void)
{
    UINT16 uiCount = 0;

    //datacode gray out condition check
    for(uiCount = 0; uiCount < DATAMGR_ACCESS_TABLE_ENTRIES; uiCount++)
    {
        //if(palDataMgr_NetworkCommandSkip((eDATA_CODE)uiCount) == FALSE &&
        //   palDataMgr_IsUIEventDataCode((eDATA_CODE)uiCount) == FALSE)
        if(CommonAPI_DataCode_IsSupportAckdone_Get((eDATA_CODE)uiCount) == TRUE)
        {
            if(palDataMgr_Data_Type((eDATA_CODE)uiCount) != DATA_TYPE_NA)
            {
                m_sLANCfg.ucDataCurrent[uiCount] = palDataMgr_DataCode_Control((eDATA_CODE)uiCount);

                if(m_sLANCfg.ucDataCurrent[uiCount] != m_sLANCfg.ucDataBackup[uiCount])
                {
                    //if((uiCount == edcBRIGHTNESS) ||
                    //    (uiCount == edcAUTO_IMAGE) ||
                    //    (uiCount == edcAUTO_SOURCE_RESYNC) )//HICC2_Doulas_0113
                    //{
                    //    LOG_MSG(db_APP_DATAPATH, "@@@%d (%d,%d)\r\n",uiCount,m_sLANCfg.ucDataCurrent[uiCount],m_sLANCfg.ucDataBackup[uiCount]);
                    //}
                    //m_sLANCfg.ucDataBackup[uiCount] = m_sLANCfg.ucDataCurrent[uiCount];
                    palLANProcSendToLAN(uiCount);
                }
            }
        }
    }

    //item control check
    for(UINT32 count = 0; count < eacINVALID; count++)
    {
        //current
        m_sLANCfg.ucItemCtrlCurrent[count] = palDataMgr_MenuItem_Control((eDATA_CONTROL)count);

        //shared memoey data
        sITEM_CONTROL_TABLE sItemCtrlShm;
        if(utilDatabase_ReadData(eSB_ITEM_CONTROL_TABLE, &sItemCtrlShm, count) != DATABASE_ACCESS_PASS)
            continue;

        //update to shared memory
        if(sItemCtrlShm.eItemControl != m_sLANCfg.ucItemCtrlCurrent[count])
        {
            sItemCtrlShm.eItemControl  = m_sLANCfg.ucItemCtrlCurrent[count];
            sItemCtrlShm.uiDataControl = count;

            utilDatabase_WriteData(eSB_ITEM_CONTROL_TABLE, &sItemCtrlShm, count);

            //need update OSD
            UINT8 OSDUpdate = TRUE;
            //palDataMgr_Data_Access(edcUI_EVENT_UPDATE_OSD, edaWRITE_THROUGH_WITH_ACTION, &OSDUpdate);
        }
    }
}

//G100_Coda_0016
// ==============================================================================
// FUNCTION NAME: palLANProcSturctDataSend
// DESCRIPTION:
//
//
// Params:
// UINT16 uiData:
// UINT16 uiSize:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/10/15, Coda Create
// --------------------
// ==============================================================================
void palLANProcSturctDataSend(UINT16 uiSturctType, UINT16 uiSize, UINT8 *pucData)
{
    utilIpc_SendData(eIPC_SEND_DATA_STRUCT, uiSturctType, pucData, eEXEC_CODE_PASS, uiSize);
}

//G100_Coda_0025
// ==============================================================================
// FUNCTION NAME: palLANProcScheduleSend
// DESCRIPTION:
//
//
// Params:
// UINT16 uiData:
// UINT16 uiSize:
// UINT8 *pucData:
//
// Returns:
//
//
// Modification History
// --------------------
// 2020/10/22, Coda Create
// --------------------
// ==============================================================================
void palLANProcScheduleSend(void)
{
    //Update Share Memory
    palLANProcSendToLAN(edcSTRUCT_SCHEDULE);
    palLANProcSendToLAN(edcSCHEDULE_ALL_DATA);


/*
    sLAN_SCHEDULE_StructPayload sStructPayload;
    sLAN_SCHEDULE_PACKET sSendSchedule;

    memset((UINT8 *)&sStructPayload, 0, sizeof(_sStructPayload));
    memset((UINT8*)&sSendSchedule, 0, sizeof(sLAN_SCHEDULE_PACKET));
    palDataMgr_Schedule_Build_LanPacket(&sSendSchedule);
    memcpy((UINT8*)&sStructPayload.Data, (UINT8*)&sSendSchedule, sizeof(sLAN_SCHEDULE_PACKET));

    UINT16 uwStructID = 1 ;//0 : time

    palLANProcSturctDataSend(uwStructID, sizeof(sLAN_SCHEDULE_StructPayload), (void *)&sStructPayload);

    LOG_MSG(db_APP_SCHEDULE, "Build Schedule Struct %d\n", sizeof(sLAN_SCHEDULE_StructPayload));
*/
}

void palLANProcScheduleSend_Update(void)
{
    palLANProcSendToLAN(edcSCHEDULE_MODE);
    palLANProcSendToLAN(edcSCHEDULE_MONDAY_ENABLE);
    palLANProcSendToLAN(edcSCHEDULE_TUESDAY_ENABLE);
    palLANProcSendToLAN(edcSCHEDULE_WEDNESDAY_ENABLE);
    palLANProcSendToLAN(edcSCHEDULE_THURSDAY_ENABLE);
    palLANProcSendToLAN(edcSCHEDULE_FRIDAY_ENABLE);
    palLANProcSendToLAN(edcSCHEDULE_SATURDAY_ENABLE);
    palLANProcSendToLAN(edcSCHEDULE_SUNDAY_ENABLE);

}

void palLANProcForceCheckUpdateData(void)   //G100_Simon_0035
{
    ucForceCheckUpdateData = TRUE;
}

void palLANProcForceCheckUpdateDataSub(void)   //HICC2_Doulas_0113
{
    ucForceCheckUpdateDataSub = TRUE;
}

void palLANProcUpdateOSD_Set(BOOL bFlag)
{
    m_bUpdateOSD = bFlag;
}

void palLANProcPowerNormal(void) //A35G2_CDS_Larry_0028
{
    UINT8   ucDHCP = 0;

    palDataMgr_Data_Access(edcLAN_DHCP, edaREAD, &ucDHCP);


    if(ucDHCP)
    {
       palLANProcSendToLAN(edcLAN_APPLY);
    }
}

void palLANProcFocusPowerStatusSend(void)
{
    palDataMgr_ImportDataToDatabase(edcPOWER_STATUS);
    utilIpc_NotifyStructureDatatypeDatacode(edcPOWER_STATUS);
}

