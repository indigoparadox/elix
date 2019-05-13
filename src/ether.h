
#ifndef ETHER_H
#define ETHER_H

#include <stdint.h>
#include "bstrlib.h"

#define ETHER_BUFFER_SIZE 1024
#define ETHER_ADDRLEN 6
#define ETHER_ADDRLEN_IPV4 4

enum ether_type {
   ETHER_TYPE_ETHER = 1,
   ETHER_TYPE_IPV4  = 0x0400,
   ETHER_TYPE_ARP   = 0x0806,
   ETHER_TYPE_IPX   = 0x8137,
   ETHER_TYPE_IPV6  = 0x86dd
};

enum arp_opcode {
   ARP_REQUEST    = 0x1,
   ARP_REPLY      = 0x2,
   ARP_RARP       = 0x3,
   ARP_RARP_REPLY = 0x4
};

struct ether_header {
   uint8_t dest_mac[6];
   uint8_t src_mac[6];
   uint16_t type;
} __attribute__((packed));

struct ether_packet {
   struct ether_header header;
   char data[];
} __attribute__((packed));

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

#define ether_htons( input ) ether_ntohs( input )
#define ether_htonl( input ) ether_ntohl( input )

struct ether_packet* ether_new_packet(
   uint8_t src_mac[6], uint8_t dest_mac[6], enum ether_type type,
   void* data, size_t data_len );
struct arp_packet_ipv4* ether_new_arp_packet_ipv4(
   enum arp_opcode op, const uint8_t* src_mac, const uint8_t* dest_mac,
   const uint8_t* src_ip, const uint8_t* dest_ip );
uint16_t ether_ntohs( const uint16_t input );
uint32_t ether_ntohl( const uint32_t input );

#endif /* ETHER_H */

