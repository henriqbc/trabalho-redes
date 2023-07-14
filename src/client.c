#include <stdlib.h>
#include <stdio.h>

#include "client.h"
#include "operation.h"
#include "message.h"
#include "shared/status.h"
#include "shared/utils.h"

int serverSocket = 0;
char *userNickname = NULL;

STATUS handleUserCommand(char *userNickname, char *command, char *commandArg);
STATUS handleServerMessage(Message *message);
void updateUserNickname(char *newNickname);

void runClient() {
  printf("Client running.\n");
}

STATUS handleUserCommand(char *userNickname, char *command, char *commandArg) {
  Operation operation = getOperationFromCommandString(command);

  Message *request = createClientMessageFromOperation(operation, userNickname, command, commandArg);

  if (operation == CONNECT) {
    // TODO precisa implementar esse cara aqui
    // to pensando em int connnectToServer(const char *ip, int port)
    // serverSocket = connectToServer(ip, port);
    if (serverSocket < 0) {
      return STATUS_FAILURE_CREATING_SOCKET;
    }
  } else if (operation == NICKNAME) {
    updateUserNickname(commandArg);
  } else if (operation == QUIT) {
    return quit();
  }

  sendMessage(serverSocket, request);

  deleteMessage(request);
  return STATUS_SUCCESS;
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

  return STATUS_ERROR;
}

void updateUserNickname(char *newNickname) {
  free(userNickname);
  assignString(userNickname, newNickname);
}
