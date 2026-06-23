#ifndef UI_APP_COMPASSPAGE_H
#define UI_APP_COMPASSPAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern Page_t Page_App_Compass;
extern void ui_App_CompassPage_screen_init(void);
extern void ui_App_CompassPage_deinit(void);
extern lv_obj_t * ui_App_CompassPage;

#ifdef __cplusplus
}
#endif

#endif
