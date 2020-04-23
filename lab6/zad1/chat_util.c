#include "chat_util.h"

key_t get_server_queue_key()
{
  char *home = getenv("HOME");
  if (!home)
    raise_error("unable to find env HOME \n");
  key_t s_key = ftok(home, LETTER);
  if (s_key == -1)
    raise_error("unable to get server key\n");
  return s_key;
}

key_t get_client_queue_key()
{
  char *home = getenv("HOME");
  if (!home)
    raise_error("unable to find env HOME \n");
  key_t c_key = ftok(home, getpid());
  if (c_key == -1)
    raise_error("unable to get client key\n");
  return c_key;
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
