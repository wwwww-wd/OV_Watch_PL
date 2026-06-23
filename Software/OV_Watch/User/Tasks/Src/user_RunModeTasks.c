/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
#include "user_RunModeTasks.h"
#include "main.h"
#include "lcd.h"
#include "lcd_init.h"
#include "CST816.h"
#include "key.h"
#include "power.h"
#include "WDOG.h"
#include "rtc.h"
#include "HWDataAccess.h"
#include "PageManager.h"
#include "task_wdog.h"
#include "stm32f4xx_it.h"
#include "../Inc/ui_Brightness_Page.h"
#include "../Inc/ui_Menu_Page.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint16_t IdleTimerCount = 0;

/* Private function prototypes -----------------------------------------------*/
extern void SystemClock_Config(void);

/* Tasks ---------------------------------------------------------------------*/

/**
  * @brief  Power management task
  * @param  argument: Not used
  * @retval None
  */
void PowerMgrTask(void *argument)
{
  uint8_t Idlestr = 0;
  uint8_t IdleBreakstr = 0;
  uint8_t Stopstr = 0;
  uint8_t HomeUpdataStr = 0;

  while(1)
  {
    // Light off on idle
    if(osMessageQueueGet(Idle_MessageQueue, &Idlestr, NULL, 1) == osOK)
    {
      LCD_Set_Light(5);
    }

    // Resume light on activity
    if(osMessageQueueGet(IdleBreak_MessageQueue, &IdleBreakstr, NULL, 1) == osOK)
    {
      IdleTimerCount = 0;
      LCD_Set_Light(brightness);
    }

    // Stop mode entry
    if(osMessageQueueGet(Stop_MessageQueue, &Stopstr, NULL, 0) == osOK)
    {
      IdleTimerCount = 0;

      // 如果不在首页，先回到首页（销毁传感器页面的定时器）
      if(!Page_Is_Home())
      {
        uint8_t pageCmd = 2;
        osMessageQueuePut(PageCmd_MessageQueue, &pageCmd, 0, 0);
        osDelay(50); // 等待 LvHandlerTask 处理页面切换
      }

      /*************************** 进入休眠前操作 ***************************/
      // 关 LED
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
      // 关 RTC 唤醒定时器（防止周期性误唤醒）
      HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
      // 关 LCD 背光 + 复位 LCD
      LCD_Close_Light();
      LCD_RES_Clr();
      CST816_Sleep();

      // 清除 EXTI 挂起标志
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
      __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
      NVIC_ClearPendingIRQ(EXTI2_IRQn);
      NVIC_ClearPendingIRQ(EXTI4_IRQn);
      NVIC_ClearPendingIRQ(EXTI9_5_IRQn);

      /******************************************************************/

      /****************************** 进入 Stop 模式 *****************************/
      sleep:
      WDOG_Disnable();
      vTaskSuspendAll();
      CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
      HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

      // ====== 休眠中 ======

      /****************************** 退出 Stop 模式 *****************************/
      SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
      HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
      SystemClock_Config();
      xTaskResumeAll();

      /******************************************************************/

      /****************************** 唤醒判断 *****************************/
      // 延时去抖，等引脚稳定
      HAL_Delay(20);

      // 检查实际引脚状态
      if(!KEY1 || KEY2 || ChargeCheck())
      {
        // 真正唤醒：KEY 按下或充电状态变化，继续恢复
      }
      else
      {
        // 误唤醒：清除标志，回到休眠
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        NVIC_ClearPendingIRQ(EXTI2_IRQn);
        NVIC_ClearPendingIRQ(EXTI4_IRQn);
        NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        IdleTimerCount = 0;
        goto sleep;
      }

      /****************************** 恢复外设 *****************************/
      // 恢复 RTC 唤醒定时器
      HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2000, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

      LCD_Init();
      LCD_Set_Light(brightness);
      CST816_Wakeup();

      if(ChargeCheck())
      {
        HardInt_Charg_flag = 1;
      }

      // 恢复看门狗
      WDOG_Port_Init();
      WDOG_Enable();

      // 刷新主页数据
      osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);
      /******************************************************************/
    }

    WDOG_CheckIn(WDOG_CH_POWER);
    osDelay(10);
  }
}

/**
  * @brief  Idle timer callback (100ms period)
  * @param  argument: Not used
  * @retval None
  */
void IdleTimerCallback(void *argument)
{
  IdleTimerCount += 1;

  // Light off after 5 seconds (50 * 100ms)
  if(IdleTimerCount == 50)
  {
    uint8_t Idlestr = 0;
    osMessageQueuePut(Idle_MessageQueue, &Idlestr, 0, 1);
  }

  // Stop mode after 10 seconds (100 * 100ms)
  if(IdleTimerCount == 100)
  {
    uint8_t Stopstr = 1;
    IdleTimerCount = 0;
    osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
  }
	if(g_Sleep)
	{
		uint8_t Stopstr = 1;
    IdleTimerCount = 0;
		g_Sleep = false;
    osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
	}
}
