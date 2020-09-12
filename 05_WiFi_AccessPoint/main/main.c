#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

/*
 * Access Point Parameters
 */
#define AP_SSID 			"ESP32"
#define AP_PASSWORD 		"12345678"
#define AP_MAX_CONNECTION 	4
#define AP_CHANNEL 			0



/*
 * Main Function
 */
void app_main()
{
  ESP_ERROR_CHECK(nvs_flash_init());

  tcpip_adapter_init();

  wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));


  /*
   * Access Point Structure
   */
	wifi_config_t ap_config = {
		.ap = {
			.ssid 			= AP_SSID,
			.password 		= AP_PASSWORD,
			.max_connection	= AP_MAX_CONNECTION,
			.channel 		= AP_CHANNEL,
			.ssid_hidden 	= 0,
		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP,&ap_config));
	ESP_ERROR_CHECK(esp_wifi_start());


      for(;;)
      {
    	  vTaskDelay(3000 / portTICK_RATE_MS);
      }
  }
