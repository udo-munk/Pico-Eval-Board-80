/*
 * Functions for using the Pico-Eval-Board LCD from the emulation
 *
 * Copyright (c) 2024 Udo Munk
 */

#ifndef LCD_H
#define LCD_H

#include "DEV_Config.h"
#include "LCD_Driver.h"

extern void lcd_init(void), lcd_exit(void), lcd_task(void);
extern void lcd_wait_term(void), lcd_banner(void);

#endif
