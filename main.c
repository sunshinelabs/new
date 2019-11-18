
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
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/err.h"
#include <lwip/sockets.h>
#include "driver/uart.h"

void ptest_start(void);

// this is the ESP SDK C starting point    hello simon132321
void app_main()
{
// initialise the platform IO system otherwise printf wont work helllo  barry123
io_init();
/* Print chip information */
esp_chip_info_t chip_info;
esp_chip_info(&chip_info);
printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
    chip_info.cores,
    (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
    (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
printf("silicon revision %d, ", chip_info.revision);
printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
   (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
fflush(stdout);//Clear cache
// call the main platform start up
printf("\n\n");
printf("P TEST  : for the ESP32 v2 module !!\n\n");
ptest_start();

}
