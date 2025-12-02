#include "utilDbgMsg.h"
#include "utilMathAPI.h"
#include "utilDatabaseAPI.h"
#include "utilStorageCfg.h"
#include "halFormatter.h"
#include "dvDDP442x.h"
#include "halLDProc.h"

#include "utilCommon.h"
#include "halScaler.h"
#include "GEC_CoreFunction.h"

#include "utilBIST.h"

#define MSB(d)              ((UINT8)((d) >> 8))
#define LSB(d)              ((UINT8)((d) & 0x00ff))


#define	SFG_Color_Max		1023
#define	SFG_Color_Blue60	240
#define	SFG_Color_Gray10	40
#define	SFG_Color_Min		0

#if 0   // A70LV_Eric.C_0009
#define	Power_Mode_Normal		830
#define	Power_Mode_Middle		700
#define	Power_Mode_Eco			300
#else
// A70LV_Eric.C_0009 Start //A70LV_Larry_0038
#define	Power_Mode_Normal_RLD_R		880
#define	Power_Mode_Normal_RLD_G		0
#define	Power_Mode_Normal_RLD_B		0
#define	Power_Mode_Normal_RLD_Y		880
#define	Power_Mode_Normal_BLD_R		770
#define	Power_Mode_Normal_BLD_G		945
#define	Power_Mode_Normal_BLD_B		955
#define	Power_Mode_Normal_BLD_Y		860

#define	Power_Mode_Eco_RLD_R		500
#define	Power_Mode_Eco_RLD_G		0
#define	Power_Mode_Eco_RLD_B		0
#define	Power_Mode_Eco_RLD_Y		500
#define	Power_Mode_Eco_BLD_R		190
#define	Power_Mode_Eco_BLD_G		235
#define	Power_Mode_Eco_BLD_B		240
#define	Power_Mode_Eco_BLD_Y		215
// A70LV_Eric.C_0009 End
#endif

#define Border_Offset 20        //A70LV_Doulas_0006

static UINT8 wDDP4422VersionSave[2] = {0};  //A70LV_Doulas_0006
static UINT16 m_uiCurrentPWM[eLD_SEG_NUMBERS] = {0};
static UINT8 m_ucCurrentDimPower = 100;
static UINT8 ucFRC_Bypass_Enable_Flag = 0;	//A70LK_Doulas_0008
static UINT8 ucOLD_Testpattern = 0; //H30K_Doulas_0037
static UINT8 m_ucCurrent_DLPLINK_Pulse = FALSE;


//BOOL bSKU_Type = TRUE;
#if 0// 1	//G100_Doulas_0009 Modify for API 9.0 DDP command
void cmdGetHalfword(UINT16* data_out, UINT8* cmd_buffer)
{
	UINT16 temp;
    temp = (*cmd_buffer << 8) | *(cmd_buffer+1);
    *data_out = temp;
}

void cmdPutHalfword(UINT16 data_out, UINT8* cmd_buffer)
{
    *cmd_buffer     = (UINT8)(data_out >> 8);
    *(cmd_buffer+1) = (UINT8)data_out & 0xff;
}

void cmdGetWord(UINT32* data_out, UINT8* cmd_buffer)
{
	UINT32 temp;
    temp = (*cmd_buffer << 8) | (*(cmd_buffer+1) << 16) | (*(cmd_buffer+2) << 8) | *(cmd_buffer+3);
    *data_out = temp;
}

void cmdPutWord(UINT32 data_out, UINT8* cmd_buffer)
{
    *cmd_buffer     = (UINT8)(data_out >> 24) & 0xff;
    *(cmd_buffer+1) = (UINT8)(data_out >> 16) & 0xff;
    *(cmd_buffer+2) = (UINT8)(data_out >> 8)  & 0xff;
    *(cmd_buffer+3) = (UINT8) data_out        & 0xff;
}
#else


void cmdGetHalfword(UINT16* data_out, UINT8* cmd_buffer)   //A70LV_Doulas_0006
{
    UINT16 temp;
    temp = (*(cmd_buffer+1) << 8) | *(cmd_buffer);
    *data_out = temp;
}

void cmdPutHalfword(UINT16 data_out, UINT8* cmd_buffer) //ZU860_John_0017 sync DDP halfword function
{
    *cmd_buffer     = (UINT8)data_out & 0xff;
    *(cmd_buffer+1) = (UINT8)(data_out >> 8);
}

void cmdGetWord(UINT32* data_out, UINT8* cmd_buffer)   //ZU860_Doulas_0046
{
    UINT32 temp;
    temp = (*cmd_buffer) | (*(cmd_buffer+1) << 8) | (*(cmd_buffer+2) << 16) | (*(cmd_buffer+3)<<24);
    *data_out = temp;
}

void cmdPutWord(UINT32 data_out, UINT8* cmd_buffer)    //ZU860_Doulas_0046
{
    *cmd_buffer     = (UINT8)(data_out) & 0xff;
    *(cmd_buffer+1) = (UINT8)(data_out >> 8) & 0xff;
    *(cmd_buffer+2) = (UINT8)(data_out >> 16) & 0xff;
    *(cmd_buffer+3) = (UINT8)(data_out >> 24) & 0xff;
}
#endif

eRESULT halFormatter_GetSSIPWMDriverLevels_1(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;    //A70LV_Doulas_0006
	UINT8	ucDatax[14] = {0};

    eResult = dv442x_Read(CMDF_SSIPWMDRIVERLEVELS, ucDatax);     //A70LV_Doulas_0006 modify
    //uSSI->sSSI.uiRed   = (UINT16)(ucDatax[0]<<8) + (UINT16)ucDatax[1];
    //uSSI->sSSI.uiGreen = (UINT16)(ucDatax[2]<<8) + (UINT16)ucDatax[3];
    //uSSI->sSSI.uiBlue  = (UINT16)(ucDatax[4]<<8) + (UINT16)ucDatax[5];
    //uSSI->sSSI.uiC1    = (UINT16)(ucDatax[6]<<8) + (UINT16)ucDatax[7];
    //uSSI->sSSI.uiC2    = (UINT16)(ucDatax[8]<<8) + (UINT16)ucDatax[9];
    cmdGetHalfword(&(uSSI->sSSI.uiRed),&ucDatax[0]);       //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiGreen),&ucDatax[2]);     //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiBlue),&ucDatax[4]);      //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiC1),&ucDatax[6]);        //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiC2),&ucDatax[8]);        //ZU860_Doulas_0046 modify

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase PWM_R_L = %d , PWM_R_M = %d(%d)\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1],eResult);

    return eResult;     //A70LV_Doulas_0006
}

eRESULT halFormatter_SetSSIPWMDriverLevels_1(uSSI_DRIVER *uSSI)
{
	eRESULT eResult = rcINVALID;  //A70LV_Doulas_0006
	UINT8	ucDatax[12] = {0};

	ucDatax[0] = LSB(uSSI->sLD.uiRed);
	ucDatax[1] = MSB(uSSI->sLD.uiRed);

	ucDatax[2] = LSB(uSSI->sLD.uiGreen);
	ucDatax[3] = MSB(uSSI->sLD.uiGreen);

	ucDatax[4] = LSB(uSSI->sLD.uiBlue);
	ucDatax[5] = MSB(uSSI->sLD.uiBlue);

	ucDatax[6] = LSB(uSSI->sLD.uiYellow);
	ucDatax[7] = MSB(uSSI->sLD.uiYellow);

	ucDatax[8] = LSB(uSSI->sLD.uiRLD_Red);
	ucDatax[9] = MSB(uSSI->sLD.uiRLD_Red);

	ucDatax[10] = LSB(uSSI->sLD.uiRLD_Yellow);
	ucDatax[11] = MSB(uSSI->sLD.uiRLD_Yellow);


	//LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Increase PWM_R_M = %d , PWM_R_L = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1]);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):(R/G/B/Y) = %d %d %d %d\r\n", __FUNCTION__, __LINE__, uSSI->sLD.uiRed, uSSI->sLD.uiGreen, uSSI->sLD.uiBlue, uSSI->sLD.uiYellow);


	eResult = dv442x_Write(CMDF_SSIPWMDRIVERLEVELS,ucDatax);	//A70LV_Doulas_0006
	return eResult; 	//A70LV_Doulas_0006
}

eRESULT halFormatter_SetDisplaySFGColor(UINT16 uiSFG_Red,UINT16 uiSFG_Green,UINT16 uiSFG_Blue)
{
    eRESULT eResult = rcINVALID;  //A70LV_Doulas_0006
	UINT8	ucDatax[6] = {0};

	//ucDatax[0] = MSB(uiSFG_Red);    //A70LV_Doulas_0028
	//ucDatax[1] = LSB(uiSFG_Red);

	//ucDatax[2] = MSB(uiSFG_Green);
	//ucDatax[3] = LSB(uiSFG_Green);

	//ucDatax[4] = MSB(uiSFG_Blue);
	//ucDatax[5] = LSB(uiSFG_Blue);
    cmdPutHalfword(uiSFG_Red,&ucDatax[0]);  //ZU860_Doulas_0046 modify
    cmdPutHalfword(uiSFG_Green,&ucDatax[2]);  //ZU860_Doulas_0046 modify
    cmdPutHalfword(uiSFG_Blue,&ucDatax[4]);  //ZU860_Doulas_0046 modify

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase SFG_R_M = %d , SFG_R_L = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1]);

	eResult = dv442x_Write(CMDF_SFGCOLOR,ucDatax);  //A70LV_Doulas_0006

    return eResult;     //A70LV_Doulas_0006
}

eRESULT halFormatter_GetDisplaySFGColor(UINT16 *uiSFG_Red,UINT16 *uiSFG_Green,UINT16 *uiSFG_Blue)  //A70LV_Doulas_0006
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

	eResult = dv442x_Read(CMDF_SFGCOLOR,ucDatax);

    cmdGetHalfword(uiSFG_Red,&ucDatax[0]);
    cmdGetHalfword(uiSFG_Green,&ucDatax[2]);
    cmdGetHalfword(uiSFG_Blue,&ucDatax[4]);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): R= %d , G= %d , B= %d\r\n", __FUNCTION__, __LINE__,*uiSFG_Red,*uiSFG_Green,*uiSFG_Blue );

    return eResult;     //A70LV_Doulas_0006
}



eRESULT halFormatter_FW_Index_Set(UINT16 uiFWIndex)
{
    eRESULT eResult = rcINVALID, ePass = rcINVALID;
    UINT8 ucDatax[3] = {0};

#ifdef PLATFORM_H60_2K

    cmdPutHalfword(uiFWIndex,&ucDatax[1]);  //G100_Doulas_0009 Modify//ZU860_Doulas_0046 modify

    eResult = dv442x_Write(CMDF_DUALCWDELAY, ucDatax);
#else

    ucDatax[0] = 0x01;
    cmdPutHalfword(uiFWIndex,&ucDatax[1]);  //G100_Doulas_0009 Modify//ZU860_Doulas_0046 modify

    eResult = dv442x_Write(CMDF_DUALCWDELAY, ucDatax);

#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Wheel = %d Increase PWIndex_M = %x , PWIndex_L = %x (Ret %d)\r\n", __FUNCTION__, __LINE__, uiFWIndex,ucDatax[0],ucDatax[1],eResult);

    return eResult;
}

eRESULT halFormatter_FW_Index_Get(UINT16 *uiFWIndex)
{
    eRESULT eResult = rcINVALID;
    UINT8 WheelNumber = 1;
    UINT8 ucDatax[3] = {0};

    dv442x_Write(CMDF_SET_GETDUALCWDELAY, &WheelNumber);

    eResult = dv442x_Read(CMDF_DUALCWDELAY, ucDatax);

    //ucDatax[0] ^= ucDatax[1];	//G100_Doulas_0009
    //ucDatax[1] ^= ucDatax[0];	//G100_Doulas_0009
    //ucDatax[0] ^= ucDatax[1];	//G100_Doulas_0009
    cmdGetHalfword(uiFWIndex,&ucDatax[0]);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase FWIndex_M = %x , FWIndex_L = %x\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1]);
    LOG_MSG(db_HAL_FORMATTER, "(ulFWIndex = %d\r\n", *uiFWIndex);

    return eResult;
}

//ZU860_John_0014 start fix wheel index command
eRESULT halFormatter_PW_Index_Get(UINT16 *uiPWIndex)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[3] = {0};

#ifdef PLATFORM_H60_2K

    UINT8 WheelNumber = 0;
    dv442x_Write(CMDF_SET_GETDUALCWDELAY, &WheelNumber);

    eResult = dv442x_Read(CMDF_UTLCWINDEX, ucDatax);	//G100_Doulas_0009 Modify
    cmdGetHalfword(uiPWIndex, &ucDatax[0]);

#else

    UINT8 WheelNumber = 2;
    dv442x_Write(CMDF_SET_GETDUALCWDELAY, &WheelNumber);

    eResult = dv442x_Read(CMDF_DUALCWDELAY, ucDatax);	//G100_Doulas_0009 Modify
    cmdGetHalfword(uiPWIndex, &ucDatax[0]);

#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase PWIndex_M = %x , PWIndex_L = %x, PWIndex = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1], *uiPWIndex);

    return eResult;
}


eRESULT halFormatter_PW_Index_Set(UINT16 uiPWIndex)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[3] =  {0};

#ifdef PLATFORM_H60_2K
    //ucDatax[0] = 0x01;
    //ucDatax[0] = LSB(uiFWIndex);
    //ucDatax[1] = MSB(uiFWIndex);
    cmdPutHalfword(uiPWIndex,&ucDatax[1]);  //G100_Doulas_0009 Modify//ZU860_Doulas_0046 modify
    eResult = dv442x_Write(CMDF_UTLCWINDEX, ucDatax);      //A70LV_Doulas_0006 modify
#else
    ucDatax[0] = 0x02;
    cmdPutHalfword(uiPWIndex,&ucDatax[1]);
    eResult = dv442x_Write(CMDF_DUALCWDELAY, ucDatax);
#endif /* PLATFORM_H60_2K */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Wheel = %d Increase PWIndex_M = %x , PWIndex_L = %x\r\n", __FUNCTION__, __LINE__, uiPWIndex,ucDatax[0],ucDatax[1]);

    LOG_MSG(db_HAL_FORMATTER, "(ulPWIndex = %d\r\n", uiPWIndex);

    return eResult;
}

eRESULT halFormatter_Actuator_Switch_Set(UINT8 ucActuator)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = XPR_4WAY_CMD_FIXEDEN; //XPR_4WAY_CMD_FIXEDE

	switch(ucActuator) // AWC0
	{
		case ets_OFF :
		{
			ucDatax[2] = 0x01; //Off
            LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Actuator_Switch: Off\r\n", __FUNCTION__, __LINE__);
		}
		break;

		case ets_ON:
		{
			ucDatax[2] = 0x00; //On
            LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Actuator_Switch: On\r\n", __FUNCTION__, __LINE__);
		}
		break;
	}

    ucDatax[1] = 0x00; //AWC0
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);

    ucDatax[1] = 0x01; //AWC1
    eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);
#endif

    return eResult;
}

eRESULT halFormatter_Actuator_Switch_Get(UINT8 *pucActuator)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = CMDF_4WAYXPR_PARAM;
    ucDatax[1] = XPR_4WAY_CMD_FIXEDEN; //XPR_4WAY_CMD_FIXEDE
    ucDatax[2] = 0x00; //AWC0
    ucDatax[3] = 0x00;

	eResult = dv442x_Read(CMDF_4WAYXPR_PARAM,ucDatax);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Get Actuator_Switch : %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

    if(ucDatax[0])
    {
        //Off
        *pucActuator = 0;
    }
    else
    {
        //On
        *pucActuator = 1;
    }

#endif


    return eResult;
}

eRESULT halFormatter_Actuator_Switch(UINT8 ucActuator) //HICC2_Doulas_0097
{
#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[3] = {0};

    ucDatax[0] = 0x00; //XPR_4WAY_CMD_FIXEDE

	switch(ucActuator) // AWC0
	{
		case ets_OFF :
		{
			ucDatax[1] = 0x00; //AWC0
			ucDatax[2] = 0x01; //Off
		}
		break;

		case ets_ON:
		{
			ucDatax[1] = 0x00; //AWC0
			ucDatax[2] = 0x00; //On
		}
		break;
	}
	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Actuator_Switch AWC0 : %d\r\n", __FUNCTION__, __LINE__, ucDatax[2]);
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);

	switch(ucActuator) // AWC1
	{
		case ets_OFF :
		{
			ucDatax[1] = 0x01; //AWC1
			ucDatax[2] = 0x01; //Off
		}
		break;

		case ets_ON:
		{
			ucDatax[1] = 0x01; //AWC1
			ucDatax[2] = 0x00; //On
		}
		break;
	}

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Actuator_Switch AWC1 : %d\r\n", __FUNCTION__, __LINE__, ucDatax[2]);
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);

    return eResult;
#else

    eRESULT eResult = rcINVALID;  //A70LV_Doulas_0006
	UINT8	ucDatax[2] = {0};

	ucDatax[0] = LSB(ucActuator+1);
	ucDatax[1] = 0x00;

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase uActuator_M = %d,uActuator_L = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1]);

	eResult = dv442x_Write(CMDF_SYSTEMMODE,ucDatax);    //A70LV_Doulas_0006

    return eResult;  //A70LV_Doulas_0006
#endif
}

eRESULT halFormatter_Actuator_FixedOut_Set(UINT8 ucCh, UINT8 ucFixedOut)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = XPR_4WAY_CMD_FIXEDEN;
    ucDatax[1] = ucCh; //AWC0
    ucDatax[2] = ucFixedOut; //On

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): Set %d FixedOut = %d\r\n", __FUNCTION__, __LINE__, ucCh, ucFixedOut);
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM, ucDatax);
#endif

    return eResult;
}

eRESULT halFormatter_Actuator_FixedOut_Get(UINT8 ucCh, UINT8 *ucFixedOut)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = CMDF_4WAYXPR_PARAM;
    ucDatax[1] = XPR_4WAY_CMD_FIXEDEN;
    ucDatax[2] = ucCh; //AWC0
    ucDatax[3] = 0x00;

	eResult = dv442x_Read(CMDF_4WAYXPR_PARAM,ucDatax);
    *ucFixedOut = ucDatax[0];

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Get %d ucFixedOut = %d\r\n", __FUNCTION__, __LINE__, ucCh,  *ucFixedOut);
#endif

    return eResult;
}


eRESULT halFormatter_Actuator_Gain_Set(UINT8 ucCh, UINT8 ucActuatorGain)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = XPR_4WAY_CMD_DACGAIN;
	ucDatax[1] = ucCh;
	ucDatax[2] = ucActuatorGain;

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Set %d ucActuatorGain = %d\r\n", __FUNCTION__, __LINE__, ucCh, ucActuatorGain);
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);
#endif

    return eResult;
}

eRESULT halFormatter_Actuator_Gain_Get(UINT8 ucCh, UINT8 *ucActuatorGain)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = CMDF_4WAYXPR_PARAM;
    ucDatax[1] = XPR_4WAY_CMD_DACGAIN;
    ucDatax[2] = ucCh; //AWC0
    ucDatax[3] = 0x00;

	eResult = dv442x_Read(CMDF_4WAYXPR_PARAM,ucDatax);
    *ucActuatorGain = ucDatax[0];
#endif

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Get %d ucActuatorGain = %d\r\n", __FUNCTION__, __LINE__, ucCh,  *ucActuatorGain);

    return eResult;
}
eRESULT halFormatter_Actuator_Segment_Length_Set(UINT8 ucCh, UINT16 uiSeg)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
	ucDatax[0] = XPR_4WAY_CMD_SEGMENTLEN;
	ucDatax[1] = ucCh;
	memcpy(&ucDatax[2], (UINT8*)&uiSeg, 2);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Set %d Segment length = %d\r\n", __FUNCTION__, __LINE__, ucCh,  uiSeg);
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);
#endif

    return eResult;
}

eRESULT halFormatter_Actuator_Segment_Length_Get(UINT8 ucCh, UINT16 *uiSeg)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = CMDF_4WAYXPR_PARAM;
    ucDatax[1] = XPR_4WAY_CMD_SEGMENTLEN;
    ucDatax[2] = ucCh; //AWC0
    ucDatax[3] = 0x00;

	eResult = dv442x_Read(CMDF_4WAYXPR_PARAM,ucDatax);
    memcpy((UINT8*)uiSeg, &ucDatax[0], 2);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Get %d Segment length = %d\r\n", __FUNCTION__, __LINE__, ucCh, *uiSeg);
#endif

    return eResult;
}

eRESULT halFormatter_Actuator_SubFrameDelay_Set(UINT8 ucCh, UINT32 ulDelay)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
	ucDatax[0] = XPR_4WAY_CMD_SFDELAY;
	ucDatax[1] = ucCh;
	memcpy(&ucDatax[2], (UINT8*)&ulDelay, 4);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Set %d delay = %d\r\n", __FUNCTION__, __LINE__, ucCh, ulDelay);
	eResult = dv442x_Write(CMDF_4WAYXPR_PARAM,ucDatax);
#endif

    return eResult;
}

eRESULT halFormatter_Actuator_SubFrameDelay_Get(UINT8 ucCh, UINT32 *ulDelay)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[6] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = CMDF_4WAYXPR_PARAM;
    ucDatax[1] = XPR_4WAY_CMD_SFDELAY; //XPR_4WAY_CMD_FIXEDE
    ucDatax[2] = ucCh; //AWC0
    ucDatax[3] = 0x00;

	eResult = dv442x_Read(CMDF_4WAYXPR_PARAM,ucDatax);
    memcpy((UINT8*)ulDelay, &ucDatax[0], 4);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):Get %d delay = %d\r\n", __FUNCTION__, __LINE__, ucCh, *ulDelay);
#endif

    return eResult;
}


eRESULT halFormatter_Standby(void) //A70LV_Larry_0212
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax = eCMD_Formatter_Standby;

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d)Standby\r\n", __FUNCTION__, __LINE__);

	//eResult = dv442x_Write(CMDF_PROJECTIONMODE,&ucDatax);
    eResult = dv442x_Write(CMDF_SETPOWER,&ucDatax);   //ZU860_Doulas_0047 modify

	MS_SLEEP( 100 );

    return eResult;
}

eRESULT halFormatter_TestPattern(UINT8 ucPattern) //HICC2_Casper_0022
{
    eRESULT eResult = rcINVALID;  //A70LV_Doulas_0006
	UINT8	ucDatax[2] = {0};

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase ucPattern = %d\r\n", __FUNCTION__, __LINE__, ucPattern);

#ifdef CURSOR_FIXTURE //HICC2_Doulas_0020
	switch(ucPattern)
	{
		case eCM_FACTORY_TP_NONE:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
		}
		break;
		case eCM_FACTORY_TP_FULL_SREEN_W:
		{
			halFormatter_SplashLoad(1);
		}
		break;
		case eCM_FACTORY_TP_13POINTS:
		{
			halFormatter_SplashLoad(3);
		}
		break;
		case eCM_FACTORY_TP_BLACK:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Min,SFG_Color_Min);
		}
		break;
		case eCM_FACTORY_TP_RED:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Max,SFG_Color_Min,SFG_Color_Min);
		}
		break;
		case eCM_FACTORY_TP_GREEN:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Max,SFG_Color_Min);
		}
		break;
		case eCM_FACTORY_TP_BLUE:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Min,SFG_Color_Max);
		}
		break;
		default:
			break;
	}
#else
	switch(ucPattern)
	{
		case eCM_FACTORY_TP_NONE:
		{
	    #if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)     //HICC2_Doulas_0093
    	    halFormatter_XPR_CalibrationPattern_Set(0);
	    #endif
			halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
		} break;

		case eCM_FACTORY_TP_WHITE:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Max,SFG_Color_Max,SFG_Color_Max);
		} break;

		case eCM_FACTORY_TP_BLACK:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Min,SFG_Color_Min);
		} break;

		case eCM_FACTORY_TP_RED:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Max,SFG_Color_Min,SFG_Color_Min);
		} break;

		case eCM_FACTORY_TP_GREEN:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Max,SFG_Color_Min);
		} break;

		case eCM_FACTORY_TP_BLUE:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Min,SFG_Color_Max);
		} break;

		case eCM_FACTORY_TP_CYAN:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Max,SFG_Color_Max);
		} break;

		case eCM_FACTORY_TP_YELLOW:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Max,SFG_Color_Max,SFG_Color_Min);
		} break;

		case eCM_FACTORY_TP_MAGENTA:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Max,SFG_Color_Min,SFG_Color_Max);
		} break;

		case eCM_FACTORY_TP_BLUE60:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Min,SFG_Color_Min,SFG_Color_Blue60);
		} break;

		case eCM_FACTORY_TP_GRAY10:
		{
			halFormatter_Projection_Mode_Set(eCMD_Formatter_SolidField);
			halFormatter_SetDisplaySFGColor(SFG_Color_Gray10,SFG_Color_Gray10,SFG_Color_Gray10);
		} break;

        case eCM_FACTORY_TP_WRGB64:
		{
		    if(ucOLD_Testpattern == eCM_FACTORY_TP_GRAY10) //H30K_Doulas_0037
		    {
		        halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
		    }
			halFormatter_SplashLoad(2);
		} break;

        case eCM_FACTORY_TP_FLARE:
        {
			//halFormatter_SplashLoad(2);
		} break;

        case eCM_FACTORY_TP_FULL_SREEN_W:
		{
			halFormatter_SplashLoad(1);
		} break;

        case eCM_FACTORY_TP_FULL_SREEN_B:
        {
			//halFormatter_SplashLoad(2);
		} break;

		case eCM_FACTORY_TP_LATERAL_COLOR:
		{
			//halFormatter_SplashLoad(3);
		} break;

		case eCM_FACTORY_TP_FOCUS_WORD:
		{
			//halFormatter_SplashLoad(1);
		} break;

		case eCM_FACTORY_TP_13POINTS:
		{
			halFormatter_SplashLoad(3);
		} break;

		case eCM_FACTORY_TP_FULL_COLOR:
		{
			//halFormatter_SplashLoad(3);
		} break;

		case eCM_FACTORY_TP_BORESIGHT:
		{

		} break;

        case eCM_FACTORY_TP_ACTUATOR:
        {
			halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
            halFormatter_XPR_CalibrationPattern_Set(1);
        } break;

		case eCM_FACTORY_TP_4K_FULL_SCREEN_1P:
		{
    	    halFormatter_XPR_CalibrationPattern_Set(0);
			halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
		} break;

		case eCM_FACTORY_TP_4K_FULL_SCREEN_2P:
		{
    	    halFormatter_XPR_CalibrationPattern_Set(0);
			halFormatter_Projection_Mode_Set(eCMD_Formatter_External);
		} break;

		default:
		{
		} break;
	}
#endif
    ucOLD_Testpattern = ucPattern; //H30K_Doulas_0037
    return eResult;  //A70LV_Doulas_0006
}


eRESULT halFormatter_HorFlip(BOOL bH_Flip)
{
    eRESULT eResult = rcINVALID;    //A70LV_Doulas_0006
	UINT8	ucDatax[1] = {0};

	ucDatax[0] = LSB(bH_Flip);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):bH_Flip = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

	eResult = dv442x_Write(CMDF_HORIZONTALFLIP,ucDatax);

    return eResult;    //A70LV_Doulas_0006
}

eRESULT halFormatter_VerFlip(BOOL bV_Flip)
{
    eRESULT eResult = rcINVALID;    //A70LV_Doulas_0006
	UINT8	ucDatax[1] = {0};

	ucDatax[0] = LSB(bV_Flip);

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):bV_Flip = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

	eResult = dv442x_Write(CMDF_VERTICALFLIP,ucDatax);

    return eResult;    //A70LV_Doulas_0006
}

//A70LV_Doulas_0006 start
void halFormatter_AsicReadySet(BOOL bBnable)
{
    dv442x_AsicReadySet(bBnable);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AsicReady %d\r\n", __FUNCTION__, __LINE__,bBnable);
}

BOOL halFormatter_AsicReadyGet(void) //HICC2_Doulas_0085
{
    return dv442x_AsicReadGet();
}

eRESULT halFormatter_HorFlipGet(BOOL *bH_Flip)
{
    eRESULT eResult = rcINVALID;    //A70LV_Doulas_0006
    UINT8   ucDatax[1] = {0};

    eResult = dv442x_Read(CMDF_HORIZONTALFLIP,ucDatax);

    *bH_Flip = (BOOL)ucDatax[0];
	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):bH_Flip Get = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

    return eResult;    //A70LV_Doulas_0006
}

eRESULT halFormatter_VerFlipGet(BOOL *bV_Flip)
{
    eRESULT eResult = rcINVALID;    //A70LV_Doulas_0006
    UINT8   ucDatax[1] = {0};

    eResult = dv442x_Read(CMDF_VERTICALFLIP,ucDatax);

    *bV_Flip = (BOOL)ucDatax[0];
	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):bV_Flip Get= %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

    return eResult;    //A70LV_Doulas_0006
}

eRESULT halFormatter_GetSSIPWMDriverLevels_2(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;
    UINT8   ucDatax[14] = {0};

    eResult = dv442x_Read(CMDF_SSIPWMDRIVERLEVELS2, ucDatax);

    //uSSI->sSSI.uiRed   = (UINT16)(ucDatax[0]<<8) + (UINT16)ucDatax[1];  //A70LV_Doulas_0028
    //uSSI->sSSI.uiGreen = (UINT16)(ucDatax[2]<<8) + (UINT16)ucDatax[3];
    //uSSI->sSSI.uiBlue  = (UINT16)(ucDatax[4]<<8) + (UINT16)ucDatax[5];
    //uSSI->sSSI.uiC1    = (UINT16)(ucDatax[6]<<8) + (UINT16)ucDatax[7];
    //uSSI->sSSI.uiC2    = (UINT16)(ucDatax[8]<<8) + (UINT16)ucDatax[9];
    cmdGetHalfword(&(uSSI->sSSI.uiRed),&ucDatax[0]);    //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiGreen),&ucDatax[2]);  //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiBlue),&ucDatax[4]);   //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiC1),&ucDatax[6]);     //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiC2),&ucDatax[8]);     //ZU860_Doulas_0046 modify
    cmdGetHalfword(&(uSSI->sSSI.uiSense),&ucDatax[10]);  	//G100_Doulas_0010 Add


	LOG_MSG(db_HAL_FORMATTER, "PWM2 (%d,%d,%d,%d)(%d,%d)\r\n",uSSI->sSSI.uiRed,
													   uSSI->sSSI.uiGreen,
													   uSSI->sSSI.uiBlue,
													   uSSI->sSSI.uiC1,
													   uSSI->sSSI.uiC2,
													   uSSI->sSSI.uiSense);
    return eResult;
}

eRESULT halFormatter_SetSSIPWMDriverLevels_2(uSSI_DRIVER *uSSI)
{
    eRESULT eResult = rcINVALID;
	UINT8	ucDatax[13] = {0};

    //ucDatax[0] = MSB(uSSI->sSSI.uiRed);       //A70LV_Doulas_0028 modify
    //ucDatax[1] = LSB(uSSI->sSSI.uiRed);
    //ucDatax[2] = MSB(uSSI->sSSI.uiGreen);
    //ucDatax[3] = LSB(uSSI->sSSI.uiGreen);
    //ucDatax[4] = MSB(uSSI->sSSI.uiBlue);
    //ucDatax[5] = LSB(uSSI->sSSI.uiBlue);
    //ucDatax[6] = MSB(uSSI->sSSI.uiC1);
    //ucDatax[7] = LSB(uSSI->sSSI.uiC1);
    //ucDatax[8] = MSB(uSSI->sSSI.uiC2);
    //ucDatax[9] = LSB(uSSI->sSSI.uiC2);
    cmdPutHalfword(uSSI->sSSI.uiRed,&ucDatax[0]);   //ZU860_Doulas_0046 modify
    cmdPutHalfword(uSSI->sSSI.uiGreen,&ucDatax[2]); //ZU860_Doulas_0046 modify
    cmdPutHalfword(uSSI->sSSI.uiBlue,&ucDatax[4]);  //ZU860_Doulas_0046 modify
    cmdPutHalfword(uSSI->sSSI.uiC1,&ucDatax[6]);    //ZU860_Doulas_0046 modify
    cmdPutHalfword(uSSI->sSSI.uiC2,&ucDatax[8]);    //ZU860_Doulas_0046 modify

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Increase PWM_R_M = %d , PWM_R_L = %d\r\n", __FUNCTION__, __LINE__, ucDatax[0],ucDatax[1]);

	eResult = dv442x_Write(CMDF_SSIPWMDRIVERLEVELS2,ucDatax);

    return eResult;
}

eRESULT halFormatter_SystemModeSet(UINT8 cSystemMode)
{
    eRESULT eResult = rcINVALID;
    UINT8    ucDatax[2] = {0};

    ucDatax[0] = cSystemMode;
    ucDatax[1] = 0x00;  // A70_Eddie_0004. DDP received two bytes, but we may not need so many system modes so fix high byte to 0x00.

    eResult = dv442x_Write(CMDF_SYSTEMMODE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):System mode set %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

    return eResult;
}

eRESULT halFormatter_SystemModeGet(UINT8 *cSystemMode)
{
    eRESULT eResult = rcINVALID;
    UINT8    ucDatax[2] = {0};  //ZU860_Doulas_0046

    eResult = dv442x_Read(CMDF_SYSTEMMODE, ucDatax);    //ZU860_Doulas_0046 modify

    *cSystemMode = ucDatax[0];                              //ZU860_Doulas_0046 modify

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):System mode get %d\r\n", __FUNCTION__, __LINE__,*cSystemMode);

    return eResult;
}

eRESULT halFormatter_SplashLoad(UINT8 cNum) //HICC2_Casper_0021
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    //ucDatax[0] = eCMD_Formatter_Splash; //H30K_Doulas_0037
    //eResult = dv442x_Write(CMDF_PROJECTIONMODE,&ucDatax[0]); //H30K_Doulas_0037
    //vTaskDelay( 100 ); //H30K_Doulas_0037

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K) //HICC2_Casper_0020
	if(cNum >0)
	{
    	ucDatax[0] = (2*cNum) - 1; //only 0 ~ 3
	}
	else
	{
        ucDatax[0] = cNum; //only 0 ~ 3
	}
#else //H30K_Doulas_0037
    ucDatax[0] = eCMD_Formatter_Splash;
    eResult = dv442x_Write(CMDF_PROJECTIONMODE,ucDatax);
    ucDatax[0] = cNum; //only 0 ~ 3
#endif
    ucDatax[1] = 0xFF; // Must be -1 for 2D Splash

    eResult = dv442x_Write(CMDF_SPLASHLOAD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Splash set %d\r\n", __FUNCTION__, __LINE__,cNum);
    //vTaskDelay( 100 ); //H30K_Doulas_0037
    return eResult;
}

eRESULT halFormatter_dv4422_TestPattern(UINT8 cTestPattern)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = cTestPattern;

    eResult = dv442x_Write(CMDF_TPGPATTERN, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Test pattern set %d\r\n", __FUNCTION__, __LINE__,cTestPattern);
    return eResult;
}

eRESULT halFormatter_CW_SpokeTest(UINT8 cEnable)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = cEnable;

    eResult = dv442x_Write(CMDF_CWSPOKETEST, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Spoke set %d\r\n", __FUNCTION__, __LINE__,cEnable);
    return eResult;
}

eRESULT halFormatter_ColorOffsetSet(UINT8 ucColorOffset)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = ucColorOffset;

    eResult = dv442x_Write(CMDF_SETCOLOROFFSET, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):ColorOffset set %d\r\n", __FUNCTION__, __LINE__,ucColorOffset);
    return eResult;
}

eRESULT halFormatter_SPLASH_SCRRENSet(UINT8 ucSplashStartupEnable)
{
#if 1
    (void)ucSplashStartupEnable;
#else
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = ucSplashStartupEnable;
    eResult = dv442x_Write(CMDF_SPLASH_SCREEN, &ucDatax[0]);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):SplashStartupEnable set %d\r\n", __FUNCTION__, __LINE__,ucSplashStartupEnable);
#endif /* 0 */
    return rcSUCCESS;
}

eRESULT halFormatter_IMG_AlgorithmSet(IMG_ALGORITHMS eAlgo, BOOL bState)
{
  //  #define ASSIGNBIT(x,bitmsk,bit)( (bit) ?( (x) |=(bitmsk)) :((x) &= ~(bitmsk)))
    eRESULT eResult = rcINVALID;
    UINT8 cAlg = 0, cBuffer = 0;

    eResult = dv442x_Read(CMDF_IMG_ALGORITHM, &cBuffer);

    if(eAlgo == IMG_ALG_GAMMA)
        cAlg = ASSIGNBIT(cBuffer, 0x01, bState);
    else if(eAlgo == IMG_ALG_CCA)
        cAlg = ASSIGNBIT(cBuffer, 0x02, bState);
    else if(eAlgo == IMG_ALG_BRICOLOR)
        cAlg = ASSIGNBIT(cBuffer, 0x04, bState);
    else if(eAlgo == IMG_ALG_STM)
        cAlg = ASSIGNBIT(cBuffer, 0x08, bState);
    else if(eAlgo == IMG_ALG_BRS)
        cAlg = ASSIGNBIT(cBuffer, 0x10, bState);

    eResult = dv442x_Write(CMDF_IMG_ALGORITHM, &cAlg);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):IMG_AlgorithmSet (%d,%d)%d\r\n", __FUNCTION__, __LINE__,eAlgo,bState,cAlg);

    return eResult;
}

eRESULT halFormatter_IMG_GammaLutSet(UINT8 cGamma)
{
    eRESULT  eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_GAMMATABLE, ucDatax);
    if((eResult != rcSUCCESS) || (ucDatax[0] != cGamma))
    {
        ucDatax[0] = cGamma;
        eResult = dv442x_Write(CMDF_GAMMATABLE, ucDatax);
    }

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):GammaLutSet %d\r\n", __FUNCTION__, __LINE__,cGamma);
    return eResult;
}

eRESULT halFormatter_IMG_GammaLutGet(UINT8* cGamma)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_GAMMATABLE, ucDatax);

    *cGamma = ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):GammaLutGet %d\r\n", __FUNCTION__, __LINE__,*cGamma);

    return eResult;
}

eRESULT halFormatter_DB_EnableSet(UINT8 cDB)
{
    eRESULT  eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = cDB;

    eResult = dv442x_Write(CMDF_DBENABLE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_EnableSet %d\r\n", __FUNCTION__, __LINE__,cDB);
    return eResult;
}

eRESULT halFormatter_DB_EnableGet(UINT8 *cDB)
{
    eRESULT  eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_DBENABLE, ucDatax);

    *cDB = ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_EnableGet %d\r\n", __FUNCTION__, __LINE__,*cDB);
    return eResult;
}

eRESULT halFormatter_DB_SetAperture(UINT16 cPosition)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    //ucDatax[0] = MSB(cPosition);    //A70LV_Doulas_0028 modify
    //ucDatax[1] = LSB(cPosition);
    cmdPutHalfword(cPosition,&ucDatax[0]);  //ZU860_Doulas_0046 modify

    eResult = dv442x_Write(CMDF_DBPOSITION, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_SetAperture %d\r\n", __FUNCTION__, __LINE__,cPosition);
    return eResult;
}

eRESULT halFormatter_DB_GetAperture(UINT16 *cPosition)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_DBPOSITION, ucDatax);

    cmdGetHalfword(cPosition,&ucDatax[0]);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_GetAperture %d\r\n", __FUNCTION__, __LINE__,*cPosition);
    return eResult;
}

eRESULT halFormatter_DB_SetMinMaxAperture(UINT16 ucMinPosition,UINT16 ucMaxPosition)
{
    //A70_Larry_0193 fixed
    eRESULT         eResult = rcINVALID;
    UINT8 ucDatax[4] = {0};

    //ucDatax[0] = MSB(ucMinPosition);        //A70LV_Doulas_0028 modify
    //ucDatax[1] = LSB(ucMinPosition);//min
    //ucDatax[2] = MSB(ucMaxPosition);
    //ucDatax[3] = LSB(ucMaxPosition);//max
    cmdPutHalfword(ucMinPosition,&ucDatax[0]);  //ZU860_Doulas_0046
    cmdPutHalfword(ucMaxPosition,&ucDatax[2]);  //ZU860_Doulas_0046

    eResult = dv442x_Write(CMDF_DBMINMAX, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_SetMinMaxAperture %d,%d\r\n", __FUNCTION__, __LINE__,ucMinPosition,ucMaxPosition);
    return eResult;
}

eRESULT halFormatter_DB_GetMinMaxAperture(UINT16 *ucMinPosition,UINT16 *ucMaxPosition)
{
    eRESULT         eResult = rcINVALID;
    UINT8 ucDatax[4] = {0};

    eResult = dv442x_Read(CMDF_DBMINMAX, ucDatax);

    cmdGetHalfword(ucMinPosition,&ucDatax[0]);
    cmdGetHalfword(ucMaxPosition,&ucDatax[2]);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_SetMinMaxAperture %d,%d\r\n", __FUNCTION__, __LINE__,*ucMinPosition,*ucMaxPosition);
    return eResult;
}

eRESULT halFormatter_DB_SetOpenCloseAperture(UINT8 cOpenClose)
{
    eRESULT  eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = cOpenClose; //1 or 0

    eResult = dv442x_Write(CMDF_DBAPERTURE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_SetOpenCloseAperture %d\r\n", __FUNCTION__, __LINE__,cOpenClose);
    return eResult;
}

eRESULT halFormatter_DB_GetCurrentAperture(UINT16 *ucCurrentAperture)
{
    eRESULT         eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_DBAPERTURE, ucDatax);
    cmdGetHalfword(ucCurrentAperture,&ucDatax[0]);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):DB_GetCurrentAperture %d\r\n", __FUNCTION__, __LINE__,*ucCurrentAperture);
    return eResult;
}

eRESULT halFormatter_SYS_AppVersionGet(UINT16 *wDDP)
{
    eRESULT eResult = rcINVALID;
    UINT8    ucDatax[21] = {0};

    eResult = dv442x_Read(CMDF_APPVERSIONS, ucDatax);

    *wDDP = (UINT16)(ucDatax[3]<<8) | (UINT16)ucDatax[2];
    wDDP4422VersionSave[0] = (UINT8)ucDatax[3];
    wDDP4422VersionSave[1] = (UINT8)ucDatax[2];

    return eResult;
}

eRESULT halFormatter_VersionSaveGet(UINT8 *pcData)
{
    pcData[0] = wDDP4422VersionSave[0];
    pcData[1] = wDDP4422VersionSave[1];
    return rcSUCCESS;
}

eRESULT halFormatter_SEQ_RevisionGet(UINT32 *ucData)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[4] = {0};

    eResult = dv442x_Read(CMDF_SEQVERSION, ucDatax);

    *ucData = (UINT32)ucDatax[0]+ (UINT32)(ucDatax[1]<<8) +(UINT32)(ucDatax[2]<<16)+(UINT32)(ucDatax[3]<<24);

    return eResult;
}

eRESULT halFormatter_3D_3DModeSet(BOOL cEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = (UINT8)cEnable;

#ifndef FRAME_SEQUENTIAL_3D_ULTRA
    eResult = dv442x_Write(CMDF_3DENABLE, ucDatax);
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3DModeSet %d\r\n", __FUNCTION__, __LINE__,cEnable);
    return eResult;
}


eRESULT halFormatter_3D_3DModeGet(BOOL *pbEnable)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_3DENABLE, ucDatax);

    *pbEnable = (BOOL)ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3DModeGet %d\r\n", __FUNCTION__, __LINE__,*pbEnable);
    return eResult;
}

eRESULT halFormatter_IMG_WhitePeakingSet(UINT8 cWhitePeak)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = cWhitePeak;

    eResult = dv442x_Write(CMDF_WHITEPEAKING, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):WhitePeakingSet %d\r\n", __FUNCTION__, __LINE__,cWhitePeak);
    return eResult;
}

eRESULT halFormatter_IMG_WhitePeakingGet(UINT8 *cWhitePeak)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_WHITEPEAKING, ucDatax);

    *cWhitePeak = ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):WhitePeakingGet %d\r\n", __FUNCTION__, __LINE__,*cWhitePeak);
    return eResult;
}

eRESULT halFormatter_3D_RefPolaritySet( BOOL bRef3D, BOOL bOddEven, BOOL bTopField, BOOL bInvert )
{
    eRESULT  eResult = rcINVALID;
    UINT8     ucDatax[4] = {0};

#ifndef FRAME_SEQUENTIAL_3D_ULTRA
    #if 1       //ZU860_Doulas_0064 Modify
    BOOL bRef3D2 = FALSE;
    BOOL bOddEven2 = FALSE;
    BOOL bTopField2 = FALSE;
    BOOL bInvert2 = FALSE;
    eResult = halFormatter_3D_RefPolarityGet(&bRef3D2,&bOddEven2,&bTopField2,&bInvert2);
    if(eResult == rcSUCCESS)
    {
        ucDatax[0] = bRef3D;
        ucDatax[1] = bOddEven2;
        ucDatax[2] = bTopField2;
        ucDatax[3] = bInvert2;
    }
    else
    {
        ucDatax[0] = bRef3D;
        ucDatax[1] = bOddEven;
        ucDatax[2] = bTopField;
        ucDatax[3] = bInvert;
    }
    #else
    ucDatax[0] = bRef3D;    //Set FALSE
    ucDatax[1] = bOddEven;  //Set FALSE
    ucDatax[2] = bTopField; //Set FALSE
    ucDatax[3] = bInvert;   //Set TRUE or FALSE(re-sync input source can't changed 3D invert)
    #endif

    eResult = dv442x_Write(CMDF_3DREFPOLARITY, ucDatax);
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D Sync Set (%d,%d,%d,%d)\r\n", __FUNCTION__, __LINE__,ucDatax[0],ucDatax[1],ucDatax[2],ucDatax[3]);
    return eResult;
}

eRESULT halFormatter_3D_RefPolarityGet( BOOL *bRef3D, BOOL *bOddEven, BOOL *bTopField, BOOL *bInvert )
{
    eRESULT  eResult = rcINVALID;
    UINT8     ucDatax[4] = {0,0,0,0};

    eResult = dv442x_Read(CMDF_3DREFPOLARITY, ucDatax);

    *bRef3D      = (BOOL)ucDatax[0];
    *bOddEven    = (BOOL)ucDatax[1];
    *bTopField   = (BOOL)ucDatax[2];
    *bInvert     = (BOOL)ucDatax[3];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D Sync Get (%d,%d,%d,%d)\r\n", __FUNCTION__, __LINE__,*bRef3D,*bOddEven,*bTopField,*bInvert);
    return eResult;
}

eRESULT halFormatter_3D_DLPLinkPulseSet(BOOL bEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8     ucDatax[2] = {0};

    ucDatax[0] = (UINT8)bEnable;

    eResult = dv442x_Write(CMDF_3DDLPLINKPULSE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DLPLinkPulseSet %d\r\n", __FUNCTION__, __LINE__,bEnable);
	if(eResult == rcSUCCESS)
		m_ucCurrent_DLPLINK_Pulse = ucDatax[0];
    return eResult;
}

eRESULT halFormatter_3D_DLPLinkPulseGet(BOOL *pbEnable)
{
    eRESULT eResult = rcINVALID;
    UINT8     ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_3DDLPLINKPULSE, ucDatax);

    *pbEnable = ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DLPLinkPulseGet %d\r\n", __FUNCTION__, __LINE__,*pbEnable);
    return eResult;
}

UINT8 halFormatter_Get_Current_DLPLink_Pulse(void)
{
	return m_ucCurrent_DLPLINK_Pulse;
}

eRESULT halFormatter_DISP_FreezeSet(UINT8 cFreeze)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Write(CMDF_FREEZE, &cFreeze);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):FreezeSet %d\r\n", __FUNCTION__, __LINE__,cFreeze);
    return eResult;
}

eRESULT halFormatter_DISP_FreezeGet(UINT8 *pcFreeze)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dv442x_Read(CMDF_FREEZE, pcFreeze);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):FreezeGet %d\r\n", __FUNCTION__, __LINE__,*pcFreeze);
    return eResult;
}

eRESULT halFormatter_DB_SetBorderCfg(UINT16 ucScalerOutHS,UINT16 ucScalerOutVS,UINT16 ucScalerOutHW,UINT16 ucScalerOutVW)
{
    eRESULT   eResult = rcINVALID;
    UINT8     ucDatax[8] = {0};
    UINT16    Offset = 0;

    Offset = ucScalerOutVW / 4;

    //ucDatax[0] = (ucScalerOutVS + Border_Offset) >> 8;
    //ucDatax[1] = (ucScalerOutVS + Border_Offset) & 0x00FF;
    //ucDatax[2] = (ucScalerOutVS + ucScalerOutVW - Border_Offset - Offset) >> 8;
    //ucDatax[3] = (ucScalerOutVS + ucScalerOutVW - Border_Offset - Offset) & 0x00FF;
    //ucDatax[4] = (ucScalerOutHS + Border_Offset) >> 8;
    //ucDatax[5] = (ucScalerOutHS + Border_Offset) & 0x00FF;
    //ucDatax[6] = (ucScalerOutHS + ucScalerOutHW - Border_Offset) >> 8;
    //ucDatax[7] = (ucScalerOutHS + ucScalerOutHW - Border_Offset) & 0x00FF;
    cmdPutHalfword(ucScalerOutVS + Border_Offset,&ucDatax[0]);                          //ZU860_Doulas_0046
    cmdPutHalfword(ucScalerOutVS + ucScalerOutVW - Border_Offset - Offset,&ucDatax[2]); //ZU860_Doulas_0046
    cmdPutHalfword(ucScalerOutHS + Border_Offset,&ucDatax[4]);                          //ZU860_Doulas_0046
    cmdPutHalfword(ucScalerOutHS + ucScalerOutHW - Border_Offset,&ucDatax[6]);          //ZU860_Doulas_0046

    eResult = dv442x_Write(CMDF_DBBORDERCONF, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_SetBorderCfg(%d,%d,%d,%d) \r\n", __FUNCTION__, __LINE__,ucScalerOutHS,ucScalerOutVS,ucScalerOutHW,ucScalerOutVW);
    return eResult;
}

eRESULT halFormatter_DB_SetClipPixels(UINT16 ucClipPixels)  //A70LK_Doulas_0006
{
    eRESULT   eResult = rcINVALID;
    UINT8     ucDatax[2] = {0};

    cmdPutHalfword(ucClipPixels,&ucDatax[0]);

    eResult = dv442x_Write(CMDF_DBCLIPPIXELS, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DB_SetClipPixels(%d) \r\n", __FUNCTION__, __LINE__,ucClipPixels);
    return eResult;
}

eRESULT halFormatter_IsColorWheelSpinning(BOOL *cSpin)
{
    eRESULT   eResult = rcINVALID;
    UINT8 ucDatax[2] = {0,0};

    eResult = dv442x_Read(CMDF_CWSPINNINGCONTROL, ucDatax);

    if(ucDatax[0] && ucDatax[1])
        *cSpin = TRUE;
    else
        *cSpin = FALSE;

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):ColorWheelSpinning %d\r\n", __FUNCTION__, __LINE__,*cSpin);
    return eResult;
}

eRESULT halFormatter_HSGSet(UINT8* hsgdata)
{
    eRESULT  eResult = rcINVALID;
    UINT8 i = 0;

    eResult = dv442x_Write(CMDF_IMAPPLYHSG, hsgdata);

    for(i = 0; i< 42;)
    {
        LOG_MSG(db_HAL_FORMATTER, "(%02x%02x)(%02x%02x)(%02x%02x)\r\n",hsgdata[i+1],hsgdata[i],hsgdata[i+3],hsgdata[i+2],hsgdata[i+5],hsgdata[i+4]);
        i += 6;
    }

    return eResult;
}

eRESULT halFormatter_HSGGet(void) //T100_Casper_0087
{
    eRESULT  eResult = rcINVALID;
	UINT8 datax[42];
    UINT8 i = 0;

    eResult = dv442x_Read(CMDF_IMAPPLYHSG, datax);

    for(i = 0; i< 42;)
    {
        LOG_MSG(db_HAL_FORMATTER, "HSG Get(%02x%02x)(%02x%02x)(%02x%02x)\r\n",datax[i+1],datax[i],datax[i+3],datax[i+2],datax[i+5],datax[i+4]);
        i += 6;
    }

    return eResult;
}

INT16 halFormatter_HSGdata_Trans(INT8 cHSG_OSDvalue , BOOL bHue)
{
    UINT16 cNewValue = 0 ;
    INT16 iOSDvalue = (INT16)cHSG_OSDvalue ;

    if(bHue)
    {
        cNewValue = (INT16)(0x0000 + ((long)iOSDvalue  * 0x4000 / 127));  // (read from Composer) Max : 1.00 , Min : -1.0 , step :
    }
    else
    {
        cNewValue = (INT16)(0x4000 + ((long)iOSDvalue  * 0x4000 / 127)) ;  // (read from Composer) Max : 2.00 , Min : 0.00 , step :

        if(cNewValue == 0x8000)
        {
            cNewValue = 0x7fff;    //make sure < 2.0
        }
        else if(cNewValue == 0)
        {
            cNewValue = 0x0001;    //make sure > 0
        }
    }
    return cNewValue ;
}

eRESULT halFormatter_HSGValueSet(sHSG_SETTING hsgdata)  //A70LV_Doulas_0015
{
    eRESULT eResult =rcSUCCESS;
    UINT8 buf[42];
    INT8 cVal = -(INT8)INI_HSG_RED_HUE;

//ZU860_John_0016 start fix hsg command
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_W_R_GAIN , 0),   &buf[36]);  //White Red Gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_W_G_GAIN , 0),   &buf[38]);  //White Green Gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_W_B_GAIN , 0),   &buf[40]);  //White Blue Gain

    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_R_GAIN , 0),     &buf[ 0]);  //red gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_R_SAT , 0),      &buf[ 2]);  //red Saturation
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_R_HUE , 1),      &buf[ 4]);  //red Hue
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_G_GAIN , 0),     &buf[ 6]);  //Green gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_G_SAT , 0),      &buf[ 8]);  //Green Saturation
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_G_HUE , 1),      &buf[10]);  //Green Hue
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_B_GAIN , 0),     &buf[12]);  //Blue gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_B_SAT , 0),      &buf[14]);  //Blue Saturation
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_B_HUE , 1),      &buf[16]);  //Blue Hue
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_C_GAIN , 0),     &buf[18]);  //Cyan gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_C_SAT , 0),      &buf[20]);  //Cyan Saturation
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_C_HUE , 1),      &buf[22]);  //Cyan Hue
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_M_GAIN , 0),     &buf[24]);  //Magenta gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_M_SAT , 0),      &buf[26]);  //Magenta Saturation
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_M_HUE , 1),      &buf[28]);  //Megenta Hue
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_Y_GAIN , 0),     &buf[30]);  //yellow gain
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_Y_SAT , 0),      &buf[32]);  //yellow Saturation
    cmdPutHalfword(halFormatter_HSGdata_Trans(cVal + (INT8)hsgdata.HSG_Y_HUE , 1),      &buf[34]);  //yellow Hue
//ZU860_John_0016 end

    eResult = halFormatter_HSGSet(buf);

    return     eResult;
}

eRESULT halFormatter_CCADataSend(UINT8 *data, UINT16 wSize)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 datax[96] = {0};
    UINT8 i = 0;

    if(data == NULL)
    {
        return rcERROR;
    }

    for(i=0; i < CCA_D_MAILBOX_SIZE ;i++)
    {
        datax[i+54] = data[i];//CCADesired
    }

    // Fill Measure data R G B W C1 C2, 6x3x2=36
    for(i=0; i < CCA_M_MAILBOX_SIZE ;i++)
    {
        datax[i] = data[i+42];//CCAMeasured
    }

    // Fill Measure data DRA_A DRA_B DRA_C, 3x3x2=18
    for(i=36; i < 54 ;i++)
    {
        datax[i] = 0;//not use
    }

    eResult = dv442x_Write(CMDF_IMAGECCA, datax);

    LOG_MSG(db_HAL_FORMATTER, "CCADataSend(%d) \r\n",wSize);
    /*for(i=1; i < 97 ;)
    {
        dbPrintf(dbFormatter, "%d(%2x%2x,%2x%2x,%2x%2x)\r\n",i,datax[i],datax[i+1],datax[i+2],datax[i+3],datax[i+4],datax[i+5]);
        i += 6;
    }*/
    return eResult;
}

eRESULT halFormatter_CCADataGet(UINT8 *data)
{
    eRESULT eResult = rcINVALID;
    UINT8 datax[98], i=0;

    eResult = dv442x_Read(CMDF_IMAGECCA, datax);

    for(i=0; i < 96 ;i++)
        data[i] = datax[i];

    /*for(i=2; i < 98 ;)
    {
        dbPrintf(dbFormatter, "%d(%2x%2x,%2x%2x,%2x%2x)\r\n",i,datax[i],datax[i+1],datax[i+2],datax[i+3],datax[i+4],datax[i+5]);
        i += 6;
    }*/

    return eResult;
}

eRESULT halFormatter_HSG_UnitGainSet(void)
{
    UINT8 buf[42];

    buf[0] = 0x40 ;  //red gain
    buf[1] = 0x00 ;  //red gain
    buf[2] = 0x40;   //red Saturation
    buf[3] = 0x00 ;    //red Saturation
    buf[4] = 0x00 ;   //red Hue
    buf[5] = 0x00 ;    //red Hue
    buf[6] = 0x40;  //Green gain
    buf[7] = 0x00;  //Green gain
    buf[8] = 0x40;  //Green Saturation
    buf[9] = 0x00;  //Green Saturation
    buf[10] = 0x00 ;  //Green Hue
    buf[11] = 0x00 ;  //Green Hue
    buf[12] = 0x40;  //Blue gain
    buf[13] = 0x00;  //Blue gain
    buf[14] = 0x40 ;  //Blue Saturation
    buf[15] = 0x00;  //Blue Saturation
    buf[16] = 0x00;  //Blue Hue
    buf[17] = 0x00;  //Blue Hue
    buf[18] = 0x40 ;  //Cyan gain
    buf[19] = 0x00 ;  //Cyan gain
    buf[20] = 0x40 ;  //Cyan Saturation
    buf[21] = 0x00 ;  //Cyan Saturation
    buf[22] = 0x00;  //Cyan Hue
    buf[23] = 0x00 ;  //Cyan Hue
    buf[24] = 0x40 ;  //Megenta gain
    buf[25] = 0x00 ;  //Megenta gain
    buf[26] = 0x40 ;  //Megenta Saturation
    buf[27] = 0x00 ;  //Megenta Saturation
    buf[28] = 0x00 ;  //Megenta Hue
    buf[29] = 0x00 ;  //Megenta Hue
    buf[30] = 0x40 ;  //yellow gain
    buf[31] = 0x00 ;  //yellow gain
    buf[32] = 0x40 ; //yellow Saturation
    buf[33] = 0x00 ;  //yellow Saturation
    buf[34] = 0x00 ;  //yellow Hue
    buf[35] = 0x00 ;  //yellow Hue
    buf[36] = 0x40; // 37-th BYTE White Red Gain
    buf[37] = 0x00; // 38-th BYTE White Red Gain
    buf[38] = 0x40; // 39-th BYTE White Green Gain
    buf[39] = 0x00; // 40-th BYTE White Green Gain
    buf[40] = 0x40; // 41-th BYTE White Blue Gain
    buf[41] = 0x00; // 42-th BYTE White Blue Gain

    return halFormatter_HSGSet(buf);
}

eRESULT halFormatter_SYS_SystemStatusGet(UINT8 *pcStatus)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_SYSTEMSTATUS, pcStatus);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):SystemStatusGet(%x,%x,%x,%x)\r\n", __FUNCTION__, __LINE__,pcStatus[0],pcStatus[1],pcStatus[2],pcStatus[3]);

    return eResult;
}

eRESULT halFormatter_DISP_BlackLevelSet(UINT8 bValue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dv442x_Write(CMDF_LIMIT_BLACK_LEVEL, &bValue);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BlackLevelSet %d\r\n", __FUNCTION__, __LINE__,bValue);
    return eResult;
}

eRESULT halFormatter_DISP_BlackLevelGet(UINT8 *bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_LIMIT_BLACK_LEVEL, bValue);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BlackLevelGet %d\r\n", __FUNCTION__, __LINE__,*bValue);
    return eResult;
}

eRESULT halFormatter_DISP_BlackLevelThresholdSet(UINT8 bValue)
{
    eRESULT eResult = rcSUCCESS;

    eResult = dv442x_Write(CMDF_LIMIT_BLACK_TOLERANCE, &bValue);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BlackLevelThresholdSet %d\r\n", __FUNCTION__, __LINE__,bValue);
    return eResult;
}

eRESULT halFormatter_DISP_BlackLevelThresholdGet(UINT8 *bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_LIMIT_BLACK_TOLERANCE, bValue);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d):BlackLevelThresholdGet %d\r\n", __FUNCTION__, __LINE__,*bValue);
    return eResult;
}

eRESULT halFormatter_DISP_BlackStatusSet(UINT8 bValue) //T100_Casper_0047
{

    eRESULT  eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = bValue;
    eResult = dv442x_Write(CMDF_LIMIT_BLACK_STATUS, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BlackLevelThresholdSet %d\r\n", __FUNCTION__, __LINE__,bValue);

    return eResult;
}

eRESULT halFormatter_DISP_BlackPowerStatusGet(UINT8 *bValue)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_LIMIT_BLACK_STATUS, bValue);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BlackPowerStatusGet %d\r\n", __FUNCTION__, __LINE__,*bValue);
    return eResult;
}

eRESULT halFormatter_ILL_AuxSyncSet( BOOL bEnable )
{
    eRESULT eResult = rcINVALID;
    UINT8     ucDatax[2] = {0};

    ucDatax[0] = bEnable;

    eResult = dv442x_Write(CMDF_LAMPENABLE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AuxSyncSet %d\r\n", __FUNCTION__, __LINE__,bEnable);
    return eResult;
}

eRESULT halFormatter_ILL_AuxSyncGet( BOOL *pbEnable )
{
    eRESULT eResult = rcINVALID;
    UINT8     ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_LAMPENABLE, ucDatax);

    *pbEnable = (BOOL)ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AuxSyncGet %d\r\n", __FUNCTION__, __LINE__,ucDatax[0]);
    return eResult;
}

eRESULT halFormatter_ILL_AuxSyncTypeSet(UINT8 *pcSyncType)
{
    eRESULT eResult = rcINVALID;

#ifdef PLATFORM_H60_2K
    eResult = dv442x_Write(CMDF_LAMPSYNCTYPE, pcSyncType);
#endif /* PLATFORM_H60_2K */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AuxSyncTypeSet %d,%d,%d\r\n", __FUNCTION__, __LINE__,*pcSyncType,*(pcSyncType+1),*(pcSyncType+2));

    return eResult;
}

eRESULT halFormatter_ILL_AuxSyncTypeGet(UINT8 *pcSyncType)
{
    eRESULT eResult = rcINVALID;

#ifdef PLATFORM_H60_2K
    eResult = dv442x_Read(CMDF_LAMPSYNCTYPE, pcSyncType);
#endif /* PLATFORM_H60_2K */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AuxSyncTypeGet %d,%d,%d\r\n", __FUNCTION__, __LINE__,*pcSyncType,*(pcSyncType+1),*(pcSyncType+2));
    return eResult;
}

eRESULT halFormatter_ILL_BoostRLDEnableSet(UINT8 cEn)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Write(CMDF_LAMPENABLE, &cEn);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AuxSyncGet %d\r\n", __FUNCTION__, __LINE__,cEn);
    return eResult;
}

eRESULT halFormatter_ILL_BoostRLDEnableGet(UINT8 *pcEn)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_LAMPENABLE, pcEn);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):AuxSyncGet %d\r\n", __FUNCTION__, __LINE__,*pcEn);
    return eResult;
}

eRESULT halFormatter_DMD_BGDisplayModeSet(UINT8 cMode)
{
    eRESULT eResult = rcINVALID;

    // A70LV_Eric.C_0002 Start
    UINT8 cData[2] = {0};

    switch(cMode)
    {
        case BG_BLK:
        case BG_WHT:
        case BG_GEN:
        case BG_RED:
        case BG_BLU:
        case BG_YLO:
        case BG_CYN:
        case BG_MGT:
            cData[0] = 0x00;    //Insertion Mode: FULL SCREEN
            cData[1] = (UINT8)cMode;    //Color
            break;

        case BG_DISABLE:
        default:
            cData[0] = 0x01;    //Insertion Mode: BORDER
            cData[1] = 0x00;    //Color
            break;
    }

    eResult = dv442x_Write(CMDF_BGCOLORMODE, cData);
    // A70LV_Eric.C_0002 End
//    eResult = dv442x_Write(CMDF_BGCOLORMODE, &cMode);   // A70LV_Eric.C_0002 mask

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BGDisplayModeSet %d, eResult=%d\r\n", __FUNCTION__, __LINE__,cMode,eResult);
    return eResult;
}

eRESULT halFormatter_DMD_BGDisplayModeGet(UINT8 *pcMode)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_BGCOLORMODE, pcMode);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):BGDisplayModeGet %d\r\n", __FUNCTION__, __LINE__,*pcMode);
    return eResult;
}

eRESULT halFormatter_SensorTimingGet(UINT16 *SensorStartDelay, UINT16 *RedSensorTiming ,UINT16 *GreenSensorTiming,UINT16 *BlueSensorTiming,UINT16 *YellowSensorTiming)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    eResult = dv442x_Read(CMDF_SSISENSORTIMING, ucDatax);

    cmdGetHalfword(SensorStartDelay,&ucDatax[0]);       //A70LV_Doulas_0028 modify
    cmdGetHalfword(RedSensorTiming,&ucDatax[2]);
    cmdGetHalfword(GreenSensorTiming,&ucDatax[4]);
    cmdGetHalfword(BlueSensorTiming,&ucDatax[6]);
    cmdGetHalfword(YellowSensorTiming,&ucDatax[8]);

    LOG_MSG(db_HAL_FORMATTER,"R SensorTiming = %d (%02x,%02x)(%02x,%02x)(%02x,%02x)(%02x,%02x)(%02x,%02x)\r\n",
                            eResult, ucDatax[0], ucDatax[1], ucDatax[2], ucDatax[3],
                            ucDatax[4], ucDatax[5], ucDatax[6], ucDatax[7],ucDatax[8], ucDatax[9]);
    return eResult;
}

eRESULT halFormatter_SensorTimingSet(UINT16 SensorStartDelay, UINT16 RedSensorTiming ,UINT16 GreenSensorTiming,UINT16 BlueSensorTiming,UINT16 YellowSensorTiming)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    cmdPutHalfword(SensorStartDelay,&ucDatax[0]);       //ZU860_Doulas_0046 modify
    cmdPutHalfword(RedSensorTiming,&ucDatax[2]);        //ZU860_Doulas_0046 modify
    cmdPutHalfword(GreenSensorTiming,&ucDatax[4]);      //ZU860_Doulas_0046 modify
    cmdPutHalfword(BlueSensorTiming,&ucDatax[6]);       //ZU860_Doulas_0046 modify
    cmdPutHalfword(YellowSensorTiming,&ucDatax[8]);     //ZU860_Doulas_0046 modify

    eResult = dv442x_Write(CMDF_SSISENSORTIMING, ucDatax);

    LOG_MSG(db_HAL_FORMATTER,"W SensorTiming = %d (%u)(%u)(%u)(%u)(%u)\r\n",
                            eResult, SensorStartDelay, RedSensorTiming,
                            GreenSensorTiming, BlueSensorTiming,YellowSensorTiming);
    return eResult;
}

eRESULT halFormatter_SensorDataGet(UINT16 *RedSensorData ,UINT16 *GreenSensorData,UINT16 *BlueSensorData,UINT16 *YellowSensorData)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[12]={0,0,0,0,0,0,0,0,0,0,0,0};

    eResult = dv442x_Read(CMDF_SSISENSOR, ucDatax);

    cmdGetHalfword(RedSensorData,&ucDatax[0]);       //A70LV_Doulas_0028 modify
    cmdGetHalfword(GreenSensorData,&ucDatax[2]);
    cmdGetHalfword(BlueSensorData,&ucDatax[4]);
    cmdGetHalfword(YellowSensorData,&ucDatax[6]);

    LOG_MSG(db_HAL_FORMATTER,"R SensorData = %d (%02x,%02x)(%02x,%02x)(%02x,%02x)(%02x,%02x)(%02x,%02x)\r\n",
                            eResult, ucDatax[0], ucDatax[1], ucDatax[2], ucDatax[3],
                            ucDatax[4], ucDatax[5], ucDatax[6], ucDatax[7],ucDatax[8], ucDatax[9]);
    return eResult;
}

eRESULT halFormatter_ImageRGBGainGet(UINT16 *CSCRedGain ,UINT16 *CSCGreenGain,UINT16 *CSCBlueGain)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[8]={0,0,0,0,0,0,0,0};

    eResult = dv442x_Read(CMDF_IMAGERGBGAIN, ucDatax);

    cmdGetHalfword(CSCRedGain,&ucDatax[0]);      //A70LV_Doulas_0028 modify
    cmdGetHalfword(CSCGreenGain,&ucDatax[2]);
    cmdGetHalfword(CSCBlueGain,&ucDatax[4]);

    LOG_MSG(db_HAL_FORMATTER,"R RGBGain = %d (%02x,%02x)(%02x,%02x)(%02x,%02x)\r\n",
                            eResult, ucDatax[0], ucDatax[1], ucDatax[2], ucDatax[3],
                            ucDatax[4], ucDatax[5]);
    return eResult;
}

eRESULT halFormatter_ImageRGBGainSet(UINT16 CSCRedGain ,UINT16 CSCGreenGain,UINT16 CSCBlueGain)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[8]={0,0,0,0,0,0,0,0};

    cmdPutHalfword(CSCRedGain,&ucDatax[0]);     //ZU860_Doulas_0046 modify
    cmdPutHalfword(CSCGreenGain,&ucDatax[2]);   //ZU860_Doulas_0046 modify
    cmdPutHalfword(CSCBlueGain,&ucDatax[4]);    //ZU860_Doulas_0046 modify

    eResult = dv442x_Write(CMDF_IMAGERGBGAIN, ucDatax);

    LOG_MSG(db_HAL_FORMATTER,"W SensorTiming = %d (%u)(%u)(%u)\r\n",
                            eResult, CSCRedGain, CSCGreenGain,CSCBlueGain);
    return eResult;
}

eRESULT halFormatter_DynamicBlackUsePWMGet(UINT16 *PWM_FULL, UINT16 *PWM_ECO)
{
    eRESULT eResult = rcINVALID;
#if defined(PLATFORM_A70G2) || defined(PLATFORM_A35G2)//HICC2_Doulas_0001 no using
    UINT8 ucDatax[24]={0};

    eResult = dv442x_Read(CMDF_DB_USE_PWM, ucDatax);

    cmdGetHalfword(&PWM_FULL[0],&ucDatax[0]);     //A70LV_Doulas_0028 modify
    cmdGetHalfword(&PWM_FULL[1],&ucDatax[2]);
    cmdGetHalfword(&PWM_FULL[2],&ucDatax[4]);
    cmdGetHalfword(&PWM_FULL[3],&ucDatax[6]);
    cmdGetHalfword(&PWM_FULL[4],&ucDatax[8]);
    //cmdGetHalfword(&PWM_FULL[5],&ucDatax[10]);  //ZU860_Doulas_0046

    cmdGetHalfword(&PWM_ECO[0],&ucDatax[10]);       //ZU860_Doulas_0046 modify
    cmdGetHalfword(&PWM_ECO[1],&ucDatax[12]);
    cmdGetHalfword(&PWM_ECO[2],&ucDatax[14]);
    cmdGetHalfword(&PWM_ECO[3],&ucDatax[16]);
    cmdGetHalfword(&PWM_ECO[4],&ucDatax[18]);
    //cmdGetHalfword(&PWM_ECO[5],&ucDatax[22]);

    LOG_MSG(db_HAL_FORMATTER,"R PWMfe = %d (%u,%u,%u,%u,%u)(%u,%u,%u,%u,%u)\r\n",
                            eResult, PWM_FULL[0], PWM_FULL[1], PWM_FULL[2], PWM_FULL[3],PWM_FULL[4],
                            PWM_ECO[0],PWM_ECO[1],PWM_ECO[2],PWM_ECO[3],PWM_ECO[4]);
#endif
    return eResult;
}

eRESULT halFormatter_DynamicBlackUsePWMSet(UINT16 *PWM_FULL, UINT16 *PWM_ECO)
{
    eRESULT eResult = rcINVALID;
#if defined(PLATFORM_A70G2) || defined(PLATFORM_A35G2)//HICC2_Doulas_0001 no using
    UINT8 ucDatax[24]={0};

    cmdPutHalfword(PWM_FULL[0],&ucDatax[0]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_FULL[1],&ucDatax[2]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_FULL[2],&ucDatax[4]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_FULL[3],&ucDatax[6]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_FULL[4],&ucDatax[8]);    //ZU860_Doulas_0046

    cmdPutHalfword(PWM_ECO[0],&ucDatax[10]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_ECO[1],&ucDatax[12]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_ECO[2],&ucDatax[14]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_ECO[3],&ucDatax[16]);    //ZU860_Doulas_0046
    cmdPutHalfword(PWM_ECO[4],&ucDatax[18]);    //ZU860_Doulas_0046

    eResult = dv442x_Write(CMDF_DB_USE_PWM, ucDatax);

    LOG_MSG(db_HAL_FORMATTER,"W PWMfe = %d (%u,%u,%u,%u,%u)(%u,%u,%u,%u,%u)\r\n",
                            eResult, PWM_FULL[0], PWM_FULL[1], PWM_FULL[2], PWM_FULL[3],PWM_FULL[4],
                            PWM_ECO[0],PWM_ECO[1],PWM_ECO[2],PWM_ECO[3],PWM_ECO[4]);
#endif
    return eResult;
}

eRESULT halFormatter_DynamicBlackTableGet(UINT8 *DA_Table)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[5]={0,0,0,0,0};

    eResult = dv442x_Read(CMDF_DBAPERTUREINDEX, Datax);

    *DA_Table = Datax[3];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DynamicBlackTableGet %d\r\n", __FUNCTION__, __LINE__,*DA_Table);
    return eResult;
}

eRESULT halFormatter_DynamicBlackTableSet(UINT8 DA_Table)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[5]={0,0,0,0,0};

    Datax[0] = 0;
    Datax[1] = 0;
    Datax[2] = 0;
    Datax[3] = DA_Table;

    eResult = dv442x_Write(CMDF_DBAPERTUREINDEX, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DynamicBlackTableSet %d\r\n", __FUNCTION__, __LINE__,DA_Table);
    return eResult;
}

eRESULT halFormatter_BCCalibrateSet(BOOL bEnable,UINT32 dRed,UINT32 dGreen,UINT32 dBlue,UINT32 dYellow)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[56];
    UINT8 i=0;
    for(i=0;i <56 ;i++)
        Datax[i] = 0;

    if(bEnable)
    {
        Datax[0] = (UINT8)(BC_CALIBRATE_ENABLE & 0xFF);
        Datax[1] = (UINT8)((BC_CALIBRATE_ENABLE >>8) & 0xFF);
        Datax[2] = (UINT8)((BC_CALIBRATE_ENABLE >>16) & 0xFF);
        Datax[3] = (UINT8)((BC_CALIBRATE_ENABLE >>24) & 0xFF);
    }
    else
    {
        Datax[0] = (UINT8)(BC_CALIBRATE_DISABLE & 0xFF);
        Datax[1] = (UINT8)((BC_CALIBRATE_DISABLE >>8) & 0xFF);
        Datax[2] = (UINT8)((BC_CALIBRATE_DISABLE >>16) & 0xFF);
        Datax[3] = (UINT8)((BC_CALIBRATE_DISABLE >>24) & 0xFF);
    }

    Datax[24] = (UINT8)(dGreen & 0xFF);
    Datax[25] = (UINT8)((dGreen >>8) & 0xFF);
    Datax[26] = (UINT8)((dGreen >>16) & 0xFF);
    Datax[27] = (UINT8)((dGreen >>24) & 0xFF);

    Datax[28] = (UINT8)(dRed & 0xFF);
    Datax[29] = (UINT8)((dRed >>8) & 0xFF);
    Datax[30] = (UINT8)((dRed >>16) & 0xFF);
    Datax[31] = (UINT8)((dRed >>24) & 0xFF);

    Datax[32] = (UINT8)(dBlue & 0xFF);
    Datax[33] = (UINT8)((dBlue >>8) & 0xFF);
    Datax[34] = (UINT8)((dBlue >>16) & 0xFF);
    Datax[35] = (UINT8)((dBlue >>24) & 0xFF);

    Datax[44] = (UINT8)(dYellow & 0xFF);
    Datax[45] = (UINT8)((dYellow >>8) & 0xFF);
    Datax[46] = (UINT8)((dYellow >>16) & 0xFF);
    Datax[47] = (UINT8)((dYellow >>24) & 0xFF);

    eResult = dv442x_Write(CMDF_BCCALIBRATE, Datax);

    LOG_MSG(db_HAL_FORMATTER, "Write BCCalibrate = %d(%x,%x,%x,%x)\r\n",bEnable,dRed,dGreen,dBlue ,dYellow);
    return eResult;
}

eRESULT halFormatter_SegmentColorSet(UINT8 cSegmentColorEnabled,UINT8 cSegmentColor)
{
    eRESULT eResult = rcINVALID;
#if defined(PLATFORM_A70G2) || defined(PLATFORM_A35G2)//HICC2_Doulas_0001
    UINT8 Datax[3]={0,0,0};

    Datax[0] = cSegmentColorEnabled;
    Datax[1] = cSegmentColor;

    eResult = dv442x_Write(CMDF_SEGMENTCOLOR, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):SegmentColorSet %d,%d\r\n", __FUNCTION__, __LINE__,cSegmentColorEnabled,cSegmentColor);
#endif
    return eResult;
}

eRESULT halFormatter_SegmentColorGet(UINT8 *cSegmentColorEnabled,UINT8 *cSegmentColor)
{
    eRESULT eResult = rcINVALID;
#if defined(PLATFORM_A70G2) || defined(PLATFORM_A35G2)//HICC2_Doulas_0001
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_SEGMENTCOLOR, Datax);

    *cSegmentColorEnabled = Datax[0];
    *cSegmentColor = Datax[1];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):SegmentColorGet %d,%d\r\n", __FUNCTION__, __LINE__,*cSegmentColorEnabled,*cSegmentColor);
#endif
    return eResult;
}

eRESULT halFormatter_DBSpeedSet(UINT16 cDB_Speed)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[3]={0,0,0};

    //ucDatax[0] = MSB(cDB_Speed);      //A70LV_Doulas_0028 modify
    //ucDatax[1] = LSB(cDB_Speed);
    cmdPutHalfword(cDB_Speed,&ucDatax[0]);  //ZU860_Doulas_0046 modify

    eResult = dv442x_Write(CMDF_DBSPEED, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DBSpeedSet %d\r\n", __FUNCTION__, __LINE__,cDB_Speed);
    return eResult;
}

eRESULT halFormatter_DBSpeedGet(UINT16 *cDB_Speed)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_DBSPEED, ucDatax);   //A70LV_Doulas_0028 modify

    cmdGetHalfword(cDB_Speed,&ucDatax[0]);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):DBSpeedGet %d\r\n", __FUNCTION__, __LINE__,*cDB_Speed);
    return eResult;
}

eRESULT halFormatter_ALC_RGB_LevelGet(UINT16 *RGBLevel)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[12]={0};

    eResult = dv442x_Read(CMDF_ALC_RGB_LEVEL, Datax);

    cmdGetHalfword(&RGBLevel[0],&Datax[0]);
    cmdGetHalfword(&RGBLevel[1],&Datax[2]);
    cmdGetHalfword(&RGBLevel[2],&Datax[4]);
    cmdGetHalfword(&RGBLevel[3],&Datax[6]);
    cmdGetHalfword(&RGBLevel[4],&Datax[8]);
    cmdGetHalfword(&RGBLevel[5],&Datax[10]);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):RGB_LevelGet %d,%d,%d,%d,%d,%d\r\n", __FUNCTION__, __LINE__,RGBLevel[0],RGBLevel[1],RGBLevel[2],RGBLevel[3],RGBLevel[4],RGBLevel[5]);   //ZU860_Doulas_0047

	return eResult;
}



eRESULT halFormatter_CWIndexClockGet(UINT32 *dClockPeriod,UINT32 *dCW1_Frequence)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[8]={0};

    eResult = dv442x_Read(CMDF_CWINDEXCLOCK, ucDatax);

    //*dClockPeriod = ((UINT32)ucDatax[0]<<24)+ ((UINT32)ucDatax[1]<<16)+ ((UINT32)ucDatax[2]<<8)+(UINT32)ucDatax[3];     //A70LV_Doulas_0028 modify
    //*dCW1_Frequence = ((UINT32)ucDatax[4]<<24)+ ((UINT32)ucDatax[5]<<16)+ ((UINT32)ucDatax[6]<<8)+(UINT32)ucDatax[7];
    cmdGetWord(dClockPeriod,&ucDatax[0]);       //ZU860_Doulas_0046
    cmdGetWord(dCW1_Frequence,&ucDatax[4]);     //ZU860_Doulas_0046

    LOG_MSG(db_HAL_FORMATTER, "CWIndexClockGet =(%x,%x,%x,%x)(%x,%x,%x,%x)\r\n",
                        ucDatax[0],ucDatax[1],ucDatax[2],ucDatax[3],
                        ucDatax[4],ucDatax[5],ucDatax[6],ucDatax[7]);
    return eResult;
}

//3D SYNC Selection:
// [0] : 3D sync input
// [1] : MST8535 3D sync output
// [2] : None 3D SYNC (Auto sync off)
eRESULT halFormatter_3D_SYNC_SelectionSet(UINT8 c3D_SYNC_Selection)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

#ifndef FRAME_SEQUENTIAL_3D_ULTRA
    Datax[0] = c3D_SYNC_Selection;

    eResult = dv442x_Write(CMDF_3DSYNCINPUTSELECT, Datax);    //ZU860_John_0013 sync with DDP API 9.0 //A70LV_Doulas_0154 modify //A70LV_Doulas_0028
    #ifdef SCALER_C341 //H30K_Doulas_0007
    if(c3D_SYNC_Selection == e3D_SYNC_INPUT_SELECTION_C821_OUTPUT_3D_SYNC)
    {
        halFormatter_Extern_3D_SYNC_In_Double_Set(0);
    }
    else
    {
        if(halScaler_Input_3D_Format_Config_Get() == eINPUT_3D_TYPE_FRAME_SEQUENTIAL)
        {
            halFormatter_Extern_3D_SYNC_In_Double_Set(0); //3D sync input 120hz bypass
        }
        else
        {
            halFormatter_Extern_3D_SYNC_In_Double_Set(1); //3D sync input 60hz ,double 3D sync

        }
    }
    #endif
#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D_SYNC_SelectionSet %d\r\n", __FUNCTION__, __LINE__,c3D_SYNC_Selection);
    return eResult;
}

eRESULT halFormatter_3D_SYNC_SelectionGet(UINT8 *c3D_SYNC_Selection)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_3DSYNCINPUTSELECT, Datax);     //ZU860_John_0013 sync with DDP API 9.0  //A70LV_Doulas_0154 modify //A70LV_Doulas_0028 modify

    *c3D_SYNC_Selection = Datax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D_SYNC_SelectionGet %d\r\n", __FUNCTION__, __LINE__,*c3D_SYNC_Selection);
    return eResult;
}

//3D SYNC Out:
// [0] : To Emitter
// [1] : To Next Projector
eRESULT halFormatter_3D_SYNC_OutSelectionSet(UINT8 c3D_SYNC_Out)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    Datax[0] = c3D_SYNC_Out;

#ifndef FRAME_SEQUENTIAL_3D_ULTRA
    eResult = dv442x_Write(CMDF_3DSYNCOUTPUTSELECT, Datax);    //ZU860_John_0013 sync with DDP API 9.0  //A70LV_Doulas_0028 modify
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D_SYNC_OutSelectionSet %d\r\n", __FUNCTION__, __LINE__,c3D_SYNC_Out);
    return eResult;
}

eRESULT halFormatter_3D_SYNC_OutSelectionGet(UINT8 *c3D_SYNC_Out)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_3DSYNCOUTPUTSELECT, Datax);    //ZU860_John_0013 sync with DDP API 9.0   //A70LV_Doulas_0028 modify

    *c3D_SYNC_Out = Datax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D_SYNC_OutSelectionGet %d\r\n", __FUNCTION__, __LINE__,*c3D_SYNC_Out);
    return eResult;
}

eRESULT halFormatter_3D_SYNC_InputMonitorGet(BOOL *b3DSyncAvaliable,BOOL *b3DSyncLost,UINT8 *cdatapath_state)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[4]={0,0,0,0};

#ifndef FRAME_SEQUENTIAL_3D_ULTRA//#ifdef PLATFORM_H60_2K //H30K_Doulas_0002
    #ifdef CONFIG_4K_DISPLAY  //H30K_Doulas_0002 Modify
    eResult = dv442x_Read(CMDF_3DSYNCINPUT_MONITOR_FANNUMSELECT, Datax);
    #else
    eResult = dv442x_Read(CMDF_3DSYNCINPUT_MONITOR, Datax);  //ZU860_John_0013 sync with DDP API 9.0
    #endif

    *b3DSyncAvaliable = (BOOL)Datax[0];
    *b3DSyncLost = (BOOL)Datax[1];
    *cdatapath_state = Datax[2];
#endif /* PLATFORM_H60_2K */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):3D_SYNC_InputMonitorGet (%d,%d,%d)\r\n", __FUNCTION__, __LINE__,*b3DSyncAvaliable,*b3DSyncLost,*cdatapath_state);
    return eResult;
}
//A70LV_Doulas_0006 end

eRESULT halFormatter_3D_ViewingModeSet(UINT8 cViewMode)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = (UINT8)cViewMode;
#ifndef FRAME_SEQUENTIAL_3D_ULTRA
    eResult = dv442x_Write(CMDF_VIEWINGMODE, ucDatax);
#endif /* FRAME_SEQUENTIAL_3D_ULTRA */

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): %d\r\n", __FUNCTION__, __LINE__,cViewMode);
    return eResult;
}

eRESULT halFormatter_3D_ViewingModeGet(UINT8 *cViewMode)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_VIEWINGMODE, ucDatax);
    *cViewMode = (BOOL)ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): %d\r\n", __FUNCTION__, __LINE__,*cViewMode);
    return eResult;
}

eRESULT halFormatter_ConstantPowerSet(UINT8 cPower) //A70LV_Larry_0025
{
    //eRESULT eResult = rcERROR;
    sLD_PWM sLD_PWM_Target = {0};
    UINT8 aucData[16] = {0};

    sLD_PWM_Target.wBLD_R = Power_Mode_Eco_BLD_R + (UINT16)((UINT32)(Power_Mode_Normal_BLD_R - Power_Mode_Eco_BLD_R)*(UINT32)(cPower - 0) /100);
    sLD_PWM_Target.wBLD_G = Power_Mode_Eco_BLD_G + (UINT16)((UINT32)(Power_Mode_Normal_BLD_G - Power_Mode_Eco_BLD_G)*(UINT32)(cPower - 0) /100);
    sLD_PWM_Target.wBLD_B = Power_Mode_Eco_BLD_B + (UINT16)((UINT32)(Power_Mode_Normal_BLD_B - Power_Mode_Eco_BLD_B)*(UINT32)(cPower - 0) /100);
    sLD_PWM_Target.wBLD_Y = Power_Mode_Eco_BLD_Y + (UINT16)((UINT32)(Power_Mode_Normal_BLD_Y - Power_Mode_Eco_BLD_Y)*(UINT32)(cPower - 0) /100);
    sLD_PWM_Target.wRLD_R = Power_Mode_Eco_RLD_R + (UINT16)((UINT32)(Power_Mode_Normal_RLD_R - Power_Mode_Eco_RLD_R)*(UINT32)(cPower - 0) /100);
    sLD_PWM_Target.wRLD_Y = Power_Mode_Eco_RLD_Y + (UINT16)((UINT32)(Power_Mode_Normal_RLD_Y - Power_Mode_Eco_RLD_Y)*(UINT32)(cPower - 0) /100);

    m_uiCurrentPWM[0] = sLD_PWM_Target.wBLD_R;
    m_uiCurrentPWM[1] = sLD_PWM_Target.wBLD_G;
    m_uiCurrentPWM[2] = sLD_PWM_Target.wBLD_B;
    m_uiCurrentPWM[3] = sLD_PWM_Target.wBLD_Y;
    m_uiCurrentPWM[4] = sLD_PWM_Target.wRLD_R;
    m_uiCurrentPWM[5] = sLD_PWM_Target.wRLD_Y;

#if 0
    aucData[0] = eLD_BANK_R1; //RLD  //A70LV_John_0070 modify hard code
    aucData[1] = (UINT8)sLD_PWM_Target.wRLD_Y;
    aucData[2] = (UINT8)(sLD_PWM_Target.wRLD_Y >> 8);
    aucData[3] = (UINT8)sLD_PWM_Target.wRLD_R;
    aucData[4] = (UINT8)(sLD_PWM_Target.wRLD_R >> 8);
    aucData[5] = 0;
    aucData[6] = 0;
    aucData[7] = 0;
    aucData[8] = 0;
    if(rcSUCCESS != halLDCtrl_LD_Power_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }

    aucData[0] = eLD_BANK_B1; //BLD1  //A70LV_John_0070 modify hard code
    aucData[1] = (UINT8)sLD_PWM_Target.wBLD_Y;
    aucData[2] = (UINT8)(sLD_PWM_Target.wBLD_Y >> 8);
    aucData[3] = (UINT8)sLD_PWM_Target.wBLD_R;
    aucData[4] = (UINT8)(sLD_PWM_Target.wBLD_R >> 8);
    aucData[5] = (UINT8)sLD_PWM_Target.wBLD_B;
    aucData[6] = (UINT8)(sLD_PWM_Target.wBLD_B >> 8);
    aucData[7] = (UINT8)sLD_PWM_Target.wBLD_G;
    aucData[8] = (UINT8)(sLD_PWM_Target.wBLD_G >> 8);
    if(rcSUCCESS != halLDCtrl_LD_Power_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }
#else
    aucData[0]  = (UINT8)sLD_PWM_Target.wBLD_Y;
    aucData[1]  = (UINT8)(sLD_PWM_Target.wBLD_Y >> 8);
    aucData[2]  = (UINT8)sLD_PWM_Target.wBLD_R;
    aucData[3]  = (UINT8)(sLD_PWM_Target.wBLD_R >> 8);
    aucData[4]  = (UINT8)sLD_PWM_Target.wBLD_B;
    aucData[5]  = (UINT8)(sLD_PWM_Target.wBLD_B >> 8);
    aucData[6]  = (UINT8)sLD_PWM_Target.wBLD_G;
    aucData[7]  = (UINT8)(sLD_PWM_Target.wBLD_G >> 8);

    aucData[8]  = (UINT8)sLD_PWM_Target.wRLD_Y;
    aucData[9]  = (UINT8)(sLD_PWM_Target.wRLD_Y >> 8);
    aucData[10] = (UINT8)sLD_PWM_Target.wRLD_R;
    aucData[11] = (UINT8)(sLD_PWM_Target.wRLD_R >> 8);
    aucData[12] = 0;
    aucData[13] = 0;
    aucData[14] = 0;
    aucData[15] = 0;

    if(rcSUCCESS != halLDCtrl_LD_PowerAll_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }
#endif /* 0 */

    return rcSUCCESS;
}

eRESULT halFormatter_PWM_Set(UINT8 cIndex, UINT16 uiValue)
{
    //eRESULT eResult = rcINVALID;
    UINT8 aucData[16] = {0};

    if(cIndex < 6)
    {
        m_uiCurrentPWM[cIndex] = uiValue;
    }
    //m_uiCurrentPWM[3] = sLD_PWM_Target.wBLD_Y;
    //m_uiCurrentPWM[0] = sLD_PWM_Target.wBLD_R;
    //m_uiCurrentPWM[2] = sLD_PWM_Target.wBLD_B;
    //m_uiCurrentPWM[1] = sLD_PWM_Target.wBLD_G;
    //m_uiCurrentPWM[5] = sLD_PWM_Target.wRLD_Y;
    //m_uiCurrentPWM[4] = sLD_PWM_Target.wRLD_R;
#if 0
#if 0
    aucData[0] = eLD_BANK_B1; //BLD1 //Y R G B  //A70LV_John_0070 modify hard code
    aucData[1] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_Y]);
    aucData[2] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_Y] >> 8);
    aucData[3] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_R]);
    aucData[4] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_R] >> 8);
    aucData[5] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_B]);
    aucData[6] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_B] >> 8);
    aucData[7] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_G]);
    aucData[8] = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_G] >> 8);
    if(rcSUCCESS != halLDCtrl_LD_Power_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }

    aucData[0] = eLD_BANK_R1; //RLD  //A70LV_John_0070 modify hard code
    aucData[1] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_Y]);
    aucData[2] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_Y] >> 8);
    aucData[3] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_R]);
    aucData[4] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_R] >> 8);
    aucData[5] = 0;
    aucData[6] = 0;
    aucData[7] = 0;
    aucData[8] = 0;
    if(rcSUCCESS != halLDCtrl_LD_Power_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }
#else
    aucData[0]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_Y]);
    aucData[1]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_Y] >> 8);
    aucData[2]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_R]);
    aucData[3]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_R] >> 8);
    aucData[4]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_B]);
    aucData[5]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_B] >> 8);
    aucData[6]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_G]);
    aucData[7]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_G] >> 8);

    aucData[8]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_Y]);
    aucData[9]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_Y] >> 8);
    aucData[10] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_R]);
    aucData[11] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_R] >> 8);
    aucData[12] = 0;
    aucData[13] = 0;
    aucData[14] = 0;
    aucData[15] = 0;

    if(rcSUCCESS != halLDCtrl_LD_PowerAll_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }
#endif /* 0 */
#endif

    uSSI_DRIVER uSSI = {0};
    uSSI.sSSI.uiRed =   m_uiCurrentPWM[eLD_SEG_BLD_R];
    uSSI.sSSI.uiGreen = m_uiCurrentPWM[eLD_SEG_BLD_G];
    uSSI.sSSI.uiBlue =  m_uiCurrentPWM[eLD_SEG_BLD_B];
    uSSI.sSSI.uiC1 =    m_uiCurrentPWM[eLD_SEG_BLD_Y];
    uSSI.sSSI.uiC2 =    m_uiCurrentPWM[eLD_SEG_RLD_R];
    uSSI.sSSI.uiSense = m_uiCurrentPWM[eLD_SEG_RLD_Y];	//A65_Clare_0006

    halFormatter_SetSSIPWMDriverLevels_1(&uSSI);

    return rcSUCCESS;
}

UINT16 halFormatter_PWM_Get(UINT8 cIndex)
{
    if(cIndex < eLD_SEG_NUMBERS)
    {
        return m_uiCurrentPWM[cIndex];
    }
    return 0;
}

eRESULT halFormatter_PWM_Update(void) //A70LV_Larry_0076
{
#if 1   //A65_Owen_0007
    eRESULT eResult = rcINVALID;
    UINT8   aucData[14] = {0};

    eResult = dv442x_Read(CMDF_SSIPWMDRIVERLEVELS, aucData);

    if(eResult == rcSUCCESS)
    {
        cmdGetHalfword(&m_uiCurrentPWM[eLD_SEG_BLD_R], &aucData[0]);
        cmdGetHalfword(&m_uiCurrentPWM[eLD_SEG_BLD_G], &aucData[2]);
        cmdGetHalfword(&m_uiCurrentPWM[eLD_SEG_BLD_B], &aucData[4]);
        cmdGetHalfword(&m_uiCurrentPWM[eLD_SEG_BLD_Y], &aucData[6]);
        cmdGetHalfword(&m_uiCurrentPWM[eLD_SEG_RLD_R], &aucData[8]);
        cmdGetHalfword(&m_uiCurrentPWM[eLD_SEG_RLD_Y], &aucData[10]);
    }
    else
    {
        LOG_MSG(db_HAL_LD, "PWM Get failed\r\n");
    }
#else
    UINT8 aucData[9] = {0};
    UINT8 ucCount = 0;
    for(ucCount = 0; ucCount < eLD_BANK_NUMBERS; ucCount++)
    {
        halLDCtrl_LD_Power_Get(eLDBANK_A70LV, aucData);
        if(aucData[0] == eLD_BANK_R1)
        {
            m_uiCurrentPWM[eLD_SEG_RLD_R] = aucData[4]<<8 | aucData[3];
            m_uiCurrentPWM[eLD_SEG_RLD_Y] = aucData[2]<<8 | aucData[1];
        }
        else
        {
            m_uiCurrentPWM[eLD_SEG_BLD_Y] = aucData[2]<<8 | aucData[1];
            m_uiCurrentPWM[eLD_SEG_BLD_R] = aucData[4]<<8 | aucData[3];
            m_uiCurrentPWM[eLD_SEG_BLD_B] = aucData[6]<<8 | aucData[5];
            m_uiCurrentPWM[eLD_SEG_BLD_G] = aucData[8]<<8 | aucData[7];
        }
    }
#endif
    LOG_MSG(db_HAL_LD, "PWM Get %d %d %d %d %d %d\r\n",
                        m_uiCurrentPWM[eLD_SEG_BLD_R],
                        m_uiCurrentPWM[eLD_SEG_BLD_G],
                        m_uiCurrentPWM[eLD_SEG_BLD_B],
                        m_uiCurrentPWM[eLD_SEG_BLD_Y],
                        m_uiCurrentPWM[eLD_SEG_RLD_R],
                        m_uiCurrentPWM[eLD_SEG_RLD_Y]);

    return eResult;
}

UINT16 halFormatter_RLD_PWM_Set(UINT16 uiRValue,UINT16 uiYValue) //G100_Doulas_0023 Modify//A70LV_Larry_0363
{
    UINT8 aucData[16] = {0};

    m_uiCurrentPWM[eLD_SEG_RLD_R] = uiRValue;
    m_uiCurrentPWM[eLD_SEG_RLD_Y] = uiYValue;

    //m_uiCurrentPWM[3] = sLD_PWM_Target.wBLD_Y;
    //m_uiCurrentPWM[0] = sLD_PWM_Target.wBLD_R;
    //m_uiCurrentPWM[2] = sLD_PWM_Target.wBLD_B;
    //m_uiCurrentPWM[1] = sLD_PWM_Target.wBLD_G;
    //m_uiCurrentPWM[5] = sLD_PWM_Target.wRLD_Y;
    //m_uiCurrentPWM[4] = sLD_PWM_Target.wRLD_R;

#if 1	//G100_Doulas_0023 Modify

	cmdPutHalfword(m_uiCurrentPWM[eLD_SEG_BLD_R], &aucData[0]);
	cmdPutHalfword(m_uiCurrentPWM[eLD_SEG_BLD_G], &aucData[2]);
	cmdPutHalfword(m_uiCurrentPWM[eLD_SEG_BLD_B], &aucData[4]);
	cmdPutHalfword(m_uiCurrentPWM[eLD_SEG_BLD_Y], &aucData[6]);
	cmdPutHalfword(m_uiCurrentPWM[eLD_SEG_RLD_R], &aucData[8]);
	cmdPutHalfword(m_uiCurrentPWM[eLD_SEG_RLD_Y], &aucData[10]);

	LOG_MSG(db_HAL_FORMATTER, "PWM (%d,%d,%d,%d)(%d,%d)\r\n", __FUNCTION__, __LINE__, m_uiCurrentPWM[eLD_SEG_BLD_R],m_uiCurrentPWM[eLD_SEG_BLD_G],
																					  m_uiCurrentPWM[eLD_SEG_BLD_B],m_uiCurrentPWM[eLD_SEG_BLD_Y],
																					  m_uiCurrentPWM[eLD_SEG_RLD_R],m_uiCurrentPWM[eLD_SEG_RLD_Y]);

    if(rcSUCCESS != dv442x_Write(CMDF_SSIPWMDRIVERLEVELS,aucData))
    {
        return rcERROR;
    }
#else
    aucData[0]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_Y]);
    aucData[1]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_Y] >> 8);
    aucData[2]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_R]);
    aucData[3]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_R] >> 8);
    aucData[4]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_B]);
    aucData[5]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_B] >> 8);
    aucData[6]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_G]);
    aucData[7]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_BLD_G] >> 8);

    aucData[8]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_Y]);
    aucData[9]  = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_Y] >> 8);
    aucData[10] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_R]);
    aucData[11] = (UINT8)(m_uiCurrentPWM[eLD_SEG_RLD_R] >> 8);
    aucData[12] = 0;
    aucData[13] = 0;
    aucData[14] = 0;
    aucData[15] = 0;

    if(rcSUCCESS != halLDCtrl_LD_PowerAll_Set(eLDBANK_A70LV, aucData))
    {
        return rcERROR;
    }
#endif /* 0 */
    //LOG_MSG(db_ALWAYS, "(func:%s, line:%d)\r\n", __FUNCTION__, __LINE__);

    return rcSUCCESS;

}


// A70LV_Eric.C_0025 Start
BOOL halDDP4422_IsAsicReady(void)
{
    return dvDDP4422_IsAsicReady();
}
// A70LV_Eric.C_0025 End

eRESULT halFormatter_ColorEnhancementSet(UINT8 ucColorEnhancement)      //A70LV_Doulas_0038
{
    eRESULT  eResult = rcINVALID;
#if defined(PLATFORM_A70G2) || defined(PLATFORM_A35G2)//HICC2_Doulas_0001 no use
    UINT8 ucDatax[2] = {0};

    ucDatax[0] = ucColorEnhancement;

    eResult = dv442x_Write(CMDF_COLOR_ENHANCEMENT, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): ColorEnhancement %d\r\n", __FUNCTION__, __LINE__,ucColorEnhancement);
#endif
    return eResult;
}

#if 0   //move to palFormatterMgr_WallColorSet
eRESULT halFormatter_WallColorSet(UINT8 ucWallColor)      //A70LV_Doulas_0044
{
    eRESULT  eResult = rcINVALID;
    UINT16 uiRedGain,uiGreenGain,uiBlueGain;
    UINT8  ucPresetMode;
    UINT8  ucGuiPresetMode;
    UINT8  ucGuiWallColor;

    ucPresetMode = halScaler_PictureSettings_Get_NotReturnUser((UINT8)eSOURCE_WINDOW_MAIN);
    if(ucWallColor >= eCM_WALL_COLOR_NUMBER)
    {
#if defined(CUSTOM_BARCO) || defined(CUSTOM_OPTOMA)     //A65_Owen_0002
        ucWallColor = eCM_WALL_COLOR_OFF;
#else
        ucWallColor = eCM_WALL_COLOR_WHITE;
#endif
    }
    ucGuiPresetMode = CM2GUI(edcPICTURE_SETTINGS, ucPresetMode);
    if((INT8)ucGuiPresetMode < 0 )
    {
        ucGuiPresetMode = 0 ;
    }

    ucGuiWallColor = CM2GUI(edcWALL_COLOR, ucWallColor);
    if((INT8)ucGuiWallColor < 0 )
    {
        ucGuiWallColor = 0 ;
    }

    uiRedGain = sWallColorValues[ucGuiPresetMode][ucGuiWallColor].RedGain ;
    uiGreenGain = sWallColorValues[ucGuiPresetMode][ucGuiWallColor].GreenGain ;
    uiBlueGain = sWallColorValues[ucGuiPresetMode][ucGuiWallColor].BlueGain ;

    eResult = halFormatter_ImageRGBGainSet(uiRedGain,uiGreenGain,uiBlueGain);
    return eResult;
}
#endif


eRESULT halFormatter_ParameterSet(UINT8 ucPowerMode,UINT8 ucPowerModeNumber,UINT8 ucPictureMode,UINT8 ucColorTemperature,UINT8 ucColorWheelSpeed,UINT8 ucInputSource)    //A70LV_Doulas_0054
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[6] = {0};

    // DDP use GUI index
    //ucPowerMode = CM2GUI(edcPOWER_MODE, ucPowerMode);
    ucPowerMode = ((INT8)ucPowerMode >= 0) ? ucPowerMode : 100;

    ucDatax[0] = ucPowerMode;
    ucDatax[1] = ucPowerModeNumber;
    ucDatax[2] = ucPictureMode;
    ucDatax[3] = ucColorTemperature;
    ucDatax[4] = ucColorWheelSpeed;
    ucDatax[5] = ucInputSource;

    eResult = dv442x_Write(CMDF_PARAMETER_SET, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): %d,ParameterSet %d,%d,%d,%d,%d,%d\r\n", __FUNCTION__, __LINE__,eResult,ucPowerMode,ucPowerModeNumber,
                ucPictureMode,ucColorTemperature,ucColorWheelSpeed,ucInputSource);
    return eResult;
}

eRESULT halFormatter_CeilingMount_RearProjectSet(BOOL bCeilingMount,BOOL bRearProject)    //A70LV_Doulas_0062
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};
    UINT8 ucLensType = 0;

    palDataMgr_Data_Access(edcLENS_TYPE, edaREAD, &ucLensType); //HICC2_Julie_0070//H30K_Julie_0005

    if(bCeilingMount)
    {
        if((ucLensType == eLENS_ID_B20U100) || (ucLensType == eLENS_ID_NVRAM_B20U100))//HICC2_Julie_0070
        {
            if(bRearProject)
                ucDatax[0] = NORMALWITHREAR;   //CelingWithRear
            else
                ucDatax[0] = NORMALPROJECTOR;  //CelingProjector
        }
        else
        {
            if(bRearProject)
                ucDatax[0] = CEILINGWITHREAR;   //CelingWithRear
            else
                ucDatax[0] = CEILINGPROJECTOR;  //CelingProjector
        }
    }
    else
    {
        if((ucLensType == eLENS_ID_B20U100) || (ucLensType == eLENS_ID_NVRAM_B20U100))
        {
            if(bRearProject)
                ucDatax[0] = CEILINGWITHREAR;   //NormalWithRear
            else
                ucDatax[0] = CEILINGPROJECTOR;  //NormalProjector
        }
        else
        {
            if(bRearProject)
                ucDatax[0] = NORMALWITHREAR;    //NormalWithRear
            else
                ucDatax[0] = NORMALPROJECTOR;   //NormalProjector
        }
    }

    eResult = dv442x_Write(CMDF_IMG_ORIENTATION, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): Ceiling %d ,Rear %d  %d\r\n", __FUNCTION__, __LINE__,bCeilingMount,bRearProject, ucDatax[0]);
    return eResult;
}

eRESULT halFormatter_CeilingMount_RearProjectGet(UINT8 *pucValue)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_IMG_ORIENTATION, ucDatax);

    *pucValue = ucDatax[0];


    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): %d \r\n", __FUNCTION__, __LINE__, ucDatax[0]);
    return eResult;
}

#if 0  //move to palFormatterMgr_COLOR_ENHANCEMENT_ValueGet
eRESULT halFormatter_COLOR_ENHANCEMENT_ValueGet(UINT8 ucDispalyMode,UINT8 ucCE,sHSG_SETTING *sHSG)    //A70LV_Doulas_0074
{
    eRESULT  eResult = rcSUCCESS;

    ucDispalyMode = CM2GUI(edcPICTURE_SETTINGS, ucDispalyMode);

    if((INT8)ucDispalyMode < 0 )
    {
        ucDispalyMode = 0 ;
    }

	if(palDataMgr_Model_ID_Get() == MODULE_TYPE_ID0_PLATFORM)
	{
		memcpy(sHSG,&sCE_TABLE_SETTING_W16[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}
	else
    {
    	memcpy(sHSG,&sCE_TABLE_SETTING_W20[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}

    //LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): Ceiling %d ,Rear %d  %d\r\n", __FUNCTION__, __LINE__,bCeilingMount,bRearProject);
    return eResult;
}
#endif


#if 0   //palFormatterMgr_CE_Add_HSG_Set
eRESULT halFormatter_CE_Add_HSG_Set(UINT8 ucDispalyMode,UINT8 ucCE,sHSG_SETTING sHSG)    //A70LV_Doulas_0074
{
    eRESULT  eResult = rcSUCCESS;
    sHSG_SETTING  sCE_HSG;


    ucDispalyMode = CM2GUI(edcPICTURE_SETTINGS, ucDispalyMode);

    if((INT8)ucDispalyMode < 0 )
    {
        ucDispalyMode = 0 ;
    }

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): ucDispalyMode %d ,ucCE %d \r\n", __FUNCTION__, __LINE__,ucDispalyMode,ucCE);

	if(palDataMgr_Model_ID_Get() == MODULE_TYPE_ID0_PLATFORM)	//G100_Clare_0015
	{
		memcpy(&sCE_HSG,&sCE_TABLE_SETTING_W16[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}
	else
	{
		memcpy(&sCE_HSG,&sCE_TABLE_SETTING_W20[ucDispalyMode][ucCE], sizeof(sHSG_SETTING));
	}

    //Red
    if(( (INT16)sHSG.HSG_R_HUE + (INT16)sCE_HSG.HSG_R_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_R_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_R_HUE + (INT16)sCE_HSG.HSG_R_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_R_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_R_HUE += sHSG.HSG_R_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_R_SAT + (INT16)sCE_HSG.HSG_R_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_R_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_R_SAT + (INT16)sCE_HSG.HSG_R_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_R_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_R_SAT += sHSG.HSG_R_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_R_GAIN + (INT16)sCE_HSG.HSG_R_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_R_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_R_GAIN + (INT16)sCE_HSG.HSG_R_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_R_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_R_GAIN += sHSG.HSG_R_GAIN - HSG_DEFAULT_VALUE;


    //Green
    if(( (INT16)sHSG.HSG_G_HUE + (INT16)sCE_HSG.HSG_G_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_G_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_G_HUE + (INT16)sCE_HSG.HSG_G_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_G_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_G_HUE += sHSG.HSG_G_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_G_SAT + (INT16)sCE_HSG.HSG_G_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_G_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_G_SAT + (INT16)sCE_HSG.HSG_G_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_G_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_G_SAT += sHSG.HSG_G_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_G_GAIN + (INT16)sCE_HSG.HSG_G_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_G_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_G_GAIN + (INT16)sCE_HSG.HSG_G_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_G_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_G_GAIN += sHSG.HSG_G_GAIN - HSG_DEFAULT_VALUE;


    //Blue
    if(( (INT16)sHSG.HSG_B_HUE + (INT16)sCE_HSG.HSG_B_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_B_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_B_HUE + (INT16)sCE_HSG.HSG_B_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_B_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_B_HUE += sHSG.HSG_B_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_B_SAT + (INT16)sCE_HSG.HSG_B_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_B_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_B_SAT + (INT16)sCE_HSG.HSG_B_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_B_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_B_SAT += sHSG.HSG_B_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_B_GAIN + (INT16)sCE_HSG.HSG_B_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_B_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_B_GAIN + (INT16)sCE_HSG.HSG_B_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_B_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_B_GAIN += sHSG.HSG_B_GAIN - HSG_DEFAULT_VALUE;


    //Cyan
    if(( (INT16)sHSG.HSG_C_HUE + (INT16)sCE_HSG.HSG_C_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_C_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_C_HUE + (INT16)sCE_HSG.HSG_C_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_C_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_C_HUE += sHSG.HSG_C_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_C_SAT + (INT16)sCE_HSG.HSG_C_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_C_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_C_SAT + (INT16)sCE_HSG.HSG_C_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_C_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_C_SAT += sHSG.HSG_C_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_C_GAIN + (INT16)sCE_HSG.HSG_C_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_C_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_C_GAIN + (INT16)sCE_HSG.HSG_C_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_C_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_C_GAIN += sHSG.HSG_C_GAIN - HSG_DEFAULT_VALUE;


    //Magenta
    if(( (INT16)sHSG.HSG_M_HUE + (INT16)sCE_HSG.HSG_M_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_M_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_M_HUE + (INT16)sCE_HSG.HSG_M_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_M_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_M_HUE += sHSG.HSG_M_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_M_SAT + (INT16)sCE_HSG.HSG_M_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_M_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_M_SAT + (INT16)sCE_HSG.HSG_M_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_M_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_M_SAT += sHSG.HSG_M_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_M_GAIN + (INT16)sCE_HSG.HSG_M_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_M_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_M_GAIN + (INT16)sCE_HSG.HSG_M_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_M_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_M_GAIN += sHSG.HSG_M_GAIN - HSG_DEFAULT_VALUE;


    //Yellow
    if(( (INT16)sHSG.HSG_Y_HUE + (INT16)sCE_HSG.HSG_Y_HUE - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_Y_HUE = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_Y_HUE + (INT16)sCE_HSG.HSG_Y_HUE - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_Y_HUE = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_Y_HUE += sHSG.HSG_Y_HUE - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_Y_SAT + (INT16)sCE_HSG.HSG_Y_SAT - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_Y_SAT = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_Y_SAT + (INT16)sCE_HSG.HSG_Y_SAT - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_Y_SAT = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_Y_SAT += sHSG.HSG_Y_SAT - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_Y_GAIN + (INT16)sCE_HSG.HSG_Y_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_Y_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_Y_GAIN + (INT16)sCE_HSG.HSG_Y_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_Y_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_Y_GAIN += sHSG.HSG_Y_GAIN - HSG_DEFAULT_VALUE;


    //White
    if(( (INT16)sHSG.HSG_W_R_GAIN + (INT16)sCE_HSG.HSG_W_R_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_W_R_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_W_R_GAIN + (INT16)sCE_HSG.HSG_W_R_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_W_R_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_W_R_GAIN += sHSG.HSG_W_R_GAIN - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_W_G_GAIN + (INT16)sCE_HSG.HSG_W_G_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_W_G_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_W_G_GAIN + (INT16)sCE_HSG.HSG_W_G_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_W_G_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_W_G_GAIN += sHSG.HSG_W_G_GAIN - HSG_DEFAULT_VALUE;

    if(( (INT16)sHSG.HSG_W_B_GAIN + (INT16)sCE_HSG.HSG_W_B_GAIN - (INT16)HSG_DEFAULT_VALUE) > HSG_MAX_VALUE)
        sCE_HSG.HSG_W_B_GAIN = HSG_MAX_VALUE;
    else if(( (INT16)sHSG.HSG_W_B_GAIN + (INT16)sCE_HSG.HSG_W_B_GAIN - (INT16)HSG_DEFAULT_VALUE) < HSG_MIN_VALUE)
        sCE_HSG.HSG_W_B_GAIN = HSG_MIN_VALUE;
    else
        sCE_HSG.HSG_W_B_GAIN += sHSG.HSG_W_B_GAIN - HSG_DEFAULT_VALUE;

    halFormatter_HSGValueSet(sCE_HSG);
    return eResult;
}
#endif

eRESULT halFormatter_Upgrade_Set(UINT8 ucEnable) //A70LV_Larry_0138
{
    dv442x_UpgradeSet(ucEnable);

    return rcSUCCESS;
}

//3D Format:
// [0] : first frame
// [1] : FIELD GPIO
eRESULT halFormatter_3D_FormatSet(UINT8 c3D_Format)         //A70LV_Doulas_0154
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    Datax[0] = c3D_Format;

    eResult = dv442x_Write(CMDF_3D_FORMAT, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_3D_FormatSet %d\r\n", __FUNCTION__, __LINE__,c3D_Format);
    return eResult;
}

eRESULT halFormatter_3D_FormatGet(UINT8 *c3D_Format)        //A70LV_Doulas_0154
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_3D_FORMAT, Datax);

    *c3D_Format = Datax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_3D_FormatGet %d\r\n", __FUNCTION__, __LINE__,*c3D_Format);
    return eResult;
}

eRESULT halFormatter_3DSourceConfigSet(UINT8 *c3D_Source)         //A70Gen2_Doulas_0023
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[13]={0};

	memcpy(Datax, c3D_Source, 13);

    eResult = dv442x_Write(CMDF_3DSOURCECONFIG, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_3DSourceConfigSet %d\r\n", __FUNCTION__, __LINE__,c3D_Source[1]);
    return eResult;
}

eRESULT halFormatter_3DSourceConfigGet(UINT8 *c3D_Source)        //A70Gen2_Doulas_0023
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[13]={0};

    eResult = dv442x_Read(CMDF_3DSOURCECONFIG, Datax);

    memcpy(c3D_Source, Datax, 13);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_3DSourceConfigGet %d\r\n", __FUNCTION__, __LINE__,Datax[1]);
    return eResult;
}

eRESULT halFormatter_ChannelSourceSet(UINT8 cSource)    //A70LV_Doulas_0187
{
    eRESULT eResult = rcINVALID;
    UINT8    ucDatax[2] = {0};

    ucDatax[0] = cSource;
    ucDatax[1] = 0x00;

    eResult = dv442x_Write(CMDF_CHANNELSOURCE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):Channel Source set %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);

    return eResult;
}

eRESULT halFormatter_LightsOutTimerSet(UINT8 cLightOutTimer) //A70LH_John_0034 add switch function between Chirstie and ScreenX //A70LV_Larry_0236
{
    eRESULT eResult = rcINVALID;

    UINT8 Datax = 0;

#if defined(CUSTOM_OPTOMA) //HICC2_Steven_0042 fix ISS-0023471 //A35G2_BRC_Casper_0125
    Datax = cLightOutTimer;
#else
    Datax = cLightOutTimer*2;
#endif

    eResult = dv442x_Write(CMDF_REALBLACK_LIGHTSOUTTIMER, &Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):LightsOutTimerSet %d\r\n", __FUNCTION__, __LINE__, Datax);    //ZU860_Doulas_0047

    return eResult;
}

//ZU860_John_0024 start add ABP command to DDP
eRESULT halFormatter_ABPINFOSet(UINT8 ucType, UINT8 *pucData)		//G100_Doulas_0010 Modify
{
    eRESULT eResult = rcINVALID;
	#pragma pack(push)	/* push current alignment to stack */
	#pragma pack(1)		/* set alignment to 1 byte boundary */
    union uTargetSensor
    {
        UINT8 ucData[9];
        struct sSensor
        {
            UINT8  ucType_Index;
            UINT16 uiColor_Yellow;
            UINT16 uiColor_Red;
            UINT16 uiColor_Blue;
            UINT16 uiColor_Green;
        }sColorIndex;
    }uData;
	#pragma pack(pop)	/* restore original alignment from stack */

	memset(uData.ucData, 0x00, 9);
    if(ucType == eABP_CONTROL_MODE)           //contorl mode does not need any parameter
    {
		uData.ucData[1] = *pucData;
    }
    else
    {
        //do not use memcpy in order to sync with DDP command shift rule
        cmdGetHalfword(&uData.sColorIndex.uiColor_Yellow, &pucData[0]);
        cmdGetHalfword(&uData.sColorIndex.uiColor_Red, &pucData[2]);
        cmdGetHalfword(&uData.sColorIndex.uiColor_Blue, &pucData[4]);
        cmdGetHalfword(&uData.sColorIndex.uiColor_Green, &pucData[6]);
    }

    uData.sColorIndex.ucType_Index = ucType;

    eResult = dv442x_Write(CMDF_ABPINFO, uData.ucData);

    //if(ucType == eABP_TARGET_LIGHTSENSOR_BLD)	//G100_Doulas_0010 remove
    {
        LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):ABPINFO set (YRBG) %d: %05d, %5d, %5d, %5d\r\n", __FUNCTION__, __LINE__,
                                                                                                uData.sColorIndex.ucType_Index,
                                                                                                uData.sColorIndex.uiColor_Yellow,
                                                                                                uData.sColorIndex.uiColor_Red,
                                                                                                uData.sColorIndex.uiColor_Blue,
                                                                                                uData.sColorIndex.uiColor_Green);
    }
    return eResult;
}

eRESULT halFormatter_ABPINFOGet(UINT8 *ucStatus)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_ABPINFO, ucStatus);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):ABPINFO get (YRBG) %d\r\n", __FUNCTION__, __LINE__,*ucStatus);

    return eResult;
}
//ZU860_John_0024 end

eRESULT halFormatter_IMG_AlgorithmEnableSet(UINT8* cAlg)            //A70LV_Doulas_0235
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Write(CMDF_ALGORITHM, cAlg);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_IMG_AlgorithmEnableSet (%d,%d)(%d,%d)(%d,%d)(%d)\r\n", __FUNCTION__, __LINE__,cAlg[0],cAlg[1],cAlg[2],cAlg[3],cAlg[4],cAlg[5],cAlg[6]);

    return eResult;
}

eRESULT halFormatter_IMG_AlgorithmEnableGet(UINT8* cAlg)            //A70LV_Doulas_0235
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_ALGORITHM, cAlg);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_IMG_AlgorithmEnableGet (%d,%d)(%d,%d)(%d,%d)(%d)\r\n", __FUNCTION__, __LINE__,cAlg[0],cAlg[1],cAlg[2],cAlg[3],cAlg[4],cAlg[5],cAlg[6]);
    return eResult;
}

eRESULT halFormatter_Projection_Mode_Set(UINT8 ucProjection_Mode)   //A70LV_Doulas_0271
{
  eRESULT eResult = rcINVALID;
  //UINT8 ucDatax = eCMD_Formatter_Standby;

	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d)Projection_Mode_Set %d\r\n", __FUNCTION__, __LINE__,ucProjection_Mode);

	eResult = dv442x_Write(CMDF_PROJECTIONMODE,&ucProjection_Mode);

    vTaskDelay( 100 );

    return eResult;
}

eRESULT halFormatter_Projection_Mode_Get(UINT8 *ucProjection_Mode)
{
  eRESULT eResult = rcINVALID;
  //UINT8 ucDatax = eCMD_Formatter_Standby;

	eResult = dv442x_Read(CMDF_PROJECTIONMODE, ucProjection_Mode);
	//MS_SLEEP( 100 );
	LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d)Projection_Mode_Get %d\r\n", __FUNCTION__, __LINE__, *ucProjection_Mode);

    return eResult;
}

eRESULT halFormatter_SlaveProjection_Mode_Get(UINT8 *ucProjection_Mode)
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax = 0;

#if defined(PLATFORM_H60_2K)

#else
    eResult = dv442x_Write(CMDF_SLAVEPROJECTIONMODE, &ucDatax);

    eResult = dv442x_Read(CMDF_SLAVEPROJECTIONMODE, ucProjection_Mode);
    //MS_SLEEP( 100 );
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d)Projection_Mode_Get %d\r\n", __FUNCTION__, __LINE__, *ucProjection_Mode);
#endif

    return eResult;
}

eRESULT halFormatter_SRCDescriptionGet(UINT8 *pcSRC) //T100_Casper_0087
{
    eRESULT eResult = rcINVALID;
	//UINT8	i = 0;
    eResult = dv442x_Read(CMDF_SRCDESCRIPTION, pcSRC);

	/*
	for(i=0 ; i <17 ; i++)
	{
    	LOG_MSG(db_HAL_FORMATTER, "SRCDescriptionGet[%d]: %d \r\n", i,pcSRC[i]);
	}
	*/
    return eResult;
}

//ZU860_John_0018 start add auto tuning of corrected color temperature//A70LV_John_0077 start add CCT function for AP
eRESULT halFormatter_APCCT_Set(UINT8* cCCT)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Write(CMDF_APCCT, cCCT); //out to DDP = 9*6 =54

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):\r\nAPCCT PWM Set.\r\n", __FUNCTION__, __LINE__);

    return eResult;
}

eRESULT halFormatter_APCCT_Get(UINT8* cCCT)
{
    eRESULT eResult = rcINVALID;

    eResult = dv442x_Read(CMDF_APCCT, cCCT); //read from DDP = 9*6*2 =108

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):\r\nAPCCT PWM Get.\r\n", __FUNCTION__, __LINE__);


    return eResult;
}
//ZU860_John_0018 end //A70LV_John_0077 end


eRESULT halFormatter_XPR_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    //0:2way 24~120hz, 4way24~30hz
    //1:4way48~120hz
    //2:4way200~240hz

    if(eXPR_ON != ucEnable)
    {
        ucDatax[0] = 0x02; //XPR Off
        ucDatax[1] = 0x00;
    }
#if 0
    else if(eXPR_OFF_4WAY_120 == ucEnable)
    {
        ucDatax[0] = 0x02; //XPR Off
        ucDatax[1] = 0x01;
    }
    else if(eXPR_OFF_4WAY_240 == ucEnable)
    {
        ucDatax[0] = 0x02; //XPR Off
        ucDatax[1] = 0x02;
    }
#endif
    else
    {
        ucDatax[0] = 0x01; //XPR On
        ucDatax[1] = 0x00;
    }

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

#endif
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Set %d\r\n", __FUNCTION__, __LINE__, ucEnable);
    return eResult;
}

eRESULT halFormatter_XPR_3DLR_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    if(ucEnable == FALSE)
    {
        ucDatax[0] = 0x31;
        ucDatax[1] = 0x00;
    }
    else
    {
        ucDatax[0] = 0x31;
        ucDatax[1] = 0x01;
    }

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Set %d\r\n", __FUNCTION__, __LINE__, ucEnable);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_DataSwap_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    if(ucEnable == FALSE)
    {
        ucDatax[0] = 0x26;
        ucDatax[1] = 0x00;
    }
    else
    {
        ucDatax[0] = 0x26;
        ucDatax[1] = 0x01;
    }

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Set %d\r\n", __FUNCTION__, __LINE__, ucEnable);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_CalibrationPattern_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    if(ucEnable)
    {
        ucDatax[0] = 0x07;
        ucDatax[1] = 0x01;
    }
    else
    {
        ucDatax[0] = 0x07;
        ucDatax[1] = 0x00;
    }

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Calibration Pattern Set %d\r\n", __FUNCTION__, __LINE__, ucEnable);

	vTaskDelay( 100 );
#endif

    return eResult;
}

eRESULT halFormatter_XPR_CalibrationPattern_Get(UINT8 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = 0x07;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    *ucValue = ucReadData[0];
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Calibration Pattern Get %d\r\n", __FUNCTION__, __LINE__, ucReadData[0]);
#endif

    return eResult;
}

eRESULT halFormatter_XPRVx1Enable_Set(UINT8 ucEnable)
{
    eRESULT  eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    if(ucEnable)
    {
        ucDatax[0] = 0x27;
        ucDatax[1] = 0x01;
    }
    else
    {
        ucDatax[0] = 0x27;
        ucDatax[1] = 0x00;
    }

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Vx1 Set %d\r\n", __FUNCTION__, __LINE__, ucEnable);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_ErrorStatus_Set(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = 0x37;

    eResult &= dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Error Status Set %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_ErrorStatus_Get(UINT8 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = 0x37;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    *ucValue = ucReadData[0];
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Error Status Get %d\r\n", __FUNCTION__, __LINE__, ucReadData[0]);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Filter_Set(UINT8 ucDatax) //A70LK_Casper_0002
{
    eRESULT eResult = rcSUCCESS;
    UINT8 acDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    //0:P5_TRP_UHD_DMD
    //1:P7_TRP_UHD_DMD
    acDatax[0] = 0x08; //filter
    acDatax[1] = ucDatax;

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, acDatax);
#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Filter Set %d\r\n", __FUNCTION__, __LINE__, ucDatax);
    return eResult;
}

eRESULT halFormatter_XPR_Filter_Get(UINT8 *ucValue) //A70LK_Casper_0002
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = 0x08;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    *ucValue = ucReadData[0];
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Filter Get %d\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Close_Loop_Enable_Axis0_Set(UINT8 ucDatax)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 acDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    acDatax[0] = FPGAcontrol_ActrAxis0CloseLoopEn;
    acDatax[1] = ucDatax;

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, acDatax);
#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Axis0 Close Loop Enable Set %d\r\n", __FUNCTION__, __LINE__, ucDatax);
    return eResult;
}

eRESULT halFormatter_XPR_Close_Loop_Enable_Axis0_Get(UINT8 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[4] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrAxis0CloseLoopEn;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);
    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    *ucValue = ucReadData[0];
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Axis0 Close Loop Enable Get %d\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Close_Loop_Enable_Axis1_Set(UINT8 ucDatax)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 acDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    acDatax[0] = FPGAcontrol_ActrAxis1CloseLoopEn;
    acDatax[1] = ucDatax;

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, acDatax);
#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Axis1 Close Loop Enable Set %d\r\n", __FUNCTION__, __LINE__, ucDatax);
    return eResult;
}

eRESULT halFormatter_XPR_Close_Loop_Enable_Axis1_Get(UINT8 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrAxis1CloseLoopEn;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);
    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    *ucValue = ucReadData[0];
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Axis1 Close Loop Enable Get %d\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Gain_of_Axis0_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrErrGainAxis0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Gain of Axis0 Get %d\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Gain_of_Axis1_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrErrGainAxis1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Gain of Axis1 Get %d\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Segment_Length_of_Axis0_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrErrSegLenAxis0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Segment Length of Axis0 Get %d\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Segment_Length_of_Axis1_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrErrSegLenAxis1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Segment Length of Axis1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_SN_Get(UINT8 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};


#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = FPGAcontrol_ActrSN0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);
    memcpy((uint8*)&ucValue[0], ucReadData, 8);


    ucDatax[0] = FPGAcontrol_ActrSN1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);
    memcpy((uint8*)&ucValue[8], ucReadData, 8);

    ucDatax[0] = FPGAcontrol_ActrSN2;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);
    memcpy((uint8*)&ucValue[16], ucReadData, 8);

    ucDatax[0] = FPGAcontrol_ActrSN3;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);
    memcpy((uint8*)&ucValue[24], ucReadData, 8);


    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR SN Get %d, %d, %d, %d, %d, %d, %d ,%d\r\n", __FUNCTION__, __LINE__, ucReadData[0],ucReadData[1],ucReadData[2],ucReadData[3],ucReadData[4],ucReadData[5],ucReadData[6],ucReadData[7]);

#endif

    return eResult;
}

eRESULT halFormatter_XPR_Eeprom_TargerAdc_Axis0_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrEepTargetAdcAxis0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM Target ADC Axis0 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}


eRESULT halFormatter_XPR_Eeprom_TargerAdc_Axis1_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrEepTargetAdcAxis1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM Target ADC Axis1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Eeprom_ZData0_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrEepZData0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM Z data0 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}


eRESULT halFormatter_XPR_Eeprom_ZData1_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrEepZData1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM Z data1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Eeprom_XData0_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = FPGAcontrol_ActrEepXData0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM X data0 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Eeprom_XData1_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = FPGAcontrol_ActrEepXData1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM X data1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Eeprom_YData0_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = FPGAcontrol_ActrEepYData0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM Y data0 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Eeprom_YData1_Get(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = FPGAcontrol_ActrEepYData1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR EERRPM Y data1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Gain_Write_EEPROM(void)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrWriteEep;

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Write to EEPROM %d\r\n", __FUNCTION__, __LINE__, ucDatax[0]);
#endif

    return eResult;
}

eRESULT halFormatter_XPR_Version_Get(UINT8 *ucValue) //HICC2_Steven_0039
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K) || defined(PLATFORM_H30_4K)
    ucDatax[0] = FPGAcontrol_getVersion;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    //*ucValue = ucReadData[0];
    memcpy((uint8*)&ucValue[0], ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Version Get V%d.%d.%d.%d\r\n", __FUNCTION__, __LINE__, *ucValue,  *(ucValue+1),  *(ucValue+2), *(ucValue+3));
#endif

    return eResult;
}

eRESULT halFormatter_WAP_SetGet(UINT8 ucWap_Mode, eWAP_POWERLEVEL eWap_Level)
{
    eRESULT eResult = rcINVALID;
    UINT8   ucDatax[2] = {0};
    UINT8   ucPictureMode;
    UINT8   ucWap_Level = (UINT8)eWAP_LEVEL_100;
    {
        switch(eWap_Level)
        {
            case eWAP_POWERLEVEL_100:   ucWap_Level = (UINT8)eWAP_LEVEL_100;   break;
            case eWAP_POWERLEVEL_50:    ucWap_Level = (UINT8)eWAP_LEVEL_50;    break;
            case eWAP_POWERLEVEL_30:    ucWap_Level = (UINT8)eWAP_LEVEL_30;    break;
            case eWAP_POWERLEVEL_10:    ucWap_Level = (UINT8)eWAP_LEVEL_10;    break;
            default:
                LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d) unsupport WAP Level %d\r\n", eWap_Level);
                return rcERROR;
        }

        ucDatax[0] = ucWap_Mode;
        ucDatax[1] = ucWap_Level;
#ifdef PLATFORM_H60_2K
        eResult = dv442x_Write(CMDF_WAP_SETGET, ucDatax);
#else
        eResult = dv442x_Write(CMDF_DDPGETASICCONFIG, ucDatax); //HICC2_Steven_0021
#endif
    }

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): WAP Display Mode[%d], Power Level[%d] Get.\r\n", __FUNCTION__, __LINE__,ucDatax[0],ucDatax[1]);

    return eResult;
}

eRESULT halFormatter_WAP_AllSet(eWAP_POWERLEVEL eWap_Level, UINT16 *ucPWM) //G50_Casper_0006
{
    eRESULT eResult = rcINVALID;
    UINT8   ucWAP_PWM[3+eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS*2] = {0};
    UINT8   ucWAP_Index = 0;
    UINT16  ucPWM_Temp = 0;
    UINT8   ucWap_Level = (UINT8)eWAP_LEVEL_100;

    switch(eWap_Level)
    {
        case eWAP_POWERLEVEL_100:   ucWap_Level = (UINT8)eWAP_LEVEL_100;   break;
        case eWAP_POWERLEVEL_50:    ucWap_Level = (UINT8)eWAP_LEVEL_50;    break;
        case eWAP_POWERLEVEL_30:    ucWap_Level = (UINT8)eWAP_LEVEL_30;    break;
        case eWAP_POWERLEVEL_10:    ucWap_Level = (UINT8)eWAP_LEVEL_10;    break;
        default:
            LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d) unsupport WAP Level %d\r\n", eWap_Level);
            return rcERROR;
    }

    ucWAP_PWM[0] = ucWap_Level;

    for(ucWAP_Index=0; ucWAP_Index < (eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS); ucWAP_Index++)
    {
        ucWAP_PWM[1 + 2*ucWAP_Index] = LSB(*(ucPWM+ucWAP_Index));
        ucWAP_PWM[2 + (2*ucWAP_Index)] = MSB(*(ucPWM+ucWAP_Index));
        LOG_MSG(db_HAL_FORMATTER, "WAP_AllSet[Level : %d] = (%d,%d)\r\n", ucWAP_PWM[0], ucWAP_PWM[(1+2*ucWAP_Index)],ucWAP_PWM[2+ (2*ucWAP_Index)]);
    }


    eResult = dv442x_Write(CMDF_WAP_ALLSET_TRPDMD_ESTATUS, ucWAP_PWM);  //out to DDP = 1+10*6*2 = 121
    return eResult;
}

eRESULT halFormatter_WAP_Set(UINT8 ucWap_Mode,eWAP_POWERLEVEL eWap_Level,UINT8 ucWap_LDSeq,UINT16 ucPWM) //G50_Casper_0006
{
    eRESULT eResult = rcINVALID;
    UINT8   ucWAP_PWM[3+eCM_PICTURE_SETTINGS_NUMBER*eLD_SEG_NUMBERS*2] = {0};
    UINT16  ucPWM_Temp = 0;
    UINT8   ucWap_Level = (UINT8)eWAP_LEVEL_100;

    switch(eWap_Level)
    {
        case eWAP_POWERLEVEL_100:   ucWap_Level = (UINT8)eWAP_LEVEL_100;   break;
        case eWAP_POWERLEVEL_50:    ucWap_Level = (UINT8)eWAP_LEVEL_50;    break;
        case eWAP_POWERLEVEL_30:    ucWap_Level = (UINT8)eWAP_LEVEL_30;    break;
        case eWAP_POWERLEVEL_10:    ucWap_Level = (UINT8)eWAP_LEVEL_10;    break;
        default:
            LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d) unsupport WAP Level %d\r\n", eWap_Level);
            return rcERROR;
    }

    ucWAP_PWM[0] = ucWap_Mode;
    ucWAP_PWM[1] = ucWap_Level;
    ucWAP_PWM[2] = ucWap_LDSeq;

    ucWAP_PWM[3] = LSB(ucPWM);
    ucWAP_PWM[4] = MSB(ucPWM);

    eResult = dv442x_Write(CMDF_WAP, ucWAP_PWM); //out to DDP = 3+2 =5

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): WAP_Set PWM[Display Mode : %d][Level : %d][Seg : %d] = [%d] = (%d,%d)\r\n", __FUNCTION__, __LINE__, ucWAP_PWM[0], ucWAP_PWM[1],ucWAP_PWM[2], ucWAP_PWM[3],ucWAP_PWM[4]);

    return eResult;
}

eRESULT halFormatter_WAP_Get(UINT16* ucPWM) //G50_Casper_0006
{
    eRESULT eResult = rcINVALID;
    UINT8   ucDisplayMode = 0;
    UINT8   ucLevenNum = 0;
    UINT8   ucWAP_PWM[14] = {0};

    eResult = dv442x_Read(CMDF_WAP, ucWAP_PWM); //read from DDP = 2+6*2 = 14

    ucDisplayMode = ucWAP_PWM[0];
    ucLevenNum = ucWAP_PWM[1];
    *ucPWM = ucWAP_PWM[3] << 8 | ucWAP_PWM[2];
    *(ucPWM+1) = ucWAP_PWM[5] << 8 | ucWAP_PWM[4];
    *(ucPWM+2) = ucWAP_PWM[7] << 8 | ucWAP_PWM[6];
    *(ucPWM+3) = ucWAP_PWM[9] << 8 | ucWAP_PWM[8];
    *(ucPWM+4) = ucWAP_PWM[11] << 8 | ucWAP_PWM[10];
    *(ucPWM+5) = ucWAP_PWM[13] << 8 | ucWAP_PWM[12];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): APCCT [Level: %d][Display Mode : %d] PWM Get= (BLD:%d,%d,%d,%d / RLD:%d,%d)\r\n", __FUNCTION__, __LINE__,
	ucLevenNum,ucDisplayMode,*ucPWM,*(ucPWM+1),*(ucPWM+2),*(ucPWM+3),*(ucPWM+4),*(ucPWM+5));

    return eResult;
}

eRESULT halFormatter_DB_StrengthSet(UINT8 ucValue)         //A70LV_Doulas_0327
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    Datax[0] = ucValue;

    eResult = dv442x_Write(CMDF_DBSTRENGTH, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,%d):halFormatter_DB_StrengthSet %d\r\n", __FUNCTION__, __LINE__,ucValue);
    return eResult;
}

eRESULT halFormatter_DB_StrengthGet(UINT8 *ucValue)        //A70LV_Doulas_0327
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_DBSTRENGTH, Datax);

    *ucValue = Datax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,%d):halFormatter_DB_StrengthGet %d\r\n", __FUNCTION__, __LINE__,*ucValue);
    return eResult;
}

eRESULT halFormatter_FRC_BypassSet(UINT8 ucValue)
{
    eRESULT  eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};

	ucFRC_Bypass_Enable_Flag = ucValue;		//A70LK_Doulas_0008 Add
    ucDatax[0] = ucValue;
    eResult = dv442x_Write(CMDF_FRCBYPASSMODE, ucDatax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): %d\r\n", __FUNCTION__, __LINE__, ucValue);
    return eResult;
}

eRESULT halFormatter_FRC_BypassGet(UINT8 *ucValue)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_FRCBYPASSMODE, Datax);
    *ucValue = Datax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d): %d\r\n", __FUNCTION__, __LINE__,*ucValue);
    return eResult;
}

UINT8 halFormatter_FRC_Bypass_Flag_Get(void)	 //A70LK_Doulas_0008
{
    return ucFRC_Bypass_Enable_Flag;
}

eRESULT halFormatter_BlankSignalSwitchSet(UINT8 ucEnable)         //ZU860_Doulas_0083
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    Datax[0] = ucEnable;

    eResult = dv442x_Write(CMDF_BLACKSIGNALSWITCH, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_BlankSignalSwitchSet %d\r\n", __FUNCTION__, __LINE__,ucEnable);
    return eResult;
}

eRESULT halFormatter_BlankSignalSwitchGet(UINT8 *ucEnable)        //ZU860_Doulas_0083
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[3]={0,0,0};

    eResult = dv442x_Read(CMDF_BLACKSIGNALSWITCH, Datax);

    *ucEnable = Datax[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):halFormatter_BlankSignalSwitchGet %d\r\n", __FUNCTION__, __LINE__,*ucEnable);
    return eResult;
}

eRESULT halFormatter_AspectRatio_Set(UINT8* ucVAl) //ZU860_Doulas_0119
{
    eRESULT eResult = rcINVALID;

#ifdef PLATFORM_H60_2K
    eResult = dv442x_Write(CMDF_ASPECT_RATIO, ucVAl);
#endif /* PLATFORM_H60_2K */

    LOG_MSG(db_HAL_FORMATTER, "AspectRatio_Set = %d\r\n", *ucVAl);

    return eResult;
}

eRESULT halFormatter_AspectRatio_Get(UINT8* ucVAl) //ZU860_Doulas_0119
{
    eRESULT eResult = rcINVALID;

#ifdef PLATFORM_H60_2K
    eResult = dv442x_Read(CMDF_ASPECT_RATIO, ucVAl);
#endif /* PLATFORM_H60_2K */

    LOG_MSG(db_HAL_FORMATTER, "AspectRatio_Get = \r\n", *ucVAl);


    return eResult;
}

eRESULT halFormatter_Smooth_Enable(BOOL bEnable, UINT8 ucValue)	//G100_Clare_0004
{
    eRESULT eResult = rcINVALID;
    UINT8 ucData[2] = {bEnable, ucValue};
    LOG_MSG(db_HAL_FORMATTER, "\r\n halFormatter_Smooth_Enable = %02d, %d \r\n", ucData[0],ucData[1]);

    eResult = dv442x_Write(CMDF_SMOOTH_ON, ucData);

    return eResult;
}
UINT8 halFormatter_Smooth_Enable_Get(UINT8* ucVAl)	//G100_Clare_0004
{
    UINT8 ucDatax[2] = {0};

    dv442x_Read(CMDF_SMOOTH_ON, ucDatax);

    *ucVAl = ucDatax[0];

    LOG_MSG(db_HAL_FORMATTER, " halFormatter_Smooth_Enable_Get = %d\r\n", *ucVAl);

    return *ucVAl;
}

eRESULT halFormatter_GetRLD_Light(UINT16 *RLD_Light)		//G100_Doulas_0010
{
    eRESULT eResult = rcINVALID;
    UINT8   ucDatax[6] = {0};

    eResult = dv442x_Read(CMDF_FANDEVICE, ucDatax);

    cmdGetHalfword(&RLD_Light[0],&ucDatax[0]);    //ZU860_Doulas_0046 modify
    cmdGetHalfword(&RLD_Light[1],&ucDatax[2]);  //ZU860_Doulas_0046 modify


	LOG_MSG(db_HAL_FORMATTER, "RLD light (%d,%d)\r\n",RLD_Light[0],RLD_Light[1]);
    return eResult;
}


eRESULT halFormatter_RX24TVersion_Get(UINT16* ucVersion) //HICC2_Doulas_0018
{
    eRESULT eResult = rcINVALID;
    UINT8 ucDatax[2] = {0};

    eResult = dv442x_Read(CMDF_RX24TVERSION, ucDatax);

    cmdGetHalfword(ucVersion,&ucDatax[0]);

    LOG_MSG(db_HAL_FORMATTER, " halFormatter_RX24TVersion_Get = %d %d\r\n", ucDatax[0], ucDatax[1]);

    return eResult;
}

eRESULT halFormatter_Extern_3D_SYNC_In_Double_Set(UINT8 ucDouble) //H30K_Doulas_0007
{
    eRESULT eResult = rcINVALID;

#ifndef FRAME_SEQUENTIAL_3D_ULTRA
    eResult = dv442x_Write(CMDF_RX24TVERSION, &ucDouble);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):FRCByPassModeSet %d\r\n", __FUNCTION__, __LINE__,ucDouble);
#endif

    return eResult;
}

#ifdef Low_Latency_All
eRESULT halFormatter_FRCByPassModeSet(BOOL ucVAl)
{
    eRESULT  eResult = rcINVALID;
    eResult = dv442x_Write(CMDF_FRCBYPASSMODE, &ucVAl);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):FRCByPassModeSet %d\r\n", __FUNCTION__, __LINE__,ucVAl);
    return eResult;
}

eRESULT halFormatter_FRCByPassModeGet(UINT8* ucVAl)
{
    eRESULT eResult = rcINVALID;
    eResult = dv442x_Read(CMDF_FRCBYPASSMODE, ucVAl);
    LOG_MSG(db_HAL_FORMATTER, "(func:%s, line:%d):FRCByPassModeGet %d\r\n", __FUNCTION__, __LINE__,*ucVAl);
    return eResult;
}
#endif	/*Low_Latency_All*/

eRESULT halFormatter_LDFadeInStartupSet(UINT8 ucShutterStartup, UINT8 ucShutterFadeIn)			//G100_Doulas_0024
{
    eRESULT eResult = rcINVALID;
#if 0 //HICC2_Doulas_0001 DDP no command
    UINT8 ucData[2] = {ucShutterStartup, ucShutterFadeIn};

    LOG_MSG(db_HAL_FORMATTER, "\r\n halFormatter_LDFadeInStartupSet = %02d, %d \r\n", ucData[0],ucData[1]);

    eResult = dv442x_Write(CMDF_LD_FADE_IN_STARTUP, &ucData[0]);
#endif
    return eResult;
}

eRESULT halFormatter_SplashAtStartupTimeoutSet(UINT8 ucTimeout)								//G100_Doulas_0024
{
    eRESULT eResult = rcINVALID;
    UINT8 ucData = ucTimeout;

    LOG_MSG(db_HAL_FORMATTER, "\r\n halFormatter_SplashAtStartupTimeoutSet = %d \r\n", ucData);

    eResult = dv442x_Write(CMDF_DBAVAILABLE, &ucData);		//set power on Splash timeout

    return eResult;
}

#if 0
void halFormatter_SKUTypeSet(BOOL ucVAl)
{
	bSKU_Type = ucVAl;
}

BOOL halFormatter_SKUTypeGet(void)	//G100_Doulas_0080
{
	return bSKU_Type;
}
#endif

void halFormatter_DimPower_SetDefault(void)
{
    m_ucCurrentDimPower = 100;
}

eRESULT halFormatter_DimPower_Set(BOOL bEnable, UINT8 ucValue, BOOL bCheck)
{
    eRESULT eResult = rcINVALID;
#if defined(PLATFORM_A70G2) || defined(PLATFORM_H60_2K) || defined(PLATFORM_H30_4K)	//HICC2_Doulas_0001
    UINT8 ucData[2] = {bEnable, ucValue};

    ucData[0] = bEnable;

    if(bCheck)
    {
        if(m_ucCurrentDimPower >= ucValue)
        {
            m_ucCurrentDimPower = ucValue;
        }
        ucData[1] = m_ucCurrentDimPower;
    }
    else
    {
        ucData[1] = ucValue;
    }

    LOG_MSG(db_HAL_FORMATTER, "\r\n halFormatter_DimPower_Set(%d) = %02d, %d \r\n", bCheck, ucData[0],ucData[1]);

    eResult = dv442x_Write(CMDF_DIMPOWER, ucData);
#endif
    return eResult;
}

eRESULT halFormatter_AutoLockResync(void)
{
    eRESULT eResult = rcINVALID;
    UINT8 Datax[1]={0};

    eResult = dv442x_Write(CMDF_RESYNC, Datax);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,%d):halFormatter_AutoLockResync\r\n", __FUNCTION__, __LINE__);
    return eResult;
}

eRESULT halFormatter_GetI2C_Diag(UINT8 *pdata)
{
    eRESULT eResult = rcINVALID;
    UINT8   ucDatax[8] = {0};

#if 0
    eResult = dv442x_Read(CMDF_I2C_DIAG, &ucDatax[0]);

    memcpy(pdata, ucDatax, 8);
#endif

	LOG_MSG(db_HAL_FORMATTER, "(func:%s,%d):halFormatter_GetI2C_Diag Work %02x%02x%02x%02x, Error %02x%02x%02x%02x\r\n", __FUNCTION__, __LINE__, ucDatax[0], ucDatax[1], ucDatax[2], ucDatax[3], ucDatax[4], ucDatax[5], ucDatax[6], ucDatax[7]);
    return eResult;
}

eRESULT halFormatter_XPR_CloseLoop_ErrorCode0(UINT32 *ucValue) //HICC2_Steven_0051 start
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrClsError0;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR close loop Error code0 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);

#endif
}

eRESULT halFormatter_XPR_CloseLoop_ErrorCode1(UINT32 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[8] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrClsError1;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    memcpy((uint8*)ucValue, ucReadData, 4);

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR close loop Error code1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);

#endif
}

eRESULT halFormatter_XPR_CloseLoop_Valid(UINT8 *ucValue)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 ucDatax[2] = {0};
    UINT8 ucReadData[4] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    ucDatax[0] = FPGAcontrol_ActrClslpValid;

    eResult &= dv442x_Write(CMDF_FPGACTRL_SETGET, ucDatax);

    eResult &= dv442x_Read(CMDF_FPGACTRL_WRRD, ucReadData);

    *ucValue = ucReadData[0];

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR close loop Error code1 Get [%d]\r\n", __FUNCTION__, __LINE__, *ucValue);

#endif
}

eRESULT halFormatter_XPR_Close_Loop_Error_Reset_Set(UINT8 ucDatax)
{
    eRESULT eResult = rcSUCCESS;
    UINT8 acDatax[2] = {0};

#if defined(PLATFORM_A70LK) || defined(PLATFORM_H60_4K)
    acDatax[0] = FPGAcontrol_ActrClslpErrorReset;
    acDatax[1] = ucDatax;

    eResult = dv442x_Write(CMDF_FPGACTRL_WRRD, acDatax);
#endif

    LOG_MSG(db_HAL_FORMATTER, "(func:%s,#%d): XPR Axis0 Close Loop Error Reset %d\r\n", __FUNCTION__, __LINE__, ucDatax);
    return eResult;
} //HICC2_Steven_0051 end

void halFormatter_Bist(void *pvData)  //H30K_Steven_0001
{
    eRESULT lResult = rcSUCCESS;
    UINT8 aucData[4];
    sBIST_DEV *psDev = (sBIST_DEV*)pvData;
    UINT16 wVerDDPApp = 0;

    psDev->lValid = 1;

    lResult = halFormatter_SYS_AppVersionGet(&wVerDDPApp);

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

