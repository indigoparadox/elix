
#include "../net.h"

/* This driver relies on a host OS anyway. */
#include <stdio.h>

#include "../ether.h"
#include "../mem.h"
#ifdef NET_CON_ECHO
#include "../bstrlib.h"
#endif /* NET_CON_ECHO */
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pcap.h>

pcap_t* g_listen_socket = NULL;
char g_pcap_errbuff[PCAP_ERRBUF_SIZE] = { 0 };

/* Open a socket and get the MAC address the socket will send from. */
int net_open_socket( bstring ifname, int* if_idx, uint8_t mac_addr[6] ) {
   int sockfd_out = 0;
   struct ifreq if_idx_req;
   struct ifreq if_mac;
   char* if_name_c = NULL;
   size_t if_name_len = 0;
   int i = 0;
   
   /* Open a raw socket from the host OS. */
   if( -1 == (sockfd_out = socket( AF_PACKET, SOCK_RAW, IPPROTO_RAW)) ) {
      do_error( "Unable to open raw socket.\n" );
      goto cleanup;
   }

   /* Prevent warnings later. */
   if_name_c = bdata( ifname );
   if_name_len = blength( ifname );

   /* Get the sending interface index. */
   memset( &if_idx_req, 0, sizeof( struct ifreq ) );
   strncpy( if_idx_req.ifr_name, if_name_c, if_name_len );
   if( 0 > ioctl( sockfd_out, SIOCGIFINDEX, &if_idx_req ) ) {
      do_error( "SIOCGIFINDEX\n" );
      close( sockfd_out );
      sockfd_out = -1;
      goto cleanup;
   }
   *if_idx = if_idx_req.ifr_ifindex;

   /* Get the sending interface MAC to return. */
   memset( &if_mac, 0, sizeof( struct ifreq ) );
   strncpy( if_mac.ifr_name, if_name_c, if_name_len );
   if( 0 > ioctl( sockfd_out, SIOCGIFHWADDR, &if_mac ) ) {
      do_error( "SIOCGIFHWADDR\n" );
      close( sockfd_out );
      sockfd_out = -1;
      goto cleanup;
   }
   for( i = 0 ; ETHER_ADDRLEN > i ; i++ ) {
      mac_addr[i] = ((uint8_t*)&if_mac.ifr_hwaddr.sa_data)[i];
   }

   /* Open up a listening "socket" with PCAP. */
   g_listen_socket = pcap_open_live( bdata( ifname ), RECV_BUFFER_LEN, 0, 512,
      g_pcap_errbuff );
   if( NULL == g_listen_socket ) {
      perror( "Unable to open listening socket" );
      goto cleanup;
   }

cleanup:
   return sockfd_out;
}

void net_close_socket( int socket ) {
   if( NULL != g_listen_socket ) {
      pcap_close( g_listen_socket );
   }
   if( 0 != socket ) {
      close( socket );
   }
}

void net_print_frame( struct ether_frame* frame, size_t frame_len ) {
   int i = 0;
   uint8_t type[2] = { 0 };
   bstring buffer = NULL;

   *(uint16_t*)type = ether_ntohs( frame->header.type );

   buffer = blk2bstr( frame, frame_len );
   for( i = 0 ; blength( buffer ) > i ; i++ ) {
      //if( '\0' == bchar( buffer, i ) ) {
         printf( "%02X ", (unsigned int)(unsigned char)bchar( buffer, i ) );
      //} else {
      //   printf( "%c", bchar( buffer, i ) );
      //}
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

cleanup:
   bdestroy( buffer );
}

int net_send_frame( 
   int socket, int if_idx, struct ether_frame* frame, size_t frame_len
) {
   size_t sent = 0;
   struct sockaddr_ll socket_address = { 0 };
   bstring buffer = NULL;

   socket_address.sll_halen = ETHER_ADDRLEN;
   socket_address.sll_ifindex = if_idx;
   memcpy( socket_address.sll_addr, frame->header.src_mac, ETHER_ADDRLEN );

   buffer = blk2bstr( frame, frame_len );
   if( 0 > sendto( socket, bdata( buffer ), blength( buffer ), 0,
      (struct sockaddr*)&socket_address, sizeof( struct sockaddr_ll ) )
   ) {
      perror( "Unable to send frame" );
   }

cleanup:
   bdestroy( buffer );
   return sent;
}

struct ether_frame* net_poll_frame( int socket, int* frame_len ) {
   struct ether_frame* frame = NULL;
   const uint8_t* buffer = NULL;
   struct pcap_pkthdr frame_pcap_hdr = { 0 };

   buffer = pcap_next( g_listen_socket, &frame_pcap_hdr );
   if( NULL == buffer ) {
      /* Nothing to read. */
      goto cleanup;
   }

   *frame_len = frame_pcap_hdr.len;
   frame = calloc( frame_pcap_hdr.len, 1 );
   memcpy( frame, buffer, frame_pcap_hdr.len );

cleanup:
   return frame;
}

