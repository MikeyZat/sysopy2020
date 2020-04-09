
CC = gcc -Wall -std=c11 -ggdb

compile:
	$(CC) main.c -o program
	$(CC) program.c -o testProgram

test:
	@echo "echo 1 | ./testProgram 1 | ./testProgram 1 | ./testProgram 2 | ./testProgram 3 | ./testProgram 5" >> input.txt
	./program input.txt

clean:
	rm -f program testProgram input.txt