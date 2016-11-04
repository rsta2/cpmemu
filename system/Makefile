#
# Makefile
#

BIN = os2ccp.bin os3bdos.bin bios.bin

all: system.bin shutdown.com

system.bin: hex2bin $(BIN)
	cat $(BIN) > $@

shutdown.com: hex2bin shutdown.hex

hex2bin: hex2bin.c
	gcc -Wall -o $@ $<

%.bin: %.hex
	./hex2bin $< $@

%.com: %.hex
	./hex2bin $< $@

clean:
	rm -f $(BIN) system.bin shutdown.com hex2bin
