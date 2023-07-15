#pragma once

#include "user.h"

// User communication channel
typedef struct Channel {
  char *name;
  int members_qty;
  User *members;
} Channel;
