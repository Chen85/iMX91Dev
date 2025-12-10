#include "utilMisc.h"
#include "utilDbgMsg.h"
#include "math.h"
#include "conf.h"
#include "GeneralFunc.h"

INT8 utilMisc_GetFileData(INT8 *cFileName, UINT8 *paucData, UINT32 ulSize)
{
    FILE *pFile = fopen((char *)cFileName, "rb");

    if(pFile == NULL || paucData == NULL || ulSize == 0)
    {
        //ASSERT_ALWAYS();
        return UTILMISC_NO_FILE ;
    }

    memset(paucData, 0, ulSize);
    if(fread(paucData, 1, ulSize, pFile) != ulSize)
    {
        fclose(pFile);
        return UTILMISC_NO_FILE;
    }

    fclose(pFile);

    return UTILMISC_EXEC_PASS;
}


INT8 utilMisc_GetReleaseVersion(UINT8 *aucString)
{
    if(utilMisc_GetStringFromConf((const char*)RELEASE_VERSION_FILE_PATH, (const char*)"ReleaseVersion", aucString) == UTILMISC_EXEC_PASS)
    {
        return UTILMISC_EXEC_PASS;
    }

    return UTILMISC_DATA_ERROR;
}


//FilePath : from file
//FieldName : field name to search
//aucDataString : result get
INT8 utilMisc_GetStringFromConf(const char *FilePath, const char *FieldName, UINT8 *aucDataString)
{
    UINT32 FileSize = 0 ;
    UINT16 FileTotalItem = 0 ;
    char *Item[CONF_MAX_ITEM][2];

    //check file
    FILE *pFile = fopen((char *)FilePath, "r");
    if(FilePath == NULL || pFile == NULL)
    {
        LOG_MSG(db_ASSERT, "(func:%s, line:%d) File not found : %s\r\n", __FUNCTION__, __LINE__, FilePath);
        return UTILMISC_NO_FILE ;
    }
    fclose(pFile);

    if(FieldName == NULL)
    {
        return UTILMISC_DATA_ERROR;
    }

    FileSize = Get_File_Size((char *)FilePath);

    if(FileSize == 0)
    {
        LOG_MSG(db_ASSERT, "(func:%s, line:%d) File Size = 0\r\n", __FUNCTION__, __LINE__);
        return UTILMISC_DATA_ERROR;
    }

    char *Buffer = (char*)malloc(FileSize+1);
    FileTotalItem = (UINT16)readConf(FilePath, Item, CONF_MAX_ITEM, Buffer, FileSize);

    snprintf(aucDataString, MAX_CONF_ITEM_VALUE_STR_LENGTH, "%s", findArgValue(FieldName, Item, FileTotalItem));
    free(Buffer);
    Buffer = NULL;

    return UTILMISC_EXEC_PASS;
}


