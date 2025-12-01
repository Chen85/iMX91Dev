#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <sys/file.h>
#include "cmd_ap.h"
#include "conf.h"
#include "hicc_config.h"
#include "conf_conv_cmd.h"
#include "hpbulib_file_path.h"
//=================================================================================================
#define MAX_ARGC 256
#define BUF_SIZE 100*1024
//=================================================================================================
pthread_mutex_t g_ConfConvMutex[eCONF_CONV_MUTEX_MAX];
//=================================================================================================
static BOOL ConfConvCmd_MutexGive(u8_t Type)
{
    if(pthread_mutex_unlock(&g_ConfConvMutex[Type]) != 0)
    {
        return FALSE;
    }

    return TRUE;
}
//-------------------------------------------------------------------------------------------------
static BOOL ConfConvCmd_MutexTake(u8_t Type)
{
    if (pthread_mutex_lock(&g_ConfConvMutex[Type]) != 0)
    {
        return FALSE;
    }

    return TRUE;
}
//-------------------------------------------------------------------------------------------------
u32_t ConfConvCmd_getProjectGroupUnitCmd(_sProjectorInfo *pInfo, char *pHeaderStr, char *pArgv[][2], int Argc)
{
    char Name[128];
    char *pStr;

    if((pInfo == NULL) || (pHeaderStr == NULL) || (pArgv == NULL))
    {
        return 1;
    }
    if(Argc == 0)
    {
        return 2;
    }

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_STATUS);
    pStr = findArgValue(Name, pArgv, Argc);
    pInfo->ProjectorStatus = atoi(pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_GROUP_STATUS);
    pStr = findArgValue(Name, pArgv, Argc);
    pInfo->GroupStatus = atoi(pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_GROUP_ID_STATUS);
    pStr = findArgValue(Name, pArgv, Argc);
    pInfo->GroupId = atoi(pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_SN);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->SN, MAX_PROJECTOR_STR_LEN, "%s", pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_PROJ_NAME);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->ProjectorName, MAX_PROJECTOR_STR_LEN, "%s", pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_GROUP_NAME);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->GroupName, MAX_PROJECTOR_STR_LEN, "%s", pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_MODEL_NAME);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->ModeName, MAX_PROJECTOR_STR_LEN, "%s", pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_IP);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->Ip, MAX_PROJECTOR_STR_LEN, "%s", pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_MAC);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->Mac, MAX_PROJECTOR_STR_LEN, "%s", pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_CAMERA_STATUS);
    pStr = findArgValue(Name, pArgv, Argc);
    pInfo->CameraStatus = atoi(pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_FD);
    pStr = findArgValue(Name, pArgv, Argc);
    pInfo->Fd = atoi(pStr);

    snprintf(Name, sizeof(Name), "%s%s", pHeaderStr, NAME_GROUPING_GROUP_NOTE);
    pStr = findArgValue(Name, pArgv, Argc);
    snprintf(pInfo->Note, COMMON_ARRAYSIZE_64, "%s", pStr);
    return 0;
}
//-------------------------------------------------------------------------------------------------
u32_t ConfConvCmd_getProjectGroupSelectCmd(_sGroupSelectInfo *pInfo)
{
    u32_t Argc;
    char *pArgv[MAX_ARGC][2];
    char ConfBuf[BUF_SIZE];
    u32_t Idx, Cnt;
    char Name[256];

    if(pInfo == NULL)
    {
        return 1;
    }

    Argc = readConf(PATH_PROJECTOR_GROUP_SELECT, pArgv, MAX_ARGC, ConfBuf, sizeof(ConfBuf));
    if(Argc == 0)
    {
        memset(pInfo, 0, sizeof(_sGroupSelectInfo));
        return 2;
    }

    Cnt = atoi((findArgValue(NAME_GROUPING_CONFIG_NUMBER, pArgv, Argc)));
    if(Cnt == 0)
    {
        memset(pInfo, 0, sizeof(_sGroupSelectInfo));
        return 3;
    }
    pInfo->Cnt = Cnt;
    pInfo->master = atoi((findArgValue(NAME_GROUPING_CONFIG_GROUP_MASTER, pArgv, Argc)));
    for(Idx=0; Idx<Cnt;Idx++)
    {
        snprintf(Name, sizeof(Name), "%s%d_", NAME_GROUPING_GROUP_PREFIX, Idx);
        ConfConvCmd_getProjectGroupUnitCmd(&(pInfo->Info[Idx]), Name, pArgv, Argc);
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u32_t ConfConvCmd_getProjectGroupSearchCmd(_sGroupSearchInfo *pInfo)
{
    u32_t Argc;
    char *pArgv[MAX_ARGC][2];
    char ConfBuf[BUF_SIZE];
    u32_t Idx, Cnt;
    char Name[256];

    if(pInfo == NULL)
    {
        return 1;
    }

    Argc = readConf(PATH_PROJECTOR_GROUP_SEARCH, pArgv, MAX_ARGC, ConfBuf, sizeof(ConfBuf));
    if(Argc == 0)
    {
        memset(pInfo, 0, sizeof(_sGroupSearchInfo));
        return 2;
    }
    Cnt = atoi((findArgValue(NAME_GROUPING_CONFIG_NUMBER, pArgv, Argc)));
    pInfo->Cnt = Cnt;
    if(Cnt == 0)
    {
        memset(pInfo, 0, sizeof(_sGroupSearchInfo));
        return 3;
    }
    for(Idx=0; Idx<Cnt;Idx++)
    {
        snprintf(Name, sizeof(Name), "%s%d_", NAME_GROUPING_GROUP_PREFIX, Idx);
        ConfConvCmd_getProjectGroupUnitCmd(&(pInfo->Info[Idx]), Name, pArgv, Argc);
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u32_t ConfConvCmd_setProjectorGroupUnitArgv(char *pArgv[][2], char *pBuf, u32_t Len, u32_t *pOffset, char *pHeaderStr, _sProjectorInfo *pInfo)
{
    #define GROUP_INTEGER_RESERVED_SIZE 10
    #define GROUP_STRING_ENDLINE_NUM    3

    if(ConfConvCmd_MutexTake(eCONF_CONV_MUTEX_SET_UNIT))
    {
        if((pArgv == NULL)|| (pBuf == NULL) || (pOffset == NULL) || (pInfo == NULL))
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 1;
        }
    // set projector status
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_STATUS) + GROUP_INTEGER_RESERVED_SIZE + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 2;
        }

        pArgv[0][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_STATUS);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[0][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%d", pInfo->ProjectorStatus);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // set group status
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_GROUP_STATUS) + GROUP_INTEGER_RESERVED_SIZE + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 3;
        }

        pArgv[1][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_GROUP_STATUS);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[1][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%d", pInfo->GroupStatus);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // set group id
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_GROUP_ID_STATUS) + GROUP_INTEGER_RESERVED_SIZE + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 4;
        }

        pArgv[2][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_GROUP_ID_STATUS);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[2][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%d", pInfo->GroupId);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // set SN
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_SN) + strlen(pInfo->SN) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 5;
        }

        pArgv[3][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_SN);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[3][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->SN);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // set projector name
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_PROJ_NAME) + strlen(pInfo->ProjectorName) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 6;
        }

        pArgv[4][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_PROJ_NAME);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[4][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->ProjectorName);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // set group name
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_GROUP_NAME) + strlen(pInfo->GroupName) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 7;
        }

        pArgv[5][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_GROUP_NAME);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[5][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->GroupName);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // set model name
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_MODEL_NAME) +strlen(pInfo->ModeName) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 8;
        }

        pArgv[6][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_MODEL_NAME);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[6][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->ModeName);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // ip
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_IP) + strlen(pInfo->Ip) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 9;
        }

        pArgv[7][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_IP);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[7][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->Ip);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // Mac
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_MAC) + strlen(pInfo->Mac) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 10;
        }

        pArgv[8][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_MAC);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[8][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->Mac);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // Camera status
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_CAMERA_STATUS) + GROUP_INTEGER_RESERVED_SIZE + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 11;
        }

        pArgv[9][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_CAMERA_STATUS);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[9][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%d", pInfo->CameraStatus);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // FD
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_FD) + GROUP_INTEGER_RESERVED_SIZE + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 11;
        }

        pArgv[10][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_FD);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[10][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%d", pInfo->Fd);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
    // Note
        if((*pOffset + strlen(pHeaderStr) + strlen(NAME_GROUPING_GROUP_NOTE) + strlen(pInfo->Note) + GROUP_STRING_ENDLINE_NUM) >= Len)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
            return 12;
        }

        pArgv[11][0] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s%s", pHeaderStr, NAME_GROUPING_GROUP_NOTE);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        pArgv[11][1] = pBuf + *pOffset;
        *pOffset += snprintf(pBuf + *pOffset, Len - *pOffset, "%s", pInfo->Note);
        *(pBuf + *pOffset) = '\0';
        *pOffset = *pOffset + 1;
        
        ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_UNIT);
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
u32_t ConfConvCmd_writeProjectGroupSearchConfByCmd(_sGroupSearchInfo *pData)
{
    char *pArgv[MAX_ARGC][2];
    u32_t Argc;
    char Buf[BUF_SIZE];
    char CntName[MAX_PROJECTOR_STR_LEN];
    char CntVauleStr[MAX_PROJECTOR_STR_LEN];
    u32_t Offset = 0;
    u32_t Idx;
    char HeaderStr[COMMON_ARRAYSIZE_64];

    if(ConfConvCmd_MutexTake(eCONF_CONV_MUTEX_SET_SEARCH_LIST))
    {
        if(pData== NULL)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_SEARCH_LIST);
            return 1;
        }

        for(Idx=0; Idx<pData->Cnt; Idx++)
        {
            snprintf(HeaderStr, sizeof(HeaderStr), "%s%d_", NAME_GROUPING_GROUP_PREFIX, Idx);
            ConfConvCmd_setProjectorGroupUnitArgv(&pArgv[Idx * NUM_GROUPING_GROUP_PARAM], Buf, sizeof(Buf) , &Offset, HeaderStr, &(pData->Info[Idx]));
        }

        snprintf(CntName, sizeof(CntName), "%s" , NAME_GROUPING_CONFIG_NUMBER);
        snprintf(CntVauleStr, sizeof(CntVauleStr), "%d" , pData->Cnt);
        pArgv[pData->Cnt * NUM_GROUPING_GROUP_PARAM][0] = CntName;
        pArgv[pData->Cnt * NUM_GROUPING_GROUP_PARAM][1] = CntVauleStr;
        writeConf(PATH_PROJECTOR_GROUP_SEARCH, pArgv, (pData->Cnt * NUM_GROUPING_GROUP_PARAM)+1 );
        
        ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_SEARCH_LIST);
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------
u32_t ConfConvCmd_writeProjectGroupSelectConfByCmd(_sGroupSelectInfo *pData)
{
    char *pArgv[MAX_ARGC][2];
    u32_t Argc;
    char Buf[BUF_SIZE];
    char CntName[MAX_PROJECTOR_STR_LEN], MasterName[MAX_PROJECTOR_STR_LEN];
    char CntVauleStr[MAX_PROJECTOR_STR_LEN], MasterValueStr[MAX_PROJECTOR_STR_LEN];
    u32_t Offset = 0;
    u32_t Idx;
    char HeaderStr[COMMON_ARRAYSIZE_64];
    u32_t i;

    if(ConfConvCmd_MutexTake(eCONF_CONV_MUTEX_SET_SELECT_LIST))
    {
        if(pData== NULL)
        {
            ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_SELECT_LIST);
            return 1;
        }

        for(Idx=0; Idx<pData->Cnt; Idx++)
        {
            snprintf(HeaderStr, sizeof(HeaderStr), "%s%d_", NAME_GROUPING_GROUP_PREFIX, Idx);
            ConfConvCmd_setProjectorGroupUnitArgv(&pArgv[Idx * NUM_GROUPING_GROUP_PARAM], Buf, sizeof(Buf) , &Offset, HeaderStr, &(pData->Info[Idx]));
        }

        snprintf(CntName, sizeof(CntName), "%s" , NAME_GROUPING_CONFIG_NUMBER);
        snprintf(CntVauleStr, sizeof(CntVauleStr), "%d" , pData->Cnt);
        pArgv[pData->Cnt * NUM_GROUPING_GROUP_PARAM][0] = CntName;
        pArgv[pData->Cnt * NUM_GROUPING_GROUP_PARAM][1] = CntVauleStr;

        snprintf(MasterName, sizeof(MasterName), "%s" , NAME_GROUPING_CONFIG_GROUP_MASTER);
        snprintf(MasterValueStr, sizeof(MasterValueStr), "%d" , pData->master);
        pArgv[(pData->Cnt * NUM_GROUPING_GROUP_PARAM) + 1][0] = MasterName;
        pArgv[(pData->Cnt * NUM_GROUPING_GROUP_PARAM) + 1][1] = MasterValueStr;
        writeConf(PATH_PROJECTOR_GROUP_SELECT, pArgv, (pData->Cnt * NUM_GROUPING_GROUP_PARAM)+ 2);

        ConfConvCmd_MutexGive(eCONF_CONV_MUTEX_SET_SELECT_LIST);
    }
    return 0; 
}
//-------------------------------------------------------------------------------------------------
void ConfConvCmd_MutexInit()
{
    u8_t Index;
    for(Index = 0; Index < eCONF_CONV_MUTEX_MAX; Index++)
    {
        if(pthread_mutex_init(&g_ConfConvMutex[Index], NULL) != 0)
        {
            printf("(func:%s, line:%d): Create ConfConvMutex Fail!\r\n", __FUNCTION__, __LINE__);
        }
    }
}
