/*****************************************************************************
* | File      	: DEV_Config.c
* | Author      : Waveshare team
* | Function    : GPIO Functions
* | Info        : Provide the hardware underlying interface	 
*----------------
* | This version: V1.0
* | Date        : 2018-01-11
* | Info        : Basic version
******************************************************************************/
#include "DEV_Config.h"
#include "pico/stdlib.h"

void DEV_GPIO_Mode(UWORD Pin, UWORD Mode)
{
	gpio_init(Pin);
	if ((Mode == 0) || (Mode == GPIO_IN)) {
		gpio_set_dir(Pin, GPIO_IN);
	} else {
		gpio_set_dir(Pin, GPIO_OUT);
	}
}

void DEV_GPIO_Init(void)
{
	DEV_GPIO_Mode(LCD_RST_PIN, GPIO_OUT);
	DEV_GPIO_Mode(LCD_DC_PIN, GPIO_OUT);
	DEV_GPIO_Mode(LCD_CS_PIN, GPIO_OUT);
	DEV_GPIO_Mode(TP_CS_PIN, GPIO_OUT);
	DEV_GPIO_Mode(TP_IRQ_PIN, GPIO_IN);
	DEV_GPIO_Mode(SD_CS_PIN, GPIO_OUT);
	//gpio_set_pulls(TP_IRQ_PIN, true, false);

	DEV_Digital_Write(TP_CS_PIN, 1);
	DEV_Digital_Write(LCD_CS_PIN, 1);
	DEV_Digital_Write(SD_CS_PIN, 1);

	gpio_set_function(LCD_BKL_PIN, GPIO_FUNC_PWM);
}

/*******************************************************************************
function:	System Init
note:		Initialize the communication method
*******************************************************************************/
uint8_t System_Init(void)
{
	DEV_GPIO_Init();
	spi_init(SPI_PORT, 40 * 1000 * 1000);
	gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
	gpio_set_function(LCD_MISO_PIN, GPIO_FUNC_SPI);

	return 0;
}

void System_Exit(void)
{
}
