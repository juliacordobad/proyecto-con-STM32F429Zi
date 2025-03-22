#ifndef COM_H
#define COM_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "Driver_USART.h"
#include "string.h"

#define SOH  0x01 //Inicio de una trama
#define EOT  0xFE //Final de una trama

#define HORA 0x20
#define Ax 0x25
#define Ay 0x26
#define Az 0x27
#define SolicitarMedidas 0x55
#define BorrarMedidas 0x60
#define RespuestaSolicitarMedidas 0xAF
	
#define MSGQUEUE_OBJECTS_COM 10

	int Init_ThCOM (void);
	
	typedef struct {                                // object data type
		char CMD;
		char LEN;
		char Payload[40];
	} MSGQUEUE_OBJ_t_COM;

	extern osMessageQueueId_t mid_MsgQueueRECIBIDO;
	extern osMessageQueueId_t mid_MsgQueueTRANSMITIR;
	
	//test
	int Init_Test(void);
	
#endif
	