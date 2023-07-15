#pragma once

#include "string.h"

enum Operation {
  INVALID_OPERATION,

  TEXT,
  CONNECT,
  QUIT,
  PING,

  JOIN,
  NICKNAME,
  NICKNAME_ALREADY_TAKEN,
  KICK,
  MUTE,
  UNMUTE,
  WHOIS,
} typedef Operation;

Operation get_operation_from_command_string(char *command);
