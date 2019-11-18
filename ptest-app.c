
// version 2.0 test
//////////////////////////////////////////////////////////////////////////////
//  Aplication device description
//  The following devices are part of the irrigation control system and not
//  part of the platform
//  1) system led : RGB
//  2) 5 Input switches
//  3) the use of OLED LCD
//  4) Valves
//  5) Flow sensor
//  6) Pump
//  7) pressure sensor   
//
//  Output devives like pumps and valves are normally connnected to IO lines
//  Input devices like sensors are read indirectly via a managed Input page
////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// All IO from this irrigation application module must use the PAPI interface
// routines to read and write the platform I
//
/////////////////////////////////////////////////////////////////////////////
#include "./include/platform4esp.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "string.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/adc.h"
#include "driver/pcnt.h"
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include <lwip/sockets.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "soc/gpio_sig_map.h"
#include "driver/mcpwm.h" 
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
//#include "cli.h"

//PCNT  
#define PCNT_TEST_UNIT      PCNT_UNIT_0
#define PCNT_H_LIM_VAL      500
#define PCNT_L_LIM_VAL      0
#define PCNT_INPUT_SIG_IO   14  
#define PCNT_INPUT_CTRL_IO   5
//PWM GPIO
#define PWM1_OUTPUT_GPIO   14  // Pulse Input GPIO
#define PWM2_OUTPUT_GPIO   15  // Control GPIO HIGH=count up, LOW=count down
  
float  DC=2.5;
int    f=50;
 
int mkptest(void)
{

return 0;
}

void lcd_show(void)
{
char iobuf[17];
sprintf(iobuf,"hello");
lcd_sfont(1);
//display on target line
lcd_text_at(1,0,iobuf);
sprintf(iobuf,"SIMON");
lcd_text_at(4,0,iobuf);
}

void init_ptest(void)
{
ctabgroup("Irrigation commands");
loadctab("ptest",mkptest,"platform test command");

//clear LCD
//lcd_clear();
//lcd_clear();
//lcd_show();
}

void init_ptest2(void)
{
ctabgroup("switch commands");
loadctab("setmod",switch1,"select mode");
loadctab("setval1",switch2,"contrl valv1");
loadctab("setval2",switch3,"contrl valv2");
loadctab("setval3",switch4,"contrl valv3");
loadctab("setpumb",switch5,"contrl pumb");
loadctab("getval1",read_switch1,"read valve1 state"); 
}


void init_ptest3(void)
{
ctabgroup("pulse commands"); 
loadctab("pcnt",simon_pcnt,"Pulse input");
loadctab("pwmo1",simon_pwm_output1,"pwm1 ouput:f DC");  
loadctab("pwmo2",simon_pwm_output2,"pwm2 ouput:f DC");  
loadctab("stpwm1",simon_pwm_stop1,"stop pwm1 outpt");  
loadctab("stpwm2",simon_pwm_stop2,"stop pwm2 outpt");   
loadctab("getad1",simon_adc,"get adc value");   
}

//////////////////////////////////////////////
//switches test
//set_xbv(x,y,z);
//get_xbv(d,x);
//switch global variable :int simon_swglo=1
//////////////////////////////////////////////
int read_switch1(void)
{
  if(get_xbv(1,0)==1)
  {
  
  printf("open  valve1\n");
   
  }
  else
  {
   
  printf("close valve1 \n");
   
  }
  return 1;	
}
 

int switch1(void)
{
  if(get_xbv(1,0)==1)
  {
  simon_swglo=1;
  set_xbv(3,2,1);
  printf("open  Manual mode\n");
  switch1_show1();
  }
  else
  {
  simon_swglo=0;
  set_xbv(3,2,1);
  printf("open Automatic mode\n");
  switch1_show2();
  }
  return 1;	
}

int switch2(void)
{

if(simon_swglo==1){
 if(get_xbv(1,1)==1)
 {
  set_xbv(3,14,1);
  printf("open  valv1\n");
   switch2_show1();
 }
 else
 {
  set_xbv(3,14,0);
  printf("close valv1\n");
 switch2_show2();
 }
 }
 return 1;	
}

int switch3(void)
{
if(simon_swglo==1){
 if(get_xbv(1,2)==1)
 {
  set_xbv(2,1,1);
  printf("open  valv2\n");
  switch3_show1();
 }
 else
 {
  set_xbv(2,1,0);
  printf("close valv2\n");
  switch3_show2();
 }
 }
 return 1;	
}

int switch4(void)
{
if(simon_swglo==1){
 if(get_xbv(1,3)==1)
 {
 set_xbv(2,2,1);
 printf("open  valv3\n");
switch4_show1();
 }
 else
 {
 set_xbv(2,2,0);
 printf("close valv3\n");
 switch4_show2();
 }
 }
 return 1;	
}

int switch5(void)
{
if(simon_swglo==1){
 if(get_xbv(1,4)==1)
 {
 set_xbv(3,11,1);
 printf("open  pump1\n");
 switch5_show1();
 }
 else
 {
 set_xbv(3,11,0);
 printf("close pum1\n");
 switch5_show2(); 
 }
 }
 return 1;	
} 

 
 //pcnt drvie 
 /* Initialize PCNT functions:
 *  - configure and initialize PCNT
 *  - set up the input filter
 *  - set up the counter events to watch
 */
static void pcnt_init(void)
{ 
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_REVERSE,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config); //start count 
    /* Configure and enable the input filter */
    // pcnt_set_filter_value(PCNT_TEST_UNIT, 100);
    // pcnt_filter_enable(PCNT_TEST_UNIT); 
    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_TEST_UNIT);
    pcnt_counter_clear(PCNT_TEST_UNIT); 
    /* Everything is set up, now go to counting */
    pcnt_counter_resume(PCNT_TEST_UNIT);
}


////////////////pwm drvie ////// 
static  void pwm_init()
{ 
    f=xatoi(argv[1]);
    DC=xatoi(argv[2]);
    //1. mcpwm gpio initialization
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PWM1_OUTPUT_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PWM2_OUTPUT_GPIO);
    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = f;    //frequency = 50Hz, i.e.  
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;  
     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);  
}
 
 /********** adc drvie *************/
static void adc_init()
{    
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten( ADC1_CHANNEL_0 , ADC_ATTEN_DB_0); 
	adc1_get_raw(ADC1_CHANNEL_4); 
}


 
/*****************************************Functional application **********************************************/
/////////////switches display////////////
///////lcd_text_at(row,clonum,iobuf)
/////////////////////////////////////////
void switch1_show1(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"open");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"manual");
 lcd_text_at(4,0,iobuf);
}
void switch1_show2(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"open");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"auto");
 lcd_text_at(4,0,iobuf);
 }


void switch2_show1(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"open");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"valvl");
 lcd_text_at(4,0,iobuf);
}

void switch2_show2(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"close");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"valv1");
 lcd_text_at(4,0,iobuf);
 }

 void switch3_show1(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"open");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"valv2");
 lcd_text_at(4,0,iobuf);
}

void switch3_show2(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"close");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"valv2");
 lcd_text_at(4,0,iobuf);
 }
 void switch4_show1(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"open");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"valv3");
 lcd_text_at(4,0,iobuf);
}

void switch4_show2(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"close");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"valv3");
 lcd_text_at(4,0,iobuf);
 }
 void switch5_show1(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"open");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"pumb");
 lcd_text_at(4,0,iobuf);
}

void switch5_show2(void)
{
 lcd_clear();
 lcd_clear();
 char iobuf[17];
 sprintf(iobuf,"close");
 lcd_sfont(1);
 lcd_text_at(1,0,iobuf);
 sprintf(iobuf,"pumb");
 lcd_text_at(4,0,iobuf); 
 }
 
/*******pcnt count************/
int simon_pcnt()
{   
	int16_t count = 0;  
    set_xbv(2,9,0); 
	pcnt_init();
    while (1) {
		vTaskDelay(500 / portTICK_PERIOD_MS);
		pcnt_get_counter_value(PCNT_TEST_UNIT, &count); 
		printf("Current counter value :%d\n",count  ); 
    }
    return 1;	
}
/*******pwm input************/
 int simon_pwm_output1()
{   
	set_xbv(2,9,0); 
    printf("Testing servo motor.......\n");
    pwm_init() ; 
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, DC);   
   return 1;	
} 
 int simon_pwm_output2()
{   
	set_xbv(2,9,0); 
    printf("Testing servo motor.......\n");
    pwm_init() ; 
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, DC);   
   return 1;	
} 
/*******pwm stop************/
int simon_pwm_stop1()
{
 mcpwm_stop(MCPWM_UNIT_0,MCPWM_TIMER_0); 
 return 1;
}

int simon_pwm_stop2()
{
 mcpwm_stop(MCPWM_UNIT_0,MCPWM_TIMER_1); 
 return 1;
}

/*******adc test************/
int simon_adc()
{
 set_xbv(2,9,0); 
 adc_init();
 while(1){
 printf("valve:%d \n",adc1_get_raw(ADC1_CHANNEL_0));
  vTaskDelay(500 / portTICK_PERIOD_MS);
 }
 return 1;
}

