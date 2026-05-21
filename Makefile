CC = gcc
CFLAGS = -Wall -Wextra -O2

all: lga test

lga: main.o physics.o io.o
	$(CC) $(CFLAGS) -o lga main.o physics.o io.o

test: test.o physics.o io.o
	$(CC) $(CFLAGS) -o run_tests test.o physics.o io.o

main.o: main.c lga.h
	$(CC) $(CFLAGS) -c main.c

physics.o: physics.c lga.h
	$(CC) $(CFLAGS) -c physics.c

io.o: io.c lga.h
	$(CC) $(CFLAGS) -c io.c

test.o: test.c lga.h
	$(CC) $(CFLAGS) -c test.c

clean:
	rm -f *.o lga run_tests grid.bin frame_*.ppm