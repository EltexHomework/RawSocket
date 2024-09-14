#include "../headers/client.h"

struct client* client;

void cleanup();

int main(void) {
  const char mac[MAC_SIZE] = SERVER_MAC; 
  client = create_client(SERVER_IP, SERVER_PORT, mac, CLIENT_IP);
  atexit(cleanup);
  run_client(client);
  exit(EXIT_SUCCESS);
}

void cleanup() {
  close_connection(client);
  free_client(client);
}
