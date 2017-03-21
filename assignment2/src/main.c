
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

volatile uint32_t msTick=0; //contains the system time in millisecond
volatile uint32_t previousTimeSevenSeg=0;
volatile uint32_t SevenSegValue = 0;
volatile uint32_t previousTimeOledDisplay = 0;

//////////////////////////////////////////////////////////////////////////////////////////
uint8_t oledLight[40]={};
uint8_t oledTemp[40]={};
uint8_t oledAccX[40]={};
uint8_t oledAccY[40]={};
uint8_t oledAccZ[40]={};
uint8_t oledMonitor[40]={};
volatile uint32_t lightReading = 30;
volatile uint32_t tempReading = 0;
volatile uint32_t xReading = 0;
volatile uint32_t yReading = 0;
volatile uint32_t zReading = 0;
volatile uint32_t xInitialReading = 0;
volatile uint32_t yInitialReading = 0; //contains the system time in millisecond
volatile uint32_t zInitialReading = 0;
volatile int runningState = 0; //initially off at stable state
volatile int mode = 0;
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
	PC.Portnum = 1;		//SW4 button is the toggle button
	PC.Pinnum = 31;
	PINSEL_ConfigPin(&PC);
	GPIO_SetDir(1,1<<31,0);
}
//void EINT3_IRQHandler (void)
//{
//	if((LPC_GPIOINT -> IO0IntStatF >> 31) & 0x1){ //btnsw4(TOGGLE MODE SWITCH) is pressed
//		LPC_GPIOINT -> IO0IntClr = 1<<31;
//	}
//}
void Toggle (void){
	int btn = (GPIO_ReadValue(1) >> 31) & 0x01;//SW4 toggle function
    if (btn == 0 && runningState == 0){
    	runningState = 1;
    }
    else if (btn == 0 && runningState == 1){
    	runningState = 0;
    }
    btn = 1;
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
    if (   (SevenSegValue == 5 && SevenSegValue != 6)
    	|| (SevenSegValue == 10 && SevenSegValue != 11)
    	|| (SevenSegValue == 15 && SevenSegValue != 0)){
    	oled_putString(0,0,(uint8_t *)oledLight,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,10,(uint8_t *)oledTemp,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,20,(uint8_t *)oledAccX,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,30,(uint8_t *)oledAccY,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,40,(uint8_t *)oledAccZ,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    	oled_putString(0,50,(uint8_t *)oledMonitor,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
    }
}

int main(void){

	SysTick_Config(SystemCoreClock / 1000 ); //generates a 1 millisec clock
	init_ssp();
	init_i2c();
	init_GPIO();
	led7seg_init();
	oled_init();
	temp_init(&getTicks);
	acc_init();
	acc_read ( &xInitialReading, &yInitialReading, &zInitialReading);

	while(1){
		//printf("Time taken (C version): %ld milliseconds\n",(msTick));
		Toggle();
		if (runningState == 1){
			SevenSeg();
			tempReading = temp_read()/10;
			acc_read ( &xReading, &yReading, &zReading);
			xReading = xReading - xInitialReading;
			yReading = yReading - yInitialReading;
			zReading = zReading - zInitialReading;

			OledDisplay(lightReading, tempReading, xReading,yReading,zReading);
		}
		if (runningState == 0){

		}
	}
}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
