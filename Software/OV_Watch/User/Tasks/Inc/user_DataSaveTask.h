#ifndef __USER_DATASAVETASK_H__
#define __USER_DATASAVETASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

void DataSaveTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif
