#include <malloc.h>
#include <string.h>

#include "message.h"
#include "shared/utils.h"
#include "shared/status.h"
#include "shared/constants.h"

#define MESSAGE_SERIALIZATION_SEPARATOR '|'
#define MESSAGE_SERIALIZATION_SEPARATOR_STRING "|"
#define MAX_PACKET_SIZE 4096

Message *create_message(char *sender_nickname, Operation operation, char *content) {

  Message *message = malloc(sizeof(Message));

  message->sender_nickname = NULL;
  assignString(message->sender_nickname, sender_nickname);
  message->operation = operation;
  assignString(message->content, content);

  message->sender_nickname = sender_nickname;
  message->content = content;

  return message;
}

void delete_message(Message *message) {
  free(message->sender_nickname);
  free(message->content);

  free(message);
}

SerializedMessage *create_serialized_message(byte *buffer, int buffer_size) {
  SerializedMessage *serialized_message = malloc(sizeof(SerializedMessage));

  serialized_message->buffer = NULL;
  assignString(serialized_message->buffer, buffer);
  serialized_message->buffer_size = buffer_size;

  return serialized_message;
}

void delete_serialized_message(SerializedMessage *serialized_message) {
  free(serialized_message->buffer);

  free(serialized_message);
}

Message *create_client_message_from_operation(Operation operation, char *sender_nickname, char *command, char *commandArg) {
  Message *message = create_message(sender_nickname, operation, operation == TEXT ? command : commandArg);
  return message;
}

SerializedMessage *serialize_message(Message *message) {
  byte *buffer = NULL;
  int buffer_size = 0;

  // SENDER NICKNAME
  int sender_nickname_size = strlen(message->sender_nickname);
  buffer = realloc(buffer, sender_nickname_size + 1);
  memcpy(buffer, message->sender_nickname, sender_nickname_size);

  // add separator
  buffer_size += sender_nickname_size + 1;
  buffer[buffer_size - 1] = MESSAGE_SERIALIZATION_SEPARATOR;

  // OPERATION
  buffer = realloc(buffer, buffer_size + sizeof(Operation) + 1);
  memcpy(buffer + buffer_size, &message->operation, sizeof(Operation));

  // add separator
  buffer_size += sizeof(Operation) + 1;
  buffer[buffer_size - 1] = MESSAGE_SERIALIZATION_SEPARATOR;

  // CONTENT
  int content_size = strlen(message->content);
  buffer = realloc(buffer, buffer_size + content_size + 1);
  memcpy(buffer + buffer_size, message->content, content_size);

  // add separator
  buffer_size += content_size + 1;
  buffer[buffer_size - 1] = MESSAGE_SERIALIZATION_SEPARATOR;

  SerializedMessage *serialized_message = create_serialized_message(buffer, buffer_size);
  free(buffer);

  return serialize_message;
}

Message *deserialize_message(SerializedMessage *serialized_message) {

  int cursor = 0;

  char *sender_nickname = substringUntil(serialized_message->buffer, MESSAGE_SERIALIZATION_SEPARATOR_STRING);
  cursor += strlen(sender_nickname) + 1;

  Operation *operation = (Operation *)substringUntil(
      serialized_message->buffer + cursor, MESSAGE_SERIALIZATION_SEPARATOR_STRING);
  cursor += sizeof(Operation) + 1;

  char *content = substringUntil(serialized_message->buffer + cursor, MESSAGE_SERIALIZATION_SEPARATOR_STRING);

  Message *message = create_message(sender_nickname, *operation, content);

  free(sender_nickname);
  free(operation);
  free(content);

  return message;
}

void send_message(int socket, Message *message) {
  byte buffer[MAX_PACKET_SIZE];
}
