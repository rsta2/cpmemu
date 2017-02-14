#
# Makefile
#

-include Config.mk

CIRCLEHOME ?= ../circle

OBJS	= main.o kernel.o \
	  z80computer.o z80.o z80memory.o z80ports.o \
	  console.o ramdisk.o multicore.o

LIBS	= $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a

EXTRACLEAN = cpmdisk

all: cpmdisk

include $(CIRCLEHOME)/app/Rules.mk

TARGET	?= kernel

all: $(TARGET).img

cpmdisk: cpmdisk.c
	gcc -Wall -O -o $@ $<
