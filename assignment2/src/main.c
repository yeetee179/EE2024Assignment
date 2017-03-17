/*****************************************************************************
 *   A demo example using several of the peripherals on the base board
 *
 *   Copyright(C) 2011, EE2024
 *   All rights reserved.
 *
 ******************************************************************************/
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
volatile uint32_t previousTime=0;
volatile uint32_t SevenSegValue = 0;
////////////////////////////////////////////////////////////////////////////////////////////////
// on every timer interrupt, msTick increases
void SysTick_Handler(void){
	msTick++;
}
//////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////
//causes the led to loop from 0 to F every second
void SevenSeg(void){
	if ( (msTick - previousTime) >= 1000){
		SevenSegValue++;
		if (SevenSegValue>15){
			SevenSegValue = 0;
		}
	    previousTime = msTick;
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
int main(void){
	SysTick_Config(SystemCoreClock / 1000 ); //generates a 1 millisec clock
	init_ssp();
	led7seg_init();
	init_i2c();

	while(1){
		//printf("Time taken (C version): %ld milliseconds\n",(msTick));
		//7 seg
		SevenSeg();
	}
}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
