CC = gcc -Wall -std=c11 -ggdb

compile:
	$(CC) -o program main.c

compile_nftw:
	$(CC) -o program_nftw main_nftw.c

run:
	make compile
	./program ./testDir -atime 0 -max-depth 1
	./program ./testDir -atime 0 -max-depth 3
	./program ./testDir -mtime +1
	./program ./testDir -mtime +1 -max-depth 4
	./program ./testDir -atime -5 -max-depth 5

run_nftw:
	make compile_nftw
	./program_nftw ./testDir -atime 0 -max-depth 1
	./program_nftw ./testDir -atime 0 -max-depth 3
	./program_nftw ./testDir -mtime +1
	./program_nftw ./testDir -mtime +1 -max-depth 4
	./program_nftw ./testDir -atime -5 -max-depth 5

zad2:
	make run
	make run_nftw
