/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
#include "user_SensUpdateTask.h"
#include "main.h"
#include "HWDataAccess.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Tasks ---------------------------------------------------------------------*/

/**
  * @brief  Sensor data update task
  * @param  argument: Not used
  * @retval None
  */
void SensorTask(void *argument)
{
  uint16_t last_steps = 0;
  uint16_t save_cnt = 0;
  
  while(1)
  {
    // Update home page data on HomeUpdata message
    uint8_t HomeUpdataStr;
    if(osMessageQueueGet(HomeUpdata_MessageQueue, &HomeUpdataStr, NULL, 0) == osOK)
    {
      // Battery
      HWInterface.Power.power_remain = HWInterface.Power.BatCalculate();
      if(HWInterface.Power.power_remain > 100)
      {
        HWInterface.Power.power_remain = 0;
      }
    }
    
    // Steps update
    if(!HWInterface.IMU.ConnectionError)
    {
      HWInterface.IMU.Steps = HWInterface.IMU.GetSteps();
    }
    
    // 每60秒或步数变化超过10步时保存
    save_cnt++;
    uint16_t current_steps = HWInterface.IMU.GetSteps();
    if(save_cnt >= 120 || (current_steps > last_steps + 10))
    {
      save_cnt = 0;
      last_steps = current_steps;
      uint8_t Datastr = 0;
      osMessageQueuePut(DataSave_MessageQueue, &Datastr, 0, 1);
    }
    
    osDelay(500);
  }
}
