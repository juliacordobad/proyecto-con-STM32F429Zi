#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "leds_N.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThLEDS;                        // thread id
 
void LEDS (void *argument);// thread function

//funciones extra prototipos y variables
static uint32_t flag;

//TEST
//TEST
void Th_LEDS_Test(void *argument); 
osThreadId_t tid_Th_LEDS_Test;
int Init_Th_LEDS_Test(void);
//***********************************************************************************
static void InitLeds(uint16_t GPIO_Pin);

int Init_ThLEDS (void) {
 
  tid_ThLEDS = osThreadNew(LEDS, NULL, NULL);
  if (tid_ThLEDS == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void LEDS (void *argument) {
	InitLeds(GPIO_PIN_0); //led1
	InitLeds(GPIO_PIN_7); //led2
	InitLeds(GPIO_PIN_14); //led3
	
	Init_Th_LEDS_Test();
	
  while (1) {
    flag = osThreadFlagsWait(63, osFlagsWaitAny, osWaitForever);
		
		if(flag == LED1_ON){ //aceleración en X mayor que la referencia
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
		}else if(flag == LED2_ON){ //aceleración en Y mayor que la referencia
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);
		}else if(flag == LED3_ON){ //aceleración en Z mayor que la referencia
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		}else if(flag == LED1_OFF){ //aceleración en X menor que la referencia
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
		}else if(flag == LED2_OFF){ //aceleración en Y menor que la referencia
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
		}else if(flag == LED3_OFF){ //aceleración en Z menor que la referencia
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		}else{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		}
			
		
    osThreadYield();                            // suspend thread
  }
}

//FUNCIONES EXTRA

	//INICIALIZACIÓN DE LOS LEDS
static void InitLeds(uint16_t GPIO_Pin){
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		__HAL_RCC_GPIOB_CLK_ENABLE();
    //Configuraci?n de los pines
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = GPIO_Pin;

    //Inicializaci?n de los pines
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

//TEST
//TEST PARA PROBAR MÓDULO DE HORA

int Init_Th_LEDS_Test(void) {
 
  tid_Th_LEDS_Test = osThreadNew(Th_LEDS_Test, NULL, NULL);
  if (tid_Th_LEDS_Test == NULL) {
    return(-1);
  }	
  return(0);
}
 
void Th_LEDS_Test (void *argument) {

  while (1) {
		osThreadFlagsSet(tid_ThLEDS, LED1_ON);
		osDelay(1000);
		
		osThreadFlagsSet(tid_ThLEDS, LED2_ON);
		osDelay(1000);
		
		osThreadFlagsSet(tid_ThLEDS, LED3_ON);
		osDelay(1000);
		
		osThreadFlagsSet(tid_ThLEDS, LED1_OFF);
		osDelay(1000);
		
		osThreadFlagsSet(tid_ThLEDS, LED2_OFF);
		osDelay(1000);
		
		osThreadFlagsSet(tid_ThLEDS, LED3_OFF);
		osDelay(1000);
  }
}