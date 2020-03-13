// @author: Mikolaj Zatorski, 2020

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char *generate_random_string(int max_size)
{
  if (max_size < 1)
  {
    return NULL;
  }
  char *base = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  size_t dict_len = strlen(base);
  char *res = calloc(max_size, sizeof(char));

  for (int i = 0; i < max_size; i++)
  {
    res[i] = base[rand() % dict_len];
  }
  // res[max_size - 1] = '\n';

  return res;
}

void generate_files(char *file_name, char *records_str, char *bytes_str)
{
  int records = (int)strtol(records_str, NULL, 10);
  int bytes = (int)strtol(bytes_str, NULL, 10);
  int file = open(file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  for (int i = 0; i < records; i++)
  {
    char *rec = generate_random_string(bytes);
    write(file, rec, bytes);
    free(rec);
  }
}

void exec_copy(char *file_1, char *file_2, char *records_str, char *bytes_str, char *option)
{
  int records = (int)strtol(records_str, NULL, 10);
  int bytes = (int)strtol(bytes_str, NULL, 10);
  char *buffer = calloc(bytes, sizeof(char));
  size_t real_bytes;
  int i = 0;

  if (strcmp(option, "lib") == 0)
  {
    FILE *fp1, *fp2;
    if ((fp1 = fopen(file_1, "r")) == NULL)
    {
      printf("ERROR - file %s can not be opened\n", file_1);
      return;
    }
    fp2 = fopen(file_2, "w");
    while ((i < records) && (real_bytes = fread(buffer, 1, bytes, fp1) > 0))
    {
      fwrite(buffer, 1, bytes, fp2);
      i++;
    }
    fclose(fp1);
    fclose(fp2);
  }
  else if (strcmp(option, "sys") == 0)
  {
    int fp1, fp2;
    fp1 = open(file_1, O_RDONLY);
    fp2 = open(file_2, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    while ((i < records) && (real_bytes = read(fp1, buffer, sizeof(buffer))) > 0)
    {
      write(fp2, buffer, real_bytes);
      i++;
    }
    close(fp1);
    close(fp2);
  }
  else
  {
    printf("Wrong argument type. Expected type: sys | lib");
  }
  free(buffer);
}

double calculate_time(clock_t start, clock_t end)
{
  return (double)(end - start) / (double)sysconf(_SC_CLK_TCK);
}

int main(int args_num, char *args[])
{

  // random generator init
  srand((unsigned int)time(NULL));

  // TIMERS
  struct tms **tms_time = calloc(2, sizeof(struct tms *));
  for (int j = 0; j < 2; j++)
  {
    tms_time[j] = (struct tms *)calloc(1, sizeof(struct tms *));
  }

  int i = 1;
  while (i < args_num)
  {
    // commands
    char *command = args[i];

    // START TIME
    times(tms_time[0]);

    if (strcmp(command, "copy") == 0)
    {
      exec_copy(args[i + 1], args[i + 2], args[i + 3], args[i + 4], args[i + 5]);
      i += 6;
    }
    else if (strcmp(command, "generate") == 0)
    {
      generate_files(args[i + 1], args[i + 2], args[i + 3]);
      i += 4;
    }
    else
    {
      i++;
    }
    printf("[USER_TIME] Executing action %s took %fs\n", command, calculate_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));
    printf("[SYSTEM_TIME] Executing action %s took %fs\n", command, calculate_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));
  }

  return 0;
}
