#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"


#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define TAG		"ESP32"
/*
 *		WiFi Parameters
 */
#define WIFI_SSID 	"pi"
#define WIFI_PASS 	"123456789"

/*
 *		Group Event Handle
 */
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;


/*
 *		WiFi Event Handler Function
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id)
    {
		case SYSTEM_EVENT_STA_START:
		{
			esp_wifi_connect();
		}
		break;

		case SYSTEM_EVENT_STA_GOT_IP:
		{
			xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		}
		break;

		case SYSTEM_EVENT_STA_DISCONNECTED:
		{
			xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		}
		break;

		default:
			break;
    }

	return ESP_OK;
}


/*
 * 			Main Task Function
 */
void main_task(void *pvParameter)
{
	printf("Main task: waiting for connection to the wifi network... ");
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	printf("connected!\n");

	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	printf("IP Address:  %s\n", ip4addr_ntoa(&ip_info.ip));
	printf("Subnet mask: %s\n", ip4addr_ntoa(&ip_info.netmask));
	printf("Gateway:     %s\n", ip4addr_ntoa(&ip_info.gw));


	while(1) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}


/*
 * 			Main Function
 */
void app_main()
{
	 /* 		Set Log Status							*/
	esp_log_level_set(TAG, ESP_LOG_INFO);

	ESP_ERROR_CHECK(nvs_flash_init());

	 /* 		Create Event Group Handle				*/
	wifi_event_group = xEventGroupCreate();


	 /* 		Initialize The TCP Stack				*/
	tcpip_adapter_init();


	 /* 		Initialize the WiFi Event Handler	   	*/
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));


	 /* 		Set WiFi Stack in STAtion Mode			*/
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));



	 /* 		Set WiFi Parameters						*/

	wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

	printf("Connecting to %s\n", WIFI_SSID);

	/*
	 * 		Tasks Create
	 */
    xTaskCreate(&main_task, "main_task", 2048, NULL, 5, NULL);
}

