CC=gcc
LDFLAGS=-framework Cocoa -framework CoreAudioKit -framework Foundation -framework QuartzCore -framework QuickTime -framework QuartzCore
OBJECTS=CSGCamera.o CSGImage.o main.o

BUNDLE=isight

$(BUNDLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BUNDLE) $(OBJECTS) $(LDFLAGS)

clean:
	rm -f *.o isight
