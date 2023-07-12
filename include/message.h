#pragma once

#include "operation.h"

struct Message {
  char *senderNickname;
  Operation operation;
  char *content;
} typedef Message;

Message *createMessage(char *senderNickname, Operation operation, char *content);

void deleteMessage(Message *message);
