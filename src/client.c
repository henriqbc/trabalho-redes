#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"
#include "client.h"

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