#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include "server.h"

struct sockaddr_in get_server_sockaddr() {
  struct sockaddr_in server_address;

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(SERVER_PORT);

  inet_pton(AF_INET, SERVER_IP, &(server_address.sin_addr));

  return server_address;
}

int create_server() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("Error starting server socket. Shutting down.\n");
    return -1;
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, NULL, 0) == -1) {
    printf("Error setting server socket option. Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  struct sockaddr_in server_address = get_server_sockaddr();

  if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
    printf("Error while binding server socket. Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  if (listen(sockfd, MAX_CONNECTION_QUEUE) == -1) {
    printf("Error while waiting for connections (listening). Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  printf("Server running.\n");

  return sockfd;
}

void signal_callback_handler(int server_socket) {
  printf("Shutting down server.");
  shutdown_server(server_socket);

  exit(0);
}

void start_server(int server_socket) {
  signal(server_socket, signal_callback_handler);

  int client_sockets[MAX_CONNECTED_CLIENTS];
  for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++) client_sockets[i] = 0;

  while (1) {
    // algo aqui
  }

  return;
}

void shutdown_server(int server_socket) { close(server_socket); }