#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

#define PROGRAM "./do"

int do_ex = 0;
static int sig;
static pid_t ppid;

char *int_to_str(int v)
{
  static char buff[20];
  sprintf(buff, "%i", v);
  return strdup(buff);
}

int succ_sig = SIGUSR2;
void fail_handler(int _) { _exit(0); }
void sig_handler(int _) { kill(ppid, succ_sig); }

void handle_t()
{
  signal(sig, sig_handler);
  if (fork() == 0)
    raise(sig);
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  succ_sig = SIGUSR1;
  raise(sig);
}

void ignore_t()
{
  signal(sig, SIG_IGN);
  if (fork() == 0)
  {
    if (do_ex)
      execl(PROGRAM, PROGRAM, "i", int_to_str(sig), int_to_str(ppid), NULL);
    raise(sig);
    kill(ppid, SIGUSR2);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  raise(sig);
  kill(ppid, SIGUSR1);
}

void mask_t()
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, sig);
  sigprocmask(SIG_SETMASK, &mask, NULL);
  signal(sig, fail_handler);
  if (fork() == 0)
  {
    if (do_ex)
      execl(PROGRAM, PROGRAM, "m", int_to_str(sig), int_to_str(ppid), NULL);
    raise(sig);
    kill(ppid, SIGUSR2);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  raise(sig);
  kill(ppid, SIGUSR1);
}

void pending_t()
{
  sigset_t newmask;
  sigemptyset(&newmask);
  sigaddset(&newmask, sig);
  sigprocmask(SIG_SETMASK, &newmask, NULL);
  raise(sig);
  sigset_t mask;

  if (fork() == 0)
  {
    if (do_ex)
      execl(PROGRAM, PROGRAM, "p", int_to_str(sig), int_to_str(ppid), NULL);
    sigpending(&mask);
    if (sigismember(&mask, sig))
      kill(ppid, SIGUSR2);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  sigpending(&mask);
  if (sigismember(&mask, sig))
    kill(ppid, SIGUSR1);
}

int main(int argc, char const *argv[])
{
  ppid = getppid();
  if (strcmp(argv[1], "-d") == 0)
  {
    do_ex = 1;
    sig = atoi(argv[2]);
    if (strcmp(argv[3], "i") == 0)
      ignore_t();
    else if (strcmp(argv[3], "m") == 0)
      mask_t();
    else if (strcmp(argv[3], "p") == 0)
      pending_t();
    else if (strcmp(argv[3], "h") == 0)
      handle_t();
  }
  else
  {
    sig = atoi(argv[1]);
    if (strcmp(argv[2], "i") == 0)
      ignore_t();
    else if (strcmp(argv[2], "m") == 0)
      mask_t();
    else if (strcmp(argv[2], "p") == 0)
      pending_t();
    else if (strcmp(argv[2], "h") == 0)
      handle_t();
  }
}