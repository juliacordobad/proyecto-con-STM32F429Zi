//#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "mpu6050.h"
//INCLUDES PARA EL TEST CON LCD
//#include "LCD.h"
//#include "stdio.h"
//**************************************************************
#define INIT_FLAG 0x01 //Flag que da comienzo al reloj

#define SLAVE_ADDRESS 0x68
#define MPU6050_SCALE_FACTOR 16384.0 //2^16
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
osThreadId_t tid_ThAcelerometro;                        // thread id

//COLA 
osMessageQueueId_t mid_MsgQueueMPU;  

//TIMER 1 SEGUNDO PARA ACTUALIZACIÓN
static osTimerId_t tim_actualizacion; 
static void TimerActualizacion_Callback (void const *arg);
//***********************************************************************

//ACELEROMETRO 12C
/* I2C driver instance */
extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
static const uint8_t registroAceleracion = 0x3B;
static const uint8_t registroTemperatura = 0x41;
static uint8_t medidas[8];
static MSGQUEUE_OBJ_tMPU medidasCola;

void Callback_I2C (uint32_t event);
static void leerAcelerometro(void);
static int8_t I2C_Init(void);
static void convertirMedidas(void);
	
//**********************************************************************************

//TEST*****************************************************************
//void Th_Acelerometro_Test(void *argument); 
//osThreadId_t tid_Th_Acelerometro_Test;
//int Init_Th_Acelerometro_Test(void);
//static char acelerometroDisplay[20];
//static char temperaturaDisplay[40];
//**********************************************************************************************************
void Acelerometro (void *argument);                   // thread function
 
int Init_ThAcelerometro (void) {
 
	tim_actualizacion = osTimerNew((osTimerFunc_t)&TimerActualizacion_Callback, osTimerPeriodic, NULL, NULL);
	
  tid_ThAcelerometro = osThreadNew(Acelerometro, NULL, NULL);
	mid_MsgQueueMPU = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(MSGQUEUE_OBJ_tMPU), NULL);
	
  if (tid_ThAcelerometro == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Acelerometro (void *argument) {
	//INICIALIZACIÓN I2C PARA ACELEROMETRO
	I2C_Init();
	
	//INICIALIZACIÓN HILO DE TEST
	//Init_Th_Acelerometro_Test();
	
	osTimerStart(tim_actualizacion, 1000U);

  while (1) {
    // Insert thread code here...
		osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
		leerAcelerometro();
		convertirMedidas();
		osMessageQueuePut(mid_MsgQueueMPU, &medidasCola, NULL, 0U);
    osThreadYield();                            // suspend thread
  }
}

//TIMER
static void TimerActualizacion_Callback (void const *arg) {
  // add user code here
	
	osThreadFlagsSet(tid_ThAcelerometro, 1);
}

//FUNCIONES EXTRA PARA ACELEROMETRO I2C
static void leerAcelerometro(void){
	I2Cdrv->MasterTransmit(SLAVE_ADDRESS, &registroAceleracion, 1, true);
	//while (I2Cdrv->GetStatus().busy);
	osThreadFlagsWait(INIT_FLAG, osFlagsWaitAll, osWaitForever);
	
	I2Cdrv->MasterReceive(SLAVE_ADDRESS, medidas, 8, false);
	//while (I2Cdrv->GetStatus().busy);	
	osThreadFlagsWait(INIT_FLAG, osFlagsWaitAll, osWaitForever);
}

static int8_t I2C_Init(void){
	I2Cdrv->Initialize  (Callback_I2C);
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  I2Cdrv->Control     (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control     (ARM_I2C_BUS_CLEAR, 0);
	
	uint8_t config[] = {0x6B, 0x00}; // 0x6B es el registro de Power Management, 0x00 para activarlo
	I2Cdrv->MasterTransmit(SLAVE_ADDRESS, config, 2, false);
	//while (I2Cdrv->GetStatus().busy);
	osThreadFlagsWait(INIT_FLAG, osFlagsWaitAll, osWaitForever);
	
	return 0;
}

static void convertirMedidas(void){
	int16_t convertidor;
	
	convertidor = (medidas[0] << 8) | medidas[1];
	medidasCola.aceleracionX = (convertidor / MPU6050_SCALE_FACTOR)  /**9.8*/; //resultado en m/s^2
	
	convertidor = (medidas[2] << 8) | medidas[3];
	medidasCola.aceleracionY = (convertidor / MPU6050_SCALE_FACTOR)  /**9.8*/; //resultado en m/s^2
	
	convertidor = (medidas[4] << 8) | medidas[5];
	medidasCola.aceleracionZ = (convertidor / MPU6050_SCALE_FACTOR) /**9.8*/; //resultado en m/s^2
	
	convertidor = (medidas[6] << 8) | medidas[7];
	medidasCola.temperatura = (convertidor)/340.0 + 36.53;
}

//Función Callback del I2C
void Callback_I2C (uint32_t event){
  uint32_t mask = ARM_I2C_EVENT_TRANSFER_DONE;
  if(event & mask){
    osThreadFlagsSet(tid_ThAcelerometro, INIT_FLAG); 
  }
}
//******************************************************************

//TEST
//int Init_Th_Acelerometro_Test(void) {
// 
//  tid_Th_Acelerometro_Test = osThreadNew(Th_Acelerometro_Test, NULL, NULL);
//  if (tid_Th_Acelerometro_Test == NULL) {
//    return(-1);
//  }	
//  return(0);
//}
// 
//void Th_Acelerometro_Test (void *argument) {
//	LCD_reset();
//	LCD_Init();
//	LCD_update();
//	
//	static MSGQUEUE_OBJ_t mensaje_recibido;
//	
//  while (1) {
//		osMessageQueueGet(mid_MsgQueue, &mensaje_recibido, NULL, osWaitForever);
//		
//    sprintf(temperaturaDisplay, "T:%.1f", mensaje_recibido.temperatura);
//		sprintf(acelerometroDisplay, "X:%.1f Y:%.1f Z:%.1f", mensaje_recibido.aceleracionX, mensaje_recibido.aceleracionY, mensaje_recibido.aceleracionZ);
//		
//		for(int i = 0; temperaturaDisplay[i] != '\0'; i++){
//			symbolToLocalBuffer(1, temperaturaDisplay[i]);
//		}
//		for(int i = 0; acelerometroDisplay[i] != '\0'; i++){
//			symbolToLocalBuffer(2, acelerometroDisplay[i]);
//		}
//		
//		LCD_update();
//		resetBuffer();
//  }
//}