#ifndef UI_APP_HRPAGE_H
#define UI_APP_HRPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_App_HR;
extern void ui_App_HRPage_screen_init(void);
extern void ui_App_HRPage_deinit(void);
extern lv_obj_t * ui_App_HRPage;

#ifdef __cplusplus
}
#endif

#endif
