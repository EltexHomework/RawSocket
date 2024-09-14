#ifndef CLIENT_H
#define CLIENT_H

#include "../../common/headers/common.h"
#include <net/ethernet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

/*
 * Used as client for connection to inet address
 * family (AF_INET) server via UDP protocol. 
 */
struct client {
  /* Address of the server on Link Layer */
  struct sockaddr_ll serv_ll;
  
  const char* serv_ip;
  const char* client_ip;
  int serv_port;
  

  /* Server file descriptor*/
  int sfd;
};

struct client* create_client(const char* ip, const int port, 
    const char mac[MAC_SIZE], const char* client_ip);

void run_client(struct client* client);

void process_input(struct client* client);

void send_message(struct client* client, char message[BUFFER_SIZE]);

char* recv_response(struct client* client);

void init_iphdr(struct iphdr* ip, char message[BUFFER_SIZE], 
                const char* client_ip, const char* server_ip);

void init_udphdr(struct udphdr* udp, ssize_t length, int server_port);

void init_etherhdr(struct ether_header* ether, uint8_t shost[MAC_SIZE], 
                   uint8_t dhost[MAC_SIZE]);

int checksum(uint16_t* buffer, int len);

void close_connection(struct client* client);

void free_client(struct client* client);

#endif // !CLIENT_H
