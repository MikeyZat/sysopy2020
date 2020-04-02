#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

pid_t pid;
bool wait = true;

int received = 0;

void on_sigusr1(int _) { received++; }
void on_sigusr2(int sig, siginfo_t *info, void *uncontext)
{
  printf("[CATCHER] received - %d\n", received);
  pid = info->si_pid;
  wait = false;
}

union sigval val = {.sival_ptr = NULL};
int main(int argc, char **argv)
{
  bool use_q = strcmp(argv[1], "sigqueue") == 0;
  bool use_k = strcmp(argv[1], "kill") == 0;
  bool use_s = strcmp(argv[1], "sigrt") == 0;

  int S1 = use_s ? SIGRTMIN : SIGUSR1;
  int S2 = use_s ? S1 + 1 : SIGUSR2;

  signal(S1, on_sigusr1);
  struct sigaction act;
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = on_sigusr2;
  sigaction(S2, &act, NULL);

  while (wait)
    ;

  printf("[PID] %d\n", getpid());
  if (use_k || use_s)
  {
    for (int i = 0; i < received; i++)
      kill(pid, S1);
    kill(pid, S2);
  }
  else if (use_q)
  {
    for (int i = 0; i < received; i++)
      sigqueue(pid, S1, val);
    sigqueue(pid, S2, val);
  }
}