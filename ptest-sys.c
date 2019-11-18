
#include "./include/platform4esp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_log.h"
#include "stdio.h"
#include "sdkconfig.h"
#include "string.h"
#include "ssd1366.h"
#include <esp32/rom/rtc.h>
#include "nvs_flash.h"
#include "nvs.h"
#include <time.h>
#include <sys/time.h>
#include <esp_sleep.h>

/////////////////////////////////////////////////////////////////////////////
//external routines
/////////////////////////////////////////////////////////////////////////////
void platform_init_iic(void);
void platform_init_oled(void);
void platform_init_wifi(void);
nvs_handle platform_init_nvs(void);
int  platform_init_rtdb(void);
void platform_init_sntp(void);
void platform_init_udp(void);
void platform_init_wserver(void);
void init_ptest(void);
void platform_init_spiffs(void);
void platform_init_cli(void);
void platform_init_msg(void);
void init_sensors(void);

extern void lcd_clear(void);
extern void lcd_sfont(uint8 n);
extern void lcd_text_at(uint8 l,uint8 c, char *b);
extern void rest_sys_flags(void);
int exec(char *s);
extern nvs_handle my_handle;
extern char * sget(char *t);
extern int get(char *t, int *v);
extern void set_led(uint8 c);
extern void lcd_onoff(uint8 p);
extern void init_scheduler();
extern void schedule();

extern void get_i8_nvs(nvs_handle h,char* s,int8_t* v) ;

///////////////////////////////
// application callouts
////////////////////////////////////////////////////////////////////////
// with modification 3v3b enable is now gpio 19
// set high to enable and this allows the oled to free the IIC bus

void plat_gpio(uint16 p, uint16 v)
{
gpio_pad_select_gpio(p);
gpio_set_direction(p, GPIO_MODE_OUTPUT);
gpio_set_level(p, v);
if (v==0) gpio_pulldown_en(p);
else gpio_pullup_en(p);
}

void plat_down()
{
lcd_clear();
lcd_clear();
// disable 5va, 5vb, 12v
set_xbv(2,9,1);
set_xbv(3,13,0);
set_xbv(2,8,1);
//set led36 and led37 off
set_xbv(3,6,1);
set_xbv(3,7,1);
// disable 3v3a 
set_xbv(1,5,0);
plat_gpio(19,0); //disable 3v3b for Oled last last
}

void plat_up()
{
//enable 3v3b for oled
plat_gpio(19,1);
// check 3v3a enable
set_xbv(1,5,1);
vTaskDelay(10/ portTICK_PERIOD_MS) ; // wait for XIO chips to reset
//set led36 on and led37 off to show system startup and power is OK
set_xbv(3,6,0);
set_xbv(3,7,1);
//set 5va, 5vb, 12v off

set_xbv(2,9,1);
set_xbv(3,13,0);
set_xbv(2,8,1);
}

void plat_sleep(uint64_t us)
{
plat_down();
esp_deep_sleep(us);
//never returns
}

extern int glsclose();

void gracefull(int secs)
{
uint64_t us;
if (glopen==1) glsclose();
vTaskDelay(1000/ portTICK_PERIOD_MS);  //wait 1 sec for socket to close
us = secs * 1000000;
plat_sleep(us);
//never returns
}


//////////////////////////////////////////////////////////////////////////////
// RESET reason
// This shows the reason for a restart
// Under normal operations this will always be a DEEPSLEEP restart
//////////////////////////////////////////////////////////////////////////////
char *print_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : return "POWERON RESET";break;          /**<1, Vbat power on reset*/
    case 3 : return "SW RESET";break;               /**<3, Software reset digital core*/
    case 4 : return "OWDT RESET";break;             /**<4, Legacy watch dog reset digital core*/
    case 5 : return "DEEPSLEEP RESET";break;        /**<5, Deep Sleep reset digital core*/
    case 6 : return "SDIO RESET";break;             /**<6, Reset by SLC module, reset digital core*/
    case 7 : return "TG0 WDT SYS RESET";break;       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : return "TG1 WDT SYS RESET";break;       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : return "RTC WDT SYS RESET";break;       /**<9, RTC Watch dog Reset digital core*/
    case 10 : return "INTRUSION RESET";break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : return "TG WDT CPU RESET";break;       /**<11, Time Group reset CPU*/
    case 12 : return "SW CPU RESET";break;          /**<12, Software reset CPU*/
    case 13 : return "RTC WDT CPU RESET";break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : return "EXT CPU RESET";break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return "RTC WDT BROWN OUT RESET";break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : return "RTC WDT RTC RESET";break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return "NO_MEANING";
  }
  
}
///////////////////////////////////////////////////////////////////////////
// This is the main platform entry point .
// Every reset and restart starts here
//////////////////////////////////////////////////////////////////////////
void ptest_start(void)
{
int r0, r1;                                               
r0 = rtc_get_reset_reason(0) ;
r1 = rtc_get_reset_reason(1) ;

// indicate time has not been set
gtset = 0;
// always initialise the CLI so that we can configure system by hand
platform_init_cli() ;  
//If we get here then power is OK otherwise we have gone back to deep sleep
// and never reach this point
printf("\nCPU0 :%s   CPU1: %s",print_reason(r0),print_reason(r1));
// turn on 3v3b to stop Oled blocking bus
plat_gpio(19,1);
printf("\nTurned on 3v3b to stop Oled block");
platform_init_iic();
// this has also enabled 3v3a otherise U1 and U10 cant be initialised properly
// now we can initialise the other power enable lines in a low level way
plat_up();
platform_init_oled(); // else use flag to indicate there is no OLED LCD
printf("\nStarting PTEST APPLICATION\n");
 

 init_ptest2();
 init_ptest3();

}
