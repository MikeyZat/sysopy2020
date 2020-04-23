#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include "chat_util.h"

int server_queue = -1;
int client_queue = -1;
int peer_queue = -1;
int peer_pid = -1;
int client_id = -1;
int working = 1;

//signal handlers
void signals_handler(int signo)
{
  struct msg message;
  if (msgrcv(client_queue, &message, MSGSZ, -COMMAND_TYPES, 0) == -1)
    show_detailed_error_and_close("Error while receiveing message");

  switch (message.msgType)
  {
  case STOP:
  {
    exit(EXIT_SUCCESS);
  }
  case CONNECT:
  {
    peer_queue = string_to_int(message.msg);
    peer_pid = message.sender;
    printf("\033[1;35mClient:\033[0m Connected with peer queue ID:\t%d \n", peer_queue);
    break;
  }
  case DISCONNECT:
  {
    peer_queue = peer_pid = -1;
    printf("\033[1;35mClient:\033[0m Disconnected from peer \n");
    break;
  }
  case ECHO:
  {
    printf("Peer: %s\n", message.msg);
    break;
  }
  }
}

void _exit(int signo)
{
  puts("SIGINT received - closing");
  stop();
}

void clean_after_exit()
{
  if (msgctl(client_queue, IPC_RMID, NULL) == -1)
    show_detailed_error_and_close("Error occured - unable to remove queue");
  else
  {
    printf("\033[1;35mClient:\033[0m Queue has been removed\n");
  }
}
// functions

void send(enum MSG_COMMAND type, char content[MAX_MSG_LENGTH])
{
  struct msg message;
  message.msgType = type;
  strcpy(message.msg, content);
  message.sender = client_id;
  if (msgsnd(server_queue, &message, MSGSZ, IPC_NOWAIT) == -1)
    show_detailed_error_and_close("Error occured - unable to send message to server");
}

void init()
{
  struct msg message;
  char content[MAX_MSG_LENGTH];
  message.msgType = INIT;
  sprintf(content, "%i", client_queue);
  strcpy(message.msg, content);
  message.sender = getpid();
  if (msgsnd(server_queue, &message, MSGSZ, IPC_NOWAIT) == -1)
    show_detailed_error_and_close("Error occured - unable to send message to server");

  if (msgrcv(client_queue, &message, MSGSZ, -COMMAND_TYPES, 0) == -1)
    show_error_and_close("Error occured - unable to connect to the server \n");

  if (message.msgType != INIT)
    show_error_and_close("Error occured - expected INIT type \n");
  sscanf(message.msg, "%d", &client_id);
  printf("\033[1;35mClient:\033[0m Client has got ID: %d \n", client_id);
}

void connect(char buf[MAX_MSG_LENGTH])
{
  send(CONNECT, buf);
}

void disconnect()
{
  char buf[MAX_MSG_LENGTH];
  send(DISCONNECT, buf);
}

void echo(char msg_content[MAX_MSG_LENGTH])
{
  if (peer_queue == -1)
  {
    show_error_and_close("Error occured - you are not connected with peer\n");
    return;
  }

  struct msg message;
  message.msgType = ECHO;
  strcpy(message.msg, msg_content);
  message.sender = client_id;
  if (msgsnd(peer_queue, &message, MSGSZ, IPC_NOWAIT) == -1)
    show_detailed_error_and_close("Error occured - unable to send message to peer");
  if (peer_pid != -1)
  {
    kill(peer_pid, SIGRTMIN);
  }
}

void list()
{
  send(LIST, "");
  struct msg msg;
  if (msgrcv(client_queue, &msg, MSGSZ, -COMMAND_TYPES, 0) == -1)
    show_error_and_close("Error occured - unable to execute command LIST \n");

  if (msg.msgType != LIST)
    show_error_and_close("Error occured - expected LIST type \n");

  printf("\033[1;35mClient:\033[0m List of active clients: \n %s \n", msg.msg);
}

void stop()
{
  send(STOP, "");
}

// utils

int starts_with(char *str, char *init_msg)
{
  if (strncmp(init_msg, str, strlen(init_msg)) == 0)
  {
    return 0;
  }
  return -1;
}

void parse_and_execute_command(char *command)
{
  if (!command)
    return;

  if (starts_with(command, "LIST") == 0)
  {
    list();
  }
  else if (starts_with(command, "CONNECT") == 0)
  {
    char tmp[100];
    char peer[MAX_MSG_LENGTH];
    if (sscanf(command, "%s %s", tmp, peer) != 2)
    {
      puts("\n received invalid command format \n Expected: CONNECT $peer_id");
      return;
    }
    connect(peer);
  }
  else if (starts_with(command, "ECHO") == 0)
  {
    char content[MAX_MSG_LENGTH];
    strcpy(content, command);
    echo(content);
  }
  else if (starts_with(command, "DISCONNECT") == 0)
  {
    disconnect();
  }
  else if (starts_with(command, "STOP") == 0)
  {
    stop();
  }
}

int main()
{
  struct sigaction action;
  action.sa_handler = signals_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGRTMIN, &action, NULL);

  signal(SIGINT, _exit);
  atexit(clean_after_exit);

  if ((server_queue = msgget(get_server_queue_key(), 0)) == -1)
    show_detailed_error_and_close("Error occured - unable to open server queue");
  if ((client_queue = msgget(get_client_queue_key(), IPC_CREAT | IPC_EXCL | 0666)) == -1)
    show_detailed_error_and_close("Error occured - unable to create client queue");

  init();
  printf("\033[1;35mClient:\033[0m Server's queue ID:\t%d \n", server_queue);

  char line[1024];
  while (1)
  {
    if (fgets(line, sizeof(line), stdin) == NULL)
      continue;
    parse_and_execute_command(line);
  }
}
