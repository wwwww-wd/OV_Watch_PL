/* Private includes -----------------------------------------------------------*/
#include "user_TasksInit.h"
#include "user_ScrRenewTask.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Screen renew task
  * @param  argument: Not used
  * @retval None
  */
void ScrRenewTask(void *argument)
{
	uint8_t keystr = 0;
	uint8_t pageCmd = 0;
	while(1)
	{
		if(osMessageQueueGet(Key_MessageQueue, &keystr, NULL, 0) == osOK)
		{
			if(keystr == 1)
			{
				pageCmd = 1;
				osMessageQueuePut(PageCmd_MessageQueue, &pageCmd, 0, 0);
			}
			else if(keystr == 2)
			{
				pageCmd = 2;
				osMessageQueuePut(PageCmd_MessageQueue, &pageCmd, 0, 0);
			}
		}
		osDelay(10);
	}
}
