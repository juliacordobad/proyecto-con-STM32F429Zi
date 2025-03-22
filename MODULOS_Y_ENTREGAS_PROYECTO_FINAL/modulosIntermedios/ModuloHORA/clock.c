#include "clock.h"
#include "LCD.h"
#include "stdio.h" //para funcion sprintf
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
                 
 
void Clock (void *argument); 
static void Timer2_Callback (void const *arg) ;
//VARIABLES 

osThreadId_t tid_ThClock;  
volatile uint8_t horas;
volatile uint8_t minutos;
volatile uint8_t segundos;

static osTimerId_t tim_id2; 
static char horaDisplay[20];

//TEST
void Th_Clock_Test(void *argument); 
osThreadId_t tid_Th_Clock_Test;
int Init_Th_Clock_Test(void);

//TEST PARA PROBAR MÓDULO DE HORA

int Init_Th_Clock_Test(void) {
 
  tid_Th_Clock_Test = osThreadNew(Th_Clock_Test, NULL, NULL);
  if (tid_Th_Clock_Test == NULL) {
    return(-1);
  }	
  return(0);
}
 
void Th_Clock_Test (void *argument) {
	LCD_reset();
	LCD_Init();
	LCD_update();
	
  while (1) {
		osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
		resetBuffer();
    sprintf(horaDisplay, "%02d:%02d:%02d", horas, minutos, segundos);
	
		for(int i = 0; horaDisplay[i] != '\0'; i++){
			symbolToLocalBuffer(1, horaDisplay[i]);
		}
		LCD_update();
  }
}

int Init_ThClock (void) {
 
	tim_id2 = osTimerNew((osTimerFunc_t)&Timer2_Callback, osTimerPeriodic, NULL, NULL);
  tid_ThClock = osThreadNew(Clock, NULL, NULL);
	
  if (tid_ThClock == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Clock (void *argument) {
 
		
	horas = 23;
	minutos = 59;
	segundos = 50;
	
	//TEST
	Init_Th_Clock_Test();
	
	osTimerStart(tim_id2, 1000U);
	
  osThreadExit();
}

//TIMER 1S
static void Timer2_Callback (void const *arg) {
  // add user code here
	if(segundos<59){
		segundos++;
	}else if(segundos == 59){
		segundos = 0;
		if(minutos<59){
			minutos++;
		}else if(minutos == 59){
			minutos = 0;
			if(horas<23){
				horas++;
			}else{
				horas = 0;
				minutos = 0;
				segundos = 0;
			}
		}
	}
	osThreadFlagsSet(tid_Th_Clock_Test, 1);
}

