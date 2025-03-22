#ifndef CLOCK_H
#define CLOCK_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
//#include "LCD.h"
//#include "stdio.h" //para funcion sprintf

	extern osThreadId_t tid_ThClock;  
	extern volatile uint8_t horas;
	extern volatile uint8_t minutos;
	extern volatile uint8_t segundos;
	
	                  
	int Init_ThClock (void);

	//test
	int Init_Th_Clock_Test(void);
#endif
