#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

#define WORKERS_NUM CREATORS_NUM + PACKERS_NUM + RESHIPPERS_NUM

pid_t workers[WORKERS_NUM];

void sigint_handler()
{
  for (int i = 0; i < WORKERS_NUM; i++)
  {
    kill(workers[i], SIGTERM);
  }
}

int main()
{
  signal(SIGINT, sigint_handler);

  sem_t *space = sem_open(S_SPACE, O_CREAT | O_RDWR, 0666, PACKAGES_NUM);
  sem_t *created = sem_open(S_CREATED, O_CREAT | O_RDWR, 0666, 0);
  sem_t *packed = sem_open(S_PACKED, O_CREAT | O_RDWR, 0666, 0);
  sem_t *can_modify = sem_open(S_CAN_MODIFY, O_CREAT | O_RDWR, 0666, 1);

  int global_memory = shm_open(S_MEMORY, O_CREAT | O_RDWR, 0666);
  ftruncate(global_memory, sizeof(memory_type));

  memory_type *memory =
      mmap(NULL, sizeof(memory_type), PROT_WRITE, MAP_SHARED, global_memory, 0);

  memory->idx = -1;
  memory->size = 0;

  int i;

  for (i = 0; i < PACKAGES_NUM; i++)
  {
    memory->packages[i].status = SENT;
    memory->packages[i].value = 0;
  }

  munmap(memory, sizeof(memory_type));

  for (i = 0; i < CREATORS_NUM; i++)
  {
    workers[i] = fork();
    if (workers[i] == 0)
    {
      execlp("./creator", "./creator", NULL);
      return 1;
    }
  }

  for (i = CREATORS_NUM; i < CREATORS_NUM + PACKERS_NUM; i++)
  {
    workers[i] = fork();
    if (workers[i] == 0)
    {
      execlp("./packer", "./packer", NULL);
      return 1;
    }
  }

  for (i = CREATORS_NUM + PACKERS_NUM; i < WORKERS_NUM; i++)
  {
    workers[i] = fork();
    if (workers[i] == 0)
    {
      execlp("./reshipper", "./reshipper", NULL);
      return 1;
    }
    i++;
  }

  for (i = 0; i < WORKERS_NUM; i++)
  {
    wait(0);
  }

  sem_close(space);
  sem_close(created);
  sem_close(packed);
  sem_close(can_modify);

  sem_unlink(S_SPACE);
  sem_unlink(S_CREATED);
  sem_unlink(S_PACKED);
  sem_unlink(S_CAN_MODIFY);

  shm_unlink(S_MEMORY);

  return 0;
}