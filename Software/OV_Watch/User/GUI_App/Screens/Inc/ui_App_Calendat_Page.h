#ifndef UI_APP_CALENDAT_PAGE_H
#define UI_APP_CALENDAT_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_App_Calendat;
extern void ui_App_Calendat_Page_screen_init(void);
extern void ui_App_Calendat_Page_deinit(void);
extern lv_obj_t * ui_App_Calendat_Page;


#ifdef __cplusplus
}
#endif

#endif
