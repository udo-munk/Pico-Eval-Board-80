/*
 * Functions for using the Pico-Eval-Board LCD
 *
 * Copyright (c) 2024 Udo Munk
 */

#include <time.h>

#include "sim.h"
#include "simdefs.h"
#include "lcd.h"
#include "LCD_GUI.h"
#include "fonts.h"

static volatile bool do_refresh = true;
static volatile bool refresh_stopped = false;

/*
 * these functions are called from the application running on core 0
 */

void lcd_init(void)
{
	LCD_SCAN_DIR lcd_scan_dir = SCAN_DIR_DFT;

	/* initialize hardware */
	System_Init();

	/* initialize LCD, set orientation and backlight */
	LCD_Init(lcd_scan_dir, 1000);
}

void lcd_exit(void)
{
	/* tell LCD task to stop drawing */
	do_refresh = false;
	/* wait until it stopped */
	while (refresh_stopped == false)
		;
	GUI_Clear(BLACK);
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

/*
 * these functions are called from the lcd task running on core 1
 */

static void lcd_show_time(void)
{
	static bool first_call = true;
	time_t Time;
	struct tm *t;
	DEV_TIME dt;

	if (first_call) {
		GUI_DisString(10, 10, "Time", &Font24, BLACK, WHITE);
		first_call = false;
	}

	time(&Time);
	t = localtime(&Time);
	dt.Hour = t->tm_hour;
	dt.Min = t->tm_min;
	dt.Sec = t->tm_sec;

	GUI_Showtime(80, 10, 235, 35, &dt, BLUE);
}

void lcd_task(void)
{
	GUI_Clear(BLACK);

	while (do_refresh) {
		lcd_show_time();
		sleep_ms(1000);
	}

	refresh_stopped = true;
}
