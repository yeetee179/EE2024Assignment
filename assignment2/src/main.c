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

#define LIGHT_WARNING_LOWER 50
#define LIGHT_WARNING_UPPER 1000
#define RGB_CLOCK 333
#define SEG_CLOCK 1000
#define TEMP_COUNTER 340 // range between 1 to 900
#define TEMP_OFFSET 0
#define TEMP_DUR 100

uint32_t MOD_LIGHT_WARNING_LOWER = LIGHT_WARNING_LOWER;
uint32_t MOD_LIGHT_WARNING_UPPER = LIGHT_WARNING_UPPER;

/* <--- SEGMENT DISPLAY ------> */
int RESET_INITIAL_7SEG = 1;		//SEGMENT DISPLAY (need to change)
int RESET_INITIAL_RGB = 1;
int RESET_INITIAL = 1;
int INITIAL_TIME_7SEG = 0;
int INITIAL_TIME_RGB = 0;
int INITIAL_TIME = 0;
int CURRENT_TIME = 0;
int SEGMENT_DISPLAY = -1;
int trans_no = 0; //no. of transmission to SAFE

int ENERGY_LED_COUNT=0;

int TIME_CAPTURE_1 = 0;			//added below
int TIME_CAPTURE_2 = 0;
int TIME_CAPTURED = 0;
int TIME_FACTOR = 0;
int REMAINDER_TIME = 0;
int TIME_NUM = 0;

int blink_red = 0;				//RGB blink setting
int blink_blue = 0;

volatile uint32_t mstick = 0;
volatile uint32_t timer = 0;

volatile int ModeNum = 0; 		//MODE: 0 - Default, 1 - Passive, 2 - Date
volatile int getInfo = 0; 		//GetInformation

int passive_init_flag = 1;
int date_init_flag = 1;

uint16_t led_num = 0xffff; 		//Energy Led setting
uint16_t led_clear = 1<<15;

uint32_t my_light = 5000;
double my_temp = 0;
double mod_temp_offset = TEMP_OFFSET;
int temp_warning_counter = 0;
int RGB_COUNTER = 1;
int temp_counter = 0;
int toggle = 0;

int32_t xoff = 0;
int32_t yoff = 0;
int32_t zoff = 0;

int32_t xoff_reset = 0;
int32_t yoff_reset = 0;
int32_t zoff_reset = 0;

int8_t x = 0;
int8_t y = 0;
int8_t z = 0;

uint8_t my_oled_light[40]={};
uint8_t my_oled_temp[40]={};
uint8_t my_oled_acc_x[40]={};
uint8_t my_oled_acc_y[40]={};
uint8_t my_oled_acc_z[40]={};
uint8_t my_oled_mode[40]={};
uint8_t my_oled_light_label[40]={};
uint8_t my_oled_temp_label[40]={};
uint8_t my_oled_acc_x_label[40]={};
uint8_t my_oled_acc_y_label[40]={};
uint8_t my_oled_acc_z_label[40]={};

uint8_t uart_transmit[60] = {};
int trans_flag = 1;

int toggle_pre = 0;				//Toggle Mode variable
int toggle_post = 0;
int read_counter = 1;
uint8_t btn1 = 1;
int flag = 99;
int pre = 99;
int select_no = 0;

static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
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
	I2C_Init(LPC_I2C2, 100000);  //working at 1000Khz

	/* Enable I2C2 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);
}

static void init_GPIO(void)
{
	// Initialize button
	PINSEL_CFG_Type PC;

	PC.Funcnum = 0;
	PC.OpenDrain = 0;
	PC.Pinmode = 0;
	PC.Portnum = 1;		//SW4 button
	PC.Pinnum = 31;
	PINSEL_ConfigPin(&PC);
	PC.Portnum = 2;		//SW3 button
	PC.Pinnum = 10;
	PINSEL_ConfigPin(&PC);
	PC.Portnum = 2;		//RED RGB
	PC.Pinnum = 0;
	PINSEL_ConfigPin(&PC);
	PC.Portnum = 0;		//BLUE RGB
	PC.Pinnum = 26;
	PINSEL_ConfigPin(&PC);

	GPIO_SetDir(1,1<<31,0);
	GPIO_SetDir(2,1<<10,0);
	GPIO_SetDir(2,1,1);
	GPIO_SetDir(0,1<<26,1);
}

void SysTick_Handler(void){
	mstick++;
	}
uint32_t getTicks(void){
    	return mstick;
}
volatile double read_temp_sensor(void){
	double my_temp = 0;
	my_temp = temp_read()/10.0;
	return my_temp;
}
volatile uint32_t read_light_sensor(void){
	light_enable();
	my_light = light_read();
	return my_light;
}
void SEGMENT_NUM (SEGMENT_DISPLAY){
	if(SEGMENT_DISPLAY == 0){led7seg_setChar('0',FALSE);}
	if(SEGMENT_DISPLAY == 1){led7seg_setChar('1',FALSE);}
	if(SEGMENT_DISPLAY == 2){led7seg_setChar('2',FALSE);}
	if(SEGMENT_DISPLAY == 3){led7seg_setChar('3',FALSE);}
	if(SEGMENT_DISPLAY == 4){led7seg_setChar('4',FALSE);}
	if(SEGMENT_DISPLAY == 5){led7seg_setChar('5',FALSE);}
	if(SEGMENT_DISPLAY == 6){led7seg_setChar('6',FALSE);}
	if(SEGMENT_DISPLAY == 7){led7seg_setChar('7',FALSE);}
	if(SEGMENT_DISPLAY == 8){led7seg_setChar('8',FALSE);}
	if(SEGMENT_DISPLAY == 9){led7seg_setChar('9',FALSE);}
	if(SEGMENT_DISPLAY == 10){led7seg_setChar('A',FALSE);}
	if(SEGMENT_DISPLAY == 11){led7seg_setChar('8',FALSE);} //B
	if(SEGMENT_DISPLAY == 12){led7seg_setChar('C',FALSE);}
	if(SEGMENT_DISPLAY == 13){led7seg_setChar('0',FALSE);} //D
	if(SEGMENT_DISPLAY == 14){led7seg_setChar('E',FALSE);}
	if(SEGMENT_DISPLAY == 15){led7seg_setChar('F',FALSE);}
}
void read_acc_sensor (int8_t *x, int8_t *y, int8_t *z, int8_t xoff, int8_t yoff, int8_t zoff){
 	acc_read(x, y, z);	//read x,y,z values
 	//account for initial offset
 	*x=*x+xoff;
 	*y=*y+yoff;
 	*z=*z+zoff;
 }
void rgb_feedback (uint32_t my_light)
{
	RGB_COUNTER++;
	if (my_light <= MOD_LIGHT_WARNING_LOWER) blink_red = 1;
	if (my_light >= MOD_LIGHT_WARNING_LOWER && my_light <= MOD_LIGHT_WARNING_UPPER)
		blink_blue = 1;
	if (RGB_COUNTER % 2 == 0){
		if (blink_blue == 1) {
			GPIO_SetValue(0,1<<26);
		}
		if (blink_red == 1) {
			GPIO_SetValue(2,1);
		}
	}else{
			GPIO_ClearValue(0,1<<26);
			GPIO_ClearValue(2,1);
		}


}
void EINT3_IRQHandler (void)
{
	if((LPC_GPIOINT -> IO0IntStatR >> 2) & 0x1){ //temperature sensor
		LPC_GPIOINT -> IO0IntClr = 1<<2;
		temp_counter++;
	}
	if((LPC_GPIOINT -> IO2IntStatF >> 10) & 0x1){ //btnsw3(TOGGLE MODE SWITCH) is pressed
		LPC_GPIOINT -> IO2IntClr = 1<<10;
		getInfo++;
	}
}
void oled_template()
{
	sprintf(my_oled_light_label,"LUX:          ");
	sprintf(my_oled_temp_label,"TEMP:          ");
	sprintf(my_oled_acc_x_label, "ACC_X:       ");
	sprintf(my_oled_acc_y_label, "ACC_Y:       ");
	sprintf(my_oled_acc_z_label, "ACC_Z:       ");
	oled_putString(10,0.5,(uint8_t *)my_oled_light_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,10.5,(uint8_t *)my_oled_temp_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,20.5,(uint8_t *)my_oled_acc_x_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,30.5,(uint8_t *)my_oled_acc_y_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,40.5,(uint8_t *)my_oled_acc_z_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}
void oled_template_select(void) // enhancement
{
	sprintf(my_oled_light_label,"LUX: %u %u   ",MOD_LIGHT_WARNING_LOWER,MOD_LIGHT_WARNING_UPPER);
	sprintf(my_oled_temp_label,"TEMP: %2.2f    ",mod_temp_offset);
	sprintf(my_oled_acc_x_label, "ACC_X: %d    ",xoff);
	sprintf(my_oled_acc_y_label, "ACC_Y: %d    ",yoff);
	sprintf(my_oled_acc_z_label, "ACC_Z: %d    ",zoff);
	if(select_no == 0)
		oled_putString(10,0.5,(uint8_t *)my_oled_light_label,OLED_COLOR_BLACK,OLED_COLOR_WHITE);
	else
		oled_putString(10,0.5,(uint8_t *)my_oled_light_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(select_no == 1)
		oled_putString(10,10.5,(uint8_t *)my_oled_temp_label,OLED_COLOR_BLACK,OLED_COLOR_WHITE);
	else
		oled_putString(10,10.5,(uint8_t *)my_oled_temp_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(select_no == 2)
		oled_putString(10,20.5,(uint8_t *)my_oled_acc_x_label,OLED_COLOR_BLACK,OLED_COLOR_WHITE);
	else
		oled_putString(10,20.5,(uint8_t *)my_oled_acc_x_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(select_no == 3)
		oled_putString(10,30.5,(uint8_t *)my_oled_acc_y_label,OLED_COLOR_BLACK,OLED_COLOR_WHITE);
	else
		oled_putString(10,30.5,(uint8_t *)my_oled_acc_y_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	if(select_no == 4)
		oled_putString(10,40.5,(uint8_t *)my_oled_acc_z_label,OLED_COLOR_BLACK,OLED_COLOR_WHITE);
	else
		oled_putString(10,40.5,(uint8_t *)my_oled_acc_z_label,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}
void oled_display(uint32_t my_light,double my_temp,int8_t x,int8_t y,int8_t z)
{
	//print sensors data on oled
	sprintf(my_oled_light,"%u      ",my_light);
    sprintf(my_oled_temp,"%2.2f    ",my_temp);
	sprintf(my_oled_acc_x, "%d     ",x);
	sprintf(my_oled_acc_y, "%d     ",y);
	sprintf(my_oled_acc_z, "%d     ",z);

	//display value on oled
	oled_putString(50,0.5,(uint8_t *)my_oled_light,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(50,10.5,(uint8_t *)my_oled_temp,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(50,20.5,(uint8_t *)my_oled_acc_x,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(50,30.5,(uint8_t *)my_oled_acc_y,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(50,40.5,(uint8_t *)my_oled_acc_z,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}
void oled_display_PASSIVE(void)
{
	sprintf(my_oled_mode, "MODE:PASSIVE");
	oled_putString(10,50.5,(uint8_t *)my_oled_mode,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}
void oled_display_CONFIG(void)
{
	sprintf(my_oled_mode, "MODE:CONFIG");
	oled_putString(10,50.5,(uint8_t *)my_oled_mode,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}
void oled_display_DATE(void)
{
	sprintf(my_oled_mode, "MODE:DATE    ");
	sprintf(my_oled_light,"DATE MODE");
	sprintf(my_oled_temp,"DATE MODE");
	sprintf(my_oled_acc_x, "DATE MODE");
	sprintf(my_oled_acc_y, "DATE MODE");
	sprintf(my_oled_acc_z, "DATE MODE");
	oled_putString(10,50.5,(uint8_t *)my_oled_mode,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,0.5,(uint8_t *)my_oled_light,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,10.5,(uint8_t *)my_oled_temp,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,20.5,(uint8_t *)my_oled_acc_x,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,30.5,(uint8_t *)my_oled_acc_y,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	oled_putString(10,40.5,(uint8_t *)my_oled_acc_z,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
}

void initenergyled(void){
	pca9532_setLeds (led_num,0);
	RESET_INITIAL = 1;
}
void energyled(void){
	pca9532_setLeds (0,led_clear);
	led_clear = led_clear/2;
	if(led_clear==0) {
		ModeNum = 1;
		SEGMENT_DISPLAY = -1;
		RESET_INITIAL_7SEG = 1;
	}
	RESET_INITIAL = 1;
}
void ToggleMode(void){
	btn1 = (GPIO_ReadValue(1) >> 31) & 0x01;			//SW4 switch function
	if (btn1 == 0) toggle_pre = 1;
	if (toggle_pre == 1 && btn1 == 1)toggle_post = 1;
	if (toggle_pre == 1 && toggle_post == 1){
		if(ModeNum == 1){
			toggle = 1;
			if(SEGMENT_DISPLAY != 15)
				flag = 16 - SEGMENT_DISPLAY;
			else
				flag = 17;
		}
		if(ModeNum == 0)
			ModeNum = 1;
		if(ModeNum >= 2)
			ModeNum++;
		toggle_pre = 0;
		toggle_post = 0;
	}
	if(pre != SEGMENT_DISPLAY &&  toggle == 1)
		flag--;
	pre = SEGMENT_DISPLAY;
	if(flag == 0){
		ModeNum = 2;
		flag = 99;
		toggle = 0;
	}
	if(ModeNum > 3)
		ModeNum = 2;
}
void InitialSetting(void){
	oled_clearScreen(OLED_COLOR_BLACK);
	led7seg_setChar(' ',FALSE);
	GPIO_ClearValue(0,1<<26);
	GPIO_ClearValue(2,1);
}
void RGBFunc(){
	CURRENT_TIME = getTicks();
	if(RESET_INITIAL_RGB == 1){
		RESET_INITIAL_RGB = 0;
		INITIAL_TIME_RGB = CURRENT_TIME; 			//set reference time for RGB
		}
	if(CURRENT_TIME - INITIAL_TIME_RGB >= RGB_CLOCK){ 	//if >= RGB_CLOCK passed
		rgb_feedback(my_light);						//Blink Blue or Red RGB
		RESET_INITIAL_RGB = 1;
	}
}
void SevenSegFunc(){
	CURRENT_TIME = getTicks();
	if(RESET_INITIAL_7SEG == 1){
		RESET_INITIAL_7SEG = 0;
		INITIAL_TIME_7SEG = CURRENT_TIME; 			//set reference time
		}
	if(CURRENT_TIME - INITIAL_TIME_7SEG >= SEG_CLOCK){ 	//if >= 1 second passed

		if(SEGMENT_DISPLAY == 15){ 					//set 7 segment display range from 0 to 15
			SEGMENT_DISPLAY = 0;
			RESET_INITIAL_7SEG = 1;

			} else{
				SEGMENT_DISPLAY++;
				RESET_INITIAL_7SEG = 1;
				read_counter = 1;
				trans_flag = 1;
			}
		SEGMENT_NUM(SEGMENT_DISPLAY); 				//7-segment display in 1 second interval
		}
}
void SystemReading(void){
	if(read_counter == 1){
		if( SEGMENT_DISPLAY == 5 || SEGMENT_DISPLAY == 10 || SEGMENT_DISPLAY == 15){
			read_counter =0;
			my_light = read_light_sensor();					//capture light sensor value
			read_acc_sensor(&x,&y,&z,xoff,yoff,zoff); 		//capture x,y,z sensor value
			oled_display(my_light,my_temp,x,y,z);			//display all values on oled screen
			}
	}
}

void getinfoBtn(int8_t *x, int8_t *y, int8_t *z, int8_t xoff, int8_t yoff, int8_t zoff){
		TIME_CAPTURE_1 = getTicks();				//capture initial time
		//read light and temperature sensor functions
		my_light = read_light_sensor();				//capture light sensor value
		my_temp = read_temp_sensor() + mod_temp_offset;				//capture temp sensor value can substitute with temp_read
		read_acc_sensor(x,y,z,xoff,yoff,zoff); 	//capture x,y,z sensor value
		oled_template();
		oled_display(my_light,my_temp,*x,*y,*z);		//display all values on oled screen
		TIME_CAPTURE_2 = getTicks();		//capture final time
		TIME_CAPTURED = TIME_CAPTURE_2 - TIME_CAPTURE_1;	//account for time difference
		TIME_FACTOR = TIME_CAPTURED / 208;
		for(TIME_NUM = 0 ; TIME_NUM  < TIME_FACTOR ; TIME_NUM++){
			energyled();
		}											//account for time difference in led
		REMAINDER_TIME = TIME_CAPTURED - TIME_FACTOR * 208;
		mstick = mstick + REMAINDER_TIME;			//remainder time is added back to global timer
		getInfo = 0;
		sprintf(uart_transmit,"%03d_-_T%2.2f_L%u_AX%d_AY%d_AZ%d \r\n",trans_no,my_temp,my_light,*x,*y,*z);
		UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
		trans_no++;
}
void EnergyLedFunc(void){
	CURRENT_TIME = getTicks();
	if(RESET_INITIAL == 1){
		RESET_INITIAL = 0;
		INITIAL_TIME = CURRENT_TIME; //set reference time for energyled
		}
	if(ENERGY_LED_COUNT == 0)
	{
		oled_clearScreen(OLED_COLOR_BLACK);
		oled_display_DATE();
		initenergyled();
	}
	ENERGY_LED_COUNT++;
	if(ENERGY_LED_COUNT >= 2){
		if(CURRENT_TIME - INITIAL_TIME >= 208){
			energyled();
		}
	}
}
void InitFunc(void){

	init_i2c();
    init_ssp();
    init_GPIO();
    led7seg_init();
    pca9532_init();
    joystick_init();
    acc_init();
    oled_init();
	temp_init(&getTicks);
 	SysTick_Config(SystemCoreClock/1000); // provide clock of 1KHz
 	init_uart();
 	InterruptConfig();

    /* Assume base board in zero-g position when reading first value. */
    acc_read(&x, &y, &z);
    xoff = 0-x;
    yoff = 0-y;
    zoff = 64-z;
    xoff_reset = xoff;
    yoff_reset = yoff;
    zoff_reset = zoff;
}
uint32_t t1 = 0;
uint32_t t2 = 0;
int initial = 0;
void read_temp (void){
		if(initial == 0){
			t1 = getTicks();
			initial = 1;
			temp_counter = 0;
		}
		if(temp_counter >= TEMP_COUNTER){
			t2 = getTicks();
			initial = 0;
			if(t2 > t1)
				t2 = t2 - t1;
			else
				t2 = (0xFFFFFFFF - t1 + 1) + t2;
			t2 = ((1000*t2)/(temp_counter) - 2731);
			my_temp = t2/10.0 + mod_temp_offset;
		}
//		if(my_temp > 30){
//			temp_warning_counter++;
//			if(temp_warning_counter >= TEMP_DUR){
//				sprintf(uart_transmit,"Threat of Algae presence.\r\n");
//			    UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
//				temp_warning_counter = 0;
//			}
//		} else {
//			temp_warning_counter = 0;
//		}
}
uint8_t status = 0;
int joystick_pre_right = 0, joystick_post_right = 0;
void joystick_toggle_right(void){
	status =  joystick_read();
	if(status == JOYSTICK_RIGHT) joystick_pre_right = 1;
	if(status != JOYSTICK_RIGHT && joystick_pre_right == 1) joystick_post_right = 1;
	if(joystick_pre_right == 1 && joystick_post_right == 1){
		joystick_pre_right = 0;
		joystick_post_right = 0;
		if(select_no == 0){
			MOD_LIGHT_WARNING_LOWER = MOD_LIGHT_WARNING_LOWER + 50;
			MOD_LIGHT_WARNING_UPPER = MOD_LIGHT_WARNING_UPPER + 50;
		}
		if(select_no == 1){
			mod_temp_offset++;
		}
		if(select_no == 2){
			xoff++;
		}
		if(select_no == 3){
			yoff++;
		}
		if(select_no == 4){
			zoff++;
		}
	}
}
int joystick_pre_left = 0, joystick_post_left = 0;
void joystick_toggle_left(void){
	status =  joystick_read();
	if(status == JOYSTICK_LEFT) joystick_pre_left = 1;
	if(status != JOYSTICK_LEFT && joystick_pre_left == 1) joystick_post_left = 1;
	if(joystick_pre_left == 1 && joystick_post_left == 1){
		joystick_pre_left = 0;
		joystick_post_left = 0;
		if(select_no == 0){
			MOD_LIGHT_WARNING_LOWER = MOD_LIGHT_WARNING_LOWER - 50;
			MOD_LIGHT_WARNING_UPPER = MOD_LIGHT_WARNING_UPPER - 50;
		}
		if(select_no == 1){
			mod_temp_offset--;
		}
		if(select_no == 2){
			xoff--;
		}
		if(select_no == 3){
			yoff--;
		}
		if(select_no == 4){
			zoff--;
		}
	}
}
void joystick_toggle_center(void){
	if(status == JOYSTICK_CENTER){
		MOD_LIGHT_WARNING_LOWER = LIGHT_WARNING_LOWER;
		MOD_LIGHT_WARNING_UPPER = LIGHT_WARNING_UPPER;
		mod_temp_offset = TEMP_OFFSET;
		xoff = xoff_reset;
		yoff = yoff_reset;
		zoff = zoff_reset;
	}
}
int joystick_pre_up = 0, joystick_post_up = 0;
void joystick_toggle_up(void){
	status =  joystick_read();
	if(status == JOYSTICK_UP) joystick_pre_up = 1;
	if(status != JOYSTICK_UP && joystick_pre_up == 1) joystick_post_up = 1;
	if(joystick_pre_up == 1 && joystick_post_up == 1){
		joystick_pre_up = 0;
		joystick_post_up = 0;
		select_no--;
		if(select_no == -1) select_no = 4;
	}
}
int joystick_pre_down = 0, joystick_post_down = 0;
void joystick_toggle_down(void){
	status =  joystick_read();
	if(status == JOYSTICK_DOWN) joystick_pre_down = 1;
	if(status != JOYSTICK_DOWN && joystick_pre_down == 1) joystick_post_down = 1;
	if(joystick_pre_down == 1 && joystick_post_down == 1){
		joystick_pre_down = 0;
		joystick_post_down = 0;
		select_no++;
		if(select_no == 5) select_no = 0;
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
	//supply power & setup working par.s for uart3
	UART_Init(LPC_UART3, &uartCfg);
	//enable transmit for uart3
	UART_TxCmd(LPC_UART3, ENABLE);
}
void InterruptConfig(void){
	LPC_GPIOINT->IO2IntClr = 1<<10;	//SW3

	 	LPC_GPIOINT -> IO2IntEnF |= 1 << 10; 	// enable interrupt p2.10 - btnsw3
	 	uint32_t ans, PG=5, PP=0b11, SP=0b000;
	 	NVIC_SetPriorityGrouping(5);
	 	ans = NVIC_EncodePriority(PG,PP,SP);
	 	NVIC_SetPriority(EINT3_IRQn,ans);		//EINT3 priority 0xC0

	 	LPC_GPIOINT -> IO0IntEnR |= 1 << 2; 	//enable interrupt P0.2 - temperature sensor
	 	PG=5, PP=0b10, SP=0b000;
	 	NVIC_SetPriorityGrouping(5);
	 	ans = NVIC_EncodePriority(PG,PP,SP);
	 	NVIC_SetPriority(EINT3_IRQn,ans);		//EINT3 priority 0x80

	    NVIC_EnableIRQ(EINT3_IRQn); 			// enable EINT3 interrupt
}
void passive_init (void){
	if(passive_init_flag == 1){
		sprintf(uart_transmit,"Entering PASSIVE Mode.\r\n");
		UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
		passive_init_flag = 0;
		date_init_flag = 1;
		ENERGY_LED_COUNT = 0;
		led_clear = 1 << 15;
		getInfo = 0;
		oled_template();
		oled_display_PASSIVE();
		my_light = read_light_sensor();					//capture light sensor value
		read_acc_sensor(&x,&y,&z,xoff,yoff,zoff); 		//capture x,y,z sensor value
		}
}
void uart_send_data(void){
	if(trans_flag == 1 && SEGMENT_DISPLAY == 15){
		trans_flag = 0;
		if(blink_blue == 1){
			sprintf(uart_transmit,"Algae was Detected.\r\n");
			UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
		}
		if(blink_red == 1){
			sprintf(uart_transmit,"Solid Wastes was Detected.\r\n");
			UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
		}
		sprintf(uart_transmit,"%03d_-_T%2.2f_L%u_AX%d_AY%d_AZ%d \r\n",trans_no,my_temp,my_light,x,y,z);
		UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
		trans_no++;							//Transmission number
		}
}
void date_init (void){
	if(date_init_flag == 1){
		sprintf(uart_transmit,"Leaving PASSIVE Mode. Entering DATE Mode.\r\n");
		UART_Send(LPC_UART3,(uint8_t *)uart_transmit, strlen(uart_transmit),BLOCKING);
		date_init_flag = 0;
		passive_init_flag = 1;
		blink_red = 0;
		blink_blue = 0;
		GPIO_ClearValue(0,1<<26);
		GPIO_ClearValue(2,1);
		led7seg_setChar(' ',FALSE);
	}
}
int main (void) {

	InitFunc();

 	while (1)
 	{
 		ToggleMode();		//Change ModeNum
 		if(ModeNum == 0)	//INITIAL MODE
 			InitialSetting();
 		if(ModeNum == 1)	//PASSIVE MODE
 		{
 			passive_init();
 			SevenSegFunc();
 			read_temp();
 			SystemReading();
			RGBFunc();
			uart_send_data();
 		}
 		if(ModeNum == 2) 	//DATE MODE
 		{
 			date_init();
 			EnergyLedFunc();
 			if(getInfo >= 1){	//SW3 pressed
 				getinfoBtn(&x, &y, &z, xoff, yoff, zoff);
 			}
 		}
 		if(ModeNum == 3){	//CONFIG MODE
 			joystick_toggle_right();
 			joystick_toggle_left();
 			joystick_toggle_center();
 			joystick_toggle_up();
 			joystick_toggle_down();
 			oled_display_CONFIG();
 			oled_template_select();
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
