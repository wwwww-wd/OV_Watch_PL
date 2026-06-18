#ifndef __TASK_WDOG_H__
#define __TASK_WDOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WDOG_CH_LVGL    (1 << 0)
#define WDOG_CH_KEY     (1 << 1)
#define WDOG_CH_SENSOR  (1 << 2)
#define WDOG_CH_POWER   (1 << 3)
#define WDOG_CH_ALL     (WDOG_CH_LVGL | WDOG_CH_KEY | WDOG_CH_SENSOR | WDOG_CH_POWER)

void WDOGFeedTask(void *argument);
void WDOG_CheckIn(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
