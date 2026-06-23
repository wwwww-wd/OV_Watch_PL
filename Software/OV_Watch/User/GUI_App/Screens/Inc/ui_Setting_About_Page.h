#ifndef UI_SETTING_ABOUT_PAGE_H
#define UI_SETTING_ABOUT_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Setting_About;
extern void ui_Setting_About_Page_screen_init(void);
extern void ui_Setting_About_Page_deinit(void);
extern lv_obj_t * ui_Setting_About_Page;
extern void ui_event_Panel8(lv_event_t * e);
extern lv_obj_t * ui_Panel8;
extern lv_obj_t * ui_Settingcard7;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_Label5;
extern lv_obj_t * ui_Settingcard11;
extern lv_obj_t * ui_Label6;
extern lv_obj_t * ui_Label2;
extern lv_obj_t * ui_Settingcard12;
extern lv_obj_t * ui_Label7;
extern lv_obj_t * ui_Label8;
extern lv_obj_t * ui_Settingcard13;
extern lv_obj_t * ui_Label4;
extern lv_obj_t * ui_Time7;

#ifdef __cplusplus
}
#endif

#endif
