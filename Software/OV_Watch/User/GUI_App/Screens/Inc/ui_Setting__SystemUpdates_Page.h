#ifndef UI_SETTING__SYSTEMUPDATES_PAGE_H
#define UI_SETTING__SYSTEMUPDATES_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Setting_SystemUpdates;
extern void ui_Setting__SystemUpdates_Page_screen_init(void);
extern void ui_Setting__SystemUpdates_Page_deinit(void);
extern lv_obj_t * ui_Setting__SystemUpdates_Page;
extern void ui_event_Panel6(lv_event_t * e);
extern lv_obj_t * ui_Panel6;
extern void ui_event_Settingcard4(lv_event_t * e);
extern lv_obj_t * ui_Settingcard4;
extern lv_obj_t * ui_Label16;
extern lv_obj_t * ui_Image8;
extern lv_obj_t * ui_Settingcard14;
extern lv_obj_t * ui_Label35;
extern lv_obj_t * ui_Image17;
extern lv_obj_t * ui_Label19;
extern lv_obj_t * ui_Time6;
//LV_IMG_DECLARE(ui_img_1816534290);    // assets/œÚ”“1.png
#ifdef __cplusplus
}
#endif

#endif
