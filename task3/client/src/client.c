#include "../headers/client.h"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <string.h>
#include <sys/socket.h>

/*
 * create_client - used to create an object of
 * client struct. 
 * @ip - ip address of the server
 * @port - port of the server
 * Return: pointer to an object of client struct
 */
struct client* create_client(const char* ip, const int port) {
  int flag = 1;
  struct client* client = (struct client*) malloc(sizeof(struct client));
  if (!client)
    print_error("malloc");

  /* Initialzie server sockaddr_un struct*/
  client->serv.sin_family = AF_INET;
  client->serv.sin_addr.s_addr = inet_addr(ip);
  client->serv.sin_port = htons(port);
  
  /* Create Raw Socket */
  client->sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (client->sfd == -1)
    print_error("socket");
  
  /* Turn on IP header init by hand */
  setsockopt(client->sfd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag));

  return client;
}

/* run_client - used to connect serv address
 * to file descriptor client->sfd
 * @client - pointer to an object of client struct
 */
void run_client(struct client* client) {
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

  /* Wait for user input */
  while (1) {
    printf("Enter message: ");
    
    /* Read user input */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
      print_error("fgets");
    buffer[strlen(buffer) - 1] = '\0';

    /* Send user message */
    send_message(client, buffer);
    
    /* Receive answer */
    char* message = recv_response(client);
      
    if (message == NULL) {
      close_connection(client);
      break;
    }
    
    /* Extract payload */
    char* payload = extract_payload(message);

    /* Log response */
    printf("CLIENT: Received response from %s:%d : %s\n",
           inet_ntoa(client->serv.sin_addr),
           ntohs(client->serv.sin_port),
           payload);
    
    /* Free allocated memory */
    free(message);
    free(payload);
  }
}

/*
 * send_message - used to send message to server.  
 * @client - pointer to an object of client struct 
 * @buffer - message
 */
void send_message(struct client* client, char message[BUFFER_SIZE]) {
  ssize_t bytes_send;
  int length = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message);
  char buffer[length];
  struct iphdr ip;
  struct udphdr udp; 
  
  /* Initialize headers */
  init_iphdr(client, &ip);
  init_udphdr(client, &udp, sizeof(struct udphdr) + strlen(message)); 
    
  /* Copy IP header to buffer */
  memcpy(buffer, &ip, sizeof(struct iphdr));

  /* Copy UDP header to buffer */
  memcpy(buffer + sizeof(struct iphdr), 
         &udp, 
         sizeof(struct udphdr));
  
  /* Copy payload to buffer */
  memcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr), 
         message, 
         strlen(message));

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
 * init_iphdr - used to initialize IP header with serv addr.
 * @client - pointer to an object of client struct
 * @ip - pointer to an object of iphdr struct
 */
void init_iphdr(struct client* client, struct iphdr* ip) {
  /* Initialize IP header */
  ip->version = 4;
  ip->ttl = 255;
  ip->id = 0;
  ip->check = 0;
  ip->saddr = 0;
  ip->tos = 0;
  ip->frag_off = 0;
  ip->tot_len = 0;
  ip->daddr = client->serv.sin_addr.s_addr;
  ip->protocol = IPPROTO_UDP;
  ip->ihl = 5;
}

/*
 * init_udphdr - used to initialize UDP header with
 * source port, dest port, check and length.
 * @client - pointer to an object of client struct
 * @udp - pointer to an object of udphdr struct
 * @length - length of UDP header 
 */
void init_udphdr(struct client* client, struct udphdr* udp, ssize_t length) {
  /* Initialize UDP header */
  udp->source = htons(CLIENT_PORT);
  udp->dest = client->serv.sin_port;  
  udp->check = 0;
  udp->len = htons(length);
}

/*
 * recv_message - used to receive message from server.
 * Allocates memory for buffer which should be freed manually.
 *
 * Return: string (message) if successful, NULL if connection terminated
 */
char* recv_response(struct client* client) {
     
  ssize_t bytes_read;
  socklen_t serv_len;
  struct sockaddr_in addr; 
  char* buffer = (char*) malloc(BUFFER_SIZE * sizeof(char));
   
  serv_len = sizeof(addr);
  
  while (1) {
    /* Receive message from server */ 
    bytes_read = recvfrom(client->sfd, buffer, BUFFER_SIZE, 0, 
                          (struct sockaddr*) &addr, &serv_len);

    if (bytes_read == -1)
      print_error("recvfrom");
    else if (bytes_read == 0)
      return NULL;
    
    /* Extract headers */
    struct iphdr* ip = (struct iphdr*) buffer;
    struct udphdr* udp = (struct udphdr*) (buffer + ip->ihl * 4);

    /* Message from server */
    if (ip->saddr == client->serv.sin_addr.s_addr &&
    udp->source == client->serv.sin_port) {
      break;
    }
  }
  
  /* Truncate buffer */
  buffer[bytes_read] = '\0';

  return buffer;
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
