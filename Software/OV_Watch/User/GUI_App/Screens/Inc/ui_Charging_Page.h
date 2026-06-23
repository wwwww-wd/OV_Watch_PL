#ifndef UI_CHARGING_PAGE_H
#define UI_CHARGING_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Charging;
extern void ui_Charging_Page_screen_init(void);
extern void ui_Charging_Page_deinit(void);
extern lv_obj_t * ui_Charging_Page;
extern void ui_event_Button4(lv_event_t * e);
extern lv_obj_t * ui_Button4;
extern lv_obj_t * ui_Label32;
extern void ui_event_Arc1(lv_event_t * e);
extern lv_obj_t * ui_Arc1;
extern lv_obj_t * ui_battery;
extern lv_obj_t * ui_Label33;

#ifdef __cplusplus
}
#endif

#endif
