#ifndef UI_APP_CALCPAGE_H
#define UI_APP_CALCPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_App_Calc;
extern void ui_App_CalcPage_screen_init(void);
extern void ui_App_CalcPage_deinit(void);
extern lv_obj_t * ui_App_CalcPage;

#ifdef __cplusplus
}
#endif

#endif
