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

#define TAG				"ESP32-> "
#define PORT			3333
#define IP_PROTOCOL		IPPROTO_IP
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
 * 			TCP Server Task Function
 */
void tcp_server_task(void *pvParameters)
{
	char addr_str[128];

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

    /* 		Socket	Create						*/
    int listen_sock = socket(dest_addr.sin_family, SOCK_STREAM, IP_PROTOCOL);
    if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelete(NULL);
            return;
    }
    ESP_LOGI(TAG, "Socket created");

    /* 		Port						*/
    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		close(listen_sock);
		vTaskDelete(NULL);
	}
	ESP_LOGI(TAG, "Socket bound, port %d", PORT);

	err = listen(listen_sock, 1);
	if (err != 0) {
		ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
		close(listen_sock);
		vTaskDelete(NULL);
	}

	/*
	 * 		Task Loop
	 */
	for(;;)
	{
		ESP_LOGI(TAG, "Socket listening");


        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        //do_retransmit(sock);

        int len;
        char rx_buffer[128];

        do{
        	send(sock, TAG, strlen(TAG), 0);
        	len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
			if (len < 0)
			{
				ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
			}
			else
			{
				rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
				ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
			}

        }while (len > 0);


        shutdown(sock, 0);
        close(sock);


	}



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
    xTaskCreate(&tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
}
