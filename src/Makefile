UISYSTEM=$(shell uname)

ifeq ($(UISYSTEM),Darwin)
  UIINCDIR = -I/opt/local/include -I/opt/local/include/SDL 
  UILIBS = -L/opt/local/lib -lSDLmain -lSDL -lSDL_image -framework Cocoa
  CB_LIBS=-framework Cocoa  -framework CoreAudio -framework QTKit
  CB_CFLAGS= -O3 -std=c99
  CB_MMFLAGS=-O3
//  CB_CFLAGS= -g
else
  UINCDIR = 
  UILIBS = -lSDL
endif

CFLAGS := -O3 $(UIINCDIR)
libs = $(UILIBS)

all: cbserver

.PHONY: all

joystick.o: joystick.c
	gcc $(CFLAGS) -c joystick.c

joystick: joystick.o
	gcc $(CFLAGS) -o $@ joystick.o $(libs)

cbserver: cbserver.o aserial.o net.o voice.o sight.o
	g++ $(CB_CFLAGS) -o $@ cbserver.o aserial.o net.o voice.o sight.o $(CB_LIBS)

cbserver.o: cbserver.cpp net.h arduino-serial.h
	g++ -c $(CB_CFLAGS)  cbserver.cpp

aserial.o: arduino-serial.c arduino-serial.h
	gcc -c $(CB_CFLAGS) -D__LIBRARY__ arduino-serial.c -o aserial.o 

net.o: net.c net.h
	gcc -c $(CB_CFLAGS) net.c

voice.o: 
	gcc -c $(CB_MMFLAGS) voice.mm

sight.o:
	gcc -c $(CB_MMFLAGS) sight.mm

clean:
	rm *.o cbserver joystick