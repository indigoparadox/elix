
#ifndef ETHER_H
#define ETHER_H

#include <stdint.h>

#define ETHER_BUFFER_SIZE 1024
#define ETHER_IF_NAME_SIZE 8

struct ether_header {
   uint8_t dest_mac[6];
   uint8_t src_mac[6];
   uint16_t type;
};

struct ether_packet {
   struct ether_header header;
   char data[ETHER_BUFFER_SIZE];
};

struct ether_packet* ether_new_packet();

#endif /* ETHER_H */

