#include <malloc.h>
#include <string.h>

#include "message.h"
#include "shared/utils.h"

Message *createMessage(char *senderNickname, Operation operation, char *content) {

  Message *message = malloc(sizeof(Message));

  assignString(message->senderNickname, senderNickname);
  message->operation = operation;
  assignString(message->content, content);

  message->senderNickname = senderNickname;
  message->content = content;

  return message;
}

void deleteMessage(Message *message) {
  free(message->senderNickname);
  free(message->content);

  free(message);
}
