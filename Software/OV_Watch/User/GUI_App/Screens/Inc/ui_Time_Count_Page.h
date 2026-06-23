#ifndef UI_TIME_COUNT_PAGE_H
#define UI_TIME_COUNT_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Time_Count;
extern void ui_Time_Count_Page_screen_init(void);
extern void ui_Time_Count_Page_deinit(void);
extern void ui_event_Time_Count_Page(lv_event_t * e);
extern lv_obj_t * ui_Time_Count_Page;
extern lv_obj_t * ui_Panel9;
extern lv_obj_t * ui_Spinbox1;
extern lv_obj_t * ui_Label9;
extern lv_obj_t * ui_Label3;
extern void ui_event_Button1(lv_event_t * e);
extern lv_obj_t * ui_Button1;
extern lv_obj_t * ui_Time9;
extern lv_obj_t * ui_Label14;
extern void ui_event_Button5(lv_event_t * e);
extern lv_obj_t * ui_Button5;

#ifdef __cplusplus
}
#endif

#endif
