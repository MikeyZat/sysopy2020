// @author: Mikolaj Zatorski, 2020
#define _XOPEN_SOURCE 500
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <linux/limits.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <ftw.h>

const char format[] = "%Y-%m-%d %H:%M:%S";
char buffer[PATH_MAX];
char *option;
char *option_arg;
int depth;

char *get_file_type(int st_mode)
{
  if (S_ISDIR(st_mode) != 0)
    return "dir";
  if (S_ISCHR(st_mode) != 0)
    return "char dev";
  if (S_ISBLK(st_mode) != 0)
    return "block dev";
  if (S_ISFIFO(st_mode) != 0)
    return "fifo";
  if (S_ISLNK(st_mode) != 0)
    return "slink";
  // if (S_ISSOCK(st_mode) != 0) on linux it can't be seen? throws compilation error
  //   return "sock";
  if (S_ISREG(st_mode) != 0)
    return "file";
  return "error";
}

void print_info(const char *path, const struct stat *file_stat)
{
  printf(" %ld \t", file_stat->st_nlink);
  printf(" %s \t", get_file_type(file_stat->st_mode));
  printf(" %ld\t", file_stat->st_size);
  strftime(buffer, PATH_MAX, format, localtime(&file_stat->st_atime));
  printf(" %s\t", buffer);
  strftime(buffer, PATH_MAX, format, localtime(&file_stat->st_mtime));
  printf(" %s\t", buffer);
  printf(" %s \t", path);
  printf("\n");
}

// this functions checks if we should show this file info
static int filter_and_print(const char *path, const struct stat *file_stat, int typeflag, struct FTW *ftwbuf)
{
  if (depth >= 0 && ftwbuf->level > depth)
    return 0;
  time_t curr_time = time(NULL);
  time_t file_time;
  if (strcmp(option, "-mtime") == 0)
  {
    file_time = file_stat->st_mtime;
  }
  else if (strcmp(option, "-atime") == 0)
  {
    file_time = file_stat->st_atime;
  }
  else
  {
    printf("Error - invalid option\n");
    return 0;
  }
  int difference = (int)(difftime(curr_time, file_time) / 86400);
  int flag = 0;
  int days = abs((int)strtol(option_arg, NULL, 10));
  if (option_arg[0] == '+')
  {
    flag = (difference > days);
  }
  else if (option_arg[0] == '-')
  {
    flag = (difference < days);
  }
  else
  {
    flag = (difference == days);
  }
  if (flag)
  {
    print_info(path, file_stat);
  }
  return 0;
}

int main(int args_num, char *args[])
{

  if (args_num != 4 && args_num != 6)
  {
    printf("Expected 3 or 5 arguments in format: \n \
     path -mtime | -atime {number} -maxdepth //optional {number} // optional\n");
    return 1;
  }

  char path[PATH_MAX];
  // check if main dir exists and can be opened
  if (realpath(args[1], path) == NULL)
  {
    printf("Given directory %s doesn't exist\n", args[1]);
    return 1;
  }
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    printf("Couldn't open the directory\n");
    return 1;
  }
  // parsing arguments comes next
  if (args_num == 4)
  {
    option = args[2];
    option_arg = args[3];
    depth = -1;
  }
  else
  {
    if (strcmp(args[2], "-maxdepth") == 0)
    {
      option = args[4];
      option_arg = args[5];
      depth = (int)strtol(args[3], NULL, 10);
    }
    else
    {
      option = args[2];
      option_arg = args[3];
      depth = (int)strtol(args[5], NULL, 10);
    }
  }
  // main logic
  nftw(path, filter_and_print, 10, FTW_PHYS);

  closedir(dir);
  return 0;
}
