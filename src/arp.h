
#ifndef ARP_H
#define ARP_H

#include <stdint.h>

#include "ether.h"

struct arp_header {
   uint16_t hwtype;
   uint16_t prototype;
   uint8_t hwsize;
   uint8_t protosize;
   uint16_t opcode;
} __attribute__((packed));

struct arp_packet_ipv4 {
   struct arp_header header;
   uint8_t src_mac[ETHER_ADDRLEN];
   uint8_t src_ip[ETHER_ADDRLEN_IPV4];
   uint8_t dest_mac[ETHER_ADDRLEN];
   uint8_t dest_ip[ETHER_ADDRLEN_IPV4];
} __attribute__((packed));

enum arp_opcode {
   ARP_REQUEST    = 0x0001,
   ARP_REPLY      = 0x0002,
   ARP_RARP       = 0x0003,
   ARP_RARP_REPLY = 0x0004
};

struct arp_packet_ipv4* arp_new_packet_ipv4(
   enum arp_opcode op, const uint8_t* src_mac, const uint8_t* dest_mac,
   const uint8_t* src_ip, const uint8_t* dest_ip );

#endif /* ARP_H */

