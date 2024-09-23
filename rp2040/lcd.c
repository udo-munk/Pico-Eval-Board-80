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
	LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT;

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
	GUI_DisString(70, 160, "Z80pack release " RELEASE, &Font24,
		      BLACK, WHITE);
	GUI_DisString(70, 185, USR_COM " "  USR_REL, &Font24,
		      BLACK, WHITE);
	GUI_DisString(40, 220, "by Udo Munk & Thomas Eberhardt", &Font20,
		      BLACK, WHITE);
	GUI_DrawCircle(100, 80, 50, BLUE, DRAW_FULL, DOT_PIXEL_1X1);
	GUI_DisString(75, 70, "Z80", &Font24, BLUE, WHITE);
	GUI_DrawRectangle(250, 30, 400, 120, BLUE, DRAW_FULL, DOT_PIXEL_1X1);
	GUI_DisString(290, 70, "8080", &Font24, BLUE, WHITE);
}
