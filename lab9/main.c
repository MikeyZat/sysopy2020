#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CLIENT_SLEEP_TIME 10
#define CREATING_TIME 5
#define CUTTING_TIME 10

pthread_mutex_t seats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t is_barber_ready = PTHREAD_COND_INITIALIZER;

int is_barber_sleeping;
int *seats;

int K, N;

int get_empty_seats()
{
  int sum = 0;

  for (int i = 0; i < K; i++)
  {
    if (seats[i] == 0)
    {
      sum++;
    }
  }

  return sum;
}

int is_waiting_room_empty()
{
  return (get_empty_seats() == K);
}

int get_waiting_clients_num()
{
  return K - get_empty_seats() - 1;
}

void cut_client()
{
  is_barber_sleeping = 0;

  int client_id;
  for (int i = 0; i < K; i++)
  {
    if (seats[i] != 0)
    {
      client_id = i;
      break;
    }
  }

  printf("Golibroda: czeka %d klientow, gole klienta %d\n", get_waiting_clients_num(), seats[client_id]);
  seats[client_id] = 0;
}

void barber()
{
  int served_clients = 0;

  while (served_clients < N)
  {
    pthread_mutex_lock(&seats_mutex);

    while (is_waiting_room_empty())
    {
      printf("Golibroda: ide spac");
      is_barber_sleeping = 1;
      pthread_cond_wait(&is_barber_ready, &seats_mutex);
    }
    cut_client();
    served_clients++;

    pthread_mutex_unlock(&seats_mutex);

    sleep(rand() % CUTTING_TIME + 1);
  }
  printf("Golibroda: koniec pracy");
}

void client(int *id)
{
  pthread_mutex_lock(&seats_mutex);

  int empty_seats_num = get_empty_seats();
  if (empty_seats_num == 0)
  {
    printf("Zajete; %d\n", *id);
    pthread_mutex_unlock(&seats_mutex);

    sleep(rand() % CLIENT_SLEEP_TIME + 1);

    client(id);
    return;
  }

  for (int i = 0; i < K; i++)
  {
    if (seats[i] == 0)
    {
      seats[i] = *id;
      break;
    }
  }
  empty_seats_num--;

  printf("Poczekalnia, wolne miejsca: %d; %d\n", empty_seats_num, *id);

  if (empty_seats_num == K - 1 && is_barber_sleeping)
  {
    printf("Budze golibrode; %d\n", *id);
    pthread_cond_broadcast(&is_barber_ready);
  }
  pthread_mutex_unlock(&seats_mutex);
}

int main(int argc, char *argv[])
{
  srand(time(NULL));

  if (argc != 3)
  {
    printf("Invalid number of arguments.\n \
    Expected $K - number of seats, $N - number of clients");
    return 1;
  }

  K = atoi(argv[1]);
  N = atoi(argv[2]);

  seats = calloc(K, sizeof(int));

  pthread_t barber_thr;
  pthread_create(&barber_thr, NULL, (void *(*)(void *))barber, NULL);

  int *client_ids = calloc(N, sizeof(int));
  pthread_t *client_threads = calloc(N, sizeof(pthread_t));

  for (int i = 0; i < N; i++)
  {
    sleep(rand() % CREATING_TIME + 1);
    client_ids[i] = i + 1;
    pthread_create(&client_threads[i], NULL, (void *(*)(void *))client,
                   client_ids + i);
  }

  pthread_join(barber_thr, NULL);
  for (int i = 0; i < N; i++)
  {
    pthread_join(client_threads[i], NULL);
  }

  free(client_threads);
  free(seats);
  free(client_ids);
}