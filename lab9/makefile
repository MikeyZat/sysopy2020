CC = gcc -Wall -std=c11 -ggdb -pthread

src = $(wildcard *.c)
obj = $(src:.c=.o)

all: main
	./main 5 15

%.o: %.c
	$(CC) -c $< -o $@

main: main.o
	$(CC) $^ -o $@

clean:
	rm -f $(obj) main
