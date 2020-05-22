CC = gcc -Wall -std=c11 -ggdb -pthread

src = $(wildcard *.c)
obj = $(src:.c=.o)

all: server client
	./server 3000 /tmp/socket

%.o: %.c
	$(CC) -c $< -o $@

client: client.o
	$(CC) $^ -o $@

server: server.o
	$(CC) $^ -o $@

clean:
	rm -f $(obj) client server