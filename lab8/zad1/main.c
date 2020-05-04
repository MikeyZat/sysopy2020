#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 70 // this comes from documentation of such images

int threads_count, height, width;

int **image;

int **histograms;

int measure_time(struct timespec *start)
{
  struct timespec stop;
  clock_gettime(CLOCK_MONOTONIC, &stop);
  int retval = (stop.tv_sec - start->tv_sec) * 1000000;
  retval += (stop.tv_nsec - start->tv_nsec) / 1000.0;
  return retval;
}

void skip_trash(FILE *img, char *buffer)
{
  while (buffer[0] == '#' || buffer[0] == '\n')
  {
    fgets(buffer, MAX_LINE_LENGTH, img);
  }
}

void get_img(char *file_name)
{
  FILE *img = fopen(file_name, "r");

  char buffer[MAX_LINE_LENGTH + 1] = {0};

  fgets(buffer, MAX_LINE_LENGTH, img); // skip P2
  skip_trash(img, buffer);

  fgets(buffer, MAX_LINE_LENGTH, img);

  width = atoi(strtok(buffer, " \t\r\n"));
  height = atoi(strtok(NULL, " \t\r\n"));

  image = calloc(height, sizeof(int *));

  for (int i = 0; i < height; i++)
  {
    image[i] = calloc(width, sizeof(int));
  }

  fgets(buffer, MAX_LINE_LENGTH, img); // skip the 255 line

  fgets(buffer, MAX_LINE_LENGTH, img);
  skip_trash(img, buffer);

  char *value = strtok(buffer, " \t\r\n");

  for (int i = 0; i < width * height; i++)
  {
    if (value == NULL)
    {
      fgets(buffer, MAX_LINE_LENGTH, img);
      skip_trash(img, buffer);
      value = strtok(buffer, " \t\r\n");
    }

    image[i / width][i % width] = atoi(value);

    value = strtok(NULL, " \t\r\n");
  }

  fclose(img);
}

void init_histograms(int k)
{
  histograms = calloc(k, sizeof(int *));
  for (int i = 0; i < k; i++)
  {
    histograms[i] = calloc(256, sizeof(int));
  }
}

// different modes

int sign_mode(int *thread_index)
{
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int k = *thread_index;
  int range = 256 / threads_count;

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      if (image[y][x] / range == k)
      {
        histograms[0][image[y][x]]++;
      }
    }
  }
  return measure_time(&start);
}

int block_mode(int *thread_index)
{
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int k = *thread_index;
  int range = width / threads_count;
  for (int x = k * range; x < (k + 1) * range; x++)
  {
    for (int y = 0; y < height; y++)
    {
      histograms[k][image[y][x]]++;
    }
  }

  return measure_time(&start);
}

int interleaved_mode(int *thread_index)
{
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int k = *thread_index;
  for (int x = k; x < width; x += threads_count)
  {
    for (int y = 0; y < height; y++)
    {
      histograms[k][image[y][x]]++;
    }
  }

  return measure_time(&start);
}

void write_histogram_to_file(char *file_name)
{
  FILE *file = fopen(file_name, "w+");

  int histogram[256] = {0};
  for (int color = 0; color < 256; color++)
  {
    for (int i = 0; i < threads_count; i++)
    {
      histogram[color] += histograms[i][color];
    }
    fprintf(file, "color %d - %d occurances\n", color, histogram[color]);
  }

  fclose(file);
}

void clean_memory()
{
  for (int i = 0; i < threads_count; i++)
  {
    free(histograms[i]);
  }
  free(histograms);
  for (int y = 0; y < height; y++)
  {
    free(image[y]);
  }
  free(image);
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("Invalid number of arguments.\n \
    Expected $threads_num $(sign|block|interleaved) $file_in_name $file_out_name");
    return 1;
  }

  threads_count = atoi(argv[1]);
  char *mode = argv[2];
  char *file_in = argv[3];
  char *file_out = argv[4];

  get_img(file_in);

  init_histograms(threads_count);

  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int (*mode_function)(int *);

  if (strcmp(mode, "sign") == 0)
  {
    mode_function = sign_mode;
  }
  else if (strcmp(mode, "block") == 0)
  {
    mode_function = block_mode;
  }
  else if (strcmp(mode, "interleaved") == 0)
  {
    mode_function = interleaved_mode;
  }
  else
  {
    clean_memory();
    printf("Invalid mode argument");
    return 1;
  }

  pthread_t *threads = calloc(threads_count, sizeof(pthread_t));
  int *args = calloc(threads_count, sizeof(int));

  for (int i = 0; i < threads_count; i++)
  {
    args[i] = i;
    pthread_create(&threads[i], NULL, (void *(*)(void *))mode_function, args + i);
  }

  for (int i = 0; i < threads_count; i++)
  {
    int execution_time;
    pthread_join(threads[i], (void *)&execution_time);
    printf("Watek nr %d pracowal %d mikrosekund\n", i, execution_time);
  }

  printf("Caly proces pracowal: %d milisekund\n", measure_time(&start));

  write_histogram_to_file(file_out);

  free(threads);
  free(args);

  clean_memory();

  return 0;
}