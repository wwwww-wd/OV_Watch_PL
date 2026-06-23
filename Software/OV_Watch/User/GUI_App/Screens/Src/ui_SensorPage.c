/* Private includes -----------------------------------------------------------*/
#include "../Inc/ui_SensorPage.h"
#include "../Inc/ui_Home_Page.h"
#include "HWDataAccess.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
lv_obj_t *ui_SensorPage = NULL;
Page_t Page_Sensor = {ui_SensorPage_init, ui_SensorPage_deinit, &ui_SensorPage};
static lv_timer_t *ui_SensorPageTimer = NULL;

static lv_obj_t *temp_label = NULL;
static lv_obj_t *humi_label = NULL;
static lv_obj_t *pressure_label = NULL;
static lv_obj_t *altitude_label = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Sensor page timer callback (1s period)
  * @param  timer: LVGL timer
  * @retval None
  */
static void SensorPage_timer_cb(lv_timer_t *timer)
{
  char buf[32];
  
  // Temperature and Humidity
  if(!HWInterface.AHT21.ConnectionError)
  {
    float humi, temp;
    HWInterface.AHT21.GetHumiTemp(&humi, &temp);
    if(temp > -40 && temp < 80 && humi > 0 && humi < 100)
    {
      HWInterface.AHT21.temperature = (int8_t)temp;
      HWInterface.AHT21.humidity = (int8_t)humi;
    }
    sprintf(buf, "Temp: %d C", HWInterface.AHT21.temperature);
    lv_label_set_text(temp_label, buf);
    sprintf(buf, "Humi: %d %%", HWInterface.AHT21.humidity);
    lv_label_set_text(humi_label, buf);
  }
  else
  {
    lv_label_set_text(temp_label, "Temp: N/A");
    lv_label_set_text(humi_label, "Humi: N/A");
  }
  
  // Pressure and Altitude
  if(!HWInterface.Barometer.ConnectionError)
  {
    float altitude = Altitude_Calculate();
    HWInterface.Barometer.altitude = (int16_t)altitude;
    sprintf(buf, "Alti: %d m", HWInterface.Barometer.altitude);
    lv_label_set_text(altitude_label, buf);
    
    // Pressure (from SPL06)
    float pressure = Pressure_Calculate();
    sprintf(buf, "Pres: %.1f hPa", pressure / 100.0f);
    lv_label_set_text(pressure_label, buf);
  }
  else
  {
    lv_label_set_text(altitude_label, "Alti: N/A");
    lv_label_set_text(pressure_label, "Pres: N/A");
  }
}

/**
  * @brief  Swipe Top event callback (back to home)
  * @param  e: event
  * @retval None
  */
static void swipe_Top_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_GESTURE)
  {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_BOTTOM)
    {
      Page_Back(LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 250, 0);
    }
  }
}

/**
  * @brief  Initialize sensor page
  * @param  None
  * @retval None
  */
void ui_SensorPage_init(void)
{
  ui_SensorPage = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_SensorPage, lv_color_hex(0x000000), 0);
  
  // Add gesture event
  lv_obj_add_event_cb(ui_SensorPage, swipe_Top_cb, LV_EVENT_GESTURE, NULL);
  
  // Title
  lv_obj_t *title = lv_label_create(ui_SensorPage);
  lv_label_set_text(title, "Sensor Data");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
  
  // Temperature
  temp_label = lv_label_create(ui_SensorPage);
  lv_label_set_text(temp_label, "Temp: -- C");
  lv_obj_set_style_text_color(temp_label, lv_color_hex(0xFF5722), 0);
  lv_obj_align(temp_label, LV_ALIGN_TOP_LEFT, 20, 60);
  
  // Humidity
  humi_label = lv_label_create(ui_SensorPage);
  lv_label_set_text(humi_label, "Humi: -- %");
  lv_obj_set_style_text_color(humi_label, lv_color_hex(0x2196F3), 0);
  lv_obj_align(humi_label, LV_ALIGN_TOP_LEFT, 20, 100);
  
  // Pressure
  pressure_label = lv_label_create(ui_SensorPage);
  lv_label_set_text(pressure_label, "Pres: -- hPa");
  lv_obj_set_style_text_color(pressure_label, lv_color_hex(0x4CAF50), 0);
  lv_obj_align(pressure_label, LV_ALIGN_TOP_LEFT, 20, 140);
  
  // Altitude
  altitude_label = lv_label_create(ui_SensorPage);
  lv_label_set_text(altitude_label, "Alti: -- m");
  lv_obj_set_style_text_color(altitude_label, lv_color_hex(0xFFC107), 0);
  lv_obj_align(altitude_label, LV_ALIGN_TOP_LEFT, 20, 180);
  
  // Hint
  lv_obj_t *hint = lv_label_create(ui_SensorPage);
  lv_label_set_text(hint, "Swipe right to return");
  lv_obj_set_style_text_color(hint, lv_color_hex(0x808080), 0);
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);
  
  // Start timer
  ui_SensorPageTimer = lv_timer_create(SensorPage_timer_cb, 1000, NULL);
}

/**
  * @brief  Deinitialize sensor page
  * @param  None
  * @retval None
  */
void ui_SensorPage_deinit(void)
{
  //NULL screen variables
  temp_label = NULL;
  humi_label = NULL;
  pressure_label = NULL;
  altitude_label = NULL;

  if(ui_SensorPageTimer != NULL)
  {
    lv_timer_del(ui_SensorPageTimer);
    ui_SensorPageTimer = NULL;
  }

}
