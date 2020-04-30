#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

int global_memory = -1;
int semaphores = -1;

#define WORKERS_NUM CREATORS_NUM + PACKERS_NUM + RESHIPPERS_NUM

pid_t workers[WORKERS_NUM];

void handle_sigint()
{
  for (int i = 0; i < WORKERS_NUM; i++)
  {
    kill(workers[i], SIGTERM);
  }
}

int main()
{
  signal(SIGTERM, handle_sigint);
  key_t key = ftok("main", 1);

  semaphores = semget(key, 4, IPC_CREAT | 0666);
  semctl(semaphores, SPACE_IDX, SETVAL, PACKAGES_NUM);
  semctl(semaphores, CREATED_IDX, SETVAL, 0);
  semctl(semaphores, PACKED_IDX, SETVAL, 0);
  semctl(semaphores, CAN_MODIFY_IDX, SETVAL, 1);

  global_memory = shmget(key, sizeof(memory_type), IPC_CREAT | 0666);
  memory_type *memory = shmat(global_memory, NULL, 0);

  memory->idx = -1;
  memory->size = 0;

  int i;

  for (i = 0; i < PACKAGES_NUM; i++)
  {
    memory->packages[i].status = SENT;
    memory->packages[i].value = 0;
  }

  shmdt(memory);

  for (i = 0; i < CREATORS_NUM; i++)
  {
    workers[i] = fork();
    if (workers[i] == 0)
    {
      execlp("./creator", "./creator", NULL);
      return 1;
    }
  }

  for (i = CREATORS_NUM; i < PACKERS_NUM + CREATORS_NUM; i++)
  {
    workers[i] = fork();
    if (workers[i] == 0)
    {
      execlp("./packer", "./packer", NULL);
      return 1;
    }
  }

  for (i = PACKERS_NUM + CREATORS_NUM; i < WORKERS_NUM; i++)
  {
    workers[i] = fork();
    if (workers[i] == 0)
    {
      execlp("./reshipper", "./reshipper", NULL);
      return 1;
    }
  }

  for (i = 0; i < WORKERS_NUM; i++)
  {
    wait(0);
  }

  if (memory != -1)
  {
    shmctl(memory, IPC_RMID, NULL);
  }

  if (semaphores != -1)
  {
    semctl(semaphores, 0, IPC_RMID);
  }

  return 0;
}