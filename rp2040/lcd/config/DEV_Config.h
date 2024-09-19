/*****************************************************************************
* | File      	: DEV_Config.h
* | Author      : Waveshare team
* | Function    : GPIO Functions
* | Info        : Provide the hardware underlying interface	 
*----------------
* | This version: V1.0
* | Date        : 2018-01-11
* | Info        : Basic version
******************************************************************************/
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "stdio.h"

#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

#define LCD_RST_PIN	15	
#define LCD_DC_PIN	8
#define LCD_CS_PIN	9
#define LCD_CLK_PIN	10
#define LCD_BKL_PIN	13
#define LCD_MOSI_PIN	11
#define LCD_MISO_PIN	12
#define TP_CS_PIN	16
#define TP_IRQ_PIN	17
#define SD_CS_PIN	22

#define SPI_PORT	spi1

static inline void DEV_Digital_Write(UWORD Pin, UBYTE Value)
{
	gpio_put(Pin, Value);
}

static inline UBYTE DEV_Digital_Read(UWORD Pin)
{
	return gpio_get(Pin);
}

static inline uint8_t SPI4W_Write_Byte(uint8_t value)
{   
	uint8_t rxDat;

	spi_write_read_blocking(SPI_PORT, &value, &rxDat, 1);

	return rxDat;
}

static inline uint8_t SPI4W_Read_Byte(uint8_t value)
{
	return SPI4W_Write_Byte(value);
}

extern void DEV_GPIO_Mode(UWORD Pin, UWORD Mode);
extern void DEV_GPIO_Init(void);

extern void System_Init(void);
extern void System_Exit(void);

#endif
