/*
 * Functions for using the Pico-Eval-Board LCD
 *
 * Copyright (c) 2024 Udo Munk
 */

#include <time.h>
#include "hardware/adc.h"
#include "hardware/divider.h"

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
		sleep_ms(20);
	GUI_Clear(BLACK);
	System_Exit();
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

static inline float read_onboard_temp(void)
{
	/* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
	const float conversionFactor = 3.3f / (1 << 12);

	float adc = (float) adc_read() * conversionFactor;
	float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

	return tempC;
}

static void lcd_show_time(void)
{
	static bool first_call = true;
	time_t Time;
	struct tm *t;
	DEV_TIME dt;
	int temp;
	divmod_result_t res;


	if (first_call) {
		GUI_DisString(10, 10, "Time", &Font24, BLACK, WHITE);
		GUI_DisString(300, 10, "Temp", &Font24, BLACK, WHITE);
		GUI_DisChar(460, 10, 'C', &Font24, BLACK, WHITE);
		GUI_DisChar(406, 10, '.', &Font24, BLACK, BLUE);
		GUI_DrawLine(0, 50, 479, 50, GRAY, LINE_SOLID,
			     DOT_PIXEL_2X2);
		first_call = false;
	}

	time(&Time);
	t = localtime(&Time);
	dt.Hour = t->tm_hour;
	dt.Min = t->tm_min;
	dt.Sec = t->tm_sec;
	GUI_Showtime(85, 10, 240, 35, &dt, BLUE);

	GUI_DrawRectangle(423, 10, 457, 34, FONT_BACKGROUND,
			  DRAW_FULL, DOT_PIXEL_1X1);
	GUI_DrawRectangle(372, 10, 406, 34, FONT_BACKGROUND,
			  DRAW_FULL, DOT_PIXEL_1X1);
	temp = (int) (read_onboard_temp() * 100.0f + 0.5f);
	res = hw_divider_divmod_u32(temp, 10);
	GUI_DisChar(440, 10, '0' + to_remainder_u32(res), &Font24,
		    BLACK, BLUE);
	res = hw_divider_divmod_u32(to_quotient_u32(res), 10);
	GUI_DisChar(423, 10, '0' + to_remainder_u32(res), &Font24,
		    BLACK, BLUE);
	res = hw_divider_divmod_u32(to_quotient_u32(res), 10);
	GUI_DisChar(389, 10, '0' + to_remainder_u32(res), &Font24,
		    BLACK, BLUE);
	res = hw_divider_divmod_u32(to_quotient_u32(res), 10);
	GUI_DisChar(372, 10, '0' + to_remainder_u32(res), &Font24,
		    BLACK, BLUE);
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
