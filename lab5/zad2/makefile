
CC = gcc -Wall -std=c11 -ggdb

compile:
	$(CC) main.c -o program

test:
	@ps -aux | head > text_file.txt
	./program text_file.txt

clean:
	rm -f program text_file