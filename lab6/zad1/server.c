#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "chat_util.h"

int loop = 1;
int server_queue = -1;
int active_clients = 0;

typedef struct
{
  int client_queue;
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

  if (msgctl(server_queue, IPC_RMID, NULL) == -1)
    show_error_and_close("Error occured - unable to remove server queue\n");
  else
    printf("\033[1;31mServer:\033[0m Queue has been deleted \n");
  exit(EXIT_SUCCESS);
}
void execute_command(struct msg *message)
{
  printf("\033[1;31mServer:\033[0m server has received new message \n");
  long type = message->msgType;
  switch (type)
  {
  case STOP:
    stop(message->sender);
    break;
  case DISCONNECT:
    disconnect(message->sender);
    break;
  case INIT:
    init(message->sender, message->msg);
    break;
  case CONNECT:
    connect(message->sender, message->msg);
    break;
  case LIST:
    list(message->sender);
    break;
  default:
    show_error_and_close("Error occured - wrong command format");
  }
}

void send_message(enum MSG_COMMAND type, char *msg, int client_id)
{
  if (client_id >= MAX_CLIENTS || client_id < 0 || clients[client_id].client_queue < 0)
  {
    show_error_and_close("Error occured - unable to send message to client - client doesn't exist\n");
  }

  struct msg message;
  // sender is useless field in this case
  message.sender = -1;
  message.msgType = type;
  strcpy(message.msg, msg);

  if (msgsnd(clients[client_id].client_queue, &message, MSGSZ, IPC_NOWAIT))
    show_detailed_error_and_close("Error occured - unable to send message");
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

  int client_queue = -1;
  sscanf(msg, "%d", &client_queue);
  if (client_queue < 0)
    show_error_and_close("Error occured - unable to open client queue \n");

  clients[id].client_queue = client_queue;
  clients[id].pid = client_pid;
  clients[id].current_peer = -1;
  clients[id].is_available = clients[id].is_active = 1;

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
  message.msgType = CONNECT;
  message.sender = peer_pid;
  strcpy(message.msg, content);

  if (msgsnd(clients[client_id].client_queue, &message, MSGSZ, IPC_NOWAIT) == -1)
    show_detailed_error_and_close("Error occured - unable to send connectio message about peer");
  if (clients[client_id].pid != -1)
    kill(clients[client_id].pid, SIGRTMIN);
}

void connect(int client_id, char message[MAX_MSG_LENGTH])
{
  int peer = string_to_int(message);
  if (peer == -1)
  {
    show_error_and_close("Error occured - invalid peer id \n");
    return;
  }

  if (peer >= MAX_CLIENTS || peer < 0 || clients[peer].is_active == 0 || clients[peer].is_available == 0)
  {
    show_error_and_close("Error occured - requested peer isn't active and available\n");
    return;
  }
  clients[client_id].is_available = clients[peer].is_available = 0;

  clients[client_id].current_peer = peer;
  clients[peer].current_peer = client_id;

  char res[MAX_MSG_LENGTH];
  sprintf(res, "%d", clients[client_id].client_queue);
  send_connect_message(res, clients[client_id].pid, peer);

  memset(res, 0, MAX_MSG_LENGTH);
  sprintf(res, "%d", clients[peer].client_queue);
  send_connect_message(res, clients[peer].pid, client_id);
}

void disconnect(int client_id)
{
  int peer = clients[client_id].current_peer;

  char res[MAX_MSG_LENGTH];

  send_message(DISCONNECT, res, client_id);
  int pid = -1;
  if ((pid = clients[client_id].pid) != -1)
    kill(pid, SIGRTMIN);
  send_message(DISCONNECT, res, peer);
  if ((pid = clients[peer].pid) != -1)
    kill(pid, SIGRTMIN);

  clients[client_id].current_peer = clients[peer].current_peer = -1;
  clients[client_id].is_available = clients[peer].is_available = 1;
}

int main()
{
  int i;
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    clients[i].client_queue = clients[i].current_peer = clients[i].pid = -1;
    clients[i].is_active = clients[i].is_available = 0;
  }

  struct sigaction sig_act;
  sig_act.sa_handler = exit_handler;
  sigemptyset(&sig_act.sa_mask);
  sig_act.sa_flags = 0;
  sigaction(SIGINT, &sig_act, NULL);

  if ((server_queue = msgget(get_server_queue_key(), IPC_CREAT | 0666)) == -1)
    show_detailed_error_and_close("Error occured - unable to create server queue");

  struct msg msgBuff;
  while (loop)
  {
    if (msgrcv(server_queue, &msgBuff, MSGSZ, -COMMAND_TYPES, 0) == -1)
      show_detailed_error_and_close("Error occured - unable to retrieve the message");
    execute_command(&msgBuff);
    usleep(100000);
  }

  if (msgctl(server_queue, IPC_RMID, NULL) == -1)
    show_detailed_error_and_close("Error occured - unable to remove server queue");
  else
    printf("\033[1;31mServer:\033[0m Queue has been deleted\n");

  return 0;
}
