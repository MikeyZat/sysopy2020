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

void generate_files(char *file_name, char *records_str, char *bytes_str)
{
  int records = (int)strtol(records_str, NULL, 10);
  int bytes = (int)strtol(bytes_str, NULL, 10);
  char *buffer = calloc(strlen(file_name) + 100, sizeof(char));
  sprintf(buffer, "head -c %d /dev/urandom > %s", records * bytes,
          file_name);
  system(buffer);
  free(buffer);
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
}

void swap_lib(FILE *file, int bytes, int i, int j)
{
  char *first_record = calloc(bytes, sizeof(char));
  char *second_record = calloc(bytes, sizeof(char));

  fseek(file, i * bytes, 0);
  fread(first_record, 1, bytes, file);
  fseek(file, j * bytes, 0);
  fread(second_record, 1, bytes, file);

  fseek(file, i * bytes, 0);
  fwrite(second_record, 1, bytes, file);
  fseek(file, j * bytes, 0);
  fwrite(first_record, 1, bytes, file);

  free(first_record);
  free(second_record);
}

int partition_lib(FILE *file, int bytes, int low, int high)
{
  int pivot = high;
  char *pivot_str = NULL;

  int i = (low - 1);

  for (int j = low; j <= high - 1; j++)
  {
    if (pivot_str == NULL)
    {
      pivot_str = calloc(bytes, sizeof(char));
      fseek(file, pivot * bytes, 0);
      fread(pivot_str, 1, bytes, file);
    }

    char *current = calloc(bytes, sizeof(char));
    fseek(file, j * bytes, 0);
    fread(current, 1, bytes, file);

    if (strcmp(current, pivot_str) < 0)
    {
      free(pivot_str);
      pivot_str = NULL;

      free(current);
      current = NULL;

      i++;
      swap_lib(file, bytes, i, j);
    }

    if (pivot_str != NULL)
    {
      free(pivot_str);
      pivot_str = NULL;
    }

    if (current != NULL)
    {
      free(current);
      current = NULL;
    }
  }
  swap_lib(file, bytes, i + 1, high);
  return i + 1;
}

void quicksort_lib(FILE *file, int bytes, int low, int high)
{
  if (low < high)
  {
    int pivot = partition_lib(file, bytes, low, high);

    quicksort_lib(file, bytes, low, pivot - 1);
    quicksort_lib(file, bytes, pivot + 1, high);
  }
}

void swap_sys(int file, int bytes, int i, int j)
{
  char *first_record = calloc(bytes, sizeof(char));
  char *second_record = calloc(bytes, sizeof(char));

  lseek(file, i * bytes, SEEK_SET);
  read(file, first_record, bytes);
  lseek(file, j * bytes, SEEK_SET);
  read(file, second_record, bytes);

  lseek(file, i * bytes, SEEK_SET);
  write(file, second_record, bytes);
  lseek(file, j * bytes, SEEK_SET);
  write(file, first_record, bytes);

  free(first_record);
  free(second_record);
}

int partition_sys(int file, int bytes, int low, int high)
{
  int pivot = high;
  char *pivot_str = NULL;

  int i = (low - 1);

  for (int j = low; j <= high - 1; j++)
  {
    if (pivot_str == NULL)
    {
      pivot_str = calloc(bytes, sizeof(char));
      lseek(file, pivot * bytes, SEEK_SET);
      read(file, pivot_str, bytes);
    }

    char *current = calloc(bytes, sizeof(char));
    lseek(file, j * bytes, SEEK_SET);
    read(file, current, bytes);

    if (strcmp(current, pivot_str) < 0)
    {
      free(pivot_str);
      pivot_str = NULL;

      free(current);
      current = NULL;

      i++;
      swap_sys(file, bytes, i, j);
    }

    if (pivot_str != NULL)
    {
      free(pivot_str);
      pivot_str = NULL;
    }

    if (current != NULL)
    {
      free(current);
      current = NULL;
    }
  }
  swap_sys(file, bytes, i + 1, high);
  return i + 1;
}

void quicksort_sys(int file, int bytes, int low, int high)
{
  if (low < high)
  {
    int pivot = partition_sys(file, bytes, low, high);

    quicksort_sys(file, bytes, low, pivot - 1);
    quicksort_sys(file, bytes, pivot + 1, high);
  }
}

void sort_files(char *file_name, char *records_str, char *bytes_str, char *option)
{
  int records = (int)strtol(records_str, NULL, 10);
  int bytes = (int)strtol(bytes_str, NULL, 10);

  if (strcmp(option, "lib") == 0)
  {
    FILE *file = fopen(file_name, "rwb+");
    quicksort_lib(file, bytes, 0, records - 1);
    fclose(file);
  }
  else if (strcmp(option, "sys") == 0)
  {
    int file = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    quicksort_sys(file, bytes, 0, records - 1);
    close(file);
  }
  else
  {
    printf("Wrong argument type. Expected type: sys | lib");
  }
}

double calculate_time(clock_t start, clock_t end)
{
  return (double)(end - start) / (double)sysconf(_SC_CLK_TCK);
}

int main(int args_num, char *args[])
{

  // TIMERS
  struct tms **tms_time = malloc(2 * sizeof(struct tms *));
  for (int j = 0; j < 2; j++)
  {
    tms_time[j] = (struct tms *)malloc(sizeof(struct tms *));
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
    else if (strcmp(command, "sort") == 0)
    {
      sort_files(args[i + 1], args[i + 2], args[i + 3], args[i + 4]);
      i += 5;
    }
    else
    {
      i++;
    }
    times(tms_time[1]);

    printf("[USER_TIME] Executing action %s took %fs\n", command, calculate_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime));
    printf("[SYSTEM_TIME] Executing action %s took %fs\n", command, calculate_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime));
  }

  return 0;
}
