#ifndef principal_h
#define principal_h

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

//demás módulos
#include "clock.h"
#include "mpu6050.h"
#include "joystick.h"
#include "leds_N.h"
#include "com.h"
#include "lcd.h"

//HILO PRINCIPAL
	extern osThreadId_t tid_ThPrincipal;                   
	int Init_ThPrincipal (void);
//**********************************************************************************

	//test
	//int Init_Th_Clock_Test(void);
#endif
