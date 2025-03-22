#ifndef MPU6050_H
#define MPU6050_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "Driver_I2C.h" 

#define MSGQUEUE_OBJECTS 10
			
	int Init_ThAcelerometro (void);

	//COLA
	typedef struct {                                // object data type
		float aceleracionX;
		float aceleracionY;
		float aceleracionZ;
		float temperatura;
	} MSGQUEUE_OBJ_tMPU;
	
	extern osMessageQueueId_t mid_MsgQueueMPU;
	
	//test
	//int Init_Th_Acelerometro_Test(void);
#endif
	