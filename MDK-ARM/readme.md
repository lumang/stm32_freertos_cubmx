# 定时器
在cubemx中，提供了定时器功能，可以设置定时器，并且可以设置定时器回调函数。
# sfud_port.c
```c

#include"main.h"
// 配置SPI 开关
#define SPI_Start() (HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port,W25QXX_CS_Pin,GPIO_PIN_RESET))
#define SPI_Stop() (HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port,W25QXX_CS_Pin,GPIO_PIN_SET))
static char log_buf[256];
extern SPI_HandleTypeDef hspi1;// spi1

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;

    /**
     * add your spi write and read code
     */
					SPI_Start();
                    if (write_size > 0) {
                        HAL_SPI_Transmit(&hspi1, (uint8_t*)write_buf, write_size, 1000);
                    }
                    if (read_size > 0) {
                        HAL_SPI_Receive(&hspi1, (uint8_t*)read_buf, read_size, 1000);
                    }
					SPI_Stop();

    return result;
}
// 新增延时函数
void delay(void) {
    uint16_t count = 10000;
    while (count--) {
        __NOP();
    }
}
sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required 必备
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable 必备
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required 必备
     */
    flash->spi.wr = spi_write_read;
    flash->retry.delay = delay;
    flash->retry.times = 10000;
    return result;
}
```
