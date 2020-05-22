#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "util.h"

pthread_mutex_t reply_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reply_cond = PTHREAD_COND_INITIALIZER;

int server_socket;
int is_O;
char buffer[MAX_MESSAGE_LENGTH + 1];
char *player_nick;

board_t board;

typedef enum
{
  START,
  WAIT_FOR_ENEMY,
  WAIT_FOR_MOVE,
  PROCESS_ENEMY_MOVE,
  MOVE,
  QUIT
} state_t;

state_t state = START;

char *cmd, *arg;

void quit_game()
{
  char buffer[MAX_MESSAGE_LENGTH + 1];
  sprintf(buffer, "quit: :%s", player_nick);
  send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);

  exit(0);
}

void check_board_status()
{
  // check for a win
  int finished = 0;
  board_field winner = get_winner(&board);
  if (winner != EMPTY)
  {
    if ((is_O && winner == O) || (!is_O && winner == X))
    {
      puts("Zwyciestwo!");
    }
    else
    {
      puts("Porazka!");
    }

    finished = 1;
  }

  // check for a draw
  int is_drawn = 1;
  for (int i = 0; i < 9 && is_drawn == 1; i++)
  {
    if (board.fields[i] == EMPTY)
    {
      is_drawn = 0;
    }
  }

  if (is_drawn && !finished)
  {
    puts("Remis!");
  }

  if (finished || is_drawn)
  {
    state = QUIT;
  }
}

void split_reply(char *reply)
{
  cmd = strtok(reply, ":");
  arg = strtok(NULL, ":");
}

void draw_board()
{
  char symbols[3] = {'-', 'O', 'X'};
  for (int y = 0; y < 3; y++)
  {
    for (int x = 0; x < 3; x++)
    {
      symbols[0] = y * 3 + x + 1 + '0';
      printf("|%c|", symbols[board.fields[y * 3 + x]]);
    }
    puts("\n---------");
  }
}

void game_loop()
{
  is_O = arg[0] == 'O';

  if (state == START)
  {
    if (strcmp(arg, "name_taken") == 0)
    {
      puts("Ta nazwa jest juz zajeta");
      exit(1);
    }
    else if (strcmp(arg, "no_enemy") == 0)
    {
      puts("Gra zacznie sie kiedy drugi gracz dolaczy");
      state = WAIT_FOR_ENEMY;
    }
    else
    {
      board = create_new_board();
      state = is_O ? MOVE : WAIT_FOR_MOVE;
    }
  }
  else if (state == WAIT_FOR_ENEMY)
  {
    pthread_mutex_lock(&reply_mutex);
    while (state != START && state != QUIT)
    {
      pthread_cond_wait(&reply_cond, &reply_mutex);
    }
    pthread_mutex_unlock(&reply_mutex);

    board = create_new_board();
    state = is_O ? MOVE : WAIT_FOR_MOVE;
  }
  else if (state == WAIT_FOR_MOVE)
  {
    puts("Czekam na ruch przeciwnika");

    pthread_mutex_lock(&reply_mutex);
    while (state != PROCESS_ENEMY_MOVE && state != QUIT)
    {
      pthread_cond_wait(&reply_cond, &reply_mutex);
    }
    pthread_mutex_unlock(&reply_mutex);
  }
  else if (state == PROCESS_ENEMY_MOVE)
  {
    int field_idx = atoi(arg);
    move(&board, field_idx);
    check_board_status();
    if (state != QUIT)
    {
      state = MOVE;
    }
  }
  else if (state == MOVE)
  {
    draw_board();

    int field_idx;
    do
    {
      printf("Enter next move (%c): ", is_O ? 'O' : 'X');
      scanf("%d", &field_idx);
      field_idx--;
    } while (!move(&board, field_idx));

    draw_board();

    char buffer[MAX_MESSAGE_LENGTH + 1];
    sprintf(buffer, "move:%d:%s", field_idx, player_nick);
    send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);

    check_board_status();
    if (state != QUIT)
    {
      state = WAIT_FOR_MOVE;
    }
  }
  else if (state == QUIT)
  {
    quit_game();
  }
  game_loop();
}

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    fprintf(stderr, "Blad - oczekuje 3 argumentow: $player_nick 'O'|'X' $destination");
    return 1;
  }

  player_nick = argv[1];
  char *type = argv[2];
  char *destination = argv[3];

  signal(SIGINT, quit_game);

  int is_local = strcmp(type, "local") == 0;
  struct sockaddr_un local_sockaddr;
  int binded_socket;

  if (is_local)
  {
    server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

    memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
    local_sockaddr.sun_family = AF_UNIX;
    strcpy(local_sockaddr.sun_path, destination);

    connect(server_socket, (struct sockaddr *)&local_sockaddr,
            sizeof(struct sockaddr_un));

    binded_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un binded_sockaddr;
    memset(&binded_sockaddr, 0, sizeof(struct sockaddr_un));
    binded_sockaddr.sun_family = AF_UNIX;
    sprintf(binded_sockaddr.sun_path, "/tmp/%d", getpid());

    bind(binded_socket, (struct sockaddr *)&binded_sockaddr,
         sizeof(struct sockaddr_un));
  }
  else
  {
    struct addrinfo *info;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    getaddrinfo("127.0.0.1", destination, &hints, &info);

    server_socket =
        socket(info->ai_family, info->ai_socktype, info->ai_protocol);

    connect(server_socket, info->ai_addr, info->ai_addrlen);

    freeaddrinfo(info);
  }
  char buffer[MAX_MESSAGE_LENGTH + 1];
  sprintf(buffer, "add: :%s", player_nick);
  if (is_local)
  {
    sendto(binded_socket, buffer, MAX_MESSAGE_LENGTH, 0,
           (struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr_un));
  }
  else
  {
    send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
  }

  int game_thread_running = 0;

  while (1)
  {
    if (is_local)
    {
      recv(binded_socket, buffer, MAX_MESSAGE_LENGTH, 0);
    }
    else
    {
      recv(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
    }
    split_reply(buffer);

    pthread_mutex_lock(&reply_mutex);
    if (strcmp(cmd, "add") == 0)
    {
      state = START;
      if (!game_thread_running)
      {
        pthread_t t;
        pthread_create(&t, NULL, (void *(*)(void *))game_loop, NULL);
        game_thread_running = 1;
      }
    }
    else if (strcmp(cmd, "move") == 0)
    {
      state = PROCESS_ENEMY_MOVE;
    }
    else if (strcmp(cmd, "quit") == 0)
    {
      state = QUIT;
      exit(0);
    }
    else if (strcmp(cmd, "ping") == 0)
    {
      sprintf(buffer, "pong: :%s", player_nick);
      send(server_socket, buffer, MAX_MESSAGE_LENGTH, 0);
    }
    pthread_cond_signal(&reply_cond);
    pthread_mutex_unlock(&reply_mutex);
  }
}