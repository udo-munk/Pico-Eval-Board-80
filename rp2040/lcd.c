/*
 * Functions for using the Pico-Eval-Board LCD
 *
 * Copyright (c) 2024 Udo Munk & Thomas Eberhardt
 */

#include <time.h>
#include "hardware/adc.h"
#include "hardware/divider.h"

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "dazzler.h"

#include "lcd.h"
#include "LCD_GUI.h"
#include "fonts.h"

static volatile bool do_refresh = true;
static volatile bool refresh_stopped = false;
volatile bool first_flag = true;

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
	LCD_Exit();
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
 * while core 0 is running the VM at the same time
 */

static inline float read_onboard_temp(void)
{
	/* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
	const float conversionFactor = 3.3f / (1 << 12);

	float adc = (float) adc_read() * conversionFactor;
	float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

	return tempC;
}

static void __not_in_flash_func(lcd_show_time)(void)
{
	time_t Time;
	struct tm *t;
	DEV_TIME dt;
	int temp;
	divmod_result_t res;

	if (first_flag) {
		GUI_DisString(10, 10, "Time", &Font24, BLACK, WHITE);
		GUI_DisString(300, 10, "Temp", &Font24, BLACK, WHITE);
		GUI_DisChar(460, 10, 'C', &Font24, BLACK, WHITE);
		GUI_DisChar(406, 10, '.', &Font24, BLACK, BLUE);
		GUI_DrawLine(0, 50, 479, 50, GRAY, LINE_SOLID,
			     DOT_PIXEL_2X2);
	}

	/* update time */
	time(&Time);
	t = localtime(&Time);
	dt.Hour = t->tm_hour;
	dt.Min = t->tm_min;
	dt.Sec = t->tm_sec;
	GUI_Showtime(85, 10, 240, 35, &dt, BLUE);

	/* update temperature */
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

static const char *__not_in_flash("hex_table") hex = "0123456789ABCDEF";
static inline char hex3(uint16_t x) { return hex[(x >> 12) & 0xf]; }
static inline char hex2(uint16_t x) { return hex[(x >> 8) & 0xf]; }
static inline char hex1(uint16_t x) { return hex[(x >> 4) & 0xf]; }
static inline char hex0(uint16_t x) { return hex[x & 0xf]; }

static void __not_in_flash_func(lcd_show_cpu)(void)
{
	if (first_flag) {
		GUI_DrawRectangle(10, 60, 140, 155, GRAY, DRAW_FULL,
				  DOT_PIXEL_1X1);
		GUI_DisString(15, 65, "PC", &Font24, GRAY, WHITE);
		GUI_DisString(15, 95, "SP", &Font24, GRAY, WHITE);
		GUI_DisString(15, 125, "AF", &Font24, GRAY, WHITE);
		GUI_DrawRectangle(150, 60, 280, 155, GRAY, DRAW_FULL,
				  DOT_PIXEL_1X1);
		GUI_DisString(155, 65, "BC", &Font24, GRAY, WHITE);
		GUI_DisString(155, 95, "DE", &Font24, GRAY, WHITE);
		GUI_DisString(155, 125, "HL", &Font24, GRAY, WHITE);
		first_flag = false;
	}

	GUI_DisChar(60, 65, hex3(PC), &Font24, BROWN, BLUE);
	GUI_DisChar(77, 65, hex2(PC), &Font24, BROWN, BLUE);
	GUI_DisChar(94, 65, hex1(PC), &Font24, BROWN, BLUE);
	GUI_DisChar(111, 65, hex0(PC), &Font24, BROWN, BLUE);

	GUI_DisChar(60, 95, hex3(SP), &Font24, BROWN, BLUE);
	GUI_DisChar(77, 95, hex2(SP), &Font24, BROWN, BLUE);
	GUI_DisChar(94, 95, hex1(SP), &Font24, BROWN, BLUE);
	GUI_DisChar(111, 95, hex0(SP), &Font24, BROWN, BLUE);

	GUI_DisChar(60, 125, hex1(A), &Font24, BROWN, BLUE);
	GUI_DisChar(77, 125, hex0(A), &Font24, BROWN, BLUE);
	GUI_DisChar(94, 125, hex1(F), &Font24, BROWN, BLUE);
	GUI_DisChar(111, 125, hex0(F), &Font24, BROWN, BLUE);

	GUI_DisChar(195, 65, hex1(B), &Font24, BROWN, BLUE);
	GUI_DisChar(212, 65, hex0(B), &Font24, BROWN, BLUE);
	GUI_DisChar(229, 65, hex1(C), &Font24, BROWN, BLUE);
	GUI_DisChar(246, 65, hex0(C), &Font24, BROWN, BLUE);

	GUI_DisChar(195, 95, hex1(D), &Font24, BROWN, BLUE);
	GUI_DisChar(212, 95, hex0(D), &Font24, BROWN, BLUE);
	GUI_DisChar(229, 95, hex1(E), &Font24, BROWN, BLUE);
	GUI_DisChar(246, 95, hex0(E), &Font24, BROWN, BLUE);

	GUI_DisChar(195, 125, hex1(H), &Font24, BROWN, BLUE);
	GUI_DisChar(212, 125, hex0(H), &Font24, BROWN, BLUE);
	GUI_DisChar(229, 125, hex1(L), &Font24, BROWN, BLUE);
	GUI_DisChar(246, 125, hex0(L), &Font24, BROWN, BLUE);
}

#define LCD_REFRESH 5 /* in ticks/frames per second */
#define LCD_REFRESH_US (1000000 / LCD_REFRESH)

void __not_in_flash_func(lcd_task)(void)
{
	absolute_time_t t;
	int64_t d;
	int ticks = 0;

	/* loops every LCD_REFRESH_US */
	while (do_refresh) {
		t = get_absolute_time();

		if (!dazzler_state) {
			/* update time/temperature once a second */
			if (first_flag) {
				GUI_Clear(BLACK);
				lcd_show_time();
			} else if (ticks == 0) {
				lcd_show_time();
			}

			/* update VM CPU registers on each run */
			lcd_show_cpu();
		} else {
			dazzler_draw();
		}

		d = absolute_time_diff_us(t, get_absolute_time());
		if (d < LCD_REFRESH_US)
			sleep_us(LCD_REFRESH_US - d);
		//else
		//	puts("REFRESH!");

		if (ticks++ >= LCD_REFRESH)
			ticks = 0;
	}

	/* task stopped refreshing the LCD */
	refresh_stopped = true;
}
