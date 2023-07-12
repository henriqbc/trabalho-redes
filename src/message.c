#include <malloc.h>

#include "message.h"

Message *createMessage(char *senderNickname, Operation operation, char *content) {

  Message *message = malloc(sizeof(Message));

  message->senderNickname = senderNickname;
  message->operation = operation;
  message->content = content;

  return message;
}

void deleteMessage(Message *message) {
  free(message->senderNickname);
  free(message->content);

  free(message);
}
