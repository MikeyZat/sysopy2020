
cc = gcc -Wall -ggdb

all: compile

compile:
	$(cc) main.c -o program
	$(cc) test.c -o test
	$(cc) do.c -o do

clean:
	rm do test program

test: all
	./program