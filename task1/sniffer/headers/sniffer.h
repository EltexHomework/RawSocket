#ifndef SNIFFER_H
#define SNIFFER_H

#include "../../common/headers/common.h"

/**
 * Used as a sniffer for UDP packets
 * on RAW socket. Skips IP header and UDP
 * header and prints payload on stdout.
 */
struct sniffer {
  /* Fd for socket */
  int raw_socket;
};

struct sniffer* create_sniffer();

void run_sniffer(struct sniffer* sniffer);

char* extract_payload(char* buffer);

void free_sniffer(struct sniffer* sniffer);

#endif // !SNIFFER_H
