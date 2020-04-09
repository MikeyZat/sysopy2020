CC = gcc -Wall -std=c11 -ggdb

compile:
	make client
	make producent
	make tester

client:
	$(CC) client.c -o client

producent:
	$(CC) producent.c -o producent

tester:
	$(CC) tester.c -o tester

generate:
	echo "1111111111111111111111111" > first.txt
	echo "2222222222222222222222222" > second.txt
	echo "3333333333333333333333333" > third.txt
	echo "4444444444444444444444444" > fourth.txt
	echo "5555555555555555555555555" > fifth.txt

first_test:
	rm -f FIFO_3
	mkfifo FIFO_3
	echo "abcdefghaasdjaosjdasoawheuahdaodsa\nasodjasodashdasohdasoh\noasduhasdiuasi\niajsdoasoasd\n12312312321" > text_file.txt
	echo "\nNow open producent in another terminal\n"
	./client FIFO_3 output.txt 50

second_test:
	make generate
	./tester

clean:
	rm -f producent client tester FIFO_3 MULTIPLE_FIFO *.txt