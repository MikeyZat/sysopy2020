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
  sem_close(space);
  sem_close(created);
  sem_close(packed);
  sem_close(can_modify);
  shm_unlink(S_MEMORY);

  exit(0);
}

int main()
{
  signal(SIGTERM, handle_sigterm);

  space = sem_open(S_SPACE, O_RDWR, 0666);
  created = sem_open(S_CREATED, O_RDWR, 0666);
  packed = sem_open(S_PACKED, O_RDWR, 0666);
  can_modify = sem_open(S_CAN_MODIFY, O_RDWR, 0666);

  int global_memory = shm_open(S_MEMORY, O_RDWR, 0666);

  while (LOOP)
  {
    sem_wait(packed);
    sem_wait(can_modify);

    memory_type *memory =
        mmap(NULL, sizeof(memory_type), PROT_WRITE, MAP_SHARED, global_memory, 0);

    memory->packages[memory->idx].status = SENT;
    memory->packages[memory->idx].value /= 2;
    memory->packages[memory->idx].value *= 3;
    int n = memory->packages[memory->idx].value;
    memory->idx = (memory->idx + 1) % PACKAGES_NUM;
    memory->size--;

    int m, x;
    sem_getvalue(created, &m);
    sem_getvalue(packed, &x);

    printf("(%d %lu) Wyslalem zamowienie o wielkosci %d. Liczba zamowien do przygotowania: %d. Liczba zamowien do wyslania: %d.\n",
           getpid(), time(NULL), n, m, x);

    sem_post(space);
    sem_post(can_modify);

    munmap(memory, sizeof(memory_type));

    sleep(1);
  }
}