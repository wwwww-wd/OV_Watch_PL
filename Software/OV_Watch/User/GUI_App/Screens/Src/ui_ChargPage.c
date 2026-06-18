/* Private includes -----------------------------------------------------------*/
#include "../Inc/ui_ChargPage.h"
#include "HWDataAccess.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
lv_obj_t *ui_ChargPage = NULL;
Page_t Page_Charg = {ui_ChargPage_init, ui_ChargPage_deinit, &ui_ChargPage};

static lv_timer_t *ui_ChargPageTimer = NULL;
static lv_obj_t *ui_ChargLabel = NULL;
static lv_obj_t *ui_ChargBatLabel = NULL;
static lv_obj_t *ui_ChargIcon = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Charging page timer callback (2s period)
  * @param  timer: LVGL timer
  * @retval None
  */
static void ChargPage_timer_cb(lv_timer_t *timer)
{
  char buf[16];
  uint8_t bat = HWInterface.Power.power_remain;
  sprintf(buf, "%d%%", bat);
  lv_label_set_text(ui_ChargBatLabel, buf);
}

/**
  * @brief  Initialize charging page
  * @param  None
  * @retval None
  */
void ui_ChargPage_init(void)
{
  ui_ChargPage = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_ChargPage, lv_color_hex(0x000000), 0);

  // Battery icon
  ui_ChargIcon = lv_label_create(ui_ChargPage);
  lv_label_set_text(ui_ChargIcon, LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_color(ui_ChargIcon, lv_color_hex(0x4CAF50), 0);
  lv_obj_set_style_text_font(ui_ChargIcon, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_ChargIcon, LV_ALIGN_CENTER, 0, -40);

  // Charging text
  ui_ChargLabel = lv_label_create(ui_ChargPage);
  lv_label_set_text(ui_ChargLabel, "Charging...");
  lv_obj_set_style_text_color(ui_ChargLabel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(ui_ChargLabel, LV_ALIGN_CENTER, 0, 0);

  // Battery percentage
  ui_ChargBatLabel = lv_label_create(ui_ChargPage);
  lv_label_set_text(ui_ChargBatLabel, "0%");
  lv_obj_set_style_text_color(ui_ChargBatLabel, lv_color_hex(0xFFC107), 0);
  lv_obj_set_style_text_font(ui_ChargBatLabel, LV_FONT_DEFAULT, 0);
  lv_obj_align(ui_ChargBatLabel, LV_ALIGN_CENTER, 0, 30);

  // Timer for battery update
  ui_ChargPageTimer = lv_timer_create(ChargPage_timer_cb, 2000, NULL);
}

/**
  * @brief  Deinitialize charging page
  * @param  None
  * @retval None
  */
void ui_ChargPage_deinit(void)
{
  if(ui_ChargPageTimer != NULL)
  {
    lv_timer_del(ui_ChargPageTimer);
    ui_ChargPageTimer = NULL;
  }
}
