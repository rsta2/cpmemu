#
# Makefile
#

-include Config.mk

CIRCLEHOME ?= ../circle

OBJS	= main.o kernel.o z80computer.o z80emu.o z80memory.o z80ports.o console.o ramdisk.o

LIBS	= $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a

EXTRACLEAN = maketables tables.h cpmdisk

all: cpmdisk

include $(CIRCLEHOME)/app/Rules.mk

TARGET	?= kernel

all: $(TARGET).img

z80emu.o: tables.h

tables.h: maketables.c
	gcc -Wall -o maketables $<
	./maketables > $@

cpmdisk: cpmdisk.c
	gcc -Wall -O -o $@ $<
