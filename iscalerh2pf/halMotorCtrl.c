// ==============================================================================
// FILE NAME: HALMOTORCTRL.C
// DESCRIPTION:
//
//
// modification history
// --------------------
// 06/09/2013, Larry
// 15/11/2013, Larry Fixed
// 06/01/2014, Larry Fixed
// 07/11/2014, Larry Fixed //A70_Larry_0197
// A70_Larry_0306 fixd
// --------------------
// ==============================================================================

#include "halMotorCtrlAPI.h"
#include "dvMotorBoard.h"
#include "utilDbgMsg.h"
#include "utilCLICmdAPI.h"
#include "dvMCUDriver.h"
#include "Board_I2C_Dev_Table.h"
#include "utilHostAPI.h"

static BOOL m_cLensEncode = FALSE;
static UINT32 MotorController = eMOTOR_CONTROLLER_MOTOR54113 ;

UINT8 wLSM_Sensor_Mute_H = FALSE;
UINT8 wLSM_Sensor_Mute_V = FALSE;
UINT8 wLSM_Boundary_Mute_H = FALSE;
UINT8 wLSM_Boundary_Mute_V = FALSE;

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
UINT8 wLSM_CalibrationStatus = 0;
UINT8 wLSM_CalibrationDone = 0;
UINT8 uLSM_Lens_Module = 0;
UINT8 uLSM_Lens_ID = 0;
UINT8 uLSM_Lens_Center_Config = 0;
UINT8 uLSM_Lens_Memory_Record = 0; //HICC2_AC_0019
UINT8 uLSM_Lens_Plug = 0; //0: has lens, 1: no lens //HICC2_AC_0021
UINT8 uLSM_Lens_NVRAM_Status = FALSE;
#endif

eHAL_MOTOR_EXEC_CODE halMotor_Init(void) //HICC2_Casper_0034
{
    MotorController = Syscfg_Value_Get_Typeint(eMotorController);
	
	return eHAL_MOTOR_EXEC_CODE_PASS;
}

void halMotorLensSet(eLENS_CMDS eLensMoving)
{
    if(MotorController == eMOTOR_CONTROLLER_SYS54605)
    {
        switch(eLensMoving)
        {
            case eLENS_CMDS_STEP_LEFT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_LEFT_STEP, 0, NULL);
                break;

            case eLENS_CMDS_STEP_RIGHT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_RIGHT_STEP, 0, NULL);
                break;

            case eLENS_CMDS_STEP_UP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_UP_STEP, 0, NULL);
                break;

            case eLENS_CMDS_STEP_DOWN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_DOWN_STEP, 0, NULL);
                break;

            case eLENS_CMDS_RUN_LEFT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_LEFT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_RUN_RIGHT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_RIGHT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_RUN_UP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_UP_RUN, 0, NULL);
                break;

            case eLENS_CMDS_RUN_DOWN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_DOWN_RUN, 0, NULL);
                break;

            case eLENS_CMDS_GO_LEFT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_LEFT_GO, 0, NULL);
                break;

            case eLENS_CMDS_GO_RIGHT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_RIGHT_GO, 0, NULL);
                break;

            case eLENS_CMDS_GO_TOP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_UP_GO, 0, NULL);
                break;

            case eLENS_CMDS_GO_DOWN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_DOWN_GO, 0, NULL);
                break;

            case eLENS_CMDS_CALIBRAION:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_CAL, 0, NULL);
                break;

            case eLENS_CMDS_CANCEL:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_CANCEL, 0, NULL);
                break;

            case eLENS_CMDS_MOVING:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_MOVING, 0, NULL);
                break;

            case eLENS_CMDS_RUN_LEFT_DOUBLE:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_LEFT_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_RUN_RIGHT_DOUBLE:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_RIGHT_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_RUN_UP_DOUBLE:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_UP_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_RUN_DOWN_DOUBLE:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_DOWN_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_OE_TEST:
                break;

            case eLENS_CMDS_POSTION_SET:
                break;

            case eLENS_CMDS_POSTION_GET:
                break;

            case eLENS_CMDS_A16_CALIBRATION:
                break;

            case eLENS_CMDS_A16_RANGE:
                break;

            case eLENS_CMDS_OLD_RANGE:
                break;

            case eLENS_CMDS_ZOOM_CALIBRAION:
                break;

            case eLENS_CMDS_FOCUS_CALIBRAION:
                break;

            case eLENS_CMDS_LENS_ID_CAL:
                break;

            case eLENS_CMDS_SHIFT_TOP:
                break;

            case eLENS_CMDS_SHIFT_BOTTOM:
                break;

            case eLENS_CMDS_SHIFT_CENTER:
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_FMT54605 ||
            MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        switch(eLensMoving)
        {
            case eLENS_CMDS_STEP_LEFT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_LEFT_STEP, 0, NULL);
                break;

            case eLENS_CMDS_STEP_RIGHT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_RIGHT_STEP, 0, NULL);
                break;

            case eLENS_CMDS_STEP_UP:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_UP_STEP, 0, NULL);
                break;

            case eLENS_CMDS_STEP_DOWN:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_DOWN_STEP, 0, NULL);
                break;

            case eLENS_CMDS_RUN_LEFT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_LEFT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_RUN_RIGHT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_RIGHT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_RUN_UP:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_UP_RUN, 0, NULL);
                break;

            case eLENS_CMDS_RUN_DOWN:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_DOWN_RUN, 0, NULL);
                break;

            case eLENS_CMDS_GO_LEFT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_LEFT_GO, 0, NULL);
                break;

            case eLENS_CMDS_GO_RIGHT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_RIGHT_GO, 0, NULL);
                break;

            case eLENS_CMDS_GO_TOP:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_UP_GO, 0, NULL);
                break;

            case eLENS_CMDS_GO_DOWN:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_DOWN_GO, 0, NULL);
                break;

            case eLENS_CMDS_CALIBRAION:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_CAL, 0, NULL);
                break;

            case eLENS_CMDS_CANCEL:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_CANCEL, 0, NULL);
                break;

            case eLENS_CMDS_MOVING:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_MOVING, 0, NULL);
                break;

            case eLENS_CMDS_RUN_LEFT_DOUBLE: //G100_Larry_0040
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_LEFT_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_RUN_RIGHT_DOUBLE: //G100_Larry_0040
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_RIGHT_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_RUN_UP_DOUBLE: //G100_Larry_0040
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_UP_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_RUN_DOWN_DOUBLE: //G100_Larry_0040
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_DOWN_RUN_DOUBLE, 0, NULL);
                break;

            case eLENS_CMDS_OE_TEST:
                //dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_POSTION_SET:
                //dvMotorRegWrite(eMOTOR_MSG_ZOOM_IN_STEP, 0, NULL);
                break;

            case eLENS_CMDS_POSTION_GET:
                //dvMotorRegWrite(eMOTOR_MSG_ZOOM_IN_RUN, 0, NULL);
                break;

            case eLENS_CMDS_A16_CALIBRATION:
                //dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_STEP, 0, NULL);
                break;

            case eLENS_CMDS_A16_RANGE:
                //dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_OLD_RANGE:
                //dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_RUN, 0, NULL);
                break;

            case eLENS_CMDS_ZOOM_CALIBRAION:    //HICC2_Doulas_0056
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_CAL, 0, NULL);
                break;

            case eLENS_CMDS_FOCUS_CALIBRAION:   //HICC2_Doulas_0056
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_CAL, 0, NULL);
                break;

            case eLENS_CMDS_LENS_ID_CAL:        //HICC2_Doulas_0056
                dvMotorRegWrite(eMOTOR_MSG_LENS_ID, 0, NULL);
                break;

            case eLENS_CMDS_SHIFT_TOP: //A35G2_Wesley_0153 //A70Gen2_Julie_0109//HICC2_Julie_0014
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_TOP, 0, NULL);
                break;

            case eLENS_CMDS_SHIFT_BOTTOM: //A35G2_Wesley_0153 //A70Gen2_Julie_0109//HICC2_Julie_0014
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_BOTTOM, 0, NULL);
                break;

            case eLENS_CMDS_SHIFT_CENTER: //A35G2_Wesley_0153 //A70Gen2_Julie_0109//HICC2_Julie_0014
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_CENTER, 0, NULL);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        switch(eLensMoving)
        {
#ifndef CURSOR_FIXTURE
            case eLENS_CMDS_STEP_LEFT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_LEFT_SINGLE_STEP, 0);
                break;

            case eLENS_CMDS_STEP_RIGHT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_RIGHT_SINGLE_STEP, 0);
                break;

            case eLENS_CMDS_STEP_UP:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_UP_SINGLE_STEP, 0);
                break;

            case eLENS_CMDS_STEP_DOWN:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_DOWN_SINGLE_STEP, 0);
                break;

            case eLENS_CMDS_RUN_LEFT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_LEFT_RUN, 0);
                break;

            case eLENS_CMDS_RUN_RIGHT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_RIGHT_RUN, 0);
                break;

            case eLENS_CMDS_RUN_UP:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_UP_RUN, 0);
                break;

            case eLENS_CMDS_RUN_DOWN:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_DOWN_RUN, 0);
                break;

            case eLENS_CMDS_GO_LEFT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_GO_LEFT_LIMIT, 0);
                break;

            case eLENS_CMDS_GO_RIGHT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_GO_RIGHT_LIMIT, 0);
                break;

            case eLENS_CMDS_GO_TOP:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_GO_UP_LIMIT, 0);
                break;

            case eLENS_CMDS_GO_DOWN:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_GO_DOWN_LIMIT, 0);
                break;

            case eLENS_CMDS_CALIBRAION:
                dvMotorGecCmd(eCLI_LSM_CALIBATION, eMLC_CALIBRATION, 0); //HICC2_AC_0010
                break;
#endif
            case eLENS_CMDS_CANCEL:
                dvMotorGecCmd(eCLI_LSM_STATUS, eMLC_LSM_STATUS_STOP, 0);
                break;

            case eLENS_CMDS_MOVING:
                dvMotorGecCmd(eCLI_LSM_CALIBATION, eMLC_CALIBRATION_RECTANGLE, 0); //HICC2_AC_0007
                break;

            case eLENS_CMDS_RUN_LEFT_DOUBLE:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_LEFT_STEP, 500);
                break;

            case eLENS_CMDS_RUN_RIGHT_DOUBLE:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_RIGHT_STEP, 500);
                break;

            case eLENS_CMDS_RUN_UP_DOUBLE:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_UP_STEP, 500);
                break;

            case eLENS_CMDS_RUN_DOWN_DOUBLE:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_DOWN_STEP, 500);
                break;

            case eLENS_CMDS_OE_TEST:
                break;

            case eLENS_CMDS_POSTION_SET:
                break;

            case eLENS_CMDS_POSTION_GET:
                dvMotorGecCmd(eCLI_LSM_POSITION, 0, 0);
                break;

            case eLENS_CMDS_A16_CALIBRATION:
                break;

            case eLENS_CMDS_A16_RANGE:
                break;

            case eLENS_CMDS_OLD_RANGE:
                break;

            case eLENS_CMDS_LENS_ID_CAL:
                break;

            case eLENS_CMDS_RELEASE:
                dvMotorGecCmd(eCLI_LSM_STATUS, eMLC_LSM_STATUS_STOP, 0);
                break;

			case eLENS_CMDS_SHIFT_CENTER:
				dvMotorGecCmd(eCLI_LSM_POSITION, eMLC_LSM_POSITION_MOVE_TO_CENTER, 0);
				break;

            default:
                break;
        }
    }
    return;
}

//G100_Wilsonj_0018 Start
void halMotorLensRunPixelSet(eLENS_CMDS eLensCmd, UINT32 ulPixel)  //T100_Casper_0099
{
    if(MotorController == eMOTOR_CONTROLLER_SYS54605)
    {
        switch(eLensCmd)
        {
            case eLENS_CMDS_PIXEL_LEFT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_LEFT_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            case eLENS_CMDS_PIXEL_RIGHT:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_RIGHT_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            case eLENS_CMDS_PIXEL_UP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_UP_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            case eLENS_CMDS_PIXEL_DOWN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_DOWN_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_FMT54605 ||
        MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        switch(eLensCmd)
        {
            case eLENS_CMDS_PIXEL_LEFT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_LEFT_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            case eLENS_CMDS_PIXEL_RIGHT:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_RIGHT_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            case eLENS_CMDS_PIXEL_UP:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_UP_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            case eLENS_CMDS_PIXEL_DOWN:
                dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_DOWN_RUN_PIXEL, 4, (UINT8 *)&ulPixel);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        switch(eLensCmd)
        {
            case eLENS_CMDS_PIXEL_LEFT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_LEFT_PIXEL, ulPixel);
                break;

            case eLENS_CMDS_PIXEL_RIGHT:
                dvMotorGecCmd(eCLI_LSM_H, eMSH_RIGHT_PIXEL, ulPixel);
                break;

            case eLENS_CMDS_PIXEL_UP:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_UP_PIXEL, ulPixel);
                break;

            case eLENS_CMDS_PIXEL_DOWN:
                dvMotorGecCmd(eCLI_LSM_V, eMSV_DOWN_PIXEL, ulPixel);
                break;

            default:
                break;
        }
    }
}
//G100_Wilsonj_0018 End

void halMotorFocusSet(eZOOMLENS_CMD eFocusMoving)
{
    if(MotorController == eMOTOR_CONTROLLER_SYS54605)
    {
        switch(eFocusMoving)
        {
            case eZOOMLENS_CMD_FOCUS_NEAR_STEP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_FOCUS_NEAR_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_FOCUS_NEAR_RUN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_FOCUS_NEAR_RUN, 0, NULL);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_STEP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_FOCUS_FAR_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_RUN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_FOCUS_FAR_RUN, 0, NULL);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_FMT54605 ||
            MotorController == eMOTOR_CONTROLLER_MOTOR54113 )
    {
        switch(eFocusMoving)
        {
            case eZOOMLENS_CMD_FOCUS_NEAR_STEP:
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_NEAR_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_FOCUS_NEAR_RUN:
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_NEAR_RUN, 0, NULL);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_STEP:
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_FAR_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_RUN:
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_FAR_RUN, 0, NULL);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_FAR_STEP: //HICC2_Julie_0070//H30K_Julie_0005
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_IN_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_FAR_RUN: //HICC2_Julie_0070//H30K_Julie_0005
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_IN_RUN, 0, NULL);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_NEAR_STEP: //HICC2_Julie_0070//H30K_Julie_0005
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_NEAR_RUN: //HICC2_Julie_0070//H30K_Julie_0005
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_RUN, 0, NULL);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        switch(eFocusMoving)
        {
            case eZOOMLENS_CMD_FOCUS_NEAR_STEP:
                dvMotorGecCmd(eCLI_LSM_FOCUS, eMFC_NEAR_SINGLE, 0);
                break;

            case eZOOMLENS_CMD_FOCUS_NEAR_RUN:
                dvMotorGecCmd(eCLI_LSM_FOCUS, eMFC_NEAR_RUN, 0);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_STEP:
                dvMotorGecCmd(eCLI_LSM_FOCUS, eMFC_FAR_SINGLE, 0);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_RUN:
                dvMotorGecCmd(eCLI_LSM_FOCUS, eMFC_FAR_RUN, 0);
                break;

        #if defined(PLATFORM_R70K) || defined (PLATFORM_H30_4K) //HICC2_AC_0004 //for FUJI lens
            case eZOOMLENS_CMD_BACK_FOCUS_NEAR_STEP:
                dvMotorGecCmd(eCLI_LSM_FOCUS2, eMFC_NEAR_SINGLE, 0);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_NEAR_RUN:
                dvMotorGecCmd(eCLI_LSM_FOCUS2, eMFC_NEAR_RUN, 0);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_FAR_STEP:
                dvMotorGecCmd(eCLI_LSM_FOCUS2, eMFC_FAR_SINGLE, 0);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_FAR_RUN:
                dvMotorGecCmd(eCLI_LSM_FOCUS2, eMFC_FAR_RUN, 0);
                break;

            case eZOOMLENS_CMD_BACK_FOCUS_RELAY_RESET: //H30K_Julie_0005
                dvMotorGecCmd(eCLI_LSM_FOCUS2, eMFC_RELAY_RESET, 0);
                break;
        #endif

            default:
                break;
        }
    }

    return;
}

void halMotorFocusSetWithDuration(eZOOMLENS_CMD eFocusMoving, UINT16 uiDuration)//A65_OPTOMA_CL_0011
{
    if(MotorController == eMOTOR_CONTROLLER_FMT54605 || MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        UINT8 ucPeriod[2];

        if(uiDuration <35)  //minimum value 35 ms
            uiDuration = 35;

        ucPeriod[0] = uiDuration & 0xff;
        ucPeriod[1] = (uiDuration>>8) & 0xff;

        switch(eFocusMoving)
        {
            case eZOOMLENS_CMD_FOCUS_NEAR_STEP_DURATION:
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_NEAR_STEP_DURATION, 2, ucPeriod);
                break;

            case eZOOMLENS_CMD_FOCUS_FAR_STEP_DURATION:
                dvMotorRegWrite(eMOTOR_MSG_FOCUS_FAR_STEP_DURATION, 2, ucPeriod);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        //TBD
    }

    return;
}

//A65_OPTOMA_CL_0011
void halMotorFocusDurationGet(UINT8 *pcData)
{
    if(MotorController == eMOTOR_CONTROLLER_FMT54605 || MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        UINT16 uiData = 0;
        eRESULT eResult = rcERROR;

        eResult = dvMotorRegRead(eMOTOR_MSG_FOCUS_PREV_STEP_DURATION, 2, pcData);
        if(eResult == rcERROR)
        {
            uiData = 0xFFFF;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        //TBD
    }
}

//A65_OPTOMA_CL_0008
UINT8 halMotorFocusGet(void)
{
    if(MotorController == eMOTOR_CONTROLLER_FMT54605 || MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        UINT8 cData = 0xF0;
        eRESULT eResult = rcERROR;

        eResult = dvMotorRegRead(eMOTOR_MSG_FOCUS_PROC_STATE, 1, &cData);
        if(eResult == rcERROR)
        {
            cData = 0xFF;
        }

        return cData;
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        UINT8 cData = 0xF0;
        //TBD
        return cData;
    }
}

void halMotorZoomSet(eZOOMLENS_CMD eZoomMoving)
{
    if(MotorController == eMOTOR_CONTROLLER_SYS54605)
    {
        switch(eZoomMoving)
        {
            case eZOOMLENS_CMD_ZOOM_IN_STEP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_ZOOM_IN_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_ZOOM_IN_RUN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_ZOOM_IN_RUN, 0, NULL);
                break;

            case eZOOMLENS_CMD_ZOOM_OUT_STEP:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_ZOOM_OUT_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_ZOOM_OUT_RUN:
                dvMCUDriverRegWrite(eCMD_MODULE_MOTOR, eMOTOR_MSG_ZOOM_OUT_RUN, 0, NULL);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_FMT54605 ||
            MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        switch(eZoomMoving)
        {
            case eZOOMLENS_CMD_ZOOM_IN_STEP:
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_IN_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_ZOOM_IN_RUN:
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_IN_RUN, 0, NULL);
                break;

            case eZOOMLENS_CMD_ZOOM_OUT_STEP:
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_STEP, 0, NULL);
                break;

            case eZOOMLENS_CMD_ZOOM_OUT_RUN:
                dvMotorRegWrite(eMOTOR_MSG_ZOOM_OUT_RUN, 0, NULL);
                break;

            default:
                break;
        }
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        switch(eZoomMoving)
        {
            case eZOOMLENS_CMD_ZOOM_IN_STEP:
                dvMotorGecCmd(eCLI_LSM_ZOOM, eMZC_ZOOM_IN_SINGLE, 0);
                break;

            case eZOOMLENS_CMD_ZOOM_IN_RUN:
                dvMotorGecCmd(eCLI_LSM_ZOOM, eMZC_ZOOM_IN_RUN, 0);
                break;

            case eZOOMLENS_CMD_ZOOM_OUT_STEP:
                dvMotorGecCmd(eCLI_LSM_ZOOM, eMZC_ZOOM_OUT_SINGLE, 0);
                break;

            case eZOOMLENS_CMD_ZOOM_OUT_RUN:
                dvMotorGecCmd(eCLI_LSM_ZOOM, eMZC_ZOOM_OUT_RUN, 0);
                break;

            default:
                break;
        }
    }
    return;
}


WORD halMotorLensCalibrationStatusGet(void) //A70LV_Larry_0129 modify //G100_Larry_0040
{
    UINT16 wData = 0xFFFF;
    eRESULT eResult = rcERROR;

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        eResult = rcSUCCESS;
        wData = wLSM_CalibrationStatus;
    }
    else
#endif
    if(MotorController == eMOTOR_CONTROLLER_SYS54605)
    {
        UINT8 cData = 0xFF;
        eResult = dvMCUDriverRegRead(eCMD_MODULE_MOTOR, eMOTOR_MSG_LENSHIFT_CAL, 1, &cData);

        wData = (UINT16)cData;
    }
    else if(MotorController == eMOTOR_CONTROLLER_FMT54605 ||
            MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        eResult = dvMotorRegRead(eMOTOR_MSG_LENSHIFT_CAL_STATE, 2, (UINT8 *)&wData); //motor borad M00.77
    }

    if(eResult == rcERROR)
    {
        wData = 0xFFFF;
    }

    return wData;
}

void halMotorLensGoCenter(void)
{
#ifdef PROJECTOR_2K

#else
	  UINT8 cData = (UINT8)eLENS_CMDS_CALIBRAION;
    dvMCUEventWrite(eCMD_MODULE_LENS, eMOTOR_EVENT_LENS, (UINT16)sizeof(UINT8), (UINT8*)&cData, TRUE);
#endif /* PROJECTOR_2K */

    return;
}

UINT8 halMotorLensMovingStateGet(void)    //G100_Owen_0054
{
    if(MotorController == eMOTOR_CONTROLLER_FMT54605 || MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        UINT8 cData = 0xFF;
        eRESULT eResult = rcERROR;

        eResult = dvMotorRegRead(eMOTOR_MSG_LENSHIFT_MOVING, 1, &cData);
        if(eResult == rcERROR)
        {
            cData = 0xFF;
        }

        return cData;
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        UINT8 cData = 0xF0;
        //TBD
        return cData;
    }
}

UINT8 halMotorLensMovingStartGet(void)
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
#ifdef PROJECTOR_2K

#else
    //dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_LENSMOVING, (UINT16)sizeof(UINT8), (UINT8*)&cData);
#endif /* PROJECTOR_2K */
    return cData;
}

//A70_Larry_0257 start
void halMotorLensPositionGet(sLENSMEMORY *psLensmemory)
{
    if(MotorController == eMOTOR_CONTROLLER_FMT54605 || MotorController == eMOTOR_CONTROLLER_MOTOR54113)
    {
        dvMotorRegRead(eMOTOR_MSG_LENS_POSITION, (UINT16)(sizeof(sLENSMEMORY)/sizeof(UINT8)), (UINT8*)psLensmemory); //A70LV_Larry_0151
    }
    else if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        //TBD
    }
}

void halMotorLensMemorySet(sLENSMEMORY *psLensmemory)
{
#ifdef PROJECTOR_2K
#else
    dvMCUEventWrite(eCMD_MODULE_LENS, eMOTOR_EVENT_LENS_MEMORY, (UINT16)(sizeof(sLENSMEMORY)/sizeof(UINT8)), (UINT8*)psLensmemory, TRUE);
#endif /* PROJECTOR_2K */
}

BOOL halMotorLensIdleGet(void)
{
    BOOL bFlag = TRUE;
#ifdef PROJECTOR_2K
#else
    dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_LENS_IDLE, (UINT16)sizeof(UINT8), (UINT8*)&bFlag);
#endif /* PROJECTOR_2K */
    return bFlag;
}

void halMotor_Focue_Limit_Get(sMOTOR_LIMIT *psMotor_limit)
{
#ifdef PROJECTOR_2K
#else
    dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_FOCUS_LIMIT, (UINT16)(sizeof(sMOTOR_LIMIT)/sizeof(UINT8)), (UINT8*)psMotor_limit);
#endif /* PROJECTOR_2K */
}
void halMotor_Zoom_Limit_Get(sMOTOR_LIMIT *psMotor_limit)
{
#ifdef PROJECTOR_2K
#else
    dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_ZOOM_LIMIT, (UINT16)(sizeof(sMOTOR_LIMIT)/sizeof(UINT8)), (UINT8*)psMotor_limit);
#endif /* PROJECTOR_2K */
}

void halMotor_Lens_Backlash_Get(sMOTOR_LIMIT *psMotor_limit) //A70_Larry_0357
{

#if defined(PLATFORM_H30_4K) //HICC2_Sammy_0003
	return;
#endif

#ifdef PROJECTOR_2K
    dvMotorRegRead(eMOTOR_MSG_LENS_BACKLASH, 4, (UINT8*)psMotor_limit); //A70LV_Larry_0277
#else
    dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_LENS_BACKLASH, (UINT16)(sizeof(sMOTOR_LIMIT)/sizeof(UINT8)), (UINT8*)psMotor_limit);
#endif /* PROJECTOR_2K */
}

UINT8 halMotor_Lens_Encode_Get(void)
{
    UINT8 cData = 0;
#ifdef PROJECTOR_2K
#else
    dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_ENCODE, (UINT16)(sizeof(UINT8)/sizeof(UINT8)), (UINT8*)&cData);
#endif /* PROJECTOR_2K */
    m_cLensEncode = (BOOL)((!cData)&0x01); //A70LH_Larry_0047
    return cData;
}

BOOL halMoter_LensEncodeSaveGet(void) //A70LH_Larry_0047
{
    return m_cLensEncode;
}
void halMotorLensMemorySaveSet(UINT8 cLensMemoryID)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        dvMotorGecCmd(eCLI_LSM_MEMORY, eMLC_LSM_MEMORY_SAVE, cLensMemoryID);
    }
    else
#endif
    dvMotorRegWrite(eMOTOR_MSG_LENS_MEM_SAVE, 1, &cLensMemoryID); //A70LV_Larry_0151
}

UINT8 halMotorLensMemoryApplyGet(void)
{
    UINT8 cData = 0;
#ifdef PROJECTOR_2K
#else
    dvMCUEventRead(eCMD_MODULE_LENS, eMOTOR_EVENT_LENS_MEMORY_APPLY, 1, (UINT8*)&cData);
#endif /* PROJECTOR_2K */
    return cData;
}

void halMotorLensMemoryApplySet(UINT8 cLensMemoryID)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    if(MotorController == eMOTOR_CONTROLLER_GEC_CMD)
    {
        dvMotorGecCmd(eCLI_LSM_MEMORY, eMLC_LSM_MEMORY_APPLY, cLensMemoryID);
    }
    else
#endif
    dvMotorRegWrite(eMOTOR_MSG_LENS_MEM_APPLY, 1, &cLensMemoryID); //A70LV_Larry_0151
}

void halMotorLensA16MotorGoCenter(void)
{
    return;
}

void halMotorLensSelectSet(UINT8 cLens)
{
#ifdef PROJECTOR_2K
#else
	  UINT8 cData;

    if(cLens == eA16_LENS)
        cData = eLENS_CMDS_A16_RANGE;
    else
        cData = eLENS_CMDS_OLD_RANGE;

    dvMCUEventWrite(eCMD_MODULE_LENS, eMOTOR_EVENT_LENS , (UINT16)sizeof(UINT8), (UINT8*)&cData, TRUE);
#endif /* PROJECTOR_2K */
}

eRESULT halMotor_Version_Get(UINT8* pcData) // A70LV_Eric.C_0024
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    return rcERROR;
#else
    return dvMotorRegRead(eMOTOR_MSG_VERSION, 2, pcData);
#endif
}

eRESULT halMotor_LENS_EEPROM_Version_Get(UINT8* pcData)  //HICC2_Steven_0027
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    return rcERROR;
#else
    return dvMotorRegRead(eMOTOR_MSG_LENS_EEPROM_VERSION, 2, pcData);
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensModule_Get
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
eRESULT halMotor_LensModule_Get(UINT8* pcData)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    pcData[0] = halMotor_LSM_Lens_Module_Get();
    return rcSUCCESS;
#else
    return dvMotorRegRead(eMOTOR_MSG_LENSHIFT_MODULE, 1, pcData);
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensModule_Set
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
eRESULT halMotor_LensModule_Set(UINT8* pcData)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    //TBD
    return rcERROR;
#else
    return dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_MODULE, 1, pcData);
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_OEJig_Set
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
eRESULT halMotor_OEJig_Set(UINT8 ucEnable)
{
    return dvMotorRegWrite(eMOTOR_MSG_LENSHIFT_OEJIG, 1, &ucEnable);
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensMemoryRecord_Get
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
eRESULT halMotor_LensMemoryRecord_Get(UINT8* pcData)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    pcData[0] = halMotor_LSM_Memory_Get(); //HICC2_AC_0019
    return rcSUCCESS;
#else
    return dvMotorRegRead(eMOTOR_MSG_LENS_MEM_RECORD, 1, pcData);
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensMemoryReset
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
eRESULT halMotor_LensMemoryReset(void)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    dvMotorGecCmd(eCLI_LSM_MEMORY, eMLC_LSM_MEMORY_CLEAR, 0); //HICC2_AC_0019
    return rcSUCCESS;
#else

#if 0
    typedef enum
    {
        eNVRAM_NODE_EVENT_INITIALIZE,   // Standard reset (not calibration nodes)
        eNVRAM_NODE_EVENT_RESETALL,     // Complete reset including calibration nodes

        eNVRAM_NODE_EVENT_NUMBERS,      // used for enumerated type range checking (DO NOT REMOVE)
    } eNVRAM_NODE_EVENT;
#endif /* 0 */
    UINT8 ucResetType = 1;

    return dvMotorRegWrite(eMOTOR_MSG_LENS_MEM_RESET, 1, &ucResetType);  //A35G2_Larry_0069
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensMemoryStatus_Get
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
UINT8 halMotor_LensMemoryStatus_Get(void)
{
    UINT8 cData = 0xFF;
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    cData = halMotor_LSM_CalibrationDone_Get(); //HICC2_AC_0017
    return cData;
#else
    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_MEM_STATUS, 1, &cData);

    if(eResult == rcERROR)
    {
        cData = 0xFF;
    }

    return cData;
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensId_Get
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
UINT8 halMotor_LensId_Get(void)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    UINT8 cData = halMotor_LSM_Lens_ID_Get(); //HICC2_AC_0010
#else
    UINT8 cData = 0xFF;
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_ID, 1, &cData);

    if(eResult == rcERROR)
    {
        cData = 0xFF;
    }
#endif
    return cData;
}

// ==============================================================================
// FUNCTION NAME: halMotor_ZoomLimit_Get
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
eRESULT halMotor_ZoomLimit_Get(sMOTOR_LIMIT *psLimit)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    return eResult;
#else
    eResult = dvMotorRegRead(eMOTOR_MSG_ZOOM_LIMIT, 4, (UINT8*)psLimit);
    return eResult;
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_FocusLimit_Get
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
eRESULT halMotor_FocusLimit_Get(sMOTOR_LIMIT *psLimit)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    return eResult;
#else
    eResult = dvMotorRegRead(eMOTOR_MSG_FOCUS_LIMIT, 4, (UINT8*)psLimit);
    return eResult;
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensPlug_Get
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
UINT8 halMotor_LensPlug_Get(void)
{
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
    UINT8 cData = halMotor_LSM_Lens_Plug_Get(); //HICC2_AC_0021

    return cData;
#else
    eRESULT eResult = rcERROR;
    UINT8 cData = 0;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_PLUG, 1, &cData);

    if(eResult == rcSUCCESS)
    {
        return cData;
    }

    return 0;
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensFujiDet_Get
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
UINT8 halMotor_LensFujiDet_Get(void)
{
    eRESULT eResult = rcERROR;
    UINT8 cData = 0;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_FUJI_DET, 1, &cData);

    if(eResult == rcSUCCESS)
    {
        return cData;
    }

    return 0;
}

eRESULT halMotor_LensIdADC_Get(UINT32 *psIDAdc) //G100_Owen_0055
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    return eResult;
#else
    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_ID_ADC, 4, (UINT8*)psIDAdc);

    return eResult;
#endif
}

// ==============================================================================
// FUNCTION NAME: halMotor_WR_Test
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
UINT8 halMotor_WR_Test(void)    //G100_Owen_0035
{
    eRESULT eResult = rcERROR;
    UINT8 ucDataW = 0x5A, ucDataR = 0;
    static UINT32 sulErrCnt = 0;

    dvMotorRegWrite(eMOTOR_MSG_TEST, 1, &ucDataW);

    dvMotorRegRead(eMOTOR_MSG_TEST, 1, &ucDataR);

    if(ucDataR == ucDataW)
    {
        eResult = rcSUCCESS;
    }
    else
    {
        LOG_MSG(db_ALWAYS, "(func:%s, line:%d) ucDataR(%d) ucDataW(%d) ErrCnt(%d)\r\n", __FUNCTION__, __LINE__, ucDataR, ucDataW, ++sulErrCnt);
    }

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensCenterEnable_Get
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
eRESULT halMotor_LensCenterEnable_Get(UINT8 *pucEnable)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_OPTICAL_CENTER_ENABLE, 1, pucEnable);

    return eResult;
}
eRESULT halMotor_LensCenterEnable_Set(UINT8 ucEnable)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegWrite(eMOTOR_MSG_LENS_OPTICAL_CENTER_ENABLE, 1, &ucEnable);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensCenterApply_Set
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
eRESULT halMotor_LensCenterApply_Set(void)
{
    eRESULT eResult = rcERROR;
    UINT8 ucData = 0;

    eResult = dvMotorRegWrite(eMOTOR_MSG_LENS_OPTICAL_CENTER_APPLY, 0, &ucData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensCalDone_Get
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
eRESULT halMotor_LensCalDone_Get(UINT8 *pucLensCalFlg)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_CAL_DONE, 1, pucLensCalFlg);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensCenterSetting
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
eRESULT halMotor_LensCenterSettingSet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    if(pcData[0] == 1)
    {
        dvMotorGecCmd(eCLI_LSM_CENTER, eMLC_LSM_CENTER_SET, 0);
    }
    else
    {
        dvMotorGecCmd(eCLI_LSM_CENTER, eMLC_LSM_CENTER_CLEAR, 0);
    }
    eResult = rcSUCCESS;
#else
    eResult = dvMotorRegWrite(eMOTOR_MSG_LENS_CENTER_CONFIG, 1, pcData);
#endif
    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensCenterSetting
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
eRESULT halMotor_LensCenterSettingGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    pcData[0] = halMotor_LSM_Center_Config_Get(); //HICC2_AC_0018
    eResult = rcSUCCESS;
#else
    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_CENTER_CONFIG, 1, pcData);
#endif
    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_ZoomDutySet
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
eRESULT halMotor_ZoomDutySet(BYTE cMax, BYTE cMin)
{
    eRESULT eResult = rcERROR;
    BYTE acData[2] = {0};

    acData[0] = cMax;
    acData[1] = cMin;

    eResult = dvMotorRegWrite(eMOTOR_MSG_ZOOM_MAX_DUTY, 2, acData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_FocusDutySet
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
eRESULT halMotor_FocusDutySet(BYTE cMax, BYTE cMin)
{
    eRESULT eResult = rcERROR;
    BYTE acData[2] = {0};

    acData[0] = cMax;
    acData[1] = cMin;

    eResult = dvMotorRegWrite(eMOTOR_MSG_FOCUS_MAX_DUTY, 2, acData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensCenterGet
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
eRESULT halMotor_LensCenterGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_H30_4K) //HICC2_Sammy_0003
	eResult = rcSUCCESS;
#else
    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_CENTER_POSITION, 8, pcData);
#endif

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensPositionFullGet
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
eRESULT halMotor_LensPositionFullGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_FULL_RANGE, sizeof(sLENS_LIMIT), pcData);

    return eResult;
}

// ==============================================================================
// FUNCTION NAME: halMotor_LensPositionActiveGet
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
eRESULT halMotor_LensPositionActiveGet(UINT8 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_ACTIVE_RANGE, sizeof(sLENS_LIMIT), pcData);

    return eResult;
}

eRESULT halMotor_Lens_Speed_Set(UINT8* pcData)
{
    eRESULT eResult = rcERROR;

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0010
    //TBD
	eResult = rcSUCCESS;
#else
    eResult = dvMotorRegWrite(eMOTOR_MSG_LENS_SPEED, 1, pcData);
#endif
    return eResult;
}

eRESULT halMotor_Lens_Speed_Control_Set(UINT8* pcData)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegWrite(eMOTOR_MSG_LENS_SPEED_CONTROL, 2, pcData);

    return eResult;
}

eRESULT halMotor_Lens_Speed_Control_Get(UINT16 *pcData)
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_SPEED_CONTROL, 2, (UINT8*)pcData);

    return eResult;
}

eRESULT halMotor_FujiLens_BackFocus_Set(UINT8* pcData) //H30K_Julie_0005
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegWrite(eMOTOR_CMD_BACK_FOCUS_ENABLE, 1, pcData);

    return eResult;
}

eRESULT halMotor_FujiLens_BackFocus_Get(UINT16 *pcData) //HICC2_Julie_0070//H30K_Julie_0005
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_CMD_BACK_FOCUS_ENABLE, 1, (UINT8*)pcData);

    return eResult;
}

UINT8 halMotor_LensShiftStatus_Get(void) //HICC2_Casper_0041
{
    UINT8 wData = 0;
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_MSG_LENS_SHIFT_STAUTS, 1, (UINT8 *)&wData);

    if(eResult == rcERROR)
    {
        wData = 0;
    }

    return wData;
}

eRESULT halMotor_Lens_Header_Get(UINT8 cSize, UINT8 *pcData)//HICC2_Julie_0061
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegRead(eMOTOR_CMD_LENS_HEADER_INFO, (UINT16)cSize, pcData);

    return eResult;
}

eRESULT halMotor_Lens_Header_Index_Set(UINT8 pcData)//HICC2_Julie_0061
{
    eRESULT eResult = rcERROR;

    eResult = dvMotorRegWrite(eMOTOR_CMD_LENS_HEADER_INDEX, 1, &pcData);

    return eResult;
}

BYTE halMotor_Lens_Header_Param_Size(UINT8 cIndex)//HICC2_Julie_0061
{
    BYTE cSize = 1;
    if(cIndex <= eLENS_HEADER_CUSTOMER_SN)
    {
        cSize = MOTOR_SN_NUMBER;
    }
    else if((cIndex >= eLENS_HEADER_NVRAM_ID) && (cIndex <= eLENS_HEADER_LIMIT_FOCUS2_NEAR))
    {
        cSize = sizeof(INT32);
    }
    return cSize;
}

UINT8 halMotor_Lsm_Sensor_Mute_H_Get(void)
{
    return wLSM_Sensor_Mute_H;
}

eRESULT halMotor_Lsm_Sensor_Mute_H_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)

    wLSM_Sensor_Mute_H = pcData;
    if(pcData == 0)
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_SENSOR_ACTIVE_H, 0);
    }
    else
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_SENSOR_MUTE_H, 0);
    }
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

UINT8 halMotor_Lsm_Sensor_Mute_V_Get(void)
{
    return wLSM_Sensor_Mute_V;
}

eRESULT halMotor_Lsm_Sensor_Mute_V_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)

    wLSM_Sensor_Mute_V = pcData;
    if(pcData == 0)
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_SENSOR_ACTIVE_V, 0);
    }
    else
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_SENSOR_MUTE_V, 0);
    }
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

UINT8 halMotor_Lsm_Boundary_Mute_H_Get(void)
{
    return wLSM_Boundary_Mute_H;
}

eRESULT halMotor_Lsm_Boundary_Mute_H_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)

    wLSM_Boundary_Mute_H = pcData;
    if(pcData == 0)
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_BOUNDARY_ACTIVE_H, 0);
    }
    else
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_BOUNDARY_MUTE_H, 0);
    }
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

UINT8 halMotor_Lsm_Boundary_Mute_V_Get(void)
{
    return wLSM_Boundary_Mute_V;
}

eRESULT halMotor_Lsm_Boundary_Mute_V_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)

    wLSM_Boundary_Mute_V = pcData;
    if(pcData == 0)
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_BOUNDARY_ACTIVE_V, 0);
    }
    else
    {
        dvMotorGecCmd(eCLI_LSM_MUTE, eMLC_LSM_BOUNDARY_MUTE_V, 0);
    }
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

eRESULT halMotor_Lsm_Boundary_Calibration_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_BOUNDARY_CAL_DONE, pcData);
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

eRESULT halMotor_Lsm_V_Up_Boundary_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_V_UP_BOUNDARY, pcData);
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

eRESULT halMotor_Lsm_V_Down_Boundary_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_V_DOWN_BOUNDARY, pcData);
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

eRESULT halMotor_Lsm_H_Left_Boundary_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_H_LEFT_BOUNDARY, pcData);
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

eRESULT halMotor_Lsm_H_Right_Boundary_Set(UINT8 pcData)
{
    eRESULT eResult = rcERROR;
#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) || defined (PLATFORM_H30_FPGA_4K)
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_H_RIGHT_BOUNDARY, pcData);
    eResult = rcSUCCESS;
#else

#endif
    return eResult;
}

#if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002

void halMotor_LSM_Version_Get(void)
{
    dvMotorGecCmd(eCLI_GEC_VERSION_R, eMFC_FW_VERIOSN_LSM, 0);
}

void halMotor_LSM_Center_Get(void)
{
    dvMotorGecCmd(eCLI_LSM_CENTER, eMLC_LSM_CENTER_GET, 0);
}

void halMotor_LSM_Center_Set(void)
{
    dvMotorGecCmd(eCLI_LSM_CENTER, eMLC_LSM_CENTER_SET, 0);
}

void halMotor_LSM_Center_Clear(void)
{
    dvMotorGecCmd(eCLI_LSM_CENTER, eMLC_LSM_CENTER_CLEAR, 0);
}

void halMotor_LSM_Status_Ask(void)
{
    dvMotorGecCmd(eCLI_LSM_STATUS, eMLC_LSM_STATUS_QUERY, 0);
}

void halMotor_LSM_CalibrationDone_Ask(void)
{
    dvMotorGecCmd(eCLI_LSM_CALIBATION, eMLC_LSM_CALIBRATION_DONE_ASK, 0);
}

void halMotor_LSM_CalibrationStatus_Set(UINT16 wData)
{
    wLSM_CalibrationStatus = wData;
}

UINT8 halMotor_LSM_CalibrationStatus_Get(void)
{
    return wLSM_CalibrationStatus;
}

void halMotor_LSM_CalibrationDone_Set(UINT8 wData)
{
    wLSM_CalibrationDone = wData;
}

UINT8 halMotor_LSM_CalibrationDone_Get(void)
{
    return wLSM_CalibrationDone;
}

void halMotor_LSM_Lens_ID_Ask(void)
{
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_SUBID_LENS_ID, 0);
}

void halMotor_LSM_Lens_Module_Set(UINT8 uData) //HICC2_AC_0010
{
    uLSM_Lens_Module = uData;
}

UINT8 halMotor_LSM_Lens_Module_Get(void)
{
    return uLSM_Lens_Module;
}

void halMotor_LSM_Lens_ID_Set(UINT8 uData)
{
    uLSM_Lens_ID = uData;
}

UINT8 halMotor_LSM_Lens_ID_Get(void)
{
    return uLSM_Lens_ID;
}

void halMotor_LSM_Factory_Menu_Lens(void)
{
    dvMotorGecCmd(eCLI_LSM_FACTORY_MENU, eMLC_LSM_FACTORY_MENU_LENS, 0);
}

void halMotor_LSM_Factory_Menu_ZoomFocus(void)
{
    dvMotorGecCmd(eCLI_LSM_FACTORY_MENU, eMLC_LSM_FACTORY_MENU_ZOOMFOCUS, 0);
}

void halMotor_LSM_Center_Config_Set(UINT8 uData) //HICC2_AC_0018
{
    uLSM_Lens_Center_Config = uData;
}

UINT8 halMotor_LSM_Center_Config_Get(void)
{
    return uLSM_Lens_Center_Config;
}

void halMotor_LSM_Memory_Set(UINT8 uData) //HICC2_AC_0019
{
    uLSM_Lens_Memory_Record = uData;
}

UINT8 halMotor_LSM_Memory_Get(void)
{
    return uLSM_Lens_Memory_Record;
}

void halMotor_LSM_Memory_Query(void)
{
    dvMotorGecCmd(eCLI_LSM_MEMORY, eMLC_LSM_MEMORY_GET, 0);
}

void halMotor_LSM_Memory_Clear(void)
{
    dvMotorGecCmd(eCLI_LSM_MEMORY, eMLC_LSM_MEMORY_CLEAR, 0);
}

void halMotor_LSM_Lens_Plug_Query(void)//DET1	//HICC2_Zonic_0009
{
	dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_LENS_DETECTION, 0);
}

void halMotor_LSM_Lens_Lock_Query(void)//DET2 //H30K_Julie_0005
{
	dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_LENS_DETECTION, 2);
}

void halMotor_LSM_Lens_Detection_Bypass(BOOL bBypass)
{
	if(bBypass == TRUE)
		dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_LENS_DETECTION_BYPASS, 1);
	else
		dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_LENS_DETECTION_BYPASS, 2);
}

void halMotor_LSM_Lens_Detection_Bypass_Query(void)
{	
	dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_LENS_DETECTION_BYPASS, 0);
}


void halMotor_LSM_Lens_Plug_Set(UINT8 uData) //HICC2_AC_0021
{
    uLSM_Lens_Plug = uData;
}

UINT8 halMotor_LSM_Lens_Plug_Get(void)
{
    return uLSM_Lens_Plug;
}

void halMotor_LSM_Zoom_Duration_Set(void)
{
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_ZOOM_DURATION, 0);
}

void halMotor_LSM_Focus_Duration_Set(void)
{
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_FOCUS_DURATION, 0);
}

void halMotor_LSM_BackFocus_Duration_Set(void)
{
    dvMotorGecCmd(eCLI_LSM_CFG, eMLC_LSM_CONFIG_FOCUS2_DURATION, 0);
}

void halMotor_LSM_NVRAM_Status_Set(UINT8 uVal)
{
    uLSM_Lens_NVRAM_Status = uVal;
}

UINT8 halMotor_LSM_NVRAM_Status_Get(void)
{
    return uLSM_Lens_NVRAM_Status;
}

#endif


