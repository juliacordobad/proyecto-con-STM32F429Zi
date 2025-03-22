#ifndef joystick_h
#define joystick_h

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <string.h>

//TEST
//#include "LCD.h"

#define MSGQUEUE_OBJECTS_JOYSTICK 1

	//tipo de los mensajes en la cola
	typedef struct{
			uint8_t tipoPulsacion;
			uint8_t boton;
		} MSGQUEUE_OBJ_tJoystick;
	//********************************
	extern osThreadId_t tid_ThJoystick;  
	
	extern osMessageQueueId_t mid_MsgQueueJoystick;
	
	int Init_ThJoystick (void);

	//test
	//int Init_Th_Joystick_Test(void);
#endif