/* Private includes -----------------------------------------------------------*/
#include "../Inc/ui_HomePage.h"
#include "../Inc/ui_SensorPage.h"
#include "HWDataAccess.h"
#include "lcd_init.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
lv_obj_t *ui_HomePage = NULL;
lv_obj_t *ui_TimeHourLabel = NULL;
lv_obj_t *ui_TimeMinuteLabel = NULL;
lv_obj_t *ui_DateLabel = NULL;
lv_obj_t *ui_StepsLabel = NULL;
lv_obj_t *ui_BatteryLabel = NULL;

static lv_timer_t *ui_HomePageTimer = NULL;

static uint8_t ui_TimeHourValue = 12;
static uint8_t ui_TimeMinuteValue = 0;
static uint8_t ui_DateYearValue = 0;
static uint8_t ui_DateMonthValue = 0;
static uint8_t ui_DateDayValue = 0;
static uint16_t ui_StepsValue = 0;
static uint8_t ui_BatteryValue = 0;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Home page timer callback (500ms period)
  * @param  timer: LVGL timer
  * @retval None
  */
static void HomePage_timer_cb(lv_timer_t *timer)
{
  char buf[20];
  
  // Time update (every 500ms)
  HW_DateTimeTypeDef DateTime;
  HWInterface.RealTimeClock.GetTimeDate(&DateTime);
  
  if(ui_TimeHourValue != DateTime.Hours)
  {
    ui_TimeHourValue = DateTime.Hours;
    sprintf(buf, "%2d", ui_TimeHourValue);
    lv_label_set_text(ui_TimeHourLabel, buf);
  }
  
  if(ui_TimeMinuteValue != DateTime.Minutes)
  {
    ui_TimeMinuteValue = DateTime.Minutes;
    sprintf(buf, "%02d", ui_TimeMinuteValue);
    lv_label_set_text(ui_TimeMinuteLabel, buf);
  }
  
  // Date update (when date changes)
  if(ui_DateYearValue != DateTime.Year || ui_DateMonthValue != DateTime.Month || ui_DateDayValue != DateTime.Date)
  {
    ui_DateYearValue = DateTime.Year;
    ui_DateMonthValue = DateTime.Month;
    ui_DateDayValue = DateTime.Date;
    sprintf(buf, "20%02d-%02d-%02d", ui_DateYearValue, ui_DateMonthValue, ui_DateDayValue);
    lv_label_set_text(ui_DateLabel, buf);
  }
  
  // Steps update (every 500ms)
  uint16_t steps = HWInterface.IMU.GetSteps();
  ui_StepsValue = steps;
  sprintf(buf, "%d", ui_StepsValue);
  lv_label_set_text(ui_StepsLabel, buf);
  
  // Battery update (every 10 seconds = 20 * 500ms)
  static uint8_t bat_cnt = 0;
  if(++bat_cnt >= 20)
  {
    bat_cnt = 0;
    uint8_t bat = HWInterface.Power.power_remain;
    if(ui_BatteryValue != bat)
    {
      ui_BatteryValue = bat;
      sprintf(buf, "%d%%", ui_BatteryValue);
      lv_label_set_text(ui_BatteryLabel, buf);
    }
  }
}

/**
  * @brief  Swipe left event callback (go to sensor page)
  * @param  e: event
  * @retval None
  */
static void swipe_left_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_GESTURE)
  {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_LEFT)
    {
      // 如果 sensor page 不存在才创建
      if(ui_SensorPage == NULL)
      {
        ui_SensorPage_init();
      }
      lv_scr_load_anim(ui_SensorPage, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
    }
  }
}

/**
  * @brief  Initialize home page
  * @param  None
  * @retval None
  */
void ui_HomePage_init(void)
{
  ui_HomePage = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_HomePage, lv_color_hex(0x000000), 0);
  
  // Add swipe left gesture
  lv_obj_add_event_cb(ui_HomePage, swipe_left_cb, LV_EVENT_GESTURE, NULL);
  
  // Time - Hour
  ui_TimeHourLabel = lv_label_create(ui_HomePage);
  lv_label_set_text(ui_TimeHourLabel, "12");
  lv_obj_set_style_text_color(ui_TimeHourLabel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(ui_TimeHourLabel, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_TimeHourLabel, LV_ALIGN_TOP_MID, -30, 60);
  
  // Time - Colon
  lv_obj_t *colon = lv_label_create(ui_HomePage);
  lv_label_set_text(colon, ":");
  lv_obj_set_style_text_color(colon, lv_color_hex(0x808080), 0);
  lv_obj_set_style_text_font(colon, LV_FONT_DEFAULT, 0);
  lv_obj_align(colon, LV_ALIGN_TOP_MID, 0, 60);
  
  // Time - Minute
  ui_TimeMinuteLabel = lv_label_create(ui_HomePage);
  lv_label_set_text(ui_TimeMinuteLabel, "00");
  lv_obj_set_style_text_color(ui_TimeMinuteLabel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(ui_TimeMinuteLabel, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_TimeMinuteLabel, LV_ALIGN_TOP_MID, 30, 60);
  
  // Date
  ui_DateLabel = lv_label_create(ui_HomePage);
  lv_label_set_text(ui_DateLabel, "2026-01-01");
  lv_obj_set_style_text_color(ui_DateLabel, lv_color_hex(0xAAAAAA), 0);
  lv_obj_set_style_text_font(ui_DateLabel, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_DateLabel, LV_ALIGN_TOP_MID, 0, 90);
  
  // Steps icon and label
  lv_obj_t *steps_icon = lv_label_create(ui_HomePage);
  lv_label_set_text(steps_icon, LV_SYMBOL_HOME);
  lv_obj_set_style_text_color(steps_icon, lv_color_hex(0x4CAF50), 0);
  lv_obj_align(steps_icon, LV_ALIGN_CENTER, -40, 40);
  
  ui_StepsLabel = lv_label_create(ui_HomePage);
  lv_label_set_text(ui_StepsLabel, "0");
  lv_obj_set_style_text_color(ui_StepsLabel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(ui_StepsLabel, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_StepsLabel, LV_ALIGN_CENTER, 10, 40);
  
  // Battery icon and label
  lv_obj_t *bat_icon = lv_label_create(ui_HomePage);
  lv_label_set_text(bat_icon, LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_color(bat_icon, lv_color_hex(0xFFC107), 0);
  lv_obj_align(bat_icon, LV_ALIGN_CENTER, -40, 80);
  
  ui_BatteryLabel = lv_label_create(ui_HomePage);
  lv_label_set_text(ui_BatteryLabel, "0%");
  lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(ui_BatteryLabel, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_BatteryLabel, LV_ALIGN_CENTER, 10, 80);
  
  // Start timer
  ui_HomePageTimer = lv_timer_create(HomePage_timer_cb, 500, NULL);
}

/**
  * @brief  Deinitialize home page
  * @param  None
  * @retval None
  */
void ui_HomePage_deinit(void)
{
  if(ui_HomePageTimer != NULL)
  {
    lv_timer_del(ui_HomePageTimer);
    ui_HomePageTimer = NULL;
  }
  
  if(ui_HomePage != NULL)
  {
    lv_obj_del(ui_HomePage);
    ui_HomePage = NULL;
  }
}
