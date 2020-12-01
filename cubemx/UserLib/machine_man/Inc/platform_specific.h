#include "string.h"
#include "machine.h" 


#define SMPS_Enable	 		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET)
#define SMPS_Disable		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET)
#define LDO_Enable	 		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_SET)
#define LDO_Disable	 		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_10,GPIO_PIN_RESET)
#define PER3V3_Enable	 	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_SET)
#define PER3V3_Disable	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_13,GPIO_PIN_RESET)

#define GSM_PWR_enable		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET)
#define GSM_PWR_disable		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_RESET)
#define GSM_Rst_enable		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET)
#define GSM_Rst_disable		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET)
#define GSM_pwrkey_on			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET)
#define GSM_pwrkey_off		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET)


void init_all_command_window(void);

