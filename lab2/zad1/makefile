CC = gcc -Wall -std=c11 -ggdb

cleanFiles:
	rm -f dane dane_c_lib dane_c_sys dane2 dane2_c_lib dane2_c_sys

compile:
	$(CC) -o program main.c

tests_1:
	./program generate dane 100 1
	./program generate dane2 200 1
	./program copy dane dane_c_lib 100 1 lib copy dane dane_c_sys 100 1 sys sort dane 100 1 lib sort dane_c_sys 100 1 sys2
	./program copy dane2 dane2_c_lib 100 1 lib copy dane2 dane2_c_sys 100 1 sys sort dane2 100 1 lib sort dane2_c_sys 100 1 sys2
	make cleanTxt

tests_4:
	./program generate dane 100 4
	./program generate dane2 200 4
	./program copy dane dane_c_lib 100 4 lib copy dane dane_c_sys 100 4 sys sort dane 100 4 lib sort dane_c_sys 100 4 sys2
	./program copy dane2 dane2_c_lib 100 4 lib copy dane2 dane2_c_sys 100 4 sys sort dane2 100 4 lib sort dane2_c_sys 100 4 sys2
	make cleanTxt

tests_512:
	./program generate dane 100 512
	./program generate dane2 200 512
	./program copy dane dane_c_lib 100 512 lib copy dane dane_c_sys 100 512 sys sort dane 100 512 lib sort dane_c_sys 100 512 sys2
	./program copy dane2 dane2_c_lib 100 512 lib copy dane2 dane2_c_sys 100 512 sys sort dane2 100 512 lib sort dane2_c_sys 100 512 sys2
	make cleanTxt

tests_1024:
	./program generate dane 100 1024
	./program generate dane2 200 1024
	./program copy dane dane_c_lib 100 1024 lib copy dane dane_c_sys 100 1024 sys sort dane 100 1024 lib sort dane_c_sys 100 1024 sys2
	./program copy dane2 dane2_c_lib 100 1024 lib copy dane2 dane2_c_sys 100 1024 sys sort dane2 100 1024 lib sort dane2_c_sys 100 1024 sys2
	make cleanTxt

tests_4096:
	./program generate dane 100 4096
	./program generate dane2 200 4096
	./program copy dane dane_c_lib 100 4096 lib copy dane dane_c_sys 100 4096 sys sort dane 100 4096 lib sort dane_c_sys 100 4096 sys2
	./program copy dane2 dane2_c_lib 100 4096 lib copy dane2 dane2_c_sys 100 4096 sys sort dane2 100 4096 lib sort dane2_c_sys 100 4096 sys2
	make cleanTxt


tests_8192:
	./program generate dane 100 8192
	./program generate dane2 200 8192
	./program copy dane dane_c_lib 100 8192 lib copy dane dane_c_sys 100 8192 sys sort dane 100 8192 lib sort dane_c_sys 100 8192 sys2
	./program copy dane2 dane2_c_lib 100 8192 lib copy dane2 dane2_c_sys 100 8192 sys sort dane2 100 8192 lib sort dane2_c_sys 100 8192 sys2
	make cleanTxt