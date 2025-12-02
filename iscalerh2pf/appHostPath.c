#include "appHostPath.h"
#include "appSystem.h"
#include "utilDbgMsg.h"
#include "Board_Uart.h"
#include "utilHostAPI.h"
#include <linux/input.h>
#include <termios.h>
//#include "utilHPBU_Tester.h"


static sPAL_HOSTPATH_INFORMATION m_sPalHostPathInfo;

static void* palHostPath_Task(void* pParameters)
{
    //char input = 255;

    Board_Uart_Init(uaMCU); //mcu
    Board_Uart_Init(uaMotor); //mcu

    utilHost_Init(eMSG_TYPE_UART_1);

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    MS_SLEEP(500);

    while(!utilHost_IPC_Ready())
    {
        MS_SLEEP(10);
        //等ipc init做完
    }

    while(1)
    {
        Board_HW_Read_UART0();
		Board_HW_Read_UART3();	//H30K_Zonic_0001, for read LSM UART message
        utilHost_UnPackProcess();
        utilHost_ExecuteProcess();
    #if defined(PLATFORM_R70K) || defined(PLATFORM_H30_4K) //HICC2_AC_0002
        Board_HW_Read_UART3();	//H30K_Zonic_0001, for read LSM UART message
        utilHost_GEC_StateProcess(eMSG_TYPE_UART_3);
    #endif
        MS_SLEEP(10);

        palSystem_TaskMonitorTimerReset(eTID_HOST_PATH, NULL);  //A35G2_Simon_0075
	}

}

eEXEC_CODE palHostPath_Init(void)
{
    UINT8 ucEcho = 0;
#if(CURRENT_RTOS_TYPE != RTOS_STATIC)
    BaseType_t xReturned;
#endif

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);


#if (SYSTEM_OS_TYPE == POSIX_COMPLIANT)

    //Create Mailbox
    LOG_MSG(db_APP_CLI, "Host queue create start\n");
    if(palMailBox_Create_MsgQueue(&m_sPalHostPathInfo.xMsgQueue, QUEUE_HOST_NAME, &m_sPalHostPathInfo.xMsgQueue_attr) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }
    LOG_MSG(db_APP_CLI, "Host queue create end\n");

    //Create Thread
    pthread_attr_init(&m_sPalHostPathInfo.xTaskHandle_attr);
    pthread_attr_setstacksize(&m_sPalHostPathInfo.xTaskHandle_attr, APP_CLI_STACK_SIZE);
    INT16 iCreateThreadError = 0 ;
    iCreateThreadError = pthread_create(&m_sPalHostPathInfo.xTaskHandle, &m_sPalHostPathInfo.xTaskHandle_attr, palHostPath_Task, (void*) NULL);

    if(iCreateThreadError != 0)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Thread Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Thread Pass! (ID:%d)\r\n", __FUNCTION__, __LINE__ , m_sPalHostPathInfo.xTaskHandle);

#else

    if(palMailBox_Create(&m_sPalHostPathInfo.xMsgQueue, &m_sPalHostPathInfo.xEventGroupHandle) != eMAIL_BOX_EXEC_CODE_PASS)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Msg Queue Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    m_sPalHostPathInfo.lPollPeriod = 100;//Because lPollPeriod to 0 means max delay

    xReturned = xTaskCreate(appCLICmd_Task, (const char *) "HOSTTSK",
                APP_HOST_STACK_SIZE,
                NULL,
                APP_HOST_PRIORITY,
                &m_sPalHostPathInfo.xTaskHandle);

    if(xReturned != pdPASS)
    {
        LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Task Fail!\r\n", __FUNCTION__, __LINE__);
        return eEXEC_CODE_FAIL;
    }

    LOG_MSG(db_APP_CLI, "(func:%s, line:%d): Create Task Pass!\r\n", __FUNCTION__, __LINE__);

#endif


    return eEXEC_CODE_PASS;
}

