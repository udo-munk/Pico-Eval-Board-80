/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024-2025 by Udo Munk & Thomas Eberhardt
 *
 * This is the main program for a Raspberry Pico (W) board,
 * substitutes z80core/simmain.c
 *
 * History:
 * 28-APR-2024 implemented first release of Z80 emulation
 * 09-MAY-2024 test 8080 emulation
 * 27-MAY-2024 add access to files on MicroSD
 * 28-MAY-2024 implemented boot from disk images with some OS
 * 31-MAY-2024 use USB UART
 * 09-JUN-2024 implemented boot ROM
 * 11-JUN-2024 ported to Pico Eval Board
 */

/* Raspberry SDK and FatFS includes */
#include <stdio.h>
#include <string.h>
#if LIB_PICO_STDIO_USB || LIB_STDIO_MSC_USB
#include <tusb.h>
#endif
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#ifdef RASPBERRYPI_PICO_W
#include "pico/cyw43_arch.h"
#endif
#include "net_vars.h"

#include "ff.h"
#include "hw_config.h"
#include "my_rtc.h"

/* Project includes */
#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcfg.h"
#include "simmem.h"
#include "simcore.h"
#include "simport.h"
#include "simio.h"
#ifdef WANT_ICE
#include "simice.h"
#endif
#include "disks.h"
#include "lcd.h"
#include "rgbled.h"

#define SWITCH_BREAK 2	/* switch we use to interrupt the system (User Key) */
#define WS2812_PIN 4	/* pin with the RGB LED */

#define BS  0x08 /* ASCII backspace */
#define DEL 0x7f /* ASCII delete */

/* CPU speed */
int speed = CPU_SPEED;

/* PIO and sm used for RGB LED */
PIO pio = pio1;
uint sm;

/*
 * callback for TinyUSB when terminal sends a break
 * stops CPU
 */
#if LIB_PICO_STDIO_USB || (LIB_STDIO_MSC_USB && !STDIO_MSC_USB_DISABLE_STDIO)
void tud_cdc_send_break_cb(uint8_t itf, uint16_t duration_ms)
{
	UNUSED(itf);
	UNUSED(duration_ms);

	cpu_error = USERINT;
	cpu_state = ST_STOPPED;
}
#endif

/*
 * interrupt handler for break switch
 * stops CPU
 */
void gpio_callback(uint gpio, uint32_t events)
{
	UNUSED(gpio);
	UNUSED(events);

	cpu_error = USERINT;
	cpu_state = ST_STOPPED;
}

int main(void)
{
	char s[2];
	uint32_t rgb = 0x005500;

	stdio_init_all();	/* initialize stdio */
#if LIB_STDIO_MSC_USB
	sd_init_driver();	/* initialize SD card driver */
	tusb_init();		/* initialize TinyUSB */
	stdio_msc_usb_init();	/* initialize MSC USB stdio */
#endif
	time_init();		/* initialize FatFS RTC */

	/*
	 * initialize hardware AD converter, enable onboard
	 * temperature sensor and select its channel
	 */
	adc_init();
	adc_set_temp_sensor_enabled(true);
	adc_select_input(4);

	/* initialize LCD */
	lcd_init();

	/* setupt interrupt for break switch */
	gpio_init(SWITCH_BREAK);
	gpio_set_dir(SWITCH_BREAK, GPIO_IN);
	gpio_pull_up(SWITCH_BREAK);
	gpio_set_irq_enabled_with_callback(SWITCH_BREAK, GPIO_IRQ_EDGE_FALL,
					   true, &gpio_callback);

	/* initialize RGB LED */
	sm = pio_claim_unused_sm(pio, true);
	uint offset = pio_add_program(pio, &ws2812_program);
	ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, true);
	put_pixel(rgb); /* red */

#if LIB_PICO_STDIO_UART
	uart_inst_t *my_uart = uart_default;
	/* destroy random input from UART after activation */
	if (uart_is_readable(my_uart))
		getchar();
#endif

	/* when using USB UART wait until it is connected */
	/* but also get out if there is input at default UART */
#if LIB_PICO_STDIO_USB || LIB_STDIO_MSC_USB
	lcd_wait_term();
	while (!tud_cdc_connected()) {
#if LIB_PICO_STDIO_UART
		if (uart_is_readable(my_uart)) {
			getchar();
			break;
		}
#endif
		rgb = rgb - 0x000100;	/* while waiting make */
		if (rgb == 0)		/* RGB LED fading */
			rgb = 0x005500;
		put_pixel(rgb);
		sleep_ms(50);
}
#endif
	put_pixel(0x000044); /* blue */

	/* print banner */
	lcd_banner();
	printf("\fZ80pack release %s, %s\n", RELEASE, COPYR);
	printf("%s release %s\n", USR_COM, USR_REL);
	printf("running on ARM Cortex-M0+ cores at %i MHz\n", SYS_CLK_MHZ);
	printf("%s\n\n", USR_CPR);

	init_cpu();		/* initialize CPU */
	PC = 0xff00;		/* power on jump into the boot ROM */
	init_disks();		/* initialize disk drives */
	init_memory();		/* initialize memory configuration */
	init_io();		/* initialize I/O devices */

	read_config();          /* read configuration from MicroSD */

#ifdef RASPBERRYPI_PICO_W	/* initialize Pico W hardware */
	/* initialize Pico W WiFi hardware */
	if (cyw43_arch_init())
		panic("CYW43 init failed\n");

	/* try to connect WiFi */
	int wifi_retry = 5;	/* max WiFi connect retries */
	int result;

	cyw43_arch_enable_sta_mode();
	printf("connecting to WiFi... ");
	if (!strlen(wifi_ssid)) {
		puts("no WiFi SSID configured.");
		goto wifi_done;
	}
	while (wifi_retry) {
		if ((result = cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password,
		    CYW43_AUTH_WPA2_AES_PSK, 30000))) {
			printf("retry... ");
			wifi_retry--;
		} else
			break;
	}

	if (result) {
		puts("failed.");
	} else {
		puts("connected.");
		//do_ntp();
	}
wifi_done:
#endif

	config();		/* configure the machine */
	save_config();          /* save configuration on MicroSD */

	f_value = speed;	/* setup speed of the CPU */
	if (f_value)
		tmax = speed * 10000;	/* theoretically */
	else
		tmax = 100000;	/* for periodic CPU accounting updates */


	put_pixel(0x440000);	/* LED green */
	multicore_launch_core1(lcd_task); /* start LCD task on core 1 */

	/* run the CPU with whatever is in memory */
#ifdef WANT_ICE
	extern void ice_cmd_loop(int);

	ice_cmd_loop(0);
#else
	run_cpu();
#endif

	put_pixel(0x000000);	/* LED off */
	exit_disks();		/* stop disk drives */

#if defined(RASPBERRYPI_PICO_W) || defined(RASPBERRYPI_PICO2_W)
	/* de-initialize Pico W WiFi hardware */
	cyw43_arch_disable_sta_mode();
	cyw43_arch_deinit();
#endif

#ifndef WANT_ICE
	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	report_cpu_stats();	/* print some execution statistics */
#endif
	puts("\nPress any key to restart CPU");
	get_cmdline(s, 2);

	lcd_exit();		/* LCD off */
	multicore_reset_core1();/* stop core 1 */

	/* reset machine */
	watchdog_reboot(0, 0, 0);
	while (true) {
		__nop();
	}
}

/*
 * Read an ICE or config command line of maximum length len - 1
 * from the terminal. For single character requests (len == 2),
 * returns immediately after input is received.
 */
int get_cmdline(char *buf, int len)
{
	int i = 0;
	char c;

	for (;;) {
		c = getchar();
		if ((c == BS) || (c == DEL)) {
			if (i >= 1) {
				putchar(BS);
				putchar(' ');
				putchar(BS);
				i--;
			}
		} else if (c != '\r') {
			if (i < len - 1) {
				buf[i++] = c;
				putchar(c);
				if (len == 2)
					break;
			}
		} else {
			break;
		}
	}
	buf[i] = '\0';
	putchar('\n');
	return 0;
}
