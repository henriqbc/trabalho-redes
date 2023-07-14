#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>

#include "server.h"
#include "client.h"
#include "operation.h"
#include "message.h"
#include "shared/status.h"
#include "shared/utils.h"

int serverSocket = 0;
char *userNickname = NULL;
bool clientRunning = true;

STATUS handle_user_command(char *userNickname, char *command, char *commandArg);
STATUS handle_server_message(Message *message);
void update_user_nickname(char *newNickname);

void send_message_loop();
void receive_message_loop();

void run_client() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("Error starting client socket. Shutting down.\n");
    return;
  }

  const struct sockaddr_in SERVER_ADDRESS = get_server_sockaddr();

  if (connect(sockfd, (struct sockaddr *)&SERVER_ADDRESS, sizeof(SERVER_ADDRESS)) == -1) {
    printf("Error connecting to the server. Shutting down.\n");
    shutdown_client(sockfd);
    return;
  }

  printf("Client succesfully connected and running.\n");
}

void shutdown_client(int client_socket) { close(client_socket); }

// essa vai ser a função que vai rodar na thread de enviar coisas
void send_message_loop() {
  while (clientRunning) {
    // pega o que o usuário escreveu

    // parseia em command e commandArg

    // taca no handle_user_command

    // lida com o status do handle userCommand
  }
}

// essa vai ser a função que vai rodar na thread de receber coisas
void receive_message_loop() {
  while (clientRunning) {
    // espera algo do server

    // taca no handle_server_message

    // lida com o status
  }
}

STATUS handle_user_command(char *userNickname, char *command, char *commandArg) {
  Operation operation = get_operation_from_command_string(command);

  Message *request = create_client_message_from_operation(operation, userNickname, command, commandArg);

  if (operation == CONNECT) {
    // TODO precisa implementar esse cara aqui
    // to pensando em int connnectToServer(const char *ip, int port)
    // serverSocket = connectToServer(ip, port);
    if (serverSocket < 0) {
      return STATUS_FAILURE_CREATING_SOCKET;
    }
  } else if (operation == NICKNAME) {
    update_user_nickname(commandArg);
  } else if (operation == QUIT) {
    return quit();
  }

  send_message(serverSocket, request);

  delete_message(request);
  return STATUS_SUCCESS;
}

// vai ser responsabilidade desse cara printar as mensagens na tela
STATUS handle_server_message(Message *message) {
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

void update_user_nickname(char *newNickname) {
  free(userNickname);
  assignString(userNickname, newNickname);
}
