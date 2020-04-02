CC = gcc -Wall -ggdb

compile:
	make sender
	make catcher

sender:
	$(CC) sender.c -o sender

catcher:
	$(CC) catcher.c -o catcher

test:
	@echo "[TEST] sigrt"
	@./catcher sigrt&
	@echo "[CATCHER] Running"
	@./sender `pidof catcher` 50 sigrt

	@echo "[TEST] kill"
	@./catcher kill&
	@echo "[CATCHER] Running"
	@./sender `pidof catcher` 50 kill

	@echo "[TEST] sigqueue"
	@./catcher sigqueue&
	@echo "[CATCHER] Running"
	@./sender `pidof catcher` 50 sigqueue

clean:
	rm sender catcher