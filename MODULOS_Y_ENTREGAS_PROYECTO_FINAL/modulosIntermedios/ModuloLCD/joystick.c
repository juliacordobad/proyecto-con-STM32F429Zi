#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "joystick.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThJoystick;                        // thread id
 
void Joystick (void *argument);                   // thread function
int Init_ThJoystick (void);

//JOYSTICK
static void InitDireccionAD(uint16_t GPIO_Pin);
static void InitDireccionAbIC(uint16_t GPIO_Pin);
void EXTI15_10_IRQHandler(void);
//***********************************************************************************************

//COLA PARA SINCRONIZACIÓN
osMessageQueueId_t mid_MsgQueueJoystick;
static tipoMensajeCola mensajesCola;
//***********************************************************************************************

//GESTIÓN DE REBOTES
osThreadId_t tid_ThREBOTES;
void REBOTES (void *argument);
int Init_ThREBOTES(void);
//**********************************************************************************************

//PULSACIÓN LARGA
osThreadId_t tid_ThPulsLARGA;
void PulsLARGA (void *argument);
int Init_ThPulsLARGA(void);
//**********************************************************************************************

//TEST
osThreadId_t tid_ThTEST;
void TEST (void *argument);
int Init_ThTEST(void);
char boton[10];
//***********************************************************************************************
int Init_ThJoystick (void) {
 
  tid_ThJoystick = osThreadNew(Joystick, NULL, NULL);
  if (tid_ThJoystick == NULL) {
    return(-1);
  }
 
	mid_MsgQueueJoystick = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(tipoMensajeCola), NULL);
	
  return(0);
}
 
void Joystick (void *argument) {
 
	//inicialización de los distintos botones del joystick
	InitDireccionAD(GPIO_PIN_11); //derecha
	InitDireccionAD(GPIO_PIN_10); //arriba
	InitDireccionAbIC(GPIO_PIN_12); //abajo
	InitDireccionAbIC(GPIO_PIN_14); //izquierda
	InitDireccionAbIC(GPIO_PIN_15); //centro
	
	Init_ThREBOTES();
	Init_ThPulsLARGA();
	Init_ThTEST();
	
  while (1) {
    ; // Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}

//FUNCIONES EXTRA
	//INICIALIZACIÓN JOYSTICK
//ARRIBA Y DERECHA GPIOB
static void InitDireccionAD(uint16_t GPIO_Pin){
	
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //habilitar interrupciones
    __HAL_RCC_GPIOB_CLK_ENABLE();
		
    //Configuraci?n de los pines
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = GPIO_Pin;
		//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //Inicializaci?n de los pines
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
//ABAJO, IZQUIERDA Y CENTRO GPIOE
static void InitDireccionAbIC(uint16_t GPIO_Pin){
	
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //habilitar interrupciones
    __HAL_RCC_GPIOE_CLK_ENABLE();
		
    //Configuraci?n de los pines
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = GPIO_Pin;
		//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //Inicializaci?n de los pines
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_10){ //arriba
		mensajesCola.boton = 1;
    osThreadFlagsSet(tid_ThREBOTES, 1);
		
	}
	if(GPIO_Pin == GPIO_PIN_11){ //derecha
		mensajesCola.boton = 2;
    osThreadFlagsSet(tid_ThREBOTES, 1);
		
	}
	if(GPIO_Pin == GPIO_PIN_12){ //abajo
		mensajesCola.boton = 3;
    osThreadFlagsSet(tid_ThREBOTES, 1);
		
	}
	if(GPIO_Pin == GPIO_PIN_14){ //izquierda
		mensajesCola.boton = 4;
    osThreadFlagsSet(tid_ThREBOTES, 1);
		
	}
	if(GPIO_Pin == GPIO_PIN_15){ //centro
		mensajesCola.boton = 5;
    osThreadFlagsSet(tid_ThREBOTES, 1);
		
	}

}

//GESTIÓN DE REBOTES
int Init_ThREBOTES (void) {
 
  tid_ThREBOTES = osThreadNew(REBOTES, NULL, NULL);
  if (tid_ThREBOTES == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void REBOTES (void *argument) {
 
  while (1) {
    osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		osDelay(50);
		osThreadFlagsSet(tid_ThPulsLARGA, 2);
		// Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}

//GESTIÓN DE REBOTES
int Init_ThPulsLARGA (void) {
 
  tid_ThPulsLARGA = osThreadNew(PulsLARGA, NULL, NULL);
  if (tid_ThPulsLARGA == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void PulsLARGA (void *argument) {
 
  while (1) {
    osThreadFlagsWait(2, osFlagsWaitAll, osWaitForever);
		osDelay(1000);
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) | 
			 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) |
		   HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) |
			 HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) |
			 HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) ){
				 strcpy(mensajesCola.tipoPulsacion, "larga");
			 }else{
				 strcpy(mensajesCola.tipoPulsacion, "corta");
			 }
				
		osMessageQueuePut(mid_MsgQueueJoystick, &mensajesCola, NULL, NULL);
		osThreadFlagsSet(tid_ThTEST, 4);
		// Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}

//TEST
int Init_ThTEST(void) {
 
  tid_ThTEST = osThreadNew(TEST, NULL, NULL);
  if (tid_ThTEST == NULL) {
    return(-1);
  }	
  return(0);
}
 
void TEST (void *argument) {
	LCD_reset();
	LCD_Init();
	LCD_update();
	
  while (1) {
		osThreadFlagsWait(4, osFlagsWaitAll, osWaitForever);
		resetBuffer();
		
		if(mensajesCola.boton == 1){
			sprintf(boton, "arriba");
		}else if(mensajesCola.boton == 2){
			sprintf(boton, "derecha");
		}else if(mensajesCola.boton == 3){
			sprintf(boton, "abajo");
		}else if(mensajesCola.boton == 4){
			sprintf(boton, "izquierda");
		}else if(mensajesCola.boton == 5){
			sprintf(boton, "centro");
		}
    
	
		for(int i = 0; boton[i] != '\0'; i++){
			symbolToLocalBuffer(1, boton[i]);
		}
		
		for(int i = 0; mensajesCola.tipoPulsacion[i] != '\0'; i++){
			symbolToLocalBuffer(2, mensajesCola.tipoPulsacion[i]);
		}
		
		LCD_update();
  }
}