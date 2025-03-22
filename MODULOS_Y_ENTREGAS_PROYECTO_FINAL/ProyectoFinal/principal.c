#include "principal.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
       
//VARIABLES PARA SINCRONIZACIÓN CONJUNTA
uint8_t estado;

static float refAx;
static float refAy;
static float refAz;

static float refAxDepuracion;
static float refAyDepuracion;
static float refAzDepuracion;

uint8_t posicionHora;
uint8_t posicionAceleracion;
uint8_t configurarAceleracion;

int horasDepuracion;
int minutosDepuracion;
int segundosDepuracion;

//****************************************************

//FUNCIONES MODULO PRINCIPAL
static void gestionarPulsaciones(void);
static void configurarReferenciasAceleracion(void);
//****************************************************

//VARIABLES LCD
MSGQUEUE_OBJ_tLCD colaLCD;
//*****************************************************
 
//VARIABLES JOYSTICK
MSGQUEUE_OBJ_tJoystick colaJoystick;
//*****************************************************

//VARIABLES MPU
MSGQUEUE_OBJ_tMPU colaMPU;
static char bufferMedidas[10][40]; //array de 10 posiciones y cada posición hasta 40 caracteres
static uint8_t posicionBuffer;
//*****************************************************

//VARIABLES COM-PC
MSGQUEUE_OBJ_t_COM colaCOMPCrecibido;
//*****************************************************

void Principal (void *argument); 
osThreadId_t tid_ThPrincipal;

int Init_ThPrincipal (void) {
 
  tid_ThPrincipal = osThreadNew(Principal, NULL, NULL);
	
  if (tid_ThPrincipal == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Principal (void *argument) {
 
	refAx = 1.0f;
	refAy = 1.0f;
	refAz = 1.0f;
		
	colaLCD.refAx = refAx;
	colaLCD.refAy = refAy;
	colaLCD.refAz = refAz;
	
	Init_ThClock();
	Init_ThLCD();
	Init_ThJoystick();
	Init_ThAcelerometro();
	Init_ThLEDS();
	Init_ThCOM();
	
	estado = 1; //inicializo en modo reposo
	
	posicionHora = 0;
	
  while (1) {
    // Insert thread code here...
		
		switch(estado){
			case 1: //REPOSO
				colaLCD.horas = horasClk;
				colaLCD.minutos = minutosClk;
				colaLCD.segundos = segundosClk;
				colaLCD.modo = 1;
				colaLCD.tipoDepuracion = 0;
				configurarAceleracion = 0;
			
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
				
				//comprobamos pulsaciones
				osMessageQueueGet(mid_MsgQueueJoystick, &colaJoystick, NULL, 0);
				if(colaJoystick.boton == 5 && colaJoystick.tipoPulsacion == 1){
					estado = 2;
					colaJoystick.tipoPulsacion = 0;
				}
			
				break;
			
			case 2: //ACTIVO
				colaLCD.modo = 2;
			
				//cogemos los datos del MPU y se los damos al LCD
				osMessageQueueGet(mid_MsgQueueMPU, &colaMPU, NULL, 0);
				colaLCD.temperatura = colaMPU.temperatura;
				colaLCD.aceleracionX = colaMPU.aceleracionX;
				colaLCD.aceleracionY = colaMPU.aceleracionY;
				colaLCD.aceleracionZ = colaMPU.aceleracionZ;
			
				if(colaMPU.aceleracionX > refAx){
					osThreadFlagsSet(tid_ThLEDS, LED1_ON);
				}else{
					osThreadFlagsSet(tid_ThLEDS, LED1_OFF);
				}
				
				if(colaMPU.aceleracionY > refAy){
					osThreadFlagsSet(tid_ThLEDS, LED2_ON);
				}else{
					osThreadFlagsSet(tid_ThLEDS, LED2_OFF);
				}
				
				if(colaMPU.aceleracionZ > refAz){
					osThreadFlagsSet(tid_ThLEDS, LED3_ON);
				}else{
					osThreadFlagsSet(tid_ThLEDS, LED3_OFF);
				}
			
				//guardar medidas en el buffer
				sprintf(bufferMedidas[posicionBuffer], "%02d:%02d:%02d--Tm:%0.1f-Ax:%.1f-Ay:%.1f-Az:%.1f", horasClk, minutosClk, segundosClk, colaMPU.temperatura,colaMPU.aceleracionX, colaMPU.aceleracionY, colaMPU.aceleracionZ);
				posicionBuffer = (posicionBuffer == 9) ? 0 : (posicionBuffer + 1);
				
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
			
				//comprobamos pulsaciones
				osMessageQueueGet(mid_MsgQueueJoystick, &colaJoystick, NULL, 0);
				if(colaJoystick.boton == 5 && colaJoystick.tipoPulsacion == 1){
					estado = 3;
					colaLCD.tipoDepuracion = 0;
					
					horasDepuracion = horasClk;
					minutosDepuracion = minutosClk;
					segundosDepuracion = segundosClk;
					
					osThreadFlagsSet(tid_ThLEDS, LED1_OFF);
					osThreadFlagsSet(tid_ThLEDS, LED2_OFF);
					osThreadFlagsSet(tid_ThLEDS, LED3_OFF);
					
					colaJoystick.tipoPulsacion = 0;
					colaJoystick.boton = 0;
				}
				break;
			
			case 3: //PROGRAMACIÓN Y DEPURACIÓN
				colaLCD.modo = 3;
			
				colaLCD.tipoDepuracion = 0;
			
				colaLCD.horas = horasDepuracion;
				colaLCD.minutos = minutosDepuracion;
				colaLCD.segundos = segundosDepuracion;
			
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
					
				//comprobamos pulsaciones
				osMessageQueueGet(mid_MsgQueueJoystick, &colaJoystick, NULL, 0);
				if(colaJoystick.boton == 5 && colaJoystick.tipoPulsacion == 1){
					estado = 1;
					colaJoystick.tipoPulsacion = 0;
					colaJoystick.boton = 0;
					colaLCD.tipoDepuracion = 0;
				}else{
//					colaLCD.tipoDepuracion = 0;
//					osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);					
					gestionarPulsaciones();
					colaJoystick.boton = 0;
					colaJoystick.tipoPulsacion = 3;
				}
				
				//modo programación
				osMessageQueueGet(mid_MsgQueueRECIBIDO, &colaCOMPCrecibido, NULL, 100);
				
				switch(colaCOMPCrecibido.CMD){
					case HORA:
						sscanf(colaCOMPCrecibido.Payload, "%d:%d:%d", &horasClk, &minutosClk, &segundosClk);
					
						horasDepuracion = horasClk;
						minutosDepuracion = minutosClk;
						segundosDepuracion = segundosClk;
					
						osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
					
						osMessageQueuePut(mid_MsgQueueTRANSMITIR, &colaCOMPCrecibido, NULL, NULL);
						colaCOMPCrecibido.CMD = 0x00; //borro el comando recibido para que no entre en un bucle infinito
						break;
					
					case Ax:
						sscanf( colaCOMPCrecibido.Payload, "%f", &refAx);
						refAxDepuracion = refAx;
						colaLCD.refAx = refAxDepuracion;
						osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
						osMessageQueuePut(mid_MsgQueueTRANSMITIR, &colaCOMPCrecibido, NULL, NULL);
						colaCOMPCrecibido.CMD = 0x00; //borro el comando recibido para que no entre en un bucle infinito
						break;
					
					case Ay:
						sscanf( colaCOMPCrecibido.Payload, "%f", &refAy);
						refAyDepuracion = refAy;
						colaLCD.refAy = refAyDepuracion;
						osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
						osMessageQueuePut(mid_MsgQueueTRANSMITIR, &colaCOMPCrecibido, NULL, NULL);
						colaCOMPCrecibido.CMD = 0x00; //borro el comando recibido para que no entre en un bucle infinito
						break;
					
					case Az:
						sscanf( colaCOMPCrecibido.Payload, "%f", &refAz);
						refAzDepuracion = refAz;
						colaLCD.refAz = refAzDepuracion;
						osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
						osMessageQueuePut(mid_MsgQueueTRANSMITIR, &colaCOMPCrecibido, NULL, NULL);
						colaCOMPCrecibido.CMD = 0x00; //borro el comando recibido para que no entre en un bucle infinito
						break;
					
					case SolicitarMedidas:
						for( int i = 0; i < 10; i++){
							if(bufferMedidas[i] != NULL){
								snprintf(colaCOMPCrecibido.Payload, sizeof(colaCOMPCrecibido.Payload), "%s", bufferMedidas[i]);
								colaCOMPCrecibido.LEN = 0x2B;
								osMessageQueuePut(mid_MsgQueueTRANSMITIR, &colaCOMPCrecibido, NULL, NULL);
							}
						}
						colaCOMPCrecibido.CMD = 0x00; //borro el comando recibido para que no entre en un bucle infinito
						break;
					
					case BorrarMedidas:
						for(int i = 0; i<10; i++){
							memset(bufferMedidas[i], 0x00, sizeof(bufferMedidas[i]));
						}
						osMessageQueuePut(mid_MsgQueueTRANSMITIR, &colaCOMPCrecibido, NULL, NULL);
						colaCOMPCrecibido.CMD = 0x00; //borro el comando recibido para que no entre en un bucle infinito
						break;
				
				}
				break;
		}
			
    osThreadYield();                            // suspend thread
  }
}

static void gestionarPulsaciones(void){
	
	if(configurarAceleracion == 0){
		if(colaJoystick.boton == 1 && colaJoystick.tipoPulsacion == 0){ //arriba
			if(posicionHora == 0){
				if(horasDepuracion/10 < 2){
					if(horasDepuracion%10 > 3){
						if(horasDepuracion/10<2){
						horasDepuracion = horasDepuracion+10;
					}
					}else{
						horasDepuracion = horasDepuracion+10;
					}
					//horasDepuracion = horasDepuracion+10;
				}
				
			}else if(posicionHora == 1){
				if(horasDepuracion%10 < 9 && horasDepuracion/10<2){
					horasDepuracion = (horasDepuracion/10)*10 + horasDepuracion%10 +1;
				}else if (horasDepuracion/10 == 2){
					if(horasDepuracion%10<3){
						horasDepuracion = (horasDepuracion/10)*10 + horasDepuracion%10 +1;
					}
				}
				colaLCD.horas = horasDepuracion;
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
				
			}else if(posicionHora == 2){
				if(minutosDepuracion/10 < 5){
					minutosDepuracion = minutosDepuracion+10;
				}
				colaLCD.minutos = minutosDepuracion;
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
				
			}else if(posicionHora == 3){
				if(minutosDepuracion%10 < 9){
					minutosDepuracion = (minutosDepuracion/10)*10 + minutosDepuracion%10 +1;
				}
				colaLCD.minutos = minutosDepuracion;
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
				
			}else if(posicionHora == 4){
				if(segundosDepuracion/10 < 5){
					segundosDepuracion = segundosDepuracion+10;
				}
				colaLCD.segundos = segundosDepuracion;
				osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
				
			}else if(posicionHora == 5){
				if(segundosDepuracion%10 < 9){
					segundosDepuracion = (segundosDepuracion/10)*10 + segundosDepuracion%10 +1;
				}	
			}
			
		}else if(colaJoystick.boton == 2 && colaJoystick.tipoPulsacion == 0){		//derecha
				if(posicionHora < 5){
					posicionHora++;
				}
				
				
		}else if(colaJoystick.boton == 3 && colaJoystick.tipoPulsacion == 0){ //abajo
			if(posicionHora == 0){
				if(horasDepuracion/10 > 0){
					horasDepuracion = horasDepuracion - 10;
				}
				
			}else if(posicionHora == 1){
				if(horasDepuracion%10 > 0){
					horasDepuracion = (horasDepuracion/10)*10 + horasDepuracion%10 - 1;
				}
				
			}else if(posicionHora == 2){
				if(minutosDepuracion/10 > 0){
					minutosDepuracion = minutosDepuracion - 10;
				}
				
			}else if(posicionHora == 3){
				if(minutosDepuracion%10 > 0){
					minutosDepuracion = (minutosDepuracion/10)*10 + minutosDepuracion%10 - 1;
				}
				
			}else if(posicionHora == 4){
				if(segundosDepuracion/10 > 0){
					segundosDepuracion = segundosDepuracion - 10;
				}
				
			}else if(posicionHora == 5){
				if(segundosDepuracion%10 > 0){
					segundosDepuracion = (segundosDepuracion/10)*10 + segundosDepuracion%10 - 1;
				}	
			}
			
		}else if(colaJoystick.boton == 4 && colaJoystick.tipoPulsacion == 0){ //izquierda
			if(posicionHora < 6 && posicionHora > 0){
				posicionHora--;
			}
			
		}else if(colaJoystick.boton == 5 && colaJoystick.tipoPulsacion == 0){		//centro
			horasClk = horasDepuracion;
			minutosClk = minutosDepuracion;
			segundosClk = segundosDepuracion;
			
			configurarAceleracion = 1;
			
			refAxDepuracion = refAx;
			refAyDepuracion = refAy;
			refAzDepuracion = refAz;
		}
	}
	else if(configurarAceleracion == 1){
		
		colaLCD.tipoDepuracion = 1;
		osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
		
		//posicionAceleracion = 0;
		configurarReferenciasAceleracion();
		
		colaJoystick.boton = 0;
		colaJoystick.tipoPulsacion = 3;
	}
	colaJoystick.boton = 0;
	colaJoystick.tipoPulsacion = 3;
}


static void configurarReferenciasAceleracion(void){
	//posicionHora = 0;
	if(colaJoystick.boton == 1 && colaJoystick.tipoPulsacion == 0){ // arriba
		if(posicionAceleracion == 0){
			refAxDepuracion = refAxDepuracion + 0.1f;
			colaLCD.refAx = refAxDepuracion;
		}else if(posicionAceleracion == 1){
			refAyDepuracion = refAyDepuracion + 0.1f;
			colaLCD.refAy = refAyDepuracion;
		}else if(posicionAceleracion == 2){
			refAzDepuracion = refAzDepuracion + 0.1f;
			colaLCD.refAz = refAzDepuracion;
		}
		osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
		
	}else if(colaJoystick.boton == 2 && colaJoystick.tipoPulsacion == 0){ //derecha
		if(posicionAceleracion < 2){
			posicionAceleracion++;
		}
	}else if(colaJoystick.boton == 3 && colaJoystick.tipoPulsacion == 0){ //abajo
		if(posicionAceleracion == 0){
			refAxDepuracion = refAxDepuracion - 0.1f;
			colaLCD.refAx = refAxDepuracion;
		}else if(posicionAceleracion == 1){
			refAyDepuracion = refAyDepuracion - 0.1f;
			colaLCD.refAy = refAyDepuracion;
		}else if(posicionAceleracion == 2){
			refAzDepuracion = refAzDepuracion - 0.1f;
			colaLCD.refAz = refAzDepuracion;
		}
		osMessageQueuePut(mid_MsgQueueLCD, &colaLCD, NULL, NULL);
	}else if(colaJoystick.boton == 4 && colaJoystick.tipoPulsacion == 0){ //izquierda
		if(posicionAceleracion > 0){
			posicionAceleracion--;
		}
	}else if(colaJoystick.boton == 5 && colaJoystick.tipoPulsacion == 0){ //izquierda
			refAx = refAxDepuracion;
			refAy = refAyDepuracion;
			refAz = refAzDepuracion;
			
			configurarAceleracion = 0;
	}
	
	colaJoystick.boton = 0;
	colaJoystick.tipoPulsacion = 3;
}
