#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "leds_N.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThLEDS;                        // thread id
 
void LEDS (void *argument);// thread function

//funciones extra prototipos y variables
static uint32_t flag;

//FUNCIONALIDAD EXTRA ZUMBADOR 
static GPIO_InitTypeDef GPIO_InitStruct_pwm;
static TIM_OC_InitTypeDef htim2OCconf;
static TIM_HandleTypeDef htim2;

static void Init_PWM(void); //funcion para inicializar la señal PWM
//
//TEST
//void Th_LEDS_Test(void *argument); 
//osThreadId_t tid_Th_LEDS_Test;
//int Init_Th_LEDS_Test(void);
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
	
	Init_PWM();
	//Init_Th_LEDS_Test();
	
  while (1) {
    flag = osThreadFlagsWait(63, osFlagsWaitAny, osWaitForever);
		
		if((flag & LED1_ON) == LED1_ON){ //aceleración en X mayor que la referencia, led1 encendido 
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
			HAL_TIM_PWM_Start(&htim2 , TIM_CHANNEL_4); //ENCIENDO EL ZUMBADOR
		}
		if((flag & LED2_ON) == LED2_ON){ //aceleración en Y mayor que la referencia, led2 encendido
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);
		}
		if((flag & LED3_ON) == LED3_ON){ //aceleración en Z mayor que la referencia, led3 encendido
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
		}
		if((flag & LED1_OFF) == LED1_OFF){ //aceleración en X menor que la referencia, led1 apagado
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
			HAL_TIM_PWM_Stop(&htim2 , TIM_CHANNEL_4); //APAGO EL ZUMBADOR
		}
		if((flag & LED2_OFF) == LED2_OFF){ //aceleración en Y menor que la referencia, led2 apagado
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
		}
		if((flag & LED3_OFF) == LED3_OFF){ //aceleración en Z menor que la referencia, led3 apagado
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
		}
		
		flag = 0;
    osThreadYield();                            // suspend thread
  }
}

//ZUMBADOR
static void Init_PWM(void){
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();
  
  GPIO_InitStruct_pwm.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct_pwm.Pull = GPIO_PULLUP;
  GPIO_InitStruct_pwm.Alternate = GPIO_AF1_TIM2; //timer 2 de alternativo para pin 3
  
  GPIO_InitStruct_pwm.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_pwm);

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 999;
  htim2.Init.Period = 167;// resutlado de 500hz

  HAL_TIM_PWM_Init(&htim2);
  htim2OCconf.OCMode = TIM_OCMODE_PWM1;
  htim2OCconf.Pulse = 84; //50% de ciclo de trabajo
  HAL_TIM_PWM_ConfigChannel(&htim2,&htim2OCconf,TIM_CHANNEL_4);
}
//*************************************************************
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

//int Init_Th_LEDS_Test(void) {
// 
//  tid_Th_LEDS_Test = osThreadNew(Th_LEDS_Test, NULL, NULL);
//  if (tid_Th_LEDS_Test == NULL) {
//    return(-1);
//  }	
//  return(0);
//}
// 
//void Th_LEDS_Test (void *argument) {

//  while (1) {
//		osThreadFlagsSet(tid_ThLEDS, 1);
//		osDelay(1000);
//		
//		osThreadFlagsSet(tid_ThLEDS, 2);
//		osDelay(1000);
//		
//		osThreadFlagsSet(tid_ThLEDS, 4);
//		osDelay(1000);
//		
//		osThreadFlagsSet(tid_ThLEDS, 8);
//		osDelay(1000);
//		
//		osThreadFlagsSet(tid_ThLEDS, 16);
//		osDelay(1000);
//		
//		osThreadFlagsSet(tid_ThLEDS, 32);
//		osDelay(1000);
//  }
//}