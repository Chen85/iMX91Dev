#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <sys/file.h>
#include "CliCfgAccess.h"
#include "conf.h"
#include "CommonType.h"
#include "hicc_config.h"
#include "GeneralFunc.h"

sCLI_SETTING sCliCfg = {0};
sCMD_SETTING sCmdCfg = {0};
sOSDINIT_SETTING sOSDCfg = {0, NULL};

INT8 InitCliFormatCfg(void)
{
    FILE *fp = fopen( CLI_FORMAT_FILE_PATH ,"rb");
    char cStrTemp[1023] = {'\0'};
    char ItemName[32] = {'\0'}, ItemIndex[32] = {'\0'};

    if(fp == NULL)
    {
        printf("Error: No CliFormat.cfg File (%s)\n", CLI_FORMAT_FILE_PATH);
        return FILE_FAIL;
    }

    while (!feof(fp))
    {
        fgets(cStrTemp,1023,fp);

        if(feof(fp))
        {
            break;//return 1;
        }
        sscanf(cStrTemp, "%31[^,],%31[^\r\n]", ItemName, ItemIndex);

        if(strncmp(ItemName, "General_RS232", 13) == 0)//barco
        {
            sCliCfg.ucCLI_Format[eCLI_STYLE_GENERAL] = atoi(ItemIndex);
        }
        else if(strncmp(ItemName, "Christie_RS232", 14) == 0)//christie
        {
            sCliCfg.ucCLI_Format[eCLI_STYLE_CHRISTIE] = atoi(ItemIndex);
        }
        else if(strncmp(ItemName, "Optoma_RS232", 12) == 0)//optoma
        {
            sCliCfg.ucCLI_Format[eCLI_STYLE_OPTOMA] = atoi(ItemIndex);
        }
    }

    fclose(fp);
    return FILE_PASS;
}

INT8 InitRs232CmdCfg(void)
{
    FILE *fp = fopen( CMD_FILE_PATH ,"rb");
    char cStrTemp[1023] = {'\0'}, cStrTemp2[1023] = {'\0'};
    char ItemName[64] = {'\0'}, ItemIndex[64] = {'\0'}, ItemValue[64] = {'\0'};
    UINT16 cVerCount = 0, cCmdCount = 0;

    if(fp == NULL)
    {
        printf("Error: No Rs232Cmd.cfg File (%s)\n", CMD_FILE_PATH);
        return FILE_FAIL;
    }

    while (!feof(fp))
    {
        fgets(cStrTemp,1023,fp);

        if(feof(fp))
        {
            break;//return 1;
        }

        sscanf(cStrTemp, "%63[^,],%1023[^\n]", ItemName, cStrTemp2);

        if(strncmp(ItemName, "VERinfo", 7) == 0)//version
        {
            sscanf(cStrTemp2, "%63[^,],%63[^\r\n]", ItemIndex, ItemValue);

            sCmdCfg.sVer_Info[cVerCount].cIndex = atoi(ItemIndex);
            memcpy(sCmdCfg.sVer_Info[cVerCount].cString, ItemValue, strlen(ItemValue)+1);

            cVerCount++;
            sCmdCfg.cTotalNum_Ver = cVerCount;
            //printf("DataCode#%04d= %s\n", atoi(ItemIndex), ItemValue);
        }
        else if(strncmp(ItemName, "CMD", 3) == 0)//command
        {
            //sscanf(cStrTemp2, "%63[^=]=%63[^\r\n]", ItemIndex, ItemValue);

            sscanf(cStrTemp2, "%[^,],%[^,],%[^\n]", (char*)&sCmdCfg.sCmd_Info[cCmdCount].cMainCmd,
                                                     (char*)&sCmdCfg.sCmd_Info[cCmdCount].cSubCmd,
                                                     (char*)&sCmdCfg.sCmd_Info[cCmdCount].cCmd_RW);

            //uCmdCfg.sCmd_Info[cCmdCount].cCMD_Support = atoi(ItemValue);

            cCmdCount++;
            sCmdCfg.cTotalNum_Cmd = cCmdCount;
            //printf("Count%03d: support=%d, Command= %s\n", cCmdCount, atoi(ItemValue), ItemIndex);
        }
    }

    fclose(fp);
    return FILE_PASS;
}

int CliCfg_Format_Get(UINT8 cStyle)
{
    return sCliCfg.ucCLI_Format[cStyle];
}

int CmdCfg_VersionInfo_Get(UINT16 cVerCount, UINT32 *pcDatacode, char *pcStr)
{
    UINT16 uiTotalNum = sCmdCfg.cTotalNum_Ver;

    if(cVerCount < uiTotalNum)
    {
        *pcDatacode = sCmdCfg.sVer_Info[cVerCount].cIndex;
        strcpy(pcStr, sCmdCfg.sVer_Info[cVerCount].cString);
    }

    return uiTotalNum;
}

#if 0
int CmdCfg_CommandInfo_Get(UINT16 cCmdCount)
{
    if(cCmdCount < uCmdCfg.cTotalNum_Cmd)
    {
        return uCmdCfg.sCmd_Info[cCmdCount].cCMD_Support;
    }

    return 0;
}
#endif /* 0 */

int CmdCfg_CommandSupport_Get(char* pcMainCmd, char* pcSubCmd, BOOL IsRead)
{
    UINT16 uiCount = 0;

    if(IsRead)
    {
        if(pcMainCmd != NULL && pcSubCmd == NULL)
        {
            for(uiCount = 0; uiCount < sCmdCfg.cTotalNum_Cmd; uiCount++)
            {
                if(strncmp("READ", sCmdCfg.sCmd_Info[uiCount].cCmd_RW, 4) == 0)
                {
                    if((strcmp(pcMainCmd, sCmdCfg.sCmd_Info[uiCount].cMainCmd) == 0) &&
                       (strncmp("NULL", sCmdCfg.sCmd_Info[uiCount].cSubCmd, 4) == 0))
                    {
                        return 1;
                    }
                }
            }
        }
        else if(pcMainCmd != NULL && pcSubCmd != NULL)
        {
            for(uiCount = 0; uiCount < sCmdCfg.cTotalNum_Cmd; uiCount++)
            {
                if(strncmp("READ", sCmdCfg.sCmd_Info[uiCount].cCmd_RW, 4) == 0)
                {
                    if((strcmp(pcMainCmd, sCmdCfg.sCmd_Info[uiCount].cMainCmd) == 0) &&
                       (strcmp(pcSubCmd, sCmdCfg.sCmd_Info[uiCount].cSubCmd) == 0))
                    {
                        return 1;
                    }
                }
            }
        }
    }
    else
    {
        if(pcMainCmd != NULL && pcSubCmd == NULL)
        {
            for(uiCount = 0; uiCount < sCmdCfg.cTotalNum_Cmd; uiCount++)
            {
                if(strncmp("WRITE", sCmdCfg.sCmd_Info[uiCount].cCmd_RW, 5) == 0)
                {
                    if((strcmp(pcMainCmd, sCmdCfg.sCmd_Info[uiCount].cMainCmd) == 0) &&
                       (strncmp("NULL", sCmdCfg.sCmd_Info[uiCount].cSubCmd, 4) == 0))
                    {
                        return 1;
                    }
                }
            }
        }
        else if(pcMainCmd != NULL && pcSubCmd != NULL)
        {
            for(uiCount = 0; uiCount < sCmdCfg.cTotalNum_Cmd; uiCount++)
            {
                if(strncmp("WRITE", sCmdCfg.sCmd_Info[uiCount].cCmd_RW, 5) == 0)
                {
                    if((strcmp(pcMainCmd, sCmdCfg.sCmd_Info[uiCount].cMainCmd) == 0) &&
                       (strcmp(pcSubCmd, sCmdCfg.sCmd_Info[uiCount].cSubCmd) == 0))
                    {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

INT8 InitOSDDafaultValueCmdCfg(void)
{
    FILE *fp = fopen( OSD_DEFAULTVALUE_FILE_PATH ,"rb");
    char cStrTemp[1023] = {'\0'}, cStrTemp2[1023] = {'\0'};
    char ItemName[64] = {'\0'};
    INT32  iValue = 0;
    UINT16  uiCount = 0;

    if(fp == NULL)
    {
        printf("Error: No OSDInit.cfg File (%s)\n", OSD_DEFAULTVALUE_FILE_PATH);
        return FILE_FAIL;
    }

    sOSDCfg.uiTotalCount = 0;

    if(sOSDCfg.psOSDDDef != NULL)
    {
        free(sOSDCfg.psOSDDDef);
    }

    while (!feof(fp)) //計算有幾個參數
    {
        fgets(cStrTemp,1023,fp);

        if(feof(fp))
        {
            break;//return 1;
        }

        sscanf(cStrTemp, "%[^,],%[^,],%d", (char*)&ItemName, (char*)&cStrTemp2, &iValue);

        if(strncmp(ItemName, "DEF", 3) == 0)
        {
            sOSDCfg.uiTotalCount++;
        }
    }

    fclose(fp);

    sOSDCfg.psOSDDDef = (sOSDDEFInfo*)malloc(sOSDCfg.uiTotalCount * sizeof(sOSDDEFInfo));

    fp = fopen( OSD_DEFAULTVALUE_FILE_PATH ,"rb");
    while (!feof(fp)) //將變數放入記憶體
    {
        fgets(cStrTemp,1023,fp);

        if(feof(fp))
        {
            break;//return 1;
        }

        sscanf(cStrTemp, "%[^,],%[^,],%d", (char*)&ItemName, (char*)&cStrTemp2, &iValue);

        if(strncmp(ItemName, "DEF", 3) == 0)
        {
            snprintf(sOSDCfg.psOSDDDef[uiCount].cString, 64, "%s\0", cStrTemp2);
            sOSDCfg.psOSDDDef[uiCount].cIndex = iValue;
            uiCount++;
        }
    }

    return FILE_PASS;
}

UINT16 InitOSDDafaultValueTotalCountGet(void)
{
    return sOSDCfg.uiTotalCount;
}

INT8 InitOSDDafaultValueGet(UINT16 uiCount, char *cString, INT32 *pcValue)
{
    if((sOSDCfg.psOSDDDef == NULL) || (uiCount >= sOSDCfg.uiTotalCount))
    {
        return 0;
    }

    snprintf(cString, 64, "%s\0", sOSDCfg.psOSDDDef[uiCount].cString);
    *pcValue = sOSDCfg.psOSDDDef[uiCount].cIndex;

    return 1;
}


