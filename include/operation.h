#pragma once

#include "string.h"

enum Operation {
  INVALID_OPERATION,
  UNAUTHORIZED,

  TEXT,
  CONNECT,
  QUIT,
  PING,

  JOIN,
  CHANNEL_NOT_FOUND,
  NICKNAME,
  NICKNAME_ALREADY_TAKEN,
  NICKNAME_ALREADY_TAKEN_CONNECT,
  KICK_SUCCEEDED,
  KICK,
  MUTE,
  UNMUTE,
  WHOIS,
} typedef Operation;

Operation get_operation_from_command_string(char *command);
