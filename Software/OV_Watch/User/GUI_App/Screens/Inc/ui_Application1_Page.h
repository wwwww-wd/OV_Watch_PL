#ifndef UI_APPLICATION1_PAGE_H
#define UI_APPLICATION1_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Application1;
extern void ui_Application1_Page_screen_init(void);
extern void ui_Application1_Page_deinit(void);
extern lv_obj_t * ui_Application1_Page;

#ifdef __cplusplus
}
#endif

#endif
