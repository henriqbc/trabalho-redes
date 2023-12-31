#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "client.h"

void handle_wrong_program_usage();

int main(int argc, char const *argv[]) {

  if (argc != 2) handle_wrong_program_usage();

  if (strcmp(argv[1], "server") == 0) {
    int server_socket = create_server();
    start_server(server_socket);
  } else if (strcmp(argv[1], "client") == 0)
    run_client();
  else
    handle_wrong_program_usage();

  return 0;
}

void handle_wrong_program_usage() {
  printf("Program usage: ./executable <server|client>.\n");
  printf("\tIf you want to run the server, use: './chat server'.\n");
  printf("\tIf you want to run the client, use: './chat client'.\n");

  exit(EXIT_FAILURE);
}
