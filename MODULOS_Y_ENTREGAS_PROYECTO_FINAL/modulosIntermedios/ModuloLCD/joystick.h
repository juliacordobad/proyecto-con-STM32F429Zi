#ifndef LEDS_H
#define LEDS_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <string.h>

//TEST
#include "LCD.h"

#define MSGQUEUE_OBJECTS 10

	//tipo de los mensajes en la cola
	typedef struct{
			char tipoPulsacion[5];
			uint8_t boton;
		} tipoMensajeCola;
	//********************************
	extern osThreadId_t tid_ThJoystick;  
	
	extern osMessageQueueId_t mid_MsgQueueJoystick;
	
	int Init_ThJoystick (void);

	//test
	int Init_Th_Joystick_Test(void);
#endif