#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>

pid_t pid;
bool wait = true;

int received = 0;

void on_sigusr1(int _) { received++; }
void on_sigusr2(int _) { wait = false; }

union sigval val = {.sival_ptr = NULL};
int main(int argc, char **argv)
{
  bool use_q = strcmp(argv[3], "sigqueue") == 0;
  bool use_k = strcmp(argv[3], "kill") == 0;
  bool use_s = strcmp(argv[3], "sigrt") == 0;

  int S1 = use_s ? SIGRTMIN : SIGUSR1;
  int S2 = use_s ? S1 + 1 : SIGUSR2;

  signal(S1, on_sigusr1);
  signal(S2, on_sigusr2);

  pid = atoi(argv[1]);
  int total = atoi(argv[2]);

  if (use_k || use_s)
  {
    for (int i = 0; i < total; i++)
      kill(pid, S1);
    kill(pid, S2);
  }
  else if (use_q)
  {
    for (int i = 0; i < total; i++)
      sigqueue(pid, S1, val);
    sigqueue(pid, S2, val);
  }
  wait = true;
  while (wait)
    ;

  printf("[SENDER] receiver: %d\n", received);
}