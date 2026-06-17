#ifndef __USER_RUNMODETASKS_H__
#define __USER_RUNMODETASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

void PowerMgrTask(void *argument);
void IdleTimerCallback(void *argument);

#ifdef __cplusplus
}
#endif

#endif
