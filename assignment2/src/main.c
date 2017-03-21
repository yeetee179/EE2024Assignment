///*****************************************************************************
// *   A demo example using several of the peripherals on the base board
// *
// *   Copyright(C) 2011, EE2024
// *   All rights reserved.
// *
// ******************************************************************************/
//#include "lpc17xx_pinsel.h"
//#include "lpc17xx_gpio.h"
//#include "lpc17xx_i2c.h"
//#include "lpc17xx_ssp.h"
//#include "lpc17xx_timer.h"
//#include "Led7seg.h"
//#include "joystick.h"
//#include "pca9532.h"
//#include "acc.h"
//#include "oled.h"
//#include "rgb.h"
//#include "light.h"
//#include "stdio.h"
//#include "string.h"
//#include "lpc17xx_uart.h"
//
//volatile uint32_t msTick=0; //contains the system time in millisecond
//volatile uint32_t previousTime=0;
//volatile uint32_t SevenSegValue = 0;
////////////////////////////////////////////////////////////////////////////////////////////
//uint8_t my_oled_light_label[40]={};
//uint8_t my_oled_temp_label[40]={};
//uint8_t my_oled_acc_x_label[40]={};
//uint8_t my_oled_acc_y_label[40]={};
//uint8_t my_oled_acc_z_label[40]={};
//uint8_t my_oled_light[40]={};
//uint8_t my_oled_temp[40]={};
//uint8_t my_oled_acc_x[40]={};
//uint8_t my_oled_acc_y[40]={};
//uint8_t my_oled_acc_z[40]={};
//volatile uint32_t my_light = 0; //contains the system time in millisecond
//volatile uint32_t my_temp = 0;
//volatile uint32_t x = 0;
//volatile uint32_t y = 0; //contains the system time in millisecond
//volatile uint32_t z = 0;
//
/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//// on every timer interrupt, msTick increases
//void SysTick_Handler(void){
//	msTick++;
//}
////////////////////////////////////////////////////////////////////////////////////////////////
//static void init_ssp(void)
//{
//	SSP_CFG_Type SSP_ConfigStruct;
//	PINSEL_CFG_Type PinCfg;
//
//	/*
//	 * Initialize SPIconnect
//	 * P0.7 - SCK;
//	 * P0.8 - MISO
//	 * P0.9 - MOSI
//	 * P2.2 - SSEL - used as GPIO
//	 */
//	PinCfg.Funcnum = 2;
//	PinCfg.OpenDrain = 0;
//	PinCfg.Pinmode = 0;
//	PinCfg.Portnum = 0;
//	PinCfg.Pinnum = 7;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Pinnum = 8;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Pinnum = 9;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Funcnum = 0;
//	PinCfg.Portnum = 2;
//	PinCfg.Pinnum = 2;
//	PINSEL_ConfigPin(&PinCfg);
//
//	SSP_ConfigStructInit(&SSP_ConfigStruct);
//
//	// Initialize SSP peripheral with parameter given in structure above
//	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
//
//	// Enable SSP peripheral
//	SSP_Cmd(LPC_SSP1, ENABLE);
//}
////////////////////////////////////////////////////////////////////////////////////////////////
//static void init_i2c(void)
//{
//	PINSEL_CFG_Type PinCfg;
//
//	/* Initialize I2C2 pin connect */
//	PinCfg.Funcnum = 2;
//	PinCfg.Pinnum = 10;
//	PinCfg.Portnum = 0;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Pinnum = 11;
//	PINSEL_ConfigPin(&PinCfg);
//
//	// Initialize I2C peripheral
//	I2C_Init(LPC_I2C2, 100000);
//
//	/* Enable I2C1 operation */
//	I2C_Cmd(LPC_I2C2, ENABLE);
//}
///////////////////////////////////////////////////////////////////////////////////////////////
////causes the led to loop from 0 to F every second
//void SevenSeg(void){
//	if ( (msTick - previousTime) >= 1000){
//		SevenSegValue++;
//		if (SevenSegValue>15){
//			SevenSegValue = 0;
//		}
//	    previousTime = msTick;
//	}
//
//	if(SevenSegValue == 0){led7seg_setChar('0',FALSE);}
//	else if(SevenSegValue == 1){led7seg_setChar('1',FALSE);}
//	else if(SevenSegValue == 2){led7seg_setChar('2',FALSE);}
//	else if(SevenSegValue == 3){led7seg_setChar('3',FALSE);}
//	else if(SevenSegValue == 4){led7seg_setChar('4',FALSE);}
//	else if(SevenSegValue == 5){led7seg_setChar('5',FALSE);}
//	else if(SevenSegValue == 6){led7seg_setChar('6',FALSE);}
//	else if(SevenSegValue == 7){led7seg_setChar('7',FALSE);}
//	else if(SevenSegValue == 8){led7seg_setChar('8',FALSE);}
//	else if(SevenSegValue == 9){led7seg_setChar('9',FALSE);}
//	else if(SevenSegValue == 10){led7seg_setChar('A',FALSE);}
//	else if(SevenSegValue == 11){led7seg_setChar('B',FALSE);} //B
//	else if(SevenSegValue == 12){led7seg_setChar('C',FALSE);}
//	else if(SevenSegValue == 13){led7seg_setChar('D',FALSE);} //D
//	else if(SevenSegValue == 14){led7seg_setChar('E',FALSE);}
//	else if(SevenSegValue == 15){led7seg_setChar('F',FALSE);}
//}
////////////////////////////////////////////////////////////////////////////////////////////////
//void oled_template(){
//	sprintf(my_oled_light_label,"LUX:          ");
//	sprintf(my_oled_temp_label,"TEMP:          ");
//	sprintf(my_oled_acc_x_label, "ACC_X:       ");
//	sprintf(my_oled_acc_y_label, "ACC_Y:       ");
//	sprintf(my_oled_acc_z_label, "ACC_Z:       ");
////	oled_putString(10,0.5,(uint8_t *)my_oled_light_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
////	oled_putString(10,10.5,(uint8_t *)my_oled_temp_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
////	oled_putString(10,20.5,(uint8_t *)my_oled_acc_x_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
////	oled_putString(10,30.5,(uint8_t *)my_oled_acc_y_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
////	oled_putString(10,40.5,(uint8_t *)my_oled_acc_z_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(0,0,(uint8_t *)my_oled_light_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(0,10,(uint8_t *)my_oled_temp_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(0,20,(uint8_t *)my_oled_acc_x_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(0,30,(uint8_t *)my_oled_acc_y_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(0,40,(uint8_t *)my_oled_acc_z_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//}
//void oled_display(uint32_t my_light,double my_temp,int8_t x,int8_t y,int8_t z)
//{
//	//print sensors data on oled
//	sprintf(my_oled_light,"%u      ",my_light);
//    sprintf(my_oled_temp,"%2.2f    ",my_temp);
//	sprintf(my_oled_acc_x, "%d     ",x);
//	sprintf(my_oled_acc_y, "%d     ",y);
//	sprintf(my_oled_acc_z, "%d     ",z);
//
//	//display value on oled
//	oled_putString(50,0.5,(uint8_t *)my_oled_light,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(50,10.5,(uint8_t *)my_oled_temp,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(50,20.5,(uint8_t *)my_oled_acc_x,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(50,30.5,(uint8_t *)my_oled_acc_y,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//	oled_putString(50,40.5,(uint8_t *)my_oled_acc_z,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
//}
//int main(void){
//	SysTick_Config(SystemCoreClock / 1000 ); //generates a 1 millisec clock
//	init_ssp();
//	led7seg_init();
//	init_i2c();
//	oled_init();
//
//	while(1){
//		//printf("Time taken (C version): %ld milliseconds\n",(msTick));
//		//7 seg
//		SevenSeg();
//		oled_template();
//		oled_display(my_light, my_temp, x,y,z);
//	}
//}
//
//void check_failed(uint8_t *file, uint32_t line)
//{
//	/* User can add his own implementation to report the file name and line number,
//	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
//
//	/* Infinite loop */
//	while(1);
//}
