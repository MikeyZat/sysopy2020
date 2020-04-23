#ifndef MY_SIMPLE_CHAT_H
#define MY_SIMPLE_CHAT_H

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>

#define SERVER_NAME "/serverQueue"
#define MAX_CLIENTS 1000
#define MAX_MSG_LENGTH 500
#define MAX_MESSAGES 10
#define SPLIT ";"
#define COMMAND_TYPES 7

struct msg
{
  long mType;
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

unsigned get_cmd_priority(enum MSG_COMMAND cmd);
char *get_client_queue_name();

struct msg *parse_to_msg(char content[MAX_MSG_LENGTH]);
char *parse_msg_to_string(struct msg *msg);

int string_to_int(char *given_string);

void show_error_and_close(char *message);
void show_detailed_error_and_close(char *message);

#endif //MY_SIMPLE_CHAT_H
