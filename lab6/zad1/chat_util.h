#ifndef MY_SIMPLE_CHAT_H
#define MY_SIMPLE_CHAT_H

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stddef.h>
#include <string.h>

#define LETTER 'a'
#define MAX_CLIENTS 500
#define MAX_MSG_LENGTH 250
#define COMMAND_TYPES 7

struct msg
{
  long msgType;
  pid_t sender;
  char msg[MAX_MSG_LENGTH];
};

#define MSGSZ sizeof(struct msg)

enum MSG_COMMAND
{
  INIT = 1L,
  STOP = 2L,
  DISCONNECT = 3L,
  LIST = 4L,
  CONNECT = 5L,
  ECHO = 6L,
};

key_t get_server_queue_key();
key_t get_client_queue_key();

int string_to_int(char *given_string);

void show_error_and_close(char *msg);
void show_detailed_error_and_close(char *msg);

#endif // MY_SIMPLE_CHAT_H
