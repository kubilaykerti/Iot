#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

/*
 * WiFi Parameter
 */
#define SSID		"pi"
#define PASSWORD 	"123456789"


/*
 * Main Function
 */
void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	tcpip_adapter_init();

  wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

  wifi_config_t config = {
	  .sta = {
		  .ssid= SSID,
		  .password = PASSWORD
	  },
  };

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA,&config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());

  for(;;)
  {
	  vTaskDelay(3000 / portTICK_RATE_MS);
  }
}
