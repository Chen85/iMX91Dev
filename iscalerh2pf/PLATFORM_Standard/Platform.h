#ifndef _PLATFORM_H_
#define _PLATFORM_H_


//A70G2 Platform

//OE CPC
#define PW_INDEX_MAX_VALUE 719
#define PW_INDEX_DEFAULT_VALUE 492//250//285	//HICC2_Doulas_0034//A70Gen2_Doulas_0006 Modify
#define PW_INDEX_MIN_VALUE 1//0 //HICC2_Julie_0039

#define FW_INDEX_MAX_VALUE 719
#define FW_INDEX_DEFAULT_VALUE 353//253//425	//HICC2_Doulas_0034//A70Gen2_Doulas_0006 Modify
#define PW_INDEX_MIN_VALUE 1//0 //HICC2_Julie_0039



#define DIM_POWER_NUMBER_MAX_VALUE          100
#define DIM_POWER_NUMBER_DEFAULT_VALUE      100
#define DIM_POWER_NUMBER_MIN_VALUE          1

#define TEMP_DIMPOWER_LEVEL             (85)  //環溫DimPower，最大亮度90%，OSD數值
#if defined (PLATFORM_H60_2K)  //HICC2_Doulas_0102
#define LVPS_110V_DIMPOWER_LEVEL        (78)  //LVPS 110V，最大亮度80%，OSD數值 //HICC2_Doulas_0111
#else
#define LVPS_110V_DIMPOWER_LEVEL        (77)  //LVPS 110V，最大亮度85%，OSD數值
#endif
#define LVPS_220V_DIMPOWER_LEVEL        (100) //LVPS 220V，最大亮度100%，OSD數值






#if 0
#define MOTOR_CTRL_FMT54113 (0)
#define MOTOR_CTRL_FMT54605 (1)

#define LD_CTRL_SYS54605_FMT54113 (0)
#define LD_CTRL_SYS54605_FMT54605 (1)

#define FRONTEND_CTRL_SYS54605 (0)


#ifdef PLATFORM_A70G2

    #pragma message ("!!!!! A70G2 Platform !!!!!")

    #define MOTOR_CTRL      MOTOR_CTRL_FMT54113
	#define LD_CTRL         LD_CTRL_SYS54605_FMT54605 //LD_CTRL_SYS54605_FMT54113
	#define FRONTEND_CTRL   FRONTEND_CTRL_SYS54605

#elif defined(PLATFORM_A35G2)

    #pragma message ("!!!!! A35G2 Platform !!!!!")

    #define MOTOR_CTRL      MOTOR_CTRL_FMT54605
	#define LD_CTRL         LD_CTRL_SYS54605_FMT54605
	#define FRONTEND_CTRL   FRONTEND_CTRL_SYS54605

#else

    #pragma message ("!!!!! Undefine Platform !!!!!")

    #define MOTOR_CTRL      MOTOR_CTRL_FMT54113
    #define LD_CTRL         LD_CTRL_SYS54605_FMT54605
    #define FRONTEND_CTRL   FRONTEND_CTRL_SYS54605

#endif


#if (LD_CTRL == LD_CTRL_SYS54605_FMT54113)
    #include "./LD_Ctrl/LD_LPC54113/halLDProc.h"
    #include "./LD_Ctrl/LD_LPC54113/halLDProc.h"
#elif (LD_CTRL == LD_CTRL_SYS54605_FMT54605)
    #include "./LD_Ctrl/LD_LPC54605/halLDProc.h"
    #include "./LD_Ctrl/LD_LPC54605/dvLDDriver.h"
#else
    #pragma message ("!!!!! Undefine Platform !!!!!")
#endif
#endif


#endif //_PLATFORM_H_

