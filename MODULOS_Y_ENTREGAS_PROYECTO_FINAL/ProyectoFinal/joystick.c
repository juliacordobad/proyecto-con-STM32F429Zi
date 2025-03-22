//#include "cmsis_os2.h"                          // CMSIS RTOS header file
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

//COLA PARA SINCRONIZACI�N
osMessageQueueId_t mid_MsgQueueJoystick;
static MSGQUEUE_OBJ_tJoystick mensajesCola;
//***********************************************************************************************

//JOYSTICK Y GESTI�N DE REBOTES
osThreadId_t tid_ThJoystick;
void Joystick (void *argument);
int Init_ThJoystick(void);
static uint16_t codigoBoton;
//**********************************************************************************************

//PULSACI�N LARGA
osThreadId_t tid_ThPulsLARGA;
void PulsLARGA (void *argument);
int Init_ThPulsLARGA(void);
static uint16_t botonPulsado;
//**********************************************************************************************

//TEST
osThreadId_t tid_ThTEST;
void TEST (void *argument);
int Init_ThTEST(void);
static MSGQUEUE_OBJ_tJoystick mensajesColaRecibidoTest;
static char nombreBoton[10];
//***********************************************************************************************

//GESTI�N DE REBOTES
int Init_ThJoystick (void) {
 
	InitDireccionAD(GPIO_PIN_11); //derecha
	InitDireccionAD(GPIO_PIN_10); //arriba
	InitDireccionAbIC(GPIO_PIN_12); //abajo
	InitDireccionAbIC(GPIO_PIN_14); //izquierda
	InitDireccionAbIC(GPIO_PIN_15); //centro
	
	Init_ThPulsLARGA();
	//Init_ThTEST();
	
	mid_MsgQueueJoystick = osMessageQueueNew(MSGQUEUE_OBJECTS_JOYSTICK, sizeof(MSGQUEUE_OBJ_tJoystick), NULL);
	
  tid_ThJoystick = osThreadNew(Joystick, NULL, NULL);
  if (tid_ThJoystick == NULL) {
    return(-1);
  }
  return(0);
}
 
void Joystick (void *argument) {
 
  while (1) {
    osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
		osDelay(50);
		
		osThreadFlagsSet(tid_ThPulsLARGA, 2);
		// Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}

//TIPO DE PULSACI�N
int Init_ThPulsLARGA (void) {
 
  tid_ThPulsLARGA = osThreadNew(PulsLARGA, NULL, NULL);
  if (tid_ThPulsLARGA == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void PulsLARGA (void *argument) {
 
	uint8_t contadorPulsacionLarga = 0;
	
  while (1) {
		
		osThreadFlagsWait(2, osFlagsWaitAll, osWaitForever);
		
    switch(botonPulsado){
			case GPIO_PIN_10:
				while((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) /*&& botonPulsado ==  GPIO_PIN_15*/)){
					osDelay(50);
					contadorPulsacionLarga++;
				}
				break;
			case GPIO_PIN_11:
				while((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) /*&& botonPulsado ==  GPIO_PIN_15*/)){
					osDelay(50);
					contadorPulsacionLarga++;
				}
				break;
			case GPIO_PIN_12:
				while((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) /*&& botonPulsado ==  GPIO_PIN_15*/)){
					osDelay(50);
					contadorPulsacionLarga++;
				}
				break;
			case GPIO_PIN_14:
				while((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) /*&& botonPulsado ==  GPIO_PIN_15*/)){
					osDelay(50);
					contadorPulsacionLarga++;
				}
				break;
			case GPIO_PIN_15:
				while((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) /*&& botonPulsado ==  GPIO_PIN_15*/)){
					osDelay(50);
					contadorPulsacionLarga++;
				}
				break;
		}
		
		if(contadorPulsacionLarga >= 20){
			mensajesCola.tipoPulsacion = 1; //larga+
			osMessageQueuePut(mid_MsgQueueJoystick, &mensajesCola, NULL, NULL);
		}else if(contadorPulsacionLarga > 1){
			mensajesCola.tipoPulsacion = 0;
			osMessageQueuePut(mid_MsgQueueJoystick, &mensajesCola, NULL, NULL);
		}
		
		contadorPulsacionLarga = 0;
		botonPulsado = 0x00;
		
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
			 
		// Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}
//FUNCIONES EXTRA
	//INICIALIZACI�N JOYSTICK
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
	}
	if(GPIO_Pin == GPIO_PIN_11){ //derecha
		mensajesCola.boton = 2;
	}
	if(GPIO_Pin == GPIO_PIN_12){ //abajo
		mensajesCola.boton = 3;	
	}
	if(GPIO_Pin == GPIO_PIN_14){ //izquierda
		mensajesCola.boton = 4;
	}
	if(GPIO_Pin == GPIO_PIN_15){ //centro
		mensajesCola.boton = 5;
	}
	botonPulsado = GPIO_Pin;
	osThreadFlagsSet(tid_ThJoystick, 1);

}

////TEST
//int Init_ThTEST(void) {
// 
//  tid_ThTEST = osThreadNew(TEST, NULL, NULL);
//  if (tid_ThTEST == NULL) {
//    return(-1);
//  }	
//  return(0);
//}
// 
//void TEST (void *argument) {
//	LCD_reset();
//	LCD_Init();
//	LCD_update();
//	
//	char tipoPulsacionPalabra[6];
//  while (1) {
//		osMessageQueueGet(mid_MsgQueueJoystick, &mensajesColaRecibidoTest, NULL, osWaitForever);
//		resetBuffer();
//		
//		if(mensajesColaRecibidoTest.boton == 1){
//			sprintf(nombreBoton, "arriba");
//		}else if(mensajesColaRecibidoTest.boton == 2){
//			sprintf(nombreBoton, "derecha");
//		}else if(mensajesColaRecibidoTest.boton == 3){
//			sprintf(nombreBoton, "abajo");
//		}else if(mensajesColaRecibidoTest.boton == 4){
//			sprintf(nombreBoton, "izquierda");
//		}else if(mensajesColaRecibidoTest.boton == 5){
//			sprintf(nombreBoton, "centro");
//		}
//    
//	
//		for(int i = 0; nombreBoton[i] != '\0'; i++){
//			symbolToLocalBuffer(1, nombreBoton[i]);
//		}
//		
//		if(mensajesCola.tipoPulsacion == 1){
//			sprintf(tipoPulsacionPalabra, "larga");
//		}else{
//			sprintf(tipoPulsacionPalabra, "corta");
//		}
//		for(int i = 0; tipoPulsacionPalabra[i] != '\0'; i++){
//			symbolToLocalBuffer(2, tipoPulsacionPalabra[i]);
//		}
//		
//		LCD_update();
//  }
//}