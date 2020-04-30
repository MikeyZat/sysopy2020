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
  srand(getpid());
  key_t key = ftok("main", 1);

  int semaphores = semget(key, 4, 0);
  int global_memory = shmget(key, sizeof(memory_type), 0);

  struct sembuf unlock_memory = {CAN_MODIFY_IDX, 1, 0};
  struct sembuf increment_created = {CREATED_IDX, 1, 0};
  struct sembuf ops_end[2] = {unlock_memory, increment_created};

  struct sembuf lock_memory = {CAN_MODIFY_IDX, -1, 0};
  struct sembuf decrement_space = {SPACE_IDX, -1, 0};
  struct sembuf ops_start[2] = {lock_memory, decrement_space};

  while (LOOP)
  {
    semop(semaphores, ops_start, 2);

    int n = rand() % MAX_CREATED_NUM + 1;

    memory_type *memory = shmat(global_memory, NULL, 0);

    int curr_idx = 0;
    if (memory->idx == -1)
    {
      memory->idx = 0;
    }
    else
    {
      curr_idx = (memory->idx + memory->size) % PACKAGES_NUM;
    }

    memory->packages[curr_idx].status = CREATED;
    memory->packages[curr_idx].value = n;
    memory->size++;

    int m = semctl(semaphores, CREATED_IDX, GETVAL) + 1;
    int x = semctl(semaphores, PACKED_IDX, GETVAL);

    printf("(%d %lu) Dostalem liczbe %d. Liczba zamowien do przygotowania: %d. Liczba paczek do wyslania: %d.\n",
           getpid(), time(NULL), n, m, x);

    semop(semaphores, ops_end, 2);
    shmdt(memory);

    sleep(1);
  }
}