# z80pack on Pico-Eval-Board

Lately z80pack got better support for bare metal systems, like the
Pi Pico microcontroller designed at Raspberry Pi. A dev prototype system
with a Pi Pico was build on breadboard and used as a research system.
While this is good for development, not everyone wants to wire a system
on breadboard with breakout boards for the tiny circuits. This also is
sensitive for moving around and transportation, so we need something
more convenient.

For the Raspberry Pi Pico board a number of extension boards and headers
exist, the one used here is this:

[Waveshare Evaluation board product page](https://www.waveshare.com/pico-eval-board.htm)
and
[Waveshare Pico Eval Board Wiki](https://www.waveshare.com/wiki/Pico-Eval-Board)

To install z80pack on this device:

1. clone z80pack: git clone https://github.com/udo-munk/z80pack.git
2. checkout dev branch: cd z80pack; git checkout dev; cd ..
3. clone this: git clone https://github.com/udo-munk/Pico-Eval-Board-80.git

To build the application for a Pi Pico RP2040 board:

	cd Pico-Eval-Board-80/rp2040
	mkdir build
	cd build
	cmake .. -G "Unix Makefiles"
	make

This device also can be used with a Pi Pico 2 RP2350 board, then build with:

	cd Pico-Eval-Board-80/rp2350
	mkdir build
	cd build
	cmake .. -G "Unix Makefiles"
	make

Flash picosim.uf2 into the device, and then prepare a MicroSD card.

In the root directory of the card create these directories:

	CONF80
	CODE80
	DISKS80

Into the CODE80 directory copy all the .bin files from src-examples.
Into the DISKS80 directory copy the disk images from disks.
CONF80 is used to save the configuration, nothing more to do there,
the directory must exist though.


Here a few pictures how Z80pack running on the device looks like:

![image](https://github.com/udo-munk/Pico-Eval-Board-80/blob/main/resources/terminal.jpg "waiting for terminal connection")
