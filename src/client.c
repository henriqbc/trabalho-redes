#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "server.h"
#include "client.h"
#include "operation.h"
#include "message.h"
#include "shared/status.h"
#include "shared/utils.h"

int server_socket = -1;
char *user_nickname = NULL;
bool client_running = true;

void update_user_nickname(char *newNickname);
int connect_to_server();
void quit();

STATUS handle_user_command(char *command, char *command_arg);
STATUS handle_server_message(Message *message);

void *send_message_loop();
void *receive_message_loop();

void run_client() {
  pthread_t send_message_thread, receive_message_thread;

  pthread_create(&send_message_thread, NULL, send_message_loop, NULL);
  pthread_create(&receive_message_thread, NULL, receive_message_loop, NULL);

  pthread_join(send_message_thread, NULL);
  pthread_join(receive_message_thread, NULL);
}

void shutdown_client(int client_socket) { close(client_socket); }

// essa vai ser a função que vai rodar na thread de enviar coisas
void *send_message_loop() {
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
    printf("%d", status);
  }

  return NULL;
}

// essa vai ser a função que vai rodar na thread de receber coisas
void *receive_message_loop() {
  while (client_running) {
    if (server_socket == -1) continue;

    Message *message = receive_message(server_socket);

    STATUS status = handle_server_message(message);

    delete_message(message);

    // lida com o status
    printf("%d", status);
  }

  return NULL;
}

STATUS handle_user_command(char *command, char *command_arg) {
  Operation operation = get_operation_from_command_string(command);

  if (operation == INVALID_OPERATION) return STATUS_INVALID_COMMAND;

  Message *request =
      create_client_message_from_operation(operation, user_nickname, command, command_arg);

  if (operation == CONNECT) {
    server_socket = connect_to_server();
  } else if (operation == NICKNAME) {
    update_user_nickname(command_arg);
  } else if (operation == QUIT) {
    delete_message(request);
    quit();
    return STATUS_SUCCESS;
  }

  if (server_socket != -1)
    send_message(server_socket, request);
  else
    printf("You must first connect to the server using '/connect'.\n");

  delete_message(request);
  return STATUS_SUCCESS;
}

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
  assignString(&user_nickname, newNickname);
}

int connect_to_server() {
  int new_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (new_socket == -1) {
    printf("Error starting client socket. Shutting down.\n");
    return new_socket;
  }

  const struct sockaddr_in SERVER_ADDRESS = get_server_sockaddr();

  if (connect(new_socket, (struct sockaddr *)&SERVER_ADDRESS, sizeof(SERVER_ADDRESS)) == -1) {
    printf("Error connecting to the server. Shutting down.\n");
    shutdown_client(new_socket);
    return new_socket;
  }

  printf("Client succesfully connected and running.\n");

  return new_socket;
}

void quit() {
  client_running = false;
  shutdown_client(server_socket);
}
