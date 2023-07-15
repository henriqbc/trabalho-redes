#include "operation.h"

Operation get_operation_from_command_string(char *command) {
  if (strcmp(command, "/connect") == 0)
    return CONNECT;
  else if (strcmp(command, "/quit") == 0)
    return QUIT;
  else if (strcmp(command, "/ping") == 0)
    return PING;
  else if (strcmp(command, "/join") == 0)
    return JOIN;
  else if (strcmp(command, "/nickname") == 0)
    return NICKNAME;
  else if (strcmp(command, "/kick") == 0)
    return KICK;
  else if (strcmp(command, "/mute") == 0)
    return MUTE;
  else if (strcmp(command, "/unmute") == 0)
    return UNMUTE;
  else if (strcmp(command, "/whois") == 0)
    return WHOIS;
  else if (command[0] == '/')
    return INVALID_OPERATION;
  else
    return TEXT;
}
