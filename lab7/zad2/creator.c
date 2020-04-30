#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

sem_t *space;
sem_t *created;
sem_t *packed;
sem_t *can_modify;

void handle_sigterm()
{
  puts("sigterm");
  sem_close(space);
  sem_close(created);
  sem_close(packed);
  sem_close(can_modify);

  shm_unlink(S_MEMORY);

  exit(0);
}

int main()
{
  srand(getpid());
  signal(SIGTERM, handle_sigterm);

  space = sem_open(S_SPACE, O_RDWR, 0666);
  created = sem_open(S_CREATED, O_RDWR, 0666);
  packed = sem_open(S_PACKED, O_RDWR, 0666);
  can_modify = sem_open(S_CAN_MODIFY, O_RDWR, 0666);

  int global_memory = shm_open(S_MEMORY, O_RDWR, 0666);

  while (LOOP)
  {
    sem_wait(space);
    sem_wait(can_modify);

    int n = rand() % MAX_CREATED_NUM + 1;

    memory_type *memory =
        mmap(NULL, sizeof(memory_type), PROT_WRITE, MAP_SHARED, global_memory, 0);

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

    int m, x;
    sem_getvalue(created, &m);
    sem_getvalue(packed, &x);

    printf("(%d %lu) Dostalem liczbe %d. Liczba zamowien do przygotowania: %d. Liczba paczek do wyslania: %d.\n",
           getpid(), time(NULL), n, m + 1, x);

    sem_post(created);
    sem_post(can_modify);

    munmap(memory, sizeof(memory_type));

    sleep(1);
  }
}