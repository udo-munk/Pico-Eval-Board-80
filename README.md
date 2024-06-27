# z80pack on Pico-Eval-Board

Lately z80pack got better support for bare metal systems, like the
RP2040 microcontroller designed at Raspberry Pi. A dev prototype system
with a Pi Pico was build on breadboard and used as a research system.
While this is good for development, not everyone wants to wire a system
on breadboard with breakout boards for the tiny circuits. This also is
sensitive for moving arround and transportation, so we need something
more convenient.

For the Raspberry Pi Pico board a number of extension boards and headers
exist, the one used here is this:

	https://www.waveshare.com/pico-eval-board.htm
	https://www.waveshare.com/wiki/Pico-Eval-Board

To install z80pack on this device:

1. clone z80pack: git clone https://github.com/udo-munk/z80pack.git
2. checkout dev branch: cd z80pack; git checkout dev; cd ..
3. clone this: git clone https://github.com/udo-munk/Pico-Eval-Board-80.git

To build the application:

	cd Pico-Eval-Board-80
	mkdir build
	cd build
	cmake ..
	make

Flash picosim.uf2 into the device, and then prepare a MicroSD card.

In the root directory of the card create these directories:

	CONF80
	CODE80
	DISKS80

Into the CODE80 directory copy all the .bin files from src-examples.
Into the DISKS80 directory copy the disk images from disks.
