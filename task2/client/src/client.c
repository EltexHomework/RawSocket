#include "../headers/client.h"
#include <netinet/udp.h>
#include <string.h>

/*
 * create_client - used to create an object of
 * client struct. 
 * @ip - ip address of the server
 * @port - port of the server
 * Return: pointer to an object of client struct
 */
struct client* create_client(const char* ip, const int port) {
  struct client* client = (struct client*) malloc(sizeof(struct client));
  if (!client)
    print_error("malloc");

  /* Initialzie server sockaddr_un struct*/
  client->serv.sin_family = AF_INET;
  client->serv.sin_addr.s_addr = inet_addr(ip);
  client->serv.sin_port = htons(port);

  client->sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (client->sfd == -1)
    print_error("socket");

  return client;
}

/* run_client - used to connect serv address
 * to file descriptor client->sfd
 * @client - pointer to an object of client struct
 */
void run_client(struct client* client) {
  /* socklen_t server_len = sizeof(client->serv);   
   
  if (connect(client->sfd, (struct sockaddr*) &client->serv, server_len) == -1)
    print_error("connect"); */

  /* Process user input */
  process_input(client);
}

/*
 * process_input - used to receive user input
 * from stdin. Terminates received string, calls
 * send_message and waits for server response.
 * @client - pointer to an object of client struct
 */
void process_input(struct client* client) {
  char buffer[BUFFER_SIZE];
  char response[BUFFER_SIZE];

  /* Wait for user input */
  while (1) {
    printf("Enter message: ");
    
    /* Read user input */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
      print_error("fgets");
    buffer[strlen(buffer) - 1] = '\0';

    /* Send user message */
    send_message(client, buffer);
    
    snprintf(response, BUFFER_SIZE, "Server %s", buffer); 

    /* Receive answer */
    while (1) {
      char* message = recv_response(client);
      if (message == NULL) {
        close_connection(client);
        break;
      }
      if (strcmp(response, message) == 0) {
        printf("SERVER: Server %s:%d send response: %s\n", 
               inet_ntoa(client->serv.sin_addr), 
               SERVER_PORT, 
               message);
        break;
      }
      free(message);
    }
  }
}

/*
 * send_message - used to send message to server.  
 * @client - pointer to an object of client struct 
 * @buffer - message
 */
void send_message(struct client* client, char message[BUFFER_SIZE]) {
  ssize_t bytes_send;
  int length = sizeof(struct udphdr) + strlen(message);
  char buffer[length];
  struct udphdr header; 
  
  /* Initialize UDP header */
  header.source = htons(CLIENT_PORT);
  header.dest = htons(SERVER_PORT);
  header.check = 0;
  header.len = htons(length);
  
  /* Copy header to buffer */
  memcpy(buffer, &header, sizeof(struct udphdr));

  /* Copy payload to buffer */
  memcpy(buffer + sizeof(struct udphdr), message, strlen(message));

  /* Send message to server */
  bytes_send = sendto(client->sfd, buffer, length, 0, 
                      (struct sockaddr*) &client->serv, sizeof(client->serv));
   
  if (bytes_send == -1)
    print_error("sendto");

  printf("CLIENT: Send message to %s:%d: %s\n", 
         inet_ntoa(client->serv.sin_addr), 
         ntohs(client->serv.sin_port), 
         message);
}

/*
 * recv_message - used to receive message from server.
 * Allocates memory for buffer which should be freed manually.
 *
 * Return: string (message) if successful, NULL if connection terminated
 */
char* recv_response(struct client* client) {
  ssize_t bytes_read;
  char* buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
  socklen_t serv_len = sizeof(client->serv);
  char* payload;
  
  /* Receive message from server */ 
  bytes_read = recvfrom(client->sfd, buffer, BUFFER_SIZE, 0, 
                        (struct sockaddr*) &client->serv, &serv_len);
  
  if (bytes_read == -1)
    print_error("recvfrom");
  else if (bytes_read == 0)
    return NULL;
  
  /* Truncate buffer */
  buffer[bytes_read] = '\0';

  /* Extract payload */
  payload = extract_payload(buffer);
  
  return payload;
}

/*
 * extract_payload - used to extract payload
 * from UDP packet. Skips IP header and UDP header
 * by calculation their length. Allocates memory
 * for payload (must be free manually).
 * @buffer - pointer to UDP packet
 *
 * Return: pointer to buffer with payload
 */
char* extract_payload(char* buffer) {
  int iphdr_length, size;
  char* ptr;
  char* payload = (char*) malloc(BUFFER_SIZE * sizeof(char));
  struct iphdr* ip;
  
  /* Extract IP header */
  ip = (struct iphdr*) buffer;

  /* Calculate IP header length */
  iphdr_length = ip->ihl * 4;

  /* Get payload */
  ptr = buffer + iphdr_length + sizeof(struct udphdr);
   
  /* Copy payload */
  strncpy(payload, ptr, strlen(ptr));

  return payload;  
}

/*
 * close_connection - used to close connection with close
 * call.
 * @client - pointer to an object of client struct
 */
void close_connection(struct client* client) {
  close(client->sfd);
}

/*
 * free_client - used to free allocated memory for client
 * object.
 * @client - pointer to an object of client struct
 */
void free_client(struct client* client) {
  free(client);
}
