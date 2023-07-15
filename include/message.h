#pragma once

#include "operation.h"

typedef struct Message {
  char *sender_nickname;
  Operation operation;
  char *content;
} Message;

typedef struct SerializedMessage {
  byte *buffer;
  int buffer_size;
} SerializedMessage;

Message *create_message(char *sender_nickname, Operation operation, char *content);

void delete_message(Message *message);

SerializedMessage *create_serialized_message(byte *buffer, int buffer_size);

void delete_serialized_message(SerializedMessage *serialized_message);

Message *create_client_message_from_operation(Operation operation, char *sender_nickname, char *command, char *commandArg);

SerializedMessage *serialize_message(Message *message);

Message *deserialize_message(SerializedMessage *serialized_message);

void send_message(int socket, Message *message);
