#ifndef UI_APPLICATION_PAGE_H
#define UI_APPLICATION_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_Application;
extern void ui_Application_Page_screen_init(void);
extern void ui_Application_Page_deinit(void);
extern lv_obj_t * ui_Application_Page;


#ifdef __cplusplus
}
#endif

#endif
