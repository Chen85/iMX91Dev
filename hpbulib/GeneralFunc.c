#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include "type_def.h"

UINT32 Get_File_Size(char *fname)
{
    FILE *fp = fopen(fname, "rb");

    if(fp == NULL)
        return 0;

    fseek(fp, 0, SEEK_END);
    UINT32 FileSize = (UINT32)ftell(fp);
    fclose(fp);

    return FileSize;
}

UINT32 Get_File_LineNumber(char *fname)
{
    FILE *fp = fopen(fname, "rb");

    if(fp == NULL)
        return 0;

    char OneLineData[1024] = {'\0'};
    UINT32 Line = 0;
    while(fgets(OneLineData, sizeof(OneLineData), fp) != NULL)
    {
        if(OneLineData[strlen(OneLineData)-1] == '\n')
            Line++;
    }

    //printf("Total Line %d\n", Line);

    return Line;
}


int HexStr2Int(char *buf)
{
    int result = 0;
    int tmp;
    int len,i,start = 0;

    len = strlen(buf);
    //printf("len=%d \r\n",len);

    if(len >= 2 &&
       buf[0] == '0' &&
       (buf[1] == 'x' || buf[1] == 'X')
    )
    {
        buf += 2;  //skip "0x" or "0X"
        start = 2;
    }

    for(i=start; i<len; i++)
    {
        if(*buf>='A' && *buf<='F')
            tmp = *buf-'A'+10;
        else if(*buf>='a' && *buf<='f')
            tmp = *buf-'a'+10;
        else if(*buf>='0' && *buf<='9')
            tmp = *buf-'0';
        else
            return 0;  //wrong hex input


        result*=16;
        result+=tmp;
        buf++;
    }

    return result;
}



int Str2Int(char *buf)
{
    int len,result = 0;

    len = strlen(buf);

    if(len >= 2 &&
       buf[0] == '0' &&
       (buf[1] == 'x' || buf[1] == 'X')
    )
    {
        return HexStr2Int(buf);
    }
    else
    {
        return atoi(buf);
    }

}


