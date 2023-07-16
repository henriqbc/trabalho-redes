#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include "server.h"
#include "message.h"
#include "operation.h"

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

  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
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

  // HACK
  // tava usando isso aqui pra testar só, tava mandando pro client na hora da conexão
  int new_socket;
  int addrlen = sizeof(server_address);
  if ((new_socket = accept(sockfd, (struct sockaddr *)&server_address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  Message *message = create_message("mano", TEXT, "fala ai");
  send_message(new_socket, message);

  while (true)
    ;

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

void kick_user_from_broadcast(char *nickname, Server *server) {
  User *new_connections = malloc(sizeof(User) * (server->connections_qty - 1));
  int copy_index = 0;
  for (int i = 0; i < server->connections_qty; i++) {
    if (strcmp(server->all_connections[i].nickname, nickname) != 0)
      new_connections[copy_index++] = server->all_connections[i];
  }

  free(server->all_connections);
  server->all_connections = new_connections;
}

void kick_user_from_channel(char *nickname, char *channel_name, Server *server) {

  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, nickname) == 0) {
      Channel target_channel = server->channels[i];
      User *updated_members = malloc(sizeof(User) * (target_channel.members_qty - 1));
      int copy_index = 0;

      for (int j = 0; j < target_channel.members_qty; j++)
        if (strcmp(server->all_connections[i].nickname, nickname) != 0)
          updated_members[copy_index++] = target_channel.members[j];

      server->channels[i].members = updated_members;
    }
  }
}

void create_channel(Channel channel, Server *server) {
  server->channels_qty++;
  server->channels = realloc(server->channels, sizeof(Channel) * server->channels_qty);
  server->channels[server->channels_qty - 1] = channel;
}

void kill_channel(char *channel_name, Server *server) {
  Channel *new_channels = malloc(sizeof(Channel) * (server->channels_qty - 1));
  int copy_index = 0;
  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, channel_name) != 0) {
      new_channels[copy_index++] = server->channels[i];
    } else {
      for (int j = 0; j < server->channels[i].members_qty; j++)
        kick_user_from_broadcast(server->channels[i].members[j].nickname, server);
    }
  }

  free(server->channels);
  server->channels = new_channels;
}

bool is_nickname_already_taken(char *nickname, Server *server) {
  for (int i = 0; i < server->connections_qty; i++)
    if (strcmp(server->all_connections[i].nickname, nickname) == 0) return true;
  return false;
}

bool channel_exists(char *channel_name, Server *server) {
  for (int i = 0; i < server->connections_qty; i++)
    if (strcmp(server->channels[i].name, channel_name) == 0) return true;
  return false;
}

void add_user_to_broadcast(User user, Server *server) {
  server->connections_qty++;
  server->all_connections =
      realloc(server->all_connections, sizeof(User) * (server->connections_qty));

  server->all_connections[server->connections_qty - 1] = user;
}

void add_user_to_channel(User user, char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    Channel current_channel = server->channels[i];
    if (strcmp(current_channel.name, channel_name) == 0) {
      current_channel.members_qty++;
      current_channel.members =
          realloc(current_channel.members, sizeof(User) * current_channel.members_qty);
      current_channel.members[current_channel.members_qty - 1] = user;

      return;
    }
  }
}

void move_user_through_channels(User user, char *current_channel_name, char *new_channel_name,
                                Server *server) {
  kick_user_from_channel(user.nickname, current_channel_name, server);
  add_user_to_channel(user, new_channel_name, server);
}

void mute_user(char *nickname, char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    Channel current_channel = server->channels[i];
    if (strcmp(current_channel.name, channel_name) == 0)
      for (int j = 0; j < current_channel.members_qty; j++) {
        if (strcmp(current_channel.members[j].nickname, nickname) == 0)
          current_channel.members[j].muted = true;
      }
  }
}

void unmute_user(char *nickname, char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    Channel current_channel = server->channels[i];
    if (strcmp(current_channel.name, channel_name) == 0)
      for (int j = 0; j < current_channel.members_qty; j++) {
        if (strcmp(current_channel.members[j].nickname, nickname) == 0)
          current_channel.members[j].muted = false;
      }
  }
}

void delete_server_config(Server *server) {
  free(server->all_connections);
  free(server->channels);
}

void shutdown_server(int server_socket) { close(server_socket); }
