UISYSTEM=$(shell uname)

ifeq ($(UISYSTEM),Darwin)
  UIINCDIR = -I/opt/local/include -I/opt/local/include/SDL 
  UILIBS = -L/opt/local/lib -lSDLmain -lSDL -lSDL_image -framework Cocoa
  CB_LIBS=-framework Cocoa  -framework CoreAudio -framework QTKit -framework CoreAudioKit -framework Foundation -framework QuartzCore -framework QuickTime  -framework QuartzCore
  CB_CFLAGS= -O3 -std=c99
  CB_MMFLAGS=-O3
  CB_DEVOBJS=voice.o sight.o isight/CSGCamera.o isight/CSGImage.o
#  CB_MMFLAGS=-g	
#  CB_CFLAGS= -g -std=c99
else
  UINCDIR = 
  UILIBS = -lSDL
  CB_DEVOBJS=
  CB_CFLAGS= -O3 -Wno-unused-result
endif

CFLAGS := -O3 $(UIINCDIR)
libs = $(UILIBS)

all: cbserver

.PHONY: all

joystick.o: joystick.c
	gcc $(CFLAGS) -c joystick.c

joystick: joystick.o
	gcc $(CFLAGS) -o $@ joystick.o $(libs)

cbserver: cbserver.o aserial.o net.o ${CB_DEVOBJS}
	g++ $(CB_CFLAGS) -o $@ cbserver.o aserial.o net.o ${CB_DEVOBJS} $(CB_LIBS)

isight/CSGCamera.o:
	make -C isight

isight/CSGImage.o:
	make -C isight

test_cbserver: cbserver.o aserial.o net.o voice.o sight.o 
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
	make -C isight
