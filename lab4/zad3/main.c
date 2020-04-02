#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <memory.h>

volatile int L, T, sent_to_child, received_by_child, received_from_child;
volatile pid_t ch;

void display_stats()
{
  printf("Sygnaly wyslane do dziecka: %d\n", sent_to_child);
  printf("\n");
  printf("Sygnaly otrzymane od dziecka: %d\n", received_from_child);
}

void child_handler(int signum, siginfo_t *info, void *context)
{
  if (signum == SIGINT)
  {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);
    printf("Dziecko otrzymalo %d sygnalow \n", received_by_child);
    exit((unsigned)received_by_child);
  }
  if (info->si_pid != getppid())
    return;

  if (T == 1 || T == 2)
  {
    if (signum == SIGUSR1)
    {
      received_by_child++;
      kill(getppid(), SIGUSR1);
      puts("dziecko <- SIGUSR1, dziecko -> SIGUSR1");
    }
    else if (signum == SIGUSR2)
    {
      received_by_child++;
      puts("Dziecko <- SIGUSR2");
      printf("Dziecko otrzymalo %d sygnalow \n", received_by_child);
      exit((unsigned)received_by_child);
    }
  }
  else if (T == 3)
  {
    if (signum == SIGRTMIN)
    {
      received_by_child++;
      kill(getppid(), SIGRTMIN);
      puts("dziecko <- SIGRTMIN, dziecko -> SIGRTMIN");
    }
    else if (signum == SIGRTMAX)
    {
      received_by_child++;
      puts("Dziecko <- SIGRTMAX");
      printf("Dziecko otrzymalo %d sygnalow \n", received_by_child);
      exit((unsigned)received_by_child);
    }
  }
}

void mother_handler(int signum, siginfo_t *info, void *context)
{
  if (signum == SIGINT)
  {
    puts("Matka <- SIGINT");
    kill(ch, SIGUSR2);
    display_stats();
    exit(9);
  }
  if (info->si_pid != ch)
    return;

  if ((T == 1 || T == 2) && signum == SIGUSR1)
  {
    received_from_child++;
    puts("Matka <- SIGUSR1");
  }
  else if (T == 3 && signum == SIGRTMIN)
  {
    received_from_child++;
    puts("Matka <- SIGRTMIN");
  }
}

void child()
{
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = child_handler;

  if (sigaction(SIGINT, &act, NULL) == -1)
    exit(1);
  if (sigaction(SIGUSR1, &act, NULL) == -1)
    exit(1);
  if (sigaction(SIGUSR2, &act, NULL) == -1)
    exit(1);
  if (sigaction(SIGRTMIN, &act, NULL) == -1)
    exit(1);
  if (sigaction(SIGRTMAX, &act, NULL) == -1)
    exit(1);

  while (1)
  {
    sleep(1);
  }
}

void mother()
{
  sleep(1);

  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = mother_handler;

  if (T == 1 || T == 2)
  {
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGINT);
    for (; sent_to_child < L; sent_to_child++)
    {
      puts("matka -> SIGUSR1");
      kill(ch, SIGUSR1);
      if (T == 2)
        sigsuspend(&mask);
    }
    puts("matka -> SIGUSR2");
    kill(ch, SIGUSR2);
  }
  else if (T == 3)
  {
    for (; sent_to_child < L; sent_to_child++)
    {
      puts("matka -> SIGTMIN");
      kill(ch, SIGRTMIN);
    }
    sent_to_child++;
    puts("matka -> SIGTMAX");
    kill(ch, SIGRTMAX);
  }

  int status = 0;
  waitpid(ch, &status, 0);
  if (WIFEXITED(status))
    received_by_child = WEXITSTATUS(status);
  else
    exit(1);
}

int main(int argc, char *argv[])
{
  sent_to_child = 0;
  received_by_child = 0;
  received_from_child = 0;

  L = (int)strtol(argv[1], '\0', 10);
  T = (int)strtol(argv[2], '\0', 10);

  ch = fork();
  if (!ch)
    child();
  else if (ch > 0)
    mother();

  display_stats();

  return 0;
}