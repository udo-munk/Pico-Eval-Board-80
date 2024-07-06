/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk & Thomas Eberhardt
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
#if LIB_PICO_STDIO_USB
#include <tusb.h>
#endif
#include "pico/stdlib.h"
#include "pico/time.h"

#include "ff.h"
#include "hw_config.h"

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

#define SWITCH_BREAK 2 /* switch we use to interrupt the system (User Key) */

#define BS  0x08 /* backspace */
#define DEL 0x7f /* delete */

/* CPU speed */
int speed = CPU_SPEED;

/*
 * interrupt handler for break switch
 * stops CPU
 */
void gpio_callback(uint gpio, uint32_t events)
{
	cpu_error = USERINT;
	cpu_state = STOPPED;
}

int main(void)
{
	char s[2];

	stdio_init_all();	/* initialize Pico stdio */

	/* initialize LCD */
	lcd_init();

	gpio_init(SWITCH_BREAK); /* setupt interrupt for break switch */
	gpio_set_dir(SWITCH_BREAK, GPIO_IN);
	gpio_pull_up(SWITCH_BREAK);
	gpio_set_irq_enabled_with_callback(SWITCH_BREAK, GPIO_IRQ_EDGE_FALL,
					   true, &gpio_callback);

	/* when using USB UART wait until it is connected */
#if LIB_PICO_STDIO_USB
	while (!tud_cdc_connected())
		sleep_ms(100);
#endif

	/* print banner */
	lcd_banner();
	printf("\fZ80pack release %s, %s\n", RELEASE, COPYR);
	printf("%s release %s\n", USR_COM, USR_REL);
	printf("%s\n\n", USR_CPR);

	init_cpu();		/* initialize CPU */
	init_disks();		/* initialize disk drives */
	init_memory();		/* initialize memory configuration */
	init_io();		/* initialize I/O devices */
	config();		/* configure the machine */

	/* setup speed of the CPU */
	f_flag = speed;
	tmax = speed * 10000; /* theoretically */

	/* power on jump into the boot ROM */
	PC = 0xff00;

	/* run the CPU with whatever is in memory */
#ifdef WANT_ICE
	extern void ice_cmd_loop(int);

	ice_cmd_loop(0);
#else
	run_cpu();
#endif

        exit_disks();		/* stop disk drives */

#ifndef WANT_ICE
	putchar('\n');
	report_cpu_error();	/* check for CPU emulation errors and report */
	report_cpu_stats();	/* print some execution statistics */
#endif
	puts("\nPress any key to restart CPU");
	get_cmdline(s, 2);

	stdio_flush();
	return 0;
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
