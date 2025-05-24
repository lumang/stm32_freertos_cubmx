/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include"W25Qxx.h"
#include<stdio.h>
#include<string.h>
#include"timers.h"//��ʱ��
#include "sfud.h"
#include<assert.h>
#include"SEGGER_RTT.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#pragma pack(push, 1) // ���ýṹ�����
typedef struct {
    char date[11];      // ���ڣ�YYYY-MM-DD
    char time[9];       // ʱ�䣺HH:MM:SS
    char type[8];       // ���ͣ�INFO/WARN/ERROR
    char content[128];  // ��־����
} LogEntry;
#pragma pack(pop)      // �ָ�Ĭ�����
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t wData[0x100];// д
uint8_t rData[0x100];// ��
uint8_t ID[4];// dev_id
uint32_t i;
#define MAX_LOG_ENTRIES 10 // ����̫�� ���� c6t6\c6t6.axf: Error: L6406E: No space in execution regions with .ANY selector matching main.o(.bss).
LogEntry logs[MAX_LOG_ENTRIES];
uint16_t log_count = 0; // ��־������
sfud_flash *flash = NULL;//  sfud �豸����
uint32_t log_write_addr = 0x0000; // ��ʼд���ַ
#define SFUD_W25Q64_SECTOR_SIZE 4096 // SFUD W25Q64 ������С
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ϵͳ�Լ� 
void SystemCheck(void)
{
  // �ƹ���˸3��
  for(i=0;i<3;i++)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
    printf("SystemCheck %d \r\n",i);
    HAL_Delay(5000);
  }
}
// sfud����
void es(void) {
  printf("����һ����������\r\n");
  const sfud_flash* flash = sfud_get_device(0);
  sfud_erase(flash, 0, 4096);
}
void ws(void) {
  printf("д������\r\n");
  const sfud_flash* flash = sfud_get_device(0);
  uint8_t buff[100] = {0};
  for (uint8_t i = 0; i < 100; i++) {
      buff[i] = i;
  }
  sfud_erase_write(flash, 0, 100, buff);
}
void rs(void) {
  printf("��ȡ����\r\n");
  const sfud_flash* flash = sfud_get_device(0);
  uint8_t buff[100] = {0};
  sfud_read(flash, 0, 100, buff);
  for (uint8_t i = 0; i < 100; i++) {
      printf("%d ", buff[i]);
      if (i % 10 == 0) {
          printf("\r\n");
      }
  }
}
const uint32_t LOG_START_ADDR = 0x000000; // ��־�洢��ʼ��ַ
#define MAX_LOGS        6000    // �����־��Ŀ��
// sfud����
//��Flash��ȡ������־���ڴ�
// �����Ŀ�Ƿ�Ϊ�գ�ȫ0xFF��
bool is_entry_empty(const LogEntry *entry) {
    const uint8_t *bytes = (const uint8_t *)entry;
    for (size_t i = 0; i < sizeof(LogEntry); i++) {
        if (bytes[i] != 0xFF) return false;
    }
    return true;
}
void log_read_all() {
    if (flash == NULL) {
        printf("[ERROR] Flash�豸δ��ʼ��!\n");
        return;
    }

    uint32_t addr = LOG_START_ADDR;
    log_count = 0;

    while (addr < flash->chip.capacity && log_count < 2) {
        LogEntry entry;
        sfud_err result = sfud_read(flash, addr, sizeof(LogEntry), (uint8_t *)&entry);

        if (result != SFUD_SUCCESS) {
            printf("[ERROR] ��ַ0x%08X��ȡʧ��: %d\n", addr, result);
            addr += sizeof(LogEntry);
            continue;
        }

        // �������ԭʼ���ݣ���ѡ��
        // printf("��ȡ��ַ: 0x%08X, ����: %s, ����: %s\n", addr, entry.date, entry.type);

        if (!is_entry_empty(&entry)) {
            // ����ַ�����ֹ��
            entry.date[sizeof(entry.date) - 1] = '\0';
            entry.time[sizeof(entry.time) - 1] = '\0';
            entry.type[sizeof(entry.type) - 1] = '\0';
            entry.content[sizeof(entry.content) - 1] = '\0';

            // �������ڴ�
            memcpy(&logs[log_count], &entry, sizeof(LogEntry));
            log_count++;
        }

        addr += sizeof(LogEntry);
    }

    printf("�ɹ���ȡ %d ����־\n", log_count);
}

// ���Ժ�������ӡ��һ����־����
void debug_first_log() {
    if (log_count == 0) {
        printf("����Ч��־!\n");
        return;
    }

    printf("��һ����־����:\n");
    printf("����: %s\n", logs[0].date);
    printf("ʱ��: %s\n", logs[0].time);
    printf("����: %s\n", logs[0].type);
    printf("����: %s\n", logs[0].content);
}
void log_init() {
    // ��ʼ�� SFUD
    sfud_init();
    flash = sfud_get_device(0);

    // ��� Flash �Ƿ�֧�ֲ�����д��
    //assert(flash != NULL && sfud_is_available(flash));
    
    // �� Flash ��ȡ�ϴε�д���ַ����־û��洢���˴��򻯣�
    // ʵ����Ŀ�пɽ� log_write_addr �洢�� Flash �Ĺ̶�λ��
}
void log_erase_sector(uint32_t addr) {
    sfud_err result = sfud_erase(flash, addr, SFUD_W25Q64_SECTOR_SIZE);
	  printf("0 �ɹ� erase sucess %d\n",result);
    //assert(result == SFUD_SUCCESS);
}
void log_write_entry(LogEntry *entry) {
    // ��ʼ��������Ϊȫ 0xFF
    uint8_t buffer[sizeof(LogEntry)];
    memset(buffer, 0xFF, sizeof(buffer));
    
    // ������Ч���ݵ�������
    memcpy(buffer, entry, sizeof(LogEntry));
    
    // д�� Flash
    sfud_err result = sfud_write(flash, log_write_addr, sizeof(buffer), buffer);
    if (result != SFUD_SUCCESS) {
        printf("Error writing log to Flash at 0x%08X\n", log_write_addr);
        return;
    }
    
    // ����д���ַ
    log_write_addr += sizeof(LogEntry);
}

#if 0
void log_read_all() {
    uint32_t addr = 0;
    log_count = 0;

    while (addr < log_write_addr && log_count < MAX_LOG_ENTRIES) {
        // �� Flash ��ȡһ����־
        sfud_read(flash, addr, sizeof(LogEntry), (uint8_t *)&logs[log_count]);
        
        // ����δд������ȫ 0xFF��
        if (logs[log_count].date[0] != 0xFF) {
            log_count++;
        }
        addr += sizeof(LogEntry);
    }
}
#endif 
//#define MAX_LOG_ENTRIES 1000  // �����־��Ŀ��
//LogEntry logs[MAX_LOG_ENTRIES];
//uint16_t log_count = 0;       // ʵ�ʶ�ȡ����־����
#include <stdbool.h>

// �����Ŀ�Ƿ�Ϊ�գ�ȫ 0xFF��
bool is_entry_empty2(const LogEntry *entry) {
    const uint8_t *bytes = (const uint8_t *)entry;
    for (size_t i = 0; i < sizeof(LogEntry); i++) {
        if (bytes[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

void log_read_all2() {
    uint32_t addr = 0x000000; // ��־�洢��ʼ��ַ
    log_count = 0;            // ���ü�����
    
    // ������־�洢����
    while (addr < log_write_addr && log_count < MAX_LOG_ENTRIES) {
        // �� Flash ��ȡһ����־
        sfud_err result = sfud_read(flash, addr, sizeof(LogEntry), (uint8_t *)&logs[log_count]);
        if (result != SFUD_SUCCESS) {
            printf("Error reading log at 0x%08X\n", addr);
            addr += sizeof(LogEntry); // ����������Ŀ
            continue;
        }
        
        // ��ͳ�Ʒǿ���Ŀ
        if (!is_entry_empty(&logs[log_count])) {
            log_count++;
        }
        
        // �ƶ�����һ����־λ��
        addr += sizeof(LogEntry);
    }
    
    printf("Total logs read: %d\n", log_count);
}
void filter_logs(const char *selected_date, const char *selected_type) {
    //lv_list_clean(log_list); // ��յ�ǰ�б�
    printf("\r\n--- Filtered Logs (Date: %s, Type: %s) ---\r\n", selected_date, selected_type);
    
    for (int i = 0; i < log_count; i++) {
        bool date_match = (strcmp(selected_date, "All") == 0) || 
                          (strcmp(logs[i].date, selected_date) == 0);
        bool type_match = (strcmp(selected_type, "All") == 0) || 
                          (strcmp(logs[i].type, selected_type) == 0);
        
        if (date_match && type_match) {
            // ��ʾ�� LVGL �б�
            //lv_obj_t *entry = lv_list_add_btn(log_list, NULL, "");
            //lv_obj_t *label = lv_label_create(entry, NULL);
            //lv_label_set_text_fmt(label, "[%s][%s] %s", 
            //                      logs[i].date, logs[i].type, logs[i].content);
            
            // ���������
            printf("[%s][%s] %s\r\n", 
                   logs[i].date, logs[i].type, logs[i].content);
        }
    }
    printf("--- End of Logs ---\r\n");
}
void log_write_and_refresh(LogEntry *entry) {
    log_write_entry(entry);      // д������־
    log_read_all();              // ���¶�ȡ��־
    filter_logs("All", "All");   // ˢ�½�����ʾ
}
// ��ӡ��һ����־��ԭʼ�ֽڣ�ʮ�����ƣ�
void debug_log_entry(const LogEntry *entry) {
    const uint8_t *bytes = (const uint8_t *)entry;
    printf("Raw data of LogEntry:\r\n");
    for (size_t i = 0; i < sizeof(LogEntry); i++) {
        printf("%02X ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\r\n");
    }
    printf("\r\n");
}

// �� log_read_and_print �е���
//debug_log_entry(&logs[0]);
void test_log_read() {
    // �����־�洢��
    log_write_addr = 0x000000;
    sfud_erase(flash, 0x000000, SFUD_W25Q64_SECTOR_SIZE);
    
    // д����Ч��־
    LogEntry entry1 = {
        .date = "2023-10-01",
        .time = "12:00:00",
        .type = "INFO",
        .content = "System started."
    };
    log_write_entry(&entry1);
    
    // д��һ������Ŀ��ȫ 0xFF��
    LogEntry entry2;
    memset(&entry2, 0xFF, sizeof(LogEntry));
    log_write_entry(&entry2);
    
    // ��ȡ��־
    log_read_all();
    // �� log_read_and_print �е���  ����ʹ��
    debug_log_entry(&logs[0]);
		printf("Test fields:\r\n");
    printf("date:  %s (len=%d)\r\n", logs[0].date, strlen(logs[0].date));
    printf("type:  %s (len=%d)\r\n", logs[0].type, strlen(logs[0].type));
    printf("content:  %s (len=%d)\r\n", logs[0].content, strlen(logs[0].content));
    // ��֤���
    if (log_count == 1 && strcmp(logs[0].date, "2023-10-01") == 0) {
        printf("Test passed: 1 valid log found.\n");
			// ���������
        printf("test [%s][%s] %s \r\n", logs[0].date, logs[0].type, logs[0].content);
    } else {
        printf("Test failed: Expected 1 log, found %d.\n", log_count);
    }
}
uint32_t log_start_addr = 0x000000; // ��־�洢��ʼ��ַ
void log_read_and_print() {
    uint32_t addr = log_start_addr;
    log_count = 0;

    // ��ȡ������־���ڴ�
    while (addr < flash->chip.capacity && log_count < MAX_LOG_ENTRIES) {
        sfud_read(flash, addr, sizeof(LogEntry), (uint8_t *)&logs[log_count]);
        
        if (!is_entry_empty(&logs[log_count])) {
            log_count++;
        }
        addr += sizeof(LogEntry);
    }

    // ͨ�����ڸ�ʽ�����
    printf("\r\n--- Log Entries (Total: %d) ---\r\n", log_count);
    for (int i = 0; i < log_count; i++) {
        printf("[%s %s][%s] %s\r\n", 
               logs[i].date, 
               logs[i].time, 
               logs[i].type, 
               logs[i].content);
    }
    printf("--- End of Logs ---\r\n");
}
void log_read_and_print2() {
    uint32_t addr = log_start_addr;
    LogEntry entry;

    printf("\r\n--- Log Entries ---\r\n");
    while (addr < flash->chip.capacity) {
        sfud_read(flash, addr, sizeof(LogEntry), (uint8_t *)&entry);
        
        if (!is_entry_empty(&entry)) {
            printf("[%s %s][%s] %s\r\n", 
                   entry.date, entry.time, entry.type, entry.content);
        }
        addr += sizeof(LogEntry);
    }
    printf("--- End of Logs ---\r\n");
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  //sfud_init(); // sfud��ʼ��
  //SystemCheck();// ϵͳ�Լ�
	SEGGER_RTT_Init();// Init rtt tools
  #if 0
	//W25QXX cubemx blog
	//https://blog.csdn.net/lwb450921/article/details/124695575
	printf("spi w25qxx example\r\n");
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);// ����
  /*-Step1- ��֤�豸ID  ************************************************Step1*/ 
	BSP_W25Qx_Init();
	BSP_W25Qx_Read_ID(ID);
    //��һλ����ID�̶�0xEF,�ڶ�λ�豸ID����������ͬ,����Ϊ��
     //W25Q16Ϊ0x14��32Ϊ0x15��40Ϊ0x12��64Ϊ0x16��80Ϊ0x13��128Ϊ0x17
	if((ID[0] != 0xEF) | (ID[1] != 0x17)) 
	{                                
		printf("something wrong in Step1 \r\n");
	}
	// printer ID info
	{
		printf(" W25Qxx ID is : ");
		for(i=0;i<2;i++)
		{
			printf("0x%02X ",ID[i]);
		}
		printf("\r\n");
	}
	/*-Step2- ������  ************************************************Step2*/ 	
	if(BSP_W25Qx_Erase_Block(0) == W25Qx_OK)
		printf(" QSPI Erase Block OK!\r\n");
	else
		printf("something wrong in Step2\r\n");
	/*-Step3- д����  ************************************************Step3*/	
	for(i =0;i<0x100;i ++)
	{
			wData[i] = i;//д������
            rData[i] = 0; //����
	}
	//
	//  д������
	if(BSP_W25Qx_Write(wData,0x00,0x100)== W25Qx_OK)
		printf(" QSPI Write OK!\r\n");
	else
		printf("something wrong in Step3\r\n");
    /*-Step4- ������  ************************************************Step4*/	
	if(BSP_W25Qx_Read(rData,0x00,0x100)== W25Qx_OK)
		printf(" QSPI Read ok\r\n\r\n");
	else
		printf("something wrong in Step4\r\n");
	
	printf("QSPI Read Data : \r\n");
	for(i =0;i<0x100;i++)
		printf("0x%02X  ",rData[i]);
	printf("\r\n\r\n");
	/*-Step5- ���ݶԱ�  ************************************************Step5*/		
	if(memcmp(wData,rData,0x100) == 0 ) 
		printf(" W25Q64FV QuadSPI Test OK\r\n");
	else
		printf(" W25Q64FV QuadSPI Test False\r\n");
    #endif
    #if 0
		es();
    ws();
    rs();
		#endif 
		//log_init();//  ��ʼ��
		SEGGER_RTT_printf(0,"HELLO SEGGER RTT\r\n");
		#if 0
		 // ģ��д��һ������־
    LogEntry new_log;
    strncpy(new_log.date, "2023-10-01", sizeof(new_log.date));
    strncpy(new_log.time, "12:05:00", sizeof(new_log.time));
    strncpy(new_log.type, "INFO", sizeof(new_log.type));
    strncpy(new_log.content, "Sensor data: 25.3��C", sizeof(new_log.content));
		//log_write_and_refresh(&new_log);
		//log_read_all();              // ���¶�ȡ��־
    //filter_logs("All", "All");   // ˢ�½�����ʾ
		test_log_read();
		  
    // ��ȡ��־
    log_read_all();
    
    // ��֤���
    if (log_count == 1 && strcmp(logs[0].date, "2023-10-01") == 0) {
        printf("Test passed: 1 valid log found.\n");
			// ���������
            printf("test [%s][%s] %s \r\n", logs[0].date, logs[0].type, logs[0].content);
    } else {
        printf("Test failed: Expected 1 log, found %d.\n", log_count);
    }
		#endif 
		//log_read_and_print2();
		//test_log_read();  // �������
		 // ��ȡ��־���ڴ�
    //log_read_all();

    // �������
    //debug_first_log();

		//rs();
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
