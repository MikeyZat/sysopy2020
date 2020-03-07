CC = gcc -Wall -std=c11 -ggdb


all: static shared clean

static:
	make cleanAll
	$(CC) -c library.c
	ar rcs library.a library.o

shared:
	make cleanAll
	$(CC) -c -fPIC library.c
	$(CC) -shared -fPIC -o library.so library.o

clean:
	rm -f *.o

cleanAll:
	rm -f *.o *.a *.so

zad2_static:
	make cleanAll
	make static
	$(CC) -o program main.c library.a -I .
	make tests

zad2_shared:
	make cleanAll
	make shared
	$(CC) -o program main.c -L. library.so -I .
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