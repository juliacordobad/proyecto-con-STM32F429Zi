#include "LCD.h"
#include "string.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"

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
  * @brief  Configura los par�metros del timer 7
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
    SPIdrv->Initialize(NULL);
    /* Power up the SPI peripheral */
    SPIdrv->PowerControl(ARM_POWER_FULL);
    /* Configure the SPI to Master, 8-bit mode @20000 kBits/sec */
    SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);
 
    /* SS line = INACTIVE = HIGH */
    SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
	
	//INICIALIZACI�N DE PINES
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
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci�n a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci�n a 0
 LCD_wr_cmd(0xB0); // P�gina 0

 for(i=0;i<128;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci�n a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci�n a 0
 LCD_wr_cmd(0xB1); // P�gina 1

 for(i=128;i<256;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //P�gina 2
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

//ESCRIBIR EN LA 1a L�NEA DEL LCD
static void symbolToLocalBuffer_L1(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25*(symbol - ' '); //al caracter en ascii que yo le meto como par�metro (en este caso la H es 72) le resta un "espacio"(32)
	
	for(i=0; i<12; i++){
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i+positionL1]= value1; //+posicionL1 ser�a el n�mero de p�xeles a la derecha que se desplazar�a
		buffer[i+128+positionL1]= value2;
	}
	
	positionL1 = positionL1+Arial12x12[offset];
}

//ESCRIBIR EN LA 2a LINEA DEL LCD
static void symbolToLocalBuffer_L2(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25*(symbol - ' '); //al caracter en ascii que yo le meto como par�metro (en este caso la H es 72) le resta un "espacio"(32)
	
	for(i=0; i<12; i++){
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i+positionL2+256]= value1; //+posicionL1 ser�a el n�mero de p�xeles a la derecha que se desplazar�a
		buffer[i+128+positionL2+256]= value2; //sumando 256 salto de l�nea
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

//RESETEAR BUFFER Y POSICI�N
void resetBuffer(void){
	memset(buffer, 0, sizeof(buffer));
	positionL1 = 0;
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


//INICIALIZACI�N DE PINES
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
 // Escribir un dato (data) usando la funci�n SPIDrv->Send(�);
	SPIdrv->Send(&data, sizeof(data));
 // Esperar a que se libere el bus SPI;
	while(SPIdrv->GetStatus().busy){}
 // Seleccionar CS = 1;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);
}

static void LCD_wr_cmd(unsigned char cmd)
{
 // Seleccionar CS = 0;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 0);
 // Seleccionar A0 = 0;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 0);
 // Escribir un comando (cmd) usando la funci�n SPIDrv->Send(�);
	SPIdrv->Send(&cmd, sizeof(cmd));
 // Esperar a que se libere el bus SPI;
	while(SPIdrv->GetStatus().busy){}
 // Seleccionar CS = 1;
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, 1);
}
