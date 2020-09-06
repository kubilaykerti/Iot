
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * Task Handles
 */

TaskHandle_t Task_1Handle = NULL;
TaskHandle_t Task_2Handle = NULL;

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
	xTaskCreate(Task_1, "Task_1", 4096, NULL, 10, &Task_1Handle);
	xTaskCreate(Task_2, "Task_2", 4096, NULL, 10, &Task_2Handle);

	for(;;)
	{
		printf("main...\n");
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

}


/*
 * Task Functions
 */
void Task_1(void *arg)
{
	for(;;)
	{
		printf("task_1...\n");
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

}

void Task_2(void *arg)
{
	for(;;)
	{
		printf("task_2...\n");
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

}
