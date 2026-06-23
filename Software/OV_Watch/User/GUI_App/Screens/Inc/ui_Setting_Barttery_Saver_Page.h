#ifndef UI_SETTING_BARTTERY_SAVER_PAGE_H
#define UI_SETTING_BARTTERY_SAVER_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Setting_Battery_Saver;
extern void ui_Setting_Barttery_Saver_Page_screen_init(void);
extern void ui_Setting_Barttery_Saver_Page_deinit(void);
extern lv_obj_t * ui_Setting_Barttery_Saver_Page;
extern void ui_event_Panel5(lv_event_t * e);
extern lv_obj_t * ui_Panel5;
extern lv_obj_t * ui_Settingcard9;
extern lv_obj_t * ui_Label20;
extern void ui_event_Switch2(lv_event_t * e);
extern lv_obj_t * ui_Switch2;
extern lv_obj_t * ui_Label21;
extern lv_obj_t * ui_Time5;

#ifdef __cplusplus
}
#endif

#endif
