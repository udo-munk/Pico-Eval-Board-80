/*
 * Functions for using the Pico-Eval-Board LCD from the emulation
 *
 * Copyright (c) 2024 Udo Munk
 */

#include "sim.h"
#include "simdefs.h"
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

void lcd_wait_term(void)
{
	GUI_Clear(BLACK);
	GUI_DisString(70, 150, "Waiting for terminal", &Font24, BLACK, RED);
}

void lcd_banner(void)
{
	GUI_Clear(BLACK);
	GUI_DisString(70, 80, "Z80pack release " RELEASE, &Font24,
			 BLACK, WHITE);
	GUI_DisString(70, 105, USR_COM " "  USR_REL, &Font24,
			 BLACK, WHITE);
}
