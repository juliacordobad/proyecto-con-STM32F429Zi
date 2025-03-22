#ifndef lcd_h
#define lcd_h
	
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "string.h"
#include "Driver_SPI.h"
#include "stdio.h"
	
	//FUNCIONES QUE SE PUEDEN UTILIZAR PARA CONTROLAR EL LCD
	void symbolToLocalBuffer(uint8_t line,uint8_t symbol);
	void LCD_reset(void);
	void LCD_Init(void);
	void LCD_update(void);
	void resetBuffer(void);
	//************************************************
	
	//PARA LA COLA
	#define MSGQUEUE_OBJECTS 10
	
	typedef struct {                                // object data type
		uint8_t modo;
		uint8_t horas;
		uint8_t minutos;
		uint8_t segundos;
		float temperatura;
		float aceleracionX;
		float aceleracionY;
		float aceleracionZ;
		float refAx;
		float refAy;
		float refAz;
		uint8_t tipoDepuracion;
		
	} MSGQUEUE_OBJ_tLCD;
	
	extern osMessageQueueId_t mid_MsgQueueLCD;
	//********************************************************
	
	//inicializar hilo LCD
	int Init_ThLCD (void);
	//********************************************************
#endif