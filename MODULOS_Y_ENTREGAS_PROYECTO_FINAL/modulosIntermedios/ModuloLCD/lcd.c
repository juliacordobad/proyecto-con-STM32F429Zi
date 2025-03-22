#include "cmsis_os2.h"                          // CMSIS RTOS header file
 
#include "lcd.h"
#include "string.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"
#include "stdio.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
#define S_TRANS_DONE_SPI  0x01

osThreadId_t tid_ThLCD;                        // thread id
 
void LCD (void *argument);                   // thread function
int Init_ThLCD (void);
void SPI_Callback (uint32_t event);

static char linea1[20];
static char linea2[20];

//COLA
osMessageQueueId_t mid_MsgQueueLCD;  
MSGQUEUE_OBJ_tLCD mensajesCola;
MSGQUEUE_OBJ_tLCD mensajesColaRecibido;
//********************************************************************************************
//TEST
uint8_t modo;
osThreadId_t tid_ThTEST;
void TEST (void *argument);
int Init_ThTEST(void);
//**************************************************************
int Init_ThLCD (void) {
 
	mid_MsgQueueLCD = osMessageQueueNew(MSGQUEUE_OBJECTS, sizeof(MSGQUEUE_OBJ_tLCD), NULL);
  tid_ThLCD = osThreadNew(LCD, NULL, NULL);
  if (tid_ThLCD == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void LCD (void *argument) {
 
	LCD_reset();
	LCD_Init();
	LCD_update();
	
	Init_ThTEST();
	
  while (1) {
     // Insert thread code here...
		osMessageQueueGet(mid_MsgQueueLCD, &mensajesColaRecibido, NULL, osWaitForever);
		resetBuffer();
		
		if(mensajesColaRecibido.modo == 1){
			sprintf(linea1, "SBM 2024");
			for(int i = 0; linea1[i] != '\0'; i++){
				symbolToLocalBuffer(1, linea1[i]);
			}
			sprintf(linea2, "00:00:00");
			for(int i = 0; linea2[i] != '\0'; i++){
				symbolToLocalBuffer(2, linea2[i]);
			}
		}else if (mensajesColaRecibido.modo == 2){
			sprintf(linea1, "ACTIVO--T:21.5");
			for(int i = 0; linea1[i] != '\0'; i++){
				symbolToLocalBuffer(1, linea1[i]);
			}
			sprintf(linea2, "X:n.n Y:n.n Z:n.n");
			for(int i = 0; linea2[i] != '\0'; i++){
				symbolToLocalBuffer(2, linea2[i]);
			}
		}else if(mensajesColaRecibido.modo == 3){
			sprintf(linea1, "---P&D---");
			for(int i = 0; linea1[i] != '\0'; i++){
				symbolToLocalBuffer(1, linea1[i]);
			}
			sprintf(linea2, "H: 00:00:00");
			for(int i = 0; linea2[i] != '\0'; i++){
				symbolToLocalBuffer(2, linea2[i]);
			}
		}
		LCD_update();
	}
		
    osThreadYield();                            // suspend thread
 }

//TEST
int Init_ThTEST(void) {
 
  tid_ThTEST = osThreadNew(TEST, NULL, NULL);
  if (tid_ThTEST == NULL) {
    return(-1);
  }	
  return(0);
}
 
void TEST (void *argument) {
	
  while (1) {
		osDelay(3000);
		mensajesCola.modo = 1;
		osMessageQueuePut(mid_MsgQueueLCD, &mensajesCola, NULL, NULL);
		
		osDelay(3000);
		mensajesCola.modo = 2;
		osMessageQueuePut(mid_MsgQueueLCD, &mensajesCola, NULL, NULL);
		
		osDelay(3000);
		mensajesCola.modo = 3;
		osMessageQueuePut(mid_MsgQueueLCD, &mensajesCola, NULL, NULL);
  }
}
 
void SPI_Callback (uint32_t event)
{
    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        /* Success: Wakeup Thread */
        osThreadFlagsSet(tid_ThLCD, S_TRANS_DONE_SPI); 
        break;
    
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        break;
    
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        break;
    }
}
 
//FUNCIONES LCD
//variables necesarias
extern ARM_DRIVER_SPI Driver_SPI1;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

static TIM_HandleTypeDef htim7;
static unsigned char buffer[511];
	
static	uint16_t positionL1 = 0;
static	uint16_t positionL2 = 0;

// PROTOTIPOS DE FUNCIONES LCD
static void initPINLCDReset(uint16_t GPIO_Pin);
static void initPINLCDcs(uint16_t GPIO_Pin);
static void initPINLCDA0(uint16_t GPIO_Pin);

static void initTIMER7(void);
static void delay(uint32_t n_microsegundos);

static void symbolToLocalBuffer_L1(uint8_t symbol);
static void symbolToLocalBuffer_L2(uint8_t symbol);


static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);

//FUNCIONES DEL LCD

/**
  * @brief  Configura los parámetros del timer 7
  * @param  None
	* 
  * @retval None
  */
void initTIMER7(void){
	__HAL_RCC_TIM7_CLK_ENABLE();
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 83;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 0xFFFF;
  HAL_TIM_Base_Init(&htim7);
}

//RESETEAR LCD
void LCD_reset(void){
	//ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
	
	/* Initialize the SPI driver */
    SPIdrv->Initialize(SPI_Callback);
    /* Power up the SPI peripheral */
    SPIdrv->PowerControl(ARM_POWER_FULL);
    /* Configure the SPI to Master, 8-bit mode @20000 kBits/sec */
    SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);
 
    /* SS line = INACTIVE = HIGH */
    SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
	
	//INICIALIZACIÓN DE PINES
	initPINLCDReset(GPIO_PIN_6);
	initPINLCDcs(GPIO_PIN_14);
	initPINLCDA0(GPIO_PIN_13);
	
	//Salidas puestas a nivel alto para los 3 pines
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 1);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
	delay(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	delay(1000);

}

//INICIALIZAR LCD
void LCD_Init(void){
	LCD_wr_cmd(0xAE);
	LCD_wr_cmd(0xA2);
	LCD_wr_cmd(0xA0);
	LCD_wr_cmd(0xC8);
	LCD_wr_cmd(0x22);
	LCD_wr_cmd(0x2F);
	LCD_wr_cmd(0x40);
	LCD_wr_cmd(0xAF);
	LCD_wr_cmd(0x81);
	LCD_wr_cmd(0x11);
	LCD_wr_cmd(0xA4);
	LCD_wr_cmd(0xA6);
}

//ACTUALIZAR/ESCRIBIR EN EL LCD
void LCD_update(void)
{
 int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB0); // Página 0

 for(i=0;i<128;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB1); // Página 1

 for(i=128;i<256;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //Página 2
 for(i=256;i<384;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3


 for(i=384;i<512;i++){
 LCD_wr_data(buffer[i]);
 }
}

//ESCRIBIR EN LA 1a LÍNEA DEL LCD
static void symbolToLocalBuffer_L1(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25*(symbol - ' '); //al caracter en ascii que yo le meto como parámetro (en este caso la H es 72) le resta un "espacio"(32)
	
	for(i=0; i<12; i++){
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i+positionL1]= value1; //+posicionL1 sería el número de píxeles a la derecha que se desplazaría
		buffer[i+128+positionL1]= value2;
	}
	
	positionL1 = positionL1+Arial12x12[offset];
}

//ESCRIBIR EN LA 2a LINEA DEL LCD
static void symbolToLocalBuffer_L2(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25*(symbol - ' '); //al caracter en ascii que yo le meto como parámetro (en este caso la H es 72) le resta un "espacio"(32)
	
	for(i=0; i<12; i++){
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i+positionL2+256]= value1; //+posicionL1 sería el número de píxeles a la derecha que se desplazaría
		buffer[i+128+positionL2+256]= value2; //sumando 256 salto de línea
	}
	
	positionL2 = positionL2+Arial12x12[offset];
}

//ESCRIBIR EN CUALQUIERA LINEA
void symbolToLocalBuffer(uint8_t line,uint8_t symbol){
	if(line == 1){
		symbolToLocalBuffer_L1(symbol);
	}else if(line == 2){
		symbolToLocalBuffer_L2(symbol);
	}else{}
}

//RESETEAR BUFFER Y POSICIÓN
void resetBuffer(void){
	memset(buffer, 0, sizeof(buffer));
	positionL1 = 0;
	positionL2 = 0;
}
//DELAY
/*static void delay(uint32_t n_microsegundos)
{
	__HAL_RCC_TIM7_CLK_ENABLE();
  
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 83;
  htim7.Init.Period = (n_microsegundos - 1);
  HAL_TIM_Base_Init(&htim7);
  
  HAL_TIM_Base_Start(&htim7);
	
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
	while(__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) == RESET){}
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
		
	HAL_TIM_Base_Stop(&htim7);
	__HAL_TIM_SET_COUNTER(&htim7, 0);
}*/
/**
  * @brief  Configura el delay
	* @param  n_microsegundos -> tiempo establecido para realizar la pausa del delay; su valor se acota entre: 0-65534
  * @retval None
  */
void delay(volatile uint32_t n_microsegundos)
{
	initTIMER7();
	
	//Iniciamos el timer 7
  HAL_TIM_Base_Start(&htim7);
 
	//Esperamos a que se consuma el tiempo que hemos establecido en segundos
	while( __HAL_TIM_GET_COUNTER(&htim7) < n_microsegundos ){
		if (__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)) {
            __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
    } 
  }
	//Paramos el timer 7
	HAL_TIM_Base_Stop(&htim7);
	//Reiniciamos el contador del timer 7
	__HAL_TIM_SET_COUNTER(&htim7, 0);
}


//INICIALIZACIÓN DE PINES
static void initPINLCDReset(uint16_t GPIO_Pin){
	
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		__HAL_RCC_GPIOA_CLK_ENABLE();
    //Configuraci?n de los pines
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = GPIO_Pin;

    //Inicializaci?n de los pines
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void initPINLCDcs(uint16_t GPIO_Pin){
	
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		__HAL_RCC_GPIOD_CLK_ENABLE();
    //Configuraci?n de los pines
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = GPIO_Pin;

    //Inicializaci?n de los pines
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void initPINLCDA0(uint16_t GPIO_Pin){
	
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		__HAL_RCC_GPIOF_CLK_ENABLE();
    //Configuraci?n de los pines
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = GPIO_Pin;

    //Inicializaci?n de los pines
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}
/************************************************************************************************************************************************************/
//extra
static void LCD_wr_data(unsigned char data)
{
 // Seleccionar CS = 0;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);
 // Seleccionar A0 = 1;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);
 // Escribir un dato (data) usando la función SPIDrv->Send(…);
	SPIdrv->Send(&data, sizeof(data));
 // Esperar a que se libere el bus SPI;
	//while(SPIdrv->GetStatus().busy){}
	osThreadFlagsWait(S_TRANS_DONE_SPI, osFlagsWaitAny, 500);
 // Seleccionar CS = 1;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);
}

static void LCD_wr_cmd(unsigned char cmd)
{
 // Seleccionar CS = 0;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);
 // Seleccionar A0 = 0;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 0);
 // Escribir un comando (cmd) usando la función SPIDrv->Send(…);
	SPIdrv->Send(&cmd, sizeof(cmd));
 // Esperar a que se libere el bus SPI;
	//while(SPIdrv->GetStatus().busy){}
	osThreadFlagsWait(S_TRANS_DONE_SPI, osFlagsWaitAny, 500);
 // Seleccionar CS = 1;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);
}

//************************************************************************************************************************************