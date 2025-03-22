#ifndef _LCD_
#define _LCD_
	
	#include "stm32f4xx_hal.h"
//	#include "Driver_SPI.h"
//	#include "Arial12x12.h"

	
	//FUNCIONES QUE SE PUEDEN UTILIZAR PARA CONTROLAR EL LCD
	
	void symbolToLocalBuffer(uint8_t line,uint8_t symbol);
	void LCD_reset(void);
	void LCD_Init(void);
	void LCD_update(void);
	void resetBuffer(void);
	
#endif
