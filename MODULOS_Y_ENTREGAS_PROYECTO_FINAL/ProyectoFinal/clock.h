#ifndef clock_h
#define clock_h

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
//#include "LCD.h"
//#include "stdio.h" //para funcion sprintf

	extern osThreadId_t tid_ThClock;  
	
	extern int horasClk;
	extern int minutosClk;
	extern int segundosClk;
	
	                  
	int Init_ThClock (void);

	//test
	//int Init_Th_Clock_Test(void);
#endif
