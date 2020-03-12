CC = gcc -Wall -std=c11 -ggdb


all: static shared clean

static:
	make cleanAll
	$(CC) -c library.c
	ar rcs library.a library.o

shared:
	make cleanAll
	$(CC) -c -fPIC library.c
	$(CC) -shared -o library.so library.o

clean:
	rm -f *.o

cleanAll:
	rm -f *.o *.a *.so

zad3_static_opt0:
	make static
	$(CC) -O -o program main.c library.a -I .
	make tests

zad3_shared_opt0:
	make shared
	$(CC) -O -o program main.c -L. library.o -I .
	make tests

zad3_dynamic_opt0:
	make shared
	$(CC) -O -o program main_d.c -ldl -I .
	make tests

zad3_static_opt2:
	make static
	$(CC) -O2 -o program main.c library.a -I .
	make tests

zad3_shared_opt2:
	make shared
	$(CC) -O2 -o program main.c -L. library.o -I .
	make tests

zad3_dynamic_opt2:
	make shared
	$(CC) -O2 -o program main_d.c -ldl -I .

zad3_static_opt3:
	make static
	$(CC) -O3 -o program main.c library.a -I .
	make tests

zad3_shared_opt3:
	make shared
	$(CC) -O3 -o program main.c -L. library.o -I .
	make tests

zad3_dynamic_opt3:
	make shared
	$(CC) -O3 -o program main_d.c -ldl -I .

zad3_static_opt0s:
	make static
	$(CC) -Os -o program main.c library.a -I .
	make tests

zad3_shared_opt0s:
	make shared
	$(CC) -Os -o program main.c -L. library.o -I .
	make tests

zad3_dynamic_opt0s:
	make shared
	$(CC) -Os -o program main_d.c -ldl -I .
	make tests

tests:
	make small
	make medium
	make big
	make simillar
	make different
	make addAndRemove

small:
	./program create_table 1 compare_pairs small.txt:small1.txt create_blocks small.txt:small1.txt remove_block 0

medium:
	./program create_table 1 compare_pairs medium.txt:medium1.txt create_blocks medium.txt:medium1.txt remove_block 0

big:
	./program create_table 1 compare_pairs big.txt:big1.txt create_blocks big.txt:big1.txt remove_block 0

simillar:
	./program create_table 1 compare_pairs e.txt:almost_e.txt create_blocks e.txt:almost_e.txt remove_block 0

different:
	./program create_table 1 compare_pairs c.txt:not_c.txt create_blocks c.txt:not_c.txt remove_block 0

addAndRemove:
	./program create_table 5 compare_pairs big.txt:big1.txt create_blocks big.txt:big1.txt remove_block 0 \
	compare_pairs big.txt:big1.txt create_blocks c.txt:not_c.txt remove_block 0 \
	compare_pairs medium.txt:medium1.txt create_blocks c.txt:not_c.txt \
	compare_pairs e.txt:almost_e.txt \
	compare_pairs c.txt:not_c.txt \
	create_blocks e.txt:almost_e.txt \
	create_blocks c.txt:not_c.txt \
	remove_block 0 \
	remove_block 2 \
	remove_block 1 \