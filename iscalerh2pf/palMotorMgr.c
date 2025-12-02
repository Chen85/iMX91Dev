#include "palMotorMgr.h"
#include "halMotorCtrlAPI.h"
#include "dvMotorBoard.h"

static BOOL m_cLensEncode = FALSE;
static BOOL m_bIsCeilingMount = FALSE;
static BOOL m_bIsRear = FALSE;
eMOTOR_GEC_LSM_DETECTION_STATE m_LensDetection_State;
static UINT8 m_LensLock_State = FALSE;

void palMotorLens_Init(void) //HICC2_Casper_0034
{
    sLENSMEMORY sLensMem = {0}; //HICC2_Casper_0030

    palMotorLensPositionGet(&sLensMem); //HICC2_Casper_0030

    palDataMgr_Data_Access(edcCUR_H_POSITION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.wLensHPosition); //HICC2_Casper_0030
    palDataMgr_Data_Access(edcCUR_V_POSITION, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.wLensVPosition); //HICC2_Casper_0030
    palDataMgr_Data_Access(edcCUR_H_DIR, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.cDirH); //HICC2_Casper_0030
    palDataMgr_Data_Access(edcCUR_V_DIR, edaWRITE_RAM_ONLY_NO_ACTION, (void*)&sLensMem.cDirV); //HICC2_Casper_0030
}

// ==============================================================================
// FUNCTION NAME: palMotorEvent_RemapSet
// DESCRIPTION:
//
//
// Params:
// BOOL bCeiling:
// BOOL bIsRear:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/09, Larry Create
// --------------------
// ==============================================================================
void palMotorEvent_CeilingSet(BOOL bCeiling)
{
    m_bIsCeilingMount = bCeiling;
}

// ==============================================================================
// FUNCTION NAME: palMotorEvent_CeilingGet
// DESCRIPTION:
//
//
// Params:
// BOOL bCeiling:
// BOOL bIsRear:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/12/25, Doulas Create
// --------------------
// ==============================================================================
BOOL palMotorEvent_CeilingGet(void)     //A70LV_Doulas_0062
{
    return m_bIsCeilingMount;
}

// ==============================================================================
// FUNCTION NAME: palMotorEvent_RearSet
// DESCRIPTION:
//
//
// Params:
// BOOL bIsRear:
//
// Returns:
//
//
// Modification History
// --------------------
// 2017/11/09, Larry Create
// --------------------
// ==============================================================================
void palMotorEvent_RearSet(BOOL bIsRear)
{
    m_bIsRear = bIsRear;
}

// ==============================================================================
// FUNCTION NAME: palMotorEvent_RearGet
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
// 2019/04/02, Larry Create
// --------------------
// ==============================================================================
BOOL palMotorEvent_RearGet(void)
{
    return m_bIsRear;
}

// ==============================================================================
// FUNCTION NAME: palMotorEvent_Remap
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
// 2017/11/09, Larry Create
// --------------------
// ==============================================================================
static eLENS_CMDS palMotorEvent_Remap(eLENS_CMDS eLensEvent)
{
    eLENS_CMDS eNewLensEvent = eLensEvent;
    BOOL bIsCeilingMount = m_bIsCeilingMount;
    UINT8 ucLensType = eLENS_ID_NVRAM_ID;

    palDataMgr_Data_Access(edcLENS_TYPE, edaREAD, (void*)&ucLensType); //HICC2_Casper_0030

    if((ucLensType == eLENS_ID_B20U100) || (ucLensType == eLENS_ID_NVRAM_B20U100))//HICC2_Julie_0070//H30K_Julie_0005
    {
        if(bIsCeilingMount)
        {
            bIsCeilingMount = FALSE;
        }
        else
        {
            bIsCeilingMount = TRUE;
        }
    }

    if(bIsCeilingMount == TRUE)
    {
        switch(eLensEvent)
        {
            case eLENS_CMDS_STEP_LEFT:
                eNewLensEvent = eLENS_CMDS_STEP_RIGHT;
                break;

            case eLENS_CMDS_STEP_RIGHT:
                eNewLensEvent = eLENS_CMDS_STEP_LEFT;
                break;

            case eLENS_CMDS_STEP_UP:
                eNewLensEvent = eLENS_CMDS_STEP_DOWN;
                break;

            case eLENS_CMDS_STEP_DOWN:
                eNewLensEvent = eLENS_CMDS_STEP_UP;
                break;

            case eLENS_CMDS_RUN_LEFT:
                eNewLensEvent = eLENS_CMDS_RUN_RIGHT;
                break;

            case eLENS_CMDS_RUN_RIGHT:
                eNewLensEvent = eLENS_CMDS_RUN_LEFT;
                break;

            case eLENS_CMDS_RUN_LEFT_DOUBLE: //G100_Larry_0040
                eNewLensEvent = eLENS_CMDS_RUN_RIGHT_DOUBLE;
                break;

            case eLENS_CMDS_RUN_RIGHT_DOUBLE: //G100_Larry_0040
                eNewLensEvent = eLENS_CMDS_RUN_LEFT_DOUBLE;
                break;

            case eLENS_CMDS_RUN_UP:
                eNewLensEvent = eLENS_CMDS_RUN_DOWN;
                break;

            case eLENS_CMDS_RUN_DOWN:
                eNewLensEvent = eLENS_CMDS_RUN_UP;
                break;

            case eLENS_CMDS_RUN_UP_DOUBLE: //G100_Larry_0040
                eNewLensEvent = eLENS_CMDS_RUN_DOWN_DOUBLE;
                break;

            case eLENS_CMDS_RUN_DOWN_DOUBLE: //G100_Larry_0040
                eNewLensEvent = eLENS_CMDS_RUN_UP_DOUBLE;
                break;

            case eLENS_CMDS_GO_LEFT:
                eNewLensEvent = eLENS_CMDS_GO_RIGHT;
                break;

            case eLENS_CMDS_GO_RIGHT:
                eNewLensEvent = eLENS_CMDS_GO_LEFT;
                break;

            case eLENS_CMDS_GO_TOP:
                eNewLensEvent = eLENS_CMDS_GO_DOWN;
                break;

            case eLENS_CMDS_GO_DOWN:
                eNewLensEvent = eLENS_CMDS_GO_TOP;
                break;

            //G100_Wilsonj_0018 Start
            case eLENS_CMDS_PIXEL_LEFT:
				eNewLensEvent = eLENS_CMDS_PIXEL_RIGHT;
				break;

			case eLENS_CMDS_PIXEL_RIGHT:
				eNewLensEvent = eLENS_CMDS_PIXEL_LEFT;
				break;

			case eLENS_CMDS_PIXEL_UP:
				eNewLensEvent = eLENS_CMDS_PIXEL_DOWN;
				break;

			case eLENS_CMDS_PIXEL_DOWN:
				eNewLensEvent = eLENS_CMDS_PIXEL_UP;
				break;
            //G100_Wilsonj_0018 End

            case eLENS_CMDS_SHIFT_TOP: //A35G2_Wesley_0153 //A70Gen2_Julie_0109//HICC2_Julie_0014
				eNewLensEvent = eLENS_CMDS_SHIFT_BOTTOM;
				break;

			case eLENS_CMDS_SHIFT_BOTTOM: //A35G2_Wesley_0153 //A70Gen2_Julie_0109//HICC2_Julie_0014
				eNewLensEvent = eLENS_CMDS_SHIFT_TOP;
				break;

            default:
                break;
        }
    }

    if(m_bIsRear == TRUE)
    {
       switch(eNewLensEvent)
       {
           case eLENS_CMDS_STEP_LEFT:
               eNewLensEvent = eLENS_CMDS_STEP_RIGHT;
               break;

           case eLENS_CMDS_STEP_RIGHT:
               eNewLensEvent = eLENS_CMDS_STEP_LEFT;
               break;

           case eLENS_CMDS_RUN_LEFT:
               eNewLensEvent = eLENS_CMDS_RUN_RIGHT;
               break;

           case eLENS_CMDS_RUN_RIGHT:
               eNewLensEvent = eLENS_CMDS_RUN_LEFT;
               break;

           case eLENS_CMDS_GO_LEFT:
               eNewLensEvent = eLENS_CMDS_GO_RIGHT;
               break;

           case eLENS_CMDS_GO_RIGHT:
               eNewLensEvent = eLENS_CMDS_GO_LEFT;
               break;

           case eLENS_CMDS_RUN_LEFT_DOUBLE: //G100_Larry_0040
               eNewLensEvent = eLENS_CMDS_RUN_RIGHT_DOUBLE;
               break;

           case eLENS_CMDS_RUN_RIGHT_DOUBLE: //G100_Larry_0040
               eNewLensEvent = eLENS_CMDS_RUN_LEFT_DOUBLE;
               break;

           default:
               break;
       }
    }

    return eNewLensEvent;
}


void palMotorLensSet(eLENS_CMDS eLensMoving)
{
    eLensMoving = palMotorEvent_Remap(eLensMoving);

    halMotorLensSet(eLensMoving);

    return;
}

void palMotorLensRunPixelSet(eLENS_CMDS eLensCmd, UINT32 ulPixel)  //T100_Casper_0099
{
    eLensCmd = palMotorEvent_Remap(eLensCmd);

    halMotorLensRunPixelSet(eLensCmd, ulPixel);
}

void palMotorFocusSet(eZOOMLENS_CMD eFocusMoving)
{
    halMotorFocusSet(eFocusMoving);

    return;
}

void palMotorFocusSetWithDuration(eZOOMLENS_CMD eFocusMoving, UINT16 uiDuration)//A65_OPTOMA_CL_0011
{
    if(uiDuration <35)  //minimum value 35 ms
        uiDuration = 35;

    halMotorFocusSetWithDuration(eFocusMoving, uiDuration);

    return;
}

UINT16 palMotorFocusDurationGet(void)
{
    UINT8 ucData[2] = {0};
    UINT16 uiData = 0;
    eRESULT eResult = rcERROR;

    halMotorFocusDurationGet(ucData);

    uiData = ucData[0] + (((UINT16)ucData[1]) <<8);

    return uiData;
}

UINT8 palMotorFocusGet(void)
{
    UINT8 cData = 0xF0;

    cData = halMotorFocusGet();

    return cData;
}

void palMotorZoomSet(eZOOMLENS_CMD eZoomMoving)
{
    halMotorZoomSet(eZoomMoving);
}


WORD palMotorLensCalibrationStatusGet(void)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    UINT16 wData = 0xFFFF;

    wData = (UINT16)halMotor_LSM_CalibrationStatus_Get();

    return wData;
#else
    UINT16 wData = 0xFFFF;

    wData = halMotorLensCalibrationStatusGet();

    return wData;
#endif
}

void palMotorLensGoCenter(void)
{
    //halMotorLensGoCenter();
}

UINT8 palMotorLensMovingStateGet(void)
{
    UINT8 cData = 0xFF;

    cData = halMotorLensMovingStateGet();

    return cData;
}

//unused
UINT8 palMotorLensMovingStartGet(void)
{
    //eLENS_MOVING_IDLE,      // Idle
    //eLENS_MOVING_GOING_TO_START,    // Go Start
    //eLENS_MOVING_GO_LEFT,   // Go Left
    //eLENS_MOVING_GO_TOP,    // Go Top
    //eLENS_MOVING_GO_RIGHT,  // Go Right
    //eLENS_MOVING_GO_DOWN,   // Go Down
    //eLENS_MOVING_GO_CENTER, // Go Center
    //eLENS_MOVING_ERROR

    UINT8 cData = 0;

    return cData;
}

void palMotorLensPositionGet(sLENSMEMORY *psLensmemory)
{
    halMotorLensPositionGet(psLensmemory);
	//OSD (Left : 0,Down : 1), Motor Driver (Near Sensor : 1,Far Sensor : 0) (Up : 1,Down : 0), Motor Driver (Near Sensor : 1,Far Sensor : 0)
    psLensmemory->cDirV = !psLensmemory->cDirV;  //HICC2_Casper_0035
}

//unused
void palMotorLensMemorySet(sLENSMEMORY *psLensmemory)
{
    //halMotorLensMemorySet(psLensmemory);
}

//unused
BOOL palMotorLensIdleGet(void)
{
    BOOL bFlag = TRUE;

    bFlag = halMotorLensIdleGet();

    return bFlag;
}


//unused
void palMotor_Focue_Limit_Get(sMOTOR_LIMIT *psMotor_limit)
{
    halMotor_Focue_Limit_Get(psMotor_limit);
}

//unused
void palMotor_Zoom_Limit_Get(sMOTOR_LIMIT *psMotor_limit)
{
    halMotor_Zoom_Limit_Get(psMotor_limit);
}


void palMotor_Lens_Backlash_Get(sMOTOR_LIMIT *psMotor_limit)
{
    halMotor_Lens_Backlash_Get(psMotor_limit);
}

//unused
UINT8 palMotor_Lens_Encode_Get(void)
{
    UINT8 cData = 0;

    cData = halMotor_Lens_Encode_Get();

    return cData;
}

BOOL palMoter_LensEncodeSaveGet(void)
{
    return m_cLensEncode;
}

void palMotorLensMemorySaveSet(UINT8 cLensMemoryID)
{
    halMotorLensMemorySaveSet(cLensMemoryID);
}

//unused
UINT8 palMotorLensMemoryApplyGet(void)
{
    halMotorLensMemoryApplyGet();
}


void palMotorLensMemoryApplySet(UINT8 cLensMemoryID)
{
    halMotorLensMemoryApplySet(cLensMemoryID);
}

//unused
void palMotorLensA16MotorGoCenter(void)
{
    halMotorLensA16MotorGoCenter();
}

//unused
void palMotorLensSelectSet(UINT8 cLens)
{
    halMotorLensSelectSet(cLens);
}

eRESULT palMotor_Version_Get(UINT8* pcData)
{
    return halMotor_Version_Get(pcData);
}

eRESULT palMotor_LENS_EEPROM_Version_Get(UINT8* pcData)  //HICC2_Steven_0027
{
    return halMotor_LENS_EEPROM_Version_Get(pcData);
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensModule_Get
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
// 2018/03/05, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensModule_Get(UINT8* pcData)
{
    return halMotor_LensModule_Get(pcData);
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensModule_Set
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
// 2018/03/05, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensModule_Set(UINT8* pcData)
{
    if(*pcData > eLENS_MODULE_A16 ||
       (*pcData > eLENS_MODULE_H60 && *pcData < eLENS_MODULE_A16)
    )
    {
        LOG_MSG(db_ALWAYS, "!!! undefined Lens Module ID %d\n", *pcData);
        return rcERROR;
    }

    return halMotor_LensModule_Set(pcData);
}

// ==============================================================================
// FUNCTION NAME: palMotor_OEJig_Set
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
// 2018/03/05, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_OEJig_Set(UINT8 Enable)
{
    return halMotor_OEJig_Set(Enable);
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensMemoryRecord_Get
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
// 2018/04/23, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensMemoryRecord_Get(UINT8* pcData)
{
    return halMotor_LensMemoryRecord_Get(pcData);
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensMemoryReset
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
// 2018/04/23, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensMemoryReset(void)
{
#if 0
    typedef enum
    {
        eNVRAM_NODE_EVENT_INITIALIZE,   // Standard reset (not calibration nodes)
        eNVRAM_NODE_EVENT_RESETALL,     // Complete reset including calibration nodes

        eNVRAM_NODE_EVENT_NUMBERS,      // used for enumerated type range checking (DO NOT REMOVE)
    } eNVRAM_NODE_EVENT;
#endif /* 0 */

    return halMotor_LensMemoryReset();  //A35G2_Larry_0069
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensMemoryStatus_Get
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
// 2018/04/23, Larry Create
// --------------------
// ==============================================================================
UINT8 palMotor_LensMemoryStatus_Get(void)
{
    UINT8 cData = 0xFF;

    cData = halMotor_LensMemoryStatus_Get();

    return cData;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensId_Get
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
// 2018/05/31, Larry Create
// --------------------
// ==============================================================================
UINT8 palMotor_LensId_Get(void)
{
    return halMotor_LensId_Get();
}

// ==============================================================================
// FUNCTION NAME: palMotor_ZoomLimit_Get
// DESCRIPTION:
//
//
// Params:
// sMOTOR_LIMIT *psLimit:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/08, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_ZoomLimit_Get(sMOTOR_LIMIT *psLimit)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_ZoomLimit_Get(psLimit);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_FocusLimit_Get
// DESCRIPTION:
//
//
// Params:
// sMOTOR_LIMIT *psLimit:
//
// Returns:
//
//
// Modification History
// --------------------
// 2018/06/08, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_FocusLimit_Get(sMOTOR_LIMIT *psLimit)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_FocusLimit_Get(psLimit);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensPlug_Get
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
// 2018/07/25, Larry Create
// --------------------
// ==============================================================================
UINT8 palMotor_LensPlug_Get(void)
{
    UINT8 cData = 0;

    cData = halMotor_LensPlug_Get();

    return cData;
}


// ==============================================================================
// FUNCTION NAME: palMotor_LensFujiDet_Get
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
// 2024/08/07, Larry Create
// --------------------
// ==============================================================================
UINT8 palMotor_LensFujiDet_Get(void)
{
    UINT8 cData = 0;

    cData = halMotor_LensFujiDet_Get();

    return cData;
}

eRESULT palMotor_LensIdADC_Get(UINT32 *psIDAdc)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensIdADC_Get(psIDAdc);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_WR_Test
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
// 2020/09/24, Owen Create
// --------------------
// ==============================================================================
UINT8 palMotor_WR_Test(void)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_WR_Test();

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensCenterEnable_Get
// DESCRIPTION:
//
//
// Params:
// UINT8 *pucEnable:
//
// Returns:
//
//
// Modification History
// --------------------
// 2021/01/12, Owen Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensCenterEnable_Get(UINT8 *pucEnable)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCenterEnable_Get(pucEnable);

    return eResult;
}

eRESULT palMotor_LensCenterEnable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCenterEnable_Set(ucEnable);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensCenterApply_Set
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
// 2021/04/11, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensCenterApply_Set(void)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCenterApply_Set();

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensCalDone_Get
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
// 2021/04/11, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensCalDone_Get(UINT8 *pucLensCalFlg)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCalDone_Get(pucLensCalFlg);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensCenterSetting
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
// 2021/04/11, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensCenterSettingSet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCenterSettingSet(pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensCenterSetting
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
// 2021/04/11, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensCenterSettingGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCenterSettingGet(pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_ZoomDutySet
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
// 2021/07/07, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_ZoomDutySet(BYTE cMax, BYTE cMin)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_ZoomDutySet(cMax, cMin);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_FocusDutySet
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
// 2021/07/07, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_FocusDutySet(BYTE cMax, BYTE cMin)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_FocusDutySet(cMax, cMin);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensCenterGet
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
// 2021/10/18, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensCenterGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensCenterGet(pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensPositionFullGet
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
// 2021/11/23, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensPositionFullGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensPositionFullGet(pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: palMotor_LensPositionActiveGet
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
// 2021/11/23, Larry Create
// --------------------
// ==============================================================================
eRESULT palMotor_LensPositionActiveGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_LensPositionActiveGet(pcData);//HICC2_Julie_0018

    return eResult;
}

eRESULT palMotor_Lens_Speed_Set(UINT8* pcData)
{
//set OFF = 250pps (Standard lens)
//set ON  = 150pps (UST lens)

    eRESULT eResult = rcERROR;

    eResult = halMotor_Lens_Speed_Set(pcData);

    return eResult;
}

eRESULT palMotor_Lens_Speed_Control_Set(UINT8* pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lens_Speed_Control_Set(pcData);

    return eResult;
}

eRESULT palMotor_Lens_Speed_Control_Get(UINT16 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lens_Speed_Control_Get(pcData);

    return eResult;
}

UINT8 palMotor_Lens_Shift_Status_Get(void) //HICC2_Casper_0041
{
    UINT8 cData = 0xFF;

    cData = halMotor_LensShiftStatus_Get();

    return cData;
}

eRESULT palMotor_Lens_FujiLens_BackFocus_Set(UINT8* pcData) //H30K_Julie_0005
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_FujiLens_BackFocus_Set(pcData);

    return eResult;
}

UINT8 palMotor_Lsm_Sensor_Mute_H_Get(void) //HICC2_AC_0046
{
    UINT8 cData = 0xFF;

    cData = halMotor_Lsm_Sensor_Mute_H_Get();

    return cData;
}

eRESULT palMotor_Lsm_Sensor_Mute_H_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_Sensor_Mute_H_Set(ucMute);

    return eResult;
}

UINT8 palMotor_Lsm_Sensor_Mute_V_Get(void)
{
    UINT8 cData = 0xFF;

    cData = halMotor_Lsm_Sensor_Mute_V_Get();

    return cData;
}

eRESULT palMotor_Lsm_Sensor_Mute_V_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_Sensor_Mute_V_Set(ucMute);

    return eResult;
}

UINT8 palMotor_Lsm_Boundary_Mute_H_Get(void)
{
    UINT8 cData = 0xFF;

    cData = halMotor_Lsm_Boundary_Mute_H_Get();

    return cData;
}

eRESULT palMotor_Lsm_Boundary_Mute_H_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_Boundary_Mute_H_Set(ucMute);

    return eResult;
}

UINT8 palMotor_Lsm_Boundary_Mute_V_Get(void)
{
    UINT8 cData = 0xFF;

    cData = halMotor_Lsm_Boundary_Mute_V_Get();

    return cData;
}

eRESULT palMotor_Lsm_Boundary_Mute_V_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_Boundary_Mute_V_Set(ucMute);

    return eResult;
}

eRESULT palMotor_Lsm_Boundary_Calibration_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_Boundary_Calibration_Set(ucMute);

    return eResult;
}

eRESULT palMotor_Lsm_V_Up_Boundary_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_V_Up_Boundary_Set(ucMute);

    return eResult;
}

eRESULT palMotor_Lsm_V_Down_Boundary_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_V_Down_Boundary_Set(ucMute);

    return eResult;
}

eRESULT palMotor_Lsm_H_Left_Boundary_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_H_Left_Boundary_Set(ucMute);

    return eResult;
}

eRESULT palMotor_Lsm_H_Right_Boundary_Set(UINT8 ucMute)
{
    eRESULT eResult = rcERROR;

    eResult = halMotor_Lsm_H_Right_Boundary_Set(ucMute);

    return eResult;
}

void palMotor_LensDetection_State_Set(BYTE ucVal)
{
    m_LensDetection_State = ucVal;
}

UINT8 palMotor_LensDetection_State_Get(void)
{
    return m_LensDetection_State;
}

void palMotor_LensLock_State_Set(BYTE ucVal) //H30K_Julie_0005
{
    //ucVal=0: LensLockPlate(DET2) isn't Detected
    //ucVal=1: LensLockPlate(DET2) is Detected
    BYTE cValue = 0;
    cValue = (ucVal == 0 ? 1 : 0);

    m_LensLock_State = cValue;
}

UINT8 palMotor_LensLock_State_Get(void) //H30K_Julie_0005
{
    return m_LensLock_State;
}

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002

eRESULT palMotor_LSM_Version_Get(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Version_Get();

    return eResult;
}

eRESULT palMotor_LSM_Center_Get(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Center_Get();

    return eResult;
}

eRESULT palMotor_LSM_Center_Set(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Center_Set();

    return eResult;
}

eRESULT palMotor_LSM_Center_Clear(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Center_Clear();

    return eResult;
}

eRESULT palMotor_LSM_Status_Ask(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Status_Ask();

    return eResult;
}

eRESULT palMotor_LSM_CalibrationDone_Ask(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_CalibrationDone_Ask();

    return eResult;
}

eRESULT palMotor_LSM_CalibrationStatus_Set(UINT8 wData)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_CalibrationStatus_Set(wData);

    return eResult;
}

void palMotor_LSM_CalibrationDone_Set(UINT8 wData)
{
    halMotor_LSM_CalibrationDone_Set(wData);
}

UINT8 palMotor_LSM_CalibrationDone_Get(void)
{
    return halMotor_LSM_CalibrationDone_Get();
}

eRESULT palMotor_LSM_Lens_ID_Get(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Lens_ID_Ask();

    return eResult;
}

eRESULT palMotor_LSM_Factory_Menu(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Factory_Menu_Lens();
    halMotor_LSM_Factory_Menu_ZoomFocus();

    return eResult;
}

eRESULT palMotor_LSM_Memory_Set(UINT8 uData) //HICC2_AC_0019
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Memory_Set(uData);

    return eResult;
}

eRESULT palMotor_LSM_Memory_Query(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Memory_Query();

    return eResult;
}

eRESULT palMotor_LSM_Memory_Clear(void)
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Memory_Clear();

    return eResult;
}

eRESULT palMotor_LSM_Plug_Query(void)	//HICC2_Zonic_0009
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Lens_Plug_Query();

    return eResult;
}

eRESULT palMotor_LSM_Lock_Query(void) //H30K_Julie_0005
{
    eRESULT eResult = rcSUCCESS;

    halMotor_LSM_Lens_Lock_Query();

    return eResult;
}

void palMotor_LSM_Lens_Plug_Set(UINT8 wData) //H30K_Julie_0005
{
    halMotor_LSM_Lens_Plug_Set(wData);
}

UINT8 palMotor_LSM_Lens_Plug_Get(void) //H30K_Julie_0005
{
    return halMotor_LSM_Lens_Plug_Get();
}

void palMotor_LSM_Lens_Detection_Bypass(BOOL bBypass)
{
	halMotor_LSM_Lens_Detection_Bypass(bBypass);
}

void palMotor_LSM_Lens_Detection_Bypass_Query(void)
{
    halMotor_LSM_Lens_Detection_Bypass_Query();
}



#endif

int palMotor_LensTable_GetRow(sFILE_PARSE *psFileParse, const char *pFile)
{
	int ret = -1;
	FILE *pf1 = fopen(pFile, "r");
	if (pf1 == NULL)
	{
		return(ret);
	}

	UINT32 i = 0;
	do 
	{
		fseek(pf1, psFileParse->GetSeek, SEEK_SET);
		memset(psFileParse->GetBuffer, 0, psFileParse->Length);
		if(fgets(psFileParse->GetBuffer, psFileParse->Length, pf1) == NULL)
		{
			if(feof(pf1) != 0)
			{
				ret = 0;
			}
			else
			{
				ret = -2;
			}
			break;
		}
		else
		{
			psFileParse->GetSeek += strlen(psFileParse->GetBuffer);
			ret = 1;
		}
		
		i++;
	} while(i <= psFileParse->Row);
	
	fclose(pf1);
	return(ret);
}

int palMotor_LensTable_ParseRow(sFILE_PARSE *psFileParse, sLENS_TYPE *psLensType)
{
	char *str = strstr(psFileParse->GetBuffer, ",,");
	if(str != NULL)
	{
		return(-1);
	}

    str = strtok(psFileParse->GetBuffer, "\r\n");

	str = strtok(psFileParse->GetBuffer, ",");
	if(str == NULL)
	{
		return(-2);
	}
	else
	{
		strcpy(psLensType->cID, str);
	}
	
	str = strtok(NULL, ",");
	if(str == NULL)
	{
		return(-3);
	}
	else
	{
		psLensType->lDefined = atoi(str);
	}

	str = strtok(NULL, ",");
	if(str == NULL)
	{
		return(-4);
	}
	else
	{
		psLensType->lSupported = atoi(str);
	}

	str = strtok(NULL, ",");
	if(str == NULL)
	{
		return(-5);
	}
	else
	{
		strcpy(psLensType->cLensName, str);
	}

	str = strtok(NULL, ",");
	if(str == NULL)
	{
		return(-6);
	}
	else
	{
		strcpy(psLensType->cThrowRatio, str);
	}

	return(1);
}

