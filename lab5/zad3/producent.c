
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <wait.h>

int main(int args_num, char **argv)
{

  if (args_num != 4)
  {
    printf("Invalid number of arguments\nexpected 3 arguments - pipe_name, file_name,  N (buffer_size)");
    return 1;
  }

  FILE *fifo = fopen(argv[1], "r+");

  if (!fifo)
  {
    printf("Unable to open the file - %s\n", argv[1]);
    exit(1);
  }

  FILE *file = fopen(argv[2], "r");

  if (!file)
  {
    printf("Unable to open the file - %s\n", argv[2]);
    exit(1);
  }

  srand(time(0));

  int N = (int)strtol(argv[3], NULL, 10), read;

  char *buffer = calloc(N + 1, sizeof(char));

  pid_t pid = getpid();

  while ((read = fread(buffer, sizeof(char), N, file)) > 0)
  {
    sleep(rand() % 3 + 1);
    buffer[read] = 0;
    fprintf(fifo, "#%d#%s\n", pid, buffer);
    fflush(fifo);
  }
}