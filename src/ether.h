
#ifndef ETHER_H
#define ETHER_H

#include <stdint.h>
#include "bstrlib.h"

#define ETHER_BUFFER_SIZE 1024

struct ether_header {
   uint8_t dest_mac[6];
   uint8_t src_mac[6];
   uint16_t type;
};

struct ether_packet {
   struct ether_header header;
   char data[ETHER_BUFFER_SIZE];
};

struct ether_packet* ether_new_packet(
   uint8_t src_mac[6], uint8_t dest_mac[6], uint16_t type, bstring data );

#endif /* ETHER_H */

