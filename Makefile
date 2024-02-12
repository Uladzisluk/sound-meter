CC = gcc
CFLAGS = -Wall
LDFLAGS = -lm

all: sound_meter

sound_meter: main.o
	$(CC) $(CFLAGS) -o $@ $^ -lasound $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f sound_meter *.o
