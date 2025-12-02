#ifndef _UTIL_BIST_C_
#define _UTIL_BIST_C_

#include "Common.h"

#if (BIST_ENABLE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilBIST.h"
#include "utilBISTTable_SYS.h"
#include "utilDbgMsg.h"
#include "appLedProcAPI.h"

#include "appDataMgr.h" //A70LK_Steven_0008

sBIST_CMD  sCmd = {0};
sBIST_DATA sBist_DATA = {0};

eMSG_BIST_STATUS m_eBIST_STATUS = eBIST_STATUS_INIT;  //HICC2_Steven_0012

int utilBIST_GetRow(type_BIST_Handle *phandle, const char *pfile_name)
{
	int ret = 1;
	FILE *pf1 = fopen(pfile_name, "r");
	if (pf1 == NULL)
	{
	    LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() fopen() == NULL\n", __FILE__, __LINE__, __func__);
		return(-1);
	}

	//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(*pseek=%d, length=%d)\n", __FILE__, __LINE__, __func__, *pseek, length);
	fseek(pf1, phandle->GetSeek, SEEK_SET);
	if(fgets(phandle->GetBuffer, phandle->Length, pf1) == NULL)
	{
		if(feof(pf1) != 0)
		{
			LOG_MSG(db_BIST, "%s<%d>: %s() feof()=%d\n", __FILE__, __LINE__, __func__, feof(pf1));
			ret = 0;
		}
		else
		{
			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%d) fgets() == NULL\n", __FILE__, __LINE__, __func__);
			ret = -2;
		}
	}
	else
	{
		//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s() %s\n", __FILE__, __LINE__, __func__, phandle->GetBuffer);
		phandle->GetSeek += strlen(phandle->GetBuffer) + LF_OFFSET;
		ret = 1;
	}

	fclose(pf1);
	return(ret);
}

int utilBIST_PutRow(type_BIST_Handle *phandle, const char *pfile_name)
{
	int ret = -1, i = 1;
	FILE *pf1;
	const char mode[2][2] = {"w", "a"};

	//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(*pseek=%d)\n", __FILE__, __LINE__, __func__, *pseek);
	if(phandle->PutSeek == 0)
	{
		i = 0;
	}
	pf1 = fopen(pfile_name, mode[i]);
	if (pf1 == NULL)
	{
		LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() fopen(%s) == NULL\n", __FILE__, __LINE__, __func__, pfile_name);
		return(ret);
	}

	fseek(pf1, phandle->PutSeek, SEEK_SET);
	if(fputs(phandle->PutBuffer, pf1) < 0)
	{
		LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() fputs() < 0\n", __FILE__, __LINE__, __func__);
		ret = -2;
	}
	else
	{
		//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s()\n%s", __FILE__, __LINE__, __func__, pbuffer);
		phandle->PutSeek += strlen(phandle->PutBuffer) + LF_OFFSET;
		ret = 1;
	}

	fclose(pf1);
	return(ret);
}

int utilBIST_ParseRow(type_BIST_Handle *phandle, type_Column_Info *pcol_info)
{
	int ret = -1, i = 0;
	char *str = strstr(phandle->GetBuffer, ",,");
	if(str != NULL)
	{
		LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(,,) %s\n", __FILE__, __LINE__, __func__, str);
		return(ret);
	}

    str = strtok(phandle->GetBuffer, "\r\n");
	if(phandle->Row == 0)
	{
		phandle->Col_Nums = 0;
		str = strtok(phandle->GetBuffer, ",");
		if(str == NULL)
		{
			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() str == NULL\n", __FILE__, __LINE__, __func__);
			return(ret);
		}
		else if(strcmp(str, "ID") != 0)
		{
			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() str = %s\n", __FILE__, __LINE__, __func__, str);
			return(ret);
		}
		//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c)=%s\n", __FILE__, __LINE__, __func__, 'A'+Col, str);
		strcpy(pcol_info[phandle->Col_Nums].Title, str);
		//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c,%s)\n", __FILE__, __LINE__, __func__, 'A'+*pcol, pcol_info[*pcol].Title);
		phandle->Col_Nums += 1;

		while(1)
		{
			str = strtok(NULL, ",");
			if(str == NULL)
			{
				//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(Col_Nums=%d)\n", __FILE__, __LINE__, __func__, phandle->Col_Nums);
				break;
			}
			//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c)=%s\n", __FILE__, __LINE__, __func__, 'A'+Col, str);
			strcpy(pcol_info[phandle->Col_Nums].Title, str);
			//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c,%s)\n", __FILE__, __LINE__, __func__, 'A'+*pcol, pcol_info[*pcol].Title);
			phandle->Col_Nums += 1;
		}
	}
	else
	{
		str = strtok(phandle->GetBuffer, ",");
		if(str == NULL)
		{
			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() str == NULL\n", __FILE__, __LINE__, __func__);
			return(ret);
		}
		else
		{
			//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c)=%s\n", __FILE__, __LINE__, __func__, 'A'+i, str);
			strcpy(pcol_info[i].AsciiValue, str);
			pcol_info[i].IntegerValue = atoi(pcol_info[i].AsciiValue);
			//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c,%s)=%d\n", __FILE__, __LINE__, __func__, 'A'+i, pcol_info[i].AsciiValue, pcol_info[i].IntegerValue);
		}

		for(i = 1; i < phandle->Col_Nums; i ++)
		{
			str = strtok(NULL, ",");
			if(str == NULL)
			{
				LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s() str == NULL\n", __FILE__, __LINE__, __func__);
				return(ret);
			}
			else
			{
				//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c)=%s\n", __FILE__, __LINE__, __func__, 'A'+i, str);
				strcpy(pcol_info[i].AsciiValue, str);
				pcol_info[i].IntegerValue = atoi(pcol_info[i].AsciiValue);
				//LOG_MSG(db_HAL_RESERVED19, "%s<%d>: %s(%c,%s)=%d\n", __FILE__, __LINE__, __func__, 'A'+i, pcol_info[i].AsciiValue, pcol_info[i].IntegerValue);
			}
		}
	}

	ret = 1;
	return(ret);
}

int utilBIST_SetCommand(type_BIST_Handle *phandle, type_Column_Info *pcol_info, type_Title_Index *ptitle_index)
{
	int ret = 0, i = 0;
	INT32 lDelayTime = 0;

	if (phandle->Row == 0)
	{
	    utilBIST_Status_Set(eBIST_STATUS_INIT);

	    // 初始化所有欄位為 0xFF（表示未找到）
	    memset(ptitle_index, 0xFF, sizeof(type_Title_Index));

	    // 欄位對應表
	    Title_Map title_map[] =
	    {
	        {"ID", 				&ptitle_index->ID},
	        {"Device", 			&ptitle_index->Device},
	        {"BUS", 			&ptitle_index->BUS},
	        {"Unit", 			&ptitle_index->Unit},
	        {"Used(0/1)", 		&ptitle_index->Used},
	        {"Test (0/1)", 		&ptitle_index->Test},
	        {"Test Count", 		&ptitle_index->TestCount},
	        {"Criterion Upper", &ptitle_index->CriterionUpper},
	        {"Criterion Low", 	&ptitle_index->CriterionLow},
	        {"Result", 			&ptitle_index->Result},
	        {"Test fail", 		&ptitle_index->Testfail},
	        {"Comm fail", 		&ptitle_index->Commfail},
	        {"Layer", 			&ptitle_index->Layer},
	        {"Statistics MAX", 	&ptitle_index->StatisticsMax},
	        {"Statistics AVG", 	&ptitle_index->StatisticsAvg},
	        {"Statistics MIN", 	&ptitle_index->StatisticsMin},
			{"Repair Code", 	&ptitle_index->RepairCode},
	    };

	    // 對應欄位標題
	    for (int i = 0; i < phandle->Col_Nums; i++)
	    {
	        int found = 0;
	        for (int j = 0; j < sizeof(title_map)/sizeof(title_map[0]); j++)
	        {
	            if (strcmp(pcol_info[i].Title, title_map[j].title) == 0)
	            {
	                *title_map[j].index_ptr = i;
	                found = 1;
	                break;
	            }
	        }
	        if (!found)
	        {
	            LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() Unknown column: %c=%s\n", __FILE__, __LINE__, __func__, 'A' + i, pcol_info[i].Title);
	            return -1;
	        }
	    }

	    // 確認所有欄位皆已正確對應（非 0xFF）
	    for (int j = 0; j < sizeof(title_map)/sizeof(title_map[0]); j++)
	    {
	        if (*title_map[j].index_ptr == 0xFF)
	        {
	            LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() Missing column: %s\n", __FILE__, __LINE__, __func__, title_map[j].title);
	            return -1;
	        }
	    }
	}
	else
	{
		//eMSG_BIST_MODULE eCMD_MODULE = BIST_MODUL_Number;
		//UINT8   ucData[8] = {0};
		//eRESULT eBIST_Result = rcSUCCESS;

		//sCmd = {0}; clean

		if((pcol_info[ptitle_index->Used].IntegerValue <= 0)
        || (pcol_info[ptitle_index->Test].IntegerValue <= 0)
        || (pcol_info[ptitle_index->TestCount].IntegerValue <= 0))
		{
			return(1);
		}

		memset(&sCmd, 0, sizeof(sBIST_DATA));
		memset(&sBist_DATA, 0, sizeof(sBIST_DATA));

		sCmd.lBistId = pcol_info[ptitle_index->ID].IntegerValue;
		sCmd.lLayer  = pcol_info[ptitle_index->Layer].IntegerValue;
		sCmd.lCount  = pcol_info[ptitle_index->TestCount].IntegerValue;

        LOG_MSG(db_BIST, "===%s(%s, BistId = %d, TestCount = %d)===\n", __func__,
            pcol_info[ptitle_index->Device].AsciiValue,
            pcol_info[ptitle_index->ID].IntegerValue,
            pcol_info[ptitle_index->TestCount].IntegerValue);

        if((UINT32)sCmd.lBistId >= BIST_REG_NUMBERS)
        {
            LOG_MSG(db_BIST, "[Error] Bist ID =%d\r\n", sCmd.lBistId);
            pcol_info[ptitle_index->Result].IntegerValue = -1;
        }
        else
        {
        	sBIST_LUT  *psBist = NULL;
        	sBIST_DEV  sDev = {0};

        	psBist = (sBIST_LUT  *)&m_asBistLut[sCmd.lBistId];

        	 if(psBist->pProc == NULL)
        	 {
        		 LOG_MSG(db_BIST, "[Error] Bist No Function=%d\r\n", sCmd.lBistId);
        		 pcol_info[ptitle_index->Result].IntegerValue = -1;
        		 //return 1;
        	 }
        	 else
        	 {
        		 if(psBist->lDevId < 0)
        		 {   //test loop in next layer
        			 //LOG_MSG(db_BIST, " Bist %d send cmd \r\n", sCmd.lBistId);
        			 utilBIST_Status_Set(eBIST_STATUS_START);
        			 psBist->pProc(&sDev);
        			 phandle->GetCommand = 1;
        			 utilBIST_Status_Set(eBIST_STATUS_IDLE);
        			 lDelayTime = 100;
        			 MS_SLEEP(lDelayTime);

        		 }
        		 else
        		 {   //test loop in current layer
        			 INT32      lIndex  = 0;
        			 sDev.lDevId = psBist->lDevId;
        			 sCmd.lLayer--;
        			 for(lIndex = 0; lIndex < sCmd.lCount; lIndex++)
        			 {
        				 psBist->pProc(&sDev);
        				 sBist_DATA.lTotalCnt++;
        				 sBist_DATA.lValid  	 = sDev.lValid;
        				 sBist_DATA.lLayer 		 = sCmd.lLayer;
        				 sBist_DATA.lCommuErrCnt += sDev.lCommuError;
        				 sBist_DATA.lErrorCnt    += sDev.lError;
        				 sBist_DATA.lAvg         += sDev.lData;

        				 if(sBist_DATA.lMax < sDev.lData)
        				 {
        					 sBist_DATA.lMax = sDev.lData;
        				 }

        				 if(sBist_DATA.lMin > sDev.lData)
        				 {
        					 sBist_DATA.lMin = sDev.lData;
        				 }

        				 LOG_MSG(db_BIST,
        	                    "Bist(%3d) Total=%2d, Err=%2d %2d, Max=%5d, Min=%5d\r\n",
        	                    psBist->lBistId,
								sBist_DATA.lTotalCnt,
								sBist_DATA.lErrorCnt,
								sBist_DATA.lCommuErrCnt,
								sBist_DATA.lMax,
								sBist_DATA.lMin);
        				 lDelayTime =  psBist->lDelay_mS;
        				 MS_SLEEP(lDelayTime);
        			 }
#if 0
        	    pcol_info[ptitle_index->Layer].IntegerValue = sBist_DATA.lLayer;
        	    lTotalError = sBist_DATA.lCommuErrCnt + sBist_DATA.lErrorCnt;

        	    if(sBist_DATA.lValid != 1)
        	    {
        	    	LOG_MSG(db_ASSERT, "!!! ERROR !!! sBIST_Data.Valid  %d\r\n", sBist_DATA.lValid);

        	        pcol_info[ptitle_index->Result].IntegerValue = -1;
        	    }
        	    else
        	    {
        	        	if(strcmp(pcol_info[ptitle_index->Unit].AsciiValue, "times") == 0)
        	        	{
        	        		if((strcmp("N/A", pcol_info[ptitle_index->CriterionUpper].AsciiValue) == 0)
        	        				|| (strcmp("N/A", pcol_info[ptitle_index->CriterionLow].AsciiValue) == 0))
        	        		{
        	        			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%s=%d) N/A\r\n", __FILE__, __LINE__, __func__,
        	        					pcol_info[ptitle_index->Device].AsciiValue, pcol_info[ptitle_index->ID].IntegerValue);

        	        			pcol_info[ptitle_index->Result].IntegerValue = -1;
        	        		}
        	        		else
        	        		{
        	        			pcol_info[ptitle_index->Result].IntegerValue = util_BIST_Check_Result( TRUE, pcol_info[ptitle_index->CriterionUpper].IntegerValue, pcol_info[ptitle_index->CriterionLow].IntegerValue, lTotalError);
#if 0
        	        			if(pcol_info[ptitle_index->Result].IntegerValue != -1)////////////////////////////////////////////////////////
        	        			{
        	        				if(sBIST_Data.RetryCnt < pcol_info[ptitle_index->CriterionLow].IntegerValue)
        	        				{
        	        					pcol_info[ptitle_index->Result].IntegerValue = -1;

        	        					LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s N/A\r\n", __FILE__, __LINE__, __func__);
        	        				}
        	        			}
#endif
        	        		}

        	        		sprintf(pcol_info[ptitle_index->StatisticsMax].AsciiValue, "%d", pcol_info[ptitle_index->TestCount].IntegerValue - lTotalError);
        	        	}
        	        	else
        	        	{
        	        		if((strcmp("N/A", pcol_info[ptitle_index->CriterionUpper].AsciiValue) == 0)
        	        				|| (strcmp("N/A", pcol_info[ptitle_index->CriterionLow].AsciiValue) == 0))
        	        		{
        	        			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%s=%d) N/A\r\n", __FILE__, __LINE__, __func__,
        	        					pcol_info[ptitle_index->Device].AsciiValue, pcol_info[ptitle_index->ID].IntegerValue);

        	        			pcol_info[ptitle_index->Result].IntegerValue = -1;
        	        		}
        	        		else
        	        		{
        	        			pcol_info[ptitle_index->Result].IntegerValue = util_BIST_Check_Result( TRUE, pcol_info[ptitle_index->CriterionUpper].IntegerValue, pcol_info[ptitle_index->CriterionLow].IntegerValue, lTotalError);
        	        		}

        	        		sprintf(pcol_info[ptitle_index->StatisticsMax].AsciiValue, "%d", sBist_DATA.lMax );
        	        		sprintf(pcol_info[ptitle_index->StatisticsMin].AsciiValue, "%d", sBist_DATA.lMin );
        	        		sprintf(pcol_info[ptitle_index->StatisticsAvg].AsciiValue, "%d", sBist_DATA.lAvg );
        	        	}
        	        }


        	    sprintf(pcol_info[ptitle_index->Layer].AsciiValue,    "%d", sBist_DATA.lLayer);
        	    sprintf(pcol_info[ptitle_index->Testfail].AsciiValue, "%d", sBist_DATA.lErrorCnt);
        	    sprintf(pcol_info[ptitle_index->Commfail].AsciiValue, "%d", sBist_DATA.lCommuErrCnt);

        	    if(pcol_info[ptitle_index->Result].IntegerValue == -1)
        	    {
        	        sprintf(pcol_info[ptitle_index->Result].AsciiValue, "FAIL");
        	        phandle->FailCount += 1;
        	    }
        	    else
        	    {
        	        sprintf(pcol_info[ptitle_index->Result].AsciiValue, "PASS");
        	    }

        	    unsigned int addr = 0;
        	    for(i = 0; i < phandle->Col_Nums; i ++)
        	    {
        	        sprintf(&phandle->PutBuffer[addr], "%s,", pcol_info[i].AsciiValue);
        	        addr += strlen(pcol_info[i].AsciiValue) + 1;
        	    }
        	    sprintf(&phandle->PutBuffer[addr-1], "\n");
        	    }
#endif
        	 }

#if 1
        	 if( phandle->GetCommand != 1)
        	 {
        		INT32     lTotalError = 0;
        		pcol_info[ptitle_index->Layer].IntegerValue = sBist_DATA.lLayer;
        	    lTotalError = sBist_DATA.lCommuErrCnt + sBist_DATA.lErrorCnt;

        	    if(sBist_DATA.lValid != 1)
        	    {
        	    	LOG_MSG(db_ASSERT, "!!! ERROR !!! sBIST_Data.Valid  %d\r\n", sBist_DATA.lValid);

        	        pcol_info[ptitle_index->Result].IntegerValue = -1;
        	    }
        	    else
        	    {
        	        	if(strcmp(pcol_info[ptitle_index->Unit].AsciiValue, "times") == 0)
        	        	{
        	        		if((strcmp("N/A", pcol_info[ptitle_index->CriterionUpper].AsciiValue) == 0)
        	        				|| (strcmp("N/A", pcol_info[ptitle_index->CriterionLow].AsciiValue) == 0))
        	        		{
        	        			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%s=%d) N/A\r\n", __FILE__, __LINE__, __func__,
        	        					pcol_info[ptitle_index->Device].AsciiValue, pcol_info[ptitle_index->ID].IntegerValue);

        	        			pcol_info[ptitle_index->Result].IntegerValue = -1;
        	        		}
        	        		else
        	        		{
        	        			pcol_info[ptitle_index->Result].IntegerValue = util_BIST_Check_Result( TRUE, pcol_info[ptitle_index->CriterionUpper].IntegerValue, pcol_info[ptitle_index->CriterionLow].IntegerValue, lTotalError);
#if 0
        	        			if(pcol_info[ptitle_index->Result].IntegerValue != -1)////////////////////////////////////////////////////////
        	        			{
        	        				if(sBIST_Data.RetryCnt < pcol_info[ptitle_index->CriterionLow].IntegerValue)
        	        				{
        	        					pcol_info[ptitle_index->Result].IntegerValue = -1;

        	        					LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s N/A\r\n", __FILE__, __LINE__, __func__);
        	        				}
        	        			}
#endif
        	        		}

        	        		sprintf(pcol_info[ptitle_index->StatisticsMax].AsciiValue, "%d", pcol_info[ptitle_index->TestCount].IntegerValue - lTotalError);
        	        	}
        	        	else
        	        	{
        	        		if((strcmp("N/A", pcol_info[ptitle_index->CriterionUpper].AsciiValue) == 0)
        	        				|| (strcmp("N/A", pcol_info[ptitle_index->CriterionLow].AsciiValue) == 0))
        	        		{
        	        			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%s=%d) N/A\r\n", __FILE__, __LINE__, __func__,
        	        					pcol_info[ptitle_index->Device].AsciiValue, pcol_info[ptitle_index->ID].IntegerValue);

        	        			pcol_info[ptitle_index->Result].IntegerValue = -1;
        	        		}
        	        		else
        	        		{
        	        			pcol_info[ptitle_index->Result].IntegerValue = util_BIST_Check_Result( FALSE, pcol_info[ptitle_index->CriterionUpper].IntegerValue, pcol_info[ptitle_index->CriterionLow].IntegerValue, lTotalError);
        	        		}

        	        		sprintf(pcol_info[ptitle_index->StatisticsMax].AsciiValue, "%d", sBist_DATA.lMax );
        	        		sprintf(pcol_info[ptitle_index->StatisticsMin].AsciiValue, "%d", sBist_DATA.lMin );
        	        		sprintf(pcol_info[ptitle_index->StatisticsAvg].AsciiValue, "%d", sBist_DATA.lAvg );
        	        	}
        	        }


        	    sprintf(pcol_info[ptitle_index->Layer].AsciiValue,    "%d", sBist_DATA.lLayer);
        	    sprintf(pcol_info[ptitle_index->Testfail].AsciiValue, "%d", sBist_DATA.lErrorCnt);
        	    sprintf(pcol_info[ptitle_index->Commfail].AsciiValue, "%d", sBist_DATA.lCommuErrCnt);

        	    if(pcol_info[ptitle_index->Result].IntegerValue == -1)
        	    {
        	        sprintf(pcol_info[ptitle_index->Result].AsciiValue, "FAIL");
        	        phandle->FailCount += 1;
        	    }
        	    else
        	    {
        	        sprintf(pcol_info[ptitle_index->Result].AsciiValue, "PASS");
        	    }

        	    unsigned int addr = 0;
        	    for(i = 0; i < phandle->Col_Nums; i ++)
        	    {
        	        sprintf(&phandle->PutBuffer[addr], "%s,", pcol_info[i].AsciiValue);
        	        addr += strlen(pcol_info[i].AsciiValue) + 1;
        	    }
        	    sprintf(&phandle->PutBuffer[addr-1], "\n");
        	    }
        }
#endif

        }

	}

	return(1);
}

int utilBIST_GetCommand(type_BIST_Handle *phandle, type_Column_Info *pcol_info, type_Title_Index *ptitle_index)
{
	int ret = rcINVALID, i = 0; //A70LK_Steven_0006

	 LOG_MSG(db_BIST, " Bist %d utilBIST_GetCommand \r\n", sCmd.lBistId);

	if(phandle->Row == 0)
	{

	}
	else
	{
		INT32 lTotalError = 0;

    	sBIST_LUT  *psBist = NULL;

    	psBist = (sBIST_LUT  *)&m_asBistLut[sCmd.lBistId];

        psBist->pProc(&sBist_DATA);

        pcol_info[ptitle_index->Layer].IntegerValue = sBist_DATA.lLayer;
        lTotalError = sBist_DATA.lCommuErrCnt + sBist_DATA.lErrorCnt;

        LOG_MSG(db_BIST, "Bist(%3d) Total=%2d, Err=%2d %2d, Max=%5d, Min=%5d\r\n", sCmd.lBistId,
       				sBist_DATA.lTotalCnt,
       				sBist_DATA.lErrorCnt,
       				sBist_DATA.lCommuErrCnt,
       				sBist_DATA.lMax,
       				sBist_DATA.lMin);


        if(sBist_DATA.lValid != 1)
        {
            LOG_MSG(db_ASSERT, "!!! ERROR !!! sBIST_Data.Valid  %d\r\n", sBist_DATA.lValid);

            pcol_info[ptitle_index->Result].IntegerValue = -1;
        }
        else
        {
        	if(strcmp(pcol_info[ptitle_index->Unit].AsciiValue, "times") == 0)
        	{
        		if((strcmp("N/A", pcol_info[ptitle_index->CriterionUpper].AsciiValue) == 0)
        				|| (strcmp("N/A", pcol_info[ptitle_index->CriterionLow].AsciiValue) == 0))
        		{
        			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%s=%d) N/A\r\n", __FILE__, __LINE__, __func__,
        					pcol_info[ptitle_index->Device].AsciiValue, pcol_info[ptitle_index->ID].IntegerValue);

        			pcol_info[ptitle_index->Result].IntegerValue = -1;
        		}
        		else
        		{
        			pcol_info[ptitle_index->Result].IntegerValue = util_BIST_Check_Result( TRUE, pcol_info[ptitle_index->CriterionUpper].IntegerValue, pcol_info[ptitle_index->CriterionLow].IntegerValue, lTotalError);

#if 0
        			if(pcol_info[ptitle_index->Result].IntegerValue != -1)////////////////////////////////////////////////////////
        			{
        				if(sBist_DATA.RetryCnt < pcol_info[ptitle_index->CriterionLow].IntegerValue)
        				{
        					pcol_info[ptitle_index->Result].IntegerValue = -1;

        					LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s N/A\r\n", __FILE__, __LINE__, __func__);
        				}
        			}
#endif
        		}

        		sprintf(pcol_info[ptitle_index->StatisticsMax].AsciiValue, "%d", pcol_info[ptitle_index->TestCount].IntegerValue - lTotalError);
        	}
        	else
        	{
        		if((strcmp("N/A", pcol_info[ptitle_index->CriterionUpper].AsciiValue) == 0)
        				|| (strcmp("N/A", pcol_info[ptitle_index->CriterionLow].AsciiValue) == 0))
        		{
        			LOG_MSG(db_ASSERT, "!!! ERROR !!! %s<%d>: %s(%s=%d) N/A\r\n", __FILE__, __LINE__, __func__,
        					pcol_info[ptitle_index->Device].AsciiValue, pcol_info[ptitle_index->ID].IntegerValue);

        			pcol_info[ptitle_index->Result].IntegerValue = -1;
        		}
        		else
        		{
        			pcol_info[ptitle_index->Result].IntegerValue = util_BIST_Check_Result( FALSE, pcol_info[ptitle_index->CriterionUpper].IntegerValue, pcol_info[ptitle_index->CriterionLow].IntegerValue, lTotalError);
        		}

        		sprintf(pcol_info[ptitle_index->StatisticsMax].AsciiValue, "%d", sBist_DATA.lMax );
        		sprintf(pcol_info[ptitle_index->StatisticsMin].AsciiValue, "%d", sBist_DATA.lMin );
        		sprintf(pcol_info[ptitle_index->StatisticsAvg].AsciiValue, "%d", sBist_DATA.lAvg );
        	}
        }


    sprintf(pcol_info[ptitle_index->Layer].AsciiValue,    "%d", sBist_DATA.lLayer);
    sprintf(pcol_info[ptitle_index->Testfail].AsciiValue, "%d", sBist_DATA.lErrorCnt);
    sprintf(pcol_info[ptitle_index->Commfail].AsciiValue, "%d", sBist_DATA.lCommuErrCnt);

    if(pcol_info[ptitle_index->Result].IntegerValue == -1)
    {
        sprintf(pcol_info[ptitle_index->Result].AsciiValue, "FAIL");
        phandle->FailCount += 1;
    }
    else
    {
        sprintf(pcol_info[ptitle_index->Result].AsciiValue, "PASS");
    }

    unsigned int addr = 0;
    for(i = 0; i < phandle->Col_Nums; i ++)
    {
        sprintf(&phandle->PutBuffer[addr], "%s,", pcol_info[i].AsciiValue);
        addr += strlen(pcol_info[i].AsciiValue) + 1;
    }
    sprintf(&phandle->PutBuffer[addr-1], "\n");

	}

	return(1);
}

void utilBIST_ReportNameGet(char *Report_name)
{
	char cModelName[20] = {'\0'};
    UINT8 cSN[SN_LENGTH] = {'\0'};
    UINT16 uwYear = 0;
    UINT8  ucMonth = 0, ucDay = 0, ucHour = 0, ucMinute = 0, ucSecond = 0;
    time_t sTime;
    struct tm *sTemp;

    char cMAC[20] = {'\0'}; //A70LK_Steven_0014
    UINT8 acMAC[6] = {0};
    UINT8 ucIndex = 0 ;
    char str[3];
    char *endptr;

    time(&sTime);
    sTemp = localtime(&sTime);
    uwYear = (1900 + sTemp->tm_year);  //start at 1900 year
    ucMonth = (1 + sTemp->tm_mon);     //start at 0-11 month
    ucDay = sTemp->tm_mday;            //start at 01-31 day
    ucHour = sTemp->tm_hour;
    ucMinute = sTemp->tm_min;
    ucSecond = sTemp->tm_sec;

    palDataMgr_Data_Access(edcMODEL_NAME, edaREAD, cModelName);
    palDataMgr_Data_Access(edcSERIAL_NUMBER, edaREAD, cSN);
    palDataMgr_Data_Access(edcLAN_MAC_ADDRESS, edaREAD, cMAC); //A70LK_Steven_0014

    for(ucIndex = 0; ucIndex < 6; ucIndex++)
    {
        str[0] = cMAC[ucIndex * 3 + 0];
        str[1] = cMAC[ucIndex * 3 + 1];
        str[2] = '\0';
        acMAC[ucIndex] = (int) strtol((const char *)str, &endptr, 16) ;
    }

    //report file name "BIST_Report_ModelName_SN_MAC_YMDHMS"
    sprintf(Report_name,  "BIST_Report_%s_%s_%02X%02X%02X%02X%02X%02X_%4d_%d_%d_%d_%d_%d.csv",cModelName,cSN, acMAC[0], acMAC[1], acMAC[2], acMAC[3], acMAC[4], acMAC[5], uwYear,ucMonth,ucDay,ucHour,ucMinute,ucSecond);

}

void utilBIST_FolderNameGet(char *Report_name)
{
#if defined(PLATFORM_H60_4K)
	sprintf(Report_name,  "H60_4K_4K1600_4K2100_BIST");
#elif defined(PLATFORM_H60_2K)
	sprintf(Report_name,  "H60_2K_DWU1800_DWU2400_BIST");
#elif defined(PLATFORM_H30_4K)
	sprintf(Report_name,  "H30_4K_ZK1320_ZK1120_BIST");
#else
	sprintf(Report_name,  "BIST");
#endif
}

int utilBIST_Handle(char led_en) // 0: no file 1: USB 2: mnt //HICC2_Steven_0062
{
	int ret = 1;
    UINT8 ucReadfileFail = 0; //HICC2_Steven_0064
    unsigned char FilePath = 0;
	type_BIST_Handle Handle;
	type_Column_Info Col_Info[20];
	type_Title_Index Title;
	char tReportFileName[128] = {'\0'};
	char tFolderFileName[128] = {'\0'};
	char tReportFilePath[255] = {'\0'};        //H60_4K_4K1600_4K2100_BIST or H60_2K_DWU1800_DWU2400_BIST or BIST
	char tBIST_mnt_Path[128] =  {'\0'};       //  "/mnt/syslog/H60_4K_4K1600_4K2100_BIST"
	char tBIST_USB_Path[128] =  {'\0'};       //  "/run/media/sda1/H60_4K_4K1600_4K2100_BIST"
	char tBIST_USB_Fail_Path[128] =  {'\0'};
	char tMntImportFile[128]  = {"\0"};
	char tUSBImportFile[128]  = {"\0"};
	UINT8 ucValue;

	utilBIST_ReportNameGet(tReportFileName);
	utilBIST_FolderNameGet(tFolderFileName);

	Handle.Row = 0;
	Handle.Col_Nums = 0;
	Handle.GetCommand = 0;
	Handle.FailCount = 0;
	Handle.Length = ROW_LENGTH;
	Handle.GetSeek = 0;
	Handle.PutSeek = 0;
	Handle.GetBuffer = (unsigned char *)malloc(Handle.Length);
	Handle.PutBuffer = (unsigned char *)malloc(Handle.Length);
	memset(&Title, 0xFF, sizeof(Title));

	LOG_MSG(db_ALWAYS, "utilBIST_Handle !!! \r\n");

#if defined(PLATFORM_H30_4K)	
    palDataMgr_Access_USB_Power(edaREAD,&ucValue);
	if(ucValue == FALSE)
	{
		UINT8 ucTmpVal = TRUE;
    	palDataMgr_Access_USB_Power(edaWRITE_RAM_ONLY_WITH_ACTION,&ucTmpVal);
		vTaskDelay(5000);
	}
#endif

	sprintf(tBIST_mnt_Path,  "%s", BIST_LOG_PATH);                          //   "/mnt/syslog/BIST"
	sprintf(tMntImportFile,  "%s/%s", tBIST_mnt_Path, BIST_IMPORT_FILE);    //   "/mnt/syslog/BIST/BIST Import.csv"
	sprintf(tBIST_USB_Path,  "%s/%s", BIST_USB_LOG_PATH, tFolderFileName);  //   "/run/media/sda1/H60_4K_4K1600_4K2100_BIST"
	sprintf(tUSBImportFile,  "%s/%s", tBIST_USB_Path, BIST_IMPORT_FILE);    //   "/run/media/sda1/H60_4K_4K1600_4K2100_BIST/BIST Import.csv"
	sprintf(tBIST_USB_Fail_Path,  "%s/%s", tBIST_USB_Path, BIST_USB_FAIL);  //   "/run/media/sda1/H60_4K_4K1600_4K2100_BIST/BIST_FAIL"

	 //LOG_MSG(db_ALWAYS, "------------------\r\n");
	 //LOG_MSG(db_ALWAYS, "a.(%s)\r\n", tBIST_mnt_Path);
	 //LOG_MSG(db_ALWAYS, "b.(%s)\r\n", tBIST_USB_Path);
	 //LOG_MSG(db_ALWAYS, "c.(%s)\r\n", tBIST_USB_Fail_Path);

    if(access(tBIST_mnt_Path, 0)==-1)//access函式是檢視檔案是不是存在
	{
		if(mkdir(tBIST_mnt_Path, 0777))//如果不存在就用mkdir函式來建立
		{
	        printf("(%s, %d) Creat file bag failed!!!", __FUNCTION__, __LINE__);
		}
	}

    if(led_en == 1)
    {
    	UINT8  ucData  = BIST_STATUS_BUSY;   //HICC2_Steven_0012
    	palDataMgr_Data_Access(edcBIST_CHECK, edaWRITE_THROUGH_WITH_ACTION, &ucData);
    	palLedProc_LED_Behavior_Set(eLED_STATUS_LED_ALWAYS_OFF);
    }

    //LOG_MSG(db_ALWAYS, "d.(%s)\r\n", tUSBImportFile);
    //LOG_MSG(db_ALWAYS, "e.(%s)\r\n", tMntImportFile);

    if(access(tUSBImportFile, R_OK) == 0)
    {
        LOG_MSG(db_BIST, "%s(%s)\r\n", __func__, tUSBImportFile);
        FilePath = 1; //USB //HICC2_Steven_0062
    }
    else if(access(tMntImportFile, R_OK) == 0)
    {
        LOG_MSG(db_BIST, "%s(%s)\r\n", __func__, tMntImportFile);
        FilePath = 2; //HICC2_Steven_0062
    }
    else
    {
        LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() tImportFile[]\r\n", __FILE__, __LINE__, __func__);
        ret = -1;
        Handle.FailCount = 1; //HICC2_Steven_0062
        FilePath = 0;
    }

    sprintf(tReportFilePath,"%s/%s",tBIST_mnt_Path, tReportFileName); //iScaler

    //LOG_MSG(db_ALWAYS, "f.(%s)\r\n", tReportFilePath);

	while(ret)// (Handle.FailCount == 0)
	{
		memset(Handle.GetBuffer,'\0', Handle.Length);
		memset(Handle.PutBuffer,'\0', Handle.Length);
		//ret = utilBIST_GetRow(&Handle, tImportFile[FilePath]);
		if(FilePath == 2) //HICC2_Steven_0062
		{
			ret = utilBIST_GetRow(&Handle, tMntImportFile);
		}
		else if(FilePath == 1) //HICC2_Steven_0062
		{
			ret = utilBIST_GetRow(&Handle, tUSBImportFile);
		}


		if(ret == 0)
		{// feof
		    break;
        }
	    else if(ret < 0)
		{
            ucReadfileFail = 1;  //HICC2_Steven_0064
			LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() utilBIST_GetRow() Row=%d\n", __FILE__, __LINE__, __func__, Handle.Row);
			break;
		}

		strcpy(Handle.PutBuffer, Handle.GetBuffer);
		ret = utilBIST_ParseRow(&Handle, Col_Info);
		if(ret <= 0)
		{
            ucReadfileFail = 1;  //HICC2_Steven_0064
			LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() utilBIST_ParseRow() Row=%d\n", __FILE__, __LINE__, __func__, Handle.Row);
			break;
		}
		ret = utilBIST_SetCommand(&Handle, Col_Info, &Title);

		if(ret <= 0)
		{
			ucReadfileFail = 1;  //HICC2_Steven_0064
			LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() utilBIST_SetCommand() Row=%d\n", __FILE__, __LINE__, __func__, Handle.Row);
			break;
		}

		if(Handle.GetCommand == 1)
		{
            Handle.GetCommand = 0;
    		ret = utilBIST_GetCommand(&Handle, Col_Info, &Title);

    		if(ret <= 0)
    		{
                ucReadfileFail = 1;  //HICC2_Steven_0064
    			LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() utilBIST_GetCommand() Row=%d\n", __FILE__, __LINE__, __func__, Handle.Row);
    			break;
    		}
		}
		MS_SLEEP(100);

		ret = utilBIST_PutRow(&Handle, tReportFilePath);
		if(ret <= 0)
		{
            ucReadfileFail = 1;  //HICC2_Steven_0064
			LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() utilBIST_PutRow() Row=%d\n", __FILE__, __LINE__, __func__, Handle.Row);
			break;
		}
        MS_SLEEP(100);
		Handle.Row += 1;
	}

    if(ucReadfileFail)  //HICC2_Steven_0064
    {
        Handle.FailCount = 1;
    }

	utilBIST_Status_Set(eBIST_STATUS_END); //HICC2_Steven_0007

	free(Handle.GetBuffer);
	free(Handle.PutBuffer);

    if(led_en == 1) //A70LK_Steven_0008
    {
		if(FilePath==1) //USB //HICC2_Steven_0062
		{
			if(Handle.FailCount == 0)
			{
			    if(access(tBIST_USB_Path, 0)==-1)//access函式是檢視檔案是不是存在
				{
					if(mkdir(tBIST_USB_Path, 0777))//如果不存在就用mkdir函式來建立
					{
				        printf("(%s, %d) Creat file bag failed!!!", __FUNCTION__, __LINE__);
					}
				}
				SYSTEM_CALL("cp -R %s %s", tReportFilePath, tBIST_USB_Path);
			}
			else
			{
			    if(access(tBIST_USB_Fail_Path, 0)==-1)//access函式是檢視檔案是不是存在
				{
					if(mkdir(tBIST_USB_Fail_Path, 0777))//如果不存在就用mkdir函式來建立
					{
				        printf("(%s, %d) Creat file bag failed!!!", __FUNCTION__, __LINE__);
					}
				}
                SYSTEM_CALL("cp -R %s %s", tReportFilePath, tBIST_USB_Fail_Path);
			}
			MS_SLEEP(30000); //delay for data write to usb  //HICC2_Steven_0064
		}

		//////appDataMgr_OPDSnapshot(eOPD_SNAPSHOT_LOG); //A70LK_Steven_0015
        if(Handle.FailCount == 0)
        {
        	 UINT8  ucData  = BIST_STATUS_PASS;   //HICC2_Steven_0012
        	 LOG_MSG(db_ALWAYS, "\r\n BIST SUCCESS !!!\r\n");
        	 palDataMgr_Data_Access(edcBIST_CHECK,        edaWRITE_THROUGH_WITH_ACTION, &ucData);
        	 palLedProc_LED_Behavior_Set(eLED_STATUS_WARMUP); //HICC2_Steven_0053
        	 MS_SLEEP(10);
        	 palLedProc_LED_Behavior_Set(eLED_STATUS_NORMAL);
        }
        else
        {
        	UINT8  ucData  = BIST_STATUS_INIT;   //HICC2_Steven_0012

            if(FilePath==0) //no file //HICC2_Steven_0062
            {
                LOG_MSG(db_ALWAYS, "\r\n BIST NO FILE FOUND !!!\r\n");

            }
            else if( ucReadfileFail == 1 )  //HICC2_Steven_0064
            {
                LOG_MSG(db_ALWAYS, "\r\n BIST READ/WRITE FILE FAIL !!!\r\n");
            }
            else
            {
                LOG_MSG(db_ALWAYS, "\r\n BIST FAIL !!!\r\n");
            }

        	palDataMgr_Data_Access(edcBIST_CHECK,        edaWRITE_THROUGH_WITH_ACTION, &ucData);
        	palLedProc_LED_Behavior_Set(eLED_STATUS_FANLOCK_COOLING); //HICC2_Steven_0005
        }
    }
#if defined(PLATFORM_H30_4K)
	SYSTEM_CALL("sync");
	//palDataMgr_Access_USB_Power(edaWRITE_RAM_ONLY_WITH_ACTION,&ucValue);
#endif
	return(ret);// (Handle.FailCount);
}

void utilBIST_Status_Set(eMSG_BIST_STATUS eStatus)
{
    UINT8 i = 0;
    UINT8 cStatusCmd = 0;

    m_eBIST_STATUS = eStatus;

    if(eStatus == eBIST_STATUS_INIT)
    {
    	memset(&sCmd, 0, sizeof(sBIST_DATA));
    	memset(&sBist_DATA, 0, sizeof(sBIST_DATA));

    	cStatusCmd = eBIST_MSG_SET_RESET_COUNT;
    }
    else if(eStatus == eBIST_STATUS_END)
    {
    	cStatusCmd = eBIST_MSG_SET_FINISH_COUNT;
    }

    if(rcSUCCESS != halMCU_BIST_Status_Set(cStatusCmd))
    {
        LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() halMCU_BIST_Status_Set()\n", __FILE__, __LINE__, __func__);
    }

    if(rcSUCCESS != halFrontEndCtrl_BIST_Status_Set(cStatusCmd))
    {
        LOG_MSG(db_BIST, "!!! ERROR !!! %s<%d>: %s() halFrontEndCtrl_BIST_Status_Set()\n", __FILE__, __LINE__, __func__);
    }
}

eMSG_BIST_STATUS utilBIST_Status_Get(void )  //HICC2_Steven_0008
{
	//LOG_MSG(db_ALWAYS, "utilBIST_Status_Get %d\n", m_eBIST_STATUS);
	return m_eBIST_STATUS;
}

#if 0
eRESULT utilMotor_BIST_Get(void) //A70LK_Steven_0003
{
	UINT8 aucVersion[8] = {0};
	UINT8 aucVerString[32] = {0};
	eRESULT eResult = rcINVALID;

	memset(aucVersion, 0, sizeof(aucVersion));
	memset(aucVerString, 0, sizeof(aucVerString));
	eResult = halMotor_Version_Get(aucVersion);

	sprintf((char*)aucVerString, "M%02d.%02d\0", aucVersion[1], aucVersion[0]);

	if(rcSUCCESS == eResult)
	{
		if(strcmp(aucVerString, "M00.00") == 0 ||
	       strcmp(aucVerString, "MFF.FF") == 0 )
	    {
				m_wError_cnt+=1; //A70LK_Steven_0006
				LOG_MSG(db_HAL_RESERVED19, "FF Motor version error %s\n", aucVerString);
	            eResult = rcERROR;
	    }
		else
		{
			LOG_MSG(db_HAL_RESERVED19, "Motor version %s\n", aucVerString);
		}
	}
	else
	{
		m_wCommeError_cnt+=1; //A70LK_Steven_0006
		LOG_MSG(db_HAL_RESERVED19, "Motor version read error\n");
	}

	return(eResult);
}


eRESULT utilDDP_BIST_DDPUSB_Test(void)  //A70LK_Steven_0011 start
{
	u32_t UsbHub = 0, Ddp = 0, Camera = 0, UsbNum = 0;
	eRESULT eResult = rcINVALID;

	if( 0!= dvDDP442x_GetUsbStatus(&UsbHub, &Ddp, &Camera, &UsbNum) )
	{
		m_wCommeError_cnt+=1;
	}
	else
	{
		if(Ddp != TRUE)
		{
			m_wError_cnt+=1;
			 eResult = rcERROR;
		}
		else
		{
			eResult = rcSUCCESS;
		}
	}
	LOG_MSG(db_HAL_RESERVED19, "[BIST] utilDDP_BIST_DDPUSB_Test %d, %d\n", Ddp, eResult);
	return(eResult);
}
eRESULT utilDDP_BIST_CameraUSB_Test(void)
{
	u32_t UsbHub = 0, Ddp = 0, Camera = 0, UsbNum = 0;
	eRESULT eResult = rcINVALID;

	if( 0!= dvDDP442x_GetUsbStatus(&UsbHub, &Ddp, &Camera, &UsbNum) )
	{
		m_wCommeError_cnt+=1;
	}
	else
	{
		if(Camera != TRUE)
		{
			m_wError_cnt+=1;
			 eResult = rcERROR;
		}
		else
		{
			eResult = rcSUCCESS;
		}
	}
	LOG_MSG(db_HAL_RESERVED19, "[BIST] utilDDP_BIST_CameraUSB_Test %d, %d\n", Camera, eResult);
	return(eResult);
} //A70LK_Steven_0011 end

eRESULT utilDDP_BIST_Get(void)
{
	UINT16 wVerDDPApp = 0;
	UINT8 aucVerString[32] = {0};
	eRESULT eResult = rcINVALID;

	memset(aucVerString, 0, sizeof(aucVerString));
	eResult = halFormatter_SYS_AppVersionGet(&wVerDDPApp);

	sprintf((char*)aucVerString, "F%02d.%02d" ,(UINT8)(wVerDDPApp>>8), (UINT8)(wVerDDPApp));   //DDP442x Fxx.xx

	if(rcSUCCESS == eResult)
	{
		if(strcmp(aucVerString, "F00.00") == 0 ||
	       strcmp(aucVerString, "FFF.FF") == 0 )
	    {
			m_wError_cnt+=1;  //A70LK_Steven_0006
			LOG_MSG(db_HAL_RESERVED19, "FF DDP version error %s\n", aucVerString);
	        eResult = rcERROR;
	    }
		else
		{
			LOG_MSG(db_HAL_RESERVED19, "DDP version %s\n", aucVerString);
		}
	}
	else
	{	m_wCommeError_cnt+=1; //A70LK_Steven_0006
		LOG_MSG(db_HAL_RESERVED19, "DDP version read error\n");
	}

	return(eResult);
} //A70LK_Steven_0003

eRESULT utilProAV_BIST_Test(void)  //A70LK_Steven_0012
{
	UINT32 ulVersiong = 0;
	UINT32 ulMainVer = 0, ulSubVer = 0, ulMinorVer = 0;
	eRESULT eResult = rcINVALID;

	if (halScaler_Version_Get(&ulVersiong)!= rcSUCCESS)
	{
		m_wCommeError_cnt+=1;
		eResult = rcERROR;
	}
	else
	{
		ulMainVer   = (ulVersiong >> 8) & 0xFF;
		ulSubVer    =  ulVersiong & 0xFF;
		ulMinorVer  = (ulVersiong >> 16)&0xFF;

		if( (ulMainVer == 0 && ulSubVer == 0 && ulMinorVer == 0) ||
				(ulMainVer == 0xff && ulSubVer == 0xff && ulMinorVer == 0xff))
		{
			m_wError_cnt+=1;
			eResult = rcERROR;
		}
		else
		{
			eResult = rcSUCCESS;
		}
	}

	LOG_MSG(db_HAL_RESERVED19, "[BIST] ProAV_BIST_Test %d, %d, %d, %d\r\n", ulMainVer, ulSubVer, ulMinorVer, eResult);
	return eResult;
}

#endif
//A70LK_Steven_0009

#if 1
int util_BIST_Check_Result(BOOL IsTimes, INT32 lUpper, INT32 lLow, INT32 lTotalError )
{
	int iResult = 1;

    LOG_MSG(db_BIST, "BIST_Check (%3d) Total=%2d, Err=%2d %2d, Max=%5d, Min=%5d, lValid=%2d\r\n", sCmd.lBistId,
				sBist_DATA.lTotalCnt,
				sBist_DATA.lErrorCnt,
				sBist_DATA.lCommuErrCnt,
				sBist_DATA.lMax,
				sBist_DATA.lMin,
				sBist_DATA.lValid);

    if(sBist_DATA.lValid != 1)
    {
    	LOG_MSG(db_BIST, "!!! ERROR !!! (%d) lValid\r\n", sBist_DATA.lValid);
    	iResult = -1;

    	return iResult;
    }

	if(IsTimes == TRUE)
	{
		if( (sCmd.lCount - lTotalError) > lUpper)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) lUpper\r\n", sCmd.lBistId);
			iResult = -1;
		}
		else if( (sCmd.lCount - lTotalError) < lLow)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) lLow\r\n",  sCmd.lBistId);
			iResult = -1;
		}
		//else if(ucLayer >= 1)
		//{
		//	LOG_MSG(db_HAL_RESERVED19, "!!! ERROR !!! (%d) Layer = %d\r\n", iRegEvent, ucLayer);
		//	iResult = -1;
		//}
	}
	else
	{
		if(sBist_DATA.lMax > lUpper ||  sBist_DATA.lMax < lLow)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) lUpper\r\n",  sCmd.lBistId);
			iResult = -1;
		}
		else if(sBist_DATA.lMin < lLow || sBist_DATA.lMin > lUpper)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) CriterionLow\r\n",  sCmd.lBistId);
			iResult = -1;
		}
		else if(lTotalError >= 1)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) Error_cnt %d\r\n", sCmd.lBistId, lTotalError);
			iResult = -1;
		}
		//else if(ucLayer >= 1)
		//{
		//	LOG_MSG(db_HAL_RESERVED19, "!!! ERROR !!! (%d) Layer = %d\r\n", iRegEvent, ucLayer);
		//	iResult = -1;
		//}
	}

	return iResult;
}
#else
int util_BIST_Check_Result(BOOL IsTimes, int iRegEvent ,int iTestCount, int iCriterionUpper, int iCriterionLow, UINT32 iMax, UINT32 iMin, UINT16 wError, UINT8 ucLayer)
{
	int iResult = 1;
	//UINT16 wError = utilBIST_Error_Count_Get() + utilBIST_CommError_Count_Get();

	LOG_MSG(db_BIST, "BIST_Check_Result (%d)(%d,%d)(%d,%d)(%d,%d)  \r\n", iRegEvent, iMax, iMin,iCriterionUpper, iCriterionLow, wError, ucLayer );

	if(IsTimes == TRUE)
	{
		if( (iTestCount - wError) > iCriterionUpper)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) CriterionUpper\r\n", iRegEvent);
			iResult = -1;
		}
		else if( (iTestCount - wError) < iCriterionLow)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) CriterionLow\r\n", iRegEvent);
			iResult = -1;
		}
		//else if(ucLayer >= 1)
		//{
		//	LOG_MSG(db_HAL_RESERVED19, "!!! ERROR !!! (%d) Layer = %d\r\n", iRegEvent, ucLayer);
		//	iResult = -1;
		//}
	}
	else
	{
		if(iMax > iCriterionUpper ||  iMax < iCriterionLow)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) CriterionUpper\r\n", iRegEvent);
			iResult = -1;
		}
		else if(iMin < iCriterionLow || iMin > iCriterionUpper)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) CriterionLow\r\n", iRegEvent);
			iResult = -1;
		}
		else if(wError >= 1)
		{
			LOG_MSG(db_BIST, "!!! ERROR !!! (%d) Error_cnt %d\r\n", iRegEvent, wError);
			iResult = -1;
		}
		//else if(ucLayer >= 1)
		//{
		//	LOG_MSG(db_HAL_RESERVED19, "!!! ERROR !!! (%d) Layer = %d\r\n", iRegEvent, ucLayer);
		//	iResult = -1;
		//}
	}

	return iResult;
}
#endif

void utilBIST_ReportRename(void) //A70LK_Steven_0013
{
    if(access(BIST_LOG_PATH, 0)==-1)//access函式是檢視檔案是不是存在
	{
    	//LOG_MSG(db_ALWAYS, "utilBIST_ReportRename  fail\r\n");
	}
    else
    {
    	char tRenameCmd[128] = {'\0'};
    	UINT8 cSN[SN_LENGTH] = {'\0'};

    	palDataMgr_Data_Access(edcSERIAL_NUMBER, edaREAD, cSN);
    	sprintf(tRenameCmd,  "cd /;cd %s;rename %s %s *.csv",BIST_LOG_PATH, Default_SN, cSN);
    	SYSTEM_CALL("%s", tRenameCmd);
    }
}

sBIST_CMD utilBIST_CmdInfo_Get(void)
{
	return sCmd;
}

#if 0
UINT8 utilBIST_GetCommand_Get(void)
{
	return phandle->GetCommand;
}

void utilBIST_GetCommand_Set(UINT8 ucflag)
{
	phandle->GetCommand = ucflag;
}
#endif


#endif /* BIST_ENABLE */
#endif /* _UTIL_BIST_C_ */
