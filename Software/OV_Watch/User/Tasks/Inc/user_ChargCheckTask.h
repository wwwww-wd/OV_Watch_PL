#ifndef __USER_CHARGCHECKTASK_H__
#define __USER_CHARGCHECKTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

void ChargCheckTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif
