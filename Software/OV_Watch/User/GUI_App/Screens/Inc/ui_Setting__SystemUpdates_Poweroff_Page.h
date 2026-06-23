#ifndef UI_SETTING__SYSTEMUPDATES_POWEROFF_PAGE_H
#define UI_SETTING__SYSTEMUPDATES_POWEROFF_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Setting_SystemUpdates_Poweroff;
extern void ui_Setting__SystemUpdates_Poweroff_Page_screen_init(void);
extern void ui_Setting__SystemUpdates_Poweroff_Page_deinit(void);
extern lv_obj_t * ui_Setting__SystemUpdates_Poweroff_Page;
extern void ui_event_Panel7(lv_event_t * e);
extern lv_obj_t * ui_Panel7;
extern lv_obj_t * ui_Label24;
extern void ui_event_Button2(lv_event_t * e);
extern lv_obj_t * ui_Button2;
extern void ui_event_Button3(lv_event_t * e);
extern lv_obj_t * ui_Button3;

#ifdef __cplusplus
}
#endif

#endif
