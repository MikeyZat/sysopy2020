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
  struct sembuf incr_packed = {PACKED_IDX, 1, 0};
  struct sembuf ops_end[2] = {unlock_memory, incr_packed};

  struct sembuf lock_memory = {CAN_MODIFY_IDX, -1, 0};
  struct sembuf decr_created = {CREATED_IDX, -1, 0};
  struct sembuf ops_start[2] = {lock_memory, decr_created};

  while (LOOP)
  {
    semop(semaphores, ops_start, 2);

    memory_type *memory = shmat(global_memory, NULL, 0);

    int index = memory->idx;
    while (memory->packages[index].status != CREATED)
    {
      index = (index + 1) % PACKAGES_NUM;
    }

    memory->packages[index].value *= 2;
    memory->packages[index].status = PACKED;
    int n = memory->packages[index].value;

    int m = semctl(semaphores, CREATED_IDX, GETVAL);
    int x = semctl(semaphores, PACKED_IDX, GETVAL) + 1;

    printf("(%d %lu) Przygotowalem zamowienie o wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d.\n",
           getpid(), time(NULL), n, m, x);

    semop(semaphores, ops_end, 2);
    shmdt(memory);

    sleep(1);
  }
}