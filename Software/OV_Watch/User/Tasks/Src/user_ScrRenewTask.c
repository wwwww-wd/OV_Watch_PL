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
	while(1)
	{
		if(osMessageQueueGet(Key_MessageQueue, &keystr, NULL, 0) == osOK)
		{
			// TODO: Implement page navigation with PageManager
			// For now, just consume the message
			(void)keystr;
		}
		osDelay(10);
	}
}
