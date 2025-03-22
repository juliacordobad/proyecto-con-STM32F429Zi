#include "clock.h"
//#include "LCD.h"
//#include "stdio.h" //para funcion sprintf
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
                 
 
void Clock (void *argument); 
static void Timer1seg_Callback (void const *arg) ;
//VARIABLES 

osThreadId_t tid_ThClock;  
int horasClk;
int minutosClk;
int segundosClk;

static osTimerId_t tim_id1seg; 
//static char horaDisplay[20];

//TEST
//void Th_Clock_Test(void *argument); 
//osThreadId_t tid_Th_Clock_Test;
//int Init_Th_Clock_Test(void);

//TEST PARA PROBAR MÓDULO DE HORA

//int Init_Th_Clock_Test(void) {
// 
//  tid_Th_Clock_Test = osThreadNew(Th_Clock_Test, NULL, NULL);
//  if (tid_Th_Clock_Test == NULL) {
//    return(-1);
//  }	
//  return(0);
//}
// 
//void Th_Clock_Test (void *argument) {
//	LCD_reset();
//	LCD_Init();
//	LCD_update();
//	
//  while (1) {
//		osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
//		resetBuffer();
//    sprintf(horaDisplay, "%02d:%02d:%02d", horasClk, minutosClk, segundosClk);
//	
//		for(int i = 0; horaDisplay[i] != '\0'; i++){
//			symbolToLocalBuffer(1, horaDisplay[i]);
//		}
//		LCD_update();
//  }
//}

int Init_ThClock (void) {
 
	tim_id1seg = osTimerNew((osTimerFunc_t)&Timer1seg_Callback, osTimerPeriodic, NULL, NULL);
  tid_ThClock = osThreadNew(Clock, NULL, NULL);
	
  if (tid_ThClock == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Clock (void *argument) {
 
		
	horasClk = 0;
	minutosClk = 0;
	segundosClk = 0;
	
	//TEST
	//Init_Th_Clock_Test();
	
	osTimerStart(tim_id1seg, 1000U);
	
  osThreadExit();
}

//TIMER 1S
static void Timer1seg_Callback (void const *arg) {
  // add user code here
	if(segundosClk<59){
		segundosClk++;
	}else if(segundosClk == 59){
		segundosClk = 0;
		if(minutosClk<59){
			minutosClk++;
		}else if(minutosClk == 59){
			minutosClk = 0;
			if(horasClk<23){
				horasClk++;
			}else{
				horasClk = 0;
				minutosClk = 0;
				segundosClk = 0;
			}
		}
	}
	//osThreadFlagsSet(tid_Th_Clock_Test, 1);
}

