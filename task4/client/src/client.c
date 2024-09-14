#include "../headers/client.h"

/*
 * create_client - used to create an object of
 * client struct. 
 * @ip - ip address of the server
 * @port - port of the server
 * Return: pointer to an object of client struct
 */
struct client* create_client(const char* ip, const int port, const char mac[MAC_SIZE], const char* client_ip) {
  int flag = 1;
  struct client* client = (struct client*) malloc(sizeof(struct client));
  if (!client)
    print_error("malloc");

  /* Initialzie server sockaddr_ll struct*/
  client->serv_ll.sll_hatype = 0;
  client->serv_ll.sll_pkttype = 0;
  client->serv_ll.sll_protocol = 0;
  client->serv_ll.sll_family = AF_PACKET;
  client->serv_ll.sll_ifindex = if_nametoindex(IFNAME); 
  client->serv_ll.sll_halen = MAC_SIZE; 
  memcpy(client->serv_ll.sll_addr, mac, MAC_SIZE);
  
  /* Initialize server endpoint */
  client->serv_ip = ip;
  client->serv_port = port;

  /* Initialize client endpoint */
  client->client_ip = client_ip;

  /* Create Raw Socket */
  client->sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (client->sfd == -1)
    print_error("socket");

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
    char* message;
    char* payload;

    printf("Enter message: ");
    
    /* Read user input */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
      print_error("fgets");
    buffer[strlen(buffer) - 1] = '\0';

    /* Send user message */
    send_message(client, buffer);
  
    printf("CLIENT: Send message to %s:%d: %s\n", 
         client->serv_ip, 
         client->serv_port, 
         buffer);
   
    /* Receive answer */
    message = recv_response(client);
    if (message == NULL) {
      close_connection(client);
      break;
    }
    
    /* Extract payload */
    payload = extract_payload(message);

    /* Log response */
    printf("CLIENT: Received response from %s:%d : %s\n",
           client->serv_ip,
           client->serv_port,
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
  uint8_t shost[MAC_SIZE] = CLIENT_MAC;  
  uint8_t dhost[MAC_SIZE] = SERVER_MAC;
  int length = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message);
  char buffer[length];
  struct iphdr ip;
  struct udphdr udp; 
  struct ether_header ether;

  /* Initialize headers */
  init_iphdr(&ip, message, client->client_ip, client->serv_ip);
  init_udphdr(&udp, sizeof(struct udphdr) + strlen(message), client->serv_port); 
  init_etherhdr(&ether, shost, dhost); 

  /* Copy IP header to buffer */
  memcpy(buffer, &ip, sizeof(struct iphdr));

  /* Copy UDP header to buffer */
  memcpy(buffer + sizeof(struct iphdr), 
         &udp, 
         sizeof(struct udphdr));

  /* Copy Ethernet header to buffer */
  memcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr), 
         &ether, 
         sizeof(struct ether_header));
  
  /* Copy payload to message */
  memcpy(buffer + sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct ether_header),
         message,
         strlen(message));

  /* Send message to server */
  bytes_send = sendto(client->sfd, buffer, length, 0, 
                      (struct sockaddr*) &client->serv_ll, sizeof(client->serv_ll));
   
  if (bytes_send == -1)
    print_error("sendto");

}

/*
 * init_iphdr - used to initialize IP header with serv addr.
 * @client - pointer to an object of client struct
 * @ip - pointer to an object of iphdr struct
 */
void init_iphdr(struct iphdr* ip, char message[BUFFER_SIZE], 
                const char* client_ip, const char* server_ip) {
  /* Initialize IP header */
  ip->version = 4;
  ip->ttl = 255;
  ip->id = 2;
  ip->check = 0;
  ip->check = checksum(message);
  ip->saddr = inet_addr(client_ip);
  ip->tos = 0;
  ip->frag_off = 0;
  ip->ihl = 5;
  ip->tot_len =  ip->ihl * 4 + strlen(message);
  ip->daddr = inet_addr(server_ip);
  ip->protocol = IPPROTO_UDP;
}

/*
 * init_udphdr - used to initialize UDP header with
 * source port, dest port, check and length.
 * @client - pointer to an object of client struct
 * @udp - pointer to an object of udphdr struct
 * @length - length of UDP header 
 */
void init_udphdr(struct udphdr* udp, ssize_t length, int server_port) {
  /* Initialize UDP header */
  udp->source = htons(CLIENT_PORT);
  udp->dest = htons(server_port);  
  udp->check = 0;
  udp->len = htons(length);
}

void init_etherhdr(struct ether_header* ether, uint8_t shost[MAC_SIZE], 
                   uint8_t dhost[MAC_SIZE]) {
  ether->ether_type = 4;
  memcpy(ether->ether_shost, shost, MAC_SIZE);
  memcpy(ether->ether_dhost, dhost, MAC_SIZE);
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
    if (ip->saddr == inet_addr(client->serv_ip) &&
    udp->source == htons(client->serv_port)) {
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
 * checksum - used to calculate checksum of buffer.
 * @buffer - buffer with max size of BUFFER_SIZE
 *
 * Return: int value representing checksum  
 */
int checksum(char buffer[BUFFER_SIZE]) {
  int csum = 0;
  int remainder = 0;
  short* ptr;
  
  /* Take first 2 bytes from buffer */
  ptr = (short *) buffer;
  
  /* Iterate throught buffer by 2 bytes */
  for (int i = 0; i < 10; i++) {
    csum = csum + *ptr;
    ptr++;
  }
  
  /* Get remainder */
  remainder = csum >> 16;

  /* Add remainder */
  csum = (csum & 0xFFFF) + remainder;

  /* Inverse checksum*/
  csum = ~csum;

  return csum;
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
