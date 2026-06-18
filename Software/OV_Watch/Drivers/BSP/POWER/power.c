#include "power.h"
#include "adc.h"
#include "delay.h"

#define INTERNAL_RES 0.128f
#define CHARGING_CUR 1

void Power_Pins_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(POWER_PORT, POWER_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = POWER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(POWER_PORT, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = CHARGE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(CHARGE_PORT, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

void Power_Enable()
{
	HAL_GPIO_WritePin(POWER_PORT,POWER_PIN,GPIO_PIN_SET);
}

void Power_DisEnable()
{
	HAL_GPIO_WritePin(POWER_PORT,POWER_PIN,GPIO_PIN_RESET);
}

uint8_t ChargeCheck()//1:charging
{
	return HAL_GPIO_ReadPin(CHARGE_PORT,CHARGE_PIN);
}

float BatCheck()
{
	uint16_t dat;
	float BatVoltage;
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1,5);
	dat = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	BatVoltage = dat *2 *3.3f /4096;
	return BatVoltage;
}

float BatCheck_8times()
{
	uint32_t dat=0;
	uint8_t i;
	float BatVoltage;
	for(i=0;i<16;i++)
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1,5);
		dat += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		delay_ms(1);
	}
	dat = dat>>4;
	BatVoltage = dat *2 *3.3f /4096;
	return BatVoltage;
}

uint8_t PowerCalculate()
{
	uint8_t power;
	float voltage;
	voltage = BatCheck_8times();

	if(ChargeCheck())
	{voltage -= INTERNAL_RES * CHARGING_CUR;}

	if(voltage >= 4.2f)
	{
		power = 100;
	}
	else if(voltage >= 3.68f)
	{
		// 3.68V~4.2V 线性插值映射到 10%~100%
		power = (uint8_t)(10 + (voltage - 3.68f) / (4.2f - 3.68f) * 90.0f);
	}
	else if(voltage >= 3.45f)
	{
		// 3.45V~3.68V 映射到 5%~10%
		power = (uint8_t)(5 + (voltage - 3.45f) / (3.68f - 3.45f) * 5.0f);
	}
	else
	{
		power = 0;
	}

	if(power > 100) power = 100;
	return power;
}

void Power_Init(void)
{
	Power_Pins_Init();
	Power_Enable();
}


