
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

/*
 * Function Prototype
 */
void wifiScan(void);

/*
 * Main Function
 */
void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	tcpip_adapter_init();

	  wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
	  ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
	  ESP_ERROR_CHECK(esp_wifi_start());

	  for(;;)
	  {
		  wifiScan();
		  vTaskDelay(3000 / portTICK_RATE_MS);
	  }

}

/*
 * Function Definition
 */
void wifiScan(void)
{
	wifi_scan_config_t scan_config = {
			.ssid = 0,
			.bssid = 0,
			.channel = 0,
			.show_hidden = true
	};

  printf("Start scanning...");
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
  printf(" completed!\n\n");


  uint8_t ap_num;
  wifi_ap_record_t ap_records[20];
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));


  printf("Found %d access points:\n", ap_num);

  printf("               SSID              | Channel | RSSI |   MAC \n\n");
  printf("----------------------------------------------------------------\n");
  for(int i = 0; i < ap_num; i++)
    printf("%32s | %7d | %4d   %2x:%2x:%2x:%2x:%2x:%2x   \n", 	ap_records[i].ssid,
    															ap_records[i].primary,
																ap_records[i].rssi ,
																*(ap_records[i].bssid),
																*(ap_records[i].bssid)+1,
																*(ap_records[i].bssid)+2,
																*(ap_records[i].bssid)+3,
																*(ap_records[i].bssid)+4,
																*(ap_records[i].bssid)+5 );
  printf("----------------------------------------------------------------\n");

}
