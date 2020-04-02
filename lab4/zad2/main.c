#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>

#define PROGRAM "./test"

static int status;

int parent_signal, child_signal;
void parent_is_signal(int _) { parent_signal = 1; }
void child_is_signal(int _) { child_signal = 1; }

char *itoa(int val)
{
  static char buff[12];
  sprintf(buff, "%i", val);
  return strdup(buff);
}

int main()
{
  signal(SIGUSR1, parent_is_signal);
  signal(SIGUSR2, child_is_signal);

  puts("EXEC   |  IGNORE  |   MASK   | PENDING  |");
  puts("SIGNAL | CH |  P  | CH |  P  | CH |  P  |");
  for (int sig = 1; sig < 23; sig++)
  {
    char *s = itoa(sig);
    printf("%7d|", sig);
    static char *arg_e[3] = {"i", "m", "p"};
    for (int i = 0; i < 3; i++)
    {
      child_signal = parent_signal = 0;
      if (fork() == 0)
      {
        execl(PROGRAM, PROGRAM, s, arg_e[i], NULL);
      }
      waitpid(WAIT_ANY, &status, WUNTRACED);
      printf("%4d|%5d|", child_signal ? 1 : 0, parent_signal ? 1 : 0);
    }
    printf("\n");
  }
  printf("\n");
  puts("FORK   |  IGNORE  | HANDLER  |   MASK   | PENDING  |");
  puts("SIGNAL | CH |  P  | CH |  P  | CH |  P  | CH |  P  |");
  for (int sig = 1; sig < 23; sig++)
  {
    char *s = itoa(sig);
    printf("%7d|", sig);
    static char *arg[4] = {"i", "h", "m", "p"};
    for (int i = 0; i < 4; i++)
    {
      child_signal = parent_signal = 0;
      if (fork() == 0)
      {
        execl(PROGRAM, PROGRAM, "-d", s, arg[i], NULL);
      }
      waitpid(WAIT_ANY, &status, WUNTRACED);
      printf("%4d|%5d|", child_signal ? 1 : 0, parent_signal ? 1 : 0);
    }
    printf("\n");
  }
}