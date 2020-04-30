CC = gcc -Wall -std=c11 -ggdb

src = $(wildcard *.c)
obj = $(src:.c=.o)

all: main creator packer reshipper
	./main

%.o: %.c
	$(CC) -c $< -o $@

main: main.o
	$(CC) $^ -o $@

creator: creator.o
	$(CC) $^ -o $@

packer: packer.o
	$(CC) $^ -o $@

reshipper: reshipper.o
	$(CC) $^ -o $@

clean:
	rm -f $(obj) main creator packer reshipper