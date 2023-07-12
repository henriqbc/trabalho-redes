#include <stdio.h>

#include "client.h"
#include "operation.h"
#include "message.h"
#include "shared/status.h"

STATUS handleUserCommand(const char *command);
STATUS handleServerMessage(Message *message);

void runClient() {
  printf("Client running.\n");
}

STATUS handleUserCommand(const char *command) {
  Operation operation = getOperationFromCommandString(command);

  switch (operation) {
    case TEXT:
      break;
    case CONNECT:
      break;
    case QUIT:
      break;
    case PING:
      break;
    default:
      return STATUS_ERROR;
  }
}

STATUS handleServerMessage(Message *message) {
  switch (message->operation) {
    case TEXT:
      break;
    case CONNECT:
      break;
    case QUIT:
      break;
    case PING:
      break;
    default:
      return STATUS_ERROR;
  }
}
