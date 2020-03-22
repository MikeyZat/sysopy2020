#define MAX_COLS_NUMBER 1000
#define MAX_LINE_LENGTH (MAX_COLS_NUMBER * 5)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int random_inclusive(int min, int max)
{
  return rand() % (max - min + 1) + min;
}

void generate_matrix(int rows, int cols, char *filename)
{
  FILE *file = fopen(filename, "w+");

  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < cols; x++)
    {
      if (x > 0)
      {
        fprintf(file, " ");
      }
      fprintf(file, "%d", random_inclusive(-100, 100));
    }
    fprintf(file, "\n");
  }
}

typedef struct
{
  int **values;
  int rows;
  int cols;
} matrix;

int get_cols_number(char *row)
{
  int cols = 0;
  char *tmp = row;
  while ((tmp = strchr(tmp, ' ')) != NULL)
  {
    cols++;
    tmp++;
  }
  return cols + 1;
}

void get_dimensions(FILE *matrix_file, int *rows, int *cols)
{
  char line[MAX_LINE_LENGTH];
  *rows = 0;
  *cols = 0;

  while (fgets(line, MAX_LINE_LENGTH, matrix_file) != NULL)
  {
    if (*cols == 0)
    {
      *cols = get_cols_number(line);
    }

    *rows = *rows + 1;
  }

  fseek(matrix_file, 0, SEEK_SET);
}

matrix load_matrix(char *filename)
{
  FILE *matrix_file = fopen(filename, "r");

  int rows, cols;
  get_dimensions(matrix_file, &rows, &cols);

  int **values = calloc(rows, sizeof(int *));
  for (int y = 0; y < rows; y++)
  {
    values[y] = calloc(cols, sizeof(int));
  }

  int x_curr, y_curr = 0;
  char line[MAX_LINE_LENGTH];
  while (fgets(line, MAX_LINE_LENGTH, matrix_file) != NULL)
  {
    x_curr = 0;
    char *encoded_number = strtok(line, " ");
    while (encoded_number != NULL)
    {
      values[y_curr][x_curr++] = atoi(encoded_number);
      encoded_number = strtok(NULL, " ");
    }

    y_curr++;
  }

  fclose(matrix_file);

  matrix retval;
  retval.values = values;
  retval.rows = rows;
  retval.cols = cols;

  return retval;
}

void test()
{
  matrix a = load_matrix("a.txt");
  matrix b = load_matrix("b.txt");
  matrix c = load_matrix("c.txt");
  int **first = a.values;
  int **second = b.values;
  int **third = c.values;
  int **result = calloc(a.rows, sizeof(int *));
  for (int i = 0; i < a.rows; i++) // init result matrix
  {
    result[i] = calloc(b.cols, sizeof(int));
    for (int j = 0; j < b.cols; j++)
    {
      result[i][j] = 0;
    }
  }

  for (int c = 0; c < a.rows; c++)
  {
    for (int d = 0; d < b.cols; d++)
    {
      int sum = 0;
      for (int k = 0; k < b.rows; k++)
      {
        sum += first[c][k] * second[k][d];
      }
      result[c][d] = sum;
    }
  }
  for (int i = 0; i < a.rows; i++) // init result matrix
  {
    for (int j = 0; j < b.cols; j++)
    {
      if (result[i][j] != third[i][j])
      {
        printf("Incorrect result of multiplying matrixes\n\
         %d != %d at index %d %d\n",
               result[i][j], third[i][j], i, j);
      }
    }
  }
  printf("Correct result of multypling the matrixes");

  for (int i = 0; i < a.rows; i++) // free result matrix
  {
    free(result[i]);
  }
  free(result);
}

int main(int argc, char *argv[])
{
  if (argc != 3 && argc != 2)
  {
    fprintf(stderr, "Usage: ./helper min max or ./helper test");
    return 1;
  }

  if (argc == 2)
  {
    if (strcmp(argv[1], "test") == 0)
    {
      test();
    }
    else
    {
      fprintf(stderr, "Usage: ./helper min max or ./helper test");
      return 1;
    }
    return 0;
  }

  srand(time(NULL));

  int min = atoi(argv[1]);
  int max = atoi(argv[2]);

  int a_rows = random_inclusive(min, max);
  int a_cols = random_inclusive(min, max);
  int b_cols = random_inclusive(min, max);

  generate_matrix(a_rows, a_cols, "a.txt");
  generate_matrix(a_cols, b_cols, "b.txt");

  system("echo \"a.txt b.txt c.txt\" > lista");

  return 0;
}