#ifndef _COMMAND_SIZE_LIST_H_
#define _COMMAND_SIZE_LIST_H_

typedef enum
{
    eFE_MSG_IDLE_SZ                   = 1,
    eFE_MSG_VERSION_SZ                = 4,
    eFE_MSG_GO_BOOTLOADER_SZ          = 1,

    eFE_MSG_BL_VER_SZ                 = 2,
    eFE_MSG_APP_VER_SZ                = 4,

    eFE_MSG_FPGA_VIDEO_VER_SZ         = 3,    //A70LV_Doulas_0176
    eFE_MSG_HDBASET_VER_SZ            = 4,    // A70LV_Eric.C_0031

    // A70LV_Eric.C_0030 Start
    eFE_MSG_VIDEO_READY_SZ            = 1,
    eFE_MSG_VIDEO_TIMING_MAIN_SZ      = 14,   //A70LV_Doulas_0176 modify
    eFE_MSG_VIDEO_FORMAT_MAIN_SZ      = 12,   //A70LV_Doulas_0154 modify//A70LV_Doulas_0112
    eFE_MSG_VIDEO_TIMING_SUB_SZ       = 14,   //A70LV_Doulas_0176 modify
    eFE_MSG_VIDEO_FORMAT_SUB_SZ       = 12,   //A70LV_Doulas_0290 modify//A70LV_Doulas_0112
    // A70LV_Eric.C_0030 End

    eFE_MSG_VIDEO_INPUT_MAIN_SZ       = 1,    // A70LV_Eric.C_0032

    eFE_MSG_VIDEO_INPUT_SUB_SZ        = 1,    //A70LV_Doulas_0102
    eFE_MSG_PIPPBP_ENABLE_SZ          = 1,    //A70LV_Doulas_0102
    eFE_MSG_INPUT_SOURCE_DETECT_SZ    = 2,    //A70LV_Doulas_0102

    eFE_MSG_VGA_HTOTAL_SZ             = 2,    //A70LV_Doulas_0102
    eFE_MSG_VGA_PHASE_SZ              = 1,    //A70LV_Doulas_0102
    eFE_MSG_VGA_SYNC_THRESHOLD_SZ     = 1,    //A70LV_Doulas_0112
    eFE_MSG_PIXEL_CLOCK_SZ            = 4,    //A70LV_Doulas_0115

    eFE_MSG_ADC7604_GAIN_SZ           = 6,    //A70LV_Doulas_0124
    eFE_MSG_ADC7604_OFFSET_SZ         = 6,    //A70LV_Doulas_0124
    eFE_MSG_VGA_TIMING_SZ             = 6,    //A70LV_Doulas_0200
//    eFE_MSG_SERIALNUMBER_EDID_SZ      = (SN_LENGTH),   //A70LV_Doulas_0200
    eFE_MSG_EDID_UPGRADE_SZ           = (SN_LENGTH + 3),  //Must < 32

	eFE_MSG_EQ_MODE_HDMI1_SZ     		= 1,
	eFE_MSG_EQ_MODE_HDMI2_SZ     		= 1,
	eFE_MSG_EQ_MODE_DVI_SZ     		= 1,

	eFE_MSG_HOT_PLUG_ENABLE_SZ        = 1,	//G100_Doulas_0006
	eFE_MSG_FORCE_HOT_PLUG_SZ         = 1,    //G100_Doulas_0007

	eFE_MSG_BACKUP_INPUT_AUTO_SW_SZ   = 1,    //G100_Steven_0016
	eFE_MSG_BACKUP_INPUT_FIRST_SZ     = 1,    //G100_Steven_0016
	eFE_MSG_BACKUP_INPUT_SECOND_SZ    = 1,    //G100_Steven_0016

	eFE_MSG_HDMI_OUT_SZ               = 1,    //G100_Steven_0021

	eFE_MSG_BACKUP_INPUT_STATUS_SZ    = 1,   //G100_Steven_0028
    eFE_MSG_BACKUP_INPUT_VIDEO_SCALER_CHECK_SZ = 1,
	eFE_MSG_AUTO_HDMI_SWITCH_SZ       = 1,     //A35G2_BRC_Simon_0006
	eFE_MSG_CUSTOMIZE_EDID_SZ         = 257,   //A35G2_Simon_0091   //bit0:Source ID, bit1~bit257:EDID
	eFE_MSG_AVI_INFO_SZ               = 16,

	eFE_MSG_OPD_INFO_SZ =  76, //G100_Steven_0059

	eFE_MSG_BACKUP_INPUT_VIDEO_READY_SZ =1, //G100_Steven_0065

    eFE_MSG_FPGA_UPG_INIT_SZ          = 1,
    eFE_MSG_FPGA_UPG_DATA_SZ          = 1033,
    eFE_MSG_FPGA_UPG_DONE_SZ          = 1,
    eFE_MSG_FPGA_UPG_REBOOT_SZ        = 1,
    eFE_MSG_FPGA_UPG_MODE_SZ          = 1,
    eFE_MSG_MODEL_NAME_SZ             = 16,	//A35G2_CDS_Coda_0032

    eFE_MSG_CMD_AUDIO_VOL_SZ          = 1,
    eFE_MSG_AUDIO_MUTE_SZ             = 1,
    eFE_MSG_AUDIO_PLAY_SZ             = 3,

	eFE_MSG_MODEL_SWITCH_ENABLE_SZ    = 1,  //A65_OPTOMA_Julie_0065

	eFE_MSG_HDR_EOTF_STREAM_SZ		  = 1,
	eFE_MSG_HDR_LUMINANCE_INFO_SZ     = 10,  //H2PF_Simon_0193

    eFE_MSG_SZ_NUMBERS
} eFE_MSG_SZ;

typedef enum
{
	eSYSTEM_MSG_HOST_READY_SZ                  = 0,
	eSYSTEM_MSG_SYSTEM_VERSION_SZ              = 2,
	eSYSTEM_MSG_SYSTEM_POWER_OFF_SZ            = 0,
	eSYSTEM_MSG_DDP_PWR_SZ                     = 1,
	eSYSTEM_MSG_KEYPAD_VERSION_SZ              = 2,
	eSYSTEM_MSG_WHEELPROTECT_VERSION_SZ        = 2,
	eSYSTEM_MSG_DEBUG_HOTKEY_SZ                = 1,
	eSYSTEM_MSG_SYSTEM_COOLING_SZ              = 0,
	eSYSTEM_MSG_SYSTEM_STATE_SZ                = 1,
	eSYSTEM_MSG_GOTO_BOOTLOADER_SZ             = 0,
	eSYSTEM_MSG_TOTAL_PROJECTOR_HOURS_SZ       = 4,
	eSYSTEM_MSG_AC_PWR_ON_SZ                   = 1,
	eSYSTEM_MSG_AUTO_PWR_ON_SZ                 = 1,
	eSYSTEM_MSG_AUTO_PWR_ON_TIMER_SZ           = 1,
	eSYSTEM_MSG_FACTORY_RESET_MODE_SZ          = 1,
	eSYSTEM_MSG_SYSTEM_HOURS_SZ                = 4,
	eSYSTEM_MSG_COMMUNICATION_MODE_SZ          = 1,
	eSYSTEM_MSG_MCU_INIT_SZ                    = 0,
	eSYSTEM_MSG_OPFU_SZ                        = 0,
	eSYSTEM_MSG_UART_BAUDRATE_SZ               = 1,
	eSYSTEM_MSG_REMOTE_ID_SZ                   = 1,
	eSYSTEM_MSG_R12_TRIGGER_SZ                 = 1,
	eSYSTEM_MSG_DDP_UPGRADE_ENABLE_SZ          = 1,
	eSYSTEM_MSG_IR_TOP_ENABLE_SZ               = 1,
	eSYSTEM_MSG_IR_FORNT_ENABLE_SZ             = 1,
	eSYSTEM_MSG_IR_REAR_ENABLE_SZ			   = 1,
	eSYSTEM_MSG_IR_HD_ENABLE_SZ                = 1,
	eSYSTEM_MSG_AC_INPUT_DET_SZ                = 1,
	eSYSTEM_MSG_AC_SOCKET_DET_SZ               = 0,
	eSYSTEM_MSG_HDBASET_EN_SZ                  = 1,
	eSYSTEM_MSG_UPGRADE_LPC1113_SZ             = 0,
	eSYSTEM_MSG_RS232_SZ                       = 1,
	eSYSTEM_MSG_HDBASET_SZ                     = 0,
	eSYSTEM_MSG_KEYEVENT_SZ                    = 0,
	eSYSTEM_MSG_REMOTE_CODE_SZ                 = 0,
	eSYSTEM_MSG_SIGNAL_PWR_ON_ENABLE_SZ        = 0,
	eSYSTEM_MSG_TEMPERATURE_SZ                 = 0,
	eSYSTEM_MSG_ERROR_LOG_NOTIFY_SZ            = 0,
	eSYSTEM_MSG_FPGA1_VERSION_SZ               = 2,
	eSYSTEM_MSG_FPGA2_VERSION_SZ               = 3,
	eSYSTEM_MSG_FPGA3_VERSION_SZ               = 3,
	eSYSTEM_MSG_POWER_RESTART_SZ               = 0,
	eSYSTEM_MSG_UART_SWITCH_SZ                 = 0,
	eSYSTEM_MSG_3D_SYNC_OUT_SZ                 = 0,
	eSYSTEM_MSG_MODEL_ID_SZ                    = 1,
	eSYSTEM_MSG_HDBASET_VERSION_SZ             = 5,
	eSYSTEM_MSG_STANDBY_ERROR_FLAG_SZ          = 1,
	eSYSTEM_MSG_VGADET_SZ                      = 1,  //G100_Steven_0077
	eSYSTEM_MSG_TEC_TESTER_SZ                  = 10,    //G100_Steven_0080
    eSYSTEM_MSG_LAN_PATH_SZ                    = 1,
    eSYSTEM_MSG_PROJECTOR_ID_SZ                = 1,
    eSYSTEM_MSG_LAN_IP_INFO_SZ                 = 4,
	eSYSTEM_MSG_HW_VERSION_SZ                  = 2,
    eSYSTEM_MSG_SNID_SZ                        = 28,
	eSYSTEM_MSG_DMD_AIRTIGHT_STATUS_SZ         = 1, //HICC2_Steven_0031
	eSYSTEM_MSG_BURN_IN_PARAMETER_SZ		   = 3,
	eSYSTEM_MSG_PIN_PROTECT_SZ                 = 1,//H30K_David_0038
    eSYSTEM_MSG_PIN_CODE_SZ                    = 4,//H30K_David_0038
    eSYSTEM_MSG_PIN_POWER_ON_SZ                = 1,//H30K_David_0038
    eSYSTEM_MSG_OESN1_SZ                       = VERSION_STRING_MAX_LENGTH,     //H30K_Tim_0011, add
    eSYSTEM_MSG_OESN2_SZ                       = VERSION_STRING_MAX_LENGTH,     //H30K_Tim_0011, add
	eSYSTEM_MSG_SYSTEM_HDBT_STATUS_SZ          = 11,

    eSYSTEM_MSG_SZ_NUMBERS,
} eSYSTEM_MSG_SZ;

typedef enum
{
    eFMT_MSG_MCU_VERSION_SZ          = 2,					//G100_Clare_0033

    eFMT_MSG_LIGHT_MODULE_STATUS_SZ  = 1,
    eFMT_MSG_LD_BANK_ENABLE_SZ       = 4,
    eFMT_MSG_LD_VOLTAGE_SZ           = 3,
    eFMT_MSG_LD_CURRENT_SZ           = 3,
    eFMT_MSG_LD_TEMPERATURE_SZ       = 3,
    eFMT_MSG_LD_OVER_TEMPERATURE_SZ  = 2,
    eFMT_MSG_LD_PO_SZ                = 9,					//G100_Clare_0033
    eFMT_MSG_LD_PWM_OUT_SZ           = 9,
    eFMT_MSG_LIGHT_SOURCE_ENABLE_SZ  = 1,
    eFMT_MSG_DYNAMIC_BLACK_SYNC_SZ   = 8,               //LDDRV_John_0016 add DynamicBlack I2C route for PWM change
    eFMT_MSG_LD_PO_ALL_SZ            = 16,              //LDDRV_John_0019 Combine BLD and RLD PWM I2C sending command
    eFMT_MSG_LD_ENABLE_BY_SOURCE_SZ  = 1,               //ZU860_Doulas_0083  LD enable for source getting
    eFMT_MSG_LD_ENABLE_BY_WHEEL_SZ   = 1,               //ZU860_Doulas_0083  LD enable for wheel stable

    eFMT_MSG_TEC_VOLTAGE_SZ          = 3,
    eFMT_MSG_TEC_CURRENT_SZ          = 3,
    eFMT_MSG_TEC_TEMPERATURE_SZ      = 3,
    eFMT_MSG_TEC_OVER_TEMPERATURE_SZ = 2,
    eFMT_MSG_TEC_JIG_DATA_SZ		 = 20, //A70LV_John_0158 add TEC JIG communication commands
    eFMT_MSG_TEC_DUTY_SZ             = 2,

    eFMT_MSG_FAN_SPEED_SZ            = 3,
    eFMT_MSG_PRESS_HUMIDITY_SZ       = 4,	//G100_Clare_0016
    eFMT_MSG_FAN_LOCK_INDICATE_SZ    = 4,
    eFMT_MSG_FAN_DUTY_RPM_SZ         = 3,
    eFMT_MSG_FAN_DISPLAY_SZ          = 1,                 		//G100_Clare_0011
    eFMT_MSG_FAN_NO_FAN_CTRL_SZ      = 1,                 //LDDRV_JS_0036 add

    eFMT_MSG_LIQUID_SPEED_SZ         = 4,					//G100_Clare_0033

	eFMT_MSG_WHEEL_SPEED_SZ          = 4,
    eFMT_MSG_THERMAL_SENSOR_SZ       = 3,

    eFMT_MSG_ORIENTATION_SZ          = 1,
    eFMT_MSG_POWER_STATUS_SZ         = 1,
    eFMT_MSG_LIGHT_SENSOR_SZ         = 10,
    eFMT_MSG_G_SENSOR_DATA_SZ        = 6,                 //ZU860_Doulas_0059
    eFMT_MSG_LS_RLD_ENABLE_SZ        = 1,			  		//G100_Doulas_0010 for get light sensor using
    eFMT_MSG_LS_T1_SZ          	     = 16,				    //G100_Doulas_0012 light sensor T1 set
    eFMT_MSG_LS_T0_SZ          	     = 4,				    //G100_Doulas_0012 light sensor T0 set
    eFMT_MSG_LIGHT_SENSOR_T1_SZ      = 8,				//G100_Doulas_0012 light sensor value
	eFMT_MSG_LIGHT_SENSOR_T0_SZ      = 8,				//G100_Doulas_0012 light sensor value
	eFMT_MSG_LIGHT_SENSOR_TRIGGER_SZ = 1,					//G100_Doulas_0012 light sensor A2μo¯S§OAa‥u
	eFMT_MSG_LIGHT_SENSOR_POSITION_SZ = 1,              //HICC2_Jacky_0001

    eLDDRV_ABP_CONTROL_MODE_SZ         = 1,
    eLDDRV_ABP_TARGET_LIGHTSENSOR_BLD_SZ = 8,
    eLDDRV_ABP_DEFAULT_PWM_BLD_SZ        = 8,
    eLDDRV_ABP_TARGET_LIGHTSENSOR_RLD_SZ = 8,
    eLDDRV_ABP_DEFAULT_PWM_RLD_SZ        = 8,
    eLDDRV_ABP_AUTOTUNING_STATE_SZ       = 1,

    eFMT_MSG_LD_VOLTAGE_ALL_SZ       = 24,	//A65_Clare_0001
    eFMT_MSG_LD_CURRENT_ALL_SZ       = 24,					//G100_Clare_0033
    eFMT_MSG_LD_TEMPERATURE_ALL_SZ   = 24,					//G100_Clare_0033
    eFMT_MSG_TEC_VOLTAGE_ALL_SZ      = 6,						//G100_Clare_0033
    eFMT_MSG_TEC_CURRENT_ALL_SZ      = 6,						//G100_Clare_0033
    eFMT_MSG_TEC_TEMPERATURE_ALL_SZ  = 6,					//G100_Clare_0033
    eFMT_MSG_FAN_SPEED_ALL_SZ        = 32,					//G100_Clare_0033
    eFMT_MSG_FAN_DUTY_ALL_SZ         = 16,					//G100_Clare_0033
    eFMT_MSG_THERMAL_SENSOR_ALL_SZ   = 12,						//G100_Clare_0033

    eFMT_MSG_FAILURE_STATUS_SZ       = 3,
    eFMT_MSG_HIGH_ALTITUDE_SZ        = 1,

    eFMT_MSG_I2C_ERR_CNT_SZ		     = 5,	//A65_Clare_0003

    eFMT_MSG_FAN_SKU_RELOAD_SZ       = 1,
	eFMT_MSG_TEC_ABNORMAL_SZ         = 1, //A65_Steven_0009

	eFMT_MSG_CUSTOMER_SZ        	 = 1,
	eFMT_MSG_PLATFORM_SZ        	 = 1, //A70Gen2_Julie_0001

    eFMT_MSG_LD_CONT_ON_TIME_SZ      = 4,
    eFMT_MSG_CEILINGMOUNT_SZ         = 1,  //LDDRV_Larry_0006
    eFMT_MSG_SHUTDOWN_SZ             = 2,  //A70LV_Larry_0200
	eFMT_MSG_FILTER_TYPE_SZ        	 = 1, //A70Gen2_Julie_0001
    eFMT_MSG_FAN_DIMMING_MODE_SZ     = 1,

    eFMT_MSG_SZ_NUMBERS,
} eFMT_MSG_SZ; //A70LV_Larry_0078 //remane, reference from driver board

typedef enum
{
	eIAP_MSG_IDLE_SZ                      = 0,
	eIAP_MSG_VERSION_SZ                   = 0,
	eIAP_MSG_GO_BOOTLOADER_SZ             = 1,
	eIAP_MSG_BL_VER_SZ                    = 2,
	eIAP_MSG_APP_VER_SZ                   = 4,
	eIAP_MSG_RUN_APP_SZ                   = 1,
	eIAP_MSG_IAP_ENABLE_SZ                = 1,
	eIAP_MSG_APP_INFO_SZ                  = 8,
	eIAP_MSG_BIN_ADDRESS_SZ               = 4,
	eIAP_MSG_BIN_DATA_SZ                  = 0,
	eIAP_MSG_PROGRAMING_FINISH_SZ         = 0,
	eIAP_MSG_BIN_CHECK_SUM_SZ             = 4,
	eIAP_MSG_PROGRAMING_SZ                = 0,
	eIAP_MSG_APP_CODE_READBACK_SZ         = 4,
	eIAP_MSG_LOG_SZ                       = 0,
	eIAP_MSG_OPFU_ENABLE_SZ               = 1,
	eIAP_MSG_EXT_FLASH_SECTOR_SELECT_SZ   = 4,
	eIAP_MSG_EXT_FLASH_SECTOR_CHECKSUM_SZ = 4,
	eIAP_MSG_EXT_FLASH_SECTOR_CRC_SZ      = 2,
	eIAP_MSG_EXT_FLASH_SECTOR_ERASE_SZ    = 1,
	eIAP_MSG_EXT_FLASH_SECTOR_SIZE_SZ     = 4,
	eIAP_MSG_SECTOR_BIN_WRITE_SZ          = 0,
	eIAP_MSG_SECTOR_BIN_CHECKSUM_SZ       = 0,
	eIAP_MSG_SECTOR_BIN_CRC_SZ            = 0,
	eIAP_MSG_SECTOR_BIN_WRITE2FLASH_SZ    = 1,
	eIAP_MSG_UPGRADE_START_SZ             = 4,
	eIAP_MSG_UPGRADE_STATE_SZ             = 1,
	eIAP_MSG_EXT_UPGRADE_DONE_SZ          = 1,
	eIAP_MSG_MESSAGE_SZ                   = 0,
	eIAP_MSG_PERCENTAGE_SZ                = 1,

    eIAP_MSG_SZ_NUMBERS,
} eIAP_MSG_SZ;

typedef enum
{
	eOPD_MSG_I2C_STRESS_TEST_SZ                      = 0,
	eOPD_MSG_I2C_STRESS_STATE_SZ                     = 4,
	eOPD_MSG_I2C_STRESS_WORK_SZ                      = 4,

	eOPD_MSG_I2C_SLAVE_INFO_SZ                       = 1,
	eOPD_MSG_I2C_SLAVE_STATE_SZ                      = 4,
	eOPD_MSG_I2C_SLAVE_WORK_SZ                       = 4,

	eOPD_MSG_I2C_RUN_TIME_TOTAL_SZ                   = 4,
	eOPD_MSG_UART_STRESS_TEST_SZ                     = 1,
	eOPD_MSG_SPI_STRESS_TEST_SZ                      = 1,
	eOPD_MSG_EVENT_RESET_SZ                          = 1,

    eOPD_MSG_SZ_NUMBERS,
}eOPD_MSG_SZ;


#endif // _COMMAND_SIZE_LIST_H_

