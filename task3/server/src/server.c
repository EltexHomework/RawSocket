#include "../headers/server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*
 * create_server - used to create an object of server
 * struct, initializes its fields.
 * @ip - ip address of the server
 * @port - port of the server
 *
 * Return: pointer to an object of server struct 
 */
struct server* create_server(const char* ip, const int port) {
  struct server* server = (struct server*) malloc(sizeof(struct server));
  if (!server)
    print_error("malloc");

  /* Initialzie sockaddr_un struct */
  server->serv.sin_family = AF_INET;
  server->serv.sin_addr.s_addr = inet_addr(ip);
  server->serv.sin_port = htons(port);

  /* Create a socket */
  server->sfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server->sfd == -1)
    print_error("socket");

  return server;
}

/*
 * run_server - used to bind server and
 * wait for data in socket.
 * @server - pointer to an object of server struct
 */
void run_server(struct server* server) {
  struct sockaddr_in client;
  socklen_t client_size = sizeof(client);

  /* Bind Endpoint to socket */
  if (bind(server->sfd, (struct sockaddr*) &server->serv, sizeof(server->serv)) == -1)
    print_error("bind");
    
  printf("SERVER: Server %s:%d started\n", inet_ntoa(server->serv.sin_addr), ntohs(server->serv.sin_port));

  /* Wait for data */
  while (1) {
    char* buffer = recv_message(server, &client);
    char* reply = edit_message(buffer);
    
    printf("SERVER: Received message from %s:%d: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buffer);
    send_message(server, &client, reply);

    free(buffer);
    free(reply);
  }
}

/*
 * send_message - used to send message to client.
 * @server - pointer to an object of server struct
 * @client - pointer to address of the client (sockaddr_in)
 * @buffer - message
 */
void send_message(struct server* server, struct sockaddr_in* client, char buffer[BUFFER_SIZE]) {
  ssize_t bytes_send;
  socklen_t client_len = sizeof(*client);

  bytes_send = sendto(server->sfd, buffer, strlen(buffer), 0, (struct sockaddr*) client, client_len);

  if (bytes_send == -1)
    print_error("sendto");
  
  printf("SERVER: Send message to %s:%d: %s\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port), buffer);
}

/*
 * recv_message - used to receive message from server.
 * allocates memory for message, then receives it all. Allocated
 * buffer must be freed manually.
 * @server - pointer to an object of server struct 
 * @client - address of the client (sockaddr_in)
 *
 * Return: string (message) if successful, NULL if connection terminated
 */
char* recv_message(struct server* server, struct sockaddr_in* client) {
  ssize_t bytes_read;
  socklen_t client_len;
  char* buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
  
  /* Get length of clients address */
  client_len = sizeof(*client);
  
  /* Receive message */
  bytes_read = recvfrom(server->sfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) client, &client_len);  
  
  /* Truncate message*/
  buffer[bytes_read] = '\0';

  if (bytes_read == -1)
    print_error("recvfrom");
  else if (bytes_read == 0)
    return NULL;

  return buffer;
}

/*
 * edit_message - used to add prefix "Server" to message.
 * Allocates new buffer. Return buffer should be free manually.
 * @message - message from client that needs to be changed
 * 
 * Return: string with prefix
 */
char* edit_message(char* message) {
  char* prefix = "Server";
  char* new_message = (char*) malloc(strlen(message) + strlen(prefix) + 2);
  if (!new_message)
    print_error("malloc");

  /* Add prefix to message */
  snprintf(new_message, strlen(message) + strlen(prefix) + 2, "%s %s", "Server", message);

  return new_message;
}

/*
 * close_connection - used to close connection.
 * @server - pointer to an object of server struct
 */
void close_connection(struct server* server) {
  close(server->sfd);
}

/*
 * free_server - free allocated memory for server 
 * @server - pointer to an object of server struct
 */
void free_server(struct server* server) {
  free(server);
}
