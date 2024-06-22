/*
 * Functions for using the Pico-Eval-Board LCD from the emulation
 *
 * Copyright (c) 2024 Udo Munk
 */

#include "lcd.h"
#include "LCD_GUI.h"
#include "fonts.h"

void lcd_init(void)
{
	LCD_SCAN_DIR  lcd_scan_dir = SCAN_DIR_DFT;

	/* initialize hardware */
	System_Init();

	/* initialize LCD, set orientation and backlight */
	LCD_Init(lcd_scan_dir, 1000);
}

void lcd_banner(void)
{
	GUI_Clear(BLACK);
	GUI_DisString_EN(50, 40, "Z80pack test", &Font24, BLACK, WHITE);
}
