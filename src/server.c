#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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

  if (listen(sockfd, MAX_CONNECTED_CLIENTS) == -1) {
    printf("Error while waiting for connections (listening). Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  printf("Server running.\n");

  return sockfd;
}

Server *build_channel_server_config(Channel *channels, int channels_qty) {
  Server *channel_server = malloc(sizeof(Server));
  if (channels_qty <= 0) {
    channel_server->channels = NULL;
    channel_server->channels_qty = 0;
    channel_server->all_connections = NULL;
    channel_server->connections_qty = 0;

    return channel_server;
  }

  if (channels_qty == 1) {
    channel_server->channels = channels;
    channel_server->channels_qty = channels_qty;
    channel_server->all_connections = channels[0].members;
    channel_server->connections_qty = channels[0].members_qty;

    return channel_server;
  }

  int computed_connections = 0;
  int all_connections_size = 0;
  User *all_connections = malloc(sizeof(User) * all_connections_size);
  for (int i = 0; i < channels_qty; i++) {
    int current_ch_members_qty = channels[i].members_qty;
    all_connections_size += current_ch_members_qty;
    all_connections = realloc(all_connections, sizeof(User) * all_connections_size);
    for (int j = 0; j < current_ch_members_qty; j++)
      all_connections[computed_connections + j] = channels[i].members[j];
    computed_connections += current_ch_members_qty;
  }

  channel_server->channels = channels;
  channel_server->channels_qty = channels_qty;
  channel_server->all_connections = all_connections;
  channel_server->connections_qty = computed_connections;

  return channel_server;
}

Server *build_broadcast_server_config(User *all_connections, int connections_qty) {
  Server *broadcast_server = malloc(sizeof(Server));
  broadcast_server->channels = NULL;
  broadcast_server->channels_qty = 0;

  if (connections_qty <= 0) {
    broadcast_server->connections_qty = 0;
    broadcast_server->all_connections = NULL;
  } else {
    broadcast_server->connections_qty = connections_qty;
    broadcast_server->all_connections = all_connections;
  }

  return broadcast_server;
}

void delete_server_config(Server *server) {
  free(server->all_connections);
  free(server->channels);
}

void set_server_to_listening_mode(int server_socket) { return; }

void shutdown_server(int server_socket) { close(server_socket); }
