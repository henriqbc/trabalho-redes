#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char const *argv[]) {

  if (argc != 2) handleWrongProgramUsage();

  enum ProgramMode mode;
  if (strcmp(argv[1], "server"))
    runServer();
  else if (strcmp(argv[1], "client"))
    runClient();
  else
    handleWrongProgramUsage();

  return 0;
}

void handleWrongProgramUsage() {
  printf("Program usage: ./executable <server|client>.\n");
  printf("\tIf you want to run the server, use: './executable server'.\n");
  printf("\tIf you want to run the client, use: './executable client'.\n");

  exit(EXIT_FAILURE);
}
