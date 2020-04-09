#define _BSD_SOURCE
#include <stdio.h>
#include <features.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 200

int main(int args_num, char **args)
{
  if (args_num != 2)
  {
    printf("Invalid number of arguments\nexpected 1 argument - file_name");
    return 1;
  }

  char *file_name = args[1];
  char *program = calloc(strlen("sort %s") + strlen(file_name), sizeof(char));
  sprintf(program, "sort %s", file_name);

  FILE *file = popen(program, "r");

  if (!file)
  {
    perror("popen failed:");
    exit(1);
  }
  char line[MAX_LINE_LENGTH];

  while (fgets(line, sizeof(line), file))
  {
    printf("%s", line);
  }
  pclose(file);
}