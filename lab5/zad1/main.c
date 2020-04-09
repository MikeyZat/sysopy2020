#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define MAX_COMMANDS 10
#define MAX_ARGUMENTS 10
#define MAX_LINE_LENGTH 100000

int main(int arg_num, char **argv)
{
  if (arg_num != 2)
  {
    printf("Invalid number of arguments\nexpected 1 argument - file_name");
    return 1;
  }
  char commands_line[MAX_LINE_LENGTH];
  FILE *commands_file = fopen(argv[1], "r");

  int current[2], previous[2] = {-1, -1};

  while (fgets(commands_line, MAX_LINE_LENGTH, commands_file) != NULL)
  {

    char *arg = strtok(commands_line, " ");
    static char *arguments[MAX_ARGUMENTS + 1];
    int i = 0;
    while (arg != NULL) // calculate number of files to init res array
    {
      // char **arguments = arguments_to_array(current_command);
      if (strcmp(arg, "|") == 0)
      {
        while (i < MAX_ARGUMENTS)
          arguments[i++] = NULL;

        pipe(current); // starting the pipe
        pid_t pid = fork();

        if (pid == 0) // child
        {

          close(previous[1]);
          dup2(previous[0], STDIN_FILENO);
          if (arg != NULL)
          {
            // write ouput to the next input
            close(current[0]);
            dup2(current[1], STDOUT_FILENO);
          }
          if (execvp(arguments[0], arguments) == -1)
          {
            exit(1);
          }
        }
        close(current[1]);
        previous[0] = current[0];
        previous[1] = current[1];
        i = 0;
      }
      else
      {
        arguments[i++] = arg;
      }
      arg = strtok(NULL, " ");
    }
    // execute last command
    while (i < MAX_ARGUMENTS)
      arguments[i++] = NULL;

    pipe(current); // starting the pipe
    pid_t pid = fork();

    if (pid == 0) // child
    {
      close(previous[1]);
      dup2(previous[0], STDIN_FILENO);

      if (execvp(arguments[0], arguments) == -1)
      {
        exit(1);
      }
    }
  }

  return 0;
}