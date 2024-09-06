#include "../headers/sniffer.h"

struct sniffer* sniffer;

void cleanup();

int main(void) {
  atexit(cleanup);
  sniffer = create_sniffer();
  printf("Starting sniffer\n");

  run_sniffer(sniffer);
  exit(EXIT_SUCCESS);
}

void cleanup() {
  free_sniffer(sniffer);
}
