
#ifndef ETHER_H
#define ETHER_H

#include <stdint.h>

#define ETHER_BUFFER_SIZE 1024

#define ETHER_HEADER_SIZE 18
#ifdef NET_JUMBO_FRAMES
#define ETHER_FRAME_DATA_SIZE_MAX (ETHER_HEADER_SIZE + 9180) /* Jumbo */
#else
#define ETHER_FRAME_DATA_SIZE_MAX (ETHER_HEADER_SIZE + 1524)
#endif /* NET_JUMBO_FRAMES */
#define ETHER_FRAME_SIZE_MAX (ETHER_HEADER_SIZE + ETHER_FRAME_DATA_SIZE_MAX)

#define ETHER_ADDRLEN 6
#define ETHER_ADDRLEN_IPV4 4

#define ETHER_ERROR_FRAME_OVERFLOW -2
#define ETHER_ERROR_INVALID_BUFFER -4

enum ether_type {
   ETHER_TYPE_ETHER = 1,
   ETHER_TYPE_IPV4  = 0x0800,
   ETHER_TYPE_ARP   = 0x0806,
   ETHER_TYPE_IPX   = 0x8137,
   ETHER_TYPE_IPV6  = 0x86dd
};

struct ether_header {
   uint8_t dest_mac[6];
   uint8_t src_mac[6];
   uint16_t type;
   /* ? */
} __attribute__((packed));

struct ether_frame {
   struct ether_header header;
   char data[ETHER_FRAME_DATA_SIZE_MAX];
} __attribute__((packed));

#define ether_htons( input ) ether_ntohs( input )
#define ether_htonl( input ) ether_ntohl( input )

int ether_new_frame(
   struct ether_frame* frame_out, int frame_out_sz,
   const uint8_t src_mac[6], const uint8_t dest_mac[6],
   enum ether_type type, void* packet, int packet_len );
int ether_get_header_len( struct ether_frame* frame, int frame_len );
uint16_t ether_ntohs( const uint16_t input );
uint32_t ether_ntohl( const uint32_t input );

#endif /* ETHER_H */

