CC = gcc -Wall -std=c11 -ggdb

cleanFiles:
	rm -f dane dane_c_lib dane_c_sys dane2 dane2_c_lib dane2_c_sys

compile:
	$(CC) -o program main.c

generate:
	./program generate dane 1000 15
	./program generate dane2 2000 15


zad1:
	make cleanFiles
	make compile
	make tests_1
	make tests_4
	make tests_512
	make tests_1024
	make tests_4096
	make tests_8192

tests_1:
	./program generate dane 25000 1
	./program generate dane2 50000 1
	./program copy dane dane_c_lib 25000 1 lib copy dane dane_c_sys 25000 1 sys sort dane_c_lib 25000 1 lib sort dane_c_sys 25000 1 sys
	./program copy dane2 dane2_c_lib 50000 1 lib copy dane2 dane2_c_sys 50000 1 sys sort dane2_c_lib 50000 1 lib sort dane2_c_sys 50000 1 sys
	make cleanFiles

tests_4:
	./program generate dane 25000 4
	./program generate dane2 50000 4
	./program copy dane dane_c_lib 25000 4 lib copy dane dane_c_sys 25000 4 sys sort dane 25000 4 lib sort dane_c_sys 25000 4 sys
	./program copy dane2 dane2_c_lib 50000 4 lib copy dane2 dane2_c_sys 50000 4 sys sort dane2_c_lib 50000 4 lib sort dane2_c_sys 50000 4 sys
	make cleanFiles

tests_512:
	./program generate dane 2000 512
	./program generate dane2 4000 512
	./program copy dane dane_c_lib 2000 512 lib copy dane dane_c_sys 2000 512 sys sort dane 2000 512 lib sort dane_c_sys 2000 512 sys
	./program copy dane2 dane2_c_lib 4000 512 lib copy dane2 dane2_c_sys 4000 512 sys sort dane2_c_lib 4000 512 lib sort dane2_c_sys 4000 512 sys
	make cleanFiles

tests_1024:
	./program generate dane 2000 1024
	./program generate dane2 4000 1024
	./program copy dane dane_c_lib 2000 1024 lib copy dane dane_c_sys 2000 1024 sys sort dane 2000 1024 lib sort dane_c_sys 2000 1024 sys
	./program copy dane2 dane2_c_lib 4000 1024 lib copy dane2 dane2_c_sys 4000 1024 sys sort dane2 4000 1024 lib sort dane2_c_sys 4000 1024 sys
	make cleanFiles

tests_4096:
	./program generate dane 2000 4096
	./program generate dane2 4000 4096
	./program copy dane dane_c_lib 2000 4096 lib copy dane dane_c_sys 2000 4096 sys sort dane 2000 4096 lib sort dane_c_sys 2000 4096 sys
	./program copy dane2 dane2_c_lib 4000 4096 lib copy dane2 dane2_c_sys 4000 4096 sys sort dane2 4000 4096 lib sort dane2_c_sys 4000 4096 sys
	make cleanFiles


tests_8192:
	./program generate dane 2000 8192
	./program generate dane2 4000 8192
	./program copy dane dane_c_lib 2000 8192 lib copy dane dane_c_sys 2000 8192 sys sort dane 2000 8192 lib sort dane_c_sys 2000 8192 sys
	./program copy dane2 dane2_c_lib 4000 8192 lib copy dane2 dane2_c_sys 4000 8192 sys sort dane2 4000 8192 lib sort dane2_c_sys 4000 8192 sys
	make cleanFiles