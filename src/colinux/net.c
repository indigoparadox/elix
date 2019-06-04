
#include "../net/net.h"

/* This driver relies on a host OS anyway. */
#include <stdio.h>

#include "../net/ether.h"
#include "../mem.h"
#ifdef NET_CON_ECHO
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
#include "../alpha.h"

char g_pcap_errbuff[PCAP_ERRBUF_SIZE] = { 0 };

/* Open a socket and get the MAC address the socket will send from. */
NET_SOCK net_open_socket( SOCKET_ID ifname ) {
   pcap_t* socket = NULL;
   
   /* Open a raw socket from the host OS. */
#ifdef NET_CON_ECHO
   printf( "Opening Socket on: %s\n", ifname );
#endif /* NET_CON_ECHO */
   socket = pcap_open_live( ifname, RECV_BUFFER_LEN, 1, 512, g_pcap_errbuff );
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
   const NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   int sent = 0;
   pcap_t* pcap_socket = (pcap_t*)socket;

#ifdef NET_CON_ECHO
   printf( "Sending on Socket: %p\n", socket );
#endif /* NET_CON_ECHO */
   sent =  pcap_sendpacket( pcap_socket, (unsigned char*)frame, frame_len );
   if( 0 != sent ) {
      perror( "Unable to send frame" );
#ifdef NET_CON_ECHO
   } else {
      printf( "Sent: %d bytes.\n", sent );
#endif /* NET_CON_ECHO */
   }

   return sent;
}

int net_poll_frame(
   const NET_SOCK socket, struct ether_frame* frame, int frame_sz
) {
   const uint8_t* buffer = NULL;
   struct pcap_pkthdr frame_pcap_hdr;
   pcap_t* pcap_socket = (pcap_t*)socket;
   int frame_len = 0;

   buffer = pcap_next( pcap_socket, &frame_pcap_hdr );
   if( NULL == buffer ) {
      /* Nothing to read. */
      goto cleanup;
   }

   /* Copy the frame to a disposable buffer for use elsewhere. */
   if( frame_sz < frame_pcap_hdr.len ) {
      perror( "Captured frame too large for buffer" );
      goto cleanup;
   }
   frame_len = frame_pcap_hdr.len;
   mcopy( frame, buffer, frame_pcap_hdr.len );

cleanup:
   return frame_len;
}

