#include "com.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 

osThreadId_t tid_ThCOMRx;                        // thread id
osThreadId_t tid_ThCOMTx;
 
void COMRx (void *argument);                   // thread function
void COMTx (void *argument); 

//CONSTANTES	
//static const char SOH = 0x01; //Inicio de una trama
//static const char EOT = 0xFE; //Final de una trama

//static const char HORA = 0x20;
//static const char Ax = 0x25;
//static const char Ay = 0x26;
//static const char Az = 0x27;
//static const char SolicitarMedidas = 0x55;
//static const char BorrarMedidas = 0x60;
//static const char RespuestaSolicitarMedidas = 0xAF;
//****************************************************************************

//USART
extern ARM_DRIVER_USART Driver_USART3;/* USART Driver */
static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;
void myUSART_callback(uint32_t event);
static int8_t USART_Init(void);
static char datoTrama;
static char datosRecibidos[13];
static void analizarDatoTrama(void);
static void recogerDatos(void);
static uint8_t posicionTrama = 0;
static char tramaDevolver[40];
static void montarTrama(void);
//****************************************************************************

//COLAS PARA LA SINCRONIZACIÓN
osMessageQueueId_t mid_MsgQueueRECIBIDO;
osMessageQueueId_t mid_MsgQueueTRANSMITIR;
MSGQUEUE_OBJ_t_COM enviar;
MSGQUEUE_OBJ_t_COM recibido;
//****************************************************************************

//AUTOMATA
static uint8_t estado = 0;// 0 initState, 1 defaultState, 
//****************************************************************************

//TEST
osThreadId_t tid_ThTest;
void Test (void *argument);
//****************************************************************************
int Init_ThCOM (void) {
 
	USART_Init();
	
  tid_ThCOMRx = osThreadNew(COMRx, NULL, NULL);
  if (tid_ThCOMRx == NULL) {
    return(-1);
  }
	
	tid_ThCOMTx = osThreadNew(COMTx, NULL, NULL);
  if (tid_ThCOMTx == NULL) {
    return(-1);
  }
 
	mid_MsgQueueRECIBIDO = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_t_COM), NULL);
  if (mid_MsgQueueRECIBIDO == NULL) {
    ; // Message Queue object not created, handle failure
  }
	
	mid_MsgQueueTRANSMITIR = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_t_COM), NULL);
  if (mid_MsgQueueTRANSMITIR == NULL) {
    ; // Message Queue object not created, handle failure
  }
	
  return(0);
}
 
void COMRx (void *argument) {
 
  while (1) {
    // Insert thread code here...
		USARTdrv->Receive(&datoTrama, 1);
		osThreadFlagsWait(2, osFlagsWaitAll, osWaitForever);
		
		if(estado == 0){
			if(datoTrama == SOH){
				estado = 1;
				posicionTrama = 0;
				//memset(datosRecibidos, 0x00, sizeof(datosRecibidos));
			}
		}else if(estado == 1){
			if(datoTrama == EOT){
				estado = 0;
				if(datosRecibidos[1] == (posicionTrama + 2)){
					if((datosRecibidos[0] == HORA | datosRecibidos[0] == Ax | datosRecibidos[0] == Ay | datosRecibidos[0] == Az | datosRecibidos[0] == SolicitarMedidas | datosRecibidos[0] == BorrarMedidas )){
						enviar.CMD = datosRecibidos[0];
						enviar.LEN = datosRecibidos[1];
						for(int i = 2; i<posicionTrama; i++){
							enviar.Payload[i-2] = datosRecibidos[i];
						}
						osMessageQueuePut(mid_MsgQueueRECIBIDO, &enviar, NULL, NULL);
					}
				}else{
					estado = 0;
				}
			}else{
				datosRecibidos[posicionTrama] = datoTrama;
				posicionTrama++;
			}
		}
		
//		USARTdrv->Receive(&datoTrama, 1);
//		osThreadFlagsWait(2, osFlagsWaitAll, osWaitForever);
//		analizarDatoTrama();
		
    osThreadYield();                            // suspend thread
  }
}
void COMTx (void *argument) {

  while (1) {
    // Insert thread code here...
		osMessageQueueGet(mid_MsgQueueTRANSMITIR, &recibido, NULL, osWaitForever);
		montarTrama();
		USARTdrv->Send(tramaDevolver, recibido.LEN);
		osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		memset(tramaDevolver, 0x00, sizeof(tramaDevolver));
		
    osThreadYield();                            // suspend thread
  }
}

//INICIALIZACIÓN USART
static int8_t USART_Init(void){
	/*Initialize the USART driver */
	if(USARTdrv->Initialize(myUSART_callback) != ARM_DRIVER_OK |
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK |
    /*Configure the USART to 4800 Bits/sec */
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 
											115200) != ARM_DRIVER_OK |
     
    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1) != ARM_DRIVER_OK |
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1) != ARM_DRIVER_OK){
			return(-1);
		}
		return 0;
}
//*******************************************************************************************

//CALLBACK
void myUSART_callback(uint32_t event)
{
  
	switch (event){
		case ARM_USART_EVENT_SEND_COMPLETE:
			osThreadFlagsSet(tid_ThCOMTx, 1);
			break;
		
		case ARM_USART_EVENT_RECEIVE_COMPLETE:
			osThreadFlagsSet(tid_ThCOMRx, 2);
			break;	
			
	}
}
//********************************************************************************************

//ANALIZAR DATO TRAMA
//static void analizarDatoTrama(void){
//	if(datoTrama == SOH){
//		datosRecibidos[posicionTrama] = datoTrama;
//		recogerDatos();
//	}
//}
//********************************************************************************************

//RECOGER DATOS
//static void recogerDatos(void){
//	uint8_t i = 0; //posicion del payload dentro de lo que envío
//	uint8_t j = 3; //el payload siempre empieza en la posición 3: 0inicio, 1comando, 2longitud, 3iniciopayload
//	
//	while(datoTrama != EOT || posicionTrama > 11){
//		posicionTrama++;
//		USARTdrv->Receive(&datoTrama, 1);
//		osThreadFlagsWait(2, osFlagsWaitAll, osWaitForever);
//		datosRecibidos[posicionTrama] = datoTrama;
//	}
//	
//	if(posicionTrama < 11 | posicionTrama > 2){
//		datosRecibidos[posicionTrama+1] = EOT;
//		
//		enviar.CMD = datosRecibidos[1];
//		enviar.LEN = datosRecibidos[2];
//		
//		while(datosRecibidos[j] != EOT){
//			enviar.Payload[i] = datosRecibidos[j];
//			i++;
//			j++;
//		}
//		
//		if(enviar.LEN == posicionTrama+1 && (enviar.CMD == HORA | enviar.CMD == Ax | enviar.CMD == Ay | enviar.CMD == Az | enviar.CMD == SolicitarMedidas | enviar.CMD == BorrarMedidas )){
//			osMessageQueuePut(mid_MsgQueueRECIBIDO, &enviar, NULL, NULL);
//		}
//		
//	}
//	
//	posicionTrama = 0;
//	
//}
//********************************************************************************************

//MONTAR TRAMA
static void montarTrama(void){
	uint8_t size = recibido.LEN;
	uint8_t payloadSize = recibido.LEN-4;
	uint8_t j = 3;
	uint8_t i = 0;
	
	tramaDevolver[0] = SOH;
	if(recibido.CMD == SolicitarMedidas){
		tramaDevolver[1] = RespuestaSolicitarMedidas;
	}else{
		tramaDevolver[1] = ~recibido.CMD; //complemento a 1 del comando recibido
	}
	tramaDevolver[2] = recibido.LEN;
	while(i<payloadSize){
		tramaDevolver[j] = recibido.Payload[i];
		j++;
		i++;
	}
	
	tramaDevolver[size-1] = EOT;
}
//********************************************************************************************

//TEST
//TEST
//int Init_Test(void) {
// 
//  tid_ThTest = osThreadNew(Test, NULL, NULL);
//  if (tid_ThTest == NULL) {
//    return(-1);
//  }	
//  return(0);
//}
// 
//void Test (void *argument) {
//	MSGQUEUE_OBJ_t_COM tramaPrueba;
//	tramaPrueba.CMD = HORA;
//	tramaPrueba.LEN = 0x0C;
//	tramaPrueba.Payload[0] = 0x31;
//	tramaPrueba.Payload[1] = 0x35;
//	tramaPrueba.Payload[2] = 0x3A;
//	tramaPrueba.Payload[3] = 0x32;
//	tramaPrueba.Payload[4] = 0x30;
//	tramaPrueba.Payload[5] = 0x3A;
//	tramaPrueba.Payload[6] = 0x31;
//	tramaPrueba.Payload[7] = 0x35;
//	osDelay(1000);
//	osMessageQueuePut(mid_MsgQueueTRANSMITIR, &tramaPrueba, NULL, NULL);
//	
//	
//  while (1) {
//		osThreadYield();
//  }
//}
//********************************************************************************************