#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <mqueue.h>
#include "chat_util.h"

int working = 1;
int server_queue = -1;
int active_clients = 0;

typedef struct
{
  int client_queue;
  char queue_name[80];
  int current_peer;
  int is_active;
  int is_available;
  pid_t pid;
} client_t;

client_t clients[MAX_CLIENTS];

void exit_handler(int sig_number)
{

  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i].client_queue != -1)
    {
      stop(i);
    }
  }

  if (mq_close(server_queue) == -1)
    show_detailed_error_and_close("Error occured - unable to delete server queue\n");
  if (mq_unlink(SERVER_NAME) == -1)
    show_detailed_error_and_close("Error occured - unable to unlink server queue\n");
  printf("\033[1;31mServer:\033[0m Queue has been removed \n");
  exit(EXIT_SUCCESS);
}

void execute_command(struct msg *message)
{
  printf("\033[1;31mServer:\033[0m server has received new message \n");
  long type = message->mType;
  switch (type)
  {
  case STOP:
    stop(message->sender);
    break;
  case INIT:
    init(message->sender, message->msg);
    break;
  case LIST:
    list(message->sender);
    break;
  case CONNECT:
    connect(message->sender, message->msg);
    break;
  case DISCONNECT:
    disconnect(message->sender);
    break;
  default:
    show_error_and_close("Error occured - invalid command");
  }
}

void send_message(enum MSG_COMMAND type, char *msg, int client_id)
{
  if (client_id >= MAX_CLIENTS || client_id < 0 || clients[client_id].client_queue < 0)
  {
    show_error_and_close("Error occured - unable to send message to client, client doesn't exist\n");
  }

  struct msg message;
  message.sender = 0;
  message.mType = type;
  strcpy(message.msg, msg);

  char *buffer = parse_msg_to_string(&message);
  if (mq_send(clients[client_id].client_queue, buffer, MAX_MSG_LENGTH, get_cmd_priority(type)) == -1)
    show_detailed_error_and_close("Error occured - unable to send message");
  free(buffer);
}

void init(int client_pid, char msg[MAX_MSG_LENGTH])
{
  int id;
  for (id = 0; id < MAX_CLIENTS; id++)
  {
    if (clients[id].client_queue == -1)
      break;
  }

  if (id >= MAX_CLIENTS)
    show_error_and_close("Error occured - too many clients\n");

  int client_queue;
  if ((client_queue = mq_open(msg, O_WRONLY)) == -1)
    show_error_and_close("Error occured - unable to open client queue \n");

  clients[id].client_queue = client_queue;
  clients[id].pid = client_pid;
  clients[id].current_peer = -1;
  clients[id].is_available = clients[id].is_active = 1;
  strcpy(clients[id].queue_name, msg);

  printf("\033[1;31mServer:\033[0m New client has been added: %d \n", id);

  char toClient[MAX_MSG_LENGTH];
  sprintf(toClient, "%d", id);
  send_message(INIT, toClient, id);
  active_clients++;
}

void stop(int client_id)
{
  if (client_id >= 0 && client_id < MAX_CLIENTS)
  {
    char buf[MAX_MSG_LENGTH];
    send_message(STOP, buf, client_id);
    if (clients[client_id].pid != -1)
      kill(clients[client_id].pid, SIGRTMIN);
    if (clients[client_id].current_peer != -1)
    {
      disconnect(client_id);
    }

    clients[client_id].client_queue = clients[client_id].current_peer = clients[client_id].pid = -1;
    clients[client_id].is_available = clients[client_id].is_active = 0;
    active_clients--;
  }
}

void list(int client_id)
{
  printf("\033[1;31mServer:\033[0m LIST for client: %d \n", client_id);
  char response[MAX_MSG_LENGTH], buf[MAX_MSG_LENGTH];
  strcpy(response, "");
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i].client_queue >= 0)
    {
      sprintf(buf, "ID: %d, AVAILABLE: %d, ACTIVE: %d \n", i, clients[i].is_available, clients[i].is_active);
      strcat(response, buf);
    }
  }
  send_message(LIST, response, client_id);
}

void send_connect_message(char content[MAX_MSG_LENGTH], int peer_pid, int client_id)
{
  struct msg message;
  message.mType = CONNECT;
  message.sender = peer_pid;
  strcpy(message.msg, content);

  char *buffer = parse_msg_to_string(&message);
  if (mq_send(clients[client_id].client_queue, buffer, MAX_MSG_LENGTH, get_cmd_priority(CONNECT)) == -1)
    show_detailed_error_and_close("Error occured - unable to send connection message about peer");
  if (clients[client_id].pid != -1)
    kill(clients[client_id].pid, SIGRTMIN);
  free(buffer);
}

void connect(int client_id, char message[MAX_MSG_LENGTH])
{
  int peer = convert_to_num(message);
  if (peer == -1)
  {
    show_error_and_close("Error occured - invalid peer id \n");
    return;
  }

  if (clients[client_id].is_available == 0)
  {
    show_error_and_close("Error occured - client tried to connect with another peer while being unavailable\n");
  }

  if (peer >= MAX_CLIENTS || peer < 0 || clients[peer].is_active == 0 || clients[peer].is_available == 0)
  {
    show_error_and_close("Error occured - requested peer is not active and available\n");
    return;
  }
  clients[client_id].is_available = 0;
  clients[peer].is_available = 0;

  clients[client_id].current_peer = peer;
  clients[peer].current_peer = client_id;

  char response[MAX_MSG_LENGTH];
  strcpy(response, clients[client_id].queue_name);
  send_connect_message(response, clients[client_id].pid, peer);

  memset(response, 0, MAX_MSG_LENGTH);
  strcpy(response, clients[peer].queue_name);
  send_connect_message(response, clients[peer].pid, client_id);
}

void disconnect(int client_id)
{
  int peer = clients[client_id].current_peer;

  char response[MAX_MSG_LENGTH];

  send_message(DISCONNECT, response, client_id);
  int pid = -1;
  if ((pid = clients[client_id].pid) != -1)
    kill(pid, SIGRTMIN);
  send_message(DISCONNECT, response, peer);
  if ((pid = clients[peer].pid) != -1)
    kill(pid, SIGRTMIN);

  clients[client_id].current_peer = clients[peer].current_peer = -1;
  clients[client_id].is_available = clients[peer].is_available = 1;
}

int main()
{
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    clients[i].client_queue = clients[i].current_peer = clients[i].pid = -1;
    clients[i].is_active = clients[i].is_available = 0;
  }
  //handle SIGINT -> delete queue
  struct sigaction action;
  action.sa_handler = exit_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);

  struct mq_attr attrs;
  attrs.mq_maxmsg = MAX_MESSAGES;
  attrs.mq_msgsize = MAX_MSG_LENGTH;

  if ((server_queue = mq_open(SERVER_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, &attrs)) == -1)
    show_detailed_error_and_close("Error occured - unable to create server queue");

  char buffer[MAX_MSG_LENGTH];
  while (working)
  {
    if (mq_receive(server_queue, buffer, MAX_MSG_LENGTH, NULL) == -1)
      show_detailed_error_and_close("Error occured - unable to receive message");
    struct msg *message = parse_to_msg(buffer);
    if (message == NULL)
    {
      show_error_and_close("Error occured - unable to receive message\n");
    }
    execute_command(message);
    free(message);
    usleep(500000);
  }

  return 0;
}
