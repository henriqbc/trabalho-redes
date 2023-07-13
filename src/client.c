#include <stdio.h>

#include "client.h"
#include "operation.h"
#include "message.h"
#include "shared/status.h"

int serverSocket = NULL;

STATUS handleUserCommand(const char *userNickname, const char *command, const char *commandArg);
STATUS handleServerMessage(Message *message);

void runClient() {
  printf("Client running.\n");
}

STATUS handleUserCommand(const char *userNickname, const char *command, const char *commandArg) {
  Operation operation = getOperationFromCommandString(command);

  Message *request = createClientMessageFromOperation(operation, userNickname, command, commandArg);
  // TODO implementar esse cara
  Message *response = talkToServer(request);  // talk to server envia e recebe messages (por isso talk, uma conversa)
                                              // se serverSocket for NULL, não faz nada e retorna NULL

  STATUS status = STATUS_SUCCESS;
  switch (operation) {
    case TEXT:
      printResponseMessage(response);
      break;
    case CONNECT:
      if (response->operation == NICKNAME_ALREADY_TAKEN) {
        status = STATUS_NICKNAME_TAKEN;
        break;
      }

      // tem algo de errado aqui, pq já rodei o talkToServer() na linha 22, então teoricamente o server socket
      // já existe né. Talvez o talk to server retorne NULL quando serverSocket == NULL, não sei.
      // mas acho que vou só mudar a ordem mesmo, talvez deixar o talk to server aqui,
      // tirar o createClientMessage de lá, algo assim

      // serverSocket = connectToServer(ip, porta);
      // if (serverSocket < 0) {
      //   status = STATUS_ERROR;
      //   break;
      // }

      printConnectedMessage();

    case PING:
      printPingMessage();
      break;
    case QUIT:
      quit();
      break;
    default:
      status = STATUS_ERROR;
  }

  deleteMessage(request);
  deleteMessage(response);
  return status;
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
