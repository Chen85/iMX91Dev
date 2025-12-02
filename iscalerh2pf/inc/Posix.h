// ===============================================================================
// FILE NAME: Posix.h
// DESCRIPTION: 
// 
// 
// Modification History
// --------------------
// 2020/02/22, Lai Create
// --------------------
// ===============================================================================


#ifndef _POSIX_H_
#define _POSIX_H_

#include "type_def.h"
#include <pthread.h>

UINT64 Posix_Tick_Get(void);
void util_TimespecAddms(struct timespec *s_time ,INT32 lms);

#endif // _POSIX_H_

