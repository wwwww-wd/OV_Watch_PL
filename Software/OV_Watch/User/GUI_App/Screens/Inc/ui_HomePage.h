#ifndef __UI_HOMEPAGE_H__
#define __UI_HOMEPAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern lv_obj_t *ui_HomePage;
extern Page_t Page_Home;
extern lv_obj_t *ui_TimeHourLabel;
extern lv_obj_t *ui_TimeMinuteLabel;
extern lv_obj_t *ui_DateLabel;
extern lv_obj_t *ui_StepsLabel;
extern lv_obj_t *ui_BatteryLabel;

void ui_HomePage_init(void);
void ui_HomePage_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
