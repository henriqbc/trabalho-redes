#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "message.h"
#include "operation.h"
#include "client.h"

typedef struct ClientInfo {
  int sockfd;
  struct sockaddr_in ip_addr;
  bool receives_broadcast;
} ClientInfo;

ClientInfo clients[MAX_CONNECTED_CLIENTS];
int connected_clients = 0;

Server *broadcast_server;
Server *channel_server;

int server_socket_id;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_in get_server_sockaddr() {
  struct sockaddr_in server_address;

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(SERVER_PORT);

  inet_pton(AF_INET, SERVER_IP, &(server_address.sin_addr));

  return server_address;
}

struct sockaddr_in get_client_addr(int client_socket) {
  struct sockaddr_in client_address;

  getsockname(client_socket, (struct sockaddr *)&client_address, sizeof(client_address));

  return client_address;
}

char *get_ip_str_from_sockaddr(struct sockaddr_in sockaddr) {
  int IP_address_size = 16 * sizeof(char);
  char *IP_address = malloc(IP_address_size);

  inet_ntop(AF_INET, &sockaddr, IP_address, IP_address_size);

  return IP_address;
}

int create_server() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    printf("Error starting server socket. Shutting down.\n");
    return -1;
  }

  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    printf("Error setting server socket option. Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  struct sockaddr_in server_address = get_server_sockaddr();

  if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
    printf("Error while binding server socket. Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  if (listen(sockfd, MAX_CONNECTION_QUEUE) < 0) {
    printf("Error while waiting for connections (listening). Shutting down.\n");
    shutdown_server(sockfd);
    return -1;
  }

  printf("Server running.\n");

  return sockfd;
}

void signal_callback_handler() {
  printf("\nShutting down server.");
  shutdown_server(server_socket_id);
  delete_server_config(broadcast_server);
  delete_server_config(channel_server);

  exit(0);
}

void define_server_sigint_handler() {
  struct sigaction act;
  act.sa_handler = signal_callback_handler;
  act.sa_mask = (__sigset_t){.__val = 0};
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);
}

void delete_server_config(Server *server) {
  free(server->all_connections);
  free(server->channels);
  free(server);
}

Channel find_user_channel(char *nickname, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    for (int j = 0; j < server->channels[i].members_qty; j++) {
      if (strcmp(server->channels[i].members[j].nickname, nickname) == 0) {
        return server->channels[i];
      }
    }
  }

  return (Channel){.members = NULL, .members_qty = -1, .name = NULL};
}

bool is_user_an_admin(char *nickname, char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, channel_name) == 0)
      return strcmp(server->channels[i].members[0].nickname, nickname) == 0;
  }

  return false;
}

bool user_already_connected(char *nickname, Server *server) {
  for (int i = 0; i < server->connections_qty; i++)
    if (strcmp(server->all_connections[i].nickname, nickname) == 0) return true;
  return false;
}

void update_channel_nickname(char *old_nickname, char *new_nickname, char *channel_name,
                             Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, channel_name) == 0) {
      for (int j = 0; j < server->connections_qty; j++) {
        if (strcmp(server->channels[i].members[j].nickname, old_nickname) == 0) {
          server->channels[i].members[j].nickname = new_nickname;
          break;
        }
      }
      break;
    }
  }

  for (int k = 0; k < server->connections_qty; k++) {
    if (strcmp(server->all_connections[k].nickname, old_nickname) == 0) {
      server->all_connections[k].nickname = new_nickname;
      break;
    }
  }
}

void update_broadcast_nickname(char *old_nickname, char *new_nickname, Server *server) {
  for (int i = 0; i < server->connections_qty; i++) {
    if (strcmp(server->all_connections[i].nickname, old_nickname) == 0) {
      server->all_connections[i].nickname = new_nickname;
      break;
    }
  }
}

char *whois_nickname(char *nickname, char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, channel_name) == 0) {
      for (int j = 0; j < server->connections_qty; j++) {
        if (strcmp(server->channels[i].members[j].nickname, nickname) == 0) {
          return server->channels[i].members[j].ip;
        }
      }
    }
  }

  return NULL;
}

bool is_receiving_broadcast(int client_socket) {
  for (int i = 0; i < connected_clients; i++) {
    if (clients[i].sockfd == client_socket) {
      return clients[i].receives_broadcast;
    }
  }

  return false;
}

bool is_muted(int client_socket, Channel *channel) {
  for (int i = 0; i < channel->members_qty; i++) {
    if (channel->members[i].socket_fd == client_socket) {
      return channel->members[i].muted;
    }
  }

  return false;
}

void set_receiving_broadcast(int client_socket, bool state) {
  for (int i = 0; i < connected_clients; i++) {
    if (clients[i].sockfd == client_socket) {
      clients[i].receives_broadcast = state;
      break;
    }
  }
}

void disconnect_user_from_broadcast_server(char *nickname, Server *server) {
  for (int i = 0; i < server->connections_qty; i++) {
    if (strcmp(server->all_connections[i].nickname, nickname) == 0) {
      for (int j = i; j < server->connections_qty - 1; j++) {
        server->all_connections[j] = server->all_connections[j + 1];
      }

      server->connections_qty -= 1;
      break;
    }
  }
}

void disconnect_user_from_channel_server(char *nickname, Channel channel, Server *server) {
  kick_user_from_channel(nickname, channel.name, server);

  // Remove from all_connections.
  for (int i = 0; i < server->connections_qty; i++) {
    if (strcmp(server->all_connections[i].nickname, nickname) == 0) {
      for (int j = i; j < server->connections_qty - 1; j++) {
        server->all_connections[j] = server->all_connections[j + 1];
      }

      server->connections_qty -= 1;
      break;
    }
  }
}

void handle_user_connect(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  if (user_already_connected(message->sender_nickname, channel_server)) {
    Channel user_channel = find_user_channel(message->sender_nickname, channel_server);
    disconnect_user_from_channel_server(message->sender_nickname, user_channel,
                                        channel_server);
  }

  if (is_nickname_already_taken(message->sender_nickname, broadcast_server)) {
    printf("Nickname is already taken.\n");
    send_response(NULL, NICKNAME_ALREADY_TAKEN_CONNECT, message->sender_nickname,
                  client_socket);
  } else {
    struct sockaddr_in client_addr = get_client_addr(client_socket);
    User new_user = (User){.nickname = message->sender_nickname,
                           .socket_fd = client_socket,
                           .ip = get_ip_str_from_sockaddr(client_addr),
                           .muted = false};

    add_user_to_all_connections(new_user, broadcast_server);
    printf("User %s successfully connected to the broadcast server!\n",
           message->sender_nickname);

    send_response(NULL, CONNECT, NULL, client_socket);

    set_receiving_broadcast(client_socket, true);
  }

  pthread_mutex_unlock(&mutex);
}

void handle_user_text(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  if (is_receiving_broadcast(client_socket)) {  // Broadcast server transmission.
    for (int i = 0; i < broadcast_server->connections_qty; i++) {
      send_message(broadcast_server->all_connections[i].socket_fd, message);
    }

    pthread_mutex_unlock(&mutex);
    return;
  }

  // Channel server transmission.
  Channel user_channel = find_user_channel(message->sender_nickname, channel_server);
  if (!is_muted(client_socket, &user_channel)) {
    for (int i = 0; i < user_channel.members_qty; i++) {
      send_message(user_channel.members[i].socket_fd, message);
    }
  }

  pthread_mutex_unlock(&mutex);
}

void handle_user_join(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  if (user_already_connected(message->sender_nickname, broadcast_server)) {
    disconnect_user_from_broadcast_server(message->sender_nickname, broadcast_server);
  }

  if (user_already_connected(message->sender_nickname, channel_server)) {
    if (!channel_exists(message->content, channel_server)) {
      char *new_channel_name = malloc(MAX_PACKET_SIZE * sizeof(char));
      strcpy(new_channel_name, message->content);
      Channel new_channel =
          (Channel){.name = new_channel_name, .members = NULL, .members_qty = 0};

      create_channel(new_channel, channel_server);
    }

    Channel user_current_channel = find_user_channel(message->sender_nickname, channel_server);

    for (int i = 0; i < user_current_channel.members_qty; i++) {
      if (strcmp(user_current_channel.members[i].nickname, message->sender_nickname) == 0) {
        User user = user_current_channel.members[i];
        move_user_through_channels(user, user_current_channel.name, message->content,
                                   channel_server);
      }
    }

    send_response(NULL, JOIN, NULL, client_socket);
    pthread_mutex_unlock(&mutex);
    return;
  }

  if (is_nickname_already_taken(message->sender_nickname, channel_server)) {
    send_response(NULL, NICKNAME_ALREADY_TAKEN, NULL, client_socket);
    printf("Nickname is already taken.\n");
  } else {
    if (!channel_exists(message->content, channel_server)) {
      char *new_channel_name = malloc(MAX_PACKET_SIZE * sizeof(char));
      strcpy(new_channel_name, message->content);
      Channel new_channel =
          (Channel){.name = new_channel_name, .members = NULL, .members_qty = 0};

      create_channel(new_channel, channel_server);
    }

    struct sockaddr_in client_addr = get_client_addr(client_socket);
    User new_user = (User){.nickname = message->sender_nickname,
                           .socket_fd = client_socket,
                           .ip = get_ip_str_from_sockaddr(client_addr),
                           .muted = false};

    add_user_to_channel(new_user, message->content, channel_server);
    add_user_to_all_connections(new_user, channel_server);
    printf("User %s successfully connected to %s channel in the channel-oriented server!\n",
           message->sender_nickname, message->content);

    send_response(NULL, JOIN, NULL, client_socket);

    set_receiving_broadcast(client_socket, false);
  }

  pthread_mutex_unlock(&mutex);
}

void handle_user_nickname_update(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  Server *user_server =
      is_receiving_broadcast(client_socket) ? broadcast_server : channel_server;
  if (is_nickname_already_taken(message->content, user_server)) {
    send_response(NULL, NICKNAME_ALREADY_TAKEN, message->sender_nickname, client_socket);
    pthread_mutex_unlock(&mutex);
    return;
  }

  if (is_receiving_broadcast(client_socket)) {
    update_broadcast_nickname(message->sender_nickname, message->content, broadcast_server);
  } else {
    Channel target_channel = find_user_channel(message->sender_nickname, channel_server);
    update_channel_nickname(message->sender_nickname, message->content, target_channel.name,
                            channel_server);
  }

  char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
  sprintf(error_message, "Successfully renamed user %s to %s!", message->sender_nickname,
          message->content);
  send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

  printf("Successfully renamed user %s to %s!\n", message->sender_nickname, message->content);
  pthread_mutex_unlock(&mutex);
}

void handle_admin_kick(Message *message, int client_socket) {

  if (strcmp(message->sender_nickname, message->content) == 0) {
    printf("Users can't kick themselves.", message->content);
    send_response(NULL, KICK_FAILED, NULL, client_socket);
    return;
  }

  pthread_mutex_lock(&mutex);

  Channel target_channel = find_user_channel(message->content, channel_server);
  User kicked_user;
  for (int i = 0; i < target_channel.members_qty; i++)
    if (strcmp(message->content, target_channel.members[i].nickname) == 0)
      kicked_user = target_channel.members[i];

  if (target_channel.name == NULL) {
    printf("User %s doesn't exist.\n", message->content);
    send_response(NULL, USER_NOT_FOUND, NULL, client_socket);
    pthread_mutex_unlock(&mutex);
    return;
  }

  if (!is_user_an_admin(message->sender_nickname, target_channel.name, channel_server)) {
    printf("User unauthorized to perform this command.\n");
    send_response(NULL, UNAUTHORIZED, NULL, client_socket);
    pthread_mutex_unlock(&mutex);
    return;
  }

  kick_user_from_channel(message->content, target_channel.name, channel_server);
  printf("Successfully kicked user %s from channel %s!\n", message->content,
         target_channel.name);

  add_user_to_all_connections(kicked_user, broadcast_server);
  set_receiving_broadcast(kicked_user.socket_fd, true);
  send_response(NULL, KICK, NULL, kicked_user.socket_fd);

  send_response(NULL, KICK_SUCCEEDED, NULL, client_socket);

  pthread_mutex_unlock(&mutex);
}

void handle_admin_mute(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  Channel target_channel = find_user_channel(message->content, channel_server);
  if (target_channel.name == NULL) {
    char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
    sprintf(error_message, "Provided nickname \"%s\" is not currently connected!",
            message->content);
    send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

    pthread_mutex_unlock(&mutex);
    return;
  }

  if (!is_user_an_admin(message->sender_nickname, target_channel.name, channel_server)) {
    char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
    sprintf(error_message, "User unauthorized to perform this command.");
    send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

    pthread_mutex_unlock(&mutex);
    return;
  }

  mute_user(message->content, target_channel.name, channel_server);

  char *response = malloc(MAX_PACKET_SIZE * sizeof(char));
  sprintf(response, "User %s has been muted.", message->content);
  send_response(NULL, SERVER_RESPONSE, response, client_socket);

  pthread_mutex_unlock(&mutex);
}

void handle_admin_unmute(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  Channel target_channel = find_user_channel(message->content, channel_server);
  if (target_channel.name == NULL) {
    char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
    sprintf(error_message, "Provided nickname \"%s\" is not currently connected!",
            message->content);
    send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

    pthread_mutex_unlock(&mutex);
    return;
  }

  if (!is_user_an_admin(message->sender_nickname, target_channel.name, channel_server)) {
    char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
    sprintf(error_message, "User unauthorized to perform this command.");
    send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

    pthread_mutex_unlock(&mutex);
    return;
  }

  unmute_user(message->content, target_channel.name, channel_server);

  char *response = malloc(MAX_PACKET_SIZE * sizeof(char));
  sprintf(response, "User %s has been unmuted.", message->content);
  send_response(NULL, SERVER_RESPONSE, response, client_socket);

  pthread_mutex_unlock(&mutex);
}

void handle_admin_whois(Message *message, int client_socket) {
  pthread_mutex_lock(&mutex);

  Channel target_channel = find_user_channel(message->content, channel_server);
  if (target_channel.name == NULL) {
    char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
    sprintf(error_message, "Provided nickname \"%s\" is not currently connected!",
            message->content);
    send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

    pthread_mutex_unlock(&mutex);
    return;
  }

  if (!is_user_an_admin(message->sender_nickname, target_channel.name, channel_server)) {
    char *error_message = malloc(MAX_PACKET_SIZE * sizeof(char));
    sprintf(error_message, "User unauthorized to perform this command.\n");
    send_response(NULL, SERVER_RESPONSE, error_message, client_socket);

    pthread_mutex_unlock(&mutex);
    return;
  }

  char *target_ip = whois_nickname(message->content, target_channel.name, channel_server);
  char *response = malloc(MAX_PACKET_SIZE * sizeof(char));
  if (target_ip != NULL) {
    sprintf(response, "%s's IP is %s!", message->content, target_ip);
  } else {
    sprintf(response, "Provided nickname \"%s\" is not currently connected to this channel!",
            message->content);
  }

  send_response(NULL, SERVER_RESPONSE, response, client_socket);

  pthread_mutex_unlock(&mutex);
}

void handle_user_quit(Message *message) {
  if (user_already_connected(message->sender_nickname, broadcast_server))
    disconnect_user_from_broadcast_server(message->sender_nickname, broadcast_server);
  else if (user_already_connected(message->sender_nickname, channel_server))
    disconnect_user_from_broadcast_server(message->sender_nickname, channel_server);

  printf("User %s just quit the server.\n", message->sender_nickname);
}

void handle_client_communication(int client_socket) {
  Message *message = receive_message(client_socket);

  while (message) {
    switch (message->operation) {
      case CONNECT:
        handle_user_connect(message, client_socket);
        break;
      case TEXT:
        handle_user_text(message, client_socket);
        break;
      case PING:
        send_response(NULL, PING, NULL, client_socket);
        break;
      case JOIN:
        handle_user_join(message, client_socket);
        break;
      case NICKNAME:
        handle_user_nickname_update(message, client_socket);
        break;
      case KICK:
        handle_admin_kick(message, client_socket);
        break;
      case MUTE:
        handle_admin_mute(message, client_socket);
        break;
      case UNMUTE:
        handle_admin_unmute(message, client_socket);
        break;
      case WHOIS:
        handle_admin_whois(message, client_socket);
        break;
      case QUIT:
        handle_user_quit(message);
        break;
      default:
        printf("Unable to recognized given command.\n");
    }
    message = receive_message(client_socket);
  }

  // Client disconnected.
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < connected_clients; i++) {
    if (clients[i].sockfd == client_socket) {
      // No need to shutdown the client socket, as the client does it itself.

      // Shift the array to remove the disconnected client
      for (int j = i; j < connected_clients - 1; j++) {
        clients[j] = clients[j + 1];
      }

      connected_clients -= 1;
      break;
    }
  }
  pthread_mutex_unlock(&mutex);

  pthread_detach(pthread_self());
}

void start_server(int server_socket) {
  define_server_sigint_handler();

  broadcast_server = build_broadcast_server_config(NULL, 0);
  channel_server = build_channel_server_config(NULL, 0);

  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

    if (client_socket < 0) {
      printf("Error accepting client connection.\n");
      continue;
    }

    if (connected_clients == MAX_CONNECTED_CLIENTS) {
      printf("Maximum number of connections reached. Rejecting new connection.\n");
      shutdown_client(client_socket);
      continue;
    }

    // Add the client to the clients array
    pthread_mutex_lock(&mutex);
    clients[connected_clients].sockfd = client_socket;
    clients[connected_clients].ip_addr = get_server_sockaddr(client_socket);
    connected_clients += 1;
    pthread_mutex_unlock(&mutex);

    // Create a new thread to handle client communication
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, handle_client_communication, client_socket) < 0) {
      printf("Error creating thread to handle new client.\n");
      shutdown_client(client_socket);
      continue;
    }

    pthread_detach(client_thread);

    continue;
  }

  // Server is shutdown on signal_callback_handler.
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
  server->connections_qty--;
}

void kick_user_from_channel(char *nickname, char *channel_name, Server *server) {

  // remove user from all connections as well
  User *updated_all_connections = malloc(sizeof(User) * (server->connections_qty - 1));
  int connections_copy_index = 0;

  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->all_connections[i].nickname, nickname) != 0)
      updated_all_connections[connections_copy_index++] = server->all_connections[i];

    if (strcmp(server->channels[i].name, channel_name) == 0) {

      Channel target_channel = server->channels[i];
      User *updated_members = malloc(sizeof(User) * (target_channel.members_qty - 1));
      int copy_index = 0;

      for (int j = 0; j < target_channel.members_qty; j++) {
        if (strcmp(target_channel.members[j].nickname, nickname) != 0)
          updated_members[copy_index++] = target_channel.members[j];
      }

      free(server->channels[i].members);
      server->channels[i].members = updated_members;
      server->channels[i].members_qty--;
    }
  }

  free(server->all_connections);
  server->all_connections = updated_all_connections;
  server->connections_qty--;
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
  for (int i = 0; i < server->connections_qty; i++) {
    printf("%s %s %d\n", server->all_connections[i].nickname, nickname,
           strcmp(server->all_connections[i].nickname, nickname) == 0);
    if (strcmp(server->all_connections[i].nickname, nickname) == 0) return true;
  }
  return false;
}

bool channel_exists(char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++)
    if (strcmp(server->channels[i].name, channel_name) == 0) return true;

  return false;
}

void add_user_to_all_connections(User user, Server *server) {
  server->connections_qty++;
  if (server->all_connections == NULL) server->all_connections = malloc(0);
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
      server->channels[i] = current_channel;

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
  char *response = malloc(MAX_PACKET_SIZE * sizeof(char));

  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, channel_name) == 0)
      for (int j = 0; j < server->channels[i].members_qty; j++) {
        if (strcmp(server->channels[i].members[j].nickname, nickname) == 0) {
          server->channels[i].members[j].muted = true;
          break;
        }
      }
  }
}

void unmute_user(char *nickname, char *channel_name, Server *server) {
  for (int i = 0; i < server->channels_qty; i++) {
    if (strcmp(server->channels[i].name, channel_name) == 0)
      for (int j = 0; j < server->channels[i].members_qty; j++) {
        if (strcmp(server->channels[i].members[j].nickname, nickname) == 0) {
          server->channels[i].members[j].muted = false;
          return;
        }
      }
  }

  printf("Unable to unmute user %s. Error: not found.\n", nickname);
}

void send_response(char *server_nickname, Operation operation, char *content,
                   int client_socket) {
  Message *response = create_message(server_nickname, operation, content);
  send_message(client_socket, response);
  delete_message(response);
}

void shutdown_server(int server_socket) {
  shutdown(server_socket, SHUT_RDWR);
  close(server_socket);
}
