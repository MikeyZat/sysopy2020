#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include <linux/limits.h>

char *join_path(char *path1, char *path2)
{
  char *path = malloc(sizeof(char) * (strlen(path1) + strlen(path2)) + 2);
  sprintf(path, "%s/%s", path1, path2);
  return path;
}

void ls(char *path)
{
  printf("\npid(%i) path(%s)\n", getpid(), path);
  static const char cmd[] = "ls -a %s";
  char *ls = calloc(sizeof(cmd) + PATH_MAX, sizeof(char));
  sprintf(ls, cmd, path);
  system(ls);
  free(ls);
}

void scan_dir(char *path)
{
  if (!path)
    return;

  DIR *dir = opendir(path);
  char new_path[PATH_MAX];
  if (!dir)
    return;

  struct dirent *rdir = readdir(dir);
  struct stat file_stat;
  while (rdir)
  {
    strcpy(new_path, path);
    strcat(new_path, "/");
    strcat(new_path, rdir->d_name);
    // get file stats
    if (lstat(new_path, &file_stat) < 0)
      continue; // can not read

    if (strcmp(rdir->d_name, ".") == 0 || strcmp(rdir->d_name, "..") == 0)
    {
      rdir = readdir(dir);
      continue;
    }

    if (S_ISDIR(file_stat.st_mode) && fork() == 0)
    {
      ls(new_path);
      scan_dir(new_path);
      exit(0);
    }
    else
      wait(NULL);
    rdir = readdir(dir);
  }
  closedir(dir);
}

int main(int args_num, char **args)
{
  char *search_path = args_num > 1 ? args[1] : ".";
  scan_dir(search_path);
  return 0;
}