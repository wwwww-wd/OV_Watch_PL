/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
#include "user_RunModeTasks.h"
#include "main.h"
#include "lcd.h"
#include "HWDataAccess.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint16_t IdleTimerCount = 0;

/* Private function prototypes -----------------------------------------------*/

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
      LCD_Set_Light(90);
    }
    
    // Stop mode entry (placeholder - full implementation later)
    if(osMessageQueueGet(Stop_MessageQueue, &Stopstr, NULL, 0) == osOK)
    {
      // TODO: Implement stop mode with STM32 STOP
      // For now, just reset idle timer
      IdleTimerCount = 0;
    }
    
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
  
  // Stop mode after 15 seconds (150 * 100ms)
  if(IdleTimerCount == 150)
  {
    uint8_t Stopstr = 1;
    IdleTimerCount = 0;
    osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
  }
}
