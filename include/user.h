#pragma once

#include <stdbool.h>

// Server client
typedef struct User {
  char *nickname;
  int socket_fd;
  bool muted;
  char *ip;
} User;
