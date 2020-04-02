#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

#define LOOP 1

int running = 1;

void print_dir()
{
  struct dirent *curr_file;

  DIR *directory = opendir(".");

  while ((curr_file = readdir(directory)) != NULL)
    puts(curr_file->d_name);
  closedir(directory);
}

void handle_signal(int sig)
{
  if (running == 1)
  {
    printf("\nOdebrano sygnal %d. Oczekuje na:\n CTRL+Z -> kontynuacja \n CTRL+C -> koniec programu \n", sig);
  }
  running = 1 - running;
}

void init_sig(int sig)
{
  printf("\nOdebrano SIGINT -> %d\n", sig);
  exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
  time_t act_time;

  struct sigaction sigs;
  sigs.sa_flags = 0;
  sigs.sa_handler = handle_signal;
  sigemptyset(&sigs.sa_mask);

  while (LOOP)
  {
    sigaction(SIGTSTP, &sigs, NULL);
    signal(SIGINT, init_sig);

    if (running)
    {
      char buffer[30];
      act_time = time(NULL);
      strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&act_time));
      puts("Files:");
      print_dir();
      puts("");
      puts(buffer);
    }
    sleep(1);
  }

  return 0;
}