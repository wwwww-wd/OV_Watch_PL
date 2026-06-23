#ifndef UI_BRIGHTNESS_PAGE_H
#define UI_BRIGHTNESS_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Brightness;
extern void ui_Brightness_Page_screen_init(void);
extern void ui_Brightness_Page_deinit(void);
extern void ui_event_Brightness_Page(lv_event_t * e);
extern lv_obj_t * ui_Brightness_Page;
extern lv_obj_t * ui_Slider1;
extern lv_obj_t * ui_Image2;
extern int brightness;
#ifdef __cplusplus
}
#endif

#endif
