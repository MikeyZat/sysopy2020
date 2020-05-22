#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
  char *nick;
  int fd;
  int is_playing;
} client_t;

client_t *clients[MAX_PLAYERS] = {NULL};
int clients_count = 0;

// GAME AND PLAYERS LOGIC

int get_player_by_nick(char *player_nick)
{
  for (int id = 0; id < MAX_PLAYERS; id++)
  {
    if (clients[id] != NULL && strcmp(clients[id]->nick, player_nick) == 0)
    {
      return id;
    }
  }
  return -1;
}

int get_opponent_id(int id) { return id % 2 == 0 ? id + 1 : id - 1; }

int add_client(char *player_nick, int fd)
{
  if (get_player_by_nick(player_nick) != -1)
    return -1;

  int index = -1;
  // check if there is any player waiting in queue
  for (int id = 0; id < MAX_PLAYERS; id += 2)
  {
    if (clients[id] != NULL && clients[id + 1] == NULL)
    {
      index = id + 1;
      break;
    }
  }

  // if noone is waiting, get first free place in queue
  for (int id = 0; id < MAX_PLAYERS && index == -1; id++)
  {
    if (clients[id] == NULL)
    {
      index = id;
    }
  }

  if (index != -1)
  {
    client_t *new_client = calloc(1, sizeof(client_t));
    new_client->nick = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
    strcpy(new_client->nick, player_nick);
    new_client->fd = fd;
    new_client->is_playing = 1;

    clients[index] = new_client;
    clients_count++;
  }

  return index;
}

void free_client(int client_id)
{
  free(clients[client_id]->nick);
  free(clients[client_id]);
  clients[client_id] = NULL;
  clients_count--;
}

int remove_client(char *player_nick)
{
  printf("Usuwam klienta o nicku: %s\n", player_nick);
  int client_id = get_player_by_nick(player_nick);
  if (client_id == -1)
  {
    printf("Blad - nie znaleziono klienta o nicku: %s\n", player_nick);
    return -1;
  }

  free_client(client_id);

  int opponent_id = get_opponent_id(client_id);

  if (clients[opponent_id] != NULL)
  {
    puts("Usuwam przeciwnika");
    free_client(opponent_id);
  }
  return 0;
}

// MAIN LOOP

void pinging_loop()
{
  puts("pinging");
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (clients[i] != NULL && !clients[i]->is_playing)
    {
      printf("Usuwam ping: %s\n", clients[i]->nick);
      remove_client(clients[i]->nick);
    }
  }

  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if (clients[i] != NULL)
    {
      puts("Wysylam ping");
      send(clients[i]->fd, "ping: ", MAX_MESSAGE_LENGTH, 0);
      clients[i]->is_playing = 0;
    }
  }
  pthread_mutex_unlock(&clients_mutex);

  sleep(1);
  pinging_loop();
}

// SOCKET LOGIC

int poll_sockets(int local_socket, int network_socket)
{
  struct pollfd *pfds = calloc(2 + clients_count, sizeof(struct pollfd));
  pfds[0].fd = local_socket;
  pfds[0].events = POLLIN;
  pfds[1].fd = network_socket;
  pfds[1].events = POLLIN;

  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < clients_count; i++)
  {
    pfds[i + 2].fd = clients[i]->fd;
    pfds[i + 2].events = POLLIN;
  }
  pthread_mutex_unlock(&clients_mutex);

  poll(pfds, clients_count + 2, -1);

  int retval;
  for (int i = 0; i < clients_count + 2; i++)
  {
    if (pfds[i].revents & POLLIN)
    {
      retval = pfds[i].fd;
      break;
    }
  }

  if (retval == local_socket || retval == network_socket)
  {
    retval = accept(retval, NULL, NULL);
  }

  free(pfds);

  return retval;
}

int setup_local_socket(char *path)
{
  int local_socket = socket(AF_UNIX, SOCK_STREAM, 0);

  struct sockaddr_un local_sockaddr;
  memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
  local_sockaddr.sun_family = AF_UNIX;
  strcpy(local_sockaddr.sun_path, path);

  unlink(path);
  // binding step
  bind(local_socket, (struct sockaddr *)&local_sockaddr,
       sizeof(struct sockaddr_un));

  // listening step
  listen(local_socket, MAX_BACKLOG);

  return local_socket;
}

int setup_network_socket(char *port)
{
  struct addrinfo *info;

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, port, &hints, &info);

  int network_socket =
      socket(info->ai_family, info->ai_socktype, info->ai_protocol);
  // binding step
  bind(network_socket, info->ai_addr, info->ai_addrlen);
  // listening step
  listen(network_socket, MAX_BACKLOG);

  freeaddrinfo(info);

  return network_socket;
}

// MAIN

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Blad - oczekuje 2 argumentow: $port_number $sciezka");
    return 1;
  }

  char *port = argv[1];
  char *socket_path = argv[2];

  srand(time(NULL));

  int local_socket = setup_local_socket(socket_path);
  int network_socket = setup_network_socket(port);

  // create thread
  pthread_t t;
  pthread_create(&t, NULL, (void *(*)(void *))pinging_loop, NULL);

  while (1)
  {
    int client_fd = poll_sockets(local_socket, network_socket);

    char buffer[MAX_MESSAGE_LENGTH + 1];
    recv(client_fd, buffer, MAX_MESSAGE_LENGTH, 0);
    puts(buffer);

    char *cmd = strtok(buffer, ":");
    char *arg = strtok(NULL, ":");
    char *player_nick = strtok(NULL, ":");

    pthread_mutex_lock(&clients_mutex);
    if (strcmp(cmd, "add") == 0)
    {
      int index = add_client(player_nick, client_fd);

      if (index == -1)
      {
        send(client_fd, "add:name_taken", MAX_MESSAGE_LENGTH, 0);
        close(client_fd);
      }
      else if (index % 2 == 0)
      {
        send(client_fd, "add:no_enemy", MAX_MESSAGE_LENGTH, 0);
      }
      else
      {
        int waiting_client_goes_first = rand() % 2;
        int first_player_index = index - waiting_client_goes_first;
        int second_player_index = get_opponent_id(first_player_index);

        send(clients[first_player_index]->fd, "add:O",
             MAX_MESSAGE_LENGTH, 0);
        send(clients[second_player_index]->fd, "add:X",
             MAX_MESSAGE_LENGTH, 0);
      }
    }
    if (strcmp(cmd, "move") == 0)
    {
      int move = atoi(arg);
      int player = get_player_by_nick(player_nick);

      sprintf(buffer, "move:%d", move);
      send(clients[get_opponent_id(player)]->fd, buffer, MAX_MESSAGE_LENGTH,
           0);
    }
    if (strcmp(cmd, "quit") == 0)
    {
      remove_client(player_nick);
    }
    if (strcmp(cmd, "pong") == 0)
    {
      int player = get_player_by_nick(player_nick);
      if (player != -1)
      {
        clients[player]->is_playing = 1;
      }
    }
    pthread_mutex_unlock(&clients_mutex);
  }
}