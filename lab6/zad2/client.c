#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <fcntl.h>
#include "chat_util.h"

int server_queue = -1;
int client_queue = -1;
int peer_queue = -1;
int peer_pid = -1;
int client_id = -1;
char *queue_name = NULL;
int working = 1;

void communication_handler(int sig_number)
{
  char buffer[MAX_MSG_LENGTH];
  if (mq_receive(client_queue, buffer, MAX_MSG_LENGTH, NULL) == -1)
    show_detailed_error_and_close("Error occured - unable to receive message");

  struct msg *message = parse_to_msg(buffer);
  if (message == NULL)
  {
    show_error_and_close("Error occured - failed to parse message\n");
  }

  switch (message->mType)
  {
  case STOP:
  {
    exit(EXIT_SUCCESS);
  }
  case CONNECT:
  {
    peer_queue = mq_open(message->msg, O_WRONLY);
    if (peer_queue == -1)
    {
      show_detailed_error_and_close("Error occured - unable to connect to peer queue");
    }
    peer_pid = message->sender;
    printf("\033[1;35mClient:\033[0m Connected with peer queue ID:\t%d \n", peer_queue);
    break;
  }
  case DISCONNECT:
  {
    peer_queue = -1;
    peer_pid = -1;
    printf("\033[1;35mClient:\033[0m Disconnected from peer \n");
    break;
  }
  case ECHO:
  {
    printf("Peer: %s\n", message->msg);
    break;
  }
  }
  free(message);
}

void exit_handler(int sig_number)
{
  puts("SIGINT received - closing");
  stop();
}

void exit_cleanup()
{
  if (mq_close(client_queue) == -1)
    show_detailed_error_and_close("Error occured - unable to close client queue");
  if (mq_close(server_queue) == -1)
    show_detailed_error_and_close("Error occured - unable to close server queue");
  if (mq_unlink(queue_name) == -1)
    show_detailed_error_and_close("Error occured - unable to unlink client queue");

  printf("\033[1;35mClient:\033[0m Queue has been closed and  deleted\n");
}

void send(enum MSG_COMMAND type, char content[MAX_MSG_LENGTH])
{
  struct msg message;
  message.mType = type;
  strcpy(message.msg, content);
  message.sender = client_id;

  char *buffer = parse_msg_to_string(&message);
  if (mq_send(server_queue, buffer, MAX_MSG_LENGTH, get_cmd_priority(type)) == -1)
    show_detailed_error_and_close("Error occured - unable to send message to server");
}

void init()
{
  struct msg message;
  char content[MAX_MSG_LENGTH];
  message.mType = INIT;
  sprintf(content, "%s", queue_name);
  strcpy(message.msg, content);
  message.sender = getpid();

  char *buffer = parse_msg_to_string(&message);
  if (mq_send(server_queue, buffer, MAX_MSG_LENGTH, get_cmd_priority(INIT)) == -1)
    show_detailed_error_and_close("Error occured - unable to send message to server");
  free(buffer);

  char response[MAX_MSG_LENGTH];
  if (mq_receive(client_queue, response, MAX_MSG_LENGTH, NULL) == -1)
    show_error_and_close("Error occured - unable to connect with server \n");

  struct msg *response_message = parse_to_msg(response);
  if (response_message == NULL)
    show_error_and_close("Error occured - unable to read INIT message");

  if (response_message->mType != INIT)
    show_error_and_close("Error occured - expected INIT type \n");
  sscanf(response_message->msg, "%d", &client_id);
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
    show_error_and_close("Error occured - must be connected with peer first\n");
    return;
  }

  struct msg message;
  message.mType = ECHO;
  strcpy(message.msg, msg_content);
  message.sender = client_id;

  char *buffer = parse_msg_to_string(&message);
  if (mq_send(peer_queue, buffer, MAX_MSG_LENGTH, get_cmd_priority(ECHO)) == -1)
    show_detailed_error_and_close("Error occured - unable to send message to peer");
  if (peer_pid != -1)
  {
    kill(peer_pid, SIGRTMIN);
  }
  free(buffer);
}

void list()
{
  send(LIST, "");
  char buffer[MAX_MSG_LENGTH];
  if (mq_receive(client_queue, buffer, MAX_MSG_LENGTH, NULL) == -1)
    show_error_and_close("Error occured - unable to get LIST \n");

  struct msg *message = parse_to_msg(buffer);

  if (message->mType != LIST)
    show_error_and_close("Error occured - expected LIST type \n");

  printf("\033[1;35mClient:\033[0m List of active clients: \n %s \n", message->msg);
  free(message);
}

void stop()
{
  send(STOP, "");
}

int starts_with(char *str, char *init_msg)
{
  if (strncmp(init_msg, str, strlen(init_msg)) == 0)
  {
    return 0;
  }
  return -1;
}

void parse_and_execute_command(char *cmd)
{
  if (!cmd)
    return;

  if (starts_with(cmd, "LIST") == 0)
  {
    list();
  }
  else if (starts_with(cmd, "CONNECT") == 0)
  {
    char tmp[100];
    char peer[MAX_MSG_LENGTH];
    if (sscanf(cmd, "%s %s", tmp, peer) != 2)
    {
      puts("\n invalid command format \n");
      return;
    }
    connect(peer);
  }
  else if (starts_with(cmd, "ECHO") == 0)
  {
    char buffer[MAX_MSG_LENGTH];
    strcpy(buffer, cmd);
    echo(buffer);
  }
  else if (starts_with(cmd, "DISCONNECT") == 0)
  {
    disconnect();
  }
  else if (starts_with(cmd, "STOP") == 0)
  {
    stop();
  }
}

int main()
{
  struct sigaction action;
  action.sa_handler = communication_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGRTMIN, &action, NULL);

  signal(SIGINT, exit_handler);
  atexit(exit_cleanup);

  queue_name = get_client_queue_name();

  if ((server_queue = mq_open(SERVER_NAME, O_WRONLY)) == -1)
    show_detailed_error_and_close("Error occured - unable to open server queue");

  struct mq_attr attrs;
  attrs.mq_maxmsg = MAX_MESSAGES;
  attrs.mq_msgsize = MAX_MSG_LENGTH;

  if ((client_queue = mq_open(queue_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attrs)) == -1)
    show_detailed_error_and_close("Error occured - unable to create client queue");

  init();
  printf("\033[1;35mClient:\033[0m Server's queue ID:\t%d \n", server_queue);
  printf("\033[1;35mClient:\033[0m Client's queue name:\t%s \n", queue_name);

  char line[1024];
  while (1)
  {
    if (fgets(line, sizeof(line), stdin) == NULL)
      continue;
    parse_and_execute_command(line);
  }
}
