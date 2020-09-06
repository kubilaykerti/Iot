
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*
 * Task Handles
 */
TaskHandle_t Task_1Handle = NULL;
TaskHandle_t Task_2Handle = NULL;

/*
 * Queue Handles
 */
QueueHandle_t queue_1 = NULL;


/*
 * Task Functions Prototypes
 */
void Task_1(void *arg);
void Task_2(void *arg);

/*
 * Main Function
 */
void app_main(void)
{
	/*
	 * Queue Create
	 */
	queue_1 = xQueueCreate(10, sizeof(uint16_t));

	if(queue_1 == NULL)
	{
		printf("error queue creating.\n");
	}

	/*
	 * Task Create
	 */
	xTaskCreate(Task_1, "Task_1", 4096, NULL, 10, &Task_1Handle);
	xTaskCreate(Task_2, "Task_2", 4096, NULL, 10, &Task_2Handle);

	if((Task_1Handle == NULL) || (Task_2Handle == NULL))
	{
		printf("error task creating.\n");
	}


	for(;;)
	{
		printf("main...\n");
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}

}


/*
 * Task Functions
 */
void Task_1(void *arg)
{
	uint16_t send = 0;

	for(;;)
	{
		send++;
		xQueueSend(queue_1, &send, 0);
		printf("task_1...\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

}

void Task_2(void *arg)
{
	uint16_t receive = 0;
	for(;;)
	{
		xQueueReceive(queue_1, &receive, portMAX_DELAY);
		printf("task_2...%d \n" , receive);
	}

}
