#ifndef __UI_CHARGPAGE_H__
#define __UI_CHARGPAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "PageManager.h"

extern lv_obj_t *ui_ChargPage;
extern Page_t Page_Charg;

void ui_ChargPage_init(void);
void ui_ChargPage_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
