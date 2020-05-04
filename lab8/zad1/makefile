CC = gcc -Wall -std=c11 -ggdb -pthread

src = $(wildcard *.c)
obj = $(src:.c=.o)

all: main
	./main 1 sign apollonian_gasket.ascii.pgm histogram
	./main 2 sign apollonian_gasket.ascii.pgm histogram
	./main 4 sign apollonian_gasket.ascii.pgm histogram
	./main 8 sign apollonian_gasket.ascii.pgm histogram
	./main 1 block apollonian_gasket.ascii.pgm histogram
	./main 2 block apollonian_gasket.ascii.pgm histogram
	./main 4 block apollonian_gasket.ascii.pgm histogram
	./main 8 block apollonian_gasket.ascii.pgm histogram
	./main 1 interleaved apollonian_gasket.ascii.pgm histogram
	./main 2 interleaved apollonian_gasket.ascii.pgm histogram
	./main 4 interleaved apollonian_gasket.ascii.pgm histogram
	./main 8 interleaved apollonian_gasket.ascii.pgm histogram

%.o: %.c
	$(CC) -c $< -o $@

main: main.o
	$(CC) $^ -o $@

clean:
	rm -f $(obj) main
	rm -f histogram