#ifndef CLIENT_H
#define CLIENT_H

#include "../../common/headers/common.h"
#include <netinet/udp.h>
#include <netinet/ip.h>

/*
 * Used as client for connection to inet address
 * family (AF_INET) server via UDP protocol. 
 */
struct client {
  /* Address of the server */
  struct sockaddr_in serv;
  
  /* Server file descriptor*/
  int sfd;
};

struct client* create_client(const char* ip, const int port);

void run_client(struct client* client);

void process_input(struct client* client);

void send_message(struct client* client, char message[BUFFER_SIZE]);

char* recv_response(struct client* client);

void init_iphdr(struct client* client, struct iphdr* ip);

void init_udphdr(struct client* client, struct udphdr* udp, ssize_t length);

char* extract_payload(char* buffer);

void close_connection(struct client* client);

void free_client(struct client* client);

#endif // !CLIENT_H
