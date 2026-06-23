/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
#include "user_ChargCheckTask.h"
#include "main.h"
#include "HWDataAccess.h"
#include "power.h"
#include "PageManager.h"
#include "ui_Charging_Page.h"
#include "stm32f4xx_it.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Charge detection task
  * @note   Detects charge cable insertion/removal via EXTI flag.
  *         Sends page commands through message queue (LVGL safe).
  * @param  argument: Not used
  * @retval None
  */
void ChargCheckTask(void *argument)
{
  while(1)
  {
    if(HardInt_Charg_flag)
    {
      HardInt_Charg_flag = 0;

      if(ChargeCheck() && Page_Get_NowPage()->page_obj != &ui_Charging_Page)
      {
        // Charging inserted, not already on charge page → load charge page
        uint8_t pageCmd = 3; // Page_Load(&Page_Charg)
        osMessageQueuePut(PageCmd_MessageQueue, &pageCmd, 0, 0);
      }
      else if(!ChargeCheck() && Page_Get_NowPage()->page_obj == &ui_Charging_Page)
      {
        // Charging removed, on charge page → go back
        uint8_t pageCmd = 1; // Page_Back()
        osMessageQueuePut(PageCmd_MessageQueue, &pageCmd, 0, 0);
      }
    }
    osDelay(500);
  }
}
