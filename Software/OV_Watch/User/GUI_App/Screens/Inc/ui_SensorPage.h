#ifndef __UI_SENSORPAGE_H__
#define __UI_SENSORPAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

extern lv_obj_t *ui_SensorPage;

void ui_SensorPage_init(void);
void ui_SensorPage_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
