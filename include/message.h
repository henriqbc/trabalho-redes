#pragma once

#include "operation.h"

typedef struct Message {
  char *sender_nickname;
  Operation operation;
  char *content;
} Message;

Message *create_message(char *sender_nickname, Operation operation, char *content);

void delete_message(Message *message);

Message *create_client_message_from_operation(Operation operation, char *sender_nickname, char *command, char *commandArg);

byte *serialize_message(Message *message);

Message *deserialize_message(byte *serialized_message);

void send_message(int socket, Message *message);
