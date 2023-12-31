#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "shared/utils.h"
#include "shared/status.h"
#include "shared/constants.h"

#define MESSAGE_SERIALIZATION_SEPARATOR '|'
#define MESSAGE_SERIALIZATION_SEPARATOR_STRING "|"

Message *create_message(char *sender_nickname, Operation operation, char *content) {

  Message *message = malloc(sizeof(Message));

  message->sender_nickname = NULL;
  assignString(&(message->sender_nickname), sender_nickname);
  message->operation = operation;
  assignString(&(message->content), content);

  return message;
}

void delete_message(Message *message) {

  free(message->sender_nickname);
  free(message->content);

  free(message);
}

void delete_message_content(Message *message) {

  free(message->sender_nickname);
  free(message->content);
}

SerializedMessage *create_serialized_message(byte *buffer, int buffer_size) {
  SerializedMessage *serialized_message = malloc(sizeof(SerializedMessage));

  serialized_message->buffer = malloc(buffer_size);
  memcpy(serialized_message->buffer, buffer, buffer_size);
  serialized_message->buffer_size = buffer_size;

  return serialized_message;
}

void delete_serialized_message(SerializedMessage *serialized_message) {
  free(serialized_message->buffer);

  free(serialized_message);
}

Message *create_client_message_from_operation(Operation operation, char *sender_nickname,
                                              char *command, char *commandArg) {

  Message *message =
      create_message(sender_nickname, operation, operation == TEXT ? command : commandArg);
  return message;
}

SerializedMessage *serialize_message(Message *message) {
  int buffer_size = sizeof(int);  // to store the buffer_size
  byte *buffer = malloc(buffer_size);

  // SENDER NICKNAME
  int sender_nickname_size =
      message->sender_nickname != NULL ? strlen(message->sender_nickname) : 0;
  buffer = realloc(buffer, buffer_size + sender_nickname_size + 1);
  memcpy(buffer + buffer_size, message->sender_nickname, sender_nickname_size);

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
  int content_size = message->content != NULL ? strlen(message->content) : 0;
  buffer = realloc(buffer, buffer_size + content_size + 1);
  memcpy(buffer + buffer_size, message->content, content_size);

  // add separator
  buffer_size += content_size + 1;
  buffer[buffer_size - 1] = MESSAGE_SERIALIZATION_SEPARATOR;

  // add final '\0' to indicate end of message
  buffer_size++;
  buffer = realloc(buffer, buffer_size);
  buffer[buffer_size - 1] = '\0';

  // add buffer_size header
  memcpy(buffer, &buffer_size, sizeof(int));

  SerializedMessage *serialized_message = create_serialized_message(buffer, buffer_size);
  free(buffer);

  return serialized_message;
}

Message *deserialize_message(SerializedMessage *serialized_message) {

  int cursor = sizeof(int);  // skip buffer_size

  char *sender_nickname = substringUntil(serialized_message->buffer + cursor,
                                         MESSAGE_SERIALIZATION_SEPARATOR_STRING);
  cursor += (sender_nickname == NULL ? 0 : strlen(sender_nickname)) + 1;

  Operation operation;
  memcpy(&operation, serialized_message->buffer + cursor, sizeof(Operation));

  cursor += sizeof(Operation) + 1;

  char *content = substringUntil(serialized_message->buffer + cursor,
                                 MESSAGE_SERIALIZATION_SEPARATOR_STRING);

  Message *message = create_message(sender_nickname, operation, content);

  free(sender_nickname);
  free(content);

  return message;
}

void send_message(int socket, Message *message) {
  SerializedMessage *serialized_message = serialize_message(message);

  int cursor = 0;
  while (cursor < serialized_message->buffer_size) {
    int packet_size = min(MAX_PACKET_SIZE, serialized_message->buffer_size - cursor);

    write(socket, serialized_message->buffer + cursor, packet_size);

    cursor += packet_size;
  }

  delete_serialized_message(serialized_message);
}

// Blocking function
Message *receive_message(int socket) {

  byte *buffer = malloc(MAX_PACKET_SIZE);

  // read first guaranteed packet
  int bytes_read =
      read(socket, buffer,
           MAX_PACKET_SIZE);  // works fine if message is smaller than MAX_PACKET_SIZE

  if (bytes_read < sizeof(Operation)) {
    free(buffer);
    return NULL;
  }

  int message_buffer_size;
  memcpy(&message_buffer_size, buffer, sizeof(int));

  // resize buffer to store all message bytes
  buffer = realloc(buffer, message_buffer_size);

  // read remaining packets, always capping to MAX_PACKET_SIZE bytes at a time
  int cursor = bytes_read;
  while (cursor < message_buffer_size) {
    int packet_size = min(MAX_PACKET_SIZE, message_buffer_size - cursor);

    bytes_read = read(socket, buffer + cursor, packet_size);

    cursor += bytes_read;
  }

  SerializedMessage *serialized_message =
      create_serialized_message(buffer, message_buffer_size);
  Message *message = deserialize_message(serialized_message);

  free(buffer);
  delete_serialized_message(serialized_message);

  return message;
}
