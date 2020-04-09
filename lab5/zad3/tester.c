#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char FIFO_NAME[] = "MULTIPLE_FIFO";

int main(int argc, char **argv)
{
  if (mkfifo(FIFO_NAME, 0644) != 0)
  {
    printf("Can not create fifo");
    exit(1);
  }
  if (fork() == 0)
  {
    execlp("./client", "./client", FIFO_NAME, "output.txt", 5, NULL);
    exit(1);
  }

  if (fork() == 0)
  {
    execlp("./producent", "./producent", FIFO_NAME, "first.txt", 3, NULL);
    exit(1);
  }

  if (fork() == 0)
  {
    execlp("./producent", "./producent", FIFO_NAME, "second.txt", 5, NULL);
    exit(1);
  }
  if (fork() == 0)
  {
    execlp("./producent", "./producent", FIFO_NAME, "third.txt", 2, NULL);
    exit(1);
  }
  if (fork() == 0)
  {
    execlp("./producent", "./producent", FIFO_NAME, "fourth.txt", 4, NULL);
    exit(1);
  }
  if (fork() == 0)
  {
    execlp("./producent", "./producent", FIFO_NAME, "fifth.txt", 1, NULL);
    exit(1);
  }

  while (wait(NULL) > 0)
    ;
  remove(FIFO_NAME);
}