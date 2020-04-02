#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handle_failure(int _) { _exit(0); }

int main(int _, char **argv)
{
  int sig = atoi(argv[2]);
  int ppid = atoi(argv[3]);

  if (strcmp(argv[1], "-i") == 0)
  {
    raise(sig);
    kill(ppid, SIGUSR2);
  }
  else if (strcmp(argv[1], "-m") == 0)
  {
    signal(sig, handle_failure);
    raise(sig);
    kill(ppid, SIGUSR2);
  }
  else if (strcmp(argv[1], "-p") == 0)
  {
    sigset_t mask;
    sigpending(&mask);
    if (sigismember(&mask, sig))
      kill(ppid, SIGUSR2);
  }
}