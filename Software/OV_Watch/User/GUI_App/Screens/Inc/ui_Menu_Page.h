#ifndef UI_MENU_PAGE_H
#define UI_MENU_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Menu;
extern void ui_Menu_Page_screen_init(void);
extern void ui_Menu_Page_deinit(void);
extern void ui_event_Menu_Page(lv_event_t * e);
extern lv_obj_t * ui_Menu_Page;
extern lv_obj_t * ui_Bar1;
extern void ui_event_battery2(lv_event_t * e);
extern lv_obj_t * ui_battery2;
extern lv_obj_t * ui_Date3;
extern lv_obj_t * ui_Panel2;
extern void ui_event_APP_APP(lv_event_t * e);
extern lv_obj_t * ui_APP;
extern void ui_event_APP2_APP(lv_event_t * e);
extern lv_obj_t * ui_APP2;
extern void ui_event_APP1_APP(lv_event_t * e);
extern lv_obj_t * ui_APP1;
extern void ui_event_APP3_APP(lv_event_t * e);
extern lv_obj_t * ui_APP3;
extern void ui_event_APP4_APP(lv_event_t * e);
extern lv_obj_t * ui_APP4;
extern void ui_event_APP5_APP(lv_event_t * e);
extern lv_obj_t * ui_APP5;
extern lv_obj_t * ui_Date4;
extern lv_obj_t * ui_Time2;
extern lv_obj_t * ui_Image3;
extern bool g_bt_icon_visible;
extern bool g_Sleep;
extern bool g_flashlight_on;

#ifdef __cplusplus
}
#endif

#endif
