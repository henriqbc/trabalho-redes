#pragma once

#include "string.h"

enum Operation {
  INVALID_OPERATION,

  TEXT,
  CONNECT,
  QUIT,
  PING,

  JOIN,
  CHANNEL_NOT_FOUND,
  NICKNAME,
  NICKNAME_ALREADY_TAKEN,
  KICK,
  MUTE,
  UNMUTE,
  WHOIS,
} typedef Operation;
