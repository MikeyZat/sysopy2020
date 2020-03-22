CC = gcc -Wall -std=c11 -ggdb

clean:
	rm -f *.txt lista macierz support
	rm -r .tmp

compile_macierz:
	rm -f macierz
	$(CC) macierz.c -o macierz

compile_support:
	rm -f support
	$(CC) support.c -o support

run:
	./macierz lista 3 10 2

create_list:
	./support 1 10

test:
	./support test


zad_2:
	make compile_macierz
	make compile_support
	make create_list
	make run
	make test