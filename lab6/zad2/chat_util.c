#include "chat_util.h"

unsigned get_cmd_priority(enum MSG_COMMAND cmd)
{
  switch (cmd)
  {
  case STOP:
    return 4;
  case DISCONNECT:
    return 3;
  case LIST:
    return 2;
  default:
    return 1;
  }
}

char *get_client_queue_name()
{
  char *name = calloc(40, sizeof(char));
  sprintf(name, "/client%d%d", getpid(), rand() % 100);
  return name;
}

enum MSG_COMMAND parse_cmd(char *cmd)
{
  if (!cmd)
    return -1;

  if (strcmp(cmd, "LIST") == 0)
  {
    return LIST;
  }
  if (strcmp(cmd, "CONNECT") == 0)
  {
    return CONNECT;
  }
  if (strcmp(cmd, "ECHO") == 0)
  {
    return ECHO;
  }
  if (strcmp(cmd, "DISCONNECT") == 0)
  {
    return DISCONNECT;
  }
  if (strcmp(cmd, "STOP") == 0)
  {
    return STOP;
  }
  if (strcmp(cmd, "INIT") == 0)
  {
    return INIT;
  }
  return -1;
}

char *cmd_to_string(enum MSG_COMMAND cmd)
{
  switch (cmd)
  {
  case LIST:
    return "LIST";
  case CONNECT:
    return "CONNECT";
  case ECHO:
    return "ECHO";
  case DISCONNECT:
    return "DISCONNECT";
  case STOP:
    return "STOP";
  case INIT:
    return "INIT";
  default:
    return NULL;
  }
}

struct msg *parse_to_msg(char content[MAX_MSG_LENGTH])
{
  struct msg *result = malloc(sizeof(struct msg));
  memset(result->msg, 0, MAX_MSG_LENGTH);
  char *token = strtok(content, SPLIT);

  enum MSG_COMMAND type = parse_cmd(token);
  if (type == -1)
  {
    free(result);
    return NULL;
  }

  token = strtok(NULL, SPLIT);
  int sender = string_to_int(token);
  if (sender == -1)
  {
    free(result);
    return NULL;
  }

  token = strtok(NULL, SPLIT);

  result->mType = type;
  result->sender = sender;
  strcpy(result->msg, "");
  if (token != NULL)
    strcat(result->msg, token);
  return result;
}

char *parse_msg_to_string(struct msg *msg)
{
  char *buffer = calloc(MAX_MSG_LENGTH, sizeof(char));
  char *type = cmd_to_string(msg->mType);
  if (type == NULL)
  {
    free(buffer);
    return NULL;
  }

  char sender[20];
  if (sprintf(sender, "%d", msg->sender) < 1)
  {
    free(buffer);
    return NULL;
  }

  strcpy(buffer, type);
  strcat(buffer, ";");
  strcat(buffer, sender);
  strcat(buffer, ";");
  strcat(buffer, msg->msg);
  return buffer;
}

int string_to_int(char *given_string)
{
  if (!given_string)
  {
    return -1;
  }
  char *tmp;
  int result = (int)strtol(given_string, &tmp, 10);
  if (strcmp(tmp, given_string) != 0)
  {
    return result;
  }
  else
  {
    return -1;
  }
}

void show_error_and_close(char *msg)
{
  fprintf(stderr, "%s", msg);
  kill(getpid(), SIGINT);
}

void show_detailed_error_and_close(char *msg)
{
  perror(msg);
  kill(getpid(), SIGINT);
}