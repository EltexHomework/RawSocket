#include "../headers/sniffer.h"

/*
 * create_sniffer - used to create an object of UDP packet
 * sniffer.
 *
 * Return: pointer to an object of sniffer struct
 */
struct sniffer* create_sniffer() {
  struct sniffer* sniffer = (struct sniffer*) malloc(sizeof(struct sniffer));
  if (!sniffer)
    print_error("malloc");
  
  /* Create Raw UDP socket */
  sniffer->raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP); 
  if (sniffer->raw_socket == -1)
    print_error("socket");

  return sniffer;
}

/*
 * run_sniffer - used to start sniffing UDP packets
 * and printing their payload.
 * @sniffer - pointer to an object of sniffer struct 
 */
void run_sniffer(struct sniffer* sniffer) {
  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(addr);
  int bytes_read;
  char buffer[BUFFER_SIZE];
  char* ptr;

  /* Sniff packets */
  while (1) {
    bytes_read = recvfrom(sniffer->raw_socket, buffer, BUFFER_SIZE, 
                          0, (struct sockaddr*) &addr, &addr_size);
    /* Error occured */
    if (bytes_read == -1) {
      print_error("recvfrom");
    }
    /* Received packet */
    else {
      /* Extract payload */
      ptr = extract_payload(buffer);

      /* Print payload */
      printf("Sniffer UDP packet. Payload: %s\n", ptr);
      free(ptr);
    }
  }
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
 * free_sniffer - used to free allocated memory
 * for sniffer object.
 * @sniffer - pointer to an object of sniffer struct
 */
void free_sniffer(struct sniffer* sniffer) {
  close(sniffer->raw_socket);
  free(sniffer);
}
