#pragma once
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3000
#define MAX_CONNECTED_CLIENTS 20
#define MAX_CONNECTION_QUEUE 20

// Gets server ip on sockaddr_in format from defined string SERVER_IP (server.h).
struct sockaddr_in get_server_sockaddr();

/* Starts server, sets it to be ready to start new connections (listen) and returns
int: socket file description (socket id). */
int create_server();

void start_server(int server_socket);

void shutdown_server(int server_socket);
