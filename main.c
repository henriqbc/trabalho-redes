#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "client.h"

void handleWrongProgramUsage();

int main(int argc, char const *argv[]) {

  if (argc != 2) handleWrongProgramUsage();

  if (strcmp(argv[1], "server") == 0)
    start_server(create_server());
  else if (strcmp(argv[1], "client") == 0)
    run_client();
  else
    handleWrongProgramUsage();

  return 0;
}

void handleWrongProgramUsage() {
  printf("Program usage: ./executable <server|client>.\n");
  printf("\tIf you want to run the server, use: './chat server'.\n");
  printf("\tIf you want to run the client, use: './chat client'.\n");

  exit(EXIT_FAILURE);
}
