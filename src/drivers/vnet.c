
#include "../net.h"

/* This driver relies on a host OS anyway. */
#include <stdio.h>

#include "../ether.h"
#include "../mem.h"
#ifdef NET_CON_ECHO
#include "../bstrlib.h"
#endif /* NET_CON_ECHO */
#ifdef _WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#endif /* _WIN32 */
#include <unistd.h>
#include <pcap.h>

char g_pcap_errbuff[PCAP_ERRBUF_SIZE] = { 0 };

/* Open a socket and get the MAC address the socket will send from. */
NET_SOCK net_open_socket( bstring ifname ) {
   pcap_t* socket = NULL;
   char* ifname_c = bdata( ifname ); /* Prevent warnings later. */
   
   /* Open a raw socket from the host OS. */
#ifdef NET_CON_ECHO
   printf( "Opening Socket on: %s\n", ifname_c );
#endif /* NET_CON_ECHO */
   socket = pcap_open_live( ifname_c, RECV_BUFFER_LEN, 1, 512, g_pcap_errbuff );
   if( NULL == socket ) {
      perror( "Unable to open raw socket" );
      goto cleanup;
   }
#ifdef NET_CON_ECHO
   printf( "Opened Socket: %p\n", socket );
#endif /* NET_CON_ECHO */

cleanup:
   return socket;
}

void net_close_socket( NET_SOCK socket ) {
   if( NULL != socket ) {
      pcap_close( socket );
   }
}

#ifdef NET_CON_ECHO
void net_print_frame( struct ether_frame* frame, size_t frame_len ) {
   int i = 0;
   uint8_t type[2] = { 0 };
   bstring buffer = NULL;

   *(uint16_t*)type = ether_ntohs( frame->header.type );

   buffer = blk2bstr( frame, frame_len );
   for( i = 0 ; blength( buffer ) > i ; i++ ) {
      printf( "%02X ", bchar( buffer, i ) );
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

/* cleanup: */
   bdestroy( buffer );
}
#endif /* NET_CON_ECHO */

int net_send_frame( 
   NET_SOCK socket, struct ether_frame* frame, size_t frame_len
) {
   int sent = 0;
   bstring buffer = NULL;
   pcap_t* pcap_socket = (pcap_t*)socket;
   unsigned char* buffer_c = (unsigned char*)bdata( buffer );

   buffer = blk2bstr( frame, frame_len );
#ifdef NET_CON_ECHO
   printf( "Sending on Socket: %p\n", socket );
#endif /* NET_CON_ECHO */
   sent =  pcap_sendpacket( pcap_socket, buffer_c, blength( buffer ) );
   if( 0 != sent ) {
      perror( "Unable to send frame" );
#ifdef NET_CON_ECHO
   } else {
      printf( "Sent: %d bytes.\n", sent );
#endif /* NET_CON_ECHO */
   }

/* cleanup: */
   bdestroy( buffer );
   return sent;
}

struct ether_frame* net_poll_frame( NET_SOCK socket, int* frame_len ) {
   struct ether_frame* frame = NULL;
   const uint8_t* buffer = NULL;
   struct pcap_pkthdr frame_pcap_hdr;
   pcap_t* pcap_socket = (pcap_t*)socket;

   buffer = pcap_next( pcap_socket, &frame_pcap_hdr );
   if( NULL == buffer ) {
      /* Nothing to read. */
      goto cleanup;
   }

   /* Copy the frame to a disposable buffer for use elsewhere. */
   *frame_len = frame_pcap_hdr.len;
   frame = calloc( frame_pcap_hdr.len, 1 );
   memcpy( frame, buffer, frame_pcap_hdr.len );

cleanup:
   return frame;
}

