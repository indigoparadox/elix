
#include "../net/net.h"

#include "../net/ether.h"
#include "../mem.h"

NET_SOCK net_open_socket( SOCKET_ID ifname ) {
   return NULL;
}

void net_close_socket( NET_SOCK socket ) {
}

#ifdef NET_CON_ECHO
void net_print_frame( struct ether_frame* frame, int frame_len ) {
   int i = 0;
   uint8_t type[2] = { 0 };
   char* buffer_c = NULL;

   *(uint16_t*)type = ether_ntohs( frame->header.type );

   buffer_c = (char*)frame;
   for( i = 0 ; frame_len > i ; i++ ) {
      printf( "%02X ", (uint8_t)buffer_c[i] );
      if( 0 == (i + 1) % 16 ) {
         printf( "\n" );
      }
   }
   printf( "\n" );
 
   printf( "*Frame Information:\nSource MAC: " );
   for( i = 0 ; ETHER_ADDRLEN > i ; i++ ) {
      printf( "%02X ", frame->header.src_mac[i] );
   }
   printf( "\nDest MAC: " );
   for( i = 0 ; ETHER_ADDRLEN > i ; i++ ) {
      printf( "%02X ", frame->header.dest_mac[i] );
   }
   printf( "\nType: %02X %02X\n", type[0], type[1]  );
}
#endif /* NET_CON_ECHO */

int net_send_frame( 
   NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   return 0;
}

int net_poll_frame(
   NET_SOCK socket, struct ether_frame* frame, int frame_sz
) {
   return 0;
}

