# z80pack for Pico-Eval-Board

Lately z80pack got better support for bare metall systems, like the
RP2040 microcontroller designed at Raspberry Pi. A dev prototype system
with a Pi Pico was build on breadboard and used as a research system.
While this is good for development, not everyone wants to wire a system
on breadboard with breakout boards for the tiny circuits. This also is
sensitive for moving arround and transporting, so the circuit design was
used for a somehwat more durable system, that also is easier to build.

For the Raspberry Pi Pico board a number of extension boards and headers
exist, the one used here is this:

	https://www.waveshare.com/wiki/Pico-Eval-Board

Just plugin one of the Pi Pico boards into it, connect USB to some PC and
use a terminal emulation to connect to the machine. Before you do prepare
a MicroSD card with the following directories:

	CODE80
	DISKS80
	CONF80

Copy the *.bin program files from src-examples into CODE80.
Copy the *.dsk disk immage files from disks into DISKS80.
In CONF80 create an empty text file with the name cfg.txt.
