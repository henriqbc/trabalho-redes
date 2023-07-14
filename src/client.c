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

int server_socket;
char *user_nickname = NULL;
bool client_running = true;

STATUS handle_user_command(char *command, char *commandArg);
STATUS handle_server_message(Message *message);
void update_user_nickname(char *newNickname);

void send_message_loop();
void receive_message_loop();

void run_client() {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    printf("Error starting client socket. Shutting down.\n");
    return;
  }

  const struct sockaddr_in SERVER_ADDRESS = get_server_sockaddr();

  if (connect(server_socket, (struct sockaddr *)&SERVER_ADDRESS, sizeof(SERVER_ADDRESS)) == -1) {
    printf("Error connecting to the server. Shutting down.\n");
    shutdown_client(server_socket);
    return;
  }

  printf("Client succesfully connected and running.\n");
}

void shutdown_client(int client_socket) { close(client_socket); }

// essa vai ser a função que vai rodar na thread de enviar coisas
void send_message_loop() {
  while (client_running) {
    char *user_input = readString(stdin, "\n");

    char *command = substringUntil(user_input, " \n");
    char *command_arg = substringUntil(user_input + strlen(command), "\n");

    STATUS status;
    if (user_input[0] != '/')
      status = handle_user_command(user_input, NULL);
    else
      status = handle_user_command(command, command_arg);

    free(user_input);
    free(command);
    free(command_arg);

    // lida com o status do handle userCommand
  }
}

// essa vai ser a função que vai rodar na thread de receber coisas
void receive_message_loop() {
  while (client_running) {
    // espera algo do server

    // taca no handle_server_message

    // lida com o status
  }
}

STATUS handle_user_command(char *command, char *commandArg) {
  Operation operation = get_operation_from_command_string(command);

  if (operation == INVALID_OPERATION) return STATUS_INVALID_COMMAND;

  Message *request = create_client_message_from_operation(operation, user_nickname, command, commandArg);

  if (operation == CONNECT) {
    // TODO precisa implementar esse cara aqui
    // to pensando em int connnectToServer(const char *ip, int port)
    // server_socket = connectToServer(ip, port);
    if (server_socket < 0) {
      return STATUS_FAILURE_CREATING_SOCKET;
    }
  } else if (operation == NICKNAME) {
    update_user_nickname(commandArg);
  } else if (operation == QUIT) {
    return quit();
  }

  send_message(server_socket, request);

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
  free(user_nickname);
  assignString(user_nickname, newNickname);
}
