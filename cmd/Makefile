# Copyright (c) 2012, Jan Vaughan
# All rights reserved.
#
# Makefile for command line tool that interacts with blinker.

USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs` -framework CoreFoundation

CFLAGS	= $(USBFLAGS) -O -Wall

all: toggle

.c.o:
	gcc $(CFLAGS) -c $<

toggle: toggle.o
	gcc -o toggle toggle.o $(USBLIBS) -lflags

clean:
	rm -f *.o toggle
