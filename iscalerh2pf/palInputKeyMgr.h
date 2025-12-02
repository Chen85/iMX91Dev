#ifndef PALINPUTKEYMGR_H
#define PALINPUTKEYMGR_H

#include "Common.h"

eRESULT palInputKeyMgr_Init(void);
void palInputKeyMgr_Register_Callback(eRESULT(*fpCallback)(void *));
void palInputKeyMgr_Proc(sKEY_DATA *psKeyData);
void palInputKeyMgr_CmdProc(sKEY_DATA *psCmdKeyData);
void palInputKeyMgr_IR_CustomId_Set(UINT16 wCustomId);
UINT16 palInputKeyMgr_IR_DefaultGet(void);
UINT16 palInputKeyMgr_IR_CustomIdGet(UINT8 ucID);


#endif // PALINPUTKEYMGR_H

