#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

int main()
{
  key_t key = ftok("main", 1);

  int semaphores = semget(key, 4, 0);
  int global_memory = shmget(key, sizeof(memory_type), 0);

  struct sembuf unlock_memory = {CAN_MODIFY_IDX, 1, 0};
  struct sembuf increment_space = {SPACE_IDX, 1, 0};
  struct sembuf ops_end[2] = {unlock_memory, increment_space};

  struct sembuf lock_memory = {CAN_MODIFY_IDX, -1, 0};
  struct sembuf decrement_packed = {PACKED_IDX, -1, 0};
  struct sembuf ops_start[2] = {lock_memory, decrement_packed};

  while (LOOP)
  {
    semop(semaphores, ops_start, 2);

    memory_type *memory = shmat(global_memory, NULL, 0);

    memory->packages[memory->idx].status = SENT;
    memory->packages[memory->idx].value /= 2;
    memory->packages[memory->idx].value *= 3;

    int n = memory->packages[memory->idx].value;

    memory->idx = (memory->idx + 1) % PACKAGES_NUM;
    memory->size--;

    int m = semctl(semaphores, CREATED_IDX, GETVAL);
    int x = semctl(semaphores, PACKED_IDX, GETVAL);

    printf("(%d %lu) Wyslalem zamowienie o wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d.\n",
           getpid(), time(NULL), n, m, x);

    semop(semaphores, ops_end, 2);

    shmdt(memory);

    sleep(1);
  }
}