/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024-2025 by Udo Munk & Thomas Eberhardt
 *
 * This module configures the machine appropriate for the
 * Z80/8080 software we want to run on it.
 *
 * History:
 * 20-APR-2024 dummy, no configuration implemented yet
 * 12-MAY-2024 implemented configuration dialog
 * 27-MAY-2024 implemented load file
 * 28-MAY-2024 implemented mount/unmount of disk images
 * 03-JUN-2024 added directory list for code files and disk images
 * 24-MAY-2025 separate read/save config file from config and add network config
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "f_util.h"
#include "ff.h"

#include "LCD_Driver.h"

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simport.h"
#include "simio.h"
#include "simcfg.h"

#include "disks.h"
#include "picosim.h"
#if LIB_STDIO_MSC_USB
#include "stdio_msc_usb.h"
#endif

#include "net_vars.h"
#if defined(RASPBERRYPI_PICO_W) || defined(RASPBERRYPI_PICO2_W)
#include "net_ntp.h"
#endif

static datetime_t t = { .year = 2024, .month = 1, .day = 1, .dotw = 1,
			.hour = 0, .min = 0, .sec = 0 };
static int brightness = 1000;
static const char *cfg = "/CONF80/" CONF_FILE;

/*
 * prompt for a filename
 */
static void prompt_fn(char *s)
{
	printf("Filename: ");
	get_cmdline(s, FNLEN+1);
	while (*s) {
		*s = toupper((unsigned char) *s);
		s++;
	}
}

/*
 * get an integer with range check
 */
static int get_int(const char *prompt, const char *hint,
		   int min_val, int max_val)
{
	int i;
	char s[6];

	for (;;) {
		printf("Enter %s%s: ", prompt, hint);
		get_cmdline(s, 5);
		if (s[0] == '\0')
			return -1;
		i = atoi(s);
		if (i < min_val || i > max_val) {
			printf("Invalid %s: range %d - %d\n",
			       prompt, min_val, max_val);
		} else
			return i;
	}
}

/*
 * try to read config file
 */
void read_config(void)
{
	unsigned int br;

	sd_res = f_open(&sd_file, cfg, FA_READ);
	if (sd_res == FR_OK) {
		f_read(&sd_file, &cpu, sizeof(cpu), &br);
		f_read(&sd_file, &speed, sizeof(speed), &br);
		f_read(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_read(&sd_file, &brightness, sizeof(brightness), &br);
		f_read(&sd_file, &t, sizeof(t), &br);
		f_read(&sd_file, &wifi_ssid, sizeof(wifi_ssid), &br);
		f_read(&sd_file, &wifi_password, sizeof(wifi_password), &br);
		f_read(&sd_file, &ntp_server, sizeof(ntp_server), &br);
		f_read(&sd_file, &utc_offset, sizeof(utc_offset), &br);
		f_read(&sd_file, &disks[0], DISKLEN+1, &br);
		f_read(&sd_file, &disks[1], DISKLEN+1, &br);
		f_read(&sd_file, &disks[2], DISKLEN+1, &br);
		f_read(&sd_file, &disks[3], DISKLEN+1, &br);
		f_close(&sd_file);
	}
#if defined(EXCLUDE_I8080) || defined(EXCLUDE_Z80)
	cpu = DEF_CPU;
#endif
}

/*
 * try to save config file
 */
void save_config(void)
{
	unsigned int br;

	sd_res = f_open(&sd_file, cfg, FA_WRITE | FA_CREATE_ALWAYS);
	if (sd_res == FR_OK) {
		f_write(&sd_file, &cpu, sizeof(cpu), &br);
		f_write(&sd_file, &speed, sizeof(speed), &br);
		f_write(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_write(&sd_file, &brightness, sizeof(brightness), &br);
		f_write(&sd_file, &t, sizeof(t), &br);
		f_write(&sd_file, &wifi_ssid, sizeof(wifi_ssid), &br);
		f_write(&sd_file, &wifi_password, sizeof(wifi_password), &br);
		f_write(&sd_file, &ntp_server, sizeof(ntp_server), &br);
		f_write(&sd_file, &utc_offset, sizeof(utc_offset), &br);
		f_write(&sd_file, &disks[0], DISKLEN+1, &br);
		f_write(&sd_file, &disks[1], DISKLEN+1, &br);
		f_write(&sd_file, &disks[2], DISKLEN+1, &br);
		f_write(&sd_file, &disks[3], DISKLEN+1, &br);
		f_close(&sd_file);
	}
}

#if defined(RASPBERRYPI_PICO_W)
/*
 * network configuration for Pico W
 */
void net_config(void)
{
	bool quit = false;
	char s[WIFI_SSID_LEN+1];

	while (!quit) {
		printf("s - WiFi SSID: %s\n", wifi_ssid);
		printf("p - WiFi password: %s\n", wifi_password);
		printf("n - NTP server: %s\n", ntp_server);
		printf("u - UTC offset in hours: %d\n", utc_offset);
		printf("q - quit\n");

		printf("\nCommand: ");
		get_cmdline(s, 2);
		putchar('\n');

		switch (tolower((unsigned char) s[0])) {
		case 's':
			printf("Enter SSID: ");
			get_cmdline(s, WIFI_SSID_LEN+1);
			strcpy(wifi_ssid, s);
			putchar('\n');
			break;

		case 'p':
			printf("Enter password: ");
			get_cmdline(s, WIFI_PWD_LEN+1);
			strcpy(wifi_password, s);
			putchar('\n');
			break;

		case 'n':
			printf("Enter NTP server: ");
			get_cmdline(s, HOST_NAME_MAX+1);
			if (strlen(s))
				strcpy(ntp_server, s);
			else
				strcpy(ntp_server, DEFAULT_NTP);
			putchar('\n');
			break;

		case 'u':
			utc_offset = get_int("UTC offset", " (hours)", -12, +14);
			putchar('\n');
			break;

		case 'q':
			quit = true;
			break;
		}
	}
}
#endif

/*
 * Configuration dialog for the machine
 */
void config(void)
{
	const char *cpath = "/CODE80";
	const char *cext = "*.BIN";
	const char *dpath = "/DISKS80";
	const char *dext = "*.DSK";
	char s[FNLEN+1];
	int go_flag = 0;
	int i, n, menu;
	static const char *dotw[7] = { "Sun", "Mon", "Tue", "Wed",
				       "Thu", "Fri", "Sat" };

#if defined(RASPBERRYPI_PICO_W) || defined(RASPBERRYPI_PICO2_W)
	if (ntp_time) {
		puts("ntp time present, using it for setting the clock\n");
		time_t local_time = ntp_time + utc_offset * 60 * 60;
		struct tm *ntp_t = gmtime(&local_time);
		t.year = ntp_t->tm_year + 1900;
		t.month = ntp_t->tm_mon + 1;
		t.day = ntp_t->tm_mday;
		t.hour = ntp_t->tm_hour;
		t.min = ntp_t->tm_min;
		t.sec = ntp_t->tm_sec;
		t.dotw = ntp_t->tm_wday;
	}
#endif

	rtc_set_datetime(&t);
	sleep_us(64);

	LCD_SetBackLight(brightness);

	menu = 1;

	while (!go_flag) {
		if (menu) {
			rtc_get_datetime(&t);
			printf("Current time: %s %04d-%02d-%02d "
			       "%02d:%02d:%02d\n", dotw[t.dotw],
			       t.year, t.month, t.day, t.hour, t.min, t.sec);
#if defined(RASPBERRYPI_PICO_W)
			printf("n - network configuration\n");
#endif
			printf("b - LCD brightness: %d\n", brightness);
			printf("a - set date\n");
			printf("t - set time\n");
#if LIB_STDIO_MSC_USB
			printf("u - enable USB mass storage access\n");
#endif
			printf("c - switch CPU, currently %s\n",
			       (cpu == Z80) ? "Z80" : "8080");
			printf("s - CPU speed: ");
			if (speed == 0)
				puts("unlimited");
			else
				printf("%d MHz\n", speed);
			printf("p - Port 255 value: %02XH\n", fp_value);
			printf("f - list files\n");
			printf("r - load file\n");
			printf("d - list disks\n");
			printf("0 - Disk 0: %s\n", disks[0]);
			printf("1 - Disk 1: %s\n", disks[1]);
			printf("2 - Disk 2: %s\n", disks[2]);
			printf("3 - Disk 3: %s\n", disks[3]);
			printf("g - run machine\n\n");
		} else
			menu = 1;
		printf("Command: ");
		get_cmdline(s, 2);
		putchar('\n');

		switch (tolower((unsigned char) s[0])) {
#if defined(RASPBERRYPI_PICO_W)
		case 'n':
			net_config();
			break;
#endif

		case 'b':
			if ((i = get_int("brightness", "", 0, 1000)) >= 0) {
				brightness = i;
				LCD_SetBackLight(brightness);
			}
			putchar('\n');
			break;

		case 'a':
			n = 0;
			rtc_get_datetime(&t);
			if ((i = get_int("weekday", " (0=Sun)", 0, 6)) >= 0) {
				t.dotw = i;
				n++;
			}
			if ((i = get_int("year", "", 0, 4095)) >= 0) {
				t.year = i;
				n++;
			}
			if ((i = get_int("month", "", 1, 12)) >= 0) {
				t.month = i;
				n++;
			}
			if ((i = get_int("day", "", 1, 31)) >= 0) {
				t.day = i;
				n++;
			}
			if (n > 0) {
				rtc_set_datetime(&t);
				sleep_us(64);
			}
			putchar('\n');
			break;

		case 't':
			n = 0;
			rtc_get_datetime(&t);
			if ((i = get_int("hour", "", 0, 23)) >= 0) {
				t.hour = i;
				n++;
			}
			if ((i = get_int("minute", "", 0, 59)) >= 0) {
				t.min = i;
				n++;
			}
			if ((i = get_int("second", "", 0, 59)) >= 0) {
				t.sec = i;
				n++;
			}
			if (n > 0) {
				rtc_set_datetime(&t);
				sleep_us(64);
			}
			putchar('\n');
			break;

#if LIB_STDIO_MSC_USB
		case 'u':
			exit_disks();
			puts("Waiting for disk to be ejected");
			stdio_msc_usb_do_msc();
			puts("Disk ejected\n");
			init_disks();
			check_disks();
			break;
#endif

		case 'c':
			if (cpu == Z80)
				switch_cpu(I8080);
			else
				switch_cpu(Z80);
			break;

		case 's':
			i = get_int("speed", " in MHz (0=unlimited)", 0, 40);
			putchar('\n');
			if (i >= 0)
				speed = i;
			break;

		case 'p':
again:
			printf("Enter value in Hex: ");
			get_cmdline(s, 3);
			if (s[0]) {
				if (!isxdigit((unsigned char) s[0]) ||
				    !isxdigit((unsigned char) s[1])) {
					puts("Invalid value: range 00 - FF");
					goto again;
				}
				fp_value = (s[0] <= '9' ? s[0] - '0' :
					    toupper((unsigned char) s[0]) -
					    'A' + 10) << 4;
				fp_value += (s[1] <= '9' ? s[1] - '0' :
					     toupper((unsigned char) s[1]) -
					     'A' + 10);
			}
			putchar('\n');
			break;

		case 'f':
			list_files(cpath, cext);
			putchar('\n');
			menu = 0;
			break;

		case 'r':
			prompt_fn(s);
			if (s[0])
				load_file(s);
			putchar('\n');
			break;

		case 'd':
			list_files(dpath, dext);
			putchar('\n');
			menu = 0;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
			i = s[0] - '0';
			prompt_fn(s);
			if (s[0])
				mount_disk(i, s);
			else {
				disks[i][0] = '\0';
				putchar('\n');
			}
			break;

		case 'g':
			go_flag = 1;
			break;

		default:
			break;
		}
	}

}
