#ifndef _CLICFGACCESS_
#define _CLICFGACCESS_

#include "type_def.h"

#ifndef __UBUNTU_SIMULATOR__
#define CMD_FILE_PATH                    "/usr/configs/ProjectSettings/Rs232Cmd.cfg"
#define CLI_FORMAT_FILE_PATH             "/usr/configs/ProjectSettings/CliFormat.cfg"
#define OSD_DEFAULTVALUE_FILE_PATH       "/usr/configs/ProjectSettings/OSDDefaultValue.cfg"
#else
#define CMD_FILE_PATH                    "../hpbulib/ProjectSettings/Rs232Cmd.cfg"
#define CLI_FORMAT_FILE_PATH             "../hpbulib/ProjectSettings/CliFormat.cfg"
#define OSD_DEFAULTVALUE_FILE_PATH       "../hpbulib/ProjectSettings/OSDDefaultValue.cfg"
#endif /* __UBUNTU_SIMULATOR__ */


//result status
#define FILE_PASS (1)
#define FILE_FAIL (-1)


typedef enum
{
    eCLI_STYLE_CHRISTIE,
    eCLI_STYLE_GENERAL,
    eCLI_STYLE_OPTOMA,

    eCLI_STYLE_NUMBER,
}eCLI_STYLE;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef struct
{
    UINT8 cDatacode_Index;
    char  cDatacode_String[128];
    char  cMainCmd[128];
    char  cSubCmd[128];
    UINT8 cEnableFlag;
}sFwVersion;

typedef struct
{
    UINT8 ucCLI_Format[eCLI_STYLE_NUMBER];
    sFwVersion sVersion_Info[64];
    UINT8 ucCMD_Support[1024];
}sCLI_SETTING;

//=======================

typedef struct
{
    UINT32 cIndex;
    char cString[64];
}sVerInfo;

typedef struct
{
    char cMainCmd[16];
    char cSubCmd[16];
    char cCmd_RW[16];
    //UINT8 cCMD_Support;
}sCmdInfo;

typedef struct
{
    UINT16 cTotalNum_Ver;
    UINT16 cTotalNum_Cmd;
    sVerInfo sVer_Info[64];
    sCmdInfo sCmd_Info[1024];

}sCMD_SETTING;

typedef struct
{
    UINT32 cIndex;
    char cString[64];
}sOSDDEFInfo;

typedef struct
{
    UINT16 uiTotalCount;
    sOSDDEFInfo *psOSDDDef;
}sOSDINIT_SETTING;


INT8 InitCliFormatCfg(void);
INT8 InitRs232CmdCfg(void);

int CliCfg_Format_Get(UINT8 cStyle);
int CmdCfg_VersionInfo_Get(UINT16 cNumber, UINT32 *pcDatacode, char *pcStr);
//int CmdCfg_CommandInfo_Get(UINT16 cCmdCount);
int CmdCfg_CommandSupport_Get(char* pcMainCmd, char* pcSubCmd, BOOL IsRead);
INT8 InitOSDDafaultValueCmdCfg(void);
UINT16 InitOSDDafaultValueTotalCountGet(void);
INT8 InitOSDDafaultValueGet(UINT16 uiCount, char *cString, INT32 *pcValue);

#endif // _CLICFGACCESS_
