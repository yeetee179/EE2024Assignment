#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "Led7seg.h"
#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "light.h"
#include "stdio.h"
#include "string.h"
#include "lpc17xx_uart.h"

#define RGB_CLOCK 333

volatile uint32_t msTick=0; //contains the system time in millisecond
volatile uint32_t previousTimeSevenSeg=0;
volatile uint32_t SevenSegValue = 0;
volatile uint32_t previousTimeOledDisplay = 0;
volatile uint32_t previousTimeToggle = 0;
//////////////////////////////////////////////////////////////////////////////////////////
uint8_t oledLight[40]={};
uint8_t oledTemp[40]={};
uint8_t oledAccX[40]={};
uint8_t oledAccY[40]={};
uint8_t oledAccZ[40]={};
uint8_t oledMonitor[40]={};
uint8_t TEMP_HIGH_WARNING = 45;
uint8_t LIGHT_LOW_WARNING = 50;
volatile uint32_t lightReading = 30;
volatile uint32_t tempReading = 0;
volatile uint32_t xReading = 0;
volatile uint32_t yReading = 0;
volatile uint32_t zReading = 0;
volatile uint32_t xInitialReading = 0;
volatile uint32_t yInitialReading = 0; //contains the system time in millisecond
volatile uint32_t zInitialReading = 0;
volatile int runningState = 0; //0 is stable, 1 is running

uint8_t uartDataTransmit[60] = {}; //data thats sent to CEMS
int transmissionCounter = 0 ;//counter for the no. of transmissions

int RESET_RGB = 0;
int PRESENT_TIME = 0;
int START_RGB_TIME = 0;
int COUNT_RGB = 0;
int red_led = 0;
int blue_led = 0;
////////////////////////////////////////////////////////////////////////////////////////////////
// on every timer interrupt, msTick increases
void SysTick_Handler(void){
	msTick++;
}
//////////////////////////////////////////////////////////////////////////////////////////////
uint32_t getTicks(void){
    return msTick;
}
static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPIconnect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);
}
//////////////////////////////////////////////////////////////////////////////////////////////
static void init_i2c(void)
{
	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void init_GPIO(void){
	PINSEL_CFG_Type PC;

	PC.Funcnum = 0;
	PC.OpenDrain = 0;
	PC.Pinmode = 0;
	PC.Portnum = 1;
	PC.Pinnum = 31;
	PINSEL_ConfigPin(&PC);
	GPIO_SetDir(1,1<<31,0);//SW4 button is the toggle button

	PC.Portnum = 2;
	PC.Pinnum = 0;
	PINSEL_ConfigPin(&PC);
	GPIO_SetDir(2,1,1);//red led is set as output

//	PC.Portnum = 2;
//	PC.Pinnum = 1;
//	PINSEL_ConfigPin(&PC);
//	GPIO_SetDir(2,1<<1,1);//green led is set as output

	PC.Portnum = 0;
	PC.Pinnum = 26;
	PINSEL_ConfigPin(&PC);
	GPIO_SetDir(0,1<<26,1);//red led is set as output
}
void SevenSeg(void){
	if ( (msTick - previousTimeSevenSeg) >= 1000){
		SevenSegValue++;
		if (SevenSegValue>15){
			SevenSegValue = 0;
		}
	    previousTimeSevenSeg = msTick;
	}
	if(SevenSegValue == 0){led7seg_setChar('0',FALSE);}
	else if(SevenSegValue == 1){led7seg_setChar('1',FALSE);}
	else if(SevenSegValue == 2){led7seg_setChar('2',FALSE);}
	else if(SevenSegValue == 3){led7seg_setChar('3',FALSE);}
	else if(SevenSegValue == 4){led7seg_setChar('4',FALSE);}
	else if(SevenSegValue == 5){led7seg_setChar('5',FALSE);}
	else if(SevenSegValue == 6){led7seg_setChar('6',FALSE);}
	else if(SevenSegValue == 7){led7seg_setChar('7',FALSE);}
	else if(SevenSegValue == 8){led7seg_setChar('8',FALSE);}
	else if(SevenSegValue == 9){led7seg_setChar('9',FALSE);}
	else if(SevenSegValue == 10){led7seg_setChar('A',FALSE);}
	else if(SevenSegValue == 11){led7seg_setChar('B',FALSE);} //B
	else if(SevenSegValue == 12){led7seg_setChar('C',FALSE);}
	else if(SevenSegValue == 13){led7seg_setChar('D',FALSE);} //D
	else if(SevenSegValue == 14){led7seg_setChar('E',FALSE);}
	else if(SevenSegValue == 15){led7seg_setChar('F',FALSE);}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void OledDisplay(uint32_t lightReading,double tempReading,int8_t xReading,int8_t yReading,int8_t zReading){
	sprintf(oledLight,"LUX: %u      ",lightReading);
	sprintf(oledTemp,"TEMP: %2.2f    ",tempReading);
	sprintf(oledAccX, "ACC_X: %d      ",xReading);
	sprintf(oledAccY, "ACC_Y: %d      ",yReading);
	sprintf(oledAccZ, "ACC_Z: %d      ",zReading);
	sprintf(oledMonitor, "MONITOR");
	oled_putString(0,50,(uint8_t *)oledMonitor,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    if (   (SevenSegValue == 5  && SevenSegValue != 6)
    	|| (SevenSegValue == 10 && SevenSegValue != 11)
    	|| (SevenSegValue == 15 && SevenSegValue != 0)){
    	oled_putString(0,0,(uint8_t *)oledLight,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,10,(uint8_t *)oledTemp,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,20,(uint8_t *)oledAccX,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,30,(uint8_t *)oledAccY,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,40,(uint8_t *)oledAccZ,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    }
}
void EINT3_IRQHandler (void)
{
//	if((LPC_GPIOINT -> IO0IntStatR >> 2) & 0x1){ //temperature sensor
//		LPC_GPIOINT -> IO0IntClr = 1<<2;
//		temp_counter++;
//	}

	if((LPC_GPIOINT -> IO2IntStatF >> 5) & 0x1) { //Light Sensor

		LPC_GPIOINT -> IO2IntClr = 1<<5;
	}
}
void rgb_setClear() {
	GPIO_ClearValue(0, 1<<26);
	GPIO_ClearValue(2,1);
}
void rgb_warning(uint32_t light_val, uint32_t temp_val) {
	COUNT_RGB++;
	if(light_val < LIGHT_LOW_WARNING) {
		blue_led = 1;
	} else {
		blue_led = 0;
	}
	if(temp_val > TEMP_HIGH_WARNING){
		red_led = 1;
	} else {
		red_led = 0;
	}
	if(COUNT_RGB % 2 == 0) {
		if(blue_led == 1) {
			GPIO_SetValue(0, 1<<26);
		}
		if(red_led == 1) {
			GPIO_SetValue(2,1);
		}
	} else {
		rgb_setClear();
	}
}
void rgb_blinker() {
	PRESENT_TIME = getTicks();
	if(RESET_RGB == 1) {
		RESET_RGB = 0;
		START_RGB_TIME = PRESENT_TIME;
	}
	else if(PRESENT_TIME - START_RGB_TIME >= RGB_CLOCK) {
		rgb_warning(lightReading, tempReading);
		RESET_RGB = 1;
	}
}
void pinsel_uart3(void){
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);
}

void init_uart(void){
	UART_CFG_Type uartCfg;
	uartCfg.Baud_rate = 115200;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;
	//pin select for uart3;
	pinsel_uart3();
	//supply power & setup working parameters for uart3
	UART_Init(LPC_UART3, &uartCfg);
	//enable transmit for uart3
	UART_TxCmd(LPC_UART3, ENABLE);
}
void UartDataSendAtF (void){
	if (SevenSegValue == 15){
		sprintf(uartData,"%03d_-_T%2.2f_L%u_AX%d_AY%d_AZ%d \r\n",
				transmissionCounter,tempReading,lightReading,xReading,yReading,zReading);
		UART_Send(LPC_UART3,(uint8_t *)uartData, strlen(uart_transmit),BLOCKING);
		transtransmissionCounter_no++;
	}
}
void Toggle(void){
	if (((GPIO_ReadValue(1) >> 31) == 0) && msTick - previousTimeToggle >=500){
		previousTimeToggle = msTick;
		if(runningState == 0)
		runningState = 1;
		else if (runningState == 1)
        runningState = 0;
	}
}
void StableState(void){
	led7seg_setChar(' ',FALSE);
	oled_clearScreen(OLED_COLOR_BLACK);
    GPIO_ClearValue( 2, 0 );
    GPIO_ClearValue( 2, 1 );
    GPIO_ClearValue( 0, (1<<26) );
}
void MonitorState(void){
    static int i;
	tempReading = temp_read()/10;
	while (i == 0){
		acc_read ( &xInitialReading, &yInitialReading, &zInitialReading);
		i++;
	}
	acc_read ( &xReading, &yReading, &zReading);
	xReading = xReading - xInitialReading;
	yReading = yReading - yInitialReading;
	zReading = zReading - zInitialReading;
	lightReading = light_read();
	SevenSeg();
	OledDisplay(lightReading, tempReading, xReading,yReading,zReading);
	rgb_blinker();
	UartDataSendAtF();
}

int main(void){
	SysTick_Config(SystemCoreClock / 1000 ); //generates a 1 millisec clock
	init_ssp();
	init_i2c();
	init_GPIO();
	led7seg_init();
	oled_init();
	temp_init(&getTicks);
	rgb_init();
	acc_init();
	light_enable();

	while(1){
		Toggle();

		if (runningState == 0){
			StableState();
		}
		else if (runningState == 1){
			MonitorState();
		}
//		printf("%d\n",runningState);

	}
}


void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}


