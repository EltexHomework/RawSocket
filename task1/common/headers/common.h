#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>

#define CLIENTS_AMOUNT 5
#define BUFFER_SIZE 65536
#define SERVER_IP "127.0.0.1" 
#define SERVER_PORT 8080
#define print_error(msg) do {perror(msg); \
  exit(EXIT_FAILURE);} while(0)

#endif // !COMMON_H
