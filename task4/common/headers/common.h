#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>

#define CLIENTS_AMOUNT 5
#define BUFFER_SIZE 128
#define SERVER_IP "192.168.0.6"
#define CLIENT_IP "192.168.0.3"
#define SERVER_MAC {0x08, 0x00, 0x27, 0x71, 0xa1, 0x6e}
#define CLIENT_MAC {0x00, 0xd8, 0x61, 0x59, 0xd5, 0x02}
#define MAC_SIZE 6
#define SERVER_PORT 8080
#define CLIENT_PORT 7777
#define PROTO_VERSION 4
#define IFNAME "enp0s3"
#define print_error(msg) do {perror(msg); \
  exit(EXIT_FAILURE);} while(0)

#endif // !COMMON_H
