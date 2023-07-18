#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "client.h"
#include "operation.h"
#include "message.h"
#include "shared/status.h"
#include "shared/utils.h"

int server_socket = -1;
char *user_nickname = NULL;
bool client_running = true;

void define_sigint_handler();
void define_user_nickname();
void print_greetings_message();

void update_user_nickname(char *newNickname);
int connect_to_server();
void quit();

STATUS handle_user_command(char *command, char *command_arg);
STATUS handle_server_message(Message *message);

void *send_message_loop();
void *receive_message_loop();

void run_client() {
  define_sigint_handler();

  define_user_nickname();

  print_greetings_message();

  pthread_t send_message_thread, receive_message_thread;

  pthread_create(&send_message_thread, NULL, send_message_loop, NULL);
  pthread_create(&receive_message_thread, NULL, receive_message_loop, NULL);

  pthread_join(send_message_thread, NULL);
  pthread_join(receive_message_thread, NULL);
}

void shutdown_client(int socket) {
  server_socket = -1;
  shutdown(socket, SHUT_RDWR);
  close(socket);
}

void *send_message_loop() {
  while (client_running) {
    char *user_input = readString(stdin, "\n");

    if (user_input == NULL) continue;

    char *command = substringUntil(user_input, " \n");
    char *command_arg = strlen(user_input) != strlen(command)
                            ? substringUntil(user_input + strlen(command) + 1, "\n")
                            : NULL;

    STATUS status;
    if (user_input[0] != '/')
      status = handle_user_command(user_input, NULL);
    else
      status = handle_user_command(command, command_arg);

    if (status == STATUS_INVALID_COMMAND) printf("'%s' is an invalid command.\n", command);

    free(user_input);
    free(command);
    free(command_arg);
  }

  return NULL;
}

void *receive_message_loop() {
  while (client_running) {
    if (server_socket == -1) continue;

    Message *message = receive_message(server_socket);
    if (!message) continue;

    handle_server_message(message);

    delete_message(message);
  }

  return NULL;
}

STATUS handle_user_command(char *command, char *command_arg) {
  Operation operation = get_operation_from_command_string(command);

  if (operation == INVALID_OPERATION) return STATUS_INVALID_COMMAND;

  Message *request =
      create_client_message_from_operation(operation, user_nickname, command, command_arg);

  if (operation == CONNECT) {
    if (server_socket != -1) {
      printf("\nAlready connected to the server.\n\n");
      delete_message(request);
      return STATUS_ERROR;
    }

    server_socket = connect_to_server();

    if (server_socket == -1) {
      printf("\nError connecting to the server.\n\n");
      delete_message(request);
      return STATUS_ERROR;
    }
  } else if (operation == QUIT) {
    if (server_socket != -1) send_message(server_socket, request);

    delete_message(request);
    quit();
    return STATUS_SUCCESS;
  } else if (operation == NICKNAME && server_socket == -1) {
    update_user_nickname(request->content);
    printf("\nSuccesfuly updated your nickname to %s!\n\n", request->content);
    delete_message(request);
    return STATUS_SUCCESS;
  }

  if (server_socket != -1)
    send_message(server_socket, request);
  else
    printf("\nYou must first connect to the server using '/connect'.\n\n");

  delete_message(request);
  return STATUS_SUCCESS;
}

STATUS handle_server_message(Message *message) {
  switch (message->operation) {
    case TEXT:
      // if the user sent a message, clear the message that he wrote
      // and print instead the message that the server sent
      if (strcmp(message->sender_nickname, user_nickname) == 0)
        printf("\033[A\33[2K\r");  // clears the line the user sent

      printf("\033[A\33[2K\r");  // clears the extra line after receiving

      printf("\n%s: %s\n\n", message->sender_nickname, message->content);
      break;
    case CONNECT:
      printf("\nSuccesfully connected to the server!\n\n");
      break;
    case PING:
      printf("\nPong!\n\n");
      break;
    case JOIN:
      printf("\nSuccesfuly joined the channel!\n\n");
      break;
    case CHANNEL_NOT_FOUND:
      printf("\nChannel not found.\n\n");
      break;
    case NICKNAME:
      update_user_nickname(message->content);
      printf("\nSuccesfuly updated your nickname to %s!\n\n", message->content);
      break;
    case NICKNAME_ALREADY_TAKEN:
      update_user_nickname(message->content);
      printf("\nThe nickname is currently unavailable, please choose another one.\n\n");
      break;
    case NICKNAME_ALREADY_TAKEN_CONNECT:
      update_user_nickname(message->content);
      printf("\nThe nickname is currently unavailable, please choose another one.\n\n");
      shutdown_client(server_socket);
      break;
    case KICK:
      printf("Unfortunately, you were kicked from this channel by the administrator.\n\n");
      break;
    case KICK_SUCCEEDED:
      printf("\nSuccesfully kicked the user.\n\n");
      break;
    case USER_NOT_FOUND:
      printf("\nUser not found.\n\n");
      break;
    case WHOIS:
      printf("\nThe desired ip is: %s\n\n.", message->content);
      break;
    case UNAUTHORIZED:
      printf("\nYou must be an admin to use this command.\n\n");
      break;
    case SERVER_RESPONSE:
      printf("\n%s\n\n", message->content);
      break;
    default:
      return STATUS_ERROR;
  }

  return STATUS_ERROR;
}

void define_user_nickname() {
  printf("Enter your nickname: ");
  user_nickname = readString(stdin, "\n");
}

void update_user_nickname(char *newNickname) {
  free(user_nickname);
  assignString(&user_nickname, newNickname);
}

int connect_to_server() {
  int new_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (new_socket == -1) {
    return -1;
  }

  const struct sockaddr_in SERVER_ADDRESS = get_server_sockaddr();

  if (connect(new_socket, (struct sockaddr *)&SERVER_ADDRESS, sizeof(SERVER_ADDRESS)) == -1) {
    shutdown_client(new_socket);
    return -1;
  }

  return new_socket;
}

void quit() {
  client_running = false;

  free(user_nickname);

  if (server_socket != -1) shutdown_client(server_socket);
}

void print_greetings_message() {
  printf("\n");
  printf("Connect to the server with '/connect'.\n");
  printf("Join a channel with '/join <channel name>'.\n");
  printf("Change your nickname with '/nickname <new nickname>'.\n");
  printf("Quit the program with '/quit'.\n");
  printf("\n");
}

void sigint_handler() { printf("\nTo exit the application, use '/quit'.\n\n"); }

void define_sigint_handler() {
  struct sigaction act;
  act.sa_handler = sigint_handler;
  act.sa_mask = (__sigset_t){.__val = 0};
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);
}
