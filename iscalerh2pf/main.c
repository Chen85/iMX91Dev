//#include "TypeDefine.h"
#include "type_def.h"
#include "hicc_config.h"
#include "hpbulib_ver.h"

//#include <stdio.h>
//#include <pthread.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>
//#include <unistd.h>
//#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
//#include <netinet/in.h>
//#include <netinet/tcp.h>
//#include "DataCtrlAPI.h"


#include "Common.h"
#include "Release.h"

#if (SYSTEM_OS_TYPE == FREERTOS)
#include "./FreeRTOS_Kernel/include/FreeRTOS.h"
#include "./FreeRTOS_Kernel/include/task.h"
#include "./FreeRTOS_Kernel/include/queue.h"
#endif

#include "utilDbgMsg.h"

#ifdef SCALER_FPGA_F34
#include "dvProAV.h"
#include "dvProAV_Interface.h"
#include "dvProAV_Access.h"
#include "dvProAV_RegTable.h"
#include "dvProAV_Scaler.h"
#include "dvFrontEndDriver.h"
#endif

#include "appSystem.h"
#include "appDataPath.h"  //H2PF_Simon_0033
#include "halFrontEndCtrlAPI.h"
#include "halBoardCtrlAPI.h"
#include "halMotorCtrlAPI.h"

#include "Board_Uart.h"
#include "Board_SPI.h"
#include "Board_I2C.h"
#include "dvDDP442x.h"

#include "utilCLICmdAPI.h"
#include "ProcessMutexData.h"     //H2PF_Simon_0033
#include "ProjectSettings.h"

static UINT8 m_uc_BOARD_STAGE = SCALER_BOARD_STAGE;
static BOOL m_bSingleBoard = 0;
static UINT8 m_ucMCU_ModelID = 0;
static UINT8 m_ucModelID = 0;

void ProcMutexData_ScalerInfoInit(void);   //H2PF_Simon_0033

int main(int argc, char **argv)
{
    int opt;
    BaseType_t xReturned;

    if(utilDatabase_Init() == -1)
    {
        printf("Create Shared Memory Fail\n");
    }

    utilDbgMsg_Init(0);

	//SharedMem_WriterInit(eSB_UI_ITEM_INFO);   //move to custom layer
	//ItemConfig_CSV_File_LoadToShm();          //move to custom layer

    printf("==============================\n");

#ifdef SCALER_FPGA_F34
    printf("HICC2.0 Platform FPGA Layer Start\n");
#elif defined(C821_C789)
    printf("HICC2.0 Platform iChipC821 Layer Start\n");
#elif defined(C341)
    printf("HICC2.0 Platform iChipC341 Layer Start\n");
#else
    printf("HICC2.0 Platform NA Layer Start\n");
#endif /* SCALER_FPGA_F34 */

#ifdef CUSTOM_CHRISTIE
        printf("HICC2.0 Custom Christie Layer Start\n");
#else
        printf("HICC2.0 Custom NA Layer Start\n");
#endif

    printf("iScaler HICC2.0 V%02d.%02d\n",VER_MAJOR, VER_MINOR);
    printf("Hpbulib V%02d.%02d\n",HPBULIB_VER_MAJOR, HPBULIB_VER_MINOR);
    printf("==============================\n");

	InitSystemCfg();
    InitRs232CmdCfg();
    InitOSDDafaultValueCmdCfg();
    utilProcMutexData_Init();    //H2PF_Simon_0033
    ProcMutexData_ScalerInfoInit();     //H2PF_Simon_0033

	GlobalSettings_Init();
    palSystem_SingleiScalerModeFlagSet(FALSE);

    opt = getopt(argc, argv, "ntsS");
    switch(opt)
    {
        case 't':
            m_uc_BOARD_STAGE = EVT_STAGE;
            printf("iScaler run test mode\n");
            break;

        case 's':
        case 'S':
            palSystem_SingleiScalerModeFlagSet(TRUE);
            printf("Single iScaler Mode (No UI)\n");
            break;

        case 'n':
        default:
            //m_uc_BOARD_STAGE = DVT_STAGE;
            printf("iScaler run normal mode\n");
            break;
    }
#ifdef APP_BACKGROUND
    Board_Uart_Init(uaPCUART);
#endif /* APP_BACKGROUND */


#ifndef SIMULATOR_ISCALER

    #ifdef SCALER_FPGA_F34
        #ifdef NO_INTERFACE
            dvProAV_InterfaceInit(eProAVIF_NA);
        #else
            dvProAV_InterfaceInit(eProAVIF_SPI);
        #endif /* NO_INTERFACE */
    #else
        Board_I2C_Init();
        Board_SPI_Init();
    #endif

        I2C_Config();
        halMotor_Init();

        dv442x_BusyPin_Init();
        halBoard_PowerKeyReset();
        halBoard_CameraEnableInit(); //G100_Larry_0029

#else
#ifdef SCALER_FPGA_F34
    dvProAV_InterfaceInit(eProAVIF_NA);
#endif /* SCALER_FPGA_F34 */

    m_uc_BOARD_STAGE = MOCKUP_STAGE;
#endif /* SIMULATOR_ISCALER */


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    if(mq_unlink(QUEUE_SYSTEM_NAME) == 0)
      LOG_MSG(db_APP_SYSTEM, "Message queue %s removed from system.\n", QUEUE_SYSTEM_NAME);

    if(mq_unlink(QUEUE_DATAPATH_NAME) == 0)
      LOG_MSG(db_APP_SYSTEM, "Message queue %s removed from system.\n", QUEUE_DATAPATH_NAME);

    if(mq_unlink(QUEUE_DATAPATHSUB_NAME) == 0)
      LOG_MSG(db_APP_SYSTEM, "Message queue %s removed from system.\n", QUEUE_DATAPATHSUB_NAME);

    if(mq_unlink(QUEUE_CLI_NAME) == 0)
      LOG_MSG(db_APP_SYSTEM, "Message queue %s removed from system.\n", QUEUE_CLI_NAME);

    if(mq_unlink(QUEUE_POLLING_NAME) == 0)
      LOG_MSG(db_APP_SYSTEM, "Message queue %s removed from system.\n", QUEUE_POLLING_NAME);

    if(mq_unlink(QUEUE_HOST_NAME) == 0)
       LOG_MSG(db_APP_SYSTEM, "Message queue %s removed from system.\n", QUEUE_HOST_NAME);


    //Create Thread
    pthread_attr_init(palSystem_GetTaskAttr());
    pthread_attr_setstacksize(palSystem_GetTaskAttr(), APP_SYSMON_STACK_SIZE);
    INT16 iCreateThreadError = 0 ;
    iCreateThreadError = pthread_create(palSystem_GetTaskHandle(), palSystem_GetTaskAttr(), palSystem_SysMonTask, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return 0;
    }

    LOG_MSG(db_APP_SYSTEM, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , palSystem_GetTaskHandle());

    pthread_join(*((pthread_t *)palSystem_GetTaskHandle()), NULL);
    LOG_MSG(db_ALWAYS, "\nFatal Error ! SYSMON Thread exit...\n");

#else

    xReturned = xTaskCreate(palSystem_SysMonTask,       /* The function that implements the task. */
                            "SYSMON",                   /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                            APP_SYSMON_STACK_SIZE,      /* The size of the stack to allocate to the task. */
                            NULL,                       /* The parameter passed to the task - not used in this simple case. */
                            APP_SYSMON_PRIORITY,        /* The priority assigned to the task. */
                            palSystem_GetTaskHandle()); /* The task handle is not required, so NULL is passed. */

    if(xReturned != pdPASS)
    {
        return 0;
    }


    /* Start the scheduler */
    vTaskStartScheduler();
#endif /* 0 */

    /* Should never arrive here */
    return 1;

}

BOOL Board_SingleBoard_Get(void)
{
#ifdef SIMULATOR_ISCALER
    return TRUE;
#else
    return m_bSingleBoard;
#endif /* SIMULATOR_ISCALER */
}

void Board_SingleBoard_Set(BOOL bValue)
{
    m_bSingleBoard = bValue;
}

void Board_MCU_ModelID_Set(UINT8 ucModelID)
{
    //#define MODULE_8K_PLATFORM          (0x81)  //Read from MCU
    //#define MODULE_9K_PLATFORM          (0x82)
    //#define MODULE_11K_PLATFORM         (0x83)

    m_ucMCU_ModelID = ucModelID;
}

UINT8 Board_MCU_ModelID_Get(void)
{
    //example, MCU 8k, 9K, 11K, fan and LD number
    //#define MODULE_8K_PLATFORM          (0x81)  //Read from MCU
    //#define MODULE_9K_PLATFORM          (0x82)
    //#define MODULE_11K_PLATFORM         (0x83)

    return m_ucMCU_ModelID;
}

void Board_ModelID_Set(UINT8 ucModelID)
{
    m_ucModelID = ucModelID;
}

UINT8 Board_ModelID_Get(void)
{
    //eMODEL_TYPE_4K13HS,    //MODEL_ID_0
    //eMODEL_TYPE_4K16HS,    //MODEL_ID_1
    //eMODEL_TYPE_4K22HS,    //MODEL_ID_2

    return m_ucModelID;
}

void ProcMutexData_ScalerInfoInit(void)  //H2PF_Simon_0033
{
#ifdef SCALER_FPGA_F34

    palDataPath_ProcMutexDataScalerInfoInit();

#endif
}
