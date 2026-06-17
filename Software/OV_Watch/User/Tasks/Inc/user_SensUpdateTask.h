#ifndef __USER_SENSUPDATETASK_H__
#define __USER_SENSUPDATETASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

void SensorTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif
