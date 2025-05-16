/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include<stdio.h>
#include"queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
 xQueueHandle xQueuePrint;// 消息队列句柄
 static char pcToPrint[80];
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTaskLEDHandle;
osThreadId myTaskPrintHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void printTask(void  const*para); 
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTaskLED(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
 xQueuePrint = xQueueCreate(2,sizeof(pcToPrint));
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTaskLED */
  osThreadDef(myTaskLED, StartTaskLED, osPriorityNormal, 0, 128);
  myTaskLEDHandle = osThreadCreate(osThread(myTaskLED), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	  osThreadDef(myTaskPrint, printTask, osPriorityNormal, 0, 128);
  myTaskPrintHandle = osThreadCreate(osThread(myTaskPrint), NULL);

  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskLED */
/**
* @brief Function implementing the myTaskLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskLED */
void StartTaskLED(void const * argument)
{
  /* USER CODE BEGIN StartTaskLED */
	uint16_t cnt = 0;
	TickType_t xFirstTime;
  /* Infinite loop */
  for(;;)
  {
		xFirstTime = xTaskGetTickCount();// 进入点的系统节拍
    osDelay(1000);
		HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		cnt = xTaskGetTickCount()-xFirstTime;
		sprintf(pcToPrint,"task 1 %3d  cnt\r\n",cnt);// 
		printf("led flash\n");
		xQueueSendToBack(xQueuePrint,pcToPrint,0);
  }
  /* USER CODE END StartTaskLED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
 void printTask(void const *arg)
{
	char pcToWrite[80];
	while(1)
	{
		xQueueReceive(xQueuePrint,pcToWrite,portMAX_DELAY);
		printf("pc print %s\n",pcToWrite);
	}
}
/* USER CODE END Application */

