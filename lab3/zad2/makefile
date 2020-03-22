CC = gcc -Wall -std=c11 -ggdb

clean:
	rm -f *.txt lista macierz support
	rm -r .tmp

macierz:
	$(CC) macierz.c -o macierz

support:
	$(CC) support.c -o support

run:
	./macierz lista 3 10 2

create_list:
	./support 1 10

test:
	./support test


zad_2:
	make macierz
	make support
	make create_list
	make run
	make test