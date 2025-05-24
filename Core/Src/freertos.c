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
#include"SEGGER_RTT.h"
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
 osThreadId myTaskPrintHandle ;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTaskLEDHandle;
osTimerId myTimer01Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void printTask(void  const*para); 
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTaskLED(void const * argument);
void Callback01(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

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
void vTimerCallback(TimerHandle_t xTimer1)
{
	sprintf(pcToPrint,"Hello FreeRTOS! timer1\r\n");
	xQueueSend(xQueuePrint,pcToPrint,portMAX_DELAY);
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

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

  /* Create the timer(s) */
  /* definition and creation of myTimer01 */
  osTimerDef(myTimer01, Callback01);
  myTimer01Handle = osTimerCreate(osTimer(myTimer01), osTimerPeriodic, NULL);
  
  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  //  xTimer = xTimerCreate("Timer", 1000 / portTICK_PERIOD_MS, pdTRUE, (void *) 0, vTimerCallback);// 创建一个定时器，定时器周期为1000ms，定时器模式为周期性定时器
  //xTimerStart(myTimer01Handle, 0);// 启动定时器
   osTimerStart(myTimer01Handle,1000);
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
		SEGGER_RTT_printf(0,pcToPrint);
		SEGGER_RTT_printf(0, "Hello, SEGGER RTT! led flash\r\n");
		xQueueSendToBack(xQueuePrint,pcToPrint,0);
  }
  /* USER CODE END StartTaskLED */
}

/* Callback01 function */
void Callback01(void const * argument)
{
  /* USER CODE BEGIN Callback01 */
		sprintf(pcToPrint,"Hello FreeRTOS! timer1\r\n");
	  xQueueSend(xQueuePrint,pcToPrint,portMAX_DELAY);
  /* USER CODE END Callback01 */
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
		for(int i=0;i<10;i++)
		{
			printf("%x\n",pcToWrite[i]);
		}
	}
}
/* USER CODE END Application */

