#pragma once

#include "operation.h"

typedef struct Message {
  char *senderNickname;
  Operation operation;
  char *content;
} Message;

Message *create_message(char *senderNickname, Operation operation, char *content);

void delete_message(Message *message);

Message *create_client_message_from_operation(Operation operation, char *senderNickname, char *command, char *commandArg);
