#ifndef leds_N_h
#define leds_N_h

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#define LED1_ON 1
#define LED1_OFF 8

#define LED2_ON 2
#define LED2_OFF 16

#define LED3_ON 4
#define LED3_OFF 32

	extern osThreadId_t tid_ThLEDS;  
	
	int Init_ThLEDS (void);

#endif