#pragma once
#include <sys/socket.h>

#include "user.h"
#include "channel.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3000
#define MAX_CONNECTED_CLIENTS 20

// Channel-oriented and Broadcasting Server Configuration
typedef struct Server {
  Channel *channels;
  int channels_qty;

  User *all_connections;
  int connections_qty;
} Server;

// Gets server ip on sockaddr_in format from defined string SERVER_IP (server.h).
struct sockaddr_in get_server_sockaddr();

/* Starts server, sets it to be ready to start new connections (listen) and returns
int: socket file description (socket id). */
int create_server();

Server *build_channel_server_config(Channel *channels, int channels_qty);
Server *build_broadcast_server_config(User *all_connections, int connections_qty);

void delete_server_config(Server *server);

void set_server_to_listening_mode(int server_socket);

void shutdown_server(int server_socket);
